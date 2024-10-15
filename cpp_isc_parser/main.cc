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

    int optind = option.parse(argc, argv);
    if (argc <= 1)
    {
        option.usage();
        std::cout << "No valid options provided. Exiting." << std::endl;
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

    string action = (string)option.retrieve("action");
    string file_isc = (string)option.retrieve("file_isc");

    if (action == "parse_isc")
    {
        cout << "ISC file: " << file_isc << "\n"
             << endl;

        isc_Circuit = parse_isc_main(file_isc);
    }

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

    return 0;
}
