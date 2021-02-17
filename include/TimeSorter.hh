#ifndef _TimeSorter_hh
#define _TimeSorter_hh

#include <bitset>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>

#include <TFile.h>
#include <TTree.h>
#include <TTreeIndex.h>
#include <TProfile.h>

// Common header
#ifndef __COMMON_HH
# include "Common.hh"
#endif

// Calibration header
#ifndef __CALIBRATION_HH
# include "Calibration.hh"
#endif

class TimeSorter {

public:

	TimeSorter( Calibration *cal );
	virtual ~TimeSorter();
	
	void	Initialise();
	void 	LoadParametersSort();
	void	SortFile( std::string input_file_name,
					  std::string output_file_name,
					  std::string log_file_name );
	bool	SetEntry( long long ts, long long ts_ext );
	void	SortTree();

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
	
	TFile *input_file;
	TTree *input_tree;
	TFile *output_file;
	TTree *output_tree;
	TTree *sorted_tree;

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
	long long nb_idx;
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

};

#endif
