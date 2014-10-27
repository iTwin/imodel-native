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

bool ManufactureNameMapperCsv (const wchar_t* resultDir,const wchar_t* dataDir)
{
	bool ok (true);
	bool duplicates (true);
	wchar_t* wcPtr;
	wchar_t filePath [2048];
	TcsNameMapper nameMapper;

	wcsncpy (filePath,dataDir,wcCount (filePath));
	wcPtr = filePath + wcslen (filePath) - 1;
	if (*wcPtr != L'\\' && *wcPtr != L'/')
	{
		++wcPtr;
		*wcPtr++ = L'\\';
	}

	nameMapper.SetRecordDuplicates (duplicates);

	// Add Parameter Name mapping
	if (ok)
	{
		wcscpy (wcPtr,L"ParameterKeyNameMap.csv");	
		ok = nameMapper.AddKeyNameMap (csMapParameterKeyName,filePath);
	}

	// Add Projection Name mapping
	if (ok)
	{
		wcscpy (wcPtr,L"ProjectionKeyNameMap.csv");	
		ok = nameMapper.AddKeyNameMap (csMapProjectionKeyName,filePath);
	}

	// Add Linear Unit mapping
	if (ok)
	{
		wcscpy (wcPtr,L"LinearUnitKeyNameMap.csv");	
		ok = nameMapper.AddKeyNameMap (csMapLinearUnitKeyName,filePath);
	}

	// Add Angular Unit mapping
	if (ok)
	{
		wcscpy (wcPtr,L"AngularUnitKeyNameMap.csv");	
		ok = nameMapper.AddKeyNameMap (csMapAngularUnitKeyName,filePath);
	}

	// Add Ellipsoid Key Name mapping
	if (ok)
	{
		wcscpy (wcPtr,L"EllipsoidKeyNameMap.csv");	
		ok = nameMapper.AddKeyNameMap (csMapEllipsoidKeyName,filePath);
	}

	// Add Datum Key Name mapping
	if (ok)
	{
		wcscpy (wcPtr,L"DatumKeyNameMap.csv");	
		ok = nameMapper.AddKeyNameMap (csMapDatumKeyName,filePath);
	}

	// Add Geographic Coordinate System (2D) Key Name mapping
	if (ok)
	{
		wcscpy (wcPtr,L"GeographicKeyNameMap.csv");	
		ok = nameMapper.AddKeyNameMap (csMapGeographicCSysKeyName,filePath);
	}

	// Add Projective Coordinate System Key Name mapping
	if (ok)
	{
		wcscpy (wcPtr,L"ProjectiveKeyNameMap.csv");	
		ok = nameMapper.AddKeyNameMap (csMapProjectedCSysKeyName,filePath);
	}

	// Add 3D Geographic Coordinate System Key Name mapping
	if (ok)
	{
		wcscpy (wcPtr,L"Geographic3DKeyNameMap.csv");	
		ok = nameMapper.AddKeyNameMap (csMapGeographic3DKeyName,filePath);
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
