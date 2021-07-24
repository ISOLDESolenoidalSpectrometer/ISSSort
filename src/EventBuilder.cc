#include "EventBuilder.hh"

EventBuilder::EventBuilder( Settings *myset ){
	
	// First get the settings
	set = myset;
	
	// ------------------------------------------------------------------------ //
	// Initialise variables and flags
	// ------------------------------------------------------------------------ //
	build_window = set->GetEventWindow();
	
	time_prev = 0;
	asic_prev = 0;
	caen_prev = 0;
	ebis_prev = 0;
	t1_prev = 0;

	n_asic_data	= 0;
	n_caen_data	= 0;
	n_info_data	= 0;

	n_caen_pulser	= 0;
	n_asic_pulser	= 0;
	
	n_ebis	= 0;
	n_t1	= 0;

	array_ctr	= 0;
	recoil_ctr	= 0;
	elum_ctr	= 0;
	zd_ctr		= 0;
	
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {
	
		n_asic_pause.push_back( 0 );
		n_asic_resume.push_back( 0 );
		flag_pause.push_back( false );
		flag_resume.push_back( false );
		pause_time.push_back( 0 );
		resume_time.push_back( 0 );
		asic_dead_time.push_back( 0 );
		asic_time_start.push_back( 0 );
		asic_time_stop.push_back( 0 );

	}
	
	for( unsigned int i = 0; i < set->GetNumberOfCAENModules(); ++i ) {

		caen_time_start.push_back( 0 );
		caen_time_stop.push_back( 0 );

	}
	
	// Some flags must be false to start
	flag_asic_pulser = false;
	flag_caen_pulser = false;

	
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

void EventBuilder::SetInputFile( std::vector<std::string> input_file_names ) {
	
	/// Overlaoded function for a single file or multiple files
	input_tree = new TChain( "iss_sort" );
	for( unsigned int i = 0; i < input_file_names.size(); i++ ) {
	
		input_tree->Add( input_file_names[i].data() );
		
	}
	input_tree->SetBranchAddress( "data", &in_data );

	return;
	
}

void EventBuilder::SetInputTree( TTree* user_tree ){
	
	// Find the tree and set branch addresses
	input_tree = (TChain*)user_tree;
	input_tree->SetBranchAddress( "data", &in_data );

	return;
	
}

void EventBuilder::SetOutput( std::string output_file_name ) {

	// These are the branches we need
	write_evts = new ISSEvts();
	array_evt = new ArrayEvt();
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
		if( time_prev > mytime ) std::cout << "*t*";
			
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
			myenergy = asic_data->GetEnergy();
			mymod = asic_data->GetModule();
			myside = asic_side.at( asic_data->GetAsic() );
			myrow = array_row.at( asic_data->GetAsic() ).at( asic_data->GetChannel() );
			
			// p-side event
			if( myside == 0 ) {
				
				mystrip = array_pid.at( asic_data->GetChannel() );
				
				pen_list.push_back( myenergy );
				ptd_list.push_back( mytime - time_first );
				pmod_list.push_back( mymod );
				pid_list.push_back( mystrip );
				prow_list.push_back( myrow );

			}

			// n-side event
			else {
				
				mystrip = array_nid.at( asic_data->GetChannel() );
				
				nen_list.push_back( myenergy );
				ntd_list.push_back( mytime - time_first );
				nmod_list.push_back( mymod );
				nid_list.push_back( mystrip );
				nrow_list.push_back( myrow );

				
			}
			
			// Is it the start event?
			if( asic_time_start.at( mymod ) == 0 )
				asic_time_start.at( mymod ) = mytime;
			
			// or is it the end event (we don't know so keep updating
			asic_time_stop.at( mymod ) = mytime;


			if( mymod == 0 && asic_data->GetAsic() == 1 && asic_data->GetChannel() == 63 ){

				std::cout << mytime << std::endl;
				asic_test_time = mytime;
				asic_test_diff = (double)caen_time - (double)asic_test_time;
				carls_test->Fill( asic_test_time, asic_test_diff );

			}

			
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
			myenergy = caen_data->GetEnergy();
			
			// Is it a recoil
			if( set->IsRecoil( mymod, mych ) ) {
				
				mysector = set->GetRecoilSector( mymod, mych );
				mylayer = set->GetRecoilLayer( mymod, mych );
				
				ren_list.push_back( myenergy );
				rtd_list.push_back( mytime - time_first );
				rid_list.push_back( mylayer );
				rsec_list.push_back( mysector );

			}
			
			// Is it an ELUM?
			if( set->IsELUM( mymod, mych ) ) {
				
				mysector = set->GetELUMSector( mymod, mych );
				
				een_list.push_back( myenergy );
				etd_list.push_back( mytime - time_first );
				esec_list.push_back( mysector );

			}

			// Is it a ZeroDegree?
			if( set->IsZD( mymod, mych ) ) {
				
				mylayer = set->GetZDLayer( mymod, mych );
				
				zen_list.push_back( myenergy );
				ztd_list.push_back( mytime - time_first );
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
				caen_prev = caen_time;

			}

			// Update ISS pulser time - only for module 0
			if( info_data->GetCode() == set->GetExternalTriggerCode() &&
			    info_data->GetModule() == 0 ) {
			   
				asic_time = info_data->GetTime();
				asic_hz = 1e9 / ( (double)asic_time - (double)asic_prev );
				if( asic_prev != 0 ) asic_freq->Fill( asic_time, asic_hz );

				flag_asic_pulser = true;
				n_asic_pulser++;
				asic_prev = asic_time;

			}
			
			// Check the pause events for each module
			if( info_data->GetCode() == set->GetPauseCode() ) {
				
				if( info_data->GetModule() >= set->GetNumberOfArrayModules() ) {
				
					n_asic_pause[info_data->GetModule()]++;
					flag_pause[info_data->GetModule()] = true;
					pause_time[info_data->GetModule()] = info_data->GetTime();
				
				}
				
				else
					std::cerr << "Bad pause event in module " << info_data->GetModule() << std::endl;
				
			}
			
			// Check the resume events for each module
			if( info_data->GetCode() == set->GetResumeCode() ) {
				
				if( info_data->GetModule() >= set->GetNumberOfArrayModules() ) {
				
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
					std::cerr << "Bad resume event in module " << info_data->GetModule() << std::endl;
				
			}

			// If we a pulser event from both DAQs, fill time difference
			if( flag_caen_pulser && flag_asic_pulser ) {
				
				daq_sync_diff = (double)caen_time - (double)asic_time;
				
				daq_sync->Fill( asic_time, daq_sync_diff );
				freq_diff->Fill( asic_time, asic_hz - caen_hz );
				pulser_loss->Fill( asic_time, (int)n_asic_pulser - (int)n_caen_pulser );
				
				flag_asic_pulser = false;
				flag_caen_pulser = false;
	
			}
			
			
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
	std::cout << "   Array events = " << array_ctr << std::endl;
	std::cout << "   Recoil events = " << recoil_ctr << std::endl;
	std::cout << "   ELUM events = " << elum_ctr << std::endl;
	std::cout << "   ZeroDegree events = " << zd_ctr << std::endl;
	std::cout << "   CAEN pulser = " << n_caen_pulser << std::endl;
	std::cout << "   ASIC pulser = " << n_asic_pulser << std::endl;
	std::cout << "   EBIS events = " << n_ebis << std::endl;
	std::cout << "   T1 events = " << n_t1 << std::endl;
	std::cout << "  Tree entries = " << output_tree->GetEntries() << std::endl;

	output_file->Write( 0, TObject::kWriteDelete );
	//output_file->Print();
	//output_file->Close();

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
			pn_mult[i][j]->Fill( pindex.size(), nindex.size() );

			// Time difference hists
			for( unsigned int k = 0; k < pindex.size(); ++k )
				for( unsigned int l = 0; l < nindex.size(); ++l )
					pn_td[i][j]->Fill( ptd_list.at( pindex.at(k) ) - ntd_list.at( nindex.at(l) ) );

			
			// Easy case, 1p vs 1n
			if( pindex.size() == 1 && nindex.size() == 1 ) {
				
				// This works without neighbour mode
				//array_evt->SetEvent( pen_list.at( pindex.at(0) ),
				//					 nen_list.at( nindex.at(0) ),
				//					 pid_list.at( pindex.at(0) ),
				//					 nid_list.at( nindex.at(0) ),
				//					 ptd_list.at( pindex.at(0) ),
				//					 ntd_list.at( nindex.at(0) ),
				//					 i );
				//
				//write_evts->AddEvt( array_evt );
				//array_ctr++;

				pn_11[i][j]->Fill( pen_list.at( pindex.at(0) ), nen_list.at( nindex.at(0) ) );
				//write_evts->AddEvt( array_evt );
				//array_ctr++;
				
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
									 i );
				
				write_evts->AddEvt( array_evt );
				array_ctr++;

				pn_max[i][j]->Fill( pmax_en, nmax_en );
			
			}
						
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
			recoil_evt->SetTime( rtd_list[i] );
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
			zd_evt->SetTime( ztd_list[i] );
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
			pn_td[i][j] = new TH1F( hname.data(), htitle.data(), 10e3, -5e4, 5e4 );
			
			hname = "pn_mult_mod" + std::to_string(i) + "_row" + std::to_string(j);
			htitle = "p-side vs. n-side multiplicity (module ";
			htitle += std::to_string(i) + ", row " + std::to_string(j) + ");mult p-side;mult n-side";
			pn_mult[i][j] = new TProfile( hname.data(), htitle.data(), 20, 0, 20, 0, 20 );
		
		}
		
	}
	
	// ----------------- //
	// Timing histograms //
	// ----------------- //
	dirname =  "timing";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	tdiff = new TH1F( "tdiff", "Time difference to first trigger;#Delta t [ns]", 2e4+1, -1, 2e5 );
	daq_sync = new TProfile( "daq_sync", "Time difference between ASIC pulser and CAEN pulser as a function of time;time [ns];#Delta t [ns]", 10.8e3, 0, 10.8e12 );
	asic_freq = new TProfile( "asic_freq", "Frequency of pulser in ISS DAQ as a function of time;time [ns];f [Hz]", 10.8e3, 0, 10.8e12 );
	caen_freq = new TProfile( "caen_freq", "Frequency of pulser in CAEN DAQ as a function of time;time [ns];f [Hz]", 10.8e3, 0, 10.8e12 );
	ebis_freq = new TProfile( "ebis_freq", "Frequency of EBIS events as a function of time;time [ns];f [Hz]", 10.8e3, 0, 10.8e12 );
	t1_freq = new TProfile( "t1_freq", "Frequency of T1 events (p+ on ISOLDE target) as a function of time;time [ns];f [Hz]", 10.8e3, 0, 10.8e12 );
	freq_diff = new TProfile( "freq_diff", "Frequency difference of pulser events in ISS/CAEN DAQs as a function of time;time [ns];#Delta f [Hz]", 10.8e3, 0, 10.8e12 );
	pulser_loss = new TProfile( "pulser_loss", "Number of missing pulser events in ISS/CAEN DAQs as a function of time;time [ns];(+ive CAEN missing, -ive ISS missing)", 10.8e3, 0, 10.8e12 );
	carls_test = new TProfile( "carls_test", "Difference between ASIC pulser event generated by common pulser and CAEN info event;time [ns];#Delta t [ns]", 10.8e3, 0, 10.8e12 );

	
	// ----------------- //
	// Timing histograms //
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
		recoil_EdE[i] = new TH2F( hname.data(), htitle.data(), 2000, 0, 20000, 2000, 0, 20000 );
		
		hname = "recoil_dEsum" + std::to_string(i);
		htitle = "Recoil dE vs Esum for sector " + std::to_string(i);
		htitle += ";Total energy, Esum [keV];Energy loss, dE [keV];Counts";
		recoil_dEsum[i] = new TH2F( hname.data(), htitle.data(), 2000, 0, 20000, 2000, 0, 20000 );
		
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
	zd = new TH2F( hname.data(), htitle.data(), 2000, 0, 20000, 2000, 0, 20000 );

	return;
	
}

