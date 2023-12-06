# Makefile for ISSSort
.PHONY: clean all doc

PWD			:= $(shell pwd)
BIN_DIR     := ./bin
SRC_DIR     := ./src
LIB_DIR     := ./lib
INC_DIR     := ./include
DOC_DIR		:= ./doc
CUR_DIR		:= \"$(PWD)/\"
AME_FILE	:= \"$(PWD)/data/mass_1.mas20\"
SRIM_DIR	:= \"$(PWD)/srim/\"

ROOTVER     := $(shell root-config --version | head -c1)
ifeq ($(ROOTVER),5)
	ROOTDICT  := rootcint
	DICTEXT   := .h
else
	ROOTDICT  := rootcling
	DICTEXT   := _rdict.pcm
endif

PLATFORM:=$(shell uname)
ifeq ($(PLATFORM),Darwin)
SHAREDSWITCH = -Qunused-arguments -shared -undefined dynamic_lookup -dynamiclib -Wl,-install_name,'@executable_path/../lib/'# NO ENDING SPACE
OSDEF = -DMACOSX
else
SHAREDSWITCH = -shared -Wl,-soname,# NO ENDING SPACE
OSDEF = -DLINUX
LIBEXTRA = -lrt
endif

# Documentation
DOC			:= doxygen
DOC_FILE	:= Doxyfile
DOC_HTML	:= documentation.html

ROOTCPPFLAGS	:= $(shell root-config --cflags)
ROOTLDFLAGS		:= $(shell root-config --ldflags)
ROOTLIBS		:= $(shell root-config --glibs) -lRHTTP -lThread
LIBS			:= $(ROOTLIBS) $(LIBEXTRA)

# Compiler.
CXX          = $(shell root-config --cxx)
CC           = $(shell root-config --cc)

# Flags for compiler.
CPPFLAGS	 = -c -Wall -Wextra $(ROOTCPPFLAGS) -g -fPIC
CPPFLAGS	+= -DUNIX -DPOSIX $(OSDEF)
INCLUDES	+= -I$(INC_DIR) -I.

# Pass in the data file locations
CPPFLAGS		+= -DAME_FILE=$(AME_FILE)
CPPFLAGS		+= -DSRIM_DIR=$(SRIM_DIR)
CPPFLAGS		+= -DCUR_DIR=$(CUR_DIR)

# Linker.
LD          = $(shell root-config --ld)
# Flags for linker.
LDFLAGS 	+= $(ROOTLDFLAGS) -g

# The object files.
OBJECTS =  		$(SRC_DIR)/AutoCalibrator.o \
				$(SRC_DIR)/Calibration.o \
				$(SRC_DIR)/CommandLineInterface.o \
				$(SRC_DIR)/Converter.o \
				$(SRC_DIR)/DataPackets.o \
				$(SRC_DIR)/DataSpy.o \
				$(SRC_DIR)/Histogrammer.o \
				$(SRC_DIR)/ISSEvts.o \
				$(SRC_DIR)/ISSGUI.o \
				$(SRC_DIR)/Reaction.o \
				$(SRC_DIR)/Settings.o \
				$(SRC_DIR)/EventBuilder.o
 
# The header files.
DEPENDENCIES =  $(INC_DIR)/AutoCalibrator.hh \
				$(INC_DIR)/Calibration.hh \
				$(INC_DIR)/CommandLineInterface.hh \
				$(INC_DIR)/Converter.hh \
				$(INC_DIR)/DataPackets.hh \
				$(INC_DIR)/DataSpy.hh \
				$(INC_DIR)/Histogrammer.hh \
				$(INC_DIR)/ISSEvts.hh \
				$(INC_DIR)/ISSGUI.hh \
				$(INC_DIR)/Reaction.hh \
				$(INC_DIR)/Settings.hh \
				$(INC_DIR)/EventBuilder.hh \
				$(INC_DIR)/FitFunctions.hh

all: $(BIN_DIR)/iss_sort $(LIB_DIR)/libiss_sort.so
 
$(LIB_DIR)/libiss_sort.so: iss_sort.o $(OBJECTS) iss_sortDict.o
	mkdir -p $(LIB_DIR)
	$(LD) iss_sort.o $(OBJECTS) iss_sortDict.o $(SHAREDSWITCH)$@ $(LIBS) -o $@

$(BIN_DIR)/iss_sort: iss_sort.o $(OBJECTS) iss_sortDict.o
	mkdir -p $(BIN_DIR)
	$(LD) -o $@ $^ $(LDFLAGS) $(LIBS)

iss_sort.o: iss_sort.cc
	$(CXX) $(CPPFLAGS) $(INCLUDES) $^

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cc $(INC_DIR)/%.hh
	$(CXX) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

iss_sortDict.o: iss_sortDict.cc iss_sortDict$(DICTEXT) $(INC_DIR)/RootLinkDef.h
	mkdir -p $(BIN_DIR)
	mkdir -p $(LIB_DIR)
	$(CXX) -fPIC $(CPPFLAGS) $(INCLUDES) -c $<
	cp iss_sortDict$(DICTEXT) $(BIN_DIR)/
	cp iss_sortDict$(DICTEXT) $(LIB_DIR)/

iss_sortDict.cc iss_sortDict$(DICTEXT): $(DEPENDENCIES) $(INC_DIR)/RootLinkDef.h
	$(ROOTDICT) -f $@ -c $(INCLUDES) $(DEPENDENCIES) $(INC_DIR)/RootLinkDef.h

clean:
	rm -vf $(BIN_DIR)/iss_sort $(SRC_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*.gch *.o $(BIN_DIR)/*.pcm *.pcm $(BIN_DIR)/*Dict* *Dict* $(LIB_DIR)/*
	
doc:
	mkdir -p $(DOC_DIR)
	$(DOC) $(DOC_FILE)
	ln -sf $(DOC_DIR)/index.html $(DOC_HTML)
	
doc-clean:
	rm -rvf $(DOC_DIR)/* $(DOC_DIR)/.??* $(DOC_HTML) 
