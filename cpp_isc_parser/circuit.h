#ifndef CIRCUIT_H
#define CIRCUIT_H
#include "fault.h"
#include "tfault.h"
#include "pattern.h"
#include <stdlib.h>
#include <string>
#include <vector>
#include <fstream>
#include <string.h>
#include <bitset>
#include <iostream>
#include <map>

#define xstr(s) ystr(s)
#define ystr(s) #s
#define OUTDIR output
#define SIMDIR simulator

typedef GATE *GATEPTR;

class CIRCUIT
{
private:
	string Name;
	PATTERN Pattern;
	vector<GATE *> Netlist;
	vector<GATE *> PIlist; // store the gate indexes of PI
	vector<GATE *> POlist;
	vector<GATE *> PPIlist;
	vector<GATE *> PPOlist;
	list<FAULT *> Flist;	// collapsing fault list
	list<FAULT *> UFlist;	// undetected fault list
	list<TFAULT *> TFlist;	// collapsing fault list
	list<TFAULT *> UTFlist; // undetected fault list
	int MaxLevel;
	int BackTrackLimit; // backtrack limit for Podem
	typedef list<GATE *> ListofGate;
	typedef list<GATE *>::iterator ListofGateIte;
	ListofGate *Queue;

	ListofGate GateStack;
	ListofGate PropagateTree;
	ListofGateIte QueueIte;
	// VLSI-Testing Lab1, stack for path
	std::vector<GATE *> path_stack;
	int path_count;
	string dest_gate_name;
	string input_name, output_name;

	ofstream ofs; // for printing logicsim output file
	// VLSI-Testing Lab3
	int evaluation_count;
	double avg_eval_cnt_pattern;
	double percent_eval_cnt;
	int pattern_num;
	ofstream ofsHeader, ofsMain, ofsEva, ofsPrintIO;
	// VLSI-Testing Lab4
	list<FAULT *> CPFlist;	// collapsing fault list
	list<FAULT *> UCPFlist; // undetected fault list

public:
	// Initialize netlist
	CIRCUIT() : MaxLevel(0), BackTrackLimit(10000)
	{
		Netlist.reserve(32768);
		PIlist.reserve(128);
		POlist.reserve(512);
		PPIlist.reserve(2048);
		PPOlist.reserve(2048);
		path_stack.clear();
		path_count = 0;
		evaluation_count = 0;
		pattern_num = 0;
	}
	CIRCUIT(int NO_GATE, int NO_PI = 128, int NO_PO = 512,
			int NO_PPI = 2048, int NO_PPO = 2048)
	{
		Netlist.reserve(NO_GATE);
		PIlist.reserve(NO_PI);
		POlist.reserve(NO_PO);
		PPIlist.reserve(NO_PPI);
		PPOlist.reserve(NO_PPO);
		path_stack.clear();
		path_count = 0;
		evaluation_count = 0;
		pattern_num = 0;
	}
	~CIRCUIT()
	{
		for (long unsigned i = 0; i < Netlist.size(); ++i)
		{
			delete Netlist[i];
		}
		list<FAULT *>::iterator fite;
		for (fite = Flist.begin(); fite != Flist.end(); ++fite)
		{
			delete *fite;
		}
		if (ofs.is_open())
			ofs.close();
		if (ofsHeader.is_open())
			ofsHeader.close();
		if (ofsMain.is_open())
			ofsMain.close();
		if (ofsEva.is_open())
			ofsEva.close();
		if (ofsPrintIO.is_open())
			ofsPrintIO.close();
	}

	void AddGate(GATE *gptr) { Netlist.push_back(gptr); }
	void SetName(string n) { Name = n; }
	string GetName() { return Name; }

	vector<GATE *> GetNetlist() { return Netlist; }
	void SetNetlist(vector<GATE *> nlst) { Netlist = nlst; }

	int GetMaxLevel() { return MaxLevel; }
	void SetBackTrackLimit(int bt) { BackTrackLimit = bt; }
	// GATE* GetNetlist()( return Netlist; ) //yong
	// Access the gates by indexes
	GATE *Gate(int index) { return Netlist[index]; }
	GATE *PIGate(int index) { return PIlist[index]; }
	GATE *POGate(int index) { return POlist[index]; }
	GATE *PPIGate(int index) { return PPIlist[index]; }
	GATE *PPOGate(int index) { return PPOlist[index]; }

	// GATE* Find_Gate_by_name(string name){
	// 	GATE* gptr;

	// 	for (int i = 0;i < No_Gate();i++) {
	// 		gptr = Gate(i);
	// 		if (gptr->GetName() == name) {
	// 			return gptr;
	// 		}
	// 	}
	// 	//print no gate found
	// 	std::cerr << "No gate found with name: " << name << std::endl;
	// 	return nullptr;

	// }

	GATE *Find_Gate_by_isc_identifier(string isc_identifier)
	{
		GATE *gptr;

		for (int i = 0; i < No_Gate(); i++)
		{
			gptr = Gate(i);
			if (gptr->Get_isc_identifier() == isc_identifier)
			{
				return gptr;
			}
		}
		// print no gate found
		std::cerr << "No gate found with isc_identifier: " << isc_identifier << std::endl;
		return nullptr;
	}

	GATE *Find_Gate_by_isc_netid(int isc_netid)
	{
		GATE *gptr;

		for (int i = 0; i < No_Gate(); i++)
		{
			gptr = Gate(i);

			// if (gptr->GetIscNetId() ==242){
			// 	cout << "find gate 242" << endl; //debug
			// }

			if (gptr->GetIscNetId() == isc_netid)
			{
				return gptr;
			}
		}
		// print no gate found
		std::cerr << "No gate found with netid: " << std::to_string(isc_netid) << std::endl;
		return nullptr;
	}

	int No_Gate() { return Netlist.size(); }
	int No_PI() { return PIlist.size(); }
	int No_PO() { return POlist.size(); }
	int No_PPI() { return PPIlist.size(); }
	int No_PPO() { return PPOlist.size(); }

	void copyPItoPattern()
	{
		vector<GATE *>::iterator it;
		for (it = PIlist.begin(); it != PIlist.end(); it++)
		{
			Pattern.addInList(*it);
		}
	}
	void genRandomPattern(string pattern_name, int number)
	{
		Pattern.setPatternName(pattern_name);
		copyPItoPattern();
		Pattern.genRandomPattern(number);
		Pattern.setPatterninput();
	}
	void genRandomPatternUnknown(string pattern_name, int number)
	{
		Pattern.setPatternName(pattern_name);
		copyPItoPattern();
		Pattern.genRandomPatternUnknown(number);
		Pattern.setPatterninput();
	}

	void InitPattern(const char *pattern)
	{
		Pattern.Initialize(const_cast<char *>(pattern), PIlist.size(), "PI");
	}

	void openFile(string file_name)
	{
		cout << "in function openFile" << endl;
		ofs.open(file_name.c_str(), ofstream::out | ofstream::app);
		if (!ofs.is_open())
			cout << "Cannot open file: " << file_name << endl;
		else
			cout << "Successfully open openFile: " << file_name << endl;
	}

	void openOutputFile(string file_name)
	{
		char str[] = "mkdir ";
		strcat(str, xstr(OUTDIR));
		system(str);

		strcpy(str, "./");
		strcat(str, xstr(OUTDIR));
		strcat(str, "/");
		strcat(str, file_name.c_str());
		ofs.open(str, ofstream::out | ofstream::trunc);
		if (!ofs.is_open())
			cout << "Cannot open file!\n";
		else
			cout << "Successfully open openOutputFile: " << file_name << endl;
	}

	void openSimulatorFile(string file_name)
	{
		char str[] = "mkdir ";
		strcat(str, xstr(SIMDIR));
		system(str);

		strcpy(str, "./");
		strcat(str, xstr(SIMDIR));
		strcat(str, "/");
		strcat(str, file_name.c_str());
		ofs.open(str, ofstream::out | ofstream::trunc);
		ofsHeader.open("./simulator/header", ofstream::out | ofstream::trunc);
		ofsMain.open("./simulator/main", ofstream::out | ofstream::trunc);
		ofsEva.open("./simulator/evaluate", ofstream::out | ofstream::trunc);
		ofsPrintIO.open("./simulator/printIO", ofstream::out | ofstream::trunc);
		if (!ofs.is_open())
			cout << "Cannot open output file!\n";
		if (!ofsHeader.is_open())
			cout << "Cannot open header!\n";
		if (!ofsMain.is_open())
			cout << "Cannot open main!\n";
		if (!ofsEva.is_open())
			cout << "Cannot open evaluate!\n";
		if (!ofsPrintIO.is_open())
			cout << "Cannot open printIO!\n";
	}

	void Schedule(GATE *gptr)
	{
		if (!gptr->GetFlag(SCHEDULED))
		{
			gptr->SetFlag(SCHEDULED);
			cout << Queue << endl;
			cout << gptr << endl;
			Queue[gptr->GetLevel()].push_back(gptr);
		}
	}

	// VLSI-Testing Lab1
	// defined in path.cc
	void path(string src_name_gate, string dest_gate_name);
	bool findPath();
	void printPath();

	// VLST-Testing Lab6
	void AtpgRandomPattern();
	void GenerateAllCPFaultListForFsim();
	int FaultSimRandomPattern();
	void closeOfs()
	{
		if (ofs.is_open())
			ofs.close();
		Pattern.closeOfs();
	}
	void genRandomPatternOnly(string pattern_name, int number)
	{
		copyPItoPattern();
		Pattern.genRandomPatternOnly(number);
		Pattern.setPatterninput();
	}

	// defined in circuit.cc

	vector<GATE *> GetGateInLevel(int);

	void Levelize_0();
	void Levelize_1();

	void Levelize();
	void Levelize_recu(GATE *, int);
	void FanoutList();
	void Check_Levelization();
	void SetMaxLevel();
	void SetupIO_ID();
	// print useful infomation
	void printNetlist();
	void printPOInputList();
	void printGateOutput();
	void printGateIdTypeOutput();
	void printSA();
	void calc_output_level_1_max(int gate_level, string expect_or_actual);
	void SetPPIZero(); // Initialize PPI state

	void InitializeQueue();
	void ScheduleFanout(GATE *);
	void SchedulePI();
	void SchedulePPI();
	void LogicSimVectors();
	void LogicSim();
	void ModLogicSimVectors();
	void ModLogicSim();
	void PrintIO();
	void PrintModIO();
	VALUE Evaluate(GATEPTR gptr);
	bitset<64> isc_Evaluate(GATEPTR gptr);
	bitset<2> ModEvaluate(GATEPTR gptr);
	void print_bitset();
	void init_level0_input_gate();
	void update_fanout_bitset(GATE *gate, string, bitset<64> bitset);
	void iterate_gates_sa_errors();
	void init_bitset(bool v12, bool oe, bool oa);
	void gather_input_output_pattern_and_show_ptn_at_diff_postion(vector<int> differing_positions, string err_out_gate_isc_identifier);
	void show_diff_pattern(std::map<string, map<string, bitset<64>>> dict_gate, vector<int> differing_positions, string err_out_gate_isc_identifier, bool b_ipt, bool b_opt_exp, bool b_opt_act);
	void show_ptn_header(map<string, map<string, bitset<64>>> dict_gate, bool b_ipt, bool, bool);
	// defined in atpg.cc
	void GenerateAllFaultList();
	void GenerateCheckPointFaultList();
	void GenerateFaultList();
	void Atpg();
	void SortFaninByLevel();
	bool CheckTest();
	bool TraceUnknownPath(GATEPTR gptr);
	bool FaultEvaluate(FAULT *fptr);
	ATPG_STATUS Podem(FAULT *fptr, int &total_backtrack_num);
	ATPG_STATUS SetUniqueImpliedValue(FAULT *fptr);
	ATPG_STATUS BackwardImply(GATEPTR gptr, VALUE value);
	GATEPTR FindPropagateGate();
	GATEPTR FindHardestControl(GATEPTR gptr);
	GATEPTR FindEasiestControl(GATEPTR gptr);
	GATEPTR FindPIAssignment(GATEPTR gptr, VALUE value);
	GATEPTR TestPossible(FAULT *fptr);
	void TraceDetectedStemFault(GATEPTR gptr, VALUE val);
	void set_actual_from_expect();
	// defined in fsim.cc
	void MarkOutputGate();
	void MarkPropagateTree(GATEPTR gptr);
	void FaultSimVectors();
	void FaultSim();
	void FaultSimEvaluate(GATE *gptr);
	bool CheckFaultyGate(FAULT *fptr);
	void InjectFaultValue(GATEPTR gptr, unsigned idx, VALUE value);
	int get_sa_error_cnt();

	// defined in psim.cc for parallel logic simulation
	void ParallelLogicSimVectors();
	void ParallelLogicSim();
	void ParallelEvaluate(GATEPTR gptr);
	void PrintParallelIOs(unsigned idx);
	void ScheduleAllPIs();
	// defined in stfsim.cc for single pattern single transition-fault simulation
	void GenerateAllTFaultList();
	void TFaultSimVectors();
	void TFaultSim_t();
	void TFaultSim();
	bool CheckTFaultyGate(TFAULT *fptr);
	bool CheckTFaultyGate_t(TFAULT *fptr);
	VALUE Evaluate_t(GATEPTR gptr);
	void LogicSim_t();
	void PrintTransition();
	void PrintTransition_t();
	void PrintIO_t();

	// defined in tfatpg.cc for transition fault ATPG
	void TFAtpg();
	ATPG_STATUS Initialization(GATEPTR gptr, VALUE target, unsigned &total_backtrack_num);
	ATPG_STATUS BackwardImply_t(GATEPTR gptr, VALUE value);
	GATEPTR FindPIAssignment_t(GATEPTR gptr, VALUE value);
	GATEPTR FindEasiestControl_t(GATEPTR gptr);
	GATEPTR FindHardestControl_t(GATEPTR gptr);
};
#endif
