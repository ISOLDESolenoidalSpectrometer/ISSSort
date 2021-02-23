# Makefile for Midas to Root for ISS
.PHONY: clean all

BIN_DIR     := ./bin
SRC_DIR     := ./src
LIB_DIR     := ./lib
INC_DIR     := ./include

ROOTVER     := $(shell root-config --version | head -c1)
ifeq ($(ROOTVER),5)
	ROOTDICT  := rootcint
	DICTEXT   := .h
else
	ROOTDICT  := rootcling
	DICTEXT   := _rdict.pcm
endif

ROOTCFLAGS   := $(shell root-config --cflags)
ROOTLDFLAGS  := $(shell root-config --ldflags)
ROOTLIBS     := $(shell root-config --glibs)
LIBS         := $(ROOTLIBS)

# Compiler.
CC          = $(shell root-config --cxx)
# Flags for compiler.
CFLAGS		= -c -Wall -Wextra $(ROOTCFLAGS) -g
INCLUDES	+= -I$(INC_DIR) -I.
# Flags for linker.
LDFLAGS 	+= $(ROOTLDFLAGS) $(ROOTLIBS)

# The object files.
OBJECTS =  		$(SRC_DIR)/Calibration.o \
				$(SRC_DIR)/Common.o \
				$(SRC_DIR)/CommandLineInterface.o \
				$(SRC_DIR)/Converter.o \
				$(SRC_DIR)/ISSEvts.o \
				$(SRC_DIR)/TimeSorter.o \
				$(SRC_DIR)/EventBuilder.o
 
# The header files.
DEPENDENCIES =  $(INC_DIR)/Calibration.hh \
				$(INC_DIR)/Common.hh \
				$(INC_DIR)/CommandLineInterface.hh \
				$(INC_DIR)/Converter.hh \
				$(INC_DIR)/ISSEvts.hh \
				$(INC_DIR)/TimeSorter.hh \
				$(INC_DIR)/EventBuilder.hh
 

$(BIN_DIR)/iss_sort: iss_sort.o $(OBJECTS) iss_sortDict.o
	mkdir -p $(BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

iss_sort.o: iss_sort.cc
	$(CC) $(CFLAGS) $(INCLUDES) $^

$(SRC_DIR)/Calibration.o: $(SRC_DIR)/Calibration.cc $(INC_DIR)/Calibration.hh
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(SRC_DIR)/Common.o: $(SRC_DIR)/Common.cc $(INC_DIR)/Common.hh
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(SRC_DIR)/CommandLineInterface.o: $(SRC_DIR)/CommandLineInterface.cc $(INC_DIR)/CommandLineInterface.hh
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(SRC_DIR)/Converter.o: $(SRC_DIR)/Converter.cc $(INC_DIR)/Converter.hh
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(SRC_DIR)/EventBuilder.o: $(SRC_DIR)/EventBuilder.cc $(INC_DIR)/EventBuilder.hh $(SRC_DIR)/ISSEvts.o
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(SRC_DIR)/ISSEvts.o: $(SRC_DIR)/ISSEvts.cc $(INC_DIR)/ISSEvts.hh
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(SRC_DIR)/TimeSorter.o: $(SRC_DIR)/TimeSorter.cc $(INC_DIR)/TimeSorter.hh $(SRC_DIR)/Calibration.o
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

iss_sortDict.o: iss_sortDict.cc iss_sortDict$(DICTEXT) $(INC_DIR)/RootLinkDef.h
	mkdir -p $(BIN_DIR)
	$(CC) -fPIC $(CFLAGS) $(INCLUDES) -c $<
	cp iss_sortDict$(DICTEXT) $(BIN_DIR)/

iss_sortDict.cc: $(DEPENDENCIES) $(INC_DIR)/RootLinkDef.h
	$(ROOTDICT) -f $@ -c $(INCLUDES) $(DEPENDENCIES) $(INC_DIR)/RootLinkDef.h


clean:
	rm -vf $(BIN_DIR)/iss_sort $(SRC_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*.gch *.o $(BIN_DIR)/*.pcm *.pcm $(BIN_DIR)/*Dict* *Dict*
