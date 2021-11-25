#include "Calibration.hh"

Calibration::Calibration( std::string filename, Settings *myset ) {

	SetFile( filename );
	set = myset;
	ReadCalibration();
		
}

Calibration::~Calibration() {

	//std::cout << "destructor" << std::endl;

}

void Calibration::ReadCalibration() {

	TEnv *config = new TEnv( fInputFile.data() );
	
	// ASIC initialisation
	fAsicOffset.resize( set->GetNumberOfArrayModules() );
	fAsicGain.resize( set->GetNumberOfArrayModules() );
	fAsicGainQuadr.resize( set->GetNumberOfArrayModules() );
	fAsicThreshold.resize( set->GetNumberOfArrayModules() );
	fAsicTime.resize( set->GetNumberOfArrayModules() );
	fAsicEnabled.resize( set->GetNumberOfArrayModules() );
	fAsicWalk.resize( set->GetNumberOfArrayModules() );
	
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

			fAsicWalk[mod][asic].resize( 3 );
			for( unsigned int i = 0; i < 3; i++ )
				fAsicWalk[mod][asic][i] = config->GetValue( Form( "asic_%d_%d.Walk%d", mod, asic, i ), 0.0 );

			for( unsigned int chan = 0; chan < set->GetNumberOfArrayChannels(); chan++ ){
				
				fAsicOffset[mod][asic][chan] = config->GetValue( Form( "asic_%d_%d_%d.Offset", mod, asic, chan ), -2936. );
				fAsicGain[mod][asic][chan] = config->GetValue( Form( "asic_%d_%d_%d.Gain", mod, asic, chan ), 14.5 );
				fAsicGainQuadr[mod][asic][chan] = config->GetValue( Form( "asic_%d_%d_%d.GainQuadr", mod, asic, chan ), 0. );
				fAsicThreshold[mod][asic][chan] = config->GetValue( Form( "asic_%d_%d_%d.Threshold", mod, asic, chan ), 0. );

			}
			
		}

	}
	
	
	// CAEN initialisation
	fCaenOffset.resize( set->GetNumberOfCAENModules() );
	fCaenGain.resize( set->GetNumberOfCAENModules() );
	fCaenGainQuadr.resize( set->GetNumberOfCAENModules() );
	fCaenThreshold.resize( set->GetNumberOfCAENModules() );
	fCaenTime.resize( set->GetNumberOfCAENModules() );

	// CAEN parameter read
	for( unsigned int mod = 0; mod < set->GetNumberOfCAENModules(); mod++ ){

		fCaenOffset[mod].resize( set->GetNumberOfCAENChannels() );
		fCaenGain[mod].resize( set->GetNumberOfCAENChannels() );
		fCaenGainQuadr[mod].resize( set->GetNumberOfCAENChannels() );
		fCaenThreshold[mod].resize( set->GetNumberOfCAENChannels() );
		fCaenTime[mod].resize( set->GetNumberOfCAENChannels() );

		for( unsigned int chan = 0; chan < set->GetNumberOfCAENChannels(); chan++ ){

			fCaenOffset[mod][chan] = config->GetValue( Form( "caen_%d_%d.Offset", mod, chan ), 0. );
			fCaenGain[mod][chan] = config->GetValue( Form( "caen_%d_%d.Gain", mod, chan ), 1. );
			fCaenGainQuadr[mod][chan] = config->GetValue( Form( "caen_%d_%d.GainQuadr", mod, chan ), 0. );
			fCaenThreshold[mod][chan] = config->GetValue( Form( "caen_%d_%d.Threshold", mod, chan ), 0. );
			fCaenTime[mod][chan] = config->GetValue( Form( "caen_%d_%d.Time", mod,  chan ), 0 );

		}
		
	}

	delete config;
	
}

float Calibration::AsicEnergy( unsigned int mod, unsigned int asic, unsigned int chan, unsigned short raw ) {
	
	float energy, raw_rand;
	TRandom *fRand = new TRandom();
	
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
	
	delete fRand;
	
	return -1;
	
}

float Calibration::AsicThreshold( unsigned int mod, unsigned int asic, unsigned int chan ) {
	
	if( mod < set->GetNumberOfArrayModules() &&
	   asic < set->GetNumberOfArrayASICs() &&
	   chan < set->GetNumberOfArrayChannels() ) {

		return fAsicThreshold[mod][asic][chan];
		
	}
	
	return -1;
	
}

long Calibration::AsicTime( unsigned int mod, unsigned int asic ){
	
	if( mod < set->GetNumberOfArrayModules() &&
	   asic < set->GetNumberOfArrayASICs() ) {

		return fAsicTime[mod][asic];
		
	}
	
	return 0;
	
}

bool Calibration::AsicEnabled( unsigned int mod, unsigned int asic ){
	
	if( mod < set->GetNumberOfArrayModules() &&
	   asic < set->GetNumberOfArrayASICs() ) {

		return fAsicEnabled[mod][asic];
		
	}
	
	return 0;
	
}

float Calibration::AsicWalk( unsigned int mod, unsigned int asic, float energy ){
	
	float walk = 0;
	
	if( mod < set->GetNumberOfArrayModules() &&
	   asic < set->GetNumberOfArrayASICs() ) {

		// p - q*exp(-r*x)
		walk = TMath::Exp( -1.0 * fAsicWalk[mod][asic][2] * energy );
		walk *= -1.0 * fAsicWalk[mod][asic][1];
		walk += fAsicWalk[mod][asic][0];
		
		return walk;
		
	}
	
	return 0;
	
}

float Calibration::CaenEnergy( unsigned int mod, unsigned int chan, unsigned short raw ) {
	
	float energy, raw_rand;
	TRandom *fRand = new TRandom();
	
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
	
	delete fRand;
	
	return -1;
	
}

float Calibration::CaenThreshold( unsigned int mod, unsigned int chan ) {
	
	if( mod < set->GetNumberOfCAENModules() &&
	   chan < set->GetNumberOfCAENChannels() ) {

		return fCaenThreshold[mod][chan];
		
	}
	
	return -1;
	
}

long Calibration::CaenTime( unsigned int mod, unsigned int chan ){
	
	if( mod < set->GetNumberOfCAENModules() &&
	   chan < set->GetNumberOfCAENChannels() ) {

		return fCaenTime[mod][chan];
		
	}
	
	return 0;
	
}
