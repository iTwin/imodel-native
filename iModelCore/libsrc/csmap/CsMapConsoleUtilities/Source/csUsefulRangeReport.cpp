//===========================================================================
// $Header$
//
//    (C) Copyright 2007 by Autodesk, Inc.
//
// The information contained herein is confidential, proprietary
// to Autodesk, Inc., and considered a trade secret as defined 
// in section 499C of the penal code of the State of California.  
// Use of this information by anyone other than authorized employees
// of Autodesk, Inc. is granted only under a written non-disclosure 
// agreement, expressly prescribing the scope and manner of such use.       
//
// CREATED BY:
//      Norm Olsen
//
// DESCRIPTION:
//

#include "csConsoleUtilities.hpp"
#include "csUsefulRangeReport.hpp"

extern "C" struct cs_Prjtab_ cs_Prjtab [];
extern "C" unsigned long KcsNmInvNumber;
extern "C" unsigned long KcsNmMapNoNumber;
extern "C" double cs_Huge;
extern "C" double cs_Mhuge;

// This module produces a report concerning the status of the current useful
// range portion of each coordinate system definition.  (Probably should do the
// same for datums and geodetic transformations, but schedule constraints
// limit the resources available to those required to satify the immediate
// need: Coordinate Reference Systems.

// We establish a basic "Useful Range Evaluation" data type (i.e. a class).
// A collection of such records is established; we'll use a vector for now;
// performance is not a big issue here as this utility may never ever be
// run again after its initial use. We then examine each coordinate system
// which has not be deprecated (i.e. not in the LEGACY group).  For each such
// system, a record is added to the above described collection.  The
// resulting collection is sorted and filtered in various ways to produce
// information concerning the useful range of all coordinate reference
// systems in relation of each other, and possible with regard to an
// equivalent specification in EPSG.

// As this utility has it's own class and supporting structure, it also
// has it's own header file.

bool csUsefulRangeReport (const wchar_t* reportDir,const wchar_t* csDictDir)
{
	bool ok (false);

	char csDictDirC [MAXPATH];
	wchar_t filePath [MAXPATH + MAXPATH];

	struct cs_Prjtab_ *prjTblPtr;  cs_Prjtab;
	std::vector<TcsUsefulRngRpt> usefulRangeVector;

	wcstombs (csDictDirC,csDictDir,sizeof (csDictDirC));
	int status = CS_altdr (csDictDirC);
	ok = (status == 0);
	if (!ok)
	{
		return false;
	}

	// Preclude a careless crash.
	if (wcslen (reportDir) > (MAXPATH - 10))
	{
		return ok;		// ok == false
	}

	const TcsEpsgDataSetV6* epsgPtr = GetEpsgObjectPtr ();
	ok = (epsgPtr != 0);
	if (!ok)
	{
		return ok;
	}

	// Build the report file path.
	wcsncpy (filePath,reportDir,MAXPATH);
	wcscat (filePath,L"\\");
	wcscat (filePath,L"CrsUsefulRangeReport.csv");

	// Create the CRS useful range collection.  We do this by cruising through
	// the coordinate system dictionary.  We'll use the binary form , since
	// we'll need the binary form of the numbers anyway.

	// We'll create the collection in a series of passes.  First pass is
	// through the Coordinate System Dictionary producing the initial
	// collection of data to work with.  Subsequent passes will add data
	// to the elements created during the initial pass.
	TcsCoordsysFile csCrsFile;
	if (csCrsFile.IsOk ())
	{
		size_t ii, csCount;
		const struct cs_Csdef_ *csDefPtr;

		double usefulRngLngW;
		double usefulRngLngE;
		double usefulRngLatS;
		double usefulRngLatN;

		csCount = csCrsFile.GetRecordCount ();
		usefulRangeVector.clear ();
		usefulRangeVector.reserve (1024);
		for (ii = 0;ii < (csCount - 1);ii +=1 )
		{
			bool epsgOk;

			TcsEpsgCode crsEpsgCode;
			TcsEpsgCode epsgAreaCode;

			csDefPtr = csCrsFile.FetchCoordinateSystem (ii);
			if (csDefPtr == 0) continue;
			
			// We ignore anything in the LEGACY group, they are deprecated.
			if (!CS_stricmp (csDefPtr->group,"LEGACY")) continue;

			// Let's skip the NERTH types as well.  They are strange beasts.
			if (!CS_stricmp (csDefPtr->prj_knm,"NERTH")) continue;
			if (!CS_stricmp (csDefPtr->prj_knm,"NRTHSRT")) continue;

			// OK, if we're still here, we'll be adding an object to the report
			// vector (assuming we don't encounter an error of some sort).
			TcsUsefulRngRpt nextReportEntry (csDefPtr->key_nm);

			// Populate the CS-MAP sort of stuff.
			for (prjTblPtr = cs_Prjtab;prjTblPtr->code != cs_PRJCOD_END;prjTblPtr++)
			{
				if (!CS_stricmp (prjTblPtr->key_nm,csDefPtr->prj_knm))
				{
					nextReportEntry.SetCrsPrjCode (prjTblPtr->code);
					nextReportEntry.SetPrjName (prjTblPtr->descr);
					break;
				}
			}

			TcsCrsRange csCrsRange (csDefPtr->ll_min,csDefPtr->ll_max);
			nextReportEntry.SetCsMapLimits (csCrsRange);

			// Get the EPSG code for this definition from the name mapper.
			crsEpsgCode = csMapNameToIdC (csMapProjGeoCSys,csMapFlvrEpsg,csMapFlvrAutodesk,csDefPtr->key_nm);
			if (crsEpsgCode == KcsNmInvNumber || crsEpsgCode == KcsNmMapNoNumber)
			{
				crsEpsgCode.Invalidate ();
			}
			epsgOk = crsEpsgCode.IsValid ();
			if (epsgOk)
			{
				nextReportEntry.SetCrsEpsgCode (crsEpsgCode);

				// We know that the EPSG code is valid, see if we can get a
				// valid Area Code.
				epsgOk = epsgPtr->GetFieldByCode (epsgAreaCode,epsgTblReferenceSystem,epsgFldAreaOfUseCode,crsEpsgCode);
				if (epsgAreaCode == KcsNmInvNumber || epsgAreaCode == KcsNmMapNoNumber)
				{
					epsgAreaCode.Invalidate ();
				}
				epsgOk = epsgOk && epsgAreaCode.IsValid ();
				if (epsgOk)
				{
					epsgOk = epsgAreaCode.IsValid ();
				}
				if (epsgOk)
				{
					std::wstring areaName;
			
					nextReportEntry.SetAreaEpsgCode (epsgAreaCode);
					epsgOk  = epsgPtr->GetFieldByCode (usefulRngLngW,epsgTblArea,epsgFldAreaWestBoundLng,epsgAreaCode);
					epsgOk &= epsgPtr->GetFieldByCode (usefulRngLngE,epsgTblArea,epsgFldAreaEastBoundLng,epsgAreaCode);
					epsgOk &= epsgPtr->GetFieldByCode (usefulRngLatS,epsgTblArea,epsgFldAreaSouthBoundLat,epsgAreaCode);
					epsgOk &= epsgPtr->GetFieldByCode (usefulRngLatN,epsgTblArea,epsgFldAreaNorthBoundLat,epsgAreaCode);
					if (epsgOk)
					{
						TcsCrsRange epsgRange (usefulRngLngW,usefulRngLatS,usefulRngLngE,usefulRngLatN);
						nextReportEntry.SetEpsgLimits (epsgRange);
					}
					epsgOk = epsgPtr->GetFieldByCode (areaName,epsgTblArea,epsgFldAreaName,epsgAreaCode);
					if (epsgOk)
					{
						nextReportEntry.SetAreaName (areaName);
					}
					
				}
			}

			short csMapWidth = nextReportEntry.CsMapLngRange ();
			short csMapHeight = nextReportEntry.CsMapLatRange ();
			short epsgWidth = nextReportEntry.EpsgLngRange ();
			short epsgHeight = nextReportEntry.EpsgLatRange ();
			short deltaWidth = csMapWidth - epsgWidth;
			short deltaHeight = csMapHeight - epsgHeight;
			long rangeEvaluation = 0L;
			if (csMapWidth == 0 && csMapHeight == 0)
			{
				rangeEvaluation -= 1L;
			}
			if (epsgWidth == 0 && epsgHeight == 0)
			{
				rangeEvaluation -= 2L;
			}
			if (rangeEvaluation == 0L)
			{
				if (deltaWidth < 0)  rangeEvaluation += -(deltaWidth + deltaWidth);
				if (deltaWidth > 30) rangeEvaluation += (deltaWidth - 30);
				if (deltaHeight < 0)  rangeEvaluation += -(deltaHeight + deltaHeight);
				if (deltaHeight > 30) rangeEvaluation += (deltaHeight - 30);
			}
			nextReportEntry.SetRangeEvaluation (rangeEvaluation);

			// Validate this entry.  If it doesn't validate, report it and
			// skip on to the next entry.
			ok = nextReportEntry.Validate ();
			if (ok)
			{
				// Add the new entry to the collection we are building.
				usefulRangeVector.push_back (nextReportEntry);
			}
			else
			{
				std::wcout << "CRS named: \"" << csDefPtr->key_nm << "\" is invalid." << std::endl;
			}
		}

		// OK, we have the entire list.  We sort the thing according to our
		// operator< encoding.
		std::sort (usefulRangeVector.begin(),usefulRangeVector.begin());

		// OK, now we simply write the data in a CSV format.  Using this technique, those
		// examining the file can use Excel (or something similar) and sort it anyway
		// they like.
		std::wofstream oStrm;
		oStrm.open (filePath,std::ios_base::out | std::ios_base::trunc);
		if (oStrm.is_open ())
		{
			std::vector<TcsUsefulRngRpt>::iterator rptItr;
			rptItr = usefulRangeVector.begin();
			for (rptItr = usefulRangeVector.begin();rptItr != usefulRangeVector.end();rptItr++)
			{
				rptItr->WriteToStream (oStrm,rptItr == usefulRangeVector.begin());
			}
			oStrm.close ();
			ok = true;
		}
	}
	// OK, we're done.
	return ok;
}
/*=============================================================================
		struct TcsCrsRange Implementation
  ===========================================================================*/
TcsCrsRange::TcsCrsRange (void)
{
	RangeSW [0] = cs_Huge;
	RangeSW [1] = cs_Huge;
	RangeNE [0] = cs_Mhuge;
	RangeNE [1] = cs_Mhuge;
}
TcsCrsRange::TcsCrsRange (double swLng,double swLat,double neLng,double neLat)
{
	RangeSW [0] = swLng;
	RangeSW [1] = swLat;
	RangeNE [0] = neLng;
	RangeNE [1] = neLat;
}
TcsCrsRange::TcsCrsRange (const double southwest [2],const double northeast [2])
{
	RangeSW [0] = southwest [0];
	RangeSW [1] = southwest [1];
	RangeNE [0] = northeast [0];
	RangeNE [1] = northeast [1];
}
TcsCrsRange::TcsCrsRange (const TcsCrsRange& source)
{
	RangeSW [0] = source.RangeSW [0];
	RangeSW [1] = source.RangeSW [1];
	RangeNE [0] = source.RangeNE [0];
	RangeNE [1] = source.RangeNE [1];
}
TcsCrsRange& TcsCrsRange::operator= (const TcsCrsRange& rhs)
{
	RangeSW [0] = rhs.RangeSW [0];
	RangeSW [1] = rhs.RangeSW [1];
	RangeNE [0] = rhs.RangeNE [0];
	RangeNE [1] = rhs.RangeNE [1];
	return *this;
}

TcsCrsRange& TcsCrsRange::operator+= (const TcsCrsRange& rhs)
{
	if (rhs.RangeSW [0] < RangeSW [0]) RangeSW [0] = rhs.RangeSW [0];
	if (rhs.RangeSW [1] < RangeSW [1]) RangeSW [1] = rhs.RangeSW [1];
	if (rhs.RangeNE [0] > RangeNE [0]) RangeNE [0] = rhs.RangeNE [0];
	if (rhs.RangeNE [1] > RangeNE [1]) RangeNE [1] = rhs.RangeNE [1];
	return *this;
}
short TcsCrsRange::LngRange (void) const
{
	short rtnValue (0);

	if (RangeSW [0] < RangeNE [0])
	{
		rtnValue = (short)((RangeNE [0] * 60.0) - (RangeSW [0] * 60.0));
	}
	return rtnValue;
}
short TcsCrsRange::LatRange (void) const
{
	short rtnValue (0);

	if (RangeSW [1] < RangeNE [1])
	{
		rtnValue = (short)((RangeNE [1] * 60.0) - (RangeSW [1] * 60.0));
	}
	return rtnValue;
}
std::wostream& operator<< (std::wostream& oStrm,const TcsCrsRange& subject)
{
	unsigned char degSign = 0260;

	long swLngDeg, swLngMin;
	long swLatDeg, swLatMin;
	long neLngDeg, neLngMin;
	long neLatDeg, neLatMin;

	long lngTmp;

	char buffer [256];
	wchar_t bufferWC [256];

	lngTmp = (long)((subject.RangeSW [0] * 60.0) + ((subject.RangeSW [0] >= 0.0) ? 0.5 : -0.5));
	swLngDeg = lngTmp / 60;
	swLngMin = abs(lngTmp) % 60;

	lngTmp = (long)((subject.RangeSW [1] * 60.0) + ((subject.RangeSW [1] >= 0.0) ? 0.5 : -0.5));
	swLatDeg = lngTmp / 60;
	swLatMin = abs(lngTmp) % 60;

	lngTmp = (long)((subject.RangeNE [0] * 60.0) + ((subject.RangeNE [0] >= 0.0) ? 0.5 : -0.5));
	neLngDeg = lngTmp / 60;
	neLngMin = abs(lngTmp) % 60;

	lngTmp = (long)((subject.RangeNE [1] * 60.0) + ((subject.RangeNE [1] >= 0.0) ? 0.5 : -0.5));
	neLatDeg = lngTmp / 60;
	neLatMin = abs(lngTmp) % 60;

	sprintf (buffer,"%4d%c%02d' %4d%c%02d' -> %4d%c%02d' %4d%c%02d'",swLngDeg,degSign,swLngMin,
																	 swLatDeg,degSign,swLatMin,
																	 neLngDeg,degSign,neLngMin,
																	 neLatDeg,degSign,neLatMin);

	mbstowcs (bufferWC,buffer,wcCount(bufferWC));
	oStrm << bufferWC;
	return oStrm;
}
/*=============================================================================
		class TcsCrsRange Implementation
  ===========================================================================*/
// Construction / Destruction / Assignment
TcsUsefulRngRpt::TcsUsefulRngRpt (void) : Valid         (false),
										CsPrjCode     (cs_PRJCOD_END),
										RngEvaluation (0),
										CrsEpsgCode   (),
										AreaEpsgCode  (),
										CsMapRange    (),
										EpsgRange     ()
{
	memset (CsKeyName,'\0',sizeof (CsKeyName));
	memset (CsPrjName,'\0',sizeof (CsPrjName));
	memset (AreaName,'\0',sizeof (AreaName));
}
TcsUsefulRngRpt::TcsUsefulRngRpt (const char* csKeyName): Valid         (false),
														  CsPrjCode     (cs_PRJCOD_END),
														  RngEvaluation (0),
														  CrsEpsgCode   (),
														  AreaEpsgCode  (),
														  CsMapRange    (),
														  EpsgRange     ()
{
	mbstowcs (CsKeyName,csKeyName,wcCount (CsKeyName));
	CsKeyName [wcCount(CsKeyName) - 1] = L'\0';
	memset (CsPrjName,'\0',sizeof (CsPrjName));
	memset (AreaName,'\0',sizeof (AreaName));
}
TcsUsefulRngRpt::TcsUsefulRngRpt (const TcsUsefulRngRpt& source) : Valid         (source.Valid),
																   CsPrjCode     (source.CsPrjCode),
																   RngEvaluation (source.RngEvaluation),
																   CrsEpsgCode   (source.CrsEpsgCode),
																   AreaEpsgCode  (source.AreaEpsgCode),
																   CsMapRange    (source.CsMapRange),
																   EpsgRange     (source.EpsgRange)
{
	wcsncpy (CsKeyName,source.CsKeyName,wcCount (CsKeyName));
	wcsncpy (CsPrjName,source.CsPrjName,wcCount (CsPrjName));
	wcsncpy (AreaName,source.AreaName,wcCount (AreaName));
}
TcsUsefulRngRpt::~TcsUsefulRngRpt (void)
{
	// Nothing to do here.
}
TcsUsefulRngRpt& TcsUsefulRngRpt::operator= (const TcsUsefulRngRpt& rhs)
{
	if (&rhs != this)
	{
		wcsncpy (CsKeyName,rhs.CsKeyName,wcCount (CsKeyName));
		wcsncpy (CsPrjName,rhs.CsPrjName,wcCount (CsPrjName));
		wcsncpy (AreaName,rhs.AreaName,wcCount (CsPrjName));
		Valid         = rhs.Valid;
		CsPrjCode     = rhs.CsPrjCode;
		RngEvaluation = rhs.RngEvaluation;
		CrsEpsgCode   = rhs.CrsEpsgCode;
		AreaEpsgCode  = rhs.AreaEpsgCode;
		CsMapRange    = rhs.CsMapRange;
		EpsgRange     = rhs.EpsgRange;
	}
	return *this;
}
// Operator Overrides / Virtual Functions
// This is the sort comparison function.  The code for this function will
// be adjusted to produce the various reports.
bool TcsUsefulRngRpt::operator< (TcsUsefulRngRpt& comparedTo)
{
	long result;

	result = RngEvaluation - comparedTo.RngEvaluation;
	return (result < 0L);
}
void TcsUsefulRngRpt::SetKeyName (const char* csKeyName)
{
	mbstowcs (CsKeyName,csKeyName,wcCount (CsKeyName));
	Valid = false;
}
void TcsUsefulRngRpt::SetPrjName (const char* prjName)
{
	mbstowcs (CsPrjName,prjName,wcCount (CsPrjName));
	Valid = false;
}
void TcsUsefulRngRpt::SetAreaName (const std::wstring& areaName)
{
	wcsncpy (AreaName,areaName.c_str (),wcCount (AreaName));
	AreaName [wcCount(AreaName) - 1] = L'\0';
	Valid = false;
}
short TcsUsefulRngRpt::SetCrsPrjCode (short csPrjCode)
{
	short rtnValue;

	rtnValue = (short)CsPrjCode;
	CsPrjCode = csPrjCode;
	Valid = false;
	return rtnValue;
}
long TcsUsefulRngRpt::SetRangeEvaluation (long rangeEvaluation)
{
	long rtnValue;
	
	rtnValue = RngEvaluation;
	RngEvaluation = rangeEvaluation;
	return rtnValue;
}
TcsEpsgCode TcsUsefulRngRpt::SetCrsEpsgCode (TcsEpsgCode crsEpsgCode)
{
	TcsEpsgCode rtnValue;
	
	if (CrsEpsgCode.IsValid ())
	{
		rtnValue = (short)CrsEpsgCode;
	}
	else
	{
		rtnValue = 0;
	}
	CrsEpsgCode = crsEpsgCode;
	return rtnValue;
}
TcsEpsgCode TcsUsefulRngRpt::SetAreaEpsgCode (TcsEpsgCode areaEpsgCode)
{
	TcsEpsgCode rtnValue;
	
	if (AreaEpsgCode.IsValid ())
	{
		rtnValue = (short)AreaEpsgCode;
	}
	else
	{
		rtnValue = 0;
	}
	AreaEpsgCode = areaEpsgCode;
	return rtnValue;
}
void TcsUsefulRngRpt::SetCsMapLimits (const struct TcsCrsRange& csRange)
{
	CsMapRange = csRange;
	Valid = false;
}
void TcsUsefulRngRpt::SetEpsgLimits (const struct TcsCrsRange& csRange)
{
	EpsgRange = csRange;
	Valid = false;
}
short TcsUsefulRngRpt::CsMapLngRange (void) const
{
	short rtnValue = CsMapRange.LngRange ();
	return rtnValue;
}
short TcsUsefulRngRpt::CsMapLatRange (void) const
{
	short rtnValue = CsMapRange.LatRange ();
	return rtnValue;
}
short TcsUsefulRngRpt::EpsgLngRange (void) const
{
	short rtnValue = EpsgRange.LngRange ();
	return rtnValue;
}
short TcsUsefulRngRpt::EpsgLatRange (void) const
{
	short rtnValue = EpsgRange.LatRange ();
	return rtnValue;
}
void TcsUsefulRngRpt::WriteToStream (std::wostream& oStrm,bool writeHeader)
{
	std::wstring wBuffer1;
	std::wstring wBuffer2;
	std::wstring wBuffer3;

	wBuffer1 = CsKeyName;
	wBuffer2 = CsPrjName;
	wBuffer3 = AreaName;
	csCsvQuoter (wBuffer1,true);
	csCsvQuoter (wBuffer2,true);
	csCsvQuoter (wBuffer3,true);

	if (writeHeader)
	{
		oStrm << L"EpsgCode"        <<  L','
			  << L"KeyName"         <<  L','
			  << L"Projection"      <<  L','
			  << L"PrjCode"         <<  L','
			  << L"EpsgArea"        <<  L','
			  << L"AreaName"        <<  L','
			  << L"RngEvaluator"    <<  L','
			  << L"CsMapWidth"      <<  L','
			  << L"CsMapHeight"     <<  L','
			  << L"EpsgWidth"       <<  L','
			  << L"EpsgHeight"      <<  L','
			  << L"CsMapRange"      <<  L','
			  << L"EpsgRange"       <<  std::endl;
	}

	oStrm << CrsEpsgCode            <<  L','
		  << wBuffer1               <<  L','
		  << wBuffer2               <<  L','
		  << CsPrjCode              <<  L','
		  << AreaEpsgCode           <<  L','
		  << wBuffer3               <<  L','
		  << RngEvaluation          <<  L','
		  << CsMapRange.LngRange () <<  L','
		  << CsMapRange.LatRange () <<  L','
		  << EpsgRange.LngRange ()  <<  L','
		  << EpsgRange.LatRange ()  <<  L','
		  << CsMapRange             <<  L',';
	if (AreaEpsgCode.IsValid ())
	{
		oStrm  << EpsgRange         <<  std::endl;
	}
	else
	{
		oStrm  <<  L','  <<  std::endl;
	}
	return;
}
bool TcsUsefulRngRpt::Validate (void)
{
	// Verify that we have a been given a valid CS-MAP
	// useful range.
	Valid = (CsMapLngRange () >= 0) &&
			(CsMapLatRange () >= 0);

	// If this definition has an equivalent EPSG code, and that
	// EPSG code has a valid Area code, we verify that we have
	// a valid EPSG useful range.
	if (Valid && CrsEpsgCode.IsValid () && AreaEpsgCode.IsValid ())
	{
		Valid = (EpsgLngRange () >= 0) &&
				(EpsgLatRange () >= 0);
	}

	return Valid;
}
bool csUsefulRangeMatchList (const wchar_t* reportDir,const wchar_t* csDictDir)
{
	bool ok (false);

	char csDictDirC [MAXPATH];
	wchar_t filePath [MAXPATH + MAXPATH];

	wcstombs (csDictDirC,csDictDir,sizeof (csDictDirC));
	int status = CS_altdr (csDictDirC);
	ok = (status == 0);
	if (!ok)
	{
		return false;
	}

	// Preclude a careless crash.
	if (wcslen (reportDir) > (MAXPATH - 10))
	{
		return ok;		// ok == false
	}

	// Build the report file path.
	wcsncpy (filePath,reportDir,MAXPATH);
	wcscat (filePath,L"\\");
	wcscat (filePath,L"CrsUsefulRangeMatch.c");
	std::wofstream oStrm;
	oStrm.open (filePath,std::ios_base::out | std::ios_base::trunc);
	
	TcsCoordsysFile csCrsFile;
	if (csCrsFile.IsOk () && oStrm.is_open())
	{
		int cmpValue;
		size_t curIdx;
		const struct cs_Csdef_* lastCRS (0);		// Last CRS with a useful range
		const struct cs_Csdef_* curCRS =(0);

		csCrsFile.OrderByProjection ();

		size_t csCount = csCrsFile.GetRecordCount ();
		for (curIdx = 1;curIdx < (csCount - 1);curIdx +=1 )
		{
			curCRS = csCrsFile.FetchCoordinateSystem (curIdx);
			if (!CS_stricmp (curCRS->group,"LEGACY"))
			{
				continue;
			}
			if (CS_crsHasUsefulRng (*curCRS))
			{
				lastCRS = curCRS;
			}
			else if (lastCRS != 0)
			{
				cmpValue = TcsCoordsysFile::ProjectionCompare (*lastCRS,*curCRS);
				if (cmpValue == 0)
				{
					const wchar_t *wcPtr;
					wchar_t pad1 [64];
					wchar_t pad2 [64];

					// We have two CRS's which are essentially the same definition
					// with regard to projection and parameters and one has a
					// useful range and the second does not.  Differences, if
					// any, between the CRS's is datum and cartesian units.
					// Given that all locating values in the definition are
					// either scales and.or geographic coordinates, the geographic
					// coordinates of the useful range of both systems should be
					// approximately the same.  CLose enough for a useful range,
					// anyway.
					wcPtr = CS_wcPad (28 - static_cast<int>(strlen (lastCRS->key_nm)));
					wcsncpy (pad1,wcPtr,wcCount (pad1));
					pad1 [wcCount (pad1) - 1] = L'\0';
					wcPtr = CS_wcPad (27 - static_cast<int>(strlen (curCRS->key_nm)));
					wcsncpy (pad2,wcPtr,wcCount (pad2));
					pad2 [wcCount (pad2) - 1] = L'\0';
					oStrm << L"\t{   \""
						  << lastCRS->key_nm
						  << L"\","
						  << pad1
						  << L"\""
						  << curCRS->key_nm
						  << L"\" "
						  << pad2
						  << L"},    //"
						  << lastCRS->ll_min [0] << L", "
						  << lastCRS->ll_min [1] << L", "
						  << lastCRS->ll_max [0] << L", "
						  << lastCRS->ll_max [1] << std::endl;
				}
			}
		}
		ok = (curIdx >= (csCount - 1));
		oStrm.close ();
	}
	return (ok);
}

struct TcsUsefulRngTransferTbl_
{
	char rngSrc [cs_KEYNM_DEF];
	char rngTrg [cs_KEYNM_DEF];
} const KcsUsefulRngTransferTbl [] =
{
//	{   "LL",                          "LL-WP"                       },    //-182, -90, 182, 90
	{   "LL27",                        "LL27-WP"                     },    //-194.35, 5.98333, -45.7333, 85.1667
	{   "LL83",                        "LL83-WP"                     },    //-194.35, 12.9333, -45.7333, 88.45
	{   "CRS:84",                      "LL84-WP"                     },    //-182, -90, 182, 90
//	{   "CRS:84",                      "LL-SJTSK"                    },    //-182, -90, 182, 90
	{   "CRS:84",                      "LL-SJTSK95"                  },    //-182, -90, 182, 90
	{   "Campo.Argentina1",            "FAJA-P1"                     },    //-73.9667, -53.5833, -70.1167, -34.5833
	{   "Campo.Argentina2",            "AR-T0"                       },    //-70.8833, -57.9833, -67.1333, -21.0167
	{   "Campo.Argentina2",            "AR-CM"                       },    //-70.8833, -57.9833, -67.1333, -21.0167
	{   "Campo.Argentina2",            "FAJA-P2"                     },    //-70.8833, -57.9833, -67.1333, -21.0167
	{   "Campo.Argentina2",            "AR-T1"                       },    //-70.8833, -57.9833, -67.1333, -21.0167
	{   "PGA98.Argentina-3",           "FAJA-P3"                     },    //-67.5, -55.2, -64.5, -21.8
	{   "Campo.Argentina4",            "FAJA-P4"                     },    //-64.8833, -58.1833, -61.1333, -18.7167
	{   "PGA98.Argentina-5",           "FAJA-P5"                     },    //-61.5, -39.2, -58.5, -23.4
	{   "PGA98.Argentina-6",           "FAJA-P6"                     },    //-58.5, -38.5, -55.5, -25.1
	{   "PGA98.Argentina-7",           "FAJA-P7"                     },    //-55.5, -27.9, -53.5, -25.5
	{   "Schwarzk.SWAfrican-15",       "NAMIBIA"                     },    //13.75, -29.5, 16.25, -16.2167
	{   "FIJI-MG",                     "FJ-MS31"                     },    //-188.5, -26.6333, -174.783, -8.26667
	{   "FIJI-MG",                     "Fiji1986a.FijiMapGrid"       },    //-188.5, -26.6333, -174.783, -8.26667
	{   "PSAD56.PeruCentral",          "PERU-C"                      },    //-79.75, -18.2167, -72.25, 1.61667
	{   "PSAD56.PeruEast",             "PERU-E"                      },    //-73.5333, -20.0167, -68.1333, -0.533333
	{   "PSAD56.PeruWest",             "PERU-W"                      },    //-81.7, -8.81667, -78.7, -2.88333
	{   "CANA83-10TM115",              "CANA27-10TM115"              },    //-121.25, 47.9, -108.75, 61.1
	{   "MTM83-17IF",                  "CANQ27-M17"                  },    //-98, 41, -94, 84
	{   "MTM83-17IF",                  "CAN27-17"                    },    //-98, 41, -94, 84
	{   "MTM83-17IF",                  "CANQ-M17"                    },    //-98, 41, -94, 84
	{   "MTM27-16IF",                  "CAN27-16"                    },    //-95, 41, -91, 84
	{   "MTM27-16IF",                  "CANQ-M16"                    },    //-95, 41, -91, 84
	{   "MTM27-16IF",                  "CANQ27-M16"                  },    //-95, 41, -91, 84
//	{   "MTM27-16IF",                  "CAN27-Ont15"                 },    //-95, 41, -91, 84
//	{   "NSRS11.WisconsinTM",          "CAN27-15"                    },    //-92.88, 42.49, -86.25, 47.29
//	{   "NSRS11.WisconsinTM",          "CANQ27-M15M"                 },    //-92.88, 42.49, -86.25, 47.29
//	{   "NSRS11.WisconsinTM",          "CANQ-M15"                    },    //-92.88, 42.49, -86.25, 47.29
	{   "MTM83-14IF",                  "CANQ27-M14"                  },    //-89, 41, -85, 84
	{   "MTM83-14IF",                  "CAN27-14"                    },    //-89, 41, -85, 84
//	{   "MTM83-14IF",                  "CAN27-Ont16"                 },    //-89, 41, -85, 84
	{   "MTM83-14IF",                  "CAN83-14"                    },    //-89, 41, -85, 84
	{   "MTM27-13IF",                  "CANQ27-M13"                  },    //-86, 41, -82, 84
//	{   "MTM27-13IF",                  "WGS84.CRTM05"                },    //-86, 41, -82, 84
	{   "MTM27-13IF",                  "CAN27-13"                    },    //-86, 41, -82, 84
	{   "MTM27-13IF",                  "CANQ-M13"                    },    //-86, 41, -82, 84
	{   "MTM83-11IF",                  "CAN27-11"                    },    //-84.5, 41, -80.5, 84
	{   "MTM83-11IF",                  "CANQ27-M11M"                 },    //-84.5, 41, -80.5, 84
	{   "MTM83-11IF",                  "CANQ-M11"                    },    //-84.5, 41, -80.5, 84
	{   "MTM27-12IF",                  "CAN27-12"                    },    //-83, 41, -79, 84
	{   "MTM27-12IF",                  "CANQ27-M12"                  },    //-83, 41, -79, 84
//	{   "MTM27-12IF",                  "US-BellSouth27"              },    //-83, 41, -79, 84
//	{   "MTM27-12IF",                  "CAN27-Ont17"                 },    //-83, 41, -79, 84
	{   "MTM27-12IF",                  "CANQ-M12"                    },    //-83, 41, -79, 84
	{   "MTM83-10IF",                  "CANQ-M10"                    },    //-81.5, 41, -77.5, 84
	{   "MTM83-10IF",                  "CAN27-10"                    },    //-81.5, 41, -77.5, 84
	{   "MTM83-10IF",                  "CANQ27-M10"                  },    //-81.5, 41, -77.5, 84
	{   "CSRS.MTM-9",                  "CANQ-M9"                     },    //-78, 45.28, -75, 62.56
	{   "CSRS.MTM-9",                  "CAN27-9"                     },    //-78, 45.28, -75, 62.56
	{   "CSRS.MTM-9",                  "CANQ27-M9"                   },    //-78, 45.28, -75, 62.56
	{   "MTM83-8IF",                   "CANQ27-M8M"                  },    //-75.5, 41, -71.5, 84
	{   "MTM83-8IF",                   "CAN27-8"                     },    //-75.5, 41, -71.5, 84
	{   "MTM83-8IF",                   "CANQ-M8"                     },    //-75.5, 41, -71.5, 84
	{   "CANQ-M7",                     "CANQ27-M7"                   },    //-72.3833, 43.35, -68.6333, 63.4667
	{   "MTM27-5IF",                   "CANNS-5"                     },    //-66.5, 41, -62.5, 84
	{   "MTM27-5IF",                   "CANNS-ATS5"                  },    //-66.5, 41, -62.5, 84
	{   "Barbados1938.BWIgrid",        "STLUCIA"                     },    //-60.71, 11.04, -57.5, 14.75
	{   "Barbados1938.BWIgrid",        "STKITTS"                     },    //-60.71, 11.04, -57.5, 14.75
	{   "Barbados1938.BWIgrid",        "MONTSERAT"                   },    //-60.71, 11.04, -57.5, 14.75
	{   "Barbados1938.BWIgrid",        "DOMINICA"                    },    //-60.71, 11.04, -57.5, 14.75
	{   "Barbados1938.BWIgrid",        "GRENADA"                     },    //-60.71, 11.04, -57.5, 14.75
	{   "Barbados1938.BWIgrid",        "ANTIGUA"                     },    //-60.71, 11.04, -57.5, 14.75
	{   "MTM27-4IF",                   "CANNS-4"                     },    //-63.5, 41, -59.5, 84
	{   "MTM27-4IF",                   "CANNS-ATS4"                  },    //-63.5, 41, -59.5, 84
	{   "MTM27-2IF",                   "CANQ27-M2"                   },    //-57.5, 44, -53.5, 60
//	{   "Zanderij.TM-54NW",            "PARA-4"                      },    //-57.8167, 4.95, -52.0833, 9.75
	{   "ETRS89.TM26",                 "EUET-26"                     },    //-30, 25.1, -24, 65.8
	{   "ETRS89.TM27",                 "ETRF-27"                     },    //-24, 27.6, -18, 66.5
	{   "ETRS89.TM27",                 "EUET-27"                     },    //-24, 27.6, -18, 66.5
	{   "ETRF89.TM28",                 "EU-28"                       },    //-18, 27.6, -12, 66.55
	{   "ETRF89.TM28",                 "EUET-28"                     },    //-18, 27.6, -12, 66.55
	{   "ETRF89.TM28",                 "SPAIN-HU28"                  },    //-18, 27.6, -12, 66.55
	{   "ETRF89.TM28",                 "GUINEA-28D"                  },    //-18, 27.6, -12, 66.55
	{   "ETRF89.TM28",                 "SPAIN-TM28-I"                },    //-18, 27.6, -12, 66.55
	{   "ETRF89.TM28",                 "GUINEA-28C"                  },    //-18, 27.6, -12, 66.55
//	{   "ETRF89.TM28",                 "GUIN-BIS"                    },    //-18, 27.6, -12, 66.55
	{   "ETRF89.TM28",                 "ETRF-28"                     },    //-18, 27.6, -12, 66.55
//	{   "ETRF89.TM29",                 "GUINEA-29C"                  },    //-12, 36, -6, 62.33
//	{   "ETRF89.TM29",                 "GUINEA-29D"                  },    //-12, 36, -6, 62.33
//	{   "ETRF89.TM29",                 "SPAIN-HU29"                  },    //-12, 36, -6, 62.33
//	{   "ETRF89.TM29",                 "IVC-29-L"                    },    //-12, 36, -6, 62.33
	{   "ETRF89.TM29",                 "EU-29"                       },    //-12, 36, -6, 62.33
//	{   "ETRF89.TM29",                 "PRT-U29"                     },    //-12, 36, -6, 62.33
//	{   "ETRF89.TM29",                 "ALG-U29"                     },    //-12, 36, -6, 62.33
//	{   "ETRF89.TM29",                 "FAR-TM"                      },    //-12, 36, -6, 62.33
//	{   "ETRF89.TM29",                 "PRT-U29-D73"                 },    //-12, 36, -6, 62.33
	{   "ETRF89.TM29",                 "ETRF-29"                     },    //-12, 36, -6, 62.33
	{   "ETRF89.TM29",                 "EUET-29"                     },    //-12, 36, -6, 62.33
//	{   "ETRF89.TM29",                 "IVC-29-A"                    },    //-12, 36, -6, 62.33
//	{   "ETRF89.TM30",                 "IVC-30-A"                    },    //-6, 34.75, 0, 62.33
	{   "ETRF89.TM30",                 "EU-30"                       },    //-6, 34.75, 0, 62.33
	{   "ETRF89.TM30",                 "EUET-30"                     },    //-6, 34.75, 0, 62.33
//	{   "ETRF89.TM30",                 "ALG-U30"                     },    //-6, 34.75, 0, 62.33
//	{   "ETRF89.TM30",                 "GIBR-ED50"                   },    //-6, 34.75, 0, 62.33
//	{   "ETRF89.TM30",                 "GIBR-TM"                     },    //-6, 34.75, 0, 62.33
//	{   "ETRF89.TM30",                 "SPAIN-UTM"                   },    //-6, 34.75, 0, 62.33
//	{   "ETRF89.TM30",                 "SPAIN-TM30-I"                },    //-6, 34.75, 0, 62.33
//	{   "ETRF89.TM30",                 "IVC-30-L"                    },    //-6, 34.75, 0, 62.33
//	{   "ETRS89.TM31",                 "DHDN/3.Gauss3d-1"            },    //0, 38.5, 6, 62.33
//	{   "ETRS89.TM31",                 "SPAIN-HU31"                  },    //0, 38.5, 6, 62.33
//	{   "ETRS89.TM31",                 "ALG-U31"                     },    //0, 38.5, 6, 62.33
//	{   "ETRS89.TM31",                 "EUET-31"                     },    //0, 38.5, 6, 62.33
//	{   "ETRS89.TM31",                 "DHDN/BeTA.Gauss3d-1"         },    //0, 38.5, 6, 62.33
	{   "ETRS89.TM31",                 "EU-31"                       },    //0, 38.5, 6, 62.33
//	{   "WGS84.TM-6NE",                "DHDN/BeTA.Gauss3d-2"         },    //1.96667, 1.83333, 9.28333, 6.81667
//	{   "WGS84.TM-6NE",                "GRMNY-S2"                    },    //1.96667, 1.83333, 9.28333, 6.81667
//	{   "ETRF89.TM32",                 "ITALY-U32"                   },    //6, 36.7, 12, 70
//	{   "ETRF89.TM32",                 "DANE-32-ED"                  },    //6, 36.7, 12, 70
//	{   "ETRF89.TM32",                 "ITALY-W"                     },    //6, 36.7, 12, 70
//	{   "ETRF89.TM32",                 "ITALY-W-Rome"                },    //6, 36.7, 12, 70
	{   "ETRF89.TM32",                 "EU-32"                       },    //6, 36.7, 12, 70
//	{   "ETRF89.TM32",                 "ZAIRE"                       },    //6, 36.7, 12, 70
//	{   "ETRF89.TM32",                 "GABON-S"                     },    //6, 36.7, 12, 70
//	{   "ETRF89.TM32",                 "GABON-N"                     },    //6, 36.7, 12, 70
//	{   "ETRF89.TM32",                 "Pulkovo42.GK-2"              },    //6, 36.7, 12, 70
//	{   "ETRF89.TM32",                 "ALG-U32"                     },    //6, 36.7, 12, 70
//	{   "ETRF89.TM32",                 "AN-ML32"                     },    //6, 36.7, 12, 70
//	{   "ETRF89.TM32",                 "GRMNY-S3"                    },    //6, 36.7, 12, 70
	{   "ETRF89.TM32",                 "EUET-32"                     },    //6, 36.7, 12, 70
//	{   "ETRF89.TM32",                 "Pulkovo42.GK/CM-9E"          },    //6, 36.7, 12, 70
//	{   "ETRS89.Kp2K-Jutland",         "Denmark-KP2000-J"            },    //7.7, 54.4333, 11.5833, 58.05
	{   "MGI-AT/Fa.AUT-West/GK",       "ATM-28"                      },    //9.25, 46.6, 12.1167, 47.8
	{   "MGI-AT/Fa.AUT-West/GK",       "MGI/gc.M28"                  },    //9.25, 46.6, 12.1167, 47.8
	{   "MGI-AT/Fa.AUT-West/GK",       "AT-BMN28"                    },    //9.25, 46.6, 12.1167, 47.8
	{   "RT90-3/7Pa.SW-75GONV",        "SW-75GONW"                   },    //10.3333, 56.15, 12.6667, 64.5
//	{   "Camacupa_1.TM-12SE",          "GRMNY-S4"                    },    //9.51667, -18.2833, 14.35, -4.91667
//	{   "Camacupa_1.TM-12SE",          "Denmark-KP2000-Z"            },    //9.51667, -18.2833, 14.35, -4.91667
//	{   "Libyan2006_1.Libya/TM-7",     "RSA-LO13/01"                 },    //12, 22.8, 14, 33
	{   "MGI-AT/a.AUT-Central/GK",     "MGI/gc.M31"                  },    //11.45, 46.05, 15.2, 49.05
	{   "MGI-AT/a.AUT-Central/GK",     "AT-BMN31"                    },    //11.45, 46.05, 15.2, 49.05
	{   "MGI-AT/a.AUT-Central/GK",     "ATM-31"                      },    //11.45, 46.05, 15.2, 49.05
	{   "RT90-3/7Pa.SW-5GONV",         "SW-5GONW"                    },    //12.2333, 54.2167, 14.3, 67.2333
//	{   "DHDN/BeTA.Gauss3d-5",         "HUN-33"                      },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "SERBIA-5"                    },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "SK-3N"                       },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "BOSNIA-5M"                   },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "BOSNIA-5M-S9"                },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "PCS2000-5"                   },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "GK-3N"                       },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "SLOVENIA-5"                  },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "AN-ML33"                     },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "MGI-C.SloveniaD48"           },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "SLOVENIA-5-S9"               },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "MGI-C.SloveniaGrid"          },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "MGI1901.Balkans-5"           },    //13.3, 50.5, 15.2333, 51.6667
	{   "DHDN/BeTA.Gauss3d-5",         "GRMNY-S5"                    },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "EUET-33"                     },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "ITALY-E"                     },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "ITALY-E-Rome"                },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "ITALY-U33"                   },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "NORGE-33"                    },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "Denmark-KP2000-B"            },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "CZ-S42-3"                    },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "CROATIA-5-S9"                },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "Pulkovo42.GK/CM-15E"         },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "DANE-33-ED"                  },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "Pulkovo42.GK-3"              },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "POL-UK33"                    },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "EU-33"                       },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "CROATIA-5"                   },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "RSA-LO15/01"                 },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "DANE-33"                     },    //13.3, 50.5, 15.2333, 51.6667
//	{   "DHDN/BeTA.Gauss3d-5",         "SK-S42-3"                    },    //13.3, 50.5, 15.2333, 51.6667
//	{   "RT90-3/7Pa.SW-25GONV",        "SW-25GONW"                   },    //13.7333, 54.8, 17.3, 69.2
	{   "RT90-3/7Pa.SW-25GONV",        "RT90-3/7Pa.SW-NAT90"         },    //13.7333, 54.8, 17.3, 69.2
	{   "MGI-AT/a.M34/GKa",            "MGI/gc.M34"                  },    //14.5333, 46.25, 17.4667, 49.2667
	{   "MGI-AT/a.M34/GKa",            "AT-BMN34"                    },    //14.5333, 46.25, 17.4667, 49.2667
	{   "MGI-AT/a.M34/GKa",            "ATM-34"                      },    //14.5333, 46.25, 17.4667, 49.2667
//	{   "Libyan2006_1.Libya/TM",       "RSA-LO17/01"                 },    //9.31, 19.51, 25.98, 36
	{   "ETRS89.PolandCS2K-6",         "PCS2000-6"                   },    //16.1333, 48.7833, 19.8833, 55.3833
//	{   "ETRS89.PolandCS2K-6",         "JUGO-6M"                     },    //16.1333, 48.7833, 19.8833, 55.3833
//	{   "ETRS89.PolandCS2K-6",         "CROATIA-6"                   },    //16.1333, 48.7833, 19.8833, 55.3833
//	{   "ETRS89.PolandCS2K-6",         "BOSNIA-6M"                   },    //16.1333, 48.7833, 19.8833, 55.3833
//	{   "ETRS89.PolandCS2K-6",         "BOSNIA-6M-S9"                },    //16.1333, 48.7833, 19.8833, 55.3833
//	{   "ETRS89.PolandCS2K-6",         "MGI1901.Balkans-6"           },    //16.1333, 48.7833, 19.8833, 55.3833
//	{   "ETRS89.PolandCS2K-6",         "CROATIA-6-S9"                },    //16.1333, 48.7833, 19.8833, 55.3833
	{   "RT90-3/7Pa.SW-0GONV",         "SW-0GONW"                    },    //16.6667, 55.7333, 19.4667, 69.75
	{   "Pulkovo/58b.Poland-V",        "POL-UK65-V"                  },    //18.1667, 49.1333, 19.8333, 51.5333
//	{   "ETRS89.FinlandGK-19",         "PSWG92"                      },    //19.09, 60, 19.5, 60.42
//	{   "ETRS89.FinlandGK-19",         "RSA-LO19/01"                 },    //19.09, 60, 19.5, 60.42
	{   "RT90-3/7Pa.SW-25GONO",        "SW-25GONE"                   },    //18.9167, 62.7167, 21.7167, 69.6833
//	{   "BUL-34",                      "BOTS-W"                      },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "GK-4N"                       },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "CZ-S42-4"                    },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "HUN-34"                      },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "RSA-LO21/01"                 },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "SK-4N"                       },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "MACED-7M"                    },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "SK-S42-4"                    },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "JUGO-7M"                     },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "POL-UK34"                    },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "GK-4"                        },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "ALBANIA"                     },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "EU-34"                       },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "LIT-GK4"                     },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "LIT-S42"                     },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "PCS2000-7"                   },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "MACED-7M-S9"                 },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "EST-P42-34"                  },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "ROM-34"                      },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "NORGE-34"                    },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "FINL-KKJ1"                   },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "MGI1901.Balkans-7"           },    //17.25, 46.5833, 24.75, 60.45
//	{   "BUL-34",                      "EUET-34"                     },    //17.25, 46.5833, 24.75, 60.45
	{   "RT90-3/7Pa.SW-5GONO",         "SW-5GONE"                    },    //21.1, 63.8833, 24.5167, 69.1667
//	{   "ELD1979.Libya-12",            "RSA-LO23/01"                 },    //21.75, 18.1667, 24.25, 34.35
//	{   "Lietuvos1994",                "PCS2000-8"                   },    //20.35, 53.6667, 27.5333, 56.7333
	{   "Lietuvos1994",                "LIT-LKS94"                   },    //20.35, 53.6667, 27.5333, 56.7333
//	{   "Lietuvos1994",                "GREECE"                      },    //20.35, 53.6667, 27.5333, 56.7333
//	{   "Lietuvos1994",                "MACED-8M-S9"                 },    //20.35, 53.6667, 27.5333, 56.7333
//	{   "Lietuvos1994",                "JUGO-8M"                     },    //20.35, 53.6667, 27.5333, 56.7333
//	{   "Lietuvos1994",                "EST-TM93"                    },    //20.35, 53.6667, 27.5333, 56.7333
//	{   "Lietuvos1994",                "LV-TM"                       },    //20.35, 53.6667, 27.5333, 56.7333
//	{   "Lietuvos1994",                "MGI1901.Balkans-8"           },    //20.35, 53.6667, 27.5333, 56.7333
//	{   "Lietuvos1994",                "MACED-8M"                    },    //20.35, 53.6667, 27.5333, 56.7333
///	{   "Lietuvos1994",                "FINL-KKJ2"                   },    //20.35, 53.6667, 27.5333, 56.7333
//	{   "Cape-1.SACS-25",              "RSA-LO25/01"                 },    //23.75, -35.2, 26.25, -23.75
//	{   "Hartebeesthoek94.Lo27",       "EG-TM1"                      },    //25.75, -34.8833, 28.25, -21.8167
//	{   "Hartebeesthoek94.Lo27",       "TURK-TM27"                   },    //25.75, -34.8833, 28.25, -21.8167
//	{   "Hartebeesthoek94.Lo27",       "EST-P42-35"                  },    //25.75, -34.8833, 28.25, -21.8167
//	{   "Hartebeesthoek94.Lo27",       "EU-35"                       },    //25.75, -34.8833, 28.25, -21.8167
//	{   "Hartebeesthoek94.Lo27",       "GK-5"                        },    //25.75, -34.8833, 28.25, -21.8167
//	{   "Hartebeesthoek94.Lo27",       "EUET-35"                     },    //25.75, -34.8833, 28.25, -21.8167
//	{   "Hartebeesthoek94.Lo27",       "FINL-YHT"                    },    //25.75, -34.8833, 28.25, -21.8167
//	{   "Hartebeesthoek94.Lo27",       "LIT-GK5"                     },    //25.75, -34.8833, 28.25, -21.8167
//	{   "Hartebeesthoek94.Lo27",       "ROM-35"                      },    //25.75, -34.8833, 28.25, -21.8167
//	{   "Hartebeesthoek94.Lo27",       "FINL-KKJ3"                   },    //25.75, -34.8833, 28.25, -21.8167
//	{   "Hartebeesthoek94.Lo27",       "NORGE-35"                    },    //25.75, -34.8833, 28.25, -21.8167
//	{   "Hartebeesthoek94.Lo27",       "RSA-LO27/01"                 },    //25.75, -34.8833, 28.25, -21.8167
//	{   "Hartebeesthoek94.Lo27",       "BOTS-E"                      },    //25.75, -34.8833, 28.25, -21.8167
//	{   "ETRF89.FinlandGK-29",         "RSA-LO29/01"                 },    //28.5, 60.95, 29.5, 69.83
//	{   "Pulkovo95.GK3d/CM-30E",       "TURK-TM30"                   },    //28.1333, 51.1667, 31.8833, 71.7167
//	{   "Pulkovo95.GK3d/CM-30E",       "FINL-KKJ4"                   },    //28.1333, 51.1667, 31.8833, 71.7167
//	{   "Hartebeesthoek94.Lo31",       "RSA-LO31/01"                 },    //29.75, -32.2167, 32.25, -21.2833
//	{   "Pulkovo95.GK3d/CM-33E",       "MOZ-M-W"                     },    //31.1333, 49.3667, 34.8833, 71.8833
//	{   "Pulkovo95.GK3d/CM-33E",       "EUET-36"                     },    //31.1333, 49.3667, 34.8833, 71.8833
//	{   "Pulkovo95.GK3d/CM-33E",       "TURK-TM33"                   },    //31.1333, 49.3667, 34.8833, 71.8833
//	{   "Pulkovo95.GK3d/CM-33E",       "TETE/b.MOZ-T-W"              },    //31.1333, 49.3667, 34.8833, 71.8833
//	{   "Pulkovo95.GK3d/CM-33E",       "EU-36"                       },    //31.1333, 49.3667, 34.8833, 71.8833
//	{   "Pulkovo95.GK3d/CM-33E",       "RSA-LO33/01"                 },    //31.1333, 49.3667, 34.8833, 71.8833
//	{   "Pulkovo95.GK3d-12",           "TURK-TM36"                   },    //34.1333, 42.0667, 37.8833, 72.5333
//	{   "ED50.Jordan/TM",              "JORDAN-TM"                   },    //34.98, 29.21, 39.33, 33.4
//	{   "Pulkovo42.GK/CM-39E",         "MOZ-M-E"                     },    //35.25, 38.65, 42.75, 71.95
//	{   "Pulkovo42.GK/CM-39E",         "TURK-TM39"                   },    //35.25, 38.65, 42.75, 71.95
//	{   "Pulkovo42.GK/CM-39E",         "EUET-37"                     },    //35.25, 38.65, 42.75, 71.95
//	{   "Pulkovo42.GK/CM-39E",         "YEM-GK7"                     },    //35.25, 38.65, 42.75, 71.95
//	{   "Pulkovo42.GK/CM-39E",         "EU-37"                       },    //35.25, 38.65, 42.75, 71.95
//	{   "Pulkovo42.GK/CM-39E",         "TETE/b.MOZ-T-E"              },    //35.25, 38.65, 42.75, 71.95
//	{   "Pulkovo42.GK3d/CM-42E",       "TURK-TM42"                   },    //40.1333, 38.2167, 43.8833, 71.8
	{   "ETRS89.TM38",                 "EUET-38"                     },    //42, 36.95, 48, 75
//	{   "ETRS89.TM38",                 "TURK-TM45"                   },    //42, 36.95, 48, 75
//	{   "ETRS89.TM38",                 "IRAN-38"                     },    //42, 36.95, 48, 75
//	{   "ETRS89.TM38",                 "YEM-GK8"                     },    //42, 36.95, 48, 75
//	{   "ETRS89.TM38",                 "KU-N"                        },    //42, 36.95, 48, 75
//	{   "ETRS89.TM38",                 "YEM-W"                       },    //42, 36.95, 48, 75
//	{   "ETRS89.TM38",                 "EU-38"                       },    //42, 36.95, 48, 75
//	{   "Pulkovo42.GK3d-17",           "EUET-39"                     },    //49.1333, 34.6833, 52.8833, 77.5667
//	{   "Pulkovo42.GK3d-17",           "UAE-W"                       },    //49.1333, 34.6833, 52.8833, 77.5667
//	{   "Pulkovo42.GK3d-17",           "YEM-U39"                     },    //49.1333, 34.6833, 52.8833, 77.5667
//	{   "Pulkovo42.GK3d-17",           "OMAN-W-P"                    },    //49.1333, 34.6833, 52.8833, 77.5667
//	{   "Pulkovo42.GK3d-17",           "OMAN-W"                      },    //49.1333, 34.6833, 52.8833, 77.5667
//	{   "Pulkovo42.GK3d-17",           "YEM-GK9"                     },    //49.1333, 34.6833, 52.8833, 77.5667
//	{   "Pulkovo42.GK3d-17",           "YEM-E"                       },    //49.1333, 34.6833, 52.8833, 77.5667
//	{   "Pulkovo42.GK3d-17",           "IRAN-39"                     },    //49.1333, 34.6833, 52.8833, 77.5667
//	{   "Pulkovo42.GK/CM-57E",         "OMAN-E-P"                    },    //53.25, 31.8667, 60.75, 88.7333
//	{   "Pulkovo42.GK/CM-57E",         "IRAN-40"                     },    //53.25, 31.8667, 60.75, 88.7333
//	{   "Pulkovo42.GK/CM-57E",         "UAE-E"                       },    //53.25, 31.8667, 60.75, 88.7333
//	{   "Pulkovo42.GK/CM-57E",         "DUB-OLD40"                   },    //53.25, 31.8667, 60.75, 88.7333
//	{   "Pulkovo42.GK/CM-57E",         "OMAN-E"                      },    //53.25, 31.8667, 60.75, 88.7333
//	{   "Pulkovo42.GK/CM-63E",         "PAK-41"                      },    //59.25, 29.9833, 66.75, 88.9167
//	{   "Pulkovo42.GK/CM-63E",         "IRAN-41"                     },    //59.25, 29.9833, 66.75, 88.9167
//	{   "Pulkovo42.GK3d-23",           "IND-42"                      },    //67.1333, 32.5667, 70.8833, 82.1333
//	{   "Pulkovo42.GK3d-23",           "PAK-42"                      },    //67.1333, 32.5667, 70.8833, 82.1333
//	{   "Beijing1954/a.GK3d-25",       "IND-43"                      },    //73.2667, 35.3333, 76.8667, 41.1167
//	{   "Beijing1954/a.GK3d-25",       "PAK-43"                      },    //73.2667, 35.3333, 76.8667, 41.1167
//	{   "Beijing1954/a.GK3d-25",       "CH-13"                       },    //73.2667, 35.3333, 76.8667, 41.1167
//	{   "Beijing1954/a.GK3d-25",       "GK-13"                       },    //73.2667, 35.3333, 76.8667, 41.1167
//	{   "Pulkovo95.GK3d/CM-81E",       "GK-14"                       },    //79.1333, 48.2667, 82.8833, 77.4333
//	{   "Pulkovo95.GK3d/CM-81E",       "CH-14"                       },    //79.1333, 48.2667, 82.8833, 77.4333
//	{   "Pulkovo95.GK3d/CM-81E",       "IND-44"                      },    //79.1333, 48.2667, 82.8833, 77.4333
//	{   "Xian80.GK-15",                "IND-45"                      },    //83.25, 25.1333, 90.75, 51.35
//	{   "Xian80.GK-15",                "B-UTM-W"                     },    //83.25, 25.1333, 90.75, 51.35
//	{   "Xian80.GK-15",                "GK-15"                       },    //83.25, 25.1333, 90.75, 51.35
//	{   "Xian80.GK-15",                "CH-15"                       },    //83.25, 25.1333, 90.75, 51.35
//	{   "Xian80.GK3d-30",              "B-BTM"                       },    //88.1333, 25.2167, 91.8833, 50.5167
//	{   "CH-16-P",                     "IND-46"                      },    //89.25, 47.4, 96.75, 78.6
//	{   "CH-16-P",                     "GK-16"                       },    //89.25, 47.4, 96.75, 78.6
//	{   "CH-16-P",                     "CH-16"                       },    //89.25, 47.4, 96.75, 78.6
//	{   "CH-16-P",                     "B-UTM-E"                     },    //89.25, 47.4, 96.75, 78.6
//	{   "CH-16-P",                     "IN-TM46N"                    },    //89.25, 47.4, 96.75, 78.6
//	{   "CH-16-P",                     "IN-TM46S"                    },    //89.25, 47.4, 96.75, 78.6
//	{   "CH-16-P",                     "MSK42-16"                    },    //89.25, 47.4, 96.75, 78.6
//	{   "CH-16-P",                     "MYAN-W54"                    },    //89.25, 47.4, 96.75, 78.6
//	{   "Xian80.GK3d/CM-99E",          "THAI-W54"                    },    //97.1333, 19.3167, 100.883, 44.8833
//	{   "Xian80.GK3d/CM-99E",          "THAI-W75"                    },    //97.1333, 19.3167, 100.883, 44.8833
//	{   "Xian80.GK3d/CM-99E",          "MYAN-E54"                    },    //97.1333, 19.3167, 100.883, 44.8833
//	{   "Xian80.GK3d/CM-99E",          "IN-TM47S"                    },    //97.1333, 19.3167, 100.883, 44.8833
//	{   "Xian80.GK3d/CM-99E",          "IN-TM47N"                    },    //97.1333, 19.3167, 100.883, 44.8833
//	{   "Xian80.GK3d/CM-99E",          "CH-17"                       },    //97.1333, 19.3167, 100.883, 44.8833
//	{   "Xian80.GK3d/CM-99E",          "MSK42-17"                    },    //97.1333, 19.3167, 100.883, 44.8833
//	{   "Xian80.GK3d/CM-99E",          "GK-17"                       },    //97.1333, 19.3167, 100.883, 44.8833
//	{   "Xian80.GK-18",                "IN-TM48S"                    },    //101.25, 19.45, 108.75, 44.5667
//	{   "Xian80.GK-18",                "GK-18"                       },    //101.25, 19.45, 108.75, 44.5667
//	{   "Xian80.GK-18",                "THAI-E75"                    },    //101.25, 19.45, 108.75, 44.5667
//	{   "Xian80.GK-18",                "IN-TM48N"                    },    //101.25, 19.45, 108.75, 44.5667
//	{   "Xian80.GK-18",                "IN-JAVAW"                    },    //101.25, 19.45, 108.75, 44.5667
//	{   "Xian80.GK-18",                "VIET-48"                     },    //101.25, 19.45, 108.75, 44.5667
//	{   "Xian80.GK-18",                "VIET-GK18"                   },    //101.25, 19.45, 108.75, 44.5667
//	{   "Xian80.GK-18",                "MSK42-18"                    },    //101.25, 19.45, 108.75, 44.5667
//	{   "Xian80.GK-18",                "THAI-E54"                    },    //101.25, 19.45, 108.75, 44.5667
//	{   "Xian80.GK-18",                "CH-18"                       },    //101.25, 19.45, 108.75, 44.5667
	{   "Hanoi1972.GK-106NE",          "VIET-GK-M"                   },    //104.133, 8.16667, 107.883, 12.7167
//	{   "Beijing1954/a.GK3d-37",       "MSK42-19"                    },    //109.133, 15.4833, 112.883, 47.8
//	{   "Beijing1954/a.GK3d-37",       "CH-19"                       },    //109.133, 15.4833, 112.883, 47.8
//	{   "Beijing1954/a.GK3d-37",       "GK-19"                       },    //109.133, 15.4833, 112.883, 47.8
//	{   "Beijing1954/a.GK3d-37",       "IN-TM49S"                    },    //109.133, 15.4833, 112.883, 47.8
//	{   "Beijing1954/a.GK3d-37",       "AMG-49"                      },    //109.133, 15.4833, 112.883, 47.8
//	{   "Beijing1954/a.GK3d-37",       "VIET-GK19"                   },    //109.133, 15.4833, 112.883, 47.8
//	{   "Beijing1954/a.GK3d-37",       "MAL-E_W"                     },    //109.133, 15.4833, 112.883, 47.8
//	{   "Beijing1954/a.GK3d-37",       "IN-TM49N"                    },    //109.133, 15.4833, 112.883, 47.8
//	{   "Beijing1954/a.GK3d-37",       "IN-JAVAC"                    },    //109.133, 15.4833, 112.883, 47.8
//	{   "Beijing1954/a.GK3d-37",       "VIET-49"                     },    //109.133, 15.4833, 112.883, 47.8
//	{   "AMG66-50-Grid",               "IN-TM50N"                    },    //113.25, -36.7667, 120.75, -18.0333
	{   "AMG66-50-Grid",               "AMG-50"                      },    //113.25, -36.7667, 120.75, -18.0333
//	{   "AMG66-50-Grid",               "GK-20"                       },    //113.25, -36.7667, 120.75, -18.0333
//	{   "AMG66-50-Grid",               "TAIWANPH6"                   },    //113.25, -36.7667, 120.75, -18.0333
//	{   "AMG66-50-Grid",               "MSK42-20"                    },    //113.25, -36.7667, 120.75, -18.0333
//	{   "AMG66-50-Grid",               "CH-20"                       },    //113.25, -36.7667, 120.75, -18.0333
//	{   "AMG66-50-Grid",               "MAL-E_E"                     },    //113.25, -36.7667, 120.75, -18.0333
//	{   "AMG66-50-Grid",               "IN-TM50S"                    },    //113.25, -36.7667, 120.75, -18.0333
//	{   "AMG66-50-Grid",               "IN-JAVAE"                    },    //113.25, -36.7667, 120.75, -18.0333
//	{   "PRS92/03.Philippines-2",      "CHINA-EAST"                  },    //118, 6.9, 120, 12.33
//	{   "PRS92/03.Philippines-2",      "TAIWAN1967-TM2-119"          },    //118, 6.9, 120, 12.33
//	{   "PRS92/03.Philippines-2",      "TAIWAN1997-TM2-119"          },    //118, 6.9, 120, 12.33
//	{   "Luzon.Philippines-III",       "TAIWAN"                      },    //119.467, 3.86667, 122.283, 22.2833
//	{   "Luzon.Philippines-III",       "TAIWAN1967-TM2-121"          },    //119.467, 3.86667, 122.283, 22.2833
//	{   "Luzon.Philippines-III",       "TAIWAN-3"                    },    //119.467, 3.86667, 122.283, 22.2833
//	{   "Luzon.Philippines-III",       "TAIWAN1997-TM2-121"          },    //119.467, 3.86667, 122.283, 22.2833
//	{   "Bjing54/a.GK3d/CM-123E",      "GK-21"                       },    //121.133, 20.5, 124.883, 56.55
//	{   "Bjing54/a.GK3d/CM-123E",      "IN-TM51S"                    },    //121.133, 20.5, 124.883, 56.55
//	{   "Bjing54/a.GK3d/CM-123E",      "AMG-51"                      },    //121.133, 20.5, 124.883, 56.55
	{   "Bjing54/a.GK3d/CM-123E",      "CH-21"                       },    //121.133, 20.5, 124.883, 56.55
//	{   "Bjing54/a.GK3d/CM-123E",      "TAIWAN-6"                    },    //121.133, 20.5, 124.883, 56.55
//	{   "Bjing54/a.GK3d/CM-123E",      "IN-TM51N"                    },    //121.133, 20.5, 124.883, 56.55
//	{   "AMG-52-Grid",                 "CH-22"                       },    //125.25, -34.6833, 132.75, -8.51667
//	{   "AMG-52-Grid",                 "IN-TM52S"                    },    //125.25, -34.6833, 132.75, -8.51667
//	{   "AMG-52-Grid",                 "GK-22"                       },    //125.25, -34.6833, 132.75, -8.51667
	{   "AMG-52-Grid",                 "AMG-52"                      },    //125.25, -34.6833, 132.75, -8.51667
//	{   "AMG-52-Grid",                 "IN-TM52N"                    },    //125.25, -34.6833, 132.75, -8.51667
//	{   "Xian80.GK/CM-135E",           "IN-TM53N"                    },    //131.65, 44.6833, 135.117, 48.7333
//	{   "Xian80.GK/CM-135E",           "IN-TM53S"                    },    //131.65, 44.6833, 135.117, 48.7333
//	{   "Xian80.GK/CM-135E",           "GK-23"                       },    //131.65, 44.6833, 135.117, 48.7333
//	{   "Xian80.GK/CM-135E",           "AMG-53"                      },    //131.65, 44.6833, 135.117, 48.7333
//	{   "Xian80.GK/CM-135E",           "CH-23"                       },    //131.65, 44.6833, 135.117, 48.7333
//	{   "AMG-54-Grid",                 "ISG_54-2"                    },    //137.25, -41.9, 144.75, -7.1
//	{   "AMG-54-Grid",                 "ISG_54-2-Grid"               },    //137.25, -41.9, 144.75, -7.1
	{   "AMG-54-Grid",                 "AMG-54"                      },    //137.25, -41.9, 144.75, -7.1
	{   "ISG94-54.3",                  "ISG_54-3-Grid"               },    //141.5, -38, 144.5, -27.5
	{   "ISG94-54.3",                  "ISG_54-3"                    },    //141.5, -38, 144.5, -27.5
	{   "ISG94-55.1",                  "ISG_55-1"                    },    //143.5, -38, 146.5, -27.5
	{   "ISG94-55.1",                  "ISG_55-1-Grid"               },    //143.5, -38, 146.5, -27.5
	{   "AMG66-55-Grid",               "PNG"                         },    //143.25, -60.05, 150.75, 3.6
//	{   "AMG66-55-Grid",               "ISG_55-2-Grid"               },    //143.25, -60.05, 150.75, 3.6
//	{   "AMG66-55-Grid",               "ISG_55-2"                    },    //143.25, -60.05, 150.75, 3.6
	{   "AMG66-55-Grid",               "AMG-55"                      },    //143.25, -60.05, 150.75, 3.6
	{   "ISG94-55.3",                  "ISG_55-3-Grid"               },    //147.5, -38, 150.5, -27.5
	{   "ISG94-55.3",                  "ISG_55-3"                    },    //147.5, -38, 150.5, -27.5
	{   "ISG94-56.1",                  "ISG_56-1-Grid"               },    //149.5, -38, 152.5, -27.5
	{   "ISG94-56.1",                  "ISG_56-1"                    },    //149.5, -38, 152.5, -27.5
//	{   "Pulkovo95.GK3d/CM-153E",      "ISG_56-2"                    },    //151.133, 43.1333, 154.883, 74.6333
//	{   "Pulkovo95.GK3d/CM-153E",      "ISG_56-2-Grid"               },    //151.133, 43.1333, 154.883, 74.6333
//	{   "Pulkovo95.GK3d/CM-153E",      "AMG-56"                      },    //151.133, 43.1333, 154.883, 74.6333
//	{   "Pulkovo42.GK/CM-159E",        "AMG-57"                      },    //155.25, 48.2667, 162.75, 73.0667
//	{   "GK-28",                       "VAN-58"                      },    //161.25, 53.8667, 168.75, 71.4667
//	{   "Pulkovo42.GK3d-57",           "VAN-59"                      },    //169.133, 58.6833, 172.883, 72.1167
	{   "Leigon_1.GhanaMetreGrid",     "GHANA-C"                     },    //-4.55, 0.0666667, 2, 12.1833
	{   "Leigon_1.GhanaMetreGrid",     "GHANA-M"                     },    //-4.55, 0.0666667, 2, 12.1833
	{   "Leigon_1.GhanaMetreGrid",     "GHANA-Y"                     },    //-4.55, 0.0666667, 2, 12.1833
	{   "HARN/HI.HI-1",                "HARN/HI.HI-1Fa"              },    //-156.267, 18.7333, -154.583, 20.4667
	{   "HARN/HI.HI-1",                "HIHP-1"                      },    //-156.267, 18.7333, -154.583, 20.4667
	{   "JGD2011-18",                  "JGD2K-18-7P-C"               },    //133.183, 23.05, 141.317, 28.45
	{   "JGD2011-18",                  "JPNGSI18-7-C"                },    //133.183, 23.05, 141.317, 28.45
	{   "HI-2",                        "HARN/HI.HI-2Fa"              },    //-157.5, 20.5, -155.5, 21.5
	{   "HI-2",                        "HIHP-2"                      },    //-157.5, 20.5, -155.5, 21.5
	{   "OLDHI.Hawaii-3",              "HIHP-3"                      },    //-158.4, 21.15, -157.533, 21.8
	{   "OLDHI.Hawaii-5",              "HIHP-5"                      },    //-160.333, 21.7, -159.967, 22.1
	{   "OLDHI.Hawaii-5",              "HARN/HI.HI-5Fa"              },    //-160.333, 21.7, -159.967, 22.1
	{   "HI-4",                        "HARN/HI.HI-4Fa"              },    //-160, 21.5833, -159, 22.5
	{   "HI-4",                        "HIHP-4"                      },    //-160, 21.5833, -159, 22.5
	{   "HongKong80b.GridSystem",      "HK-TM80"                     },    //113.8, 22.1167, 114.65, 22.6667
	{   "HongKong80b.GridSystem",      "HK-TM63"                     },    //113.8, 22.1167, 114.65, 22.6667
	{   "NSRS11.FL-WF",                "FLHP-WF"                     },    //-83.6, 25.9333, -80.85, 29.9333
	{   "NSRS11.FL-WF",                "FLHP-W"                      },    //-83.6, 25.9333, -80.85, 29.9333
	{   "HARN/FL.FL-EF",               "FLHP-EF"                     },    //-82.6333, 23.7667, -79.6833, 31.4667
	{   "HARN/FL.FL-EF",               "FLHP-E"                      },    //-82.6333, 23.7667, -79.6833, 31.4667
	{   "QND95b.QatarNational",        "QATAR-NG"                    },    //50.6333, 24.3167, 51.75, 26.35
	{   "JGD2011-16",                  "JPNGSI16-7"                  },    //122.05, 23.7167, 126.433, 25.9667
	{   "JGD2011-16",                  "JPNGSI16-Grid"               },    //122.05, 23.7167, 126.433, 25.9667
	{   "JGD2011-16",                  "JGD2K-16-7P"                 },    //122.05, 23.7167, 126.433, 25.9667
	{   "JGD2011-15",                  "JPNGSI15-7"                  },    //125.5, 23.5833, 130.5, 27.4167
	{   "JGD2011-15",                  "JPNGSI15-Grid"               },    //125.5, 23.5833, 130.5, 27.4167
	{   "JGD2011-15",                  "JGD2K-15-7P"                 },    //125.5, 23.5833, 130.5, 27.4167
	{   "JGD2K.PlnRctCS-XVII",         "JGD2K-17-7P"                 },    //129.783, 23.6667, 131.883, 26.4833
	{   "JGD2K.PlnRctCS-XVII",         "JPNGSI17-Grid"               },    //129.783, 23.6667, 131.883, 26.4833
	{   "JGD2K.PlnRctCS-XVII",         "JPNGSI17-7"                  },    //129.783, 23.6667, 131.883, 26.4833
	{   "Tokyo.PlnRctCS-XIV",          "JPNGSI14-Grid"               },    //140.183, 23.05, 143.317, 28.45
	{   "Tokyo.PlnRctCS-XIV",          "JPNGSI14-7"                  },    //140.183, 23.05, 143.317, 28.45
	{   "Tokyo.PlnRctCS-XIV",          "JGD2K-14-7P"                 },    //140.183, 23.05, 143.317, 28.45
	{   "Tokyo.PlnRctCS-XIX",          "JPNGSI19-7"                  },    //141.5, 23.05, 156.5, 28.45
	{   "Tokyo.PlnRctCS-XIX",          "JPNGSI19-Grid"               },    //141.5, 23.05, 156.5, 28.45
	{   "Tokyo.PlnRctCS-XIX",          "JGD2K-19-7P"                 },    //141.5, 23.05, 156.5, 28.45
	{   "HARN/MS.MS-WF/a",             "MSHP-WF/a"                   },    //-91.9167, 30.6, -89.1, 35.4
	{   "HARN/MS.MS-WF/a",             "MSHP-W"                      },    //-91.9167, 30.6, -89.1, 35.4
	{   "MS83-EF",                     "MSHP-E"                      },    //-90.2, 29.5167, -87.85, 35.5167
	{   "MS83-EF",                     "MSHP-EF"                     },    //-90.2, 29.5167, -87.85, 35.5167
	{   "AL-W",                        "HARN/AL.AL-WF"               },    //-88.7333, 29.65, -86.0333, 35.5
	{   "AL-W",                        "ALHP-W"                      },    //-88.7333, 29.65, -86.0333, 35.5
	{   "AL-W",                        "ALHP-WF"                     },    //-88.7333, 29.65, -86.0333, 35.5
	{   "NSRS11.GA-WF",                "GAHP-W"                      },    //-85.9333, 30.1833, -82.6667, 35.4333
	{   "NSRS11.GA-WF",                "GAHP-WF"                     },    //-85.9333, 30.1833, -82.6667, 35.4333
	{   "NSRS07.GA-E",                 "GAHP-EF"                     },    //-83.8, 29.9333, -80.45, 35.1167
	{   "NSRS07.GA-E",                 "GAHP-E"                      },    //-83.8, 29.9333, -80.45, 35.1167
//	{   "Old-Egyp.Red",                "EG-EP"                       },    //28.0167, 20.8167, 37.8833, 34.9333
	{   "Old-Egyp.Red",                "EG-R"                        },    //28.0167, 20.8167, 37.8833, 34.9333
//	{   "Old-Egyp.Red",                "EG-P"                        },    //28.0167, 20.8167, 37.8833, 34.9333
	{   "Old-Egyp.Blue",               "EG-B"                        },    //31.95, 27.3333, 37.45, 31.6667
	{   "NSRS11.AL-E",                 "ALHP-E"                      },    //-87.0333, 30.6, -84.65, 35.4
	{   "NSRS11.AL-E",                 "ALHP-EF"                     },    //-87.0333, 30.6, -84.65, 35.4
	{   "NSRS11.AL-E",                 "HARN/AL.AL-EF"               },    //-87.0333, 30.6, -84.65, 35.4
	{   "NSRS07.AZ-W",                 "AZHP-WIF"                    },    //-115.1, 31.5667, -112.233, 37.5
	{   "NSRS07.AZ-W",                 "AZHP-WF"                     },    //-115.1, 31.5667, -112.233, 37.5
	{   "NSRS07.AZ-W",                 "AZHP-W"                      },    //-115.1, 31.5667, -112.233, 37.5
	{   "NSRS07.AZ-W",                 "HARN/AZ.AZ-WF"               },    //-115.1, 31.5667, -112.233, 37.5
	{   "AZ-C",                        "AZHP-CIF"                    },    //-113.717, 30.7667, -110.083, 37.5667
	{   "AZ-C",                        "AZHP-C"                      },    //-113.717, 30.7667, -110.083, 37.5667
	{   "AZ-C",                        "AZHP-CF"                     },    //-113.717, 30.7667, -110.083, 37.5667
	{   "AZ-C",                        "HARN/AZ.AZ-CF"               },    //-113.717, 30.7667, -110.083, 37.5667
	{   "NSRS11.AZ-EIF",               "AZHP-EIF"                    },    //-112.05, 30.7667, -108.717, 37.5667
	{   "NSRS11.AZ-EIF",               "HARN/AZ.AZ-EF"               },    //-112.05, 30.7667, -108.717, 37.5667
	{   "NSRS11.AZ-EIF",               "AZHP-EF"                     },    //-112.05, 30.7667, -108.717, 37.5667
	{   "NSRS11.AZ-EIF",               "AZHP-E"                      },    //-112.05, 30.7667, -108.717, 37.5667
	{   "NSRS07.NM-WF",                "NMHP-W"                      },    //-109.383, 30.7667, -105.983, 37.5667
	{   "NSRS07.NM-WF",                "NMHP-WF"                     },    //-109.383, 30.7667, -105.983, 37.5667
	{   "NSRS07.NM-CF",                "NMHP-C"                      },    //-108.083, 31.25, -104.483, 37.5167
	{   "NSRS07.NM-CF",                "NMHP-CF"                     },    //-108.083, 31.25, -104.483, 37.5167
	{   "NSRS07.NM-E",                 "NMHP-E"                      },    //-106.05, 31.5, -102.667, 37.5
	{   "NSRS07.NM-E",                 "NMHP-EF"                     },    //-106.05, 31.5, -102.667, 37.5
//	{   "Palestine23a.Belt",           "JORDAN-BTM"                  },    //34.22, 29.19, 39.3, 33.38
//	{   "Palestine23a.Belt",           "ISRAEL-TM"                   },    //34.22, 29.19, 39.3, 33.38
	{   "JGD2011-01",                  "JGD2K-01-7P"                 },    //128.067, 26.1667, 130.467, 35.4833
	{   "JGD2011-01",                  "JPNGSI01-7"                  },    //128.067, 26.1667, 130.467, 35.4833
	{   "JGD2011-01",                  "JPNGSI01-Grid"               },    //128.067, 26.1667, 130.467, 35.4833
	{   "JGD2011-02",                  "JGD2K-02-7P"                 },    //129.333, 30.6167, 132.467, 34.3333
	{   "JGD2011-02",                  "JPNGSI02-7"                  },    //129.333, 30.6167, 132.467, 34.3333
	{   "JGD2011-02",                  "JPNGSI02-Grid"               },    //129.333, 30.6167, 132.467, 34.3333
	{   "JGD2K-04",                    "JPNGSI04-Grid"               },    //131.55, 32.45, 135.217, 34.8
	{   "JGD2K-04",                    "JGD2K-04-7P"                 },    //131.55, 32.45, 135.217, 34.8
	{   "JGD2K-04",                    "JPNGSI04-7"                  },    //131.55, 32.45, 135.217, 34.8
	{   "NV83-W",                      "NVHP-WF"                     },    //-120.383, 36.45, -116.617, 42.5
	{   "NV83-W",                      "NVHP-W"                      },    //-120.383, 36.45, -116.617, 42.5
	{   "NSRS11.NV-CF",                "NVHP-CF"                     },    //-118.583, 35.5, -114.583, 41.4833
	{   "NSRS11.NV-CF",                "NVHP-C"                      },    //-118.583, 35.5, -114.583, 41.4833
	{   "HARN/NV.NV-E",                "NVHP-E"                      },    //-117.383, 34.3, -113.667, 42.7
	{   "HARN/NV.NV-E",                "NVHP-EF"                     },    //-117.383, 34.3, -113.667, 42.7
	{   "HARN/MO.MO-C",                "MOHP-CF"                     },    //-94.0833, 36.0833, -91.1167, 41.0167
	{   "HARN/MO.MO-C",                "MOHP-C"                      },    //-94.0833, 36.0833, -91.1167, 41.0167
	{   "HARN/MO.MO-C",                "HARN/MO.MO-CF"               },    //-94.0833, 36.0833, -91.1167, 41.0167
	{   "NSRS11.MO-E",                 "HARN/MO.MO-EF"               },    //-92.3167, 35.5333, -88.75, 41.0667
	{   "NSRS11.MO-E",                 "MOHP-E"                      },    //-92.3167, 35.5333, -88.75, 41.0667
	{   "NSRS11.MO-E",                 "MOHP-EF"                     },    //-92.3167, 35.5333, -88.75, 41.0667
	{   "JGD2011-03",                  "JPNGSI03-7"                  },    //130.4, 33.4, 133.883, 36.6167
	{   "JGD2011-03",                  "JPNGSI03-Grid"               },    //130.4, 33.4, 133.883, 36.6167
	{   "JGD2011-03",                  "JGD2K-03-7P"                 },    //130.4, 33.4, 133.883, 36.6167
	{   "JGD2011-05",                  "JPNGSI05-7"                  },    //132.833, 33.95, 135.767, 35.8333
	{   "JGD2011-05",                  "JGD2K-05-7P"                 },    //132.833, 33.95, 135.767, 35.8333
	{   "JGD2011-05",                  "JPNGSI05-Grid"               },    //132.833, 33.95, 135.767, 35.8333
	{   "JGD2K.PlnRctCS-VI",           "JPNGSI06-7"                  },    //134.567, 33.1167, 137.267, 36.5833
	{   "JGD2K.PlnRctCS-VI",           "JGD2K-06-7P"                 },    //134.567, 33.1167, 137.267, 36.5833
	{   "JGD2K.PlnRctCS-VI",           "JPNGSI06-Grid"               },    //134.567, 33.1167, 137.267, 36.5833
	{   "JGD2K-07",                    "JPNGSI07-Grid"               },    //136.05, 34.25, 138.05, 37.8333
	{   "JGD2K-07",                    "JPNGSI07-7"                  },    //136.05, 34.25, 138.05, 37.8333
	{   "JGD2K-07",                    "JGD2K-07-7P"                 },    //136.05, 34.25, 138.05, 37.8333
	{   "JGD2K-08",                    "JPNGSI08-7"                  },    //137.167, 33.6333, 140.233, 39.1167
	{   "JGD2K-08",                    "JGD2K-08-7P"                 },    //137.167, 33.6333, 140.233, 39.1167
	{   "JGD2K-08",                    "JPNGSI08-Grid"               },    //137.167, 33.6333, 140.233, 39.1167
	{   "Tokyo.PlnRctCS-IX",           "JGD2K-09-7P"                 },    //138.683, 31.9667, 141.1, 36.2833
	{   "Tokyo.PlnRctCS-IX",           "JPNGSI09-Grid"               },    //138.683, 31.9667, 141.1, 36.2833
	{   "Tokyo.PlnRctCS-IX",           "JPNGSI09-7"                  },    //138.683, 31.9667, 141.1, 36.2833
	{   "MO83-WF",                     "MOHP-WF"                     },    //-96.5, 36, -92.75, 41
	{   "MO83-WF",                     "MOHP-W"                      },    //-96.5, 36, -92.75, 41
	{   "MO83-WF",                     "HARN/MO.MO-WF"               },    //-96.5, 36, -92.75, 41
	{   "IL83-W",                      "ILHP-WF"                     },    //-91.85, 36.4333, -88.6, 43.0667
	{   "IL83-W",                      "ILHP-W"                      },    //-91.85, 36.4333, -88.6, 43.0667
	{   "HARN/IL.IL-EF",               "ILHP-EF"                     },    //-89.55, 36.5167, -86.7333, 43.05
	{   "HARN/IL.IL-EF",               "ILHP-E"                      },    //-89.55, 36.5167, -86.7333, 43.05
	{   "NSRS07.IN-W",                 "INHP-WF"                     },    //-88.3333, 37.3833, -86, 42.1667
	{   "NSRS07.IN-W",                 "INHP-W"                      },    //-88.3333, 37.3833, -86, 42.1667
	{   "HARN/IN.IN-EF",               "INHP-E"                      },    //-86.8167, 37.5833, -84.5667, 42.15
	{   "HARN/IN.IN-EF",               "INHP-EF"                     },    //-86.8167, 37.5833, -84.5667, 42.15
	{   "NSRS07.DE",                   "DEHP"                        },    //-75.8833, 38.3, -74.85, 39.9833
	{   "NSRS07.DE",                   "DEHPF"                       },    //-75.8833, 38.3, -74.85, 39.9833
	{   "NY83-E",                      "NJHP"                        },    //-76.2, 40.4833, -72.9167, 45.4333
	{   "NY83-E",                      "NJHPF"                       },    //-76.2, 40.4833, -72.9167, 45.4333
	{   "NY83-E",                      "NYHP-EF"                     },    //-76.2, 40.4833, -72.9167, 45.4333
	{   "NY83-E",                      "NYHP-E"                      },    //-76.2, 40.4833, -72.9167, 45.4333
	{   "Datum73b.ModPortgGrd",        "PRT-IGC3"                    },    //-9.91667, 36.5, -5.78333, 42.6667
	{   "Datum73b.ModPortgGrd",        "PRT-SCE"                     },    //-9.91667, 36.5, -5.78333, 42.6667
	{   "NSRS07.NY-W",                 "NYHP-WF"                     },    //-80.0667, 41.8333, -77.0667, 43.8
	{   "NSRS07.NY-W",                 "NYHP-W"                      },    //-80.0667, 41.8333, -77.0667, 43.8
	{   "NY83-C",                      "NYHP-CF"                     },    //-78.0833, 41.7667, -74.7333, 44.6333
	{   "NY83-C",                      "NYHP-C"                      },    //-78.0833, 41.7667, -74.7333, 44.6333
	{   "JGD2K.PlnRctCS-X",            "JPNGSI10-7"                  },    //139.183, 37.3333, 142.4, 42.0167
	{   "JGD2K.PlnRctCS-X",            "JPNGSI10-Grid"               },    //139.183, 37.3333, 142.4, 42.0167
	{   "JGD2K.PlnRctCS-X",            "JGD2K-10-7P"                 },    //139.183, 37.3333, 142.4, 42.0167
	{   "NSRS07.WY-W",                 "WYHP-W"                      },    //-111.3, 40.6333, -108.8, 45.0333
	{   "NSRS07.WY-W",                 "WYHP-WF"                     },    //-111.3, 40.6333, -108.8, 45.0333
	{   "NSRS07.WY-WC",                "WYHP-WC"                     },    //-111.5, 40.6, -107.05, 45.4
	{   "NSRS07.WY-WC",                "WYHP-WCF"                    },    //-111.5, 40.6, -107.05, 45.4
	{   "WY83-ECF",                    "WYHP-ECF"                    },    //-108.967, 40.6, -105.667, 45.4
	{   "WY83-ECF",                    "WYHP-EC"                     },    //-108.967, 40.6, -105.667, 45.4
	{   "HARN/WY.WY-EF",               "WYHP-EF"                     },    //-106.617, 40.6, -103.767, 45.4
	{   "HARN/WY.WY-EF",               "WYHP-E"                      },    //-106.617, 40.6, -103.767, 45.4
	{   "NSRS11.RI",                   "RIHP"                        },    //-71.95, 41.2, -71, 42.0833
	{   "NSRS11.RI",                   "RIHPF"                       },    //-71.95, 41.2, -71, 42.0833
	{   "NSRS07.ID-WF",                "IDHP-W"                      },    //-117.6, 41.3, -113.95, 49.7
	{   "NSRS07.ID-WF",                "IDHP-WF"                     },    //-117.6, 41.3, -113.95, 49.7
	{   "HARN/MT.ID-CF",               "IDHP-CF"                     },    //-115.617, 41.6333, -112.35, 46.0667
	{   "HARN/MT.ID-CF",               "IDHP-C"                      },    //-115.617, 41.6333, -112.35, 46.0667
	{   "NSRS07.ID-E",                 "IDHP-EF"                     },    //-113.517, 41.7333, -110.783, 45.0333
	{   "NSRS07.ID-E",                 "IDHP-E"                      },    //-113.517, 41.7333, -110.783, 45.0333
	{   "VT",                          "HARN/NE.VTF"                 },    //-73.6833, 42.5, -71.2667, 45.25
	{   "VT",                          "NSRS07.VTF"                  },    //-73.6833, 42.5, -71.2667, 45.25
	{   "VT",                          "VTHP"                        },    //-73.6833, 42.5, -71.2667, 45.25
	{   "VT",                          "VTHPF"                       },    //-73.6833, 42.5, -71.2667, 45.25
	{   "VT",                          "NSRS11.VTF"                  },    //-73.6833, 42.5, -71.2667, 45.25
	{   "HARN/NE.NHF",                 "NHHPF"                       },    //-72.8, 42.4333, -70.4167, 45.5667
	{   "HARN/NE.NHF",                 "NHHP"                        },    //-72.8, 42.4333, -70.4167, 45.5667
	{   "HARN/ME.ME-W",                "MEHP-W"                      },    //-71.3167, 42.6833, -69.05, 46.9167
	{   "HARN/ME.ME-W",                "MEHP-WF"                     },    //-71.3167, 42.6833, -69.05, 46.9167
	{   "NSRS11.ME-EF",                "MEHP-EF"                     },    //-70.4167, 43.5333, -66.5333, 47.8333
	{   "NSRS11.ME-EF",                "MEHP-E"                      },    //-70.4167, 43.5333, -66.5333, 47.8333
	{   "JGD2K-11",                    "JPNGSI11-Grid"               },    //139.083, 40.9167, 141.483, 45.9167
	{   "JGD2K-11",                    "JPNGSI11-7"                  },    //139.083, 40.9167, 141.483, 45.9167
	{   "JGD2K-11",                    "JGD2K-11-7P"                 },    //139.083, 40.9167, 141.483, 45.9167
	{   "JGD2K.PlnRctCS-XII",          "JGD2K-12-7P"                 },    //141, 41.4833, 143.5, 45.8667
	{   "JGD2K.PlnRctCS-XII",          "JPNGSI12-Grid"               },    //141, 41.4833, 143.5, 45.8667
	{   "JGD2K.PlnRctCS-XII",          "JPNGSI12-7"                  },    //141, 41.4833, 143.5, 45.8667
	{   "JGD2K.PlnRctCS-XIII",         "JGD2K-13"                    },    //142.783, 41.6, 147.45, 44.5833
	{   "JGD2K.PlnRctCS-XIII",         "JPNGSI13-Grid"               },    //142.783, 41.6, 147.45, 44.5833
	{   "JGD2K.PlnRctCS-XIII",         "JGD2K-13-7P"                 },    //142.783, 41.6, 147.45, 44.5833
	{   "JGD2K.PlnRctCS-XIII",         "JPNGSI13-7"                  },    //142.783, 41.6, 147.45, 44.5833
	{   "Luxembourg30b.Gauss",         "LUX-NTF"                     },    //5.63333, 49.4, 6.63333, 50.2833
	{   "NSRS11.CA/Teale",             "HARN.CA/Teale"               },    //-125.733, 31.5833, -112.833, 42.95
//	{   "WORLD-MERCATOR",              "AFRICA-Merc"                 },    //-225, -90, 225, 90
	{   "IND-I/a",                     "IND-I-M"                     },    //56.3, 27.25, 101.95, 36.3333
//	{   "IND-I/a",                     "PAK-I-M"                     },    //56.3, 27.25, 101.95, 36.3333
//	{   "IND-I/a",                     "PAK-I/a"                     },    //56.3, 27.25, 101.95, 36.3333
	{   "IND-IIA/a",                   "IND-IIA-M"                   },    //58.2333, 20.3, 84.6333, 28.7
//	{   "IND-IIA/a",                   "PAK-IIA/a"                   },    //58.2333, 20.3, 84.6333, 28.7
//	{   "IND-IIA/a",                   "PAK-IIA-M"                   },    //58.2333, 20.3, 84.6333, 28.7
	{   "IND-IVA/a",                   "IND-IVA-M"                   },    //73.0833, 7.3, 81.2167, 15.7
	{   "IND-IIIA/a",                  "IND-IIIA-M"                  },    //67.9833, 14.4, 89.1167, 21.6
//	{   "IND-IIB/a",                   "B-IIB/a"                     },    //79.5, 20.3, 104.5, 28.7
//	{   "IND-IIB/a",                   "B-IIB-M"                     },    //79.5, 20.3, 104.5, 28.7
//	{   "IND-IIB/a",                   "IND-IIB-M"                   },    //79.5, 20.3, 104.5, 28.7
	{   "SAD1969.BzPolyconic/01",      "SA1969.BzPolyconic"          },    //-64.5167, -39.9833, -25.0833, 11.3167
	{   "ETRS89.Europe/EqArea",        "ETRS-LAEA"                   },    //-10.67, 34.5, 31.55, 71.05
	{   "WGS84.PlateCarree",           "WORLD-EQDIST-CYL"            },    //-225, -90, 225, 90
	{   "Trinidad1903.Trinidad",       "Trinidad"                    },    //-62.3333, 9.66667, -59.7333, 11.6667
	{   "Trinidad1903.Trinidad",       "Trinidad-CL"                 },    //-62.3333, 9.66667, -59.7333, 11.6667
	{   "Trinidad1903.Trinidad",       "Trinidad-CF"                 },    //-62.3333, 9.66667, -59.7333, 11.6667
//	{   "Palestine23a.Grid",           "ISRAEL"                      },    //34.22, 29.19, 39.3, 33.38
//	{   "Palestine23a.Grid",           "ISRAEL-MOD"                  },    //34.22, 29.19, 39.3, 33.38
//	{   "Palestine23a.Grid",           "ISRAEL-PALES"                },    //34.22, 29.19, 39.3, 33.38
//	{   "Palestine23a.Grid",           "JORDAN"                      },    //34.22, 29.19, 39.3, 33.38
	{   "CH1903.LV03/01",              "SWISS-PLUS"                  },    //5.4, 45.6333, 11.05, 48
	{   "Netherlands-RDNew",           "Netherlands-RD"              },    //2.7, 50.45, 7.75, 54.05
	{   "Jamaica1969.NtlGrid",         "Jamaica"                     },    //-78.4, 17.65, -76.1, 18.6
	{   "DeirEzZor_2.Syria",           "SYR-LAM"                     },    //34.1833, 31.8167, 43.2833, 37.7833
	{   "SAMOA",                       "SAMOA-LAM"                   },    //-180, -14.7667, -160, -13.7667
	{   "NSRS07.CA-IIF",               "CAHP-IIF"                    },    //-124.617, 37.8167, -118.983, 40.3667
	{   "NSRS07.CA-IIF",               "CAHP-II"                     },    //-124.617, 37.8167, -118.983, 40.3667
	{   "NSRS11.CA-I",                 "CAHP-IF"                     },    //-125, 39.35, -119.45, 42.2333
	{   "NSRS11.CA-I",                 "CAHP-I"                      },    //-125, 39.35, -119.45, 42.2333
	{   "HARN/WO.WA-NF",               "WAHP-N"                      },    //-125.767, 46.8833, -116.067, 49.25
	{   "HARN/WO.WA-NF",               "WAHP-NF"                     },    //-125.767, 46.8833, -116.067, 49.25
	{   "NSRS11.CA-III",               "CAHP-III"                    },    //-123.667, 36.55, -117.2, 38.9
	{   "NSRS11.CA-III",               "CAHPIIIF"                    },    //-123.667, 36.55, -117.2, 38.9
	{   "HARN/WO.OR-S",                "ORHP-SIF"                    },    //-125.55, 41.7333, -115.933, 44.8167
	{   "HARN/WO.OR-S",                "ORHP-SF"                     },    //-125.55, 41.7333, -115.933, 44.8167
	{   "HARN/WO.OR-S",                "HARN/WO.OR-SF"               },    //-125.55, 41.7333, -115.933, 44.8167
	{   "HARN/WO.OR-S",                "ORHP-S"                      },    //-125.55, 41.7333, -115.933, 44.8167
	{   "NSRS11.OregonLambert",        "HARN.OregonLambert"          },    //-125.617, 41.5833, -115.45, 46.6833
	{   "NSRS11.OregonLambert",        "HARN.OregonLambert.ft"       },    //-125.617, 41.5833, -115.45, 46.6833
	{   "HARN/WO.OR-N",                "HARN/WO.OR-NF"               },    //-125.133, 43.7167, -115.5, 46.4833
	{   "HARN/WO.OR-N",                "ORHP-N"                      },    //-125.133, 43.7167, -115.5, 46.4833
	{   "HARN/WO.OR-N",                "ORHP-NF"                     },    //-125.133, 43.7167, -115.5, 46.4833
	{   "HARN/WO.OR-N",                "ORHP-NIF"                    },    //-125.133, 43.7167, -115.5, 46.4833
	{   "WA83-S",                      "WAHP-S"                      },    //-125.333, 45.3333, -115.983, 47.8167
	{   "WA83-S",                      "WAHP-SF"                     },    //-125.333, 45.3333, -115.983, 47.8167
	{   "CA-IV",                       "CAHP-IV"                     },    //-122.8, 35.6167, -114.833, 37.75
	{   "CA-IV",                       "CAHP-IVF"                    },    //-122.8, 35.6167, -114.833, 37.75
	{   "HARN/CA.CA-VF",               "CAHP-V"                      },    //-122.333, 32.4667, -113.217, 36.1167
	{   "HARN/CA.CA-VF",               "CAHP-VF"                     },    //-122.333, 32.4667, -113.217, 36.1167
	{   "HARN/CA.CAVIF",               "CAHPVI"                      },    //-118.617, 32.3833, -113.95, 34.2333
	{   "HARN/CA.CAVIF",               "CAHPVIF"                     },    //-118.617, 32.3833, -113.95, 34.2333
	{   "NSRS11.UT-SIF",               "UTHP-S"                      },    //-114.683, 36.8333, -108.417, 38.7333
	{   "NSRS11.UT-SIF",               "UTHP-SIF"                    },    //-114.683, 36.8333, -108.417, 38.7333
	{   "NSRS11.UT-SIF",               "UTHP-SF"                     },    //-114.683, 36.8333, -108.417, 38.7333
	{   "HARN/UT.UT-CIF",              "UTHP-CF"                     },    //-114.683, 38.2333, -108.433, 41.3333
	{   "HARN/UT.UT-CIF",              "UTHP-CIF"                    },    //-114.683, 38.2333, -108.433, 41.3333
	{   "HARN/UT.UT-CIF",              "UTHP-C"                      },    //-114.683, 38.2333, -108.433, 41.3333
	{   "NSRS07.UT-NIF",               "UTHP-NF"                     },    //-114.683, 40.4167, -108.433, 42.15
	{   "NSRS07.UT-NIF",               "UTHP-NIF"                    },    //-114.683, 40.4167, -108.433, 42.15
	{   "NSRS07.UT-NIF",               "UTHP-N"                      },    //-114.683, 40.4167, -108.433, 42.15
	{   "NSRS07.MT",                   "MTHP"                        },    //-117.55, 43.9, -102.517, 49.4667
	{   "NSRS07.MT",                   "MTHPF"                       },    //-117.55, 43.9, -102.517, 49.4667
	{   "NSRS07.MT",                   "HARN/MT.MTF"                 },    //-117.55, 43.9, -102.517, 49.4667
	{   "NSRS07.MT",                   "MTHPIF"                      },    //-117.55, 43.9, -102.517, 49.4667
	{   "CO83-SF",                     "COHP-S"                      },    //-109.933, 36.8333, -101.167, 38.8333
	{   "NSRS11.CO-C",                 "COHPCF"                      },    //-109.933, 37.95, -101.183, 40.2833
	{   "NSRS11.CO-C",                 "COHP-C"                      },    //-109.933, 37.95, -101.183, 40.2833
	{   "NSRS07.CO-NF",                "COHP-NF"                     },    //-109.933, 39.4167, -101.183, 41.15
	{   "NSRS07.CO-NF",                "COHP-N"                      },    //-109.933, 39.4167, -101.183, 41.15
	{   "NSRS07.TX-NF",                "TXHP-N"                      },    //-103.417, 34.1, -99.6167, 36.7
	{   "NSRS07.TX-NF",                "TXHP-NF"                     },    //-103.417, 34.1, -99.6167, 36.7
	{   "ND83-SF",                     "NDHP-S"                      },    //-104.5, 45.5, -96, 49
	{   "ND83-SF",                     "HARN/ND.ND-SF"               },    //-104.5, 45.5, -96, 49
	{   "ND83-SF",                     "NDHP-SF"                     },    //-104.5, 45.5, -96, 49
	{   "ND83-SF",                     "NDHP-SIF"                    },    //-104.5, 45.5, -96, 49
	{   "ND83-N",                      "NDHP-NIF"                    },    //-104.95, 46.9833, -95.9333, 49.1833
	{   "ND83-N",                      "NDHP-NF"                     },    //-104.95, 46.9833, -95.9333, 49.1833
	{   "ND83-N",                      "NDHP-N"                      },    //-104.95, 46.9833, -95.9333, 49.1833
	{   "ND83-N",                      "HARN/ND.ND-NF"               },    //-104.95, 46.9833, -95.9333, 49.1833
	{   "NSRS07.TX-CF",                "TXHP-C"                      },    //-108.3, 29.5333, -91.8667, 32.5
	{   "NSRS07.TX-CF",                "TXHP-CF"                     },    //-108.3, 29.5333, -91.8667, 32.5
	{   "SD83-S",                      "SDHP-SF"                     },    //-105.017, 42.2667, -95.4833, 45.0167
	{   "SD83-S",                      "SDHP-S"                      },    //-105.017, 42.2667, -95.4833, 45.0167
	{   "TX-DOT27",                    "TX-DOT83"                    },    //-108.267, 24.7667, -91.8667, 37.5667
	{   "NSRS07.NB",                   "NE-HP"                       },    //-105.15, 39.7, -94.2333, 43.3
	{   "NSRS07.NB",                   "NE-HPF"                      },    //-105.15, 39.7, -94.2333, 43.3
	{   "NSRS11.SD-N",                 "SDHP-N"                      },    //-105.017, 43.9667, -95.5, 46.1167
	{   "NSRS11.SD-N",                 "SDHP-NF"                     },    //-105.017, 43.9667, -95.5, 46.1167
	{   "NSRS11.TX-SC",                "TXHP-SCF"                    },    //-106.4, 27.4833, -92.3667, 30.9667
	{   "NSRS11.TX-SC",                "TXHP-SC"                     },    //-106.4, 27.4833, -92.3667, 30.9667
	{   "NSRS11.TX-SF",                "TXHP-S"                      },    //-100.617, 25.6, -96.4333, 28.4333
	{   "NSRS11.TX-SF",                "TXHP-SF"                     },    //-100.617, 25.6, -96.4333, 28.4333
	{   "NSRS11.TX-NC",                "TXHP-NCF"                    },    //-104.2, 31.4333, -92.8667, 34.8667
	{   "NSRS11.TX-NC",                "TXHP-NC"                     },    //-104.2, 31.4333, -92.8667, 34.8667
	{   "NSRS07.KS-SF",                "KSHP-S"                      },    //-102.983, 36.8, -93.6833, 39.05
	{   "NSRS07.KS-SF",                "KSHP-SF"                     },    //-102.983, 36.8, -93.6833, 39.05
	{   "OK83-SF",                     "OKHP-S"                      },    //-100.7, 33.4333, -93.7333, 35.75
	{   "OK83-SF",                     "OKHP-SF"                     },    //-100.7, 33.4333, -93.7333, 35.75
	{   "OK-N",                        "OKHP-NF"                     },    //-104.067, 35.1, -93.3667, 37.1667
	{   "OK-N",                        "OKHP-N"                      },    //-104.067, 35.1, -93.3667, 37.1667
	{   "HARN/KS.KS-N",                "KSHP-N"                      },    //-102.983, 38.3667, -93.6667, 40.15
	{   "HARN/KS.KS-N",                "KSHP-NF"                     },    //-102.983, 38.3667, -93.6667, 40.15
	{   "MN83-CF",                     "MNHP-C"                      },    //-97.4167, 45.0667, -91.7167, 47.7
	{   "MN83-CF",                     "MNHP-CF"                     },    //-97.4167, 45.0667, -91.7167, 47.7
	{   "NSRS11.MN-SF",                "MNHP-S"                      },    //-97.55, 43.2833, -90.5167, 45.8
	{   "NSRS11.MN-SF",                "MNHP-SF"                     },    //-97.55, 43.2833, -90.5167, 45.8
	{   "IA83-SF",                     "IAHP-S"                      },    //-96.8833, 40.2, -89.3833, 42.2
	{   "IA83-SF",                     "IAHP-SF"                     },    //-96.8833, 40.2, -89.3833, 42.2
	{   "HARN/IA.IA-N",                "IAHP-N"                      },    //-97.45, 41.6833, -89.3333, 43.6667
	{   "HARN/IA.IA-N",                "IAHP-NF"                     },    //-97.45, 41.6833, -89.3333, 43.6667
	{   "MN83-NF",                     "MNHP-N"                      },    //-98.1833, 46.3833, -88.5167, 49.65
	{   "MN83-NF",                     "MNHP-NF"                     },    //-98.1833, 46.3833, -88.5167, 49.65
	{   "NSRS11.LA-NF",                "LAHP-N"                      },    //-94.4333, 30.6333, -90.4667, 33.2333
	{   "NSRS11.LA-NF",                "LAHP-NF"                     },    //-94.4333, 30.6333, -90.4667, 33.2333
	{   "NSRS07.AR-SF",                "ARHP-S"                      },    //-94.9833, 32.7833, -89.8833, 35.3167
	{   "NSRS07.AR-SF",                "ARHP-SF"                     },    //-94.9833, 32.7833, -89.8833, 35.3167
	{   "AR-N",                        "ARHP-NF"                     },    //-95.2333, 34.5, -89.0333, 36.6667
	{   "AR-N",                        "ARHP-N"                      },    //-95.2333, 34.5, -89.0333, 36.6667
	{   "LA83-OF",                     "LAHP-O"                      },    //-94.7, 28.4333, -88.1, 33.4333
	{   "LA83-OF",                     "LAHP-OF"                     },    //-94.7, 28.4333, -88.1, 33.4333
	{   "LA83-OF",                     "HARN/LA.LA-OF"               },    //-94.7, 28.4333, -88.1, 33.4333
	{   "LA83-OF",                     "HARN/LA.LA-O"                },    //-94.7, 28.4333, -88.1, 33.4333
	{   "NSRS07.LA-S",                 "LAHP-S"                      },    //-94.5833, 28.6333, -88.1167, 31.2833
	{   "NSRS07.LA-S",                 "LAHP-SF"                     },    //-94.5833, 28.6333, -88.1167, 31.2833
	{   "NSRS07.WI-SF",                "WIHP-S"                      },    //-91.9833, 42.3, -86.4, 44.5167
	{   "NSRS07.WI-SF",                "WIHP-SF"                     },    //-91.9833, 42.3, -86.4, 44.5167
	{   "NSRS11.WI-C",                 "WIHP-C"                      },    //-93.7167, 43.8167, -85.4167, 45.9833
	{   "NSRS11.WI-C",                 "WIHP-CF"                     },    //-93.7167, 43.8167, -85.4167, 45.9833
	{   "HARN/WI.WI-NF",               "WIHP-N"                      },    //-93.5, 45.1833, -87.45, 47.5
	{   "HARN/WI.WI-NF",               "WIHP-NF"                     },    //-93.5, 45.1833, -87.45, 47.5
	{   "ILLIMAP",                     "ILDNR-M"                     },    //-93, 35, -86, 43
	{   "ILLIMAP",                     "ILDNR-F"                     },    //-93, 35, -86, 43
	{   "NSRS11.MI-NIF",               "MIHP-N"                      },    //-91.2833, 44.7667, -82.5833, 48.6333
	{   "NSRS11.MI-NIF",               "MIHP-NF"                     },    //-91.2833, 44.7667, -82.5833, 48.6333
	{   "NSRS11.MI-NIF",               "MIHP-NIF"                    },    //-91.2833, 44.7667, -82.5833, 48.6333
	{   "NSRS11.MI-NIF",               "HARN/MI.MI-NF"               },    //-91.2833, 44.7667, -82.5833, 48.6333
	{   "HARN/TN.TN",                  "TNHPF"                       },    //-91.4, 34.8333, -80.5833, 36.85
	{   "HARN/TN.TN",                  "TNHP"                        },    //-91.4, 34.8333, -80.5833, 36.85
	{   "KY83-S",                      "KYHP-SF"                     },    //-90.5167, 36.3333, -81.0167, 38.3667
	{   "KY83-S",                      "KYHP-S"                      },    //-90.5167, 36.3333, -81.0167, 38.3667
	{   "BellSouth27",                 "US-BellSouth"                },    //-122, 24, -74, 40
	{   "NSRS07.FL-NF",                "FLHP-N"                      },    //-88.3333, 29.0333, -81.3333, 31.1833
	{   "NSRS07.FL-NF",                "FLHP-NF"                     },    //-88.3333, 29.0333, -81.3333, 31.1833
	{   "HARN/MI.MI-S",                "MIHP-SIF"                    },    //-87.8333, 41.45, -81.5, 44.4667
	{   "HARN/MI.MI-S",                "MIHP-S"                      },    //-87.8333, 41.45, -81.5, 44.4667
	{   "HARN/MI.MI-S",                "HARN/MI.MI-SF"               },    //-87.8333, 41.45, -81.5, 44.4667
	{   "HARN/MI.MI-S",                "MIHP-SF"                     },    //-87.8333, 41.45, -81.5, 44.4667
	{   "HARN/MI.MI-CIF",              "MIHP-CIF"                    },    //-87.65, 43.6, -81.6667, 46.1333
	{   "HARN/MI.MI-CIF",              "MIHP-C"                      },    //-87.65, 43.6, -81.6667, 46.1333
	{   "HARN/MI.MI-CIF",              "HARN/MI.MI-CF"               },    //-87.65, 43.6, -81.6667, 46.1333
	{   "HARN/MI.MI-CIF",              "MIHP-CF"                     },    //-87.65, 43.6, -81.6667, 46.1333
	{   "HARN/KY.KY-NF",               "KYHP-N"                      },    //-86.4, 37.5667, -82.05, 39.2833
	{   "HARN/KY.KY-NF",               "KYHP-NF"                     },    //-86.4, 37.5667, -82.05, 39.2833
	{   "OH-S",                        "OHHP-S"                      },    //-85.3167, 38.2, -80.1833, 40.55
	{   "OH-S",                        "OHHP-SF"                     },    //-85.3167, 38.2, -80.1833, 40.55
	{   "OH83-NF",                     "OHHP-NF"                     },    //-85.3167, 39.8833, -79.9833, 42.55
	{   "OH83-NF",                     "OHHP-N"                      },    //-85.3167, 39.8833, -79.9833, 42.55
	{   "NSRS11.SCIF",                 "SCHP"                        },    //-83.95, 31.7333, -77.9333, 35.5333
	{   "NSRS11.SCIF",                 "SCHPF"                       },    //-83.95, 31.7333, -77.9333, 35.5333
	{   "NSRS11.SCIF",                 "SCHPIF"                      },    //-83.95, 31.7333, -77.9333, 35.5333
	{   "NSRS11.SCIF",                 "HARN/SC.SCF"                 },    //-83.95, 31.7333, -77.9333, 35.5333
	{   "NSRS11.WV-S",                 "WVHP-SF"                     },    //-83.1, 37, -78.6167, 39.35
	{   "NSRS11.WV-S",                 "WVHP-S"                      },    //-83.1, 37, -78.6167, 39.35
	{   "NSRS11.WV-NF",                "WVHP-N"                      },    //-82.2667, 38.5667, -77.2333, 40.8333
	{   "NSRS11.WV-NF",                "WVHP-NF"                     },    //-82.2667, 38.5667, -77.2333, 40.8333
	{   "NSRS07.NC",                   "NCHPF"                       },    //-85.4333, 33.55, -74.2667, 36.8667
	{   "NSRS07.NC",                   "NCHP"                        },    //-85.4333, 33.55, -74.2667, 36.8667
	{   "HARN/VA.VA-SF",               "VAHP-S"                      },    //-84.7333, 36.3667, -74.2667, 38.45
	{   "HARN/VA.VA-SF",               "VAHP-SF"                     },    //-84.7333, 36.3667, -74.2667, 38.45
	{   "VA-N",                        "VAHP-N"                      },    //-80.5, 37.6167, -76.0667, 39.6333
	{   "VA-N",                        "VAHP-NF"                     },    //-80.5, 37.6167, -76.0667, 39.6333
	{   "PA83-S",                      "PAHP-SF"                     },    //-81.25, 39.5667, -74, 41.3167
	{   "PA83-S",                      "PAHP-S"                      },    //-81.25, 39.5667, -74, 41.3167
	{   "NSRS07.PA-N",                 "PAHP-N"                      },    //-81.25, 40.4167, -73.9667, 42.7167
	{   "NSRS07.PA-N",                 "PAHP-NF"                     },    //-81.25, 40.4167, -73.9667, 42.7167
	{   "NSRS07.MDF",                  "MDHPF"                       },    //-80.05, 37.8, -74.4167, 39.9
	{   "NSRS07.MDF",                  "MDHP"                        },    //-80.05, 37.8, -74.4167, 39.9
	{   "NY83-LI",                     "NYHP-LI"                     },    //-74.3333, 40.4667, -71.5167, 41.2833
	{   "NY83-LI",                     "NYHP-LIF"                    },    //-74.3333, 40.4667, -71.5167, 41.2833
	{   "NSRS07.CTF",                  "CTHPF"                       },    //-73.9667, 40.8833, -71.55, 42.15
	{   "NSRS07.CTF",                  "CTHP"                        },    //-73.9667, 40.8833, -71.55, 42.15
	{   "HARN/NE.MAF",                 "MAHPF"                       },    //-73.95, 41.3167, -69.4, 43.0333
	{   "HARN/NE.MAF",                 "MAHP"                        },    //-73.95, 41.3167, -69.4, 43.0333
	{   "HARN/NE.MA-IS",               "MAHP-ISF"                    },    //-71.0333, 41.1667, -69.7667, 41.5333
	{   "HARN/NE.MA-IS",               "MAHP-IS"                     },    //-71.0333, 41.1667, -69.7667, 41.5333
	{   "NAD27.QuebecLambert",         "CANQ-LCC-27"                 },    //-82.7, 43.2333, -54.2667, 64.3667
	{   "NSRS07.PRHP",                 "PRHP"                        },    //-68.3833, 17.5333, -64.0833, 18.6667
	{   "NSRS07.PRHP",                 "PRHPF"                       },    //-68.3833, 17.5333, -64.0833, 18.6667
	{   "NSRS07.PRHP",                 "HARN/PV.PRHPF"               },    //-68.3833, 17.5333, -64.0833, 18.6667
//	{   "PSAD56.IcnRegional",          "VEN-ICN"                     },    //-75.0833, -0.516667, -58.1, 13.4167
	{   "Belge72/b.Lambert72A",        "Belge1972-LS"                },    //2.05, 49.3167, 6.88333, 51.7
	{   "ETRF89.Europe/Lambert",       "ETRS-LCC"                    },    //-10.67, 34.5, 31.55, 71.05
	{   "MGI.Austria",                 "MGI-AT/a.AustriaLambert"     },    //9.53, 46.41, 17.17, 49.02
	{   "MGI.Austria",                 "AT-LCC"                      },    //9.53, 46.41, 17.17, 49.02
	{   "MGI.Austria",                 "MGI/gc.AustriaLambert"       },    //9.53, 46.41, 17.17, 49.02
	{   "Estonia97.Estonia",           "EST-97"                      },    //21.0667, 57.35, 28.7667, 59.9167
	{   "Estonia97.Estonia",           "EST-LAMB"                    },    //21.0667, 57.35, 28.7667, 59.9167
	{   "VICT",                        "VICT-Grid"                   },    //139.817, -39.6667, 151.117, -33.4833
	{   "Belgium1972",                 "Belge1972-LB"                },    //2, 49.5, 7, 52
	{   "HARN/WI.BurnettWI-M",         "BurnettWI-M"                 },    //-93.1244, 45.5292, -91.7911, 46.2681
	{   "HARN/WI.BurnettWI-IF",        "BurnettWI-F"                 },    //-93.1244, 45.5292, -91.7911, 46.2681
	{   "HARN/WI.BurnettWI-IF",        "BurnettWI-IF"                },    //-93.1244, 45.5292, -91.7911, 46.2681
	{   "HARN/WI.PepinWI-M",           "PierceWI-M"                  },    //-92.8944, 44.4083, -91.5611, 44.8639
	{   "HARN/WI.PepinWI-M",           "PepinWI-M"                   },    //-92.8944, 44.4083, -91.5611, 44.8639
	{   "HARN/WI.PepinWI-F",           "PepinWI-F"                   },    //-92.8944, 44.4083, -91.5611, 44.8639
	{   "HARN/WI.PepinWI-F",           "PierceWI-F"                  },    //-92.8944, 44.4083, -91.5611, 44.8639
	{   "HARN/WI.PepinWI-IF",          "PepinWI-IF"                  },    //-92.8944, 44.4083, -91.5611, 44.8639
	{   "HARN/WI.PepinWI-IF",          "PierceWI-IF"                 },    //-92.8944, 44.4083, -91.5611, 44.8639
	{   "HARN/WI.WashburnWI-M",        "WashburnWI-M"                },    //-92.45, 45.5833, -91.1167, 46.3389
	{   "HARN/WI.WashburnWI-F",        "WashburnWI-F"                },    //-92.45, 45.5833, -91.1167, 46.3389
	{   "HARN/WI.WashburnWI-IF",       "WashburnWI-IF"               },    //-92.45, 45.5833, -91.1167, 46.3389
	{   "HARN/WI.ChippewaWI-M",        "ChippewaWI-M"                },    //-91.9611, 44.65, -90.6278, 45.3056
	{   "HARN/WI.ChippewaWI-F",        "ChippewaWI-IF"               },    //-91.9611, 44.65, -90.6278, 45.3056
	{   "HARN/WI.ChippewaWI-F",        "ChippewaWI-F"                },    //-91.9611, 44.65, -90.6278, 45.3056
	{   "HARN/WI.EauClaireWI-M",       "EauClaireWI-M"               },    //-91.9556, 44.5889, -90.6222, 45.1556
	{   "HARN/WI.EauClaireWI-F",       "EauClaireWI-F"               },    //-91.9556, 44.5889, -90.6222, 45.1556
	{   "HARN/WI.EauClaireWI-IF",      "EauClaireWI-IF"              },    //-91.9556, 44.5889, -90.6222, 45.1556
	{   "HARN/WI.BayfieldWI-M",        "BayfieldWI-M"                },    //-91.8169, 46.1583, -90.4861, 47.1806
	{   "HARN/WI.BayfieldWI-F",        "BayfieldWI-F"                },    //-91.8169, 46.1583, -90.4861, 47.1806
	{   "HARN/WI.BayfieldWI-IF",       "BayfieldWI-IF"               },    //-91.8169, 46.1583, -90.4861, 47.1806
	{   "HARN/WI.SawyerWI-M",          "SawyerWI-M"                  },    //-91.7833, 45.5389, -90.45, 46.2611
	{   "HARN/WI.SawyerWI-F",          "SawyerWI-F"                  },    //-91.7833, 45.5389, -90.45, 46.2611
	{   "HARN/WI.SawyerWI-IF",         "SawyerWI-IF"                 },    //-91.7833, 45.5389, -90.45, 46.2611
	{   "HARN/WI.VernonWI-M",          "VernonWI-M"                  },    //-91.45, 43.3583, -90.1167, 43.7917
	{   "HARN/WI.VernonWI-F",          "VernonWI-F"                  },    //-91.45, 43.3583, -90.1167, 43.7917
	{   "HARN/WI.VernonWI-IF",         "VernonWI-IF"                 },    //-91.45, 43.3583, -90.1167, 43.7917
	{   "HARN/WI.JacksonWI-M",         "JacksonWI-M"                 },    //-91.4056, 44.0361, -90.0722, 44.5472
	{   "HARN/WI.JacksonWI-F",         "JacksonWI-F"                 },    //-91.4056, 44.0361, -90.0722, 44.5472
	{   "HARN/WI.JacksonWI-IF",        "JacksonWI-IF"                },    //-91.4056, 44.0361, -90.0722, 44.5472
	{   "HARN/WI.MonroeWI-M",          "MonroeWI-M"                  },    //-91.3083, 43.6778, -89.975, 44.3222
	{   "HARN/WI.MonroeWI-F",          "MonroeWI-F"                  },    //-91.3083, 43.6778, -89.975, 44.3222
	{   "HARN/WI.MonroeWI-IF",         "MonroeWI-IF"                 },    //-91.3083, 43.6778, -89.975, 44.3222
	{   "HARN/WI.TaylorWI-M",          "TaylorWI-M"                  },    //-91.15, 44.9333, -89.8167, 45.4222
	{   "HARN/WI.TaylorWI-F",          "TaylorWI-F"                  },    //-91.15, 44.9333, -89.8167, 45.4222
	{   "HARN/WI.TaylorWI-IF",         "TaylorWI-IF"                 },    //-91.15, 44.9333, -89.8167, 45.4222
	{   "HARN/WI.RichlandWI-M",        "RichlandWI-M"                },    //-91.0972, 42.9611, -89.7639, 43.6833
	{   "HARN/WI.RichlandWI-F",        "RichlandWI-F"                },    //-91.0972, 42.9611, -89.7639, 43.6833
	{   "HARN/WI.RichlandWI-IF",       "RichlandWI-IF"               },    //-91.0972, 42.9611, -89.7639, 43.6833
	{   "HARN/WI.WoodWI-M",            "WoodWI-M"                    },    //-90.6667, 43.9986, -89.3333, 44.7264
	{   "HARN/WI.WoodWI-F",            "WoodWI-F"                    },    //-90.6667, 43.9986, -89.3333, 44.7264
	{   "HARN/WI.WoodWI-IF",           "WoodWI-IF"                   },    //-90.6667, 43.9986, -89.3333, 44.7264
	{   "HARN/WI.LafayetteWI-M",       "GreenWI-M"                   },    //-90.5056, 42.3347, -89.1722, 42.9403
	{   "HARN/WI.LafayetteWI-M",       "LafayetteWI-M"               },    //-90.5056, 42.3347, -89.1722, 42.9403
	{   "HARN/WI.GreenWI-F",           "GreenWI-F"                   },    //-90.5056, 42.3347, -89.1722, 42.9403
	{   "HARN/WI.GreenWI-F",           "LafayetteWI-F"               },    //-90.5056, 42.3347, -89.1722, 42.9403
	{   "HARN/WI.GreenWI-IF",          "GreenWI-IF"                  },    //-90.5056, 42.3347, -89.1722, 42.9403
	{   "HARN/WI.GreenWI-IF",          "LafayetteWI-IF"              },    //-90.5056, 42.3347, -89.1722, 42.9403
	{   "HARN/WI.MarathonWI-M",        "MarathonWI-M"                },    //-90.4367, 44.5897, -89.1033, 45.2119
	{   "HARN/WI.MarathonWI-F",        "MarathonWI-IF"               },    //-90.4367, 44.5897, -89.1033, 45.2119
	{   "HARN/WI.MarathonWI-F",        "MarathonWI-F"                },    //-90.4367, 44.5897, -89.1033, 45.2119
	{   "HARN/WI.OneidaWI-M",          "OneidaWI-M"                  },    //-90.2111, 45.4292, -88.8778, 45.9792
	{   "HARN/WI.OneidaWI-F",          "OneidaWI-F"                  },    //-90.2111, 45.4292, -88.8778, 45.9792
	{   "HARN/WI.OneidaWI-IF",         "OneidaWI-IF"                 },    //-90.2111, 45.4292, -88.8778, 45.9792
	{   "HARN/WI.PortageWI-M",         "PortageWI-M"                 },    //-90.1667, 43.95, -88.8333, 44.8833
	{   "HARN/WI.PortageWI-F",         "PortageWI-F"                 },    //-90.1667, 43.95, -88.8333, 44.8833
	{   "HARN/WI.PortageWI-IF",        "PortageWI-IF"                },    //-90.1667, 43.95, -88.8333, 44.8833
	{   "HARN/WI.VilasWI-M",           "VilasWI-M"                   },    //-90.1556, 45.7833, -88.8222, 46.3722
	{   "HARN/WI.VilasWI-F",           "VilasWI-F"                   },    //-90.1556, 45.7833, -88.8222, 46.3722
	{   "HARN/WI.VilasWI-IF",          "VilasWI-IF"                  },    //-90.1556, 45.7833, -88.8222, 46.3722
	{   "HARN/WI.DaneWI-M",            "DaneWI-M"                    },    //-90.0889, 42.7472, -88.7556, 43.3917
	{   "HARN/WI.DaneWI-F",            "DaneWI-F"                    },    //-90.0889, 42.7472, -88.7556, 43.3917
	{   "HARN/WI.DaneWI-IF",           "DaneWI-IF"                   },    //-90.0889, 42.7472, -88.7556, 43.3917
	{   "HARN/WI.ColumbiaWI-M",        "ColumbiaWI-M"                },    //-90.0611, 43.2042, -88.7278, 43.7208
	{   "HARN/WI.ColumbiaWI-F",        "ColumbiaWI-F"                },    //-90.0611, 43.2042, -88.7278, 43.7208
	{   "HARN/WI.ColumbiaWI-IF",       "ColumbiaWI-IF"               },    //-90.0611, 43.2042, -88.7278, 43.7208
	{   "HARN/WI.MarquetteWI-M",       "GreenLakeWI-M"               },    //-89.9083, 43.5264, -88.575, 44.0875
	{   "HARN/WI.MarquetteWI-M",       "MarquetteWI-M"               },    //-89.9083, 43.5264, -88.575, 44.0875
	{   "HARN/WI.MarquetteWI-F",       "GreenLakeWI-F"               },    //-89.9083, 43.5264, -88.575, 44.0875
	{   "HARN/WI.MarquetteWI-F",       "MarquetteWI-F"               },    //-89.9083, 43.5264, -88.575, 44.0875
	{   "HARN/WI.MarquetteWI-IF",      "MarquetteWI-IF"              },    //-89.9083, 43.5264, -88.575, 44.0875
	{   "HARN/WI.MarquetteWI-IF",      "GreenLakeWI-IF"              },    //-89.9083, 43.5264, -88.575, 44.0875
	{   "HARN/WI.WausharaWI-M",        "WausharaWI-M"                },    //-89.9083, 43.8361, -88.575, 44.3917
	{   "HARN/WI.WausharaWI-F",        "WausharaWI-F"                },    //-89.9083, 43.8361, -88.575, 44.3917
	{   "HARN/WI.WausharaWI-IF",       "WausharaWI-IF"               },    //-89.9083, 43.8361, -88.575, 44.3917
	{   "HARN/WI.LangladeWI-M",        "LangladeWI-M"                },    //-89.7, 44.8458, -88.3667, 45.4625
	{   "HARN/WI.LangladeWI-F",        "LangladeWI-F"                },    //-89.7, 44.8458, -88.3667, 45.4625
	{   "HARN/WI.LangladeWI-IF",       "LangladeWI-IF"               },    //-89.7, 44.8458, -88.3667, 45.4625
	{   "HARN/WI.WalworthWI-M",        "WalworthWI-M"                },    //-89.2083, 42.5083, -87.875, 42.8306
	{   "HARN/WI.WalworthWI-F",        "WalworthWI-F"                },    //-89.2083, 42.5083, -87.875, 42.8306
	{   "HARN/WI.WalworthWI-IF",       "WalworthWI-IF"               },    //-89.2083, 42.5083, -87.875, 42.8306
	{   "HARN/WI.GrantWI-M",           "GrantWI-M"                   },    //-91.3306, 41.4111, -90.2669, 42.6611
	{   "HARN/WI.GrantWI-F",           "GrantWI-F"                   },    //-91.3306, 41.4111, -90.2669, 42.6611
	{   "HARN/WI.GrantWI-IF",          "GrantWI-IF"                  },    //-91.3306, 41.4111, -90.2669, 42.6611
	{   "HARN/WI.DodgeWI-M",           "DodgeWI-M"                   },    //-89.3056, 41.4722, -88.2444, 42.7222
	{   "HARN/WI.DodgeWI-M",           "JeffersonWI-M"               },    //-89.3056, 41.4722, -88.2444, 42.7222
	{   "HARN/WI.DodgeWI-F",           "JeffersonWI-F"               },    //-89.3056, 41.4722, -88.2444, 42.7222
	{   "HARN/WI.DodgeWI-F",           "DodgeWI-F"                   },    //-89.3056, 41.4722, -88.2444, 42.7222
	{   "HARN/WI.DodgeWI-IF",          "JeffersonWI-IF"              },    //-89.3056, 41.4722, -88.2444, 42.7222
	{   "HARN/WI.DodgeWI-IF",          "DodgeWI-IF"                  },    //-89.3056, 41.4722, -88.2444, 42.7222
	{   "HARN/WI.RockWI-M",            "RockWI-M"                    },    //-89.6464, 41.9444, -88.4981, 43.1944
	{   "HARN/WI.RockWI-F",            "RockWI-F"                    },    //-89.6464, 41.9444, -88.4981, 43.1944
	{   "HARN/WI.RockWI-IF",           "RockWI-IF"                   },    //-89.6464, 41.9444, -88.4981, 43.1944
	{   "HARN/WI.OzaukeeWI-M",         "MilwaukeeWI-M"               },    //-88.3736, 42.2167, -87.4153, 43.4667
	{   "HARN/WI.OzaukeeWI-M",         "OzaukeeWI-M"                 },    //-88.3736, 42.2167, -87.4153, 43.4667
	{   "HARN/WI.OzaukeeWI-M",         "KenoshaWI-M"                 },    //-88.3736, 42.2167, -87.4153, 43.4667
	{   "HARN/WI.OzaukeeWI-M",         "RacineWI-M"                  },    //-88.3736, 42.2167, -87.4153, 43.4667
	{   "HARN/WI.MilwaukeeWI-F",       "RacineWI-IF"                 },    //-88.3736, 42.2167, -87.4153, 43.4667
	{   "HARN/WI.MilwaukeeWI-F",       "MilwaukeeWI-F"               },    //-88.3736, 42.2167, -87.4153, 43.4667
	{   "HARN/WI.MilwaukeeWI-F",       "OzaukeeWI-IF"                },    //-88.3736, 42.2167, -87.4153, 43.4667
	{   "HARN/WI.MilwaukeeWI-F",       "OzaukeeWI-F"                 },    //-88.3736, 42.2167, -87.4153, 43.4667
	{   "HARN/WI.MilwaukeeWI-F",       "KenoshaWI-F"                 },    //-88.3736, 42.2167, -87.4153, 43.4667
	{   "HARN/WI.MilwaukeeWI-F",       "RacineWI-F"                  },    //-88.3736, 42.2167, -87.4153, 43.4667
	{   "HARN/WI.MilwaukeeWI-F",       "MilwaukeeWI-IF"              },    //-88.3736, 42.2167, -87.4153, 43.4667
	{   "HARN/WI.MilwaukeeWI-F",       "KenoshaWI-IF"                },    //-88.3736, 42.2167, -87.4153, 43.4667
	{   "HARN/WI.IowaWI-M",            "IowaWI-M"                    },    //-90.6917, 42.5389, -89.6306, 43.7889
	{   "HARN/WI.IowaWI-F",            "IowaWI-F"                    },    //-90.6917, 42.5389, -89.6306, 43.7889
	{   "HARN/WI.IowaWI-IF",           "IowaWI-IF"                   },    //-90.6917, 42.5389, -89.6306, 43.7889
	{   "HARN/WI.WaukeshaWI-M",        "WaukeshaWI-M"                },    //-88.7556, 42.5694, -87.6944, 43.8194
	{   "HARN/WI.WaukeshaWI-F",        "WaukeshaWI-F"                },    //-88.7556, 42.5694, -87.6944, 43.8194
	{   "HARN/WI.WaukeshaWI-IF",       "WaukeshaWI-IF"               },    //-88.7556, 42.5694, -87.6944, 43.8194
	{   "HARN/WI.OutagamieWI-M",       "WinnebagoWI-M"               },    //-89.0742, 42.7194, -87.9258, 43.9694
	{   "HARN/WI.OutagamieWI-M",       "OutagamieWI-M"               },    //-89.0742, 42.7194, -87.9258, 43.9694
	{   "HARN/WI.OutagamieWI-M",       "CalumetWI-M"                 },    //-89.0742, 42.7194, -87.9258, 43.9694
	{   "HARN/WI.OutagamieWI-M",       "FondDuLacWI-M"               },    //-89.0742, 42.7194, -87.9258, 43.9694
	{   "HARN/WI.OutagamieWI-IF",      "WinnebagoWI-IF"              },    //-89.0742, 42.7194, -87.9258, 43.9694
	{   "HARN/WI.OutagamieWI-IF",      "WinnebagoWI-F"               },    //-89.0742, 42.7194, -87.9258, 43.9694
	{   "HARN/WI.OutagamieWI-IF",      "CalumetWI-F"                 },    //-89.0742, 42.7194, -87.9258, 43.9694
	{   "HARN/WI.OutagamieWI-IF",      "CalumetWI-IF"                },    //-89.0742, 42.7194, -87.9258, 43.9694
	{   "HARN/WI.OutagamieWI-IF",      "FondDuLacWI-F"               },    //-89.0742, 42.7194, -87.9258, 43.9694
	{   "HARN/WI.OutagamieWI-IF",      "FondDuLacWI-IF"              },    //-89.0742, 42.7194, -87.9258, 43.9694
	{   "HARN/WI.OutagamieWI-IF",      "OutagamieWI-F"               },    //-89.0742, 42.7194, -87.9258, 43.9694
	{   "HARN/WI.OutagamieWI-IF",      "OutagamieWI-IF"              },    //-89.0742, 42.7194, -87.9258, 43.9694
	{   "HARN/WI.SaukWI-M",            "SaukWI-M"                    },    //-90.5125, 42.8194, -89.2875, 44.0694
	{   "HARN/WI.SaukWI-F",            "SaukWI-F"                    },    //-90.5125, 42.8194, -89.2875, 44.0694
	{   "HARN/WI.SaukWI-IF",           "SaukWI-IF"                   },    //-90.5125, 42.8194, -89.2875, 44.0694
	{   "HARN/WI.WashingtonWI-M",      "WashingtonWI-M"              },    //-88.6764, 42.9181, -87.4514, 44.1681
	{   "HARN/WI.WashingtonWI-F",      "WashingtonWI-F"              },    //-88.6764, 42.9181, -87.4514, 44.1681
	{   "HARN/WI.WashingtonWI-IF",     "WashingtonWI-IF"             },    //-88.6764, 42.9181, -87.4514, 44.1681
	{   "HARN/WI.BrownWI-M",           "BrownWI-M"                   },    //-88.75, 43, -87.25, 44.25
	{   "HARN/WI.BrownWI-F",           "BrownWI-F"                   },    //-88.75, 43, -87.25, 44.25
	{   "HARN/WI.BrownWI-IF",          "BrownWI-IF"                  },    //-88.75, 43, -87.25, 44.25
	{   "HARN/WI.TrempealeauWI-M",     "TrempealeauWI-M"             },    //-91.8458, 43.1611, -90.8875, 44.4111
	{   "HARN/WI.TrempealeauWI-F",     "TrempealeauWI-F"             },    //-91.8458, 43.1611, -90.8875, 44.4111
	{   "HARN/WI.SheboyganWI-M",       "ManitowocWI-M"               },    //-88.3, 43.2667, -86.8, 44.5167
	{   "HARN/WI.SheboyganWI-M",       "KewauneeWI-M"                },    //-88.3, 43.2667, -86.8, 44.5167
	{   "HARN/WI.SheboyganWI-M",       "SheboyganWI-M"               },    //-88.3, 43.2667, -86.8, 44.5167
	{   "HARN/WI.ManitowocWI-IF",      "SheboyganWI-IF"              },    //-88.3, 43.2667, -86.8, 44.5167
	{   "HARN/WI.ManitowocWI-IF",      "KewauneeWI-F"                },    //-88.3, 43.2667, -86.8, 44.5167
	{   "HARN/WI.ManitowocWI-IF",      "KewauneeWI-IF"               },    //-88.3, 43.2667, -86.8, 44.5167
	{   "HARN/WI.ManitowocWI-IF",      "ManitowocWI-F"               },    //-88.3, 43.2667, -86.8, 44.5167
	{   "HARN/WI.ManitowocWI-IF",      "SheboyganWI-F"               },    //-88.3, 43.2667, -86.8, 44.5167
	{   "HARN/WI.ManitowocWI-IF",      "ManitowocWI-IF"              },    //-88.3, 43.2667, -86.8, 44.5167
	{   "HARN/WI.AdamsWI-M",           "JuneauWI-M"                  },    //-90.4119, 43.3667, -89.5881, 44.6167
	{   "HARN/WI.AdamsWI-M",           "AdamsWI-M"                   },    //-90.4119, 43.3667, -89.5881, 44.6167
	{   "HARN/WI.AdamsWI-F",           "JuneauWI-F"                  },    //-90.4119, 43.3667, -89.5881, 44.6167
	{   "HARN/WI.AdamsWI-F",           "AdamsWI-F"                   },    //-90.4119, 43.3667, -89.5881, 44.6167
	{   "HARN/WI.AdamsWI-IF",          "JuneauWI-IF"                 },    //-90.4119, 43.3667, -89.5881, 44.6167
	{   "HARN/WI.AdamsWI-IF",          "AdamsWI-IF"                  },    //-90.4119, 43.3667, -89.5881, 44.6167
	{   "HARN/WI.WaupacaWI-M",         "WaupacaWI-M"                 },    //-89.3908, 43.4203, -88.2425, 44.6703
	{   "HARN/WI.WaupacaWI-F",         "WaupacaWI-F"                 },    //-89.3908, 43.4203, -88.2425, 44.6703
	{   "HARN/WI.WaupacaWI-IF",        "WaupacaWI-IF"                },    //-89.3908, 43.4203, -88.2425, 44.6703
	{   "HARN/WI.LaCrosseWI-M",        "LaCrosseWI-M"                },    //-91.9636, 43.4511, -90.6697, 44.7011
	{   "HARN/WI.LaCrosseWI-F",        "LaCrosseWI-F"                },    //-91.9636, 43.4511, -90.6697, 44.7011
	{   "HARN/WI.LaCrosseWI-IF",       "LaCrosseWI-IF"               },    //-91.9636, 43.4511, -90.6697, 44.7011
	{   "HARN/WI.BuffaloWI-M",         "BuffaloWI-M"                 },    //-92.5472, 43.4814, -91.0472, 44.7314
	{   "HARN/WI.BuffaloWI-F",         "BuffaloWI-F"                 },    //-92.5472, 43.4814, -91.0472, 44.7314
	{   "HARN/WI.BuffaloWI-IF",        "BuffaloWI-IF"                },    //-92.5472, 43.4814, -91.0472, 44.7314
	{   "HARN/WI.ClarkWI-M",           "ClarkWI-M"                   },    //-91.3553, 43.6, -90.0614, 44.85
	{   "HARN/WI.ClarkWI-F",           "ClarkWI-F"                   },    //-91.3553, 43.6, -90.0614, 44.85
	{   "HARN/WI.ClarkWI-IF",          "ClarkWI-IF"                  },    //-91.3553, 43.6, -90.0614, 44.85
	{   "HARN/WI.RuskWI-M",            "RuskWI-M"                    },    //-91.5972, 43.9169, -90.5336, 45.1669
	{   "HARN/WI.RuskWI-F",            "RuskWI-F"                    },    //-91.5972, 43.9169, -90.5336, 45.1669
	{   "HARN/WI.RuskWI-IF",           "RuskWI-IF"                   },    //-91.5972, 43.9169, -90.5336, 45.1669
	{   "HARN/WI.ForestWI-M",          "ForestWI-M"                  },    //-89.2075, 44.0056, -88.0592, 45.2556
	{   "HARN/WI.ForestWI-F",          "ForestWI-F"                  },    //-89.2075, 44.0056, -88.0592, 45.2556
	{   "HARN/WI.ForestWI-IF",         "ForestWI-IF"                 },    //-89.2075, 44.0056, -88.0592, 45.2556
	{   "HARN/WI.StCroixWI-M",         "StCroixWI-M"                 },    //-93.2458, 44.0361, -92.0208, 45.2861
	{   "HARN/WI.StCroixWI-F",         "StCroixWI-F"                 },    //-93.2458, 44.0361, -92.0208, 45.2861
	{   "HARN/WI.StCroixWI-IF",        "StCroixWI-IF"                },    //-93.2458, 44.0361, -92.0208, 45.2861
	{   "HARN/WI.ShawanoWI-M",         "ShawanoWI-M"                 },    //-89.3681, 44.0361, -87.8431, 45.2861
	{   "HARN/WI.ShawanoWI-F",         "ShawanoWI-F"                 },    //-89.3681, 44.0361, -87.8431, 45.2861
	{   "HARN/WI.ShawanoWI-IF",        "ShawanoWI-IF"                },    //-89.3681, 44.0361, -87.8431, 45.2861
	{   "HARN/WI.OcontoWI-M",          "OcontoWI-M"                  },    //-88.6444, 44.3972, -87.1722, 45.6472
	{   "HARN/WI.OcontoWI-IF",         "OcontoWI-IF"                 },    //-88.6444, 44.3972, -87.1722, 45.6472
	{   "HARN/WI.OcontoWI-IF",         "OcontoWI-F"                  },    //-88.6444, 44.3972, -87.1722, 45.6472
	{   "HARN/WI.DoorWI-M",            "DoorWI-M"                    },    //-88.0083, 44.4, -86.5336, 45.65
	{   "HARN/WI.DoorWI-F",            "DoorWI-IF"                   },    //-88.0083, 44.4, -86.5336, 45.65
	{   "HARN/WI.DoorWI-F",            "DoorWI-F"                    },    //-88.0083, 44.4, -86.5336, 45.65
	{   "HARN/WI.DunnWI-M",            "DunnWI-M"                    },    //-92.3886, 44.4083, -91.4003, 45.6583
	{   "HARN/WI.DunnWI-F",            "DunnWI-F"                    },    //-92.3886, 44.4083, -91.4003, 45.6583
	{   "HARN/WI.DunnWI-IF",           "DunnWI-IF"                   },    //-92.3886, 44.4083, -91.4003, 45.6583
	{   "HARN/WI.PriceWI-M",           "PriceWI-M"                   },    //-90.9681, 44.5556, -90.0097, 45.8056
	{   "HARN/WI.PriceWI-F",           "PriceWI-F"                   },    //-90.9681, 44.5556, -90.0097, 45.8056
	{   "HARN/WI.PriceWI-IF",          "PriceWI-IF"                  },    //-90.9681, 44.5556, -90.0097, 45.8056
	{   "HARN/WI.PolkWI-M",            "PolkWI-M"                    },    //-93.3833, 44.6611, -91.8833, 45.9111
	{   "HARN/WI.PolkWI-F",            "PolkWI-F"                    },    //-93.3833, 44.6611, -91.8833, 45.9111
	{   "HARN/WI.PolkWI-IF",           "PolkWI-IF"                   },    //-93.3833, 44.6611, -91.8833, 45.9111
	{   "HARN/WI.MarinetteWI-M",       "MarinetteWI-M"               },    //-88.5675, 44.6917, -86.8547, 45.9417
	{   "HARN/WI.MarinetteWI-F",       "MarinetteWI-F"               },    //-88.5675, 44.6917, -86.8547, 45.9417
	{   "HARN/WI.MarinetteWI-IF",      "MarinetteWI-IF"              },    //-88.5675, 44.6917, -86.8547, 45.9417
	{   "HARN/WI.MenomineeWI-M",       "MenomineeWI-M"               },    //-89.0636, 44.7167, -87.7697, 45.9667
	{   "HARN/WI.MenomineeWI-F",       "MenomineeWI-F"               },    //-89.0636, 44.7167, -87.7697, 45.9667
	{   "HARN/WI.MenomineeWI-IF",      "MenomineeWI-IF"              },    //-89.0636, 44.7167, -87.7697, 45.9667
	{   "HARN/WI.LincolnWI-M",         "LincolnWI-M"                 },    //-90.2125, 44.8444, -89.2542, 46.0944
	{   "HARN/WI.LincolnWI-F",         "LincolnWI-F"                 },    //-90.2125, 44.8444, -89.2542, 46.0944
	{   "HARN/WI.LincolnWI-IF",        "LincolnWI-IF"                },    //-90.2125, 44.8444, -89.2542, 46.0944
	{   "HARN/WI.BarronWI-M",          "BarronWI-M"                  },    //-92.4242, 45.1333, -91.2758, 46.3833
	{   "HARN/WI.BarronWI-F",          "BarronWI-F"                  },    //-92.4242, 45.1333, -91.2758, 46.3833
	{   "HARN/WI.BarronWI-IF",         "BarronWI-IF"                 },    //-92.4242, 45.1333, -91.2758, 46.3833
	{   "HARN/WI.IronWI-M",            "IronWI-M"                    },    //-90.8297, 45.4333, -89.6814, 46.6833
	{   "HARN/WI.IronWI-F",            "IronWI-F"                    },    //-90.8297, 45.4333, -89.6814, 46.6833
	{   "HARN/WI.IronWI-IF",           "IronWI-IF"                   },    //-90.8297, 45.4333, -89.6814, 46.6833
	{   "HARN/WI.FlorenceWI-M",        "FlorenceWI-M"                },    //-88.8047, 45.4389, -87.4786, 46.6889
	{   "HARN/WI.FlorenceWI-F",        "FlorenceWI-F"                },    //-88.8047, 45.4389, -87.4786, 46.6889
	{   "HARN/WI.FlorenceWI-IF",       "FlorenceWI-IF"               },    //-88.8047, 45.4389, -87.4786, 46.6889
	{   "HARN/WI.AshlandWI-M",         "AshlandWI-M"                 },    //-91.1528, 45.7061, -90.0917, 46.9561
	{   "HARN/WI.AshlandWI-IF",        "AshlandWI-IF"                },    //-91.1528, 45.7061, -90.0917, 46.9561
	{   "HARN/WI.AshlandWI-IF",        "AshlandWI-F"                 },    //-91.1528, 45.7061, -90.0917, 46.9561
	{   "HARN/WI.DouglasWI-M",         "DouglasWI-M"                 },    //-92.53, 45.8833, -91.3033, 47.1333
	{   "HARN/WI.DouglasWI-F",         "DouglasWI-IF"                },    //-92.53, 45.8833, -91.3033, 47.1333
	{   "HARN/WI.DouglasWI-F",         "DouglasWI-F"                 },    //-92.53, 45.8833, -91.3033, 47.1333
	{   "UTM84-2S",                    "HARN.UTM-2S"                 },    //-174.75, -88, -167.25, 8
	{   "UTM84-10N",                   "UTMHP-10"                    },    //-126.75, -8.4, -119.25, 90
	{   "UTM84-10N",                   "UTMHP-10F"                   },    //-126.75, -8.4, -119.25, 90
	{   "UTM84-10N",                   "UTMHP-10IF"                  },    //-126.75, -8.4, -119.25, 90
	{   "UTM27-11F",                   "UTMHP-11"                    },    //-120.5, -1, -113.5, 84
	{   "UTM27-11F",                   "UTMHP-11F"                   },    //-120.5, -1, -113.5, 84
	{   "UTM27-11F",                   "UTMHP-11IF"                  },    //-120.5, -1, -113.5, 84
	{   "HARN/12.UTM-12",              "UTMHP-12IF"                  },    //-114.75, 29.5667, -107.25, 50.7667
	{   "HARN/12.UTM-12",              "UTMHP-12"                    },    //-114.75, 29.5667, -107.25, 50.7667
	{   "HARN/12.UTM-12",              "UTMHP-12F"                   },    //-114.75, 29.5667, -107.25, 50.7667
	{   "UTM83-13F",                   "UTMHP-13"                    },    //-108.5, -1, -101.5, 84
	{   "UTM83-13F",                   "UTMHP-13F"                   },    //-108.5, -1, -101.5, 84
	{   "UTM83-13F",                   "UTMHP-13IF"                  },    //-108.5, -1, -101.5, 84
//	{   "SIRGAS2000.UTM-14N",          "UTMHP-14"                    },    //-102, 0, -96, 29.9
//	{   "SIRGAS2000.UTM-14N",          "UTMHP-14F"                   },    //-102, 0, -96, 29.9
//	{   "SIRGAS2000.UTM-14N",          "UTMHP-14IF"                  },    //-102, 0, -96, 29.9
	{   "UTM27-15F",                   "UTMHP-15"                    },    //-96.5, -1, -89.5, 84
	{   "UTM27-15F",                   "UTMHP-15IF"                  },    //-96.5, -1, -89.5, 84
	{   "UTM27-15F",                   "UTMHP-15F"                   },    //-96.5, -1, -89.5, 84
	{   "HARN/16.UTM-16",              "UTMHP-16F"                   },    //-90.75, 26.9, -83.25, 50.25
	{   "HARN/16.UTM-16",              "UTMHP-16"                    },    //-90.75, 26.9, -83.25, 50.25
	{   "HARN/16.UTM-16",              "UTMHP-16IF"                  },    //-90.75, 26.9, -83.25, 50.25
//	{   "SIRGAS2000.UTM-17N",          "UTMHP-17"                    },    //-84, 0, -78, 16
//	{   "SIRGAS2000.UTM-17N",          "UTMHP-17F"                   },    //-84, 0, -78, 16
//	{   "SIRGAS2000.UTM-17N",          "UTMHP-17IF"                  },    //-84, 0, -78, 16
	{   "UTM83-18",                    "UTMHP-18F"                   },    //-78.75, 22.7167, -71.25, 89.5667
	{   "UTM83-18",                    "UTMHP-18"                    },    //-78.75, 22.7167, -71.25, 89.5667
	{   "UTM83-18",                    "UTMHP-18IF"                  },    //-78.75, 22.7167, -71.25, 89.5667
	{   "MI83-OB",                     "MI27-OB"                     },    //-91.45, 41.0333, -81.0833, 48.9667
	{   "TMBLI-B.RSOBorneo.m",         "BORNEO-CH"                   },    //108.333, 0.2, 120.467, 8
	{   "TMBLI-B.RSOBorneo.m",         "BORNEO"                      },    //108.333, 0.2, 120.467, 8
	{   "",                            ""                            }
};

bool csUsefulRangeTransfer (const wchar_t* csDictDir,int ticketNbr)
{
	bool ok (true);

	char crsAscFilePath [MAXPATH];

	const struct TcsUsefulRngTransferTbl_* tblPtr;

	wcstombs (crsAscFilePath,csDictDir,sizeof (crsAscFilePath));
	CS_stncat (crsAscFilePath,"\\coordsys.asc",sizeof (crsAscFilePath));
	TcsDefFile coordsysAsc (dictTypCoordsys,crsAscFilePath);


	for (tblPtr = KcsUsefulRngTransferTbl;ok && tblPtr->rngSrc [0] != '\0';tblPtr += 1)
	{
		const char *cPtrK;
		char llMin0 [64];
		char llMin1 [64];
		char llMax0 [64];
		char llMax1 [64];
		char comment [64];

		TcsAscDefinition* rngSrcDefPtr;
		TcsAscDefinition* rngTrgDefPtr;
		TcsDefLine* defLinePtr;

		rngSrcDefPtr = coordsysAsc.GetDefinition (tblPtr->rngSrc);
		rngTrgDefPtr = coordsysAsc.GetDefinition (tblPtr->rngTrg);
		sprintf (comment,"# Ticket %d: Useful range copied from %s.",ticketNbr,tblPtr->rngSrc);

		ok = (rngSrcDefPtr != 0) && (rngTrgDefPtr != 0);
		if (ok)
		{
			cPtrK = rngSrcDefPtr->GetValue ("MIN_LNG:");
			ok = (cPtrK != 0);
			if (ok)
			{
				CS_stncp (llMin0,cPtrK,sizeof (llMin0));
			}
		}
		if (ok)
		{
			cPtrK = rngSrcDefPtr->GetValue ("MIN_LAT:");
			ok = (cPtrK != 0);
			if (ok)
			{
				CS_stncp (llMin1,cPtrK,sizeof (llMin1));
			}
		}
		if (ok)
		{
			cPtrK = rngSrcDefPtr->GetValue ("MAX_LNG:");
			ok = (cPtrK != 0);
			if (ok)
			{
				CS_stncp (llMax0,cPtrK,sizeof (llMax0));
			}
		}
		if (ok)
		{
			cPtrK = rngSrcDefPtr->GetValue ("MAX_LAT:");
			ok = (cPtrK != 0);
			if (ok)
			{
				CS_stncp (llMax1,cPtrK,sizeof (llMax1));
			}
		}
		if (ok)
		{
			defLinePtr = rngTrgDefPtr->GetLine ("MIN_LNG:");
			if (defLinePtr == 0)
			{
				TcsDefLine newLine (dictTypCoordsys,"MIN_LNG:",llMin0,comment);
				ok = rngTrgDefPtr->InsertAfter ("Y_OFF:",newLine);
				if (!ok)
				{
					ok = rngTrgDefPtr->Append (newLine);
				}
			}
			else
			{
				defLinePtr->SetValue (llMin0);
				defLinePtr->SetComment (comment);
			}
		}
		if (ok)
		{
			defLinePtr = rngTrgDefPtr->GetLine ("MIN_LAT:");
			if (defLinePtr == 0)
			{
				TcsDefLine newLine (dictTypCoordsys,"MIN_LAT:",llMin1,0);
				ok = rngTrgDefPtr->InsertAfter ("MIN_LNG:",newLine);
				if (!ok)
				{
					ok = rngTrgDefPtr->Append (newLine);
				}
			}
			else
			{
				defLinePtr->SetValue (llMin1);
			}
		}
		if (ok)
		{
			defLinePtr = rngTrgDefPtr->GetLine ("MAX_LNG:");
			if (defLinePtr == 0)
			{
				TcsDefLine newLine (dictTypCoordsys,"MAX_LNG:",llMax0,0);
				ok = rngTrgDefPtr->InsertAfter ("MIN_LAT:",newLine);
				if (!ok)
				{
					ok = rngTrgDefPtr->Append (newLine);
				}
			}
			else
			{
				defLinePtr->SetValue (llMax0);
			}
		}
		if (ok)
		{
			defLinePtr = rngTrgDefPtr->GetLine ("MAX_LAT:");
			if (defLinePtr == 0)
			{
				TcsDefLine newLine (dictTypCoordsys,"MAX_LAT:",llMax1,0);
				ok = rngTrgDefPtr->InsertAfter ("MAX_LNG:",newLine);
				if (!ok)
				{
					ok = rngTrgDefPtr->Append (newLine);
				}
			}
			else
			{
				defLinePtr->SetValue (llMax1);
			}
		}
	}
	if (ok)
	{
		ok = coordsysAsc.WriteToFile (crsAscFilePath);
	}
	return ok;
}
