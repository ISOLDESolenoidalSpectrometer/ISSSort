#include "Reaction.hh"

ClassImp( Particle )
ClassImp( Reaction )

// Particles
Particle::Particle(){}
Particle::~Particle(){}


double alpha_function( double *x, double *params ){

	double alpha = x[0];
	double z = params[0];
	double rho = params[1];
	double p = params[2];
	double gqb = params[3];
	
	double root = p * TMath::Sin(alpha);
	root -= gqb * rho * TMath::Tan(alpha);
	root -= gqb * z;

	return root;

}


// Reaction things
Reaction::Reaction( std::string filename, Settings *myset ){
		
	// Get the info from the user input
	set = myset;
	SetFile( filename );
	ReadReaction();

	// Root finder algorithm
	fa = new TF1( "alpha_function", alpha_function, 0.0, TMath::Pi()/2.0, 4 );

	
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
	Beam.SetA( config->GetValue( "BeamA", 29.98394686 ) );
	Beam.SetZ( config->GetValue( "BeamZ", 12 ) );

	Eb = config->GetValue( "BeamE", 8520.0 ); // in keV/u
	Eb *= Beam.GetMass_u(); // keV
	Beam.SetEnergyLab( Eb ); // keV
	
	Target.SetA( config->GetValue( "TargetA", 2.0135575247 ) );
	Target.SetZ( config->GetValue( "TargetZ", 1 ) );
	Target.SetEnergyLab( 0.0 );
	
	Ejectile.SetA( config->GetValue( "EjectileA", 1.0072786227 ) );
	Ejectile.SetZ( config->GetValue( "EjectileZ", 1 ) );
	
	Recoil.SetA( config->GetValue( "RecoilA", 30.9901318 ) );
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

void Reaction::MakeReaction( TVector3 vec, double en ){
	
	// Set the input parameters, might use them in another function
	Ejectile.SetEnergyLab(en);	// ejectile energy in keV
	z = vec.Z();			// measured z in mm
	//Ejectile.SetEnergyLab( 4387. );
	//z = -300.;
	rho	= vec.Perp();		// perpenicular distance from beam axis to interaction point
    
    //----------------//
    // EX calculation //
    //----------------//
	params[0] = z;									// z in mm
	params[1] = rho;								// rho in mm
	params[2] = Ejectile.GetMomentumLab();			// p
	params[3] = Ejectile.GetZ() * GetField_corr(); 	// qb
	params[3] /= TMath::TwoPi(); 					// qb/2pi
	params[3] *= Ejectile.GetGamma();				// γ3*qb/2pi

	// RootFinder algorithm
	fa->SetParameters( params );
	ROOT::Math::RootFinder rf( ROOT::Math::RootFinder::kGSL_NEWTON ); // with derivatives
	//ROOT::Math::RootFinder rf( ROOT::Math::RootFinder::kBRENT ); // without derivatives
 	ROOT::Math::GradFunctor1D wf( *fa );
	rf.SetFunction( wf, 0.2 * TMath::Pi() ); // with derivatives
	//rf.SetFunction( wf, 0.0, TMath::PiOver2() ); // without derivatives
	rf.Solve( 500, 1e-5, 1e-6 );
	if( rf.Status() ) alpha = TMath::QuietNaN();
	else alpha = rf.Root();
	
	// Get the real z value at beam axis and lab angle
	if( z < 0 ) z -= rho * TMath::Tan( alpha );
	else z += rho * TMath::Tan( alpha );
	Ejectile.SetThetaLab( TMath::PiOver2() - alpha );

	// Total energy of ejectile in centre of mass
	e3_cm = Ejectile.GetEnergyTotLab();
	e3_cm -= GetBeta() * Ejectile.GetMomentumLab() * TMath::Sin( alpha );
	e3_cm *= GetGamma();
	Ejectile.SetEnergyTotCM( e3_cm );
	Recoil.SetEnergyTotCM( GetEnergyTotCM() - e3_cm );

	// Theta_CM
	theta_cm  = GetGamma() * Ejectile.GetEnergyTotCM();
	theta_cm -= Ejectile.GetEnergyTotLab();
	theta_cm /= GetGamma() * GetBeta() * Ejectile.GetMomentumCM();
	theta_cm  = TMath::ACos( theta_cm );
	Recoil.SetThetaCM( theta_cm );
	Ejectile.SetThetaCM( TMath::Pi() - theta_cm );
	
	// Ex
	Ex  = TMath::Power( GetEnergyTotCM(), 2.0 );
	Ex -= 2.0 * GetEnergyTotCM() * Ejectile.GetEnergyTotCM();
	Ex += TMath::Power( Ejectile.GetMass(), 2.0 );
	Ex  = TMath::Sqrt( Ex ) - Recoil.GetMass();
    Recoil.SetEx( Ex );
    Ejectile.SetEx( 0.0 );

  	
	std::cout << "-----------------------" << std::endl;
	std::cout << "  z_meas = " << vec.Z() << " mm" << std::endl;
	std::cout << "     rho = " << rho << " mm" << std::endl;
	std::cout << "       z = " << z << " mm" << std::endl;
	std::cout << "    beta = " << GetBeta() << std::endl;
	std::cout << "   gamma = " << GetGamma() << std::endl;
	std::cout << "   alpha = " << alpha*TMath::RadToDeg() << " deg" << std::endl;
	std::cout << "    e_cm = " << GetEnergyTotCM()*1e-3 << " MeV" << std::endl;
	std::cout << "   e3_cm = " << e3_cm*1e-3 << " MeV" << std::endl;
	std::cout << "theta_cm = " << theta_cm*TMath::RadToDeg() << " deg" << std::endl;
	std::cout << "      Ex = " << Ex*1e-3 << " MeV" << std::endl;

	
  	return;	

}

