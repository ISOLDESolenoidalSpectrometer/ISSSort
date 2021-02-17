#ifndef __ISSEVTS_HH__
#define __ISSEVTS_HH__

#include <iostream>
#include <vector>
#include <string>

#include "TVector2.h"
#include "TVector3.h"
#include "TObject.h"

class ISSEvts : public TObject {
//class ISSEvts {

public:
	
	// setup functions
	ISSEvts();
	~ISSEvts();
	
	void Initialise();
	
	// Event reconstruction
	void SetParticle( float mypen, float mynen, float myz,
					  TVector2 myphixy, long myptd, long myntd );
	void AddRecoil( std::vector<float> mydiff,
				    std::vector<int> myid,
				    int mysec, long mytd );
	
	// ISOLDE timestamping
	inline void SetEBIS( long t ){ ebis = t; return; };
	inline void SetT1( long t ){ t1 = t; return; };
	
	// Get multiplicities
	inline int MultRecoil(){ return ren_total.size(); };

	
private:
	
	// variables for particle event
	float pen;		///< p-side energy in keV
	float nen;		///< n-side energy in keV
	TVector3 pos;	///< interaction position as a TVector3 object. x is vertical, positive to the sky. y is horizontal, positive to XT03 (right). z is along beam axis, positive in beam direction. Phi is positive going clockwise from vertical
	long  ptd;		///< p-side time difference to first trigger in event
	long  ntd;		///< n-side time difference to first trigger in event

	// variables for recoil events
	std::vector<float>					ren_total;	///< total energy of all recoil elements, i.e dE+E
	//std::vector<std::vector<float>>		ren_diff;	///< differential energy list, i.e. Silicon dE-E length = 2
	//std::vector<std::vector<int>>		rid;		///< differential id list, i.e. dE = 0, E = 1, for example
	std::vector<int>					rsec;		///< sector of the recoil detector, i.e 0-3 for QQQ1 quadrants
	std::vector<long>					rtd;		///< time difference of first recoil energy to first trigger in event

	
	// variables for timestamping
	long ebis;		///< time difference between first trigger in event and EBIS pulse
	long t1;		///< time difference between first trigger in event and proton pulse

	ClassDef( ISSEvts, 1 );
	
};

#endif

