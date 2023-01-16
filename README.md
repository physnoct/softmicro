# softmicro
This software is a simulator for a multi-byte size instruction microprocessor. This is to replace a hardware Z80 by something like a Z80 on steroids, a "biblically accurate" Z80!
This processor will be implemented in software running on a modern chip. The instruction set is defined in softmicro.ods. It's not compatible with a Z80. It's a first draft which will be subject to change. The simulator works based on simple checks.

Files description:
The files are all piled up in this folder.

softmicro.cpu       definition of softmicro for custom ASM (https://github.com/hlorenzi/customasm)
Makefile            Makefile for assembling files
                    make:  compiles the assembler file
                    make load: copy the binary in a .binary file in the user's home folder (/home/usrname)

*.c, *.h files      Files for running a simulator. Make a project in your favorite IDE.
                    Needs ncurses.
softmicro.ods       LibreOffice file describing all the instructions

*.awk files         A structured macro preprocessor allowing the use of structured assembly (do .. while, etc.)
structures.txt      A description of the structured constructs
