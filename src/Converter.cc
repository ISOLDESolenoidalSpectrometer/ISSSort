#include "Converter.hh"
#include "Common.hh"
#include "DutClass.hh"

#include <bitset>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <string>

#include <TFile.h>
#include <TTree.h>


//int p_conv_itr_max= 100;

void DoConvertion(std::string input_list_name, std::string output_file_name, std::string log_file_name, std::string histo_file_name){
	
	//to read list of input files
	std::string input_file_name;
	int n_input_files;
	
	
	std::ifstream input_list (input_list_name.c_str(), std::ios::in);
	if (input_list.is_open()){
		
		
		//Create log file.
		std::ofstream log_file;
		log_file.open(log_file_name.c_str(), std::ios::out);
		
		//Open Root file to save a few diagnostic histograms
		TFile* histo_file = new TFile(histo_file_name.c_str(),"update");
		
		//test Histogram ---- add more here!
		// TH1I * Hmidas_type= new TH1I("Hmidas_type","Hmidas_type",4,-0.5,3.5);
		
		
		// Create Root ntuple.
		TFile* output_file = new TFile(output_file_name.c_str(),"recreate");
		TTree* output_tree = new TTree("R3B_unpack","R3B_unpack");
		common::struct_entry_unpack s_entry;
		output_tree->Branch("entry_unpack", &s_entry.tm_stp_lsb,"tm_stp_lsb/l:info_field/l:adc_data/s:type/b:hit/b:mod_id/b:lad_id/b:side_id/b:asic_id/b:ch_id/b:info_code/b");
		//    output_file->cd();
		
		input_list >> n_input_files;
		
		if( n_input_files < 1 ) std::cout << "... invalid number of data input files for Converter step! N= " << n_input_files << std::endl;
		
		else std::cout << "... files for converter step: " << n_input_files << std::endl;
		
		//loop through each file to be processed
		for(int iter_f=0; iter_f<n_input_files ;iter_f++){
			
			input_list >> input_file_name;
			
			std::cout << "Converting file: " << input_file_name << std::endl;
			
			// Read the file.
			std::ifstream input_file (input_file_name.c_str(), std::ios::in|std::ios::binary);;
			
			if (input_file.is_open()){
				
				
				// Set the size of the block and its components.
				//const int HEADER_SIZE=24; // 24 byte
				//const int MAIN_SIZE=65496; // 65496 byte
				//const int EMPTY_SPACE_SIZE= 16; // 16 byte
				//const int BLOCK_SIZE= HEADER_SIZE + MAIN_SIZE + EMPTY_SPACE_SIZE; // 65536 byte
				const int HEADER_SIZE=24; // Size of header in bytes
				const int BLOCK_SIZE= 0x10000; //Max block size is 64kb. Amount of useful data given in header
				const int MAIN_SIZE= BLOCK_SIZE - HEADER_SIZE;   
				
				// Calculate the size of the file.
				input_file.seekg (0, input_file.end);
				int size_end = input_file.tellg();
				input_file.seekg (0, input_file.beg);
				int size_beg = input_file.tellg();
				int FILE_SIZE = size_end - size_beg;
				
				
				// Calculate the number of blocks in the file.
				int BLOCKS_NUM = FILE_SIZE / BLOCK_SIZE;
				
				
				//a sanity check for file size...
				//QQQ: add more strict test? 
				if( (FILE_SIZE%BLOCK_SIZE) != 0){
					std::cout << " *WARNIGN* Converter.cc:: missing some a data block? file, block, Nblock: " << FILE_SIZE << "  " << BLOCK_SIZE << "  "<< BLOCKS_NUM << std::endl << std::endl;
				}
				
				std::cout << " Converter.cc:: Blocks in file: #file, #block, #Nblock: " << FILE_SIZE << "  " << BLOCK_SIZE << "  "<< BLOCKS_NUM << std::endl << std::endl;
				log_file << " Converter.cc:: Blocks in file: #file, #block, #Nblock: " << FILE_SIZE << "  " << BLOCK_SIZE << "  "<< BLOCKS_NUM << std::endl << std::endl;
				
				
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
				//for (int itr_1=0; (itr_1< BLOCKS_NUM && itr_1 < p_conv_itr_max); itr_1++){
				for (int itr_1=0; itr_1< BLOCKS_NUM ; itr_1++){
					
					// Take one block each time
					// and analyze it.
					
					//AE: move this outside loop... just because!
					// Set the arrays for the block components.
					//  char block_header[HEADER_SIZE];
					//      char block_main[MAIN_SIZE];
					//      char block_empty_space[EMPTY_SPACE_SIZE];
					
					// Get the block.
					input_file.read((char*)&block_header, sizeof(block_header));
					input_file.read((char*)&block_data, sizeof(block_data));
					//AE: this read in block_data	input_file.read((char*)&block_empty_space, sizeof(block_empty_space));
					
					// Process header.
					unsigned char header_id[8]; // 8 byte. Must be this string 'EBYEDATA'.
					header_id[0] = block_header[0];
					header_id[1] = block_header[1];
					header_id[2] = block_header[2];
					header_id[3] = block_header[3];
					header_id[4] = block_header[4];
					header_id[5] = block_header[5];
					header_id[6] = block_header[6];
					header_id[7] = block_header[7];
					
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
					//header_DataLen = 
					//	(block_header[20] & 0xFF) << 24 | (block_header[21]& 0xFF) << 16 |
					//	(block_header[22] & 0xFF) << 8  | (block_header[23]& 0xFF) ;
					// *** BUT WHY? IF FREAKING ENDIAN IS RIGHT??
					header_DataLen=
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
					
					log_file << "== and in HEX:"<<std::hex<<std::endl;
					for(int iter=0;iter<24;iter++){
						log_file << int(block_header[iter]) << " ";
						if( (iter+1)%4==0 ) log_file << std::endl;
					}
					
					log_file << "== and in HEX (again!):"<<std::hex<<std::endl;
					for(int iter=0;iter<24;iter++){
						unsigned short int number;
						number = block_header[iter] & 0xFF;
						
						log_file << number << " ";
						if( (iter+1)%4==0 ) log_file << std::endl;
					}
					
					// Check endianess.
					if (header_MyEndian !=256 && itr_1<3 ){std::cout << "Endianess is correct!" << std::endl;}
					// Nota bene: I did not implent the code for correct endian. (?MB?)	  
					// Correct endianess.
					// (Byte sequence 4321 -> 1234).
					
					// Each block_main should contain a pair of words as such:
					// word_0 = 0xFFFFFFFF, word_1 = 0xFFFFFFFF.
					// This marks the point after which 
					// there are not more good-data in the block_main.
					// Cherck for each block_main the terminator is found.
					bool flag_terminator = false;
					
					
					// std::cout<< "size of data word (bytes): " <<std::dec <<sizeof(unsigned int)<<std::endl;
					
					// Process main.
					//for (int itr_2=0; itr_2< MAIN_SIZE; itr_2+=8)
					for (unsigned int itr_2=0; itr_2< header_DataLen; itr_2+=8)  // 8 to read 2x 32-bit (=2x 4-bytes) words
					{
						
						// It retrieves two 32-bit words each iteration;
						// It checks endieness and correct it.
						// Check if the pair of words are type-A or type-B.
						// If type-A then store it till new one comes.
						// If type-B then compute module-id, ASIC-id, channel-id, timestamp, ADC counts.
						
						// The information is grouped into 2 words of 32 bits (4 byte).
						unsigned int word_0; 
						unsigned int word_1;
						
						//AE: move outside loop...
						// Check endianess.
						//if (header_MyEndian !=256 ){std::cout << "Endianess is correct!" << std::endl; break;}
						// Nota bene: I did not implent the code for correct endian.
						// Correct endianess.
						// (Byte sequence 4321 -> 1234).
						
						// Assemble words from string of unsigned char.
						word_0 = (block_data[itr_2] & 0xFF) << 24 | (block_data[itr_2+1] & 0xFF) << 16 | 
						(block_data[itr_2+2] & 0xFF) << 8 | (block_data[itr_2+3] & 0xFF);
						
						// Assemble words from string of unsigned char.
						word_1 = (block_data[itr_2+4] & 0xFF) << 24 | (block_data[itr_2+5] & 0xFF) << 16 | 
						(block_data[itr_2+6] & 0xFF) << 8 | (block_data[itr_2+7] & 0xFF);
						
						
						
						// .... AGAIN: WHY? IF FREAKING ENDIAN IS RIGHT???
						// Assemble words from string of unsigned char.
						//ae	    word_0 = (block_data[itr_2] & 0xFF)  | (block_data[itr_2+1] & 0xFF) << 8 | 
						//ae	      (block_data[itr_2+2] & 0xFF) << 16 | (block_data[itr_2+3] & 0xFF) << 24;
						
						// Assemble words from string of unsigned char.
						//ae	    word_1 = (block_data[itr_2+4] & 0xFF) | (block_data[itr_2+5] & 0xFF) << 8 | 
						//ae	      (block_data[itr_2+6] & 0xFF) << 16 | (block_data[itr_2+7] & 0xFF) << 24;
						
						
						// Put the info in binary and dump it to file.
						// AE: do only for first few bits of data
						// only do it for first 25 blocks of data
						if(itr_2<750 && itr_1<10){
							std::bitset<32> _word_0(word_0);
							std::bitset<32> _word_1(word_1);
							
							log_file << "= Entry: " << (int) itr_2/8  << std::endl;
							log_file << "Word 0: " << _word_0 << "  "<< std::hex << "0x"<<word_0 << std::dec<<std::endl; // Must have first 2 bits: 10.
							log_file << "Word 1: " << _word_1 << "  "<< std::hex << "0x"<<word_1 << std::dec<< std::endl; // Must have first 4 bits: 0000.
							
							unsigned short int number;
							log_file << "= Entry (in Hex!): " << std::hex  << std::endl;
							
							for(int iter=0; iter<8; iter++){
								number = block_data[itr_2+iter] & 0xFF;
								log_file << number << " ";
								if( (iter+1)%4==0) log_file << std::endl; 
							}
						}
						
						// Check the trailer: reject or keep the block.
						//	  if( (word_0 & 0xFFFFFFFF) == 0xFFFFFFFF &&
						//  (word_1 & 0xFFFFFFFF) == 0xFFFFFFFF) {
						//AIDA format: only one FFFFFFFF word in end of block
						if( (word_0 & 0xFFFFFFFF) == 0xFFFFFFFF || (word_1 & 0xFFFFFFFF) == 0xFFFFFFFF){
							log_file << "End of block " << itr_1 << " ("<<std::hex<<word_0<<", "<<word_1<<")"<<std::dec<<std::endl; 
							flag_terminator=true;
							break; //QQQ: is this use of 'break' fail safe?
						}
						
						
						//initialized... to ensure default value of zero when filling TTree
						unsigned long my_tm_stp=0;
						unsigned long my_info_field=0;
						unsigned long my_adc_data=0;
						//	    unsigned long my_sample_lenght=0x0; // not used in ***R3B***, remove
						unsigned long my_type=0;
						unsigned long my_hit=0;
						unsigned long my_lad_id=0;
						unsigned long my_side_id=0;
						unsigned long my_mod_id=0;
						unsigned long my_ch_id=0;
						//	    unsigned long my_adc_range=0x0; // not used in ***R3B***, remove
						unsigned long my_info_code=0;
						unsigned long my_asic_id= 0; // new for R3B tracker
						
						my_type= (word_0 >> 30) & 0x3; 
						
						//ADC data
						if(my_type == 0x3){
							
							my_hit = ( word_0 >> 29 ) & 0x1;
							//***R3B***: change to bits 0:11!   +done+
							my_adc_data = word_0 & 0xFFF; //bits 0:11
							//byteN(word1, 3) << 8 | byteN(word1, 4);
							
							//***R3B***: ADCchannelIdent are bits 28:12
							//***R3B***: and there is no 'my_adc_range' in R3B.
							//***R3B***: mod_id= but 16:11, asic_id= bit 10:7, ch_id= bit 6:0
							unsigned int ADCchanIdent=  (word_0 >> 12) & 0x0001FFFF; //bits 28:12
							//(byteN(data1, 1) << 8 | byteN(data1, 2)) & 0x0FFF;
							my_mod_id = (ADCchanIdent >> 11) & 0x003F; // bits 16:11 
							my_lad_id = (my_mod_id >> 1) & 0x001F; // 5 bits  
							my_side_id = my_mod_id & 0x0001; // 5 bits  
							my_asic_id= (ADCchanIdent >> 7 ) & 0x000F; //bits 10:7
							my_ch_id = ADCchanIdent & 0x007F; //bits 0:6
							
							//  my_adc_range = (word_0 >> 28) & 0x00000001; //bit 28
							
							my_tm_stp = word_1 & 0x0FFFFFFF;  //bits 0:27
							
						}
						//Information data
						else if(my_type == 0x2){
							
							my_info_field= word_0 & 0x000FFFFF; //bits 0:19
							
							my_mod_id = (word_0 >> 24) & 0x0000003F; //bits 24:29 // !!! Only module ID of side 0 - (as one uses same info-field for side 1) !!!
							my_lad_id = (my_mod_id >> 1) & 0x001F; // 5 bits  
							
							my_info_code= (word_0 >> 20) & 0x0000000F; //bits 20:23
							
							my_tm_stp = word_1 & 0x0FFFFFFF;  //bits 0:27
							
						}
						//Sample trace: Sample Lenght
						//***R3B***: not used (I think!). remove this code, as well as waveform data (0x0)
						//    else if(my_type == 0x1){
						//  my_sample_lenght = word_0 & 0x0000FFFF; //bits 0:15
						//   unsigned int ADCchanIdent=  (word_0 >> 16) & 0x00000FFF; //bits 16:27
						//  my_mod_id = (ADCchanIdent >> 6) & 0x0000003F;
						//  my_ch_id = ADCchanIdent & 0x0000003F;
						//  my_tm_stp = word_1 & 0x0FFFFFFF;  //bits 0:27
						
						// }
						//Sample trace: Waveform data
						//  else if(my_type == 0x0){
						//   // 4 x 14 bits Waveform samples
						//  // use short and long words of struct_entry_midas to store this info
						//  my_tm_stp=  (word_0 >> 16) & 0x00003FFF;    //sample n
						//  my_info_field=  word_0 & 0x00003FFF;        //sample n+1
						//  my_adc_data=  (word_1 >> 16) & 0x00003FFF;  //sample n+2
						//  my_sample_lenght=  word_1  & 0x00003FFF;    //sample n+3
						//}
						else{
							//output error message!
							log_file << "WARNING: WRONG TYPE! word 0: " << word_0 << ", my_type: "<<my_type<<std::endl;
						}
						
						
						if(itr_2<750 && itr_1 < 10){   
							// Print the information.
							log_file << "Info:" << std::endl;
							
							// Put the info in binary and dump it to file.
							// std::bitset<64> _time_stamp(time_stamp);
							//	    std::bitset<32> _adc_counts(adc_counts);
							//	    std::bitset<32> _channel_id(channel_id);
							//	    std::bitset<32> _asic_id(asic_id);
							//	    std::bitset<32> _module_id(module_id);
							
							//	    log_file << "_time_stamp: " << _time_stamp << std::endl;
							//	    log_file << "_adc_counts: " << _adc_counts << std::endl;
							//	    log_file << "_channel_id: " << _channel_id << std::endl;
							//	    log_file << "_asic_id: " << _asic_id << std::endl;
							//	    log_file << "_module_id: " << _module_id << std::endl;
							
							// It dumps  the info to file, in dec_base.
							
							log_file <<  "type: " << std::dec<< int(my_type) << std::hex<<" 0x"<< int(my_type)<<std::endl;
							log_file <<  "hit: " << std::dec<< int(my_hit) << std::hex<<" 0x"<< int(my_hit)<<std::endl;
							log_file<< "module id: " << std::dec<< int(my_mod_id) << std::hex<<" 0x"<< int(my_mod_id)<<std::endl;
							log_file<< "ladder id: " << std::dec<< int(my_lad_id) << std::hex<<" 0x"<< int(my_lad_id)<<std::endl;
							log_file<< "side id: " << std::dec<< int(my_side_id) << std::hex<<" 0x"<< int(my_side_id)<<std::endl;
							log_file<< "asic id: " << std::dec<< int(my_asic_id) << std::hex<<" 0x"<< int(my_asic_id)<<std::endl;
							log_file<< "channel id: " << std::dec<< int(my_ch_id) << std::hex<<" 0x"<< int(my_ch_id)<<std::endl;
							log_file<< "time stamp(LSB): " << std::dec<< my_tm_stp << std::hex<<" 0x"<< my_tm_stp<<std::endl;
							log_file<< "ADC data: " << std::dec<< my_adc_data << std::hex<<" 0x"<< my_adc_data<<std::endl;
							//  log_file<< "sample lenght: " << std::dec<< my_sample_lenght << std::hex<<" 0x"<< my_sample_lenght<<std::endl;
							log_file<< "info code: " << std::dec<< int(my_info_code) << std::hex<<" 0x"<< int(my_info_code)<<std::endl;
							log_file<< "info field: " << std::dec<< my_info_field << std::hex<<" 0x"<< my_info_field<<std::endl;
							
							log_file << std::dec << std::endl;
							//	    log_file << "adc_counts: " << adc_counts  << std::endl;
							//log_file << "channel_id: " << channel_id  << std::endl;
							//log_file << "asic_id: " << asic_id << std::endl;
							//log_file << "module_id: " << module_id << std::endl;
						}
						
						// Fill Ntuple.
						s_entry.tm_stp_lsb = my_tm_stp; // & 0xFFFFFFFF;
						s_entry.info_field = my_info_field; // & 0xFFFFFFFF;
						s_entry.adc_data = my_adc_data; // & 0x0000FFFF;
						// s_entry.sample_lenght = my_sample_lenght & 0x0000FFFF;
						
						s_entry.type = my_type; // & 0x00FF;
						s_entry.hit = my_hit;
						s_entry.mod_id = my_mod_id; // & 0x00FF;
						s_entry.lad_id = my_lad_id; // & 0x00FF;
						s_entry.side_id = my_side_id; // & 0x00FF;
						
						s_entry.asic_id = my_asic_id;
						s_entry.ch_id = my_ch_id; // & 0x00FF;
						// s_entry.adc_range = my_adc_range & 0x00FF;
						s_entry.info_code = my_info_code; // & 0x00FF;
						
						
						//
						// Now collect data from this entry
						//
						//save this entry to TTree
						output_tree->Fill();
						
						//fill histograms
						//            Hmidas_type->Fill(s_entry.type);
						
						//}// End of: if ( (word_0 & 0xC0000000)==0xC0000000 )
						
					}// End of: itr_2 loop.
					
					// Check if terminator sequence is found in each block.
					if (!flag_terminator){
						//search for terminator sequence in next few points
						
						for (int itr_3=header_DataLen; itr_3< MAIN_SIZE; itr_3+=4){  // to read 1x 32bit word (= a 1x 4-byte word)
							
							unsigned int word_0; 
							
							word_0 = (block_data[itr_3] & 0xFF) << 24 | (block_data[itr_3+1] & 0xFF) << 16 | 
							(block_data[itr_3+2] & 0xFF) << 8 | (block_data[itr_3+3] & 0xFF);
							
							
							if( (word_0 & 0xFFFFFFFF) == 0xFFFFFFFF  || (word_0 & 0xFFFFFFFF) == 0x5E5E5E5E){  // To do: Ask Vic for 5E5E5E5E !
								if(!flag_terminator)log_file << "End of block " << itr_1 << " (after reading good data)"<<std::endl; 
								flag_terminator=true;
							}
							
							else if(flag_terminator==false){
								//good data still in buffer?
								std::cout << " ***Warning: Data after end of block! " << itr_1 << " word_0= "; 
								printf(" %X \n", word_0);
								
								log_file << " ***Warning: Data after end of block! " << itr_1 << " word_0= " << word_0 << std::endl; 
							}
							
							else{
								log_file << " 5e5e5e5e? " << std::hex << " 0x" << word_0<< ", " << std::dec;
							}
							
						}
						
						log_file << std::endl;
					}//if(!flag_terminator)
					
					//check once more after going over left overs.... 
					if(!flag_terminator){
						std::cout << "\nERROR - Terminator sequence not found in block_data." << std::endl; break;}
					
					
				}// End of: itr_1 loop.
				
				
				input_file.close(); 
				std::cout << "\n...done with file "<<iter_f<<" ." << std::endl;
				
			}// End of: "if is_open" for input_file
			else{std::cout << "... cannot open file!"<< std::endl;}
			
		} //for( iter_f<n_input_list
		
		output_file->Write("R3B_unpack");
		output_file->Print(); 
		
		
		//   histo_file->cd();
		//write histograms (etc...) to common root file
		// histo_file->Write("XXXX");
		//    histo_file->Write("Hmidas_type");
		histo_file->Close();
		
		output_file->Close(); 
		log_file.close();
		
		input_list.close();
		
		// house-keeping -> Segmentation fault if I try to do this. Is 'delete' being done by closing the file?
		//delete Hmidas_type;
		
	} // End of: "if is_open" for input_list
	else{std::cout << "... cannot open file with list of input data for this step!"<< std::endl;}
	
}

void Converter (DutClass* dut){
	
	// Do convertion of ASCII files ?
	if ( !dut->GetFlagConvertFile() ) {return;}
	
	std::cout << " +++ R3B Analysis:: processing Converter step ..."<<std::endl;
	
	// Loop over the runs, make name for input and output files.
	// Pass the names to function "DoConvertion".
	
	dut->SetFlagFirstInPipe(false);
	
	//write names of input files to a temporary text file
	std::string input_list_name;
	input_list_name= dut->GetDataInputList() ; 
	std::ofstream input_list (input_list_name.c_str(), std::ios::out|std::ios::trunc);
	
	//if list of input files to be read from text file
	if(dut->GetFlagInputType()){
		
		
		//user provided file with list of input data files 
		std::string user_list_name;
		user_list_name= dut->GetDataListUser() ; 
		std::ifstream user_list (user_list_name.c_str(), std::ios::in);
		
		std::cout <<  "File list from file.... "<<user_list_name.c_str() << std::endl;
		
		
		if(user_list.is_open()){
			int n_files=0;
			std::string in_line;
			
			
			
			while( getline( user_list, in_line) ){
				//first loop count number of non empty lines
				if(!in_line.empty() && '\n' != in_line[0]){
					n_files++;
					// input_list << in_line << '\n';
				}
			}
			
			
			
			input_list << n_files << '\n';
			
			user_list.close();
			user_list.open(user_list_name.c_str(), std::ios::in);
			
			while( getline( user_list, in_line) ){
				//skip eempty lines (only '\n'
				if(!in_line.empty() && '\n' != in_line[0]){
					//n_files++;
					input_list << in_line << '\n';
				}
				std::cout << "  line " << in_line << " !!! ..."<< std::endl;   
			}
			
			
			
			user_list.close();
			std::cout << "  Read " << n_files << " from user list ..."<< std::endl;   
		}
		else{ // !user_list.is_open()
			std::cout << "... cannot open user provided file with input list!"<< std::endl;
		}
		
	}
	
	//if input files are sequential from a initial prefix
	else{
		
		std::cout <<  "Files list from iteration.... "<< std::endl;
		
		
		input_list << dut->GetRunTotal() << '\n';
		
		for (int itr_run=dut->GetRunFirst(); itr_run< ( dut->GetRunTotal()+dut->GetRunFirst() ); itr_run++){
			
			std::stringstream name_input_file;
			name_input_file << dut->GetDataInputPath()
			<< dut->GetDataFilePrefix()
			<< "_"
			<< itr_run;
			
			std::cout <<  "Files in input: "<<name_input_file.str() << std::endl;
			
			input_list << name_input_file.str() << '\n';
		}
		
	}
	
	input_list.close();
	
	//std::string input_list_name;
	//input_list_name= dut->GetDataInputList() ;
	
	std::stringstream name_output_file;
	name_output_file << dut->GetDataOutputPath()
	<< dut->GetDataOutputPrefix()
	<< "_unpack.root";
	
	std::stringstream name_log_file;
	name_log_file << dut->GetDataOutputPath()
	<< dut->GetDataOutputPrefix()
	<< "_unpack_log.txt";
	
	
	std::stringstream name_histo_file;
	name_histo_file << dut->GetDataOutputPath()
	<< dut->GetDataOutputPrefix()
	<< "_histo.root";
	
	
	DoConvertion(input_list_name.c_str(), name_output_file.str(), name_log_file.str(), name_histo_file.str());
	
	
}
