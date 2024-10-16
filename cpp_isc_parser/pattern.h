#ifndef PATTERN_H
#define PATTERN_H

#include <fstream>
#include <stdlib.h>
#include "gate.h"

#define xstr(s) ystr(s) 
#define ystr(s) #s     
#define INDIR input

using namespace std;

class PATTERN
{
    private:
        ifstream patterninput;
        vector<GATE*> inlist;
        int no_pi_infile;
				string pattern_name;
				ofstream ofs;
    public:
        PATTERN(): no_pi_infile(0){}
				~PATTERN(){
					if(ofs.is_open())
						ofs.close();
					if(patterninput.is_open())
						patterninput.close();
				}
				bool eof() { return (patterninput.eof()); }
				void Initialize(char* IFileName, int no_pi, string TAG);
        //sign next input pattern to PI
        void ReadNextPattern();
        void ReadNextPattern_t();
				void ReadNextPattern(unsigned idx);
				// Add for VLSI-Testing lab2
        void ReadNextModPattern();
				void addInList(GATE* gate) {inlist.push_back(gate);}
				void setPatternName(string name) {pattern_name = name;}
				void setupPatternFile();
				void setPatterninput();
				void initPattern();
				void genRandomPattern(int pattern_number);
				void genRandomPatternUnknown(int pattern_number);
				// VLSI-Testing lab3
				vector<GATE*>* getInlistPtr() { return &inlist;}
				// VLSI-Testing lab6
				void genRandomPatternOnly(int pattern_number);
				void closeOfs() {
					if(ofs.is_open())
						ofs.close();
				}
};
#endif
