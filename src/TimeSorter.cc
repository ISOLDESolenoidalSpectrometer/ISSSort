#include "TimeSorter.hh"

TimeSorter::TimeSorter( ){
	
	//std::cout << "constructor" << std::endl;

}

TimeSorter::~TimeSorter() {
	
	//std::cout << "destructor" << std::endl;

}

bool TimeSorter::SetInputFile( std::string input_file_name ){
	
	// Open next Root input file.
	input_file = new TFile( input_file_name.data(), "read" );
	if( input_file->IsZombie() ) {
		
		std::cout << "Cannot open " << input_file_name << std::endl;
		return false;
		
	}
	
	// Set the input tree
	SetInputTree( (TTree*)input_file->Get("iss_sort") );

	std::cout << "Sorting file by timestamp: " << input_file->GetName() << std::endl;

	return true;
	
}

void TimeSorter::SetInputTree( TTree* user_tree ){

	// Find the tree and set branch addresses
	input_tree = user_tree;
	input_tree->SetCacheSize(200000000); // 200 MB
	input_tree->SetCacheEntryRange(0,input_tree->GetEntries()-1);
	input_tree->AddBranchToCache( "*", kTRUE );
	//input_tree->StopCacheLearningPhase();
	return;
	
}

void TimeSorter::SetOutput( std::string output_file_name ){
	
	// Open root file
	output_file = new TFile( output_file_name.data(), "recreate", "Time sorted ISS data" );
	//output_file->SetCompressionLevel(0);

	// Create output Root file and Tree.
	output_file->cd();
	output_tree = (TTree*)input_tree->CloneTree(0);
	output_tree->SetDirectory( output_file->GetDirectory("/") );
	output_tree->SetName( "iss_sort" );
	output_tree->SetTitle( "Time sorted, calibrated ISS data" );
	//output_tree->SetBasketSize( "*", 16000 );
	//output_tree->SetAutoFlush( 30*1024*1024 );	// 30 MB
	//output_tree->SetAutoSave( 100*1024*1024 );	// 100 MB
	output_tree->AutoSave();
	//output_tree->GetBranch( "data" )->SetCompressionLevel(0);

	// Create log file.
	std::string log_file_name = output_file_name.substr( 0, output_file_name.find_last_of(".") );
	log_file_name += ".log";
	log_file.open( log_file_name.data(), std::ios::app );

	return;
	
};

unsigned long TimeSorter::SortFile( unsigned long start_sort ) {

	// Start timer
	time( &t_start );
	
	// Time sort all entries of the tree
	n_entries = input_tree->GetEntries();
	std::cout << " Sorting: number of entries in input tree = " << n_entries << std::endl;
	log_file << " Sorting: number of entries in input tree = " << n_entries << std::endl;

	if( n_entries > 0 && start_sort < n_entries  ) {
		
		nb_idx = input_tree->BuildIndex( "data.GetTimeMSB()", "data.GetTimeLSB()" );
		//nb_idx = input_tree->BuildIndex( "0", "data.GetTime()" );
		att_index = (TTreeIndex*)input_tree->GetTreeIndex();
	
		std::cout << " Sorting: size of the sorted index = " << nb_idx << std::endl;
		log_file << " Sorting: size of the sorted index = " << nb_idx << std::endl;

		// Loop on t_raw entries and fill t
		for( unsigned long i = 0; i < nb_idx; ++i ) {
			
			idx = att_index->GetIndex()[i];
			if( idx < start_sort ) continue;
			input_tree->GetEntry( idx );
			output_tree->Fill();
			
			if( i % (nb_idx/100) == 0 || i+1 == nb_idx ) {
				
				std::cout << " " << std::setw(6) << std::setprecision(4);
				std::cout << (float)(i+1)*100.0/(float)nb_idx << "%    \r";
				std::cout.flush();
				
			}

		}

	}
	
	else {
		
		std::cout << " Sorting: nothing to sort " << std::endl;
		log_file << " Sorting: nothing to sort " << std::endl;
		
	}
	

	// Write histograms, trees and clean up
	std::cout << "Read " << input_file->GetBytesRead();
	std::cout << " bytes in " << input_file->GetReadCalls();
	std::cout << " transactions" << std::endl;
	//input_tree->PrintCacheStats();
	output_tree->Write( 0, TObject::kWriteDelete );
	output_file->SaveSelf();
	//output_file->Print();
	
	std::cout << "End TimeSorter: time elapsed = " << time(NULL)-t_start << " sec." << std::endl;
	log_file << "End TimeSorter: time elapsed = " << time(NULL)-t_start << " sec." << std::endl;
	
	
	return n_entries;
	
}
