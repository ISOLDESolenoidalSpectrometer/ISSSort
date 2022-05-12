#include "ISSEvts.hh"

ClassImp(ArrayEvt)
ClassImp(ArrayPEvt)
ClassImp(RecoilEvt)
ClassImp(MwpcEvt)
ClassImp(ElumEvt)
ClassImp(ZeroDegreeEvt)
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
	
	std::vector<ArrayEvt>().swap(array_event);
	std::vector<ArrayPEvt>().swap(arrayp_event);
	std::vector<RecoilEvt>().swap(recoil_event);
	std::vector<MwpcEvt>().swap(mwpc_event);
	std::vector<ElumEvt>().swap(elum_event);
	std::vector<ZeroDegreeEvt>().swap(zd_event);

	
	ebis = -999;
	t1 = -999;

	return;

}

void ISSEvts::AddEvt( ArrayEvt *event ) {
	
	// Make a copy of the event and push it back
	ArrayEvt fill_evt;
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

void ISSEvts::AddEvt( ArrayPEvt *event ) {
	
	// Make a copy of the event and push it back
	ArrayPEvt fill_evt;
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

void ISSEvts::AddEvt( RecoilEvt *event ) {
	
	// Make a copy of the event and push it back
	RecoilEvt fill_evt;
	fill_evt.SetEvent( event->GetEnergies(),
					   event->GetIDs(),
					   event->GetSector(),
					   event->GetdETime(),
					   event->GetETime() );

	recoil_event.push_back( fill_evt );
	
}

void ISSEvts::AddEvt( MwpcEvt *event ) {
	
	// Make a copy of the event and push it back
	MwpcEvt fill_evt;
	fill_evt.SetEvent( event->GetTacDiff(),
					   event->GetAxis(),
					   event->GetTime() );
	
	mwpc_event.push_back( fill_evt );
	
}

void ISSEvts::AddEvt( ElumEvt *event ) {
	
	// Make a copy of the event and push it back
	ElumEvt fill_evt;
	fill_evt.SetEvent( event->GetEnergy(),
					   event->GetID(),
					   event->GetSector(),
					   event->GetTime() );
	
	elum_event.push_back( fill_evt );
	
}

void ISSEvts::AddEvt( ZeroDegreeEvt *event ) {
	
	// Make a copy of the event and push it back
	ZeroDegreeEvt fill_evt;
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
ArrayEvt::ArrayEvt(){}
ArrayEvt::~ArrayEvt(){}

void ArrayEvt::SetEvent( float mypen, float mynen,
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

void ArrayEvt::CopyEvent( ArrayEvt *in ){
	
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

float ArrayEvt::GetX(){
	
	return GetPhiXY().X();

}

float ArrayEvt::GetY(){
	
	return GetPhiXY().Y();

}

float ArrayEvt::GetPhi(){
	
	return GetPhiXY().Phi();

}

float ArrayEvt::GetZ(){
	
	/// Get the z position of the interaction
	/// Note that there is no radial correction yet implemented
	/// the origin is at the target position
	/// z is positive in the beam direction relative to silicon edge
	/// x is positive in the vertical direction towards the sky
	/// y is positive in the horizontal direction towards XT03 (right)
	/// phi is positive in the clockwise direction, looking from the origin to positive z (beam direction)

    float d = 127.5 - (float)pid;       // take centre of the end strip
    d *= 0.953;                         // p-side strip pitch = 0.953 mm
    d += 1.508;                         // distance from wafer edge to active region
    d += 125.5 * (3.0 - (float)row);    // move to correct row (125.0 mm wafer length + 0.5 mm inter-wafer gap)

	return d; // in mm
	
}

TVector2 ArrayEvt::GetPhiXY(){
	
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


TVector3 ArrayEvt::GetPosition(){
	
	TVector3 pos( GetX(), GetY(), GetZ() );
	
	return pos;
	
}

// ------------------- //
// Array p-side events //
// ------------------- //
ArrayPEvt::ArrayPEvt(){}
ArrayPEvt::~ArrayPEvt(){}


// ------------- //
// Recoil events //
// ------------- //
RecoilEvt::RecoilEvt(){}
RecoilEvt::~RecoilEvt(){}

void RecoilEvt::SetEvent( std::vector<float> myenergy,
						  std::vector<unsigned char> myid, unsigned char mysec,
						  unsigned long mydetime, unsigned long myetime ) {
	
	energy = myenergy;
	id = myid;
	sec = mysec;
	detime = mydetime;
	etime = myetime;

	return;
	
}

void RecoilEvt::ClearEvent(){
	
	energy.clear();
	id.clear();
	std::vector<float>().swap(energy);
	std::vector<unsigned char>().swap(id);

}

// ----------- //
// MWPC events //
// ----------- //
MwpcEvt::MwpcEvt(){}
MwpcEvt::~MwpcEvt(){}

void MwpcEvt::SetEvent( int mytacdiff, unsigned char myaxis,
						unsigned long mytime ) {
	
	tacdiff = mytacdiff;
	axis = myaxis;
	time = mytime;
	
	return;
	
}


// ----------- //
// Elum events //
// ----------- //
ElumEvt::ElumEvt(){}
ElumEvt::~ElumEvt(){}

void ElumEvt::SetEvent( float myenergy, unsigned char myid,
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
ZeroDegreeEvt::ZeroDegreeEvt(){}
ZeroDegreeEvt::~ZeroDegreeEvt(){}

void ZeroDegreeEvt::SetEvent( std::vector<float> myenergy,
							  std::vector<unsigned char> myid, unsigned char mysec,
							  unsigned long mydetime, unsigned long myetime ) {
	
	energy = myenergy;
	id = myid;
	sec = mysec;
	detime = mydetime;
	etime = myetime;

	return;
	
}

void ZeroDegreeEvt::ClearEvent(){
	
	energy.clear();
	id.clear();
	std::vector<float>().swap(energy);
	std::vector<unsigned char>().swap(id);

}
