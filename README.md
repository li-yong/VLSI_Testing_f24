# VLSI-Testing

This project is a VLSI testing tool designed to parse ISC files, and populate the circuit netlist. The tool is implemented in C++ and Python with various functionalities for circuit analysis and testing.

The Project contains two implementations:
## Python Parser
The Python parser reads the ISC file, populate the netlist and saved to a python file.  
The Python parse implemented reqirement #1 and part of #2 in programing project 1 of 654 VLSI_Testing in Fall 2004.

Python Parser Example:
```bash
$ python py_isc_parser/py_isc_parser.py --action parse_to_json --file_isc ./ISCAS-85/c17.isc  --out_json ./c17.json
Netlist parsed. Next to process the output gates
Circuit Json Output saved to ./c17.json

$ head -15 ./c17.json
[
    {
        "line_number": "1",
        "net_id": "1",
        "identifier": "1gat",
        "gate_type": "inpt",
        "fanout": 1,
        "fanin": 0,
        "inputs": [],
        "output": [
            "10"
        ],
        "faults": ">sa1"
    },
    {
```

More details can be found in py_isc_parser/README.md

## C++ Parser 
C++ Parser read from the ISC file, then parse line by line to populate the circuit network.

It implemented reqirement #2, #3 and #4 in programing project 1 of 654 VLSI_Testing in Fall 2004.

### How to run
The tool developed in Ubuntu, but also have brief build and verification on Windows.
Linux binary:   cpp_isc_parser.exe
Windows binary: cpp_isc_parser_win.exe

### Build/Installation
flex and bison are required for build.
```
cd cpp_isc_parser &&  make 
```

Build on Windows MSYS2 UCRT64 preferred.


### Usage
To run the tool, use the following command:

Linux:
```sh
./cpp_isc_parser.exe -parse_isc -file_isc <filename>
```

Windows:
```
cpp_isc_parser_win.exe -parse_isc -file_isc <filename>
```

### Cpp Parser Example
```bash
$ ./cpp_isc_parser/cpp_isc_parser.exe -parse_isc -file_isc ISCAS-85/c17.isc 
Gate_number	Gate_type	Fanout_gates_list
1	PI	 10
2	PI	 16
3	PI	 10 11
6	PI	 11
7	PI	 19
10	NAND	 22
11	NAND	 16 19
16	NAND	 22 23
19	NAND	 23
22	NAND	 24
23	NAND	 25
24	PO	
25	PO	

Gate1	Gate2	Fault
1	0	1
2	0	1
3	0	0
3	0	1
6	0	1
7	0	1
10	3	1
10	0	1
11	3	1
11	0	0
11	0	1
16	11	1
16	0	0
16	0	1
19	11	1
19	0	1
22	16	1
22	0	0
22	0	1
23	16	1
23	0	0
23	0	1
Press Enter to continue...
```
