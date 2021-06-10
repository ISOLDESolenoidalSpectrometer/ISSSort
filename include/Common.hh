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

const int n_det		= 1;	///< detector types  just the array for now, expand for recoils etc.
const int n_layer	= 4;	///< 4x2 DSSSDs per module, but paired; dE-E for recoil, gas cathodes (13?)
const int n_strip	= 128;	///< strip number of DSSSD
const int n_side	= 2;	///< p-side and n-side

const int n_unit	= n_module * n_asic;	///< number of physical ASIC devices/units
const int n_sector	= n_module * 2;			///< 6 sides of the hexagonal array or 4 quadrants for recoil, 4 arms of the FiFi
const int n_pairs	= n_module * n_layer;	///< this is basically pairs of wafers relevant for readout
const int n_wafer	= n_sector * n_layer;	///< this is "detectors" in event builder

const int sync_code = 14;	///< This is the info code for the sync pulse, 5 before 2019 and 14 after 2019 (what is it though?)

// Liam's structs for different branches
struct info_data {
	
	unsigned long	tm_stp_lsb;	///< least significant bits timestamp
	unsigned long	field;	///< contains the MS bits for the timestamp
	unsigned char	type;	///< type: 0 = sample waveform, 1 = sample length, 2 = info data, 3 = ADC data
	unsigned char	code;	///< 5 is high significant bits of timestamp; 14 is external timestamp ...
	
};

struct event_id {
	
	unsigned char	mod;
	unsigned char	asic;
	unsigned char	ch;

};

struct adc_data {
	
	unsigned short	value;
	unsigned char	hit;

};

struct real_data {
	
	unsigned long long		t_ext;	///< external timestamp
	unsigned long long		time;	///< absolute reconstructed timestamp
	float 					energy;	///< calibrated energy
	unsigned char			hit;	///< hit event = 1, neighbour strip = 0
	unsigned char			det;	///< detector type 0 = array; 1 = recoil
	unsigned char			layer;	///< 4 wafers along array, 2 dE-E, 13 for gas
	unsigned char			sector;	///< 6 edges of hexagonal array, 4 quadrants of the recoil
	unsigned char			strip;	///< obvious for DSSSD, useless for recoils
	unsigned char			side;	///< p-side = 0; n-side = 1

	
};


// End of Liam's structs

}

#endif
