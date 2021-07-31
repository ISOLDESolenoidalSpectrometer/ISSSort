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
Reaction::~Reaction(){}

void Reaction::ReadReaction() {

	TEnv *config = new TEnv( fInputFile.data() );
	
	// Magnetic field stuff
	Mfield = config->GetValue( "MField", 2.0 );
	
	// Get particle properties
	Eb = config->GetValue( "BeamE", 8.5 );
	
	Beam.SetEnergyLab( Eb );
	Beam.SetMass( config->GetValue( "BeamA", 30. ) );
	Beam.SetZ( config->GetValue( "BeamZ", 12 ) );
	
	Target.SetEnergyLab( 0.0 );
	Target.SetMass( config->GetValue( "TargetA", 2. ) );
	Target.SetZ( config->GetValue( "TargetZ", 1 ) );
	
	Ejectile.SetMass( config->GetValue( "EjectileA", 1. ) );
	Ejectile.SetZ( config->GetValue( "EjectileZ", 1 ) );
	
	Recoil.SetMass( config->GetValue( "RecoilA", 31. ) );
	Recoil.SetZ( config->GetValue( "RecoilZ", 12 ) );
	
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
    
    
    // e3_cm calculation requires calcualtion of lab angle alpha using Newtons method
    double alpha = 0;
    double nalpha = 0;
    double fa = 0;
    double dfa = 0;
    double Ex = 0;
    double theta_cm = 0;

    // fa is a function of alpha
    // dfa is the derivative of this function
    // where alpha is the scattering angle in the lab frame
    // and nalpha is a iterative parameter to look for convergence
    while( fa > 10e-5 ) {
    
		alpha = nalpha;
		
		fa  = TMath::Sqrt( TMath::Power( Ejectile.GetEnergyLab(), 2.0 ) - TMath::Power( Ejectile.GetMass(), 2.0 ) );
		fa *= TMath::Sin(alpha);
		fa -= gamma * Ejectile.GetZ() * GetField() / ( 2.0*TMath::Pi() ) * rho * TMath::Tan(alpha);
		fa += gamma * Ejectile.GetZ() * GetField() / ( 2.0*TMath::Pi() ) * ejec_vec.Z();
            
		dfa =  TMath::Sqrt( TMath::Power( Ejectile.GetEnergyLab(), 2.0 ) - TMath::Power( Ejectile.GetMass(), 2.0 ) );
		dfa *= TMath::Cos(alpha);
		dfa -= gamma * Ejectile.GetZ() * GetField() / ( 2.0*TMath::Pi() ) * rho / TMath::Cos(alpha);

		nalpha = alpha - fa/dfa;
            
	}
    
    // Check that the derivative is positive and alpha is less than 180 deg.
    // e3_cm is then the energy of the ejectile in the centre of mass frame
    // Ex is the excitation energy of the recoil
    // theta_cm is the angle of the ejectile in the centre of mass frame
	if( dfa > 0 && TMath::Abs(alpha) < TMath::Pi() / 2.0 ) {

    	e3_cm  = TMath::Power( Ejectile.GetEnergyLab(), 2.0 );
    	e3_cm -= TMath::Power( Ejectile.GetMass(), 2.0 ) * TMath::Sin(alpha);
    	e3_cm  = beta * TMath::Sqrt( e3_cm );
		e3_cm  = gamma * ( Ejectile.GetEnergyLab() - e3_cm );
		
		Ex =  TMath::Power( etot_lab, 2.0 );
		Ex -= 2.0 * etot_cm * e3_cm;
		Ex += TMath::Power( Ejectile.GetMass(), 2.0 );
		Ex =  TMath::Sqrt( Ex );
		Ex -= Recoil.GetMass();

		theta_cm  = TMath::Sqrt( TMath::Power( etot_cm, 2.0 ) - TMath::Power( Ejectile.GetMass(), 2.0 ) );
		theta_cm  = Ejectile.GetEnergyLab() / gamma / beta / theta_cm;
		theta_cm  = gamma * e3_cm - theta_cm;
		theta_cm  = TMath::Pi() - TMath::ACos( theta_cm );

	}
    
    // Cannot be solved, so just throw a quiet NaN
    else {
    
		Ex = TMath::QuietNaN();
        theta_cm = TMath::QuietNaN();
        
    }
    
    Recoil.SetEx( Ex );
	Ejectile.SetThetaCM( theta_cm );
	Ejectile.SetEnergyCM( e3_cm );
	Recoil.SetThetaCM( 2.0*TMath::Pi() - theta_cm );
  	
  	return;	

}

