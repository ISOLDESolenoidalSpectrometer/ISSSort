// An example script to show how to plot data from the rxtree
// including looking up the strip and module IDs for the array

// To run this, do the following:
// root -l
// root [0] gSystem->Load("libiss_sort")
// root [1] .L utils/elum_energy_calculator.cc
// root [2] elum_energy_calculator

// Maybe you need to include the headers, depending on the ROOT
// version. New version require you only to load a library.
//#include “Settings.hh”
//#include “ISSEvts.hh”
//#include “Reaction.hh”

R__LOAD_LIBRARY(libiss_sort.so)

// Create a settings reaction and events instance
std::shared_ptr<ISSSettings> myset;
std::shared_ptr<ISSReaction> myreact;

void plot_from_rxtree( std::string rootfile = "myhists.root",
					   std::string reaction_file = "dummy" ) {
	
	// Open file and get tree
	TFile *f = new TFile( rootfile.data() );
	TTree *t = (TTree*)f->Get("rxtree");
	
	// Make a reaction instance, etc
	myset = std::make_shared<ISSSettings>( "dummy" );
	myreact = std::make_shared<ISSReaction>( reaction_file, myset, false );
	
	// Set branches
	ISSRxEvent *myrxevt = new ISSRxEvent();
	ISSRxInfo *myrxinfo = new ISSRxInfo();
	t->SetBranchAddress( "RxEvent", &myrxevt );
	t->SetBranchAddress( "RxInfo", &myrxinfo );

	// Make a histogram or two
	TH2F *zphimap = new TH2F( "zphimap", "z-phi hit map", 1000, -1000, 1000, 560, -100, 460 );
	TH2F *pmodmap = new TH2F( "pmodmap", "p vs module hit map", 512, -0.5, 511.5, 3, -0.5, 2.5 );
	TH2F *pnmap = new TH2F( "pnmap", "p vs n hit map", 512, -0.5, 511.5, 264, -0.5, 263.5 );

	// Loop over events
	for( unsigned int i = 0; i < t->GetEntries(); i++ ){
		
		// Get entry
		t->GetEntry(i);
		
		// Calculate module number
		double z = myrxevt->GetDistance();
		double phi = myrxevt->GetPhi();
		int mymod = myrxevt->GetModule();
		int mypid = myrxevt->GetPID();
		int mynid = myrxevt->GetNID();
		int myrow = myrxevt->GetRow();

		// Plot hit maps
		zphimap->Fill( z, phi*TMath::RadToDeg() ); // reaction z-phi, not measured z-phi
		pmodmap->Fill( myrow*128 + mypid, mymod );
		pnmap->Fill( myrow*128 + mypid, mymod*88 + mynid );

	}
	
	return;
	
}
