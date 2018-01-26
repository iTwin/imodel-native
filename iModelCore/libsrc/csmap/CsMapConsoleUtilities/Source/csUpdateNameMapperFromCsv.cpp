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

#include <iomanip>

extern "C" unsigned long KcsNmInvNumber;
extern "C" unsigned long KcsNmMapNoNumber;
extern "C" double cs_Zero;

TcsNameMapper VcsNameMapper;

// This utility updates the NameMapper.csv source file with new information
// extracted from another data file in .csv format.
//
// Specifically, we update the NNameMapper source file with EPSG codes and
// names, and ESRI codes and names, extracted from both the current EPSG
// dataset and a .csv file which contains the latest ESRI information in the
// following form:
//	Field 1: Integer denoting the flavor of the entry
//	Field 2: EPSG code of the subject system, 0 == unknown
//	Field 3: Numeric ID of the "Field 1" flavor for the subject system
//	FIeld 4: The definition of the system in WKT of the "Field 1" flavor.
//
// If time, energy, and money are sufficient, we may try also verifying that
// All geographic references, datum references, ellipsoid references, and
// unit references are also updated to the degree possible.  This gets rather
// iffy as there is no direct correlation between the names and a definition.
// With regard to the system name; well the name is essentially defined by the
// WKT system definition provided.

bool csNameMapperUpdateEpsg (const TcsEpsgDataSetV6* epsgPtr,TcsNameMapper& VcsNameMapper,
															 EcsMapObjType objType,
															 TcsEpsgCode epsgCode);
bool csNameMapperUpdateFlvr (const TcsEpsgDataSetV6* epsgPtr,TcsNameMapper& VcsNameMapper,
															 EcsMapObjType objType,
															 EcsNameFlavor flavor,
															 TcsEpsgCode epsgCode,
															 unsigned long wkidCode,
															 const wchar_t* crsName);
bool csNameMapperUpdateFlvr (const TcsEpsgDataSetV6* epsgPtr,TcsNameMapper& VcsNameMapper,
															 EcsMapObjType objType,
															 EcsNameFlavor flavor,
															 unsigned long wkidCode,
															 const wchar_t* crsName);
const wchar_t* csNameMapperHasName (EcsMapObjType objType,EcsNameFlavor trgFlavor,
														  EcsNameFlavor srcFlavor,
														  const wchar_t* srcName);
const wchar_t* csNameMapperHasNameC (EcsMapObjType objType,EcsNameFlavor trgFlavor,
														   EcsNameFlavor srcFlavor,
														   const char* srcName);

// This is not great style, but it relieves us of a lot of argument
// passing and suplicate code.  Perhaps this should be an object.

wchar_t wktUnitName [64];
wchar_t wktDatumName [64];
wchar_t wktSpheroidName [64];
wchar_t wktGeogCSName [128];
wchar_t wktProjectionName [128];
wchar_t wktProjCSName [128];
wchar_t wktRlsLevel [64];
wchar_t epsgRlsLevel [64];

#define VERBOSE //

bool csUpdateNameMapperFromCsv (const wchar_t* csDictTrgDir,const wchar_t* csDictSrcDir,
															const wchar_t* srcCsvFullPath)
{
	bool ok (false);
	const wchar_t* wcPtrK;

	wchar_t fullPath [1024];

	std::wifstream iStrm1;
	std::wifstream iStrm2;

	// We construct a NameMapper from scratch so we can access
	// all features directly.
	std::wcout << L"Reading NameMapper" << std::endl;

	VcsNameMapper.SetRecordDuplicates (true);

	wcscpy (fullPath,csDictSrcDir);
	wcscat (fullPath,L"\\NameMapper.csv");
	iStrm2.open (fullPath,std::ios_base::in);
	ok = iStrm2.is_open ();
	if (ok)
	{
		ok = iStrm2.good ();
	}
	if (ok)
	{
		TcsCsvStatus csvStatus;

		csvStatus.SetObjectName (L"NameMapper");
		EcsCsvStatus lclStatus = VcsNameMapper.ReadFromStream (iStrm2,csvStatus);
		iStrm2.close ();
		ok = (lclStatus == csvOk);
		if (ok)
		{
			ok = VcsNameMapper.IsInitialized ();
			if (!ok)
			{
				std::wcerr << L"NameMapper initialization failed." << std::endl;;
			}
		}
		else
		{
			std::wcerr << L"NameMapper load Failed: " <<  csvStatus.GetMessage () << std::endl;
		}
	}
	if (ok)
	{
		// Currently, the NameMapper always logs the 32 initial flavor
		// definition entries as duplicates.  More like an irritation
		// than bug, we defer fixing this to another time.  For now,
		// if the duplicate count is 32 or less, we assume there are
		// no duplicates.
		if (VcsNameMapper.GetDuplicateCount () > 32)
		{
			VcsNameMapper.WriteDuplicates (std::wcout);
		}
		else
		{
			VcsNameMapper.ClearDuplicateList ();
		}
	}
	if (!ok)
	{
		return ok;
	}

	std::wcout << L"Reading EPSG Database" << std::endl;

	// We're going to need access to the latest EPSG database.
	const TcsEpsgDataSetV6* epsgPtr;
	epsgPtr = GetEpsgObjectPtr ();
	ok = (epsgPtr != 0);
	if (!ok)
	{
		return ok;
	}
	wcPtrK = epsgPtr->GetRevisionLevel ();
	ok = (wcPtrK != 0);
	if (ok)
	{
		wcsncpy (epsgRlsLevel,wcPtrK,wcCount (epsgRlsLevel));
		epsgRlsLevel [wcCount (epsgRlsLevel) - 1] = L'\0';
	}
	else
	{
		wcsncpy (epsgRlsLevel,L"<release unknown>",wcCount (epsgRlsLevel));
		epsgRlsLevel [wcCount (epsgRlsLevel) - 1] = L'\0';
	}

	// Fetch a usable copy of the source .csv file which carries the update
	// information.  As WKT has lots of double quotes, commas, and other stuff
	// in it, we actually expect a file with the vertical bar as the field
	// separation character, with no "text" quoting.  The TcsCsvFileBase
	// object really likes to have three delimiters, so we just picked two
	// additional arkane characters which we are confident will never show up
	// in a WKT string.

	std::wcout << L"Reading WKT Database" << std::endl;

	wchar_t delimiters [6] = L"|``";
	TcsCsvStatus csvStatus;
	TcsCsvFileBase crsWktCsv (true,5,7,delimiters);
	iStrm1.open (srcCsvFullPath,std::ios_base::in);
	ok = iStrm1.is_open ();
	if (ok)
	{
		ok = crsWktCsv.ReadFromStream (iStrm1,true,csvStatus);
		iStrm1.close ();
	}
	if (!ok)
	{
		std::wcerr << L"Reading of " << srcCsvFullPath << L" failed." << std::endl;
		return ok;
	}

	// All set to go.  We loop once for each entry in the update data
	// source file.

	std::wcout << L"Processing WKT Catalog entries now." << std::endl;

	unsigned recNbr;
	for (recNbr = 0;ok && recNbr < crsWktCsv.RecordCount ();recNbr += 1)
	{
		bool isGeographic (false);
		bool isProjective (false);

		long tmpLong;
		wchar_t *wcDummy;
		const wchar_t *wcPtr;

		EcsNameFlavor flavor (csMapFlvrUnknown);
		EcsMapObjType objType (csMapNone);
		TcsGenericId srchCode;		// Default constructor initializes to 0UL, we're counting on that.
		TcsGenericId wkidCode;
		TcsGenericId epsgId;
		TcsEpsgCode epsgCode;

		char wktText [2048];
		std::wstring fieldData;

		// Somewhat redundant, but this is a utility.  Performance and
		// efficiency are not serious objectives.
		memset (wktUnitName,'\0',sizeof (wktUnitName));
		memset (wktDatumName,'\0',sizeof (wktDatumName));
		memset (wktSpheroidName,'\0',sizeof (wktSpheroidName));
		memset (wktProjectionName,'\0',sizeof (wktProjectionName));
		memset (wktGeogCSName,'\0',sizeof (wktGeogCSName));
		memset (wktProjCSName,'\0',sizeof (wktProjCSName));
		memset (wktRlsLevel,'\0',sizeof (wktRlsLevel));
		memset (wktText,'\0',sizeof (wktText));
		fieldData.clear ();

		if ((recNbr % 100) == 0)
		{
			std::wcout << L"Processing WKT record number " << recNbr << L"\r" << std::flush;
		}
		// Extract the fields from the WKT Catalog .CSV file.
		// First field is the WKT flavor
		ok = crsWktCsv.GetField (fieldData,recNbr,static_cast<short>(0),csvStatus);
		if (ok)
		{
			tmpLong = wcstol (fieldData.c_str (),&wcDummy,10);
			flavor = static_cast<EcsNameFlavor>(tmpLong);
		}

		// The currently assigned EPSG code for this WKT system.
		if (ok)
		{
			ok = crsWktCsv.GetField (fieldData,recNbr,1,csvStatus);
			if (ok)
			{
				tmpLong = wcstol (fieldData.c_str (),&wcDummy,10);
				epsgCode = tmpLong;
			}
		}

		// If the extracted EPSG code is zero, there is no known EPSG
		// cporrespondence between the WKT entry and EPSG. Since we have
		// nothing to match this with, we simply ignoire this record
		// in this utiltiy module.
		if (epsgCode == 0UL)
		{
			continue;
		}

		if ((epsgCode >= 4000 && epsgCode <= 4022) ||
			(epsgCode >= 4024 && epsgCode <= 4025) ||
			(epsgCode >= 4027 && epsgCode <= 4036) ||
			(epsgCode >= 4041 && epsgCode <= 4045) ||
			(epsgCode >= 4000 && epsgCode <= 4022) ||
			(epsgCode >= 4000 && epsgCode <= 4022) ||
			(epsgCode >= 4047 && epsgCode <= 4047) ||
			(epsgCode >= 4052 && epsgCode <= 4054))
		{
			// These are all cartographically referenced, i.e. referenced to an
			// ellipsoid and not a datum.   They don't really count in the
			// overall scheme of things; they seem to brak all the consistency
			// rules.
			continue;
		}

		// Determine that the EPSG code is valid and of the type that we deal
		// with here.  If not, we just skip it.
		ok = epsgPtr->GetFieldByCode (fieldData,epsgTblReferenceSystem,epsgFldCoordRefSysName,epsgCode);
		if (!ok)
		{
			// This EPSG code appears to be bogus.  We'll skip it for now.
			// We can use this line of code in _DEBUG to set a break
			// point and determine which of the non-zero EPSG codes in the
			// WktDataCatalog file are bogus (if any).
			ok = true;
			continue;
		}

		// Get the type of definition EPSG thinks this is.  We'll assume EPSG
		// tp be correct on most all matters.  What we want to do
		// right here isfilter out any EPSG/WKT definitions which are not
		// Projective, GHeographic 2D, and/or Geographic 3D.  The Oracle
		// entries in the WktDataCatalog have a lot of Geocentric definitions
		// typed as GEOGCS's.  This is not nice.
		EcsCrsType crsType = epsgPtr->GetCrsType (epsgCode);
		if ((crsType != epsgCrsTypGeographic2D) &&
			(crsType != epsgCrsTypGeographic3D) &&
			(crsType != epsgCrsTypProjected))
		{
			// This EPSG code is not of type we know how to deal with.
			continue;
		}

		// The WKID code (or SRID code. I.e. numeric code value for whatever
		// flavor we are dealing with.)
		if (ok)
		{
			ok = crsWktCsv.GetField (fieldData,recNbr,2,csvStatus);
			if (ok)
			{
				tmpLong = wcstol (fieldData.c_str (),&wcDummy,10);
				wkidCode = tmpLong;
			}
		}

		// The WKT release level, or what we will call the Flavored Release
		// Level.  That is, the release level of all information in the record.
		// A value along the lines of 10.2 is expected, 10 being the major
		// revision level and 2 being the minor revision level.
		if (ok)
		{
			ok = crsWktCsv.GetField (fieldData,recNbr,3,csvStatus);
			if (ok)
			{
				wcsncpy (wktRlsLevel,fieldData.c_str (),wcCount (wktRlsLevel));
				wktRlsLevel [wcCount (wktRlsLevel) - 1] = L'\0';
			}
		}

		// Extract the WKT, and the parse it with the simple WKT parser.
		if (ok)
		{
			ok = crsWktCsv.GetField (fieldData,recNbr,4,csvStatus);
			if (ok)
			{
				wcstombs (wktText,fieldData.c_str (),sizeof (wktText));
				wktText [sizeof (wktText) - 1] = '\0';
			}
		}
		// Test the WKT.
		if (ok)
		{
			ok = CS_isWkt (wktText);
		}

		// Parse the WKT and extract the specific values of interest to us here.
		if (ok)
		{
			const TrcWktElement *wktElePtr;

			// At this initial point, wktElements will only be the top level element.
			// Later on, it will be morphed into a container of all the sub-elements,
			// thus the name is plural.
			TrcWktElement wktElements (wktText);
			ErcWktEleType wktEleType = wktElements.GetElementType ();
			ok = (wktEleType != rcWktUnknown);
			if (ok)
			{
				isGeographic = (wktEleType == rcWktGeogCS);
				isProjective = (wktEleType == rcWktProjCS);
				if (!(isGeographic || isProjective))
				{
					// It's neither geographic or projective.  We don't
					// know what it is.  Just skip it for now.
					continue;
				}

				// Establish the object type for this particular item.
				objType = csMapNone;
				if (isProjective && crsType == epsgCrsTypProjected)
				{
					objType = csMapProjectedCSysKeyName;
				}
				else if (isGeographic)
				{
					// If the type is geographic, we query the EPSG database
					// to determine if 2D or 3D.  Many GEOGC's in the Oracle
					// flavor are actually geocentric, so we filter that out
					// as well.
					if (crsType == epsgCrsTypGeographic2D)
					{
						objType = csMapGeographicCSysKeyName;
					}
					else if (crsType == epsgCrsTypGeographic3D)
					{
						objType = csMapGeographic3DKeyName;
					}
				}
				if (objType == csMapNone)
				{
					// This is, usually, an Oracle geocentric system with a
					// GEOGCS WKT type.
					continue;
				}

				// Parse the rest of the WKT definition.
				wktElements.ParseChildren ();
			}
			if (ok)
			{
				// Now we should be able to extract the remainder of what
				// interests us in this utility. Regardless of type, there
				// should be a UNIT name, a DATUM name, and a SPHEROID name.
				wktElePtr = wktElements.ChildLocate (rcWktUnit);
				ok = (wktElePtr != 0);
				if (ok)
				{
					if (!CS_stristr (wktElePtr->GetElementNameC (),"(EPSG "))
					{
						mbstowcs (wktUnitName,wktElePtr->GetElementNameC (),wcCount (wktUnitName));
					}
				}
				if (ok && isGeographic)
				{
					if (!CS_stristr (wktElePtr->GetElementNameC (),"(EPSG "))
					{
						// For geographic systems, the geographic name is at the top level.
						mbstowcs (wktGeogCSName,wktElements.GetElementNameC (),wcCount (wktGeogCSName));
					}

					// For geographic systems, the datum is at the top level.
					wktElePtr = wktElements.ChildLocate (rcWktDatum);
					ok = (wktElePtr != 0);
					if (ok)
					{
						if (!CS_stristr (wktElePtr->GetElementNameC (),"(EPSG "))
						{
							mbstowcs (wktDatumName,wktElePtr->GetElementNameC (),wcCount (wktDatumName));
						}
						const TrcWktElement *lclWktPtr = wktElePtr->ChildLocate (rcWktSpheroid);
						ok = (lclWktPtr != 0);
						if (ok)
						{
							if (!CS_stristr (wktElePtr->GetElementNameC (),"(EPSG "))
							{
								mbstowcs (wktSpheroidName,lclWktPtr->GetElementNameC (),wcCount (wktSpheroidName));
							}
						}
					}

					// TAKE NOTE!!!  For geographic systems, the wktProjCSName array is empty.
					wktProjCSName [0] = L'\0';
				}
				else if (ok && isProjective)
				{
					// If the type is Projective, there is a GeogCS name
					// distinct from the ProjCS name.  There is also a projection name.
					mbstowcs (wktProjCSName,wktElements.GetElementNameC (),wcCount (wktProjCSName));
					wktElePtr = wktElements.ChildLocate (rcWktGeogCS);
					ok = (wktElePtr != 0);
					if (ok)
					{
						if (!CS_stristr (wktElePtr->GetElementNameC (),"(EPSG "))
						{
							mbstowcs (wktGeogCSName,wktElePtr->GetElementNameC (),wcCount (wktGeogCSName));
						}
						wktElePtr = wktElePtr->ChildLocate (rcWktDatum);
						ok = (wktElePtr != 0);
						if (ok)
						{
							if (!CS_stristr (wktElePtr->GetElementNameC (),"(EPSG "))
							{
								mbstowcs (wktDatumName,wktElePtr->GetElementNameC (),wcCount (wktDatumName));
							}
							wktElePtr = wktElePtr->ChildLocate (rcWktSpheroid);
							ok = (wktElePtr != 0);
							if (ok)
							{
								if (!CS_stristr (wktElePtr->GetElementNameC (),"(EPSG "))
								{
									mbstowcs (wktSpheroidName,wktElePtr->GetElementNameC (),wcCount (wktSpheroidName));
								}
							}
						}
					}
					if (ok)
					{
						wktElePtr = wktElements.ChildLocate (rcWktProjection);
						ok = (wktElePtr != 0);
						if (ok)
						{
							// In Oracle, especially in release 10, the projection
							// name is not a true projection name.  It speciiifies
							// only nmemonic for the projection, and implies the
							// projection and all projection parameters by way of a
							// reference to the EPSG 'Coordinate Operation" code in
							// the form of a "(EPSG 1606").  We filter these
							// references out here as they have no value in our
							// effort to update the NameMapper.
							if (!CS_stristr (wktElePtr->GetElementNameC (),"(EPSG "))
							{
								mbstowcs (wktProjectionName,wktElePtr->GetElementNameC (),wcCount (wktProjectionName));
							}
						}
					}
				}
			}
		}
		else
		{
			std::wcerr << L"Invalid WKT on record number " << recNbr << L"of the WKT catalog." << std::endl;
			ok = true;
			continue;
		}

		TcsGenericId genericId = VcsNameMapper.Locate (objType,csMapFlvrEpsg,epsgCode);
		if (genericId.IsNotKnown ())
		{
			ok = true;
			continue;
		}

		// Some preliminary data collection before the real work of this process.
		// First, verify that NameMapper recognizes the units of this system.	
		const wchar_t* aDeskName;
		if (wktUnitName [0] != L'\0')
		{
			EcsMapObjType lclObjType = isProjective ? csMapLinearUnitKeyName : csMapAngularUnitKeyName;
			aDeskName = csNameMapperHasName (lclObjType,csMapFlvrAutodesk,flavor,wktUnitName);
			if (aDeskName == 0)
			{
				std::wcerr << lclObjType << L',' << flavor << L",\"" << wktUnitName << "\"," << recNbr + 2 << std::endl;
			}
		}
		if (wktSpheroidName [0] != L'\0')
		{
			EcsMapObjType lclObjType = csMapEllipsoidKeyName;
			aDeskName = csNameMapperHasName (lclObjType,csMapFlvrAutodesk,flavor,wktSpheroidName);
			if (aDeskName == 0)
			{
				std::wcerr << lclObjType << L',' << flavor << L",\"" << wktSpheroidName << "\"," << recNbr + 2 << std::endl;
			}
		}
		if (wktDatumName [0] != L'\0')
		{
			EcsMapObjType lclObjType = csMapDatumKeyName;
			aDeskName = csNameMapperHasName (lclObjType,csMapFlvrAutodesk,flavor,wktDatumName);
			if (aDeskName == 0)
			{
				std::wcerr << lclObjType << L',' << flavor << L",\"" << wktDatumName << "\"," << recNbr + 2 << std::endl;
			}
		}
		if (wktProjectionName [0] != L'\0')
		{
			EcsMapObjType lclObjType = csMapProjectionKeyName;
			aDeskName = csNameMapperHasName (lclObjType,csMapFlvrAutodesk,flavor,wktProjectionName);
			if (aDeskName == 0)
			{
				std::wcerr << lclObjType << L',' << flavor << L",\"" << wktProjectionName << "\"," << recNbr + 2 << std::endl;
			}
		}
		if (isGeographic && wktGeogCSName [0] != L'\0')
		{
			EcsMapObjType lclObjType = objType;
			aDeskName = csNameMapperHasName (lclObjType,csMapFlvrAutodesk,flavor,wktGeogCSName);
			if (aDeskName == 0)
			{
				std::wcerr << lclObjType << L',' << flavor << L",\"" << wktGeogCSName << "\"," << recNbr + 2 << std::endl;
			}
		}
		if (isProjective && wktProjCSName [0] != L'\0')
		{
			EcsMapObjType lclObjType = objType;
			aDeskName = csNameMapperHasName (lclObjType,csMapFlvrAutodesk,flavor,wktProjCSName);
			if (aDeskName == 0)
			{
				std::wcerr << lclObjType << L',' << flavor << L",\"" << wktProjCSName << "\"," << recNbr + 2 << std::endl;
			}
		}

		//*********************************************************************
		//  For now, we deal only with systems for which we have an EPSG
		// definition in the Name Mapper.  As of this writing, (Nov 2014) there
		// are hundreds of CRS definitions in the ESRI WKT reptoire which we
		// have not dealt with before (I.e. there is no EPSG entry in the
		// NameMapper.  We'll deal with those in future.  Time constraints
		// require that we deal with the systems which we know about.
		//*********************************************************************
		aDeskName = 0;
		if (isGeographic)
		{
			aDeskName = csNameMapperHasName (objType,csMapFlvrAutodesk,flavor,wktGeogCSName);
		}
		if (isProjective)
		{
			aDeskName = csNameMapperHasName (objType,csMapFlvrAutodesk,flavor,wktProjCSName);
		}
		if (aDeskName == 0)
		{
			ok = true;
			continue;
		}

		// Data capture is done.  Now to put the captured data into the right places.
		// We start by updating the EPSG name for all systems for which we are given
		// an EPSG code.  The assumption here is that if flavor 'X' did something,
		// maybe EPSG did so as well.
		if (epsgCode.IsValid ())
		{
			// We have an EPSG code, so we presume it is worth checking the
			// EPSG name which is current in the NameMapper.  The name is verified
			// against the current EPSG database.
			ok = csNameMapperUpdateEpsg (epsgPtr,VcsNameMapper,objType,epsgCode);
		}
		if (ok)
		{
			// Select the appropriate name to deal with..
			wcPtr = (objType ==  csMapProjectedCSysKeyName) ? wktProjCSName : wktGeogCSName;

			// Now, we need to deal with the flavored name and ID.  If we
			// have an EPSG code, we need to match the flavored name and ID
			// with the existing EPSG entry.
			if (epsgCode.IsValid ())
			{

				// wcPtr points to the name we want in the NameMapper for this
				// wkid code, the EPSG code to be used to locate the specific
				// system which is to be updated.  That is, there is a group of
				// name mapper entries with a common generic ID which "define"
				// as far as the NameMapper is concerned the specific CRS
				// system.  The EPSG code provided to this function is used,
				// primarily, to determine the commonly shared generic ID value.
				ok = csNameMapperUpdateFlvr (epsgPtr,VcsNameMapper,objType,flavor,
																		epsgCode,
																		static_cast<unsigned long>(wkidCode),
																		wcPtr);
			}
			else
			{
				// There is no EPSG code which we can use to identify an
				// existing group.  We have a more sophisticated algorithm
				// to update, and//or possibllly create a new CRS system
				// group in the NameMapper.

				// As of Noov 2014, this function is never called;;; shedule
				// contraints prevent this author from finishing this portion
				// of this utility application.
				ok = csNameMapperUpdateFlvr (epsgPtr,VcsNameMapper,objType,flavor,
																		static_cast<unsigned long>(wkidCode),
																		wcPtr);
			}
		}
		if (!ok)
		{
			ok = true;
		}
	}
	std::wcout << L"Processing of WKT catalog is complete." << std::endl;

	// If all of that succeeded, we write a new NameMapper.csv source file out.
	if (ok)
	{
		std::wofstream oStrm1;
		std::wofstream oStrm2;

		std::wcout << L"Writing new Name Mapper." << std::endl;

		wcscpy (fullPath,csDictTrgDir);
		wcscat (fullPath,L"\\");
		wcscat (fullPath,L"NameMapper.csv");
		oStrm1.open (fullPath,std::ios_base::out | std::ios_base::trunc);
		ok = oStrm1.is_open ();
		if (ok)
		{
			// WriteAsCsv does not return a status.  This is not so
			// hot; probably related to the fact that the operator<<
			// function is involved somehow.
			VcsNameMapper.WriteAsCsv (oStrm1,true);
			oStrm1.close ();

			if (VcsNameMapper.GetDuplicateCount () > 0)
			{
				std::wcerr << VcsNameMapper.GetDuplicateCount () << L" duplicates detected." << std::endl;
				wcscpy (fullPath,csDictTrgDir);
				wcscat (fullPath,L"\\");
				wcscat (fullPath,L"NameMapperDuplicates.csv");
				oStrm2.open (fullPath,std::ios_base::out | std::ios_base::trunc);
				ok = oStrm2.is_open ();
				if (ok)
				{
					std::wcout << L"Writing Name Mapper Duplicates." << std::endl;

					// WriteAsCsv does not return a status.  This is not so
					// hot; probably related to the fact that the operator<<
					// function is involved somehow.
					VcsNameMapper.WriteDuplicates (oStrm2);
					oStrm2.close ();
				}
			}
		}
	}
	return ok;
}

// Given an EPSG code of a CRS, the EPSG database, and the NameMapper, update
// the NameMapper to contain the latest EPSG name available.  Since we do not
// usually deal with  EPSG WKT, we make no attempt to keep old names around.
// If the name is different, we simply replace it in the namemapper.
bool csNameMapperUpdateEpsg (const TcsEpsgDataSetV6* epsgPtr,TcsNameMapper& VcsNameMapper,
															 EcsMapObjType objType,
															 TcsEpsgCode epsgCode)
{
	bool ok;
	const wchar_t* wcPtrK = 0;
	
	wchar_t aliasComment [256];

	TcsGenericId systemId;
	std::wstring fieldData;

	TcsNameMap oldEntry;
	TcsNameMap newEntry;

	// Extract the latest EPSG name from the database.  This gets us the name,
	// regardless of deprecation, associated with a specific CRS code number as
	// of the current release level.
	ok = epsgPtr->GetFieldByCode (fieldData,epsgTblReferenceSystem,epsgFldCoordRefSysName,epsgCode);
	if (!ok)
	{
		// Oops!!!  We have a reference to a non-existent code value.
		// Report it and then ignore the rest of this entry.
		std::wcerr << L"WKT data has bogus reference to EPSG code ["
				   << static_cast<unsigned long>(epsgCode)
				   << L"]."
				   << std::endl;
		ok = true;		
		return ok;
	}

	// Get the system to which the caller is referencing.  That is, get the
	// generic ID which is common to all references to the specific system
	// identified by the EPSG code.
	systemId = VcsNameMapper.Locate (objType,csMapFlvrEpsg,epsgCode);
	if (systemId.IsNotKnown ())
	{
		std::wcerr << L"NameMapper has no reference to EPSG code ["
				   << static_cast<unsigned long>(epsgCode)
				   << L"]. Object: "
				   << objType
				   << std::endl;
		ok = true;
		return ok;
	}

	// Get the reference within this system which carries the EPSG code we
	// have been given. When an EPSG CRS definition gets deprecated for benign
	// reasons such as a name change (which is quite common), there may be
	// several EPSG name map entries within a specific system.  We need to
	// obtain the specific one which has the provided EPSG code as its
	// numeric ID.
	ok = VcsNameMapper.ExtractSpecificId (oldEntry,objType,systemId,
														csMapFlvrEpsg,
														static_cast<unsigned long>(epsgCode));

	if (ok)
	{
		// There appears to be a record that might need updating.  Get the
		// name associated with the entry located above.
		wcPtrK = oldEntry.GetNamePtr ();
		ok = (wcPtrK != 0 && *wcPtrK != L'\0');
		if (!ok)
		{
			// This should never happen.  Name map entries may not have a
			// numeric ID, but they all must have a name.
			std::wcerr << L"NameMapper has no name for EPSG code ["
						<< static_cast<unsigned long>(epsgCode)
						<< L"]."
						<< std::endl;
			ok = true;
			return ok;
		}
	}
	if (ok)
	{
		if (wcsicmp (wcPtrK,fieldData.c_str ()))
		{
			// The names exist, but are different.
			TcsNameMap newEntry (oldEntry);
			swprintf (aliasComment,wcCount(aliasComment),L"EPSG changed this name in release %s.",epsgRlsLevel);
			newEntry.SetNameId (fieldData.c_str ());
			newEntry.SetComments (aliasComment);
			ok = VcsNameMapper.Replace (newEntry,oldEntry);

VERBOSE		std::wcout << L"EPSG::" << epsgCode << L" name updated to \"" << fieldData.c_str() << L"\"." << std::endl;
		}
	}
	return ok;
}
// This overloaded function (probably a bad idea to overload it):
// 1> locates the system which is identified by the EPSG code,
// 2> locates an existing flavored name entry in the NameMapper for
//	  this system which might already exist.
//		a> if the name provided already exists, we're done.
//		b> if 'a' name already exists but is different, we alias the existing name.
//		c> if the name does not exist, we create a new NameMapperEntry for the new name with
//		   the given wkid number.
bool csNameMapperUpdateFlvr (const TcsEpsgDataSetV6* epsgPtr,TcsNameMapper& VcsNameMapper,
															 EcsMapObjType objType,
															 EcsNameFlavor flavor,
															 TcsEpsgCode epsgCode,
															 unsigned long wkidCode,
															 const wchar_t* crsName)
{
	bool ok (true);
	const wchar_t* wcPtrK (0);

	TcsGenericId epsgGeneric;
	TcsGenericId flavorGeneric;

	wchar_t aliasComment [256];

	// This overload requires a valid wkid.
	if (wkidCode == 0UL)
	{
		return false;
	}

	// Convert the EPSG ID to a genric ID.
	epsgGeneric = VcsNameMapper.Locate (objType,csMapFlvrEpsg,epsgCode);
	ok = epsgGeneric.IsKnown();
	if (!ok)
	{
		// NameMapper is unaware of this EPSG code.  Not supposed to happen in
		// this function.
		return false;
	}

	// See if this name already exists in the NameMapper.  If it is, and
	// it references the same system, we have nothing to do.  At this point
	// we do not check if the name is an alias or not.  The name will map
	// to this system, that's all we care about at this time.
	flavorGeneric = VcsNameMapper.Locate (objType,flavor,crsName);
	if (flavorGeneric == epsgGeneric)
	{
		return ok;
	}

	// Need to locate the name wihtin the identified system which has the
	// provided wkidCode.  Is the provided wkidCode unique?  Good
	// question.  There is nothing in the design of the NameMapper to
	// guarantee this.
	if (ok)
	{
		// Get the existing 'flavor' name for this system withthe provided wkid.
		wcPtrK = VcsNameMapper.LocateName (objType,flavor,epsgGeneric);
	}
	if (wcPtrK == 0)
	{
		// No flavored name exists related to the item identified by the EPSG
		// code.  We simply add a new record to the NameMapper.
		TcsNameMap newEntry (epsgGeneric,objType,flavor,wkidCode,crsName);
		ok = VcsNameMapper.Add (newEntry);
		if (ok)
		{
VERBOSE		std::wcout << epsgGeneric << L":" << objType << L":" << flavor << L":" << crsName << L" added to the NameMapper." << std::endl;
		}
		else
		{
			// Given how weird these data sets are, I suppose its likely
			// for this operation to fail occasionally.
			wcPtrK = TcsNameMapper::FlvrNbrToName (flavor);
			std::wcerr << L"Add new entry failed: "
					   << objType
					   << L":"
					   << wcPtrK
					   << L":"
					   << wkidCode
					   << L" \""
					   << crsName
					   << L"\"  [EPSG: "
					   << static_cast<unsigned long>(epsgCode)
					   << L"]."
					   << std::endl;
			ok = true;
		}
	}
	else if (wcsicmp (wcPtrK,crsName))
	{
		// The appropriate entry exists, but the names are different.  We need
		// to adjust the name mapper.
		swprintf (aliasComment,wcCount (aliasComment),L"Name changed by flavor source in release %s.",wktRlsLevel);
		ok = VcsNameMapper.AliasExistingName (objType,flavor,epsgGeneric,crsName,aliasComment);
		if (!ok)
		{
			// Should not happen.
			std::wcerr << L"NameMapper AliasExistingName failed: "
					   << objType
					   << L":"
					   << flavor
					   << L":"
					   << static_cast<unsigned long>(epsgCode)
					   << L":"
					   << crsName
					   << std::endl;
			ok = true;
		}
	}
	return ok;
}
// This updates a a flavored entry for which we do not have a matching EPSG code.
// This is yet to be rigorously tested.
bool csNameMapperUpdateFlvr (const TcsEpsgDataSetV6* epsgPtr,TcsNameMapper& VcsNameMapper,
															 EcsMapObjType objType,
															 EcsNameFlavor flavor,
															 unsigned long wkidCode,
															 const wchar_t* crsName)
{
	bool ok (true);
	bool hasName (false);
	bool nameOk (false);
	bool codeOk (false);
	const wchar_t* wcPtrK;

	TcsGenericId srchCodeNm;
	TcsGenericId srchCodeCd;

	wchar_t aliasComment [256];

	// Here for a flavored Name and ID, but no EPSG code to associate them
	// with.  If we don't have both a name and an ID, we skip this item as
	// there is no way to link this entry to anything else now or in the
	// future.
	hasName = (crsName != 0 && *crsName != L'\0');
	if (wkidCode != 0 && hasName)
	{
		// We have both a name and an ID.  Search the NameMapper
		// for an existing entry; preferring to use Name over ID.
		srchCodeNm = VcsNameMapper.Locate (objType,flavor,crsName);
		srchCodeCd = VcsNameMapper.Locate (objType,flavor,wkidCode);
		if (srchCodeNm.IsNotKnown() && srchCodeCd.IsNotKnown ())
		{
			ok = true;		// We can handle this situation.
			nameOk = codeOk = false;
		}
		else if (srchCodeNm.IsKnown() && srchCodeCd.IsKnown ())
		{
			// We located an entry using the name and code.  If they are
			// the same entry, then there is nothing to do.  If they are
			// not the same, then we have a problem.
			ok = (srchCodeNm == srchCodeCd);
			nameOk = codeOk = ok;
		}
		else
		{
			ok = true;
			nameOk = srchCodeNm.IsKnown();
			codeOk = srchCodeNm.IsKnown();
		}

		// Search code will be unknown if we could not find an entry by either
		// name or id number.
		if (ok && !nameOk && !codeOk)
		{
			TcsGenericId unknownId;
			TcsGenericId lclGenericId;
			// The entry is new, we simply add an appropriate entry to the
			// NameMapper.  Must use the NameMapper to get a generic ID to
			// use.  GenericID's must be unique, and we like to generate them
			// such that they contain flavor information in the new unique
			// code.  If a flavor ID has been given in the source data, we use
			// that to generate a generic ID to be used when creating the
			// new record.
			if (wkidCode == 0UL)
			{
				lclGenericId = VcsNameMapper.GetNextDfltId (flavor);
			}
			else
			{
				lclGenericId = TcsGenericId (flavor,wkidCode);
			}

			// Add the new item.
			TcsNameMap nextItem (lclGenericId,objType,flavor,wkidCode,crsName);
			ok = VcsNameMapper.Add (nextItem);
			if (!ok)
			{
				wcPtrK = TcsNameMapper::FlvrNbrToName (flavor);
				std::wcerr << L"Add new entry failed: "
				<< objType
				<< L":"
				<< wcPtrK
				<< L":"
				<< wkidCode
				<< L" \""
				<< crsName
				<< L"\"  [WKID: "
				<< static_cast<unsigned long>(wkidCode)
				<< L"]."
				<< std::endl;
				ok = true;
			}
		}
		else if (ok && !nameOk && codeOk)
		{
			// An entry exists which was identified by the wkidCode value.
			// Update the name.  The assumptions here are:
			//  1> Since an entry exists, there must be a name.
			//     NameMapper entries are NOT required to have ID's,
			//     but are required to have names.
			//  2> If the name in the NameMapper was up to date, the
			//     name check above would have found it and we wouldn't
			//     be executing this block of code.
			swprintf (aliasComment,wcCount (aliasComment),L"Name changed by flavor source in release %s.",wktRlsLevel);
			ok = VcsNameMapper.AliasExistingName (objType,flavor,wkidCode,crsName,aliasComment);
		}
		else if (ok && nameOk && !codeOk)
		{
			// The flavored numeric ID is not a part of the STD::set<> key, so
			// we could indeed change this in place without having to remove and
			// add back in.  CUrrently, we pretend that it is in order to
			// reduce the likely hood of a cut & paste code error.
			TcsNameMap extractedItem;
			ok = VcsNameMapper.ExtractAndRemove (extractedItem,objType,flavor,crsName,0,0);
			if (ok)
			{
				TcsGenericId genericId = extractedItem.GetGenericId ();
				if (genericId.IsKnown ())
				{
					// One might think we should look at the generic ID of this
					// extracted item since they often have the flavored numeric
					// ID buried in them.  However, if we did change that, we
					// would lose any association with other entries in the
					// NameMapper.  So we leave it, and perhaps there will be
					// a utility to regenerate generic ID's some day.
					extractedItem.SetNumericId (wkidCode);
					ok = VcsNameMapper.Add (extractedItem);
				}
			}
			else
			{
				wcPtrK = TcsNameMapper::FlvrNbrToName (flavor);
				std::wcerr << L"Numeric ID Add failed: "
						   << objType
						   << L":"
						   << wcPtrK
						   << L":"
						   << wkidCode
						   << L" \""
						   << crsName
						   << L"\"."
						   << std::endl;
				ok = true;
			}
		}
		else if (!ok)
		{
				wcPtrK = TcsNameMapper::FlvrNbrToName (flavor);
				std::wcerr << L"Out of sync: "
						   << wcPtrK
						   << L":"
						   << wkidCode
						   << L" \""
						   << crsName
						   << L"\"  [EPSG: 0]."
						   << std::endl;
				ok = true;
		}
	}
	return ok;
}
const wchar_t* csNameMapperHasName (EcsMapObjType objType,EcsNameFlavor trgFlavor,EcsNameFlavor srcFlavor,const wchar_t* srcName)
{
	const wchar_t* rtnValue = 0;

	TcsGenericId genericId;

	genericId = VcsNameMapper.Locate (objType,srcFlavor,srcName);
	if (genericId.IsKnown ())
	{
		rtnValue = VcsNameMapper.LocateName (objType,trgFlavor,genericId);
	}
	return rtnValue;
}
const wchar_t* csNameMapperHasNameC (EcsMapObjType objType,EcsNameFlavor trgFlavor,EcsNameFlavor srcFlavor,const char* srcName)
{
	const wchar_t* rtnValue = 0;
	wchar_t nmBufr [256];
	TcsGenericId genericId;

	mbstowcs (nmBufr,srcName,wcCount (nmBufr));
	genericId = VcsNameMapper.Locate (objType,srcFlavor,nmBufr);
	if (genericId.IsKnown ())
	{
		rtnValue = VcsNameMapper.LocateName (objType,trgFlavor,genericId);
	}
	return rtnValue;
}
