#ifndef _Common_hh
#define _Common_hh

class DutClass;

class TCanvas;
class TH2D;
class TObject; 

#include <string>

namespace common {

const int n_side     = 2;   //
const int n_asic     = 16;  //
//  const int n_channel  = 128;
const int n_channel  = 16;

//new
const int n_module = 60; //***R3B***: (0-59) module = Nb of detectors x 2 (because 2 sides)
//const int n_detector = 1;
const int n_detector = 30; // 6 inner + 12 outer +(12 extra outer when available)
//const int n_detector = 48; // for STUB it is the channel id (max=  (16*3) channels)
//number of detector for stub exp.
//const int n_det = 9; // for STUB exp: STUB=0 to 3; Recoil=4;   // Not used in R3B
// const int n_dssd = 2;
//mapping of FEE64->DSSD geometry


//from Marcello Borri's code
struct struct_entry_dut{
	unsigned long tm_stp;
	unsigned int ADC_data;
	unsigned int ch_id;
	unsigned int ASIC_id;
	unsigned int mod_id;
};

// make some assumptions about size of variable types
// check them at begining of execution
// sizeof(int)>=4
// sizeof(long long)>=8

//variables sorted in decreasing oredr of size: from TTree.html
// if type==0, then tm_stp_lsb, info_field, adc_data and sample_lenght have information of
// next four waveform samples (quick and dirty temporary fix!)
struct struct_entry_unpack{
	unsigned long tm_stp_lsb;  //least significant bits timestamp
	unsigned long info_field;
	
	//  unsigned short sample_data[4];
	unsigned short adc_data;
	//    unsigned short sample_lenght; //not used for *R3B*
	// type: 0= sample waveform, 1= sample lenght, 2= info data, 3= ADC data
	unsigned char type;
	unsigned char hit;
	unsigned char mod_id;  // 6 bits
	unsigned char lad_id;  // most 5 significant bits from the 6 bits module_id
	unsigned char side_id; // last bit from the 6 bits module_id
	
	//  unsigned char side_id; //***R3B*** add this??
	unsigned char asic_id; //***R3B*** new variable
	unsigned char ch_id;
	//    unsigned char adc_range; //not used for *R3B*
	unsigned char info_code;
	
};


//variables sorted in decreasing order of size: from TTree.html
struct struct_entry_sort{
	long long tm_stp; //reconstructed timestamp (MSB+LSB)
	long long tm_stp_ext; //reconstructed timestamp (MSB+LSB)
	// not used *R3B* ->    long long info; //MBS info data (external timestamp), anything else(?)
	long long nevent;
	int type;
	int hit;
	int hit_id;
	int det_id;  // for STUB this is VME mod id
	int side_id;
	int asic_id; // new *R3B*
	int ch_id;
	// not used *R3B* ->    int type; // QQQ: 0= 20 MeV or 1 GeV (decays), 1= 20 GeV (checked pulser data only in type 0)
	// type>=10: type = info_code+10 (i.e., PAUSE, RESUME, SYNC100, etc...)
	int adc_data;
	bool sync_flag; // check SYNC100 pulses received for this module
	bool pause_flag; // check Pause signals followed by proper Resume signal: true= SYNC100 paused...
	bool ext_flag; // check Ext signals from CALIFA/ muon telescope
};


//variables sorted in decreasing order of size: from TTree.html
struct struct_entry_calibrate{
	double energy;
	double time_stamp; //time_aida;
	//*R3B* no:  double time_ext;
	// *R3B* no: int type; // 0: low range (decay), 1: high range (implant), 2: discriminator, 3: MBS(EXT) info code
	//    long nevent;
	//int type;
	int detector;
	int side;
	int strip;
	bool sync_flag;
};


struct struct_entry_event{
	
	double energy_sum[n_detector][2];
	double energy_max[n_detector][2];
	
	double x[n_detector][2];
	double x_energy_max[n_detector][2];
	double x_rms[n_detector][2]; //not implemented yet
	double x_min[n_detector][2];
	double x_max[n_detector][2];
	
	double time;
	double time_rms; //not implemented yet
	double time_min;
	double time_max;
	
	int n_hit[n_detector][2];
	
	bool flag_time; // logic?
};


extern void SaveObject(TObject* object, const std::string& name, DutClass* dut);
extern void SaveCanvas(TCanvas* canvas, const std::string& name, DutClass* dut);

}

#endif
