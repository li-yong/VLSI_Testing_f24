# VLSI-Testing

This project is a VLSI testing tool designed to parse ISC files and populate the circuit netlist. The tool is implemented in C++ and Python, offering various functionalities for circuit analysis and testing.

The entire project composed by three sub-projects, each sub-project's  `scope`, `specification`, `usage`, `example_output` are in `README_Project_n.md`.

# Build
In Linux, build with  `g++`. For Windows, build in `MSYS2 UCRT64` is preferred.
```
$ cd cpp_isc_parser
$ make 
g++ -c -Wno-all -DDEBUG -g -std=c++11  -o circuit.o circuit.cc
g++ -c -Wno-all -DDEBUG -g -std=c++11  -o main.o main.cc
g++ -c -Wno-all -DDEBUG -g -std=c++11  -o GetLongOpt.o GetLongOpt.cc
g++ -c -Wno-all -DDEBUG -g -std=c++11  -o parser_isc.o parser_isc.cc
g++ -Wno-all -DDEBUG -g -std=c++11 -o cpp_isc_parser.exe circuit.o main.o GetLongOpt.o parser_isc.o  
```
 `cpp_isc_parser.exe` will be generated in the current working directory.


