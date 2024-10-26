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
                  "Action to perform, available options: \n\t\t parse_isc: parse the input isc file\n\t\t ppsfp: perform parallel pattern single fault simulation on the circuit\n\t\t", 0);

    option.enroll("file_isc", GetLongOpt::MandatoryValue,
                  "Input isc circuit file path. e.g C17.isc", 0);
    option.enroll("help", GetLongOpt::NoValue,
                  "Print this help summary", 0);

    option.enroll("input", GetLongOpt::MandatoryValue,
                  "set the input pattern file", 0);
    option.enroll("output", GetLongOpt::MandatoryValue,
                  "set the output pattern file", 0);

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

        isc_Circuit->Levelize();
        isc_Circuit->init_bitset(true, true, true); // bool inputv, bool oe, bool oa
        cout << "Circuit initalizated." << endl;

        //  isc_Circuit->print_bitset();
        isc_Circuit->init_level0_input_gate();

        cout << "Random patterns generated on Input gates, parallel pattern count 64." << endl;
        // isc_Circuit->print_bitset();

        /******************************************
         * CACLUATE THE ERROR FREE CIRCUIT OUTPUT
        /******************************************/
        int total_sa_error = isc_Circuit->get_sa_error_cnt();
        int detected_sa_error = 0;

        // calc expect value for  level 1 to level max gates
        for (int gate_level = 1; gate_level <= isc_Circuit->GetMaxLevel(); gate_level++)
        {
            isc_Circuit->calc_output_level_1_max(gate_level, "EXPECT");
        }

        for (int gate_level = 1; gate_level <= isc_Circuit->GetMaxLevel(); gate_level++)
        {
            isc_Circuit->calc_output_level_1_max(gate_level, "ACTUAL");
        }

        cout << "Good circuit output calculated." << endl;

        if (read_input("Show patterns on Gates? 'yes' or 'no': "))
        {
            // string a = isc_Circuit->GetFunctionString(G_PI);
            isc_Circuit->print_bitset();
        }

        if (!read_input("Run Parallel Pattern Single Fault (PPSF) Simulation? 'yes' or 'no': "))
        {
            cout << "Exiting." << endl;
            exit(0);
        }

        // isc_Circuit->printNetlist();

        /******************************************
         * INJECT SA FAULTS
        /******************************************/
        // iterate the FAULTS in circuits
        cout << "\nInjecting SA faults one at a time. See if any 64 parallel Pattern could catch the fault." << endl;
        detected_sa_error += isc_Circuit->iterate_gates_sa_errors(detected_sa_error);
        double err_detected_ratio = (double)detected_sa_error / (double)total_sa_error;

        cout << "Total SA errors: " << total_sa_error << ", detected " << detected_sa_error << ". detect ratio " << err_detected_ratio << endl;
    }
    else if (action == "PODEM")
    {

        cout << "\nLevelize the gates in circuit" << endl;
        isc_Circuit->Levelize();

        cout << "\nCalculating gates controlability" << endl;
        isc_Circuit->calc_gate_controlabilty();

        if (read_input("\nShow Controllability CC0, CC1 ? 'yes' or 'no': "))
        {
            isc_Circuit->show_controlability();
        }


        cout << "\nInitalizing gates for backtracing. Set gate input to x"<< endl;
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

    else
    {
        cout << "Invalid action provided. Exiting." << endl;
        exit(0);
    }

    return 0;
}
