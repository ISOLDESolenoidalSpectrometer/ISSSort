#include "AutoCalibrator.hh"

ISSAutoCalibrator::ISSAutoCalibrator( ISSSettings *myset, ISSReaction *myreact ){
	
	// First get the settings and reaction
	set = myset;
	react = myreact;
	
	// Create the directory for the plots
	gSystem->Exec( "mkdir -p autocal" );
	
	// Set some global defaults
	//ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Fumili");
	
}

int ISSAutoCalibrator::SetInputFile( std::string input_file_name ) {
	
	// Open next Root input file.
	input_file = new TFile( input_file_name.data(), "read" );
	if( input_file->IsZombie() ) {
		
		std::cout << "Cannot open " << input_file_name << std::endl;
		return 1;
		
	}

	return 0;
	
}

void ISSAutoCalibrator::SaveCalFile( std::string name_results_file ){
	
	// Output
	std::ofstream cal_file;
	cal_file.open( name_results_file );
	if( cal_file.is_open() ) {

		cal->PrintCalibration( cal_file, "ae" );
		cal_file.close();

	}

	else {

		std::cerr << "Couldn't open " << name_results_file;
		std::cerr << std::endl;
		
	}
		
}

// --------------------------------------------------------------------------------------------- //
void ISSAutoCalibrator::FindPeaks( TH1F *h, std::vector<float> &centroids ){
	
	std::vector<std::vector<int>> peak_info;	// Index corresponds to individual peaks
	std::vector<int> individual_peak;			// First entry is the height, second entry is the channel
	individual_peak.resize(2);
	
	// Reset peak estimates
	individual_peak[0] = 0;
	individual_peak[1] = 0;
	
	peak_info.resize(0);
	
	// Get info about the histogram -> find the maximum value in the desired region
	int max = 0;
	for ( int j = 0; j < h->GetNbinsX(); j++ ){
		if ( h->GetBinLowEdge(j) > gPeakChannelLowerThreshold ){
			if ( h->GetBinContent(j) > max ){
				max = h->GetBinContent(j);
			}
		}
	}
	
	int threshold = max*gPeakHeightThresholdFraction;	// Count threshold to stop accepting small peaks
	bool b_record_peaks = 1;		// Stores whether a peak has dipped enough to count as a peak
	int current_peak_height = 0;	// Stores the current height of a peak
	
	// Loop over each bin
	for ( int j = 0; j < h->GetNbinsX(); j++ ){
		// Check to see if the histogram has "dipped" enough to define a peak ( or is now much larger...)
		if ( b_record_peaks == 0 ){
			if ( h->GetBinContent(j) < gPeakHeightDipFraction*current_peak_height || h->GetBinContent(j) > current_peak_height ){
				b_record_peaks = 1;
			}
		}
	
		// Check to see if the current bin is over the required thresholds
		if ( b_record_peaks == 1 && h->GetBinContent(j) > threshold && h->GetBinLowEdge(j) > gPeakChannelLowerThreshold ){
			// Now check if bins either side <= current bin content
			// (shouldn't have peak on edge of spectrum, but may need to code that in later...)
			if ( h->GetBinContent(j-1) <= h->GetBinContent(j) && h->GetBinContent(j+1) <= h->GetBinContent(j) ){
				// Store those that pass this process
				individual_peak[0] = h->GetBinContent(j);
				individual_peak[1] = h->GetBinCenter(j);
				peak_info.push_back( individual_peak );
				
				// Store most recent peak information
				b_record_peaks = 0;
				current_peak_height = h->GetBinContent(j);
			}
		}
	}
	
	// Now deal with all the competitors -> set bad ones to zero
	for ( unsigned int j = 1; j < peak_info.size(); j++ ){
		for ( unsigned int k = 0; k < j; k++ ){
			// First, test that both of the test peaks have positive heights
			if ( peak_info[j][0] > 0 && peak_info[k][0] > 0 ){
				// Second, test that the peaks are within range of each other
				if ( TMath::Abs( peak_info[j][1] - peak_info[k][1] ) < 0.5*gAlphaPeakWidthEstimate ){
					// Third, set the smaller one to have negative height (of its measured height for debugging)
					if ( peak_info[j][0] > peak_info[k][0] ){
						peak_info[k][0] = -peak_info[k][0];
					}
					else{
						peak_info[j][0] = -peak_info[j][0];
					}
				}
			}
		}
	}
	
	// Delete negative entries
	for ( unsigned int j = 0; j < peak_info.size(); j++ ){
		if ( peak_info[j][0] < 0 ){
			peak_info.erase( peak_info.begin() + j );
			j--;
		}
	}
	
	// CUSTOM TWEAKING - EDIT AS NECESSARY
	// If 5 peaks detected, if the first peak is smaller than the second, then delete the first
	if ( peak_info.size() == gNumberOfAlphaPeaks+1 ){
		if ( peak_info[0][0] < peak_info[1][0] ){
			peak_info.erase( peak_info.begin() );
		}
	}
	
	// Store the peak info into the alpha spectrum
	if( peak_info.size() ==  gNumberOfAlphaPeaks ) {

		for ( unsigned int j = 0; j < peak_info.size(); j++ )
			centroids[j] = peak_info[j][1];
	
	}
	
	return;
	
}

bool ISSAutoCalibrator::FitSpectrum( TH1F *h, std::vector<float> &centroids, std::vector<float> &errors ){

	// Define array to store fit parameters
	const int npars = 2*gNumberOfAlphaPeaks + 2;
	double par[npars];
	
	// Define total fit
	TF1 *total = new TF1( "totalfit", MultiAlphaGaussianBG, gPeakChannelLowerThreshold, gPeakChannelUpperThreshold, npars );

	// Format total fit
	//total->SetNpx(600);
	total->SetLineColor(kBlack);
	total->SetLineWidth(2);
	
	// Define individual fits
	TF1 *indie_peaks[gNumberOfAlphaPeaks];
	
	// Set bg limits
	total->SetParameter( 0, 0.1 );	// Flat background
	total->SetParLimits( 0, 0, 100 );	// Flat background
	total->SetParName( 0, "Flat bg" );

	// Gaussian width
	total->SetParameter( 1, 2.5 );
	total->SetParLimits( 1, 0.2, 10. );	// Sigma
	total->SetParName( 1, "Sigma" );

	// Set up individual fits and set parameter limits
	for( int i = 0; i < gNumberOfAlphaPeaks; i++ ){
		
		// Individual peaks
		indie_peaks[i] = new TF1( Form( "ind%i", i ), AlphaGaussian,
								 centroids[i]-gAlphaPeakWidthEstimate,
								 centroids[i]+gAlphaPeakWidthEstimate, 3 );
		//indie_peaks[i]->SetNpx(600);
		indie_peaks[i]->SetLineColor(kRed);
		indie_peaks[i]->SetLineWidth(2);
		
		// Parameter limits
		total->SetParLimits( 2*i + 2, 0.1*h->GetBinContent( h->FindBin( centroids[i] ) ),
							10.0*h->GetBinContent( h->FindBin( centroids[i] ) ) );				// Amplitude
		total->SetParLimits( 2*i + 3, 0.8*centroids[i], 1.2*centroids[i] );						// Mean
		
		// Initial parameters
		total->SetParameter( 2*i + 2, h->GetBinContent( h->FindBin( centroids[i] ) ) );
		total->SetParameter( 2*i + 3, centroids[i] );
		
		// Parameter names
		total->SetParName( 2*i + 2, Form( "%02i Amp.", i ) );
		total->SetParName( 2*i + 3, Form( "%02i Mean", i ) );

	}
	
	// Do the fit
	gErrorIgnoreLevel = kBreak; //kPrint, kInfo, kWarning, kError, kBreak, kSysError, kFatal
	h->Sumw2();
	h->GetXaxis()->SetRangeUser( gPeakChannelLowerThreshold, gPeakChannelUpperThreshold );
	int fitstatus = h->Fit( total, "Q0WLM" );

	// Check out the individual peak parameters
	for( int i = 0; i < gNumberOfAlphaPeaks; i++ ){
		
		indie_peaks[i]->FixParameter( 0, par[2*i + 2] );
		indie_peaks[i]->FixParameter( 1, par[2*i + 3] );
		indie_peaks[i]->FixParameter( 2, par[1] );
		
		centroids[i] = total->GetParameter(2*i + 3);
		errors[i] = total->GetParError(2*i + 3);

	}
	
	// Draw?
	TCanvas *c = new TCanvas("CANVAS-GAUSSIANFIT", "CANVAS-GAUSSIANFIT", 1200, 900 );
	c->cd();
	gStyle->SetOptFit(1111);
	h->GetXaxis()->SetRangeUser( gPeakChannelLowerThreshold, gPeakChannelUpperThreshold+200 );
	h->Draw();
	total->Draw("SAME");
	for( int i = 0; i < gNumberOfAlphaPeaks; i++ )
		indie_peaks[i]->Draw("SAME");
	
	std::string pdfname = "autocal/" + std::string(h->GetName()) + "_spec.pdf";
	c->SaveAs( pdfname.data() );
	gErrorIgnoreLevel = kInfo;
		
	// Clean up
	delete c;
	delete total;
	for( int i = 0; i < gNumberOfAlphaPeaks; ++i )
		delete indie_peaks[i];

	if( fitstatus != 0 ) return true;
	else return false;
	
}

void ISSAutoCalibrator::CalibrateChannel( std::vector<float> &centroids, std::vector<float> &errors,
										 unsigned int mod, unsigned int asic, unsigned int chan ){
	
	// Simple linear fit
	TF1 *calfit = new TF1( "calfit", "pol1", gPeakChannelLowerThreshold, gPeakChannelUpperThreshold );

	// Work out the x,y,z position (approximatley)
	// We would need to work with coincidences to do this properly
	// Then we could avoid some of this and use the ISSEvts class
	float z;
	int row = asic;
	int side = mod * 2 + 1;
	if( asic >= 1 ) row--;
	if( asic >= 4 ) row--;
	if( asic == 0 || asic == 2 || asic == 3 || asic == 5 ){
		
		z = 127.5 - (float)chan;		// take centre of the end strip
		
	}
	else {
		
		z = 64.0;
		if( chan > 64 ) row++;
		if( chan < 24 || chan > 104 ) side--;

	}

	z *= 0.953;							// p-side strip pitch = 0.953 mm
	z += 1.508;							// distance from wafer edge to active region
	z += 125.5 * (3.0 - (float)row);	// move to correct row (125.0 mm wafer length + 0.5 mm inter-wafer gap)

	// Assume it's in the centre of the side
	TVector2 vec2( 26.75, 0.0 );
	vec2 = vec2.Rotate( -1.0 * TMath::Pi() / 6. );		// first face is at -30˚
	vec2 = vec2.Rotate( side * 2. * TMath::Pi() / 6. );	// rotate 60˚ for each face

	TVector3 vec3( vec2.X(), vec2.Y(), z );

	// Calculate the detected alpha energies
	float DetectedEnergy[gNumberOfAlphaPeaks];
	for( unsigned int i = 0; i < gNumberOfAlphaPeaks; ++i )
		DetectedEnergy[i] = react->SimulateDecay( vec3, gAlphaParticleEnergy[i] );

	
	// Now do a graph of the centroids against detected energy
	TGraphErrors *g = new TGraphErrors( gNumberOfAlphaPeaks,
									   centroids.data(), DetectedEnergy,
									   errors.data(), gAlphaParticleErrors );
	
	// Fit, nice and simple
	gErrorIgnoreLevel = kError;
	g->Fit( calfit, "Q" );
	
	// Get the results
	cal->SetAsicEnergyCalibration( mod, asic, chan,
								  calfit->GetParameter(0),
								  calfit->GetParameter(1),
								  0.0 );

	// Calculate the residuals
	float Residuals[gNumberOfAlphaPeaks], ResErr[gNumberOfAlphaPeaks];
	for( unsigned int i = 0; i < gNumberOfAlphaPeaks; ++i ){
		
		Residuals[i]  = DetectedEnergy[i] - calfit->GetParameter(0);
		Residuals[i] -= centroids[i] * calfit->GetParameter(1);
		ResErr[i] = errors[i] * calfit->GetParameter(1);

	}
	
	// Graph the resdiuals
	TGraphErrors *r = new TGraphErrors( gNumberOfAlphaPeaks,
									   DetectedEnergy, Residuals,
									   gAlphaParticleErrors, ResErr );
	
	// Draw the results
	TCanvas *c = new TCanvas("c","Calibration fits",600,900);
	c->Divide(1,2);
	c->cd(1);
	g->Draw("A*P");
	c->cd(2);
	r->Draw("A*P");
	
	// Save to pdf
	std::string pdfname = "autocal/asic_" + std::to_string(mod);
	pdfname += "_" + std::to_string(asic) + "_";
	pdfname += std::to_string(chan) + "_cal.pdf";
	c->SaveAs( pdfname.data() );
	gErrorIgnoreLevel = kInfo;

	delete g;
	delete calfit;
	delete c;
	
	return;
	
}

void ISSAutoCalibrator::DoFits(){

	// Stuff for histogram and peaks
	TH2F *m;
	TH1F *h;
	std::vector<float> centroids;
	std::vector<float> errors;
	centroids.resize( gNumberOfAlphaPeaks );
	errors.resize( gNumberOfAlphaPeaks );
	
	// Progress bar
	int nchans = set->GetNumberOfArrayModules();
	nchans *= set->GetNumberOfArrayASICs();
	nchans *= set->GetNumberOfArrayChannels();

	// Loop over all the channels and perform the alpha spectrum fits
	for( unsigned int mod = 0; mod < set->GetNumberOfArrayModules(); mod++ ){

		for( unsigned int asic = 0; asic < set->GetNumberOfArrayASICs(); asic++ ){
			
			// Get the histogram
			std::string hname = "asic_";
			hname += std::to_string(mod) + "_";
			hname += std::to_string(asic);

			std::string mname = "asic_hists/module_";
			mname += std::to_string(mod) + "/";
			mname += hname;

			m = (TH2F*)input_file->Get( mname.data() );

			for( unsigned int chan = 0; chan < set->GetNumberOfArrayChannels(); chan++ ){
				
				//if( mod != 0 || asic != 5 || chan < 100 ) continue;
								
				// Get the histogram
				std::string pname = hname + "_" + std::to_string(chan);
				h = (TH1F*)m->ProjectionY( pname.data(), chan+1, chan+1 );
				
				// Skip if it's an empty channel or just low stats
				if( h->Integral( gPeakChannelLowerThreshold, gPeakChannelUpperThreshold ) < 100 )
					continue;
				
				// To rebin or not to rebin
				//h->Rebin(2);
				
				// Find the peak centroids for the starting parameters
				FindPeaks( h, centroids );

				// Fit the spectrum with a Gaussian or CrystalBall
				if( !FitSpectrum( h, centroids, errors ) ) {
					
					std::cout << "Fit fail on module " << mod;
					std::cout << ", asic " << asic;
					std::cout << ", channel " << chan;
					std::cout << std::endl;
					
					continue; // skip if fit fails

				}
				
				// Calibrate the channel
				CalibrateChannel( centroids, errors, mod, asic, chan );
				
				// Percent complete
				float chanNo = mod * set->GetNumberOfArrayASICs() * set->GetNumberOfArrayChannels();
				chanNo += asic * set->GetNumberOfArrayChannels();
				chanNo += chan;
				float percent = chanNo*100.0/(float)nchans;

				if( (int)chanNo % (nchans/100) == 0 || chanNo+1 == nchans ) {
				
					// Progress bar in GUI
					//if( _prog_ ) prog->SetPosition( percent );

					// Progress bar in terminal
					std::cout << " " << std::setw(6) << std::setprecision(4);
					std::cout << percent << "%    \r";
					std::cout.flush();
					gSystem->ProcessEvents();
					
				}


			} // chan

		} // asic

	} // mod

	delete m;
	delete h;
	
	return;
	
}
