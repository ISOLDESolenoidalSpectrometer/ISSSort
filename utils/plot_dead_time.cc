//#include "DataPackets.hh"

R__LOAD_LIBRARY(libiss_sort.so)

void plot_dead_time( std::string filename = "sorted/R4_13.root", unsigned char crate = 1,
					 unsigned char mod = 0, unsigned char channel = 0 ){

	// Open file
	TFile *f = new TFile( filename.data() );
	
	// Get Tree
	TTree *t = (TTree*)f->Get("iss_sort");

	// Get entries
	unsigned long long nentries = t->GetEntries();
	
	// Branches, etc
	std::shared_ptr<ISSVmeData> vme1;
	std::shared_ptr<ISSVmeData> vme2;
	ISSDataPackets *data = new ISSDataPackets;
	t->SetBranchAddress( "data", &data );
	
	// Canvas
	TCanvas *c = new TCanvas();
	
	// Histogram
	std::string htitle = "Time difference between consecutive events in crate ";
	htitle += std::to_string(crate) + ", module " + std::to_string(mod);
	htitle += ", channel " + std::to_string(channel) + ";#Deltat [ns]";
	TH1F *h = new TH1F( "h", htitle.data(), 10000, -50, 999995 );

	// Loop
	for( unsigned long long i = 0; i < nentries; ++i ){
	
		// Get entry
		t->GetEntry(i);
		
		// Check if it is FebexData
		if( !data->IsVme() ) continue;

		// If it is febex, get the data packet
		vme1 = data->GetVmeData();

		// Confirm it's in the right channel
		if( crate != vme1->GetCrate() ||
		   mod != vme1->GetModule() ||
		   channel != vme1->GetChannel() )
			continue;
				
	
		// Loop on second event
		for( unsigned long long j = i+1; j < nentries; ++j ){

			// Get entry
			t->GetEntry(j);

			// Check if it is FebexData
			if( !data->IsVme() ) continue;

			// If it is febex, get the data packet
			vme2 = data->GetVmeData();

			// Check it's in the same channel
			if( crate != vme2->GetCrate() ||
			   mod != vme2->GetModule() ||
			   channel != vme2->GetChannel() )
				continue;
			
			// Plot time difference
			h->Fill( (double)vme2->GetTime() - (double)vme1->GetTime() );

			// Once we have the next event in this channel, we're finished
			break;
			
		} // j

	} // i
	
	// Draw it
	h->Draw();
	c->SetLogy();
	
}
