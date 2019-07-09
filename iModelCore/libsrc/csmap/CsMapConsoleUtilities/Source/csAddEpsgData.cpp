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

bool csAddEpsgCodes (TcsDefFile& dictionaryAsc);
bool csAddSridCodes (TcsDefFile& dictionaryAsc);
bool csAddEpsgQuadrant (TcsDefFile& dictionaryAsc,const TcsEpsgDataSetV6& epsgDataSet);

bool csAddEpsgCodes (const wchar_t* csDictSrcDir,const wchar_t* epsgDir,const wchar_t* csDictTrgDir)
{
	bool ok (false);

	int st;

	char csSrcAscFilePath [MAXPATH];
	char csTrgAscFilePath [MAXPATH];

	// Initialize CS-MAP, need this for the NameMapper.
	wcstombs (csSrcAscFilePath,csDictSrcDir,sizeof (csSrcAscFilePath));
	st = CS_altdr (csSrcAscFilePath);
	if (st != 0)
	{
		return ok;
	}

	// Open up the EPSG database.
	TcsEpsgDataSetV6 epsgDataSet (epsgDir,L"7.05");

	// Open up the Coordinate System ASCII definition file.
	wcstombs (csSrcAscFilePath,csDictSrcDir,sizeof (csSrcAscFilePath));
	strcat (csSrcAscFilePath,"\\coordsys.asc");
	TcsDefFile coordsysAsc (dictTypCoordsys,csSrcAscFilePath);

	// Open up the Datum ASCII definition file.
	wcstombs (csSrcAscFilePath,csDictSrcDir,sizeof (csSrcAscFilePath));
	strcat (csSrcAscFilePath,"\\datums.asc");
	TcsDefFile datumsAsc (dictTypDatum,csSrcAscFilePath);

	// Open up the Ellipsoid ASCII definition file.
	wcstombs (csSrcAscFilePath,csDictSrcDir,sizeof (csSrcAscFilePath));
	strcat (csSrcAscFilePath,"\\elipsoid.asc");
	TcsDefFile elipsoidAsc (dictTypEllipsoid,csSrcAscFilePath);

	// Add all known EPSG codes
	ok = csAddEpsgCodes (coordsysAsc);
	if (ok)
	{
		ok = csAddEpsgCodes (datumsAsc);
	}
	if (ok)
	{
		ok = csAddEpsgCodes (elipsoidAsc);
	}
	if (ok)
	{
		ok = csAddSridCodes (coordsysAsc);
	}
	if (ok)
	{
		ok = csAddEpsgQuadrant (coordsysAsc,epsgDataSet);
	}

	if (ok)
	{
		std::ofstream oStrm;

		wcstombs (csTrgAscFilePath,csDictTrgDir,sizeof (csTrgAscFilePath));
		strcat (csTrgAscFilePath,"\\coordsys.asc");
		oStrm.open (csTrgAscFilePath,std::ios_base::out | std::ios_base::trunc);
		ok = oStrm.is_open ();
		if (ok)
		{
			coordsysAsc.WriteToStream (oStrm);
			oStrm.close ();
		}
	}
	if (ok)
	{
		std::ofstream oStrm;

		wcstombs (csTrgAscFilePath,csDictTrgDir,sizeof (csTrgAscFilePath));
		strcat (csTrgAscFilePath,"\\datums.asc");
		oStrm.open (csTrgAscFilePath,std::ios_base::out | std::ios_base::trunc);
		ok = oStrm.is_open ();
		if (ok)
		{
			datumsAsc.WriteToStream (oStrm);
			oStrm.close ();
		}
	}
	if (ok)
	{
		std::ofstream oStrm;

		wcstombs (csTrgAscFilePath,csDictTrgDir,sizeof (csTrgAscFilePath));
		strcat (csTrgAscFilePath,"\\elipsoid.asc");
		oStrm.open (csTrgAscFilePath,std::ios_base::out | std::ios_base::trunc);
		ok = oStrm.is_open ();
		if (ok)
		{
			elipsoidAsc.WriteToStream (oStrm);
			oStrm.close ();
		}
	}
	return ok;
}

// Adds EPSG code to any CS-MAP definition which does not have one,
// using the NameMapper as a base.  We attempt to do this in a
// manner which will support eventual 'Diff'ing" of the source
// and target file.
bool csAddEpsgCodes (TcsDefFile& dictionaryAsc)
{
	bool ok (false);
	
	unsigned long epsgCode;
	unsigned long ascEpsgCode;

	EcsDictType dictType;
	EcsMapObjType codeType;

	char *dummy;
	const char *chPtr;
	TcsDefLine* defLinePtr;
	TcsAscDefinition* ascDefPtr;

	char chTemp [128];
	char defKeyName [cs_KEYNM_DEF];

	dictType = dictionaryAsc.GetDictType ();
	switch (dictType) {
	case dictTypCoordsys:
		codeType = csMapProjGeoCSys;
		break;
	case dictTypDatum:
		codeType = csMapDatumKeyName;
		break;
	case dictTypEllipsoid:
		codeType = csMapEllipsoidKeyName;
		break;
	default:
		codeType = csMapNone;
		break;
	}
	if (codeType == csMapNone)
	{
		return ok;
	}

	ok = true;
	size_t defCount = dictionaryAsc.GetDefinitionCount () - 1;
	for (unsigned index = 0;ok && index < defCount;index += 1)
	{
		ascDefPtr = &(*dictionaryAsc [index]);
		if (ascDefPtr == 0)
		{
			ok = false;
			break;
		}

		// Get the definition name.
		chPtr = ascDefPtr->GetDefinitionName ();
		if (chPtr == 0)
		{
			// Should only be the last enry in the file.  Maybe
			// this is bug in the TcsDefFile object.
			continue;
		}
		CS_stncp (defKeyName,chPtr,sizeof (defKeyName));

		// This definition does not have an EPSG code, see if the
		// NameMapper has one.
		epsgCode = csMapNameToIdC (codeType,csMapFlvrEpsg,csMapFlvrAutodesk,defKeyName);
		if (epsgCode <= KcsNmMapNoNumber || epsgCode >= KcsNmInvNumber)
		{
			// No EPSG code available, nothing we can do.
			continue;
		}

		// Get the current EPSG code value.
		defLinePtr = ascDefPtr->GetLine ("EPSG:");
		if (defLinePtr == 0)
		{
			bool insertOk;

			// There is no EPSG code line in the definition, we need to add one.
			sprintf (chTemp,"%ld",epsgCode);
			TcsDefLine newLine (dictType,"EPSG:",chTemp,0);
			insertOk = ascDefPtr->InsertAfter ("SOURCE:",newLine);
			if (!insertOk)
			{
				ok = ascDefPtr->InsertAfter ("DESC_NM:",newLine);
				if (!ok)
				{
					printf ("%s: EPSG insert failed.\n",defKeyName);
				}
			}
		}
		else
		{
			// There is an EPSG label, see if the value is correct.
			chPtr = defLinePtr->GetValue ();
			if (chPtr != 0)
			{
				ascEpsgCode = strtoul (chPtr,&dummy,10);
				if (ascEpsgCode == 0UL)
				{
					// Essentially, there is no EPSG code, simply replace the value
					// on the existing line.
					sprintf (chTemp,"%ld",epsgCode);
					defLinePtr->SetValue (chTemp);
				}
				else
				{
					// There is a value there.  If they match, all is well.
					// If not, notify the user.
					if (ascEpsgCode != epsgCode)
					{
						printf ("%s has incorrect EPSG code [%ld,%ld]\n",defKeyName,ascEpsgCode,epsgCode);
					}
				}
			}
		}
	}
	return ok;
}
// Adds SRID code to any CS-MAP definition which does not have one,
// using the NameMapper as a base.  We attempt to do this in a
// manner which will support eventual 'Diff'ing" of the source
// and target file.
bool csAddSridCodes (TcsDefFile& dictionaryAsc)
{
	bool ok (false);
	
	unsigned long sridCode;
	unsigned long ascSridCode;

	EcsDictType dictType;
	EcsMapObjType codeType;

	char *dummy;
	const char *chPtr;
	TcsDefLine* defLinePtr;
	TcsAscDefinition* ascDefPtr;

	char chTemp [128];
	char defKeyName [cs_KEYNM_DEF];

	dictType = dictionaryAsc.GetDictType ();
	switch (dictType) {
	case dictTypCoordsys:
		codeType = csMapProjGeoCSys;
		break;
	case dictTypDatum:
		codeType = csMapDatumKeyName;
		break;
	case dictTypEllipsoid:
		codeType = csMapEllipsoidKeyName;
		break;
	default:
		codeType = csMapNone;
		break;
	}
	if (codeType == csMapNone)
	{
		return ok;
	}

	ok = true;
	size_t defCount = dictionaryAsc.GetDefinitionCount () - 1;
	for (unsigned index = 0;ok && index < defCount;index += 1)
	{
		ascDefPtr = &(*dictionaryAsc [index]);
		if (ascDefPtr == 0)
		{
			ok = false;
			break;
		}

		// Get the definition name.
		chPtr = ascDefPtr->GetDefinitionName ();
		if (chPtr == 0)
		{
			// Should only be the last enry in the file.  Maybe
			// this is bug in the TcsDefFile object.
			continue;
		}
		CS_stncp (defKeyName,chPtr,sizeof (defKeyName));

		// This definition does not have an SRID code, see if the
		// NameMapper has one.
		sridCode = csMapNameToIdC (codeType,csMapFlvrOracle,csMapFlvrAutodesk,defKeyName);
		if (sridCode <= KcsNmMapNoNumber || sridCode >= KcsNmInvNumber)
		{
			// No Oracle SRID code, try Oracle 9
			sridCode = csMapNameToIdC (codeType,csMapFlvrOracle9,csMapFlvrAutodesk,defKeyName);
			if (sridCode <= KcsNmMapNoNumber || sridCode >= KcsNmInvNumber)
			{
				// No Oracle 9 code either, nothing we can do.
				continue;
			}
		}

		// Get the current EPSG code value.
		defLinePtr = ascDefPtr->GetLine ("SRID:");
		if (defLinePtr == 0)
		{
			bool insertOk;

			// There is no EPSG code line in the definition, we need to add one.
			sprintf (chTemp,"%ld",sridCode);
			TcsDefLine newLine (dictType,"SRID:",chTemp,0);
			insertOk = ascDefPtr->InsertAfter ("EPSG:",newLine);
			if (!insertOk)
			{
				insertOk = ascDefPtr->InsertAfter ("SOURCE:",newLine);
				if (!insertOk)
				{
					ok = ascDefPtr->InsertAfter ("DESC_NM:",newLine);
					if (!ok)
					{
						printf ("%s: SRID insert failed.\n",defKeyName);
					}
				}
			}
		}
		else
		{
			// There is an EPSG label, see if the value is correct.
			chPtr = defLinePtr->GetValue ();
			if (chPtr != 0)
			{
				ascSridCode = strtoul (chPtr,&dummy,10);
				if (ascSridCode == 0UL)
				{
					// Essentially, there is no EPSG code, simply replace the value
					// on the existing line.
					sprintf (chTemp,"%ld",sridCode);
					defLinePtr->SetValue (chTemp);
				}
				else
				{
					// There is a value there.  If they match, all is well.
					// If not, notify the user.
					if (ascSridCode != sridCode)
					{
						printf ("%s has incorrect SRID code [%ld,%ld]\n",defKeyName,ascSridCode,sridCode);
					}
				}
			}
		}
	}
	return ok;
}
bool csAddEpsgQuadrant (TcsDefFile& dictionaryAsc,const TcsEpsgDataSetV6& epsgDataSet)
{
	bool ok (false);

	short epsgQuad;
	
	TcsEpsgCode horzUom;
	TcsEpsgCode vertUom;	
	unsigned long ascEpsgCode;

	char *dummy;
	const char *chPtr;
	TcsDefLine* defLinePtr;
	TcsDefLine* quadLinePtr;
	TcsAscDefinition* ascDefPtr;

	EcsDictType dictType;

	char chTemp [128];
	char defKeyName [cs_KEYNM_DEF];

	ok = true;
	dictType = dictionaryAsc.GetDictType ();
	size_t defCount = dictionaryAsc.GetDefinitionCount () - 1;
	for (unsigned index = 0;ok && index < defCount;index += 1)
	{
		ascDefPtr = &(*dictionaryAsc [index]);
		if (ascDefPtr == 0)
		{
			ok = false;
			break;
		}

		// Get the definition name, for possible error message.
		chPtr = ascDefPtr->GetDefinitionName ();
		if (chPtr == 0)
		{
			// Should only be the last enry in the file.  Maybe
			// this is bug in the TcsDefFile object.
			continue;
		}
		CS_stncp (defKeyName,chPtr,sizeof (defKeyName));

		// Get the current EPSG code value.
		defLinePtr = ascDefPtr->GetLine ("EPSG:");
		if (defLinePtr == 0)
		{
			// No EPSG code, no EPSG quadrant.
			continue;
		}
		else
		{
			// There is an EPSG label, get the code value.
			chPtr = defLinePtr->GetValue ();
			if (chPtr != 0)
			{
				bool insertOk;

				ascEpsgCode = strtoul (chPtr,&dummy,10);
				if (ascEpsgCode == 0UL)
				{
					// No EPSG code, no EPSG quadrant.
					continue;
				}
				else
				{
					// There is a value there.
					TcsEpsgCode epsgCode (ascEpsgCode);
					ok = epsgDataSet.GetCoordsysQuad (epsgQuad,horzUom,vertUom,epsgCode);
					if (ok)
					{
						// We have an Epsg Quadrant value in the CS-MAP syntax.  See if
						// this definition already has an EPSG_QD specification.
						quadLinePtr = ascDefPtr->GetLine ("EPSG_QD:");
						if (quadLinePtr != 0)
						{
							// Yup! There is a EPSG Quad specification. So we will not
							// add one.  But we will verify that it is the same as the
							// the value we would add.  If this fails, we issue a message.
							chPtr = quadLinePtr->GetValue ();
							short csmapEpsgCode = static_cast<short>(strtoul (chPtr,&dummy,10));
							if (csmapEpsgCode != epsgQuad)
							{
								printf ("Regarding %s: Current EPSG code value is %d, .ASC file value is %d.\n",defKeyName,epsgQuad,csmapEpsgCode);
							}
							continue;
						}
					}
					// If we're still here, and all is OK, we add the EPSG_QD entry to
					// the current definition.
					if (ok)
					{
						sprintf (chTemp,"%d",epsgQuad);
						TcsDefLine newLine (dictType,"EPSG_QD:",chTemp,0);
						// Add after any QUAD: specification which may already exist.
						insertOk = ascDefPtr->InsertAfter ("QUAD:",newLine);
						if (!insertOk)
						{
							// There is no QUAD:, try inserting after MAP_SCL:
							insertOk = ascDefPtr->InsertBefore ("MAP_SCL:",newLine);
						}
						if (!insertOk)
						{
							// There is no MAP_SCL: either. SImply append to the end.
							insertOk = ascDefPtr->Append (newLine);
						}
					}
				}
			}
		}
	}
	return ok;
}
