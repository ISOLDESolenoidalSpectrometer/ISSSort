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

//#include "include/Settings.hh"
//#include "include/Reaction.hh"

#include "TVector3.h"
#include "TGraph2D.h"

#include <iostream>

const double radius = 27.0; // mm

std::shared_ptr<ISSSettings> myset;
std::shared_ptr<ISSReaction> myreact;

void z_to_angle_conveter( std::string reaction_file ){

	// Make a reaction instance
	myset = std::make_shared<ISSSettings>( "dummy" );
	myreact = std::make_shared<ISSReaction>( reaction_file, myset, false );

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

double get_theta_cm_debug( double z_det, double E_x ){
	
	// Make a vector for the interaction position
	TVector3 pos( radius, 0.0, z_det );

	// Set excitation energy
	myreact->GetRecoil()->SetEx(E_x);
	
	// Simulate the reaction and get all values
	myreact->SimulateReaction( pos );
	double theta_cm = myreact->GetThetaCM();
	double Qvalue = myreact->GetQvalue();
	double CMenergy_tot = myreact->GetEnergyTotCM();
	double BetaFactor  = myreact->GetBeta();
	double BeamEnergyLab = myreact->GetBeam()->GetEnergyLab();

	double KE_ejectile = myreact->GetEjectile()->GetEnergyLab();
	double theta_ejectile = myreact->GetEjectile()->GetThetaLab();

	double TotalCME_ejectile = myreact->GetEjectile()->GetEnergyTotCM();
	double MomentumCM_ejectile = myreact->GetEjectile()->GetMomentumCM();

	double KE_recoil = myreact->GetRecoil()->GetEnergyLab();
	double theta_recoil = myreact->GetRecoil()->GetThetaLab();

    
	double mass_beam_u = myreact->GetBeam()->GetMass_u();
	double mass_target_u = myreact->GetTarget()->GetMass_u();
    double mass_ejectile_u = myreact->GetEjectile()->GetMass_u();
	double mass_recoil_u = myreact->GetRecoil()->GetMass_u();

	double mass_beam = myreact->GetBeam()->GetMass();
	double mass_target = myreact->GetTarget()->GetMass();
    double mass_ejectile = myreact->GetEjectile()->GetMass();
	double mass_recoil = myreact->GetRecoil()->GetMass();

	std::string name_beam = myreact->GetBeam()->GetIsotope();
	std::string name_target = myreact->GetTarget()->GetIsotope();
	std::string name_ejectile = myreact->GetEjectile()->GetIsotope();
	std::string name_recoil = myreact->GetRecoil()->GetIsotope();

	// Print the results
	std::cout << std::fixed << std::setprecision(5);
	std::cout << "-------------------Reaction info---------------" << std::endl;
    std::cout << "Beam " << name_beam << " mass = " << mass_beam_u << " u = " << mass_beam << " keV/c^2" << std::endl;
	std::cout << "Target  " << name_target << " mass = " << mass_target_u << " u = " << mass_target << " keV/c^2" <<std::endl;
	std::cout << "Ejectile  " << name_ejectile << " mass = " << mass_ejectile_u <<" u = " << mass_ejectile << " keV/c^2" << std::endl;
	std::cout << "Recoil  " << name_recoil << " mass = " << mass_recoil_u << " u = " << mass_recoil << " keV/c^2" << std::endl;
	std::cout << "Beam energy (lab)= " << BeamEnergyLab << " keV" << std::endl;
	std::cout << "z_det = " << z_det << " mm" << ", E_x = " << E_x << " keV";
	std::cout << ", θ_cm = " << theta_cm << " radian = " << theta_cm* TMath::RadToDeg() << " degree" << std::endl;
	std::cout << "Q = " << Qvalue << " keV, total CM energy = " << CMenergy_tot << " keV" << std::endl;
	std::cout << "Beta factor = " << BetaFactor << std::endl;
	std::cout << "-------------------Ejectile info---------------" << std::endl;
	std::cout << "Kinetic energy ejectile lab = " << KE_ejectile << " keV" << ", theta lab ejectile = " << theta_ejectile << " radian" << std::endl;
	std::cout << "Total cm energy ejectile is = " << TotalCME_ejectile << " keV, ejectile cm momentum = " << MomentumCM_ejectile << " keV/c" << std::endl;
	//So far the recoil info is not added in ISSsort in SimulateReaction (so this will give rubbish numbers)
	//std::cout << "-------------------Recoil info-----------------" << std::endl;
	//std::cout << "Kinetic energy recoil lab = " << KE_recoil << " keV" << ", theta lab recoil = " << theta_recoil << " radian" << std::endl;

	return theta_cm;
	
}
