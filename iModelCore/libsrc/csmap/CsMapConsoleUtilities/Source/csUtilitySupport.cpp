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

extern "C" unsigned long KcsNmInvNumber;
extern "C" unsigned long KcsNmMapNoNumber;
extern "C" double cs_Zero;
extern "C" double cs_LlNoise;
extern "C" struct cs_Prjtab_ cs_Prjtab [];
extern "C" struct cs_PrjprmMap_ cs_PrjprmMap [];

const wchar_t* dbl2wcs (double dblVal)
{
	static wchar_t wcBufr [128];

	double logOfVal;
	wchar_t* wcPtr;

	// We handle very small numbers, and very large numbers differently from
	// you rnormal type number.
	logOfVal = log10 (dblVal);
	if (logOfVal >= 9.0)
	{
		// A very large number.
		swprintf (wcBufr,128,L"%.10E",dblVal);
	}
	else if (logOfVal <= -6.0)
	{
		// A very small number.  
		swprintf (wcBufr,128,L"%G",dblVal);
	}
	else
	{
		// A normal number.  Convert at high precision, but trim trailing
		// zeros.  Always leave one zero after the decimal point for
		// integer values.
		swprintf (wcBufr,128,L"%.12f",dblVal);
		wcPtr = wcschr (wcBufr,L'.');
		if (wcPtr != 0)	
		{
			// We have a high precision number and it has a decimal point.
			wcPtr = wcBufr + wcslen (wcBufr) - 1;
			while (*wcPtr == L'0')
			{
				*wcPtr = L'\0';
				wcPtr -= 1;
			}
			if (*wcPtr == L'.')
			{
				wcPtr += 1;
				*wcPtr++ = L'0';
				*wcPtr = L'\0';
			}
		}
	}
	return wcBufr;
}

unsigned short CS_getPrjCode (const char* projKeyName)
{
	unsigned short projCode = cs_PRJCOD_END;
	struct cs_Prjtab_ *pp;

	for (pp = cs_Prjtab;*pp->key_nm != '\0';pp++)
	{
		if (!CS_stricmp (pp->key_nm,projKeyName))
		{
			projCode = pp->code;
			break;
		}
	}
	return projCode;
}

unsigned char CS_getParmCode (unsigned short projCode,unsigned parmNbr)
{
	unsigned char parmCode;
	struct cs_PrjprmMap_* prmTblPtr;

	parmCode = cs_PRMCOD_NOTUSED;
	if (parmNbr >= 1 && parmNbr <= 24)
	{
		for (prmTblPtr = cs_PrjprmMap;prmTblPtr->prj_code != cs_PRJCOD_END;prmTblPtr += 1)
		{
			if (prmTblPtr->prj_code == projCode)
			{
				parmCode = prmTblPtr->prm_types [parmNbr - 1];
				break;
			}
		}
	}
	return parmCode;
}
bool CS_crsHasUsefulRng (const struct cs_Csdef_& csDef)
{
	bool hasUsefulRange;
	
	hasUsefulRange = (csDef.ll_max [0] != 0.0) || (csDef.ll_max [1] != 0.0) ||
					 (csDef.ll_min [0] != 0.0) || (csDef.ll_min [1] != 0.0);
	return hasUsefulRange;
}

// The following function reads a CSV with one set of delimiters and writes
// with a second set of delimiters.  Especially useful when dealing with
// WKT files where all the element names are quoted strings, and you need
// to get rid of the double quotes.
bool CsvDelimiterConvert (const wchar_t* csDataDir,const wchar_t* inputFile,bool labels,
																			const wchar_t* fromDelims,
																			const wchar_t* toDelims,
																			const wchar_t* outputFile)
{
	bool ok (false);

	wchar_t filePath [MAXPATH + MAXPATH];

	std::wifstream iStrm;
	std::wofstream oStrm;

	TcsCsvStatus csvStatus;
	TcsCsvFileBase csvData (labels,3,5,fromDelims);

	// Open source file
	wcsncpy (filePath,csDataDir,MAXPATH);
	wcscat (filePath,L"\\");
	wcscat (filePath,inputFile);
	iStrm.open (filePath,std::ios_base::in);
	ok = iStrm.is_open ();

	// Read the input file.
	if (ok)
	{
		ok = csvData.ReadFromStream (iStrm,labels,csvStatus);
		iStrm.close ();
	}

	if (ok)
	{
		csvData.SetDelimiters (toDelims);
	}
	if (ok)
	{
		// Create/Truncate the output file.
		wcsncpy (filePath,csDataDir,MAXPATH);
		wcscat (filePath,L"\\");
		wcscat (filePath,outputFile);
		oStrm.open (filePath,std::ios_base::out | std::ios_base::trunc);
		ok = oStrm.is_open ();
	}
	if (ok)
	{
		ok = csvData.WriteToStream (oStrm,labels,csvStatus);
		oStrm.close ();
	}
	return ok;
}

// Returns a pointer to a TcsEpsgDataSetV6 object.  Specifically, a
// TcsEpsgDataSetV6 object initialized with the contents of the the folder
// pointed to by the csEpsgDir global variable defined in the
// csConsoleUtilities main module.  The prupose of this module is to
// elimiate the possibility of having two of these floating around as
// it is quite time consuming to buyild one of these things.

// The following should be const, but the dumb Micros??t linker can't find it if it is.
extern wchar_t csEpsgDir [];
TcsEpsgDataSetV6* csEpsgDataSetV6Ptr = 0;

const TcsEpsgDataSetV6* GetEpsgObjectPtr ()
{
	if (csEpsgDataSetV6Ptr == 0)
	{
		csEpsgDataSetV6Ptr = new TcsEpsgDataSetV6 (csEpsgDir);
	}
	return csEpsgDataSetV6Ptr;
}
void ReleaseEpsgObjectPtr ()
{
	delete csEpsgDataSetV6Ptr;
	csEpsgDataSetV6Ptr = 0;
}

// Replaces an occurence of the provided "find" string with the provided
// "rplWith" string, in the provided string.  Note:
// 1> Search is NOT case sensitive.
// 2> results are returned in the original buffer.
// 3> result is always null terminated, and
// 4> never longer than strSize -1 characters.
// 5> Function does nothing silently if strSize >= 1024.

bool CS_strrpl (char* string1,size_t strSize,const char* find,const char* rplWith)
{
	bool ok (false);
	
	size_t count1;
	size_t count2;
	size_t count3;
	size_t count4;
	
	char* cPtr;

	char wrkStr [1024];

	if (strSize < 1024)
	{
		const char* cPtrK = CS_stristr (string1,find);
		if (cPtrK != 0)
		{
			count1 = (cPtrK - string1);
			count2 = strlen (find);
			count3 = strlen (rplWith);
			count4 = strlen (string1) - count1 - count2;

			cPtr = CS_stncp (wrkStr,string1,(int)(count1 + 1));
			cPtr = CS_stncp (cPtr,rplWith,(int)(count3 + 1));
			cPtr = CS_stncp (cPtr,(string1 + count1 + count2),(int)(count4 + 1));
			
			CS_stncp (string1,wrkStr,(int)strSize);
			ok = true;
		}
	}
	return ok;
}

int CS_nmMprRplName (TcsCsvFileBase& csvFile,short fldNbr,const char* oldName,
														  const char* newName,
														  bool once)
{
	bool ok;					// get the primary loop started

	int rplCount (0);
	unsigned idx;
	unsigned recCount;
	const char* kCp;

	char chrFieldData [512];
	wchar_t wcFieldData [512];

	std::wstring fieldData;
	TcsCsvStatus csvStatus;

	recCount = csvFile.RecordCount ();
	ok = (recCount > 0);
	for (idx = 0;ok && idx < recCount;idx += 1)
	{
		kCp = 0;
		ok= csvFile.GetField (fieldData,idx,fldNbr,csvStatus);
		if (ok)
		{
			wcstombs (chrFieldData,fieldData.c_str (),sizeof (chrFieldData));
			chrFieldData [sizeof (chrFieldData) - 1] = '\0';
			ok = (strlen (chrFieldData) < (sizeof (chrFieldData) - 1));
			if (ok)
			{
				kCp = CS_stristr (chrFieldData,oldName);
			}
		}
		if (kCp == 0)
		{
			continue;
		}
		
		// We have found a record with this name as one of the aliases.
		// Replace the name in the field data, ad then in the csvTable.
		CS_strrpl (chrFieldData,sizeof (chrFieldData),oldName,newName);
		mbstowcs (wcFieldData,chrFieldData,wcCount (wcFieldData));
		fieldData = std::wstring (wcFieldData);
		ok = csvFile.ReplaceField (fieldData,idx,fldNbr,csvStatus);
		rplCount += 1;

		// If once is true, then we are to only make one replacement.
		if (!ok || once)
		{
			break;
		}
	}
	return ok?rplCount:-1;
}
//newPage//
EcsIntersectionType CS_intersection2D (const double firstFrm  [2],
									   const double firstTo   [2],
									   const double secondFrm [2],
									   const double secondTo  [2],
									   double intersection [2])
{
	EcsIntersectionType result;
	double delX1, delY1;
	double delX2, delY2;
	double num, denom;			// numerator and denominator
	double fuzz;

	// Compute a value which we will use as a substitute for zero.  We start
	// with the absolute value of the largest of all the coordinates in the
	// calculation.
	fuzz = fabs (firstFrm [0]);
	if (fabs (firstFrm  [1]) > fuzz) fuzz = fabs (firstFrm  [1]); 
	if (fabs (firstTo   [0]) > fuzz) fuzz = fabs (firstTo   [0]); 
	if (fabs (firstTo   [1]) > fuzz) fuzz = fabs (firstTo   [1]); 
	if (fabs (secondFrm [0]) > fuzz) fuzz = fabs (secondFrm [0]); 
	if (fabs (secondFrm [1]) > fuzz) fuzz = fabs (secondFrm [1]); 
	if (fabs (secondTo  [0]) > fuzz) fuzz = fabs (secondTo  [0]); 
	if (fabs (secondTo  [1]) > fuzz) fuzz = fabs (secondTo  [1]);

	// Take the magnitude of the largest coordinate involved in this
	// and divide by 100 billion.  That is, we assume that any two numbers
	// where the first 12 digits of precision are the same, are the same
	// number.  Due to calculation slop using real numbers, we rarely
	// encounter the cold hard zero value which indicates that the two
	// lines are -arallel or colinear.
	fuzz *= 1.0E-12;

	// Compute the denominator which also tells us if the lines are
	// parallel or collinear.
	delX1 = firstTo  [0] - firstFrm  [0];
	delY1 = firstTo  [1] - firstFrm  [1];
	delX2 = secondTo [0] - secondFrm [0];
	delY2 = secondTo [1] - secondFrm [1];
	denom = delX2 * delY1 - delX1 * delY2;

	// If parallel or colinear, we can't do much.  There is no intersection
	// except at infinity.
	if (fabs (denom) < fuzz)
	{
		return (isectParallel);
	}

	// Compute the intersection.
	num = delX1 * delX2 * (secondFrm [1] - firstFrm [1]) +
		  delX2 * delY1 *  firstFrm  [0] -
		  delX1 * delY2 *  secondFrm [0];
	intersection [0] = num / denom;

	num = delY1 * delY2 * (secondFrm [0] - firstFrm [0]) +	
		  delY2 * delX1 *  firstFrm [1] -
		  delY1 * delX2 *  secondFrm [1];
	intersection [1] = num / -denom;

	// Now to compute the location of the intersection point relative to the
	// line segments involved. This is often very important.  That is, does the
	// actual intersection point reside on either line; perhaps its on both
	// lines.  We start by assuming the the intersection point does not reside
	// on either line.

	// We write code which works independent of whatever values are
	// assigned to the intersection type enumerator. A bit slower, a
	// bit larger, but much more robust.
	result = isectNeither;
	if (fabs (delX1) > fabs (delY1))
	{
		// first line segment is more horizontal than vertical. We will use the
		// x value to test if the resulting point is on the segment.
		if (delX1 >= 0.0)
		{
			if (intersection [0] >= firstFrm [0] && intersection [0] <= firstTo [0])
			{
				result = isectFirst;
			}
		}
		else
		{
			if (intersection [0] <= firstFrm [0] && intersection [0] >= firstTo [0])
			{
				result = isectFirst;
			}
		}
	}
	else
	{
		// first segment is more verical than horizontal. We will use the y
		// value to test if the resulting point is on the segment.
		if (delY1 >= 0.0)
		{
			if (intersection [1] >= firstFrm [1] && intersection [1] <= firstTo [1])
			{
				result = isectFirst;
			}
		}
		else
		{
			if (intersection [1] <= firstFrm [1] && intersection [1] >= firstTo [1])
			{
				result = isectFirst;
			}
		}
	}

	// Same stuff with the second segment.
	if (fabs (delX2) > fabs (delY2))
	{
		if (delX2 >= 0.0)
		{
			if (intersection [0] >= secondFrm [0] && intersection [0] <= secondTo [0])
			{
				 result = (result == isectFirst) ? isectBoth : isectSecond;
			}
		}
		else
		{
			if (intersection [0] <= secondFrm [0] && intersection [0] >= secondTo [0])
			{
				 result = (result == isectFirst) ? isectBoth : isectSecond;
			}
		}
	}
	else
	{
		if (delY2 >= 0.0)
		{
			if (intersection [1] >= secondFrm [1] && intersection [1] <= secondTo [1])
			{
				 result = (result == isectFirst) ? isectBoth : isectSecond;
			}
		}
		else
		{
			if (intersection [1] <= secondFrm [1] && intersection [1] >= secondTo [1])
			{
				 result = (result == isectFirst) ? isectBoth : isectSecond;
			}
		}
	}
	return result;
}
//newPage//
///////////////////////////////////////////////////////////////////////////////
// TcsCoordsysFile Object  --  Abstracts the CS-MAP Coordsys file.
//
// This object is used to access CS-MAP Coordsys files.
//
// Standard sort function. The constructor uses this function to sort the
// array thus enabling use of binary search to get coordinate systems by
// key name.
bool TcsCoordsysFile::KeyNameLessThan (const cs_Csdef_& lhs,const cs_Csdef_& rhs)
{
	int cmpValue = CS_stricmp (lhs.key_nm,rhs.key_nm);
	return (cmpValue < 0);
}
//
// Function used to sort the dictionary image into "CRS" order.  That is,
// to bring all CRS's which are identical as far as projection and basic
// projection parameters together.  Originally devised to connect CRS
// definitions without a useful range with "identical" CRS definitions
// which did have a useful range.  Identical, in this case, implies that
// datum and unit differences are ignored, as they would have minimal
// effect on the the geographic useful range.
// 
int TcsCoordsysFile::ProjectionCompare (const cs_Csdef_& lhs,const cs_Csdef_& rhs)
{
	int cmpValue;
	unsigned short lhsPrjCode;
	unsigned short rhsPrjCode;
	double tmpDbl;

	lhsPrjCode = CS_getPrjCode (lhs.prj_knm);
	rhsPrjCode = CS_getPrjCode (rhs.prj_knm);
	cmpValue = lhsPrjCode - rhsPrjCode;
	if (cmpValue == 0)
	{
		if (lhsPrjCode == 1)
		{
			cmpValue = CS_stricmp (lhs.dat_knm,rhs.dat_knm);
		}
		else
		{
			tmpDbl = lhs.org_lng - rhs.org_lng;
			if (tmpDbl < -1.0E-05)
			{
				cmpValue = -1;
			}
			else if (tmpDbl > 1.0E-05)
			{
				cmpValue = 1;
			}
			if (cmpValue == 0)
			{
				tmpDbl = lhs.org_lat - rhs.org_lat;
				if (tmpDbl < -1.0E-05)
				{
					cmpValue = -1;
				}
				else if (tmpDbl > 1.0E-05)
				{
					cmpValue = 1;
				}
			}
		}
		if (cmpValue == 0)
		{
			tmpDbl = lhs.prj_prm1 - rhs.prj_prm1;
			if (tmpDbl < -1.0E-05)
			{
				cmpValue = -1;
			}
			else if (tmpDbl > 1.0E-05)
			{
				cmpValue = 1;
			}
		}
		if (cmpValue == 0)
		{
			tmpDbl = lhs.prj_prm2 - rhs.prj_prm2;
			if (tmpDbl < -1.0E-05)
			{
				cmpValue = -1;
			}
			else if (tmpDbl > 1.0E-05)
			{
				cmpValue = 1;
			}
		}
		if (cmpValue == 0)
		{
			tmpDbl = lhs.prj_prm3 - rhs.prj_prm3;
			if (tmpDbl < -1.0E-05)
			{
				cmpValue = -1;
			}
			else if (tmpDbl > 1.0E-05)
			{
				cmpValue = 1;
			}
		}
		if (cmpValue == 0)
		{
			tmpDbl = lhs.prj_prm4 - rhs.prj_prm4;
			if (tmpDbl < -1.0E-05)
			{
				cmpValue = -1;
			}
			else if (tmpDbl > 1.0E-05)
			{
				cmpValue = 1;
			}
		}
	}
	return cmpValue;
}
bool TcsCoordsysFile::ProjectionLessThan (const cs_Csdef_& lhs,const cs_Csdef_& rhs)
{
	int cmpValue;
	
	cmpValue = ProjectionCompare (lhs,rhs);

	// If we are still the same at this point, we simply force a definition
	// with a useful range to preceed a definition which does not have a
	// useful range.
	if (cmpValue == 0)
	{
		bool lhsHasRng = CS_crsHasUsefulRng (lhs);
		bool rhsHasRng = CS_crsHasUsefulRng (rhs);
		if (lhsHasRng && !rhsHasRng)
		{
			cmpValue = -1;
		}
		else if (rhsHasRng && !lhsHasRng)
		{
			cmpValue = +1;
		}
	}
	return (cmpValue < 0);
}

TcsCoordsysFile::TcsCoordsysFile () : Ok                (false),
									  SortedByName      (false),
									  CurrentIndex      (0),
									  CoordinateSystems ()
{
	int myCrypt;
	struct cs_Csdef_ csDef;

	csFILE* csStrm = CS_csopn (_STRM_BINRD);
	if (csStrm != NULL)
	{
		while (CS_csrd (csStrm,&csDef,&myCrypt))
		{
			CoordinateSystems.push_back (csDef);
		}

		// Sort by key name to assure a binary searchable order.
		std::sort (CoordinateSystems.begin (),CoordinateSystems.end (),KeyNameLessThan);
		SortedByName = true;
		CS_fclose (csStrm);
		Ok = true;
	}
}
TcsCoordsysFile::~TcsCoordsysFile ()
{
}
const cs_Csdef_* TcsCoordsysFile::FetchCoordinateSystem (size_t index) const
{
	const struct cs_Csdef_* rtnValue = 0;
	if (index < CoordinateSystems.size ())
	{
		rtnValue = &CoordinateSystems [index];
	}
	return rtnValue;
}
const cs_Csdef_* TcsCoordsysFile::FetchCoordinateSystem (const char* keyName) const
{
	const struct cs_Csdef_* csDefPtr = 0;
	struct cs_Csdef_ locateValue;
	std::vector<struct cs_Csdef_>::const_iterator locItr;

	memset (&locateValue,'\0',sizeof (locateValue));
	CS_stncp (locateValue.key_nm,keyName,sizeof (locateValue.key_nm));
	if (SortedByName)
	{
		locItr = std::lower_bound (CoordinateSystems.begin (),CoordinateSystems.end (),locateValue,KeyNameLessThan);
		if (locItr != CoordinateSystems.end ())
		{
			csDefPtr = &(*locItr);
			if (CS_stricmp (csDefPtr->key_nm,keyName))
			{
				csDefPtr = 0;
			}
		}
	}
	else
	{
		for (locItr = CoordinateSystems.begin ();locItr != CoordinateSystems.end ();locItr++)
		{
			csDefPtr = &(*locItr);
			if (!CS_stricmp (csDefPtr->key_nm,keyName))
			{
				break;
			}
		}
	}
	return csDefPtr;
}
const struct cs_Csdef_* TcsCoordsysFile::FetchNextCs (void)
{
	const struct cs_Csdef_* csDefPtr = FetchCoordinateSystem (CurrentIndex);
	if (csDefPtr != 0)
	{
		CurrentIndex += 1;
	}
	return csDefPtr;
}
void TcsCoordsysFile::OrderByProjection (void)
{
	SortedByName = false;
	std::sort (CoordinateSystems.begin (),CoordinateSystems.end (),ProjectionLessThan);
}

//newPage//
///////////////////////////////////////////////////////////////////////////////
// TcsDatumsFile Object  --  Abstracts the CS-MAP Datums file.
//
// This object is used to access CS-MAP Datums files.
//
bool TcsDatumsFile::KeyNameLessThan (const cs_Dtdef_& lhs,const cs_Dtdef_& rhs)
{
	int cmpValue = CS_stricmp (lhs.key_nm,rhs.key_nm);
	return (cmpValue < 0);
}
TcsDatumsFile::TcsDatumsFile () : Ok           (false),
								  CurrentIndex (0),
								  Datums       ()
{
	int myCrypt;
	cs_Dtdef_ dtDef;

	csFILE* dtStrm = CS_dtopn (_STRM_BINRD);
	if (dtStrm != NULL)
	{
		while (CS_dtrd (dtStrm,&dtDef,&myCrypt))
		{
			Datums.push_back (dtDef);
		}	

		// Sort by key name to assure a binary searchable order.
		std::sort (Datums.begin (),Datums.end (),KeyNameLessThan);
		CS_fclose (dtStrm);
		Ok = true;
	}
}
TcsDatumsFile::~TcsDatumsFile ()
{
}
const cs_Dtdef_* TcsDatumsFile::FetchDatum (size_t index) const
{
	const cs_Dtdef_* rtnValue = 0;
	if (index < Datums.size ())
	{
		rtnValue = &Datums [index];
	}
	return rtnValue;
}
const cs_Dtdef_* TcsDatumsFile::FetchDatum (const char* keyName) const
{
	const cs_Dtdef_* dtDefPtr = 0;
	cs_Dtdef_ locateValue;
	std::vector<cs_Dtdef_>::const_iterator locItr;
	
	CS_stncp (locateValue.key_nm,keyName,sizeof (locateValue.key_nm));
	locItr = std::lower_bound (Datums.begin (),Datums.end (),locateValue,KeyNameLessThan);
	if (locItr != Datums.end ())
	{
		dtDefPtr = &(*locItr);
		if (CS_stricmp (dtDefPtr->key_nm,keyName))
		{
			dtDefPtr = 0;
		}
	}
	return dtDefPtr;
}
const struct cs_Dtdef_* TcsDatumsFile::FetchNextDt (void)
{
	const struct cs_Dtdef_* dtDefPtr = FetchDatum (CurrentIndex);
	if (dtDefPtr != 0)
	{
		CurrentIndex += 1;
	}
	return dtDefPtr;
}
//newPage//
///////////////////////////////////////////////////////////////////////////////
// TcsElipsoidFile Object  --  Abstracts the CS-MAP Elipsoid file.
//
// This object is used to access CS-MAP Elipsoid files.
//
// Note, that the incorrect spelling (Elipsoid) is often used as that was the
// actual name of the file when it was originally invented; which dates back
// to the days where file names could not exceed 8 characters.
//
bool TcsElipsoidFile::KeyNameLessThan (const cs_Eldef_& lhs,const cs_Eldef_& rhs)
{
	int cmpValue = CS_stricmp (lhs.key_nm,rhs.key_nm);
	return (cmpValue < 0);
}
TcsElipsoidFile::TcsElipsoidFile () : Ok      (false),
									  CurrentIndex (0),
									  Ellipsoids   ()
{
	int myCrypt;
	cs_Eldef_ elDef;

	csFILE* elStrm = CS_elopn (_STRM_BINRD);
	if (elStrm != NULL)
	{
		while (CS_elrd (elStrm,&elDef,&myCrypt))
		{
			Ellipsoids.push_back (elDef);
		}	

		// Sort by key name to assure a binary searchable order.
		std::sort (Ellipsoids.begin (),Ellipsoids.end (),KeyNameLessThan);
		CS_fclose (elStrm);
		Ok = true;
	}
}
TcsElipsoidFile::~TcsElipsoidFile ()
{
	// TcsFile closes on destruction.
}
const cs_Eldef_* TcsElipsoidFile::FetchEllipsoid (size_t index) const
{
	const cs_Eldef_* rtnValue = 0;
	if (index < Ellipsoids.size ())
	{
		rtnValue = &Ellipsoids [index];
	}
	return rtnValue;
}
const cs_Eldef_* TcsElipsoidFile::FetchEllipsoid (const char* keyName) const
{
	const cs_Eldef_* elDefPtr = 0;
	cs_Eldef_ locateValue;
	std::vector<cs_Eldef_>::const_iterator locItr;
	
	CS_stncp (locateValue.key_nm,keyName,sizeof (locateValue.key_nm));
	locItr = std::lower_bound (Ellipsoids.begin (),Ellipsoids.end (),locateValue,KeyNameLessThan);
	if (locItr != Ellipsoids.end ())
	{
		elDefPtr = &(*locItr);
		if (CS_stricmp (elDefPtr->key_nm,keyName))
		{
			elDefPtr = 0;
		}
	}
	return elDefPtr;
}
const struct cs_Eldef_* TcsElipsoidFile::FetchNextEl (void)
{
	const struct cs_Eldef_* elDefPtr = FetchEllipsoid (CurrentIndex);
	if (elDefPtr != 0)
	{
		CurrentIndex += 1;
	}
	return elDefPtr;
}
//newPage//
TcsCategoryItem::TcsCategoryItem (void) : ToBeDeleted (false)
{
	ItemName [0] = '\0';
	Description [0] = '\0';
}
TcsCategoryItem::TcsCategoryItem (const char* itemName,const char* description) : ToBeDeleted (false)
{
	CS_stncp (ItemName,itemName,sizeof (ItemName));
	Description [0] = '\0';
	if (description != 0)
	{
		CS_stncp (Description,description,sizeof (Description));
	}
}
TcsCategoryItem::TcsCategoryItem (const TcsCategoryItem& source) : ToBeDeleted (source.ToBeDeleted)
{
	CS_stncp (ItemName,source.ItemName,sizeof (ItemName));
	CS_stncp (Description,source.Description,sizeof (Description));
}
TcsCategoryItem::~TcsCategoryItem (void)
{
}
TcsCategoryItem& TcsCategoryItem::operator= (const TcsCategoryItem& rhs)
{
	if (&rhs != this)
	{
		ToBeDeleted = rhs.ToBeDeleted;
		CS_stncp (ItemName,rhs.ItemName,sizeof (ItemName));
		CS_stncp (Description,rhs.Description,sizeof (Description));
	}
	return *this;
}
bool TcsCategoryItem::ReadFromStream (std::istream& inStrm)
{
	bool ok (false);

	char *cp;
	char wrkBufr [1024];
	char lineBufr [1024];

	ItemName [0] = '\0';
	Description [0] = '\0';
	if (inStrm.good ())
	{
		do
		{
			inStrm.getline (lineBufr,sizeof (lineBufr),'\n');
			CS_trim (lineBufr);
		} while (lineBufr [0] == ';');

		cp = strchr (lineBufr,'=');
		if (cp != 0)
		{
			*cp++ = '\0';
			CS_stncp (wrkBufr,lineBufr,sizeof (wrkBufr));
			CS_trim (wrkBufr);
			CS_stncp (ItemName,wrkBufr,sizeof (ItemName));
			CS_stncp (wrkBufr,cp,sizeof (wrkBufr));
			CS_trim (wrkBufr);
			CS_stncp (Description,wrkBufr,sizeof (Description));
			ok = true;
		}
	}
	return ok;
}
void TcsCategoryItem::SetToBeDeleted (bool toBeDeleted)
{
	ToBeDeleted = toBeDeleted;
}
void TcsCategoryItem::SetItemName (const char* itemName)
{
	CS_stncp (ItemName,itemName,sizeof (ItemName));
}
void TcsCategoryItem::SetDescription (const char* description)
{
	CS_stncp (Description,description,sizeof (Description));
}
bool TcsCategoryItem::WriteToStream (std::ostream& outStrm)
{
	outStrm << " "
			<< ItemName
			<< " = "
			<< Description
			<< std::endl;
	return (outStrm.good ());
}

TcsCategory::TcsCategory (void) : Items ()
{
	CategoryName [0] = '\0';
}
TcsCategory::TcsCategory (const char* categoryName) : Items ()
{
	CS_stncp (CategoryName,categoryName,sizeof (CategoryName));
}
TcsCategory::TcsCategory (const TcsCategory& source) : Items (source.Items)
{
	CS_stncp (CategoryName,source.CategoryName,sizeof (CategoryName));
}
TcsCategory::~TcsCategory (void)
{
}
TcsCategory& TcsCategory::operator= (const TcsCategory& rhs)
{
	if (&rhs != this)
	{
		CS_stncp (CategoryName,rhs.CategoryName,sizeof (CategoryName));
		Items = rhs.Items;
	}
	return *this;
}
size_t TcsCategory::GetItemCount (void) const
{
	return Items.size ();
}
const char* TcsCategory::GetCategoryName (void) const
{
	return CategoryName;
}
char* TcsCategory::GetCategoryName (void)
{
	return CategoryName;
}
const TcsCategoryItem* TcsCategory::GetItem (size_t index) const
{
	const TcsCategoryItem* itemPtr = 0;

	if (index < Items.size ())
	{
		itemPtr = &Items [index];
	}
	return itemPtr;
}
TcsCategoryItem* TcsCategory::GetItem (size_t index)
{
	TcsCategoryItem* itemPtr = 0;

	if (index < Items.size ())
	{
		itemPtr = &Items [index];
	}
	return itemPtr;
}
const TcsCategoryItem* TcsCategory::GetItem (const char* itemName) const
{
	const char *itmNamePtr;
	const TcsCategoryItem* itemPtr = 0;
	std::vector<TcsCategoryItem>::const_iterator itmItr;
	
	for (itmItr = Items.begin ();itmItr != Items.end ();itmItr++)
	{
		itmNamePtr = itmItr->GetItemName ();
		if (!CS_stricmp (itmNamePtr,itemName))
		{
			itemPtr = &(*itmItr);
			break;
		}
	}
	return itemPtr;
}
TcsCategoryItem* TcsCategory::GetItem (const char* itemName)
{
	const char *itmNamePtr;
	TcsCategoryItem* itemPtr = 0;
	std::vector<TcsCategoryItem>::iterator itmItr;
	
	for (itmItr = Items.begin ();itmItr != Items.end ();itmItr++)
	{
		itmNamePtr = itmItr->GetItemName ();
		if (!CS_stricmp (itmNamePtr,itemName))
		{
			itemPtr = &(*itmItr);
			break;
		}
	}
	return itemPtr;
}
void TcsCategory::SetCategoryName (const char* categoryName)
{
	CS_stncp (CategoryName,categoryName,sizeof (CategoryName));
}
void TcsCategory::AddItem (const TcsCategoryItem& newItem)
{
	Items.push_back (newItem);
}
void TcsCategory::RemoveItem (const TcsCategoryItem& oldItem)
{
	const char *itmNamePtr;
	std::vector<TcsCategoryItem>::iterator itmItr;
	
	for (itmItr = Items.begin ();itmItr != Items.end ();itmItr++)
	{
		itmNamePtr = itmItr->GetItemName ();
		if (!CS_stricmp (itmNamePtr,oldItem.GetItemName()))
		{
			Items.erase (itmItr);
			break;
		}
	}
}
bool TcsCategory::ReadFromStream (std::istream& inStrm)
{
	bool ok (false);

	char *cp1, *cp2;
	char lineBufr [1024];

	CategoryName [0] = '\0';
	if (inStrm.good ())
	{
		do
		{
			inStrm.getline (lineBufr,sizeof (lineBufr),'\n');
			CS_trim (lineBufr);
		} while (lineBufr [0] == ';');

		cp1 = strchr (lineBufr,'[');
		if (cp1 != 0)
		{
			cp1 += 1;
			cp2 = strchr (cp1,']');
			if (cp2 != 0)
			{
				*cp2 = '\0';
				CS_stncp (CategoryName,cp1,sizeof (CategoryName));
				bool itmOk = true;
				while (itmOk)
				{
					TcsCategoryItem newItem;
					itmOk = newItem.ReadFromStream (inStrm);
					if (itmOk)
					{
						Items.push_back (newItem);
					}
				}
				ok = true;
			}
		}
	}
	return ok;	
}
bool TcsCategory::WriteToStream (std::ostream& outStrm)
{
	std::vector<TcsCategoryItem>::iterator itmItr;

	outStrm << '['
			<< CategoryName
			<< ']'
			<< std::endl;
	for (itmItr = Items.begin ();itmItr != Items.end ();itmItr++)
	{
		if (!itmItr->IsToBeDeleted ())
		{
			itmItr->WriteToStream (outStrm);
		}
	}
	outStrm << std::endl;
	return (outStrm.good ());
}
//newPage//
const char TcsCategoryFile::KcsObsoleteCatName [] = "Obsolete Coordinate Systems";
TcsCategoryFile::TcsCategoryFile (void) : CopyrightNotice (),
										  Categories ()
{
}
TcsCategoryFile::TcsCategoryFile (std::istream& inStrm) : CopyrightNotice (),
														  Categories ()
{
	ReadFromStream (inStrm);
}
TcsCategoryFile::~TcsCategoryFile (void)
{
}
const TcsCategory* TcsCategoryFile::FetchCategory (size_t index) const
{
	const TcsCategory* categoryPtr = 0;
	
	if (index < Categories.size ())
	{
		categoryPtr = &Categories [index];
	}
	return categoryPtr;
}
TcsCategory* TcsCategoryFile::FetchCategory (size_t index)
{
	TcsCategory* categoryPtr = 0;

	if (index < Categories.size ())
	{
		categoryPtr = &Categories [index];
	}
	return categoryPtr;
}
const TcsCategory* TcsCategoryFile::FetchCategory (const char* categoryName) const
{
	const char *categoryNamePtr;
	const TcsCategory* categoryPtr = 0;
	std::vector<TcsCategory>::const_iterator catItr;
	
	for (catItr = Categories.begin ();catItr != Categories.end ();catItr++)
	{
		categoryNamePtr = catItr->GetCategoryName ();
		if (!CS_stricmp (categoryNamePtr,categoryName))
		{
			categoryPtr = &(*catItr);
			break;
		}
	}
	return categoryPtr;
}
TcsCategory* TcsCategoryFile::FetchCategory (const char* categoryName)
{
	char *categoryNamePtr;
	TcsCategory* categoryPtr = 0;
	std::vector<TcsCategory>::iterator catItr;
	
	for (catItr = Categories.begin ();catItr != Categories.end ();catItr++)
	{
		categoryNamePtr = catItr->GetCategoryName ();
		if (!CS_stricmp (categoryNamePtr,categoryName))
		{
			categoryPtr = &(*catItr);
			break;
		}
	}
	return categoryPtr;
}
void TcsCategoryFile::AddCategory (const TcsCategory& newCategory)
{
	Categories.push_back (newCategory);
}
bool TcsCategoryFile::ReadFromStream (std::istream& inStrm)
{
	bool ok (false);
	
	char lineBufr [1024];

	while (inStrm.good ())
	{
		std::ifstream::pos_type fPos;
		fPos = inStrm.tellg ();
		inStrm.getline (lineBufr,sizeof (lineBufr),'\n');
		CS_trim (lineBufr);
		if (lineBufr [0] == '\0' || lineBufr [0] == ';')
		{
			if (lineBufr [0] != '\0')
			{
				CopyrightNotice += lineBufr;
			}
			CopyrightNotice += '\n';
		}
		else
		{
			inStrm.seekg (fPos);
			break;
		}
	}

	while (inStrm.good ())
	{
		bool catOk = true;
		while (catOk)
		{
			while (inStrm.good ())
			{
				int ccc = inStrm.peek();
				if (ccc == '[') break;
				inStrm.get();
			}
			catOk = inStrm.good();
			if (catOk)
			{
				TcsCategory nextCategory;
				catOk = nextCategory.ReadFromStream (inStrm);
				if (catOk)
				{
					Categories.push_back (nextCategory);
					ok = true;
				}
			}
		}
		ok = true;
	}
	return ok;
}
bool TcsCategoryFile::InitializeFromFile (const char* categoryPathName)
{
	bool ok;
	std::ifstream inStrm;
	
	CopyrightNotice = "";
	Categories.clear ();

	inStrm.open (categoryPathName,std::ios_base::in);
	ok = inStrm.is_open ();
	if (ok)
	{
		ok = ReadFromStream (inStrm);
		inStrm.close ();
	}
	return ok;
}
bool TcsCategoryFile::DeprecateCrs (const char* oldCrsName,const char* newCrsName,
														   const char* deprNote,
														   const char* newCrsDescription)
{
	bool ok (false);
	unsigned rplCount;
	TcsCategory* obsoleteCat;
	TcsCategory* categoryPtr;
	TcsCategoryItem* itemPtr;
	std::vector<TcsCategory>::iterator catItr;

	char itemDescription [256];

	// We will need to add a copy of every entry that we modify to the
	// Obsolete category, so we lobtain a pointer to that category now.
	obsoleteCat = FetchCategory (TcsCategoryFile::KcsObsoleteCatName);
	ok = (obsoleteCat != 0);

	// Formulate the description which we will apply to the deprecated
	// category item.
	sprintf (itemDescription,"%s Replaced by %s.",deprNote,newCrsName);

	// Scan all catagories looking for an item with an item name equal to
	// the provided old CRS name.
	// NOTICE: This code currently assumes that a CSR name is never duplicated
	// with a category.
	rplCount = 0;
	for (catItr = Categories.begin ();ok && catItr != Categories.end ();catItr++)
	{
		categoryPtr = &(*catItr);
		if (categoryPtr == obsoleteCat)
		{
			continue;
		}
		itemPtr = categoryPtr->GetItem (oldCrsName);
		if (itemPtr != 0)
		{
			// We have found a reference to this CRS in current category.
			// If we have not done so already, make sure the obsolete category
			// has a refernce to the CRS we are deprecating.
			if (rplCount == 0)
			{
				TcsCategoryItem deprecatedItem (*itemPtr);
				deprecatedItem.SetDescription (itemDescription);
				obsoleteCat->AddItem (deprecatedItem);
			}
			rplCount += 1;

			// Replace the old name of this entry with the new name and
			// new description, is any.
			// We're removing an item from a category.  This does not affect
			// the category iterator.
			categoryPtr->RemoveItem (*itemPtr);
		}
	}
	if (ok)
	{
		ok = (rplCount > 0);
	}
	return ok;
}
bool TcsCategoryFile::DeprecateCrsDel (const char* oldCrsName,const char* useCrs,
															  const char* deprNote)
{
	bool ok (false);
	unsigned rplCount;
	TcsCategory* obsoleteCat;
	TcsCategory* categoryPtr;
	TcsCategoryItem* itemPtr;
	std::vector<TcsCategory>::iterator catItr;

	char itemDescription [256];

	// We will need to add a copy of every entry that we modify to the
	// Obsolete category, so we obtain a pointer to that category now.
	obsoleteCat = FetchCategory (TcsCategoryFile::KcsObsoleteCatName);
	ok = (obsoleteCat != 0);

	// Formulate the description which we will apply to the deprecated
	// category item.
	sprintf (itemDescription,"%s - use %s for new work.",deprNote,useCrs);

	// Scan all catagories looking for an item with an item name equal to
	// the provided old CRS name.
	rplCount = 0;
	for (catItr = Categories.begin ();ok && catItr != Categories.end ();catItr++)
	{
		categoryPtr = &(*catItr);
		if (categoryPtr == obsoleteCat)
		{
			continue;
		}
		itemPtr = categoryPtr->GetItem (oldCrsName);
		if (itemPtr != 0)
		{
			// We have found a reference to this CRS in current category.
			// If we have not done so already, make sure the obsolete category
			// has a refernce to the CRS we are deprecating.
			if (rplCount == 0)
			{
				TcsCategoryItem deprecatedItem (*itemPtr);
				deprecatedItem.SetDescription (itemDescription);
				obsoleteCat->AddItem (deprecatedItem);
			}
			rplCount += 1;

			// Remove the deprecated item from this category.
			categoryPtr->RemoveItem (*itemPtr);
		}
	}
	if (ok)
	{
		ok = (rplCount > 0);
	}
	return ok;
}
bool TcsCategoryFile::WriteToStream (std::ostream& outStrm)
{
	bool ok;
	std::vector<TcsCategory>::iterator catItr;

	ok = outStrm.good ();
	if (ok)
	{
		outStrm << CopyrightNotice
				<< std::endl;

		for (catItr = Categories.begin ();ok && catItr != Categories.end ();catItr++)
		{
			ok = catItr->WriteToStream (outStrm);
		}
		outStrm << std::endl;
	}
	return ok;
}
bool TcsCategoryFile::WriteToFile (const char* categoryPathName)
{
	bool ok;
	std::ofstream outStrm;
	outStrm.open (categoryPathName,std::ios_base::out | std::ios_base::trunc);
	ok = outStrm.is_open ();
	if (ok)
	{
		ok = WriteToStream (outStrm);
		outStrm.close ();
	}
	return ok;
}
//newPage//
///////////////////////////////////////////////////////////////////////////////
// TcsMrtFile Object  --  Abstracts the CS-MAP Multiple Regression file.
//
// This object is used to access CS-MAP Multiple Regression (.MRT) data files.
// The constructor fails if the file is not found where it is expected or
// anyother opening problem.  .MRT files were never really encrypted. 
//
TcsMrtFile::TcsMrtFile (const char* filePath) : Ok (false)
{
	size_t rdCnt (0);

	memset ((void*)&MregDefinition,0,sizeof (MregDefinition));

	// Open the file in binary read mode and read the entire thing into the
	// structure which is our only data member.  In short, the MregDefinition
	// object is simply an in memory image of the file.  Note that the file
	// we are reading will rarely be the maximum size indicated by the
	// struct csDmaMreg_ object.  We simply assume (ass-u-me) that there
	// is no error wohich would be indicated by a returned read count less
	// than the amount of data requrested.
	//
	// The magic number is in the struct, so we don't need to do anything
	// special here.
	csFILE* csStrm = CS_fopen (filePath,_STRM_BINRD);
	if (csStrm != NULL)
	{
		rdCnt  = CS_fread ((void*)&MregDefinition,1,sizeof (MregDefinition),csStrm);
		Ok = feof (csStrm) && !ferror (csStrm);
		CS_fclose (csStrm);
	}	

	// Swap the byes incase we're running on a Big Endian machine.
	CS_bswap (&MregDefinition,cs_BSWP_DMAMREG);
	
	// Verify the magic number.
	if (MregDefinition.magic != (long)cs_MULREG_MAGIC ||
	    static_cast<long>(rdCnt) != MregDefinition.mr_size)
	{
		Ok = false;
	}
}
TcsMrtFile::~TcsMrtFile ()
{
}
// The technique used here is not very efficient, but it is expected that this
// function will only be used in the conversion process.  That is, used a few
// times during installation and setup, and never again used.  So, we opt for
// simplicity of the calling sequence.
double TcsMrtFile::GetLambdaCoeff (size_t uuPwr,size_t vvPwr) const
{
	unsigned wrd;
	unsigned uuIdx, vvIdx;
	unsigned bitNbr;
	unsigned long mask;

	int lambdaIdx = 0;
	double coefficient = cs_Zero;

	// Start a double iteration which checks bits in the appropriate
	// bitmap.  Each 'true' bit represents a valid coefficient in the
	// coefs array in the definition structure.
	for (vvIdx = 0;vvIdx < MregDefinition.max_vv;vvIdx += 1)
	{
		for (uuIdx = 0;uuIdx < MregDefinition.max_uu;uuIdx += 1)
		{
			// Compute the bit map bit number for this iteration.
			bitNbr = (unsigned)(vvIdx * 10 + uuIdx);
			wrd = bitNbr >> 5;
			mask = (unsigned long)0x80000000L >> (bitNbr & 0x1F);

			// See if this term is in the Lambda calculation.
			if ((MregDefinition.lng_map [wrd] & mask) != 0)
			{
				// Coefficient for the current loop indicies exists.
				if (vvIdx == vvPwr && uuIdx == uuPwr)
				{
					coefficient = MregDefinition.coefs [lambdaIdx];
					break;
				}
				lambdaIdx++;
			}
		}
	}
	return coefficient;
}
double TcsMrtFile::GetPhiCoeff (size_t uuPwr,size_t vvPwr) const
{
	unsigned wrd;
	unsigned uuIdx, vvIdx;
	unsigned bitNbr;
	unsigned long mask;

	int phiIdx = static_cast<int>(MregDefinition.lat_idx);
	double coefficient = cs_Zero;

	// Start a double iteration which checks bits in the appropriate
	// bitmap.  Each 'true' bit represents a valid coefficient in the
	// coefs arraybv in the definition structure.
	for (vvIdx = 0;vvIdx < MregDefinition.max_vv;vvIdx += 1)
	{
		for (uuIdx = 0;uuIdx < MregDefinition.max_uu;uuIdx += 1)
		{
			// Compute the bit map bit number for this iteration.
			bitNbr = (vvIdx * 10 + uuIdx);
			wrd = bitNbr >> 5;
			mask = 0x80000000UL >> (bitNbr & 0x1F);

			// See if this term is in the Lambda calculation.
			if ((MregDefinition.lat_map [wrd] & mask) != 0)
			{
				// Coefficient for the current loop indicies exists.
				if (vvIdx == vvPwr && uuIdx == uuPwr)
				{
					coefficient = MregDefinition.coefs [phiIdx];
					break;
				}
				phiIdx++;
			}
		}
	}
	return coefficient;
}
double TcsMrtFile::GetHgtCoeff (size_t uuPwr,size_t vvPwr) const
{
	unsigned wrd;
	unsigned uuIdx, vvIdx;
	unsigned bitNbr;
	unsigned long mask;

	int hgtIdx = static_cast<int>(MregDefinition.hgt_idx);
	double coefficient = cs_Zero;

	// Start a double iteration which checks bits in the appropriate
	// bitmap.  Each 'true' bit represents a valid coefficient in the
	// coefs arraybv in the definition structure.
	for (vvIdx = 0;vvIdx < MregDefinition.max_vv;vvIdx += 1)
	{
		for (uuIdx = 0;uuIdx < MregDefinition.max_uu;uuIdx += 1)
		{
			// Compute the bit map bit number for this iteration.
			bitNbr = (vvIdx * 10 + uuIdx);
			wrd = bitNbr >> 5;
			mask = 0x80000000UL >> (bitNbr & 0x1F);

			// See if this term is in the Lambda calculation.
			if ((MregDefinition.hgt_map [wrd] & mask) != 0)
			{
				// Coefficient for the current loop indicies exists.
				if (vvIdx == vvPwr && uuIdx == uuPwr)
				{
					coefficient = MregDefinition.coefs [hgtIdx];
					break;
				}
				hgtIdx++;
			}
		}
	}
	return coefficient;
}
//newPage//
///////////////////////////////////////////////////////////////////////////////
// TcsGdcEntry Object  --  Entry in a Geodetic Data Catalog
//
// This objects represents one object in a Geodetic data catalog.  A geodetic
// data catalog (TcsGdcFile) is basicly a collection (i.e. vector) of these
// things.
//
TcsGdcEntry::TcsGdcEntry () : Density (0.0),BufferSize (0L),
											Flags (0UL),
											Relative (0)
{
	PathName [0] = '\0';
}
TcsGdcEntry::TcsGdcEntry (const char* path,short relative,long bufferSize,unsigned long flags,
																		  double density)
																		      :
																		  Density (density),
																		  BufferSize (bufferSize),
																		  Flags (flags),
																		  Relative (relative)
{
	CS_stncp (PathName,path,sizeof (PathName));
}
TcsGdcEntry::TcsGdcEntry (const TcsGdcEntry& source) : Density (source.Density),
													   BufferSize (source.BufferSize),
													   Flags (source.Flags),
													   Relative (source.Relative)
{
	CS_stncp (PathName,source.PathName,sizeof (PathName));
}
TcsGdcEntry::~TcsGdcEntry (void)
{
	// Nothing to do here, yet.
}
TcsGdcEntry& TcsGdcEntry::operator= (const TcsGdcEntry& rhs)
{
	if (&rhs != this)
	{
		CS_stncp (PathName,rhs.PathName,sizeof (PathName));
		Density    = rhs.Density;
		BufferSize = rhs.BufferSize;
		Flags      = rhs.Flags;
		Relative   = rhs.Relative;
	}
	return *this;
}
TcsGdcFile::TcsGdcFile (const char* catalogPath) : Ok (false),InitialComment (),
															  MiddleComment (),
															  TrailingComment ()
{
	bool quote;

	short relative;
	unsigned long flags;
	long bufferSize;
	double density;

	char* cp;
	char* cpt;
	csFILE* gdcStrm;

	char relString [4];
	char relStringPar [4];
	char cTemp [csMAXPATH];
	char lineBufr [csMAXPATH + 64];
	char filePath [csMAXPATH];

	std::string comment;

	// Initialize the two arrays in our object.
	memset (FileFolder,'\0',sizeof (FileFolder));
	memset (Fallback,'\0',sizeof (Fallback));

	gdcStrm = NULL;

	// The following initializations are kind of hokey, but we want to
	// reference the cs_DirsepC variable.  Alternative is to use something
	// like sprintf.  That isn't very esthetic either.
	relString [0] = '.';
	relString [1] = cs_DirsepC;
	relString [2] = '\0';
	relString [3] = '\0';

	relStringPar [0] = '.';
	relStringPar [1] = '.';
	relStringPar [2] = cs_DirsepC;
	relStringPar [3] = '\0';

	// Extract the file folder portion of the file path to the catalog.
	// This is used to resolve relative path names.
	CS_stncp (filePath,catalogPath,sizeof (filePath));
	cp = strrchr (filePath,'\\');
	if (cp == NULL)
	{
		cp = strrchr (filePath,'/');
	}
	if (cp == NULL)
	{
		// We have no path information, just a file name.
		*cs_DirP = '\0';
		CS_stncp (FileFolder,cs_Dir,sizeof (FileFolder));
		cp = FileFolder + strlen (FileFolder) - 1;
		*cp = '\0';
		CS_stncp (cs_DirP,catalogPath,MAXPATH);
		CS_stncp (filePath,cs_Dir,sizeof (filePath));
	}
	else
	{
		*cp++ = '\0';
		CS_stncp (FileFolder,filePath,sizeof (FileFolder));
		CS_stncp (filePath,catalogPath,sizeof (filePath));
	}

	// Open up the actual file.
	gdcStrm = CS_fopen (filePath,_STRM_TXTRD);
	if (gdcStrm != NULL)
	{
		Ok = true;		
	}

	// Process each line in the file; extract a catalog entry for each.
	while (Ok)
	{
		if (CS_feof (gdcStrm))
		{
			break;
		}
		CS_fgets (lineBufr,sizeof (lineBufr),gdcStrm);
		if (CS_ferror (gdcStrm))
		{
			Ok = false;
			break;
		}

		// Make sure it's null terminated.
		lineBufr [sizeof (lineBufr) - 1] = '\0';

		// Trim white space on the front, and end.
		CS_trim (lineBufr);

		// Ignore empty lines.
		if (lineBufr [0] == '\0') continue;

		// Capture the first three complete line comments, ignore all others.
		if (lineBufr [0] == '#')
		{
			// If we have already captured three comments, we can stop screwing
			// around with them.
			if (!TrailingComment.empty ()) continue;

			// If its a whole line comment, tack a new line back on to the end.
			CS_stncat (lineBufr,"\n",sizeof (lineBufr));

			// Start/Continue the comment capture.
			comment += lineBufr;
			continue;
		}

		// Deal with any accumulated comment.
		if (!comment.empty ())
		{
			if      (InitialComment.empty ())  InitialComment  = comment;
			else if (MiddleComment.empty ())   MiddleComment   = comment;
			else if (TrailingComment.empty ()) TrailingComment = comment;
			comment.erase ();
		}

		// Trim, comments at the end of the line.
		quote = false;
		for (cp = lineBufr;*cp != '\0';cp += 1)
		{
			if (*cp == '"') quote = !quote;
			if (quote) continue;
			if (*cp == '#') break;
		}
		if (*cp == '#')
		{
			*cp = '\0';
			CS_trim (lineBufr);
		}

		// See if this is a fallback definition.  They can appear anywhere,
		// we are not case sensitive.  If there are more than one, we
		// honor only the last one that we see.
		if (CS_strnicmp (lineBufr,"fallback",8) == 0)
		{
			// This is a fallback definition.
			cp = lineBufr + 8;
			while (isspace (*cp)) cp += 1;
			if (*cp == '=' || *cp == ',')
			{
				cp += 1;
				while (isspace (*cp)) cp += 1;
			}
			CS_stncp (Fallback,cp,sizeof (Fallback));
			continue;
		}

		// Parse the file name and the buffer size from the file.
		cpt = cTemp;
		quote = false;
		for (cp = lineBufr;*cp != '\0';cp += 1)
		{
			if (*cp == '"')
			{
				if (quote)
				{
					if (*(cp + 1) == '"')
					{
						*cpt++ = '"';
						cp += 1;
						continue;
					}
					else
					{
						quote = false;
					}
				}
				else
				{
					quote = true;
				}
			}
			else if (!quote && *cp == ',')
			{
				break;
			}
			else
			{
				*cpt++ = *cp;
			}
		}
		*cpt = '\0';

		// Parse the buffer size, if its there.
		bufferSize = 0L;
		if (*cp != '\0')
		{
			cp += 1;
			bufferSize = strtol (cp,0,0);

			// Skip past the buffer size.
			while (*cp != '\0' && *cp != ',') cp += 1;
			if (*cp == ',') cp += 1;
		}

		// Parse the flags argument if there.  Note, this can be decimal or
		// hexadecimal.
		flags = 0UL;
		if (*cp != '\0')
		{
			flags = strtoul (cp,0,0);

			// Skip past the flags.
			while (*cp != '\0' && *cp != ',') cp += 1;
			if (*cp == ',') cp += 1;
		}

		// Parse the density argument if there.
		density = 0.0;
		if (*cp != '\0')
		{
			density = strtod (cp,0);
			if (density < 0.25 || density > 60.0)
			{
				Ok = false;
				break;
			}
			density /= 60.0;

			// Skip past the flags.
			while (*cp != '\0' && *cp != ',') cp += 1;
			if (*cp == ',') cp += 1;
		}

		// Standardize the directory separator character. */
		CSrplDirSep (cTemp);

		// Do the relative bit.
		if (!strncmp (cTemp,relString,strlen (relString)))
		{
			// It's a relative entry; relative to the directory of the
			// catalog file.  Convert to an absolute path.
			relative = 1;
			cp = CS_stncp (filePath,FileFolder,sizeof (filePath));
			size_t room = sizeof (filePath) - strlen (filePath) - 1;
			*cp++ = cs_DirsepC;
			CS_stncp (cp,&cTemp [strlen (relString)],static_cast<int>(room));
		}
		else if (!strncmp (cTemp,relStringPar,strlen(relStringPar)))
		{
			// It's a relative entry; relative to the parent of the directory
			// in which the catalog file resides.  Convert to absolute by
			// leaving the .. reference in the middle.  This is much easier
			// and less error prone than trying to back out a directory from
			// the catalog path.
			relative = 2;
			cp = CS_stncp (filePath,FileFolder,sizeof(filePath));
			size_t room = sizeof (filePath) - strlen (filePath) - 1;
			*cp++ = cs_DirsepC;
			CS_stncp (cp,cTemp,static_cast<int>(room));
		}
		else
		{
			// Its a full path name.
			relative = 0;
			CS_stncp (filePath,cTemp,sizeof (filePath));
		}

		// Add a catalog entry to the catalog list.
		TcsGdcEntry *entryPtr = new TcsGdcEntry (filePath,relative,bufferSize,flags,density);
		Entries.push_back (*entryPtr);
		delete entryPtr;
	}
	if (gdcStrm != NULL)
	{
		CS_fclose (gdcStrm);
		gdcStrm = NULL;
	}

	// Handle any comment tacked on the end.
	if (!comment.empty ())
	{
		if      (InitialComment.empty ())  InitialComment  = comment;
		else if (MiddleComment.empty ())   MiddleComment   = comment;
		else if (TrailingComment.empty ()) TrailingComment = comment;
	}
}
TcsGdcFile::~TcsGdcFile (void)
{
	// Nothing to do, yet!!!
}
const TcsGdcEntry* TcsGdcFile::GetEntryPtr (size_t index) const
{
	const TcsGdcEntry* rtnValue = 0;

	if (index < Entries.size ())
	{
		rtnValue = &Entries [index]; 
	}
	return rtnValue;
}
bool TcsGdcFile::ConvertToRelative (char* entryPath) const
{
	bool converted (false);
	
	size_t mtchLen = strlen (FileFolder);
	if (mtchLen > 0)
	{
		if (!CS_strnicmp (FileFolder,entryPath,mtchLen))
		{
			*entryPath = '.';
			CS_stcpy ((entryPath + 1),(entryPath + mtchLen));
			converted = true;
		}
	}
	return converted;
}
//newPage//
TcsKeyName::TcsKeyName (EcsKeyNmType type) : Type     (type),
											 Quality  (0U),
											 EpsgCode (0UL)
{
	memset (KeyName,'\0',sizeof (KeyName));
}
TcsKeyName::TcsKeyName (EcsKeyNmType type,const char* keyName,unsigned long epsgCode)
																:
															  Type     (type),
															  Quality  (0U),
															  EpsgCode (epsgCode)
{
	memset (KeyName,'\0',sizeof (KeyName));
	CS_stncp (KeyName,keyName,sizeof (KeyName));
}
TcsKeyName::TcsKeyName (EcsKeyNmType type,const wchar_t* keyName,unsigned long epsgCode)
																	:
																 Type     (type),
																 Quality  (0U),
																 EpsgCode (epsgCode)
{
	memset (KeyName,'\0',sizeof (KeyName));
	wcstombs (KeyName,keyName,sizeof (KeyName));
}
TcsKeyName::TcsKeyName (const TcsKeyName& source) : Type     (source.Type),
													Quality  (source.Quality),
													EpsgCode (source.EpsgCode)
{
	CS_stncp (KeyName,source.KeyName,sizeof (KeyName));
}
TcsKeyName::~TcsKeyName (void)
{
}
TcsKeyName& TcsKeyName::operator= (const TcsKeyName& rhs)
{
	if (&rhs != this)
	{
		Type     = rhs.Type;
		Quality  = rhs.Quality;
		EpsgCode = rhs.EpsgCode;
		CS_stncp (KeyName,rhs.KeyName,sizeof (KeyName));
	}
	return *this;
}
bool TcsKeyName::operator< (const TcsKeyName& rhs) const
{
	int result = CS_stricmp (KeyName,rhs.KeyName);
	bool isLessThan = (result < 0);
	return isLessThan;
}
bool TcsKeyName::operator== (const TcsKeyName& rhs)
{
	bool isEqual;
	isEqual = !CS_stricmp (KeyName,rhs.KeyName);
	return isEqual;
}
TcsKeyName::operator const char* ()
{
	return KeyName;
}
TcsKeyName::operator const char* () const
{
	return KeyName;
}
bool TcsKeyName::QualitySort (const TcsKeyName& lhs,const TcsKeyName& rhs)
{
	bool lhsComesFirst;

	// This will look strange, but we what the object with the
	// highest quality value first.
	lhsComesFirst = (lhs.Quality > rhs.Quality);
	return lhsComesFirst;
}
EcsKeyNmType TcsKeyName::GetKeyNameType ()
{
	return Type;
}
const char* TcsKeyName::GetKeyName ()
{
	return KeyName;
}
bool TcsKeyName::SetQuality (const cs_Csdef_ *csDefPtr)
{
	bool ok (false);

	Quality = 0U;

	if (Type == csKyNmTypeCRS && (csDefPtr != 0))
	{
		ok = true;
		Quality = 2;
		if (csDefPtr->dat_knm [0] != '\0')
		{
			Quality += 1;
			const char* csDotPtr = strchr (csDefPtr->key_nm,'.');
			if (csDotPtr != NULL)
			{
				Quality += 1;
				const char* csKeyNm = csDefPtr->key_nm;
				const char* dtKeyNm = csDefPtr->dat_knm;
				while (csKeyNm < csDotPtr)
				{
					if (*csKeyNm == *dtKeyNm)
					{
						Quality += 1;
						csKeyNm++;
						dtKeyNm++;
					}
					else
					{
						break;
					}
				}
			}
		}
	}
	return ok;
}
std::wostream& operator<< (std::wostream& outStrm,const TcsKeyName& keyName)
{
	char ccTemp1 [64];
	char ccTemp2 [64];
	wchar_t wcTemp [64];

	sprintf (ccTemp1,"\"%s\",",static_cast<const char*>(keyName));
	sprintf (ccTemp2,"%-28s",ccTemp1);
	mbstowcs (wcTemp,ccTemp2,64);
	outStrm << wcTemp;
	return outStrm;
}
TcsKeyNameList::TcsKeyNameList (void) : KeyNameList ()
{
}
TcsKeyNameList::TcsKeyNameList (const TcsKeyName& primary) : KeyNameList ()
{
	KeyNameList.push_front (primary);
}
TcsKeyNameList::TcsKeyNameList (const TcsKeyName& primary,const TcsKeyName& secondary)
{
	KeyNameList.push_front (primary);
	KeyNameList.push_back (secondary);
}
TcsKeyNameList::TcsKeyNameList (const TcsKeyNameList& source)
									:
								KeyNameList (source.KeyNameList)
{
}
TcsKeyNameList::~TcsKeyNameList (void)
{
}
TcsKeyNameList& TcsKeyNameList::operator= (const TcsKeyNameList& rhs)
{
	if (&rhs != this)
	{
		KeyNameList = rhs.KeyNameList;
	}
	return *this;
}
TcsKeyNameList& TcsKeyNameList::operator+= (const TcsKeyName& rhs)
{
	KeyNameList.push_back (rhs);
	return *this;
}
bool TcsKeyNameList::operator< (const TcsKeyNameList& rhs)
{
	const char* leftSide = KeyNameList.front ();
	const char* rightSide = rhs.KeyNameList.front ();
	int result = CS_stricmp (leftSide,rightSide);
	return (result < 0);
}
bool TcsKeyNameList::operator== (const TcsKeyNameList& rhs)
{
	const char* leftSide = KeyNameList.front ();
	const char* rightSide = rhs.KeyNameList.front ();
	int result = CS_stricmp (leftSide,rightSide);
	return (result == 0);
}
TcsKeyName* TcsKeyNameList::GetFirst (void)
{
	TcsKeyName* rtnValue (0);
	
	Rewind ();
	if (currentPos != KeyNameList.end ())
	{
		rtnValue = &(*currentPos);
		currentPos++;
	}
	return rtnValue;
}
TcsKeyName* TcsKeyNameList::GetNext (void)
{
	TcsKeyName* rtnValue (0);
	
	if (currentPos != KeyNameList.end ())
	{
		rtnValue = &(*currentPos);
		currentPos++;
	}
	return rtnValue;
}
void TcsKeyNameList::Rewind (void)
{
	currentPos = KeyNameList.begin ();
}
void TcsKeyNameList::Arrange ()
{
	std::list<TcsKeyName>::iterator listItr;
	
	std::stable_sort (KeyNameList.begin (),KeyNameList.end (),TcsKeyName::QualitySort);
}
void TcsKeyNameList::WriteToStream (std::wostream& listStream)
{
	std::list<TcsKeyName>::iterator listItr1;
	std::list<TcsKeyName>::iterator listItr2;
	std::list<TcsKeyName>::iterator listItrn;

	listItr1 = KeyNameList.begin ();
	listItr2 = listItr1;
	listItr2++;
	listItrn = listItr2;
	listItrn++;

	listStream << L"{  "
			   << *listItr1
			   << *listItr2;
	for (;listItrn != KeyNameList.end ();listItrn++)
	{
		listStream << std::endl;
		listStream << L"                               "
				   << *listItrn;
	}
	listStream << L"    }," << std::endl;
}
//newPage//
//=========================================================================
// TcsNameMapperSource -- An object which contains the NameMapper .csv
// source file as a plain old .csv file.  Useful when doing maintenance.
// Doing maintenance on the NameMapper when implemented in it natural
// form, i.e. a std::set<TcsNameMap>, is quite restrictive as you can't
// change data in the individual TcsNameMap elements which are a part of
// the key for std::set<TcsNameMap>.  SInce most all elements are part
// of the key, this limitation can be quite daunting.
//
// A the NameMapper carries nmemonics for the flavor, rather than the
// numeric value actually used internally, this object necessarily
// maintains a table of the nmemonic names assigned to the various flavors.
// Otherwise, it's pretty much a simple shell in top the the
// TcsCsvFileBase object which contains the actual NameMapper data.
//=========================================================================
// Construction, Destruction, and Assignment
TcsNameMapperSource::TcsNameMapperSource (void) : TcsCsvFileBase (true,9,11),
												  Ok (true)
{
	unsigned index;

	for (index = 0;index < 32;index += 1)
	{
		FlavorNames [index][0] = '\0';
		FlavorIdValues [index] = 0UL;
	}
	memset (NameBuffer,'\0',sizeof (NameBuffer));
}
TcsNameMapperSource::TcsNameMapperSource (const TcsNameMapperSource& source)
											:
										  TcsCsvFileBase (source),
										  Ok             (source.Ok)
{
	for (int idx = 0;idx < FlavorCount;idx += 1)
	{
		wcsncpy (FlavorNames [idx],source.FlavorNames [idx],FlavorSize);
		FlavorNames [idx][FlavorSize - 1] = L'\0';
		FlavorIdValues [idx] = source.FlavorIdValues [idx];
	}
	wcsncpy (NameBuffer,source.NameBuffer,wcCount (NameBuffer));
	NameBuffer [wcCount (NameBuffer) - 1] = L'\0';
}
TcsNameMapperSource::~TcsNameMapperSource (void)
{
	// Nothing to do, yet!
}
TcsNameMapperSource& TcsNameMapperSource::operator= (const TcsNameMapperSource& rhs)
{
	if (&rhs != this)
	{
		TcsCsvFileBase::operator= (rhs);
		Ok = rhs.Ok;
		for (int idx = 0;idx < FlavorCount;idx += 1)
		{
			wcsncpy (FlavorNames [idx],rhs.FlavorNames [idx],FlavorSize);
			FlavorNames [idx] [FlavorSize - 1] = L'\0';
			FlavorIdValues [idx] = rhs.FlavorIdValues [idx];
		}
		wcsncpy (NameBuffer,rhs.NameBuffer,wcCount (NameBuffer));
		NameBuffer [wcCount (NameBuffer) - 1] = L'\0';
	}
	return *this;
}
//=========================================================================
// Operator Overrides
//=========================================================================
// Public Named Member Functions
bool TcsNameMapperSource::ReadFromFile (const char* csvSourceFile)
{
	bool ok;

	std::wifstream wiStrm;

	TcsCsvStatus csvStatus;

	wiStrm.open (csvSourceFile,std::ios_base::in);
	ok = wiStrm.is_open ();
	if (ok)
	{
		ok = ReadFromStream (wiStrm,true,csvStatus);
		if (ok)
		{
			ok = InitializeFlavors ();
		}
		wiStrm.close ();
	}
	return ok;
}
unsigned long TcsNameMapperSource::GetIdent (unsigned recNbr) const
{
	return GetFieldAsUlong (recNbr,IdentFld);
}
unsigned long TcsNameMapperSource::GetType (unsigned recNbr) const
{
	return GetFieldAsUlong (recNbr,IdentFld);
}
EcsNameFlavor TcsNameMapperSource::GetFlavor (unsigned recNbr) const
{
	bool ok;
	
	unsigned long ulValue;
	EcsNameFlavor result (csMapFlvrUnknown);
	std::wstring fieldData;
	TcsCsvStatus status;

	ok = (recNbr < RecordCount ());
	if (ok)
	{
		ok = GetField (fieldData,recNbr,FlvrFld,status);
		if (ok)
		{
			wchar_t firstChar = *fieldData.c_str();
			if (iswdigit (firstChar))
			{
				// The flavor field is numeric, this is easy.
				ulValue = wcstoul (fieldData.c_str(),0,10);
				result = static_cast<EcsNameFlavor>(ulValue);
			}
			else
			{
				// The flavor field is a nmemonic.  Need to work through
				// the nmomonc table to get a numeric value.
				for (int idx = 0;idx < FlavorCount;idx += 1)
				{
					if (!wcsicmp (FlavorNames [idx],fieldData.c_str ()))
					{
						result = static_cast<EcsNameFlavor>(FlavorIdValues [idx]);
						break;
					}
				}
			}
		}
	}
	return result;
}
unsigned long TcsNameMapperSource::GetFlavorIdent (unsigned recNbr) const
{
	return GetFieldAsUlong (recNbr,IdentFld);
}
const wchar_t* TcsNameMapperSource::GetName (unsigned recNbr) const
{
	return GetFieldAsString (recNbr,NameFld);
}
unsigned long TcsNameMapperSource::GetDupSort (unsigned recNbr) const
{
	return GetFieldAsUlong (recNbr,IdentFld);
}
unsigned long TcsNameMapperSource::GetAliasFlag (unsigned recNbr) const
{
	return GetFieldAsUlong (recNbr,IdentFld);
}
unsigned long TcsNameMapperSource::GetFlags (unsigned recNbr) const
{
	return GetFieldAsUlong (recNbr,IdentFld);
}
unsigned long TcsNameMapperSource::GetDeprecation (unsigned recNbr) const
{
	return GetFieldAsUlong (recNbr,IdentFld);
}
const wchar_t* TcsNameMapperSource::GetRemarks (unsigned recNbr) const
{
	return GetFieldAsString (recNbr,RemrksFld);
}
const wchar_t* TcsNameMapperSource::GetComment (unsigned recNbr) const
{
	return GetFieldAsString (recNbr,CommntFld);
}
bool TcsNameMapperSource::SetIdent (unsigned recNbr,unsigned long newIdent)
{
	return SetUlongField (recNbr,IdentFld,newIdent);
}
bool TcsNameMapperSource::SetType (unsigned recNbr,unsigned long newType)
{
	return SetUlongField (recNbr,IdentFld,newType);
}
bool TcsNameMapperSource::SetFlavor (unsigned recNbr,EcsNameFlavor newFlavor)
{
	bool ok (false);
	int idx;

	// If the record number is greater than Flavor count, we write
	// the nmemonic value.  Otherwise we wrtie as an integer.
	if (recNbr < FlavorCount)
	{
		ok = SetUlongField (recNbr,FlvrFld,newFlavor);
	}
	else
	{
		for (idx = 0;idx < FlavorCount;idx += 1)
		{
			if (FlavorIdValues [idx] == static_cast<unsigned long>(newFlavor))
			{
				ok = SetStringField (recNbr,FlvrFld,FlavorNames [idx]);
				break;
			}
		}
		ok = (idx < FlavorCount);	
	}
	return ok;
}
bool TcsNameMapperSource::SetFlavorIdent (unsigned recNbr,unsigned long newFlavorIdent)
{
	return SetUlongField (recNbr,FlvrIdFld,newFlavorIdent);
}
bool TcsNameMapperSource::SetName (unsigned recNbr,const wchar_t* newName)
{
	return SetStringField (recNbr,NameFld,newName);
}
bool TcsNameMapperSource::SetDupSort (unsigned recNbr,unsigned long newDupSort)
{
	return SetUlongField (recNbr,DupSrtFld,newDupSort);
}
bool TcsNameMapperSource::SetAliasFlag (unsigned recNbr,unsigned long newAliasFlag)
{
	return SetUlongField (recNbr,AliasFld,newAliasFlag);
}
bool TcsNameMapperSource::SetFlags (unsigned recNbr,unsigned long newFlags)
{
	return SetUlongField (recNbr,FlagsFld,newFlags);
}
bool TcsNameMapperSource::SetDeprecation (unsigned recNbr,unsigned long newDeprecation)
{
	return SetUlongField (recNbr,DeprctFld,newDeprecation);
}
bool TcsNameMapperSource::SetRemarks (unsigned recNbr,const wchar_t* newRemarks)
{
	return SetStringField (recNbr,RemrksFld,newRemarks);
}
bool TcsNameMapperSource::SetComment (unsigned recNbr,const wchar_t* newComment)
{
	return SetStringField (recNbr,CommntFld,newComment);
}
bool TcsNameMapperSource::Locate (unsigned& result,unsigned long type,EcsNameFlavor flavor,const wchar_t* name) const
{
	bool ok (false);
	unsigned recordCount;
	unsigned recNbr (UlErrorValue);

	recordCount = RecordCount ();
	for (recNbr = 0;recNbr < recordCount;recNbr += 1)
	{
		if ((GetType (recNbr) != type) || (GetFlavor (recNbr) != flavor))
		{
			continue;
		}
		if (wcsicmp (GetName (recNbr),name))
		{
			continue;
		}
		ok = true;
		break;
	}
	result = ok ? recNbr : UlErrorValue;
	return ok;
}
bool TcsNameMapperSource::Locate (unsigned& result,unsigned long type,EcsNameFlavor flavor,unsigned long flvrId)
{
	bool ok (false);
	unsigned recordCount;
	unsigned recNbr;

	recordCount = RecordCount ();
	for (recNbr = 0;recNbr < recordCount;recNbr += 1)
	{
		if ((GetType (recNbr) != type) || (GetFlavor (recNbr) != flavor))
		{
			continue;
		}
		if (GetFlavorIdent (recNbr) != flvrId)
		{
			continue;
		}
		ok = true;
		break;
	}
	result = ok ? recNbr : UlErrorValue;
	return ok;
}
bool TcsNameMapperSource::Locate (unsigned& result,unsigned long type,EcsNameFlavor flavor,const char* name)
{
	bool ok;
	
	wchar_t wcName [256];
	
	mbstowcs (wcName,name,wcCount (wcName));
	wcName [255] = L'\0';
	ok = TcsNameMapperSource::Locate (result,type,flavor,wcName);
	return ok;
}
bool TcsNameMapperSource::LocateNameMap (unsigned& result,TcsNameMap& locator,bool byId)
{
	bool ok (false);
	unsigned recNbr;
	unsigned long type;
	EcsNameFlavor flavor;
	unsigned long flvrId;
	const wchar_t* namePtr;

	type = locator.GetMapClass ();
	flavor = locator.GetFlavor ();

	if (byId)
	{
		flvrId = locator.GetNumericId ();
		ok = Locate (recNbr,type,flavor,flvrId);
	}
	else
	{
		namePtr = locator.GetNamePtr ();
		ok = Locate (recNbr,type,flavor,namePtr);
	}
	result = ok ? recNbr : UlErrorValue;
	return ok;
}
bool TcsNameMapperSource::ReplaceNameMap (unsigned recNbr,TcsNameMap& updatedNameMap)
{
	bool ok;
	
	        ok = SetIdent (recNbr,updatedNameMap.GetGenericId ());
	if (ok) ok = SetType (recNbr,updatedNameMap.GetMapClass ());
	if (ok) ok = SetType (recNbr,updatedNameMap.GetMapClass ());
	if (ok) ok = SetFlavor (recNbr,updatedNameMap.GetFlavor ());
	if (ok) ok = SetFlavorIdent (recNbr,updatedNameMap.GetNumericId ());
	if (ok) ok = SetName (recNbr,updatedNameMap.GetNamePtr ());
	if (ok) ok = SetDupSort(recNbr,updatedNameMap.GetDupSort ());
	if (ok) ok = SetAliasFlag (recNbr,updatedNameMap.GetAliasFlag ());
	if (ok) ok = SetFlags (recNbr,(unsigned long)updatedNameMap.GetUserValue ());
	if (ok) ok = SetDeprecation (recNbr,updatedNameMap.DeprecatedBy ());
	if (ok) ok = SetRemarks (recNbr,updatedNameMap.GetRemarks ());
	if (ok) ok = SetComment (recNbr,updatedNameMap.GetComments ());
	return ok;
}
bool TcsNameMapperSource::AddNameMap (TcsNameMap& newNameMap)
{
	return false;
}
bool TcsNameMapperSource::DeleteNameMap (unsigned recNbr)
{
	return false;
}
bool TcsNameMapperSource::RenameObject (EcsMapObjType type,EcsNameFlavor flavor,
														   const char* currentName,
														   const char* newName)
{
	bool ok;
	unsigned recNbr;
	wchar_t wcNewName [256];

	ok = Locate (recNbr,type,flavor,currentName);
	if (ok)
	{
		mbstowcs (wcNewName,newName,wcCount (wcNewName));
		wcNewName [255] = L'\0';
		ok = SetName (recNbr,wcNewName);
	}
	return ok;
}
bool TcsNameMapperSource::WriteToFile (const char* csvSourceFile,bool overwrite)
{
	bool ok;

	std::wofstream woStrm;

	TcsCsvStatus csvStatus;

	if (overwrite)
	{
		woStrm.open (csvSourceFile,std::ios_base::out | std::ios_base::trunc);
	}
	else
	{
		woStrm.open (csvSourceFile,std::ios_base::out | std::ios_base::app);
	}
	ok = woStrm.is_open ();
	if (ok)
	{
		ok = WriteToStream (woStrm,true,csvStatus);
		woStrm.close ();
		woStrm.flush ();
	}
	return ok;
}
//=========================================================================
// Protected Named Member Functions
bool TcsNameMapperSource::InitializeFlavors ()
{
	bool ok;

	unsigned index;
	unsigned recordCount;

	unsigned long idntValue;
	unsigned long typeValue;
	unsigned long flvrIdx;
	
	std::wstring fieldData;
	TcsCsvStatus status;

	for (index = 0;index < 32;index += 1)
	{
		FlavorNames [index][0] = '\0';
		FlavorIdValues [index] = 0UL;
	}

	ok = true;
	recordCount = RecordCount ();
	for (index = 0;ok && (index < recordCount);index += 1)
	{
		typeValue = GetFieldAsUlong (index,TypeFld);
		if (typeValue == 1UL)
		{
			// Its a flavor name definition record.  Extract the nmemonic and
			// save it in the proper place.
			ok = GetField (fieldData,index,NameFld,status);
			if (ok)
			{
				flvrIdx = GetFieldAsUlong (index,FlvrFld);
				wcsncpy (FlavorNames [flvrIdx],fieldData.c_str (),FlavorSize);
				FlavorNames [flvrIdx][FlavorSize - 1] = L'\0';
			}
		}
		else if (typeValue > 1)
		{
			// It is not a flavor nmemonic definition.  What we need to do
			// here is accumulate the highest currently used Generic ID for
			// each flavor.
			ok = GetField (fieldData,index,FlvrFld,status);
			if (ok)
			{
				idntValue = GetFieldAsUlong (index,IdentFld);
				for (flvrIdx = 0;flvrIdx < FlavorCount;flvrIdx += 1)
				{
					if (!wcsicmp (FlavorNames [flvrIdx],fieldData.c_str ()))
					{
						if (idntValue > FlavorIdValues [flvrIdx])
						{
							FlavorIdValues [flvrIdx] = idntValue;
							break; 
						}
					}
				}
			}
		}
	}
	return ok;
}
unsigned long TcsNameMapperSource::GetFieldAsUlong (unsigned recNbr,short fldNbr) const
{
	bool ok;
	unsigned long result (UlErrorValue);
	std::wstring fieldData;
	TcsCsvStatus status;

	ok = (recNbr < RecordCount ());
	if (ok)
	{
		ok = GetField (fieldData,recNbr,fldNbr,status);
		if (ok)
		{
			wchar_t firstChar = *fieldData.c_str();
			ok = iswdigit (firstChar);
			if (ok)
			{
				result = wcstoul (fieldData.c_str(),0,10);
			}
		}
	}
	return result;
}
// This function should return a pointer to the actual string in the .CSV file
// object.  That is the indent.  We need high performance to the name for
// simple comparison purposes, so we deisre very much to avoid copying the
// string.
const wchar_t* TcsNameMapperSource::GetFieldAsString (unsigned recNbr,short fldNbr) const
{
	bool ok;
	const wchar_t* result (0);
	std::wstring fieldData;
	TcsCsvStatus status;

	ok = (recNbr < RecordCount ());
	if (ok)
	{
		ok = GetField (fieldData,recNbr,fldNbr,status);
		if (ok)
		{
			result = fieldData.c_str();
		}
	}
	return result;
}
bool TcsNameMapperSource::SetUlongField (unsigned recNbr,short fldNbr,unsigned long newValue)
{
	bool ok;

	wchar_t buffer [256]; 	
	std::wstring fieldData;
	TcsCsvStatus status;
	
	ok = (recNbr < RecordCount ());
	if (ok)
	{
		ok = (fldNbr < FieldCount (recNbr));
		if (ok)
		{
			swprintf (buffer,wcCount (buffer),L"%lu",newValue);
			fieldData = buffer;
			ok = ReplaceField (fieldData,recNbr,fldNbr,status);
		}
	}
	return ok;
}
bool TcsNameMapperSource::SetStringField (unsigned recNbr,short fldNbr,const wchar_t* newString)
{
	bool ok;

	std::wstring fieldData;
	TcsCsvStatus status;
	
	ok = (recNbr < RecordCount ());
	if (ok)
	{
		ok = (fldNbr < FieldCount (recNbr));
		if (ok)
		{
			fieldData = newString;
			ok = ReplaceField (fieldData,recNbr,fldNbr,status);
		}
	}
	return ok;
}
//newPage//
//=============================================================================
// TcsLasLosFile -- An objet specifically designed to construct and
// subsequently write customized .l?s file sets as used by the USGS program
// known as NADCON.
//
// Note that the member variable DataImage is assumed to be in one of two
// states.
//	1> Null value, clearly meaning no memory has been allocated as yet and
//	   that there is obviously no data of value in the "DataImage".
//	2> Memory has been allocated and the size of the memory block is specified
//	   by the RecCount and EleCount member variables. Upon allocation, the
//	   "DataImage will be initialized to a special value
//	   (TcsLasLosFile::NoValue) and other elements may have valuable data
//	   in them.
//
// Also note that the SetGridValue and GetGridValue member functions which
// take longitude and latitude as doubles for arguments expect the longitude
// and latitude arguemnts to be in degrees modulo the appropriate cell size.
// To enforce/accomplish this, the provided arguments are converted to an
// integer number of seconds before being used to calculate an index value
// into the DataImage array.  This assumes that the cell size is always an
// integer number of seconds, which has always been the case.
//
// NoValue < NoValueTest by definition
//		[for example: bool noValue = (gridValue < NoValueTest);
const float TcsLasLosFile::NoValue       = -1.0000E+24F;
const float TcsLasLosFile::NoValueTest   = -1.5000E+23F;
// EdgeValue < EdgeValueTest by definition
//		[for example: bool edgeValue = (gridValue < NoValueTest);
// Yes, that means applying the edge value test to a "NoValue" produces true.
const float TcsLasLosFile::EdgeValue     = -1.0000E+23F;
const float TcsLasLosFile::EdgeValueTest = -1.5000E+22F;
//=============================================================================
// Construction  /  Destruction  /  Assignment
TcsLasLosFile::TcsLasLosFile (void) : EleCount       (0),
									  RecCount       (0),
									  SwLongitude    (0.0),
									  SwLatitude     (0.0),
									  DeltaLongitude (0.0),
									  DeltaLatitude  (0.0),
									  DataImage      (0),
									  ZeeCount       (0L),
									  Angle          (0.0)
{
	memset (FileIdent,'\0',sizeof (FileIdent));
	memset (Program,'\0',sizeof (Program));
	memset (FilePath,'\0',sizeof (FilePath));
}
TcsLasLosFile::TcsLasLosFile (double swLng,double swLat,double cellSize,long32_t recCount,
																		long32_t eleCount)
																			:
																		 EleCount       (eleCount),
																		 RecCount       (recCount),
																		 SwLongitude    (swLng),
																		 SwLatitude     (swLat),
																		 DeltaLongitude (cellSize),
																		 DeltaLatitude  (cellSize),
																		 DataImage      (0),
																		 ZeeCount       (0L),
																		 Angle          (0.0)
{
	memset (FileIdent,'\0',sizeof (FileIdent));
	memset (Program,'\0',sizeof (Program));
	memset (FilePath,'\0',sizeof (FilePath));

	size_t arraySize = EleCount * RecCount;
	DataImage = new float [arraySize];
	for (size_t idx = 0;idx < arraySize;idx += 1)
	{
		DataImage [idx] = NoValue;
	}
	return;
}
TcsLasLosFile::TcsLasLosFile (const TcsLasLosFile& source) : EleCount       (source.EleCount),
															 RecCount       (source.RecCount),
															 SwLongitude    (source.SwLongitude),
															 SwLatitude     (source.SwLatitude),
															 DeltaLongitude (source.DeltaLongitude),
															 DeltaLatitude  (source.DeltaLatitude),
															 DataImage      (0),
															 ZeeCount       (source.ZeeCount),
															 Angle          (source.Angle)
{
	if (source.DataImage != 0)
	{
		size_t arraySize = EleCount * RecCount;
		DataImage = new float [arraySize];
		for (size_t idx = 0;idx < arraySize;idx += 1)
		{
			DataImage [idx] = source.DataImage [idx];
		}
	}

	CS_stncp (FileIdent,source.FileIdent,sizeof (FileIdent));
	CS_stncp (Program,source.Program,sizeof (Program));
	CS_stncp (FilePath,source.FilePath,sizeof (FilePath));
	return;
}
TcsLasLosFile& TcsLasLosFile::operator= (const TcsLasLosFile& rhs)
{
	if (this != &rhs)
	{
		if (DataImage != 0)
		{
			delete [] DataImage;
			DataImage = 0;					// defensive :>)
		}
		EleCount       = rhs.EleCount;
		RecCount       = rhs.RecCount;
		SwLongitude    = rhs.SwLongitude;
		SwLatitude     = rhs.SwLatitude;
		DeltaLongitude = rhs.DeltaLongitude;
		DeltaLatitude  = rhs.DeltaLatitude;

		if (rhs.DataImage != 0)
		{
			size_t arraySize = EleCount * RecCount;
			DataImage = new float [arraySize];
			for (size_t idx = 0;idx < arraySize;idx += 1)
			{
				DataImage [idx] = rhs.DataImage [idx];
			}
		}

		CS_stncp (FileIdent,rhs.FileIdent,sizeof (FileIdent));
		CS_stncp (Program,rhs.Program,sizeof (Program));
		CS_stncp (FilePath,rhs.FilePath,sizeof (FilePath));
		ZeeCount = rhs.ZeeCount;
		Angle    = rhs.Angle;
	}
	return *this;
}
TcsLasLosFile::~TcsLasLosFile (void)
{
	delete [] DataImage;		// delete [] 0 is a legal no op per Strooustrup
}
//=============================================================================
// Named Member Functions (Methods)
bool TcsLasLosFile::ReadFromFile (const char* filePath)
{
	bool ok (false);
	
	size_t rdCnt;
	float ioFloat;
	long32_t ioLong;
	long strmPos;

	// If we have some existing data, we need to get rid of it.  This function
	// will replace everything.
	delete [] DataImage;
	DataImage = 0;

	// Open the file.
	csFILE* lasStrm = CS_fopen (filePath,_STRM_BINRD);
	ok = (lasStrm != NULL);
	if (ok)
	{
		CS_stncp (FilePath,filePath,sizeof (FilePath));
		// To avoid compiler dependent staructure packing problems, we read each
		// element of a NADCON filer header individually.

		// Should we do something about by swapping  here?  Currently,
		// we intend to use this function once, in the Windows environment,
		// so we'll simply assume a little endian environment all around.
		
		// Read in 56 bytes of FileIdent.
		rdCnt  = CS_fread (FileIdent,1,sizeof (FileIdent),lasStrm);
		ok = (rdCnt == sizeof (FileIdent));

		// Read in 8 bytes of Program.
		rdCnt  = CS_fread (Program,1,sizeof (Program),lasStrm);
		ok &= (rdCnt == sizeof (Program));

		// Element Count, and so forth
		rdCnt  = CS_fread (&ioLong,1,sizeof (ioLong),lasStrm);
		ok &= (rdCnt == sizeof (ioLong));
		EleCount = ioLong;

		rdCnt  = CS_fread (&ioLong,1,sizeof (ioLong),lasStrm);
		ok &= (rdCnt == sizeof (ioLong));
		RecCount = ioLong;

		rdCnt  = CS_fread (&ioLong,1,sizeof (ioLong),lasStrm);
		ok &= (rdCnt == sizeof (ioLong));
		ZeeCount = ioLong;

		rdCnt  = CS_fread (&ioFloat,1,sizeof (ioFloat),lasStrm);
		ok &= (rdCnt == sizeof (ioFloat));
		SwLongitude = static_cast<double>(ioFloat);

		rdCnt  = CS_fread (&ioFloat,1,sizeof (ioFloat),lasStrm);
		ok &= (rdCnt == sizeof (ioFloat));
		DeltaLongitude = static_cast<double>(ioFloat);

		rdCnt  = CS_fread (&ioFloat,1,sizeof (ioFloat),lasStrm);
		ok &= (rdCnt == sizeof (ioFloat));
		SwLatitude = static_cast<double>(ioFloat);

		rdCnt  = CS_fread (&ioFloat,1,sizeof (ioFloat),lasStrm);
		ok &= (rdCnt == sizeof (ioFloat));
		DeltaLatitude = static_cast<double>(ioFloat);

		rdCnt  = CS_fread (&ioFloat,1,sizeof (ioFloat),lasStrm);
		ok &= (rdCnt == sizeof (ioFloat));
		Angle = static_cast<double>(ioFloat);

		if (ok)
		{
			// We have read everything we need. Note that due to the FORTRAN
			// heritage of the NADCON program and thus supporting data files,
			// the Heaser record is the same size as any ther record.  Thus,
			// the size of the Header record is:
			//		EleCount * sizeof (float) + sizeof (long32_t);
			//
			// Above, we have read 96 bytes. Thus, it is quite likely that the
			// input stream is not proper;y positioned to read in data values,
			// YET.
			// (As a side note, that means the minimum EleCount value which
			// works for all this is 23!!!)
			strmPos = EleCount * sizeof (float) + sizeof (long32_t);
			int seekStat = CS_fseek (lasStrm,strmPos,SEEK_SET);
			ok = (seekStat == 0);
		}

		// Read in the grid data.  Note, each record has a long32_t record
		// number element on the front.  I have chosen to throw these away for
		// this object.
		if (ok)
		{
			size_t arraySize = EleCount * RecCount;
			DataImage = new float [arraySize];
			for (size_t idx = 0;idx < arraySize;idx += 1)
			{
				DataImage [idx] = NoValue;
			}
		}
		for (long32_t recIdx = 0;ok && recIdx < RecCount;recIdx += 1)
		{
			rdCnt  = CS_fread (&ioLong,1,sizeof (ioLong),lasStrm);
			ok &= (rdCnt == sizeof (ioLong));
			// One would think that this long value would contain the record
			// number, but that does not appear to be the case.  Testing
			// suggests this value is alsways zero.
			// ok = (ioLong == recIdx);		// Defensive

			for (long32_t eleIdx = 0;ok && eleIdx < EleCount;eleIdx += 1)
			{
				rdCnt  = CS_fread (&ioFloat,1,sizeof (ioFloat),lasStrm);
				ok &= (rdCnt == sizeof (ioFloat));
				size_t index = ((recIdx * EleCount) + eleIdx);
				DataImage [index] = ioFloat;
			}
		}
		CS_fclose (lasStrm);
	}
	return ok;
}
bool TcsLasLosFile::ReadFromFile (const wchar_t* filePath)
{
	char ccFilePath [MAXPATH];
	
	wcstombs (ccFilePath,filePath,sizeof (ccFilePath));
	bool ok = ReadFromFile (ccFilePath);
	return ok;
}
bool TcsLasLosFile::GetGridValue (float& result,long32_t recIdx,long32_t eleIdx) const
{
	bool ok (false);

	if (RecCount > 0L && recIdx >= 0L && recIdx < RecCount &&
		EleCount > 0L && eleIdx >= 0L && eleIdx < EleCount &&
		DataImage != 0)
	{
		size_t index = (recIdx * EleCount) + eleIdx;
		result = DataImage [index];
		ok = true;
	}
	return ok;
}
bool TcsLasLosFile::GetGridValue (double& result,long32_t recNbr,long32_t eleNbr) const
{
	bool ok;
	float floatResult;
	
	ok = GetGridValue (floatResult,recNbr,eleNbr);
	if (ok)
	{
		result = static_cast<double>(floatResult);
	}
	return ok;
}
bool TcsLasLosFile::GetGridValue (double& result,double longitude,double latitude) const
{
	// This round value may be a bit too small.  We could easily go to one half
	// a minute as far as any las/los file we know of.  Be careful!  The las/los
	// files for Hawaii and PRVI have ugly numbers for the grid cell size which
	// may cause problems here.
	static const double rndValue = (1.0 / 3600) * 0.5;	// one half second in degrees

	bool ok (false);
	long32_t recIdx;
	long32_t eleIdx;

	eleIdx = (long32_t)((longitude - SwLongitude + rndValue) / DeltaLongitude);
	recIdx = (long32_t)((latitude  - SwLatitude  + rndValue) / DeltaLatitude);

	// Note, GetGridValue overload used below will check the validity of the
	// index values, not need to do so here.
	ok = GetGridValue (result,recIdx,eleIdx);
	return ok;
}
bool TcsLasLosFile::SetGridValue (long32_t recIdx,long32_t eleIdx,float gridValue)
{
	bool ok (false);

	if (RecCount > 0L && recIdx >= 0L && recIdx < RecCount &&
		EleCount > 0L && eleIdx >= 0L && eleIdx < EleCount &&
		DataImage != 0)
	{
		size_t index = (recIdx * EleCount) + eleIdx;
		DataImage [index] = gridValue;
		ok = true;
	}
	return ok;
}
bool TcsLasLosFile::SetGridValue (double longitude,double latitude,double gridValue)
{
	// This round value may be a bit too small.  We could easily go to one  half
	// a minute as far as any las/los file we know of.  Be careful!  The las/los
	// files for Hawaii and PRVI have ugly numbers for the grid cell size which
	// may cause problems here.
	static const double rndValue = (1.0 / 3600) * 0.5;	// one half second in degrees

	bool ok (false);
	long32_t recIdx;
	long32_t eleIdx;

	eleIdx = (long32_t)((longitude - SwLongitude + rndValue) / DeltaLongitude);
	recIdx = (long32_t)((latitude  - SwLatitude  + rndValue) / DeltaLatitude);

	// Note, SetGridValue overload used below will check the validity of the
	// index values, not need to do so here.
	ok = SetGridValue (recIdx,eleIdx,gridValue);
	return ok;
}
bool TcsLasLosFile::GetGridLocation (double lngLat [2],long32_t recIdx,long32_t eleIdx) const
{
	bool ok;

	ok = (RecCount > 0L && recIdx >= 0L && recIdx < RecCount &&
		 EleCount > 0L && eleIdx >= 0L && eleIdx < EleCount &&
		 DataImage != 0);
	if (ok)
	{
		lngLat [0] = SwLongitude + (DeltaLongitude * static_cast<double>(eleIdx));
		lngLat [1] = SwLatitude  + (DeltaLatitude  * static_cast<double>(recIdx));
	}
	return ok;
}
bool TcsLasLosFile::SetGridValue (long32_t recNbr,long32_t eleNbr,double gridValue)
{
	float floatValue = static_cast<float>(gridValue);
	bool ok = SetGridValue (recNbr,eleNbr,floatValue);
	return ok;
}
void TcsLasLosFile::SetFileIdent (const char* fileIdent)
{
	memset (FileIdent,'\0',sizeof (FileIdent));
	if (fileIdent != 0)
	{
		CS_stncp (FileIdent,fileIdent,sizeof (FileIdent));
	}
}
void TcsLasLosFile::SetProgram (const char* program)
{
	memset (Program,'\0',sizeof (Program));
	if (program != 0)
	{
		CS_stncp (Program,program,sizeof (Program));
	}
}
bool TcsLasLosFile::IsCovered (double longitude,double latitude) const
{
	bool isCovered;

	double maxLongitude = SwLongitude + (DeltaLongitude * static_cast<double>(EleCount - 1));
	isCovered = (longitude >= SwLongitude) && (longitude <= maxLongitude);
	if (isCovered)
	{
		double maxLatitude = SwLatitude + (DeltaLatitude * static_cast<double>(RecCount - 1));
		isCovered = (latitude >= SwLatitude) && (latitude <= maxLatitude);
	}
	return isCovered;
}
bool TcsLasLosFile::IsCovered (double lngLat [2]) const
{
	return IsCovered (lngLat [0],lngLat [1]);
}
//=============================================================================
// This function does not work well.  It relies on the subject grid geometry
// meeting some standards which are not usually met.  Essentially, a geometry
// such as:
//
//        *********************************************
//        *********************************************
//        *****************        ********************
//        *****************        ********************
//        *****************        ********************
//                                 ********************
//                                 ********************
// will generate holes in a grid which previously did not have any holes.
// Use of this function is commented out until a better solution can be
// devised. 
//=============================================================================
// This function finds the edge of coverage for each record and column, then
// fills the edge 'No Value' with the edge real value times delta.
bool TcsLasLosFile::EdgeFillDelta (double delta)
{
	bool ok;

	long32_t eleIdx;
	long32_t recIdx;

	double gridValue;
	double edgeValue;

	//  Yes, delta had better be less than 1.0, and should not be negative.
	ok = (delta < 1.0) && (delta > 0.0);

	// As east/west aggregate of the 48 state is greater east to west than it
	// is north to south, we do the element columns first.
	for (eleIdx = 0L;ok && eleIdx < EleCount;eleIdx += 1)
	{
		// Do the south edge of each element column.
		for (recIdx = 0L;ok && recIdx < RecCount;recIdx += 1)
		{
			ok = GetGridValue (gridValue,recIdx,eleIdx);
			if (ok && (gridValue > NoValueTest))
			{
				// Found a real value.  Set the previous 'No Value' to the
				// indicated value, and then we are done.  Nothing to do if the
				// edge is the grid boundary.
				if (recIdx > 0)
				{
					edgeValue = gridValue * delta;
					ok = SetGridValue ((recIdx - 1L),eleIdx,edgeValue);
				}
				break;
			}
			// Normal termination of the loop (i.e. no break) means the entire
			// element column is 'No Value'; and we purposely leave it that way.
		}
		// Do the north edge.
		for (recIdx = (RecCount - 1);ok && recIdx >= 0;recIdx -= 1)
		{
			ok = GetGridValue (gridValue,recIdx,eleIdx);
			if (ok && (gridValue > NoValueTest))
			{
				// Found a real value.  Set the previous 'No Value' to the
				// indicated value, and then we are done.  Nothing to do if the
				// edge is the grid boundary.
				if (recIdx < (RecCount - 1L))
				{
					edgeValue = gridValue * delta;
					ok = SetGridValue ((recIdx + 1L),eleIdx,edgeValue);
				}
				break;
			}
			// Normal termination of the loop (i.e. no break) means the entire
			// element column is 'No Value'; and we leave it that way.
		}
	}

	// Do all the east/west edges now.
	for (recIdx = 0L;ok && recIdx < RecCount;recIdx += 1)
	{
		for (eleIdx = 0L;ok && eleIdx < EleCount;eleIdx += 1)
		{
			ok = GetGridValue (gridValue,recIdx,eleIdx);
			if (ok && (gridValue > NoValueTest))
			{
				// Found a real value.  Set the previous 'No Value' to the
				// indicated value, and then we are done.  Nothing to do if the
				// edge is the grid boundary.
				if (eleIdx > 0)
				{
					edgeValue = gridValue * delta;
					ok = SetGridValue (recIdx,(eleIdx - 1L),edgeValue);
				}
				break;
			}
			// Normal termination of the loop (i.e. no break) means the entire
			// record is 'No Value'; and we leave it that way.
		}
		for (eleIdx = (EleCount - 1L);ok && eleIdx >= 0;eleIdx -= 1)
		{
			ok = GetGridValue (gridValue,recIdx,eleIdx);
			if (ok && (gridValue > NoValueTest))
			{
				// Found a real value.  Set the previous 'No Value' to the
				// indicated value, and then we are done.  Nothing to do if the
				// edge is the grid boundary.
				if (eleIdx < (EleCount - 1L))
				{
					edgeValue = gridValue * delta;
					ok = SetGridValue (recIdx,(eleIdx + 1L),edgeValue);
				}
				break;
			}
			// Normal termination of the loop (i.e. no break) means the entire
			// element column is 'No Value'; and we leave it that way.
		}
	}
	return ok;
}
// This function replaces all remaining edge 'No Value' cells with the
// specified fill value.
bool TcsLasLosFile::EdgeFill (double fillValue)
{
	bool ok (true);
	long32_t recIdx;
	long32_t eleIdx;
	float gridValue;
	float myFillValue;

	myFillValue = static_cast<float>(fillValue);

	for (recIdx = 0L;ok && recIdx < RecCount;recIdx += 1)
	{
		for (eleIdx = 0L;ok && eleIdx < EleCount;eleIdx += 1)
		{
			ok = GetGridValue (gridValue,recIdx,eleIdx);
			// We have a separate test value for edges, but at this point,
			// there should not be any.
			if (ok && (gridValue > NoValueTest))
			{
				// Found a real value, this is the western edge of valid data.
				break;
			}
			ok = SetGridValue (recIdx,eleIdx,EdgeValue);
		}
		for (eleIdx = (EleCount - 1L);ok && eleIdx >= 0;eleIdx -= 1)
		{
			ok = GetGridValue (gridValue,recIdx,eleIdx);
			if (ok && (gridValue > NoValueTest))
			{
				// Found a real value, the eastern edge of valid data.
				break;
			}
			ok = SetGridValue (recIdx,eleIdx,EdgeValue);
		}
	}
	// Now we locate the edges in the north/south direction.  The above will
	// have marked lots of cells with the EdgeValue.  This is true in the
	// cases where the entire record was in the "edge" area.  The algorithm
	// below needs to consider these values as NoValue.  That is why we have
	// the two different values.
	for (eleIdx = 0L;ok && eleIdx < EleCount;eleIdx += 1)
	{
		for (recIdx = 0L;ok && recIdx < RecCount;recIdx += 1)
		{
			ok = GetGridValue (gridValue,recIdx,eleIdx);
			// Using EdgeValueTest, this test should stop this loop only when
			// we hit a real grid value (i.e. a value in the [-10.0:+10.0]
			// range.
			if (ok && (gridValue > EdgeValueTest))
			{
				// Found the southern edge of valid data.
				break;
			}
			// It may already be set to EdgeValue, but we make sure.
			ok = SetGridValue (recIdx,eleIdx,EdgeValue);
		}
		for (recIdx = (RecCount - 1L);ok && recIdx >= 0;recIdx -= 1)
		{
			ok = GetGridValue (gridValue,recIdx,eleIdx);
			if (ok && (gridValue > EdgeValueTest))
			{
				// Found the northern edge of valid data.
				break;
			}
			ok = SetGridValue (recIdx,eleIdx,EdgeValue);
		}
	}

	// Having done the above, we have properly identified all cells which are
	// edge cells and we replace these value with zeros.
	for (eleIdx = 0L;ok && eleIdx < EleCount;eleIdx += 1)
	{
		for (recIdx = 0L;ok && recIdx < RecCount;recIdx += 1)
		{
			ok = GetGridValue (gridValue,recIdx,eleIdx);
			if (ok && (gridValue < NoValueTest))
			{
				// This is edge fill, specifically do not want to fill
				// NoValue cells; at least not yet.
				continue;
			}
			if (ok && (gridValue < EdgeValueTest))
			{
				ok = SetGridValue (recIdx,eleIdx,myFillValue);
			}
		}
	}
	return ok;
}
long32_t TcsLasLosFile::HoleCheck (std::wostream& rptStream,bool verbose)
{
	bool ok (true);
	long32_t holeCount (0);

	float gridValue;
	double gridLL [2];

	for (long32_t recIdx = 0L;ok && recIdx < RecCount;recIdx += 1)
	{
		for (long32_t eleIdx = 0L;ok && eleIdx < EleCount;eleIdx += 1)
		{
			ok = GetGridValue (gridValue,recIdx,eleIdx);
			if (ok)
			{
				// Note, this test will also find edge values which have
				// not been properly dealt with.
				if (verbose && gridValue < EdgeValueTest)
				{
					holeCount += 1;
					GetGridLocation (gridLL,recIdx,eleIdx);
					std::wcout << "Hole detected at "
							   << -gridLL [0]
							   << L"W and "
							   << gridLL [1]
							   << L"N."
							   << std::endl;
				}
			}
		}
	}
	if (!ok)
	{
		holeCount = -1;
	}
	return holeCount;
}
bool TcsLasLosFile::WriteToFile (const char* filePath) const
{
	bool ok;
	
	size_t wrCnt;
	float ioFloat;
	long32_t ioLong;
	long recordSize;

	// If we don't have any data, there is nothing much to do but fail.
	ok = (DataImage != 0);
	if (ok)
	{
		// When we get to writing data, we will need to know the size of a
		// data record.  The Header must also be exactly this size.  The
		// sizeof (long32_t) is required to account for the record number
		// element which FORTRAN likes to write in front of each record.
		recordSize = EleCount * sizeof (float) + sizeof (long32_t);

		// Open the file, truncate or create as appropriate.
		csFILE* lasStrm = CS_fopen (filePath,_STRM_BINWR);
		ok = (lasStrm != NULL);
		if (ok)
		{
			// To avoid compiler dependent structure packing problems, we write
			// each element of a NADCON filer header individually.

			// Should we do something about by swapping  here?  Currently,
			// we intend to use this function once, in the Windows environment,
			// so we'll simply assume a little endian environment all around.
			
			// Write 56 bytes of FileIdent.
			wrCnt  = CS_fwrite (FileIdent,1,sizeof (FileIdent),lasStrm);
			ok = (wrCnt == sizeof (FileIdent));

			// Write 8 bytes of Program.
			wrCnt  = CS_fwrite (Program,1,sizeof (Program),lasStrm);
			ok &= (wrCnt == sizeof (Program));

			// Element Count, and so forth
			wrCnt  = CS_fwrite (&EleCount,1,sizeof (EleCount),lasStrm);
			ok &= (wrCnt == sizeof (EleCount));

			wrCnt  = CS_fwrite (&RecCount,1,sizeof (RecCount),lasStrm);
			ok &= (wrCnt == sizeof (RecCount));

			wrCnt  = CS_fwrite (&ZeeCount,1,sizeof (ZeeCount),lasStrm);
			ok &= (wrCnt == sizeof (ZeeCount));

			ioFloat = static_cast<float>(SwLongitude);
			wrCnt  = CS_fwrite (&ioFloat,1,sizeof (ioFloat),lasStrm);
			ok &= (wrCnt == sizeof (ioFloat));

			ioFloat = static_cast<float>(DeltaLongitude);
			wrCnt  = CS_fwrite (&ioFloat,1,sizeof (ioFloat),lasStrm);
			ok &= (wrCnt == sizeof (ioFloat));

			ioFloat = static_cast<float>(SwLatitude);
			wrCnt  = CS_fwrite (&ioFloat,1,sizeof (ioFloat),lasStrm);
			ok &= (wrCnt == sizeof (ioFloat));

			ioFloat = static_cast<float>(DeltaLatitude);
			wrCnt  = CS_fwrite (&ioFloat,1,sizeof (ioFloat),lasStrm);
			ok &= (wrCnt == sizeof (ioFloat));

			ioFloat = static_cast<float>(Angle);
			wrCnt  = CS_fwrite (&ioFloat,1,sizeof (ioFloat),lasStrm);
			ok &= (wrCnt == sizeof (ioFloat));

			// OK, the Header record has to exactly as long as any other record.
			// So we have to write some zeros until s uch time as we have
			// written the equivalent of a data record.  What we have written
			// above is the equivalent of 96 bytes.  We could make the
			// performance of this function a skoch better at the expense of
			// more complexity.  I choose the simple appropach.
			ioFloat = 0.0;
			while (ok && ftell (lasStrm) < recordSize)
			{
				ioFloat = static_cast<float>(Angle);
				wrCnt  = CS_fwrite (&ioFloat,1,sizeof (ioFloat),lasStrm);
				ok &= (wrCnt == sizeof (ioFloat));
			}
		}
		// We still good, we can write RecCount data records now.
		for (long32_t recIdx = 0L;ok && recIdx < RecCount;recIdx += 1)
		{
			ioLong = recIdx + 1;
			wrCnt  = CS_fwrite (&ioLong,1,sizeof (ioLong),lasStrm);
			ok &= (wrCnt == sizeof (ioLong));
			for (long32_t eleIdx = 0L;eleIdx < EleCount;eleIdx += 1)
			{
				size_t index = (recIdx * EleCount) + eleIdx;
				ioFloat = DataImage [index];
				wrCnt  = CS_fwrite (&ioFloat,1,sizeof (ioFloat),lasStrm);
				ok &= (wrCnt == sizeof (ioFloat));
			}
		}
		CS_fclose (lasStrm);
	}
	return ok;
}
bool TcsLasLosFile::WriteToFile (const wchar_t* filePath) const
{
	char ccFilePath [MAXPATH];
	
	wcstombs (ccFilePath,filePath,sizeof (ccFilePath));
	bool ok = WriteToFile (ccFilePath);
	return ok;
}
// The following is useful for examining the data values in a grid file
// using a spreadsheet program such as Excel.
bool TcsLasLosFile::WriteGridToCsv (const char* filePath) const
{
	bool ok;
	
	// If we don't have any data, there is nothing much to do but fail.
	ok = (DataImage != 0);
	if (ok)
	{
		// Open the file, truncate or create as appropriate.
		csFILE* csvStrm = CS_fopen (filePath,_STRM_BINWR);
		ok = (csvStrm != NULL);
		if (ok)
		{
			// We're still good, we can write RecCount data records now.
			// Write a header record with EleCount cells which carry the
			// longitude values.
			double cellLongitude = SwLongitude;
			fprintf (csvStrm,"Lat/Long,");
			for (long32_t eleIdx = 0L;eleIdx < EleCount;eleIdx += 1)
			{
				if (eleIdx != (EleCount - 1))
				{
					fprintf (csvStrm,"%#8.3f,",cellLongitude);
				}
				else
				{
					fprintf (csvStrm,"%#8.3f\n",cellLongitude);
				}
				cellLongitude += DeltaLongitude;
			}

			// We write in reverse record order so that when viewing in Excel,
			// the values are displayed with north on top and south on the
			// bottom. I got tired of having to sort the spreadsheet.
			double recLatitude = SwLatitude + (DeltaLatitude * static_cast<double>(RecCount - 1));
			for (long32_t recIdx = (RecCount - 1);ok && recIdx >= 0;recIdx -= 1)
			{
				fprintf (csvStrm,"%#8.3f,",recLatitude);
				for (long32_t eleIdx = 0L;eleIdx < EleCount;eleIdx += 1)
				{
					double gridValue;
					ok = GetGridValue (gridValue,recIdx,eleIdx);
					if (eleIdx != (EleCount - 1))
					{
						fprintf (csvStrm,"%#8.5f,",gridValue);
					}
					else
					{
						fprintf (csvStrm,"%#8.5f\n",gridValue);
					}
				}
				recLatitude -= DeltaLatitude;
			}
			CS_fclose (csvStrm);
		}
	}
	return ok;
}
//newPage//
//=============================================================================
// TcsFenceElement -- An element of a two dimensional fence
TcsFenceElement::TcsFenceElement (void) : Xx (0.0),
										  Yy (0.0)
{
}
TcsFenceElement::TcsFenceElement (double xx,double yy) : Xx (xx),
														 Yy (yy)
{
}
TcsFenceElement::TcsFenceElement (const TcsFenceElement& source) : Xx (source.Xx),
																   Yy (source.Yy)
{
}
TcsFenceElement TcsFenceElement::operator= (const TcsFenceElement& rhs)
{
	if (this != &rhs)
	{
		Xx = rhs.Xx;
		Yy = rhs.Yy;
	}
	return *this;
}
TcsFenceElement::~TcsFenceElement (void)
{
}
//=============================================================================
// TcsFence --  A collection of TcsFenceElement's.  Note a usable collection of
// this type has impled properties of:
//	1> The collection is closed; the last element is the same as the first.
//	2> The implied boundary segments do not cross each other in any way.
//	3> There are no holes or otherwise internal sub-boundaries
// A successful call to the CLose function inplies that all of these implied
//
// In its current state, this object is designed to provide the "is a point
// inside the boundary function" for a very specific case. More general use
// will require an upgrade; be forewarned.
TcsFence::TcsFence (void) : Ok       (false),
							Fuzz     (1.0),
							Elements ()
{
	RemovedPoint [0] = 0.0;
	RemovedPoint [1] = 0.0;
	LowerLeft [0]    =  1.0;
	LowerLeft [1]    =  1.0;
	UpperRight [0]   = -1.0;
	UpperRight [1]   = -1.0;
}
TcsFence::TcsFence (const char* epsgXmlPolygonFile) : Ok       (false),
													  Fuzz     (1.0),
													  Elements ()
{
	ProcessEpsgXml (epsgXmlPolygonFile);
	Close ();
}
TcsFence::TcsFence (const TcsFence& source) : Ok       (source.Ok),
											  Fuzz     (source.Fuzz),
											  Elements (source.Elements)
{
	RemovedPoint [0] = source.RemovedPoint [0];
	RemovedPoint [1] = source.RemovedPoint [1];
	LowerLeft [0]    = source.LowerLeft [0];
	LowerLeft [1]    = source.LowerLeft [1];
	UpperRight [0]   = source.UpperRight [0];
	UpperRight [1]   = source.UpperRight [1];
}
TcsFence TcsFence::operator= (const TcsFence& rhs)
{
	if (this != &rhs)
	{
		Ok               = rhs.Ok;
		Fuzz             = rhs.Fuzz;
		RemovedPoint [0] = rhs.RemovedPoint [0];
		RemovedPoint [1] = rhs.RemovedPoint [1];
		LowerLeft [0]    = rhs.LowerLeft [0];
		LowerLeft [1]    = rhs.LowerLeft [1];
		UpperRight [0]   = rhs.UpperRight [0];
		UpperRight [1]   = rhs.UpperRight [1];
		Elements         = rhs.Elements;
	}
	return *this;
}
TcsFence::~TcsFence (void)
{
}
TcsFence& TcsFence::operator+ (const TcsFenceElement& newElement)
{
	Ok = Add (newElement);
	return *this;
}
void TcsFence::operator+= (const TcsFenceElement& newElement)
{
	Ok = Add (newElement);
}
bool TcsFence::Add (const TcsFenceElement& newElement)
{
	if (IsClosed ())			// true return implies eleCount > 2
	{
		Elements.pop_back ();
	}
	Elements.push_back (newElement);
	Ok = false;
	return true;
}
bool TcsFence::Close (void)
{
	Ok = SetUpCalc ();
	return Ok;
}
bool TcsFence::IsInside (double xx,double yy) const
{
	bool isInside;
	double queryPoint [2];

	queryPoint [0] = xx;
	queryPoint [1] = yy;
	isInside = IsInside (queryPoint);
	return isInside;
}
bool TcsFence::IsInside (double xy [2]) const
{
	bool isInside (false);
	bool isOn (false);

	long32_t isectCount (0);
	EcsIntersectionType isectType;

	double fenceFrom [2];
	double fenceTo [2];
	double isectPoint [2];

	std::vector<TcsFenceElement>::const_iterator curItr;
	std::vector<TcsFenceElement>::const_iterator nxtItr;

	if (Ok)			// Means its closed and has 3 or more points + close point.
	{
		curItr = nxtItr = Elements.begin ();
		++nxtItr;
		while (nxtItr != Elements.end ())
		{
			fenceFrom [0] = curItr->GetX ();
			fenceFrom [1] = curItr->GetY ();
			fenceTo   [0] = nxtItr->GetX ();
			fenceTo   [1] = nxtItr->GetY ();
			isectType = CS_intersection2D (fenceFrom,fenceTo,RemovedPoint,xy,isectPoint);
			if (isectType == isectBoth)
			{
				isectCount += 1;
				if (!isOn)
				{
					double deltaX = isectPoint [0] - xy [0];
					double deltaY = isectPoint [1] - xy [1];
					isOn = (fabs (deltaX) < Fuzz) && (fabs (deltaY) < Fuzz);
				}
			}
			curItr = nxtItr;
			++nxtItr;
		}
		isInside = ((isectCount & 1) != 0) || isOn;
	}
	return isInside;
}
bool TcsFence::ProcessEpsgXml (const char* epsgXmlPolygonFile)
{
	bool ok;

	char *prsPtr0;
	char *prsPtr1;
	char *prsPtr2;
	char *xmlPtr;
	char *startPtr;
	char *endPtr;
	char *startTag = "<gml:posList>";
	char *endTag = "</gml:posList>";

	// Prepare for possible error;
	xmlPtr = 0;

	// Open the file.
	csFILE* xmlStrm = CS_fopen (epsgXmlPolygonFile,_STRM_TXTRD);
	ok = (xmlStrm != NULL);
	if (ok)
	{
		// Read in the entire file.
		CS_fseek (xmlStrm,0,SEEK_END);
		long fileSize = CS_ftell (xmlStrm);
		CS_fseek (xmlStrm,0,SEEK_SET);
		xmlPtr = new char [static_cast<size_t>(fileSize)];
		ok = (xmlPtr != NULL);
		if (ok)
		{
			size_t rdCount = CS_fread (xmlPtr,1,fileSize,xmlStrm);
			ok = (static_cast<long>(rdCount) == fileSize);
		}
		CS_fclose (xmlStrm);
	}
	startPtr = 0;
	endPtr = 0;
	if (ok)
	{
		// Find the start of the polygon data.
		startPtr = strstr (xmlPtr,startTag);
		ok = (startPtr != 0);
		if (ok)
		{
			startPtr += strlen (startTag);
			
			// Find the end of the polygon data.
			endPtr = strstr (startPtr,endTag);
			ok = (endPtr != 0);
		}
	}
	
	// Parse out the individual points and add to the Elements collection.
	if (ok)
	{
		prsPtr0 = startPtr;
		while (prsPtr0 < endPtr)
		{
			double yy = strtod (prsPtr0,&prsPtr1);
			prsPtr1 += 1;
			double xx = strtod (prsPtr1,&prsPtr2);
			prsPtr2 += 1;
			TcsFenceElement newElement (xx,yy);
			Elements.push_back (newElement);

			while (prsPtr2 < endPtr && isspace (*prsPtr2)) prsPtr2++;
			prsPtr0 = prsPtr2;
		}
	}
	delete [] xmlPtr;
	return ok;
}
bool TcsFence::SetUpCalc (void)
{
	bool ok (false);
	std::vector<TcsFenceElement>::iterator eleItr;

	// All of this works and is "conceptually sound even if there is only one
	// element.
	if (!Elements.empty ())
	{
		for (eleItr = Elements.begin ();eleItr != Elements.end ();eleItr++)
		{
			if (eleItr == Elements.begin ())
			{
				LowerLeft [0] = UpperRight [0] = eleItr->GetX ();
				LowerLeft [1] = UpperRight [1] = eleItr->GetY ();
			}
			else
			{
				if (eleItr->GetX () < LowerLeft [0])  LowerLeft [0]  = eleItr->GetX ();
				if (eleItr->GetY () < LowerLeft [1])  LowerLeft [1]  = eleItr->GetY ();
				if (eleItr->GetX () > UpperRight [0]) UpperRight [0] = eleItr->GetX ();
				if (eleItr->GetY () > UpperRight [1]) UpperRight [1] = eleItr->GetY ();
			}
		}
		
		// Compute a Fuzz value.
		Fuzz = fabs (UpperRight [0]);
		if (fabs (UpperRight [1]) > Fuzz) Fuzz = fabs (UpperRight [1]);
		if (fabs (LowerLeft  [0]) > Fuzz) Fuzz = fabs (LowerLeft  [0]);
		if (fabs (LowerLeft  [1]) > Fuzz) Fuzz = fabs (LowerLeft  [1]);
		Fuzz *= 1.0E-12;
		
		// Compute a removed point.  That is, a point we know to be outside any
		// possible boundary constructed from the points in cluded in this
		// fence so far. We arbitrairly choose a point to the lower left (i.e.
		// southwest) of the boundary as defined (so far at least).
		RemovedPoint [0] = LowerLeft [0] - fabs (LowerLeft [0]);
		RemovedPoint [1] = LowerLeft [1] - fabs (LowerLeft [1]);

		// Last thing is to close the boundary and compute the OK flag.  This
		// we do only in the case where the fence has at least 3 (presumably
		// unique) elements in it.  Note we do not set the Ok member, we only
		// compute the value and return it.
		size_t eleCount = Elements.size ();
		if (eleCount >= 3)
		{
			ok = true;
			if (!IsClosed ())
			{
				// Don't close the fence unless it needs it.  The user could
				// have done that already for us; but maybe not.  Note, the
				// IsCLosed function is indeed PRIVATE.  You don't want to call
				// that member function unless a valid Fuzz value has been
				// cacluated.
				eleItr = Elements.begin ();
				Elements.push_back (*eleItr);		// This should force a copy
			}
		}
	}
	return ok;
}
// The following function relies on the fact that a valid Fuzz value  has been
// calculated.  Note, this function will consider a fence with one point in it
// closed.  It will also consider a fence with exactly two points which are
// identical as closed.
bool TcsFence::IsClosed (void)
{
	bool closed (false);
	size_t eleCount = Elements.size ();
	if (eleCount > 0)
	{
		TcsFenceElement& first = Elements [0];
		TcsFenceElement& last  = Elements [eleCount - 1];
		double deltaX = fabs (first.GetX () - last.GetX ());
		double deltaY = fabs (first.GetY () - last.GetY ());
		closed = (deltaX < Fuzz) && (deltaY < Fuzz);
	}
	return closed;
}


