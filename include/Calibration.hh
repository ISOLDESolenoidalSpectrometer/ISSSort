#ifndef __CALIBRATION_HH
#define __CALIBRATION_HH

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <array>
#include <cstdlib>

#include "TSystem.h"
#include "TEnv.h"
#include "TRandom.h"
#include "TMath.h"

// Common header
#ifndef __COMMON_HH
# include "Common.hh"
#endif

/// A class to read in the calibration file in ROOT's TConfig format.
/// Each ASIC channel can have offset, gain and quadratic terms.
/// Each channel also has a threshold for each channel (not implemented)
/// and there is a time offset parameter for each ASIC module, too.

class Calibration {

public:

	Calibration( std::string filename );
	virtual ~Calibration();
	void ReadCalibration();
	void PrintCalibration();
	void SetFile( std::string filename ){
		fInputFile = filename;
	}
	const std::string InputFile(){
		return fInputFile;
	}
	float AsicEnergy( int mod, int asic, int chan, unsigned short raw );
	float AsicThreshold( int mod, int asic, int chan );
	float AsicTime( int mod, int asic );
	


private:

	std::string fInputFile;

	TRandom* fRand;
  
	std::vector< std::vector< std::vector<float> > > fAsicOffset;
	std::vector< std::vector< std::vector<float> > > fAsicGain;
	std::vector< std::vector< std::vector<float> > > fAsicGainQuadr;
	std::vector< std::vector< std::vector<float> > > fAsicThreshold;
	std::vector< std::vector<float> > fAsicTime;

	//ClassDef(Calibration, 1)
   
};

#endif
