#ifndef GATE_H
#define GATE_H
#include <bitset>
#include "typeemu.h" 
using namespace std;

class GATE
{
	private:
		string Name;

		//extend to isc format
		vector<string> isc_StuckAtFaults;
		vector<string> isc_input_gates;
		int isc_net_id; //uniq id for each gate. e.g 1
		string isc_identifier; //name of the gate. e.g 1gat
		// string isc_gate_type; //dup to Function
		int isc_fo_Cnt;
		int isc_fi_Cnt;

		unsigned ID;
		GATEFUNC Function;
		vector<GATE*> Input_list;
		vector<GATE*> Output_list;
		vector<GATE*> Input_fan_list; //gate10->gate8(fanfrom)->gate3. Save g3 to g10 Input_fan_list
		bitset<NumFlags> Flag;
		int Level;
		VALUE Value;
		VALUE Value_t;
		bool Inversion;
		//Utility variable
		unsigned StuckAt[2]; //ryan
		unsigned Count[2]; //used by Levelize(), FindStaticPivot(),PathSearch
		//one pair of WireValues (length defined by PatternNum)
		bitset<PatternNum> WireValue[2]; 
		bitset<PatternNum> FaultFlag;
		// VLSI-Testing Lab1, label for finding paths
		DFS_STATUS dfs_status;
		// VLSI-Testing lab2
		bitset<2> mod_value;

	public:
		//Initialize GATE
		GATE(): Function(G_BAD), Level(-1), Value(X), Value_t(X), Inversion(false) {
			Input_list.reserve(4);
			Output_list.reserve(4);
			Input_fan_list.reserve(4);

			isc_StuckAtFaults.clear(); // Initialize the vector
			// StuckAt[0]=0; //sa0 fault. 0 == no fault. 1==sa0. defined from ISCAS input.
			// StuckAt[1]=0; //sa1 fault. 0 == no fault. 1==sa1
			Count[0] = (0);
			Count[1] = (0);
			WireValue[0].set();   //All parallel bitsets are set to X
			WireValue[1].reset();
			// VLSI-Testing Lab1, initial path label
			dfs_status = WHITE;
			// VLSI-Testing lab2
			mod_value.reset();
		}
		~GATE() {}
		//VLSI-Testing Lab1, operations with the dfs status
		void setDFSStatus(DFS_STATUS flag) {dfs_status = flag;}
		// VLSI-Testing Lab2
		void SetModValue(bitset<2> bits) { mod_value = bits; }
		bitset<2> GetModValue() {return mod_value;}
		// VLSI-Testing lab3
		bitset<PatternNum>* getWireValue() { return WireValue;}

		DFS_STATUS getDFSStatus() {return dfs_status;}
		void SetName(string n){ Name = n;}

		void Set_isc_StuckAt(const vector<string>& faults){isc_StuckAtFaults = faults;}
		vector<string> Get_isc_StuckAt(){return isc_StuckAtFaults;}

		void Set_isc_net_id(int id){isc_net_id = id;}
		int  Get_isc_net_id(){return isc_net_id;};

		void Set_isc_identifier(string id){isc_identifier = id;}
		string Get_isc_identifier(){return isc_identifier;}

		void Set_isc_input_gates(const vector<string>& input_gates){isc_input_gates = input_gates;}
		vector<string> Get_isc_input_gates(){return isc_input_gates;}

		// void Set_isc_gate_type(string type){isc_gate_type = type;}
		void Set_isc_fo_Cnt(int cnt){isc_fo_Cnt = cnt;}
		void Set_isc_fi_Cnt(int cnt){isc_fi_Cnt = cnt;}	

		void SetID(unsigned id){ ID = id;}
		void SetFunction(GATEFUNC f){ Function = f;}
		void AddInput_list(GATE* gptr){Input_list.push_back(gptr);}
		void AddInput_fan_list(GATE* gptr){Input_fan_list.push_back(gptr);}

		
		vector<GATE*> &GetInput_list() { return Input_list; }
		vector<GATE*> &GetOutput_list() { return Output_list; }
		vector<GATE*> &GetInput_fan_list() { return Input_fan_list; }


		void AddOutput_list(GATE* gptr){Output_list.push_back(gptr);}
		void SetLevel(signed l){ Level = l;}
		void SetValue(VALUE v) {Value = v;}
		void InverseValue() {Value = NotTable[Value];}
		void SetValue_t(VALUE v) {Value_t = v;}
		void InverseValue_t() {Value_t = NotTable[Value_t];}
		void IncCount(unsigned i = 0) {Count[i]++;}
		void DecCount(unsigned i = 0) {Count[i]--;}
		void ResetCount(unsigned i = 0) {Count[i] = 0;}
		void ResetAllCount() {Count[0] = 0;Count[1] = 0;}
		unsigned GetCount(unsigned i = 0) { return Count[i];}
		string GetName(){ return Name;}
		unsigned GetID(){ return ID;}
		unsigned GetIscNetId() { return isc_net_id; }
		unsigned GetStuckAt(unsigned i){ return StuckAt[i];}
		GATEFUNC GetFunction(){ return Function;}

		int Get_isc_fo_cnt(){return isc_fo_Cnt;}
		int Get_isc_fi_cnt(){return isc_fi_Cnt;}

		unsigned No_Fanin() { return Input_list.size();}
		unsigned No_Fanout() { return Output_list.size();}
		GATE* Fanin(int i) { return Input_list[i];}
		GATE* Fanout(int i) { return Output_list[i];}
		void ChangeFanin(int i, GATE* g) {Input_list[i] = g;}
		void ChangeFanout(int i, GATE* g) {Output_list[i] = g;}
		signed GetLevel() { return Level;}
		VALUE GetValue() { return Value;}
		VALUE GetValue_t() { return Value_t;}
		void SetInversion(){Inversion = true;}
		void UnSetInversion(){Inversion = false;}
		void SetFlag(FLAGS f) { Flag.set(f); }
		void ResetFlag(FLAGS f) { Flag.reset(f); }
		void ResetFlag() { Flag.reset(); }
		bool GetFlag(FLAGS f) { return Flag[f]; }
		bool Is_Inversion() { return Inversion;}
		bool Is_Unique(unsigned j)
		{
			for (unsigned i = 0;i < j;i++) {
				if (Fanin(i)->GetID() == Fanin(j)->GetID()) return false;
			}
			return true;
		}

		//for fault simulation
		void SetValue1() { WireValue[0].set(); }
		void SetValue1(bitset<PatternNum> &value) { WireValue[0] = value; }
		void SetValue1(unsigned idx) { WireValue[0].set(idx); }
		void ResetValue1() { WireValue[0].reset(); }
		void ResetValue1(unsigned idx) { WireValue[0].reset(idx); }
		void SetValue2(bitset<PatternNum> &value) { WireValue[1] = value; }
		void SetValue2() { WireValue[1].set(); }
		void SetValue2(unsigned idx) { WireValue[1].set(idx); }
		void ResetValue2() { WireValue[1].reset(); }
		void ResetValue2(unsigned idx) { WireValue[1].reset(idx); }
		bool GetValue1(unsigned idx) { return WireValue[0][idx]; }
		bool GetValue2(unsigned idx) { return WireValue[1][idx]; }
		bitset<PatternNum> GetValue1() { return WireValue[0]; }
		bitset<PatternNum> GetValue2() { return WireValue[1]; }
		void ParallelInv() {
			bitset<PatternNum> value(~WireValue[0]);
			WireValue[0] = ~WireValue[1];
			WireValue[1] = value;
		}
		void SetFaultFlag(unsigned idx) { FaultFlag.set(idx); }
		void ResetFaultFlag() { FaultFlag.reset(); }
		bool GetFaultFlag(unsigned idx) { return FaultFlag[idx]; }
		void SetFaultFreeValue() {
			switch (Value) {
				case S1:
					WireValue[0].set(); WireValue[1].set();
					break;
				case S0:
					WireValue[0].reset(); WireValue[1].reset();
					break;
				case X:
					WireValue[0].set(); WireValue[1].reset();
					break;
				default: break;
			}
		}

		void SetWireValue(unsigned i) { WireValue[i].set(); }
		void SetWireValue(unsigned i, bitset<PatternNum> &value) { WireValue[i] = value; }
		void SetWireValue(unsigned i, unsigned idx) { WireValue[i].set(idx); }
		void ResetWireValue(unsigned i) { WireValue[i].reset(); }
		void ResetWireValue(unsigned i, unsigned idx) { WireValue[i].reset(idx); }
		bool GetWireValue(unsigned i, unsigned idx) { return WireValue[i][idx]; }
		bitset<PatternNum> GetWireValue(unsigned i) { return WireValue[i]; }
		char GetTransition_t(){
			if (Value==S0){
				if(Value_t==S0) { return '0'; }
				else{ return 'R'; }
			}
			else{
				if(Value_t==S0) { return 'F'; }
				else{ return '1'; }
			}
		}
		char GetTransition(){
			if (Value_t==S0){
				if(Value==S0) { return '0'; }
				else{ return 'R'; }
			}
			else{
				if(Value==S0) { return 'F'; }
				else{ return '1'; }
			}
		}
};

#endif
