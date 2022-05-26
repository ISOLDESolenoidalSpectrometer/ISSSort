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
#include <TGProgressBar.h>
#include <TSystem.h>


// Settings header
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif

// Calibration header
#ifndef __CALIBRATION_HH
# include "Calibration.hh"
#endif

// Data packets header
#ifndef __DATAPACKETS_hh
# include "DataPackets.hh"
#endif

// ISS Events tree
#ifndef __ISSEVTS_HH
# include "ISSEvts.hh"
#endif

// Reaction header
#ifndef __REACTION_HH
# include "Reaction.hh"
#endif

// Histogram header
#ifndef __HISTOGRAMMER_hh
# include "Histogrammer.hh"
#endif



class ISSEventBuilder {
	
public:

	ISSEventBuilder( ISSSettings *myset );
	virtual ~ISSEventBuilder(){};

	void	SetInputFile( std::string input_file_name );
	void	SetInputTree( TTree* user_tree );
	void	SetOutput( std::string output_file_name );
	void	StartFile();	///< called for every file
	void	Initialise();	///< called for every event
	void	MakeEventHists();
	
	inline void AddCalibration( ISSCalibration *mycal ){
		cal = mycal;
		overwrite_cal = true;
	};
	
	unsigned long	BuildEvents( unsigned long start_build = 0 );

	// Resolve multiplicities etc
	void ArrayFinder();
	void RecoilFinder();
	void MwpcFinder();
	void ElumFinder();
	void ZeroDegreeFinder();
	//void GammaFinder(); // in the future :-)
		
	inline TFile* GetFile(){ return output_file; };
	inline TTree* GetTree(){ return output_tree; };
	inline void CloseOutput(){
		output_file->Close();
	};
	void CleanHists();

	inline void AddProgressBar( std::shared_ptr<TGProgressBar> myprog ){
		prog = myprog;
		_prog_ = true;
	};


private:
	
	/// Input tree
	TFile *input_file;
	TTree *input_tree;
	ISSDataPackets *in_data = 0;
	ISSAsicData *asic_data;
	ISSCaenData *caen_data;
	ISSInfoData *info_data;

	/// Outputs
	TFile *output_file;
	TTree *output_tree;
	ISSEvts *write_evts;
	ISSArrayEvt *array_evt;
	ISSArrayPEvt *arrayp_evt;
	ISSRecoilEvt *recoil_evt;
	ISSMwpcEvt *mwpc_evt;
	ISSElumEvt *elum_evt;
	ISSZeroDegreeEvt *zd_evt;
	
	// Do calibration
	ISSCalibration *cal;
	bool overwrite_cal;
	
	// Settings file
	ISSSettings *set;
	
	// Progress bar
	bool _prog_;
	std::shared_ptr<TGProgressBar> prog;

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
	bool flag_caen_pulser;
	std::vector<bool> flag_pause, flag_resume;
	bool noise_flag, event_open;

	// Time variables
	long		 		time_diff;
	unsigned long long	time_prev, time_min, time_max, time_first;
	unsigned long long  ebis_time, t1_time, ebis_prev, t1_prev;
	unsigned long long	caen_time, caen_prev;
	double asic_hz, fpga_hz, caen_hz, ebis_hz, t1_hz;
	double fpga_tdiff, asic_tdiff;
	std::vector<unsigned long long> fpga_time, fpga_prev;
	std::vector<unsigned long long> asic_time, asic_prev;
	std::vector<unsigned long long> pause_time, resume_time, asic_dead_time;
	std::vector<unsigned long long> asic_time_start, asic_time_stop;
	std::vector<unsigned long long> caen_time_start, caen_time_stop;

	// Data variables - generic
	unsigned char		mymod;		///< module number
	unsigned char		mych;		///< channel number
	unsigned long long	mytime;		///< absolute timestamp
	float 				myenergy;	///< calibrated energy
	int 				mywalk;		///< time walk correction
	bool				mythres;	///< above threshold?

	// Data variables - Array
	unsigned char		myasic;		///< ASIC number
	unsigned char		myside;		///< p-side = 0; n-side = 1
	unsigned char		myrow;		///< 4 wafers along array, 2 dE-E, 13 for gas
	int					mystrip;	///< strip number for DSSSD
	
	// Data variables - Recoil / ELUM / ZeroDegree / MWPC
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
	
	// MWPC variables
	std::vector<unsigned short>	mwpctac_list;	///< TAC time from the MWPC
	std::vector<long>			mwpctd_list;	///< list of ELUM time differences for ELUMFinder
	std::vector<int>			mwpcaxis_list;	///< list of axis IDs for the MWPC
	std::vector<int>			mwpcid_list;	///< list of TAC IDs for the MWPC

	// ELUM variables
	std::vector<float>	een_list;	///< list of ELUM energies for ELUMFinder
	std::vector<long>	etd_list;	///< list of ELUM time differences for ELUMFinder
	std::vector<int>	esec_list;	///< list of ELUM sectors for ELUMFinder

	// ZeroDegree variables
	std::vector<float>	zen_list;	///< list of ZeroDegree energies for ELUMFinder
	std::vector<long>	ztd_list;	///< list of ZeroDegree time differences for ELUMFinder
	std::vector<int>	zid_list;	///< list of ZeroDegree IDs/layers for ELUMFinder

	// Counters
	unsigned int		hit_ctr, array_ctr, arrayp_ctr, recoil_ctr, mwpc_ctr, elum_ctr, zd_ctr;
	unsigned long		n_asic_data, n_caen_data, n_info_data;
	unsigned long long	n_entries;
	unsigned long		n_caen_pulser;
	unsigned long		n_ebis, n_t1;
	std::vector<unsigned long>	n_fpga_pulser;
	std::vector<unsigned long>	n_asic_pause, n_asic_resume, n_asic_pulser;

	// Array Histograms
	std::vector<std::vector<TH2F*>> pn_11;
	std::vector<std::vector<TH2F*>> pn_12;
	std::vector<std::vector<TH2F*>> pn_21;
	std::vector<std::vector<TH2F*>> pn_22;
	std::vector<std::vector<TH2F*>> pn_ab;
	std::vector<std::vector<TH2F*>> pn_nab;
	std::vector<std::vector<TH2F*>> pn_pab;
	std::vector<std::vector<TH2F*>> pn_max;
	std::vector<std::vector<TH1F*>> pn_td;
	std::vector<std::vector<TH2F*>> pn_mult;

	// Timing histograms
	TH1F *tdiff, *tdiff_clean;
	TProfile *caen_freq, *ebis_freq, *t1_freq;
	std::vector<TH1F*> fpga_td, asic_td;
	std::vector<TProfile*> fpga_pulser_loss, fpga_freq_diff;
	std::vector<TProfile*> fpga_freq, fpga_sync;
	std::vector<TProfile*> asic_pulser_loss, asic_freq_diff;
	std::vector<TProfile*> asic_freq, asic_sync;

	// Recoil histograms
	std::vector<TH2F*> recoil_EdE;
	std::vector<TH2F*> recoil_dEsum;

	// MWPC histograms
	std::vector<std::vector<TH1F*>> mwpc_tac_axis;
	std::vector<TH1F*> mwpc_hit_axis;
	TH2F *mwpc_pos;

	// ELUM histograms
	TH2F *elum;
	
	// ZeroDegree histograms
	TH2F *zd;
	
};

#endif

