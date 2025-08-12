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
#include <TKey.h>
#include <TROOT.h>

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

	ISSHistogrammer();
	ISSHistogrammer( std::shared_ptr<ISSReaction> myreact, std::shared_ptr<ISSSettings> myset );
	virtual ~ISSHistogrammer(){};

	void Initialise();
	void MakeHists();
	void ResetHist( TObject *obj );
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
		std::cout << " Writing output file...\r";
		std::cout.flush();
		output_file->Write( 0, TObject::kWriteDelete );
		output_file->Close();
		std::cout << " Writing output file... Done!" << std::endl << std::endl;
		//input_tree->ResetBranchAddresses();
	};
	inline void PurgeOutput(){ output_file->Purge(2); }

	inline TFile* GetFile(){ return output_file; };

	// Spy histograms
	void PlotDefaultHists();
	void PlotPhysicsHists();
	void SetSpyHists( std::vector<std::vector<std::string>> hists, short layout[2] );

	// Adds the settings from the external settings file to the class
	inline void AddSettings( std::shared_ptr<ISSSettings> myset ){
		set = myset;
		overwrite_set = true;
	};
	inline void AddReaction( std::shared_ptr<ISSReaction> myreact ){ react = myreact; };
	inline void GenerateReaction( std::string react_file, bool flag_source ){
		if( set.get() == nullptr ){
			std::cerr << "No settings file given to histogrammer... Exiting!!\n";
			exit(0);
		}
		react = std::make_shared<ISSReaction>( react_file, set, flag_source );
	};

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

	// Gamma - array coincidence
	inline bool	PromptCoincidence( std::shared_ptr<ISSGammaRayEvt> g, std::shared_ptr<ISSArrayEvt> a ){
		if( g->GetTime() - a->GetTime() > react->GetArrayGammaPromptTime(0) &&
		    g->GetTime() - a->GetTime() < react->GetArrayGammaPromptTime(1) ) return true;
		else return false;
	};

	// Recoil - elum coincidence
	inline bool	PromptCoincidence( std::shared_ptr<ISSRecoilEvt> r, std::shared_ptr<ISSElumEvt> e ){
		if( r->GetTime() - e->GetTime() > react->GetElumRecoilPromptTime(0) &&
		    r->GetTime() - e->GetTime() < react->GetElumRecoilPromptTime(1) ) return true;
		else return false;
	};

	// Recoil - lume coincidence
	inline bool	PromptCoincidence( std::shared_ptr<ISSMultiLayerDetectorEvt> r, std::shared_ptr<ISSLumeEvt> l ){
		if( r->GetTime() - l->GetTime() > react->GetLumeRecoilPromptTime(0) &&
		    r->GetTime() - l->GetTime() < react->GetLumeRecoilPromptTime(1) ) return true;
		else return false;
	};

	// Fission - Fission coincidence
	inline bool	PromptCoincidence( std::shared_ptr<ISSCDEvt> f1, std::shared_ptr<ISSCDEvt> f2 ){
		if( f1->GetTime() - f2->GetTime() > react->GetFissionFissionPromptTime(0) &&
		    f1->GetTime() - f2->GetTime() < react->GetFissionFissionPromptTime(1) ) return true;
		else return false;
	};

	// Fission - Gamma coincidence
	inline bool	PromptCoincidence( std::shared_ptr<ISSGammaRayEvt> g, std::shared_ptr<ISSCDEvt> f ){
		if( g->GetTime() - f->GetTime() > react->GetFissionGammaPromptTime(0) &&
		    g->GetTime() - f->GetTime() < react->GetFissionGammaPromptTime(1) ) return true;
		else return false;
	};

	// Recoil - Gamma coincidence
	inline bool	PromptCoincidence( std::shared_ptr<ISSGammaRayEvt> g, std::shared_ptr<ISSRecoilEvt> r ){
		if( g->GetTime() - r->GetTime() > react->GetRecoilGammaPromptTime(0) &&
		    g->GetTime() - r->GetTime() < react->GetRecoilGammaPromptTime(1) ) return true;
		else return false;
	};

	// Gamma - Gamma coincidence
	inline bool	PromptCoincidence( std::shared_ptr<ISSGammaRayEvt> g1, std::shared_ptr<ISSGammaRayEvt> g2 ){
		if( g1->GetTime() - g2->GetTime() > react->GetGammaGammaPromptTime(0) &&
		    g1->GetTime() - g2->GetTime() < react->GetGammaGammaPromptTime(1) ) return true;
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

	inline bool	RandomCoincidence( std::shared_ptr<ISSMultiLayerDetectorEvt> r, std::shared_ptr<ISSLumeEvt> l ){
		if( r->GetTime() - l->GetTime() > react->GetLumeRecoilRandomTime(0) &&
		    r->GetTime() - l->GetTime() < react->GetLumeRecoilRandomTime(1) ) return true;
		else return false;
	};

	inline bool	RandomCoincidence( std::shared_ptr<ISSCDEvt> f1, std::shared_ptr<ISSCDEvt> f2 ){
		if( f1->GetTime() - f2->GetTime() > react->GetFissionFissionRandomTime(0) &&
		    f1->GetTime() - f2->GetTime() < react->GetFissionFissionRandomTime(1) ) return true;
		else return false;
	};

	inline bool	RandomCoincidence( std::shared_ptr<ISSGammaRayEvt> g, std::shared_ptr<ISSCDEvt> f ){
		if( g->GetTime() - f->GetTime() > react->GetFissionGammaRandomTime(0) &&
		    g->GetTime() - f->GetTime() < react->GetFissionGammaRandomTime(1) ) return true;
		else return false;
	};

	inline bool	RandomCoincidence( std::shared_ptr<ISSGammaRayEvt> g, std::shared_ptr<ISSArrayEvt> a ){
		if( g->GetTime() - a->GetTime() > react->GetArrayGammaRandomTime(0) &&
		   g->GetTime() - a->GetTime() < react->GetArrayGammaRandomTime(1) ) return true;
		else return false;
	};

	inline bool	RandomCoincidence( std::shared_ptr<ISSGammaRayEvt> g1, std::shared_ptr<ISSGammaRayEvt> g2 ){
		if( g1->GetTime() - g2->GetTime() > react->GetGammaGammaRandomTime(0) &&
		    g1->GetTime() - g2->GetTime() < react->GetGammaGammaRandomTime(1) ) return true;
		else return false;
	};

	inline bool	RandomCoincidence( std::shared_ptr<ISSGammaRayEvt> g, std::shared_ptr<ISSRecoilEvt> r ){
		if( g->GetTime() - r->GetTime() > react->GetRecoilGammaRandomTime(0) &&
		    g->GetTime() - r->GetTime() < react->GetRecoilGammaRandomTime(1) ) return true;
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
	inline bool	OnBeam( std::shared_ptr<ISSGammaRayEvt> g ){
		if( g->GetTime() - read_evts->GetEBIS() >= 0 &&
		    g->GetTime() - read_evts->GetEBIS() < react->GetEBISOnTime() ) return true;
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
	inline bool	OffBeam( std::shared_ptr<ISSGammaRayEvt> g ){
		if( g->GetTime() - read_evts->GetEBIS() >= react->GetEBISOnTime() &&
		    g->GetTime() - read_evts->GetEBIS() < react->GetEBISOffTime() ) return true;
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

	// Gamma-ray energy gate
	inline bool GammaEnergyCut( std::shared_ptr<ISSGammaRayEvt> g ){
		if( g->GetEnergy() > react->GetGammaEnergyCut(0) &&
		    g->GetEnergy() < react->GetGammaEnergyCut(1) ) return true;
		else return false;
	};

	// Recoil energy gate
	inline bool RecoilCut( std::shared_ptr<ISSRecoilEvt> r ){
		std::shared_ptr<TCutG> mycut = react->GetRecoilCut( r->GetSector() );
		if( mycut.get() == nullptr ) return false;
		return mycut->IsInside( r->GetEnergyRest( set->GetRecoilEnergyRestStart(), set->GetRecoilEnergyRestStop() ),
							    r->GetEnergyLoss( set->GetRecoilEnergyLossStart(), set->GetRecoilEnergyLossStop() ) );
	};
	inline bool RecoilCut( std::shared_ptr<ISSCDEvt> r ){
		std::shared_ptr<TCutG> mycut = react->GetRecoilCut(0);
		if( mycut.get() == nullptr ) return false;
		else return mycut->IsInside( r->GetEnergyRest( set->GetCDEnergyRestStart(), set->GetCDEnergyRestStop() ),
									 r->GetEnergyLoss( set->GetCDEnergyLossStart(), set->GetCDEnergyLossStop() ) );
	};

	// Fission fragment energy gates
	inline bool FissionCut( std::shared_ptr<ISSCDEvt> f, std::shared_ptr<TCutG> mycut ){

		// x variable
		double xvar;
		if( std::strcmp( mycut->GetVarX(), "dE" ) == 0 )
			xvar = f->GetEnergyLoss( set->GetCDEnergyRestStart(), set->GetCDEnergyRestStop() );
		else if( std::strcmp( mycut->GetVarX(), "E" ) == 0 )
			xvar = f->GetEnergyRest( set->GetCDEnergyLossStart(), set->GetCDEnergyLossStop() );
		else if( std::strcmp( mycut->GetVarX(), "Etot" ) == 0 )
			xvar = f->GetEnergyTotal( set->GetCDEnergyLossStart(), set->GetCDEnergyLossStop() );
		else if( std::strcmp( mycut->GetVarX(), "ring" ) == 0 )
			xvar = f->GetRing();
		else // assume E by default
			xvar = f->GetEnergyRest( set->GetCDEnergyLossStart(), set->GetCDEnergyLossStop() );

		// y variable
		double yvar;
		if( std::strcmp( mycut->GetVarY(), "dE" ) == 0 )
			yvar = f->GetEnergyLoss( set->GetCDEnergyRestStart(), set->GetCDEnergyRestStop() );
		else if( std::strcmp( mycut->GetVarY(), "E" ) == 0 )
			yvar = f->GetEnergyRest( set->GetCDEnergyLossStart(), set->GetCDEnergyLossStop() );
		else if( std::strcmp( mycut->GetVarY(), "Etot" ) == 0 )
			yvar = f->GetEnergyTotal( set->GetCDEnergyLossStart(), set->GetCDEnergyLossStop() );
		else if(std::strcmp( mycut->GetVarY(), "ring" ) == 0 )
			yvar = f->GetRing();
		else // assume dE by default
			yvar = f->GetEnergyLoss( set->GetCDEnergyRestStart(), set->GetCDEnergyRestStop() );

		return mycut->IsInside( xvar, yvar );

	};
	inline bool FissionCutHeavy( std::shared_ptr<ISSCDEvt> f ){
		std::shared_ptr<TCutG> mycut = react->GetFissionCutHeavy();
		if( mycut.get() == nullptr ) return false;
		else return FissionCut( f, mycut );
	};
	inline bool FissionCutLight( std::shared_ptr<ISSCDEvt> f ){
		std::shared_ptr<TCutG> mycut = react->GetFissionCutLight();
		if( mycut.get() == nullptr ) return false;
		else return FissionCut( f, mycut );
	};


private:

	// Reaction
	std::shared_ptr<ISSReaction> react;

	// Settings file
	std::shared_ptr<ISSSettings> set;
	bool overwrite_set;

	/// Input tree
	TChain *input_tree;
	ISSEvts *read_evts = nullptr;
	std::shared_ptr<ISSMultiLayerDetectorEvt> generic_evt;
	std::shared_ptr<ISSArrayEvt> array_evt;
	std::shared_ptr<ISSArrayPEvt> arrayp_evt;
	std::shared_ptr<ISSRecoilEvt> recoil_evt;
	std::shared_ptr<ISSElumEvt> elum_evt;
	std::shared_ptr<ISSZeroDegreeEvt> zd_evt;
	std::shared_ptr<ISSLumeEvt> lume_evt;
	std::shared_ptr<ISSGammaRayEvt> gamma_evt1; // need two gamma_evts so
	std::shared_ptr<ISSGammaRayEvt> gamma_evt2; // we can look for coincidences
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

	// Check if histograms are made
	bool hists_ready = false;

	// Canvas and hist lists for the spy
	std::vector<std::vector<std::string>> spyhists;
	short spylayout[2];
	std::unique_ptr<TCanvas> c1, c2;
	bool spymode = false;

	// Counters
	unsigned long n_entries;

	//------------//
	// Histograms //
	//------------//

	// Timing
	std::vector<std::vector<TH1F*>> recoil_array_td;
	std::vector<std::vector<TH1F*>> recoil_elum_td;
	std::vector<TH1F*> fission_array_td;
	std::vector<TH1F*> recoil_lume_td;
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

	// Multiplicities
	TH2F *mult_array_fission, *mult_array_recoil, *mult_array_gamma;
	TH2F *mult_gamma_fission, *mult_gamma_recoil;

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
	TH2F* fission_fission_secsec;
	TH2F* fission_fission_ringring;
	TH2F* fission_fission_dEdE;
	TH2F* fission_fission_dEdE_array;
	TH2F* fission_dE_vs_ring;
	TH2F* fission_Etot_vs_ring;
	TH2F* fission_xy_map;
	TH2F* fission_xy_map_cutH;
	TH2F* fission_xy_map_cutL;

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
	TH2F *E_vs_z_fission, *E_vs_z_fissionT, *E_vs_z_fission_gamma;
	TH2F *E_vs_z_fission_random, *E_vs_z_fissionT_random, *E_vs_z_fission_gamma_random;
	TH2F *E_vs_z_gamma, *E_vs_z_gammaT;
	TH2F *E_vs_z_gamma_random, *E_vs_z_gammaT_random;

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
	TH2F *E_vs_theta_gamma, *E_vs_theta_gammaT;
	TH2F *E_vs_theta_gamma_random, *E_vs_theta_gammaT_random;

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
	TH2F *Ex_vs_theta_gamma, *Ex_vs_theta_gammaT;
	TH2F *Ex_vs_theta_gamma_random, *Ex_vs_theta_gammaT_random;

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
	TH2F *Ex_vs_z_gamma, *Ex_vs_z_gammaT;
	TH2F *Ex_vs_z_gamma_random, *Ex_vs_z_gammaT_random;

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
	TH1F *Ex_fission, *Ex_fissionT, *Ex_fission_gamma;
	TH1F *Ex_fission_random, *Ex_fissionT_random, *Ex_fission_gamma_random;
	TH1F *Ex_gamma, *Ex_gammaT;
	TH1F *Ex_gamma_random, *Ex_gammaT_random;
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
	TH1F *Theta_gamma, *Theta_gammaT;
	TH1F *Theta_gamma_random, *Theta_gammaT_random;

	// ELUM
	std::vector<TH1F*> elum_sec;
	std::vector<TH1F*> elum_ebis_sec;
	std::vector<TH1F*> elum_ebis_on_sec;
	std::vector<TH1F*> elum_ebis_off_sec;
	std::vector<TH1F*> elum_recoil_sec;
	std::vector<TH1F*> elum_recoilT_sec;
	std::vector<TH1F*> elum_recoil_random_sec;
	std::vector<TH1F*> elum_recoilT_random_sec;
	TH1F *elum, *elum_ebis, *elum_ebis_on, *elum_ebis_off;
	TH1F *elum_recoil, *elum_recoilT;
	TH1F *elum_recoil_random, *elum_recoilT_random;
	TH2F *elum_vs_T1;

    // LUME
	std::vector<TH1F*> lume_det;
	std::vector<TH1F*> lume_ebis_det;
	std::vector<TH1F*> lume_ebis_on_det;
	std::vector<TH1F*> lume_ebis_off_det;
	std::vector<TH1F*> lume_recoil_det;
	std::vector<TH1F*> lume_recoilT_det;
	std::vector<TH1F*> lume_recoil_random_det;
	std::vector<TH1F*> lume_recoilT_random_det;
	std::vector<TH2F*> lume_E_vs_x_det;
	std::vector<TH2F*> lume_E_vs_x_ebis_det;
	std::vector<TH2F*> lume_E_vs_x_ebis_on_det;
	std::vector<TH2F*> lume_E_vs_x_ebis_off_det;
	TH1F *lume, *lume_ebis, *lume_ebis_on, *lume_ebis_off;
    TH2F *lume_E_vs_x, *lume_vs_T1, *lume_E_vs_x_ebis;
    TH2F *lume_E_vs_x_ebis_on, *lume_E_vs_x_ebis_off;
    TH2F *lume_E_vs_x_wide;
	TH1F *lume_recoil, *lume_recoilT;
	TH1F *lume_recoil_random, *lume_recoilT_random;

	// Gamma rays
	TH1F *gamma_ebis, *gamma_ebis_on, *gamma_ebis_off;
	TH2F *gamma_gamma_ebis;
	TH1F *gamma_fission, *gamma_recoil, *gamma_recoilT, *gamma_array;
	TH1F *gamma_gamma_td, *gamma_fission_td, *gamma_recoil_td, *gamma_array_td;
	TH2F *gamma_gamma_fission, *gamma_gamma_recoil, *gamma_gamma_array;
	TH2F *gamma_Ex_ebis, *gamma_Ex_fission, *gamma_Ex_recoilT, *gamma_Ex_recoil;
	std::vector<TH1F*> gamma_array_cut;
	std::vector<TH2F*> gamma_gamma_array_cut;

};

#endif
