#include "AutoCalibrator.hh"

///////////////////////////////////////////////////////////////////////////////
/// This constructs the autocalibrator for fitting alpha spectra recorded in ISS
/// \param[in] myset Pointer to a relevant ISSSettings object for this data
/// \param[in] myreact Pointer to a relevant ISSReaction object for this data
/// \param[in] autocal_file A string containing the file name of the autocal file that changes the way this object processes alpha data
ISSAutoCalibrator::ISSAutoCalibrator( ISSSettings *myset, ISSReaction *myreact, std::string autocal_file = "" ){
	
	// First store the settings and reaction objects, and deal with the autocal input file
	set = myset;
	react = myreact;
	SetFile( autocal_file );
	
	// Read the autocal settings from the file
	ReadAutocalSettings();
	
	// Create the directories for the plots
	gSystem->Exec( "mkdir -p autocal" );
	gSystem->Exec( "mkdir -p autocal/spec" );
	gSystem->Exec( "mkdir -p autocal/cal" );
	
	// Make directory for debug plots
	if ( _debug_ ){
		gSystem->Exec( "mkdir -p autocal/debug-find-possible-peaks" );
		gSystem->Exec( "mkdir -p autocal/debug-find-actual-peaks" );
	}
	
	// Set some global defaults
	//ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Fumili");
	
	// Set defaults
	my_max_amp = 0;
	my_threshold = 0;
	
}

///////////////////////////////////////////////////////////////////////////////
/// This function is called in the ISSAutoCalibrator::ISSAutoCalibrator() constructor. This reads all of the user settings for controlling the way the calibration behaves. The behaviours you are able to control are all listed in the default autocal.dat file
void ISSAutoCalibrator::ReadAutocalSettings(){
	
	// Create a TEnv object for reading user inputs
	TEnv *config = new TEnv( autocal_settings_input_file.data() );
	
	// Read the settings for controlling global behaviour
	_debug_ = config->GetValue( "AutocalDebug", 0 );
	_only_manual_fits_ = config->GetValue("AutocalOnlyManualFits", 0 );
	myfit = (fit_shape)config->GetValue( "AutocalFitShape", fit_shape::gaussian );
	rebin_factor = config->GetValue( "AutocalRebinFactor", 1);
	image_file_type = config->GetValue( "AutocalImageFileType", "png" );
	_print_bad_calibrations_ = config->GetValue( "AutocalPrintBadCalibrations", 0 );

	// Check image file type ( see https://root.cern/doc/master/classTPad.html )
	if ( image_file_type !=   "ps" && image_file_type != "eps" && image_file_type != "pdf" &&
	     image_file_type !=  "svg" && image_file_type != "tex" && image_file_type != "gif" &&
	     image_file_type !=  "xpm" && image_file_type != "png" && image_file_type != "jpg" &&
	     image_file_type != "tiff" && image_file_type != "cxx" && image_file_type != "xml" &&
   	     image_file_type != "json" && image_file_type != "root" ){
   	 	
   	 	std::cout << "Unsupported image file type " << image_file_type << " selected. Reverting to default (png)" << std::endl;
   	 	image_file_type = "png";
   	 	
	}
	
	// Default fit values and controls
	// --- Background --- //
	default_fit_bg = config->GetValue( "DefaultFit.Background", 0.1 );
	default_fit_bg_lb = config->GetValue( "DefaultFit.BackgroundLB", 0.0 );
	default_fit_bg_ub = config->GetValue( "DefaultFit.BackgroundUB", 50 );
	
	// Check values
	if ( default_fit_bg < default_fit_bg_lb || default_fit_bg > default_fit_bg_ub || default_fit_bg_lb >= default_fit_bg_ub ){
		std::cout << "DefaultFit.Background parameters are all wrong - setting to defaults..." << std::endl;
		default_fit_bg = 0.1;
		default_fit_bg_lb = 0.0;
		default_fit_bg_ub = 50.0;
	}
	
	
	// --- Sigma --- //
	default_fit_sigma = config->GetValue( "DefaultFit.Sigma", 2.5 );
	default_fit_sigma_lb = config->GetValue( "DefaultFit.SigmaLB", 0.2 );
	default_fit_sigma_ub = config->GetValue( "DefaultFit.SigmaUB", 10.0 );
	
	// Check values
	if ( default_fit_sigma < default_fit_sigma_lb || default_fit_sigma > default_fit_sigma_ub || default_fit_sigma_lb >= default_fit_sigma_ub ){
		std::cout << "DefaultFit.Sigma parameters are all wrong - setting to defaults..." << std::endl;
		default_fit_sigma = 2.5;
		default_fit_sigma_lb = 0.2;
		default_fit_sigma_ub = 10.0;
	}
	
	
	// --- Crystal ball alpha --- //
	default_fit_crystal_ball_alpha = config->GetValue( "DefaultFit.CrystalBallAlpha", 1 );
	default_fit_crystal_ball_alpha_lb = config->GetValue( "DefaultFit.CrystalBallAlphaLB", 1e-6 );
	default_fit_crystal_ball_alpha_ub = config->GetValue( "DefaultFit.CrystalBallAlphaUB", 10 );
	
	// Check values
	if ( default_fit_crystal_ball_alpha < default_fit_crystal_ball_alpha_lb || default_fit_crystal_ball_alpha > default_fit_crystal_ball_alpha_ub || default_fit_crystal_ball_alpha_lb >= default_fit_crystal_ball_alpha_ub ){
		std::cout << "DefaultFit.CrystalBallAlpha parameters are all wrong - setting to defaults..." << std::endl;
		default_fit_crystal_ball_alpha = 1;
		default_fit_crystal_ball_alpha_lb = 1e-6;
		default_fit_crystal_ball_alpha_ub = 10;
	}
	
	
	// --- CrystalBall N --- //
	default_fit_crystal_ball_n = config->GetValue( "DefaultFit.CrystalBallN", 1 );
	default_fit_crystal_ball_n_lb = config->GetValue( "DefaultFit.CrystalBallNLB", 1e-6 );
	default_fit_crystal_ball_n_ub = config->GetValue( "DefaultFit.CrystalBallNUB", 20 );
	
	// Check values
	if ( default_fit_crystal_ball_n < default_fit_crystal_ball_n_lb || default_fit_crystal_ball_n > default_fit_crystal_ball_n_ub || default_fit_crystal_ball_n_lb >= default_fit_crystal_ball_n_ub ){
		std::cout << "DefaultFit.CrystalBallN parameters are all wrong - setting to defaults..." << std::endl;
		default_fit_crystal_ball_n = 1;
		default_fit_crystal_ball_n_lb = 1e-6;
		default_fit_crystal_ball_n_ub = 20;
	}
	
	
	// --- Amplitude fraction --- //
	default_fit_amplitude_fraction_lb = config->GetValue( "DefaultFit.AmplitudeFractionLB", 0.2 );
	default_fit_amplitude_fraction_ub = config->GetValue( "DefaultFit.AmplitudeFractionUB", 2.0 );
	
	// Check values
	if ( default_fit_amplitude_fraction_lb < 0 || default_fit_amplitude_fraction_ub < 0 || default_fit_amplitude_fraction_lb >= default_fit_amplitude_fraction_ub ){
		std::cout << "DefaultFit.AmplitudeFraction parameters are all wrong - setting to defaults..." << std::endl;
		default_fit_amplitude_fraction_lb = 0.2;
		default_fit_amplitude_fraction_ub = 2.0;
	}
	
	
	// --- Peak width estimate --- //
	default_fit_peak_width_estimate = config->GetValue( "DefaultFit.PeakWidthEstimate", 30 );
	
	// Check value
	if ( default_fit_peak_width_estimate < 0 ){
		std::cout << "DefaultFit.PeakWidthEstimate parameters must be positive - setting to default..." << std::endl;
		default_fit_peak_width_estimate = 30;
	}


	// --- Peak height threshold fraction --- //
	default_fit_peak_height_threshold_fraction = config->GetValue( "DefaultFit.PeakHeightThresholdFraction", 0.08 );
	
	// Check value
	if ( default_fit_peak_height_threshold_fraction < 0.0 || default_fit_peak_height_threshold_fraction > 1.0 ){
		std::cout << "DefaultFit.PeakHeightThresholdFraction must be between 0 and 1 - setting to default..." << std::endl;
	}
	
	
	// --- Peak height dip fraction --- //
	default_fit_peak_height_dip_fraction = config->GetValue( "DefaultFit.PeakHeightDipFraction", 0.6 );
	
	// Check value
	if ( default_fit_peak_height_dip_fraction < 0.0 || default_fit_peak_height_dip_fraction > 1.0 ){
		std::cout << "DefaultFit.PeakHeightDipFraction must be between 0 and 1 - setting to default..." << std::endl;
	}
	
	
	// --- Peak channel thresholds --- //
	default_fit_peak_channel_threshold_lb = config->GetValue( "DefaultFit.PeakChannelThresholdLB", 360 );
	default_fit_peak_channel_threshold_ub = config->GetValue( "DefaultFit.PeakChannelThresholdUB", 700 );
	
	// Check values
	if ( default_fit_peak_channel_threshold_lb < 0 || default_fit_peak_channel_threshold_lb > default_fit_peak_channel_threshold_ub || default_fit_peak_channel_threshold_ub < 0 || default_fit_peak_channel_threshold_lb > 4096 || default_fit_peak_channel_threshold_ub > 4096 ){
		std::cout << "DefaultFit.PeakChannelThresholds are all wrong - setting to defaults..." << std::endl;
		default_fit_peak_channel_threshold_lb = 360;
		default_fit_peak_channel_threshold_ub = 700;
	}
	
	
	
	
	// Prepare to read individual fit parameters
	// Resize fit parameter vectors
	my_bg.resize( set->GetNumberOfArrayModules() );
	my_bg_lb.resize( set->GetNumberOfArrayModules() );
	my_bg_ub.resize( set->GetNumberOfArrayModules() );
	my_sigma.resize( set->GetNumberOfArrayModules() );
	my_sigma_lb.resize( set->GetNumberOfArrayModules() );
	my_sigma_ub.resize( set->GetNumberOfArrayModules() );
	my_centroid.resize( set->GetNumberOfArrayModules() );
	my_centroid_lb.resize( set->GetNumberOfArrayModules() );
	my_centroid_ub.resize( set->GetNumberOfArrayModules() );
	manual_fit_channel.resize( set->GetNumberOfArrayModules() );
	
	for ( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ){
	
		my_bg[i].resize( set->GetNumberOfArrayASICs() );
		my_bg_lb[i].resize( set->GetNumberOfArrayASICs() );
		my_bg_ub[i].resize( set->GetNumberOfArrayASICs() );
		my_sigma[i].resize( set->GetNumberOfArrayASICs() );
		my_sigma_lb[i].resize( set->GetNumberOfArrayASICs() );
		my_sigma_ub[i].resize( set->GetNumberOfArrayASICs() );
		my_centroid[i].resize( set->GetNumberOfArrayASICs() );
		my_centroid_lb[i].resize( set->GetNumberOfArrayASICs() );
		my_centroid_ub[i].resize( set->GetNumberOfArrayASICs() );
		manual_fit_channel[i].resize( set->GetNumberOfArrayASICs() );
		
		for ( unsigned int j = 0; j < set->GetNumberOfArrayASICs(); ++j ){
		
			my_bg[i][j].resize( set->GetNumberOfArrayChannels() );
			my_bg_lb[i][j].resize( set->GetNumberOfArrayChannels() );
			my_bg_ub[i][j].resize( set->GetNumberOfArrayChannels() );
			my_sigma[i][j].resize( set->GetNumberOfArrayChannels() );
			my_sigma_lb[i][j].resize( set->GetNumberOfArrayChannels() );
			my_sigma_ub[i][j].resize( set->GetNumberOfArrayChannels() );
			my_centroid[i][j].resize( set->GetNumberOfArrayChannels() );
			my_centroid_lb[i][j].resize( set->GetNumberOfArrayChannels() );
			my_centroid_ub[i][j].resize( set->GetNumberOfArrayChannels() );
			manual_fit_channel[i][j].resize( set->GetNumberOfArrayChannels() );
			
			for ( unsigned int k = 0; k < set->GetNumberOfArrayChannels(); ++k ){
			
				my_centroid[i][j][k].resize( FF_num_alpha_peaks );
				my_centroid_lb[i][j][k].resize( FF_num_alpha_peaks );
				my_centroid_ub[i][j][k].resize( FF_num_alpha_peaks );
				manual_fit_channel[i][j][k] = false;
				
			}
			
		}
		
	}
	
	// Temporary values for testing logic etc. (initialise to zero to avoid weird behaviour!)
	double bg_tmp = 0;
	double bg_lb_tmp = 0;
	double bg_ub_tmp = 0;
	double sig_tmp = 0;
	double sig_lb_tmp = 0;
	double sig_ub_tmp = 0;
	double centroid_tmp = 0;
	double centroid_lb_tmp = 0;
	double centroid_ub_tmp = 0;
	
	// Loop over module
	for ( unsigned int i = 0; i < set->GetNumberOfArrayModules(); ++i ){
	
		// Loop over asic
		for ( unsigned int j = 0; j < set->GetNumberOfArrayASICs(); ++j ){
		
			// Loop over channel
			for ( unsigned int k = 0; k < set->GetNumberOfArrayChannels(); ++k ){
			
				// Background values
				bg_tmp = config->GetValue( Form( "man_%d_%d_%d.Background", i, j, k ), default_fit_bg );
				bg_lb_tmp = config->GetValue( Form( "man_%d_%d_%d.BackgroundLB", i, j, k ), default_fit_bg_lb );
				bg_ub_tmp = config->GetValue( Form( "man_%d_%d_%d.BackgroundUB", i, j, k ), default_fit_bg_ub );
				
				// Check the logic
				if ( bg_lb_tmp <= bg_tmp && bg_tmp <= bg_ub_tmp && bg_lb_tmp < bg_ub_tmp ){
					my_bg[i][j][k] = bg_tmp;
					my_bg_lb[i][j][k] = bg_lb_tmp;
					my_bg_ub[i][j][k] = bg_ub_tmp;
				}
				else{
					std::cout << Form( "man_%d_%d_%d.Background parameters are not logical! Using defaults...", i, j, k ) << std::endl;
					my_bg[i][j][k] = default_fit_bg;
					my_bg_lb[i][j][k] = default_fit_bg_lb;
					my_bg_ub[i][j][k] = default_fit_bg_ub;
				}
				
				// Check if a manual fit has been done
				if ( bg_tmp != default_fit_bg || bg_lb_tmp != default_fit_bg_lb || bg_ub_tmp != default_fit_bg_ub ){
					manual_fit_channel[i][j][k] = true;
				}
				
				// Sigma values
				sig_tmp = config->GetValue( Form( "man_%d_%d_%d.Sigma", i, j, k ), default_fit_sigma );
				sig_lb_tmp = config->GetValue( Form( "man_%d_%d_%d.SigmaLB", i, j, k ), default_fit_sigma_lb );
				sig_ub_tmp = config->GetValue( Form( "man_%d_%d_%d.SigmaUB", i, j, k ), default_fit_sigma_ub );
				
				// Check the logic
				if ( sig_lb_tmp <= sig_tmp && sig_tmp <= sig_ub_tmp && sig_lb_tmp < sig_ub_tmp ){
					my_sigma[i][j][k] = sig_tmp;
					my_sigma_lb[i][j][k] = sig_lb_tmp;
					my_sigma_ub[i][j][k] = sig_ub_tmp;
				}
				else{
					std::cout << Form( "man_%d_%d_%d.Sigma parameters are not logical!", i, j, k ) << std::endl;
					my_sigma[i][j][k] = default_fit_sigma;
					my_sigma_lb[i][j][k] = default_fit_sigma_lb;
					my_sigma_ub[i][j][k] = default_fit_sigma_ub;
				}
				
				// Check if a manual fit has been done
				if ( sig_tmp != default_fit_sigma || sig_lb_tmp != default_fit_sigma_lb || sig_ub_tmp != default_fit_sigma_ub ){
					manual_fit_channel[i][j][k] = true;
				}
				
				// Centroid values (ADC not energy)
				// Loop over number of peaks
				for ( int l = 0; l < FF_num_alpha_peaks; ++l ){
				
					centroid_tmp = config->GetValue( Form( "man_%d_%d_%d.Centroid_%d", i, j, k, l ), -2.0 );
					centroid_lb_tmp = config->GetValue( Form( "man_%d_%d_%d.CentroidLB_%d", i, j, k, l ), -3.0 );
					centroid_ub_tmp = config->GetValue( Form( "man_%d_%d_%d.CentroidUB_%d", i, j, k, l ), -1.0 );
					
					// Check UB, C, and LB are all in the right order if defined (this should work by default)
					if ( 
						( centroid_tmp >= centroid_lb_tmp || centroid_tmp == -2.0 ) &&
						( centroid_tmp <= centroid_ub_tmp || centroid_ub_tmp == -1.0 ) && 
						( centroid_ub_tmp > centroid_lb_tmp || centroid_ub_tmp == -1.0 )
					){
						my_centroid[i][j][k][l] = centroid_tmp;
						my_centroid_lb[i][j][k][l] = centroid_lb_tmp;
						my_centroid_ub[i][j][k][l] = centroid_ub_tmp;
					}
					else{
						std::cout << Form( "Something wrong in %d_%d_%d.%d: %f <= %f <= %f is wrong? Neglecting LB and UB...", i, j, k, l, centroid_lb_tmp, centroid_tmp, centroid_ub_tmp ) << std::endl;
						my_centroid_lb[i][j][k][l] = -3.0;
						my_centroid_ub[i][j][k][l] = -1.0;
					}
					
					// Check if a manual fit has been done
					if ( centroid_tmp > 0 || centroid_lb_tmp > 0 || centroid_ub_tmp > 0 ){
						manual_fit_channel[i][j][k] = true;
					}	
				
				} // peak
				
				// Before we go, check whether the user has indicated whether they want the manual fit on or off. This overrides any automatic on or off switches!
				int test_manual = config->GetValue( Form( "man_%d_%d_%d", i, j, k ), -1 ); // Set default to -1 so that we know whether this has worked
				
				if ( test_manual == 0 ){
					manual_fit_channel[i][j][k] = false;	
				}
				else if ( test_manual == 1 ){
					manual_fit_channel[i][j][k] = true;
				}
				
				
			} // channel
			
		} // asic
		
	} // module
	
	
	// Finished
	delete config;

	return;
}


///////////////////////////////////////////////////////////////////////////////
/// This function is called in the do_autocal() function in the main body of iss_sort.cc
/// \param[in] output_file_name The name of the file produced by hadd-ing all the source files together
/// \returns 0 if it can open the output file, 1 if it cannot
int ISSAutoCalibrator::SetOutputFile( std::string output_file_name ) {
	
	// Open output file.
	output_file = new TFile( output_file_name.data(), "read" );
	if( output_file->IsZombie() ) {
		
		std::cout << "Cannot open " << output_file_name << std::endl;
		return 1;
		
	}

	return 0;
	
}


///////////////////////////////////////////////////////////////////////////////
/// Takes the ISSCalibration object that has been manipulated by the ISSAutoCalibrator functions and prints the total calibration into a file. This is called in the do_autocal() function in iss_sort.cc
/// \param[in] name_results_file The name of the file that is to be printed
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

///////////////////////////////////////////////////////////////////////////////
/// Looks throughout the alpha spectrum and determines where the alpha peaks are likely to be located. It then sets the values of centroids when it has found the number of peaks it thinks are correct. If it finds one less than expected, it will set centroids to have a dummy peak in the final position, which will then be dealt with in the ISSAutoCalibrator::FitSpectrum() function. The results of its peak-finding stages are printed out if the debug flag is enabled.
/// \param[in] h The alpha spectrum histogram
/// \param[in] centroids A vector of floats (passed by reference) that will store the guesses for the centroids for the ISSAutoCalibrator::FitSpectrum() stage
void ISSAutoCalibrator::FindPeaks( TH1F *h, std::vector<float> &centroids ){

	// Set maximum amplitude for plotting to zero
	my_max_amp = 0;
	my_threshold = 0;
	
	// Containers to hold information about potential peaks: centroids and heights
	std::vector<std::vector<int>> peak_info;	// First index corresponds to individual peaks, second index corresponds to height (0) and channel (1)
	std::vector<int> individual_peak;			// First entry is the height, second entry is the channel
	
	// (Re)set peak estimates and vector sizes
	peak_info.resize(0);
	individual_peak.resize(2);
	individual_peak[0] = 0;
	individual_peak[1] = 0;
	
	// Get info about the histogram -> find the maximum value in the desired region BUT ignore any noise spikes still present
	int max = 0;						// max of the histogram (excluding noise)
	int noise_max = 0;					// Stores the max bin value of any noise
	int noise_window = 2;				// Full window size is 2*noise_window + 1 bins
	bool in_noise = false;				// Is it in a region of noise?
	double hist_int = h->Integral();	// Total number of counts in the spectrum
	std::vector<int> noise_bins; 		// Store start and end of noise bins
	std::vector<int>::iterator nb_it;	// Iterator for noise bins

	// First loop over the spectrum and identify noisy channels
	for ( int j = noise_window; j < h->GetNbinsX() - noise_window; j++ ){
	
		// We're not in noise yet but check if we are
		if ( !in_noise ){
			
			// If integral of noise_window is significant fraction of total counts, then probably noise
			if ( h->Integral(j-noise_window,j+noise_window)/hist_int > 0.4 ){
			
				in_noise = true;
				noise_bins.push_back(j-noise_window);	// Started a few channels back in all probability so store this
				
			}
			
		}
		
		// Find the maximum of the noise - search around
		else if ( in_noise ){
		
			// Look around current bin, what is max noise here?
			for ( int k = -noise_window; k <= noise_window ; ++k ){
				
				if ( h->GetBinContent(j+k) > noise_max ){
				
					noise_max = h->GetBinContent(j+k);
					
				}
				
			}
			
			// Check if we have left the noisy region
			if ( h->GetBinContent(j) < 0.1*noise_max ){
				
				// Reset the noise checkers
				noise_bins.push_back(j + noise_window);
				in_noise = false;
				noise_max = 0;
				
			}
			
		}
		
	}

	// Find the maximum bin in the spectrum that is NOT noise
	in_noise = false;
	bool noise_detected = (bool)noise_bins.size();
	nb_it = noise_bins.begin();
	
	// Loop over bins
	for ( int j = 0; j < h->GetNbinsX(); j++ ){
	
		// Ignore any noise in the spectrum
		if ( noise_detected ){
		
			// Check if we are entering noise
			if ( !in_noise && j == *nb_it ){
			
				in_noise = true;
				nb_it++;
				
			}
			// Check if we are leaving noise
			else if ( in_noise && j == *nb_it ){
			
				in_noise = false;
				nb_it++;
				
			}
			
		}
		
		// Check whether there is a new contender to be maximum
		if ( !in_noise && h->GetBinContent(j) > max ){
		
			// Check that the contender bin is above an energy threshold
			if ( h->GetBinLowEdge(j) > default_fit_peak_channel_threshold_lb ){
				max = h->GetBinContent(j);
			}
			
		}
		
	}

	// Define global maximum bin for other functions
	my_max_amp = max;

	// Define quantities based on this analysis
	my_threshold = my_max_amp*default_fit_peak_height_threshold_fraction;	// Threshold on counts to stop accepting small peaks
	bool b_record_peaks = 1;		// Stores whether a peak has dipped enough to count as a peak (true if it has dipped enough)
	int current_peak_height = 0;	// Stores the current height of a peak
	
	// Reset noise conditions again
	in_noise = false;
	nb_it = noise_bins.begin();
	
	// Loop over each bin
	for ( int j = 0; j < h->GetNbinsX(); j++ ){
	
		// Ignore any noise in the spectrum
		if ( noise_detected ){
		
			// Check if we are entering noise
			if ( !in_noise && j == *nb_it ){
			
				in_noise = true;
				nb_it++;
				
			}
			// Check if we are leaving noise
			else if ( in_noise && j == *nb_it ){
			
				in_noise = false;
				nb_it++;
				
			}
			
		}
		
		// Get possible peaks if not in noise
		if ( !in_noise ){
		
			// In a state where we're not recording peaks
			if ( b_record_peaks == 0 ){
				
				// Check to see if the histogram has "dipped" enough to define a peak ( or is now much larger...)
				if ( h->GetBinContent(j) < default_fit_peak_height_dip_fraction*current_peak_height || h->GetBinContent(j) > current_peak_height ){
				
					b_record_peaks = 1; // This tells the code that recording peaks is possible
				
				}
				
			}
		
			// Check to see if the current bin is over the required thresholds
			if ( b_record_peaks == 1 && h->GetBinContent(j) > my_threshold && h->GetBinLowEdge(j) > default_fit_peak_channel_threshold_lb ){
			
				// Now check if bins either side <= current bin content
				// (shouldn't have peak on edge of spectrum, but may need to code that in later...)
				if ( h->GetBinContent(j-1) <= h->GetBinContent(j) && h->GetBinContent(j+1) <= h->GetBinContent(j) ){
				
					// Store those that pass this process
					individual_peak[0] = h->GetBinContent(j);
					individual_peak[1] = h->GetBinCenter(j);
					peak_info.push_back( individual_peak );
					
					// Store most recent peak information
					b_record_peaks = 0;	// Tell it we're unable to record more peaks until above condition is satisfied
					current_peak_height = h->GetBinContent(j);
					
				}
				
			}
		}
		
	}

	// Print possible peak locations in debug mode
	if ( _debug_ ){
	
		// Draw histogram on a canvas
		TCanvas *c1 = new TCanvas( "c_debug_all_peaks", "CANVAS", 1200, 900 );
		
		// Format histogram
		h->SetTitle( Form( "Possible peaks in %s; ADC value; Counts", h->GetName() ) );
		
		// Draw histogram
		h->Draw();
		
		// Set axis limits
		h->GetXaxis()->SetRangeUser( default_fit_peak_channel_threshold_lb, default_fit_peak_channel_threshold_ub );
		h->GetYaxis()->SetRangeUser( 0, 1.1*my_max_amp );
		
		// Define triangle properties
		double triangle_height = h->GetMaximum()*0.04;
		double triangle_width = 4;
		
		TPolyLine *p1[peak_info.size()];
		double x[3] = {0,0,0};
		double y[3] = {0,0,0};
		
		// Define coordinates to draw triangles that mark out peaks
		for ( unsigned int i = 0; i < peak_info.size(); ++i ){
		
			for ( int j = 0; j < 3; ++j ){
			
				x[j] = peak_info[i][1] + (j-1)*triangle_width;
				y[j] = h->GetBinContent( h->FindBin( peak_info[i][1] ) ) + 0.25*triangle_height + TMath::Abs( j-1 )*triangle_height;
				
			}
			
			// Define and draw triangles
			p1[i] = new TPolyLine(3,x,y);
			p1[i]->SetFillColor(kBlue);
			p1[i]->Draw("F SAME");
			
		}
		
		// Print the canvas
		gErrorIgnoreLevel = kWarning;
		c1->SaveAs( Form("autocal/debug-find-possible-peaks/%s_FindPeaks_possible.%s", h->GetName(), image_file_type.data() ) );
		gErrorIgnoreLevel = kInfo;
		
		// Delete the objects
		delete c1;
		for ( unsigned int i = 0; i < peak_info.size(); ++i ) delete p1[i];

	}

	// Now deal with all the competitors -> set bad ones to zero
	// Loop over all peaks
	for ( unsigned int j = 1; j < peak_info.size(); j++ ){
	
		// Loop over all peaks k to compare with j
		for ( unsigned int k = 0; k < j; k++ ){
		
			// First, test that both of the test peaks have positive heights
			if ( peak_info[j][0] > 0 && peak_info[k][0] > 0 ){
			
				// Second, test that the peaks are within range of each other
				if ( TMath::Abs( peak_info[j][1] - peak_info[k][1] ) < 0.5*default_fit_peak_width_estimate ){
				
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
	
	// SPECIALISED TWEAKING BELOW
	// If > gAlphaNumPeaks detected, find the smallest ones and delete those until we have the right number
	while ( (int)peak_info.size() > FF_num_alpha_peaks ){
		int min_index = 0;
		for ( unsigned int i = 1; i < peak_info.size(); ++i ){
			if ( peak_info[i][0] < peak_info[min_index][0] ){ min_index = i; }
		}
		peak_info.erase( peak_info.begin() + min_index );
	}

	// If < num_alpha_peaks, then probably too difficult to fit the last peak, so only have the first 3 --> push a dummy peak for #4
	// NOTE THIS MAY NOT HOLD FOR EVERY ALPHA SPECTRUM!!!
	if ( (int)peak_info.size() == FF_num_alpha_peaks - 1 ){
		individual_peak[0] = -1;
		individual_peak[1] = -1;
		peak_info.push_back( individual_peak );
	}
	
	// Store the peak info into the alpha spectrum
	if( (int)peak_info.size() ==  FF_num_alpha_peaks ) {

		for ( unsigned int j = 0; j < peak_info.size(); j++ ){
			centroids[j] = peak_info[j][1];
		}
	
	}
	else{
		std::cout << h->GetName() << "has " << peak_info.size() << " peaks!" << std::endl;
	}
	
	// Print peak locations in debug mode
	if ( _debug_ ){
	
		// Draw histogram on a canvas
		TCanvas *c2 = new TCanvas( "c_debug_final_peaks", "CANVAS", 1200, 900 );
		
		// Format histogram
		h->SetTitle( Form( "Found peaks in %s; ADC value; Counts", h->GetName() ) );
		
		// Draw histogram
		h->Draw();
		
		// Set axis limits
		h->GetXaxis()->SetRangeUser( default_fit_peak_channel_threshold_lb, default_fit_peak_channel_threshold_ub );
		h->GetYaxis()->SetRangeUser( 0, 1.1*my_max_amp );
		
		// Define triangle properties
		double triangle_height = h->GetMaximum()*0.04;
		double triangle_width = 4;
		
		TPolyLine *p2[peak_info.size()];
		double x[3] = {0,0,0};
		double y[3] = {0,0,0};
		
		// Define coordinates to draw triangles that mark out peaks
		for ( unsigned int i = 0; i < peak_info.size(); ++i ){
		
			for ( int j = 0; j < 3; ++j ){
			
				x[j] = peak_info[i][1] + (j-1)*triangle_width;
				y[j] = h->GetBinContent( h->FindBin( peak_info[i][1] ) ) + 0.25*triangle_height + TMath::Abs( j-1 )*triangle_height;
				
			}
			
			// Define and draw triangles
			p2[i] = new TPolyLine(3,x,y);
			p2[i]->SetFillColor(kRed);
			p2[i]->Draw("F SAME");
			
		}
		
		// Print the canvas
		gErrorIgnoreLevel = kWarning;
		c2->SaveAs( Form("autocal/debug-find-actual-peaks/%s_FindPeaks_final.%s", h->GetName(), image_file_type.data() ) );
		gErrorIgnoreLevel = kInfo;
		
		// Delete the objects
		delete c2;
		for ( unsigned int i = 0; i < peak_info.size(); ++i ) delete p2[i];

	}
	
	return;
	
}


///////////////////////////////////////////////////////////////////////////////
/// Takes the centroid guesses and tries to fit a series of peaks. The guesses used for the parameters are either specified by the user in the autocal file, or the default guesses are used, which should cover the majority of cases.
/// \param[in] h Histogram of the alpha spectrum
/// \param[in] centroids Vector of floats (passed by reference) containing the guesses for the centroids. This is overwritten with the fitted centroids
/// \param[in] errors Vector of floats (passed by reference) used to store the uncertainties on the centroids calculated by the fits
/// \param[in] mod The module number (used for implementing custom parameter guesses)
/// \param[in] asic The asic number  (used for implementing custom parameter guesses)
/// \param[in] chan The channel number  (used for implementing custom parameter guesses)
/// \returns 0 if the fit worked, 1 if it did not
bool ISSAutoCalibrator::FitSpectrum( TH1F *h, std::vector<float> &centroids, std::vector<float> &errors, unsigned int mod, unsigned int asic, unsigned int chan ){

	// First remove any dummies out of the fit (negative centroids)
	for ( unsigned int i = 0; i < centroids.size(); ++i ){
		if ( centroids[i] < 0 ){
			centroids.erase( centroids.begin() + i );
			errors.erase( errors.begin() + i );
			--i;
		}
	}
	
	// Declare the number found alpha peaks
	const int NumberOfFoundAlphaPeaks = centroids.size();
	
	// Print statement if fitting fewer peaks than expected (debug only)
	if ( _debug_ && NumberOfFoundAlphaPeaks < FF_num_alpha_peaks ){
		std::cout << Form( "mod_%d_%d_%d: fitting only %d peaks...", mod, asic, chan, NumberOfFoundAlphaPeaks ) << std::endl;
	}

	// Define array to store fit parameters based on the fit shape
	int npars_tmp = 0;
	if ( myfit == fit_shape::gaussian ) npars_tmp = 2*NumberOfFoundAlphaPeaks + 2;
	else if ( myfit == fit_shape::crystalball ) npars_tmp = 2*NumberOfFoundAlphaPeaks + 4;
	else return false;
	
	const int npars = npars_tmp;
	double par[npars];
	
	// Define total fit based on fit shape
	TF1 *total;
	if ( myfit == fit_shape::gaussian ){
		total = new TF1( "totalfit", MultiAlphaGaussianBG, default_fit_peak_channel_threshold_lb, default_fit_peak_channel_threshold_ub, npars );
	}
	else{
		total = new TF1( "totalfit", MultiCrystalBallFunctionBG, default_fit_peak_channel_threshold_lb, default_fit_peak_channel_threshold_ub, npars );
	}
	

	// Format total fit
	total->SetNpx(600);
	total->SetLineColor(kBlack);
	total->SetLineWidth(2);
	
	// Define individual fits
	TF1 *indie_peaks[NumberOfFoundAlphaPeaks];
	
	// Set bg limits --> MUST BE PARAMETER 0 OF TOTAL FIT
	total->SetParameter( 0, my_bg[mod][asic][chan] );	// Flat background
	total->SetParLimits( 0, my_bg_lb[mod][asic][chan], my_bg_ub[mod][asic][chan] );	// Flat background
	total->SetParName( 0, "Flat bg" );

	// Width --> MUST BE PARAMETER 1 OF TOTAL FIT
	total->SetParameter( 1, my_sigma[mod][asic][chan] );
	total->SetParLimits( 1, my_sigma_lb[mod][asic][chan], my_sigma_ub[mod][asic][chan] );	// Sigma
	total->SetParName( 1, "Sigma" );
	
	// Additional parameters for other fit shapes
	if ( myfit == fit_shape::crystalball ){
		// Crystal ball alpha + n
		total->SetParameter( 2, default_fit_crystal_ball_alpha );		// Alpha = par 2
		total->SetParLimits( 2, default_fit_crystal_ball_alpha_lb, default_fit_crystal_ball_alpha_ub );
		total->SetParName( 2, "Alph" );
		
		total->SetParameter( 3, 1 );		// n = par 3
		total->SetParLimits( 3, 1e-6, 20 );
		total->SetParName( 3, "n   " );
	}

	// Define a mean and amp index
	int mean_index = 0;
	int amp_index = 0;

	// Set up individual fits and set parameter limits
	for( int i = 0; i < NumberOfFoundAlphaPeaks; i++ ){
	
		// Define the upper and lower bound guesses for the centroid
		float lb = centroids[i] - 0.5*default_fit_peak_width_estimate;
		float ub = centroids[i] + 0.5*default_fit_peak_width_estimate;
		
		// Check the lb values are OK and reassign to manual values from user if specified
		if ( my_centroid_lb[mod][asic][chan][i] <= centroids[i] && my_centroid_lb[mod][asic][chan][i] > 0  ){
		
			lb = my_centroid_lb[mod][asic][chan][i];
		
		}
		// Specified but too high...print error
		else if ( my_centroid_lb[mod][asic][chan][i] > 0 ){
		
			std::cout << Form( "Bad man_%d_%d_%d.CentroidLB_%d = %f v.s. Centroid = %f", mod, asic, chan, i, my_centroid_lb[mod][asic][chan][i], centroids[i] ) << std::endl;
			
		}
		
		// Check the ub values are OK and reassign to manual values from user if specified
		if ( my_centroid_ub[mod][asic][chan][i] <= centroids[i] && my_centroid_ub[mod][asic][chan][i] > 0  ){
		
			ub = my_centroid_ub[mod][asic][chan][i];
			
		}
		// Specified but too low...print error
		else if ( my_centroid_ub[mod][asic][chan][i] > 0 ){
		
			std::cout << Form( "Bad man_%d_%d_%d.CentroidUB_%d = %f v.s. Centroid = %f", mod, asic, chan, i, my_centroid_ub[mod][asic][chan][i], centroids[i] ) << std::endl;
			
		}
		
		
		// Declare individual fits and calculate amp and mean indices
		if ( myfit == fit_shape::gaussian ){
		
			indie_peaks[i] = new TF1( Form( "ind%i", i ), AlphaGaussianBG, lb, ub, 4 );
			amp_index = 2*i + 2;
			mean_index = 2*i + 3;
			
		}
		else if ( myfit == fit_shape::crystalball ){
		
			indie_peaks[i] = new TF1( Form( "ind%i", i ), CrystalBallFunctionBG, lb, ub, 6 );
			amp_index = 2*i + 4;
			mean_index = 2*i + 5;
			
		}
		
		// Set parameter limits for amp and mean
		total->SetParLimits( amp_index, default_fit_amplitude_fraction_lb*h->GetBinContent( h->FindBin( centroids[i] ) ),
							default_fit_amplitude_fraction_ub*h->GetBinContent( h->FindBin( centroids[i] ) ) );				// Amplitude
		total->SetParLimits( mean_index, lb, ub );				// Mean
		
		// Initial parameters
		total->SetParameter( amp_index, h->GetBinContent( h->FindBin( centroids[i] ) ) );
		total->SetParameter( mean_index, centroids[i] );
		
		// Parameter names
		total->SetParName( amp_index, Form( "%02i Amp.", i ) );
		total->SetParName( mean_index, Form( "%02i Mean", i ) );
		
		// Formatting
		indie_peaks[i]->SetNpx(600);
		indie_peaks[i]->SetLineColor(kRed);
		indie_peaks[i]->SetLineWidth(2);

	}
	
	// Set the axis range and change error level to avoid printing to console
	gErrorIgnoreLevel = kBreak; //kPrint, kInfo, kWarning, kError, kBreak, kSysError, kFatal
	h->Sumw2();
	h->GetXaxis()->SetRangeUser( default_fit_peak_channel_threshold_lb, default_fit_peak_channel_threshold_ub );
	
	// Define fit string and fit the spectrum
	std::string fit_string = ( _debug_ && _only_manual_fits_ ? "" : "Q" );
	fit_string = fit_string + "0WLMS";
	TFitResultPtr fit_ptr = h->Fit( total, fit_string.data() );
	
	// Get the fit status and chi^2
	bool fitstatus = (bool)fit_ptr;
	double chi2 = fit_ptr->Chi2();
	
	// Print thresholds for fit to check whether anything is at the limit ( [debug + manual fits only] / fit fail )
	if ( ( _debug_ && _only_manual_fits_ ) || fitstatus <= 0 ){
	
		// Variables
		int wid = 10;					// Print width
		double par_min = 0;				// Value of parameter LB
		double par_max = 0;				// Value of parameter UB
		double par_val = 0;				// Parameter value
		
		// Loop over total fit parameters
		for ( int i = 0; i < total->GetNpar(); ++i ){
		
			// Get the parameter LB, UB, and value
			total->GetParLimits(i, par_min, par_max );
			par_val = total->GetParameter(i);
			
			// Check if parameters at a lower limit (or very very close)
			if ( TMath::Abs( par_val - par_min ) < 0.001 ){
				
				// Print the limit
				std::cout << "Fit warning on module " << mod << ", asic " << asic << ", channel " << chan << ": " <<
					std::left << std::setw(wid) << total->GetParName(i) << " at lower limit, " << 
					par_min << " (actual value = " << par_val << ")" << std::endl;
					
			}
			
			// Check if parameters at a higher limit (or very very close)
			else if ( TMath::Abs( par_val - par_max ) < 0.001 ){
				
				// Print the limit
				std::cout << "Fit warning on module " << mod << ", asic " << asic << ", channel " << chan << ": " <<
					std::left << std::setw(wid) << total->GetParName(i) << " at upper limit, " << 
					par_max << " (actual value = " << par_val << ")" << std::endl;
					
			}
				
		}
	
	}
	
	// Fill the parameter index
	total->GetParameters(&par[0]);

	// Define the individual peak fits from the total fit
	int ind_mean_index = 0;
	int ind_amp_index = 0;
	
	// Loop over number of peaks
	for( int i = 0; i < NumberOfFoundAlphaPeaks; i++ ){
	
		// Fit bg and width
		indie_peaks[i]->FixParameter( 0, par[0] );
		indie_peaks[i]->FixParameter( 1, par[1] );
	
		// Calculate parameter indices
		if ( myfit == fit_shape::gaussian ){
			
			amp_index = 2*i + 2;
			mean_index = 2*i + 3;
			ind_amp_index = 2;
			ind_mean_index = 3;
			
		}
		if ( myfit == fit_shape::crystalball ){
		
			amp_index = 2*i + 4;
			mean_index = 2*i + 5;
			ind_amp_index = 4;
			ind_mean_index = 5;
			
			// Fix alpha + n
			indie_peaks[i]->FixParameter( 2, par[2] );
			indie_peaks[i]->FixParameter( 3, par[3] );
			
		}
		
		// Fix amplitude and mean parameters for individual fits
		indie_peaks[i]->FixParameter( ind_amp_index, par[amp_index] );
		indie_peaks[i]->FixParameter( ind_mean_index, par[mean_index] );
		
		// Get the centroids and errors
		centroids[i] = total->GetParameter(mean_index);
		errors[i] = total->GetParError(mean_index);
		
		// Run some sensible checks and print warnings
		// Check all heights are above the defined threshold
		if ( par[amp_index] < my_threshold ){
		
			std::cout << "Fit warning on module " << mod;
			std::cout << ", asic " << asic;
			std::cout << ", channel " << chan;
			std::cout << ": peaks not above threshold" << std::endl;
		}
		
		// Check that all of the peaks are well separated (at least 1 s.d. away from each other)
		if( i < NumberOfFoundAlphaPeaks - 1 ){
		
			if ( centroids[i+1] - centroids[i] - 2*par[1] < 0 ){
			
				std::cout << "Fit warning on module " << mod;
				std::cout << ", asic " << asic;
				std::cout << ", channel " << chan;
				std::cout << ": peaks " << i << " and " << i+1 << " are quite close together" << std::endl;
				
			}
			
		}
		
		// Check the value of the chi^2
		if ( chi2 > 1e6 ){
		
			std::cout << "Fit warning on module " << mod;
			std::cout << ", asic " << asic;
			std::cout << ", channel " << chan;
			std::cout << ": chi-squared value very large" << std::endl;
		
		}
		
	}
	
	// Draw the fitted peaks on the spectrum, alongside the individual fits
	TCanvas *c = new TCanvas( "c_fitted_peaks", Form( "Fitted alpha peaks: module %d asic %d channel %d", mod, asic, chan ), 1600, 900 );
	c->cd();
	
	// Format appearance of canvas
	gStyle->SetOptFit(1111);
	h->GetXaxis()->SetRangeUser( default_fit_peak_channel_threshold_lb, default_fit_peak_channel_threshold_ub+200 );
	h->GetYaxis()->SetRangeUser( 0, 1.1*my_max_amp );
	h->SetTitle( Form( "Fitted alpha peaks: module %d asic %d channel %d; ADC value; Counts", mod, asic, chan ) );
	
	// Draw the spectrum and all of the fits
	h->Draw();
	total->Draw("SAME");
	for( int i = 0; i < NumberOfFoundAlphaPeaks; i++ ) indie_peaks[i]->Draw("SAME");
	
	// Save the canvas
	std::string imgname = "autocal/spec/" + std::string(h->GetName()) + "_spec." + image_file_type;
	c->SaveAs( imgname.data() );
	gErrorIgnoreLevel = kInfo;
	
	// Clean up the memory
	delete c;
	delete total;
	for( int i = 0; i < NumberOfFoundAlphaPeaks; ++i ){
		delete indie_peaks[i];
	}

	// Return value
	return fitstatus;
	
}


///////////////////////////////////////////////////////////////////////////////
/// Does a linear fit of the ADC value against the calculated alpha particle energy. This does a fancy correction to the energy based on where the particle lands on the array.
/// \param[in] centroids Vector of floats (passed by reference) containing the fitted centroids from the ISSAutoCalibrator::FitSpectrum() function
/// \param[in] errors Vector of floats (passed by reference) containing the errors of the fitted centroids from the ISSAutoCalibrator::FitSpectrum() function
/// \param[in] mod Module number
/// \param[in] asic ASIC number
/// \param[in] chan Channel number
void ISSAutoCalibrator::CalibrateChannel( std::vector<float> &centroids, std::vector<float> &errors,
										 unsigned int mod, unsigned int asic, unsigned int chan ){
						
	// Get the number of found alpha peaks				 
	const int NumberOfFoundAlphaPeaks = centroids.size();
	
	// Simple linear fit
	TF1 *calfit = new TF1( "calfit", "pol1", default_fit_peak_channel_threshold_lb, default_fit_peak_channel_threshold_ub );

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
	float DetectedEnergy[NumberOfFoundAlphaPeaks];
	for( int i = 0; i < NumberOfFoundAlphaPeaks; ++i )
		DetectedEnergy[i] = react->SimulateDecay( vec3, FF_alpha_peak_energy[i] );

	
	// Now do a graph of the centroids against detected energy
	
	TGraphErrors *g = new TGraphErrors( NumberOfFoundAlphaPeaks,
									   centroids.data(), DetectedEnergy,
									   errors.data(), FF_alpha_peak_energy_error );
	g->SetTitle( "Calibration Fit; ADC Value; Alpha Particle Energy [keV]" );
	
	// Fit, nice and simple
	gErrorIgnoreLevel = kError;
	g->Fit( calfit, "Q" );
	
	// Set the calibration in the calibration object
	cal->SetAsicEnergyCalibration( mod, asic, chan,
								  calfit->GetParameter(0),
								  calfit->GetParameter(1),
								  0.0 );

	// Calculate the residuals
	float Residuals[NumberOfFoundAlphaPeaks], ResErr[NumberOfFoundAlphaPeaks];
	for( int i = 0; i < NumberOfFoundAlphaPeaks; ++i ){
		
		Residuals[i]  = DetectedEnergy[i] - calfit->GetParameter(0);
		Residuals[i] -= centroids[i] * calfit->GetParameter(1);
		ResErr[i] = errors[i] * calfit->GetParameter(1);

	}
	
	// Graph the resdiuals
	TGraphErrors *r = new TGraphErrors( NumberOfFoundAlphaPeaks,
									   DetectedEnergy, Residuals,
									   FF_alpha_peak_energy_error, ResErr );
	r->SetTitle( "Residuals Plot; ADC Value; Alpha Particle Energy Residuals [keV]" );
	
	// Draw the results
	TCanvas *c = new TCanvas("c","Calibration fits",800,900);
	c->Divide(1,2);
	c->cd(1);
	gStyle->SetOptFit(1111);
	g->Draw("A*P");
	c->Update();
	
	// Move the stats box on the calibration fit so that you can see the whole fit
	TPaveStats *st = (TPaveStats*)gPad->GetPrimitive("stats");
	if ( st != NULL ){
		st->SetX1NDC(0.80);	st->SetX2NDC(0.98);
		st->SetY1NDC(0.35); st->SetY2NDC(0.65);
		c->Modified(); c->Update();
	}
	
	// Draw the residuals
	c->cd(2);
	r->Draw("A*P");
	
	// Save to png
	std::string imgname = "autocal/cal/asic_" + std::to_string(mod);
	imgname += "_" + std::to_string(asic) + "_";
	imgname += std::to_string(chan) + "_cal." + image_file_type;
	c->SaveAs( imgname.data() );
	gErrorIgnoreLevel = kInfo;
	
	// Clean up memory
	delete g;
	delete calfit;
	delete c;
	
	return;
	
}

///////////////////////////////////////////////////////////////////////////////
/// Calls a lot of the functions in this class so that the behaviour can be controlled. For every module-asic-channel combination, this function calls:
/// - ISSAutoCalibrator::FindPeaks()
/// - ISSAutoCalibrator::FitSpectrum()
/// - ISSAutoCalibrator::CalibrateChannel()
/// Error messages are printed if any of the fits fail or warnings are issued
void ISSAutoCalibrator::DoFits(){

	// Stuff for histogram and peaks
	TH2F *m;
	TH1F *h;
	std::vector<float> centroids;
	std::vector<float> errors;
	
	// Progress bar
	int nchans = set->GetNumberOfArrayModules();
	nchans *= set->GetNumberOfArrayASICs();
	nchans *= set->GetNumberOfArrayChannels();

	// Loop over all the channels and perform the alpha spectrum fits
	// Loop over modules in the array
	for( unsigned int mod = 0; mod < set->GetNumberOfArrayModules(); mod++ ){

		// Loop over ASICs in the module
		for( unsigned int asic = 0; asic < set->GetNumberOfArrayASICs(); asic++ ){

			// Get the histogram
			std::string hname = "asic_";
			hname += std::to_string(mod) + "_";
			hname += std::to_string(asic);

			std::string mname = "asic_hists/module_";
			mname += std::to_string(mod) + "/";
			mname += hname;

			m = (TH2F*)output_file->Get( mname.data() );

			// Loop over channels in the asic
			for( unsigned int chan = 0; chan < set->GetNumberOfArrayChannels(); chan++ ){
				
				// Only do the fits if the user desires
				if ( _only_manual_fits_ == false || ( _only_manual_fits_ == true && manual_fit_channel[mod][asic][chan] == true ) ){
				
					// Clear and resize the vectors holding the centroid and error information
					centroids.clear();
					errors.clear();
					centroids.resize( FF_num_alpha_peaks );
					errors.resize( FF_num_alpha_peaks );
					
					// Get the histogram of the alpha spectrum
					std::string pname = hname + "_" + std::to_string(chan);
					h = (TH1F*)m->ProjectionY( pname.data(), chan+1, chan+1 );
					
					// Skip if it's an empty channel or just low stats
					if( h->Integral( default_fit_peak_channel_threshold_lb, default_fit_peak_channel_threshold_ub ) < 100 )
						continue;
					
					// Rebin if requested by user (default is 1)
					h->Rebin( rebin_factor );
					
					// Find the peak centroids for the starting parameters (or impose mandatory ones)
					FindPeaks( h, centroids );
					
					// Impose user-defined centroids to override those from the FindPeaks function
					if ( manual_fit_channel[mod][asic][chan] ){
						
						for ( int i = 0; i < FF_num_alpha_peaks; ++i ){
						
							if ( my_centroid[mod][asic][chan][i] > 0 ){ centroids[i] = my_centroid[mod][asic][chan][i]; }
							
						}
						
					}

					// Fit the spectrum with the user-defined peak shape, and get the status of the fit
					int fitstatus = FitSpectrum( h, centroids, errors, mod, asic, chan );
					
					// Print error messages if the user-defined fit fails for some reason
					if( fitstatus == 0 ) {
						
						std::cout << "Fit    fail on module " << mod;
						std::cout << ", asic " << asic;
						std::cout << ", channel " << chan;
						std::cout << ": fit did not converge" << std::endl;
						
						if ( !_print_bad_calibrations_ ){
							continue;	// Skip if it fails
						}

					}
					else{
						// Print whether the fit succeeds if doing manual fits and in debug mode
						if ( _debug_ && _only_manual_fits_ ){
							std::cout << "Fit     win on module " << mod;
							std::cout << ", asic " << asic;
							std::cout << ", channel " << chan;
							std::cout << std::endl;
						}
					}
					
					// Calibrate the channel
					CalibrateChannel( centroids, errors, mod, asic, chan );
					
					// Print progress in percent complete
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

				}
			} // chan

		} // asic

	} // mos
	
	return;
	
}
