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
#include "TF1.h"
#include "Math/RootFinder.h"
#include "Math/Functor.h"


// Settings header
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif

/// A class to read in the calibration file in ROOT's TConfig format.
/// Each ASIC channel can have offset, gain and quadratic terms.
/// Each channel also has a threshold (not implemented)
/// and there is a time offset parameter for each ASIC module, too.

class ISSCalibration {

public:

	ISSCalibration( std::string filename, ISSSettings *myset );
	inline virtual ~ISSCalibration() {
		delete fRand;
	};
	void ReadCalibration();
	void PrintCalibration( std::ostream &stream, std::string opt );
	void SetFile( std::string filename ){
		fInputFile = filename;
	}
	const std::string InputFile(){
		return fInputFile;
	}
	float AsicEnergy( unsigned int mod, unsigned int asic, unsigned int chan, unsigned short raw );
	unsigned int AsicThreshold( unsigned int mod, unsigned int asic, unsigned int chan );
	long AsicTime( unsigned int mod, unsigned int asic );
	bool AsicEnabled( unsigned int mod, unsigned int asic );
	float AsicWalk( unsigned int mod, unsigned int asic, float energy );
	float CaenEnergy( unsigned int mod, unsigned int chan, unsigned short raw );
	unsigned int CaenThreshold( unsigned int mod, unsigned int chan );
	long CaenTime( unsigned int mod, unsigned int chan );
	
	inline void SetAsicEnergyCalibration( unsigned int mod, unsigned int asic, unsigned int chan,
										 float offset, float gain, float gainquadr ){
		if( mod < set->GetNumberOfArrayModules() &&
		   asic < set->GetNumberOfArrayASICs() &&
		   chan < set->GetNumberOfArrayChannels() ) {
			fAsicOffset[mod][asic][chan] = offset;
			fAsicGain[mod][asic][chan] = gain;
			fAsicGainQuadr[mod][asic][chan] = gainquadr;
		}
	};
	inline void SetAsicThreshold( unsigned int mod, unsigned int asic, unsigned int chan,
								 unsigned int thres ){
		if( mod < set->GetNumberOfArrayModules() &&
		   asic < set->GetNumberOfArrayASICs() &&
		   chan < set->GetNumberOfArrayChannels() )
			fAsicThreshold[mod][asic][chan] = thres;
	};
	inline void SetAsicTime( unsigned int mod, unsigned int asic,
							long time ){
		if( mod < set->GetNumberOfArrayModules() &&
		   asic < set->GetNumberOfArrayASICs() )
			fAsicTime[mod][asic] = time;
	};
	inline void SetAsicEnabled( unsigned int mod, unsigned int asic,
							   bool enabled ){
		if( mod < set->GetNumberOfArrayModules() &&
		   asic < set->GetNumberOfArrayASICs() )
			fAsicEnabled[mod][asic] = enabled;
	};
	inline void SetCaenEnergyCalibration( unsigned int mod, unsigned int chan,
										 float offset, float gain, float gainquadr ){
		if( mod < set->GetNumberOfCAENModules() &&
		   chan < set->GetNumberOfCAENChannels() ) {
			fCaenOffset[mod][chan] = offset;
			fCaenGain[mod][chan] = gain;
			fCaenGainQuadr[mod][chan] = gainquadr;
		}
	};
	inline void SetCaenThreshold( unsigned int mod, unsigned int chan,
								 unsigned int thres ){
		if( mod < set->GetNumberOfCAENModules() &&
		   chan < set->GetNumberOfCAENChannels() )
			fCaenThreshold[mod][chan] = thres;
	};
	inline void SetCaenTime( unsigned int mod, unsigned int chan,
							long time ){
		if( mod < set->GetNumberOfCAENModules() &&
		   chan < set->GetNumberOfCAENChannels() )
			fCaenTime[mod][chan] = time;
	};



private:

	std::string fInputFile;
	
	ISSSettings *set;
	
	TRandom *fRand;

	std::vector< std::vector< std::vector<float> > > fAsicOffset;
	std::vector< std::vector< std::vector<float> > > fAsicGain;
	std::vector< std::vector< std::vector<float> > > fAsicGainQuadr;
	std::vector< std::vector< std::vector<unsigned int> > > fAsicThreshold;
	std::vector< std::vector<long> > fAsicTime;
	std::vector< std::vector<bool> > fAsicEnabled;
	std::vector< std::vector< std::vector<double> > > fAsicWalk;

	std::vector< std::vector<float> > fCaenOffset;
	std::vector< std::vector<float> > fCaenGain;
	std::vector< std::vector<float> > fCaenGainQuadr;
	std::vector< std::vector<unsigned int> > fCaenThreshold;
	std::vector< std::vector<long> > fCaenTime;
	
	float fAsicOffsetDefault;
	float fAsicGainDefault;
	float fAsicGainQuadrDefault;
	float fCaenOffsetDefault;
	float fCaenGainDefault;
	float fCaenGainQuadrDefault;
	
	// Stuff for the time walk calculation
	std::unique_ptr<ROOT::Math::RootFinder> rf;
	std::unique_ptr<TF1> fa, fb;
	double walk_params[5];


	//ClassDef(ISSCalibration, 1)
   
};

#endif
