#include "Reaction.hh"

ClassImp( Particle )
ClassImp( Reaction )

// Particles
Particle::Particle(){}
Particle::~Particle(){}


double newton_function( double *x, double *params ){

	double alpha = x[0];
	double z = params[0];
	double rho = params[1];
	double p = params[2];
	double gqb = params[3] / TMath::Pi() / 2.0;
	
	double root = gqb * z;
	root += p * TMath::Sin(alpha);
	root -= gqb * rho * TMath::Tan(alpha);

	return root;

}


// Reaction things
Reaction::Reaction( std::string filename, Settings *myset ){
		
	// Get the info from the user input
	set = myset;
	SetFile( filename );
	ReadReaction();
	
}
Reaction::~Reaction(){

	for( unsigned int i = 0; i < recoil_cut.size(); ++i )
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
	ncuts = set->GetNumberOfRecoilSectors();
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
    // double alpha;
    
  	// The Lorentz factor, gamma
	//double beta = TMath::Power( Beam.GetEnergyLab() + Beam.GetMass(), 2.0 ) - TMath::Power( Beam.GetMass(), 2.0 );
	//beta = TMath::Sqrt( beta ) / ( Beam.GetMass() + Target.GetMass() + Beam.GetEnergyLab() );
	//double gamma = 1.0 / TMath::Sqrt( 1.0 - TMath::Power( beta, 2.0 ) );
	
	

	// newton_pars[4];
    //newton_pars[0] = ejec_vec.Z();
    //newton_pars[1] = ejec_vec.Perp();
   // newton_pars[2] = TMath::Sqrt( TMath::Power( en, 2.0 ) - TMath::Power( Ejectile.GetMass(), 2.0 ) );
    //newton_pars[3] = beta * gamma * GetField() / ( TMath::Pi() * 2.0 );
   // 
    //TF1 *f = new TF1( "newton_function", newton_function, 0.0, TMath::Pi()/2.0 ); 
	//ROOT::Math::Roots::Newton rf();
   // ROOT::Math::IGenFunction wf( &f );
   // rf.SetFunction(wf,alpha,newton_pars);
 //   rf.Solve(100,1e-5,1e-6);
  // std::cout << rf.Root() << std::endl;
    
    // Total energy in the laboratory frame
	double etot_lab = Eb + Beam.GetMass() + Target.GetMass();
    
    // Total energy in the centre of mass
	double etot_cm = TMath::Power( Beam.GetMass(), 2.0 );
	etot_cm += TMath::Power( Target.GetMass(), 2.0 );
	etot_cm += 2.0 * ( Eb + Beam.GetMass() ) * Target.GetMass();
	etot_cm = TMath::Sqrt( etot_cm );
	
    double gamma = etot_lab/etot_cm;
    double beta = TMath::Sqrt(1-1/TMath::Power(gamma,2.0));
 	
    
    
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
	double Z = Ejectile.GetZ() * GetField_corr() / (2.0*TMath::Pi()) * gamma * ejec_vec.Z() * beta;
	double H = TMath::Sqrt(TMath::Power(gamma*beta,2.0)) * (TMath::Power( (Ejectile.GetEnergyLab() + Ejectile.GetMass()), 2.0 ) - TMath::Power( Ejectile.GetMass(), 2.0 ));
	double e3_cm;
	
	int iter = 0;

    // fa is a function of alpha
    // dfa is the derivative of this function
    // where alpha is the scattering angle in the lab frame
    // and nalpha is a iterative parameter to look for convergence
    //if(TMath::Abs(Z) <H) {
    //	do {
    //
	//	alpha = nalpha;
	//	
	//	fa  = TMath::Power(( Ejectile.GetEnergyLab() + Ejectile.GetMass()), 2.0 ) - TMath::Power( Ejectile.GetMass(), 2.0 );
	//	fa *= TMath::Power( gamma * beta, 2.0 );
	//	fa  = TMath::Sqrt( fa )* TMath::Sin(alpha);
	//	fa -= gamma * beta * Ejectile.GetZ() * GetField_corr() / ( 2.0*TMath::Pi() ) * rho * TMath::Tan(alpha);
	//	fa -= gamma * beta * Ejectile.GetZ() * GetField_corr() / ( 2.0*TMath::Pi() ) * ejec_vec.Z();
	//       
	//	dfa  = TMath::Power( Ejectile.GetEnergyLab() + Ejectile.GetMass(), 2.0 ) - TMath::Power( Ejectile.GetMass(), 2.0 );
	//	dfa *= TMath::Power( gamma * beta, 2.0 );
	//	dfa  = TMath::Sqrt( fa )* TMath::Cos(alpha) ;
	//	dfa -= gamma * beta * Ejectile.GetZ() * GetField_corr() / ( 2.0*TMath::Pi() ) * rho / TMath::Power( TMath::Cos(alpha), 2.0 );
	//
	//	nalpha = alpha - fa/dfa;
	//	
	//	iter++;
	//	
	//	if( iter > 100 || TMath::Abs( nalpha ) > TMath::PiOver2() ) break;
	//	
	//} while( TMath::Abs( alpha - nalpha ) > 0.001 );
	//alpha=nalpha;


    
    // Check that the derivative is positive and alpha is less than 180 deg.
    // e3_cm is then the energy of the ejectile in the centre of mass frame
    // Ex is the excitation energy of the recoil
    // theta_cm is the angle of the ejectile in the centre of mass frame
	//if( dfa > 0 && TMath::Abs(alpha) < TMath::PiOver2() ) {
	//
	//	K  = TMath::Power( Ejectile.GetEnergyLab() + Ejectile.GetMass(), 2.0 ) - TMath::Power( Ejectile.GetMass(), 2.0 );
	//	K *= TMath::Power( gamma * beta, 2.0 );
	//	K *= TMath::Sin(alpha);
	//	
	//	X = TMath::ACos(Ejectile.GetMass() / (((Ejectile.GetEnergyLab() + Ejectile.GetMass()) * gamma) - K));
	//	
	//	P =  Ejectile.GetMass() * TMath::Tan(X);
	//	
	//	Ex = TMath::Power( Ejectile.GetMass(), 2.0) + TMath::Power( etot_cm, 2.0 );
	//	Ex -= 2.0 * etot_cm * TMath::Sqrt(TMath::Power( P, 2.0) + TMath::Power( Ejectile.GetMass(), 2.0));
	//	Ex = TMath::Sqrt(Ex);
	//	Ex -= Recoil.GetMass();
		
		
		
    	//e3_cm  = TMath::Power( Ejectile.GetEnergyLab() + Ejectile.GetMass(), 2.0 ) - TMath::Power( Ejectile.GetMass(), 2.0 );
    	//e3_cm  = TMath::Sqrt( e3_cm ) * TMath::Sin(alpha) * beta;
  		//e3_cm  = gamma * ( Ejectile.GetEnergyLab() - e3_cm );
		
		//Ex =  TMath::Power( etot_cm, 2.0 );
		//Ex -= 2.0 * etot_cm * e3_cm;
		//Ex += TMath::Power( Ejectile.GetMass(), 2.0 );
		//Ex =  TMath::Sqrt( Ex );
		//Ex -= Recoil.GetMass();
		
	

	//	theta_cm  = TMath::Sqrt( TMath::Power( P , 2.0 ) + TMath::Power( Ejectile.GetMass(), 2.0 ) );
	//	theta_cm  = gamma * theta_cm - Ejectile.GetEnergyLab() - Ejectile.GetMass()  ;
	//	theta_cm /= gamma * beta * P;
	//	theta_cm  = TMath::ACos( theta_cm );

	//}
	
   
	// Cannot be solved, so just throw a quiet NaN
	//else {
	// 
	//	Ex = TMath::QuietNaN();
	//    theta_cm = TMath::QuietNaN();
	//    
	//}
    
	//} else {
	 
	//	Ex = TMath::QuietNaN();
	//	theta_cm = TMath::QuietNaN();
	//
    //}
    
 	//std::cout << "e3_cm = " << e3_cm << std::endl;
 	//std::cout << "en = " << en << std::endl;
 	//std::cout << "z = " << ejec_vec.Z() << std::endl;
	//std::cout << "rho = " << rho << std::endl;
 	//std::cout << "alpha = " << alpha << std::endl;
 	//std::cout << "m3 = " << Ejectile.GetMass() << std::endl;
 	//std::cout << "Bcorr = " << GetField_corr() << std::endl;
	//std::cout << "Ex = " << Ex << std::endl;
	//std::cout << "theta_cm = " << theta_cm << std::endl;
	//std::cout << "Ep = " << Ejectile.GetEnergyLab() << std::endl;
	//std::cout << "beta = " << beta << std::endl; CORRECT
	//std::cout << "gamma = " << gamma << std::endl; CORRECT
	//std::cout << "etot_cm = " << etot_cm << std::endl; CORRECT
	
	
	// A dirty way to calculate Ex assuming an infinitely small detector
	    e3_cm  = Ejectile.GetEnergyLab() + Ejectile.GetMass() ;
    	e3_cm  -= beta * GetField_corr() / TMath::TwoPi() * ejec_vec.Z() ;
  		e3_cm  = gamma * e3_cm;
		
		Ex = TMath::Power(etot_cm, 2.0) + TMath::Power( Ejectile.GetMass(), 2.0);
		Ex -= 2 * etot_cm* e3_cm;
		Ex = TMath::Sqrt(Ex);
		Ex -= Recoil.GetMass();
		
	//std::cout << "Ex = " << Ex << std::endl;
	//std::cout << "m1 = " << Beam.GetMass() << std::endl;
	//std::cout << "m2 = " << Target.GetMass() << std::endl;
	//std::cout << "m3 = " << Ejectile.GetMass() << std::endl;
	//std::cout << "m4 = " << Recoil.GetMass() << std::endl;
	//std::cout << "Beam energy" << Eb << std::endl;
   // std::cout << "beta = " << beta << std::endl; 
	//std::cout << "gamma = " << gamma << std::endl; 
	//std::cout << "etot_cm = " << etot_cm << std::endl; 
	
    Recoil.SetEx( Ex );
	//Ejectile.SetThetaCM( theta_cm );
	// Ejectile.SetEnergyCM( e3_cm );
	//Recoil.SetThetaCM( 2.0*TMath::Pi() - theta_cm );
  	
  	return;	

}

