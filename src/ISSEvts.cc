#include "ISSEvts.hh"

ClassImp(ISSArrayEvt)
ClassImp(ISSArrayPEvt)
ClassImp(ISSRecoilEvt)
ClassImp(ISSMwpcEvt)
ClassImp(ISSElumEvt)
ClassImp(ISSZeroDegreeEvt)
ClassImp(ISSGammaRayEvt)
ClassImp(ISSEvts)


// ---------- //
// ISS events //
// ---------- //
ISSEvts::ISSEvts(){}
ISSEvts::~ISSEvts(){}

void ISSEvts::ClearEvt() {
	
	array_event.clear();
	arrayp_event.clear();
	recoil_event.clear();
	mwpc_event.clear();
	elum_event.clear();
	zd_event.clear();
	gamma_event.clear();

	std::vector<ISSArrayEvt>().swap(array_event);
	std::vector<ISSArrayPEvt>().swap(arrayp_event);
	std::vector<ISSRecoilEvt>().swap(recoil_event);
	std::vector<ISSMwpcEvt>().swap(mwpc_event);
	std::vector<ISSElumEvt>().swap(elum_event);
	std::vector<ISSZeroDegreeEvt>().swap(zd_event);
	std::vector<ISSGammaRayEvt>().swap(gamma_event);
	
	ebis = -999;
	t1 = -999;

	return;

}

void ISSEvts::AddEvt( std::shared_ptr<ISSArrayEvt> event ) {
	
	// Make a copy of the event and push it back
	ISSArrayEvt fill_evt;
	fill_evt.SetEvent( event->GetPEnergy(),
					   event->GetNEnergy(),
					   event->GetPID(),
					   event->GetNID(),
					   event->GetPTime(),
					   event->GetNTime(),
					   event->GetPHit(),
					   event->GetNHit(),
					   event->GetModule(),
					   event->GetRow() );
	
	array_event.push_back( fill_evt );
	
}

void ISSEvts::AddEvt( std::shared_ptr<ISSArrayPEvt> event ) {
	
	// Make a copy of the event and push it back
	ISSArrayPEvt fill_evt;
	fill_evt.SetEvent( event->GetPEnergy(),
					   event->GetNEnergy(),
					   event->GetPID(),
					   event->GetNID(),
					   event->GetPTime(),
					   event->GetNTime(),
					   event->GetPHit(),
					   event->GetNHit(),
					   event->GetModule(),
					   event->GetRow() );

	arrayp_event.push_back( fill_evt );
	
}

void ISSEvts::AddEvt( std::shared_ptr<ISSRecoilEvt> event ) {
	
	// Make a copy of the event and push it back
	ISSRecoilEvt fill_evt;
	fill_evt.SetEvent( event->GetEnergies(),
					   event->GetIDs(),
					   event->GetSector(),
					   event->GetdETime(),
					   event->GetETime() );

	recoil_event.push_back( fill_evt );
	
}

void ISSEvts::AddEvt( std::shared_ptr<ISSMwpcEvt> event ) {
	
	// Make a copy of the event and push it back
	ISSMwpcEvt fill_evt;
	fill_evt.SetEvent( event->GetTacDiff(),
					   event->GetAxis(),
					   event->GetTime() );
	
	mwpc_event.push_back( fill_evt );
	
}

void ISSEvts::AddEvt( std::shared_ptr<ISSElumEvt> event ) {
	
	// Make a copy of the event and push it back
	ISSElumEvt fill_evt;
	fill_evt.SetEvent( event->GetEnergy(),
					   event->GetID(),
					   event->GetSector(),
					   event->GetTime() );
	
	elum_event.push_back( fill_evt );
	
}

void ISSEvts::AddEvt( std::shared_ptr<ISSZeroDegreeEvt> event ) {
	
	// Make a copy of the event and push it back
	ISSZeroDegreeEvt fill_evt;
	fill_evt.SetEvent( event->GetEnergies(),
					   event->GetIDs(),
					   event->GetSector(),
					   event->GetdETime(),
					   event->GetETime() );

	zd_event.push_back( fill_evt );
	
}

void ISSEvts::AddEvt( std::shared_ptr<ISSGammaRayEvt> event ) {
	
	// Make a copy of the event and push it back
	ISSGammaRayEvt fill_evt;
	fill_evt.SetEvent( event->GetEnergy(),
					   event->GetID(),
					   event->GetType(),
					   event->GetTime() );
	
	gamma_event.push_back( fill_evt );
	
}

// ------------ //
// Array events //
// ------------ //
ISSArrayEvt::ISSArrayEvt(){}
ISSArrayEvt::~ISSArrayEvt(){}

void ISSArrayEvt::SetEvent( float mypen, float mynen,
						 unsigned char mypid, unsigned char mynid,
						 double myptime, double myntime,
						 bool myphit, bool mynhit,
						 unsigned char mymod, unsigned char myrow ) {
	
	pen = mypen;
	ptime = myptime;
	pid = mypid;
	phit = myphit;

	nen = mynen;
	ntime = myntime;
	nid = mynid;
	nhit = mynhit;

	mod = mymod;
	row = myrow;

	return;
	
}

void ISSArrayEvt::CopyEvent( std::shared_ptr<ISSArrayEvt> in ){
	
	pen 	= in->GetPEnergy();
	ptime	= in->GetPTime();
	pid		= in->GetPID();
	phit	= in->GetPHit();

	nen		= in->GetNEnergy();
	ntime	= in->GetNTime();
	nid		= in->GetNID();
	nhit	= in->GetNHit();

	mod		= in->GetModule();
	row		= in->GetRow();

	return;
	
}

float ISSArrayEvt::GetX(){
	
	return GetPhiXY().X();

}

float ISSArrayEvt::GetY(){
	
	return GetPhiXY().Y();

}

float ISSArrayEvt::GetPhi(){
	
	return GetPhiXY().Phi();

}

float ISSArrayEvt::GetZ(){
	
	/// Get the z position of the interaction
	/// Note that there is no radial correction yet implemented
	/// the origin is at the target position
	/// z is positive in the beam direction relative to silicon edge
	/// x is positive in the vertical direction towards the sky
	/// y is positive in the horizontal direction towards XT03 (right)
	/// phi is positive in the clockwise direction, looking from the origin to positive z (beam direction)

	float d = 127.5 - (float)pid;		// take centre of the end strip
	d *= 0.953;							// p-side strip pitch = 0.953 mm
	d += 1.508;							// distance from wafer edge to active region
	d += 125.5 * (3.0 - (float)row);	// move to correct row (125.0 mm wafer length + 0.5 mm inter-wafer gap)

	return d; // in mm
	
}

TVector2 ISSArrayEvt::GetPhiXY(){
	
	/// Get the phi angle of the interaction, and in the meantime define x,y too
	/// Note that there is no radial correction yet implemented
	/// the origin is at the target position
	/// z is positive in the beam direction
	/// x is positive in the vertical direction towards the sky
	/// y is positive in the horizontal direction towards XT03 (right)
	/// phi is positive in the clockwise direction, looking from the origin to positive z (beam direction)
	
	// Start with a flat wafer pointing to the sky
	float x = 54.0 / 2.0; 	// diameter is 54.0 mm according to Chris Everett's CAD drawings
	float y = nid%11 - 5.0;	// take strip relative to centre (11 strips per face: 22 strips per module)
	y *= 2.0;				// n-side strip pitch = 2.0 mm

	// This vector can now be rotated to the correct position
	TVector2 vec( x, y );
	
	vec = vec.Rotate( -1.0 * TMath::Pi() / 6. );		// first face is at -30˚
	vec = vec.Rotate( nid%2 * 2. * TMath::Pi() / 6. );	// rotate 60˚ for each face
	vec = vec.Rotate( mod * 2. * TMath::Pi() / 3. );	// rotate 120˚ for each module
	
	return vec;

}


TVector3 ISSArrayEvt::GetPosition(){
	
	TVector3 pos( GetX(), GetY(), GetZ() );
	
	return pos;
	
}

char ISSArrayEvt::FindModule( unsigned short detNo ){
	
	/// Return the module number depending on the detector number from NPTool
	if (detNo > 24) return -1;
    return ((detNo - 1) % 6) / 2;
	
}

char ISSArrayEvt::FindRow( unsigned short detNo ){
	
	/// Return the row number depending on the detector number from NPTool
	if( detNo > 24 ) return -1;
	else return 3 - ( detNo - 1 ) / 6;
	
}

char ISSArrayEvt::FindAsicP( unsigned short detNo ){
	
	/// Return the p-side ASIC number depending on the detector number from NPTool
	if( detNo > 24 ) return -1;

	unsigned char row = FindRow( detNo );
	unsigned char asic = row;
	if( row > 0 ) asic++;
	if( row > 3 ) asic++;

	return asic;
	
}

char ISSArrayEvt::FindAsicN( unsigned short detNo ){
	
	/// Return the n-side ASIC number depending on the detector number from NPTool
	if( detNo > 24 ) return -1;

	unsigned char row = FindRow( detNo );
	unsigned char asic = 1;
	if( row == 2 || row == 3 ) asic = 4;

	return asic;
	
}

char ISSArrayEvt::FindModule( double phi ){
	
	/// Return the module number depending on the phi angle
	// First put us in the 0 -> 2π range
	if( phi < 0.0 ) phi += TMath::TwoPi();
	if( phi > TMath::TwoPi() ) phi -= TMath::TwoPi();

	// module 0 combos
	if( phi > 5. * TMath::Pi() / 3. ) return 0;
	else if( phi <= TMath::Pi() / 3. ) return 0;
	
	// module 1 combos
	else if( phi > TMath::Pi() / 3. && phi <= TMath::Pi() ) return 1;

	// module 2 combos
	else if( phi > TMath::Pi() && phi <= 5. * TMath::Pi() / 3. ) return 2;

	// shouldn't be anything else left
	else return -1;
	
}

char ISSArrayEvt::FindRow( double z ){
	
	/// Return the row number depending on the z position
	
	// Physical silicon length and gaps
	double wafer_length = 125.0;
	double wafer_gap = 0.5;
	
	// Loop over each row
	for( unsigned char i = 0; i < 4; i++ ){
		
		// z0 is defined to edge of first wafer, so we start from 0
		if( z >  (double)(3-i) * wafer_length + (double)(3-i) * wafer_gap &&
		    z <= (double)(4-i) * wafer_length + (double)(3-i) * wafer_gap )
			return i;
		
	}

	// everything else won't hit the silicon
	return -1;
	
}

char ISSArrayEvt::FindPID( double z ){
	
	/// Return the pid number depending on the z position and row number
	
	// First get the row number
	char row = FindRow( z );
	
	// Straight away return 0 if we don't hit the silicon
	if( row < 0 ) return -1;
	
	// Physical silicon length and gaps
	double wafer_length = 125.0;
	double wafer_gap = 0.5;
	double wafer_guard = 1.508;
	double strip_pitch = 0.953;

	// Shift the z to within a single silicon
	z -= (double)(3-row) * ( wafer_length + wafer_gap );

	// Shift the z to edge of the first strip
	z -= wafer_guard;

	// Loop over each strip
	for( unsigned char i = 0; i < 128; i++ ){
		
		// z0 is defined to edge of first wafer, so we start from 0
		if( z >  (double)(127-i) * strip_pitch &&
		    z <= (double)(128-i) * strip_pitch )
			return i;
		
	}
	
	// everything else won't hit the silicon
	return -1;
	
}

char ISSArrayEvt::FindNID( double phi ){
	
	/// Return the nid number depending on the phi position
	
	// First get the module number
	char mod = FindModule( phi );
	
	// Straight away return 0 if we don't hit the silicon
	if( mod < 0 ) return -1;
	
	// Strip geometry
	double strip_pitch = 2.0;
	double det_radius = 54.0 / 2.0;

	// Shift the phi to within a single module
	phi += TMath::TwoPi() / 6.;
	phi -= (double)mod * TMath::TwoPi() / 3.;

	// Then put us in the 0 -> 2π range
	if( phi < 0.0 ) phi += TMath::TwoPi();
	if( phi > TMath::TwoPi() ) phi -= TMath::TwoPi();
	
	// Check if we are on side A or B
	bool sideB = false;
	if( phi > TMath::TwoPi() / 6. ) {
	
		sideB = true;
		phi -= TMath::TwoPi() / 6.;
		
	}
	
	// Loop over each strip
	for( unsigned char i = 0; i < 11; i++ ){
		
		// Vector for the strip edge
		TVector2 vec_low( det_radius, ( (double)i-5.5 ) * strip_pitch );
		TVector2 vec_upp( det_radius, ( (double)i-4.5 ) * strip_pitch );

		// Rotate by pi/6 to get it aligned with reference
		vec_low.Rotate( TMath::TwoPi() / 6. );
		vec_upp.Rotate( TMath::TwoPi() / 6. );

		// check if we are in the strip
		if( phi > vec_low.Phi() && phi <= vec_upp.Phi() )
			return i + 11 * sideB;
		
	}
	
	// everything else won't hit the silicon
	return -1;
	
}


// ------------------- //
// Array p-side events //
// ------------------- //
ISSArrayPEvt::ISSArrayPEvt(){}
ISSArrayPEvt::~ISSArrayPEvt(){}


// ------------- //
// Recoil events //
// ------------- //
ISSRecoilEvt::ISSRecoilEvt(){}
ISSRecoilEvt::~ISSRecoilEvt(){}

void ISSRecoilEvt::SetEvent( std::vector<float> myenergy,
						  std::vector<unsigned char> myid, unsigned char mysec,
						  double mydetime, double myetime ) {
	
	energy = myenergy;
	id = myid;
	sec = mysec;
	detime = mydetime;
	etime = myetime;

	return;
	
}

void ISSRecoilEvt::ClearEvent(){
	
	energy.clear();
	id.clear();
	std::vector<float>().swap(energy);
	std::vector<unsigned char>().swap(id);

}

// ----------- //
// MWPC events //
// ----------- //
ISSMwpcEvt::ISSMwpcEvt(){}
ISSMwpcEvt::~ISSMwpcEvt(){}

void ISSMwpcEvt::SetEvent( int mytacdiff, unsigned char myaxis,
						double mytime ) {
	
	tacdiff = mytacdiff;
	axis = myaxis;
	time = mytime;
	
	return;
	
}


// ----------- //
// Elum events //
// ----------- //
ISSElumEvt::ISSElumEvt(){}
ISSElumEvt::~ISSElumEvt(){}

void ISSElumEvt::SetEvent( float myenergy, unsigned char myid,
						unsigned char mysec, double mytime ) {
	
	energy = myenergy;
	id = myid;
	sec = mysec;
	time = mytime;
	
	return;
	
}


// ----------------- //
// ZeroDegree events //
// ----------------- //
ISSZeroDegreeEvt::ISSZeroDegreeEvt(){}
ISSZeroDegreeEvt::~ISSZeroDegreeEvt(){}

void ISSZeroDegreeEvt::SetEvent( std::vector<float> myenergy,
							  std::vector<unsigned char> myid, unsigned char mysec,
							  double mydetime, double myetime ) {
	
	energy = myenergy;
	id = myid;
	sec = mysec;
	detime = mydetime;
	etime = myetime;

	return;
	
}

void ISSZeroDegreeEvt::ClearEvent(){
	
	energy.clear();
	id.clear();
	std::vector<float>().swap(energy);
	std::vector<unsigned char>().swap(id);

}


// ---------------- //
// Gamma-ray events //
// ---------------- //
ISSGammaRayEvt::ISSGammaRayEvt(){}
ISSGammaRayEvt::~ISSGammaRayEvt(){}

void ISSGammaRayEvt::SetEvent( float myenergy, unsigned char myid,
							  unsigned char mytype, double mytime ) {
	
	energy = myenergy;
	id = myid;
	type = mytype;
	time = mytime;
	
	return;
	
}

