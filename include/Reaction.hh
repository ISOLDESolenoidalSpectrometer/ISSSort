#ifndef __REACTION_HH__
#define __REACTION_HH__

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>

#include "TSystem.h"
#include "TEnv.h"
#include "TMath.h"
#include "TObject.h"
#include "TString.h"
#include "TFile.h"
#include "TCutG.h"
#include "TVector3.h"
#include "TF1.h"
#include "TError.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "Math/RootFinder.h"
#include "Math/Functor.h"

// Settings header
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif

// Make sure that the data and srim file are defined
#ifndef AME_FILE
# define AME_FILE "./data/mass_1.mas20"
#endif
#ifndef SRIM_DIR
# define SRIM_DIR "./srim/"
#endif


const double p_mass  = 938272.08816;	///< mass of the proton in keV/c^2
const double n_mass  = 939565.42052;	///< mass of the neutron in keV/c^2
const double u_mass  = 931494.10242;	///< atomic mass unit in keV/c^2
const double kg_mm_s =   299.792458;	/// 1 keV/c in kg•mm/s

// Element names
const std::vector<std::string> gElName = {
	"n","H","He","Li","Be","B","C","N","O","F","Ne","Na","Mg",
	"Al","Si","P","S","Cl","Ar","K","Ca","Sc","Ti","V","Cr",
	"Mn","Fe","Co","Ni","Cu","Zn","Ga","Ge","As","Se","Br","Kr",
	"Rb","Sr","Y","Zr","Nb","Mo","Tc","Ru","Rh","Pd","Ag","Cd",
	"In","Sn","Sb","Te","I","Xe","Cs","Ba","La","Ce","Pr","Nd",
	"Pm","Sm","Eu","Gd","Tb","Dy","Ho","Er","Tm","Yb","Lu","Hf",
	"Ta","W","Re","Os","Ir","Pt","Au","Hg","Tl","Pb","Bi","Po",
	"At","Rn","Fr","Ra","Ac","Th","Pa","U","Np","Pu","Am","Cm",
	"Bk","Cf","Es","Fm","Md","No","Lr","Rf","Db","Sg","Bh","Hs",
	"Mt","Ds","Rg","Cn","Nh","Fl","Ms","Lv","Ts","Og","Uue","Ubn"
};


/// A class to read in the reaction file in ROOT's TConfig format.
/// And also to do the physics stuff for the reaction

class ISSParticle : public TObject {
	
public:
	
	// setup functions
	ISSParticle() {};
	~ISSParticle() {};
	
	// Get properties
	inline double		GetMass_u(){
		return GetMass() / u_mass;
	};			// returns mass in u
	inline double		GetMass(){
		double mass = (double)GetN() * n_mass;
		mass += (double)GetZ() * p_mass;
		mass -= (double)GetA() * bindingE;
		return mass;
	};		// returns mass in keV/c^2
	inline int			GetA(){ return A; };	// returns mass number
	inline int			GetZ(){ return Z; };
	inline int			GetN(){ return A-Z; };
	inline std::string	GetIsotope(){
		return std::to_string( GetA() ) + gElName.at( GetZ() );
	};
	inline double		GetBindingEnergy(){ return bindingE; };
	inline double		GetEnergyLab(){ return Elab; }; // Ek = (γ-1)m0
	inline double		GetEnergyTotLab(){ // Etot = Ek + m0 = γm0
		return GetMass() + GetEnergyLab();
	};
	inline double		GetEnergyTotCM(){ return Ecm_tot; };
	inline double		GetMomentumLab(){
		return TMath::Sqrt( TMath::Power( GetEnergyTotLab(), 2.0 ) - TMath::Power( GetMass(), 2.0 ) );
	};
	inline double		GetMomentumCM(){
		return TMath::Sqrt( TMath::Power( GetEnergyTotCM(), 2.0 ) - TMath::Power( GetMass(), 2.0 ) );
	};
	inline double		GetGamma(){ // Etot = γm0
		return GetEnergyTotLab() / GetMass();
	};
	inline double GetBeta(){
		return TMath::Sqrt( 1.0 - 1.0 / TMath::Power( GetGamma(), 2.0 ) );
	};
	inline double		GetThetaCM(){ return ThetaCM; };
	inline double		GetThetaLab(){ return ThetaLab; };
	inline double		GetEx(){ return Ex; };

	// Set properties
	inline void		SetA( int myA ){ A = myA; };
	inline void		SetZ( int myZ ){ Z = myZ; };
	inline void		SetBindingEnergy( double myBE ){ bindingE = myBE; };
	inline void		SetEnergyLab( double myElab ){ Elab = myElab; };
	inline void		SetEnergyTotCM( double myEcm ){ Ecm_tot = myEcm; };
	inline void		SetThetaCM( double mytheta ){ ThetaCM = mytheta; };
	inline void		SetThetaLab( double mytheta ){ ThetaLab = mytheta; };
	inline void		SetEx( double myEx ){ Ex = myEx; };


private:
	
	// Properties of reaction particles
	int		A;			///< mass number, A of the particle, obviously
	int		Z; 			///< The Z of the particle, obviously
	double	bindingE;	///< binding energy per nucleon in keV/c^2
	double	Ecm_tot;	///< total  energy in the centre of mass frame
	double	Elab;		///< energy in the laboratory system
	double	ThetaCM;	///< theta in the centre of mass frame in radians
	double	ThetaLab;	///< theta in the laboratory system in radians
	double	Ex;			///< Excitation energy in keV

	
	ClassDef( ISSParticle, 1 )
	
};

class ISSReaction {
	
public:
	
	// setup functions
	ISSReaction( std::string filename, ISSSettings *myset, bool source );
	virtual ~ISSReaction();
	
	// Main functions
	void AddBindingEnergy( short Ai, short Zi, TString ame_be_str );
	void ReadMassTables();
	void ReadReaction();
	void SetFile( std::string filename ){
		fInputFile = filename;
	}
	const std::string InputFile(){
		return fInputFile;
	}
	
	// This is the function called event-by-event
	void	MakeReaction( TVector3 vec, double en );
	void	SimulateReaction( TVector3 vec, double ex );
	float	SimulateDecay( TVector3 vec, double en );

	// Get values
	inline double GetField(){ return Mfield; };
	inline double GetField_corr(){ return Mfield*kg_mm_s; };
	inline double GetArrayDistance(){ return z0; };
	inline double GetThetaCM(){ return Recoil.GetThetaCM(); };
	inline double GetDistance(){ return z; };
	inline double GetEx(){ return Recoil.GetEx(); };
	inline double GetQvalue(){
		return Beam.GetMass() + Target.GetMass() -
			Ejectile.GetMass() - Recoil.GetMass();
	};
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
	inline double		GetGamma(){
		return GetEnergyTotLab() / GetEnergyTotCM();
	};
	inline double GetBeta(){
		return TMath::Sqrt( 1.0 - 1.0 / TMath::Power( GetGamma(), 2.0 ) );
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

	// Energy loss and stopping powers
	double GetEnergyLoss( double Ei, double dist, std::unique_ptr<TGraph> &g );
	bool ReadStoppingPowers( std::string isotope1, std::string isotope2, std::unique_ptr<TGraph> &g );

	// Get cuts
	inline TCutG* GetRecoilCut( unsigned int i ){
		if( i < nrecoilcuts ) return recoil_cut.at(i);
		else return nullptr;
	};
	
	// Get cuts
	inline unsigned int GetNumberOfEvsZCuts(){ return nevszcuts; };
	inline TCutG* GetEvsZCut( unsigned int i ){
		if( i < nevszcuts ) return e_vs_z_cut.at(i);
		else return nullptr;
	};

	// It's a source only measurement
	inline void SourceOnly(){ flag_source = true; };

	
private:

	std::string fInputFile;
	
	// Settings file
	ISSSettings *set;
	
	// Mass tables
	std::map< std::string, double > ame_be; ///< List of biniding energies from  AME2021

	// Stuff with the magnet and detectors
	double Mfield;		///< Magnetic field strength in Telsa
	double z0;			///< Distance between the array and first silicon wafer
	double deadlayer;	///< Dead layer on array silicon in mm of Si equivalent
	
	// Reaction partners
	ISSParticle Beam, Target, Ejectile, Recoil;
	
	// Initial properties from file
	double Eb;		///< laboratory beam energy in keV/u
	
	// Stuff for the Ex calculation
	std::unique_ptr<ROOT::Math::RootFinder> rf;
	std::unique_ptr<TF1> fa, fb;
	double params[4];
	double e3_cm;
	double Ex;
	double theta_cm;
	double theta_lab;
	double alpha;

	// EBIS time windows
	double EBIS_On;		///< beam on max time in ns
	double EBIS_Off;	///< beam off max time in ns
	
	// Experimental info on the ejectile
	double rho;			///< Distance from the beam axis to the interaction point in the detector
	double z_meas;		///< measured z distance from target that ejectile interesect the silicon detector
	double z;			///< projected z distance from target that ejectile interesect the beam axis

	// Target thickness and offsets
	double target_thickness;	///< target thickness in units of mg/cm^2
	double x_offset;			///< horizontal offset of the target/beam position, with respect to the array in mm
	double y_offset;			///< vertical offset of the target/beam position, with respect to the array in mm
	
	// Cuts
	unsigned int nrecoilcuts;
	unsigned int nevszcuts;
	std::vector<std::string> recoilcutfile, recoilcutname;
	std::vector<std::string> evszcutfile, evszcutname;
	TFile *recoil_file;
	TFile *e_vs_z_file;
	std::vector<TCutG*> recoil_cut;
	std::vector<TCutG*> e_vs_z_cut;

	// Stopping powers
	std::vector<std::unique_ptr<TGraph>> gStopping;
	bool stopping;
	
	// Flag in case it's an alpha source
	bool flag_source;

};

#endif
