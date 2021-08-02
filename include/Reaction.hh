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
#include "Math/RootFinderAlgorithms.h"
#include "Math/Functor.h"

// Settings header
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif


#define u_mass 931492.10142f;	///< 1 atomic mass unit in keV/c^2
#define c_mm_ns 299.792458f;	///< speed of light in mm/ns



/// A class to read in the reaction file in ROOT's TConfig format.
/// And also to do the physics stuff for the reaction

class Particle : public TObject {
	
public:
	
	// setup functions
	Particle();
	~Particle();
	
	// Get properties
	inline float	GetMass_u(){ return A; };			// returns mass in u
	inline float	GetMass(){ return A*u_mass; };		// returns mass in keV/c^2
	inline int		GetA(){ return (int)(A+0.499); };	// returns mass number
	inline int		GetZ(){ return Z; };
	inline float	GetEnergyCM(){ return Ecm; };
	inline float	GetEnergyLab(){ return Elab; };
	inline float	GetThetaCM(){ return ThetaCM; };
	inline float	GetThetaLab(){ return ThetaLab; };
	inline float	GetEx(){ return Ex; };
	inline float	GetQ(){ return Q; };

	// Set properties
	inline void		SetA( float myA ){ A = myA; };
	inline void		SetZ( int myZ ){ Z = myZ; };
	inline void		SetEnergyCM( float myEcm ){ Ecm = myEcm; };
	inline void		SetEnergyLab( float myElab ){ Elab = myElab; };
	inline void		SetThetaCM( float mytheta ){ ThetaCM = mytheta; };
	inline void		SetThetaLab( float mytheta ){ ThetaLab = mytheta; };
	inline void		SetEx( float myEx ){ Ex = myEx; };
	inline void		SetQ( float myQ ){ Q = myQ; };


private:
	
	// Properties of reaction particles
	float	A;			///< mass in atomic units, u
	int		Z; 			///< The Z of the particle, obviously
	float	Elab;		///< energy in the laboratory system
	float	Ecm;		///< energy in the centre of mass frame
	float	ThetaCM;	///< theta in the laboratory system in radians
	float	ThetaLab;	///< theta in the centre of mass frame in radians
	float	Ex;			///< Excitation energy in keV
	float	Q;			///< Q-values in keV

	
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
	void	MakeReaction( TVector3 vec, float en );
	
	// Some extra calculation steps
	void	CalculateZ();
	
	// Get values
	inline float GetField(){ return Mfield; };
	inline float GetField_corr(){ return Mfield*c_mm_ns; };
	inline float GetThetaCM(){ return Ejectile.GetThetaCM(); };
	inline float GetZ(){ return z; };
	inline float GetEx(){ return Recoil.GetEx(); };

	// Set values
	inline void	SetField( float m ){ Mfield = m; };
	
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

	// Stuff with the magnet
	float Mfield; ///< Magnetic field strength in Telsa
	
	// Reaction partners
	Particle Beam, Target, Ejectile, Recoil;
	
	// Initial properties from file
	float Eb;		///< laboratory beam energy in keV/u
	
	// Experimental info on the ejectile
	TVector3 ejec_vec;	///< 3-vector for the ejectile at the point in intersects the detector array
	float rho;			///< Distance from the beam axis to the interaction point in the detector
	float z;			///< projected z distance from target that ejectile interesect the beam axis
	
	// Cuts
	std::vector<std::string> cutfile, cutname;
	TFile *recoil_file;
	std::vector<TCutG*> recoil_cut;
	
};

#endif
