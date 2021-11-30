#ifndef __REACTION_HH__
#define __REACTION_HH__

#include <iostream>
#include <vector>
#include <string>

#include "TSystem.h"
#include "TEnv.h"
#include "TMath.h"
#include "TObject.h"
#include "TFile.h"
#include "TCutG.h"
#include "TVector3.h"
#include "TF1.h"
#include "TError.h"
#include "Math/RootFinder.h"
#include "Math/Functor.h"

// Settings header
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif


#define u_mass 931494.10242f;	///< 1 atomic mass unit in keV/c^2
#define kg_mm_s 299.792458f;	/// 1 keV/c in kg•mm/s


/// A class to read in the reaction file in ROOT's TConfig format.
/// And also to do the physics stuff for the reaction

class Particle : public TObject {
	
public:
	
	// setup functions
	Particle();
	~Particle();
	
	// Get properties
	inline double	GetMass_u(){ return A; };			// returns mass in u
	inline double	GetMass(){ return A*u_mass; };		// returns mass in keV/c^2
	inline int		GetA(){ return (int)(A+0.499); };	// returns mass number
	inline int		GetZ(){ return Z; };
	inline double	GetEnergyLab(){ return Elab; };
	inline double	GetEnergyTotLab(){
		return GetMass() + GetEnergyLab();
	};
	inline double	GetEnergyTotCM(){ return Ecm_tot; };
	inline double	GetMomentumLab(){
		return TMath::Sqrt( TMath::Power( GetEnergyTotLab(), 2.0 ) - TMath::Power( GetMass(), 2.0 ) );
	};
	inline double	GetMomentumCM(){
		return TMath::Sqrt( TMath::Power( GetEnergyTotCM(), 2.0 ) - TMath::Power( GetMass(), 2.0 ) );
	};
	inline double	GetGamma(){
		return GetEnergyTotLab() / GetMass();
	};
	inline double	GetThetaCM(){ return ThetaCM; };
	inline double	GetThetaLab(){ return ThetaLab; };
	inline double	GetEx(){ return Ex; };

	// Set properties
	inline void		SetA( double myA ){ A = myA; };
	inline void		SetZ( int myZ ){ Z = myZ; };
	inline void		SetEnergyLab( double myElab ){ Elab = myElab; };
	inline void		SetEnergyTotCM( double myEcm ){ Ecm_tot = myEcm; };
	inline void		SetThetaCM( double mytheta ){ ThetaCM = mytheta; };
	inline void		SetThetaLab( double mytheta ){ ThetaLab = mytheta; };
	inline void		SetEx( double myEx ){ Ex = myEx; };


private:
	
	// Properties of reaction particles
	double	A;			///< mass in atomic units, u
	int		Z; 			///< The Z of the particle, obviously
	double	Ecm_tot;	///< total  energy in the centre of mass frame
	double	Elab;		///< energy in the laboratory system
	double	ThetaCM;	///< theta in the centre of mass frame in radians
	double	ThetaLab;	///< theta in the laboratory system in radians
	double	Ex;			///< Excitation energy in keV

	
	ClassDef( Particle, 1 )
	
};

class Reaction {
	
public:
	
	// setup functions
	Reaction( std::string filename, Settings *myset );
	~Reaction();
	
	// Main functions
	void ReadReaction();
	void SetFile( std::string filename ){
		fInputFile = filename;
	}
	const std::string InputFile(){
		return fInputFile;
	}
	
	// This is the function called event-by-event
	void	MakeReaction( TVector3 vec, double en );
		
	// Get values
	inline double GetField(){ return Mfield; };
	inline double GetField_corr(){ return Mfield*kg_mm_s; };
	inline double GetArrayDistance(){ return z0; };
	inline double GetThetaCM(){ return Recoil.GetThetaCM(); };
	inline double GetDistance(){ return z; };
	inline double GetEx(){ return Recoil.GetEx(); };
	inline double GetEnergyTotLab(){
		return Beam.GetEnergyTotLab() + Target.GetEnergyTotLab();
	};
	inline double GetEnergyTotCM(){
		double etot = TMath::Power( Beam.GetMass(), 2.0 );
		etot += TMath::Power( Target.GetMass(), 2.0 );
		etot += 2.0 * Beam.GetEnergyTotLab() * Target.GetMass();
		etot = TMath::Sqrt( etot );
		return etot;
	};
	inline double GetBeta(){
		return TMath::Sqrt( 2.0 * Beam.GetEnergyLab() / Beam.GetMass() );
	};
	inline double GetGamma(){
		return 1.0 / TMath::Sqrt( 1.0 - TMath::Power( GetBeta(), 2.0 ) );
	};
	inline double GetZmeasured(){ return z_meas; };
	inline double GetZprojected(){ return z; };
	inline double GetEBISOnTime(){ return EBIS_On; };
	inline double GetEBISOffTime(){ return EBIS_Off; };
	inline double GetEBISRatio(){ return EBIS_On / ( EBIS_Off - EBIS_On ); };

	// Set values
	inline void	SetField( double m ){ Mfield = m; };
	inline void	SetArrayDistance( double d ){ z0 = d; };
	
	// Target offsets
	inline void SetOffsetX( double x ){ x_offset = x; };
	inline void SetOffsetY( double y ){ y_offset = y; };

	// Get cuts
	unsigned int ncuts;
	inline TCutG* GetRecoilCut( unsigned int i ){
		if( i < ncuts ) return recoil_cut.at(i);
		else return nullptr;
	};
	
private:

	std::string fInputFile;
	
	// Settings file
	Settings *set;

	// Stuff with the magnet and detectors
	double Mfield;	///< Magnetic field strength in Telsa
	double z0;		///< Distance between the array and first silicon wafer
	
	// Reaction partners
	Particle Beam, Target, Ejectile, Recoil;
	
	// Initial properties from file
	double Eb;		///< laboratory beam energy in keV/u
	
	// Stuff for the Ex calculation
	TF1 *fa;
	double alpha;
	double params[4];
	double e3_cm;
	double Ex;
	double theta_cm;
	
	// EBIS time windows
	double EBIS_On;		///< beam on max time in ns
	double EBIS_Off;	///< beam off max time in ns
	
	// Experimental info on the ejectile
	double rho;			///< Distance from the beam axis to the interaction point in the detector
	double z_meas;		///< measured z distance from target that ejectile interesect the silicon detector
	double z;			///< projected z distance from target that ejectile interesect the beam axis

	// Target offsets
	double x_offset;	///< horizontal offset of the target/beam position, with respect to the array in mm
	double y_offset;	///< vertical offset of the target/beam position, with respect to the array in mm
	
	// Cuts
	std::vector<std::string> cutfile, cutname;
	TFile *recoil_file;
	std::vector<TCutG*> recoil_cut;
	
};

#endif
