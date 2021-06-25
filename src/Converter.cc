#include "Converter.hh"

Converter::Converter() {
	
	my_tm_stp_msb = 0;
	my_tm_stp_hsb = 0;

	// Start counters at zero
	for( unsigned int i = 0; i < common::n_module; ++i ) {
				
		ctr_asic_hit[i]	= 0;	// hits on each module
		ctr_asic_ext[i]	= 0;	// external timestamps
		
	}
	
	for( unsigned int i = 0; i < common::n_caen_mod; ++i ) {
				
		ctr_caen_hit[i]	= 0;	// hits on each module
		ctr_caen_ext[i]	= 0;	// external timestamps
		
	}

}

Converter::~Converter() {
	
	//std::cout << "destructor" << std::endl;

}


void Converter::SetOutput( std::string output_file_name ){
	
	// Open output file
	output_file = new TFile( output_file_name.data(), "recreate", 0 );

	// Create log file.
	std::string log_file_name = output_file_name.substr( 0, output_file_name.find_last_of(".") );
	log_file_name += ".log";
	log_file.open( log_file_name.c_str(), std::ios::out );
	
	return;

};


void Converter::MakeTree() {

	// Create Root tree
	if( gDirectory->GetListOfKeys()->Contains( "iss" ) ) {
		
		output_tree = (TTree*)gDirectory->Get("iss");
		output_tree->SetBranchAddress( "data", &data_packet );

	}
	
	else {
	
		output_tree = new TTree( "iss", "iss" );
		data_packet = new DataPackets();
		output_tree->Branch( "data", "DataPackets", &data_packet );
		
	}
	
	asic_data = new AsicData();
	caen_data = new CaenData();
	info_data = new InfoData();
	
	asic_data->ClearData();
	caen_data->ClearData();
	info_data->ClearData();
	
	return;
	
}

void Converter::MakeHists() {
	
	std::string hname, htitle;
	std::string dirname, maindirname, subdirname;
	
	// Make directories
	maindirname = "asic_hists";

	// Loop over ISS modules
	for( int i = 0; i < common::n_module; ++i ) {
		
		subdirname = "/module_" + std::to_string(i);
		
		// Loop over ASICs for the array
		for( int j = 0; j < common::n_asic; ++j ) {
			
			dirname = maindirname + subdirname;
			dirname += "/asic_" + std::to_string(j);
			
			if( !output_file->GetDirectory( dirname.data() ) )
				output_file->mkdir( dirname.data() );
			output_file->cd( dirname.data() );

			// Loop over channels of each ASIC
			for( int k = 0; k < common::n_channel; ++k ) {
				
				hname = "asic_" + std::to_string(i);
				hname += "_" + std::to_string(j);
				hname += "_" + std::to_string(k);
				
				htitle = "Raw ASIC spectra for module " + std::to_string(i);
				htitle += ", ASIC " + std::to_string(j);
				htitle += ", channel " + std::to_string(k);
				
				htitle += ";ADC channel;Counts";
				
				if( output_file->GetListOfKeys()->Contains( hname.data() ) )
					hasic[i][j][k] = (TH1F*)output_file->Get( hname.data() );
				
				else {
					
					hasic[i][j][k] = new TH1F( hname.data(), htitle.data(),
								4096, -0.5, 4095.5 );
					hasic[i][j][k]->SetDirectory(
							output_file->GetDirectory( dirname.data() ) );
					
				}
				
			}
			
		}
		
	}
	
	// Make directories
	maindirname = "caen_hists";

	// Loop over CAEN modules
	for( int i = 0; i < common::n_caen_mod; ++i ) {
		
		dirname = maindirname + "/module_" + std::to_string(i);
		
		if( !output_file->GetDirectory( dirname.data() ) )
			output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );

		// Loop over channels of each CAEN module
		for( int j = 0; j < common::n_caen_ch; ++j ) {
			
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
			
		}
					
	}
	
	// Make directories
	dirname = "timing_hists";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	// Loop over ISS modules
	for( unsigned int i = 0; i < common::n_module; ++i ) {
		
		hname = "hasic_hit" + std::to_string(i);
		htitle = "Profile of ts versus hit_id in ISS module " + std::to_string(i);

		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hasic_hit[i] = (TProfile*)output_file->Get( hname.data() );

		else {

			hasic_hit[i] = new TProfile( hname.data(), htitle.data(), 3001 , 0., 30000., 0, 3.61e14 );
			hasic_hit[i]->SetDirectory(
					output_file->GetDirectory( dirname.data() ) );

		}
		
		hname = "hasic_ext" + std::to_string(i);
		htitle = "Profile of external trigger ts versus hit_id in ISS module " + std::to_string(i);

		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hasic_ext[i] = (TProfile*)output_file->Get( hname.data() );

		else {

			hasic_ext[i] = new TProfile( hname.data(), htitle.data(), 3001 , 0., 30000., 0, 3.61e14 );
			hasic_ext[i]->SetDirectory(
					output_file->GetDirectory( dirname.data() ) );

		}
	
	}
	
	// Loop over CAEN modules
	for( unsigned int i = 0; i < common::n_caen_mod; ++i ) {
		
		hname = "hcaen_hit" + std::to_string(i);
		htitle = "Profile of ts versus hit_id in CAEN module " + std::to_string(i);

		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hcaen_hit[i] = (TProfile*)output_file->Get( hname.data() );

		else {

			hcaen_hit[i] = new TProfile( hname.data(), htitle.data(), 3001 , 0., 30000., 0, 3.61e14 );
			hcaen_hit[i]->SetDirectory(
					output_file->GetDirectory( dirname.data() ) );

		}

	
		hname = "hcaen_ext" + std::to_string(i);
		htitle = "Profile of external trigger ts versus hit_id in CAEN module " + std::to_string(i);

		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hcaen_ext[i] = (TProfile*)output_file->Get( hname.data() );

		else {

			hcaen_ext[i] = new TProfile( hname.data(), htitle.data(), 3001 , 0., 30000., 0, 3.61e13 );
			hcaen_ext[i]->SetDirectory(
					output_file->GetDirectory( dirname.data() ) );

		}

	}
	
	return;
	
}

// Function to copy the header from a DataSpy, for example
void Converter::SetBlockHeader( char *input_header ){
	
	// Copy header
	for( int i = 0; i < HEADER_SIZE ; i++ )
		block_header[i] = input_header[i];

	return;
	
}

// Function to process header words
void Converter::ProcessBlockHeader( int nblock ){
		
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
	for( UInt_t i = 0; i < 8 ; i++ )
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
	
	//nwords = header_DataLen / sizeof(ULong64_t);
	nwords = MAIN_SIZE / sizeof(ULong64_t);

	// Print header info.
	log_file << "== DATA BLOCK: " << nblock << std::endl;
	log_file << "Header_id: " << header_id << std::endl;
	log_file << "Header_sequence: " << header_sequence << std::endl;
	log_file << "Header_stream: " << header_stream << std::endl;
	log_file << "Header tape: " << header_tape << std::endl;
	log_file << "Header_MyEndian: " << header_MyEndian << std::endl;
	log_file << "Header_DataEndian: " << header_DataEndian << std::endl;
	log_file << "Header_DataLen: " << header_DataLen << std::endl;
	
	log_file << "== and in HEX:" << std::hex << std::endl;
	for( UInt_t i = 0; i < 24; i++ ) {
		
		log_file << int( block_header[i] ) << " ";
		if( (i+1)%4 == 0 ) log_file << std::endl;
		
	}
	
	if( std::string(header_id).substr(0,8) != "EBYEDATA" ) {
	
		std::cerr << "Bad header in block " << nblock << std::endl;
		exit(0);
	
	}
	
	return;
	
}


// Function to copy the main data from a DataSpy, for example
void Converter::SetBlockData( char *input_data ){
	
	// Copy header
	for( UInt_t i = 0; i < MAIN_SIZE ; i++ )
		block_data[i] = input_data[i];

	return;
	
}


// Function to process data words
void Converter::ProcessBlockData( int nblock ){
	
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
		for( UInt_t i = 0; i < nwords; i++ ) {
			ULong64_t word = (swap & SWAP_ENDIAN) ? Swap64(data[i]) : data[i];
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
	for( UInt_t i = 0; i < nwords; i++ ) {
		
		word = GetWord(i);
		word_0 = (word & 0xFFFFFFFF00000000) >> 32;
		word_1 = (word & 0x00000000FFFFFFFF);

		// Put the info in binary and dump it to file.
		// only do it for first 25 words of data
		if( i < 25 && nblock < 10 ){
			
			std::bitset<32> _word_0(word_0);
			std::bitset<32> _word_1(word_1);
			
			log_file << "= Entry: " << i  << std::endl;
			log_file << "Word 0: " << _word_0 << "  "<< std::hex << "0x"<<word_0 << std::dec << std::endl; // Must have first 2 bits: 10.
			log_file << "Word 1: " << _word_1 << "  "<< std::hex << "0x"<<word_1 << std::dec << std::endl; // Must have first 4 bits: 0000.
						
		}
		
		// Check the trailer: reject or keep the block.
		if( ( word_0 & 0xFFFFFFFF ) == 0xFFFFFFFF ||
		    ( word_0 & 0xFFFFFFFF ) == 0x5E5E5E5E ||
		    ( word_1 & 0xFFFFFFFF ) == 0xFFFFFFFF ||
		    ( word_1 & 0xFFFFFFFF ) == 0x5E5E5E5E ){
			
			log_file << "End of block " << nblock << " (" << std::hex;
			log_file << word_0 << ", " << word_1 << ")" << std::dec << std::endl;
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
			// Otherwise, we rely on the #define directive at the top of Converter.hh
#ifdef ASIC_ONLY
			flag_asic_data = true;
			ProcessASICData();
#else
			if( ((word_1 >> 28) & 0x00000001) == 0x00000001 ){

				flag_asic_data = true;
				ProcessASICData();
				
			}
			
			else {
				
				ProcessCAENData();
				FinishCAENData();

			}
#endif
		}
		
		// Information data
		else if( my_type == 0x2 ){
			
			ProcessInfoData();

		}
		
		// Trace header
		else if( my_type == 0x1 ){
			
			// contains the sample length
			nsamples = word_1 & 0x0000FFFF; // 16 bits from 0 in second word
			if( i < 25 && nblock < 10 ) log_file << "nsamples = " << nsamples << std::endl;
			
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
			log_file << "WARNING: WRONG TYPE! word 0: " << word_0;
			log_file << ", my_type: " << my_type << std::endl;
		
		}
		
		
		if( i < 25 && nblock < 10 ){
			
			// Print the information.
			log_file << "Info:" << std::endl;
			
			log_file << "type: " << std::dec << int(my_type) << std::hex <<" 0x" << int(my_type)<<std::endl;
			log_file << "hit: " << std::dec << int(my_hit) << std::hex<<" 0x"<< int(my_hit)<<std::endl;
			log_file << "module id: " << std::dec << int(my_mod_id) << std::hex<<" 0x"<< int(my_mod_id)<<std::endl;
			log_file << "asic id: " << std::dec << int(my_asic_id) << std::hex<<" 0x"<< int(my_asic_id)<<std::endl;
			log_file << "channel id: " << std::dec << int(my_ch_id) << std::hex<<" 0x"<< int(my_ch_id)<<std::endl;
			log_file << "time stamp(LSB): " << std::dec << my_tm_stp << std::hex<<" 0x"<< my_tm_stp<<std::endl;
			log_file << "ADC data: " << std::dec << my_adc_data << std::hex<<" 0x"<< my_adc_data<<std::endl;
			log_file << "ADC id: " << std::dec << int(my_data_id) << std::hex<<" 0x"<< int(my_data_id)<<std::endl;
			log_file << "info code: " << std::dec << int(my_info_code) << std::hex<<" 0x"<< int(my_info_code)<<std::endl;
			log_file << "info field: " << std::dec << my_info_field << std::hex<<" 0x"<< my_info_field<<std::endl;
			
			log_file << std::dec << std::endl;

		}
		
		
	} // loop - i < header_DataLen
	
	return;

}

void Converter::ProcessASICData(){

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
	
	// reconstruct time stamp= HSB+MSB+LSB
	my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb << 28 ) | my_tm_stp_lsb;
	
	// Check things make sense
	if( my_mod_id >= common::n_module ||
		my_asic_id >= common::n_asic ||
		my_ch_id >= common::n_channel ) {
		
		std::cout << "Bad ASIC event with mod_id=" << my_mod_id;
		std::cout << " asic_id=" << my_asic_id;
		std::cout << " ch_id=" << my_ch_id << std::endl;
		
	}

	else {
		
		// Fill histograms
		hasic[my_mod_id][my_asic_id][my_ch_id]->Fill( my_adc_data );
		hasic_hit[my_mod_id]->Fill( ctr_asic_hit[my_mod_id], my_tm_stp, 1 );


		// Make an AsicData item
		asic_data->SetTime( my_tm_stp );
		asic_data->SetAdcValue( my_adc_data );
		asic_data->SetHitBit( my_hit );
		asic_data->SetModule( my_mod_id );
		asic_data->SetAsic( my_asic_id );
		asic_data->SetChannel( my_ch_id );
		asic_data->SetEnergy( cal->AsicEnergy( my_mod_id, my_asic_id, my_ch_id, my_adc_data ) );
		
		// Set this data and fill event to tree
		data_packet->SetData( asic_data );
		output_tree->Fill();
		asic_data->Clear();
		
		// Count asic hit per module
		ctr_asic_hit[my_mod_id]++;

	}
	
	// We need a new time stamps I think?
	hsb_ready = false;
	
	return;
	
}

void Converter::ProcessCAENData(){

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
	
	// reconstruct time stamp= MSB+LSB
	my_tm_stp_lsb = word_1 & 0x0FFFFFFF;  // 28 bits from 0
	my_tm_stp = ( my_tm_stp_msb << 28 ) | my_tm_stp_lsb;
	
	// CAEN timestamps are 4 ns precision
	my_tm_stp = my_tm_stp*4;

	// Check things make sense
	if( my_mod_id >= common::n_caen_mod ||
		my_ch_id >= common::n_caen_ch ) {
		
		std::cout << "Bad CAEN event with mod_id=" << my_mod_id;
		std::cout << " ch_id=" << my_ch_id;
		std::cout << " data_id=" << my_data_id << std::endl;
		return;

	}
	
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
		hcaen[my_mod_id][my_ch_id]->Fill( my_adc_data );
		
		caen_data->SetQlong( my_adc_data );
		caen_data->SetEnergy( cal->CaenEnergy( my_mod_id, my_ch_id, my_adc_data ) );
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

void Converter::FinishCAENData(){
	
	// Got all items
	if( flag_caen_data0 && flag_caen_data1 && flag_caen_data3 && flag_caen_trace ){

		// Fill histograms
		hcaen_hit[caen_data->GetModule()]->Fill( ctr_caen_hit[caen_data->GetModule()], caen_data->GetTime(), 1 );

		// Check if this is actually just a timestamp
		flag_caen_info = false;
		if( caen_data->GetModule() == common::caen_pulser_mod &&
		    caen_data->GetChannel() == common::caen_pulser_ch ){
			
			flag_caen_info = true;
			my_info_code = common::pulser_code;
			
		}
		
		else if( caen_data->GetModule() == common::caen_ebis_mod &&
		    caen_data->GetChannel() == common::caen_ebis_ch ){
			
			flag_caen_info = true;
			my_info_code = common::ebis_code;
			
		}
		
		else if( caen_data->GetModule() == common::caen_t1_mod &&
		    caen_data->GetChannel() == common::caen_t1_ch ){
			
			flag_caen_info = true;
			my_info_code = common::t1_code;
			
		}

		// If this is a timestamp, fill an info event
		if( flag_caen_info ) {
					
			info_data->SetTime( caen_data->GetTime() );
			info_data->SetCode( my_info_code );
			data_packet->SetData( info_data );
			output_tree->Fill();
			info_data->Clear();

			// Fill histograms
			hcaen_ext[caen_data->GetModule()]->Fill( ctr_caen_ext[caen_data->GetModule()], caen_data->GetTime(), 1 );

			// Count external trigger event
			ctr_caen_ext[caen_data->GetModule()]++;

		}

		// Otherwise it is real data, so fule is caen event
		else {
			
			// Set this data and fill event to tree
			data_packet->SetData( caen_data );
			output_tree->Fill();
			
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

void Converter::ProcessInfoData(){

	// MIDAS info data format
	my_info_field = word_0 & 0x000FFFFF; //bits 0:19
	my_mod_id = (word_0 >> 24) & 0x0000003F; //bits 24:29
	my_info_code = (word_0 >> 20) & 0x0000000F; //bits 20:23
	my_tm_stp_lsb = word_1 & 0x0FFFFFFF;  //bits 0:27
	
	ts_flag = false;

	// HSB of timstamp
	if( my_info_code == common::thsb_code ) {
		
		my_tm_stp_hsb = my_info_field & 0x000FFFFF;
		hsb_ready = true;
	
	}
	
	// MSB of timstamp in sync pulse or CAEN extended time stamp
	if( my_info_code == common::sync_code ) {
		
		my_tm_stp_msb = my_info_field & 0x000FFFFF;
		ts_flag = true;
		
	}
	
	// External trigger
	if( my_info_code == common::extt_code ) {
		
		my_tm_stp_msb = my_info_field & 0x000FFFFF;
		ts_flag = true;

	}

	// Check what to do with this
	if( ts_flag ) {
		
		// reconstruct time stamp= HSB+MSB+LSB
		if( hsb_ready )
			my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb << 28 ) | ( my_tm_stp_lsb & 0x0FFFFFFF );

		// reconstruct time stamp= MSB+LSB
		else
			my_tm_stp = ( my_tm_stp_msb << 28 ) | ( my_tm_stp_lsb & 0x0FFFFFFF );
		
		hsb_ready = false;
		
	}
	
	else return;
	
	// Create an info event and fill the tree for external triggers
	if( common::extt_code == my_info_code ) {

		// Fill histograms
		hasic_ext[my_mod_id]->Fill( ctr_asic_ext[my_mod_id], my_tm_stp, 1 );

		info_data->SetTime( my_tm_stp );
		info_data->SetCode( my_info_code );
		data_packet->SetData( info_data );
		output_tree->Fill();
		info_data->Clear();
		
		// Count external trigger event
		ctr_asic_ext[my_mod_id]++;

	}
	
	return;
	
}


// Function to run the conversion for a single file
int Converter::ConvertFile( std::string input_file_name,
							 int start_block,
							 int end_block ) {
	
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
	int size_end = input_file.tellg();
	input_file.seekg( 0, input_file.beg );
	int size_beg = input_file.tellg();
	int FILE_SIZE = size_end - size_beg;
	
	// Calculate the number of blocks in the file.
	int BLOCKS_NUM = FILE_SIZE / BLOCK_SIZE;
	
	//a sanity check for file size...
	//QQQ: add more strict test?
	if( FILE_SIZE % BLOCK_SIZE != 0 ){
		
		std::cout << " *WARNING* " << __PRETTY_FUNCTION__;
		std::cout << "\tMissing data blocks?" << std::endl;

	}
	
	sslogs << "\t File size = " << FILE_SIZE << std::endl;
	sslogs << "\tBlock size = " << BLOCK_SIZE << std::endl;
	sslogs << "\t  N blocks = " << BLOCKS_NUM << std::endl;

	std::cout << sslogs.str() << std::endl;
	log_file << __PRETTY_FUNCTION__ << std::endl;
	log_file << sslogs.str() << std::endl;
	sslogs.str( std::string() ); // clean up
	
	// Data format: http://npg.dl.ac.uk/documents/edoc504/edoc504.html
	// The information is split into 2 words of 32 bits (4 byte).
	// We will collect the data in 64 bit words and split later
	
	
	// Loop over all the blocks.
	for( int nblock = 0; nblock < BLOCKS_NUM ; nblock++ ){
		
		// Take one block each time and analyze it.
		if( nblock % 200 == 0 || nblock+1 == BLOCKS_NUM ) {
			
			std::cout << " " << std::setw(8) << std::setprecision(4);
			std::cout << (float)(nblock+1)*100.0/(float)BLOCKS_NUM << "%\r";
			std::cout.flush();
			
		}
		
		// Get the header.
		input_file.read( (char*)&block_header, sizeof(block_header) );
		// Get the block
		input_file.read( (char*)&block_data, sizeof(block_data) );


		// Process header.
		ProcessBlockHeader( nblock );

		
		// Check if we are before the start block or after the end block
		if( nblock < start_block || ( nblock > end_block && end_block > 0 ) )
			continue;


		// Process the main block data until terminator found
		data = (ULong64_t *)(block_data);
		ProcessBlockData( nblock );
		
		// Check once more after going over left overs....
		if( !flag_terminator && flag_asic_data ){

			std::cout << std::endl << __PRETTY_FUNCTION__ << std::endl;
			std::cout << "\tERROR - Terminator sequence not found in data.\n";
			break;
			
		}
		
		
	} // loop - nblock < BLOCKS_NUM
	
	input_file.close();
	output_file->Write( 0, TObject::kWriteDelete );
	//output_file->Print();
	log_file.close();
	
	return BLOCKS_NUM;
	
}
