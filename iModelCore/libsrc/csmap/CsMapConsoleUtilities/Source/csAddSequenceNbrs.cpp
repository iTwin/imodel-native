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

bool SequenceKeyNameMap (EcsMapObjType objType,const wchar_t* filePath);

bool AddSequenceNumbers (const wchar_t* dataDir)
{
	bool ok (true);
	wchar_t* wcPtr;
	wchar_t filePath [260];
	TcsNameMapper nameMapper;

	wcsncpy (filePath,dataDir,wcCount (filePath));
	wcPtr = filePath + wcslen (filePath) - 1;
	if (*wcPtr != L'\\' && *wcPtr != L'/')
	{
		++wcPtr;
		*wcPtr++ = L'\\';
	}

	if (ok)
	{
		wcscpy (wcPtr,L"ParameterKeyNameMap.csv");
		ok = SequenceKeyNameMap (csMapParameterKeyName,filePath);
	}
	if (ok)
	{
		wcscpy (wcPtr,L"ProjectionKeyNameMap.csv");	
		ok = SequenceKeyNameMap (csMapProjectionKeyName,filePath);
	}
	if (ok)
	{
		wcscpy (wcPtr,L"LinearUnitKeyNameMap.csv");	
		ok = SequenceKeyNameMap (csMapLinearUnitKeyName,filePath);
	}
	if (ok)
	{
		wcscpy (wcPtr,L"AngularUnitKeyNameMap.csv");	
		ok = SequenceKeyNameMap (csMapAngularUnitKeyName,filePath);
	}
	if (ok)
	{
		wcscpy (wcPtr,L"EllipsoidKeyNameMap.csv");	
		ok = SequenceKeyNameMap (csMapEllipsoidKeyName,filePath);
	}
	if (ok)
	{
		wcscpy (wcPtr,L"DatumKeyNameMap.csv");	
		ok = SequenceKeyNameMap (csMapDatumKeyName,filePath);
	}
	if (ok)
	{
		wcscpy (wcPtr,L"GeographicKeyNameMap.csv");	
		ok = SequenceKeyNameMap (csMapGeographicCSysKeyName,filePath);
	}
	if (ok)
	{
		wcscpy (wcPtr,L"ProjectiveKeyNameMap.csv");	
		ok = SequenceKeyNameMap (csMapProjectedCSysKeyName,filePath);
	}
	if (ok)
	{
		wcscpy (wcPtr,L"Geographic3DKeyNameMap.csv");	
		ok = SequenceKeyNameMap (csMapGeographic3DKeyName,filePath);
	}
	return ok;
}

bool SequenceKeyNameMap (EcsMapObjType objType,const wchar_t* filePath)
{
	bool ok (false);
	unsigned long seqNbr;
	TcsCsvStatus csvStatus;

	TcsKeyNameMapFile keyNameMap (filePath,28);

	if (keyNameMap.RecordCount () == 0)
	{
		// Empty file is not an error.
		return true;
	}
	keyNameMap.Rewind ();

	seqNbr = 0;
	do
	{
		seqNbr += 100;
		ok = keyNameMap.ReplaceField (csMapFldSeqNbr,seqNbr);
	} while (ok && keyNameMap.NextRecord ());

	if (ok)
	{
		std::wofstream oStrm;
		oStrm.open (filePath,std::ios_base::out | std::ios_base::trunc);
		if (oStrm.is_open ())
		{
			keyNameMap.SetFieldLabel (L"SeqNbr",0);		
			keyNameMap.WriteToStream (oStrm,true,csvStatus);
			oStrm.close ();
		}
	}
	return ok;
}
