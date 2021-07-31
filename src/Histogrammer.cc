#include "Histogrammer.hh"

Histogrammer::Histogrammer( Reaction *myreact ){
	
	react = myreact;
	
}
Histogrammer::~Histogrammer(){}


void Histogrammer::MakeHists() {
	
	
	
}

void Histogrammer::Loop() {
	
	nentries = input_tree->GetEntries();
	
	// Loop overy array events
	for( unsigned int i = 0; i < nentries; ++i ){
		
		
		
	}
	
	return;
	
}

void Histogrammer::Terminate() {
	
	// Close output file
	output_file->Close();
	
}

void Histogrammer::SetInputFile( std::vector<std::string> input_file_names ) {
	
	/// Overlaoded function for a single file or multiple files
	input_tree = new TChain( "evt_tree" );
	for( unsigned int i = 0; i < input_file_names.size(); i++ ) {
	
		input_tree->Add( input_file_names[i].data() );
		
	}
	input_tree->SetBranchAddress( "ISSEvts", &read_evts );

	return;
	
}

void Histogrammer::SetInputTree( TTree *user_tree ){
	
	// Find the tree and set branch addresses
	input_tree = (TChain*)user_tree;
	input_tree->SetBranchAddress( "ISSEvts", &read_evts );

	return;
	
}
