// ROOT include.
#include <TROOT.h>
#include <TTree.h>
#include <TFile.h>
#include <THttpServer.h>
#include <TThread.h>
#include <TGClient.h>
#include <TApplication.h>

// C++ include.
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <sstream>

// Command line interface
#ifndef __COMMAND_LINE_INTERFACE
# include "CommandLineInterface.hh"
#endif


// Some compiler things
#ifndef CURDIR
# define CURDIR "./"
#endif
#ifndef AMEDIR
# define AMEDIR "./data"
#endif
#ifndef SRIMDIR
# define SRIMDIR "./srim"
#endif

Bool_t bRunMon = kTRUE;
Bool_t bFirstRun = kTRUE;
std::string curFileMon;

// Default parameters and name
std::string output_name;
std::string datadir_name;
std::string name_set_file;
std::string name_cal_file;
std::string name_react_file;
std::string name_autocal_file;
std::vector<std::string> input_names;

// a flag at the input to force or not the conversion
bool flag_convert = false;
bool flag_events = false;
bool flag_source = false;
bool flag_autocal = false;
bool flag_data = false;
bool flag_pace4 = false;
bool flag_nptool = false;

// select what steps of the analysis to be forced
std::vector<bool> force_convert;
bool force_sort = false;
bool force_events = false;

// Flag if we want to launch the GUI for sorting
bool gui_flag = false;

// Flag for somebody needing help on command line
bool help_flag = false;

// DataSpy
bool flag_spy = false;
int open_spy_data = -1;

// Monitoring input file
bool flag_monitor = false;
int mon_time = -1; // update time in seconds

// Settings file
ISSSettings *myset;

// Calibration file
ISSCalibration *mycal;
bool overwrite_cal = false;

// Reaction file
ISSReaction *myreact;

// Struct for passing to the thread
typedef struct thptr {
	
	ISSCalibration *mycal;
	ISSSettings *myset;
	ISSReaction *myreact;
	
} thread_data;

// Server and controls for the GUI
THttpServer *serv;
int port_num = 8030;

// Pointers to the thread events TODO: sort out inhereted class stuff
std::shared_ptr<ISSConverter> conv_mon;
std::shared_ptr<ISSEventBuilder> eb_mon;
std::shared_ptr<ISSHistogrammer> hist_mon;

void reset_conv_hists();
void reset_evnt_hists();
void reset_phys_hists();
void stop_monitor();
void start_monitor();
