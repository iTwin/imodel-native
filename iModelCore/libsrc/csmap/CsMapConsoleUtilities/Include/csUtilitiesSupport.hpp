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

const wchar_t* dbl2wcs (double dblVal);

const TcsEpsgDataSetV6* GetEpsgObjectPtr (void);
void ReleaseEpsgObjectPtr (void);

bool CS_strrpl (char* string1,size_t strSize,const char* find,const char* rplWith);
int CS_nmMprRplName (TcsCsvFileBase& csvFile,short fldNnr,const char* oldName,const char* newName,bool once);

void csWriteProjectionCsv (std::wostream& oStrm);
void csWriteParameterCsv (std::wostream& oStrm);
void csWriteLinearUnitCsv (std::wostream& oStrm);
void csWriteAngularUnitCsv (std::wostream& oStrm);

///////////////////////////////////////////////////////////////////////////////
// Below, we define a class for each of the file types.  We use these objects
// to open the file, check the magic number, and support the sequential
// access to the contents of the file.  Of course, at some time in the
// future, we could easily add random access to the records.  All of these
// objects derive from TcsFile are read only, thus they are reasonably simple.
// File opens on construction, and remains open until destruction.  An
// exception is thrown if the file open fails, the magic number is wrong, or
// a read error occurs.
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
	///////////////////////////////////////////////////////////////////////////
	// Construction  /  Destruction  /  Assignment
	TcsCoordsysFile (void);
	virtual ~TcsCoordsysFile (void);							// closes the file
	///////////////////////////////////////////////////////////////////////////
	// Public Named Member Functions
	bool IsOk (void) const {return Ok; };
	size_t GetRecordCount (void) const {return CoordinateSystems.size (); }
	void Rewind (void) {CurrentIndex = 0; }
	const cs_Csdef_* FetchCoordinateSystem (size_t index) const;
	const cs_Csdef_* FetchCoordinateSystem (const char* keyName) const;
	const struct cs_Csdef_* FetchNextCs (void);			// returns 0 on EOF.
private:
	///////////////////////////////////////////////////////////////////////////
	// Private Data Members
	bool Ok;
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
	bool TcsCategoryFile::DeprecateCrs (const char* oldCrsName,const char* newCrsName,
															   const char* deprNote,
															   const char* newCrsDescription = 0);
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
class TcsNameMapperSource : public TcsCsvFileBase
{
	static const short IdntFld = 0;
	static const short TypeFld = 1;
	static const short FlvrFld = 2;
	static const short NameFld = 4;
public:
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
	bool RenameObject (EcsMapObjType nameSpace,EcsNameFlavor flavor,const char* currentName,
																	const char* newName);
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
	bool GetFieldAsUlong (unsigned long& rtnValue,unsigned recordNbr,short fieldNbr);
	//=========================================================================
	// Private Data Members
	bool Ok;
	wchar_t FlavorNames [32][32];
	unsigned long FlavorIdValues [32];
};
