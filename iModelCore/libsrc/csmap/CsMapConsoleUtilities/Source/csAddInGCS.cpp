/*
 * Copyright (c) 2008, Autodesk, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Autodesk, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Autodesk, Inc. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Autodesk, Inc. OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "csConsoleUtilities.hpp"

extern "C" unsigned long KcsNmInvNumber;
extern "C" unsigned long KcsNmMapNoNumber;

// We devise the CS-MAP names for this systems from data in the InGCS data
// file.  We extract the EPSG code from the EPSG 8.8 Alias table by creating
// an EPSG Alias name from the data in the InGCS file.  This can be done as
// the EPSG Alias names are very consistently formed.  This implies that
// there may be several InGCS systems which refer to the same EPSG code.  This
// is expected.  This is why the "abbeviations" were invented.
//
// We will need to add entries to the name mapper. Here, we encounter the same
// problem that EPSG had. What we will do is make a unique entry in the NameMapper
// for each EPSG code, with the official EPSG name.  We add to that additional
// entries with the alias flag set.  In this manner, the names we assign to the
// InGCS systems will map to the correct EPSG code.
//
// Going the other way, i.e. mapping an EPSG code back to our coordinate system names,
// the DupSort field will determine which name is selected.  We will assign sequential
// DupSort values starting at (arbitraryily) 10.  Users in a specific county, for
// example, could renumber their county to DupSort value of 1 so that the EPSG
// code would always map to their county system name.  We will provide an API
// which will facilitate this reordering process.
//

// Each record in the InGCS.csv file is converted to the following
// form.  This is accomplished in a function of its own.  In this form,
// we can use elements of this structure to record various states for
// each definition.

// In CS-MAP, all characters are 8 bit characters.  So we convert from whatever
// is in the .csv source data file to char's when we read the file.  Life is a
// bit simpler that way.

struct TcsInGcsSpec
{
	char        ZoneName [32];
	char        CountyCode [8];
	char        EpsgAbbreviation [32];
	double      OriginLatitude;
	double      OriginLongitude;
	double      ScaleReduction;
	double      FalseEasting;
	double      FalseNorthing;
	double      TestEasting;
	double      TestNorthing;
	TcsEpsgCode EpsgCodeMetric;
	TcsEpsgCode EpsgCodeUsFoot;
	double      FalseEastingFeet;
	double      FalseNorthingFeet;
	double      TestEastingFeet;
	double      TestNorthingFeet;
	char        EpsgNameMetric [128];
	char        EpsgNameUsFoot [128];
	char        EpsgAliasMetric [128];
	char        EpsgAliasUsFoot [128];
	char        EpsgZoneMetric [128];
	char        EpsgZoneUsFoot [128];
	char        CsMapKeyNameMetric [32];
	char        CsMapKeyNameUsFoot [32];
	char        CsMapDescriptionMetric [64];
	char        CsMapDescriptionUsFoot [64];
	short       DupSortCode;
	bool        MainEpsgWritten;
	double      MinLongitude;
	double      MinLatitude;
	double      MaxLongitude;
	double      MaxLatitude;
};

// Since we know how many there are, we have a fixed array.  No real need
// for std::vectors.
const unsigned KcsInGcsSpecCount = 92;
TcsInGcsSpec VcsInGcsSpecs [KcsInGcsSpecCount];

// The "data source" for all definitions is the same:
const char inGcsSource [64] = "Indiana GCS: http://www.in.gov/indot/3397.htm";

// Basic Process is in several phases. In most cases, there exists a
// specific sub-function for each individual phase.  The whole thing is
// done programatically, to avoid typographical errors.  Also, if a
// problem is found, we fix it and run the process again.  Repeat until
// the result passes all tests and reviews.
//
//	1> Read data source file.
//	2> Extract necessary information from EPSG.
//	3> Generate CS-MAP names and descriptive elements.
//	3> Make calculations for two different unit systems.
//	4> Assign NameMapper DupSort values.
//	5> Loop once for each of the 92 "zones"
//		a> Create metric definition and add to the coordsys.asc file.
//		b> Modify the units and write modified definition to coordinsys.asc file.
//	6> Write coordsys.asc file to targte directory.
//	7> Open category.asc file and extract the Indiana category.
//	8> Loop once for each of the 92 zones.
//		a> Write metric entry
//		b> Write US Foot entry
//	9> Close categoty file and write to the target directory.
// 10> Open the NameMapper file,
// 11> Loop once for each of the 92 zones.
//		a> Add primary "system" entries to NameMapper if not already added.
//         one for the metric system and one for the US foot system.
//		b> Add the CS-MAP entries (meteric and foot) for this particular zone.
// 12> Sort and write the modified NameMapper to the target directory.
// 13> Copy the source Test.Dat file to the target directory.
// 14> Open the target Test.Dat file for appending.
// 15> Loop once for each of the 92 zones.
//		a> Append a forward and inverse test for the metric system.
//		b> Append a forward and inverse test for the foot system.
// 16> Close eup and return "true".
//		

bool csAddInGcsReadData (const wchar_t* dataDir,const wchar_t* dataName);
double CS_wc2d (const wchar_t* wcNbr);
bool csAddInGcsAddEpsgData (const wchar_t* epsgDir);
bool csAddInGcsGenerateCsMapData (void);
bool csAddInGcsDupSort (void);
bool csAddInGcsCalcUsFoot (void);
bool csAddInGcsBuildCrs (TcsAscDefinition& csMapCrsDef,unsigned idx);
bool csAddInGcsToUsFeet (TcsAscDefinition& csMapCrsDef,unsigned idx);
bool csAddInGcsCategories (TcsCategory* categoryPtr);
bool csAddInGcsNameMap (TcsNameMapper& nameMapper);
bool csAddInGcsTestData (std::ostream& outStrm);

bool csAddInGCS (const wchar_t* trgDir,const wchar_t* dataDir,const wchar_t* dataName,
															  const wchar_t* dictDir,
															  const wchar_t* epsgDir)
{
	bool ok;
	unsigned idx;

	// Read the source data file; populate the source data items in the
	// VcsInGcsSpecs array.  After this we do not need the source data
	// file any more.
	ok = csAddInGcsReadData (dataDir,dataName);
	
	// Fill extract the necessary info from EPSG.  This will necessarily
	// read the active EPSG database (needs to be 8.8 or later).  Once this
	// is complete, we no longer need the EPSG database.  That's the paln,
	// anyway.
	if (ok)
	{
		ok = csAddInGcsAddEpsgData (epsgDir);
	}
	
	// Generate CsMap name and descriptive elements.  Combine InGCS and EPSG
	// data into the data elements required to produce the definitions.
	if (ok)
	{
		ok = csAddInGcsGenerateCsMapData ();
	}

	// Assign the DupSort numbers.  When adding entries to the NameMapper,
	// in 35 cases there will be several names for a given EPSG ID number.  These
	// names will be marked as aliases which enables the NameMapper to map the
	// names to the EPSG code (not a big problem).  When mapping the EPSG code
	// to a name, however, there is a problem; the NameMapper has no way of
	// knowing (in these 35 cases) which name to return.  We assign DupSort
	// numbers to these names and in that way specify which name gets returned.
	// End users in a specific Indiana county, therefore, will be able (if they
	// want) to modify the DupSort numbers so that their ounty name comes up.
	if (ok)
	{
		ok = csAddInGcsDupSort ();
	}
	
	// Add the US Foot values. Calculate the appropriate values for the
	// "US Survey Foot" version of the system.
	if (ok)
	{
		ok = csAddInGcsCalcUsFoot ();
	}

	// Open the coordsys.asc file as a TcsAscFile object.
	if (ok)
	{
		// Create a TcsDefFile object which represents the current
		// coordsys.asc dictionary file.
		char coordsysPathName [512];
		wcstombs (coordsysPathName,dictDir,sizeof (coordsysPathName));
		CS_stncat (coordsysPathName,"\\coordsys.asc",sizeof (coordsysPathName));
		TcsDefFile csCoordSysAsc (dictTypCoordsys,coordsysPathName);

		// Loop once for each entry in the array
		for (idx = 0;ok && idx < KcsInGcsSpecCount;idx += 1)
		{
			// Construct a TcsAscDefinition for this entry using meters.
			const TcsDefLine firstLine (dictTypCoordsys,"CS_NAME:",VcsInGcsSpecs [idx].CsMapKeyNameMetric,0);
			TcsAscDefinition csMapCrsDef (dictTypCoordsys,firstLine);
			ok = csAddInGcsBuildCrs (csMapCrsDef,idx);

			// Insert into the coordsys.asc file.  This will be slow, but
			// we only need this to run once successfully.
			if (ok)
			{
				ok = csCoordSysAsc.InsertBefore ("NSRS11.KS-N",csMapCrsDef);
			}

			// Modify the definition to be the Foot version.
			if (ok)
			{
				ok = csAddInGcsToUsFeet (csMapCrsDef,idx);
			}
			if (ok)
			{
				ok = csCoordSysAsc.InsertBefore ("NSRS11.KS-N",csMapCrsDef);
			}
			// On each loop, the TcsDefinition object goes out of scope
			// and is deleted.
		}

		// Write the coordsys.asc file to the target directory.
		if (ok)
		{
			wcstombs (coordsysPathName,trgDir,sizeof (coordsysPathName));
			CS_stncat (coordsysPathName,"\\coordsys.asc",sizeof (coordsysPathName));
			ok = csCoordSysAsc.WriteToFile (coordsysPathName);
		}
		// csCoordSysAsc should go out of scope here and be deleted.
	}

	// Deal with the category file.
	if (ok)
	{
		// Get a TcsCategoryFile object we can work with.
		char categoryPathName [512];
		wcstombs (categoryPathName,dictDir,sizeof (categoryPathName));
		CS_stncat (categoryPathName,"\\category.asc",sizeof (categoryPathName));
		TcsCategoryFile categoryFile;
		ok = categoryFile.InitializeFromFile (categoryPathName);
		if (ok)
		{
			// Extract the Indiana category, the only one we need to
			// deal with.
			TcsCategory* categoryPtr = categoryFile.FetchCategory ("USA, Indiana");
			ok = (categoryPtr != NULL);
			if (ok)
			{
				// Add 184 entries to the "USA, Indiana" category.  Note, that
				// the new entries are actually added to the TcsCategory object
				// as it exists within the TcsCategoryFile object.
				ok = csAddInGcsCategories (categoryPtr);
			}
		}
		if (ok)
		{
			// Write the modified category object out to a category file
			// in the target directory.
			wcstombs (categoryPathName,trgDir,sizeof (categoryPathName));
			CS_stncat (categoryPathName,"\\category.asc",sizeof (categoryPathName));
			ok = categoryFile.WriteToFile (categoryPathName);
		}
		// categoryFile goes out of scope here and should be deleted.
	}
	
	// Open the NameMapper
	if (ok)
	{
		std::wifstream inStrm;
		std::wofstream outStrm;
		EcsCsvStatus csvSt;
		TcsCsvStatus csvStatus;
		TcsNameMapper nameMapper;
		char nameMapperPathName [512];

		wcstombs (nameMapperPathName,dictDir,sizeof (nameMapperPathName));
		CS_stncat (nameMapperPathName,"\\NameMapper.csv",sizeof (nameMapperPathName));
		inStrm.open (nameMapperPathName,std::ios::in);
		ok = inStrm.is_open ();
		if (ok)
		{
			csvSt = nameMapper.ReadFromStream (inStrm,csvStatus);
			ok = (csvSt == csvOk);
			inStrm.close ();
		}
		if (ok)
		{
			// Loop and do all nameMapper entries here.
			ok = csAddInGcsNameMap (nameMapper);
		}
		if (ok)
		{
			wcstombs (nameMapperPathName,trgDir,sizeof (nameMapperPathName));
			CS_stncat (nameMapperPathName,"\\NameMapper.csv",sizeof (nameMapperPathName));
			outStrm.open (nameMapperPathName,std::ios::out | std::ios::trunc);
			ok = outStrm.is_open ();
			if (ok)
			{
				// No status return?  Guess we just have to trust it.
				nameMapper.WriteAsCsv (outStrm,true);
				ok = true;
				outStrm.close ();
			}
		}
		// nameMapper goes out of scope here and should be deleted.
	}

	// Open the source test data file.
	if (ok)
	{	
		std::ifstream inStrm;
		std::ofstream outStrm;
		char testDataPathName [512];

		wcstombs (testDataPathName,dictDir,sizeof (testDataPathName));
		CS_stncat (testDataPathName,"\\..\\TestCpp\\TEST.DAT",sizeof (testDataPathName));
		inStrm.open (testDataPathName,std::ios::in);
		ok = inStrm.is_open ();
		if (ok)
		{
			wcstombs (testDataPathName,trgDir,sizeof (testDataPathName));
			CS_stncat (testDataPathName,"\\TEST.DAT",sizeof (testDataPathName));
			outStrm.open (testDataPathName,std::ios::out | std::ios::trunc);
			ok = outStrm.is_open ();
		}
		if (ok)
		{
			// Copy the test data file to the target directory
			while (inStrm.good ())
			{
				outStrm.put (static_cast<char>(inStrm.get ()));
			}
			inStrm.close();

			// Add InGcs test data to the output stream,
			ok = csAddInGcsTestData (outStrm);
			outStrm.close ();
		}
	}

	// Open the source test data file.
	if (ok)
	{	
		std::ifstream inStrm;
		std::ofstream outStrm;
		char testDataPathName [512];

		wcstombs (testDataPathName,dictDir,sizeof (testDataPathName));
		CS_stncat (testDataPathName,"\\..\\TestCpp\\TEST_All.DAT",sizeof (testDataPathName));
		inStrm.open (testDataPathName,std::ios::in);
		ok = inStrm.is_open ();
		if (ok)
		{
			wcstombs (testDataPathName,trgDir,sizeof (testDataPathName));
			CS_stncat (testDataPathName,"\\TEST_All.DAT",sizeof (testDataPathName));
			outStrm.open (testDataPathName,std::ios::out | std::ios::trunc);
			ok = outStrm.is_open ();
		}
		if (ok)
		{
			// Copy the test data file to the target directory
			while (inStrm.good ())
			{
				outStrm.put (static_cast<char>(inStrm.get ()));
			}
			inStrm.close ();

			// Add InGcs test data to the output stream,
			ok = csAddInGcsTestData (outStrm);
			outStrm.close ();
		}
	}

	// That should be it.
	return ok;
}

// The following functions imply reads in the source data file and
// parses the data into form indicated by the TcsInGcsSpec structure.
// No processing is done other than to properly populate the structure
// array.
bool csAddInGcsReadData (const wchar_t* dataDir,const wchar_t* dataName)
{
	bool ok;
	unsigned idx;
	std::wifstream inStrm;

	wchar_t* dummy;
	wchar_t csvPathName [512];
	
	std::wstring fldData;

	TcsCsvStatus csvStatus;
	TcsCsvFileBase inGcsCsvFile (true,14,14);

	TcsInGcsSpec xx;					// used only to make sizeof references sane.

	wcscpy (csvPathName,dataDir);
	wcscat (csvPathName,L"\\");
	wcscat (csvPathName,dataName);
	inStrm.open (csvPathName,std::ios::in);
	ok = inStrm.is_open ();
	if (ok)
	{
		ok = inGcsCsvFile.ReadFromStream (inStrm,true,csvStatus);
	}
	for (idx = 0;ok && idx < KcsInGcsSpecCount;idx += 1)
	{
		// There are fourteen fields, which we collapse to 10.
		ok = inGcsCsvFile.GetField (fldData,idx,(short)0,csvStatus);
		if (ok)
		{
			wcstombs (VcsInGcsSpecs [idx].ZoneName,fldData.c_str (),sizeof (xx.ZoneName));
			ok = inGcsCsvFile.GetField (fldData,idx,1,csvStatus);
		}
		if (ok)
		{
			wcstombs (VcsInGcsSpecs [idx].CountyCode,fldData.c_str (),sizeof (xx.CountyCode));
			ok = inGcsCsvFile.GetField (fldData,idx,2,csvStatus);
		}
		if (ok)
		{
			wcstombs (VcsInGcsSpecs [idx].EpsgAbbreviation,fldData.c_str (),sizeof (xx.EpsgAbbreviation));
			ok = inGcsCsvFile.GetField (fldData,idx,3,csvStatus);
		}
		if (ok)
		{
			VcsInGcsSpecs [idx].OriginLatitude = wcstod (fldData.c_str (),&dummy);
			ok = inGcsCsvFile.GetField (fldData,idx,4,csvStatus);
		}
		if (ok)
		{
			VcsInGcsSpecs [idx].OriginLatitude += (wcstod (fldData.c_str (),&dummy) / 60.0);
			ok = inGcsCsvFile.GetField (fldData,idx,5,csvStatus);
		}
		if (ok)
		{
			VcsInGcsSpecs [idx].OriginLatitude += (wcstod (fldData.c_str (),&dummy) / 3600.0);
			ok = inGcsCsvFile.GetField (fldData,idx,6,csvStatus);
		}
		if (ok)
		{
			VcsInGcsSpecs [idx].OriginLongitude = wcstod (fldData.c_str (),&dummy);
			ok = inGcsCsvFile.GetField (fldData,idx,7,csvStatus);
		}
		if (ok)
		{
			VcsInGcsSpecs [idx].OriginLongitude += (wcstod (fldData.c_str (),&dummy) / 60.0);
			ok = inGcsCsvFile.GetField (fldData,idx,8,csvStatus);
		}
		if (ok)
		{
			VcsInGcsSpecs [idx].OriginLongitude += (wcstod (fldData.c_str (),&dummy) / 3600.0);
			ok = inGcsCsvFile.GetField (fldData,idx,9,csvStatus);
		}
		if (ok)
		{
			VcsInGcsSpecs [idx].ScaleReduction = wcstod (fldData.c_str (),&dummy);
			ok = inGcsCsvFile.GetField (fldData,idx,10,csvStatus);
		}
		if (ok)
		{
			// Commas are a problem here.
			VcsInGcsSpecs [idx].FalseEasting = CS_wc2d (fldData.c_str ());
			ok = inGcsCsvFile.GetField (fldData,idx,11,csvStatus);
		}
		if (ok)
		{
			// Commas are a problem here.
			VcsInGcsSpecs [idx].FalseNorthing = CS_wc2d (fldData.c_str ());
			ok = inGcsCsvFile.GetField (fldData,idx,12,csvStatus);
		}
		if (ok)
		{
			// Commas are a problem here.
			VcsInGcsSpecs [idx].TestEasting = CS_wc2d (fldData.c_str ());
			ok = inGcsCsvFile.GetField (fldData,idx,13,csvStatus);
		}
		if (ok)
		{
			// Commas are a problem here.
			VcsInGcsSpecs [idx].TestNorthing = CS_wc2d (fldData.c_str ());
		}
		// Need to flip the sign onlongitude, Indiana is west of Greenwich.
		VcsInGcsSpecs [idx].OriginLongitude = -VcsInGcsSpecs [idx].OriginLongitude;

		VcsInGcsSpecs [idx].EpsgCodeMetric = 0UL;
		VcsInGcsSpecs [idx].EpsgCodeUsFoot = 0UL;
		VcsInGcsSpecs [idx].FalseEastingFeet = 0UL;
		VcsInGcsSpecs [idx].FalseNorthingFeet = 0UL;
		VcsInGcsSpecs [idx].TestEastingFeet = 0UL;
		VcsInGcsSpecs [idx].TestNorthingFeet = 0UL;
		memset (VcsInGcsSpecs [idx].EpsgNameMetric,'\0',sizeof (xx.EpsgNameMetric));
		memset (VcsInGcsSpecs [idx].EpsgNameUsFoot,'\0',sizeof (xx.EpsgNameUsFoot));
		memset (VcsInGcsSpecs [idx].EpsgAliasMetric,'\0',sizeof (xx.EpsgAliasMetric));
		memset (VcsInGcsSpecs [idx].EpsgAliasUsFoot,'\0',sizeof (xx.EpsgAliasUsFoot));
		memset (VcsInGcsSpecs [idx].EpsgZoneMetric,'\0',sizeof (xx.EpsgZoneMetric));
		memset (VcsInGcsSpecs [idx].EpsgZoneUsFoot,'\0',sizeof (xx.EpsgZoneUsFoot));
		memset (VcsInGcsSpecs [idx].CsMapKeyNameMetric,'\0',sizeof (xx.CsMapKeyNameMetric));
		memset (VcsInGcsSpecs [idx].CsMapKeyNameUsFoot,'\0',sizeof (xx.CsMapKeyNameUsFoot));
		memset (VcsInGcsSpecs [idx].CsMapDescriptionMetric,'\0',sizeof (xx.CsMapDescriptionMetric));
		memset (VcsInGcsSpecs [idx].CsMapDescriptionUsFoot,'\0',sizeof (xx.CsMapDescriptionUsFoot));
		VcsInGcsSpecs [idx].DupSortCode = 0;
		VcsInGcsSpecs [idx].MainEpsgWritten = false;
		VcsInGcsSpecs [idx].MinLongitude = 0.0;
		VcsInGcsSpecs [idx].MinLatitude = 0.0;
		VcsInGcsSpecs [idx].MaxLongitude = 0.0;
		VcsInGcsSpecs [idx].MaxLatitude = 0.0;
	}
	return ok;
}
double CS_wc2d (const wchar_t* wcNbr)
{
	long32_t format;
	double rtnValue;
	char ccTemp [64];

	wcstombs (ccTemp,wcNbr,sizeof (ccTemp));
	format = CS_atof (&rtnValue,ccTemp);
	if (format < 0)
	{
		rtnValue = 0.0;
	}
	return rtnValue;
}
// This module extracts EPSG names and numbers from the current EPSG database.
bool csAddInGcsAddEpsgData (const wchar_t* epsgDir)
{
	bool ok;

	unsigned idx;
	unsigned recordNbr;
	unsigned invalidRecNbr;
	wchar_t lkUpAbbrev [32];
	wchar_t lkUpName [128];

	const TcsEpsgTable* tblCrsPtr (0);
	const TcsEpsgTable* tblAliasPtr (0);
	const TcsEpsgTable* tblAreaPtr (0);

	std::wstring fldData;

	TcsInGcsSpec xx;

	invalidRecNbr = TcsCsvFileBase::GetInvalidRecordNbr ();
	
	// Get a usable copy of the EPSG database.
	TcsEpsgDataSetV6 epsgData (epsgDir);
	ok = epsgData.IsOk ();

	// We're going to need to access several tables in an unusual way.
	if (ok)
	{
		tblCrsPtr = epsgData.GetTablePtr (epsgTblReferenceSystem);
		tblAliasPtr = epsgData.GetTablePtr (epsgTblAlias);
		tblAreaPtr = epsgData.GetTablePtr (epsgTblArea);
	}

	// For each entry in the VcsInGcsSpecs array, we extract the required
	// EPSG Names and numbers.
	for (idx = 0;ok && idx < KcsInGcsSpecCount;idx += 1)
	{
		TcsEpsgCode areaCode;
		double leftLng  (0.0);
		double rightLng (0.0);
		double southLat (0.0);
		double northLat (0.0);

		// Manufacture the EPSG abbreviation name for the metric version
		// of the current system.
		
		mbstowcs (lkUpAbbrev,VcsInGcsSpecs [idx].EpsgAbbreviation,wcCount (lkUpAbbrev));
		swprintf (lkUpName,wcCount (lkUpName),L"InGCS 2011 %s (m)",lkUpAbbrev);
		recordNbr = tblAliasPtr->EpsgLocateFirst (epsgFldAlias,lkUpName,true);
		ok = (recordNbr != invalidRecNbr);
		if (ok)
		{
			wcstombs (VcsInGcsSpecs [idx].EpsgAliasMetric,lkUpName,sizeof (xx.EpsgAliasMetric));
			tblAliasPtr->GetAsEpsgCode (VcsInGcsSpecs [idx].EpsgCodeMetric,recordNbr,epsgFldObjectCode);
		}
		swprintf (lkUpName,wcCount (lkUpName),L"InGCS 2011 %s (ftUS)",lkUpAbbrev);
		recordNbr = tblAliasPtr->EpsgLocateFirst (epsgFldAlias,lkUpName,true);
		ok = (recordNbr != invalidRecNbr);
		if (ok)
		{
			wcstombs (VcsInGcsSpecs [idx].EpsgAliasUsFoot,lkUpName,sizeof (xx.EpsgAliasUsFoot));
			tblAliasPtr->GetAsEpsgCode (VcsInGcsSpecs [idx].EpsgCodeUsFoot,recordNbr,epsgFldObjectCode);
		}

		// Should now have CRS codes.  Use them to get the real EPSG CRS Names.
		if (ok)
		{
			ok = tblCrsPtr->GetField (fldData,VcsInGcsSpecs [idx].EpsgCodeMetric,epsgFldCoordRefSysName);
			wcstombs (VcsInGcsSpecs [idx].EpsgNameMetric,fldData.c_str (),sizeof (xx.EpsgNameMetric));
		}
		if (ok)
		{
			ok = tblCrsPtr->GetField (fldData,VcsInGcsSpecs [idx].EpsgCodeUsFoot,epsgFldCoordRefSysName);
			wcstombs (VcsInGcsSpecs [idx].EpsgNameUsFoot,fldData.c_str (),sizeof (xx.EpsgNameUsFoot));
		}
		// Given the EPSG code value, extract the "Area of Use" EPSG Code.  Then extract from
		// the Area table, the min/max :at/Long for use in making the definition.
		if (ok)
		{
			ok = tblCrsPtr->GetAsEpsgCode (areaCode,VcsInGcsSpecs [idx].EpsgCodeMetric,epsgFldAreaOfUseCode);
		}
		if(ok)
		{
			ok = tblAreaPtr->GetAsReal (leftLng,areaCode,epsgFldAreaWestBoundLng);
		}
		if(ok)
		{
			ok = tblAreaPtr->GetAsReal (rightLng,areaCode,epsgFldAreaEastBoundLng);
		}
		if(ok)
		{
			ok = tblAreaPtr->GetAsReal (southLat,areaCode,epsgFldAreaSouthBoundLat);
		}
		if(ok)
		{
			ok = tblAreaPtr->GetAsReal (northLat,areaCode,epsgFldAreaNorthBoundLat);
		}
		if (ok)
		{
			VcsInGcsSpecs [idx].MinLongitude = leftLng;
			VcsInGcsSpecs [idx].MinLatitude  = southLat;
			VcsInGcsSpecs [idx].MaxLongitude = rightLng;
			VcsInGcsSpecs [idx].MaxLatitude  = northLat;
		}
	}
	return ok;
}
bool csAddInGcsGenerateCsMapData (void)
{
	bool ok (true);

	unsigned idx;

	TcsInGcsSpec xx;			// Used only in sizeof operator.

	for (idx = 0;ok && idx < KcsInGcsSpecCount;idx += 1)
	{
		// Manufacture a CS-MAP coordinate system key name.  Metric first.
		sprintf (VcsInGcsSpecs [idx].CsMapKeyNameMetric,"InGCS11-%s",VcsInGcsSpecs [idx].ZoneName);
		// Use the EPSG CRS name as the CS-MAP Description.
		CS_stncp (VcsInGcsSpecs [idx].CsMapDescriptionMetric,VcsInGcsSpecs [idx].EpsgNameMetric,sizeof (xx.CsMapDescriptionMetric));
		//Now US Feet.
		sprintf (VcsInGcsSpecs [idx].CsMapKeyNameUsFoot,"InGCS11-%s-F",VcsInGcsSpecs [idx].ZoneName);
		CS_stncp (VcsInGcsSpecs [idx].CsMapDescriptionUsFoot,VcsInGcsSpecs [idx].EpsgNameUsFoot,sizeof (xx.CsMapDescriptionUsFoot));
	}
	ok = (idx >= KcsInGcsSpecCount);
	return ok;
}
bool csAddInGcsDupSort (void)
{
	bool ok (true);

	unsigned idx;

	char cntyCode1;
	char cntyCode2;

	char wrkBufr [32];

	for (idx = 0;ok && idx < KcsInGcsSpecCount;idx += 1)
	{
		cntyCode1 = VcsInGcsSpecs [idx].CountyCode [0];
		cntyCode2 = VcsInGcsSpecs [idx].CountyCode [1];
		memset (wrkBufr,'\0',sizeof (wrkBufr));
		CS_stncp (wrkBufr,VcsInGcsSpecs [idx].EpsgAbbreviation,sizeof (wrkBufr));
		if (wrkBufr [2] == '\0')
		{
			VcsInGcsSpecs [idx].DupSortCode = 0;
		}
		else
		{
			if (wrkBufr [0] == cntyCode1 && wrkBufr [1] == cntyCode2)
			{
				VcsInGcsSpecs [idx].DupSortCode = 10;
			}
			if (wrkBufr [3] == cntyCode1 && wrkBufr [4] == cntyCode2)
			{
				VcsInGcsSpecs [idx].DupSortCode = 11;
			}
			if (wrkBufr [6] == cntyCode1 && wrkBufr [7] == cntyCode2)
			{
				VcsInGcsSpecs [idx].DupSortCode = 12;
			}
		}
	}
	ok = (idx >= KcsInGcsSpecCount);
	return ok;
}
bool csAddInGcsCalcUsFoot (void)
{
	bool ok (true);

	unsigned idx;

	double mToFt = (3937.0 / 1200.00);

	for (idx = 0;ok && idx < KcsInGcsSpecCount;idx += 1)
	{
		VcsInGcsSpecs [idx].FalseEastingFeet  = VcsInGcsSpecs [idx].FalseEasting  * mToFt;
		VcsInGcsSpecs [idx].FalseNorthingFeet = VcsInGcsSpecs [idx].FalseNorthing * mToFt;
		VcsInGcsSpecs [idx].TestEastingFeet   = VcsInGcsSpecs [idx].TestEasting   * mToFt;
		VcsInGcsSpecs [idx].TestNorthingFeet  = VcsInGcsSpecs [idx].TestNorthing  * mToFt;
	}
	return ok;
}
bool csAddInGcsBuildCrs (TcsAscDefinition& csMapCrsDef,unsigned idx)
{
	bool ok (true);
	
	long tmpLng;
	double tmpDbl;
	

	char wrkBufr [128];
	
	// We get here once for each zone.  We build the metric definition
	/// which will be modified (only four changes) by the calling module
	// to get the US Foot version.  The TcsAscDefinition given to us
	// will already have the first entry "CS_NAME:" in it.  This became
	// necessary as the AscFixer code does some special stuff with the
	// first entry upon construction.  If I had remembered that, I would
	// have had this function construct the object; but that's history
	// now.
	
	// This gets quite painful.  Almost now wish I opted for doing this
	// project manually with a lot of cut and pasting.  There are about
	// 22 lines, almost each has its own individual character.  Using cut-paste,
	// brute forcing it appears to be the quickest way to get this job done.
	// Ideally, this program will only be used once.
	if (ok)
	{
		const TcsDefLine crsLine (dictTypCoordsys,"DESC_NM:",VcsInGcsSpecs [idx].CsMapDescriptionMetric,0);
		ok = csMapCrsDef.Append (crsLine);
	}
	if (ok)
	{
		const TcsDefLine crsLine (dictTypCoordsys,"GROUP:","OTHR-US",0);
		ok = csMapCrsDef.Append (crsLine);
	}
	if (ok)
	{
		const TcsDefLine crsLine (dictTypCoordsys,"SOURCE:","Indiana DOT: www.in.gov/indot/3397.htm",0);
		ok = csMapCrsDef.Append (crsLine);
	}
	if (ok)
	{
		sprintf (wrkBufr,"%lu",static_cast<unsigned long>(VcsInGcsSpecs [idx].EpsgCodeMetric));
		const TcsDefLine crsLine (dictTypCoordsys,"EPSG:",wrkBufr,0);
		ok = csMapCrsDef.Append (crsLine);
	}
	if (ok)
	{
		// Oracle SRID number qill invariably be the EPSG code, but we don't
		// know that yet.  We'll leave it zero unless the client decides to
		// have the EPSG code value used.
		const TcsDefLine crsLine (dictTypCoordsys,"SRID:","0",0);
		ok = csMapCrsDef.Append (crsLine);
	}
	if (ok)
	{
		const TcsDefLine crsLine (dictTypCoordsys,"DT_NAME:","NSRS11",0);
		ok = csMapCrsDef.Append (crsLine);
	}
	if (ok)
	{
		const TcsDefLine crsLine (dictTypCoordsys,"PROJ:","TM",0);
		ok = csMapCrsDef.Append (crsLine);
	}
	if (ok)
	{
		const TcsDefLine crsLine (dictTypCoordsys,"UNIT:","METER",0);
		ok = csMapCrsDef.Append (crsLine);
	}
	if (ok)
	{
		long32_t format;
		format = (cs_ATOF_MINSEC | cs_ATOF_DIRCHR | cs_ATOF_XEAST | cs_ATOF_MINSEC0) + 1;
		CS_ftoa (wrkBufr,sizeof (wrkBufr),VcsInGcsSpecs [idx].OriginLongitude,format);
		const TcsDefLine crsLine (dictTypCoordsys,"PARM1:",wrkBufr,0);
		ok = csMapCrsDef.Append (crsLine);
	}
	if (ok)
	{
		sprintf (wrkBufr,"%.6f",VcsInGcsSpecs [idx].ScaleReduction);
		const TcsDefLine crsLine (dictTypCoordsys,"SCL_RED:",wrkBufr,0);
		ok = csMapCrsDef.Append (crsLine);
	}
	if (ok)
	{
		long32_t format;
		format = (cs_ATOF_MINSEC | cs_ATOF_DIRCHR | cs_ATOF_MINSEC0) + 1;
		CS_ftoa (wrkBufr,sizeof (wrkBufr),VcsInGcsSpecs [idx].OriginLatitude,format);
		const TcsDefLine crsLine (dictTypCoordsys,"ORG_LAT:",wrkBufr,0);
		ok = csMapCrsDef.Append (crsLine);
	}
	if (ok)
	{
		long32_t format;
		format = cs_ATOF_COMMA + 2;
		CS_ftoa (wrkBufr,sizeof (wrkBufr),VcsInGcsSpecs [idx].FalseEasting,format);
		const TcsDefLine crsLine (dictTypCoordsys,"X_OFF:",wrkBufr,0);
		ok = csMapCrsDef.Append (crsLine);
	}
	if (ok)
	{
		long32_t format;
		format = cs_ATOF_COMMA + 2;
		CS_ftoa (wrkBufr,sizeof (wrkBufr),VcsInGcsSpecs [idx].FalseNorthing,format);
		const TcsDefLine crsLine (dictTypCoordsys,"Y_OFF:",wrkBufr,0);
		ok = csMapCrsDef.Append (crsLine);
	}
	if (ok)
	{
		long32_t format;
		format = (cs_ATOF_MINSEC | cs_ATOF_DIRCHR | cs_ATOF_XEAST | cs_ATOF_MINSEC0) + 1;
		tmpLng = static_cast<long>(VcsInGcsSpecs [idx].MinLongitude * 60.0 - 1.0);
		tmpDbl = static_cast<double>(tmpLng) / 60.0;
		CS_ftoa (wrkBufr,sizeof (wrkBufr),tmpDbl,format);
		const TcsDefLine crsLine (dictTypCoordsys,"MIN_LNG:",wrkBufr,0);
		ok = csMapCrsDef.Append (crsLine);
	}
	if (ok)
	{
		long32_t format;
		format = (cs_ATOF_MINSEC | cs_ATOF_DIRCHR | cs_ATOF_MINSEC0) + 1;
		tmpLng = static_cast<long>(VcsInGcsSpecs [idx].MinLatitude * 60.0 - 1.0);
		tmpDbl = static_cast<double>(tmpLng) / 60.0;
		CS_ftoa (wrkBufr,sizeof (wrkBufr),tmpDbl,format);
		const TcsDefLine crsLine (dictTypCoordsys,"MIN_LAT:",wrkBufr,0);
		ok = csMapCrsDef.Append (crsLine);
	}
	if (ok)
	{
		long32_t format;
		format = (cs_ATOF_MINSEC | cs_ATOF_DIRCHR | cs_ATOF_XEAST | cs_ATOF_MINSEC0) + 1;
		tmpLng = static_cast<long>(VcsInGcsSpecs [idx].MaxLongitude * 60.0 + 1.0);
		tmpDbl = static_cast<double>(tmpLng) / 60.0;
		CS_ftoa (wrkBufr,sizeof (wrkBufr),tmpDbl,format);
		const TcsDefLine crsLine (dictTypCoordsys,"MAX_LNG:",wrkBufr,0);
		ok = csMapCrsDef.Append (crsLine);
	}
	if (ok)
	{
		long32_t format;
		format = (cs_ATOF_MINSEC | cs_ATOF_DIRCHR | cs_ATOF_MINSEC0) + 1;
		tmpLng = static_cast<long>(VcsInGcsSpecs [idx].MaxLatitude * 60.0 + 1.0);
		tmpDbl = static_cast<double>(tmpLng) / 60.0;
		CS_ftoa (wrkBufr,sizeof (wrkBufr),tmpDbl,format);
		const TcsDefLine crsLine (dictTypCoordsys,"MAX_LAT:",wrkBufr,0);
		ok = csMapCrsDef.Append (crsLine);
	}
	if (ok)
	{
		const TcsDefLine crsLine (dictTypCoordsys,"QUAD:","1",0);
		ok = csMapCrsDef.Append (crsLine);
	}	if (ok)
	{
		const TcsDefLine crsLine (dictTypCoordsys,"EPSG_QD:","1",0);
		ok = csMapCrsDef.Append (crsLine);
	}
	if (ok)
	{
		const TcsDefLine crsLine (dictTypCoordsys,"MAP_SCL:","1.0",0);
		ok = csMapCrsDef.Append (crsLine);
	}
	return ok;
}
bool csAddInGcsToUsFeet (TcsAscDefinition& csMapCrsDef,unsigned idx)
{
	bool ok;

	TcsDefLine* linePtr;
	char wrkBufr [128];

	// This function converts the provided Asc Definition from meters
	// to feet.  Six elements of the definition need to changed.
	linePtr = csMapCrsDef.GetLine ("CS_NAME:");
	ok = (linePtr != NULL);
	if (ok)
	{
		linePtr->SetValue (VcsInGcsSpecs [idx].CsMapKeyNameUsFoot);
		linePtr = csMapCrsDef.GetLine ("DESC_NM:");
		ok = (linePtr != NULL);
	}
	if (ok)
	{
		linePtr->SetValue (VcsInGcsSpecs [idx].CsMapDescriptionUsFoot);
		linePtr = csMapCrsDef.GetLine ("EPSG:");
		ok = (linePtr != NULL);
	}
	if (ok)
	{
		sprintf (wrkBufr,"%lu",static_cast<unsigned long>(VcsInGcsSpecs [idx].EpsgCodeUsFoot));
		linePtr->SetValue (wrkBufr);
		linePtr = csMapCrsDef.GetLine ("UNIT:");
		ok = (linePtr != NULL);
	}
	if (ok)
	{
		linePtr->SetValue ("FOOT");
		linePtr = csMapCrsDef.GetLine ("X_OFF:");
		ok = (linePtr != NULL);
	}
	if (ok)
	{
		long32_t format;
		char wrkBufr [64];
		format = cs_ATOF_COMMA + 2;
		CS_ftoa (wrkBufr,sizeof (wrkBufr),VcsInGcsSpecs [idx].FalseEastingFeet,format);
		linePtr->SetValue (wrkBufr);
		linePtr = csMapCrsDef.GetLine ("Y_OFF:");
		ok = (linePtr != NULL);
	}	
	if (ok)
	{
		long32_t format;
		char wrkBufr [64];
		format = cs_ATOF_COMMA + 2;
		CS_ftoa (wrkBufr,sizeof (wrkBufr),VcsInGcsSpecs [idx].FalseNorthingFeet,format);
		linePtr->SetValue (wrkBufr);
	}
	return ok;
}
bool csAddInGcsCategories (TcsCategory* categoryPtr)
{
	bool ok (true);

	unsigned idx;

	for (idx = 0;ok && idx < KcsInGcsSpecCount;idx += 1)
	{
		const TcsCategoryItem metricItm (VcsInGcsSpecs [idx].CsMapKeyNameMetric,VcsInGcsSpecs [idx].CsMapDescriptionMetric);
		categoryPtr->AddItem (metricItm);
	}
	for (idx = 0;ok && idx < KcsInGcsSpecCount;idx += 1)
	{
		const TcsCategoryItem footItm (VcsInGcsSpecs [idx].CsMapKeyNameUsFoot,VcsInGcsSpecs [idx].CsMapDescriptionUsFoot);
		categoryPtr->AddItem (footItm);
	}
	return ok;
}
bool csAddInGcsNameMap (TcsNameMapper& nameMapper)
{
	bool ok (true);
	unsigned idx;
	wchar_t wcTemp [128];

	// Loop once for each zone.  Add a whole flock of entries, twice.
	// One flock for the metric systems; a second flock for the US Foot systems.
	// The order in which we write to the NameMapper file is imaterial as
	// the NameMapper re-orders per its needs upon loading the .csv file.
	// This loop, we do the metric systems.
	for (idx = 0;ok && idx < KcsInGcsSpecCount;idx += 1)
	{
		// Establish the generic ID for all these entries.  In this case, since
		// all zones have an EPSG ID number, the generic ID is the EPSG ID in
		// all cases.
		TcsGenericId genericId = VcsInGcsSpecs [idx].EpsgCodeMetric;

		// Establish the base entry, which is the EPSG entry with the CRS
		// table system name.
		mbstowcs (wcTemp, VcsInGcsSpecs [idx].EpsgNameMetric,wcCount (wcTemp));
		TcsNameMap epsgItm (genericId,csMapProjectedCSysKeyName,csMapFlvrEpsg,
																static_cast<unsigned long>(genericId),
																wcTemp);
		nameMapper += epsgItm;

		// Establish the Abbreviation as an EPSG flavored alias to the
		// CRS table name.
		mbstowcs (wcTemp, VcsInGcsSpecs [idx].EpsgAliasMetric,wcCount (wcTemp));
		epsgItm.SetAliasFlag (1);
		epsgItm.SetNameId (wcTemp);
		nameMapper += epsgItm;

		// Autodesk flavor.
		mbstowcs (wcTemp, VcsInGcsSpecs [idx].CsMapKeyNameMetric,wcCount (wcTemp));
		TcsNameMap aDeskItm (genericId,csMapProjectedCSysKeyName,csMapFlvrAutodesk,
																 0UL,
																 wcTemp);
		if (VcsInGcsSpecs [idx].DupSortCode != 0)
		{
			aDeskItm.SetDupSort (VcsInGcsSpecs [idx].DupSortCode);
		}
		if (VcsInGcsSpecs [idx].DupSortCode > 10)
		{
			aDeskItm.SetAliasFlag (1);
		}
		nameMapper += aDeskItm;

		// Again with ther CsMap flavor.
		TcsNameMap csMapItm (genericId,csMapProjectedCSysKeyName,csMapFlvrCsMap,
																 0UL,
																 wcTemp);
		if (VcsInGcsSpecs [idx].DupSortCode != 0)
		{
			csMapItm.SetDupSort (VcsInGcsSpecs [idx].DupSortCode);
		}
		if (VcsInGcsSpecs [idx].DupSortCode > 10)
		{
			csMapItm.SetAliasFlag (1);
		}
		nameMapper += csMapItm;
	}
	// Now we do the US Foot systems.
	for (idx = 0;ok && idx < KcsInGcsSpecCount;idx += 1)
	{
		// Establish the generic ID for all these entries.  In this case, since
		// all zones have an EPSG ID number, the generic ID is the EPSG ID in
		// all cases.
		TcsGenericId genericId = VcsInGcsSpecs [idx].EpsgCodeUsFoot;

		// Establish the base entry, which is the EPSG entry with the CRS
		// table system name.
		mbstowcs (wcTemp, VcsInGcsSpecs [idx].EpsgNameUsFoot,wcCount (wcTemp));
		TcsNameMap epsgItm (genericId,csMapProjectedCSysKeyName,csMapFlvrEpsg,
																static_cast<unsigned long>(genericId),
																wcTemp);
		nameMapper += epsgItm;

		// Establish the Abbreviation as an EPSG flavored alias to the
		// CRS table name.
		mbstowcs (wcTemp, VcsInGcsSpecs [idx].EpsgAliasUsFoot,wcCount (wcTemp));
		epsgItm.SetAliasFlag (1);
		epsgItm.SetNameId (wcTemp);
		nameMapper += epsgItm;

		// For the Autodeak Flavor
		mbstowcs (wcTemp, VcsInGcsSpecs [idx].CsMapKeyNameUsFoot,wcCount (wcTemp));
		TcsNameMap aDeskItm (genericId,csMapProjectedCSysKeyName,csMapFlvrAutodesk,
																 0UL,
																 wcTemp);
		if (VcsInGcsSpecs [idx].DupSortCode != 0)
		{
			aDeskItm.SetDupSort (VcsInGcsSpecs [idx].DupSortCode);
		}
		if (VcsInGcsSpecs [idx].DupSortCode > 10)
		{
			aDeskItm.SetAliasFlag (1);
		}
		nameMapper += aDeskItm;

		// Again with ther CsMap flavor.
		TcsNameMap csMapItm (genericId,csMapProjectedCSysKeyName,csMapFlvrCsMap,
																 0UL,
																 wcTemp);
		if (VcsInGcsSpecs [idx].DupSortCode != 0)
		{
			csMapItm.SetDupSort (VcsInGcsSpecs [idx].DupSortCode);
		}
		if (VcsInGcsSpecs [idx].DupSortCode > 10)
		{
			csMapItm.SetAliasFlag (1);
		}
		nameMapper += csMapItm;
	}
	return true;
}
bool csAddInGcsTestData (std::ostream& outStrm)
{
	bool ok (true);

	unsigned idx;
	char wrtBufr [256];

	for (idx = 0;ok && idx < KcsInGcsSpecCount;idx += 1)
	{
		sprintf (wrtBufr,"NSRS11.LL,-85.000,42.000,%s,%.3f,%.3f,0.002,0.002",VcsInGcsSpecs [idx].CsMapKeyNameMetric,
																			 VcsInGcsSpecs [idx].TestEasting,
																			 VcsInGcsSpecs [idx].TestNorthing);
		outStrm << wrtBufr << std::endl;
		sprintf (wrtBufr,"%s,%.3f,%.3f,NSRS11.LL,-85.000,42.000,2.0E-07,2.0E-07",VcsInGcsSpecs [idx].CsMapKeyNameMetric,
																				 VcsInGcsSpecs [idx].TestEasting,
																				 VcsInGcsSpecs [idx].TestNorthing);
		outStrm << wrtBufr << std::endl;
	}
	for (idx = 0;ok && idx < KcsInGcsSpecCount;idx += 1)
	{
		sprintf (wrtBufr,"NSRS11.LL,-85.000,42.000,%s,%.3f,%.3f,0.003,0.003",VcsInGcsSpecs [idx].CsMapKeyNameUsFoot,
																		   VcsInGcsSpecs [idx].TestEastingFeet,
																		   VcsInGcsSpecs [idx].TestNorthingFeet);
		outStrm << wrtBufr << std::endl;
		sprintf (wrtBufr,"%s,%.3f,%.3f,NSRS11.LL,-85.000,42.000,3.0E-07,3.0E-07",VcsInGcsSpecs [idx].CsMapKeyNameUsFoot,
																				 VcsInGcsSpecs [idx].TestEastingFeet,
																				 VcsInGcsSpecs [idx].TestNorthingFeet);
		outStrm << wrtBufr << std::endl;
	}
	return ok;
}
