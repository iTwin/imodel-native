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

#include "cs_map.h"
#include "csConsoleUtilities.hpp"
#include <iomanip>

extern "C" unsigned long KcsNmInvNumber;
extern "C" unsigned long KcsNmMapNoNumber;
extern "C" double cs_Zero;

void FormatStringW (std::wstring& toBeFixed,unsigned width);

bool csCrsNamesToSource (const wchar_t* csvPath,const wchar_t* csvCrsNames,const wchar_t* srcFileName)
{
	bool ok;

	short fldNbr;

	wchar_t fullPath [1024];

	std::wifstream iStrm;
	std::wofstream oStrm;

	std::wstring oldDatumName;
	std::wstring newDatumName;
	std::wstring oldCrsName;
	std::wstring newCrsName;
	std::wstring defaultStuff (L"    0UL,    0UL, \"\"                                   }, ");

	TcsCsvStatus csvStatus;
	TcsCsvFileBase crsNamesCsv (false,4,4);

	wcscpy (fullPath,csvPath);
	wcscat (fullPath,L"\\");
	wcscat (fullPath,csvCrsNames);
	iStrm.open (fullPath,std::ios_base::in);
	ok = iStrm.is_open ();
	if (ok)
	{
		ok = crsNamesCsv.ReadFromStream (iStrm,false,csvStatus);
		iStrm.close ();
	}
	
	if (ok)
	{
		wcscpy (fullPath,csvPath);
		wcscat (fullPath,L"\\");
		wcscat (fullPath,srcFileName);
		oStrm.open (fullPath,std::ios_base::out | std::ios_base::trunc);
		ok = oStrm.is_open ();
	}

	if (ok)
	{
		unsigned recCount = crsNamesCsv.RecordCount ();
		for (unsigned idx = 0;ok && idx < recCount;idx += 1)
		{
			fldNbr = 0;
			ok = crsNamesCsv.GetField (oldDatumName,idx,fldNbr,csvStatus);
			if (ok)
			{
				fldNbr = 1;
				ok = crsNamesCsv.GetField (newDatumName,idx,fldNbr,csvStatus);
			}
			if (ok)
			{
				fldNbr = 2;
				ok = crsNamesCsv.GetField (oldCrsName,idx,fldNbr,csvStatus);
			}
			if (ok)
			{
				fldNbr = 3;
				ok = crsNamesCsv.GetField (newCrsName,idx,fldNbr,csvStatus);
			}

			FormatStringW (oldDatumName,28);
			FormatStringW (newDatumName,28);
			FormatStringW (oldCrsName,28);
			FormatStringW (newCrsName,28);

			oStrm << L"  { "
				  << oldDatumName
				  << newDatumName
				  << oldCrsName
				  << newCrsName
				  << defaultStuff
				  << std::endl;
		}
		oStrm.close ();		
	}
	return ok;
}
void FormatStringW (std::wstring& toBeFixed,unsigned width)
{
	toBeFixed.insert (0,1,L'\"');
	toBeFixed.append (1,L'\"');
	toBeFixed.append (1,L',');
	while (toBeFixed.length () < width)
	{
		toBeFixed.append (1,L' ');
	}
	return;
}
