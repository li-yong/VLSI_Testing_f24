#include <iostream>
#include <ctime>
#include <string>
#include "circuit.h"
#include "GetLongOpt.h"
#include "parser_isc.h"
#include <typeinfo>
#include <random>

#include <vector>
#include <bitset>
#include <algorithm>
#include "LFSR.h"

using namespace std;

// All defined in readcircuit.l
extern char *yytext;
extern FILE *yyin;
extern CIRCUIT Circuit;
extern CIRCUIT isc_Circuit;

extern int yyparse(void);
extern bool ParseError;

extern void Interactive();

GetLongOpt option;

int SetupOption(int argc, char **argv)
{

    option.enroll("action", GetLongOpt::MandatoryValue,
                  "Action to perform, available options: \n\t\t parse_isc: parse the input isc file.\n\t\t ppsfp: `Parallel Pattern Single Fault Propagation` Simulation on the circuit.\n\t\t ATPG: Automatic Test Pattern Generation (ATPG) with Path-Oriented Decision Making (PODEM) algorithm.\n\t\t BIST: BIST Circuit simulation.", 0);

    option.enroll("file_isc", GetLongOpt::MandatoryValue,
                  "Input isc circuit file path. e.g C17.isc", 0);
    option.enroll("help", GetLongOpt::NoValue,
                  "Print this help summary", 0);

    int optind = option.parse(argc, argv);
    if (argc <= 1)
    {
        option.usage();
        cout << "No valid options provided. Exiting." << endl;
        exit(0);
    }
    if (option.retrieve("help"))
    {
        option.usage();
        exit(0);
    }
    return optind;
}

bool read_input(const string &prompt)
{
    string input;
    cout << prompt;      // Display the prompt to the user
    getline(cin, input); // Read the user's input

    // Convert input to lowercase for case-insensitive comparison
    transform(input.begin(), input.end(), input.begin(), ::tolower);

    // Return true if input is "yes" or "y", otherwise false
    return (input == "yes" || input == "y");
}

int main(int argc, char **argv)
{
    SetupOption(argc, argv);

    // Start parsing ISC
    CIRCUIT *isc_Circuit = new CIRCUIT();

    string action = (string)option.retrieve("action");
    string file_isc = (string)option.retrieve("file_isc");

    cout << "ISC file: " << file_isc << "\n"
         << endl;

    isc_Circuit = parse_isc_main(file_isc);

    isc_Circuit->SetupIO_ID();

    cout << "Circuit Gates:" << endl;
    cout << "Num of Gate " << isc_Circuit->No_Gate() << endl;
    cout << "Num of PI " << isc_Circuit->No_PI() << endl;
    cout << "Num of PO " << isc_Circuit->No_PO() << endl;

    if (action == "parse_isc")
    {
        string pattern_name = (string)option.retrieve("output");

        /////////////////////////////////
        //
        // Gate number, followed by gate type, followed by a list of fanout gates
        // 1 PI 10
        // 2 PI 16
        // 3 PI 11 10
        //////////////////////////////////
        cout << "Gate\tType\tFanout" << endl;
        isc_Circuit->printGateIdTypeOutput();
        cout << endl;

        /////////////////////////////////
        // write out the fault list in the format <gate1 gate2 fault> as shown below, where
        // the stuck-at-fault 0 or 1 is in the connection between gate1 and gate2.
        // 1 0 1
        // 2 0 1
        // 3 0 0
        /////////////////////////////////

        cout << "Gate1\tGate2\tFault" << endl;
        isc_Circuit->printSA();
        cout << "\nCircuit processing completed. " << file_isc << "\n"
             << endl;
    }
    else if (action == "ppsfp")
    {
        /******************************************
         * INIT THE 64bit BITSET ON EACH INPUT GATE
        /******************************************/

        bool interaction = true;

        isc_Circuit->Levelize();
        isc_Circuit->init_bitset(true, true, true); // bool inputv, bool oe, bool oa
        cout << "Circuit initalizated." << endl;

        int total_sa_error = isc_Circuit->get_sa_error_cnt();
        int detected_sa_error_realtime = 0;
        int detected_sa_error = 0;

        int loop_cnt = 0;

        for (int loop_num = 1; loop_num < 11; ++loop_num)
        {
            if (detected_sa_error_realtime == total_sa_error)
            {
                cout << "\nAll SA errors detected in " << loop_cnt << " loops." << endl;
                break;
            }

            cout << "\nLoop number: " << loop_num << endl;

            //  isc_Circuit->print_bitset();
            isc_Circuit->init_level0_input_gate();
            cout << "Random patterns generated on Input gates, parallel pattern count 64." << endl;
            // isc_Circuit->print_bitset();

            /******************************************
             * CACLUATE THE ERROR FREE CIRCUIT OUTPUT
            /******************************************/
            // calc expect value for  level 1 to level max gates
            for (int gate_level = 1; gate_level <= isc_Circuit->GetMaxLevel(); gate_level++)
            {
                // isc_Circuit->print_bitset();
                isc_Circuit->calc_output_level_1_max(gate_level, "EXPECT", vector<string>{});
                // isc_Circuit->print_bitset();
            }

            for (int gate_level = 1; gate_level <= isc_Circuit->GetMaxLevel(); gate_level++)
            {
                // isc_Circuit->print_bitset();
                isc_Circuit->calc_output_level_1_max(gate_level, "ACTUAL", vector<string>{});
                // isc_Circuit->print_bitset();
            }

            cout << "Good circuit output calculated." << endl;

            if (interaction && read_input("Show patterns on Gates? 'yes' or 'no': "))
            {
                isc_Circuit->print_bitset();
            }

            if (interaction)
            {
                if (!read_input("Interactive mode? 'yes' or 'no': "))
                {
                    interaction = false;
                }
            }

            /******************************************
             * INJECT SA FAULTS
            /******************************************/
            // iterate the FAULTS in circuits
            cout << "\nInjecting SA faults one at a time. See if any 64 parallel Pattern could catch the fault." << endl;
            detected_sa_error_realtime += isc_Circuit->iterate_gates_sa_errors(detected_sa_error);
            // isc_Circuit->iterate_gates_sa_errors(detected_sa_error);
            loop_cnt++;

        } // end of the for loop

        int left_sa_error = isc_Circuit->get_sa_error_cnt();
        detected_sa_error = total_sa_error - left_sa_error;

        double err_detected_ratio = (double)detected_sa_error / (double)total_sa_error;

        cout << "\n=== PPSFP Simulation Completed ===" << endl;
        cout << "Circuit              : " << file_isc << endl;
        cout << "Total SA Errors      : " << total_sa_error << endl;
        cout << "Detected Errors      : " << detected_sa_error << endl;
        cout << "Detection Ratio      : " << err_detected_ratio << endl;
        cout << "Iteration Count      : " << loop_cnt << endl;
    }
    else if (action == "ATPG")
    {

        cout << "\nLevelize the gates in circuit" << endl;
        isc_Circuit->Levelize();

        cout << "\nCalculating gates controllability" << endl;
        isc_Circuit->calc_gate_controlabilty();

        if (read_input("\nShow Controllability CC0, CC1 ? 'yes' or 'no': "))
        {
            isc_Circuit->show_controllability();
        }

        cout << "\nInitalizing gates for backtracing. Set gate input to x" << endl;
        isc_Circuit->podem_bt_candidates_init();
        // isc_Circuit->Atpg();

        int sa_error_cnt = isc_Circuit->get_sa_error_cnt();

        // iterate the SAlist
        if (!read_input("\nRun PODEM ATPG? 'yes' or 'no': "))
        {
            cout << "\nExiting." << endl;
            exit(0);
        }

        cout << "\nIterating all SA errors in circuit, then backtrace input pattern to justify the SA error" << endl;
        vector<GATE *> netlist = isc_Circuit->GetNetlist();

        for (unsigned i = 0; i < netlist.size(); i++)
        {

            GATE *g; // gate under test
            g = netlist[i];
            string gate_isc_identifier = g->Get_isc_identifier();

            GATEFUNC fun = g->GetFunction();
            string stuck_error;

            vector<string> SAlist = g->Get_isc_StuckAt();

            for (size_t n = 0; n < SAlist.size(); ++n)
            {

                string target_value = "0";

                // cout << error_gate_isc_ident << " " << SAlist[n] << endl;

                if (SAlist[n] == ">sa0")
                {
                    target_value = "1"; // if stuck at 0, then normal value should be 1
                }
                else if (SAlist[n] == ">sa1")
                {
                    target_value = "0";
                }

                // find a path by backtracing.
                // string gate_isc_identifier = "23gat";
                cout << "\n==" << SAlist[n] << "@" << gate_isc_identifier << ". Backtracing to find PI gate pattern to justify gate " << gate_isc_identifier << " to value " << target_value << endl;
                GATE::Result result = isc_Circuit->isc_findPath(gate_isc_identifier, target_value);
                if (result.resolved)
                {
                    string input_gate_value = isc_Circuit->bt_get_input_value();
                    cout << "\t" << input_gate_value << endl;
                }

                cout << "\nPress ENTER to continue..., ctrl+c to exit" << std::endl;
                cin.get();
                // cout << "print path" << endl;

                // iterate the isc_circuit netlist

                // cout << "PODEM" << endl;
            }
        } // for loop, salist.

    } // for loop, netlist.

    else if (action == "test_case_01010001")
    {
        vector<int> poly_vec = {3, 1}; // X^5+X^3+X+1.

        bool debug = false;
        // debug = true;

        string inputS = "01010001";
        string initS = "00000"; // initial state is all 0 if inputS is provided.
        LFSR *lfsr_tpg = new LFSR(5);
        int scan_ff_num = 5;

        inputS = "01010001";

        vector<int> input_vector = isc_Circuit->convert_stringToBinaryVector(inputS, true);
        vector<string> signature = {};

        for (int i = 0; i < input_vector.size(); ++i)
        {

            signature = isc_Circuit->ora_misr(lfsr_tpg, poly_vec, scan_ff_num, input_vector[i]); // just print
            if (debug)
            {
                cout << "\tmisr_loop " << i << ", input " << input_vector[i] << ", output: ";

                for (int j = 0; j < signature.size(); ++j)
                {
                    cout << signature[j];
                }
                cout << endl;
            }
        }
        exit(0);
    }
    else if (action == "test_case_111")
    {
        vector<int> poly_vec_tpg = {1}; // X^3+x+1

        bool debug = false;
        debug = true;

        string initS = "111"; // initial state is all 0 if inputS is provided.
        LFSR *lfsr_tpg = new LFSR(3);
        int scan_ff_num = 3;

        vector<int> bv = isc_Circuit->convert_stringToBinaryVector(initS, true);

        /* Initiate LFSR  */

        for (int i = 0; i < bv.size(); ++i)
        {
            lfsr_tpg->setBit(i, bv[i]);
        }
        cout << "initial " << initS << endl;

        int i = 0;

        while (i < 8)
        {
            i += 1;
            vector<int> tmp = isc_Circuit->tpg_lfsr(lfsr_tpg, poly_vec_tpg, scan_ff_num, debug);
            for (int j = 0; j < tmp.size(); ++j)
            {
                cout << tmp[j];
            }
            cout << endl;
            // cout << "" << endl;
        }

        exit(0);
    }
    else if (action == "BIST")
    {

        vector<int> poly_vec = {3, 1}; // X^5+X^3+X+1.

        // LFSR is 32-bits with equation h(x) = x32 + x22 + x2 + x1 + 1
        vector<int> poly_vec_tpg = {22, 2, 1};
        int sff_num_tpg = 32;

        // The ORA is a 16-bit LFSR whose equation is h(x) = x16 + x15 + x13 + x4 + 1.
        vector<int> poly_vec_ora = {15, 13, 4};
        int sff_num_ora = 16;

        int scan_ff_num; // number of scan flip-flops.

        // bool test_case_01010001; // Enable the inputS. Set to True if inputS is provided.
        string inputS; // The string feeding to the input of the LFSR. Not the feedback input.

        string initS; // The initial state of the LFSR.

        LFSR *lfsr_tpg; // Test Pattern Generator LFSR

        bool debug = false;
        // debug = true;

        // Initialize the register with the input
        // Iterate from right (least significant bit) to left (most significant bit)

        initS = "101010101010101010101010100101010";
        initS = "10101010101010101010101010010101";

        initS = initS.substr(initS.size() - sff_num_tpg); // take the last 32 bits of the string.

        lfsr_tpg = new LFSR(sff_num_tpg); // Test Pattern Generator LFSR, 32bits

        vector<int> bv = isc_Circuit->convert_stringToBinaryVector(initS, true);

        /* Initiate LFSR  */

        for (int i = 0; i < bv.size(); ++i)
        {
            lfsr_tpg->setBit(i, bv[i]);
            // cout << bv[i];
            // auto d = lfsr_tpg->getBit(i);
            // cout << d;
        }

        // cout << endl;

        // uint32_t a = lfsr_tpg->get32bit();

        // // a = 8;
        // vector<int> b = isc_Circuit->convert_intToBinaryVector(a, 32);

        // for (int i = 0; i < b.size(); ++i)
        // {
        //     cout << b[i];
        // }
        // cout << endl;

        /* SCAN  */

        bool interaction = true;
        interaction = false; // debug

        isc_Circuit->Levelize();
        isc_Circuit->init_bitset(true, true, true); // bool inputs_value, bool output_expected, bool output_actual

        // isc_Circuit->print_bitset();

        cout << "Circuit initialized." << endl;

        int total_sa_error = isc_Circuit->get_sa_error_cnt();
        int detected_sa_error_realtime = 0;
        int detected_sa_error = 0;

        int loop_num = 100; // 100
        // cout << lfsr->get16bit() << endl;

        LFSR *lfsr_ora = new LFSR(16);

        int alias_cnt = 0;
        int loop_cnt = 0;

        while (loop_cnt < loop_num)
        {
            if (detected_sa_error_realtime == total_sa_error)
            {
                cout << "\nAll SA errors detected in " << loop_cnt << " loops." << endl;
                break;
            }

            vector<int> tpg_generated_input = {};

            while (tpg_generated_input.size() < isc_Circuit->No_PI())
            {

                // isc_Circuit->print_lfsr_32(lfsr_tpg); // debug

                vector<int> tmp = isc_Circuit->tpg_lfsr(lfsr_tpg, poly_vec_tpg, sff_num_tpg, debug);
                tpg_generated_input.insert(tpg_generated_input.end(), tmp.begin(), tmp.end());
            }

            // if (debug)
            // {
                cout << "\nLoop " << loop_cnt << ", TPG generated test pattern ";

                for (int i = 0; i < tpg_generated_input.size(); ++i)
                {
                    cout << tpg_generated_input[i];
                }

                cout << '\n';
            // }

            //  ASSIGN TPG GENERATED INPUT PATTERN TO GATES
            tpg_generated_input = isc_Circuit->assign_tpg_to_input(tpg_generated_input, debug);

            /******************************************
             * CACLUATE THE ERROR FREE CIRCUIT OUTPUT
            /******************************************/
            // calc expect value for  level 1 to level max gates
            for (int gate_level = 1; gate_level <= isc_Circuit->GetMaxLevel(); gate_level++)
            {
                // isc_Circuit->print_bitset(true);
                isc_Circuit->calc_output_level_1_max(gate_level, "EXPECT", vector<string>{});
                // isc_Circuit->print_bitset(true);
            }

            for (int gate_level = 1; gate_level <= isc_Circuit->GetMaxLevel(); gate_level++)
            {
                // isc_Circuit->print_bitset(true);
                isc_Circuit->calc_output_level_1_max(gate_level, "ACTUAL", vector<string>{});
                // isc_Circuit->print_bitset(true);
            }

            cout << "Good circuit output was simulated. ";

            if (debug)
            {
                cout << "good circuit inputs, expected output, actual output: " << endl;
                isc_Circuit->print_bitset(true);
            }

            // calculate the golden signature
            string circuit_output = isc_Circuit->get_circuit_output();

            // if (debug)
            // {
                cout << "Good circuit output: " << circuit_output << endl;
            // }

            cout << "Calculating signature on ORA. " ;
            vector<string> golden_signature = isc_Circuit->calc_po_signature(circuit_output, lfsr_ora, poly_vec_ora, sff_num_ora, debug);

            // print out the golden_signature
            cout << "Golden signature calculated: ";
            for (int i = 0; i < golden_signature.size(); ++i)
            {
                cout << golden_signature[i];
            }

            cout << endl;

            if (interaction && read_input("Show patterns on Gates? 'yes' or 'no': "))
            {
                isc_Circuit->print_bitset(true);
            }

            if (interaction)
            {
                if (!read_input("Interactive mode? 'yes' or 'no': "))
                {
                    interaction = false;
                }
            }
            /******************************************
             * INJECT SA FAULTS
            /******************************************/
            // iterate the FAULTS in circuits
            cout << "\nInjecting SA faults one at a time." << endl;

            detected_sa_error_realtime += isc_Circuit->iterate_gates_sa_errors_lfsr(detected_sa_error, poly_vec_ora, sff_num_ora, golden_signature, circuit_output, &alias_cnt, debug);
            // isc_Circuit->iterate_gates_sa_errors(detected_sa_error);
            loop_cnt++;
            // cout << "Loop cont: " << loop_cnt << endl;

            // } // end of the while loop
        } // for loop, 100 times on BIST simulation.

        int left_sa_error = isc_Circuit->get_sa_error_cnt();
        detected_sa_error = total_sa_error - left_sa_error;

        double err_detected_ratio = (double)detected_sa_error / (double)total_sa_error;

        cout << "\n=== LFSR Simulation Completed ===" << endl;
        cout << "Circuit              : " << file_isc << endl;
        cout << "Total SA Errors      : " << total_sa_error << endl;
        cout << "Detected Errors      : " << detected_sa_error << endl;
        cout << "Detection Ratio      : " << err_detected_ratio << endl;
        cout << "Alias Cnt            : " << alias_cnt << endl;
        cout << "Loop  Cnt            : " << loop_cnt << endl;

        // cout << "Iteration Count      : " << loop_cnt << endl;
    }

    else
    {
        cout << "Invalid action provided. Exiting." << endl;
        exit(0);
    }

    return 0;
}
