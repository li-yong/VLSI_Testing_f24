#include <iostream>
// #include <alg.h>
#include "circuit.h"
#include "GetLongOpt.h"
using namespace std;

extern GetLongOpt option;

void CIRCUIT::FanoutList()
{
    unsigned i = 0, j;
    GATE *gptr;
    for (; i < No_Gate(); i++)
    {
        gptr = Gate(i);
        for (j = 0; j < gptr->No_Fanin(); j++)
        {
            gptr->Fanin(j)->AddOutput_list(gptr);
        }
    }
}

void CIRCUIT::Levelize_recu(GATE *gptr, int lel = 0)
{

    string isc_id = gptr->Get_isc_identifier();

    if (lel > gptr->GetLevel())
    {
        gptr->SetLevel(lel);
        // cout << gptr->Get_isc_identifier() << " set to LV " << lel << endl;
    }

    for (int j = 0; j < gptr->No_Fanout(); j++)
    {
        GATE *out = gptr->Fanout(j);

        if (out->GetFunction() == G_FROM)
        {
            continue;
        }

        Levelize_recu(out, lel + 1);
    }
}

void CIRCUIT::Levelize()
{
    list<GATE *> Queue;
    GATE *gptr;
    GATE *out;
    unsigned j = 0;

    for (unsigned i = 0; i < No_PI(); i++)
    {
        gptr = PIGate(i);
        // cout << gptr->Get_isc_identifier() << " set to level 0" << endl;

        Levelize_recu(gptr, 0);
    }
}

void CIRCUIT::Check_Levelization()
{

    GATE *gptr;
    GATE *in;
    unsigned i, j;
    for (i = 0; i < No_Gate(); i++)
    {
        gptr = Gate(i);
        if (gptr->GetFunction() == G_PI)
        {
            if (gptr->GetLevel() != 0)
            {
                cout << "Wrong Level for PI : " << gptr->GetName() << endl;
                exit(-1);
            }
        }
        else if (gptr->GetFunction() == G_PPI)
        {
            if (gptr->GetLevel() != 0)
            {
                cout << "Wrong Level for PPI : " << gptr->GetName() << endl;
                exit(-1);
            }
        }
        else if (gptr->GetFunction() == G_FROM)
        {
            ; // do nothing
        }
        else
        {
            for (j = 0; j < gptr->No_Fanin(); j++)
            {
                in = gptr->Fanin(j);
                if (in->GetLevel() >= gptr->GetLevel())
                {
                    cout << "Wrong Level for: " << gptr->GetName() << '\t' << gptr->GetID() << '\t' << gptr->GetLevel() << " with fanin " << in->GetName() << '\t' << in->GetID() << '\t' << in->GetLevel() << endl;
                }
            }
        }
    }
}

void CIRCUIT::SetMaxLevel()
{
    for (unsigned i = 0; i < No_Gate(); i++)
    {
        if (Gate(i)->GetLevel() > MaxLevel)
        {
            MaxLevel = Gate(i)->GetLevel();
        }
    }
}

// Setup the Gate ID and Inversion
// Setup the list of PI PPI PO PPO
void CIRCUIT::SetupIO_ID()
{
    unsigned i = 0;
    GATE *gptr;
    vector<GATE *>::iterator Circuit_ite = Netlist.begin();
    for (; Circuit_ite != Netlist.end(); Circuit_ite++, i++)
    {
        gptr = (*Circuit_ite);
        gptr->SetID(i);
        switch (gptr->GetFunction())
        {
        case G_PI:
            PIlist.push_back(gptr);
            break;
        case G_PO:
            POlist.push_back(gptr);
            break;
        case G_PPI:
            PPIlist.push_back(gptr);
            break;
        case G_PPO:
            PPOlist.push_back(gptr);
            break;
        case G_NOT:
            gptr->SetInversion();
            break;
        case G_NAND:
            gptr->SetInversion();
            break;
        case G_NOR:
            gptr->SetInversion();
            break;
        default:
            break;
        }
    }
}

// VLSI-Testing Lab1
// Print out the netlist stored in the Netlist data structure
void CIRCUIT::printNetlist()
{
    char *GATEFUNC_Table[12] = {"G_PI", "G_PO", "G_PPI", "G_PPO", "G_NOT",
                                "G_AND", "G_NAND", "G_OR", "G_NOR", "G_DFF", "G_BUF", "G_BAD"};
    vector<GATE *>::iterator it_net;

    for (it_net = Netlist.begin(); it_net != Netlist.end(); ++it_net)
    {
        cout << "Gate Nate: " << (*it_net)->GetName();
        cout << ", Gate function: ";
        cout << GATEFUNC_Table[(*it_net)->GetFunction()];
        cout << ";\n";
    }
}

void CIRCUIT::printPOInputList()
{
    unsigned no_gate_fanin, i;
    vector<GATE *>::iterator it_po;

    for (it_po = POlist.begin(); it_po != POlist.end(); ++it_po)
    {
        cout << "PO: " << (*it_po)->GetName() << endl;
        no_gate_fanin = (*it_po)->No_Fanin();
        for (i = 0; i < no_gate_fanin; ++i)
            cout << "  " << (*it_po)->Fanin(i)->GetName() << endl;
    }
}

void CIRCUIT::printGateOutput()
{
    unsigned no_gate_fanout, i;
    vector<GATE *>::iterator it_net;

    for (it_net = Netlist.begin(); it_net != Netlist.end(); ++it_net)
    {
        cout << "Gate: " << (*it_net)->GetName() << endl;
        no_gate_fanout = (*it_net)->No_Fanout();
        for (i = 0; i < no_gate_fanout; ++i)
            cout << "  " << (*it_net)->Fanout(i)->GetName() << endl;
    }
}

// VLSI-Testing lab3
void CIRCUIT::printStatResult()
{
    avg_eval_cnt_pattern = (double)evaluation_count / pattern_num;
    percent_eval_cnt = (double)avg_eval_cnt_pattern / No_Gate() * 100;
    cout << "Pattern count = " << pattern_num << endl;
    cout << "Gate count = " << No_Gate() << endl;
    cout << "Evaluation count = " << evaluation_count << endl;
    cout << "Average evaluations over patterns = " << avg_eval_cnt_pattern << endl;
    cout << "Percentage of average evaluations over gates = "
         << percent_eval_cnt << "%\n";
}
