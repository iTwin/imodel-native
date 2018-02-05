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

extern "C" unsigned long KcsNmInvNumber;
extern "C" unsigned long KcsNmMapNoNumber;

bool OracleTxt2WktTest (const wchar_t* csDictDir,const wchar_t* csDataDir,const wchar_t* inputFile,
																		  const wchar_t* delimiters,
																		  const wchar_t* oracleVersion)
{
	bool ok (false);

	char csDictDirC [MAXPATH];
	wchar_t filePath [MAXPATH + MAXPATH];

	std::wofstream oStrm;

	TcsCsvStatus csvStatus;
	TcsCsvFileBase oracleDataIn (false,3,5,delimiters);

	wcstombs (csDictDirC,csDictDir,sizeof (csDictDirC));
	int status = CS_altdr (csDictDirC);
	ok = (status == 0);
	if (!ok)
	{
		return false;
	}

	const TcsEpsgDataSetV6* epsgPtr = GetEpsgObjectPtr ();
	ok = (epsgPtr != 0);
	if (!ok)
	{
		return ok;
	}
	std::wcout << L"Using EPSG Database, revision "
		  << epsgPtr->GetRevisionLevel ()
		  << std::endl;

	// Open source file
	wcsncpy (filePath,csDataDir,MAXPATH);
	wcscat (filePath,L"\\");
	wcscat (filePath,inputFile);
	std::wifstream iStrm (filePath,std::ios_base::in);
	ok = iStrm.is_open ();
	if (ok)
	{
		// Create/Truncate the output file.
		wcsncpy (filePath,csDataDir,MAXPATH);
		wcscat (filePath,L"\\");
		wcscat (filePath,L"OracleWktTest.txt");
		oStrm.open (filePath,std::ios_base::out | std::ios_base::app);
		ok = oStrm.is_open ();
	}

	// Read the input file.
	if (ok)
	{
		ok = oracleDataIn.ReadFromStream (iStrm,false,csvStatus);
		if (ok)
		{
			iStrm.close ();
		}
	}

	// Process each line in the file.
	if (ok)
	{
		bool recOk;
		TcsEpsgCode epsgCode;
		unsigned long oracleCode;
		const wchar_t* wcPtr;
		wchar_t* dummy;
		std::wstring epsgName;
		std::wstring oracleName;
		std::wstring csvField;
		std::wstring oracleWkt;

		for (unsigned idx = 0;ok && idx < oracleDataIn.RecordCount ();idx += 1)
		{
			short outFlavor = 3;		// Oracle, since release 10.

			// Get the Oracle name.
			recOk = false;
			ok = oracleDataIn.GetField (oracleName,idx,(short)0,csvStatus);
			if (!ok)
			{
				continue;
			}
			if (oracleName.find (L"height") != oracleName.npos)
			{
				std::wcout << L"Ignoring Oracle entry named "
						   << oracleName
						   << L"."
						   << std::endl;
				continue;
			}
			if (oracleName.find (L"depth") != oracleName.npos)
			{
				std::wcout << L"Ignoring Oracle entry named "
						   << oracleName
						   << L"."
						   << std::endl;
				continue;
			}

			// Get the Wkt string.
			recOk = true;
			ok = oracleDataIn.GetField (oracleWkt,idx,2,csvStatus);
			if (!ok)
			{
				continue;
			}
			if (oracleWkt.length () < 24)
			{
				std::wcout << L"Ignoring Oracle entry named "
						   << oracleName
						   << L"; no WKT definition provided."
						   << std::endl;
				continue;
			}

			// If the WKT field is quoted, unquote it.  Note, we know that the
			// WKT string is not empty at this point.
			{
				wchar_t firstChar;
				wchar_t lastChar;
				firstChar = oracleWkt.at(0);
				lastChar = oracleWkt.at(oracleWkt.length() - 1);
				if (firstChar == L'"' && lastChar == L'"')
				{
					// Unquote the WKT string.
					oracleWkt.erase(oracleWkt.length()-1,1);
					oracleWkt.erase(0,1);
				}
			}

			// Get the SRID field.
			ok = oracleDataIn.GetField (csvField,idx,1,csvStatus);
			oracleCode = wcstoul (csvField.c_str (),&dummy,10);

			//  See if it is an Oracle 9 SRID.
			wcPtr = csMapIdToName (csMapProjGeoCSys,csMapFlvrOracle9,csMapFlvrOracle9,oracleCode);
			if (wcPtr != NULL)
			{
				// NameMapper says its an Oracle9.
				outFlavor = 14;
				epsgCode = csMapIdToId (csMapProjGeoCSys,csMapFlvrEpsg,csMapFlvrOracle9,oracleCode);
				if (epsgCode ==  KcsNmInvNumber)
				{
					epsgCode.Invalidate ();
				}
			}
			else
			{
				// Else, process as a Release 10+ record.
				epsgCode = csMapIdToId (csMapProjGeoCSys,csMapFlvrOracle,csMapFlvrEpsg,oracleCode);
				if (epsgCode == KcsNmInvNumber)
				{
					epsgCode.Invalidate ();
				}
			}

			// If the epsgCode is invalid, and the SSRID code is in the range
			// of a valid EPSG code, we see if there is a an EPSG entry with
			// the same ID and name as the Oracle string.  If so, we assume
			// the the SRID is a valid EPSG code.
			if (epsgCode.IsNotValid () && oracleCode < 32768 && oracleName.length () > 6)
			{
				bool lclOk;
				
				lclOk = epsgPtr->GetFieldByCode (epsgName,epsgTblReferenceSystem,epsgFldCoordRefSysName,oracleCode);
				if (lclOk)
				{
					const wchar_t* epsgPtr;
					const wchar_t* oraclePtr;
					epsgPtr = epsgName.c_str ();
					oraclePtr = oracleName.c_str ();
					if (!wcsicmp (epsgPtr,oraclePtr))
					{
						epsgCode = oracleCode;
					}
					else
					{
					std::wcout << L"Name matching failure: "
							   << epsgName
							   << L" != "
							   << oracleName
							   << L'.'
							   << std::endl;
					}
				}
			}
			if (recOk)
			{
				oStrm << outFlavor
					  << L'|'
					  << static_cast<unsigned long>(epsgCode)
					  << L'|'
					  << oracleCode
					  << L'|'
					  << oracleVersion
					  << L'|'
					  << oracleWkt
					  << std::endl;
			}
		}
		oStrm.close ();
	}
	return ok;
}
