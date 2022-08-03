#include "Converter.hh"

ISSConverter::ISSConverter( ISSSettings *myset ) {

	// We need to do initialise, but only after Settings are added
	set = myset;

	my_tm_stp_msb = 0;
	my_tm_stp_hsb = 0;
	
	// Start counters at zero
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {
				
		ctr_asic_hit.push_back(0);	// hits on each module
		ctr_asic_ext.push_back(0);	// external timestamps
		ctr_asic_pause.push_back(0);
		ctr_asic_resume.push_back(0);

	}
	
	for( unsigned int i = 0; i < set->GetNumberOfCAENModules(); ++i ) {
				
		ctr_caen_hit.push_back(0);	// hits on each module
		ctr_caen_ext.push_back(0);	// external timestamps

	}
	
	// Default that we do not have a source only run
	flag_source = false;
	
	// No progress bar by default
	_prog_ = false;

}

void ISSConverter::SetOutput( std::string output_file_name ){

	// Open output file
	output_file = new TFile( output_file_name.data(), "recreate" );
	//if( !flag_source ) output_file->SetCompressionLevel(0);

	return;

};


void ISSConverter::MakeTree() {

	// Create Root tree
	const int splitLevel = 2; // don't split branches = 0, full splitting = 99
	const int bufsize = sizeof(ISSCaenData) + sizeof(ISSAsicData) + sizeof(ISSInfoData);
	output_tree = new TTree( "iss", "iss" );
	data_packet = std::make_unique<ISSDataPackets>();
	output_tree->Branch( "data", "ISSDataPackets", data_packet.get(), bufsize, splitLevel );

	sorted_tree = (TTree*)output_tree->CloneTree(0);
	sorted_tree->SetName("iss_sort");
	sorted_tree->SetTitle( "Time sorted, calibrated ISS data" );
	sorted_tree->SetDirectory( output_file->GetDirectory("/") );
	output_tree->SetDirectory( output_file->GetDirectory("/") );
	
	output_tree->SetAutoFlush(-1e9);
	sorted_tree->SetAutoFlush(-1e9);

	asic_data = std::make_shared<ISSAsicData>();
	caen_data = std::make_shared<ISSCaenData>();
	info_data = std::make_shared<ISSInfoData>();

	asic_data->ClearData();
	caen_data->ClearData();
	info_data->ClearData();
	
	return;
	
}

void ISSConverter::MakeHists() {
	
	std::string hname, htitle;
	std::string dirname, maindirname, subdirname;
	
	// Make directories
	maindirname = "asic_hists";

	// Resize vectors
	hasic.resize( set->GetNumberOfArrayModules() );
	hasic_cal.resize( set->GetNumberOfArrayModules() );
	hpside.resize( set->GetNumberOfArrayModules() );
	hnside.resize( set->GetNumberOfArrayModules() );
	
	// Loop over ISS modules
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {
		
		hasic[i].resize( set->GetNumberOfArrayASICs() );
		hasic_cal[i].resize( set->GetNumberOfArrayASICs() );
		subdirname = "/module_" + std::to_string(i);
		dirname = maindirname + subdirname;
		
		if( !output_file->GetDirectory( dirname.data() ) )
			output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );
			
		// calibrated p-side sum
		hname = "pside_mod" + std::to_string(i);
		htitle = "Calibrated p-side ASIC spectra for module " + std::to_string(i);
		htitle += ";Energy (keV);Counts per 15 keV";
		
		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hpside[i] = (TH1F*)output_file->Get( hname.data() );
		
		else {
			
			hpside[i] = new TH1F( hname.data(), htitle.data(),
						1500, -7.5, 29992.5 );
			hpside[i]->SetDirectory(
					output_file->GetDirectory( dirname.data() ) );
			
		}

		// calibrated n-side sum
		hname = "nside_mod" + std::to_string(i);
		htitle = "Calibrated n-side ASIC spectra for module " + std::to_string(i);
		htitle += ";Energy (keV);Counts per 15 keV";
		
		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hnside[i] = (TH1F*)output_file->Get( hname.data() );
		
		else {
			
			hnside[i] = new TH1F( hname.data(), htitle.data(),
						1500, -7.5, 29992.5 );
			hnside[i]->SetDirectory(
					output_file->GetDirectory( dirname.data() ) );
			
		}

		// Loop over ASICs for the array
		for( unsigned int j = 0; j < set->GetNumberOfArrayASICs(); ++j ) {
			
			if( !output_file->GetDirectory( dirname.data() ) )
				output_file->mkdir( dirname.data() );
			output_file->cd( dirname.data() );
				
			// Uncalibrated
			hname = "asic_" + std::to_string(i);
			hname += "_" + std::to_string(j);
				
			htitle = "Raw ASIC spectra for module " + std::to_string(i);
			htitle += ", ASIC " + std::to_string(j);
			
			htitle += ";Channel;ADC value;Counts";
			
			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hasic[i][j] = (TH2F*)output_file->Get( hname.data() );
				
			else {
					
				hasic[i][j] = new TH2F( hname.data(), htitle.data(),
							set->GetNumberOfArrayChannels(), -0.5, set->GetNumberOfArrayChannels()-0.5,
							4096, -0.5, 4095.5 );
				hasic[i][j]->SetDirectory(
						output_file->GetDirectory( dirname.data() ) );
					
			}
			
			// Calibrated
			hname = "asic_" + std::to_string(i);
			hname += "_" + std::to_string(j);
			hname += "_cal";
				
			htitle = "Calibrated ASIC spectra for module " + std::to_string(i);
			htitle += ", ASIC " + std::to_string(j);
			
			htitle += ";Channel;Energy (keV);Counts per 15 keV";
			
			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hasic_cal[i][j] = (TH2F*)output_file->Get( hname.data() );
				
			else {
					
				hasic_cal[i][j] = new TH2F( hname.data(), htitle.data(),
							set->GetNumberOfArrayChannels(), -0.5, set->GetNumberOfArrayChannels()-0.5,
							1500, -7.5, 29992.5 );
				hasic_cal[i][j]->SetDirectory(
						output_file->GetDirectory( dirname.data() ) );
					
			}

				
		}
		
	}
	
	// Make directories
	maindirname = "caen_hists";
	
	// Resive vectors
	hcaen.resize( set->GetNumberOfCAENModules() );
	hcaen_cal.resize( set->GetNumberOfCAENModules() );

	// Loop over CAEN modules
	for( unsigned int i = 0; i < set->GetNumberOfCAENModules(); ++i ) {
		
		hcaen[i].resize( set->GetNumberOfCAENChannels() );
		hcaen_cal[i].resize( set->GetNumberOfCAENChannels() );
		dirname = maindirname + "/module_" + std::to_string(i);
		
		if( !output_file->GetDirectory( dirname.data() ) )
			output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );

		// Loop over channels of each CAEN module
		for( unsigned int j = 0; j < set->GetNumberOfCAENChannels(); ++j ) {
			
			// Uncalibrated
			hname = "caen_" + std::to_string(i);
			hname += "_" + std::to_string(j);
			
			htitle = "Raw CAEN V1725 spectra for module " + std::to_string(i);
			htitle += ", channel " + std::to_string(j);
			
			htitle += ";Qlong;Counts";
			
			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hcaen[i][j] = (TH1F*)output_file->Get( hname.data() );
			
			else {
				
				hcaen[i][j] = new TH1F( hname.data(), htitle.data(),
										   65536, -0.5, 65535.5 );
			
				hcaen[i][j]->SetDirectory(
						output_file->GetDirectory( dirname.data() ) );
				
			}
			
			// Calibrated
			hname = "caen_" + std::to_string(i);
			hname += "_" + std::to_string(j);
			hname += "_cal";
			
			htitle = "Calibrated CAEN V1725 spectra for module " + std::to_string(i);
			htitle += ", channel " + std::to_string(j);
			
			htitle += ";Energy (keV);Counts per 10 keV";
			
			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hcaen_cal[i][j] = (TH1F*)output_file->Get( hname.data() );
			
			else {
				
				hcaen_cal[i][j] = new TH1F( hname.data(), htitle.data(),
										   4000, -5, 39995 );
			
				hcaen_cal[i][j]->SetDirectory(
						output_file->GetDirectory( dirname.data() ) );
				
			}

			
		}
					
	}
	
	// Make directories
	dirname = "timing_hists";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );
	
	// Energy histogram of pulser channel
	if( output_file->GetListOfKeys()->Contains( "asic_pulser_energy" ) )
		asic_pulser_energy = (TH1F*)output_file->Get( "asic_pulser_energy" );
	
	else {
		
		asic_pulser_energy = new TH1F( "asic_pulser_energy",
									  "ASIC energy for pulser event;ADC value;counts",
								   4096, -0.5, 4095.5 );
	
		asic_pulser_energy->SetDirectory(
				output_file->GetDirectory( dirname.data() ) );
		
	}


	// Resize vectors
	hasic_hit.resize( set->GetNumberOfArrayModules() );
	hasic_ext.resize( set->GetNumberOfArrayModules() );
	hasic_pause.resize( set->GetNumberOfArrayModules() );
	hasic_resume.resize( set->GetNumberOfArrayModules() );
	hcaen_hit.resize( set->GetNumberOfCAENModules() );
	hcaen_ext.resize( set->GetNumberOfCAENModules() );
	
	// Loop over ISS modules
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {
		
		hname = "hasic_hit" + std::to_string(i);
		htitle = "Profile of ts versus hit_id in ISS module " + std::to_string(i);

		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hasic_hit[i] = (TProfile*)output_file->Get( hname.data() );

		else {

			hasic_hit[i] = new TProfile( hname.data(), htitle.data(), 10800, 0., 108000. );
			hasic_hit[i]->SetDirectory(
					output_file->GetDirectory( dirname.data() ) );

		}
		
		hname = "hasic_ext" + std::to_string(i);
		htitle = "Profile of external trigger ts versus hit_id in ISS module " + std::to_string(i);

		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hasic_ext[i] = (TProfile*)output_file->Get( hname.data() );

		else {

			hasic_ext[i] = new TProfile( hname.data(), htitle.data(), 10800, 0., 108000. );
			hasic_ext[i]->SetDirectory(
					output_file->GetDirectory( dirname.data() ) );

		}

		hname = "hasic_pause" + std::to_string(i);
		htitle = "Profile of ts versus pause events in ISS module " + std::to_string(i);

		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hasic_pause[i] = (TProfile*)output_file->Get( hname.data() );

		else {

			hasic_pause[i] = new TProfile( hname.data(), htitle.data(), 1000, 0., 10000. );
			hasic_pause[i]->SetDirectory(
					output_file->GetDirectory( dirname.data() ) );

		}

		hname = "hasic_resume" + std::to_string(i);
		htitle = "Profile of ts versus resume events in ISS module " + std::to_string(i);

		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hasic_resume[i] = (TProfile*)output_file->Get( hname.data() );

		else {

			hasic_resume[i] = new TProfile( hname.data(), htitle.data(), 1000, 0., 10000. );
			hasic_resume[i]->SetDirectory(
					output_file->GetDirectory( dirname.data() ) );

		}
	
	}
	
	// Loop over CAEN modules
	for( unsigned int i = 0; i < set->GetNumberOfCAENModules(); ++i ) {
		
		hname = "hcaen_hit" + std::to_string(i);
		htitle = "Profile of ts versus hit_id in CAEN module " + std::to_string(i);

		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hcaen_hit[i] = (TProfile*)output_file->Get( hname.data() );

		else {

			hcaen_hit[i] = new TProfile( hname.data(), htitle.data(), 10800, 0., 1080000. );
			hcaen_hit[i]->SetDirectory(
					output_file->GetDirectory( dirname.data() ) );

		}

	
		hname = "hcaen_ext" + std::to_string(i);
		htitle = "Profile of external trigger ts versus hit_id in CAEN module " + std::to_string(i);

		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hcaen_ext[i] = (TProfile*)output_file->Get( hname.data() );

		else {

			hcaen_ext[i] = new TProfile( hname.data(), htitle.data(), 10800, 0., 108000. );
			hcaen_ext[i]->SetDirectory(
					output_file->GetDirectory( dirname.data() ) );

		}

	}
	
	return;
	
}

// Function to copy the header from a DataSpy, for example
void ISSConverter::SetBlockHeader( char *input_header ){
	
	// Copy header
	for( unsigned int i = 0; i < HEADER_SIZE; i++ )
		block_header[i] = input_header[i];

	return;
	
}

// Function to process header words
void ISSConverter::ProcessBlockHeader( unsigned long nblock ){
		
	// For each new header, reset the swap mode
	swap = 0;
	
	// Flag if we have ASIC data
	flag_asic_data = false;
	
	// Flags for CAEN data items
	flag_caen_data0 = false;
	flag_caen_data1 = false;
	flag_caen_data3 = false;
	flag_caen_trace = false;

	// Flag when we find the end of the data
	flag_terminator = false;

	// Process header.
	for( UInt_t i = 0; i < 8; i++ )
		header_id[i] = block_header[i];
	
	header_sequence =
	(block_header[8] & 0xFF) << 24 | (block_header[9]& 0xFF) << 16 |
	(block_header[10]& 0xFF) << 8  | (block_header[11]& 0xFF);
	
	header_stream = (block_header[12] & 0xFF) << 8 | (block_header[13]& 0xFF);
	
	header_tape = (block_header[14] & 0xFF) << 8 | (block_header[15]& 0xFF);
	
	header_MyEndian = (block_header[16] & 0xFF) << 8 | (block_header[17]& 0xFF);
	
	header_DataEndian = (block_header[18] & 0xFF) << 8 | (block_header[19]& 0xFF);
	
	header_DataLen =
	(block_header[20] & 0xFF) | (block_header[21]& 0xFF) << 8 |
	(block_header[22] & 0xFF) << 16  | (block_header[23]& 0xFF) << 24 ;
	

	if( std::string(header_id).substr(0,8) != "EBYEDATA" ) {
	
		std::cerr << "Bad header in block " << nblock << std::endl;
		exit(0);
	
	}
	
	//std::cout << nblock << "\t" << header_DataLen << std::endl;
	
	return;
	
}


// Function to copy the main data from a DataSpy, for example
void ISSConverter::SetBlockData( char *input_data ){
	
	// Copy header
	for( UInt_t i = 0; i < MAIN_SIZE; i++ )
		block_data[i] = input_data[i];

	return;
	
}


// Function to process data words
void ISSConverter::ProcessBlockData( unsigned long nblock ){
	
	// Get the data in 64-bit words and check endieness and swap if needed
	// Data format here: http://npg.dl.ac.uk/documents/edoc504/edoc504.html
	// Unpack in to two 32-bit words for purposes of data format
		
	// Swap mode is unknown for the first block of data, so let's work it out
	if( (swap & SWAP_KNOWN) == 0 ) {

		// See if we can figure out the swapping - the DataEndian word of the
		// header is 256 if the endianness is correct, otherwise swap endianness
		if( header_DataEndian != 256 ) swap |= SWAP_ENDIAN;
		
		// However, that is not all, the words may also be swapped, so check
		// for that. Bits 31:30 should always be zero in the timestamp word
		for( UInt_t i = 0; i < WORD_SIZE; i++ ) {
			word = (swap & SWAP_ENDIAN) ? Swap64(data[i]) : data[i];
			if( word & 0xC000000000000000LL ) {
				swap |= SWAP_KNOWN;
				break;
			}
			if( word & 0x00000000C0000000LL ) {
				swap |= SWAP_KNOWN;
				swap |= SWAP_WORDS;
				break;
			}
		}
		
	}

	
	// Process all words
	for( UInt_t i = 0; i < WORD_SIZE; i++ ) {
				
		word = GetWord(i);
		word_0 = (word & 0xFFFFFFFF00000000) >> 32;
		word_1 = (word & 0x00000000FFFFFFFF);
		
		// Check the trailer: reject or keep the block.
		if( ( word_0 & 0xFFFFFFFF ) == 0xFFFFFFFF ||
		    ( word_0 & 0xFFFFFFFF ) == 0x5E5E5E5E ||
		    ( word_1 & 0xFFFFFFFF ) == 0xFFFFFFFF ||
		    ( word_1 & 0xFFFFFFFF ) == 0x5E5E5E5E ){
			
			flag_terminator = true;
			return;
			
		}
		else if( i >= header_DataLen/sizeof(ULong64_t) ){
			
			flag_terminator = true;
			return;
			
		}

		
			
		// Data type is highest two bits
		my_type = ( word_0 >> 30 ) & 0x3;
		
		// ADC data - we need to know if it is CAEN or ASIC
		if( my_type == 0x3 ){
			
			// Decide if this is an ASIC or CAEN event
			// ISS/R3B ASICs will have 28th bit of word_1 set to 1
			// This is for data after to R3B data format change (June 2021)
			// Otherwise, we read it in from the settings file
			if( ((word_1 >> 28) & 0x00000001) == 0x00000001 ||
			    set->IsASICOnly() ){

				flag_asic_data = true;
				ProcessASICData();
				
			}
			
			else {
				
				ProcessCAENData();
				FinishCAENData();

			}

		}
		
		// Information data
		else if( my_type == 0x2 ){
			
			ProcessInfoData();

		}
		
		// Trace header
		else if( my_type == 0x1 ){
			
			// contains the sample length
			nsamples = word_0 & 0xFFFF; // 16 bits from 0
			
			// Get the samples from the trace
			for( UInt_t j = 0; j < nsamples; j++ ){
				
				// get next word
				i++;
				sample_packet = GetWord(i);
				
				block_test = ( sample_packet >> 32 ) & 0x00000000FFFFFFFF;
				trace_test = ( sample_packet >> 62 ) & 0x0000000000000003;
				
				if( trace_test == 0 && block_test != 0x5E5E5E5E ){
					
					caen_data->AddSample( ( sample_packet >> 48 ) & 0x0000000000003FFF );
					caen_data->AddSample( ( sample_packet >> 32 ) & 0x0000000000003FFF );
					caen_data->AddSample( ( sample_packet >> 16 ) & 0x0000000000003FFF );
					caen_data->AddSample( sample_packet & 0x0000000000003FFF );
					
				}
				
				else {
					
					//std::cout << "This isn't a trace anymore..." << std::endl;
					//std::cout << "Sample #" << j << " of " << nsamples << std::endl;
					//std::cout << " trace_test = " << (int)trace_test << std::endl;

					i--;
					break;
					
				}

			}
			
			flag_caen_trace = true;
			FinishCAENData();

		}
		
		else {
			
			// output error message!
			std::cerr << "WARNING: WRONG TYPE! word 0: " << word_0;
			std::cerr << ", my_type: " << my_type << std::endl;
		
		}
		
	} // loop - i < header_DataLen
	
	return;

}

void ISSConverter::ProcessASICData(){

	// ISS/R3B ASIC data format
	my_hit = ( word_0 >> 29 ) & 0x1;
	my_adc_data = word_0 & 0xFFF; // 12 bits from 0
	
	// ADCchannelIdent are bits 28:12
	// mod_id= bit 16:11, asic_id= bit 10:7, ch_id= bit 6:0
	unsigned int ADCchanIdent = (word_0 >> 12) & 0x0001FFFF; // 17 bits from 12
	my_mod_id = (ADCchanIdent >> 11) & 0x003F; // 6 bits from 11
	my_asic_id = (ADCchanIdent >> 7 ) & 0x000F; // 4 bits from 7
	my_ch_id = ADCchanIdent & 0x007F; // 7 bits from 0
	
	my_tm_stp_lsb = word_1 & 0x0FFFFFFF;  // 28 bits from 0
	
	// Check things make sense
	if( my_mod_id >= set->GetNumberOfArrayModules() ||
		my_asic_id >= set->GetNumberOfArrayASICs() ||
		my_ch_id >= set->GetNumberOfArrayChannels() ) {
		
		std::cout << "Bad ASIC event with mod_id=" << my_mod_id;
		std::cout << " asic_id=" << my_asic_id;
		std::cout << " ch_id=" << my_ch_id << std::endl;
		
		return;
		
	}
	
	// reconstruct time stamp= HSB+MSB+LSB
	my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb_asic << 28 ) | my_tm_stp_lsb;
	
	// Pulser in a spare n-side channel should be counted as info data
	if( my_asic_id == set->GetArrayPulserAsic() &&
		my_ch_id == set->GetArrayPulserChannel() ) {
		
		// Check energy to set threshold
		asic_pulser_energy->Fill( my_adc_data );
		
		// If it's above an energy threshold, then count it
		if( my_adc_data > set->GetArrayPulserThreshold() ) {
		   
			info_data->SetModule( my_mod_id );
			info_data->SetTime( my_tm_stp );
			info_data->SetCode( set->GetArrayPulserCode() );
			data_packet->SetData( info_data );
			if( !flag_source ) output_tree->Fill();
			info_data->Clear();
			data_packet->ClearData();
			
		}
		
	}

	else {

		// Calibrate
		my_energy = cal->AsicEnergy( my_mod_id, my_asic_id, my_ch_id, my_adc_data );
		
		// Is it disabled?
		if( !cal->AsicEnabled( my_mod_id, my_asic_id ) ) return;
		
		// Fill histograms
		hasic[my_mod_id][my_asic_id]->Fill( my_ch_id, my_adc_data );
		hasic_cal[my_mod_id][my_asic_id]->Fill( my_ch_id, my_energy );
		hasic_hit[my_mod_id]->Fill( ctr_asic_hit[my_mod_id], my_tm_stp, 1 );

		if( my_asic_id == 0 || my_asic_id == 2 || my_asic_id == 3 || my_asic_id == 5 )
			hpside[my_mod_id]->Fill( my_energy );
		else if( my_asic_id == 1 || my_asic_id == 4 )
			hnside[my_mod_id]->Fill( my_energy );


		// Make an AsicData item
		asic_data->SetTime( my_tm_stp + cal->AsicTime( my_mod_id, my_asic_id ) );
		asic_data->SetWalk( (int)cal->AsicWalk( my_mod_id, my_asic_id, my_energy ) );
		asic_data->SetAdcValue( my_adc_data );
		asic_data->SetHitBit( my_hit );
		asic_data->SetModule( my_mod_id );
		asic_data->SetAsic( my_asic_id );
		asic_data->SetChannel( my_ch_id );
		asic_data->SetEnergy( my_energy );
		
		// Check if it's over threshold
		if( my_adc_data > cal->AsicThreshold( my_mod_id, my_asic_id, my_ch_id ) )
			asic_data->SetThreshold( true );
		else asic_data->SetThreshold( false );
		
		// Set this data and fill event to tree
		data_packet->SetData( asic_data );
		if( !flag_source ) output_tree->Fill();
		asic_data->Clear();
		data_packet->ClearData();
		
		// Count asic hit per module
		ctr_asic_hit[my_mod_id]++;

	}
		
	return;
	
}

void ISSConverter::ProcessCAENData(){

	// CAEN data format
	my_adc_data = word_0 & 0xFFFF; // 16 bits from 0
	
	// ADCchannelIdent are bits 28:16
	// mod_id= bit 12:8, data_id= bit 7:6, ch_id= bit 5:0
	// data_id: Qlong = 0; Qshort = 1; baseline = 2; fine timing = 3
	// in this implementation, we only use Qlong, i.e. data_id = 0
	unsigned int ADCchanIdent = (word_0 >> 16) & 0x1FFF; // 13 bits from 16
	my_mod_id = (ADCchanIdent >> 8) & 0x001F; // 5 bits from 8
	my_data_id = (ADCchanIdent >> 6 ) & 0x0003; // 2 bits from 6
	my_ch_id = ADCchanIdent & 0x003F; // 6 bits from 0
	
	// Check things make sense
	if( my_mod_id >= set->GetNumberOfCAENModules() ||
		my_ch_id >= set->GetNumberOfCAENChannels() ) {
		
		std::cout << "Bad CAEN event with mod_id=" << my_mod_id;
		std::cout << " ch_id=" << my_ch_id;
		std::cout << " data_id=" << my_data_id << std::endl;
		return;

	}
	
	// reconstruct time stamp= MSB+LSB
	my_tm_stp_lsb = word_1 & 0x0FFFFFFF;  // 28 bits from 0
	my_tm_stp = ( my_tm_stp_msb << 28 ) | my_tm_stp_lsb;
	
	// CAEN timestamps are 4 ns precision
	my_tm_stp = my_tm_stp*4;
	
	// First of the data items
	if( !flag_caen_data0 && !flag_caen_data1 && !flag_caen_data3 ){
		
		// Make a CaenData item, need to add Qshort and traces
		caen_data->SetTime( my_tm_stp );
		caen_data->SetModule( my_mod_id );
		caen_data->SetChannel( my_ch_id );
		
	}
	
	// If we already have all the data items, then the next event has
	// already occured before we found traces. This means that there
	// is not trace data. So set the flag to be true and finish the
	// event with an empty trace.
	else if( flag_caen_data0 && flag_caen_data1 && flag_caen_data3 ){
		
		// Fake trace flag, but with an empty trace
		flag_caen_trace = true;
		
		// Finish up the previous event
		FinishCAENData();

		// Then set the info correctly for this event
		caen_data->SetTime( my_tm_stp );
		caen_data->SetModule( my_mod_id );
		caen_data->SetChannel( my_ch_id );
		
	}

	// Qlong
	if( my_data_id == 0 ) {
		
		// Fill histograms
		my_energy = cal->CaenEnergy( my_mod_id, my_ch_id, my_adc_data );
		hcaen[my_mod_id][my_ch_id]->Fill( my_adc_data );
		hcaen_cal[my_mod_id][my_ch_id]->Fill( my_energy );
		
		caen_data->SetQlong( my_adc_data );
		caen_data->SetEnergy( my_energy );

		// Check if it's over threshold
		if( my_adc_data > cal->CaenThreshold( my_mod_id, my_ch_id ) )
			caen_data->SetThreshold( true );
		else caen_data->SetThreshold( false );

		flag_caen_data0 = true;

	}
	
	// Qshort
	if( my_data_id == 1 ) {
		
		my_adc_data = my_adc_data & 0x7FFF; // 15 bits from 0
		caen_data->SetQshort( my_adc_data );
		flag_caen_data1 = true;

	}

	// Fine timing
	if( my_data_id == 3 ) {
		
		my_adc_data = my_adc_data & 0x03FF; // 10 bits from 0
		caen_data->SetFineTime( my_adc_data );
		flag_caen_data3 = true;

	}

	
	return;

}

void ISSConverter::FinishCAENData(){
	
	// Got all items
	if( flag_caen_data0 && flag_caen_data1 && flag_caen_data3 && flag_caen_trace ){

		// Fill histograms
		hcaen_hit[caen_data->GetModule()]->Fill( ctr_caen_hit[caen_data->GetModule()], caen_data->GetTime(), 1 );

		// Check if this is actually just a timestamp
		flag_caen_info = false;
		if( caen_data->GetModule() == set->GetCAENPulserModule() &&
		    caen_data->GetChannel() == set->GetCAENPulserChannel() ){
			
			flag_caen_info = true;
			//my_info_code = set->GetCAENPulserCode();
			my_info_code = 20; // CAEN pulser is always 20 (defined here)

		}
		
		else if( caen_data->GetModule() == set->GetEBISModule() &&
		    caen_data->GetChannel() == set->GetEBISChannel() ){
			
			flag_caen_info = true;
			//my_info_code = set->GetEBISCode();
			my_info_code = 21; // CAEN EBIS is always 21 (defined here), Array EBIS is 15
			
		}
		
		else if( caen_data->GetModule() == set->GetT1Module() &&
		    caen_data->GetChannel() == set->GetT1Channel() ){
			
			flag_caen_info = true;
			//my_info_code = set->GetT1Code();
			my_info_code = 22; // CAEN T1 is always 22 (defined here)
			
		}

		// If this is a timestamp, fill an info event
		if( flag_caen_info ) {
				
			// Add the time offset to this channel
			info_data->SetTime( caen_data->GetTime() + cal->CaenTime( caen_data->GetModule(), caen_data->GetChannel() ) );
			info_data->SetModule( caen_data->GetModule() + set->GetNumberOfArrayModules() );
			info_data->SetCode( my_info_code );
			data_packet->SetData( info_data );
			if( !flag_source ) output_tree->Fill();
			info_data->Clear();
			data_packet->ClearData();

			// Fill histograms for external trigger
			if( my_info_code == set->GetCAENPulserCode() ) {
				
				hcaen_ext[caen_data->GetModule()]->Fill( ctr_caen_ext[caen_data->GetModule()], caen_data->GetTime(), 1 );

				// Count external trigger event
				ctr_caen_ext[caen_data->GetModule()]++;
					
			}

		}

		// Otherwise it is real data, so fill a caen event
		else {
			
			// Set this data and fill event to tree
			// Also add the time offset when we do this
			caen_data->SetTime( caen_data->GetTime() + cal->CaenTime( caen_data->GetModule(), caen_data->GetChannel() ) );
			data_packet->SetData( caen_data );
			if( !flag_source ) output_tree->Fill();
			data_packet->ClearData();
			
			//std::cout << "Complete CAEN event" << std::endl;
			//std::cout << "Trace length = " << caen_data->GetTraceLength() << std::endl;

		}

	}
	
	// missing something
	else if( my_tm_stp != caen_data->GetTime() ) {
		
		std::cout << "Missing something in CAEN data and new event occured" << std::endl;
		std::cout << " Qlong       = " << flag_caen_data0 << std::endl;
		std::cout << " Qshort      = " << flag_caen_data1 << std::endl;
		std::cout << " fine timing = " << flag_caen_data3 << std::endl;
		std::cout << " trace data  = " << flag_caen_trace << std::endl;

	}

	// This is normal, just not finished yet
	else return;
	
	// Count the hit, even if it's bad
	ctr_caen_hit[caen_data->GetModule()]++;
	
	// Assuming it did finish, in a good way or bad, clean up.
	flag_caen_data0 = false;
	flag_caen_data1 = false;
	flag_caen_data3 = false;
	flag_caen_trace = false;
	caen_data->ClearData();
	
	return;

}

void ISSConverter::ProcessInfoData(){

	// MIDAS info data format
	my_mod_id = (word_0 >> 24) & 0x003F; // bits 24:29

	my_info_field = word_0 & 0x000FFFFF; //bits 0:19
	my_info_code = (word_0 >> 20) & 0x0000000F; //bits 20:23
	my_tm_stp_lsb = word_1 & 0x0FFFFFFF;  //bits 0:27

	// HSB of timestamp
	if( my_info_code == set->GetTimestampCode() ) {
		
		my_tm_stp_hsb = my_info_field & 0x000FFFFF;

	}
	
	// MSB of timestamp in sync pulse or CAEN extended time stamp
	if( my_info_code == set->GetSyncCode() ) {
		
		// We don't know yet if it's from CAEN or ISS
		// In CAEN this would be the extended timestamp
		// In ISS it is the Sync100 pulses
		my_tm_stp_msb = my_info_field & 0x000FFFFF;
		my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb << 28 ) | ( my_tm_stp_lsb & 0x0FFFFFFF );

	}
	
	// MSB of timestamp in ASIC data
	if( my_info_code == set->GetExtItemCode() ) {
		
		// ASIC data has info code 7 for extended timestamp
		my_tm_stp_msb_asic = my_info_field & 0x000FFFFF;
		my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb_asic << 28 ) | ( my_tm_stp_lsb & 0x0FFFFFFF );

	}
	
	// External trigger 1
	if( my_info_code == set->GetExternalTriggerCode() ) {
		
		my_tm_stp_msb = my_info_field & 0x000FFFFF;
		my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb << 28 ) | ( my_tm_stp_lsb & 0x0FFFFFFF );
		hasic_ext[my_mod_id]->Fill( ctr_asic_ext[my_mod_id], my_tm_stp, 1 );
		ctr_asic_ext[my_mod_id]++;

	}

	// External trigger 2
	if( my_info_code == 15 ) {
		
		my_tm_stp_msb = my_info_field & 0x000FFFFF;
		my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb << 28 ) | ( my_tm_stp_lsb & 0x0FFFFFFF );

	}

	// Pause
    if( my_info_code == set->GetPauseCode() ) {
         
		my_tm_stp_msb = my_info_field & 0x000FFFFF;
		my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb << 28 ) | ( my_tm_stp_lsb & 0x0FFFFFFF );
		hasic_pause[my_mod_id]->Fill( ctr_asic_pause[my_mod_id], my_tm_stp, 1 );
		ctr_asic_pause[my_mod_id]++;

    }

	// Resume
	if( my_info_code == set->GetResumeCode() ) {
         
		my_tm_stp_msb = my_info_field & 0x000FFFFF;
		my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb << 28 ) | ( my_tm_stp_lsb & 0x0FFFFFFF );
		hasic_resume[my_mod_id]->Fill( ctr_asic_resume[my_mod_id], my_tm_stp, 1 );
		ctr_asic_resume[my_mod_id]++;

    }

	// Create an info event and fill the tree for external triggers and pause/resume
	if( my_info_code == set->GetExternalTriggerCode() ||
	    my_info_code == set->GetPauseCode() ||
	    my_info_code == set->GetResumeCode() ||
	    my_info_code == 15 ) {

		info_data->SetModule( my_mod_id );
		info_data->SetTime( my_tm_stp );
		info_data->SetCode( my_info_code );
		data_packet->SetData( info_data );
		if( !flag_source ) output_tree->Fill();
		info_data->Clear();
		data_packet->ClearData();

	}

	return;
	
}

// Common function called to process data in a block from file or DataSpy
bool ISSConverter::ProcessCurrentBlock( int nblock ) {
	
	// Process header.
	ProcessBlockHeader( nblock );

	// Process the main block data until terminator found
	data = (ULong64_t *)(block_data);
	ProcessBlockData( nblock );
			
	// Check once more after going over left overs....
	if( !flag_terminator && flag_asic_data ){

		std::cout << std::endl << __PRETTY_FUNCTION__ << std::endl;
		std::cout << "\tERROR - Terminator sequence not found in data.\n";
		return false;
		
	}
	
	return true;

}

// Function to convert a block of data from DataSpy
int ISSConverter::ConvertBlock( char *input_block, int nblock ) {
	
	// Get the header.
	std::memmove( &block_header, &input_block[0], HEADER_SIZE );
	
	// Get the block
	std::memmove( &block_data, &input_block[HEADER_SIZE], MAIN_SIZE );
	
	// Process the data
	ProcessCurrentBlock( nblock );
	
	// Print time
	//std::cout << "Last time stamp of block = " << my_tm_stp << std::endl;

	return nblock+1;
	
}

// Function to run the conversion for a single file
int ISSConverter::ConvertFile( std::string input_file_name,
							 unsigned long start_block,
							 long end_block ) {
	
	// Read the file.
	std::ifstream input_file( input_file_name, std::ios::in|std::ios::binary );
	if( !input_file.is_open() ){
		
		std::cout << "Cannot open " << input_file_name << std::endl;
		return -1;
		
	}

	// Conversion starting
	std::cout << "Converting file: " << input_file_name;
	std::cout << " from block " << start_block << std::endl;
	
	
	// Calculate the size of the file.
	input_file.seekg( 0, input_file.end );
	unsigned long long size_end = input_file.tellg();
	input_file.seekg( 0, input_file.beg );
	unsigned long long size_beg = input_file.tellg();
	unsigned long long FILE_SIZE = size_end - size_beg;
	
	// Calculate the number of blocks in the file.
	unsigned long BLOCKS_NUM = FILE_SIZE / DATA_BLOCK_SIZE;
	
	//a sanity check for file size...
	//QQQ: add more strict test?
	if( FILE_SIZE % DATA_BLOCK_SIZE != 0 ){
		
		std::cout << " *WARNING* " << __PRETTY_FUNCTION__;
		std::cout << "\tMissing data blocks?" << std::endl;

	}
	
	sslogs << "\t File size = " << FILE_SIZE << std::endl;
	sslogs << "\tBlock size = " << DATA_BLOCK_SIZE << std::endl;
	sslogs << "\t  N blocks = " << BLOCKS_NUM << std::endl;

	std::cout << sslogs.str() << std::endl;
	sslogs.str( std::string() ); // clean up
	
	// Data format: http://npg.dl.ac.uk/documents/edoc504/edoc504.html
	// The information is split into 2 words of 32 bits (4 byte).
	// We will collect the data in 64 bit words and split later
	
	
	// Loop over all the blocks.
	for( unsigned long nblock = 0; nblock < BLOCKS_NUM ; nblock++ ){
		
		// Take one block each time and analyze it.
		if( nblock % 200 == 0 || nblock+1 == BLOCKS_NUM ) {
			
			// Percent complete
			float percent = (float)(nblock+1)*100.0/(float)BLOCKS_NUM;
			
			// Progress bar in GUI
			if( _prog_ ) {
				
				prog->SetPosition( percent );
				gSystem->ProcessEvents();
				
			}

			// Progress bar in terminal
			std::cout << " " << std::setw(8) << std::setprecision(4);
			std::cout << percent << "%\r";
			std::cout.flush();

		}

		
		// Get the header.
		input_file.read( (char*)&block_header, HEADER_SIZE );
		// Get the block
		input_file.read( (char*)&block_data, MAIN_SIZE );


		// Check if we are before the start block or after the end block
		if( nblock < start_block || ( (long)nblock > end_block && end_block > 0 ) )
			continue;


		// Process current block. If it's the end, stop.
		if( !ProcessCurrentBlock( nblock ) ) break;
		
		
	} // loop - nblock < BLOCKS_NUM
	
	// Close input
	input_file.close();
	
	// Print time
	//std::cout << "Last time stamp in file = " << my_tm_stp << std::endl;
	
	return BLOCKS_NUM;
	
}

unsigned long long ISSConverter::SortTree(){
	
	// Reset the sorted tree so it's empty before we start
	sorted_tree->Reset();
	
	// Load the full tree if possible
	output_tree->SetMaxVirtualSize(2e9); // 2GB
	sorted_tree->SetMaxVirtualSize(2e9); // 2GB
	output_tree->LoadBaskets(2e9); // Load 2 GB of data to memory
	//output_tree->OptimizeBaskets(2000000000);
	
	// Check we have entries and build time-ordered index
	if( output_tree->GetEntries() ){

		std::cout << "\n Building time-ordered index of events..." << std::endl;
		output_tree->BuildIndex( "data.GetTime()" );

	}
	else return 0;
	
	// Get index and prepare for sorting
	TTreeIndex *att_index = (TTreeIndex*)output_tree->GetTreeIndex();
	unsigned long long nb_idx = att_index->GetN();
	std::cout << " Sorting: size of the sorted index = " << nb_idx << std::endl;

	// Loop on t_raw entries and fill t
	for( unsigned long i = 0; i < nb_idx; ++i ) {
		
		data_packet->ClearData();
		unsigned long long idx = att_index->GetIndex()[i];
		if( output_tree->MemoryFull(3000) ) output_tree->DropBaskets();
		output_tree->GetEntry( idx );
		sorted_tree->Fill();

		// Progress bar
		bool update_progress = false;
		if( nb_idx < 200 )
			update_progress = true;
		else if( i % (nb_idx/100) == 0 || i+1 == nb_idx )
			update_progress = true;
		
		if( update_progress ) {
			
			// Percent complete
			float percent = (float)(i+1)*100.0/(float)nb_idx;
			
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

	}
	
	// Reset the output tree so it's empty after we've finished
	output_tree->FlushBaskets();
	output_tree->Reset();

	return nb_idx;
	
}
