#include "Settings.hh"

#include <iomanip>

ISSSettings::ISSSettings() {

	// Just defaults for the default constructor
	ISSSettings( "defaults" );

}

ISSSettings::ISSSettings( std::string filename ) {

	// Set filename
	SetFile( filename );

	// Go read the settings
	ReadSettings();

}

/// Copy constructor
ISSSettings::ISSSettings( ISSSettings *myset ){

	if( !myset ) return; // Handle null pointer case

	// Set filename
	SetFile( myset->InputFile() );

	// Copy array settings
	n_array_mod = myset->GetNumberOfArrayModules();
	n_array_asic = myset->GetNumberOfArrayASICs();
	n_array_ch = myset->GetNumberOfArrayChannels();
	n_array_row = myset->GetNumberOfArrayRows();
	n_array_pstrip = myset->GetNumberOfArrayPstrips();
	n_array_nstrip = myset->GetNumberOfArrayNstrips();

	// Copy CAEN settings
	n_caen_mod = myset->GetNumberOfCAENModules();
	n_caen_ch = myset->GetNumberOfCAENChannels();
	caen_model = myset->GetCAENModels();

	// Copy Mesytec settings
	n_mesy_mod = myset->GetNumberOfMesytecModules();
	n_mesy_ch = myset->GetNumberOfMesytecChannels();
	n_mesy_logic = myset->GetNumberOfMesytecLogicInputs();

	// Copy info code settings
	extt_code = myset->GetExternalTriggerCode();
	sync_code = myset->GetSyncCode();
	ext_item_code = myset->GetExtItemCode();
	thsb_code = myset->GetTimestampCode();
	pause_code = myset->GetPauseCode();
	resume_code = myset->GetResumeCode();

	// Copy pulser and timing signal settings
	asic_pulser_asic_0 = myset->GetArrayPulserAsic0();
	asic_pulser_ch_0 = myset->GetArrayPulserChannel0();
	asic_pulser_code_0 = myset->GetArrayPulserCode0();
	asic_pulser_asic_1 = myset->GetArrayPulserAsic1();
	asic_pulser_ch_1 = myset->GetArrayPulserChannel1();
	asic_pulser_code_1 = myset->GetArrayPulserCode1();
	asic_pulser_thres = myset->GetArrayPulserThreshold();
	caen_pulser_mod = myset->GetCAENPulserModule();
	caen_pulser_ch = myset->GetCAENPulserChannel();
	caen_pulser_code =GetCAENPulserCode();
	mesy_pulser_mod = myset->GetMesytecPulserModule();
	mesy_pulser_ch = myset->GetMesytecPulserChannel();
	mesy_pulser_code = myset->GetMesytecPulserCode();
	vme_ebis_crate = myset->GetEBISCrate();
	vme_ebis_mod = myset->GetEBISModule();
	vme_ebis_ch = myset->GetEBISChannel();
	ebis_code = myset->GetEBISCode();
	vme_t1_crate = myset->GetT1Crate();
	vme_t1_mod = myset->GetT1Module();
	vme_t1_ch = myset->GetT1Channel();
	t1_code = myset->GetT1Code();
	vme_sc_crate = myset->GetSCCrate();
	vme_sc_mod = myset->GetSCModule();
	vme_sc_ch = myset->GetSCChannel();
	sc_code = myset->GetSCCode();
	vme_laser_crate = myset->GetLaserCrate();
	vme_laser_mod = myset->GetLaserModule();
	vme_laser_ch = myset->GetLaserChannel();
	laser_code = myset->GetLaserCode();

	// Copy event builder settings
	build_by_tm_stp = myset->BuildByTimeStamp();
	event_window = myset->GetEventWindow();
	recoil_hit_window = myset->GetRecoilHitWindow();
	array_pn_hit_window = myset->GetArrayPNHitWindow();
	array_pp_hit_window = myset->GetArrayPPHitWindow();
	array_nn_hit_window = myset->GetArrayNNHitWindow();
	zd_hit_window = myset->GetZeroDegreeHitWindow();
	gamma_hit_window = myset->GetGammaRayHitWindow();
	lume_hit_window = myset->GetLumeHitWindow();
	cd_rs_hit_window = myset->GetCDRSHitWindow();
	cd_dd_hit_window = myset->GetCDDDHitWindow();

	// Copy data settings
	block_size = myset->GetBlockSize();
	flag_asic_only = myset->IsASICOnly();
	flag_caen_only = myset->IsCAENOnly();
	flag_mesy_only = myset->IsMesyOnly();

	// Copy event rejection settings
	clipped_reject = myset->GetClippedRejection();
	overflow_reject = myset->GetOverflowRejection();

	// Copy recoil detector settings
	n_recoil_sector = myset->GetNumberOfRecoilSectors();
	n_recoil_layer = myset->GetNumberOfRecoilLayers();
	recoil_eloss_start = myset->GetRecoilEnergyLossStop();
	recoil_eloss_stop = myset->GetRecoilEnergyLossStop();
	recoil_erest_start = myset->GetRecoilEnergyRestStart();
	recoil_erest_stop = myset->GetRecoilEnergyRestStop();
	recoil_etot_start = myset->GetRecoilEnergyTotalStart();
	recoil_etot_stop = myset->GetRecoilEnergyTotalStop();
	recoil_vme = myset->GetRecoilCrates();
	recoil_mod = myset->GetRecoilModules();
	recoil_ch = myset->GetRecoilChannels();
	recoil_sector = myset->GetRecoilSectors();
	recoil_layer = myset->GetRecoilLayers();

	// Copy MWPC settings
	n_mwpc_axes = myset->GetNumberOfMWPCAxes();
	mwpc_vme = myset->GetMwpcCrates();
	mwpc_mod = myset->GetMwpcModules();
	mwpc_ch  = myset->GetMwpcChannels();
	mwpc_axis = myset->GetMwpcAxes();
	mwpc_tac = myset->GetMwpcTacs();

	// Copy ELUM settings
	n_elum_sector = myset->GetNumberOfELUMSectors();
	elum_vme = myset->GetELUMCrates();
	elum_mod = myset->GetELUMModules();
	elum_ch  = myset->GetELUMChannels();
	elum_sector = myset->GetELUMSectors();

	// Copy ZeroDegree settings
	n_zd_layer = myset->GetNumberOfZDLayers();
	zd_vme = myset->GetZDCrates();
	zd_mod = myset->GetZDModules();
	zd_ch  = myset->GetZDChannels();
	zd_layer = myset->GetZDLayers();

	// Copy ScintArray settings
	n_scint_detector = myset->GetNumberOfScintArrayDetectors();
	scint_vme = myset->GetScintArrayCrates();
	scint_mod = myset->GetScintArrayModules();
	scint_ch  = myset->GetScintArrayChannels();
	scint_detector = myset->GetScintArrayDetectors();

	// Copy LUME settings
	n_lume = myset->GetNumberOfLUMEDetectors();
	lume_vme = myset->GetLUMECrates();
	lume_mod = myset->GetLUMEModules();
	lume_ch  = myset->GetLUMEChannels();
	lume_detector = myset->GetLUMEDetectors();
	lume_type = myset->GetLUMETypes();

	// Copy CD detector settings
	n_cd_sector = myset->GetNumberOfCDSectors();
	n_cd_layer = myset->GetNumberOfCDLayers();
	cd_eloss_start = myset->GetCDEnergyLossStop();
	cd_eloss_stop = myset->GetCDEnergyLossStop();
	cd_erest_start = myset->GetCDEnergyRestStart();
	cd_erest_stop = myset->GetCDEnergyRestStop();
	cd_etot_start = myset->GetCDEnergyTotalStart();
	cd_etot_stop = myset->GetCDEnergyTotalStop();
	cd_vme = myset->GetCDCrates();
	cd_mod = myset->GetCDModules();
	cd_ch  = myset->GetCDChannels();
	cd_layer = myset->GetCDLayers();
	cd_strip = myset->GetCDStrips();
	cd_side = myset->GetCDSides();

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
	for( unsigned int i = 0; i < n_caen_mod; ++i )
		caen_model.push_back( config->GetValue( Form( "CAEN_%d.Model", i ), 1725 ) );

	// Mesytec initialisation
	n_mesy_mod = config->GetValue( "NumberOfMesytecModules", 0 );
	n_mesy_ch = config->GetValue( "NumberOfMesytecChannels", 32 );
	n_mesy_logic = config->GetValue( "NumberOfMesytecLogicInputs", 2 );


	// Info code initialisation
	unsigned char tmp_val_uchar;
	extt_code = config->GetValue( "ExternalTriggerCode", 14 );
	ext_item_code = config->GetValue( "ExtItemCode", 7 );
	sync_code = config->GetValue( "SyncCode", 4 );
	thsb_code = config->GetValue( "TimestampCode", 5 );
	pause_code = config->GetValue( "PauseCode", 2 );
	resume_code = config->GetValue( "ResumeCode", 3 );
	asic_pulser_asic_0 = config->GetValue( "PulserAsicArray_0", 1 );
	asic_pulser_ch_0 = config->GetValue( "PulserChannelArray_0", 63 );
	asic_pulser_code_0 = config->GetValue( "PulserCodeArray_0", 18 );
	asic_pulser_asic_1 = config->GetValue( "PulserAsicArray_1", 4 );
	asic_pulser_ch_1 = config->GetValue( "PulserChannelArray_1", 63 );
	asic_pulser_code_1 = config->GetValue( "PulserCodeArray_1", 19 );
	asic_pulser_thres = config->GetValue( "PulserThresholdArray", 1000 );
	caen_pulser_mod = config->GetValue( "PulserModuleCAEN", 1 );
	caen_pulser_ch = config->GetValue( "PulserChannelCAEN", 13 );
	caen_pulser_code = config->GetValue( "PulserCodeCAEN", 20 );
	mesy_pulser_mod = config->GetValue( "PulserModuleMesytec", 1 );
	mesy_pulser_ch = config->GetValue( "PulserChannelMesytec", 13 );
	mesy_pulser_code = 25;
	tmp_val_uchar = config->GetValue( "EBISCrate", 0 );
	vme_ebis_crate = config->GetValue( "EBIS.Crate", tmp_val_uchar );
	tmp_val_uchar = config->GetValue( "EBISModule", 1 );
	vme_ebis_mod = config->GetValue( "EBIS.Module", tmp_val_uchar );
	tmp_val_uchar = config->GetValue( "EBISChannel", 14 );
	vme_ebis_ch = config->GetValue( "EBIS.Channel", tmp_val_uchar );
	tmp_val_uchar = config->GetValue( "EBISCode", 21 );
	ebis_code = config->GetValue( "EBIS.Code", tmp_val_uchar );
	tmp_val_uchar = config->GetValue( "T1Crate", 0 );
	vme_t1_crate = config->GetValue( "T1.Crate", tmp_val_uchar );
	tmp_val_uchar = config->GetValue( "T1Module", 1 );
	vme_t1_mod = config->GetValue( "T1.Module", tmp_val_uchar );
	tmp_val_uchar = config->GetValue( "T1Channel", 15 );
	vme_t1_ch = config->GetValue( "T1.Channel", tmp_val_uchar );
	t1_code = 22;
	tmp_val_uchar = config->GetValue( "SCCrate", 0 );
	vme_sc_crate = config->GetValue( "SC.Crate", tmp_val_uchar );
	tmp_val_uchar = config->GetValue( "SCModule", 1 );
	vme_sc_mod = config->GetValue( "SC.Module", tmp_val_uchar );
	tmp_val_uchar = config->GetValue( "SCChannel", 12 );
	vme_sc_ch = config->GetValue( "SC.Channel", tmp_val_uchar );
	sc_code = 23;
	tmp_val_uchar = config->GetValue( "LaserCrate", 0 );
	vme_laser_crate = config->GetValue( "Laser.Crate", tmp_val_uchar );
	tmp_val_uchar = config->GetValue( "LaserModule", 1 );
	vme_laser_mod = config->GetValue( "Laser.Module", tmp_val_uchar );
	tmp_val_uchar = config->GetValue( "LaserChannel", 11 );
	vme_laser_ch = config->GetValue( "Laser.Channel", tmp_val_uchar );
	laser_code = 24;


	// Event builder
	build_by_tm_stp = config->GetValue( "BuildByTimeStamp", true );
	event_window = config->GetValue( "EventWindow", 3e3 );
	recoil_hit_window = config->GetValue( "RecoilHitWindow", 500 );
	array_pn_hit_window = config->GetValue( "ArrayHitWindow.PN", 500 );
	array_pp_hit_window = config->GetValue( "ArrayHitWindow.PP", 500 );
	array_nn_hit_window = config->GetValue( "ArrayHitWindow.NN", 500 );
	if( array_pn_hit_window == 500 ) // backwards compatibility
		array_pn_hit_window = config->GetValue( "ArrayHitWindow", 500 );
	zd_hit_window = config->GetValue( "ZeroDegreeHitWindow", 500 );
	gamma_hit_window = config->GetValue( "GammaRayHitWindow", 500 );
	lume_hit_window = config->GetValue( "LumeHitWindow", 500 );
	cd_rs_hit_window = config->GetValue( "CDHitWindow.RS", 500 );
	cd_dd_hit_window = config->GetValue( "CDHitWindow.DD", 500 );


	// Data things
	block_size = config->GetValue( "DataBlockSize", 0x10000 );
	flag_asic_only = config->GetValue( "ASICOnlyData", false );
	flag_caen_only = config->GetValue( "CAENOnlyData", false );
	flag_mesy_only = config->GetValue( "MesytecOnlyData", false );


	// Event rejection
	clipped_reject = config->GetValue( "ClippedRejection", true );
	overflow_reject = config->GetValue( "OverflowRejection", true );


	// Recoil detector
	n_recoil_sector = config->GetValue( "NumberOfRecoilSectors", 4 );
	n_recoil_layer  = config->GetValue( "NumberOfRecoilLayers", 2 );
	recoil_eloss_start = config->GetValue( "RecoilEnergyLossStart", 0 );
	recoil_eloss_stop  = config->GetValue( "RecoilEnergyLossStop", 0 );
	recoil_erest_start = config->GetValue( "RecoilEnergyRestStart", 1 );
	recoil_erest_stop  = config->GetValue( "RecoilEnergyRestStop", 1 );
	recoil_etot_start  = config->GetValue( "RecoilEnergyTotalStart", 0 );
	recoil_etot_stop   = config->GetValue( "RecoilEnergyTotalStop", 1 );
	if( recoil_eloss_start >= n_recoil_layer ){
		std::cerr << "RecoilEnergyLossStart must be less than NumberOfRecoilLayers" << std::endl;
		std::cerr << "Reverting RecoilEnergyLossStart to default value of 0" << std::endl;
		recoil_eloss_start = 0;
	}
	if( recoil_eloss_stop >= n_recoil_layer ){
		std::cerr << "RecoilEnergyLossStop must be less than NumberOfRecoilLayers" << std::endl;
		std::cerr << "Reverting RecoilEnergyLossStop to default value of 0" << std::endl;
		recoil_eloss_stop = 0;
	}
	if( recoil_erest_start >= n_recoil_layer ){
		std::cerr << "RecoilEnergyRestStart must be less than NumberOfRecoilLayers" << std::endl;
		std::cerr << "Reverting RecoilEnergyRestStart to default value of 1" << std::endl;
		recoil_erest_start = 1;
	}
	if( recoil_erest_stop >= n_recoil_layer ){
		std::cerr << "RecoilEnergyRestStop must be less than NumberOfRecoilLayers" << std::endl;
		std::cerr << "Reverting RecoilEnergyRestStop to default value of 1" << std::endl;
		recoil_erest_stop = 1;
	}
	if( recoil_etot_start >= n_recoil_layer ){
		std::cerr << "RecoilEnergyTotalStart must be less than NumberOfRecoilLayers" << std::endl;
		std::cerr << "Reverting RecoilEnergyTotalStart to default value of 0" << std::endl;
		recoil_etot_start = 0;
	}
	if( recoil_etot_stop >= n_recoil_layer ){
		std::cerr << "RecoilEnergyTotalStop must be less than NumberOfRecoilLayers" << std::endl;
		std::cerr << "Reverting RecoilEnergyTotalStop to default value of 1" << std::endl;
		recoil_etot_stop = 1;
	}

	recoil_vme.resize( n_recoil_sector );
	recoil_mod.resize( n_recoil_sector );
	recoil_ch.resize( n_recoil_sector );
	recoil_sector.resize( GetNumberOfVmeCrates() );
	recoil_layer.resize( GetNumberOfVmeCrates() );

	for( unsigned int i = 0; i < GetNumberOfVmeCrates(); ++i ){

		recoil_sector[i].resize( GetMaximumNumberOfVmeModules() );
		recoil_layer[i].resize( GetMaximumNumberOfVmeModules() );

		for( unsigned int j = 0; j < GetMaximumNumberOfVmeModules(); ++j ){

			for( unsigned int k = 0; k < GetMaximumNumberOfVmeChannels(); ++k ){

				recoil_sector[i][j].push_back( -1 );
				recoil_layer[i][j].push_back( -1 );

			}

		}

	}

	for( unsigned int i = 0; i < n_recoil_sector; ++i ){

		recoil_vme[i].resize( n_recoil_layer );
		recoil_mod[i].resize( n_recoil_layer );
		recoil_ch[i].resize( n_recoil_layer );

		for( unsigned int j = 0; j < n_recoil_layer; ++j ){

			recoil_vme[i][j] = config->GetValue( Form( "Recoil_%d_%d.Crate", i, j ), 0 );
			recoil_mod[i][j] = config->GetValue( Form( "Recoil_%d_%d.Module", i, j ), 0 );
			recoil_ch[i][j] = config->GetValue( Form( "Recoil_%d_%d.Channel", i, j ), 2*(int)i+(int)j );

			if( recoil_vme[i][j] < GetNumberOfVmeCrates() &&
			    recoil_mod[i][j] < GetMaximumNumberOfVmeModules() &&
			    recoil_ch[i][j] < GetMaximumNumberOfVmeChannels() ) {

				recoil_sector[recoil_vme[i][j]][recoil_mod[i][j]][recoil_ch[i][j]] = i;
				recoil_layer[recoil_vme[i][j]][recoil_mod[i][j]][recoil_ch[i][j]]  = j;

			}

			else {

				std::cerr << "Dodgy recoil settings:";
				std::cerr << " crate = " << (int)recoil_vme[i][j];
				std::cerr << " module = " << (int)recoil_mod[i][j];
				std::cerr << " channel = " << (int)recoil_ch[i][j] << std::endl;

			}

		}

	}

	// MWPC
	n_mwpc_axes = config->GetValue( "NumberOfMWPCAxes", 2 ); // x and y usually

	mwpc_vme.resize( n_mwpc_axes );
	mwpc_mod.resize( n_mwpc_axes );
	mwpc_ch.resize( n_mwpc_axes );

	mwpc_axis.resize( GetNumberOfVmeCrates() );
	mwpc_tac.resize( GetNumberOfVmeCrates() );

	for( unsigned int i = 0; i < GetNumberOfVmeCrates(); ++i ){

		mwpc_axis[i].resize( GetMaximumNumberOfVmeModules() );
		mwpc_tac[i].resize( GetMaximumNumberOfVmeModules() );

		for( unsigned int j = 0; j < GetMaximumNumberOfVmeModules(); ++j ){

			for( unsigned int k = 0; k < GetMaximumNumberOfVmeChannels(); ++k ){

				mwpc_axis[i][j].push_back( -1 );
				mwpc_tac[i][j].push_back( -1 );

			}

		}

	}


	for( unsigned int i = 0; i < n_mwpc_axes; ++i ){

		mwpc_vme[i].resize( 2 );
		mwpc_mod[i].resize( 2 );
		mwpc_ch[i].resize( 2 );

		for( unsigned int j = 0; j < 2; ++j ){ // two TACs per axis

			mwpc_vme[i][j] = config->GetValue( Form( "MWPC_%d_%d.Crate", i, j ), 0 );
			mwpc_mod[i][j] = config->GetValue( Form( "MWPC_%d_%d.Module", i, j ), 1 );
			mwpc_ch[i][j] = config->GetValue( Form( "MWPC_%d_%d.Channel", i, j ), 8+(int)i*2+(int)j );

			if( mwpc_vme[i][j] < GetNumberOfVmeCrates() &&
			    mwpc_mod[i][j] < GetMaximumNumberOfVmeModules() &&
			     mwpc_ch[i][j] < GetMaximumNumberOfVmeChannels() ) {

				mwpc_axis[mwpc_vme[i][j]][mwpc_mod[i][j]][mwpc_ch[i][j]] = i;
				mwpc_tac[mwpc_vme[i][j]][mwpc_mod[i][j]][mwpc_ch[i][j]] = j;

			}

			else {

				std::cerr << "Dodgy MWPC settings:";
				std::cerr << " crate = " << (int)mwpc_vme[i][j];
				std::cerr << " module = " << (int)mwpc_mod[i][j];
				std::cerr << " channel = " << (int)mwpc_ch[i][j] << std::endl;

			}
		}

	}

	// ELUM detector
	n_elum_sector = config->GetValue( "NumberOfELUMSectors", 4 );

	elum_vme.resize( n_elum_sector );
	elum_mod.resize( n_elum_sector );
	elum_ch.resize( n_elum_sector );
	elum_sector.resize( GetNumberOfVmeCrates() );

	for( unsigned int i = 0; i < GetNumberOfVmeCrates(); ++i ) {

		elum_sector[i].resize( GetMaximumNumberOfVmeModules() );

		for( unsigned int j = 0; j < GetMaximumNumberOfVmeModules(); ++j )
			for( unsigned int k = 0; k < GetMaximumNumberOfVmeChannels(); ++k )
				elum_sector[i][j].push_back( -1 );

	}


	for( unsigned int i = 0; i < n_elum_sector; ++i ){

		elum_vme[i] = config->GetValue( Form( "ELUM_%d.Crate", i ), 0 );
		elum_mod[i] = config->GetValue( Form( "ELUM_%d.Module", i ), 1 );
		elum_ch[i] = config->GetValue( Form( "ELUM_%d.Channel", i ), (int)i );

		if( elum_vme[i] < GetNumberOfVmeCrates() &&
		    elum_mod[i] < GetMaximumNumberOfVmeModules() &&
		     elum_ch[i] < GetMaximumNumberOfVmeChannels() )
			elum_sector[elum_vme[i]][elum_mod[i]][elum_ch[i]] = i;

		else {

			std::cerr << "Dodgy ELUM settings:";
			std::cerr << " crate = " << (int)elum_vme[i];
			std::cerr << " module = " << (int)elum_mod[i];
			std::cerr << " channel = " << (int)elum_ch[i] << std::endl;

		}

	}

	// ZeroDegree detector
	n_zd_layer = config->GetValue( "NumberOfZDLayers", 2 );

	zd_vme.resize( n_zd_layer );
	zd_mod.resize( n_zd_layer );
	zd_ch.resize( n_zd_layer );
	zd_layer.resize( GetNumberOfVmeCrates() );

	for( unsigned int i = 0; i < GetNumberOfVmeCrates(); ++i ) {

		zd_layer[i].resize( GetMaximumNumberOfVmeModules() );

		for( unsigned int j = 0; j < GetMaximumNumberOfVmeModules(); ++j )
			for( unsigned int k = 0; k < GetMaximumNumberOfVmeChannels(); ++k )
				zd_layer[i][j].push_back( -1 );

	}

	for( unsigned int i = 0; i < n_zd_layer; ++i ){

		zd_vme[i] = config->GetValue( Form( "ZD_%d.Crate", i ), 0 );
		zd_mod[i] = config->GetValue( Form( "ZD_%d.Module", i ), 1 );
		zd_ch[i] = config->GetValue( Form( "ZD_%d.Channel", i ), (int)i+6 );

		if( zd_vme[i] < GetNumberOfVmeCrates() &&
		    zd_mod[i] < GetMaximumNumberOfVmeModules() &&
		     zd_ch[i] < GetMaximumNumberOfVmeChannels() )
			zd_layer[zd_vme[i]][zd_mod[i]][zd_ch[i]] = i;

		else {

			std::cerr << "Dodgy ZeroDegree settings:";
			std::cerr << " crate = " << (int)zd_vme[i];
			std::cerr << " module = " << (int)zd_mod[i];
			std::cerr << " channel = " << (int)zd_ch[i] << std::endl;

		}

	}

	// ScintArray
	n_scint_detector = config->GetValue( "NumberOfScintArrayDetectors", 0 );

	scint_vme.resize( n_scint_detector );
	scint_mod.resize( n_scint_detector );
	scint_ch.resize( n_scint_detector );
	scint_detector.resize( GetNumberOfVmeCrates() );

	for( unsigned int i = 0; i < GetNumberOfVmeCrates(); ++i ) {

		scint_detector[i].resize( GetMaximumNumberOfVmeModules() );

		for( unsigned int j = 0; j < GetMaximumNumberOfVmeModules(); ++j )
			for( unsigned int k = 0; k < GetMaximumNumberOfVmeChannels(); ++k )
				scint_detector[i][j].push_back( -1 );

	}

	for( unsigned int i = 0; i < n_scint_detector; ++i ){

		unsigned char v = 1;
		unsigned char m = 0;
		unsigned char c = i;
		if( i > 31 ){	// next board, channels start again at zero
			m++;
			c -= 32;
		}
		if( i > 63 ){	// next board, doesn't exist, last channels in board 0
			m++;
			c -= 32;
		}
		scint_vme[i] = config->GetValue( Form( "ScintArray_%d.Crate", i ), (int)v );
		scint_mod[i] = config->GetValue( Form( "ScintArray_%d.Module", i ), (int)m );
		scint_ch[i] = config->GetValue( Form( "ScintArray_%d.Channel", i ), (int)c );

		if( scint_vme[i] < GetNumberOfVmeCrates() &&
		    scint_mod[i] < GetMaximumNumberOfVmeModules() &&
		     scint_ch[i] < GetMaximumNumberOfVmeChannels() )
			scint_detector[scint_vme[i]][scint_mod[i]][scint_ch[i]] = i;

		else {

			std::cerr << "Dodgy ScintArray settings:";
			std::cerr << " crate = " << (int)scint_vme[i];
			std::cerr << " module = " << (int)scint_mod[i];
			std::cerr << " channel = " << (int)scint_ch[i] << std::endl;

		}

	}

	// LUME
	n_lume = config->GetValue( "NumberOfLUMEDetectors", 0 );
	lume_vme.resize( n_lume );
	lume_mod.resize( n_lume );
	lume_ch.resize( n_lume );
	lume_detector.resize( GetNumberOfVmeCrates() );
	lume_type.resize( GetNumberOfVmeCrates() );

	for( unsigned int i = 0; i < GetNumberOfVmeCrates(); ++i ) {

		lume_detector[i].resize( GetMaximumNumberOfVmeModules() );
		lume_type[i].resize( GetMaximumNumberOfVmeModules() );

		for( unsigned int j = 0; j < GetMaximumNumberOfVmeModules(); ++j ) {
			for( unsigned int k = 0; k < GetMaximumNumberOfVmeChannels(); ++k ){
				lume_detector[i][j].push_back( -1 );
				lume_type[i][j].push_back( -1 );
			}
		}

	}

	// Setup the LUME types
	lume_type_list.push_back( "be" ); // back energy
	lume_type_list.push_back( "ne" ); // near energy
	lume_type_list.push_back( "fe" ); // far energy

	// Loop over all LUME detectors
	for( unsigned int i = 0; i < n_lume; ++i ){

		lume_vme[i].resize(lume_type_list.size());
		lume_mod[i].resize(lume_type_list.size());
		lume_ch[i].resize(lume_type_list.size());

		// And there are three types of signals for each
		for( unsigned int j = 0; j < lume_type_list.size(); ++j ){

			// Assuming they are all in the same Mesytec module
			unsigned char mm = 5;
			unsigned char cc = i * lume_type_list.size() + j;

			lume_vme[i][j] = config->GetValue( Form( "LUME_%d_%s.Crate", i, lume_type_list[j].data() ), (int) 1 );
			lume_mod[i][j] = config->GetValue( Form( "LUME_%d_%s.Module", i, lume_type_list[j].data() ), (int) mm );
			lume_ch[i][j] = config->GetValue( Form( "LUME_%d_%s.Channel", i, lume_type_list[j].data() ), (int) cc );

			if( lume_vme[i][j] < GetNumberOfVmeCrates() &&
			    lume_mod[i][j] < GetMaximumNumberOfVmeModules() &&
			     lume_ch[i][j] < GetMaximumNumberOfVmeChannels() ) {

				lume_detector[lume_vme[i][j]][lume_mod[i][j]][lume_ch[i][j]] = i;
				lume_type[lume_vme[i][j]][lume_mod[i][j]][lume_ch[i][j]] = j;

			}

			else {

				std::cerr << "Dodgy LUME settings:";
				std::cerr << " crate = " << (int)lume_vme[i][j];
				std::cerr << " module = " << (int)lume_mod[i][j];
				std::cerr << " channel = " << (int)lume_ch[i][j] << std::endl;

			}

		}

	}

	// CD
	n_cd_ring   = config->GetValue( "NumberOfCDRings", 16 );
	n_cd_sector = config->GetValue( "NumberOfCDSectors", 8 );
	n_cd_layer  = config->GetValue( "NumberOfCDLayers", 0 );
	n_cd_side   = 2;
	cd_eloss_start = config->GetValue( "CDEnergyLossStart", 0 );
	cd_eloss_stop  = config->GetValue( "CDEnergyLossStop", 0 );
	cd_erest_start = config->GetValue( "CDEnergyRestStart", 1 );
	cd_erest_stop  = config->GetValue( "CDEnergyRestStop", 1 );
	cd_etot_start  = config->GetValue( "CDEnergyTotalStart", 0 );
	cd_etot_stop   = config->GetValue( "CDEnergyTotalStop", 1 );
	if( cd_eloss_start >= n_cd_layer && n_cd_layer != 0 ){
		std::cerr << "CDEnergyLossStart must be less than NumberOfRecoilLayers" << std::endl;
		std::cerr << "Reverting CDEnergyLossStart to default value of 0" << std::endl;
		cd_eloss_start = 0;
	}
	if( cd_eloss_stop >= n_cd_layer && n_cd_layer != 0 ){
		std::cerr << "CDEnergyLossStop must be less than NumberOfRecoilLayers" << std::endl;
		std::cerr << "Reverting CDEnergyLossStop to default value of 0" << std::endl;
		cd_eloss_stop = 0;
	}
	if( cd_erest_start >= n_cd_layer && n_cd_layer != 0 ){
		std::cerr << "CDEnergyRestStart must be less than NumberOfRecoilLayers" << std::endl;
		std::cerr << "Reverting CDEnergyRestStart to default value of 1" << std::endl;
		cd_erest_start = 1;
	}
	if( cd_erest_stop >= n_cd_layer && n_cd_layer != 0 ){
		std::cerr << "CDEnergyRestStop must be less than NumberOfRecoilLayers" << std::endl;
		std::cerr << "Reverting CDEnergyRestStop to default value of 1" << std::endl;
		cd_erest_stop = 1;
	}
	if( cd_etot_start >= n_cd_layer && n_cd_layer != 0 ){
		std::cerr << "CDEnergyTotalStart must be less than NumberOfRecoilLayers" << std::endl;
		std::cerr << "Reverting CDEnergyTotalStart to default value of 0" << std::endl;
		cd_etot_start = 0;
	}
	if( cd_etot_stop >= n_cd_layer && n_cd_layer != 0 ){
		std::cerr << "CDEnergyTotalStop must be less than NumberOfRecoilLayers" << std::endl;
		std::cerr << "Reverting CDEnergyTotalStop to default value of 1" << std::endl;
		cd_etot_stop = 1;
	}
	cd_vme.resize( n_cd_layer );
	cd_mod.resize( n_cd_layer );
	cd_ch.resize( n_cd_layer );
	cd_layer.resize( GetNumberOfVmeCrates() );
	cd_strip.resize( GetNumberOfVmeCrates() );
	cd_side.resize( GetNumberOfVmeCrates() );

	for( unsigned int i = 0; i < GetNumberOfVmeCrates(); ++i ){

		cd_layer[i].resize( GetMaximumNumberOfVmeModules() );
		cd_strip[i].resize( GetMaximumNumberOfVmeModules() );
		cd_side[i].resize( GetMaximumNumberOfVmeModules() );

		for( unsigned int j = 0; j < GetMaximumNumberOfVmeModules(); ++j ){

			for( unsigned int k = 0; k < GetMaximumNumberOfVmeChannels(); ++k ){

				cd_layer[i][j].push_back( -1 );
				cd_strip[i][j].push_back( -1 );
				cd_side[i][j].push_back( -1 );

			}

		}

	}

	for( unsigned int i = 0; i < n_cd_layer; ++i ){

		cd_vme[i].resize( n_cd_side );
		cd_mod[i].resize( n_cd_side );
		cd_ch[i].resize( n_cd_side );

		for( unsigned int j = 0; j < n_cd_side; ++j ){

			std::string sidechar = "R";
			unsigned int nstrips = n_cd_ring;
			if( j == 1 ) {
				sidechar = "S";
				nstrips = n_cd_sector;
			}

			cd_vme[i][j].resize( nstrips );
			cd_mod[i][j].resize( nstrips );
			cd_ch[i][j].resize( nstrips );

			for( unsigned int k = 0; k < nstrips; ++k ){

				unsigned char mm = 2*i + j + 2;
				unsigned char cc = k;
				if( k > GetMaximumNumberOfVmeChannels() ) {
					mm++;
					cc -= GetMaximumNumberOfVmeChannels();
				}
				cd_vme[i][j][k] = (int)config->GetValue( Form( "CD_%d_%d.%s.Crate", i, k, sidechar.data() ), (int)1 ); // layer, ring
				cd_mod[i][j][k] = (int)config->GetValue( Form( "CD_%d_%d.%s.Module", i, k, sidechar.data() ), (int)mm );
				cd_ch[i][j][k] = (int)config->GetValue( Form( "CD_%d_%d.%s.Channel", i, k, sidechar.data() ), (int)cc );

				if( cd_vme[i][j][k] < GetNumberOfVmeCrates() &&
				   cd_mod[i][j][k] < GetMaximumNumberOfVmeModules() &&
				   cd_ch[i][j][k] < GetMaximumNumberOfVmeChannels() ) {

					cd_strip[cd_vme[i][j][k]][cd_mod[i][j][k]][cd_ch[i][j][k]] = k;
					cd_side[cd_vme[i][j][k]][cd_mod[i][j][k]][cd_ch[i][j][k]]  = j;
					cd_layer[cd_vme[i][j][k]][cd_mod[i][j][k]][cd_ch[i][j][k]] = i;

				}

				else {

					std::cerr << "Dodgy CD settings:";
					std::cerr << " crate = " << (int)cd_vme[i][j][k];
					std::cerr << " module = " << (int)cd_mod[i][j][k];
					std::cerr << " channel = " << (int)cd_ch[i][j][k] << std::endl;

				}

			}

		}

	}


	// Perform a couple of sanity checks
	if( n_caen_mod > 16 ) {

		n_caen_mod = 16;
		std::cout << "Maximum number of CAEN modules is 16" << std::endl;

	}

	if( n_mesy_mod > 16 ) {

		n_mesy_mod = 16;
		std::cout << "Maximum number of Mesytec modules is 16" << std::endl;

	}

	// Finished
	delete config;

}


bool ISSSettings::IsRecoil( unsigned char vme, unsigned char mod, unsigned char ch ) {

	/// Return true if this is a recoil event
	if( recoil_sector[(int)vme][(int)mod][(int)ch] >= 0 ) return true;
	else return false;

}

char ISSSettings::GetRecoilSector( unsigned char vme, unsigned char mod, unsigned char ch ) {

	/// Return the sector or quadrant of a recoil event by module and channel number
	if( vme < GetNumberOfVmeCrates() &&
	    mod < GetMaximumNumberOfVmeModules() &&
	     ch < GetMaximumNumberOfVmeChannels() )
		return recoil_sector[(int)vme][(int)mod][(int)ch];

	else {

		std::cerr << "Bad recoil event:";
		std::cerr << " crate = " << (int)vme;
		std::cerr << " module = " << (int)mod;
		std::cerr << " channel = " << (int)ch << std::endl;
		return -1;

	}

}

char ISSSettings::GetRecoilLayer( unsigned char vme, unsigned char mod, unsigned char ch ) {

	/// Return the sector or quadrant of a recoil event by module and channel number
	if( vme < GetNumberOfVmeCrates() &&
	    mod < GetMaximumNumberOfVmeModules() &&
	     ch < GetMaximumNumberOfVmeChannels() )
		return recoil_layer[(int)vme][(int)mod][(int)ch];

	else {

		std::cerr << "Bad recoil event:";
		std::cerr << " crate = " << (int)vme;
		std::cerr << " module = " << (int)mod;
		std::cerr << " channel = " << (int)ch << std::endl;
		return -1;

	}

}

char ISSSettings::GetRecoilCrate( unsigned char sec, unsigned char layer ){

	// Returns the module of the recoil detector
	if( sec < n_recoil_sector && layer < n_recoil_layer )
		return recoil_vme[(int)sec][(int)layer];

	else {

		std::cerr << "Bad recoil event: sector = " << (int)sec;
		std::cerr << " layer = " << (int)layer << std::endl;
		return -1;

	}


}

char ISSSettings::GetRecoilModule( unsigned char sec, unsigned char layer ){

	// Returns the module of the recoil detector
	if( sec < n_recoil_sector && layer < n_recoil_layer )
		return recoil_mod[(int)sec][(int)layer];

	else {

		std::cerr << "Bad recoil event: sector = " << (int)sec;
		std::cerr << " layer = " << (int)layer << std::endl;
		return -1;

	}


}

char ISSSettings::GetRecoilChannel( unsigned char sec, unsigned char layer ){

	// Returns the channel of the recoil detector
	if( sec < n_recoil_sector && layer < n_recoil_layer )
		return recoil_ch[(int)sec][(int)layer];

	else {

		std::cerr << "Bad recoil event: sector = " << (int)sec;
		std::cerr << " layer = " << (int)layer << std::endl;
		return -1;

	}


}

bool ISSSettings::IsMWPC( unsigned char vme, unsigned char mod, unsigned char ch ) {

	/// Return true if this is a MWPC event
	if( mwpc_axis[(int)vme][(int)mod][(int)ch] >= 0 && mwpc_axis[(int)vme][(int)mod][(int)ch] < (int)n_mwpc_axes ) return true;
	else return false;

}

char ISSSettings::GetMWPCAxis( unsigned char vme, unsigned char mod, unsigned char ch ) {

	/// Return the axis number of an MWPC event by module and channel number
	if( vme < GetNumberOfVmeCrates() &&
		mod < GetMaximumNumberOfVmeModules() &&
	     ch < GetMaximumNumberOfVmeChannels() )
		return mwpc_axis[(int)vme][(int)mod][(int)ch];

	else {

		std::cerr << "Bad MWPC event:";
		std::cerr << " crate = " << (int)vme;
		std::cerr << " module = " << (int)mod;
		std::cerr << " channel = " << (int)ch << std::endl;
		return -1;

	}

}

char ISSSettings::GetMWPCID( unsigned char vme, unsigned char mod, unsigned char ch ) {

	/// Return the TAC number of an MWPC event by module and channel number
	if( vme < GetNumberOfVmeCrates() &&
	    mod < GetMaximumNumberOfVmeModules() &&
	     ch < GetMaximumNumberOfVmeChannels() )
		return mwpc_tac[(int)vme][(int)mod][(int)ch];

	else {

		std::cerr << "Bad MWPC event:";
		std::cerr << " crate = " << (int)vme;
		std::cerr << " module = " << (int)mod;
		std::cerr << " channel = " << (int)ch << std::endl;
		return -1;

	}

}

bool ISSSettings::IsELUM( unsigned char vme, unsigned char mod, unsigned char ch ) {

	/// Return true if this is an ELUM event
	if( elum_sector[(int)vme][(int)mod][(int)ch] >= 0 ) return true;
	else return false;

}


char ISSSettings::GetELUMSector( unsigned char vme, unsigned char mod, unsigned char ch ) {

	/// Return the sector or quadrant of a ELUM event by module and channel number
	if( vme < GetNumberOfVmeCrates() &&
	    mod < GetMaximumNumberOfVmeModules() &&
	     ch < GetMaximumNumberOfVmeChannels() )
		return elum_sector[(int)vme][(int)mod][(int)ch];

	else {

		std::cerr << "Bad ELUM event:";
		std::cerr << " crate = " << (int)vme;
		std::cerr << " module = " << (int)mod;
		std::cerr << " channel = " << (int)ch << std::endl;
		return -1;

	}

}

bool ISSSettings::IsZD( unsigned char vme, unsigned char mod, unsigned char ch ) {

	/// Return true if this is an ZeroDegree event
	if( zd_layer[(int)vme][(int)mod][(int)ch] >= 0 ) return true;
	else return false;

}


char ISSSettings::GetZDLayer( unsigned char vme, unsigned char mod, unsigned char ch ) {

	/// Return the layer of a ZeroDegree event by module and channel number
	if( vme < GetNumberOfVmeCrates() &&
	    mod < GetMaximumNumberOfVmeModules() &&
	     ch < GetMaximumNumberOfVmeChannels() )
		return zd_layer[(int)vme][(int)mod][(int)ch];

	else {

		std::cerr << "Bad ZeroDegree event:";
		std::cerr << " crate = " << (int)vme;
		std::cerr << " module = " << (int)mod;
		std::cerr << " channel = " << (int)ch << std::endl;
		return -1;

	}

}


bool ISSSettings::IsScintArray( unsigned char vme, unsigned char mod, unsigned char ch ) {

	/// Return true if this is an ScintArray event
	if( scint_detector[(int)vme][(int)mod][(int)ch] >= 0 ) return true;
	else return false;

}


char ISSSettings::GetScintArrayDetector( unsigned char vme, unsigned char mod, unsigned char ch ) {

	/// Return the detector ID of a ScintArray event by module and channel number
	if( vme < GetNumberOfVmeCrates() &&
	    mod < GetMaximumNumberOfVmeModules() &&
	     ch < GetMaximumNumberOfVmeChannels() )
		return scint_detector[(int)vme][(int)mod][(int)ch];

	else {

		std::cerr << "Bad ScintArray event:";
		std::cerr << " crate = " << (int)vme;
		std::cerr << " module = " << (int)mod;
		std::cerr << " channel = " << (int)ch << std::endl;
		return -1;

	}

}

bool ISSSettings::IsLUME( unsigned char vme, unsigned char mod, unsigned char ch ) {

	/// Return true if this is an ScintArray event
	if( lume_detector[(int)vme][(int)mod][(int)ch] >= 0 ) return true;
	else return false;

}

char ISSSettings::GetLUMEType( unsigned char vme, unsigned char mod, unsigned char ch ) {

	/// Return the type of LUME detector signal by module and channel number
	if( vme < GetNumberOfVmeCrates() &&
	    mod < GetMaximumNumberOfVmeModules() &&
	     ch < GetMaximumNumberOfVmeChannels() )
		return lume_type[(int)vme][(int)mod][(int)ch];

	else {

		std::cerr << "Bad LUME event: ";
		std::cerr << " crate = " << (int)vme;
		std::cerr << " module = " << (int)mod;
		std::cerr << " channel = " << (int)ch << std::endl;
		return -1;

	}

}

char ISSSettings::GetLUMEDetector( unsigned char vme, unsigned char mod, unsigned char ch ) {

	/// Return ID of LUME t detector (total energy signal)
	if( vme < GetNumberOfVmeCrates() &&
	    mod < GetMaximumNumberOfVmeModules() &&
	     ch < GetMaximumNumberOfVmeChannels() )
		return lume_detector[(int)vme][(int)mod][(int)ch];

	else {

		std::cerr << "Bad LUME event: ";
		std::cerr << " crate = " << (int)vme;
		std::cerr << " module = " << (int)mod;
		std::cerr << " channel = " << (int)ch << std::endl;
		return -1;

	}

}

char ISSSettings::GetCDLayer( unsigned char vme, unsigned char mod, unsigned char ch ) {

	/// Return the layer of a CD event by module and channel number
	if( vme < GetNumberOfVmeCrates() &&
	   mod < GetMaximumNumberOfVmeModules() &&
	   ch < GetMaximumNumberOfVmeChannels() )
		return cd_layer[(int)vme][(int)mod][(int)ch];

	else {

		std::cerr << "Bad CD event:";
		std::cerr << " crate = " << (int)vme;
		std::cerr << " module = " << (int)mod;
		std::cerr << " channel = " << (int)ch << std::endl;
		return -1;

	}

}

char ISSSettings::GetCDSector( unsigned char vme, unsigned char mod, unsigned char ch ) {

	/// Return the layer of a CD event by module and channel number
	if( vme < GetNumberOfVmeCrates() &&
	   mod < GetMaximumNumberOfVmeModules() &&
	   ch < GetMaximumNumberOfVmeChannels() ) {

		if( cd_side[(int)vme][(int)mod][(int)ch] == 1 ) // sectors in side 1
			return cd_strip[(int)vme][(int)mod][(int)ch];

		else
			return -1;

	}

	else {

		std::cerr << "Bad CD event:";
		std::cerr << " crate = " << (int)vme;
		std::cerr << " module = " << (int)mod;
		std::cerr << " channel = " << (int)ch << std::endl;
		return -1;

	}

}

char ISSSettings::GetCDRing( unsigned char vme, unsigned char mod, unsigned char ch ) {

	/// Return the layer of a CD event by module and channel number
	if( vme < GetNumberOfVmeCrates() &&
	   mod < GetMaximumNumberOfVmeModules() &&
	   ch < GetMaximumNumberOfVmeChannels() ) {

		if( cd_side[(int)vme][(int)mod][(int)ch] == 0 ) // rings in side 0
			return cd_strip[(int)vme][(int)mod][(int)ch];

		else
			return -1;

	}

	else {

		std::cerr << "Bad CD event:";
		std::cerr << " crate = " << (int)vme;
		std::cerr << " module = " << (int)mod;
		std::cerr << " channel = " << (int)ch << std::endl;
		return -1;

	}

}

bool ISSSettings::IsCD( unsigned char vme, unsigned char mod, unsigned char ch ) {

	/// Return true if this is a CD event
	if( cd_strip[(int)vme][(int)mod][(int)ch] >= 0 ||
	    cd_layer[(int)vme][(int)mod][(int)ch] >= 0 ) return true;
	else return false;

}

void ISSSettings::PrintSettings() {

	std::cout << "=== Settings begin ===" << std::endl;

#define PRINT_SETTING(x) do {					\
		std::cout << std::setw(25) << std::left << #x ":" <<	\
		" " << (x) << std::endl;				\
	} while (0)

#define PRINT_SETTING_INT(x) do {				\
		std::cout << std::setw(25) << std::left << #x ":" <<	\
			" " << ((int) (x)) << std::endl;			\
	} while (0)

#define PRINT_SETTING_VECT_INT(x) do {				\
		for (size_t i = 0; i < x.size(); i++) {		\
			std::cout << std::setw(25) << std::left <<	\
			Form("%s[%d]:",#x, (int) i) <<			\
			" " << ((int) (x[i])) << std::endl;		\
		}							\
	} while (0)

#define PRINT_SETTING_VECT_VECT_INT(x) do {			\
		for (size_t i = 0; i < x.size(); i++) {		\
			for (size_t j = 0; j < x[i].size(); j++) {	\
				std::cout << std::setw(25) << std::left <<	\
				Form("%s[%d][%d]:",#x, (int) i, (int) j) <<	\
				" " << ((int) (x[i][j])) << std::endl;	\
			}							\
		}							\
	} while (0)

#define PRINT_SETTING_VECT_VECT_VECT_INT(x) do {			\
		for (size_t i = 0; i < x.size(); i++) {		\
			for (size_t j = 0; j < x[i].size(); j++) {	\
				for (size_t k = 0; k < x[i][j].size(); k++) {	\
					std::cout << std::setw(25) << std::left <<	\
					Form("%s[%d][%d][%d]:",#x, (int) i, (int) j, (int) k) <<	\
					" " << ((int) (x[i][j][k])) << std::endl;	\
				}							\
			}							\
		}							\
	} while (0)

	// Array settings
	PRINT_SETTING_INT(n_array_mod);
	PRINT_SETTING_INT(n_array_asic);
	PRINT_SETTING_INT(n_array_ch);

	// Array geometry
	PRINT_SETTING_INT(n_array_row);
	PRINT_SETTING_INT(n_array_pstrip);
	PRINT_SETTING_INT(n_array_nstrip);

	// CAEN settings
	PRINT_SETTING_INT(n_caen_mod);
	PRINT_SETTING_INT(n_caen_ch);
	PRINT_SETTING_VECT_INT(caen_model);

	// Mesytec settings
	PRINT_SETTING_INT(n_mesy_mod);
	PRINT_SETTING_INT(n_mesy_ch);
	PRINT_SETTING_INT(n_mesy_logic);

	// Info code settings
	PRINT_SETTING_INT(extt_code);
	PRINT_SETTING_INT(sync_code);
	PRINT_SETTING_INT(ext_item_code);
	PRINT_SETTING_INT(thsb_code);
	PRINT_SETTING_INT(pause_code);
	PRINT_SETTING_INT(resume_code);
	PRINT_SETTING_INT(asic_pulser_asic_0);
	PRINT_SETTING_INT(asic_pulser_ch_0);
	PRINT_SETTING_INT(asic_pulser_code_0);
	PRINT_SETTING_INT(asic_pulser_asic_1);
	PRINT_SETTING_INT(asic_pulser_ch_1);
	PRINT_SETTING_INT(asic_pulser_code_1);
	PRINT_SETTING_INT(asic_pulser_thres);
	PRINT_SETTING_INT(caen_pulser_mod);
	PRINT_SETTING_INT(caen_pulser_ch);
	PRINT_SETTING_INT(caen_pulser_code);
	PRINT_SETTING_INT(vme_ebis_crate);
	PRINT_SETTING_INT(vme_ebis_mod);
	PRINT_SETTING_INT(vme_ebis_ch);
	PRINT_SETTING_INT(ebis_code);
	PRINT_SETTING_INT(vme_t1_crate);
	PRINT_SETTING_INT(vme_t1_mod);
	PRINT_SETTING_INT(vme_t1_ch);
	PRINT_SETTING_INT(t1_code);
	PRINT_SETTING_INT(vme_sc_crate);
	PRINT_SETTING_INT(vme_sc_mod);
	PRINT_SETTING_INT(vme_sc_ch);
	PRINT_SETTING_INT(sc_code);
	PRINT_SETTING_INT(vme_laser_crate);
	PRINT_SETTING_INT(vme_laser_mod);
	PRINT_SETTING_INT(vme_laser_ch);
	PRINT_SETTING_INT(laser_code);

	// Event builder
	PRINT_SETTING(event_window);
	PRINT_SETTING(recoil_hit_window);
	PRINT_SETTING(array_pn_hit_window);
	PRINT_SETTING(array_pp_hit_window);
	PRINT_SETTING(array_nn_hit_window);
	PRINT_SETTING(zd_hit_window);
	PRINT_SETTING(gamma_hit_window);
	PRINT_SETTING(lume_hit_window);

	// Data format
	PRINT_SETTING_INT(block_size);
	PRINT_SETTING_INT(flag_caen_only);
	PRINT_SETTING_INT(flag_mesy_only);
	PRINT_SETTING_INT(flag_asic_only);

	// Recoil detectors
	PRINT_SETTING_INT(n_recoil_sector);
	PRINT_SETTING_INT(n_recoil_layer);
	PRINT_SETTING_INT(recoil_eloss_start);
	PRINT_SETTING_INT(recoil_eloss_stop);
	PRINT_SETTING_INT(recoil_erest_start);
	PRINT_SETTING_INT(recoil_erest_stop);
	PRINT_SETTING_INT(recoil_etot_start);
	PRINT_SETTING_INT(recoil_etot_stop);
	PRINT_SETTING_VECT_VECT_INT(recoil_vme);
	PRINT_SETTING_VECT_VECT_INT(recoil_mod);
	PRINT_SETTING_VECT_VECT_INT(recoil_ch);
	PRINT_SETTING_VECT_VECT_VECT_INT(recoil_sector);
	PRINT_SETTING_VECT_VECT_VECT_INT(recoil_layer);

	// MWPC
	PRINT_SETTING_INT(n_mwpc_axes);
	PRINT_SETTING_VECT_VECT_INT(mwpc_vme);
	PRINT_SETTING_VECT_VECT_INT(mwpc_mod);
	PRINT_SETTING_VECT_VECT_INT(mwpc_ch);
	PRINT_SETTING_VECT_VECT_VECT_INT(mwpc_axis);
	PRINT_SETTING_VECT_VECT_VECT_INT(mwpc_tac);

	// ELUM detector
	PRINT_SETTING_INT(n_elum_sector);
	PRINT_SETTING_VECT_INT(elum_vme);
	PRINT_SETTING_VECT_INT(elum_mod);
	PRINT_SETTING_VECT_INT(elum_ch);
	PRINT_SETTING_VECT_VECT_VECT_INT(elum_sector);

	// ZeroDegree detector
	PRINT_SETTING_INT(n_zd_layer);
	PRINT_SETTING_VECT_INT(zd_vme);
	PRINT_SETTING_VECT_INT(zd_mod);
	PRINT_SETTING_VECT_INT(zd_ch);
	PRINT_SETTING_VECT_VECT_VECT_INT(zd_layer);

	// ELUM detector
	PRINT_SETTING_INT(n_scint_detector);
	PRINT_SETTING_VECT_INT(scint_vme);
	PRINT_SETTING_VECT_INT(scint_mod);
	PRINT_SETTING_VECT_INT(scint_ch);
	PRINT_SETTING_VECT_VECT_VECT_INT(scint_detector);

	// LUME detector
	PRINT_SETTING_INT(n_lume);
	PRINT_SETTING_VECT_VECT_INT(lume_vme);
	PRINT_SETTING_VECT_VECT_INT(lume_mod);
	PRINT_SETTING_VECT_VECT_INT(lume_ch);
	PRINT_SETTING_VECT_VECT_VECT_INT(lume_detector);
	PRINT_SETTING_VECT_VECT_VECT_INT(lume_type);

	// CD detector
	PRINT_SETTING_INT(n_cd_layer);
	PRINT_SETTING_INT(n_cd_sector);
	PRINT_SETTING_INT(n_cd_ring);
	PRINT_SETTING_INT(cd_eloss_start);
	PRINT_SETTING_INT(cd_eloss_stop);
	PRINT_SETTING_INT(cd_erest_start);
	PRINT_SETTING_INT(cd_erest_stop);
	PRINT_SETTING_INT(cd_etot_start);
	PRINT_SETTING_INT(cd_etot_stop);
	PRINT_SETTING_VECT_VECT_VECT_INT(cd_vme);
	PRINT_SETTING_VECT_VECT_VECT_INT(cd_mod);
	PRINT_SETTING_VECT_VECT_VECT_INT(cd_ch);
	PRINT_SETTING_VECT_VECT_VECT_INT(cd_layer);
	PRINT_SETTING_VECT_VECT_VECT_INT(cd_side);
	PRINT_SETTING_VECT_VECT_VECT_INT(cd_strip);


	std::cout << "==== Settings end ====" << std::endl;
}
