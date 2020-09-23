#include "DutClass.hh"

#include <TFile.h>

#include <iostream>
#include <sstream>

DutClass::DutClass( std::string dut_name,
				   int dut_sides,
				   int dut_asics,
				   std::string base_path,
				   std::string data_input_path,
				   std::string data_output_path,
				   std::string data_file_prefix,
				   std::string data_output_prefix,
				   std::string data_input_list,
				   std::string data_list_user,
				   int run_first,
				   int run_total,
				   bool flag_first_in_pipe,
				   bool flag_input_type,
				   bool flag_convert_file,
				   bool flag_sort_file,
				   bool flag_calibrate_file,
				   bool flag_eventbuild_file
				   ){
	
	this->dut_name         = dut_name;
	this->dut_sides        = dut_sides;
	this->dut_asics        = dut_asics;
	this->base_path  = base_path;
	this->data_input_path  = data_input_path;
	this->data_output_path = data_output_path;
	this->data_file_prefix = data_file_prefix;
	this->data_output_prefix = data_output_prefix;
	this->data_input_list = data_input_list;
	this->data_list_user = data_list_user;
	this->run_first        = run_first;
	this->run_total        = run_total;
	this->flag_first_in_pipe= flag_first_in_pipe;
	this->flag_input_type= flag_input_type;
	this->flag_convert_file= flag_convert_file;
	this->flag_sort_file= flag_sort_file;
	this->flag_calibrate_file= flag_calibrate_file;
	this->flag_eventbuild_file= flag_eventbuild_file;
	
	
}

DutClass::~DutClass(){;}

// -----------
// Set functions.
// -----------

void DutClass::DutClass::SetDutName (std::string name){
	dut_name=name;
}

void DutClass::DutClass::SetDutSides (int value){
	dut_sides=value;
}

void DutClass::SetBasePath (std::string name){
	base_path=name;
}

void DutClass::SetDataInputPath (std::string name){
	data_input_path=name;
}

void DutClass::SetDataOutputPath (std::string name){
	data_output_path=name;
}

void DutClass::SetDataFilePrefix (std::string name){
	data_file_prefix=name;
}

void DutClass::SetDataOutputPrefix (std::string name){
	data_output_prefix=name;
}

void DutClass::SetDataInputList (std::string name){
	data_input_list=name;
}

void DutClass::SetDataListUser (std::string name){
	data_list_user=name;
}
void DutClass::SetRunFirst(int value){
	run_first=value;
}

void DutClass::SetRunTotal(int value){
	run_total=value;
}

void DutClass::SetFlagFirstInPipe(bool flag){
	flag_first_in_pipe=flag;
}

void DutClass::SetFlagInputType(bool flag){
	flag_input_type=flag;
}

void DutClass::SetFlagConvertFile(bool flag){
	flag_convert_file=flag;
}

void DutClass::SetFlagSortFile(bool flag){
	flag_sort_file=flag;
}

void DutClass::SetFlagCalibrateFile(bool flag){
	flag_calibrate_file=flag;
}


void DutClass::SetFlagEventBuildFile(bool flag){
	flag_eventbuild_file=flag;
}


void DutClass::SetRunPresent (int value){
	run_present = value;
}

void DutClass::SetEntry (common::struct_entry_dut s_entry){
	s_entry_dut = s_entry;
}


// -----------
// Get functions.
// -----------

std::string DutClass::GetDutName(){
	return dut_name;
}

int DutClass::GetDutSides(){
	return dut_sides;
}

int DutClass::GetDutAsics(){
	return dut_asics;
}

std::string DutClass::GetBasePath(){
	return base_path;
}

std::string DutClass::GetDataInputPath(){
	return data_input_path;
}

std::string DutClass::GetDataOutputPath(){
	return data_output_path;
}

std::string DutClass::GetDataFilePrefix(){
	return data_file_prefix;
}

std::string DutClass::GetDataOutputPrefix(){
	return data_output_prefix;
}

std::string DutClass::GetDataInputList(){
	return data_input_list;
}

std::string DutClass::GetDataListUser(){
	return data_list_user;
}

int DutClass::GetRunFirst(){
	return run_first;
}

int DutClass::GetRunTotal(){
	return run_total;
}

bool DutClass::GetFlagFirstInPipe(){
	return flag_first_in_pipe;
}

bool DutClass::GetFlagInputType(){
	return flag_input_type;
}

bool DutClass::GetFlagConvertFile(){
	return flag_convert_file;
}

bool DutClass::GetFlagSortFile(){
	return flag_sort_file;
}

bool DutClass::GetFlagCalibrateFile(){
	return flag_calibrate_file;
}

bool DutClass::GetFlagEventBuildFile(){
	return flag_eventbuild_file;
}


int DutClass::GetRunPresent(){
	return run_present;
} 

common::struct_entry_dut DutClass::GetEntry(){
	return s_entry_dut;
}


//common::struct_entry_midas DutClass::GetEntryMidas(){
//  return s_entry_midas;
//}

TFile* DutClass::GetFileOutput(){
	return file_output;
}
