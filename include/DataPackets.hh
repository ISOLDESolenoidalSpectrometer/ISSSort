#ifndef __DATAPACKETS_HH
#define __DATAPACKETS_HH

#include <memory>
#include <iostream>

#include "TObject.h"

class ISSAsicData : public TObject {
	
public:

	ISSAsicData();
	~ISSAsicData();
	
	inline double					GetTime() { return (double)timestamp; };
	inline unsigned long long		GetTimeStamp() { return timestamp; };
	inline unsigned short			GetAdcValue() { return adc_value; };
	inline unsigned char			GetModule() { return mod; };
	inline unsigned char			GetAsic() { return asic; };
	inline unsigned char			GetChannel() { return ch; };
	inline bool						GetHitBit() { return hit_bit; };
	inline float					GetEnergy() { return energy; };
	inline double					GetWalk() { return walk; };
	inline bool						IsOverThreshold() { return thres; };

	inline void SetTimeStamp( unsigned long long t ){ timestamp = t; };
	inline void SetWalk( double w ){ walk = w; };
	inline void SetAdcValue( unsigned short adc ){ adc_value = adc; };
	inline void SetModule( unsigned char m ){ mod = m; };
	inline void SetAsic( unsigned char a ){ asic = a; };
	inline void SetChannel( unsigned char c ){ ch = c; };
	inline void SetHitBit( bool h ){ hit_bit = h; };
	inline void SetEnergy( float e ){ energy = e; };
	inline void SetThreshold( bool t ){ thres = t; };
	
	void ClearData();

protected:
	
	unsigned long long		timestamp;
	unsigned short			adc_value;
	unsigned char			mod;
	unsigned char			asic;
	unsigned char			ch;
	bool					hit_bit;
	bool					thres;		///< is the energy over threshold?
	float					energy;
	double					walk;

	
	ClassDef( ISSAsicData, 4 )
	
};

class ISSVmeData : public TObject {
	
public:
	
	ISSVmeData();
	~ISSVmeData();
	
	inline double			GetTime() { return (double)timestamp + finetime; };
	inline unsigned long	GetTimeStamp() { return timestamp; };
	inline float			GetFineTime() { return finetime; };
	inline float			GetBaseline() { return baseline; };
	inline unsigned short	GetTraceLength() { return trace.size(); };
	inline std::vector<unsigned short> GetTrace() { return trace; };
	inline unsigned short	GetSample( unsigned int i = 0 ) {
		if( i >= trace.size() ) return 0;
		return trace.at(i);
	};
	inline unsigned char	GetCrate() { return vme; };
	inline unsigned char	GetModule() { return mod; };
	inline unsigned char	GetChannel() { return ch; };
	inline unsigned short	GetQlong() { return Qlong; };
	inline unsigned short	GetQshort() { return Qshort; };
	inline unsigned short	GetQdiff() { return (int)Qlong-(int)Qshort; };
	inline float			GetEnergy() { return energy; };
	inline bool				IsOverThreshold() { return thres; };
	
	inline void	SetTimeStamp( unsigned long long t ) { timestamp = t; };
	inline void	SetFineTime( float t ) { finetime = t; };
	inline void	SetBaseline( float b ) { baseline = b; };
	inline void	SetTrace( std::vector<unsigned short> t ) { trace = t; };
	inline void AddSample( unsigned short s ) { trace.push_back(s); };
	inline void	SetQlong( unsigned short q ) { Qlong = q; };
	inline void	SetQshort( unsigned short q ) { Qshort = q; };
	inline void	SetCrate( unsigned char v ) { vme = v; };
	inline void	SetModule( unsigned char m ) { mod = m; };
	inline void	SetChannel( unsigned char c ) { ch = c; };
	inline void SetEnergy( float e ){ energy = e; };
	inline void SetThreshold( bool t ){ thres = t; };
	
	inline void ClearTrace() { trace.clear(); };
	void ClearData();
	
protected:
	
	unsigned long long			timestamp;
	float						finetime;
	float						baseline;
	std::vector<unsigned short>	trace;
	unsigned short				Qlong;
	unsigned short				Qshort;
	unsigned char				vme;
	unsigned char				mod;
	unsigned char				ch;
	bool						thres;		///< is the energy over threshold?
	float						energy;

	
	ClassDef( ISSVmeData, 1 )

};



class ISSCaenData : public ISSVmeData {
	
public:
	
	ISSCaenData();
	~ISSCaenData();

	ClassDef( ISSCaenData, 7 )
	
};


class ISSMesyData : public ISSVmeData {
	
public:
	
	ISSMesyData();
	~ISSMesyData();

	ClassDef( ISSMesyData, 1 )
	
};

class ISSInfoData : public TObject {
	
public:

	ISSInfoData();
	ISSInfoData( unsigned long long t, unsigned char c, unsigned char m );
	~ISSInfoData();
	
	inline double	 			GetTime(){ return (double)timestamp; };
	inline unsigned long long	GetTimeStamp(){ return timestamp; };
	inline unsigned char 		GetCode(){ return code; };
	inline unsigned char 		GetModule(){ return mod; };
	
	inline void SetTimeStamp( unsigned long long t ){ timestamp = t; };
	inline void SetCode( unsigned char c ){ code = c; };
	inline void SetModule( unsigned char m ){ mod = m; };

	void ClearData();

protected:
	
	unsigned long long		timestamp;	///< timestamp of info event
	unsigned char			code;	///< code here represents which information timestamp we have
	unsigned char			mod;	///< module ID of the event
	/// code = 4 is extended timestimp, i.e. next 16 bits
	/// code = 14 is external timestamp to ISS for checking sync to CAEN
	/// code = 20 is CAEN pulser event for checking sync to ISS
	/// code = 21 is EBIS proton timestamp
	/// code = 22 is T1 timestamp

	
	ClassDef( ISSInfoData, 2 )
	
};

class ISSDataPackets : public TObject {
	
public:
	
	inline bool	IsAsic() { return asic_packets.size(); };
	inline bool	IsVme() { return caen_packets.size() + mesy_packets.size(); };
	inline bool	IsCaen() { return caen_packets.size(); };
	inline bool	IsMesy() { return mesy_packets.size(); };
	inline bool	IsInfo() { return info_packets.size(); };
	
	void SetData( std::shared_ptr<ISSAsicData> data );
	void SetData( std::shared_ptr<ISSCaenData> data );
	void SetData( std::shared_ptr<ISSMesyData> data );
	void SetData( std::shared_ptr<ISSInfoData> data );

	// These methods are not very safe for access
	inline std::shared_ptr<ISSAsicData> GetAsicData() { return std::make_shared<ISSAsicData>( asic_packets.at(0) ); };
	inline std::shared_ptr<ISSCaenData> GetCaenData() { return std::make_shared<ISSCaenData>( caen_packets.at(0) ); };
	inline std::shared_ptr<ISSMesyData> GetMesyData() { return std::make_shared<ISSMesyData>( mesy_packets.at(0) ); };
	inline std::shared_ptr<ISSInfoData> GetInfoData() { return std::make_shared<ISSInfoData>( info_packets.at(0) ); };
	
	// Complicated way to get the time...
	double GetTime();
	double GetTimeWithWalk();
	unsigned long long GetTimeStamp();
	UInt_t GetTimeMSB();
	UInt_t GetTimeLSB();

	void ClearData();

protected:
	
	std::vector<ISSAsicData> asic_packets;
	std::vector<ISSCaenData> caen_packets;
	std::vector<ISSMesyData> mesy_packets;
	std::vector<ISSInfoData> info_packets;

	ClassDef( ISSDataPackets, 3 )

};


#endif
