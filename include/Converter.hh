#ifndef __CONVERTER_HH
#define __CONVERTER_HH

#include <bitset>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <sstream>
#include <string>
#include <cstring>
#include <memory>

#include <TFile.h>
#include <TTree.h>
#include <TTreeIndex.h>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>
#include <TGProgressBar.h>
#include <TSystem.h>


// Settings header
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif

// Calibration header
#ifndef __CALIBRATION_HH
# include "Calibration.hh"
#endif

// Data packets header
#ifndef __DATAPACKETS_HH
# include "DataPackets.hh"
#endif

class ISSConverter {

public:
	
	ISSConverter( std::shared_ptr<ISSSettings> myset );
	virtual ~ISSConverter(){};
	

	int ConvertFile( std::string input_file_name,
					 unsigned long start_block = 0,
					 long end_block = -1 );
	int ConvertBlock( char *input_block, int nblock );
	void MakeHists();
	void ResetHists();
	void MakeTree();
	void StartFile();
	unsigned long long SortTree();

	bool ProcessCurrentBlock( int nblock );

	void SetBlockHeader( char *input_header );
	void ProcessBlockHeader( unsigned long nblock );

	void SetBlockData( char *input_data );
	void ProcessBlockData( unsigned long nblock );

	void ProcessASICData();
	void ProcessCAENData();
	void ProcessMesytecData();
	void ProcessInfoData();
	void FinishCAENData();
	void FinishMesytecData();
	void GetVMEChanID();

	void SetOutput( std::string output_file_name );
	
	inline void CloseOutput(){
		std::cout << "\n Writing data and closing the file" << std::endl;
		//output_tree->SetDirectory(0);
		output_file->Write( 0, TObject::kWriteDelete );
		PurgeOutput();
		output_file->Close();
		//output_tree->ResetBranchAddresses();
		//sorted_tree->ResetBranchAddresses();
	};
	inline void PurgeOutput(){ output_file->Purge(2); }
	inline TFile* GetFile(){ return output_file; };
	inline TTree* GetTree(){ return output_tree; };
	inline TTree* GetSortedTree(){ return sorted_tree; };

	inline void AddCalibration( std::shared_ptr<ISSCalibration> mycal ){ cal = mycal; };
	inline void SourceOnly(){ flag_source = true; };

	inline void AddProgressBar( std::shared_ptr<TGProgressBar> myprog ){
		prog = myprog;
		_prog_ = true;
	};


private:

	// enumeration for swapping modes
	enum swap_t {
		SWAP_KNOWN  = 1,  // We know the swapping mode already
		SWAP_WORDS  = 2,  // We need to swap pairs of 32-bit words
		SWAP_ENDIAN = 4   // We need to swap endianness
	};
	Int_t swap;

	// Swap endianness of a 32-bit integer 0x01234567 -> 0x67452301
	inline UInt_t Swap32(UInt_t datum) {
		return(((datum & 0xFF000000) >> 24) |
			   ((datum & 0x00FF0000) >>  8) |
			   ((datum & 0x0000FF00) <<  8) |
			   ((datum & 0x000000FF) << 24));
	};
	
	// Swap the two halves of a 64-bit integer 0x0123456789ABCDEF ->
	// 0x89ABCDEF01234567
	inline ULong64_t SwapWords(ULong64_t datum) {
		return(((datum & 0xFFFFFFFF00000000LL) >> 32) |
			   ((datum & 0x00000000FFFFFFFFLL) << 32));
	};
	
	// Swap endianness of a 64-bit integer 0x0123456789ABCDEF ->
	// 0xEFCDAB8967452301
	inline ULong64_t Swap64(ULong64_t datum) {
		return(((datum & 0xFF00000000000000LL) >> 56) |
			   ((datum & 0x00FF000000000000LL) >> 40) |
			   ((datum & 0x0000FF0000000000LL) >> 24) |
			   ((datum & 0x000000FF00000000LL) >>  8) |
			   ((datum & 0x00000000FF000000LL) <<  8) |
			   ((datum & 0x0000000000FF0000LL) << 24) |
			   ((datum & 0x000000000000FF00LL) << 40) |
			   ((datum & 0x00000000000000FFLL) << 56));
	};
	
	// Get nth word
	inline ULong64_t GetWord( UInt_t n = 0 ){

		// If word number is out of range, return zero
		if( n >= WORD_SIZE ) return(0);

		// Perform byte swapping according to swap mode
		ULong64_t result = data[n];
		if (swap & SWAP_ENDIAN) result = Swap64(result);
		if (swap & SWAP_WORDS)  result = SwapWords(result);
		return(result);
		
	};

	
	// Flag for source run
	bool flag_source;

	// Logs
	std::stringstream sslogs;

	
	// Set the size of the block and its components.
	static const int HEADER_SIZE = 24; // Size of header in bytes
	//static const int DATA_BLOCK_SIZE = 0x20000; // Block size for CAEN data = 128 kB prior to June 2021
	static const int DATA_BLOCK_SIZE = 0x10000; // Block size for ISS/ASIC data = 64 kB, also CAEN data from June 2021 onwards
	static const int MAIN_SIZE = DATA_BLOCK_SIZE - HEADER_SIZE;
	static const int WORD_SIZE = MAIN_SIZE / sizeof(ULong64_t);

	// Set the arrays for the block components.
	char block_header[HEADER_SIZE];
	char block_data[MAIN_SIZE];
	
	// Data words - 1 word of 64 bits (8 bytes)
	ULong64_t word;
	
	// Data words - 2 words of 32 bits (4 byte).
	UInt_t word_0;
	UInt_t word_1;
	
	// Pointer to the data words
	ULong64_t *data;
	
	// End of data in  a block looks like:
	// word_0 = 0xFFFFFFFF, word_1 = 0xFFFFFFFF.
	// This flag is set true when we hit that point
	bool flag_terminator;

	// Flag to identify CAEN or ASIC data
	bool flag_asic_data;
	bool flag_caen_data0;
	bool flag_caen_data1;
	bool flag_caen_data2;
	bool flag_caen_data3;
	bool flag_caen_trace;
	bool flag_mesy_data0;
	bool flag_mesy_data1;
	bool flag_mesy_data2;
	bool flag_mesy_data3;
	bool flag_mesy_trace;
	bool flag_caen_info;


	// Raw data variables
	Char_t header_id[8]; // 8 byte. Must be this string 'EBYEDATA'.
	UInt_t header_sequence; // 4 byte.
	UShort_t header_stream; // 2 byte.
	UShort_t header_tape; // 2 byte.
	UShort_t header_MyEndian; // 2 byte. If 1 then correct endianess.
	UShort_t header_DataEndian; // 2 byte.
	UInt_t header_DataLen; // 4 byte.
	
	// Interpretated variables
	unsigned long long my_tm_stp;
	unsigned long my_tm_stp_lsb;
	unsigned long my_tm_stp_msb;
	unsigned long my_tm_stp_msb_asic;
	unsigned long my_tm_stp_hsb;
	unsigned long my_info_field;
	unsigned long my_info_code;
	unsigned long my_type;
	unsigned long my_adc_data;
	unsigned char my_vme_id;	// 0= CAEN, 1= Mesytec
	unsigned char my_mod_id;
	unsigned char my_ch_id;
	unsigned char my_asic_id;
	unsigned char my_data_id;
	bool my_hit;
	float my_energy;

	// For traces
	unsigned int nsamples;

	// Data types
	std::unique_ptr<ISSDataPackets> data_packet;
	std::shared_ptr<ISSAsicData> asic_data;
	std::shared_ptr<ISSCaenData> caen_data;
	std::shared_ptr<ISSMesyData> mesy_data;
	std::shared_ptr<ISSInfoData> info_data;
	
	// Output stuff
	TFile *output_file;
	TTree *output_tree;
	TTree *sorted_tree;

	// Counters
	std::vector<unsigned long> ctr_asic_hit;		// hits on each ISS module
	std::vector<unsigned long> ctr_asic_ext;		// external (pulser) ISS timestamps
	std::vector<unsigned long> ctr_asic_pause;   	// pause acq for module
	std::vector<unsigned long> ctr_asic_resume;  	// resume acq for module
	std::vector<unsigned long> ctr_caen_hit;		// hits on each CAEN module
	std::vector<unsigned long> ctr_caen_ext;		// external (pulser) CAEN timestamps
	std::vector<unsigned long> ctr_mesy_hit;		// hits on each Mesytec module
	std::vector<unsigned long> ctr_mesy_ext;		// external (pulser) Mesytec timestamps

	// Histograms
	std::vector<TProfile*> hasic_hit;
	std::vector<TProfile*> hasic_ext;
	std::vector<TProfile*> hasic_pause;
	std::vector<TProfile*> hasic_resume;
	std::vector<TProfile*> hcaen_hit;
	std::vector<TProfile*> hcaen_ext;
	std::vector<TProfile*> hmesy_hit;
	std::vector<TProfile*> hmesy_ext;

	// ASIC pulser for timing
	std::vector<std::vector<TH1F*>> asic_pulser_energy;

	std::vector<std::vector<TH2F*>> hasic;
	std::vector<std::vector<TH2F*>> hasic_cal;
	std::vector<std::vector<TH1F*>> hcaen_qlong;
	std::vector<std::vector<TH1F*>> hcaen_qshort;
	std::vector<std::vector<TH1F*>> hcaen_qdiff;
	std::vector<std::vector<TH1F*>> hcaen_cal;
	std::vector<std::vector<TH1F*>> hmesy_qlong;
	std::vector<std::vector<TH1F*>> hmesy_qshort;
	std::vector<std::vector<TH1F*>> hmesy_qdiff;
	std::vector<std::vector<TH1F*>> hmesy_cal;

	std::vector<TH1F*> hpside;
	std::vector<TH1F*> hnside;
	
	// 	Settings file
	std::shared_ptr<ISSSettings> set;

	// 	Calibrator
	std::shared_ptr<ISSCalibration> cal;

	// Progress bar
	bool _prog_;
	std::shared_ptr<TGProgressBar> prog;


};

#endif
