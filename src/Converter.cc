#include "Converter.hh"

ISSConverter::ISSConverter( std::shared_ptr<ISSSettings> myset ) {

	// We need to do initialise, but only after Settings are added
	set = myset;

	my_tm_stp_msb = 0;
	my_tm_stp_hsb = 0;
	
	// Resize counters
	ctr_asic_hit.resize( set->GetNumberOfArrayModules() );
	ctr_asic_ext.resize( set->GetNumberOfArrayModules() );
	ctr_asic_pause.resize( set->GetNumberOfArrayModules() );
	ctr_asic_resume.resize( set->GetNumberOfArrayModules() );
	ctr_caen_hit.resize( set->GetNumberOfCAENModules() );
	ctr_caen_ext.resize( set->GetNumberOfCAENModules() );
	ctr_mesy_hit.resize( set->GetNumberOfMesytecModules() );
	ctr_mesy_ext.resize( set->GetNumberOfMesytecModules() );

	// Start counters at zero
	StartFile();
	
	// Default that we do not have a source only run
	flag_source = false;
	
	// No progress bar by default
	_prog_ = false;
	
}

void ISSConverter::StartFile(){
	
	// Start counters at zero
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {
				
		ctr_asic_hit[i] = 0;	// hits on each module
		ctr_asic_ext[i] = 0;	// external timestamps
		ctr_asic_pause[i] = 0;
		ctr_asic_resume[i] = 0;

	}
	
	for( unsigned int i = 0; i < set->GetNumberOfCAENModules(); ++i ) {
		
		ctr_caen_hit[i] = 0;	// hits on each module
		ctr_caen_ext[i] = 0;	// external timestamps
		
	}
	
	for( unsigned int i = 0; i < set->GetNumberOfMesytecModules(); ++i ) {
		
		ctr_mesy_hit[i] = 0;	// hits on each module
		ctr_mesy_ext[i] = 0;	// external timestamps
		
	}
	
	return;
	
}

void ISSConverter::SetOutput( std::string output_file_name ){

	// Open output file
	output_file = new TFile( output_file_name.data(), "recreate" );
	//if( !flag_source ) output_file->SetCompressionLevel(0);

	return;

};


void ISSConverter::MakeTree() {

	// Create Root tree
	const int splitLevel = 2; // don't split branches = 0, full splitting = 99
	const int bufsize = sizeof(ISSCaenData) + sizeof(ISSAsicData) + sizeof(ISSInfoData);
	output_tree = new TTree( "iss", "iss" );
	data_packet = std::make_unique<ISSDataPackets>();
	output_tree->Branch( "data", "ISSDataPackets", data_packet.get(), bufsize, splitLevel );

	sorted_tree = (TTree*)output_tree->CloneTree(0);
	sorted_tree->SetName("iss_sort");
	sorted_tree->SetTitle( "Time sorted, calibrated ISS data" );
	sorted_tree->SetDirectory( output_file->GetDirectory("/") );
	output_tree->SetDirectory( output_file->GetDirectory("/") );
	
	output_tree->SetAutoFlush(-10e6);
	sorted_tree->SetAutoFlush(-10e6);

	asic_data = std::make_shared<ISSAsicData>();
	caen_data = std::make_shared<ISSCaenData>();
	mesy_data = std::make_shared<ISSMesyData>();
	info_data = std::make_shared<ISSInfoData>();

	asic_data->ClearData();
	caen_data->ClearData();
	mesy_data->ClearData();
	info_data->ClearData();
	
	return;
	
}

void ISSConverter::MakeHists() {
	
	std::string hname, htitle;
	std::string dirname, maindirname, subdirname;
	
	// Make directories
	maindirname = "asic_hists";

	// Resize vectors
	hasic.resize( set->GetNumberOfArrayModules() );
	hasic_cal.resize( set->GetNumberOfArrayModules() );
	hpside.resize( set->GetNumberOfArrayModules() );
	hnside.resize( set->GetNumberOfArrayModules() );
	
	// Loop over ISS modules
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {
		
		hasic[i].resize( set->GetNumberOfArrayASICs() );
		hasic_cal[i].resize( set->GetNumberOfArrayASICs() );
		subdirname = "/module_" + std::to_string(i);
		dirname = maindirname + subdirname;
		
		if( !output_file->GetDirectory( dirname.data() ) )
			output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );
			
		// calibrated p-side sum
		hname = "pside_mod" + std::to_string(i);
		htitle = "Calibrated p-side ASIC spectra for module " + std::to_string(i);
		htitle += ";Energy (keV);Counts per 15 keV";
		
		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hpside[i] = (TH1F*)output_file->Get( hname.data() );
		
		else {
			
			hpside[i] = new TH1F( hname.data(), htitle.data(),
						1500, -7.5, 29992.5 );
			hpside[i]->SetDirectory(
					output_file->GetDirectory( dirname.data() ) );
			
		}

		// calibrated n-side sum
		hname = "nside_mod" + std::to_string(i);
		htitle = "Calibrated n-side ASIC spectra for module " + std::to_string(i);
		htitle += ";Energy (keV);Counts per 15 keV";
		
		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hnside[i] = (TH1F*)output_file->Get( hname.data() );
		
		else {
			
			hnside[i] = new TH1F( hname.data(), htitle.data(),
						1500, -7.5, 29992.5 );
			hnside[i]->SetDirectory(
					output_file->GetDirectory( dirname.data() ) );
			
		}

		// Loop over ASICs for the array
		for( unsigned int j = 0; j < set->GetNumberOfArrayASICs(); ++j ) {
			
			if( !output_file->GetDirectory( dirname.data() ) )
				output_file->mkdir( dirname.data() );
			output_file->cd( dirname.data() );
				
			// Uncalibrated
			hname = "asic_" + std::to_string(i);
			hname += "_" + std::to_string(j);
				
			htitle = "Raw ASIC spectra for module " + std::to_string(i);
			htitle += ", ASIC " + std::to_string(j);
			
			htitle += ";Channel;ADC value;Counts";
			
			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hasic[i][j] = (TH2F*)output_file->Get( hname.data() );
				
			else {
					
				hasic[i][j] = new TH2F( hname.data(), htitle.data(),
							set->GetNumberOfArrayChannels(), -0.5, set->GetNumberOfArrayChannels()-0.5,
							4096, -0.5, 4095.5 );
				hasic[i][j]->SetDirectory(
						output_file->GetDirectory( dirname.data() ) );
					
			}
			
			// Calibrated
			hname = "asic_" + std::to_string(i);
			hname += "_" + std::to_string(j);
			hname += "_cal";
				
			htitle = "Calibrated ASIC spectra for module " + std::to_string(i);
			htitle += ", ASIC " + std::to_string(j);
			
			htitle += ";Channel;Energy (keV);Counts per 15 keV";
			
			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hasic_cal[i][j] = (TH2F*)output_file->Get( hname.data() );
				
			else {
					
				hasic_cal[i][j] = new TH2F( hname.data(), htitle.data(),
							set->GetNumberOfArrayChannels(), -0.5, set->GetNumberOfArrayChannels()-0.5,
							1500, -7.5, 29992.5 );
				hasic_cal[i][j]->SetDirectory(
						output_file->GetDirectory( dirname.data() ) );
					
			}

				
		}
		
	}
	
	// Make directories
	maindirname = "caen_hists";
	
	// Resive vectors
	hcaen_qlong.resize( set->GetNumberOfCAENModules() );
	hcaen_qshort.resize( set->GetNumberOfCAENModules() );
	hcaen_qdiff.resize( set->GetNumberOfCAENModules() );
	hcaen_cal.resize( set->GetNumberOfCAENModules() );
	
	// Loop over CAEN modules
	for( unsigned int i = 0; i < set->GetNumberOfCAENModules(); ++i ) {
		
		hcaen_qlong[i].resize( set->GetNumberOfCAENChannels() );
		hcaen_qshort[i].resize( set->GetNumberOfCAENChannels() );
		hcaen_qdiff[i].resize( set->GetNumberOfCAENChannels() );
		hcaen_cal[i].resize( set->GetNumberOfCAENChannels() );
		dirname = maindirname + "/module_" + std::to_string(i);
		
		if( !output_file->GetDirectory( dirname.data() ) )
			output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );
		
		// Loop over channels of each CAEN module
		for( unsigned int j = 0; j < set->GetNumberOfCAENChannels(); ++j ) {
			
			// Uncalibrated - Qlong
			hname = "caen_" + std::to_string(i);
			hname += "_" + std::to_string(j);
			hname += "_qlong";
			
			htitle = "Raw CAEN V1725 spectra for module " + std::to_string(i);
			htitle += ", channel " + std::to_string(j);
			
			htitle += ";Qlong;Counts";
			
			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hcaen_qlong[i][j] = (TH1F*)output_file->Get( hname.data() );
			
			else {
				
				hcaen_qlong[i][j] = new TH1F( hname.data(), htitle.data(),
											 65536, -0.5, 65535.5 );
				
				hcaen_qlong[i][j]->SetDirectory(
												output_file->GetDirectory( dirname.data() ) );
				
			}
			
			// Uncalibrated - Qshort
			hname = "caen_" + std::to_string(i);
			hname += "_" + std::to_string(j);
			hname += "_qshort";
			
			htitle = "Raw CAEN V1725 spectra for module " + std::to_string(i);
			htitle += ", channel " + std::to_string(j);
			
			htitle += ";Qshort;Counts";
			
			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hcaen_qshort[i][j] = (TH1F*)output_file->Get( hname.data() );
			
			else {
				
				hcaen_qshort[i][j] = new TH1F( hname.data(), htitle.data(),
											  32768, -0.5, 32767.5 );
				
				hcaen_qshort[i][j]->SetDirectory(
												 output_file->GetDirectory( dirname.data() ) );
				
			}
			
			// Uncalibrated - Qdiff
			hname = "caen_" + std::to_string(i);
			hname += "_" + std::to_string(j);
			hname += "_qdiff";
			
			htitle = "Raw CAEN V1725 spectra for module " + std::to_string(i);
			htitle += ", channel " + std::to_string(j);
			
			htitle += ";Qdiff;Counts";
			
			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hcaen_qdiff[i][j] = (TH1F*)output_file->Get( hname.data() );
			
			else {
				
				hcaen_qdiff[i][j] = new TH1F( hname.data(), htitle.data(),
											 65536, -0.5, 65535.5 );
				
				hcaen_qdiff[i][j]->SetDirectory(
												output_file->GetDirectory( dirname.data() ) );
				
			}
			
			// Calibrated
			hname = "caen_" + std::to_string(i);
			hname += "_" + std::to_string(j);
			hname += "_cal";
			
			htitle = "Calibrated CAEN V1725 spectra for module " + std::to_string(i);
			htitle += ", channel " + std::to_string(j);
			
			htitle += ";Energy (keV);Counts per 10 keV";
			
			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hcaen_cal[i][j] = (TH1F*)output_file->Get( hname.data() );
			
			else {
				
				hcaen_cal[i][j] = new TH1F( hname.data(), htitle.data(),
										   4000, -5, 39995 );
				
				hcaen_cal[i][j]->SetDirectory(
											  output_file->GetDirectory( dirname.data() ) );
				
			}
			
			
		}
		
	}
	
	// Make directories
	maindirname = "mesy_hists";
	
	// Resive vectors
	hmesy_qlong.resize( set->GetNumberOfMesytecModules() );
	hmesy_qshort.resize( set->GetNumberOfMesytecModules() );
	hmesy_qdiff.resize( set->GetNumberOfMesytecModules() );
	hmesy_cal.resize( set->GetNumberOfMesytecModules() );
	
	// Loop over CAEN modules
	for( unsigned int i = 0; i < set->GetNumberOfMesytecModules(); ++i ) {
		
		hmesy_qlong[i].resize( set->GetNumberOfMesytecChannels() );
		hmesy_qshort[i].resize( set->GetNumberOfMesytecChannels() );
		hmesy_qdiff[i].resize( set->GetNumberOfMesytecChannels() );
		hmesy_cal[i].resize( set->GetNumberOfMesytecChannels() );
		dirname = maindirname + "/module_" + std::to_string(i);
		
		if( !output_file->GetDirectory( dirname.data() ) )
			output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );
		
		// Loop over channels of each CAEN module
		for( unsigned int j = 0; j < set->GetNumberOfMesytecChannels(); ++j ) {
			
			// Uncalibrated - Qlong
			hname = "mesy_" + std::to_string(i);
			hname += "_" + std::to_string(j);
			hname += "_qlong";
			
			htitle = "Raw Mesytec spectra for module " + std::to_string(i);
			htitle += ", channel " + std::to_string(j);
			
			htitle += ";Qlong;Counts";
			
			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hmesy_qlong[i][j] = (TH1F*)output_file->Get( hname.data() );
			
			else {
				
				hmesy_qlong[i][j] = new TH1F( hname.data(), htitle.data(),
											 65536, -0.5, 65535.5 );
				
				hmesy_qlong[i][j]->SetDirectory(
												output_file->GetDirectory( dirname.data() ) );
				
			}
			
			// Uncalibrated - Qshort
			hname = "mesy_" + std::to_string(i);
			hname += "_" + std::to_string(j);
			hname += "_qshort";
			
			htitle = "Raw Mesytec spectra for module " + std::to_string(i);
			htitle += ", channel " + std::to_string(j);
			
			htitle += ";Qshort;Counts";
			
			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hmesy_qshort[i][j] = (TH1F*)output_file->Get( hname.data() );
			
			else {
				
				hmesy_qshort[i][j] = new TH1F( hname.data(), htitle.data(),
											  32768, -0.5, 32767.5 );
				
				hmesy_qshort[i][j]->SetDirectory(
												output_file->GetDirectory( dirname.data() ) );
				
			}
			
			// Uncalibrated - Qdiff
			hname = "mesy_" + std::to_string(i);
			hname += "_" + std::to_string(j);
			hname += "_qdiff";
			
			htitle = "Raw Mesytec spectra for module " + std::to_string(i);
			htitle += ", channel " + std::to_string(j);
			
			htitle += ";Qdiff;Counts";
			
			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hmesy_qdiff[i][j] = (TH1F*)output_file->Get( hname.data() );
			
			else {
				
				hmesy_qdiff[i][j] = new TH1F( hname.data(), htitle.data(),
											 65536, -0.5, 65535.5 );
				
				hmesy_qdiff[i][j]->SetDirectory(
												output_file->GetDirectory( dirname.data() ) );
				
			}
			
			// Calibrated
			hname = "mesy_" + std::to_string(i);
			hname += "_" + std::to_string(j);
			hname += "_cal";
			
			htitle = "Calibrated Mesytec spectra for module " + std::to_string(i);
			htitle += ", channel " + std::to_string(j);
			
			htitle += ";Energy (keV);Counts per 10 keV";
			
			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hmesy_cal[i][j] = (TH1F*)output_file->Get( hname.data() );
			
			else {
				
				hmesy_cal[i][j] = new TH1F( hname.data(), htitle.data(),
										   4000, -5, 39995 );
				
				hmesy_cal[i][j]->SetDirectory(
											  output_file->GetDirectory( dirname.data() ) );
				
			}
			
			
		}
		
	}
	
	// Make directories
	dirname = "timing_hists";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );
	
	// Loop over ISS modules
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {
		
		
	} // i: number of ISS modules

	// Resize vectors
	asic_pulser_energy.resize( set->GetNumberOfArrayModules() );
	hasic_hit.resize( set->GetNumberOfArrayModules() );
	hasic_ext.resize( set->GetNumberOfArrayModules() );
	hasic_pause.resize( set->GetNumberOfArrayModules() );
	hasic_resume.resize( set->GetNumberOfArrayModules() );
	hcaen_hit.resize( set->GetNumberOfCAENModules() );
	hcaen_ext.resize( set->GetNumberOfCAENModules() );
	hmesy_hit.resize( set->GetNumberOfMesytecModules() );
	hmesy_ext.resize( set->GetNumberOfMesytecModules() );

	// Loop over ISS modules
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {
		
		asic_pulser_energy[i].resize(2);
		
		// Loop over pulsers per module
		for( unsigned int j = 0; j < 2; ++j ) {
			
			// Energy histogram of pulser channel
			hname = "asic_pulser_energy_" + std::to_string(i) + "_" + std::to_string(j);
			htitle = "ASIC energy for pulser " + std::to_string(j) + " event in module ";
			htitle += std::to_string(i) + ";ADC value;counts";
			
			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				asic_pulser_energy[i][j] = (TH1F*)output_file->Get( hname.data() );
			
			else {
				
				asic_pulser_energy[i][j] = new TH1F( hname.data(), htitle.data(), 4096, -0.5, 4095.5 );
				
				asic_pulser_energy[i][j]->SetDirectory(
													   output_file->GetDirectory( dirname.data() ) );
				
			}
			
		} // j: number of pulsers per module


		// Timestamp of hits
		hname = "hasic_hit" + std::to_string(i);
		htitle = "Profile of ts versus hit_id in ISS module " + std::to_string(i);

		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hasic_hit[i] = (TProfile*)output_file->Get( hname.data() );

		else {

			hasic_hit[i] = new TProfile( hname.data(), htitle.data(), 10800, 0., 108000. );
			hasic_hit[i]->SetDirectory(
					output_file->GetDirectory( dirname.data() ) );

		}
		
		// Timestamps of external FPGA triggers from 10 Hz sync pulser
		hname = "hasic_ext" + std::to_string(i);
		htitle = "Profile of external trigger ts versus hit_id in ISS module " + std::to_string(i);

		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hasic_ext[i] = (TProfile*)output_file->Get( hname.data() );

		else {

			hasic_ext[i] = new TProfile( hname.data(), htitle.data(), 10800, 0., 108000. );
			hasic_ext[i]->SetDirectory(
					output_file->GetDirectory( dirname.data() ) );

		}

		// Pause events timestamps
		hname = "hasic_pause" + std::to_string(i);
		htitle = "Profile of ts versus pause events in ISS module " + std::to_string(i);

		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hasic_pause[i] = (TProfile*)output_file->Get( hname.data() );

		else {

			hasic_pause[i] = new TProfile( hname.data(), htitle.data(), 1000, 0., 10000. );
			hasic_pause[i]->SetDirectory(
					output_file->GetDirectory( dirname.data() ) );

		}

		// Resume events timestamps
		hname = "hasic_resume" + std::to_string(i);
		htitle = "Profile of ts versus resume events in ISS module " + std::to_string(i);

		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hasic_resume[i] = (TProfile*)output_file->Get( hname.data() );

		else {

			hasic_resume[i] = new TProfile( hname.data(), htitle.data(), 1000, 0., 10000. );
			hasic_resume[i]->SetDirectory(
					output_file->GetDirectory( dirname.data() ) );

		}
	
	}
	
	// Loop over CAEN modules
	for( unsigned int i = 0; i < set->GetNumberOfCAENModules(); ++i ) {
		
		hname = "hcaen_hit" + std::to_string(i);
		htitle = "Profile of ts versus hit_id in CAEN module " + std::to_string(i);
		
		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hcaen_hit[i] = (TProfile*)output_file->Get( hname.data() );
		
		else {
			
			hcaen_hit[i] = new TProfile( hname.data(), htitle.data(), 10800, 0., 1080000. );
			hcaen_hit[i]->SetDirectory(
									   output_file->GetDirectory( dirname.data() ) );
			
		}
		
		
		hname = "hcaen_ext" + std::to_string(i);
		htitle = "Profile of external trigger ts versus hit_id in CAEN module " + std::to_string(i);
		
		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hcaen_ext[i] = (TProfile*)output_file->Get( hname.data() );
		
		else {
			
			hcaen_ext[i] = new TProfile( hname.data(), htitle.data(), 10800, 0., 108000. );
			hcaen_ext[i]->SetDirectory(
									   output_file->GetDirectory( dirname.data() ) );
			
		}
		
	}
	
	// Loop over Mesytec modules
	for( unsigned int i = 0; i < set->GetNumberOfMesytecModules(); ++i ) {
		
		hname = "hmesy_hit" + std::to_string(i);
		htitle = "Profile of ts versus hit_id in Mesytec module " + std::to_string(i);
		
		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hmesy_hit[i] = (TProfile*)output_file->Get( hname.data() );
		
		else {
			
			hmesy_hit[i] = new TProfile( hname.data(), htitle.data(), 10800, 0., 1080000. );
			hmesy_hit[i]->SetDirectory(
									   output_file->GetDirectory( dirname.data() ) );
			
		}
		
		
		hname = "hmesy_ext" + std::to_string(i);
		htitle = "Profile of external trigger ts versus hit_id in Mesytec module " + std::to_string(i);
		
		if( output_file->GetListOfKeys()->Contains( hname.data() ) )
			hmesy_ext[i] = (TProfile*)output_file->Get( hname.data() );
		
		else {
			
			hmesy_ext[i] = new TProfile( hname.data(), htitle.data(), 10800, 0., 108000. );
			hmesy_ext[i]->SetDirectory(
									   output_file->GetDirectory( dirname.data() ) );
			
		}
		
	}
	
	return;
	
}

void ISSConverter::ResetHists() {
	
	std::cout << "in ISSConverter::ResetHist()" << std::endl;
	
	for( unsigned int i = 0; i < hasic_hit.size(); ++i )
		hasic_hit[i]->Reset("ICESM");
	
	for( unsigned int i = 0; i < hasic_ext.size(); ++i )
		hasic_ext[i]->Reset("ICESM");
	
	for( unsigned int i = 0; i < hasic_pause.size(); ++i )
		hasic_pause[i]->Reset("ICESM");

	for( unsigned int i = 0; i < hasic_resume.size(); ++i )
		hasic_resume[i]->Reset("ICESM");
	
	for( unsigned int i = 0; i < hcaen_hit.size(); ++i )
		hcaen_hit[i]->Reset("ICESM");
	
	for( unsigned int i = 0; i < hcaen_ext.size(); ++i )
		hcaen_ext[i]->Reset("ICESM");
	
	for( unsigned int i = 0; i < hmesy_hit.size(); ++i )
		hmesy_hit[i]->Reset("ICESM");
	
	for( unsigned int i = 0; i < hmesy_ext.size(); ++i )
		hmesy_ext[i]->Reset("ICESM");
	
	for( unsigned int i = 0; i < asic_pulser_energy.size(); ++i )
		for( unsigned int j = 0; j < asic_pulser_energy[i].size(); ++j )
			asic_pulser_energy[i][j]->Reset("ICESM");
	
	for( unsigned int i = 0; i < hasic.size(); ++i )
		for( unsigned int j = 0; j < hasic[i].size(); ++j )
			hasic[i][j]->Reset("ICESM");

	for( unsigned int i = 0; i < hasic_cal.size(); ++i )
		for( unsigned int j = 0; j < hasic_cal[i].size(); ++j )
			hasic_cal[i][j]->Reset("ICESM");
	
	for( unsigned int i = 0; i < hcaen_qlong.size(); ++i )
		for( unsigned int j = 0; j < hcaen_qlong[i].size(); ++j )
			hcaen_qlong[i][j]->Reset("ICESM");
	
	for( unsigned int i = 0; i < hcaen_qshort.size(); ++i )
		for( unsigned int j = 0; j < hcaen_qshort[i].size(); ++j )
			hcaen_qshort[i][j]->Reset("ICESM");
	
	for( unsigned int i = 0; i < hcaen_qdiff.size(); ++i )
		for( unsigned int j = 0; j < hcaen_qdiff[i].size(); ++j )
			hcaen_qdiff[i][j]->Reset("ICESM");
	
	for( unsigned int i = 0; i < hcaen_cal.size(); ++i )
		for( unsigned int j = 0; j < hcaen_cal[i].size(); ++j )
			hcaen_cal[i][j]->Reset("ICESM");
	
	for( unsigned int i = 0; i < hmesy_qlong.size(); ++i )
		for( unsigned int j = 0; j < hmesy_qlong[i].size(); ++j )
			hmesy_qlong[i][j]->Reset("ICESM");
	
	for( unsigned int i = 0; i < hmesy_qshort.size(); ++i )
		for( unsigned int j = 0; j < hmesy_qshort[i].size(); ++j )
			hmesy_qshort[i][j]->Reset("ICESM");
	
	for( unsigned int i = 0; i < hmesy_qdiff.size(); ++i )
		for( unsigned int j = 0; j < hmesy_qdiff[i].size(); ++j )
			hmesy_qdiff[i][j]->Reset("ICESM");
	
	for( unsigned int i = 0; i < hmesy_cal.size(); ++i )
		for( unsigned int j = 0; j < hmesy_cal[i].size(); ++j )
			hmesy_cal[i][j]->Reset("ICESM");
	
	for( unsigned int i = 0; i < hpside.size(); ++i )
		hpside[i]->Reset("ICESM");
	
	for( unsigned int i = 0; i < hnside.size(); ++i )
		hnside[i]->Reset("ICESM");
	
	return;
	
}

// Function to copy the header from a DataSpy, for example
void ISSConverter::SetBlockHeader( char *input_header ){
	
	// Copy header
	for( unsigned int i = 0; i < HEADER_SIZE; i++ )
		block_header[i] = input_header[i];

	return;
	
}

// Function to process header words
void ISSConverter::ProcessBlockHeader( unsigned long nblock ){
		
	// For each new header, reset the swap mode
	swap = 0;
	
	// Flag if we have ASIC data
	flag_asic_data = false;
	
	// Flags for CAEN data items
	flag_caen_data0 = false;
	flag_caen_data1 = false;
	flag_caen_data2 = false;
	flag_caen_data3 = false;
	flag_caen_trace = false;
	
	// Flags for Mesytec data items
	flag_mesy_data0 = false;
	flag_mesy_data1 = false;
	flag_mesy_data3 = false;
	flag_mesy_trace = false;
	
	// Flag when we find the end of the data
	flag_terminator = false;

	// Process header.
	for( UInt_t i = 0; i < 8; i++ )
		header_id[i] = block_header[i];
	
	header_sequence =
	(block_header[8] & 0xFF) << 24 | (block_header[9]& 0xFF) << 16 |
	(block_header[10]& 0xFF) << 8  | (block_header[11]& 0xFF);
	
	header_stream = (block_header[12] & 0xFF) << 8 | (block_header[13]& 0xFF);
	
	header_tape = (block_header[14] & 0xFF) << 8 | (block_header[15]& 0xFF);
	
	header_MyEndian = (block_header[16] & 0xFF) << 8 | (block_header[17]& 0xFF);
	
	header_DataEndian = (block_header[18] & 0xFF) << 8 | (block_header[19]& 0xFF);
	
	header_DataLen =
	(block_header[20] & 0xFF) | (block_header[21]& 0xFF) << 8 |
	(block_header[22] & 0xFF) << 16  | (block_header[23]& 0xFF) << 24 ;
	

	if( std::string(header_id).substr(0,8) != "EBYEDATA" ) {
	
		std::cerr << "Bad header in block " << nblock << std::endl;
		exit(0);
	
	}
	
	//std::cout << nblock << "\t" << header_DataLen << std::endl;
	
	return;
	
}


// Function to copy the main data from a DataSpy, for example
void ISSConverter::SetBlockData( char *input_data ){
	
	// Copy header
	for( UInt_t i = 0; i < MAIN_SIZE; i++ )
		block_data[i] = input_data[i];

	return;
	
}


// Function to process data words
void ISSConverter::ProcessBlockData( unsigned long nblock ){

	(void) nblock; // Avoid unused parameter warning.
	
	// Get the data in 64-bit words and check endieness and swap if needed
	// Data format here: http://npg.dl.ac.uk/documents/edoc504/edoc504.html
	// Unpack in to two 32-bit words for purposes of data format
		
	// Swap mode is unknown for the first block of data, so let's work it out
	if( (swap & SWAP_KNOWN) == 0 ) {

		// See if we can figure out the swapping - the DataEndian word of the
		// header is 256 if the endianness is correct, otherwise swap endianness
		if( header_DataEndian != 256 ) swap |= SWAP_ENDIAN;
		
		// However, that is not all, the words may also be swapped, so check
		// for that. Bits 31:30 should always be zero in the timestamp word
		for( UInt_t i = 0; i < WORD_SIZE; i++ ) {
			word = (swap & SWAP_ENDIAN) ? Swap64(data[i]) : data[i];
			if( word & 0xC000000000000000LL ) {
				swap |= SWAP_KNOWN;
				break;
			}
			if( word & 0x00000000C0000000LL ) {
				swap |= SWAP_KNOWN;
				swap |= SWAP_WORDS;
				break;
			}
		}
		
	}

	
	// Process all words
	for( UInt_t i = 0; i < WORD_SIZE; i++ ) {
				
		word = GetWord(i);
		word_0 = (word & 0xFFFFFFFF00000000) >> 32;
		word_1 = (word & 0x00000000FFFFFFFF);
		
		// Check the trailer: reject or keep the block.
		if( ( word_0 & 0xFFFFFFFF ) == 0xFFFFFFFF ||
		    ( word_0 & 0xFFFFFFFF ) == 0x5E5E5E5E ||
		    ( word_1 & 0xFFFFFFFF ) == 0xFFFFFFFF ||
		    ( word_1 & 0xFFFFFFFF ) == 0x5E5E5E5E ){
			
			flag_terminator = true;
			return;
			
		}
		else if( i >= header_DataLen/sizeof(ULong64_t) ){
			
			flag_terminator = true;
			return;
			
		}
			
		// Data type is highest two bits
		my_type = ( word_0 >> 30 ) & 0x3;
		
		// ADC data - we need to know if it is CAEN or ASIC
		if( my_type == 0x3 ){
			
			// Decide if this is an ASIC or CAEN event
			// ISS/R3B ASICs will have 28th bit of word_1 set to 1
			// This is for data after to R3B data format change (June 2021)
			// Otherwise, we read it in from the settings file
			if( ((word_1 >> 28) & 0x00000001) == 0x00000001 ||
			    set->IsASICOnly() ){

				flag_asic_data = true;
				ProcessASICData();
				
			}
			
			// Otherwise it's from the VME crates
			else {
				
				// Check channel ID and VME crate ID
				GetVMEChanID();
				
				// We've got something from the CAEN DAQ
				if( my_vme_id == 0 ) {
					
					ProcessCAENData();
					FinishCAENData();

				}
				
				// or we've got something from the Mesytec DAQ
				else if( my_vme_id == 1 ) {
					
					ProcessMesytecData();
					FinishMesytecData();
					
				}

			}

		}
		
		// Information data
		else if( my_type == 0x2 ){
			
			ProcessInfoData();

		}
		
		// Trace header
		else if( my_type == 0x1 ){
			
			// Get channel ID
			GetVMEChanID();
			
			// make a vector to store the samples
			std::vector<unsigned short> samples;

			// contains the sample length
			nsamples = word_0 & 0xFFFF; // 16 bits from 0
			
			// reconstruct time stamp= MSB+LSB
			my_tm_stp_lsb = word_1 & 0x0FFFFFFF;  // 28 bits from 0
			my_tm_stp = ( my_tm_stp_msb << 28 ) | my_tm_stp_lsb;
			caen_data->SetTimeStamp( my_tm_stp );
			
			// Get the samples from the trace
			for( unsigned int j = 0; j < nsamples/4; j++ ){
				
				// get next word
				ULong64_t sample_packet = GetWord(i++);

				UInt_t block_test = ( sample_packet >> 32 ) & 0x00000000FFFFFFFF;
				unsigned char trace_test = ( sample_packet >> 62 ) & 0x0000000000000003;
				
				//if( trace_test == 0 && block_test != 0x5E5E5E5E ){
				if( block_test != 0x5E5E5E5E ){
				
					// Pairs need to be swapped
					samples.push_back( ( sample_packet >> 32 ) & 0x0000000000003FFF );
					samples.push_back( ( sample_packet >> 48 ) & 0x0000000000003FFF );
					samples.push_back( sample_packet & 0x0000000000003FFF );
					samples.push_back( ( sample_packet >> 16 ) & 0x0000000000003FFF );
					
				}
				
				else {
					
					std::cout << "This isn't a trace anymore..." << std::endl;
					std::cout << "Sample #" << j << " of " << nsamples << std::endl;
					std::cout << " trace_test = " << (int)trace_test << std::endl;

					//i--;
					//break;
					
				}
				
			}
			
			// Add to the CAEN packet
			if( my_vme_id == 0 ){
				
				caen_data->SetModule( my_mod_id );
				caen_data->SetChannel( my_ch_id );
				for( unsigned int k = 0; k < samples.size(); k++ )
					caen_data->AddSample( samples.at(k) );
				
				flag_caen_trace = true;
				FinishCAENData();

			}
			
			// Add to the Mesytec packet
			else if( my_vme_id == 1 ){
				
				mesy_data->SetModule( my_mod_id );
				mesy_data->SetChannel( my_ch_id );
				for( unsigned int k = 0; k < samples.size(); k++ )
					mesy_data->AddSample( samples.at(k) );

				flag_mesy_trace = true;
				FinishMesytecData();

			}
				
		}
		
		else {
			
			// output error message!
			std::cerr << "WARNING: WRONG TYPE! word 0: " << word_0;
			std::cerr << ", my_type: " << my_type << std::endl;
		
		}
		
	} // loop - i < header_DataLen
	
	return;

}

void ISSConverter::ProcessASICData(){

	// ISS/R3B ASIC data format
	my_hit = ( word_0 >> 29 ) & 0x1;
	my_adc_data = word_0 & 0xFFF; // 12 bits from 0
	
	// ADCchannelIdent are bits 28:12
	// mod_id= bit 16:11, asic_id= bit 10:7, ch_id= bit 6:0
	unsigned int ADCchanIdent = (word_0 >> 12) & 0x0001FFFF; // 17 bits from 12
	my_mod_id = (ADCchanIdent >> 11) & 0x003F; // 6 bits from 11
	my_asic_id = (ADCchanIdent >> 7 ) & 0x000F; // 4 bits from 7
	my_ch_id = ADCchanIdent & 0x007F; // 7 bits from 0
	
	my_tm_stp_lsb = word_1 & 0x0FFFFFFF;  // 28 bits from 0
	
	// Check things make sense
	if( my_mod_id >= set->GetNumberOfArrayModules() ||
		my_asic_id >= set->GetNumberOfArrayASICs() ||
		my_ch_id >= set->GetNumberOfArrayChannels() ) {
		
		std::cout << "Bad ASIC event with mod_id=" << my_mod_id;
		std::cout << " asic_id=" << my_asic_id;
		std::cout << " ch_id=" << my_ch_id << std::endl;
		
		return;
		
	}
	
	// reconstruct time stamp= HSB+MSB+LSB
	my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb_asic << 28 ) | my_tm_stp_lsb;
	
	// Calibrate
	my_energy = cal->AsicEnergy( my_mod_id, my_asic_id, my_ch_id, my_adc_data );
	
	// Fill histograms
	hasic[my_mod_id][my_asic_id]->Fill( my_ch_id, my_adc_data );
	hasic_cal[my_mod_id][my_asic_id]->Fill( my_ch_id, my_energy );
	
	// Is it disabled?
	if( !cal->AsicEnabled( my_mod_id, my_asic_id ) ) return;
	
	// Pulser in a spare n-side channel should be counted as info data
	bool pulser_trigger = false;
	
	if( my_asic_id == set->GetArrayPulserAsic0() &&
	   my_ch_id == set->GetArrayPulserChannel0() ) {
		
		// Check energy to set threshold
		asic_pulser_energy[my_mod_id][0]->Fill( my_adc_data );
		
		info_data->SetModule( my_mod_id );
		info_data->SetTimeStamp( my_tm_stp );
		info_data->SetCode( set->GetArrayPulserCode0() );
		pulser_trigger = true;
		
	}
	
	else if( my_asic_id == set->GetArrayPulserAsic1() &&
	   my_ch_id == set->GetArrayPulserChannel1() ) {
		
		// Check energy to set threshold
		asic_pulser_energy[my_mod_id][1]->Fill( my_adc_data );
		
		info_data->SetModule( my_mod_id );
		info_data->SetTimeStamp( my_tm_stp );
		info_data->SetCode( set->GetArrayPulserCode1() );
		pulser_trigger = true;
		
	}

	// If it's a pulser trigger above an energy threshold, then count it
	if( my_adc_data > set->GetArrayPulserThreshold() && pulser_trigger ) {
		
		data_packet->SetData( info_data );
		if( !flag_source ) output_tree->Fill();
		info_data->Clear();
		data_packet->ClearData();
		
	}

	
	// Otherwise fill a physics data item
	else {
		
		if( my_asic_id == 0 || my_asic_id == 2 || my_asic_id == 3 || my_asic_id == 5 )
			hpside[my_mod_id]->Fill( my_energy );
		else if( my_asic_id == 1 || my_asic_id == 4 )
			hnside[my_mod_id]->Fill( my_energy );
		
		
		// Make an AsicData item
		asic_data->SetTimeStamp( my_tm_stp + cal->AsicTime( my_mod_id, my_asic_id ) );
		asic_data->SetWalk( (int)cal->AsicWalk( my_mod_id, my_asic_id, my_energy, my_hit ) );
		asic_data->SetAdcValue( my_adc_data );
		asic_data->SetHitBit( my_hit );
		asic_data->SetModule( my_mod_id );
		asic_data->SetAsic( my_asic_id );
		asic_data->SetChannel( my_ch_id );
		asic_data->SetEnergy( my_energy );
		
		// Check if it's over threshold
		if( my_adc_data > cal->AsicThreshold( my_mod_id, my_asic_id, my_ch_id ) )
			asic_data->SetThreshold( true );
		else asic_data->SetThreshold( false );
		
		// Set this data and fill event to tree
		data_packet->SetData( asic_data );
		if( !flag_source ) output_tree->Fill();
		asic_data->Clear();
		data_packet->ClearData();
		
		// Count asic hit per module
		ctr_asic_hit[my_mod_id]++;
		hasic_hit[my_mod_id]->Fill( ctr_asic_hit[my_mod_id], my_tm_stp, 1 );

	}
		
	return;
	
}

void ISSConverter::GetVMEChanID(){
	
	// ADCchannelIdent are bits 28:16
	// vme_id= bit 12, mod_id= bit 11:8, data_id= bit 7:6, ch_id= bit 5:0
	// data_id: Qlong = 0; Qshort = 1; baseline = 2; fine timing = 3
	unsigned int ADCchanIdent = (word_0 >> 16) & 0x1FFF; // 13 bits from 16
	my_vme_id = (ADCchanIdent >> 12) & 0x0001; // 1 bits from 12
	my_mod_id = (ADCchanIdent >> 8) & 0x000F; // 4 bits from 8
	my_data_id = (ADCchanIdent >> 6 ) & 0x0003; // 2 bits from 6
	my_ch_id = ADCchanIdent & 0x003F; // 6 bits from 0
	
	return;
	
}

void ISSConverter::ProcessCAENData(){
	
	// CAEN data format
	my_adc_data = word_0 & 0xFFFF; // 16 bits from 0
	
	// Check things make sense
	if( my_mod_id >= set->GetNumberOfCAENModules() ||
	   my_ch_id >= set->GetNumberOfCAENChannels() ) {
		
		std::cout << "Bad CAEN event with mod_id=" << (int) my_mod_id;
		std::cout << " ch_id=" << (int) my_ch_id;
		std::cout << " data_id=" << (int) my_data_id << std::endl;
		return;
		
	}
	
	// reconstruct time stamp= MSB+LSB
	my_tm_stp_lsb = word_1 & 0x0FFFFFFF;  // 28 bits from 0
	my_tm_stp = ( my_tm_stp_msb << 28 ) | my_tm_stp_lsb;
	
	// CAEN timestamps are 4 ns precision for V1725 and 2 ns for V1730
	if( set->GetCAENModel( my_mod_id ) == 1730 ) my_tm_stp = my_tm_stp*2;
	else if( set->GetCAENModel( my_mod_id ) == 1725 ) my_tm_stp = my_tm_stp*4;
	else my_tm_stp = my_tm_stp*4;
	
	// First of the data items
	if( !flag_caen_data0 && !flag_caen_data1 && !flag_caen_data2 && !flag_caen_data3 ){
		
		// Make a CaenData item, need to add Qlong, Qshort and traces
		caen_data->SetTimeStamp( my_tm_stp );
		caen_data->SetCrate(0); // CAEN crate is always 0
		caen_data->SetModule( my_mod_id );
		caen_data->SetChannel( my_ch_id );
		
	}
	
	// If we already have all the data items, then the next event has
	// already occured before we found traces. This means that there
	// is not trace data. So set the flag to be true and finish the
	// event with an empty trace.
	else if( flag_caen_data0 && flag_caen_data1 && ( flag_caen_data2 || flag_caen_data3 ) ){
		
		// Fake trace flag, but with an empty trace
		flag_caen_trace = true;
		
		// Finish up the previous event
		FinishCAENData();
		
		// Then set the info correctly for this event
		caen_data->SetTimeStamp( my_tm_stp );
		caen_data->SetCrate(0); // CAEN crate is always 0
		caen_data->SetModule( my_mod_id );
		caen_data->SetChannel( my_ch_id );
		
	}
	
	// Qlong
	if( my_data_id == 0 ) {
		
		// Fill histograms
		hcaen_qlong[my_mod_id][my_ch_id]->Fill( my_adc_data );
		if( my_adc_data == 0xFFFF ) caen_data->SetQlong( 0 );
		else caen_data->SetQlong( my_adc_data );
		flag_caen_data0 = true;
		
	}
	
	// Qshort
	if( my_data_id == 1 ) {
		
		my_adc_data = my_adc_data & 0x7FFF; // 15 bits from 0
		hcaen_qshort[my_mod_id][my_ch_id]->Fill( my_adc_data );
		if( my_adc_data == 0x7FFF ) caen_data->SetQshort( 0 );
		else caen_data->SetQshort( my_adc_data );
		flag_caen_data1 = true;
		
	}
	
	// Extra word items
	// Do these have to be defined in the settings file?
	//  set->GetCAENExtras( my_mod_id, my_ch_id ) == 0 or 1
	//  0: Fine timing
	//  1: Baseline
	//if( my_data_id == 3 ) {
	//
	//	my_adc_data = my_adc_data & 0xFFFF; // 16 bits from 0
	//	flag_caen_data3 = true;
	//
	//	// Fine timing
	//	if( set->GetCAENExtras( my_mod_id, my_ch_id ) == 0 ){
	//
	//		// CAEN timestamps are 4 ns precision for V1725 and 2 ns for V1730
	//		if( set->GetCAENModel( my_mod_id ) == 1730 )
	//			caen_data->SetFineTime( (float)my_adc_data * 2. / 1000. );
	//		else if( set->GetCAENModel( my_mod_id ) == 1725 )
	//			caen_data->SetFineTime( (float)my_adc_data * 4. / 1000. );
	//		caen_data->SetBaseline( 0.0 );
	//
	//	}
	//
	//	// Baseline
	//	else if( set->GetCAENExtras( my_mod_id, my_ch_id ) == 1 ){
	//
	//		caen_data->SetFineTime( 0.0 );
	//		caen_data->SetBaseline( (float)my_adc_data / 4. );
	//
	//	}
	//
	//}
	
	
	// But MIDAS says my_data_id == 2 or 3
	//  2: basline
	//  3: fine timing
	// http://npg.dl.ac.uk/documents/edoc504/edoc504.html
	
	// Baseline
	if( my_data_id == 2 ) {
		
		my_adc_data = my_adc_data & 0xFFFF; // 16 bits from 0
		flag_caen_data2 = true;
		
		caen_data->SetFineTime( 0.0 );
		caen_data->SetBaseline( (float)my_adc_data / 4. );
		
	}
	
	// Fine timing
	if( my_data_id == 3 ) {
		
		my_adc_data = my_adc_data & 0x03FF; // 10 bits from 0
		flag_caen_data3 = true;
		
		// CAEN timestamps are 4 ns precision for V1725 and 2 ns for V1730
		if( set->GetCAENModel( my_mod_id ) == 1730 )
			caen_data->SetFineTime( (float)my_adc_data * 2. / 1000. );
		else if( set->GetCAENModel( my_mod_id ) == 1725 )
			caen_data->SetFineTime( (float)my_adc_data * 4. / 1000. );
		caen_data->SetBaseline( 0.0 );
		
	}
	
	return;
	
}

void ISSConverter::FinishCAENData(){
	
	// Got all items
	if( ( flag_caen_data0 && flag_caen_data1 && ( flag_caen_data2 || flag_caen_data3 ) ) || flag_caen_trace ){
		
		// Fill histograms
		hcaen_hit[caen_data->GetModule()]->Fill( ctr_caen_hit[caen_data->GetModule()], caen_data->GetTime(), 1 );
		
		// Difference between Qlong and Qshort
		int qdiff = (int)caen_data->GetQlong() - (int)caen_data->GetQshort();
		hcaen_qdiff[caen_data->GetModule()][caen_data->GetChannel()]->Fill( qdiff );
		
		// Choose the energy we want to use
		unsigned short adc_value = 0;
		std::string entype = cal->CaenType( caen_data->GetModule(), caen_data->GetChannel() );
		if( entype == "Qlong" ) adc_value = caen_data->GetQlong();
		else if( entype == "Qshort" ) adc_value = caen_data->GetQshort();
		else if( entype == "Qdiff" ) adc_value = caen_data->GetQdiff();
		my_energy = cal->CaenEnergy( caen_data->GetModule(), caen_data->GetChannel(), adc_value );
		caen_data->SetEnergy( my_energy );
		hcaen_cal[caen_data->GetModule()][caen_data->GetChannel()]->Fill( my_energy );
		
		// Check if it's over threshold
		if( adc_value > cal->CaenThreshold( caen_data->GetModule(), caen_data->GetChannel() ) )
			caen_data->SetThreshold( true );
		else caen_data->SetThreshold( false );
		
		
		// Check if this is actually just a timestamp
		flag_caen_info = false;
		if( caen_data->GetModule() == set->GetCAENPulserModule() &&
		   caen_data->GetChannel() == set->GetCAENPulserChannel() ){
			
			flag_caen_info = true;
			//my_info_code = set->GetCAENPulserCode();
			my_info_code = 20; // CAEN pulser is always 20 (defined here)
			
		}
		
		else if( caen_data->GetModule() == set->GetEBISModule() &&
				caen_data->GetChannel() == set->GetEBISChannel() ){
			
			flag_caen_info = true;
			//my_info_code = set->GetEBISCode();
			my_info_code = 21; // CAEN EBIS is always 21 (defined here), Array EBIS is 15
			
		}
		
		else if( caen_data->GetModule() == set->GetT1Module() &&
				caen_data->GetChannel() == set->GetT1Channel() ){
			
			flag_caen_info = true;
			//my_info_code = set->GetT1Code();
			my_info_code = 22; // CAEN T1 is always 22 (defined here)
			
		}
		
		else if( caen_data->GetModule() == set->GetSCModule() &&
				caen_data->GetChannel() == set->GetSCChannel() ){
			
			flag_caen_info = true;
			my_info_code = 23; // CAEN SC is always 23 (defined here)
			
		}
		
		else if( caen_data->GetModule() == set->GetLaserModule() &&
				caen_data->GetChannel() == set->GetLaserChannel() ){
			
			flag_caen_info = true;
			my_info_code = 24; // CAEN Laser status is always 24 (defined here)
			
		}
		
		// If this is a timestamp, fill an info event
		if( flag_caen_info ) {
			
			// Check it's over threshold and not just noise
			if( caen_data->IsOverThreshold() ) {
				
				// Add the time offset to this channel
				info_data->SetTimeStamp( caen_data->GetTime() + cal->CaenTime( caen_data->GetModule(), caen_data->GetChannel() ) );
				info_data->SetModule( caen_data->GetModule() + set->GetNumberOfArrayModules() );
				info_data->SetCode( my_info_code );
				data_packet->SetData( info_data );
				if( !flag_source ) output_tree->Fill();
				data_packet->ClearData();
				
			}
			
			// Fill histograms for external trigger
			if( my_info_code == 20 ) {
				
				hcaen_ext[caen_data->GetModule()]->Fill( ctr_caen_ext[caen_data->GetModule()], caen_data->GetTime(), 1 );
				
				// Count external trigger event
				ctr_caen_ext[caen_data->GetModule()]++;
				
			}
			
		}
		
		// Otherwise it is real data, so fill a caen event
		else {
			
			// Set this data and fill event to tree
			// Also add the time offset when we do this
			caen_data->SetTimeStamp( caen_data->GetTime() + cal->CaenTime( caen_data->GetModule(), caen_data->GetChannel() ) );
			data_packet->SetData( caen_data );
			if( !flag_source ) output_tree->Fill();
			data_packet->ClearData();
			
			//std::cout << "Complete CAEN event" << std::endl;
			//std::cout << "Trace length = " << caen_data->GetTraceLength() << std::endl;
			
		}
		
	}
	
	// missing something
	else if( (long long)my_tm_stp != (long long)caen_data->GetTimeStamp() ) {
		
		std::cout << "Missing something in CAEN data and new event occured" << std::endl;
		std::cout << " Qlong       = " << flag_caen_data0 << std::endl;
		std::cout << " Qshort      = " << flag_caen_data1 << std::endl;
		std::cout << " baseline    = " << flag_caen_data2 << std::endl;
		std::cout << " fine timing = " << flag_caen_data3 << std::endl;
		std::cout << " trace data  = " << flag_caen_trace << std::endl;
		
	}
	
	// This is normal, just not finished yet
	else return;
	
	// Count the hit, even if it's bad
	ctr_caen_hit[caen_data->GetModule()]++;
	
	// Assuming it did finish, in a good way or bad, clean up.
	flag_caen_data0 = false;
	flag_caen_data1 = false;
	flag_caen_data2 = false;
	flag_caen_data3 = false;
	flag_caen_trace = false;
	info_data->ClearData();
	caen_data->ClearData();
	
	return;
	
}

void ISSConverter::ProcessMesytecData(){
	
	// Mesytec data format
	my_adc_data = word_0 & 0xFFFF; // 16 bits from 0
	
	// Check things make sense
	if( my_mod_id >= set->GetNumberOfMesytecModules() ||
		my_ch_id >= set->GetNumberOfMesytecChannels() ) {
		
		std::cout << "Bad Mesytec event with mod_id=" << (int) my_mod_id;
		std::cout << " ch_id=" << (int) my_ch_id;
		std::cout << " data_id=" << (int) my_data_id << std::endl;
		return;
		
	}
	
	// reconstruct time stamp= MSB+LSB
	my_tm_stp_lsb = word_1 & 0x0FFFFFFF;  // 28 bits from 0
	my_tm_stp = ( my_tm_stp_msb << 28 ) | my_tm_stp_lsb;
	
	// Mesytec timestamps are 4 ns precision? TBD
	my_tm_stp = my_tm_stp*4;
	
	// First of the data items
	if( !flag_mesy_data0 && !flag_mesy_data1 && !flag_mesy_data2 && !flag_mesy_data3 ){
		
		// Make a MesyData item, need to add Qlong and traces
		mesy_data->SetTimeStamp( my_tm_stp );
		mesy_data->SetCrate(1); // Mesytec crate is always 1
		mesy_data->SetModule( my_mod_id );
		mesy_data->SetChannel( my_ch_id );
		
	}
	
	// If we already have all the data items, then the next event has
	// already occured before we found traces. This means that there
	// is not trace data. So set the flag to be true and finish the
	// event with an empty trace.
	else if( flag_mesy_data0 && flag_mesy_data1 && flag_mesy_data3 ){
		
		// Fake trace flag, but with an empty trace
		flag_mesy_trace = true;
		
		// Finish up the previous event
		FinishMesytecData();
		
		// Then set the info correctly for this event
		mesy_data->SetTimeStamp( my_tm_stp );
		mesy_data->SetCrate(1); // Mesytec crate is always 1
		mesy_data->SetModule( my_mod_id );
		mesy_data->SetChannel( my_ch_id );
		
	}
	
	// Qlong
	if( my_data_id == 0 ) {
		
		// Fill histograms
		hmesy_qlong[my_mod_id][my_ch_id]->Fill( my_adc_data );
		if( my_adc_data == 0xFFFF ) mesy_data->SetQlong( 0 );
		else mesy_data->SetQlong( my_adc_data );
		flag_mesy_data0 = true;
		
	}
	
	// Qshort
	if( my_data_id == 1 ) {
		
		// Fill histograms
		my_adc_data = my_adc_data & 0x7FFF; // 15 bits from 0
		hmesy_qshort[my_mod_id][my_ch_id]->Fill( my_adc_data );
		if( my_adc_data == 0x7FFF ) mesy_data->SetQshort( 0 );
		else mesy_data->SetQshort( my_adc_data );
		flag_mesy_data1 = true;
		
	}
	
	// Fine timing
	if( my_data_id == 3 ) {
		
		my_adc_data = my_adc_data & 0x03FF; // 10 bits from 0
		flag_mesy_data3 = true;
		
		// Mesy timestamps are 10 ns precision? TBD
		mesy_data->SetFineTime( (float)my_adc_data * 4. / 1000. );
		
	}
	
	return;
	
}

void ISSConverter::FinishMesytecData(){
	
	// Got all items
	if( ( flag_mesy_data0 && flag_mesy_data1 && flag_mesy_data3 ) || flag_mesy_trace ){
		
		// Fill histograms
		hmesy_hit[mesy_data->GetModule()]->Fill( ctr_mesy_hit[mesy_data->GetModule()], mesy_data->GetTime(), 1 );

		// Difference between Qlong and Qshort
		int qdiff = (int)mesy_data->GetQlong() - (int)mesy_data->GetQshort();
		hmesy_qdiff[mesy_data->GetModule()][mesy_data->GetChannel()]->Fill( qdiff );
		
		// Choose the energy we want to use
		unsigned short adc_value = 0;
		std::string entype = cal->MesytecType( mesy_data->GetModule(), mesy_data->GetChannel() );
		if( entype == "Qlong" ) adc_value = mesy_data->GetQlong();
		else if( entype == "Qshort" ) adc_value = mesy_data->GetQshort();
		else if( entype == "Qdiff" ) adc_value = mesy_data->GetQdiff();
		my_energy = cal->MesytecEnergy( mesy_data->GetModule(), mesy_data->GetChannel(), adc_value );
		mesy_data->SetEnergy( my_energy );
		hmesy_cal[mesy_data->GetModule()][mesy_data->GetChannel()]->Fill( my_energy );
		
		// Check if it's over threshold
		if( adc_value > cal->MesytecThreshold( mesy_data->GetModule(), mesy_data->GetChannel() ) )
			mesy_data->SetThreshold( true );
		else mesy_data->SetThreshold( false );
		
		
		// Set this data and fill event to tree
		// Also add the time offset when we do this
		mesy_data->SetTimeStamp( mesy_data->GetTime() + cal->MesytecTime( mesy_data->GetModule(), mesy_data->GetChannel() ) );
		data_packet->SetData( mesy_data );
		if( !flag_source ) output_tree->Fill();
		data_packet->ClearData();
		
	}
	
	// missing something
	else if( (long long)my_tm_stp != (long long)mesy_data->GetTimeStamp() ) {
		
		std::cout << "Missing something in Mesytec data and new event occured" << std::endl;
		std::cout << " Qlong       = " << flag_mesy_data0 << std::endl;
		std::cout << " Qshort      = " << flag_mesy_data1 << std::endl;
		std::cout << " fine timing = " << flag_mesy_data3 << std::endl;
		std::cout << " trace data  = " << flag_mesy_trace << std::endl;
		
	}
	
	// This is normal, just not finished yet
	else return;
	
	// Count the hit, even if it's bad
	ctr_mesy_hit[mesy_data->GetModule()]++;
	
	// Assuming it did finish, in a good way or bad, clean up.
	flag_mesy_data0 = false;
	flag_mesy_data1 = false;
	flag_mesy_data3 = false;
	flag_mesy_trace = false;
	info_data->ClearData();
	mesy_data->ClearData();
	
	return;
	
}



void ISSConverter::ProcessInfoData(){

	// MIDAS info data format
	my_mod_id = (word_0 >> 24) & 0x003F; // bits 24:29

	my_info_field = word_0 & 0x000FFFFF; //bits 0:19
	my_info_code = (word_0 >> 20) & 0x0000000F; //bits 20:23
	my_tm_stp_lsb = word_1 & 0x0FFFFFFF;  //bits 0:27

	// HSB of timestamp
	if( my_info_code == set->GetTimestampCode() ) {
		
		my_tm_stp_hsb = my_info_field & 0x0000FFFF;

	}
	
	// MSB of timestamp in sync pulse or VME extended time stamp
	if( my_info_code == set->GetSyncCode() ) {
		
		// We don't know yet if it's from VME or ISS
		// In VME this would be the extended timestamp
		// In ISS it is the Sync100 pulses
		my_tm_stp_msb = my_info_field & 0x000FFFFF;
		my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb << 28 ) | ( my_tm_stp_lsb & 0x0FFFFFFF );

	}
	
	// MSB of timestamp in ASIC data
	if( my_info_code == set->GetExtItemCode() ) {
		
		// ASIC data has info code 7 for extended timestamp
		my_tm_stp_msb_asic = my_info_field & 0x000FFFFF;
		my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb_asic << 28 ) | ( my_tm_stp_lsb & 0x0FFFFFFF );

	}
	
	// External trigger 1
	if( my_info_code == set->GetExternalTriggerCode() ) {
		
		my_tm_stp_msb = my_info_field & 0x000FFFFF;
		my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb << 28 ) | ( my_tm_stp_lsb & 0x0FFFFFFF );
		hasic_ext[my_mod_id]->Fill( ctr_asic_ext[my_mod_id], my_tm_stp, 1 );
		ctr_asic_ext[my_mod_id]++;

	}

	// External trigger 2
	if( my_info_code == 15 ) {
		
		my_tm_stp_msb = my_info_field & 0x000FFFFF;
		my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb << 28 ) | ( my_tm_stp_lsb & 0x0FFFFFFF );

	}

	// Pause
    if( my_info_code == set->GetPauseCode() ) {
         
		my_tm_stp_msb = my_info_field & 0x000FFFFF;
		my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb << 28 ) | ( my_tm_stp_lsb & 0x0FFFFFFF );
		hasic_pause[my_mod_id]->Fill( ctr_asic_pause[my_mod_id], my_tm_stp, 1 );
		ctr_asic_pause[my_mod_id]++;

    }

	// Resume
	if( my_info_code == set->GetResumeCode() ) {
         
		my_tm_stp_msb = my_info_field & 0x000FFFFF;
		my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb << 28 ) | ( my_tm_stp_lsb & 0x0FFFFFFF );
		hasic_resume[my_mod_id]->Fill( ctr_asic_resume[my_mod_id], my_tm_stp, 1 );
		ctr_asic_resume[my_mod_id]++;

    }

	// Create an info event and fill the tree for external triggers and pause/resume
	if( my_info_code == set->GetExternalTriggerCode() ||
	    my_info_code == set->GetPauseCode() ||
	    my_info_code == set->GetResumeCode() ||
	    my_info_code == 15 ) {

		info_data->SetModule( my_mod_id );
		info_data->SetTimeStamp( my_tm_stp );
		info_data->SetCode( my_info_code );
		data_packet->SetData( info_data );
		if( !flag_source ) output_tree->Fill();
		info_data->Clear();
		data_packet->ClearData();

	}

	return;
	
}

// Common function called to process data in a block from file or DataSpy
bool ISSConverter::ProcessCurrentBlock( int nblock ) {
	
	// Process header.
	ProcessBlockHeader( nblock );

	// Process the main block data until terminator found
	data = (ULong64_t *)(block_data);
	ProcessBlockData( nblock );
			
	// Check once more after going over left overs....
	if( !flag_terminator && flag_asic_data ){

		std::cout << std::endl << __PRETTY_FUNCTION__ << std::endl;
		std::cout << "\tERROR - Terminator sequence not found in data.\n";
		return false;
		
	}
	
	return true;

}

// Function to convert a block of data from DataSpy
int ISSConverter::ConvertBlock( char *input_block, int nblock ) {
	
	// Get the header.
	std::memmove( &block_header, &input_block[0], HEADER_SIZE );
	
	// Get the block
	std::memmove( &block_data, &input_block[HEADER_SIZE], MAIN_SIZE );
	
	// Process the data
	ProcessCurrentBlock( nblock );
	
	// Print time
	//std::cout << "Last time stamp of block = " << my_tm_stp << std::endl;

	return nblock+1;
	
}

// Function to run the conversion for a single file
int ISSConverter::ConvertFile( std::string input_file_name,
							 unsigned long start_block,
							 long end_block ) {
	
	// Read the file.
	std::ifstream input_file( input_file_name, std::ios::in|std::ios::binary );
	if( !input_file.is_open() ){
		
		std::cout << "Cannot open " << input_file_name << std::endl;
		return -1;
		
	}
	
	// Reset counters
	StartFile();

	// Conversion starting
	std::cout << "Converting file: " << input_file_name;
	std::cout << " from block " << start_block << std::endl;
	
	
	// Calculate the size of the file.
	input_file.seekg( 0, input_file.end );
	unsigned long long size_end = input_file.tellg();
	input_file.seekg( 0, input_file.beg );
	unsigned long long size_beg = input_file.tellg();
	unsigned long long FILE_SIZE = size_end - size_beg;
	
	// Calculate the number of blocks in the file.
	unsigned long BLOCKS_NUM = FILE_SIZE / DATA_BLOCK_SIZE;
	
	//a sanity check for file size...
	//QQQ: add more strict test?
	if( FILE_SIZE % DATA_BLOCK_SIZE != 0 ){
		
		std::cout << " *WARNING* " << __PRETTY_FUNCTION__;
		std::cout << "\tMissing data blocks?" << std::endl;

	}
	
	sslogs << "\t File size = " << FILE_SIZE << std::endl;
	sslogs << "\tBlock size = " << DATA_BLOCK_SIZE << std::endl;
	sslogs << "\t  N blocks = " << BLOCKS_NUM << std::endl;

	std::cout << sslogs.str() << std::endl;
	sslogs.str( std::string() ); // clean up
	
	// Data format: http://npg.dl.ac.uk/documents/edoc504/edoc504.html
	// The information is split into 2 words of 32 bits (4 byte).
	// We will collect the data in 64 bit words and split later
	
	
	// Loop over all the blocks.
	for( unsigned long nblock = 0; nblock < BLOCKS_NUM ; nblock++ ){
		
		// Take one block each time and analyze it.
		if( nblock % 200 == 0 || nblock+1 == BLOCKS_NUM ) {
			
			// Percent complete
			float percent = (float)(nblock+1)*100.0/(float)BLOCKS_NUM;
			
			// Progress bar in GUI
			if( _prog_ ) {
				
				prog->SetPosition( percent );
				gSystem->ProcessEvents();
				
			}

			// Progress bar in terminal
			std::cout << " " << std::setw(8) << std::setprecision(4);
			std::cout << percent << "%\r";
			std::cout.flush();

		}

		
		// Get the header.
		input_file.read( (char*)&block_header, HEADER_SIZE );
		// Get the block
		input_file.read( (char*)&block_data, MAIN_SIZE );


		// Check if we are before the start block or after the end block
		if( nblock < start_block || ( (long)nblock > end_block && end_block > 0 ) )
			continue;
		
		
		// Each time we have completed a block, optimise filling
		if( nblock == start_block + 1 )
			output_tree->OptimizeBaskets(30e6);	 // output tree basket size max 30 MB


		// Process current block. If it's the end, stop.
		if( !ProcessCurrentBlock( nblock ) ) break;
		
		
	} // loop - nblock < BLOCKS_NUM
	
	// Close input
	input_file.close();
	
	// Print time
	//std::cout << "Last time stamp in file = " << my_tm_stp << std::endl;
	
	return BLOCKS_NUM;
	
}

unsigned long long ISSConverter::SortTree(){
	
	// Reset the sorted tree so it's empty before we start
	sorted_tree->Reset();
	
	// Load the full tree if possible
	output_tree->SetMaxVirtualSize(2e9); // 2GB
	sorted_tree->SetMaxVirtualSize(2e9); // 2GB
	output_tree->LoadBaskets(1e9); 		 // Load 1 GB of data to memory
	
	// Check we have entries and build time-ordered index
	if( output_tree->GetEntries() ){

		std::cout << "\n Building time-ordered index of events..." << std::endl;
		output_tree->BuildIndex( "data.GetTimeStamp()" );

	}
	else return 0;
	
	// Get index and prepare for sorting
	TTreeIndex *att_index = (TTreeIndex*)output_tree->GetTreeIndex();
	unsigned long long nb_idx = att_index->GetN();
	std::cout << " Sorting: size of the sorted index = " << nb_idx << std::endl;

	// Loop on t_raw entries and fill t
	for( unsigned long i = 0; i < nb_idx; ++i ) {
		
		// Clean up old data
		data_packet->ClearData();
		
		// Get time-ordered event index
		unsigned long long idx = att_index->GetIndex()[i];
		
		// Check if the input or output trees are filling
		if( output_tree->MemoryFull(30e6) )
			output_tree->DropBaskets();
		if( sorted_tree->MemoryFull(30e6) )
			sorted_tree->FlushBaskets();
		
		// Get entry from unsorted tree and fill to sorted tree
		output_tree->GetEntry( idx );
		sorted_tree->Fill();

		// Optimise filling tree
		if( i == 100 ) sorted_tree->OptimizeBaskets(30e6);	 // sorted tree basket size max 30 MB

		// Progress bar
		bool update_progress = false;
		if( nb_idx < 200 )
			update_progress = true;
		else if( i % (nb_idx/100) == 0 || i+1 == nb_idx )
			update_progress = true;
		
		// Print progress
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
	
	// Reset the output tree so it's empty after we've finished
	output_tree->FlushBaskets();
	output_tree->Reset();

	return nb_idx;
	
}
