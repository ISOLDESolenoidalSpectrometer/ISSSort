#ifndef _AutoCalibrator_hh
#define _AutoCalibrator_hh

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <TFile.h>
#include <TFitResult.h>
#include <TTree.h>
#include <TMath.h>
#include <TVector2.h>
#include <TVector3.h>
#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TGProgressBar.h>
#include <TPaveStats.h>
#include <TPolyLine.h>
#include <TSystem.h>
#include <TStyle.h>
#include "Math/MinimizerOptions.h"


// Settings header
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif

// Calibration header
#ifndef __CALIBRATION_HH
# include "Calibration.hh"
#endif

// Reaction header
#ifndef __REACTION_HH
# include "Reaction.hh"
#endif

// Fit functions
#ifndef _FitFunctions_hh
#include "FitFunctions.hh"
#endif

/*!
* \brief Calibrates alpha spectra in the ISS Liverpool array
*
* \details The ISSAutoCalibrator class takes a particular set of alpha data runs and tries to create a calibration file for the use of the ISSCalibration class. This can be used to align all of your detectors in energy when sorting for other physics reactions. The operation of this class is primarily laid out in the ISSAutoCalibrator::DoFits() function, which is outlined below.
*
* The first stage is where it tries to find the peaks in the spectrum in the ISSAutoCalibrator::FindPeaks() function. This is a rather crude process at the moment, and it will find many things that could be a peak, whilst rejecting any noise. However, it knows how many peaks it expects to see, so it will try to reduce the number of possible peaks to the actual number of peaks. It is also able to deal with one fewer than expected peaks, assuming that the missing peak is the last one. If the debug mode is on, you can see all of the possible peaks it finds, as well as the final number of peaks it thinks there are. If this process doesn't work for any channels, then initial guesses for the centroids of each peak can be fed manually into the autocal file. Hopefully this won't be too many!
*
* The next stage is where it tries to fit the spectrum with the desired fit shape, which can also be specified in the autocal file (default is Gaussian because it's quicker and mostly gets the job done). These fit functions are defined in the FitFunctions.hh file. If any fits fail or if any parameters are at a limit, then a warning message will be printed to the console. The results of this fit can be found in the printed fitted spectra, and the corresponding linear calibration fit that is produced when creating the calibration file.
*
* __Things to bear in mind__
* - Not all fits that converge are good fits! Make sure you check every fit that is spat out by this!
* - Not all fits that fail to converge are necessarily useless: it might be a parameter that is failing to converge on a particular value. This is bad if it's the centroid (what you want), but not as bad if it's e.g. the amplitude of the peak.
* - You don't have to do every single fit every time it fails. You can turn on manual fits only, and the code will only fit peaks with values straying from the default in the autocal file. Very useful if you have a few channels you want to work on.
* - If, for whatever reason, you change this code, don't assume that your previous good fits will work again! Any changes to this code will require a checking of your previously good fits. The code is designed to be used through the autocal file for the majority of cases!
* - The file produced by this code is __NOT__ using the new calibration that you are producing. It will be using whatever you feed iss_sort at the time. You will have to resort all the data with the new calibration if you want to see how it works.
* - Patrick developed most of this, so direct all problems to him. It's probably his fault!
*
* Have fun calibrating all your channels!!
*/

class ISSAutoCalibrator {
	
public:

	ISSAutoCalibrator( ISSSettings *myset, ISSReaction *myreact, std::string autocal_file ); ///< Constructor
	virtual inline ~ISSAutoCalibrator(){}; ///< Destructor (currently empty)
	
	void ReadAutocalSettings(); ///< Read the autocal settings from the input file
	inline void SetFile( std::string filename ){
		autocal_settings_input_file = filename;
	} ///< Sets the name of the autocal input file in the class

	int	SetOutputFile( std::string output_file_name ); ///< Sets the name of the output root file produced by the autocal hadd-ing process
	
	void DoFits(); ///< The heart of this class, moving from alpha spectra to a calibration
	void FindPeaks( TH1F *h, std::vector<float> &centroids ); ///< Finds the desired number of alpha peaks
	bool FitSpectrum( TH1F *h, std::vector<float> &centroids, std::vector<float> &errors, unsigned int mod, unsigned int asic, unsigned int chan ); ///< Fits the found/specified peaks with the user-specified fit shape
	void CalibrateChannel( std::vector<float> &centroids, std::vector<float> &errors,
						  unsigned int mod, unsigned int asic, unsigned int chan ); ///< Communicates with the ISSCalibration object to produce a calibration file
	void SaveCalFile( std::string name_results_file ); ///< Saves the calibration to a file

	inline void AddCalibration( ISSCalibration *mycal ){
		cal = mycal;
	}; ///< Assigns the calibration pointer in the ISSAutoCalibrator object

	inline void AddProgressBar( std::shared_ptr<TGProgressBar> myprog ){
		prog = myprog;
		_prog_ = true;
	}; ///< Adds a progress bar to the GUI
	
	inline bool GetDebugStatus(){ return _debug_; } ///< Returns the debug status of the ISSAutoCalibrator
	inline bool OnlyManualFitStatus(){ return _only_manual_fits_; } ///< Returns the manual fit status of the ISSAutoCalibrator
	inline std::string GetFitShapeName(){
		if ( myfit == fit_shape::gaussian ){ return "Gaussian"; }
		else if ( myfit == fit_shape::crystalball ){ return "Crystal Ball"; }
		else{ return "UNDEFINED"; }
	} ///< Returns the name of the assigned fit shape used in the ISSAutoCalibrator

private:
	
	// Output file
	TFile *output_file; ///< The output file resulting from the hadd process of all the input files to iss_sort
		
	// Settings file
	ISSSettings *set; ///< Pointer to the settings object

	// Calibration
	ISSCalibration *cal; ///< Pointer to the calibration object

	// Reaction information
	ISSReaction *react; ///< Pointer to the reaction object

	// Progress bar
	bool _prog_;							///< True if the GUI is being used
	std::shared_ptr<TGProgressBar> prog;	///< A progress bar for this stage in the GUI
	
	// Autocal settings options
	std::string autocal_settings_input_file;	///< The name of the autocal file used to control the fits
	
	// Global control options
	bool _debug_;					///< Allows the printing of more information to the console and more images to disk
	bool _only_manual_fits_;		///< Constrains fitting to only those specified in the autocal input file
	fit_shape myfit;				///< Stores the fit shape for peak-fitting
	int rebin_factor;				///< Factor by which to rebin the ADC value that makes up the alpha particle spectrum
	std::string image_file_type;	///< The file format to print the autocal images. Must be supported by ROOT!
	bool _print_bad_calibrations_;	///< Decide whether to print calibrations for fits that failed
	
	// Default parameters
	float default_fit_bg;								///< Initial guess for background of alpha spectrum
	float default_fit_bg_lb;							///< Lower limit for background of alpha spectrum
	float default_fit_bg_ub;							///< Upper limit for background of alpha spectrum
	float default_fit_sigma;							///< Initial guess for sigma of each peak in alpha spectrum
	float default_fit_sigma_lb;							///< Lower limit for sigma of each peak in alpha spectrum
	float default_fit_sigma_ub;							///< Upper limit for sigma of each peak in alpha spectrum
	float default_fit_crystal_ball_alpha;				///< Initial guess for alpha for crystal ball fit in alpha spectrum
	float default_fit_crystal_ball_alpha_lb;			///< Lower limit for alpha for crystal ball fit in alpha spectrum
	float default_fit_crystal_ball_alpha_ub;			///< Upper limit for alpha for crystal ball fit in alpha spectrum
	float default_fit_crystal_ball_n;					///< Initial guess for n for crystal ball fit in alpha spectrum
	float default_fit_crystal_ball_n_lb;				///< Lower limit for n for crystal ball fit in alpha spectrum
	float default_fit_crystal_ball_n_ub;				///< Upper limit for n for crystal ball fit in alpha spectrum
	float default_fit_amplitude_fraction_lb;			///< The lower limit for the amplitude of a peak expressed as a fraction of the height of the bin
	float default_fit_amplitude_fraction_ub;			///< The upper limit for the amplitude of a peak expressed as a fraction of the height of the bin
	float default_fit_peak_width_estimate;				///< A guess for the width of a window, centred on the centroid guess, that determines the upper and lower bounds for the centroid parameter
	float default_fit_peak_height_threshold_fraction;	///< This fraction multiplied by the maximum amplitude in the spectrum determines a threshold on the number of counts, over which each peak must rise
	float default_fit_peak_height_dip_fraction;			///< Ensures that the number of counts has dipped to this fraction of the height of the last peak before being ready to record another peak
	float default_fit_peak_channel_threshold_lb;		///< Puts a lower limit on the channel number where peaks can be identified and fit
	float default_fit_peak_channel_threshold_ub;		///< Puts an upper limit on the channel number where peaks can be identified and fit
	
	// Manual fits
	std::vector< std::vector< std::vector<float> > > my_bg;							///< Vector used to store background guesses for the fits
	std::vector< std::vector< std::vector<float> > > my_bg_lb;						///< Vector used to store background lower bound limits for the fits
	std::vector< std::vector< std::vector<float> > > my_bg_ub;						///< Vector used to store background upper bound limits for the fits
	std::vector< std::vector< std::vector<float> > > my_sigma;						///< Vector used to store sigma (standard deviation) guesses for the fits
	std::vector< std::vector< std::vector<float> > > my_sigma_lb;					///< Vector used to store sigma lower bound limits for the fits
	std::vector< std::vector< std::vector<float> > > my_sigma_ub;					///< Vector used to store sigma upper bound limits for the fits
	std::vector< std::vector< std::vector< std::vector<float> > > > my_centroid;	///< Vector used to store centroid guesses for the fits
	std::vector< std::vector< std::vector< std::vector<float> > > > my_centroid_lb; ///< Vector used to store centroid lower bound limits for the fits
	std::vector< std::vector< std::vector< std::vector<float> > > > my_centroid_ub; ///< Vector used to store centroid upper bound limits for the fits
	std::vector< std::vector< std::vector<bool> > > manual_fit_channel;				///< Boolean for deciding whether the channel is being manually fit or not
	
	// Plotting options (carry across different functions)
	double my_max_amp;			///< Stores the maximum amplitude of peaks for plotting purposes
	double my_threshold;		///< Stores the value of a threshold for each alpha-particle spectrum

};

#endif

