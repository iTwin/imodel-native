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

#include "csTestCpp.hpp"

extern "C"
{
	extern "C" int cs_Error;
	extern "C" int cs_Errno;
	extern "C" int csErrlng;
	extern "C" int csErrlat;
	extern "C" unsigned short cs_ErrSup;

	#if _RUN_TIME <= _rt_UNIXPCC
	extern "C" ulong32_t cs_Doserr;
	#endif
}

extern wchar_t const csEpsgDir [];

unsigned long KcsDeprecatedCRSs [] =
{
//	    0UL,			// Uncomment this line to turn this feature off.
	 2492UL,			// Deprecated by EPSG at 6.15, not replaced by
						// anything, CS_MAP equivalent remains undeprecated.
	 2493UL,			// Deprecated by EPSG at 6.15, not replaced by
						// anything, CS_MAP equivalent remains undeprecated.
	21817UL,			// Deprecated by EPSG at 6.7, not replaced by
						// anything, CS_MAP equivalent remains undeprecated.
	23853UL,			// Deprecated by EPSG at 6.7, not replaced by
						// anything, CS_MAP equivalent remains undeprecated.
	23886UL,			// Deprecated by EPSG at 6.7, not replaced by
						// anything, CS_MAP equivalent remains undeprecated.
	    0UL				// Terminates this table.
};

int CStestM (const TcsEpsgDataSetV6& epsgV6,bool verbose,long32_t duration)
{
	bool ok;
	bool deprecated;		// current EPSG definition is deprecated
	bool legacy;			// current CS-MAP definition is deprecated.

	enum EcsMapSt csMapSt;
	EcsCrsType crsType;

	TcsEpsgCode epsgCode;
	TcsEpsgCode dtmCode;
	TcsEpsgCode epsgCodeBack;

	struct cs_Eldef_ *csMapElDef;
	struct cs_Dtdef_ *csMapDtDef;
	struct cs_Csdef_ *csMapCsDef;

	char csMapKeyName [32];
	char epsgKeyName [128];
	char mapEpsgName [128];

	std::wstring fldData;

	int idx;					// loop index variable
	int errCnt = 0;				// total number of errors detected.
	int wrnCnt = 0;				// total number of warnings detected (warning == noMap)

	int failedCnt = 0;			// number failed due to EPSG access
	int mapCnt = 0;				// number of EPSG ellipsoids which did not map to CS-MAP
	int missingCnt = 0;			// number of missing CS-MAP definitions to which an EPSG code maps
	int deprCnt = 0;			// number of deprecation mismatches.
	int nameCnt = 0;            // number of mismatched name entries.
	int okCnt = 0;				// number of successful name maps.
	printf ("[ M]Auditing the NameMapper table\n");

	unsigned recordCount = epsgV6.GetRecordCount (epsgTblEllipsoid);
	for (unsigned index = 0;index < recordCount;++index)\
	{
		csMapKeyName [0] = '\0';
		epsgKeyName [0] = '\0';
		mapEpsgName [0] = '\0';
		
		ok = epsgV6.GetCodeByIndex (epsgCode,epsgTblEllipsoid,epsgFldEllipsoidCode,index);
		if (!ok)
		{
			failedCnt += 1;
			continue;
		}
		ok = epsgV6.GetFieldByCode (fldData,epsgTblEllipsoid,epsgFldEllipsoidName,epsgCode);
		if (!ok)
		{
			failedCnt += 1;
			continue;
		}
		wcstombs (epsgKeyName,fldData.c_str (),sizeof (epsgKeyName));
		epsgKeyName [sizeof (epsgKeyName) - 1] = '\0';

		deprecated = epsgV6.IsDeprecated (epsgTblEllipsoid,epsgCode);
		csMapSt = csMapIdToNameC (csMapEllipsoidKeyName,csMapKeyName,sizeof (csMapKeyName),
																	 csMapFlvrAutodesk,
																	 csMapFlvrEpsg,
																	 epsgCode);
		if (csMapSt != csMapOk)
		{
			if (!deprecated)
			{
				mapCnt += 1;
				printf ("EPSG ellipsoid '%s' [%lu] not mapped to a CS-MAP ellipsoid.\n",
						 epsgKeyName,
						 static_cast<unsigned long>(epsgCode));
			}
			continue;
		}
		else
		{
			csMapElDef = CS_eldef (csMapKeyName);
			if (csMapElDef == 0)
			{
				missingCnt += 1;
				printf ("EPSG ellipsoid '%s' [%lu] maps to CS-MAP '%s'which does _NOT_ exist in the dictionary.\n",
						 epsgKeyName,
						 static_cast<ulong32_t>(epsgCode),
						 csMapKeyName);
			}
			else
			{
				legacy = (CS_stricmp (csMapElDef->group,"LEGAC") == 0);
				if (deprecated && !legacy)
				{
					deprCnt += 1;
					printf ("Deprecated EPSG ellipsoid '%s' [%lu] maps to CS-MAP '%s' which is _NOT_ in the LEGACY group.\n",
							 epsgKeyName,
							 static_cast<ulong32_t>(epsgCode),
							 csMapKeyName);
				}
				else if (legacy && !deprecated)
				{
					deprCnt += 1;
					printf ("EPSG ellipsoid '%s' [%lu] maps to CS-MAP '%s', but '%s' is in the LEGACY group.\n",
							 epsgKeyName,
							 static_cast<ulong32_t>(epsgCode),
							 csMapKeyName,
							 csMapKeyName);
				}
				// Verify that the dictionary has the appropriate EPSG code.
				if (static_cast<long>(csMapElDef->epsgNbr) != static_cast<long>(epsgCode))
				{
					mapCnt += 1;
					printf ("EPSG code which appears in definition of ellipsoid named '%s' does not match current EPSG database.\n",csMapKeyName);
				}
				okCnt += 1;
				CS_free (csMapElDef);
			}
		}

		// Verify that the Name Mapper has the same name as the EPSG database.
		if (!deprecated)
		{
			csMapSt = csMapIdToNameC (csMapEllipsoidKeyName,mapEpsgName,sizeof (mapEpsgName),
																	    csMapFlvrEpsg,
																	    csMapFlvrEpsg,
																	    epsgCode);
			if (csMapSt != csMapOk || CS_stricmp (epsgKeyName,mapEpsgName) != 0)
			{
				nameCnt += 1;
				printf ("Name Mapper name [%s] not the same as EPSG database name for ellipsoid '%s' [%lu].\n",
						mapEpsgName, 
						epsgKeyName,
						static_cast<unsigned long>(epsgCode));
			}
		}
	}
	printf ("\tEllipsoid Audit: ok = %d, noMap = %d, missingDef = %d, deprecationErr = %d, nameDiff = %d, EPSGaccess = %d\n",okCnt,mapCnt,missingCnt,deprCnt,nameCnt,failedCnt);
	errCnt += (failedCnt + missingCnt + deprCnt);
	wrnCnt += mapCnt;

	failedCnt = 0;			// number failed due to EPSG access
	mapCnt = 0;				// number of EPSG datums which did not map to CS-MAP
	missingCnt = 0;			// number of missing CS-MAP definitions to which an EPSG code maps
	deprCnt = 0;			// number of deprecation mismatches.
	nameCnt = 0;            // number of names which don't match
	okCnt = 0;				// number of successful name maps.
	recordCount = epsgV6.GetRecordCount (epsgTblDatum);
	for (unsigned index = 0;index < recordCount;++index)
	{
		csMapKeyName [0] = '\0';
		epsgKeyName [0] = '\0';
		mapEpsgName [0] = '\0';

		ok = epsgV6.GetCodeByIndex (epsgCode,epsgTblDatum,epsgFldDatumCode,index);
		if (!ok)
		{
			failedCnt += 1;
			continue;
		}
		
		// We only do EPSG datums of the "geodetic" type.
		ok = epsgV6.GetFieldByCode (fldData,epsgTblDatum,epsgFldDatumType,epsgCode);
		if (ok)
		{
			EcsDtmType dtmType = GetEpsgDtmType (fldData.c_str ());
			if (dtmType != epsgDtmTypGeodetic)
			{
				continue;
			}
		}

		// We skip the EPSG datums which are actually not datums, just references
		// to an ellipsoid.
		if (epsgCode >= 6001UL && epsgCode <= 6054UL)
		{
			continue;
		}
		
		ok = epsgV6.GetFieldByCode (fldData,epsgTblDatum,epsgFldDatumName,epsgCode);
		if (!ok)
		{
			failedCnt += 1;
			continue;
		}
		wcstombs (epsgKeyName,fldData.c_str (),sizeof (epsgKeyName));
		epsgKeyName [sizeof (epsgKeyName) - 1] = '\0';

		deprecated = epsgV6.IsDeprecated (epsgTblDatum,epsgCode);
		csMapSt = csMapIdToNameC (csMapDatumKeyName,csMapKeyName,sizeof (csMapKeyName),
																 csMapFlvrAutodesk,
																 csMapFlvrEpsg,
																 epsgCode);
		if (csMapSt != csMapOk)
		{
			if (!deprecated)
			{
				mapCnt += 1;
				if (verbose)
				{
					// There's a ton of datums which we don't support.  So for
					// now we list these only if the verbose command line
					// is present.
					printf ("EPSG datum '%s' [%lu] not mapped to a CS-MAP datum.\n",
							 epsgKeyName,
							 static_cast<unsigned long>(epsgCode));
				}
			}
			continue;
		}
		else
		{
			csMapDtDef = CS_dtdef (csMapKeyName);
			if (csMapDtDef == 0)
			{
				missingCnt += 1;
				printf ("EPSG datum '%s' [%lu] maps to CS-MAP '%s' which does _NOT_ exist in the dictionary.\n",
						 epsgKeyName,
						 static_cast<ulong32_t>(epsgCode),
						 csMapKeyName);
			}
			else
			{
				legacy = (CS_stricmp (csMapDtDef->group,"LEGACY") == 0);
				if (deprecated && !legacy)
				{
					deprCnt += 1;
					printf ("Deprecated EPSG datum '%s' [%lu] maps to CS-MAP '%s' which is _NOT_ in the LEGACY group.\n",
							 epsgKeyName,
							 static_cast<ulong32_t>(epsgCode),
							 csMapKeyName);
				}
				else if (legacy && !deprecated)
				{
					deprCnt += 1;
					printf ("EPSG datum '%s' [%lu] maps to CS-MAP '%s' which is in the LEGACY group.\n",
							 epsgKeyName,
							 static_cast<ulong32_t>(epsgCode),
							 csMapKeyName);
				}
				// Verify that the dictionary has the appropriate EPSG code.
				if (static_cast<long>(csMapDtDef->epsgNbr) != static_cast<long>(epsgCode))
				{
					mapCnt += 1;
					printf ("EPSG code which appears in definition of datum named '%s' does not match current EPSG database.\n",csMapKeyName);
				}
				okCnt += 1;
				CS_free (csMapDtDef);
			}
		}
		
		// Verify that the Name Mapper has the same name as the EPSG database.
		if (!deprecated)
		{
			csMapSt = csMapIdToNameC (csMapDatumKeyName,mapEpsgName,sizeof (mapEpsgName),
																	csMapFlvrEpsg,
																	csMapFlvrEpsg,
																	epsgCode);
			if (csMapSt != csMapOk || CS_stricmp (epsgKeyName,mapEpsgName) != 0)
			{
				nameCnt += 1;
				printf ("Name Mapper name [%s] not the same as EPSG database name for datum '%s' [%lu].\n",
						mapEpsgName, 
						epsgKeyName,
						static_cast<unsigned long>(epsgCode));
			}
		}
	}
	printf ("\tDatum Audit: ok = %d, noMap = %d, missingDef = %d, deprecationErr = %d,nameDiff = %d, EPSGaccess = %d\n",okCnt,mapCnt,missingCnt,deprCnt,nameCnt,failedCnt);
	errCnt += (failedCnt + missingCnt + deprCnt);
	wrnCnt += mapCnt;

	// Now for the Coordinate System dictionary definitions.
	failedCnt = 0;			// number failed due to EPSG access
	mapCnt = 0;				// number of EPSG coordinate systems which did not map to CS-MAP
	missingCnt = 0;			// number of missing CS-MAP definitions to which an EPSG code maps
	deprCnt = 0;			// number of deprecation mismatches.
	nameCnt = 0;			// number of name mismatches.
	okCnt = 0;				// number of successful name maps.
	recordCount = epsgV6.GetRecordCount (epsgTblReferenceSystem);
	for (unsigned index = 0;index < recordCount;++index)
	{
		csMapKeyName [0] = '\0';
		epsgKeyName [0] = '\0';
		mapEpsgName [0] = '\0';

		ok = epsgV6.GetCodeByIndex (epsgCode,epsgTblReferenceSystem,epsgFldCoordRefSysCode,index);
		if (!ok)
		{
			failedCnt += 1;
			continue;
		}

		if (epsgCode >= 4001UL && epsgCode <= 4054UL)
		{
			// Skip the Geographic CRS's which are simply references to the
			// direct ellipsoid reference datums.
			continue;
		}

		// There are a ton of CRS definitions in EPSG which have a code number
		// greater than 32,767 all of which have been deprecated.  We simply skip
		// over these because they are all deprecated and CS-MAP has never
		// definied equivalents for any of these.
		if (epsgCode >= 32768UL)
		{
			continue;
		}

		// We only do EPSG reference systems of the "projected" or "geographic2D" type.
		ok = epsgV6.GetFieldByCode (fldData,epsgTblReferenceSystem,epsgFldCoordRefSysKind,epsgCode);
		if (ok)
		{
			crsType = GetEpsgCrsType (fldData.c_str ());
			if (crsType != epsgCrsTypProjected && crsType != epsgCrsTypGeographic2D)
			{
				continue;
			}
		}
		else
		{
			failedCnt += 1;
			continue;
		}

		ok = epsgV6.GetFieldByCode (fldData,epsgTblReferenceSystem,epsgFldCoordRefSysName,epsgCode);
		if (!ok)
		{
			failedCnt += 1;
			continue;
		}
		wcstombs (epsgKeyName,fldData.c_str (),sizeof (epsgKeyName));
		epsgKeyName [sizeof (epsgKeyName) - 1] = '\0';

		// We skip the following specific EPSG codes for the reasons given in
		// the comments.
		if (epsgCode == 2192UL)	// replaced by 2154???
		{
			// We skip this one as EPSG has deprected the definition giving
			// the reason: "Withdrawn and replaced before being put into use."
			// It appears to us that this is a valid system and quite likely
			// to be of value to users; thus we are reluctant to deprecate
			// definition.  So, for now, we just skip this test for this
			// specific definition.
			continue;
		}

		// If this system appears in the KcsDeprecatedCRSs table, it is a
		// system deprecated by EPSG, but not replaced by antything, and
		// for which the CS_MAP equivalent system is not deprecated.  Thus,
		// fot hese systems we force a "not deprecated" situation.
		for (idx = 0;KcsDeprecatedCRSs [idx] != 0UL;idx += 1)
		{
			if (epsgCode == KcsDeprecatedCRSs [idx]) break;
		}
		if (KcsDeprecatedCRSs [idx] != 0UL)
		{
			// In the list, thus we consider this CRS to be _NOT_
			// deprecated.
			deprecated = false;
		}
		else
		{
			// This system is not in the list, thus we use the EPSG database
			// deprecation indicator as is.
			deprecated = epsgV6.IsDeprecated (epsgTblReferenceSystem,epsgCode);
		}

		csMapSt = csMapIdToNameC (csMapProjGeoCSys,csMapKeyName,sizeof (csMapKeyName),
																csMapFlvrAutodesk,
																csMapFlvrEpsg,
																epsgCode);
		if (csMapSt != csMapOk)
		{
			if (!deprecated)
			{
				mapCnt += 1;
				if (verbose)
				{
					// There's a ton of these.  We suppress reporting them
					// individually until we get the real problems isolated
					// and straightened out.
					printf ("EPSG CRS '%s' [%lu] not mapped to a CS-MAP CRS.\n",
							 epsgKeyName,
							 static_cast<unsigned long>(epsgCode));
				}
			}
			continue;
		}
		if (csMapSt == csMapOk && deprecated)
		{
			unsigned long tmpUL;

			// Many EPSG definitions are deprecated simply because of a name
			// change; the definitions remain the same.  We try to catch this
			// problem here.
			tmpUL = csMapNameToIdC (csMapProjGeoCSys,csMapFlvrEpsg,csMapFlvrAutodesk,csMapKeyName);
			if (tmpUL > 0UL && tmpUL < 32768UL)
			{
				if (tmpUL != epsgCode)
				{
					continue;
				}
			}
		}
		if (csMapSt == csMapOk)
		{
			csMapCsDef = CS_csdef (csMapKeyName);
			if (csMapCsDef == 0)
			{
				missingCnt += 1;
				printf ("EPSG CRS '%s' [%lu] maps to CS-MAP '%s' which does _NOT_ exist in the dictionary.\n",
						 epsgKeyName,
						 static_cast<ulong32_t>(epsgCode),
						 csMapKeyName);
			}
			else
			{
				legacy = (CS_stricmp (csMapCsDef->group,"LEGACY") == 0);
				if (deprecated && !legacy)
				{
					deprCnt += 1;
					printf ("Deprecated EPSG CRS '%s' [%lu] maps to CS-MAP '%s' which is _NOT_ in the LEGACY group.\n",
							 epsgKeyName,
							 static_cast<ulong32_t>(epsgCode),
							 csMapKeyName);
				}
				else if (legacy && !deprecated)
				{
					deprCnt += 1;
					printf ("EPSG CRS '%s' [%lu] maps to CS-MAP '%s' which is in the LEGACY group.\n",
							 epsgKeyName,
							 static_cast<ulong32_t>(epsgCode),
							 csMapKeyName);
				}
				// Verify that the dictionary has the appropriate EPSG code.
				if (static_cast<long>(csMapCsDef->epsgNbr) != static_cast<long>(epsgCode))
				{
					mapCnt += 1;
					printf ("EPSG code (%d) which appears in definition of CRS named '%s' does not match current EPSG database.\n",
																	csMapCsDef->epsgNbr,
																	csMapKeyName);
				}
				okCnt += 1;
				CS_free (csMapCsDef);
			}
		}

		// Verify that the Name Mapper has the same name as the EPSG database.
		//  In this case, we also want to verify that the name is properly
		// classified.
		if (!deprecated)
		{
			EcsMapObjType mapObjTyp = (crsType == epsgCrsTypProjected) ? csMapProjectedCSysKeyName : csMapGeographicCSysKeyName;
			csMapSt = csMapIdToNameC (mapObjTyp,mapEpsgName,sizeof (mapEpsgName),
															csMapFlvrEpsg,
															csMapFlvrEpsg,
															epsgCode);
			if (csMapSt != csMapOk || CS_stricmp (epsgKeyName,mapEpsgName) != 0)
			{
				nameCnt += 1;
				printf ("Name Mapper name [%s] not the same as EPSG database name for CRS '%s' [%lu].\n",
						mapEpsgName, 
						epsgKeyName,
						static_cast<unsigned long>(epsgCode));
			}
		}
	}
	printf ("\tCoordinate System Audit: ok = %d, noMap = %d, missingDef = %d, deprecationErr = %d, nameDiff = %d, EPSGaccess = %d\n",okCnt,mapCnt,missingCnt,deprCnt,nameCnt,failedCnt);
	if (verbose)
	{
		printf ("%d Warning conditions encvountered in testM\n",wrnCnt);
	}
	errCnt += (failedCnt + missingCnt + deprCnt + nameCnt);
	wrnCnt += mapCnt;

	return errCnt;
}
