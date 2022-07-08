#ifndef _EventBuilder_hh
#define _EventBuilder_hh

#include <fstream>
#include <iostream>
#include <sstream>
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

///
/// The ISSEventBuilder Class takes a list of time-sorted events from all of the detectors, and packages them up into a series of physics events. 
/// TODO Describe some of the logic for the class here.
///
/// The constructor for this class requires an ISSSettings object, which allows it to use parameters defined in the ``settings.dat'' file. This includes:
/// - Settings which encode the wiring of the detectors e.g. the number of CAEN modules used.
/// - The size of the event window used to combine events
///
/// This size of the event window is crucial for determining which signals belong to which events. The default parameter for this is 3 microseconds *which is currently a blind choice that seems to work*. Perhaps you, dear reader, can come up with a more rigorous reason!

class ISSEventBuilder {
	
public:
	
	ISSEventBuilder( ISSSettings *myset ); ///< Constructor
	/// Destructor (currently empty)
	virtual ~ISSEventBuilder(){};

	void	SetInputFile( std::string input_file_name ); ///< Function to set the input file from which events are built
	void	SetInputTree( TTree* user_tree ); ///< Grabs the input tree from the input file defined in ISSEventBuilder::SetInputFile
	void	SetOutput( std::string output_file_name ); ///< TODO Brief description.
	void	StartFile();	///< Called for every file
	void	Initialise();	///< Called for every event
	void	MakeEventHists(); ///< Creates histograms for events that occur
	
	/// Adds the calibration from the external calibration file to the class
	/// \param[in] mycal The ISSCalibration object which is constructed by the ISSCalibration constructor used in iss_sort.cc
	inline void AddCalibration( ISSCalibration *mycal ){
		cal = mycal;
		overwrite_cal = true;
	};
	
	unsigned long	BuildEvents( unsigned long start_build = 0 ); ///< The heart of this class

	// Resolve multiplicities etc
	void ArrayFinder(); ///< TODO Brief description.
	void RecoilFinder(); ///< TODO Brief description.
	void MwpcFinder(); ///< TODO Brief description.
	void ElumFinder(); ///< TODO Brief description.
	void ZeroDegreeFinder(); ///< TODO Brief description.
	//void GammaFinder(); // in the future :-)
		
	inline TFile* GetFile(){ return output_file; }; ///< TODO Brief description.
	inline TTree* GetTree(){ return output_tree; }; ///< TODO Brief description.
	
	inline void CloseOutput(){
		output_file->Close();
		log_file.close(); //?? to close or not to close?
	}; ///< TODO Brief description.
	void CleanHists(); ///< TODO Brief description.

	inline void AddProgressBar( std::shared_ptr<TGProgressBar> myprog ){
		prog = myprog;
		_prog_ = true;
	}; ///< TODO Brief description.


private:
	
	/// Input tree
	TFile *input_file; ///< Pointer to the time-sorted input ROOT file
	TTree *input_tree; ///< Pointer to the TTree in the input file
	ISSDataPackets *in_data = 0; ///< Pointer to the TBranch containing the data in the time-sorted input ROOT file
	ISSAsicData *asic_data; ///< Pointer to a given entry in the tree of some data from the ASICs
	ISSCaenData *caen_data; ///< Pointer to a given entry in the tree of some data from the CAEN
	ISSInfoData *info_data; ///< Pointer to a given entry in the tree of the "info" datatype

	/// Outputs
	TFile *output_file; ///< Pointer to the output ROOT file containing events
	TTree *output_tree; ///< Pointer to the output ROOT tree containing events
	ISSEvts *write_evts; ///< Container for storing hits on all detectors in order to construct events
	ISSArrayEvt *array_evt; ///< Container for storing hits on the array
	ISSArrayPEvt *arrayp_evt; ///< Container for storing hits on the array that are only on the p-side detectors
	ISSRecoilEvt *recoil_evt; ///< Container for storing hits on the recoil detectors
	ISSMwpcEvt *mwpc_evt; ///< Container for storing hits on the MWPCs
	ISSElumEvt *elum_evt; ///< Container for storing hits on the luminosity detector
	ISSZeroDegreeEvt *zd_evt; ///< Container for storing hits on the zero-degree detector
	
	// Do calibration
	ISSCalibration *cal; ///< Pointer to an ISSCalibration object, used for accessing gain-matching parameters and thresholds
	bool overwrite_cal; ///< Boolean determining whether an energy calibration should be used (true) or not (false). Set in the ISSEventBuilder::AddCalibration function
	
	// Settings file
	ISSSettings *set; ///< Pointer to the settings object. Assigned in constructor
	
	// Progress bar
	bool _prog_; ///< TODO Brief description.
	std::shared_ptr<TGProgressBar> prog; ///< TODO Brief description.

	// Log file
	std::ofstream log_file;
	
	// Flag to know we've opened a file on disk
	bool flag_input_file;

	// These things should probably be in the settings file
	long build_window;  ///< Length of build window in ns
	
	// Some more things that should be in a settings file
	std::vector<unsigned char> asic_side; ///< Vector containing 0 for p-side and 1 for n-side where the index is the asic number
	std::vector<unsigned char> asic_row; ///< Vector containing the smallest row number for a given p/n-side asic where the index is the asic number
	std::vector<std::vector<unsigned char>> array_row; ///< Gives the row of the array for each channel (accessed via asic number and channel number on strip). Unused channels have their value as 0
	std::vector<std::vector<int>> array_pid; ///< Gives each p-side strip on the array a unique number for identification (accessed via asic number and channel number on strip)
	std::vector<std::vector<int>> array_nid; ///< Gives each n-side strip on the array a number for identification (accessed via asic number and channel number on strip)

	// Flags
	bool flag_close_event; ///< TODO Brief description.
	bool flag_caen_pulser; ///< TODO Brief description.
	std::vector<bool> flag_pause, flag_resume; ///< TODO Brief description.
	bool event_open; ///< TODO Brief description.

	// Time variables
	long		 		time_diff;	///< TODO Brief description.
	unsigned long long	time_prev;	///< TODO Brief description.
	unsigned long long	time_min;	///< TODO Brief description.
	unsigned long long	time_max;	///< TODO Brief description.
	unsigned long long	time_first;	///< TODO Brief description.
	unsigned long long  ebis_time;	///< TODO Brief description.
	unsigned long long  t1_time;	///< TODO Brief description.
	unsigned long long  ebis_prev;	///< TODO Brief description.
	unsigned long long  t1_prev;	///< TODO Brief description.
	unsigned long long	caen_time;	///< TODO Brief description.
	unsigned long long	caen_prev;	///< TODO Brief description.
	double asic_hz;		///< The frequency of asic hits in Hz (i.e. inverse of time difference between current and last event)
	double fpga_hz;		///< The frequency of ISS time pulses in FPGAs in Hz (i.e. inverse of time difference between current and last event)
	double caen_hz;		///< The frequency of caen hits in Hz (i.e. inverse of time difference between current and last event)
	double ebis_hz;		///< The frequency of ebis pulses in Hz (i.e. inverse of time difference between current and last event)
	double t1_hz;		///< TODO Brief description.
	double fpga_tdiff;	///< TODO Brief description.
	double asic_tdiff;	///< TODO Brief description.
	std::vector<unsigned long long> fpga_time, fpga_prev; ///< TODO Brief description.
	std::vector<unsigned long long> asic_time, asic_prev; ///< TODO Brief description.
	std::vector<unsigned long long> pause_time, resume_time, asic_dead_time; ///< TODO Brief description.
	std::vector<unsigned long long> asic_time_start;	///< Holds the time of the first hit on each asic in the input time-sorted tree (index denotes asic module)
	std::vector<unsigned long long> asic_time_stop;		///< Holds the time of the last hit on each asic in the input time-sorted tree (index denotes asic module)
	std::vector<unsigned long long> caen_time_start;	///< Holds the time of the first hit on each caen in the input time-sorted tree (index denotes caen module)
	std::vector<unsigned long long> caen_time_stop;		///< Holds the time of the last hit on each asic in the input time-sorted tree (index denotes caen module)

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
	unsigned int		hit_ctr;		///< Counts the number of hits that make up an event
	unsigned int		array_ctr;		///< TODO
	unsigned int		arrayp_ctr;		///< TODO
	unsigned int		recoil_ctr;		///< TODO
	unsigned int		mwpc_ctr;		///< TODO
	unsigned int		elum_ctr;		///< TODO
	unsigned int		zd_ctr;			///< TODO
	unsigned long		n_asic_data;	///< Counter for the number of asic data packets in a file
	unsigned long		n_caen_data;	///< Counter for number of caen data packets in a file
	unsigned long		n_info_data; 	///< Counter for number of info data packets in a file
	unsigned long long	n_entries; 		///< Number of entries in the time-sorted data input tree
	unsigned long		n_caen_pulser;	///< Number of caen pulser hits in the time-sorted data input tree
	unsigned long		n_ebis;			///< Number of ebis pulses in the time-sorted data input tree
	unsigned long		n_t1; 			///< Number of t1 pulses in the time-sorted data input tree
	std::vector<unsigned long>	n_fpga_pulser;	///< Number of fpga pulses in the time-sorted data input tree (indexed by module in the array)
	std::vector<unsigned long>	n_asic_pause;	///< Number of asic pause signals in the time-sorted data input tree (indexed by module in the array)
	std::vector<unsigned long>	n_asic_resume;	///< Number of asic resume signals in the time-sorted data input tree (indexed by module in the array)
	std::vector<unsigned long>	n_asic_pulser;	///< Number of asic pulses in the time-sorted data input tree (indexed by module in the array)

	// Array Histograms
	std::vector<std::vector<TH2F*>> pn_11; ///< TODO Brief description.
	std::vector<std::vector<TH2F*>> pn_12; ///< TODO Brief description.
	std::vector<std::vector<TH2F*>> pn_21; ///< TODO Brief description.
	std::vector<std::vector<TH2F*>> pn_22; ///< TODO Brief description.
	std::vector<std::vector<TH2F*>> pn_ab; ///< TODO Brief description.
	std::vector<std::vector<TH2F*>> pn_nab; ///< TODO Brief description.
	std::vector<std::vector<TH2F*>> pn_pab; ///< TODO Brief description.
	std::vector<std::vector<TH2F*>> pn_max; ///< TODO Brief description.
	std::vector<std::vector<TH1F*>> pn_td; ///< TODO Brief description.
	std::vector<std::vector<TH1F*>> pp_td; ///< TODO Brief description.
	std::vector<std::vector<TH1F*>> nn_td; ///< TODO Brief description.
	std::vector<std::vector<TH2F*>> pn_mult; ///< TODO Brief description.

	// Timing histograms
	TH1F *tdiff, *tdiff_clean; ///< TODO Brief description.
	TProfile *caen_freq;	///< TODO Brief description.
	TProfile *ebis_freq;	///< TProfile containg the frequency of ebis pulses relative to the ebis time
	TProfile *t1_freq;		///< TProfile containg the frequency of T1 pulses relative to the T1 time
	std::vector<TH1F*> fpga_td, asic_td; ///< TODO Brief description.
	std::vector<TProfile*> fpga_pulser_loss, fpga_freq_diff; ///< TODO Brief description.
	std::vector<TProfile*> fpga_freq, fpga_sync; ///< TODO Brief description.
	std::vector<TProfile*> asic_pulser_loss, asic_freq_diff; ///< TODO Brief description.
	std::vector<TProfile*> asic_freq, asic_sync; ///< TODO Brief description.

	// Recoil histograms
	std::vector<TH2F*> recoil_EdE; ///< TODO Brief description.
	std::vector<TH2F*> recoil_dEsum; ///< TODO Brief description.

	// MWPC histograms
	std::vector<std::vector<TH1F*>> mwpc_tac_axis; ///< TODO Brief description.
	std::vector<TH1F*> mwpc_hit_axis; ///< TODO Brief description.
	TH2F *mwpc_pos; ///< TODO Brief description.

	// ELUM histograms
	TH2F *elum; ///< TODO Brief description.
	
	// ZeroDegree histograms
	TH2F *zd; ///< TODO Brief description.
	
};

#endif

