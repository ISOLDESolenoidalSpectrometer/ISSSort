#ifndef _EventBuilder_hh
#define _EventBuilder_hh

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <TFile.h>
#include <TTree.h>
#include <TMath.h>
#include <TChain.h>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>
#include <TVector2.h>
#include <TVector3.h>

// Settings header
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif

// Calibration header
#ifndef __CALIBRATION_HH
# include "Calibration.hh"
#endif

// ISS Events tree
#ifndef __ISSEVTS_HH
# include "ISSEvts.hh"
#endif

// Data packets header
#ifndef _DataPackets_hh
# include "DataPackets.hh"
#endif


class EventBuilder {
	
public:

	EventBuilder( Settings *myset );
	virtual ~EventBuilder();

	void	SetInputFile( std::vector<std::string> input_file_names );
	void	SetInputTree( TTree* user_tree );
	void	SetOutput( std::string output_file_name );
	void	Initialise();
	void	MakeEventHists();
	
	unsigned long	BuildEvents( unsigned long start_build = 0 );

	// Resolve multiplicities etc
	void ArrayFinder();
	void RecoilFinder();
	void ElumFinder();
	void ZeroDegreeFinder();
	//void GammaFinder(); // in the future :-)
	
	// Geometry functions
	float GetZ( int layer, int strip );
	TVector2 GetPhiXY( int sector, int strip );
	
	inline TFile* GetFile(){ return output_file; };
	inline TTree* GetTree(){ return output_tree; };
	inline void CloseOutput(){
		output_file->Close();
	};


private:
	
	/// Input tree
	TChain *input_tree;
	DataPackets *in_data;
	AsicData *asic_data;
	CaenData *caen_data;
	InfoData *info_data;

	/// Outputs
	TFile *output_file;
	TTree *output_tree;
	ISSEvts *write_evts;
	ArrayEvt *array_evt;
	RecoilEvt *recoil_evt;
	ElumEvt *elum_evt;
	ZeroDegreeEvt *zd_evt;
	
	// Settings file
	Settings *set;

	
	// These things should probably be in the settings file
	long build_window;  /// length of build window in ns
	
	// Some more things that should be in a settings file
	std::vector<unsigned char> asic_side;
	std::vector<unsigned char> asic_row;
	std::vector<std::vector<unsigned char>> array_row;
	std::vector<int> array_pid;
	std::vector<int> array_nid;

	// Flags
	bool flag_close_event;
	bool flag_asic_pulser;
	bool flag_caen_pulser;

	// Time variables
	long		 		time_diff;
	unsigned long long	time_prev, time_min, time_max, time_first;
	unsigned long long  ebis_time, t1_time, ebis_prev, t1_prev;
	unsigned long long	asic_time, caen_time, asic_prev, caen_prev;
	double asic_hz, caen_hz, ebis_hz, t1_hz, daq_sync_diff;

	// Data variables - generic
	unsigned char		mymod;		///< module number
	unsigned char		mych;		///< channel number
	unsigned long long	mytime;		///< absolute timestamp
	float 				myenergy;	///< calibrated energy
	
	// Data variables - Array
	unsigned char		myside;		///< p-side = 0; n-side = 1
	unsigned char		myrow;		///< 4 wafers along array, 2 dE-E, 13 for gas
	int					mystrip;	///< strip number for DSSSD
	
	// Data variables - Recoil / ELUM / ZeroDegree
	unsigned char		mysector;	///< 4 quadrants of the recoil, for example
	unsigned char		mylayer;	///< 2 layers for the dE-E, for example


	// Array variables
	std::vector<float>		pen_list;	///< list of p-side energies for ParticleFinder
	std::vector<float>		nen_list;	///< list of n-side energies for ParticleFinder
	std::vector<long>		ptd_list;	///< list of p-side time differences for ParticleFinder
	std::vector<long>		ntd_list;	///< list of n-side time differences for ParticleFinder
	std::vector<int>		pid_list;	///< list of p-side strip ids
	std::vector<int>		nid_list;	///< list of n-side strip ids
	std::vector<int>		pmod_list;	///< list of p-side modules numbers
	std::vector<int>		nmod_list;	///< list of n-side modules numbers
	std::vector<int>		prow_list;	///< list of p-side row numbers
	std::vector<int>		nrow_list;	///< list of n-side row numbers

	// Recoil variables
	std::vector<float>	ren_list;	///< list of recoil energies for RecoilFinder
	std::vector<long>	rtd_list;	///< list of recoil time differences for RecoilFinder
	std::vector<int>	rid_list;	///< list of recoil IDs/layers for RecoilFinder
	std::vector<int>	rsec_list;	///< list of recoil sectors for RecoilFinder
	
	// ELUM variables
	std::vector<float>	een_list;	///< list of ELUM energies for ELUMFinder
	std::vector<long>	etd_list;	///< list of ELUM time differences for ELUMFinder
	std::vector<int>	esec_list;	///< list of ELUM sectors for ELUMFinder

	// ZeroDegree variables
	std::vector<float>	zen_list;	///< list of ZeroDegree energies for ELUMFinder
	std::vector<long>	ztd_list;	///< list of ZeroDegree time differences for ELUMFinder
	std::vector<int>	zid_list;	///< list of ZeroDegree IDs/layers for ELUMFinder

	// Counters
	unsigned int		hit_ctr, array_ctr, recoil_ctr, elum_ctr, zd_ctr;
	unsigned long		n_asic_data, n_caen_data, n_info_data;
	unsigned long long	n_entries;
	unsigned long		n_caen_pulser, n_asic_pulser;
	unsigned long		n_ebis, n_t1;

	// Histograms
	std::vector<std::vector<TH2F*>> pn_11;
	std::vector<std::vector<TH2F*>> pn_12;
	std::vector<std::vector<TH2F*>> pn_21;
	std::vector<std::vector<TH2F*>> pn_22;
	std::vector<std::vector<TH1F*>> pn_td;

	std::vector<std::vector<TProfile*>> pn_mult;

	TH1F *tdiff;
	TProfile *daq_sync;
	TProfile *caen_freq, *asic_freq, *freq_diff;
	TProfile *ebis_freq, *t1_freq;
	TProfile *pulser_loss;

	
};

#endif

