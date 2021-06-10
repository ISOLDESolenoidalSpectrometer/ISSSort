#include "Converter.hh"

Converter::Converter() {
		
}

Converter::~Converter() {
	
	//std::cout << "destructor" << std::endl;

}

void Converter::MakeHists() {
	
	std::string hname, htitle;
	
	// Loop over ISS modules
	for( int i = 0; i < common::n_module; ++i ) {
		
		// Loop over ASICs for the array
		for( int j = 0; j < common::n_asic; ++j ) {
			
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
				
				else hasic[i][j][k] = new TH1F( hname.data(), htitle.data(),
								4096, -0.5, 4095.5 );
				
			}
			
		}
		
	}
	
	// Loop over CAEN modules
	for( int i = 0; i < common::n_caen_mod; ++i ) {
		
		// Loop over channels of each CAEN module
		for( int j = 0; j < common::n_caen_ch; ++j ) {
			
			hname = "caen_" + std::to_string(i);
			hname += "_" + std::to_string(j);
			
			htitle = "Raw CAEN V1725 spectra for module " + std::to_string(i);
			htitle += ", channel " + std::to_string(j);
			
			htitle += ";Qlong;Counts";
			
			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hcaen[i][j] = (TH1F*)output_file->Get( hname.data() );
			
			else hcaen[i][j] = new TH1F( hname.data(), htitle.data(),
										   65536, -0.5, 65535.5 );
			
		}
					
	}

	
	return;
	
}

// Clean up function
void Converter::Initialise() {
	
	my_tm_stp = 0;
	my_info_field = 0;
	my_info_code = 0;
	my_type = 0;
	my_adc_data = 0;
	my_hit = 0;
	my_mod_id = 0;
	my_ch_id = 0;
	my_asic_id = 0;

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
	
	if( std::string(header_id) != "EBYEDATA" ) {
	
		std::cerr << "Bad header in block " << nblock << std::endl;
		exit(0);
	
	}
	
	return;
	
}


// Function to copy the main data from a DataSpy, for example
void Converter::SetBlockData( char *input_data ){
	
	// Copy header
	for( int i = 0; i < MAIN_SIZE ; i++ )
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
		
	
		// initialized... to ensure default value of zero when filling TTree
		Initialise();
		
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
				
				flag_caen_data = true;
				ProcessCAENData();
				
			}
#endif
		}
		
		// Information data
		else if( my_type == 0x2 ){
			
			ProcessInfoData();
			
		}
		
		// Trace header
		else if( my_type == 0x1 ){
			
			// TODO - contains the sample length
			nsamples = word_1 & 0xFFFF; // 16 bits from 0 in second word
			if( i < 25 && nblock < 10 ) log_file << "nsamples = " << nsamples << std::endl;
			continue;
			
			
		}
		
		// Trace sample
		else if( my_type == 0x0 ){
			
			// TODO - 4 x 14-bit samples in the 64-bit word
			continue;

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

void Converter::ProcessInfoData(){

	// MIDAS info data format
	my_info_field = word_0 & 0x000FFFFF; //bits 0:19
	my_mod_id = (word_0 >> 24) & 0x0000003F; //bits 24:29
	my_info_code = (word_0 >> 20) & 0x0000000F; //bits 20:23
	my_tm_stp = word_1 & 0x0FFFFFFF;  //bits 0:27

	// Fill tree
	FillTree();
	
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
	
	my_tm_stp = word_1 & 0x0FFFFFFF;  // 28 bits from 0
	
	
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

		// Fill tree
		FillTree();
	
	}
	
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
	
	// just set asic_id = 0 as a bodge until we have separate trees
	my_asic_id = 0;
	
	my_tm_stp = word_1 & 0x0FFFFFFF;  // 28 bits from 0

	// Check things make sense and only use Qlong
	if( my_mod_id >= common::n_caen_mod ||
		my_ch_id >= common::n_caen_ch ) {
		
		std::cout << "Bad CAEN event with mod_id=" << my_mod_id;
		std::cout << " ch_id=" << my_ch_id;
		std::cout << " data_id=" << my_data_id << std::endl;

	}

	else if( my_data_id == 0 ) {
		
		// Fill histograms
		hcaen[my_mod_id][my_ch_id]->Fill( my_adc_data );

		// Shift module number in the tree by number of ASIC/ISS modules
		my_mod_id += common::n_module;
		
		// Fill tree
		FillTree();

	}
	
	return;

}


void Converter::FillTree(){
	
	// Fill branches
	s_info.tm_stp_lsb	= my_tm_stp;
	s_info.field		= my_info_field;
	s_info.type			= my_type;
	s_info.code			= my_info_code;
	
	s_id.mod			= my_mod_id;
	s_id.asic			= my_asic_id;
	s_id.ch				= my_ch_id;

	s_adc.value			= my_adc_data;
	s_adc.hit			= my_hit;
	

	// Save this entry to TTree
	output_tree->Fill();

	return;
	
}


// Function to run the conversion for a single file
int Converter::ConvertFile( std::string input_file_name,
							 std::string output_file_name,
							 std::string log_file_name,
							 int start_block,
							 int end_block ) {
	
	// Read the file.
	std::ifstream input_file( input_file_name, std::ios::in|std::ios::binary );
	if( !input_file.is_open() ){
		
		std::cout << "Cannot open " << input_file_name << std::endl;
		return -1;
		
	}
	
	// Create log file.
	//std::ofstream log_file;
	//std::stringstream sslogs;
	log_file.open( log_file_name.c_str(), std::ios::out );
	
	// test Histogram ---- add more here!
	// TH1I *Hmidas_type = new TH1I("Hmidas_type","Hmidas_type",4,-0.5,3.5);
	
	// Create Root tree.
	output_file = new TFile( output_file_name.c_str(), "update" );
	output_tree = new TTree( "iss", "iss" );
	output_tree->Branch( "info_data", &s_info, "tm_stp_lsb/l:field/l:type/b:code/b");
	output_tree->Branch( "event_id",  &s_id,   "mod/b:asic/b:ch/b");
	output_tree->Branch( "adc_data",  &s_adc,  "value/s:hit/b");

	// Initialise
	MakeHists();
	Initialise();

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
	output_file->Write();
	//output_file->Print();
	output_file->Close();
	log_file.close();
	
	return BLOCKS_NUM;
	
}
