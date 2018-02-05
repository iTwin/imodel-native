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

//=============================================================================
//            Regression  Test  Generation  and  Execution
//=============================================================================
//
// Basic Regression Test file format. Hopefully, should the format of the
// test file change in the future, twiddling the following will be all that is
// necessary to accommodate such changes.  Note, code expects the numeric values
// to be actual field numbers in the CSV file.  Note, that numerical values
// assigned to each identifier indicate the order of fields in the CSV record.
// That they are explicitly assigned a value and defined in numerical order is
// simply a matter of convenience for debugging and the maintainer.
enum EcsTestFldNbr {	fldNbrFirst    =  0,
						fldNbrPointId  =  0,
						fldNbrTestType =  1,
						fldNbrSrcAuth  =  2,
						fldNbrSrcCrs   =  3,
						fldNbrTrgAuth  =  4,
						fldNbrTrgCrs   =  5,
						fldNbrSrcOrd1  =  6,
						fldNbrSrcOrd2  =  7,
						fldNbrSrcOrd3  =  8,
						fldNbrTrgOrd1  =  9,
						fldNbrTrgOrd2  = 10,
						fldNbrTrgOrd3  = 11,
						fldNbrTolOrd1  = 12,
						fldNbrTolOrd2  = 13,
						fldNbrTolOrd3  = 14,
						fldNbrMinCnt   = 15,	// Minimum number of fields required for a valid record
						fldNbrUsing    = 15,
						fldNbrDataSrc  = 16,
						fldNbrComment  = 17,
						fldNbrLast     = 17,	// Signals last field in record
						fldNbrMaxCnt   = 18		// End of table, error, unassigned
				   };

// The following associates a field number with a CSV file label.
struct TcsOsGeoTestField
{
	EcsTestFldNbr FldNumber;
	wchar_t       FldLabel [64];
	unsigned long UserFlags;		// for future convenience
};

// EcsTestMethod enumerates the supported test methods.
enum EcsTestMethod {    testMthNone = 0,
						testMthCrs2D,
						testMthCrs3D,
						testMthGeoid,
						testMthUnknown = 999
					};
// EcsCrsAuthority enumerates the supported CRS ID authorities.
enum EcsCrsAuthority {  crsAuthNone = 0,
						crsAuthCsMap,
						crsAuthEpsg,
//                      crsAuthProj4,	// not supported, currently
						crsAuthSrid,
						crsAuthUnknown = 999
					};
enum EcsCrsDblFormat {  crsDblFrmtNone = 0,
						crsDblFrmtCart,
						crsDblFrmtGeo,
						crsDblFrmtTol,
						crsDblFrmtUnknown = 999
					 };

// The following structure is used to define specific test points for certain
// CRS systems where the automated test point generation does not work very
// well.
struct TcsOsGeoTestPoints
{
	char CrsName [cs_KEYNM_DEF];
	double Longitude;
	double Latitude;
	double Height;
};

//============================================================================
// TcsCrsRange  -- Essentially the useful range of a specific CRS.
//
// Used in generating regression tests:
//	1> test points for a specific CRS are generated using these values.
//	2> intersecting geographic CRS's are identified using this object.
//
// Note that SW implies a true, hard, southwest relationship to NE.  Thus,
// to cross the +/- 180 degrees crack, a SW longitude may be less than -180
// and a NE longitude may be greater than +180.
//
// Fuzz values are computed based on the supplied values and used as a proxy
// for zero to remove the effect of least significant bit noise which may
// result from calculations done with 'doubles".
//
class TcsCrsRange
{
	static const double Fuzzy;			// Base fuzz value.
public:
	TcsCrsRange (void);
	TcsCrsRange (double sw [2],double ne [2]);
	TcsCrsRange (double swX,double swY,double neX,double neY);
	TcsCrsRange (const TcsCrsRange& source);
	TcsCrsRange& operator= (const TcsCrsRange& rhs);
	~TcsCrsRange (void);

	void Reset (void);
	void SetSW (double sw [2]);
	void SetNE (double ne [2]);
	void SetRange (double sw [2],double ne [2]);
	bool IsNull () const;
	double GetSwX () const {return RangeSW [0];};
	double GetSwY () const {return RangeSW [1];};
	double GetNeX () const {return RangeNE [0];};
	double GetNeY () const {return RangeNE [1];};
	void GetSW (double sw [2]) const;
	void GetNE (double ne [2]) const;
	void Include (double xy [2]);
	void Include (double xx,double yy);
	void ExpandTo (const TcsCrsRange& range);
	void ShrinkTo (const TcsCrsRange& range);
	bool IsWithin (double xy [2],bool onIsOut = false) const;
	bool IsWithin (const double& xx,const double& yy,bool onIsOut = false) const;
	bool GenerateTestPoint (double sourcePoint [3],int variation) const;
private:
	double FuzzEW (void) const;
	double FuzzNS (void) const;
	double RangeSW [2];
	double RangeNE [2];
};
//newPage//
///////////////////////////////////////////////////////////////////////////////
// TcsTestCrs  --  Used to build a collection of CRS's to test
//
// Associates a useful range with the name of a CRS which is to be tested.
// Thus, enables the determination of intersecting CRS's and supports the
// generation of automatically generated test points.
//
// A collection (usually std::vector) of the following objects is used to
// generate geographic test cases.  The intent is to generate test cases for
// all geographic CRS's which overlap in coverage.
//
// EPSG and SRID codes are captured upon construction if available. Used to
// maintain connection to external data resources during test construction.
//
class TcsTestCrs
{
public:
	TcsTestCrs (void);
	TcsTestCrs (const char* crsName,bool isGeo,double oneMeter,
											   const TcsCrsRange& range);
	TcsTestCrs (const char* crsName);
	TcsTestCrs (const TcsTestCrs& source);
	TcsTestCrs& operator= (const TcsTestCrs& rhs);
	~TcsTestCrs (void);

	bool IsGeo (void) const {return Geographic; };
	unsigned long GetEpsgCode (void) const {return EpsgCode; };
	unsigned long GetSridCode (void) const {return SridCode; };
	double GetOneMeter (void) const {return OneMeter; };
	const TcsCrsRange& GetRange (void) const {return Range; };
	const char* GetCrsName (void) const {return CrsName; };

	void SetIsGeographic (bool isGeo) {Geographic = isGeo; };
	void SetEpsgCode (unsigned long epsgCode) {EpsgCode = epsgCode; };
	void SetSridCode (unsigned long sridCode) {SridCode = sridCode; };
	void SetOneMeter (double oneMeter) {OneMeter = oneMeter; };
	void SetRange (const TcsCrsRange& range) {Range = range; };
	void SetCrsName (const char* crsName);

	bool IsValid (void) const;

private:
	bool Geographic;
	unsigned long EpsgCode;
	unsigned long SridCode;
	double OneMeter;				// one meter in CRS units
	TcsCrsRange Range;
	char CrsName [cs_KEYNM_DEF];
};
//newPage//
///////////////////////////////////////////////////////////////////////////////
// TcsTestCase  -- Complete test case
//
// A "Test Case" is a complete test case as constructed from a specific
// version of the library in preparation for writing to a Regression Test
// data file. AMD/OR as extracted from a Regression Test data file.
//
class TcsTestCase
{
	// This prefix is used on all test cases written to a regression test file.
	// A test number is appended to given each point a unique ID. As this
	// is a part of each test, shorter is better than longer.  Including the
	// SVN revision number in this prefix is a useful convention.
	static char IdPrefix [32];

public:
	static inline void SetIdPrefix (const char* prefix)
	{
		CS_stncp (IdPrefix,prefix,sizeof (IdPrefix));
	}

	// This function calculates a tolerance suitable for a given test.
	// The argument is one meter in terms of the target CRS unit.
	// As of this revision (~2742), the tolerance produced is approximately
	// 2 millimeters.
	static double CalcTolerance (double oneMeter);

	TcsTestCase (void);
	TcsTestCase (EcsTestMethod testMthd,const TcsTestCrs& srcCrs,
										const TcsTestCrs& trgCrs,
										unsigned testNbr);
	// Inverting constructor, distinguished from the normal copy constructor
	// by the first argument. Used to generate inverse tests on projected
	// CRS test cases.
	TcsTestCase (unsigned testNbr,const TcsTestCase& fwdTestCase);
	TcsTestCase (const TcsCsvRecord& csvRecord);
	TcsTestCase (const TcsTestCase& source);
	TcsTestCase& operator= (const TcsTestCase& rhs);
	~TcsTestCase (void);

	EcsTestMethod GetTestMthd (void) const {return TestMthd; };
	bool IsThreeD (void) const {return ThreeD; };
	bool SrcIsGeo (void) const {return SourceCrs.IsGeo (); };
	bool TrgIsGeo (void) const {return TargetCrs.IsGeo (); };
	double SrcOneMeter (void) const {return SourceCrs.GetOneMeter (); };
	double TrgOneMeter (void) const {return TargetCrs.GetOneMeter (); };
	unsigned long GetSrcEpsg (void) const {return SourceCrs.GetEpsgCode (); };
	unsigned long GetSrcSrid (void) const {return SourceCrs.GetSridCode (); };
	unsigned long GetTrgEpsg (void) const {return TargetCrs.GetEpsgCode (); };
	unsigned long GetTrgSrid (void) const {return TargetCrs.GetSridCode (); };
	const char* GetSrcCrs (void) const {return SourceCrs.GetCrsName (); };
	const char* GetTrgCrs (void) const {return TargetCrs.GetCrsName (); };
	void GetTestPnt (double testPnt [3]) const;
	void GetResultPnt (double resultPnt [3]) const;
	void GetTolerance (double tolerance [3]) const;
	const TcsCrsRange& GetRange (void) const {return Range; };
	const char* GetPointID (void) const {return PointID.c_str (); };
	const char* GetUsing (void) const {return Using.c_str (); };
	const char* GetDataSource (void) const {return DataSource.c_str (); };
	const char* GetComment (void) const {return Comment.c_str (); };
	int GetStatus (void) const {return Status; };

	void SetTestMthd (EcsTestMethod testMthd) {TestMthd = testMthd; };
	void SetThreeD (bool threeD) { ThreeD = threeD; };
	void SetRange (const TcsCrsRange& crsRange) { Range = crsRange; };
	void SetPointID (const char* pointID) {PointID = pointID;};
	void SetSourceCrs (const TcsTestCrs& srcCrs) {SourceCrs = srcCrs; };
	void SetTargetCrs (const TcsTestCrs& trgCrs) {TargetCrs = trgCrs; };
	void SetTestPnt (const double testPnt [3]);
	void SetResultPnt (const double resultPnt [3]);
	void SetTolerance (const double tolerance [3]);
	void SetUsing (const char* usingStr) {Using = usingStr; };
	void SetDataSource (const char* dataSource) {DataSource = dataSource; };
	void SetComment (const char* comment) {Comment = comment; };

	bool Validate (void) const;
	int Finish (void);
	bool ExecuteTest (double& dd,int& status,double calcResult [3]) const;
	void TestInversion ();	// used to generate inverse tests.

	void ToCsvRecord (TcsCsvRecord& cvsRecord) const;

protected:
	void CnvrtDouble (std::wstring& result,double value,EcsCrsDblFormat frmt) const;

private:
	// Per Stroustrop, deferring smaller items to last to obtain max memory
	// efficiency.
	TcsTestCrs SourceCrs;
	TcsTestCrs TargetCrs;
	double TestPnt [3];
	double ResultPnt [3];
	double Tolerance [3];
	TcsCrsRange Range;
	std::string PointID;
	std::string Using;
	std::string DataSource;
	std::string Comment;
	int Status;					// Status of the conversion which produced the
								// ResultPnt data.
	EcsTestMethod TestMthd;
	bool ThreeD;
};
//newPage//
//==============================================================================
// TcsOsGeoTestFile -- Regression Test File
//
class TcsOsGeoTestFile : public TcsCsvFileBase
{
private:
	struct TcsTestTypeMap
	{
		EcsTestMethod TestType;
		wchar_t       TestName [32];
	};
	struct TcsAuthMap
	{
		EcsCrsAuthority Authority;
		wchar_t         AuthName [32];
	};
	static const TcsTestTypeMap KcsTestTypeMap [];
	static const TcsAuthMap KcsAuthMap [];

public:
	static const wchar_t* TestMthdName (EcsTestMethod testMthdId);
	static EcsTestMethod TestMthdId (const wchar_t* testMthdName);
	static const wchar_t* CrsAuthName (EcsCrsAuthority testAuthId);
	static EcsCrsAuthority TestAuthId (const wchar_t* testAuthName);
	static bool WriteLabels (std::wofstream& regressStrm);

	//==========================================================================
	// Construction, Destruction, and Assignment
	TcsOsGeoTestFile (const wchar_t* dataFilePath);
	TcsOsGeoTestFile (const char* dataFilePath);
	TcsOsGeoTestFile (const TcsOsGeoTestFile& source);
	~TcsOsGeoTestFile (void);
	TcsOsGeoTestFile& operator= (const TcsOsGeoTestFile& rhs);
	//==========================================================================
	// Operator Overrides
	//==========================================================================
	// Public Named Member Functions
	bool IsOk (void) const {return Ok; }
	const TcsCsvRecord& GetRec (unsigned recordNbr) const;
	bool GetTestName (std::wstring& testName,unsigned recordNbr);
	EcsTestMethod GetTestMethod (unsigned recordNbr);
	EcsCrsAuthority GetSrcCrsKey (std::wstring& srcKey,unsigned recordNbr);
	EcsCrsAuthority GetTrgCrsKey (std::wstring& trgKey,unsigned recordNbr);
	bool GetSourceCoordinates (double sourceCoord [3],unsigned recordNbr);
	bool GetTargetCoordinates (double targetCoord [3],unsigned recordNbr);
	bool GetTolerances (double tolerance [3],unsigned recordNbr);
	bool GetDataSource (std::wstring& dataSource,unsigned recordNbr);
	bool GetDataComment (std::wstring& dataComment,unsigned recordNbr);
private:
	EcsCrsAuthority GetCrsKey (unsigned recordNbr,bool target);

	bool Ok;
	TcsCsvStatus Status;
};

// TcsCrsList is a collection of CRS definitions which are to be tested. This
// object is used, for example, to build a list of all geographic coordinate
// system definitions.
typedef std::vector<TcsTestCrs> TcsCrsList;

// TcsTestList is a collection of actual tests.  That is, CRS systems which are
// to be tested, paired with a target system which defines a specific set of
// numerical results which can be generated and compared to calculations
// performed by some library in the future.
typedef std::vector<TcsTestCase> TcsTestList;

