#ifndef __SETTINGS_HH
#define __SETTINGS_HH

#include <iostream>
#include <fstream>
#include <string>

#include "TSystem.h"
#include "TEnv.h"

/*! \brief Class to implement user "settings" to the ISS sort code
*
* A class to read in the settings file in ROOT's TConfig format. The ISSSettings class contains all the information about detector layout, data settings, and defines which detector is which. These can be changed directly in the settings file that is fed into ISSSort using the "-s" flag.
*
*/ 

class ISSSettings {

public:

	ISSSettings( std::string filename );
	inline virtual ~ISSSettings() {};
	
	void ReadSettings();
	void PrintSettings();
	void SetFile( std::string filename ){
		fInputFile = filename;
	}
	const std::string InputFile(){
		return fInputFile;
	}
	
	// Array settings
	inline unsigned int GetNumberOfArrayModules(){ return n_array_mod; };
	inline unsigned int GetNumberOfArrayASICs(){ return n_array_asic; };
	inline unsigned int GetNumberOfArrayChannels(){ return n_array_ch; };

	inline unsigned int GetNumberOfArrayRows(){ return n_array_row; };
	inline unsigned int GetNumberOfArrayPstrips(){ return n_array_pstrip; };
	inline unsigned int GetNumberOfArrayNstrips(){ return n_array_nstrip; };
	inline unsigned int GetNumberOfArraySides(){ return n_array_side; };

	
	// CAEN settings
	inline unsigned char GetNumberOfCAENModules(){ return n_caen_mod; };
	inline unsigned char GetNumberOfCAENChannels(){ return n_caen_ch; };
	inline unsigned int GetCAENModel( unsigned char i ){
		if( i < n_caen_mod )
			return caen_model[i];
		else return 1725;
	};
	inline unsigned char GetCAENExtras( unsigned char i, unsigned char j ){
		if( i < n_caen_mod && j < n_caen_ch )
			return caen_extras[i][j];
		else return 0;
	};
	
	// Mesytec settings
	inline unsigned char GetNumberOfMesytecModules(){ return n_mesy_mod; };
	inline unsigned char GetNumberOfMesytecChannels(){ return n_mesy_ch; };
	
	// VME totals
	inline unsigned int GetNumberOfVmeModules(){ return n_caen_mod + n_mesy_mod; };
	inline unsigned int GetMaximumNumberOfVmeChannels(){
		if( n_caen_ch > n_mesy_ch ) return n_caen_ch;
		else return n_mesy_ch;
	};

	
	// Info settings
	inline unsigned char GetExternalTriggerCode(){ return extt_code; };
	inline unsigned char GetSyncCode(){ return sync_code; };
	inline unsigned char GetExtItemCode(){ return ext_item_code; };
	inline unsigned char GetTimestampCode(){ return thsb_code; };

	inline unsigned char GetPauseCode(){ return pause_code;};
	inline unsigned char GetResumeCode(){ return resume_code;};

	inline unsigned char GetArrayPulserAsic0(){ return asic_pulser_asic_0; };
	inline unsigned char GetArrayPulserChannel0(){ return asic_pulser_ch_0; };
	inline unsigned char GetArrayPulserCode0(){ return asic_pulser_code_0; };
	inline unsigned char GetArrayPulserAsic1(){ return asic_pulser_asic_1; };
	inline unsigned char GetArrayPulserChannel1(){ return asic_pulser_ch_1; };
	inline unsigned char GetArrayPulserCode1(){ return asic_pulser_code_1; };
	inline unsigned char GetArrayPulserThreshold(){ return asic_pulser_thres; };

	inline unsigned char GetCAENPulserModule(){ return caen_pulser_mod; };
	inline unsigned char GetCAENPulserChannel(){ return caen_pulser_ch; };
	inline unsigned char GetCAENPulserCode(){ return caen_pulser_code; };
	
	inline unsigned char GetEBISModule(){ return caen_ebis_mod; };
	inline unsigned char GetEBISChannel(){ return caen_ebis_ch; };
	inline unsigned char GetEBISCode(){ return ebis_code; };

	inline unsigned char GetT1Module(){ return caen_t1_mod; };
	inline unsigned char GetT1Channel(){ return caen_t1_ch; };
	inline unsigned char GetT1Code(){ return t1_code; };

	inline unsigned char GetSCModule(){ return caen_sc_mod; };
	inline unsigned char GetSCChannel(){ return caen_sc_ch; };
	inline unsigned char GetSCCode(){ return sc_code; };

	inline unsigned char GetLaserModule(){ return caen_laser_mod; };
	inline unsigned char GetLaserChannel(){ return caen_laser_ch; };
	inline unsigned char GetLaserCode(){ return laser_code; };


	// Event builder
	inline double GetEventWindow(){ return event_window; };
	inline double GetRecoilHitWindow(){ return recoil_hit_window; }
	inline double GetArrayPNHitWindow(){ return array_pn_hit_window; }
	inline double GetArrayPPHitWindow(){ return array_pp_hit_window; }
	inline double GetArrayNNHitWindow(){ return array_nn_hit_window; }
	inline double GetZeroDegreeHitWindow(){ return zd_hit_window; }
	inline double GetGammaRayHitWindow(){ return gamma_hit_window; }

	
	// Data settings
	inline unsigned int GetBlockSize(){ return block_size; };
	inline bool IsCAENOnly(){ return flag_caen_only; };
	inline bool IsASICOnly(){ return flag_asic_only; };

	
	// Recoil detector
	inline unsigned char GetNumberOfRecoilSectors(){ return n_recoil_sector; };
	inline unsigned char GetNumberOfRecoilLayers(){ return n_recoil_layer; };
	inline unsigned char GetNumberOfRecoilElements(){ return n_recoil_sector * n_recoil_layer; };
	inline unsigned char GetRecoilEnergyLossStart(){ return recoil_eloss_start; };
	inline unsigned char GetRecoilEnergyLossStop(){ return recoil_eloss_stop; };
	inline unsigned char GetRecoilEnergyRestStart(){ return recoil_erest_start; };
	inline unsigned char GetRecoilEnergyRestStop(){ return recoil_erest_stop; };
	inline unsigned char GetRecoilEnergyTotalStart(){ return recoil_etot_start; };
	inline unsigned char GetRecoilEnergyTotalStop(){ return recoil_etot_stop; };
	char GetRecoilSector( unsigned char mod, unsigned char ch );
	char GetRecoilLayer( unsigned char mod, unsigned char ch );
	char GetRecoilModule( unsigned char sec, unsigned char layer );
	char GetRecoilChannel( unsigned char sec, unsigned char layer );
	bool IsRecoil( unsigned char mod, unsigned char ch );
	
	// MWPC
	inline unsigned char GetNumberOfMWPCAxes(){ return n_mwpc_axes; };
	char GetMWPCAxis( unsigned char mod, unsigned char ch );
	char GetMWPCID( unsigned char mod, unsigned char ch );
	bool IsMWPC( unsigned char mod, unsigned char ch );

	// ELUM detector
	inline unsigned char GetNumberOfELUMSectors(){ return n_elum_sector; };
	char GetELUMSector( unsigned char mod, unsigned char ch );
	bool IsELUM( unsigned char mod, unsigned char ch );

	// ZeroDegree detector
	inline unsigned char GetNumberOfZDLayers(){ return n_zd_layer; };
	char GetZDLayer( unsigned char mod, unsigned char ch );
	bool IsZD( unsigned char mod, unsigned char ch );

	// Scintillation detectors
	inline unsigned char GetNumberOfScintArrayDetectors(){ return n_scint_detector; };
	char GetScintArrayDetector( unsigned char mod, unsigned char ch );
	bool IsScintArray( unsigned char mod, unsigned char ch );


private:

	std::string fInputFile;

	// Array settings
	unsigned char n_array_mod;	///< 3 modules make a full array
	unsigned char n_array_asic;	///< 4 p-side + 2 n-side per module
	unsigned char n_array_ch;	///< 128 channels per ASIC
	
	// Array geometry
	unsigned char n_array_row;		///< 4x2 DSSSDs per module, but paired; dE-E for recoil, gas cathodes (13?)
	unsigned char n_array_pstrip;	///< number of p-side strips in each DSSSD
	unsigned char n_array_nstrip;	///< strip number of DSSSD
	unsigned char n_array_side;		///< p-side and n-side


	// CAEN settings
	unsigned char n_caen_mod;
	unsigned char n_caen_ch;
	std::vector<unsigned int> caen_model;
	std::vector<std::vector<unsigned char>> caen_extras;
	
	// Mesytec settings
	unsigned char n_mesy_mod;
	unsigned char n_mesy_ch;

	// Info code settings
	unsigned char extt_code;			///< This is the info code for the external timestamp, 5 before 2019 and 14 after 2019 (This is ISS == 14)
	unsigned char sync_code;			///< Medium significant bits of the timestamp are here
	unsigned char ext_item_code;		///< Medium significant bits of the timestamp are here from the ASIC ADC
	unsigned char thsb_code;			///< Highest significant bits of the timestamp are here
	unsigned char pause_code;      		///< Info code when ISS acquisition has paused due to a full buffer
	unsigned char resume_code;			///< Info code when ISS acquisition has resumed after a pause.
	unsigned char asic_pulser_asic_0;	///< Location of the pulser in the ASIC frontends (asic)
	unsigned char asic_pulser_ch_0;		///< Location of the pulser in the ASIC frontends (channel)
	unsigned char asic_pulser_code_0;	///< Info code when we have a pulser event in InfoData packets from ASICs
	unsigned char asic_pulser_asic_1;	///< Location of the pulser in the ASIC frontends (asic)
	unsigned char asic_pulser_ch_1;		///< Location of the pulser in the ASIC frontends (channel)
	unsigned char asic_pulser_code_1;	///< Info code when we have a pulser event in InfoData packets from ASICs
	unsigned char asic_pulser_thres;	///< Threshold on energy for the pulser event in InfoData packets from ASICs
	unsigned char caen_pulser_mod;		///< Location of the pulser in the CAEN system (module)
	unsigned char caen_pulser_ch;		///< Location of the pulser in the CAEN system (channel) - Daresbury test
	unsigned char caen_pulser_code;		///< Info code when we have a pulser event in InfoData packets from CAEN
	unsigned char caen_ebis_mod;		///< Location of the EBIS signal in the CAEN system (module)
	unsigned char caen_ebis_ch;			///< Location of the EBIS signal in the CAEN system (channel)
	unsigned char ebis_code;			///< Info code when we have an EBIS event in InfoData packets
	unsigned char caen_t1_mod;			///< Location of the T1 signal in the CAEN system (module)
	unsigned char caen_t1_ch;			///< Location of the T1 signal in the CAEN system (channel)
	unsigned char t1_code;				///< Info code when we have a T1 event in InfoData packets
	unsigned char caen_sc_mod;			///< Location of the SuperCycle signal in the CAEN system (module)
	unsigned char caen_sc_ch;			///< Location of the SuperCycle signal in the CAEN system (channel)
	unsigned char sc_code;				///< Info code when we have aSuperCycle event in InfoData packets
	unsigned char caen_laser_mod;		///< Location of the Laser signal in the CAEN system (module)
	unsigned char caen_laser_ch;		///< Location of the Laser signal in the CAEN system (channel)
	unsigned char laser_code;			///< Info code when we have a Laser event in InfoData packets

	
	// Event builder
	double event_window;			///< Event builder time window in ns
	double recoil_hit_window;		///< Time window in ns for correlating recoil E-dE hits
	double array_pn_hit_window;		///< Time window in ns for correlating p-n hits on the array
	double array_pp_hit_window;		///< Time window in ns for correlating p-p hits on the array
	double array_nn_hit_window;		///< Time window in ns for correlating n-n hits on the array
	double zd_hit_window;			///< Time window in ns for correlating ZeroDegree E-dE hits
	double gamma_hit_window;		///< Time window in ns for correlating Gamma-Gamma hits (addback?)

	
	// Data format
	unsigned int block_size;		///< not yet implemented, needs C++ style reading of data files
	bool flag_caen_only;			///< when there is only CAEN data in the file
	bool flag_asic_only;			///< when there is only CAEN data in the file

	
	// Recoil detectors
	unsigned char n_recoil_sector;						///< Number of recoil detector sectors or quadrants; 1 for gas and 4 for Si
	unsigned char n_recoil_layer;						///< Number of recoil detector layers; 13 for gas and 2 for Si
	unsigned char recoil_eloss_start;					///< Start layer for integrating energy loss, 0 for Silicon, about 1 for gas.
	unsigned char recoil_eloss_stop;					///< Stop layer for integrating energy loss, 0 for Silicon, about 1 for gas.
	unsigned char recoil_erest_start;					///< Start layer for integrating energy rest, 1 for Silicon, about 5 for gas.
	unsigned char recoil_erest_stop;					///< Stop layer for integrating energy rest, 1 for Silicon, about 5 for gas.
	unsigned char recoil_etot_start;					///< Start layer for integrating energy total, 0 for Silicon, 0 for gas.
	unsigned char recoil_etot_stop;						///< Stop layer for integrating energy total, 1 for Silicon, about 8 for gas.
	std::vector<std::vector<unsigned char>> recoil_mod;	///< A list of module numbers for each recoil detector sector and layer
	std::vector<std::vector<unsigned char>> recoil_ch;	///< A list of channel numbers for each recoil detector sector and layer
	std::vector<std::vector<char>> recoil_sector;		///< A channel map for the recoil sectors (-1 if not a recoil)
	std::vector<std::vector<char>> recoil_layer;			///< A channel map for the recoil layers (-1 if not a recoil)

	
	// MWPC
	unsigned char n_mwpc_axes;							///< Number of MWPC axes (usually 2: x and y). Two TACs per axis
	std::vector<std::vector<unsigned char>> mwpc_mod;	///< Module number of each TAC input of each axis of the MWPC
	std::vector<std::vector<unsigned char>> mwpc_ch;	///< Channel number of each TAC input of each axis of the MWPC
	std::vector<std::vector<char>> mwpc_axis;			///< A channel map for the MWPC axes (-1 if not an MWPC)
	std::vector<std::vector<char>> mwpc_tac;			///< A channel map for the MWPC TACs (-1 if not an MWPC)

	
	// ELUM detector
	unsigned char n_elum_sector;				///< Number of ELUM detector sectors or quadrants; usually 4, maybe 6 in the future?
	std::vector<unsigned char> elum_mod;			///< A list of module numbers for each ELUM detector sector
	std::vector<unsigned char> elum_ch;			///< A list of channel numbers for each ELUM detector sector
	std::vector<std::vector<char>> elum_sector;	///< A channel map for the ELUM sectors (-1 if not an ELUM)

	
	// ZeroDegree detector
	unsigned char n_zd_layer;					///< Number of ZeroDegree detector layers; always 2, because it's silicon dE-E
	std::vector<unsigned char> zd_mod;			///< A list of module numbers for each ZeroDegree detector layer
	std::vector<unsigned char> zd_ch;			///< A list of channel numbers for each ZeroDegree detector layor
	std::vector<std::vector<char>> zd_layer;	///< A channel map for the ZeroDegree layers (-1 if not a ZeroDegree)

	
	// ELUM detector
	unsigned char n_scint_detector;					///< Number of ScintArray detectors
	std::vector<unsigned char> scint_mod;			///< A list of module numbers for each ScintArray detectors
	std::vector<unsigned char> scint_ch;			///< A list of channel numbers for each ScintArray detectors
	std::vector<std::vector<char>> scint_detector;	///< A channel map for the ScintArray detectors (-1 if not an ScintArray detector)

	
};

#endif
