bison -d -t readcircuit.y
mv readcircuit.tab.c readcircuit.tab.cc
g++ -c -O2 -Wall -g  -o readcircuit.tab.o readcircuit.tab.cc
flex readcircuit.l 
mv lex.yy.c lex.yy.cc
g++ -c -O2 -Wall -g  -o lex.yy.o lex.yy.cc
g++ -c -O2 -Wall -g  -o circuit.o circuit.cc
g++ -c -O2 -Wall -g  -o main.o main.cc
g++ -c -O2 -Wall -g  -o GetLongOpt.o GetLongOpt.cc
g++ -c -O2 -Wall -g  -o atpg.o atpg.cc
g++ -c -O2 -Wall -g  -o fsim.o fsim.cc
g++ -c -O2 -Wall -g  -o sim.o sim.cc
g++ -c -O2 -Wall -g  -o psim.o psim.cc
g++ -c -O2 -Wall -g  -o stfsim.o stfsim.cc
g++ -c -O2 -Wall -g  -o tfatpg.o tfatpg.cc
g++ -c -O2 -Wall -g  -o path.o path.cc
g++ -c -O2 -Wall -g  -o pattern.o pattern.cc
g++ -c -O2 -Wall -g  -o compiledCodeSim.o compiledCodeSim.cc
g++ -c -O2 -Wall -g  -o checkpoint.o checkpoint.cc
g++ -c -O2 -Wall -g  -o bridging.o bridging.cc
g++ -c -O2 -Wall -g  -o bfsim.o bfsim.cc
g++ -O2 -Wall -g -o atpg readcircuit.tab.o lex.yy.o circuit.o main.o GetLongOpt.o atpg.o fsim.o sim.o psim.o stfsim.o tfatpg.o path.o pattern.o compiledCodeSim.o checkpoint.o bridging.o bfsim.o -lreadline -lcurses 
