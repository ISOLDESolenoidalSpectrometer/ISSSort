#include "Calibrator.hh"

Calibrator::Calibrator( Calibration *mycal ){
	
	cal = mycal;
	
	LoadParametersCalib(); //maybe also file to read param from file
	
}

Calibrator::~Calibrator() {
	
	//std::cout << "destructor" << std::endl;

}

void Calibrator::Initialise(){
	
	// Start counters at zero
	for( unsigned int i = 0; i < common::n_module; ++i ) {
		
		my_tm_stp[i] 			= 0;
		my_tm_stp_ext[i]		= 0;
		my_tm_stp_sync[i]		= 0;
		tm_stp_msb_modules[i]	= 0;	// medium significant bits
		tm_stp_hsb_modules[i]	= 0;	// highest significant bits

		ctr_hit[i]				= 0;	// hits on each module
		ctr_ext[i]				= 0;	// external timestamps
		ctr_sync[i]				= 0;	// sync timestamps

	}
	
	// Flags
	ext_flag = false;
	sync_flag = false;
	hsb_ready = false;

	// Reset Timestamps
	t0_sync = 0;
	time( &t_start );

	return;
	
}

bool Calibrator::SetInputFile( std::string input_file_name ){
	
	// Open next Root input file.
	input_file = new TFile( input_file_name.data(), "read" );
	if( input_file->IsZombie() ) {
		
		std::cout << "Cannot open " << input_file_name << std::endl;
		return false;
		
	}

	// Set the input tree
	SetInputTree( (TTree*)input_file->Get("iss") );

	std::cout << "Calibrating file: " << input_file->GetName() << std::endl;

	return true;
	
}

void Calibrator::SetInputTree( TTree* user_tree ){
	
	// Find the tree and set branch addresses
	input_tree = user_tree;
	//input_tree->SetBranchAddress( "info_data", &s_info );
	//input_tree->SetBranchAddress( "event_id",  &s_id   );
	//input_tree->SetBranchAddress( "adc_data",  &s_adc  );
	input_tree->SetBranchAddress( "data",  &data_packet  );

	return;
	
}

void Calibrator::SetOutput( std::string output_file_name ){

	// Create output Root file and Tree.
	output_file = new TFile( output_file_name.data(), "recreate", "ISS calibrated data", 0 );
	output_file->cd();
	output_tree = new TTree( "iss_calib", "Calibrated ISS data" );
	output_tree->Branch( "data", &s_data, "t_ext/l:time/l:energy/F:hit/b:det/b:layer/b:sector/b:strip/b:side/b" );
	output_tree->SetDirectory( output_file );

	// Create log file.
	std::string log_file_name = output_file_name.substr( 0, output_file_name.find_last_of(".") );
	log_file_name += ".log";
	log_file.open( log_file_name.c_str(), std::ios::out );
	
	// Make histograms too
	MakeHists();

	return;
	
}

bool Calibrator::SetEntry( long long ts ) {
	
	float my_energy	= -999;
	int my_det 		= 255;
	int my_layer	= 255;
	int my_sector	= 255;
	int my_strip	= 255;
	int my_side		= 255;
	int my_hit		= 2;

	bool my_sync_flag	= false;
	bool my_ext_flag	= false;
	
	bool fill_flag = false;
	
	
	// if INFO CODE data
	if( s_info.type == 2 ){
		
		switch( s_info.code ) {
			
			// look for SYNC-like pulses
			case common::sync_code:

				my_sync_flag = true;
				fill_flag = false; // to skip filling the iss_calib tree with SYNC100 pulses

			// external triggers, this is the sync to the CAEN system
			case common::extt_code:

				fill_flag = true; // we need to fill these
				my_ext_flag = true;

			default:
				
				// skip everything else!
				fill_flag = false;
				std::cout << "Unknown info code = " << s_info.code << std::endl;
				
		}
		
	}
	
	// ADC data
	else if( s_info.type == 3 ) {
		
		fill_flag = true;
		
		// Set various IDs
		my_energy	= cal->AsicEnergy( s_id.mod,
						s_id.asic, s_id.ch, s_adc.value );
		my_hit		= s_adc.hit;
		my_det 		= p_det_type[s_id.mod][s_id.asic][s_id.ch];
		my_layer	= p_layer_id[s_id.mod][s_id.asic][s_id.ch];
		my_sector	= p_sector_id[s_id.mod][s_id.asic][s_id.ch];
		my_strip	= p_strip_id[s_id.mod][s_id.asic][s_id.ch];
		my_side		= p_side_id[s_id.mod][s_id.asic][s_id.ch];

	}
	
	// Real data
	s_data.time			= ts;
	s_data.energy		= my_energy;
	s_data.hit			= my_hit;
	s_data.det 			= my_det;
	s_data.layer		= my_layer;
	s_data.sector		= my_sector;
	s_data.strip		= my_strip;
	s_data.side			= my_side;

	// Check for disabled channels
	if( !p_ch_enable[s_id.mod][s_id.asic][s_id.ch] ) fill_flag = false;

	return fill_flag;
	
}


void Calibrator::LoadParametersCalib() { // maybe also file to read param from file
	
	// p-side = 0; n-side = 1;
	int array_side[common::n_asic]  = { 0, 1, 0, 0, 1, 0 };
	int array_layer[common::n_asic] = { 0, 0, 1, 2, 2, 3 };

	// Loop over modules
	for( int i = 0; i < common::n_module; ++i ) {
		
		// Deal with the array, i.e. modules 0-2
		if( i < 3 ) {
			
			// Loop over ASICs for the array
			for( int j = 0; j < common::n_asic; ++j ) {
				
				// Loop over channels of each ASIC
				for( int k = 0; k < common::n_channel; ++k ) {
					
					p_det_type[i][j][k]		= 0;
					p_side_id[i][j][k]		= array_side[j];
					p_sector_id[i][j][k]	= i*2;
					p_strip_id[i][j][k]		= k;
					p_layer_id[i][j][k]		= array_layer[j];

					// p-side: all channels used
					if( array_side[j] == 0 )
						p_ch_enable[i][j][k] = true;

					// n-side: 11 channels per ASIC 0/2A
					else if( k >= 11 && k <= 21 ) {
						
						p_ch_enable[i][j][k] = true;
						p_strip_id[i][j][k] = k - 11;

					}
					
					// n-side: 11 channels per ASIC 0/2B
					else if( k >= 28 && k <= 38 ) {
						
						p_ch_enable[i][j][k] = true;
						p_sector_id[i][j][k]++;
						p_strip_id[i][j][k] = 38 - k;

					}
					
					// n-side: 11 channels per ASIC 1/3A
					else if( k >= 89 && k <= 99 ) {
						
						p_ch_enable[i][j][k] = true;
						p_layer_id[i][j][k]++;
						p_sector_id[i][j][k]++;
						p_strip_id[i][j][k] = 99 - k;

					}
					
					// n-side: 11 channels per ASIC 1/3B
					else if( k >= 106 && k <= 116 ) {
						
						p_ch_enable[i][j][k] = true;
						p_layer_id[i][j][k]++;
						p_strip_id[i][j][k] = k - 106;

					}

					// ignore the empty channels
					else {
						
						p_ch_enable[i][j][k]	= false;
						p_det_type[i][j][k]		= 0;
						p_side_id[i][j][k]		= 255;
						p_sector_id[i][j][k]	= 255;
						p_strip_id[i][j][k]		= 255;
						p_layer_id[i][j][k]		= 255;

					}
					
				} // k
				
			} // j
			
		} // module < 3, i.e. array
		
		// Will this be the CAEN digitiser?
		//else if( module == 4 ){}
		
	} // i
	
	return;
	
}

void Calibrator::MakeHists(){
	
	//---- declare histograms here!  ----//
	std::string hname, htitle;
	for( unsigned int i = 0; i < common::n_module; ++i ) {
		
		hname = "hprof" + std::to_string(i);
		htitle = "Profile of ts versus hit_id in module " + std::to_string(i);
		hprof[i] = new TProfile( hname.data(), htitle.data(), 3001 , 0., 3000., 2.60e14, 3.61e14 );
	
		hname = "hprofExt" + std::to_string(i);
		htitle = "Profile of ts versus hit_id in module " + std::to_string(i);
		hprofExt[i] = new TProfile( hname.data(), htitle.data(), 3001 , 0., 3000., 2.60e14, 3.61e14 );

		hname = "hprofSync" + std::to_string(i);
		htitle = "Profile of ts versus hit_id in module " + std::to_string(i);
		hprofSync[i] = new TProfile( hname.data(), htitle.data(), 3001 , 0., 3000., 2.60e14, 3.61e14 );
	
	}

	return;
	
}

unsigned long Calibrator::CalibFile( unsigned long start_entry ) {

	// Get ready to calibrate
	Initialise();

	n_entries = input_tree->GetEntries();
	std::cout << " Reading raw data: number of entries in input tree = " << n_entries << std::endl;
	log_file << " Reading raw data: number of entries in input tree = " << n_entries << std::endl;
	std::cout << " Starting the calibration at entry = " << start_entry << std::endl;
	log_file << " Starting the calibration at entry = " << start_entry << std::endl;

	
	// Main loop over TTree to process raw MIDAS data entries
	for( unsigned long i = start_entry; i < n_entries; i++ ){
		
		input_tree->GetEntry(i);
		//std::cout << s_info.tm_stp_lsb << std::endl;
		//std::cout << s_id.asic << std::endl;

		ts_flag = false;
		sync_flag = false;
		ext_flag = false;

		// if INFO code
		if( s_info.type == 3 ) ctr_hit[s_id.mod]++;
		
		if( s_info.type == 2 ){
			
			// If SYNC pulse or external timestamp
			if( s_info.code == common::sync_code || s_info.code == common::extt_code ) {

				my_tm_stp_msb = (s_info.field & 0x000FFFFF);	// MS bits (47:28) of timestamp
				tm_stp_msb_modules[s_id.mod] = my_tm_stp_msb;	// update for use with other data types
				
				// reconstruct time stamp= MSB+LSB
				my_tm_stp[s_id.mod] = (tm_stp_msb_modules[s_id.mod] << 28 ) | (s_info.tm_stp_lsb & 0x0FFFFFFF);

				if( s_info.code == common::sync_code ) {
					
					ctr_ext[s_id.mod]++;
					my_tm_stp_sync[s_id.mod] = my_tm_stp[s_id.mod];
					sync_flag = true;
					ts_flag = true;

				}
				
				if( s_info.code == common::extt_code ) {
					
					ctr_sync[s_id.mod]++;
					my_tm_stp_ext[s_id.mod] = my_tm_stp[s_id.mod];
					ext_flag = true;
					ts_flag = true;

				}

			}
			
			// Highest significant bits in code 5
			// We need to get code 4 next and alert that hsb are ready
			if( s_info.code == common::thsb_code ){
				
				my_tm_stp_hsb = ( s_info.field & 0x000FFFFF); // MS bits (47:28) of timestamp
				tm_stp_hsb_modules[s_id.mod] = my_tm_stp_hsb;
				ts_flag = false;
				hsb_ready = true;
				
			}
			
		}
		
		// if timestamp is complete (after we get sync code)
		if( ts_flag ){
			
			// reconstruct time stamp= HSB+MSB+LSB
			if( hsb_ready )
				my_tm_stp[s_id.mod] = ( tm_stp_hsb_modules[s_id.mod] << 48 ) | ( tm_stp_msb_modules[s_id.mod] << 28 ) | ( s_info.tm_stp_lsb & 0x0FFFFFFF );

			// reconstruct time stamp= MSB+LSB
			else
				my_tm_stp[s_id.mod] = ( tm_stp_msb_modules[s_id.mod] << 28 ) | ( s_info.tm_stp_lsb & 0x0FFFFFFF );
				
		}
		
		else continue;
		
		if( SetEntry( my_tm_stp[s_id.mod] ) ) {
			
			//std::cout << s_info.tm_stp_lsb << std::endl;
			
			output_tree->Fill();
			
			hprof[s_id.mod]->Fill( ctr_hit[s_id.mod], my_tm_stp[s_id.mod], 1 );
			hprofExt[s_id.mod]->Fill( ctr_ext[s_id.mod], my_tm_stp_ext[s_id.mod], 1 );
			hprofSync[s_id.mod]->Fill( ctr_sync[s_id.mod], my_tm_stp_sync[s_id.mod], 1 );
			
		}

	} // End of main loop over TTree to process raw MIDAS data entries
	

	// Write histograms, trees and clean up
	output_file->cd();
	output_file->Write();
	//output_file->Print();
	
	// Print some stats
	for( unsigned int i = 0; i < common::n_module; ++i ) {
		
		log_file << " Number of hit in module #" << i << ": " << ctr_hit[i] << "" << std::endl;
		log_file << "   Nb of ext:  " << ctr_ext[i] << std::endl;
		log_file << "   Nb of sync: " << ctr_sync[i] << std::endl;
			
		// Clean histograms
		//delete hprof[i];
		//delete hprofExt[i];
		//delete hprofSync[i];

	}
	
	std::cout << "End Calibrator: time elapsed = " << time(NULL)-t_start << " sec." << std::endl;
	log_file << "End Calibrator: time elapsed = " << time(NULL)-t_start << " sec." << std::endl;
		
	return n_entries;
	
}
