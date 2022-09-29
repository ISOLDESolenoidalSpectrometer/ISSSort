#include "Reaction.hh"

ClassImp( ISSParticle )
ClassImp( ISSReaction )

double alpha_function( double *x, double *params ){

	// Equation to solve for alpha, LHS = 0
	double alpha = x[0];
	double z = params[0];
	double rho = params[1];
	double p = params[2];
	double qb = params[3];
	
	double root = p * TMath::Sin(alpha);
	root -= qb * rho * TMath::Tan(alpha);
	root -= qb * z;

	return root;

}

double alpha_derivative( double *x, double *params ){

	// Derivative of the alpha equation
	double alpha = x[0];
	//double z = params[0]; // unused in derivative
	double rho = params[1];
	double p = params[2];
	double qb = params[3];
	
	double root = p * TMath::Cos(alpha);
	root -= qb * rho / TMath::Cos(alpha) / TMath::Cos(alpha);

	return root;

}

// This solves for z using Peter Butler's method
double butler_function( double *x, double *params ){

	// Equation to solve for z, LHS = 0
	double z = x[0];
	double z_meas = params[0];
	double rho = params[1];
	double p = params[2];
	double qb = params[3];
	
	
	double root = rho * qb;
	root /= p * TMath::Sin( TMath::ACos( qb * z / p ) );
	root = TMath::ASin( root );
	root /= TMath::Pi();
	root -= 1.0;
	root *= z;
	root += z_meas;

	return root;

}

// Reaction things
ISSReaction::ISSReaction( std::string filename, ISSSettings *myset, bool source ){
		
	// Read in mass tables
	ReadMassTables();
	
	// Check if it's a source run
	flag_source = source;

	// Get the info from the user input
	set = myset;
	SetFile( filename );
	ReadReaction();
	
	// Root finder algorithm - for alpha like Ryan does
	fa = std::make_unique<TF1>( "alpha_function", alpha_function, 0.0, TMath::Pi()/2.0, 4 );
	fb = std::make_unique<TF1>( "alpha_derivative", alpha_derivative, 0.0, TMath::Pi()/2.0, 4 );
	rf = std::make_unique<ROOT::Math::RootFinder>( ROOT::Math::RootFinder::kGSL_NEWTON );
	
	// Root finder algorithm - The Peter Butler method
	//double low_limit = z0;
	//double upp_limit = z0;
	//if( z0 < 0.0 ) low_limit -= 600.0;
	//else upp_limit += 600.0;
	//fa = std::make_unique<TF1>( "butler_function", butler_function, low_limit, upp_limit, 4 );
	//rf = std::make_unique<ROOT::Math::RootFinder>( ROOT::Math::RootFinder::kGSL_NEWTON );
	
}

ISSReaction::~ISSReaction(){

	for( unsigned int i = 0; i < recoil_cut.size(); ++i )
		delete (recoil_cut[i]);
	recoil_cut.clear();
	//recoil_file->Close();

}

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

void ISSReaction::ReadReaction() {

	TEnv *config = new TEnv( fInputFile.data() );
	
	std::string isotope_key;
	
	// Magnetic field stuff
	Mfield = config->GetValue( "Mfield", 2.0 );
	
	// Detector to target distances and dead layer of Si
	z0 = config->GetValue( "ArrayDistance", 100.0 );
	deadlayer = config->GetValue( "ArrayDeadlayer", 0.0005 ); // units of mm of Si

	// Get particle properties
	Beam.SetA( config->GetValue( "BeamA", 30 ) );
	Beam.SetZ( config->GetValue( "BeamZ", 12 ) );
	if( Beam.GetZ() < 0 || Beam.GetZ() >= (int)gElName.size() ){
		
		std::cout << "Not a recognised element with Z = ";
		std::cout << Beam.GetZ() << " (beam)" << std::endl;
		exit(1);
		
	}
	Beam.SetBindingEnergy( ame_be.at( Beam.GetIsotope() ) );

	Eb = config->GetValue( "BeamE", 8520.0 ); // in keV/A
	Eb *= Beam.GetA(); // keV
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
	EBIS_On = config->GetValue( "EBIS_On", 1.2e6 );		// normally 1.2 ms in slow extraction
	EBIS_Off = config->GetValue( "EBIS_Off", 2.52e7 );	// this allows a off window 20 times bigger than on

	// Target thickness and offsets
	target_thickness = config->GetValue( "TargetThickness", 0.200 ); // units of mg/cm^2
	x_offset = config->GetValue( "TargetOffset.X", 0.0 );	// of course this should be 0.0 if you centre the beam! Units of mm, vertical
	y_offset = config->GetValue( "TargetOffset.Y", 0.0 );	// of course this should be 0.0 if you centre the beam! Units of mm, horizontal

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
	for( unsigned int i = 0; i < 3; ++i )
		gStopping.push_back( std::make_unique<TGraph>() );
	
	if( !flag_source ) {
		stopping &= ReadStoppingPowers( Beam.GetIsotope(), Target.GetIsotope(), gStopping[0] );
		stopping &= ReadStoppingPowers( Ejectile.GetIsotope(), Target.GetIsotope(), gStopping[1] );
	}
	stopping &= ReadStoppingPowers( Ejectile.GetIsotope(), "Si", gStopping[2] );

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

	// Finished
	delete config;

}

double ISSReaction::GetEnergyLoss( double Ei, double dist, std::unique_ptr<TGraph> &g ) {

	/// Returns the energy loss at a given initial energy and distance travelled
	/// A negative distance will add the energy back on, i.e. travelling backwards
	/// This means that you will get a negative energy loss as a return value
	unsigned int Nmeshpoints = 50; // number of steps to take in integration
	double dx = dist/(double)Nmeshpoints;
	double E = Ei;
	
	for( unsigned int i = 0; i < Nmeshpoints; i++ ){

		if( E < 100. ) break; // when we fall below 100 keV we assume maximum energy loss
		E -= g->Eval(E) * dx;
		
	}
	
	return Ei - E;

}

bool ISSReaction::ReadStoppingPowers( std::string isotope1, std::string isotope2, std::unique_ptr<TGraph> &g ) {
	 
	/// Open stopping power files and make TGraphs of data
	
	// Change target material depending on species
	if( isotope2 == "1H" ) isotope2 = "CH2";
	if( isotope2 == "2H" ) isotope2 = "CD2";
	if( isotope2 == "3H" ) isotope2 = "tTi";

	// Make title
	std::string title = "Stopping powers for ";
	title += isotope1 + " in " + isotope2;
	title += ";" + isotope1 + " energy [keV];";
	title += "Energy loss in " + isotope2;
	if( isotope2 == "Si" ) title += " [keV/#mum]";
	else title += " [keV/(mg/cm^{2})]";
	
	// Initialise an empty TGraph
	g->SetTitle( title.c_str() );

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


	std::string line, units, tmp_str;
	std::stringstream line_ss;
	double En, nucl, elec, total, tmp_dbl;
	 
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

		// Read in data
		line_ss.str("");
		line_ss << line;
		line_ss >> En >> units >> nucl >> elec >> tmp_dbl >> tmp_str >> tmp_dbl >> tmp_str;
		
		if( units == "eV" ) En *= 1E-3;
		else if( units == "keV" ) En *= 1E0;
		else if( units == "MeV" ) En *= 1E3;
		else if( units == "GeV" ) En *= 1E6;
		
		total = nucl + elec ; // in some units, conversion done later
		
		g->SetPoint( g->GetN(), En, total );
		
		// If we've reached the end, stop
		if( line.substr( 3, 9 ) == "---------" ) break;
		
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
	if( isotope2 == "Si" ) conv = conv_keVum * 1E3; // silicon thickness in mm, energy in keV
	else conv = conv_MeVmgcm2 * 1E3; // target thickness in mg/cm2, energy in keV
	for( Int_t i = 0; i < g->GetN(); ++i ){
		
		g->GetPoint( i, En, total );
		g->SetPoint( i, En, total*conv );
		
	}
	
	// Draw the plot and save it somewhere
	TCanvas *c = new TCanvas();
	c->SetLogx();
	//c->SetLogy();
	g->Draw("A*");
	std::string pdfname = srimfilename.substr( 0, srimfilename.find_last_of(".") ) + ".pdf";
	c->SaveAs( pdfname.c_str() );
	
	delete c;
	input_file.close();
	
	// ROOT can be noisey again
	gErrorIgnoreLevel = kInfo;

	return true;
	 
}

float ISSReaction::SimulateDecay( TVector3 vec, double en ){

	/// This function will use the interaction position and decay energy of an ejectile
	/// event, to solve the kinematics and define parameters such as:
	/// theta_lab, etc. It returns the detected energy of the ejectile

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
	rho = vec.Perp();					// perpenicular distance from beam axis to interaction point

    //------------------------//
    // Kinematics calculation //
    //------------------------//
	params[0] = z_meas;										// z in mm
	params[1] = rho;										// rho in mm
	params[2] = Ejectile.GetMomentumLab();					// p
	params[3] = (float)Ejectile.GetZ() * GetField_corr(); 	// qb
	params[3] /= TMath::TwoPi(); 							// qb/2pi
		
	// Set parameters
	fa->SetParameters( params );
	fb->SetParameters( params ); // not used in Butler method
	
	// Build the function and derivative, then solve (for alpha)
	gErrorIgnoreLevel = kBreak; // suppress warnings and errors, but not breaks
	ROOT::Math::GradFunctor1D wf( *fa, *fb );
	rf->SetFunction( wf, 0.2 * TMath::Pi() ); // with derivatives
	rf->Solve( 500, 1e-5, 1e-6 );
	
	// Or use Butler's method
	//ROOT::Math::Functor1D wf( *fa ); // Butler method
	//rf->SetFunction( wf, z - 100., z + 100. ); // without derivatives
	//rf->Solve( 500, 1e-5, 1e-6 );

	// Check result
	if( rf->Status() ){
		//z = TMath::QuietNaN();
		alpha = TMath::QuietNaN();
	}
	//else z = rf->Root();
	else alpha = rf->Root();
	gErrorIgnoreLevel = kInfo; // print info and above again

	// Calculate the lab angle from z position (Butler method)
	//alpha  = (float)Ejectile.GetZ() * GetField_corr(); 	// qb
	//alpha /= TMath::TwoPi(); 							// qb/2pi
	//alpha *= z / Ejectile.GetMomentumLab();				// * z/p
	//alpha  = TMath::ACos( alpha );

	// Get the real z value at beam axis and lab angle
	if( z_meas < 0 ) z = z_meas - rho * TMath::Tan( alpha );
	else z = z_meas + rho * TMath::Tan( alpha );
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


void ISSReaction::SimulateReaction( TVector3 vec, double ex ){

	/// This function will use the interaction position and excitaion energy of an ejectile
	/// event, to solve the reaction kinematics and define parameters such as:
	/// theta_cm, theta_lab,, E_lab, E_det, etc.

	
}

void ISSReaction::MakeReaction( TVector3 vec, double en ){
	
	/// This function will use the interaction position and detected energy of an ejectile
	/// event, to solve the reaction kinematics and define parameters such as:
	/// theta_cm, theta_lab, Ex, E_lab, etc.
	
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
	rho = vec.Perp();					// perpenicular distance from beam axis to interaction point
    
	//------------------------//
    // Kinematics calculation //
    //------------------------//
	params[0] = z_meas;										// z in mm
	params[1] = rho;										// rho in mm
	params[2] = Ejectile.GetMomentumLab();					// p
	params[3] = (float)Ejectile.GetZ() * GetField_corr(); 	// qb
	params[3] /= TMath::TwoPi(); 							// qb/2pi
	
	// Apply the energy loss correction and solve again
	// Keep going for 50 iterations or until we are better than 0.01% change
	alpha = 0.5 * TMath::PiOver2();
	double alpha_prev = 9999.;
	//z = z_meas;
	//double z_prev = 0.0;
	unsigned int iter = 0;

	gErrorIgnoreLevel = kBreak; // suppress warnings and errors, but not breaks
	//while( TMath::Abs( ( z - z_prev ) / z ) > 0.0001 && iter < 50 ) {
	while( TMath::Abs( ( alpha - alpha_prev ) / alpha ) > 0.0001 && iter < 50 ) {

		// Calculate the lab angle from z position (Butler method)
		//alpha  = (float)Ejectile.GetZ() * GetField_corr(); 	// qb
		//alpha /= TMath::TwoPi(); 							// qb/2pi
		//alpha *= z / Ejectile.GetMomentumLab();				// * z/p
		//alpha  = TMath::ACos( alpha );

		// Distance is negative because energy needs to be recovered
		// First we recover the energy lost in the Si dead layer
		double dist = -1.0 * deadlayer / TMath::Abs( TMath::Cos( alpha ) );
		double eloss = GetEnergyLoss( en, dist, gStopping[2] );
		Ejectile.SetEnergyLab( en - eloss );
		
		// First we recover the energy lost in the target
		dist = -0.5 * target_thickness / TMath::Abs( TMath::Sin( alpha ) );
		eloss = GetEnergyLoss( Ejectile.GetEnergyLab(), dist, gStopping[1] );
		Ejectile.SetEnergyLab( Ejectile.GetEnergyLab() - eloss );
		
		// Set parameters
		alpha_prev = alpha;
		//z_prev = z;
		params[2] = Ejectile.GetMomentumLab(); // p
		fa->SetParameters( params );
		fb->SetParameters( params ); // not calculated for Butler function
		
		// Build the alpha function and derivative, then solve
		ROOT::Math::GradFunctor1D wf( *fa, *fb ); // alpha method
		rf->SetFunction( wf, 0.2 * TMath::Pi() ); // with derivatives
		rf->Solve( 500, 1e-5, 1e-6 );
		
		// Or use Butler's method
		//ROOT::Math::Functor1D wf( *fa ); // Butler method
		//rf->SetFunction( wf, z - 100., z + 100. ); // without derivatives
		//rf->Solve( 500, 1e-5, 1e-6 );
		
		// Check result
		if( rf->Status() ){
			//z = TMath::QuietNaN();
			alpha = TMath::QuietNaN();
			break;
		}
		//else z = rf->Root();
		else alpha = rf->Root();
		
		//std::cout << "Iter " << iter << ": z = " << z << std::endl;
		
		iter++;
			
	}
	
	gErrorIgnoreLevel = kInfo; // print info and above again

	// Get the real z value at beam axis and lab angle (alpha method)
	if( z_meas < 0 ) z = z_meas - rho * TMath::Tan( alpha );
	else z = z_meas + rho * TMath::Tan( alpha );
	Ejectile.SetThetaLab( TMath::PiOver2() + alpha );
	
	// Calculate the lab angle from z position (Butler method)
	//alpha  = (float)Ejectile.GetZ() * GetField_corr(); 	// qb
	//alpha /= TMath::TwoPi(); 							// qb/2pi
	//alpha *= z / Ejectile.GetMomentumLab();				// * z/p
	//alpha  = TMath::ACos( alpha );
	//theta_lab  = alpha + TMath::PiOver2();
	//Ejectile.SetThetaLab( theta_lab );

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
	//if( z > 0 || z < 0 ){
	//
	//	std::cout << "z_meas = " << z_meas << std::endl;
	//	std::cout << "z_corr = " << z << std::endl;
	//	std::cout << "Ep = " << en << std::endl;
	//	std::cout << "alpha = " << alpha*TMath::RadToDeg() << std::endl;
	//	std::cout << "e3_cm = " << e3_cm << std::endl;
	//	std::cout << "e4_cm = " << Recoil.GetEnergyTotCM() << std::endl;
	//	std::cout << "etot_cm = " << GetEnergyTotCM() << std::endl;
	//	std::cout << "etot_lab = " << GetEnergyTotLab() << std::endl;
	//	std::cout << "gamma = " << GetGamma() << std::endl;
	//	std::cout << "beta = " << GetBeta() << std::endl;
	//	std::cout << "theta_cm = " << Recoil.GetThetaCM()*TMath::RadToDeg() << std::endl;
	//	std::cout << "Ex = " << Recoil.GetEx() << std::endl;
	//	std::cout << std::endl;
	//
	//}

  	return;	

}

