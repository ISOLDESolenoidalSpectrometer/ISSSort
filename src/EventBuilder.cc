#include "EventBuilder.hh"
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

int p_even_hits_pulser; //if hit>p_even_hits_max: assume it's a pulser event...
double p_even_time_window;

void LoadParametersEventBuilder(int n_dssd, std::string file_name){

  std::cout << "Loading EventBuild parameters for stack of "<<n_dssd<<" DSSDs. Currently only default parameters implemented"
	    << "\n     but function receives file: "<<file_name<<std::endl;
 
  p_even_hits_pulser= 64;
  p_even_time_window= 750; // depends on calibration used by previous step
 


  return;
}


void DoEventBuild(std::string input_list_name, std::string output_file_name, std::string log_file_name, std::string param_file_name){


  bool flag_debug= true;
  int n_cout =0;

 
  LoadParametersEventBuilder(common::n_module, param_file_name);
  std::cout << " Loaded parameters?: "<<  p_even_hits_pulser << " "<<p_even_time_window<<std::endl;
 

  //to read list of input files
  std::string input_file_name;
  int n_input_files;

  std::ifstream input_list (input_list_name.c_str(), std::ios::in);
  if (input_list.is_open()){

    //Create log file.
    std::ofstream log_file;
    log_file.open(log_file_name.c_str(), std::ios::out);

    // Create Root ntuple.
    TFile* output_file = new TFile(output_file_name.c_str(),"recreate");
    TTree* output_tree = new TTree("R3B_event","R3B_event");
    common::struct_entry_event s_entry;


    std::stringstream branch_info;
    //THIS WILL ONLY WORK FOR 1-9 DSSDs in STACK!
    char detectors;
    if(common::n_detector<10) detectors= (char)(((int)'0')+common::n_detector);
    else {

      detectors='2'; //my default for now

      std::cout<< "\n\n************************************************"
	       << "    WARNING!! Too many detectors defined for the stack for this implementation of the code:"<<common::n_detector
	       << "\n  something bound to go very wrong!"
	       << "\n**************************************************"<<std::endl;

      log_file<< "\n\n***  WARNING!! Too many detectors for DSSD stack:"<<common::n_detector<<" ***"
	       << "\n  something bound to go very wrong!\n"<<std::endl;

    }

     branch_info <<"energy_sum["<<detectors<<"][2]/D:";
     branch_info <<"energy_max["<<detectors<<"][2]/D:";
     branch_info <<"x["<<detectors<<"][2]/D:";
     branch_info <<"x_energy_max["<<detectors<<"][2]/D:";
     branch_info <<"x_rms["<<detectors<<"][2]/D:";
     branch_info <<"x_min["<<detectors<<"][2]/D:";
     branch_info <<"x_max["<<detectors<<"][2]/D:";
     branch_info <<"time/D:time_rms/D:time_min/D:time_max/D:";
     branch_info <<"n_hit["<<detectors<<"][2]/I:";
     branch_info << "flag_time/O";


    std::string contenedor;
    contenedor = branch_info.str();

    std::cout << " trying to use this TBranch name: " << contenedor << std::endl;
    //    output_tree->Branch("entry_event", &s_entry.x,branch_info.data());
    //    output_tree->Branch("entry_event", &s_entry.energy_x_low[0],contenedor.data());
    output_tree->Branch("entry_event", &s_entry.energy_sum[0],contenedor.data());


    //Open Root file to save a few diagnostic histograms
    //TFile* histo_file = new TFile(histo_file_name.c_str(),"update");
    //---- declare histograms here!  ----//
    //TH1I * Hhistogram= new TH1I("Hhistogram","my histogram;x [ch]",10,-0.5,9.5);
    //output_file->cd(); 

    input_list >> n_input_files;

    if( n_input_files < 1) std::cout << "... invalid number of data input files for Calibrate step! N= "<< n_input_files <<std::endl;
    
    //loop through each file to be processed
    for(int iter_f=0;iter_f<n_input_files;iter_f++){
      
      // Read name and open next Root input file.
      input_list >> input_file_name;
      std::cout << "Event Building file: " << input_file_name << std::endl;
      TFile* input_file = new TFile(input_file_name.c_str(),"read");

      if (input_file != 0){

	TTree *input_tree= (TTree*) input_file->Get("R3B_calib");

	common::struct_entry_calibrate s_entry_calib;
	input_tree->SetBranchAddress("entry_calib",&s_entry_calib,0);

	long long n_entries;
	n_entries = input_tree->GetEntries();

	std::cout << "\n\n Event Building: number of entries in input tree= " << n_entries << ", for file " << input_file_name << std::endl;

	//for structure struct_entry_event
	//skipe this intermediate step...
	//	double my_energy_x_low[common::n_dssd];
	//	double my_energy_x_high[common::n_dssd];

	//arrays to check synchronizatino is going fine (or not!)
	//unsigned long tm_stp_msb_modules[common::n_fee64]={0};

	//bool fill_flag;
	bool flag_skip=false;
	bool flag_close_event= false;
	bool my_flag_time= false;

	double strip_x_min[common::n_detector][2]={{0}};
	double strip_x_max[common::n_detector][2]={{0}};
	//int strip_y_min[common::n_dssd]={0};//should be initialized to 127!
	//int strip_y_max[common::n_dssd]={0};

	double time_prev=0; //to check data stream is time ordered
	double my_time_max=0;
	double my_time_min=0;
	double my_time_first=0;

	double sum_e[common::n_detector][2]={{0}}; // ***R3B*** only one range
	double e_max[common::n_detector][2]={{0}};


	double sum_x[common::n_detector][2]={{0}};
	double strip_e_max[common::n_detector][2]={{0}};

	int hits[common::n_detector][2]={{0}};

	long long n_events=0;
	int hit_counter=0;

	int my_strip;
	int my_detector;
	int my_side; //***r3b***


	for(int j=0;j<common::n_detector;j++){
		strip_x_min[j][0]= 9999;
		strip_x_min[j][1]= 9999;
		strip_x_max[j][0]= -1;
		strip_x_max[j][1]= -1;
		strip_e_max[j][0]= -1;
		strip_e_max[j][1]= -1;
		e_max[j][0]=-1;
		e_max[j][1]=-1;
	}


	//
	// Main loop over TTree to process raw MIDAS data entries
	//
	for(long long i=0; i<n_entries; i++){

	  input_tree->GetEntry(i);
	  // --- check values will not make code crash!! e.g. s_entry_midas.mod_id within boundaries!
	  
	  /*****************************/
	  /*    do something with it!  */
	  /*****************************/

	  //DO SOME SANITY CHECKS? DSSD plane number, strip number, etc...

	  //
	  //add values of this entry to current event
	  //
	  my_side= s_entry_calib.side;
	  my_strip= s_entry_calib.strip;
	  my_detector= s_entry_calib.detector;

	  //QQQ: include condition to check signal above (software) threshold!!

	  //Building Event with information from ADC(high/low range) and EXT timestamp correlations
	  //***R3B*** all same type (hit if energy above threshold)
	  //if(my_type==0 || my_type==1 || my_type==3){

	  //check time stamp monotonically increases!
	  if(time_prev>s_entry_calib.time_stamp){
	      std::cout<<"*t*";

	      log_file << "\n\n**** WARNING!! R3B time stamp gone down! ***"
		       << "\n  t_prev: " << time_prev<<", t_this:"<<s_entry_calib.time_stamp
		       << "   (entry= "<<i<<")\n"<<std::endl;

	      my_flag_time= false; //***r3b*** must reset this somewhere...
	      //*r3b* allow for 'out of order' data for now  //  flag_skip=true;
	    }

	  //***r3b*** no check of time order (for now)	//  else{ //if all is good with life

	  //if this is first datum included in Event
	  if(hit_counter==0){
	    my_time_min= s_entry_calib.time_stamp;
	    //   time_ext_offset= s_entry_calib.time_ext-s_entry_calib.time_aida;
	    my_time_max= s_entry_calib.time_stamp; //also is largest (time) datum

	    my_time_first= s_entry_calib.time_stamp;

	    my_flag_time= true;
	  }

	  //	  if(my_type==3) s_entry.flag_veto= true;
	  //else s_entry.flag_veto= false;
	      
	  hit_counter++; //increase counter for bits of data included in this event
	  // }  //else{ (if all good with life)

	  //record time of R3B for next 
	  time_prev= s_entry_calib.time_stamp;
	  // ***r3b*** not used //} //if(correct type)

	  //if ADC hit: record position and energy
	  //and multiplicity
	  // ***R3B*** only ADC hits! //	  if( (my_type==0 || my_type==1) && !flag_skip){
	    
	  // no my_hits_type[my_type]= my_hits_type[my_type]+1; //count how many of each...

	  //	      if(my_strip<128){ //x strip! -> 0:127
	  //if(my_side==0){ //***R3B*** two sides per module	
	  //add to position of this event
	  sum_x[my_detector][my_side]= sum_x[my_detector][my_side]+ my_strip;
	  //record one additional hit on this event
	  hits[my_detector][my_side]= hits[my_detector][my_side]+1;
	  //check if this is lowest or highest strip unmber for this event
	  if(my_strip<strip_x_min[my_detector][my_side]) strip_x_min[my_detector][my_side]= my_strip;
	  if(my_strip>strip_x_max[my_detector][my_side]) strip_x_max[my_detector][my_side]= my_strip;


	  //if signal from low range (decay)
	  //		if(my_type==0){		  
	  //add to energy of this event
	  sum_e[my_detector][my_side]= sum_e[my_detector][my_side]+ s_entry_calib.energy;
	  if( s_entry_calib.energy > e_max[my_detector][my_side]){
	    e_max[my_detector][my_side]= s_entry_calib.energy;
	    strip_e_max[my_detector][my_side]= my_strip;
	  }


	  if(s_entry_calib.time_stamp>my_time_max) my_time_max= s_entry_calib.time_stamp;
	  else if(s_entry_calib.time_stamp<my_time_min) my_time_min= s_entry_calib.time_stamp;

 
	  //	}// if ADC data...

	     
	  //update skip flag and time aida previous
	  flag_skip= false; //????
	  

	  //------------------------------
	  //  check if last datum from this event and do some cleanup
	  //------------------------------
	  
	  // last entry in TTree??
     
	  //	  long long next; 
	  //	  next= i+1;
	  if( (i+1)==n_entries){
	    flag_close_event=true; //set flag to close this event
	    
	  }
	  else{  //check if next entry is beyond time window: close event!
	    input_tree->GetEntry(i+1);
	    
	    // window= time_stamp_first +/- time_window
	    if( s_entry_calib.time_stamp > (my_time_first + p_even_time_window) || s_entry_calib.time_stamp < (my_time_first - p_even_time_window) ){
		
		flag_close_event= true; //set flag to close this event
		
		//but also check if event could have outrun window, or overlap between close events
		if(s_entry_calib.time_stamp < (my_time_max+p_even_time_window) || s_entry_calib.time_stamp > (my_time_min-p_even_time_window)){
		    

		  my_flag_time= false;
		  
		  if(n_cout<25){
		    std::cout<< "\n\n*** WARNING!! Event window too narrow?!"
			     << "\n  t_event_first= " << my_time_first
			     << "\n  t_event_min= " << my_time_min
			     << "\n  t_event_max= " << my_time_max
			     << "\n  t_next_datum= " << s_entry_calib.time_stamp		       
			     << "\n  Event#: "<<n_events <<", R3B_calib entry#: " <<i <<" (last in event)\n"<<std::endl;
		    n_cout++;
		  }
		  
		  log_file<< "\n*** WARNING!! Event window too narrow?!"
			  << "\n  t_event_first= " << my_time_first
			  << "\n  t_event_min= " << my_time_min
			  << "\n  t_event_max= " << my_time_max
			  << "\n  t_next_datum= " << s_entry_calib.time_stamp			       
			  << "  Event#: "<<n_events <<", R3B_calib entry#: " <<i <<" (last in event)\n"<<std::endl;	
		  
		}
	      }
	    } //if next entry beyond time window: close event!
	  
	    
	    //-------------------
	    //if close this event and number of datums in event>0
	    //-------------------
	    if(flag_close_event && hit_counter>0){
	  
	      s_entry.time= my_time_min+(my_time_max-my_time_min)/2; 
	      // s_entry.time_ext= s_entry.time_aida+time_ext_offset;
	      s_entry.time_min= my_time_min;
	      s_entry.time_max= my_time_max;
	      s_entry.time_rms= -1;
	      
	      /*** calculate relevant info in struct_entry_event and  write it to file ***/
	      for(int j=0;j<common::n_detector;j++){
		
		//this can be done directly during processing entry
		s_entry.energy_sum[j][0]= sum_e[j][0];
		s_entry.energy_sum[j][1]= sum_e[j][1];
		s_entry.energy_max[j][0]= e_max[j][0];
		s_entry.energy_max[j][1]= e_max[j][1];
		//s_entry.energy_y_low[j]= sum_e_y_low[j];
		//s_entry.energy_x_high[j]= sum_e_x_high[j];
		//s_entry.energy_y_high[j]= sum_e_y_high[j];
		
		for(int k=0;k<2;k++){
		  if(hits[j][k]>0){
		    s_entry.x[j][k]= sum_x[j][k]/hits[j][k];
		    s_entry.x_energy_max[j][k]= strip_e_max[j][k];
		    s_entry.x_min[j][k]= strip_x_min[j][k];
		    s_entry.x_max[j][k]= strip_x_max[j][k];
		    s_entry.x_rms[j][k]= -1;
		  }
		  else{
		    s_entry.x[j][k]=-1;
		    s_entry.x_energy_max[j][k]= -1;
		    s_entry.x_min[j][k]=-1;
		    s_entry.x_max[j][k]=-1;
		    s_entry.x_rms[j][k]= -1;
		  }
		}
		
		s_entry.n_hit[j][0]= hits[j][0];
		s_entry.n_hit[j][1]= hits[j][1];
	      }
	      
	      s_entry.flag_time = my_flag_time; /// implement later?
	      
	      
	      
	      
	      
	      /***** done with calculating values for this struct_event_entry ******/
	    
	      if(flag_debug && n_events<10){
		std::cout<< "\n ++++ EVENT "<<n_events<< "++++\n"
			 << " t(aida): "<<  "0x"<< std::dec <<s_entry.time- 2.55766e+13
			 << "\n t(aida_min): "<<  "0x"<< std::hex <<s_entry.time_min- 2.55766e+13
			 << "\n t(aida_max): "<< "0x"<<  std::hex <<s_entry.time_max- 2.55766e+13
			 << "\n t(aida calib): "<< "0x"<< std::hex << s_entry_calib.time_stamp - 2.55766e+13
			 << "\n Multi: "<< std::dec <<  hit_counter <<std::endl;
	      }
	      
	      
	      //----------------------------------
	      // Now collect data from this entry
	      //----------------------------------
	      //save this entry to TTree
	      output_tree->Fill();
	      n_events++;
	      //Fill histograms here

	    
	      //--------------------------
	      //clear values of arrays to store intermediate info
	      //-------------------------
	      
	      flag_close_event= false; //wait for noe event
	      flag_skip= false;
	      hit_counter=0;
	      //my_multi=0;      
	      //my_hits_type[0]=0;
	      //my_hits_type[1]=0;
	      
	      for(int j=0;j<common::n_detector;j++){
		strip_x_min[j][0]= 9999;
		strip_x_min[j][1]= 9999;
		strip_x_max[j][0]= -1;
		strip_x_max[j][1]= -1;
		strip_e_max[j][0]= -1;
		strip_e_max[j][1]= -1;
		e_max[j][0]=-1;
		e_max[j][1]=-1;

		sum_e[j][0]=0;
		sum_e[j][1]=0;
	      
		sum_x[j][0]=0;
		sum_x[j][1]=0;
		hits[j][0]=0;
		hits[j][1]=0;

		my_flag_time= true;
	      }
	    
	      
	    } //if close event && hit_counter>0
	    
	  } // End of main loop over TTree to process raw MIDAS data entries (for n_entries)
  
	
	output_file->Write();
	output_file->Print(); 

	// write histograms (etc...) to common root file
	//histo_file->cd();
	//histo_file->Write("Hhistogram");
 
	// last step: close all files (closing them can delete some objects from memory)
	//histo_file->Close();
	output_file->Close();         
	input_file->Close(); // Close TFile
	
	std::cout << "\n EventBuilder ...done with file "<<iter_f<<" ." << std::endl;     
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

  std::cout<< "\nFlush..."<<std::endl;

  return;
}

void EventBuilder (DutClass* dut){
 
  //DoCalibrate("one", "two", "three", "four"); 
  //std::cout << " Done with test"<<std::endl;
 
  /*******************/


  // Do convertion of ASCII files ?
  if ( !dut->GetFlagEventBuildFile() ) {return;}
  
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
      /************BUBUBUBUGGGGG****************
     //user provided file with list of input data files 
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
	user_list.close();
      ***********************************/
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
			<< "_calib.root"; //previous step=> Calibrator.cc

	input_list << 1 << '\n' <<name_input_file.str() << '\n';

  }

  //close temp file with list of input data files
  input_list.close();

  //std::string input_list_name;
  //input_list_name= dut->GetDataInputList() ; 

  std::stringstream name_output_file;
  name_output_file << dut->GetDataOutputPath() 
		   << dut->GetDataOutputPrefix()
		   << "_event.root";
  
  std::stringstream name_log_file;
  name_log_file << dut->GetDataOutputPath()
		<< dut->GetDataOutputPrefix()
		<< "_event_log.txt";
 
  std::stringstream name_param_file;
  name_param_file << dut->GetBasePath() << "/files/parameters.txt";
    
  DoEventBuild(input_list_name.c_str(), name_output_file.str(), name_log_file.str(), name_param_file.str());

  /***********/
  
}
