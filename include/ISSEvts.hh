#ifndef __ISSEVTS_HH__
#define __ISSEVTS_HH__

#include <iostream>
#include <vector>
#include <string>
#include <memory>

#include "TVector2.h"
#include "TVector3.h"
#include "TObject.h"

class ISSArrayEvt : public TObject {

public:
	
	// setup functions
	ISSArrayEvt();
	~ISSArrayEvt();

	// Event reconstruction
	void SetEvent( float mypen, float mynen,
				   unsigned char mypid, unsigned char mynid,
				   unsigned long myptime, unsigned long myntime,
				   unsigned char mymod, unsigned char row );
	
	// Copy event
	void CopyEvent( std::shared_ptr<ISSArrayEvt> in );
	
	// Return functions
	inline float 			GetEnergy(){ return pen; };
	inline float 			GetPEnergy(){ return pen; };
	inline float 			GetNEnergy(){ return nen; };
	inline unsigned char 	GetPID(){ return pid; };
	inline unsigned char 	GetNID(){ return nid; };
	inline unsigned long	GetTime(){ return ptime; };
	inline double			GetTimeDouble(){ return (double)ptime; };
	inline unsigned long	GetPTime(){ return ptime; };
	inline unsigned long 	GetNTime(){ return ntime; };
	inline unsigned char	GetModule(){ return mod; };
	inline unsigned char	GetRow(){ return row; };

	float		GetX();
	float		GetY();
	float		GetZ();
	float		GetPhi();
	TVector2	GetPhiXY();
	TVector3	GetPosition();

private:

	// variables for particle event
	unsigned char	mod;	///< module number
	unsigned char	row;	///< row number of the silicon
	float			pen;	///< p-side energy in keV
	float 			nen;	///< n-side energy in keV
	unsigned char	pid;	///< p-side strip id, from 0 - 511, i.e along all 4 wafers
	unsigned char	nid;	///< n-side strip id, from 0 - 65, i.e. around all 6 sides
	unsigned long	ptime;	///< p-side timestamp
	unsigned long	ntime;	///< n-side timestamp


	ClassDef( ISSArrayEvt, 2 )

};

class ISSArrayPEvt : public ISSArrayEvt {

public:
	
	// setup functions
	ISSArrayPEvt();
	~ISSArrayPEvt();

private:


	ClassDef( ISSArrayPEvt, 1 )

};

class ISSRecoilEvt : public TObject {

public:
		
	// setup functions
	ISSRecoilEvt();
	~ISSRecoilEvt();

	void SetEvent( std::vector<float> myenergy,
				   std::vector<unsigned char> myid, unsigned char mysec,
				   unsigned long mydetime, unsigned long myetime );
	
	void ClearEvent();
	
	inline void AddRecoil( float myenergy, unsigned char myid ){
		energy.push_back( myenergy );
		id.push_back( myid );
	};
	
	inline void SetSector( unsigned char s ){ sec = s; };
	inline void SetdETime( unsigned long t ){ detime = t; };
	inline void SetETime( unsigned long t ){ etime = t; };

	
	inline unsigned char	GetDepth(){ return energy.size(); };
	inline unsigned char	GetSector(){ return sec; };
	inline unsigned long	GetTime(){ return detime; };
	inline double			GetTimeDouble(){ return (double)detime; };
	inline unsigned long	GetdETime(){ return detime; };
	inline unsigned long	GetETime(){ return etime; };

	inline std::vector<float>			GetEnergies(){ return energy; };
	inline std::vector<unsigned char>	GetIDs(){ return id; };

	
	inline float GetEnergy( unsigned char i ){
		if( i < energy.size() ) return energy.at(i);
		else return 0;
	};
	
	inline float GetEnergyLoss( unsigned char i = 0 ){
		float total = 0;
		for( unsigned int j = 0; j < energy.size(); ++j )
			if( GetID(j) <= i ) total += energy.at(j);
		return total;
	};

	inline float GetEnergyRest( unsigned char i = 1 ){
		float total = 0;
		for( unsigned int j = i; j < energy.size(); ++j )
			if( GetID(j) >= i ) total += energy.at(j);
		return total;
	};
	
	inline float GetEnergyTotal(){
		float total = 0;
		for( unsigned int i = 0; i < energy.size(); ++i )
			total += energy.at(i);
		return total;
	};
	
	inline unsigned char GetID( unsigned char i ){
		if( i < id.size() ) return id.at(i);
		else return -1;
	};


	
private:

	// variables for recoil events
	std::vector<float>			energy;	///< differential energy list, i.e. Silicon dE-E length = 2
	std::vector<unsigned char>	id;		///< differential id list, i.e. dE = 0, E = 1, for example
	unsigned char				sec;	///< sector of the recoil detector, i.e 0-3 for QQQ1 quadrants
	unsigned long				detime;	///< time stamp of dE event
	unsigned long				etime;	///< time stamp of E event

	ClassDef( ISSRecoilEvt, 2 )

};

class ISSMwpcEvt : public TObject {

public:
		
	// setup functions
	ISSMwpcEvt();
	~ISSMwpcEvt();
	
	void SetEvent( int mytacdiff, unsigned char myaxis,
				   unsigned long mytime );

	inline void SetTacDiff( int t ){ tacdiff = t; };
	inline void SetAxis( unsigned char a ){ axis = a; };
	inline void SetTime( unsigned long t ){ time = t; };

	inline int				GetTacDiff(){ return tacdiff; };
	inline unsigned char	GetAxis(){ return axis; };
	inline unsigned long	GetTime(){ return time; };
	inline double			GetTimeDouble(){ return (double)time; };

	
private:

	int				tacdiff;	///< TAC differences
	unsigned char	axis;		///< axis ID, usually just x=0 and y=1
	unsigned long	time;		///< time stamp of the MWPC event
	
	ClassDef( ISSMwpcEvt, 1 );

};

class ISSElumEvt : public TObject {

public:
		
	// setup functions
	ISSElumEvt();
	~ISSElumEvt();
	
	void SetEvent( float myenergy, unsigned char myid,
				   unsigned char mysec, unsigned long mytime );

	inline void SetEnergy( float e ){ energy = e; };
	inline void SetSector( unsigned char s ){ sec = s; };
	inline void SetID( unsigned char i ){ id = i; };
	inline void SetTime( unsigned long t ){ time = t; };

	inline float			GetEnergy(){ return energy; };
	inline unsigned char	GetID(){ return id; };
	inline unsigned char	GetSector(){ return sec; };
	inline unsigned long	GetTime(){ return time; };
	inline double			GetTimeDouble(){ return (double)time; };

	
private:

	float			energy;	///< Energy in the ELUM detector
	unsigned char	id;		///< ID list, well, we only have one ELUM detector so it is always == 0
	unsigned char	sec;	///< sector or quandrant of the ELUM detector, i.e. 0-3 when split into 4
	unsigned long	time;	///< time stamp of the ELUM event
	
	ClassDef( ISSElumEvt, 2 );

};

class ISSZeroDegreeEvt : public TObject {

public:
		
	// setup functions
	ISSZeroDegreeEvt();
	~ISSZeroDegreeEvt();

	void SetEvent( std::vector<float> myenergy,
				   std::vector<unsigned char> myid, unsigned char mysec,
				   unsigned long mydetime, unsigned long myetime );
	
	void ClearEvent();

	inline void AddZeroDegree( float myenergy, unsigned char myid ){
		energy.push_back( myenergy );
		id.push_back( myid );
	};

	inline void SetSector( unsigned char s ){ sec = s; };
	inline void SetdETime( unsigned long t ){ detime = t; };
	inline void SetETime( unsigned long t ){ etime = t; };

	inline unsigned char	GetDepth(){ return energy.size(); };
	inline unsigned char	GetSector(){ return sec; };
	inline unsigned long	GetTime(){ return detime; };
	inline double			GetTimeDouble(){ return (double)detime; };
	inline unsigned long	GetdETime(){ return detime; };
	inline unsigned long	GetETime(){ return etime; };

	inline std::vector<float>			GetEnergies(){ return energy; };
	inline std::vector<unsigned char>	GetIDs(){ return id; };

	inline float GetEnergy( unsigned char i ){
		if( i < energy.size() ) return energy.at(i);
		else return 0;
	};
	
	inline float GetEnergyLoss( unsigned char i = 0 ){
		float total = 0;
		for( unsigned int j = 0; j < energy.size(); ++j )
			if( GetID(j) <= i ) total += energy.at(j);
		return total;
	};

	inline float GetEnergyRest( unsigned char i = 1 ){
		float total = 0;
		for( unsigned int j = i; j < energy.size(); ++j )
			if( GetID(j) >= i ) total += energy.at(j);
		return total;
	};
	
	inline float GetEnergyTotal(){
		float total = 0;
		for( unsigned int i = 0; i < energy.size(); ++i )
			total += energy.at(i);
		return total;
	};

	inline int GetID( unsigned char i ){
		if( i < id.size() ) return id.at(i);
		else return -1;
	};

private:

	std::vector<float>			energy;	///< differential energy list, i.e. Silicon dE-E length = 2
	std::vector<unsigned char>	id;		///< differential id list, i.e. dE = 0, E = 1, for example
	unsigned char				sec;	///< sector or quandrant of the ZeroDegree detector, i.e. 0 because we just have one
	unsigned long				detime;	///< time stamp of ZeroDegree event
	unsigned long				etime;	///< time stamp of ZeroDegree event

	ClassDef( ISSZeroDegreeEvt, 2 )

};



class ISSEvts : public TObject {
//class ISSEvts {

public:
	
	// setup functions
	ISSEvts();
	~ISSEvts();
	
	void AddEvt( std::shared_ptr<ISSArrayEvt> event );
	void AddEvt( std::shared_ptr<ISSArrayPEvt> event );
	void AddEvt( std::shared_ptr<ISSRecoilEvt> event );
	void AddEvt( std::shared_ptr<ISSMwpcEvt> event );
	void AddEvt( std::shared_ptr<ISSElumEvt> event );
	void AddEvt( std::shared_ptr<ISSZeroDegreeEvt> event );
	
	inline unsigned int GetArrayMultiplicity(){ return array_event.size(); };
	inline unsigned int GetArrayPMultiplicity(){ return arrayp_event.size(); };
	inline unsigned int GetRecoilMultiplicity(){ return recoil_event.size(); };
	inline unsigned int GetMwpcMultiplicity(){ return mwpc_event.size(); };
	inline unsigned int GetElumMultiplicity(){ return elum_event.size(); };
	inline unsigned int GetZeroDegreeMultiplicity(){ return zd_event.size(); };
	
	inline std::shared_ptr<ISSArrayEvt> GetArrayEvt( unsigned int i ){
		if( i < array_event.size() ) return std::make_shared<ISSArrayEvt>( array_event.at(i) );
		else return nullptr;
	};
	inline std::shared_ptr<ISSArrayPEvt> GetArrayPEvt( unsigned int i ){
		if( i < arrayp_event.size() ) return std::make_shared<ISSArrayPEvt>( arrayp_event.at(i) );
		else return nullptr;
	};
	inline std::shared_ptr<ISSRecoilEvt> GetRecoilEvt( unsigned int i ){
		if( i < recoil_event.size() ) return std::make_shared<ISSRecoilEvt>( recoil_event.at(i) );
		else return nullptr;
	};
	inline std::shared_ptr<ISSMwpcEvt> GetMwpcEvt( unsigned int i ){
		if( i < mwpc_event.size() ) return std::make_shared<ISSMwpcEvt>( mwpc_event.at(i) );
		else return nullptr;
	};
	inline std::shared_ptr<ISSElumEvt> GetElumEvt( unsigned int i ){
		if( i < elum_event.size() ) return std::make_shared<ISSElumEvt>( elum_event.at(i) );
		else return nullptr;
	};
	inline std::shared_ptr<ISSZeroDegreeEvt> GetZeroDegreeEvt( unsigned int i ){
		if( i < zd_event.size() ) return std::make_shared<ISSZeroDegreeEvt>( zd_event.at(i) );
		else return nullptr;
	};

	void ClearEvt();
	
	// ISOLDE timestamping
	inline void SetEBIS( unsigned long t ){ ebis = t; return; };
	inline void SetT1( unsigned long t ){ t1 = t; return; };
	
	inline unsigned long GetEBIS(){ return ebis; };
	inline unsigned long GetT1(){ return t1; };

	
private:
	
	// variables for timestamping
	unsigned long ebis;		///< absolute EBIS pulse time
	unsigned long t1;		///< absolute proton pulse time

	std::vector<ISSArrayEvt> array_event;
	std::vector<ISSArrayPEvt> arrayp_event;
	std::vector<ISSRecoilEvt> recoil_event;
	std::vector<ISSMwpcEvt> mwpc_event;
	std::vector<ISSElumEvt> elum_event;
	std::vector<ISSZeroDegreeEvt> zd_event;

	ClassDef( ISSEvts, 3 )
	
};

#endif

