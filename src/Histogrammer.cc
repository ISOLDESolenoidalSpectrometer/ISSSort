#include "Histogrammer.hh"

ISSHistogrammer::ISSHistogrammer(){

	Initialise();

}

void ISSHistogrammer::Initialise(){

	// No progress bar by default
	_prog_ = false;

	// No settings file by default
	overwrite_set = false;

	// Make the histograms track the sum of the weights for correctly
	// performing the error propagation when subtracting
	TH1::SetDefaultSumw2(kTRUE);

	// Histogrammer options
	//TH1::AddDirectory(kFALSE);

}

void ISSHistogrammer::SetOutput( std::string output_file_name ){

	// Check we have built the reaction already
	if( react.get() == nullptr ){
		std::cerr << "No reaction file given to histogrammer... Exiting!!\n";
		exit(0);
	}

	// These are the branches we need
	rx_evts	= std::make_unique<ISSRxEvent>();
	rx_info	= std::make_unique<ISSRxInfo>();

	// --------------------------------------------------------- //
	// Create output file and create reaction tree
	// --------------------------------------------------------- //
	output_file = new TFile( output_file_name.data(), "recreate" );
	output_tree = new TTree( "rxtree", "Reaction data tree" );
	output_tree->Branch( "RxEvent", rx_evts.get() );
	output_tree->Branch( "RxInfo", rx_info.get() );
	output_tree->SetAutoFlush();

	// Setup the reaction info
	rx_info->SetRxInfo( react );

	// Histograms in separate function
	MakeHists();

	// flag to denote that hists are ready (used for spy)
	hists_ready = true;

	// Write once
	output_file->Write();

}

void ISSHistogrammer::MakeHists() {

	std::string hname, htitle;
	std::string dirname;

	std::vector<double> zbins;
	double d0 = react->GetArrayDistance();
	double d;

	for( int row = 0; row < 4; row++ ){

		for( int ch = 0; ch < 128; ch++ ){

			// Get the info from the ISSEvt class
			ISSArrayEvt tmp_evt;
			tmp_evt.SetEvent( 0, 0, 127-ch, 0, 0, 0, 0, 0, 0, 3-row );
			d = tmp_evt.GetZ();
			d -= 0.953 / 2.0; // from centre of strip to the edge

			if( d0 < 0 ) zbins.push_back( d0 - d );
			else zbins.push_back( d0 + d );

		} // ch

		// Add the upper edge of the wafer
		if( d0 < 0 ) zbins.push_back( d0 - d - 0.953 );
		else zbins.push_back( d0 + d + 0.953 );

	} // row

	// Add a bin to the start and end for space
	if( d0 < 0 ) {
		zbins.push_back( d0 + 10. );
		zbins.push_back( d0 - d - 10. );
	}
	else {
		zbins.push_back( d0 - 10. );
		zbins.push_back( d0 + d + 10. );
	}

	// Order the bins
	std::sort( zbins.begin(), zbins.end() );

	// Array physics histograms
	// Singles mode
	dirname = "SinglesMode";
	output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	hname = "E_vs_z";
	htitle = "Energy vs. z distance;z [mm];Energy [keV];Counts per mm per 20 keV";
	E_vs_z = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

	hname = "Theta";
	htitle = "Centre of mass angle;#theta_{CM} [deg];Counts per deg";
	Theta = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

	hname = "Ex";
	htitle = "Excitation energy;Excitation energy [keV];Counts per 20 keV";
	Ex = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "E_vs_theta";
	htitle = "Energy vs. centre of mass angle;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
	E_vs_theta = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

	hname = "Ex_vs_theta";
	htitle = "Excitation energy vs. centre of mass angle;#theta_{CM} [deg.];Excitation energy [keV];Counts per deg per 20 keV";
	Ex_vs_theta = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "Ex_vs_z";
	htitle = "Excitation energy vs. measured z;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
	Ex_vs_z = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	// For each user cut
	E_vs_z_cut.resize( react->GetNumberOfEvsZCuts() );
	Theta_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_cut.resize( react->GetNumberOfEvsZCuts() );
	E_vs_theta_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_vs_theta_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_vs_z_cut.resize( react->GetNumberOfEvsZCuts() );
	for( unsigned int j = 0; j < react->GetNumberOfEvsZCuts(); ++j ) {

		dirname = "SinglesMode/cut_" + std::to_string(j);
		output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );

		hname = "E_vs_z_cut" + std::to_string(j);
		htitle = "Energy vs. z distance for user cut " + std::to_string(j);
		htitle += ";z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "Theta_cut"+ std::to_string(j);
		htitle = "Centre of mass angle for user cut " + std::to_string(j);
		htitle += ";#theta_{CM} [deg];Counts per deg";
		Theta_cut[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Ex_cut" + std::to_string(j);
		htitle = "Excitation energy for user cut " + std::to_string(j);
		htitle += ";Excitation energy [keV];Counts per 20 keV";
		Ex_cut[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "E_vs_theta_cut";
		htitle = "Energy vs. centre of mass angle for user cut " + std::to_string(j);
		htitle += ";#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "Ex_vs_theta_cut" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for user cut " + std::to_string(j);
		htitle += ";#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_cut" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for user cut " + std::to_string(j);
		htitle += ";z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );


	}

	// For each array module
	E_vs_z_mod.resize( set->GetNumberOfArrayModules() );
	Theta_mod.resize( set->GetNumberOfArrayModules() );
	Ex_mod.resize( set->GetNumberOfArrayModules() );
	E_vs_theta_mod.resize( set->GetNumberOfArrayModules() );
	Ex_vs_theta_mod.resize( set->GetNumberOfArrayModules() );
	Ex_vs_z_mod.resize( set->GetNumberOfArrayModules() );
	for( unsigned int j = 0; j < set->GetNumberOfArrayModules(); ++j ) {

		dirname = "SinglesMode/module_" + std::to_string(j);
		output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );

		E_vs_z_mod.resize( set->GetNumberOfArrayModules() );
		hname = "E_vs_z_mod" + std::to_string(j);
		htitle = "Energy vs. z distance for module " + std::to_string(j);
		htitle += ";z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "Theta_mod"+ std::to_string(j);
		htitle = "Centre of mass angle for module " + std::to_string(j);
		htitle += ";#theta_{CM} [deg];Counts per deg";
		Theta_mod[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Ex_mod" + std::to_string(j);
		htitle = "Excitation energy for module " + std::to_string(j);
		htitle += ";Excitation energy [keV];Counts per 20 keV";
		Ex_mod[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "E_vs_theta_mod"+ std::to_string(j);
		htitle = "Energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += ";#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "Ex_vs_theta_mod" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += ";#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_mod" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += ";z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(),  react->HistExBins(), react->HistExMin(), react->HistExMax() );

	}

	// EBIS mode
	dirname = "EBISMode";
	output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	hname = "E_vs_z_ebis";
	htitle = "Energy vs. z distance gated on EBIS and off beam subtracted;z [mm];Energy [keV];Counts per mm per 20 keV";
	E_vs_z_ebis = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

	hname = "E_vs_z_ebis_on";
	htitle = "Energy vs. z distance gated on EBIS;z [mm];Energy [keV];Counts per mm per 20 keV";
	E_vs_z_ebis_on = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

	hname = "E_vs_z_ebis_off";
	htitle = "Energy vs. z distance gated off EBIS;z [mm];Energy [keV];Counts per mm per 20 keV";
	E_vs_z_ebis_off = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

	hname = "Theta_ebis";
	htitle = "Centre of mass angle gated by EBIS and off beam subtracted;#theta_{CM} [deg];Counts per deg";
	Theta_ebis = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

	hname = "Theta_ebis_on";
	htitle = "Centre of mass angle gated on EBIS;#theta_{CM} [deg];Counts per deg";
	Theta_ebis_on = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

	hname = "Theta_ebis_off";
	htitle = "Centre of mass angle gated off EBIS;#theta_{CM} [deg];Counts per deg";
	Theta_ebis_off = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

	hname = "Ex_ebis";
	htitle = "Excitation energy gated by EBIS and off beam subtracted;Excitation energy [keV];Counts per 20 keV";
	Ex_ebis = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "Ex_ebis_on";
	htitle = "Excitation energy gated on EBIS;Excitation energy [keV];Counts per 20 keV";
	Ex_ebis_on = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "Ex_ebis_off";
	htitle = "Excitation energy gated off EBIS;Excitation energy [keV];Counts per 20 keV";
	Ex_ebis_off = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "E_vs_theta_ebis";
	htitle = "Energy vs. centre of mass angle gated by EBIS and off beam subtracted;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
	E_vs_theta_ebis = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

	hname = "E_vs_theta_ebis_on";
	htitle = "Energy vs. centre of mass angle gated on EBIS;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
	E_vs_theta_ebis_on = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

	hname = "E_vs_theta_ebis_off";
	htitle = "Energy vs. centre of mass angle gated off EBIS;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
	E_vs_theta_ebis_off = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

	hname = "Ex_vs_theta_ebis";
	htitle = "Excitation energy vs. centre of mass angle gated by EBIS and off beam subtracted;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
	Ex_vs_theta_ebis = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "Ex_vs_theta_ebis_on";
	htitle = "Excitation energy vs. centre of mass angle gated on EBIS;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
	Ex_vs_theta_ebis_on = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "Ex_vs_theta_ebis_off";
	htitle = "Excitation energy vs. centre of mass angle gated off EBIS;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
	Ex_vs_theta_ebis_off = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "Ex_vs_z_ebis";
	htitle = "Excitation energy vs. measured z gated by EBIS and off beam subtracted;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
	Ex_vs_z_ebis = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(),  react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "Ex_vs_z_ebis_on";
	htitle = "Excitation energy vs. measured z gated on EBIS;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
	Ex_vs_z_ebis_on = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "Ex_vs_z_ebis_off";
	htitle = "Excitation energy vs. measured z gated off EBIS;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
	Ex_vs_z_ebis_off = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	// For each user cut
	E_vs_z_ebis_cut.resize( react->GetNumberOfEvsZCuts() );
	E_vs_z_ebis_on_cut.resize( react->GetNumberOfEvsZCuts() );
	E_vs_z_ebis_off_cut.resize( react->GetNumberOfEvsZCuts() );
	Theta_ebis_cut.resize( react->GetNumberOfEvsZCuts() );
	Theta_ebis_on_cut.resize( react->GetNumberOfEvsZCuts() );
	Theta_ebis_off_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_ebis_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_ebis_on_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_ebis_off_cut.resize( react->GetNumberOfEvsZCuts() );
	E_vs_theta_ebis_cut.resize( react->GetNumberOfEvsZCuts() );
	E_vs_theta_ebis_on_cut.resize( react->GetNumberOfEvsZCuts() );
	E_vs_theta_ebis_off_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_vs_theta_ebis_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_vs_theta_ebis_on_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_vs_theta_ebis_off_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_vs_z_ebis_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_vs_z_ebis_on_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_vs_z_ebis_off_cut.resize( react->GetNumberOfEvsZCuts() );
	for( unsigned int j = 0; j < react->GetNumberOfEvsZCuts(); ++j ) {

		dirname = "EBISMode/cut_" + std::to_string(j);
		output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );

		hname = "E_vs_z_ebis_cut" + std::to_string(j);
		htitle = "Energy vs. z distance for user cut " + std::to_string(j);
		htitle += " gated by EBIS and off beam subtracted;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_ebis_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "E_vs_z_ebis_on_cut" + std::to_string(j);
		htitle = "Energy vs. z distance for user cut " + std::to_string(j);
		htitle += " gated on EBIS;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_ebis_on_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "E_vs_z_ebis_off_cut" + std::to_string(j);
		htitle = "Energy vs. z distance for user cut " + std::to_string(j);
		htitle += " gated off EBIS;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_ebis_off_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "Theta_ebis_cut"+ std::to_string(j);
		htitle = "Centre of mass angle for user cut " + std::to_string(j);
		htitle += " gated by EBIS and off beam subtracted;#theta_{CM} [deg];Counts per deg";
		Theta_ebis_cut[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Theta_ebis_on_cut"+ std::to_string(j);
		htitle = "Centre of mass angle for user cut " + std::to_string(j);
		htitle += " gated on EBIS;#theta_{CM} [deg];Counts per deg";
		Theta_ebis_on_cut[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Theta_ebis_off_cut"+ std::to_string(j);
		htitle = "Centre of mass angle for user cut " + std::to_string(j);
		htitle += " gated off EBIS;#theta_{CM} [deg];Counts per deg";
		Theta_ebis_off_cut[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Ex_ebis_cut" + std::to_string(j);
		htitle = "Excitation energy for user cut " + std::to_string(j);
		htitle += " gated by EBIS and off beam subtracted;Excitation energy [keV];Counts per mm per 20 keV";
		Ex_ebis_cut[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_ebis_on_cut" + std::to_string(j);
		htitle = "Excitation energy for user cut " + std::to_string(j);
		htitle += " gated on EBIS;Excitation energy [keV];Counts per 20 keV";
		Ex_ebis_on_cut[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_ebis_off_cut" + std::to_string(j);
		htitle = "Excitation energy for user cut " + std::to_string(j);
		htitle += " gated off EBIS;Excitation energy [keV];Counts per 20 keV";
		Ex_ebis_off_cut[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "E_vs_theta_ebis_cut";
		htitle = "Energy vs. centre of mass angle for user cut " + std::to_string(j);
		htitle += " gated by EBIS and off beam subtracted;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_ebis_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "E_vs_theta_ebis_on_cut";
		htitle = "Energy vs. centre of mass angle for user cut " + std::to_string(j);
		htitle += " gated on EBIS;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_ebis_on_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "E_vs_theta_ebis_off_cut";
		htitle = "Energy vs. centre of mass angle for user cut " + std::to_string(j);
		htitle += " gated off EBIS;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_ebis_off_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "Ex_vs_theta_ebis_cut" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for user cut " + std::to_string(j);
		htitle += " gated by EBIS and off beam subtracted;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_ebis_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_theta_ebis_on_cut" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for user cut " + std::to_string(j);
		htitle += " gated on EBIS;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_ebis_on_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_theta_ebis_off_cut" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for user cut " + std::to_string(j);
		htitle += " gated off EBIS;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_ebis_off_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_ebis_cut" + std::to_string(j);
		htitle = "Excitation energy vs. measured z for user cut " + std::to_string(j);
		htitle += " gated by EBIS and off beam subtracted;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_ebis_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_ebis_on_cut" + std::to_string(j);
		htitle = "Excitation energy vs. measured z  for user cut " + std::to_string(j);
		htitle += " gated on EBIS;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_ebis_on_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_ebis_off_cut" + std::to_string(j);
		htitle = "Excitation energy vs. measured z  for user cut " + std::to_string(j);
		htitle += " gated off EBIS;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_ebis_off_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	}

	// For each array module
	E_vs_z_ebis_mod.resize( set->GetNumberOfArrayModules() );
	E_vs_z_ebis_on_mod.resize( set->GetNumberOfArrayModules() );
	E_vs_z_ebis_off_mod.resize( set->GetNumberOfArrayModules() );
	Theta_ebis_mod.resize( set->GetNumberOfArrayModules() );
	Theta_ebis_on_mod.resize( set->GetNumberOfArrayModules() );
	Theta_ebis_off_mod.resize( set->GetNumberOfArrayModules() );
	Ex_ebis_mod.resize( set->GetNumberOfArrayModules() );
	Ex_ebis_on_mod.resize( set->GetNumberOfArrayModules() );
	Ex_ebis_off_mod.resize( set->GetNumberOfArrayModules() );
	E_vs_theta_ebis_mod.resize( set->GetNumberOfArrayModules() );
	E_vs_theta_ebis_on_mod.resize( set->GetNumberOfArrayModules() );
	E_vs_theta_ebis_off_mod.resize( set->GetNumberOfArrayModules() );
	Ex_vs_theta_ebis_mod.resize( set->GetNumberOfArrayModules() );
	Ex_vs_theta_ebis_on_mod.resize( set->GetNumberOfArrayModules() );
	Ex_vs_theta_ebis_off_mod.resize( set->GetNumberOfArrayModules() );
	Ex_vs_z_ebis_mod.resize( set->GetNumberOfArrayModules() );
	Ex_vs_z_ebis_on_mod.resize( set->GetNumberOfArrayModules() );
	Ex_vs_z_ebis_off_mod.resize( set->GetNumberOfArrayModules() );
	for( unsigned int j = 0; j < set->GetNumberOfArrayModules(); ++j ) {

		dirname = "EBISMode/module_" + std::to_string(j);
		output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );

		hname = "E_vs_z_ebis_mod" + std::to_string(j);
		htitle = "Energy vs. z distance for module " + std::to_string(j);
		htitle += " gated by EBIS and off beam subtracted;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_ebis_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "E_vs_z_ebis_on_mod" + std::to_string(j);
		htitle = "Energy vs. z distance for module " + std::to_string(j);
		htitle += " gated on EBIS;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_ebis_on_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "E_vs_z_ebis_off_mod" + std::to_string(j);
		htitle = "Energy vs. z distance for module " + std::to_string(j);
		htitle += " gated off EBIS;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_ebis_off_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "Theta_ebis_mod"+ std::to_string(j);
		htitle = "Centre of mass angle for module " + std::to_string(j);
		htitle += " gated by EBIS and off beam subtracted;#theta_{CM} [deg];Counts per deg";
		Theta_ebis_mod[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Theta_ebis_on_mod"+ std::to_string(j);
		htitle = "Centre of mass angle for module " + std::to_string(j);
		htitle += " gated on EBIS;#theta_{CM} [deg];Counts per deg";
		Theta_ebis_on_mod[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Theta_ebis_off_mod"+ std::to_string(j);
		htitle = "Centre of mass angle for module " + std::to_string(j);
		htitle += " gated off EBIS;#theta_{CM} [deg];Counts per deg";
		Theta_ebis_off_mod[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Ex_ebis_mod" + std::to_string(j);
		htitle = "Excitation energy for module " + std::to_string(j);
		htitle += " gated by EBIS and off beam subtracted;Excitation energy [keV];Counts per mm per 20 keV";
		Ex_ebis_mod[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_ebis_on_mod" + std::to_string(j);
		htitle = "Excitation energy for module " + std::to_string(j);
		htitle += " gated on EBIS;Excitation energy [keV];Counts per 20 keV";
		Ex_ebis_on_mod[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_ebis_off_mod" + std::to_string(j);
		htitle = "Excitation energy for module " + std::to_string(j);
		htitle += " gated off EBIS;Excitation energy [keV];Counts per 20 keV";
		Ex_ebis_off_mod[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "E_vs_theta_ebis_mod"+ std::to_string(j);
		htitle = "Energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += " gated by EBIS and off beam subtracted;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_ebis_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "E_vs_theta_ebis_on_mod"+ std::to_string(j);
		htitle = "Energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += " gated on EBIS;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_ebis_on_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "E_vs_theta_ebis_off_mod"+ std::to_string(j);
		htitle = "Energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += " gated off EBIS;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_ebis_off_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "Ex_vs_theta_ebis_mod" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += " gated by EBIS and off beam subtracted;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_ebis_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_theta_ebis_on_mod" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += " gated on EBIS;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_ebis_on_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_theta_ebis_off_mod" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += " gated off EBIS;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_ebis_off_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_ebis_mod" + std::to_string(j);
		htitle = "Excitation energy vs. measured z for module " + std::to_string(j);
		htitle += " gated by EBIS and off beam subtracted;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_ebis_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_ebis_on_mod" + std::to_string(j);
		htitle = "Excitation energy vs. measured z  for module " + std::to_string(j);
		htitle += " gated on EBIS;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_ebis_on_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_ebis_off_mod" + std::to_string(j);
		htitle = "Excitation energy vs. measured z  for module " + std::to_string(j);
		htitle += " gated off EBIS;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_ebis_off_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	}

	// Recoil mode
	dirname = "RecoilMode";
	output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	hname = "E_vs_z_recoil";
	htitle = "Energy vs. z distance gated on recoils;z [mm];Energy [keV];Counts per mm per 20 keV";
	E_vs_z_recoil = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

	hname = "E_vs_z_recoilT";
	htitle = "Energy vs. z distance with a prompt time gate on recoils;z [mm];Energy [keV];Counts per mm per 20 keV";
	E_vs_z_recoilT = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

	hname = "E_vs_z_recoil_random";
	htitle = "Energy vs. z distance time-random gated on recoils;z [mm];Energy [keV];Counts per mm per 20 keV";
	E_vs_z_recoil_random = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

	hname = "E_vs_z_recoilT_random";
	htitle = "Energy vs. z distance with a random time gate on recoils;z [mm];Energy [keV];Counts per mm per 20 keV";
	E_vs_z_recoilT_random = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

	hname = "Theta_recoil";
	htitle = "Centre of mass angle gated on recoils;#theta_{CM} [deg];Counts per deg";
	Theta_recoil = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

	hname = "Theta_recoilT";
	htitle = "Centre of mass angle with a prompt time gate on recoils;#theta_{CM} [deg];Counts per deg";
	Theta_recoilT = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

	hname = "Theta_recoil_random";
	htitle = "Centre of mass angle time-random gated on recoils;#theta_{CM} [deg];Counts per deg";
	Theta_recoil_random = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

	hname = "Theta_recoilT_random";
	htitle = "Centre of mass angle with a random time gate on recoils;#theta_{CM} [deg];Counts per deg";
	Theta_recoilT_random = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

	hname = "Ex_recoil";
	htitle = "Excitation energy gated by recoils;Excitation energy [keV];Counts per 20 keV";
	Ex_recoil = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "Ex_recoilT";
	htitle = "Excitation energy with a prompt time gate on all recoils;Excitation energy [keV];Counts per 20 keV";
	Ex_recoilT = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "Ex_recoil_random";
	htitle = "Excitation energy time-random gated by recoils;Excitation energy [keV];Counts per 20 keV";
	Ex_recoil_random = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "Ex_recoilT_random";
	htitle = "Excitation energy with a random time gate on all recoils;Excitation energy [keV];Counts per 20 keV";
	Ex_recoilT_random = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "E_vs_theta_recoil";
	htitle = "Energy vs. centre of mass angle gated by recoils;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
	E_vs_theta_recoil = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

	hname = "E_vs_theta_recoilT";
	htitle = "Energy vs. centre of mass angle with a prompt time gate on all recoils;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
	E_vs_theta_recoilT = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

	hname = "E_vs_theta_recoil_random";
	htitle = "Energy vs. centre of mass angle time-random gated by recoils;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
	E_vs_theta_recoil_random = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

	hname = "E_vs_theta_recoilT_random";
	htitle = "Energy vs. centre of mass angle with a random time gate on all recoils;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
	E_vs_theta_recoilT_random = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

	hname = "Ex_vs_theta_recoil";
	htitle = "Excitation energy vs. centre of mass angle gated by recoils;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
	Ex_vs_theta_recoil = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "Ex_vs_theta_recoilT";
	htitle = "Excitation energy vs. centre of mass angle with a prompt time gate on all recoils;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
	Ex_vs_theta_recoilT = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "Ex_vs_theta_recoil_random";
	htitle = "Excitation energy vs. centre of mass angle time-random gated by recoils;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
	Ex_vs_theta_recoil_random = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "Ex_vs_theta_recoilT_random";
	htitle = "Excitation energy vs. centre of mass angle with a random time gate on all recoils;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
	Ex_vs_theta_recoilT_random = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "Ex_vs_z_recoil";
	htitle = "Excitation energy vs. measured z gated by recoils;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
	Ex_vs_z_recoil = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "Ex_vs_z_recoilT";
	htitle = "Excitation energy vs. measured z with a prompt time gate on all recoils;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
	Ex_vs_z_recoilT = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "Ex_vs_z_recoil_random";
	htitle = "Excitation energy vs. measured z time-random gated by recoils;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
	Ex_vs_z_recoil_random = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "Ex_vs_z_recoilT_random";
	htitle = "Excitation energy vs. measured z with a random time gate on all recoils;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
	Ex_vs_z_recoilT_random = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	// For each user cut
	E_vs_z_recoil_cut.resize( react->GetNumberOfEvsZCuts() );
	E_vs_z_recoilT_cut.resize( react->GetNumberOfEvsZCuts() );
	E_vs_z_recoil_random_cut.resize( react->GetNumberOfEvsZCuts() );
	E_vs_z_recoilT_random_cut.resize( react->GetNumberOfEvsZCuts() );
	Theta_recoil_cut.resize( react->GetNumberOfEvsZCuts() );
	Theta_recoilT_cut.resize( react->GetNumberOfEvsZCuts() );
	Theta_recoil_random_cut.resize( react->GetNumberOfEvsZCuts() );
	Theta_recoilT_random_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_recoil_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_recoilT_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_recoil_random_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_recoilT_random_cut.resize( react->GetNumberOfEvsZCuts() );
	E_vs_theta_recoil_cut.resize( react->GetNumberOfEvsZCuts() );
	E_vs_theta_recoilT_cut.resize( react->GetNumberOfEvsZCuts() );
	E_vs_theta_recoil_random_cut.resize( react->GetNumberOfEvsZCuts() );
	E_vs_theta_recoilT_random_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_vs_theta_recoil_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_vs_theta_recoilT_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_vs_theta_recoil_random_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_vs_theta_recoilT_random_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_vs_z_recoil_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_vs_z_recoilT_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_vs_z_recoil_random_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_vs_z_recoilT_random_cut.resize( react->GetNumberOfEvsZCuts() );

	for( unsigned int j = 0; j < react->GetNumberOfEvsZCuts(); ++j ) {

		dirname = "RecoilMode/cut_" + std::to_string(j);
		output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );

		hname = "E_vs_z_recoil_cut" + std::to_string(j);
		htitle = "Energy vs. z distance for user cut " + std::to_string(j);
		htitle += " gated on recoils;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_recoil_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "E_vs_z_recoilT_cut" + std::to_string(j);
		htitle = "Energy vs. z distance for user cut " + std::to_string(j);
		htitle += " with a prompt time gate on all recoils;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_recoilT_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "E_vs_z_recoil_random_cut" + std::to_string(j);
		htitle = "Energy vs. z distance for user cut " + std::to_string(j);
		htitle += " time-random gated on recoils;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_recoil_random_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "E_vs_z_recoilT_random_cut" + std::to_string(j);
		htitle = "Energy vs. z distance for user cut " + std::to_string(j);
		htitle += " with a random time gate on all recoils;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_recoilT_random_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "Theta_recoil_cut"+ std::to_string(j);
		htitle = "Centre of mass angle for user cut " + std::to_string(j);
		htitle += " gated on recoils;#theta_{CM} [deg];Counts per deg";
		Theta_recoil_cut[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Theta_recoilT_cut"+ std::to_string(j);
		htitle = "Centre of mass angle for user cut " + std::to_string(j);
		htitle += " with a prompt time gate on all recoils;#theta_{CM} [deg];Counts per deg";
		Theta_recoilT_cut[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Theta_recoil_random_cut"+ std::to_string(j);
		htitle = "Centre of mass angle for user cut " + std::to_string(j);
		htitle += " time-random gated on recoils;#theta_{CM} [deg];Counts per deg";
		Theta_recoil_random_cut[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Theta_recoilT_random_cut"+ std::to_string(j);
		htitle = "Centre of mass angle for user cut " + std::to_string(j);
		htitle += " with a random time gate on all recoil;#theta_{CM} [deg];Counts per deg";
		Theta_recoilT_random_cut[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Ex_recoil_cut" + std::to_string(j);
		htitle = "Excitation energy for user cut " + std::to_string(j);
		htitle += " gated by recoils;Excitation energy [keV];Counts per 20 keV";
		Ex_recoil_cut[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_recoilT_cut" + std::to_string(j);
		htitle = "Excitation energy for user cut " + std::to_string(j);
		htitle += " with a prompt time gate on all recoils;Excitation energy [keV];Counts per 20 keV";
		Ex_recoilT_cut[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_recoil_random_cut" + std::to_string(j);
		htitle = "Excitation energy for user cut " + std::to_string(j);
		htitle += " time-random gated by recoils;Excitation energy [keV];Counts per 20 keV";
		Ex_recoil_random_cut[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_recoilT_random_cut" + std::to_string(j);
		htitle = "Excitation energy for user cut " + std::to_string(j);
		htitle += " with a random time gate on all recoils;Excitation energy [keV];Counts per 20 keV";
		Ex_recoilT_random_cut[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "E_vs_theta_recoil_cut";
		htitle = "Energy vs. centre of mass angle for user cut " + std::to_string(j);
		htitle += " gated by recoils;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_recoil_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "E_vs_theta_recoilT_cut";
		htitle = "Energy vs. centre of mass angle for user cut " + std::to_string(j);
		htitle += " with a prompt time gate on all recoils;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_recoilT_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "E_vs_theta_recoil_random_cut";
		htitle = "Energy vs. centre of mass angle for user cut " + std::to_string(j);
		htitle += " time-random gated by recoils;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_recoil_random_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "E_vs_theta_recoilT_random_cut";
		htitle = "Energy vs. centre of mass angle for user cut " + std::to_string(j);
		htitle += " with a random time gate on all recoils;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_recoilT_random_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "Ex_vs_theta_recoil_cut" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for user cut " + std::to_string(j);
		htitle += " gated by recoils;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_recoil_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_theta_recoilT_cut" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for user cut " + std::to_string(j);
		htitle += " with a prompt time gate on all recoils;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_recoilT_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_theta_recoil_random_cut" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for user cut " + std::to_string(j);
		htitle += " time-random gated by recoils;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_recoil_random_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_theta_recoilT_random_cut" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for user cut " + std::to_string(j);
		htitle += " with a random time gate on all recoils;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_recoilT_random_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_recoil_cut" + std::to_string(j);
		htitle = "Excitation energy vs. measured z for user cut " + std::to_string(j);
		htitle += " gated by recoils;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_recoil_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_recoilT_cut" + std::to_string(j);
		htitle = "Excitation energy vs. measured z for user cut " + std::to_string(j);
		htitle += " with a prompt time gate on all recoils;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_recoilT_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_recoil_random_cut" + std::to_string(j);
		htitle = "Excitation energy vs. measured z for user cut " + std::to_string(j);
		htitle += " time-random gated by recoils;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_recoil_random_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_recoilT_random_cut" + std::to_string(j);
		htitle = "Excitation energy vs. measured z for user cut " + std::to_string(j);
		htitle += " with a random time gate on all recoils;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_recoilT_random_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	} // Array

	// For each array module
	E_vs_z_recoil_mod.resize( set->GetNumberOfArrayModules() );
	E_vs_z_recoilT_mod.resize( set->GetNumberOfArrayModules() );
	E_vs_z_recoil_random_mod.resize( set->GetNumberOfArrayModules() );
	E_vs_z_recoilT_random_mod.resize( set->GetNumberOfArrayModules() );
	Theta_recoil_mod.resize( set->GetNumberOfArrayModules() );
	Theta_recoilT_mod.resize( set->GetNumberOfArrayModules() );
	Theta_recoil_random_mod.resize( set->GetNumberOfArrayModules() );
	Theta_recoilT_random_mod.resize( set->GetNumberOfArrayModules() );
	Ex_recoil_mod.resize( set->GetNumberOfArrayModules() );
	Ex_recoilT_mod.resize( set->GetNumberOfArrayModules() );
	Ex_recoil_random_mod.resize( set->GetNumberOfArrayModules() );
	Ex_recoilT_random_mod.resize( set->GetNumberOfArrayModules() );
	E_vs_theta_recoil_mod.resize( set->GetNumberOfArrayModules() );
	E_vs_theta_recoilT_mod.resize( set->GetNumberOfArrayModules() );
	E_vs_theta_recoil_random_mod.resize( set->GetNumberOfArrayModules() );
	E_vs_theta_recoilT_random_mod.resize( set->GetNumberOfArrayModules() );
	Ex_vs_theta_recoil_mod.resize( set->GetNumberOfArrayModules() );
	Ex_vs_theta_recoilT_mod.resize( set->GetNumberOfArrayModules() );
	Ex_vs_theta_recoil_random_mod.resize( set->GetNumberOfArrayModules() );
	Ex_vs_theta_recoilT_random_mod.resize( set->GetNumberOfArrayModules() );
	Ex_vs_z_recoil_mod.resize( set->GetNumberOfArrayModules() );
	Ex_vs_z_recoilT_mod.resize( set->GetNumberOfArrayModules() );
	Ex_vs_z_recoil_random_mod.resize( set->GetNumberOfArrayModules() );
	Ex_vs_z_recoilT_random_mod.resize( set->GetNumberOfArrayModules() );

	for( unsigned int j = 0; j < set->GetNumberOfArrayModules(); ++j ) {

		dirname = "RecoilMode/module_" + std::to_string(j);
		output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );

		hname = "E_vs_z_recoil_mod" + std::to_string(j);
		htitle = "Energy vs. z distance for module " + std::to_string(j);
		htitle += " gated on recoils;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_recoil_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "E_vs_z_recoilT_mod" + std::to_string(j);
		htitle = "Energy vs. z distance for module " + std::to_string(j);
		htitle += " with a prompt time gate on all recoils;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_recoilT_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "E_vs_z_recoil_random_mod" + std::to_string(j);
		htitle = "Energy vs. z distance for module " + std::to_string(j);
		htitle += " time-random gated on recoils;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_recoil_random_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "E_vs_z_recoilT_random_mod" + std::to_string(j);
		htitle = "Energy vs. z distance for module " + std::to_string(j);
		htitle += " with a random time gate on all recoils;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_recoilT_random_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "Theta_recoil_mod"+ std::to_string(j);
		htitle = "Centre of mass angle for module " + std::to_string(j);
		htitle += " gated on recoils;#theta_{CM} [deg];Counts per deg";
		Theta_recoil_mod[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Theta_recoilT_mod"+ std::to_string(j);
		htitle = "Centre of mass angle for module " + std::to_string(j);
		htitle += " with a prompt time gate on all recoils;#theta_{CM} [deg];Counts per deg";
		Theta_recoilT_mod[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Theta_recoil_random_mod"+ std::to_string(j);
		htitle = "Centre of mass angle for module " + std::to_string(j);
		htitle += " time-random gated on recoils;#theta_{CM} [deg];Counts per deg";
		Theta_recoil_random_mod[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Theta_recoilT_random_mod"+ std::to_string(j);
		htitle = "Centre of mass angle for module " + std::to_string(j);
		htitle += " with a random time gate on all recoils;#theta_{CM} [deg];Counts per deg";
		Theta_recoilT_random_mod[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Ex_recoil_mod" + std::to_string(j);
		htitle = "Excitation energy for module " + std::to_string(j);
		htitle += " gated by recoils;Excitation energy [keV];Counts per 20 keV";
		Ex_recoil_mod[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_recoilT_mod" + std::to_string(j);
		htitle = "Excitation energy for module " + std::to_string(j);
		htitle += " with a prompt time gate on all recoils;Excitation energy [keV];Counts per 20 keV";
		Ex_recoilT_mod[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_recoil_random_mod" + std::to_string(j);
		htitle = "Excitation energy for module " + std::to_string(j);
		htitle += " time-random gated by recoils;Excitation energy [keV];Counts per 20 keV";
		Ex_recoil_random_mod[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_recoilT_random_mod" + std::to_string(j);
		htitle = "Excitation energy for module " + std::to_string(j);
		htitle += " with a random time gate on all recoils;Excitation energy [keV];Counts per 20 keV";
		Ex_recoilT_random_mod[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "E_vs_theta_recoil_mod"+ std::to_string(j);
		htitle = "Energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += " gated by recoils;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_recoil_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "E_vs_theta_recoilT_mod"+ std::to_string(j);
		htitle = "Energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += " with a prompt time gate on all recoils;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_recoilT_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "E_vs_theta_recoil_random_mod"+ std::to_string(j);
		htitle = "Energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += " time-random gated by recoils;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_recoil_random_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "E_vs_theta_recoilT_random_mod"+ std::to_string(j);
		htitle = "Energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += " with a random time gate on all recoils;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_recoilT_random_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "Ex_vs_theta_recoil_mod" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += " gated by recoils;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_recoil_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_theta_recoilT_mod" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += " with a prompt time gate on all recoils;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_recoilT_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_theta_recoil_random_mod" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += " time-random gated by recoils;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_recoil_random_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_theta_recoilT_random_mod" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for module " + std::to_string(j);
		htitle += " with a random time gate on all recoils;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_recoilT_random_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_recoil_mod" + std::to_string(j);
		htitle = "Excitation energy vs. measured z for module " + std::to_string(j);
		htitle += " gated by recoils;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_recoil_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_recoilT_mod" + std::to_string(j);
		htitle = "Excitation energy vs. measured z for module " + std::to_string(j);
		htitle += " with a prompt time gate on all recoils;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_recoilT_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_recoil_random_mod" + std::to_string(j);
		htitle = "Excitation energy vs. measured z for module " + std::to_string(j);
		htitle += " time-random gated by recoils;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_recoil_random_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_recoilT_random_mod" + std::to_string(j);
		htitle = "Excitation energy vs. measured z for module " + std::to_string(j);
		htitle += " with a random time gate on all recoils;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_recoilT_random_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	} // Array

	// Fission mode
	if( react->IsFission() && set->GetNumberOfCDLayers() > 0 ) {

		dirname = "FissionMode";
		output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );

		hname = "E_vs_z_fission";
		htitle = "Energy vs. z distance gated on fission fragments;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_fission = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "E_vs_z_fissionT";
		htitle = "Energy vs. z distance with a prompt time gate on fission fragments;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_fissionT = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "E_vs_z_fission_gamma";
		htitle = "Energy vs. z distance gated on fission fragments with a coincident gamma-ray in the energy gate;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_fission_gamma = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "E_vs_z_fission_random";
		htitle = "Energy vs. z distance time-random gated on fission fragments;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_fission_random = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "E_vs_z_fissionT_random";
		htitle = "Energy vs. z distance with a random time gate on fission fragments;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_fissionT_random = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "E_vs_z_fission_gamma_random";
		htitle = "Energy vs. z distance time-random gated on fission fragments with a coincident gamma-ray in the energy gate;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_fission_gamma_random = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "Theta_fission";
		htitle = "Centre of mass angle gated on fission fragments;#theta_{CM} [deg];Counts per deg";
		Theta_fission = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Theta_fissionT";
		htitle = "Centre of mass angle with a prompt time gate on fission fragments;#theta_{CM} [deg];Counts per deg";
		Theta_fissionT = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Theta_fission_random";
		htitle = "Centre of mass angle time-random gated on fission fragments;#theta_{CM} [deg];Counts per deg";
		Theta_fission_random = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Theta_fissionT_random";
		htitle = "Centre of mass angle with a random time gate on fission fragments;#theta_{CM} [deg];Counts per deg";
		Theta_fissionT_random = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Ex_fission";
		htitle = "Excitation energy gated by fission fragments;Excitation energy [keV];Counts per 20 keV";
		Ex_fission = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_fissionT";
		htitle = "Excitation energy with a prompt time gate on all fission fragments;Excitation energy [keV];Counts per 20 keV";
		Ex_fissionT = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_fission_gamma";
		htitle = "Excitation energy gated by fission fragments with a coincident gamma-ray in the energy gate;Excitation energy [keV];Counts per 20 keV";
		Ex_fission_gamma = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_fission_1FF";
		htitle = "Excitation energy gated by 1 fission fragment;Excitation energy [keV];Counts per 20 keV";
		Ex_fission_1FF = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_fission_1FF_gamma";
		htitle = "Excitation energy gated by 1 fission fragment and (any) gamma;Excitation energy [keV];Counts per 20 keV";
		Ex_fission_1FF_gamma = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_fission_2FF";
		htitle = "Excitation energy gated by 2 fission fragments 180 apart;Excitation energy [keV];Counts per 20 keV";
		Ex_fission_2FF = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_fission_random";
		htitle = "Excitation energy time-random gated by fission fragments;Excitation energy [keV];Counts per 20 keV";
		Ex_fission_random = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_fissionT_random";
		htitle = "Excitation energy with a random time gate on all fission fragments;Excitation energy [keV];Counts per 20 keV";
		Ex_fissionT_random = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_fission_gamma_random";
		htitle = "Excitation energy time-random gated by fission fragments with a coincident gamma-ray in the energy gate;Excitation energy [keV];Counts per 20 keV";
		Ex_fission_gamma_random = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "E_vs_theta_fission";
		htitle = "Energy vs. centre of mass angle gated by fission fragments;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_fission = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "E_vs_theta_fissionT";
		htitle = "Energy vs. centre of mass angle with a prompt time gate on all fission fragments;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_fissionT = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "E_vs_theta_fission_random";
		htitle = "Energy vs. centre of mass angle time-random gated by fission fragments;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_fission_random = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "E_vs_theta_fissionT_random";
		htitle = "Energy vs. centre of mass angle with a random time gate on all fission fragments;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_fissionT_random = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "Ex_vs_theta_fission";
		htitle = "Excitation energy vs. centre of mass angle gated by fission fragments;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_fission = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_theta_fissionT";
		htitle = "Excitation energy vs. centre of mass angle with a prompt time gate on all fission fragments;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_fissionT = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_theta_fission_random";
		htitle = "Excitation energy vs. centre of mass angle time-random gated by fission fragments;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_fission_random = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_theta_fissionT_random";
		htitle = "Excitation energy vs. centre of mass angle with a random time gate on all fission fragments;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_fissionT_random = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_fission";
		htitle = "Excitation energy vs. measured z gated by fission fragments;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_fission = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_fissionT";
		htitle = "Excitation energy vs. measured z with a prompt time gate on all fission fragments;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_fissionT = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_fission_random";
		htitle = "Excitation energy vs. measured z time-random gated by fission fragments;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_fission_random = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_fissionT_random";
		htitle = "Excitation energy vs. measured z with a random time gate on all fission fragments;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_fissionT_random = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		// For each user cut
		E_vs_z_fission_cut.resize( react->GetNumberOfEvsZCuts() );
		E_vs_z_fissionT_cut.resize( react->GetNumberOfEvsZCuts() );
		E_vs_z_fission_random_cut.resize( react->GetNumberOfEvsZCuts() );
		E_vs_z_fissionT_random_cut.resize( react->GetNumberOfEvsZCuts() );
		Theta_fission_cut.resize( react->GetNumberOfEvsZCuts() );
		Theta_fissionT_cut.resize( react->GetNumberOfEvsZCuts() );
		Theta_fission_random_cut.resize( react->GetNumberOfEvsZCuts() );
		Theta_fissionT_random_cut.resize( react->GetNumberOfEvsZCuts() );
		Ex_fission_cut.resize( react->GetNumberOfEvsZCuts() );
		Ex_fissionT_cut.resize( react->GetNumberOfEvsZCuts() );
		Ex_fission_random_cut.resize( react->GetNumberOfEvsZCuts() );
		Ex_fissionT_random_cut.resize( react->GetNumberOfEvsZCuts() );
		E_vs_theta_fission_cut.resize( react->GetNumberOfEvsZCuts() );
		E_vs_theta_fissionT_cut.resize( react->GetNumberOfEvsZCuts() );
		E_vs_theta_fission_random_cut.resize( react->GetNumberOfEvsZCuts() );
		E_vs_theta_fissionT_random_cut.resize( react->GetNumberOfEvsZCuts() );
		Ex_vs_theta_fission_cut.resize( react->GetNumberOfEvsZCuts() );
		Ex_vs_theta_fissionT_cut.resize( react->GetNumberOfEvsZCuts() );
		Ex_vs_theta_fission_random_cut.resize( react->GetNumberOfEvsZCuts() );
		Ex_vs_theta_fissionT_random_cut.resize( react->GetNumberOfEvsZCuts() );
		Ex_vs_z_fission_cut.resize( react->GetNumberOfEvsZCuts() );
		Ex_vs_z_fissionT_cut.resize( react->GetNumberOfEvsZCuts() );
		Ex_vs_z_fission_random_cut.resize( react->GetNumberOfEvsZCuts() );
		Ex_vs_z_fissionT_random_cut.resize( react->GetNumberOfEvsZCuts() );
		for( unsigned int j = 0; j < react->GetNumberOfEvsZCuts(); ++j ) {

			dirname = "FissionMode/cut_" + std::to_string(j);
			output_file->mkdir( dirname.data() );
			output_file->cd( dirname.data() );

			hname = "E_vs_z_fission_cut" + std::to_string(j);
			htitle = "Energy vs. z distance for user cut " + std::to_string(j);
			htitle += " gated on fission fragments;z [mm];Energy [keV];Counts per mm per 20 keV";
			E_vs_z_fission_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

			hname = "E_vs_z_fissionT_cut" + std::to_string(j);
			htitle = "Energy vs. z distance for user cut " + std::to_string(j);
			htitle += " with a prompt time gate on all fission fragments;z [mm];Energy [keV];Counts per mm per 20 keV";
			E_vs_z_fissionT_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

			hname = "E_vs_z_fission_random_cut" + std::to_string(j);
			htitle = "Energy vs. z distance for user cut " + std::to_string(j);
			htitle += " time-random gated on fission fragments;z [mm];Energy [keV];Counts per mm per 20 keV";
			E_vs_z_fission_random_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

			hname = "E_vs_z_fissionT_random_cut" + std::to_string(j);
			htitle = "Energy vs. z distance for user cut " + std::to_string(j);
			htitle += " with a random time gate on all fission fragments;z [mm];Energy [keV];Counts per mm per 20 keV";
			E_vs_z_fissionT_random_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

			hname = "Theta_fission_cut"+ std::to_string(j);
			htitle = "Centre of mass angle for user cut " + std::to_string(j);
			htitle += " gated on fission fragments;#theta_{CM} [deg];Counts per deg";
			Theta_fission_cut[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

			hname = "Theta_fissionT_cut"+ std::to_string(j);
			htitle = "Centre of mass angle for user cut " + std::to_string(j);
			htitle += " with a prompt time gate on all fission fragments;#theta_{CM} [deg];Counts per deg";
			Theta_fissionT_cut[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

			hname = "Theta_fission_random_cut"+ std::to_string(j);
			htitle = "Centre of mass angle for user cut " + std::to_string(j);
			htitle += " time-random gated on fission fragments;#theta_{CM} [deg];Counts per deg";
			Theta_fission_random_cut[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

			hname = "Theta_fissionT_random_cut"+ std::to_string(j);
			htitle = "Centre of mass angle for user cut " + std::to_string(j);
			htitle += " with a random time gate on all fission;#theta_{CM} [deg];Counts per deg";
			Theta_fissionT_random_cut[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

			hname = "Ex_fission_cut" + std::to_string(j);
			htitle = "Excitation energy for user cut " + std::to_string(j);
			htitle += " gated by fission fragments;Excitation energy [keV];Counts per 20 keV";
			Ex_fission_cut[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "Ex_fissionT_cut" + std::to_string(j);
			htitle = "Excitation energy for user cut " + std::to_string(j);
			htitle += " with a prompt time gate on all fission fragments;Excitation energy [keV];Counts per 20 keV";
			Ex_fissionT_cut[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "Ex_fission_random_cut" + std::to_string(j);
			htitle = "Excitation energy for user cut " + std::to_string(j);
			htitle += " time-random gated by fission fragments;Excitation energy [keV];Counts per 20 keV";
			Ex_fission_random_cut[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "Ex_fissionT_random_cut" + std::to_string(j);
			htitle = "Excitation energy for user cut " + std::to_string(j);
			htitle += " with a random time gate on all fission fragments;Excitation energy [keV];Counts per 20 keV";
			Ex_fissionT_random_cut[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "E_vs_theta_fission_cut";
			htitle = "Energy vs. centre of mass angle for user cut " + std::to_string(j);
			htitle += " gated by fission fragments;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
			E_vs_theta_fission_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

			hname = "E_vs_theta_fissionT_cut";
			htitle = "Energy vs. centre of mass angle for user cut " + std::to_string(j);
			htitle += " with a prompt time gate on all fission fragments;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
			E_vs_theta_fissionT_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

			hname = "E_vs_theta_fission_random_cut";
			htitle = "Energy vs. centre of mass angle for user cut " + std::to_string(j);
			htitle += " time-random gated by fission fragments;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
			E_vs_theta_fission_random_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

			hname = "E_vs_theta_fissionT_random_cut";
			htitle = "Energy vs. centre of mass angle for user cut " + std::to_string(j);
			htitle += " with a random time gate on all fission fragments;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
			E_vs_theta_fissionT_random_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

			hname = "Ex_vs_theta_fission_cut" + std::to_string(j);
			htitle = "Excitation energy vs. centre of mass angle for user cut " + std::to_string(j);
			htitle += " gated by fission fragments;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
			Ex_vs_theta_fission_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "Ex_vs_theta_fissionT_cut" + std::to_string(j);
			htitle = "Excitation energy vs. centre of mass angle for user cut " + std::to_string(j);
			htitle += " with a prompt time gate on all fission fragments;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
			Ex_vs_theta_fissionT_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "Ex_vs_theta_fission_random_cut" + std::to_string(j);
			htitle = "Excitation energy vs. centre of mass angle for user cut " + std::to_string(j);
			htitle += " time-random gated by fission fragments;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
			Ex_vs_theta_fission_random_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "Ex_vs_theta_fissionT_random_cut" + std::to_string(j);
			htitle = "Excitation energy vs. centre of mass angle for user cut " + std::to_string(j);
			htitle += " with a random time gate on all fission fragments;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
			Ex_vs_theta_fissionT_random_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "Ex_vs_z_fission_cut" + std::to_string(j);
			htitle = "Excitation energy vs. measured z for user cut " + std::to_string(j);
			htitle += " gated by fission fragments;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
			Ex_vs_z_fission_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "Ex_vs_z_fissionT_cut" + std::to_string(j);
			htitle = "Excitation energy vs. measured z for user cut " + std::to_string(j);
			htitle += " with a prompt time gate on all fission fragments;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
			Ex_vs_z_fissionT_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "Ex_vs_z_fission_random_cut" + std::to_string(j);
			htitle = "Excitation energy vs. measured z for user cut " + std::to_string(j);
			htitle += " time-random gated by fission fragments;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
			Ex_vs_z_fission_random_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "Ex_vs_z_fissionT_random_cut" + std::to_string(j);
			htitle = "Excitation energy vs. measured z for user cut " + std::to_string(j);
			htitle += " with a random time gate on all fission fragments;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
			Ex_vs_z_fissionT_random_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		} // Array

		// For each array module
		E_vs_z_fission_mod.resize( set->GetNumberOfArrayModules() );
		E_vs_z_fissionT_mod.resize( set->GetNumberOfArrayModules() );
		E_vs_z_fission_random_mod.resize( set->GetNumberOfArrayModules() );
		E_vs_z_fissionT_random_mod.resize( set->GetNumberOfArrayModules() );
		Theta_fission_mod.resize( set->GetNumberOfArrayModules() );
		Theta_fissionT_mod.resize( set->GetNumberOfArrayModules() );
		Theta_fission_random_mod.resize( set->GetNumberOfArrayModules() );
		Theta_fissionT_random_mod.resize( set->GetNumberOfArrayModules() );
		Ex_fission_mod.resize( set->GetNumberOfArrayModules() );
		Ex_fissionT_mod.resize( set->GetNumberOfArrayModules() );
		Ex_fission_random_mod.resize( set->GetNumberOfArrayModules() );
		Ex_fissionT_random_mod.resize( set->GetNumberOfArrayModules() );
		E_vs_theta_fission_mod.resize( set->GetNumberOfArrayModules() );
		E_vs_theta_fissionT_mod.resize( set->GetNumberOfArrayModules() );
		E_vs_theta_fission_random_mod.resize( set->GetNumberOfArrayModules() );
		E_vs_theta_fissionT_random_mod.resize( set->GetNumberOfArrayModules() );
		Ex_vs_theta_fission_mod.resize( set->GetNumberOfArrayModules() );
		Ex_vs_theta_fissionT_mod.resize( set->GetNumberOfArrayModules() );
		Ex_vs_theta_fission_random_mod.resize( set->GetNumberOfArrayModules() );
		Ex_vs_theta_fissionT_random_mod.resize( set->GetNumberOfArrayModules() );
		Ex_vs_z_fission_mod.resize( set->GetNumberOfArrayModules() );
		Ex_vs_z_fissionT_mod.resize( set->GetNumberOfArrayModules() );
		Ex_vs_z_fission_random_mod.resize( set->GetNumberOfArrayModules() );
		Ex_vs_z_fissionT_random_mod.resize( set->GetNumberOfArrayModules() );
		for( unsigned int j = 0; j < set->GetNumberOfArrayModules(); ++j ) {

			dirname = "FissionMode/module_" + std::to_string(j);
			output_file->mkdir( dirname.data() );
			output_file->cd( dirname.data() );

			hname = "E_vs_z_fission_mod" + std::to_string(j);
			htitle = "Energy vs. z distance for module " + std::to_string(j);
			htitle += " gated on fission fragments;z [mm];Energy [keV];Counts per mm per 20 keV";
			E_vs_z_fission_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

			hname = "E_vs_z_fissionT_mod" + std::to_string(j);
			htitle = "Energy vs. z distance for module " + std::to_string(j);
			htitle += " with a prompt time gate on all fission fragments;z [mm];Energy [keV];Counts per mm per 20 keV";
			E_vs_z_fissionT_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

			hname = "E_vs_z_fission_random_mod" + std::to_string(j);
			htitle = "Energy vs. z distance for module " + std::to_string(j);
			htitle += " time-random gated on fission fragments;z [mm];Energy [keV];Counts per mm per 20 keV";
			E_vs_z_fission_random_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

			hname = "E_vs_z_fissionT_random_mod" + std::to_string(j);
			htitle = "Energy vs. z distance for module " + std::to_string(j);
			htitle += " with a random time gate on all fission fragments;z [mm];Energy [keV];Counts per mm per 20 keV";
			E_vs_z_fissionT_random_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

			hname = "Theta_fission_mod"+ std::to_string(j);
			htitle = "Centre of mass angle for module " + std::to_string(j);
			htitle += " gated on fission fragments;#theta_{CM} [deg];Counts per deg";
			Theta_fission_mod[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

			hname = "Theta_fissionT_mod"+ std::to_string(j);
			htitle = "Centre of mass angle for module " + std::to_string(j);
			htitle += " with a prompt time gate on all fission fragments;#theta_{CM} [deg];Counts per deg";
			Theta_fissionT_mod[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

			hname = "Theta_fission_random_mod"+ std::to_string(j);
			htitle = "Centre of mass angle for module " + std::to_string(j);
			htitle += " time-random gated on fission fragments;#theta_{CM} [deg];Counts per deg";
			Theta_fission_random_mod[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

			hname = "Theta_fissionT_random_mod"+ std::to_string(j);
			htitle = "Centre of mass angle for module " + std::to_string(j);
			htitle += " with a random time gate on all fission fragments;#theta_{CM} [deg];Counts per deg";
			Theta_fissionT_random_mod[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

			hname = "Ex_fission_mod" + std::to_string(j);
			htitle = "Excitation energy for module " + std::to_string(j);
			htitle += " gated by fission fragments;Excitation energy [keV];Counts per 20 keV";
			Ex_fission_mod[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "Ex_fissionT_mod" + std::to_string(j);
			htitle = "Excitation energy for module " + std::to_string(j);
			htitle += " with a prompt time gate on all fission fragments;Excitation energy [keV];Counts per 20 keV";
			Ex_fissionT_mod[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "Ex_fission_random_mod" + std::to_string(j);
			htitle = "Excitation energy for module " + std::to_string(j);
			htitle += " time-random gated by fission fragments;Excitation energy [keV];Counts per 20 keV";
			Ex_fission_random_mod[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "Ex_fissionT_random_mod" + std::to_string(j);
			htitle = "Excitation energy for module " + std::to_string(j);
			htitle += " with a random time gate on all fission fragments;Excitation energy [keV];Counts per 20 keV";
			Ex_fissionT_random_mod[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "E_vs_theta_fission_mod"+ std::to_string(j);
			htitle = "Energy vs. centre of mass angle for module " + std::to_string(j);
			htitle += " gated by fission fragments;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
			E_vs_theta_fission_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

			hname = "E_vs_theta_fissionT_mod"+ std::to_string(j);
			htitle = "Energy vs. centre of mass angle for module " + std::to_string(j);
			htitle += " with a prompt time gate on all fission fragments;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
			E_vs_theta_fissionT_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

			hname = "E_vs_theta_fission_random_mod"+ std::to_string(j);
			htitle = "Energy vs. centre of mass angle for module " + std::to_string(j);
			htitle += " time-random gated by fission fragments;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
			E_vs_theta_fission_random_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

			hname = "E_vs_theta_fissionT_random_mod"+ std::to_string(j);
			htitle = "Energy vs. centre of mass angle for module " + std::to_string(j);
			htitle += " with a random time gate on all fission fragments;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
			E_vs_theta_fissionT_random_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

			hname = "Ex_vs_theta_fission_mod" + std::to_string(j);
			htitle = "Excitation energy vs. centre of mass angle for module " + std::to_string(j);
			htitle += " gated by fission fragments;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
			Ex_vs_theta_fission_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "Ex_vs_theta_fissionT_mod" + std::to_string(j);
			htitle = "Excitation energy vs. centre of mass angle for module " + std::to_string(j);
			htitle += " with a prompt time gate on all fission fragments;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
			Ex_vs_theta_fissionT_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "Ex_vs_theta_fission_random_mod" + std::to_string(j);
			htitle = "Excitation energy vs. centre of mass angle for module " + std::to_string(j);
			htitle += " time-random gated by fission fragments;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
			Ex_vs_theta_fission_random_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "Ex_vs_theta_fissionT_random_mod" + std::to_string(j);
			htitle = "Excitation energy vs. centre of mass angle for module " + std::to_string(j);
			htitle += " with a random time gate on all fission fragments;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
			Ex_vs_theta_fissionT_random_mod[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "Ex_vs_z_fission_mod" + std::to_string(j);
			htitle = "Excitation energy vs. measured z for module " + std::to_string(j);
			htitle += " gated by fission fragments;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
			Ex_vs_z_fission_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "Ex_vs_z_fissionT_mod" + std::to_string(j);
			htitle = "Excitation energy vs. measured z for module " + std::to_string(j);
			htitle += " with a prompt time gate on all fission fragments;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
			Ex_vs_z_fissionT_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "Ex_vs_z_fission_random_mod" + std::to_string(j);
			htitle = "Excitation energy vs. measured z for module " + std::to_string(j);
			htitle += " time-random gated by fission fragments;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
			Ex_vs_z_fission_random_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

			hname = "Ex_vs_z_fissionT_random_mod" + std::to_string(j);
			htitle = "Excitation energy vs. measured z for module " + std::to_string(j);
			htitle += " with a random time gate on all fission fragments;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
			Ex_vs_z_fissionT_random_mod[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		} // Array

	} // fission mode


	// Gamma mode
	if( react->GammaRayHistsEnabled() ){

		dirname = "GammaRayMode";
		output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );

		hname = "E_vs_z_gamma";
		htitle = "Energy vs. z distance gated on gammas;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_gamma = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "E_vs_z_gammaT";
		htitle = "Energy vs. z distance with a prompt time gate on gammas;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_gammaT = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "E_vs_z_gamma_random";
		htitle = "Energy vs. z distance time-random gated on gammas;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_gamma_random = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "E_vs_z_gammaT_random";
		htitle = "Energy vs. z distance with a random time gate on gammas;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_gammaT_random = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "Theta_gamma";
		htitle = "Centre of mass angle gated on gammas;#theta_{CM} [deg];Counts per deg";
		Theta_gamma = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Theta_gammaT";
		htitle = "Centre of mass angle with a prompt time gate on gammas;#theta_{CM} [deg];Counts per deg";
		Theta_gammaT = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Theta_gamma_random";
		htitle = "Centre of mass angle time-random gated on gammas;#theta_{CM} [deg];Counts per deg";
		Theta_gamma_random = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Theta_gammaT_random";
		htitle = "Centre of mass angle with a random time gate on gammas;#theta_{CM} [deg];Counts per deg";
		Theta_gammaT_random = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Ex_gamma";
		htitle = "Excitation energy gated by gammas;Excitation energy [keV];Counts per 20 keV";
		Ex_gamma = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_gammaT";
		htitle = "Excitation energy with a prompt time gate on all gammas;Excitation energy [keV];Counts per 20 keV";
		Ex_gammaT = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_gamma_random";
		htitle = "Excitation energy time-random gated by gammas;Excitation energy [keV];Counts per 20 keV";
		Ex_gamma_random = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_gammaT_random";
		htitle = "Excitation energy with a random time gate on all gammas;Excitation energy [keV];Counts per 20 keV";
		Ex_gammaT_random = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "E_vs_theta_gamma";
		htitle = "Energy vs. centre of mass angle gated by gammas;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_gamma = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "E_vs_theta_gammaT";
		htitle = "Energy vs. centre of mass angle with a prompt time gate on all gammas;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_gammaT = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "E_vs_theta_gamma_random";
		htitle = "Energy vs. centre of mass angle time-random gated by gammas;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_gamma_random = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "E_vs_theta_gammaT_random";
		htitle = "Energy vs. centre of mass angle with a random time gate on all gammas;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_gammaT_random = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "Ex_vs_theta_gamma";
		htitle = "Excitation energy vs. centre of mass angle gated by gammas;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_gamma = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_theta_gammaT";
		htitle = "Excitation energy vs. centre of mass angle with a prompt time gate on all gammas;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_gammaT = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_theta_gamma_random";
		htitle = "Excitation energy vs. centre of mass angle time-random gated by gammas;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_gamma_random = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_theta_gammaT_random";
		htitle = "Excitation energy vs. centre of mass angle with a random time gate on all gammas;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_gammaT_random = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_gamma";
		htitle = "Excitation energy vs. measured z gated by gammas;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_gamma = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_gammaT";
		htitle = "Excitation energy vs. measured z with a prompt time gate on all gammas;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_gammaT = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_gamma_random";
		htitle = "Excitation energy vs. measured z time-random gated by gammas;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_gamma_random = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_gammaT_random";
		htitle = "Excitation energy vs. measured z with a random time gate on all gammas;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_gammaT_random = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	} // gamma mode


	// T1 mode
	dirname = "T1Mode";
	output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	hname = "E_vs_z_T1";
	htitle = "Energy vs. z distance with a time gate on T1 proton pulse;z [mm];Energy [keV];Counts per mm per 20 keV";
	E_vs_z_T1 = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

	hname = "Theta_T1";
	htitle = "Centre of mass angle with a time gate on T1 proton pulse;#theta_{CM} [deg];Counts per deg";
	Theta_T1 = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

	hname = "Ex_T1";
	htitle = "Excitation energy with a time gate on T1 proton pulse;Excitation energy [keV];Counts per 20 keV";
	Ex_T1 = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "E_vs_theta_T1";
	htitle = "Energy vs. centre of mass angle with a time gate on T1 proton pulse;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
	E_vs_theta_T1 = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

	hname = "Ex_vs_T1";
	htitle = "Excitation energy as a function of time since T1 proton pulse;Event time - T1 [ns];Excitation energy [keV];Counts per 20 keV";
	Ex_vs_T1 = new TH2F( hname.data(), htitle.data(), 1000, 0, 100e9, react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "Ex_vs_theta_T1";
	htitle = "Excitation energy vs. centre of mass angle with a time gate on T1 proton pulse;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
	Ex_vs_theta_T1 = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

	hname = "Ex_vs_z_T1";
	htitle = "Excitation energy vs. measured z with a time gate on T1 proton pulse;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
	Ex_vs_z_T1 = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	// For each user cut
	E_vs_z_T1_cut.resize( react->GetNumberOfEvsZCuts() );
	Theta_T1_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_T1_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_vs_T1_cut.resize( react->GetNumberOfEvsZCuts() );
	E_vs_theta_T1_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_vs_theta_T1_cut.resize( react->GetNumberOfEvsZCuts() );
	Ex_vs_z_T1_cut.resize( react->GetNumberOfEvsZCuts() );

	for( unsigned int j = 0; j < react->GetNumberOfEvsZCuts(); ++j ) {

		dirname = "T1Mode/cut_" + std::to_string(j);
		output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );

		hname = "E_vs_z_T1_cut" + std::to_string(j);
		htitle = "Energy vs. z distance for user cut " + std::to_string(j);
		htitle += " with a time gate on T1 proton pulse;z [mm];Energy [keV];Counts per mm per 20 keV";
		E_vs_z_T1_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

		hname = "Theta_T1_cut"+ std::to_string(j);
		htitle = "Centre of mass angle for user cut " + std::to_string(j);
		htitle += " with a time gate on T1 proton pulse;#theta_{CM} [deg];Counts per deg";
		Theta_T1_cut[j] = new TH1F( hname.data(), htitle.data(),  180, 0, 180.0 );

		hname = "Ex_T1_cut" + std::to_string(j);
		htitle = "Excitation energy for user cut " + std::to_string(j);
		htitle += " with a time gate on T1 proton pulse;Excitation energy [keV];Counts per 20 keV";
		Ex_T1_cut[j] = new TH1F( hname.data(), htitle.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "E_vs_theta_T1_cut";
		htitle = "Energy vs. centre of mass angle for user cut " + std::to_string(j);
		htitle += " with a time gate on T1 proton pulse;#theta_{CM} [deg];Energy [keV];Counts per deg per 20 keV";
		E_vs_theta_T1_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistElabBins(), react->HistElabMin(), react->HistElabMax()  );

		hname = "Ex_vs_T1_cut" + std::to_string(j);
		htitle = "Excitation energy as a function of time since T1 proton pulse;Event time - T1 [ns];Excitation energy [keV];Counts per 20 keV";
		Ex_vs_T1_cut[j] = new TH2F( hname.data(), htitle.data(), 1000, 0, 100e9, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_theta_T1_cut" + std::to_string(j);
		htitle = "Excitation energy vs. centre of mass angle for user cut " + std::to_string(j);
		htitle += " with a time gate on T1 proton pulse;#theta_{CM} [deg];Excitation energy [keV];Counts per deg per 20 keV";
		Ex_vs_theta_T1_cut[j] = new TH2F( hname.data(), htitle.data(), 180, 0, 180.0, react->HistExBins(), react->HistExMin(), react->HistExMax() );

		hname = "Ex_vs_z_T1_cut" + std::to_string(j);
		htitle = "Excitation energy vs. measured z for user cut " + std::to_string(j);
		htitle += " with a time gate on T1 proton pulse;z [mm];Excitation energy [keV];Counts per mm per 20 keV";
		Ex_vs_z_T1_cut[j] = new TH2F( hname.data(), htitle.data(), zbins.size()-1, zbins.data(), react->HistExBins(), react->HistExMin(), react->HistExMax() );

	} // Array


	// For timing
	dirname = "Timing";
	output_file->mkdir( dirname.data() );

	// For recoil sectors, but only if we are not doing fission
	if( !react->IsFission() ) {

		dirname = "RecoilDetector";
		output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );

		recoil_array_td.resize( set->GetNumberOfRecoilSectors() );
		recoil_elum_td.resize( set->GetNumberOfRecoilSectors() );
		recoil_EdE.resize( set->GetNumberOfRecoilSectors() );
		recoil_EdE_cut.resize( set->GetNumberOfRecoilSectors() );
		recoil_EdE_array.resize( set->GetNumberOfRecoilSectors() );
		recoil_bragg.resize( set->GetNumberOfRecoilSectors() );
		recoil_dE_vs_T1.resize( set->GetNumberOfRecoilSectors() );
		recoil_dE_eloss.resize( set->GetNumberOfRecoilSectors() );
		recoil_E_eloss.resize( set->GetNumberOfRecoilSectors() );

		// Loop over each recoil sector
		for( unsigned int i = 0; i < set->GetNumberOfRecoilSectors(); ++i ) {

			dirname = "RecoilDetector/sector_" + std::to_string(i);
			output_file->mkdir( dirname.data() );
			output_file->cd( dirname.data() );

			// Recoil energy plots
			hname = "recoil_EdE_sec" + std::to_string(i);
			htitle = "Recoil dE-E plot for sector " + std::to_string(i);
			htitle += " - singles;Rest energy, E [keV];Energy loss, dE [keV];Counts";
			recoil_EdE[i] = new TH2F( hname.data(), htitle.data(),
									 react->HistRecoilBins(), react->HistRecoilMin(), react->HistRecoilMax(),
									 react->HistRecoilBins(), react->HistRecoilMin(), react->HistRecoilMax() );

			hname = "recoil_EdE_cut_sec" + std::to_string(i);
			htitle = "Recoil dE-E plot for sector " + std::to_string(i);
			htitle += " - with energy cut;Rest energy, E [keV];Energy loss, dE [keV];Counts";
			recoil_EdE_cut[i] = new TH2F( hname.data(), htitle.data(),
										 react->HistRecoilBins(), react->HistRecoilMin(), react->HistRecoilMax(),
										 react->HistRecoilBins(), react->HistRecoilMin(), react->HistRecoilMax() );

			hname = "recoil_EdE_array_sec" + std::to_string(i);
			htitle = "Recoil dE-E plot for sector " + std::to_string(i);
			htitle += " - in coincidence with array;Rest energy, E [keV];Energy loss, dE [keV];Counts";
			recoil_EdE_array[i] = new TH2F( hname.data(), htitle.data(),
										   react->HistRecoilBins(), react->HistRecoilMin(), react->HistRecoilMax(),
										   react->HistRecoilBins(), react->HistRecoilMin(), react->HistRecoilMax() );

			hname = "recoil_bragg_sec" + std::to_string(i);
			htitle = "Recoil Bragg plot for sector " + std::to_string(i);
			htitle += ";Bragg ID;Energy loss, dE [keV];Counts";
			recoil_bragg[i] = new TH2F( hname.data(), htitle.data(),
									   set->GetNumberOfRecoilLayers(), -0.5, set->GetNumberOfRecoilLayers()-0.5,
									   react->HistRecoilBins(),react->HistRecoilMin(), react->HistRecoilMax() );

			hname = "recoil_dE_vs_T1_sec" + std::to_string(i);
			htitle = "Recoil dE plot versus T1 time for sector " + std::to_string(i);
			htitle += ";Time since T1 proton pulse [ns];Energy loss, dE [keV];Counts";
			recoil_dE_vs_T1[i] = new TH2F( hname.data(), htitle.data(),
										  5000, 0, 50e9,
										  react->HistRecoilBins(), react->HistRecoilMin(), react->HistRecoilMax() );

			hname = "recoil_dE_eloss_sec" + std::to_string(i);
			htitle = "Recoil dE energy loss for sector " + std::to_string(i);
			htitle += ";Energy loss, dE [keV];Counts";
			recoil_dE_eloss[i] = new TH1F( hname.data(), htitle.data(), react->HistRecoilBins(), react->HistRecoilMin(), react->HistRecoilMax() );

			hname = "recoil_E_eloss_sec" + std::to_string(i);
			htitle = "Recoil E energy loss for sector " + std::to_string(i);
			htitle += ";Energy loss, E [keV];Counts";
			recoil_E_eloss[i] = new TH1F( hname.data(), htitle.data(), react->HistRecoilBins(), react->HistRecoilMin(), react->HistRecoilMax() );

			// Timing plots
			output_file->cd( "Timing" );
			recoil_array_td[i].resize( set->GetNumberOfArrayModules() );
			recoil_elum_td[i].resize( set->GetNumberOfELUMSectors() );

			// For array modules
			for( unsigned int j = 0; j < set->GetNumberOfArrayModules(); ++j ) {

				hname = "td_recoil_array_sec" + std::to_string(i) + "_mod" + std::to_string(j);
				htitle = "Time difference between recoil sector " + std::to_string(i);
				htitle += " and array module " + std::to_string(j);
				htitle += ";#Deltat;Counts";
				recoil_array_td[i][j] = new TH1F( hname.data(), htitle.data(),
												 1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );

			}

			// For ELUM sectors
			for( unsigned int j = 0; j < set->GetNumberOfELUMSectors(); ++j ) {

				hname = "td_recoil_elum_sec" + std::to_string(i) + "_mod" + std::to_string(j);
				htitle = "Time difference between recoil sector " + std::to_string(i);
				htitle += " and ELUM sector " + std::to_string(j);
				htitle += ";#Deltat;Counts";
				recoil_elum_td[i][j] = new TH1F( hname.data(), htitle.data(),
												1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );

			}

		} // Recoils

		// Recoil-array time walk
		output_file->cd( "Timing" );
		recoil_array_tw_hit0 = new TH2F( "tw_recoil_array_hit0",
										"Time-walk histogram for array-recoil coincidences with hit bit false;#Deltat [ns];Array energy [keV];Counts",
										1000, -1.0*set->GetEventWindow(), 1.0*set->GetEventWindow(),
										react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );
		recoil_array_tw_hit1 = new TH2F( "tw_recoil_array_hit1",
										"Time-walk histogram for array-recoil coincidences with hit bit true;#Deltat [ns];Array energy [keV];Counts",
										1000, -1.0*set->GetEventWindow(), 1.0*set->GetEventWindow(),
										react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );
		recoil_array_tw_hit0_prof = new TProfile( "tw_recoil_array_hit0_prof", "Time-walk profile for recoil-array coincidences with hit bit false;Array energy;#Delta t", 2000, 0, 60000 );
		recoil_array_tw_hit1_prof = new TProfile( "tw_recoil_array_hit1_prof", "Time-walk profile for recoil-array coincidences with hit bit true;Array energy;#Delta t", 2000, 0, 60000 );

		recoil_array_tw_hit0_row.resize( set->GetNumberOfArrayModules() );
		recoil_array_tw_hit1_row.resize( set->GetNumberOfArrayModules() );

		// Loop over ISS modules
		for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {

			recoil_array_tw_hit0_row[i].resize( set->GetNumberOfArrayRows() );
			recoil_array_tw_hit1_row[i].resize( set->GetNumberOfArrayRows() );

			// Loop over rows of the array
			for( unsigned int j = 0; j < set->GetNumberOfArrayRows(); ++j ) {

				hname = "tw_recoil_array_hit0_mod_" + std::to_string(i) + "_row" + std::to_string(j);
				htitle = "Time-walk histogram for array-recoil coincidences (module ";
				htitle += std::to_string(i) + ", row " + std::to_string(j) + ") with hit bit false;Deltat [ns];Array energy [keV];Counts";
				recoil_array_tw_hit0_row[i][j] = new TH2F( hname.data(), htitle.data(), 1000, -1.0*set->GetEventWindow(), 1.0*set->GetEventWindow(),
														  react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

				hname = "tw_recoil_array_hit1_mod_" + std::to_string(i) + "_row" + std::to_string(j);
				htitle = "Time-walk histogram for array-recoil coincidences (module ";
				htitle += std::to_string(i) + ", row " + std::to_string(j) + ") with hit bit true;Deltat [ns];Array energy [keV];Counts";
				recoil_array_tw_hit1_row[i][j] = new TH2F( hname.data(), htitle.data(), 1000, -1.0*set->GetEventWindow(), 1.0*set->GetEventWindow(),
														  react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

			}

		}

	} // recoils: not fission

	// Generic recoils
	output_file->cd( "Timing" );
	recoil_lume_td.resize( set->GetNumberOfLUMEDetectors() );

	// For LUME sectors
	for( unsigned int j = 0; j < set->GetNumberOfLUMEDetectors(); ++j ) {

		hname = "td_recoil_lume_det" + std::to_string(j);
		htitle = "Time difference between recoils in recoil/CD detector ";
		htitle += " and LUME detector " + std::to_string(j);
		htitle += ";#Deltat;Counts";
		recoil_lume_td[j] = new TH1F( hname.data(), htitle.data(),
									 1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );

	}


	// For fission fragment sectors
	if( react->IsFission() && set->GetNumberOfCDLayers() > 0 ) {

		dirname = "FissionDetector";
		output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );

		// Fission fragment energy plots
		hname = "fission_EdE";
		htitle = "fission dE-E plot";
		htitle += " - singles;Rest energy, E [keV];Energy loss, dE [keV];Counts";
		fission_EdE = new TH2F( hname.data(), htitle.data(),
							   react->HistFissionBins(), react->HistFissionMin(), react->HistFissionMax(),
							   react->HistFissionBins(), react->HistFissionMin(), react->HistFissionMax() );

		hname = "fission_EdE_cutH";
		htitle = "fission dE-E plot";
		htitle += " - with energy cut on the heavy fragment;Rest energy, E [keV];Energy loss, dE [keV];Counts";
		fission_EdE_cutH = new TH2F( hname.data(), htitle.data(),
									react->HistFissionBins(), react->HistFissionMin(), react->HistFissionMax(),
									react->HistFissionBins(), react->HistFissionMin(), react->HistFissionMax() );

		hname = "fission_EdE_cutL";
		htitle = "fission dE-E plot";
		htitle += " - with energy cut on the light fragment;Rest energy, E [keV];Energy loss, dE [keV];Counts";
		fission_EdE_cutL = new TH2F( hname.data(), htitle.data(),
									react->HistFissionBins(), react->HistFissionMin(), react->HistFissionMax(),
									react->HistFissionBins(), react->HistFissionMin(), react->HistFissionMax() );

		hname = "fission_EdE_array";
		htitle = "fission dE-E plot";
		htitle += " - in coincidence with array;Rest energy, E [keV];Energy loss, dE [keV];Counts";
		fission_EdE_array = new TH2F( hname.data(), htitle.data(),
									 react->HistFissionBins(), react->HistFissionMin(), react->HistFissionMax(),
									 react->HistFissionBins(), react->HistFissionMin(), react->HistFissionMax() );

		hname = "fission_bragg";
		htitle = "fission Bragg plot";
		htitle += ";Bragg ID;Energy loss, dE [keV];Counts";
		fission_bragg = new TH2F( hname.data(), htitle.data(),
								 set->GetNumberOfCDLayers(), -0.5, set->GetNumberOfCDLayers()-0.5,
								 react->HistFissionBins(), react->HistFissionMin(), react->HistFissionMax() );

		hname = "fission_dE_vs_T1";
		htitle = "fission dE plot versus T1 time";
		htitle += ";Time since T1 proton pulse [ns];Energy loss, dE [keV];Counts";
		fission_dE_vs_T1 = new TH2F( hname.data(), htitle.data(),
									5000, 0, 50e9,
									react->HistFissionBins(), react->HistFissionMin(), react->HistFissionMax() );

		hname = "fission_dE_eloss";
		htitle = "fission dE energy loss";
		htitle += ";Energy loss, dE [keV];Counts";
		fission_dE_eloss = new TH1F( hname.data(), htitle.data(),
									react->HistFissionBins(), react->HistFissionMin(), react->HistFissionMax() );

		hname = "fission_E_eloss";
		htitle = "fission E energy loss";
		htitle += ";Energy loss, E [keV];Counts";
		fission_E_eloss = new TH1F( hname.data(), htitle.data(),
								   react->HistFissionBins(), react->HistFissionMin(), react->HistFissionMax() );

		hname = "fission_fission_dEdE";
		htitle = "fission-fission dE-dE plot";
		htitle += " - coincidence with each other;Fragment 1 dE [keV];Fragment 2 dE [keV];Counts";
		fission_fission_dEdE = new TH2F( hname.data(), htitle.data(),
										react->HistFissionBins(), react->HistFissionMin(), react->HistFissionMax(),
										react->HistFissionBins(), react->HistFissionMin(), react->HistFissionMax() );

		hname = "fission_fission_secsec";
		htitle = "fission-fission sector-sector plot";
		htitle += " - coincidence with each other, random subtracted;Fragment 1 sector [keV];Fragment 2 sector [keV];Counts";
		fission_fission_secsec = new TH2F( hname.data(), htitle.data(),
										set->GetNumberOfCDSectors(), -0.5, set->GetNumberOfCDSectors()-0.5,
										set->GetNumberOfCDSectors(), -0.5, set->GetNumberOfCDSectors()-0.5 );

		hname = "fission_fission_ringring";
		htitle = "fission-fission ring-ring plot";
		htitle += " - coincidence with each other, random subtracted;Fragment 1 ring [keV];Fragment 2 ring [keV];Counts";
		fission_fission_ringring = new TH2F( hname.data(), htitle.data(),
										set->GetNumberOfCDRings(), -0.5, set->GetNumberOfCDRings()-0.5,
										set->GetNumberOfCDRings(), -0.5, set->GetNumberOfCDRings()-0.5 );

		hname = "fission_fission_dEdE_array";
		htitle = "fission-fission dE-dE plot";
		htitle += " - coincidence with each other and an array event, random subtracted;Fragment 1 dE [keV];Fragment 2 dE [keV];Counts";
		fission_fission_dEdE_array = new TH2F( hname.data(), htitle.data(),
											  react->HistFissionBins(), react->HistFissionMin(), react->HistFissionMax(),
											  react->HistFissionBins(), react->HistFissionMin(), react->HistFissionMax() );

		hname = "fission_dE_vs_ring";
		htitle = "fission dE versus ring number";
		htitle += ";Ring number;Fragment dE [keV];Counts";
		fission_dE_vs_ring = new TH2F( hname.data(), htitle.data(),
									  set->GetNumberOfCDRings(), -0.5, set->GetNumberOfCDRings() - 0.5,
									  react->HistFissionBins(), react->HistFissionMin(), react->HistFissionMax() );

		hname = "fission_Etot_vs_ring";
		htitle = "fission E total versus ring number";
		htitle += ";Ring number;Fragment dE [keV];Counts";
		fission_Etot_vs_ring = new TH2F( hname.data(), htitle.data(),
									  set->GetNumberOfCDRings(), -0.5, set->GetNumberOfCDRings() - 0.5,
									  react->HistFissionBins(), react->HistFissionMin(), react->HistFissionMax() );

		hname = "fission_xy_map";
		htitle = "Fission fragment X-Y hit map;y (horizontal) [mm];x (vertical) [mm];Counts";
		fission_xy_map = new TH2F( hname.data(), htitle.data(), 361, -45.125, 45.125, 361, -45.125, 45.125 );

		hname = "fission_xy_map_cutH";
		htitle = "Fission fragment X-Y hit map, with cut on heavy fragment;y (horizontal) [mm];x (vertical) [mm];Counts";
		fission_xy_map_cutH = new TH2F( hname.data(), htitle.data(), 361, -45.125, 45.125, 361, -45.125, 45.125 );

		hname = "fission_xy_map_cutL";
		htitle = "Fission fragment X-Y hit map, with cut on light fragment;y (horizontal) [mm];x (vertical) [mm];Counts";
		fission_xy_map_cutL = new TH2F( hname.data(), htitle.data(), 361, -45.125, 45.125, 361, -45.125, 45.125 );


		// Timing plots
		output_file->cd( "Timing" );
		fission_array_td.resize( set->GetNumberOfArrayModules() );

		// Fission-fission time difference
		hname = "fission_fission_td";
		htitle = "Time difference between two fission events";
		htitle += ";#Deltat;Counts";
		fission_fission_td = new TH1F( hname.data(), htitle.data(),
									  1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );

		// Fission-fission time difference per sector
		hname = "fission_fission_td_sec";
		htitle = "Time difference between two fission events";
		htitle += "CD sector of first hit;#Deltat (first hit - second hit);Counts";
		fission_fission_td_sec = new TH2F( hname.data(), htitle.data(),
										  set->GetNumberOfCDSectors(), -0.5, set->GetNumberOfCDSectors()-0.5,
										  1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );

		// For array modules
		for( unsigned int j = 0; j < set->GetNumberOfArrayModules(); ++j ) {

			hname = "td_fission_array_mod" + std::to_string(j);
			htitle = "Time difference between fission detector ";
			htitle += " and array module " + std::to_string(j);
			htitle += ";#Deltat;Counts";
			fission_array_td[j] = new TH1F( hname.data(), htitle.data(),
										   1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );

		}


		// fission-array time walk
		output_file->cd( "Timing" );
		fission_array_tw_hit0 = new TH2F( "tw_fission_array_hit0",
										 "Time-walk histogram for array-fission coincidences with hit bit false;#Deltat [ns];Array energy [keV];Counts",
										 1000, -1.0*set->GetEventWindow(), 1.0*set->GetEventWindow(),
										 react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );
		fission_array_tw_hit1 = new TH2F( "tw_fission_array_hit1",
										 "Time-walk histogram for array-fission coincidences with hit bit true;#Deltat [ns];Array energy [keV];Counts",
										 1000, -1.0*set->GetEventWindow(), 1.0*set->GetEventWindow(),
										 react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );
		fission_array_tw_hit0_prof = new TProfile( "tw_fission_array_hit0_prof", "Time-walk profile for fission-array coincidences with hit bit false;Array energy;#Delta t", 2000, 0, 60000 );
		fission_array_tw_hit1_prof = new TProfile( "tw_fission_array_hit1_prof", "Time-walk profile for fission-array coincidences with hit bit true;Array energy;#Delta t", 2000, 0, 60000 );

		fission_array_tw_hit0_row.resize( set->GetNumberOfArrayModules() );
		fission_array_tw_hit1_row.resize( set->GetNumberOfArrayModules() );

		// Loop over ISS modules
		for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {

			fission_array_tw_hit0_row[i].resize( set->GetNumberOfArrayRows() );
			fission_array_tw_hit1_row[i].resize( set->GetNumberOfArrayRows() );

			// Loop over rows of the array
			for( unsigned int j = 0; j < set->GetNumberOfArrayRows(); ++j ) {

				hname = "tw_fission_array_hit0_mod_" + std::to_string(i) + "_row" + std::to_string(j);
				htitle = "Time-walk histogram for array-fission coincidences (module ";
				htitle += std::to_string(i) + ", row " + std::to_string(j) + ") with hit bit false;Deltat [ns];Array energy [keV];Counts";
				fission_array_tw_hit0_row[i][j] = new TH2F( hname.data(), htitle.data(), 1000, -1.0*set->GetEventWindow(), 1.0*set->GetEventWindow(),
														   react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

				hname = "tw_fission_array_hit1_mod_" + std::to_string(i) + "_row" + std::to_string(j);
				htitle = "Time-walk histogram for array-fission coincidences (module ";
				htitle += std::to_string(i) + ", row " + std::to_string(j) + ") with hit bit true;Deltat [ns];Array energy [keV];Counts";
				fission_array_tw_hit1_row[i][j] = new TH2F( hname.data(), htitle.data(), 1000, -1.0*set->GetEventWindow(), 1.0*set->GetEventWindow(),
														   react->HistElabBins(), react->HistElabMin(), react->HistElabMax() );

			}

		}

	} // fission

	// EBIS time windows
	output_file->cd( "Timing" );
	ebis_td_array = new TH1F( "ebis_td_array", "Array time with respect to EBIS;#Deltat;Counts per 20 #mus", 5.5e3, -0.1e8, 1e8  );
	ebis_td_elum = new TH1F( "ebis_td_elum", "ELUM time with respect to EBIS;#Deltat;Counts per 20 #mus", 5.5e3, -0.1e8, 1e8  );
	ebis_td_lume = new TH1F( "ebis_td_lume", "LUME time with respect to EBIS;#Deltat;Counts per 20 #mus", 5.5e3, -0.1e8, 1e8  );
	if( react->IsFission() && set->GetNumberOfCDLayers() > 0 )
		ebis_td_fission = new TH1F( "ebis_td_fission", "Fission fragment time with respect to EBIS;#Deltat;Counts per 20 #mus", 5.5e3, -0.1e8, 1e8  );
	else if( !react->IsFission() )
		ebis_td_recoil = new TH1F( "ebis_td_recoil", "Recoil time with respect to EBIS;#Deltat;Counts per 20 #mus", 5.5e3, -0.1e8, 1e8  );

	// Supercycle and proton pulses
	if( react->IsFission() && set->GetNumberOfCDLayers() > 0 ) {
		t1_td_fission = new TH1F( "t1_td_fission", "Fission fragment time difference with respect to the T1;#Deltat;Counts per 20 #mus", 5.5e3, -0.1e11, 1e11 );
		sc_td_fission = new TH1F( "sc_td_fission", "Fission fragment time difference with respect to the SuperCycle;#Deltat;Counts per 20 #mus", 5.5e3, -0.1e11, 1e11 );
	}
	else if( !react->IsFission() ) {
		t1_td_recoil = new TH1F( "t1_td_recoil", "Recoil time difference with respect to the T1;#Deltat;Counts per 20 #mus", 5.5e3, -0.1e11, 1e11 );
		sc_td_recoil = new TH1F( "sc_td_recoil", "Recoil time difference with respect to the SuperCycle;#Deltat;Counts per 20 #mus", 5.5e3, -0.1e11, 1e11 );
	}

	// For ELUM sectors
	dirname = "ElumDetector";
	output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	elum = new TH1F( "elum", "ELUM singles;Energy (keV);Counts per 5 keV", 10000, 0, 50000 );
	elum_ebis = new TH1F( "elum_ebis", "ELUM gated by EBIS and off beam subtracted;Energy (keV);Counts per 5 keV", 10000, 0, 50000 );
	elum_ebis_on = new TH1F( "elum_ebis_on", "ELUM gated on EBIS;Energy (keV);Counts per 5 keV", 10000, 0, 50000 );
	elum_ebis_off = new TH1F( "elum_ebis_off", "ELUM gated off EBIS;Energy (keV);Counts per 5 keV", 10000, 0, 50000 );
	elum_vs_T1 = new TH2F( "elum_vs_T1", "ELUM energy versus T1 time (gated on EBIS);Energy (keV);Counts per 5 keV", 5000, 0, 50e9, 10000, 0, 50000 );

	// Recoils only if we are not doing fission
	if( !react->IsFission() ) {
		elum_recoil = new TH1F( "elum_recoil", "ELUM gated on recoils;Energy (keV);Counts per 5 keV", 10000, 0, 50000 );
		elum_recoilT = new TH1F( "elum_recoilT", "ELUM with prompt time gate on all recoils;Energy (keV);Counts per 5 keV", 10000, 0, 50000 );
		elum_recoil_random = new TH1F( "elum_recoil_random", "ELUM with time-random gate on recoils;Energy (keV);Counts per 5 keV", 10000, 0, 50000 );
		elum_recoilT_random = new TH1F( "elum_recoilT_random", "ELUM with random time gate on all recoils;Energy (keV);Counts per 5 keV", 10000, 0, 50000 );
	}

	elum_sec.resize( set->GetNumberOfELUMSectors() );
	elum_ebis_sec.resize( set->GetNumberOfELUMSectors() );
	elum_ebis_on_sec.resize( set->GetNumberOfELUMSectors() );
	elum_ebis_off_sec.resize( set->GetNumberOfELUMSectors() );

	if( !react->IsFission() ) {
		elum_recoil_sec.resize( set->GetNumberOfELUMSectors() );
		elum_recoilT_sec.resize( set->GetNumberOfELUMSectors() );
		elum_recoil_random_sec.resize( set->GetNumberOfELUMSectors() );
		elum_recoilT_random_sec.resize( set->GetNumberOfELUMSectors() );
	}

	for( unsigned int j = 0; j < set->GetNumberOfELUMSectors(); ++j ) {

		dirname = "ElumDetector/sector_" + std::to_string(j);
		output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );

		hname = "elum_sec" + std::to_string(j);
		htitle = "ELUM singles for sector " + std::to_string(j);
		htitle += ";Energy [keV];Counts 5 keV";
		elum_sec[j] = new TH1F( hname.data(), htitle.data(), 10000, 0, 50000 );

		hname = "elum_ebis_sec" + std::to_string(j);
		htitle = "ELUM events for sector " + std::to_string(j);
		htitle += " gated by EBIS and off beam subtracted;Energy [keV];Counts 5 keV";
		elum_ebis_sec[j] = new TH1F( hname.data(), htitle.data(), 10000, 0, 50000 );

		hname = "elum_ebis_on_sec" + std::to_string(j);
		htitle = "ELUM events for sector " + std::to_string(j);
		htitle += " gated on EBIS;Energy [keV];Counts 5 keV";
		elum_ebis_on_sec[j] = new TH1F( hname.data(), htitle.data(), 10000, 0, 50000 );

		hname = "elum_ebis_off_sec" + std::to_string(j);
		htitle = "ELUM events for sector " + std::to_string(j);
		htitle += " gated off EBIS;Energy [keV];Counts 5 keV";
		elum_ebis_off_sec[j] = new TH1F( hname.data(), htitle.data(), 10000, 0, 50000 );

		// Recoils
		if( !react->IsFission() ) {

			hname = "elum_recoil_sec" + std::to_string(j);
			htitle = "ELUM singles for sector " + std::to_string(j);
			htitle += " gated on recoils;Energy [keV];Counts 5 keV";
			elum_recoil_sec[j] = new TH1F( hname.data(), htitle.data(), 10000, 0, 50000 );

			hname = "elum_recoilT_sec" + std::to_string(j);
			htitle = "ELUM singles for sector " + std::to_string(j);
			htitle += " with a prompt time gate on all recoils;Energy [keV];Counts 5 keV";
			elum_recoilT_sec[j] = new TH1F( hname.data(), htitle.data(), 10000, 0, 50000 );

			hname = "elum_recoil_random_sec" + std::to_string(j);
			htitle = "ELUM singles for sector " + std::to_string(j);
			htitle += " time-random gated on recoils;Energy [keV];Counts 5 keV";
			elum_recoil_random_sec[j] = new TH1F( hname.data(), htitle.data(), 10000, 0, 50000 );

			hname = "elum_recoilT_random_sec" + std::to_string(j);
			htitle = "ELUM singles for sector " + std::to_string(j);
			htitle += " with a random time gate on all recoils;Energy [keV];Counts 5 keV";
			elum_recoilT_random_sec[j] = new TH1F( hname.data(), htitle.data(), 10000, 0, 50000 );

		}

	} // ELUM

	// For LUME detectors
	dirname = "LumeDetector";
	output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	lume = new TH1F( "lume", "LUME singles;Energy (keV);Counts per 5 keV", react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );
	lume_ebis = new TH1F( "lume_ebis", "LUME gated by EBIS and off beam subtracted;Energy (keV);Counts per 5 keV", react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );
	lume_ebis_on = new TH1F( "lume_ebis_on", "LUME gated on EBIS;Energy (keV);Counts per 5 keV", react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );
	lume_ebis_off = new TH1F( "lume_ebis_off", "LUME gated off EBIS;Energy (keV);Counts per 5 keV", react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );
	lume_recoil = new TH1F( "lume_recoil", "LUME in prompt time coincidence with an energy-gated recoil event;Energy (keV);Counts per 5 keV", react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );
	lume_recoilT = new TH1F( "lume_recoilT", "LUME in prompt time coincidence with a recoil event;Energy (keV);Counts per 5 keV", react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );
	lume_recoil_random = new TH1F( "lume_recoil_random", "LUME in random time coincidence with an energy-gated recoil event;Energy (keV);Counts per 5 keV", react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );
	lume_recoilT_random = new TH1F( "lume_recoilT_random", "LUME in random time coincidence with a recoil event;Energy (keV);Counts per 5 keV", react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );
	lume_vs_T1 = new TH2F( "lume_vs_T1", "LUME energy versus T1 time (gated on EBIS);Energy (keV);Counts per 5 keV", 5000, 0, 50e9, react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );
	lume_E_vs_x = new TH2F( "lume_E_vs_x", "LUME energy versus position;Position;Energy (keV)", 400, -2, 2, react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );
	lume_E_vs_x_ebis = new TH2F( "lume_E_vs_x_ebis", "LUME energy versus position gated by EBIS and off beam subtracted;Position;Energy (keV)", 400, -2, 2, react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );
	lume_E_vs_x_ebis_on = new TH2F( "lume_E_vs_x_ebis_on", "LUME energy versus position gated on EBIS;Position;Energy (keV)", 400, -2, 2, react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );
	lume_E_vs_x_ebis_off = new TH2F( "lume_E_vs_x_ebis_off", "LUME energy versus positiongated off EBIS ;Position;Energy (keV)", 400, -2, 2, react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );
	lume_E_vs_x_wide = new TH2F( "lume_E_vs_x_wide", "LUME energy versus position;Position;Energy (keV)", 400, -2, 2, 8000, -200, 159800 );

	lume_det.resize( set->GetNumberOfLUMEDetectors() );
	lume_ebis_det.resize( set->GetNumberOfLUMEDetectors() );
	lume_ebis_on_det.resize( set->GetNumberOfLUMEDetectors() );
	lume_ebis_off_det.resize( set->GetNumberOfLUMEDetectors() );
	lume_recoil_det.resize( set->GetNumberOfLUMEDetectors() );
	lume_recoilT_det.resize( set->GetNumberOfLUMEDetectors() );
	lume_recoil_random_det.resize( set->GetNumberOfLUMEDetectors() );
	lume_recoilT_random_det.resize( set->GetNumberOfLUMEDetectors() );
	lume_E_vs_x_det.resize( set->GetNumberOfLUMEDetectors() );
	lume_E_vs_x_ebis_det.resize( set->GetNumberOfLUMEDetectors() );
	lume_E_vs_x_ebis_on_det.resize( set->GetNumberOfLUMEDetectors() );
	lume_E_vs_x_ebis_off_det.resize( set->GetNumberOfLUMEDetectors() );

	// Loop over number of LUME detectors
	for( unsigned int i = 0; i < set->GetNumberOfLUMEDetectors(); ++i ) {

		dirname = "LumeDetector/detector_" + std::to_string(i);
		output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );

		hname = "lume_det_" + std::to_string(i);
		htitle = "LUME energy spectrum for detector " + std::to_string(i);
		htitle += ";Energy [keV];Counts per 5 keV";
		lume_det[i] = new TH1F( hname.data(), htitle.data(),
							   react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );

		hname = "lume_ebis_det_" + std::to_string(i);
		htitle = "LUME events for  detector " + std::to_string(i);
		htitle += " gated by EBIS and off beam subtracted;Energy (keV);Counts per 5 keV";
		lume_ebis_det[i] = new TH1F( hname.data(), htitle.data(),
									react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );

		hname = "lume_ebis_on_det_" + std::to_string(i);
		htitle = "LUME events for detector " + std::to_string(i);
		htitle += " gated on EBIS ;Energy (keV);Counts per 5 keV";
		lume_ebis_on_det[i] = new TH1F( hname.data(), htitle.data(),
									   react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );

		hname = "lume_ebis_off_det_" + std::to_string(i);
		htitle = "LUME events for detector " + std::to_string(i);
		htitle += " gated off EBIS ;Energy (keV);Counts per 5 keV";
		lume_ebis_off_det[i] = new TH1F( hname.data(), htitle.data(),
										react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );

		hname = "lume_recoil_det_" + std::to_string(i);
		htitle = "LUME energy spectrum for detector " + std::to_string(i);
		htitle += " in prompt time coincidence with an energy-gated recoil event;";
		htitle += "Energy [keV];Counts per 5 keV";
		lume_recoil_det[i] = new TH1F( hname.data(), htitle.data(),
									  react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );

		hname = "lume_recoilT_det_" + std::to_string(i);
		htitle = "LUME energy spectrum for detector " + std::to_string(i);
		htitle += " in prompt time coincidence with a recoil event;";
		htitle += "Energy [keV];Counts per 5 keV";
		lume_recoilT_det[i] = new TH1F( hname.data(), htitle.data(),
									   react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );

		hname = "lume_recoil_random_det_" + std::to_string(i);
		htitle = "LUME energy spectrum for detector " + std::to_string(i);
		htitle += " in random time coincidence with an energy-gated recoil event;";
		htitle += "Energy [keV];Counts per 5 keV";
		lume_recoil_random_det[i] = new TH1F( hname.data(), htitle.data(),
											 react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );

		hname = "lume_recoilT_random_det_" + std::to_string(i);
		htitle = "LUME energy spectrum for detector " + std::to_string(i);
		htitle += " in random time coincidence with a recoil event;";
		htitle += "Energy [keV];Counts per 5 keV";
		lume_recoilT_random_det[i] = new TH1F( hname.data(), htitle.data(),
											  react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );

		hname = "lume_E_vs_x_det_" + std::to_string(i);
		htitle = "LUME energy vs position spectrum for detector " + std::to_string(i);
		htitle += ";Position;Energy [keV]";
		lume_E_vs_x_det[i] = new TH2F( hname.data(), htitle.data(), 400, -2., 2., react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );

		hname = "lume_E_vs_x_ebis_det_" + std::to_string(i);
		htitle = "LUME energy vs position spectrum for detector " + std::to_string(i);
		htitle += " gated by EBIS and off beam subtracted;Position;Energy [keV]";
		lume_E_vs_x_ebis_det[i] = new TH2F( hname.data(), htitle.data(), 400, -2., 2., react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );

		hname = "lume_E_vs_x_ebis_on_det_" + std::to_string(i);
		htitle = "LUME energy vs position spectrum for detector " + std::to_string(i);
		htitle += " gated on EBIS;Position;Energy [keV]";
		lume_E_vs_x_ebis_on_det[i] = new TH2F( hname.data(), htitle.data(), 400, -2., 2., react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );

		hname = "lume_E_vs_x_ebis_off_det_" + std::to_string(i);
		htitle = "LUME energy vs position spectrum for detector " + std::to_string(i);
		htitle += " gated off EBIS;Position;Energy [keV]";
		lume_E_vs_x_ebis_off_det[i] = new TH2F( hname.data(), htitle.data(), 400, -2., 2., react->HistLumeBins(), react->HistLumeMin(), react->HistLumeMax() );

	} // LUME


	// For gamma-ray detectors
	if( react->GammaRayHistsEnabled() ) {

		dirname = "GammaRays";
		output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );

		gamma_ebis = new TH1F( "gamma_ebis", "Gamma-ray singles, EBIS on-off;Energy (keV);Counts",
							  react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax() );
		gamma_ebis_on = new TH1F( "gamma_ebis_on", "Gamma-ray singles, gated on EBIS on;Energy (keV);Counts",
								 react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax() );
		gamma_ebis_off = new TH1F( "gamma_ebis_off", "Gamma-ray singles, gated on EBIS off;Energy (keV);Counts",
								  react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax() );

		gamma_fission = new TH1F( "gamma_fission", "Gamma-ray singles, gated on fission fragments;Energy (keV);Counts",
								 react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax() );
		gamma_recoil = new TH1F( "gamma_recoil", "Gamma-ray singles, gated on recoils;Energy (keV);Counts",
								react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax() );
		gamma_recoilT = new TH1F( "gamma_recoilT", "Gamma-ray singles, gated on any recoils;Energy (keV);Counts",
								react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax() );
		gamma_array = new TH1F( "gamma_array", "Gamma-ray singles, gated on any array event;Energy (keV);Counts",
							   react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax() );

		gamma_gamma_ebis = new TH2F( "gamma_gamma_ebis", "Gamma-ray coincidence matrix, time-random subtracted, EBIS on-off;Energy (keV);Energy (keV);Counts",
									react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax(),
									react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax() );
		gamma_gamma_fission = new TH2F( "gamma_gamma_fission", "Gamma-ray coincidence matrix, time-random subtracted, gated on fission fragments;Energy (keV);Energy (keV);Counts",
									   react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax(),
									   react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax() );
		gamma_gamma_recoil = new TH2F( "gamma_gamma_recoil", "Gamma-ray coincidence matrix, time-random subtracted, gated on recoils;Energy (keV);Energy (keV);Counts",
									  react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax(),
									  react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax() );
		gamma_gamma_array = new TH2F( "gamma_gamma_array", "Gamma-ray coincidence matrix, time-random subtracted, gated on any array event;Energy (keV);Energy (keV);Counts",
									 react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax(),
									 react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax() );

		gamma_Ex_ebis = new TH2F( "gamma_Ex_ebis", "Gamma-ray energy vs excitation energy, time-random subtracted, EBIS on-off;E_{#gamma} (keV);E_{x} (keV);Counts",
								 react->HistExBins(), react->HistExMin(), react->HistExMax(),
								 react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax() );
		gamma_Ex_fission = new TH2F( "gamma_Ex_fission", "Gamma-ray vs excitation energy, time-random subtracted, gated on fission fragments;E_{#gamma} (keV);E_{x} (keV);Counts",
									react->HistExBins(), react->HistExMin(), react->HistExMax(),
									react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax() );
		gamma_Ex_recoilT = new TH2F( "gamma_Ex_recoilT", "Gamma-ray vs excitation energy, time-random subtracted, gated on any recoil;E_{#gamma} (keV);E_{x} (keV);Counts",
								   react->HistExBins(), react->HistExMin(), react->HistExMax(),
								   react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax() );
		gamma_Ex_recoil = new TH2F( "gamma_Ex_recoil", "Gamma-ray vs excitation energy, time-random subtracted, gated on recoils in energy cut;E_{#gamma} (keV);E_{x} (keV);Counts",
								   react->HistExBins(), react->HistExMin(), react->HistExMax(),
								   react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax() );


		// Check the E vs z cuts from the user
		gamma_array_cut.resize( react->GetNumberOfEvsZCuts() );
		gamma_gamma_array_cut.resize( react->GetNumberOfEvsZCuts() );
		for( unsigned int i = 0; i < react->GetNumberOfEvsZCuts(); ++i ){

			hname = "gamma_array_cut" + std::to_string(i);
			htitle = "Gamma-ray singles, gated on array events in cut " + std::to_string(i);
			htitle += ", time-random subtracted;Energy (keV);Counts";
			gamma_array_cut[i] = new TH1F( hname.data(), htitle.data(),
										  react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax() );

			hname = "gamma_gamma_array_cut" + std::to_string(i);
			htitle = "Gamma-ray coincidence matrix, gated on array events in cut " + std::to_string(i);
			htitle += ", time-random subtracted;Energy (keV);Energy (keV);Counts";
			gamma_gamma_array_cut[i] = new TH2F( hname.data(), htitle.data(),
												react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax(),
												react->HistGammaBins(), react->HistGammaMin(), react->HistGammaMax() );

		} // E vs z cuts

		// Timing histograms
		output_file->cd( "Timing" );

		hname = "gamma_gamma_td";
		htitle = "Time difference between two gamma-ray events";
		htitle += ";#Deltat;Counts";
		gamma_gamma_td = new TH1F( hname.data(), htitle.data(),
								  1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );

		hname = "gamma_fission_td";
		htitle = "Time difference between a gamma-ray and fission event";
		htitle += ";#Deltat;Counts";
		gamma_fission_td = new TH1F( hname.data(), htitle.data(),
									1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );

		hname = "gamma_recoil_td";
		htitle = "Time difference between a gamma-ray and recoil event";
		htitle += ";#Deltat;Counts";
		gamma_recoil_td = new TH1F( hname.data(), htitle.data(),
								   1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );

		hname = "gamma_array_td";
		htitle = "Time difference between a gamma-ray and array event";
		htitle += ";#Deltat;Counts";
		gamma_array_td = new TH1F( hname.data(), htitle.data(),
								  1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );


	} // gamma-ray hists


	// Multiplicities
	dirname = "Multiplicities";
	output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	mult_array_fission = new TH2F( "mult_array_fission", "Multiplicity map;CD multiplicity;Array multiplicity;Counts",
								  20, -0.5, 19.5, 20, -0.5, 19.5 );
	mult_array_recoil = new TH2F( "mult_array_recoil", "Multiplicity map;Recoil multiplicity;Array multiplicity;Counts",
								  20, -0.5, 19.5, 20, -0.5, 19.5 );
	mult_array_gamma = new TH2F( "mult_array_gamma", "Multiplicity map;Gamma-ray multiplicity;Array multiplicity;Counts",
								  20, -0.5, 19.5, 20, -0.5, 19.5 );
	mult_gamma_fission = new TH2F( "mult_gamma_fission", "Multiplicity map;CD multiplicity;Gamma-ray multiplicity;Counts",
								  20, -0.5, 19.5, 20, -0.5, 19.5 );
	mult_gamma_recoil = new TH2F( "mult_gamma_recoil", "Multiplicity map;Recoil multiplicity;Gamma-ray multiplicity;Counts",
								  20, -0.5, 19.5, 20, -0.5, 19.5 );

}


void ISSHistogrammer::PlotDefaultHists() {

	// Check that we're ready
	if( !hists_ready ) return;

	// Make the canvas
	c1 = std::make_unique<TCanvas>("Diagnostics","Monitor hists");
	c1->Divide(2,2);

	// Plot things
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); i++ ){

		c1->cd(i+1);
		if( E_vs_z_ebis_on_mod[i] != nullptr )
			E_vs_z_ebis_on_mod[i]->Draw("colz");

	}

	c1->cd( set->GetNumberOfArrayModules() + 1 );
	if( E_vs_z_ebis_on != nullptr )
		E_vs_z_ebis_on->Draw("colz");

	return;

}

void ISSHistogrammer::SetSpyHists( std::vector<std::vector<std::string>> hists, short layout[2] ) {

	// Copy the input hists and layouts
	spyhists = hists;
	spylayout[0] = layout[0];
	spylayout[1] = layout[1];

	// Flag that we have spy mode
	spymode = true;

}

void ISSHistogrammer::PlotPhysicsHists() {

	// Escape if we haven't built the hists to avoid a seg fault
	if( !hists_ready ){

		std::cout << "Cannot plot diagnostics yet, wait until histogrammer is ready" << std::endl;
		return;

	}

	// Get appropriate layout and number of hists
	unsigned short maxhists = spylayout[0] * spylayout[1];
	if( maxhists == 0 ) maxhists = 1;
	if( spyhists.size() > maxhists ) {

		std::cout << "Too many histograms for layout size. Plotting the first ";
		std::cout << maxhists << " histograms in the list." << std::endl;

	}
	else maxhists = spyhists.size();

	// Make the canvas
	c2 = std::make_unique<TCanvas>("Physics","User hists");
	if( maxhists > 1 && spylayout[0] > 0 && spylayout[1] > 0 )
		c2->Divide( spylayout[0], spylayout[1] );

	// User defined histograms
	TH1F *ptr_th1;
	TH2F *ptr_th2;
	for( unsigned int i = 0; i < maxhists; i++ ){

		// Go to corresponding canvas
		c2->cd(i+1);

		// Get this histogram of the right type
		if( spyhists[i][1] == "TH1" || spyhists[i][1] == "TH1F" || spyhists[i][1] == "TH1D" ) {

			ptr_th1 = (TH1F*)output_file->Get( spyhists[i][0].data() );
			if( ptr_th1 != nullptr )
				ptr_th1->Draw( spyhists[i][2].data() );

		}

		else if( spyhists[i][1] == "TH2" || spyhists[i][1] == "TH2F" || spyhists[i][1] == "TH2D" ) {

			ptr_th2 = (TH2F*)output_file->Get( spyhists[i][0].data() );
			if( ptr_th2 != nullptr )
				ptr_th2->Draw( spyhists[i][2].data() );

		}

		else std::cout << "Type " << spyhists[i][1] << " not currently supported" << std::endl;

	}

	return;

}


void ISSHistogrammer::ResetHist( TObject *obj ) {

	if( obj == nullptr ) return;

	if( obj->InheritsFrom( "TH2" ) ) {
		( (TH2*)obj )->Reset("ICESM");
		( (TH2*)obj )->GetZaxis()->UnZoom();
	}
	else if( obj->InheritsFrom( "TH1" ) )
		( (TH1*)obj )->Reset("ICESM");

	return;

}

void ISSHistogrammer::ResetHists() {

	std::cout << "in ISSHistogrammer::Reset_Hist()" << std::endl;

	// Timing
	ebis_td_array->Reset("ICESM");
	ebis_td_elum->Reset("ICESM");
	ebis_td_lume->Reset("ICESM");

	// Recoils, but only if we are not doing fission
	if( !react->IsFission() ) {

		ebis_td_recoil->Reset("ICESM");
		t1_td_recoil->Reset("ICESM");
		sc_td_recoil->Reset("ICESM");
		recoil_array_tw_hit0->Reset("ICESM");
		recoil_array_tw_hit1->Reset("ICESM");

		for( unsigned int i = 0; i < recoil_array_td.size(); ++i )
			for( unsigned int j = 0; j < recoil_array_td[i].size(); ++j )
				recoil_array_td[i][j]->Reset("ICESM");

		for( unsigned int i = 0; i < recoil_elum_td.size(); ++i )
			for( unsigned int j = 0; j < recoil_elum_td[i].size(); ++j )
				recoil_elum_td[i][j]->Reset("ICESM");

		for( unsigned int i = 0; i < recoil_array_tw_hit0_row.size(); ++i )
			for( unsigned int j = 0; j < recoil_array_tw_hit0_row[i].size(); ++j )
				recoil_array_tw_hit0_row[i][j]->Reset("ICESM");

		for( unsigned int i = 0; i < recoil_array_tw_hit1_row.size(); ++i )
			for( unsigned int j = 0; j < recoil_array_tw_hit1_row[i].size(); ++j )
				recoil_array_tw_hit1_row[i][j]->Reset("ICESM");

		for( unsigned int i = 0; i < recoil_EdE.size(); ++i )
			recoil_EdE[i]->Reset("ICESM");

		for( unsigned int i = 0; i < recoil_EdE_cut.size(); ++i )
			recoil_EdE_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < recoil_bragg.size(); ++i )
			recoil_bragg[i]->Reset("ICESM");

		for( unsigned int i = 0; i < recoil_dE_vs_T1.size(); ++i )
			recoil_dE_vs_T1[i]->Reset("ICESM");

		for( unsigned int i = 0; i < recoil_dE_eloss.size(); ++i )
			recoil_dE_eloss[i]->Reset("ICESM");

		for( unsigned int i = 0; i < recoil_E_eloss.size(); ++i )
			recoil_E_eloss[i]->Reset("ICESM");

	}

	for( unsigned int i = 0; i < recoil_lume_td.size(); ++i )
		recoil_lume_td[i]->Reset("ICESM");

	// Fission
	if( react->IsFission() && set->GetNumberOfCDLayers() > 0 ) {

		ebis_td_fission->Reset("ICESM");
		t1_td_fission->Reset("ICESM");
		sc_td_fission->Reset("ICESM");
		fission_array_tw_hit0->Reset("ICESM");
		fission_array_tw_hit1->Reset("ICESM");
		fission_fission_td->Reset("ICESM");
		fission_fission_td_sec->Reset("ICESM");

		for( unsigned int i = 0; i < fission_array_td.size(); ++i )
			fission_array_td[i]->Reset("ICESM");

		for( unsigned int i = 0; i < fission_array_tw_hit0_row.size(); ++i )
			for( unsigned int j = 0; j < fission_array_tw_hit0_row[i].size(); ++j )
				fission_array_tw_hit0_row[i][j]->Reset("ICESM");

		for( unsigned int i = 0; i < fission_array_tw_hit1_row.size(); ++i )
			for( unsigned int j = 0; j < fission_array_tw_hit1_row[i].size(); ++j )
				fission_array_tw_hit1_row[i][j]->Reset("ICESM");

		fission_EdE->Reset("ICESM");
		fission_EdE_cutH->Reset("ICESM");
		fission_EdE_cutL->Reset("ICESM");
		fission_bragg->Reset("ICESM");
		fission_dE_vs_T1->Reset("ICESM");
		fission_dE_eloss->Reset("ICESM");
		fission_E_eloss->Reset("ICESM");
		fission_fission_dEdE->Reset("ICESM");
		fission_fission_dEdE_array->Reset("ICESM");
		fission_dE_vs_ring->Reset("ICESM");
		fission_xy_map->Reset("ICESM");
		fission_xy_map_cutH->Reset("ICESM");
		fission_xy_map_cutL->Reset("ICESM");

	}

	// Array - E vs. z
	E_vs_z->Reset("ICESM");
	E_vs_z_ebis->Reset("ICESM");
	E_vs_z_ebis_on->Reset("ICESM");
	E_vs_z_ebis_off->Reset("ICESM");
	E_vs_z_T1->Reset("ICESM");

	E_vs_z_recoil->Reset("ICESM");
	E_vs_z_recoilT->Reset("ICESM");
	E_vs_z_recoil_random->Reset("ICESM");
	E_vs_z_recoilT_random->Reset("ICESM");

	if( react->GammaRayHistsEnabled() ){

		E_vs_z_gamma->Reset("ICESM");
		E_vs_z_gammaT->Reset("ICESM");
		E_vs_z_gamma_random->Reset("ICESM");
		E_vs_z_gammaT_random->Reset("ICESM");

	}

	for( unsigned int i = 0; i < E_vs_z_recoil_cut.size(); ++i )
		E_vs_z_recoil_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_z_recoilT_cut.size(); ++i )
		E_vs_z_recoilT_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_z_recoil_mod.size(); ++i )
		E_vs_z_recoil_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_z_recoilT_mod.size(); ++i )
		E_vs_z_recoilT_mod[i]->Reset("ICESM");


	if( react->IsFission() && set->GetNumberOfCDLayers() > 0 ) {

		E_vs_z_fission->Reset("ICESM");
		E_vs_z_fissionT->Reset("ICESM");
		E_vs_z_fission_gamma->Reset("ICESM");
		E_vs_z_fission_random->Reset("ICESM");
		E_vs_z_fissionT_random->Reset("ICESM");
		E_vs_z_fission_gamma_random->Reset("ICESM");

		for( unsigned int i = 0; i < E_vs_z_fission_cut.size(); ++i )
			E_vs_z_fission_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < E_vs_z_fissionT_cut.size(); ++i )
			E_vs_z_fissionT_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < E_vs_z_fission_mod.size(); ++i )
			E_vs_z_fission_mod[i]->Reset("ICESM");

		for( unsigned int i = 0; i < E_vs_z_fissionT_mod.size(); ++i )
			E_vs_z_fissionT_mod[i]->Reset("ICESM");

		for( unsigned int i = 0; i < E_vs_z_fission_random_cut.size(); ++i )
			E_vs_z_fission_random_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < E_vs_z_fissionT_random_cut.size(); ++i )
			E_vs_z_fissionT_random_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < E_vs_z_fission_random_mod.size(); ++i )
			E_vs_z_fission_random_mod[i]->Reset("ICESM");

		for( unsigned int i = 0; i < E_vs_z_fissionT_random_mod.size(); ++i )
			E_vs_z_fissionT_random_mod[i]->Reset("ICESM");

	}

	for( unsigned int i = 0; i < E_vs_z_T1_cut.size(); ++i )
		E_vs_z_T1_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_z_ebis_off_cut.size(); ++i )
		E_vs_z_ebis_off_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_z_ebis_on_cut.size(); ++i )
		E_vs_z_ebis_on_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_z_ebis_cut.size(); ++i )
		E_vs_z_ebis_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_z_mod.size(); ++i )
		E_vs_z_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_z_ebis_mod.size(); ++i )
		E_vs_z_ebis_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_z_ebis_on_mod.size(); ++i )
		E_vs_z_ebis_on_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_z_ebis_off_mod.size(); ++i )
		E_vs_z_ebis_off_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_z_cut.size(); ++i )
		E_vs_z_cut[i]->Reset("ICESM");

	// Array - Ex vs. thetaCM
	Ex_vs_theta->Reset("ICESM");
	Ex_vs_theta_ebis->Reset("ICESM");
	Ex_vs_theta_ebis_on->Reset("ICESM");
	Ex_vs_theta_ebis_off->Reset("ICESM");
	Ex_vs_theta_T1->Reset("ICESM");

	Ex_vs_theta_recoil->Reset("ICESM");
	Ex_vs_theta_recoilT->Reset("ICESM");
	Ex_vs_theta_recoil_random->Reset("ICESM");
	Ex_vs_theta_recoilT_random->Reset("ICESM");

	if( react->GammaRayHistsEnabled() ){

		Ex_vs_theta_gamma->Reset("ICESM");
		Ex_vs_theta_gammaT->Reset("ICESM");
		Ex_vs_theta_gamma_random->Reset("ICESM");
		Ex_vs_theta_gammaT_random->Reset("ICESM");

	}

	for( unsigned int i = 0; i < Ex_vs_theta_recoil_mod.size(); ++i )
		Ex_vs_theta_recoil_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_theta_recoilT_mod.size(); ++i )
		Ex_vs_theta_recoilT_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_theta_recoil_random_mod.size(); ++i )
		Ex_vs_theta_recoil_random_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_theta_recoilT_random_mod.size(); ++i )
		Ex_vs_theta_recoilT_random_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_theta_recoil_cut.size(); ++i )
		Ex_vs_theta_recoil_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_theta_recoilT_cut.size(); ++i )
		Ex_vs_theta_recoilT_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_theta_recoil_random_cut.size(); ++i )
		Ex_vs_theta_recoil_random_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_theta_recoilT_random_cut.size(); ++i )
		Ex_vs_theta_recoilT_random_cut[i]->Reset("ICESM");

	if( react->IsFission() && set->GetNumberOfCDLayers() > 0 ) {

		Ex_vs_theta_fission->Reset("ICESM");
		Ex_vs_theta_fissionT->Reset("ICESM");
		Ex_vs_theta_fission_random->Reset("ICESM");
		Ex_vs_theta_fissionT_random->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_vs_theta_fission_mod.size(); ++i )
			Ex_vs_theta_fission_mod[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_vs_theta_fissionT_mod.size(); ++i )
			Ex_vs_theta_fissionT_mod[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_vs_theta_fission_random_mod.size(); ++i )
			Ex_vs_theta_fission_random_mod[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_vs_theta_fissionT_random_mod.size(); ++i )
			Ex_vs_theta_fissionT_random_mod[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_vs_theta_fission_cut.size(); ++i )
			Ex_vs_theta_fission_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_vs_theta_fissionT_cut.size(); ++i )
			Ex_vs_theta_fissionT_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_vs_theta_fission_random_cut.size(); ++i )
			Ex_vs_theta_fission_random_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_vs_theta_fissionT_random_cut.size(); ++i )
			Ex_vs_theta_fissionT_random_cut[i]->Reset("ICESM");

	}

	for( unsigned int i = 0; i < Ex_vs_theta_mod.size(); ++i )
		Ex_vs_theta_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_theta_ebis_mod.size(); ++i )
		Ex_vs_theta_ebis_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_theta_ebis_off_mod.size(); ++i )
		Ex_vs_theta_ebis_off_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_theta_ebis_on_mod.size(); ++i )
		Ex_vs_theta_ebis_on_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_theta_ebis_on_cut.size(); ++i )
		Ex_vs_theta_ebis_on_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_theta_ebis_cut.size(); ++i )
		Ex_vs_theta_ebis_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_theta_cut.size(); ++i )
		Ex_vs_theta_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_theta_T1_cut.size(); ++i )
		Ex_vs_theta_T1_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_theta_ebis_off_cut.size(); ++i )
		Ex_vs_theta_ebis_off_cut[i]->Reset("ICESM");

	// Array - E vs. theta
	E_vs_theta->Reset("ICESM");
	E_vs_theta_ebis->Reset("ICESM");
	E_vs_theta_ebis_on->Reset("ICESM");
	E_vs_theta_ebis_off->Reset("ICESM");
	E_vs_theta_T1->Reset("ICESM");

	E_vs_theta_recoil->Reset("ICESM");
	E_vs_theta_recoilT->Reset("ICESM");
	E_vs_theta_recoil_random->Reset("ICESM");
	E_vs_theta_recoilT_random->Reset("ICESM");

	if( react->GammaRayHistsEnabled() ){

		E_vs_theta_gamma->Reset("ICESM");
		E_vs_theta_gammaT->Reset("ICESM");
		E_vs_theta_gamma_random->Reset("ICESM");
		E_vs_theta_gammaT_random->Reset("ICESM");

	}

	for( unsigned int i = 0; i < E_vs_theta_recoil_mod.size(); ++i )
		E_vs_theta_recoil_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_theta_recoilT_mod.size(); ++i )
		E_vs_theta_recoilT_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_theta_recoil_random_mod.size(); ++i )
		E_vs_theta_recoil_random_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_theta_recoilT_random_mod.size(); ++i )
		E_vs_theta_recoilT_random_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_theta_recoil_cut.size(); ++i )
		Ex_vs_theta_recoil_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_theta_recoilT_cut.size(); ++i )
		E_vs_theta_recoilT_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_theta_recoil_random_cut.size(); ++i )
		E_vs_theta_recoil_random_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_theta_recoilT_random_cut.size(); ++i )
		E_vs_theta_recoilT_random_cut[i]->Reset("ICESM");


	if( react->IsFission() && set->GetNumberOfCDLayers() > 0 ) {

		E_vs_theta_fission->Reset("ICESM");
		E_vs_theta_fissionT->Reset("ICESM");
		E_vs_theta_fission_random->Reset("ICESM");
		E_vs_theta_fissionT_random->Reset("ICESM");

		for( unsigned int i = 0; i < E_vs_theta_fission_mod.size(); ++i )
			E_vs_theta_fission_mod[i]->Reset("ICESM");

		for( unsigned int i = 0; i < E_vs_theta_fissionT_mod.size(); ++i )
			E_vs_theta_fissionT_mod[i]->Reset("ICESM");

		for( unsigned int i = 0; i < E_vs_theta_fission_random_mod.size(); ++i )
			E_vs_theta_fission_random_mod[i]->Reset("ICESM");

		for( unsigned int i = 0; i < E_vs_theta_fissionT_random_mod.size(); ++i )
			E_vs_theta_fissionT_random_mod[i]->Reset("ICESM");

		for( unsigned int i = 0; i < E_vs_theta_fission_cut.size(); ++i )
			Ex_vs_theta_fission_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < E_vs_theta_fissionT_cut.size(); ++i )
			E_vs_theta_fissionT_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < E_vs_theta_fission_random_cut.size(); ++i )
			E_vs_theta_fission_random_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < E_vs_theta_fissionT_random_cut.size(); ++i )
			E_vs_theta_fissionT_random_cut[i]->Reset("ICESM");

	}

	for( unsigned int i = 0; i < E_vs_theta_mod.size(); ++i )
		E_vs_theta_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_theta_ebis_mod.size(); ++i )
		E_vs_theta_ebis_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_theta_ebis_off_mod.size(); ++i )
		E_vs_theta_ebis_off_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_theta_ebis_on_mod.size(); ++i )
		E_vs_theta_ebis_on_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_theta_ebis_on_cut.size(); ++i )
		E_vs_theta_ebis_on_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_theta_ebis_cut.size(); ++i )
		E_vs_theta_ebis_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_theta_cut.size(); ++i )
		E_vs_theta_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_theta_T1_cut.size(); ++i )
		E_vs_theta_T1_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < E_vs_theta_ebis_off_cut.size(); ++i )
		E_vs_theta_ebis_off_cut[i]->Reset("ICESM");

	// Array - Ex vs. z
	Ex_vs_z->Reset("ICESM");
	Ex_vs_z_ebis->Reset("ICESM");
	Ex_vs_z_ebis_on->Reset("ICESM");
	Ex_vs_z_ebis_off->Reset("ICESM");
	Ex_vs_z_T1->Reset("ICESM");

	Ex_vs_z_recoil->Reset("ICESM");
	Ex_vs_z_recoilT->Reset("ICESM");
	Ex_vs_z_recoil_random->Reset("ICESM");
	Ex_vs_z_recoilT_random->Reset("ICESM");

	if( react->GammaRayHistsEnabled() ){

		Ex_vs_z_gamma->Reset("ICESM");
		Ex_vs_z_gammaT->Reset("ICESM");
		Ex_vs_z_gamma_random->Reset("ICESM");
		Ex_vs_z_gammaT_random->Reset("ICESM");

	}

	for( unsigned int i = 0; i < Ex_vs_z_recoil_mod.size(); ++i )
		Ex_vs_z_recoil_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_z_recoilT_mod.size(); ++i )
		Ex_vs_z_recoilT_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_z_recoil_random_mod.size(); ++i )
		Ex_vs_z_recoil_random_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_z_recoilT_random_mod.size(); ++i )
		Ex_vs_z_recoilT_random_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_z_recoil_cut.size(); ++i )
		Ex_vs_z_recoil_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_z_recoilT_cut.size(); ++i )
		Ex_vs_z_recoilT_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_z_recoil_random_cut.size(); ++i )
		Ex_vs_z_recoil_random_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_z_recoilT_random_cut.size(); ++i )
		Ex_vs_z_recoilT_random_cut[i]->Reset("ICESM");


	if( react->IsFission() && set->GetNumberOfCDLayers() > 0 ) {

		Ex_vs_z_fission->Reset("ICESM");
		Ex_vs_z_fissionT->Reset("ICESM");
		Ex_vs_z_fission_random->Reset("ICESM");
		Ex_vs_z_fissionT_random->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_vs_z_fission_mod.size(); ++i )
			Ex_vs_z_fission_mod[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_vs_z_fissionT_mod.size(); ++i )
			Ex_vs_z_fissionT_mod[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_vs_z_fission_random_mod.size(); ++i )
			Ex_vs_z_fission_random_mod[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_vs_z_fissionT_random_mod.size(); ++i )
			Ex_vs_z_fissionT_random_mod[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_vs_z_fission_cut.size(); ++i )
			Ex_vs_z_fission_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_vs_z_fissionT_cut.size(); ++i )
			Ex_vs_z_fissionT_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_vs_z_fission_random_cut.size(); ++i )
			Ex_vs_z_fission_random_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_vs_z_fissionT_random_cut.size(); ++i )
			Ex_vs_z_fissionT_random_cut[i]->Reset("ICESM");

	}

	for( unsigned int i = 0; i < Ex_vs_z_T1_cut.size(); ++i )
		Ex_vs_z_T1_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_z_mod.size(); ++i )
		Ex_vs_z_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_z_ebis_mod.size(); ++i )
		Ex_vs_z_ebis_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_z_ebis_on_mod.size(); ++i )
		Ex_vs_z_ebis_on_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_z_ebis_off_mod.size(); ++i )
		Ex_vs_z_ebis_off_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_z_cut.size(); ++i )
		Ex_vs_z_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_z_ebis_cut.size(); ++i )
		Ex_vs_z_ebis_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_z_ebis_on_cut.size(); ++i )
		Ex_vs_z_ebis_on_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_z_ebis_off_cut.size(); ++i )
		Ex_vs_z_ebis_off_cut[i]->Reset("ICESM");

	// Array - Ex
	Ex->Reset("ICESM");
	Ex_ebis->Reset("ICESM");
	Ex_ebis_on->Reset("ICESM");
	Ex_ebis_off->Reset("ICESM");
	Ex_T1->Reset("ICESM");
	Ex_vs_T1->Reset("ICESM");

	Ex_recoil->Reset("ICESM");
	Ex_recoilT->Reset("ICESM");
	Ex_recoil_random->Reset("ICESM");
	Ex_recoilT_random->Reset("ICESM");

	if( react->GammaRayHistsEnabled() ){

		Ex_gamma->Reset("ICESM");
		Ex_gammaT->Reset("ICESM");
		Ex_gamma_random->Reset("ICESM");
		Ex_gammaT_random->Reset("ICESM");

	}

	for( unsigned int i = 0; i < Ex_recoil_cut.size(); ++i )
		Ex_recoil_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_recoilT_cut.size(); ++i )
		Ex_recoilT_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_recoil_random_cut.size(); ++i )
		Ex_recoil_random_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_recoilT_random_cut.size(); ++i )
		Ex_recoilT_random_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_recoil_mod.size(); ++i )
		Ex_recoil_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_recoilT_mod.size(); ++i )
		Ex_recoilT_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_recoil_random_mod.size(); ++i )
		Ex_recoil_random_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_recoilT_random_mod.size(); ++i )
		Ex_recoilT_random_mod[i]->Reset("ICESM");


	if( react->IsFission() && set->GetNumberOfCDLayers() > 0 ) {

		Ex_fission->Reset("ICESM");
		Ex_fissionT->Reset("ICESM");
		Ex_fission_gamma->Reset("ICESM");
		Ex_fission_1FF->Reset("ICESM");
		Ex_fission_1FF_gamma->Reset("ICESM");
		Ex_fission_2FF->Reset("ICESM");
		Ex_fission_random->Reset("ICESM");
		Ex_fissionT_random->Reset("ICESM");
		Ex_fission_gamma_random->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_fission_cut.size(); ++i )
			Ex_fission_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_fissionT_cut.size(); ++i )
			Ex_fissionT_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_fission_random_cut.size(); ++i )
			Ex_fission_random_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_fissionT_random_cut.size(); ++i )
			Ex_fissionT_random_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_fission_mod.size(); ++i )
			Ex_fission_mod[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_fissionT_mod.size(); ++i )
			Ex_fissionT_mod[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_fission_random_mod.size(); ++i )
			Ex_fission_random_mod[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Ex_fissionT_random_mod.size(); ++i )
			Ex_fissionT_random_mod[i]->Reset("ICESM");

	}

	for( unsigned int i = 0; i < Ex_T1_cut.size(); ++i )
		Ex_T1_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_vs_T1_cut.size(); ++i )
		Ex_vs_T1_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_mod.size(); ++i )
		Ex_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_ebis_mod.size(); ++i )
		Ex_ebis_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_ebis_on_mod.size(); ++i )
		Ex_ebis_on_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_ebis_off_mod.size(); ++i )
		Ex_ebis_off_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_cut.size(); ++i )
		Ex_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_ebis_cut.size(); ++i )
		Ex_ebis_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_ebis_on_cut.size(); ++i )
		Ex_ebis_on_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Ex_ebis_off_cut.size(); ++i )
		Ex_ebis_off_cut[i]->Reset("ICESM");

	// Array - Theta
	Theta->Reset("ICESM");
	Theta_ebis->Reset("ICESM");
	Theta_ebis_on->Reset("ICESM");
	Theta_ebis_off->Reset("ICESM");
	Theta_T1->Reset("ICESM");

	Theta_recoil->Reset("ICESM");
	Theta_recoilT->Reset("ICESM");
	Theta_recoil_random->Reset("ICESM");
	Theta_recoilT_random->Reset("ICESM");

	if( react->GammaRayHistsEnabled() ){

		Theta_gamma->Reset("ICESM");
		Theta_gammaT->Reset("ICESM");
		Theta_gamma_random->Reset("ICESM");
		Theta_gammaT_random->Reset("ICESM");

	}

	for( unsigned int i = 0; i < Theta_recoil_cut.size(); ++i )
		Theta_recoil_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Theta_recoilT_cut.size(); ++i )
		Theta_recoilT_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Theta_recoil_random_cut.size(); ++i )
		Theta_recoil_random_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Theta_recoilT_random_cut.size(); ++i )
		Theta_recoilT_random_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Theta_recoil_mod.size(); ++i )
		Theta_recoil_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Theta_recoilT_mod.size(); ++i )
		Theta_recoilT_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Theta_recoil_random_mod.size(); ++i )
		Theta_recoil_random_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Theta_recoilT_random_mod.size(); ++i )
		Theta_recoilT_random_mod[i]->Reset("ICESM");

	if( react->IsFission() && set->GetNumberOfCDLayers() > 0 ) {

		Theta_fission->Reset("ICESM");
		Theta_fissionT->Reset("ICESM");
		Theta_fission_random->Reset("ICESM");
		Theta_fissionT_random->Reset("ICESM");

		for( unsigned int i = 0; i < Theta_fission_cut.size(); ++i )
			Theta_fission_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Theta_fissionT_cut.size(); ++i )
			Theta_fissionT_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Theta_fission_random_cut.size(); ++i )
			Theta_fission_random_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Theta_fissionT_random_cut.size(); ++i )
			Theta_fissionT_random_cut[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Theta_fission_mod.size(); ++i )
			Theta_fission_mod[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Theta_fissionT_mod.size(); ++i )
			Theta_fissionT_mod[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Theta_fission_random_mod.size(); ++i )
			Theta_fission_random_mod[i]->Reset("ICESM");

		for( unsigned int i = 0; i < Theta_fissionT_random_mod.size(); ++i )
			Theta_fissionT_random_mod[i]->Reset("ICESM");

	}

	for( unsigned int i = 0; i < Theta_T1_cut.size(); ++i )
		Theta_T1_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Theta_mod.size(); ++i )
		Theta_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Theta_ebis_mod.size(); ++i )
		Theta_ebis_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Theta_ebis_on_mod.size(); ++i )
		Theta_ebis_on_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Theta_ebis_off_mod.size(); ++i )
		Theta_ebis_off_mod[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Theta_cut.size(); ++i )
		Theta_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Theta_ebis_cut.size(); ++i )
		Theta_ebis_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Theta_ebis_on_cut.size(); ++i )
		Theta_ebis_on_cut[i]->Reset("ICESM");

	for( unsigned int i = 0; i < Theta_ebis_off_cut.size(); ++i )
		Theta_ebis_off_cut[i]->Reset("ICESM");


	// ELUM
	elum->Reset("ICESM");
	elum_ebis->Reset("ICESM");
	elum_ebis_on->Reset("ICESM");
	elum_ebis_off->Reset("ICESM");
	elum_vs_T1->Reset("ICESM");

	for( unsigned int i = 0; i < elum_sec.size(); ++i )
		elum_sec[i]->Reset("ICESM");

	for( unsigned int i = 0; i < elum_ebis_sec.size(); ++i )
		elum_ebis_sec[i]->Reset("ICESM");

	for( unsigned int i = 0; i < elum_ebis_on_sec.size(); ++i )
		elum_ebis_on_sec[i]->Reset("ICESM");

	for( unsigned int i = 0; i < elum_ebis_off_sec.size(); ++i )
		elum_ebis_off_sec[i]->Reset("ICESM");

	if( !react->IsFission() ) {

		elum_recoil->Reset("ICESM");
		elum_recoilT->Reset("ICESM");
		elum_recoil_random->Reset("ICESM");
		elum_recoilT_random->Reset("ICESM");

		for( unsigned int i = 0; i < elum_recoil_sec.size(); ++i )
			elum_recoil_sec[i]->Reset("ICESM");

		for( unsigned int i = 0; i < elum_recoilT_sec.size(); ++i )
			elum_recoilT_sec[i]->Reset("ICESM");

		for( unsigned int i = 0; i < elum_recoil_random_sec.size(); ++i )
			elum_recoil_random_sec[i]->Reset("ICESM");

		for( unsigned int i = 0; i < elum_recoilT_random_sec.size(); ++i )
			elum_recoilT_random_sec[i]->Reset("ICESM");

	}

	lume_recoil->Reset("ICESM");
	lume_recoilT->Reset("ICESM");
	lume_recoil_random->Reset("ICESM");
	lume_recoilT_random->Reset("ICESM");

	for( unsigned int i = 0; i < lume_recoil_det.size(); ++i )
		lume_recoilT_det[i]->Reset("ICESM");

	for( unsigned int i = 0; i < lume_recoilT_det.size(); ++i )
		lume_recoilT_det[i]->Reset("ICESM");

	for( unsigned int i = 0; i < lume_recoil_random_det.size(); ++i )
		lume_recoil_random_det[i]->Reset("ICESM");

	for( unsigned int i = 0; i < lume_recoilT_random_det.size(); ++i )
		lume_recoilT_random_det[i]->Reset("ICESM");


	// LUME (All vectors have the same size.)
	for( unsigned int i = 0; i < lume_det.size(); ++i ) {
		lume_det[i]->Reset("ICESM");
		lume_ebis_det[i]->Reset("ICESM");
		lume_ebis_on_det[i]->Reset("ICESM");
		lume_ebis_off_det[i]->Reset("ICESM");
		lume_recoil_det[i]->Reset("ICESM");
		lume_recoilT_det[i]->Reset("ICESM");
		lume_recoil_random_det[i]->Reset("ICESM");
		lume_recoilT_random_det[i]->Reset("ICESM");
		lume_E_vs_x_det[i]->Reset("ICESM");
		lume_E_vs_x_ebis_det[i]->Reset("ICESM");
		lume_E_vs_x_ebis_on_det[i]->Reset("ICESM");
		lume_E_vs_x_ebis_off_det[i]->Reset("ICESM");
	}

	lume->Reset("ICESM");
	lume_E_vs_x->Reset("ICESM");
	lume_E_vs_x_wide->Reset("ICESM");
	lume_ebis->Reset("ICESM");
	lume_ebis_on->Reset("ICESM");
	lume_ebis_off->Reset("ICESM");
	lume_recoil->Reset("ICESM");
	lume_recoilT->Reset("ICESM");
	lume_recoil_random->Reset("ICESM");
	lume_recoilT_random->Reset("ICESM");
	lume_vs_T1->Reset("ICESM");
	lume_E_vs_x_ebis->Reset("ICESM");
	lume_E_vs_x_ebis_on->Reset("ICESM");
	lume_E_vs_x_ebis_off->Reset("ICESM");

	if( react->GammaRayHistsEnabled() ){

		gamma_ebis->Reset("ICESM");
		gamma_ebis_on->Reset("ICESM");
		gamma_ebis_off->Reset("ICESM");
		gamma_gamma_ebis->Reset("ICESM");
		gamma_fission->Reset("ICESM");
		gamma_recoil->Reset("ICESM");
		gamma_recoilT->Reset("ICESM");
		gamma_array->Reset("ICESM");
		gamma_gamma_fission->Reset("ICESM");
		gamma_gamma_recoil->Reset("ICESM");
		gamma_gamma_array->Reset("ICESM");
		gamma_Ex_ebis->Reset("ICESM");
		gamma_Ex_fission->Reset("ICESM");
		gamma_Ex_recoil->Reset("ICESM");
		gamma_gamma_td->Reset("ICESM");
		gamma_fission_td->Reset("ICESM");
		gamma_recoil_td->Reset("ICESM");
		gamma_array_td->Reset("ICESM");

		for( unsigned int i = 0; i < gamma_array_cut.size(); ++i )
			gamma_array_cut[i]->Reset("ICESM");
		for( unsigned int i = 0; i < gamma_gamma_array_cut.size(); ++i )
			gamma_gamma_array_cut[i]->Reset("ICESM");

	}

	return;

}

unsigned long ISSHistogrammer::FillHists() {

	/// Main function to fill the histograms
	n_entries = input_tree->GetEntries();

	std::cout << " ISSHistogrammer: number of entries in event tree = ";
	std::cout << n_entries << std::endl;

	if( !n_entries ){

		std::cout << " ISSHistogrammer: Nothing to do..." << std::endl;
		return n_entries;

	}
	else {

		std::cout << " ISSHistogrammer: Start filling histograms" << std::endl;

	}

	// ------------------------------------------------------------------------ //
	// Main loop over TTree to find events
	// ------------------------------------------------------------------------ //
	for( unsigned int i = 0; i < n_entries; ++i ){

		// Current event data
		input_tree->GetEntry(i);

		// Check laser mode
		if( react->GetLaserMode() == 0 && read_evts->GetLaserStatus() ) continue;
		if( react->GetLaserMode() == 1 && !read_evts->GetLaserStatus() ) continue;

		// tdiff variable
		double tdiff;

		// Check the array mode, if we have p-side only or not
		bool psideonly = react->GetArrayHistMode();
		unsigned int array_mult = read_evts->GetArrayMultiplicity();
		if( psideonly ) array_mult = read_evts->GetArrayPMultiplicity();

		// Fill the multiplicity plots
		if( read_evts->GetCDMultiplicity() || array_mult )
			mult_array_fission->Fill( read_evts->GetCDMultiplicity(), array_mult );
		if( read_evts->GetRecoilMultiplicity() || array_mult )
			mult_array_recoil->Fill( read_evts->GetRecoilMultiplicity(), array_mult );
		if( read_evts->GetGammaRayMultiplicity() || array_mult )
			mult_array_gamma->Fill( read_evts->GetGammaRayMultiplicity(), array_mult );
		if( read_evts->GetCDMultiplicity() || read_evts->GetGammaRayMultiplicity() )
			mult_gamma_fission->Fill( read_evts->GetCDMultiplicity(), read_evts->GetGammaRayMultiplicity() );
		if( read_evts->GetRecoilMultiplicity() || read_evts->GetGammaRayMultiplicity() )
			mult_gamma_recoil->Fill( read_evts->GetRecoilMultiplicity(), read_evts->GetGammaRayMultiplicity() );


		// Loop over array events
		for( unsigned int j = 0; j < array_mult; ++j ){

			// Get array event - GetArrayEvt is "normal" mode, GetArrayPEvt is p-side only
			if( psideonly ) array_evt = read_evts->GetArrayPEvt(j);
			else array_evt = read_evts->GetArrayEvt(j);

			// Do the reaction
			react->MakeReaction( array_evt->GetPosition(), array_evt->GetEnergy() );

			// Setup the output tree if user wants it
			if( react->RxTreeEnabled() ) {

				// This is an array event after the reaction is calculated
				rx_evts->SetRxEvent( react, array_evt->GetEnergy(),
									array_evt->GetTime() - read_evts->GetEBIS(),
									array_evt->GetTime() - read_evts->GetT1(),
									read_evts->GetLaserStatus() );

				// Then fill the tree
				output_tree->Fill();

				// Clean up if the next event is going to make the tree full
				if( output_tree->MemoryFull(30e6) )
					output_tree->DropBaskets();

			}


			// Singles
			E_vs_z->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
			E_vs_z_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
			Theta->Fill( react->GetThetaCM() * TMath::RadToDeg() );
			Theta_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
			Ex->Fill( react->GetEx() );
			Ex_mod[array_evt->GetModule()]->Fill( react->GetEx() );
			E_vs_theta->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
			E_vs_theta_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
			Ex_vs_theta->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
			Ex_vs_theta_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
			Ex_vs_z->Fill( react->GetZmeasured(), react->GetEx() );
			Ex_vs_z_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), react->GetEx() );

			// Look for gamma-rays in coincidence with the array
			if( react->GammaRayHistsEnabled() ) {

				// Loop over gamma-ray events
				for( unsigned int k = 0; k < read_evts->GetGammaRayMultiplicity(); ++k ) {

					// Get event
					gamma_evt1 = read_evts->GetGammaRayEvt(k);

					// Time difference
					gamma_array_td->Fill( gamma_evt1->GetTime() - array_evt->GetTime() );

					// Egamma (no Doppler correction yet applied)
					if( PromptCoincidence( gamma_evt1, array_evt ) )
						gamma_array->Fill( gamma_evt1->GetEnergy() );

					else if( RandomCoincidence( gamma_evt1, array_evt ) )
						gamma_array->Fill( gamma_evt1->GetEnergy(), -1.0 * react->GetArrayGammaFillRatio() );

					// Loop over second gamma-ray events
					for( unsigned int l = 0; l < read_evts->GetGammaRayMultiplicity(); ++l ) {

						// Skip self coincidence
						if( l == k ) continue;

						// Get event
						gamma_evt2 = read_evts->GetGammaRayEvt(l);

						// Egamma matrix (no Doppler correction yet applied)
						if( PromptCoincidence( gamma_evt1, array_evt ) && PromptCoincidence( gamma_evt1, gamma_evt2 ) )
							gamma_gamma_array->Fill( gamma_evt1->GetEnergy(), gamma_evt2->GetEnergy() );

						else if( RandomCoincidence( gamma_evt1, array_evt ) && PromptCoincidence( gamma_evt1, gamma_evt2 ) )
							gamma_gamma_array->Fill( gamma_evt1->GetEnergy(), gamma_evt2->GetEnergy(), -1.0 * react->GetArrayGammaFillRatio() );

						else if( PromptCoincidence( gamma_evt1, array_evt ) && RandomCoincidence( gamma_evt1, gamma_evt2 ) )
							gamma_gamma_array->Fill( gamma_evt1->GetEnergy(), gamma_evt2->GetEnergy(), -1.0 * react->GetGammaGammaFillRatio() );

						else if( RandomCoincidence( gamma_evt1, array_evt ) && RandomCoincidence( gamma_evt1, gamma_evt2 ) )
							gamma_gamma_array->Fill( gamma_evt1->GetEnergy(), gamma_evt2->GetEnergy(), react->GetGammaGammaFillRatio() * react->GetArrayGammaFillRatio());

					} // l

				} // k

			} // gamma-ray hists enabled

			// Check the E vs z cuts from the user
			for( unsigned int k = 0; k < react->GetNumberOfEvsZCuts(); ++k ){

				// Is inside the cut
				if( react->GetEvsZCut(k)->IsInside( react->GetZmeasured(), array_evt->GetEnergy() ) ){

					E_vs_z_cut[k]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
					Theta_cut[k]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
					Ex_cut[k]->Fill( react->GetEx() );
					E_vs_theta_cut[k]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
					Ex_vs_theta_cut[k]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
					Ex_vs_z_cut[k]->Fill( react->GetZmeasured(), react->GetEx() );

					// Look for gamma-rays in coincidence with the array
					if( react->GammaRayHistsEnabled() ) {

						// Loop over gamma-ray events
						for( unsigned int l = 0; l < read_evts->GetGammaRayMultiplicity(); ++l ) {

							// Get event
							gamma_evt1 = read_evts->GetGammaRayEvt(l);

							// Egamma matrix (no Doppler correction yet applied)
							if( PromptCoincidence( gamma_evt1, array_evt ) )
								gamma_array_cut[k]->Fill( gamma_evt1->GetEnergy() );

							else if( RandomCoincidence( gamma_evt1, array_evt ) )
								gamma_array_cut[k]->Fill( gamma_evt1->GetEnergy(), -1.0 * react->GetArrayGammaFillRatio() );

							// Loop over second gamma-ray events
							for( unsigned int m = 0; m < read_evts->GetGammaRayMultiplicity(); ++m ) {

								// Skip self coincidence
								if( m == l ) continue;

								// Get event
								gamma_evt2 = read_evts->GetGammaRayEvt(m);

								// Egamma matrix (no Doppler correction yet applied)
								if( PromptCoincidence( gamma_evt1, array_evt ) && PromptCoincidence( gamma_evt1, gamma_evt2 ) )
									gamma_gamma_array_cut[k]->Fill( gamma_evt1->GetEnergy(), gamma_evt2->GetEnergy() );

								else if( RandomCoincidence( gamma_evt1, array_evt ) && PromptCoincidence( gamma_evt1, gamma_evt2 ) )
									gamma_gamma_array_cut[k]->Fill( gamma_evt1->GetEnergy(), gamma_evt2->GetEnergy(), -1.0 * react->GetArrayGammaFillRatio() );

								else if( PromptCoincidence( gamma_evt1, array_evt ) && RandomCoincidence( gamma_evt1, gamma_evt2 ) )
									gamma_gamma_array_cut[k]->Fill( gamma_evt1->GetEnergy(), gamma_evt2->GetEnergy(), -1.0 * react->GetGammaGammaFillRatio() );

								else if( RandomCoincidence( gamma_evt1, array_evt ) && RandomCoincidence( gamma_evt1, gamma_evt2 ) )
									gamma_gamma_array_cut[k]->Fill( gamma_evt1->GetEnergy(), gamma_evt2->GetEnergy(), react->GetGammaGammaFillRatio() * react->GetArrayGammaFillRatio());

							} // m

						} // l

					} // if gamma-ray hists enabled

				} // inside cut

			} // loop over cuts


			// EBIS time
			ebis_td_array->Fill( array_evt->GetTime() - read_evts->GetEBIS() );

			// Check for events in the EBIS on-beam window
			if( OnBeam( array_evt ) ){

				E_vs_z_ebis->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
				E_vs_z_ebis_on->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
				E_vs_z_ebis_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
				E_vs_z_ebis_on_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
				Theta_ebis->Fill( react->GetThetaCM() * TMath::RadToDeg() );
				Theta_ebis_on->Fill( react->GetThetaCM() * TMath::RadToDeg() );
				Theta_ebis_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
				Theta_ebis_on_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
				Ex_ebis->Fill( react->GetEx() );
				Ex_ebis_on->Fill( react->GetEx() );
				Ex_ebis_mod[array_evt->GetModule()]->Fill( react->GetEx() );
				Ex_ebis_on_mod[array_evt->GetModule()]->Fill( react->GetEx() );
				E_vs_theta_ebis->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy());
				E_vs_theta_ebis_on->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
				E_vs_theta_ebis_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
				E_vs_theta_ebis_on_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
				Ex_vs_theta_ebis->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
				Ex_vs_theta_ebis_on->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
				Ex_vs_theta_ebis_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
				Ex_vs_theta_ebis_on_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
				Ex_vs_z_ebis->Fill( react->GetZmeasured(), react->GetEx() );
				Ex_vs_z_ebis_on->Fill( react->GetZmeasured(), react->GetEx() );
				Ex_vs_z_ebis_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), react->GetEx() );
				Ex_vs_z_ebis_on_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), react->GetEx() );

				// Look for gamma-rays in coincidence with the array
				if( react->GammaRayHistsEnabled() ) {

					// Loop over gamma-ray events
					for( unsigned int k = 0; k < read_evts->GetGammaRayMultiplicity(); ++k ) {

						// Get event
						gamma_evt1 = read_evts->GetGammaRayEvt(k);

						// Ex versus Egamma (no Doppler correction yet applied)
						if( PromptCoincidence( gamma_evt1, array_evt ) )
							gamma_Ex_ebis->Fill( gamma_evt1->GetEnergy(), react->GetEx() );

						else if( RandomCoincidence( gamma_evt1, array_evt ) )
							gamma_Ex_ebis->Fill( gamma_evt1->GetEnergy(), react->GetEx(), -1.0 * react->GetArrayGammaFillRatio() );

					} // k

				} // gamma-ray hists enabled

				// Check for events in the user-defined T1 window
				Ex_vs_T1->Fill( array_evt->GetTime() - read_evts->GetT1(), react->GetEx() );
				if( T1Cut( array_evt ) ) {

					E_vs_z_T1->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
					Theta_T1->Fill( react->GetThetaCM() * TMath::RadToDeg() );
					Ex_T1->Fill( react->GetEx() );
					E_vs_theta_T1->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
					Ex_vs_theta_T1->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
					Ex_vs_z_T1->Fill( react->GetZmeasured(), react->GetEx() );

				} // T1

				// Check the E vs z cuts from the user
				for( unsigned int k = 0; k < react->GetNumberOfEvsZCuts(); ++k ){

					// Is inside the cut
					if( react->GetEvsZCut(k)->IsInside( react->GetZmeasured(), array_evt->GetEnergy() ) ){

						E_vs_z_ebis_cut[k]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
						E_vs_z_ebis_on_cut[k]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
						Theta_ebis_cut[k]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
						Theta_ebis_on_cut[k]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
						Ex_ebis_cut[k]->Fill( react->GetEx() );
						Ex_ebis_on_cut[k]->Fill( react->GetEx() );
						E_vs_theta_ebis_cut[k]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
						E_vs_theta_ebis_on_cut[k]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
						Ex_vs_theta_ebis_cut[k]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
						Ex_vs_theta_ebis_on_cut[k]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
						Ex_vs_z_ebis_cut[k]->Fill( react->GetZmeasured(), react->GetEx() );
						Ex_vs_z_ebis_on_cut[k]->Fill( react->GetZmeasured(), react->GetEx() );

						// Check for events in the user-defined T1 window
						Ex_vs_T1_cut[k]->Fill( array_evt->GetTime() - read_evts->GetT1(), react->GetEx() );
						if( T1Cut( array_evt ) ) {

							E_vs_z_T1_cut[k]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
							Theta_T1_cut[k]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
							Ex_T1_cut[k]->Fill( react->GetEx() );
							E_vs_theta_T1_cut[k]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
							Ex_vs_theta_T1_cut[k]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
							Ex_vs_z_T1_cut[k]->Fill( react->GetZmeasured(), react->GetEx() );

						} // T1

					} // inside cut

				} // loop over cuts

			} // ebis

			else if( OffBeam( array_evt ) ){

				E_vs_z_ebis->Fill( react->GetZmeasured(), array_evt->GetEnergy(), -1.0 * react->GetEBISFillRatio() );
				E_vs_z_ebis_off->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
				E_vs_z_ebis_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), array_evt->GetEnergy(), -1.0 * react->GetEBISFillRatio() );
				E_vs_z_ebis_off_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
				Theta_ebis->Fill( react->GetThetaCM() * TMath::RadToDeg(), -1.0 * react->GetEBISFillRatio() );
				Theta_ebis_off->Fill( react->GetThetaCM() * TMath::RadToDeg() );
				Theta_ebis_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), -1.0 * react->GetEBISFillRatio() );
				Theta_ebis_off_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
				Ex_ebis->Fill( react->GetEx(), -1.0 * react->GetEBISFillRatio() );
				Ex_ebis_off->Fill( react->GetEx() );
				Ex_ebis_mod[array_evt->GetModule()]->Fill( react->GetEx(), -1.0 * react->GetEBISFillRatio() );
				Ex_ebis_off_mod[array_evt->GetModule()]->Fill( react->GetEx() );
				E_vs_theta_ebis->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy(), -1.0 * react->GetEBISFillRatio());
				E_vs_theta_ebis_off->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
				E_vs_theta_ebis_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy(), -1.0 * react->GetEBISFillRatio() );
				E_vs_theta_ebis_off_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
				Ex_vs_theta_ebis->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx(), -1.0 * react->GetEBISFillRatio() );
				Ex_vs_theta_ebis_off->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
				Ex_vs_theta_ebis_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx(), -1.0 * react->GetEBISFillRatio() );
				Ex_vs_theta_ebis_off_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
				Ex_vs_z_ebis->Fill( react->GetZmeasured(), react->GetEx(), -1.0 * react->GetEBISFillRatio() );
				Ex_vs_z_ebis_off->Fill( react->GetZmeasured(), react->GetEx() );
				Ex_vs_z_ebis_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), react->GetEx(), -1.0 * react->GetEBISFillRatio() );
				Ex_vs_z_ebis_off_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), react->GetEx() );

				// Look for gamma-rays in coincidence with the array
				if( react->GammaRayHistsEnabled() ) {

					// Loop over gamma-ray events
					for( unsigned int k = 0; k < read_evts->GetGammaRayMultiplicity(); ++k ) {

						// Get event
						gamma_evt1 = read_evts->GetGammaRayEvt(k);

						// Ex versus Egamma (no Doppler correction yet applied)
						if( PromptCoincidence( gamma_evt1, array_evt ) )
							gamma_Ex_ebis->Fill( gamma_evt1->GetEnergy(), react->GetEx(), -1.0 * react->GetEBISFillRatio() );

						else if( RandomCoincidence( gamma_evt1, array_evt ) )
							gamma_Ex_ebis->Fill( gamma_evt1->GetEnergy(), react->GetEx(), -1.0 * react->GetEBISFillRatio() * react->GetArrayGammaFillRatio() );

					} // k

				} // gamma-ray hists enabled

				// Check for events in the user-defined T1 window
				Ex_vs_T1->Fill( array_evt->GetTime() - read_evts->GetT1(), react->GetEx(), -1.0 * react->GetEBISFillRatio() );
				if( T1Cut( array_evt ) ) {

					E_vs_z_T1->Fill( react->GetZmeasured(), array_evt->GetEnergy(), -1.0 * react->GetEBISFillRatio() );
					Theta_T1->Fill( react->GetThetaCM() * TMath::RadToDeg(), -1.0 * react->GetEBISFillRatio() );
					Ex_T1->Fill( react->GetEx(), -1.0 * react->GetEBISFillRatio() );
					E_vs_theta_T1->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy(), -1.0 * react->GetEBISFillRatio() );
					Ex_vs_theta_T1->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx(), -1.0 * react->GetEBISFillRatio() );
					Ex_vs_z_T1->Fill( react->GetZmeasured(), react->GetEx(), -1.0 * react->GetEBISFillRatio() );

				} // T1

				// Check the E vs z cuts from the user
				for( unsigned int k = 0; k < react->GetNumberOfEvsZCuts(); ++k ){

					// Is inside the cut
					if( react->GetEvsZCut(k)->IsInside( react->GetZmeasured(), array_evt->GetEnergy() ) ){

						E_vs_z_ebis_cut[k]->Fill( react->GetZmeasured(), array_evt->GetEnergy(), -1.0 * react->GetEBISFillRatio() );
						E_vs_z_ebis_off_cut[k]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
						Theta_ebis_cut[k]->Fill( react->GetThetaCM() * TMath::RadToDeg(), -1.0 * react->GetEBISFillRatio() );
						Theta_ebis_off_cut[k]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
						Ex_ebis_cut[k]->Fill( react->GetEx(), -1.0 * react->GetEBISFillRatio() );
						Ex_ebis_off_cut[k]->Fill( react->GetEx() );
						E_vs_theta_ebis_cut[k]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy(), -1.0 * react->GetEBISFillRatio() );
						E_vs_theta_ebis_off_cut[k]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
						Ex_vs_theta_ebis_cut[k]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx(), -1.0 * react->GetEBISFillRatio() );
						Ex_vs_theta_ebis_off_cut[k]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
						Ex_vs_z_ebis_cut[k]->Fill( react->GetZmeasured(), react->GetEx(), -1.0 * react->GetEBISFillRatio() );
						Ex_vs_z_ebis_off_cut[k]->Fill( react->GetZmeasured(), react->GetEx() );

						// Check for events in the user-defined T1 window
						Ex_vs_T1_cut[k]->Fill( array_evt->GetTime() - read_evts->GetT1(), react->GetEx(), -1.0 * react->GetEBISFillRatio() );
						if( T1Cut( array_evt ) ) {

							E_vs_z_T1_cut[k]->Fill( react->GetZmeasured(), array_evt->GetEnergy(), -1.0 * react->GetEBISFillRatio() );
							Theta_T1_cut[k]->Fill( react->GetThetaCM() * TMath::RadToDeg(), -1.0 * react->GetEBISFillRatio() );
							Ex_T1_cut[k]->Fill( react->GetEx(), -1.0 * react->GetEBISFillRatio() );
							E_vs_theta_T1_cut[k]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy(), -1.0 * react->GetEBISFillRatio() );
							Ex_vs_theta_T1_cut[k]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx(), -1.0 * react->GetEBISFillRatio() );
							Ex_vs_z_T1_cut[k]->Fill( react->GetZmeasured(), react->GetEx(), -1.0 * react->GetEBISFillRatio() );

						} // T1

					} // inside cut

				} // loop over cuts

			} // off ebis


			// Loop over recoil or fission events
			bool promptcheckT = false;
			bool randomcheckT = false;
			bool promptcheckE = false;
			bool randomcheckE = false;
			bool energycut = false;
			std::vector<unsigned int> promptgammaidx;
			std::vector<unsigned int> randomgammaidx;
			std::vector<unsigned int> promptgammaidx1FF;
			bool has1FF = false;
			bool has2FF = false;
			bool has1FFgamma = false;

			// If we have fission mode
			if( react->IsFission() && set->GetNumberOfCDLayers() > 0 ) {

				// Loop over CD events to check for random and prompt coincidences
				for( unsigned int k = 0; k < read_evts->GetCDMultiplicity(); ++k ){

					// Get CD event
					cd_evt1 = read_evts->GetCDEvt(k);

					if (  PromptCoincidence( cd_evt1, array_evt ) && cd_evt1->GetEnergyTotal() > 5e5 )
					  has1FF = true;

					// Time differences
					tdiff = cd_evt1->GetTime() - array_evt->GetTime();
					fission_array_td[array_evt->GetModule()]->Fill( tdiff );

					if( array_evt->GetPHit() ) { // hit bit = true

						fission_array_tw_hit1->Fill( tdiff, array_evt->GetEnergy() );
						fission_array_tw_hit1_prof->Fill( array_evt->GetEnergy(), tdiff );
						fission_array_tw_hit1_row[array_evt->GetModule()][array_evt->GetRow()]->Fill( tdiff, array_evt->GetEnergy() );

					}

					else { // hit bit = false

						fission_array_tw_hit0->Fill( tdiff, array_evt->GetEnergy() );
						fission_array_tw_hit0_prof->Fill( array_evt->GetEnergy(), tdiff );
						fission_array_tw_hit0_row[array_evt->GetModule()][array_evt->GetRow()]->Fill( tdiff, array_evt->GetEnergy() );

					}

					// Loop over coincident CD events
					for( unsigned int l = 0; l < read_evts->GetCDMultiplicity(); ++l ){

						// Skip self-coincidences
						if( k == l ) continue;

						// Get CD event
						cd_evt2 = read_evts->GetCDEvt(l);

						// Check for prompt events with coincident fissions that are "back-to-back"
						if( PromptCoincidence( cd_evt1, array_evt ) && PromptCoincidence( cd_evt1, cd_evt2 ) &&
						   TMath::Abs( cd_evt1->GetSector() - cd_evt2->GetSector() ) >= 0.5*set->GetNumberOfCDSectors()-2 &&
						   TMath::Abs( cd_evt1->GetSector() - cd_evt2->GetSector() ) <= 0.5*set->GetNumberOfCDSectors()+2 ){
						  if ( cd_evt2->GetEnergyTotal() > 5e5 && cd_evt1->GetEnergyTotal() > 5e5 ){
							has2FF = true;
						  }

							promptcheckT = true;

							// Check energy gate
							if( FissionCutHeavy( cd_evt1 ) && FissionCutLight( cd_evt2 ) ) {

								promptcheckE = true;

								// Search for coincident prompt gamma-rays
								for( unsigned int m = 0; m < read_evts->GetGammaRayMultiplicity(); ++m ){

									gamma_evt1 = read_evts->GetGammaRayEvt(m);

									// These gammas are coincident with the fission event, AND coincident with the array
									if( PromptCoincidence( gamma_evt1, array_evt ) &&
									    PromptCoincidence( gamma_evt1, cd_evt1 ) ) {

										// Add this gamma to the list if it hasn't been already
										if( std::find( promptgammaidx.begin(), promptgammaidx.end(), m ) == promptgammaidx.end() )
											promptgammaidx.push_back(m);

									} // fission-gamma AND gamma-array coincidence

								} // m

							} // fission energy cuts

						} // fission-fission AND fission-array time

						// Check for random events with coincident fissions
						if( RandomCoincidence( cd_evt1, array_evt ) && PromptCoincidence( cd_evt1, cd_evt2 ) ){
							randomcheckT = true;

							// Check energy gate
							if( FissionCutHeavy( cd_evt1 ) && FissionCutLight( cd_evt2 ) ) {

								randomcheckE = true;

								// Search for coincident prompt gamma-rays
								for( unsigned int m = 0; m < read_evts->GetGammaRayMultiplicity(); ++m ){

									gamma_evt1 = read_evts->GetGammaRayEvt(m);

									// These gammas are coincident with the fission event, but random with the array
									if( RandomCoincidence( gamma_evt1, array_evt ) &&
									    PromptCoincidence( gamma_evt1, cd_evt1 ) ) {

										// Add this gamma to the list if it hasn't been already
										if( std::find( randomgammaidx.begin(), randomgammaidx.end(), m ) == randomgammaidx.end() )
											randomgammaidx.push_back(m);

									} // fission-gamma AND gamma-array coincidence

								} // m

							} // fission energy cuts

						} // fission-fission AND fission-array time

					} // cd events 2

					// Another loop for checking array - 1 FF - gamma coincidence
					for( unsigned int m = 0; m < read_evts->GetGammaRayMultiplicity(); ++m ){

					  gamma_evt1 = read_evts->GetGammaRayEvt(m);

					  if( PromptCoincidence( gamma_evt1, array_evt ) )
						  has1FFgamma = true;

					}

				} // cd events 1

				// Fill Ex histogram gated on just 1 FF if its energy is larger than 0
				if ( has1FF ){
				  Ex_fission_1FF->Fill( react->GetEx() );

				  if ( has1FFgamma )
					Ex_fission_1FF_gamma->Fill( react->GetEx() );
				}
				if (has2FF)
				  Ex_fission_2FF->Fill( react->GetEx() );

				// Fill prompt hists
				if( promptcheckT == true ){

					// Fission fragments in coincidence with an array event
					fission_EdE_array->Fill( cd_evt1->GetEnergyRest( set->GetRecoilEnergyRestStart(), set->GetRecoilEnergyRestStop() ),
											cd_evt1->GetEnergyLoss( set->GetRecoilEnergyLossStart(), set->GetRecoilEnergyLossStop() ) );
					fission_EdE_array->Fill( cd_evt2->GetEnergyRest( set->GetRecoilEnergyRestStart(), set->GetRecoilEnergyRestStop() ),
											cd_evt2->GetEnergyLoss( set->GetRecoilEnergyLossStart(), set->GetRecoilEnergyLossStop() ) );
					fission_fission_dEdE_array->Fill( cd_evt1->GetEnergyLoss( set->GetRecoilEnergyLossStart(), set->GetRecoilEnergyLossStop() ),
													 cd_evt2->GetEnergyLoss( set->GetRecoilEnergyLossStart(), set->GetRecoilEnergyLossStop() ) );

					// Array histograms
					E_vs_z_fissionT->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
					E_vs_z_fissionT_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
					Theta_fissionT->Fill( react->GetThetaCM() * TMath::RadToDeg() );
					Theta_fissionT_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
					Ex_fissionT->Fill( react->GetEx() );
					Ex_fissionT_mod[array_evt->GetModule()]->Fill( react->GetEx() );
					E_vs_theta_fissionT->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
					E_vs_theta_fissionT_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
					Ex_vs_theta_fissionT->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
					Ex_vs_theta_fissionT_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
					Ex_vs_z_fissionT->Fill( react->GetZmeasured(), react->GetEx() );
					Ex_vs_z_fissionT_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), react->GetEx() );

					// Check the E vs z cuts from the user
					for( unsigned int l = 0; l < react->GetNumberOfEvsZCuts(); ++l ){

						// Is inside the cut
						if( react->GetEvsZCut(l)->IsInside( react->GetZmeasured(), array_evt->GetEnergy() ) ){

							E_vs_z_fissionT_cut[l]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
							Theta_fissionT_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
							Ex_fissionT_cut[l]->Fill( react->GetEx() );
							E_vs_theta_fissionT_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
							Ex_vs_theta_fissionT_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
							Ex_vs_z_fissionT_cut[l]->Fill( react->GetZmeasured(), react->GetEx() );

						} // inside cut

					} // loop over cuts

					// Fill energy gate hists
					if( promptcheckE == true ) {

						E_vs_z_fission->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
						E_vs_z_fission_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
						Theta_fission->Fill( react->GetThetaCM() * TMath::RadToDeg() );
						Theta_fission_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
						Ex_fission->Fill( react->GetEx() );
						Ex_fission_mod[array_evt->GetModule()]->Fill( react->GetEx() );
						E_vs_theta_fission->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
						E_vs_theta_fission_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
						Ex_vs_theta_fission->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
						Ex_vs_theta_fission_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
						Ex_vs_z_fission->Fill( react->GetZmeasured(), react->GetEx() );
						Ex_vs_z_fission_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), react->GetEx() );

						// Check the E vs z cuts from the user
						for( unsigned int l = 0; l < react->GetNumberOfEvsZCuts(); ++l ){

							// Is inside the cut
							if( react->GetEvsZCut(l)->IsInside( react->GetZmeasured(), array_evt->GetEnergy() ) ){

								E_vs_z_fission_cut[l]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
								Theta_fission_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
								Ex_fission_cut[l]->Fill( react->GetEx() );
								E_vs_theta_fission_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
								Ex_vs_theta_fission_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
								Ex_vs_z_fission_cut[l]->Fill( react->GetZmeasured(), react->GetEx() );

							} // inside cut

						} // loop over cuts

					} // energy cuts

				} // prompt

				// Fill random hists, even if we filled it already as a prompt hit...
				// Justification is that larger random windows can be used whilst keeping
				// the event ratio for prompt and random consistent. Using an else here would
				// mean that the scaling of the randoms would not increase linearly with window width.
				// Some events will be added and subtracted if there is a prompt AND random coinicidence
				// with two different fission fragments, so the error bar goes up, but if the random
				// window is larger, the weighting of the random will be <1 and a small contribution.
				if( randomcheckT == true ){

					// Array histograms
					E_vs_z_fissionT_random->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
					E_vs_z_fissionT_random_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
					Theta_fissionT_random->Fill( react->GetThetaCM() * TMath::RadToDeg() );
					Theta_fissionT_random_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
					Ex_fissionT_random->Fill( react->GetEx() );
					Ex_fissionT_random_mod[array_evt->GetModule()]->Fill( react->GetEx() );
					E_vs_theta_fissionT_random->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
					E_vs_theta_fissionT_random_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
					Ex_vs_theta_fissionT_random->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
					Ex_vs_theta_fissionT_random_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
					Ex_vs_z_fissionT_random->Fill( react->GetZmeasured(), react->GetEx() );
					Ex_vs_z_fissionT_random_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), react->GetEx() );

					// Check the E vs z cuts from the user
					for( unsigned int l = 0; l < react->GetNumberOfEvsZCuts(); ++l ){

						// Is inside the cut
						if( react->GetEvsZCut(l)->IsInside( react->GetZmeasured(), array_evt->GetEnergy() ) ){

							E_vs_z_fissionT_random_cut[l]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
							Theta_fissionT_random_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
							Ex_fissionT_random_cut[l]->Fill( react->GetEx() );
							E_vs_theta_fissionT_random_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
							Ex_vs_theta_fissionT_random_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
							Ex_vs_z_fissionT_random_cut[l]->Fill( react->GetZmeasured(), react->GetEx() );

						} // inside cut

					} // loop over cuts

					// Fill energy gate hists
					if( randomcheckE == true ) {

						E_vs_z_fission_random->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
						E_vs_z_fission_random_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
						Theta_fission_random->Fill( react->GetThetaCM() * TMath::RadToDeg() );
						Theta_fission_random_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
						Ex_fission_random->Fill( react->GetEx() );
						Ex_fission_random_mod[array_evt->GetModule()]->Fill( react->GetEx() );
						E_vs_theta_fission_random->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
						E_vs_theta_fission_random_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
						Ex_vs_theta_fission_random->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
						Ex_vs_theta_fission_random_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
						Ex_vs_z_fission_random->Fill( react->GetZmeasured(), react->GetEx() );
						Ex_vs_z_fission_random_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), react->GetEx() );

						// Check the E vs z cuts from the user
						for( unsigned int l = 0; l < react->GetNumberOfEvsZCuts(); ++l ){

							// Is inside the cut
							if( react->GetEvsZCut(l)->IsInside( react->GetZmeasured(), array_evt->GetEnergy() ) ){

								E_vs_z_fission_random_cut[l]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
								Theta_fission_random_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
								Ex_fission_random_cut[l]->Fill( react->GetEx() );
								Ex_vs_theta_fission_random_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
								Ex_vs_theta_fission_random_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
								Ex_vs_z_fission_random_cut[l]->Fill( react->GetZmeasured(), react->GetEx() );

							} // inside cut

						} // loop over cuts

					} // energy cuts

				} // random

				// Now we have our gamma rays from earlier
				// We fill these matrices in a loop, so there'll be double counting
				// which means you cannot simply take a projection of them...
				bool prompt_gamma_energy = false;
				bool random_gamma_energy = false;
				for( unsigned int k = 0; k < promptgammaidx.size(); ++k ){

					gamma_evt1 = read_evts->GetGammaRayEvt( promptgammaidx[k] );
					gamma_Ex_fission->Fill( react->GetEx(), gamma_evt1->GetEnergy() );

					// check the user-defined energy gate
					if( GammaEnergyCut( gamma_evt1 ) )
						prompt_gamma_energy = true;

				} // k - prompt gammas

				// And the randoms
				for( unsigned int k = 0; k < randomgammaidx.size(); ++k ){

					gamma_evt1 = read_evts->GetGammaRayEvt( randomgammaidx[k] );
					gamma_Ex_fission->Fill( react->GetEx(), gamma_evt1->GetEnergy(), -1.0 * react->GetArrayGammaFillRatio() );

					// check the user-defined energy gate
					if( GammaEnergyCut( gamma_evt1 ) )
						random_gamma_energy = true;

				} // k - random gammas

				// Fill the gamma-ray gated Ex spectra, but only once!
				// It is done like this to avoid double counting array events
				if( prompt_gamma_energy ) {

					E_vs_z_fission_gamma->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
					Ex_fission_gamma->Fill( react->GetEx() );

				}

				// And the randoms
				if( random_gamma_energy ) {

					E_vs_z_fission_gamma_random->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
					Ex_fission_gamma_random->Fill( react->GetEx() );

				}

			} // fission mode finished

			// -----------
			// Recoil mode
			promptcheckT = false;
			randomcheckT = false;
			promptcheckE = false;
			randomcheckE = false;
			energycut = false;
			std::vector<unsigned int>().swap( promptgammaidx );
			std::vector<unsigned int>().swap( randomgammaidx );

			// Check if we use the CD or the recoil detector
			unsigned int generic_mult = 0;
			if( react->RecoilType() == 0 )
				generic_mult = read_evts->GetRecoilMultiplicity();
			else if( react->RecoilType() == 1 )
				generic_mult = read_evts->GetCDMultiplicity();

			// Find recoil
			double bg_frac = -1.0;
			for( unsigned int k = 0; k < generic_mult; ++k ){

				// Get event depending on type, first is normal recoil detector
				if( react->RecoilType() == 0 && set->GetNumberOfRecoilSectors() > 0 ) {

					recoil_evt = read_evts->GetRecoilEvt(k);
					generic_evt = recoil_evt;
					energycut = RecoilCut( recoil_evt );
					bg_frac = react->GetArrayRecoilFillRatio();

					// Time differences
					tdiff = generic_evt->GetTime() - array_evt->GetTime();

					if( react->RecoilType() == 0 && set->GetNumberOfRecoilSectors() > 0 )
						recoil_array_td[recoil_evt->GetSector()][array_evt->GetModule()]->Fill( tdiff );

					if( array_evt->GetPHit() ) { // hit bit = true

						recoil_array_tw_hit1->Fill( tdiff, array_evt->GetEnergy() );
						recoil_array_tw_hit1_prof->Fill( array_evt->GetEnergy(), tdiff );
						recoil_array_tw_hit1_row[array_evt->GetModule()][array_evt->GetRow()]->Fill( tdiff, array_evt->GetEnergy() );

					}

					else { // hit bit = false

						recoil_array_tw_hit0->Fill( tdiff, array_evt->GetEnergy() );
						recoil_array_tw_hit0_prof->Fill( array_evt->GetEnergy(), tdiff );
						recoil_array_tw_hit0_row[array_evt->GetModule()][array_evt->GetRow()]->Fill( tdiff, array_evt->GetEnergy() );

					}

				} // normal recoil detector

				// Then the CD as a recoil detector
				else if( react->RecoilType() == 1 && set->GetNumberOfCDLayers() > 0 ) {

					cd_evt1 = read_evts->GetCDEvt(k);
					generic_evt = cd_evt1;
					promptcheckT = PromptCoincidence( cd_evt1, array_evt );
					randomcheckT = RandomCoincidence( cd_evt1, array_evt );
					energycut = RecoilCut( cd_evt1 );
					bg_frac = react->GetArrayFissionFillRatio();

				}

				// or else don't bother
				else break;

				// Check for prompt events with recoils
				if( promptcheckT )
					if( energycut )
						promptcheckE = true;

				// Check for random events with recoils
				if( randomcheckT )
					if( energycut )
						randomcheckE = true;

			} // k

			// Fill prompt hists
			if( promptcheckT == true ){

				// Recoils in coincidence with an array event
				if( react->RecoilType() == 0 && set->GetNumberOfRecoilSectors() > 0 )
					recoil_EdE_array[recoil_evt->GetSector()]->Fill( recoil_evt->GetEnergyRest( set->GetRecoilEnergyRestStart(), set->GetRecoilEnergyRestStop() ), recoil_evt->GetEnergyLoss( set->GetRecoilEnergyLossStart(), set->GetRecoilEnergyLossStop() ) );

				// Array histograms
				E_vs_z_recoilT->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
				E_vs_z_recoilT_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
				Theta_recoilT->Fill( react->GetThetaCM() * TMath::RadToDeg() );
				Theta_recoilT_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
				Ex_recoilT->Fill( react->GetEx() );
				Ex_recoilT_mod[array_evt->GetModule()]->Fill( react->GetEx() );
				E_vs_theta_recoilT->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
				E_vs_theta_recoilT_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
				Ex_vs_theta_recoilT->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
				Ex_vs_theta_recoilT_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
				Ex_vs_z_recoilT->Fill( react->GetZmeasured(), react->GetEx() );
				Ex_vs_z_recoilT_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), react->GetEx() );

				// Look for gamma-rays in coincidence with the array and recoil
				if( react->GammaRayHistsEnabled() ) {

					// Loop over gamma-ray events
					for( unsigned int k = 0; k < read_evts->GetGammaRayMultiplicity(); ++k ) {

						// Get event
						gamma_evt1 = read_evts->GetGammaRayEvt(k);

						// Ex versus Egamma (no Doppler correction yet applied)
						if( PromptCoincidence( gamma_evt1, array_evt )  )
							gamma_Ex_recoilT->Fill( gamma_evt1->GetEnergy(), react->GetEx() );

						else if( RandomCoincidence( gamma_evt1, array_evt ) )
							gamma_Ex_recoilT->Fill( gamma_evt1->GetEnergy(), react->GetEx(), -1.0 * react->GetArrayGammaFillRatio() );

					} // k

				} // gamma-ray hists enabled

				// Check the E vs z cuts from the user
				for( unsigned int l = 0; l < react->GetNumberOfEvsZCuts(); ++l ){

					// Is inside the cut
					if( react->GetEvsZCut(l)->IsInside( react->GetZmeasured(), array_evt->GetEnergy() ) ){

						E_vs_z_recoilT_cut[l]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
						Theta_recoilT_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
						Ex_recoilT_cut[l]->Fill( react->GetEx() );
						E_vs_theta_recoilT_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
						Ex_vs_theta_recoilT_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
						Ex_vs_z_recoilT_cut[l]->Fill( react->GetZmeasured(), react->GetEx() );

					} // inside cut

				} // loop over cuts

				// Fill energy gate hists
				if( promptcheckE == true ) {

					E_vs_z_recoil->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
					E_vs_z_recoil_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
					Theta_recoil->Fill( react->GetThetaCM() * TMath::RadToDeg() );
					Theta_recoil_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
					Ex_recoil->Fill( react->GetEx() );
					Ex_recoil_mod[array_evt->GetModule()]->Fill( react->GetEx() );
					E_vs_theta_recoil->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
					E_vs_theta_recoil_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
					Ex_vs_theta_recoil->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
					Ex_vs_theta_recoil_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
					Ex_vs_z_recoil->Fill( react->GetZmeasured(), react->GetEx() );
					Ex_vs_z_recoil_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), react->GetEx() );

					// Look for gamma-rays in coincidence with the array and recoil
					if( react->GammaRayHistsEnabled() ) {

						// Loop over gamma-ray events
						for( unsigned int k = 0; k < read_evts->GetGammaRayMultiplicity(); ++k ) {

							// Get event
							gamma_evt1 = read_evts->GetGammaRayEvt(k);

							// Ex versus Egamma (no Doppler correction yet applied)
							if( PromptCoincidence( gamma_evt1, array_evt )  )
								gamma_Ex_recoil->Fill( gamma_evt1->GetEnergy(), react->GetEx() );

							else if( RandomCoincidence( gamma_evt1, array_evt ) )
								gamma_Ex_recoil->Fill( gamma_evt1->GetEnergy(), react->GetEx(), -1.0 * react->GetArrayGammaFillRatio() );

						} // k

					} // gamma-ray hists enabled

					// Check the E vs z cuts from the user
					for( unsigned int l = 0; l < react->GetNumberOfEvsZCuts(); ++l ){

						// Is inside the cut
						if( react->GetEvsZCut(l)->IsInside( react->GetZmeasured(), array_evt->GetEnergy() ) ){

							E_vs_z_recoil_cut[l]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
							Theta_recoil_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
							Ex_recoil_cut[l]->Fill( react->GetEx() );
							E_vs_theta_recoil_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
							Ex_vs_theta_recoil_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
							Ex_vs_z_recoil_cut[l]->Fill( react->GetZmeasured(), react->GetEx() );

						} // inside cut

					} // loop over cuts

				} // energy cuts

			} // prompt

			// Fill random hists, even if we filled it already as a prompt hit...
			// Justification is that larger random windows can be used whilst keeping
			// the event ratio for prompt and random consistent. Using an else here would
			// mean that the scaling of the randoms would not increase linearly with window width.
			// Some events will be added and subtracted if there is a prompt AND random coinicidence
			// with two different recoil, so the error bar goes up, but if the random window
			// is larger, the weighting of the random will be <1 and a small contribution.
			if( randomcheckT == true ){

				// Array histograms
				E_vs_z_recoilT_random->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
				E_vs_z_recoilT_random_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
				Theta_recoilT_random->Fill( react->GetThetaCM() * TMath::RadToDeg() );
				Theta_recoilT_random_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
				Ex_recoilT_random->Fill( react->GetEx() );
				Ex_recoilT_random_mod[array_evt->GetModule()]->Fill( react->GetEx() );
				E_vs_theta_recoilT_random->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
				E_vs_theta_recoilT_random_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
				Ex_vs_theta_recoilT_random->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
				Ex_vs_theta_recoilT_random_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
				Ex_vs_z_recoilT_random->Fill( react->GetZmeasured(), react->GetEx() );
				Ex_vs_z_recoilT_random_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), react->GetEx() );

				// Look for gamma-rays in coincidence with the array and recoil
				if( react->GammaRayHistsEnabled() ) {

					// Loop over gamma-ray events
					for( unsigned int k = 0; k < read_evts->GetGammaRayMultiplicity(); ++k ) {

						// Get event
						gamma_evt1 = read_evts->GetGammaRayEvt(k);

						// Ex versus Egamma (no Doppler correction yet applied)
						if( PromptCoincidence( gamma_evt1, array_evt )  )
							gamma_Ex_recoilT->Fill( gamma_evt1->GetEnergy(), react->GetEx(), -1.0 * bg_frac );

						else if( RandomCoincidence( gamma_evt1, array_evt ) )
							gamma_Ex_recoilT->Fill( gamma_evt1->GetEnergy(), react->GetEx(), bg_frac * react->GetArrayGammaFillRatio() );

					} // k

				} // gamma-ray hists enabled

				// Check the E vs z cuts from the user
				for( unsigned int l = 0; l < react->GetNumberOfEvsZCuts(); ++l ){

					// Is inside the cut
					if( react->GetEvsZCut(l)->IsInside( react->GetZmeasured(), array_evt->GetEnergy() ) ){

						E_vs_z_recoilT_random_cut[l]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
						Theta_recoilT_random_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
						Ex_recoilT_random_cut[l]->Fill( react->GetEx() );
						E_vs_theta_recoilT_random_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
						Ex_vs_theta_recoilT_random_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
						Ex_vs_z_recoilT_random_cut[l]->Fill( react->GetZmeasured(), react->GetEx() );

					} // inside cut

				} // loop over cuts

				// Fill energy gate hists
				if( randomcheckE == true ) {

					E_vs_z_recoil_random->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
					E_vs_z_recoil_random_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
					Theta_recoil_random->Fill( react->GetThetaCM() * TMath::RadToDeg() );
					Theta_recoil_random_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
					Ex_recoil_random->Fill( react->GetEx() );
					Ex_recoil_random_mod[array_evt->GetModule()]->Fill( react->GetEx() );
					E_vs_theta_recoil_random->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
					E_vs_theta_recoil_random_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
					Ex_vs_theta_recoil_random->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
					Ex_vs_theta_recoil_random_mod[array_evt->GetModule()]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
					Ex_vs_z_recoil_random->Fill( react->GetZmeasured(), react->GetEx() );
					Ex_vs_z_recoil_random_mod[array_evt->GetModule()]->Fill( react->GetZmeasured(), react->GetEx() );

					// Look for gamma-rays in coincidence with the array and recoil
					if( react->GammaRayHistsEnabled() ) {

						// Loop over gamma-ray events
						for( unsigned int k = 0; k < read_evts->GetGammaRayMultiplicity(); ++k ) {

							// Get event
							gamma_evt1 = read_evts->GetGammaRayEvt(k);

							// Ex versus Egamma (no Doppler correction yet applied)
							if( PromptCoincidence( gamma_evt1, array_evt )  )
								gamma_Ex_recoil->Fill( gamma_evt1->GetEnergy(), react->GetEx(), -1.0 * react->GetArrayRecoilFillRatio() );

							else if( RandomCoincidence( gamma_evt1, array_evt ) )
								gamma_Ex_recoil->Fill( gamma_evt1->GetEnergy(), react->GetEx(), react->GetArrayRecoilFillRatio() * react->GetArrayGammaFillRatio() );

						} // k

					} // gamma-ray hists enabled

					// Check the E vs z cuts from the user
					for( unsigned int l = 0; l < react->GetNumberOfEvsZCuts(); ++l ){

						// Is inside the cut
						if( react->GetEvsZCut(l)->IsInside( react->GetZmeasured(), array_evt->GetEnergy() ) ){

							E_vs_z_recoil_random_cut[l]->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
							Theta_recoil_random_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg() );
							Ex_recoil_random_cut[l]->Fill( react->GetEx() );
							Ex_vs_theta_recoil_random_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
							Ex_vs_theta_recoil_random_cut[l]->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
							Ex_vs_z_recoil_random_cut[l]->Fill( react->GetZmeasured(), react->GetEx() );

						} // inside cut

					} // loop over cuts

				} // energy cuts

			} // random


			// -----------
			// Gamma mode
			if( react->GammaRayHistsEnabled() ){

				// Reset the booleans
				promptcheckT = false;
				randomcheckT = false;
				promptcheckE = false;
				randomcheckE = false;
				energycut = false;

				// Find gamma-rays in coincidence
				for( unsigned int k = 0; k < read_evts->GetGammaRayMultiplicity(); ++k ){

					// Get event
					gamma_evt1 = read_evts->GetGammaRayEvt(k);

					// Check for prompt events with recoils
					promptcheckT = PromptCoincidence( gamma_evt1, array_evt );
					if( promptcheckT )
						if( GammaEnergyCut( gamma_evt1 ) )
							promptcheckE = true;

					// Check for random events with recoils
					randomcheckT = RandomCoincidence( gamma_evt1, array_evt );
					if( randomcheckT )
						if( GammaEnergyCut( gamma_evt1 ) )
							randomcheckE = true;

				} // k

				// Fill prompt hists
				if( promptcheckT == true ){

					// Array histograms
					E_vs_z_gammaT->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
					Theta_gammaT->Fill( react->GetThetaCM() * TMath::RadToDeg() );
					Ex_gammaT->Fill( react->GetEx() );
					E_vs_theta_gammaT->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
					Ex_vs_theta_gammaT->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
					Ex_vs_z_gammaT->Fill( react->GetZmeasured(), react->GetEx() );

					// Fill energy gate hists
					if( promptcheckE == true ) {

						E_vs_z_gamma->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
						Theta_gamma->Fill( react->GetThetaCM() * TMath::RadToDeg() );
						Ex_gamma->Fill( react->GetEx() );
						E_vs_theta_gamma->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
						Ex_vs_theta_gamma->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
						Ex_vs_z_gamma->Fill( react->GetZmeasured(), react->GetEx() );

					} // energy cut

				} // prompt

				// Fill random hists, even if we filled it already as a prompt hit...
				if( randomcheckT == true ){

					// Array histograms
					E_vs_z_gammaT_random->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
					Theta_gammaT_random->Fill( react->GetThetaCM() * TMath::RadToDeg() );
					Ex_gammaT_random->Fill( react->GetEx() );
					E_vs_theta_gammaT_random->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
					Ex_vs_theta_gammaT_random->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
					Ex_vs_z_gammaT_random->Fill( react->GetZmeasured(), react->GetEx() );

					// Fill energy gate hists
					if( randomcheckE == true ) {

						E_vs_z_gamma_random->Fill( react->GetZmeasured(), array_evt->GetEnergy() );
						Theta_gamma_random->Fill( react->GetThetaCM() * TMath::RadToDeg() );
						Ex_gamma_random->Fill( react->GetEx() );
						E_vs_theta_gamma_random->Fill( react->GetThetaCM() * TMath::RadToDeg(), array_evt->GetEnergy() );
						Ex_vs_theta_gamma_random->Fill( react->GetThetaCM() * TMath::RadToDeg(), react->GetEx() );
						Ex_vs_z_gamma_random->Fill( react->GetZmeasured(), react->GetEx() );

					} // energy cut

				} // random

			} // gamma mode

		} // array

		// Loop over ELUM events
		for( unsigned int j = 0; j < read_evts->GetElumMultiplicity(); ++j ){

			// Get ELUM event
			elum_evt = read_evts->GetElumEvt(j);

			// EBIS time
			ebis_td_elum->Fill( elum_evt->GetTime() - read_evts->GetEBIS() );

			// Singles
			elum->Fill( elum_evt->GetEnergy() );
			elum_sec[elum_evt->GetSector()]->Fill( elum_evt->GetEnergy() );

			// Check for events in the EBIS on-beam window
			if( OnBeam( elum_evt ) ){

				elum_ebis->Fill( elum_evt->GetEnergy() );
				elum_ebis_sec[elum_evt->GetSector()]->Fill( elum_evt->GetEnergy() );
				elum_ebis_on->Fill( elum_evt->GetEnergy() );
				elum_ebis_on_sec[elum_evt->GetSector()]->Fill( elum_evt->GetEnergy() );
				elum_vs_T1->Fill( elum_evt->GetTime() - read_evts->GetT1(), elum_evt->GetEnergy() );

			} // ebis

			else {

				elum_ebis->Fill( elum_evt->GetEnergy(), -1.0 * react->GetEBISFillRatio() );
				elum_ebis_sec[elum_evt->GetSector()]->Fill( elum_evt->GetEnergy(), -1.0 * react->GetEBISFillRatio() );
				elum_ebis_off->Fill( elum_evt->GetEnergy() );
				elum_ebis_off_sec[elum_evt->GetSector()]->Fill( elum_evt->GetEnergy() );

			}

			// Coincidence with recoil/fission events
			bool promptcheck = false;
			bool randomcheck = false;

			// Only have ELUM when in recoil mode, i.e. not fission mode
			if( !react->IsFission() ) {

				for( unsigned int k = 0; k < read_evts->GetRecoilMultiplicity(); ++k ){

					// Get recoil event
					recoil_evt = read_evts->GetRecoilEvt(k);

					// Time differences
					tdiff = recoil_evt->GetTime() - elum_evt->GetTime();
					recoil_elum_td[recoil_evt->GetSector()][elum_evt->GetSector()]->Fill( tdiff );

					// Check for prompt events with recoils
					if( PromptCoincidence( recoil_evt, elum_evt ) )
						promptcheck = true;

					// Check for random events with recoils
					if( RandomCoincidence( recoil_evt, elum_evt ) )
						randomcheck = true;

				} // recoils

				// Plot the prompt events
				if( promptcheck == true ){

					elum_recoilT->Fill( elum_evt->GetEnergy() );
					elum_recoilT_sec[elum_evt->GetSector()]->Fill( elum_evt->GetEnergy() );

					// Add an energy gate
					if( RecoilCut( recoil_evt ) ) {

						elum_recoil->Fill( elum_evt->GetEnergy() );
						elum_recoil_sec[elum_evt->GetSector()]->Fill( elum_evt->GetEnergy() );

					} // energy cuts

				} // prompt

				// Plot the random events, but only if we didn't already use it as a prompt
				else if( randomcheck == true ){

					elum_recoilT_random->Fill( elum_evt->GetEnergy() );
					elum_recoilT_random_sec[elum_evt->GetSector()]->Fill( elum_evt->GetEnergy() );

					// Add an energy gate
					if( RecoilCut( recoil_evt ) ) {

						elum_recoil_random->Fill( elum_evt->GetEnergy() );
						elum_recoil_random_sec[elum_evt->GetSector()]->Fill( elum_evt->GetEnergy() );

					} // energy cuts

				} // random

			} // end of recoil mode

		} // ELUM

		// Fission mode
		if( react->IsFission() && set->GetNumberOfCDLayers() > 0 ) {

			// Loop over CD events
			for( unsigned int j = 0; j < read_evts->GetCDMultiplicity(); ++j ){

				// Get CD event
				cd_evt1 = read_evts->GetCDEvt(j);

				// EBIS, T1, SC time
				ebis_td_fission->Fill( cd_evt1->GetTime() - read_evts->GetEBIS() );
				t1_td_fission->Fill( cd_evt1->GetTime() - read_evts->GetT1() );
				sc_td_fission->Fill( cd_evt1->GetTime() - read_evts->GetSC() );

				// Hit map
				fission_xy_map->Fill( cd_evt1->GetY(true), cd_evt1->GetX(true) );

				// Energy versus ring number
				fission_dE_vs_ring->Fill( cd_evt1->GetRing(),
										 cd_evt1->GetEnergyLoss( set->GetRecoilEnergyLossStart(), set->GetRecoilEnergyLossStop() ) );
				fission_Etot_vs_ring->Fill( cd_evt1->GetRing(), cd_evt1->GetEnergyTotal() );

				// Energy EdE plot, unconditioned
				fission_EdE->Fill( cd_evt1->GetEnergyRest( set->GetRecoilEnergyRestStart(), set->GetRecoilEnergyRestStop() ),
								  cd_evt1->GetEnergyLoss( set->GetRecoilEnergyLossStart(), set->GetRecoilEnergyLossStop() ) );

				// Energy dE versus T1 time
				fission_dE_vs_T1->Fill( cd_evt1->GetTime() - read_evts->GetT1(),
									   cd_evt1->GetEnergyLoss( set->GetRecoilEnergyLossStart(), set->GetRecoilEnergyLossStop() ) );

				// Bragg curve
				for( unsigned int k = 0; k < cd_evt1->GetEnergies().size(); ++k )
					fission_bragg->Fill( cd_evt1->GetID(k), cd_evt1->GetEnergy( cd_evt1->GetID(k) ) );

				// Energy EdE plot and hit map, after cut on heavy fragment
				if( FissionCutHeavy( cd_evt1 ) ) {

					fission_xy_map_cutH->Fill( cd_evt1->GetY(true), cd_evt1->GetX(true) );
					fission_EdE_cutH->Fill( cd_evt1->GetEnergyRest( set->GetRecoilEnergyRestStart(), set->GetRecoilEnergyRestStop() ),
										   cd_evt1->GetEnergyLoss( set->GetRecoilEnergyLossStart(), set->GetRecoilEnergyLossStop() ) );

				}

				// Energy EdE plot and hit map, after cut on light fragment
				if( FissionCutLight( cd_evt1 ) ) {

					fission_xy_map_cutL->Fill( cd_evt1->GetY(true), cd_evt1->GetX(true) );
					fission_EdE_cutL->Fill( cd_evt1->GetEnergyRest( set->GetRecoilEnergyRestStart(), set->GetRecoilEnergyRestStop() ),
											cd_evt1->GetEnergyLoss( set->GetRecoilEnergyLossStart(), set->GetRecoilEnergyLossStop() ) );

				}

				fission_dE_eloss->Fill( cd_evt1->GetEnergyLoss( set->GetRecoilEnergyLossStart(), set->GetRecoilEnergyLossStop() ) );
				fission_E_eloss->Fill( cd_evt1->GetEnergyRest( set->GetRecoilEnergyRestStart(), set->GetRecoilEnergyRestStop() ) );

				// Loop over coincident CD events
				for( unsigned int k = 0; k < read_evts->GetCDMultiplicity(); ++k ){

					// Skip self-coincidences
					if( j == k ) continue;

					// Get CD event
					cd_evt2 = read_evts->GetCDEvt(k);

					// Time difference
					double ff_td = cd_evt1->GetTime() - cd_evt2->GetTime();
					fission_fission_td->Fill( ff_td );
					fission_fission_td_sec->Fill( cd_evt1->GetSector(), ff_td );

					// Energy matrix
					if( PromptCoincidence( cd_evt1, cd_evt2 ) ) {

						fission_fission_dEdE->Fill( cd_evt1->GetEnergyLoss( set->GetCDEnergyLossStart(), set->GetCDEnergyLossStop() ),
												   cd_evt2->GetEnergyLoss( set->GetCDEnergyLossStart(), set->GetCDEnergyLossStop() ) );
						fission_fission_secsec->Fill( cd_evt1->GetSector(), cd_evt2->GetSector() );
						fission_fission_ringring->Fill( cd_evt1->GetRing(), cd_evt2->GetRing() );

					} // prompt CD coincidences

					else if( RandomCoincidence( cd_evt1, cd_evt2 ) ) {

						fission_fission_dEdE->Fill( cd_evt1->GetEnergyLoss( set->GetCDEnergyLossStart(), set->GetCDEnergyLossStop() ),
												   cd_evt2->GetEnergyLoss( set->GetCDEnergyLossStart(), set->GetCDEnergyLossStop() ),
												   -1.0*react->GetFissionFissionFillRatio() );
						fission_fission_secsec->Fill( cd_evt1->GetSector(), cd_evt2->GetSector(), -1.0*react->GetFissionFissionFillRatio() );
						fission_fission_ringring->Fill( cd_evt1->GetRing(), cd_evt2->GetRing(), -1.0*react->GetFissionFissionFillRatio() );

					} // random CD coincidences

				} // cd events 2

			} // cd events 1

		} // end of fission mode

		// Recoil mode
		else {

			// Loop over recoil events
			for( unsigned int j = 0; j < read_evts->GetRecoilMultiplicity(); ++j ){

				// Get recoil event
				recoil_evt = read_evts->GetRecoilEvt(j);

				// EBIS, T1, SC time
				ebis_td_recoil->Fill( recoil_evt->GetTime() - read_evts->GetEBIS() );
				t1_td_recoil->Fill( recoil_evt->GetTime() - read_evts->GetT1() );
				sc_td_recoil->Fill( recoil_evt->GetTime() - read_evts->GetSC() );

				// Energy EdE plot, unconditioned
				recoil_EdE[recoil_evt->GetSector()]->Fill( recoil_evt->GetEnergyRest( set->GetRecoilEnergyRestStart(), set->GetRecoilEnergyRestStop() ),
														  recoil_evt->GetEnergyLoss( set->GetRecoilEnergyLossStart(), set->GetRecoilEnergyLossStop() ) );

				// Energy dE versus T1 time
				recoil_dE_vs_T1[recoil_evt->GetSector()]->Fill( recoil_evt->GetTime() - read_evts->GetT1(),
															   recoil_evt->GetEnergyLoss( set->GetRecoilEnergyLossStart(), set->GetRecoilEnergyLossStop() ) );

				// Bragg curve
				for( unsigned int k = 0; k < recoil_evt->GetEnergies().size(); ++k )
					recoil_bragg[recoil_evt->GetSector()]->Fill( recoil_evt->GetID(k), recoil_evt->GetEnergy(k) );

				// Energy EdE plot, after cut
				if( RecoilCut( recoil_evt ) )
					recoil_EdE_cut[recoil_evt->GetSector()]->Fill( recoil_evt->GetEnergyRest( set->GetRecoilEnergyRestStart(), set->GetRecoilEnergyRestStop() ),
																  recoil_evt->GetEnergyLoss( set->GetRecoilEnergyLossStart(), set->GetRecoilEnergyLossStop() ) );

				recoil_dE_eloss[recoil_evt->GetSector()]->Fill( recoil_evt->GetEnergyLoss( set->GetRecoilEnergyLossStart(), set->GetRecoilEnergyLossStop() ) );
				recoil_E_eloss[recoil_evt->GetSector()]->Fill( recoil_evt->GetEnergyRest( set->GetRecoilEnergyRestStart(), set->GetRecoilEnergyRestStop() ) );

			} // recoils

		} // end of recoil mode

		// Loop over LUME events
		for( unsigned int j = 0; j < read_evts->GetLumeMultiplicity(); ++j ){

			// Get LUME event
			lume_evt = read_evts->GetLumeEvt(j);

			int det_id = lume_evt->GetID();
			if( det_id >= set->GetNumberOfLUMEDetectors() ){
				std::cerr << "Bad LUME detector ID " << det_id << ". Only " << set->GetNumberOfLUMEDetectors();
				std::cerr << " detectors are set. Ignoring this event for histogramming." << std::endl;
				continue;
			}

			// EBIS time
			ebis_td_lume->Fill( lume_evt->GetTime() - read_evts->GetEBIS() );

			// Singles
			lume->Fill( lume_evt->GetBE() );
			lume_det[det_id]->Fill( lume_evt->GetBE() );

			// E versus x
			lume_E_vs_x->Fill( lume_evt->GetX(),lume_evt->GetBE(),1. );
			lume_E_vs_x_wide->Fill( lume_evt->GetX(),lume_evt->GetBE(),1. );
			lume_E_vs_x_det[det_id]->Fill( lume_evt->GetX(),lume_evt->GetBE(),1 );

			// Check for events in the EBIS on-beam window
			if( OnBeam( lume_evt ) ){

				lume_vs_T1->Fill( lume_evt->GetTime() - read_evts->GetT1(), lume_evt->GetBE() );
				lume_ebis->Fill( lume_evt->GetBE() );
				lume_ebis_on->Fill( lume_evt->GetBE() );
				lume_E_vs_x_ebis->Fill( lume_evt->GetX(),lume_evt->GetBE(),1. );
				lume_E_vs_x_ebis_on->Fill( lume_evt->GetX(),lume_evt->GetBE(),1. );

				lume_ebis_on_det[det_id]->Fill( lume_evt->GetBE() );
				lume_E_vs_x_ebis_det[det_id]->Fill( lume_evt->GetX(),lume_evt->GetBE(),1. );
				lume_E_vs_x_ebis_on_det[det_id]->Fill( lume_evt->GetX(),lume_evt->GetBE(),1. );

			}

			else if( OffBeam( lume_evt ) ){

				lume_ebis->Fill( lume_evt->GetBE(), -1.* react->GetEBISFillRatio() );
				lume_ebis_off->Fill( lume_evt->GetBE() );
				lume_ebis_off_det[det_id]->Fill( lume_evt->GetBE() );

				lume_E_vs_x_ebis->Fill( lume_evt->GetX(),lume_evt->GetBE(),-1.* react->GetEBISFillRatio() );
				lume_E_vs_x_ebis_off->Fill( lume_evt->GetX(),lume_evt->GetBE(),1. );
				lume_E_vs_x_ebis_det[det_id]->Fill( lume_evt->GetX(),lume_evt->GetBE(),-1.* react->GetEBISFillRatio() );
				lume_E_vs_x_ebis_off_det[det_id]->Fill( lume_evt->GetX(),lume_evt->GetBE(),1. );

			} // ebis

			// Loop over recoil events
			bool promptcheckE = false;
			bool randomcheckE = false;
			bool promptcheckT = false;
			bool randomcheckT = false;
			bool energycut = false;

			// Check if we use the CD or the recoil detector
			unsigned int generic_mult = 0;
			if( react->RecoilType() == 0 )
				generic_mult = read_evts->GetRecoilMultiplicity();
			else if( react->RecoilType() == 1 )
				generic_mult = read_evts->GetCDMultiplicity();

			// Loop over CD/Recoil events to check for random and prompt coincidences
			for( unsigned int k = 0; k < generic_mult; ++k ){

				// Get event
				if( react->RecoilType() == 0 && set->GetNumberOfRecoilSectors() > 0 ) {

					recoil_evt = read_evts->GetRecoilEvt(k);
					generic_evt = recoil_evt;
					energycut = RecoilCut( recoil_evt );
					promptcheckT = PromptCoincidence( recoil_evt, lume_evt );
					randomcheckT = RandomCoincidence( recoil_evt, lume_evt );

				}

				else if( react->RecoilType() == 1 && set->GetNumberOfCDLayers() > 0 ) {

					cd_evt1 = read_evts->GetCDEvt(k);
					generic_evt = cd_evt1;
					energycut = RecoilCut( cd_evt1 );
					promptcheckT = PromptCoincidence( cd_evt1, lume_evt );
					randomcheckT = RandomCoincidence( cd_evt1, lume_evt );

				}

				else break;

				// Time differences
				tdiff = generic_evt->GetTime() - lume_evt->GetTime();
				recoil_lume_td[det_id]->Fill( tdiff );

				// Check for prompt events with coincident recoils
				if( promptcheckT )
					if( energycut )
						promptcheckE = true;

				// Check for random events with coincident recoils
				if( randomcheckT )
					if( energycut )
						randomcheckE = true;

			} // generic recoil events

			// Fill prompt hists
			if( promptcheckT == true ){

				lume_recoilT->Fill( lume_evt->GetBE() );
				lume_recoilT_det[det_id]->Fill( lume_evt->GetBE() );

				// Fill energy gate hists
				if( promptcheckE == true ) {

					lume_recoil->Fill( lume_evt->GetBE() );
					lume_recoil_det[det_id]->Fill( lume_evt->GetBE() );

				} // energy cuts

			} // prompt

			// Fill random hists
			else if( randomcheckT == true ){

				lume_recoilT_random->Fill( lume_evt->GetBE() );
				lume_recoilT_random_det[det_id]->Fill( lume_evt->GetBE() );

				// Fill energy gate hists
				if( randomcheckE == true ) {

					lume_recoil_random->Fill( lume_evt->GetBE() );
					lume_recoil_random_det[det_id]->Fill( lume_evt->GetBE() );

				} // energy cuts

			} // random

		} // j - LUMEs


		// Look for gamma-rays
		if( react->GammaRayHistsEnabled() ) {

			// Loop over gamma-ray events
			for( unsigned int j = 0; j < read_evts->GetGammaRayMultiplicity(); ++j ) {

				// Get event
				gamma_evt1 = read_evts->GetGammaRayEvt(j);

				// Loop over gamma-ray events
				for( unsigned int k = j+1; k < read_evts->GetGammaRayMultiplicity(); ++k ) {

					// Get event
					gamma_evt2 = read_evts->GetGammaRayEvt(k);

					// Time differences
					gamma_gamma_td->Fill( gamma_evt1->GetTime() - gamma_evt2->GetTime() );
					gamma_gamma_td->Fill( gamma_evt2->GetTime() - gamma_evt1->GetTime() );

				} // k

				// Check EBIS on or off
				if( OnBeam( gamma_evt1 ) ) {

					// Singles
					gamma_ebis->Fill( gamma_evt1->GetEnergy() );
					gamma_ebis_on->Fill( gamma_evt1->GetEnergy() );

					// Loop over gamma-ray events
					for( unsigned int k = 0; k < read_evts->GetGammaRayMultiplicity(); ++k ) {

						// Skip self coincindence
						if( k == j ) continue;

						// Get event
						gamma_evt2 = read_evts->GetGammaRayEvt(k);

						// Ex versus Egamma (no Doppler correction yet applied)
						if( PromptCoincidence( gamma_evt1, gamma_evt2 ) )
							gamma_gamma_ebis->Fill( gamma_evt1->GetEnergy(), gamma_evt2->GetEnergy() );

						else if( RandomCoincidence( gamma_evt1, gamma_evt2 ) )
							gamma_gamma_ebis->Fill( gamma_evt1->GetEnergy(), gamma_evt2->GetEnergy(), -1.0 * react->GetGammaGammaFillRatio() );

					} // k - gammas

				} // EBIS on

				// beam off
				else if( OffBeam( gamma_evt1 ) ) {

					// Singles
					gamma_ebis->Fill( gamma_evt1->GetEnergy(), -1.0 * react->GetEBISFillRatio() );
					gamma_ebis_off->Fill( gamma_evt1->GetEnergy() );

					// Loop over gamma-ray events
					for( unsigned int k = 0; k < read_evts->GetGammaRayMultiplicity(); ++k ) {

						// Skip self coincindence
						if( k == j ) continue;

						// Get event
						gamma_evt2 = read_evts->GetGammaRayEvt(k);

						// Ex versus Egamma (no Doppler correction yet applied)
						if( PromptCoincidence( gamma_evt1, gamma_evt2 ) )
							gamma_gamma_ebis->Fill( gamma_evt1->GetEnergy(), gamma_evt2->GetEnergy(), -1.0 * react->GetEBISFillRatio() );

						else if( RandomCoincidence( gamma_evt1, gamma_evt2 ) )
							gamma_gamma_ebis->Fill( gamma_evt1->GetEnergy(), gamma_evt2->GetEnergy(), react->GetEBISFillRatio() * react->GetGammaGammaFillRatio() );

					} // k - gammas

				} // EBIS off

				// Loop over recoil events
				bool promptcheckE = false;
				bool randomcheckE = false;
				bool promptcheckT = false;
				bool randomcheckT = false;
				bool energycut = false;
				double bg_frac = -1.0;

				// Check if we use the CD or the recoil detector
				unsigned int generic_mult = 0;
				if( react->RecoilType() == 0 ) {

					generic_mult = read_evts->GetRecoilMultiplicity();
					bg_frac = react->GetRecoilGammaFillRatio();

				}
				else if( react->RecoilType() == 1 ) {

					generic_mult = read_evts->GetCDMultiplicity();
					bg_frac = react->GetFissionGammaFillRatio();

				}

				// Loop over CD/Recoil events to check for random and prompt coincidences
				for( unsigned int k = 0; k < generic_mult; ++k ){

					// Get event
					if( react->RecoilType() == 0 && set->GetNumberOfRecoilSectors() > 0 ) {

						recoil_evt = read_evts->GetRecoilEvt(k);
						generic_evt = recoil_evt;
						promptcheckT = PromptCoincidence( gamma_evt1, recoil_evt );
						randomcheckT = RandomCoincidence( gamma_evt1, recoil_evt );
						energycut = RecoilCut( recoil_evt );
						gamma_recoil_td->Fill( gamma_evt1->GetTime() - recoil_evt->GetTime() );

					}

					else if( react->RecoilType() == 1 && set->GetNumberOfCDLayers() > 0 ) {

						cd_evt1 = read_evts->GetCDEvt(k);
						generic_evt = cd_evt1;
						promptcheckT = PromptCoincidence( gamma_evt1, cd_evt1 );
						randomcheckT = RandomCoincidence( gamma_evt1, cd_evt1 );
						energycut = RecoilCut( cd_evt1 );
						gamma_fission_td->Fill( gamma_evt1->GetTime() - cd_evt1->GetTime() );

					}

					else break;

					// Check for prompt events with coincident recoils
					if( promptcheckT )
						if( energycut )
							promptcheckE = true;

					// Check for random events with coincident recoils
					if( randomcheckT )
						if( energycut )
							randomcheckE = true;

				} // generic recoil events

				// Fill prompt hists
				if( promptcheckT == true ){

					gamma_recoilT->Fill( gamma_evt1->GetEnergy() );

					// Fill energy gate hists
					if( promptcheckE == true ) {

						gamma_recoil->Fill( gamma_evt1->GetEnergy() );

					} // energy cuts

				} // prompt

				// Fill random hists
				else if( randomcheckT == true ){

					gamma_recoilT->Fill( gamma_evt1->GetEnergy(), -1.0 * bg_frac );

					// Fill energy gate hists
					if( randomcheckE == true ) {

						gamma_recoil->Fill( gamma_evt1->GetEnergy(), -1.0 * bg_frac );

					} // energy cuts

				} // random

				// Loop over second gamma-ray events
				for( unsigned int k = 0; k < read_evts->GetGammaRayMultiplicity(); ++k ) {

					// Skip self coincidence
					if( k == j ) continue;

					// Get event
					gamma_evt2 = read_evts->GetGammaRayEvt(k);

					// Fill prompt hists
					if( promptcheckT && promptcheckE ){

						// Egamma matrix (no Doppler correction yet applied)
						if( PromptCoincidence( gamma_evt1, gamma_evt2 ) )
							gamma_gamma_recoil->Fill( gamma_evt1->GetEnergy(), gamma_evt2->GetEnergy() );

						else if( RandomCoincidence( gamma_evt1, gamma_evt2 ) )
							gamma_gamma_recoil->Fill( gamma_evt1->GetEnergy(), gamma_evt2->GetEnergy(), -1.0 * react->GetGammaGammaFillRatio() );

					} // prompt

					// Fill random hists
					else if( randomcheckT && randomcheckE ){

						// Egamma matrix (no Doppler correction yet applied)
						if( PromptCoincidence( gamma_evt1, gamma_evt2 ) )
							gamma_gamma_recoil->Fill( gamma_evt1->GetEnergy(), gamma_evt2->GetEnergy(), -1.0 * bg_frac );

						else if( RandomCoincidence( gamma_evt1, gamma_evt2 ) )
							gamma_gamma_recoil->Fill( gamma_evt1->GetEnergy(), gamma_evt2->GetEnergy(), bg_frac * react->GetGammaGammaFillRatio() );

					} // random

				} // k

			} // j - gammas

		} // gamma hists enabled


		// Progress bar
		bool update_progress = false;
		if( n_entries < 200 )
			update_progress = true;
		else if( i % (n_entries/100) == 0 || i+1 == n_entries )
			update_progress = true;

		if( update_progress ) {

			// Percent complete
			float percent = (float)(i+1)*100.0/(float)n_entries;

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

	} // all events

	return n_entries;

}

void ISSHistogrammer::SetInputTree( TTree *user_tree ){

	// Find the tree and set branch addresses
	input_tree = (TChain*)user_tree;
	input_tree->SetBranchAddress( "ISSEvts", &read_evts );

	return;

}

void ISSHistogrammer::SetInputFile( std::vector<std::string> input_file_names ) {

	/// Overloaded function for a single file or multiple files
	TFile *input_file = nullptr;
	input_tree = new TChain( "evt_tree" );
	for( unsigned int i = 0; i < input_file_names.size(); i++ ) {

		// Add to the chain
		input_tree->Add( input_file_names[i].data() );

		// Read settings from first file in chain (that works)
		if( set.get() == nullptr ) {

			input_file = new TFile( input_file_names[0].data() );
			if( input_file->GetListOfKeys()->Contains( "Settings" ) )
				set = std::make_shared<ISSSettings>( (ISSSettings*)input_file->Get( "Settings" ) );
			else
				set = std::make_shared<ISSSettings>();

			// Close input file again
			input_file->Close();

		}

	}

	input_tree->SetBranchAddress( "ISSEvts", &read_evts );

	return;

}

void ISSHistogrammer::SetInputFile( std::string input_file_name ) {

	/// Overloaded function for a single file or multiple files
	input_tree = new TChain( "evt_tree" );
	input_tree->Add( input_file_name.data() );
	input_tree->SetBranchAddress( "ISSEvts", &read_evts );

	// Read settings from file if needed
	if( set.get() == nullptr ) {

		TFile *input_file = new TFile( input_file_name.data() );
		if( input_file->GetListOfKeys()->Contains( "Settings" ) )
			set = std::make_shared<ISSSettings>( (ISSSettings*)input_file->Get( "Settings" ) );
		else
			set = std::make_shared<ISSSettings>();

		// Close input file again
		input_file->Close();

	}

	return;

}

void ISSHistogrammer::SetPace4File( std::vector<std::string> input_file_names ) {

	/// Overloaded function for a single file or multiple files
	TTree *pace4tree = new TTree( "evt_tree", "evt_tree" );
	input_tree = (TChain*)pace4tree;
	read_evts = new ISSEvts;
	input_tree->Branch( "ISSEvts", "ISSEvts", &read_evts );
	input_tree->SetDirectory(0);

	for( unsigned int i = 0; i < input_file_names.size(); i++ ) {

		ReadPace4File( input_file_names[i] );

	}

	return;

}

void ISSHistogrammer::SetPace4File( std::string input_file_name ) {

	/// Overloaded function for a single file or multiple files
	TTree *pace4tree = new TTree( "evt_tree", "evt_tree" );
	input_tree = (TChain*)pace4tree;
	read_evts = new ISSEvts;
	input_tree->Branch( "ISSEvts", "ISSEvts", &read_evts );
	input_tree->SetDirectory(0);

	ReadPace4File( input_file_name );

	return;

}

void ISSHistogrammer::ReadPace4File( std::string input_file_name ) {

	// Get a copy of the reaction stuff
	// TODO: Make sure this is done inside the loop to get accurate energy losses
	// for that, we need to have a working copy constructor.
	//ISSReaction pace4react( *react );

	// for now, we just point to the original, which is fine for everything except energy losses
	std::shared_ptr<ISSReaction> pace4react = react;

	// Remember the ejectile
	unsigned int Zp = pace4react->GetEjectile()->GetZ();
	unsigned int Ap = pace4react->GetEjectile()->GetA();

	// Default reaction stuff
	double z0 = pace4react->GetArrayDistance();

	// Need an array event objects for later
	array_evt = std::make_unique<ISSArrayEvt>();
	arrayp_evt = std::make_unique<ISSArrayPEvt>();

	// Random generator
	TRandom3 rand;

	// Open the file
	std::ifstream pace4file;
	pace4file.open( input_file_name );

	// Check it is open
	if( !pace4file.is_open() ) {

		std::cout << "Couldn't open PACE4 file: " << input_file_name << std::endl;
		return;

	}

	// Read data on each line of the file
	std::string line;
	std::stringstream line_ss;
	double decay_mode, N_mode, N_All, chain, Z_f, N_f;
	double Z_c, N_c, J_c, J_f, M_Jc, fiss_prob;
	double Ex_i, Ex_f, Ep_lab, Ap_lab;

	// First two lines are headers
	for( unsigned int i = 0; i < 2; i++ )
		std::getline( pace4file, line );

	// Record position in the file to return to later
	std::streampos data_start = pace4file.tellg();
	std::ios_base::iostate file_state = pace4file.rdstate();

	// Count the number of data
	unsigned long number_of_data = 0;
	while( std::getline( pace4file, line ) && !pace4file.eof() )
		number_of_data++;
	std::cout << "Found " << number_of_data << " PACE4 events" << std::endl;

	// Go back to the start of the data
	pace4file.clear();
	pace4file.seekg( data_start );
	pace4file.setstate( file_state );

	// The rest should be data
	unsigned long current_data = 0;
	while( std::getline( pace4file, line ) && !pace4file.eof() ){

		// Clear the old data and increment counter
		read_evts->ClearEvt();
		current_data++;

		// Skip over really short lines
		if( line.length() < 10 ) continue;

		// Read in data
		line_ss.clear();
		line_ss.str("");
		line_ss << line;
		line_ss >> decay_mode >> N_mode >> N_All >> chain;
		line_ss >> Z_f >> N_f >> Z_c >> N_c >> J_c >> J_f;
		line_ss >> M_Jc >> fiss_prob >> Ex_i >> Ex_f >> Ep_lab >> Ap_lab;

		// sensible results?
		if( Ep_lab < 0 || Ap_lab < 0 || decay_mode <= 0 || decay_mode >= 4 )
			continue;

		// Check for fission when Z_c is negative
		if( Z_c < 0 ) continue;

		// Energy is in MeV, we need keV
		Ep_lab *= 1e3;

		// Now randomise the energy because PACE4 gives strange descrete values
		if( Ep_lab < 400. ) Ep_lab += ( rand.Rndm() - 0.5 ) * 400.;
		else if( Ep_lab < 800. ) Ep_lab += ( rand.Rndm() - 0.5 ) * 800.;
		else if( Ep_lab < 1200. ) Ep_lab += ( rand.Rndm() - 0.5 ) * 1200.;
		else Ep_lab += ( rand.Rndm() - 0.5 ) * 1600.;

		// Threshold the energy at 200 keV
		// This is lower than reality, but reasonable for simulation
		if( Ep_lab < 200. ) continue;

		// Randomise angle across the 0.1 degree precision
		// and convert from degrees to radians
		Ap_lab += rand.Rndm() * 8.0 - 4.0;
		if( Ap_lab > 180.0 ) Ap_lab -= 2.0 * ( Ap_lab - 180.0 );
		if( Ap_lab < 0.0 ) Ap_lab *= -1.0;
		Ap_lab *= TMath::DegToRad();

		// particle ID is decay_mode
		// 1: neutron - we don't care about these
		if( decay_mode == 1 ) continue;

		// 2: proton
		else if( decay_mode == 2 ) {

			pace4react->GetEjectile()->SetZ(1);
			pace4react->GetEjectile()->SetA(1);

		}

		// 3: alpha
		else if( decay_mode == 3 ) {

			pace4react->GetEjectile()->SetZ(2);
			pace4react->GetEjectile()->SetA(4);

		}

		// Randomly generate the phi angle
		double phi_lab = rand.Rndm() * TMath::TwoPi();
		double phi_det = TMath::TwoPi() - phi_lab; // almost true

		// Simulate the particle emission and get the detected energy
		double Edet = pace4react->SimulateEmission( Ep_lab, Ap_lab, phi_lab, 0 );

		// Important z values
		double z_meas = pace4react->GetZmeasured();

		// If we're not in the same hemisphere, forget it
		if( z0 * z_meas < 0 ) continue;

		// Shift the z in to the array reference
		if( z0 < 0. ) z_meas = -1.0 * z_meas + z0;
		else z_meas -= z0;

		// Find out where we hit the array
		int mod = array_evt->FindModule( phi_det );
		int row = array_evt->FindRow( z_meas );
		int pid = array_evt->FindPID( z_meas );
		int nid = array_evt->FindNID( phi_det );

		// Create an array event assuming that it hits it
		if( mod >= 0 && row >= 0 && pid >= 0 && nid >= 0 ){

			//std::cout << "Ep_lab = " << Ep_lab;
			//std::cout << ", Ep_det = " << Edet;
			//std::cout << ", theta_lab = " << Ap_lab;
			//std::cout << ", z_meas = " << pace4react.GetZmeasured();
			//std::cout << ", mod = " << (int)mod << ", row = " << (int)row;
			//std::cout << ", pid = " << (int)pid << ", nid = " << (int)nid;
			//std::cout << std::endl;

			array_evt->SetEvent( Edet, Edet, pid, nid, 1e6, 1e6, true, true, mod, row );
			arrayp_evt->SetEvent( Edet, Edet, pid, nid, 1e6, 1e6, true, true, mod, row );
			read_evts->AddEvt( array_evt );
			read_evts->AddEvt( arrayp_evt );

			// Fill the tree
			input_tree->Fill();

		}

		// Progress bar
		bool update_progress = false;
		if( number_of_data < 200 )
			update_progress = true;
		else if( current_data % (number_of_data/100) == 0 ||
				current_data == number_of_data )
			update_progress = true;

		if( update_progress ) {

			// Percent complete
			float percent = (float)current_data*100.0/(float)number_of_data;

			// Progress bar in GUI
			if( _prog_ ) {

				prog->SetPosition( percent );
				gSystem->ProcessEvents();

			}

			// Progress bar in terminal
			std::cout << " Reading data: " << std::setw(6) << std::setprecision(4);
			std::cout << percent << "%    \r";
			std::cout.flush();

		} // progress bar

	}

	// Reset the ejectile before we finish
	pace4react->GetEjectile()->SetZ(Zp);
	pace4react->GetEjectile()->SetA(Ap);


	// Close file and return
	pace4file.close();
	return;

}
