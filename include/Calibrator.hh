#ifndef _Calibrator_hh
#define _Calibrator_hh

#include <bitset>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>

#include <TFile.h>
#include <TTree.h>
#include <TProfile.h>

// Common header
#ifndef __COMMON_HH
# include "Common.hh"
#endif

// Calibration header
#ifndef __CALIBRATION_HH
# include "Calibration.hh"
#endif

class Calibrator {

public:

	Calibrator( Calibration *cal );
	virtual ~Calibrator();
	
	bool	SetInputFile( std::string input_file_name );
	void	SetInputTree( TTree* user_tree );
	void	SetOutput( std::string output_file_name );

	void	Initialise();
	void	MakeHists();
	void 	LoadParametersCalib();
	bool	SetEntry( long long ts, long long ts_ext );

	unsigned long CalibFile( unsigned long start_entry = 0 );

	inline void CloseOutput(){
		output_file->Close();
		input_file->Close(); // Close TFile
		log_file.close(); //?? to close or not to close?
	};
	inline TFile* GetFile(){ return output_file; };
	inline TTree* GetTree(){ return output_tree; };


private:
	
	bool	p_ch_enable[common::n_module][common::n_asic][common::n_channel];
	int		p_det_type[common::n_module][common::n_asic][common::n_channel];
	int		p_layer_id[common::n_module][common::n_asic][common::n_channel];
	int		p_sector_id[common::n_module][common::n_asic][common::n_channel];
	int		p_strip_id[common::n_module][common::n_asic][common::n_channel];
	int		p_side_id[common::n_module][common::n_asic][common::n_channel];
	
	// Branches
	common::info_data s_info;
	common::event_id  s_id;
	common::adc_data  s_adc;

	common::real_data s_data;

	Calibration *cal;
	
	// Log file
	std::ofstream log_file;

	// ROOT files
	TFile *input_file;
	TTree *input_tree;
	TFile *output_file;
	TTree *output_tree;

	// For real data and full timestamps
	long long my_tm_stp[common::n_module];
	long long my_tm_stp_ext[common::n_module];
	unsigned long my_tm_stp_msb; // most significant bits of time-stamp (in info code)
	unsigned long my_tm_stp_hsb; // most significant bits of time-stamp (in info code)

	// arrays to check synchronization is going fine (or not!)
	long long t0_sync;
	unsigned long tm_stp_msb_modules[common::n_module];	// medium significant bits
	unsigned long tm_stp_hsb_modules[common::n_module];	// highest significant bit

	// Flags
	bool no_sync_flag;
	bool Ext_flag;
	bool DetTag_Ext[common::n_unit];
	bool SyncTag[common::n_unit];
	
	// Counters
	unsigned long long n_entries;
	int ctr_hit[common::n_module];		// hits on each module
	int ctr_hit_Ext[common::n_module];	// external timestamps
	int ctr_hit_Sync[common::n_module];	// sync timestamps
	int ctr_pause[common::n_module];	// info code 2
	int ctr_resume[common::n_module];	// info code 3
	int ctr_code_sync[common::n_module];	// info code "common::sync_code"
	int ctr_code4[common::n_module];	// info code 4
	int ctr_code7[common::n_module];	// info code 7
	
	// Sort time
	time_t t_start;
	
	// Histograms
	TProfile *hprof[common::n_module];
	TProfile *hprofExt[common::n_module];
	TProfile *hprofSync[common::n_module];


};

#endif
