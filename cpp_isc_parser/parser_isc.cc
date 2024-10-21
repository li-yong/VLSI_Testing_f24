#include "gate.h"
#include "circuit.h"
#include <iostream>
#include <fstream>
#include <cstring>
// #include <sstream>
#include <vector>
#include <sstream>
#include "hash.h"

using namespace std;

CIRCUIT isc_Circuit;
CIRCUIT global_netlist;

unsigned isc_no_pi = 0;
unsigned isc_no_po = 0;
unsigned isc_no_dff = 0;
unsigned isc_no_gate = 0;
string isc_name;

GATE *isc_gptr = NULL;
GATE *isc_inlist = NULL;

Hash<string, GATE *, Str_hash_function> isc_NameTable;

// Function to process lines that have "nand" in the 3rd field
void proc_nand(GATE *isc_gptr, const vector<string> &fields, const string &line2)
{
    // cout << "Processing NAND lines:\n";

    istringstream stream(line2); // Create a string stream
    string nand_input;
    vector<string> inputs{};

    // Extract words separated by spaces and store them in the result vector
    while (stream >> nand_input)
    {
        inputs.push_back(nand_input);
    }

    vector<string> faults;

    if (fields.size() == 5)
    {
        faults = {};
    }
    else if (fields.size() == 6)
    {
        faults = {fields[5]};
    }
    else if (fields.size() == 7)
    {
        faults = {fields[5], fields[6]};
    }
    else
    {
        cerr << "Error to parse nand gate Stuck_At_Fault" << endl;
        exit(EXIT_FAILURE);
    }

    //<net_id> <identifier> <gate_type> <fanout> <fanin> <faults>
    int net_id = stoi(fields[0]);
    isc_name = fields[1]; // gate name, should be unique
    if (!isc_NameTable.is_member(isc_name))
    {
        isc_gptr->SetName(isc_name);

        isc_gptr->Set_isc_net_id(net_id);
        isc_gptr->Set_isc_identifier(isc_name);
        isc_gptr->Set_isc_StuckAt(faults);
        isc_gptr->Set_isc_fo_Cnt(stoi(fields[3]));
        isc_gptr->Set_isc_fi_Cnt(stoi(fields[4]));

        // gptr->SetStuckAt(0,faults[0]=="sa0"?1:0);
        // gptr->SetStuckAt(1,faults[0]=="sa1"?1:0);
        isc_NameTable.insert(isc_name, isc_gptr);
        isc_Circuit.AddGate(isc_gptr);
    }
    else
    {
        isc_gptr = isc_NameTable.get_value(isc_name);
        isc_gptr->SetFunction(G_NAND);
    }

    // GATE* t = isc_Circuit.Find_Gate_by_name("1gat");
    isc_gptr->Set_isc_input_gates(inputs);
}

// Function to process lines that have "input" in the 3rd field
void proc_input(GATE *isc_gptr, const vector<string> &fields)
{
    vector<string> inputs{};
    vector<string> faults;

    if (fields.size() == 5)
    {
        faults = {};
    }
    else if (fields.size() == 6)
    {
        faults = {fields[5]};
    }
    else if (fields.size() == 7)
    {
        faults = {fields[5], fields[6]};
    }
    else
    {
        cerr << "Error to parse input gate Stuck_At_Fault" << endl;
        exit(EXIT_FAILURE);
    }

    //<net_id> <identifier> <gate_type> <fanout> <fanin> <faults>
    int net_id = stoi(fields[0]);
    isc_name = fields[1]; // gate name, should be unique

    // if (net_id == 1459){
    //     cout << net_id << endl;
    //     cout << "find gate 1459" << endl; //debug
    // }

    if (!isc_NameTable.is_member(isc_name))
    {
        isc_gptr->SetName(isc_name);
        isc_gptr->Set_isc_net_id(net_id);
        isc_gptr->Set_isc_identifier(isc_name);
        isc_gptr->Set_isc_StuckAt(faults);
        isc_gptr->Set_isc_fo_Cnt(stoi(fields[3]));
        isc_gptr->Set_isc_fi_Cnt(stoi(fields[4])); // FI of G_PI should be 0

        isc_NameTable.insert(isc_name, isc_gptr);
        isc_Circuit.AddGate(isc_gptr);
    }
    else
    {
        isc_gptr = isc_NameTable.get_value(isc_name);
    }
}

// Function to process lines that have "from" in the 3rd field
void proc_fan_from(GATE *isc_gptr, const vector<string> &fields)
{
    // vector<string> inputs ={fields[3]};
    vector<string> faults;

    if (fields.size() == 4)
    {
        faults = {};
    }
    else if (fields.size() == 5)
    {
        faults = {fields[4]};
    }
    else if (fields.size() == 6)
    {
        faults = {fields[4], fields[5]};
    }
    else
    {
        cerr << "Error to parse Fan From Stuck_At_Fault" << endl;
        exit(EXIT_FAILURE);
    }

    //<net_id> <identifier> <gate_type> <fanout> <fanin> <faults>
    int net_id = stoi(fields[0]);
    isc_name = fields[1]; // gate name, should be unique
    if (!isc_NameTable.is_member(isc_name))
    {
        isc_gptr->SetName(isc_name);
        isc_gptr->SetFunction(G_FROM);
        isc_gptr->Set_isc_net_id(net_id);
        isc_gptr->Set_isc_identifier(isc_name);
        isc_gptr->Set_isc_StuckAt(faults);
        isc_gptr->Set_isc_fo_Cnt(-1); // No Fan_IN, Fan_out for G_FROM.
        isc_gptr->Set_isc_fi_Cnt(-1);

        isc_NameTable.insert(isc_name, isc_gptr);
        isc_Circuit.AddGate(isc_gptr);
    }
    else
    {
        isc_gptr = isc_NameTable.get_value(isc_name);
        isc_gptr->SetFunction(G_FROM);
    }

    vector<string> isc_input_gates = {fields[3]};

    isc_gptr->Set_isc_input_gates(isc_input_gates);
}

// Function to process the file content stored in a string
void processBuffer(const string &buffer)
{
    istringstream stream(buffer); // Create a string stream from the buffer
    string line;

    // Read and process the content line by line
    while (getline(stream, line))
    {
        // cout << "Processing line: " << line << endl;

        // Check if the line starts with '*', if so, skip it
        if (!line.empty() && line[0] == '*')
        {
            // cout << "Skipping line: " << line << endl;
            continue; // Skip the rest of the loop for this line
        }

        // Split the line into fields (assumes space-delimited fields)
        istringstream lineStream(line);
        vector<string> fields;
        string field;
        while (lineStream >> field)
        {
            fields.push_back(field); // Push each field into a vector
        }

        if (fields.size() < 4)
        { // At least 4 fileds.  14    14fan from    11gat
            cout << "Line: " << line << endl;
            cerr << "Not a gate. unexpected source file format, please check the source file." << endl;
            exit(EXIT_FAILURE);
        }

        isc_gptr = new GATE;

        // #FO == 0, set primary output. (not output to any gates.)
        if (fields[3] == "0")
        {
            isc_gptr->SetFunction(G_PO);
        }

        // #FI == 0, set primary input. (no input from any gates.)
        if (fields[4] == "0")
        {
            isc_gptr->SetFunction(G_PI);
        }

        // Check if there are at least 3 fields and if the 3rd field is "nand"
        string func = fields[2];

        if (func == "nand" || func == "and" || func == "or" || func == "xor" || func == "not" || func == "nor" || func == "buff")
        {
            if (func == "nand")
            {
                isc_gptr->SetFunction(G_NAND);
            }
            if (func == "and")
            {
                isc_gptr->SetFunction(G_AND);
            }
            if (func == "or")
            {
                isc_gptr->SetFunction(G_OR);
            }
            if (func == "xor")
            {
                isc_gptr->SetFunction(G_XOR);
            }
            if (func == "not")
            {
                isc_gptr->SetFunction(G_NOT);
            }
            if (func == "nor")
            {
                isc_gptr->SetFunction(G_NOR);
            }
            if (func == "buff")
            {
                isc_gptr->SetFunction(G_BUF);
            }

            // Get the next line from the buffer
            string next_line;
            if (getline(stream, next_line))
            {
                // Pass the current line and next line to proc_nand
                proc_nand(isc_gptr, fields, next_line);
            }
        }

        // process input
        else if (fields[2] == "inpt")
        {
            proc_input(isc_gptr, fields);
        }

        // process input
        else if (fields[2] == "from")
        {
            proc_fan_from(isc_gptr, fields);
        }

        else
        {
            cout << "Line: " << line << endl;
            cerr << "don't understand the line" << endl;
            exit(EXIT_FAILURE);
        }

    } // end of while read the input
} // end of processBuffer

void update_output_from_input(GATE *cur_gate, GATE *input_gate_of_cur)
{

    string name = input_gate_of_cur->Get_isc_identifier();
    if (name == "10gat") // debug
    {
        // cout << 1 << endl;
    }

    if (cur_gate->GetFunction() == G_FROM)
    {
        return; //>>> Not need to update a gate output to FAN FROM.
        // it will be covered at the real output gate.
    }

    if (input_gate_of_cur->GetFunction() == G_FROM)
    {

        string n = input_gate_of_cur->Get_isc_identifier();
        // int isc_net_id = input_gate_of_cur->Get_isc_net_id();

        vector<string> L2_input_list = input_gate_of_cur->Get_isc_input_gates();
        for (size_t i2 = 0; i2 < L2_input_list.size(); ++i2)
        {
            GATE *L2_input_gate = isc_Circuit.Find_Gate_by_isc_identifier(L2_input_list[i2]);
            GATEFUNC fc = L2_input_gate->GetFunction();
            // GATE *x = isc_Circuit.Find_Gate_by_isc_netid(isc_net_id);
            if (fc == G_FROM)
            {
                cout << "something wrong. FanFrom connected FanFrom. " << endl;
                exit(-1);
            }
            // cout << "add gate output, FROM " << L2_input_gate->Get_isc_identifier() << " TO " << cur_gate->Get_isc_identifier() << endl;
            L2_input_gate->AddOutput_list(cur_gate);

            // cout << "add fan gate output, FROM " << L2_input_gate->Get_isc_identifier() << " TO " << input_gate_of_cur->Get_isc_identifier() << endl; // add fan gate output, FROM 3gat TO 8fan
            L2_input_gate->AddOutput_fan_list(input_gate_of_cur); // L2_input_gate: 3gat, input_gate_of_cur: 8fan
        }
    }
    else if (input_gate_of_cur->GetFunction() != G_PO)
    {
        // cout << "add gate output, FROM " << input_gate_of_cur->Get_isc_identifier() << " TO " << cur_gate->Get_isc_identifier() << endl;
        input_gate_of_cur->AddOutput_list(cur_gate);
    }
    // input_n2 = isc_Circuit.Find_Gate_by_isc_netid(stoi(ipt));
}

// update fan from gate to real gate.
void update_fan_from_input()
{

    // cout << "func update_fan_from_input <<<" << endl;

    vector<GATE *> netlist = isc_Circuit.GetNetlist();

    for (unsigned i = 0; i < netlist.size(); i++)
    {
        GATE *g;
        g = netlist[i];
        string name = g->GetName();
        GATEFUNC fun = g->GetFunction();
        vector<string> inputlist = g->Get_isc_input_gates();

        if (fun == G_FROM)
        {

            // cout << "current fan from :" + name + "type " << endl;

            for (size_t i = 0; i < inputlist.size(); ++i)
            {
                string ipt = inputlist[i];

                GATE *input_gate = isc_Circuit.Find_Gate_by_isc_identifier(ipt);
                if (input_gate == nullptr)
                {
                    cerr << " null ptr. A non-existed gate was referered in src. " << endl;
                    exit(-1);
                }

                if (input_gate->GetFunction() == G_FROM)
                {
                    cerr << "something wrong. FanFrm feed to FanFrm " << endl;
                    exit(-1);
                }

                g->AddInput_list(input_gate);
            }
        }
    }
}

// a function to get the max net id in the isc_Circuit
int get_max_netid()
{
    vector<GATE *> netlist = isc_Circuit.GetNetlist();

    int max_net_id = -1;

    for (unsigned i = 0; i < netlist.size(); i++)
    {
        GATE *g;
        g = netlist[i];
        int t = g->GetIscNetId();
        if (t > max_net_id)
        {
            max_net_id = t;
        }
        // cout << "current g:" + name + "   type " << endl;
    }

    // cout << "max net id: " << max_net_id << endl;
    return max_net_id;
}

void trvel_netlist()
{
    // 2nd run to parse input. Becasue in 1st run, input gates may be used **before** reference.
    vector<GATE *> netlist = isc_Circuit.GetNetlist();

    int max_net_id = get_max_netid();

    for (unsigned i = 0; i < netlist.size(); i++)
    {
        GATE *g;
        g = netlist[i];
        string name = g->GetName();
        GATEFUNC fun = g->GetFunction();

        int fo = g->Get_isc_fo_cnt();
        // int fi = g->Get_isc_fi_cnt();

        if (fo == 0)
        {
            // cout << "no downstream gate" << endl;
            // make a po gate
            GATE *po = new GATE();
            po->SetFunction(G_PO);
            // po->Set_isc_identifier("PPO_" + name);
            // po->SetName("PPO_" + name);
            // po->Set_isc_input_gates({g->Get_isc_identifier()});
            // po->Set_isc_fo_Cnt(0);
            // po->Set_isc_fi_Cnt(1);
            // po->AddInput_list(g); 

            // isc_Circuit.AddGate(po);

            max_net_id += 1;
            po->Set_isc_net_id(max_net_id);
            // g->AddOutput_list(po);
        }

        // cout << "current g:" + name + "   type " << endl;

        vector<string> inputlist = g->Get_isc_input_gates();

        if (fun == G_FROM)
        {

            for (size_t i = 0; i < inputlist.size(); ++i)
            {
                string ipt = inputlist[i];

                // GATE *input_gate = isc_Circuit.Find_Gate_by_isc_netid(stoi(ipt));
                GATE *input_gate = isc_Circuit.Find_Gate_by_isc_identifier(ipt);

                // debug
                // if (input_gate->Get_isc_identifier() == "8fan")
                // {
                //     cout << 1 << endl;
                // }

                update_output_from_input(g, input_gate);
            }
        }

        if (fun == G_NAND || fun == G_NOT || fun == G_AND || fun == G_NAND || fun == G_OR || fun == G_XOR || fun == G_NOR || fun == G_DFF || fun == G_BUF || fun == G_PO)
        {

            for (size_t i = 0; i < inputlist.size(); ++i)
            {
                // std::cout << inputlist[i] << std::endl;
                string ipt = inputlist[i];

                GATE *input_gate = isc_Circuit.Find_Gate_by_isc_netid(stoi(ipt));

                // // debug

                // if (name == "10gat")
                // {
                //     cout << 1 << endl;
                // }

                // if (input_gate->Get_isc_identifier() == "10gat")
                // {
                //     cout << 1 << endl;
                // }

                if (input_gate == nullptr)
                {
                    cerr << " null ptr. A non-existed gate was referered in src. " << endl;
                    exit(-1);
                }

                if (input_gate->GetFunction() == G_FROM)
                {
                    GATE *L2_input_gate = isc_Circuit.Find_Gate_by_isc_identifier(input_gate->Get_isc_input_gates()[0]); // suppose FAN From only have **ONE** input defined in the isc.
                    g->AddInput_list(L2_input_gate);
                    g->AddInput_fan_list(input_gate); // add 8fan as input of 10gat
                    input_gate->AddOutput_list(g);    // add 10gat as output of 8fan
                }
                else
                {
                    g->AddInput_list(input_gate);
                }

                // backtrace the input. This gate is the output of the previous level gate.
                // update the next level gate output to this gate.
                update_output_from_input(g, input_gate);
            }
        }

    } // end of gates iteration. All ISC info should has been f
}

CIRCUIT *parse_isc_main(string filename)
{
    CIRCUIT *localC = new CIRCUIT();
    // Open the file
    ifstream inputFile(filename);
    if (!inputFile.is_open())
    {
        cerr << "Error: Could not open file " << filename << endl;
        return localC;
    }

    // Read the file into a string
    string buffer((istreambuf_iterator<char>(inputFile)),
                  istreambuf_iterator<char>());

    inputFile.close(); // Close the file after reading

    // extract the filename from the full path filename. eg /home/isc/isc1.txt -> isc1.txt
    size_t found = filename.find_last_of("/\\");
    string file_name = filename.substr(found + 1);
    isc_Circuit.SetName(file_name);

    // Pass the string buffer to the processing function
    processBuffer(buffer);

    // update fan_from input to real gate.
    update_fan_from_input();

    // fill inputlist
    trvel_netlist();

    // isc_Circuit.Name
    // isc_Circuit.FanoutList();
    // isc_Circuit.SetupIO_ID();
    // isc_Circuit.Levelize();
    // isc_Circuit.Check_Levelization();
    // isc_Circuit.InitializeQueue();

    localC = &isc_Circuit;

    // Add this line to pause the program

    return localC;
}
