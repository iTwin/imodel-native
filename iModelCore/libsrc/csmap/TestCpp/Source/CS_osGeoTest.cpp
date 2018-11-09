/*
 * Copyright (c) 2008, Autodesk, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Autodesk, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Autodesk, Inc. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Autodesk, Inc. OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "csTestCpp.hpp"

static const TcsOsGeoTestField KcsOsGeoTestFields [] =
{
	{ fldNbrPointId,    L"TestName",        0UL},
	{ fldNbrTestType,   L"TestType",        0UL},
	{ fldNbrSrcAuth,    L"SrcCrsAuth",      0UL},
	{ fldNbrSrcCrs,     L"SrcCrs",          0UL},
	{ fldNbrTrgAuth,    L"TrgCrsAuth",      0UL},
	{ fldNbrTrgCrs,     L"TrgCrs",          0UL},
	{ fldNbrSrcOrd1,    L"SrcOrd1",         0UL},
	{ fldNbrSrcOrd2,    L"SrcOrd2",         0UL},
	{ fldNbrSrcOrd3,    L"SrcOrd3",         0UL},
	{ fldNbrTrgOrd1,    L"TrgOrd1",         0UL},
	{ fldNbrTrgOrd2,    L"TrgOrd2",         0UL},
	{ fldNbrTrgOrd3,    L"TrgOrd3",         0UL},
	{ fldNbrTolOrd1,    L"TolOrd1",         0UL},
	{ fldNbrTolOrd2,    L"TolOrd2",         0UL},
	{ fldNbrTolOrd3,    L"TolOrd3",         0UL},
	{ fldNbrUsing,      L"Using",           0UL},
	{ fldNbrDataSrc,    L"DataSource",      0UL},
	{ fldNbrComment,    L"Comment",         0UL},
	{ fldNbrMaxCnt,     L"",                0UL}	// End of table marker
};

const double cs_MetersPerDegree = 111319.5;		/* degree of longitude of the
												   WGS84 ellipsoid at the
												   equator. */

// The automatic test point generation algorithm used here does not
// always generate an appropriate test point for certain CRS's.  The
// following table provides a specific test point for specific CRS's
// for which this condition is known to be true.
static const TcsOsGeoTestPoints KcsOsGeoTestPoints [] =
{
	{ "BPCNC",        -81.0000000,  39.000000,  101.000 },	// BiPolar Conic
	{ "",               0.0000000,   0.000000,    0.000 }
};

//=============================================================================
// Implementation of the TcsCrsRange class.
//
// Used to generate test points and find CRS combinations whose useful ranges
// intersect.
//
const double TcsCrsRange::Fuzzy = 1.0E-07;
TcsCrsRange::TcsCrsRange (void)
{
	RangeSW [0] = RangeSW [1] = 0.0;
	RangeNE [0] = RangeNE [1] = 0.0;
	return;
}
TcsCrsRange::TcsCrsRange (double sw [2],double ne [2])
{
	RangeSW [0] = sw [0];
	RangeSW [1] = sw [1];
	RangeNE [0] = ne [0];
	RangeNE [1] = ne [1];
	return;
}
TcsCrsRange::TcsCrsRange (double swX,double swY,double neX,double neY)
{
	RangeSW [0] = swX;
	RangeSW [1] = swY;
	RangeNE [0] = neX;
	RangeNE [1] = neY;
	return;
}
TcsCrsRange::TcsCrsRange (const TcsCrsRange& source)
{
	RangeSW [0] = source.RangeSW [0];
	RangeSW [1] = source.RangeSW [1];
	RangeNE [0] = source.RangeNE [0];
	RangeNE [1] = source.RangeNE [1];
	return;
}
TcsCrsRange& TcsCrsRange::operator= (const TcsCrsRange& rhs)
{
	if (&rhs != this)
	{
		RangeSW [0] = rhs.RangeSW [0];
		RangeSW [1] = rhs.RangeSW [1];
		RangeNE [0] = rhs.RangeNE [0];
		RangeNE [1] = rhs.RangeNE [1];
	}
	return *this;
}
TcsCrsRange::~TcsCrsRange (void)
{
	return;
}
void TcsCrsRange::Reset (void)
{
	RangeSW [0] = RangeNE [0] = 0.0;
	RangeSW [1] = RangeNE [1] = 0.0;
	return;
}
void TcsCrsRange::SetSW (double sw [2])
{
	RangeSW [0] = sw [0];
	RangeSW [1] = sw [1];
	return;
}
void TcsCrsRange::SetNE (double ne [2])
{
	RangeNE [0] = ne [0];
	RangeNE [1] = ne [1];
	return;
}
void TcsCrsRange::SetRange (double sw [2],double ne [2])
{
	RangeSW [0] = sw [0];
	RangeSW [1] = sw [1];
	RangeNE [0] = ne [0];
	RangeNE [1] = ne [1];
	return;
}
bool TcsCrsRange::IsNull () const
{
	bool isNull;
	isNull = ((RangeNE [0] - RangeSW [0]) < FuzzEW ()) ||
			 ((RangeNE [1] - RangeSW [1]) < FuzzNS ());
	return isNull;
}
void TcsCrsRange::GetSW (double sw [2]) const
{
	sw [0] = RangeSW [0];
	sw [1] = RangeSW [1];
	return;
}
void TcsCrsRange::GetNE (double ne [2]) const
{
	ne [0] = RangeNE [0];
	ne [1] = RangeNE [1];
	return;
}
void TcsCrsRange::Include (double xy [2])
{
	Include (xy [0],xy [1]);
	return;
}
void TcsCrsRange::Include (double xx,double yy)
{
	if      (xx < RangeSW [0]) RangeSW [0] = xx;
	else if (xx > RangeNE [0]) RangeNE [0] = xx;
	if      (yy < RangeSW [1]) RangeSW [1] = yy;
	else if (yy > RangeNE [1]) RangeNE [1] = yy;
}
void TcsCrsRange::ExpandTo (const TcsCrsRange& range)
{
	if (range.RangeSW [0] < RangeSW [0]) RangeSW [0] = range.RangeSW [0];
	if (range.RangeSW [1] < RangeSW [1]) RangeSW [1] = range.RangeSW [1];
	if (range.RangeNE [0] > RangeNE [0]) RangeNE [0] = range.RangeNE [0];
	if (range.RangeNE [1] > RangeNE [1]) RangeNE [1] = range.RangeNE [1];
	return;
}
void TcsCrsRange::ShrinkTo (const TcsCrsRange& range)
{
	if (range.RangeSW [0] > RangeSW [0]) RangeSW [0] = range.RangeSW [0];
	if (range.RangeSW [1] > RangeSW [1]) RangeSW [1] = range.RangeSW [1];
	if (range.RangeNE [0] < RangeNE [0]) RangeNE [0] = range.RangeNE [0];
	if (range.RangeNE [1] < RangeNE [1]) RangeNE [1] = range.RangeNE [1];
	return;
}
bool TcsCrsRange::IsWithin (double xy [2],bool onIsOut) const
{
	return IsWithin (xy [0],xy [1],onIsOut);
}
bool TcsCrsRange::IsWithin (const double& xx,const double& yy,bool onIsOut) const
{
	bool isIn;

	if (onIsOut)
	{
		isIn = (xx > RangeSW [0]) && (xx < RangeNE [0]);
		if (isIn)
		{
			isIn = (yy > RangeSW [1]) && (xx < RangeNE [1]);
		}
	}
	else
	{
		isIn = (xx >= RangeSW [0]) && (xx <= RangeNE [0]);
		if (isIn)
		{
			isIn = (yy >= RangeSW [1]) && (xx <= RangeNE [1]);
		}
	}
	return isIn;
}
// Variation is used to move the generated test point around within the
// range of the CRS.  Intended to be the "point number" of a generated
// set of tests.
bool TcsCrsRange::GenerateTestPoint (double sourcePoint [3],int variation) const
{
	bool ok (false);

	int myVariation;

	double deltaEW;
	double deltaNS;
	double deltaZ;
	
	myVariation = variation % 7;
	deltaEW = fabs (RangeNE [0] - RangeSW [0]);
	deltaNS = fabs (RangeNE [1] - RangeSW [1]);

	switch (myVariation) {
	case 0:
		deltaEW *= 0.46;
		deltaNS *= 0.54;
		deltaZ   = 50.0;
		ok       = true;
		break;
	case 1:
		deltaEW *= 0.115;
		deltaNS *= 0.135;
		deltaZ   = 150.0;
		ok       = true;
		break;
	case 2:
		deltaEW *= 0.115;
		deltaNS *= 0.885;
		deltaZ   = 1.0;
		ok       = true;
		break;
	case 3:
		deltaEW *= 0.885;
		deltaNS *= 0.115;
		deltaZ   = 0.0;
		ok       = true;
		break;
	case 4:
		deltaEW *= 0.20;
		deltaNS *= 0.30;
		deltaZ   = 1500.0;
		ok       = true;
		break;
	case 5:
		deltaEW *= 0.72;
		deltaNS *= 0.67;
		deltaZ   = 0.0;
		ok       = true;
		break;
	case 6:
		deltaEW *= 0.12;
		deltaNS *= 0.87;
		deltaZ   = 10.0;
		ok       = true;
		break;
	default:
		deltaEW *= 0.47;
		deltaNS *= 0.58;
		deltaZ   = -50.0;
		ok       = true;
		break;
	}
	
	sourcePoint [0] = RangeSW [0] + deltaEW;
	sourcePoint [1] = RangeSW [1] + deltaNS;
	sourcePoint [2] = deltaZ;

	return ok;
}
double TcsCrsRange::FuzzEW (void) const
{
	double fuzz;
	double sw;
	double ne;

	sw = fabs (RangeSW [0]);
	ne = fabs (RangeNE [0]);
	fuzz = (sw > ne) ? sw : ne;
	fuzz *= Fuzzy;
	if (fuzz < Fuzzy) fuzz = Fuzzy;

	return fuzz;
}
double TcsCrsRange::FuzzNS (void) const
{
	double fuzz;
	double sw;
	double ne;

	sw = fabs (RangeSW [1]);
	ne = fabs (RangeNE [1]);
	fuzz = (sw > ne) ? sw : ne;
	fuzz *= Fuzzy;
	if (fuzz < Fuzzy) fuzz = Fuzzy;

	return fuzz;
}
//newPage//
//=============================================================================
// Implementation of the TcsTestCrs class.
//
// Associates a CRS name with the useful range of the named CRS.
//
TcsTestCrs::TcsTestCrs (void) : Geographic (false),
								EpsgCode   (0UL),
								SridCode   (0UL),
								OneMeter   (1.0),
								Range      ()
{
	memset (CrsName,'\0',sizeof (CrsName));
}
TcsTestCrs::TcsTestCrs (const char* crsName,bool isGeo,
											double oneMeter,
											const TcsCrsRange& range)
												:
											Geographic (isGeo),
											EpsgCode   (0UL),
											SridCode   (0UL),
											OneMeter   (oneMeter),
											Range      (range)
{
	CS_stncp (CrsName,crsName,sizeof (CrsName));
}
TcsTestCrs::TcsTestCrs (const char* crsName) : Geographic (false),
											   EpsgCode   (0UL),
											   SridCode   (0UL),
											   OneMeter   (1.0),
											   Range      ()
{
	double unitFactor;

	CS_stncp (CrsName,crsName,sizeof (CrsName));
	cs_Csdef_ *csDefPtr = CS_csdef (CrsName);
	if (csDefPtr != 0)
	{
		Geographic = !CS_stricmp (csDefPtr->prj_knm,"LL");
		if (Geographic)
		{
			unitFactor = CS_unitlu (cs_UTYP_ANG,csDefPtr->unit);
			if (unitFactor == 0.0)
			{
				unitFactor = 1.0;
			}
			OneMeter = (1.0 / unitFactor) * (1.0 / cs_MetersPerDegree);
		}
		else
		{
			unitFactor = CS_unitlu (cs_UTYP_LEN,csDefPtr->unit);
			if (unitFactor == 0.0)
			{
				unitFactor = 1.0;
			}
			OneMeter = (1.0 / unitFactor);
		}
		EpsgCode = csDefPtr->epsgNbr;
		SridCode = csDefPtr->srid;
		Range.SetRange (csDefPtr->ll_min,csDefPtr->ll_max);
		CS_free (csDefPtr);
	}
	else
	{
		// Signal internal error by setting CrsName to the null string.
		memset (CrsName,'\0',sizeof (CrsName));
	}
}
TcsTestCrs::TcsTestCrs (const TcsTestCrs& source) : Geographic (source.Geographic),
													EpsgCode   (source.EpsgCode),
													SridCode   (source.SridCode),
													OneMeter   (source.OneMeter),
													Range      (source.Range)
{
	CS_stncp (CrsName,source.CrsName,sizeof (CrsName));
}
TcsTestCrs& TcsTestCrs::operator= (const TcsTestCrs& rhs)
{
	if (&rhs != this)
	{
		Geographic = rhs.Geographic;
		EpsgCode   = rhs.EpsgCode;
		SridCode   = rhs.SridCode;
		OneMeter   = rhs.OneMeter;
		Range      = rhs.Range;
		CS_stncp (CrsName,rhs.CrsName,sizeof (CrsName));
	}
	return *this;
}
TcsTestCrs::~TcsTestCrs (void)
{
}
void TcsTestCrs::SetCrsName (const char* crsName)
{
	CS_stncp (CrsName,crsName,sizeof (CrsName));
}
bool TcsTestCrs::IsValid () const
{
	bool valid;
	cs_Csdef_ *csDefPtr = 0;

	csDefPtr = CS_csdef (CrsName);
	valid = (csDefPtr != 0);
	if (valid)
	{
		valid = !Range.IsNull();
		CS_free (csDefPtr);
	}
	return valid;
}
//newPage//
//==============================================================================
// Implementation of TcsTestCase  -- Complete test case 
//
// A "Test Case" is a complete test case as constructed from a specific
// version of the library in preparation for writing to a Regression Test
// data file; AnD/OR as extracted from a Regression Test data file.
//
char TcsTestCase::IdPrefix [32] = "DefaultID-";
double TcsTestCase::CalcTolerance (double oneMeter)
{
	// Currently, this is global and is intended to produce a tolerance
	// of 2 millimeters for any given unit.
	return oneMeter * 0.002;
}
TcsTestCase::TcsTestCase () : SourceCrs   (),
							  TargetCrs   (),
							  Range       (),
							  PointID     (),
							  Using       (),
							  DataSource  (),
							  Comment     (),
							  Status      (0),
							  TestMthd    (testMthNone),
							  ThreeD      (false)
{
	for (unsigned idx = 0;idx < 3;idx += 1)
	{
		TestPnt [idx] = ResultPnt [idx] = Tolerance [idx] = 0.0;
	}
}
TcsTestCase::TcsTestCase (EcsTestMethod testMthd,const TcsTestCrs& srcCrs,
												 const TcsTestCrs& trgCrs,
												 unsigned testNbr)
													:
												 SourceCrs  (srcCrs),
												 TargetCrs  (trgCrs),
												 Range      (),
												 PointID    (),
												 Using      (),
												 DataSource (),
												 Comment    (),
												 Status     (0),
												 TestMthd   (testMthd),
												 ThreeD     (false)
{
	int status;
	char cnvrtBufr [256];
	double testPntLL [3];

	cs_Csprm_ *projPtr;

	// Extract data from the provided TcsTestCrs objects.
	ThreeD = (TestMthd == testMthCrs3D);

	// Calculate the intersection of the two ranges.
	Range = SourceCrs.GetRange ();
	Range.ShrinkTo (TargetCrs.GetRange ());

	// Generate the test point for this test.  This is necessary
	// in the case of geographic systems in order to accommodate
	// geographic systems in units other than degrees and/or
	// prime meridian other than Greenwich.
	Range.GenerateTestPoint (testPntLL,testNbr);		/*lint !e534 */

	// This automated algorithm above does not work well for certain CRS's,
	// therefore we check the exception list to determine if an override
	// is present.
	const TcsOsGeoTestPoints* tblPtr;
	for (tblPtr = KcsOsGeoTestPoints;tblPtr->CrsName [0] != '\0';tblPtr += 1)
	{
		if (!CS_stricmp (SourceCrs.GetCrsName (),tblPtr->CrsName))
		{
			testPntLL [0] = tblPtr->Longitude;
			testPntLL [1] = tblPtr->Latitude;
			testPntLL [2] = tblPtr->Height;
		}
	}

	if (TestMthd == testMthCrs2D)
	{
		testPntLL [2] = 0.0;
	}
	projPtr = CS_csloc (SourceCrs.GetCrsName ());
	status = CS_ll2cs (projPtr,TestPnt,testPntLL);
	CS_free (projPtr);

	// Calculate the appropriate tolerances.  Height/Z coordinates are always
	// meters.
	Tolerance [0] = Tolerance [1] = CalcTolerance (TargetCrs.GetOneMeter ());
	Tolerance [2] = (TestMthd == testMthCrs3D) ? CalcTolerance (1.0) : 0.0;

	sprintf (cnvrtBufr,"%s-%06u",IdPrefix,testNbr);
	PointID = cnvrtBufr;

	// Finally, calculate the CS_MAP result using whatever library version this
	// module is linked with.
	Status = Finish ();

	// If status is not zero, add a comment to the test to record this
	// occurrence.
	if (Status != 0)
	{
		sprintf (cnvrtBufr,"Test point generation is suspicious (%d).\n",Status);
		Comment += cnvrtBufr;
	}
	// Negative Cartesian coordinates are often a sign of a problem.
	if (!SourceCrs.IsGeo () && (TestPnt [0] < 0.0 || TestPnt [1] < 0.0))
	{
		sprintf (cnvrtBufr,"Test point is suspect: test coordinates are negative.\n");
		Comment += cnvrtBufr;
	}
}
// The following constructor is used to generate an inverse of an existing
// test case.  Note, that as Range is in standard geographic coordinates,
// it does not need to change.  We need at least one argument different
// from the normal copy constructor, and it turns out we need the 'testNbr'
// value to give the inverse test a new and different (we hope) PointID.
// So, we view this as an 'inverting copy constructor', distinguished from
// the normal copy constructor by the first argument.
TcsTestCase::TcsTestCase (unsigned testNbr,const TcsTestCase& fwdTestCase)
												:
										   SourceCrs   (fwdTestCase.TargetCrs),
										   TargetCrs   (fwdTestCase.SourceCrs),
										   Range       (fwdTestCase.Range),
										   PointID     (),
										   Using       (),
										   DataSource  (),
										   Comment     (),
										   Status      (0),
										   TestMthd    (fwdTestCase.TestMthd),
										   ThreeD      (fwdTestCase.ThreeD)
{
	char cnvrtBufr [256];

	TestPnt   [0] = fwdTestCase.ResultPnt [0];
	TestPnt   [1] = fwdTestCase.ResultPnt [1];
	TestPnt   [2] = fwdTestCase.ResultPnt [2];
	Tolerance [0] = Tolerance [1] = CalcTolerance (TargetCrs.GetOneMeter ());
	Tolerance [2] = (ThreeD) ? CalcTolerance (1.0) : 0.0;

	sprintf (cnvrtBufr,"%s-%06u",IdPrefix,testNbr);
	PointID = cnvrtBufr;

	Status = Finish ();
	
	if (Status != 0)
	{
		sprintf (cnvrtBufr,"Test point generation is suspect (status = %d).\n",Status);
		Comment += cnvrtBufr;
	}
	if (!SourceCrs.IsGeo () && (TestPnt [0] < 0.0 || TestPnt [1] < 0.0))
	{
		sprintf (cnvrtBufr,"Test point generation is suspect as test coords are negative.\n");
		Comment += cnvrtBufr;
	}
}
// Construct a Test Case obtained from a Regression Test data file.
TcsTestCase::TcsTestCase (const TcsCsvRecord& csvRecord) : SourceCrs  (),
														   TargetCrs  (),
														   Range      (),
														   PointID    (),
														   Using      (),
														   DataSource (),
														   Comment    (),
														   Status     (0),
														   TestMthd   (testMthNone),
														   ThreeD     (false)
{
	bool ok;
	short fieldCnt;
	short idx;

	EcsCrsAuthority srcAuth (crsAuthNone);
	EcsCrsAuthority trgAuth (crsAuthNone);

	wchar_t *wcPtr;

	char charData [256];
	char srcCrsName [256];
	char trgCrsName [256];

	TcsCsvStatus csvStatus;
	std::wstring fieldData;

	srcCrsName [0] = '\0';			// keep lint happy
	trgCrsName [0] = '\0';			// keep lint happy
	fieldCnt = csvRecord.FieldCount ();

	if (fieldCnt > fldNbrMaxCnt) fieldCnt = fldNbrMaxCnt;
	ok = (fieldCnt >= fldNbrMinCnt);
	for (idx = fldNbrFirst;ok && idx < fieldCnt;idx += 1)
	{
		ok = csvRecord.GetField (fieldData,idx,csvStatus);
		if (!ok)
		{
			continue;		/*lint !e845 */
		}
		wcstombs (charData,fieldData.c_str(),sizeof (charData));

		switch (idx) {
		case fldNbrPointId:
			// Point ID
			PointID = charData;
			break;

		case fldNbrTestType:
			// Test Method
			TestMthd = TcsOsGeoTestFile::TestMthdId (fieldData.c_str ());
			ThreeD = (TestMthd == testMthCrs3D);
			break;

		case fldNbrSrcAuth:
			// Source CRS name authority
			srcAuth = TcsOsGeoTestFile::TestAuthId (fieldData.c_str ());
			break;

		case fldNbrSrcCrs:
			// Source CRS name/ID
			CS_stncp (srcCrsName,charData,sizeof (srcCrsName));
			break;

		case fldNbrTrgAuth:
			// Target CRS name authority
			trgAuth = TcsOsGeoTestFile::TestAuthId (fieldData.c_str ());
			break;

		case fldNbrTrgCrs:
			// Target CRS name
			CS_stncp (trgCrsName,charData,sizeof (trgCrsName));
			break;

		case fldNbrSrcOrd1:
			// Test point Easting/X/Longitude
			TestPnt [0] = wcstod (fieldData.c_str (),&wcPtr);
			break;

		case fldNbrSrcOrd2:
			// Test point Northing/Y/Latitude
			TestPnt [1] = wcstod (fieldData.c_str (),&wcPtr);
			break;

		case fldNbrSrcOrd3:
			// Test point Elevation/Z/Height
			TestPnt [2] = wcstod (fieldData.c_str (),&wcPtr);
			break;

		case fldNbrTrgOrd1:
			// Result point Easting/X/Longitude
			ResultPnt [0] = wcstod (fieldData.c_str (),&wcPtr);
			break;

		case fldNbrTrgOrd2:
			// Result point Northing/Y/Latitude
			ResultPnt [1] = wcstod (fieldData.c_str (),&wcPtr);
			break;

		case fldNbrTrgOrd3:
			// Result point Elevation/Z/Height
			ResultPnt [2] = wcstod (fieldData.c_str (),&wcPtr);
			break;

		case fldNbrTolOrd1:
			// Easting/X/Longitude Tolerance
			Tolerance [0] = wcstod (fieldData.c_str (),&wcPtr);
			break;

		case fldNbrTolOrd2:
			// Northing/Y/Latitude Tolerance
			Tolerance [1] = wcstod (fieldData.c_str (),&wcPtr);
			break;

		case fldNbrTolOrd3:
			// Elevation/Z/Height Tolerance
			Tolerance [2] = wcstod (fieldData.c_str (),&wcPtr);
			break;

		case fldNbrUsing:
			// Using field
			Using = charData;
			break;

		case fldNbrDataSrc:
			// Using field
			DataSource = charData;
			break;

		case fldNbrComment:
			// Using field
			Comment = charData;
			break;

		default:
			ok = false;
			break;
		}
	}

	if (srcAuth == crsAuthCsMap)
	{
		SourceCrs = TcsTestCrs (srcCrsName);
	}
	if (trgAuth == crsAuthCsMap)
	{
		TargetCrs = TcsTestCrs (trgCrsName);
	}
}
TcsTestCase::TcsTestCase (const TcsTestCase& source) : SourceCrs   (source.SourceCrs),
													   TargetCrs   (source.TargetCrs),
													   Range       (source.Range),
													   PointID     (source.PointID),
													   Using       (source.Using),
													   DataSource  (source.DataSource),
													   Comment     (source.Comment),
													   Status      (source.Status),
													   TestMthd    (source.TestMthd),
													   ThreeD      (source.ThreeD)
{
	unsigned idx;

	for (idx = 0;idx < 3;idx += 1)
	{
		TestPnt   [idx] = source.TestPnt   [idx];
		ResultPnt [idx] = source.ResultPnt [idx];
		Tolerance [idx] = source.Tolerance [idx];
	}
}
TcsTestCase& TcsTestCase::operator= (const TcsTestCase& rhs)
{
	unsigned idx;

	if (&rhs != this)
	{
		SourceCrs   = rhs.SourceCrs;
		TargetCrs   = rhs.TargetCrs;
		Range       = rhs.Range;
		PointID     = rhs.PointID;
		Using       = rhs.Using;
		DataSource  = rhs.DataSource;
		Comment     = rhs.Comment;
		Status      = rhs.Status;
		TestMthd    = rhs.TestMthd;
		ThreeD      = rhs.ThreeD;

		for (idx = 0;idx < 3;idx += 1)
		{
			TestPnt   [idx] = rhs.TestPnt   [idx];
			ResultPnt [idx] = rhs.ResultPnt [idx];
			Tolerance [idx] = rhs.Tolerance [idx];
		}
	}
	return *this;
}
TcsTestCase::~TcsTestCase (void)
{
}
void TcsTestCase::GetTestPnt (double testPnt [3]) const
{
	testPnt [0] = TestPnt [0];
	testPnt [1] = TestPnt [1];
	testPnt [2] = TestPnt [2];
}
void TcsTestCase::GetResultPnt (double resultPnt [3]) const
{
	resultPnt [0] = ResultPnt [0];
	resultPnt [1] = ResultPnt [1];
	resultPnt [2] = ResultPnt [2];
}
void TcsTestCase::GetTolerance (double tolerance [3]) const
{
	tolerance [0] = Tolerance [0];
	tolerance [1] = Tolerance [1];
	tolerance [2] = Tolerance [2];
}
void TcsTestCase::SetTestPnt (const double testPnt [3])
{
	TestPnt [0] = testPnt [0];
	TestPnt [1] = testPnt [1];
	TestPnt [2] = testPnt [2];
}
void TcsTestCase::SetResultPnt (const double resultPnt [3])
{
	ResultPnt [0] = resultPnt [0];
	ResultPnt [1] = resultPnt [1];
	ResultPnt [2] = resultPnt [2];
}
void TcsTestCase::SetTolerance (const double tolerance [3])
{
	Tolerance [0]  = tolerance [0];
	Tolerance [1]  = tolerance [1];
	Tolerance [2]  = tolerance [2];
}
// Verify the content of the structure.  Does not perform the conversion test.
// A failure here is intended to mean something is wrong in the regression test
// file or the code manipulating it; not the library.
bool TcsTestCase::Validate (void) const
{
	bool ok;

	if (ThreeD)
	{
		ok = (TestMthd == testMthCrs3D);
	}
	else
	{
		ok = (TestMthd == testMthCrs2D);
	}

	ok &= !Range.IsNull();

	ok &= SourceCrs.IsValid ();
	ok &= TargetCrs.IsValid ();

	ok &= (TestPnt [0] != 0.0);
	ok &= (TestPnt [1] != 0.0);
	if (ThreeD)
	{
		ok &= (TestPnt [2] != 0.0);
	}

	ok &= (ResultPnt [0] != 0.0);
	ok &= (ResultPnt [1] != 0.0);
	if (ThreeD)
	{
		ok &= (ResultPnt [2] != 0.0);
	}

	ok &= (Tolerance [0] > 0.0);
	ok &= (Tolerance [1] > 0.0);
	if (ThreeD)
	{
		ok &= (Tolerance [2] > 0.0);
	}

	ok &= (PointID.length () != 0);

	return ok;
}
// This function will compute the actual test 'ResultPnt'
int TcsTestCase::Finish (void)
{
	int status;

	if (ThreeD)
	{
		ResultPnt [0] = TestPnt [0];
		ResultPnt [1] = TestPnt [1];
		ResultPnt [2] = TestPnt [2];
		status = CS_cnvrt3D (SourceCrs.GetCrsName(),TargetCrs.GetCrsName (),ResultPnt);
	}
	else
	{
		ResultPnt [0] = TestPnt [0];
		ResultPnt [1] = TestPnt [1];
		ResultPnt [2] = 0.0;
		status = CS_cnvrt (SourceCrs.GetCrsName(),TargetCrs.GetCrsName (),ResultPnt);
	}
	return status;
}
bool TcsTestCase::ExecuteTest (double& dd,int& status,double calcResult [3]) const
{
	bool ok (false);
	double delta [3];
	double calcPnt [3];

	dd = 1.0E+06;
	status = -1;
	if (TestMthd == testMthCrs2D)
	{
		calcPnt [0] = TestPnt [0];
		calcPnt [1] = TestPnt [1];
		status = CS_cnvrt (SourceCrs.GetCrsName(),TargetCrs.GetCrsName(),calcPnt);
		ok = (status >= 0);
		if (ok)
		{
			calcResult [0] = calcPnt [0];
			calcResult [1] = calcPnt [1];
			calcResult [2] = 0.0;

			delta [0] = fabs (calcPnt [0] - ResultPnt [0]);
			delta [1] = fabs (calcPnt [1] - ResultPnt [1]);
			ok = (delta [0] <= Tolerance [0]) &&
				 (delta [1] <= Tolerance [1]);
			dd = sqrt (delta [0] * delta [0] +
					   delta [1] * delta [1]);

			// dd is  now in target system units.  Convert to millimeters.
			dd = (dd / TargetCrs.GetOneMeter ()) * 1000.00;
		}
	}
	else if (TestMthd == testMthCrs3D)
	{
		calcPnt [0] = TestPnt [0];
		calcPnt [1] = TestPnt [1];
		calcPnt [2] = TestPnt [2];
		status = CS_cnvrt3D (SourceCrs.GetCrsName(),TargetCrs.GetCrsName(),calcPnt);
		ok = (status >= 0);
		if (ok)
		{
			calcResult [0] = calcPnt [0];
			calcResult [1] = calcPnt [1];
			calcResult [2] = calcPnt [2];

			delta [0] = fabs (calcPnt [0] - ResultPnt [0]);
			delta [1] = fabs (calcPnt [1] - ResultPnt [1]);
			delta [2] = fabs (calcPnt [2] - ResultPnt [2]);
			ok = (delta [0] <= Tolerance [0]) &&
				 (delta [1] <= Tolerance [1]) &&
				 (delta [2] <= Tolerance [2]);
			dd = sqrt (delta [0] * delta [0] + 
					   delta [1] * delta [1] +
					   delta [2] * delta [2]);

			// dd is  now in target system units.  Convert to millimeters.
			dd = (dd / TargetCrs.GetOneMeter ()) * 1000.00;
		}
	}
	return ok;
}
// Replaces internal contents into the provided CSV Record structure.
void TcsTestCase::ToCsvRecord (TcsCsvRecord& cvsRecord) const
{
	EcsCrsDblFormat dblFrmt (crsDblFrmtNone);
	TcsCsvStatus status;

	wchar_t cnvrtBufr [256];
	std::wstring fieldData;

	// Point ID
	mbstowcs (cnvrtBufr,PointID.c_str(),256);
	fieldData = cnvrtBufr;
	cvsRecord.ReplaceField (fieldData,fldNbrPointId,status);

	// Test Method
	fieldData = TcsOsGeoTestFile::TestMthdName (TestMthd);
	cvsRecord.ReplaceField (fieldData,fldNbrTestType,status);

	// Source CRS name authority
	fieldData = TcsOsGeoTestFile::CrsAuthName (crsAuthCsMap);
	cvsRecord.ReplaceField (fieldData,fldNbrSrcAuth,status);

	// Source CRS name
	mbstowcs (cnvrtBufr,SourceCrs.GetCrsName (),256);
	fieldData = cnvrtBufr;
	cvsRecord.ReplaceField (fieldData,fldNbrSrcCrs,status);

	// Target CRS name authority
	fieldData = TcsOsGeoTestFile::CrsAuthName (crsAuthCsMap);
	cvsRecord.ReplaceField (fieldData,fldNbrTrgAuth,status);

	// Target CRS name
	mbstowcs (cnvrtBufr,TargetCrs.GetCrsName (),256);
	fieldData = cnvrtBufr;
	cvsRecord.ReplaceField (fieldData,fldNbrTrgCrs,status);

	// Test point coordinates.
	dblFrmt = (SourceCrs.IsGeo ()) ? crsDblFrmtGeo : crsDblFrmtCart;
	CnvrtDouble (fieldData,TestPnt [0],dblFrmt);
	cvsRecord.ReplaceField (fieldData,fldNbrSrcOrd1,status);
	CnvrtDouble (fieldData,TestPnt [1],dblFrmt);
	cvsRecord.ReplaceField (fieldData,fldNbrSrcOrd2,status);
	CnvrtDouble (fieldData,TestPnt [2],dblFrmt);
	cvsRecord.ReplaceField (fieldData,fldNbrSrcOrd3,status);

	// Result point coordinates.
	dblFrmt = (TargetCrs.IsGeo ()) ? crsDblFrmtGeo : crsDblFrmtCart;
	CnvrtDouble (fieldData,ResultPnt [0],dblFrmt);
	cvsRecord.ReplaceField (fieldData,fldNbrTrgOrd1,status);
	CnvrtDouble (fieldData,ResultPnt [1],dblFrmt);
	cvsRecord.ReplaceField (fieldData,fldNbrTrgOrd2,status);
	CnvrtDouble (fieldData,ResultPnt [2],dblFrmt);
	cvsRecord.ReplaceField (fieldData,fldNbrTrgOrd3,status);

	// Tolerances
	dblFrmt = crsDblFrmtTol;
	CnvrtDouble (fieldData,Tolerance [0],dblFrmt);
	cvsRecord.ReplaceField (fieldData,fldNbrTolOrd1,status);
	CnvrtDouble (fieldData,Tolerance [1],dblFrmt);
	cvsRecord.ReplaceField (fieldData,fldNbrTolOrd2,status);
	CnvrtDouble (fieldData,Tolerance [2],dblFrmt);
	cvsRecord.ReplaceField (fieldData,fldNbrTolOrd3,status);

	// Using string
	mbstowcs (cnvrtBufr,Using.c_str(),256);
	fieldData = cnvrtBufr;
	cvsRecord.ReplaceField (fieldData,fldNbrUsing,status);

	// Data Source string
	mbstowcs (cnvrtBufr,DataSource.c_str(),256);
	fieldData = cnvrtBufr;
	cvsRecord.ReplaceField (fieldData,fldNbrDataSrc,status);

	// Comment string
	mbstowcs (cnvrtBufr,Comment.c_str(),256);
	fieldData = cnvrtBufr;
	cvsRecord.ReplaceField (fieldData,fldNbrComment,status);

	return;
}
void TcsTestCase::CnvrtDouble (std::wstring& result,double value,
													EcsCrsDblFormat frmt) const
{
	int prec (3);
	wchar_t cnvrtBufr [256];

	switch (frmt) {
	case crsDblFrmtCart:
		if (fabs (value) >= 1.0E-12)	// Can't take log of negative number or zero.
		{
			prec = 12 - static_cast<int>(log10 (fabs(value)));
		}
		else
		{
			prec = 3;
		}
		if (prec <= 0) prec =  1;
		if (prec > 10) prec = 10;
		swprintf (cnvrtBufr,256,L"%.*f",prec,value);
		break;
	case crsDblFrmtGeo:
		swprintf (cnvrtBufr,256,L"%.8f",value);
		break;
	case crsDblFrmtTol:
		swprintf (cnvrtBufr,256,L"%.2g",value);
		break;
	default:
		swprintf (cnvrtBufr,256,L"%.3g",value);
		break;
	}
	result = cnvrtBufr;
	return;
}
//newPage//
//==============================================================================
// TcsOsFeoTestFile  -- Implementation

//newPage//
//=========================================================================
// TcsOsGeoTestFile Implementation
const TcsOsGeoTestFile::TcsTestTypeMap TcsOsGeoTestFile::KcsTestTypeMap [] =
{
	{ testMthCrs2D,  L"Crs2D"     },	// Two dimensional CRS conversion
	{ testMthCrs3D,  L"Crs3D"     },	// Three dimensional CRS conversion
	{ testMthGeoid,  L"Geoid"     },	// Geoid Height determination
	{ testMthNone,   L""          }		// End of table marker
};
const TcsOsGeoTestFile::TcsAuthMap TcsOsGeoTestFile::KcsAuthMap [] =
{
	{ crsAuthCsMap,   L"CsMap"    },	// CS-MAP key name
	{ crsAuthEpsg,    L"EPSG"     },	// EPSG CRS code
//	{ crsAuthProj4,   L"Proj4"    },	// Not supported, as yet
	{ crsAuthSrid,    L"SRID"     },	// Oracle SRID identifier
	{ crsAuthNone,    L""         }		// End of table marker.
};
//=========================================================================
// Static Member Functions
const wchar_t* TcsOsGeoTestFile::TestMthdName (EcsTestMethod testMethodId)
{
	unsigned index;
	const wchar_t* rtnValue = 0;

	for (index = 0;KcsTestTypeMap [index].TestType != testMthNone;index += 1)
	{
		if (testMethodId == KcsTestTypeMap [index].TestType)
		{
			rtnValue = KcsTestTypeMap [index].TestName;
			break;
		}
	}
	return rtnValue;
}
EcsTestMethod TcsOsGeoTestFile::TestMthdId (const wchar_t* testMethodName)
{
	unsigned index;
	EcsTestMethod rtnValue (testMthUnknown);

	for (index = 0;KcsTestTypeMap [index].TestType != testMthNone;index += 1)
	{
		if (!CS_wcsicmp (testMethodName,KcsTestTypeMap [index].TestName))
		{
			rtnValue = KcsTestTypeMap [index].TestType;
			break;
		}
	}
	return rtnValue;
}
const wchar_t* TcsOsGeoTestFile::CrsAuthName (EcsCrsAuthority authorityId)
{
	unsigned index;
	const wchar_t* rtnValue = 0;

	for (index = 0;KcsAuthMap [index].Authority != crsAuthNone;index += 1)
	{
		if (authorityId == KcsAuthMap [index].Authority)
		{
			rtnValue = KcsAuthMap [index].AuthName;
			break;
		}
	}
	return rtnValue;
}
EcsCrsAuthority TcsOsGeoTestFile::TestAuthId (const wchar_t* authorityName)
{
	unsigned index;
	EcsCrsAuthority rtnValue (crsAuthUnknown);

	for (index = 0;KcsAuthMap [index].Authority != crsAuthNone;index += 1)
	{
		if (!CS_wcsicmp (authorityName,KcsAuthMap [index].AuthName))
		{
			rtnValue = KcsAuthMap [index].Authority;
			break;
		}
	}
	return rtnValue;
}
bool TcsOsGeoTestFile::WriteLabels (std::wofstream& regressStrm)
{
	bool ok (false);
	int tblIdx;
	const TcsOsGeoTestField* tblPtr;

	for (tblIdx = fldNbrFirst;tblIdx <= fldNbrLast;++tblIdx)
	{
		for (tblPtr = KcsOsGeoTestFields;tblPtr->FldNumber != fldNbrMaxCnt;tblPtr += 1)
		{
			if (tblPtr->FldNumber == tblIdx) break;
		}
		if (tblIdx <= fldNbrLast)
		{
			regressStrm << tblPtr->FldLabel;
		}
		if (tblIdx < fldNbrLast)
		{
			regressStrm << L',';
		}
	}
	regressStrm << std::endl;
	ok = regressStrm.good ();
	return ok;
}
//=============================================================================
// Construction, Destruction, and Assignment
//
// The purpose of this object is to generate and/or read a .CSV file with
// regression test capability.  Thus, we naturally derive from TcsCsvFileBase.
// All file I/O, parsing, and field extraction is accomplished through the
// base class.
//
// This constructor is used to read a regression test file for testing
// purposes. The regression test .CSV files are required to carry labels
// in the first record.  As each record has a comment field, comment
// records are not supported at the current time.  Note that a valid
// record in this file must contain at least 15 fields, but not more
// than 18.
//
TcsOsGeoTestFile::TcsOsGeoTestFile (const wchar_t* dataFilePath)
											:
									TcsCsvFileBase (true,14,18),
									Ok             (false),
									Status         ()
{
	bool firstIsLabels (true);

#if (_RUN_TIME >= _rt_UNIXPCC)
	// Linux/gcc does not support a wide character version of the
	// std:wifstream constructor.  Strange, but true.
	char dataFilePathChr [MAXPATH];
	wcstombs (dataFilePathChr,dataFilePath,sizeof (dataFilePathChr));
	std::wifstream inStrm (dataFilePathChr,std::ios_base::in);
#else
	std::wifstream inStrm (dataFilePath,std::ios_base::in);
#endif
	if (inStrm.is_open ())
	{
		Ok = ReadFromStream (inStrm,firstIsLabels,Status);
		inStrm.close ();
	}
}
TcsOsGeoTestFile::TcsOsGeoTestFile (const char* dataFilePath)
											:
									TcsCsvFileBase (true,14,18),
									Ok             (false),
									Status         ()
{
	bool firstIsLabels (true);

	std::wifstream inStrm (dataFilePath,std::ios_base::in);
	if (inStrm.is_open ())
	{
		Ok = ReadFromStream (inStrm,firstIsLabels,Status);        
		inStrm.close ();
	}
}
TcsOsGeoTestFile::TcsOsGeoTestFile (const TcsOsGeoTestFile& source)
									:
									TcsCsvFileBase (dynamic_cast<const TcsCsvFileBase&>(source)),
									Ok             (source.Ok),
									Status         (source.Status)
{
}
TcsOsGeoTestFile::~TcsOsGeoTestFile (void)
{
	// Nothing to do here.
}
TcsOsGeoTestFile& TcsOsGeoTestFile::operator= (const TcsOsGeoTestFile& rhs)
{
	if (&rhs != this)
	{
		TcsCsvFileBase::operator= (dynamic_cast<const TcsCsvFileBase&>(rhs));
		Ok     = rhs.Ok;
		Status = rhs.Status;
	}
	return *this;
}
//=========================================================================
// Operator Overrides
//=========================================================================
// Public Named Member Functions
const TcsCsvRecord& TcsOsGeoTestFile::GetRec (unsigned recordNbr) const
{
	return GetRecord (recordNbr);
}
bool TcsOsGeoTestFile::GetTestName (std::wstring& testName,unsigned recordNbr)
{
	bool ok (false);

	ok = GetField (testName,recordNbr,static_cast<short>(0),Status);
	return ok;
}
EcsTestMethod TcsOsGeoTestFile::GetTestMethod (unsigned recordNbr)
{
	bool ok (false);
	EcsTestMethod rtnValue (testMthUnknown);

	std::wstring fieldData;

	ok = GetField (fieldData,recordNbr,1,Status);
	if (ok)
	{
		rtnValue = TestMthdId (fieldData.c_str ());
	}
	return rtnValue;
}
EcsCrsAuthority TcsOsGeoTestFile::GetCrsKey (unsigned recordNbr,bool target)
{
	bool ok;
	short fldNbr;

	EcsCrsAuthority rtnValue (crsAuthUnknown);

	std::wstring fieldData;

	fldNbr = static_cast<short>(target ? fldNbrTrgAuth : fldNbrSrcAuth);
	ok = GetField (fieldData,recordNbr,fldNbr,Status);
	if (ok)
	{
		rtnValue = TestAuthId (fieldData.c_str ());
	}
	return rtnValue;
}
EcsCrsAuthority TcsOsGeoTestFile::GetSrcCrsKey (std::wstring& srcKey,unsigned recordNbr)
{
	bool ok (false);
	EcsCrsAuthority rtnValue (crsAuthUnknown);

	rtnValue = GetCrsKey (recordNbr,false);
	if (rtnValue != crsAuthUnknown)
	{
		ok = GetField (srcKey,recordNbr,3,Status);
		if (!ok)
		{
			rtnValue = crsAuthUnknown;
		}
	}
	return rtnValue;
}
EcsCrsAuthority TcsOsGeoTestFile::GetTrgCrsKey (std::wstring& trgKey,unsigned recordNbr)
{
	bool ok (false);
	EcsCrsAuthority rtnValue;

	rtnValue = GetCrsKey (recordNbr,true);
	if (rtnValue != crsAuthUnknown)
	{
		ok = GetField (trgKey,recordNbr,5,Status);
		if (!ok)
		{
			rtnValue = crsAuthUnknown;
		}
	}
	return rtnValue;
}
bool TcsOsGeoTestFile::GetSourceCoordinates (double sourceCoord [3],unsigned recordNbr)
{
	bool ok;
	wchar_t* wcPtr;
	double realX (0.0);
	double realY (0.0);
	double realZ (0.0);
	std::wstring fieldData;

	ok = GetField (fieldData,recordNbr,6,Status);
	if (ok)
	{
		realX = wcstod (fieldData.c_str (),&wcPtr);
		ok = (fabs (realX) < HUGE_VAL) ;
	}
	if (ok)
	{
		ok = GetField (fieldData,recordNbr,7,Status);
		if (ok)
		{
			realY = wcstod (fieldData.c_str (),&wcPtr);
			ok = fabs (realY) < HUGE_VAL;
		}
	}
	if (ok)
	{
		ok = GetField (fieldData,recordNbr,8,Status);
		if (ok)
		{
			realZ = wcstod (fieldData.c_str (),&wcPtr);
			ok = fabs (realZ) < HUGE_VAL;
		}
	}
	if (ok)
	{
		sourceCoord [0] = realX;
		sourceCoord [1] = realY;
		sourceCoord [2] = realZ;
	}
	return ok;
}
bool TcsOsGeoTestFile::GetTargetCoordinates (double targetCoord [3],unsigned recordNbr)
{
	bool ok;
	wchar_t* wcPtr;
	double realX (0.0);
	double realY (0.0);
	double realZ (0.0);
	std::wstring fieldData;

	ok = GetField (fieldData,recordNbr,9,Status);
	if (ok)
	{
		realX = wcstod (fieldData.c_str (),&wcPtr);
		ok = (fabs (realX) < HUGE_VAL) ;
	}
	if (ok)
	{
		ok = GetField (fieldData,recordNbr,10,Status);
		if (ok)
		{
			realY = wcstod (fieldData.c_str (),&wcPtr);
			ok = fabs (realY) < HUGE_VAL;
		}
	}
	if (ok)
	{
		ok = GetField (fieldData,recordNbr,11,Status);
		if (ok)
		{
			realZ = wcstod (fieldData.c_str (),&wcPtr);
			ok = fabs (realZ) < HUGE_VAL;
		}
	}
	if (ok)
	{
		targetCoord [0] = realX;
		targetCoord [1] = realY;
		targetCoord [2] = realZ;
	}
	return ok;
}
bool TcsOsGeoTestFile::GetTolerances (double tolerance [3],unsigned recordNbr)
{
	bool ok;
	wchar_t* wcPtr;
	double realX (0.0);
	double realY (0.0);
	double realZ (0.0);
	std::wstring fieldData;

	ok = GetField (fieldData,recordNbr,12,Status);
	if (ok)
	{
		realX = wcstod (fieldData.c_str (),&wcPtr);
		ok = (fabs (realX) < HUGE_VAL) ;
	}
	if (ok)
	{
		ok = GetField (fieldData,recordNbr,13,Status);
		if (ok)
		{
			realY = wcstod (fieldData.c_str (),&wcPtr);
			ok = fabs (realY) < HUGE_VAL;
		}
	}
	if (ok)
	{
		ok = GetField (fieldData,recordNbr,14,Status);
		if (ok)
		{
			realZ = wcstod (fieldData.c_str (),&wcPtr);
			ok = fabs (realZ) < HUGE_VAL;
		}
	}
	if (ok)
	{
		tolerance [0] = realX;
		tolerance [1] = realY;
		tolerance [2] = realZ;
	}
	return ok;
}
bool TcsOsGeoTestFile::GetDataSource (std::wstring& dataSource,unsigned recordNbr)
{
	bool ok = GetField (dataSource,recordNbr,13,Status);
	return ok;
}
bool TcsOsGeoTestFile::GetDataComment (std::wstring& dataComment,unsigned recordNbr)
{
	bool ok = GetField (dataComment,recordNbr,13,Status);
	return ok;
}
