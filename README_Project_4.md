# VLSI-Testing Project 3

This document explains the project 4 BIST implementation. 



Load and Initialize the circuit.
Init TPG_LFSR
Init ORA_MISR

For 1 to 100:
	TPG Generate a 32bit test pattern.  e.g 10110101010101010101011101001010
	Simulate good circuit Output.  e.g  `Good circuit output: 10`
	MISR calculate the golden_signature. e.g. `Golden signature calculated: 0100000000000000`

	For each SA errors in circuit:
		Reset circuit to error free
		inject the SA
		Simulate circuit output
		MISR calculate the signature.

		IF (Sig != Sig_golden):
			Error_Detected; remove SA from circuit; 
			<!-- continue next SA -->


		IF (OP == OP_golden) and (Sig == Sig_golden):
			Error_Not_Detected
			<!-- continue next SA -->
		
		IF (OP != OP_golden) and (Sig == Sig_golden):
			aliasing += 1

	
	Print BIST Result
			











Project 3 use the same circuits as the Project, with additional functional features:
1. 

Observability. User have options to:
1.  View the generated pattens on each Input gate.
2.  View the good circuit output for those input.
3.  If error detected, view the expected output vs actual output, show the mismatched output gate.
4.  View how many patterns detected a SA fault, and print the pattern catches this SA fault.


# Usage and Demo
## Invoke

Invoke with `-action ATPG` with `-file_isc`. Then program will read the circuit definition file then proceed to ATPG function which are depicted as following.

```
$ ./cpp_isc_parser.exe -action ATPG -file_isc ../ISCAS-85/c17.isc 
```

## Levelize the gate, then Calculate the Controllability `CC0` `CC1` in level order.

```
Levelize the gates in circuit

Calculating gates controllability
init controllability at level 1
init controllability at level 2
init controllability at level 3
init controllability done
```

## Show CC0, CC1
```
Show Controllability CC0, CC1 ? 'yes' or 'no': yes
1  1gat, CC0 1, CC1 1
2  2gat, CC0 1, CC1 1
3  3gat, CC0 1, CC1 1
8  8fan, CC0 1, CC1 1
9  9fan, CC0 1, CC1 1
6  6gat, CC0 1, CC1 1
7  7gat, CC0 1, CC1 1
10  10gat, CC0 3, CC1 2
11  11gat, CC0 3, CC1 2
14  14fan, CC0 3, CC1 2
15  15fan, CC0 3, CC1 2
16  16gat, CC0 4, CC1 2
20  20fan, CC0 4, CC1 2
21  21fan, CC0 4, CC1 2
19  19gat, CC0 4, CC1 2
22  22gat, CC0 5, CC1 4
23  23gat, CC0 5, CC1 5
```

## Initialize the gate input
As the objective is to find the particular input pattern to test a certain SA fault,  all input are initialized to `x` means `unknown` at the start of the ATPG.
```
Initalizing gates for backtracing. Set gate input to x
```

### Iterate the SA faults, run ATPG for each SA fault.
For a SA fault, the `objective` is to set the output of the gate to Normal value, which is the opposite to the SA value. Backtrace function then translate the `objective` into `PI assignment`. If there are more than once optional path, `controllability` is referenced on selecting the optimal path. The backtrace runs through the target gate backwards until the `PI` gate.


If there is a `PI` combination that can lead to the Target gate to the Target value, that combination pattern is printed.
```
Run PODEM ATPG? 'yes' or 'no': yes

Iterating all SA errors in circuit, then backtrace input pattern to justify the SA error

==>sa1@1gat. Backtracing to find PI gate pattern to justify gate 1gat to value 0
	1gat_0|2gat_1|3gat_1|6gat_1|7gat_1|

Press ENTER to continue..., ctrl+c to exit
```

If no `PI` combination can satisfy the required gate and value, due to conflicting on the gate, then that SA error is untestable.

```
==>sa1@10gat. Backtracing to find PI gate pattern to justify gate 10gat to value 0
	conflict, not possible set NAND gate to 0, because one input is 0

Press ENTER to continue..., ctrl+c to exit
```

