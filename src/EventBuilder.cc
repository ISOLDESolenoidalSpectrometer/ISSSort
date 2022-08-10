#include "EventBuilder.hh"
///////////////////////////////////////////////////////////////////////////////
/// This constructs the event-builder object, setting parameters for this process by grabbing information from the settings file (or using default parameters defined in the constructor)
/// \param[in] myset The ISSSettings object which is constructed by the ISSSettings constructor used in iss_sort.cc
ISSEventBuilder::ISSEventBuilder( ISSSettings *myset ){
	
	// First get the settings
	set = myset;
	
	// No calibration file by default
	overwrite_cal = false;
	
	// No input file at the start by default
	flag_input_file = false;
	
	// No progress bar by default
	_prog_ = false;

	// ------------------------------------------------------------------------ //
	// Initialise variables and flags
	// ------------------------------------------------------------------------ //
	build_window = set->GetEventWindow();

	// Resize 17 vectors to match modules of detectors
	n_fpga_pulser.resize( set->GetNumberOfArrayModules() );
	n_asic_pulser.resize( set->GetNumberOfArrayModules() );
	n_asic_pause.resize( set->GetNumberOfArrayModules() );
	n_asic_resume.resize( set->GetNumberOfArrayModules() );
	flag_pause.resize( set->GetNumberOfArrayModules() );
	flag_resume.resize( set->GetNumberOfArrayModules() );
	pause_time.resize( set->GetNumberOfArrayModules() );
	resume_time.resize( set->GetNumberOfArrayModules() );
	asic_dead_time.resize( set->GetNumberOfArrayModules() );
	asic_time_start.resize( set->GetNumberOfArrayModules() );
	asic_time_stop.resize( set->GetNumberOfArrayModules() );
	asic_time.resize( set->GetNumberOfArrayModules() );
	asic_prev.resize( set->GetNumberOfArrayModules() );
	fpga_time.resize( set->GetNumberOfArrayModules() );
	fpga_prev.resize( set->GetNumberOfArrayModules() );
	caen_time_start.resize( set->GetNumberOfCAENModules() );
	caen_time_stop.resize( set->GetNumberOfCAENModules() );

	// p-side = 0; n-side = 1;
	asic_side.push_back(0); // asic 0 = p-side
	asic_side.push_back(1); // asic 1 = n-side
	asic_side.push_back(0); // asic 2 = p-side
	asic_side.push_back(0); // asic 3 = p-side
	asic_side.push_back(1); // asic 4 = n-side
	asic_side.push_back(0); // asic 5 = p-side

	asic_row.push_back(0); // asic 0 = row 0 p-side
	asic_row.push_back(0); // asic 1 = row 0 and 1 n-side
	asic_row.push_back(1); // asic 2 = row 1 p-side
	asic_row.push_back(2); // asic 3 = row 2 p-side
	asic_row.push_back(2); // asic 4 = row 2 and 3 n-side
	asic_row.push_back(3); // asic 5 = row 3 p-side

	array_row.resize( set->GetNumberOfArrayASICs() );
	array_pid.resize( set->GetNumberOfArrayASICs() );
	array_nid.resize( set->GetNumberOfArrayASICs() );
	
	
	// Loop over ASICs in a module
	for( unsigned int i = 0; i < set->GetNumberOfArrayASICs(); ++i ) {
		
		// Loop over channels in each ASIC
		for( unsigned int j = 0; j < set->GetNumberOfArrayChannels(); ++j ) {

			// p-side: all channels used; fill n-side with -1; fill array_row with row number for p-side
			if( asic_side.at(i) == 0 ) {
			
				array_pid[i].push_back( j );
				array_nid[i].push_back( -1 );
				array_row.at(i).push_back( asic_row.at(i) );
				
			}
			
			// n-side: 11 channels per ASIC 0/2A; fill p-side with -1; fill array row for n-side
			else if( j >= 11 && j <= 21 ) {
				
				mystrip = j - 11;
				array_nid[i].push_back( mystrip );
				array_pid[i].push_back( -1 );
				array_row.at(i).push_back( asic_row.at(i) );
				
			}
			
			// n-side: 11 channels per ASIC 0/2B; fill p-side with -1; fill array row for n-side
			else if( j >= 28 && j <= 38 ) {
				
				mystrip = 38 - j + set->GetNumberOfArrayNstrips();
				array_nid[i].push_back( mystrip );
				array_pid[i].push_back( -1 );
				array_row.at(i).push_back( asic_row.at(i) );

			}
			
			// n-side: 11 channels per ASIC 1/3B; fill p-side with -1; fill array row for n-side
			else if( j >= 89 && j <= 99 ) {
				
				mystrip = j - 89 + set->GetNumberOfArrayNstrips();
				array_nid[i].push_back( mystrip );
				array_pid[i].push_back( -1 );
				array_row.at(i).push_back( asic_row.at(i) + 1 ); // nside need incrementing for odd wafers

			}
			
			// n-side: 11 channels per ASIC 1/3A; fill p-side with -1; fill array row for n-side
			else if( j >= 106 && j <= 116 ) {
				
				mystrip = 116 - j;
				array_nid[i].push_back( mystrip );
				array_pid[i].push_back( -1 );
				array_row.at(i).push_back( asic_row.at(i) + 1 ); // nside need incrementing for odd wafers

			}
			
			// n-side and p-side: empty channels -> set to -1; set array_row to 0
			else{
				array_nid[i].push_back(-1);
				array_pid[i].push_back( -1 );
				array_row.at(i).push_back( 0 );	// N.B. these should only be for unused channels for the n-sides, but this is an actual row number so could run into problems down the line...
			}
	
		}
		
	}
	
	return;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Reset private-member counters, arrays and flags for processing the next input file. Called in the ISSEventBuilder::SetInputFile and ISSEventBuilder::SetInputTree functions
void ISSEventBuilder::StartFile(){
	
	// Call for every new file
	// Reset counters etc.
	
	time_prev = 0;
	time_min = 0;
	time_max = 0;
	time_first = 0;
	caen_time = 0;
	caen_prev = 0;
	ebis_time = 0;
	ebis_prev = 0;
	t1_time = 0;
	t1_prev = 0;

	n_asic_data	= 0;
	n_caen_data	= 0;
	n_info_data	= 0;

	n_caen_pulser = 0;

	n_ebis	= 0;
	n_t1	= 0;

	array_ctr	= 0;
	arrayp_ctr	= 0;
	recoil_ctr	= 0;
	mwpc_ctr	= 0;
	elum_ctr	= 0;
	zd_ctr		= 0;
	
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {
	
		n_fpga_pulser[i] = 0;
		n_asic_pulser[i] = 0;
		n_asic_pause[i] = 0;
		n_asic_resume[i] = 0;
		flag_pause[i] = false;
		flag_resume[i] = false;
		pause_time[i] = 0;
		resume_time[i] = 0;
		asic_dead_time[i] = 0;
		asic_time_start[i] = 0;
		asic_time_stop[i] = 0;
		asic_time[i] = 0;
		asic_prev[i] = 0;
		fpga_time[i] = 0;
		fpga_prev[i] = 0;

	}
	
	for( unsigned int i = 0; i < set->GetNumberOfCAENModules(); ++i ) {

		caen_time_start[i] = 0;
		caen_time_stop[i] = 0;

	}
	
	// Some flags must be false to start
	flag_caen_pulser = false;
		
}

////////////////////////////////////////////////////////////////////////////////
/// The ROOT file is opened in read-only mode to avoid modification. If the file is not found, an error message is printed and the function does not set an input tree, and does not call ISSEventBuilder::StartFile.
/// \param [in] input_file_name The ROOT file containing the time-sorted events from that run (typical suffix is "_sort.root")
void ISSEventBuilder::SetInputFile( std::string input_file_name ) {
	
	/// Overloaded function for a single file or multiple files
	//input_tree = new TTree( "iss" );
	//input_tree->Add( input_file_name.data() );
	
	// Open next Root input file.
	input_file = new TFile( input_file_name.data(), "read" );
	if( input_file->IsZombie() ) {
		
		std::cout << "Cannot open " << input_file_name << std::endl;
		return;
		
	}
	
	flag_input_file = true;
	
	// Set the input tree
	SetInputTree( (TTree*)input_file->Get("iss_sort") );
	StartFile();

	return;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Sets the private member input_tree to the parameter user_tree, sets the branch address and calls the ISSEventBuilder::StartFile function
/// \param [in] user_tree The name of the tree in the ROOT file containing the time-sorted events
void ISSEventBuilder::SetInputTree( TTree *user_tree ){
	
	// Find the tree and set branch addresses
	input_tree = user_tree;
	in_data = nullptr;
	input_tree->SetBranchAddress( "data", &in_data );

	return;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Constructs a number of objects for storing measurements from different detectors, that can then be wrapped up into physics events. Also creates an output file and output tree, and calls the ISSEventBuilder::MakeEventHists function
/// \param [in] output_file_name The ROOT file for storing the events from the event-building process (typical suffix is "_events.root")
void ISSEventBuilder::SetOutput( std::string output_file_name ) {

	// These are the branches we need
	write_evts	= std::make_unique<ISSEvts>();
	array_evt	= std::make_shared<ISSArrayEvt>();
	arrayp_evt	= std::make_shared<ISSArrayPEvt>();
	recoil_evt	= std::make_shared<ISSRecoilEvt>();
	mwpc_evt	= std::make_shared<ISSMwpcEvt>();
	elum_evt	= std::make_shared<ISSElumEvt>();
	zd_evt		= std::make_shared<ISSZeroDegreeEvt>();

	// ------------------------------------------------------------------------ //
	// Create output file and create events tree
	// ------------------------------------------------------------------------ //
	output_file = new TFile( output_file_name.data(), "recreate" );
	output_tree = new TTree( "evt_tree", "evt_tree" );
	output_tree->Branch( "ISSEvts", "ISSEvts", write_evts.get() );
	output_tree->SetAutoFlush();

	// Create log file.
	std::string log_file_name = output_file_name.substr( 0, output_file_name.find_last_of(".") );
	log_file_name += ".log";
	log_file.open( log_file_name.data(), std::ios::app );
	
	// Hisograms in separate function
	MakeEventHists();
	
}

////////////////////////////////////////////////////////////////////////////////
/// Clears the vectors that store energies, time differences, ids, module numbers, row numbers, recoil sectors etc. Also resets flags that are relevant for building events
void ISSEventBuilder::Initialise(){

	/// This is called at the end of every execution/loop
	
	flag_close_event = false;
	event_open = false;
	
	hit_ctr = 0;
	
	// Clear all vectors with the standard method
	pen_list.clear();
	nen_list.clear();
	ptd_list.clear();
	ntd_list.clear();
	pid_list.clear();
	nid_list.clear();
	pmod_list.clear();
	nmod_list.clear();
	prow_list.clear();
	nrow_list.clear();
	
	ren_list.clear();
	rtd_list.clear();
	rid_list.clear();
	rsec_list.clear();
	
	mwpctac_list.clear();
	mwpctd_list.clear();
	mwpcaxis_list.clear();
	mwpcid_list.clear();

	een_list.clear();
	etd_list.clear();
	esec_list.clear();
	
	zen_list.clear();
	ztd_list.clear();
	zid_list.clear();
	
	// Now swap all these vectors with empty vectors to ensure they are fully cleared
	std::vector<float>().swap(pen_list);
	std::vector<float>().swap(nen_list);
	std::vector<long>().swap(ptd_list);
	std::vector<long>().swap(ntd_list);
	std::vector<int>().swap(pid_list);
	std::vector<int>().swap(nid_list);
	std::vector<int>().swap(pmod_list);
	std::vector<int>().swap(nmod_list);
	std::vector<int>().swap(prow_list);
	std::vector<int>().swap(nrow_list);
	
	std::vector<float>().swap(ren_list);
	std::vector<long>().swap(rtd_list);
	std::vector<int>().swap(rid_list);
	std::vector<int>().swap(rsec_list);
	
	std::vector<unsigned short>().swap(mwpctac_list);
	std::vector<long>().swap(mwpctd_list);
	std::vector<int>().swap(mwpcaxis_list);
	std::vector<int>().swap(mwpcid_list);

	std::vector<float>().swap(een_list);
	std::vector<long>().swap(etd_list);
	std::vector<int>().swap(esec_list);
	
	std::vector<float>().swap(zen_list);
	std::vector<long>().swap(ztd_list);
	std::vector<int>().swap(zid_list);

	write_evts->ClearEvt();
	
	return;
	
}

////////////////////////////////////////////////////////////////////////////////
/// This loops over all events found in the input file and wraps them up and stores them in the output file
/// \return The number of entries in the tree that have been sorted (=0 if there is an error)
unsigned long ISSEventBuilder::BuildEvents() {
	
	/// Function to loop over the sort tree and build array and recoil events

	// Load the full tree if possible
	output_tree->SetMaxVirtualSize(2e9); // 2GB
	input_tree->SetMaxVirtualSize(2e9); // 2GB
	input_tree->LoadBaskets(1e9); // Load 2 GB of data to memory

	if( input_tree->LoadTree(0) < 0 ){
		
		std::cout << " Event Building: nothing to do" << std::endl;
		return 0;
		
	}
	
	// Get ready and go
	Initialise();
	n_entries = input_tree->GetEntries();

	std::cout << " Event Building: number of entries in input tree = ";
	std::cout << n_entries << std::endl;

	
	// ------------------------------------------------------------------------ //
	// Main loop over TTree to find events
	// ------------------------------------------------------------------------ //
	for( unsigned long i = 0; i < n_entries; ++i ) {
		
		// Current event data
		if( input_tree->MemoryFull(30e6) )
			input_tree->DropBaskets();
		if( i == 0 ) input_tree->GetEntry(i);
		
		// Get the time of the event
		mytime = in_data->GetTime();
		
		//std::cout << i << "\t" << mytime << std::endl;
				
		// check time stamp monotonically increases!
		if( time_prev > mytime ) {
			
			std::cout << "Out of order event in file ";
			std::cout << input_tree->GetName() << std::endl;
			
		}
		
		// record time of this event
		time_prev = mytime;
		
		// assume this is above threshold initially
		mythres = true;

		
		// ------------------------------------------ //
		// Find particles on the array
		// ------------------------------------------ //
		if( in_data->IsAsic() ) {
			
			// Increment event counter
			n_asic_data++;
			
			asic_data = in_data->GetAsicData();
			mymod = asic_data->GetModule();
			mych = asic_data->GetChannel();
			myasic = asic_data->GetAsic();
			myside = asic_side.at( myasic );
			myrow = array_row.at( myasic ).at( mych );
			if( overwrite_cal ) {
			
				myenergy = cal->AsicEnergy( mymod, myasic,
									 mych, asic_data->GetAdcValue() );
				mywalk = cal->AsicWalk( mymod, myasic, myenergy );
			
				/*if( asic_data->GetAdcValue() > cal->AsicThreshold( mymod, myasic, mych ) )
					mythres = true;
				else mythres = false;*/
				if( asic_data->GetAdcValue() < cal->AsicThreshold( mymod, myasic, mych ) )
					mythres = false;
				
			}
			
			else {
				
				myenergy = asic_data->GetEnergy();
				mywalk = asic_data->GetWalk();
				mythres = asic_data->IsOverThreshold();
			
			}

			// If it's below zero in energy, consider it below threshold
			if( myenergy < 0 ) mythres = false;
			
			// If it's below threshold do not use as window opener
			if ( mythres ) event_open = true;
			

			// p-side event
			if( myside == 0 && mythres ) {
			
			// test here about hit bit value
			//if( myside == 0 && !asic_data->GetHitBit() ) {

				mystrip = array_pid.at( asic_data->GetAsic() ).at( asic_data->GetChannel() );
				
				// Only use if it is an event from a detector
				if( mystrip >= 0 ) {
							
					pen_list.push_back( myenergy );
					ptd_list.push_back( mytime + mywalk );
					pmod_list.push_back( mymod );
					pid_list.push_back( mystrip );
					prow_list.push_back( myrow );

					hit_ctr++; // increase counter for bits of data included in this event

				}
				
			}

			// n-side event
			else if( myside == 1 && mythres ) {

			// test here about hit bit value
			//else if( myside == 1 && asic_data->GetHitBit() ) {

				mystrip = array_nid.at( asic_data->GetAsic() ).at( asic_data->GetChannel() );

				// Only use if it is an event from a detector
				if( mystrip >= 0 ) {
							
					nen_list.push_back( myenergy );
					ntd_list.push_back( mytime + mywalk );
					nmod_list.push_back( mymod );
					nid_list.push_back( mystrip );
					nrow_list.push_back( myrow );

					hit_ctr++; // increase counter for bits of data included in this event

				}
				
			}

			// Is it the start event?
			if( asic_time_start.at( mymod ) == 0 )
				asic_time_start.at( mymod ) = mytime;
			
			// or is it the end event (we don't know so keep updating)
			asic_time_stop.at( mymod ) = mytime;
			
		}

		// ------------------------------------------ //
		// Find recoils and other things
		// ------------------------------------------ //
		else if( in_data->IsCaen() ) {
			
			// Increment event counter
			n_caen_data++;
			
			caen_data = in_data->GetCaenData();
			mymod = caen_data->GetModule();
			mych = caen_data->GetChannel();
			if( overwrite_cal ) {
				
				myenergy = cal->CaenEnergy( mymod, mych,
									caen_data->GetQlong() );
				
				/*if( caen_data->GetQlong() > cal->CaenThreshold( mymod, mych ) )
					mythres = true;
				else mythres = false;*/
				if( caen_data->GetQlong() < cal->CaenThreshold( mymod, mych ) )
					mythres = false;

			}
			
			else {
				
				myenergy = caen_data->GetEnergy();
				mythres = caen_data->IsOverThreshold();

			}
			
			// If it's below threshold do not use as window opener
			if ( mythres ) event_open = true;


			// DETERMINE WHICH TYPE OF CAEN EVENT THIS IS
			// Is it a recoil?
			if( set->IsRecoil( mymod, mych ) && mythres ) {
				
				mysector = set->GetRecoilSector( mymod, mych );
				mylayer = set->GetRecoilLayer( mymod, mych );
				
				ren_list.push_back( myenergy );
				rtd_list.push_back( mytime );
				rid_list.push_back( mylayer );
				rsec_list.push_back( mysector );

				hit_ctr++; // increase counter for bits of data included in this event

			}
			
			// Is it an MWPC?
			else if( set->IsMWPC( mymod, mych ) && mythres ) {
				
				// Check for properly integrated TAC signals
				if( caen_data->GetQlong() > caen_data->GetQshort() ) {
				
					mwpctac_list.push_back( caen_data->GetTAC() );
					mwpctd_list.push_back( mytime );
					mwpcaxis_list.push_back( set->GetMWPCAxis( mymod, mych ) );
					mwpcid_list.push_back( set->GetMWPCID( mymod, mych ) );
					
				}

				hit_ctr++; // increase counter for bits of data included in this event

			}
			
			// Is it an ELUM?
			else if( set->IsELUM( mymod, mych ) && mythres ) {
				
				mysector = set->GetELUMSector( mymod, mych );
				
				een_list.push_back( myenergy );
				etd_list.push_back( mytime );
				esec_list.push_back( mysector );

				hit_ctr++; // increase counter for bits of data included in this event

			}

			// Is it a ZeroDegree?
			else if( set->IsZD( mymod, mych ) && mythres ) {
				
				mylayer = set->GetZDLayer( mymod, mych );
				
				zen_list.push_back( myenergy );
				ztd_list.push_back( mytime );
				zid_list.push_back( mylayer );
				
				hit_ctr++; // increase counter for bits of data included in this event

			}
			
			
			// Is it the start event?
			if( caen_time_start.at( mymod ) == 0 )
				caen_time_start.at( mymod ) = mytime;
			
			// or is it the end event (we don't know so keep updating)
			caen_time_stop.at( mymod ) = mytime;

		}
		
		
		// ------------------------------------------ //
		// Find info events, like timestamps etc
		// ------------------------------------------ //
		else if( in_data->IsInfo() ) {
			
			// Increment event counter
			n_info_data++;
			info_data = in_data->GetInfoData();
			
			// if there are no data so far, set this as time_first - multiple info events will just update this so won't be a problem
			if( hit_ctr == 0 )
				time_first = mytime;
			
			
			// CHECK ALL OF THE INFO DATA CODE VALUES
			// Update EBIS time
			// N.B. if you are exceeding the limits of long long, then your DAQ has been running too long
			if( info_data->GetCode() == set->GetEBISCode() ){
			
				// Each ASIC module sends ebis_time signal, so make sure difference between last ebis pulse and now is longer than the time it takes for them all to enter the DAQ
				if ( TMath::Abs( (long long)ebis_time - (long long)info_data->GetTime() ) > 1e3 ){
					
					ebis_time = info_data->GetTime();
					ebis_hz = 1e9 / ( (long long)ebis_time - (long long)ebis_prev );
					if( ebis_prev != 0 ) ebis_freq->Fill( ebis_time, ebis_hz );
					ebis_prev = ebis_time;
					n_ebis++;
				}
				
			}
		
			// Update T1 time
			else if( info_data->GetCode() == set->GetT1Code() ){
				
				if ( TMath::Abs( (long long)t1_time - (long long)info_data->GetTime() ) > 1e3 ){
				
					t1_time = info_data->GetTime();
					t1_hz = 1e9 / ( (long long)t1_time - (long long)t1_prev );
					if( t1_prev != 0 ) t1_freq->Fill( t1_time, t1_hz );
					t1_prev = t1_time;
					n_t1++;
				}

			}
			
			// Update CAEN pulser time
			else if( info_data->GetCode() == set->GetCAENPulserCode() ) {
				
				caen_time = info_data->GetTime();
				caen_hz = 1e9 / ( (long long)caen_time - (long long)caen_prev );
				if( caen_prev != 0 ) caen_freq->Fill( caen_time, caen_hz );

				flag_caen_pulser = true;
				n_caen_pulser++;

			}

			// Update ISS pulser time in FPGA
			else if( info_data->GetCode() == set->GetExternalTriggerCode() ) {
			   
				fpga_time[info_data->GetModule()] = info_data->GetTime();
				fpga_hz = 1e9 / ( (long long)fpga_time[info_data->GetModule()] - (long long)fpga_prev[info_data->GetModule()] );

				if( fpga_prev[info_data->GetModule()] != 0 )
					fpga_freq[info_data->GetModule()]->Fill( fpga_time[info_data->GetModule()], fpga_hz );

				n_fpga_pulser[info_data->GetModule()]++;

			}
			
			// Update ISS pulser time in ASICs
			else if( info_data->GetCode() == set->GetArrayPulserCode() ) {
			   
				asic_time[info_data->GetModule()] = info_data->GetTime();
				asic_hz = 1e9 / ( (long long)asic_time[info_data->GetModule()] - (long long)asic_prev[info_data->GetModule()] );

				if( asic_prev[info_data->GetModule()] != 0 )
					asic_freq[info_data->GetModule()]->Fill( asic_time[info_data->GetModule()], asic_hz );

				n_asic_pulser[info_data->GetModule()]++;

			}
			
			// Check the pause events for each module
			else if( info_data->GetCode() == set->GetPauseCode() ) {
				
				if( info_data->GetModule() < set->GetNumberOfArrayModules() ) {
				
					n_asic_pause[info_data->GetModule()]++;
					flag_pause[info_data->GetModule()] = true;
					pause_time[info_data->GetModule()] = info_data->GetTime();
				
				}
				
				else{
				
					std::cerr << "Bad pause event in module " << (int)info_data->GetModule() << std::endl;
					
				}
				
			}
			
			// Check the resume events for each module
			else if( info_data->GetCode() == set->GetResumeCode() ) {
				
				if( info_data->GetModule() < set->GetNumberOfArrayModules() ) {
				
					n_asic_resume[info_data->GetModule()]++;
					flag_resume[info_data->GetModule()] = true;
					resume_time[info_data->GetModule()] = info_data->GetTime();

					// If we didn't get the pause, module was stuck at start of run
					if( !flag_pause[info_data->GetModule()] ) {

						std::cout << "Module " << info_data->GetModule();
						std::cout << " was blocked at start of run for ";
						std::cout << (double)resume_time[info_data->GetModule()]/1e9;
						std::cout << " seconds" << std::endl;

					}
					else{
					
						// Do have pause and resume -> work out the dead time
						asic_dead_time[info_data->GetModule()] += resume_time[info_data->GetModule()];
						asic_dead_time[info_data->GetModule()] -= pause_time[info_data->GetModule()];
						
						// Reset flags
						flag_pause[info_data->GetModule()] = false;
						flag_resume[info_data->GetModule()] = false;
					
					}
				
				}
				
				else
					std::cerr << "Bad resume event in module " << (int)info_data->GetModule() << std::endl;
				
			}

			// If we have a pulser event from the CAEN DAQs, fill time difference
			if( flag_caen_pulser ) {
			
				for( unsigned int j = 0; j < set->GetNumberOfArrayModules(); ++j ) {
				
					fpga_tdiff = (double)caen_time - (double)fpga_time[j];
					asic_tdiff = (double)caen_time - (double)asic_time[j];

					// If diff is greater than 5 ms, we have the wrong pair
					if( fpga_tdiff > 5e6 ) fpga_tdiff = (double)caen_prev - (double)fpga_time[j];
					else if( fpga_tdiff < -5e6 ) fpga_tdiff = (double)caen_time - (double)fpga_prev[j];
					if( asic_tdiff > 5e6 ) asic_tdiff = (double)caen_prev - (double)asic_time[j];
					else if( asic_tdiff < -5e6 ) asic_tdiff = (double)caen_time - (double)asic_prev[j];
					
					// ??? Could be the case that |fpga_tdiff| > 5e6 after these conditional statements...change to while loop? Or have an extra condition?

					fpga_td[j]->Fill( fpga_tdiff );
					fpga_sync[j]->Fill( fpga_time[j], fpga_tdiff );
					fpga_freq_diff[j]->Fill( fpga_time[j], fpga_hz - caen_hz );
					fpga_pulser_loss[j]->Fill( fpga_time[j], (int)n_fpga_pulser[j] - (int)n_caen_pulser );
					
					asic_td[j]->Fill( asic_tdiff );
					asic_sync[j]->Fill( asic_time[j], asic_tdiff );
					asic_freq_diff[j]->Fill( asic_time[j], asic_hz - caen_hz );
					asic_pulser_loss[j]->Fill( asic_time[j], (int)n_asic_pulser[j] - (int)n_caen_pulser );

				}

				flag_caen_pulser = false;

			}
			
			// Now reset previous timestamps
			if( info_data->GetCode() == set->GetCAENPulserCode() )
				caen_prev = caen_time;
			if( info_data->GetCode() == set->GetExternalTriggerCode() )
				fpga_prev[info_data->GetModule()] = fpga_time[info_data->GetModule()];
			if( info_data->GetCode() == set->GetArrayPulserCode() )
				asic_prev[info_data->GetModule()] = asic_time[info_data->GetModule()];

						
		}
		
		// Sort out the timing for the event window
		// but only if it isn't an info event, i.e only for real data
		if ( !in_data->IsInfo() ){
			
			// if this is first datum included in Event
			if( hit_ctr == 1 && mythres ) {
				
				time_min	= mytime;
				time_max	= mytime;
				time_first	= mytime;
				
			}
			
			// Update max time
			if( mytime > time_max ) time_max = mytime;
			else if( mytime < time_min ) time_min = mytime;
			
		} // not info data

		
		// Debug
		//if( mytime-ebis_time > 40e6 ) {
		//	std::cout << "Entry #" << i << ": time = " << mytime << std::endl;
		//	if( in_data->IsInfo() ) std::cout << "\tInfo code = " << (int)info_data->GetCode() << std::endl;
		//	else if( in_data->IsAsic() ) {
		//		std::cout << "\tAsic = " << (int)asic_data->GetAsic() << std::endl;
		//		std::cout << "\tMod  = " << (int)asic_data->GetModule() << std::endl;
		//		std::cout << "\tCh   = " << (int)asic_data->GetChannel() << std::endl;
		//	}
		//	else if( in_data->IsCaen() ) std::cout << "\tCAEN = " << (int)caen_data->GetModule() << std::endl;
		//	else std::cout << "\tUnknown event type" << std::endl;
		//}
		
		//------------------------------
		//  check if last datum from this event and do some cleanup
		//------------------------------
		
		if( input_tree->GetEntry(i+1) ) {
					
			time_diff = in_data->GetTime() - time_first;

			// window = time_stamp_first + time_window
			if( time_diff > build_window )
				flag_close_event = true; // set flag to close this event

			// we've gone on to the next file in the chain
			else if( time_diff < 0 )
				flag_close_event = true; // set flag to close this event
				
			// Fill tdiff hist only for real data
			if( !in_data->IsInfo() ) {
				
				tdiff->Fill( time_diff );
				if( mythres )
					tdiff_clean->Fill( time_diff );
			
			}

		}
		
		// Debug
		//if( mytime-ebis_time > 40e6 ) {
		//	std::cout << "\tNext time = " << in_data->GetTime() << std::endl;
		//	std::cout << "\ttime_diff = " << time_diff << std::endl;
		//	std::cout << "\ttime-EBIS = " << mytime-ebis_time << std::endl;
		//	std::cout << "\thit_ctr = " << hit_ctr << std::endl;
		//	std::cout << "\tEvent open? " << event_open << std::endl;
		//	std::cout << "\tClose event? " << flag_close_event << std::endl;
		//}
		
		//----------------------------
		// if close this event or last entry
		//----------------------------
		if( flag_close_event || (i+1) == n_entries ) {

			// If we opened the event, then sort it out
			if( event_open ) {
			
				//----------------------------------
				// Build array events, recoils, etc
				//----------------------------------
				ArrayFinder();		// add an ArrayEvt for each n/p pair
				RecoilFinder();		// add a RecoilEvt for each dE-E
				MwpcFinder();		// add an MwpcEvt for pair of TAC events
				ElumFinder();		// add an ElumEvt for each S1 event
				ZeroDegreeFinder();	// add a ZeroDegreeEvt for each dE-E

				// ------------------------------------
				// Add timing and fill the ISSEvts tree
				// ------------------------------------
				write_evts->SetEBIS( ebis_time );
				write_evts->SetT1( t1_time );
				if( write_evts->GetArrayMultiplicity() ||
					write_evts->GetArrayPMultiplicity() ||
					write_evts->GetRecoilMultiplicity() ||
					write_evts->GetMwpcMultiplicity() ||
					write_evts->GetElumMultiplicity() ||
					write_evts->GetZeroDegreeMultiplicity() )
					output_tree->Fill();

				// Clean up if the next event is going to make the tree full
				if( output_tree->MemoryFull(30e6) )
					output_tree->DropBaskets();
			}
			
			//--------------------------------------------------
			// clear values of arrays to store intermediate info
			//--------------------------------------------------
			Initialise();
			
		} // if close event
				
		// Progress bar
		bool update_progress = false;
		if( n_entries < 200 )
			update_progress = true;
		else if( i % (n_entries/100) == 0 || i+1 == n_entries )
			update_progress = true;
		
		if( update_progress ) {

			// Percent complete
			float percent = (float)(i+1)*100.0/(float)n_entries;

			// Progress bar in GUI
			if( _prog_ ) {
				
				prog->SetPosition( percent );
				gSystem->ProcessEvents();
				
			}

			// Progress bar in terminal
			std::cout << " " << std::setw(6) << std::setprecision(4);
			std::cout << percent << "%    \r";
			std::cout.flush();

		}
		
		
	} // End of main loop over TTree to process raw MIDAS data entries (for n_entries)
	
	// TODO -> if we end on a pause with no resume, add any remaining time to the dead time
	
	//--------------------------
	// Clean up
	//--------------------------
	std::stringstream ss_log;
	ss_log << "\n ISSEventBuilder finished..." << std::endl;
	ss_log << "  ASIC data packets = " << n_asic_data << std::endl;
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {
		ss_log << "   Module " << i << " pause = " << n_asic_pause[i] << std::endl;
		ss_log << "           resume = " << n_asic_resume[i] << std::endl;
		ss_log << "        dead time = " << (double)asic_dead_time[i]/1e9 << " s" << std::endl;
		ss_log << "        live time = " << (double)(asic_time_stop[i]-asic_time_start[i])/1e9 << " s" << std::endl;
	}
	ss_log << "  CAEN data packets = " << n_caen_data << std::endl;
	for( unsigned int i = 0; i < set->GetNumberOfCAENModules(); ++i ) {
		ss_log << "   Module " << i << " live time = ";
		ss_log << (double)(caen_time_stop[i]-caen_time_start[i])/1e9;
		ss_log << " s" << std::endl;
	}
	ss_log << "  Info data packets = " << n_info_data << std::endl;
	ss_log << "   Array p/n-side correlated events = " << array_ctr << std::endl;
	ss_log << "   Array p-side only events = " << arrayp_ctr << std::endl;
	ss_log << "   Recoil events = " << recoil_ctr << std::endl;
	ss_log << "   MWPC events = " << mwpc_ctr << std::endl;
	ss_log << "   ELUM events = " << elum_ctr << std::endl;
	ss_log << "   ZeroDegree events = " << zd_ctr << std::endl;
	ss_log << "   CAEN pulser = " << n_caen_pulser << std::endl;
	ss_log << "   FPGA pulser" << std::endl;
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i )
		ss_log << "    Module " << i << " = " << n_fpga_pulser[i] << std::endl;
	ss_log << "   ASIC pulser" << std::endl;
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i )
		ss_log << "    Module " << i << " = " << n_asic_pulser[i] << std::endl;
	ss_log << "   EBIS events = " << n_ebis << std::endl;
	ss_log << "   T1 events = " << n_t1 << std::endl;
	ss_log << "  Tree entries = " << output_tree->GetEntries() << std::endl;

	std::cout << ss_log.str();
	if( log_file.is_open() && flag_input_file ) log_file << ss_log.str();
	
	std::cout << " Writing output file...\r";
	std::cout.flush();
	
	// Force the rest of the events in the buffer to disk
	output_tree->FlushBaskets();
	output_file->Write( 0, TObject::kWriteDelete );
	//output_file->Print();
	//output_file->Close();
	
	// Dump the input buffers
	input_tree->DropBaskets();

	std::cout << " Writing output file... Done!" << std::endl << std::endl;

	return n_entries;
	
}

////////////////////////////////////////////////////////////////////////////////
/// This function processes a series of vectors that are populated in a given build window, and deals with the signals accordingly. This is currently done on a case-by-case basis i.e. each different number of p-side and n-side hits is dealt with in it's own section. Charge addback is implemented for neighbouring strips that fall within a prompt coincidence window defined by the user in the ISSSettings file.
void ISSEventBuilder::ArrayFinder() {
	
	std::vector<unsigned int> pindex; // Stores the index of the p-side hits in a given module and row
	std::vector<unsigned int> nindex; // Stores the index of the n-side hits in a given module and row
	int pmax_idx, nmax_idx;		// Stores the maximum-energy index for the p-side and n-side hits
	int ptmp_idx, ntmp_idx;		// Stores a temporary index for the p-side and n-side hits
	float pmax_en, nmax_en;		// Stores the maximum energy for the p-side and n-side hits
	float psum_en, nsum_en;		// Stores the summed energy for the p-side and n-side hits

	
	// Do each module and row individually
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {
		
		for( unsigned int j = 0; j < set->GetNumberOfArrayRows(); ++j ) {
			
			// Empty the array of indexes
			pindex.clear();
			nindex.clear();
			std::vector<unsigned int>().swap(pindex);
			std::vector<unsigned int>().swap(nindex);
			pmax_idx = nmax_idx = -1;
			pmax_en = nmax_en = -99999.;
			psum_en = nsum_en = 0;

			// Loop over p-side events
			for( unsigned int k = 0; k < pen_list.size(); ++k ) {
				
				// Check if it is the module and row we want
				if( pmod_list.at(k) == (int)i && prow_list.at(k) == (int)j ) {
					
					// Put in the index
					pindex.push_back( k );
					
					// Check if it is max energy
					if( pen_list.at(k) > pmax_en ){
					
						pmax_en = pen_list.at(k);
						pmax_idx = k;
					
					}
					
				}
				
			}
			
			// Loop over n-side events
			for( unsigned int l = 0; l < nen_list.size(); ++l ) {
				
				// Check if it is the module and row we want
				if( nmod_list.at(l) == (int)i && nrow_list.at(l) == (int)j ) {
				
					// Put in the index
					nindex.push_back( l );
					
					// Check if it is max energy
					if( nen_list.at(l) > nmax_en ){
					
						nmax_en = nen_list.at(l);
						nmax_idx = l;
					
					}

				}
				
			}
			
			// Multiplicty hist
			if( pindex.size() || nindex.size() )
				pn_mult[i][j]->Fill( pindex.size(), nindex.size() );

			// Time difference hists (not prompt)
			// p-n time
			for( unsigned int k = 0; k < pindex.size(); ++k ) {
				for( unsigned int l = 0; l < nindex.size(); ++l ) {
					pn_td[i][j]->Fill( ptd_list.at( pindex.at(k) ) - ntd_list.at( nindex.at(l) ) );
					pn_td_Ep[i][j]->Fill( ptd_list.at( pindex.at(k) ) - ntd_list.at( nindex.at(l) ), pen_list.at( pindex.at(k) ) );
					pn_td_En[i][j]->Fill( ptd_list.at( pindex.at(k) ) - ntd_list.at( nindex.at(l) ), nen_list.at( nindex.at(l) ) );
				}
			}
			
			// p-p time
			for( unsigned int k = 0; k < pindex.size(); ++k )
				for( unsigned int l = k+1; l < pindex.size(); ++l )
					pp_td[i][j]->Fill( ptd_list.at( pindex.at(k) ) - ptd_list.at( pindex.at(l) ) );

			// n-n time
			for( unsigned int k = 0; k < nindex.size(); ++k )
				for( unsigned int l = k+1; l < nindex.size(); ++l )
					nn_td[i][j]->Fill( ntd_list.at( nindex.at(k) ) - ntd_list.at( nindex.at(l) ) );
					
			// Easy case, p == 1 vs n == 1
			if( pindex.size() == 1 && nindex.size() == 1 ) {
			
				// Fill 1p1n histogram
				pn_11[i][j]->Fill( pen_list.at( pindex.at(0) ), nen_list.at( nindex.at(0) ) );
			
				// Prompt coincidence
				if ( TMath::Abs( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(0) ) ) < set->GetArrayHitWindow() ){
				
					// Fill 1p1n prompt histogram
					pn_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(0) ) );
				
					// Fill single event as a nice p/n correlation
					array_evt->SetEvent( pen_list.at( pindex.at(0) ),
										 nen_list.at( nindex.at(0) ),
										 pid_list.at( pindex.at(0) ),
										 nid_list.at( nindex.at(0) ),
										 ptd_list.at( pindex.at(0) ),
										 ntd_list.at( nindex.at(0) ),
										 i, j );
					
					write_evts->AddEvt( array_evt );
					array_ctr++;

					// High n-side threshold situation, fill p-only event
					arrayp_evt->CopyEvent( array_evt );
					write_evts->AddEvt( arrayp_evt );
					arrayp_ctr++;
				
				}
				else{
					// Not prompt coincidence, assume pure p-side
					arrayp_evt->SetEvent( pen_list.at( pindex.at(0) ),
									  0,
									  pid_list.at( pindex.at(0) ),
									  5,
									  ptd_list.at( pindex.at(0) ),
									  0,
									  i, j );

					write_evts->AddEvt( arrayp_evt );
					arrayp_ctr++;
				}

			}
			
			// p == 1 vs n == 0
			else if( pindex.size() == 1 && nindex.size() == 0 ) {
				
				// High n-side threshold situation, fill p-only event
				arrayp_evt->SetEvent( pen_list.at( pindex.at(0) ),
									  0,
									  pid_list.at( pindex.at(0) ),
									  5,
									  ptd_list.at( pindex.at(0) ),
									  0,
									  i, j );

				write_evts->AddEvt( arrayp_evt );
				arrayp_ctr++;

			}
			
			// p == 2 vs n == 1
			else if( pindex.size() == 2 && nindex.size() == 1 ) {

				pn_21[i][j]->Fill( pen_list.at( pindex.at(0) ), nen_list.at( nindex.at(0) ) );
				pn_21[i][j]->Fill( pen_list.at( pindex.at(1) ), nen_list.at( nindex.at(0) ) );
				
				// Neighbour strips and prompt coincidence (p-sides)
				if( TMath::Abs( pid_list.at( pindex.at(0) ) - pid_list.at( pindex.at(1) ) ) == 1 &&
				    TMath::Abs( ptd_list.at( pindex.at(0) ) - ptd_list.at( pindex.at(1) ) ) < set->GetArrayHitWindow() ) {
				    
				    // Fill pp prompt histogram
					pp_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(0) ) - ptd_list.at( pindex.at(1) ) );
					
					// Simple sum of both energies, cross-talk not included yet
					psum_en  = pen_list.at( pindex.at(0) );
					psum_en += pen_list.at( pindex.at(1) );

					// Check that p's and n are coincident
					if ( TMath::Abs( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(0) ) ) < set->GetArrayHitWindow() &&
					     TMath::Abs( ptd_list.at( pindex.at(1) ) - ntd_list.at( nindex.at(0) ) ) < set->GetArrayHitWindow() ){
					
						// Fill pn prompt histogram
						pn_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(0) ) );
						pn_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(1) ) - ntd_list.at( nindex.at(0) ) );
						
						// Fill addback histogram
						pn_pab[i][j]->Fill( psum_en, nen_list.at( nindex.at(0) ) );
						
						// Fill the addback event
						array_evt->SetEvent( psum_en,
											 nen_list.at( nindex.at(0) ),
											 pid_list.at( pmax_idx ),
											 nid_list.at( nindex.at(0) ),
											 ptd_list.at( pmax_idx ),
											 ntd_list.at( nindex.at(0) ),
											 i, j );

						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
					
					}
					else{
						
						// p's coincident, no n coincidence -> do a pp event
						arrayp_evt->SetEvent( psum_en,
									  0,
									  pid_list.at( pmax_idx ),
									  5,
									  ptd_list.at( pmax_idx ),
									  0,
									  i, j );

						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
					
					}

				}
				
				// Non-neighbour strips or not coincident -> don't addback!
				else {
					// p1 and n coincident
					if ( TMath::Abs( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(0) ) ) < set->GetArrayHitWindow() ){
						
						// Fill pn prompt histogram
						pn_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(0) ) );
						
						// Fill single event as a nice p/n correlation
						array_evt->SetEvent( pen_list.at( pindex.at(0) ),
											 nen_list.at( nindex.at(0) ),
											 pid_list.at( pindex.at(0) ),
											 nid_list.at( nindex.at(0) ),
											 ptd_list.at( pindex.at(0) ),
											 ntd_list.at( nindex.at(0) ),
											 i, j );
						
						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
						
					}
					
					// p2 and n coincident
					else if ( TMath::Abs( ptd_list.at( pindex.at(1) ) - ntd_list.at( nindex.at(0) ) ) < set->GetArrayHitWindow() ){
					
						// Fill pn prompt histogram
						pn_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(1) ) - ntd_list.at( nindex.at(0) ) );
						
						// Fill single event as a nice p/n correlation
						array_evt->SetEvent( pen_list.at( pindex.at(1) ),
											 nen_list.at( nindex.at(0) ),
											 pid_list.at( pindex.at(1) ),
											 nid_list.at( nindex.at(0) ),
											 ptd_list.at( pindex.at(1) ),
											 ntd_list.at( nindex.at(0) ),
											 i, j );
						
						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
					
					}
					
					// Nothing in coincidence - take max energy p
					else{
						// Do a p-only event
						arrayp_evt->SetEvent( pen_list.at( pmax_idx ),
									  0,
									  pid_list.at( pmax_idx ),
									  5,
									  ptd_list.at( pmax_idx ),
									  0,
									  i, j );

						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
					}
					
				}
								
			}
			
			// p == 1 vs n == 2
			else if( pindex.size() == 1 && nindex.size() == 2 ) {

				pn_12[i][j]->Fill( pen_list.at( pindex.at(0) ), nen_list.at( nindex.at(0) ) );
				pn_12[i][j]->Fill( pen_list.at( pindex.at(0) ), nen_list.at( nindex.at(1) ) );
				
				// Neighbour strips and prompt coincidence
				if( TMath::Abs( nid_list.at( nindex.at(0) ) - nid_list.at( nindex.at(1) ) ) == 1 &&
				    TMath::Abs( ntd_list.at( nindex.at(0) ) - ntd_list.at( nindex.at(1) ) ) < set->GetArrayHitWindow() ) {
				    
				    // Fill nn prompt histogram
					nn_td_prompt[i][j]->Fill( ntd_list.at( nindex.at(0) ) - ntd_list.at( nindex.at(1) ) );
						
					// Simple sum of both energies, cross-talk not included yet
					nsum_en  = nen_list.at( nindex.at(0) );
					nsum_en += nen_list.at( nindex.at(1) );

					// Check that p and n are coincident
					if ( TMath::Abs( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(0) ) ) < set->GetArrayHitWindow() &&
					     TMath::Abs( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(1) ) ) < set->GetArrayHitWindow() ){
					
						// Fill pn prompt histogram
						pn_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(0) ) );
						pn_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(1) ) );

						// Fill addback histogram
						pn_nab[i][j]->Fill( pen_list.at( pindex.at(0) ), nsum_en );

						// Fill the addback event
						array_evt->SetEvent( pen_list.at( pindex.at(0) ),
											 nsum_en,
											 pid_list.at( pindex.at(0) ),
											 nid_list.at( nmax_idx ),
											 ptd_list.at( pindex.at(0) ),
											 ntd_list.at( nmax_idx ),
											 i, j );

						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
					}
					else{
						// Have a pure p event and throw out both n's
						arrayp_evt->SetEvent( pen_list.at( pindex.at(0) ),
									  0,
									  pid_list.at( pindex.at(0) ),
									  5,
									  ptd_list.at( pindex.at(0) ),
									  0,
									  i, j );

						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
						
					}

				}
				
				// Non-neighbour strips
				else {
				
					// n1 and p coincident
					if ( TMath::Abs( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(0) ) ) < set->GetArrayHitWindow() ){
						
						// Fill pn prompt histogram
						pn_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(0) ) );
						
						// Fill single event as a nice p/n correlation
						array_evt->SetEvent( pen_list.at( pindex.at(0) ),
											 nen_list.at( nindex.at(0) ),
											 pid_list.at( pindex.at(0) ),
											 nid_list.at( nindex.at(0) ),
											 ptd_list.at( pindex.at(0) ),
											 ntd_list.at( nindex.at(0) ),
											 i, j );
						
						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
					}
					// n2 and p coincident
					else if ( TMath::Abs( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(1) ) ) < set->GetArrayHitWindow() ){
					
						// Fill pn prompt histogram
						pn_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(1) ) );
					
						// Fill single event as a nice p/n correlation
						array_evt->SetEvent( pen_list.at( pindex.at(0) ),
											 nen_list.at( nindex.at(1) ),
											 pid_list.at( pindex.at(0) ),
											 nid_list.at( nindex.at(1) ),
											 ptd_list.at( pindex.at(0) ),
											 ntd_list.at( nindex.at(1) ),
											 i, j );
						
						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
					}
					
					// No coincidence - keep p
					else{
						arrayp_evt->SetEvent( pen_list.at( pindex.at(0) ),
									  0,
									  pid_list.at( pindex.at(0) ),
									  5,
									  ptd_list.at( pindex.at(0) ),
									  0,
									  i, j );

						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
					}

				}

			}
			
			// p == 2 vs n == 0 - p-side only
			else if( pindex.size() == 2 && nindex.size() == 0 ) {
				
				// Neighbour strips and prompt coincidence
				if( TMath::Abs( pid_list.at( pindex.at(0) ) - pid_list.at( pindex.at(1) ) ) == 1 &&
				    TMath::Abs( ptd_list.at( pindex.at(0) ) - ptd_list.at( pindex.at(1) ) ) < set->GetArrayHitWindow() ) {
					
					// Fill pp prompt histogram
					pp_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(0) ) - ptd_list.at( pindex.at(1) ) );
					
					// Simple sum of both energies, cross-talk not included yet
					psum_en  = pen_list.at( pindex.at(0) );
					psum_en += pen_list.at( pindex.at(1) );

					// Fill addback histogram
					pn_pab[i][j]->Fill( psum_en, -1 );

					// Fill add back event
					arrayp_evt->SetEvent( psum_en,
										  0,
										  pid_list.at( pmax_idx ),
										  5,
										  ptd_list.at( pmax_idx ),
										  0,
										  i, j );

					write_evts->AddEvt( arrayp_evt );
					arrayp_ctr++;

				}
				
				// Non-neighbour strips
				else {
					
					// Fill maximum energy only
					arrayp_evt->SetEvent( pen_list.at( pmax_idx ),
										  0,
										  pid_list.at( pmax_idx ),
										  5,
										  ptd_list.at( pmax_idx ),
										  0,
										  i, j );

					write_evts->AddEvt( arrayp_evt );
					arrayp_ctr++;

				}

			}
			
			// p == 2 vs n == 2
			else if( pindex.size() == 2 && nindex.size() == 2 ) {

				pn_22[i][j]->Fill( pen_list.at( pindex.at(0) ), nen_list.at( nindex.at(0) ) );
				pn_22[i][j]->Fill( pen_list.at( pindex.at(0) ), nen_list.at( nindex.at(1) ) );
				pn_22[i][j]->Fill( pen_list.at( pindex.at(1) ), nen_list.at( nindex.at(0) ) );
				pn_22[i][j]->Fill( pen_list.at( pindex.at(1) ), nen_list.at( nindex.at(1) ) );
				
				// Neighbour strips for both p and n and prompt coincidences for p and n respectively
				if( TMath::Abs( pid_list.at( pindex.at(0) ) - pid_list.at( pindex.at(1) ) ) == 1 &&
				    TMath::Abs( nid_list.at( nindex.at(0) ) - nid_list.at( nindex.at(1) ) ) == 1 && 
				    TMath::Abs( ptd_list.at( pindex.at(0) ) - ptd_list.at( pindex.at(1) ) ) < set->GetArrayHitWindow() &&
				    TMath::Abs( ntd_list.at( nindex.at(0) ) - ntd_list.at( nindex.at(1) ) ) < set->GetArrayHitWindow() ) {
				    
				    // Fill pp and nn prompt histograms
					pp_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(0) ) - ptd_list.at( pindex.at(1) ) );
					nn_td_prompt[i][j]->Fill( ntd_list.at( nindex.at(0) ) - ntd_list.at( nindex.at(1) ) );
					
					
					// Simple sum of both energies, cross-talk not included yet
					psum_en  = pen_list.at( pindex.at(0) );
					psum_en += pen_list.at( pindex.at(1) );
					nsum_en  = nen_list.at( nindex.at(0) );
					nsum_en += nen_list.at( nindex.at(1) );
					
					// Check p and n prompt with each other
					if( TMath::Abs( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(0) ) ) < set->GetArrayHitWindow() &&
					    TMath::Abs( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(1) ) ) < set->GetArrayHitWindow() &&
					    TMath::Abs( ptd_list.at( pindex.at(1) ) - ntd_list.at( nindex.at(0) ) ) < set->GetArrayHitWindow() &&
					    TMath::Abs( ptd_list.at( pindex.at(1) ) - ntd_list.at( nindex.at(1) ) ) < set->GetArrayHitWindow() ){
					
						// Fill pn prompt histogram
						pn_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(0) ) );
						pn_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(1) ) );
						pn_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(1) ) - ntd_list.at( nindex.at(0) ) );
						pn_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(1) ) - ntd_list.at( nindex.at(1) ) );

						// Fill addback histogram
						pn_ab[i][j]->Fill( psum_en, nsum_en );

						// Fill the addback event
						array_evt->SetEvent( psum_en,
											 nsum_en,
											 pid_list.at( pmax_idx ),
											 nid_list.at( nmax_idx ),
											 ptd_list.at( pmax_idx ),
											 ntd_list.at( nmax_idx ),
											 i, j );

						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;

					}
					else{
					
						// Discard two n's and just take two p's
						// Fill addback histogram
						pn_pab[i][j]->Fill( psum_en, -1 );
						
						// Fill the addback event
						arrayp_evt->SetEvent( psum_en,
											  0,
											  pid_list.at( pmax_idx ),
											  5,
											  ptd_list.at( pmax_idx ),
											  0,
											  i, j );

						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
					
					}

				}
				
				// Neighbour strips - p-side only and only p-prompt coincidences
				else if( TMath::Abs( pid_list.at( pindex.at(0) ) - pid_list.at( pindex.at(1) ) ) == 1 &&
				         TMath::Abs( ptd_list.at( pindex.at(0) ) - ptd_list.at( pindex.at(1) ) ) < set->GetArrayHitWindow() ) {
					
					// Fill pp prompt histogram
					pp_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(0) ) - ptd_list.at( pindex.at(1) ) );
					
					// Simple sum of both energies, cross-talk not included yet
					psum_en  = pen_list.at( pindex.at(0) );
					psum_en += pen_list.at( pindex.at(1) );
					
					// Check if any of the n-sides coincident with p
					// n0 coincident with pp
					if ( TMath::Abs( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(0) ) ) < set->GetArrayHitWindow() &&
						
					     TMath::Abs( ptd_list.at( pindex.at(1) ) - ntd_list.at( nindex.at(0) ) ) < set->GetArrayHitWindow() ){
						// Fill pn prompt histogram
						pn_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(0) ) );
						pn_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(1) ) - ntd_list.at( nindex.at(0) ) );
						
						// Fill addback histogram
						pn_pab[i][j]->Fill( psum_en, nen_list.at(0) );

						// Fill the addback event for p-side, but max for n-side
						array_evt->SetEvent( psum_en,
											 nen_list.at( 0 ),
											 pid_list.at( pmax_idx ),
											 nid_list.at( 0 ),
											 ptd_list.at( pmax_idx ),
											 ntd_list.at( 0 ),
											 i, j );

						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
						
					}
					// n1 coincident with pp
					else if ( TMath::Abs( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(1) ) ) < set->GetArrayHitWindow() &&
					          TMath::Abs( ptd_list.at( pindex.at(1) ) - ntd_list.at( nindex.at(1) ) ) < set->GetArrayHitWindow() ){
						
						// Fill pn prompt histogram
						pn_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(1) ) );
						pn_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(1) ) - ntd_list.at( nindex.at(1) ) );
						
						// Fill addback histogram
						pn_pab[i][j]->Fill( psum_en, nen_list.at( 1 ) );

						// Fill the addback event for p-side, but max for n-side
						array_evt->SetEvent( psum_en,
											 nen_list.at( 1 ),
											 pid_list.at( pmax_idx ),
											 nid_list.at( 1 ),
											 ptd_list.at( pmax_idx ),
											 ntd_list.at( 1 ),
											 i, j );

						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;

					}
					// p's coincident but n's are not with p's or each other
						// Fill addback histogram
					else{
						pn_pab[i][j]->Fill( psum_en, -1 );
				
						// No n-sides coincident -> p-sides only
						arrayp_evt->SetEvent( psum_en,
										  0,
										  pid_list.at( pmax_idx ),
										  5,
										  ptd_list.at( pmax_idx ),
										  0,
										  i, j );

						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;

					}

				}

				// Neighbour strips - n-side only prompt and p not prompt
				else if( TMath::Abs( nid_list.at( nindex.at(0) ) - nid_list.at( nindex.at(1) ) ) == 1 &&
				         TMath::Abs( ntd_list.at( nindex.at(0) ) - ntd_list.at( nindex.at(1) ) ) < set->GetArrayHitWindow()) {
					
					// Fill nn prompt histogram
					nn_td_prompt[i][j]->Fill( ntd_list.at( nindex.at(0) ) - ntd_list.at( nindex.at(1) ) );
					
					// Simple sum of both energies, cross-talk not included yet
					nsum_en  = nen_list.at( nindex.at(0) );
					nsum_en += nen_list.at( nindex.at(1) );

					// Check p0 with n sides
					if ( TMath::Abs( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(0) ) ) < set->GetArrayHitWindow() &&
					     TMath::Abs( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(1) ) ) < set->GetArrayHitWindow() ){
					
						// Fill pn prompt histogram
						pn_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(0) ) );
						pn_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(1) ) );

						// Fill addback histogram
						pn_nab[i][j]->Fill( pen_list.at( pindex.at(0) ), nsum_en );

						// Fill the addback event
						array_evt->SetEvent( pen_list.at( pindex.at(0) ),
											 nsum_en,
											 pid_list.at( pindex.at(0) ),
											 nid_list.at( nmax_idx ),
											 ptd_list.at( pindex.at(0) ),
											 ntd_list.at( nmax_idx ),
											 i, j );

						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;

					}
					
					// Check p1 with n sides
					else if( TMath::Abs( ptd_list.at( pindex.at(1) ) - ntd_list.at( nindex.at(0) ) ) < set->GetArrayHitWindow() &&
					         TMath::Abs( ptd_list.at( pindex.at(1) ) - ntd_list.at( nindex.at(1) ) ) < set->GetArrayHitWindow() ){

						// Fill pn prompt histogram
						pn_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(1) ) - ntd_list.at( nindex.at(0) ) );
						pn_td_prompt[i][j]->Fill( ptd_list.at( pindex.at(1) ) - ntd_list.at( nindex.at(1) ) );

						// Fill addback histogram
						pn_nab[i][j]->Fill( pen_list.at( pindex.at(1) ), nsum_en );

						// Fill the addback event
						array_evt->SetEvent( pen_list.at( pindex.at(1) ),
											 nsum_en,
											 pid_list.at( pindex.at(1) ),
											 nid_list.at( nmax_idx ),
											 ptd_list.at( pindex.at(1) ),
											 ntd_list.at( nmax_idx ),
											 i, j );

						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;

					}
					
					// n sides coincident but neither p coincident with them or each other
					else{
						// ??? TODO
					}

				}
				
				// Non-neighbour strips, maybe two events?
				else {
				
					// Pairing [0,0] because energy difference is smaller than [1,0]
					if( TMath::Abs( pen_list.at( pindex.at(0) ) - nen_list.at( nindex.at(0) ) ) <
					    TMath::Abs( pen_list.at( pindex.at(1) ) - nen_list.at( nindex.at(0) ) ) ) {
						
						// Try filling the [0,0] and then the [1,1]
						for ( int k = 0; k < 2; ++k ){
							ptmp_idx = pindex.at(k);
							ntmp_idx = nindex.at(k);
							
							// Prompt coincidence condition
							if ( TMath::Abs( ptd_list.at( ptmp_idx ) - ntd_list.at( ntmp_idx ) ) < set->GetArrayHitWindow() ){
							
								// Fill pn-prompt coincidence histogram
								pn_td_prompt[i][j]->Fill( ptd_list.at( ptmp_idx ) - ntd_list.at( ntmp_idx ) );
								
								// Fill addback histogram
								pn_ab[i][j]->Fill( pen_list.at( ptmp_idx ), nen_list.at( ntmp_idx ) );

								array_evt->SetEvent( pen_list.at( ptmp_idx ),
													 nen_list.at( ntmp_idx ),
													 pid_list.at( ptmp_idx ),
													 nid_list.at( ntmp_idx ),
													 ptd_list.at( ptmp_idx ),
													 ntd_list.at( ntmp_idx ),
													 i, j );

								write_evts->AddEvt( array_evt );
								array_ctr++;

								// High n-side threshold situation, fill p-only event
								arrayp_evt->CopyEvent( array_evt );
								write_evts->AddEvt( arrayp_evt );
								arrayp_ctr++;
							}
							else{
								// [0,0]/[1,1] not in prompt coincidence, so just store p as separate event
								arrayp_evt->SetEvent( pen_list.at( ptmp_idx ),
													  0,
													  pid_list.at( ptmp_idx ),
													  5,
													  ptd_list.at( ptmp_idx ),
													  0,
													  i, j );

								write_evts->AddEvt( arrayp_evt );
								arrayp_ctr++;
							}
							
						}
						
					}

					// If not, pair [0,1] and [1,0]
					else {
						
						for ( int k = 0; k < 2; ++k ){
							// Fill the [0,1]/[1,0]
							ptmp_idx = pindex.at(k);
							ntmp_idx = nindex.at(1 - k);
							
							// Prompt coincidence condition
							if ( TMath::Abs( ptd_list.at( ptmp_idx ) - ntd_list.at( ntmp_idx ) ) < set->GetArrayHitWindow() ){
								
								// Fill pn-prompt coincidence histogram
								pn_td_prompt[i][j]->Fill( ptd_list.at( ptmp_idx ) - ntd_list.at( ntmp_idx ) );
								
								// Fill addback histogram
								pn_ab[i][j]->Fill( pen_list.at( ptmp_idx ), nen_list.at( ntmp_idx ) );

								array_evt->SetEvent( pen_list.at( ptmp_idx ),
													 nen_list.at( ntmp_idx ),
													 pid_list.at( ptmp_idx ),
													 nid_list.at( ntmp_idx ),
													 ptd_list.at( ptmp_idx ),
													 ntd_list.at( ntmp_idx ),
													 i, j );

								write_evts->AddEvt( array_evt );
								array_ctr++;

								// High n-side threshold situation, fill p-only event
								arrayp_evt->CopyEvent( array_evt );
								write_evts->AddEvt( arrayp_evt );
								arrayp_ctr++;
							}
							else{
							
								// [0,0]/[1,1] not in prompt coincidence, so just store p as separate event
								arrayp_evt->SetEvent( pen_list.at( ptmp_idx ),
													  0,
													  pid_list.at( ptmp_idx ),
													  5,
													  ptd_list.at( ptmp_idx ),
													  0,
													  i, j );

								write_evts->AddEvt( arrayp_evt );
								arrayp_ctr++;
								
							}

						}

					}

				}

			}
			
			// Higher multiplicities need to be dealt with
			// For now, we bodge!! Just take the maximum energy
			// But make sure that both p- and n-sides are good
			// TODO prompt coincidences
			else if( pmax_idx >= 0 && nmax_idx >= 0 ){

				array_evt->SetEvent( pen_list.at( pmax_idx ),
									 nen_list.at( nmax_idx ),
									 pid_list.at( pmax_idx ),
									 nid_list.at( nmax_idx ),
									 ptd_list.at( pmax_idx ),
									 ntd_list.at( nmax_idx ),
									 i, j );

				write_evts->AddEvt( array_evt );
				array_ctr++;

				// High n-side threshold situation, fill p-only event
				arrayp_evt->CopyEvent( array_evt );
				write_evts->AddEvt( arrayp_evt );
				arrayp_ctr++;

			}
			
			// Histogram for n vs p-side max energies
			pn_max[i][j]->Fill( pmax_en, nmax_en );
			
		} // j; row

	} // i; module
	
	// Clean up
	//delete array_evt;
	//delete arrayp_evt;
	
	return;

}

////////////////////////////////////////////////////////////////////////////////
/// This function takes a series of E and dE signals on the silicon recoil detector and determines what hits to keep from these using sensible conditions including a prompt coincidence window. Signals are triggered by the dE detector, but if a corresponding E signal is not found, then a hit at E = 0 is still recorded.
void ISSEventBuilder::RecoilFinder() {

	// Checks to prevent re-using events
	std::vector<unsigned int> index;
	bool flag_skip;
	
	// Loop over recoil events
	for( unsigned int i = 0; i < ren_list.size(); ++i ) {
		
		// Find the dE event, usually the trigger
		if( rid_list[i] == 0 || set->GetNumberOfRecoilLayers() == 13 ){
			
			recoil_evt->ClearEvent();
			recoil_evt->SetdETime( rtd_list[i] );
			recoil_evt->SetSector( rsec_list[i] );
			recoil_evt->AddRecoil( ren_list[i], rid_list[i] );
			
			index.push_back(i);

			// Look for matching E events
			for( unsigned int j = 0; j < ren_list.size(); ++j ) {

				// Check if we already used this hit
				flag_skip = false;
				for( unsigned int k = 0; k < index.size(); ++k )
					if( index[k] == j ) flag_skip = true;
				
				// Found a match
				// ^^^ Not sure if this will work with the ionisation chamber!
				if( i != j && 		// Not looking at the same hit
					!flag_skip &&	// Not looking at a previously-used hit
				    rsec_list[i] == rsec_list[j] &&		// They are in the same sector
				    rid_list[i] != rid_list[j] &&		// They are not in the same layer
				    TMath::Abs( rtd_list[i] - rtd_list[j] ) < set->GetRecoilHitWindow() // The hits lie within the recoil hit window
				   ){
				
					index.push_back(j);
					recoil_evt->AddRecoil( ren_list[j], rid_list[j] );
					recoil_EdE[rsec_list[i]]->Fill( ren_list[j], ren_list[i] );
					
					if( rid_list[j] == (int)set->GetRecoilEnergyLossDepth() ){
					
						recoil_evt->SetETime( rtd_list[j] );
						
					}
					
				}
				
			}
			
			// Histogram the recoils
			recoil_EdE[rsec_list[i]]->Fill( recoil_evt->GetEnergyRest( set->GetRecoilEnergyLossDepth() ),
								recoil_evt->GetEnergyLoss( set->GetRecoilEnergyLossDepth() - 1 ) );
			recoil_dEsum[rsec_list[i]]->Fill( recoil_evt->GetEnergyTotal(),
								recoil_evt->GetEnergyLoss( set->GetRecoilEnergyLossDepth() - 1 ) );
			recoil_E_singles[rsec_list[i]]->Fill( recoil_evt->GetEnergyRest( set->GetRecoilEnergyLossDepth() ) );
			recoil_dE_singles[rsec_list[i]]->Fill( recoil_evt->GetEnergyLoss( set->GetRecoilEnergyLossDepth() - 1 ) );
			
			// Fill the tree and get ready for next recoil event
			write_evts->AddEvt( recoil_evt );
			recoil_ctr++;

		}
		
	}
	
	// Clean up
	//delete recoil_evt;
	
	return;

}

////////////////////////////////////////////////////////////////////////////////
/// Assesses the validity of hits in the MWPC detector
void ISSEventBuilder::MwpcFinder() {

	// Checks to prevent re-using events
	std::vector<unsigned int> index;
	bool flag_skip;
	int tac_diff;

	// Loop over MWPC events
	for( unsigned int i = 0; i < mwpctac_list.size(); ++i ) {

		// TAC singles spectra
		mwpc_tac_axis[mwpcaxis_list[i]][mwpcid_list[i]]->Fill( mwpctac_list[i] );

		// Get first TAC of the axis
		if( mwpcid_list[i] == 0 ){

			// Look for matching pair
			for( unsigned int j = 0; j < mwpctac_list.size(); ++j ) {

				// Check if we already used this hit
				flag_skip = false;
				for( unsigned int k = 0; k < index.size(); ++k )
					if( index[k] == j ) flag_skip = true;

				// Found a match
				if( i != j && mwpcid_list[j] == 1 && !flag_skip &&
				    mwpcaxis_list[i] == mwpcaxis_list[j] ){
					
					index.push_back(j);
					tac_diff = (int)mwpctac_list[i] - (int)mwpctac_list[j];
					mwpc_evt->SetEvent( tac_diff, mwpcaxis_list[i], mwpctd_list[i] );
					
					// MWPC profiles, i.e TAC difference spectra
					mwpc_hit_axis[mwpcaxis_list[i]]->Fill( tac_diff );
					
					// Only make one TAC event for a given pair
					break;
					
				}
				
			}
			
			// Write event to tree
			write_evts->AddEvt( mwpc_evt );
			mwpc_ctr++;

		}

	}
	
	// If we have a 2 axis system, do an x-y plot
	if( write_evts->GetMwpcMultiplicity() == 2 ) {
		
		mwpc_pos->Fill( write_evts->GetMwpcEvt(0)->GetTacDiff(),
					    write_evts->GetMwpcEvt(1)->GetTacDiff() );
		
	}

	// Clean up
	//delete mwpc_evt;

	return;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Assesses the validity of events in the ELUM detector
void ISSEventBuilder::ElumFinder() {
	
	// Loop over ELUM events
	for( unsigned int i = 0; i < een_list.size(); ++i ) {

		// Set the ELUM event (nice and easy)
		elum_evt->SetEvent( een_list[i], 0,
							esec_list[i], etd_list[i] );

		// Write event to tree
		write_evts->AddEvt( elum_evt );
		elum_ctr++;
		
		// Histogram the data
		elum->Fill( esec_list[i], een_list[i] );
		
		/*if ( een_list[i] > 8000 ){
			std::cout << std::setw(2) << esec_list[i] <<
			std::setw(8) << een_list[i] <<
			std::setw(16) << etd_list[i] << std::endl;
		}*/
		
	}
	
	// Clean up
	//delete elum_evt;
	
	return;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Assesses the validity of events in the zero degree detector. All dE events are recorded because they act
/// as the trigger, even though there may not be a corresponding E signal on this detector. Also imposes a 
/// prompt coincidence on these hits, which can be altered in the ISSSettings file
void ISSEventBuilder::ZeroDegreeFinder() {

	// Checks to prevent re-using events
	std::vector<unsigned int> index;
	bool flag_skip;
	
	// Loop over ZeroDegree events
	for( unsigned int i = 0; i < zen_list.size(); ++i ) {

		// Find the dE event, usually the trigger
		if( zid_list[i] == 0 ){
			
			zd_evt->ClearEvent();
			zd_evt->SetdETime( ztd_list[i] );
			zd_evt->SetSector( 0 ); // always 0 ZeroDegree
			zd_evt->AddZeroDegree( zen_list[i], 0 );

			// Look for matching dE events
			for( unsigned int j = 0; j < zen_list.size(); ++j ) {

				// Check if we already used this hit
				flag_skip = false;
				for( unsigned int k = 0; k < index.size(); ++k )
					if( index[k] == j ) flag_skip = true;

				
				// Found a match
				if( i != j && zid_list[j] != 0 && !flag_skip &&
				  TMath::Abs( ztd_list[i] - ztd_list[j] ) < set->GetZeroDegreeHitWindow() ){
					
					index.push_back(j);
					zd_evt->AddZeroDegree( zen_list[j], zid_list[j] );
					if( zid_list[j] == 1 ) zd_evt->SetETime( ztd_list[i] );
					
					// Histogram the ZeroDegree
					zd->Fill( zen_list[j], zen_list[i] );

				}
				
			}
			
			// Fill the tree and get ready for next recoil event
			write_evts->AddEvt( zd_evt );
			zd_ctr++;
			
		}
		
	}
	
	// Clean up
	//delete zd_evt;

	return;
	
}

////////////////////////////////////////////////////////////////////////////////
/// *This function doesn't fill any histograms*, but just creates them. Called by the ISSEventBuilder::SetOutput function
void ISSEventBuilder::MakeEventHists(){
	
	std::string hname, htitle;
	std::string dirname, maindirname, subdirname;
	
	// ----------------- //
	// Timing histograms //
	// ----------------- //
	dirname =  "timing";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	tdiff = new TH1F( "tdiff", "Time difference to first trigger;#Delta t [ns]", 1.5e3, -0.5e5, 1.0e5 );
	tdiff_clean = new TH1F( "tdiff_clean", "Time difference to first trigger without noise;#Delta t [ns]", 1.5e3, -0.5e5, 1.0e5 );

	caen_freq = new TProfile( "caen_freq", "Frequency of pulser in CAEN DAQ as a function of time;time [ns];f [Hz]", 10.8e4, 0, 10.8e12 );

	asic_td.resize( set->GetNumberOfArrayModules() );
	asic_freq.resize( set->GetNumberOfArrayModules() );
	asic_sync.resize( set->GetNumberOfArrayModules() );
	asic_pulser_loss.resize( set->GetNumberOfArrayModules() );
	asic_freq_diff.resize( set->GetNumberOfArrayModules() );
	fpga_td.resize( set->GetNumberOfArrayModules() );
	fpga_freq.resize( set->GetNumberOfArrayModules() );
	fpga_sync.resize( set->GetNumberOfArrayModules() );
	fpga_pulser_loss.resize( set->GetNumberOfArrayModules() );
	fpga_freq_diff.resize( set->GetNumberOfArrayModules() );
	
	// Loop over ISS modules
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {

		hname = "asic_td_" + std::to_string(i);
		htitle = "Time difference between ASIC and CAEN pulser events in module ";
		htitle += std::to_string(i) + ";#Delta t [ns]";
		asic_td[i] = new TH1F( hname.data(), htitle.data(), 1.6e3 , -4e3, 4e3 );
		
		hname = "asic_freq_" + std::to_string(i);
		htitle = "Frequency of pulser in ISS DAQ (ASICs) as a function of time in module ";
		htitle += std::to_string(i) + ";time [ns];f [Hz]";
		asic_freq[i] = new TProfile( hname.data(), htitle.data(), 10.8e4, 0, 10.8e12 );
		
		hname = "asic_sync_" + std::to_string(i);
		htitle = "Time difference between ASIC and CAEN events as a function of time in module ";
		htitle += std::to_string(i) + ";time [ns];#Delta t [ns]";
		asic_sync[i] = new TProfile( hname.data(), htitle.data(), 10.8e4, 0, 10.8e12 );
		
		hname = "asic_pulser_loss_" + std::to_string(i);
		htitle = "Number of missing/extra pulser events in ASICs as a function of time in module ";
		htitle += std::to_string(i) + ";time [ns];(-ive CAEN missing, +ive ISS missing)";
		asic_pulser_loss[i] = new TProfile( hname.data(), htitle.data(), 10.8e4, 0, 10.8e12 );
		
		hname = "asic_freq_diff_" + std::to_string(i);
		htitle = "Frequency difference of pulser events in ISS/CAEN DAQs from ASICs as a function of time in module ";
		htitle += std::to_string(i) + ";#time [ns];#Delta f [Hz]";
		asic_freq_diff[i] = new TProfile( hname.data(), htitle.data(), 10.8e4, 0, 10.8e12 );

		hname = "fpga_td_" + std::to_string(i);
		htitle = "Time difference between FPGA and CAEN pulser events in module ";
		htitle += std::to_string(i) + ";#Delta t [ns]";
		fpga_td[i] = new TH1F( hname.data(), htitle.data(), 1.6e3 , -4e3, 4e3 );

		hname = "fpga_freq_" + std::to_string(i);
		htitle = "Frequency of pulser in ISS DAQ (FPGA) as a function of time in module ";
		htitle += std::to_string(i) + ";time [ns];f [Hz]";
		fpga_freq[i] = new TProfile( hname.data(), htitle.data(), 10.8e4, 0, 10.8e12 );

		hname = "fpga_sync_" + std::to_string(i);
		htitle = "Number of missing/extra pulser events in FPGA as a function of time in module ";
		htitle += std::to_string(i) + ";time [ns];(-ive CAEN missing, +ive ISS missing)";
		fpga_sync[i] = new TProfile( hname.data(), htitle.data(), 10.8e4, 0, 10.8e12 );

		hname = "fpga_pulser_loss_" + std::to_string(i);
		htitle = "Frequency difference of pulser events in ISS/CAEN DAQs from FPGA as a function of time in module ";
		htitle += std::to_string(i) + ";#time [ns];#Delta f [Hz]";
		fpga_pulser_loss[i] = new TProfile( hname.data(), htitle.data(), 10.8e4, 0, 10.8e12 );

		hname = "fpga_freq_diff_" + std::to_string(i);
		htitle = "Time difference between FPGA and CAEN pulser events in module ";
		htitle += std::to_string(i) + ";#Delta t [ns]";
		fpga_freq_diff[i] = new TProfile( hname.data(), htitle.data(), 10.8e4, 0, 10.8e12 );

	}

	ebis_freq = new TProfile( "ebis_freq", "Frequency of EBIS events as a function of time;time [ns];f [Hz]", 10.8e4, 0, 10.8e12 );
	t1_freq = new TProfile( "t1_freq", "Frequency of T1 events (p+ on ISOLDE target) as a function of time;time [ns];f [Hz]", 10.8e4, 0, 10.8e12 );

	
	// Make directories
	maindirname = "array";

	// ---------------- //
	// Array histograms //
	// ---------------- //
	pn_11.resize( set->GetNumberOfArrayModules() );
	pn_12.resize( set->GetNumberOfArrayModules() );
	pn_21.resize( set->GetNumberOfArrayModules() );
	pn_22.resize( set->GetNumberOfArrayModules() );
	pn_ab.resize( set->GetNumberOfArrayModules() );
	pn_nab.resize( set->GetNumberOfArrayModules() );
	pn_pab.resize( set->GetNumberOfArrayModules() );
	pn_max.resize( set->GetNumberOfArrayModules() );
	pn_td.resize( set->GetNumberOfArrayModules() );
	pn_td_Ep.resize( set->GetNumberOfArrayModules() );
	pn_td_En.resize( set->GetNumberOfArrayModules() );
	pp_td.resize( set->GetNumberOfArrayModules() );
	nn_td.resize( set->GetNumberOfArrayModules() );
	pn_mult.resize( set->GetNumberOfArrayModules() );
	
	pn_td_prompt.resize( set->GetNumberOfArrayModules() );
	pp_td_prompt.resize( set->GetNumberOfArrayModules() );
	nn_td_prompt.resize( set->GetNumberOfArrayModules() );

	// Loop over ISS modules
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {
	
		dirname = maindirname + "/module_" + std::to_string(i);

		if( !output_file->GetDirectory( dirname.data() ) )
			output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );

		pn_11[i].resize( set->GetNumberOfArrayRows() );
		pn_12[i].resize( set->GetNumberOfArrayRows() );
		pn_21[i].resize( set->GetNumberOfArrayRows() );
		pn_22[i].resize( set->GetNumberOfArrayRows() );
		pn_ab[i].resize( set->GetNumberOfArrayRows() );
		pn_nab[i].resize( set->GetNumberOfArrayRows() );
		pn_pab[i].resize( set->GetNumberOfArrayRows() );
		pn_max[i].resize( set->GetNumberOfArrayRows() );
		pn_td[i].resize( set->GetNumberOfArrayRows() );
		pn_td_Ep[i].resize( set->GetNumberOfArrayRows() );
		pn_td_En[i].resize( set->GetNumberOfArrayRows() );
		pp_td[i].resize( set->GetNumberOfArrayRows() );
		nn_td[i].resize( set->GetNumberOfArrayRows() );
		pn_mult[i].resize( set->GetNumberOfArrayRows() );
		
		pn_td_prompt[i].resize( set->GetNumberOfArrayRows() );
		pp_td_prompt[i].resize( set->GetNumberOfArrayRows() );
		nn_td_prompt[i].resize( set->GetNumberOfArrayRows() );

		// Loop over rows of the array
		for( unsigned int j = 0; j < set->GetNumberOfArrayRows(); ++j ) {

			hname = "pn_1v1_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side multiplicity = 1 vs. n-side multiplicity = 1 (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");p-side energy [keV];n-side energy [keV]";
			pn_11[i][j] = new TH2F( hname.data(), htitle.data(), 2e3, 0, 2e4, 2e3, 0, 2e4 );
			
			hname = "pn_1v2_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side multiplicity = 1 vs. n-side multiplicity = 2 (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");p-side energy [keV];n-side energy [keV]";
			pn_12[i][j] = new TH2F( hname.data(), htitle.data(), 2e3, 0, 2e4, 2e3, 0, 2e4 );
			
			hname = "pn_2v1_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side multiplicity = 2 vs. n-side multiplicity = 1 (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");p-side energy [keV];n-side energy [keV]";
			pn_21[i][j] = new TH2F( hname.data(), htitle.data(), 2e3, 0, 2e4, 2e3, 0, 2e4 );
			
			hname = "pn_2v2_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side multiplicity = 2 vs. n-side multiplicity = 2 (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");p-side energy [keV];n-side energy [keV]";
			pn_22[i][j] = new TH2F( hname.data(), htitle.data(), 2e3, 0, 2e4, 2e3, 0, 2e4 );
			
			hname = "pn_ab_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side addback energy vs. n-side addback energy (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");p-side energy [keV];n-side energy [keV]";
			pn_ab[i][j] = new TH2F( hname.data(), htitle.data(), 2e3, 0, 2e4, 2e3, 0, 2e4 );
			
			hname = "pn_nab_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side singles energy vs. n-side addback energy (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");p-side energy [keV];n-side energy [keV]";
			pn_nab[i][j] = new TH2F( hname.data(), htitle.data(), 2e3, 0, 2e4, 2e3, 0, 2e4 );
			
			hname = "pn_pab_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side addback energy vs. n-side singles energy (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");p-side energy [keV];n-side energy [keV]";
			pn_pab[i][j] = new TH2F( hname.data(), htitle.data(), 2e3, 0, 2e4, 2e3, 0, 2e4 );
			
			hname = "pn_max_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side max energy vs. n-side max energy (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");p-side energy [keV];n-side energy [keV]";
			pn_max[i][j] = new TH2F( hname.data(), htitle.data(), 2e3, 0, 2e4, 2e3, 0, 2e4 );
			
			hname = "pn_td_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side vs. n-side time difference (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");time difference [ns];counts";
			pn_td[i][j] = new TH1F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20 );
			
			hname = "pn_td_Ep_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side n-side time difference vs p-side energy (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");time difference [ns];p-side energy [keV]";
			pn_td_Ep[i][j] = new TH2F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20, 2e3, 0, 2e4 );
			
			hname = "pn_td_En_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side n-side time difference vs n-side energy (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");time difference [ns];n-side energy [keV]";
			pn_td_En[i][j] = new TH2F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20, 2e3, 0, 2e4  );
						
			hname = "pp_td_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side vs. p-side time difference (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");time difference [ns];counts";
			pp_td[i][j] = new TH1F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20 );
			
			hname = "nn_td_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "n-side vs. n-side time difference (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");time difference [ns];counts";
			nn_td[i][j] = new TH1F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20 );
			
			hname = "pn_mult_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side vs. n-side multiplicity (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");mult p-side;mult n-side";
			pn_mult[i][j] = new TH2F( hname.data(), htitle.data(), 6, -0.5, 5.5, 6, -0.5, 5.5 );
			
			// --------------------------------------------------------------------------------- //
			hname = "pn_td_prompt_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side vs. n-side prompt time difference (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");time difference [ns];counts";
			pn_td_prompt[i][j] = new TH1F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20 );
			
			hname = "pp_td_prompt_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side vs. p-side time difference (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");time difference [ns];counts";
			pp_td_prompt[i][j] = new TH1F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20 );
			
			hname = "nn_td_prompt_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "n-side vs. n-side time difference (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");time difference [ns];counts";
			nn_td_prompt[i][j] = new TH1F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20 );
		
		}
		
	}
	
	// ----------------- //
	// Recoil histograms //
	// ----------------- //
	dirname = "recoils";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	
	// ----------------- //
	// Recoil histograms //
	// ----------------- //
	recoil_EdE.resize( set->GetNumberOfRecoilSectors() );
	recoil_EdE_raw.resize( set->GetNumberOfRecoilSectors() );
	recoil_dEsum.resize( set->GetNumberOfRecoilSectors() );
	recoil_E_singles.resize( set->GetNumberOfRecoilSectors() );
	recoil_dE_singles.resize( set->GetNumberOfRecoilSectors() );
	recoil_E_dE_tdiff.resize( set->GetNumberOfRecoilSectors() );
	
	// Loop over number of recoil sectors
	for( unsigned int i = 0; i < set->GetNumberOfRecoilSectors(); ++i ) {
	
		hname = "recoil_EdE" + std::to_string(i);
		htitle = "Recoil dE vs E for sector " + std::to_string(i);
		htitle += ";Rest energy, E [keV];Energy loss, dE [keV];Counts";
		recoil_EdE[i] = new TH2F( hname.data(), htitle.data(), 2000, 0, 200000, 2000, 0, 200000 );
		
		hname = "recoil_dEsum" + std::to_string(i);
		htitle = "Recoil dE vs Esum for sector " + std::to_string(i);
		htitle += ";Total energy, Esum [keV];Energy loss, dE [keV];Counts";
		recoil_dEsum[i] = new TH2F( hname.data(), htitle.data(), 2000, 0, 200000, 2000, 0, 200000 );
		
		hname = "recoil_EdE_raw" + std::to_string(i);
		htitle = "Recoil dE vs E for sector " + std::to_string(i);
		htitle += ";Rest energy, E [arb.];Energy loss, dE [arb.];Counts";
		recoil_EdE_raw[i] = new TH2F( hname.data(), htitle.data(), 2048, 0, 65536, 2048, 0, 65536 );
	
		hname = "recoil_E_singles" + std::to_string(i);		
		htitle = "Recoil E singles in sector " + std::to_string(i);
		htitle += "; E [keV]; Counts";
		recoil_E_singles[i] = new TH1F( hname.data(), htitle.data(), 2000, 0, 200000 );
		
		hname = "recoil_dE_singles" + std::to_string(i);		
		htitle = "Recoil dE singles in sector " + std::to_string(i);
		htitle += "; dE [keV]; Counts";
		recoil_dE_singles[i] = new TH1F( hname.data(), htitle.data(), 2000, 0, 200000 );
		
		hname = "recoil_E_dE_tdiff" + std::to_string(i);		
		htitle = "Recoil E-dE time difference " + std::to_string(i);
		htitle += "; #Delta t [ns]; Counts";
		recoil_E_dE_tdiff[i] = new TH1F( hname.data(), htitle.data(), 2000, -6e3, 6e3 );
	}
	
	
	// ---------------- //
	// MWPC histograms //
	// ---------------- //
	dirname = "mwpc";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );
	
	mwpc_tac_axis.resize( set->GetNumberOfMWPCAxes() );
	mwpc_hit_axis.resize( set->GetNumberOfMWPCAxes() );

	// Loop over number of recoil sectors
	for( unsigned int i = 0; i < set->GetNumberOfMWPCAxes(); ++i ) {

		hname = "mwpc_hit_axis" + std::to_string(i);
		htitle = "MWPC TAC difference for axis " + std::to_string(i) + ";TAC difference;Counts";
		mwpc_hit_axis[i] = new TH1F( hname.data(), htitle.data(), 8192, -65536, 65536 );

		mwpc_tac_axis[i].resize( 2 );
		for( unsigned int j = 0; j < 2; ++j ) {

			hname = "mwpc_tac" + std::to_string(j) + "_axis" + std::to_string(i);
			htitle = "MWPC TAC" + std::to_string(j) + " time for axis " + std::to_string(i) + ";TAC time;Counts";
			mwpc_tac_axis[i][j] = new TH1F( hname.data(), htitle.data(), 65536, 0, 65536 );
			
		}
		
	}
	
	hname = "mwpc_pos";
	htitle = "MWPC x-y TAC difference;x;y;Counts";
	mwpc_pos = new TH2F( hname.data(), htitle.data(), 8192, -65536, 65536, 8192, -65536, 65536 );

	
	// ---------------- //
	// ELUM histograms //
	// ---------------- //
	output_file->cd();
	hname = "elum";
	htitle = "ELUM energy vs sector;Sector;Energy [keV];Counts";
	elum = new TH2F( hname.data(), htitle.data(),
			set->GetNumberOfELUMSectors(), -0.5, set->GetNumberOfELUMSectors()-0.5, 2000, 0, 20000 );

	
	// --------------------- //
	// ZeroDegree histograms //
	// --------------------- //
	output_file->cd();
	hname = "zd";
	htitle = "ZeroDegree dE vs E;Rest Energy [keV];Energy Loss [keV];Counts";
	zd = new TH2F( hname.data(), htitle.data(), 2000, 0, 20000, 2000, 0, 200000 );

	return;
	
}

////////////////////////////////////////////////////////////////////////////////
/// This function cleans up all of the histograms used in the EventBuilder class, by deleting them and clearing all histogram vectors.
void ISSEventBuilder::CleanHists() {

	// Clean up the histograms to save memory for later
	for( unsigned int i = 0; i < pn_11.size(); i++ ) {
		for( unsigned int j = 0; j < pn_11.at(i).size(); j++ )
			delete (pn_11[i][j]);
		pn_11.clear();
	}

	for( unsigned int i = 0; i < pn_12.size(); i++ ) {
		for( unsigned int j = 0; j < pn_12.at(i).size(); j++ )
			delete (pn_12[i][j]);
		pn_12.clear();
	}

	for( unsigned int i = 0; i < pn_21.size(); i++ ) {
		for( unsigned int j = 0; j < pn_21.at(i).size(); j++ )
			delete (pn_21[i][j]);
		pn_21.clear();
	}

	for( unsigned int i = 0; i < pn_22.size(); i++ ) {
		for( unsigned int j = 0; j < pn_22.at(i).size(); j++ )
			delete (pn_22[i][j]);
		pn_22.clear();
	}

	for( unsigned int i = 0; i < pn_ab.size(); i++ ) {
		for( unsigned int j = 0; j < pn_ab.at(i).size(); j++ )
			delete (pn_ab[i][j]);
		pn_ab.clear();
	}

	for( unsigned int i = 0; i < pn_nab.size(); i++ ) {
		for( unsigned int j = 0; j < pn_nab.at(i).size(); j++ )
			delete (pn_nab[i][j]);
		pn_nab.clear();
	}

	for( unsigned int i = 0; i < pn_pab.size(); i++ ) {
		for( unsigned int j = 0; j < pn_pab.at(i).size(); j++ )
			delete (pn_pab[i][j]);
		pn_pab.clear();
	}

	for( unsigned int i = 0; i < pn_max.size(); i++ ) {
		for( unsigned int j = 0; j < pn_max.at(i).size(); j++ )
			delete (pn_max[i][j]);
		pn_max.clear();
	}

	for( unsigned int i = 0; i < pn_td.size(); i++ ) {
		for( unsigned int j = 0; j < pn_td.at(i).size(); j++ )
			delete (pn_td[i][j]);
		pn_td.clear();
	}
	
	for( unsigned int i = 0; i < pp_td.size(); i++ ) {
		for( unsigned int j = 0; j < pp_td.at(i).size(); j++ )
			delete (pp_td[i][j]);
		pp_td.clear();
	}

	for( unsigned int i = 0; i < nn_td.size(); i++ ) {
		for( unsigned int j = 0; j < nn_td.at(i).size(); j++ )
			delete (nn_td[i][j]);
		nn_td.clear();
	}

	for( unsigned int i = 0; i < pn_td_Ep.size(); i++ ) {
		for( unsigned int j = 0; j < pn_td_Ep.at(i).size(); j++ )
			delete (pn_td_Ep[i][j]);
		pn_td_Ep.clear();
	}
	
	for( unsigned int i = 0; i < pn_td_En.size(); i++ ) {
		for( unsigned int j = 0; j < pn_td_En.at(i).size(); j++ )
			delete (pn_td_En[i][j]);
		pn_td_En.clear();
	}
	
	for( unsigned int i = 0; i < pn_mult.size(); i++ ) {
		for( unsigned int j = 0; j < pn_mult.at(i).size(); j++ )
			delete (pn_mult[i][j]);
		pn_mult.clear();
	}
	
	for( unsigned int i = 0; i < pn_td_prompt.size(); i++ ) {
		for( unsigned int j = 0; j < pn_td_prompt.at(i).size(); j++ )
			delete (pn_td_prompt[i][j]);
		pn_td_prompt.clear();
	}
	
	for( unsigned int i = 0; i < pp_td_prompt.size(); i++ ) {
		for( unsigned int j = 0; j < pp_td_prompt.at(i).size(); j++ )
			delete (pp_td_prompt[i][j]);
		pp_td_prompt.clear();
	}

	for( unsigned int i = 0; i < nn_td_prompt.size(); i++ ) {
		for( unsigned int j = 0; j < nn_td_prompt.at(i).size(); j++ )
			delete (nn_td_prompt[i][j]);
		nn_td_prompt.clear();
	}


	for( unsigned int i = 0; i < recoil_EdE.size(); i++ )
		delete (recoil_EdE[i]);
	recoil_EdE.clear();
	
	for( unsigned int i = 0; i < recoil_dEsum.size(); i++ )
		delete (recoil_dEsum[i]);
	recoil_dEsum.clear();
	
	for( unsigned int i = 0; i < recoil_EdE_raw.size(); i++ )
		delete (recoil_EdE_raw[i]);
	recoil_EdE_raw.clear();
		
	for( unsigned int i = 0; i < recoil_E_singles.size(); i++ )
		delete (recoil_E_singles[i]);
	recoil_E_singles.clear();
		
	for( unsigned int i = 0; i < recoil_dE_singles.size(); i++ )
		delete (recoil_dE_singles[i]);
	recoil_dE_singles.clear();
		
	for( unsigned int i = 0; i < recoil_E_dE_tdiff.size(); i++ )
		delete (recoil_E_dE_tdiff[i]);
	recoil_E_dE_tdiff.clear();

	
	for( unsigned int i = 0; i < mwpc_tac_axis.size(); i++ ) {
		for( unsigned int j = 0; j < mwpc_tac_axis.at(i).size(); j++ )
			delete (mwpc_tac_axis[i][j]);
		mwpc_tac_axis.clear();
	}

	for( unsigned int i = 0; i < mwpc_hit_axis.size(); i++ )
		delete (mwpc_hit_axis[i]);
	mwpc_hit_axis.clear();
	
	delete mwpc_pos;
	delete elum;
	delete zd;

	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); i++ ){
		delete (fpga_td[i]);
		delete (asic_td[i]);
		delete (fpga_pulser_loss[i]);
		delete (fpga_freq_diff[i]);
		delete (fpga_freq[i]);
		delete (fpga_sync[i]);
		delete (asic_pulser_loss[i]);
		delete (asic_freq_diff[i]);
		delete (asic_freq[i]);
		delete (asic_sync[i]);
	}
		
	fpga_td.clear();
	fpga_sync.clear();
	fpga_freq.clear();
	fpga_freq_diff.clear();
	fpga_pulser_loss.clear();
	asic_td.clear();
	asic_sync.clear();
	asic_freq.clear();
	asic_freq_diff.clear();
	asic_pulser_loss.clear();
	
	delete tdiff;
	delete tdiff_clean;
	delete caen_freq;
	delete ebis_freq;
	delete t1_freq;

	return;

}

