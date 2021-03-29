#include "EventBuilder.hh"

EventBuilder::EventBuilder( Calibration *mycal,
						    std::vector<std::string> input_file_names,
						    std::string output_file_name ){
	
	// ------------------------------------------------------------------------ //
	// Initialise variables and flags
	// ------------------------------------------------------------------------ //
	cal = mycal;

	p_even_hits_pulser = 64;
	p_even_time_window = 3e3;
	
	time_prev = 0;
	n_events  = 0;
	n_cout = 0;
	
	
	// ------------------------------------------------------------------------ //
	// Initialise events and setup the input tree
	// ------------------------------------------------------------------------ //
	input_tree = new TChain( "iss_sort" );
	for( unsigned int i = 0; i < input_file_names.size(); i++ ) {
	
		input_tree->Add( input_file_names[i].data() );
		
	}
	input_tree->SetBranchAddress( "data", &in_data );
	n_entries = input_tree->GetEntries();


	// ------------------------------------------------------------------------ //
	// Create output file and create events tree
	// ------------------------------------------------------------------------ //
	output_file = new TFile( output_file_name.data(), "recreate" );
	output_tree = new TTree( "evt_tree", "evt_tree" );
	output_tree->Branch( "ISSEvts", "ISSEvts", &write_evts );

	
	// -------------------------------------------- //
	// Event lists for reconstruction
	// -------------------------------------------- //
	pen_list.resize( common::n_pairs );
	nen_list.resize( common::n_pairs );
	ptd_list.resize( common::n_pairs );
	ntd_list.resize( common::n_pairs );
	z_list.resize( common::n_pairs );
	phixy_list.resize( common::n_pairs );
	ren_list.resize( common::n_sector );
	rtd_list.resize( common::n_sector );
	rid_list.resize( common::n_sector );
	
	// Get ready and go
	Initialise();
	MakeEventHists();
	BuildEvents();

}

EventBuilder::~EventBuilder(){
			
}

void EventBuilder::Initialise(){

	/// This is called at the end of every execution/loop
	
	flag_close_event = false;

	hit_ctr		= 0;
	array_ctr	= 0;
	recoil_ctr	= 0;
	
	for( int i = 0; i < common::n_pairs; ++i ){
		
		pen_list.at(i).resize(0);
		nen_list.at(i).resize(0);
		ptd_list.at(i).resize(0);
		ntd_list.at(i).resize(0);
		z_list.at(i).resize(0);
		phixy_list.at(i).resize(0);
		
	}
	
	for( int i = 0; i < common::n_sector; ++i ){
		
		ren_list.at(i).resize(0);
		rtd_list.at(i).resize(0);
		rid_list.at(i).resize(0);

	}
	
	write_evts.Initialise();
	
	return;
	
}

void EventBuilder::BuildEvents() {
	
	/// Function to loop over the sort tree and build array and recoil events
	
	std::cout << " Event Building: number of entries in input tree = ";
	std::cout << n_entries << std::endl;

	
	// ------------------------------------------------------------------------ //
	// Main loop over TTree to find events
	// ------------------------------------------------------------------------ //
	for( unsigned long long i = 0; i < n_entries; ++i ) {
		
		// Current event data
		input_tree->GetEntry(i);
		mytime		= in_data.time;
		myenergy	= in_data.energy;
		mydet		= in_data.det;
		mylayer		= in_data.layer;
		mysector	= in_data.sector;
		mystrip		= in_data.strip;
		myside		= in_data.side;

		// check time stamp monotonically increases!
		if( time_prev > mytime ) std::cout << "*t*";
			

		// if this is first datum included in Event
		if( hit_ctr == 0 ) {
			
			time_min	= mytime;
			time_max	= mytime;
			time_first	= mytime;
			
		}
		hit_ctr++; // increase counter for bits of data included in this event


		// ------------------------------------------ //
		// Find particles on the array
		// ------------------------------------------ //
		if( mydet == 0 ) {
			
			mypair = mysector/2;
			mypair *= common::n_layer;
			mypair += mylayer;
			
			if( myside == 0 && mypair < common::n_pairs ) { // p-side
			
				pen_list.at(mypair).push_back( myenergy );
				ptd_list.at(mypair).push_back( mytime - time_first );
				z_list.at(mypair).push_back( GetZ( mylayer, mystrip ) );
				
			}
			
			else if( myside == 1 && mypair < common::n_pairs ) { // n-side
			
				nen_list.at(mypair).push_back( myenergy );
				ntd_list.at(mypair).push_back( mytime - time_first );
				phixy_list.at(mypair).push_back( GetPhiXY( mysector, mystrip ) );

				
			}
			
		}

		// ------------------------------------------ //
		// Find recoils
		// ------------------------------------------ //
		if( mydet == 1 ) {
			
			ren_list.at(mysector).push_back( myenergy );
			rtd_list.at(mysector).push_back( mytime - time_first );
			rid_list.at(mysector).push_back( mylayer );
			
		}
				
		// record time of this events
		time_prev = mytime;
		
		// Update min and max
		if( mytime > time_max ) time_max = mytime;
		else if( mytime < time_min ) time_min = mytime;

		
		//------------------------------
		//  check if last datum from this event and do some cleanup
		//------------------------------
		
		if( (i+1) == n_entries )
			flag_close_event = true; // set flag to close this event
			
		else {  //check if next entry is beyond time window: close event!

			input_tree->GetEntry(i+1);
			
			time_diff = in_data.time - time_first;
			tdiff->Fill( time_diff );
			
			// window = time_stamp_first + time_window
			if( time_diff > p_even_time_window )
				flag_close_event = true; // set flag to close this event

			// we've gone on to the next file in the chain
			else if( time_diff < 0 )
				flag_close_event = true; // set flag to close this event

		} // if next entry beyond time window: close event!
		
		
		//----------------------------
		// if close this event and number of datums in event>0
		//----------------------------
		if( flag_close_event && hit_ctr > 0 ) {

			//----------------------------------
			// Build array events and recoils
			//----------------------------------
			RecoilFinder();		// same recoils for all array particles
			ParticleFinder();	// every array particle will fill one event
		
			//----------------------------------
			// clear values of arrays to store intermediate info
			//----------------------------------
			n_events++;
			Initialise();
			
		} // if close event && hit_ctr > 0
		
	} // End of main loop over TTree to process raw MIDAS data entries (for n_entries)
	
	//--------------------------
	// Clean up
	//--------------------------

	std::cout << "\n EventBuilder finished with " << n_events;
	std::cout << " events and " << output_tree->GetEntries();
	std::cout << " tree entries" << std::endl;

	output_file->Write();
	//output_file->Print();
	output_file->Close();

	return;
	
}

float EventBuilder::GetZ( int layer, int strip ){
	
	/// Get the z position of the interaction
	/// Note that there is no radial correction yet implemented
	/// the origin is at the target position
	/// z is positive in the beam direction
	/// x is positive in the vertical direction towards the sky
	/// y is positive in the horizontal direction towards XT03 (right)
	/// phi is positive in the clockwise direction, looking from the origin to positive z (beam direction)

	//float z = Cal->GetTargetDist(); // not yet implemented
	float z = -10.0; 			// just until it is in the cal file
	float d = 127.5 - strip;	// take centre of the end strip
	d *= 0.95;					// p-side strip pitch = 0.95 mm
	d += (3-layer) * 125.5;		// inter wafer distance (to be confirmed)
	d += 1.7;					// distance from wafer edge to active region

	if( z > 0 ) z += d;	// forward direction (downstream)
	else z -= d;		// backward direction (upstream)
		
	return z; // in mm
	
}

TVector2 EventBuilder::GetPhiXY( int sector, int strip ){
	
	/// Get the phi angle of the interaction, and in the meantime define x,y too
	/// Note that there is no radial correction yet implemented
	/// the origin is at the target position
	/// z is positive in the beam direction
	/// x is positive in the vertical direction towards the sky
	/// y is positive in the horizontal direction towards XT03 (right)
	/// phi is positive in the clockwise direction, looking from the origin to positive z (beam direction)
	
	// Start with a flat wafer pointing to the sky
	float x = 53.5 / 2.0; 	// diameter is 53.5 mm
	float y = strip - 4.5;	// take strip relative to centre (11 strips)
	y *= 2.0;				// n-side strip pitch = 2.0 mm

	// This vector can now be rotated to the correct position
	TVector2 vec( x, y );
	
	vec = vec.Rotate( TMath::Pi() / 6. );			// start in the centre of first face
	vec = vec.Rotate( sector * TMath::Pi() / 3. );	// get side of hexagon
	
	return vec;

}

void EventBuilder::ParticleFinder() {
	
	bool good_event;
	
	// Do each layer individually
	for( int i = 0; i < common::n_pairs; ++i ) {
		
		good_event = false;
		
		pn_mult[i]->Fill( ptd_list.at(i).size(), ntd_list.at(i).size() );
		
		for( unsigned int j = 0; j < ptd_list.at(i).size(); ++j ) {

			for( unsigned int k = 0; k < ntd_list.at(i).size(); ++k ) {

				pn_td[i]->Fill( ptd_list.at(i).at(j) - ntd_list.at(i).at(k) );
				
			}

		}

		// Easy case, 1p vs 1n
		if( pen_list.at(i).size() == 1 &&
		    nen_list.at(i).size() == 1 ) {

			write_evts.SetParticle( pen_list.at(i).at(0),
									nen_list.at(i).at(0),
									z_list.at(i).at(0),
									phixy_list.at(i).at(0),
									ptd_list.at(i).at(0),
									ntd_list.at(i).at(0) );

			good_event = true;
			
			pn_11[i]->Fill( pen_list.at(i).at(0), nen_list.at(i).at(0) );
			
		}
		
		// Others too, but who can be arsed at the moment
		else if( pen_list.at(i).size() == 2 &&
				 nen_list.at(i).size() == 1 ) {
			
			pn_21[i]->Fill( pen_list.at(i).at(0), nen_list.at(i).at(0) );
			pn_21[i]->Fill( pen_list.at(i).at(1), nen_list.at(i).at(0) );

		}
		
		else if( pen_list.at(i).size() == 1 &&
				 nen_list.at(i).size() == 2 ) {
			
			pn_12[i]->Fill( pen_list.at(i).at(0), nen_list.at(i).at(0) );
			pn_12[i]->Fill( pen_list.at(i).at(0), nen_list.at(i).at(1) );

		}
		
		else if( pen_list.at(i).size() == 2 &&
				 nen_list.at(i).size() == 2 ) {
		
			pn_22[i]->Fill( pen_list.at(i).at(0), nen_list.at(i).at(0) );
			pn_22[i]->Fill( pen_list.at(i).at(1), nen_list.at(i).at(0) );
			pn_22[i]->Fill( pen_list.at(i).at(0), nen_list.at(i).at(1) );
			pn_22[i]->Fill( pen_list.at(i).at(1), nen_list.at(i).at(1) );
			
		}
		
		// Fill one particle per layer
		if( good_event ) output_tree->Fill();

	}
	
}

void EventBuilder::RecoilFinder() {
	
	
}

void EventBuilder::MakeEventHists(){
	
	std::string hname, htitle;
	for( unsigned int i = 0; i < common::n_pairs; ++i ) {
		
		hname = "pn_1v1_s" + std::to_string(i);
		htitle = "p-side multiplicity = 1 vs. n-side multiplicity = 1 (DSSSD pair ";
		htitle += std::to_string(i) + ");p-side energy [keV];n-side energy [keV]";
		pn_11[i] = new TH2F( hname.data(), htitle.data(), 2e3, 0, 2e4, 2e3, 0, 2e4 );

		hname = "pn_1v2_s" + std::to_string(i);
		htitle = "p-side multiplicity = 1 vs. n-side multiplicity = 2 (DSSSD pair ";
		htitle += std::to_string(i) + ");p-side energy [keV];n-side energy [keV]";
		pn_12[i] = new TH2F( hname.data(), htitle.data(), 2e3, 0, 2e4, 2e3, 0, 2e4 );

		hname = "pn_2v1_s" + std::to_string(i);
		htitle = "p-side multiplicity = 2 vs. n-side multiplicity = 1 (DSSSD pair ";
		htitle += std::to_string(i) + ");p-side energy [keV];n-side energy [keV]";
		pn_21[i] = new TH2F( hname.data(), htitle.data(), 2e3, 0, 2e4, 2e3, 0, 2e4 );

		hname = "pn_2v2_s" + std::to_string(i);
		htitle = "p-side multiplicity = 2 vs. n-side multiplicity = 2 (DSSSD pair ";
		htitle += std::to_string(i) + ");p-side energy [keV];n-side energy [keV]";
		pn_22[i] = new TH2F( hname.data(), htitle.data(), 2e3, 0, 2e4, 2e3, 0, 2e4 );
		
		hname = "pn_td_s" + std::to_string(i);
		htitle = "p-side vs. n-side time difference (DSSSD pair ";
		htitle += std::to_string(i) + ");time difference [ns];counts";
		pn_td[i] = new TH1F( hname.data(), htitle.data(), 10e3, -5e4, 5e4 );
		
		hname = "pn_mult_s" + std::to_string(i);
		htitle = "p-side vs. n-side multiplicity (DSSSD pair ";
		htitle += std::to_string(i) + ");mult p-side;mult n-side";
		pn_mult[i] = new TProfile( hname.data(), htitle.data(), 20, 0, 20, 0, 20 );
		
	}
	
	tdiff = new TH1F( "tdiff", "Time difference to first trigger;#Delta t [ns]", 2e4+1, -1, 2e5 );
	
	return;
	
}

