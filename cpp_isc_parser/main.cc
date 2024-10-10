#include <iostream>
#include <ctime>
#include <string>
#include "circuit.h"
#include "GetLongOpt.h"
#include "pattern.h"
#include "parser_isc.h"
#include <typeinfo>
using namespace std;

// All defined in readcircuit.l
extern char *yytext;
extern FILE *yyin;
extern CIRCUIT Circuit;
extern CIRCUIT isc_Circuit; // yong

extern int yyparse(void);
extern bool ParseError;

extern void Interactive();

GetLongOpt option;


int SetupOption(int argc, char **argv)
{
    option.usage("--parse_isc --file_isc <filename>");

    // -------------------------------------
    option.enroll("parse_isc", GetLongOpt::NoValue,
                  "parse the input isc file", 0);
    option.enroll("file_isc", GetLongOpt::MandatoryValue,
                  "input isc file path", 0);
    option.enroll("help", GetLongOpt::NoValue,
                  "print this help summary", 0);

    int optind = option.parse(argc, argv);
    if (optind < 1)
    {
        std::cout << "No valid options provided. Exiting." << std::endl;
        exit(0);
    }
    if (option.retrieve("help"))
    {
        std::cout << "Help option detected. Showing usage." << std::endl;
        option.usage();
        exit(0);
    }
    return optind;
}

//add an usage function to print out the usage of the program
void usage()
{
        cout << "Usage: ./main [options]" << endl;
        cout << "Options:" << endl;
        cout << "  --parse_isc --file_isc <filename> : parse the input isc file" << endl;
        cout << "  --help : print this help summary" << endl;
}

int main(int argc, char **argv)
{
        int optind = SetupOption(argc, argv);

        // Start parsing ISC
        CIRCUIT *isc_Circuit;
        if (option.retrieve("parse_isc"))
        {
                string file_isc = (string)option.retrieve("file_isc");

                isc_Circuit = parse_isc_main(file_isc);
        }

        /////////////////////////////////
        //
        // Gate number, followed by gate type, followed by a list of fanout gates
        // 1 PI 10
        // 2 PI 16
        // 3 PI 11 10
        //////////////////////////////////
        cout << "Gate_number\tGate_type\tFanout_gates_list" << endl;
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

        cout << "Press Enter to continue..." << endl;
        // cin.get();

        return 0;
}
