# servers/common/Makefile
# Standard Makefile for Esperanza servers.
# To use it, simply link it inside the server directory.


# Find the CPU architecture
CPU = $(shell grep '\#define CONFIG_CPU' ../../config.h | cut -c 20-)

# Address at which the Kicker will be loaded
ifndef SERVER_START
ifeq ($(CPU),IA32)
SERVER_START = 0x8048000
else
SERVER_START = 0x400000
endif
endif

# Sources
SOURCES = ../common/crt0.S $(wildcard src/*.c src/*.S)
ifndef OBJDIR
OBJDIR= obj
endif 
OBJECTS = $(patsubst ../common/%.S, $(OBJDIR)/%.o,			\
		$(patsubst  ../common/%.c, $(OBJDIR)/%.o,		\
		$(patsubst src/%.S, $(OBJDIR)/%.o,			\
		$(patsubst src/%.c, $(OBJDIR)/%.o, $(SOURCES)))))

# Used programs
AS = as
CC = gcc
CPP = cpp
LD = ld
RM = rm
STRIP = strip

# Compilation flags: warn, no default
CFLAGS = -W -Winline -Wall -Werror -Wno-unused \
	-fno-builtin -nostdinc -nostdlib -ffreestanding -pipe -mno-sse

AFLAGS =
LDFLAGS = -N -nostdlib -Ttext $(SERVER_START)

ifeq ($(CPU),IA32)
CFLAGS += -m32
AFLAGS += --32
LDFLAGS += -melf_i386
endif

# Directory for headers
INCLUDE = -Iinclude -I../include -include ../../config.h

# Target
TARGET = $(shell basename `pwd`)

.PHONY: clean

all: $(TARGET) $(TARGET)-elf

# Create the server
$(TARGET): $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) --oformat binary -o $@ $(OBJECTS)

# Create the server ELF object
$(TARGET)-elf: $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

# Assembly sources are passed first to the CPP preprocessor then to GNU As
$(OBJDIR)/%.o: src/%.S
	@if [ ! -d $(OBJDIR) ] ; then mkdir $(OBJDIR); fi
	$(CPP) $(INCLUDE) $< | $(AS) $(AFLAGS) -o $@
$(OBJDIR)/%.o: ../common/%.S
	@if [ ! -d $(OBJDIR) ] ; then mkdir $(OBJDIR); fi
	$(CPP) $(INCLUDE) $< | $(AS) $(AFLAGS) -o $@

# All the C sources are passed to GCC
$(OBJDIR)/%.o: src/%.c
	@if [ ! -d $(OBJDIR) ] ; then mkdir $(OBJDIR); fi
	$(CC) $(CFLAGS) -c $(INCLUDE) -o $@ $<
$(OBJDIR)/%.o: ../common/%.c
	@if [ ! -d $(OBJDIR) ] ; then mkdir $(OBJDIR); fi
	$(CC) $(CFLAGS) -c $(INCLUDE) -o $@ $<

# Remove everything created by make
clean:
	$(RM) -fr $(TARGET) *~ */*~ $(OBJDIR) *.bak
