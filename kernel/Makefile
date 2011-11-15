# kernel/Makefile


# Find the CPU architecture
CPU = $(shell grep '\#define CONFIG_CPU' ../config.h | cut -c 20-)

# Address at which the kernel will be loaded
ifndef KERNEL_START
KERNEL_START = 0
endif

# Sources
SOURCES = $(wildcard src/_*.c src/[^_]*.c src/*.S)
ifndef OBJDIR
OBJDIR= obj
endif 
OBJECTS = $(patsubst src/%.S, $(OBJDIR)/%.o, \
	$(patsubst src/%.c, $(OBJDIR)/%.o, $(SOURCES)))

# Used programs
AS = as
CC = gcc
CPP = cpp
LD = ld
RM = rm
STRIP = strip
M4 = m4

# Compilation flags: warn, no default
CFLAGS = -W -Winline -Wall -Werror -Wno-unused \
	-fno-builtin -nostdinc -nostdlib -ffreestanding -pipe
AFLAGS =
LDFLAGS = -N -nostdlib -Ttext $(KERNEL_START) --entry=init

ifeq ($(CPU),IA32)
CFLAGS += -m32
AFLAGS += --32
LDFLAGS += -melf_i386 -T kernel32.lds
else
LDFLAGS += -T kernel64.lds
endif

ifdef DEBUG
CFLAGS += -ggdb
AFLAGS += -g
LDFLAGS += -g
endif

# Directory for headers
INCLUDE = -I ./include -include ../config.h

# Target
KERNEL = kernel

.PHONY: clean


# Create the kicker
$(KERNEL): $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

# Assembly sources are passed first to the CPP preprocessor then to GNU As
$(OBJDIR)/%.o: src/%.S
	@if [ ! -d $(OBJDIR) ] ; then mkdir $(OBJDIR); fi
	$(CPP) $(INCLUDE) $< | $(AS) $(AFLAGS) -o $@

# All the C sources are passed to GCC
$(OBJDIR)/%.o: src/%.c
	@if [ ! -d $(OBJDIR) ] ; then mkdir $(OBJDIR); fi
	$(CC) $(CFLAGS) -c $(INCLUDE) -o $@ $<

# M4 generated sources
src/%: src/%.m4
	$(M4) $< > $@

# Remove everything created by make
clean:
	$(RM) -fr $(KERNEL) *~ */*~ $(OBJDIR) *.bak
