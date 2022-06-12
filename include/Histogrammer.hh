#ifndef __HISTOGRAMMER_hh
#define __HISTOGRAMMER_hh

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include <TFile.h>
#include <TTree.h>
#include <TMath.h>
#include <TChain.h>
#include <TProfile.h>
#include <TH1.h>
#include <TH2.h>
#include <TCutG.h>

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


class Histogrammer {
	
public:

	Histogrammer( Reaction *myreact, Settings *myset );
	virtual inline ~Histogrammer(){};
	
	void MakeHists();
	unsigned long FillHists( unsigned long start_fill = 0 );
	void Terminate();
	
	void SetInputFile( std::vector<std::string> input_file_names );
	void SetInputFile( std::string input_file_name );
	void SetInputTree( TTree* user_tree );

	inline void SetOutput( std::string output_file_name ){
		output_file = new TFile( output_file_name.data(), "recreate" );
		MakeHists();
	};
	inline void CloseOutput( ){
		output_file->Close();
	};

	inline TFile* GetFile(){ return output_file; };
	
	// Coincidence conditions (to be put in settings file eventually)
	// Time walk corrections can also be added here
	inline bool	PromptCoincidence( RecoilEvt *r, ArrayEvt *a ){
		if( (double)r->GetTime() - (double)a->GetTime() > -350 &&
			(double)r->GetTime() - (double)a->GetTime() < 250 ) return true;
		else return false;
	};
	inline bool	RandomCoincidence( RecoilEvt *r, ArrayEvt *a ){
		if( (double)r->GetTime() - (double)a->GetTime() > 600 &&
			(double)r->GetTime() - (double)a->GetTime() < 1200 ) return true;
		else return false;
	};
	inline bool	PromptCoincidence( RecoilEvt *r, ElumEvt *e ){
		if( (double)r->GetTime() - (double)e->GetTime() > -400 &&
			(double)r->GetTime() - (double)e->GetTime() < 100 ) return true;
		else return false;
	};
	inline bool	RandomCoincidence( RecoilEvt *r, ElumEvt *e ){
		if( (double)r->GetTime() - (double)e->GetTime() > 500 &&
			(double)r->GetTime() - (double)e->GetTime() < 1500 ) return true;
		else return false;
	};
	inline bool	OnBeam( RecoilEvt *r ){
		if( (double)r->GetTime() - (double)read_evts->GetEBIS() >= 0 &&
			(double)r->GetTime() - (double)read_evts->GetEBIS() < react->GetEBISOnTime() ) return true;
		else return false;
	};
	inline bool	OnBeam( ElumEvt *e ){
		if( (double)e->GetTime() - (double)read_evts->GetEBIS() >= 0 &&
			(double)e->GetTime() - (double)read_evts->GetEBIS() < react->GetEBISOnTime() ) return true;
		else return false;
	};
	inline bool	OnBeam( ArrayEvt *a ){
		if( (double)a->GetTime() - (double)read_evts->GetEBIS() >= 0 &&
			(double)a->GetTime() - (double)read_evts->GetEBIS() < react->GetEBISOnTime() ) return true;
		else return false;
	};
	inline bool	OnBeam( ZeroDegreeEvt *z ){
		if( (double)z->GetTime() - (double)read_evts->GetEBIS() >= 0 &&
			(double)z->GetTime() - (double)read_evts->GetEBIS() < react->GetEBISOnTime() ) return true;
		else return false;
	};
	inline bool	OffBeam( RecoilEvt *r ){
		if( (double)r->GetTime() - (double)read_evts->GetEBIS() >= react->GetEBISOnTime() &&
			(double)r->GetTime() - (double)read_evts->GetEBIS() < react->GetEBISOffTime() ) return true;
		else return false;
	};
	inline bool	OffBeam( ElumEvt *e ){
		if( (double)e->GetTime() - (double)read_evts->GetEBIS() >= react->GetEBISOnTime() &&
			(double)e->GetTime() - (double)read_evts->GetEBIS() < react->GetEBISOffTime() ) return true;
		else return false;
	};
	inline bool	OffBeam( ArrayEvt *a ){
		if( (double)a->GetTime() - (double)read_evts->GetEBIS() >= react->GetEBISOnTime() &&
			(double)a->GetTime() - (double)read_evts->GetEBIS() < react->GetEBISOffTime() ) return true;
		else return false;
	};
	inline bool	OffBeam( ZeroDegreeEvt *z ){
		if( (double)z->GetTime() - (double)read_evts->GetEBIS() >= react->GetEBISOnTime() &&
			(double)z->GetTime() - (double)read_evts->GetEBIS() < react->GetEBISOffTime() ) return true;
		else return false;
	};

	// Recoil energy gate
	inline bool RecoilCut( RecoilEvt *r ){
		if( react->GetRecoilCut( r->GetSector() )->IsInside( r->GetEnergyRest(), r->GetEnergyLoss() ) )
			return true;
		else return false;
	}

private:
	
	// Reaction
	Reaction *react;
	
	// Settings file
	Settings *set;
	
	/// Input tree
	TChain *input_tree;
	ISSEvts *read_evts = 0;
	ArrayEvt *array_evt;
	ArrayPEvt *arrayp_evt;
	RecoilEvt *recoil_evt;
	ElumEvt *elum_evt;
	ZeroDegreeEvt *zd_evt;
	
	/// Output file
	TFile *output_file;
	
	
	// Counters
	unsigned long n_entries;
	
	//------------//
	// Histograms //
	//------------//
	
	// Timing
	std::vector<std::vector<TH1F*>> recoil_array_td;
	std::vector<std::vector<TH1F*>> recoil_elum_td;
	TH2F *recoil_array_tw;
	TProfile *recoil_array_tw_prof;
	TH1F *ebis_td_recoil, *ebis_td_array, *ebis_td_elum;

	// Recoils
	std::vector<TH2F*> recoil_EdE;
	std::vector<TH2F*> recoil_EdE_cut;
	std::vector<TH2F*> recoil_EdE_array;

	// Array - E vs. z
	std::vector<TH2F*> E_vs_z_mod;
	std::vector<TH2F*> E_vs_z_ebis_mod;
	std::vector<TH2F*> E_vs_z_ebis_on_mod;
	std::vector<TH2F*> E_vs_z_ebis_off_mod;
	std::vector<TH2F*> E_vs_z_recoil_mod;
	std::vector<TH2F*> E_vs_z_recoilT_mod;
	std::vector<TH2F*> E_vs_z_cut;
	std::vector<TH2F*> E_vs_z_ebis_cut;
	std::vector<TH2F*> E_vs_z_ebis_on_cut;
	std::vector<TH2F*> E_vs_z_ebis_off_cut;
	std::vector<TH2F*> E_vs_z_recoil_cut;
	std::vector<TH2F*> E_vs_z_recoilT_cut;
	TH2F *E_vs_z, *E_vs_z_ebis, *E_vs_z_ebis_on, *E_vs_z_ebis_off;
	TH2F *E_vs_z_recoil, *E_vs_z_recoilT;
	
	// Array - Ex vs. thetaCM
	std::vector<TH2F*> Ex_vs_theta_mod;
	std::vector<TH2F*> Ex_vs_theta_ebis_mod;
	std::vector<TH2F*> Ex_vs_theta_ebis_on_mod;
	std::vector<TH2F*> Ex_vs_theta_ebis_off_mod;
	std::vector<TH2F*> Ex_vs_theta_recoil_mod;
	std::vector<TH2F*> Ex_vs_theta_recoilT_mod;
	std::vector<TH2F*> Ex_vs_theta_cut;
	std::vector<TH2F*> Ex_vs_theta_ebis_cut;
	std::vector<TH2F*> Ex_vs_theta_ebis_on_cut;
	std::vector<TH2F*> Ex_vs_theta_ebis_off_cut;
	std::vector<TH2F*> Ex_vs_theta_recoil_cut;
	std::vector<TH2F*> Ex_vs_theta_recoilT_cut;
	TH2F *Ex_vs_theta, *Ex_vs_theta_ebis, *Ex_vs_theta_ebis_on, *Ex_vs_theta_ebis_off;
	TH2F *Ex_vs_theta_recoil, *Ex_vs_theta_recoilT;
		
	// Array - Ex vs. z
	std::vector<TH2F*> Ex_vs_z_mod;
	std::vector<TH2F*> Ex_vs_z_ebis_mod;
	std::vector<TH2F*> Ex_vs_z_ebis_on_mod;
	std::vector<TH2F*> Ex_vs_z_ebis_off_mod;
	std::vector<TH2F*> Ex_vs_z_recoil_mod;
	std::vector<TH2F*> Ex_vs_z_recoilT_mod;
	std::vector<TH2F*> Ex_vs_z_cut;
	std::vector<TH2F*> Ex_vs_z_ebis_cut;
	std::vector<TH2F*> Ex_vs_z_ebis_on_cut;
	std::vector<TH2F*> Ex_vs_z_ebis_off_cut;
	std::vector<TH2F*> Ex_vs_z_recoil_cut;
	std::vector<TH2F*> Ex_vs_z_recoilT_cut;
	TH2F *Ex_vs_z, *Ex_vs_z_ebis, *Ex_vs_z_ebis_on, *Ex_vs_z_ebis_off;
	TH2F *Ex_vs_z_recoil, *Ex_vs_z_recoilT;
		
	// Array - Ex
	std::vector<TH1F*> Ex_mod;
	std::vector<TH1F*> Ex_ebis_mod;
	std::vector<TH1F*> Ex_ebis_on_mod;
	std::vector<TH1F*> Ex_ebis_off_mod;
	std::vector<TH1F*> Ex_recoil_mod;
	std::vector<TH1F*> Ex_recoilT_mod;
	std::vector<TH1F*> Ex_cut;
	std::vector<TH1F*> Ex_ebis_cut;
	std::vector<TH1F*> Ex_ebis_on_cut;
	std::vector<TH1F*> Ex_ebis_off_cut;
	std::vector<TH1F*> Ex_recoil_cut;
	std::vector<TH1F*> Ex_recoilT_cut;
	TH1F *Ex, *Ex_ebis, *Ex_ebis_on, *Ex_ebis_off;
	TH1F *Ex_recoil, *Ex_recoilT;
		
	// ELUM
	std::vector<TH1F*> elum_sec;
	std::vector<TH1F*> elum_ebis_sec;
	std::vector<TH1F*> elum_ebis_on_sec;
	std::vector<TH1F*> elum_ebis_off_sec;
	std::vector<TH1F*> elum_recoil_sec;
	std::vector<TH1F*> elum_recoilT_sec;
	TH1F *elum, *elum_ebis, *elum_ebis_on, *elum_ebis_off;
	TH1F *elum_recoil, *elum_recoilT;
	
};

#endif
