#include "EventBuilder.hh"
///////////////////////////////////////////////////////////////////////////////
/// This constructs the event-builder object
ISSEventBuilder::ISSEventBuilder(){

	// No settings file by default
	overwrite_set = false;

	// No calibration file by default
	overwrite_cal = false;

	// No input file at the start by default
	flag_input_file = false;

	// No progress bar by default
	_prog_ = false;

	// Histogrammer options
	//TH1::AddDirectory(kFALSE);

}

///////////////////////////////////////////////////////////////////////////////
/// This maps the vectors for the array just once, assuming you dont change array geomerty between run files!
void ISSEventBuilder::ArrayMapping(){

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

	// ------------------------------------------------------------------------ //
	// Initialise variables and flags
	// ------------------------------------------------------------------------ //
	build_window = set->GetEventWindow();

	// Resize 17 vectors to match modules of detectors
	n_mesy_pulser.resize( set->GetNumberOfMesytecModules(), 0 );
	n_fpga_pulser.resize( set->GetNumberOfArrayModules(), 0 );
	n_asic_pulser.resize( set->GetNumberOfArrayModules(), 0 );
	n_asic_pause.resize( set->GetNumberOfArrayModules(), 0 );
	n_asic_resume.resize( set->GetNumberOfArrayModules(), 0 );
	flag_pause.resize( set->GetNumberOfArrayModules(), false );
	flag_resume.resize( set->GetNumberOfArrayModules(), false );
	pause_time.resize( set->GetNumberOfArrayModules(), 0 );
	resume_time.resize( set->GetNumberOfArrayModules(), 0 );
	asic_dead_time.resize( set->GetNumberOfArrayModules(), 0 );
	asic_time_start.resize( set->GetNumberOfArrayModules(), 0 );
	asic_time_stop.resize( set->GetNumberOfArrayModules(), 0 );
	asic_time.resize( set->GetNumberOfArrayModules(), 0 );
	asic_prev.resize( set->GetNumberOfArrayModules(), 0 );
	fpga_time.resize( set->GetNumberOfArrayModules(), 0 );
	fpga_prev.resize( set->GetNumberOfArrayModules(), 0 );
	mesy_time.resize( set->GetNumberOfMesytecModules(), 0 );
	mesy_prev.resize( set->GetNumberOfMesytecModules(), 0 );
	vme_time_start.resize( set->GetNumberOfVmeCrates() );
	vme_time_stop.resize( set->GetNumberOfVmeCrates() );

	for( unsigned int i = 0; i < set->GetNumberOfVmeCrates(); ++i ){

		vme_time_start[i].resize( set->GetMaximumNumberOfVmeModules(), 0 );
		vme_time_stop[i].resize( set->GetMaximumNumberOfVmeModules(), 0 );

	}

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
	cd_ctr		= 0;

	// Some flags must be false to start
	flag_caen_pulser = false;

}

/////////////////////
/// Initialisation of the process with first file or things
void ISSEventBuilder::ConfigureInput() {

	// Read settings from file
	if( !overwrite_set ) {

		if( input_file->GetListOfKeys()->Contains( "Settings" ) )
			set = std::make_shared<ISSSettings>( (ISSSettings*)input_file->Get( "Settings" ) );
		else
			set = std::make_shared<ISSSettings>();

	}

	// Read calibration from the file
	if( !overwrite_cal ) {

		if( input_file->GetListOfKeys()->Contains( "Calibration" ) )
			cal = std::make_shared<ISSCalibration>( (ISSCalibration*)input_file->Get( "Calibration" ) );
		else
			cal = std::make_shared<ISSCalibration>();
		cal->AddSettings( set );

	}

	// Do the array mapping just once after settings
	ArrayMapping();

	// Setup counters
	StartFile();

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

	return;

}

////////////////////////////////////////////////////////////////////////////////
/// Sets the private member input_tree to the parameter user_tree, sets the branch address and calls the ISSEventBuilder::StartFile function
/// \param [in] user_tree The name of the tree in the ROOT file containing the time-sorted events
void ISSEventBuilder::SetInputTree( TTree *user_tree ){

	// Find the tree and set branch addresses
	input_tree = user_tree;
	input_tree->SetBranchAddress( "data", &in_data );

	// Configure the input files
	ConfigureInput();

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

	return;

}

////////////////////////////////////////////////////////////////////////////////
/// Sets the nptool_tree to the parameter user_tree, sets the branch address
/// \param [in] user_tree The name of the tree in the ROOT file containing the NPTool simulation events
void ISSEventBuilder::SetNPToolTree( TTree *user_tree ){

	// Find the tree and set branch addresses
	nptool_tree = user_tree;
	nptool_tree->SetBranchAddress( "ISS", &sim_data );

	// Configure the input files
	ConfigureInput();

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

	// --------------------------------------------------------- //
	// Create output file and create events tree
	// --------------------------------------------------------- //
	output_file = new TFile( output_file_name.data(), "recreate" );
	output_tree = new TTree( "evt_tree", "evt_tree" );
	output_tree->Branch( "ISSEvts", write_evts.get() );
	output_tree->SetAutoFlush();

	// Create log file.
	std::string log_file_name = output_file_name.substr( 0, output_file_name.find_last_of(".") );
	log_file_name += ".log";
	log_file.open( log_file_name.data(), std::ios::app );

	// Histograms in separate function
	MakeHists();

	// Write once
	output_file->Write();

}

////////////////////////////////////////////////////////////////////////////////
/// Clears the vectors that store energies, time differences, ids, module numbers, row numbers, recoil sectors etc. Also resets flags that are relevant for building events
void ISSEventBuilder::Initialise(){

	/// This is called at the end of every execution/loop

	flag_close_event = false;
	event_open = false;

	hit_ctr = 0;

	// Swap all vectors with empty vectors
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

	std::vector<float>().swap(mwpctac_list);
	std::vector<unsigned short>().swap(mwpctac_raw_list);
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

	cdren_list.resize( set->GetNumberOfCDLayers() );
	cdrtd_list.resize( set->GetNumberOfCDLayers() );
	cdrid_list.resize( set->GetNumberOfCDLayers() );
	cdsen_list.resize( set->GetNumberOfCDLayers() );
	cdstd_list.resize( set->GetNumberOfCDLayers() );
	cdsid_list.resize( set->GetNumberOfCDLayers() );
	for( unsigned int i = 0; i < set->GetNumberOfCDLayers(); ++i ){

		std::vector<float>().swap( cdren_list[i] );
		std::vector<double>().swap( cdrtd_list[i] );
		std::vector<char>().swap( cdrid_list[i] );
		std::vector<float>().swap( cdsen_list[i] );
		std::vector<double>().swap( cdstd_list[i] );
		std::vector<char>().swap( cdsid_list[i] );

	}

	std::vector<std::shared_ptr<ISSCDEvt>>().swap( cd_evt );
	write_evts->ClearEvt();

	return;

}

////////////////////////////////////////////////////////////////////////////////
/// This loops over all events found in the input file and wraps them up and stores them in the output file
/// \return The number of entries in the tree that have been sorted (=0 if there is an error)
unsigned long ISSEventBuilder::BuildEvents() {

	/// Function to loop over the sort tree and build array and recoil events

	// Load the full tree if possible
	// output_tree->SetMaxVirtualSize(1e9); // 1 GB
	// input_tree->SetMaxVirtualSize(1e9); // 1 GB
	// input_tree->LoadBaskets(1e9); // Load 1 GB of data to memory

	if( input_tree->LoadTree(0) < 0 ){

		std::cout << " Event Building: nothing to do" << std::endl;
		return 0;

	}

	// Get ready and go
	Initialise();
	n_entries = input_tree->GetEntries();

	std::cout << " Event Building: number of entries in input tree = ";
	std::cout << n_entries << std::endl;

	// Get the index needed for time-ordering
	TTreeIndex *att_index = nullptr;

	// Event building by timestamp only
	if( set->BuildByTimeStamp() ) {
		std::cout << " Event Building: using raw timestamp for event ordering" << std::endl;
		//input_tree->BuildIndex( "GetTimeStamp()" );
	}

	// Or apply time-walk correction, i.e. get new time ordering
	else {
		std::cout << " Event Building: applying time walk-correction to event ordering" << std::endl;
		input_tree->BuildIndex( "GetTimeWithWalk()" );
		att_index = (TTreeIndex*)input_tree->GetTreeIndex();
	}


	// ------------------------------------------------------------------------ //
	// Main loop over TTree to find events
	// ------------------------------------------------------------------------ //
	for( unsigned long i = 0; i < n_entries; ++i ) {

		// Get time-ordered event index (with or without walk correction)
		unsigned long long idx = i;
		if( !set->BuildByTimeStamp() )
			idx = att_index->GetIndex()[i];

		// Current event data
		//if( input_tree->MemoryFull(30e6) )
		//	input_tree->DropBaskets();

		if( i == 0 ) input_tree->GetEntry(idx);

		// Get the time of the event (with or without walk correction)
		if( set->BuildByTimeStamp() ) mytime = in_data->GetTime(); // no correction
		else mytime = in_data->GetTimeWithWalk(); // with correction

		//std::cout << std::setprecision(15) << i << "\t";
		//std::cout << in_data->GetTimeStamp() << "\t" << mytime << std::endl;

		// check time stamp monotonically increases!
		// but allow for the fine time of the CAEN system
		if( (unsigned long long)time_prev > mytime + 5.0 ) {

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

				mystrip = array_pid.at( myasic ).at( mych );

				// Only use if it is an event from a detector
				if( mystrip >= 0 ) {

					pen_list.push_back( myenergy );
					ptd_list.push_back( mytime );
					if( set->BuildByTimeStamp() )
						pwalk_list.push_back( mytime + mywalk );
					else
						pwalk_list.push_back( mytime );
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

				mystrip = array_nid.at( asic_data->GetAsic() ).at( asic_data->GetChannel() );

				// Only use if it is an event from a detector
				if( mystrip >= 0 ) {

					nen_list.push_back( myenergy );
                    ntd_list.push_back( mytime );
					if( set->BuildByTimeStamp() )
						nwalk_list.push_back( mytime + mywalk );
					else
						nwalk_list.push_back( mytime );
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

			// Check for clipped flags
			myclipped = vme_data->IsClipped();

			// New calibration supplied
			if( overwrite_cal ) {

				std::string entype = cal->VmeType( myvme, mymod, mych );
				unsigned short adc_value = 0;
				if( entype == "Qlong" ) {
					adc_value = vme_data->GetQlong();
				}
				else if( entype == "Qshort" ){
					adc_value = vme_data->GetQshort();
				}
				else if( entype == "Qdiff" ){
					adc_value = vme_data->GetQdiff();
				}
				myenergy = cal->VmeEnergy( myvme, mymod, mych, adc_value );

				if( adc_value < cal->VmeThreshold( myvme, mymod, mych ) )
					mythres = false;

			}

			else {

				myenergy = vme_data->GetEnergy();
				mythres = vme_data->IsOverThreshold();

			}

			// Check for overflows
			std::string entype = cal->VmeType( myvme, mymod, mych );
			if( entype == "Qlong" ) {
				myoverflow = vme_data->IsOverflowLong();
			}
			else if( entype == "Qshort" ){
				myoverflow = vme_data->IsOverflowShort();
			}
			else if( entype == "Qdiff" ){
				myoverflow = vme_data->IsOverflowLong();
				myoverflow |= vme_data->IsOverflowShort();
			}

			//std::cout << "Recoil: " << set->IsRecoil( myvme, mymod, mych ) << std::endl;
			//std::cout << "MWPC: " << set->IsMWPC( myvme, mymod, mych ) << std::endl;
			//std::cout << "ELUM: " << set->IsELUM( myvme, mymod, mych ) << std::endl;
			//std::cout << "ZD: " << set->IsZD( myvme, mymod, mych ) << std::endl;
			//std::cout << "ScintArray: " << set->IsScintArray( myvme, mymod, mych ) << std::endl;
			//std::cout << "LUME: " << set->IsLUME( myvme, mymod, mych ) << std::endl;
			//std::cout << "CD: " << set->IsCD( myvme, mymod, mych ) << std::endl;


			// If it's below threshold do not use as window opener
			if( mythres ) event_open = true;

			// DETERMINE WHICH TYPE OF VME EVENT THIS IS
			// Is it a recoil?
			if( set->IsRecoil( myvme, mymod, mych ) &&
			   mythres &&											// check threshold
			   ( !myclipped || !set->GetClippedRejection() ) &&		// check clipped
			   ( !myoverflow || !set->GetOverflowRejection() ) ) {	// check overflow

				mysector = set->GetRecoilSector( myvme, mymod, mych );
				mylayer = set->GetRecoilLayer( myvme, mymod, mych );

				ren_list.push_back( myenergy );
				rtd_list.push_back( mytime );
				rid_list.push_back( mylayer );
				rsec_list.push_back( mysector );

				hit_ctr++; // increase counter for bits of data included in this event

			}

			// Is it an MWPC?
			else if( set->IsMWPC( myvme, mymod, mych ) &&
					mythres &&											// check threshold
					( !myclipped || !set->GetClippedRejection() ) &&	// check clipped
					( !myoverflow || !set->GetOverflowRejection() ) ) {	// check overflow

				mwpctac_list.push_back( myenergy );
				mwpctac_raw_list.push_back( vme_data->GetQdiff() );
				mwpctd_list.push_back( mytime );
				mwpcaxis_list.push_back( set->GetMWPCAxis( myvme, mymod, mych ) );
				mwpcid_list.push_back( set->GetMWPCID( myvme, mymod, mych ) );

				hit_ctr++; // increase counter for bits of data included in this event

			}

			// Is it an ELUM?
			else if( set->IsELUM( myvme, mymod, mych ) &&
					mythres &&											// check threshold
					( !myclipped || !set->GetClippedRejection() ) &&	// check clipped
					( !myoverflow || !set->GetOverflowRejection() ) ) {	// check overflow

				mysector = set->GetELUMSector( myvme, mymod, mych );

				een_list.push_back( myenergy );
				etd_list.push_back( mytime );
				esec_list.push_back( mysector );

				hit_ctr++; // increase counter for bits of data included in this event

			}

			// Is it a ZeroDegree?
			else if( set->IsZD( myvme, mymod, mych ) &&
					mythres &&											// check threshold
					( !myclipped || !set->GetClippedRejection() ) &&	// check clipped
					( !myoverflow || !set->GetOverflowRejection() ) ) {	// check overflow

				mylayer = set->GetZDLayer( myvme, mymod, mych );

				zen_list.push_back( myenergy );
				ztd_list.push_back( mytime );
				zid_list.push_back( mylayer );

				hit_ctr++; // increase counter for bits of data included in this event

			}

			// Is it a ScintArray?
			else if( set->IsScintArray( myvme, mymod, mych ) &&
					mythres &&											// check threshold
					( !myclipped || !set->GetClippedRejection() ) &&	// check clipped
					( !myoverflow || !set->GetOverflowRejection() ) ) {	// check overflow

				myid = set->GetScintArrayDetector( myvme, mymod, mych );

				saen_list.push_back( myenergy );
				satd_list.push_back( mytime );
				said_list.push_back( myid );

				hit_ctr++; // increase counter for bits of data included in this event

			}

			// Is it a LUME?
			else if( set->IsLUME( myvme, mymod, mych ) &&
					mythres &&											// check threshold
					( !myclipped || !set->GetClippedRejection() ) &&	// check clipped
					( !myoverflow || !set->GetOverflowRejection() ) ) {	// check overflow

				// Get LUME signal type (0 = total energy, 1 = near side, 2 = far side)
				mytype = set->GetLUMEType( myvme, mymod, mych );
				myid = set->GetLUMEDetector( myvme, mymod, mych );

				switch( mytype ) {
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

			// Is it a CD event?
			else if( set->IsCD( myvme, mymod, mych ) &&
					mythres &&											// check threshold
					( !myclipped || !set->GetClippedRejection() ) &&	// check clipped
					( !myoverflow || !set->GetOverflowRejection() ) ) {	// check overflow

				mylayer = set->GetCDLayer( myvme, mymod, mych );
				myring = set->GetCDRing( myvme, mymod, mych );
				mysector = set->GetCDSector( myvme, mymod, mych );

				// Ring side
				if( myring >= 0 && mylayer >= 0 && mylayer < set->GetNumberOfCDLayers() ) {

					cdren_list[mylayer].push_back( myenergy );
					cdrtd_list[mylayer].push_back( mytime );
					cdrid_list[mylayer].push_back( myring );

					hit_ctr++; // increase counter for bits of data included in this event

				}

				// Sector side
				if( mysector >= 0 && mylayer >= 0 && mylayer < set->GetNumberOfCDLayers() ) {

					cdsen_list[mylayer].push_back( myenergy );
					cdstd_list[mylayer].push_back( mytime );
					cdsid_list[mylayer].push_back( mysector );

					hit_ctr++; // increase counter for bits of data included in this event

				}

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

			// Update Mesytec pulser time
			else if( info_data->GetCode() == set->GetMesytecPulserCode() ) {

				mesy_time[info_data->GetModule()] = info_data->GetTime();
				info_tdiff = mesy_time[info_data->GetModule()] - mesy_prev[info_data->GetModule()];

				if( mesy_prev[info_data->GetModule()] != 0 )
					mesy_period[info_data->GetModule()]->Fill( info_tdiff );

				n_mesy_pulser[info_data->GetModule()]++;

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

				// Time difference for array
				for( unsigned int j = 0; j < set->GetNumberOfArrayModules(); ++j ) {

					double fpga_tdiff = caen_time - fpga_time[j];
					double asic_tdiff = caen_time - asic_time[j];

					// If diff is greater than 50 us, we have the wrong pair
					if( fpga_tdiff > 5e4 ) fpga_tdiff = caen_prev - fpga_time[j];
					else if( fpga_tdiff < -5e4 ) fpga_tdiff = caen_time - fpga_prev[j];
					if( asic_tdiff > 5e4 ) asic_tdiff = caen_prev - asic_time[j];
					else if( asic_tdiff < -5e4 ) asic_tdiff = caen_time - asic_prev[j];

					// ??? Could be the case that |fpga_tdiff| > 5e6 after these conditional statements...change to while loop? Or have an extra condition?

					fpga_td[j]->Fill( fpga_tdiff );
					fpga_sync[j]->Fill( fpga_time[j], fpga_tdiff );
					fpga_pulser_loss[j]->Fill( fpga_time[j], (int)n_fpga_pulser[j] - (int)n_caen_pulser );

					asic_td[j]->Fill( asic_tdiff );
					asic_sync[j]->Fill( asic_time[j], asic_tdiff );
					asic_pulser_loss[j]->Fill( asic_time[j], (int)n_asic_pulser[j] - (int)n_caen_pulser );

				}

				// Time differnce for Mesytec modules
				for( unsigned int j = 0; j < set->GetNumberOfMesytecModules(); ++j ) {

					double mesy_tdiff = caen_time - mesy_time[j];
					//std::cout << j << ": " << caen_time << " - " << mesy_time[j];
					//std::cout << " = " << mesy_tdiff << std::endl;

					// If diff is greater than 50 us, we have the wrong pair
					if( mesy_tdiff > 5e4 ) mesy_tdiff = caen_prev - mesy_time[j];
					else if( mesy_tdiff < -5e4 ) mesy_tdiff = caen_time - mesy_prev[j];

					mesy_td[j]->Fill( mesy_tdiff );
					mesy_sync[j]->Fill( mesy_time[j], mesy_tdiff );
					mesy_pulser_loss[j]->Fill( mesy_time[j], (int)n_mesy_pulser[j] - (int)n_caen_pulser );

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
			if( info_data->GetCode() == set->GetMesytecPulserCode() )
				mesy_prev[info_data->GetModule()] = mesy_time[info_data->GetModule()];


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
		unsigned long long idx_next;
		if( i+1 == n_entries ) idx_next = n_entries;
		else {
			idx_next = i+1;
			if( !set->BuildByTimeStamp() )
				idx_next = att_index->GetIndex()[i+1];
		}

		if( input_tree->GetEntry(idx_next) ) {

			// Time difference to next event (with or without time walk correction)
			if( set->BuildByTimeStamp() )
				time_diff = in_data->GetTime() - time_first; // no correction
			else
				time_diff = in_data->GetTimeWithWalk() - time_first; // with correction

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
				LumeFinder();		// add a LumeEvt for each LUME
				CdFinder();			// add a CDEvt for CD

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
				   write_evts->GetGammaRayMultiplicity() ||
				   write_evts->GetLumeMultiplicity() ||
				   write_evts->GetCDMultiplicity() )
					output_tree->Fill();

				// Clean up if the next event is going to make the tree full
				//if( output_tree->MemoryFull(30e6) )
				//	output_tree->DropBaskets();
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
	ss_log << "   CD events = " << cd_ctr << std::endl;
	ss_log << "   CAEN pulser = " << n_caen_pulser << std::endl;
	ss_log << "   Mesytec pulser" << std::endl;
	for( unsigned int i = 0; i < set->GetNumberOfMesytecModules(); ++i )
		ss_log << "    Module " << i << " = " << n_mesy_pulser[i] << std::endl;
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
			recoil_evt->SetRing( 0 );
			recoil_evt->AddFragment( ren_list[i], rid_list[i] );

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
						recoil_evt->AddFragment( ren_list[j], rid_list[j] );

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
	int tac_diff_raw;
	float tac_diff_mm;

	// Loop over MWPC events
	for( unsigned int i = 0; i < mwpctac_list.size(); ++i ) {

		// TAC singles spectra
		mwpc_tac_axis[mwpcaxis_list[i]][mwpcid_list[i]]->Fill( mwpctac_list[i] );

		// Get first TAC of the axis
		if( mwpcid_list[i] == 0 ){

			// Look for matching pair
			for( unsigned int j = 0; j < mwpctac_list.size(); ++j ) {

				// Skip itself
				if( i != j ) continue;

				// Check if we already used this hit
				if( std::find( index.begin(), index.end(), j ) != index.end() )
					continue;

				// Found a match
				if( mwpcid_list[j] == 1 && mwpcaxis_list[i] == mwpcaxis_list[j] ){

					index.push_back(j);
					tac_diff_mm = (int)mwpctac_list[i] - (int)mwpctac_list[j];
					tac_diff_raw = (int)mwpctac_raw_list[i] - (int)mwpctac_raw_list[j];
					mwpc_evt->SetEvent( tac_diff_raw, tac_diff_mm, mwpcaxis_list[i], mwpctd_list[i] );

					// MWPC profiles, i.e TAC difference spectra
					mwpc_hit_axis[mwpcaxis_list[i]]->Fill( tac_diff_raw );

					// Write event to tree
					write_evts->AddEvt( mwpc_evt );
					mwpc_ctr++;

					// Only make one TAC event for a given pair
					break;

				}

			}

		}

	}

	// If we have a 2 axis system, do an x-y plot
	if( write_evts->GetMwpcMultiplicity() == 2 ) {

		// Skip if they are in the same axis, weird events
		if( write_evts->GetMwpcEvt(0)->GetAxis() != write_evts->GetMwpcEvt(1)->GetAxis() ) {

			// Check the orientation
			if( write_evts->GetMwpcEvt(0)->GetAxis() == 1 ) {

				mwpc_pos_raw->Fill( write_evts->GetMwpcEvt(0)->GetTacDiff(),
							   write_evts->GetMwpcEvt(1)->GetTacDiff() );

				mwpc_pos_mm->Fill( write_evts->GetMwpcEvt(0)->GetPosition(),
							   write_evts->GetMwpcEvt(1)->GetPosition() );

			}

			else {

				mwpc_pos_raw->Fill( write_evts->GetMwpcEvt(1)->GetTacDiff(),
							   write_evts->GetMwpcEvt(0)->GetTacDiff() );

				mwpc_pos_mm->Fill( write_evts->GetMwpcEvt(1)->GetPosition(),
							   write_evts->GetMwpcEvt(0)->GetPosition() );

			} // orientation

		} // not same axis

	} // multiplicity 2

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
			zd_evt->SetRing( 0 ); // always 0 ZeroDegree
			zd_evt->AddFragment( zen_list[i], 0 );

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
					zd_evt->AddFragment( zen_list[j], zid_list[j] );
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
							0, 0, satd_list[i] );

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

		ne_energy = has_ln ? ne_energy : 0;
		fe_energy = has_lf ? fe_energy : 0;

		lume_evt->SetEvent(be_energy, be_id, be_timestamp, ne_energy, fe_energy);

		if (has_ln || has_lf){
			double x = ( fe_energy - ne_energy ) / ( ne_energy + fe_energy );
			lume_evt->SetX(x);
			lume_E_vs_x[be_id]->Fill(x, be_energy);
			lume_ne_vs_fe[be_id]->Fill(fe_energy, ne_energy);
		}

		write_evts->AddEvt( lume_evt );
		lume_ctr++;
	}

	return;
}

////////////////////////////////////////////////////////////////////////////////
/// This function takes a series of E and dE signals on the silicon CD (fission fragments) detector and determines what hits to keep
/// from these using sensible conditions including a prompt coincidence window.
/// Signals are triggered by the dE detector, but if a corresponding E signal is not found, then a hit at E = 0 is still recorded.
/// We can try to build a number of particles per event by comparing energies, times and sector IDs.
/// This is difficult, but maybe by going through this excercise we can improve how things are done for recoils?
void ISSEventBuilder::CdFinder() {

	//std::cout << __PRETTY_FUNCTION__ << std::endl;

	// Quick check that we have a CD at all
	if( set->GetNumberOfCDLayers() == 0 ) return;

	// Multiplicity, time and energy matrices
	for( unsigned int i = 0; i < set->GetNumberOfCDLayers(); ++i ) {

		// Multiplicity
		if( cdren_list[i].size() || cdsen_list[i].size() )
			cd_rs_mult[i]->Fill( cdren_list[i].size(), cdsen_list[i].size() );

		// Plot each sector energy against each ring energy
		// and ring/sector ID against energy
		for( unsigned int j = 0; j < cdsen_list[i].size(); j++ ) {

		    cd_s_en[i]->Fill( cdsid_list[i][j], cdsen_list[i][j] );

			  for( unsigned int k = 0; k < cdren_list[i].size(); k++ ) {

				cd_rs_en[i]->Fill( cdren_list[i][k], cdsen_list[i][j] );
				cd_rs_td[i]->Fill( cdrtd_list[i][k] - cdstd_list[i][j] );

			} // k - rings

		} // j - sectors

		// Do not double count ring ID vs energy events
		for( unsigned int k = 0; k < cdren_list[i].size(); k++ ) {

			cd_r_en[i]->Fill( cdrid_list[i][k], cdren_list[i][k]);

		} // k - rings

	} // i - layer

	// Look for the dE signal as a trigger
	unsigned char dE_idx = set->GetCDEnergyLossStart();

	// If we never found a sector in the dE at all, there's no "trigger"
	if( cdsen_list[dE_idx].size() == 0 ) return;

	// If we have at least one sector and at least one ring, we have at least one event!
	if( cdsen_list[dE_idx].size() > 0 && cdren_list[dE_idx].size() > 0 ){

		// Keep track of which events we've used
		std::vector<unsigned int> used_idx;

		// Check all combinations for charge-sharing
		for( unsigned int j = 0; j < cdsen_list[dE_idx].size(); j++ ) {

			// Check if it's been used already
			bool skip_flag = false;
			if( std::find( used_idx.begin(), used_idx.end(), j ) != used_idx.end() )
				continue;

			// Mark this hit as used
			used_idx.push_back(j);

			// Check all other sector hits to find a neighbour
			for( unsigned int k = j+1; k < cdsen_list[dE_idx].size(); k++ ) {

				// Check if it's been used already
				if( std::find( used_idx.begin(), used_idx.end(), k ) != used_idx.end() )
					continue;

				// Check if they are really neighbours
				float	sec_en_diff = cdsen_list[dE_idx][j] - cdsen_list[dE_idx][k];
				int		sec_id_diff = cdsid_list[dE_idx][j] - cdsid_list[dE_idx][k];
				double	sec_td_diff = cdstd_list[dE_idx][j] - cdstd_list[dE_idx][k];
				if( ( TMath::Abs( sec_id_diff ) == 1 ||
					  TMath::Abs( sec_id_diff ) == set->GetNumberOfCDSectors()-1 ) &&
				     TMath::Abs( sec_td_diff ) < set->GetCDRSHitWindow() ) {

					// Charge-sharing add-back, use highest energy for sector ID and time
					float sumen = cdsen_list[dE_idx][j] + cdsen_list[dE_idx][k];
					int secid_tmp = cdsid_list[dE_idx][j];
					double sectd_tmp = cdstd_list[dE_idx][j];
					if( sec_en_diff < 0 ) {
						secid_tmp = cdsid_list[dE_idx][k];
						sectd_tmp = cdstd_list[dE_idx][k];
					}

					// Mark hit as used
					used_idx.push_back(k);

					// Make a particle event in the CD
					cd_evt.push_back( std::make_shared<ISSCDEvt>() );

					// Add the first fragment and set the ring and sector
					cd_evt.back()->AddFragment( sumen, dE_idx );
					cd_evt.back()->SetSector( secid_tmp );
					cd_evt.back()->SetdETime( sectd_tmp );

					// Finished with this, go back to the j loop
					skip_flag = true;
					break;

				} // neighbours, do charge-sharing addback

			} // all other sector hits - k

			// Check if we used this in an add-back event
			if( skip_flag ) continue;

			// We only get here if we didn't find a neighbour
			// So let's make a single event out of this
			cd_evt.push_back( std::make_shared<ISSCDEvt>() );
			cd_evt.back()->AddFragment( cdsen_list[dE_idx][j], dE_idx );
			cd_evt.back()->SetSector( cdsid_list[dE_idx][j] );
			cd_evt.back()->SetdETime( cdstd_list[dE_idx][j] );

		} // j

	} // Multiple sectors and at least 1 ring
	// Now we've finished finding particles in the dE layer of the CD

	// Find the matching rings for each particle based on time difference
	// We can re-use the same ring for each particle, because they are 360˚ and
	// we might be unlucky enough for both fragments to pileup in the same ring.
	std::vector<unsigned int> used_idx;
	for( unsigned int partid = 0; partid < cd_evt.size(); ++partid ) {

		// Set the dE and log for finding the ring
		double sectime_cur = cd_evt[partid]->GetdETime();
		double secen_cur = cd_evt[partid]->GetEnergy(0);

		// Find matching ring, easy if there is only 1
		int rindex_match = -1;
		int rindex_reuse = -1;
		double rs_ediff_best = 8e12;

		// Search for the best match based on energy
		for( unsigned int j = 0; j < cdrtd_list[dE_idx].size(); ++j ) {

			bool ring_used = false;
			if( std::find( used_idx.begin(), used_idx.end(), j ) != used_idx.end() )
				ring_used = true;

			double rs_tdiff_cur = cdrtd_list[dE_idx][j] - sectime_cur;
			double rs_ediff_cur = cdren_list[dE_idx][j] - secen_cur;
			rs_tdiff_cur = TMath::Abs( rs_tdiff_cur );
			rs_ediff_cur = TMath::Abs( rs_ediff_cur );
			if( rs_ediff_cur < rs_ediff_best &&
			   rs_tdiff_cur < set->GetCDRSHitWindow() ) {

				// only update if the ring hasn't already been used
				if( !ring_used ) {

					rindex_match = j;
					rs_ediff_best = rs_ediff_cur;

				}

				// otherwise add it to a list we might need to reuse later
				else rindex_reuse = j;

			} // better match found

		} // loop over all ring hits - j

		// Set the ring for the dE layer
		if( rs_ediff_best < 9e12 && rindex_match >= 0 ) {
			cd_evt[partid]->SetRing( cdrid_list[dE_idx][rindex_match] );
			used_idx.push_back( rindex_match );
		}
		else if( partid > 0 && rindex_reuse >= 0 )
			cd_evt[partid]->SetRing( cdrid_list[dE_idx][rindex_reuse] );
		else
			cd_evt[partid]->SetRing( 0xff ); // no ring found

	} // partid - finished matching rings

	// ------------------------------------------------------------------------------//
	// Now we need to loop over every other layer and find a match for each particle //
	// ------------------------------------------------------------------------------//
	for( unsigned int i = 0; i < set->GetNumberOfCDLayers(); ++i ) {

		// Don't do the dE layer again though
		if( i == dE_idx ) continue;

		// Mark up those that are used already
		std::vector<unsigned int> used_idx;

		// Check every particle for a match in the other layers
		for( unsigned int partid = 0; partid < cd_evt.size(); ++partid ) {

			//std::cout << "particle " << partid << " has " << (int)cdsen_list[i].size();
			//std::cout << " coincident hits and finds a match with id = ";

			// Find the matching sector in the available list
			for( unsigned int j = 0; j < cdsen_list[i].size(); ++j ) {

				// General time difference, without sector matching
				double layer_td_diff = cd_evt[partid]->GetTime() - cdstd_list[i][j];
				cd_id_td->Fill( i, layer_td_diff );

				// Check for time coincidence
				if( TMath::Abs( layer_td_diff ) > set->GetCDDDHitWindow() )
					continue;

				// Particles must be in the same sector ± 2
				int layer_id_diff = cdsid_list[i][j] - cd_evt[partid]->GetSector();
				if( TMath::Abs( layer_id_diff ) != 0 &&
				   TMath::Abs( layer_id_diff ) != 1 &&
				   TMath::Abs( layer_id_diff ) != 2 &&
				   TMath::Abs( layer_id_diff ) != set->GetNumberOfCDSectors()-1 &&
				   TMath::Abs( layer_id_diff ) != set->GetNumberOfCDSectors()-2 )
					continue;


				// Check if it's been used already
				bool skip_flag = false;
				if( std::find( used_idx.begin(), used_idx.end(), j ) != used_idx.end() )
					continue;

				// Check all other sector hits to find a neighbour
				for( unsigned int k = j+1; k < cdsen_list[i].size(); k++ ) {

					// Check if it's been used already
					skip_flag = false;
					if( std::find( used_idx.begin(), used_idx.end(), k ) != used_idx.end() ) {
						skip_flag = true;
						continue;
					}

					// Check if they are really neighbours
					int		sec_id_diff = cdsid_list[i][j] - cdsid_list[i][k];
					double	sec_td_diff = cdstd_list[i][j] - cdstd_list[i][k];
					if( ( TMath::Abs( sec_id_diff ) == 1 ||
						 TMath::Abs( sec_id_diff ) == set->GetNumberOfCDSectors()-1 ) &&
					   TMath::Abs( sec_td_diff ) < set->GetCDRSHitWindow() ) {

						// Charge-sharing add-back
						float sumen = cdsen_list[i][j] + cdsen_list[i][k];

						// Mark hit as used
						used_idx.push_back(k);

						// Add fragment for this particle and set the E time if needed
						cd_evt[partid]->AddFragment( sumen, i );
						if( i == set->GetCDEnergyRestStart() )
							cd_evt[partid]->SetETime( cdstd_list[i][j] );
						//std::cout << j << " + " << k;

						// Finished with this, go back to the j loop
						skip_flag = true;
						break;

					} // neighbours, do charge-sharing addback

				} // all other sector hits - k

				// Check if we did charge-sharing
				if( skip_flag ) continue;

				// Add fragment for event 1 and set the E time if needed
				cd_evt[partid]->AddFragment( cdsen_list[i][j], i );
				if( i == set->GetCDEnergyRestStart() )
					cd_evt[partid]->SetETime( cdstd_list[i][j] );
				//std::cout << j;

				// Move the next particle and mark as used
				used_idx.push_back(j);
				break;

			} // all other sector hits - j

			//std::cout << std::endl;

		} // all other particles - partid

	} // all other layers - i

	// Histogram the CD fragments and write the event to the tree
	for( unsigned int partid = 0; partid < cd_evt.size(); ++partid ) {

		cd_EdE->Fill( cd_evt[partid]->GetEnergyRest( set->GetCDEnergyRestStart(), set->GetCDEnergyRestStop() ),
					  cd_evt[partid]->GetEnergyLoss( set->GetCDEnergyLossStart(), set->GetCDEnergyLossStop() ) );
		cd_dEsum->Fill( cd_evt[partid]->GetEnergyTotal( set->GetCDEnergyTotalStart(), set->GetCDEnergyTotalStop() ),
					    cd_evt[partid]->GetEnergyLoss( set->GetCDEnergyLossStart(), set->GetCDEnergyLossStop() ) );
		write_evts->AddEvt( cd_evt[partid] );
		cd_ctr++;

	} // partid

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

	mesy_td.resize( set->GetNumberOfMesytecModules() );
	mesy_period.resize( set->GetNumberOfMesytecModules() );
	mesy_sync.resize( set->GetNumberOfMesytecModules() );
	mesy_pulser_loss.resize( set->GetNumberOfMesytecModules() );

	// Loop over Mesytec modules
	for( unsigned int i = 0; i < set->GetNumberOfMesytecModules(); ++i ) {

		hname = "mesy_td_" + std::to_string(i);
		htitle = "Time difference between Mesytec and CAEN pulser events in module ";
		htitle += std::to_string(i) + ";#Delta t [ns]";
		mesy_td[i] = new TH1F( hname.data(), htitle.data(), 1.6e3 , -4e3, 4e3 );

		hname = "mesy_period_" + std::to_string(i);
		htitle = "Period of pulser in Mesytec DAQ in module ";
		htitle += std::to_string(i) + ";time [ns];f [Hz]";
		mesy_period[i] = new TH1F( hname.data(), htitle.data(), 1000, 0, 1e9 );

		hname = "mesy_sync_" + std::to_string(i);
		htitle = "Time difference between Mesytec and CAEN events as a function of time in module ";
		htitle += std::to_string(i) + ";time [ns];#Delta t [ns]";
		mesy_sync[i] = new TProfile( hname.data(), htitle.data(), 10.8e4, 0, 10.8e12 );

		hname = "mesy_pulser_loss_" + std::to_string(i);
		htitle = "Number of missing/extra pulser events in Mesytec as a function of time in module ";
		htitle += std::to_string(i) + ";#time [ns];(-ive CAEN missing, +ive Mesytec missing)";
		mesy_pulser_loss[i] = new TProfile( hname.data(), htitle.data(), 10.8e4, 0, 10.8e12 );

	}

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
			pn_11[i][j] = new TH2F( hname.data(), htitle.data(), 1e3, 0, 2e4, 1e3, 0, 2e4 );

			hname = "pn_1v2_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side multiplicity = 1 vs. n-side multiplicity = 2 (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");p-side energy [keV];n-side energy [keV]";
			pn_12[i][j] = new TH2F( hname.data(), htitle.data(), 1e3, 0, 2e4, 1e3, 0, 2e4 );

			hname = "pn_2v1_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side multiplicity = 2 vs. n-side multiplicity = 1 (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");p-side energy [keV];n-side energy [keV]";
			pn_21[i][j] = new TH2F( hname.data(), htitle.data(), 1e3, 0, 2e4, 1e3, 0, 2e4 );

			hname = "pn_2v2_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side multiplicity = 2 vs. n-side multiplicity = 2 (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");p-side energy [keV];n-side energy [keV]";
			pn_22[i][j] = new TH2F( hname.data(), htitle.data(), 1e3, 0, 2e4, 1e3, 0, 2e4 );

			hname = "pn_ab_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side addback energy vs. n-side addback energy (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");p-side energy [keV];n-side energy [keV]";
			pn_ab[i][j] = new TH2F( hname.data(), htitle.data(), 1e3, 0, 2e4, 1e3, 0, 2e4 );

			hname = "pn_nab_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side singles energy vs. n-side addback energy (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");p-side energy [keV];n-side energy [keV]";
			pn_nab[i][j] = new TH2F( hname.data(), htitle.data(), 1e3, 0, 2e4, 1e3, 0, 2e4 );

			hname = "pn_pab_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side addback energy vs. n-side singles energy (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");p-side energy [keV];n-side energy [keV]";
			pn_pab[i][j] = new TH2F( hname.data(), htitle.data(), 1e3, 0, 2e4, 1e3, 0, 2e4 );

			hname = "pn_max_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side max energy vs. n-side max energy (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");p-side energy [keV];n-side energy [keV]";
			pn_max[i][j] = new TH2F( hname.data(), htitle.data(), 1e3, 0, 2e4, 1e3, 0, 2e4 );

			hname = "pn_td_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side vs. n-side time difference after correction (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");time difference [ns];counts";
			pn_td[i][j] = new TH1F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20 );

			hname = "pn_td_Ep_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side n-side time difference after correction vs p-side energy (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");time difference [ns];p-side energy [keV]";
			pn_td_Ep[i][j] = new TH2F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20, 1e3, 0, 2e4 );

			hname = "pn_td_En_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side n-side time difference after correction vs n-side energy (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");time difference [ns];n-side energy [keV]";
			pn_td_En[i][j] = new TH2F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20, 1e3, 0, 2e4  );

			hname = "pn_td_uncorrected_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side vs. n-side time difference before correction (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");time difference [ns];counts";
			pn_td_uncorrected[i][j] = new TH1F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20 );

			hname = "pn_td_Ep_uncorrected_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side n-side time difference before correction vs p-side energy (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");time difference [ns];p-side energy [keV]";
			pn_td_Ep_uncorrected[i][j] = new TH2F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20, 1e3, 0, 2e4 );

			hname = "pn_td_En_uncorrected_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side n-side time difference before correction vs n-side energy (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");time difference [ns];n-side energy [keV]";
			pn_td_En_uncorrected[i][j] = new TH2F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20, 1e3, 0, 2e4  );

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
		recoil_EdE[i] = new TH2F( hname.data(), htitle.data(), 2000, 0, 400000, 2000, 0, 400000 );

		hname = "recoil_dEsum" + std::to_string(i);
		htitle = "Recoil dE vs Esum for sector " + std::to_string(i);
		htitle += ";Total energy, Esum [keV];Energy loss, dE [keV];Counts";
		recoil_dEsum[i] = new TH2F( hname.data(), htitle.data(), 2000, 0, 400000, 2000, 0, 400000 );

		hname = "recoil_EdE_raw" + std::to_string(i);
		htitle = "Recoil dE vs E for sector " + std::to_string(i);
		htitle += ";Rest energy, E [arb.];Energy loss, dE [arb.];Counts";
		recoil_EdE_raw[i] = new TH2F( hname.data(), htitle.data(), 2048, 0, 65536, 2048, 0, 65536 );

		hname = "recoil_E_singles" + std::to_string(i);
		htitle = "Recoil E singles in sector " + std::to_string(i);
		htitle += "; E [keV]; Counts";
		recoil_E_singles[i] = new TH1F( hname.data(), htitle.data(), 2000, 0, 400000 );

		hname = "recoil_dE_singles" + std::to_string(i);
		htitle = "Recoil dE singles in sector " + std::to_string(i);
		htitle += "; dE [keV]; Counts";
		recoil_dE_singles[i] = new TH1F( hname.data(), htitle.data(), 2000, 0, 400000 );

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

	hname = "mwpc_pos_raw";
	htitle = "MWPC x-y TAC difference;x [arb. units];y [arb. units];Counts";
	mwpc_pos_raw = new TH2F( hname.data(), htitle.data(), 2048, -65536, 65536, 2048, -65536, 65536 );

	hname = "mwpc_pos_mm";
	htitle = "MWPC x-y TAC difference;x [mm];y [mm];Counts";
	mwpc_pos_mm = new TH2F( hname.data(), htitle.data(), 2000, -200, 200, 2000, -200, 200 );


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
	zd_EdE = new TH2F( hname.data(), htitle.data(), 2000, 0, 20000, 2000, 0, 400000 );


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
	lume_ne_vs_fe.resize( set->GetNumberOfLUMEDetectors() );

	// Loop over number of LUME detectors
	for( unsigned int i = 0; i < set->GetNumberOfLUMEDetectors(); ++i ) {

		hname = "lume_E_" + std::to_string(i);
		htitle = "LUME energy spectrum;Energy [keV];Counts per 2 keV";
		lume_E[i] = new TH1F( hname.data(), htitle.data(), 4100, -200, 8000);

		hname = "lume_E_vs_x_" + std::to_string(i);
		htitle = "LUME energy vs position spectrum;Position;Energy [keV]";
		lume_E_vs_x[i] = new TH2F( hname.data(), htitle.data(), 400, -2., 2., 2050, -200, 8000 );

		hname = "lume_ne_vs_fe" + std::to_string(i);
		htitle = "LUME near-side energy vs far-side energy;Energy [keV];Energy [keV]";
		lume_ne_vs_fe[i] = new TH2F( hname.data(), htitle.data(), 2050, -200, 8000, 2050, -200, 8000 );

	}

	// ------------- //
	// CD histograms //
	// ------------- //
	dirname = "cd";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	cd_rs_mult.resize( set->GetNumberOfCDLayers() );
	cd_rs_en.resize( set->GetNumberOfCDLayers() );
	cd_rs_td.resize( set->GetNumberOfCDLayers() );
	cd_r_en.resize( set->GetNumberOfCDLayers() );
	cd_s_en.resize( set->GetNumberOfCDLayers() );

	for( unsigned int i = 0; i < set->GetNumberOfCDLayers(); ++i ) {

		hname = "cd_rs_mult_" + std::to_string(i);
		htitle = "ring vs. sector multiplicity (layer ";
		htitle += std::to_string(i) + ");mult rings;mult sectors";
		cd_rs_mult[i] = new TH2F( hname.data(), htitle.data(), 6, -0.5, 5.5, 6, -0.5, 5.5 );

		hname = "cd_rs_en_" + std::to_string(i);
		htitle = "ring vs. sector energy (layer ";
		htitle += std::to_string(i) + ");Energy rings (keV);Energy sectors (keV)";
		cd_rs_en[i] = new TH2F( hname.data(), htitle.data(), 1e3, 0, 2e6, 1e3, 0, 2e6 );

		hname = "cd_rs_td_" + std::to_string(i);
		htitle = "ring vs. sector time difference (layer ";
		htitle += std::to_string(i) + ");Time difference, #Deltat = t_{ring} - t_{sector} (ns)";
		cd_rs_td[i] = new TH1F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20 );

		hname = "cd_r_en_" + std::to_string(i);
		htitle = "ring ID vs. energy (layer ";
		htitle += std::to_string(i) + ");Ring ID; Energy (10 keV)";
		cd_r_en[i] = new TH2F( hname.data(), htitle.data(),
							  set->GetNumberOfCDRings()+1, -0.5, set->GetNumberOfCDRings()+0.5,
							  1e3, 0, 2e6 );

		hname = "cd_s_en_" + std::to_string(i);
		htitle = "Sector ID vs. energy (layer ";
		htitle += std::to_string(i) + ");Sector ID; Energy (10 keV)";
		cd_s_en[i] = new TH2F( hname.data(), htitle.data(),
							  set->GetNumberOfCDSectors()+1, -0.5, set->GetNumberOfCDSectors()+0.5,
							  1e3, 0, 2e6 );

	}

	hname = "cd_EdE";
	htitle = "CD fission fragments dE vs E;Rest Energy [keV];Energy Loss [keV];Counts";
	cd_EdE = new TH2F( hname.data(), htitle.data(), 1e3, 0, 2e6, 1e3, 0, 2e6 );

	hname = "cd_dEsum";
	htitle = "CD fission fragments dE vs total energy;Total Energy [keV];Energy Loss [keV];Counts";
	cd_dEsum = new TH2F( hname.data(), htitle.data(), 1e3, 0, 2e6, 1e3, 0, 2e6 );

	hname = "cd_id_td";
	htitle = "CD time difference between dE layer (id=0) and all other layers;";
	htitle +=  + "CD layer ID;Time difference, #Deltat = t_{0} - t_{id} (ns)";
	cd_id_td = new TH2F( hname.data(), htitle.data(),
						set->GetNumberOfCDLayers()+1, -0.5, set->GetNumberOfCDLayers()+0.5,
						600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20 );


	// flag to denote that hists are ready (used for spy)
	hists_ready = true;

	return;

}

////////////////////////////////////////////////////////////////////////////////
/// This function plots common histograms for diagnostic purposes; used during the DataSpy
void ISSEventBuilder::PlotDiagnostics() {

	// Escape if we haven't built the hists to avoid a seg fault
	if( !hists_ready ){

		std::cout << "Cannot plot diagnostics yet, wait until event builder is ready" << std::endl;
		return;

	}

	// Make canvas
	c1 = std::make_unique<TCanvas>("SyncCheck","SyncCheck");
	c1->Divide( 3, 4 );

	// ASIC time difference histograms
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); i++ ){

		c1->cd(i+1);
		asic_td[i]->Draw("hist");

	}

	// Mesytec time difference histograms
	for( unsigned int i = 0; i < set->GetNumberOfMesytecModules(); i++ ){

		c1->cd(i+1 + set->GetNumberOfArrayModules());
		mesy_td[i]->Draw("hist");

	}

	output_file->cd();
	c1->Write();

}

////////////////////////////////////////////////////////////////////////////////
/// This function empties the histograms used in the EventBuilder class; used during the DataSpy
void ISSEventBuilder::ResetHist( TObject *obj ) {

	if( obj == nullptr ) return;

	if( obj->InheritsFrom( "TH2" ) ) {
		( (TH2*)obj )->Reset("ICESM");
		( (TH2*)obj )->GetZaxis()->UnZoom();
	}
	else if( obj->InheritsFrom( "TH1" ) )
		( (TH1*)obj )->Reset("ICESM");

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

	mwpc_pos_mm->Reset("ICESM");
	mwpc_pos_raw->Reset("ICESM");
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

	for( unsigned int i = 0; i < lume_ne_vs_fe.size(); i++ )
		lume_ne_vs_fe[i]->Reset("ICESM");

	for( unsigned int i = 0; i < cd_rs_mult.size(); i++ )
		cd_rs_mult[i]->Reset("ICESM");

	for( unsigned int i = 0; i < cd_rs_en.size(); i++ )
		cd_rs_en[i]->Reset("ICESM");

	for( unsigned int i = 0; i < cd_rs_td.size(); i++ )
		cd_rs_td[i]->Reset("ICESM");

	cd_EdE->Reset("ICESM");
	cd_dEsum->Reset("ICESM");

	for( unsigned int i = 0; i < set->GetNumberOfMesytecModules(); i++ ){
		mesy_td[i]->Reset("ICESM");
		mesy_pulser_loss[i]->Reset("ICESM");
		mesy_period[i]->Reset("ICESM");
		mesy_sync[i]->Reset("ICESM");
	}

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
