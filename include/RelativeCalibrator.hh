#ifndef __CDCALIBRATOR_HH
#define __CDCALIBRATOR_HH

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <memory>

#include <TFile.h>
#include <TTree.h>
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



class MiniballCDCalibrator {

public:

	MiniballCDCalibrator( std::shared_ptr<MiniballSettings> myset );
	~MiniballCDCalibrator() {};

	void	SetInputFile( std::vector<std::string> input_file_names );
	void	SetInputFile( std::string input_file_name );
	void	SetInputTree( TTree *user_tree );
	void	SetMBSInfoTree( TTree *user_tree );
	void	SetOutput( std::string output_file_name, bool cWrite = false );
	void	StartFile();	///< called for every file
	void	Initialise();	///< called for every event
	void	MakeHists();
	void	ResetHists();
	
	inline void AddCalibration( std::shared_ptr<MiniballCalibration> mycal ){
		cal = mycal;
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
	bool FindCDChannels( int det, int sec, int side, int strip, int &adc, int &ch );
	bool FindCDChannels( int det, int sec, int side, int strip, int &sfp, int &board, int &ch );

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
		mbsinfo_tree->Reset();
		output_file->Purge(2);
	}


private:
	
	/// Input tree
	TFile *input_file;
	TChain *input_tree;
	TChain *mbsinfo_tree;
	MiniballDataPackets *in_data = nullptr;
	MBSInfoPackets *mbs_info = nullptr;
	std::shared_ptr<DgfData> dgf_data;
	std::shared_ptr<AdcData> adc_data;
	std::shared_ptr<FebexData> febex_data;
	std::shared_ptr<InfoData> info_data;

	/// Outputs
	TFile *output_file;
	std::ofstream output_cal;

	// Do calibration
	std::shared_ptr<MiniballCalibration> cal;
	bool overwrite_cal;
	
	// Settings file
	std::shared_ptr<MiniballSettings> set;
	
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
	unsigned char		mysfp;			///< sfp number
	unsigned char		myboard;		///< febex board number
	//unsigned char		mydgf;			///< DGF module number
	unsigned char		myadc;			///< ADC module number
	unsigned char		mych;			///< channel number
	unsigned long long	mytime;			///< absolute timestamp
	long				myhittime;		///< hit time with respect to event time
	unsigned long long	myeventid;		///< MBS event id
	unsigned long long	preveventid;	///< previous MBS event id
	unsigned long long	myeventtime;	///< MBS event time
	float 				myenergy;		///< calibrated energy
	bool				mythres;		///< above threshold?
	bool				mypileup;		///< pileup flag?
	bool				myclipped;		///< clipped flag?


	// CD detector specific variables
	std::vector<float>					cd_en_list;		///< list of CD energies for ParticleFinder
	std::vector<unsigned int>			cd_Q_list;		///< list of CD uncalibrated energies for ParticleFinder
	std::vector<unsigned long long>		cd_ts_list;		///< list of CD timestamps for ParticleFinder
	std::vector<unsigned char>			cd_det_list;	///< list of CD detector IDs
	std::vector<unsigned char>			cd_sec_list;	///< list of CD sector IDs
	std::vector<unsigned char>			cd_side_list;	///< list of CD side IDs; 0 = p, 1 = n
	std::vector<unsigned char>			cd_strip_list;	///< list of CD strip IDs

	// Tag strips for calibration
	unsigned char ptag = 0;
	unsigned char ntag = 0;

	// Reference calibration coefficients
	double noffset = 0.0;
	double ngain = 1.0;
	double poffset = 0.0;
	double pgain = 1.0;

	// Counters
	unsigned long hit_ctr;
	unsigned long n_entries;
	unsigned long n_mbs_entries;

	// CD histograms
	std::vector<std::vector<std::vector<TH2F*>>> cd_pen_nen;
	std::vector<std::vector<std::vector<TH2F*>>> cd_nen_pen;
	std::vector<std::vector<std::vector<TH2F*>>> cd_pen_nQ;
	std::vector<std::vector<std::vector<TH2F*>>> cd_nQ_pQ;


};

#endif

