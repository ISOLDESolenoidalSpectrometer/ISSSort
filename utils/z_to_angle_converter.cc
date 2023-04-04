// A script to convert from z position to angle.
// Useful when making angular distributions gated by z
// Input is the reaction file, the z distance and the excitation energy

// To run this, do the following:
// root -l
// root [0] gSystem->Load("libiss_sort.so")
// root [1] .L utils/z_to_angle_conveter.cc++
// root [2] z_to_angle_conveter( "reaction.dat" )
// root [3] get_theta_cm( z_det, E_x )  --> in radians
// root [4] get_theta_cm( z_det, E_x ) * TMath::RadToDeg()  --> in degrees

#include "include/Settings.hh"
#include "include/Reaction.hh"

#include "TVector3.h"
#include "TGraph2D.h"

#include <iostream>

const double radius = 27.5; // mm

ISSSettings *myset;
ISSReaction *myreact;

void z_to_angle_conveter( std::string reaction_file ){

	// Make a reaction instance
	myset = new ISSSettings( "dummy" );
	myreact = new ISSReaction( reaction_file, myset, false );

}

double get_theta_cm( double z_det, double E_x ){
	
	// Make a vector for the interaction position
	TVector3 pos( radius, 0.0, z_det );

	// Set excitation energy
	myreact->GetRecoil()->SetEx(E_x);
	
	// Get theta_cm
	myreact->SimulateReaction( pos );
	double theta_cm = myreact->GetThetaCM();
	
	// Print the result
	std::cout << "z_det = " << z_det << ", E_x = " << E_x;
	std::cout << ", θ_cm = " << theta_cm << std::endl;

	return theta_cm;
	
}
