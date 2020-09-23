#include "Common.hh"
#include "DutClass.hh"

#include <TCanvas.h>
#include <TFile.h>
#include <TH2D.h>
#include <TObject.h>

#include <sstream>

void common::SaveObject(TObject* object, 
						const std::string& name, 
						DutClass* dut){
	
	std::stringstream object_name;
	object_name << dut->GetDutName()
	<< name;
	
	TFile* file_output = dut->GetFileOutput();
	file_output->cd();
	object->Write(object_name.str().c_str());
	
}

void common::SaveCanvas(TCanvas* canvas, const std::string& name, DutClass* dut){
	
	std::stringstream canvas_name;
	canvas_name << dut->GetDutName()
	<< name;
	canvas->SaveAs(canvas_name.str().c_str());
	
}
