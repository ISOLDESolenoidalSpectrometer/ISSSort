#include "Reaction.hh"

// Uncomment this below to switch to the Butler algorithm (currently in testing)
#define butler_algorithm

ClassImp( ISSParticle )
ClassImp( ISSReaction )

///////////////////////////////////////////////////////////////////////////////
/// Minimisation function used for solving for angle alpha
/// \param[in] x Initial guess for angle alpha
/// \param[in] params Various parameters required for this minimisation
/// \returns root This number should be zero when minimised
double alpha_function( double *x, double *params ){

	// Equation to solve for alpha, LHS = 0
	double alpha = x[0];
	double z = params[0];
	double r_meas = params[1];
	double p = params[2];	//  p
	double qb = params[3];	//  qb / 2*pi

	double root = p * TMath::Sin(alpha);
	root -= qb * r_meas * TMath::Tan(alpha);
	root -= qb * z;

	return root;

}

///////////////////////////////////////////////////////////////////////////////
/// Derivative of alpha_function
/// \param[in] x Initial guess for angle alpha
/// \param[in] params Various parameters required for this minimisation
/// \returns root The derivative of the minimisation function
double alpha_derivative( double *x, double *params ){

	// Derivative of the alpha equation
	double alpha = x[0];
	//double z = params[0]; // unused in derivative
	double r_meas = params[1];
	double p = params[2];	//  p
	double qb = params[3];	//  qb / 2*pi

	double root = p * TMath::Cos(alpha);
	root -= qb * r_meas / TMath::Cos(alpha) / TMath::Cos(alpha);

	return root;

}

///////////////////////////////////////////////////////////////////////////////
/// This solves for z using Peter Butler's method
/// \param[in] x The initial guess for z
/// \param[in] params Various parameters required for this minimisation
/// \returns root This number should be zero when minimised
double butler_function( double *x, double *params ){

	// Equation to solve for z, LHS = 0
	double z = x[0];
	double z_meas = params[0];
	double r_meas = params[1];
	double p = params[2];	//  p
	double qb = params[3];	//  qb / 2*pi

	// From Sam Bennett's first derivation, modified by A. Ceulemans
	double theta_lab = TMath::ACos( qb * z / p );
	double r_max = TMath::Abs( 2.0 * p * TMath::Sin( theta_lab ) / ( qb * TMath::TwoPi()) );
	double psi = 2 * TMath::ASin( r_meas / r_max );
	double root = z_meas - z * ( 1.0 - psi / TMath::TwoPi() );

	return root;

}

///////////////////////////////////////////////////////////////////////////////
/// This is the derivative of Peter Butler's method
/// \param[in] x The initial guess for z
/// \param[in] params Various parameters required for this minimisation
/// \returns The derivative of the minimisation function
double butler_derivative( double *x, double *params ){

	// Create the original function and solve the derivative
	std::unique_ptr<TF1> func = std::make_unique<TF1>( "butler_function", butler_function, 0.0, TMath::PiOver2(), 4 );
	return func->Derivative( x[0], params );

}

///////////////////////////////////////////////////////////////////////////////
/// This solves for the theta_cm kinematics
/// \param[in] x The initial guess for theta_cm
/// \param[in] params Various parameters required for this minimisation
/// \returns root This number should be zero when minimised
double theta_cm_function( double *x, double *params ){

	// Input to the function
	double theta_cm = x[0];
	double z_meas = params[0];
	double r_meas = params[1];
	double Ex = params[2];
	double qb = params[3]; //  over 2*pi
	double T1 = params[4];
	double m1 = params[5];
	double m2 = params[6];
	double m3 = params[7];
	double m4 = params[8];
	
	// Invariant mass = total energy in centre of mass frame
	double Etot_cm = m1*m1 + m2*m2;
	Etot_cm += 2.0 * ( T1 + m1 ) * m2;
	Etot_cm = TMath::Sqrt( Etot_cm );
	
	// Work out what gamma and beta are
	double E_lab = T1 + m1 + m2;
	double gamma = E_lab / Etot_cm;
	double beta = TMath::Sqrt( 1.0 - 1.0 / TMath::Power( gamma, 2.0 ) );

	// What about the energy of the ejectile after the collision?
	double e3_cm = TMath::Power( Etot_cm, 2.0 );
	e3_cm -= TMath::Power( m4 + Ex, 2.0 );
	e3_cm += TMath::Power( m3, 2.0 );
	e3_cm /= 2.0 * Etot_cm;

	// Now the momentum and velocity of the ejectile
	double p3_cm = TMath::Power( e3_cm, 2.0 );
	p3_cm -= TMath::Power( m3, 2.0 );
	p3_cm = TMath::Sqrt( p3_cm );
	
	// From Daniel Clarke:
	//double p_lab = beta * E_lab;
	double theta_lab = p3_cm * TMath::Sin( TMath::Pi() - theta_cm );
	theta_lab /= p3_cm * TMath::Cos( TMath::Pi() - theta_cm ) + beta * e3_cm;
	theta_lab /= gamma;
	theta_lab = TMath::ATan( theta_lab );
	if( theta_lab < 0 ) theta_lab = TMath::Pi() + theta_lab;

	// Energy of ejectile in the lab
	double e3_lab = gamma * beta * p3_cm * TMath::Cos( TMath::Pi() - theta_cm );
	e3_lab += gamma * e3_cm;
	double p3_lab = TMath::Sqrt( e3_lab*e3_lab - m3*m3 );
	double gamma3 = e3_lab / m3;

	// Maximum radius of particle, z position and missing orbit fraction (psi)
	double r_max = TMath::Abs( 2.0 * p3_lab * TMath::Sin( theta_lab ) / ( qb * TMath::TwoPi()) );
	double z = p3_lab * TMath::Cos( theta_lab ) / qb;
	double psi = 2.0 * TMath::ASin( r_meas / r_max );
	
	// This is the equation to find the root of
	double root = z_meas - z * ( 1.0 - psi / TMath::TwoPi() );

	return root;

}

///////////////////////////////////////////////////////////////////////////////
/// This solves for the derivative of theta_cm kinematics function
/// \param[in] x The initial guess for theta_cm
/// \param[in] params Various parameters required for this minimisation
/// \returns derivative of theta_cm_function
double theta_cm_derivative( double *x, double *params ){

	// Create the original function and solve the derivative
	std::unique_ptr<TF1> func = std::make_unique<TF1>( "theta_cm_function", theta_cm_function, 0.0, TMath::Pi(), 9 );
	return func->Derivative( x[0], params );
	
}

///////////////////////////////////////////////////////////////////////////////
/// Parameterised constructor for the ISSReaction object. It reads in the mass 
/// tables, assigns values to various pointers, reads the reaction file, and 
/// sets up the root-finding algorithm for finding the angle alpha.
/// \param[in] filename A string holding the name of the reaction file
/// \param[in] myset A pointer to the ISSSettings object
/// \param[in] source A boolean to check if this run is a source run
ISSReaction::ISSReaction( std::string filename, ISSSettings *myset, bool source ){
		
	// Setup the ROOT finder algorithms
#ifdef butler_algorithm
	// Root finder algorithm - The Peter Butler method
	double low_limit = z0;
	double upp_limit = z0;
	if( z0 < 0.0 ) low_limit -= 600.0;
	else upp_limit += 600.0;
	fa = std::make_unique<TF1>( "butler_function",   butler_function,   low_limit, upp_limit, 4 );
	fb = std::make_unique<TF1>( "butler_derivative", butler_derivative, low_limit, upp_limit, 4 );
#else
	// Root finder algorithm - for alpha like Ryan Tang does
	fa = std::make_unique<TF1>( "alpha_function",   alpha_function,   0.0, TMath::PiOver2(), 4 );
	fb = std::make_unique<TF1>( "alpha_derivative", alpha_derivative, 0.0, TMath::PiOver2(), 4 );
#endif
	rf = std::make_unique<ROOT::Math::RootFinder>( ROOT::Math::RootFinder::kGSL_NEWTON );

	// Root finder for the simulation function
	fsim = std::make_unique<TF1>( "theta_cm_function",   theta_cm_function,   0.0, TMath::Pi(), 9 );
	dsim = std::make_unique<TF1>( "theta_cm_derivative", theta_cm_derivative, 0.0, TMath::Pi(), 9 );
	rfsim = std::make_unique<ROOT::Math::RootFinder>( ROOT::Math::RootFinder::kGSL_NEWTON );


	// Read in mass tables
	ReadMassTables();
	
	// Check if it's a source run
	flag_source = source;

	// Get the info from the user input
	set = myset;
	SetFile( filename );
	ReadReaction();
	
}

///////////////////////////////////////////////////////////////////////////////
/// Deletes the pointers to the TCutG recoil cuts and clears the vector holding
/// them.
ISSReaction::~ISSReaction(){

	for( unsigned int i = 0; i < recoil_cut.size(); ++i )
		delete (recoil_cut[i]);
	recoil_cut.clear();
	//recoil_file->Close();

}

///////////////////////////////////////////////////////////////////////////////
/// Adds a binding energy from a string from the mass table to the ame_be
/// private variable. Called in the ISSReaction::ReadMassTables() function
/// \param[in] Ai The mass number
/// \param[in] Zi The proton number
/// \param[in] ame_be_str The relevant line from the AME 2020 evaluation file
void ISSReaction::AddBindingEnergy( short Ai, short Zi, TString ame_be_str ) {
	
	// A key for the isotope
	std::string isotope_key;
	isotope_key = std::to_string( Ai ) + gElName.at( Zi );
	
	// Remove # from AME data and replace with decimal point
	if ( ame_be_str.Contains("#") )
		ame_be_str.ReplaceAll("#",".");

	// An * means there is no data, fill with a 0
	if ( ame_be_str.Contains("*") )
		ame_be.insert( std::make_pair( isotope_key, 0 ) );
	
	// Otherwise add the real data
	else
		ame_be.insert( std::make_pair( isotope_key, ame_be_str.Atof() ) );
	
	return;
	
}

///////////////////////////////////////////////////////////////////////////////
/// Stores the binding energies per nucleon for each nucleus from the AME 
/// 2020 file
void ISSReaction::ReadMassTables() {

	// Input data file is in the source code
	// AME_FILE is passed as a definition at compilation time in Makefile
	std::ifstream input_file;
	input_file.open( AME_FILE );
	
	std::string line, BE_str, N_str, Z_str;
	std::string startline = "1N-Z";
	
	short Ai, Zi, Ni;

	// Loop over the file
	if( input_file.is_open() ){

		// Read first line
		std::getline( input_file, line );
		
		// Look for start of data
		while( line.substr( 0, startline.size() ) != startline ){
			
			// Read next line, but break if it's the end of the file
			if( !std::getline( input_file, line ) ){
				
				std::cout << "Can't read mass tables from ";
				std::cout << AME_FILE << std::endl;
				exit(1);
				
			}

		}
		
		// Read one more nonsense line with the units on
		std::getline( input_file, line );
		
		// Now process the data
		while( std::getline( input_file, line ) ){
			
			// Get mass excess from the line
			N_str = line.substr( 5, 5 );
			Z_str = line.substr( 9, 5 );
			BE_str = line.substr( 54, 13 );
			
			// Get N and Z
			Ni = std::stoi( N_str );
			Zi = std::stoi( Z_str );
			Ai = Ni + Zi;
			
			// Add mass value
			AddBindingEnergy( Ai, Zi, BE_str );
			
		}
		
	}
	
	else {
		
		std::cout << "Mass tables file doesn't exist: " << AME_FILE << std::endl;
		exit(1);
		
	}
	
	return;

}

///////////////////////////////////////////////////////////////////////////////
/// Reads the contents of the reaction file given via user input. Also calls 
/// ReadStoppingPowers function for each of the nuclides going through the 
/// different materials for later corrections.
void ISSReaction::ReadReaction() {

	TEnv *config = new TEnv( fInputFile.data() );
	
	std::string isotope_key;
	
	// Magnetic field stuff
	Mfield = config->GetValue( "Mfield", 2.0 );
	
	// Detector to target distances and dead layer of Si
	z0 = config->GetValue( "ArrayDistance", -100.0 );
	deadlayer = config->GetValue( "ArrayDeadlayer", 0.0004 ); // units of mm of Al

	// Get particle properties
	Beam.SetA( config->GetValue( "BeamA", 30 ) );
	Beam.SetZ( config->GetValue( "BeamZ", 12 ) );
	if( Beam.GetZ() < 0 || Beam.GetZ() >= (int)gElName.size() ){
		
		std::cout << "Not a recognised element with Z = ";
		std::cout << Beam.GetZ() << " (beam)" << std::endl;
		exit(1);
		
	}
	Beam.SetBindingEnergy( ame_be.at( Beam.GetIsotope() ) );

	Eb = config->GetValue( "BeamE", 8520.0 ); // in keV/u
	Eb *= Beam.GetMass_u(); // keV
	Beam.SetEnergyLab( Eb ); // keV
	
	Target.SetA( config->GetValue( "TargetA", 2 ) );
	Target.SetZ( config->GetValue( "TargetZ", 1 ) );
	Target.SetEnergyLab( 0.0 );
	if( Target.GetZ() < 0 || Target.GetZ() >= (int)gElName.size() ){
		
		std::cout << "Not a recognised element with Z = ";
		std::cout << Target.GetZ() << " (target)" << std::endl;
		exit(1);
		
	}
	Target.SetBindingEnergy( ame_be.at( Target.GetIsotope() ) );

	Ejectile.SetA( config->GetValue( "EjectileA", 1 ) );
	Ejectile.SetZ( config->GetValue( "EjectileZ", 1 ) );
	if( Ejectile.GetZ() < 0 || Ejectile.GetZ() >= (int)gElName.size() ){
		
		std::cout << "Not a recognised element with Z = ";
		std::cout << Ejectile.GetZ() << " (ejectile)" << std::endl;
		exit(1);
		
	}
	Ejectile.SetBindingEnergy( ame_be.at( Ejectile.GetIsotope() ) );

	Recoil.SetA( config->GetValue( "RecoilA", 31 ) );
	Recoil.SetZ( config->GetValue( "RecoilZ", 12 ) );
	if( Recoil.GetZ() < 0 || Recoil.GetZ() >= (int)gElName.size() ){
		
		std::cout << "Not a recognised element with Z = ";
		std::cout << Recoil.GetZ() << " (recoil)" << std::endl;
		exit(1);
		
	}
	Recoil.SetBindingEnergy( ame_be.at( Recoil.GetIsotope() ) );
	
	// Get recoil energy cut
	nrecoilcuts = set->GetNumberOfRecoilSectors();
	recoil_cut.resize( nrecoilcuts );
	recoilcutfile.resize( nrecoilcuts );
	recoilcutname.resize( nrecoilcuts );
	for( unsigned int i = 0; i < nrecoilcuts; ++i ) {
	
		recoilcutfile.at(i) = config->GetValue( Form( "RecoilCut_%d.File", i ), "NULL" );
		recoilcutname.at(i) = config->GetValue( Form( "RecoilCut_%d.Name", i ), "CUTG" );
		
		// Check if it is given by the user
		if( recoilcutfile.at(i) != "NULL" ) {
		
			recoil_file = new TFile( recoilcutfile.at(i).data(), "READ" );
			if( recoil_file->IsZombie() )
				std::cout << "Couldn't open " << recoilcutfile.at(i) << " correctly" << std::endl;
				
			else {
			
				if( !recoil_file->GetListOfKeys()->Contains( recoilcutname.at(i).data() ) )
					std::cout << "Couldn't find " << recoilcutname.at(i) << " in " << recoilcutfile.at(i) << std::endl;
				else
					recoil_cut.at(i) = (TCutG*)recoil_file->Get( recoilcutname.at(i).data() )->Clone();

			}
			
			recoil_file->Close();
			
		}
		
		// Assign an empty cut file if none is given, so the code doesn't crash
		if( !recoil_cut.at(i) ) recoil_cut.at(i) = new TCutG();
	
	}
	
	// Get E versus z cuts
	nevszcuts = config->GetValue( "NumberOfEvsZCuts", 1 );
	e_vs_z_cut.resize( nevszcuts );
	evszcutfile.resize( nevszcuts );
	evszcutname.resize( nevszcuts );
	for( unsigned int i = 0; i < nevszcuts; ++i ) {
	
		evszcutfile.at(i) = config->GetValue( Form( "EvsZCut_%d.File", i ), "NULL" );
		evszcutname.at(i) = config->GetValue( Form( "EvsZCut_%d.Name", i ), "CUTG" );
		
		// Check if it is given by the user
		if( evszcutfile.at(i) != "NULL" ) {
		
			e_vs_z_file = new TFile( evszcutfile.at(i).data(), "READ" );
			if( e_vs_z_file->IsZombie() )
				std::cout << "Couldn't open " << evszcutfile.at(i) << " correctly" << std::endl;
				
			else {
			
				if( !e_vs_z_file->GetListOfKeys()->Contains( evszcutname.at(i).data() ) )
					std::cout << "Couldn't find " << evszcutname.at(i) << " in " << evszcutfile.at(i) << std::endl;
				else
					e_vs_z_cut.at(i) = (TCutG*)e_vs_z_file->Get( evszcutname.at(i).data() )->Clone();

			}
			
			e_vs_z_file->Close();
			
		}
		
		// Assign an empty cut file if none is given, so the code doesn't crash
		if( !e_vs_z_cut.at(i) ) e_vs_z_cut.at(i) = new TCutG();
	
	}
	
	// EBIS time window
	double tmp_on = config->GetValue( "EBIS.On", 1.2e6 );		// normally 1.2 ms in slow extraction
	double tmp_off = config->GetValue( "EBIS.Off", 2.52e7 );	// this allows an off window 20 times bigger than on
	EBIS_On = config->GetValue( "EBIS_On", tmp_on );	// backwards compatibility
	EBIS_Off = config->GetValue( "EBIS_Off", tmp_off );	// backwards compatibility window 20 times bigger than on
	EBIS_ratio = config->GetValue( "EBIS.FillRatio", GetEBISTimeRatio() );	// this is the measured ratio of EBIS On/off. Default is just the time window ratio

	// T1 time window
	t1_min_time = config->GetValue( "T1.Min", 0 );		// default = 0
	t1_max_time = config->GetValue( "T1.Max", 1.2e9 );	// default = 1.2 seconds

	
	// Array-Recoil time windows
	array_recoil_prompt[0] = config->GetValue( "ArrayRecoil_PromptTime.Min", -300 );	// lower limit for array-recoil prompt time difference
	array_recoil_prompt[1] = config->GetValue( "ArrayRecoil_PromptTime.Max", 300 );		// upper limit for array-recoil prompt time difference
	array_recoil_random[0] = config->GetValue( "ArrayRecoil_RandomTime.Min", 600 );		// lower limit for array-recoil random time difference
	array_recoil_random[1] = config->GetValue( "ArrayRecoil_RandomTime.Max", 1200 );	// upper limit for array-recoil random time difference

	// Array-Recoil fill ratios
	array_recoil_ratio = config->GetValue( "ArrayRecoil_FillRatio", GetArrayRecoilTimeRatio() );

	// Target thickness and offsets
	target_thickness = config->GetValue( "TargetThickness", 0.200 ); // units of mg/cm^2
	x_offset = config->GetValue( "TargetOffset.X", 0.0 );	// of course this should be 0.0 if you centre the beam! Units of mm, vertical
	y_offset = config->GetValue( "TargetOffset.Y", 0.0 );	// of course this should be 0.0 if you centre the beam! Units of mm, horizontal

	// ELUM geometry
	elum_z    = config->GetValue( "ELUM.Distance", -1.0 ); // units of mm
	elum_rin  = config->GetValue( "ELUM.InnerRadius", 10.0 ); // units of mm
	elum_rout = config->GetValue( "ELUM.OuterRadius", 20.0 ); // units of mm

	// If it's a source run, we can ignore most of that
	// or better still, initialise everything and overwrite what we need
	if( flag_source ){
		
		Ejectile.SetA(4);
		Ejectile.SetZ(2);
		Ejectile.SetBindingEnergy( ame_be.at( Ejectile.GetIsotope() ) );
		Beam.SetEnergyLab(0.0);	// prevent any CM/Lab transformation
		target_thickness = 0.0; // no energy loss in target/source?
		
	}
	
	// Get the stopping powers in TGraphs
	stopping = true;
	for( unsigned int i = 0; i < 5; ++i ) {
		gStopping.push_back( std::make_unique<TGraph>() );
		gRange.push_back( std::make_unique<TGraph>() );
	}
	
	if( !flag_source ) {
		stopping &= ReadStoppingPowers( Beam.GetIsotope(), Target.GetIsotope(), gStopping[0], gRange[0] );
		stopping &= ReadStoppingPowers( Ejectile.GetIsotope(), Target.GetIsotope(), gStopping[1], gRange[1] );
	}
	stopping &= ReadStoppingPowers( Ejectile.GetIsotope(), "Al", gStopping[2], gRange[2] );

	// Get the electric and nuclear stopping powers for the PHC in a TGraph
	phcurves = true;
	phcurves &= ReadStoppingPowers( Ejectile.GetIsotope(), "Si", gStopping[3], gRange[3], true, false ); // electric only from SRIM files
	phcurves &= ReadStoppingPowers( Ejectile.GetIsotope(), "Si", gStopping[4], gRange[4], false, true ); // nuclear only from SRIM files
	
	// Get the PHC data in a TGraph
	gPHC = std::make_unique<TGraph>();
	gPHC_inv = std::make_unique<TGraph>();
	CalculatePulseHeightCorrection( Ejectile.GetIsotope() );

	// Some diagnostics and info
	if( !flag_source ) {
		
		std::cout << std::endl << " +++  ";
		std::cout << Beam.GetIsotope() << "(" << Target.GetIsotope() << ",";
		std::cout << Ejectile.GetIsotope() << ")" << Recoil.GetIsotope();
		std::cout << "  +++" << std::endl;
		std::cout << "Q-value = " << GetQvalue()*0.001 << " MeV" << std::endl;
		std::cout << "Incoming beam energy = ";
		std::cout << Beam.GetEnergyLab()*0.001 << " MeV" << std::endl;
		std::cout << "Target thickness = ";
		std::cout << target_thickness << " mg/cm^2" << std::endl;

	}
	else std::cout << std::endl << " +++  Alpha Source Run  +++";
	
	// Calculate the energy loss
	if( stopping ){
		
		// But only if it's not a source run
		if( !flag_source ) {
		
			double eloss = GetEnergyLoss( Beam.GetEnergyLab(), 0.5 * target_thickness, gStopping[0] );
			Beam.SetEnergyLab( Beam.GetEnergyLab() - eloss );
			std::cout << "Beam energy at centre of target = ";
			std::cout << Beam.GetEnergyLab()*0.001 << " MeV" << std::endl;

		}
		
	}
	else std::cout << "Stopping powers not calculated" << std::endl;

	
	// Do some ELUM calculations
	double theta_cm_inner, theta_cm_outer, theta_cm_centre;
	double energy_inner,   energy_outer,   energy_centre;
	if( elum_z > 0 ) {
		
		// Remember the real ejectile information
		int tmpA = Ejectile.GetA();
		int tmpZ = Ejectile.GetZ();
		double tmpBE = Ejectile.GetBindingEnergy();
		int tmpA2 = Recoil.GetA();
		int tmpZ2 = Recoil.GetZ();
		double tmpBE2 = Recoil.GetBindingEnergy();

		// Then pretend we have elastic scattering
		Ejectile.SetA( Target.GetA() );
		Ejectile.SetZ( Target.GetZ() );
		Ejectile.SetBindingEnergy( Target.GetBindingEnergy() );
		Recoil.SetA( Beam.GetA() );
		Recoil.SetZ( Beam.GetZ() );
		Recoil.SetBindingEnergy( Beam.GetBindingEnergy() );

		//std::cout << std::endl << " +++  ";
		//std::cout << Beam.GetIsotope() << "(" << Target.GetIsotope() << ",";
		//std::cout << Ejectile.GetIsotope() << ")" << Recoil.GetIsotope();
		//std::cout << "  +++" << std::endl;
		//std::cout << "Q-value = " << GetQvalue()*0.001 << " MeV" << std::endl;

		// Define interaction position of the inner/outer edges and centre
		TVector3 elum_inner_hit( elum_rin,  0.0, elum_z );
		TVector3 elum_outer_hit( elum_rout, 0.0, elum_z );
		TVector3 elum_centre_hit( 0.5*(elum_rout+elum_rin), 0.0, elum_z );

		// Simulate the elastic scattering reaction - inner
		SimulateReaction( elum_inner_hit );
		theta_cm_inner = Recoil.GetThetaCM();
		energy_inner = Ejectile.GetEnergyLab();

		// Simulate the elastic scattering reaction - outer
		SimulateReaction( elum_outer_hit );
		theta_cm_outer = Recoil.GetThetaCM();
		energy_outer = Ejectile.GetEnergyLab();

		// Simulate the elastic scattering reaction - centre
		SimulateReaction( elum_centre_hit );
		theta_cm_centre = Recoil.GetThetaCM();
		energy_centre = Ejectile.GetEnergyLab();

		// Change ejectile and recoils back again
		Ejectile.SetA( tmpA );
		Ejectile.SetZ( tmpZ );
		Ejectile.SetBindingEnergy( tmpBE );
		Recoil.SetA( tmpA2 );
		Recoil.SetZ( tmpZ2 );
		Recoil.SetBindingEnergy( tmpBE2 );

		std::cout << std::setprecision(5);
		std::cout << "ELUM found at " << elum_z << " mm" << std::endl;
		std::cout << " θ_cm = " << theta_cm_centre * TMath::RadToDeg();
		std::cout << " degrees; E_lab = " << energy_centre << " keV" << std::endl;
		std::cout << "\t" << theta_cm_inner * TMath::RadToDeg();
		std::cout << " < θ_cm  < " << theta_cm_outer * TMath::RadToDeg();
		std::cout << " degrees" << std::endl << "\t" << energy_inner;
		std::cout << " < E_lab < " << energy_outer << " keV" << std::endl;

	}
	
	// Finished
	delete config;

}

///////////////////////////////////////////////////////////////////////////////
/// Returns the energy loss at a given initial energy and distance travelled.
/// A negative distance will add the energy back on, i.e. travelling backwards.
/// This means that you will get a negative energy loss as a return value
/// This interpolates the TGraph using a spline
/// \param[in] Ei The initial energy of the nuclide
/// \param[in] dist The distance the nuclide travels in the target material
/// \param[in] g The TGraph of the energy loss generated in the ISSReaction::ReadReaction() function
/// \returns Ei-E The energy loss of a particular nuclide in a particular material
double ISSReaction::GetEnergyLoss( double Ei, double dist, std::unique_ptr<TGraph> &g ) {

	unsigned int Nmeshpoints = 50; // number of steps to take in integration
	double dx = dist/(double)Nmeshpoints;
	double E = Ei;
	
	for( unsigned int i = 0; i < Nmeshpoints; i++ ){

		if( E < 100. ) break; // when we fall below 100 keV we assume maximum energy loss
		E -= g->Eval(E) * dx;
		
	}
	
	return Ei - E;

}

///////////////////////////////////////////////////////////////////////////////
/// Returns the nuclear specific energy loss at a given initial energy and over the range travelled.
/// A negative distance will add the energy back on, i.e. travelling backwards.
/// This means that you will get a negative energy loss as a return value
/// This interpolates the TGraph using a spline
/// \param[in] Ei The initial energy of the nuclide
/// \param[in] range The range over which the nuclide travels in the target material
/// \param[in] gn The TGraph of the nuclear energy loss
/// \param[in] gtot The TGraph of the total energy loss
/// \returns En: The nuclear specific energy loss of a particular nuclide over the range
double ISSReaction::GetNuclearEnergyLoss( double Ei, double range, std::unique_ptr<TGraph> &gn, std::unique_ptr<TGraph> &gtot ) {

	unsigned int Nmeshpoints = 200; // number of steps to take in integration
	double dx = range/(double)Nmeshpoints;
	double E = Ei;
	double En = 0;
	
	for( unsigned int i = 0; i < Nmeshpoints; i++ ){

		if( E < 0.5 ) break; // when we fall below 0.5 keV we assume we're stopped
		E -= gtot->Eval(E) * dx;
		En += gn->Eval(E) * dx;
		
	}
	
	return En;

}


///////////////////////////////////////////////////////////////////////////////
/// Reads the stopping powers from SRIM files located in the directories, and 
/// makes a TGraph from the data within. Generates a pdf file of the reaction 
/// whenever it's called
/// \param[in] isotope1 The beam species
/// \param[in] isotope2 The target species
/// \param[in] g Pointer to TGraph object where stopping powers will be plotted
/// \param[in] r Pointer to TGraph object where range will be plotted
/// \param[in] electriconly Boolean to decide whether to include dE/dx for electric only
/// \param[in] nculearonly Boolean to decide whether to include dE/dx for nuclear only
/// \returns true/false depending on whether the function succeeds operating
bool ISSReaction::ReadStoppingPowers( std::string isotope1, std::string isotope2,
									 std::unique_ptr<TGraph> &g, std::unique_ptr<TGraph> &r,
									 bool electriconly, bool nuclearonly ) {

	/// Open stopping power files and make TGraphs of data
	
	// Change target material depending on species
	if( isotope2 == "1H" ) isotope2 = "CH2";
	if( isotope2 == "2H" ) isotope2 = "CD2";
	if( isotope2 == "3H" ) isotope2 = "tTi";

	// Make title for stopping
	std::string title = "Stopping powers for ";
	title += isotope1 + " in " + isotope2;
	title += ";" + isotope1 + " energy [keV];";
	title += "Energy loss in " + isotope2;
	if( isotope2 == "Si" || isotope2 == "Al" ) title += " [keV/mm]";
	else title += " [keV/(mg/cm^{2})]";
	g->SetTitle( title.c_str() );

	// Make title for range
	title = "Range of ";
	title += isotope1 + " in " + isotope2;
	title += ";" + isotope1 + " energy [keV];";
	title += "Range in " + isotope2;
	title += " [mm]";
	r->SetTitle( title.c_str() );

	// Keep things quiet from ROOT
	gErrorIgnoreLevel = kWarning;

	// Open the data file
	// SRIM_DIR is defined at compilation and is in source code
	std::string srimfilename = std::string( SRIM_DIR ) + "/";
	srimfilename += isotope1 + "_" + isotope2 + ".txt";
	
	std::ifstream input_file;
	input_file.open( srimfilename, std::ios::in );

	// If it fails to open print an error
	if( !input_file.is_open() ) {
		
		std::cerr << "Cannot open " << srimfilename << std::endl;
		return false;
		  
	}


	std::string line, units, length, tmp_str;
	std::stringstream line_ss;
	double En, nucl, elec, total, range, tmp_dbl;
	 
	// Test file format
	std::getline( input_file, line );
	if( line.substr( 3, 5 ) == "=====" ) {
		
		// Advance
		while( std::getline( input_file, line ) && !input_file.eof() ) {
			
			// Skip over the really short lines
			if( line.length() < 10 ) continue;
			
			// Check for the start of the data
			if( line.substr( 3, 5 ) == "-----" ) break;
			
		}
		
	}
	else {
		
		std::cerr << "Not a srim file: " << srimfilename << std::endl;
		return false;
		
	}

	// Read in the data
	while( std::getline( input_file, line ) && !input_file.eof() ) {
		
		// Skip over the really short lines
		if( line.length() < 10 ) continue;

		// If we've reached the end, stop
		if( line.substr( 3, 9 ) == "---------" ) break;
		
		// Read in data
		line_ss.clear();
		line_ss.str("");
		line_ss << line;
		line_ss >> En >> units >> elec >> nucl >> range >> length >> tmp_dbl >> tmp_str;
		
		if( units == "eV" ) En *= 1E-3;
		else if( units == "keV" ) En *= 1E0;
		else if( units == "MeV" ) En *= 1E3;
		else if( units == "GeV" ) En *= 1E6;
		
		if( length == "A" ) range *= 1E-7;
		else if( length == "um" ) range *= 1E-3;
		else if( length == "mm" ) range *= 1E0;
		else if( length == "m" ) range *= 1E3;
		else if( length == "km" ) range *= 1E6;

		if( electriconly ) total = elec; // electric stopping only
		else if( nuclearonly ) total = nucl; // nuclear stopping only
		else total = nucl + elec ; // in some units, conversion done later
		
		g->SetPoint( g->GetN(), En, total );
		r->SetPoint( g->GetN(), En, range );

	}
	
	// Get next line and check there are conversion factors
	std::getline( input_file, line );
	if( line.substr( 0, 9 ) != " Multiply" ){
		
		std::cerr << "Couldn't get conversion factors from ";
		std::cerr << srimfilename << std::endl;
		return false;
		
	}
	std::getline( input_file, line ); // next line is just ------

	// Get conversion factors
	double conv, conv_keVum, conv_MeVmgcm2;
	std::getline( input_file, line ); // first conversion is eV / Angstrom
	std::getline( input_file, line ); // keV / micron
	conv_keVum = std::stod( line.substr( 0, 15 ) );
	std::getline( input_file, line ); // MeV / mm
	std::getline( input_file, line ); // keV / (ug/cm2)
	std::getline( input_file, line ); // MeV / (mg/cm2)
	conv_MeVmgcm2 = std::stod( line.substr( 0, 15 ) );
	
	// Now convert all the points in the plot
	if( isotope2 == "Si" || isotope2 == "Al" ) conv = conv_keVum * 1E3; // silicon/aluminium thickness in mm, energy in keV
	else conv = conv_MeVmgcm2 * 1E3; // target thickness in mg/cm2, energy in keV
	for( Int_t i = 0; i < g->GetN(); ++i ){
		
		g->GetPoint( i, En, total );
		g->SetPoint( i, En, total*conv );
		
	}
	
	// Draw the plot and save it somewhere
	TCanvas *c = new TCanvas();
	c->SetLogx();
	g->Draw("A*");
	std::string pdfname = srimfilename.substr( 0, srimfilename.find_last_of(".") );
	if( electriconly ) pdfname += "_dedx_e.pdf";
	else if( nuclearonly ) pdfname += "_dedx_n.pdf";
	else pdfname += "_dedx.pdf";
	c->SaveAs( pdfname.c_str() );
	//c->SetLogy();
	r->Draw("A*");
	pdfname = srimfilename.substr( 0, srimfilename.find_last_of(".") ) + "_range.pdf";
	c->SaveAs( pdfname.c_str() );

	delete c;
	input_file.close();
	
	// ROOT can be noisy again
	gErrorIgnoreLevel = kInfo;

	return true;
	 
}

///////////////////////////////////////////////////////////////////////////////
/// Calculates energy correction required for the pulse height non-linearity, either to get true energy or detected energy
/// \param[in] Ei The initial energy that should be corrected
/// \param[in] detected Should be true if this is the detected energy and false if it is the actual ion energy
/// \return energy correction to obtain the true ion energy (if detected) or the charge collected (if !detected)
double ISSReaction::GetPulseHeightCorrection( double Ei, bool detected ) {

	// If we failed to read the data, return a zero correction value
	if( !phcurves ) return 0;

	if( detected ) return gPHC_inv->Eval(Ei); // get actual ion energy from PH (detected energy)
	else return gPHC->Eval(Ei); // get PH for actual ion energy

}

///////////////////////////////////////////////////////////////////////////////
/// Calculates the pulse height correction information from the SRIM data
void ISSReaction::CalculatePulseHeightCorrection( std::string isotope ) {
	 
	/// Open stopping power files and make TGraphs of data
	
	// Make titles
	std::string title = "Pulse height corrections for ";
	title += isotope;
	title += ";" + isotope + " energy after dead layer [keV];";
	title += "Deposited energy [keV/#mum]";
	std::string title_inv = "Pulse height corrections for ";
	title_inv += isotope;
	title_inv += ";Deposited energy [keV/#mum];";
	title_inv += isotope + " energy after dead layer [keV]";

	// Initialise an empty TGraph
	gPHC->SetTitle( title.c_str() );
	gPHC_inv->SetTitle( title_inv.c_str() );

	// Keep things quiet from ROOT
	gErrorIgnoreLevel = kWarning;

	// Read in the data to a temporary graph
	TGraph *gRes = new TGraph();

	// Calculate the stopping
	double E, Emax = 100e3;
	double range, dEdx_n, dEdx_e;

	// number of steps to take in integration
	unsigned int Nmeshpoints = 2e4;
	double dE = Emax/(double)Nmeshpoints;
	double Edet = 0.0;

	// Do the numerical integration
	for( unsigned int i = 0; i < Nmeshpoints; i++ ){

		E = (double)i + 0.5;
		E *= dE;
		
		// Evaluate stopping powers and range
		range = gRange[3]->Eval(E);
		dEdx_e = gStopping[3]->Eval(E);

		// Calculate the nuclear stopping
		// NB: this is inefficient, but still only takes a couple of seconds
		// ideally the integrated nuclear energy loss versus energy would be
		// calculated only once and stored as a TGraph or something. Who cares?
		dEdx_n = GetNuclearEnergyLoss( E, range, gStopping[4], gStopping[2] );
		
		// From W. N. Lennard et al. NIM A248 (1986) 454
		double PHC = e0_Si - k_Si * dEdx_e;
		PHC = e0_Si / PHC;
		Edet += dE * PHC;
		
		//if( E > 2000. && E < 2010. ) {
		//	std::cout << "Energy = " << E << " keV" << std::endl;
		//	std::cout << "Range = " << range << " mm" << std::endl;
		//	std::cout << "dEdx_e = " << dEdx_e << " keV/mm" << std::endl;
		//	std::cout << "dEdx_n = " << dEdx_n << " keV" << std::endl;
		//	std::cout << "PHC = " << PHC << " keV" << std::endl;
		//	std::cout << "Edet = " << Edet << " keV" << std::endl;
		//}
		
		gPHC->SetPoint( gPHC->GetN(), E + dEdx_n, Edet );
		gPHC_inv->SetPoint( gPHC_inv->GetN(), Edet, E + dEdx_n );
		gRes->SetPoint( gRes->GetN(), E + dEdx_n, Edet - E - dEdx_n );

	}
	
	// Draw the plot and save it somewhere
	TCanvas *c = new TCanvas();
	//c->SetLogx();
	//c->SetLogy();
	gRes->Draw("AP");
	gRes->GetYaxis()->SetTitle("E_{0} - E_{detected} (keV)");
	gRes->GetXaxis()->SetTitle("E_{0} (keV)");
	std::string pdfname = std::string( SRIM_DIR ) + "/" + isotope + "_phc.pdf";
	c->SaveAs( pdfname.c_str() );

	delete c;
	delete gRes;
	
	// ROOT can be noisy again
	gErrorIgnoreLevel = kInfo;
	 
}


///////////////////////////////////////////////////////////////////////////////
/// This function will use the interaction position and decay energy of an ejectile
/// event, to solve the kinematics and define parameters such as:
/// theta_lab, etc. It returns the detected energy of the ejectile
/// \param[in] vec The position of the interaction with the detector
/// \param[in] en The energy of the decay
/// \returns en-eloss
float ISSReaction::SimulateDecay( TVector3 vec, double en ){

	// Apply the X and Y offsets directly to the TVector3 input
	// We move the array opposite to the target, which replicates the same
	// geometrical shift that is observed with respect to the beam
	vec.SetX( vec.X() - x_offset );
	vec.SetY( vec.Y() - y_offset );

	// Set the input parameters, might use them in another function
	Ejectile.SetEnergyLab(en);			// ejectile energy in keV
	z_meas = vec.Z();					// measured z in mm
	if( z0 < 0 ) z_meas = z0 - z_meas;	// upstream
	else z_meas += z0;					// downstream

    //------------------------//
    // Kinematics calculation //
    //------------------------//
	params[0] = z_meas;										// z in mm
	params[1] = vec.Perp();									// r_meas in mm
	params[2] = Ejectile.GetMomentumLab();					// p3
	params[3] = (float)Ejectile.GetZ() * GetField_corr(); 	// qb
	params[3] /= TMath::TwoPi(); 							// qb/2pi
		
	// Set parameters
	fa->SetParameters( params );
	fb->SetParameters( params );
	
	// Build the function and derivative, then solve for z
	gErrorIgnoreLevel = kBreak; // suppress warnings and errors, but not breaks
	ROOT::Math::GradFunctor1D wf( *fa, *fb );
	rf->SetFunction( wf, z_meas );
	rf->Solve( 500, 1e-5, 1e-6 );

	// Check result
	if( rf->Status() ){
		z = TMath::QuietNaN();
	}
	else z = rf->Root();
	gErrorIgnoreLevel = kInfo; // print info and above again

	// Calculate the lab angle from z position (Butler method)
	alpha  = (float)Ejectile.GetZ() * GetField_corr(); 	// qb
	alpha /= TMath::TwoPi(); 							// qb/2pi
	alpha *= z / Ejectile.GetMomentumLab();				// * z/p
	alpha  = TMath::ASin( alpha );
	Ejectile.SetThetaLab( TMath::PiOver2() + alpha );
	
	// Calculate the energy loss
	// Distance is postive because energy is lost
	double dist = 1.0 * deadlayer / TMath::Abs( TMath::Cos( alpha ) );
	double eloss = GetEnergyLoss( en, dist, gStopping[2] );
	
	//std::cout << "z = " << z_meas << " mm, angle = ";
	//std::cout << alpha*TMath::RadToDeg() << " deg, dead layer = ";
	//std::cout << dist*1e3 << " µm: " << en << " - " << eloss << std::endl;
	
	return en - eloss;

}

///////////////////////////////////////////////////////////////////////////////
/// This function  uses the interaction position of an ejectile
/// to solve the reaction kinematics and define parameters such as:
/// theta_cm, theta_lab,, E_lab, E_det, etc.
/// One must set things like beam energy, excitation energy, etc, in advance
/// \param[in] vec detection position vector
void ISSReaction::SimulateReaction( TVector3 vec ){

	// Apply the X and Y offsets directly to the TVector3 input
	// We move the array opposite to the target, which replicates the same
	// geometrical shift that is observed with respect to the beam
	vec.SetX( vec.X() - x_offset );
	vec.SetY( vec.Y() - y_offset );

	// Set the input parameters, might use them in another function
	//Ejectile.SetEnergyLab(en);		// ejectile energy in keV
	z_meas = vec.Z();					// measured z in mm
	r_meas = vec.Perp();				// measured radius

	//------------------------//
    // Kinematics calculation //
    //------------------------//
	params[0] = z_meas;										// z in mm
	params[1] = r_meas;										// r_meas in mm
	params[2] = Recoil.GetEx();								// Ex
	params[3] = (float)Ejectile.GetZ() * GetField_corr(); 	// qb
	params[3] /= TMath::TwoPi(); 							// qb/2pi
	params[4] = Beam.GetEnergyLab();						// beam energy
	params[5] = Beam.GetMass();
	params[6] = Target.GetMass();
	params[7] = Ejectile.GetMass();
	params[8] = Recoil.GetMass();

	//for( int i = 0; i < 9; ++i )
	//	std::cout << "params[" << i << "] = " << params[i] << ";" << std::endl;

	// Set parameters
	fsim->SetParameters( params );
	dsim->SetParameters( params );

	// Build the theta_cm function, then solve
	gErrorIgnoreLevel = kBreak; // suppress warnings and errors, but not breaks
	ROOT::Math::GradFunctor1D wf( *fsim, *dsim );
	if( rfsim->SetFunction( wf, TMath::PiOver4() ) ) {
		rfsim->Solve( 500, 1e-5, 1e-6 );
		theta_cm = rfsim->Root();
	}
	else theta_cm = TMath::QuietNaN();
	gErrorIgnoreLevel = kInfo; // print info and above again

	// Set the theta_cm of the ejectile
	Ejectile.SetThetaCM( TMath::Pi() - theta_cm );
	Recoil.SetThetaCM( theta_cm );

	// What about the energy of the ejectile after the collision?
	e3_cm = TMath::Power( GetEnergyTotCM(), 2.0 );
	e3_cm -= TMath::Power( Recoil.GetMass() + Recoil.GetEx(), 2.0 );
	e3_cm += TMath::Power( Ejectile.GetMass(), 2.0 );
	e3_cm /= 2.0 * GetEnergyTotCM();
	Ejectile.SetEnergyTotCM( e3_cm );
	
	// From Daniel Clarke:
	theta_lab = Ejectile.GetMomentumCM() * TMath::Sin( Ejectile.GetThetaCM() );
	theta_lab /= Ejectile.GetMomentumCM() * TMath::Cos( Ejectile.GetThetaCM() ) + GetBeta() * Ejectile.GetEnergyTotCM();
	theta_lab /= GetGamma();
	theta_lab = TMath::ATan( theta_lab );
	Ejectile.SetThetaLab( theta_lab );

	// Energy of ejectile in the lab
	double e3_lab = TMath::Cos( Ejectile.GetThetaCM() );
	e3_lab *= GetBeta() * Ejectile.GetMomentumCM();
	e3_lab += Ejectile.GetEnergyTotCM();
	e3_lab *= GetGamma();
	Ejectile.SetEnergyLab( e3_lab - Ejectile.GetMass() );

}

///////////////////////////////////////////////////////////////////////////////
/// This function will use the interaction position and detected energy of an ejectile
/// event, to solve the reaction kinematics and define parameters such as:
/// theta_cm, theta_lab, Ex, E_lab, etc.
/// \param[in] vec The interaction point
/// \param[in] en The initial energy of the ejectile
void ISSReaction::MakeReaction( TVector3 vec, double en ){
	
	// Apply the X and Y offsets directly to the TVector3 input
	// We move the array opposite to the target, which replicates the same
	// geometrical shift that is observed with respect to the beam
	vec.SetX( vec.X() - x_offset );
	vec.SetY( vec.Y() - y_offset );
	
	// Set the input parameters, might use them in another function
	//Ejectile.SetEnergyLab(en);		// ejectile energy in keV
	z_meas = vec.Z();					// measured z in mm
	r_meas = vec.Perp();				// measured radius
	if( z0 < 0 ) z_meas = z0 - z_meas;	// upstream
	else z_meas += z0;					// downstream
	
	// Pulse height conversion to proton energy using RDP
	en = GetPulseHeightCorrection( en, true );
	Ejectile.SetEnergyLab(en);

	//------------------------//
    // Kinematics calculation //
    //------------------------//
	params[0] = z_meas;										// z in mm
	params[1] = r_meas;										// r_meas in mm
	params[2] = Ejectile.GetMomentumLab();					// p
	params[3] = (float)Ejectile.GetZ() * GetField_corr(); 	// qb
	params[3] /= TMath::TwoPi(); 							// qb/2pi
	
	//for( unsigned int i = 0; i < 4; ++i )
	//	std::cout << "params[" << i << "] = " << params[i] << std::endl;
	
	// Apply the energy loss correction and solve again
	// Keep going for 50 iterations or until we are better than 0.01% change
	unsigned int iter = 0;
	gErrorIgnoreLevel = kBreak; // suppress warnings and errors, but not breaks

#ifdef butler_algorithm
	z = z_meas;
	double z_prev = 0.0;

	while( TMath::Abs( ( z - z_prev ) / z ) > 0.00001 && iter < 50 ) {

		// Calculate the lab angle from z position (Butler method)
		alpha  = (float)Ejectile.GetZ() * GetField_corr(); 	// qb
		alpha /= TMath::TwoPi(); 							// qb/2pi
		alpha *= z;											// * z
		alpha /= Ejectile.GetMomentumLab();					// over p
		alpha  = TMath::ASin( alpha );
		
#else
	alpha = TMath::PiOver4();
	double alpha_prev = 9999.;
	
	while( TMath::Abs( ( alpha - alpha_prev ) / alpha ) > 0.0001 && iter < 50 ) {
#endif

		// Distance is negative because energy needs to be recovered
		// First we recover the energy lost in the Si dead layer
		double dist = -1.0 * deadlayer / TMath::Abs( TMath::Cos( alpha ) );
		double eloss = GetEnergyLoss( en, dist, gStopping[2] );
		Ejectile.SetEnergyLab( en - eloss );
		
		// Then we recover the energy lost in the target
		dist = -0.5 * target_thickness / TMath::Abs( TMath::Sin( alpha ) );
		eloss = GetEnergyLoss( Ejectile.GetEnergyLab(), dist, gStopping[1] );
		Ejectile.SetEnergyLab( Ejectile.GetEnergyLab() - eloss );

		// Set parameters
		params[2] = Ejectile.GetMomentumLab(); // p
		fa->SetParameters( params );
		fb->SetParameters( params );
		ROOT::Math::GradFunctor1D wf( *fa, *fb );

#ifdef butler_algorithm
		// Use Butler's method and solve the root
		z_prev = z;
		rf->SetFunction( wf, z_meas );
		rf->Solve( 500, 1e-5, 1e-6 );
		
		// Check result
		if( rf->Status() ){
			z = TMath::QuietNaN();
			break;
		}
		else z = rf->Root();
#else
		// Build the alpha function and derivative, then solve
		alpha_prev = alpha;
		rf->SetFunction( wf, 0.2 * TMath::Pi() ); // with derivatives
		rf->Solve( 500, 1e-5, 1e-6 );
		
		// Check result
		if( rf->Status() ){
			alpha = TMath::QuietNaN();
			break;
		}
		else alpha = rf->Root();
#endif
		
		iter++;
			
	}
	
	gErrorIgnoreLevel = kInfo; // print info and above again

#ifdef butler_algorithm
	// Calculate the lab angle from z position (Butler method)
	alpha  = (float)Ejectile.GetZ() * GetField_corr(); 	// qb
	alpha /= TMath::TwoPi(); 							// qb/2pi
	alpha *= z;											// * z
	alpha /= Ejectile.GetMomentumLab();					// over p
	alpha  = TMath::ASin( alpha );
	theta_lab = alpha;
	if( z_meas < 0 ) theta_lab += TMath::PiOver2();
	Ejectile.SetThetaLab( theta_lab );
#else
	// Get the real z value at beam axis and lab angle (alpha method)
	if( z_meas < 0 ) z = z_meas - r_meas * TMath::Tan( alpha );
	else z = z_meas + r_meas * TMath::Tan( alpha );
	Ejectile.SetThetaLab( TMath::PiOver2() + alpha );
#endif

	// Total energy of ejectile in centre of mass
	e3_cm = Ejectile.GetEnergyTotLab();
	e3_cm -= GetBeta() * Ejectile.GetMomentumLab() * TMath::Sin( alpha );
	e3_cm *= GetGamma();
	Ejectile.SetEnergyTotCM( e3_cm );
	Recoil.SetEnergyTotCM( GetEnergyTotCM() - e3_cm );

	// Theta_CM
	theta_cm  = Ejectile.GetEnergyTotCM();
	theta_cm -= Ejectile.GetEnergyTotLab() / GetGamma();
	theta_cm /= GetBeta() * Ejectile.GetMomentumCM();
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
	
	
	
	// Debug output
	if( z > 0 || z < 0 ){
	
		//std::cout << "z_meas = " << z_meas << std::endl;
		//std::cout << "z_corr = " << z << std::endl;
		//std::cout << "Ep = " << en << std::endl;
		//std::cout << "alpha = " << alpha*TMath::RadToDeg() << std::endl;
		//std::cout << "e3_cm = " << e3_cm << std::endl;
		//std::cout << "e4_cm = " << Recoil.GetEnergyTotCM() << std::endl;
		//std::cout << "etot_cm = " << GetEnergyTotCM() << std::endl;
		//std::cout << "etot_lab = " << GetEnergyTotLab() << std::endl;
		//std::cout << "gamma = " << GetGamma() << std::endl;
		//std::cout << "beta = " << GetBeta() << std::endl;
		//std::cout << "theta_cm = " << Recoil.GetThetaCM()*TMath::RadToDeg() << std::endl;
		//std::cout << "Ex = " << Recoil.GetEx() << std::endl;
		//std::cout << std::endl;
	
	}

  	return;	

}

