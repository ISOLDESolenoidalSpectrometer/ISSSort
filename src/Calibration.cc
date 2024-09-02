#include "Calibration.hh"

////////////////////////////////////////////////////////////////////////////////
/// Defines the time walk function, which has the form:
/// \f$A + B\cdot \mathrm{d}t + \exp( C + D\cdot \mathrm{d}t ) - E\f$
///  where \f$A,B,C,D\f$ are specified in the input calibration file and \f$E\f$ 
/// is the energy of the particle on the array
/// \param[in] x The dt parameter in the correction
/// \param[in] params The fitted parameters (A--E) in the function
/// \returns The value of the function at \f$\mathrm{d}t\f$
double walk_function( double *x, double *params ){
	
	double deltaT = x[0];
	double A = params[0];
	double B = params[1];
	double C = params[2];
	double D = params[3];
	double E = params[4];

	// Root to solve
	double root = A;
	root += B * deltaT;
	root += TMath::Exp( C + D * deltaT );
	root -= E;
	
	return root;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Defines the time walk function derivative, which has the form:
/// \f$B\cdot + D\cdot\exp( C + D\cdot \mathrm{d}t )\f$
///  with definitions identical to those in walk_function( double *x, double *params )
/// \param[in] x The dt parameter in the correction
/// \param[in] params The fitted parameters (A--E) in the function
/// \returns The value of the function at \f$\mathrm{d}t\f$
double walk_derivative( double *x, double *params ){

	double deltaT = x[0];
	//double A = parama[0] // unused in derivative
	double B = params[1];
	double C = params[2];
	double D = params[3];
	//double E = params[4] // unused in derivative

	// Derivative of root to solve
	double root = B;
	root += D * TMath::Exp( C + D * deltaT );

	return root;
}

////////////////////////////////////////////////////////////////////////////////
/// Constructs the ISSCalibration object. Initialises private variables, reads
/// the calibration input file, and sets up the root-finding algorithm for the
/// time walk correction.
/// \param[in] filename String containing the directory location of the input 
/// calibration file
/// \param[in] myset Pointer to the ISSSettings object
ISSCalibration::ISSCalibration( std::string filename, std::shared_ptr<ISSSettings> myset ) {

	SetFile( filename );
	set = myset;
	ReadCalibration();
	fRand = new TRandom();
	
	// Root finder algorithm
	fa = new TF1( "walk_function", walk_function, -2e4, 2e4, 5 );
	fb = new TF1( "walk_derivative", walk_derivative, -2e4, 2e4, 5 );
	rf = std::make_unique<ROOT::Math::RootFinder>( ROOT::Math::RootFinder::kBRENT );
		
}

////////////////////////////////////////////////////////////////////////////////
/// Reads the calibration input file and sets the values of all of the variables
/// required for calibrating data in ISS. Uses a TEnv environment to read the 
/// contents of the file. Called in ISSCalibration::ISSCalibration( std::string filename, ISSSettings *myset )
void ISSCalibration::ReadCalibration() {

	TEnv *config = new TEnv( fInputFile.data() );
	
	// ASIC initialisation
	fAsicOffset.resize( set->GetNumberOfArrayModules() );
	fAsicGain.resize( set->GetNumberOfArrayModules() );
	fAsicGainQuadr.resize( set->GetNumberOfArrayModules() );
	fAsicThreshold.resize( set->GetNumberOfArrayModules() );
	fAsicTime.resize( set->GetNumberOfArrayModules() );
	fAsicEnabled.resize( set->GetNumberOfArrayModules() );
	fAsicWalkHit0.resize( set->GetNumberOfArrayModules() );
	fAsicWalkHit1.resize( set->GetNumberOfArrayModules() );
	fAsicWalkType.resize( set->GetNumberOfArrayModules() );
	
	fAsicOffsetDefault = -4100.0;
	fAsicGainDefault = 16.0;
	fAsicGainQuadrDefault = 0.0;

	// ASIC parameter read
	for( unsigned int mod = 0; mod < set->GetNumberOfArrayModules(); mod++ ){

		fAsicOffset[mod].resize( set->GetNumberOfArrayASICs() );
		fAsicGain[mod].resize( set->GetNumberOfArrayASICs() );
		fAsicGainQuadr[mod].resize( set->GetNumberOfArrayASICs() );
		fAsicThreshold[mod].resize( set->GetNumberOfArrayASICs() );
		fAsicTime[mod].resize( set->GetNumberOfArrayASICs() );
		fAsicEnabled[mod].resize( set->GetNumberOfArrayASICs() );
		fAsicWalkHit0[mod].resize( set->GetNumberOfArrayASICs() );
		fAsicWalkHit1[mod].resize( set->GetNumberOfArrayASICs() );
		fAsicWalkType[mod].resize( set->GetNumberOfArrayASICs() );

		for( unsigned int asic = 0; asic < set->GetNumberOfArrayASICs(); asic++ ){

			fAsicOffset[mod][asic].resize( set->GetNumberOfArrayChannels() );
			fAsicGain[mod][asic].resize( set->GetNumberOfArrayChannels() );
			fAsicGainQuadr[mod][asic].resize( set->GetNumberOfArrayChannels() );
			fAsicThreshold[mod][asic].resize( set->GetNumberOfArrayChannels() );

			fAsicTime[mod][asic] = config->GetValue( Form( "asic_%d_%d.Time", mod, asic ), 0.0 );
			fAsicEnabled[mod][asic] = config->GetValue( Form( "asic_%d_%d.Enabled", mod, asic ), true );
			
			fAsicWalkType[mod][asic] = config->GetValue( Form( "asic_%d_%d.WalkType", mod, asic ), 0 ); // default is still Annie's
			fAsicWalkHit0[mod][asic].resize( nwalkpars );
			fAsicWalkHit1[mod][asic].resize( nwalkpars );
			for( unsigned int i = 0; i < nwalkpars; i++ ) {
				
				double tmpwalk = config->GetValue( Form( "asic_%d_%d.Walk%d", mod, asic, i ), 0.0 ); // backwards compatibility
				fAsicWalkHit0[mod][asic][i] = config->GetValue( Form( "asic_%d_%d.Walk%d.Hit0", mod, asic, i ), tmpwalk );
				fAsicWalkHit1[mod][asic][i] = config->GetValue( Form( "asic_%d_%d.Walk%d.Hit1", mod, asic, i ), fAsicWalkHit0[mod][asic][i] );
				
			}
			
			for( unsigned int chan = 0; chan < set->GetNumberOfArrayChannels(); chan++ ){
				
				fAsicOffset[mod][asic][chan] = config->GetValue( Form( "asic_%d_%d_%d.Offset", mod, asic, chan ), fAsicOffsetDefault );
				fAsicGain[mod][asic][chan] = config->GetValue( Form( "asic_%d_%d_%d.Gain", mod, asic, chan ), fAsicGainDefault );
				fAsicGainQuadr[mod][asic][chan] = config->GetValue( Form( "asic_%d_%d_%d.GainQuadr", mod, asic, chan ), fAsicGainQuadrDefault );
				fAsicThreshold[mod][asic][chan] = config->GetValue( Form( "asic_%d_%d_%d.Threshold", mod, asic, chan ), 0 );

			}
			
		}

	}
	
	
	// CAEN initialisation
	fCaenOffset.resize( set->GetNumberOfCAENModules() );
	fCaenGain.resize( set->GetNumberOfCAENModules() );
	fCaenGainQuadr.resize( set->GetNumberOfCAENModules() );
	fCaenThreshold.resize( set->GetNumberOfCAENModules() );
	fCaenTime.resize( set->GetNumberOfCAENModules() );
	fCaenType.resize( set->GetNumberOfCAENModules() );
	
	fCaenOffsetDefault = 0.0;
	fCaenGainDefault = 1.0;
	fCaenGainQuadrDefault = 0.0;
	
	// CAEN parameter read
	for( unsigned int mod = 0; mod < set->GetNumberOfCAENModules(); mod++ ){
		
		fCaenOffset[mod].resize( set->GetNumberOfCAENChannels() );
		fCaenGain[mod].resize( set->GetNumberOfCAENChannels() );
		fCaenGainQuadr[mod].resize( set->GetNumberOfCAENChannels() );
		fCaenThreshold[mod].resize( set->GetNumberOfCAENChannels() );
		fCaenTime[mod].resize( set->GetNumberOfCAENChannels() );
		fCaenType[mod].resize( set->GetNumberOfCAENChannels() );
		
		for( unsigned int chan = 0; chan < set->GetNumberOfCAENChannels(); chan++ ){
			
			fCaenOffset[mod][chan] = config->GetValue( Form( "caen_%d_%d.Offset", mod, chan ), fCaenOffsetDefault );
			fCaenGain[mod][chan] = config->GetValue( Form( "caen_%d_%d.Gain", mod, chan ), fCaenGainDefault );
			fCaenGainQuadr[mod][chan] = config->GetValue( Form( "caen_%d_%d.GainQuadr", mod, chan ), fCaenGainQuadrDefault );
			fCaenThreshold[mod][chan] = config->GetValue( Form( "caen_%d_%d.Threshold", mod, chan ), 0 );
			fCaenTime[mod][chan] = config->GetValue( Form( "caen_%d_%d.Time", mod,  chan ), 0.0 );
			fCaenType[mod][chan] = config->GetValue( Form( "caen_%d_%d.Type", mod,  chan ), "Qlong" );
			if( fCaenType[mod][chan] != "Qlong" &&
			    fCaenType[mod][chan] != "Qshort" &&
			    fCaenType[mod][chan] != "Qdiff" ) {
				std::cerr << "Incorrect CAEN energy type, must be Qlong, Qshort or Qdiff. ";
				std::cerr << "Defaulting to Qlong" << std::endl;
				fCaenType[mod][chan] = "Qlong";
			}

		}
		
	}
	
	// Mesytec initialisation
	fMesyOffset.resize( set->GetNumberOfMesytecModules() );
	fMesyGain.resize( set->GetNumberOfMesytecModules() );
	fMesyGainQuadr.resize( set->GetNumberOfMesytecModules() );
	fMesyThreshold.resize( set->GetNumberOfMesytecModules() );
	fMesyTime.resize( set->GetNumberOfMesytecModules() );
	fMesyType.resize( set->GetNumberOfMesytecModules() );

	fMesyOffsetDefault = 0.0;
	fMesyGainDefault = 1.0;
	fMesyGainQuadrDefault = 0.0;
	
	// Mesytec parameter read
	for( unsigned int mod = 0; mod < set->GetNumberOfMesytecModules(); mod++ ){
		
		fMesyOffset[mod].resize( set->GetNumberOfMesytecChannels() );
		fMesyGain[mod].resize( set->GetNumberOfMesytecChannels() );
		fMesyGainQuadr[mod].resize( set->GetNumberOfMesytecChannels() );
		fMesyThreshold[mod].resize( set->GetNumberOfMesytecChannels() );
		fMesyTime[mod].resize( set->GetNumberOfMesytecChannels() );
		fMesyType[mod].resize( set->GetNumberOfMesytecChannels() );

		for( unsigned int chan = 0; chan < set->GetNumberOfMesytecChannels(); chan++ ){
			
			fMesyOffset[mod][chan] = config->GetValue( Form( "mesy_%d_%d.Offset", mod, chan ), fMesyOffsetDefault );
			fMesyGain[mod][chan] = config->GetValue( Form( "mesy_%d_%d.Gain", mod, chan ), fMesyGainDefault );
			fMesyGainQuadr[mod][chan] = config->GetValue( Form( "mesy_%d_%d.GainQuadr", mod, chan ), fMesyGainQuadrDefault );
			fMesyThreshold[mod][chan] = config->GetValue( Form( "mesy_%d_%d.Threshold", mod, chan ), 0 );
			fMesyTime[mod][chan] = config->GetValue( Form( "mesy_%d_%d.Time", mod,  chan ), 0.0 );
			fMesyType[mod][chan] = config->GetValue( Form( "mesy_%d_%d.Type", mod,  chan ), "Qlong" );
			if( fMesyType[mod][chan] != "Qlong" &&
				fMesyType[mod][chan] != "Qshort" &&
				fMesyType[mod][chan] != "Qdiff" ) {
				std::cerr << "Incorrect Mesytec energy type, must be Qlong, Qshort or Qdiff. ";
				std::cerr << "Defaulting to Qlong" << std::endl;
				fMesyType[mod][chan] = "Qlong";
			}

		}
		
	}
	
	// Get Time-walk graphs
	tw_graph.resize( set->GetNumberOfArrayModules()  );
	twgraphfile.resize( set->GetNumberOfArrayModules()  );
	twgraphname.resize( set->GetNumberOfArrayModules()  );

	for( unsigned int mod = 0; mod < set->GetNumberOfArrayModules(); mod++ ){

		tw_graph[mod].resize( set->GetNumberOfArrayASICs() );
		twgraphfile[mod].resize( set->GetNumberOfArrayASICs() );
		twgraphname[mod].resize( set->GetNumberOfArrayASICs() );

		for( unsigned int asic = 0; asic < set->GetNumberOfArrayASICs(); asic++ ){
			
			tw_graph[mod][asic].resize( HitN );
			twgraphfile[mod][asic].resize( HitN );
			twgraphname[mod][asic].resize( HitN );

			for( unsigned int i = 0; i < HitN; i++ ) {

				twgraphfile[mod][asic][i] = config->GetValue( Form( "asic_%d_%d.WalkFile.Hit%d", mod, asic, i ), "NULL" );
				twgraphname[mod][asic][i] = config->GetValue( Form( "asic_%d_%d.WalkName.Hit%d", mod, asic, i ), "Graph" );
				
				// Check if it is given by the user
				if( twgraphfile[mod][asic][i] != "NULL" ) {
					
					TFile *tw_file = new TFile( twgraphfile[mod][asic][i].data(), "READ" );
					if( tw_file->IsZombie() )
						std::cout << "Couldn't open " << twgraphfile[mod][asic][i] << " correctly" << std::endl;
					
					else {
						
						if( !tw_file->GetListOfKeys()->Contains( twgraphname[mod][asic][i].data() ) )
							std::cout << "Couldn't find " << twgraphname[mod][asic][i] << " in " << twgraphfile[mod][asic][i] << std::endl;
						else
							tw_graph[mod][asic][i] = std::make_shared<TGraph>( *static_cast<TGraph*>( tw_file->Get( twgraphname[mod][asic][i].data() )->Clone() ) );
						
					}
					
					tw_file->Close();
					
				}

				// Assign an empty graph file if none is given, so the code doesn't crash
				if( !tw_graph[mod][asic][i] ) tw_graph[mod][asic][i] = std::make_shared<TGraph>();

			} // hit-bit

		} // asic
		
	} // mod
	
	delete config;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Calculates the energy on a particular ASIC. Also adds a small random number
/// to remove binning issues
/// \param[in] mod The module on the array
/// \param[in] asic The ASIC number on the module
/// \param[in] chan The channel number on the ASIC
/// \param[in] raw The raw energy recorded on this ASIC
/// \returns Calibrated energy (if parameters are set), the raw energy (if 
/// parameters are not set), or -1 (if the mod, asic, or channel are out of range)
float ISSCalibration::AsicEnergy( unsigned int mod, unsigned int asic, unsigned int chan, unsigned short raw ) {
	
	float energy, raw_rand;
	
	if( mod < set->GetNumberOfArrayModules() &&
	   asic < set->GetNumberOfArrayASICs() &&
	   chan < set->GetNumberOfArrayChannels() ) {

		raw_rand = raw + 0.5 - fRand->Uniform();

		energy = fAsicGainQuadr[mod][asic][chan] * raw_rand * raw_rand;
		energy += fAsicGain[mod][asic][chan] * raw_rand;
		energy += fAsicOffset[mod][asic][chan];

		// Check if we have defaults
		if( TMath::Abs( fAsicGainQuadr[mod][asic][chan] ) < 1e-6 &&
		    TMath::Abs( fAsicGain[mod][asic][chan] - 1.0 ) < 1e-6 &&
		    TMath::Abs( fAsicOffset[mod][asic][chan] ) < 1e-6 )
			
			return raw;
		
		else return energy;
		
	}
		
	return -1;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Getter for the ASIC threshold
/// \param[in] mod The module on the array
/// \param[in] asic The ASIC number on the module
/// \param[in] chan The channel number on the ASIC
unsigned int ISSCalibration::AsicThreshold( unsigned int mod, unsigned int asic, unsigned int chan ) {
	
	if( mod < set->GetNumberOfArrayModules() &&
	   asic < set->GetNumberOfArrayASICs() &&
	   chan < set->GetNumberOfArrayChannels() ) {

		return fAsicThreshold[mod][asic][chan];
		
	}
	
	return -1;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Getter for the ASIC time
/// \param[in] mod The module on the array
/// \param[in] asic The ASIC number on the module
long double ISSCalibration::AsicTime( unsigned int mod, unsigned int asic ){
	
	if( mod < set->GetNumberOfArrayModules() &&
	   asic < set->GetNumberOfArrayASICs() ) {

		return fAsicTime[mod][asic];
		
	}
	
	return 0;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Getter for whether the ASIC is enabled or not
/// \param[in] mod The module on the array
/// \param[in] asic The ASIC number on the module
bool ISSCalibration::AsicEnabled( unsigned int mod, unsigned int asic ){
	
	if( mod < set->GetNumberOfArrayModules() &&
	   asic < set->GetNumberOfArrayASICs() ) {

		return fAsicEnabled[mod][asic];
		
	}
	
	return 0;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Calculates the ASIC time walk
/// \param[in] mod The module on the array
/// \param[in] asic The ASIC number on the module
/// \param[in] energy The energy of the signal
/// \param[in] hit Tne hit bit value for the event
/// \returns The time-walk-corrected energy of the signal
float ISSCalibration::AsicWalk( unsigned int mod, unsigned int asic, float energy, bool hit ){
	
	float walk = 0;
	
	if( mod < set->GetNumberOfArrayModules() &&
	   asic < set->GetNumberOfArrayASICs() ) {
		
		
		if( fAsicWalkType[mod][asic] == 2 ) {

			if( hit )walk = -tw_graph[mod][asic][1]->Eval(energy);
			else walk = -tw_graph[mod][asic][0]->Eval(energy);

		}

		else{
			// Check if all values are defaulted to zero - no walk correction
			bool nowalk = true;
			for( unsigned int i = 0; i < nwalkpars; i++ ) {
				
				if( ( TMath::Abs( fAsicWalkHit0[mod][asic][i] ) > 1.0e-6 && !hit ) ||
					( TMath::Abs( fAsicWalkHit1[mod][asic][i] ) > 1.0e-6 &&  hit ) )
					nowalk = false;
				
			}
				
			// If no walk correction, just return 0
			if( nowalk ) return 0.0;

			// else calculate the walk using the defined function
			else {
				
				// Params for time walk function ROOT finder
				for( unsigned int i = 0; i < nwalkpars; i++ ){
					
					if( hit ) walk_params[i] = fAsicWalkHit1[mod][asic][i];
					else walk_params[i] = fAsicWalkHit0[mod][asic][i];
					
				}
					
				// Last one is always the energy
				walk_params[nwalkpars] = energy;
				
				// Annie Dolan's function
				if( fAsicWalkType[mod][asic] == 0 ) {
					
					// Set parameters
					fa->SetParameters( walk_params );
					fb->SetParameters( walk_params );
					
					// Build the function and derivative, then solve
					gErrorIgnoreLevel = kBreak; // suppress warnings and errors, but not breaks
					ROOT::Math::GradFunctor1D wf( *fa, *fb );
					rf->SetFunction( wf, -2e4, 2e4 ); // limits
					rf->Solve( 500, 1e-4, 1e-5 );
					
					// Check result
					if( rf->Status() ){
						walk = TMath::QuietNaN();
					}
					else walk = rf->Root();
					gErrorIgnoreLevel = kInfo; // print info and above again
					
				}
				
				// Sam Reeve's function
				if( fAsicWalkType[mod][asic] == 1 ) {
				
					// Functional form: y = a + b / (c*E)^d
					// where y is time walk and E is energy
					walk = walk_params[1];
					walk /= TMath::Power( walk_params[2]*walk_params[4], walk_params[3] );
					walk += walk_params[0];
					
					
					// Find limit inside event window
					// Solved for time walk = 3000 ns
					// E = ( b / (y - a) )^(1/d) / c
					double E_limit;
					double dT_limit = 3000;
					E_limit = walk_params[1];
					E_limit /= dT_limit - walk_params[0];
					E_limit = TMath::Power( E_limit, 1/walk_params[3] );
					E_limit /= walk_params[2];
					E_limit = TMath::Abs(E_limit);
					
					// Add minus as the correction is added to time in EventBuilder
					if( walk_params[4] < E_limit ){
						walk = -3000;
					}
					else{
						walk = -walk;
					}
					
				}
				
			}

		}

	}


	
	return walk;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Calculates the energy on a particular CAEN detector. Also adds a small random 
/// number to remove binning issues
/// \param[in] mod The number of the CAEN module
/// \param[in] chan The channel number on the CAEN module
/// \param[in] raw The raw energy recorded on this detector
/// \returns Calibrated energy (if parameters are set), the raw energy (if 
/// parameters are not set), or -1 (if the mod, asic, or channel are out of range)
float ISSCalibration::CaenEnergy( unsigned int mod, unsigned int chan, int raw ) {
	
	float energy, raw_rand;
	
	//std::cout << "mod=" << mod << "; chan=" << chan << std::endl;

	if( mod < set->GetNumberOfCAENModules() &&
	   chan < set->GetNumberOfCAENChannels() ) {

		raw_rand = raw + 0.5 - fRand->Uniform();

		energy = 0;
		energy =  fCaenGainQuadr[mod][chan] * raw_rand * raw_rand;
		energy += fCaenGain[mod][chan] * raw_rand;
		energy += fCaenOffset[mod][chan];

		// Check if we have defaults
		if( TMath::Abs( fCaenGainQuadr[mod][chan] ) < 1e-6 &&
		    TMath::Abs( fCaenGain[mod][chan] - 1.0 ) < 1e-6 &&
		    TMath::Abs( fCaenOffset[mod][chan] ) < 1e-6 )
			
			return raw;
		
		else return energy;
		
	}

	return -1;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Getter for the CAEN threshold
/// \param[in] mod The number of the CAEN module
/// \param[in] chan The channel number of the detector
unsigned int ISSCalibration::CaenThreshold( unsigned int mod, unsigned int chan ) {
	
	if( mod < set->GetNumberOfCAENModules() &&
	   chan < set->GetNumberOfCAENChannels() ) {

		return fCaenThreshold[mod][chan];
		
	}
	
	return -1;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Getter for the CAEN time
/// \param[in] mod The number of the CAEN module
/// \param[in] chan The channel number of the detector
long double ISSCalibration::CaenTime( unsigned int mod, unsigned int chan ){
	
	if( mod < set->GetNumberOfCAENModules() &&
	   chan < set->GetNumberOfCAENChannels() ) {

		return fCaenTime[mod][chan];
		
	}
	
	return 0;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Getter for the CAEN type = the type of 
/// \param[in] mod The number of the CAEN module
/// \param[in] chan The channel number of the detector
std::string ISSCalibration::CaenType( unsigned int mod, unsigned int chan ){
	
	if( mod < set->GetNumberOfCAENModules() &&
	   chan < set->GetNumberOfCAENChannels() ) {

		return fCaenType[mod][chan];
		
	}
	
	return 0;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Calculates the energy on a particular Mesytec detector. Also adds a small random
/// number to remove binning issues
/// \param[in] mod The number of the Mesytec module
/// \param[in] chan The channel number on the Mesytec module
/// \param[in] raw The raw energy recorded on this detector
/// \returns Calibrated energy (if parameters are set), the raw energy (if
/// parameters are not set), or -1 (if the mod, asic, or channel are out of range)
float ISSCalibration::MesytecEnergy( unsigned int mod, unsigned int chan, int raw ) {
	
	float energy, raw_rand;
	
	//std::cout << "mod=" << mod << "; chan=" << chan << std::endl;
	
	if( mod < set->GetNumberOfMesytecModules() &&
	   chan < set->GetNumberOfMesytecChannels() ) {
		
		raw_rand = raw + 0.5 - fRand->Uniform();
		
		energy = 0;
		energy =  fMesyGainQuadr[mod][chan] * raw_rand * raw_rand;
		energy += fMesyGain[mod][chan] * raw_rand;
		energy += fMesyOffset[mod][chan];
		
		// Check if we have defaults
		if( TMath::Abs( fMesyGainQuadr[mod][chan] ) < 1e-6 &&
		    TMath::Abs( fMesyGain[mod][chan] - 1.0 ) < 1e-6 &&
		    TMath::Abs( fMesyOffset[mod][chan] ) < 1e-6 )
			
			return raw;
		
		else return energy;
		
	}
	
	return -1;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Getter for the Mesytec threshold
/// \param[in] mod The number of the Mesytec module
/// \param[in] chan The channel number of the detector
unsigned int ISSCalibration::MesytecThreshold( unsigned int mod, unsigned int chan ) {
	
	if( mod < set->GetNumberOfMesytecModules() &&
	   chan < set->GetNumberOfMesytecChannels() ) {
		
		return fMesyThreshold[mod][chan];
		
	}
	
	return -1;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Getter for the Mesytec time
/// \param[in] mod The number of the Mesytec module
/// \param[in] chan The channel number of the detector
long double ISSCalibration::MesytecTime( unsigned int mod, unsigned int chan ){
	
	if( mod < set->GetNumberOfMesytecModules() &&
	   chan < set->GetNumberOfMesytecChannels() ) {
		
		return fMesyTime[mod][chan];
		
	}
	
	return 0;
	
}

////////////////////////////////////////////////////////////////////////////////
/// Getter for the Mesytec type = the type of
/// \param[in] mod The number of the Mesytec module
/// \param[in] chan The channel number of the detector
std::string ISSCalibration::MesytecType( unsigned int mod, unsigned int chan ){
	
	if( mod < set->GetNumberOfMesytecModules() &&
	   chan < set->GetNumberOfMesytecChannels() ) {
		
		return fMesyType[mod][chan];
		
	}
	
	return 0;
	
}


////////////////////////////////////////////////////////////////////////////////
/// Prints the calibration to a specified output
/// \param[in] stream Determines where the calibration will be printed
/// \param[in] opt Determines which parameters in the calibration are going to be printed:
/// c = caen only
/// a = asic only
/// e = print only energy terms (i.e. not time, whether device is enabled, or type)
void ISSCalibration::PrintCalibration( std::ostream &stream, std::string opt ){
	
	// Check options for energy only
	bool caen_only = false;
	bool asic_only = false;
	bool energy_only = false;
	if( opt.find("c") != std::string::npos || opt.find("C") != std::string::npos ) caen_only = true;
	if( opt.find("a") != std::string::npos || opt.find("A") != std::string::npos ) asic_only = true;
	if( opt.find("e") != std::string::npos || opt.find("E") != std::string::npos ) energy_only = true;
	
	if( !caen_only ) {
	
		// ASIC print
		for( unsigned int mod = 0; mod < set->GetNumberOfArrayModules(); mod++ ){

			for( unsigned int asic = 0; asic < set->GetNumberOfArrayASICs(); asic++ ){

				// Print these unless we request energy only parameters
				if( !energy_only ) {
					
					// Don't bother printing if they are just defaults
					if( TMath::Abs( fAsicTime[mod][asic] ) > 1e-9 ) stream << Form( "asic_%d_%d.Time: %Lf", mod, asic, fAsicTime[mod][asic] ) << std::endl;
					if( !fAsicEnabled[mod][asic] ) stream << Form( "asic_%d_%d.Enabled: %d", mod, asic, 0 ) << std::endl;
		
					for( unsigned int i = 0; i < nwalkpars; i++ ) {
						
						if( fAsicWalkHit0[mod][asic][i] > 1e-9 || fAsicWalkHit0[mod][asic][i] < 1e-9 )
							stream << Form( "asic_%d_%d.Walk%d.Hit0: %f", mod, asic, i, fAsicWalkHit0[mod][asic][i] ) << std::endl;
						
						if( fAsicWalkHit1[mod][asic][i] > 1e-9 || fAsicWalkHit1[mod][asic][i] < 1e-9 )
							stream << Form( "asic_%d_%d.Walk%d.Hit1: %f", mod, asic, i, fAsicWalkHit1[mod][asic][i] ) << std::endl;
						
					}
					
				}
				
				for( unsigned int chan = 0; chan < set->GetNumberOfArrayChannels(); chan++ ){
					
					// Don't bother printing if they are just defaults
					if( TMath::Abs( fAsicGainQuadr[mod][asic][chan] - fAsicOffsetDefault ) > 1e-6 &&
						TMath::Abs( fAsicGain[mod][asic][chan] - fAsicGainDefault ) > 1e-6 &&
					    TMath::Abs( fAsicOffset[mod][asic][chan] - fAsicGainQuadrDefault ) > 1e-6 ) {

						stream << Form( "asic_%d_%d_%d.Offset: %f", mod, asic, chan, fAsicOffset[mod][asic][chan] ) << std::endl;
						stream << Form( "asic_%d_%d_%d.Gain: %f", mod, asic, chan, fAsicGain[mod][asic][chan] ) << std::endl;
						stream << Form( "asic_%d_%d_%d.GainQuadr: %f", mod, asic, chan, fAsicGainQuadr[mod][asic][chan] ) << std::endl;
						
					} // defaults
				
					if( fAsicThreshold[mod][asic][chan] != 0 )
						stream << Form( "asic_%d_%d_%d.Threshold: %u", mod, asic, chan, fAsicThreshold[mod][asic][chan] ) << std::endl;
					
				} // chan
				
			} // asic

		} // mod

	} // !caen_only
	
	if( !asic_only ) {
	
		// CAEN print
		for( unsigned int mod = 0; mod < set->GetNumberOfCAENModules(); mod++ ){

			for( unsigned int chan = 0; chan < set->GetNumberOfCAENChannels(); chan++ ){

				if( TMath::Abs( fCaenGainQuadr[mod][chan] - fCaenOffsetDefault ) > 1e-6 &&
					TMath::Abs( fCaenGain[mod][chan] - fCaenGainDefault ) > 1e-6 &&
					TMath::Abs( fCaenOffset[mod][chan] - fCaenGainQuadrDefault ) > 1e-6 ) {

					stream << Form( "caen_%d_%d.Offset: %f", mod, chan, fCaenOffset[mod][chan] ) << std::endl;
					stream << Form( "caen_%d_%d.Gain: %f", mod, chan, fCaenGain[mod][chan] ) << std::endl;
					stream << Form( "caen_%d_%d.GainQuadr: %f", mod, chan, fCaenGainQuadr[mod][chan] ) << std::endl;

				}
		
				if( fCaenThreshold[mod][chan] != 0 )
					stream << Form( "caen_%d_%d.Threshold: %u", mod, chan, fCaenThreshold[mod][chan] ) << std::endl;

				if( !energy_only && TMath::Abs( fCaenTime[mod][chan] ) > 1e-9 )
					stream << Form( "caen_%d_%d.Time: %Lf", mod, chan, fCaenTime[mod][chan] ) << std::endl;

				if( !energy_only && fCaenType[mod][chan] != 0 )
					stream << Form( "caen_%d_%d.Type: %s", mod, chan, fCaenType[mod][chan].data() ) << std::endl;

			} // chan
			
		} // mod

	} // !asic_only

};
