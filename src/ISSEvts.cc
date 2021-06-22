#include "ISSEvts.hh"

ClassImp(ArrayEvt)
ClassImp(RecoilEvt)
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
	recoil_event.clear();
	elum_event.clear();
	zd_event.clear();
	
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
					   event->GetModule() );
	
	array_event.push_back( fill_evt );
	
}

void ISSEvts::AddEvt( RecoilEvt *event ) {
	
	// Make a copy of the event and push it back
	RecoilEvt fill_evt;
	fill_evt.SetEvent( event->GetEnergies(),
					   event->GetIDs(),
					   event->GetSector(),
					   event->GetTime() );
	
	recoil_event.push_back( fill_evt );
	
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
					   event->GetTime() );
	
	zd_event.push_back( fill_evt );
	
}

// ------------ //
// Array events //
// ------------ //
ArrayEvt::ArrayEvt(){}
ArrayEvt::~ArrayEvt(){}

void ArrayEvt::SetEvent( float mypen, float mynen,
						 int mypid, int mynid,
						 long myptd, long myntd,
						 int mymod ) {
	
	pen = mypen;
	ptd = myptd;
	pid = mypid;
	
	nen = mynen;
	ntd = myntd;
	nid = mynid;
	
	mod = mymod;

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
	/// z is positive in the beam direction
	/// x is positive in the vertical direction towards the sky
	/// y is positive in the horizontal direction towards XT03 (right)
	/// phi is positive in the clockwise direction, looking from the origin to positive z (beam direction)

	//float z = Cal->GetTargetDist(); // not yet implemented
	float z = -10.0; 						// just until it is in the cal file
	float d = common::n_pstrip - pid;		// take centre of the end strip
	d *= 0.95;								// p-side strip pitch = 0.95 mm
	d += (3 - pid/common::n_pstrip) * 3.9;	// inter wafer distance (to be confirmed)
	d += 1.7;								// distance from wafer edge to active region

	if( z > 0 ) z += d;	// forward direction (downstream)
	else z -= d;		// backward direction (upstream)
		
	return z; // in mm
	
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
	float y = nid - 4.5;	// take strip relative to centre (11 strips)
	y *= 2.0;				// n-side strip pitch = 2.0 mm

	// This vector can now be rotated to the correct position
	TVector2 vec( x, y );
	
	vec = vec.Rotate( TMath::Pi() / 6. );		// start in the centre of first face
	vec = vec.Rotate( mod * 2. * TMath::Pi() / 3. );	// rotate for each module
	
	return vec;

}


TVector3 ArrayEvt::GetPosition(){
	
	TVector3 pos;
	
	return pos;
	
}


// ------------- //
// Recoil events //
// ------------- //
RecoilEvt::RecoilEvt(){}
RecoilEvt::~RecoilEvt(){}

void RecoilEvt::SetEvent( std::vector<float> myenergy,
						 std::vector<int> myid,
						 int mysec, long mytd ) {
	
	energy = myenergy;
	id = myid;
	sec = mysec;
	td = mytd;
	
	return;
	
}

// ----------- //
// Elum events //
// ----------- //
ElumEvt::ElumEvt(){}
ElumEvt::~ElumEvt(){}

void ElumEvt::SetEvent( float myenergy, int myid,
						 int mysec, long mytd ) {
	
	energy = myenergy;
	id = myid;
	sec = mysec;
	td = mytd;
	
	return;
	
}


// ----------------- //
// ZeroDegree events //
// ----------------- //
ZeroDegreeEvt::ZeroDegreeEvt(){}
ZeroDegreeEvt::~ZeroDegreeEvt(){}

void ZeroDegreeEvt::SetEvent( std::vector<float> myenergy,
						 std::vector<int> myid,
						 int mysec, long mytd ) {
	
	energy = myenergy;
	id = myid;
	sec = mysec;
	td = mytd;
	
	return;
	
}
