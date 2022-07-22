#include "Settings.hh"

ISSSettings::ISSSettings( std::string filename ) {
	
	SetFile( filename );
	ReadSettings();
	
}

void ISSSettings::ReadSettings() {
	
	TEnv *config = new TEnv( fInputFile.data() );
	
	// Array initialisation
	n_array_mod = config->GetValue( "NumberOfArrayModules", 3 );
	n_array_asic = config->GetValue( "NumberOfArrayASICs", 6 );
	n_array_ch = config->GetValue( "NumberOfArrayChannels", 128 );
	
	// Array Geometry
	n_array_row = config->GetValue( "NumberOfArrayRows", 4 );
	n_array_pstrip = config->GetValue( "NumberOfArrayPstrips", 128 );
	n_array_nstrip = config->GetValue( "NumberOfArrayNstrips", 11 );
	
	// CAEN initialisation
	n_caen_mod = config->GetValue( "NumberOfCAENModules", 2 );
	n_caen_ch = config->GetValue( "NumberOfCAENChannels", 16 );
	
	// Info code initialisation
	extt_code = config->GetValue( "ExternalTriggerCode", 14 );
	ext_item_code = config->GetValue( "ExtItemCode", 7 );
	sync_code = config->GetValue( "SyncCode", 4 );
	thsb_code = config->GetValue( "TimestampCode", 5 );
	pause_code = config->GetValue( "PauseCode", 2 );
	resume_code = config->GetValue( "ResumeCode", 3 );
	asic_pulser_asic = config->GetValue( "PulserAsicArray", 1 );
	asic_pulser_ch = config->GetValue( "PulserChannelArray", 63 );
	asic_pulser_code = config->GetValue( "PulserCodeArray", 19 );
	asic_pulser_thres = config->GetValue( "PulserThresholdArray", 1000 );
	caen_pulser_mod = config->GetValue( "PulserModuleCAEN", 1 );
	caen_pulser_ch = config->GetValue( "PulserChannelCAEN", 13 );
	caen_pulser_code = config->GetValue( "PulserCodeCAEN", 20 );
	caen_ebis_mod = config->GetValue( "EBISModule", 1 );
	caen_ebis_ch = config->GetValue( "EBISChannel", 14 );
	ebis_code = config->GetValue( "EBISCode", 21 );
	caen_t1_mod = config->GetValue( "T1Module", 1 );
	caen_t1_ch = config->GetValue( "T1Channel", 15 );
	t1_code = config->GetValue( "T1Code", 22 );

	
	// Event builder
	event_window = config->GetValue( "EventWindow", 3e3 );
	recoil_hit_window = config->GetValue( "RecoilHitWindow", 500 );
	array_hit_window = config->GetValue( "ArrayHitWindow", 500 );
	zd_hit_window = config->GetValue( "ZeroDegreeHitWindow", 500 );

	
	// Data things
	block_size = config->GetValue( "DataBlockSize", 0x10000 );
	flag_asic_only = config->GetValue( "ASICOnlyData", false );
	flag_caen_only = config->GetValue( "CAENOnlyData", false );

	
	// Recoil detector
	n_recoil_sector = config->GetValue( "NumberOfRecoilSectors", 4 );
	n_recoil_layer  = config->GetValue( "NumberOfRecoilLayers", 2 );
	recoil_eloss_depth = config->GetValue( "RecoilEnergyLossDepth", 1 );

	recoil_mod.resize( n_recoil_sector );
	recoil_ch.resize( n_recoil_sector );
	recoil_sector.resize( n_caen_mod );
	recoil_layer.resize( n_caen_mod );
	
	for( unsigned int i = 0; i < n_caen_mod; ++i ){
		
		for( unsigned int j = 0; j < n_caen_ch; ++j ){
		
			recoil_sector[i].push_back( -1 );
			recoil_layer[i].push_back( -1 );
		
		}
	
	}
	
	for( unsigned int i = 0; i < n_recoil_sector; ++i ){
		
		recoil_mod[i].resize( n_recoil_layer );
		recoil_ch[i].resize( n_recoil_layer );

		for( unsigned int j = 0; j < n_recoil_layer; ++j ){
			
			recoil_mod[i][j] = config->GetValue( Form( "Recoil_%d_%d.Module", i, j ), 0 );
			recoil_ch[i][j] = config->GetValue( Form( "Recoil_%d_%d.Channel", i, j ), 2*(int)j+(int)i );
			
			if( recoil_mod[i][j] < n_caen_mod && recoil_ch[i][j] < n_caen_ch ){
				
				recoil_sector[recoil_mod[i][j]][recoil_ch[i][j]] = i;
				recoil_layer[recoil_mod[i][j]][recoil_ch[i][j]]  = j;
				
			}
			
			else {
				
				std::cerr << "Dodgy recoil settings: module = " << recoil_mod[i][j];
				std::cerr << " channel = " << recoil_ch[i][j] << std::endl;
				
			}
			
		}
		
	}
	
	// MWPC
	n_mwpc_axes = config->GetValue( "NumberOfMWPCAxes", 2 ); // x and y usually

	mwpc_mod.resize( n_mwpc_axes );
	mwpc_ch.resize( n_mwpc_axes );
	mwpc_axis.resize( n_caen_mod );
	mwpc_tac.resize( n_caen_mod );

	for( unsigned int i = 0; i < n_caen_mod; ++i ){
		
		for( unsigned int j = 0; j < n_caen_ch; ++j ){
		
			mwpc_axis[i].push_back( -1 );
			mwpc_tac[i].push_back( -1 );
		
		}
	
	}
	
	for( unsigned int i = 0; i < n_mwpc_axes; ++i ){
	
		mwpc_mod[i].resize( 2 );
		mwpc_ch[i].resize( 2 );
	
		for( unsigned int j = 0; j < 2; ++j ){ // two TACs per axis

			mwpc_mod[i][j] = config->GetValue( Form( "MWPC_%d_%d.Module", i, j ), 1 );
			mwpc_ch[i][j] = config->GetValue( Form( "MWPC_%d_%d.Channel", i, j ), 8+(int)i*2+(int)j );

			if( mwpc_mod[i][j] < n_caen_mod && mwpc_ch[i][j] < n_caen_ch ){
				
				mwpc_axis[mwpc_mod[i][j]][mwpc_ch[i][j]] = i;
				mwpc_tac[mwpc_mod[i][j]][mwpc_ch[i][j]] = j;

			}

			else {
				
				std::cerr << "Dodgy MWPC settings: module = " << mwpc_mod[i][j];
				std::cerr << " channel = " << mwpc_ch[i][j] << std::endl;
				
			}
		}
		
	}
	
	// ELUM detector
	n_elum_sector = config->GetValue( "NumberOfELUMSectors", 4 );
	
	elum_mod.resize( n_elum_sector );
	elum_ch.resize( n_elum_sector );
	elum_sector.resize( n_caen_mod );
	
	for( unsigned int i = 0; i < n_caen_mod; ++i )
		for( unsigned int j = 0; j < n_caen_ch; ++j )
			elum_sector[i].push_back( -1 );

	
	for( unsigned int i = 0; i < n_elum_sector; ++i ){
				
		elum_mod[i] = config->GetValue( Form( "ELUM_%d.Module", i ), 1 );
		elum_ch[i] = config->GetValue( Form( "ELUM_%d.Channel", i ), (int)i );
		
		if( elum_mod[i] < n_caen_mod && elum_ch[i] < n_caen_ch )
			elum_sector[elum_mod[i]][elum_ch[i]] = i;

		else {
			
			std::cerr << "Dodgy ELUM settings: module = " << elum_mod[i];
			std::cerr << " channel = " << elum_ch[i] << std::endl;
			
		}
			
	}
	
	// ZeroDegree detector
	n_zd_layer = config->GetValue( "NumberOfZDLayers", 2 );
	
	zd_mod.resize( n_zd_layer );
	zd_ch.resize( n_zd_layer );
	zd_layer.resize( n_caen_mod );
	
	for( unsigned int i = 0; i < n_caen_mod; ++i )
		for( unsigned int j = 0; j < n_caen_ch; ++j )
			zd_layer[i].push_back( -1 );
	
	for( unsigned int i = 0; i < n_zd_layer; ++i ){
		
		zd_mod[i] = config->GetValue( Form( "ZD_%d.Module", i ), 1 );
		zd_ch[i] = config->GetValue( Form( "ZD_%d.Channel", i ), (int)i+6 );
		
		if( zd_mod[i] < n_caen_mod && zd_ch[i] < n_caen_ch )
			zd_layer[zd_mod[i]][zd_ch[i]] = i;

		else {
			
			std::cerr << "Dodgy ELUM settings: module = " << elum_mod[i];
			std::cerr << " channel = " << elum_ch[i] << std::endl;
			
		}
		
	}
	
	
	// Finished
	delete config;
	
}


bool ISSSettings::IsRecoil( unsigned int mod, unsigned int ch ) {
	
	/// Return true if this is a recoil event
	if( recoil_sector[mod][ch] >= 0 ) return true;
	else return false;
	
}

int ISSSettings::GetRecoilSector( unsigned int mod, unsigned int ch ) {
	
	/// Return the sector or quadrant of a recoil event by module and channel number
	if( mod < n_caen_mod && ch < n_caen_ch )
		return recoil_sector[mod][ch];
	
	else {
		
		std::cerr << "Bad recoil event: module = " << mod;
		std::cerr << " channel = " << ch << std::endl;
		return -1;
		
	}
	
}

int ISSSettings::GetRecoilLayer( unsigned int mod, unsigned int ch ) {
	
	/// Return the sector or quadrant of a recoil event by module and channel number
	if( mod < n_caen_mod && ch < n_caen_ch )
		return recoil_layer[mod][ch];
	
	else {
		
		std::cerr << "Bad recoil event: module = " << mod;
		std::cerr << " channel = " << ch << std::endl;
		return -1;
		
	}
	
}

int ISSSettings::GetRecoilModule( unsigned char sec, unsigned int layer ){
	
	// Returns the module of the recoil detector
	if( sec < n_recoil_sector && layer < n_recoil_layer )
		return recoil_mod[(int)sec][layer];
	
	else {
		
		std::cerr << "Bad recoil event: sector = " << (int)sec;
		std::cerr << " layer = " << layer << std::endl;
		return -1;
		
	}
	
	
}

int ISSSettings::GetRecoilChannel( unsigned char sec, unsigned int layer ){
	
	// Returns the channel of the recoil detector
	if( sec < n_recoil_sector && layer < n_recoil_layer )
		return recoil_ch[(int)sec][layer];
	
	else {
		
		std::cerr << "Bad recoil event: sector = " << (int)sec;
		std::cerr << " layer = " << layer << std::endl;
		return -1;
		
	}
	
	
}

bool ISSSettings::IsMWPC( unsigned int mod, unsigned int ch ) {
	
	/// Return true if this is a MWPC event
	if( mwpc_axis[mod][ch] >= 0 && mwpc_axis[mod][ch] < (int)n_mwpc_axes ) return true;
	else return false;
	
}

int ISSSettings::GetMWPCAxis( unsigned int mod, unsigned int ch ) {
	
	/// Return the axis number of an MWPC event by module and channel number
	if( mod < n_caen_mod && ch < n_caen_ch )
		return mwpc_axis[mod][ch];
	
	else {
		
		std::cerr << "Bad MWPC event: module = " << mod;
		std::cerr << " channel = " << ch << std::endl;
		return -1;
		
	}
	
}

int ISSSettings::GetMWPCID( unsigned int mod, unsigned int ch ) {
	
	/// Return the TAC number of an MWPC event by module and channel number
	if( mod < n_caen_mod && ch < n_caen_ch )
		return mwpc_tac[mod][ch];
	
	else {
		
		std::cerr << "Bad MWPC event: module = " << mod;
		std::cerr << " channel = " << ch << std::endl;
		return -1;
		
	}
	
}

bool ISSSettings::IsELUM( unsigned int mod, unsigned int ch ) {
	
	/// Return true if this is an ELUM event
	if( elum_sector[mod][ch] >= 0 ) return true;
	else return false;
	
}


int ISSSettings::GetELUMSector( unsigned int mod, unsigned int ch ) {
	
	/// Return the sector or quadrant of a ELUM event by module and channel number
	if( mod < n_caen_mod && ch < n_caen_ch )
		return elum_sector[mod][ch];
	
	else {
		
		std::cerr << "Bad ELUM event: module = " << mod;
		std::cerr << " channel = " << ch << std::endl;
		return -1;
		
	}
	
}

bool ISSSettings::IsZD( unsigned int mod, unsigned int ch ) {
	
	/// Return true if this is an ZeroDegree event
	if( zd_layer[mod][ch] >= 0 ) return true;
	else return false;
	
}


int ISSSettings::GetZDLayer( unsigned int mod, unsigned int ch ) {
	
	/// Return the layer of a ZeroDegree event by module and channel number
	if( mod < n_caen_mod && ch < n_caen_ch )
		return zd_layer[mod][ch];
	
	else {
		
		std::cerr << "Bad ZeroDegree event: module = " << mod;
		std::cerr << " channel = " << ch << std::endl;
		return -1;
		
	}
	
}
