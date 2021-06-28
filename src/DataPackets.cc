#include "DataPackets.hh"

ClassImp(AsicData)
ClassImp(CaenData)
ClassImp(InfoData)
ClassImp(DataPackets)

AsicData::AsicData(){}
AsicData::~AsicData(){}
AsicData::AsicData( unsigned long t, unsigned short adc, unsigned char h,
					unsigned char m, unsigned char a, unsigned char c ) :
					time(t), adc_value(adc), hit_bit(h), mod(m), asic(a), ch(c) {}

CaenData::CaenData(){}
CaenData::~CaenData(){}
CaenData::CaenData( unsigned long t, unsigned short f,
				    std::vector<unsigned short> tr,
					unsigned short ql, unsigned short qs,
					unsigned char m, unsigned char c ) :
					time(t), finetime(f), trace(tr), Qlong(ql), Qshort(qs), mod(m), ch(c) {}

InfoData::InfoData(){}
InfoData::~InfoData(){}
InfoData::InfoData( unsigned long t, unsigned char c ) :
					time(t), code(c) {}


void DataPackets::SetData( AsicData *data ){
	
	// Reset the vector to size = 0
	// We only want to have one element per Tree entry
	ClearData();
	
	// Make a copy of the input data and push it back
	AsicData fill_data;
	fill_data.SetTime( data->GetTime() );
	fill_data.SetAdcValue( data->GetAdcValue() );
	fill_data.SetHitBit( data->GetHitBit() );
	fill_data.SetModule( data->GetModule() );
	fill_data.SetAsic( data->GetAsic() );
	fill_data.SetChannel( data->GetChannel() );
	fill_data.SetEnergy( data->GetEnergy() );

	asic_packets.push_back( fill_data );
	
}

void DataPackets::SetData( CaenData *data ){
	
	// Reset the vector to size = 0
	// We only want to have one element per Tree entry
	ClearData();
	
	// Make a copy of the input data and push it back
	CaenData fill_data;
	
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

void DataPackets::SetData( InfoData *data ){
	
	// Reset the vector to size = 0
	// We only want to have one element per Tree entry
	ClearData();
	
	// Make a copy of the input data and push it back
	InfoData fill_data;
	fill_data.SetTime( data->GetTime() );
	fill_data.SetCode( data->GetCode() );
	
	info_packets.push_back( fill_data );
	
}


void DataPackets::ClearData(){
	
	asic_packets.clear();
	caen_packets.clear();
	info_packets.clear();
	
	return;
	
}

unsigned long DataPackets::GetTime(){
		
	if( IsAsic() ) return GetAsicData()->GetTime();
	if( IsCaen() ) return GetCaenData()->GetTime();
	if( IsInfo() ) return GetInfoData()->GetTime();

	return 0;
	
}

UInt_t DataPackets::GetTimeMSB(){
	
	return ( (this->GetTime() >> 32) & 0x0000FFFF );
	
}

UInt_t DataPackets::GetTimeLSB(){
	
	return (UInt_t)this->GetTime();
	
}

void CaenData::ClearData(){
	
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

void AsicData::ClearData(){
	
	time = 0;
	adc_value = 0;
	hit_bit = 2;  ///< valid values are 0 and 1, so reset to 2
	mod = 255;
	asic = 255;
	ch = 255;
	energy = -999.;

	return;
	
}

void InfoData::ClearData(){
	
	time = 0;
	code = 0;
	
	return;
	
}

