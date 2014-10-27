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

bool ListUnmappedEpsgCodes (const wchar_t* epsgDir,const wchar_t* dictDir)
{
	bool ok (false);
	bool deprecated (false);

	unsigned recCount;
	unsigned epsgIndex;
	TcsEpsgCode epsgCode;
	TcsEpsgCode areaCode;

	const wchar_t* wcPtr;
	std::wstring fldData;
	std::wstring epsgName;
	std::wstring epsgArea;	

	char dictDirC [1024];
	
	wcstombs (dictDirC,dictDir,sizeof (dictDirC));
	int st = CS_altdr (dictDirC);
	if (st != 0)
	{
		std::wcout << L"Dictionary directory specification is in error." << std::endl;
		return false;
	}

	try
	{
		TcsEpsgDataSetV6 epsgDataSet (epsgDir,L"7.01");

		// For now, all we'll be doing is the coordinate reference systems.
		const TcsEpsgTable* crsTblPtr = epsgDataSet.GetTablePtr (epsgTblReferenceSystem);
		const TcsEpsgTable* areaTblPtr = epsgDataSet.GetTablePtr (epsgTblArea);
		recCount = crsTblPtr->RecordCount ();
		std::wcout << L"EPSG Code,EPSG Name,Area of Use" << std::endl;
		
		// Loop once for each entry in the table.
		for (epsgIndex = 0;epsgIndex < recCount;epsgIndex += 1)
		{
			deprecated = crsTblPtr->IsDeprecated (epsgIndex);
			if (deprecated)
			{
				continue;
			}
			crsTblPtr->GetField (fldData,epsgIndex,epsgFldCoordRefSysKind);
			EcsCrsType crsType = GetEpsgCrsType (fldData.c_str ());
			if (crsType != epsgCrsTypProjected)
			{
				continue;
			}
			
			// It's a projected type.  Get the EPSG code and name, and then see if
			// the NameMapper has a mapping for it.
			ok = crsTblPtr->GetAsEpsgCode (epsgCode,epsgIndex,epsgFldCoordRefSysCode);
			// See if this EPSG code maps to an Autodesk name.
			if (ok)
			{
				wcPtr = csMapIdToName (csMapProjectedCSysKeyName,csMapFlvrAutodesk,csMapFlvrEpsg,epsgCode);
				if (wcPtr != 0)
				{
					continue;
				}
			}

			// This is a projected coordinate system, and there is no mapping
			// for this EPSG code value.
			if (ok)
			{
				ok = crsTblPtr->GetAsEpsgCode (areaCode,epsgIndex,epsgFldAreaOfUseCode);
			}
			if (ok)
			{
				ok = crsTblPtr->GetField (epsgName,epsgIndex,epsgFldCoordRefSysName);
			}
			if (ok)
			{
				ok = areaTblPtr->GetField (epsgArea,areaCode,epsgFldAreaName);
			}
			if (ok)
			{
				csCsvQuoter (epsgName);
				csCsvQuoter (epsgArea);
				std::wcout	<< epsgCode << L','
							<< epsgName << L','
							<< epsgArea << std::endl;
			}
		}
	}
	catch (...)
	{
		ok = false;
	}
	csReleaseNameMapper ();
	return ok;
}
