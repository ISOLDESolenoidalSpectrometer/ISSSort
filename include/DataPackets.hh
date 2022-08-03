#ifndef __DATAPACKETS_hh
#define __DATAPACKETS_hh

#include <memory>

#include "TObject.h"

class ISSAsicData : public TObject {
	
public:

	ISSAsicData();
	ISSAsicData( unsigned long t, unsigned short adc,
			  unsigned char m, unsigned char a,
			  unsigned char c, bool h, bool th,
			  float e, int w );
	~ISSAsicData();
	
	inline unsigned long	GetTime() { return time; };
	inline unsigned short	GetAdcValue() { return adc_value; };
	inline unsigned char	GetModule() { return mod; };
	inline unsigned char	GetAsic() { return asic; };
	inline unsigned char	GetChannel() { return ch; };
	inline bool				GetHitBit() { return hit_bit; };
	inline float			GetEnergy() { return energy; };
	inline int				GetWalk() { return walk; };
	inline bool				IsOverThreshold() { return thres; };

	inline void SetTime( unsigned long t ){ time = t; };
	inline void SetWalk( int w ){ walk = w; };
	inline void SetAdcValue( unsigned short adc ){ adc_value = adc; };
	inline void SetModule( unsigned char m ){ mod = m; };
	inline void SetAsic( unsigned char a ){ asic = a; };
	inline void SetChannel( unsigned char c ){ ch = c; };
	inline void SetHitBit( bool h ){ hit_bit = h; };
	inline void SetEnergy( float e ){ energy = e; };
	inline void SetThreshold( bool t ){ thres = t; };
	
	void ClearData();

protected:
	
	unsigned long	time;
	unsigned short	adc_value;
	unsigned char	mod;
	unsigned char	asic;
	unsigned char	ch;
	bool			hit_bit;
	bool			thres;		///< is the energy over threshold?
	float			energy;
	int				walk;

	
	ClassDef( ISSAsicData, 3 )
	
};

class ISSCaenData : public TObject {
	
public:

	ISSCaenData();
	ISSCaenData( unsigned long t, unsigned short f,
			  std::vector<unsigned short> tr,
			  unsigned short ql, unsigned short qs,
			  unsigned char m, unsigned char c,
			  bool th );
	~ISSCaenData();

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
	inline unsigned short	GetTAC() { return (int)Qlong-(int)Qshort; };
	inline float			GetEnergy() { return energy; };
	inline bool				IsOverThreshold() { return thres; };

	inline void	SetTime( unsigned long t ) { time = t; };
	inline void	SetFineTime( unsigned short t ) { finetime = t; };
	inline void	SetTrace( std::vector<unsigned short> t ) { trace = t; };
	inline void AddSample( unsigned short s ) { trace.push_back(s); };
	inline void	SetQlong( unsigned short q ) { Qlong = q; };
	inline void	SetQshort( unsigned short q ) { Qshort = q; };
	inline void	SetModule( unsigned char m ) { mod = m; };
	inline void	SetChannel( unsigned char c ) { ch = c; };
	inline void SetEnergy( float e ){ energy = e; };
	inline void SetThreshold( bool t ){ thres = t; };

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
	bool						thres;		///< is the energy over threshold?
	float						energy;

	
	ClassDef( ISSCaenData, 2 )
	
};

class ISSInfoData : public TObject {
	
public:

	ISSInfoData();
	ISSInfoData( unsigned long t, unsigned char c, unsigned char m );
	~ISSInfoData();
	
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

	
	ClassDef( ISSInfoData, 1 )
	
};

class ISSDataPackets : public TObject {
	
public:
	
	inline bool	IsAsic() { return asic_packets.size(); };
	inline bool	IsCaen() { return caen_packets.size(); };
	inline bool	IsInfo() { return info_packets.size(); };
	
	void SetData( std::shared_ptr<ISSAsicData> data );
	void SetData( std::shared_ptr<ISSCaenData> data );
	void SetData( std::shared_ptr<ISSInfoData> data );

	// These methods are not very safe for access
	inline std::shared_ptr<ISSAsicData> GetAsicData() { return std::make_shared<ISSAsicData>( asic_packets.at(0) ); };
	inline std::shared_ptr<ISSCaenData> GetCaenData() { return std::make_shared<ISSCaenData>( caen_packets.at(0) ); };
	inline std::shared_ptr<ISSInfoData> GetInfoData() { return std::make_shared<ISSInfoData>( info_packets.at(0) ); };
	
	// Complicated way to get the time...
	unsigned long GetTime();
	UInt_t GetTimeMSB();
	UInt_t GetTimeLSB();

	void ClearData();

protected:
	
	std::vector<ISSAsicData> asic_packets;
	std::vector<ISSCaenData> caen_packets;
	std::vector<ISSInfoData> info_packets;

	ClassDef( ISSDataPackets, 1 )

};


#endif
