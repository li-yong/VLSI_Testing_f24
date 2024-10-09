
# py_isc_parser

`py_isc_parser` is a Python script for parsing ISC netlist files and converting them into JSON format. It also provides functionality to print the circuit structure and save it to a JSON file.




## Usage
```
$ python py_isc_parser.py --help
Usage: py_isc_parser.py [options]

Options:
  -h, --help           show this help message and exit
  --action=ACTION      Action to perform: [parse_to_json]
  --file_isc=FILE_ISC  Path to the ISC file
  --out_json=OUT_JSON  Path to the output JSON file
  --show_gate_output   Print down steam gate. Current gate output feed to
                       which gates.
  --debug              Enable debug mode
```

## Examples
### To parse an ISC file and save the output to a JSON file:
```
$ python py_isc_parser.py --action parse_to_json --file_isc ../circuits/ISCAS-85/c880.isc --out_json ./c880.json
Netlist parsed. Next to process the output gates
Circuit Json Output saved to ./c880.json
```

### Show the OUTPUT for a gate. ISC file only provide the INPUT.
```
$ python py_isc_parser.py --action parse_to_json --file_isc ../circuits/ISCAS-85/c17.isc --out_json ./c17.json --show_gate_output
Netlist parsed. Next to process the output gates
FROM: 1 TO 10
FROM: 2 TO 16
FROM: 3 TO 10,11
FROM: 6 TO 11
FROM: 7 TO 19
FROM: 10 TO 22
FROM: 11 TO 16,19
FROM: 16 TO 22,23
FROM: 19 TO 23
Circuit Json Output saved to ./c17.json
```

### Output JSON entries.  
c17.json, gate 10 input from gates 1 and 8, output to gate 22.
```json
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
    ....
    ....

    {
        "line_number": "10",
        "net_id": "10",
        "identifier": "10gat",
        "gate_type": "nand",
        "fanout": 1,
        "fanin": 2,
        "inputs": [
            "1",
            "8"
        ],
        "output": [
            "22"
        ],
        "faults": ">sa1"
    },
]
```