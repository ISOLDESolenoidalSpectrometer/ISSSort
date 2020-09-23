# Makefile for Midas to Root for ISS
.PHONY: clean all

BIN_DIR     := ./bin
SRC_DIR     := ./src
LIB_DIR     := ./lib
INC_DIR     := ./include

ROOTVER     := $(shell root-config --version | head -c1)
ifeq ($(ROOTVER),5)
	ROOTCINT  := rootcint
	DICTEXT   := .h
else
	ROOTCINT  := rootcling
	DICTEXT   := _rdict.pcm
endif

ROOTCFLAGS   := $(shell root-config --cflags)
ROOTLDFLAGS  := $(shell root-config --ldflags)
ROOTLIBS     := $(shell root-config --glibs)
LIBS         := $(ROOTLIBS)

# Compiler.
CC          = $(shell root-config --cxx)
# Flags for compiler.
CFLAGS		= -c -Wall -Wextra $(ROOTCFLAGS)
INCLUDES	+= -I$(INC_DIR) -I.
# Flags for linker.
LDFLAGS 	+= $(ROOTLDFLAGS) $(ROOTLIBS)

# The object files.
OBJECTS =   $(SRC_DIR)/Common.o \
			$(SRC_DIR)/Converter.o \
			$(SRC_DIR)/TimeSorter.o \
			$(SRC_DIR)/Calibrator.o \
			$(SRC_DIR)/EventBuilder.o \
			$(SRC_DIR)/DutClass.o \
			$(SRC_DIR)/MyStyle.o
 
#ANALYSIS =  AnalysisPulse.o  
#OBJECTS += $(ANALYSIS)

midas2root : midas2root.o $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^
#	$(CC) $(LDFLAGS) $(LIBS) -o $@ $^

midas2root.o : midas2root.cc
	$(CC) $(CFLAGS) $(INCLUDES) $^
	
$(SRC_DIR)/%.o: $(SRC_DIR)/%.cc $(INC_DIR)/%.hh
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -vf midas2root $(SRC_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*.gch
