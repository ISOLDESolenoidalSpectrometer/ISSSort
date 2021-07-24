#ifndef __ISSEVTS_HH__
#define __ISSEVTS_HH__

#include <iostream>
#include <vector>
#include <string>

#include "TVector2.h"
#include "TVector3.h"
#include "TObject.h"

class ArrayEvt : public TObject {

public:
	
	// setup functions
	ArrayEvt();
	~ArrayEvt();

	// Event reconstruction
	void SetEvent( float mypen, float mynen,
				   int mypid, int mynid,
				   long myptd, long myntd,
				   int mymod );
	
	inline float 	GetEnergy(){ return pen; };
	inline float 	GetPEnergy(){ return pen; };
	inline float 	GetNEnergy(){ return nen; };
	inline int	 	GetPID(){ return pid; };
	inline int	 	GetNID(){ return nid; };
	inline long	 	GetPTime(){ return ptd; };
	inline long	 	GetNTime(){ return ntd; };
	inline int		GetModule(){ return mod; };

	float		GetX();
	float		GetY();
	float		GetZ();
	float		GetPhi();
	TVector2	GetPhiXY();
	TVector3	GetPosition();

private:

	// variables for particle event
	float pen;		///< p-side energy in keV
	float nen;		///< n-side energy in keV
	int	  pid;		///< p-side strip id, from 0 - 511, i.e along all 4 wafers
	int   nid;		///< n-side strip id, from 0 - 65, i.e. around all 6 sides
	long  ptd;		///< p-side time difference to first trigger in event
	long  ntd;		///< n-side time difference to first trigger in event
	int	  mod;		///< module number

	ClassDef( ArrayEvt, 1 )

};

class RecoilEvt : public TObject {

public:
		
	// setup functions
	RecoilEvt();
	~RecoilEvt();

	void SetEvent( std::vector<float> myenergy,
					std::vector<int> myid,
					int mysec, long mytd );
	
	void ClearEvent();
	
	inline void AddRecoil( float myenergy, int myid ){
		energy.push_back( myenergy );
		id.push_back( myid );
	};
	
	inline void SetSector( int s ){ sec = s; };
	inline void SetTime( long t ){ td = t; };

	
	inline unsigned int		GetDepth(){ return energy.size(); };
	inline unsigned int		GetSector(){ return sec; };
	inline unsigned long	GetTime(){ return td; };
	
	inline std::vector<float>	GetEnergies(){ return energy; };
	inline std::vector<int>		GetIDs(){ return id; };

	inline float GetEnergy( unsigned int i ){
		if( i < energy.size() ) return energy.size();
		else return 0;
	};
	
	inline int GetID( unsigned int i ){
		if( i < id.size() ) return id.size();
		else return -1;
	};

	inline float GetTotalEnergy(){
		float total = 0;
		for( unsigned int i = 0; i < energy.size(); ++i )
			total += energy.at(i);
		return total;
	};

	
private:

	// variables for recoil events
	std::vector<float>		energy;	///< differential energy list, i.e. Silicon dE-E length = 2
	std::vector<int>		id;		///< differential id list, i.e. dE = 0, E = 1, for example
	int						sec;	///< sector of the recoil detector, i.e 0-3 for QQQ1 quadrants
	long					td;		///< time difference of first recoil energy to first trigger in event

	ClassDef( RecoilEvt, 1 )

};

class ElumEvt : public TObject {

public:
		
	// setup functions
	ElumEvt();
	~ElumEvt();
	
	void SetEvent( float myenergy, int myid, int mysec, long mytd );

	inline void SetEnergy( float e ){ energy = e; };
	inline void SetSector( int s ){ sec = s; };
	inline void SetID( int i ){ id = i; };
	inline void SetTime( long t ){ td = t; };

	inline float			GetEnergy(){ return energy; };
	inline unsigned int		GetID(){ return id; };
	inline unsigned int		GetSector(){ return sec; };
	inline unsigned long	GetTime(){ return td; };

	
private:

	float	energy;	///< Energy in the ELUM detector
	int		id;		///< ID list, well, we only have one ELUM detector so it is always == 0
	int		sec;	///< sector or quandrant of the ELUM detector, i.e. 0-3 when split into 4
	long	td;		///< time difference between ELUM event and first trigger in event
	
	ClassDef( ElumEvt, 1 );

};

class ZeroDegreeEvt : public TObject {

public:
		
	// setup functions
	ZeroDegreeEvt();
	~ZeroDegreeEvt();

	void SetEvent( std::vector<float> myenergy,
					std::vector<int> myid,
					int mysec, long mytd );
	
	void ClearEvent();

	inline void AddZeroDegree( float myenergy, int myid ){
		energy.push_back( myenergy );
		id.push_back( myid );
	};

	inline void SetSector( int s ){ sec = s; };
	inline void SetTime( long t ){ td = t; };

	inline unsigned int		GetDepth(){ return energy.size(); };
	inline unsigned int		GetSector(){ return sec; };
	inline unsigned long	GetTime(){ return td; };

	inline std::vector<float>	GetEnergies(){ return energy; };
	inline std::vector<int>		GetIDs(){ return id; };

	inline float GetEnergy( unsigned int i ){
		if( i < energy.size() ) return energy.size();
		else return 0;
	};
	
	inline int GetID( unsigned int i ){
		if( i < id.size() ) return id.size();
		else return -1;
	};

	inline float GetTotalEnergy(){
		float total = 0;
		for( unsigned int i = 0; i < energy.size(); ++i )
			total += energy.at(i);
		return total;
	};

private:

	std::vector<float>		energy;	///< differential energy list, i.e. Silicon dE-E length = 2
	std::vector<int>		id;		///< differential id list, i.e. dE = 0, E = 1, for example
	int						sec;	///< sector or quandrant of the ZeroDegree detector, i.e. 0 because we just have one
	long					td;		///< time difference between ZeroDegree event and first trigger in event

	ClassDef( ZeroDegreeEvt, 1 )

};



class ISSEvts : public TObject {
//class ISSEvts {

public:
	
	// setup functions
	ISSEvts();
	~ISSEvts();
	
	void AddEvt( ArrayEvt *event );
	void AddEvt( RecoilEvt *event );
	void AddEvt( ElumEvt *event );
	void AddEvt( ZeroDegreeEvt *event );
	
	inline unsigned int GetArrayMultiplicity(){ return array_event.size(); };
	inline unsigned int GetRecoilMultiplicity(){ return recoil_event.size(); };
	inline unsigned int GetElumMultiplicity(){ return elum_event.size(); };
	inline unsigned int GetZeroDegreeMultiplicity(){ return zd_event.size(); };
	
	inline ArrayEvt* GetArrayEvt( unsigned int i ){
		if( i < array_event.size() ) return &array_event.at(i);
		else return nullptr;
	};
	inline RecoilEvt* GetRecoilEvt( unsigned int i ){
		if( i < recoil_event.size() ) return &recoil_event.at(i);
		else return nullptr;
	};
	inline ElumEvt* GetElumEvt( unsigned int i ){
		if( i < elum_event.size() ) return &elum_event.at(i);
		else return nullptr;
	};
	inline ZeroDegreeEvt* GetZeroDegreeEvt( unsigned int i ){
		if( i < zd_event.size() ) return &zd_event.at(i);
		else return nullptr;
	};

	void ClearEvt();
	
	// ISOLDE timestamping
	inline void SetEBIS( long t ){ ebis = t; return; };
	inline void SetT1( long t ){ t1 = t; return; };
	
	inline long GetEBIS(){ return ebis; };
	inline long GetT1(){ return t1; };

	
private:
	
	// variables for timestamping
	long ebis;		///< time difference between first trigger in event and previous EBIS pulse
	long t1;		///< time difference between first trigger in event and previous proton pulse

	std::vector<ArrayEvt> array_event;
	std::vector<RecoilEvt> recoil_event;
	std::vector<ElumEvt> elum_event;
	std::vector<ZeroDegreeEvt> zd_event;

	ClassDef( ISSEvts, 1 )
	
};

#endif

