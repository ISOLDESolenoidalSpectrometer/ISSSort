#ifndef __ISSDATA__
#define __ISSDATA__
/*****************************************************************************
 * Copyright (C) 2009-2019    this file is part of the NPTool Project        *
 *                                                                           *
 * For the licensing terms see $NPTOOL/Licence/NPTool_Licence                *
 * For the list of contributors see $NPTOOL/Licence/Contributors             *
 *****************************************************************************/

/*****************************************************************************
 * Original Author: Adrien MATTA  contact address: a.matta@surrey.ac.uk      *
 * Author: M. Labiche                     address: marc.labiche@stfc.ac.uk    *
 *                                                                           *
 * Creation Date  : Jul 2019                                                 *
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
#include <vector>
#include <iostream>

#include "TObject.h"

class TIssData : public TObject {
private:
	
	// DSSD
	std::vector<unsigned short>	fIss_StripFront_DetectorNbr; // detector number 1-24 for the individual wafers
	std::vector<unsigned short>	fIss_StripFront_StripNbr;	 // strip number 1-128 on front
	std::vector<double>			fIss_StripFront_Energy;		 // front strip energy
	std::vector<double>			fIss_StripFront_TimeCFD;	// cfd time
	std::vector<double>			fIss_StripFront_TimeLED;	// led time
	std::vector<double>			fIss_StripFront_Time;		// front strip time
	
	std::vector<unsigned short>	fIss_StripBack_DetectorNbr; // detector number 1-24 for the individual wafers
	std::vector<unsigned short>	fIss_StripBack_StripNbr;	// strip number 1-11 on back
	std::vector<double>			fIss_StripBack_Energy;		// back strip energy
	std::vector<double>			fIss_StripBack_TimeCFD;		// cfd time
	std::vector<double>			fIss_StripBack_TimeLED;		// led time
	std::vector<double>			fIss_StripBack_Time;		// back strip time
	
public:
	
	TIssData();
	~TIssData();
	
	void	Clear();
	void	Clear(const Option_t*) {};
	void	Dump() const;
	
	/////////////////////           SETTERS           ////////////////////////
	// DSSD
	inline void SetFront_DetectorNbr(const unsigned short& DetNbr)  {fIss_StripFront_DetectorNbr.push_back(DetNbr);}
	inline void SetFront_StripNbr(const unsigned short& StripNbr)   {fIss_StripFront_StripNbr.push_back(StripNbr);}
	inline void SetFront_Energy(const double& Energy)       {fIss_StripFront_Energy.push_back(Energy);}
	inline void SetFront_TimeCFD(const double& TimeCFD)     {fIss_StripFront_TimeCFD.push_back(TimeCFD);}
	inline void SetFront_TimeLED(const double& TimeLED)     {fIss_StripFront_TimeLED.push_back(TimeLED);}
	inline void SetFront_Time(const double& Time)           {fIss_StripFront_Time.push_back(Time);}
	
	inline void SetBack_DetectorNbr(const unsigned short& DetNbr)   {fIss_StripBack_DetectorNbr.push_back(DetNbr);}
	inline void SetBack_StripNbr(const unsigned short& StripNbr)    {fIss_StripBack_StripNbr.push_back(StripNbr);}
	inline void SetBack_Energy(const double& Energy)        {fIss_StripBack_Energy.push_back(Energy);}
	inline void SetBack_TimeCFD(const double& TimeCFD)      {fIss_StripBack_TimeCFD.push_back(TimeCFD);}
	inline void SetBack_TimeLED(const double& TimeLED)      {fIss_StripBack_TimeLED.push_back(TimeLED);}
	inline void SetBack_Time(const double& Time)            {fIss_StripBack_Time.push_back(Time);}
	
	inline void SetFront(const unsigned short& DetNbr,const unsigned short& StripNbr,const double& Energy,const double& TimeCFD,const double& TimeLED,const double& Time = 0)	{
		SetFront_DetectorNbr(DetNbr);
		SetFront_StripNbr(StripNbr);
		SetFront_Energy(Energy);
		SetFront_TimeCFD(TimeCFD);
		SetFront_TimeLED(TimeLED);
		SetFront_Time(Time);
	};
	
	inline void SetBack(const unsigned short &DetNbr,const unsigned short &StripNbr,const double &Energy,const double &TimeCFD,const double &TimeLED,const double &Time = 0)	{
		SetBack_DetectorNbr(DetNbr);
		SetBack_StripNbr(StripNbr);
		SetBack_Energy(Energy);
		SetBack_TimeCFD(TimeCFD);
		SetBack_TimeLED(TimeLED);
		SetBack_Time(Time);
	};
	
	/////////////////////           GETTERS           ////////////////////////
	// DSSD
	inline unsigned short	GetFront_DetectorNbr(const unsigned int &i) const {return fIss_StripFront_DetectorNbr[i];}//!
	inline unsigned short	GetFront_StripNbr(const unsigned int &i)    const {return fIss_StripFront_StripNbr[i];}//!
	inline double			GetFront_Energy(const unsigned int &i)      const {return fIss_StripFront_Energy[i]*1e3;}//!
	inline double			GetFront_TimeCFD(const unsigned int &i)     const {return fIss_StripFront_TimeCFD[i];}//!
	inline double			GetFront_TimeLED(const unsigned int &i)     const {return fIss_StripFront_TimeLED[i];}//!
	inline double			GetFront_Time(const unsigned int &i)        const {return fIss_StripFront_Time[i];}//!
		
	inline unsigned short	GetBack_DetectorNbr(const unsigned int &i) const {return fIss_StripBack_DetectorNbr[i];}//!
	inline unsigned short	GetBack_StripNbr(const unsigned int &i)    const {return fIss_StripBack_StripNbr[i];}//!
	inline double			GetBack_Energy(const unsigned int &i)      const {return fIss_StripBack_Energy[i]*1e3;}//!
	inline double			GetBack_TimeCFD(const unsigned int &i)     const {return fIss_StripBack_TimeCFD[i];}//!
	inline double			GetBack_TimeLED(const unsigned int &i)     const {return fIss_StripBack_TimeLED[i];}//!
	inline double			GetBack_Time(const unsigned int &i)        const {return fIss_StripBack_Time[i];}//!
	
	
	
	inline unsigned int GetMultiplicityFront() const {return fIss_StripFront_DetectorNbr.size();}//!
	inline unsigned int GetMultiplicityBack()  const {return fIss_StripBack_DetectorNbr.size();}//!
	
	ClassDef(TIssData,1);  // IssData structure
	
};

#endif
