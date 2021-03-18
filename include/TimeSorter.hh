#ifndef _TimeSorter_hh
#define _TimeSorter_hh

#include <bitset>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <ctime>

#include <TFile.h>
#include <TTree.h>
#include <TTreeIndex.h>

// Common header
#ifndef __COMMON_HH
# include "Common.hh"
#endif

class TimeSorter {

public:

	TimeSorter();
	virtual ~TimeSorter();
	
	void	SortFile( std::string input_file_name,
					  std::string output_file_name,
					  std::string log_file_name );
	void	SortTree();

private:
		
	TFile *input_file;
	TTree *input_tree;
	TFile *output_file;
	TTree *output_tree;
	
	TTreeIndex *att_index;

	// Counters
	unsigned long long n_entries;
	unsigned long long nb_idx, idx;
	
	// Sort time
	time_t t_start;

};

#endif
