#include "Calibrator.hh"
#include "Common.hh"
#include "DutClass.hh"

//#include <bitset>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <string>

#include <TFile.h>
#include <TTree.h>

double p_cali_gain; //[common::n_module][common::n_side][common::n_asic][common::n_channel]={{{{0}}}};
double p_cali_offset; //[common::n_module][common::n_side][common::n_asic][common::n_channel]={{{{0}}}};
//int p_cali_geom_detector[common::n_module][common::n_side]={{0}}; // [det][side] -> location in array
//int p_cali_geom_strip[common::n_module]={0}; // [strip] -> ??? position mm ???

// which FEE64 modules are running...
//bool p_cali_mod_enable[common::n_module]={false};
//bool p_cali_strip_enable[common::n_module][common::n_side][common::n_asic][common::n_channel]={{{{false}}}}; //do we want to mask out some strips

double p_cali_time;

void LoadParametersCali(int n_module, std::string file_name){
	
	std::cout << "Loading parameters for stack of "<<n_module<<" DSSDs. Currently only default parameters implemented"
	<< "\n     hopefuly we can load parameters from file soon!!!"<<std::endl;
	
	//all enabled
	
	//geometry of stack->FEE64 modules
	//for(int i=0;i<common::n_module;i++){
	// p_cali_mod_enable[i]= true; // enable all
	//    p_cali_geom_detector[i]= i; //(i-1)/4; // sequentially numbered DSSDs in stack: 0,1,2...? (make 1,2,3.. and 0 garbage?)
	//   p_cali_geom_strip[i]= 0; //(i-1)%4; // 0,1,2,3,0,1,2,3,0, ...
	//}
	
	p_cali_gain= 1;
	p_cali_offset=0;
	//  for(int i=0; i<common::n_module; i++){
	//  for(int j=0; j<128; j++){
	//    p_cali_gain[i][j]=my_gain;
	//    p_cali_offset[i][j]=my_offset;
	//  }
	// }
	
	p_cali_time=1.e-3 ; //convert clocks to usec
	
	return;
}


void DoCalibrate(std::string input_list_name, std::string output_file_name, std::string log_file_name, std::string histo_file_name){
	
	
	bool flag_debug= true;
	
	//this solution didn't work!
	//parameter::struct_parameter_cali p_cali;
	//int este=5;
	//LoadParameterCali(este);
	//std::cout << "\n\nDID it WORK??? "<<std::endl;
	
	int MapDet[3][16]={ {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3},   // STUB 0, 1, 2, & 3  // -1 nothing
		{4, 4, 4, 4,                                       // 
			5, 5,                                             // dE/E
			6, 6, 6, 6, 6, 6, 6, 6,                           // Recoil
			-1, -1},                                          // Spare
		{7, 7, 7, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}};     // Luminosity S1 (4 quadrants) det: 7 Spare 
	
	int MapSig[3][16]={{0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3},   // STUB
		{0, 1, 2, 3,                                        
			0, 1,                                              // dE/E
			0, 1, 2, 3, 4, 5, 6, 7,                            // Recoil
			-1, -1},                                           // Spare
		{0, 1, 2, 3, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}};// Luminisity S1 (4 quadrants) & Spare
	
	/* for experiment: 
	 int MapDet[4][16]={ {0, 0, -1, -1, 0, -1, 0, 1, 1, 1, 1, 2, 2, -1, -1, -1},     // STUB 0, 1, 2, & 3  //  Spare is -1
	 {2, 2, 3, 3, 3, 3, -1, 4, 4, 4, 4, 4, 4, 4, 4}              // Recoil is 4
	 {5, 5, 5, 5, 6, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1}};    // Luminosity S1 det: 5;  // ; dE/E is 6 ; Spare= -1 
	 
	 int MapSig[4][16]={ {1, 0, -1, -1, 3, -1, 2, 1, 0, 3, 2, 1, 0, -1, -1, -1},     // STUB: 0 is X1; 1 is X2, 2 is Energy; 3 is Guard
	 {3, 2, 1, 0, 3, 2, -1, 1, 0, 1, 0, 0, 1, 0, 1, -1},                                // Recoil (0 si for dE, 1 is for E)          
	 {0, 1, 2, 3, 0, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}};        // Luminisity S1 (0,1,2,3) & DE-E (0 for dE, 1 for E)
	 */
	
	
	
	LoadParametersCali(common::n_module, histo_file_name);
	//  std::cout << " Loaded parameters?: "<<  p_cali_mod_enable[2] << " "<<p_cali_geom_detector[2] << " "<<p_cali_geom_strip[2]<<std::endl;
	// std::cout <<p_cali_gain[2][0] << " "<<p_cali_offset[2][0]<< " "<<p_cali_polarity[2][0]<< " "<<p_cali_scale_low[2][0]
	//	    << " "<<p_cali_scale_high[0][0]<<std::endl;
	
	
	//to read list of input files
	std::string input_file_name;
	int n_input_files;
	
	std::ifstream input_list (input_list_name.c_str(), std::ios::in);
	if (input_list.is_open()){
		
		
		//Create log file.
		std::ofstream log_file;
		log_file.open(log_file_name.c_str(), std::ios::out);
		
		//Open Root file to save a few diagnostic histograms
		//TFile* histo_file = new TFile(histo_file_name.c_str(),"update");
		//---- declare histograms here!  ----//
		//TH1I * Hhistogram= new TH1I("Hhistogram","my histogram;x [ch]",10,-0.5,9.5);
		//output_file->cd(); 
		
		
		// Create Root ntuple.
		TFile* output_file = new TFile(output_file_name.c_str(),"recreate");
		TTree* output_tree = new TTree("ISS_calib","ISS_calib");
		common::struct_entry_calibrateSTUB s_entry;
		output_tree->Branch("entry_calib", &s_entry.energy,"energy/D:time_stamp/D:BPulseNb/L:detector/I:signal/I:sync_flag/O");
		
		
		input_list >> n_input_files;
		
		if( n_input_files < 1) std::cout << "... invalid number of data input files for Calibrate step! N= "<< n_input_files <<std::endl;
		
		//loop through each file to be processed
		for(int iter_f=0;iter_f<n_input_files;iter_f++){
			
			// Read name and open next Root input file.
			input_list >> input_file_name;
			std::cout << "Calibrating file: " << input_file_name << std::endl;
			TFile* input_file = new TFile(input_file_name.c_str(),"read");
			
			if (input_file != 0){
				
				TTree *input_tree= (TTree*) input_file->Get("ISS_sort");
				
				common::struct_entry_sortSTUB s_entry_sort;
				input_tree->SetBranchAddress("entry_sort",&s_entry_sort,0);
				
				long long n_entries;
				n_entries = input_tree->GetEntries();
				
				std::cout << "\n\n Calibrating: number of entries in input tree= " << n_entries << ", for file " << input_file_name << std::endl;
				
				//for structure struct_entry_calibrate
				double my_energy;
				double my_time_stamp;
				int my_ADCmodule;
				int my_channel;
				int my_BPulseNb;
				int my_detector;
				int my_signal;
				bool my_sync_flag;
				
				//arrays to check synchronizatino is going fine (or not!)
				//unsigned long tm_stp_msb_modules[common::n_module]={0};
				
				bool fill_flag;
				
				//	double my_time_offset;
				
				//
				// Main loop over TTree to process raw MIDAS data entries
				//
				
				
				for(long long i=0; i<n_entries; i++){
					
					//default values for struct_entry_sort
					my_energy= -1;
					my_time_stamp= -1;
					//	  my_time_ext= -1;
					my_BPulseNb= -1;
					my_ADCmodule= -1;
					my_channel= -1;
					my_detector= -1;
					my_signal= -1;
					my_sync_flag= false;
					
					fill_flag=true; // *R3B* no selection by type (section commented out bellow)
					
					input_tree->GetEntry(i);
					// --- check values will not make code crash!! e.g. s_entry_midas.det_id within boundaries!
					
					/*    do something with it!  */
					
					
					//------------------------------------------------------------------
					//
					//   *R3B* all same types: skip this part (select types to store)
					//
					//-----------------------------------------------------------------
					/***********
					 if(s_entry_sort.type==0){ //low range of amplifier (decay)
					 my_type= 0; fill_flag=true;
					 }
					 else if(s_entry_sort.type==1){ //high range of ampifier (implant)
					 my_type= 1; fill_flag=true;
					 }
					 else if(s_entry_sort.type==16){ //discriminator: skip for now
					 my_type= 2; fill_flag=false;
					 }
					 else if(s_entry_sort.type==18){ //MSB(EXT) info code: we can remove it after code well debugged?
					 my_type= 3; fill_flag=true;
					 //get offset AIDA clock<->EXT clock
					 my_time_offset = s_entry_sort.info*p_cali_time_ext-s_entry_sort.tm_stp;
					 }
					 else{ //skip all others
					 fill_flag=false;
					 }
					 *********************/
					
					
					
					if(fill_flag){
						
						//------------------------------------------------------------------
						//
						//   calculate timing of event: AIDA, and EXT with offset
						//
						//-----------------------------------------------------------------
						my_time_stamp= p_cali_time*s_entry_sort.tm_stp;
						
						
						//--------------------------------------------------------------
						//
						//    Calibrated energy: *R3B* only one range
						//
						//-----------------------------------------------------------
						
						// e0=gain*ADC+offset (gain matching)
						my_energy= p_cali_gain*s_entry_sort.adc_data + p_cali_offset;
						
						//--------------------------------------------------------------
						//   
						//    Geometry calibration
						//
						//-----------------------------------------------------------
						
						//my_detector= MapDet[s_entry_sort.det_id];
						my_BPulseNb= s_entry_sort.BPulseNb;
						
						//my_detector= MapDet[s_entry_sort.det_id];
						my_ADCmodule= s_entry_sort.adc_mod;
						my_channel= s_entry_sort.ch_id;
						
						//std::cout << "my_ADCmodule=" <<  my_ADCmodule << std::endl;
						//std::cout << "my_channel=" <<  my_channel << std::endl;
						//std::cout << "my_detector=" <<  MapDet[my_ADCmodule][my_channel] << std::endl;
						//std::cout << "my_signal=" <<  MapSig[my_ADCmodule][my_channel] << std::endl;
						
						
						my_detector= MapDet[my_ADCmodule][my_channel];
						my_signal= MapSig[my_ADCmodule][my_channel] ;  
						
						//std::cout << "my_detector=" << my_detector  << std::endl;
						//std::cout << "my_signal=" <<   my_signal << std::endl;
						
						//my_strip= s_entry_sort.asic_id*128+s_entry_sort.ch_id;
						
						
						//out of sync if... !SYNC && PAUSE
						// QQQ: but check condition PAUSE&!SYNC (?)
						if(s_entry_sort.sync_flag && !s_entry_sort.pause_flag){
							my_sync_flag=true;
						}
						
						s_entry.energy= my_energy;
						s_entry.time_stamp= my_time_stamp;
						// s_entry.time_ext= my_time_ext;
						s_entry.BPulseNb= my_BPulseNb;
						s_entry.detector= my_detector;
						//s_entry.side= my_side;
						//s_entry.strip= my_strip;
						s_entry.signal= my_signal;
						
						s_entry.sync_flag= my_sync_flag;
						
						if(flag_debug && i<1000){
							log_file<<":::data"<<i<<"::: "
							<< s_entry.energy<<"  "
							<< s_entry.time_stamp<<"  "
							// << s_entry.time_ext<<"  "
							<< s_entry.BPulseNb<<"  "
							<< s_entry.detector<<"  "
							//      << s_entry.side<<"  "
							//      << s_entry.strip<<"  "
							<< s_entry.signal<<"  "
							<< s_entry.sync_flag<<std::endl;
						}
						
						//
						// Now collect data from this entry
						//
						//save this entry to TTree
						output_tree->Fill();
						
						//Fill histograms here
						// Hhistogram->Fill(s_entry.type);
					} //if(fill_flag)
					
				}// End of main loop over TTree to process raw MIDAS data entries
				
				output_file->Write();
				output_file->Print(); 
				
				// write histograms (etc...) to common root file
				//histo_file->cd();
				//histo_file->Write("Hhistogram");
				
				// last step: close all files (closing them can delete some objects from memory)
				//histo_file->Close();
				output_file->Close();         
				input_file->Close(); // Close TFile
				
				std::cout << "\n Calibrator ...done with file "<<iter_f<<" ." << std::endl;     
			}// End of: "if is_open".
			else{std::cout << "... cannot open file!"<< std::endl;}
			
		} //for( iter_f<n_input_list
		
		//    output_file->Write();
		//    output_file->Print(); 
		//    output_file->Close(); 
		//    log_file.close();
		
		input_list.close();
		
	} // End of: "if is_open" for input_list
	else{std::cout << "... cannot open file with list of input data for this step!"<< std::endl;}
	
	
	return;
}

void Calibrator (DutClass* dut){
	
	//DoCalibrate("one", "two", "three", "four"); 
	//std::cout << " Done with test"<<std::endl;
	
	/*******************/
	
	
	// Do convertion of ASCII files ?
	if ( !dut->GetFlagCalibrateFile() ) {return;}
	
	// Loop over the runs, make name for input and output files.
	// Pass the names to function "DoConvertion".
	
	// Write names of input files to a temporary text file
	std::string input_list_name;
	input_list_name= dut->GetDataInputList() ; 
	std::ofstream input_list (input_list_name.c_str(), std::ios::out|std::ios::trunc);  
	
	// First step of analysis pipe => get root files to loop through
	if( dut->GetFlagFirstInPipe() ){
		
		dut->SetFlagFirstInPipe(false);
		
		//if list of input files to be read from text file
		if(dut->GetFlagInputType()){
			
			//user provided file with list of input data files 
			/*************BUGBUGBUG******************
			 
			 std::string user_list_name;
			 user_list_name= dut->GetDataInputList() ; 
			 std::ifstream user_list (user_list_name.c_str(), std::ios::in);
			 
			 if(user_list.is_open()){
			 std::string in_line;
			 int n_files=0;
			 
			 while( getline( user_list, in_line) ){
			 //skip eempty lines (only '\n')
			 if(!in_line.empty() && '\n' != in_line[0]){
			 n_files++;
			 input_list << in_line << '\n';
			 }
			 }
			 
			 input_list.seekp(std::ios::beg);
			 input_list << n_files << '\n';
			 
			 ************************/
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
			
			input_list << dut->GetRunTotal() << '\n';
			
			for (int itr_run=dut->GetRunFirst(); itr_run< ( dut->GetRunTotal()+dut->GetRunFirst() ); itr_run++){
				std::stringstream name_input_file;
				name_input_file << dut->GetDataInputPath()
				<< dut->GetDataFilePrefix()
				<< itr_run
				<< "_midas.root"; //previous step=> Converter.cc
				
				input_list << name_input_file.str() << '\n';
			}      
		}
		
	}
	
	// Input Root file will be default TFile created in Converter step
	else{
		std::stringstream name_input_file;
		name_input_file << dut->GetDataOutputPath()
		<< dut->GetDataOutputPrefix()
		<< "_sort.root"; //previous step=> Sorter.cc
		
		input_list << 1 << '\n' <<name_input_file.str() << '\n';
		
	}
	
	//close temp file with list of input data files
	input_list.close();
	
	//std::string input_list_name;
	//input_list_name= dut->GetDataInputList() ; 
	
	std::stringstream name_output_file;
	name_output_file << dut->GetDataOutputPath()
	<< dut->GetDataOutputPrefix()
	<< "_calib.root";
	
	std::stringstream name_log_file;
	name_log_file << dut->GetDataOutputPath()
	<< dut->GetDataOutputPrefix()
	<< "_calib_log.txt";
	
	std::stringstream name_histo_file;
	name_histo_file << dut->GetDataOutputPath()
	<< dut->GetDataOutputPrefix()
	<< "_histo.root";
	
	DoCalibrate(input_list_name.c_str(), name_output_file.str(), name_log_file.str(), name_histo_file.str());
	
	/***********/
	
}
