#include "TimeSorter.hh"
#include "Common.hh"
#include "DutClass.hh"

#include <bitset>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <vector>

#include <ctime>

#include <TFile.h>
#include <TTree.h>
#include <TProfile.h>


bool p_sort_mod_enable[common::n_module]={false};
// this will not be necesary when PCB flange properly wired: mod_id= det_id(msb)+side_id(last bit)
int p_sort_det_id[common::n_module]={0}; // to map mod_id->det_id 
int p_sort_side_id[common::n_module]={0}; //to map mod_id->side_id





bool SortEntry(common::struct_entry_unpack & e_unpack, common::struct_entry_sort & e_sort, long long index, long long ts, long long ts_ext, int hit_id){
	
	int my_type= -1; //
	int my_hit= -1;
	int my_det_id= -1; //
	int my_side_id= -1; //
	int my_asic_id= -1;
	int my_ch_id= -1;
	int my_adc_data= -1;
	bool my_sync_flag=false;
	bool my_pause_flag=false;
	bool my_ext_flag=false;
	
	bool fill_flag= false;
	
	
	//if INFO CODE data
	if(e_unpack.type==2){
		
		// keep simple: for now only record SYNC-like pulses
		//    if(e_unpack.info_code==4 || e_unpack.info_code==5 || e_unpack.info_code==7){  // Before 2019
		if(e_unpack.info_code==4 || e_unpack.info_code==14 || e_unpack.info_code==7){      // Since 2019
			my_sync_flag= true;
			fill_flag= true; //to skip filling the R3B_sort tree with SYNC100 pulses
			//      my_pause_flag=false;
			if(e_unpack.info_code==14)my_ext_flag=true;
		}
		//skip everything else!
		else fill_flag= false;
	} //if(GREAT info data)
	
	else if(e_unpack.type==3){
		my_hit = e_unpack.hit;
		my_adc_data= e_unpack.adc_data;
		my_asic_id= e_unpack.asic_id;
		my_ch_id= e_unpack.ch_id;
		
		fill_flag= true;
	}
	
	my_det_id= p_sort_det_id[e_unpack.mod_id];
	my_side_id= p_sort_side_id[e_unpack.mod_id];
	my_type= e_unpack.type;
	
	//  e_sort.tm_stp=ts[my_det_id];
	//  e_sort.tm_stp_ext=ts_ext[my_det_id];
	e_sort.tm_stp=ts;
	e_sort.tm_stp_ext=ts_ext;
	e_sort.nevent=index;
	e_sort.type= my_type;
	e_sort.hit= my_hit;
	e_sort.hit_id= hit_id;
	e_sort.det_id= my_det_id;
	e_sort.side_id= my_side_id;
	e_sort.asic_id= my_asic_id;
	e_sort.ch_id= my_ch_id;
	e_sort.adc_data= my_adc_data;
	e_sort.sync_flag= my_sync_flag;
	e_sort.pause_flag= my_pause_flag;
	e_sort.ext_flag= my_ext_flag;
	
	return fill_flag;
}


void InsertionSort(std::vector<long long> & v_ts, std::vector<long long> & v_index){
	
	int v_n = v_ts.size();
	for(int i=1; i<v_n; i++){
		
		long long next_ts, next_index;
		next_ts= v_ts.at(i);
		next_index= v_index.at(i);
		
		int j;
		for(j= i; j>0 && v_ts.at(j-1) > next_ts; j--){
			v_ts[j]= v_ts.at(j-1);
			v_index[j]= v_index.at(j-1);
		}
		
		v_ts[j]= next_ts;
		v_index[j]= next_index;
		
	}
	
	return;
	
}

void LoadParametersSort(int n_module){ //maybe also file to read param from file
	
	
	/* 	bool my_enable[33]={ true,
	 true, true, true, true,
	 false, true, true, false,
	 false, false, true, false,
	 false, false, false, true,
	 false, false, false, false,
	 false, false, false, false,
	 false, false, false, false,
	 false, false, false, false};
	 */
	bool my_enable[60]={ false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false,
		true, true, false, false, true, true,
		true, true, false, false, true, true};
	
	
	// side 0: n-side, side 1= p-side
	// mixed alpha source in vacuum: mod_id=6: det_id=0, side_id=0
	//                               mod_id=8: det_id=0, side_id=1
	// Aug2014 data
	//int side_assignment[10]={99,99,99,99,99,
	//			 99,0,99,1,99};
	//int det_assignment[10]={99,99,99,99,99,
	//			99,0,99,0,99};
	
	// Feb2019 data
	// module number goe from 0 to 59
	// mod id 0-23: 2nd outer layer (don't exist)
	// mod id 24-47: 1st outer layer
	// mod id 48-59: inner layer
	
	int side_assignment[60]={99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  // 2nd louter layer (doesn't exist)
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  // 2nd louter layer (doesn't exist)
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  // 1st louter layer
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  // 1st outer Layer
		0,  1,  0,  1,  0,  1,  // inner Layer
		0,  1,  0,  1,  0,  1}; // inner Layer
	
	int det_assignment[60]={99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, // 2nd louter layer (doesn't exist)
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, // 2nd louter layer (doesn't exist)
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, // 1st outer Layer
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, // 1st outer Layer
		4,  4,  3,  3,  2, 2,    // inner Layer
		1,  1,  0,  0,  5, 5 };  // inner Layer
	
	// side 0: n-side, side 1= p-side
	// after wiring was fixed in flange?
	// mixed alpha source in vacuum: mod_id=6: det_id=0, side_id=0
	//                               mod_id=8: det_id=0, side_id=1
	// Sept 2014 data
	//int side_assignment[10]={99,99,99,99,99,
	//			 99,0,1,99,99};
	//int det_assignment[10]={99,99,99,99,99,
	//			99,0,0,99,99};
	
	//	for(int j=0;j<common::n_module;j++){
	for(int j=0;j<n_module;j++){
		//p_sort_mod_enable[j]= true; //my_enable[j];
		p_sort_mod_enable[j]= my_enable[j];
		p_sort_det_id[j]= det_assignment[j];
		p_sort_side_id[j]= side_assignment[j];
	}
	
	return;
}

void DoSort(std::string input_list_name, std::string output_file_name, std::string log_file_name, std::string histo_file_name){
	
	bool b_debug=true;
	
	bool DetTag_Ext[18]={false};
	bool SyncTag[18]={false};
	long long t0_sync =0;
	
	time_t t_start;
	t_start= time(0);
	
	std::cout << " Loading sort parameters...." << std::endl;
	LoadParametersSort(common::n_module); //maybe also file to read param from file
	
	//to read list of input files
	std::string input_file_name;
	int n_input_files;
	
	//  std::string input_list_name;
	//  input_list_name= dut->GetDataInputList() ;
	std::cout << "Will open file with input list: "<<input_list_name << std::endl;
	std::ifstream input_list (input_list_name.c_str(), std::ios::in);
	
	if (input_list.is_open()){
		input_list >> n_input_files;
		
		
		// std::cout << "Opened!!!!!!!!11: "<<input_list_name << std::endl;
		//  std::cout << " n_input_files: "<<n_input_files << std::endl;
		//  std::string in_line;
		//  while( getline( input_list, in_line) ){
		//    //skip eempty lines (only '\n')
		//    if(!in_line.empty() && '\n' != in_line[0]){
		//	//n_files++;
		//	std::cout << in_line << std::endl;
		//    }
		//  }
		
		//  return;
		
		
		//Create log file.
		std::ofstream log_file;
		log_file.open(log_file_name.c_str(), std::ios::out);
		
		
		
		
		//Open Root file to save a few diagnostic histograms
		//     TFile* histo_file = new TFile(histo_file_name.c_str(),"update");
		TFile* histo_file = new TFile(histo_file_name.c_str(),"recreate");
		
		//---- declare histograms here!  ----//
		//TH1I * Hhistogram= new TH1I("Hhistogram","my histogram;x [ch]",10,-0.5,9.5);
		//    TH1I * HEcualquiera[2];
		//    HEcualquiera[0]= new TH1I("HEcualquiera0","Test ADC (mod=7, ch=24, adc_range=0);adc value",1024,0,65536);
		//    HEcualquiera[1]= new TH1I("HEcualquiera1","Test ADC (mod=7, ch=24, adc_range=1);adc value",1024,0,65536);
		
		//
		
		TProfile *hprof1 = new TProfile("hprof1","Profile of ts versus hit_id in Det 1 ",3001 , 0., 3000., 2.60e14, 3.61e14);
		TProfile *hprof2 = new TProfile("hprof2","Profile of ts versus hit_id in Det 2 ",3001 , 0., 3000., 2.60e14, 3.61e14);
		TProfile *hprof4 = new TProfile("hprof4","Profile of ts versus hit_id in Det 4 ",3001 , 0., 3000., 2.60e14, 3.61e14);
		TProfile *hprof5 = new TProfile("hprof5","Profile of ts versus hit_id in Det 5 ",3001 , 0., 3000., 2.60e14, 3.61e14);
		
		TProfile *hprof1Ext = new TProfile("hprof1Ext","Profile of ts_ext versus hit_id_ext in Det 1 ",10001 , 0., 10000., 2.60e14, 3.61e14);
		TProfile *hprof2Ext = new TProfile("hprof2Ext","Profile of ts_ext versus hit_id_ext in Det 2 ",10001 , 0., 10000., 2.60e14, 3.61e14);
		TProfile *hprof4Ext = new TProfile("hprof4Ext","Profile of ts_ext versus hit_id_ext in Det 4 ",10001 , 0., 10000., 2.60e14, 3.61e14);
		TProfile *hprof5Ext = new TProfile("hprof5Ext","Profile of ts_ext versus hit_id_ext in Det 5 ",10001 , 0., 10000., 2.60e14, 3.61e14);
		TProfile *hprof1ExtSync = new TProfile("hprof1ExtSync","Profile of ts_ext versus hit_id_ext in Det 1 ",10001 , 0., 10000., 2.60e14, 3.61e14);
		TProfile *hprof2ExtSync = new TProfile("hprof2ExtSync","Profile of ts_ext versus hit_id_ext in Det 2 ",10001 , 0., 10000., 2.60e14, 3.61e14);
		TProfile *hprof4ExtSync = new TProfile("hprof4ExtSync","Profile of ts_ext versus hit_id_ext in Det 4 ",10001 , 0., 10000., 2.60e14, 3.61e14);
		TProfile *hprof5ExtSync = new TProfile("hprof5ExtSync","Profile of ts_ext versus hit_id_ext in Det 5 ",10001 , 0., 10000., 2.60e14, 3.61e14);
		
		
		// Create Root ntuple.
		TFile* output_file = new TFile(output_file_name.c_str(),"recreate");
		TTree* output_tree = new TTree("R3B_sort","R3B_sort");
		common::struct_entry_sort s_entry;
		output_tree->Branch("entry_sort", &s_entry.tm_stp,"tm_stp/l:tm_stp_ext/l:nevent/L:type/I:hit/I:hit_id/I:det_id/I:side_id/I:asic_id/I:ch_id/I:adc_data/I:sync_flag/O:pause_flag/O:ext_flag/O");
		output_file->cd();
		
		
		
		
		//    TTree* temp_tree = new TTree("R3B_temp","R3B_temp");
		//    common::struct_entry_sort s_temp;
		//    temp_tree->Branch("entry_temp", &s_temp.tm_stp,"tm_stp/L:nevent/L:type/I:hit/I:det_id/I:side_id/I:asic_id/I:ch_id/I:adc_data/I:sync_flag/O:pause_flag/O");
		//long long index_first=0;
		
		if( n_input_files < 1) std::cout << "... invalid number of data input files for Sort step! N= "<< n_input_files <<std::endl;
		
		//loop through each file to be processed
		for(int iter_f=0;iter_f<n_input_files;iter_f++){
			
			// Read name and open next Root input file.
			input_list >> input_file_name;
			std::cout << "Sorting file: " << input_file_name << std::endl;
			TFile* input_file = new TFile(input_file_name.c_str(),"read");
			
			if (input_file != 0){
				
				TTree *input_tree= (TTree*) input_file->Get("R3B_unpack");
				
				common::struct_entry_unpack s_entry_unpack;
				input_tree->SetBranchAddress("entry_unpack",&s_entry_unpack,0);
				
				long long n_entries;
				n_entries = input_tree->GetEntries();
				
				std::cout << "\n\n Sorting: number of entries in input tree= " << n_entries << ", for file " << input_file_name << std::endl;
				log_file << "\n\n Sorting: number of entries in input tree= " << n_entries << ", for file " << input_file_name << std::endl;
				
				//for structure struct_entry_sort
				long long my_tm_stp[18];
				long long my_tm_stp_ext[18];
				///	long long my_nevent;
				///	int my_type;
				///	int my_hit;
				int my_hit_id_Ext[18]={0};
				int my_hit_id_ExtSync[18]={0};
				int my_hit_id[18]={0};
				int my_det_id;
				int my_side_id;
				
				///	int my_asic_id;
				///	int my_ch_id;
				///	//int my_type;
				///	int my_adc_data;
				///	bool my_sync_flag=false;
				///	bool my_pause_flag=false;
				
				//arrays to check synchronizatino is going fine (or not!)
				unsigned long tm_stp_msb_modules[common::n_detector]={0};  // medium significant bits
				unsigned long tm_stp_hsb_modules[common::n_detector]={0}; // highest significant bit
				
				//some counter (per detector
				int my_pause[18]={0};   // info code 2
				int my_resume[18]={0};  // info code 3
				int my_code14[18]={0};  // info code 14
				int my_code4[18]={0};   // info code 4
				int my_code7[18]={0};   // info code 7
				
				// --- SYNC100, PAUSE status flags initialized before run. QQQ: what should their initial states be?
				//
				// are they used for anything now... I think no effect with current logic of code.
				// ignore for oct test
				//bool sync_flag_modules[common::n_detector]={false}; //true= last SYNC100 received correctly
				//bool pause_flag_modules[common::n_detector]={false}; //false= synchronization is running (i.e. not paused)
				
				//long long my_tm_stp_prev[common::n_detector]={0}; //check only for adc...? or all
				
				unsigned long my_tm_stp_msb; // most significant bits of time-stamp (in info code)
				unsigned long my_tm_stp_hsb; // most significant bits of time-stamp (in info code)
				
				//bool fill_flag;
				bool no_sync_flag;
				bool Ext_flag = false;
				
				//
				//all things for ordering data by time stamp
				//
				bool ts_sort_flag;
				ts_sort_flag = false;
				
				
				
				std::vector<long long> ts_temp;
				std::vector<long long> index_temp;
				
				ts_temp.reserve(2048);
				index_temp.reserve(2048);
				
				const int MAXsort= 100000; //do we need maximum number when we do not want to sort any more!?
				
				
				
				//
				// Main loop over TTree to process raw MIDAS data entries
				//
				//if(n_entries>9999) n_entries=9999;
				for(long long i=0; i<n_entries; i++){
					
					input_tree->GetEntry(i);
					// --- check values will not make code crash!! e.g. s_entry_unpack.mod_id within boundaries!
					// --- although this probably should be done in Converter step (?)
					
					
					my_det_id= p_sort_det_id[s_entry_unpack.mod_id];
					my_side_id= p_sort_side_id[s_entry_unpack.mod_id];
					
					no_sync_flag=true;
					
					
					//if INFO code
					if(s_entry_unpack.type==3)my_hit_id[my_det_id]++;
					
					if(s_entry_unpack.type==2){
						
						if(s_entry_unpack.info_code==2)my_pause[my_det_id]++;
						if(s_entry_unpack.info_code==3)my_resume[my_det_id]++;
						
						
						//If SYNC pulse
						//if(s_entry_unpack.info_code==4 || s_entry_unpack.info_code==5 || s_entry_unpack.info_code==7){ // Before 2019
						if(s_entry_unpack.info_code==4 || s_entry_unpack.info_code==14 || s_entry_unpack.info_code==7){  // Since 2019
							
							if(s_entry_unpack.info_code==4)my_code4[my_det_id]++;
							if(s_entry_unpack.info_code==14)my_code14[my_det_id]++;
							if(s_entry_unpack.info_code==7)my_code7[my_det_id]++;
							
							no_sync_flag= false;
							
							my_tm_stp_msb= (s_entry_unpack.info_field & 0x000FFFFF); // MS bits (47:28) of timestamp
							tm_stp_msb_modules[my_det_id]= my_tm_stp_msb; //update for use with other data types
							
							if( s_entry_unpack.info_code==14 )my_hit_id_Ext[my_det_id]++;
							if( s_entry_unpack.info_code==14 && SyncTag[my_det_id] )my_hit_id_ExtSync[my_det_id]++;
							
							//sync_flag_modules[my_det_id]=true;
							//my_sync_flag= true;
							
							//pause_flag_modules[my_det_id]=false;
							//my_pause_flag=false;
							
							//reconstruct time stamp= MSB+LSB
							my_tm_stp[my_det_id]= (  (tm_stp_msb_modules[my_det_id] << 28 ) | (s_entry_unpack.tm_stp_lsb & 0x0FFFFFFF) );
							
							if(s_entry_unpack.info_code==14){
								my_tm_stp_ext[my_det_id]=my_tm_stp[my_det_id];
								Ext_flag= true;
							}
						}
						
						if(s_entry_unpack.info_code==5){  // Since 2019  high significant bits
							
							my_tm_stp_hsb= (s_entry_unpack.info_field & 0x000FFFFF); // MS bits (47:28) of timestamp
							tm_stp_hsb_modules[my_det_id]= my_tm_stp_hsb;
							
							if(!Ext_flag){ // ie: code 4 prior this code 5
								my_tm_stp[my_det_id]= (  (tm_stp_hsb_modules[my_det_id] << 48 ) | (tm_stp_msb_modules[my_det_id] << 28 ) | (s_entry_unpack.tm_stp_lsb & 0x0FFFFFFF) );
							}else          // ie: code 14 prior this code 5
							{
								my_tm_stp_ext[my_det_id]= (tm_stp_hsb_modules[my_det_id] << 48 ) | (my_tm_stp_ext[my_det_id] & 0x0FFFFFFFFFFFF);
								
								//std::cout << "ext time stamp found: " << t0_sync << " in det #" << my_det_id << std::endl;
								
								
								//
								// sanity check of det synchronisation using external signal
								//
								// Let's determine the lowest time stamp when EVERY detector DAQs start receiving external time stamp.
								if(!DetTag_Ext[my_det_id]){ // if first time we see this detector in the datastream
									if(my_tm_stp_ext[my_det_id]>=t0_sync){
										t0_sync= my_tm_stp_ext[my_det_id];
										DetTag_Ext[my_det_id]=true;
										std::cout << "new t0_sync found: " << t0_sync << " in det #" << my_det_id << std::endl;
									}
								}
								
								
								Ext_flag= false;
							}
							
							//std::cout << "ts data= " << my_tm_stp <<  std::endl;
							
							//vectors to order data by time-stamp
							ts_temp.push_back(my_tm_stp[my_det_id]);
							index_temp.push_back(i);
							
							ts_sort_flag= true;
						}
					}
					/////end 10/9/14 afternnn//////
					
					
					
					//if any other data type
					if(no_sync_flag){
						//reconstruct time stamp= HSB+MSB+LSB
						my_tm_stp[my_det_id]= (  (tm_stp_hsb_modules[my_det_id] << 48 ) | (tm_stp_msb_modules[my_det_id] << 28 ) | (s_entry_unpack.tm_stp_lsb & 0x0FFFFFFF) );
						
						//std::cout << "ts data= " <<  my_tm_stp <<  std::endl;
						//vectors to order data by time-stamp
						ts_temp.push_back(my_tm_stp[my_det_id]);
						index_temp.push_back(i);
						
						//if temp arrays too large, sort
						if(ts_temp.size()>MAXsort){
							ts_sort_flag= true;
						}
						//if last, sort
						else if((i+1)==n_entries){
							ts_sort_flag= true;
						}
					}
					
					
					
					if(SortEntry(s_entry_unpack, s_entry, 0 , my_tm_stp[my_det_id], my_tm_stp_ext[my_det_id], my_hit_id[my_det_id])){
						output_tree->Fill();
						if(my_det_id==1)hprof1->Fill(my_hit_id[my_det_id],my_tm_stp[my_det_id],1);
						if(my_det_id==2)hprof2->Fill(my_hit_id[my_det_id],my_tm_stp[my_det_id],1);
						if(my_det_id==4)hprof4->Fill(my_hit_id[my_det_id],my_tm_stp[my_det_id],1);
						if(my_det_id==5)hprof5->Fill(my_hit_id[my_det_id],my_tm_stp[my_det_id],1);
						if(my_det_id==1)hprof1Ext->Fill(my_hit_id_Ext[my_det_id],my_tm_stp_ext[my_det_id],1);
						if(my_det_id==2)hprof2Ext->Fill(my_hit_id_Ext[my_det_id],my_tm_stp_ext[my_det_id],1);
						if(my_det_id==4)hprof4Ext->Fill(my_hit_id_Ext[my_det_id],my_tm_stp_ext[my_det_id],1);
						if(my_det_id==5)hprof5Ext->Fill(my_hit_id_Ext[my_det_id],my_tm_stp_ext[my_det_id],1);
						
					}
					
					
					
					/* Put in comment to skip time ordering:
					 
					 if(ts_sort_flag){
					 
					 long long n_first= index_temp.at(0);
					 long long n_loop= index_temp.size();
					 
					 InsertionSort(ts_temp,index_temp);
					 ts_sort_flag= false;
					 
					 //transport sorted data to output_tree
					 for(long long j=0; j<n_loop; j++){	    //transport sorted data to output_tree
					 
					 //set address of new struct_entry_sort to temp_tree Branch? or is s_temp enough?
					 input_tree->GetEntry(index_temp.at(j));
					 
					 if(SortEntry(s_entry_unpack, s_entry, j+n_first, ts_temp.at(j)) ){
					 output_tree->Fill();
					 }
					 else{
					 //log_file << "*** ENTRY NOT FILLED:\n"
					 //	 <<     "    type= "<<s_entry_unpack.type << std::endl
					 //	 <<     "    mod_id= "<<s_entry_unpack.mod_id << std::endl;
					 //std::cout << " *!->NF<-!* ";
					 }
					 }
					 
					 //index_temp.clear();
					 //ts_temp.clear();
					 
					 } //if ts_sort_flag
					 */
					
					
				}// End of main loop over TTree to process raw MIDAS data entries
				
				int d=0;
				for (int k=0; k<(common::n_detector); k++){
					if(p_sort_mod_enable[2*k]){
						d=p_sort_det_id[2*k];
						std::cout << "\n Number of hit in Det #" <<  d << ": "<< my_hit_id[d] <<"" << std::endl;
						std::cout << "\n   Nb of PAUSE/RESUME: "<< my_pause[d] <<"/" <<  my_resume[d] << std::endl;
						std::cout << "\n   Nb of code4: "<< my_code4[d] << std::endl;
						std::cout << "\n   Nb of code14: "<< my_code14[d] << std::endl;
						std::cout << "\n   Nb of code7: "<< my_code7[d] << std::endl;
						
					}
				}
				//std::cout << "\n Number of hit in Det 2: "<< my_hit_id[2] <<"" << std::endl;
				
				//std::cout << "\n Number of hit in Det 4: "<< my_hit_id[4] <<"" << std::endl;
				
				//std::cout << "\n Number of hit in Det 5: "<< my_hit_id[5] <<"" << std::endl;
				
				
				output_file->Write();
				output_file->Print();
				
				// last step: close all files (closing them can delete some objects from memory)
				output_file->Close();
				
				
				histo_file->cd();
				
				hprof1->Write();
				hprof2->Write();
				hprof4->Write();
				hprof5->Write();
				hprof1Ext->Write();
				hprof2Ext->Write();
				hprof4Ext->Write();
				hprof5Ext->Write();
				
				histo_file->Close();
				
				
				input_file->Close(); // Close TFile
				
				std::cout << "\n Converter ...done with file "<<iter_f<<" ." << std::endl;
			}// End of: "if is_open".
			else{std::cout << "... cannot open file!"<< std::endl;}
			
		} //for( iter_f<n_input_list
		
		
		
		std::cout << "\n\n End TimeSort: time elapsed = " <<time(0)-t_start <<" sec.\n"<<std::endl;
		log_file << "\n\n End TimeSort: time elapsed = " <<time(0)-t_start <<" sec.\n"<<std::endl;
		
		log_file.close(); //?? to close or not to close?
		
		input_list.close();
		
		
		
	} // End of: "if is_open" for input_list
	else{std::cout << "... cannot open file with list of input data for this step!"<< std::endl;}
	
}

void TimeSorter (DutClass* dut){
	
	// Do convertion of ASCII files ?
	if ( !dut->GetFlagSortFile() ) {return;}
	
	// Loop over the runs, make name for input and output files.
	// Pass the names to function "DoConvertion".
	
	// Write names of input files to a temporary text file
	std::string input_list_name;
	input_list_name= dut->GetDataInputList() ;
	std::ofstream input_list (input_list_name.c_str(), std::ios::out|std::ios::trunc);
	
	std::cout<<"\n\n\n---->  PREPARING SORT: opened data input list: "<<input_list_name << std::endl;
	
	// First step of analysis pipe => get root files to loop through
	if( dut->GetFlagFirstInPipe() ){
		
		dut->SetFlagFirstInPipe(false);
		
		//if list of input files to be read from text file
		if(dut->GetFlagInputType()){
			/*********************
			 //user provided file with list of input data files
			 std::string user_list_name;
			 user_list_name= dut->GetDataListUser() ;
			 std::ifstream user_list (user_list_name.c_str(), std::ios::in);
			 std::cout<<"PREPARING SORT opened user input list: "<<user_list_name << std::endl;
			 
			 if(user_list.is_open()){
			 std::string in_line;
			 int n_files=0;
			 
			 while( getline( user_list, in_line) ){
			 
			 std::cout<<"       line "<<n_files<<": "<<in_line << std::endl;
			 
			 input_list << "         "; //save space to put number of input lists
			 //skip eempty lines (only '\n')
			 if(!in_line.empty() && '\n' != in_line[0]){
			 n_files++;
			 input_list << in_line << '\n';
			 }
			 }
			 
			 input_list.seekp(std::ios::beg);
			 input_list << n_files << '\n';
			 ***********/
			
			//COPY FROM CONVERT
			
			
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
				
				//COPY FROM CONVERT
				
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
				<< "_unpack.root"; //previous step=> Converter.cc
				
				input_list << name_input_file.str() << '\n';
			}
		}
		
	}
	
	// Input Root file will be default TFile created in Converter step
	else{
		std::stringstream name_input_file;
		name_input_file << dut->GetDataOutputPath()
		<< dut->GetDataOutputPrefix()
		<< "_unpack.root"; //previous step=> Converter.cc
		
		input_list << 1 << '\n' <<name_input_file.str() << '\n';
		
	}
	
	//close temp file with list of input data files
	input_list.close();
	
	//std::string input_list_name;
	//input_list_name= dut->GetDataInputList() ;
	
	std::stringstream name_output_file;
	name_output_file << dut->GetDataOutputPath()
	<< dut->GetDataOutputPrefix()
	<< "_sort.root";
	
	std::stringstream name_log_file;
	name_log_file << dut->GetDataOutputPath()
	<< dut->GetDataOutputPrefix()
	<< "_sort_log.txt";
	
	std::stringstream name_histo_file;
	name_histo_file << dut->GetDataOutputPath()
	<< dut->GetDataOutputPrefix()
	<< "_histo.root";
	
	DoSort(input_list_name.c_str(), name_output_file.str(), name_log_file.str(), name_histo_file.str());
	
}
