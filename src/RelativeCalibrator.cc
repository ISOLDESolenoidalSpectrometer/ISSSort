#include "RelativeCalibrator.hh"

ISSRelativeCalibrator::ISSRelativeCalibrator(){

	// No settings file by default
	overwrite_set = false;

	// No calibration file by default
	overwrite_cal = false;

	// No input file at the start by default
	flag_input_file = false;

	// No progress bar by default
	_prog_ = false;

	// Intialise the hist list
	histlist = new TList();

}

void ISSRelativeCalibrator::StartFile(){

	// ------------------------------------------------------------------------ //
	// Initialise variables and flags
	// ------------------------------------------------------------------------ //
	build_window = set->GetEventWindow();

	// Reset counters etc.
	time_prev		= 0;
	time_min		= 0;
	time_max		= 0;
	time_first		= 0;

}

void ISSRelativeCalibrator::SetInputFile( std::vector<std::string> input_file_names ) {

	/// Overloaded function for a single file or multiple files
	input_tree = new TChain( "iss_sort" );
	for( unsigned int i = 0; i < input_file_names.size(); i++ ) {

		input_tree->Add( input_file_names[i].data() );

	}

	flag_input_file = true;

	input_tree->SetBranchAddress( "data", &in_data );
	StartFile();

	return;

}

void ISSRelativeCalibrator::SetInputFile( std::string input_file_name ) {

	// Open next Root input file.
	input_file = new TFile( input_file_name.data(), "read" );
	if( input_file->IsZombie() ) {
		
		std::cout << "Cannot open " << input_file_name << std::endl;
		return;
		
	}
	
	flag_input_file = true;
	
	// Set the input tree
	SetInputTree( (TTree*)input_file->Get("iss_sort") );

	return;
	
}

void ISSRelativeCalibrator::SetInputTree( TTree *user_tree ){

	// Find the tree and set branch addresses
	input_tree = (TChain*)user_tree;
	input_tree->SetBranchAddress( "data", &in_data );
	StartFile();

	return;
	
}

void ISSRelativeCalibrator::SetOutput( std::string output_file_name, bool cWrite ) {

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

void ISSRelativeCalibrator::Initialise(){

	/// This is called at the end of every execution/loop
	
	flag_close_event = false;
	event_open = false;

	hit_ctr = 0;
	
	// Swap all vectors with empty vectors
	std::vector<float>().swap(pQ_list);
	std::vector<float>().swap(nQ_list);
	std::vector<double>().swap(ptd_list);
	std::vector<double>().swap(ntd_list);
	std::vector<double>().swap(pwalk_list);
	std::vector<double>().swap(nwalk_list);
	std::vector<char>().swap(pid_list);
	std::vector<char>().swap(nid_list);
	std::vector<char>().swap(pmod_list);
	std::vector<char>().swap(nmod_list);
	std::vector<char>().swap(prow_list);
	std::vector<char>().swap(nrow_list);
	std::vector<bool>().swap(phit_list);
	std::vector<bool>().swap(nhit_list);

	return;
	
}


void ISSRelativeCalibrator::MakeHists(){

	std::string hname, htitle;

	// ------------- //
	// CD histograms //
	// ------------- //
	array_pQ_nQ.resize( set->GetNumberOfArrayModules() );
	array_nQ_pQ.resize( set->GetNumberOfArrayModules() );

	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {

		array_pQ_nQ[i].resize( set->GetNumberOfArrayRows() );
		array_nQ_pQ[i].resize( set->GetNumberOfArrayRows() );

		for( unsigned int j = 0; j < set->GetNumberOfArrayRows(); ++j ) {

			array_pQ_nQ[i][j].resize( set->GetNumberOfArrayPstrips() );

			for( unsigned int k = 0; k < set->GetNumberOfArrayPstrips(); ++k ) {

				hname  = "array_" + std::to_string(i) + "_" + std::to_string(j);
				hname  += "_pQ_" + std::to_string(ptag) + "_nQ_" + std::to_string(k);
				htitle  = "CD p-side ADC value vs n-side ADC value for module " + std::to_string(i);
				htitle += ", row " + std::to_string(j) + ", pid " + std::to_string(ptag);
				htitle += ", nid " + std::to_string(k);
				htitle += ";p-side (ADC units);n-side (ADC units);Counts";
				array_pQ_nQ[i][j][k] = new TH2S( hname.data(), htitle.data(),
												1024, 0, 4096, 1024, 0, 4096 );
				histlist->Add(array_pQ_nQ[i][j][k]);

			} // k

			array_nQ_pQ[i][j].resize( set->GetNumberOfArrayNstrips()*2 );

			for( unsigned int k = 0; k < set->GetNumberOfArrayNstrips()*2; ++k ) {

				hname  = "array_" + std::to_string(i) + "_" + std::to_string(j);
				hname  += "_nQ_" + std::to_string(ntag) + "_pQ_" + std::to_string(k);
				htitle  = "CD n-side ADC value vs p-side ADC value for module " + std::to_string(i);
				htitle += ", row " + std::to_string(j) + ", pid " + std::to_string(k);
				htitle += ", nid " + std::to_string(ntag);
				htitle += ";n-side (ADC units);p-side (ADC units);Counts";
				array_nQ_pQ[i][j][k] = new TH2S( hname.data(), htitle.data(),
												1024, 0, 4096, 1024, 0, 4096 );
				histlist->Add(array_nQ_pQ[i][j][k]);

			} // k

		} // j

	} // i
	
	
	// flag to denote that hists are ready (used for spy)
	hists_ready = true;

	return;
	
}

// Reset histograms in the DataSpy
void ISSRelativeCalibrator::ResetHists(){

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

void ISSRelativeCalibrator::CalibratePsides() {

	// Create a TF1 for the linear fit
	auto pfit = std::make_unique<TF1>( "pfit", "[0]+[1]*x", 0, 1e9 );

	// Some canvases to check fits
	gErrorIgnoreLevel = kError;
	std::vector<std::vector<std::unique_ptr<TCanvas>>> canv;
	canv.resize( set->GetNumberOfArrayModules() );

	// Loop over detectors
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {

		canv[i].resize( set->GetNumberOfArrayRows() );

		// Loop over the sectors
		for( unsigned int j = 0; j < set->GetNumberOfArrayRows(); ++j ) {

			std::string cname = "cal_p_" + std::to_string(i) + "_" + std::to_string(j);
			canv[i][j] = std::make_unique<TCanvas>( cname.data(), cname.data(), 800, 1000 );

			// Loop over all the strips
			for( unsigned int k = 0; k < set->GetNumberOfArrayPstrips(); ++k ) {

				// Get the right histogram to do the fit
				auto res = array_pQ_nQ[i][j][k]->Fit( pfit.get(), "QWL" );
				if( res != 0 ) continue;
				double fit_gain = ngain / pfit->GetParameter(1);
				double fit_offset = noffset - pfit->GetParameter(0) * fit_gain;

				// If we have the n-side tag, set the gain and offset
				if( k == ptag ) {
					std::cout << "!! This is the p-side tag channel, cross-check check the parameters below !!" << std::endl;
				}

				// Get the output names for the calibration file
				std::string cal_base = "asic_";
				std::string modchstr;

				// Search for the correct ADC and channel combination
				int fmod  = i;
				int fasic = set->GetArrayAsic(i,j,0,k);
				int fch   = set->GetArrayChannel(i,j,0,k);
				if( fasic < 0 || fch < 0 ) continue;
				modchstr  = std::to_string(fmod) + "_";
				modchstr += std::to_string(fasic) + "_";
				modchstr += std::to_string(fch);

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
				if( k == 0 && set->GetNumberOfArrayPstrips() != 1 )
					pdfname += "(";
				else if( k > 0 && k == set->GetNumberOfArrayPstrips() - 1 )
					pdfname += ")";
				canv[i][j]->Print( pdfname.data(), "pdf" );

			} // k

		} // j

	} // i

	// Reset warning level
	gErrorIgnoreLevel = kInfo;

	return;

}

void ISSRelativeCalibrator::CalibrateNsides() {

	// Create a TF1 for the linear fit
	auto nfit = std::make_unique<TF1>( "nfit", "[0]+[1]*x", 0, 1e9 );

	// Some canvases to check fits
	gErrorIgnoreLevel = kError;
	std::vector<std::vector<std::unique_ptr<TCanvas>>> canv;
	canv.resize( set->GetNumberOfArrayModules() );

	// Loop over detectors
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {

		canv[i].resize( set->GetNumberOfArrayRows() );

		// Loop over the sectors
		for( unsigned int j = 0; j < set->GetNumberOfArrayRows(); ++j ) {

			std::string cname = "cal_n_" + std::to_string(i) + "_" + std::to_string(j);
			canv[i][j] = std::make_unique<TCanvas>( cname.data(), cname.data(), 800, 1000 );

			// Get the p-side gain and offset
			pgain = cal->GetAsicGains().at(i).at(j).at(ptag);
			poffset = cal->GetAsicOffsets().at(i).at(j).at(ptag);

			// Loop over all the strips
			for( unsigned int k = 0; k < set->GetNumberOfArrayNstrips()*2; ++k ) {

				// Get the right histogram to do the fit
				auto res = array_nQ_pQ[i][j][k]->Fit( nfit.get(), "QWL" );
				if( res != 0 ) continue;
				double fit_gain = pgain / nfit->GetParameter(1);
				double fit_offset = poffset - nfit->GetParameter(0) * fit_gain;

				// If we have the n-side tag, set the gain and offset
				if( k == ntag ) {
					ngain = fit_gain;
					noffset = fit_offset;
				}

				// Get the output names for the calibration file
				std::string cal_base = "asic_";
				std::string modchstr;

				// Search for the correct ADC and channel combination
				int fmod  = i;
				int fasic = set->GetArrayAsic(i,j,1,k);
				int fch   = set->GetArrayChannel(i,j,1,k);
				if( fasic < 0 || fch < 0 ) continue;
				modchstr  = std::to_string(fmod) + "_";
				modchstr += std::to_string(fasic) + "_";
				modchstr += std::to_string(fch);


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
				if( k == 0 && set->GetNumberOfArrayNstrips() != 1 )
					pdfname += "(";
				else if( k > 0 && k == set->GetNumberOfArrayNstrips() - 1 )
					pdfname += ")";
				canv[i][j]->Print( pdfname.data(), "pdf" );

			} // k

		} // j

	} // i

	// Reset warning level
	gErrorIgnoreLevel = kInfo;

	return;

}

void ISSRelativeCalibrator::FillPixelHists() {

	// Do each module and row individually
	for( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ) {

		for( unsigned int j = 0; j < set->GetNumberOfArrayRows(); ++j ) {

			// Variables for the finder algorithm
			std::vector<unsigned char> pindex;
			std::vector<unsigned char> nindex;

			// Loop over p-side events
			for( unsigned int k = 0; k < pQ_list.size(); ++k ) {

				// Check if it is the module and row we want
				if( pmod_list.at(k) == (int)i && prow_list.at(k) == (int)j ) {

					// Put in the index
					pindex.push_back( k );

				}

			}

			// Loop over n-side events
			for( unsigned int l = 0; l < nQ_list.size(); ++l ) {

				// Check if it is the module and row we want
				if( nmod_list.at(l) == (int)i && nrow_list.at(l) == (int)j ) {

					// Put in the index
					nindex.push_back( l );

				}

			}

			// Keep only multiplicity 1v1 events
			if( pindex.size() != 1 || nindex.size() != 1 )
				continue;

			// Fill the hit in the right pixel
			int pid = pid_list[pindex[0]];
			int nid = nid_list[nindex[0]];
			unsigned int pQ = pQ_list[pindex[0]];
			unsigned int nQ = nQ_list[nindex[0]];

			// skip events with very diiferent energies
			//if( nQ / pQ > 1.5 || pQ / nQ > 1.5 ) continue;

			// For p-side tags
			if( pid == ptag ) {

				array_nQ_pQ[i][j][nid]->Fill( nQ, pQ );

			}
			
			// For n-side tags
			if( nid == ntag ) {

				array_pQ_nQ[i][j][pid]->Fill( pQ, nQ );

			}

		} // j

	} // i


}

unsigned long ISSRelativeCalibrator::FillHists() {

	/// Function to loop over the sort tree and build array and recoil events

	if( input_tree->LoadTree(0) < 0 ){
		
		std::cout << " CD Calibrator: nothing to do" << std::endl;
		return 0;
		
	}
	
	// Get ready and go
	Initialise();
	n_entries = input_tree->GetEntries();

	std::cout << " Relative Array Calibrator: number of entries in input tree = ";
	std::cout << n_entries << std::endl;

	// Get the index needed for time-ordering
	TTreeIndex *att_index = nullptr;

	// Event building by timestamp only
	if( set->BuildByTimeStamp() ) {
		std::cout << " Event Building: using raw timestamp for event ordering" << std::endl;
		//input_tree->BuildIndex( "GetTimeStamp()" );
	}

	// Or apply time-walk correction, i.e. get new time ordering
	else {
		std::cout << " Event Building: applying time walk-correction to event ordering" << std::endl;
		input_tree->BuildIndex( "GetTimeWithWalk()" );
		att_index = (TTreeIndex*)input_tree->GetTreeIndex();
	}


	// ------------------------------------------------------------------------ //
	// Main loop over TTree to find events
	// ------------------------------------------------------------------------ //
	for( unsigned long i = 0; i < n_entries; ++i ) {
		
		// Get time-ordered event index (with or without walk correction)
		unsigned long long idx = i;
		if( !set->BuildByTimeStamp() )
			idx = att_index->GetIndex()[i];

		// Current event data
		//if( input_tree->MemoryFull(30e6) )
		//	input_tree->DropBaskets();

		if( i == 0 ) input_tree->GetEntry(idx);

		// Get the time of the event (with or without walk correction)
		if( set->BuildByTimeStamp() ) mytime = in_data->GetTime(); // no correction
		else mytime = in_data->GetTimeWithWalk(); // with correction

		//std::cout << std::setprecision(15) << i << "\t";
		//std::cout << in_data->GetTimeStamp() << "\t" << mytime << std::endl;

		// check time stamp monotonically increases!
		// but allow for the fine time of the CAEN system
		if( (unsigned long long)time_prev > mytime + 5.0 ) {

			std::cout << "Out of order event in file ";
			std::cout << input_tree->GetName() << std::endl;

		}

		// record time of this event
		time_prev = mytime;

		// assume this is above threshold initially
		mythres = true;


		// ------------------------------------------ //
		// Find particles on the array
		// ------------------------------------------ //
		if( in_data->IsAsic() ) {

			asic_data = in_data->GetAsicData();
			mymod = asic_data->GetModule();
			mych = asic_data->GetChannel();
			myasic = asic_data->GetAsic();
			myside = set->GetArraySide( mymod, myasic );
			myrow = set->GetArrayRow( mymod, myasic, mych );
			myhitbit = asic_data->GetHitBit();
			myenergy = cal->AsicEnergy( mymod, myasic,
										mych, asic_data->GetAdcValue() );
			mywalk = cal->AsicWalk( mymod, myasic, myenergy, myhitbit );
			if( asic_data->GetAdcValue() < cal->AsicThreshold( mymod, myasic, mych ) )
				mythres = false;


			// p-side event
			if( myside == 0 && mythres ) {

				mystrip = set->GetArrayStrip( mymod, myasic, mych );

				// Only use if it is an event from a detector
				if( mystrip >= 0 ) {

					pQ_list.push_back( asic_data->GetAdcValue() );
					ptd_list.push_back( mytime );
					if( set->BuildByTimeStamp() )
						pwalk_list.push_back( mytime + mywalk );
					else
						pwalk_list.push_back( mytime );
					pmod_list.push_back( mymod );
					pid_list.push_back( mystrip );
					prow_list.push_back( myrow );
					phit_list.push_back( myhitbit );

					event_open = true; // real data open events (above threshold and from a strip)
					hit_ctr++; // increase counter for bits of data included in this event

				}

			}

			// n-side event
			else if( myside == 1 && mythres ) {

				mystrip = set->GetArrayStrip( mymod, myasic, mych );

				// Only use if it is an event from a detector
				if( mystrip >= 0 ) {

					nQ_list.push_back( asic_data->GetAdcValue() );
					ntd_list.push_back( mytime );
					if( set->BuildByTimeStamp() )
						nwalk_list.push_back( mytime + mywalk );
					else
						nwalk_list.push_back( mytime );
					nmod_list.push_back( mymod );
					nid_list.push_back( mystrip );
					nrow_list.push_back( myrow );
					nhit_list.push_back( myhitbit );

					event_open = true; // // real data open events (above threshold and from a strip)
					hit_ctr++; // increase counter for bits of data included in this event

				}

			}

		}

		// Sort out the timing for the event window
		// but only if it isn't an info event, i.e only for real data
		if ( !in_data->IsInfo() ){

			// if this is first datum included in Event
			if( hit_ctr == 1 && mythres ) {

				time_min	= mytime;
				time_max	= mytime;
				time_first	= mytime;

			}

			// Update max time
			if( mytime > time_max ) time_max = mytime;
			else if( mytime < time_min ) time_min = mytime;

		} // not info data


		//------------------------------
		//  check if last datum from this event and do some cleanup
		//------------------------------
		unsigned long long idx_next;
		if( i+1 == n_entries ) idx_next = n_entries;
		else {
			idx_next = i+1;
			if( !set->BuildByTimeStamp() )
				idx_next = att_index->GetIndex()[i+1];
		}

		if( input_tree->GetEntry(idx_next) ) {

			// Time difference to next event (with or without time walk correction)
			if( set->BuildByTimeStamp() )
				time_diff = in_data->GetTime() - time_first; // no correction
			else
				time_diff = in_data->GetTimeWithWalk() - time_first; // with correction

			// window = time_stamp_first + time_window
			if( time_diff > build_window )
				flag_close_event = true; // set flag to close this event

			// we've gone on to the next file in the chain
			else if( time_diff < 0 )
				flag_close_event = true; // set flag to close this event

		}

		//----------------------------
		// if close this event or last entry
		//----------------------------
		if( flag_close_event || (i+1) == n_entries ) {

			//--------------------------------------------------
			// clear values of arrays to store intermediate info
			//--------------------------------------------------
			FillPixelHists();
			Initialise();

		} // if close event

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


	} // End of main loop over TTree to process raw MIDAS data entries (for n_entries)



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

	std::cout << "\n RelativeCalibrator finished..." << std::endl;

	return n_entries;
	
}
