#include "Histogrammer.hh"

Histogrammer::Histogrammer( Reaction *myreact, Settings *myset ){
	
	react = myreact;
	set = myset;
		
}

Histogrammer::~Histogrammer(){}


void Histogrammer::MakeHists() {

	std::string hname, htitle;
	
	float zmax, zmin = react->GetArrayDistance();
	if( zmin < 0 ) { // upstream
		zmax = zmin + 10;
		zmin -= 520;
	}
	else { //downstream
		zmax = zmin + 520;
		zmin -= 10;
	}

	// Array physics histograms
	hname = "E_vs_z";
	htitle = "Energy vs. z distance;z [mm];Energy [keV];Counts per mm per 20 keV";
	E_vs_z = new TH2F( hname.data(), htitle.data(), 530, zmin, zmax, 800, 0, 16000 );
	
	hname = "E_vs_z_ebis";
	htitle = "Energy vs. z distance gated on EBIS and off beam subtracted;z [mm];Energy [keV];Counts per mm per 20 keV";
	E_vs_z_ebis = new TH2F( hname.data(), htitle.data(), 530, zmin, zmax, 800, 0, 16000 );
	
	hname = "E_vs_z_ebis_on";
	htitle = "Energy vs. z distance gated on EBIS;z [mm];Energy [keV];Counts per mm per 20 keV";
	E_vs_z_ebis_on = new TH2F( hname.data(), htitle.data(), 530, zmin, zmax, 800, 0, 16000 );
	
	hname = "E_vs_z_ebis_off";
	htitle = "Energy vs. z distance gated off EBIS;z [mm];Energy [keV];Counts per mm per 20 keV";
	E_vs_z_ebis_off = new TH2F( hname.data(), htitle.data(), 530, zmin, zmax, 800, 0, 16000 );
	
	hname = "E_vs_z_recoil";
	htitle = "Energy vs. z distance gated on recoils;z [mm];Energy [keV];Counts per mm per 20 keV";
	E_vs_z_recoil = new TH2F( hname.data(), htitle.data(), 530, zmin, zmax, 800, 0, 16000 );
	
	hname = "E_vs_z_recoilT";
	htitle = "Energy vs. z distance with a time gate on recoils;z [mm];Energy [keV];Counts per mm per 20 keV";
	E_vs_z_recoilT = new TH2F( hname.data(), htitle.data(), 530, zmin, zmax, 800, 0, 16000 );
	
	hname = "Ex";
	htitle = "Excitation energy;Excitation energy [keV];Counts per 20 keV";
	Ex = new TH1F( hname.data(), htitle.data(), 800, -1000, 15000 );

	hname = "Ex_ebis";
	htitle = "Excitation energy gated by EBIS and off beam subtracted;Excitation energy [keV];Counts per 20 keV";
	Ex_ebis = new TH1F( hname.data(), htitle.data(), 800, -1000, 15000 );
	
	hname = "Ex_ebis_on";
	htitle = "Excitation energy gated on EBIS;Excitation energy [keV];Counts per 20 keV";
	Ex_ebis_on = new TH1F( hname.data(), htitle.data(), 800, -1000, 15000 );
	
	hname = "Ex_ebis_off";
	htitle = "Excitation energy gated off EBIS;Excitation energy [keV];Counts per 20 keV";
	Ex_ebis_off = new TH1F( hname.data(), htitle.data(), 800, -1000, 15000 );
	
	hname = "Ex_recoil";
	htitle = "Excitation energy gated by recoils;Excitation energy [keV];Counts per 20 keV";
	Ex_recoil = new TH1F( hname.data(), htitle.data(), 800, -1000, 15000 );
	
	hname = "Ex_recoilT";
	htitle = "Excitation energy with a time gate on all recoils;Excitation energy [keV];Counts per 20 keV";
	Ex_recoilT = new TH1F( hname.data(), htitle.data(), 800, -1000, 15000 );
	
	hname = "Ex_vs_theta";
	htitle = "Excitation energy vs. centre of mass angle;#theta_{CM} [rad.];Excitation energy [keV];Counts per mm per 20 keV";
	Ex_vs_theta = new TH2F( hname.data(), htitle.data(), 314, 0, TMath::Pi(), 800, -1000, 15000 );

	hname = "Ex_vs_theta_ebis";
	htitle = "Excitation energy vs. centre of mass angle gated by EBIS and off beam subtracted;#theta_{CM} [rad.];Excitation energy [keV];Counts per mm per 20 keV";
	Ex_vs_theta_ebis = new TH2F( hname.data(), htitle.data(), 314, 0, TMath::Pi(), 800, -1000, 15000 );
	
	hname = "Ex_vs_theta_ebis_on";
	htitle = "Excitation energy vs. centre of mass angle gated on EBIS;#theta_{CM} [rad.];Excitation energy [keV];Counts per mm per 0.01 rad";
	Ex_vs_theta_ebis_on = new TH2F( hname.data(), htitle.data(), 314, 0, TMath::Pi(), 800, -1000, 15000 );
	
	hname = "Ex_vs_theta_ebis_off";
	htitle = "Excitation energy vs. centre of mass angle gated off EBIS;#theta_{CM} [rad.];Excitation energy [keV];Counts per mm per 0.01 rad";
	Ex_vs_theta_ebis_off = new TH2F( hname.data(), htitle.data(), 314, 0, TMath::Pi(), 800, -1000, 15000 );
	
	hname = "Ex_vs_theta_recoil";
	htitle = "Excitation energy vs. centre of mass angle gated by recoils;#theta_{CM} [rad.];Excitation energy [keV];Counts per mm per 0.01 rad";
	Ex_vs_theta_recoil = new TH2F( hname.data(), htitle.data(), 314, 0, TMath::Pi(), 800, -1000, 15000 );
	
	hname = "Ex_vs_theta_recoilT";
	htitle = "Excitation energy vs. centre of mass angle with a time gate on all recoils;#theta_{CM} [rad.];Excitation energy [keV];Counts per mm per 0.01 rad";
	Ex_vs_theta_recoilT = new TH2F( hname.data(), htitle.data(), 314, 0, TMath::Pi(), 800, -1000, 15000 );

	// For each array module
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
	
		hname = "E_vs_z_mod" + std::to_string(j);
		htitle = "Energy vs. z distance for module " + std::to_string(j);
		htitle += ";z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_mod[j] = new TH2F( hname.data(), htitle.data(), 530, zmin, zmax, 800, 0, 16000 );
		
		hname = "E_vs_z_ebis_mod" + std::to_string(j);
		htitle = "Energy vs. z distance for module " + std::to_string(j);
		htitle += " gated on EBIS;z [mm];Energy [keV];Counts per mm per 20 keV";	
		E_vs_z_ebis_mod[j] = new TH2F( hname.data(), htitle.data(), 530, zmin, zmax, 800, 0, 16000 );
		
		hname = "E_vs_z_recoil_mod" + std::to_string(j);
		htitle = "Energy vs. z distance for module " + std::to_string(j);
		htitle += " gated on recoils;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_recoil_mod[j] = new TH2F( hname.data(), htitle.data(), 530, zmin, zmax, 800, 0, 16000 );
		
		hname = "E_vs_z_recoilT_mod" + std::to_string(j);
		htitle = "Energy vs. z distance for module " + std::to_string(j);
		htitle += " with a time gate on all recoils;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_recoilT_mod[j] = new TH2F( hname.data(), htitle.data(), 530, zmin, zmax, 800, 0, 16000 );
		
		hname = "Ex_mod" + std::to_string(j);
		htitle = "Excitation energy for module " + std::to_string(j);
		htitle += ";Excitation energy [keV];Counts per mm per 20 keV";
		Ex_mod[j] = new TH1F( hname.data(), htitle.data(), 850, -2000, 15000 );

		hname = "Ex_ebis_mod" + std::to_string(j);
		htitle = "Excitation energy for module " + std::to_string(j);
		htitle += " gated by EBIS;Excitation energy [keV];Counts per mm per 20 keV";
		Ex_ebis_mod[j] = new TH1F( hname.data(), htitle.data(), 850, -2000, 15000 );
		
		hname = "Ex_recoil_mod" + std::to_string(j);
		htitle = "Excitation energy for module " + std::to_string(j);
		htitle += " gated by recoils;Excitation energy [keV];Counts per mm per 20 keV";
		Ex_recoil_mod[j] = new TH1F( hname.data(), htitle.data(), 850, -2000, 15000 );
		
		hname = "Ex_recoilT_mod" + std::to_string(j);
		htitle = "Excitation energy for module " + std::to_string(j);
		htitle += " with a time gate on all recoils;Excitation energy [keV];Counts per mm per 20 keV";
		Ex_recoilT_mod[j] = new TH1F( hname.data(), htitle.data(), 850, -2000, 15000 );
		
		hname = "Ex_vs_theta_mod" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += ";#theta_{CM} [rad.];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_theta_mod[j] = new TH2F( hname.data(), htitle.data(), 314, 0, TMath::Pi(), 850, -2000, 15000 );

		hname = "Ex_vs_theta_ebis_mod" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += " gated by EBIS;#theta_{CM} [rad.];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_theta_ebis_mod[j] = new TH2F( hname.data(), htitle.data(), 314, 0, TMath::Pi(), 850, -2000, 15000 );
		
		hname = "Ex_vs_theta_recoil_mod" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += " gated by recoils;#theta_{CM} [rad.];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_theta_recoil_mod[j] = new TH2F( hname.data(), htitle.data(), 314, 0, TMath::Pi(), 850, -2000, 15000 );
	
		hname = "Ex_vs_theta_recoilT_mod" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += " with a time gate on all recoils;#theta_{CM} [rad.];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_theta_recoilT_mod[j] = new TH2F( hname.data(), htitle.data(), 314, 0, TMath::Pi(), 850, -2000, 15000 );
	
	} // Array
	
	// For recoil sectors
	recoil_array_td.resize( set->GetNumberOfRecoilSectors() );
	recoil_elum_td.resize( set->GetNumberOfRecoilSectors() );
	recoil_EdE.resize( set->GetNumberOfRecoilSectors() );
	recoil_EdE_cut.resize( set->GetNumberOfRecoilSectors() );
	recoil_EdE_array.resize( set->GetNumberOfRecoilSectors() );

	// Loop over each recoil sector
	for( unsigned int i = 0; i < set->GetNumberOfRecoilSectors(); ++i ) {
	
		// Recoil energy plots
		hname = "recoil_EdE_sec" + std::to_string(i);
		htitle = "Recoil dE-E plot for sector " + std::to_string(i);
		htitle += " - singles;Rest energy, E [keV];Energy loss, dE [keV];Counts";
		recoil_EdE[i] = new TH2F( hname.data(), htitle.data(),
									2000, 0, 200000, 2000, 0, 200000 );

		hname = "recoil_EdE_cut_sec" + std::to_string(i);
		htitle = "Recoil dE-E plot for sector " + std::to_string(i);
		htitle += " - with energy cut;Rest energy, E [keV];Energy loss, dE [keV];Counts";
		recoil_EdE_cut[i] = new TH2F( hname.data(), htitle.data(),
									2000, 0, 200000, 2000, 0, 200000 );

		hname = "recoil_EdE_array_sec" + std::to_string(i);
		htitle = "Recoil dE-E plot for sector " + std::to_string(i);
		htitle += " - in coincidence with array;Rest energy, E [keV];Energy loss, dE [keV];Counts";
		recoil_EdE_array[i] = new TH2F( hname.data(), htitle.data(),
									2000, 0, 200000, 2000, 0, 200000 );

		// Timing plots
		recoil_array_td[i].resize( set->GetNumberOfArrayModules() );
		recoil_elum_td[i].resize( set->GetNumberOfELUMSectors() );
	
		// For array modules
		for( unsigned int j = 0; j < set->GetNumberOfArrayModules(); ++j ) {
		
			hname = "td_recoil_array_sec" + std::to_string(i) + "_mod" + std::to_string(j);
			htitle = "Time difference between array module " + std::to_string(i);
			htitle += " and recoil sector " + std::to_string(j);
			htitle += ";#Deltat;Counts";
			recoil_array_td[i][j] = new TH1F( hname.data(), htitle.data(),
						1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );
		
		}

		// For ELUM sectors
		for( unsigned int j = 0; j < set->GetNumberOfELUMSectors(); ++j ) {
		
			hname = "td_recoil_elum_sec" + std::to_string(i) + "_mod" + std::to_string(j);
			htitle = "Time difference between array module " + std::to_string(i);
			htitle += " and ELUM sector " + std::to_string(j);
			htitle += ";#Deltat;Counts";
			recoil_elum_td[i][j] = new TH1F( hname.data(), htitle.data(),
						1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );
		
		}
			
	} // Recoils
	
	// Recoil-array time walk
	recoil_array_tw = new TH2F( "tw_recoil_array",
						"Time-walk histogram for array-recoil coincidences;#Deltat [ns];Array energy [keV];Counts",
						1000, -1.0*set->GetEventWindow(), 1.0*set->GetEventWindow(),
						800, 0, 16000 );
	
	// EBIS time windows
	ebis_td_recoil = new TH1F( "ebis_td_recoil", "Recoil time with respect to EBIS;#Deltat;Counts per 20 #mus", 5e3, 0, 1e8  );
	ebis_td_array = new TH1F( "ebis_td_array", "Array time with respect to EBIS;#Deltat;Counts per 20 #mus", 5e3, 0, 1e8  );

	
	// For ELUM sectors
	elum_sec.resize( set->GetNumberOfELUMSectors() );
	elum_ebis_sec.resize( set->GetNumberOfELUMSectors() );
	elum_recoil_sec.resize( set->GetNumberOfELUMSectors() );
	elum_recoilT_sec.resize( set->GetNumberOfELUMSectors() );

	for( unsigned int j = 0; j < set->GetNumberOfELUMSectors(); ++j ) {
		
		hname = "elum_sec" + std::to_string(j);
		htitle = "ELUM singles for sector " + std::to_string(j);
		htitle += ";Energy [keV];Counts 5 keV";
		elum_sec[j] = new TH1F( hname.data(), htitle.data(), 10000, 0, 50000 );

		hname = "elum_ebis_sec" + std::to_string(j);
		htitle = "ELUM events for sector " + std::to_string(j);
		htitle += " gated on EBIS;Energy [keV];Counts 5 keV";
		elum_ebis_sec[j] = new TH1F( hname.data(), htitle.data(), 10000, 0, 50000 );

		hname = "elum_recoil_sec" + std::to_string(j);
		htitle = "ELUM singles for sector " + std::to_string(j);
		htitle += " gated on recoils;Energy [keV];Counts 5 keV";
		elum_recoil_sec[j] = new TH1F( hname.data(), htitle.data(), 10000, 0, 50000 );

		hname = "elum_recoilT_sec" + std::to_string(j);
		htitle = "ELUM singles for sector " + std::to_string(j);
		htitle += " with a time gate on all recoils;Energy [keV];Counts 5 keV";
		elum_recoilT_sec[j] = new TH1F( hname.data(), htitle.data(), 10000, 0, 50000 );
		
		
	} // ELUM
	
	elum = new TH1F( "elum", "ELUM singles;Energy (keV);Counts per 5 keV", 10000, 0, 50000 );
	elum_ebis = new TH1F( "elum_ebis", "ELUM gated on EBIS;Energy (keV);Counts per 5 keV", 10000, 0, 50000 );
	elum_recoil = new TH1F( "elum_recoil", "ELUM gate on recoils;Energy (keV);Counts per 5 keV", 10000, 0, 50000 );
	elum_recoilT = new TH1F( "elum_recoilT", "ELUM with time gate on all recoils;Energy (keV);Counts per 5 keV", 10000, 0, 50000 );
	
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
		// if you want the p-side only events, use GetArrayPMultiplicity
		// if you want the "normal" mode using p/n-coincidences, use GetArrayMultiplicity
		for( unsigned int j = 0; j < read_evts->GetArrayMultiplicity(); ++j ){
		//for( unsigned int j = 0; j < read_evts->GetArrayPMultiplicity(); ++j ){

			// Get array event (uncomment the option you want)
			// GetArrayEvt is "normal" mode
			// GetArrayPEvt is p-side only events
			array_evt = read_evts->GetArrayEvt(j);
			//array_evt = read_evts->GetArrayPEvt(j);
			
			// Do the reaction
			react->MakeReaction( array_evt->GetPosition(), array_evt->GetEnergy() );
			
			// Singles
			E_vs_z->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
			E_vs_z_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
			Ex->Fill( react->GetEx() );
			Ex_mod[array_evt->GetModule()]->Fill( react->GetEx() );
			Ex_vs_theta->Fill( react->GetThetaCM(), react->GetEx() );
			Ex_vs_theta_mod[array_evt->GetModule()]->Fill( react->GetThetaCM(), react->GetEx() );
		
			// EBIS time
			ebis_td_array->Fill( (double)array_evt->GetTime() - (double)read_evts->GetEBIS() );

			// Check for events in the EBIS on-beam window
			if( OnBeam( array_evt ) ){
				
				E_vs_z_ebis->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
				E_vs_z_ebis_on->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
				E_vs_z_ebis_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
				Ex_ebis->Fill( react->GetEx() );
				Ex_ebis_on->Fill( react->GetEx() );
				Ex_ebis_mod[array_evt->GetModule()]->Fill( react->GetEx() );
				Ex_vs_theta_ebis->Fill( react->GetThetaCM(), react->GetEx() );
				Ex_vs_theta_ebis_on->Fill( react->GetThetaCM(), react->GetEx() );
				Ex_vs_theta_ebis_mod[array_evt->GetModule()]->Fill( react->GetThetaCM(), react->GetEx() );
				
			} // ebis
			
			else if( OffBeam( array_evt ) ){
				
				E_vs_z_ebis->Fill( react->GetZmeasured(), array_evt->GetEnergy(), -1.0 * react->GetEBISRatio() );
				Ex_ebis->Fill( react->GetEx(), -1.0 * react->GetEBISRatio() );
				Ex_vs_theta_ebis->Fill( react->GetThetaCM(), react->GetEx(), -1.0 * react->GetEBISRatio() );
				E_vs_z_ebis_off->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
				Ex_ebis_off->Fill( react->GetEx() );
				Ex_vs_theta_ebis_off->Fill( react->GetThetaCM(), react->GetEx() );

			}
			
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
				
					E_vs_z_recoilT->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
					E_vs_z_recoilT_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
					Ex_recoilT->Fill( react->GetEx() );
					Ex_recoilT_mod[array_evt->GetModule()]->Fill( react->GetEx() );
					Ex_vs_theta_recoilT->Fill( react->GetThetaCM(), react->GetEx() );
					Ex_vs_theta_recoilT_mod[array_evt->GetModule()]->Fill( react->GetThetaCM(), react->GetEx() );

					// Recoils in coincidence with an array event
					recoil_EdE_array[recoil_evt->GetSector()]->Fill( recoil_evt->GetEnergyRest(), recoil_evt->GetEnergyLoss() );

					// Add an energy gate
					if( RecoilCut( recoil_evt ) ) {
					
						E_vs_z_recoil->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
						E_vs_z_recoil_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
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
		

		// Loop over recoil events
		for( unsigned int i = 0; i < read_evts->GetRecoilMultiplicity(); ++i ){

			// Get recoil event
			recoil_evt = read_evts->GetRecoilEvt(i);
			
			// EBIS time
			ebis_td_recoil->Fill( (double)recoil_evt->GetTime() - (double)read_evts->GetEBIS() );
			
			// Energy EdE plot, unconditioned
			recoil_EdE[recoil_evt->GetSector()]->Fill( recoil_evt->GetEnergyRest(), recoil_evt->GetEnergyLoss() );
			
			// Energy EdE plot, after cut
			if( RecoilCut( recoil_evt ) )
				recoil_EdE_cut[recoil_evt->GetSector()]->Fill( recoil_evt->GetEnergyRest(), recoil_evt->GetEnergyLoss() );
			
		} // recoils

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
	
	/// Overlaaded function for a single file or multiple files
	input_tree = new TChain( "evt_tree" );
	for( unsigned int i = 0; i < input_file_names.size(); i++ ) {
	
		input_tree->Add( input_file_names[i].data() );
		
	}
	input_tree->SetBranchAddress( "ISSEvts", &read_evts );

	return;
	
}

void Histogrammer::SetInputFile( std::string input_file_name ) {
	
	/// Overloaded function for a single file or multiple files
	input_tree = new TChain( "evt_tree" );
	input_tree->Add( input_file_name.data() );
	input_tree->SetBranchAddress( "ISSEvts", &read_evts );

	return;
	
}

void Histogrammer::SetInputTree( TTree *user_tree ){
	
	// Find the tree and set branch addresses
	input_tree = (TChain*)user_tree;
	input_tree->SetBranchAddress( "ISSEvts", &read_evts );

	return;
	
}
