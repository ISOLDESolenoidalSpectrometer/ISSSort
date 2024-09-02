/*****************************************************************************
 * Copyright (C) 2009-2019   this file is part of the NPTool Project         *
 *                                                                           *
 * For the licensing terms see $NPTOOL/Licence/NPTool_Licence                *
 * For the list of contributors see $NPTOOL/Licence/Contributors             *
 *****************************************************************************/

/*****************************************************************************
 * Original Author: Adrien MATTA  contact address: a.matta@surrey.ac.uk      *
 * Author: M. Labiche                     address: marc.labiche@stfc.ac.uk   *
 *                                                                            *
 * Creation Date  : July 2019                                                *
 * Last update    :                                                          *
 *---------------------------------------------------------------------------*
 * Decription:                                                               *
 *  This class hold Iss Raw data                                             *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Comment:                                                                  *
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
#include "TIssData.hh"

ClassImp(TIssData)

TIssData::TIssData() {
	
	// Default constructor
	Clear();

}

TIssData::~TIssData() {}

void TIssData::Clear() {
	
	// DSSD
	fIss_StripFront_DetectorNbr.clear();
	fIss_StripFront_StripNbr.clear();
	fIss_StripFront_Energy.clear();
	fIss_StripFront_TimeCFD.clear();
	fIss_StripFront_TimeLED.clear();
	fIss_StripFront_Time.clear();

	std::vector<unsigned short>().swap(fIss_StripFront_DetectorNbr);
	std::vector<unsigned short>().swap(fIss_StripFront_StripNbr);
	std::vector<double>().swap(fIss_StripFront_Energy);
	std::vector<double>().swap(fIss_StripFront_TimeCFD);
	std::vector<double>().swap(fIss_StripFront_TimeLED);
	std::vector<double>().swap(fIss_StripFront_Time);
	
	fIss_StripBack_DetectorNbr.clear();
	fIss_StripBack_StripNbr.clear();
	fIss_StripBack_Energy.clear();
	fIss_StripBack_TimeCFD.clear();
	fIss_StripBack_TimeLED.clear();
	fIss_StripBack_Time.clear();
	
	std::vector<unsigned short>().swap(fIss_StripBack_DetectorNbr);
	std::vector<unsigned short>().swap(fIss_StripBack_StripNbr);
	std::vector<double>().swap(fIss_StripBack_Energy);
	std::vector<double>().swap(fIss_StripBack_TimeCFD);
	std::vector<double>().swap(fIss_StripBack_TimeLED);
	std::vector<double>().swap(fIss_StripBack_Time);

}



void TIssData::Dump() const {
	
	std::cout << "XXXXXXXXXXXXXXXXXXXXXXXX New Event XXXXXXXXXXXXXXXXX" << std::endl;
	
	// DSSD
	// Front
	std::cout << "Iss Strip Front Mult = " << fIss_StripFront_DetectorNbr.size() << std::endl;
	for( unsigned short i = 0; i < fIss_StripFront_DetectorNbr.size(); i++ ){
		std::cout << "DetNbr (Front): " << fIss_StripFront_DetectorNbr[i]
		<< "   Strip: " << fIss_StripFront_StripNbr[i]
		<< "   Energy: " << fIss_StripFront_Energy[i]
		<< "   Time CFD: " << fIss_StripFront_TimeCFD[i]
		<< "   Time LED: " << fIss_StripFront_TimeLED[i] << std::endl;
	}
	
	// Back
	std::cout << "Iss Strip Back Mult  = " << fIss_StripBack_DetectorNbr.size() << std::endl;
	for( unsigned short i = 0; i < fIss_StripBack_DetectorNbr.size(); i++ ){
		std::cout << "DetNbr (Back): " << fIss_StripBack_DetectorNbr[i]
		<< "   Strip: " << fIss_StripBack_StripNbr[i]
		<< "   Energy: " << fIss_StripBack_Energy[i]
		<< "   Time CFD: " << fIss_StripBack_TimeCFD[i]
		<< "   Time LED: " << fIss_StripBack_TimeLED[i] << std::endl;
	}

}
