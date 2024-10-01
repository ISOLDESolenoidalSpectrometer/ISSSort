#include "Settings.hh"

#include <iomanip>

ISSSettings::ISSSettings( std::string filename ) {
	
	// Setup the LUME types
	lume_type_list.push_back( "be" ); // back energy
	lume_type_list.push_back( "ne" ); // near energy
	lume_type_list.push_back( "fe" ); // far energy

	// Set filename
	SetFile( filename );
	
	// Go read the settings
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
	for( unsigned int i = 0; i < n_caen_mod; ++i )
		caen_model.push_back( config->GetValue( Form( "CAEN_%d.Model", i ), 1725 ) );
	
	// Mesytec initialisation
	n_mesy_mod = config->GetValue( "NumberOfMesytecModules", 0 );
	n_mesy_ch = config->GetValue( "NumberOfMesytecChannels", 16 );
	
	
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
	
	
	// Recoil detector
	n_recoil_sector = config->GetValue( "NumberOfRecoilSectors", 4 );
	n_recoil_layer  = config->GetValue( "NumberOfRecoilLayers", 2 );
	recoil_eloss_start = config->GetValue( "RecoilEnergyLossStart", 0 );
	recoil_eloss_stop  = config->GetValue( "RecoilEnergyLossStop", 0 );
	recoil_erest_start = config->GetValue( "RecoilEnergyRestStart", 1 );
	recoil_erest_stop  = config->GetValue( "RecoilEnergyRestStop", 1 );
	recoil_etot_start  = config->GetValue( "RecoilEnergyTotalStart", 0 );
	recoil_etot_stop   = config->GetValue( "RecoilEnergyTotalStop", 1 );
	
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
				std::cerr << " crate = " << recoil_vme[i][j];
				std::cerr << " module = " << recoil_mod[i][j];
				std::cerr << " channel = " << recoil_ch[i][j] << std::endl;
				
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
				std::cerr << " crate = " << mwpc_vme[i][j];
				std::cerr << " module = " << mwpc_mod[i][j];
				std::cerr << " channel = " << mwpc_ch[i][j] << std::endl;
				
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
			std::cerr << " crate = " << elum_vme[i];
			std::cerr << " module = " << elum_mod[i];
			std::cerr << " channel = " << elum_ch[i] << std::endl;
			
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
			std::cerr << " crate = " << zd_vme[i];
			std::cerr << " module = " << zd_mod[i];
			std::cerr << " channel = " << zd_ch[i] << std::endl;
			
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
			std::cerr << " crate = " << scint_vme[i];
			std::cerr << " module = " << scint_mod[i];
			std::cerr << " channel = " << scint_ch[i] << std::endl;
			
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
	
	// Loop over all LUME detectors
	for( unsigned int i = 0; i < n_lume; ++i ){

		lume_vme[i].resize(lume_type_list.size());
		lume_mod[i].resize(lume_type_list.size());
		lume_ch[i].resize(lume_type_list.size());

		// And there are three types of signals for each
		for( unsigned int j = 0; j < lume_type_list.size(); ++j ){
			
			// Assuming they are all in the same Mesytec module
			unsigned char mm = 2;
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
				std::cerr << " crate = " << lume_vme[i][j];
				std::cerr << " module = " << lume_mod[i][j];
				std::cerr << " channel = " << lume_ch[i][j] << std::endl;
				
			}
			
		}
		
	}

	// CD
	n_cd_sector = config->GetValue( "NumberOfCDSectors", 4 );
	n_cd_layer  = config->GetValue( "NumberOfCDLayers", 2 );
	n_cd_ring  = config->GetValue( "NumberOfCDRings", 2 );
	cd_eloss_start = config->GetValue( "RecoilEnergyLossStart", 0 );
	cd_eloss_stop  = config->GetValue( "RecoilEnergyLossStop", 0 );
	cd_erest_start = config->GetValue( "RecoilEnergyRestStart", 1 );
	cd_erest_stop  = config->GetValue( "RecoilEnergyRestStop", 1 );
	cd_etot_start  = config->GetValue( "RecoilEnergyTotalStart", 0 );
	cd_etot_stop   = config->GetValue( "RecoilEnergyTotalStop", 1 );

	cd_sector_vme.resize( n_cd_layer );
	cd_ring_vme.resize( n_cd_layer );
	cd_sector_mod.resize( n_cd_layer );
	cd_ring_mod.resize( n_cd_layer );
	cd_sector_ch.resize( n_recoil_layer );
	cd_ring_ch.resize( n_recoil_layer );
	cd_sector.resize( GetNumberOfVmeCrates() );
	cd_layer.resize( GetNumberOfVmeCrates() );
	cd_ring.resize( GetNumberOfVmeCrates() );
		
	for( unsigned int i = 0; i < GetNumberOfVmeCrates(); ++i ){
		
	        cd_sector[i].resize( GetMaximumNumberOfVmeModules() );
		cd_layer[i].resize( GetMaximumNumberOfVmeModules() );
		cd_ring[i].resize( GetMaximumNumberOfVmeModules() );
		
		for( unsigned int j = 0; j < GetMaximumNumberOfVmeModules(); ++j ){
			
			for( unsigned int k = 0; k < GetMaximumNumberOfVmeChannels(); ++k ){
				
				cd_sector[i][j].push_back( -1 );
				cd_layer[i][j].push_back( -1 );
				cd_ring[i][j].push_back( -1 );
				
			}
			
		}
		
	}

	for( unsigned int i = 0; i < n_cd_layer; ++i ){
		
	        cd_ring_vme[i].resize( n_cd_ring );
		cd_sector_vme[i].resize( n_cd_sector );
	        cd_ring_mod[i].resize( n_cd_ring );
		cd_sector_mod[i].resize( n_cd_sector );
		cd_ring_ch[i].resize( n_cd_ring );
		cd_sector_ch[i].resize( n_cd_sector );
		
		for( unsigned int j = 0; j < n_cd_ring; ++j ){
			
			cd_ring_vme[i][j] = config->GetValue( Form( "CD_Ring_%d_%d.Crate", j, i ), 0 );
			cd_ring_mod[i][j] = config->GetValue( Form( "CD_Ring_%d_%d.Module", j, i ), 0 );
			cd_ring_ch[i][j] = config->GetValue( Form( "CD_Ring_%d_%d.Channel", j, i ), 2*(int)j+(int)i );
			
			if( cd_ring_vme[i][j] < GetNumberOfVmeCrates() &&
			    cd_ring_mod[i][j] < GetMaximumNumberOfVmeModules() &&
			    cd_ring_ch[i][j] < GetMaximumNumberOfVmeChannels() ) {

			        cd_ring[cd_ring_vme[i][j]][cd_ring_mod[i][j]][cd_ring_ch[i][j]] = j;
			        cd_layer[cd_ring_vme[i][j]][cd_ring_mod[i][j]][cd_ring_ch[i][j]]  = i;
				
			}
			
			else {
				
				std::cerr << "Dodgy CD settings:";
				std::cerr << " crate = " << cd_ring_vme[i][j];
				std::cerr << " module = " << cd_ring_mod[i][j];
				std::cerr << " channel = " << cd_ring_ch[i][j] << std::endl;
				
			}
			
		}

		for( unsigned int j = 0; j < n_cd_sector; ++j ){
			
			cd_sector_vme[i][j] = config->GetValue( Form( "CD_Sector_%d_%d.Crate", j, i ), 0 );
			cd_sector_mod[i][j] = config->GetValue( Form( "CD_Sector_%d_%d.Module", j, i ), 0 );
			cd_sector_ch[i][j] = config->GetValue( Form( "CD_Sector_%d_%d.Channel", j, i ), 2*(int)j+(int)i );
			
			if( cd_sector_vme[i][j] < GetNumberOfVmeCrates() &&
			    cd_sector_mod[i][j] < GetMaximumNumberOfVmeModules() &&
			    cd_sector_ch[i][j] < GetMaximumNumberOfVmeChannels() ) {

			        cd_sector[cd_sector_vme[i][j]][cd_sector_mod[i][j]][cd_sector_ch[i][j]] = j;
			        cd_layer[cd_sector_vme[i][j]][cd_sector_mod[i][j]][cd_sector_ch[i][j]]  = i;
				
			}
			
			else {
				
				std::cerr << "Dodgy CD settings:";
				std::cerr << " crate = " << cd_sector_vme[i][j];
				std::cerr << " module = " << cd_sector_mod[i][j];
				std::cerr << " channel = " << cd_sector_ch[i][j] << std::endl;
				
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
      ch < GetMaximumNumberOfVmeChannels() )
    return cd_sector[(int)vme][(int)mod][(int)ch];
	
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
      ch < GetMaximumNumberOfVmeChannels() )
    return cd_ring[(int)vme][(int)mod][(int)ch];
	
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
	if( cd_sector[(int)vme][(int)mod][(int)ch] >= 0 || cd_layer[(int)vme][(int)mod][(int)ch] >= 0 ) return true;
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
	PRINT_SETTING_VECT_VECT_INT(cd_ring_vme);
	PRINT_SETTING_VECT_VECT_INT(cd_sector_vme);
	PRINT_SETTING_VECT_VECT_INT(cd_ring_mod);
	PRINT_SETTING_VECT_VECT_INT(cd_sector_mod);
	PRINT_SETTING_VECT_VECT_INT(cd_ring_ch);
	PRINT_SETTING_VECT_VECT_INT(cd_sector_ch);
	PRINT_SETTING_VECT_VECT_VECT_INT(cd_layer);
	PRINT_SETTING_VECT_VECT_VECT_INT(cd_sector);
	PRINT_SETTING_VECT_VECT_VECT_INT(cd_ring);
	

	std::cout << "==== Settings end ====" << std::endl;
}
