#include "ISSEvts.hh"

ISSEvts::ISSEvts() {
	
	Initialise();
	
}

ISSEvts::~ISSEvts() {
	
	//std::cout << "destructor" << endl;
	
}

void ISSEvts::Initialise() {
	
	pen	= -9999;
	nen	= -9999;
	ptd	= -9999;
	ntd	= -9999;

	ren_total.resize(0);
	//ren_diff.resize(0);
	//rid.resize(0);
	rsec.resize(0);
	rtd.resize(0);
	
	ebis	= -999;
	t1		= -999;

	return;

}

void ISSEvts::SetParticle( float mypen, float mynen, float myz,
						   TVector2 myphixy, long myptd, long myntd ) {
	
	pen = mypen;
	nen = mynen;
	ptd = myptd;
	ntd = myntd;

	pos = TVector3( myphixy.X(), myphixy.Y(), myz );

	return;
	
}

void ISSEvts::AddRecoil( std::vector<float> mydiff,
						 std::vector<int> myid,
						 int mysec, long mytd ) {
	
	//ren_diff.push_back( mydiff );
	//rid.push_back( myid );
	rsec.push_back( mysec );
	rtd.push_back( mytd );
	
	float total = 0;
	for( unsigned int i = 0; i < mydiff.size(); ++i )
		total += mydiff.at(i);
	ren_total.push_back( total );
	
	
	return;
	
}

