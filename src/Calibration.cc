#include "Calibration.hh"

double walk_function( double *x, double *params ){
	
	// Root to solve
	double root = params[0];
	root += params[1] * x[0];
	root += TMath::Exp( params[2] + params[3] * x[0] );
	root -= params[4];
	
	return root;
	
}

double walk_derivative( double *x, double *params ){

	// Derivative of root to solve
	double root = TMath::Exp( params[2] + params[3] * x[0] );
	root *= params[3];
	root += params[1];
	root -= params[4];
	
	return root;
}


ISSCalibration::ISSCalibration( std::string filename, ISSSettings *myset ) {

	SetFile( filename );
	set = myset;
	ReadCalibration();
	fRand = new TRandom();
	
	// Root finder algorithm
	fa = std::make_unique<TF1>( "walk_function", walk_function, -2e4, 2e4, 5 );
	fb = std::make_unique<TF1>( "walk_derivative", walk_derivative, -2e4, 2e4, 5 );
	rf = std::make_unique<ROOT::Math::RootFinder>( ROOT::Math::RootFinder::kBRENT );

		
}

void ISSCalibration::ReadCalibration() {

	TEnv *config = new TEnv( fInputFile.data() );
	
	// ASIC initialisation
	fAsicOffset.resize( set->GetNumberOfArrayModules() );
	fAsicGain.resize( set->GetNumberOfArrayModules() );
	fAsicGainQuadr.resize( set->GetNumberOfArrayModules() );
	fAsicThreshold.resize( set->GetNumberOfArrayModules() );
	fAsicTime.resize( set->GetNumberOfArrayModules() );
	fAsicEnabled.resize( set->GetNumberOfArrayModules() );
	fAsicWalk.resize( set->GetNumberOfArrayModules() );
	
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
		fAsicWalk[mod].resize( set->GetNumberOfArrayASICs() );

		for( unsigned int asic = 0; asic < set->GetNumberOfArrayASICs(); asic++ ){

			fAsicOffset[mod][asic].resize( set->GetNumberOfArrayChannels() );
			fAsicGain[mod][asic].resize( set->GetNumberOfArrayChannels() );
			fAsicGainQuadr[mod][asic].resize( set->GetNumberOfArrayChannels() );
			fAsicThreshold[mod][asic].resize( set->GetNumberOfArrayChannels() );

			fAsicTime[mod][asic] = config->GetValue( Form( "asic_%d_%d.Time", mod, asic ), 0 );
			fAsicEnabled[mod][asic] = config->GetValue( Form( "asic_%d_%d.Enabled", mod, asic ), true );

			fAsicWalk[mod][asic].resize( 4 );
			for( unsigned int i = 0; i < 4; i++ )
				fAsicWalk[mod][asic][i] = config->GetValue( Form( "asic_%d_%d.Walk%d", mod, asic, i ), 0.0 );

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

		for( unsigned int chan = 0; chan < set->GetNumberOfCAENChannels(); chan++ ){

			fCaenOffset[mod][chan] = config->GetValue( Form( "caen_%d_%d.Offset", mod, chan ), fCaenOffsetDefault );
			fCaenGain[mod][chan] = config->GetValue( Form( "caen_%d_%d.Gain", mod, chan ), fCaenGainDefault );
			fCaenGainQuadr[mod][chan] = config->GetValue( Form( "caen_%d_%d.GainQuadr", mod, chan ), fCaenGainQuadrDefault );
			fCaenThreshold[mod][chan] = config->GetValue( Form( "caen_%d_%d.Threshold", mod, chan ), 0. );
			fCaenTime[mod][chan] = config->GetValue( Form( "caen_%d_%d.Time", mod,  chan ), 0 );

		}
		
	}

	delete config;
	
}

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

unsigned int ISSCalibration::AsicThreshold( unsigned int mod, unsigned int asic, unsigned int chan ) {
	
	if( mod < set->GetNumberOfArrayModules() &&
	   asic < set->GetNumberOfArrayASICs() &&
	   chan < set->GetNumberOfArrayChannels() ) {

		return fAsicThreshold[mod][asic][chan];
		
	}
	
	return -1;
	
}

long ISSCalibration::AsicTime( unsigned int mod, unsigned int asic ){
	
	if( mod < set->GetNumberOfArrayModules() &&
	   asic < set->GetNumberOfArrayASICs() ) {

		return fAsicTime[mod][asic];
		
	}
	
	return 0;
	
}

bool ISSCalibration::AsicEnabled( unsigned int mod, unsigned int asic ){
	
	if( mod < set->GetNumberOfArrayModules() &&
	   asic < set->GetNumberOfArrayASICs() ) {

		return fAsicEnabled[mod][asic];
		
	}
	
	return 0;
	
}

float ISSCalibration::AsicWalk( unsigned int mod, unsigned int asic, float energy ){
	
	float walk = 0;
	
	if( mod < set->GetNumberOfArrayModules() &&
	   asic < set->GetNumberOfArrayASICs() ) {
		
		if( TMath::Abs( fAsicWalk[mod][asic][0] ) < 1.0e-6 &&
		    TMath::Abs( fAsicWalk[mod][asic][1] ) < 1.0e-6 &&
		    TMath::Abs( fAsicWalk[mod][asic][2] ) < 1.0e-6 &&
		    TMath::Abs( fAsicWalk[mod][asic][3] ) < 1.0e-6
		   ) {
			
			walk = 0;
			
		}

		else {
			
			// Params for time walk function ROOT finder
			walk_params[0] = fAsicWalk[mod][asic][0];
			walk_params[1] = fAsicWalk[mod][asic][1];
			walk_params[2] = fAsicWalk[mod][asic][2];
			walk_params[3] = fAsicWalk[mod][asic][3];
			walk_params[4] = energy;

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
				
	}
	
	return walk;
	
}

float ISSCalibration::CaenEnergy( unsigned int mod, unsigned int chan, unsigned short raw ) {
	
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

unsigned int ISSCalibration::CaenThreshold( unsigned int mod, unsigned int chan ) {
	
	if( mod < set->GetNumberOfCAENModules() &&
	   chan < set->GetNumberOfCAENChannels() ) {

		return fCaenThreshold[mod][chan];
		
	}
	
	return -1;
	
}

long ISSCalibration::CaenTime( unsigned int mod, unsigned int chan ){
	
	if( mod < set->GetNumberOfCAENModules() &&
	   chan < set->GetNumberOfCAENChannels() &&
	   mod >= 0 && chan >= 0 ) {

		return fCaenTime[mod][chan];
		
	}
	
	return 0;
	
}

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
					if( fAsicTime[mod][asic] != 0 ) stream << Form( "asic_%d_%d.Time: %ld", mod, asic, fAsicTime[mod][asic] ) << std::endl;
					if( !fAsicEnabled[mod][asic] ) stream << Form( "asic_%d_%d.Enabled: %d", mod, asic, 0 ) << std::endl;
		
					for( unsigned int i = 0; i < 3; i++ )
						if( fAsicWalk[mod][asic][i] > 1e-9 || fAsicWalk[mod][asic][i] < 1e-9 )
							stream << Form( "asic_%d_%d.Walk%d: %f", mod, asic, i, fAsicWalk[mod][asic][i] ) << std::endl;

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

				if( !energy_only && fCaenTime[mod][chan] != 0 )
					stream << Form( "caen_%d_%d.Time: %ld", mod, chan, fCaenTime[mod][chan] ) << std::endl;

			} // chan
			
		} // mod

	} // !asic_only

};
