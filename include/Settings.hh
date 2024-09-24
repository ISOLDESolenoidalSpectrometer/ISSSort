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

	
	// CAEN settings
	inline unsigned char GetNumberOfCAENModules(){ return n_caen_mod; };
	inline unsigned char GetNumberOfCAENChannels(){ return n_caen_ch; };
	inline unsigned int GetCAENModel( unsigned char i ){
		if( i < n_caen_mod )
			return caen_model[i];
		else return 1725;
	};
	
	// Mesytec settings
	inline unsigned char GetNumberOfMesytecModules(){ return n_mesy_mod; };
	inline unsigned char GetNumberOfMesytecChannels(){ return n_mesy_ch; };
	
	// VME totals
	inline unsigned int GetNumberOfVmeCrates(){ return 2; }; // fixed: CAEN + Mesytec
	inline unsigned int GetNumberOfVmeModules(){ return n_caen_mod + n_mesy_mod; };
	inline unsigned int GetMaximumNumberOfVmeModules(){
		if( n_caen_mod > n_mesy_mod ) return n_caen_mod;
		else return n_mesy_mod;
	};
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
	
	inline unsigned char GetMesytecPulserModule(){ return mesy_pulser_mod; };
	inline unsigned char GetMesytecPulserChannel(){ return mesy_pulser_ch; };
	inline unsigned char GetMesytecPulserCode(){ return mesy_pulser_code; };
	
	inline unsigned char GetEBISCrate(){ return vme_ebis_crate; };
	inline unsigned char GetEBISModule(){ return vme_ebis_mod; };
	inline unsigned char GetEBISChannel(){ return vme_ebis_ch; };
	inline unsigned char GetEBISCode(){ return ebis_code; };

	inline unsigned char GetT1Crate(){ return vme_t1_crate; };
	inline unsigned char GetT1Module(){ return vme_t1_mod; };
	inline unsigned char GetT1Channel(){ return vme_t1_ch; };
	inline unsigned char GetT1Code(){ return t1_code; };

	inline unsigned char GetSCCrate(){ return vme_sc_crate; };
	inline unsigned char GetSCModule(){ return vme_sc_mod; };
	inline unsigned char GetSCChannel(){ return vme_sc_ch; };
	inline unsigned char GetSCCode(){ return sc_code; };

	inline unsigned char GetLaserCrate(){ return vme_laser_crate; };
	inline unsigned char GetLaserModule(){ return vme_laser_mod; };
	inline unsigned char GetLaserChannel(){ return vme_laser_ch; };
	inline unsigned char GetLaserCode(){ return laser_code; };


	// Event builder
	inline double GetEventWindow(){ return event_window; };
	inline double GetRecoilHitWindow(){ return recoil_hit_window; }
	inline double GetArrayPNHitWindow(){ return array_pn_hit_window; }
	inline double GetArrayPPHitWindow(){ return array_pp_hit_window; }
	inline double GetArrayNNHitWindow(){ return array_nn_hit_window; }
	inline double GetZeroDegreeHitWindow(){ return zd_hit_window; }
	inline double GetGammaRayHitWindow(){ return gamma_hit_window; }
        inline double GetLumeHitWindow(){ return lume_hit_window; }
  inline double GetCDRSHitWindow(){ return cd_rs_hit_window; }
  inline double GetCDDDHitWindow(){ return cd_dd_hit_window; }


	// Data settings
	inline unsigned int GetBlockSize(){ return block_size; };
	inline bool IsASICOnly(){ return flag_asic_only; };
	inline bool IsCAENOnly(){ return flag_caen_only; };
	inline bool IsMesyOnly(){ return flag_mesy_only; };

	
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
	char GetRecoilSector( unsigned char vme, unsigned char mod, unsigned char ch );
	char GetRecoilLayer( unsigned char vme, unsigned char mod, unsigned char ch );
	char GetRecoilCrate( unsigned char sec, unsigned char layer );
	char GetRecoilModule( unsigned char sec, unsigned char layer );
	char GetRecoilChannel( unsigned char sec, unsigned char layer );
	bool IsRecoil( unsigned char vme, unsigned char mod, unsigned char ch );
	
	// MWPC
	inline unsigned char GetNumberOfMWPCAxes(){ return n_mwpc_axes; };
	char GetMWPCAxis( unsigned char vme, unsigned char mod, unsigned char ch );
	char GetMWPCID( unsigned char vme, unsigned char mod, unsigned char ch );
	bool IsMWPC( unsigned char vme, unsigned char mod, unsigned char ch );

	// ELUM detector
	inline unsigned char GetNumberOfELUMSectors(){ return n_elum_sector; };
	char GetELUMSector( unsigned char vme, unsigned char mod, unsigned char ch );
	bool IsELUM( unsigned char vme, unsigned char mod, unsigned char ch );

	// ZeroDegree detector
	inline unsigned char GetNumberOfZDLayers(){ return n_zd_layer; };
	char GetZDLayer( unsigned char vme, unsigned char mod, unsigned char ch );
	bool IsZD( unsigned char vme, unsigned char mod, unsigned char ch );

	// Scintillation detectors
	inline unsigned char GetNumberOfScintArrayDetectors(){ return n_scint_detector; };
	char GetScintArrayDetector( unsigned char vme, unsigned char mod, unsigned char ch );
	bool IsScintArray( unsigned char vme, unsigned char mod, unsigned char ch );

	// LUME detectors
	inline unsigned char GetNumberOfLUMEDetectors(){ return n_lume; };
	char GetLUMEDetector( unsigned char vme, unsigned char mod, unsigned char ch );
	char GetLUMEType( unsigned char vme, unsigned char mod, unsigned char ch );
	bool IsLUME( unsigned char vme, unsigned char mod, unsigned char ch );

  // CD detectors
  inline unsigned char GetNumberOfCDLayers(){ return n_cd_layer; };
  inline unsigned char GetNumberOfCDSectors(){ return n_cd_sector; };
  inline unsigned char GetNumberOfCDRings(){ return n_cd_ring; };
  char GetCDLayer( unsigned char vme, unsigned char mod, unsigned char ch );
  char GetCDSector( unsigned char vme, unsigned char mod, unsigned char ch );
  char GetCDRing( unsigned char vme, unsigned char mod, unsigned char ch );
  bool IsCD( unsigned char vme, unsigned char mod, unsigned char ch );
  
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


	// CAEN settings
	unsigned char n_caen_mod;
	unsigned char n_caen_ch;
	std::vector<unsigned int> caen_model;
	
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
	unsigned char mesy_pulser_mod;		///< Location of the pulser in the Mesytec system (module)
	unsigned char mesy_pulser_ch;		///< Location of the pulser in the Mesytec system (channel) - Daresbury test
	unsigned char mesy_pulser_code;		///< Info code when we have a pulser event in InfoData packets from CAEN
	unsigned char vme_ebis_crate;		///< Location of the EBIS signal in the VME system (crate: 0 = CAEN; 1 = Mesytec)
	unsigned char vme_ebis_mod;			///< Location of the EBIS signal in the VME system (module)
	unsigned char vme_ebis_ch;			///< Location of the EBIS signal in the VME system (channel)
	unsigned char ebis_code;			///< Info code when we have an EBIS event in InfoData packets
	unsigned char vme_t1_crate;			///< Location of the T1 signal in the VME system (crate: 0 = CAEN; 1 = Mesytec)
	unsigned char vme_t1_mod;			///< Location of the T1 signal in the VME system (module)
	unsigned char vme_t1_ch;			///< Location of the T1 signal in the VME system (channel)
	unsigned char t1_code;				///< Info code when we have a T1 event in InfoData packets
	unsigned char vme_sc_crate;			///< Location of the SuperCycle signal in the VME system (crate: 0 = CAEN; 1 = Mesytec)
	unsigned char vme_sc_mod;			///< Location of the SuperCycle signal in the VME system (module)
	unsigned char vme_sc_ch;			///< Location of the SuperCycle signal in the VME system (channel)
	unsigned char sc_code;				///< Info code when we have a SuperCycle event in InfoData packets
	unsigned char vme_laser_crate;		///< Location of the Laser/RILIS signal in the VME system (crate: 0 = CAEN; 1 = Mesytec)
	unsigned char vme_laser_mod;		///< Location of the Laser/RILIS signal in the VME system (module)
	unsigned char vme_laser_ch;			///< Location of the Laser/RILIS signal in the VME system (channel)
	unsigned char laser_code;			///< Info code when we have a Laser/RILIS  event in InfoData packets

	
	// Event builder
	double event_window;			///< Event builder time window in ns
	double recoil_hit_window;		///< Time window in ns for correlating recoil E-dE hits
	double array_pn_hit_window;		///< Time window in ns for correlating p-n hits on the array
	double array_pp_hit_window;		///< Time window in ns for correlating p-p hits on the array
	double array_nn_hit_window;		///< Time window in ns for correlating n-n hits on the array
	double zd_hit_window;			///< Time window in ns for correlating ZeroDegree E-dE hits
	double gamma_hit_window;		///< Time window in ns for correlating Gamma-Gamma hits (addback?)
	double lume_hit_window;			///< Time window in ns for correlating hits in LUME detectors (be, ne, and fe signals)
  double cd_rs_hit_window;		///< Time window in ns for correlating CD hits in rings and sectors of one detector
  double cd_dd_hit_window;		///< Time window in ns for correlating CD dE-E hits

	
	// Data format
	unsigned int block_size;		///< not yet implemented, needs C++ style reading of data files
	bool flag_asic_only;			///< when there is only CAEN data in the file
	bool flag_caen_only;			///< when there is only CAEN data in the file
	bool flag_mesy_only;			///< when there is only Mesytec data in the file
	

	// Recoil detectors
	unsigned char n_recoil_sector;								///< Number of recoil detector sectors or quadrants; 1 for gas and 4 for Si
	unsigned char n_recoil_layer;								///< Number of recoil detector layers; 13 for gas and 2 for Si
	unsigned char recoil_eloss_start;							///< Start layer for integrating energy loss, 0 for Silicon, about 1 for gas.
	unsigned char recoil_eloss_stop;							///< Stop layer for integrating energy loss, 0 for Silicon, about 1 for gas.
	unsigned char recoil_erest_start;							///< Start layer for integrating energy rest, 1 for Silicon, about 5 for gas.
	unsigned char recoil_erest_stop;							///< Stop layer for integrating energy rest, 1 for Silicon, about 5 for gas.
	unsigned char recoil_etot_start;							///< Start layer for integrating energy total, 0 for Silicon, 0 for gas.
	unsigned char recoil_etot_stop;								///< Stop layer for integrating energy total, 1 for Silicon, about 8 for gas.
	std::vector<std::vector<unsigned char>> recoil_vme;			///< A list of VME crate numbers for each recoil detector sector and layer
	std::vector<std::vector<unsigned char>> recoil_mod;			///< A list of module numbers for each recoil detector sector and layer
	std::vector<std::vector<unsigned char>> recoil_ch;			///< A list of channel numbers for each recoil detector sector and layer
	std::vector<std::vector<std::vector<char>>> recoil_sector;	///< A channel map for the recoil sectors (-1 if not a recoil)
	std::vector<std::vector<std::vector<char>>> recoil_layer;	///< A channel map for the recoil layers (-1 if not a recoil)

	
	// MWPC
	unsigned char n_mwpc_axes;									///< Number of MWPC axes (usually 2: x and y). Two TACs per axis
	std::vector<std::vector<unsigned char>> mwpc_vme;			///< VME crate number  of each TAC input of each axis of the MWPC
	std::vector<std::vector<unsigned char>> mwpc_mod;			///< Module number of each TAC input of each axis of the MWPC
	std::vector<std::vector<unsigned char>> mwpc_ch;			///< Channel number of each TAC input of each axis of the MWPC
	std::vector<std::vector<std::vector<char>>> mwpc_axis;		///< A channel map for the MWPC axes (-1 if not an MWPC)
	std::vector<std::vector<std::vector<char>>> mwpc_tac;		///< A channel map for the MWPC TACs (-1 if not an MWPC)

	
	// ELUM detector
	unsigned char n_elum_sector;								///< Number of ELUM detector sectors or quadrants; usually 4, maybe 6 in the future?
	std::vector<unsigned char> elum_vme;						///< A list of VME crate numbers for each ELUM detector sector
	std::vector<unsigned char> elum_mod;						///< A list of module numbers for each ELUM detector sector
	std::vector<unsigned char> elum_ch;							///< A list of channel numbers for each ELUM detector sector
	std::vector<std::vector<std::vector<char>>> elum_sector;	///< A channel map for the ELUM sectors (-1 if not an ELUM)

	
	// ZeroDegree detector
	unsigned char n_zd_layer;									///< Number of ZeroDegree detector layers; always 2, because it's silicon dE-E
	std::vector<unsigned char> zd_vme;							///< A list of VME crate numbers for each ZeroDegree detector layer
	std::vector<unsigned char> zd_mod;							///< A list of module numbers for each ZeroDegree detector layer
	std::vector<unsigned char> zd_ch;							///< A list of channel numbers for each ZeroDegree detector layer
	std::vector<std::vector<std::vector<char>>> zd_layer;		///< A channel map for the ZeroDegree layers (-1 if not a ZeroDegree)

	
	// ELUM detector
	unsigned char n_scint_detector;								///< Number of ScintArray detectors
	std::vector<unsigned char> scint_vme;						///< A list of VME crate numbers for each ScintArray detectors
	std::vector<unsigned char> scint_mod;						///< A list of module numbers for each ScintArray detectors
	std::vector<unsigned char> scint_ch;						///< A list of channel numbers for each ScintArray detectors
	std::vector<std::vector<std::vector<char>>> scint_detector;	///< A channel map for the ScintArray detectors (-1 if not an ScintArray detector)

	
	// LUME detectors, each of them has 3 outputs - total energy (read out from the back side), near (read out from one edge), and far (from the other edge), total energy gives energy information, near and far side give position of hit
	unsigned char n_lume;										///< Number of LUME detectors, should be 4
	std::vector<std::vector<unsigned char>> lume_vme;			///< A list of VME crate numbers for the LUME detector and type
	std::vector<std::vector<unsigned char>> lume_mod;			///< A list of module numbers for the LUME detector and type
	std::vector<std::vector<unsigned char>> lume_ch;			///< A list of channel numbers for the LUME detector and type
	std::vector<std::vector<std::vector<char>>> lume_detector;	///< A channel map for the LUME detector IDs
	std::vector<std::vector<std::vector<char>>> lume_type;		///< A channel map for the LUME detector types (0= be, 1= ne, 2= fe)
	std::vector<std::string> lume_type_list;					///< A list of string types for the LUME signals

  // CD detectors
  unsigned char n_cd_layer; ///< Number of CD detector layers (layer 0 is dE, layer 1 is E)
  unsigned char n_cd_sector;								///< Number of CD detector sectors for each detector
  unsigned char n_cd_ring;								///< Number of CD detector rings for each detector
  std::vector<std::vector<unsigned char>> cd_ring_vme; ///< A list of VME crate numbers for each CD detector layer and each ring
  std::vector<std::vector<unsigned char>> cd_sector_vme; ///< A list of VME crate numbers for each CD detector layer and each sector
  std::vector<std::vector<unsigned char>> cd_ring_mod;							///< A list of module numbers for each CD detector layer and each ring
  std::vector<std::vector<unsigned char>> cd_sector_mod;							///< A list of module numbers for each CD detector layer and each sector
  std::vector<std::vector<unsigned char>> cd_ring_ch;							///< A list of channel numbers for each CD detector layer and each ring
  std::vector<std::vector<unsigned char>> cd_sector_ch;							///< A list of channel numbers for each CD detector layer and each sector
  std::vector<std::vector<std::vector<char>>> cd_layer;		///< A channel map for the CD layers (-1 if not a CD)
  std::vector<std::vector<std::vector<char>>> cd_sector; ///< A channel map for the CD sectors (-1 if not a CD)
  std::vector<std::vector<std::vector<char>>> cd_ring; ///< A channel map for the CD rings (-1 if not a CD)
  
};

#endif
