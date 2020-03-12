
#  Project Name
PROJECT=main

#  Type of CPU/MCU in target hardware
CPU = cortex-m4

#  Build the list of object files needed.  All object files will be built in
#  the working directory, not the source directories.
#
#  You will need as a minimum your $(PROJECT).o file.
#  You will also need code for startup (following reset) and
#  any code needed to get the PLL configured.
OBJECTS    = $(PROJECT).o \
          startup_MK64F12.o \
		  system_MK64F12.o \
		  led.o \
		  uart.o

#  Select the target type.  This is typically arm-none-eabi.
#  If your toolchain supports other targets, those target
#  folders should be at the same level in the toolchain as
#  the arm-none-eabi folders.
TARGETTYPE = arm-none-eabi


#  All possible source directories other than '.' must be defined in
#  the VPATH variable.  This lets make tell the compiler where to find
#  source files outside of the working directory.  If you need more
#  than one directory, separate their paths with ':'.

               
#  List of directories to be searched for include files during compilation
INCDIRS = -ICMSIS/Include


# Name and path to the linker script
LSCRIPT = linker.lds


OPTIMIZATION = 2
DEBUG = -g


#  Compiler options
GCFLAGS = -Wall -Werror -Wextra -std=gnu99 -fno-common -mcpu=$(CPU) -mthumb -O$(OPTIMIZATION) $(DEBUG) -mfloat-abi=hard -mfpu=fpv4-sp-d16
GCFLAGS += $(INCDIRS)

# You can uncomment the following line to create an assembly output
# listing of your C files.  If you do this, however, the sed script
# in the compilation below won't work properly.
# GCFLAGS += -c -g -Wa,-a,-ad


#  Assembler options
ASFLAGS = -mcpu=$(CPU)

# Uncomment the following line if you want an assembler listing file
# for your .s files.  If you do this, however, the sed script
# in the assembler invocation below won't work properly.
#ASFLAGS += -alhs


#  Linker options
LDFLAGS  = -nostdlib -nostartfiles -Map=$(PROJECT).map -T$(LSCRIPT)


#  Tools paths
#
#  Define an explicit path to the GNU tools used by make.
#  If you are ABSOLUTELY sure that your PATH variable is
#  set properly, you can remove the BINDIR variable.
#


CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
AR = arm-none-eabi-ar
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size
OBJDUMP = arm-none-eabi-objdump

#  Define a command for removing folders and files during clean.  The
#  simplest such command is Linux' rm with the -f option.  You can find
#  suitable versions of rm on the web.
REMOVE = rm -f

#########################################################################

all:: $(PROJECT).hex $(PROJECT).bin stats dump

$(PROJECT).bin: $(PROJECT).elf
	$(OBJCOPY) -O binary -j .text -j .data $(PROJECT).elf $(PROJECT).bin

$(PROJECT).hex: $(PROJECT).elf
	$(OBJCOPY) -R .stack -O ihex $(PROJECT).elf $(PROJECT).hex

#  Linker invocation
$(PROJECT).elf: $(OBJECTS)
	$(LD) $(OBJECTS) $(LDFLAGS) -o $(PROJECT).elf

stats: $(PROJECT).elf
	$(SIZE) $(PROJECT).elf
   
dump: $(PROJECT).elf
	$(OBJDUMP) -h $(PROJECT).elf   

clean:
	$(REMOVE) *.o
	$(REMOVE) $(PROJECT).hex
	$(REMOVE) $(PROJECT).elf
	$(REMOVE) $(PROJECT).map
	$(REMOVE) $(PROJECT).bin
	$(REMOVE) *.lst

#  The toolvers target provides a sanity check, so you can determine
#  exactly which version of each tool will be used when you build.
#  If you use this target, make will display the first line of each
#  tool invocation.
#  To use this feature, enter from the command-line:
#    make -f $(PROJECT).mak toolvers
toolvers:
	$(CC) --version | sed q
	$(AS) --version | sed q
	$(LD) --version | sed q
	$(AR) --version | sed q
	$(OBJCOPY) --version | sed q
	$(SIZE) --version | sed q
	$(OBJDUMP) --version | sed q
   
#########################################################################
#  Default rules to compile .c and .cpp file to .o
#  and assemble .s files to .o

#  There are two options for compiling .c files to .o; uncomment only one.
#  The shorter option is suitable for making from the command-line.
#  The option with the sed script on the end is used if you want to
#  compile from Visual Studio; the sed script reformats error messages
#  so Visual Studio's IntelliSense feature can track back to the source
#  file with the error.
.c.o :
	@echo Compiling $<, writing to $@...
#    $(CC) $(GCFLAGS) -c $< -o $@ > $(basename $@).lst
	$(CC) $(GCFLAGS) -c $< -o $@ 2>&1 | sed -e 's/\(\w\+\):\([0-9]\+\):/\1(\2):/'
   

#  There are two options for assembling .s files to .o; uncomment only one.
#  The shorter option is suitable for making from the command-line.
#  The option with the sed script on the end is used if you want to
#  compile from Visual Studio; the sed script reformats error messages
#  so Visual Studio's IntelliSense feature can track back to the source
#  file with the error.
.s.o :
	@echo Assembling $<, writing to $@...
#    $(AS) $(ASFLAGS) -o $@ $<  > $(basename $@).lst
	$(AS) $(ASFLAGS) -o $@ $<  2>&1 | sed -e 's/\(\w\+\):\([0-9]\+\):/\1(\2):/'
#########################################################################