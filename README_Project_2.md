# VLSI-Testing

This document explains the project 2 implementation. Build and Run are identical as Project 1.

Project 2 use the same circuits as the Project, with additional functional features:
1. Randomly assign 64 independent input values to the inputs of the circuit.
2. Perform Parallal Pattern Single Fault (PPSFP) fault simulation.
3. Simulate the good(error-free) circuit output.
4. Inject Stuck At (SA) fault defined to circuit one at a time.
5. Parallel simulate patterns, compare the output with good output generate on error-free circuit.
6. Write out faults detected at the end of each pattern.

Observability. User have options to:
1.  View the generated pattens on each Input gate.
2.  View the good circuit output for those input.
3.  If error detected, view the expected output vs actual output, show the mismatched output gate.
4.  View how many patterns detected a SA fault, and print the pattern catched this SA fault.


# Demo
## Invoke

Invoke with `-action ppsfp` with `-file_isc`. Then program will 
    parse the ciruit, Initialize Circuit, Generate Random Pattern, and Simulate Good circuit value.

```
$ ./cpp_isc_parser.exe -action ppsfp -file_isc ../ISCAS-85/c17.isc 
ISC file: ../ISCAS-85/c17.isc

Circuit initalizated.
Random patterns generated on Input gates, parallel pattern count 64.
Good circuit output calculated.
```

## Show geneated patterns
Each pattern is in 64bit, each bit stands for a pattern, total 64 patterns in parallel.
Note `PI` gate don't have the `input vaule` because no gate input to `PI` gate.
Other gates (`AND`,`NAND`, `OR`, `NOR`, `NOT`, `BUF` ) show both `input value`, `expected output` and `actual output`.
The `actual output` are identify to `expected output` on the good circuit. 
```
Show patterns on Gates? 'yes' or 'no': yes
isc_identifer: 1gat, type G_PI
	 expected output: 1010101101001110101010011010010111010000101000110000011111110101
	 actual   output: 1010101101001110101010011010010111010000101000110000011111110101
isc_identifer: 2gat, type G_PI
	 expected output: 1100100100011101111011001001111101000110101111101111001010011100
	 actual   output: 1100100100011101111011001001111101000110101111101111001010011100
...
...
isc_identifer: 22gat, type G_NAND
	 input    value : 1111111010111011010111100111111001101111011111111111111101001110
	 input    value : 0011011111100011100100110110100010111001110100010000110101100011
	 expected output: 1111111111111111111111111111111111111111111111111111111111111111
	 actual   output: 1100100101011100111011011001011111010110101011101111001010111101
isc_identifer: 23gat, type G_NAND
	 input    value : 0011011111100011100100110110100010111001110100010000110101100011
	 input    value : 0110110100011111110101110111101011111001110100001000111110100111
	 expected output: 1111111111111111111111111111111111111111111111111111111111111111
	 actual   output: 1101101011111100011011001001011101000110001011111111001011011100

```

## Iterate SA fault, inject SA fault one by one
Totally 23 SA fault defined in `c17.isc`. 
Take NAND gate `22gat` for example, it has both `>sa0` and `sa1` defined.
The output shows both of them were detected,
Both `sa0` and `sa1` are covered by input pattern: `1gat_0,2gat_0,3gat_1,6gat_1,7gat_0`

```
Run Parallel Pattern Single Fault (PPSF) Simulation? 'yes' or 'no': yes

== Injecting SA0 on 16gat. Remaining sa error: 8
PO: 22gat
expected output: 1111111111111111111111111111111111111111111111111111111111111111
actual   output: 1100100101011100111011011001011111010110101011101111001010111101
stuck error detected on gate 16gat, removed the >sa0 fr'
om the SAlist.
16gat SA0 detected by 24 Input Patterns. Details of the first Input/Output pattern:
	Input   Pattern: 1gat_0,2gat_0,3gat_1,6gat_1,7gat_0
	Output Expected: 22gat_1
	Output   Actual: 22gat_0

== Injecting SA1 on 16gat. Remaining sa error: 7
PO: 22gat
expected output: 1111111111111111111111111111111111111111111111111111111111111111
actual   output: 1100100101011100111011011001011111010110101011101111001010111101
stuck error detected on gate 16gat, removed the >sa1 from the SAlist.
16gat SA1 detected by 24 Input Patterns. Details of the first Input/Output pattern:
	Input   Pattern: 1gat_0,2gat_0,3gat_1,6gat_1,7gat_0
	Output Expected: 22gat_1
	Output   Actual: 22gat_0

```