/* Logic Simulator
 * Last update: 2006/09/20 */
#include <iostream>
#include <bitset>
#include "gate.h"
#include "circuit.h"
#include "pattern.h"
#include "GetLongOpt.h"
using namespace std;

extern GetLongOpt option;


extern GATE* NameToGate(string);

void PATTERN::Initialize(char* InFileName, int no_pi, string TAG)
{
    patterninput.open(InFileName, ios::in);
    if (!patterninput.is_open()) {
        cout << "Unable to open input pattern file: " << InFileName << endl;
        exit( -1);
    }
    string piname;
    while (no_pi_infile < no_pi) {

    }
    return;
}

//initial Queue for logic simulation
void CIRCUIT::InitializeQueue()
{
    SetMaxLevel();
    Queue = new ListofGate[MaxLevel + 1];
    return;
}
