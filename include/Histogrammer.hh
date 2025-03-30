#ifndef __HISTOGRAMMER_HH
#define __HISTOGRAMMER_HH

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

#include <TFile.h>
#include <TTree.h>
#include <TMath.h>
#include <TRandom3.h>
#include <TChain.h>
#include <TProfile.h>
#include <TH1.h>
#include <TH2.h>
#include <TCutG.h>
#include <TGProgressBar.h>
#include <TSystem.h>


// Reaction header
#ifndef __REACTION_HH
# include "Reaction.hh"
#endif

// ISS Events tree
#ifndef __ISSEVTS_HH
# include "ISSEvts.hh"
#endif

// Settings file
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif


class ISSHistogrammer {
	
public:

	ISSHistogrammer( std::shared_ptr<ISSReaction> myreact, std::shared_ptr<ISSSettings> myset );
	virtual ~ISSHistogrammer(){};
	
	void MakeHists();
	void ResetHists();
	unsigned long FillHists();
	
	void SetInputFile( std::vector<std::string> input_file_names );
	void SetInputFile( std::string input_file_name );
	void SetInputTree( TTree* user_tree );

	void SetPace4File( std::vector<std::string> input_file_names );
	void SetPace4File( std::string input_file_name );
	void ReadPace4File( std::string input_file_name );

	void SetOutput( std::string output_file_name );
	inline void CloseOutput(){
		PurgeOutput();
		output_file->Close();
		//input_tree->ResetBranchAddresses();
	};
	inline void PurgeOutput(){ output_file->Purge(2); }

	inline TFile* GetFile(){ return output_file; };
	
	inline void AddProgressBar( std::shared_ptr<TGProgressBar> myprog ){
		prog = myprog;
		_prog_ = true;
	};
	
	// Recoil - array coincidence
	inline bool	PromptCoincidence( std::shared_ptr<ISSRecoilEvt> r, std::shared_ptr<ISSArrayEvt> a ){
		if( r->GetTime() - a->GetTime() > react->GetArrayRecoilPromptTime(0) &&
		   r->GetTime() - a->GetTime() < react->GetArrayRecoilPromptTime(1) ) return true;
		else return false;
	};

	// Fission - array coincidence
	inline bool	PromptCoincidence( std::shared_ptr<ISSCDEvt> f, std::shared_ptr<ISSArrayEvt> a ){
		if( f->GetTime() - a->GetTime() > react->GetArrayFissionPromptTime(0) &&
		    f->GetTime() - a->GetTime() < react->GetArrayFissionPromptTime(1) ) return true;
		else return false;
	};
	
	// Recoil - elum coincidence
	inline bool	PromptCoincidence( std::shared_ptr<ISSRecoilEvt> r, std::shared_ptr<ISSElumEvt> e ){
		if( r->GetTime() - e->GetTime() > react->GetElumRecoilPromptTime(0) &&
		    r->GetTime() - e->GetTime() < react->GetElumRecoilPromptTime(1) ) return true;
		else return false;
	};
	
	// Fission - elum coincidence
	inline bool	PromptCoincidence( std::shared_ptr<ISSCDEvt> f, std::shared_ptr<ISSElumEvt> e ){
		if( f->GetTime() - e->GetTime() > react->GetElumFissionPromptTime(0) &&
		    f->GetTime() - e->GetTime() < react->GetElumFissionPromptTime(1) ) return true;
		else return false;
	};

	// Fission - Fission coincidence
	inline bool	PromptCoincidence( std::shared_ptr<ISSCDEvt> f1, std::shared_ptr<ISSCDEvt> f2 ){
		if( f1->GetTime() - f2->GetTime() > react->GetFissionFissionPromptTime(0) &&
		    f1->GetTime() - f2->GetTime() < react->GetFissionFissionPromptTime(1) ) return true;
		else return false;
	};

	inline bool	RandomCoincidence( std::shared_ptr<ISSRecoilEvt> r, std::shared_ptr<ISSArrayEvt> a ){
		if( r->GetTime() - a->GetTime() > react->GetArrayRecoilRandomTime(0) &&
			r->GetTime() - a->GetTime() < react->GetArrayRecoilRandomTime(1) ) return true;
		else return false;
	};
	
	inline bool	RandomCoincidence( std::shared_ptr<ISSCDEvt> f, std::shared_ptr<ISSArrayEvt> a ){
		if( f->GetTime() - a->GetTime() > react->GetArrayFissionRandomTime(0) &&
		    f->GetTime() - a->GetTime() < react->GetArrayFissionRandomTime(1) ) return true;
		else return false;
	};

	inline bool	RandomCoincidence( std::shared_ptr<ISSRecoilEvt> r, std::shared_ptr<ISSElumEvt> e ){
		if( r->GetTime() - e->GetTime() > react->GetElumRecoilRandomTime(0) &&
			r->GetTime() - e->GetTime() < react->GetElumRecoilRandomTime(1) ) return true;
		else return false;
	};

	inline bool	RandomCoincidence( std::shared_ptr<ISSCDEvt> f1, std::shared_ptr<ISSCDEvt> f2 ){
		if( f1->GetTime() - f2->GetTime() > react->GetFissionFissionRandomTime(0) &&
		    f1->GetTime() - f2->GetTime() < react->GetFissionFissionRandomTime(1) ) return true;
		else return false;
	};

	inline bool	OnBeam( std::shared_ptr<ISSRecoilEvt> r ){
		if( r->GetTime() - read_evts->GetEBIS() >= 0 &&
			r->GetTime() - read_evts->GetEBIS() < react->GetEBISOnTime() ) return true;
		else return false;
	};
	inline bool	OnBeam( std::shared_ptr<ISSElumEvt> e ){
		if( e->GetTime() - read_evts->GetEBIS() >= 0 &&
			e->GetTime() - read_evts->GetEBIS() < react->GetEBISOnTime() ) return true;
		else return false;
	};
	inline bool	OnBeam( std::shared_ptr<ISSArrayEvt> a ){
		if( a->GetTime() - read_evts->GetEBIS() >= 0 &&
			a->GetTime() - read_evts->GetEBIS() < react->GetEBISOnTime() ) return true;
		else return false;
	};
	inline bool	OnBeam( std::shared_ptr<ISSZeroDegreeEvt> z ){
		if( z->GetTime() - read_evts->GetEBIS() >= 0 &&
			z->GetTime() - read_evts->GetEBIS() < react->GetEBISOnTime() ) return true;
		else return false;
	};
	inline bool	OnBeam( std::shared_ptr<ISSLumeEvt> l ){
		if( l->GetTime() - read_evts->GetEBIS() >= 0 &&
		    l->GetTime() - read_evts->GetEBIS() < react->GetEBISOnTime() ) return true;
		else return false;
	};
	inline bool	OnBeam( std::shared_ptr<ISSCDEvt> f ){
		if( f->GetTime() - read_evts->GetEBIS() >= 0 &&
		    f->GetTime() - read_evts->GetEBIS() < react->GetEBISOnTime() ) return true;
		else return false;
	};
	inline bool	OffBeam( std::shared_ptr<ISSRecoilEvt> r ){
		if( r->GetTime() - read_evts->GetEBIS() >= react->GetEBISOnTime() &&
			r->GetTime() - read_evts->GetEBIS() < react->GetEBISOffTime() ) return true;
		else return false;
	};
	inline bool	OffBeam( std::shared_ptr<ISSElumEvt> e ){
		if( e->GetTime() - read_evts->GetEBIS() >= react->GetEBISOnTime() &&
			e->GetTime() - read_evts->GetEBIS() < react->GetEBISOffTime() ) return true;
		else return false;
	};
	inline bool	OffBeam( std::shared_ptr<ISSArrayEvt> a ){
		if( a->GetTime() - read_evts->GetEBIS() >= react->GetEBISOnTime() &&
			a->GetTime() - read_evts->GetEBIS() < react->GetEBISOffTime() ) return true;
		else return false;
	};
	inline bool	OffBeam( std::shared_ptr<ISSZeroDegreeEvt> z ){
		if( z->GetTime() - read_evts->GetEBIS() >= react->GetEBISOnTime() &&
			z->GetTime() - read_evts->GetEBIS() < react->GetEBISOffTime() ) return true;
		else return false;
	};
	inline bool	OffBeam( std::shared_ptr<ISSLumeEvt> l ){
		if( l->GetTime() - read_evts->GetEBIS() >= react->GetEBISOnTime() &&
		    l->GetTime() - read_evts->GetEBIS() < react->GetEBISOffTime() ) return true;
		else return false;
	};
	inline bool	OffBeam( std::shared_ptr<ISSCDEvt> f ){
		if( f->GetTime() - read_evts->GetEBIS() >= react->GetEBISOnTime() &&
		    f->GetTime() - read_evts->GetEBIS() < react->GetEBISOffTime() ) return true;
		else return false;
	};
	inline bool	T1Cut( std::shared_ptr<ISSRecoilEvt> r ){
		if( r->GetTime() - read_evts->GetT1() > react->GetT1MinTime() &&
			r->GetTime() - read_evts->GetT1() < react->GetT1MaxTime() ) return true;
		else return false;
	};
	inline bool	T1Cut( std::shared_ptr<ISSElumEvt> e ){
		if( e->GetTime() - read_evts->GetT1() > react->GetT1MinTime() &&
			e->GetTime() - read_evts->GetT1() < react->GetT1MaxTime() ) return true;
		else return false;
	};
	inline bool	T1Cut( std::shared_ptr<ISSArrayEvt> a ){
		if( a->GetTime() - read_evts->GetT1() > react->GetT1MinTime() &&
			a->GetTime() - read_evts->GetT1() < react->GetT1MaxTime() ) return true;
		else return false;
	};
	inline bool	T1Cut( std::shared_ptr<ISSZeroDegreeEvt> z ){
		if( z->GetTime() - read_evts->GetT1() > react->GetT1MinTime() &&
		    z->GetTime() - read_evts->GetT1() < react->GetT1MaxTime() ) return true;
		else return false;
	};
	inline bool	T1Cut( std::shared_ptr<ISSLumeEvt> l ){
		if( l->GetTime() - read_evts->GetT1() > react->GetT1MinTime() &&
		    l->GetTime() - read_evts->GetT1() < react->GetT1MaxTime() ) return true;
		else return false;
	};
	inline bool	T1Cut( std::shared_ptr<ISSCDEvt> f ){
		if( f->GetTime() - read_evts->GetT1() > react->GetT1MinTime() &&
		    f->GetTime() - read_evts->GetT1() < react->GetT1MaxTime() ) return true;
		else return false;
	};
	inline bool	LaserOn(){ return read_evts->GetLaserStatus(); };
	inline bool	LaserOff(){ return !read_evts->GetLaserStatus(); };

	// Recoil energy gate
	inline bool RecoilCut( std::shared_ptr<ISSRecoilEvt> r ){
		return react->GetRecoilCut( r->GetSector() )
					->IsInside( r->GetEnergyRest( set->GetRecoilEnergyRestStart(), set->GetRecoilEnergyRestStop() ),
								r->GetEnergyLoss( set->GetRecoilEnergyLossStart(), set->GetRecoilEnergyLossStop() ) );
	}

	// Fission fragment energy gates
	inline bool FissionCutHeavy( std::shared_ptr<ISSCDEvt> r ){
		return react->GetFissionCutHeavy()
		->IsInside( r->GetEnergyRest( set->GetCDEnergyRestStart(), set->GetCDEnergyRestStop() ),
				   r->GetEnergyLoss( set->GetCDEnergyLossStart(), set->GetCDEnergyLossStop() ) );
	}
	inline bool FissionCutLight( std::shared_ptr<ISSCDEvt> r ){
		return react->GetFissionCutLight()
		->IsInside( r->GetEnergyRest( set->GetCDEnergyRestStart(), set->GetCDEnergyRestStop() ),
				   r->GetEnergyLoss( set->GetCDEnergyLossStart(), set->GetCDEnergyLossStop() ) );
	}


private:
	
	// Reaction
	std::shared_ptr<ISSReaction> react;
	
	// Settings file
	std::shared_ptr<ISSSettings> set;
	
	/// Input tree
	TChain *input_tree;
	ISSEvts *read_evts = nullptr;
	std::shared_ptr<ISSArrayEvt> array_evt;
	std::shared_ptr<ISSArrayPEvt> arrayp_evt;
	std::shared_ptr<ISSRecoilEvt> recoil_evt;
	std::shared_ptr<ISSElumEvt> elum_evt;
	std::shared_ptr<ISSZeroDegreeEvt> zd_evt;
	std::shared_ptr<ISSLumeEvt> lume_evt;
	std::shared_ptr<ISSCDEvt> cd_evt1; // need two cd_evt's because
	std::shared_ptr<ISSCDEvt> cd_evt2; // we look for coincidences

	/// Output file and tree
	TFile *output_file;
	TTree *output_tree;
	std::unique_ptr<ISSRxEvent> rx_evts; ///< Container for event-by-event reaction data
	std::unique_ptr<ISSRxInfo> rx_info; ///< Container for reaction info (doesn't change for each event)

	
	// Progress bar
	bool _prog_;
	std::shared_ptr<TGProgressBar> prog;

	// Counters
	unsigned long n_entries;
	
	//------------//
	// Histograms //
	//------------//
	
	// Timing
	std::vector<std::vector<TH1F*>> recoil_array_td;
	std::vector<std::vector<TH1F*>> recoil_elum_td;
	std::vector<TH1F*> fission_array_td;
	std::vector<TH1F*> fission_elum_td;
	TH1F *fission_fission_td;
	TH2F *fission_fission_td_sec;
	TH2F *recoil_array_tw_hit0, *recoil_array_tw_hit1;
	std::vector<std::vector<TH2F*>> recoil_array_tw_hit0_row;
	std::vector<std::vector<TH2F*>> recoil_array_tw_hit1_row;
	TProfile *recoil_array_tw_hit0_prof, *recoil_array_tw_hit1_prof;
	TH2F *fission_array_tw_hit0, *fission_array_tw_hit1;
	std::vector<std::vector<TH2F*>> fission_array_tw_hit0_row;
	std::vector<std::vector<TH2F*>> fission_array_tw_hit1_row;
	TProfile *fission_array_tw_hit0_prof, *fission_array_tw_hit1_prof;
	TH1F *ebis_td_recoil, *ebis_td_fission, *ebis_td_array, *ebis_td_elum, *ebis_td_lume;
	TH1F *t1_td_recoil, *sc_td_recoil;
	TH1F *t1_td_fission, *sc_td_fission;

	// Recoils
	std::vector<TH2F*> recoil_EdE;
	std::vector<TH2F*> recoil_EdE_cut;
	std::vector<TH2F*> recoil_EdE_array;
	std::vector<TH2F*> recoil_bragg;
	std::vector<TH2F*> recoil_dE_vs_T1;
	std::vector<TH1F*> recoil_dE_eloss;
	std::vector<TH1F*> recoil_E_eloss;

	// Fission fragments
	TH2F* fission_EdE;
	TH2F* fission_EdE_cutH;
	TH2F* fission_EdE_cutL;
	TH2F* fission_EdE_array;
	TH2F* fission_bragg;
	TH2F* fission_dE_vs_T1;
	TH1F* fission_dE_eloss;
	TH1F* fission_E_eloss;
	TH2F* fission_fission_dEdE;

	// Array - E vs. z
	std::vector<TH2F*> E_vs_z_mod;
	std::vector<TH2F*> E_vs_z_ebis_mod;
	std::vector<TH2F*> E_vs_z_ebis_on_mod;
	std::vector<TH2F*> E_vs_z_ebis_off_mod;
	std::vector<TH2F*> E_vs_z_recoil_mod;
	std::vector<TH2F*> E_vs_z_recoilT_mod;
	std::vector<TH2F*> E_vs_z_recoil_random_mod;
	std::vector<TH2F*> E_vs_z_recoilT_random_mod;
	std::vector<TH2F*> E_vs_z_fission_mod;
	std::vector<TH2F*> E_vs_z_fissionT_mod;
	std::vector<TH2F*> E_vs_z_fission_random_mod;
	std::vector<TH2F*> E_vs_z_fissionT_random_mod;
	std::vector<TH2F*> E_vs_z_cut;
	std::vector<TH2F*> E_vs_z_ebis_cut;
	std::vector<TH2F*> E_vs_z_ebis_on_cut;
	std::vector<TH2F*> E_vs_z_ebis_off_cut;
	std::vector<TH2F*> E_vs_z_recoil_cut;
	std::vector<TH2F*> E_vs_z_recoilT_cut;
	std::vector<TH2F*> E_vs_z_recoil_random_cut;
	std::vector<TH2F*> E_vs_z_recoilT_random_cut;
	std::vector<TH2F*> E_vs_z_fission_cut;
	std::vector<TH2F*> E_vs_z_fissionT_cut;
	std::vector<TH2F*> E_vs_z_fission_random_cut;
	std::vector<TH2F*> E_vs_z_fissionT_random_cut;
	std::vector<TH2F*> E_vs_z_T1_cut;
	TH2F *E_vs_z, *E_vs_z_ebis, *E_vs_z_ebis_on, *E_vs_z_ebis_off;
	TH2F *E_vs_z_recoil, *E_vs_z_recoilT, *E_vs_z_T1;
	TH2F *E_vs_z_recoil_random, *E_vs_z_recoilT_random;
	TH2F *E_vs_z_fission, *E_vs_z_fissionT;
	TH2F *E_vs_z_fission_random, *E_vs_z_fissionT_random;

	// Array - E vs. thetaCM
	std::vector<TH2F*> E_vs_theta_mod;
	std::vector<TH2F*> E_vs_theta_ebis_mod;
	std::vector<TH2F*> E_vs_theta_ebis_on_mod;
	std::vector<TH2F*> E_vs_theta_ebis_off_mod;
	std::vector<TH2F*> E_vs_theta_recoil_mod;
	std::vector<TH2F*> E_vs_theta_recoilT_mod;
	std::vector<TH2F*> E_vs_theta_recoil_random_mod;
	std::vector<TH2F*> E_vs_theta_recoilT_random_mod;
	std::vector<TH2F*> E_vs_theta_fission_mod;
	std::vector<TH2F*> E_vs_theta_fissionT_mod;
	std::vector<TH2F*> E_vs_theta_fission_random_mod;
	std::vector<TH2F*> E_vs_theta_fissionT_random_mod;
	std::vector<TH2F*> E_vs_theta_cut;
	std::vector<TH2F*> E_vs_theta_ebis_cut;
	std::vector<TH2F*> E_vs_theta_ebis_on_cut;
	std::vector<TH2F*> E_vs_theta_ebis_off_cut;
	std::vector<TH2F*> E_vs_theta_recoil_cut;
	std::vector<TH2F*> E_vs_theta_recoilT_cut;
	std::vector<TH2F*> E_vs_theta_recoil_random_cut;
	std::vector<TH2F*> E_vs_theta_recoilT_random_cut;
	std::vector<TH2F*> E_vs_theta_fission_cut;
	std::vector<TH2F*> E_vs_theta_fissionT_cut;
	std::vector<TH2F*> E_vs_theta_fission_random_cut;
	std::vector<TH2F*> E_vs_theta_fissionT_random_cut;
	std::vector<TH2F*> E_vs_theta_T1_cut;
	TH2F *E_vs_theta, *E_vs_theta_ebis, *E_vs_theta_ebis_on, *E_vs_theta_ebis_off;
	TH2F *E_vs_theta_recoil, *E_vs_theta_recoilT, *E_vs_theta_T1;
	TH2F *E_vs_theta_recoil_random, *E_vs_theta_recoilT_random;
	TH2F *E_vs_theta_fission, *E_vs_theta_fissionT;
	TH2F *E_vs_theta_fission_random, *E_vs_theta_fissionT_random;

	// Array - Ex vs. thetaCM
	std::vector<TH2F*> Ex_vs_theta_mod;
	std::vector<TH2F*> Ex_vs_theta_ebis_mod;
	std::vector<TH2F*> Ex_vs_theta_ebis_on_mod;
	std::vector<TH2F*> Ex_vs_theta_ebis_off_mod;
	std::vector<TH2F*> Ex_vs_theta_recoil_mod;
	std::vector<TH2F*> Ex_vs_theta_recoilT_mod;
	std::vector<TH2F*> Ex_vs_theta_recoil_random_mod;
	std::vector<TH2F*> Ex_vs_theta_recoilT_random_mod;
	std::vector<TH2F*> Ex_vs_theta_fission_mod;
	std::vector<TH2F*> Ex_vs_theta_fissionT_mod;
	std::vector<TH2F*> Ex_vs_theta_fission_random_mod;
	std::vector<TH2F*> Ex_vs_theta_fissionT_random_mod;
	std::vector<TH2F*> Ex_vs_theta_cut;
	std::vector<TH2F*> Ex_vs_theta_ebis_cut;
	std::vector<TH2F*> Ex_vs_theta_ebis_on_cut;
	std::vector<TH2F*> Ex_vs_theta_ebis_off_cut;
	std::vector<TH2F*> Ex_vs_theta_recoil_cut;
	std::vector<TH2F*> Ex_vs_theta_recoilT_cut;
	std::vector<TH2F*> Ex_vs_theta_recoil_random_cut;
	std::vector<TH2F*> Ex_vs_theta_recoilT_random_cut;
	std::vector<TH2F*> Ex_vs_theta_fission_cut;
	std::vector<TH2F*> Ex_vs_theta_fissionT_cut;
	std::vector<TH2F*> Ex_vs_theta_fission_random_cut;
	std::vector<TH2F*> Ex_vs_theta_fissionT_random_cut;
	std::vector<TH2F*> Ex_vs_theta_T1_cut;
	TH2F *Ex_vs_theta, *Ex_vs_theta_ebis, *Ex_vs_theta_ebis_on, *Ex_vs_theta_ebis_off;
	TH2F *Ex_vs_theta_recoil, *Ex_vs_theta_recoilT, *Ex_vs_theta_T1;
	TH2F *Ex_vs_theta_recoil_random, *Ex_vs_theta_recoilT_random;
	TH2F *Ex_vs_theta_fission, *Ex_vs_theta_fissionT;
	TH2F *Ex_vs_theta_fission_random, *Ex_vs_theta_fissionT_random;

	// Array - Ex vs. z
	std::vector<TH2F*> Ex_vs_z_mod;
	std::vector<TH2F*> Ex_vs_z_ebis_mod;
	std::vector<TH2F*> Ex_vs_z_ebis_on_mod;
	std::vector<TH2F*> Ex_vs_z_ebis_off_mod;
	std::vector<TH2F*> Ex_vs_z_recoil_mod;
	std::vector<TH2F*> Ex_vs_z_recoilT_mod;
	std::vector<TH2F*> Ex_vs_z_recoil_random_mod;
	std::vector<TH2F*> Ex_vs_z_recoilT_random_mod;
	std::vector<TH2F*> Ex_vs_z_fission_mod;
	std::vector<TH2F*> Ex_vs_z_fissionT_mod;
	std::vector<TH2F*> Ex_vs_z_fission_random_mod;
	std::vector<TH2F*> Ex_vs_z_fissionT_random_mod;
	std::vector<TH2F*> Ex_vs_z_cut;
	std::vector<TH2F*> Ex_vs_z_ebis_cut;
	std::vector<TH2F*> Ex_vs_z_ebis_on_cut;
	std::vector<TH2F*> Ex_vs_z_ebis_off_cut;
	std::vector<TH2F*> Ex_vs_z_recoil_cut;
	std::vector<TH2F*> Ex_vs_z_recoilT_cut;
	std::vector<TH2F*> Ex_vs_z_recoil_random_cut;
	std::vector<TH2F*> Ex_vs_z_recoilT_random_cut;
	std::vector<TH2F*> Ex_vs_z_fission_cut;
	std::vector<TH2F*> Ex_vs_z_fissionT_cut;
	std::vector<TH2F*> Ex_vs_z_fission_random_cut;
	std::vector<TH2F*> Ex_vs_z_fissionT_random_cut;
	std::vector<TH2F*> Ex_vs_z_T1_cut;
	TH2F *Ex_vs_z, *Ex_vs_z_ebis, *Ex_vs_z_ebis_on, *Ex_vs_z_ebis_off;
	TH2F *Ex_vs_z_recoil, *Ex_vs_z_recoilT, *Ex_vs_z_T1;
	TH2F *Ex_vs_z_recoil_random, *Ex_vs_z_recoilT_random;
	TH2F *Ex_vs_z_fission, *Ex_vs_z_fissionT;
	TH2F *Ex_vs_z_fission_random, *Ex_vs_z_fissionT_random;

	// Array - Ex
	std::vector<TH1F*> Ex_mod;
	std::vector<TH1F*> Ex_ebis_mod;
	std::vector<TH1F*> Ex_ebis_on_mod;
	std::vector<TH1F*> Ex_ebis_off_mod;
	std::vector<TH1F*> Ex_recoil_mod;
	std::vector<TH1F*> Ex_recoilT_mod;
	std::vector<TH1F*> Ex_recoil_random_mod;
	std::vector<TH1F*> Ex_recoilT_random_mod;
	std::vector<TH1F*> Ex_fission_mod;
	std::vector<TH1F*> Ex_fissionT_mod;
	std::vector<TH1F*> Ex_fission_random_mod;
	std::vector<TH1F*> Ex_fissionT_random_mod;
	std::vector<TH1F*> Ex_cut;
	std::vector<TH1F*> Ex_ebis_cut;
	std::vector<TH1F*> Ex_ebis_on_cut;
	std::vector<TH1F*> Ex_ebis_off_cut;
	std::vector<TH1F*> Ex_recoil_cut;
	std::vector<TH1F*> Ex_recoilT_cut;
	std::vector<TH1F*> Ex_recoil_random_cut;
	std::vector<TH1F*> Ex_recoilT_random_cut;
	std::vector<TH1F*> Ex_fission_cut;
	std::vector<TH1F*> Ex_fissionT_cut;
	std::vector<TH1F*> Ex_fission_random_cut;
	std::vector<TH1F*> Ex_fissionT_random_cut;
	std::vector<TH1F*> Ex_T1_cut;
	std::vector<TH2F*> Ex_vs_T1_cut;
	TH1F *Ex, *Ex_ebis, *Ex_ebis_on, *Ex_ebis_off;
	TH1F *Ex_recoil, *Ex_recoilT, *Ex_T1;
	TH1F *Ex_recoil_random, *Ex_recoilT_random;
	TH1F *Ex_fission, *Ex_fissionT;
	TH1F *Ex_fission_random, *Ex_fissionT_random;
	TH2F *Ex_vs_T1;

	// Array - thetaCM
	std::vector<TH1F*> Theta_mod;
	std::vector<TH1F*> Theta_ebis_mod;
	std::vector<TH1F*> Theta_ebis_on_mod;
	std::vector<TH1F*> Theta_ebis_off_mod;
	std::vector<TH1F*> Theta_recoil_mod;
	std::vector<TH1F*> Theta_recoilT_mod;
	std::vector<TH1F*> Theta_recoil_random_mod;
	std::vector<TH1F*> Theta_recoilT_random_mod;
	std::vector<TH1F*> Theta_fission_mod;
	std::vector<TH1F*> Theta_fissionT_mod;
	std::vector<TH1F*> Theta_fission_random_mod;
	std::vector<TH1F*> Theta_fissionT_random_mod;
	std::vector<TH1F*> Theta_cut;
	std::vector<TH1F*> Theta_ebis_cut;
	std::vector<TH1F*> Theta_ebis_on_cut;
	std::vector<TH1F*> Theta_ebis_off_cut;
	std::vector<TH1F*> Theta_recoil_cut;
	std::vector<TH1F*> Theta_recoilT_cut;
	std::vector<TH1F*> Theta_recoil_random_cut;
	std::vector<TH1F*> Theta_recoilT_random_cut;
	std::vector<TH1F*> Theta_fission_cut;
	std::vector<TH1F*> Theta_fissionT_cut;
	std::vector<TH1F*> Theta_fission_random_cut;
	std::vector<TH1F*> Theta_fissionT_random_cut;
	std::vector<TH1F*> Theta_T1_cut;
	TH1F *Theta, *Theta_ebis, *Theta_ebis_on, *Theta_ebis_off;
	TH1F *Theta_recoil, *Theta_recoilT, *Theta_T1;
	TH1F *Theta_recoil_random, *Theta_recoilT_random;
	TH1F *Theta_fission, *Theta_fissionT;
	TH1F *Theta_fission_random, *Theta_fissionT_random;
	TH2F *Theta_vs_T1;
		
	// ELUM
	std::vector<TH1F*> elum_sec;
	std::vector<TH1F*> elum_ebis_sec;
	std::vector<TH1F*> elum_ebis_on_sec;
	std::vector<TH1F*> elum_ebis_off_sec;
	std::vector<TH1F*> elum_recoil_sec;
	std::vector<TH1F*> elum_recoilT_sec;
	std::vector<TH1F*> elum_recoil_random_sec;
	std::vector<TH1F*> elum_recoilT_random_sec;
	std::vector<TH1F*> elum_fission_sec;
	std::vector<TH1F*> elum_fissionT_sec;
	std::vector<TH1F*> elum_fission_random_sec;
	std::vector<TH1F*> elum_fissionT_random_sec;
	TH1F *elum, *elum_ebis, *elum_ebis_on, *elum_ebis_off;
	TH1F *elum_recoil, *elum_recoilT;
	TH1F *elum_recoil_random, *elum_recoilT_random;
	TH1F *elum_fission, *elum_fissionT;
	TH1F *elum_fission_random, *elum_fissionT_random;
	TH2F *elum_vs_T1;

    // LUME
	std::vector<TH1F*> lume_det;
	std::vector<TH1F*> lume_ebis_det;
	std::vector<TH1F*> lume_ebis_on_det;
	std::vector<TH1F*> lume_ebis_off_det;
	std::vector<TH2F*> lume_E_vs_x_det;
	std::vector<TH2F*> lume_E_vs_x_ebis_det;
	std::vector<TH2F*> lume_E_vs_x_ebis_on_det;
	std::vector<TH2F*> lume_E_vs_x_ebis_off_det;
	TH1F *lume, *lume_ebis, *lume_ebis_on, *lume_ebis_off;
    TH2F *lume_E_vs_x, *lume_vs_T1, *lume_E_vs_x_ebis;
    TH2F *lume_E_vs_x_ebis_on, *lume_E_vs_x_ebis_off;
    TH2F *lume_E_vs_x_wide;

};

#endif
