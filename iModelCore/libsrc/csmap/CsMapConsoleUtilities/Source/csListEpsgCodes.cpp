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

bool ListEpsgCodes (const wchar_t* dictDir)
{
	int st;
	unsigned epsgIndex;
	unsigned long epsgCode;
	enum EcsMapObjType objType;
	
	const wchar_t* adskName;
	const wchar_t* epsgName;

	char dictDirC [1024];
	
	wcstombs (dictDirC,dictDir,sizeof (dictDirC));
	st = CS_altdr (dictDirC);
	if (st != 0)
	{
		std::wcout << L"Dictionary directory specification is in error." << std::endl;
		return false;
	}

	std::wcout << L"EPSG Code" << L"," << L"Autodesk Key Name" << L"," << L"EPSG Name" << std::endl;

	epsgIndex = 0;
	objType = csMapGeographicCSysKeyName;
	for (;;)
	{
		// Loop once for each EPSG code in the name mapper database.
		epsgCode = csGetIdsByIdx (objType,csMapFlvrEpsg,epsgIndex++);
		if (epsgCode == KcsNmInvNumber)
		{
			if (objType != csMapGeographicCSysKeyName)
			{
				// We're done.
				break;
			}
			
			// Done with the geographics.  Do the Projectives.
			objType = csMapProjectedCSysKeyName;
			epsgIndex = 0;
			continue;
		}
		if (epsgCode == KcsNmMapNoNumber)
		{
			//This entry did not have an ID of the indicated type???
			continue;
		}
		
		// We have an EPSG code number.  Extract the Autodesk Name and
		// the EPSG name and print them out to wcout.
		adskName = csMapIdToName (objType,csMapFlvrAutodesk,csMapFlvrEpsg,epsgCode);
		epsgName = csMapIdToName (objType,csMapFlvrEpsg,csMapFlvrEpsg,epsgCode);
		if (adskName != 0 && *adskName != L'\0' && epsgName != 0 && *epsgName != L'\0')
		{
			std::wcout << epsgCode << L",\"" << adskName << L"\",\"" << epsgName << L"\"" << std::endl;
		}
	}
	csReleaseNameMapper ();
	return true;
}
