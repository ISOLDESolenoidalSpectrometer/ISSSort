# Makefile for Midas to Root for ISS
.PHONY: clean all

ROOTVER     := $(shell root-config --version | head -c1)

ifeq ($(ROOTVER),5)
	ROOTCINT  := rootcint
	DICTEXT   := .h
else
	ROOTCINT  := rootcling
	DICTEXT   := _rdict.pcm
endif

# Compiler.
CC          	= $(shell root-config --cxx)
# Flags for compiler.
CFLAGS		= -c -Wall -Wextra
# Flags for linker.
LDFLAGS		= -lrt

BIN_DIR     := ./bin
LIB_DIR     := ./lib
INC_DIR     := ./include

ROOTCFLAGS   := $(shell root-config --cflags)
ROOTLDFLAGS  := $(shell root-config --ldflags)
ROOTLIBS     := $(shell root-config --glibs)
LIBS         := $(ROOTLIBS)

# Add root flags.
CFLAGS 		+= $(ROOTCFLAGS) -I$(INC_DIR)
# Add root.
LDFLAGS 	+= $(ROOTLDFLAGS) $(ROOTLIBS)

# The object files.
OBJECTS =   Common.o \
			Converter.o \
			TimeSorter.o \
			Calibrator.o \
			EventBuilder.o \
			DutClass.o \
			MyStyle.o
 
#ANALYSIS =  AnalysisPulse.o  

#OBJECTS += $(ANALYSIS)

# Specify the file to search for .cc/.h files.
vpath %.cc ./src
vpath %.h ./include

midas2root : midas2root.o $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^
#	$(CC) $(LDFLAGS) $(LIBS) -o $@ $^

midas2root.o : midas2root.cc
	$(CC) $(CFLAGS) $^

%.o: %.cc %.hh
	$(CPP) $(CFLAGS) $(INCLUDES) -c $< -o $@

Common.o: Common.h Common.cc
	$(CC) $(CFLAGS) $^

Converter.o : Converter.h Converter.cc 
	$(CC) $(CFLAGS) $^

TimeSorter.o : TimeSorter.h TimeSorter.cc 
	$(CC) $(CFLAGS) $^

Calibrator.o : Calibrator.h Calibrator.cc 
	$(CC) $(CFLAGS) $^

EventBuilder.o : EventBuilder.h EventBuilder.cc 
	$(CC) $(CFLAGS) $^

DutClass.o :  DutClass.h DutClass.cc
	$(CC) $(CFLAGS) $^

MyStyle.o : MyStyle.h MyStyle.cc
	$(CC) $(CFLAGS) $^

clean:
	rm -vf *.exe *.o *~ include/*.gch
