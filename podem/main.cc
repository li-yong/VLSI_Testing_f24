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
        // -------------------------------------
        option.enroll("parse_isc", GetLongOpt::NoValue,
                      "parse the input isc file", 0);
        option.enroll("file_isc", GetLongOpt::MandatoryValue,
                      "input isc file path", 0);

        int optind = option.parse(argc, argv);
        if (optind < 1)
        {
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
        int optind = SetupOption(argc, argv);
        clock_t time_init, time_end;
        time_init = clock();


        // Start parsing ISC

        CIRCUIT *isc_Circuit;
        if (option.retrieve("parse_isc"))
        {
                string file_isc = (string)option.retrieve("file_isc");



                isc_Circuit = parse_isc_main(file_isc);

        }

        // end of parsing ISC

        // iterate the isc_Circuit, print the gate info.
        // isc_Circuit->printNetlist();
        // isc_Circuit->printPOInputList();
        // isc_Circuit->printGateOutput();


        // isc_Circuit->printGateIdTypeOutput();
        isc_Circuit->printSA();


        time_end = clock();
        cout << "total CPU time = " << double(time_end - time_init) / CLOCKS_PER_SEC << endl;
        cout << endl;

        return 0;
}
