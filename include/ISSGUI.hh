// Very basic ROOT GUI to perform sorting of Miniball data
//
// Liam Gaffney (liam.gaffney@cern.ch) - 02/05/2017

#ifndef __ISSGUI_HH
#define __ISSGUI_HH

// ROOT include
#include <TSystem.h>
#include <TEnv.h>
#include <TGFileDialog.h>
#include <TGFontDialog.h>
#include <TGTextEntry.h>
#include <TGNumberEntry.h>
#include <TGButton.h>
#include <TGButtonGroup.h>
#include <TGProgressBar.h>
#include <TGLabel.h>
#include <TColor.h>
#include <TGFrame.h>
#include <TGWindow.h>
#include <TApplication.h>
#include <TGListBox.h>
#include <TList.h>
#include <TString.h>
#include <TObjString.h>
#include <TG3DLine.h>
#include <RQ_OBJECT.h>

// My code include.
#include "Settings.hh"
#include "Calibration.hh"
#include "Converter.hh"
#include "EventBuilder.hh"
#include "Reaction.hh"
#include "Histogrammer.hh"
#include "AutoCalibrator.hh"

// C++ include.
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>
#include <memory>
//#include <filesystem>


class ISSDialog {
	
	RQ_OBJECT( "ISSDialog" )
	
private:
	
	// Popup dialogs
	TGTransientFrame    *fDialog;
	TGTextButton        *but_yes, *but_no;
	TGLabel				*dialog_msg;
	TGHorizontalFrame   *frame_button;
	
	// Answer
	bool answer;
	
public:
	
	// Popup dialogs
	ISSDialog( const TGWindow *p, const TGWindow *main, TString msg, bool &ans );
	virtual ~ISSDialog();
	
	// Do the answer
	void SayYes();
	void SayNo();
	
	ClassDef( ISSDialog, 0 )
	
};

class ISSGUI {
	
	RQ_OBJECT( "ISSGUI" )
	
protected:
	
	// Frames
	TGMainFrame	        *main_frame;		// main frame
	TGVerticalFrame     *left_frame;		// left frame
	TGHorizontalFrame   *fbut_frame;		// sub frame for add/remove files buttons
	TGVerticalFrame		*centre_frame;		// centre frame for input files and directories, etc
	TGHorizontalFrame	*centre_files;		// centre frame for input files and directories, etc
	TGVerticalFrame     *centre_label;		// centre frame for input files and directories, etc
	TGVerticalFrame     *centre_text;		// centre frame for input files and directories, etc
	TGVerticalFrame     *centre_button;		// centre frame for input files and directories, etc
	TGHorizontalFrame 	*centre_go;			// bottom of frame for buttons and options
	TGVerticalFrame     *centre_progress;	// centre frame for progress bars

	// Separators
	TGVertical3DLine	*fVertical_0;
	TGHorizontal3DLine	*fHorizontal_0;

	// Progress bars
	std::shared_ptr<TGHProgressBar>	prog_conv;
	std::shared_ptr<TGHProgressBar>	prog_sort;
	std::shared_ptr<TGHProgressBar>	prog_evnt;
	std::shared_ptr<TGHProgressBar>	prog_hist;


	// Labels
	TGLabel				*lab_run_files;		// label for run file list
	TGLabel				*lab_set_file;		// label for settings file
	TGLabel				*lab_cal_file;		// label for calibration file
	TGLabel				*lab_rea_file;		// label for reaction file
	TGLabel				*lab_out_file;		// label for output file
	
	// Run list box
	TGListBox           *run_list_box;
	TList				*run_selected;
	
	// Text entries
	TGTextEntry         *text_add_file;		// text entry for adding a file
	TGTextEntry         *text_set_file;		// text entry for settings file
	TGTextEntry         *text_cal_file;		// text entry for calibration file
	TGTextEntry         *text_rea_file;		// text entry for reaction file
	TGTextEntry         *text_out_file;		// text entry for output file

	// Check buttons
	TGCheckButton       *check_force;		// check button to force resort
	TGCheckButton       *check_source;		// check button for source only
	TGCheckButton       *check_autocal;		// check button for automatic calibration
	TGCheckButton       *check_event;		// check button to rebuild events

	// Action buttons
	TGTextButton        *but_sel;			// button to select files
	TGTextButton        *but_add;			// button to add files
	TGTextButton        *but_del;			// button to remove files
	TGTextButton        *but_open;			// button to open configuration
	TGTextButton        *but_save;			// button to save configuration
	TGTextButton        *but_sort;			// button to do the sorting
	TGTextButton        *but_set;			// button to open settings file
	TGTextButton        *but_cal;			// button to open calibration file
	TGTextButton        *but_rea;			// button to open reaction file
	TGTextButton        *but_out;			// button to open output file

	
	// Files
	std::shared_ptr<ISSSettings> myset;
	std::shared_ptr<ISSCalibration> mycal;
	std::shared_ptr<ISSReaction> myrea;
	
	// select what steps of the analysis to be forced
	std::vector<bool> force_convert;
	bool force_sort;
	bool force_events;
	bool flag_convert;
	bool flag_events;
	bool flag_source;
	bool flag_autocal;

	
public:

	ISSGUI();				// constructor
	virtual ~ISSGUI();		// destructor
	
	// Analysis steps
	void gui_convert();
	void gui_sort();
	void gui_build();
	void gui_hist();
	void gui_autocal();

	// Slots
	TString		get_filename();
	inline void on_set_clicked(){ text_set_file->SetText( get_filename() ); };
	inline void on_cal_clicked(){ text_cal_file->SetText( get_filename() ); };
	inline void on_rea_clicked(){ text_rea_file->SetText( get_filename() ); };
	inline void on_out_clicked(){ text_out_file->SetText( get_filename() ); };
	inline void on_open_clicked(){ LoadSetup( get_filename() ); };
	inline void on_save_clicked(){ SaveSetup( get_filename() ); };
	void		on_del_clicked();
	void		on_sel_clicked();
	void		on_add_clicked();
	void		on_sort_clicked();

	// Save setup
	void SaveSetup( TString setupfile );
	void LoadSetup( TString setupfile );

	// File list
	std::vector<TString> filelist;
	std::vector<bool> filestatus;
	


	ClassDef( ISSGUI, 0 )
	
};

#endif
