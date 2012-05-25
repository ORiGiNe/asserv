
ifeq ($(strip $(DIR_BASE_WINAVR)),)
	DIR_BASE_WINAVR = G:/
endif

ifeq ($(strip $(SystemDrive)),)
	platform=Linux
	DIRAVR=/usr# Anciennement /usr/lib
	DEPDIR=.dep

else
	platform=Windows
	WINAVR_VER=20100110
	WINAVR_COMPDIR=WinAVR-$(WINAVR_VER)
	DIRAVR=$(DIR_BASE_WINAVR)/$(WINAVR_COMPDIR)
	DEPDIR=.windep

	# Define programs and commands.
	SHELL=sh
	WINSHELL=cmd
endif

## executables
CC=avr-gcc
OBJCOPY=avr-objcopy
OBJDUMP=avr-objdump
SIZE=avr-size
AR=avr-ar rcs
NM=avr-nm
REMOVE=rm -f
REMOVEDIR=$(REMOVE) -r
COPY=cp
AWK=awk
TEE=tee
TIME=time

## Address sections
ADRDEFS =

# MCU name
TARGET = arduino-robot
MCU = atmega2560
CLK_SPEED = 16

# Custom defines
DEFINES_CFLAGS =

# Processor frequency.
#     This will define a symbol, F_CPU, in all source code files equal to the
#     processor frequency. You can then use this symbol in your source code to
#     calculate timings. Do NOT tack on a 'UL' at the end, this will be done
#     automatically to create a 32-bit value in your source code.
#     Typical values are:
#         F_CPU =  1000000
#         F_CPU =  1843200
#         F_CPU =  2000000
#         F_CPU =  3686400
#         F_CPU =  4000000
#         F_CPU =  7372800
#         F_CPU =  8000000
#         F_CPU = 11059200
#         F_CPU = 14745600
#         F_CPU = 16000000
#         F_CPU = 18432000
#         F_CPU = 20000000
F_CPU = $(CLK_SPEED)000000

# Object files directory
#     To put object files in current directory, use a dot (.), do NOT make
#     this an empty or blank macro!
OBJDIR = .

# Optimization level, can be [0, 1, 2, 3, s].
#     0 = turn off optimization. s = optimize for size.
#     (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
OPT = s

# Compiler flag to set the C Standard level.
#     c89   = "ANSI" C
#     gnu89 = c89 plus GCC extensions
#     c99   = ISO C99 standard (not yet fully implemented)
#     gnu99 = c99 plus GCC extensiöons
CSTANDARD = -std=gnu99

# Debugging format.
#     Native formats for AVR-GCC's -g are dwarf-2 [default] or stabs.
#     AVR Studio 4.10 requires dwarf-2.
#     AVR [Extended] COFF format requires stabs, plus an avr-objcopy run.
DEBUG = dwarf-2

# Build Warnings
# -Wunreachable-code -Wnosign-compare -mshort-calls -fno-unit-at-a-time
WARNINGS = -Wall -Wextra -Wshadow -Wpointer-arith -Wbad-function-cast \
           -Wcast-align -Wsign-compare -Waggregate-return -Wstrict-prototypes \
           -Wmissing-prototypes -Wmissing-declarations -Wunused -Wundef

# C dialect options
# -Wunreachable-code -Wnosign-compare -mshort-calls -fno-unit-at-a-time
CDIALECTOPT = -fsigned-char -funsigned-bitfields -fpack-struct -fshort-enums

# Place -D or -U options here for C sources
CDEFS = -DF_CPU=$(F_CPU)UL -DGCC_MEGA_AVR

# Place -D or -U options here for ASM sources
ADEFS = -DF_CPU=$(F_CPU) -DGCC_MEGA_AVR

# Compiler flags to generate dependency files.
GENDEPFLAGS = -MMD -MP -MF $(DEPDIR)/$(@F).d

# Compiler flags to handle target architecture.
ARCHFLAGS = -mmcu=$(MCU)

# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
#     Use forward slashes for directory separators.
#     For a directory that has spaces, enclose it in quotes.
INCLUDE_DIRS = $(DIRAVR)/avr/include $(DIRAVR)/avr/include/avr $(INCLUDE)

#---------------- Compiler Options C ----------------
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
COMMON_CFLAGS = -g$(DEBUG)
COMMON_CFLAGS += $(CDEFS)
COMMON_CFLAGS += -O$(OPT)
COMMON_CFLAGS += $(CDIALECTOPT)
COMMON_CFLAGS += $(WARNINGS)
COMMON_CFLAGS += $(patsubst %,-I%,$(INCLUDE_DIRS))
COMMON_CFLAGS += $(CSTANDARD)
COMMON_CFLAGS += $(ADRDEFS)
CFLAGS = $(COMMON_CFLAGS) -Wa,-adhlns=$(<:%.c=$(OBJDIR)/%.lst)

#---------------- Assembler Options ----------------
#  -Wa,...:   tell GCC to pass this to the assembler.
#  -adhlns:   create listing
#  -gstabs:   have the assembler create line number information; note that
#             for use in COFF files, additional information about filenames
#             and function names needs to be present in the assembler source
#             files -- see avr-libc docs [FIXME: not yet described there]
#  -listing-cont-lines: Sets the maximum number of continuation lines of hex
#       dump that will be displayed for a given single line of source input.
ASFLAGS = $(ADEFS) -Wa,-adhlns=$(<:%.S=$(OBJDIR)/%.lst),-gstabs,--listing-cont-lines=100

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = $(ARCHFLAGS) -I. $(CFLAGS) $(GENDEPFLAGS) $(DEFINES_CFLAGS)
ALL_ASFLAGS = $(ARCHFLAGS) -I. -x assembler-with-cpp $(ASFLAGS)

## Files
# Name of library file
LIBNAME=lib$(TARGET)
FILE_MAIN=main
DIR_EEPROM=hexEeprom
DIR_FLASHEFB=hexFlashEfb
DIR_FUSES=hexFuses
DIR_LOCKBITS=hexLockbits
DIR_SIG=hexSignature

## Checks
# Expected fuses low byte
EXPECT_FUSES_LOW=FF

# Dependencies support
SUFFIXES += .d

#---------------- Library Options ----------------
#---------------- External Memory Options ----------------

# 64 KB of external RAM, starting after internal RAM (ATmega128!),
# used for variables (.data/.bss) and heap (malloc()).
#EXTMEMOPTS = -Wl,-Tdata=0x801100,--defsym=__heap_end=0x80ffff

# 64 KB of external RAM, starting after internal RAM (ATmega128!),
# only used for heap (malloc()).
#EXTMEMOPTS = -Wl,--section-start,.data=0x801100,--defsym=__heap_end=0x80ffff

#EXTMEMOPTS =

#---------------- Linker Options ----------------
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -Wl,-Map=$(patsubst %elf,%map, $@),--cref
LDFLAGS += $(EXTMEMOPTS)

# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_BEGIN = -------- Building \'$(TARGET)\' on platform \'$(platform)\' --------
MSG_END = --------  Build successful  --------
MSG_SIZE_BEFORE = Size before:
MSG_SIZE_AFTER = Size after:
MSG_COFF = Converting to AVR COFF:
MSG_EXTENDED_COFF = Converting to AVR Extended COFF:
MSG_FLASH = Creating load file for Flash:
MSG_EEPROM = Creating load file for EEPROM:
MSG_SIG = Creating signature file:
MSG_FLASH = Creating load file for Flash data:
MSG_EXTENDED_LISTING = Creating Extended Listing:
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_LINKING = Linking:
MSG_COMPILING = Compiling C:
MSG_COMPILING_ASM = Compiling ASM:
MSG_ASSEMBLING = Assembling:
MSG_CLEANING = Cleaning project:
MSG_CREATING_LIBRARY = Creating library:
