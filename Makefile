.SUFFIXES: .asm .hex .rel .LNK .bin .softmi .pre

# Otherwise, considered as intermediary files and implicitly deleted
.PRECIOUS: *.pre
.SECONDARY: *.pre

PGM = main

OBJ = main.hex

INCLUDES = bios.inc

AWKPATH = .
STRUCT = $(AWKPATH)/struct_softmicro.awk

STRUCT_FILES = struct_softmicro.awk struct_cond.awk struct_macros.awk struct_tests.awk

AS = -customasm
ASFLAGS = -f annotated
#LINK = -sdldz80

all: $(PGM).hex $(PGM).bin

clean:
	-rm *.rel *.sym *.hex *.bin

# Structured preprocessor
.asm.softmi:
	$(STRUCT) $< > $@

# Compile
.softmi.hex:
	$(AS) $(ASFLAGS) -o $@ $<

# Link
#$(PGM).hex: $(OBJ)
#	$(LINK) -f $(PGM).lnk

# Binary
$(PGM).bin: $(PGM).hex
	hex2bin -b $(PGM).hex

load:
	cp main.hex /dev/ttyS0

install:
	cp $(STRUCT_FILES) $(AWKPATH)
	cp gawk.sh /etc/profile.d

