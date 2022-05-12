#include "ISSEvts.hh"

ClassImp(ISSArrayEvt)
ClassImp(ISSArrayPEvt)
ClassImp(ISSRecoilEvt)
ClassImp(ISSMwpcEvt)
ClassImp(ISSElumEvt)
ClassImp(ISSZeroDegreeEvt)
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
	
	std::vector<ISSArrayEvt>().swap(array_event);
	std::vector<ISSArrayPEvt>().swap(arrayp_event);
	std::vector<ISSRecoilEvt>().swap(recoil_event);
	std::vector<ISSMwpcEvt>().swap(mwpc_event);
	std::vector<ISSElumEvt>().swap(elum_event);
	std::vector<ISSZeroDegreeEvt>().swap(zd_event);

	
	ebis = -999;
	t1 = -999;

	return;

}

void ISSEvts::AddEvt( ISSArrayEvt *event ) {
	
	// Make a copy of the event and push it back
	ISSArrayEvt fill_evt;
	fill_evt.SetEvent( event->GetPEnergy(),
					   event->GetNEnergy(),
					   event->GetPID(),
					   event->GetNID(),
					   event->GetPTime(),
					   event->GetNTime(),
					   event->GetModule(),
					   event->GetRow() );
	
	array_event.push_back( fill_evt );
	
}

void ISSEvts::AddEvt( ISSArrayPEvt *event ) {
	
	// Make a copy of the event and push it back
	ISSArrayPEvt fill_evt;
	fill_evt.SetEvent( event->GetPEnergy(),
					   event->GetNEnergy(),
					   event->GetPID(),
					   event->GetNID(),
					   event->GetPTime(),
					   event->GetNTime(),
					   event->GetModule(),
					   event->GetRow() );

	arrayp_event.push_back( fill_evt );
	
}

void ISSEvts::AddEvt( ISSRecoilEvt *event ) {
	
	// Make a copy of the event and push it back
	ISSRecoilEvt fill_evt;
	fill_evt.SetEvent( event->GetEnergies(),
					   event->GetIDs(),
					   event->GetSector(),
					   event->GetdETime(),
					   event->GetETime() );

	recoil_event.push_back( fill_evt );
	
}

void ISSEvts::AddEvt( ISSMwpcEvt *event ) {
	
	// Make a copy of the event and push it back
	ISSMwpcEvt fill_evt;
	fill_evt.SetEvent( event->GetTacDiff(),
					   event->GetAxis(),
					   event->GetTime() );
	
	mwpc_event.push_back( fill_evt );
	
}

void ISSEvts::AddEvt( ISSElumEvt *event ) {
	
	// Make a copy of the event and push it back
	ISSElumEvt fill_evt;
	fill_evt.SetEvent( event->GetEnergy(),
					   event->GetID(),
					   event->GetSector(),
					   event->GetTime() );
	
	elum_event.push_back( fill_evt );
	
}

void ISSEvts::AddEvt( ISSZeroDegreeEvt *event ) {
	
	// Make a copy of the event and push it back
	ISSZeroDegreeEvt fill_evt;
	fill_evt.SetEvent( event->GetEnergies(),
					   event->GetIDs(),
					   event->GetSector(),
					   event->GetdETime(),
					   event->GetETime() );

	zd_event.push_back( fill_evt );
	
}

// ------------ //
// Array events //
// ------------ //
ISSArrayEvt::ISSArrayEvt(){}
ISSArrayEvt::~ISSArrayEvt(){}

void ISSArrayEvt::SetEvent( float mypen, float mynen,
						 unsigned char mypid, unsigned char mynid,
						 unsigned long myptime, unsigned long myntime,
						 unsigned char mymod, unsigned char myrow ) {
	
	pen = mypen;
	ptime = myptime;
	pid = mypid;
	
	nen = mynen;
	ntime = myntime;
	nid = mynid;
	
	mod = mymod;
	row = myrow;

	return;
	
}

void ISSArrayEvt::CopyEvent( ISSArrayEvt *in ){
	
	pen 	= in->GetPEnergy();
	ptime	= in->GetPTime();
	pid		= in->GetPID();
	
	nen		= in->GetNEnergy();
	ntime	= in->GetNTime();
	nid		= in->GetNID();
	
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
	float x = 53.5 / 2.0; 	// diameter is 53.5 mm
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
						  unsigned long mydetime, unsigned long myetime ) {
	
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
						unsigned long mytime ) {
	
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
						unsigned char mysec, unsigned long mytime ) {
	
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
							  unsigned long mydetime, unsigned long myetime ) {
	
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
