#ifndef _Calibrator_hh
#define _Calibrator_hh

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

#include "Common.hh"


void Calibrator( std::string input_file_name, std::string output_file_name,
				 std::string log_file_name, std::string log_cal_name );

#endif
