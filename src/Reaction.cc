#include "Reaction.hh"

ClassImp( Particle )
ClassImp( Reaction )

// Particles
Particle::Particle(){}
Particle::~Particle(){}

// Reaction things
Reaction::Reaction( std::string filename ){
		
	// Get the info from the user input
	SetFile( filename );
	ReadReaction();
	
}
Reaction::~Reaction(){

	for( unsigned int i; i < recoil_cut.size(); ++i )
		delete (recoil_cut[i]);
	recoil_cut.clear();
	//recoil_file->Close();

}

void Reaction::ReadReaction() {

	TEnv *config = new TEnv( fInputFile.data() );
	
	// Magnetic field stuff
	Mfield = config->GetValue( "Mfield", 2.0 );
	
	// Get particle properties
	Beam.SetA( config->GetValue( "BeamA", 30. ) );
	Beam.SetZ( config->GetValue( "BeamZ", 12 ) );

	Eb = config->GetValue( "BeamE", 8520.0 ); // in keV/u
	Eb *= Beam.GetMass_u(); // keV
	Beam.SetEnergyLab( Eb ); // keV
	
	Target.SetA( config->GetValue( "TargetA", 2. ) );
	Target.SetZ( config->GetValue( "TargetZ", 1 ) );
	Target.SetEnergyLab( 0.0 );
	
	Ejectile.SetA( config->GetValue( "EjectileA", 1. ) );
	Ejectile.SetZ( config->GetValue( "EjectileZ", 1 ) );
	
	Recoil.SetA( config->GetValue( "RecoilA", 31. ) );
	Recoil.SetZ( config->GetValue( "RecoilZ", 12 ) );
	
	// Get recoil energy cut
	ncuts = config->GetValue( "NumberOfRecoilCuts", 4 );
	recoil_cut.resize( ncuts );
	cutfile.resize( ncuts );
	cutname.resize( ncuts );
	for( unsigned int i = 0; i < ncuts; ++i ) {
	
		cutfile.at(i) = config->GetValue( Form( "RecoilCut_%d.File", i ), "NULL" );
		cutname.at(i) = config->GetValue( Form( "RecoilCut_%d.Name", i ), "CUTG" );
		
		// Check if it is given by the user
		if( cutfile.at(i) != "NULL" ) {
		
			recoil_file = new TFile( cutfile.at(i).data(), "READ" );
			if( recoil_file->IsZombie() )
				std::cout << "Couldn't open " << cutfile.at(i) << " correctly" << std::endl;
				
			else {
			
				if( !recoil_file->GetListOfKeys()->Contains( cutname.at(i).data() ) )
					std::cout << "Couldn't find " << cutname.at(i) << " in " << cutfile.at(i) << std::endl;
				else
					recoil_cut.at(i) = (TCutG*)recoil_file->Get( cutname.at(i).data() )->Clone();

			}
			
			recoil_file->Close();
			
		}
		
		// Assign an empty cut file if none is given, so the code doesn't crash
		if( !recoil_cut.at(i) ) recoil_cut.at(i) = new TCutG();
	
	}
	
	// Finished
	delete config;

}

void Reaction::CalculateZ(){
	
	// Sharpy's projection from detector to axis of z
	z = ejec_vec.Z();
	
	
}

void Reaction::MakeReaction( TVector3 vec, float en ){
	
	// Set the input parameters, might use them in another function
	ejec_vec = vec;				// 3-vector of detected ejectile
	Ejectile.SetEnergyLab(en);	// ejectile energy in keV
	rho	= ejec_vec.Perp();		// perpenicular distance from beam axis to interaction point
	CalculateZ();				// get the real z value at beam axis
    
    ////////////////////
    // EX calculation //
    ////////////////////
    
    // Total energy in the laboratory frame
	double etot_lab = Eb + Beam.GetMass() + Target.GetMass();
    
    // Total energy in the centre of mass
	double etot_cm = TMath::Power( Beam.GetMass(), 2.0 );
	etot_cm += TMath::Power( Target.GetMass(), 2.0 );
	etot_cm += 2.0 * ( Eb + Beam.GetMass() ) * Target.GetMass();
	etot_cm = TMath::Sqrt( etot_cm );
 	
 	// The Lorentz factor, gamma
	double gamma = etot_lab / etot_cm;
	double beta = TMath::Sqrt( 1.0 - 1.0 / TMath::Power( gamma, 2.0 ) );
	double e3_cm = 0;
    
    
    // e3_cm calculation requires calcualtion of lab angle alpha using Newton's method
	double nalpha = 0;
	double alpha = 0;
	double fa;
	double dfa;
	double Ex;
	double theta_cm;
	double K;
	double X;
	double P;
	double Z = Ejectile.GetZ() * GetField_corr() / 2.0*TMath::Pi() * gamma *ejec_vec.Z() * beta;
	double H = TMath::Sqrt(TMath::Power(gamma*beta,2.0)) * (TMath::Power( Ejectile.GetEnergyLab() + Ejectile.GetMass(), 2.0 ) - TMath::Power( Ejectile.GetMass(), 2.0 ));
	
	int iter = 0;

    // fa is a function of alpha
    // dfa is the derivative of this function
    // where alpha is the scattering angle in the lab frame
    // and nalpha is a iterative parameter to look for convergence
    if(TMath::Abs(Z) <H) {
    	do {
    
		alpha = nalpha;
		
		fa  = TMath::Power( Ejectile.GetEnergyLab() + Ejectile.GetMass(), 2.0 ) - TMath::Power( Ejectile.GetMass(), 2.0 );
		fa *= TMath::Power( gamma * beta, 2.0 );
		fa  = TMath::Sqrt( TMath::Sin(alpha) * fa );
		fa -= gamma * beta * Ejectile.GetZ() * GetField_corr() / ( 2.0*TMath::Pi() ) * rho * TMath::Tan(alpha);
		fa -= gamma * beta * Ejectile.GetZ() * GetField_corr() / ( 2.0*TMath::Pi() ) * ejec_vec.Z();
            
		dfa  = TMath::Power( Ejectile.GetEnergyLab() + Ejectile.GetMass(), 2.0 ) - TMath::Power( Ejectile.GetMass(), 2.0 );
		dfa *= TMath::Power( gamma * beta, 2.0 );
		dfa  = TMath::Sqrt( TMath::Cos(alpha) * fa );
		dfa -= gamma * beta * Ejectile.GetZ() * GetField_corr() / ( 2.0*TMath::Pi() ) * rho / TMath::Power( TMath::Cos(alpha), 2.0 );

		nalpha = alpha - fa/dfa;
		
		iter++;
		
		if( iter > 100 || TMath::Abs( nalpha ) > TMath::PiOver2() ) break;
		
		}while( TMath::Abs( alpha - nalpha ) > 1e-4 );
		alpha = nalpha;
		
		dfa  = TMath::Power( Ejectile.GetEnergyLab() + Ejectile.GetMass(), 2.0 ) - TMath::Power( Ejectile.GetMass(), 2.0 );
		dfa *= TMath::Power( gamma * beta, 2.0 );
		dfa  = TMath::Sqrt( TMath::Cos(alpha) * fa );
		dfa -= gamma * beta * Ejectile.GetZ() * GetField_corr() / ( 2.0*TMath::Pi() ) * rho / TMath::Power( TMath::Cos(alpha), 2.0 );
	
	
	//std::cout << "en = " << en << std::endl;
 	//std::cout << "z = " << ejec_vec.Z() << std::endl;
	//std::cout << "rho = " << rho << std::endl;
 	//std::cout << "alpha = " << alpha << std::endl;
    
    // Check that the derivative is positive and alpha is less than 180 deg.
    // e3_cm is then the energy of the ejectile in the centre of mass frame
    // Ex is the excitation energy of the recoil
    // theta_cm is the angle of the ejectile in the centre of mass frame
	if( dfa > 0 && TMath::Abs(alpha) < TMath::PiOver2() ) {

		K  = TMath::Power( Ejectile.GetEnergyLab() + Ejectile.GetMass(), 2.0 ) - TMath::Power( Ejectile.GetMass(), 2.0 );
		K *= TMath::Power( gamma * beta, 2.0 );
		K *= TMath::Sin(alpha);
		
		X = Ejectile.GetMass() / (((Ejectile.GetEnergyLab() + Ejectile.GetMass()) * gamma) - K);
		
		P =  Ejectile.GetMass() * TMath::Tan(X);
		
		Ex = TMath::Power( Ejectile.GetMass(), 2.0) + TMath::Power( etot_cm, 2.0 );
		Ex -= 2.0 * etot_cm * TMath::Sqrt(TMath::Power( P, 2.0) + TMath::Power( Ejectile.GetMass(), 2.0));
		Ex = TMath::Sqrt(Ex);
		Ex -= Recoil.GetMass();
		
		
		
    	//e3_cm  = TMath::Power( Ejectile.GetEnergyLab() + Ejectile.GetMass(), 2.0 ) - TMath::Power( Ejectile.GetMass(), 2.0 );
    	//e3_cm  = TMath::Sqrt( e3_cm ) * TMath::Sin(alpha) * beta;
  		//e3_cm  = gamma * ( Ejectile.GetEnergyLab() - e3_cm );
		
		//Ex =  TMath::Power( etot_cm, 2.0 );
		//Ex -= 2.0 * etot_cm * e3_cm;
		//Ex += TMath::Power( Ejectile.GetMass(), 2.0 );
		//Ex =  TMath::Sqrt( Ex );
		//Ex -= Recoil.GetMass();
		
	

		theta_cm  = TMath::Sqrt( TMath::Power( P , 2.0 ) + TMath::Power( Ejectile.GetMass(), 2.0 ) );
		theta_cm  = gamma * theta_cm - Ejectile.GetEnergyLab() - Ejectile.GetMass()  ;
		theta_cm /= gamma * beta * P;
		theta_cm  = TMath::ACos( theta_cm );

	}
	
   
    // Cannot be solved, so just throw a quiet NaN
    else {
    
		Ex = TMath::QuietNaN();
        theta_cm = TMath::QuietNaN();
        
    }
    
   } else {
    
		Ex = TMath::QuietNaN();
        theta_cm = TMath::QuietNaN();
        
    }
    
 	//std::cout << "e3_cm = " << e3_cm << std::endl;
	std::cout << "Ex = " << Ex << std::endl;
	std::cout << "theta_cm = " << theta_cm << std::endl;
	
    Recoil.SetEx( Ex );
	Ejectile.SetThetaCM( theta_cm );
	//Ejectile.SetEnergyCM( e3_cm );
	Recoil.SetThetaCM( 2.0*TMath::Pi() - theta_cm );
  	
  	return;	

}

