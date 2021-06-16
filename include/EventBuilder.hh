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
	void ParticleFinder();
	void RecoilFinder();
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
	common::real_data in_data;
	
	/// Outputs
	TFile *output_file;
	TTree *output_tree;
	ISSEvts write_evts;
	
	// These things should probably be in the calibration file
	int p_even_hits_pulser; ///> if hit>p_even_hits_max: assume it's a pulser event...
	long p_even_time_window;  /// depends on calibration used by previous step
	
	// Flags
	bool flag_close_event;

	// Time variables
	long		 		time_diff;
	unsigned long long	time_prev, time_min, time_max, time_first;

	// Incoming data variables
	unsigned long long	mytime;		///< absolute timestamp
	float 				myenergy;	///< calibrated energy
	unsigned int		mydet;		///< detector type 0 = array; 1 = recoil; 2 = monitor
	unsigned int		mylayer;	///< 4 wafers along array, 2 dE-E, 13 for gas
	unsigned int		mysector;	///< 6 edges of hexagonal array, 4 quadrants of the recoil
	unsigned int		mystrip;	///< obvious for DSSSD, useless for recoils
	unsigned int		myside;		///< p-side = 0; n-side = 1
	unsigned int		mypair;		///< Number of the DSSSD pair in the readout

	// Particle varibles
	std::vector<std::vector<float>>		pen_list;	///< list of p-side energies for ParticleFinder, per layer
	std::vector<std::vector<float>>		nen_list;	///< list of n-side energies for ParticleFinder, per layer
	std::vector<std::vector<long>>		ptd_list;	///< list of p-side time differences for ParticleFinder, per layer
	std::vector<std::vector<long>>		ntd_list;	///< list of n-side time differences for ParticleFinder, per layer
	std::vector<std::vector<float>>		z_list;		///< list of z values for ParticleFinder, per layer
	std::vector<std::vector<TVector2>>	phixy_list;	///< list of TVector2 values containing phi, x, y info for ParticleFinder, per layer

	// Recoil variables
	std::vector<std::vector<float>>	ren_list;	///< list of recoil energies for RecoilFinder, per quadrant
	std::vector<std::vector<long>>	rtd_list;	///< list of recoil time differences for RecoilFinder, per quadrant
	std::vector<std::vector<float>>	rid_list;	///< list of recoil ids for RecoilFinder, per quadrant

	// Counters
	unsigned int		hit_ctr;
	unsigned int		array_ctr;
	unsigned int		recoil_ctr;
	unsigned int		n_events;
	unsigned int		n_cout;
	unsigned long long	n_entries;
	
	// Histograms
	TH2F *pn_11[common::n_pairs];
	TH2F *pn_12[common::n_pairs];
	TH2F *pn_21[common::n_pairs];
	TH2F *pn_22[common::n_pairs];

	TH1F *pn_td[common::n_pairs];

	TProfile *pn_mult[common::n_pairs];

	TH1F *tdiff;

	
};

#endif

