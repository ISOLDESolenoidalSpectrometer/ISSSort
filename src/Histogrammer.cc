#include "Histogrammer.hh"

Histogrammer::Histogrammer( Reaction *myreact, Settings *myset ){
	
	react = myreact;
	set = myset;
		
}

Histogrammer::~Histogrammer(){}


void Histogrammer::MakeHists() {

	std::string hname, htitle;
	
	// For recoil sectors
	recoil_array_td.resize( set->GetNumberOfRecoilSectors() );
	recoil_elum_td.resize( set->GetNumberOfRecoilSectors() );
	
	for( unsigned int i = 0; i < set->GetNumberOfRecoilSectors(); ++i ) {
	
		recoil_array_td[i].resize( set->GetNumberOfArrayModules() );
		recoil_elum_td[i].resize( set->GetNumberOfELUMSectors() );
	
		// For array modules
		for( unsigned int j = 0; j < set->GetNumberOfArrayModules(); ++j ) {
		
			hname = "td_recoil_array_" + std::to_string(i) + "_" + std::to_string(j);
			recoil_array_td[i][j] = new TH1F( hname.data(), htitle.data(),
						set->GetEventWindow(), -1.0*set->GetEventWindow(), 1.0*set->GetEventWindow() );
		
		}		

		// For ELUM sectors
		for( unsigned int j = 0; j < set->GetNumberOfELUMSectors(); ++j ) {
		
			hname = "td_recoil_elum_" + std::to_string(i) + "_" + std::to_string(j);
			recoil_elum_td[i][j] = new TH1F( hname.data(), htitle.data(),
						set->GetEventWindow(), -1.0*set->GetEventWindow(), 1.0*set->GetEventWindow() );
		
		}
			
	} // Recoils
	
	// Recoil-array time walk
	recoil_array_tw = new TH2F( "recoil_array_tw",
						"Time-walk histogram for array-recoil coincidences;#Delta{t} [ns];Array energy [keV];Counts",
						set->GetEventWindow()*2, -1.0*set->GetEventWindow(), 1.0*set->GetEventWindow(),
						800, 0, 16000 );
					
	// For array modules
	E_vs_z_mod.resize( set->GetNumberOfArrayModules() );
	E_vs_z_ebis_mod.resize( set->GetNumberOfArrayModules() );
	E_vs_z_recoil_mod.resize( set->GetNumberOfArrayModules() );
	E_vs_z_recoilT_mod.resize( set->GetNumberOfArrayModules() );
	Ex_mod.resize( set->GetNumberOfArrayModules() );
	Ex_ebis_mod.resize( set->GetNumberOfArrayModules() );
	Ex_recoil_mod.resize( set->GetNumberOfArrayModules() );
	Ex_recoilT_mod.resize( set->GetNumberOfArrayModules() );
	Ex_vs_theta_mod.resize( set->GetNumberOfArrayModules() );
	Ex_vs_theta_ebis_mod.resize( set->GetNumberOfArrayModules() );
	Ex_vs_theta_recoil_mod.resize( set->GetNumberOfArrayModules() );		
	Ex_vs_theta_recoilT_mod.resize( set->GetNumberOfArrayModules() );		
	
	for( unsigned int j = 0; j < set->GetNumberOfArrayModules(); ++j ) {
	
		hname = "E_vs_z_" + std::to_string(j);
		htitle = "Energy vs. z distance for module " + std::to_string(j);
		htitle += ";z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_mod[j] = new TH2F( hname.data(), htitle.data(), 3000, -1500, 1500, 800, 0, 16000 );
		
		hname = "E_vs_z_ebis_" + std::to_string(j);
		htitle = "Energy vs. z distance for module " + std::to_string(j);
		htitle += " gated on EBIS;z [mm];Energy [keV];Counts per mm per 20 keV";	
		E_vs_z_ebis_mod[j] = new TH2F( hname.data(), htitle.data(), 3000, -1500, 1500, 800, 0, 16000 );
		
		hname = "E_vs_z_recoil_" + std::to_string(j);
		htitle = "Energy vs. z distance for module " + std::to_string(j);
		htitle += " gated on recoils;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_recoil_mod[j] = new TH2F( hname.data(), htitle.data(), 3000, -1500, 1500, 800, 0, 16000 );	
		
		hname = "E_vs_z_recoilT_" + std::to_string(j);
		htitle = "Energy vs. z distance for module " + std::to_string(j);
		htitle += " with a time gate on all recoils;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_recoilT_mod[j] = new TH2F( hname.data(), htitle.data(), 3000, -1500, 1500, 800, 0, 16000 );	
		
		hname = "Ex_" + std::to_string(j);
		htitle = "Excitation energy for module " + std::to_string(j);
		htitle += ";Excitation energy [keV];Counts per mm per 20 keV";
		Ex_mod[j] = new TH1F( hname.data(), htitle.data(), 800, 0, 16000 );

		hname = "Ex_ebis_" + std::to_string(j);
		htitle = "Excitation energy for module " + std::to_string(j);
		htitle += " gated by EBIS;Excitation energy [keV];Counts per mm per 20 keV";
		Ex_ebis_mod[j] = new TH1F( hname.data(), htitle.data(), 800, 0, 16000 );
		
		hname = "Ex_recoil_" + std::to_string(j);
		htitle = "Excitation energy for module " + std::to_string(j);
		htitle += " gated by recoils;Excitation energy [keV];Counts per mm per 20 keV";
		Ex_recoil_mod[j] = new TH1F( hname.data(), htitle.data(), 800, 0, 16000 );
		
		hname = "Ex_recoilT_" + std::to_string(j);
		htitle = "Excitation energy for module " + std::to_string(j);
		htitle += " with a time gate on all recoils;Excitation energy [keV];Counts per mm per 20 keV";
		Ex_recoilT_mod[j] = new TH1F( hname.data(), htitle.data(), 800, 0, 16000 );
		
		hname = "Ex_vs_theta_" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += ";#theta_{CM} [rad.];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_theta_mod[j] = new TH2F( hname.data(), htitle.data(), 314, 0, TMath::Pi(), 800, 0, 16000 );

		hname = "Ex_vs_theta_ebis_" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += " gated by EBIS;#theta_{CM} [rad.];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_theta_ebis_mod[j] = new TH2F( hname.data(), htitle.data(), 314, 0, TMath::Pi(), 800, 0, 16000 );
		
		hname = "Ex_vs_theta_recoil_" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += " gated by recoils;#theta_{CM} [rad.];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_theta_recoil_mod[j] = new TH2F( hname.data(), htitle.data(), 314, 0, TMath::Pi(), 800, 0, 16000 );
	
		hname = "Ex_vs_theta_recoilT_" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += " with a time gate on all recoils;#theta_{CM} [rad.];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_theta_recoilT_mod[j] = new TH2F( hname.data(), htitle.data(), 314, 0, TMath::Pi(), 800, 0, 16000 );
	
	} // Array
	
	hname = "E_vs_z";
	htitle = "Energy vs. z distance;z [mm];Energy [keV];Counts per mm per 20 keV";
	E_vs_z = new TH2F( hname.data(), htitle.data(), 3000, -1500, 1500, 800, 0, 16000 );
	
	hname = "E_vs_z_ebis";
	htitle = "Energy vs. z distance gated on EBIS;z [mm];Energy [keV];Counts per mm per 20 keV";	
	E_vs_z_ebis = new TH2F( hname.data(), htitle.data(), 3000, -1500, 1500, 800, 0, 16000 );
	
	hname = "E_vs_z_recoil";
	htitle = "Energy vs. z distance gated on recoils;z [mm];Energy [keV];Counts per mm per 20 keV";
	E_vs_z_recoil = new TH2F( hname.data(), htitle.data(), 3000, -1500, 1500, 800, 0, 16000 );	
	
	hname = "E_vs_z_recoilT";
	htitle = "Energy vs. z distance with a time gate on recoils;z [mm];Energy [keV];Counts per mm per 20 keV";
	E_vs_z_recoilT = new TH2F( hname.data(), htitle.data(), 3000, -1500, 1500, 800, 0, 16000 );	
	
	hname = "Ex";
	htitle = "Excitation energy;Excitation energy [keV];Counts per mm per 20 keV";
	Ex = new TH1F( hname.data(), htitle.data(), 800, 0, 16000 );

	hname = "Ex_ebis";
	htitle = "Excitation energy gated by EBIS;Excitation energy [keV];Counts per mm per 20 keV";
	Ex_ebis = new TH1F( hname.data(), htitle.data(), 800, 0, 16000 );
	
	hname = "Ex_recoil";
	htitle = "Excitation energy gated by recoils;Excitation energy [keV];Counts per mm per 20 keV";
	Ex_recoil = new TH1F( hname.data(), htitle.data(), 800, 0, 16000 );
	
	hname = "Ex_recoilT";
	htitle = "Excitation energy with a time gate on all recoils;Excitation energy [keV];Counts per mm per 20 keV";
	Ex_recoilT = new TH1F( hname.data(), htitle.data(), 800, 0, 16000 );
	
	hname = "Ex_vs_theta";
	htitle = "Excitation energy vs. centre of mass angle;#theta_{CM} [rad.];Excitation energy [keV];Counts per mm per 20 keV";
	Ex_vs_theta = new TH2F( hname.data(), htitle.data(), 314, 0, TMath::Pi(), 800, 0, 16000 );

	hname = "Ex_vs_theta_ebis";
	htitle = "Excitation energy vs. centre of mass angle gated by EBIS;#theta_{CM} [rad.];Excitation energy [keV];Counts per mm per 20 keV";
	Ex_vs_theta_ebis = new TH2F( hname.data(), htitle.data(), 314, 0, TMath::Pi(), 800, 0, 16000 );
	
	hname = "Ex_vs_theta_recoil";
	htitle = "Excitation energy vs. centre of mass angle gated by recoils;#theta_{CM} [rad.];Excitation energy [keV];Counts per mm per 20 keV";
	Ex_vs_theta_recoil = new TH2F( hname.data(), htitle.data(), 314, 0, TMath::Pi(), 800, 0, 16000 );
	
	hname = "Ex_vs_theta_recoilT";
	htitle = "Excitation energy vs. centre of mass angle with a time gate on all recoils;#theta_{CM} [rad.];Excitation energy [keV];Counts per mm per 20 keV";
	Ex_vs_theta_recoilT = new TH2F( hname.data(), htitle.data(), 314, 0, TMath::Pi(), 800, 0, 16000 );
	
	
	// For ELUM sectors
	elum_sec.resize( set->GetNumberOfELUMSectors() );
	elum_ebis_sec.resize( set->GetNumberOfELUMSectors() );
	elum_recoil_sec.resize( set->GetNumberOfELUMSectors() );
	elum_recoilT_sec.resize( set->GetNumberOfELUMSectors() );

	for( unsigned int j = 0; j < set->GetNumberOfELUMSectors(); ++j ) {
		
		hname = "elum_sec" + std::to_string(j);
		htitle = "ELUM singles for sector " + std::to_string(j);
		htitle += ";Energy [keV];Counts 100 keV";
		elum_sec[j] = new TH1F( hname.data(), htitle.data(), 4096, 0, 409600 );

		hname = "elum_ebis_sec" + std::to_string(j);
		htitle = "ELUM events for sector " + std::to_string(j);
		htitle += " gated on EBIS;Energy [keV];Counts 100 keV";
		elum_ebis_sec[j] = new TH1F( hname.data(), htitle.data(), 4096, 0, 409600 );

		hname = "elum_recoil_sec" + std::to_string(j);
		htitle = "ELUM singles for sector " + std::to_string(j);
		htitle += " gated on recoils;Energy [keV];Counts 100 keV";
		elum_recoil_sec[j] = new TH1F( hname.data(), htitle.data(), 4096, 0, 409600 );

		hname = "elum_recoilT_sec" + std::to_string(j);
		htitle = "ELUM singles for sector " + std::to_string(j);
		htitle += " with a time gate on all recoils;Energy [keV];Counts 100 keV";
		elum_recoilT_sec[j] = new TH1F( hname.data(), htitle.data(), 4096, 0, 409600 );
		
		
	} // ELUM
	
	elum = new TH1F( "elum", "ELUM singles", 4096, 0, 409600 );
	elum_ebis = new TH1F( "elum_ebis", "ELUM gated on EBIS", 4096, 0, 409600 );
	elum_recoil = new TH1F( "elum_recoil", "ELUM gate on recoils", 4096, 0, 409600 );
	elum_recoilT = new TH1F( "elum_recoilT", "ELUM with time gate on all recoils", 4096, 0, 409600 );
	
}

unsigned long Histogrammer::FillHists( unsigned long start_fill ) {
	
	/// Main function to fill the histograms
	n_entries = input_tree->GetEntries();

	std::cout << " Histogrammer: number of entries in event tree = ";
	std::cout << n_entries << std::endl;
	
	if( start_fill == n_entries ){
	
		std::cout << " Histogrammer: Nothing to do..." << std::endl;
		return n_entries;
	
	}
	else {
	
		std::cout << " Histogrammer: Start filling at event #";
		std::cout << std::to_string( start_fill ) << std::endl;
	
	}
	
	// ------------------------------------------------------------------------ //
	// Main loop over TTree to find events
	// ------------------------------------------------------------------------ //
	for( unsigned int i = start_fill; i < n_entries; ++i ){

		// Current event data
		input_tree->GetEntry(i);
		
		// tdiff variable
		double tdiff;
	
			
		// Loop over array events
		for( unsigned int j = 0; j < read_evts->GetArrayMultiplicity(); ++j ){
			
			// Get array event
			array_evt = read_evts->GetArrayEvt(j);
			
			// Do the reaction
			react->MakeReaction( array_evt->GetPosition(), array_evt->GetEnergy() );		
		
			// Singles
			E_vs_z->Fill( array_evt->GetZ(), array_evt->GetEnergy() );
			E_vs_z_mod[array_evt->GetModule()]->Fill( array_evt->GetZ(), array_evt->GetEnergy() );
			Ex->Fill( react->GetEx() );
			Ex_mod[array_evt->GetModule()]->Fill( react->GetEx() );
			Ex_vs_theta->Fill( react->GetThetaCM(), react->GetEx() );
			Ex_vs_theta_mod[array_evt->GetModule()]->Fill( react->GetThetaCM(), react->GetEx() );
		
			// Check for events in the EBIS on-beam window
			if( OnBeam( array_evt ) ){
				
				E_vs_z_ebis->Fill( array_evt->GetZ(), array_evt->GetEnergy() );
				E_vs_z_ebis_mod[array_evt->GetModule()]->Fill( array_evt->GetZ(), array_evt->GetEnergy() );
				Ex_ebis->Fill( react->GetEx() );
				Ex_ebis_mod[array_evt->GetModule()]->Fill( react->GetEx() );
				Ex_vs_theta_ebis->Fill( react->GetThetaCM(), react->GetEx() );
				Ex_vs_theta_ebis_mod[array_evt->GetModule()]->Fill( react->GetThetaCM(), react->GetEx() );
				
			} // ebis	
			
			// Loop over recoil events
			for( unsigned int k = 0; k < read_evts->GetRecoilMultiplicity(); ++k ){
				
				// Get recoil event
				recoil_evt = read_evts->GetRecoilEvt(k);	
				
				// Time differences
				tdiff = (double)recoil_evt->GetTime() - (double)array_evt->GetTime();
				recoil_array_td[recoil_evt->GetSector()][array_evt->GetModule()]->Fill( tdiff );		
				recoil_array_tw->Fill( tdiff, array_evt->GetEnergy() );		

				// Check for prompt events with recoils
				if( PromptCoincidence( recoil_evt, array_evt ) ){
				
					E_vs_z_recoilT->Fill( array_evt->GetZ(), array_evt->GetEnergy() );
					E_vs_z_recoilT_mod[array_evt->GetModule()]->Fill( array_evt->GetZ(), array_evt->GetEnergy() );
					Ex_recoilT->Fill( react->GetEx() );
					Ex_recoilT_mod[array_evt->GetModule()]->Fill( react->GetEx() );
					Ex_vs_theta_recoilT->Fill( react->GetThetaCM(), react->GetEx() );
					Ex_vs_theta_recoilT_mod[array_evt->GetModule()]->Fill( react->GetThetaCM(), react->GetEx() );

					// Add an energy gate
					if( RecoilCut( recoil_evt ) ) {
					
						E_vs_z_recoil->Fill( array_evt->GetZ(), array_evt->GetEnergy() );
						E_vs_z_recoil_mod[array_evt->GetModule()]->Fill( array_evt->GetZ(), array_evt->GetEnergy() );
						Ex_recoil->Fill( react->GetEx() );
						Ex_recoil_mod[array_evt->GetModule()]->Fill( react->GetEx() );
						Ex_vs_theta_recoil->Fill( react->GetThetaCM(), react->GetEx() );
						Ex_vs_theta_recoil_mod[array_evt->GetModule()]->Fill( react->GetThetaCM(), react->GetEx() );
						
					
					} // energy cuts
								
				} // prompt	

			} // recoils		

		} // array
				

		// Loop over ELUM events
		for( unsigned int j = 0; j < read_evts->GetElumMultiplicity(); ++j ){
			
			// Get ELUM event
			elum_evt = read_evts->GetElumEvt(j);
			
			// Singles
			elum->Fill( elum_evt->GetEnergy() );
			elum_sec[elum_evt->GetSector()]->Fill( elum_evt->GetEnergy() );
		
			// Check for events in the EBIS on-beam window
			if( OnBeam( elum_evt ) ){
				
				elum_ebis->Fill( elum_evt->GetEnergy() );
				elum_ebis_sec[elum_evt->GetSector()]->Fill( elum_evt->GetEnergy() );
				
			} // ebis	
			
			// Loop over recoil events
			for( unsigned int k = 0; k < read_evts->GetRecoilMultiplicity(); ++k ){
				
				// Get recoil event
				recoil_evt = read_evts->GetRecoilEvt(k);	
				
				// Time differences
				tdiff = (double)recoil_evt->GetTime() - (double)elum_evt->GetTime();
				recoil_elum_td[recoil_evt->GetSector()][elum_evt->GetSector()]->Fill( tdiff );
				
				// Check for prompt events with recoils
				if( PromptCoincidence( recoil_evt, elum_evt ) ){
				
					elum_recoilT->Fill( elum_evt->GetEnergy() );
					elum_recoilT_sec[elum_evt->GetSector()]->Fill( elum_evt->GetEnergy() );
					
					// Add an energy gate
					if( RecoilCut( recoil_evt ) ) {
					
						elum_recoil->Fill( elum_evt->GetEnergy() );
						elum_recoil_sec[elum_evt->GetSector()]->Fill( elum_evt->GetEnergy() );						
					
					} // energy cuts			
				
				} // prompt	

			} // recoils

		} // ELUM

		if( i % 10000 == 0 || i+1 == n_entries ) {
			
			std::cout << " " << std::setw(8) << std::setprecision(4);
			std::cout << (float)(i+1)*100.0/(float)n_entries << "%    \r";
			std::cout.flush();
			
		}


	} // all events
	
	output_file->Write();
	
	return n_entries;
	
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
