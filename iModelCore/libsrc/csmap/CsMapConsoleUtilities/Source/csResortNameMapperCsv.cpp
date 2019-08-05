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

bool ResortNameMapperCsv (const wchar_t* resultDir,const wchar_t* dataDir,bool duplicates)
{
	bool ok (true);
	
	EcsCsvStatus readStatus;
	
	wchar_t* wcPtr;
	wchar_t filePath [2048];
	TcsCsvStatus csvStatus;
	TcsNameMapper nameMapper;

	if (dataDir == NULL || *dataDir == L'\0')
	{
		// Keep the following from crashing in the event of bogus parameters.
		return false;
	}

	wcsncpy (filePath,dataDir,wcCount (filePath));
	wcPtr = filePath + wcslen (filePath);
	if (*(wcPtr - 1) != L'\\' && *(wcPtr - 1) != L'/')
	{
		*wcPtr++ = L'\\';
	}
	wcscpy (wcPtr,L"NameMapper.csv");	
	
	nameMapper.SetRecordDuplicates (duplicates);

	std::wifstream iStrm (filePath,std::ios_base::in);
	if (iStrm.is_open ())
	{
		readStatus = nameMapper.ReadFromStream (iStrm,csvStatus);
		ok = (readStatus == csvOk);
		iStrm.close ();
	}

	if (ok)
	{
		wcsncpy (filePath,resultDir,wcCount (filePath));
		wcPtr = filePath + wcslen (filePath) - 1;
		if (*wcPtr != L'\\' && *wcPtr != L'/')
		{
			++wcPtr;
			*wcPtr++ = L'\\';
		}
		wcscpy (wcPtr,L"NameMapper.csv");	

		std::wofstream oStrm (filePath,std::ios_base::out | std::ios_base::trunc);
		if (oStrm.is_open ())
		{
			nameMapper.WriteAsCsv (oStrm,true);
			oStrm.close ();

			if (duplicates)
			{
				wcscpy (wcPtr,L"NameMapperDuplicates.csv");	
				oStrm.open (filePath,std::ios_base::out | std::ios_base::trunc);
				if (oStrm.is_open ())
				{
					nameMapper.WriteDuplicates (oStrm);
					oStrm.close ();
				}
			}
		}
	}
	return ok;
}
