#include "Calibration.hh"

Calibration::Calibration( std::string filename ) {

	SetFile( filename );
	ReadCalibration();
		
}

Calibration::~Calibration() {

	//std::cout << "destructor" << std::endl;

}

void Calibration::ReadCalibration() {

	TEnv *config = new TEnv( fInputFile.data() );
	
	// ASIC initialisation
	fAsicOffset.resize( common::n_module );
	fAsicGain.resize( common::n_module );
	fAsicGainQuadr.resize( common::n_module );
	fAsicThreshold.resize( common::n_module );
	fAsicTime.resize( common::n_module );
	
	// ASIC parameter read
	for( int mod = 0; mod < common::n_module; mod++ ){

		fAsicOffset[mod].resize( common::n_asic );
		fAsicGain[mod].resize( common::n_asic );
		fAsicGainQuadr[mod].resize( common::n_asic );
		fAsicThreshold[mod].resize( common::n_asic );
		fAsicTime[mod].resize( common::n_asic );
		
		for( int asic = 0; asic < common::n_asic; asic++ ){

			fAsicOffset[mod][asic].resize( common::n_channel );
			fAsicGain[mod][asic].resize( common::n_channel );
			fAsicGainQuadr[mod][asic].resize( common::n_channel );
			fAsicThreshold[mod][asic].resize( common::n_channel );

			fAsicTime[mod][asic] = config->GetValue( Form( "asic_%d_%d.Time", mod, asic ), 0. );

			for( int chan = 0; chan < common::n_channel; chan++ ){
				
				fAsicOffset[mod][asic][chan] = config->GetValue( Form( "asic_%d_%d_%d.Offset", mod, asic, chan ), 0. );
				fAsicGain[mod][asic][chan] = config->GetValue( Form( "asic_%d_%d_%d.Gain", mod, asic, chan ), 1. );
				fAsicGainQuadr[mod][asic][chan] = config->GetValue( Form( "asic_%d_%d_%d.GainQuadr", mod, asic, chan ), 0. );
				fAsicThreshold[mod][asic][chan] = config->GetValue( Form( "asic_%d_%d_%d.Threshold", mod, asic, chan ), 0. );

			}
			
		}

	}
	
	
	// CAEN initialisation
	fCaenOffset.resize( common::n_caen_mod );
	fCaenGain.resize( common::n_caen_mod );
	fCaenGainQuadr.resize( common::n_caen_mod );
	fCaenThreshold.resize( common::n_caen_mod );
	fCaenTime.resize( common::n_caen_mod );

	// CAEN parameter read
	for( int mod = 0; mod < common::n_caen_mod; mod++ ){

		fCaenOffset[mod].resize( common::n_caen_ch );
		fCaenGain[mod].resize( common::n_caen_ch );
		fCaenGainQuadr[mod].resize( common::n_caen_ch );
		fCaenThreshold[mod].resize( common::n_caen_ch );

		fCaenTime[mod] = config->GetValue( Form( "caen_%d.Time", mod ), 0. );

		for( int chan = 0; chan < common::n_caen_ch; chan++ ){

			fCaenOffset[mod][chan] = config->GetValue( Form( "caen_%d_%d.Offset", mod, chan ), 0. );
			fCaenGain[mod][chan] = config->GetValue( Form( "caen_%d_%d.Gain", mod, chan ), 1. );
			fCaenGainQuadr[mod][chan] = config->GetValue( Form( "caen_%d_%d.GainQuadr", mod, chan ), 0. );
			fCaenThreshold[mod][chan] = config->GetValue( Form( "caen_%d_%d.Threshold", mod, chan ), 0. );

		}
		
	}

	delete config;
	
}

float Calibration::AsicEnergy( int mod, int asic, int chan, unsigned short raw ) {
	
	float energy, raw_rand;
	TRandom *fRand = new TRandom();
	
	if( mod >= 0 && mod  < common::n_module &&
	   asic >= 0 && asic < common::n_asic &&
	   chan >= 0 && chan < common::n_channel ) {

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



float Calibration::CaenEnergy( int mod, int chan, unsigned short raw ) {
	
	float energy, raw_rand;
	TRandom *fRand = new TRandom();
	
	//std::cout << "mod=" << mod << "; chan=" << chan << std::endl;

	if( mod >= 0 && mod  < common::n_caen_mod &&
	   chan >= 0 && chan < common::n_caen_ch ) {

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

float Calibration::CaenThreshold( int mod, int chan ) {
	
	if( mod >= 0 && mod  < common::n_caen_mod &&
	   chan >= 0 && chan < common::n_caen_ch ) {

		return fCaenThreshold[mod][chan];
		
	}
	
	return -1;
	
}

float Calibration::CaenTime( int mod ){
	
	if( mod >= 0 && mod  < common::n_caen_mod ) {

		return fCaenTime[mod];
		
	}
	
	return 0;
	
}
