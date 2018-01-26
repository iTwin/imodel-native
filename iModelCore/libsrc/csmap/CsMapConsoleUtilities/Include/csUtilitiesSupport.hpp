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

#include "csAscFixer.hpp"

// The results of an intersection calculation.  Code is written such that the
// values are independent of each other (i.e. each case can be any value, as
// long as different from anyother value).
enum EcsIntersectionType {	isectParallel = -1,
							isectNeither = 0,
							isectFirst,
							isectSecond,
							isectBoth
						 };

const wchar_t* dbl2wcs (double dblVal);
unsigned char CS_getParmCode (unsigned short projCode,unsigned parmNbr);	// first parm == 1
unsigned short CS_getPrjCode (const char* projKeyName);
bool CS_crsHasUsefulRng (const struct cs_Csdef_& csDef);
const TcsEpsgDataSetV6* GetEpsgObjectPtr (void);
void ReleaseEpsgObjectPtr (void);
bool CsvDelimiterConvert (const wchar_t* csDataDir,const wchar_t* inputFile,bool labels,
																			const wchar_t* fromDelims,
																			const wchar_t* toDelims,
																			const wchar_t* outputFile);

bool CS_strrpl (char* string1,size_t strSize,const char* find,const char* rplWith);
int CS_nmMprRplName (TcsCsvFileBase& csvFile,short fldNnr,const char* oldName,const char* newName,bool once);

EcsIntersectionType CS_intersection2D (const double firstFrm  [2],
									   const double firstTo   [2],
									   const double secondFrm [2],
									   const double secondTo  [2],
									   double intersection [2]);

void csWriteProjectionCsv (std::wostream& oStrm);
void csWriteParameterCsv (std::wostream& oStrm);
void csWriteLinearUnitCsv (std::wostream& oStrm);
void csWriteAngularUnitCsv (std::wostream& oStrm);

///////////////////////////////////////////////////////////////////////////////
// Below, we define a class for each of the file types.  We use these objects
// to open the file, check the magic number, and support the sequential
// access to the contents of the file.  Designed for high performance read
// only access; thus no write member function is provided.
//
//	Coordinate System Dictionary File 
//
class TcsCoordsysFile
{
	///////////////////////////////////////////////////////////////////////////
	// Disable copying/assignment
	TcsCoordsysFile (const TcsCoordsysFile&  source);			// not implemented
	TcsCoordsysFile& operator= (const TcsCoordsysFile& rhs);	// not implemented
public:
	static bool KeyNameLessThan (const cs_Csdef_& lhs,const cs_Csdef_& rhs);
	static bool ProjectionLessThan (const cs_Csdef_& lhs,const cs_Csdef_& rhs);
	///////////////////////////////////////////////////////////////////////////
	// Construction  /  Destruction  /  Assignment
	TcsCoordsysFile (void);
	virtual ~TcsCoordsysFile (void);
	///////////////////////////////////////////////////////////////////////////
	// Public Named Member Functions
	bool IsOk (void) const {return Ok; };
	size_t GetRecordCount (void) const {return CoordinateSystems.size (); }
	void Rewind (void) {CurrentIndex = 0; }
	const cs_Csdef_* FetchCoordinateSystem (size_t index) const;
	const cs_Csdef_* FetchCoordinateSystem (const char* keyName) const;
	const struct cs_Csdef_* FetchNextCs (void);			// returns 0 on EOF.
	void OrderByProjection (void);
	static int TcsCoordsysFile::ProjectionCompare (const cs_Csdef_& lhs,const cs_Csdef_& rhs);
private:
	///////////////////////////////////////////////////////////////////////////
	// Private Data Members
	bool Ok;
	bool SortedByName;
	size_t CurrentIndex;
	std::vector<struct cs_Csdef_> CoordinateSystems;
};
///////////////////////////////////////////////////////////////////////////////
//	Datum Dictionary File 
class TcsDatumsFile
{
	///////////////////////////////////////////////////////////////////////////
	// Disable copying/assignment
	TcsDatumsFile (const TcsDatumsFile& source);			// not implemented
	TcsDatumsFile& operator= (const TcsDatumsFile& rhs);	// not implemented
public:
	static bool KeyNameLessThan (const cs_Dtdef_& lhs,const cs_Dtdef_& rhs);
	///////////////////////////////////////////////////////////////////////////
	// Construction  /  Destruction  /  Assignment
	TcsDatumsFile (void);
	~TcsDatumsFile (void);								// closes the file
	///////////////////////////////////////////////////////////////////////////
	// Public Named Member Functions
	bool IsOk (void) const {return Ok; };
	size_t GetRecordCount (void) const {return Datums.size (); }
	void Rewind (void) {CurrentIndex = 0; }
	const cs_Dtdef_* FetchDatum (size_t index) const;
	const cs_Dtdef_* FetchDatum (const char* keyName) const;
	const struct cs_Dtdef_* FetchNextDt (void);
private:
	///////////////////////////////////////////////////////////////////////////
	// Private Data Members
	bool Ok;
	size_t CurrentIndex;
	std::vector<cs_Dtdef_> Datums;
};
///////////////////////////////////////////////////////////////////////////////
//	Elipsoid Dictionary File 
class TcsElipsoidFile
{
	///////////////////////////////////////////////////////////////////////////
	// Disable copying/assignment
	TcsElipsoidFile (const TcsElipsoidFile&  source);		// not implemented
	TcsElipsoidFile& operator= (const TcsElipsoidFile& rhs);	// not implemented
public:
	static bool KeyNameLessThan (const cs_Eldef_& lhs,const cs_Eldef_& rhs);
	///////////////////////////////////////////////////////////////////////////
	// Construction  /  Destruction  /  Assignment
	TcsElipsoidFile (void);
	~TcsElipsoidFile (void);								// closes the file
	///////////////////////////////////////////////////////////////////////////
	// Public Named Member Functions
	bool IsOk (void) const {return Ok; };
	size_t GetRecordCount (void) const {return Ellipsoids.size (); }
	void Rewind (void) {CurrentIndex = 0; }
	const cs_Eldef_* FetchEllipsoid (size_t index) const;
	const cs_Eldef_* FetchEllipsoid (const char* keyName) const;
	const struct cs_Eldef_* FetchNextEl (void);
private:
	///////////////////////////////////////////////////////////////////////////
	// Private Data Members
	bool Ok;
	size_t CurrentIndex;
	std::vector<cs_Eldef_> Ellipsoids;
};
///////////////////////////////////////////////////////////////////////////////
//	Category Dictionary File
class TcsCategoryItem
{
public:
	TcsCategoryItem (void);
	TcsCategoryItem (std::istream& inStrm);
	TcsCategoryItem (const char* itemName,const char* description = 0);
	TcsCategoryItem (const TcsCategoryItem& source);
	~TcsCategoryItem (void);
	TcsCategoryItem& operator= (const TcsCategoryItem& rhs);
	
	bool IsOk (void) const {return ItemName [0] != '\0'; }
	bool IsToBeDeleted (void) const {return ToBeDeleted; }
	const char* GetItemName (void) const {return ItemName; }
	const char* GetDescription (void) const {return Description; }
	void SetToBeDeleted (bool toBeDeleted = true); 
	void SetItemName (const char* itemName);
	void SetDescription (const char* description);
	bool ReadFromStream (std::istream& inStrm);
	bool WriteToStream (std::ostream& outStrm);
private:
	bool ToBeDeleted;
	char ItemName [32];
	char Description [128];
};
class TcsCategory
{
public:
	TcsCategory (void);
	TcsCategory (const char* categoryName);
	TcsCategory (const TcsCategory& source);
	TcsCategory (std::istream& inStream);
	~TcsCategory (void);
	TcsCategory& operator= (const TcsCategory& rhs);
	size_t GetItemCount (void) const;
	char* GetCategoryName (void);
	const char* GetCategoryName (void) const;
	const TcsCategoryItem* GetItem (size_t index) const;
	TcsCategoryItem* GetItem (size_t index);
	const TcsCategoryItem* GetItem (const char* itemName) const;
	TcsCategoryItem* GetItem (const char* itemName);
	void SetCategoryName (const char* categoryName);
	void AddItem (const TcsCategoryItem& newItem);
	void RemoveItem (const TcsCategoryItem& oldItem);
	bool ReadFromStream (std::istream& inStrm);
	bool WriteToStream (std::ostream& outStrm);
private:
	char CategoryName [128];
	std::vector<TcsCategoryItem> Items;
};
class TcsCategoryFile
{
	///////////////////////////////////////////////////////////////////////////
	// Disable copying/assignment
	TcsCategoryFile (const TcsCategoryFile&  source);		// not implemented
	TcsCategoryFile& operator= (const TcsCategoryFile& rhs);	// not implemented
public:
	static const char KcsObsoleteCatName [128];
	///////////////////////////////////////////////////////////////////////////
	// Construction  /  Destruction  /  Assignment
	TcsCategoryFile (void);
	TcsCategoryFile (std::istream& inStrm);
	~TcsCategoryFile (void);								// closes the file
	///////////////////////////////////////////////////////////////////////////
	// Public Named Member Functions
	size_t GetCategoryCount (void) const {return Categories.size (); }
	const TcsCategory* FetchCategory (size_t index) const;
	TcsCategory* FetchCategory (size_t index);
	const TcsCategory* FetchCategory (const char* categoryName) const;
	TcsCategory* FetchCategory (const char* categoryName);
	void AddCategory (const TcsCategory& newCategory);
	bool ReadFromStream (std::istream& inStrm);
	bool InitializeFromFile (const char* categoryPathName);
	bool DeprecateCrs (const char* oldCrsName,const char* newCrsName,
											  const char* deprNote,
											  const char* newCrsDescription = 0);
	bool DeprecateCrsDel (const char* oldCrsName,const char* useCrs,const char* deprNote);
	bool WriteToStream (std::ostream& outStrm);
	bool WriteToFile (const char* categoryPathName);
private:
	///////////////////////////////////////////////////////////////////////////
	// Private Data Members
	std::string CopyrightNotice;
	std::vector<TcsCategory> Categories;
};
///////////////////////////////////////////////////////////////////////////////
//	Multiple Regression Definition File 
class TcsMrtFile
{
	///////////////////////////////////////////////////////////////////////////
	// Disable copying/assignment
	TcsMrtFile (const TcsMrtFile&  source);		// not implemented
	TcsMrtFile& operator= (const TcsMrtFile& rhs);// not implemented
public:
	///////////////////////////////////////////////////////////////////////////
	// Construction  /  Destruction  /  Assignment
	TcsMrtFile (const char* filePath);
	~TcsMrtFile (void);								// closes the file
	///////////////////////////////////////////////////////////////////////////
	// Public Named Member Functions
	bool IsOk (void) const {return Ok; };
	double GetNormalizingScale (void) const {return MregDefinition.kk; };
	double GetLambdaOffset (void) const {return MregDefinition.vv_off; };
	double GetPhiOffset (void) const {return MregDefinition.uu_off; };
	double GetLambdaCoeff (size_t uuPwr,size_t vvPwr) const;
	double GetPhiCoeff (size_t uuPwr,size_t vvPwr) const;
	double GetHgtCoeff (size_t uuPwr,size_t vvPwr) const;
private:
	///////////////////////////////////////////////////////////////////////////
	// Private Data Members
	bool Ok;
	struct csDmaMReg_ MregDefinition;
};
///////////////////////////////////////////////////////////////////////////////
//	Geodetic Data Catalog File
//
// A Geodetic Data Catalog file is a collection of entries.  We define the
// entry object first.
//
class TcsGdcEntry
{
public:
	TcsGdcEntry (void);
	TcsGdcEntry (const char* path,short relative,long bufferSize = 0,unsigned long flags = 0,double density = 0.0);
	TcsGdcEntry (const TcsGdcEntry& source);
	~TcsGdcEntry (void);
	TcsGdcEntry& operator= (const TcsGdcEntry& rhs);
	///////////////////////////////////////////////////////////////////////////
	// Public Named Member Functions
	const char* GetEntryPath (void) const {return PathName; };
	double GetDensity (void) const {return Density; };
	long GetBufferSize (void) const {return BufferSize; };
	unsigned long GetFlags (void) const {return Flags; };
	short GetRelative (void) const {return Relative; };
private:
	char PathName [520];
	double Density;
	long BufferSize;
	unsigned long Flags;
	short Relative;	
};
//
class TcsGdcFile
{
	///////////////////////////////////////////////////////////////////////////
	// Disable copying/assignment
public:
	///////////////////////////////////////////////////////////////////////////
	// Construction  /  Destruction  /  Assignment
	TcsGdcFile (const char* filePath);
	TcsGdcFile (const TcsGdcFile&  source);					// not implemented
	~TcsGdcFile (void);
	TcsGdcFile& operator= (const TcsGdcFile& rhs);			// not implemented
	///////////////////////////////////////////////////////////////////////////
	// Public Named Member Functions
	bool IsOk (void) const {return Ok; };
	size_t GetEntryCount (void) const {return Entries.size (); };
	const TcsGdcEntry* GetEntryPtr (size_t index) const;
	bool ConvertToRelative (char* entryPath) const;
	const char* GetCatalogDirectory (void) const {return FileFolder; };
	const char* GetFallbackDatum (void) const {return Fallback; };
	const char* GetInitialComment (void) const {return InitialComment.c_str(); };
	const char* GetMiddleComment (void) const {return MiddleComment.c_str(); };
	const char* GetTrailingComment (void) const {return TrailingComment.c_str(); };
private:
	///////////////////////////////////////////////////////////////////////////
	// Private Data Members
	bool Ok;
	char FileFolder [520];
	char Fallback [cs_KEYNM_DEF];
	std::string InitialComment;
	std::string MiddleComment;
	std::string TrailingComment;
	std::vector<TcsGdcEntry> Entries;
};
//newPage//
enum EcsKeyNmType {	csKyNmTypNone =   0,
					csKyNmTypeCRS =   1,
					csKyNmTypeDtm =   2,
					csKyNmTypeElp =   3,
					csKyNmTypeUnk = 999,
				  };
class TcsKeyName
{
public:
	static bool QualitySort (const TcsKeyName& lhs,const TcsKeyName& rhs);
	TcsKeyName (EcsKeyNmType type);
	TcsKeyName (EcsKeyNmType type,const char* keyName,unsigned long epsgCode = 0UL);
	TcsKeyName (EcsKeyNmType type,const wchar_t* keyName,unsigned long epsgCode = 0UL);
	TcsKeyName (const TcsKeyName& source);
	~TcsKeyName (void);
	TcsKeyName& operator= (const TcsKeyName& rhs);
	bool operator< (const TcsKeyName& rhs) const;
	bool operator== (const TcsKeyName& rhs);
	operator const char* (void);
	operator const char* (void) const;
	operator unsigned long (void);
	operator unsigned long (void) const;
	EcsKeyNmType GetKeyNameType ();
	const char* GetKeyName (void);
	unsigned long GetEpsgCode (void);
	unsigned GetQuality ();
	bool SetQuality (const cs_Csdef_ *csDefPtr);
private:
	EcsKeyNmType Type;
	unsigned Quality;
	unsigned long EpsgCode;
	char KeyName [32];
};
std::wostream& operator<< (std::wostream& outStrm,const TcsKeyName& keyName);
class TcsKeyNameList
{
public:
	TcsKeyNameList (void);
	TcsKeyNameList (const TcsKeyName& primary);
	TcsKeyNameList (const TcsKeyName& primary,const TcsKeyName& secondary);
	TcsKeyNameList (const TcsKeyNameList& source);
	~TcsKeyNameList (void);
	TcsKeyNameList& operator+= (const TcsKeyName& rhs);
	TcsKeyNameList& operator= (const TcsKeyNameList& rhs);
	bool operator< (const TcsKeyNameList& rhs);
	bool operator== (const TcsKeyNameList& rhs);
	TcsKeyName* GetFirst (void);
	TcsKeyName* GetNext (void);
	void Rewind (void);
	void Arrange (void);
	void WriteToStream (std::wostream& listStream);
private:
	std::list<TcsKeyName>::iterator currentPos;
	std::list<TcsKeyName> KeyNameList;
};
std::wostream& operator<< (std::wostream& outStrm,const TcsKeyNameList& keyNameList);
//newPage//
// The following object is useful for maintaining the Name Mapper Source
// file in its version controlled .CSV file format.  This is slowly being
// replaced by the TcsNameMapperVector object below, which supports maintaining
// the Name Mapper Source data file in the form of an internal
// std::vector<TcsNameMap> form.
class TcsNameMapperSource : public TcsCsvFileBase
{
	static const short IdentFld  = 0;
	static const short TypeFld   = 1;
	static const short FlvrFld   = 2;
	static const short FlvrIdFld = 3;
	static const short NameFld   = 4;
	static const short DupSrtFld = 5;
	static const short AliasFld  = 6;
	static const short FlagsFld  = 7;
	static const short DeprctFld = 8;
	static const short RemrksFld = 8;
	static const short CommntFld = 8;

	static const short FlavorCount = 32;
	static const short FlavorSize = 32;
public:
	static const unsigned long UlErrorValue = 1999999999UL;
	//=========================================================================
	// Construction, Destruction, and Assignment
	TcsNameMapperSource (void);
	TcsNameMapperSource (const TcsNameMapperSource& source);
	virtual ~TcsNameMapperSource (void);
	TcsNameMapperSource& operator= (const TcsNameMapperSource& rhs);
	//=========================================================================
	// Operator Overrides
	//=========================================================================
	// Public Named Member Functions
	bool IsOk (void) const {return Ok; }
	bool ReadFromFile (const char* csvSourceFile);
	unsigned long GetIdent (unsigned recNbr) const;
	unsigned long GetType (unsigned recNbr) const;
	EcsNameFlavor GetFlavor (unsigned recNbr) const;
	unsigned long GetFlavorIdent (unsigned recNbr) const;
	const wchar_t* GetName (unsigned recNbr) const;
	unsigned long GetDupSort (unsigned recNbr) const;
	unsigned long GetAliasFlag (unsigned recNbr) const;
	unsigned long GetFlags (unsigned recNbr) const;
	unsigned long GetDeprecation (unsigned recNbr) const;
	const wchar_t* GetRemarks (unsigned recNbr) const;
	const wchar_t* GetComment (unsigned recNbr) const;
	bool SetIdent (unsigned recNbr,unsigned long newIdent);
	bool SetType (unsigned recNbr,unsigned long newType);
	bool SetFlavor (unsigned recNbr,EcsNameFlavor newFlavor);
	bool SetFlavorIdent (unsigned recNbr,unsigned long newFlavorIdent);
	bool SetName (unsigned recNbr,const wchar_t* newName);
	bool SetDupSort (unsigned recNbr,unsigned long newDupSort);
	bool SetAliasFlag (unsigned recNbr,unsigned long newAliasFlag);
	bool SetFlags (unsigned recNbr,unsigned long newFlags);
	bool SetDeprecation (unsigned recNbr,unsigned long newDeprecation);
	bool SetRemarks (unsigned recNbr,const wchar_t* newRemarks);
	bool SetComment (unsigned recNbr,const wchar_t* newComment);
	bool Locate (unsigned& recNbr,unsigned long type,EcsNameFlavor flavor,const wchar_t* name) const;
	bool Locate (unsigned& recNbr,unsigned long type,EcsNameFlavor flavor,const char* name);
	bool Locate (unsigned& recNbr,unsigned long type,EcsNameFlavor flavor,unsigned long flvrId);
	bool GetNameMap (TcsNameMap& result,unsigned recNbr);
	bool LocateNameMap (unsigned& recNbr,TcsNameMap& locator,bool byId = false);
	bool ReplaceNameMap (unsigned recNbr,TcsNameMap& updatedNameMap);
	bool AddNameMap (TcsNameMap& newNameMap);
	bool DeleteNameMap (unsigned recNbr);
	bool RenameObject (EcsMapObjType nameSpace,EcsNameFlavor flavor,const char* currentName,
																	const char* newName);
	bool WriteToFile (const char* csvSourceFile,bool overwrite = true);
protected:
	//=========================================================================
	// Protected Named Member Functions
	unsigned long GetFieldAsUlong (unsigned recNbr,short fldNbr) const;
	const wchar_t* GetFieldAsString (unsigned recNbr,short fldNbr) const;
	bool SetUlongField (unsigned recNbr,short fldNbr,unsigned long newValue);
	bool SetStringField (unsigned recNbr,short fldNbr,const wchar_t* newString);
	//=========================================================================
	// Protected Data Members
private:
	//=========================================================================
	// Private Member Functions
	bool InitializeFlavors ();
	bool GetFieldAsUlong (unsigned long& rtnValue,unsigned recordNbr,short fieldNbr);
	//=========================================================================
	// Private Data Members
	bool Ok;
	wchar_t FlavorNames [FlavorCount][FlavorSize];
	unsigned long FlavorIdValues [FlavorCount];
	wchar_t NameBuffer [256];
};
class TcsNameMapperVector 
{
	static const short FlavorCount = 32;
	static const short FlavorSize = 32;
public:
	//=========================================================================
	// Construction, Destruction, and Assignment
	TcsNameMapperVector (void);
	TcsNameMapperVector (const TcsNameMapperVector& source);
	virtual ~TcsNameMapperVector (void);
	TcsNameMapperVector& operator= (const TcsNameMapperVector& rhs);
	//=========================================================================
	// Operator Overrides
	//=========================================================================
	// Public Named Member Functions
	bool IsOk (void) const {return Ok; }
	bool ReadFromFile (const char* csvSourceFile);

	const TcsNameMap& Locate (unsigned long type,EcsNameFlavor flavor,const wchar_t* name) const;
	TcsNameMap& Locate (unsigned long type,EcsNameFlavor flavor,const wchar_t* name);
	const TcsNameMap& Locate (unsigned long type,EcsNameFlavor flavor,const char* name) const;
	TcsNameMap& Locate (unsigned long type,EcsNameFlavor flavor,const char* name);
	const TcsNameMap& Locate (unsigned long type,EcsNameFlavor flavor,unsigned long flvrId) const;
	TcsNameMap& Locate (unsigned long type,EcsNameFlavor flavor,unsigned long flvrId);
	bool AddNameMap (TcsNameMap& newNameMap);
	bool DeleteNameMap (TcsNameMap& nameMap);
	bool RenameObject (EcsMapObjType type,EcsNameFlavor flavor,const char* currentName,const char* newName);
	bool RenameObject (EcsMapObjType type,EcsNameFlavor flavor,const wchar_t* currentName,const wchar_t* newName);
	bool Deprecate (TcsNameMap& deprecatedNameMap,const TcsGenericId& deprecatedBy);
	bool ResortVector (void);
	bool WriteToFile (const char* csvSourceFile,bool overwrite = true);
protected:
	//=========================================================================
	// Protected Named Member Functions
	//=========================================================================
	// Protected Data Members
private:
	//=========================================================================
	// Private Member Functions
	bool InitializeFlavors ();
	//=========================================================================
	// Private Data Members
	bool Ok;
	wchar_t FlavorNames [FlavorCount][FlavorSize];
	unsigned long FlavorIdValues [FlavorCount];
	std::vector<TcsNameMap> NameMapVector;
};
//newPage//
// The following object is used when creating files of the NadCon .las and .los
// format.  Note that this same format is also used for such things as certain
// geiod height and vertical datum grid data files.  This object was devised
// expressly for the project of constructing a single set of .las/.los grid
// files which would function as 48hpgn.l?s and provide for a single definitive
// means of converting from NAD83 to HARN, avoiding the problem of which file
// to use in regions of overlaping coverage.
//
// Note, that there has been for a decade (or more) a struct cs_NadconFile_, so
// the name of this object needs to be different as we choose not to mess with
// struct cs_NadconFile_ which has been working fine for a long long time.
class TcsLasLosFile
{
public:
	// NoValue is used to mark cells for which no value has been determined yet.
	// NoValueTest is used to avoid using == on real (e.g. float, double) values.
	static const float NoValue;
	static const float NoValueTest;
	// EdgeValue is used to mark cells which have not had a value determined by
	//		the main algorithm and determined to be outside the basic coverage
	//		area of the aggregate HPGN grid.
	// EdgeValueTest is used to avoid using == on real (e.g. float, double) values.
	static const float EdgeValue;
	static const float EdgeValueTest;

	// Construction  /  Destruction  /  Assignment
	TcsLasLosFile (void);
	TcsLasLosFile (double swLng,double swLat,double cellSize,long32_t recCount,long32_t eleCount);
	TcsLasLosFile (const TcsLasLosFile& source);
	TcsLasLosFile& operator= (const TcsLasLosFile& rhs);
	virtual ~TcsLasLosFile (void);
	// Operator Overloads
	// Named Member Functions (Methods)
	bool ReadFromFile (const char* filePath);
	bool ReadFromFile (const wchar_t* filePath);
	double GetSwLongitude (void) const {return SwLongitude; }
	double GetDeltaLongitude (void) const {return DeltaLongitude; }
	double GetSwLatitude (void) const {return SwLatitude; }
	double GetDeltaLatitude (void) const {return DeltaLatitude; }
	long32_t GetRecordCount (void) const {return RecCount; }
	long32_t GetElementCount (void) const {return EleCount; }
	bool GetGridValue (float& result,long32_t recIdx,long32_t eleIdx) const;
	bool GetGridValue (double& result,long32_t recIdx,long32_t eleIdx) const;
	bool GetGridValue (double &result,double longitude,double latitude) const;
	bool GetGridLocation (double lngLat [2],long32_t recIdx,long32_t eleIdx) const;
	const char* GetFileIdent (void) const {return FileIdent; }
	const char* GetProgram (void) const {return Program; }
	long32_t GetZeeCount (void) const {return ZeeCount; }
	double GetAngle (void) const {return Angle; }

	bool SetSwLongitude (double swLongitude) {SwLongitude = swLongitude; return true; }
	bool SetSwLatitude (double swLatitude) {SwLatitude = swLatitude; return true; }
	bool SetDeltaLongitude (double deltaLongitude) {DeltaLongitude = deltaLongitude; return true; }
	bool SetDeltaLatitude (double deltaLatitude) {DeltaLatitude = deltaLatitude; return true; }
	bool SetRecordCount (long32_t recCount) {RecCount = recCount; return true; }
	bool SetElementCount (long32_t eleCount) {EleCount = eleCount; return true; }
	bool SetGridValue (long32_t recIdx,long32_t eleIdx,float gridValue);
	bool SetGridValue (long32_t recIdx,long32_t eleIdx,double gridValue);
	bool SetGridValue (double longitude,double latitude,float gridValue);
	bool SetGridValue (double longitude,double latitude,double gridValue);
	void SetFileIdent (const char* fileIdent);
	void SetProgram (const char* program);
	void SetZeeCount (long32_t zeeCount) {ZeeCount = zeeCount; }
	void SetAngle (double angle) {Angle = angle; }

	bool IsCovered (double longitude,double latitude) const;
	bool IsCovered (double lngLat [2]) const;

	bool EdgeFillDelta (double delta);
	bool EdgeFill (double fillValue);
	long32_t HoleCheck (std::wostream& rptStream,bool verbose = false);

	bool WriteToFile (const char* filePath) const;
	bool WriteToFile (const wchar_t* filePath) const;
	bool WriteGridToCsv (const char* filePath) const;
private:
	long32_t EleCount;		/* Number of data elements in each record. NADCON
							   calls it a column count.  This value does NOT
							   include the record number element which is the
							   first item in each record.  It is used to
							   calculate record size and the longitude extent
							   of the rectangular region of coverage. */
	long32_t RecCount;		/* The number of records in the file. Does NOT
							   include the header record. Notice, that the
							   header record is the same length as the others,
							   but only the first 'n' bytes are actually used,
							   where n is the number of bytes in a NADCON
							   header. This value also defines the latitude
							   extent of the rectangular region of coverage. */
	double SwLongitude;		/* The longitude associated with the first data
							   record in the file. In degrees, east longitude
							   is positive. */
	double SwLatitude;		/* The latitude associated with the first data
							   element in each record. In degrees, north
							   latitude is positive. */
	double DeltaLongitude;	/* The size, in degrees of longitude,
							   of the individual grid cells. */
	double DeltaLatitude;	/* The size, in degrees of latitude,
							   of the individual grid cells. */
	float *DataImage;		/* This is the data image of the file. This image
							   Does NOT include the record numbers which NADCON
							   expects on each record. View this as an array of
							   floats:
									array [RecCount][EleCount];    */
	// The following elements are read from an existing file and preserved
	// for subsequent writing.  These values can also be set and retrieved
	// for duplicating a file and/or reproducing a file. These values are
	// not used for anything, so they are isloated in this manner.
	char FileIdent [56];
	char Program [8];
	long32_t ZeeCount;
	double Angle;
	char FilePath [MAXPATH];
};
//newPage//
// A fence element is simply a point within a fence. A TcsFence is a collection
// of TcsFenceElements along with a predetermined bounding box (min/max).. We
// could use the term polygon, but a fence is a special type of polygon. I'm
// sure there is some fancy technical name for this, but a fence is a polygon
// which does not cross itself, and does not contain any inner polygons (i.e.
// holes).
class TcsFenceElement
{
public:
	TcsFenceElement (void);
	TcsFenceElement (double xx,double yy);
	TcsFenceElement (const TcsFenceElement& source);
	TcsFenceElement operator= (const TcsFenceElement& rhs);
	~TcsFenceElement (void);

	double GetX (void) const {return Xx; };
	double GetY (void) const {return Yy; };
	void GetXY (double xy [2]) const {xy[0] = Xx; xy[1] = Yy; };

	void SetX (double xx) {Xx = xx; };
	void SetY (double yy) {Yy = yy; };
	void SetXY (double xx,double yy) {Xx = xx; Yy = yy; };
	void SetXY (double xy [2]) {Xx = xy [0]; Yy = xy [1]; };
private:
	double Xx;
	double Yy;
};
class TcsFence
{
public:
	TcsFence (void);
	TcsFence (const char* epsgXmlPolygonFile);
	TcsFence (const TcsFence& source);
	TcsFence operator= (const TcsFence& rhs);
	~TcsFence (void);

	TcsFence& operator+ (const TcsFenceElement& newElement);
	void operator+= (const TcsFenceElement& newElement);

	bool IsOk (void) const {return Ok; };
	bool Add (const TcsFenceElement& newElement);
	bool Close (void);

	bool IsInside (double xx,double yy) const;
	bool IsInside (double xy [2]) const;
protected:
	bool ProcessEpsgXml (const char* epsgXmlPolygonFile);
private:
	bool SetUpCalc (void);
	bool IsClosed (void);
	bool Ok;
	double Fuzz;
	double RemovedPoint [2];
	double LowerLeft [2];		// minimums
	double UpperRight [2];		// maximums
	std::vector<TcsFenceElement> Elements;
};
