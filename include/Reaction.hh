#ifndef __REACTION_HH
#define __REACTION_HH

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <memory>

#include "TSystem.h"
#include "TEnv.h"
#include "TMath.h"
#include "TObject.h"
#include "TString.h"
#include "TFile.h"
#include "TCutG.h"
#include "TVector3.h"
#include "TAxis.h"
#include "TF1.h"
#include "TFormula.h"
#include "TError.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "Math/RootFinder.h"
#include "Math/Functor.h"
#include "TSpline.h"

// Settings header
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif

// ISSEvts header
#ifndef __ISSEVTS_HH
# include "ISSEvts.hh"
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
const double T_to_mm =   299.792458;	///< in units of 1/mm
const double k_Si 	 =     2.88e-10;	///< k value - mm/e-h pair for PHD in silicon 
const double e0_Si 	 =     3.67e-03;	///< epsilon_0 for silicon for PHD in keV

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
};///< Symbols for each element in the nuclear chart


///////////////////////////////////////////////////////////////////////////////
/*!
* \brief Stores information about individual particles in a given reaction
*
* Storage class primarily for information particular to a given particle. A 
* number of these come together to give information as part of the ISSReaction 
* class.
*
*/
class ISSParticle : public TObject {
	
public:
	
	// setup functions
	ISSParticle() {};///< Constructor
	~ISSParticle() {};///< Destructor

	// Getters
	inline int		GetA(){ return A; };///< Getter for A
	inline int		GetZ(){ return Z; };///< Getter for Z
	inline int		GetN(){ return A-Z; };///< Calculates N = A - Z
	inline double	GetBindingEnergy(){ return bindingE; };///< Getter for bindingE
	inline double	GetEnergyLab(){ return Elab; };///< Getter for the Elab; Ek = (γ-1)m0
	inline double	GetEnergyTotCM(){ return Ecm_tot; };///< Getter for the total energy in the CM frame
	inline double	GetThetaCM(){ return ThetaCM; };///< Getter for ThetaCM
	inline double	GetThetaLab(){ return ThetaLab; };///< Getter for ThetaLab
	inline double	GetEx(){ return Ex; };///< Getter for Ex
	
	// Setters
	inline void		SetA( int myA ){ A = myA; };///< Setter for A
	inline void		SetZ( int myZ ){ Z = myZ; };///< Setter for Z
	inline void		SetBindingEnergy( double myBE ){ bindingE = myBE; };///< Setter for bindingE
	inline void		SetEnergyLab( double myElab ){ Elab = myElab; };///< Setter for Elab
	inline void		SetEnergyTotCM( double myEcm ){ Ecm_tot = myEcm; };///< Setter for Ecm_tot
	inline void		SetThetaCM( double mytheta ){ ThetaCM = mytheta; };///< Setter for ThetaCM
	inline void		SetThetaLab( double mytheta ){ ThetaLab = mytheta; };///< Setter for ThetaLab
	inline void		SetEx( double myEx ){ Ex = myEx; };///< Setter for Ex
	
	// Calculate/produce properties of the particle
	inline double		GetMass_u(){
		return GetMass() / u_mass;
	};///< Returns mass in u

	inline double		GetMass(){
		double mass = (double)GetN() * n_mass;
		mass += (double)GetZ() * p_mass;
		mass -= (double)GetA() * bindingE;
		return mass;
	};///< Returns mass in keV/c^2

	inline std::string	GetIsotope(){
		return std::to_string( GetA() ) + gElName.at( GetZ() );
	};///< Produces the isotope symbol including mass number e.g. 208Pb

	inline double		GetEnergyTotLab(){
		return GetMass() + GetEnergyLab();
	};///< Calculates the total energy in the lab frame: Etot = Ek + m0 = γm0

	
	inline double		GetMomentumLab(){
		return TMath::Sqrt( TMath::Power( GetEnergyTotLab(), 2.0 ) - TMath::Power( GetMass(), 2.0 ) );
	};///< Calculates the total momentum in the Lab frame
	
	inline double		GetMomentumCM(){
		return TMath::Sqrt( TMath::Power( GetEnergyTotCM(), 2.0 ) - TMath::Power( GetMass(), 2.0 ) );
	};///< Calculates the total momentum in the CM frame
	
	inline double		GetGamma(){ 
		return GetEnergyTotLab() / GetMass();
	};///< Calculates the gamma factor: Etot = γm0
	
	inline double GetBeta(){
		return TMath::Sqrt( 1.0 - 1.0 / TMath::Power( GetGamma(), 2.0 ) );
	};///< Calculates the beta factor


private:
	
	// Properties of reaction particles
	int		A;			///< Mass number, A, of the particle
	int		Z; 			///< Proton number, Z, of the particle
	double	bindingE;	///< Binding energy per nucleon in keV/c^2 (NOT keV/u!!)
	double	Ecm_tot;	///< Total  energy in the centre of mass frame
	double	Elab;		///< Energy in the laboratory system
	double	ThetaCM;	///< Theta in the centre of mass frame in radians
	double	ThetaLab;	///< Theta in the laboratory system in radians
	double	Ex=0;		///< Excitation energy in keV

	ClassDef( ISSParticle, 1 )
	
};


///////////////////////////////////////////////////////////////////////////////
/*!
* \brief Reads in the reaction file in ROOT's TConfig format. And also to do the physics stuff for the reaction.
*
* Holds all the physics information about a given reaction. Calculates relevant 
* kinematic quantities and is accessed for plotting histograms.
*
*/
class ISSReaction {
	
public:
	
	// setup functions
	ISSReaction( std::string filename, std::shared_ptr<ISSSettings> myset, bool source );///< Constructor
	//ISSReaction( ISSReaction &t ); ///< TODO: Copy constructor
	~ISSReaction(){};///< Destructor
	
	// Main functions
	void AddBindingEnergy( short Ai, short Zi, TString ame_be_str );///< Add a binding energy to the ame_be mass-table map
	void ReadMassTables();///< Reads the AME2020 mass tables
	void ReadReaction();///< Reads the reaction input file
	
	void SetFile( std::string filename ){
		fInputFile = filename;
	}///< Setter for the reaction file location
	
	const std::string InputFile(){
		return fInputFile;
	}///< Getter for the reaction file location
	
	// This is the function called event-by-event
	void	MakeReaction( TVector3 vec, double en );///< Called event-by-event for transfer reactions
	void	SimulateReaction( TVector3 vec );///< Setup your particles, then call this with the ejectile detection position
	double	SimulateDecay( TVector3 vec, double en, int detector = 0 );///< Called during the autocalibration process with alphas
	double	SimulateEmission( double en, double theta_lab, double phi_lab, int detector = 0 );///< Called during the simulation of particle emission

	// Getters
	inline double GetField(){ return Mfield; };///< Getter for the magnetic field strength
	inline double GetField_corr(){ return Mfield*T_to_mm; };///< Calculates magnetic field strength in MeV/ e*mm*c
	inline double GetArrayDistance(){ return z0; };///< Getter for the distance between the array and first silicon wafer
	inline double GetArrayDeadlayer(){ return deadlayer; };		///< Getter for the array dead layer
	inline double GetELUMDistance(){ return elum_z; };			///< Getter for the distance between the target and the ELUM silicon
	inline double GetELUMInnerRadius(){ return elum_rin; };		///< Getter for the ELUM inner radius
	inline double GetELUMOuterRadius(){ return elum_rout; };	///< Getter for the ELUM outer radius
	inline double GetELUMDeadlayer(){ return elum_deadlayer; };	///< Getter for the ELUM dead layer
	inline double GetThetaCM(){ return Recoil.GetThetaCM(); };///< Getter for the CM angle of the recoil/ejectile
	inline double GetDistance(){ return z; };///< Getter for the interaction point
	inline double GetEx(){ return Recoil.GetEx(); };///< Getter for the excitation energy

	inline double GetOffsetX(){ return x_offset; };
	inline double GetOffsetY(){ return y_offset; };

	inline double GetEBISOnTime(){ return EBIS_On; };///< Getter for the EBIS ON time
	inline double GetEBISOffTime(){ return EBIS_Off; };///< Getter for the EBIS OFF time
	inline double GetEBISTimeRatio(){ return EBIS_On / ( EBIS_Off - EBIS_On ); };///< Getter for the EBIS time ratio
	inline double GetEBISFillRatio(){ return EBIS_ratio; };///< Getter for the EBIS fill ratio

	inline double GetT1MinTime(){ return t1_min_time; };///< Getter for the T1 time cut minimum
	inline double GetT1MaxTime(){ return t1_max_time; };///< Getter for the T1 time cut maximum

	inline unsigned char GetLaserMode(){ return laser_mode; }; ///< Getter for LaserMode value
	
	inline double GetZmeasured(){ return z_meas; };///< Getter for the measured z (where the particle lands on the array)
	inline double GetZprojected(){ return z; };///< Getter for the projected z (where the particle would intersect with the beam axis)
	
	inline double GetQvalue(){
		return Beam.GetMass() + Target.GetMass() -
			Ejectile.GetMass() - Recoil.GetMass();
	};///< Calculates the Q value for the reaction
	
	inline double GetEnergyTotLab(){
		return Beam.GetEnergyTotLab() + Target.GetEnergyTotLab();
	};///< Calculates the total energy in the lab frame
	
	inline double GetEnergyTotCM(){
		double etot = TMath::Power( Beam.GetMass(), 2.0 );
		etot += TMath::Power( Target.GetMass(), 2.0 );
		etot += 2.0 * Beam.GetEnergyTotLab() * Target.GetMass();
		etot = TMath::Sqrt( etot );
		return etot;
	};///< Calculates the total energy in the CM frame
	
	inline double		GetGamma(){
		return GetEnergyTotLab() / GetEnergyTotCM();
	};///< Calculates the gamma factor for going between lab and CM frames
	
	inline double GetBeta(){
		return TMath::Sqrt( 1.0 - 1.0 / TMath::Power( GetGamma(), 2.0 ) );
	};///< Calculates the beta factor for going between lab and CM frames
	
	// Array-recoil time difference
	inline double GetArrayRecoilPromptTime( unsigned char i ){
		// i = 0 for lower limit and i = 1 for upper limit
		if( i < 2 ) return array_recoil_prompt[i];
		else return 0;
	};///< Getter for array-recoil prompt time difference, used for defining coincidence windows
	
	inline double GetArrayRecoilRandomTime( unsigned char i ){
		// i = 0 for lower limit and i = 1 for upper limit
		if( i < 2 ) return array_recoil_random[i];
		else return 0;
	};///< Getter for array-recoil random time difference, used for defining coincidence windows
	
	inline double GetArrayRecoilTimeRatio(){
		return ( array_recoil_prompt[1] - array_recoil_prompt[0] ) / ( array_recoil_random[1] - array_recoil_random[0] );
	};///< Returns prompt window/random window
	
	inline double GetArrayRecoilFillRatio(){
		return array_recoil_ratio;
	};///< Getter for array-recoil fill ratio (unused?)
	
	
	// ELUM-recoil time difference
	inline double GetElumRecoilPromptTime( unsigned char i ){
		// i = 0 for lower limit and i = 1 for upper limit
		if( i < 2 ) return elum_recoil_prompt[i];
		else return 0;
	};///< Getter for elum-recoil prompt time difference, used for defining coincidence windows
	
	inline double GetElumRecoilRandomTime( unsigned char i ){
		// i = 0 for lower limit and i = 1 for upper limit
		if( i < 2 ) return elum_recoil_random[i];
		else return 0;
	};///< Getter for elum-recoil random time difference, used for defining coincidence windows
	
	inline double GetElumRecoilTimeRatio(){
		return ( elum_recoil_prompt[1] - elum_recoil_prompt[0] ) / ( elum_recoil_random[1] - elum_recoil_random[0] );
	};///< Returns prompt window/random window
	
	inline double GetElumRecoilFillRatio(){
		return elum_recoil_ratio;
	};///< Getter for array-recoil fill ratio (unused?)
	
	
	// Setters
	inline void	SetField( double m ){ Mfield = m; };					///< Setter for the magnetic field strength
	inline void	SetArrayDistance( double d ){ z0 = d; };				///< Setter for the distance between the target and first silicon wafer of the array
	inline void	SetArrayDeadlayer( double d ){ deadlayer = d; };		///< Setter for the array dead layer
	inline void	SetELUMDistance( double d ){ elum_z = d; };				///< Setter for the distance between the target and ELUM silicon
	inline void	SetELUMDeadlayer( double d ){ elum_deadlayer = d; };	///< Setter for the ELUM dead layer

	inline void SetOffsetX( double x ){ x_offset = x; };	///< Setter for the target offset (X)
	inline void SetOffsetY( double y ){ y_offset = y; };	///< Setter for the target offset (Y)

	// Energy loss and stopping powers
	double GetEnergyLoss( double Ei, double dist, std::unique_ptr<TGraph> &g );///< Calculate the energy loss of a given energy of particle through a given material
	double GetNuclearEnergyLoss( double Ei, double range, std::unique_ptr<TGraph> &gn, std::unique_ptr<TGraph> &gtot );///< Calculate the nuclear specific energy loss
	bool ReadStoppingPowers( std::string isotope1, std::string isotope2,
				std::unique_ptr<TGraph> &g, std::unique_ptr<TGraph> &r,
				bool electriconly = false, bool nuclearonly = false );///< Reads relevant SRIM files

	// Pulse height correction
	double GetPulseHeightCorrection( double Ei, bool detected ); ///< Returns the pulse height correction from the relevant TGraph
	void CalculatePulseHeightCorrection( std::string isotope ); ///< This function generates all neccessary pulse-height correction plots

	// Get cuts
	inline unsigned int GetNumberOfRecoilCuts(){ return nrecoilcuts; };///< Getter for the number of recoil cuts
	inline std::shared_ptr<TCutG> GetRecoilCut( unsigned int i ){
		if( i < nrecoilcuts ) return recoil_cut.at(i);
		else return nullptr;
	};///< Getter for particular recoil cuts
	
	// Get cuts
	inline unsigned int GetNumberOfEvsZCuts(){ return nevszcuts; };///< Getter for the number of E vs z cuts
	inline std::shared_ptr<TCutG> GetEvsZCut( unsigned int i ){
		if( i < nevszcuts ) return e_vs_z_cut.at(i);
		else return nullptr;
	};///< Returns a particular cut applied to the E vs z plot

	// It's a source only measurement
	inline void SourceOnly(){ flag_source = true; };///< Flags the measurement as source only
	inline bool IsSource(){ return flag_source; };
	
	// Get filename and other copy stuff
	inline std::string GetFileName(){ return fInputFile; };
	inline std::shared_ptr<ISSSettings> GetSettings(){ return set; };
	inline std::map< std::string, double > GetMassTables(){ return ame_be; };
	//inline void CopyMinimisationFunction( TF1 *fin ){ fa->Copy( *fin ); };
	//inline void CopyMinimisationDerivative( TF1 *fin ){ fb->Copy( *fin ); };

	// Copiers for the particles
	inline ISSParticle CopyBeam(){ return Beam; };
	inline ISSParticle CopyTarget(){ return Target; };
	inline ISSParticle CopyEjectile(){ return Ejectile; };
	inline ISSParticle CopyRecoil(){ return Recoil; };

	// Getters for the particles
	inline ISSParticle* GetBeam(){ return &Beam; };
	inline ISSParticle* GetTarget(){ return &Target; };
	inline ISSParticle* GetEjectile(){ return &Ejectile; };
	inline ISSParticle* GetRecoil(){ return &Recoil; };

private:

	std::string fInputFile;///< The directory location of the input reaction file
	
	// Settings file
	std::shared_ptr<ISSSettings> set;///< Smart pointer to the ISSSettings object
	
	// Mass tables
	std::map< std::string, double > ame_be;///< List of binding energies from AME2020

	// Stuff with the magnet and detectors
	double Mfield;			///< Magnetic field strength in Telsa
	double z0;				///< Distance between the array and first silicon wafer
	double deadlayer;		///< Dead layer on array silicon in mm of Si equivalent

	// Reaction partners
	ISSParticle Beam;		///< Beam particle
	ISSParticle Target;		///< Target particle
	ISSParticle Ejectile;	///< Ejectile particle
	ISSParticle Recoil;		///< Recoil particle
	
	// Initial properties from file
	double Eb;		///< Laboratory beam energy in keV/u
	
	// Stuff for the Ex calculation
	std::unique_ptr<ROOT::Math::RootFinder> rf;		///< Pointer to a root finder object
	std::unique_ptr<ROOT::Math::RootFinder> rfsim;	///< Pointer to a root finder object for the simulation of the reaction
	TF1 *fa;	///< Pointer to the minimisation function
	TF1 *fb;	///< Pointer to the derivative of the minimisation function
	TF1 *fsim;	///< Pointer to the minimisation function for the simulation of the reaction
	TF1 *dsim;	///< Pointer to the derivative of the minimisation function for the simulation of the reaction
	double params[9];			///< Array for holding parameters for the functions
	double e3_cm;				///< Total energy of ejectile in centre of mass
	double Ex;					///< Excitation energy of recoil
	double theta_cm;			///< CM angle for ejectile/recoil
	double alpha;				///< An angle used for calculating ThetaCM and Ex

	// EBIS time windows
	double EBIS_On;		///< Beam on max time in ns
	double EBIS_Off;	///< Beam off max time in ns
	double EBIS_ratio;	///< Ratio of ebis on/off as measured
	
	// T1 time window
	double t1_max_time;	///< T1 pulse max time
	double t1_min_time;	///< T1 pulse min time
	
	// Laser mode
	unsigned char laser_mode;	///< user can select which laser data to sort, with values:
								///< 0 = laser off NOT on
								///< 1 = laser on NOT off
								///< 2 = laser on OR off (default)

	// Coincidence windows
	double array_recoil_prompt[2]; ///< Prompt time windows between recoil and array event
	double array_recoil_random[2]; ///< Prompt time window between recoil and array event
	double array_recoil_ratio; // fill ratios
	double elum_recoil_prompt[2]; ///< Prompt time windows between recoil and elum event
	double elum_recoil_random[2]; ///< Prompt time window between recoil and elum event
	double elum_recoil_ratio; // fill ratios

	// Experimental info on the ejectile
	double r_meas;		///< Measured radius of the ejectile when it interects the array
	double z_meas;		///< Measured z distance from target that ejectile interesect the silicon detector
	double z;			///< Projected z distance from target that ejectile interesect the beam axis

	// Target thickness and offsets
	double target_thickness;	///< Target thickness in units of mg/cm^2
	double x_offset;			///< Horizontal offset of the target/beam position, with respect to the array in mm
	double y_offset;			///< Vertical offset of the target/beam position, with respect to the array in mm
	
	// ELUM geometry
	double elum_z;		///< z position of the ELUM (usually positive, but if negative assumed not to exist in setup)
	double elum_rin;	///< inner radius of the ELUM detector
	double elum_rout;	///< outer radius of the ELUM detector
	double elum_deadlayer;	///< Dead layer on ELUM in mm of Si equivalent

	// Cuts
	unsigned int nrecoilcuts;					///< The number of recoil cuts
	unsigned int nevszcuts;						///< The number of E vs z cuts
	std::vector<std::string> recoilcutfile;		///< The location of the recoil cut files
	std::vector<std::string> recoilcutname;		///< The names of the recoil cuts
	std::vector<std::string> evszcutfile;		///< The location of the E vs z cut files
	std::vector<std::string> evszcutname;		///< The names of the E vs z cuts
	//TFile *recoil_file;							///< Pointer for opening the recoil cut files (??? could this be a local variable)
	//TFile *e_vs_z_file;							///< Pointer for the E vs z cut file names (??? could this be a local variable)
	std::vector<std::shared_ptr<TCutG>> recoil_cut;				///< Vector containing the recoil cuts
	std::vector<std::shared_ptr<TCutG>> e_vs_z_cut;				///< Vector containing the E vs z cuts

	// Stopping powers
	std::vector<std::unique_ptr<TGraph>> gStopping;	///< Vector of pointer to relevant stopping-power TGraphs relevant to the reaction of study
	std::vector<std::unique_ptr<TGraph>> gRange;	///< Vector of pointer to relevant range TGraphs relevant to the reaction of study
	std::unique_ptr<TGraph> gPHC;					///< TGraph containing pulse-height correction information
	std::unique_ptr<TGraph> gPHC_inv;				///< Flipped axes of gPHC
	bool stopping;									///< Flag to indicate whether calculation of stopping powers has worked or not
	bool phcurves;									///< Flag to indicate whether pulse height correction data was read successfully
	std::vector<double> alpha_energies;				///< Energies of the alphas for calibration in a source run

	
	// Flag in case it's an alpha source
	bool flag_source;	///< Flag in case it's an alpha source run

};

#endif
