#include "DataPackets.hh"

ClassImp(ISSAsicData)
ClassImp(ISSCaenData)
ClassImp(ISSInfoData)
ClassImp(ISSDataPackets)

ISSAsicData::ISSAsicData(){}
ISSAsicData::~ISSAsicData(){}
ISSAsicData::ISSAsicData( unsigned long t, unsigned short adc, unsigned char m,
					unsigned char a, unsigned char c, bool h, bool th, float e, int w ) :
					time(t), adc_value(adc), mod(m), asic(a), ch(c), hit_bit(h), thres(th), energy(e), walk(w) {}

ISSCaenData::ISSCaenData(){}
ISSCaenData::~ISSCaenData(){}
ISSCaenData::ISSCaenData( unsigned long t, unsigned short f,
				    std::vector<unsigned short> tr,
					unsigned short ql, unsigned short qs,
					unsigned char m, unsigned char c,
				    bool th ) :
					time(t), finetime(f), trace(tr), Qlong(ql), Qshort(qs), mod(m), ch(c), thres(th) {}

ISSInfoData::ISSInfoData(){}
ISSInfoData::~ISSInfoData(){}
ISSInfoData::ISSInfoData( unsigned long t, unsigned char c, unsigned char m ) :
					time(t), code(c), mod(m) {}


void ISSDataPackets::SetData( ISSAsicData *data ){
	
	// Reset the vector to size = 0
	// We only want to have one element per Tree entry
	ClearData();
	
	// Make a copy of the input data and push it back
	ISSAsicData fill_data;
	fill_data.SetTime( data->GetTime() );
	fill_data.SetWalk( data->GetWalk() );
	fill_data.SetAdcValue( data->GetAdcValue() );
	fill_data.SetHitBit( data->GetHitBit() );
	fill_data.SetModule( data->GetModule() );
	fill_data.SetAsic( data->GetAsic() );
	fill_data.SetChannel( data->GetChannel() );
	fill_data.SetEnergy( data->GetEnergy() );
	fill_data.SetThreshold( data->IsOverThreshold() );

	asic_packets.push_back( fill_data );
	
}

void ISSDataPackets::SetData( ISSCaenData *data ){
	
	// Reset the vector to size = 0
	// We only want to have one element per Tree entry
	ClearData();
	
	// Make a copy of the input data and push it back
	ISSCaenData fill_data;
	
	fill_data.SetTime( data->GetTime() );
	fill_data.SetFineTime( data->GetFineTime() );
	fill_data.SetTrace( data->GetTrace() );
	fill_data.SetQlong( data->GetQlong() );
	fill_data.SetQshort( data->GetQshort() );
	fill_data.SetModule( data->GetModule() );
	fill_data.SetChannel( data->GetChannel() );
	fill_data.SetEnergy( data->GetEnergy() );

	caen_packets.push_back( fill_data );

}

void ISSDataPackets::SetData( ISSInfoData *data ){
	
	// Reset the vector to size = 0
	// We only want to have one element per Tree entry
	ClearData();
	
	// Make a copy of the input data and push it back
	ISSInfoData fill_data;
	fill_data.SetTime( data->GetTime() );
	fill_data.SetCode( data->GetCode() );
	fill_data.SetModule( data->GetModule() );

	info_packets.push_back( fill_data );
	
}


void ISSDataPackets::ClearData(){
	
	asic_packets.clear();
	caen_packets.clear();
	info_packets.clear();
	
	return;
	
}

unsigned long ISSDataPackets::GetTime(){
		
	if( IsAsic() ) return GetAsicData()->GetTime();
	if( IsCaen() ) return GetCaenData()->GetTime();
	if( IsInfo() ) return GetInfoData()->GetTime();

	return 0;
	
}

UInt_t ISSDataPackets::GetTimeMSB(){
	
	return ( (this->GetTime() >> 32) & 0x0000FFFF );
	
}

UInt_t ISSDataPackets::GetTimeLSB(){
	
	return (UInt_t)this->GetTime();
	
}

void ISSCaenData::ClearData(){
	
	time = 0;
	finetime = 0;
	trace.clear();
	Qlong = 0;
	Qshort = 0;
	mod = 255;
	ch = 255;
	energy = -999.;

	return;
	
}

void ISSAsicData::ClearData(){
	
	time = 0;
	walk = 0;
	adc_value = 0;
	hit_bit = 2;  ///< valid values are 0 and 1, so reset to 2
	mod = 255;
	asic = 255;
	ch = 255;
	energy = -999.;

	return;
	
}

void ISSInfoData::ClearData(){
	
	time = 0;
	code = 0;
	mod = 255;
	
	return;
	
}

