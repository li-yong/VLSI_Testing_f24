
CXX      = g++
LINK     = g++
# Remove -DDEBUG flag for VLST-Testing Lab0
# CXXFLAGS = -O2 -Wno-all -g
# CXXFLAGS = -O2 -Wall -DDEBUG
CXXFLAGS = -Wno-all -DDEBUG -g -std=c++11
# CXXFLAGS = -g -DDEBUG_ATPG -Wall #Turn on debugging
INCPATH  =
LIBS     = 
DEL_FILE = rm -f
DEL_DIR  = rm -fr
MEMUSG	 = /usr/bin/time -f "Average memory usage: %K \nMaximum memory usage: %M\n"

####### Files

HEADERS = gate.h fault.h circuit.h GetLongOpt.h typeemu.h pattern.h hash.h tfault.h

# Add source file assignment.c to be compiled
SOURCES = circuit.cc main.cc GetLongOpt.cc \
					parser_isc.cc sim.cc atpg.cc fsim.cc 
					# path.cc \
					# sim.cc stfsim.cc atpg.cc fsim.cc pattern.cc
					# readcircuit.tab.cc

# Add object file assignment.o to be linked
OBJECTS = circuit.o main.o GetLongOpt.o  \
					parser_isc.o sim.o  atpg.o fsim.o 
					# path.o \
					# stfsim.o atpg.o fsim.o pattern.o
					# readcircuit.tab.h

TARGET  = cpp_isc_parser.exe

####### Implicit rules

.SUFFIXES: .o .cpp .cc .cxx

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

####### Build rules
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LINK) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS) $(INCPATH)

$(OBJECTS): $(HEADERS) Makefile


clean:
	@$(DEL_FILE) $(OBJECTS) $(TARGET)
	@$(DEL_FILE) readcircuit.tab.cc readcircuit.tab.h lex.yy.cc
	@$(DEL_DIR)	 output input simulator
