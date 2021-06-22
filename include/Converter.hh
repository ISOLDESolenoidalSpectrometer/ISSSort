#ifndef _Converter_hh
#define _Converter_hh

#include <bitset>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <sstream>
#include <string>

#include <TFile.h>
#include <TTree.h>
#include <TH1.h>
#include <TProfile.h>

// Common header
#ifndef __COMMON_HH
# include "Common.hh"
#endif

// Calibration header
#ifndef __CALIBRATION_HH
# include "Calibration.hh"
#endif

// Data packets header
#ifndef _DataPackets_hh
# include "DataPackets.hh"
#endif

// Uncomment for ASIC only data prior to June 2021
//#define ASIC_ONLY

// Uncomment for CAEN only data prior to June 2021
//#define CAEN_ONLY


class Converter {

public:
	
	Converter();
	~Converter();
	

	int ConvertFile( std::string input_file_name,
					  int start_block = 0,
					  int end_block = -1);
	void MakeHists();
	void MakeTree();
	
	void SetBlockHeader( char *input_header );
	void ProcessBlockHeader( int nblock );

	void SetBlockData( char *input_data );
	void ProcessBlockData( int nblock );
	
	void ProcessASICData();
	void ProcessCAENData();
	void ProcessInfoData();
	void FinishCAENData();

	void SetOutput( std::string output_file_name );
	
	inline void CloseOutput(){
		output_file->Close();
		delete data_packet;
		//delete asic_data;
		//delete caen_data;
		//delete info_data;
	};
	inline TFile* GetFile(){ return output_file; };
	inline TTree* GetTree(){ return output_tree; };

	inline void AddCalibration( Calibration *mycal ){ cal = mycal; };

	

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
		if( n >= nwords ) return(0);

		// Perform byte swapping according to swap mode
		ULong64_t result = data[n];
		if (swap & SWAP_ENDIAN) result = Swap64(result);
		if (swap & SWAP_WORDS)  result = SwapWords(result);
		return(result);
		
	};


	// Log file
	std::ofstream log_file;
	std::stringstream sslogs;

	
	// Set the size of the block and its components.
	static const int HEADER_SIZE = 24; // Size of header in bytes
#ifdef CAEN_ONLY
	static const int BLOCK_SIZE = 0x20000; // Block size for CAEN data = 128 kB prior to June 2021
#else
	static const int BLOCK_SIZE = 0x10000; // Block size for ISS/ASIC data = 64 kB, also CAEN data from June 2021 onwards
#endif
	static const int MAIN_SIZE = BLOCK_SIZE - HEADER_SIZE;
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
	bool flag_caen_data3;
	bool flag_caen_trace;
	bool flag_caen_info;


	// Raw data variables
	Char_t header_id[8]; // 8 byte. Must be this string 'EBYEDATA'.
	UInt_t header_sequence; // 4 byte.
	UShort_t header_stream; // 2 byte.
	UShort_t header_tape; // 2 byte.
	UShort_t header_MyEndian; // 2 byte. If 256 then correct endianess.
	UShort_t header_DataEndian; // 2 byte.
	UInt_t header_DataLen; // 4 byte.
	UInt_t nwords; // number of 64 bit words
	
	// Interpretated variables
	unsigned long my_tm_stp;
	unsigned long my_tm_stp_lsb;
	unsigned long my_tm_stp_msb;
	unsigned long my_tm_stp_hsb;
	unsigned long my_info_field;
	unsigned long my_info_code;
	unsigned long my_type;
	unsigned long my_adc_data;
	unsigned long my_hit;
	unsigned long my_mod_id;
	unsigned long my_ch_id;
	unsigned long my_asic_id;
	unsigned long my_data_id;
	
	// Flags
	bool ts_flag;
	bool hsb_ready;

	// For traces
	unsigned int nsamples;
	ULong64_t sample_packet;
	unsigned char trace_test;
	UInt_t block_test;

	// Data types
	DataPackets *data_packet;
	AsicData *asic_data;
	CaenData *caen_data;
	InfoData *info_data;
	
	// Output stuff
	TFile *output_file;
	TTree *output_tree;
	
	// Counters
	int ctr_asic_hit[common::n_module];		// hits on each ISS module
	int ctr_asic_ext[common::n_module];		// external (pulser) ISS timestamps
	int ctr_caen_hit[common::n_module];		// hits on each CAEN module
	int ctr_caen_ext[common::n_module];		// external (pulser) CAEN timestamps

	// Histograms
	TProfile *hasic_hit[common::n_module];
	TProfile *hasic_ext[common::n_module];
	TProfile *hcaen_hit[common::n_caen_mod];
	TProfile *hcaen_ext[common::n_caen_mod];

	TH1F *hasic[common::n_module][common::n_asic][common::n_channel];
	TH1F *hcaen[common::n_caen_mod][common::n_caen_ch];
	
	// 	Calibrator
	Calibration *cal;


};

#endif
