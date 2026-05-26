#include "CDCalibrator.hh"

MiniballCDCalibrator::MiniballCDCalibrator( std::shared_ptr<MiniballSettings> myset ){

	// First get the settings
	set = myset;
	
	// No calibration file by default
	overwrite_cal = false;
	
	// No input file at the start by default
	flag_input_file = false;
	
	// Progress bar starts as false
	_prog_ = false;

	// Start at MBS event 0
	preveventid = 0;

	// ------------------------------- //
	// Initialise variables and flags  //
	// ------------------------------- //
	build_window = set->GetEventWindow();

	// Intialise the hist list
	histlist = new TList();

}

void MiniballCDCalibrator::StartFile(){

	// Call for every new file
	// Reset counters etc.
	
	time_prev		= 0;
	time_min		= 0;
	time_max		= 0;
	time_first		= 0;

}

void MiniballCDCalibrator::SetInputFile( std::vector<std::string> input_file_names ) {

	/// Overloaded function for a single file or multiple files
	input_tree = new TChain( "mb_sort" );
	mbsinfo_tree = new TChain( "mbsinfo" );
	for( unsigned int i = 0; i < input_file_names.size(); i++ ) {

		input_tree->Add( input_file_names[i].data() );
		mbsinfo_tree->Add( input_file_names[i].data() );

	}

	flag_input_file = true;

	input_tree->SetBranchAddress( "data", &in_data );
	mbsinfo_tree->SetBranchAddress( "mbsinfo", &mbs_info );
	if (mbsinfo_tree->GetEntries()) mbsinfo_tree->BuildIndex("GetEventID()");

	return;

}

void MiniballCDCalibrator::SetInputFile( std::string input_file_name ) {

	// Open next Root input file.
	input_file = new TFile( input_file_name.data(), "read" );
	if( input_file->IsZombie() ) {
		
		std::cout << "Cannot open " << input_file_name << std::endl;
		return;
		
	}
	
	flag_input_file = true;
	
	// Set the input tree
	SetInputTree( (TTree*)input_file->Get("mb_sort") );
	SetMBSInfoTree( (TTree*)input_file->Get("mbsinfo") );
	StartFile();

	return;
	
}

void MiniballCDCalibrator::SetInputTree( TTree *user_tree ){

	// Find the tree and set branch addresses
	input_tree = (TChain*)user_tree;
	in_data = nullptr;
	input_tree->SetBranchAddress( "data", &in_data );

	return;
	
}

void MiniballCDCalibrator::SetMBSInfoTree( TTree *user_tree ){

	// Find the tree and set branch addresses
	mbsinfo_tree = (TChain*)user_tree;
	mbs_info = nullptr;
	mbsinfo_tree->SetBranchAddress( "mbsinfo", &mbs_info );

	return;

}

void MiniballCDCalibrator::SetOutput( std::string output_file_name, bool cWrite ) {

	// ------------------------------------------------------------------------ //
	// Create output file and create events tree
	// ------------------------------------------------------------------------ //
	output_file = new TFile( output_file_name.data(), "recreate" );

	// Hisograms in separate function
	MakeHists();

	// Output the calibration coefficients
	std::string cal_file_name = output_file_name.substr( 0, output_file_name.find_last_of(".") );
	cal_file_name += ".cal";
	output_cal.open( cal_file_name.data(), std::ios::trunc );

	// Write once at the start if in spy
	if( cWrite ) output_file->Write();

}

void MiniballCDCalibrator::Initialise(){

	/// This is called at the end of every execution/loop
	
	flag_close_event = false;
	event_open = false;

	hit_ctr = 0;
	
	std::vector<float>().swap(cd_en_list);
	std::vector<unsigned int>().swap(cd_Q_list);
	std::vector<unsigned long long>().swap(cd_ts_list);
	std::vector<unsigned char>().swap(cd_det_list);
	std::vector<unsigned char>().swap(cd_sec_list);
	std::vector<unsigned char>().swap(cd_side_list);
	std::vector<unsigned char>().swap(cd_strip_list);
	
	return;
	
}


int nextPowerOf2(unsigned int n) {
    if (n == 0) return 1;

    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;

    return n + 1;
}


void MiniballCDCalibrator::MakeHists(){

	std::string hname, htitle;

	// ------------- //
	// CD histograms //
	// ------------- //
	cd_pen_nQ.resize( set->GetNumberOfCDDetectors() );
	cd_nQ_pQ.resize( set->GetNumberOfCDDetectors() );

	// Get sizes and scales
	double maxEn = set->GetCDCalibratorMaxEnergy();
	// read FEBEX gain and offset for reference p strip in first quadrant (febex_1_0_ptag) to get an idea of raw charge range
	double maxRawEn = ( maxEn - cal->FebexOffset(1,0,ptag) ) / cal->FebexGain(1,0,ptag);
	// round that value to the next power of two
	int maxQ = nextPowerOf2(std::round(maxRawEn));
	unsigned int Qbins = 8192;

	if( set->GetNumberOfCaenAdcModules() > 0 ) {
		maxQ = 4096;
		Qbins = 4096;
	}

	else if( set->GetNumberOfFebexSfps() > 1 &&
			set->GetNumberOfFebexBoards() > 0 &&
			set->GetNumberOfFebexChannels() > 0 ) {

		if( cal->FebexType( 1, 0, 0 ) == "Qshort" ) {
			maxQ = 65536;
		}
	}

	for( unsigned int i = 0; i < set->GetNumberOfCDDetectors(); ++i ) {
		
		cd_pen_nQ[i].resize( set->GetNumberOfCDSectors() );
		cd_nQ_pQ[i].resize( set->GetNumberOfCDSectors() );

		for( unsigned int j = 0; j < set->GetNumberOfCDSectors(); ++j ) {

			cd_nQ_pQ[i][j].resize( set->GetNumberOfCDPStrips() );

			for( unsigned int k = 0; k < set->GetNumberOfCDPStrips(); ++k ) {

				hname  = "cd_" + std::to_string(i) + "_" + std::to_string(j);
				hname  += "_nQ_" + std::to_string(ntag) + "_pQ_" + std::to_string(k);
				htitle  = "CD n-side raw charge vs p-side raw charge for detector " + std::to_string(i);
				htitle += ", sector " + std::to_string(j) + ", pid " + std::to_string(k);
				htitle += ", nid " + std::to_string(ntag);
				htitle += ";n-side raw charge (ADC units);p-side raw charge (ADC units);Counts";
				cd_nQ_pQ[i][j][k] = new TH2F( hname.data(), htitle.data(),
											  Qbins, 0, maxQ, Qbins, 0, maxQ );
				histlist->Add(cd_nQ_pQ[i][j][k]);

			} // k

			cd_pen_nQ[i][j].resize( set->GetNumberOfCDNStrips() );

			for( unsigned int k = 0; k < set->GetNumberOfCDNStrips(); ++k ) {

				hname  = "cd_" + std::to_string(i) + "_" + std::to_string(j);
				hname  += "_pen_" + std::to_string(ptag) + "_nQ_" + std::to_string(k);
				htitle  = "CD p-side energy vs n-side raw charge for detector " + std::to_string(i);
				htitle += ", sector " + std::to_string(j) + ", pid " + std::to_string(ptag);
				htitle += ", nid " + std::to_string(k);
				htitle += ";p-side energy (keV);n-side raw charge (ADC units);Counts";
				cd_pen_nQ[i][j][k] = new TH2F( hname.data(), htitle.data(),
											  4000, 0, maxEn, Qbins, 0, maxQ );
				histlist->Add(cd_pen_nQ[i][j][k]);

			} // k

		} // j

	} // i
	
	
	// flag to denote that hists are ready (used for spy)
	hists_ready = true;

	return;
	
}

// Reset histograms in the DataSpy
void MiniballCDCalibrator::ResetHists(){

	// Loop over the hist list
	TIter next( histlist->MakeIterator() );
	while( TObject *obj = next() ) {

		if( obj->InheritsFrom( "TH2" ) )
			( (TH2*)obj )->Reset("ICESM");
		else if( obj->InheritsFrom( "TH1" ) )
			( (TH1*)obj )->Reset("ICESM");

	}

	return;

}

bool MiniballCDCalibrator::FindCDChannels( int det, int sec, int side, int strip, int &adc, int &ch ) {

	// Loop over ADCs
	for( unsigned int m = 0; m < set->GetNumberOfCaenAdcModules(); ++m ) {

		// Loop over channels
		for( unsigned int c = 0; c < set->GetNumberOfCaenAdcChannels(); ++c ) {

			// Check that it's a CD
			if( !set->IsCD(m,c) ) continue;

			// Check we have the correct CD detector
			if( set->GetCDDetector(m,c) != det ) continue;

			// Check we have the correct sector
			if( set->GetCDSector(m,c) != sec ) continue;

			// Check we have an P side (==0)
			if( set->GetCDSide(m,c) != side ) continue;

			// Check we have the correct strip
			if( set->GetCDStrip(m,c) != strip ) continue;

			// Then we got the right channel
			adc = m;
			ch = c;
			return true;

		} // c

	} // m

	std::cerr << "CD strip not found, det=" << det << ", sec=" << sec;
	std::cerr << ", side=" << side << ", strip=" << strip << std::endl;
	return false;

}

bool MiniballCDCalibrator::FindCDChannels( int det, int sec, int side, int strip, int &sfp, int &board, int &ch ) {

	// Loop over SFPs
	for( unsigned int s = 0; s < set->GetNumberOfFebexSfps(); ++s ) {

		// Loop over boards
		for( unsigned int m = 0; m < set->GetNumberOfFebexBoards(); ++m ) {

			// Loop over channels
			for( unsigned int c = 0; c < set->GetNumberOfFebexChannels(); ++c ) {

				// Check that it's a CD
				if( !set->IsCD(s,m,c) ) continue;

				// Check we have the correct CD detector
				if( set->GetCDDetector(s,m,c) != det ) continue;

				// Check we have the correct sector
				if( set->GetCDSector(s,m,c) != sec ) continue;

				// Check we have the right side
				if( set->GetCDSide(s,m,c) != side ) continue;

				// Check we have the correct strip
				if( set->GetCDStrip(s,m,c) != strip ) continue;

				// Then we got the right channel
				sfp = s;
				board = m;
				ch = c;
				return true;

			} // c

		} // m

	} // s

	std::cerr << "CD strip not found, det=" << det << ", sec=" << sec;
	std::cerr << ", side=" << side << ", strip=" << strip << std::endl;
	return false;

}

void MiniballCDCalibrator::CalibratePsides() {

	// Check if we have old or new DAQ
	bool oldDAQ = false;
	if( set->GetNumberOfCaenAdcModules() > 0 )
		oldDAQ = true;

	// Create a TF1 for the linear fit
	auto pfit = std::make_unique<TF1>( "pfit", "[0]+[1]*x", 0, 1e9 );

	// Some canvases to check fits
	gErrorIgnoreLevel = kError;
	std::vector<std::vector<std::unique_ptr<TCanvas>>> canv;
	canv.resize( set->GetNumberOfCDDetectors() );

	// Loop over detectors
	for( unsigned int i = 0; i < set->GetNumberOfCDDetectors(); ++i ) {

		canv[i].resize( set->GetNumberOfCDSectors() );

		// Loop over the sectors
		for( unsigned int j = 0; j < set->GetNumberOfCDSectors(); ++j ) {

			std::string cname = "cdcal_p_" + std::to_string(i) + "_" + std::to_string(j);
			canv[i][j] = std::make_unique<TCanvas>( cname.data(), cname.data(), 800, 1000 );

			// Loop over all the strips
			for( unsigned int k = 0; k < set->GetNumberOfCDPStrips(); ++k ) {

				// Get the right histogram to do the fit
				auto res = cd_nQ_pQ[i][j][k]->Fit( pfit.get(), "QWL" );
				if( res != 0 ) continue;
				double fit_gain = ngain / pfit->GetParameter(1);
				double fit_offset = noffset - pfit->GetParameter(0) * fit_gain;

				// If we have the n-side tag, set the gain and offset
				if( k == ptag ) {
					std::cout << "!! This is the p-side tag channel, cross-check check the parameters below !!" << std::endl;
					pgain = fit_gain;
					poffset = fit_offset;
				}

				// Get the output names for the calibration file
				std::string cal_base;
				std::string modchstr;
				int fsfp, fmod, fch;
				if( oldDAQ ) {

					// Search for the correct ADC and channel combination
					cal_base = "adc_";
					if( !FindCDChannels( i, j, 0, k, fmod, fch ) )
					   continue;
					modchstr = std::to_string(fmod) + "_" + std::to_string(fch);

				} // old DAQ

				else {

					// Search for the correct ADC and channel combination
					cal_base = "febex_";
					if( !FindCDChannels( i, j, 0, k, fsfp, fmod, fch ) )
						continue;
					modchstr  = std::to_string(fsfp) + "_" + std::to_string(fmod);
					modchstr += "_" + std::to_string(fch);

				} // new DAQ

				// Add gain and offset
				std::string gainstr = cal_base + modchstr + ".Gain: " + std::to_string( fit_gain );
				std::string offsetstr = cal_base + modchstr + ".Offset: " + std::to_string( fit_offset );

				// Write them to the file
				std::cout << gainstr << std::endl;
				std::cout << offsetstr << std::endl;
				output_cal << gainstr << std::endl;
				output_cal << offsetstr << std::endl;

				// Print to a file
				std::string pdfname = cname + ".pdf";
				if( k == 0 && set->GetNumberOfCDPStrips() != 1 )
					pdfname += "(";
				else if( k > 0 && k == set->GetNumberOfCDPStrips() - 1 )
					pdfname += ")";
				canv[i][j]->Print( pdfname.data(), "pdf" );

			} // k

		} // j

	} // i

	// Reset warning level
	gErrorIgnoreLevel = kInfo;

	return;

}

void MiniballCDCalibrator::CalibrateNsides() {

	// Check if we have old or new DAQ
	bool oldDAQ = false;
	if( set->GetNumberOfCaenAdcModules() > 0 )
		oldDAQ = true;

	// Create a TF1 for the linear fit
	auto nfit = std::make_unique<TF1>( "nfit", "[0]+[1]*x", 0, 1e9 );

	// Some canvases to check fits
	gErrorIgnoreLevel = kError;
	std::vector<std::vector<std::unique_ptr<TCanvas>>> canv;
	canv.resize( set->GetNumberOfCDDetectors() );

	// Loop over detectors
	for( unsigned int i = 0; i < set->GetNumberOfCDDetectors(); ++i ) {

		canv[i].resize( set->GetNumberOfCDSectors() );

		// Loop over the sectors
		for( unsigned int j = 0; j < set->GetNumberOfCDSectors(); ++j ) {

			std::string cname = "cdcal_n_" + std::to_string(i) + "_" + std::to_string(j);
			canv[i][j] = std::make_unique<TCanvas>( cname.data(), cname.data(), 800, 1000 );

			// Loop over all the strips
			for( unsigned int k = 0; k < set->GetNumberOfCDNStrips(); ++k ) {

				// Get the right histogram to do the fit
				auto res = cd_pen_nQ[i][j][k]->Fit( nfit.get(), "QWL" );
				if( res != 0 ) continue;
				double fit_gain = 1.0 / nfit->GetParameter(1);
				double fit_offset = -1.0 * nfit->GetParameter(0) * fit_gain;
				//double fit_gain = 1.0;
				//double fit_offset = 0.0;

				// If we have the n-side tag, set the gain and offset
				if( k == ntag ) {
					ngain = fit_gain;
					noffset = fit_offset;
				}

				// Get the output names for the calibration file
				std::string cal_base;
				std::string modchstr;
				int fsfp, fmod, fch;
				if( oldDAQ ) {

					// Search for the correct ADC and channel combination
					cal_base = "adc_";
					if( !FindCDChannels( i, j, 1, k, fmod, fch ) )
					   continue;
					modchstr = std::to_string(fmod) + "_" + std::to_string(fch);

				} // old DAQ

				else {
					
					// Search for the correct ADC and channel combination
					cal_base = "febex_";
					if( !FindCDChannels( i, j, 1, k, fsfp, fmod, fch ) )
						continue;
					modchstr  = std::to_string(fsfp) + "_" + std::to_string(fmod);
					modchstr += "_" + std::to_string(fch);
					
				} // new DAQ

				// Add gain and offset
				std::string gainstr = cal_base + modchstr + ".Gain: " + std::to_string( fit_gain );
				std::string offsetstr = cal_base + modchstr + ".Offset: " + std::to_string( fit_offset );

				// Write them to the file
				std::cout << gainstr << std::endl;
				std::cout << offsetstr << std::endl;
				output_cal << gainstr << std::endl;
				output_cal << offsetstr << std::endl;

				// Print to a file
				std::string pdfname = cname + ".pdf";
				if( k == 0 && set->GetNumberOfCDNStrips() != 1 )
					pdfname += "(";
				else if( k > 0 && k == set->GetNumberOfCDNStrips() - 1 )
					pdfname += ")";
				canv[i][j]->Print( pdfname.data(), "pdf" );

			} // k

		} // j

	} // i

	// Reset warning level
	gErrorIgnoreLevel = kInfo;

	return;

}

void MiniballCDCalibrator::FillPixelHists() {

	// Variables for the finder algorithm
	std::vector<unsigned char> pindex;
	std::vector<unsigned char> nindex;

	// Loop over each detector and sector
	for( unsigned int i = 0; i < set->GetNumberOfCDDetectors(); ++i ){

		for( unsigned int j = 0; j < set->GetNumberOfCDSectors(); ++j ){

			// Reset variables for a new detector element
			pindex.clear();
			nindex.clear();
			std::vector<unsigned char>().swap(pindex);
			std::vector<unsigned char>().swap(nindex);

			// Calculate p/n side multiplicities and get indicies
			for( unsigned int k = 0; k < cd_en_list.size(); ++k ){

				// Test that we have the correct detector and quadrant
				if( i != cd_det_list.at(k) || j != cd_sec_list.at(k) )
					continue;

				// Check max energy and push back the multiplicity
				if( cd_side_list.at(k) == 0 )
					pindex.push_back(k);

				else if( cd_side_list.at(k) == 1 )
					nindex.push_back(k);

			} // k: all CD events

			// Keep only multiplicity 1v1 events
			if( pindex.size() != 1 || nindex.size() != 1 )
				continue;

			// Fill the hit in the right pixel
			int pid = cd_strip_list[pindex[0]];
			int nid = cd_strip_list[nindex[0]];
			double pen = cd_en_list[pindex[0]];
			double nen = cd_en_list[nindex[0]];
			unsigned int pQ = cd_Q_list[pindex[0]];
			unsigned int nQ = cd_Q_list[nindex[0]];

			// skip events with very diiferent energies
			if( nQ / pQ > 1.5 || pQ / nQ > 1.5 ) continue;

			// For p-side tags
			if( pid == ptag ) {

				cd_pen_nQ[i][j][nid]->Fill( pen, nQ );
				
			}
			
			// For n-side tags
			if( nid == ntag ) {

				cd_nQ_pQ[i][j][pid]->Fill( nQ, pQ );

			}

		} // j

	} // i


}

unsigned long MiniballCDCalibrator::FillHists() {

	/// Function to loop over the sort tree and build array and recoil events

	if( input_tree->LoadTree(0) < 0 ){
		
		std::cout << " CD Calibrator: nothing to do" << std::endl;
		return 0;
		
	}
	
	// Get ready and go
	Initialise();
	n_entries = input_tree->GetEntries();
	n_mbs_entries = mbsinfo_tree->GetEntries();

	std::cout << " CD Calibrator: number of entries in input tree = ";
	std::cout << n_entries << std::endl;

	std::cout << "\tnumber of MBS Events/triggers in input tree = ";
	std::cout << n_mbs_entries << std::endl;
	
	// ------------------------------------------------------------------------ //
	// Main loop over TTree to find events
	// ------------------------------------------------------------------------ //
	for( unsigned long i = 0; i < n_entries; ++i ) {
		
		// First event, yes please!
		if( i == 0 ){

			input_tree->GetEntry(i);
			myeventid = in_data->GetEventID();
			myeventtime = in_data->GetTime();

			// Try to get the MBS info event with the index
			if( mbsinfo_tree->GetEntryWithIndex( myeventid ) < 0 &&
			    n_mbs_entries > 0 ) {

				// Look for the matches MBS Info event if we didn't match automatically
				for( unsigned long j = 0; j < n_mbs_entries; ++j ){

					mbsinfo_tree->GetEntry(j);
					if( mbs_info->GetEventID() == myeventid ) {
						myeventtime = mbs_info->GetTime();
						break;
					}

					// Panic if we failed!
					if( j+1 == n_mbs_entries ) {
						std::cerr << "Didn't find matching MBS Event IDs at start of the file: ";
						std::cerr << myeventid << std::endl;
					}

				}

			}

			//std::cout << "MBS Trigger time = " << myeventtime << std::endl;

		}

		// Get the time of the event
		if( set->GetMbsEventMode() ) {
		
			myhittime = in_data->GetTime();
			mytime = myeventtime + myhittime;
			
		}
		
		else mytime = in_data->GetTime();
		
		// check time stamp monotonically increases in time-ordered mode!
		if( time_prev > mytime && !set->GetMbsEventMode() ) {
			
			std::cout << "Out of order event in ";
			std::cout << input_tree->GetName() << std::endl;
			
		}
			
		// check event id is increasing in MBS event ordered mode
		if( preveventid > myeventid && set->GetMbsEventMode() ) {

			std::cout << "Out of order MBS event " << myeventid;
			std::cout << " < " << preveventid << std::endl;

		}

		// record time of this event
		time_prev = mytime;
		
		// assume this is above threshold initially
		mythres = true;

		// ------------------------------------------ //
		// Find FEBEX data
		// ------------------------------------------ //
		if( in_data->IsFebex() ) {
			
			// Get the data
			febex_data = in_data->GetFebexData();
			mysfp = febex_data->GetSfp();
			myboard = febex_data->GetBoard();
			mych = febex_data->GetChannel();
			mypileup = febex_data->IsPileup();
			myclipped = febex_data->IsClipped();

			// Update calibration always for CD calibrator
			unsigned int adc_tmp_value;
			if( cal->FebexType( mysfp, myboard, mych ) == "Qshort" )
				adc_tmp_value = febex_data->GetQshort();
			else if( cal->FebexType( mysfp, myboard, mych ) == "Qint" )
				adc_tmp_value = febex_data->GetQint();
			else adc_tmp_value = febex_data->GetQshort();

			myenergy = cal->FebexEnergy( mysfp, myboard, mych, adc_tmp_value );

			if( adc_tmp_value > cal->FebexThreshold( mysfp, myboard, mych ) )
				mythres = true;
			else mythres = false;

			// Is it a particle from the CD?
			if( set->IsCD( mysfp, myboard, mych ) && mythres ) {

				// Increment counts and open the event
				hit_ctr++;

				// Clipped rejection and pileup rejection
				if( ( !myclipped || !set->GetClippedRejection() ) &&
				   ( !mypileup || !set->GetPileupRejection() ) ) {

					event_open = true;
					cd_en_list.push_back( myenergy );
					cd_Q_list.push_back( adc_tmp_value );
					cd_ts_list.push_back( mytime );
					cd_det_list.push_back( set->GetCDDetector( mysfp, myboard, mych ) );
					cd_sec_list.push_back( set->GetCDSector( mysfp, myboard, mych ) );
					cd_side_list.push_back( set->GetCDSide( mysfp, myboard, mych ) );
					cd_strip_list.push_back( set->GetCDStrip( mysfp, myboard, mych ) );

				}

			}

		}
		
		// ------------------------------------------ //
		// Find ADC data
		// ------------------------------------------ //
		if( in_data->IsAdc() ) {
			
			// Get the data
			adc_data = in_data->GetAdcData();
			myadc = adc_data->GetModule();
			mych = adc_data->GetChannel();
			myclipped = adc_data->IsClipped();

			// Update calibration always for CD calibrator
			unsigned int adc_tmp_value = adc_data->GetQshort();
			myenergy = cal->AdcEnergy( myadc, mych, adc_tmp_value );

			if( adc_tmp_value > cal->AdcThreshold( myadc, mych ) )
				mythres = true;
			else mythres = false;

			// Is it a particle from the CD?
			if( set->IsCD( myadc, mych ) && mythres ) {
				
				// Increment counts and open the event
				hit_ctr++;
				
				if( !myclipped || !set->GetClippedRejection() ) {

					event_open = true;
					cd_en_list.push_back( myenergy );
					cd_Q_list.push_back( adc_tmp_value );
					cd_ts_list.push_back( mytime );
					cd_det_list.push_back( set->GetCDDetector( myadc, mych ) );
					cd_sec_list.push_back( set->GetCDSector( myadc, mych ) );
					cd_side_list.push_back( set->GetCDSide( myadc, mych ) );
					cd_strip_list.push_back( set->GetCDStrip( myadc, mych ) );
					
				}
				
			}
			
		}

		// Sort out the timing for the event window
		// but only if it isn't an info event, i.e only for real data
		if( !in_data->IsInfo() ) {
			
			// if this is first datum included in Event
			if( hit_ctr == 1 && mythres ) {
				
				time_min	= mytime;
				time_max	= mytime;
				time_first	= mytime;
				
			}
			
			// Update min and max
			if( mytime > time_max ) time_max = mytime;
			else if( mytime < time_min ) time_min = mytime;
			
		} // not info data

		//------------------------------
		//  check if last datum from this event and do some cleanup
		//------------------------------
		
		if( input_tree->GetEntry(i+1) ) {
			
			// Get the next MBS event ID
			preveventid = myeventid;
			myeventid = in_data->GetEventID();

			// If the next MBS event ID is the same, carry on
			// If not, we have to go look for the next trigger time
			if( myeventid != preveventid ) {

				// Close the event
				flag_close_event = true;

				// And find the next MBS event ID
				if( mbsinfo_tree->GetEntryWithIndex( myeventid ) < 0 &&
				    n_mbs_entries > 0 ) {

					std::cerr << "MBS Event " << myeventid << " not found by index, looking up manually" << std::endl;

					// Look for the matches MBS Info event if we didn't match automatically
					for( unsigned long j = 0; j < n_mbs_entries; ++j ){

						mbsinfo_tree->GetEntry(j);
						if( mbs_info->GetEventID() == myeventid ) {
							myeventtime = mbs_info->GetTime();
							break;
						}

						// Panic if we failed!
						if( j+1 == n_mbs_entries ) {
							std::cerr << "Didn't find matching MBS Event IDs at start of the file: ";
							std::cerr << myeventid << std::endl;
						}
					}

				}

				else myeventtime = mbs_info->GetTime();

			}

			// BELOW IS THE TIME-ORDERED METHOD!

			// Get the time of the next event
			if( set->GetMbsEventMode() ) {
			
				myhittime = in_data->GetTime();
				mytime = myeventtime + myhittime;
				
			}
			
			else mytime = in_data->GetTime();

			// Calculate time diff
			time_diff = mytime - time_first;

			// window = time_stamp_first + time_window
			if( time_diff > build_window )
				flag_close_event = true; // set flag to close this event

			// we've gone on to the next file in the chain
			else if( time_diff < 0 )
				flag_close_event = true; // set flag to close this event
				
		} // if next entry beyond time window: close event!
		
		
		//----------------------------
		// if close this event or last entry
		//----------------------------
		if( flag_close_event || (i+1) == n_entries ) {

			//--------------------------------------------------
			// clear values of arrays to store intermediate info
			//--------------------------------------------------
			FillPixelHists();
			Initialise();
			
		} // if close event && hit_ctr > 0
		
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
		
	} // End of main loop over TTree to process raw FEBEX data entries (for n_entries)


	//--------------------------
	// Do the fitting to get calibration coefficients
	//--------------------------

	std::cout << "\n\nUsing p-side strip " << (int)ptag << " as reference for calibrating n-sides" << std::endl;
	CalibrateNsides();
	std::cout << "\n\nUsing n-side strip " << (int)ntag << " as reference for calibrating p-sides" << std::endl;
	CalibratePsides();

	//--------------------------
	// Clean up
	//--------------------------

	std::cout << "\n MiniballCDCalibrator finished..." << std::endl;

	return n_entries;
	
}
