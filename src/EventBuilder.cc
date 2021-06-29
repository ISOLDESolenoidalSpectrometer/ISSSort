#include "EventBuilder.hh"

EventBuilder::EventBuilder(){
	
	// ------------------------------------------------------------------------ //
	// Initialise variables and flags
	// ------------------------------------------------------------------------ //
	build_window = 3e3;
	
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

	array_ctr	= 0;
	recoil_ctr	= 0;
	elum_ctr	= 0;
	zd_ctr		= 0;
	
	// Some flags must be false to start
	flag_asic_pulser = false;
	flag_caen_pulser = false;

	
	 // p-side = 0; n-side = 1;
	array_side.push_back(0); // asic 0 = p-side
	array_side.push_back(1); // asic 1 = n-side
	array_side.push_back(0); // asic 2 = p-side
	array_side.push_back(0); // asic 3 = p-side
	array_side.push_back(1); // asic 4 = n-side
	array_side.push_back(0); // asic 5 = p-side

	array_row.push_back(0); // asic 0 = row 0 p-side
	array_row.push_back(0); // asic 1 = row 0 and 1 n-side
	array_row.push_back(1); // asic 2 = row 1 p-side
	array_row.push_back(2); // asic 3 = row 2 p-side
	array_row.push_back(2); // asic 4 = row 2 and 3 n-side
	array_row.push_back(3); // asic 5 = row 3 p-side
	
	
	// Loop over ASICs in a module
	for( int i = 0; i < common::n_asic; ++i ) {
		
		// Loop over channels in each ASIC
		for( int j = 0; j < common::n_channel; ++j ) {
			
			// p-side: all channels used
			if( array_side.at(i) == 0 ) {
				
				mystrip = j + common::n_pstrip * array_row.at(i);
				array_pid.push_back( mystrip );
				
			}
			
			// n-side: 11 channels per ASIC 0/2A
			else if( j >= 11 && j <= 21 ) {
				
				mystrip = j - 11;
				array_nid.push_back( mystrip );
				
			}
			
			// n-side: 11 channels per ASIC 0/2B
			else if( j >= 28 && j <= 38 ) {
				
				mystrip = 38 - j + common::n_nstrip;
				array_nid.push_back( mystrip );

			}
			
			// n-side: 11 channels per ASIC 1/3A
			else if( j >= 89 && j <= 99 ) {
				
				mystrip = j - 89 + common::n_nstrip;
				array_nid.push_back( mystrip );

			}
			
			// n-side: 11 channels per ASIC 1/3B
			else if( j >= 106 && j <= 116 ) {
				
				mystrip = 116 - j;
				array_nid.push_back( mystrip );

			}
			
			// n-side: empty channels
			else array_nid.push_back(-1);
	
		}
		
	}
	
}

EventBuilder::~EventBuilder(){}

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

	// ------------------------------------------------------------------------ //
	// Create output file and create events tree
	// ------------------------------------------------------------------------ //
	output_file = new TFile( output_file_name.data(), "recreate" );
	output_tree = new TTree( "evt_tree", "evt_tree" );
	output_tree->Branch( "ISSEvts", "ISSEvts", &write_evts );

	MakeEventHists();

}

void EventBuilder::Initialise(){

	/// This is called at the end of every execution/loop
	
	flag_close_event = false;

	hit_ctr = 0;
	
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
			myside = array_side.at( asic_data->GetAsic() );
			myrow = array_row.at( asic_data->GetAsic() );
			
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
			
		}

		// ------------------------------------------ //
		// Find recoils and other things
		// ------------------------------------------ //
		if( in_data->IsCaen() ) {
			
			// Increment event counter
			n_caen_data++;
			
			caen_data = in_data->GetCaenData();
					
		}
		
		
		// ------------------------------------------ //
		// Find info events, like timestamps etc
		// ------------------------------------------ //
		if( in_data->IsInfo() ) {
			
			// Increment event counter
			n_info_data++;
			
			info_data = in_data->GetInfoData();
			
			// Update EBIS time
			if( info_data->GetCode() == common::ebis_code ) {
				
				ebis_time = info_data->GetTime();
				ebis_hz = 1e9 / ( (double)ebis_time - (double)ebis_prev );
				if( ebis_prev != 0 ) ebis_freq->Fill( ebis_time, ebis_hz );
				ebis_prev = ebis_time;
				
			}
		
			// Update T1 time
			if( info_data->GetCode() == common::t1_code ){
				
				t1_time = info_data->GetTime();
				t1_hz = 1e9 / ( (double)t1_time - (double)t1_prev );
				if( t1_prev != 0 ) t1_freq->Fill( t1_time, t1_hz );
				t1_prev = t1_time;

			}
			
			// Update CAEN pulser time
			if( info_data->GetCode() == common::pulser_code ) {
				
				caen_time = info_data->GetTime();
				caen_hz = 1e9 / ( (double)caen_time - (double)caen_prev );
				if( caen_prev != 0 ) caen_freq->Fill( caen_time, caen_hz );

				flag_caen_pulser = true;
				n_caen_pulser++;
				caen_prev = caen_time;

			}

			// Update ISS pulser time
			if( info_data->GetCode() == common::extt_code ){
				
				asic_time = info_data->GetTime();
				asic_hz = 1e9 / ( (double)asic_time - (double)asic_prev );
				if( asic_prev != 0 ) asic_freq->Fill( asic_time, asic_hz );

				flag_asic_pulser = true;
				n_asic_pulser++;
				asic_prev = asic_time;

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
			output_tree->Fill();
			
			//--------------------------------------------------
			// clear values of arrays to store intermediate info
			//--------------------------------------------------
			Initialise();
			
		} // if close event && hit_ctr > 0
		
	} // End of main loop over TTree to process raw MIDAS data entries (for n_entries)
	
	//--------------------------
	// Clean up
	//--------------------------

	std::cout << "\n EventBuilder finished..." << std::endl;
	std::cout << "  ASIC data packets = " << n_asic_data << std::endl;
	std::cout << "  CAEN data packets = " << n_caen_data << std::endl;
	std::cout << "  Info data packets = " << n_info_data << std::endl;
	std::cout << "   Array events = " << array_ctr << std::endl;
	std::cout << "   Recoil events = " << recoil_ctr << std::endl;
	std::cout << "   ELUM events = " << elum_ctr << std::endl;
	std::cout << "   ZeroDegree events = " << zd_ctr << std::endl;
	std::cout << "  Tree entries = " << output_tree->GetEntries() << std::endl;

	output_file->Write( 0, TObject::kWriteDelete );
	//output_file->Print();
	//output_file->Close();

	return n_entries;
	
}


void EventBuilder::ArrayFinder() {
	
	std::vector<unsigned int> pindex;
	std::vector<unsigned int> nindex;

	// Do each module and row individually
	for( int i = 0; i < common::n_module; ++i ) {
		
		for( int j = 0; j < common::n_row; ++j ) {
			
			// Empty the array of indexes
			pindex.clear();
			nindex.clear();

			// Loop over p-side events
			for( unsigned int k = 0; k < pen_list.size(); ++k ) {
				
				// Check if it is the module and row we want
				if( pmod_list.at(k) == i && prow_list.at(k) == j ) {
					
					// Put in the index
					pindex.push_back( k );
					
				}
				
			}
			
			// Loop over n-side events
			for( unsigned int l = 0; l < nen_list.size(); ++l ) {
				
				// Check if it is the module and row we want
				if( nmod_list.at(l) == i && nrow_list.at(l) == j ) {
				
					// Put in the index
					nindex.push_back( l );

				}
				
			}
			
			// Multiplicty hist
			pn_mult[i][j]->Fill( pindex.size(), nindex.size() );

			// Time difference hists
			for( unsigned int k = 0; k < pindex.size(); ++k )
				for( unsigned int l = 0; l < nindex.size(); ++l )
					pn_td[i][j]->Fill( ptd_list.at( pindex.at(k) ) - ntd_list.at( pindex.at(j) ) );

			
			// Easy case, 1p vs 1n
			if( pindex.size() == 1 && nindex.size() == 1 ) {
				
				array_evt->SetEvent( pen_list.at( pindex.at(0) ),
									 nen_list.at( nindex.at(0) ),
									 pid_list.at( pindex.at(0) ),
									 nid_list.at( nindex.at(0) ),
									 ptd_list.at( pindex.at(0) ),
									 ntd_list.at( nindex.at(0) ),
									 i );
				
				write_evts->AddEvt( array_evt );
				array_ctr++;

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
						
		}

	}
	
}

void EventBuilder::RecoilFinder() {
	
	
}

void EventBuilder::ElumFinder() {
	
	
}

void EventBuilder::ZeroDegreeFinder() {
	
	
}

void EventBuilder::MakeEventHists(){
	
	std::string hname, htitle;
	std::string dirname, maindirname, subdirname;
	
	// Make directories
	maindirname = "array_hists";

	// ---------------- //
	// Array histograms //
	// ---------------- //
	// Loop over ISS modules
	for( unsigned int i = 0; i < common::n_module; ++i ) {
	
		dirname = maindirname + "/module_" + std::to_string(i);

		if( !output_file->GetDirectory( dirname.data() ) )
			output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );

		// Loop over rows of the array
		for( int j = 0; j < common::n_row; ++j ) {

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

	return;
	
}

