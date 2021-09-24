#include "EventBuilder.hh"

EventBuilder::EventBuilder( Settings *myset ){
	
	// First get the settings
	set = myset;
	
	// ------------------------------------------------------------------------ //
	// Initialise variables and flags
	// ------------------------------------------------------------------------ //
	build_window = set->GetEventWindow();

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
	
	
	// Loop over ASICs in a module
	for( unsigned int i = 0; i < set->GetNumberOfArrayASICs(); ++i ) {
		
		// Loop over channels in each ASIC
		for( unsigned int j = 0; j < set->GetNumberOfArrayChannels(); ++j ) {

			// Sort out the rows 
			// pside all ok
			// nside need incrementing for odd wafers
			array_row.at(i).push_back( asic_row.at(i) );
			
			// p-side: all channels used
			if( asic_side.at(i) == 0 ) {
				
				mystrip = j + set->GetNumberOfArrayPstrips() * asic_row.at(i);
				array_pid.push_back( mystrip );
				
			}
			
			// n-side: 11 channels per ASIC 0/2A
			else if( j >= 11 && j <= 21 ) {
				
				mystrip = j - 11;
				array_nid.push_back( mystrip );
				
			}
			
			// n-side: 11 channels per ASIC 0/2B
			else if( j >= 28 && j <= 38 ) {
				
				mystrip = 38 - j + set->GetNumberOfArrayNstrips();
				array_nid.push_back( mystrip );

			}
			
			// n-side: 11 channels per ASIC 1/3A
			else if( j >= 89 && j <= 99 ) {
				
				mystrip = j - 89 + set->GetNumberOfArrayNstrips();
				array_nid.push_back( mystrip );
				array_row.at(i).at(j)++;

			}
			
			// n-side: 11 channels per ASIC 1/3B
			else if( j >= 106 && j <= 116 ) {
				
				mystrip = 116 - j;
				array_nid.push_back( mystrip );
				array_row.at(i).at(j)++;

			}
			
			// n-side: empty channels
			else array_nid.push_back(-1);
	
		}
		
	}
	
}

EventBuilder::~EventBuilder(){
	
	//delete output_tree;
	//delete write_evts;
	//delete array_evt;
	//delete recoil_evt;
	//delete elum_evt;
	//delete zd_evt;

}

void EventBuilder::StartFile(){
	
	// Call for every new file
	// Reset counters etc.
	
	time_prev = 0;
	caen_time = 0;
	caen_prev = 0;
	ebis_prev = 0;
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

void EventBuilder::SetInputFile( std::vector<std::string> input_file_names ) {
	
	/// Overloaded function for a single file or multiple files
	input_tree = new TChain( "iss_sort" );
	for( unsigned int i = 0; i < input_file_names.size(); i++ ) {
	
		input_tree->Add( input_file_names[i].data() );
		
	}
	input_tree->SetBranchAddress( "data", &in_data );
	
	StartFile();

	return;
	
}

void EventBuilder::SetInputFile( std::string input_file_name ) {
	
	/// Overloaded function for a single file or multiple files
	input_tree = new TChain( "iss_sort" );
	input_tree->Add( input_file_name.data() );
	input_tree->SetBranchAddress( "data", &in_data );

	StartFile();

	return;
	
}

void EventBuilder::SetInputTree( TTree *user_tree ){
	
	// Find the tree and set branch addresses
	input_tree = (TChain*)user_tree;
	input_tree->SetBranchAddress( "data", &in_data );

	StartFile();

	return;
	
}

void EventBuilder::SetOutput( std::string output_file_name ) {

	// These are the branches we need
	write_evts = new ISSEvts();
	array_evt = new ArrayEvt();
	arrayp_evt = new ArrayPEvt();
	recoil_evt = new RecoilEvt();
	elum_evt = new ElumEvt();
	zd_evt = new ZeroDegreeEvt();

	// ------------------------------------------------------------------------ //
	// Create output file and create events tree
	// ------------------------------------------------------------------------ //
	output_file = new TFile( output_file_name.data(), "recreate" );
	output_tree = new TTree( "evt_tree", "evt_tree" );
	output_tree->Branch( "ISSEvts", "ISSEvts", &write_evts );

	// Hisograms in separate function
	MakeEventHists();
	
}

void EventBuilder::Initialise(){

	/// This is called at the end of every execution/loop
	
	flag_close_event = false;

	hit_ctr = 0;
	
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
	
	een_list.clear();
	etd_list.clear();
	esec_list.clear();
	
	zen_list.clear();
	ztd_list.clear();
	zid_list.clear();
	
	write_evts->ClearEvt();
	
	return;
	
}

unsigned long EventBuilder::BuildEvents( unsigned long start_build ) {
	
	/// Function to loop over the sort tree and build array and recoil events

	if( input_tree->LoadTree(0) < 0 ){
		
		std::cout << " Event Building: nothing to do" << std::endl;
		return 0;
		
	}
	
	// Get ready and go
	Initialise();
	n_entries = input_tree->GetEntries();

	std::cout << " Event Building: number of entries in input tree = ";
	std::cout << n_entries << std::endl;
	std::cout << " Start build at event " << start_build << std::endl;

	
	// ------------------------------------------------------------------------ //
	// Main loop over TTree to find events
	// ------------------------------------------------------------------------ //
	for( unsigned long i = start_build; i < n_entries; ++i ) {
		
		// Current event data
		input_tree->GetEntry(i);
		
		// Get the time of the event
		mytime = in_data->GetTime();
				
		// check time stamp monotonically increases!
		if( time_prev > mytime ) {
			
			std::cout << "Out of order event in file ";
			std::cout << input_tree->GetFile()->GetName() << std::endl;
			
		}
			
		// Sort out the timing for the event window
		// but only if it isn't an info event, i.e only for real data
		if( !in_data->IsInfo() ) {
			
			// if this is first datum included in Event
			if( hit_ctr == 0 ) {
				
				time_min	= mytime;
				time_max	= mytime;
				time_first	= mytime;
				
			}
			hit_ctr++; // increase counter for bits of data included in this event
			
			// record time of this event
			time_prev = mytime;
			
			// Update min and max
			if( mytime > time_max ) time_max = mytime;
			else if( mytime < time_min ) time_min = mytime;
			
		}

		// ------------------------------------------ //
		// Find particles on the array
		// ------------------------------------------ //
		if( in_data->IsAsic() ) {
			
			// Increment event counter
			n_asic_data++;
			
			asic_data = in_data->GetAsicData();
			mymod = asic_data->GetModule();
			myside = asic_side.at( asic_data->GetAsic() );
			myrow = array_row.at( asic_data->GetAsic() ).at( asic_data->GetChannel() );
			if( overwrite_cal ) {
			
				myenergy = cal->AsicEnergy( mymod, asic_data->GetAsic(),
									 asic_data->GetChannel(), asic_data->GetAdcValue() );
				mywalk = cal->AsicWalk( mymod, asic_data->GetAsic(), myenergy );
			
				if( asic_data->GetAdcValue() > cal->AsicThreshold( mymod, asic_data->GetAsic(), mych ) )
					mythres = true;
				else mythres = false;
				
			}
			
			else {
				
				myenergy = asic_data->GetEnergy();
				mywalk = asic_data->GetWalk();
				mythres = asic_data->IsOverThreshold();
			
			}
			
			// p-side event
			if( myside == 0 && mythres ) {
			
			// test here about hit bit value
			//if( myside == 0 && !asic_data->GetHitBit() ) {

				mystrip = array_pid.at( asic_data->GetChannel() );
				
				pen_list.push_back( myenergy );
				ptd_list.push_back( mytime + mywalk );
				pmod_list.push_back( mymod );
				pid_list.push_back( mystrip );
				prow_list.push_back( myrow );

			}

			// n-side event
			else if( myside == 1 && mythres ) {

			// test here about hit bit value
			//else if( myside == 1 && asic_data->GetHitBit() ) {

				mystrip = array_nid.at( asic_data->GetChannel() );
				
				nen_list.push_back( myenergy );
				ntd_list.push_back( mytime + mywalk );
				nmod_list.push_back( mymod );
				nid_list.push_back( mystrip );
				nrow_list.push_back( myrow );

				
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
		if( in_data->IsCaen() ) {
			
			// Increment event counter
			n_caen_data++;
			
			caen_data = in_data->GetCaenData();
			mymod = caen_data->GetModule();
			mych = caen_data->GetChannel();
			if( overwrite_cal ) {
				
				myenergy = cal->CaenEnergy( mymod, mych,
									caen_data->GetQlong() );
				
				if( caen_data->GetQlong() > cal->CaenThreshold( mymod, mych ) )
					mythres = true;
				else mythres = false;

			}
			
			else {
				
				myenergy = caen_data->GetEnergy();
				mythres = caen_data->IsOverThreshold();

			}
			
			// Is it a recoil
			if( set->IsRecoil( mymod, mych ) && mythres ) {
				
				mysector = set->GetRecoilSector( mymod, mych );
				mylayer = set->GetRecoilLayer( mymod, mych );
				
				ren_list.push_back( myenergy );
				rtd_list.push_back( mytime );
				rid_list.push_back( mylayer );
				rsec_list.push_back( mysector );

			}
			
			// Is it an ELUM?
			if( set->IsELUM( mymod, mych ) && mythres ) {
				
				mysector = set->GetELUMSector( mymod, mych );
				
				een_list.push_back( myenergy );
				etd_list.push_back( mytime );
				esec_list.push_back( mysector );

			}

			// Is it a ZeroDegree?
			if( set->IsZD( mymod, mych ) && mythres ) {
				
				mylayer = set->GetZDLayer( mymod, mych );
				
				zen_list.push_back( myenergy );
				ztd_list.push_back( mytime );
				zid_list.push_back( mylayer );
				
			}
			
			// Is it the start event?
			if( caen_time_start.at( mymod ) == 0 )
				caen_time_start.at( mymod ) = mytime;
			
			// or is it the end event (we don't know so keep updating
			caen_time_stop.at( mymod ) = mytime;


		}
		
		
		// ------------------------------------------ //
		// Find info events, like timestamps etc
		// ------------------------------------------ //
		if( in_data->IsInfo() ) {
			
			// Increment event counter
			n_info_data++;
			
			info_data = in_data->GetInfoData();
			
			// Update EBIS time
			if( info_data->GetCode() == set->GetEBISCode() ) {
				
				ebis_time = info_data->GetTime();
				ebis_hz = 1e9 / ( (double)ebis_time - (double)ebis_prev );
				if( ebis_prev != 0 ) ebis_freq->Fill( ebis_time, ebis_hz );
				ebis_prev = ebis_time;
				n_ebis++;
				
			}
		
			// Update T1 time
			if( info_data->GetCode() == set->GetT1Code() ){
				
				t1_time = info_data->GetTime();
				t1_hz = 1e9 / ( (double)t1_time - (double)t1_prev );
				if( t1_prev != 0 ) t1_freq->Fill( t1_time, t1_hz );
				t1_prev = t1_time;
				n_t1++;

			}
			
			// Update CAEN pulser time
			if( info_data->GetCode() == set->GetCAENPulserCode() ) {
				
				caen_time = info_data->GetTime();
				caen_hz = 1e9 / ( (double)caen_time - (double)caen_prev );
				if( caen_prev != 0 ) caen_freq->Fill( caen_time, caen_hz );

				flag_caen_pulser = true;
				n_caen_pulser++;

			}

			// Update ISS pulser time in FPGA
			if( info_data->GetCode() == set->GetExternalTriggerCode() ) {
			   
				fpga_time[info_data->GetModule()] = info_data->GetTime();
				fpga_hz = 1e9 / ( (double)fpga_time[i] - (double)fpga_prev[i] );

				if( fpga_prev[info_data->GetModule()] != 0 )
					fpga_freq[info_data->GetModule()]->Fill( fpga_time[info_data->GetModule()], fpga_hz );

				n_fpga_pulser[info_data->GetModule()]++;

			}
			
			// Update ISS pulser time in ASICs
			if( info_data->GetCode() == set->GetArrayPulserCode() ) {
			   
				asic_time[info_data->GetModule()] = info_data->GetTime();
				asic_hz = 1e9 / ( (double)asic_time[i] - (double)asic_prev[i] );

				if( asic_prev[info_data->GetModule()] != 0 )
					asic_freq[info_data->GetModule()]->Fill( asic_time[info_data->GetModule()], asic_hz );

				n_asic_pulser[info_data->GetModule()]++;

			}
			
			// Check the pause events for each module
			if( info_data->GetCode() == set->GetPauseCode() ) {
				
				if( info_data->GetModule() < set->GetNumberOfArrayModules() ) {
				
					n_asic_pause[info_data->GetModule()]++;
					flag_pause[info_data->GetModule()] = true;
					pause_time[info_data->GetModule()] = info_data->GetTime();
				
				}
				
				else
					std::cerr << "Bad pause event in module " << (int)info_data->GetModule() << std::endl;
				
			}
			
			// Check the resume events for each module
			if( info_data->GetCode() == set->GetResumeCode() ) {
				
				if( info_data->GetModule() < set->GetNumberOfArrayModules() ) {
				
					n_asic_resume[info_data->GetModule()]++;
					flag_resume[info_data->GetModule()] = true;
					resume_time[info_data->GetModule()] = info_data->GetTime();
					
					// Work out the dead time
					asic_dead_time[info_data->GetModule()] += resume_time[info_data->GetModule()];
					asic_dead_time[info_data->GetModule()] -= pause_time[info_data->GetModule()];

					// If we have didn't get the pause, module was stuck at start of run
					if( !flag_pause[info_data->GetModule()] ) {

						std::cout << "Module " << info_data->GetModule();
						std::cout << " was blocked at start of run for ";
						std::cout << (double)resume_time[info_data->GetModule()]/1e9;
						std::cout << " seconds" << std::endl;
					
					}
				
				}
				
				else
					std::cerr << "Bad resume event in module " << (int)info_data->GetModule() << std::endl;
				
			}

			// If we a pulser event from the CAEN DAQs, fill time difference
			if( flag_caen_pulser ) {
			
				for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {
				
					fpga_tdiff = (double)caen_time - (double)fpga_time[i];
					asic_tdiff = (double)caen_time - (double)asic_time[i];

					// If diff is greater than 5 ms, we have the wrong pair
					if( fpga_tdiff > 5e6 ) fpga_tdiff = (double)caen_prev - (double)fpga_time[i];
					else if( fpga_tdiff < -5e6 ) fpga_tdiff = (double)caen_time - (double)fpga_prev[i];
					if( asic_tdiff > 5e6 ) asic_tdiff = (double)caen_prev - (double)asic_time[i];
					else if( asic_tdiff < -5e6 ) asic_tdiff = (double)caen_time - (double)asic_prev[i];

					fpga_td[i]->Fill( fpga_tdiff );
					fpga_sync[i]->Fill( fpga_time[i], fpga_tdiff );
					fpga_freq_diff[i]->Fill( fpga_time[i], fpga_hz - caen_hz );
					fpga_pulser_loss[i]->Fill( fpga_time[i], (int)n_fpga_pulser[i] - (int)n_caen_pulser );
					
					asic_td[i]->Fill( asic_tdiff );
					asic_sync[i]->Fill( asic_time[i], asic_tdiff );
					asic_freq_diff[i]->Fill( asic_time[i], asic_hz - caen_hz );
					asic_pulser_loss[i]->Fill( asic_time[i], (int)n_asic_pulser[i] - (int)n_caen_pulser );	

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

		
		//------------------------------
		//  check if last datum from this event and do some cleanup
		//------------------------------
		
		if( (i+1) == n_entries )
			flag_close_event = true; // set flag to close this event
			
		else {  //check if next entry is beyond time window: close event!

			input_tree->GetEntry(i+1);
						
			time_diff = in_data->GetTime() - time_first;

			// window = time_stamp_first + time_window
			if( time_diff > build_window )
				flag_close_event = true; // set flag to close this event

			// we've gone on to the next file in the chain
			else if( time_diff < 0 )
				flag_close_event = true; // set flag to close this event
				
			// Fill tdiff hist only for real data
			if( !in_data->IsInfo() ) tdiff->Fill( time_diff );


		} // if next entry beyond time window: close event!
		
		
		//----------------------------
		// if close this event and number of datums in event>0
		//----------------------------
		if( flag_close_event && hit_ctr > 0 ) {

			//----------------------------------
			// Build array events, recoils, etc
			//----------------------------------
			ArrayFinder();		// add an ArrayEvt for each n/p pair
			RecoilFinder();		// add a RecoilEvt for each dE-E
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
			    write_evts->GetElumMultiplicity() ||
			    write_evts->GetZeroDegreeMultiplicity() ) output_tree->Fill();


			//--------------------------------------------------
			// clear values of arrays to store intermediate info
			//--------------------------------------------------
			Initialise();
			
		} // if close event && hit_ctr > 0
		
		if( i % 10000 == 0 || i+1 == n_entries ) {
			
			std::cout << " " << std::setw(8) << std::setprecision(4);
			std::cout << (float)(i+1)*100.0/(float)n_entries << "%    \r";
			std::cout.flush();
			
		}
		
		
	} // End of main loop over TTree to process raw MIDAS data entries (for n_entries)
	
	//--------------------------
	// Clean up
	//--------------------------

	std::cout << "\n EventBuilder finished..." << std::endl;
	std::cout << "  ASIC data packets = " << n_asic_data << std::endl;
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {
		std::cout << "   Module " << i << " pause = " << n_asic_pause[i] << std::endl;
		std::cout << "           resume = " << n_asic_resume[i] << std::endl;
		std::cout << "        dead time = " << (double)asic_dead_time[i]/1e9 << " s" << std::endl;
		std::cout << "        live time = " << (double)(asic_time_stop[i]-asic_time_start[i])/1e9 << " s" << std::endl;
	}
	std::cout << "  CAEN data packets = " << n_caen_data << std::endl;
	for( unsigned int i = 0; i < set->GetNumberOfCAENModules(); ++i ) {
		std::cout << "   Module " << i << " live time = ";
		std::cout << (double)(caen_time_stop[i]-caen_time_start[i])/1e9;
		std::cout << " s" << std::endl;
	}
	std::cout << "  Info data packets = " << n_info_data << std::endl;
	std::cout << "   Array p/n-side correlated events = " << array_ctr << std::endl;
	std::cout << "   Array p-side only events = " << arrayp_ctr << std::endl;
	std::cout << "   Recoil events = " << recoil_ctr << std::endl;
	std::cout << "   ELUM events = " << elum_ctr << std::endl;
	std::cout << "   ZeroDegree events = " << zd_ctr << std::endl;
	std::cout << "   CAEN pulser = " << n_caen_pulser << std::endl;
	std::cout << "   FPGA pulser" << std::endl;
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i )
		std::cout << "    Module " << i << " = " << n_fpga_pulser[i] << std::endl;
	std::cout << "   ASIC pulser" << std::endl;
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i )
		std::cout << "    Module " << i << " = " << n_asic_pulser[i] << std::endl;
	std::cout << "   EBIS events = " << n_ebis << std::endl;
	std::cout << "   T1 events = " << n_t1 << std::endl;
	std::cout << "  Tree entries = " << output_tree->GetEntries() << std::endl;

	output_file->Write( 0, TObject::kWriteDelete );
	//output_file->Print();
	//output_file->Close();
	
	std::cout << std::endl;
	
	return n_entries;
	
}


void EventBuilder::ArrayFinder() {
	
	std::vector<unsigned int> pindex;
	std::vector<unsigned int> nindex;
	int pmax_idx, nmax_idx;
	float pmax_en, nmax_en;

	// Do each module and row individually
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {
		
		for( unsigned int j = 0; j < set->GetNumberOfArrayRows(); ++j ) {
			
			// Empty the array of indexes
			pindex.clear();
			nindex.clear();
			std::vector<unsigned int>().swap(pindex);
			std::vector<unsigned int>().swap(nindex);
			pmax_idx = nmax_idx = -1;
			pmax_en = nmax_en = -1;

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

			// Time difference hists
			for( unsigned int k = 0; k < pindex.size(); ++k )
				for( unsigned int l = 0; l < nindex.size(); ++l )
					pn_td[i][j]->Fill( ptd_list.at( pindex.at(k) ) - ntd_list.at( nindex.at(l) ) );

			
			// Easy case, 1p vs 1n
			if( pindex.size() == 1 && nindex.size() == 1 ) {
				
				pn_11[i][j]->Fill( pen_list.at( pindex.at(0) ), nen_list.at( nindex.at(0) ) );
				
			}
			
			// Others too, but who can be arsed at the moment
			if( pindex.size() == 2 && nindex.size() == 1 ) {

				pn_21[i][j]->Fill( pen_list.at( pindex.at(0) ), nen_list.at( nindex.at(0) ) );
				pn_21[i][j]->Fill( pen_list.at( pindex.at(1) ), nen_list.at( nindex.at(0) ) );
				
			}
			
			if( pindex.size() == 1 && nindex.size() == 2 ) {

				pn_12[i][j]->Fill( pen_list.at( pindex.at(0) ), nen_list.at( nindex.at(0) ) );
				pn_12[i][j]->Fill( pen_list.at( pindex.at(0) ), nen_list.at( nindex.at(1) ) );
				
			}
			
			if( pindex.size() == 2 && nindex.size() == 2 ) {

				pn_22[i][j]->Fill( pen_list.at( pindex.at(0) ), nen_list.at( nindex.at(0) ) );
				pn_22[i][j]->Fill( pen_list.at( pindex.at(0) ), nen_list.at( nindex.at(1) ) );
				pn_22[i][j]->Fill( pen_list.at( pindex.at(1) ), nen_list.at( nindex.at(0) ) );
				pn_22[i][j]->Fill( pen_list.at( pindex.at(1) ), nen_list.at( nindex.at(1) ) );
				
			}
			
			// Just take the maximum energy instead for now
			// But make sure that both p- and n-sides are good
			if( pmax_idx >= 0 && nmax_idx >= 0 ){

				array_evt->SetEvent( pen_list.at( pmax_idx ),
									 nen_list.at( nmax_idx ),
									 pid_list.at( pmax_idx ),
									 nid_list.at( nmax_idx ),
									 ptd_list.at( pmax_idx ),
									 ntd_list.at( nmax_idx ),
									 i, j );
				
				write_evts->AddEvt( array_evt );
				array_ctr++;

			}

			// Add a bodge to ignore n-side events for now
			// use the ArrayPEvt class
			if( pmax_idx >= 0 ){
			
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
			
			// Histogram for n vs p-side max energies
			pn_max[i][j]->Fill( pmax_en, nmax_en );
			
		}

	}
		
}

void EventBuilder::RecoilFinder() {
		
	// Checks to prevent re-using events
	std::vector<unsigned int> index;
	bool flag_skip;
	float sum_energy;
	
	// Loop over recoil events
	for( unsigned int i = 0; i < ren_list.size(); ++i ) {
		
		// Find the dE event, usually the trigger
		if( rid_list[i] == 0 ){
			
			sum_energy = ren_list[i];
			recoil_evt->ClearEvent();
			recoil_evt->SetdETime( rtd_list[i] );
			recoil_evt->SetSector( rsec_list[i] );
			recoil_evt->AddRecoil( ren_list[i], 0 );

			// Look for matching dE events
			for( unsigned int j = 0; j < ren_list.size(); ++j ) {

				// Check if we already used this hit
				flag_skip = false;
				for( unsigned int k = 0; k < index.size(); ++k )
					if( index[k] == j ) flag_skip = true;

				
				// Found a match
				if( i != j && rid_list[j] != 0 && !flag_skip &&
				    rsec_list[i] == rsec_list[j] ){
					
					sum_energy += ren_list[j];
					index.push_back(j);
					recoil_evt->AddRecoil( ren_list[j], rid_list[j] );
					if( rid_list[j] == 1 ) recoil_evt->SetETime( rtd_list[j] );
					
					// All vs. dE
					recoil_EdE[rsec_list[i]]->Fill( ren_list[j], ren_list[i] );
					
				}
				
			}
			
			// Histogram the recoils
			recoil_dEsum[rsec_list[i]]->Fill( sum_energy, ren_list[i] );
			
			// Fill the tree and get ready for next recoil event
			write_evts->AddEvt( recoil_evt );
			recoil_ctr++;
			
		}
		
	}
	
}

void EventBuilder::ElumFinder() {
	
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
		
	}
	
}

void EventBuilder::ZeroDegreeFinder() {
	
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
				if( i != j && zid_list[j] != 0 && !flag_skip ){
					
					index.push_back(j);
					zd_evt->AddZeroDegree( zen_list[j], zid_list[j] );
					if( zid_list[j] == 1 ) zd_evt->SetETime( ztd_list[i] );
					
					// Histogram the ZeroDegree
					zd->Fill( zid_list[i], zen_list[j] );

				}
				
			}
			
			// Fill the tree and get ready for next recoil event
			write_evts->AddEvt( zd_evt );
			zd_ctr++;
			
			
		}
		
	}
	
}

void EventBuilder::MakeEventHists(){
	
	std::string hname, htitle;
	std::string dirname, maindirname, subdirname;
	
	// ----------------- //
	// Timing histograms //
	// ----------------- //
	dirname =  "timing";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	tdiff = new TH1F( "tdiff", "Time difference to first trigger;#Delta t [ns]", 1e3, -10, 1e5 );

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
		asic_td[i] = new TH1F( hname.data(), htitle.data(), 16e3 , -400e3, 400e3 );
		
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
		fpga_td[i] = new TH1F( hname.data(), htitle.data(), 16e3 , -400e3, 400e3 );

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
	pn_max.resize( set->GetNumberOfArrayModules() );
	pn_td.resize( set->GetNumberOfArrayModules() );
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
		pn_max[i].resize( set->GetNumberOfArrayRows() );
		pn_td[i].resize( set->GetNumberOfArrayRows() );
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
			
			hname = "pn_max_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side max energy vs. n-side max energy (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");p-side energy [keV];n-side energy [keV]";
			pn_max[i][j] = new TH2F( hname.data(), htitle.data(), 2e3, 0, 2e4, 2e3, 0, 2e4 );
			
			hname = "pn_td_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side vs. n-side time difference (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");time difference [ns];counts";
			pn_td[i][j] = new TH1F( hname.data(), htitle.data(), 600, -1.0*set->GetEventWindow()-20, set->GetEventWindow()+20 );
			
			hname = "pn_mult_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side vs. n-side multiplicity (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");mult p-side;mult n-side";
			pn_mult[i][j] = new TProfile( hname.data(), htitle.data(), 20, -0.5, 19.5 );
		
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
	recoil_dEsum.resize( set->GetNumberOfRecoilSectors() );
	
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
		
	}
	
	
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
	htitle = "ZeroDegree dE vs E;Energy Loss [keV]; Rest Energy [keV];Counts";
	zd = new TH2F( hname.data(), htitle.data(), 2000, 0, 20000, 2000, 0, 200000 );

	return;
	
}

void EventBuilder::CleanHists() {

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

	for( unsigned int i = 0; i < pn_mult.size(); i++ ) {
		for( unsigned int j = 0; j < pn_mult.at(i).size(); j++ )
			delete (pn_mult[i][j]);
		pn_mult.clear();
	}

	for( unsigned int i = 0; i < recoil_EdE.size(); i++ )
		delete (recoil_EdE[i]);
	
	for( unsigned int i = 0; i < recoil_dEsum.size(); i++ )
		delete (recoil_dEsum[i]);

	pn_12.clear();
	pn_21.clear();
	pn_22.clear();
	pn_max.clear();
	pn_td.clear();
	pn_mult.clear();
	recoil_EdE.clear();
	recoil_dEsum.clear();
	
	delete elum;
	delete zd;

	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); i++ ){
		delete (fpga_td[i]);
		delete (fpga_sync[i]);
		delete (fpga_freq[i]);
		delete (fpga_freq_diff[i]);
		delete (fpga_pulser_loss[i]);
		delete (asic_td[i]);
		delete (asic_sync[i]);
		delete (asic_freq[i]);
		delete (asic_freq_diff[i]);
		delete (asic_pulser_loss[i]);
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
	delete caen_freq;
	delete ebis_freq;
	delete t1_freq;

	return;

}

