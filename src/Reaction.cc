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
	ejec_vec = vec;			// 3-vector of detected ejectile
	ejec_en = en;			// ejectile energy in keV
	rho	= ejec_vec.Perp();	// perpenicular distance from beam axis to interaction point
	CalculateZ();			// get the real z value at beam axis
	
	// Do you funky maths here
	// If you need the magnetic field, use Mfield
	// Beam energy is available as Eb, but also Beam.GetEnergyLab()
	// To get the particle info use things like:
	// Recoil.GetThetaCM();
	// Ejectile.GetEnergyCM();
	// Recoil.SetEx( 312.5 ); // unit of keV and mm
	// The atomic mass unit is defined as u_mass.
	// Eventually we may be able to use the mass tables.
	// In the main code we only need the Reaction class, so all
	// values you might need should have "shortcuts" in the .hh file
	// e.g. inline float GetEx(){ return Recoil.GetEx(); }; to return the
	// excitation energy of the recoiling nucleus.

	
	

}

