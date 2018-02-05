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


// The following list of include files includes one or more files which
// are not really referenced in all modules.  This list, and the specific
// order thereof, isoptimized for the use of pre-compiled headers.  As
// these modules will compiled hunreds of times more freequently than they
// are changed, a small price to pay for build efficiency which is significant.
/*lint -e766 */		/* Disable PC-Lint's warning of unreferenced headers */

#include "cs_map.h"
#include "cs_NameMapper.hpp"
//  cs_NameMapper.hpp includes cs_CsvFileSupport.hpp
//  cs_NameMapper.hpp includes csNameMapperSupport.hpp
#include "cs_WktObject.hpp"
#include "cs_wkt.h"

#include "cs_Legacy.h"
#include "cs_EpsgStuff.h"
#include "csUtilitiesSupport.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>

#if defined (_MSC_VER) && _MSC_VER >= 800	/* MS Visual C++ 1.0 or later */
#pragma warning(disable:4702)			// unreachable code
#endif
#include <set>
#if defined (_MSC_VER) && _MSC_VER >= 800	/* MS Visual C++ 1.0 or later */
#pragma warning(default:4702)
#endif
#include <string>

#if defined (_MSC_VER) && _MSC_VER >= 800	/* MS Visual C++ 1.0 or later */
#	pragma warning( disable : 4311 )		// 'type cast': pointer truncation from 'void *' to 'unsigned long'
#	pragma warning( disable : 4302 )		// 'type cast': truncation from 'void *' to 'unsigned long'
#endif

#ifndef wcCount
#	define wcCount(array) (sizeof (array) / sizeof (wchar_t))
#endif

// Declare various utility functions.  Several are obsolete and
// should be removed as their future value is nil.
bool ManufactureNameMapperCsv (const wchar_t* resultDir,const wchar_t* dataDir);
bool ResortNameMapperCsv (const wchar_t* resultDir,const wchar_t* dataDir,bool duplicates = false);
bool csWriteNsrsList (const wchar_t* csDictSrcDir,const wchar_t* csTrgDataFileDir);
bool csAddNsrs07Nsrs11 (const wchar_t* csDictSrcDir,const wchar_t* csDictTrgDir);
bool AddOracle9Mappings (const wchar_t* csDataDir,TcsCsvStatus& status);
bool AddOracle10Mappings (const wchar_t* csDataDir,TcsCsvStatus& status);
bool AddSequenceNumbers (const wchar_t* dataDir);
bool ListEpsgCodes (const wchar_t* dictDir);
bool ThreeParameterFixer (const char* csDictSrcDir,const char* csDictTrgDir);
bool GeocentricFixer (const char* csDictSrcDir,const char* csDictTrgDir);
bool ListUnmappedEpsgCodes (const wchar_t* epsgDir,const wchar_t* dictDir);
bool ListDuplicateDefinitions (const wchar_t* listPath,const wchar_t* dictDir);
bool DeprecateDupliateDefs (const wchar_t* srcDictDir,const wchar_t* trgDictDir);
bool csAddEpsgCodes (const wchar_t* csDictSrcDir,const wchar_t* epsgDir,const wchar_t* csDictTrgDir);
//bool csGenerateHpgnTable (const wchar_t* tablePath,const wchar_t* dictDir);
bool csOrgTransformations (const wchar_t* csDictDir,const wchar_t* csDictTrgDir);
bool csGenerateRegressTestFile (const wchar_t* csFileName,const wchar_t* csDataDir,const wchar_t* csDictDir);
bool csGenerateBlueBookTestData (const wchar_t* testDataDir,bool phaseTwo);
bool ReplaceOldHpgnCrsNames (const wchar_t* dataDir,const wchar_t* resultDir);
bool Epsg706Updates (const wchar_t* srcDictDir,const wchar_t* dataDir,const wchar_t* trgDictDir);
bool csCrsNamesToSource (const wchar_t* csvPath,const wchar_t* csvCrsNames,const wchar_t* srcFileName);
bool SevenParameterFix (const wchar_t* csDictTrgDir,const wchar_t* csDataTrgDir,const wchar_t* csDictSrcDir,const wchar_t* csDataSrcDir);
bool SevenParameterFlipList (std::wostream& listStrm,const wchar_t* dictDir);
bool csUsefulRangeReport (const wchar_t* reportDir,const wchar_t* csDictDir);
bool csUsefulRangeMatchList (const wchar_t* reportDir,const wchar_t* csDictDir);
bool csWriteEsriWktTestFile (const wchar_t* csDataSrcDir,const wchar_t* csDataTrgDir);
bool csWriteNsrsAudit (const wchar_t* csDictSrcDir,const wchar_t* csTrgDataFileDir);
bool csUpdateNameMapperFromCsv (const wchar_t* csDictTrgDir,const wchar_t* csDictSrcDir,const wchar_t* srcCsvFullPath);
bool csGenerate48Hpgn (const wchar_t* csDictDir,const wchar_t* epsgPolygonDir,bool verbose = false);
bool csGenerate48HpgnTest (const wchar_t* csDictDir);
bool csCsdToCsvEL (const wchar_t* csDictDir,bool incLegacy = false);
bool csCsdToCsvDT (const wchar_t* csDictDir,bool incLegacy = false);
bool csCsdToCsvCS (const wchar_t* csDictDir,bool incLegacy = false);
bool csCsdToCsvCT (const wchar_t* csDictDir,bool incLegacy = false);
bool csCsdToCsvGX (const wchar_t* csDictDir,bool incLegacy = false);
bool csCsdToCsvGP (const wchar_t* csDictDir,bool incLegacy = false);
// This range transfer transfers from one Coordsys.ASC definition to another.
bool csUsefulRangeTransfer (const wchar_t* csDictDir,int ticketNbr);
// This range transfer transfers from EPSG to Coordsys.asc
bool csUsefulRangeXfer (const wchar_t* csDictDir,const wchar_t* csDataDir,int ticketNbr);

bool OracleTxt2WktTest (const wchar_t* csDictDir,const wchar_t* csDataDir,const wchar_t* inputFile,
																		  const wchar_t* delimiters,
																		  const wchar_t* oracleVersion);
bool csGetNsrs2011EpsgCodes (const TcsEpsgDataSetV6* epsgPtr);
bool csGetNsrs2011EsriCodes (void);
bool csFixNsrs2011 (const wchar_t* csDictDir,const wchar_t* csTempDir);
bool PreProcessWktCatalog (const wchar_t* csDataTrgDir,const wchar_t* csDataSrcDir);

bool csAddInGCS (const wchar_t* trgDir,const wchar_t* dataDir,const wchar_t* dataName,
															  const wchar_t* dictDir,
															  const wchar_t* epsgDir);

bool csDeprecateWiHpgn (const wchar_t* trgDictDir,const wchar_t* srcDictDir);


// Various structures used in the various utilities.  Again, the
// future value of many of these is nil, and they should be
// removed.
struct csWktPrmNameMap_
{
	enum EcsWktParameter PrmCode;
	unsigned short MsiParmCode;
	unsigned short EpsgNbr;
	char WktOgcName [48];
	char WktGeoTiffName [48];
	char WktEsriName [48];
	char WktOracleName [48];
	char WktGeoToolsName [48];
	char EpsgName   [64];
	char AppAltName [64];
	char LclAltName [64];
};

struct csWktUnitNameMap_
{
	char MsiName [16];					/* MSI unit name. */
	unsigned short EpsgNbr;				/* EPSG "Unit of Measure" code number */
	char EpsgName [32];					/* EPSG name as of v6.2 */
	char WktOgcName [32];				/* WKT OGC name, this may be the best choice */
	char WktGeoTiffName [32];			/* WKT GeoTiff name */ 
	char WktEsriName [32];				/* WKT ESRI name */
	char WktOracleName [32];			/* WKT Oracle name */
	char WktGeoToolsName [32];			/* WKT Geo Tools name */
	char AppAltName [32];
	char LclAltName [32];
};

// Declare useful CS-MAP data tables.
extern "C" struct csWktPrjNameMap_ csWktPrjNameMap [];
extern "C" struct csWktPrmNameMap_ csWktPrmNameMap [];
extern "C" struct csWktUnitNameMap_ csWktUnitNameMap [];
extern "C" struct cs_Prjtab_ cs_Prjtab [];
extern "C" struct cs_Prjprm_ csPrjprm [];
extern "C" char cs_Dir [];
extern "C" char* cs_DirP;
extern "C" char cs_DirsepC;
extern "C" char cs_ExtsepC;
extern "C" char cs_OptchrC;

