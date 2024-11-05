# VLSI-Testing Project 2

This document explains the implementation of Project 2. The build and run procedures are identical to those in Project 1.

Project 2 uses the same circuits as Project 1, with additional functional features:
1. Randomly assign 64 independent input values to the circuit inputs.
2. Perform Parallel Pattern Single Fault Propagation (`PPSFP`) fault simulation.
3. Simulate the error-free (good) circuit output.
4. Inject Stuck-At (SA) faults into the circuit, one at a time.
5. Perform parallel simulation of patterns, and compare the output with the error-free circuit output.
6. Output the detected faults at the end of each pattern.

Observability: Users have the option to:
1. View the generated patterns on each input gate.
2. View the error-free circuit output for those inputs.
3. If an error is detected, view the expected output vs. actual output and display the mismatched output gate.
4. View the number of patterns that detected an SA fault and print the patterns that detected this SA fault.


# Usage and Demo
## Invocation

Invoke with `-action ppsfp` and `-file_isc`. Here, `ppsfp` stands for "Parallel Pattern Single Fault Propagation."
The program will parse the circuit, initialize it, generate random patterns, and simulate the output for the error-free circuit.

```bash
$ ./cpp_isc_parser.exe -action ppsfp -file_isc ../ISCAS-85/c17.isc 
ISC file: ../ISCAS-85/c17.isc

Circuit initialized.
Random patterns generated on input gates, with a parallel pattern count of 64.
Error-free circuit output calculated.
```


## Viewing Generated Patterns
Each pattern is represented by a 64-bit value, with each bit corresponding to a pattern, resulting in a total of 64 patterns processed in parallel.
Note: `PI` (Primary Input) gates do not have `input values` because no gate provides input to a `PI` gate.
Other gates (`AND`, `NAND`, `OR`, `NOR`, `NOT`, `BUF`) display both the `input values`, `expected output`, and `actual output`.
The `actual output` matches the `expected output` in the error-free circuit.



```bash
Show patterns on Gates? 'yes' or 'no': yes
isc_identifier: 1gat, type G_PI
	 expected output: 1010101101001110101010011010010111010000101000110000011111110101
	 actual   output: 1010101101001110101010011010010111010000101000110000011111110101

...
...
isc_identifier: 23gat, type G_NAND
	 input    value : 1001010111101110101100110111111101011111010101111010000011101001
	 input    value : 0111101111001101111100101110110100011100110111001101001111011101
	 expected output: 1110111000110011010011011001001011100011101010110111111100110110
	 actual   output: 1110111000110011010011011001001011100011101010110111111100110110

...
isc_identifier: 837gat, type G_NAND 
	 input    value : 0011111001100011111010111111101011101111111001110100100001110000
	 input    value : 1111011101111101100110111111111110101011011110011011100110111111
	 input    value : 1111111111111101101111111110110110111111111111111011101111111101  <<< example of a more than two inputs gate, c880.isc
	 expected output: 1100100110011110011101000001011101010100100111101111011111001111
	 actual   output: 1100100110011110011101000001011101010100100111101111011111001111

```


## Iterating and Injecting SA Faults One by One
In `c17.isc`, there are 5 Primary Input (`PI`) gates, 2 Primary Output (`PO`) gates, and 23 defined `SA` (Stuck-At) faults. We will iterate through these SA faults one by one. For each SA fault, 64 input patterns are evaluated simultaneously across all gates in the circuit. The output of the `PO` gates is then compared with the error-free circuit output. Any mismatched bits indicate a test pattern that detected the injected SA fault.

For example, consider the NAND gate `19gat`, which has an `sa1` fault defined.
The output shows that `sa1@19gat` was detected at the Primary Output `23gat`, triggered by the input pattern:
|1gat|2gat|3gat|6gat|7gat|
| ---| ---| ---| ---| ---|
|0|0|0|0|1|


```
Run Parallel Pattern Single Fault Propagation(PPSFP) Simulation? 'yes' or 'no': yes

== Injecting SA1 on 19gat. Remaining sa error: 5
PO: 22gat,did not detect the stuck error. 
PO: 23gat
expected output: 1110111000110011010011011001001011100011101010110111111100110110
actual   output: 0110101000010001010011001000000010100000101010000101111100010110
stuck error detected on gate 19gat, removed the >sa1 from the SAlist.
19gat SA1 detected by 14 Input Patterns. Details of the first Input/Output pattern:
	Input   Pattern: 1gat_0,2gat_0,3gat_0,6gat_0,7gat_1,
	Output Expected: 23gat_1,
	Output   Actual: 23gat_0,
```

## Simulation Summary
The program iterates 10 times, with 64 parallel test patterns in each iteration. This results in a maximum of 640 patterns examined in total.
At the end, the simulation summary is printed as follows:


```
=== PPSFP Simulation Completed ===
Circuit              : ./ISCAS-85/c17.isc
Total SA Errors      : 22
Detected Errors      : 22
Detection Ratio      : 1
Iteration Count      : 1


=== PPSFP Simulation Completed ===
Circuit              : ./ISCAS-85/c880.isc
Total SA Errors      : 942
Detected Errors      : 918
Detection Ratio      : 0.974522
Iteration Count      : 10
```
