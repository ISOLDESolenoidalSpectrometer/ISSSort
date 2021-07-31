// My code include.
#include "Settings.hh"
#include "Calibration.hh"
#include "Converter.hh"
#include "TimeSorter.hh"
#include "EventBuilder.hh"
#include "Reaction.hh"
#include "Histogrammer.hh"

// ROOT include.
#include <TTree.h>
#include <TFile.h>
#include <THttpServer.h>
#include <TThread.h>

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


// Default parameters and name
std::string output_name;
std::string datadir_name = "/eos/experiment/isolde-iss/2021/";
std::string name_set_file;
std::string name_cal_file;
std::string name_react_file;
std::vector<std::string> input_names;

// a flag at the input to force the conversion
bool flag_convert = false;

// select what steps of the analysis to be forced
std::vector<bool> force_convert;
bool force_sort = false;

// Flag for somebody needing help on command line
bool help_flag = false;

// Monitoring input file
bool flag_monitor = false;
int mon_time = -1; // update time in seconds

// Settings file
Settings *set;

// Calibration file
Calibration *cal;
bool overwrite_cal = false;

// Reaction file
Reaction *react;

// Server and controls for the GUI
THttpServer *serv;
Bool_t bRunMon = kTRUE;
Bool_t bFirstRun = kTRUE;
string curFileMon;
int port_num = 8030;

// Function to call the monitoring loop
void* monitor_run( void* ptr ){
//void monitor_run(){
	
	// This function is called to run when monitoring
	Converter conv_mon( set );
	TimeSorter sort_mon;
	EventBuilder eb_mon( set );

	// Data/Event counters
	int start_block = 0;
	int nblocks = 0;
	//unsigned long start_calib = 0;
	//unsigned long ncalib = 0;
	unsigned long start_sort = 0;
	unsigned long nsort = 0;
	unsigned long start_build = 0;
	unsigned long nbuild = 0;

	// Converter setup
	curFileMon = input_names.at(0); // maybe change in GUI later?
	conv_mon.AddCalibration( cal );
	conv_mon.SetOutput( "monitor_singles.root" );
	conv_mon.MakeTree();
	conv_mon.MakeHists();

	while( bRunMon ) {
		
		// Lock the main thread
		//TThread::Lock();
		
		// Convert
		nblocks = conv_mon.ConvertFile( curFileMon, start_block );
		start_block = nblocks;
		
		// Sort
		if( bFirstRun ) {
			sort_mon.SetInputTree( conv_mon.GetTree() );
			sort_mon.SetOutput( "monitor_sort.root" );
		}
		nsort = sort_mon.SortFile( start_sort );
		start_sort = nsort;
		
		// Event builder
		if( bFirstRun ) {
			eb_mon.SetInputTree( sort_mon.GetTree() );
			eb_mon.SetOutput( "monitor_events.root" );
		}
		nbuild = eb_mon.BuildEvents( start_build );
		start_build = nbuild;
		
		// If this was the first time we ran, do stuff?
		if( bFirstRun ) {
			
			bFirstRun = kFALSE;
			
		}
		
		// Now we can unlock the main thread again
		//TThread::UnLock();

		// Update the Canvas, but not when in a thread
		//gSystem->ProcessEvents();

		// This makes things unresponsive!
		// Unless we are threading?
		gSystem->Sleep( mon_time * 1e3 );

	}
	
	conv_mon.CloseOutput();
	//calib_mon.CloseOutput();
	sort_mon.CloseOutput();
	eb_mon.CloseOutput();

	return 0;
	
}

//void* start_http( void* ptr ){
void start_http(){

	// Server for JSROOT
	string server_name = "http:" + to_string(port_num) + "?top=ISSDAQMonitoring";
	serv = new THttpServer( server_name.data() );
	serv->SetReadOnly(kFALSE);

	// enable monitoring and
	// specify items to draw when page is opened
	serv->SetItemField("/","_monitoring","5000");
	serv->SetItemField("/","_layout","grid2x2");
	//serv->SetItemField("/","_drawitem","[hpxpy,hpx,Debug]");
	//serv->SetItemField("/","_drawopt","col");
	
	// register simple start/stop commands
	serv->RegisterCommand("/Start", "bRunMon=kTRUE;", "button;/usr/share/root/icons/ed_execute.png");
	serv->RegisterCommand("/Stop",  "bRunMon=kFALSE;", "button;/usr/share/root/icons/ed_interrupt.png");

	// hide commands so the only show as buttons
	serv->Hide("/Start");
	serv->Hide("/Stop");

	// Add data directory
	if( datadir_name.size() > 0 ) serv->AddLocation( "data/", datadir_name.data() );
	
	return;
	
}

int main( int argc, char *argv[] ){
	
	// Command line interface, stolen from MiniballCoulexSort
	CommandLineInterface *interface = new CommandLineInterface();

	interface->Add("-i", "List of input files", &input_names );
	interface->Add("-m", "Monitor input file every X seconds", &mon_time );
	interface->Add("-p", "Port number for web server (default 8030)", &port_num );
	interface->Add("-o", "Output file for events tree", &output_name );
	interface->Add("-d", "Data directory to add to the monitor", &datadir_name );
	interface->Add("-f", "Flag to force new ROOT conversion", &flag_convert );
	interface->Add("-s", "Settings file", &name_set_file );
	interface->Add("-c", "Calibration file", &name_cal_file );
	interface->Add("-r", "Reaction file", &name_react_file );
	interface->Add("-h", "Print this help", &help_flag );

	interface->CheckFlags( argc, argv );
	if( help_flag ) {
		
		interface->CheckFlags( 1, argv );
		return 0;
		
	}

	// Check we have data files
	if( !input_names.size() ) {
			
			std::cout << "You have to provide at least one input file!" << std::endl;
			return 1;
			
	}
	
	// Check if we should be monitoring the input
	if( mon_time > 0 && input_names.size() == 1 ) {
		
		flag_monitor = true;
		std::cout << "Running iss_sort in a loop every " << mon_time;
		std::cout << " seconds\nMonitoring " << input_names.at(0) << std::endl;
		
	}
	
	else if( mon_time > 0 && input_names.size() != 1 ) {
		
		flag_monitor = false;
		std::cout << "Cannot monitor multiple input files, switching to normal mode" << std::endl;
				
	}
	
	// Check the ouput file name
	if( output_name.length() == 0 ) {
	
		if( input_names.size() == 1 ) output_name = input_names.at(0) + "_events.root";
		else output_name = input_names.at(0).substr( 0, input_names.at(0).find_last_of("/") ) + "_events.root";
		
	}
	
	// Check we have a Settings file
	if( name_set_file.length() > 0 ) {
		
		std::cout << "Settings file: " << name_set_file << std::endl;
		
	}
	else {
		
		std::cout << "No settings file provided. Using defaults." << std::endl;
		name_set_file = "dummy";

	}
	
	// Check we have a calibration file
	if( name_cal_file.length() > 0 ) {
		
		std::cout << "Calibration file: " << name_cal_file << std::endl;
		overwrite_cal = true;

	}
	else {
		
		std::cout << "No calibration file provided. Using defaults." << std::endl;
		name_cal_file = "dummy";

	}
	
	// Check we have a reaction file
	if( name_react_file.length() > 0 ) {
		
		std::cout << "Reaction file: " << name_react_file << std::endl;
		
	}
	else {
		
		std::cout << "No reaction file provided. Using defaults." << std::endl;
		name_react_file = "dummy";

	}
	
	set = new Settings( name_set_file );
	cal = new Calibration( name_cal_file, set );
	react = new Reaction( name_react_file );


	
	//-------------------//
	// Online monitoring //
	//-------------------//
	if( flag_monitor ) {
		
		// Thread for the HTTP server
		//TThread *th = new TThread( "http_server", start_http, (void*)nullptr );
		//th->Run();

		// Start the HTTP server from the main thread (should usually do this)
		start_http();
		
		// Thread for the monitor process
		TThread *th = new TThread( "monitor", monitor_run, (void*)nullptr );
		th->Run();
		
		// Just call monitor process without threading
		//monitor_run();
		
		// wait until we finish
		while( bRunMon ){
			
			gSystem->Sleep(100);
			gSystem->ProcessEvents();
			
		}
		
		return 0;
		
	}


	

	//------------------------//
	// Run conversion to ROOT //
	//------------------------//
	Converter conv( set );
	std::cout << "\n +++ ISS Analysis:: processing Converter +++" << std::endl;

	TFile *rtest;
	std::ifstream ftest;
	std::string name_input_file;
	std::string name_output_file;
	
	// Check each file
	for( unsigned int i = 0; i < input_names.size(); i++ ){
			
		name_input_file = input_names.at(i);
		name_output_file = input_names.at(i) + ".root";
		
		force_convert.push_back( false );

		// If it doesn't exist, we have to convert it anyway
		// The convert flag will force it to be converted
		ftest.open( name_output_file.data() );
		if( !ftest.is_open() ) force_convert.at(i) = true;
		else {
			
			ftest.close();
			rtest = new TFile( name_output_file.data() );
			if( rtest->IsZombie() ) force_convert.at(i) = true;
			if( !flag_convert && !force_convert.at(i) )
				std::cout << name_output_file << " already converted" << std::endl;
			rtest->Close();
			
		}

		if( flag_convert || force_convert.at(i) ) {
			
			std::cout << name_input_file << " --> ";
			std::cout << name_output_file << std::endl;
			
			conv.SetOutput( name_output_file );
			conv.MakeTree();
			conv.MakeHists();
			conv.AddCalibration( cal );
			conv.ConvertFile( name_input_file );
			conv.CloseOutput();

		}
		
	}
		
	
	//-------------------------//
	// Do time sorting of data //
	//-------------------------//
	TimeSorter sort;
	std::cout << "\n +++ ISS Analysis:: processing TimeSorter +++" << std::endl;
	
	// Check each file
	for( unsigned int i = 0; i < input_names.size(); i++ ){
			
		name_input_file = input_names.at(i) + ".root";
		name_output_file = input_names.at(i) + "_sort.root";


		// We need to time sort it if we just converted it
		if( flag_convert || force_convert.at(i) )
			force_sort = true;
			
		// If it doesn't exist, we have to sort it anyway
		else {
			
			ftest.open( name_output_file.data() );
			if( !ftest.is_open() ) force_sort = true;
			else {
				
				ftest.close();
				rtest = new TFile( name_output_file.data() );
				if( rtest->IsZombie() ) force_sort = true;
				if( !force_sort )
					std::cout << name_output_file << " already sorted" << std::endl;
				rtest->Close();
				
			}
			
		}

		if( force_sort ) {
		
			std::cout << name_input_file << " --> ";
			std::cout << name_output_file << std::endl;
			
			sort.SetInputFile( name_input_file );
			sort.SetOutput( name_output_file );
			sort.SortFile();
			sort.CloseOutput();

			force_sort = false;

		}
	
	}
	
	
	//-----------------------//
	// Physics event builder //
	//-----------------------//
	EventBuilder eb( set );
	std::cout << "\n +++ ISS Analysis:: processing EventBuilder +++" << std::endl;

	eb.SetOutput( output_name );
	std::vector<string> name_event_files;
	
	// Update calibration file if given
	if( overwrite_cal ) eb.AddCalibration( cal );

	// We are going to chain all the files now
	for( unsigned int i = 0; i < input_names.size(); i++ ){
			
		name_input_file = input_names.at(i) + "_sort.root";
		name_event_files.push_back( name_input_file );
	
	}
	eb.SetInputFile( name_event_files );

	// Then build events
	eb.BuildEvents();
	
	
	//------------------------------//
	// Finally make some histograms //
	//------------------------------//
	Histogrammer hist( react, set );
	hist.SetInputTree( eb.GetTree() );

	std::cout << "\n +++ ISS Analysis:: processing Histogrammer +++" << std::endl;

	std::string output_name_hists = output_name.substr( 0, output_name.find_last_of("_") );
	output_name_hists += "_hists.root";
	hist.SetOutput( output_name_hists.data() );
	hist.FillHists();
	hist.CloseOutput();

	
	// Now we can close the event builder file...
	// We need to keep it open during the histogramming
	// because I am using a pointer to the tree in memory
	eb.CloseOutput();
	
	cout << "Finished!\n";
			
	return 0;
	
}
