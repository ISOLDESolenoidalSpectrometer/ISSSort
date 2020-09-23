// My code include.
//#include "AnalysisClass.h"
//#include "Analyzer.h"
#include "Common.h"
#include "Converter.h"
#include "TimeSorter.h"
#include "Calibrator.h"
#include "EventBuilder.h"
#include "DutClass.h"

// Analysis include.
//#include "AnalysisPulse.h"

// ROOT include.
#include <TTree.h>

// C++ include.
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <sstream>



// DESCRIPTION:
// A Unit Under Test (UUT) object is created.
// The UUT contains as may Device Under Test (DUT) as you need.
// Each DUT contains a set of analysis.

DutClass* MakeDut( std::string name, std::string runNo ){
  
  std::string dut_name = name;
  
  int dut_sides = common::n_side;
  int dut_asics = common::n_side*common::n_asic;
  
  //path for data input files (merged MIDAS or Root)
  std::string base_path  = "/home/ISOL/Midas-to-Root/";
 
  //path for data input files (merged MIDAS or Root)
  //  std::string data_input_path  = "/MIDAS/R3B-Data/"; //TapeData/NULL/";
  std::string data_input_path  = "/TapeData/LivTest/";


  //path to save Root output files
  std::string data_output_path = "/home/ISOL/Midas-to-Root/analysis/"; 

  //file for temprary storage of list of input files
  //make this hard coded ?
  std::string data_input_list = "/home/ISOL/Midas-to-Root/temp/test_input.dat";
  //file with list of input data files provided by the User
  std::string data_list_user = "/home/ISOL/Midas-to-Root/files/my_data.dat";



  std::stringstream output_name;
  output_name << "Alphas_R" << runNo << "_0";
 
  std::stringstream data_file_prefix;
  data_file_prefix << "R" << runNo;
  std::string data_output_prefix = output_name.str();
  int run_first = 0; //CHANGE OUTPUT PREFIX!!!
  int run_total = 1;

  std::cout << "\n\n????is this it??? " << data_output_prefix << " "<<run_first<<std::endl;


  
 
  bool flag_first_in_pipe = true;
  bool flag_input_type = false; //true= read from text file, false= iterate
  //bool flag_input_type = true; //true= read from text file, false= iterate

  //select what steps of the analysis to go through
  bool flag_convert_file = true;
  bool flag_sort_file = true;
  bool flag_calibrate_file = true;
  bool flag_eventbuild_file = true;

  //allways false!
  //  bool flag_analyze_file = false;
  
  //  std::vector<AnalysisClass*> v_analysis;
  //v_analysis.push_back(new AnalysisPulse());
  

 
  return new DutClass( dut_name,
		       dut_sides,
		       dut_asics,
		       base_path,
		       data_input_path,
		       data_output_path,
		       data_file_prefix.str(),
		       data_output_prefix,
		       data_input_list,
		       data_list_user,
		       run_first,
		       run_total,
		       flag_first_in_pipe,
		       flag_input_type,
		       flag_convert_file,
		       flag_sort_file,
		       flag_calibrate_file,
		       flag_eventbuild_file
		    );
 
}

int main ( int argc, char **argv ){
  
  //  std::cout << "Let it be !2!" << std::endl;

  if( argc < 2 || argc > 2 ) {

    std::cout << "Usage: " << argv[0] << " <runNo>" << std::endl;

  }

  else {

    std::string runNo = argv[1];
    std::cout << "Run number " << runNo << std::endl;
    DutClass* dut = MakeDut( "ISS", runNo );
    Converter( dut );      // To convert midas to root                                                                                      
  }

  //  for(int i=0; i<=0; i++)  {
    
    //    DutClass* dut = MakeDut("ISS-at-CERN", i);
    //DutClass* dut = MakeDut("ISS", i);
    
    // Do convertion.
    //Converter( dut );      // To convert midas to root
    //TimeSorter( dut );   // To order data in time
    //Calibrator( dut );   // To calibrate
    //EventBuilder( dut ); // To build event
    
    
    //  }

  return 0;
  
}
