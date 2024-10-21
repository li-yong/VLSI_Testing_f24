#include <iostream>
// #include <alg.h>
#include "circuit.h"
#include "GetLongOpt.h"
#include <algorithm> // For sort and unique
#include <vector>    // For vector
#include <set>
#include <bitset>
#include <random>
#include <map>

using namespace std;

extern GetLongOpt option;

void CIRCUIT::FanoutList()
{
    int i = 0, j;
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
        cout << gptr->Get_isc_identifier() << " set to LV " << lel << endl;
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

// recuive. from levl0->level1->levenn
void CIRCUIT::Levelize()

{

    int max_gate_level = 0;

    // init gate level to -1
    for (unsigned i = 0; i < No_Gate(); i++)
    {
        GATE *g;
        g = Gate(i);
        if (g->GetFunction() == G_PI)
        {
            g->SetLevel(0);
        }
        else
        {
            g->SetLevel(-1);
        }
    }

    // iternate netlist, print the level of the each gate
    for (unsigned i = 0; i < No_Gate(); i++)
    {
        GATE *g;
        g = Gate(i);
        string name = g->Get_isc_identifier();
        GATEFUNC fun = g->GetFunction();
        int level = g->GetLevel();
        // cout << "Gate: " << name << " Level: " << level << endl;

        vector<GATE *> inputlist = g->GetInput_list();
        vector<GATE *> outputlist = g->GetOutput_list();

        // remove duplicate entries from outputlist
        sort(outputlist.begin(), outputlist.end());
        auto it = unique(outputlist.begin(), outputlist.end());
        outputlist.erase(it, outputlist.end());

        // vector<GATE *> inputfanlist = g->GetInput_fan_list();
        // vector<GATE *> outputlfanist = g->GetOutput_fan_list();

        // check output gate level
        for (size_t i2 = 0; i2 < outputlist.size(); ++i2)
        {
            if (outputlist[i2]->GetFunction() == G_FROM | outputlist[i2]->GetFunction() == G_PO)
            //  if (outputlist[i2]->GetFunction() == G_FROM )
            {
                continue;
            }

            // cout << name << " output is " << outputlist[i2]->GetName() << endl;
            // cout << "output gate level is " << outputlist[i2]->GetLevel() << endl;

            if (outputlist[i2]->GetLevel() < level + 1)
            {
                if (level == 4)
                {
                    cout << "debug" << endl;
                }
                outputlist[i2]->SetLevel(level + 1);
                // cout << "set " << outputlist[i2]->Get_isc_identifier() << " output level to " << outputlist[i2]->GetLevel() << endl;
                max_gate_level = max(max_gate_level, outputlist[i2]->GetLevel());
            }
        }

        // // check input gate level
        // for (size_t i1 = 0; i1 < inputlist.size(); ++i1)
        // {
        //     cout << name << " input is " << inputlist[i1]->GetName() << endl;
        // }
    }
    // cout << "max level is " << max_gate_level << endl;
    SetMaxLevel();
}

vector<GATE *> CIRCUIT::GetGateInLevel(int cur_lv)
{
    // int cur_lv = 0;
    vector<GATE *> gates_in_lv;

    for (long int i = 0; i < Netlist.size(); i++)
    {
        string name = Netlist[i]->Get_isc_identifier();
        int lvl = Netlist[i]->GetLevel();

        if (lvl == cur_lv)
        {
            // cout << name << " gate_in_level " << lvl << endl;
            gates_in_lv.push_back(Netlist[i]);
        }
    }

    return gates_in_lv;
}

// recuive. from levl0->end, levl0->end. hung.
void CIRCUIT::Levelize_1()
{
    list<GATE *> Queue;
    GATE *gptr;
    // GATE *out;
    // int j = 0;

    for (int i = 0; i < No_PI(); i++)
    {
        gptr = PIGate(i);
        // cout << gptr->Get_isc_identifier() << " set to level 0" << endl;

        Levelize_recu(gptr, 0);
    }
    cout << " all gates level set completed." << endl;
}

// original
void CIRCUIT::Levelize_0()
{
    list<GATE *> Queue;
    GATE *gptr;
    GATE *out;
    int j = 0;
    for (int i = 0; i < No_PI(); i++)
    {
        gptr = PIGate(i);
        gptr->SetLevel(0);
        for (j = 0; j < gptr->No_Fanout(); j++)
        {
            out = gptr->Fanout(j);
            if (out->GetFunction() != G_PPI)
            {
                out->IncCount();
                if (out->GetCount() == out->No_Fanin())
                {
                    out->SetLevel(1);
                    Queue.push_back(out);
                }
            }
        }
    }
    for (int i = 0; i < No_PPI(); i++)
    {
        gptr = PPIGate(i);
        gptr->SetLevel(0);
        for (j = 0; j < gptr->No_Fanout(); j++)
        {
            out = gptr->Fanout(j);
            if (out->GetFunction() != G_PPI)
            {
                out->IncCount();
                if (out->GetCount() ==
                    out->No_Fanin())
                {
                    out->SetLevel(1);
                    Queue.push_back(out);
                }
            }
        }
    }
    int l1, l2;
    while (!Queue.empty())
    {
        gptr = Queue.front();
        Queue.pop_front();
        l2 = gptr->GetLevel();
        for (j = 0; j < gptr->No_Fanout(); j++)
        {
            out = gptr->Fanout(j);
            if (out->GetFunction() != G_PPI)
            {
                l1 = out->GetLevel();
                if (l1 <= l2)
                    out->SetLevel(l2 + 1);
                out->IncCount();
                if (out->GetCount() ==
                    out->No_Fanin())
                {
                    Queue.push_back(out);
                }
            }
        }
    }
    for (int i = 0; i < No_Gate(); i++)
    {
        Gate(i)->ResetCount();
    }
}

void CIRCUIT::Check_Levelization()
{

    GATE *gptr;
    GATE *in;
    int i, j;
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
    int i = 0;
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
    const char *GATEFUNC_Table[12] = {"G_PI", "G_PO", "G_PPI", "G_PPO", "G_NOT",
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
    int no_gate_fanin, i;
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
    int no_gate_fanout, i;
    vector<GATE *>::iterator it_net;

    for (it_net = Netlist.begin(); it_net != Netlist.end(); ++it_net)
    {
        cout << "Gate: " << (*it_net)->GetName() << endl;
        no_gate_fanout = (*it_net)->No_Fanout();
        for (i = 0; i < no_gate_fanout; ++i)
            cout << "  " << (*it_net)->Fanout(i)->GetName() << endl;
    }
}

void CIRCUIT::printGateIdTypeOutput()
{
    vector<GATE *>::iterator it_net;

    for (it_net = Netlist.begin(); it_net != Netlist.end(); ++it_net)
    {
        // cout << "Gate: " << (*it_net)->GetName() << endl;
        GATEFUNC function = (*it_net)->GetFunction();
        string function_s;

        string isc_id = (*it_net)->Get_isc_identifier();
        int netid = (*it_net)->Get_isc_net_id();
        vector<GATE *> outputlist = (*it_net)->GetOutput_list();

        // iterate the outputlist, get the net_id by outputlist[n]->Get_isc_net_id() then join them with space.
        string po;
        // iterate the outputlist

        if (function == G_FROM)
        {
            continue;
        }

        if (function == G_PI)
        {
            function_s = "PI";
        }
        else if (function == G_PO)
        {
            function_s = "PO";
        }
        else if (function == G_PPI)
        {
            function_s = "PPI";
        }

        else if (function == G_PPO)
        {
            function_s = "PPO";
        }

        else if (function == G_NOT)
        {
            function_s = "NOT";
        }

        else if (function == G_AND)
        {
            function_s = "AND";
        }

        else if (function == G_NAND)
        {
            function_s = "NAND";
        }
        else if (function == G_OR)
        {
            function_s = "OR";
        }
        else if (function == G_XOR)
        {
            function_s = "XOR";
        }
        else if (function == G_BUF)
        {
            function_s = "BUF";
        }
        else if (function == G_NOR)
        {
            function_s = "BAD";
        }
        else if (function == G_BAD)
        {
            function_s = "BAD";
        }
        else if (function == G_FROM)
        {
            function_s = "FROM";
        }
        else if (function == G_DFF)
        {
            function_s = "DFF";
        }

        for (size_t n = outputlist.size(); n > 0; --n)
        {
            // cout << outputlist[n]->Get_isc_net_id() << endl;
            po += " " + to_string(outputlist[n - 1]->Get_isc_net_id());
        }
        cout << netid << "\t" << function_s << "\t" << po << endl;
    }
}

void CIRCUIT::printSA()
{

    vector<GATE *>::iterator it_net;

    set<GATEFUNC> regular_gate_set = {G_NOT, G_AND, G_NAND, G_OR, G_NOR, G_BUF};

    for (it_net = Netlist.begin(); it_net != Netlist.end(); ++it_net)
    {
        vector<string> input_gate = (*it_net)->Get_isc_input_gates();
        vector<GATE *> inputlist = (*it_net)->GetInput_list();
        vector<GATE *> input_fan_list = (*it_net)->GetInput_fan_list();
        vector<GATE *> output_fan_list = (*it_net)->GetOutput_fan_list();

        vector<GATE *> outputlist = (*it_net)->GetOutput_list();
        vector<string> SAlist = (*it_net)->Get_isc_StuckAt();
        GATEFUNC function = (*it_net)->GetFunction();
        int netid = (*it_net)->Get_isc_net_id();

        // if (netid == 3)
        // {
        //     cout << "debug" << endl;
        // }

        if (function == G_FROM)
        {
            continue;
        }

        // int fi_cnt = (*it_net)->Get_isc_fi_cnt();
        // int fo_cnt = (*it_net)->Get_isc_fo_cnt();
        string net_id = to_string((*it_net)->Get_isc_net_id());
        vector<string> input_gate_id_list;
        vector<string> output_gate_id_list;

        // no fan from since here.
        for (size_t n1 = 0; n1 < SAlist.size(); ++n1)
        {
            cout << net_id << "\t" << 0 << "\t" << SAlist[n1].substr(3) << endl;
        } // iterate the SAlist

        // init input_gate_id_list to empty list
        input_gate_id_list = {};

        // // take care the inputfan from gate. If a input gate is fan from gate.
        // for (size_t n2 = 0; n2 < input_fan_list.size(); ++n2)
        // {
        //     int n = input_fan_list[n2]->Get_isc_net_id();             // 8 fanfrom
        //     vector<string> f = input_fan_list[n2]->Get_isc_StuckAt(); // sa1. 8gateflt
        //     vector<GATE *> g = input_fan_list[n2]->GetInput_list();   // 8gateinpt, which is 3gat

        //     for (size_t n3 = 0; n3 < g.size(); ++n3)
        //     {
        //         int gn = g[n3]->Get_isc_net_id(); // 3. got 3 gate

        //         for (size_t n4 = 0; n4 < f.size(); ++n4)
        //         {
        //             // cout << net_id << "\t" << gn << "\t" << f[n4].substr(3) << endl; //yong debug
        //         }
        //     }
        // }

        // take care the output fanfrom gate. If a output gate is fan from gate.
        for (size_t n5 = 0; n5 < output_fan_list.size(); ++n5)
        {

            int a = output_fan_list[n5]->GetOutput_list()[0]->Get_isc_net_id(); // 10, 10gat
            int b = netid;                                                      // 3, 3fan
            if (output_fan_list[n5]->Get_isc_StuckAt().size() > 0)
            {
                string c = output_fan_list[n5]->Get_isc_StuckAt()[0].substr(3);
                cout << a << "\t" << b << "\t" << c << endl;
            }
        }
    }
}

void CIRCUIT::init_bitset(bool v1, bool v2, bool oe, bool oa)
{
    vector<GATE *>::iterator it_net;
    bitset<64> bitset_all_zero;

    for (it_net = Netlist.begin(); it_net != Netlist.end(); ++it_net)
    {

        if (v1)
        {
            (*it_net)->SetValue1(bitset_all_zero);
        }

        if (v2)
        {
            (*it_net)->SetValue2(bitset_all_zero);
        }

        if (oe)
        {
            (*it_net)->set_isc_bitset_output_expected(bitset_all_zero);
        }

        if (oa)
        {
            (*it_net)->set_isc_bitset_output_actual(bitset_all_zero);
        }
    }
}

void CIRCUIT::print_bitset()
{
    bitset<64> input_gate_value_1, input_gate_value_2, isc_bitset_output_expected, isc_bitset_output_actual;

    vector<GATE *>::iterator it_net;

    for (it_net = Netlist.begin(); it_net != Netlist.end(); ++it_net)
    {
        vector<string> input_gate = (*it_net)->Get_isc_input_gates();
        vector<GATE *> inputlist = (*it_net)->GetInput_list();
        vector<GATE *> input_fan_list = (*it_net)->GetInput_fan_list();
        vector<GATE *> output_fan_list = (*it_net)->GetOutput_fan_list();

        vector<GATE *> outputlist = (*it_net)->GetOutput_list();
        vector<string> SAlist = (*it_net)->Get_isc_StuckAt();
        GATEFUNC function = (*it_net)->GetFunction();
        int netid = (*it_net)->Get_isc_net_id();
        string isc_identifer = (*it_net)->Get_isc_identifier();

        isc_bitset_output_expected = (*it_net)->get_isc_bitset_output_expected();
        isc_bitset_output_actual = (*it_net)->get_isc_bitset_output_actual();

        input_gate_value_1 = (*it_net)->GetValue1();
        input_gate_value_2 = (*it_net)->GetValue2();

        {

            cout << "isc_identifer: " << isc_identifer << endl;
            // for (const auto &bits : )
            // {
            cout << "\t input    value1: " << input_gate_value_1 << endl;
            cout << "\t input    value2: " << input_gate_value_2 << endl;
            cout << "\t expected output: " << isc_bitset_output_expected << endl;
            cout << "\t actual   output: " << isc_bitset_output_actual << endl;
            // }
        }
    }
}

void CIRCUIT::update_fanout_bitset(GATE *gate, string expect_or_actual, bitset<64> bitset)
{
    // get v[i]'s output gate list
    vector<GATE *> outputfanlist = gate->GetOutput_fan_list();
    for (long unsigned j = 0; j < outputfanlist.size(); j++)
    {
        // cout << "output gate: " << outputfanlist[j]->Get_isc_identifier() << endl;
        // get outputlist[j] function
        GATEFUNC fun = outputfanlist[j]->GetFunction();
        if (fun != G_FROM)
        {
            cout << "something wrong. output gate should be G_FROM" << endl;
            exit(0);
        }

        if (expect_or_actual == "EXPECT")
        {
            outputfanlist[j]->set_isc_bitset_output_expected(bitset);
        }
        else if (expect_or_actual == "ACTUAL")
        {
            string bitString = bitset.to_string();
            outputfanlist[j]->set_isc_bitset_output_actual(bitset);
        }
    }
}

// handle level 0 input gates
void CIRCUIT::init_level0_input_gate()
{
    // Initialize random number generator
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<uint64_t> dis(0, UINT64_MAX);

    // get level 0 gates. Input gate only have one pin.
    vector<GATE *> v = GetGateInLevel(0);

    // cout << "get level 0 output gates" << endl;
    for (long unsigned i = 0; i < v.size(); i++)
    {
        // cout << v[i]->Get_isc_identifier() << "  " << v[i]->GetLevel() << endl;
        bitset<64> bitset_random = bitset<64>(dis(gen));
        string bitString = bitset_random.to_string();

        v[i]->set_isc_bitset_output_expected(bitset_random);
        v[i]->set_isc_bitset_output_actual(bitset_random);

        // handle fanout
        update_fanout_bitset(v[i], "EXPECT", bitset_random);
        update_fanout_bitset(v[i], "ACTUAL", bitset_random);
    }
}

// handle level 1->max level gates
void CIRCUIT::calc_output_level_1_max(int gate_level, string expect_or_actual)
{
    vector<GATE *> v = GetGateInLevel(gate_level);
    // cout << "\nget level " << gate_level << " gates" << endl;

    for (long unsigned i = 0; i < v.size(); i++)
    {
        // cout << v[i]->Get_isc_identifier() << ", level " << v[i]->GetLevel() << endl;

        // if (v[i]->Get_isc_identifier() == "11gat")
        // {
        //     cout << "debug" << endl;
        // }

        // if (gate_level == 4)
        // {
        //     cout << "debug" << endl;
        // }

        // get input gates of v[i]
        vector<string> input_gates = v[i]->Get_isc_input_gates();

        bitset<64> input_gate_value_exp_1, input_gate_value_exp_2;
        bitset<64> input_gate_value_act_1, input_gate_value_act_2;

        for (long unsigned j = 0; j < input_gates.size(); j++)
        {
            // cout << "\t input gate: " << input_gates[j] << endl;

            // find gate by net id
            GATE *input_gate = Find_Gate_by_isc_netid(stoi(input_gates[j]));
            string input_gate_name = input_gate->Get_isc_identifier();

            // cout << "\t input gate name: " << input_gate_name << endl;

            // get input gate's value
            if (j == 0)
            {
                input_gate_value_exp_1 = input_gate->get_isc_bitset_output_expected();
                input_gate_value_act_1 = input_gate->get_isc_bitset_output_actual();

                if (expect_or_actual == "ACTUAL" & input_gate_value_exp_1 != input_gate_value_act_1)
                {
                    // cout << "\t Debug, input gate ACTUAL is NOT EXPECTED! Something has wrong before current gate." << endl;
                }

                if (expect_or_actual == "EXPECT")
                {
                    v[i]->SetValue1(input_gate_value_exp_1);
                }
                else if (expect_or_actual == "ACTUAL")
                {

                    v[i]->SetValue1(input_gate_value_act_1);
                }
            }
            else if (j == 1)
            {
                input_gate_value_exp_2 = input_gate->get_isc_bitset_output_expected();
                input_gate_value_act_2 = input_gate->get_isc_bitset_output_actual();

                if (expect_or_actual == "ACTUAL" & input_gate_value_exp_2 != input_gate_value_act_2)
                {
                    // cout << "\t Debug, input gate ACTUAL is NOT EXPECTED! Something has wrong before current gate" << endl;
                }

                if (expect_or_actual == "EXPECT")
                {
                    v[i]->SetValue2(input_gate_value_exp_2);
                }
                else if (expect_or_actual == "ACTUAL")
                {
                    v[i]->SetValue2(input_gate_value_act_2);
                }
            }
            else
            {
                cout << "\t something wrong when parsing gate input" << endl;
                exit(0);
            }

            // cout << "  " << endl;
        }

        //
        bitset<64> result = isc_Evaluate(v[i], v[i]->GetValue1(), v[i]->GetValue2());

        if (expect_or_actual == "EXPECT")
        {
            v[i]->set_isc_bitset_output_expected(result);
        }
        else if (expect_or_actual == "ACTUAL")
        {
            v[i]->set_isc_bitset_output_actual(result);
        }

        update_fanout_bitset(v[i], expect_or_actual, result);

        // cout << "  " << endl;
    }
}

////iterate the gates in circuits
void CIRCUIT::iterate_gates_sa_errors()
{
    vector<GATE *> netlist = GetNetlist();

    for (unsigned i = 0; i < netlist.size(); i++)
    {
        GATE *g;
        g = netlist[i];
        string name = g->Get_isc_identifier();
        GATEFUNC fun = g->GetFunction();

        bitset<64> bitset_all_zero;
        bitset<64> bitset_all_one;
        bitset_all_one.set();

        // iterate the SAlist
        vector<string> SAlist = g->Get_isc_StuckAt();

        for (size_t n = 0; n < SAlist.size(); ++n)
        {
            cout << name << " " << SAlist[n] << endl;

            if (SAlist[n] == ">sa0")
            {
                cout << "injecting SA0 on " << name << endl;
                g->set_isc_bitset_output_actual(bitset_all_zero);
            }
            else if (SAlist[n] == ">sa1")
            {
                cout << "injecting SA1 on " << name << endl;
                g->set_isc_bitset_output_actual(bitset_all_one);
            }

            // evaluate actual output for each gate
            for (int gate_level = 1; gate_level <= GetMaxLevel(); gate_level++)
            {
                calc_output_level_1_max(gate_level, "ACTUAL");
            }

            // print_bitset();

            // Iterate the PO GATES

            // iterate the gates in circuits
            for (unsigned i = 0; i < netlist.size(); i++)
            {
                GATE *g;
                g = netlist[i];
                int foc = g->Get_isc_fo_cnt();
                if (foc > 0 | g->GetFunction() == G_FROM)
                {
                    continue;
                }

                cout << "\nPO: " << g->Get_isc_identifier() << endl;
                cout << "expected output: " << g->get_isc_bitset_output_expected() << endl;
                cout << "actual   output: " << g->get_isc_bitset_output_actual() << endl;

                if (g->get_isc_bitset_output_expected() != g->get_isc_bitset_output_actual() & g->Get_isc_fo_cnt() == 0)
                {
                    cout << "PO: " << g->Get_isc_identifier() << " is failing" << endl;

                    vector<string> salist = g->Get_isc_StuckAt();

                    salist.erase(remove(salist.begin(), salist.end(), SAlist[n]), salist.end());
                    g->Set_isc_StuckAt(salist);

                    cout << "stuck error detected on gate " << g->Get_isc_identifier() << ", removed the " << SAlist[n] << " from the SAlist" << endl;

                    vector<int> differing_positions;
                    for (int i = 0; i < 64; i++)
                    {
                        if (g->get_isc_bitset_output_expected()[i] != g->get_isc_bitset_output_actual()[i])
                        {
                            differing_positions.push_back(i);
                            break; // only need to find one position. YONG DEBUG
                        }
                    }

                    gather_input_output_pattern_and_show_ptn_at_diff_postion(differing_positions);
                }
            }
        }
    }
}

void CIRCUIT::gather_input_output_pattern_and_show_ptn_at_diff_postion(vector<int> differing_positions)
{

    // Assuming dict_bitset is defined as follows:
    map<string, bitset<64>> dict_str_bs;
    // key: gate_isc_identifier
    map<string, map<string, bitset<64>>> dict_gate;

    // iterate gates in circuits
    vector<GATE *> netlist = GetNetlist();
    for (unsigned i = 0; i < netlist.size(); i++)
    {
        GATE *g;
        g = netlist[i];
        string isc_identifier = g->Get_isc_identifier();
        GATEFUNC fun = g->GetFunction();

        bitset<64> bitset_exp, bitset_act;

        bitset_exp = g->get_isc_bitset_output_expected();
        bitset_act = g->get_isc_bitset_output_actual();

        string bitString_exp = bitset_exp.to_string();
        string bitString_act = bitset_act.to_string();
        cout << bitString_act << endl;

        dict_gate[isc_identifier]["bs_op_exp"] = bitset_exp;
        dict_gate[isc_identifier]["bs_op_act"] = bitset_act;
        dict_gate[isc_identifier]["bs_value1"] = g->GetValue1();
        dict_gate[isc_identifier]["bs_value2"] = g->GetValue2();

        // cout << "debug, parsed gate bitsets (op_exp, op_act, v1, v2). " << isc_identifier<< endl;
    }

    // print input patterns
    show_ptn_header(dict_gate, true, false,false);
    show_diff_pattern(dict_gate, differing_positions, true, false, false);

    // print output patterns expected
    show_ptn_header(dict_gate, false, true,false);
    show_diff_pattern(dict_gate, differing_positions, false, true,false);

    // print output patterns actual
    show_ptn_header(dict_gate, false, false, true);
    show_diff_pattern(dict_gate, differing_positions, false, false,true);

    cout << "debug" << endl;
}

void CIRCUIT::show_ptn_header(map<string, map<string, bitset<64>>> dict_gate, bool b_ipt, bool b_opt_exp, bool b_opt_act)
{
    string h = "";
    if (b_ipt)
    {
        h = "Input Pattern:\n";
    }
    else if (b_opt_exp)
    {
        h = "Output Pattern Expected:\n";
    }
    else if (b_opt_act)
    {
        h = "Output Pattern Actual:\n";
    }

    // iternate the dict_gate
    for (auto const &gate : dict_gate)
    {

        GATE *g = Find_Gate_by_isc_identifier(gate.first);
        string gate_isc_identifier = g->Get_isc_identifier();
        string gate_net_id = to_string(g->Get_isc_net_id());

        if (g->Get_isc_fi_cnt() == 0 & b_ipt)
        {
            h += gate_net_id + "_" + gate_isc_identifier + ",";
        }

        if (g->Get_isc_fo_cnt() == 0 & b_opt_exp)
        {
            h += gate_net_id + "_" + gate_isc_identifier + ",";
        }
        if (g->Get_isc_fo_cnt() == 0 & b_opt_act)
        {
            h += gate_net_id + "_" + gate_isc_identifier + ",";
        }
    }
    cout << h << endl;
}

void CIRCUIT::show_diff_pattern(map<string, map<string, bitset<64>>> dict_gate, vector<int> differing_positions, bool b_ipt, bool b_opt_exp, bool b_opt_act)
{


    string h = "";
    if (b_ipt)
    {
        h = "";
    }
    else if (b_opt_exp)
    {
        h = "";
    }
    else if (b_opt_act)
    {
        h = "";
    }


    // cout << "Bits differ at the following positions:" << endl;
    for (int pos : differing_positions)
    {
        // cout << pos << " ";

        // iternate the dict_gate
        for (auto const &gate : dict_gate)
        {

            bitset<64> bs_op_exp = gate.second.at("bs_op_exp");
            bitset<64> bs_op_act = gate.second.at("bs_op_act");
            bitset<64> bs_value1 = gate.second.at("bs_value1");
            bitset<64> bs_value2 = gate.second.at("bs_value2");

            GATE *g = Find_Gate_by_isc_identifier(gate.first);
            string gate_isc_identifier = g->Get_isc_identifier();
            string gate_net_id = to_string(g->Get_isc_net_id());
            GATEFUNC gate_function = g->GetFunction();

            if (g->Get_isc_fi_cnt() == 0 & gate_function == G_PI & b_ipt)
            {

                cout << gate_net_id << "_" << gate_isc_identifier << "_" << bs_value1[pos] << ",";
            }

            if (g->Get_isc_fo_cnt() == 0 & b_opt_exp)
            {
                cout << gate_net_id << "_" << gate_isc_identifier << "_" << bs_op_exp[pos] << ",";
            }
                
            if (g->Get_isc_fo_cnt() == 0 & b_opt_act)
            {
                cout << gate_net_id << "_" << gate_isc_identifier << "_" << bs_op_act[pos] << ",";
            }


            // cout << gate_name << " " << bitset_exp[pos] << " " << bitset_act[pos] << " " << bitset_v1[pos] << " " << bitset_v2[pos] << endl;
        }
    }
    cout << endl;
}