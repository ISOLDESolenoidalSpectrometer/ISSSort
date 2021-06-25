#ifndef _Common_hh
#define _Common_hh

#include <TCanvas.h>
#include <TFile.h>
#include <TH2D.h>
#include <TObject.h>

#include <sstream>
#include <string>

namespace common {

const int n_caen_mod	= 2;	///< 2 CAEN V1725 modules
const int n_caen_ch		= 16;	///< 16 channels per CAEN V1725

const int n_module	= 3;	///< 3 modules make a full array
const int n_asic	= 6;	///< 4 p-side + 2 n-side per module
const int n_channel	= 128;	///< 128 channels per ASIC

const int n_row		= 4;	///< 4x2 DSSSDs per module, but paired; dE-E for recoil, gas cathodes (13?)
const int n_pstrip	= 128;	///< number of p-side strips in each DSSSD
const int n_nstrip	= 11;	///< strip number of DSSSD
const int n_side	= 2;	///< p-side and n-side

//const int n_unit	= n_module * n_asic;	///< number of physical ASIC devices/units
//const int n_sector	= n_module * 2;			///< 6 sides of the hexagonal array or 4 quadrants for recoil, 4 arms of the FiFi
//const int n_pairs	= n_module * n_row;	///< this is basically pairs of wafers relevant for readout
//const int n_wafer	= n_sector * n_row;	///< this is "detectors" in event builder

const int extt_code = 14;	///< This is the info code for the external timestamp, 5 before 2019 and 14 after 2019 (This is ISS == 14)
const int sync_code = 4;	///< Medium significant bits of the timestamp are here
const int thsb_code = 5;	///< Highest significant bits of the timestamp are here

const int caen_pulser_mod = 0;	///< Location of the pulser in the CAEN system (module)
//const int caen_pulser_ch = 13;	///< Location of the pulser in the CAEN system (module) - CERN experiments
const int caen_pulser_ch = 1;	///< Location of the pulser in the CAEN system (module) - Daresbury test
const int pulser_code = 20;		///< Info code when we have a pulser event in InfoData packets
const int caen_ebis_mod = 0;	///< Location of the EBIS signal in the CAEN system (module)
const int caen_ebis_ch = 14;	///< Location of the EBIS signal in the CAEN system (module)
const int ebis_code = 21;		///< Info code when we have an EBIS event in InfoData packets
const int caen_t1_mod = 0;		///< Location of the T1 signal in the CAEN system (module)
const int caen_t1_ch = 15;		///< Location of the T1 signal in the CAEN system (module)
const int t1_code = 22;			///< Info code when we have a T1 event in InfoData packets

}

#endif
