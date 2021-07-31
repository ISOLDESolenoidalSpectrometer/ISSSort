#ifndef __DATAPACKETS_hh
#define __DATAPACKETS_hh

#include "TObject.h"

class AsicData : public TObject {
	
public:

	AsicData();
	AsicData( unsigned long t, unsigned short adc, unsigned char h,
			 unsigned char m, unsigned char a, unsigned char c );
	~AsicData();
	
	inline unsigned long	GetTime() { return time; };
	inline unsigned short	GetAdcValue() { return adc_value; };
	inline unsigned char	GetHitBit() { return hit_bit; };
	inline unsigned char	GetModule() { return mod; };
	inline unsigned char	GetAsic() { return asic; };
	inline unsigned char	GetChannel() { return ch; };
	inline float			GetEnergy() { return energy; };

	inline void SetTime( unsigned long t ){ time = t; };
	inline void SetAdcValue( unsigned short adc ){ adc_value = adc; };
	inline void SetHitBit( unsigned char h ){ hit_bit = h; };
	inline void SetModule( unsigned char m ){ mod = m; };
	inline void SetAsic( unsigned char a ){ asic = a; };
	inline void SetChannel( unsigned char c ){ ch = c; };
	inline void SetEnergy( float e ){ energy = e; };

	void ClearData();

protected:
	
	unsigned long	time;
	unsigned short	adc_value;
	unsigned char	hit_bit;
	unsigned char	mod;
	unsigned char	asic;
	unsigned char	ch;
	float			energy;

	
	ClassDef( AsicData, 1 )
	
};

class CaenData : public TObject {
	
public:

	CaenData();
	CaenData( unsigned long t, unsigned short f,
			  std::vector<unsigned short> tr,
			  unsigned short ql, unsigned short qs,
			  unsigned char m, unsigned char c );
	~CaenData();

	inline unsigned long	GetTime() { return time; };
	inline unsigned short	GetFineTime() { return finetime; };
	inline unsigned short	GetTraceLength() { return trace.size(); };
	inline std::vector<unsigned short> GetTrace() { return trace; };
	inline unsigned short	GetSample( unsigned int i = 0 ) {
		if( i >= trace.size() ) return 0;
		return trace.at(i);
	};
	inline unsigned short	GetQlong() { return Qlong; };
	inline unsigned short	GetQshort() { return Qshort; };
	inline unsigned char	GetModule() { return mod; };
	inline unsigned char	GetChannel() { return ch; };
	inline float			GetEnergy() { return energy; };

	inline void	SetTime( unsigned long t ) { time = t; };
	inline void	SetFineTime( unsigned short t ) { finetime = t; };
	inline void	SetTrace( std::vector<unsigned short> t ) { trace = t; };
	inline void AddSample( unsigned short s ) { trace.push_back(s); };
	inline void	SetQlong( unsigned short q ) { Qlong = q; };
	inline void	SetQshort( unsigned short q ) { Qshort = q; };
	inline void	SetModule( unsigned char m ) { mod = m; };
	inline void	SetChannel( unsigned char c ) { ch = c; };
	inline void SetEnergy( float e ){ energy = e; };

	inline void ClearTrace() { trace.clear(); };
	void ClearData();

protected:
	
	unsigned long				time;
	unsigned short				finetime;
	std::vector<unsigned short>	trace;
	unsigned short				Qlong;
	unsigned short				Qshort;
	unsigned char				mod;
	unsigned char				ch;
	float						energy;

	
	ClassDef( CaenData, 1 )
	
};

class InfoData : public TObject {
	
public:

	InfoData();
	InfoData( unsigned long t, unsigned char c, unsigned char m );
	~InfoData();
	
	inline unsigned long GetTime(){ return time; };
	inline unsigned char GetCode(){ return code; };
	inline unsigned char GetModule(){ return mod; };
	
	inline void SetTime( unsigned long t ){ time = t; };
	inline void SetCode( unsigned char c ){ code = c; };
	inline void SetModule( unsigned char m ){ mod = m; };

	void ClearData();

protected:
	
	unsigned long	time;	///< timestamp of info event
	unsigned char	code;	///< code here represents which information timestamp we have
	unsigned char	mod;	///< module ID of the event
	/// code = 4 is extended timestimp, i.e. next 16 bits
	/// code = 14 is external timestamp to ISS for checking sync to CAEN
	/// code = 20 is CAEN pulser event for checking sync to ISS
	/// code = 21 is EBIS proton timestamp
	/// code = 22 is T1 timestamp

	
	ClassDef( InfoData, 1 )
	
};

class DataPackets : public TObject {
	
public:
	
	inline bool	IsAsic() { return asic_packets.size(); };
	inline bool	IsCaen() { return caen_packets.size(); };
	inline bool	IsInfo() { return info_packets.size(); };
	
	//inline void SetData( AsicData *data ) { ClearData(); asic_packets.push_back( data ); };
	//inline void SetData( CaenData *data ) { ClearData(); caen_packets.push_back( data ); };
	//inline void SetData( InfoData *data ) { ClearData(); info_packets.push_back( data ); };
	void SetData( AsicData *data );
	void SetData( CaenData *data );
	void SetData( InfoData *data );

	// These methods are not very safe for access
	inline AsicData* GetAsicData() { return &asic_packets.at(0); };
	inline CaenData* GetCaenData() { return &caen_packets.at(0); };
	inline InfoData* GetInfoData() { return &info_packets.at(0); };
	
	// Complicated way to get the time...
	unsigned long GetTime();
	UInt_t GetTimeMSB();
	UInt_t GetTimeLSB();

	void ClearData();

protected:
	
	std::vector<AsicData> asic_packets;
	std::vector<CaenData> caen_packets;
	std::vector<InfoData> info_packets;

	ClassDef( DataPackets, 1 )

};


#endif
