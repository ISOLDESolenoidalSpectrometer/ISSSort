#include "Converter.hh"

Converter::Converter() {
	
	MakeHists();
	Initialise();
	
}

Converter::~Converter() {
	
	//std::cout << "destructor" << std::endl;

}

void Converter::MakeHists() {
	
	std::string hname, htitle;
	
	// Loop over modules
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
				
				hasic[i][j][k] = new TH1F( hname.data(), htitle.data(),
								4096, -0.5, 4095.5 );
				
			}
			
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


// Function to run the conversion for a single file
void Converter::ConvertFile( std::string input_file_name,
							 std::string output_file_name,
							 std::string log_file_name ){
	
	// Read the file.
	std::ifstream input_file( input_file_name, std::ios::in|std::ios::binary );
	if( !input_file.is_open() ){
		
		std::cout << "Cannot open " << input_file_name << std::endl;
		return;
		
	}
	
	// Create log file.
	std::ofstream log_file;
	std::stringstream sslogs;
	log_file.open( log_file_name.c_str(), std::ios::out );
	
	// test Histogram ---- add more here!
	// TH1I *Hmidas_type = new TH1I("Hmidas_type","Hmidas_type",4,-0.5,3.5);
	
	// Create Root tree.
	TFile *output_file = new TFile( output_file_name.c_str(), "recreate" );
	TTree *output_tree = new TTree( "iss", "iss" );
	output_tree->Branch( "info_data", &s_info, "tm_stp_lsb/l:field/l:type/b:code/b");
	output_tree->Branch( "event_id",  &s_id,   "mod/b:asic/b:ch/b");
	output_tree->Branch( "adc_data",  &s_adc,  "value/s:hit/b");

	// Initialise histograms
	MakeHists();
	
	// Conversion starting
	std::cout << "Converting file: " << input_file_name << std::endl;
	
	// Set the size of the block and its components.
	//const int HEADER_SIZE=24; // 24 byte
	//const int MAIN_SIZE=65496; // 65496 byte
	//const int EMPTY_SPACE_SIZE= 16; // 16 byte
	//const int BLOCK_SIZE= HEADER_SIZE + MAIN_SIZE + EMPTY_SPACE_SIZE; // 65536 byte
	const int HEADER_SIZE = 24; // Size of header in bytes
	const int BLOCK_SIZE = 0x10000; //Max block size is 64kb. Amount of useful data given in header
	const int MAIN_SIZE = BLOCK_SIZE - HEADER_SIZE;
	
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
	
	// The information is split into 2 words of 32 bits (4 byte).
	// The words can by of two types: A or B.
	// Type A words have word_0 with msb=10.
	// Type B words have word_0 with msb=11.
	//    unsigned int word_0A;
	//    unsigned int word_1A;
	//    unsigned int word_0B;
	//    unsigned int word_1B;
	
	// Set the arrays for the block components.
	char block_header[HEADER_SIZE];
	char block_data[MAIN_SIZE];
	
	// Loop over all the blocks.
	for( int itr_1 = 0; itr_1 < BLOCKS_NUM ; itr_1++ ){
		
		// Take one block each time and analyze it.
		if( itr_1 % 200 == 0 || itr_1+1 == BLOCKS_NUM ) {
			
			std::cout << " " << std::setw(8) << std::setprecision(4);
			std::cout << (float)(itr_1+1)*100.0/(float)BLOCKS_NUM << "%\r";
			std::cout.flush();
			
		}

		// Get the block.
		input_file.read( (char*)&block_header, sizeof(block_header) );
		input_file.read( (char*)&block_data, sizeof(block_data) );
		
		// Process header.
		unsigned char header_id[8]; // 8 byte. Must be this string 'EBYEDATA'.
		for( int itr_2 = 0; itr_2 < 8 ; itr_2++ )
			header_id[itr_2] = block_header[itr_2];
		
		unsigned int header_sequence; // 4 byte.
		header_sequence =
		(block_header[8] & 0xFF) << 24 | (block_header[9]& 0xFF) << 16 |
		(block_header[10]& 0xFF) << 8  | (block_header[11]& 0xFF);
		
		unsigned short int header_stream; // 2 byte.
		header_stream = (block_header[12] & 0xFF) << 8 | (block_header[13]& 0xFF);
		
		unsigned short int header_tape; // 2 byte.
		header_tape = (block_header[14] & 0xFF) << 8 | (block_header[15]& 0xFF);
		
		unsigned short int header_MyEndian; // 2 byte. If 256 then correct endianess.
		header_MyEndian = (block_header[16] & 0xFF) << 8 | (block_header[17]& 0xFF);
		
		unsigned short int header_DataEndian; // 2 byte.
		header_DataEndian = (block_header[18] & 0xFF) << 8 | (block_header[19]& 0xFF);
		
		unsigned int header_DataLen; // 4 byte.
		header_DataLen =
		(block_header[20] & 0xFF) | (block_header[21]& 0xFF) << 8 |
		(block_header[22] & 0xFF) << 16  | (block_header[23]& 0xFF) << 24 ;
		
		
		// Print header info.
		log_file << "== DATA BLOCK: " << itr_1 << std::endl;
		log_file << "Header_id: " << header_id << std::endl;
		log_file << "Header_sequence: " << header_sequence << std::endl;
		log_file << "Header_stream: " << header_stream << std::endl;
		log_file << "Header tape: " << header_tape << std::endl;
		log_file << "Header_MyEndian: " << header_MyEndian << std::endl;
		log_file << "Header_DataEndian: " << header_DataEndian << std::endl;
		log_file << "Header_DataLen: " << header_DataLen << std::endl;
		
		log_file << "== and in HEX:" << std::hex << std::endl;
		for( int iter = 0; iter < 24; iter++ ) {
			
			log_file << int( block_header[iter] ) << " ";
			if( (iter+1)%4 == 0 ) log_file << std::endl;
			
		}
		
		log_file << "== and in HEX (again!):" << std::hex << std::endl;
		for( int iter = 0; iter < 24; iter++ ){
			
			unsigned short int number;
			number = block_header[iter] & 0xFF;
			
			log_file << number << " ";
			if( (iter+1) % 4 == 0 ) log_file << std::endl;
			
		}
		
		// Check endianess.
		if( header_MyEndian != 256 && itr_1 < 3 )
			std::cout << "Endianess is correct!" << std::endl;
		// Nota bene: I did not implent the code for correct endian. (?MB?)
		// Correct endianess.
		// (Byte sequence 4321 -> 1234).
		
		// Each block_main should contain a pair of words as such:
		// word_0 = 0xFFFFFFFF, word_1 = 0xFFFFFFFF.
		// This marks the point after which
		// there are not more good-data in the block_main.
		// Check for each block_main the terminator is found.
		bool flag_terminator = false;

		// Process main ... 8 to read 2x 32-bit (=2x 4-bytes) words
		for( unsigned int itr_2 = 0; itr_2 < header_DataLen; itr_2 += 8 ) {
			
			// Data format here: http://npg.dl.ac.uk/documents/edoc504/edoc504.html
			// It retrieves two 32-bit words each iteration;
			// It checks endieness and correct it.
			// Check if the pair of words are type-A or type-B.
			// If type-A then store it till new one comes.
			// If type-B then compute module-id, ASIC-id, channel-id, timestamp, ADC counts.
			
			// The information is grouped into 2 words of 32 bits (4 byte).
			unsigned int word_0;
			unsigned int word_1;
			
			// Assemble words from string of unsigned char.
			word_0 = (block_data[itr_2] & 0xFF) << 24 | (block_data[itr_2+1] & 0xFF) << 16 |
			(block_data[itr_2+2] & 0xFF) << 8 | (block_data[itr_2+3] & 0xFF);
			
			// Assemble words from string of unsigned char.
			word_1 = (block_data[itr_2+4] & 0xFF) << 24 | (block_data[itr_2+5] & 0xFF) << 16 |
			(block_data[itr_2+6] & 0xFF) << 8 | (block_data[itr_2+7] & 0xFF);
			
			// Put the info in binary and dump it to file.
			// AE: do only for first few bits of data
			// only do it for first 25 blocks of data
			if( itr_2 < 750 && itr_1 < 10 ){
				
				std::bitset<32> _word_0(word_0);
				std::bitset<32> _word_1(word_1);
				
				log_file << "= Entry: " << (int) itr_2/8  << std::endl;
				log_file << "Word 0: " << _word_0 << "  "<< std::hex << "0x"<<word_0 << std::dec << std::endl; // Must have first 2 bits: 10.
				log_file << "Word 1: " << _word_1 << "  "<< std::hex << "0x"<<word_1 << std::dec << std::endl; // Must have first 4 bits: 0000.
				
				unsigned short int number;
				log_file << "= Entry (in Hex!): " << std::hex  << std::endl;
				for( int iter = 0; iter < 8; iter++ ){
					
					number = block_data[itr_2+iter] & 0xFF;
					log_file << number << " ";
					if( (iter+1)%4 == 0 ) log_file << std::endl;
					
				}
				
			}
			
			// Check the trailer: reject or keep the block.
			if( (word_0 & 0xFFFFFFFF) == 0xFFFFFFFF ||
			    (word_1 & 0xFFFFFFFF) == 0xFFFFFFFF ) {
				
				log_file << "End of block " << itr_1 << " (" << std::hex;
				log_file << word_0 << ", " << word_1 << ")" << std::dec << std::endl;
				flag_terminator = true;
				break; //QQQ: is this use of 'break' fail safe?
				
			}
			
			// initialized... to ensure default value of zero when filling TTree
			Initialise();
			
			my_type = ( word_0 >> 30 ) & 0x3;
			
			// ADC data
			if( my_type == 0x3 ){
				
				my_hit = ( word_0 >> 29 ) & 0x1;
				my_adc_data = word_0 & 0xFFF; //bits 0:11
				
				//***R3B***: ADCchannelIdent are bits 28:12
				//***R3B***: and there is no 'my_adc_range' in R3B.
				//***R3B***: mod_id= bit 16:11, asic_id= bit 10:7, ch_id= bit 6:0
				unsigned int ADCchanIdent = (word_0 >> 12) & 0x0001FFFF; //bits 28:12
				//(byteN(data1, 1) << 8 | byteN(data1, 2)) & 0x0FFF;
				my_mod_id = (ADCchanIdent >> 11) & 0x003F; // bits 16:11
				my_asic_id = (ADCchanIdent >> 7 ) & 0x000F; //bits 10:7
				my_ch_id = ADCchanIdent & 0x007F; //bits 0:6
				
				//  my_adc_range = (word_0 >> 28) & 0x00000001; //bit 28
				
				my_tm_stp = word_1 & 0x0FFFFFFF;  //bits 0:27
				
			}
			
			// Information data
			else if( my_type == 0x2 ){
				
				my_info_field = word_0 & 0x000FFFFF; //bits 0:19
				my_mod_id = (word_0 >> 24) & 0x0000003F; //bits 24:29
				my_info_code = (word_0 >> 20) & 0x0000000F; //bits 20:23
				my_tm_stp = word_1 & 0x0FFFFFFF;  //bits 0:27
				
			}
			
			else {
				
				// output error message!
				log_file << "WARNING: WRONG TYPE! word 0: " << word_0;
				log_file << ", my_type: " << my_type << std::endl;
			
			}
			
			
			if( itr_2 < 750 && itr_1 < 10 ){
				
				// Print the information.
				log_file << "Info:" << std::endl;
				
				log_file << "type: " << std::dec << int(my_type) << std::hex <<" 0x" << int(my_type)<<std::endl;
				log_file << "hit: " << std::dec << int(my_hit) << std::hex<<" 0x"<< int(my_hit)<<std::endl;
				log_file << "module id: " << std::dec << int(my_mod_id) << std::hex<<" 0x"<< int(my_mod_id)<<std::endl;
				log_file << "asic id: " << std::dec << int(my_asic_id) << std::hex<<" 0x"<< int(my_asic_id)<<std::endl;
				log_file << "channel id: " << std::dec << int(my_ch_id) << std::hex<<" 0x"<< int(my_ch_id)<<std::endl;
				log_file << "time stamp(LSB): " << std::dec << my_tm_stp << std::hex<<" 0x"<< my_tm_stp<<std::endl;
				log_file << "ADC data: " << std::dec << my_adc_data << std::hex<<" 0x"<< my_adc_data<<std::endl;
				log_file << "info code: " << std::dec << int(my_info_code) << std::hex<<" 0x"<< int(my_info_code)<<std::endl;
				log_file << "info field: " << std::dec << my_info_field << std::hex<<" 0x"<< my_info_field<<std::endl;
				
				log_file << std::dec << std::endl;

			}
			
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
			
			// Check things make sense
			if( my_mod_id >= common::n_module || my_mod_id < 0 ||
			    my_asic_id >= common::n_asic || my_asic_id < 0 ||
			    my_ch_id >= common::n_channel || my_ch_id < 0 ) {
				
				continue;
				
			}

			// Save this entry to TTree
			output_tree->Fill();
			
			// Fill histograms
			if( my_type == 0x3 )
				hasic[my_mod_id][my_asic_id][my_ch_id]->Fill( my_adc_data );
			
		} // End of: itr_2 loop.
		
		// Check if terminator sequence is found in each block.
		if( !flag_terminator ){
		
			unsigned int word_0;

			// search for terminator sequence in next few points
			// to read 1x 32bit word (= a 1x 4-byte word)
			for( int itr_3 = header_DataLen; itr_3 < MAIN_SIZE; itr_3 += 4 ){
				
				word_0 = (block_data[itr_3] & 0xFF) << 24 | (block_data[itr_3+1] & 0xFF) << 16 |
				(block_data[itr_3+2] & 0xFF) << 8 | (block_data[itr_3+3] & 0xFF);
				
				// Found it?
				if( ( word_0 & 0xFFFFFFFF ) == 0xFFFFFFFF ||
				    ( word_0 & 0xFFFFFFFF ) == 0x5E5E5E5E ){  // To do: Ask Vic for 5E5E5E5E !

					if( !flag_terminator )
						log_file << "End of block " << itr_1 << " (after reading good data)" << std::endl;

					flag_terminator = true;

				}
				
				else if( !flag_terminator ){
					
					// good data still in buffer?
					sslogs << " ***Warning: Data after end of block! " << itr_1 << " word_0 = ";
					sslogs << std::hex << " 0x" << word_0 << std::endl;
					
					std::cout << sslogs.str();
					log_file << sslogs.str();
					sslogs.str( std::string() ); // clean up

				}
				
				else
					log_file << " 5e5e5e5e? " << std::hex << " 0x" << word_0 << ", " << std::dec;

				
			}
			
			log_file << std::endl;
			
		} // if(!flag_terminator)
		
		// Check once more after going over left overs....
		if( !flag_terminator ){

			std::cout << std::endl << __PRETTY_FUNCTION__ << std::endl;
			std::cout << "\tERROR - Terminator sequence not found in block_data.\n";
			break;
			
		}
		
		
	} // End of: itr_1 loop.
	
	input_file.close();
	output_file->Write();
	//output_file->Print();
	output_file->Close();
	log_file.close();
	
	return;
	
}
