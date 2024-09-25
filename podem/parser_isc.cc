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
void proc_nand(const vector<string> &fields, const string &line2)
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

    if (fields.size() == 6)
    {
        faults = {fields[5]};
    }
    else if (fields.size() == 7)
    {
        faults = {fields[5], fields[6]};
    }
    else
    {
        cerr << "Erron to parse nand gate Stuck_At_Fault" << endl;
        exit(EXIT_FAILURE);
    }

    //<net_id> <identifier> <gate_type> <fanout> <fanin> <faults>
    isc_gptr = new GATE;
    int net_id = stoi(fields[0]);
    isc_name = fields[1]; // gate name, should be unique
    if (!isc_NameTable.is_member(isc_name))
    {
        isc_gptr->SetName(isc_name);
        isc_gptr->SetFunction(G_NAND);
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
void proc_input(const vector<string> &fields)
{
    vector<string> inputs{};
    vector<string> faults;

    if (fields.size() == 6)
    {
        faults = {fields[5]};
    }
    else if (fields.size() == 7)
    {
        faults = {fields[5], fields[6]};
    }
    else
    {
        cerr << "Erron to parse input gate Stuck_At_Fault" << endl;
        exit(EXIT_FAILURE);
    }

    //<net_id> <identifier> <gate_type> <fanout> <fanin> <faults>
    isc_gptr = new GATE;
    int net_id = stoi(fields[0]);
    isc_name = fields[1]; // gate name, should be unique
    if (!isc_NameTable.is_member(isc_name))
    {
        isc_gptr->SetName(isc_name);
        isc_gptr->SetFunction(G_PI);
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
        isc_gptr->SetFunction(G_PI);
    }
}

// Function to process lines that have "from" in the 3rd field
void proc_fan_from(const vector<string> &fields)
{
    // vector<string> inputs ={fields[3]};
    vector<string> faults;

    if (fields.size() == 5)
    {
        faults = {fields[4]};
    }
    else
    {
        cerr << "Erron to parse Fan From Stuck_At_Fault" << endl;
        exit(EXIT_FAILURE);
    }

    //<net_id> <identifier> <gate_type> <fanout> <fanin> <faults>
    isc_gptr = new GATE;
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
        // Process each line (e.g., print it or parse it)
        // cout << "Processing line: " << line << endl;
        // cout << "Debug: This is a debug message!" << endl;

        // Add your specific line processing logic here (e.g., tokenizing, searching, etc.)
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

        // Check if there are at least 3 fields and if the 3rd field is "nand"
        if (fields[2] == "nand")
        {
            // Get the next line from the buffer
            string next_line;
            if (getline(stream, next_line))
            {
                // Pass the current line and next line to proc_nand
                proc_nand(fields, next_line);
            }
        }

        // process input
        else if (fields[2] == "inpt")
        {
            proc_input(fields);
        }

        // process input
        else if (fields[2] == "from")
        {
            proc_fan_from(fields);
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
    // if (name == "16gat") //debug
    // {
    //     cout << 1 << endl;
    // }

    if (cur_gate->GetFunction() == G_FROM)
    {
        return; //>>> Not need to update a gate output to FAN FROM.
        // it will be covered at the real output gate.
    }

    if (input_gate_of_cur->GetFunction() == G_FROM)
    {

        string n = input_gate_of_cur->Get_isc_identifier();
        int isc_net_id = input_gate_of_cur->Get_isc_net_id();

        vector<string> L2_input_list = input_gate_of_cur->Get_isc_input_gates();
        for (size_t i2 = 0; i2 < L2_input_list.size(); ++i2)
        {
            GATE *L2_input_gate = isc_Circuit.Find_Gate_by_isc_netid(stoi(L2_input_list[i2]));
            GATEFUNC fc = L2_input_gate->GetFunction();
            // GATE *x = isc_Circuit.Find_Gate_by_isc_netid(isc_net_id);
            if (fc == G_FROM)
            {
                cout << "something wrong. FanFrom connected FanFrom. " << endl;
                exit(-1);
            }
            L2_input_gate->AddOutput_list(cur_gate);
        }
    }
    else if (input_gate_of_cur->GetFunction() != G_PO)
    {
        input_gate_of_cur->AddOutput_list(cur_gate);
    }
    // input_n2 = isc_Circuit.Find_Gate_by_isc_netid(stoi(ipt));
}

// update fan from gate to real gate.
void update_fan_from_input()
{

    // cout << "func update_fan_from_input <<<" << endl;

    vector<GATE *> netlist = isc_Circuit.GetNetlist();

    // cout << typeid(netlist[0]).name() << endl;

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

                GATE *input_gate = isc_Circuit.Find_Gate_by_name(ipt);
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

void trvel_netlist()
{
    // 2nd run to parse input. Becasue in 1st run, input gates may be used **before** reference.
    vector<GATE *> netlist = isc_Circuit.GetNetlist();

    // cout << typeid(netlist[0]).name() << endl;

    for (unsigned i = 0; i < netlist.size(); i++)
    {
        GATE *g;
        g = netlist[i];
        string name = g->GetName();
        GATEFUNC fun = g->GetFunction();
        // cout << "current g:" + name + "   type " << endl;

        vector<string> inputlist = g->Get_isc_input_gates();

        if (fun == G_PI)
        {
            ; // do nothing
        }

        if (fun == G_FROM)
        {

            for (size_t i = 0; i < inputlist.size(); ++i)
            {
                string ipt = inputlist[i];

                GATE *input_gate = isc_Circuit.Find_Gate_by_isc_netid(stoi(ipt));

                // debug
                // if (input_gate->Get_isc_identifier() == "8fan")
                // {
                //     cout << 1 << endl;
                // }

                update_output_from_input(g, input_gate);
            }
        }

        if (fun == G_NAND | fun == G_NOT | fun == G_AND | fun == G_NAND | fun == G_OR | fun == G_NOR | fun == G_DFF | fun == G_BUF | fun == G_PO)
        {

            for (size_t i = 0; i < inputlist.size(); ++i)
            {
                // std::cout << inputlist[i] << std::endl;
                string ipt = inputlist[i];

                GATE *input_gate = isc_Circuit.Find_Gate_by_isc_netid(stoi(ipt));

                // // debug
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
                    GATE *L2_input_gate = isc_Circuit.Find_Gate_by_name(input_gate->Get_isc_input_gates()[0]); // suppose FAN From only have **ONE** input defined in the isc.
                    g->AddInput_list(L2_input_gate);
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
    CIRCUIT *localC;
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

    // Pass the string buffer to the processing function
    processBuffer(buffer);

    // update fan from input to real gate.
    update_fan_from_input();

    // fill inputlist
    trvel_netlist();

    // isc_Circuit.Name
    isc_Circuit.FanoutList();
    isc_Circuit.SetupIO_ID();
    isc_Circuit.Levelize();
    isc_Circuit.Check_Levelization();
    isc_Circuit.InitializeQueue();

    localC = &isc_Circuit;

    // Add this line to pause the program
    cout << "Press Enter to continue..." << endl;
    // cin.get();
    return localC;
}