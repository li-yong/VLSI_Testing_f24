import re
import json

# Define a data structure to store gate information
class Gate:
    def __init__(self, gate_id, gate_type, inputs, outputs, faults):
        self.gate_id = gate_id
        self.gate_type = gate_type
        self.inputs = inputs
        self.outputs = outputs
        self.faults = faults
    
    def __repr__(self):
        return f"Gate ID: {self.gate_id}, Type: {self.gate_type}, Inputs: {self.inputs}, Outputs: {self.outputs}, Faults: {self.faults}"

# Function to parse the netlist
def parse_netlist(content):
    gates = []
    lines = content.splitlines()

    # Regular expression to capture gate information
    gate_pattern = re.compile(r"(\d+)\s+(\d+gat|\d+fan)\s+(\w+)\s+(\d+)\s+(\d+)\s*(>.*)?")

    for line in lines:
        match = gate_pattern.match(line.strip())
        if match:
            gate_id = match.group(1)
            gate_type = match.group(2).replace("gat", "gate").replace("fan", "fanout")
            inputs = match.group(3)
            outputs = match.group(4)
            faults = match.group(6).strip() if match.group(6) else "None"
            
            # Create a gate object
            gate = Gate(gate_id, gate_type, inputs, outputs, faults)
            gates.append(gate)
    
    return gates

# Print the circuit structure (gates and their connections)
def print_circuit_structure(gates):
    for gate in gates:
        print(gate)

def save_to_json(gates, output_file):
    json_data = []
    for line_number, gate in gates:
        gate_dict = {
            "line_number": line_number,
            "identifier": gate.gate_id,
            "gate_type": gate.gate_type,
            "fanout": int(gate.outputs),
            "fanin": int(gate.inputs) if gate.inputs.isdigit() else 0,
            "inputs": [int(gate.inputs)] if gate.inputs.isdigit() else [],
            "faults": gate.faults.split() if gate.faults != "None" else []
        }
        json_data.append(gate_dict)
    
    with open(output_file, 'w') as f:
        json.dump(json_data, f, indent=4)


# Example usage
if __name__ == "__main__":
    # Simulating the content read from the file (as an example)
    netlist_content = '''*c17 iscas example (to test conversion program only)
    *---------------------------------------------------
    *  total number of lines in the netlist ..............    17
        1     1gat inpt    1   0      >sa1
        2     2gat inpt    1   0      >sa1
        3     3gat inpt    2   0 >sa0 >sa1
        8     8fan from     3gat      >sa1
        9     9fan from     3gat      >sa1
        6     6gat inpt    1   0      >sa1
    10    10gat nand    1   2      >sa1
    11    11gat nand    2   2 >sa0 >sa1
    16    16gat nand    2   2 >sa0 >sa1
    19    19gat nand    1   2      >sa1
    22    22gat nand    0   2 >sa0 >sa1
    '''

    # Parse the netlist
    gates = parse_netlist(netlist_content)

    # Print the circuit structure
    print_circuit_structure(gates)

    save_to_json(gates, 'output.json')
