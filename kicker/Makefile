# kicker/Makefile


KICKER_START = 0x100000


# Sources
SOURCES = $(wildcard src/*.S src/*.c)
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

# Compilation flags: warn as much as possible, no default, 32bits
CFLAGS = -W -Winline -Wall -Wno-unused -Werror \
	-nostdinc -nostdlib -fno-builtin -ffreestanding \
	-m32 -pipe -finline-functions -mno-sse
AFLAGS = --32
LDFLAGS = -nostdlib -Ttext $(KICKER_START) -melf_i386

# Directory for headers
INCLUDE = -I ./include -include ../config.h

# Target
KICKER = kicker

.PHONY: clean


# Create the kicker
$(KICKER): $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)
#	$(STRIP) --strip-unneeded $@

# Assembly sources are passed first to the CPP preprocessor then to GNU AS
$(OBJDIR)/%.o: src/%.S
	@if [ ! -d $(OBJDIR) ] ; then mkdir $(OBJDIR); fi
	$(CPP) $(INCLUDE) $< | $(AS) $(AFLAGS) -o $@

# All the C sources are passed to GCC
$(OBJDIR)/%.o: src/%.c
	@if [ ! -d $(OBJDIR) ] ; then mkdir $(OBJDIR); fi
	$(CC) $(CFLAGS) -c $(INCLUDE) -o $@ $<

# Remove everything created by make
clean:
	$(RM) -fr $(KICKER) *~ */*~ $(OBJDIR) *.bak
