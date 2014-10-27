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
		// A normal number.  COnvert at high precision, but trim trailing
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

// Returns a pointer to a TcsEpsgDataSetV6 object.  Specifically, a
// TcsEpsgDataSetV6 object initialized with the contents of the the folder
// pointed to by the csEpsgDir global variable defined in the
// csConsoleUtilities main module.  The prupose of this module is to
// elimiate the possibility of having two of these floating around as
// it is quite time consuming to buyild one of these things.

// The following should be const, by the dumb Micros??t linker can't find it if it is.
extern wchar_t csEpsgDir [];
TcsEpsgDataSetV6* csEpsgDataSetV6Ptr = 0;

const TcsEpsgDataSetV6* GetEpsgObjectPtr ()
{
	if (csEpsgDataSetV6Ptr == 0)
	{
		csEpsgDataSetV6Ptr = new TcsEpsgDataSetV6 (csEpsgDir,L"7.05");
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
///////////////////////////////////////////////////////////////////////////////
// TcsCoordsysFile Object  --  Abstracts the CS-MAP Coordsys file.
//
// This object is used to access CS-MAP Coordsys files.
//
bool TcsCoordsysFile::KeyNameLessThan (const cs_Csdef_& lhs,const cs_Csdef_& rhs)
{
	int cmpValue = CS_stricmp (lhs.key_nm,rhs.key_nm);
	return (cmpValue < 0);
}
TcsCoordsysFile::TcsCoordsysFile () : Ok                (false),
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
	
	CS_stncp (locateValue.key_nm,keyName,sizeof (locateValue.key_nm));
	locItr = std::lower_bound (CoordinateSystems.begin (),CoordinateSystems.end (),locateValue,KeyNameLessThan);
	if (locItr != CoordinateSystems.end ())
	{
		csDefPtr = &(*locItr);
		if (CS_stricmp (csDefPtr->key_nm,keyName))
		{
			csDefPtr = 0;
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
			itemPtr->SetItemName (newCrsName);
			if (newCrsDescription != 0 && *newCrsDescription != '\0')
			{
				itemPtr->SetDescription (newCrsDescription);
			}
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
}
TcsNameMapperSource::TcsNameMapperSource (const TcsNameMapperSource& source)
											:
										  TcsCsvFileBase (source)
{
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
bool TcsNameMapperSource::RenameObject (EcsMapObjType nameSpace,EcsNameFlavor flavor,
																const char* currentName,
																const char* newName)
{
	bool ok;

	unsigned long recordIdx;
	const wchar_t* flvrPtr;

	wchar_t wOldName [256];
	wchar_t wNewName [256];

	std::wstring field;
	std::wstring flvrName;
	std::wstring objtName;

	TcsCsvStatus status;

	ok = true;
	mbstowcs (wOldName,currentName,wcCount (wOldName));
	flvrPtr = FlavorNames [static_cast<int>(flavor)];
	for (recordIdx = 0;ok && recordIdx < RecordCount ();recordIdx += 1)
	{
		ok = GetField (flvrName,recordIdx,FlvrFld,status);
		if (ok && wcsicmp (flvrName.c_str(),flvrPtr))
		{
			continue;
		}
		// Flavor matches, see if the old name matches.
		if (ok)
		{
			ok = GetField (objtName,recordIdx,NameFld,status);
			if (ok && wcsicmp (flvrName.c_str(),wOldName))
			{
				// We have found it, make the change.
				mbstowcs (wNewName,newName,wcCount (wNewName));
				field = wNewName;
				ok = ReplaceField (field,recordIdx,NameFld,status);
				break;
			}
		}		
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
		woStrm.open (csvSourceFile,std::ios_base::out | std::ios_base::trunc);
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
	
	std::wstring field;
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
		ok = GetFieldAsUlong (typeValue,index,TypeFld);
		if (ok)
		{
			flvrIdx = 0UL;				// to keep lint happy
			if (typeValue == 1UL)
			{
				ok = GetField (field,index,NameFld,status);
				if (ok)
				{
					ok = GetFieldAsUlong (flvrIdx,index,FlvrFld);
				}
				if (ok)
				{
					wcsncpy (FlavorNames [flvrIdx],field.c_str (),32);
					FlavorNames [flvrIdx][31] = L'\0';
				}
			}
			else if (typeValue > 1)
			{
				idntValue = 0UL;			// to keep lint happy
				ok = GetField (field,index,FlvrFld,status);
				if (ok)
				{
					ok = GetFieldAsUlong (idntValue,index,IdntFld);
				}
				if(ok)
				{
					for (flvrIdx = 0;flvrIdx < 32;flvrIdx += 1)
					{
						if (!wcsicmp (FlavorNames [flvrIdx],field.c_str ()))
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
	}
	return ok;
}
bool TcsNameMapperSource::GetFieldAsUlong (unsigned long& rtnValue,unsigned recordNbr,
																   short fieldNbr)
{
	bool ok (false);

	wchar_t* dummy;
	std::wstring field;
	TcsCsvStatus status;
	
	ok = GetField (field,recordNbr,fieldNbr,status);
	if (ok)
	{
		wchar_t firstChar = *field.c_str();
		ok = iswdigit (firstChar);
		if (ok)
		{
			rtnValue = wcstoul (field.c_str(),&dummy,10);
		}
	}
	return ok;
}
