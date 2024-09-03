#include "DataPackets.hh"

ClassImp(ISSAsicData)
ClassImp(ISSVmeData)
ClassImp(ISSCaenData)
ClassImp(ISSMesyData)
ClassImp(ISSInfoData)
ClassImp(ISSDataPackets)

ISSAsicData::ISSAsicData(){}
ISSAsicData::~ISSAsicData(){}

ISSVmeData::ISSVmeData(){}
ISSVmeData::~ISSVmeData(){}

ISSCaenData::ISSCaenData(){}
ISSCaenData::~ISSCaenData(){}

ISSMesyData::ISSMesyData(){}
ISSMesyData::~ISSMesyData(){}

ISSInfoData::ISSInfoData(){}
ISSInfoData::~ISSInfoData(){}


void ISSDataPackets::SetData( std::shared_ptr<ISSAsicData> data ){
	
	// Reset the vector to size = 0
	// We only want to have one element per Tree entry
	ClearData();
	
	// Make a copy of the input data and push it back
	ISSAsicData fill_data;
	fill_data.SetTimeStamp( data->GetTime() );
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

void ISSDataPackets::SetData( std::shared_ptr<ISSCaenData> data ){
	
	// Reset the vector to size = 0
	// We only want to have one element per Tree entry
	ClearData();
	
	// Make a copy of the input data and push it back
	ISSCaenData fill_data;
	
	fill_data.SetTimeStamp( data->GetTimeStamp() );
	fill_data.SetFineTime( data->GetFineTime() );
	fill_data.SetBaseline( data->GetBaseline() );
	fill_data.SetTrace( data->GetTrace() );
	fill_data.SetQlong( data->GetQlong() );
	fill_data.SetQshort( data->GetQshort() );
	fill_data.SetCrate( data->GetCrate() );
	fill_data.SetModule( data->GetModule() );
	fill_data.SetChannel( data->GetChannel() );
	fill_data.SetEnergy( data->GetEnergy() );
	fill_data.SetThreshold( data->IsOverThreshold() );
	
	caen_packets.push_back( fill_data );
	
}

void ISSDataPackets::SetData( std::shared_ptr<ISSMesyData> data ){
	
	// Reset the vector to size = 0
	// We only want to have one element per Tree entry
	ClearData();
	
	// Make a copy of the input data and push it back
	ISSMesyData fill_data;
	
	fill_data.SetTimeStamp( data->GetTimeStamp() );
	fill_data.SetFineTime( data->GetFineTime() );
	fill_data.SetBaseline( data->GetBaseline() );
	fill_data.SetTrace( data->GetTrace() );
	fill_data.SetQlong( data->GetQlong() );
	fill_data.SetQshort( data->GetQshort() );
	fill_data.SetCrate( data->GetCrate() );
	fill_data.SetModule( data->GetModule() );
	fill_data.SetChannel( data->GetChannel() );
	fill_data.SetEnergy( data->GetEnergy() );
	fill_data.SetThreshold( data->IsOverThreshold() );
	
	mesy_packets.push_back( fill_data );
	
}

void ISSDataPackets::SetData( std::shared_ptr<ISSInfoData> data ){
	
	// Reset the vector to size = 0
	// We only want to have one element per Tree entry
	ClearData();
	
	// Make a copy of the input data and push it back
	ISSInfoData fill_data;
	fill_data.SetTimeStamp( data->GetTime() );
	fill_data.SetCode( data->GetCode() );
	fill_data.SetModule( data->GetModule() );

	info_packets.push_back( fill_data );
	
}


void ISSDataPackets::ClearData(){
	
	asic_packets.clear();
	caen_packets.clear();
	mesy_packets.clear();
	info_packets.clear();
	
	std::vector<ISSAsicData>().swap(asic_packets);
	std::vector<ISSCaenData>().swap(caen_packets);
	std::vector<ISSMesyData>().swap(mesy_packets);
	std::vector<ISSInfoData>().swap(info_packets);

	return;
	
}

double ISSDataPackets::GetTime(){
		
	if( IsAsic() ) return GetAsicData()->GetTime();
	if( IsCaen() ) return GetCaenData()->GetTime();
	if( IsMesy() ) return GetMesyData()->GetTime();
	if( IsInfo() ) return GetInfoData()->GetTime();

	return 0;
	
}

double ISSDataPackets::GetTimeWithWalk(){
		
	if( IsAsic() ) return GetAsicData()->GetTime() + GetAsicData()->GetWalk();
	if( IsCaen() ) return GetCaenData()->GetTime();
	if( IsMesy() ) return GetMesyData()->GetTime();
	if( IsInfo() ) return GetInfoData()->GetTime();

	return 0;
	
}

unsigned long long ISSDataPackets::GetTimeStamp(){
		
	if( IsAsic() ) return GetAsicData()->GetTimeStamp();
	if( IsCaen() ) return GetCaenData()->GetTimeStamp();
	if( IsMesy() ) return GetMesyData()->GetTimeStamp();
	if( IsInfo() ) return GetInfoData()->GetTimeStamp();

	return 0;
	
}

UInt_t ISSDataPackets::GetTimeMSB(){
	
	return ( ((unsigned long long)this->GetTimeStamp() >> 32) & 0xFFFFFFFF );
	
}

UInt_t ISSDataPackets::GetTimeLSB(){
	
	return (UInt_t)this->GetTimeStamp();
	
}

void ISSAsicData::ClearData(){
	
	timestamp = 0.0;
	walk = 0;
	adc_value = 0;
	hit_bit = 2;  ///< valid values are 0 and 1, so reset to 2
	mod = 255;
	asic = 255;
	ch = 255;
	energy = -999.;
	thres = true;

	return;
	
}

void ISSVmeData::ClearData(){
	
	timestamp = 0.0;
	finetime = 0.0;
	baseline = 0.0;
	trace.clear();
	Qlong = 0;
	Qshort = 0;
	vme = 255;
	mod = 255;
	ch = 255;
	energy = -999.;
	thres = true;
	
	return;
	
}



void ISSInfoData::ClearData(){
	
	timestamp = 0.0;
	code = 0;
	mod = 255;
	
	return;
	
}

