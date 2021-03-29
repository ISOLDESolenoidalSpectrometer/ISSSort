#include "Calibration.hh"

Calibration::Calibration( std::string filename ) {

	SetFile( filename );
	ReadCalibration();
	fRand = new TRandom();
		
}

Calibration::~Calibration() {

	//std::cout << "destructor" << std::endl;

}

void Calibration::ReadCalibration() {

	TEnv *config = new TEnv( fInputFile.data() );
	fAsicOffset.resize( common::n_module );
	fAsicGain.resize( common::n_module );
	fAsicGainQuadr.resize( common::n_module );
	fAsicTime.resize( common::n_module );

	for( int mod = 0; mod < common::n_module; mod++ ){

		fAsicOffset[mod].resize( common::n_asic );
		fAsicGain[mod].resize( common::n_asic );
		fAsicGainQuadr[mod].resize( common::n_asic );
		fAsicTime[mod].resize( common::n_asic );
		
		for( int asic = 0; asic < common::n_asic; asic++ ){

			fAsicOffset[mod][asic].resize( common::n_channel );
			fAsicGain[mod][asic].resize( common::n_channel );
			fAsicGainQuadr[mod][asic].resize( common::n_channel );

			fAsicTime[mod][asic] = config->GetValue( Form( "asic_%d_%d.Time", mod, asic ), 0. );

			for( int chan = 0; chan < common::n_channel; chan++ ){
				
				fAsicOffset[mod][asic][chan] = config->GetValue( Form( "asic_%d_%d_%d.Offset", mod, asic, chan ), -2500.0 );
				fAsicGain[mod][asic][chan] = config->GetValue( Form( "asic_%d_%d_%d.Gain", mod, asic, chan ), 12.5 );
				fAsicGainQuadr[mod][asic][chan] = config->GetValue( Form( "asic_%d_%d_%d.GainQuadr", mod, asic, chan ), 0.0 );
				
			}
			
		}

	}

	delete config;
	
}

float Calibration::AsicEnergy( int mod, int asic, int chan, unsigned short raw ) {
	
	float energy, raw_rand;
	
	if( mod >= 0 && mod  < common::n_module &&
	   asic >= 0 && asic < common::n_asic &&
	   chan >= 0 && chan < common::n_channel ) {

		raw_rand = raw + 0.5 - fRand->Uniform();

		energy = fAsicGainQuadr[mod][asic][chan] * raw_rand * raw_rand;
		energy += fAsicGain[mod][asic][chan] * raw_rand;
		energy += fAsicOffset[mod][asic][chan];

		// Check if we have defaults 1 and 0 for ADC channels instead of energy
		if( TMath::Abs( fAsicOffset[mod][asic][chan] ) < 1e-6 &&
		    TMath::Abs( fAsicGain[mod][asic][chan] - 1.0 ) < 1e-6 &&
		    TMath::Abs( fAsicGainQuadr[mod][asic][chan] ) < 1e-6 )
			
			return raw;
		
		else return energy;
		
	}
	
	return -1;
	
}

float Calibration::AsicThreshold( int mod, int asic, int chan ) {
	
	if( mod >= 0 && mod  < common::n_module &&
	   asic >= 0 && asic < common::n_asic &&
	   chan >= 0 && chan < common::n_channel ) {

		return fAsicThreshold[mod][asic][chan];
		
	}
	
	return -1;
	
}

float Calibration::AsicTime( int mod, int asic ){
	
	if( mod >= 0 && mod  < common::n_module &&
	   asic >= 0 && asic < common::n_asic ) {

		return fAsicTime[mod][asic];
		
	}
	
	return 0;
	
}
