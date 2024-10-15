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

#define xstr(s) ystr(s)
#define ystr(s) #s
#define OUTDIR output
#define SIMDIR simulator

typedef GATE* GATEPTR;

class CIRCUIT
{
	private:
		string Name;
		PATTERN Pattern;
		vector<GATE*> Netlist;
		vector<GATE*> PIlist; //store the gate indexes of PI
		vector<GATE*> POlist;
		vector<GATE*> PPIlist;
		vector<GATE*> PPOlist;
		list<FAULT*> Flist; //collapsing fault list
		list<FAULT*> UFlist; //undetected fault list
		list<TFAULT*> TFlist; //collapsing fault list
		list<TFAULT*> UTFlist; //undetected fault list
		int MaxLevel;
		int BackTrackLimit; //backtrack limit for Podem
		typedef list<GATE*> ListofGate;
		typedef list<GATE*>::iterator ListofGateIte;
		ListofGate* Queue;
		ListofGate GateStack;
		ListofGate PropagateTree;
		ListofGateIte QueueIte;
		//VLSI-Testing Lab1, stack for path
		std::vector<GATE*> path_stack;
		int path_count;
		string dest_gate_name;
		string input_name, output_name;
		ofstream ofs; // for printing logicsim output file
		//VLSI-Testing Lab3
		int evaluation_count;
		double avg_eval_cnt_pattern;
		double percent_eval_cnt;
		int pattern_num;
		ofstream ofsHeader, ofsMain, ofsEva, ofsPrintIO;
		//VLSI-Testing Lab4
		list<FAULT*> CPFlist; //collapsing fault list
		list<FAULT*> UCPFlist; //undetected fault list


	public:
		//Initialize netlist
		CIRCUIT(): MaxLevel(0), BackTrackLimit(10000) {
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
				int NO_PPI = 2048, int NO_PPO = 2048) {
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
		~CIRCUIT() {
			for (long unsigned i = 0;i<Netlist.size();++i) { delete Netlist[i]; }
			list<FAULT*>::iterator fite;
			for (fite = Flist.begin();fite!=Flist.end();++fite) { delete *fite; }
			if(ofs.is_open())
				ofs.close();
			if(ofsHeader.is_open())
				ofsHeader.close();
			if(ofsMain.is_open())
				ofsMain.close();
			if(ofsEva.is_open())
				ofsEva.close();
			if(ofsPrintIO.is_open())
				ofsPrintIO.close();
		}

		void AddGate(GATE* gptr) { Netlist.push_back(gptr); }
		void SetName(string n){ Name = n;}
		string GetName(){ return Name;}


		vector<GATE*> GetNetlist(){return Netlist;}
		void SetNetlist(vector<GATE*> nlst ){Netlist = nlst;}

		int GetMaxLevel(){ return MaxLevel;}
		void SetBackTrackLimit(int bt) { BackTrackLimit = bt; }
		// GATE* GetNetlist()( return Netlist; ) //yong
		//Access the gates by indexes
		GATE* Gate(int index) { return Netlist[index]; }
		GATE* PIGate(int index) { return PIlist[index]; }
		GATE* POGate(int index) { return POlist[index]; }
		GATE* PPIGate(int index) { return PPIlist[index]; }
		GATE* PPOGate(int index) { return PPOlist[index]; }

		GATE* Find_Gate_by_name(string name){
			GATE* gptr;
			
			for (int i = 0;i < No_Gate();i++) {
				gptr = Gate(i);
				if (gptr->GetName() == name) {
					return gptr;
				}
			}
			//print no gate found
			std::cerr << "No gate found with name: " << name << std::endl;
			return nullptr;

		}


		GATE* Find_Gate_by_isc_netid(int isc_netid){
			GATE* gptr;
			
			
			for (int i = 0;i < No_Gate();i++) {
				gptr = Gate(i);
				if (gptr->GetIscNetId() == isc_netid) {
					return gptr;
				}
			}
			//print no gate found
			std::cerr << "No gate found with netid: " << std::to_string(isc_netid) << std::endl;
			return nullptr;

		}

		int No_Gate() { return Netlist.size(); }
		int No_PI() { return PIlist.size(); }
		int No_PO() { return POlist.size(); }
		int No_PPI() { return PPIlist.size(); }
		int No_PPO() { return PPOlist.size(); }

		void copyPItoPattern(){
			vector<GATE*>::iterator it; 
			for(it = PIlist.begin(); it != PIlist.end(); it++){
				Pattern.addInList(*it);
			}
		}
		void genRandomPattern(string pattern_name, int number){
			Pattern.setPatternName(pattern_name);
			copyPItoPattern();
			Pattern.genRandomPattern(number);
			Pattern.setPatterninput();
		}
		void genRandomPatternUnknown(string pattern_name, int number){
			Pattern.setPatternName(pattern_name);
			copyPItoPattern();
			Pattern.genRandomPatternUnknown(number);
			Pattern.setPatterninput();
		}

		void InitPattern(const char *pattern) {
			Pattern.Initialize(const_cast<char *>(pattern), PIlist.size(), "PI");
		}

		void openFile(string file_name) {
			cout << "in function openFile" << endl;
			ofs.open(file_name.c_str(), ofstream::out | ofstream::app);
			if(!ofs.is_open())
				cout << "Cannot open file: " << file_name << endl;
			else
				cout << "Successfully open file: " << file_name << endl;
		}

		void openOutputFile(string file_name) {
			char str[] = "mkdir ";
			strcat(str, xstr(OUTDIR));
			system(str);

			strcpy(str, "./"); 
			strcat(str, xstr(OUTDIR));
			strcat(str, "/");
			strcat(str, file_name.c_str());
			ofs.open(str, ofstream::out | ofstream::trunc);
			if(!ofs.is_open())
				cout << "Cannot open file!\n";
		}

		void openSimulatorFile(string file_name) {
			char str[] = "mkdir ";
			strcat(str, xstr(SIMDIR));
			system(str);

			strcpy(str, "./"); 
			strcat(str, xstr(SIMDIR));
			strcat(str, "/");
			strcat(str, file_name.c_str());
			ofs.open(str, ofstream::out | ofstream::trunc);
			ofsHeader.open("./simulator/header", ofstream::out|ofstream::trunc);
			ofsMain.open("./simulator/main", ofstream::out|ofstream::trunc);
			ofsEva.open("./simulator/evaluate", ofstream::out|ofstream::trunc);
			ofsPrintIO.open("./simulator/printIO", ofstream::out|ofstream::trunc);
			if(!ofs.is_open())
				cout << "Cannot open output file!\n";
			if(!ofsHeader.is_open())
				cout << "Cannot open header!\n";
			if(!ofsMain.is_open())
				cout << "Cannot open main!\n";
			if(!ofsEva.is_open())
				cout << "Cannot open evaluate!\n";
			if(!ofsPrintIO.is_open())
				cout << "Cannot open printIO!\n";
		}

		void Schedule(GATE* gptr)
		{
			if (!gptr->GetFlag(SCHEDULED)) {
				gptr->SetFlag(SCHEDULED);
				Queue[gptr->GetLevel()].push_back(gptr);
			}
		}

		// VLSI-Testing Lab1
		// defined in path.cc
		void path(string src_name_gate, string dest_gate_name);
		bool findPath();
		void printPath();

		//defined in fsim.cc
		void MarkOutputGate();

		// VLST-Testing Lab6
		void AtpgRandomPattern();
		void GenerateAllCPFaultListForFsim();
		int FaultSimRandomPattern();
		void closeOfs() { 
			if(ofs.is_open())
				ofs.close(); 
			Pattern.closeOfs();
		}
		void genRandomPatternOnly(string pattern_name, int number){
			copyPItoPattern();
			Pattern.genRandomPatternOnly(number);
			Pattern.setPatterninput();
		}
			
		//defined in circuit.cc

		vector<GATE *> GetGateInLevel(int);


		void Levelize_0();
		void Levelize_1();

		void Levelize();
		void Levelize_recu(GATE*, int);
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

		void InitializeQueue();
		int get_max_netid();


		//defined in atpg.cc
		void GenerateAllFaultList();
		void GenerateCheckPointFaultList();
		void GenerateFaultList();
		void Atpg();
		void SortFaninByLevel();
		bool CheckTest();
		bool TraceUnknownPath(GATEPTR gptr);
		bool FaultEvaluate(FAULT* fptr);
		ATPG_STATUS Podem(FAULT* fptr, int &total_backtrack_num);
		ATPG_STATUS SetUniqueImpliedValue(FAULT* fptr);
		ATPG_STATUS BackwardImply(GATEPTR gptr, VALUE value);
		GATEPTR FindPropagateGate();
		GATEPTR FindHardestControl(GATEPTR gptr);
		GATEPTR FindEasiestControl(GATEPTR gptr);
		GATEPTR FindPIAssignment(GATEPTR gptr, VALUE value);
		GATEPTR TestPossible(FAULT* fptr);
		void TraceDetectedStemFault(GATEPTR gptr, VALUE val);

};
#endif
