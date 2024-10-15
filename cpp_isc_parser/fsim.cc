/* stuck-at fault simulator for combinational circuit
 * Last update: 2006/12/09 */
#include <iostream>
#include "circuit.h"
#include "GetLongOpt.h"
using namespace std;

extern GetLongOpt option;


//mark gates connecting to PO
void CIRCUIT::MarkOutputGate()
{
    for (unsigned i = 0;i<POlist.size();++i) {
        POlist[i]->SetFlag(OUTPUT);
        POlist[i]->Fanin(0)->SetFlag(OUTPUT);
    }
    return;
}
