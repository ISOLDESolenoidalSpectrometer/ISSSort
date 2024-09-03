#include "EventBuilder.hh"
///////////////////////////////////////////////////////////////////////////////
/// This constructs the event-builder object, setting parameters for this process by grabbing information from the settings file (or using default parameters defined in the constructor)
/// \param[in] myset The ISSSettings object which is constructed by the ISSSettings constructor used in iss_sort.cc
ISSEventBuilder::ISSEventBuilder( std::shared_ptr<ISSSettings> myset ){
	
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
	vme_time_start.resize( set->GetNumberOfVmeCrates() );
	vme_time_stop.resize( set->GetNumberOfVmeCrates() );
	
	for( unsigned int i = 0; i < set->GetNumberOfVmeCrates(); ++i ){
		
		vme_time_start[i].resize( set->GetMaximumNumberOfVmeModules() );
		vme_time_stop[i].resize( set->GetMaximumNumberOfVmeModules() );
		
	}
	
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
			else {
				
				array_nid[i].push_back( -1 );
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
	ebis_prev = 0;
	t1_prev = 0;
	sc_prev = 0;
	laser_prev = 0;

	n_asic_data	= 0;
	n_caen_data	= 0;
	n_mesy_data	= 0;
	n_info_data	= 0;

	n_caen_pulser = 0;

	n_ebis	= 0;
	n_t1	= 0;
	n_sc	= 0;
	n_laser	= 0;

	array_ctr	= 0;
	arrayp_ctr	= 0;
	recoil_ctr	= 0;
	mwpc_ctr	= 0;
	elum_ctr	= 0;
	zd_ctr		= 0;
	gamma_ctr	= 0;
	lume_ctr	= 0;

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
	
	for( unsigned int i = 0; i < set->GetNumberOfVmeCrates(); ++i ) {
		
		for( unsigned int j = 0; j < set->GetMaximumNumberOfVmeModules(); ++j ) {
			
			vme_time_start[i][j] = 0;
			vme_time_stop[i][j] = 0;
			
		}

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
	input_tree->SetBranchAddress( "data", &in_data );

	return;
	
}

////////////////////////////////////////////////////////////////////////////////
/// This reads in the NPTool simulation data and creates
/// \param [in] input_file_name The ROOT file containing the TIssData tree from NPTool
void ISSEventBuilder::SetNPToolFile( std::string input_file_name ) {
	
	// Open Simulation Root input file from NPTool.
	input_file = new TFile( input_file_name.data(), "read" );
	if( input_file->IsZombie() ) {
		
		std::cout << "Cannot open " << input_file_name << std::endl;
		return;
		
	}
	
	flag_input_file = true;
	flag_nptool = true;

	// Set the input tree
	SetNPToolTree( (TTree*)input_file->Get("SimulatedTree") );
	StartFile();

	return;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Sets the nptool_tree to the parameter user_tree, sets the branch address
/// \param [in] user_tree The name of the tree in the ROOT file containing the NPTool simulation events
void ISSEventBuilder::SetNPToolTree( TTree *user_tree ){
	
	// Find the tree and set branch addresses
	nptool_tree = user_tree;
	nptool_tree->SetBranchAddress( "ISS", &sim_data );

	return;
	
}



////////////////////////////////////////////////////////////////////////////////
/// Constructs a number of objects for storing measurements from different detectors, that can then be wrapped up into physics events. Also creates an output file and output tree, and calls the ISSEventBuilder::MakeHists function
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
	gamma_evt	= std::make_shared<ISSGammaRayEvt>();
	lume_evt	= std::make_shared<ISSLumeEvt>();

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
	MakeHists();
	
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
	ptd_list.clear(); //uncorrected time
	ntd_list.clear(); //uncorrected time
    pwalk_list.clear(); //corrected time
    nwalk_list.clear(); //corrected time
	pid_list.clear();
	nid_list.clear();
	pmod_list.clear();
	nmod_list.clear();
	prow_list.clear();
	nrow_list.clear();
	phit_list.clear();
	nhit_list.clear();

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
	
	saen_list.clear();
	satd_list.clear();
	said_list.clear();

	lbe_list.clear();
	lne_list.clear();
	lfe_list.clear();
	lbe_td_list.clear();
	lne_td_list.clear();
	lfe_td_list.clear();
	lbe_id_list.clear();
	lne_id_list.clear();
	lfe_id_list.clear();
	
	// Now swap all these vectors with empty vectors to ensure they are fully cleared
	std::vector<float>().swap(pen_list);
	std::vector<float>().swap(nen_list);
	std::vector<double>().swap(ptd_list);
	std::vector<double>().swap(ntd_list);
    std::vector<double>().swap(pwalk_list);
    std::vector<double>().swap(nwalk_list);
	std::vector<char>().swap(pid_list);
	std::vector<char>().swap(nid_list);
	std::vector<char>().swap(pmod_list);
	std::vector<char>().swap(nmod_list);
	std::vector<char>().swap(prow_list);
	std::vector<char>().swap(nrow_list);
	std::vector<bool>().swap(phit_list);
	std::vector<bool>().swap(nhit_list);

	std::vector<float>().swap(ren_list);
	std::vector<double>().swap(rtd_list);
	std::vector<char>().swap(rid_list);
	std::vector<char>().swap(rsec_list);
	
	std::vector<unsigned short>().swap(mwpctac_list);
	std::vector<double>().swap(mwpctd_list);
	std::vector<char>().swap(mwpcaxis_list);
	std::vector<char>().swap(mwpcid_list);

	std::vector<float>().swap(een_list);
	std::vector<double>().swap(etd_list);
	std::vector<char>().swap(esec_list);
	
	std::vector<float>().swap(zen_list);
	std::vector<double>().swap(ztd_list);
	std::vector<char>().swap(zid_list);

	std::vector<float>().swap(saen_list);
	std::vector<double>().swap(satd_list);
	std::vector<char>().swap(said_list);

	std::vector<float>().swap(lbe_list);
	std::vector<float>().swap(lne_list);
	std::vector<float>().swap(lfe_list);
	std::vector<double>().swap(lbe_td_list);
	std::vector<double>().swap(lne_td_list);
	std::vector<double>().swap(lfe_td_list);
	std::vector<char>().swap(lbe_id_list);
	std::vector<char>().swap(lne_id_list);
	std::vector<char>().swap(lfe_id_list);

	write_evts->ClearEvt();
	
	return;
	
}

////////////////////////////////////////////////////////////////////////////////
/// This loops over all events found in the input file and wraps them up and stores them in the output file
/// \return The number of entries in the tree that have been sorted (=0 if there is an error)
unsigned long ISSEventBuilder::BuildEvents() {
	
	/// Function to loop over the sort tree and build array and recoil events

	// Load the full tree if possible
	output_tree->SetMaxVirtualSize(5e8); // 500 MB
	input_tree->SetMaxVirtualSize(5e8); // 500 MB
	input_tree->LoadBaskets(5e8); // Load 500 MB of data to memory

	if( input_tree->LoadTree(0) < 0 ){
		
		std::cout << " Event Building: nothing to do" << std::endl;
		return 0;
		
	}
	
	// Get ready and go
	Initialise();
	n_entries = input_tree->GetEntries();

	std::cout << " Event Building: number of entries in input tree = ";
	std::cout << n_entries << std::endl;
	
	// Apply time-walk correction, i.e. get new time ordering
	//std::cout << " Event Building: applying time walk-correction to event ordering" << std::endl;
	//input_tree->BuildIndex( "GetTimeWithWalk()" );
	input_tree->BuildIndex( "GetTimeStamp()" );
	TTreeIndex *att_index = (TTreeIndex*)input_tree->GetTreeIndex();

	(void) att_index; // Avoid unused variable warning.

	// ------------------------------------------------------------------------ //
	// Main loop over TTree to find events
	// ------------------------------------------------------------------------ //
	for( unsigned long i = 0; i < n_entries; ++i ) {
		
		// Get time-ordered event index (with or without walk correction)
		unsigned long long idx = i; // no correction
		//unsigned long long idx = att_index->GetIndex()[i]; // with correction

		// Current event data
		if( input_tree->MemoryFull(30e6) )
			input_tree->DropBaskets();
		if( i == 0 ) input_tree->GetEntry(idx);
		
		// Get the time of the event (with or without walk correction)
		mytime = in_data->GetTime(); // no correction
		//mytime = in_data->GetTimeWithWalk(); // with correction

		//std::cout << std::setprecision(15) << i << "\t";
		//std::cout << in_data->GetTimeStamp() << "\t" << mytime << std::endl;
				
		// check time stamp monotonically increases!
		// but allow for the fine time of the CAEN system
		if( (unsigned long long)time_prev > in_data->GetTimeStamp() + 5.0 ) {
			
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
			myhitbit = asic_data->GetHitBit();
			if( overwrite_cal ) {
			
				myenergy = cal->AsicEnergy( mymod, myasic,
									 mych, asic_data->GetAdcValue() );
				mywalk = cal->AsicWalk( mymod, myasic, myenergy, myhitbit );
			
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

			// p-side event
			if( myside == 0 && mythres ) {
			
			// test here about hit bit value
			//if( myside == 0 && mythres && !asic_data->GetHitBit() ) {

				mystrip = array_pid.at( myasic ).at( mych );
				
				// Only use if it is an event from a detector
				if( mystrip >= 0 ) {
							
					pen_list.push_back( myenergy );
					ptd_list.push_back( mytime );
					pwalk_list.push_back( mytime + mywalk );
					pmod_list.push_back( mymod );
					pid_list.push_back( mystrip );
					prow_list.push_back( myrow );
					phit_list.push_back( myhitbit );

					event_open = true; // real data open events (above threshold and from a strip)
					hit_ctr++; // increase counter for bits of data included in this event

				}
				
			}

			// n-side event
			else if( myside == 1 && mythres ) {

			// test here about hit bit value
			//else if( myside == 1 && mythres && asic_data->GetHitBit() ) {

				mystrip = array_nid.at( asic_data->GetAsic() ).at( asic_data->GetChannel() );

				// Only use if it is an event from a detector
				if( mystrip >= 0 ) {
							
					nen_list.push_back( myenergy );
                    ntd_list.push_back( mytime );
                    nwalk_list.push_back( mytime + mywalk );
					nmod_list.push_back( mymod );
					nid_list.push_back( mystrip );
					nrow_list.push_back( myrow );
					nhit_list.push_back( myhitbit );

					event_open = true; // // real data open events (above threshold and from a strip)
					hit_ctr++; // increase counter for bits of data included in this event

				}
				
			}

			// Is it the start event?
			if( asic_time_start.at( mymod ) == 0 )
				asic_time_start.at( mymod ) = mytime;
			
			// or is it the end event (we don't know so keep updating)
			asic_time_stop.at( mymod ) = mytime;
			
		}

		// -------------------------------------------- //
		// Find recoils and other things in VME systems //
		// -------------------------------------------- //
		else if( in_data->IsVme() ) {
			
			// Check if it is Meystec or CAEN
			if( in_data->IsCaen() ) {
				
				// Increment event counter and set data type
				n_caen_data++;
				vme_data = in_data->GetCaenData();
				
			}
			else if( in_data->IsMesy() ){
				
				// Increment event counter and set data type
				n_mesy_data++;
				vme_data = in_data->GetMesyData();

			}

			// Get channel ID
			myvme = vme_data->GetCrate();
			mymod = vme_data->GetModule();
			mych = vme_data->GetChannel();
			
			// New calibration supplied
			if( overwrite_cal ) {
				
				std::string entype = cal->VmeType( myvme, mymod, mych );
				unsigned short adc_value = 0;
				if( entype == "Qlong" ) adc_value = vme_data->GetQlong();
				else if( entype == "Qshort" ) adc_value = vme_data->GetQshort();
				else if( entype == "Qdiff" ) adc_value = vme_data->GetQdiff();
				myenergy = cal->VmeEnergy( myvme, mymod, mych, adc_value );
				
				if( adc_value < cal->VmeThreshold( myvme, mymod, mych ) )
					mythres = false;

			}
			
			else {
				
				myenergy = vme_data->GetEnergy();
				mythres = vme_data->IsOverThreshold();

			}
			
			// If it's below threshold do not use as window opener
			if( mythres ) event_open = true;

			// DETERMINE WHICH TYPE OF CAEN EVENT THIS IS
			// Is it a recoil?
			if( set->IsRecoil( myvme, mymod, mych ) && mythres ) {
				
				mysector = set->GetRecoilSector( myvme, mymod, mych );
				mylayer = set->GetRecoilLayer( myvme, mymod, mych );
				
				ren_list.push_back( myenergy );
				rtd_list.push_back( mytime );
				rid_list.push_back( mylayer );
				rsec_list.push_back( mysector );

				hit_ctr++; // increase counter for bits of data included in this event

			}
			
			// Is it an MWPC?
			else if( set->IsMWPC( myvme, mymod, mych ) && mythres ) {
				
				mwpctac_list.push_back( myenergy );
				mwpctd_list.push_back( mytime );
				mwpcaxis_list.push_back( set->GetMWPCAxis( myvme, mymod, mych ) );
				mwpcid_list.push_back( set->GetMWPCID( myvme, mymod, mych ) );

				hit_ctr++; // increase counter for bits of data included in this event

			}
			
			// Is it an ELUM?
			else if( set->IsELUM( myvme, mymod, mych ) && mythres ) {
				
				mysector = set->GetELUMSector( myvme, mymod, mych );
				
				een_list.push_back( myenergy );
				etd_list.push_back( mytime );
				esec_list.push_back( mysector );
				
				hit_ctr++; // increase counter for bits of data included in this event

			}

			// Is it a ZeroDegree?
			else if( set->IsZD( myvme, mymod, mych ) && mythres ) {
				
				mylayer = set->GetZDLayer( myvme, mymod, mych );
				
				zen_list.push_back( myenergy );
				ztd_list.push_back( mytime );
				zid_list.push_back( mylayer );
				
				hit_ctr++; // increase counter for bits of data included in this event

			}
			
			// Is it a ScintArray?
			else if( set->IsScintArray( myvme, mymod, mych ) && mythres ) {
			
				myid = set->GetScintArrayDetector( myvme, mymod, mych );
				
				saen_list.push_back( myenergy );
				satd_list.push_back( mytime );
				said_list.push_back( myid );
				
				hit_ctr++; // increase counter for bits of data included in this event

			}
			// Is it a LUME?
			else if( set->IsLUME( myvme, mymod, mych ) && mythres ) {

				// Get LUME signal type (0 = total energy, 1 = near side, 2 = far side)
				mytype = set->GetLUMEType( myvme, mymod, mych );
				myid = set->GetLUMEDetector( myvme, mymod, mych );

				switch (mytype) {
				case 0:
					lbe_list.push_back( myenergy );
					lbe_td_list.push_back( mytime );
					lbe_id_list.push_back( myid );
					break;
				case 1:
					lne_list.push_back( myenergy );
					lne_td_list.push_back( mytime );
					lne_id_list.push_back( myid );
					break;
				case 2:
					lfe_list.push_back( myenergy );
					lfe_td_list.push_back( mytime );
					lfe_id_list.push_back( myid );
					break;
				default:
					break;
				}

				hit_ctr++; // increase counter for bits of data included in this event

			}

			// Is it the start event?
			if( vme_time_start.at( myvme ).at( mymod ) == 0 )
				vme_time_start.at( myvme ).at( mymod ) = mytime;
			
			// or is it the end event (we don't know so keep updating)
			vme_time_stop.at( myvme ).at( mymod ) = mytime;

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
			double info_tdiff;
			if( info_data->GetCode() == set->GetEBISCode() ){
			
				// Each ASIC module sends ebis_time signal, so make sure difference between last ebis pulse and now is longer than the time it takes for them all to enter the DAQ
				info_tdiff = info_data->GetTime() - ebis_prev;
				if( TMath::Abs( info_tdiff ) > 1e3 ){
					
					ebis_prev = info_data->GetTime();
					if( ebis_prev != 0 ) ebis_period->Fill( info_tdiff );
					n_ebis++;
					
				}
				
			}
		
			// Update T1 time
			else if( info_data->GetCode() == set->GetT1Code() ){
				
				info_tdiff = info_data->GetTime() - t1_prev;
				if( TMath::Abs( info_tdiff ) > 1e3 ){
				
					t1_prev = info_data->GetTime();
					if( t1_prev != 0 ){
						t1_period->Fill( info_tdiff );
						supercycle->Fill( t1_prev - sc_prev );
					}
					n_t1++;

				}

			}
			
			// Update SuperCycle time
			else if( info_data->GetCode() == set->GetSCCode() ){
				
				info_tdiff = info_data->GetTime() - sc_prev;
				if( TMath::Abs( info_tdiff ) > 1e3 ){
				
					sc_prev = info_data->GetTime();
					if( sc_prev != 0 ) sc_period->Fill( info_tdiff );
					n_sc++;

				}

			}
			
			// Update Laser status time
			else if( info_data->GetCode() == set->GetLaserCode() ){
				
				info_tdiff = info_data->GetTime() - laser_prev;
				if( TMath::Abs( info_tdiff ) > 1e3 ){
				
					laser_prev = info_data->GetTime();
					if( laser_prev != 0 ) laser_period->Fill( info_tdiff );
					n_laser++;

				}

			}
			
			// Update CAEN pulser time
			else if( info_data->GetCode() == set->GetCAENPulserCode() ) {
				
				caen_time = info_data->GetTime();
				if( caen_prev != 0 ) caen_period->Fill( caen_time - caen_prev );
				flag_caen_pulser = true;
				n_caen_pulser++;
				

			}

			// Update ISS pulser time in FPGA
			else if( info_data->GetCode() == set->GetExternalTriggerCode() ) {
			   
				fpga_time[info_data->GetModule()] = info_data->GetTime();
				info_tdiff = fpga_time[info_data->GetModule()] - fpga_prev[info_data->GetModule()];

				if( fpga_prev[info_data->GetModule()] != 0 )
					fpga_period[info_data->GetModule()]->Fill( info_tdiff );

				n_fpga_pulser[info_data->GetModule()]++;

			}
			
			// Update ISS pulser time in ASICs
			else if( info_data->GetCode() == set->GetArrayPulserCode0() ) {
			   
				asic_time[info_data->GetModule()] = info_data->GetTime();
				info_tdiff = asic_time[info_data->GetModule()] - asic_prev[info_data->GetModule()];

				if( asic_prev[info_data->GetModule()] != 0 )
					asic_period[info_data->GetModule()]->Fill( info_tdiff );

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
						std::cout << resume_time[info_data->GetModule()]/1e9;
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
				
					double fpga_tdiff = caen_time - fpga_time[j];
					double asic_tdiff = caen_time - asic_time[j];

					// If diff is greater than 5 ms, we have the wrong pair
					if( fpga_tdiff > 5e6 ) fpga_tdiff = caen_prev - fpga_time[j];
					else if( fpga_tdiff < -5e6 ) fpga_tdiff = caen_time - fpga_prev[j];
					if( asic_tdiff > 5e6 ) asic_tdiff = caen_prev - asic_time[j];
					else if( asic_tdiff < -5e6 ) asic_tdiff = caen_time - asic_prev[j];
					
					// ??? Could be the case that |fpga_tdiff| > 5e6 after these conditional statements...change to while loop? Or have an extra condition?

					fpga_td[j]->Fill( fpga_tdiff );
					fpga_sync[j]->Fill( fpga_time[j], fpga_tdiff );
					fpga_pulser_loss[j]->Fill( fpga_time[j], (int)n_fpga_pulser[j] - (int)n_caen_pulser );
					
					asic_td[j]->Fill( asic_tdiff );
					asic_sync[j]->Fill( asic_time[j], asic_tdiff );
					asic_pulser_loss[j]->Fill( asic_time[j], (int)n_asic_pulser[j] - (int)n_caen_pulser );

				}

				flag_caen_pulser = false;

			}
			
			// Now reset previous timestamps
			if( info_data->GetCode() == set->GetCAENPulserCode() )
				caen_prev = caen_time;
			if( info_data->GetCode() == set->GetExternalTriggerCode() )
				fpga_prev[info_data->GetModule()] = fpga_time[info_data->GetModule()];
			if( info_data->GetCode() == set->GetArrayPulserCode0() )
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
		unsigned long long idx_next = i+1; // no correction
		
		// Comment out the two lines below to ignore time-walk correction
		//if( i+1 == n_entries ) idx_next = n_entries; // with correction
		//else idx_next = att_index->GetIndex()[i+1]; // with correction

		if( input_tree->GetEntry(idx_next) ) {
					
			// Time difference to next event (with or without time walk correction)
			time_diff = in_data->GetTime() - time_first; // no correction
			//time_diff = in_data->GetTimeWithWalk() - time_first; // with correction

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
				GammaRayFinder();	// add a GammaRay event for ScintArray/HPGe events
				LumeFinder();           // add a LumeEvt for each LUME

				// ------------------------------------
				// Add timing and fill the ISSEvts tree
				// ------------------------------------
				write_evts->SetEBIS( ebis_prev );
				write_evts->SetT1( t1_prev );
				write_evts->SetSC( sc_prev );
				if( TMath::Abs( (double)ebis_prev - (double)laser_prev ) < 1e3
					&& laser_prev > 0 ) write_evts->SetLaserStatus( true );
				else
					write_evts->SetLaserStatus( false );
				
				// Fill only if we have some physics events
				if( write_evts->GetArrayMultiplicity() ||
					write_evts->GetArrayPMultiplicity() ||
					write_evts->GetRecoilMultiplicity() ||
					write_evts->GetMwpcMultiplicity() ||
					write_evts->GetElumMultiplicity() ||
					write_evts->GetZeroDegreeMultiplicity() ||
					write_evts->GetGammaRayMultiplicity()  ||
					write_evts->GetLumeMultiplicity() )
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
		ss_log << "        dead time = " << asic_dead_time[i]/1e9 << " s" << std::endl;
		ss_log << "        live time = " << (asic_time_stop[i]-asic_time_start[i])/1e9 << " s" << std::endl;
	}
	for( unsigned int i = 0; i < set->GetNumberOfVmeCrates(); ++i ) {
		unsigned int nmods = set->GetNumberOfCAENModules();
		if( i == 0 ) ss_log << "  CAEN data packets = " << n_caen_data << std::endl;
		else {
			ss_log << "  Mesytec data packets = " << n_mesy_data << std::endl;
			nmods = set->GetNumberOfMesytecModules();
		}
		for( unsigned int j = 0; j < nmods; ++j ) {
			ss_log << "   Module " << j << " live time = ";
			ss_log << (vme_time_stop[i][j]-vme_time_start[i][j])/1e9;
			ss_log << " s" << std::endl;
		}
	}
	ss_log << "  Info data packets = " << n_info_data << std::endl;
	ss_log << "   Array p/n-side correlated events = " << array_ctr << std::endl;
	ss_log << "   Array p-side only events = " << arrayp_ctr << std::endl;
	ss_log << "   Recoil events = " << recoil_ctr << std::endl;
	ss_log << "   MWPC events = " << mwpc_ctr << std::endl;
	ss_log << "   ELUM events = " << elum_ctr << std::endl;
	ss_log << "   ZeroDegree events = " << zd_ctr << std::endl;
	ss_log << "   Gamma-ray events = " << gamma_ctr << std::endl;
	ss_log << "   LUME events = " << lume_ctr << std::endl;
	ss_log << "   CAEN pulser = " << n_caen_pulser << std::endl;
	ss_log << "   FPGA pulser" << std::endl;
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i )
		ss_log << "    Module " << i << " = " << n_fpga_pulser[i] << std::endl;
	ss_log << "   ASIC pulser" << std::endl;
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i )
		ss_log << "    Module " << i << " = " << n_asic_pulser[i] << std::endl;
	ss_log << "   EBIS events = " << n_ebis << std::endl;
	ss_log << "   T1 events = " << n_t1 << std::endl;
	ss_log << "   SC events = " << n_sc << std::endl;
	ss_log << "   Laser events = " << n_laser << std::endl;
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
/// This loops over all events found in the input NPTool simualtion file and wraps them up and stores them in the output file
/// \return The number of entries in the tree that have been sorted (=0 if there is an error)
unsigned long ISSEventBuilder::BuildSimulatedEvents() {
	
	/// Function to loop over the sort tree and build array and recoil events

	if( nptool_tree->LoadTree(0) < 0 ){
		
		std::cout << " Event Building with NPTool data: nothing to do" << std::endl;
		return 0;
		
	}
	
	// Get ready and go
	Initialise();
	n_entries = nptool_tree->GetEntries();

	std::cout << " Event Building with NPTool data: number of entries in input tree = ";
	std::cout << n_entries << std::endl;
	
	// ------------------------------------------------------------------------ //
	// Main loop over TTree to find events
	// ------------------------------------------------------------------------ //
	for( unsigned long i = 0; i < n_entries; ++i ) {

		// Current event data
		nptool_tree->GetEntry(i);

		// ------------------------------------------- //
		// Find particles on the front strips of array //
		// ------------------------------------------- //
		for( unsigned int j = 0; j < sim_data->GetMultiplicityFront(); j++ ){
			
			mymod = array_evt->FindModule( sim_data->GetFront_DetectorNbr(j) );
			myrow = array_evt->FindRow( sim_data->GetFront_DetectorNbr(j) );
			myasic = array_evt->FindAsicP( sim_data->GetFront_DetectorNbr(j) );
			mystrip = 128 - sim_data->GetFront_StripNbr(j);
			myenergy = sim_data->GetFront_Energy(j);
			mytime = sim_data->GetFront_Time(j);
			myCFDtime = sim_data->GetFront_TimeCFD(j);
			myhitbit = 1;
			
			// If it's below zero in energy, consider it below threshold
			if( myenergy < 0 ) continue;

			// Only use if it is an event from a detector
			if( mystrip < 0 ) continue;
			
			// Only use if it is a single orbit event
			if( myCFDtime > 80 || myCFDtime < 59 ) continue;
			
			pen_list.push_back( myenergy );
			ptd_list.push_back( mytime );
			pwalk_list.push_back( mytime );
			pmod_list.push_back( mymod );
			pid_list.push_back( mystrip );
			prow_list.push_back( myrow );
			phit_list.push_back( myhitbit );
			
			event_open = true; // real data open events (above threshold and from a strip)
			hit_ctr++; // increase counter for bits of data included in this event
			
		} // j
		
		// ------------------------------------------ //
		// Find particles on the back strips of array //
		// ------------------------------------------ //
		for( unsigned int j = 0; j < sim_data->GetMultiplicityBack(); j++ ){
			
			mymod = array_evt->FindModule( sim_data->GetBack_DetectorNbr(j) );
			myrow = array_evt->FindRow( sim_data->GetBack_DetectorNbr(j) );
			myasic = array_evt->FindAsicN( sim_data->GetBack_DetectorNbr(j) );
			mystrip = 11 - sim_data->GetBack_StripNbr(j);
			myenergy = sim_data->GetBack_Energy(j);
			mytime = sim_data->GetBack_Time(j);
			myCFDtime = sim_data->GetBack_TimeCFD(j);
			myhitbit = 1;
			
			// If it's below zero in energy, consider it below threshold
			if( myenergy < 0 ) continue;
			
			// Only use if it is an event from a detector
			if( mystrip < 0 ) continue;
			
			// Only use if it is a single orbit event
			if( myCFDtime > 80 || myCFDtime < 59 ) continue;

			nen_list.push_back( myenergy );
			ntd_list.push_back( mytime );
			nwalk_list.push_back( mytime );
			nmod_list.push_back( mymod );
			nid_list.push_back( mystrip );
			nrow_list.push_back( myrow );
			nhit_list.push_back( myhitbit );
			
			event_open = true; // real data open events (above threshold and from a strip)
			hit_ctr++; // increase counter for bits of data included in this event
			
		} // j
		
		// If we opened the event, then sort it out
		if( event_open ) {

			//----------------------------------
			// Build array events, recoils, etc
			//----------------------------------
			ArrayFinder();		// add an ArrayEvt for each n/p pair
			
			// ------------------------------------
			// Add timing and fill the ISSEvts tree
			// ------------------------------------
			
			// Fill only if we have some physics events
			if( write_evts->GetArrayMultiplicity() ||
			   write_evts->GetArrayPMultiplicity() )
				output_tree->Fill();
			
		}
		
		//--------------------------------------------------
		// clear values of arrays to store intermediate info
		//--------------------------------------------------
		Initialise();
		
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
	
	//--------------------------
	// Clean up
	//--------------------------
	std::stringstream ss_log;
	ss_log << "\n ISSEventBuilder for NPTool data finished..." << std::endl;
	ss_log << "   Array p/n-side correlated events = " << array_ctr << std::endl;
	ss_log << "   Array p-side only events = " << arrayp_ctr << std::endl;
	ss_log << "  Tree entries = " << output_tree->GetEntries() << std::endl;

	std::cout << ss_log.str();
	if( log_file.is_open() && flag_input_file ) log_file << ss_log.str();
	
	std::cout << " Writing output file...\r";
	std::cout.flush();
	
	// Force the rest of the events in the buffer to disk
	output_tree->FlushBaskets();
	output_file->Write( 0, TObject::kWriteDelete );
	
	// Dump the input buffers
	nptool_tree->DropBaskets();

	std::cout << " Writing output file... Done!" << std::endl << std::endl;

	return n_entries;
	
}



////////////////////////////////////////////////////////////////////////////////
/// This function processes a series of vectors that are populated in a given build window, and deals with the signals accordingly. This is currently done on a case-by-case basis i.e. each different number of p-side and n-side hits is dealt with in it's own section. Charge addback is implemented for neighbouring strips that fall within a prompt coincidence window defined by the user in the ISSSettings file.
void ISSEventBuilder::ArrayFinder() {
	
	//std::cout << __PRETTY_FUNCTION__ << std::endl;
	
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
			
			// p-p time
			for( unsigned int k = 0; k < pindex.size(); ++k )
				for( unsigned int l = k+1; l < pindex.size(); ++l )
					if(TMath::Abs( pid_list.at( pindex.at(k) ) - pid_list.at( pindex.at(l) ))==1 )
						pp_td[i][j]->Fill( pwalk_list.at( pindex.at(k) ) - pwalk_list.at( pindex.at(l) ) );
			
			// n-n time
			for( unsigned int k = 0; k < nindex.size(); ++k )
				for( unsigned int l = k+1; l < nindex.size(); ++l )
					if(TMath::Abs( nid_list.at( nindex.at(k) ) - nid_list.at( nindex.at(l) ))==1 )
						nn_td[i][j]->Fill( nwalk_list.at( nindex.at(k) ) - nwalk_list.at( nindex.at(l) ) );
			
			// p-n time
			// Time walk hists (not prompt, not corrected) - used to calculate the timewalk parameters
			for( unsigned int k = 0; k < pindex.size(); ++k ) {
				for( unsigned int l = 0; l < nindex.size(); ++l ) {
					
					pn_td[i][j]->Fill( pwalk_list.at( pindex.at(k) ) - nwalk_list.at( nindex.at(l) ) );
					pn_td_Ep[i][j]->Fill( pwalk_list.at( pindex.at(k) ) - nwalk_list.at( nindex.at(l) ), pen_list.at( pindex.at(k) ) );
					pn_td_En[i][j]->Fill( pwalk_list.at( pindex.at(k) ) - nwalk_list.at( nindex.at(l) ), nen_list.at( nindex.at(l) ) );
					pn_td_uncorrected[i][j]->Fill( ptd_list.at( pindex.at(k) ) - ntd_list.at( nindex.at(l) ) );
					pn_td_Ep_uncorrected[i][j]->Fill( ptd_list.at( pindex.at(k) ) - ntd_list.at( nindex.at(l) ), pen_list.at( pindex.at(k) ) );
					pn_td_En_uncorrected[i][j]->Fill( ptd_list.at( pindex.at(k) ) - ntd_list.at( nindex.at(l) ), nen_list.at( nindex.at(l) ) );
		
				}
			}
			
			// Easy case, p == 1 vs n == 1
			if( pindex.size() == 1 && nindex.size() == 1 ) {
			
				// Fill 1p1n histogram
				pn_11[i][j]->Fill( pen_list.at( pindex.at(0) ), nen_list.at( nindex.at(0) ) );

				// Prompt coincidence
				if( TMath::Abs( pwalk_list.at( pindex.at(0) ) - nwalk_list.at( nindex.at(0) ) ) < set->GetArrayPNHitWindow() ){
				
					// Fill single event as a nice p/n correlation
					array_evt->SetEvent( pen_list.at( pindex.at(0) ),
										 nen_list.at( nindex.at(0) ),
										 pid_list.at( pindex.at(0) ),
										 nid_list.at( nindex.at(0) ),
										 pwalk_list.at( pindex.at(0) ),
										 nwalk_list.at( nindex.at(0) ),
										 phit_list.at( pindex.at(0) ),
										 nhit_list.at( nindex.at(0) ),
										 i, j );
					
					write_evts->AddEvt( array_evt );
					array_ctr++;

					// High n-side threshold situation, fill p-only event
					arrayp_evt->CopyEvent( array_evt );
					write_evts->AddEvt( arrayp_evt );
					arrayp_ctr++;
				
				}
				
				else {
					
					// Not prompt coincidence, assume pure p-side
					arrayp_evt->SetEvent( pen_list.at( pindex.at(0) ),
									  0,
									  pid_list.at( pindex.at(0) ),
									  5,
									  pwalk_list.at( pindex.at(0) ),
									  0,
									  phit_list.at( pindex.at(0) ),
									  false,
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
									  pwalk_list.at( pindex.at(0) ),
									  0,
									  phit_list.at( pindex.at(0) ),
									  false,
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
				    ( TMath::Abs( pwalk_list.at( pindex.at(0) ) - pwalk_list.at( pindex.at(1) ) ) < set->GetArrayPPHitWindow()
					 || TMath::Abs( ptd_list.at( pindex.at(0) ) - ptd_list.at( pindex.at(1) ) ) == 0 ) ) {

					// Simple sum of both energies, cross-talk not included yet
					psum_en  = pen_list.at( pindex.at(0) );
					psum_en += pen_list.at( pindex.at(1) );

					// Check that p's and n are coincident
					if ( TMath::Abs( pwalk_list.at( pindex.at(0) ) - nwalk_list.at( nindex.at(0) ) ) < set->GetArrayPNHitWindow() ||
					     TMath::Abs( pwalk_list.at( pindex.at(1) ) - nwalk_list.at( nindex.at(0) ) ) < set->GetArrayPNHitWindow() ){
					
						// Fill addback histogram
						pn_pab[i][j]->Fill( psum_en, nen_list.at( nindex.at(0) ) );
						
						// Fill the addback event
						array_evt->SetEvent( psum_en,
											 nen_list.at( nindex.at(0) ),
											 pid_list.at( pmax_idx ),
											 nid_list.at( nindex.at(0) ),
											 pwalk_list.at( pmax_idx ),
											 nwalk_list.at( nindex.at(0) ),
											 phit_list.at( pindex.at(0) ),
											 nhit_list.at( nindex.at(0) ),
											 i, j );

						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
					
					}
					
					else {
						
						// p's coincident, no n coincidence -> do a pp event
						arrayp_evt->SetEvent( psum_en,
									  0,
									  pid_list.at( pmax_idx ),
									  5,
									  pwalk_list.at( pmax_idx ),
									  0,
									  phit_list.at( pmax_idx ),
									  false,
									  i, j );

						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
					
					}

				}
				
				// Non-neighbour strips or not coincident -> don't addback!
				else {
					
					// p1 and n coincident
					if( TMath::Abs( pwalk_list.at( pindex.at(0) ) - nwalk_list.at( nindex.at(0) ) ) < set->GetArrayPNHitWindow() ){
						
						// Fill single event as a nice p/n correlation
						array_evt->SetEvent( pen_list.at( pindex.at(0) ),
											 nen_list.at( nindex.at(0) ),
											 pid_list.at( pindex.at(0) ),
											 nid_list.at( nindex.at(0) ),
											 pwalk_list.at( pindex.at(0) ),
											 nwalk_list.at( nindex.at(0) ),
											 phit_list.at( pindex.at(0) ),
											 nhit_list.at( nindex.at(0) ),
											 i, j );
						
						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
						
					}
					
					// p2 and n coincident
					else if( TMath::Abs( pwalk_list.at( pindex.at(1) ) - nwalk_list.at( nindex.at(0) ) ) < set->GetArrayPNHitWindow() ){
					
						// Fill single event as a nice p/n correlation
						array_evt->SetEvent( pen_list.at( pindex.at(1) ),
											 nen_list.at( nindex.at(0) ),
											 pid_list.at( pindex.at(1) ),
											 nid_list.at( nindex.at(0) ),
											 pwalk_list.at( pindex.at(1) ),
											 nwalk_list.at( nindex.at(0) ),
											 phit_list.at( pindex.at(1) ),
											 nhit_list.at( nindex.at(0) ),
											 i, j );
						
						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
					
					}
					
					// Nothing in coincidence - take max energy p
					else {
						
						// Do a p-only event
						arrayp_evt->SetEvent( pen_list.at( pmax_idx ),
									  0,
									  pid_list.at( pmax_idx ),
									  5,
									  pwalk_list.at( pmax_idx ),
									  0,
									  phit_list.at( pmax_idx ),
									  false,
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
				   ( TMath::Abs( nwalk_list.at( nindex.at(0) ) - nwalk_list.at( nindex.at(1) ) ) < set->GetArrayNNHitWindow()
					|| TMath::Abs( ntd_list.at( nindex.at(0) ) - ntd_list.at( nindex.at(1) ) ) == 0 ) ) {

					// Simple sum of both energies, cross-talk not included yet
					nsum_en  = nen_list.at( nindex.at(0) );
					nsum_en += nen_list.at( nindex.at(1) );

					// Check that p and n are coincident
					if( TMath::Abs( pwalk_list.at( pindex.at(0) ) - nwalk_list.at( nindex.at(0) ) ) < set->GetArrayPNHitWindow() ||
					   TMath::Abs( pwalk_list.at( pindex.at(0) ) - nwalk_list.at( nindex.at(1) ) ) < set->GetArrayPNHitWindow() ){
					
						// Fill addback histogram
						pn_nab[i][j]->Fill( pen_list.at( pindex.at(0) ), nsum_en );

						// Fill the addback event
						array_evt->SetEvent( pen_list.at( pindex.at(0) ),
											 nsum_en,
											 pid_list.at( pindex.at(0) ),
											 nid_list.at( nmax_idx ),
											 pwalk_list.at( pindex.at(0) ),
											 nwalk_list.at( nmax_idx ),
											 phit_list.at( pindex.at(0) ),
											 nhit_list.at( nmax_idx ),
											 i, j );

						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
						
					}
					
					else {
						
						// Have a pure p event and throw out both n's
						arrayp_evt->SetEvent( pen_list.at( pindex.at(0) ),
									  0,
									  pid_list.at( pindex.at(0) ),
									  5,
									  pwalk_list.at( pindex.at(0) ),
									  0,
									  phit_list.at( pindex.at(0) ),
									  false,
									  i, j );

						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
						
					}

				}
				
				// Non-neighbour strips
				else {
				
					// n1 and p coincident
					if( TMath::Abs( pwalk_list.at( pindex.at(0) ) - nwalk_list.at( nindex.at(0) ) ) < set->GetArrayPNHitWindow() ){
						
						// Fill single event as a nice p/n correlation
						array_evt->SetEvent( pen_list.at( pindex.at(0) ),
											 nen_list.at( nindex.at(0) ),
											 pid_list.at( pindex.at(0) ),
											 nid_list.at( nindex.at(0) ),
											 pwalk_list.at( pindex.at(0) ),
											 nwalk_list.at( nindex.at(0) ),
											 phit_list.at( pindex.at(0) ),
											 nhit_list.at( nindex.at(0) ),
											 i, j );
						
						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
						
					}
					
					// n2 and p coincident
					else if( TMath::Abs( pwalk_list.at( pindex.at(0) ) - nwalk_list.at( nindex.at(1) ) ) < set->GetArrayPNHitWindow() ){
					
						// Fill single event as a nice p/n correlation
						array_evt->SetEvent( pen_list.at( pindex.at(0) ),
											 nen_list.at( nindex.at(1) ),
											 pid_list.at( pindex.at(0) ),
											 nid_list.at( nindex.at(1) ),
											 pwalk_list.at( pindex.at(0) ),
											 nwalk_list.at( nindex.at(1) ),
											 phit_list.at( pindex.at(0) ),
											 nhit_list.at( nindex.at(1) ),
											 i, j );
						
						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
						
					}
					
					// No coincidence - keep p
					else {
						
						arrayp_evt->SetEvent( pen_list.at( pindex.at(0) ),
									  0,
									  pid_list.at( pindex.at(0) ),
									  5,
									  pwalk_list.at( pindex.at(0) ),
									  0,
									  phit_list.at( pindex.at(0) ),
									  false,
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
				    ( TMath::Abs( pwalk_list.at( pindex.at(0) ) - pwalk_list.at( pindex.at(1) ) ) < set->GetArrayPPHitWindow()
					 || TMath::Abs( ptd_list.at( pindex.at(0) ) - ptd_list.at( pindex.at(1) ) ) == 0 ) ) {

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
										  pwalk_list.at( pmax_idx ),
										  0,
										  phit_list.at( pmax_idx ),
										  false,
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
										  pwalk_list.at( pmax_idx ),
										  0,
										  phit_list.at( pmax_idx ),
										  false,
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
				    ( TMath::Abs( pwalk_list.at( pindex.at(0) ) - pwalk_list.at( pindex.at(1) ) ) < set->GetArrayPPHitWindow()
					 || TMath::Abs( ptd_list.at( pindex.at(0) ) - ptd_list.at( pindex.at(1) ) ) == 0 ) &&
				    ( TMath::Abs( nwalk_list.at( nindex.at(0) ) - nwalk_list.at( nindex.at(1) ) ) < set->GetArrayNNHitWindow()
					|| TMath::Abs( ntd_list.at( nindex.at(0) ) - ntd_list.at( nindex.at(1) ) ) == 0 ) ) {
				    
					// Simple sum of both energies, cross-talk not included yet
					psum_en  = pen_list.at( pindex.at(0) );
					psum_en += pen_list.at( pindex.at(1) );
					nsum_en  = nen_list.at( nindex.at(0) );
					nsum_en += nen_list.at( nindex.at(1) );
					
					// Check p and n prompt with each other
					if( TMath::Abs( pwalk_list.at( pindex.at(0) ) - nwalk_list.at( nindex.at(0) ) ) < set->GetArrayPNHitWindow() ||
					    TMath::Abs( pwalk_list.at( pindex.at(0) ) - nwalk_list.at( nindex.at(1) ) ) < set->GetArrayPNHitWindow() ||
					    TMath::Abs( pwalk_list.at( pindex.at(1) ) - nwalk_list.at( nindex.at(0) ) ) < set->GetArrayPNHitWindow() ||
					    TMath::Abs( pwalk_list.at( pindex.at(1) ) - nwalk_list.at( nindex.at(1) ) ) < set->GetArrayPNHitWindow() ){
					
						// Fill addback histogram
						pn_ab[i][j]->Fill( psum_en, nsum_en );

						// Fill the addback event
						array_evt->SetEvent( psum_en,
											 nsum_en,
											 pid_list.at( pmax_idx ),
											 nid_list.at( nmax_idx ),
											 pwalk_list.at( pmax_idx ),
											 nwalk_list.at( nmax_idx ),
											 phit_list.at( pmax_idx ),
											 nhit_list.at( nmax_idx ),
											 i, j );

						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;

					}
					
					else {
					
						// Discard two n's and just take two p's
						// Fill addback histogram
						pn_pab[i][j]->Fill( psum_en, -1 );
						
						// Fill the addback event
						arrayp_evt->SetEvent( psum_en,
											  0,
											  pid_list.at( pmax_idx ),
											  5,
											  pwalk_list.at( pmax_idx ),
											  0,
											  phit_list.at( pmax_idx ),
											  false,
											  i, j );

						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
					
					}

				}
				
				// Neighbour strips - p-side only and only p-prompt coincidences
				else if( TMath::Abs( pid_list.at( pindex.at(0) ) - pid_list.at( pindex.at(1) ) ) == 1 &&
						( TMath::Abs( pwalk_list.at( pindex.at(0) ) - pwalk_list.at( pindex.at(1) ) ) < set->GetArrayPPHitWindow()
						 || TMath::Abs( ptd_list.at( pindex.at(0) ) - ptd_list.at( pindex.at(1) ) ) == 0 ) ) {
					
					// Simple sum of both energies, cross-talk not included yet
					psum_en  = pen_list.at( pindex.at(0) );
					psum_en += pen_list.at( pindex.at(1) );
					
					// Check if any of the n-sides coincident with p
					// n0 coincident with pp
					if ( TMath::Abs( pwalk_list.at( pindex.at(0) ) - nwalk_list.at( nindex.at(0) ) ) < set->GetArrayPNHitWindow() ||
					     TMath::Abs( pwalk_list.at( pindex.at(1) ) - nwalk_list.at( nindex.at(0) ) ) < set->GetArrayPNHitWindow() ){
						
						// Fill addback histogram
						pn_pab[i][j]->Fill( psum_en, nen_list.at( nindex.at(0) ) );

						// Fill the addback event for p-side, but max for n-side
						array_evt->SetEvent( psum_en,
											 nen_list.at( nindex.at(0) ),
											 pid_list.at( pmax_idx ),
											 nid_list.at( nindex.at(0) ),
											 pwalk_list.at( pmax_idx ),
											 nwalk_list.at( nindex.at(0) ),
											 phit_list.at( pmax_idx ),
											 nhit_list.at( nindex.at(0) ),
											 i, j );

						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
						
					}
					
					// n1 coincident with pp
					else if ( TMath::Abs( ptd_list.at( pindex.at(0) ) - ntd_list.at( nindex.at(1) ) ) < set->GetArrayPNHitWindow() ||
					          TMath::Abs( ptd_list.at( pindex.at(1) ) - ntd_list.at( nindex.at(1) ) ) < set->GetArrayPNHitWindow() ){
						
						// Fill addback histogram
						pn_pab[i][j]->Fill( psum_en, nen_list.at( nindex.at(1) ) );

						// Fill the addback event for p-side, but max for n-side
						array_evt->SetEvent( psum_en,
											 nen_list.at( nindex.at(1) ),
											 pid_list.at( pmax_idx ),
											 nid_list.at( nindex.at(1) ),
											 pwalk_list.at( pmax_idx ),
											 nwalk_list.at( nindex.at(1) ),
											 phit_list.at( pmax_idx ),
											 nhit_list.at( nindex.at(1) ),
											 i, j );

						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;
						
					}
					
					// p's coincident but n's are not with p's or each other
					else {

						// Fill addback histogram
						pn_pab[i][j]->Fill( psum_en, -1 );
				
						// No n-sides coincident -> p-sides only
						arrayp_evt->SetEvent( psum_en,
										  0,
										  pid_list.at( pmax_idx ),
										  5,
										  pwalk_list.at( pmax_idx ),
										  0,
										  phit_list.at( pmax_idx ),
										  false,
										  i, j );

						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;

					}

				}

				// Neighbour strips - n-side only prompt and p not prompt
				else if( TMath::Abs( nid_list.at( nindex.at(0) ) - nid_list.at( nindex.at(1) ) ) == 1 &&
						( TMath::Abs( nwalk_list.at( nindex.at(0) ) - nwalk_list.at( nindex.at(1) ) ) < set->GetArrayNNHitWindow()
						 || TMath::Abs( ntd_list.at( nindex.at(0) ) - ntd_list.at( nindex.at(1) ) ) == 0 ) ) {
					
					// Simple sum of both energies, cross-talk not included yet
					nsum_en  = nen_list.at( nindex.at(0) );
					nsum_en += nen_list.at( nindex.at(1) );

					// Check p0 with n sides
					if ( TMath::Abs( pwalk_list.at( pindex.at(0) ) - nwalk_list.at( nindex.at(0) ) ) < set->GetArrayPNHitWindow() ||
					     TMath::Abs( pwalk_list.at( pindex.at(0) ) - nwalk_list.at( nindex.at(1) ) ) < set->GetArrayPNHitWindow() ){
					
						// Fill addback histogram
						pn_nab[i][j]->Fill( pen_list.at( pindex.at(0) ), nsum_en );

						// Fill the addback event
						array_evt->SetEvent( pen_list.at( pindex.at(0) ),
											 nsum_en,
											 pid_list.at( pindex.at(0) ),
											 nid_list.at( nmax_idx ),
											 pwalk_list.at( pindex.at(0) ),
											 nwalk_list.at( nmax_idx ),
											 phit_list.at( pindex.at(0) ),
											 nhit_list.at( nmax_idx ),
											 i, j );

						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;

					}
					
					// Check p1 with n sides
					else if( TMath::Abs( pwalk_list.at( pindex.at(1) ) - nwalk_list.at( nindex.at(0) ) ) < set->GetArrayPNHitWindow() ||
					         TMath::Abs( pwalk_list.at( pindex.at(1) ) - nwalk_list.at( nindex.at(1) ) ) < set->GetArrayPNHitWindow() ){

						// Fill addback histogram
						pn_nab[i][j]->Fill( pen_list.at( pindex.at(1) ), nsum_en );

						// Fill the addback event
						array_evt->SetEvent( pen_list.at( pindex.at(1) ),
											 nsum_en,
											 pid_list.at( pindex.at(1) ),
											 nid_list.at( nmax_idx ),
											 pwalk_list.at( pindex.at(1) ),
											 nwalk_list.at( nmax_idx ),
											 phit_list.at( pindex.at(1) ),
											 nhit_list.at( nmax_idx ),
											 i, j );

						write_evts->AddEvt( array_evt );
						array_ctr++;

						// High n-side threshold situation, fill p-only event
						arrayp_evt->CopyEvent( array_evt );
						write_evts->AddEvt( arrayp_evt );
						arrayp_ctr++;

					}
					
					// n sides coincident but neither p coincident with them or each other
					else {
						// ??? TODO: we need to build some p-only events here
					}

				}
				
				// Non-neighbour strips, maybe two events?
				else {
				
					// Pairing [0,0] because energy difference is smaller than [1,0]
					if( TMath::Abs( pen_list.at( pindex.at(0) ) - nen_list.at( nindex.at(0) ) ) <
					    TMath::Abs( pen_list.at( pindex.at(1) ) - nen_list.at( nindex.at(0) ) ) ) {
						
						// Try filling the [0,0] and then the [1,1]
						for( int k = 0; k < 2; ++k ){
							
							ptmp_idx = pindex.at(k);
							ntmp_idx = nindex.at(k);
							
							// Prompt coincidence condition
							if( TMath::Abs( pwalk_list.at( ptmp_idx ) - nwalk_list.at( ntmp_idx ) ) < set->GetArrayPNHitWindow() ){
							
								// Fill addback histogram
								pn_ab[i][j]->Fill( pen_list.at( ptmp_idx ), nen_list.at( ntmp_idx ) );

								array_evt->SetEvent( pen_list.at( ptmp_idx ),
													 nen_list.at( ntmp_idx ),
													 pid_list.at( ptmp_idx ),
													 nid_list.at( ntmp_idx ),
													 pwalk_list.at( ptmp_idx ),
													 nwalk_list.at( ntmp_idx ),
													 phit_list.at( ptmp_idx ),
													 nhit_list.at( ntmp_idx ),
													 i, j );

								write_evts->AddEvt( array_evt );
								array_ctr++;

								// High n-side threshold situation, fill p-only event
								arrayp_evt->CopyEvent( array_evt );
								write_evts->AddEvt( arrayp_evt );
								arrayp_ctr++;
							}
							
							else {
								
								// [0,0]/[1,1] not in prompt coincidence, so just store p as separate event
								arrayp_evt->SetEvent( pen_list.at( ptmp_idx ),
													  0,
													  pid_list.at( ptmp_idx ),
													  5,
													  pwalk_list.at( ptmp_idx ),
													  0,
													  phit_list.at( ptmp_idx ),
													  false,
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
							if( TMath::Abs( pwalk_list.at( ptmp_idx ) - nwalk_list.at( ntmp_idx ) ) < set->GetArrayPNHitWindow() ){
								
								// Fill addback histogram
								pn_ab[i][j]->Fill( pen_list.at( ptmp_idx ), nen_list.at( ntmp_idx ) );

								array_evt->SetEvent( pen_list.at( ptmp_idx ),
													 nen_list.at( ntmp_idx ),
													 pid_list.at( ptmp_idx ),
													 nid_list.at( ntmp_idx ),
													 pwalk_list.at( ptmp_idx ),
													 nwalk_list.at( ntmp_idx ),
													 phit_list.at( ptmp_idx ),
													 nhit_list.at( ntmp_idx ),
													 i, j );

								write_evts->AddEvt( array_evt );
								array_ctr++;

								// High n-side threshold situation, fill p-only event
								arrayp_evt->CopyEvent( array_evt );
								write_evts->AddEvt( arrayp_evt );
								arrayp_ctr++;
							}
							
							else {
							
								// [0,0]/[1,1] not in prompt coincidence, so just store p as separate event
								arrayp_evt->SetEvent( pen_list.at( ptmp_idx ),
													  0,
													  pid_list.at( ptmp_idx ),
													  5,
													  pwalk_list.at( ptmp_idx ),
													  0,
													  phit_list.at( ptmp_idx ),
													  false,
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
									 pwalk_list.at( pmax_idx ),
									 nwalk_list.at( nmax_idx ),
									 phit_list.at( pmax_idx ),
									 nhit_list.at( nmax_idx ),
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

	//std::cout << __PRETTY_FUNCTION__ << std::endl;
	
	// Checks to prevent re-using events
	std::vector<unsigned int> index;
	std::vector<unsigned int> layers;
	bool flag_skip;
	
	// Loop over recoil events
	for( unsigned int i = 0; i < ren_list.size(); ++i ) {
		
		// Find the dE event, usually the trigger
		if( rid_list[i] == (int)set->GetRecoilEnergyLossStart() ){
			
			recoil_evt->ClearEvent();
			recoil_evt->SetdETime( rtd_list[i] );
			recoil_evt->SetSector( rsec_list[i] );
			recoil_evt->AddRecoil( ren_list[i], rid_list[i] );
			
			index.push_back(i);
			layers.push_back(rid_list[i]);

			// Look for matching E events
			for( unsigned int j = 0; j < ren_list.size(); ++j ) {

				// Check if we already used this hit
				flag_skip = false;
				for( unsigned int k = 0; k < index.size(); ++k ) {
					if( index[k] == j ) flag_skip = true;
					if( (int)layers[k] == rid_list[j] ) flag_skip = true;
				}
				
				// Found a match
				// ^^^ Not sure if this will work with the ionisation chamber!
				if( i != j && 		// Not looking at the same hit
				   !flag_skip &&	// Not looking at a previously-used hit
				   rsec_list[i] == rsec_list[j] &&		// They are in the same sector
				   rid_list[i] != rid_list[j]			// They are not in the same layer
				   ){
					
					if( rid_list[j] == (int)set->GetRecoilEnergyRestStart() )
						recoil_E_dE_tdiff[rsec_list[i]]->Fill( rtd_list[j] - rtd_list[i] );
					recoil_tdiff[rsec_list[i]]->Fill( rid_list[j], rtd_list[j] - rtd_list[i] );
					
					// The hits lie within the recoil hit window
					if( TMath::Abs( rtd_list[i] - rtd_list[j] ) < set->GetRecoilHitWindow() ) {
						
						index.push_back(j);
						layers.push_back(rid_list[j]);
						recoil_evt->AddRecoil( ren_list[j], rid_list[j] );
						
						if( rid_list[j] == (int)set->GetRecoilEnergyRestStart() )
							recoil_evt->SetETime( rtd_list[j] );
						
					}
					
				}
				
			}
			
			// Histogram the recoils
			recoil_EdE[rsec_list[i]]->Fill( recoil_evt->GetEnergyRest( set->GetRecoilEnergyRestStart(), set->GetRecoilEnergyRestStop() ),
								recoil_evt->GetEnergyLoss( set->GetRecoilEnergyLossStart(), set->GetRecoilEnergyLossStop() ) );
			recoil_dEsum[rsec_list[i]]->Fill( recoil_evt->GetEnergyTotal( set->GetRecoilEnergyTotalStart(), set->GetRecoilEnergyTotalStop() ),
								recoil_evt->GetEnergyLoss( set->GetRecoilEnergyLossStart(), set->GetRecoilEnergyLossStop() ) );
			recoil_E_singles[rsec_list[i]]->Fill( recoil_evt->GetEnergyRest( set->GetRecoilEnergyRestStart(), set->GetRecoilEnergyRestStop() ) );
			recoil_dE_singles[rsec_list[i]]->Fill( recoil_evt->GetEnergyLoss( set->GetRecoilEnergyLossStart(), set->GetRecoilEnergyLossStop() ) );
			
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

	//std::cout << __PRETTY_FUNCTION__ << std::endl;
	
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
	
	//std::cout << __PRETTY_FUNCTION__ << std::endl;
	
	// Loop over ELUM events
	for( unsigned int i = 0; i < een_list.size(); ++i ) {

		// Set the ELUM event (nice and easy)
		elum_evt->SetEvent( een_list[i], 0,
							esec_list[i], etd_list[i] );

		// Write event to tree
		write_evts->AddEvt( elum_evt );
		elum_ctr++;

		// Histogram the data
		elum_E->Fill( een_list[i] );
		elum_E_vs_sec->Fill( esec_list[i], een_list[i] );

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

	//std::cout << __PRETTY_FUNCTION__ << std::endl;
	
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
					zd_EdE->Fill( zen_list[j], zen_list[i] );

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
/// Builds gamma-ray events from the ScintArray and maybe also HPGe detectors in the future
void ISSEventBuilder::GammaRayFinder() {
	
	//std::cout << __PRETTY_FUNCTION__ << std::endl;
	
	// Loop over ScintArray events
	for( unsigned int i = 0; i < saen_list.size(); ++i ) {
		
		// Histogram the data
		gamma_E->Fill( saen_list[i] );
		gamma_E_vs_det->Fill( said_list[i], saen_list[i] );
		
		// Coincidences
		for( unsigned int j = i+1; j < saen_list.size(); ++j ) {
			
			double tdiff = satd_list[j] - satd_list[i];
			gamma_gamma_td->Fill( tdiff );
			gamma_gamma_td->Fill( -tdiff );
			
			// Just prompt hits for now in a gg matrix
			// This should really be used for add-back?
			if( TMath::Abs(tdiff) < set->GetGammaRayHitWindow() ){
				
				gamma_gamma_E->Fill( saen_list[i], saen_list[j] );
				gamma_gamma_E->Fill( saen_list[j], saen_list[i] );

			} // prompt
				
		} // j
		
		// Set the GammaRay event (nice and easy, ScintArray type = 0)
		gamma_evt->SetEvent( saen_list[i], said_list[i],
							 0, satd_list[i] );

		// Write event to tree
		write_evts->AddEvt( gamma_evt );
		gamma_ctr++;
		
	} // i
	
	return;
	
}

void ISSEventBuilder::LumeFinder() {

	size_t n_index = 0;
	size_t f_index = 0;

	for (size_t b_index = 0; b_index < lbe_list.size(); ++b_index) {
		double be_timestamp = lbe_td_list[b_index];
		float be_energy = lbe_list[b_index];
		char be_id = lbe_id_list[b_index];
		double time_window = set->GetLumeHitWindow();

		lume_E[be_id]->Fill(be_energy);
		// Find the corresponding n signal
		bool has_ln = false;
		float ne_energy = 0;

		for (; n_index < lne_list.size(); ++n_index) {

			if (lne_id_list[n_index] != be_id) continue;

			if (std::abs(lne_td_list[n_index] - be_timestamp) <= time_window) {
				ne_energy = lne_list[n_index];
				++n_index;
				has_ln = true;
				break;
			}

			}

		// Find the corresponding f signal
		bool has_lf = false;
		float fe_energy = 0;

		for (; f_index < lfe_list.size(); ++f_index) {

			if (lfe_id_list[f_index] != be_id) continue;

			if (std::abs(lfe_td_list[f_index] - be_timestamp) <= time_window) {
				fe_energy = lfe_list[f_index];
				++f_index;
				has_lf = true;
				break;
			}
		}

		lume_evt->SetEvent(be_energy, be_id, be_timestamp, has_ln ? ne_energy : TMath::QuietNaN(), has_lf ? fe_energy : TMath::QuietNaN() );

		if (has_ln && has_lf)
		  lume_E_vs_x[be_id]->Fill(( ne_energy - fe_energy ) / ( ne_energy + fe_energy ), be_energy);

		write_evts->AddEvt( lume_evt );
		lume_ctr++;
	}

	return;
}

////////////////////////////////////////////////////////////////////////////////
/// *This function doesn't fill any histograms*, but just creates them. Called by the ISSEventBuilder::SetOutput function
void ISSEventBuilder::MakeHists(){
	
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

	caen_period = new TH1F( "caen_period", "Period of pulser in CAEN DAQ as a function of time;time [ns];f [Hz]", 1000, 0, 1e9 );

	asic_td.resize( set->GetNumberOfArrayModules() );
	asic_period.resize( set->GetNumberOfArrayModules() );
	asic_sync.resize( set->GetNumberOfArrayModules() );
	asic_pulser_loss.resize( set->GetNumberOfArrayModules() );
	fpga_td.resize( set->GetNumberOfArrayModules() );
	fpga_period.resize( set->GetNumberOfArrayModules() );
	fpga_sync.resize( set->GetNumberOfArrayModules() );
	fpga_pulser_loss.resize( set->GetNumberOfArrayModules() );
	
	// Loop over ISS modules
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {

		hname = "asic_td_" + std::to_string(i);
		htitle = "Time difference between ASIC and CAEN pulser events in module ";
		htitle += std::to_string(i) + ";#Delta t [ns]";
		asic_td[i] = new TH1F( hname.data(), htitle.data(), 1.6e3 , -4e3, 4e3 );
		
		hname = "asic_period_" + std::to_string(i);
		htitle = "Period of pulser in ISS DAQ (ASICs) in module ";
		htitle += std::to_string(i) + ";time [ns];f [Hz]";
		asic_period[i] = new TH1F( hname.data(), htitle.data(), 1000, 0, 1e9 );
		
		hname = "asic_sync_" + std::to_string(i);
		htitle = "Time difference between ASIC and CAEN events as a function of time in module ";
		htitle += std::to_string(i) + ";time [ns];#Delta t [ns]";
		asic_sync[i] = new TProfile( hname.data(), htitle.data(), 10.8e4, 0, 10.8e12 );
		
		hname = "asic_pulser_loss_" + std::to_string(i);
		htitle = "Number of missing/extra pulser events in ASICs as a function of time in module ";
		htitle += std::to_string(i) + ";time [ns];(-ive CAEN missing, +ive ISS missing)";
		asic_pulser_loss[i] = new TProfile( hname.data(), htitle.data(), 10.8e4, 0, 10.8e12 );
		
		hname = "fpga_td_" + std::to_string(i);
		htitle = "Time difference between FPGA and CAEN pulser events in module ";
		htitle += std::to_string(i) + ";#Delta t [ns]";
		fpga_td[i] = new TH1F( hname.data(), htitle.data(), 1.6e3 , -4e3, 4e3 );

		hname = "fpga_period_" + std::to_string(i);
		htitle = "Period of pulser in ISS DAQ (FPGA) in module ";
		htitle += std::to_string(i) + ";time [ns];f [Hz]";
		fpga_period[i] = new TH1F( hname.data(), htitle.data(), 1000, 0, 1e9 );

		hname = "fpga_sync_" + std::to_string(i);
		htitle = "Time difference between FPGA and CAEN events as a function of time in module ";
		htitle += std::to_string(i) + ";time [ns];#Delta t [ns]";
		fpga_sync[i] = new TProfile( hname.data(), htitle.data(), 10.8e4, 0, 10.8e12 );

		hname = "fpga_pulser_loss_" + std::to_string(i);
		htitle = "Number of missing/extra pulser events in FPGA as a function of time in module ";
		htitle += std::to_string(i) + ";#time [ns];(-ive CAEN missing, +ive ISS missing)";
		fpga_pulser_loss[i] = new TProfile( hname.data(), htitle.data(), 10.8e4, 0, 10.8e12 );

	}

	ebis_period = new TH1F( "ebis_period", "Period of EBIS events;T [ns]", 3000, 0, 3e9 );
	t1_period = new TH1F( "t1_period", "Period of T1 events (p+ on ISOLDE target);T [ns]", 1000, 0, 100e9 );
	sc_period = new TH1F( "sc_period", "Period of SuperCycle events (PSB cycle start);T [ns]", 1000, 0, 1000e9 );
	laser_period = new TH1F( "laser_period", "Period of Laser Status events (triggered by EBIS);T [ns]", 1000, 0, 10e9 );
	supercycle = new TH1F( "supercycle", "SuperCycle structure;T1-SC [ns]", 1000, 0, 100e9 );

	
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
	pn_td_uncorrected.resize( set->GetNumberOfArrayModules() );
	pn_td_Ep_uncorrected.resize( set->GetNumberOfArrayModules() );
	pn_td_En_uncorrected.resize( set->GetNumberOfArrayModules() );
	pp_td.resize( set->GetNumberOfArrayModules() );
	nn_td.resize( set->GetNumberOfArrayModules() );
	pn_mult.resize( set->GetNumberOfArrayModules() );

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
		pn_td_uncorrected[i].resize( set->GetNumberOfArrayRows() );
		pn_td_Ep_uncorrected[i].resize( set->GetNumberOfArrayRows() );
		pn_td_En_uncorrected[i].resize( set->GetNumberOfArrayRows() );
		pp_td[i].resize( set->GetNumberOfArrayRows() );
		nn_td[i].resize( set->GetNumberOfArrayRows() );
		pn_mult[i].resize( set->GetNumberOfArrayRows() );
		
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
			htitle = "p-side vs. n-side time difference after correction (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");time difference [ns];counts";
			pn_td[i][j] = new TH1F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20 );
			
			hname = "pn_td_Ep_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side n-side time difference after correction vs p-side energy (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");time difference [ns];p-side energy [keV]";
			pn_td_Ep[i][j] = new TH2F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20, 2e3, 0, 2e4 );
			
			hname = "pn_td_En_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side n-side time difference after correction vs n-side energy (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");time difference [ns];n-side energy [keV]";
			pn_td_En[i][j] = new TH2F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20, 2e3, 0, 2e4  );
						
			hname = "pn_td_uncorrected_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side vs. n-side time difference before correction (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");time difference [ns];counts";
			pn_td_uncorrected[i][j] = new TH1F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20 );
			
			hname = "pn_td_Ep_uncorrected_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side n-side time difference before correction vs p-side energy (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");time difference [ns];p-side energy [keV]";
			pn_td_Ep_uncorrected[i][j] = new TH2F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20, 2e3, 0, 2e4 );
			
			hname = "pn_td_En_uncorrected_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side n-side time difference before correction vs n-side energy (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");time difference [ns];n-side energy [keV]";
			pn_td_En_uncorrected[i][j] = new TH2F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20, 2e3, 0, 2e4  );
						
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
	recoil_tdiff.resize( set->GetNumberOfRecoilSectors() );

	// Loop over number of recoil sectors
	for( unsigned int i = 0; i < set->GetNumberOfRecoilSectors(); ++i ) {
	
		hname = "recoil_EdE" + std::to_string(i);
		htitle = "Recoil dE vs E for sector " + std::to_string(i);
		htitle += ";Rest energy, E [keV];Energy loss, dE [keV];Counts";
		recoil_EdE[i] = new TH2F( hname.data(), htitle.data(), 4000, 0, 800000, 4000, 0, 800000 );
		
		hname = "recoil_dEsum" + std::to_string(i);
		htitle = "Recoil dE vs Esum for sector " + std::to_string(i);
		htitle += ";Total energy, Esum [keV];Energy loss, dE [keV];Counts";
		recoil_dEsum[i] = new TH2F( hname.data(), htitle.data(), 4000, 0, 800000, 4000, 0, 800000 );
		
		hname = "recoil_EdE_raw" + std::to_string(i);
		htitle = "Recoil dE vs E for sector " + std::to_string(i);
		htitle += ";Rest energy, E [arb.];Energy loss, dE [arb.];Counts";
		recoil_EdE_raw[i] = new TH2F( hname.data(), htitle.data(), 2048, 0, 65536, 2048, 0, 65536 );
	
		hname = "recoil_E_singles" + std::to_string(i);		
		htitle = "Recoil E singles in sector " + std::to_string(i);
		htitle += "; E [keV]; Counts";
		recoil_E_singles[i] = new TH1F( hname.data(), htitle.data(), 4000, 0, 800000 );
		
		hname = "recoil_dE_singles" + std::to_string(i);		
		htitle = "Recoil dE singles in sector " + std::to_string(i);
		htitle += "; dE [keV]; Counts";
		recoil_dE_singles[i] = new TH1F( hname.data(), htitle.data(), 4000, 0, 800000 );
		
		hname = "recoil_E_dE_tdiff" + std::to_string(i);		
		htitle = "Recoil E-dE time difference in sector" + std::to_string(i);
		htitle += "; #Delta t [ns]; Counts";
		recoil_E_dE_tdiff[i] = new TH1F( hname.data(), htitle.data(), 2000, -6e3, 6e3 );

		hname = "recoil_tdiff" + std::to_string(i);
		htitle = "Recoil-Recoil time difference in sector " + std::to_string(i);
		htitle += " with respect to layer " + std::to_string( set->GetRecoilEnergyLossStart() );
		htitle += ";Recoil layer ID;#Delta t [ns];Counts";
		recoil_tdiff[i] = new TH2F( hname.data(), htitle.data(), set->GetNumberOfRecoilLayers(), -0.5, set->GetNumberOfRecoilLayers()-0.5, 2000, -6e3, 6e3 );
		
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
	dirname = "elum";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );
	
	hname = "elum_E_vs_sec";
	htitle = "ELUM energy vs sector;Sector;Energy [keV];Counts";
	elum_E_vs_sec = new TH2F( hname.data(), htitle.data(),
			set->GetNumberOfELUMSectors()+1, -0.5, set->GetNumberOfELUMSectors()+0.5, 2000, 0, 20000 );

	hname = "elum_E";
	htitle = "ELUM energy;Energy [keV];Counts";
	elum_E = new TH1F( hname.data(), htitle.data(), 2000, 0, 20000 );


	// --------------------- //
	// ZeroDegree histograms //
	// --------------------- //
	dirname = "zd";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );
	
	hname = "zd_EdE";
	htitle = "ZeroDegree dE vs E;Rest Energy [keV];Energy Loss [keV];Counts";
	zd_EdE = new TH2F( hname.data(), htitle.data(), 2000, 0, 20000, 4000, 0, 800000 );

	
	// -------------------- //
	// Gamma-ray histograms //
	// -------------------- //
	dirname = "gammas";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );
	

	hname = "gamma_E_vs_det";
	htitle = "Gamma-ray energy vs detector ID;Detector ID;Energy [keV];Counts per 2 keV";
	gamma_E_vs_det = new TH2F( hname.data(), htitle.data(),
			set->GetNumberOfScintArrayDetectors()+1, -0.5, set->GetNumberOfScintArrayDetectors()+0.5, 4000, 0, 8000 );

	hname = "gamma_E";
	htitle = "Gamma-ray energy;Energy [keV];Counts per 2 keV";
	gamma_E = new TH1F( hname.data(), htitle.data(), 4000, 0, 8000 );

	hname = "gamma_gamma_E";
	htitle = "Gamma-ray energy coincidence matrix;Energy [keV];Energy [keV];Counts";
	gamma_gamma_E = new TH2F( hname.data(), htitle.data(), 4000, 0, 8000, 4000, 0, 8000 );

	hname = "gamma_gamma_td";
	htitle = "Gamma-gamma time difference;#Deltat [ns];Counts";
	gamma_gamma_td = new TH1F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20 );

	// -------------------- //
	// LUME histograms      //
	// -------------------- //
	dirname = "lume";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	lume_E.resize( set->GetNumberOfLUMEDetectors() );
	lume_E_vs_x.resize( set->GetNumberOfLUMEDetectors() );

	// Loop over number of LUME detectors
	for( unsigned int i = 0; i < set->GetNumberOfLUMEDetectors(); ++i ) {

		hname = "lume_E_" + std::to_string(i);
		htitle = "LUME energy spectrum;Energy [ch];Counts";
		lume_E[i] = new TH1F( hname.data(), htitle.data(), 1000, -200, 80000);

		hname = "lume_E_vs_x_" + std::to_string(i);
		htitle = "LUME energy vs position spectrum;Position;Energy [ch]";
		lume_E_vs_x[i] = new TH2F( hname.data(), htitle.data(), 1000, -1.01, 1.01, 1000, -200, 80000 );

	}

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

	for( unsigned int i = 0; i < pn_td_uncorrected.size(); i++ ) {
		for( unsigned int j = 0; j < pn_td_uncorrected.at(i).size(); j++ )
			delete (pn_td_uncorrected[i][j]);
		pn_td_uncorrected.clear();
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
	
	for( unsigned int i = 0; i < pn_td_Ep_uncorrected.size(); i++ ) {
		for( unsigned int j = 0; j < pn_td_Ep_uncorrected.at(i).size(); j++ )
			delete (pn_td_Ep_uncorrected[i][j]);
		pn_td_Ep_uncorrected.clear();
	}
	
	for( unsigned int i = 0; i < pn_td_En_uncorrected.size(); i++ ) {
		for( unsigned int j = 0; j < pn_td_En_uncorrected.at(i).size(); j++ )
			delete (pn_td_En_uncorrected[i][j]);
		pn_td_En_uncorrected.clear();
	}
	
	for( unsigned int i = 0; i < pn_mult.size(); i++ ) {
		for( unsigned int j = 0; j < pn_mult.at(i).size(); j++ )
			delete (pn_mult[i][j]);
		pn_mult.clear();
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

	for( unsigned int i = 0; i < recoil_tdiff.size(); i++ )
		delete (recoil_tdiff[i]);
	recoil_tdiff.clear();


	for( unsigned int i = 0; i < mwpc_tac_axis.size(); i++ ) {
		for( unsigned int j = 0; j < mwpc_tac_axis.at(i).size(); j++ )
			delete (mwpc_tac_axis[i][j]);
		mwpc_tac_axis.clear();
	}

	for( unsigned int i = 0; i < mwpc_hit_axis.size(); i++ )
		delete (mwpc_hit_axis[i]);
	mwpc_hit_axis.clear();
	
	delete mwpc_pos;
	delete elum_E;
	delete elum_E_vs_sec;
	delete zd_EdE;
	delete gamma_E;
	delete gamma_E_vs_det;
	delete gamma_gamma_E;
	delete gamma_gamma_td;

	for( unsigned int i = 0; i < lume_E.size(); i++ )
		delete (lume_E[i]);
	lume_E.clear();

	for( unsigned int i = 0; i < lume_E_vs_x.size(); i++ )
		delete (lume_E_vs_x[i]);
	lume_E_vs_x.clear();

	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); i++ ){
		delete (fpga_td[i]);
		delete (asic_td[i]);
		delete (fpga_pulser_loss[i]);
		delete (fpga_period[i]);
		delete (fpga_sync[i]);
		delete (asic_pulser_loss[i]);
		delete (asic_period[i]);
		delete (asic_sync[i]);
	}
		
	fpga_td.clear();
	fpga_sync.clear();
	fpga_period.clear();
	fpga_pulser_loss.clear();
	asic_td.clear();
	asic_sync.clear();
	asic_period.clear();
	asic_pulser_loss.clear();
	
	delete tdiff;
	delete tdiff_clean;
	delete caen_period;
	delete ebis_period;
	delete t1_period;
	delete sc_period;
	delete laser_period;
	delete supercycle;

	return;

}

////////////////////////////////////////////////////////////////////////////////
/// This function empties the histograms used in the EventBuilder class; used during the DataSpy
void ISSEventBuilder::ResetHists() {

	for( unsigned int i = 0; i < pn_11.size(); i++ )
		for( unsigned int j = 0; j < pn_11.at(i).size(); j++ )
			pn_11[i][j]->Reset("ICESM");

	for( unsigned int i = 0; i < pn_12.size(); i++ )
		for( unsigned int j = 0; j < pn_12.at(i).size(); j++ )
			pn_12[i][j]->Reset("ICESM");

	for( unsigned int i = 0; i < pn_21.size(); i++ )
		for( unsigned int j = 0; j < pn_21.at(i).size(); j++ )
			pn_21[i][j]->Reset("ICESM");

	for( unsigned int i = 0; i < pn_22.size(); i++ )
		for( unsigned int j = 0; j < pn_22.at(i).size(); j++ )
			pn_22[i][j]->Reset("ICESM");

	for( unsigned int i = 0; i < pn_ab.size(); i++ )
		for( unsigned int j = 0; j < pn_ab.at(i).size(); j++ )
			pn_ab[i][j]->Reset("ICESM");

	for( unsigned int i = 0; i < pn_nab.size(); i++ )
		for( unsigned int j = 0; j < pn_nab.at(i).size(); j++ )
			pn_nab[i][j]->Reset("ICESM");

	for( unsigned int i = 0; i < pn_pab.size(); i++ )
		for( unsigned int j = 0; j < pn_pab.at(i).size(); j++ )
			pn_pab[i][j]->Reset("ICESM");

	for( unsigned int i = 0; i < pn_max.size(); i++ )
		for( unsigned int j = 0; j < pn_max.at(i).size(); j++ )
			pn_max[i][j]->Reset("ICESM");

	for( unsigned int i = 0; i < pn_td.size(); i++ )
		for( unsigned int j = 0; j < pn_td.at(i).size(); j++ )
			pn_td[i][j]->Reset("ICESM");
	
	for( unsigned int i = 0; i < pn_td_uncorrected.size(); i++ )
		for( unsigned int j = 0; j < pn_td_uncorrected.at(i).size(); j++ )
			pn_td_uncorrected[i][j]->Reset("ICESM");
	
	for( unsigned int i = 0; i < pp_td.size(); i++ )
		for( unsigned int j = 0; j < pp_td.at(i).size(); j++ )
			pp_td[i][j]->Reset("ICESM");

	for( unsigned int i = 0; i < nn_td.size(); i++ )
		for( unsigned int j = 0; j < nn_td.at(i).size(); j++ )
			nn_td[i][j]->Reset("ICESM");

	for( unsigned int i = 0; i < pn_td_Ep.size(); i++ )
		for( unsigned int j = 0; j < pn_td_Ep.at(i).size(); j++ )
			pn_td_Ep[i][j]->Reset("ICESM");
	
	for( unsigned int i = 0; i < pn_td_En.size(); i++ )
		for( unsigned int j = 0; j < pn_td_En.at(i).size(); j++ )
			pn_td_En[i][j]->Reset("ICESM");
	
	for( unsigned int i = 0; i < pn_td_Ep_uncorrected.size(); i++ )
		for( unsigned int j = 0; j < pn_td_Ep_uncorrected.at(i).size(); j++ )
			pn_td_Ep_uncorrected[i][j]->Reset("ICESM");
	
	for( unsigned int i = 0; i < pn_td_En_uncorrected.size(); i++ )
		for( unsigned int j = 0; j < pn_td_En_uncorrected.at(i).size(); j++ )
			pn_td_En_uncorrected[i][j]->Reset("ICESM");
	
	for( unsigned int i = 0; i < pn_mult.size(); i++ )
		for( unsigned int j = 0; j < pn_mult.at(i).size(); j++ )
			pn_mult[i][j]->Reset("ICESM");


	for( unsigned int i = 0; i < recoil_EdE.size(); i++ )
		recoil_EdE[i]->Reset("ICESM");
	
	for( unsigned int i = 0; i < recoil_dEsum.size(); i++ )
		recoil_dEsum[i]->Reset("ICESM");
	
	for( unsigned int i = 0; i < recoil_EdE_raw.size(); i++ )
		recoil_EdE_raw[i]->Reset("ICESM");
		
	for( unsigned int i = 0; i < recoil_E_singles.size(); i++ )
		recoil_E_singles[i]->Reset("ICESM");
		
	for( unsigned int i = 0; i < recoil_dE_singles.size(); i++ )
		recoil_dE_singles[i]->Reset("ICESM");
		
	for( unsigned int i = 0; i < recoil_E_dE_tdiff.size(); i++ )
		recoil_E_dE_tdiff[i]->Reset("ICESM");
	
	for( unsigned int i = 0; i < recoil_tdiff.size(); i++ )
		recoil_tdiff[i]->Reset("ICESM");
	
	for( unsigned int i = 0; i < mwpc_tac_axis.size(); i++ )
		for( unsigned int j = 0; j < mwpc_tac_axis.at(i).size(); j++ )
			mwpc_tac_axis[i][j]->Reset("ICESM");

	for( unsigned int i = 0; i < mwpc_hit_axis.size(); i++ )
		mwpc_hit_axis[i]->Reset("ICESM");
	
	mwpc_pos->Reset("ICESM");
	elum_E->Reset("ICESM");
	elum_E_vs_sec->Reset("ICESM");
	zd_EdE->Reset("ICESM");
	gamma_E->Reset("ICESM");
	gamma_E_vs_det->Reset("ICESM");
	gamma_gamma_E->Reset("ICESM");
	gamma_gamma_td->Reset("ICESM");

	for( unsigned int i = 0; i < lume_E.size(); i++ )
		lume_E[i]->Reset("ICESM");

	for( unsigned int i = 0; i < lume_E_vs_x.size(); i++ )
		lume_E_vs_x[i]->Reset("ICESM");

	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); i++ ){
		fpga_td[i]->Reset("ICESM");
		asic_td[i]->Reset("ICESM");
		fpga_pulser_loss[i]->Reset("ICESM");
		fpga_period[i]->Reset("ICESM");
		fpga_sync[i]->Reset("ICESM");
		asic_pulser_loss[i]->Reset("ICESM");
		asic_period[i]->Reset("ICESM");
		asic_sync[i]->Reset("ICESM");
	}

	tdiff->Reset("ICESM");
	tdiff_clean->Reset("ICESM");
	caen_period->Reset("ICESM");
	ebis_period->Reset("ICESM");
	t1_period->Reset("ICESM");
	sc_period->Reset("ICESM");
	laser_period->Reset("ICESM");
	supercycle->Reset("ICESM");

	return;

}

