x86instructionrate: x86instructionrate.s x86instructionrate.c x86instructionrate.h
	gcc -O3 x86instructionrate.s x86instructionrate.c x86instructionrate.h -o x86instructionrate
arm_instructionrate: arminstructionrate.s arminstructionrate.c arminstructionrate.h
	gcc -O3 arminstructionrate.s arminstructionrate.c arminstructionrate.h -o arminstructionrate
x86_instructionrate_win64:
	x86_64-w64-mingw32-gcc -O3 x86instructionrate.c x86instructionrate.s x86instructionrate.h -o x86instructionrate.exe
