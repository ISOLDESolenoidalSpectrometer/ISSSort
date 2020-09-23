#ifndef _DutClass_hh
#define _DutClass_hh

#include "Common.h"

#include "TH2.h"

#include <vector>
#include <string>

//class AnalysisClass;

class TFile;

class DutClass{
 public:
  
  DutClass( std::string dut_name,
	    int dut_sides,
	    int dut_asics,
	    std::string base_path,
	    std::string data_input_path,
	    std::string data_output_path,
	    std::string data_file_prefix,
	    std::string data_output_prefix,
	    std::string data_input_list,
	    std::string data_list_user,
	    int run_first,
	    int run_total,
	    bool flag_first_in_pipe,
	    bool flag_input_type,
	    bool flag_convert_file,
	    bool flag_sort_file,
	    bool flag_calibrate_file,
	    bool flag_eventbuild_file

	    ); 
  
  ~DutClass();
  
 public:
  void SetDutName (std::string name);
  void SetDutSides (int value);
  void SetDutAsics (int value);
  void SetBasePath (std::string name);
  void SetDataInputPath (std::string name);
  void SetDataOutputPath (std::string name);
  void SetDataFilePrefix (std::string name);
  void SetDataOutputPrefix (std::string name); 
  void SetDataInputList (std::string name);
  void SetDataListUser (std::string name);
  void SetRunFirst (int value);
  void SetRunTotal (int value);
  void SetFlagFirstInPipe (bool flag);
  void SetFlagInputType (bool flag);
  void SetFlagConvertFile (bool flag);
  void SetFlagSortFile (bool flag);
  void SetFlagCalibrateFile (bool flag);  
  void SetFlagEventBuildFile (bool flag);   

  
  void SetUpDut();
  void SetRunPresent (int value);
  void SetEntry (common::struct_entry_dut s_entry);
  //  void SetEntryMidas (common::struct_entry_midas s_entry);

  std::string GetDutName();
  int GetDutSides();
  int GetDutAsics();
  std::string GetBasePath();
  std::string GetDataInputPath();
  std::string GetDataOutputPath();
  std::string GetDataFilePrefix();
  std::string GetDataOutputPrefix();
  std::string GetDataInputList();
  std::string GetDataListUser();
  int GetRunFirst();
  int GetRunTotal();
  bool GetFlagFirstInPipe();
  bool GetFlagInputType();
  bool GetFlagConvertFile();
  bool GetFlagSortFile();
  bool GetFlagCalibrateFile();
  bool GetFlagEventBuildFile();

  
  TFile* GetFileOutput ();
  int GetRunPresent (); 
  common::struct_entry_dut GetEntry ();
  //  common::struct_entry_midas GetEntryMidas ();

 private:
  std::string dut_name;
  int dut_sides;
  int dut_asics;
  std::string base_path;
  std::string data_input_path;
  std::string data_output_path;
  std::string data_file_prefix;
  std::string data_output_prefix;
  std::string data_pedestal_file_name;
  std::string data_input_list;
  std::string data_list_user;
  int run_first;
  int run_total;
  bool flag_first_in_pipe;
  bool flag_input_type;
  bool flag_convert_file;
  bool flag_sort_file;  
  bool flag_calibrate_file; 
  bool flag_eventbuild_file;   

  
  common::struct_entry_dut s_entry_dut;

  TFile* file_output;
  int run_present;
  
};

#endif
