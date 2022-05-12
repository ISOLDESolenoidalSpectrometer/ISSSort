#ifndef _AutoCalibrator_hh
#define _AutoCalibrator_hh

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <TFile.h>
#include <TTree.h>
#include <TMath.h>
#include <TVector2.h>
#include <TVector3.h>
#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TGProgressBar.h>
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

class ISSAutoCalibrator {
	
public:

	ISSAutoCalibrator( ISSSettings *myset, ISSReaction *myreact );
	virtual inline ~ISSAutoCalibrator(){};

	int	SetInputFile( std::string input_file_name );
	
	void DoFits();
	void FindPeaks( TH1F *h, std::vector<float> &centroids );
	bool FitSpectrum( TH1F *h, std::vector<float> &centroids, std::vector<float> &errors );
	void CalibrateChannel( std::vector<float> &centroids, std::vector<float> &errors,
						  unsigned int mod, unsigned int asic, unsigned int chan );
	void SaveCalFile( std::string name_results_file );

	inline void AddCalibration( ISSCalibration *mycal ){
		cal = mycal;
	};

	inline void AddProgressBar( std::shared_ptr<TGProgressBar> myprog ){
		prog = myprog;
		_prog_ = true;
	};


private:
	
	/// Input file
	TFile *input_file;
		
	// Settings file
	ISSSettings *set;

	// Calibration
	ISSCalibration *cal;

	// Reaction information
	ISSReaction *react;

	// Progress bar
	bool _prog_;
	std::shared_ptr<TGProgressBar> prog;

};

#endif

