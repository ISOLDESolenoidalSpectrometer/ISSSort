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
#include <TProfile.h>
#include <TH1I.h>

double p_cali_gain; //[common::n_module][common::n_side][common::n_asic][common::n_channel]={{{{0}}}};
double p_cali_offset; //[common::n_module][common::n_side][common::n_asic][common::n_channel]={{{{0}}}};
//int p_cali_geom_detector[common::n_module][common::n_side]={{0}}; // [det][side] -> location in array
//int p_cali_geom_strip[common::n_module]={0}; // [strip] -> ??? position mm ???

// which FEE64 modules are running...
//bool p_cali_mod_enable[common::n_module]={false};
//bool p_cali_strip_enable[common::n_module][common::n_side][common::n_asic][common::n_channel]={{{{false}}}}; //do we want to mask out some strips

double p_cali_time;

void LoadParametersCali(int n_module, std::string file_name){
	
	std::cout << "Loading parameters for stack of "<< n_module/2 <<" DSSDs. Currently only default parameters implemented"
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
	
	p_cali_time=1; //convert clocks-> usec/ns/....
	
	return;
}


void DoCalibrate(std::string input_list_name, std::string output_file_name, std::string log_file_name, std::string histo_file_name){
	
	
	bool flag_debug= true;
	
	long long t0_Sync=0;  // from timesort
	
	long long time_window = 20000;  // ns
	long long time_diff;  // ns
	long long timediff;  // ms
	
	int coincDet1 =0 ; //
	int coincDet2 =0 ; //
	int coincDet4 =0 ; //
	int coincDet5 =0 ; //
	
	if(t0_Sync==0){
		std::cout << "Please enter the t0_Sync values found during the previous process (timesort) " << std::endl;
		std::cin >> t0_Sync;
	}
	
	//this solution didn't work!
	//parameter::struct_parameter_cali p_cali;
	//int este=5;
	//LoadParameterCali(este);
	//std::cout << "\n\nDID it WORK??? "<<std::endl;
	
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
		//     TFile* histo_file = new TFile(histo_file_name.c_str(),"update");
		TFile* histo_file = new TFile(histo_file_name.c_str(),"update");
		
		//---- declare histograms here!  ----//
		//TH1I * Hhistogram= new TH1I("Hhistogram","my histogram;x [ch]",10,-0.5,9.5);
		//    TH1I * HEcualquiera[2];
		//    HEcualquiera[0]= new TH1I("HEcualquiera0","Test ADC (mod=7, ch=24, adc_range=0);adc value",1024,0,65536);
		//    HEcualquiera[1]= new TH1I("HEcualquiera1","Test ADC (mod=7, ch=24, adc_range=1);adc value",1024,0,65536);
		
		TH1I * HistTimeSi1_Pl= new TH1I("HistTimeSi1_Pl","time difference between Det1 and muon detector",30000,-5000,25000);
		TH1I * HistTime2Si1_Pl= new TH1I("HistTime2Si1_Pl","time difference between Det1 and muon detector",3000,-1,1);
		TH1I * HistTimeSi2_Pl= new TH1I("HistTimeSi2_Pl","time difference between Det2 and muon detector",30000,-5000,25000);
		TH1I * HistTimeSi4_Pl= new TH1I("HistTimeSi4_Pl","time difference between Det4 and muon detector",30000,-5000,25000);
		TH1I * HistTimeSi5_Pl= new TH1I("HistTimeSi5_Pl","time difference between Det5 and muon detector",30000,-5000,25000);
		//
		
		TProfile *hprof1ExtSync = new TProfile("hprof1ExtSync","Profile of ts_ext versus hit_id_ext in Det 1 ",10001 , 0., 10000., 2.60e14, 3.61e14);
		TProfile *hprof2ExtSync = new TProfile("hprof2ExtSync","Profile of ts_ext versus hit_id_ext in Det 2 ",10001 , 0., 10000., 2.60e14, 3.61e14);
		TProfile *hprof4ExtSync = new TProfile("hprof4ExtSync","Profile of ts_ext versus hit_id_ext in Det 4 ",10001 , 0., 10000., 2.60e14, 3.61e14);
		TProfile *hprof5ExtSync = new TProfile("hprof5ExtSync","Profile of ts_ext versus hit_id_ext in Det 5 ",10001 , 0., 10000., 2.60e14, 3.61e14);
		
		TProfile *hprofExtDiff1_4 = new TProfile("hprofExtDiff1_4","time diff in Det 1 & 4 ",9001 , 0., 9000., -2., 2.);
		TProfile *hprofExtDiff1_5 = new TProfile("hprofExtDiff1_5","time diff in Det 1 & 5 ",9001 , 0., 9000., -2., 2.);
		TProfile *hprofExtDiff2_4 = new TProfile("hprofExtDiff2_4","time diff in Det 2 & 4 ",9001 , 0., 9000., -2., 2.);
		TProfile *hprofExtDiff2_5 = new TProfile("hprofExtDiff2_5","time diff in Det 2 & 5 ",9001 , 0., 9000., -2., 2.);
		
		TProfile *hprof1ExtDiff = new TProfile("hprof1ExtDiff","Profile of ts_ext (x 5ms) versus timediff ( x 5ms) in Det 1 ",10001 , -5000, 25000., 2.60e8, 2.65e8);
		
		
		
		// Create Root ntuple.
		TFile* output_file = new TFile(output_file_name.c_str(),"recreate");
		TTree* output_tree = new TTree("R3B_calib","R3B_calib");
		common::struct_entry_calibrate s_entry;
		output_tree->Branch("entry_calib", &s_entry.energy,"energy/D:time_stamp/D:detector/I:side/I:strip/I:sync_flag/O");
		
		
		input_list >> n_input_files;
		
		if( n_input_files < 1) std::cout << "... invalid number of data input files for Calibrate step! N= "<< n_input_files <<std::endl;
		
		//loop through each file to be processed
		for(int iter_f=0;iter_f<n_input_files;iter_f++){
			
			// Read name and open next Root input file.
			input_list >> input_file_name;
			std::cout << "Calibrating file: " << input_file_name << std::endl;
			TFile* input_file = new TFile(input_file_name.c_str(),"read");
			
			if (input_file != 0){
				
				TTree *input_tree= (TTree*) input_file->Get("R3B_sort");
				
				common::struct_entry_sort s_entry_sort;
				input_tree->SetBranchAddress("entry_sort",&s_entry_sort,0);
				
				long long n_entries;
				n_entries = input_tree->GetEntries();
				
				std::cout << "\n\n Calibrating: number of entries in input tree= " << n_entries << ", for file " << input_file_name << std::endl;
				
				//for structure struct_entry_calibrate
				double my_energy;
				double my_time_stamp;
				int my_hit;
				long long my_time_ext;
				int my_type;
				int my_detector;
				int my_side;
				int my_strip;
				bool my_sync_flag;
				
				int my_asic, my_ch;
				
				int my_hit_id_ExtSync[18]={0};
				int my_hit_id_Sync[18]={0};
				
				//arrays to check synchronizatino is going fine (or not!)
				
				//unsigned long tm_stp_msb_modules[common::n_module]={0};
				
				bool fill_flag;
				
				//	double my_time_offset;
				
				
				std::vector<long long> ts_tempDet1;
				std::vector<long long> index_tempDet1;
				ts_tempDet1.reserve(2048);
				index_tempDet1.reserve(2048);
				
				std::vector<long long> side_tempDet1;
				side_tempDet1.reserve(2048);
				std::vector<long long> asic_tempDet1;
				asic_tempDet1.reserve(2048);
				std::vector<long long> ch_tempDet1;
				ch_tempDet1.reserve(2048);
				
				
				std::vector<long long> ts_tempDet5;
				std::vector<long long> side_tempDet5;
				std::vector<long long> asic_tempDet5;
				std::vector<long long> ch_tempDet5;
				std::vector<long long> index_tempDet5;
				ts_tempDet5.reserve(2048);
				side_tempDet5.reserve(2048);
				asic_tempDet5.reserve(2048);
				ch_tempDet5.reserve(2048);
				index_tempDet5.reserve(2048);
				
				std::vector<long long> ts_tempExtDet1;
				std::vector<long long> index_tempExtDet1;
				ts_tempExtDet1.reserve(2048);
				index_tempExtDet1.reserve(2048);
				std::vector<long long> ts_tempExtDet4;
				std::vector<long long> index_tempExtDet4;
				ts_tempExtDet4.reserve(2048);
				index_tempExtDet4.reserve(2048);
				std::vector<long long> ts_tempExtDet2;
				std::vector<long long> index_tempExtDet2;
				ts_tempExtDet2.reserve(2048);
				index_tempExtDet2.reserve(2048);
				std::vector<long long> ts_tempExtDet5;
				std::vector<long long> index_tempExtDet5;
				ts_tempExtDet5.reserve(2048);
				index_tempExtDet5.reserve(2048);
				
				long long n_indexDet1, n_indexDet4,  n_indexDet2, n_indexDet5;
				n_indexDet1=n_indexDet4=n_indexDet2=n_indexDet5=0;
				bool Diff_tag1_4=false;
				bool Diff_tag1_5=false;
				bool Diff_tag2_4=false;
				bool Diff_tag2_5=false;
				
				//
				// Main loop over TTree to process raw MIDAS data entries
				//
				
				
				for(long long i=0; i<n_entries; i++){
					
					//default values for struct_entry_sort
					my_energy= -1;
					my_time_stamp= -1;
					my_time_ext= -1;
					my_type= -1;
					my_detector= -1;
					my_side= -1;
					my_strip= -1;
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
					my_type=s_entry_sort.type;
					
					
					//
					// Synchronisation check
					//
					if(my_type==2){ // Checking synchronisation using external time stamp
						my_detector=s_entry_sort.det_id;	
						my_time_ext=s_entry_sort.tm_stp_ext;
						
						if( my_time_ext >= t0_Sync && s_entry_sort.ext_flag)
						{
							
							if(my_detector==1){
								ts_tempExtDet1.push_back(my_time_ext);
								index_tempExtDet1.push_back(my_hit_id_ExtSync[my_detector]);
							}
							if(my_detector==2){
								ts_tempExtDet2.push_back(my_time_ext);
								index_tempExtDet2.push_back(my_hit_id_ExtSync[my_detector]);
							}
							if(my_detector==4){
								ts_tempExtDet4.push_back(my_time_ext);
								index_tempExtDet4.push_back(my_hit_id_ExtSync[my_detector]);
							}
							if(my_detector==5){
								ts_tempExtDet5.push_back(my_time_ext);
								index_tempExtDet5.push_back(my_hit_id_ExtSync[my_detector]);
							}
							
							if(my_detector==1)hprof1ExtSync->Fill(my_hit_id_ExtSync[my_detector],my_time_ext,1);
							if(my_detector==2)hprof2ExtSync->Fill(my_hit_id_ExtSync[my_detector],my_time_ext,1);
							if(my_detector==4)hprof4ExtSync->Fill(my_hit_id_ExtSync[my_detector],my_time_ext,1);
							if(my_detector==5)hprof5ExtSync->Fill(my_hit_id_ExtSync[my_detector],my_time_ext,1);
							
							my_hit_id_ExtSync[my_detector]++;
							
						}   
						
						
						n_indexDet1= index_tempExtDet1.size(); 
						n_indexDet4= index_tempExtDet4.size(); 
						n_indexDet2= index_tempExtDet2.size(); 
						n_indexDet5= index_tempExtDet5.size(); 
						
						if(n_indexDet1 >= 9000 && n_indexDet4 >= 9000 && !Diff_tag1_4){
							for(int ind=0; ind<9000; ind++)
							{	
								hprofExtDiff1_4->Fill(ind, (ts_tempExtDet1[ind] - ts_tempExtDet4[ind]) ,1);
							}
							Diff_tag1_4=true;			
						} 
						if(n_indexDet1 >= 9000 && n_indexDet5 >= 9000 && !Diff_tag1_5){
							for(int ind=0; ind<9000; ind++)
							{	
								hprofExtDiff1_5->Fill(ind, (ts_tempExtDet1[ind] - ts_tempExtDet5[ind]) ,1);
							}
							Diff_tag1_5=true;			
						} 
						if(n_indexDet2 >= 9000 && n_indexDet4 >= 9000 && !Diff_tag2_4){
							for(int ind=0; ind<9000; ind++)
							{	
								hprofExtDiff2_4->Fill(ind, (ts_tempExtDet2[ind] - ts_tempExtDet4[ind]) ,1);
							}
							Diff_tag2_4=true;			
						} 
						if(n_indexDet2 >= 9000 && n_indexDet5 >= 9000 && !Diff_tag2_5){
							for(int ind=0; ind<9000; ind++)
							{	
								hprofExtDiff2_5->Fill(ind, (ts_tempExtDet2[ind] - ts_tempExtDet5[ind]) ,1);
							}
							Diff_tag2_5=true;			
						} 
						
						//	fill_flag=true;
					}
					
					
					if(my_type==3){ // Checking synchronisation using external time stamp
						my_detector=s_entry_sort.det_id;	
						my_time_stamp=s_entry_sort.tm_stp;
						my_hit=s_entry_sort.hit;
						my_side=s_entry_sort.side_id;
						my_ch=s_entry_sort.ch_id;
						my_asic=s_entry_sort.asic_id;
						
						if( my_hit && my_detector==1 && my_time_stamp>=t0_Sync && (ts_tempExtDet1[my_hit_id_ExtSync[my_detector]-1])) // -1 because we want previous ts_tempExt
						{
							
							ts_tempDet1.push_back(my_time_stamp);
							side_tempDet1.push_back(my_side);
							asic_tempDet1.push_back(my_asic);
							ch_tempDet1.push_back(my_ch);
							
							index_tempDet1.push_back(my_hit_id_Sync[my_detector]);
							my_hit_id_Sync[my_detector]++;
							
							time_diff = ts_tempExtDet1[my_hit_id_ExtSync[my_detector]-1] -  my_time_stamp;
							
							//if( sqrt(time_diff*time_diff) < time_window){
							//std::cout << "Time_ext=" << ts_tempExtDet1[my_hit_id_ExtSync[my_detector]-1] << "ns" << std::endl;
							//std::cout << "Time ext index=" << my_hit_id_ExtSync[my_detector]-1 << std::endl;
							//std::cout << "Time_adcdata=" <<  my_time_stamp << "ns" << std::endl;
							//std::cout << "Time_diff=" <<  time_diff << "ns" << std::endl;
							coincDet1++;
							HistTimeSi1_Pl->Fill(time_diff/1.e6);
							hprof1ExtDiff->Fill( time_diff/1.e6, ts_tempExtDet1[my_hit_id_ExtSync[my_detector]-1]/1.e6, 1);
							//}
							
						}
						if( my_hit && my_detector==2 && my_time_stamp>=t0_Sync && (ts_tempExtDet2[my_hit_id_ExtSync[my_detector]-1]))
						{
							
							time_diff = ts_tempExtDet2[my_hit_id_ExtSync[my_detector]-1] -  my_time_stamp;
							//if( sqrt(time_diff*time_diff) < time_window){
							coincDet2++;
							HistTimeSi2_Pl->Fill(time_diff/1.e6);
							
							//}
						}
						
						if( my_hit && my_detector==4 && my_time_stamp>=t0_Sync && (ts_tempExtDet4[my_hit_id_ExtSync[my_detector]-1]))
						{
							
							time_diff = ts_tempExtDet4[my_hit_id_ExtSync[my_detector]-1] -  my_time_stamp;
							//if( sqrt(time_diff*time_diff) < time_window){
							coincDet4++;
							HistTimeSi4_Pl->Fill(time_diff/1.e6);
							
							//}
						}
						if( my_hit && my_detector==5 && my_time_stamp>=t0_Sync && (ts_tempExtDet5[my_hit_id_ExtSync[my_detector]-1]))
						{
							
							ts_tempDet5.push_back(my_time_stamp);
							side_tempDet5.push_back(my_side);
							asic_tempDet5.push_back(my_asic);
							ch_tempDet5.push_back(my_ch);
							
							index_tempDet5.push_back(my_hit_id_Sync[my_detector]);
							my_hit_id_Sync[my_detector]++;
							
							
							time_diff = ts_tempExtDet5[my_hit_id_ExtSync[my_detector]-1] -  my_time_stamp;
							//if( sqrt(time_diff*time_diff) < time_window){
							coincDet5++;
							HistTimeSi5_Pl->Fill(time_diff/1.e6);
							
							//}
						}
						
						
						
					} // end of type ==3
					
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
						
						my_detector= s_entry_sort.det_id;
						my_side= s_entry_sort.side_id;
						my_strip= s_entry_sort.asic_id*128+s_entry_sort.ch_id;
						
						
						//out of sync if... !SYNC && PAUSE
						// QQQ: but check condition PAUSE&!SYNC (?)
						if(s_entry_sort.sync_flag && !s_entry_sort.pause_flag){
							my_sync_flag=true;
						}
						
						s_entry.energy= my_energy;
						s_entry.time_stamp= my_time_stamp;
						// s_entry.time_ext= my_time_ext;
						// s_entry.type= my_type;
						s_entry.detector= my_detector;
						s_entry.side= my_side;
						s_entry.strip= my_strip;
						s_entry.sync_flag= my_sync_flag;
						
						if(flag_debug && i<1000){
							log_file<<":::data"<<i<<"::: "
							<< s_entry.energy<<"  "
							<< s_entry.time_stamp<<"  "
							// << s_entry.time_ext<<"  "
							//  << s_entry.type<<"  "
							<< s_entry.detector<<"  "
							<< s_entry.side<<"  "
							<< s_entry.strip<<"  "
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
				histo_file->cd();
				//histo_file->Write("Hhistogram");
				hprof1ExtSync->Write();
				hprof2ExtSync->Write();
				hprof4ExtSync->Write();
				hprof5ExtSync->Write();
				hprofExtDiff1_4->Write();
				hprofExtDiff1_5->Write();
				hprofExtDiff2_4->Write();
				hprofExtDiff2_5->Write();
				HistTimeSi1_Pl->Write();
				HistTimeSi2_Pl->Write();
				HistTimeSi4_Pl->Write();
				HistTimeSi5_Pl->Write();
				hprof1ExtDiff->Write();
				
				// last step: close all files (closing them can delete some objects from memory)
				output_file->Close();         
				input_file->Close(); // Close TFile
				
				std::cout << "coincidences in Det1=" << coincDet1 << std::endl;
				std::cout << "coincidences in Det2=" << coincDet2 << std::endl;
				std::cout << "coincidences in Det4=" << coincDet4 << std::endl;
				std::cout << "coincidences in Det5=" << coincDet5 << std::endl;
				
				std::cout << "\n Calibrator ...done with file "<<iter_f<<" ." << std::endl;     
				
				
				for(int k=0;k<9000;k++)
				{	
					for (int kk=0;kk<1397;kk++) //Det 1
					{
						
						timediff= (ts_tempExtDet1[k]-ts_tempDet1[kk]); ///1.e6;
						if ( timediff < 200000 && timediff > -200000.){
							HistTime2Si1_Pl->Fill(timediff/1.e6);
							std::cout << "Det1 coincidence: k=" << k << " kk= " << kk << " diff=" << timediff << " Side: " << side_tempDet1[kk] << " Asic: " << asic_tempDet1[kk] << " Channel: " << ch_tempDet1[kk] << std::endl;
							std::cout << " ts ext =" <<  ts_tempExtDet1[k] << "*5ns  ts= " << ts_tempDet1[kk] << " *5ns" << std::endl;
						}
					}
					
					for (int kk=0;kk<16359;kk++) // Det 5
					{
						
						timediff= (ts_tempExtDet5[k]-ts_tempDet5[kk]); ///1.e6;
						if ( timediff < 20000 && timediff > -20000.){
							HistTime2Si1_Pl->Fill(timediff/1.e6);
							std::cout << "Det5 coincidence: k=" << k << " kk= " << kk << " diff=" << timediff << " Side: " << side_tempDet5[kk] << " Asic: " << asic_tempDet5[kk] << " Channel: " << ch_tempDet5[kk] << std::endl;
							std::cout << " ts ext =" <<  ts_tempExtDet5[k] << "*5ns  ts= " << ts_tempDet5[kk] << " *5ns" << std::endl;
						}
					}
				}
				
				
				HistTime2Si1_Pl->Write();
				
				histo_file->Close();
				
				
				
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
				name_input_file << dut->GetDataOutputPath()
				<< "Cosmic_"
				<< dut->GetDataFilePrefix()
				<< "_"
				<< itr_run
				<< "_sort.root"; //previous step=> Converter.cc
				//			<< "_midas.root"; //previous step=> Converter.cc
				//	name_input_file << dut->GetDataInputPath()
				//			<< dut->GetDataFilePrefix()
				//			<< itr_run
				//			<< "_midas.root"; //previous step=> Converter.cc
				
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
