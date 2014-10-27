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

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#pragma warning(disable:4702)
#include <set>
#pragma warning(default:4702)

#include "cs_map.h"
#include "cs_Legacy.h"
#include "cs_wkt.h"
#include "csNameMapper.hpp"
#include "csepsgstuff.h"
#include "csUtilitiesSupport.hpp"

#ifndef wcCount
#	define wcCount(array) (sizeof (array) / sizeof (wchar_t))
#endif

// Decalre various utility functions.  Several are obsolete and
// should be removed as their future value is nil.
bool ManufactureNameMapperCsv (const wchar_t* resultDir,const wchar_t* dataDir);
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
bool ReplaceOldHpgnCrsNames (const wchar_t* dataDir,const wchar_t* resultDir);
bool Epsg706Updates (const wchar_t* srcDictDir,const wchar_t* dataDir,const wchar_t* trgDictDir);
bool csCrsNamesToSource (const wchar_t* csvPath,const wchar_t* csvCrsNames,const wchar_t* srcFileName);
bool SevenParameterFix (const wchar_t* csDictTrgDir,const wchar_t* csDataTrgDir,const wchar_t* csDictSrcDir,const wchar_t* csDataSrcDir);
bool SevenParameterFlipList (std::wostream& listStrm,const wchar_t* dictDir);

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

