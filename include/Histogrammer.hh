#ifndef __HISTOGRAMMER_hh
#define __HISTOGRAMMER_hh

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

// Reaction header
#ifndef __REACTION_HH
# include "Reaction.hh"
#endif

// ISS Events tree
#ifndef __ISSEVTS_HH
# include "ISSEvts.hh"
#endif


class Histogrammer {
	
public:

	Histogrammer( Reaction *myreact );
	~Histogrammer();
	
	void MakeHists();
	void Loop();
	void Terminate();
	
	void SetInputFile( std::vector<std::string> input_file_names );
	void SetInputTree( TTree* user_tree );

	inline void SetOutput( std::string output_file_name ){
		output_file = new TFile( output_file_name.data(), "recreate" );
	};

	inline TFile* GetFile(){ return output_file; };

private:
	
	// Reaction
	Reaction *react;
	
	/// Input tree
	TFile *input_file;
	TChain *input_tree;
	ISSEvts *read_evts;
	ArrayEvt *array_evt;
	RecoilEvt *recoil_evt;
	ElumEvt *elum_evt;
	ZeroDegreeEvt *zd_evt;


	/// Output file
	TFile *output_file;
	
	
	// Counters
	unsigned long nentries;

	
};

#endif
