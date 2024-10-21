#include <iostream>
#include <ctime>
#include <string>
#include "circuit.h"
#include "GetLongOpt.h"
#include "pattern.h"
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
                  "Action to perform, available options: \n\t\t parse_isc: parse the input isc file", 0);

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

int main(int argc, char **argv)
{
    SetupOption(argc, argv);

    // Start parsing ISC
    CIRCUIT *isc_Circuit = new CIRCUIT();
    string pattern_name = (string)option.retrieve("output");

    string action = (string)option.retrieve("action");
    string file_isc = (string)option.retrieve("file_isc");

    cout << "ISC file: " << file_isc << "\n"
         << endl;

    isc_Circuit = parse_isc_main(file_isc);

    if (action == "parse_isc")
    {

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

        // isc_Circuit->FanoutList(); //fuck!

        // isc_Circuit->SetupIO_ID(); // increase the input gate number. drop it. FUCK!
//  isc_Circuit->print_bitset();
        isc_Circuit->Levelize(); // Ryan overwrited
        // isc_Circuit->Check_Levelization();
        isc_Circuit->InitializeQueue();

        isc_Circuit->GenerateAllFaultList();
        isc_Circuit->SortFaninByLevel();
        // isc_Circuit->MarkOutputGate();

        // isc_Circuit->print_bitset();
        isc_Circuit->init_bitset();
//  isc_Circuit->print_bitset();
        isc_Circuit->init_level0_input_gate();
        // isc_Circuit->print_bitset();

        /******************************************
         * CACLUATE THE ERROR FREE CIRCUIT OUTPUT
        /******************************************/

        // calc expect value for  level 1 to level max gates
        for (int gate_level = 1; gate_level <= isc_Circuit->GetMaxLevel(); gate_level++)
        {
            isc_Circuit->calc_output_level_1_max(gate_level, "EXPECT");
            isc_Circuit->calc_output_level_1_max(gate_level,"ACTUAL");
        }

      
        isc_Circuit->print_bitset();
        // exit(0);


        /******************************************
         * INJECT SA FAULTS
        /******************************************/
        // iterate the FAULTS in circuits
        isc_Circuit->iterate_gates_sa_errors();

        // isc_Circuit->Levelize_0();

        // feed the random vector to the circuit, input gates
        for (int i = 0; i < isc_Circuit->No_PI(); i++)
        {
            // isc_Circuit->PIGate(i)->SetValue(random_vector[i]);
            // isc_Circuit->PIGate(i)->SetValue(input_patterns[i][0]);

            cout << "debug" << endl;
        }

        PATTERN pattern;
        // pattern->Initialize("pattern", isc_Circuit->No_PI(), "PI");
        // isc_Circuit->copyPItoPattern();
        // isc_Circuit->Pattern.genRandomPattern(10);
        // isc_Circuit->Pattern.setPatterninput();
        // isc_Circuit->Levelize_0();
        // isc_Circuit->Levelize_1();

        /* Set Pattern*/

        // isc_Circuit->MarkOutputGate();
        // isc_Circuit->InitializeQueue();
        // isc_Circuit->PPSFP();
    }
    else if (action == "path")
    {
        string src_name_gate = "1";
        string dest_gate_name = "2";
        isc_Circuit->path(src_name_gate, dest_gate_name);
    }
    else
    {
        cout << "Invalid action provided. Exiting." << endl;
        exit(0);
    }

    return 0;
}
