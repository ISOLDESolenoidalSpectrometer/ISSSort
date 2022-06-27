#ifndef __SETTINGS_HH
#define __SETTINGS_HH

#include <iostream>
#include <fstream>
#include <string>

#include "TSystem.h"
#include "TEnv.h"

/// A class to read in the settings file in ROOT's TConfig format.
/// This has the number of modules, channels and things
/// It also defines which detectors are which

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
	inline unsigned int GetNumberOfCAENModules(){ return n_caen_mod; };
	inline unsigned int GetNumberOfCAENChannels(){ return n_caen_ch; };
	
	// Info settings
	inline unsigned int GetExternalTriggerCode(){ return extt_code; };
	inline unsigned int GetSyncCode(){ return sync_code; };
	inline unsigned int GetExtItemCode(){ return ext_item_code; };
	inline unsigned int GetTimestampCode(){ return thsb_code; };

	inline unsigned int GetPauseCode(){ return pause_code;};
	inline unsigned int GetResumeCode(){ return resume_code;};

	inline unsigned int GetArrayPulserAsic(){ return asic_pulser_asic; };
	inline unsigned int GetArrayPulserChannel(){ return asic_pulser_ch; };
	inline unsigned int GetArrayPulserCode(){ return asic_pulser_code; };
	inline unsigned int GetArrayPulserThreshold(){ return asic_pulser_thres; };

	inline unsigned int GetCAENPulserModule(){ return caen_pulser_mod; };
	inline unsigned int GetCAENPulserChannel(){ return caen_pulser_ch; };
	inline unsigned int GetCAENPulserCode(){ return caen_pulser_code; };
	
	inline unsigned int GetEBISModule(){ return caen_ebis_mod; };
	inline unsigned int GetEBISChannel(){ return caen_ebis_ch; };
	inline unsigned int GetEBISCode(){ return ebis_code; };

	inline unsigned int GetT1Module(){ return caen_t1_mod; };
	inline unsigned int GetT1Channel(){ return caen_t1_ch; };
	inline unsigned int GetT1Code(){ return t1_code; };


	// Event builder
	inline double GetEventWindow(){ return event_window; };
	
	
	// Data settings
	inline unsigned int GetBlockSize(){ return block_size; };
	inline unsigned int IsCAENOnly(){ return flag_caen_only; };
	inline unsigned int IsASICOnly(){ return flag_asic_only; };

	
	// Recoil detector
	inline unsigned int GetNumberOfRecoilSectors(){ return n_recoil_sector; };
	inline unsigned int GetNumberOfRecoilLayers(){ return n_recoil_layer; };
	inline unsigned int GetNumberOfRecoilElements(){ return n_recoil_sector * n_recoil_layer; };
	inline unsigned int GetRecoilEnergyLossDepth(){ return recoil_eloss_depth; };
	int GetRecoilSector( unsigned int mod, unsigned int ch );
	int GetRecoilLayer( unsigned int mod, unsigned int ch );
	bool IsRecoil( unsigned int mod, unsigned int ch );
	
	// MWPC
	inline unsigned int GetNumberOfMWPCAxes(){ return n_mwpc_axes; };
	int GetMWPCAxis( unsigned int mod, unsigned int ch );
	int GetMWPCID( unsigned int mod, unsigned int ch );
	bool IsMWPC( unsigned int mod, unsigned int ch );

	// ELUM detector
	inline unsigned int GetNumberOfELUMSectors(){ return n_elum_sector; };
	int GetELUMSector( unsigned int mod, unsigned int ch );
	bool IsELUM( unsigned int mod, unsigned int ch );

	// ZeroDegree detector
	inline unsigned int GetNumberOfZDLayers(){ return n_zd_layer; };
	int GetZDLayer( unsigned int mod, unsigned int ch );
	bool IsZD( unsigned int mod, unsigned int ch );


private:

	std::string fInputFile;

	// Array settings
	unsigned int n_array_mod;	///< 3 modules make a full array
	unsigned int n_array_asic;	///< 4 p-side + 2 n-side per module
	unsigned int n_array_ch;	///< 128 channels per ASIC
	
	// Array geometry
	unsigned int n_array_row;		///< 4x2 DSSSDs per module, but paired; dE-E for recoil, gas cathodes (13?)
	unsigned int n_array_pstrip;	///< number of p-side strips in each DSSSD
	unsigned int n_array_nstrip;	///< strip number of DSSSD
	unsigned int n_array_side;		///< p-side and n-side


	// CAEN settings
	unsigned int n_caen_mod;
	unsigned int n_caen_ch;
	
	
	// Info code settings
	unsigned int extt_code;			///< This is the info code for the external timestamp, 5 before 2019 and 14 after 2019 (This is ISS == 14)
	unsigned int sync_code;			///< Medium significant bits of the timestamp are here
	unsigned int ext_item_code;		///< Medium significant bits of the timestamp are here from the ASIC ADC
	unsigned int thsb_code;			///< Highest significant bits of the timestamp are here
	unsigned int pause_code;        ///< Info code when ISS acquisition has paused due to a full buffer
	unsigned int resume_code;       ///< Info code when ISS acquisition has resumed after a pause.
	unsigned int asic_pulser_asic;	///< Location of the pulser in the ASIC frontends (asic)
	unsigned int asic_pulser_ch;	///< Location of the pulser in the ASIC frontends (channel)
	unsigned int asic_pulser_code;	///< Info code when we have a pulser event in InfoData packets from ASICs
	unsigned int asic_pulser_thres;	///< Threshold on energy for the pulser event in InfoData packets from ASICs
	unsigned int caen_pulser_mod;	///< Location of the pulser in the CAEN system (module)
	unsigned int caen_pulser_ch;	///< Location of the pulser in the CAEN system (channel) - Daresbury test
	unsigned int caen_pulser_code;	///< Info code when we have a pulser event in InfoData packets from CAEN
	unsigned int caen_ebis_mod;		///< Location of the EBIS signal in the CAEN system (module)
	unsigned int caen_ebis_ch;		///< Location of the EBIS signal in the CAEN system (channel)
	unsigned int ebis_code;			///< Info code when we have an EBIS event in InfoData packets
	unsigned int caen_t1_mod;		///< Location of the T1 signal in the CAEN system (module)
	unsigned int caen_t1_ch;		///< Location of the T1 signal in the CAEN system (channel)
	unsigned int t1_code;			///< Info code when we have a T1 event in InfoData packets
	
	
	// Event builder
	double event_window;			///< Event builder time window in ns
	
	// Data format
	unsigned int block_size;		///< not yet implemented, needs C++ style reading of data files
	bool flag_caen_only;			///< when there is only CAEN data in the file
	bool flag_asic_only;			///< when there is only CAEN data in the file

	
	// Recoil detectors
	unsigned int n_recoil_sector;						///< Number of recoil detector sectors or quadrants; 1 for gas and 4 for Si
	unsigned int n_recoil_layer;						///< Number of recoil detector layers; 13 for gas and 2 for Si
	unsigned int recoil_eloss_depth;					///< Number of layers summed for energy loss, 1 for Silicon, about 5 for gas.
	std::vector<std::vector<unsigned int>> recoil_mod;	///< A list of module numbers for each recoil detector sector and layer
	std::vector<std::vector<unsigned int>> recoil_ch;	///< A list of channel numbers for each recoil detector sector and layer
	std::vector<std::vector<int>> recoil_sector;		///< A channel map for the recoil sectors (-1 if not a recoil)
	std::vector<std::vector<int>> recoil_layer;			///< A channel map for the recoil layers (-1 if not a recoil)

	
	// MWPC
	unsigned int n_mwpc_axes;							///< Number of MWPC axes (usually 2: x and y). Two TACs per axis
	std::vector<std::vector<unsigned int>> mwpc_mod;	///< Module number of each TAC input of each axis of the MWPC
	std::vector<std::vector<unsigned int>> mwpc_ch;		///< Channel number of each TAC input of each axis of the MWPC
	std::vector<std::vector<int>> mwpc_axis;			///< A channel map for the MWPC axes (-1 if not an MWPC)
	std::vector<std::vector<int>> mwpc_tac;				///< A channel map for the MWPC TACs (-1 if not an MWPC)

	
	// ELUM detector
	unsigned int n_elum_sector;					///< Number of ELUM detector sectors or quadrants; usually 4, maybe 6 in the future?
	std::vector<unsigned int> elum_mod;			///< A list of module numbers for each ELUM detector sector
	std::vector<unsigned int> elum_ch;			///< A list of channel numbers for each ELUM detector sector
	std::vector<std::vector<int>> elum_sector;	///< A channel map for the ELUM sectors (-1 if not an ELUM)

	
	// ZeroDegree detector
	unsigned int n_zd_layer;				///< Number of ZeroDegree detector layers; always 2, because it's silicon dE-E
	std::vector<unsigned int> zd_mod;		///< A list of module numbers for each ZeroDegree detector layer
	std::vector<unsigned int> zd_ch;		///< A list of channel numbers for each ZeroDegree detector layor
	std::vector<std::vector<int>> zd_layer;	///< A channel map for the ZeroDegree layers (-1 if not a ZeroDegree)

	
};

#endif
