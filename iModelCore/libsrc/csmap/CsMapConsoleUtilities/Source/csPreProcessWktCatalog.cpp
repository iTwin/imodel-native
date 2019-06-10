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
//	This function pre-processes a WKT Data Catalog file.
//
//	A WKT Data Catalog file is defined as a .CSV file with the following
//	characteristics:
//		1> Separator delimiter is the vertical bar character L'|';
//		2> None of the fields in the records are "text quoted".
//		3> Readable by TcsCsvFileBase with delimiters specified as L"|``"
//			(The last two delimiter characters being quite arbitrary, just
//			 some character that should never appear in the datat to be
//			 processed.  A character is necessary as the TcsCsvFileBase
//			 object expects all delimiter specifications to contain
//			 exactly three characters.)
//		4> There are minimimum of five fields:
//			a> Flavor, as an numeric integer, per EcsNameFlavor
//			b> The EPSG code number of the item, 0 if not known
//			c> The flavor specific numeric ID, 0 if not known.  This is
//			   refered to as WKID hereinafter.
//			d> The release level of the WKT definition as a real number,
//			   e.g. (10.2)
//			e> The actual text of the WKT string; unquoted so as to avoid the
//			   distracting double quoting of all the stings in the WKT.
//			f> Optionally a Comment field which carries comments related to the
//			   specific WKT text itself.
//			g> Optionally a Remark field which carries remarks concerning the
//			   specific entry as it relates to the data catalog and maintnenance
//			   thereof.
//
//	This Pre-Processor will, sequentially:
//		1> Read the entire file into a TcsCsvFileBase object.
//		2> Sort the entire catalog by two fields: Flavor and WKT
//		3> Eliminate duplicate WKT specifications from the catalog, and in so
//		   doing:
//			a> Keep the record with the highest release level.
//			b> Transfer a EPSG code value from an older duplicate, to the
//			   newer record if the newer record has a 0 for EPSG code.
//		4> Resort the results into order by Flavor, Release, and WKID.
//		5> Write the result to a file with the same name in the directory
//		   specified as targetDirectory.
//
//	It is intended that this processed WKT catalog be used to update the
//	NameMapper, DIctionaries, etc. and also the base for rigorous WKT testing.
//
//	At this time, there is nothing that is flavor specific in this code.
//	Hopefully, this statement will remain true.
//
//  At this time, sorting of the CSV object is rather inefficient time wise.
//	The sorting is accomplished using the STL std::stablesort algorithm, so
//	little in the way of meaningful progress reporting is possible without
//	jeodardizing the the process.
//

#include "csConsoleUtilities.hpp"

#include <iomanip>

extern "C" unsigned long KcsNmInvNumber;
extern "C" unsigned long KcsNmMapNoNumber;
extern "C" double cs_Zero;

bool PreProcessWktCatalog (const wchar_t* csDataTrgDir,const wchar_t* csDataSrcDir)
{
	bool ok (false);

	wchar_t* wcPtr;

	wchar_t fullPath [1024];

	std::wifstream iStrm;
	std::wofstream oStrm;

	wchar_t inDelimiters [4] = L"|``";
	wchar_t outDelimiters [4] = L"|``";

	CS_altdr (NULL);

	// Fetch a usable copy of the source .csv file which is to be preprocessed.
	// It could be at some point in time desireable to read a datafile with
	// standard delimiters, and switch to the delimiters defined above for
	// output purposes.  The standard delimiters make for easy editting in
	// Excel etc.  But all those double double quotes drive this author crazy.
	// Note that this code EXPECTS a label line.
	TcsCsvStatus csvStatus;
	TcsCsvFileBase crsWktCsv (true,5,7,inDelimiters);

	wcsncpy (fullPath,csDataSrcDir,MAXPATH);
	wcscat (fullPath,L"\\");
	wcscat (fullPath,L"WktDataCatalog.txt");
	iStrm.open (fullPath,std::ios_base::in);
	ok = iStrm.is_open ();
	if (!ok)
	{
		return ok;
	}

	// Read in the entire data catalog.
	std::wcout << L"Reading the source data catalog." << std::endl;
	ok = crsWktCsv.ReadFromStream (iStrm,true,csvStatus);
	iStrm.close ();
	if (!ok)
	{
		return ok;
	}

	// Construct the sort functor appropriate for this phase and sort the
	// entire catalog.
	std::wcout << L"Sorting all "
			   << crsWktCsv.RecordCount()
			   << L"WKT entries.  Please wait . . ."
			   << std::endl;
	TcsCsvSortFunctor sortFunctor1 (0,4,3);
	ok = crsWktCsv.StableSort (sortFunctor1);
	if (!ok)
	{
		std::wcout << L"Initial sort failed." << std::endl;
	}

	// Loop through all records and delete duplicate records; but only
	// after preserving the EPSG code if the the retained record does not
	// have one.
	std::wcout << L"Eliminating duplicate WKT definitions."
				   << std::endl;
	unsigned recNbr;
	unsigned dupCount;
	long prvFlvr, curFlvr;
	long prvEpsg, curEpsg;
	long prvWKid, curWKid;
	double prvRls, curRls;
	std::wstring prvWkt (L"  ");		// Initialize to keep lint happy.
	std::wstring curWkt (L"  ");

	dupCount = 0;
	prvFlvr = curFlvr = prvEpsg = curEpsg = prvWKid = curWKid = 0;			// keep lint happy
	curRls = 0.0;															// keep lint happy
	for (recNbr = 0;ok && recNbr < crsWktCsv.RecordCount();recNbr += 1)
	{
		std::wstring fieldData;

		ok = crsWktCsv.GetField (fieldData,recNbr,(short)0,csvStatus);
		if (ok) curFlvr = wcstol (fieldData.c_str (),&wcPtr,10);
		if (ok)
		{
			ok = crsWktCsv.GetField (fieldData,recNbr,1,csvStatus);
			if (ok) curEpsg = wcstol (fieldData.c_str (),&wcPtr,10);
		}
		if (ok)
		{
			crsWktCsv.GetField (fieldData,recNbr,2,csvStatus);
			curWKid = wcstol (fieldData.c_str (),&wcPtr,10);
		}
		if (ok)
		{
			ok = crsWktCsv.GetField (fieldData,recNbr,3,csvStatus);
			if (ok) curRls = wcstod (fieldData.c_str (),&wcPtr);
		}
if (ok && (curFlvr == 2) && (curWKid >= 53000) && (curWKid < 55000))
{
	fieldData = L"9999.99";
	ok = crsWktCsv.ReplaceField (fieldData,recNbr,3,csvStatus);
	continue;
}
if (ok && (curFlvr == 3) && (curWKid >= 53000) && (curWKid < 54000))
{
	fieldData = L"9999.99";
	ok = crsWktCsv.ReplaceField (fieldData,recNbr,3,csvStatus);
	continue;
}
if (ok && (curFlvr == 3) && (curWKid >= 2000000))
{
	fieldData = L"9999.99";
	ok = crsWktCsv.ReplaceField (fieldData,recNbr,3,csvStatus);
	continue;
}
		if (ok)
		{
			ok = crsWktCsv.GetField (curWkt,recNbr,4,csvStatus);
			// We may want to put something here to improve the
			// detection of duplicates here; eg remove all
			// whitespce which is not within a quoted string.

			// Definitely would like to eliminate and Authority element
			// on the end of the string, just for comparison purposes.
			// ESRI 10.1 has them, and 10.2 doesn't.  This causes a
			// mismach and we miss about 3,000 duplicates.
			if (ok)
			{
				std::wstring::size_type pos1;
				std::wstring::size_type pos2;
				pos1 = curWkt.rfind (L",AUTHORITY[\"EPSG\",");
				if (pos1 != std::wstring::npos)
				{
					pos2 = curWkt.find (L']',pos1 + 1);
					if (pos2 != std::wstring::npos)
					{
						curWkt.erase (pos1,pos2 - pos1 + 1);
					}
				}
				pos1 = curWkt.rfind (L",AUTHORITY[\"ESRI\",");
				if (pos1 != std::wstring::npos)
				{
					pos2 = curWkt.find (L']',pos1 + 1);
					if (pos2 != std::wstring::npos)
					{
						curWkt.erase (pos1,pos2 - pos1 + 1);
					}
				}
			}
		}
		if (ok && curEpsg == 0)
		{
			const char *cccWktName;
			char cccWkt [1024];

			// Parse the WKT text sufficiently enough to get the type and
			// the name.
			wcstombs (cccWkt,curWkt.c_str(),sizeof (cccWkt));
			cccWkt [sizeof  (cccWkt) - 1] = '\0';
			TrcWktElement wktElements (cccWkt);
			ErcWktEleType wktEleType = wktElements.GetElementType ();
			ok = (wktEleType != rcWktUnknown);
			if (ok)
			{
				bool isGeographic = (wktEleType == rcWktGeogCS);
				bool isProjective = (wktEleType == rcWktProjCS);
				if (isGeographic || isProjective)
				{
					unsigned long epsgCode;
					EcsMapObjType objType (csMapNone);

					// Establish the object type for this particular item.
					cccWktName = wktElements.GetElementNameC ();
					objType = isProjective ? csMapProjectedCSysKeyName : csMapGeographicCSysKeyName;
					epsgCode = csMapNameToIdC (objType,csMapFlvrEpsg,static_cast<EcsNameFlavor>(curFlvr),cccWktName);
					if (epsgCode > 0UL && epsgCode < KcsNmInvNumber)
					{
						wchar_t tmpBufr [128];
						int iTmp = (int)epsgCode;
						_itow (iTmp,tmpBufr,10);
						fieldData = tmpBufr;
						ok = crsWktCsv.ReplaceField (fieldData,recNbr,1,csvStatus);
					}
				}
				else
				{
					// It's not a GEOGCS or a PROJCS; mark it for delete.
					fieldData = L"9999.99";
					ok = crsWktCsv.ReplaceField (fieldData,recNbr,3,csvStatus);
				}
			}
		}
		if (ok && recNbr != 0)
		{
			if (curFlvr == prvFlvr)
			{
				// The guts of this whole mess.  The compare in the Sort Functor
				// is case insensitive.  So, we had beetr do the same here.
				if (CS_wcsicmp (curWkt.c_str(),prvWkt.c_str ()) == 0)
				{
					// We have a duplicate. We mark the previous record
					// for deletion by setting the release fiels to an
					// absurdly high number.  Then, if the current EPSG
					// number is zero, and the previous EPSG number is
					// non-zero, we update the EPSG number field of the
					// record we are keeping.  Thus, an EPSG number
					// equivalence will propogate through multiple
					// releases.
					if (curEpsg == 0 && prvEpsg != 0)
					{
						ok = crsWktCsv.GetField (fieldData,(recNbr - 1),1,csvStatus);
						if (ok)
						{
							ok = crsWktCsv.ReplaceField (fieldData,recNbr,1,csvStatus);
						}
					}
					// If we actually deleted the previous record here, things would
					// get nasty (i.e. record numbers and record count changes) and
					// since the TcsCsvFileBase is std::vector<> collection of
					// TcsCsvRecords, a deletion from the middle of the vector could
					// be incredibly slow.  We just mark the previous record for
					// deletion by setting the release field to an absurd value.
					if (ok)
					{
						fieldData = L"9999.99";
						ok = crsWktCsv.ReplaceField (fieldData,(recNbr - 1),3,csvStatus);
					}
					if (ok)
					{
						dupCount += 1;
					}
				}
			}
		}
		if (ok)
		{
			prvFlvr = curFlvr;
			prvEpsg = curEpsg;
			prvWKid = curWKid;
			prvRls  = curRls;
			prvWkt  = curWkt;
		}
	}
	if (ok)
	{
		std::wcout << L"Have located "
				   << dupCount
				   << L" duplicates."
				   << std::endl;
	}
	if (ok)
	{
		std::wcout << L"Starting final sort of "
				   << crsWktCsv.RecordCount () - dupCount
				   << L"records now."
				   << std::endl;
	}
	if (ok)
	{
		TcsCsvSortFunctor sortFunctor1 (0,2,3);
		ok = crsWktCsv.StableSort (sortFunctor1);
		if (!ok)
		{
			std::wcout << L"Final sort failed." << std::endl;
		}
	}

	std::wcout << L"Writing result file now."
			   << std::endl;
	if (ok)
	{
		// Create/truncate the result file.
		wcsncpy (fullPath,csDataTrgDir,MAXPATH);
		wcscat (fullPath,L"\\");
		wcscat (fullPath,L"WktDataCatalog.txt");
		oStrm.open (fullPath,std::ios_base::out | std::ios_base::trunc);
		ok = oStrm.is_open ();
		if (ok)
		{
			const TcsCsvRecord& labels = crsWktCsv.GetAllLabels ();
			labels.WriteToStream (oStrm,csvStatus,outDelimiters);
		}
	}
	if (ok)
	{
		// Wtite the reminder of the data file, skipping those records marked
		// for deletion.
		unsigned recNbr;
		for (recNbr = 0;ok && recNbr < crsWktCsv.RecordCount ();recNbr += 1)
		{
			double release;
			std::wstring fieldData;

			// This is problematical, as there is no convenient way to return
			// an error consition.
			ok = crsWktCsv.GetField (fieldData,recNbr,(short)3,csvStatus);
			if (ok)
			{
				release = wcstod (fieldData.c_str(),&wcPtr);
				if (release > 888.88)
				{
					continue;
				}
			}				

			TcsCsvRecord record;
			ok = crsWktCsv.GetRecord (record,recNbr,csvStatus);
			if (ok)
			{
				record.WriteToStream (oStrm,csvStatus,outDelimiters);
			}
		}
	}
	return (ok);
}
