// A script to convert from z position to angle.
// Useful when making angular distributions gated by z
// Input is the reaction file, the z distance and the excitation energy

// To run this, do the following:
// root -l
// root [0] gSystem->Load("libiss_sort.so")
// root [1] .L utils/z_to_angle_simulator.cc++
// root [2] z_to_angle_simulator( "reaction.dat" )
// root [3] get_theta_cm( z_det, E_x )  --> in radians
// root [4] get_theta_cm( z_det, E_x ) * TMath::RadToDeg()  --> in degrees

//#include "include/Settings.hh"
//#include "include/Reaction.hh"

#include "TVector3.h"
#include "TGraph2D.h"

#include <iostream>

const unsigned int ncalcs = 10;
const double min_energy = 200; // keV
const double max_energy = 12200; // keV
const double min_z = 0.0; // mm
const double max_z = 510.0; // mm
const double radius = 27.5; // mm

std::shared_ptr<ISSSettings> myset;
std::shared_ptr<ISSReaction> myreact;

TGraph2D *g;

void z_to_angle_simulator( std::string reaction_file ){

	// Make a reaction instance
	myset = std::make_shared<ISSSettings>( "dummy" );
	myreact = std::make_shared<ISSReaction>( reaction_file, myset, false );

	// Array distance
	double z_0 = myreact->GetArrayDistance();

	// A graph of Ex versus ThetaCM for this z value
	g = new TGraph2D();
	g->SetTitle("ThetaCM as a function of z_meas and Ex;z_meas [mm];E_{x} [keV]");

	// Do the reaction for a range of proton energies
	for( unsigned int i = 0; i < ncalcs+1; ++i ){
		
		// Calculate the proton energy
		double en = min_energy + ( max_energy - min_energy ) / ncalcs * (double)i;
		
		// Do the reaction for a range of z positions
		for( unsigned int j = 0; j < ncalcs+1; ++j ){
			
			// Calculate the z position
			double z_det = min_z + ( max_z - min_z ) / ncalcs * (double)j;
			double z_meas = z_det + z_0; // forward
			if( z_0 < 0 ) z_meas = z_0 - z_det; // backwards

			// Make a vector for the interaction position
			TVector3 pos( radius, 0.0, z_det );

			// Make the reaction (this should be actually the simulation function)
			myreact->MakeReaction( pos, en );
			
			std::cout << en << "\t" << z_det << "\t";
			std::cout << z_0 << "\t" << z_meas << "\t";
			std::cout << myreact->GetDistance() << "\t";
			std::cout << myreact->GetEx() << "\t";
			std::cout << myreact->GetThetaCM() << std::endl;
			
			// Common sense checks
			if( TMath::IsNaN( myreact->GetEx() ) || TMath::IsNaN( myreact->GetThetaCM() ) ) continue;
			if( myreact->GetEx() < -100 || myreact->GetEx() > 1e6 ) continue;

			// Fill graph
			g->SetPoint( g->GetN(), z_meas, myreact->GetEx(), myreact->GetThetaCM() );

		}
		
	}
	
	// Draw the graph
	g->Draw("colz");
	
}

double get_theta_cm( double z_det, double E_x ){
	
	double theta_cm = g->Interpolate( z_det, E_x );
	
	// Print the result
	std::cout << "z_det = " << z_det << ", E_x = " << E_x;
	std::cout << ", θ_cm = " << theta_cm << std::endl;

	return theta_cm;
	
}
