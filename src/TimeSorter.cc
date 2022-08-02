#include "TimeSorter.hh"

ISSTimeSorter::ISSTimeSorter(){
	
	// No progress bar by default
	_prog_ = false;
	
	// No input/output files yet
	flag_input_file = false;
	flag_output_file = false;

}

bool ISSTimeSorter::SetInputFile( std::string input_file_name ){
	
	// Open next Root input file.
	input_file = new TFile( input_file_name.data(), "read" );
	if( input_file->IsZombie() ) {
		
		std::cout << "Cannot open " << input_file_name << std::endl;
		return false;
		
	}
	
	flag_input_file = true;
	
	// Set the input tree
	SetInputTree( (TTree*)input_file->Get("iss") );

	std::cout << "Sorting file by timestamp: " << input_file->GetName() << std::endl;

	return true;
	
}

void ISSTimeSorter::SetInputTree( TTree* user_tree ){

	// Find the tree and set branch addresses
	input_tree = user_tree;
	input_tree->SetBranchAddress( "data", &in_data );
	input_tree->SetCacheSize(200000000); // 200 MB
	//input_tree->SetCacheEntryRange( 0, input_tree->GetEntries()-1 );
	input_tree->AddBranchToCache( "*", kTRUE );
	//input_tree->StopCacheLearningPhase();
	
	// Need to make a clone for the output
	MakeTree();
	
	return;
	
}

void ISSTimeSorter::SetOutput( std::string output_file_name ){

	// Open ROOT file
	SetOutputFile( output_file_name );
	
	// Create output tree
	MakeTree();
	
}

void ISSTimeSorter::SetOutputFile( std::string output_file_name ){
	
	// Open root file
	output_file = new TFile( output_file_name.data(), "recreate", "Time sorted ISS data" );
	//output_file->SetCompressionLevel(0);
	output_file->cd();

	flag_output_file = true;

	return;
	
};

void ISSTimeSorter::MakeTree(){

	// Create output Root file and Tree.
	output_tree = (TTree*)input_tree->CloneTree(0);
	output_tree->SetName( "iss_sort" );
	output_tree->SetTitle( "Time sorted, calibrated ISS data" );
	if( flag_output_file ) {
		output_tree->SetDirectory( output_file->GetDirectory("/") );
		//output_tree->SetBasketSize( "*", 16000 );
		//output_tree->SetAutoFlush( 30*1024*1024 );	// 30 MB
		//output_tree->SetAutoSave( 100*1024*1024 );	// 100 MB
		output_tree->AutoSave();
		//output_tree->GetBranch( "data" )->SetCompressionLevel(0);
	}

	return;
	
};


unsigned long ISSTimeSorter::SortTree( unsigned long start_sort ) {

	// Start timer
	time( &t_start );
	
	// Time sort all entries of the tree
	n_entries = input_tree->GetEntries();
	std::cout << " Sorting: number of entries in input tree = " << n_entries << std::endl;

	if( n_entries > 0 && start_sort < n_entries ) {
		
		// Build time-ordered index
		std::cout << "Building time-ordered index of events..." << std::endl;
		input_tree->BuildIndex( "data.GetTimeMSB()", "data.GetTimeLSB()" );
		//input_tree->BuildIndex( "data.GetTime()" );

		att_index = (TTreeIndex*)input_tree->GetTreeIndex();
		nb_idx = att_index->GetN();
		
		std::cout << " Sorting: size of the sorted index = " << nb_idx << std::endl;

		// Loop on t_raw entries and fill t
		for( unsigned long i = start_sort; i < n_entries; ++i ) {
			
			idx = att_index->GetIndex()[i];
			input_tree->GetEntry( idx );
			output_tree->Fill();
			
			//std::cout << idx << "\t" << in_data->GetTime() << std::endl;
			
			// Progress bar
			bool update_progress = false;
			if( nb_idx < 200 )
				update_progress = true;
			else if( i % (nb_idx/100) == 0 || i+1 == nb_idx )
				update_progress = true;
			
			if( update_progress ) {
				
				// Percent complete
				float percent = (float)(i+1)*100.0/(float)nb_idx;
				
				// Progress bar in GUI
				if( _prog_ ) {
					
					prog->SetPosition( percent );
					gSystem->ProcessEvents();
					
				}
				
				// Progress bar in terminal
				std::cout << " " << std::setw(6) << std::setprecision(4);
				std::cout << percent << "%    \r";
				std::cout.flush();

			}

		}

	}
	
	else std::cout << " Sorting: nothing to sort " << std::endl;
	

	// Write histograms, trees and clean up
	if( flag_input_file ) {
		
		std::cout << "Read " << input_file->GetBytesRead();
		std::cout << " bytes in " << input_file->GetReadCalls();
		std::cout << " transactions" << std::endl;
		
	}
	else input_tree->PrintCacheStats();
	
	output_tree->Write( 0, TObject::kWriteDelete );
	if( flag_output_file ) output_file->SaveSelf();
	
	std::cout << "End ISSTimeSorter: time elapsed = " << time(NULL)-t_start << " sec." << std::endl;
	
	
	return n_entries;
	
}
