import re
import json
import os
import sys

from optparse import OptionParser
import logging
logging.basicConfig(format='%(asctime)s %(message)s', datefmt='%m_%d %H:%M:%S', level=logging.DEBUG)
logging.basicConfig(filename='py_isc.log', filemode='a', format='%(asctime)s %(message)s', datefmt='%m_%d %H:%M:%S', level=logging.DEBUG)




# Define a data structure to store gate information
class Gate:
    def __init__(self, gate_net_id, gate_id, gate_type, gate_fanout_cnt, gate_fanin_cnt, gate_faults, input_gates_netid_list,fanout_branch_from_gateid,output_gates_netid_list=[]):
        self.gate_net_id = gate_net_id
        self.gate_id = gate_id
        self.gate_type = gate_type
        self.gate_fanout_cnt = gate_fanout_cnt
        self.gate_fanin_cnt = gate_fanin_cnt
        self.gate_faults = gate_faults
        self.input_gates_netid_list = input_gates_netid_list
        self.fanout_branch_from_gateid = fanout_branch_from_gateid
        self.output_gates_netid_list = output_gates_netid_list
    

    def __repr__(self):
        return f"netid:{self.gate_net_id}, gate_id: {self.gate_id}, type: {self.gate_type}, InputsCnt: {self.gate_fanin_cnt}, OutputsCnt: {self.gate_fanout_cnt}, Faults: {self.gate_faults}, InputGateNetList: {self.input_gates_netid_list},OutputGateNetList:{self.output_gates_netid_list}, fanout_branch_from_gateid: {self.fanout_branch_from_gateid}" 

# Function to parse the netlist
def parse_netlist(content):
    gates = []
    lines = content.splitlines()

    # Regular expression to capture gate information
    gate_pattern = re.compile(r"(\d+)\s+(\d+gat|\d+fan)\s+(\w+)\s+(\d+)\s+(\d+)\s*(>.*)?")

    line_cursor = -1

    while line_cursor+1 < len(lines):
        line_cursor += 1 

        line = lines[line_cursor]

        # if line start with * then skip
        if line.startswith("*"):
            continue

        if line is None or line.strip() == "": continue

        #split line by space   
        line = line.strip()
        # print(line)
        line = re.sub(r'\s+', ' ', line) #replace multiple spaces with single space
        groups = line.split(" ")

        gate_net_id = groups[0]
        gate_id = groups[1]
        gate_type = groups[2]
        gate_faults=[]
        input_gates_netid_list = []
        fanout_branch_from_netid=None
        output_gates_netid_list=[]

        #Handle from gate (wire)
        if gate_id.find('fan')>0 and gate_type in ('from'):
            if len(groups) < 3:
                logging.error(f"Invalid line fields for `from`: {line}")
                continue

            fanout_branch_from_netid = groups[3]
            if len(groups) > 4:
                gate_faults = ",".join(groups[4:])


        #Handle regular gate
        if gate_id.find('gat')>0 and gate_type in ('inpt', 'nand', 'nor', 'not', 'buff', 'and', 'or', 'xor', 'xnor'):
            if len(groups) < 4:
                logging.error(f"Invalid line fields for gate: {line}")
                continue

            gate_fanout_cnt = int(groups[3])
            gate_fanin_cnt = int(groups[4])
            if len(groups) > 5:
                gate_faults = ",".join(groups[5:])

            if gate_fanin_cnt>0:
                #grab the next line.
                line_cursor += 1
                next_line = lines[line_cursor]
                input_gates_netid_list = next_line.strip().split()
                

            
        # Create a gate object. both `from` or `regular` gate
        gate = Gate(gate_net_id, gate_id, gate_type, gate_fanout_cnt, gate_fanin_cnt, gate_faults, input_gates_netid_list,fanout_branch_from_netid,output_gates_netid_list)
        gates.append(gate)
    
    return gates

# Find a gate by its netid
def find_gate_by_netid(gates, netid):
    for gate in gates:
        if gate.gate_net_id == netid:
            # return gates.index(gate)
            return gate
    return None

# Find a gate by its gate_id
def find_gate_by_gateid(gates, gate_id):
    for gate in gates:
        if gate.gate_id == gate_id:
            #return the index of the gate
            # return gates.index(gate)
            return gate
    return None


# Print the circuit structure (gates and their connections)
def populate_output_gates(gates):
    for gate in gates:
        parent_gate = None
        # print(gate)
       
        # if gate.gate_type == "from":
        #     parent_gate = find_gate_by_gateid(gates, gate.fanout_branch_from_gateid)
        #     parent_gate.output_gates_netid_list.append(gate.gate_net_id)
        #     print(f"{parent_gate.gate_id} -> Fanout Branch {gate.gate_id}")

        
        if gate.gate_fanin_cnt==0:
            continue

        #loop the input gates netid list
        for input_gate_netid in gate.input_gates_netid_list:
            parent_gate = find_gate_by_netid(gates, input_gate_netid)
            # parent_gate = gates[parent_gate_index]

            if parent_gate.gate_type == "from":
                parent_gate = find_gate_by_gateid(gates, parent_gate.fanout_branch_from_gateid)
                # parent_gate = gates[parent_gate_index]
                # print(f"Wire {gate.gate_id} -> {gate.fanout_branch_from_gateid}")

            # tmp = parent_gate.output_gates_netid_list
            # tmp.append(gate.gate_net_id)
            # parent_gate.output_gates_netid_list = tmp


            parent_gate.output_gates_netid_list.append(gate.gate_net_id)

            # print(f"updated output gate, {parent_gate.gate_id} -> {gate.gate_id}")
    # print("output gate populated")
    return gates



def print_circuit_output(gates):
    for gate in gates:
        if gate.gate_type == "from":
            continue
        # if gate.gate_fanin_cnt>0:
            # print("TO: "+",".join(gate.input_gates_netid_list) +" -> "+gate.gate_id)
        if gate.gate_fanout_cnt>0:
            print("FROM: " +gate.gate_net_id+" TO "+",".join(gate.output_gates_netid_list))

def save_to_json(gates, output_file):
    json_data = []
    for gate in gates:
        gate_dict = {
            "line_number": gate.gate_net_id,
            "net_id": gate.gate_net_id,
            "identifier": gate.gate_id,
            "gate_type": gate.gate_type,
            "fanout": gate.gate_fanout_cnt,
            "fanin": gate.gate_fanin_cnt,
            "inputs": gate.input_gates_netid_list,
            "output": gate.output_gates_netid_list,
            "faults": gate.gate_faults,
        }
        json_data.append(gate_dict)
    
    with open(output_file, 'w') as f:
        json.dump(json_data, f, indent=4)
        print("Circuit Json Output saved to", output_file)


# Example usage
if __name__ == "__main__":
    parser = OptionParser()
    parser.add_option("--file_isc", type="str", help="Path to the ISC file")
    parser.add_option("--out_json", type="str", help="Path to the output JSON file")
    parser.add_option("--action", type="str", help="Action to perform: [parse_to_json]")
    parser.add_option("--debug", action="store_true", dest="debug", default=False, help="Enable debug mode")

    (options, args) = parser.parse_args()
    file_isc = options.file_isc
    out_json = options.out_json

    action= options.action
    debug = options.debug

    if action not in ["parse_to_json"]:
        logging.error(f"Invalid action: {action}")
        sys.exit(1)
    
    if action=="parse_to_json" and not file_isc:
        logging.error("Please provide the path to the ISC file")
        sys.exit(1)

    #check file exist or not
    if file_isc is not None and not os.path.exists(file_isc):
        logging.error(f"File {file_isc} does not exist")
        sys.exit(1)

    with open(file_isc, 'r') as f:
        netlist_content = f.read()


    if action == "parse_to_json":
        # Parse the netlist
        gates = parse_netlist(netlist_content)
        gates = populate_output_gates(gates)

        # Print the circuit structure
        # print_circuit_structure(gates)
        print_circuit_output(gates)

        save_to_json(gates, out_json)
