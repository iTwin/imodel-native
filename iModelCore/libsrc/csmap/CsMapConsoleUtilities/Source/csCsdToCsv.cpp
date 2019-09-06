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

extern "C"
{
	extern char cs_Csname[];
	extern char cs_Dtname[];
	extern char cs_Elname[];
	extern char cs_Ctname[];
	extern char cs_Gxname[];
	extern char cs_Gpname[];

	extern char cs_Dir[];
	extern char cs_UserDir[];
	extern char* cs_DirP;

	extern char cs_Unique;
	extern char cs_DirsepC;
	extern short cs_Protect;
	extern int cs_Error;
}

extern "C" unsigned long KcsNmMapNoNumber;
extern "C" unsigned long KcsNmInvNumber;

// There are six functions in this module.  Each of these functions produce
// a version of the associated asc/csd files in .csv form.  This is done
// so that dictionaries can be examined (and possibly modified, but this is
// not currently planned) in an Excel environment where data can be easily
// sorted, grouped, and otherwise manipulated and the results eaily visible.
// The original intent here is to provide a basis for improving the EPSG
// number references in each dictionary.  This is of paticular interest
// in the case of the Geodetic Path and Geodetic Transformation dictionaries
// where the EPSG references shgould be to EPSG Paths and EPDG Coordinate
// Operations respectively.  Having a more EPDG number references in the
// Datum dictionary would also be helpful.
//
// The original purpose of improving EPSG reference coverage and accuracy
// is so that the useful range of a CRS can be extracted from the EPSG
// database.  While this has been done at the CRS level, what is expected
// is to be able use the datum, and hence the implied geodetic
// transformation, EPSG defintion to obtain a useful range definition for
// a CRS reference to such datum.
//
bool csCsdToCsvEL (const wchar_t* csDictDir,bool incLegacy)
{
	bool ok (false);

	int crypt;
	unsigned long epsgNbr;

	char *ccPtr;
	const wchar_t* wcPtr;

	double invFlattening;

	char csDictDirC [MAXPATH];
	wchar_t wcBuffer [MAXPATH + MAXPATH];

	std::wstring wcSortField;
	std::wstring wcKeyName;
	std::wstring wcGroup;
	std::wstring wcDescription;
	std::wstring wcSource;
	std::wstring wcEpsgName;

	csFILE* csdStrm;
	std::wofstream csvStrm;

	struct cs_Eldef_ elDef;

	wcstombs (csDictDirC,csDictDir,sizeof (csDictDirC));
	int status = CS_altdr (csDictDirC);
	ok = (status == 0);
	if (!ok)
	{
		return ok;
	}

	csdStrm = NULL;
	ccPtr = CS_stncp (cs_DirP,cs_Elname,cs_FNM_MAXLEN);
	CS_strrpl (cs_Dir,MAXPATH,".CSD",".csv");
	csvStrm.open (cs_Dir,std::ios_base::out | std::ios_base::trunc);
	if (csvStrm.is_open ())
	{
		csdStrm = CS_elopn (_STRM_BINRD);
	}
	if (csdStrm != NULL)
	{
		// We use the wcSortField item to insert a synamic qualifier which
		// will help in analysis.  This to be adjusted by hard code
		// inserted into this module as required.
		wcSortField = L" ";

		// Getting here implies that both opens were successful.
		// Write a label line for our CSV file. There is no good
		// reason for wusing a wide character stream, currently.
		// But we do it anyway.
		csvStrm << L"Sort"                << L','
				<< L"Key Name"            << L','
				<< L"Group"               << L','
				<< L"Semi-Major"          << L','
				<< L"Semi-Minor"          << L','
				<< L"Flattening"          << L','
				<< L"Eccentricity"        << L','
				<< L"Descriptive Name"    << L','
				<< L"Source"              << L','
				<< L"Protect"             << L','
				<< L"EPSG Nbr"            << L','
				<< L"WKT Flavor"          << L','
				<< L"Inv Flattening"      << L','
				<< L"EccentricitySq"      << L','
				<< L"Mapper EPSG Name"    << L','
				<< L"Mapper EPSG Nbr"     << std::endl;
	
		// Loop through each definition in the ellipsoid dictionary,
		// and produce a .csv replica.
		while (CS_elrd (csdStrm,&elDef,&crypt))
		{
			// Skip this one if its deprecated, if so instructed.
			if (!incLegacy && (CS_stricmp (elDef.group,"LEGAC") == 0))
			{
				continue;
			}

			// Here once for each ellipsoid definition.  Simply write
			// a single csv record.  We quote only those fields which may
			// contain a special character or two.  The choice of using
			// wide characters makes fdome of this laborious.  Maybe that
			// was not a good choice.
			mbstowcs (wcBuffer,elDef.key_nm,wcCount (wcBuffer));
			wcKeyName = wcBuffer;
			csCsvQuoter (wcKeyName);

			mbstowcs (wcBuffer,elDef.group,wcCount (wcBuffer));
			wcGroup = wcBuffer;
			csCsvQuoter (wcGroup);

			mbstowcs (wcBuffer,elDef.name,wcCount (wcBuffer));
			wcDescription = wcBuffer;
			csCsvQuoter (wcDescription);

			mbstowcs (wcBuffer,elDef.source,wcCount (wcBuffer));
			wcSource = wcBuffer;
			csCsvQuoter (wcSource);

			wcPtr = csMapNameToName (csMapEllipsoidKeyName,csMapFlvrEpsg,
														   csMapFlvrAutodesk,
														   wcKeyName.c_str ());
			if (wcPtr != 0)
			{
				wcEpsgName = std::wstring (wcPtr);
				csCsvQuoter (wcEpsgName);
			}
			else
			{
				wcEpsgName = L"No EPSG association in name mapper.";
			}

			epsgNbr = csMapNameToIdC (csMapEllipsoidKeyName,csMapFlvrEpsg,
															csMapFlvrAutodesk,
															elDef.key_nm);
			if (epsgNbr == KcsNmInvNumber)
			{
				epsgNbr = 0UL;
			}

			if (elDef.flat == 0.0)
			{
				invFlattening = 0.0;
			}
			else
			{
				invFlattening = (1.0 / elDef.flat);
			}

			csvStrm << wcSortField                    << L','
					<< wcKeyName                      << L','
					<< wcGroup                        << L','
					<< std::fixed
					<< std::showpoint
					<< std::setprecision(3)
					<< elDef.e_rad                    << L','
					<< elDef.p_rad                    << L','
					<< std::setprecision(8)
					<< elDef.flat                     << L','
					<< elDef.ecent                    << L','
					<< wcDescription                  << L','
					<< wcSource                       << L','
					<< elDef.protect                  << L','
					<< elDef.epsgNbr                  << L','
					<< elDef.wktFlvr                  << L','
					<< std::setprecision (6)
					<< invFlattening                  << L','
					<< std::setprecision (8)
					<< (elDef.ecent * elDef.ecent)    << L','
					<< wcEpsgName                     << L','
					<< epsgNbr                        << std::endl;
		}
		csvStrm.close ();
		CS_fclose (csdStrm);
	}
	return ok;
}
bool csCsdToCsvDT (const wchar_t* csDictDir,bool incLegacy)
{
	bool ok (false);

	int crypt;
	unsigned long epsgNbr;

	char *ccPtr;
	const wchar_t* wcPtr;

	char csDictDirC [MAXPATH];
	wchar_t wcBuffer [MAXPATH + MAXPATH];

	std::wstring wcSortField;
	std::wstring wcKeyName;
	std::wstring wcEllipsoid;
	std::wstring wcGroup;
	std::wstring wcLocation;
	std::wstring wcCountrySt;
	std::wstring wcDescription;
	std::wstring wcSource;
	std::wstring wcEpsgName;
	std::wstring wcMethod;

	csFILE* csdStrm;
	std::wofstream csvStrm;

	struct cs_Dtdef_ dtDef;

	wcstombs (csDictDirC,csDictDir,sizeof (csDictDirC));
	int status = CS_altdr (csDictDirC);
	ok = (status == 0);
	if (!ok)
	{
		return ok;
	}

	csdStrm = NULL;
	ccPtr = CS_stncp (cs_DirP,cs_Dtname,cs_FNM_MAXLEN);
	CS_strrpl (cs_Dir,MAXPATH,".CSD",".csv");
	csvStrm.open (cs_Dir,std::ios_base::out | std::ios_base::trunc);
	if (csvStrm.is_open ())
	{
		csdStrm = CS_dtopn (_STRM_BINRD);
	}
	if (csdStrm != NULL)
	{
		wcSortField = L" ";

		// Getting here implies that both opens were successful.
		// Write a label line for our CSV file. There is no good
		// reason for wusing a wide character stream, currently.
		// But we do it anyway.
		csvStrm << L"Sort"                << L','
				<< L"Key Name"            << L','
				<< L"Ellipsoid"           << L','
				<< L"Group"               << L','
				<< L"Location"            << L','
				<< L"Country/State"       << L','
				<< L"Delta X"             << L','
				<< L"Delta Y"             << L','
				<< L"Delta Z"             << L','
				<< L"Rotation X"          << L','
				<< L"Rotation Y"          << L','
				<< L"Rotation Z"          << L','
				<< L"Scale"               << L','
				<< L"Description"         << L','
				<< L"Source"              << L','
				<< L"Protect"             << L','
				<< L"Method"              << L','
				<< L"EPSG Nbr"            << L','
				<< L"WKT Flavor"          << L','
				<< L"Mapper EPSG Name"    << L','
				<< L"Mapper EPSG Nbr"     << std::endl;
	
		// Loop through each definition in the datum dictionary,
		// and produce a .csv replica.
		while (CS_dtrd (csdStrm,&dtDef,&crypt))
		{
			// Skip this one if its deprecated, if so instructed.
			if (!incLegacy && (CS_stricmp (dtDef.group,"LEGACY") == 0))
			{
				continue;
			}
			// Here once for each ellipsoid definition.  Simply write
			// a single csv record.  We quote only those fields which may
			// contain a special character or two.  The choice of using
			// wide characters makes fdome of this laborious.  Maybe that
			// was not a good choice.
			mbstowcs (wcBuffer,dtDef.key_nm,wcCount (wcBuffer));
			wcKeyName = wcBuffer;
			csCsvQuoter (wcKeyName);

			mbstowcs (wcBuffer,dtDef.ell_knm,wcCount (wcBuffer));
			wcEllipsoid = wcBuffer;
			csCsvQuoter (wcEllipsoid);

			mbstowcs (wcBuffer,dtDef.group,wcCount (wcBuffer));
			wcGroup = wcBuffer;
			csCsvQuoter (wcGroup);

			mbstowcs (wcBuffer,dtDef.locatn,wcCount (wcBuffer));
			wcLocation = wcBuffer;
			csCsvQuoter (wcLocation);

			mbstowcs (wcBuffer,dtDef.cntry_st,wcCount (wcBuffer));
			wcCountrySt = wcBuffer;
			csCsvQuoter (wcCountrySt);

			mbstowcs (wcBuffer,dtDef.name,wcCount (wcBuffer));
			wcDescription = wcBuffer;
			csCsvQuoter (wcDescription);

			mbstowcs (wcBuffer,dtDef.source,wcCount (wcBuffer));
			wcSource = wcBuffer;
			csCsvQuoter (wcSource);

			wcPtr = NULL;
			switch (dtDef.to84_via) {
			default:
			case cs_DTCTYP_NONE:
				wcPtr = L"None";
				break;
			case cs_DTCTYP_MOLO:
				wcPtr = L"Molodensky";
				break;
			case cs_DTCTYP_MREG:
				wcPtr = L"Multiple Regression";
				break;
			case cs_DTCTYP_BURS:
				wcPtr = L"Bursa/Wolf";
				break;
			case cs_DTCTYP_NAD27:
				wcPtr = L"NAD27";
				break;
			case cs_DTCTYP_NAD83:
				wcPtr = L"NAD83";
				break;
			case cs_DTCTYP_WGS84:
				wcPtr = L"WGS84";
				break;
			case cs_DTCTYP_WGS72:
				wcPtr = L"WGS72";
				break;
			case cs_DTCTYP_HPGN:
				wcPtr = L"HPGN";
				break;
			case cs_DTCTYP_7PARM:
				wcPtr = L"7 Parameter";
				break;
			case cs_DTCTYP_AGD66:
				wcPtr = L"AGD66";
				break;
			case cs_DTCTYP_3PARM:
				wcPtr = L"Three Parameter";
				break;
			case cs_DTCTYP_6PARM:
				wcPtr = L"Six Parameter";
				break;
			case cs_DTCTYP_4PARM:
				wcPtr = L"Four Parameter";
				break;
			case cs_DTCTYP_AGD84:
				wcPtr = L"AGD84";
				break;
			case cs_DTCTYP_NZGD49:
				wcPtr = L"NZGD49";
				break;
			case cs_DTCTYP_ATS77:
				wcPtr = L"ATS77";
				break;
			case cs_DTCTYP_GDA94:
				wcPtr = L"GDA94";
				break;
			case cs_DTCTYP_NZGD2K:
				wcPtr = L"NZGD2K";
				break;
			case cs_DTCTYP_CSRS:
				wcPtr = L"CSRS";
				break;
			case cs_DTCTYP_TOKYO:
				wcPtr = L"Tokyo";
				break;
			case cs_DTCTYP_RGF93:
				wcPtr = L"RGF93";
				break;
			case cs_DTCTYP_ED50:
				wcPtr = L"ED50";
				break;
			case cs_DTCTYP_DHDN:
				wcPtr = L"DHDN";
				break;
			case cs_DTCTYP_ETRF89:
				wcPtr = L"ETRF89";
				break;
			case cs_DTCTYP_GEOCTR:
				wcPtr = L"Geocentric";
				break;
			case cs_DTCTYP_CHENYX:
				wcPtr = L"ChenYX";
				break;
			}
			wcMethod = wcPtr;
			csCsvQuoter (wcMethod);

			wcPtr = csMapNameToName (csMapDatumKeyName,csMapFlvrEpsg,
													   csMapFlvrAutodesk,
													   wcKeyName.c_str ());
			if (wcPtr != 0)
			{
				wcEpsgName = std::wstring (wcPtr);
				csCsvQuoter (wcEpsgName);
			}
			else
			{
				wcEpsgName = L"No EPSG association in name mapper.";
			}

			epsgNbr = csMapNameToIdC (csMapDatumKeyName,csMapFlvrEpsg,
														csMapFlvrAutodesk,
														dtDef.key_nm);
			if (epsgNbr == KcsNmInvNumber)
			{
				epsgNbr = 0UL;
			}

			csvStrm << wcSortField                    << L','
					<< wcKeyName                      << L','
					<< wcEllipsoid                    << L','
					<< wcGroup                        << L','
					<< wcLocation                     << L','
					<< wcCountrySt                    << L','
					<< std::fixed
					<< std::showpoint
					<< std::setprecision(5)
					<< dtDef.delta_X                  << L','
					<< dtDef.delta_Y                  << L','
					<< dtDef.delta_Z                  << L','
					<< dtDef.rot_X                    << L','
					<< dtDef.rot_Y                    << L','
					<< dtDef.rot_Z                    << L','
					<< dtDef.bwscale                  << L','
					<< std::fixed
					<< std::noshowpoint
					<< std::setprecision(0)
					<< wcDescription                  << L','
					<< wcSource                       << L','
					<< dtDef.protect                  << L','
					<< wcMethod                       << L','
					<< dtDef.epsgNbr                  << L','
					<< dtDef.wktFlvr                  << L','
					<< wcEpsgName                     << L','
					<< epsgNbr                        << std::endl;
		}
		csvStrm.close ();
		CS_fclose (csdStrm);
	}
	return ok;
}
bool csCsdToCsvCS (const wchar_t* csDictDir,bool incLegacy)
{
	bool ok (false);

	int crypt;
	unsigned char prjParm1;
	unsigned short prjCode;
	unsigned long epsgCrsCode;

	char *ccPtr;
	const wchar_t* wcPtr;

	char csDictDirC [MAXPATH];
	wchar_t wcBuffer [MAXPATH + MAXPATH];

	std::wstring wcSortField;
	std::wstring wcKeyName;
	std::wstring wcDatum;
	std::wstring wcEllipsoid;
	std::wstring wcProjection;
	std::wstring wcGroup;
	std::wstring wcLocation;
	std::wstring wcCountrySt;
	std::wstring wcUnit;
	std::wstring wcDescription;
	std::wstring wcSource;
	std::wstring wcEpsgName;
	std::wstring wcMethod;

	csFILE* csdStrm;
	std::wofstream csvStrm;

	struct cs_Csdef_ csDef;

	wcstombs (csDictDirC,csDictDir,sizeof (csDictDirC));
	int status = CS_altdr (csDictDirC);
	ok = (status == 0);
	if (!ok)
	{
		return ok;
	}

	csdStrm = NULL;
	ccPtr = CS_stncp (cs_DirP,cs_Csname,cs_FNM_MAXLEN);
	CS_strrpl (cs_Dir,MAXPATH,".CSD",".csv");
	csvStrm.open (cs_Dir,std::ios_base::out | std::ios_base::trunc);
	if (csvStrm.is_open ())
	{
		csdStrm = CS_csopn (_STRM_BINRD);
	}
	if (csdStrm != NULL)
	{
		// Getting here implies that both opens were successful.
		// Write a label line for our CSV file. There is no good
		// reason for using a wide character stream, currently.
		// But we do it anyway.
		csvStrm << L"Sort"                << L','
				<< L"CRS Key Name"        << L','
				<< L"Datum Key Name"      << L','
				<< L"Ellipsoid Key Name"  << L','
				<< L"Projection Code"     << L','
				<< L"Group"               << L','
				<< L"Location"            << L','
				<< L"Country/State"       << L','
				<< L"Unit"                << L','
				<< L"Parm  1"             << L','
				<< L"Parm  2"             << L','
				<< L"Parm  3"             << L','
				<< L"Parm  4"             << L','
				<< L"Parm  5"             << L','
				<< L"Parm  6"             << L','
				<< L"Parm  7"             << L','
				<< L"Parm  8"             << L','
				<< L"Parm  9"             << L','
				<< L"Parm 10"             << L','
				<< L"Parm 11"             << L','
				<< L"Parm 12"             << L','
				<< L"Parm 13"             << L','
				<< L"Parm 14"             << L','
				<< L"Parm 15"             << L','
				<< L"Parm 16"             << L','
				<< L"Parm 17"             << L','
				<< L"Parm 18"             << L','
				<< L"Parm 19"             << L','
				<< L"Parm 20"             << L','
				<< L"Parm 21"             << L','
				<< L"Parm 22"             << L','
				<< L"Parm 23"             << L','
				<< L"Parm 24"             << L','
				<< L"Org Lng"             << L','
				<< L"Org Lat"             << L','
				<< L"False Easting"       << L','
				<< L"False Northing"      << L','
				<< L"Scale Reduction"     << L','
				<< L"Unit Scale"          << L','
				<< L"Map Scale"           << L','
				<< L"Scale"               << L','
				<< L"Minimum X"           << L','
				<< L"Minimum Y"           << L','
				<< L"Height Lng"          << L','
				<< L"Height Lat"          << L','
				<< L"Height Elev"         << L','
				<< L"Geoid Sep"           << L','
				<< L"Min Lng"             << L','
				<< L"Min Lat"             << L','
				<< L"Max Lng"             << L','
				<< L"Max Lat"             << L','
				<< L"Min X"               << L','
				<< L"Min Y"               << L','
				<< L"Max X"               << L','
				<< L"Max Y"               << L','
				<< L"Description"         << L','
				<< L"Source"              << L','
				<< L"Quad"                << L','
				<< L"Order"               << L','
				<< L"Zones"               << L','
				<< L"Protect"             << L','
				<< L"Epsg Quad"           << L','
				<< L"Srid"                << L','
				<< L"Epsg Nbr"            << L','
				<< L"Wkt Flavor"          << L','
				<< L"Epsg Name"           << L','
				<< L"Epsg CRS Code"       << std::endl;

		// Loop through each definition in the coordinate system
		// dictionary, and produce a .csv replica.
		while (CS_csrd (csdStrm,&csDef,&crypt))
		{
			// Skip this one if its deprecated, if so instructed.
			if (!incLegacy && (CS_stricmp (csDef.group,"LEGACY") == 0))
			{
				continue;
			}
			// Here once for each coordinate system definition.  Simply write
			// a single csv record.  We quote only those fields which may
			// contain a special character or two.  The choice of using
			// wide characters makes some of this laborious.  Maybe that
			// was not a good choice.
			mbstowcs (wcBuffer,csDef.key_nm,wcCount (wcBuffer));
			wcKeyName = wcBuffer;
			csCsvQuoter (wcKeyName);

			mbstowcs (wcBuffer,csDef.dat_knm,wcCount (wcBuffer));
			wcDatum = wcBuffer;
			csCsvQuoter (wcDatum);

			mbstowcs (wcBuffer,csDef.elp_knm,wcCount (wcBuffer));
			wcEllipsoid = wcBuffer;
			csCsvQuoter (wcEllipsoid);

			mbstowcs (wcBuffer,csDef.prj_knm,wcCount (wcBuffer));
			wcProjection = wcBuffer;
			csCsvQuoter (wcProjection);

			mbstowcs (wcBuffer,csDef.group,wcCount (wcBuffer));
			wcGroup = wcBuffer;
			csCsvQuoter (wcGroup);

			mbstowcs (wcBuffer,csDef.locatn,wcCount (wcBuffer));
			wcLocation = wcBuffer;
			csCsvQuoter (wcLocation);

			mbstowcs (wcBuffer,csDef.cntry_st,wcCount (wcBuffer));
			wcCountrySt = wcBuffer;
			csCsvQuoter (wcCountrySt);

			mbstowcs (wcBuffer,csDef.unit,wcCount (wcBuffer));
			wcUnit = wcBuffer;
			csCsvQuoter (wcUnit);

			mbstowcs (wcBuffer,csDef.desc_nm,wcCount (wcBuffer));
			wcDescription = wcBuffer;
			csCsvQuoter (wcDescription);

			mbstowcs (wcBuffer,csDef.source,wcCount (wcBuffer));
			wcSource = wcBuffer;
			csCsvQuoter (wcSource);

			wcPtr = csMapNameToName (csMapProjGeoCSys,csMapFlvrEpsg,
													  csMapFlvrAutodesk,
													  wcKeyName.c_str ());
			if (wcPtr != 0)
			{
				wcEpsgName = std::wstring (wcPtr);
				csCsvQuoter (wcEpsgName);
			}
			else
			{
				wcEpsgName = L"No EPSG association in name mapper.";
			}

			epsgCrsCode = csMapNameToIdC (csMapProjGeoCSys,csMapFlvrEpsg,
														   csMapFlvrAutodesk,
														   csDef.key_nm);
			if (epsgCrsCode == KcsNmInvNumber)
			{
				epsgCrsCode = 0UL;
			}

			// I found the following most useful.  For the Transverse Mercator,
			// and similarly with the UTM Projection, the org_lng value in
			// the definition is zero; the cntral meridian being indicated by
			// the value of parameter1.  This code sets the org_lng field to
			// the actual longitude indicated of the central meridian for
			// various projections.
			prjParm1 = cs_PRMCOD_NOTUSED;
			prjCode = CS_getPrjCode (csDef.prj_knm);
			if (prjCode != cs_PRJCOD_END)
			{
				prjParm1 = CS_getParmCode (prjCode,1);
			}

			switch (prjParm1) {
			case cs_PRMCOD_CNTMER:
				csDef.org_lng = csDef.prj_prm1;
				break;
			case cs_PRMCOD_UTMZN:
				csDef.org_lng = (csDef.prj_prm1 * 6.0) - 183.0;
				break;
			case cs_PRMCOD_NOTUSED:
			default:
				// We do nothing here.
				break;
			}

			// Example of using the sort field.  In this case, we mark each
			// definition which has useful range with a 'Y', and those
			// which do not have a useful range specification with an 'N'.
			// We can then sort by this in Excel (or something similar) and
			// analyze the result.
			wcSortField = L"Y";
			if (fabs (csDef.ll_min [0]) < 0.001 && 
			    fabs (csDef.ll_min [1]) < 0.001 && 
			    fabs (csDef.ll_max [0]) < 0.001 && 
			    fabs (csDef.ll_max [1]) < 0.001)
			{
				wcSortField = L"N";
			}

			csvStrm << wcSortField                    << L','
					<< wcKeyName                      << L','
					<< wcDatum                        << L','
					<< wcEllipsoid                    << L','
					<< wcProjection                   << L','
					<< wcGroup                        << L','
					<< wcLocation                     << L','
					<< wcCountrySt                    << L','
					<< wcUnit                         << L','
					<< std::fixed
					<< std::showpoint
					<< std::setprecision(10)
					<< csDef.prj_prm1                 << L','
					<< csDef.prj_prm2                 << L','
					<< csDef.prj_prm3                 << L','
					<< csDef.prj_prm4                 << L','
					<< csDef.prj_prm5                 << L','
					<< csDef.prj_prm6                 << L','
					<< csDef.prj_prm7                 << L','
					<< csDef.prj_prm8                 << L','
					<< csDef.prj_prm9                 << L','
					<< csDef.prj_prm10                << L','
					<< csDef.prj_prm11                << L','
					<< csDef.prj_prm12                << L','
					<< csDef.prj_prm13                << L','
					<< csDef.prj_prm14                << L','
					<< csDef.prj_prm15                << L','
					<< csDef.prj_prm16                << L','
					<< csDef.prj_prm17                << L','
					<< csDef.prj_prm18                << L','
					<< csDef.prj_prm19                << L','
					<< csDef.prj_prm20                << L','
					<< csDef.prj_prm21                << L','
					<< csDef.prj_prm22                << L','
					<< csDef.prj_prm23                << L','
					<< csDef.prj_prm24                << L','

					<< csDef.org_lng                  << L','
					<< csDef.org_lat                  << L','
					<< csDef.x_off                    << L','
					<< csDef.y_off                    << L','
					<< csDef.scl_red                  << L','
					<< csDef.unit_scl                 << L','
					<< csDef.map_scl                  << L','
					<< csDef.scale                    << L','
					<< csDef.zero[0]                  << L','
					<< csDef.zero[1]                  << L','
					<< csDef.hgt_lng                  << L','
					<< csDef.hgt_lat                  << L','
					<< csDef.hgt_zz                   << L','
					<< csDef.geoid_sep                << L','
					<< csDef.ll_min[0]                << L','
					<< csDef.ll_min[1]                << L','
					<< csDef.ll_max[0]                << L','
					<< csDef.ll_max[1]                << L','
					<< csDef.xy_min[0]                << L','
					<< csDef.xy_min[1]                << L','
					<< csDef.xy_max[0]                << L','
					<< csDef.xy_max[1]                << L','
					<< std::fixed
					<< std::noshowpoint
					<< std::setprecision(0)
					<< wcDescription                  << L','
					<< wcSource                       << L','
					<< csDef.quad                     << L','
					<< csDef.order                    << L','
					<< csDef.zones                    << L','
					<< csDef.protect                  << L','
					<< csDef.epsg_qd                  << L','
					<< csDef.srid                     << L','
					<< csDef.epsgNbr                  << L','
					<< csDef.wktFlvr                  << L','
					<< wcEpsgName                     << L','
					<< epsgCrsCode                    << std::endl;
		}
		csvStrm.close ();
		CS_fclose (csdStrm);
	}
	return ok;
}
bool csCsdToCsvCT (const wchar_t* csDictDir,bool incLegacy)
{
	bool ok (false);

	char *ccPtr;
	struct cs_CtItmName_ *itmPtr;

	char crsDefName [cs_KEYNM_DEF];
	char crsDefDescr [64];
	char csDictDirC [MAXPATH];
	wchar_t wcBuffer [MAXPATH + MAXPATH];

	struct cs_Ctdef_ ctDef;

	std::wstring wcSortField;
	std::wstring wcCatName;
	std::wstring wcCrsName;
	std::wstring wcCrsDescr;

	csFILE* csdStrm;
	std::wofstream csvStrm;

	/* ctDef must be initialized to something rational. */
	memset (&ctDef,'\0',sizeof (ctDef));

	wcstombs (csDictDirC,csDictDir,sizeof (csDictDirC));
	int status = CS_altdr (csDictDirC);
	ok = (status == 0);
	if (!ok)
	{
		return ok;
	}

	csdStrm = NULL;
	ccPtr = CS_stncp (cs_DirP,cs_Ctname,cs_FNM_MAXLEN);
	CS_strrpl (cs_Dir,MAXPATH,".CSD",".csv");
	csvStrm.open (cs_Dir,std::ios_base::out | std::ios_base::trunc);
	if (csvStrm.is_open ())
	{
		csdStrm = CS_ctopn (_STRM_BINRD);
		ok = (csdStrm != NULL);
	}
	if (ok)
	{
		// We use the wcSortField item to insert a synamic qualifier which
		// will help in analysis.  This to be adjusted by hard code
		// inserted into this module as required.
		wcSortField = L" ";

		// Getting here implies that both opens were successful.
		// Write a label line for our CSV file. There is no good
		// reason for wusing a wide character stream, currently.
		// But we do it anyway.
		csvStrm << L"Sort"                << L','
				<< L"Cat Name"            << L','
				<< L"CRS Name"            << L','
				<< L"CRS Description"     << std::endl;

		// Loop through each category in the category dictionary,
		while (CS_ctrd (csdStrm,&ctDef) > 0)
		{
			mbstowcs (wcBuffer,ctDef.ctName,wcCount (wcBuffer));
			wcCatName = wcBuffer;
			csCsvQuoter (wcCatName);

			/* We have the category name, now loop once for each
			   CRS in the ctaegory. */
			for (ulong32_t idx = 0;idx < ctDef.nameCnt;idx++)
			{
				itmPtr = ctDef.csNames + idx;
				ccPtr = itmPtr->csName;
				CS_stncp (crsDefName,ccPtr,sizeof (crsDefName));
				mbstowcs (wcBuffer,ccPtr,wcCount (wcBuffer));
				wcCrsName = wcBuffer;

				CS_getDescriptionOf (crsDefName,crsDefDescr,sizeof (crsDefDescr));
				mbstowcs (wcBuffer,crsDefDescr,wcCount (wcBuffer));
				wcCrsDescr = wcBuffer;
				csCsvQuoter (wcCrsDescr);

				/* write the entry out */
		
				csvStrm << wcSortField                 << L','
						<< wcCatName                   << L','
						<< wcCrsName                   << L','
						<< wcCrsDescr                  << std::endl;
			}
		}
		csvStrm.close ();
		CS_fclose (csdStrm);
		CSclnCategory (&ctDef);
	}
	return ok;
}
bool csCsdToCsvGX (const wchar_t* csDictDir,bool incLegacy)
{
	return false;
}
bool csCsdToCsvGP (const wchar_t* csDictDir,bool incLegacy)
{
	return false;
}
