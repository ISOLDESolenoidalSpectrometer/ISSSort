#ifndef __DATAPACKETS_HH
#define __DATAPACKETS_HH

#include <memory>
#include <iostream>

#include "TObject.h"
#include "TGraph.h"

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
	inline TGraph* GetTraceGraph() {
		std::vector<int> x, y;
		std::string title = "Trace for crate " + std::to_string( GetCrate() );
		title += ", module " + std::to_string( GetModule() );
		title += ", channel " + std::to_string( GetChannel() );
		title += ";time [samples];signal";
		for( unsigned short i = 0; i < GetTraceLength(); ++i ){
			x.push_back( i );
			y.push_back( GetSample(i) );
		}
		std::shared_ptr<TGraph> g = std::make_shared<TGraph>(
					GetTraceLength(), x.data(), y.data() );
		g.get()->SetTitle( title.data() );
		return (TGraph*)g.get()->Clone();
	};

	inline unsigned char	GetCrate() { return vme; };
	inline unsigned char	GetModule() { return mod; };
	inline unsigned char	GetChannel() { return ch; };
	inline unsigned short	GetQlong() { return Qlong; };
	inline unsigned short	GetQshort() { return Qshort; };
	inline unsigned short	GetQdiff() {
		if( overflow_long || overflow_short )
			return 0xffff;
		else
			return (int)Qlong-(int)Qshort;
	};
	inline float			GetEnergy() { return energy; };
	inline bool				IsOverThreshold() { return thres; };
	inline bool				IsOverflowLong() { return overflow_long; };
	inline bool				IsOverflowShort() { return overflow_short; };
	inline bool				IsClipped() { return clipped; };

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
	inline void	SetOverflowLong( bool o ) { overflow_long = o; };
	inline void	SetOverflowShort( bool o ) { overflow_short = o; };
	inline void	SetClipped( bool c ) { clipped = c; };

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
	bool						clipped;	///< does firmware park pluse clipped?
	bool						overflow_long;
	bool						overflow_short;
	float						energy;


	ClassDef( ISSVmeData, 2 )

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

	ISSDataPackets() {};
	~ISSDataPackets() {};

	ISSDataPackets( std::shared_ptr<ISSDataPackets> in ) { SetData(in); };
	ISSDataPackets( std::shared_ptr<ISSAsicData> in ) { SetData(in); };
	ISSDataPackets( std::shared_ptr<ISSCaenData> in ) { SetData(in); };
	ISSDataPackets( std::shared_ptr<ISSMesyData> in ) { SetData(in); };
	ISSDataPackets( std::shared_ptr<ISSInfoData> in ) { SetData(in); };

	inline bool	IsAsic() const { return asic_packets.size(); };
	inline bool	IsVme()  const { return caen_packets.size() + mesy_packets.size(); };
	inline bool	IsCaen() const { return caen_packets.size(); };
	inline bool	IsMesy() const { return mesy_packets.size(); };
	inline bool	IsInfo() const { return info_packets.size(); };

	void SetData( std::shared_ptr<ISSDataPackets> in ){
		if( in->IsAsic() ) SetData( in->GetAsicData() );
		if( in->IsCaen() ) SetData( in->GetCaenData() );
		if( in->IsMesy() ) SetData( in->GetMesyData() );
		if( in->IsInfo() ) SetData( in->GetInfoData() );
	};
	void SetData( std::shared_ptr<ISSAsicData> data );
	void SetData( std::shared_ptr<ISSCaenData> data );
	void SetData( std::shared_ptr<ISSMesyData> data );
	void SetData( std::shared_ptr<ISSInfoData> data );

	// These methods are not very safe for access
	inline std::shared_ptr<ISSVmeData> GetVmeData() const {
		if( caen_packets.size() )
			return std::make_shared<ISSVmeData>( caen_packets.at(0) );
		else if( mesy_packets.size() )
			return std::make_shared<ISSVmeData>( mesy_packets.at(0) );
		else return nullptr;
	};
	inline std::shared_ptr<ISSAsicData> GetAsicData() const {
		return std::make_shared<ISSAsicData>( asic_packets.at(0) );
	};
	inline std::shared_ptr<ISSCaenData> GetCaenData() const {
		return std::make_shared<ISSCaenData>( caen_packets.at(0) );
	};
	inline std::shared_ptr<ISSMesyData> GetMesyData() const {
		return std::make_shared<ISSMesyData>( mesy_packets.at(0) );
	};
	inline std::shared_ptr<ISSInfoData> GetInfoData() const {
		return std::make_shared<ISSInfoData>( info_packets.at(0) );
	};

	// Complicated way to get the time...
	double GetTime() const;
	double GetTimeWithWalk() const;
	unsigned long long GetTimeStamp() const;
	UInt_t GetTimeMSB() const;
	UInt_t GetTimeLSB() const;

	// Sorting function to do time ordering
	bool operator <( const ISSDataPackets &data ) const {
		return( GetTime() < data.GetTime() );
	};

	void ClearData();

protected:

	std::vector<ISSAsicData> asic_packets;
	std::vector<ISSCaenData> caen_packets;
	std::vector<ISSMesyData> mesy_packets;
	std::vector<ISSInfoData> info_packets;

	ClassDef( ISSDataPackets, 5 )

};


#endif
