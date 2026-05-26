#ifndef __RELATIVECALIBRATOR_HH
#define __RELATIVECALIBRATOR_HH

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <memory>

#include <TFile.h>
#include <TTree.h>
#include <TTreeIndex.h>
#include <TMath.h>
#include <TChain.h>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>
#include <TVector2.h>
#include <TVector3.h>
#include <TGProgressBar.h>
#include <TSystem.h>
#include <TKey.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TLinearFitter.h>
#include <TROOT.h>

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



class ISSRelativeCalibrator {

public:

	ISSRelativeCalibrator();
	~ISSRelativeCalibrator() {};

	void	SetInputFile( std::vector<std::string> input_file_names );
	void	SetInputFile( std::string input_file_name );
	void	SetInputTree( TTree *user_tree );
	void	SetOutput( std::string output_file_name, bool cWrite = false );
	void	StartFile();	///< called for every file
	void	Initialise();	///< called for every event
	void	MakeHists();
	void	ResetHists();
	
	/// Adds the settings from the external settings file to the class
	/// \param[in] myset The ISSSettings object which is constructed by the ISSSettings constructor used in iss_sort.cc
	inline void AddSettings( std::shared_ptr<ISSSettings> myset ){
		set = myset;
		overwrite_set = true;
	};

	/// Adds the calibration from the external calibration file to the class
	/// \param[in] mycal The ISSCalibration object which is constructed by the ISSCalibration constructor used in iss_sort.cc
	inline void AddCalibration( std::shared_ptr<ISSCalibration> mycal ){
		if( set.get() == nullptr ){
			std::cerr << "No settings given... Exiting!!" << std::endl;
			exit(0);
		}
		cal = mycal;
		cal->AddSettings( set );
		overwrite_cal = true;
	};
	inline void AddProgressBar( std::shared_ptr<TGProgressBar> myprog ){
		prog = myprog;
		_prog_ = true;
	};

	inline void SetPsideTagId( unsigned char id ) { ptag = id; };
	inline void SetNsideTagId( unsigned char id ) { ntag = id; };

	void CalibratePsides();
	void CalibrateNsides();

	unsigned long	FillHists();
	void			FillPixelHists();

	inline TFile* GetFile(){ return output_file; };
	inline void CloseOutput(){
		std::cout << "Writing output file...\r";
		std::cout.flush();
		output_file->Write( nullptr, TObject::kOverwrite );
		std::cout << "Writing output file... Done!" << std::endl << std::endl;
		PurgeOutput();
		output_file->Close();
		output_cal.close();
	}; ///< Closes the output files from this class
	inline void PurgeOutput(){
		input_tree->Reset();
		output_file->Purge(2);
	}


private:
	
	/// Input tree
	TFile *input_file;							///< Pointer to the time-sorted input ROOT file
	TChain *input_tree;							///< Pointer to the TTree in the data input file
	ISSDataPackets *in_data = nullptr;			///< Pointer to the TBranch containing the data in the time-sorted input ROOT file
	std::shared_ptr<ISSAsicData> asic_data;		///< Pointer to a given entry in the tree of some data from the ASICs
	std::shared_ptr<ISSVmeData> vme_data;		///< Pointer to a given entry in the tree of generic VME data from CAEN or Mesytec
	std::shared_ptr<ISSInfoData> info_data;		///< Pointer to a given entry in the tree of the "info" datatype

	/// Outputs
	TFile *output_file;
	std::ofstream output_cal;

	// Do calibration
	std::shared_ptr<ISSCalibration> cal; ///< Pointer to an ISSCalibration object, used for accessing gain-matching parameters and thresholds
	bool overwrite_cal; ///< Boolean determining whether an energy calibration should be used (true) or not (false). Set in the ISSEventBuilder::AddCalibration function

	// Settings file
	std::shared_ptr<ISSSettings> set; ///< Pointer to the settings object. Assigned in constructor
	bool overwrite_set; ///< Boolean determining whether an settings should be used (true) or not (false). Set in the ISSEventBuilder::AddSettings function

	// Progress bar
	bool _prog_;
	std::shared_ptr<TGProgressBar> prog;
	
	// Check if histograms are made
	bool hists_ready = false;

	// List of histograms for reset later
	TList *histlist;

	// Flag to know we've opened a file on disk
	bool flag_input_file;

	// Build window which comes from the settings file
	long build_window;  /// length of build window in ns

	// Flags
	bool flag_close_event;
	bool event_open;

	// Time variables
	long		 		time_diff;
	unsigned long long	time_prev, time_min, time_max, time_first;

	// Data variables - generic
	unsigned char		mymod;		///< module number
	unsigned char		mych;		///< channel number
	double				mytime;		///< absolute timestamp
	float 				myenergy;	///< calibrated energy
	int 				mywalk;		///< time walk correction
	bool				mythres;	///< above threshold?

	// Data variables - Array
	unsigned char		myasic;		///< ASIC number
	unsigned char		myside;		///< p-side = 0; n-side = 1
	unsigned char		myrow;		///< 4 wafers along array, 2 dE-E, 13 for gas
	int					mystrip;	///< strip number for DSSSD
	bool				myhitbit;	///< hit bit enabled?


	// Array variables
	std::vector<float>			pQ_list;	///< list of p-side ADC values
	std::vector<float>			nQ_list;	///< list of n-side ADC values
	std::vector<double>			ptd_list;	///< list of p-side time differences without time walk correction
	std::vector<double>			ntd_list;	///< list of n-side time differences without time walk correction
	std::vector<double>			pwalk_list;	///< list of p-side time differences WITH time walk correction
	std::vector<double>			nwalk_list;	///< list of n-side time differences WITH time walk correction
	std::vector<char>			pid_list;	///< list of p-side strip ids
	std::vector<char>			nid_list;	///< list of n-side strip ids
	std::vector<char>			pmod_list;	///< list of p-side modules numbers
	std::vector<char>			nmod_list;	///< list of n-side modules numbers
	std::vector<char>			prow_list;	///< list of p-side row numbers
	std::vector<char>			nrow_list;	///< list of n-side row numbers
	std::vector<bool>			phit_list;	///< list of p-side hit bit values
	std::vector<bool>			nhit_list;	///< list of n-side hit bit values

	// Tag strips for calibration
	unsigned char ptag = 0;
	unsigned char ntag = 0;

	// Reference calibration coefficients
	std::vector<std::vector<float>> noffset;
	std::vector<std::vector<float>> ngain;
	std::vector<std::vector<float>> poffset;
	std::vector<std::vector<float>> pgain;

	// Counters
	unsigned long hit_ctr;
	unsigned long n_entries;

	// CD histograms
	std::vector<std::vector<std::vector<TH2S*>>> array_pQ_nQ;
	std::vector<std::vector<std::vector<TH2S*>>> array_nQ_pQ;

};

#endif

