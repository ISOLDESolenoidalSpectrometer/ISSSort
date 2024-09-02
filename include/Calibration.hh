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
#include "TGraph.h"
#include "TFile.h"
#include "Math/RootFinder.h"
#include "Math/Functor.h"

// Settings header
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif

// Number of time walk parameters
const unsigned char nwalkpars = 4;

// Hit-bit number for time walk graphs
const unsigned char HitN = 2;

/*!
* \brief A class to read in the calibration file in ROOT's TConfig format.
*
* \details Each ASIC channel can have offset, gain and quadratic terms.
* Each channel also has a threshold (not implemented)
* and there is a time offset parameter for each ASIC module, too.
*/

class ISSCalibration {

public:

	ISSCalibration( std::string filename, std::shared_ptr<ISSSettings> myset );///< Constructor
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

	// Getters
	float AsicEnergy( unsigned int mod, unsigned int asic, unsigned int chan, unsigned short raw );
	unsigned int AsicThreshold( unsigned int mod, unsigned int asic, unsigned int chan );
	long double AsicTime( unsigned int mod, unsigned int asic );
	bool AsicEnabled( unsigned int mod, unsigned int asic );
	float AsicWalk( unsigned int mod, unsigned int asic, float energy, bool hit );
	float CaenEnergy( unsigned int mod, unsigned int chan, int raw );
	unsigned int CaenThreshold( unsigned int mod, unsigned int chan );
	long double CaenTime( unsigned int mod, unsigned int chan );
	std::string CaenType( unsigned int mod, unsigned int chan );
	float MesytecEnergy( unsigned int mod, unsigned int chan, int raw );
	unsigned int MesytecThreshold( unsigned int mod, unsigned int chan );
	long double MesytecTime( unsigned int mod, unsigned int chan );
	std::string MesytecType( unsigned int mod, unsigned int chan );
	
	// Generic VME getters
	inline float VmeEnergy( unsigned int vme, unsigned int mod, unsigned int chan, int raw ){
		if( vme == 0 ) return CaenEnergy( mod, chan, raw );
		else if( vme == 1 ) return MesytecEnergy( mod, chan, raw );
		else {
			std::cerr << "Only two VME crates currently supported: CAEN (vme_id=0) and Mesytec (vme_id=1)" << std::endl;
			return 0;
		}
	};
	inline unsigned int VmeThreshold( unsigned int vme, unsigned int mod, unsigned int chan ){
		if( vme == 0 ) return CaenThreshold( mod, chan );
		else if( vme == 1 ) return MesytecThreshold( mod, chan );
		else {
			std::cerr << "Only two VME crates currently supported: CAEN (vme_id=0) and Mesytec (vme_id=1)" << std::endl;
			return 0;
		}
	};
	inline long double VmeTime( unsigned int vme, unsigned int mod, unsigned int chan ){
		if( vme == 0 ) return CaenTime( mod, chan );
		else if( vme == 1 ) return MesytecTime( mod, chan );
		else {
			std::cerr << "Only two VME crates currently supported: CAEN (vme_id=0) and Mesytec (vme_id=1)" << std::endl;
			return 0;
		}
	};
	inline std::string VmeType( unsigned int vme, unsigned int mod, unsigned int chan ){
		if( vme == 0 ) return CaenType( mod, chan );
		else if( vme == 1 ) return MesytecType( mod, chan );
		else {
			std::cerr << "Only two VME crates currently supported: CAEN (vme_id=0) and Mesytec (vme_id=1)" << std::endl;
			return "";
		}
	};


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
							long double time ){
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
							long double time ){
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
	
	/// Setter for the Mesytec energy calibration parameters
	/// \param[in] mod The module in the Mesytec DAQ
	/// \param[in] chan The channel number of the Mesytec module
	/// \param[in] offset Constant in Mesytec energy calculation
	/// \param[in] gain Linear term in Mesytec energy calculation
	/// \param[in] gainquadr Quadratic term in Mesytec energy calculation
	inline void SetMesytecEnergyCalibration( unsigned int mod, unsigned int chan,
										 float offset, float gain, float gainquadr ){
		if( mod < set->GetNumberOfMesytecModules() &&
		   chan < set->GetNumberOfMesytecChannels() ) {
			fMesyOffset[mod][chan] = offset;
			fMesyGain[mod][chan] = gain;
			fMesyGainQuadr[mod][chan] = gainquadr;
		}
	};
	
	/// Setter for the Mesytec threshold
	/// \param[in] mod The module in the Mesytec DAQ
	/// \param[in] chan The channel number of the Mesytec module
	/// \param[in] thres The threshold value
	inline void SetMesytecThreshold( unsigned int mod, unsigned int chan,
								 unsigned int thres ){
		if( mod < set->GetNumberOfMesytecModules() &&
		   chan < set->GetNumberOfMesytecChannels() )
			fMesyThreshold[mod][chan] = thres;
	};
	
	/// Setter for the Mesytec time
	/// \param[in] mod The module in the Mesytec DAQ
	/// \param[in] chan The channel number of the Mesytec module
	/// \param[in] time The time value
	inline void SetMesytecTime( unsigned int mod, unsigned int chan,
							long double time ){
		if( mod < set->GetNumberOfMesytecModules() &&
		   chan < set->GetNumberOfMesytecChannels() )
			fMesyTime[mod][chan] = time;
	};
	
	/// Setter for the Mesytec type
	/// \param[in] mod The module in the Mesytec DAQ
	/// \param[in] chan The channel number of the Mesytec module
	/// \param[in] thres The type (default = Qlong)
	inline void SetMesytecType( unsigned int mod, unsigned int chan,
							std::string type ){
		if( mod < set->GetNumberOfMesytecModules() &&
		   chan < set->GetNumberOfMesytecChannels() )
			fMesyType[mod][chan] = type;
	};
	


private:

	std::string fInputFile;///< The location of the calibration input file
	
	std::shared_ptr<ISSSettings> set;///< Pointer to the ISSSettings object
	
	TRandom *fRand;///< Used to eliminate binning issues

	// Calibration file value storage
	std::vector< std::vector< std::vector<float> > > fAsicOffset;///< Constant in ASIC energy calculation
	std::vector< std::vector< std::vector<float> > > fAsicGain;///< Linear term in ASIC energy calculation
	std::vector< std::vector< std::vector<float> > > fAsicGainQuadr;///< Quadratic term in ASIC energy calibration
	std::vector< std::vector< std::vector<unsigned int> > > fAsicThreshold;///< Threshold for raw signals on the ASICs
	std::vector< std::vector<long double> > fAsicTime;///< Time offset for signals on a given ASIC
	std::vector< std::vector<bool> > fAsicEnabled;///< Boolean determining if ASIC is enabled or not
	std::vector< std::vector< std::vector<double> > > fAsicWalkHit0;///< Time-walk parameters for the ASICs for hit bit 0 events
	std::vector< std::vector< std::vector<double> > > fAsicWalkHit1;///< Time-walk parameters for the ASICs for hit bit 1 events
	std::vector< std::vector<unsigned char> > fAsicWalkType;///< Type of time-walk parameters for the ASICs, with 0: Annie Dolan's poly + exp function and 1: Sam Reeve's inverted thing

	std::vector< std::vector<float> > fCaenOffset;///< Constant in CAEN energy calculation
	std::vector< std::vector<float> > fCaenGain;///< Linear term in CAEN energy calculation
	std::vector< std::vector<float> > fCaenGainQuadr;///< Quadratic term in CAEN energy calibration
	std::vector< std::vector<unsigned int> > fCaenThreshold;///< Threshold for raw signals from detectors in the CAEN DAQ
	std::vector< std::vector<long double> > fCaenTime;///< Time offset for signals on a given detector in the CAEN DAQ
	std::vector< std::vector<std::string> > fCaenType;///< The type assigned to the CAEN signal
	
	std::vector< std::vector<float> > fMesyOffset;///< Constant in Mesytec energy calculation
	std::vector< std::vector<float> > fMesyGain;///< Linear term in Mesytec energy calculation
	std::vector< std::vector<float> > fMesyGainQuadr;///< Quadratic term in Mesytec energy calibration
	std::vector< std::vector<unsigned int> > fMesyThreshold;///< Threshold for raw signals from detectors in the Mesytec DAQ
	std::vector< std::vector<long double> > fMesyTime;///< Time offset for signals on a given detector in the Mesytec DAQ
	std::vector< std::vector<std::string> > fMesyType;///< The type assigned to the Mesytec signal

	float fAsicOffsetDefault;///< The default constant in ASIC energy calculations
	float fAsicGainDefault;///< The default linear term in ASIC energy calculations
	float fAsicGainQuadrDefault;///< The default quadratic term in ASIC energy calculations
	float fCaenOffsetDefault;///< The default constant in CAEN energy calculations
	float fCaenGainDefault;///< The default linear term in CAEN energy calculations
	float fCaenGainQuadrDefault;///< The default quadratic term in CAEN energy calculations
	float fMesyOffsetDefault;///< The default constant in Mesytec energy calculations
	float fMesyGainDefault;///< The default linear term in Mesytec energy calculations
	float fMesyGainQuadrDefault;///< The default quadratic term in Mesytec energy calculations

	// Stuff for the time walk calculation
	std::unique_ptr<ROOT::Math::RootFinder> rf;///< Root finding object for the time-walk function: walk_function( double *x, double *params )
	TF1 *fa;///< TF1 for the time walk function: walk_function( double *x, double *params )
	TF1 *fb;///< TF1 for the time walk function derivative: walk_derivative( double *x, double *params )
	double walk_params[nwalkpars+1];///< Parameters used to store time-walk parameters

	// Time-walk Graphs
	std::vector< std::vector< std::vector< std::string > > > twgraphfile;///< The location of the time walk graph files
	std::vector< std::vector< std::vector< std::string > > > twgraphname;///< The names of the time walk graphs
	std::vector< std::vector< std::vector< std::shared_ptr<TGraph> > > > tw_graph;///< Vector containing time walk graphs



	//ClassDef(ISSCalibration, 1)
   
};

#endif
