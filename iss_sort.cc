// ============================================================================================= //
/*! \mainpage The ISOLDE Solenoidal Spectrometer (ISS) sort code
* **Author**: Liam Gaffney [(liam.gaffney@liverpool.ac.uk)](mailto:liam.gaffney@liverpool.ac.uk), University of Liverpool
*
* **Documentation and bug fixes**: Patrick MacGregor [(patrick.macgregor@cern.ch)](mailto:patrick.macgregor@cern.ch),
* CERN
*
* See the [GitHub page](https://github.com/ISOLDESolenoidalSpectrometer/ISSSort) for installation 
* instructions, to submit bug reports, and to understand the sorting philosophy. The documentation 
* here focuses on the purpose of different variables and functions. N.B. it's not complete, but
* should become more complete over time.
*/
// ============================================================================================= //

// Settings header
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif

// Calibration header
#ifndef __CALIBRATION_HH
# include "Calibration.hh"
#endif

// Reaction header
#ifndef __REACTION_HH
# include "Reaction.hh"
#endif

// Converter header
#ifndef __CONVERTER_HH
# include "Converter.hh"
#endif

// EventBuilder header
#ifndef __EVENTBUILDER_HH
# include "EventBuilder.hh"
#endif

// Histogrammer header
#ifndef __HISTOGRAMMER_HH
# include "Histogrammer.hh"
#endif

// AutoCalibrator header
#ifndef __AUTOCALIBRATOR_HH
# include "AutoCalibrator.hh"
#endif

// DataSpy header
#ifndef __DATASPY_HH
# include "DataSpy.hh"
#endif

// ISSGUI header
#ifndef __ISSGUI_HH
# include "ISSGUI.hh"
#endif

// Command line interface
#ifndef __COMMAND_LINE_INTERFACE_HH
# include "CommandLineInterface.hh"
#endif

// My code include.
#include "iss_sort.hh"

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
std::shared_ptr<ISSSettings> myset;
bool flag_print_settings = false;

// Calibration file
std::shared_ptr<ISSCalibration> mycal;
bool overwrite_cal = false;

// Reaction file
std::shared_ptr<ISSReaction> myreact;

// Struct for passing to the thread
typedef struct thptr {
	
	std::shared_ptr<ISSCalibration> mycal;
	std::shared_ptr<ISSSettings> myset;
	std::shared_ptr<ISSReaction> myreact;
	
} thread_data;

// Server and controls for the GUI
THttpServer *serv;
int port_num = 8030;

// Pointers to the thread events TODO: sort out inhereted class stuff
std::shared_ptr<ISSConverter> conv_mon;
std::shared_ptr<ISSEventBuilder> eb_mon;
std::shared_ptr<ISSHistogrammer> hist_mon;


void reset_conv_hists(){
	conv_mon->ResetHists();
}

void reset_evnt_hists(){
	eb_mon->ResetHists();
}

void reset_phys_hists(){
	hist_mon->ResetHists();
}

void stop_monitor(){
	bRunMon = kFALSE;
}

void start_monitor(){
	bRunMon = kTRUE;
}

// Function to call the monitoring loop
void* monitor_run( void* ptr ){
	
	/// This function is called to run when monitoring

	// Load macros in thread
	std::string rootline = ".L " + std::string(CUR_DIR) + "include/MonitorMacros.hh";
	gROOT->ProcessLine( rootline.data() );

	// Get the settings, file etc.
	thptr *calfiles = (thptr*)ptr;
	
	// Setup the different steps
	conv_mon = std::make_shared<ISSConverter>( calfiles->myset );
	eb_mon = std::make_shared<ISSEventBuilder>( calfiles->myset );
	hist_mon = std::make_shared<ISSHistogrammer>( calfiles->myreact, calfiles->myset );
	
	// Data blocks for Data spy
	if( flag_spy && myset->GetBlockSize() != 0x10000 ) {
	
		// only 64 kB supported atm
		std::cerr << "Currently only supporting 64 kB block size" << std::endl;
		exit(1);
	
	}
	DataSpy myspy;
	long long buffer[8*1024];
	int file_id = 0; ///> TapeServer volume = /dev/file/<id> ... <id> = 0 on issdaqpc2
	if( flag_spy ) myspy.Open( file_id ); /// open the data spy
	int spy_length = 0;

	// Data/Event counters
	int start_block = 0;
	int nblocks = 0;
	unsigned long nbuild = 0;

	// Converter setup
	if( !flag_spy ) curFileMon = input_names.at(0); // maybe change in GUI later?
	if( flag_source ) conv_mon->SourceOnly();
	conv_mon->AddCalibration( calfiles->mycal );
	conv_mon->SetOutput( "monitor_singles.root" );
	conv_mon->MakeTree();
	conv_mon->MakeHists();
	
	// Update server settings
	// title of web page
	std::string toptitle;
	if( !flag_spy ) toptitle = curFileMon.substr( curFileMon.find_last_of("/")+1,
							curFileMon.length()-curFileMon.find_last_of("/")-1 );
	else toptitle = "DataSpy ";
	toptitle += " (" + std::to_string( mon_time ) + " s)";
	serv->SetItemField("/", "_toptitle", toptitle.data() );

	// While the sort is running
	while( true ) {
		
		// While the sort is running, bRunMon is true
		while( bRunMon ) {
			
			// Lock the main thread
			//TThread::Lock();
			
			// Convert - from file
			if( !flag_spy ) {
				
				nblocks = conv_mon->ConvertFile( curFileMon, start_block );
				start_block = nblocks;
				
			}
			
			// Convert - from shared memory
			else {
				
				// First check if we have data
				std::cout << "Looking for data from DataSpy" << std::endl;
				spy_length = myspy.Read( file_id, (char*)buffer, calfiles->myset->GetBlockSize() );
				if( spy_length == 0 && bFirstRun ) {
					std::cout << "No data yet on first pass" << std::endl;
					gSystem->Sleep( 2e3 );
					continue;
				}
				
				// Keep reading until we have all the data
				// This could be multi-threaded to process data and go back to read more
				int block_ctr = 0;
				long byte_ctr = 0;
				int poll_ctr = 0;
				while( block_ctr < 200 && poll_ctr < 1000 ){

					//std::cout << "Got some data from DataSpy, block " << block_ctr << std::endl;
					if( spy_length > 0 ) {
						nblocks = conv_mon->ConvertBlock( (char*)buffer, 0 );
						block_ctr += nblocks;
					}
					
					// Read a new block
					gSystem->Sleep( 1 ); // wait 1 ms between each read
					spy_length = myspy.Read( file_id, (char*)buffer, calfiles->myset->GetBlockSize() );

					byte_ctr += spy_length;
					poll_ctr++;
					
				}

				std::cout << "Got " << byte_ctr << " bytes of data from DataSpy" << std::endl;

				// Sort the packets we just got, then do the rest of the analysis
				conv_mon->SortTree();
				conv_mon->PurgeOutput();

			}
			
			// Only do the rest if it is not a source run
			if( !flag_source ) {
				
				// Event builder
				if( bFirstRun ) {
					eb_mon->SetOutput( "monitor_events.root" );
					eb_mon->StartFile();
				}
				TTree *sorted_tree = conv_mon->GetSortedTree()->CloneTree();
				eb_mon->SetInputTree( sorted_tree );
				eb_mon->GetTree()->Reset();
				nbuild = eb_mon->BuildEvents();
				eb_mon->PurgeOutput();
				delete sorted_tree;
				
				// Histogrammer
				if( bFirstRun ) {
					hist_mon->SetOutput( "monitor_hists.root" );
				}
				if( nbuild ) {
					TTree *evt_tree = eb_mon->GetTree()->CloneTree();
					hist_mon->SetInputTree( evt_tree );
					hist_mon->FillHists();
					hist_mon->PurgeOutput();
					delete evt_tree;
				}
				
				// If this was the first time we ran, do stuff?
				if( bFirstRun ) {
					
					bFirstRun = kFALSE;
					
				}
				
			}
			
			// This makes things unresponsive!
			// Unless we are threading?
			gSystem->Sleep( mon_time * 1e3 );
			
		} // bRunMon
		
	} // always running
	

	// Close the dataSpy before exiting
	if( flag_spy ) myspy.Close( file_id );

	// Close all outputs
	conv_mon->CloseOutput();
	eb_mon->CloseOutput();
	hist_mon->CloseOutput();

	return 0;
	
}

//void* start_http( void* ptr ){
void start_http(){

	// Server for JSROOT
	std::string server_name = "http:" + std::to_string(port_num) + "?top=ISSDAQMonitoring";
	serv = new THttpServer( server_name.data() );
	serv->SetReadOnly(kFALSE);

	// enable monitoring and
	// specify items to draw when page is opened
	serv->SetItemField("/","_monitoring","5000");
	//serv->SetItemField("/","_layout","grid2x2");
	//serv->SetItemField("/","_drawitem","[hpxpy,hpx,Debug]");
	serv->SetItemField("/","drawopt","[colz,hist]");
	
	// register simple start/stop commands
	//serv->RegisterCommand("/Start", "bRunMon=kTRUE;", "button;/usr/share/root/icons/ed_execute.png");
	//serv->RegisterCommand("/Stop",  "bRunMon=kFALSE;", "button;/usr/share/root/icons/ed_interrupt.png");
	serv->RegisterCommand("/Start", "StartMonitor()");
	serv->RegisterCommand("/Stop", "StopMonitor()");
	serv->RegisterCommand("/ResetSingles", "ResetConv()");
	serv->RegisterCommand("/ResetEvents", "ResetEvnt()");
	serv->RegisterCommand("/ResetHists", "ResetHist()");

	// hide commands so the only show as buttons
	//serv->Hide("/Start");
	//serv->Hide("/Stop");
	//serv->Hide("/Reset");
	
	return;
	
}

void do_convert(){
	
	//------------------------//
	// Run conversion to ROOT //
	//------------------------//
	ISSConverter conv( myset );
	conv.AddCalibration( mycal );
	if( flag_source ) conv.SourceOnly();
	std::cout << "\n +++ ISS Analysis:: processing Converter +++" << std::endl;

	TFile *rtest;
	std::ifstream ftest;
	std::string name_input_file;
	std::string name_output_file;
	
	// Check each file
	for( unsigned int i = 0; i < input_names.size(); i++ ){
			
		name_input_file = input_names.at(i).substr( input_names.at(i).find_last_of("/")+1,
												   input_names.at(i).length() - input_names.at(i).find_last_of("/")-1 );
		name_input_file = name_input_file.substr( 0,
												 name_input_file.find_last_of(".") );
		name_output_file = name_input_file.substr( 0,
												  name_input_file.find_last_of(".") );
		if( flag_source ) name_output_file = name_output_file + "_source.root";
		else name_output_file = name_output_file + ".root";
		
		name_output_file = datadir_name + "/" + name_output_file;
		name_input_file = input_names.at(i);

		force_convert.push_back( false );

		// If input doesn't exist, skip it
		ftest.open( name_input_file.data() );
		if( !ftest.is_open() ) {
			
			std::cerr << name_input_file << " does not exist" << std::endl;
			continue;
			
		}
		else ftest.close();
		
		// If output doesn't exist, we have to convert it anyway
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
			conv.ConvertFile( name_input_file );

			// Sort the tree before writing and closing
			if( !flag_source ) conv.SortTree();
			conv.CloseOutput();

		}
		
	}
	
	return;
	
}


bool do_build(){
	
	//-----------------------//
	// Physics event builder //
	//-----------------------//
	ISSEventBuilder eb( myset );
	std::cout << "\n +++ ISS Analysis:: processing EventBuilder +++" << std::endl;
	
	TFile *rtest;
	std::ifstream ftest;
	std::string name_input_file;
	std::string name_output_file;
	bool return_flag = false;
	
	// Update calibration file if given
	if( overwrite_cal ) eb.AddCalibration( mycal );

	// Do event builder for each file individually
	for( unsigned int i = 0; i < input_names.size(); i++ ){
			
		name_input_file = input_names.at(i).substr( input_names.at(i).find_last_of("/")+1,
												   input_names.at(i).length() - input_names.at(i).find_last_of("/")-1 );
		name_input_file = name_input_file.substr( 0,
												 name_input_file.find_last_of(".") );
		name_output_file = name_input_file.substr( 0,
												  name_input_file.find_last_of(".") );
		
		name_output_file = datadir_name + "/" + name_output_file + "_events.root";
		name_input_file = datadir_name + "/" + name_input_file + ".root";

		// Check if the input file exists
		ftest.open( name_input_file.data() );
		if( !ftest.is_open() ) {
			
			std::cerr << name_input_file << " does not exist" << std::endl;
			continue;
			
		}
		else {
			
			return_flag = true;
			ftest.close();

		}

		// We need to do event builder if we just converted it
		// specific request to do new event build with -e
		// this is useful if you need to add a new calibration
		if( flag_convert || force_convert.at(i) || flag_events )
			force_events = true;

		// If it doesn't exist, we have to sort it anyway
		else {
			
			ftest.open( name_output_file.data() );
			if( !ftest.is_open() ) force_events = true;
			else {
				
				ftest.close();
				rtest = new TFile( name_output_file.data() );
				if( rtest->IsZombie() ) force_events = true;
				if( !force_events )
					std::cout << name_output_file << " already built" << std::endl;
				rtest->Close();
				
			}
			
		}
		
		if( force_events ) {

			std::cout << name_input_file << " --> ";
			std::cout << name_output_file << std::endl;

			eb.SetInputFile( name_input_file );
			eb.SetOutput( name_output_file );
			eb.BuildEvents();
			eb.CloseOutput();
		
			force_events = false;
			
		}
		
	}
	
	return return_flag;
	
}

void do_hist(){
	
	//------------------------------//
	// Finally make some histograms //
	//------------------------------//
	ISSHistogrammer hist( myreact, myset );
	std::cout << "\n +++ ISS Analysis:: processing Histogrammer +++" << std::endl;

	std::ifstream ftest;
	std::string name_input_file;
	
	std::vector<std::string> name_hist_files;
	
	// We are going to chain all the event files now
	for( unsigned int i = 0; i < input_names.size(); i++ ){

		name_input_file = input_names.at(i).substr( input_names.at(i).find_last_of("/")+1,
												   input_names.at(i).length() - input_names.at(i).find_last_of("/")-1 );
		name_input_file = name_input_file.substr( 0,
												 name_input_file.find_last_of(".") );
		name_input_file = datadir_name + "/" + name_input_file + "_events.root";

		ftest.open( name_input_file.data() );
		if( !ftest.is_open() ) {
			
			std::cerr << name_input_file << " does not exist" << std::endl;
			continue;
			
		}
		else ftest.close();

		name_hist_files.push_back( name_input_file );
		
	}

	// Only do something if there are valid files
	if( name_hist_files.size() ) {
		
		hist.SetOutput( output_name );
		hist.SetInputFile( name_hist_files );
		hist.FillHists();
		hist.CloseOutput();

	}
	
	return;
	
}

void do_nptool(){
	
	//-----------------------//
	// Physics event builder //
	//-----------------------//
	ISSEventBuilder eb( myset );
	std::cout << "\n +++ ISS Analysis:: processing EventBuilder for NPTool data +++" << std::endl;
	
	TFile *rtest;
	std::ifstream ftest;
	std::string name_input_file;
	std::string name_output_file;
	
	// Do event builder for each file individually
	for( unsigned int i = 0; i < input_names.size(); i++ ){
		
		name_input_file = input_names.at(i).substr( input_names.at(i).find_last_of("/")+1,
												   input_names.at(i).length() - input_names.at(i).find_last_of("/")-1 );
		name_output_file = name_input_file.substr( 0,
												  name_input_file.find_last_of(".") );
		
		name_output_file = datadir_name + "/" + name_output_file + "_events.root";
		name_input_file = input_names.at(i);
		
		// Check if the input file exists
		ftest.open( name_input_file.data() );
		if( !ftest.is_open() ) {
			
			std::cerr << name_input_file << " does not exist" << std::endl;
			continue;
			
		}
		else {
			
			ftest.close();
			
		}
		
		// We need to do event builder if there is a specific
		// request to do so with either the -e or -f flag
		if( flag_convert || flag_events )
			force_events = true;
		
		// If it doesn't exist, we have to sort it anyway
		else {
			
			ftest.open( name_output_file.data() );
			if( !ftest.is_open() ) force_events = true;
			else {
				
				ftest.close();
				rtest = new TFile( name_output_file.data() );
				if( rtest->IsZombie() ) force_events = true;
				if( !force_events )
					std::cout << name_output_file << " already built" << std::endl;
				rtest->Close();
				
			}
			
		}
		
		if( force_events ) {
			
			std::cout << name_input_file << " --> ";
			std::cout << name_output_file << std::endl;
			
			eb.SetNPToolFile( name_input_file );
			eb.SetOutput( name_output_file );
			eb.BuildSimulatedEvents();
			eb.CloseOutput();
			
			force_events = false;
			
		}
		
	}
	
	return;
	
}

void do_pace4(){
	
	//-----------------------------------------------------//
	// Make some histograms from the PACE4 simulation data //
	//-----------------------------------------------------//
	ISSHistogrammer hist( myreact, myset );
	std::cout << "\n +++ ISS Analysis:: processing Histogrammer with PACE4 data +++" << std::endl;

	std::ifstream ftest;
	std::string name_input_file;
	
	std::vector<std::string> name_hist_files;
	
	// We are going to chain all the event files now
	for( unsigned int i = 0; i < input_names.size(); i++ ){

		name_input_file = input_names.at(i);

		ftest.open( name_input_file.data() );
		if( !ftest.is_open() ) {
			
			std::cerr << name_input_file << " does not exist" << std::endl;
			continue;
			
		}
		else ftest.close();

		name_hist_files.push_back( name_input_file );
		
	}

	// Only do something if there are valid files
	if( name_hist_files.size() ) {
		
		hist.SetOutput( output_name );
		hist.SetPace4File( name_hist_files );
		hist.FillHists();
		hist.CloseOutput();

	}
	
	return;
	
}

void do_autocal(){

	//-----------------------------------//
	// Run automatic calibration routine //
	//-----------------------------------//
	ISSAutoCalibrator autocal( myset, myreact, name_autocal_file );
	autocal.AddCalibration( mycal );
	std::cout << "\n +++ ISS Analysis:: processing AutoCalibration +++" << std::endl;
	
	// Autocal debug messages
	if ( autocal.GetDebugStatus() ){
		std::cout << "  !  AUTOCAL DEBUG MODE" << std::endl;
		std::cout << "  !  Fitting " << autocal.GetFitShapeName() << "s" << std::endl;
		if ( autocal.OnlyManualFitStatus() ){
			std::cout << "  !  Doing manual fits only" << std::endl;
		}
	}

	TFile *rtest;
	std::ifstream ftest;
	std::string name_input_file;
	std::string name_output_file = "autocal.root";
	std::string hadd_file_list = "";
	std::string name_results_file = "autocal_results.cal";

	// Check each file
	for( unsigned int i = 0; i < input_names.size(); i++ ){
			
		name_input_file = datadir_name + "/" + input_names.at(i) + "_source.root";

		// Add to list if the converted file exists
		ftest.open( name_input_file.data() );
		if( ftest.is_open() ) {
		
			ftest.close();
			rtest = new TFile( name_input_file.data() );
			if( !rtest->IsZombie() ) {
				hadd_file_list += " " + name_input_file;
			}
			else {
				std::cout << "Skipping " << name_input_file;
				std::cout << ", it's broken" << std::endl;
			}
			rtest->Close();
			
		}
		
		else {
			std::cout << "Skipping " << name_input_file;
			std::cout << ", file does not exist" << std::endl;
		}

	}
	
	// Perform the hadd (doesn't work on Windows)
	std::string cmd = "hadd -k -T -v 0 -f ";
	cmd += name_output_file;
	cmd += hadd_file_list;
	gSystem->Exec( cmd.data() );
	
	// Give this file to the autocalibrator
	if( autocal.SetOutputFile( name_output_file ) ) return;
	autocal.DoFits();
	autocal.SaveCalFile( name_results_file );
	
}

int main( int argc, char *argv[] ){
	
	// Command line interface, stolen from MiniballCoulexSort
	CommandLineInterface *interface = new CommandLineInterface();

	interface->Add("-i", "List of input files", &input_names );
	interface->Add("-o", "Output file for histogram file", &output_name );
	interface->Add("-s", "Settings file", &name_set_file );
	interface->Add("-c", "Calibration file", &name_cal_file );
	interface->Add("-r", "Reaction file", &name_react_file );
	interface->Add("-nptool", "Flag for NPTool simulation input", &flag_nptool );
	interface->Add("-pace4", "Flag for PACE4 particle input", &flag_pace4 );
	interface->Add("-f", "Flag to force new ROOT conversion", &flag_convert );
	interface->Add("-e", "Flag to force new event builder (new calibration)", &flag_events );
	interface->Add("-source", "Flag to define an source only run", &flag_source );
	interface->Add("-autocal", "Flag to perform automatic calibration of alpha source data", &flag_autocal );
	interface->Add("-autocalfile", "Alpha source fit control file", &name_autocal_file );
	interface->Add("-print-settings", "Print settings", &flag_print_settings );
	interface->Add("-spy", "Flag to run the DataSpy", &flag_spy );
	interface->Add("-m", "Monitor input file every X seconds", &mon_time );
	interface->Add("-p", "Port number for web server (default 8030)", &port_num );
	interface->Add("-d", "Output directory for sorted files", &datadir_name );
	interface->Add("-g", "Launch the GUI", &gui_flag );
	interface->Add("-h", "Print this help", &help_flag );

	interface->CheckFlags( argc, argv );
	if( help_flag ) {
		
		interface->CheckFlags( 1, argv );
		return 0;
		
	}

	// If we are launching the GUI
	if( gui_flag || argc == 1 ) {
		
		TApplication theApp( "App", &argc, argv );
		new ISSGUI();
		theApp.Run();
		
		return 0;

	}

	// Check we have data files
	if( !input_names.size() && !flag_spy ) {
			
			std::cout << "You have to provide at least one input file (data or simulation) unless you are in DataSpy mode!" << std::endl;
			return 1;
			
	}
	
	// Check if this is a source run
	if( flag_autocal ){
		
		flag_source = true;
		
		if ( name_autocal_file.length() > 0 ){
		
			std::cout << "Autocal file: " << name_autocal_file << std::endl;
		
		}
		
		else{
		
			std::cout << "No autocal file provided. Using defaults." << std::endl;
			name_autocal_file = "dummy";
		
		}
		
	}
	
	// Check if we have real data, i.e. not simulation
	if( !flag_pace4 && !flag_nptool ) flag_data = true;
	
	
	// Check if we should be monitoring the input
	if( flag_spy ) {
		
		flag_monitor = true;
		if( mon_time < 0 ) mon_time = 30;
		std::cout << "Getting data from shared memory every " << mon_time;
		std::cout << " seconds using DataSpy" << std::endl;
		
	}
	
	else if( mon_time >= 0 && input_names.size() == 1 ) {
		
		if( flag_data ) {

			flag_monitor = true;
			std::cout << "Running sort in a loop every " << mon_time;
			std::cout << " seconds\nMonitoring " << input_names.at(0) << std::endl;
		
		}
		
		else {
			
			flag_monitor = false;
			std::cout << "Cannot monitor simulation input files, switching to normal mode" << std::endl;

		}
		
	}
	
	else if( mon_time >= 0 && input_names.size() != 1 ) {
		
		flag_monitor = false;
		std::cout << "Cannot monitor multiple input files, switching to normal mode" << std::endl;
				
	}
	
	// Check the directory we are writing to
	if( datadir_name.length() == 0 ) {
		
		if( bool( input_names.size() ) ) {
			
			// Probably in the current working directory
			if( input_names.at(0).find("/") == std::string::npos )
				datadir_name = "./sorted";
			
			// Called from a different directory
			else {
				
				datadir_name = input_names.at(0).substr( 0,
														input_names.at(0).find_last_of("/") );
				datadir_name += "/sorted";
				
			}
			
			// Create the directory if it doesn't exist (not Windows compliant)
			std::string cmd = "mkdir -p " + datadir_name;
			gSystem->Exec( cmd.data() );
			
		}
		
		else datadir_name = "dataspy";
		
		std::cout << "Sorted data files being saved to " << datadir_name << std::endl;
		
	}

	// Check the ouput file name
	if( output_name.length() == 0 ) {
		
		if( bool( input_names.size() ) ) {
			
			std::string name_input_file = input_names.at(0).substr( input_names.at(0).find_last_of("/")+1,
																   input_names.at(0).length() - input_names.at(0).find_last_of("/")-1 );
			name_input_file = name_input_file.substr( 0,
													 name_input_file.find_last_of(".") );
			
			if( input_names.size() > 1 ) {
				output_name = datadir_name + "/" + name_input_file + "_hists_";
				output_name += std::to_string(input_names.size()) + "_subruns.root";
			}
			else
				output_name = datadir_name + "/" + name_input_file + "_hists.root";
			
		}
		
		else output_name = datadir_name + "/monitor_hists.root";
		
	}

	// Check we have a Settings file
	if( name_set_file.length() > 0 ) {
		
		// Test if the file exists
		std::ifstream ftest;
		ftest.open( name_set_file.data() );
		if( !ftest.is_open() ) {
			
			std::cout << name_set_file << " does not exist.";
			std::cout << " Using defaults" << std::endl;
			name_set_file = "dummy";

		}
		
		else {
		
			ftest.close();
			std::cout << "Settings file: " << name_set_file << std::endl;
		
		}
		
	}
	else {
		
		std::cout << "No settings file provided. Using defaults." << std::endl;
		name_set_file = "dummy";

	}
	
	// Check we have a calibration file
	if( name_cal_file.length() > 0 ) {
		
		// Test if the file exists
		std::ifstream ftest;
		ftest.open( name_cal_file.data() );
		if( !ftest.is_open() ) {
			
			std::cout << name_cal_file << " does not exist.";
			std::cout << " Using defaults" << std::endl;
			name_cal_file = "dummy";

		}
		
		else {
			
			ftest.close();
			std::cout << "Calibration file: " << name_cal_file << std::endl;
			overwrite_cal = true;
			
		}

	}
	else {
		
		std::cout << "No calibration file provided. Using defaults." << std::endl;
		name_cal_file = "dummy";

	}
	
	// Check we have a reaction file
	if( name_react_file.length() > 0 ) {
		
		// Test if the file exists
		std::ifstream ftest;
		ftest.open( name_react_file.data() );
		if( !ftest.is_open() ) {
			
			std::cout << name_react_file << " does not exist.";
			std::cout << " Using defaults" << std::endl;
			name_react_file = "dummy";

		}
		
		else {
		
			ftest.close();
			std::cout << "Reaction file: " << name_react_file << std::endl;

		}
		
	}
	else {
		
		std::cout << "No reaction file provided. Using defaults." << std::endl;
		name_react_file = "dummy";

	}
	
	myset = std::make_shared<ISSSettings>( name_set_file );
	mycal = std::make_shared<ISSCalibration>( name_cal_file, myset );
	myreact = std::make_shared<ISSReaction>( name_react_file, myset, flag_source );

	if (flag_print_settings)
		myset->PrintSettings();

	//-------------------//
	// Online monitoring //
	//-------------------//
	if( flag_monitor || flag_spy ) {
		
		// Make some data for the thread
		thread_data data;
		data.mycal = mycal;
		data.myset = myset;
		data.myreact = myreact;

		// Start the HTTP server from the main thread (should usually do this)
		start_http();
		gSystem->ProcessEvents();

		// Thread for the monitor process
		TThread *th = new TThread( "monitor", monitor_run, (void*) &data );
		th->Run();
		
		// wait until we finish
		while( true ){
			
			gSystem->Sleep(10);
			gSystem->ProcessEvents();
			
		}
		std::cout << "Finished" << std::endl;
		
		return 0;
		
	}


	//------------------//
	// Run the analysis //
	//------------------//
	if( flag_data ) {
		
		// Convert MIDAS file and time sort
		do_convert();
		
		// If it's not a source run, do the event building
		if( !flag_source && !flag_autocal ) {
			
			// Build events and if successful, do histogramming
			if( do_build() ) do_hist();
			
		}
		
		// Autocal routine is run independently
		else if( flag_autocal ) do_autocal();
		
	}
	
	// Simulation analysis - PACE4
	else if( flag_pace4 ) do_pace4();
	
	// Simulation analysis - NPTool
	else if( flag_nptool ) {
		
		do_nptool();
		do_hist();
		
	}
	
	std::cout << "\n\nFinished!\n";

	return 0;

}
