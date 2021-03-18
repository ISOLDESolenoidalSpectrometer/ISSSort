#include "TimeSorter.hh"

TimeSorter::TimeSorter( ){
	
	//std::cout << "constructor" << std::endl;

}

TimeSorter::~TimeSorter() {
	
	//std::cout << "destructor" << std::endl;

}

void TimeSorter::SortTree(){
	
	
	return;
	
}

void TimeSorter::SortFile( std::string input_file_name,
						   std::string output_file_name,
						   std::string log_file_name ) {

	// Create log file.
	std::ofstream log_file;
	log_file.open( log_file_name.data(), std::ios::app );
	
	// Start timer
	time( &t_start );
	
	// Open next Root input file.
	std::cout << "Sorting file by timestamp: " << input_file_name << std::endl;
	input_file = new TFile( input_file_name.data(), "read" );
	if( input_file->IsZombie() ) {
		
		std::cout << "Cannot open " << input_file_name << std::endl;
		return;
		
	}

	// Find the tree from the input file and set branch addresses
	input_tree = (TTree*)input_file->Get("iss_calib");
	n_entries = input_tree->GetEntries();

	// Create output Root file and Tree.
	output_file = new TFile( output_file_name.data(), "recreate", "Time sorted ISS data" );
	output_file->cd();
	output_tree = (TTree*)input_tree->CloneTree(0);
	output_tree->SetDirectory( output_file );
	output_tree->SetName( "iss_sort" );
	output_tree->SetTitle( "Time sorted, calibrated ISS data" );
	//output_tree->SetBasketSize( "*", 16000 );
	output_tree->SetAutoFlush( 30*1024*1024 );	// 30 MB
	output_tree->SetAutoSave( 100*1024*1024 );	// 100 MB
	output_tree->AutoSave();
	
	// Time sort all entries of the tree
	std::cout << " Sorting: number of entries in calibrated tree = " << n_entries << std::endl;
	log_file << " Sorting: number of entries in calibrated tree = " << n_entries << std::endl;

	nb_idx = input_tree->BuildIndex( "0", "time" );
	att_index = (TTreeIndex*)input_tree->GetTreeIndex();

	std::cout << " Sorting: size of the sorted index = " << nb_idx << std::endl;
	log_file << " Sorting: size of the sorted index = " << nb_idx << std::endl;

	// Loop on t_raw entries and fill t
	for( unsigned long long i = 0; i < nb_idx; ++i ) {
		
		idx = att_index->GetIndex()[i];
		input_tree->GetEntry( idx );
		output_tree->Fill();
		
		if( i % 100000 == 0 || i+1 == nb_idx ) {
			
			std::cout << " " << std::setw(8) << std::setprecision(4);
			std::cout << (float)(i+1)*100.0/(float)nb_idx << "%\r";
			std::cout.flush();
			
		}

	}

	
	// Write histograms, trees and clean up
	output_file->cd();
	output_tree->GetCurrentFile()->Write();
	//output_file->Print();
	output_file->Close();
	input_file->Close(); // Close TFile
	
	
	std::cout << "End TimeSorter: time elapsed = " << time(NULL)-t_start << " sec." << std::endl;
	log_file << "End TimeSorter: time elapsed = " << time(NULL)-t_start << " sec." << std::endl;
	
	log_file.close(); //?? to close or not to close?
	
	return;
	
}
