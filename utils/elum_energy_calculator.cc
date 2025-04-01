// A script to calculate the alpha energy in the ELUM
// Input is the reaction file for ELUM geometry

// To run this, do the following:
// root -l
// root [0] gSystem->Load("libiss_sort")
// root [1] .L utils/elum_energy_calculator.cc
// root [2] elum_energy_calculator

//#include "include/Settings.hh"
//#include "include/Reaction.hh"

#include "TVector3.h"
#include <iostream>

std::shared_ptr<ISSSettings> myset;
std::shared_ptr<ISSReaction> myreact;

void elum_energy_calculator( std::string reaction_file ){

	// Make a reaction instance
	myset = std::make_shared<ISSSettings>( "dummy" );
	myreact = std::make_shared<ISSReaction>( reaction_file, myset, true );

	// Centre of the ELUM
	double elum_z = myreact->GetELUMDistance();
	double elum_rin = myreact->GetELUMInnerRadius();
	double elum_rout = myreact->GetELUMOuterRadius();
	TVector3 elum_hit( 0.5*(elum_rout+elum_rin), 0.0, elum_z );

	// Energies of all the alphas
	std::vector<double> alphas;
	alphas.push_back( 3182.69 );
	alphas.push_back( 5148.31 );
	alphas.push_back( 5478.62 );
	alphas.push_back( 5795.04 );

	// Loop over all alphas
	std::cout << std::endl << "ELUM at " << elum_z << " mm" << std::endl;
	std::cout << "Alpha energy\tEnergy loss" << std::endl;
	for( unsigned int i = 0; i < alphas.size(); i++ ) {

		double det_en = myreact->SimulateDecay( elum_hit, alphas[i], 1 );
		double enloss = alphas[i] - det_en;
		std::cout << alphas[i] << " keV \t" << enloss << " keV" << std::endl;

	}

}
