# Hey Emacs, this is a -*- makefile -*-
#----------------------------------------------------------------------------
# WinAVR Makefile Template written by Eric B. Weddington, Jörg Wunsch, et al.
#
# Released to the Public Domain
#
# Additional material for this makefile was written by:
# Peter Fleury
# Tim Henigan
# Colin O'Flynn
# Reiner Patommel
# Markus Pfaff
# Sander Pool
# Frederik Rouleau
# Carlos Lamas
#
#----------------------------------------------------------------------------
# On command line:
#
# make all = Make software.
#
# make clean = Clean out built project files.
#
# make coff = Convert ELF to AVR COFF.
#
# make extcoff = Convert ELF to AVR Extended COFF.
#
# make debug = Start either simulavr or avarice as specified for debugging,
#              with avr-gdb or avr-insight as the front end for debugging.
#
# make filename.s = Just compile filename.c into the assembler code only.
#
# make filename.i = Create a preprocessed source file for use in submitting
#                   bug reports to the GCC project.
#
# To rebuild project do "make clean" then "make all".
#----------------------------------------------------------------------------

############ EFB start #########################################################
FREERTOS_DIR = src/FreeRTOS
EFB_DIR = src/FrameWork_EFB
SYSTEM_DIR = src/System
PROJECT_DIR = src
INCLUDE = $(FREERTOS_DIR) $(EFB_DIR) $(PROJECT_DIR) $(SYSTEM_DIR)
SRC_DIRS = $(FREERTOS_DIR) $(EFB_DIR) $(PROJECT_DIR) $(SYSTEM_DIR)
SRC = $(wildcard $(patsubst %, %/*.c, $(SRC_DIRS)))

include mkopts.mak

# Output format. (can be srec, ihex, binary)
FORMAT = ihex

# List Assembler source files here.
#     Make them always end in a capital .S.  Files ending in a lowercase .s
#     will not be considered source files but generated files (assembler
#     output from the compiler), and will be deleted upon "make clean"!
#     Even though the DOS/Win* filesystem matches both .s and .S the same,
#     it will preserve the spelling of the filenames, and gcc itself does
#     care about how the name is spelled on its command-line.
ASRC =

# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
#     Use forward slashes for directory separators.
#     For a directory that has spaces, enclose it in quotes.
INCLUDE_DIRS +=

#---------------- Library Options ----------------
# Minimalistic printf version
PRINTF_LIB_MIN = -Wl,-u,vfprintf -lprintf_min

# Floating point printf version (requires MATH_LIB = -lm below)
PRINTF_LIB_FLOAT = -Wl,-u,vfprintf -lprintf_flt

# If this is left blank, then it will use the Standard printf version.
PRINTF_LIB =
#PRINTF_LIB = $(PRINTF_LIB_MIN)
#PRINTF_LIB = $(PRINTF_LIB_FLOAT)

# Minimalistic scanf version
SCANF_LIB_MIN = -Wl,-u,vfscanf -lscanf_min

# Floating point + %[ scanf version (requires MATH_LIB = -lm below)
SCANF_LIB_FLOAT = -Wl,-u,vfscanf -lscanf_flt

# If this is left blank, then it will use the Standard scanf version.
SCANF_LIB =
#SCANF_LIB = $(SCANF_LIB_MIN)
#SCANF_LIB = $(SCANF_LIB_FLOAT)

MATH_LIB = -lm

# List any extra directories to look for libraries here.
#     Each directory must be seperated by a space.
#     Use forward slashes for directory separators.
#     For a directory that has spaces, enclose it in quotes.
EXTRALIBDIRS =

#---------------- Linker Options ----------------
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS += $(patsubst %,-L%,$(EXTRALIBDIRS))
LDFLAGS += $(PRINTF_LIB) $(SCANF_LIB) $(MATH_LIB)

#---------------- Debugging Options ----------------
# For simulavr only - target MCU frequency.
DEBUG_MFREQ = $(F_CPU)

# Set the DEBUG_UI to either gdb or insight.
# DEBUG_UI = gdb
DEBUG_UI = gdb

# Set the debugging back-end to either avarice, simulavr.
#DEBUG_BACKEND = avarice
DEBUG_BACKEND = simulavr

# GDB Init Filename.
GDBINIT_FILE = __avr_gdbinit

# Debugging port used to communicate between GDB / avarice / simulavr.
DEBUG_PORT = 4242

# Debugging host used to communicate between GDB / avarice / simulavr, normally
#     just set to localhost unless doing some sort of crazy debugging when
#     avarice is running on a different computer.
DEBUG_HOST = localhost

#============================================================================

# Define all object files.
LIB_OBJ = $(SRC:%.c=%.o) $(ASRC:%.S=%.o)
OBJ = $(LIB_OBJ)
# Define all listing files.
LST = $(SRC:%.c=$(OBJDIR)/%.lst) $(ASRC:%.S=$(OBJDIR)/%.lst)

# Default target.
all: checkEnv begin sizebefore build sizeafter end
# $$$$$
#	@if [ ! "$(shell $(OBJDUMP) -j .fuse -s $(TARGET).elf |$(AWK) '/[[:digit:]]+/ {print substr($$2,5,2)}')" = "$(EXPECT_FUSES_LOW)" ] ; then printf "Error: Fuses seems strange, internal clock looks disabled !!\n" >&2;$(CP) /GENERROR &> /dev/null ;fi

# Main help
help:
	@echo "Help"
	@echo "  VAR =" $(VAR)
	@echo "  TODO =" $(SRC)
	@echo $(OBJ)
	@echo $(LST)

# Change the build target to build a HEX file or a library.
build: elf hex eep sig fus lck lss sym lib

elf: $(TARGET).elf
hex: $(TARGET).hex
eep: $(TARGET).eep
sig: $(TARGET).sig
lck: $(TARGET).lck
fus: $(TARGET).fus
lss: $(TARGET).lss
sym: $(TARGET).sym
lib: $(LIBNAME).a

checkEnv:
	@if [ ! -d $(DIR_GENFILES) ] ; then printf "Error: Unable to find generated source files in directory '$(DIR_GENFILES)'. Override it with environment variable 'DIR_GENFILES'.\n" >&2;$(CP) /GENERROR &> /dev/null ;fi
ifeq ($(strip $(platform)),Windows)
	#@if [ ! -d $(DIRAVR) ] ; then printf "Error: Unable to find WinAvr compiler '$(WINAVR_COMPDIR)' in base directory '$(DIR_BASE_WINAVR)'. Override it with environment variable 'DIR_BASE_WINAVR'.\n" >&2;$(CP) /GENERROR &> /dev/null ;fi
endif

# AVR Studio 3.x does not check make's exit code but relies on
# the following magic strings to be generated by the compile job.
begin:
	@echo
	@echo $(MSG_BEGIN)

end:
	@echo $(MSG_END)
	@echo

# Display size of file.
HEXSIZE = $(SIZE) --target=$(FORMAT) $(TARGET).hex
ELFSIZE = $(SIZE) -B $(TARGET).elf #  --mcu=$(MCU)

sizebefore:
	@if test -f $(TARGET).elf; then echo; echo "|=========> $(MSG_SIZE_BEFORE)"; $(ELFSIZE); \
	2>/dev/null; echo; fi

sizeafter:
	@if test -f $(TARGET).elf; then echo; echo "|=========> $(MSG_SIZE_AFTER)"; $(ELFSIZE); \
	2>/dev/null; echo; fi

# Display compiler version information.
gccversion :
	@$(CC) --version

# Generate avr-gdb config/init file which does the following:
#     define the reset signal, load the target file, connect to target, and set
#     a breakpoint at main().
gdb-config:
	@$(REMOVE) $(GDBINIT_FILE)
	@echo define reset >> $(GDBINIT_FILE)
	@echo SIGNAL SIGHUP >> $(GDBINIT_FILE)
	@echo end >> $(GDBINIT_FILE)
	@echo file $(TARGET).elf >> $(GDBINIT_FILE)
	@echo target remote $(DEBUG_HOST):$(DEBUG_PORT) >> $(GDBINIT_FILE)
ifeq ($(DEBUG_BACKEND),simulavr)
	@echo load >> $(GDBINIT_FILE)
endif
	@echo break main >> $(GDBINIT_FILE)

debug: gdb-config $(TARGET).elf
	@simulavr --gdbserver --device $(MCU) --clock-freq $(DEBUG_MFREQ) --port $(DEBUG_PORT)&
	@avr-$(DEBUG_UI) --command=$(GDBINIT_FILE)

# Convert ELF to COFF for use in debugging / simulating in AVR Studio or VMLAB.
COFFCONVERT = $(OBJCOPY) --debugging
COFFCONVERT += --change-section-address .data-0x800000
COFFCONVERT += --change-section-address .bss-0x800000
COFFCONVERT += --change-section-address .noinit-0x800000
COFFCONVERT += --change-section-address .eeprom-0x810000

coff: $(TARGET).elf
	@echo
	@echo $(MSG_COFF) $(TARGET).cof
	$(COFFCONVERT) -O coff-avr $< $(TARGET).cof

extcoff: $(TARGET).elf
	@echo
	@echo $(MSG_EXTENDED_COFF) $(TARGET).cof
	$(COFFCONVERT) -O coff-ext-avr $< $(TARGET).cof

## Automatic rules
# Create final output files (.hex, .eep) from ELF output file.
%.hex: %.elf
	@echo
	@echo "|=======> $(MSG_FLASH) $@"
	$(OBJCOPY) -O $(FORMAT) -R .eeprom -R .fuse -R .lock -R .signature --gap-fill 255 $< $@

%.eep: %.elf
	@echo
	@echo "|=======> $(MSG_EEPROM) $@"
	-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	--change-section-lma .eeprom=0 --no-change-warnings -O $(FORMAT) $< $@ || exit 0

%.sig: %.elf
	@echo
	@echo "|=======> $(MSG_SIG) $@"
	-$(OBJCOPY) -j .signature --set-section-flags=.signature="alloc,load" \
	--change-section-lma .signature=0 --no-change-warnings -O $(FORMAT) $< $@ || exit 0
	#srec_cat AVR_Pam_Epil.sig -intel -o toto -HEX_Dump

%.lck: %.elf
	@echo
	@echo "|=======> $(MSG_SIG) $@"
	-$(OBJCOPY) -j .lock --set-section-flags=.lock="alloc,load" \
	--change-section-lma .lock=0 --no-change-warnings -O $(FORMAT) $< $@ || exit 0
	#srec_cat AVR_Pam_Epil.sig -intel -o toto -HEX_Dump

%.fus: %.elf
	@echo
	@echo "|=======> $(MSG_SIG) $@"
	-$(OBJCOPY) -j .fuse --set-section-flags=.fuse="alloc,load" \
	--change-section-lma .fuse=0 --no-change-warnings -O $(FORMAT) $< $@ || exit 0
	#srec_cat AVR_Pam_Epil.sig -intel -o toto -HEX_Dump

# Create extended listing file from ELF output file.
%.lss: %.elf
	@echo
	@echo "|=======> $(MSG_EXTENDED_LISTING) $@"
	$(OBJDUMP) -h -S -z $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo
	@echo "|=======> $(MSG_SYMBOL_TABLE) $@"
	$(NM) -n $< > $@

# Create library from object files.
.SECONDARY : $(TARGET).a
.PRECIOUS : $(OBJ)
%.a: $(OBJ)
	@echo
	@echo "|=====> $(MSG_CREATING_LIBRARY) $@"
	$(AR) $@ $(filter-out ${FILE_MAIN}.o,$(LIB_OBJ))

# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(OBJ)
%.elf: $(OBJ)
	@echo
	@echo "|=====> $(MSG_LINKING) $@"
	$(CC) $(ALL_CFLAGS) $^ --output $@ $(LDFLAGS)

# Compile: create object files from C source files.
$(OBJDIR)/%.o : %.c
	@echo
	@echo "===> $(MSG_COMPILING) $<"
	$(CC) -c $(ALL_CFLAGS) $< -o $@

# Compile: create assembler files from C source files.
%.s : %.c
	@echo
	@echo "|===> $(MSG_COMPILING_ASM) $<"
	$(CC) -S $(ALL_CFLAGS) $< -o $@

# Assemble: create object files from assembler source files.
$(OBJDIR)/%.o : %.S
	@echo
	@echo "|=====> $(MSG_ASSEMBLING) $<"
	$(CC) -c $(ALL_ASFLAGS) $< -o $@

# Create preprocessed source for use in sending a bug report.
%.i : %.c
	$(CC) -E -mmcu=$(MCU) -I. $(CFLAGS) $< -o $@

# Target: run unit tests
check: all
	@$(MAKE) -C ../tests

# Target: run acceptance tests
accept: all
	@$(MAKE) -C ../accept

# Target: clean project.
clean: begin clean_list end

clean_list :
	@echo
	@echo $(MSG_CLEANING)
	@$(REMOVE) $(TARGET).hex
	@$(REMOVE) $(TARGET).eep
	@$(REMOVE) $(TARGET).sig
	@$(REMOVE) $(TARGET).lck
	@$(REMOVE) $(TARGET).fus
	@$(REMOVE) $(TARGET).cof
	@$(REMOVE) $(TARGET).elf
	@$(REMOVE) $(TARGET).map
	@$(REMOVE) $(TARGET).sym
	@$(REMOVE) $(TARGET).lss
	@$(REMOVE) $(LIBNAME).a
	@$(REMOVE) $(SRC:%.c=%.o) $(SRC:%.c=%.lst)
	@$(REMOVE) $(SRC:.c=.s)
	@$(REMOVE) $(SRC:.c=.d)
	@$(REMOVE) $(SRC:.c=.i)
	@$(REMOVEDIR) $(DEPDIR)


# Create object files directory
$(shell mkdir $(OBJDIR) 2>/dev/null)

# Include the dependency files.
-include $(shell mkdir $(DEPDIR) 2>/dev/null) $(wildcard $(DEPDIR)/*)

# Listing of phony targets.
.PHONY : help all checkEnv begin finish end sizebefore sizeafter gccversion \
build elf hex eep sig lck fus lss sym coff extcoff \
clean clean_list debug gdb-config check show_src
