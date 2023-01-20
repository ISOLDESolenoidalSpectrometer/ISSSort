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

/*!
* \brief A class to read in the calibration file in ROOT's TConfig format.
*
* \details Each ASIC channel can have offset, gain and quadratic terms.
* Each channel also has a threshold (not implemented)
* and there is a time offset parameter for each ASIC module, too.
*/

class ISSCalibration {

public:

	ISSCalibration( std::string filename, ISSSettings *myset );///< Constructor
	/// Destructor
	inline virtual ~ISSCalibration() {
		delete fRand;
	};

	void ReadCalibration();
	void PrintCalibration( std::ostream &stream, std::string opt );

	/// Setter for the location of the input file
	/// \param[in] filename The location of the input file
	void SetFile( std::string filename ){
		fInputFile = filename;
	}

	/// Getter for the calibration input file location
	const std::string InputFile(){
		return fInputFile;
	}

	float AsicEnergy( unsigned int mod, unsigned int asic, unsigned int chan, unsigned short raw );
	unsigned int AsicThreshold( unsigned int mod, unsigned int asic, unsigned int chan );
	long AsicTime( unsigned int mod, unsigned int asic );
	bool AsicEnabled( unsigned int mod, unsigned int asic );
	float AsicWalk( unsigned int mod, unsigned int asic, float energy );
	float CaenEnergy( unsigned int mod, unsigned int chan, int raw );
	unsigned int CaenThreshold( unsigned int mod, unsigned int chan );
	long CaenTime( unsigned int mod, unsigned int chan );
	std::string CaenType( unsigned int mod, unsigned int chan );
	
	/// Setter for the ASIC energy calibration parameters
	/// \param[in] mod The module on the array
	/// \param[in] asic The ASIC number on the module
	/// \param[in] chan The channel number on the ASIC
	/// \param[in] offset Constant in ASIC energy calculation
	/// \param[in] gain Linear term in ASIC energy calculation
	/// \param[in] gainquadr Quadratic term in ASIC energy calculation
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

	/// Setter for the ASIC threshold
	/// \param[in] mod The module on the array
	/// \param[in] asic The ASIC number on the module
	/// \param[in] chan The channel number on the ASIC
	/// \param[in] thres The threshold value
	inline void SetAsicThreshold( unsigned int mod, unsigned int asic, unsigned int chan,
								 unsigned int thres ){
		if( mod < set->GetNumberOfArrayModules() &&
		   asic < set->GetNumberOfArrayASICs() &&
		   chan < set->GetNumberOfArrayChannels() )
			fAsicThreshold[mod][asic][chan] = thres;
	};

	/// Setter for the ASIC time value
	/// \param[in] mod The module on the array
	/// \param[in] asic The ASIC number on the module
	/// \param[in] time The time value
	inline void SetAsicTime( unsigned int mod, unsigned int asic,
							long time ){
		if( mod < set->GetNumberOfArrayModules() &&
		   asic < set->GetNumberOfArrayASICs() )
			fAsicTime[mod][asic] = time;
	};

	/// Setter for turning the ASIC on or off
	/// \param[in] mod The module on the array
	/// \param[in] asic The ASIC number on the module
	/// \param[in] enabled The value signifying the ASIC's on/off status
	inline void SetAsicEnabled( unsigned int mod, unsigned int asic,
							   bool enabled ){
		if( mod < set->GetNumberOfArrayModules() &&
		   asic < set->GetNumberOfArrayASICs() )
			fAsicEnabled[mod][asic] = enabled;
	};

	/// Setter for the CAEN energy calibration parameters
	/// \param[in] mod The module in the CAEN DAQ
	/// \param[in] chan The channel number of the CAEN module
	/// \param[in] offset Constant in CAEN energy calculation
	/// \param[in] gain Linear term in CAEN energy calculation
	/// \param[in] gainquadr Quadratic term in CAEN energy calculation
	inline void SetCaenEnergyCalibration( unsigned int mod, unsigned int chan,
										 float offset, float gain, float gainquadr ){
		if( mod < set->GetNumberOfCAENModules() &&
		   chan < set->GetNumberOfCAENChannels() ) {
			fCaenOffset[mod][chan] = offset;
			fCaenGain[mod][chan] = gain;
			fCaenGainQuadr[mod][chan] = gainquadr;
		}
	};

	/// Setter for the CAEN threshold
	/// \param[in] mod The module in the CAEN DAQ
	/// \param[in] chan The channel number of the CAEN module
	/// \param[in] thres The threshold value
	inline void SetCaenThreshold( unsigned int mod, unsigned int chan,
								 unsigned int thres ){
		if( mod < set->GetNumberOfCAENModules() &&
		   chan < set->GetNumberOfCAENChannels() )
			fCaenThreshold[mod][chan] = thres;
	};

	/// Setter for the CAEN time
	/// \param[in] mod The module in the CAEN DAQ
	/// \param[in] chan The channel number of the CAEN module
	/// \param[in] time The time value
	inline void SetCaenTime( unsigned int mod, unsigned int chan,
							long time ){
		if( mod < set->GetNumberOfCAENModules() &&
		   chan < set->GetNumberOfCAENChannels() )
			fCaenTime[mod][chan] = time;
	};

	/// Setter for the CAEN type
	/// \param[in] mod The module in the CAEN DAQ
	/// \param[in] chan The channel number of the CAEN module
	/// \param[in] thres The type (default = Qlong)
	inline void SetCaenType( unsigned int mod, unsigned int chan,
							std::string type ){
		if( mod < set->GetNumberOfCAENModules() &&
		   chan < set->GetNumberOfCAENChannels() )
			fCaenType[mod][chan] = type;
	};

private:

	std::string fInputFile;///< The location of the calibration input file
	
	ISSSettings *set;///< Pointer to the ISSSettings object
	
	TRandom *fRand;///< Used to eliminate binning issues

	// Calibration file value storage
	std::vector< std::vector< std::vector<float> > > fAsicOffset;///< Constant in ASIC energy calculation
	std::vector< std::vector< std::vector<float> > > fAsicGain;///< Linear term in ASIC energy calculation
	std::vector< std::vector< std::vector<float> > > fAsicGainQuadr;///< Quadratic term in ASIC energy calibration
	std::vector< std::vector< std::vector<unsigned int> > > fAsicThreshold;///< Threshold for raw signals on the ASICs
	std::vector< std::vector<long> > fAsicTime;///< Time offset for signals on a given ASIC
	std::vector< std::vector<bool> > fAsicEnabled;///< Boolean determining if ASIC is enabled or not
	std::vector< std::vector< std::vector<double> > > fAsicWalk;///< Time-walk parameters for the ASICs, with definitions in walk_function( double *x, double *params )

	std::vector< std::vector<float> > fCaenOffset;///< Constant in CAEN energy calculation
	std::vector< std::vector<float> > fCaenGain;///< Linear term in CAEN energy calculation
	std::vector< std::vector<float> > fCaenGainQuadr;///< Quadratic term in CAEN energy calibration
	std::vector< std::vector<unsigned int> > fCaenThreshold;///< Threshold for raw signals from detectors in the CAEN DAQ
	std::vector< std::vector<long> > fCaenTime;///< Time offset for signals on a given detector in the CAEN DAQ
	std::vector< std::vector<std::string> > fCaenType;///< The type assigned to the CAEN signal

	float fAsicOffsetDefault;///< The default constant in ASIC energy calculations
	float fAsicGainDefault;///< The default linear term in ASIC energy calculations
	float fAsicGainQuadrDefault;///< The default quadratic term in ASIC energy calculations
	float fCaenOffsetDefault;///< The default constant in CAEN energy calculations
	float fCaenGainDefault;///< The default linear term in CAEN energy calculations
	float fCaenGainQuadrDefault;///< The default quadratic term in CAEN energy calculations
	
	// Stuff for the time walk calculation
	std::unique_ptr<ROOT::Math::RootFinder> rf;///< Root finding object for the time-walk function: walk_function( double *x, double *params )
	std::unique_ptr<TF1> fa;///< TF1 for the time walk function: walk_function( double *x, double *params )
	std::unique_ptr<TF1> fb;///< TF1 for the time walk function derivative: walk_derivative( double *x, double *params )
	double walk_params[5];///< Parameters used to store time-walk parameters


	//ClassDef(ISSCalibration, 1)
   
};

#endif
