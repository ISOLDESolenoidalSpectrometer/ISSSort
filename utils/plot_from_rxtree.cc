// An example script to show how to plot data from the rxtree
// including looking up the strip and module IDs for the array

// To run this, do the following:
// root -l
// root [0] gSystem->Load("libiss_sort.so")
// root [1] .L utils/elum_energy_calculator.cc
// root [2] elum_energy_calculator

// Maybe you need to include the headers, depending on the ROOT
// version. New version require you only to load a library.
//#include “Settings.hh”
//#include “ISSEvts.hh”
//#include “Reaction.hh”

// Create a settings reaction and events instance
std::shared_ptr<ISSSettings> myset;
std::shared_ptr<ISSArrayEvt> myevt;
std::shared_ptr<ISSReaction> myreact;
std::shared_ptr<ISSRxEvent>  myrxevt;

void plot_from_rxtree( std::string reaction_file = "dummy" ) {
	
	// Open file and get tree
	TFile *f = new TFile( “myhists.root” );
	TTree *t = (TTree*)f->Get(“rxtree”);
	
	// Make a reaction instance, etc
	myset = std::make_shared<ISSSettings>( "dummy" );
	myreact = std::make_shared<ISSReaction>( reaction_file, myset, false );
	myevt = std::make_shared<ISSArrayEvt>();
	
	// Set branches
	myrxevt = std::make_shared<ISSRxEvent>();
	myrxinf = std::make_shared<ISSRxInfo>();
	t->SetBranchAddress( "RxEvent", &myrxevt );
	t->SetBranchAddress( "RxInfo", &myrxinfo );

	// Make a histogram or two
	TH2F *zphimap = new TH2F( "zphimap", "z-phi hit map", 1000, -1000, 1000, 360, 0, 360 );
	TH2F *pmodmap = new TH2F( "pmodmap", "p vs module hit map", 512, -0.5, 511.5, 3, -0.5, 2.5 );

	// Loop over events
	for( unsigned int i = 0; i < t->GetEntries(); i++ ){
		
		// Get entry
		t->GetEntry(i);
		
		// Calculate module number
		int mymod = FindModule( myrxevt->GetPhiMeasured() );
		int mypid = FindPID( myrxevt->GetDistanceMeasured() );
		int mynid = FindNID( myrxevt->GetPhiMeasured() );
		int myrow = FindRow( myrxevt->GetDistanceMeasured() );

		// Plot hit maps
		zphimap->Fill( myrxevt->GetDistance(), myrxevt->GetPhi() ); // reaction z-phi, not measured z-phi
		pmodmap->Fill( myrow*128 + mypid, mymod );
		
	}
	
	return;
	
}
