//#include "DataPackets.hh"

R__LOAD_LIBRARY(libiss_sort.so)

void read_traces( std::string filename = "sorted/test_traces.root" ) {

	// Open file
	TFile *f = new TFile( filename.data() );

	// Get Tree
	TTree *t = (TTree*)f->Get("iss_sort");

	// Get entries
	unsigned long long nentries = t->GetEntries();

	// Branches, etc
	ISSDataPackets *data = new ISSDataPackets;
	t->SetBranchAddress( "data", &data );

	// Canvas
	TCanvas *c1 = new TCanvas();

	// Loop
	for( unsigned long long i = 0; i < nentries; ++i ){

		// Skip except for every 16
		//if( i%16 != 0 ) continue;

		// Get entry
		t->GetEntry(i);

		// Check if it is FebexData
		if( !data->IsVme() ) continue;

		// If it is febex, get the data packet
		auto vmedata = data->GetVmeData();

		// Draw trace
		if( vmedata->GetTraceLength() > 0 ) {
			vmedata->GetTraceGraph()->Draw("al");

			// Add label
			TText lab;
			std::string lab_str = "Event #" + std::to_string(i);
			//lab_str += "\nCrate  =  " + std::to_string( vmedata->GetCrate() );
			//lab_str += "\nModule =  " + std::to_string( vmedata->GetModule() );
			//lab_str += "\nChannel = " + std::to_string( vmedata->GetChannel() );
			lab.DrawTextNDC( 0.2, 0.3, lab_str.data() );

			// Update the canvas and wait 50 ms
			c1->Update();
			gSystem->ProcessEvents();
			gSystem->Sleep(200);

		}

	}

	return;

}

