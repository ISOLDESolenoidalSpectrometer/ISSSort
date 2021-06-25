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

// Common header
#ifndef __COMMON_HH
# include "Common.hh"
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

	EventBuilder();
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

	
	// These things should probably be in the settings file
	long build_window;  /// length of build window in ns
	
	// Some more things that should be in a settings file
	std::vector<unsigned char> array_side;
	std::vector<unsigned char> array_row;
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
	unsigned long long	mytime;		///< absolute timestamp
	float 				myenergy;	///< calibrated energy
	
	// Data variables - Array
	unsigned char		mymod;		///< iss module number
	unsigned char		myside;		///< p-side = 0; n-side = 1
	unsigned char		myrow;		///< 4 wafers along array, 2 dE-E, 13 for gas
	int					mystrip;	///< strip number for DSSSD
	
	// Data variables - Recoil / ELUM / ZeroDegree
	unsigned char		mysector;	///< 4 quadrants of the recoil


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
	std::vector<float>	rid_list;	///< list of recoil ids for RecoilFinder
	std::vector<float>	rsec_list;	///< list of recoil sectors for RecoilFinder

	// Counters
	unsigned int		hit_ctr;
	unsigned long		array_ctr;
	unsigned long		recoil_ctr;
	unsigned long		elum_ctr;
	unsigned long		zd_ctr;
	unsigned long		n_asic_data;
	unsigned long		n_caen_data;
	unsigned long		n_info_data;
	unsigned long long	n_entries;
	unsigned long		n_caen_pulser;
	unsigned long		n_asic_pulser;

	// Histograms
	TH2F *pn_11[common::n_module][common::n_row];
	TH2F *pn_12[common::n_module][common::n_row];
	TH2F *pn_21[common::n_module][common::n_row];
	TH2F *pn_22[common::n_module][common::n_row];
	TH1F *pn_td[common::n_module][common::n_row];

	TProfile *pn_mult[common::n_row][common::n_row];

	TH1F *tdiff;
	TProfile *daq_sync;
	TProfile *caen_freq, *asic_freq, *freq_diff;
	TProfile *ebis_freq, *t1_freq;
	TProfile *pulser_loss;

	
};

#endif

