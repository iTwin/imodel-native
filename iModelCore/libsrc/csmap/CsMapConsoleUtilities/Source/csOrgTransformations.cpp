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

bool csWriteTransformationAsc (std::wofstream& gtStrm,std::wofstream& gpStrm,
													  const cs_Dtdef_* dtDefPtr,
													  const TcsDefFile& mregAsc,
													  const char* dtmPivot);
bool csAdjustSrcAndTrg (std::wofstream& gpStrm,char *srcDatum,char *trgDatum,
															  const cs_Dtdef_* dtDefPtr,
															  const char* dtmPivot);
bool csWriteGeodeticPath (std::wofstream& gpStrm,const cs_GeodeticPath_ *gpPtr);
bool csConvertMrtFile (cs_GeodeticTransform_::csGeodeticXformParameters::csGeodeticXformParmsDmaMulReg_* mulrgParms,
																		 const cs_Dtdef_ *dtDefPtr,
																		 const TcsDefFile& mregAsc);
bool csExtractMrtRange (double lng [2],double lat [2],const cs_Dtdef_ *dtDefPtr);
bool csWriteHardCodedStuff (std::wofstream& gtStrm,std::wofstream& gpStrm,const wchar_t* csDictDir);
bool csNewHpgnDatums (std::wofstream& gtStrm,std::wofstream& gpStrm,const wchar_t* csDictDir,const wchar_t* csDictTrgDir);
bool csNewHpgnDatumsPhaseOne (TcsDefFile& datumsAsc);
bool csNewHpgnDatumsPhaseTwo (std::wofstream& gtStrm);
bool csNewHpgnDatumsPhaseThree (TcsDefFile& coordsysAsc);
bool csNewHpgnDatumsPhaseFour (TcsDefFile& coordsysAsc);
bool csNewHpgnDatumsPhaseFive (TcsCategoryFile& categoryAsc,TcsDefFile& coordsysAsc);
bool csXformToStream (std::wofstream& gtStrm,cs_GeodeticTransform_* xfrmPtr);


const TcsGdcFile* gdcAgd66ToGda94 (0);
const TcsGdcFile* gdcAgd84ToGda94 (0);
const TcsGdcFile* gdcAts77ToCsrs (0);
const TcsGdcFile* gdcCh1903ToPlus (0);
const TcsGdcFile* gdcDhdnToEtrf89 (0);
const TcsGdcFile* gdcEd50ToEtrf89 (0);
const TcsGdcFile* gdcNad27ToAts77 (0);
const TcsGdcFile* gdcNad27ToCsrs (0);
const TcsGdcFile* gdcNad27ToNad83 (0);
const TcsGdcFile* gdcNad83ToCsrs (0);
const TcsGdcFile* gdcNad83ToHarn (0);
const TcsGdcFile* gdcNzgd49ToNzgd2K (0);
const TcsGdcFile* gdcRgf93ToNtf (0);
const TcsGdcFile* gdcTokyoToJgd2k (0);

const cs_Dtdef_ cs_Chrts95DtDef =
{
	"CH1903Plus_1",		/* key_nm */
	"BESSEL",			/* ell_knm */
	"",					/* group */
	"",					/* locatn */
	"",					/* cntry_st */
	"",					/* fill */
	674.374,			/* DeltaX */
	15.056,				/* DeltaY */
	405.346,			/* DeltaZ */
	0.0,
	0.0,
	0.0,
	0.0,
	"CH1903+ to CHTRF95 (Swiss Terrestrial Reference Frame 1995)", 	/* name */
	"",					/* source */
	0,					/* protect */
	cs_DTCTYP_GEOCTR,	/* to84_via */ 
	6151,				/* epsgNbr */
	0,					/* wktFlvr */
	0,					/* fill01 */
	0,					/* fill02 */
	0,					/* fill03 */
	0,					/* fill04 */
};

// One entry per new datum, and a list of the existing ??HP systems which need
// to be created and reference the new datum.
struct csHarnDatumTable_
{
	char datumCode [16];
	char file1 [16];
	char file2 [16];
	long epsgCode;
	char state1  [8];
	char state2  [8];
	char state3  [8];
	char state4  [8];
	char state5  [8];
	char state6  [8];
	char state7  [8];
	char state8  [8];
	char state9  [8];
	char state10 [8];
	char state11 [8];
	char state12 [8];
	char state13 [8];
	char state14 [8];
	char datumName [64];
	char geogName [64];
	char xfrmName [64];
}
	csHarnDatumTable [] =
{
	{ "AL",  "al" ,       "",  1474L,  "AL",    "",    "",    "",    ""  },
	{ "AR",  "ar" ,       "",  1704L,  "AR",    "",    "",    "",    ""  },
	{ "AZ",  "az" ,       "",  1475L,  "AZ",    "",    "",    "",    ""  },
//	{ "CA",  "ca" ,       "",     0L,  "CA",    "",    "",    "",    ""  },
	{ "CA",  "cn" ,     "cs",  1476L,  "CA",    "",    "",    "",    ""  },	// CN = Califormia North, CS = California South
	{ "CO",  "co" ,       "",  1478L,  "CO",    "",    "",    "",    ""  },
	{ "MT",  "em" ,     "wm",  1481L,  "MT",  "ID",    "",    "",    ""  },
	{ "SA",  "es" ,     "ws",  1579L,  "SA",    "",    "",    "",    ""  },	// EASTERN/WESTERN SAMOA
	{ "TX",  "et" ,     "wt",  1498L,  "TX",    "",    "",    "",    ""  },
	{ "FL",  "fl" ,       "",  1480L,  "FL",    "",    "",    "",    ""  },
	{ "GA",  "ga" ,       "",  1479L,  "GA",    "",    "",    "",    ""  },
	{ "GU",  "gu" ,       "",  1068L,  "GU",    "",    "",    "",    ""  },	// GUAM
	{ "HI",  "hi" ,       "",  1520L,  "HI",    "",    "",    "",    ""  },
	{ "IA",  "ia" ,       "",  1705L,  "IA",    "",    "",    "",    ""  },
	{ "IL",  "il" ,       "",  1553L,  "IL",    "",    "",    "",    ""  },
	{ "IN",  "in" ,       "",  1521L,  "IN",    "",    "",    "",    ""  },
	{ "KS",  "ks" ,       "",  1522L,  "KS",    "",    "",    "",    ""  },
	{ "KY",  "ky" ,       "",  1483L,  "KY",    "",    "",    "",    ""  },
	{ "LA",  "la" ,       "",  1484L,  "LA",    "",    "",    "",    ""  },
	{ "MD",  "md" ,       "",  1485L,  "MD",  "DE",    "",    "",    ""  },
	{ "ME",  "me" ,       "",  1486L,  "ME",    "",    "",    "",    ""  },
	{ "MI",  "mi" ,       "",  1487L,  "MI",    "",    "",    "",    ""  },
	{ "MN",  "mn" ,       "",  1706L,  "MN",    "",    "",    "",    ""  },
	{ "MO",  "mo" ,       "",  1707L,  "MO",    "",    "",    "",    ""  },
	{ "MS",  "ms" ,       "",  1488L,  "MS",    "",    "",    "",    ""  },
	{ "NB",  "nb" ,       "",  1489L,  "NB",    "",    "",    "",    ""  },
	{ "NC",  "nc" ,       "", 15835L,  "NC",    "",    "",    "",    ""  },
	{ "ND",  "nd" ,       "",  1493L,  "ND",    "",    "",    "",    ""  },
	{ "NE",  "ne" ,       "",  1490L,  "NH",  "VT",  "MA",  "CT",  "RI"  },	// NE == New England
	{ "NJ",  "nj" ,       "",  1554L,  "NJ",    "",    "",    "",    ""  },
	{ "NM",  "nm" ,       "",  1491L,  "NM",    "",    "",    "",    ""  },
	{ "NV",  "nv" ,       "",  1523L,  "NV",    "",    "",    "",    ""  },
	{ "NY",  "ny" ,       "",  1492L,  "NY",    "",    "",    "",    ""  },
	{ "OH",  "oh" ,       "",  1524L,  "OH",    "",    "",    "",    ""  },
	{ "OK",  "ok" ,       "",  1494L,  "OK",    "",    "",    "",    ""  },
	{ "PA",  "pa" ,       "", 15838L,  "PA",    "",    "",    "",    ""  },
	{ "PV",  "pv" ,       "",  1495L,  "PV",    "",    "",    "",    ""  },
	{ "SC",  "sc" ,       "", 15836L,  "SC",    "",    "",    "",    ""  },
	{ "SD",  "sd" ,       "",  1496L,  "SD",    "",    "",    "",    ""  },
	{ "TN",  "tn" ,       "",  1497L,  "TN",    "",    "",    "",    ""  },
	{ "UT",  "ut" ,       "",  1525L,  "UT",    "",    "",    "",    ""  },
	{ "VA",  "va" ,       "",  1500L,  "VA",    "",    "",    "",    ""  },
	{ "WI",  "wi" ,       "",  1502L,  "WI",    "",    "",    "",    ""  },
	{ "WO",  "wo" ,       "",  1501L,  "WA",  "OR",    "",    "",    ""  },
	{ "WV",  "wv" ,       "",  1526L,  "WV",    "",    "",    "",    ""  },
	{ "WY",  "wy" ,       "",  1503L,  "WY",    "",    "",    "",    ""  },
//	Datums for specific UTM Zones
//  For these guys, we the state1 thru state14 elements to list the file names.
	{ "02", "UTM" ,   "UTM-2",     0L,  "hi",    "",    "",    "",    "",    "",    "",    "",    "",    "",    "",    "",    "",   ""  },
	{ "10", "UTM" ,  "UTM-10",     0L,  "wo",  "cn",  "cs",    "",    "",    "",    "",    "",    "",    "",    "",    "",    "",   ""  },
	{ "11", "UTM" ,  "UTM-11",     0L,  "wm",  "wo",  "cs",  "nv",  "ut",  "az",  "nm",    "",    "",    "",    "",    "",    "",   ""  },
	{ "12", "UTM" ,  "UTM-12",     0L,  "wm",  "em",  "wy",  "ut",  "co",  "az",  "nm",    "",    "",    "",    "",    "",    "",   ""  },
	{ "13", "UTM" ,  "UTM-13",     0L,  "em",  "wy",  "co",  "nm",  "nd",  "sd",  "nb",  "ks",  "ok",  "wt",    "",    "",    "",   ""  },
	{ "14", "UTM" ,  "UTM-14",     0L,  "nd",  "sd",  "nb",  "ks",  "ok",  "et",  "wt",    "",    "",    "",    "",    "",    "",   ""  },
	{ "15", "UTM" ,  "UTM-15",     0L,  "mn",  "wi",  "ia",  "il",  "mo",  "ar",  "la",  "ms",  "nb",    "",    "",    "",    "",   ""  },
	{ "16", "UTM" ,  "UTM-16",     0L,  "wi",  "mi",  "il",  "in",  "oh",  "ky",  "tn",  "ms",  "al",  "ga",  "fl",    "",    "",   ""  },
	{ "17", "UTM" ,  "UTM-17",     0L,  "mi",  "oh",  "ky",  "tn",  "ga",  "fl",  "wv",  "pa",  "va",  "nc",  "sc",    "",    "",   ""  },
	{ "18", "UTM" ,  "UTM-18",     0L,  "ny",  "ne",  "nj",	 "pa",  "md",  "va",  "nc",  "sc",    "",    "",    "",    "",    "",   ""  },
	{ "19", "UTM" ,  "UTM-19",     0L,  "me",  "ne",    "",	   "",    "",    "",    "",    "",    "",    "",    "",    "",    "",   ""  },
// end of table marker.
	{   "",    "" ,        "",     0L,    "",    "",    "",    "",    "",    "",    "",    "",    "",    "",    "",    "",    "",   ""  }
};

// 
struct csHarnCrsMapTable_
{
	char oldName [24];
	char newName [24];
	char datumName [24];
} csHarnCrsMapTable [] =
{
	{                 "AdamsWI-F",            "HARN/WI.AdamsWI-F",  "HARN/WI"  },
	{                "AdamsWI-IF",           "HARN/WI.AdamsWI-IF",  "HARN/WI"  },
	{                 "AdamsWI-M",            "HARN/WI.AdamsWI-M",  "HARN/WI"  },
	{                    "ALHP-E",                 "HARN/AL.AL-E",  "HARN/AL"  },
	{                   "ALHP-EF",                "HARN/AL.AL-EF",  "HARN/AL"  },
	{                    "ALHP-W",                 "HARN/AL.AL-W",  "HARN/AL"  },
	{                   "ALHP-WF",                "HARN/AL.AL-WF",  "HARN/AL"  },
	{                    "ARHP-N",                 "HARN/AR.AR-N",  "HARN/AR"  },
	{                   "ARHP-NF",                "HARN/AR.AR-NF",  "HARN/AR"  },
	{                    "ARHP-S",                 "HARN/AR.AR-S",  "HARN/AR"  },
	{                   "ARHP-SF",                "HARN/AR.AR-SF",  "HARN/AR"  },
	{               "AshlandWI-F",          "HARN/WI.AshlandWI-F",  "HARN/WI"  },
	{              "AshlandWI-IF",         "HARN/WI.AshlandWI-IF",  "HARN/WI"  },
	{               "AshlandWI-M",          "HARN/WI.AshlandWI-M",  "HARN/WI"  },
	{                    "AZHP-C",                 "HARN/AZ.AZ-C",  "HARN/AZ"  },
	{                   "AZHP-CF",                "HARN/AZ.AZ-CF",  "HARN/AZ"  },
	{                  "AZHP-CIF",               "HARN/AZ.AZ-CIF",  "HARN/AZ"  },
	{                    "AZHP-E",                 "HARN/AZ.AZ-E",  "HARN/AZ"  },
	{                   "AZHP-EF",                "HARN/AZ.AZ-EF",  "HARN/AZ"  },
	{                  "AZHP-EIF",               "HARN/AZ.AZ-EIF",  "HARN/AZ"  },
	{                    "AZHP-W",                 "HARN/AZ.AZ-W",  "HARN/AZ"  },
	{                   "AZHP-WF",                "HARN/AZ.AZ-WF",  "HARN/AZ"  },
	{                  "AZHP-WIF",               "HARN/AZ.AZ-WIF",  "HARN/AZ"  },
	{                "BarronWI-F",           "HARN/WI.BarronWI-F",  "HARN/WI"  },
	{               "BarronWI-IF",          "HARN/WI.BarronWI-IF",  "HARN/WI"  },
	{                "BarronWI-M",           "HARN/WI.BarronWI-M",  "HARN/WI"  },
	{              "BayfieldWI-F",         "HARN/WI.BayfieldWI-F",  "HARN/WI"  },
	{             "BayfieldWI-IF",        "HARN/WI.BayfieldWI-IF",  "HARN/WI"  },
	{              "BayfieldWI-M",         "HARN/WI.BayfieldWI-M",  "HARN/WI"  },
	{                 "BrownWI-F",            "HARN/WI.BrownWI-F",  "HARN/WI"  },
	{                "BrownWI-IF",           "HARN/WI.BrownWI-IF",  "HARN/WI"  },
	{                 "BrownWI-M",            "HARN/WI.BrownWI-M",  "HARN/WI"  },
	{               "BuffaloWI-F",          "HARN/WI.BuffaloWI-F",  "HARN/WI"  },
	{              "BuffaloWI-IF",         "HARN/WI.BuffaloWI-IF",  "HARN/WI"  },
	{               "BuffaloWI-M",          "HARN/WI.BuffaloWI-M",  "HARN/WI"  },
	{               "BurnettWI-F",          "HARN/WI.BurnettWI-F",  "HARN/WI"  },
	{              "BurnettWI-IF",         "HARN/WI.BurnettWI-IF",  "HARN/WI"  },
	{               "BurnettWI-M",          "HARN/WI.BurnettWI-M",  "HARN/WI"  },
	{                    "CAHP-I",                 "HARN/CA.CA-I",  "HARN/CA"  },
	{                   "CAHP-IF",                "HARN/CA.CA-IF",  "HARN/CA"  },
	{                   "CAHP-II",                "HARN/CA.CA-II",  "HARN/CA"  },
	{                  "CAHP-IIF",               "HARN/CA.CA-IIF",  "HARN/CA"  },
	{                  "CAHP-III",               "HARN/CA.CA-III",  "HARN/CA"  },
	{                   "CAHP-IV",                "HARN/CA.CA-IV",  "HARN/CA"  },
	{                  "CAHP-IVF",               "HARN/CA.CA-IVF",  "HARN/CA"  },
	{                    "CAHP-V",                 "HARN/CA.CA-V",  "HARN/CA"  },
	{                   "CAHP-VF",                "HARN/CA.CA-VF",  "HARN/CA"  },
	{                  "CAHPIIIF",               "HARN/CA.CAIIIF",  "HARN/CA"  },
	{                    "CAHPVI",                 "HARN/CA.CAVI",  "HARN/CA"  },
	{                   "CAHPVIF",                "HARN/CA.CAVIF",  "HARN/CA"  },
	{               "CalumetWI-F",          "HARN/WI.CalumetWI-F",  "HARN/WI"  },
	{              "CalumetWI-IF",         "HARN/WI.CalumetWI-IF",  "HARN/WI"  },
	{               "CalumetWI-M",          "HARN/WI.CalumetWI-M",  "HARN/WI"  },
	{              "ChippewaWI-F",         "HARN/WI.ChippewaWI-F",  "HARN/WI"  },
	{             "ChippewaWI-IF",        "HARN/WI.ChippewaWI-IF",  "HARN/WI"  },
	{              "ChippewaWI-M",         "HARN/WI.ChippewaWI-M",  "HARN/WI"  },
	{                 "ClarkWI-F",            "HARN/WI.ClarkWI-F",  "HARN/WI"  },
	{                "ClarkWI-IF",           "HARN/WI.ClarkWI-IF",  "HARN/WI"  },
	{                 "ClarkWI-M",            "HARN/WI.ClarkWI-M",  "HARN/WI"  },
	{                    "COHP-C",                 "HARN/CO.CO-C",  "HARN/CO"  },
	{                    "COHP-N",                 "HARN/CO.CO-N",  "HARN/CO"  },
	{                   "COHP-NF",                "HARN/CO.CO-NF",  "HARN/CO"  },
	{                    "COHP-S",                 "HARN/CO.CO-S",  "HARN/CO"  },
	{               "COHP-SF-MOD",            "HARN/CO.CO-SF-MOD",  "HARN/CO"  },
	{                    "COHPCF",                 "HARN/CO.COCF",  "HARN/CO"  },
	{              "ColumbiaWI-F",         "HARN/WI.ColumbiaWI-F",  "HARN/WI"  },
	{             "ColumbiaWI-IF",        "HARN/WI.ColumbiaWI-IF",  "HARN/WI"  },
	{              "ColumbiaWI-M",         "HARN/WI.ColumbiaWI-M",  "HARN/WI"  },
	{            "CrawfordWI-F/a",       "HARN/WI.CrawfordWI-F/a",  "HARN/WI"  },
	{           "CrawfordWI-IF/a",      "HARN/WI.CrawfordWI-IF/a",  "HARN/WI"  },
	{            "CrawfordWI-M/a",       "HARN/WI.CrawfordWI-M/a",  "HARN/WI"  },
	{                      "CTHP",                   "HARN/NE.CT",  "HARN/NE"  },
	{                     "CTHPF",                  "HARN/NE.CTF",  "HARN/NE"  },
	{                  "DaneWI-F",             "HARN/WI.DaneWI-F",  "HARN/WI"  },
	{                 "DaneWI-IF",            "HARN/WI.DaneWI-IF",  "HARN/WI"  },
	{                  "DaneWI-M",             "HARN/WI.DaneWI-M",  "HARN/WI"  },
	{                      "DEHP",                   "HARN/MD.DE",  "HARN/MD"  },
	{                     "DEHPF",                  "HARN/MD.DEF",  "HARN/MD"  },
	{                 "DodgeWI-F",            "HARN/WI.DodgeWI-F",  "HARN/WI"  },
	{                "DodgeWI-IF",           "HARN/WI.DodgeWI-IF",  "HARN/WI"  },
	{                 "DodgeWI-M",            "HARN/WI.DodgeWI-M",  "HARN/WI"  },
	{                  "DoorWI-F",             "HARN/WI.DoorWI-F",  "HARN/WI"  },
	{                 "DoorWI-IF",            "HARN/WI.DoorWI-IF",  "HARN/WI"  },
	{                  "DoorWI-M",             "HARN/WI.DoorWI-M",  "HARN/WI"  },
	{               "DouglasWI-F",          "HARN/WI.DouglasWI-F",  "HARN/WI"  },
	{              "DouglasWI-IF",         "HARN/WI.DouglasWI-IF",  "HARN/WI"  },
	{               "DouglasWI-M",          "HARN/WI.DouglasWI-M",  "HARN/WI"  },
	{                  "DunnWI-F",             "HARN/WI.DunnWI-F",  "HARN/WI"  },
	{                 "DunnWI-IF",            "HARN/WI.DunnWI-IF",  "HARN/WI"  },
	{                  "DunnWI-M",             "HARN/WI.DunnWI-M",  "HARN/WI"  },
	{             "EauClaireWI-F",        "HARN/WI.EauClaireWI-F",  "HARN/WI"  },
	{            "EauClaireWI-IF",       "HARN/WI.EauClaireWI-IF",  "HARN/WI"  },
	{             "EauClaireWI-M",        "HARN/WI.EauClaireWI-M",  "HARN/WI"  },
	{                    "FLHP-E",                 "HARN/FL.FL-E",  "HARN/FL"  },
	{                   "FLHP-EF",                "HARN/FL.FL-EF",  "HARN/FL"  },
	{                    "FLHP-N",                 "HARN/FL.FL-N",  "HARN/FL"  },
	{                   "FLHP-NF",                "HARN/FL.FL-NF",  "HARN/FL"  },
	{                    "FLHP-W",                 "HARN/FL.FL-W",  "HARN/FL"  },
	{                   "FLHP-WF",                "HARN/FL.FL-WF",  "HARN/FL"  },
	{              "FlorenceWI-F",         "HARN/WI.FlorenceWI-F",  "HARN/WI"  },
	{             "FlorenceWI-IF",        "HARN/WI.FlorenceWI-IF",  "HARN/WI"  },
	{              "FlorenceWI-M",         "HARN/WI.FlorenceWI-M",  "HARN/WI"  },
	{             "FondDuLacWI-F",        "HARN/WI.FondDuLacWI-F",  "HARN/WI"  },
	{            "FondDuLacWI-IF",       "HARN/WI.FondDuLacWI-IF",  "HARN/WI"  },
	{             "FondDuLacWI-M",        "HARN/WI.FondDuLacWI-M",  "HARN/WI"  },
	{                "ForestWI-F",           "HARN/WI.ForestWI-F",  "HARN/WI"  },
	{               "ForestWI-IF",          "HARN/WI.ForestWI-IF",  "HARN/WI"  },
	{                "ForestWI-M",           "HARN/WI.ForestWI-M",  "HARN/WI"  },
	{                    "GAHP-E",                 "HARN/GA.GA-E",  "HARN/GA"  },
	{                   "GAHP-EF",                "HARN/GA.GA-EF",  "HARN/GA"  },
	{                    "GAHP-W",                 "HARN/GA.GA-W",  "HARN/GA"  },
	{                   "GAHP-WF",                "HARN/GA.GA-WF",  "HARN/GA"  },
	{                 "GrantWI-F",            "HARN/WI.GrantWI-F",  "HARN/WI"  },
	{                "GrantWI-IF",           "HARN/WI.GrantWI-IF",  "HARN/WI"  },
	{                 "GrantWI-M",            "HARN/WI.GrantWI-M",  "HARN/WI"  },
	{             "GreenLakeWI-F",        "HARN/WI.GreenLakeWI-F",  "HARN/WI"  },
	{            "GreenLakeWI-IF",       "HARN/WI.GreenLakeWI-IF",  "HARN/WI"  },
	{             "GreenLakeWI-M",        "HARN/WI.GreenLakeWI-M",  "HARN/WI"  },
	{                 "GreenWI-F",            "HARN/WI.GreenWI-F",  "HARN/WI"  },
	{                "GreenWI-IF",           "HARN/WI.GreenWI-IF",  "HARN/WI"  },
	{                 "GreenWI-M",            "HARN/WI.GreenWI-M",  "HARN/WI"  },
	{             "HARN.CA/Teale",             "HARN/CA.CA/Teale",  "HARN/CA"  },
	{    "HARN.FloridaGDL/Albers",      "HARN/FL.FloridaGDL/Albr",  "HARN/FL"  },
	{             "HARN.ME2K-C/a",             "HARN/ME.ME2K-C/a",  "HARN/ME"  },
	{               "HARN.ME2K-E",               "HARN/ME.ME2K-E",  "HARN/ME"  },
	{               "HARN.ME2K-W",               "HARN/ME.ME2K-W",  "HARN/ME"  },
	{        "HARN.OregonLambert",        "HARN/WO.OregonLambert",  "HARN/WO"  },
	{     "HARN.OregonLambert.ft",      "HARN/WO.OregonLambert-F",  "HARN/WO"  },
	{         "HARN.Texas/Albers",         "HARN/TX.Texas/Albers",  "HARN/TX"  },
	{        "HARN.Texas/Lambert",        "HARN/TX.Texas/Lambert",  "HARN/TX"  },
	{               "HARN.UTM-2S",               "HARN/02.UTM-2S",  "HARN/02"  },
	{                    "HIHP-1",                 "HARN/HI.HI-1",  "HARN/HI"  },
	{                   "HIHP-1F",                "HARN/HI.HI-1F",  "HARN/HI"  },
	{                    "HIHP-2",                 "HARN/HI.HI-2",  "HARN/HI"  },
	{                   "HIHP-2F",                "HARN/HI.HI-2F",  "HARN/HI"  },
	{                    "HIHP-3",                 "HARN/HI.HI-3",  "HARN/HI"  },
	{                   "HIHP-3F",                "HARN/HI.HI-3F",  "HARN/HI"  },
	{                    "HIHP-4",                 "HARN/HI.HI-4",  "HARN/HI"  },
	{                   "HIHP-4F",                "HARN/HI.HI-4F",  "HARN/HI"  },
	{                    "HIHP-5",                 "HARN/HI.HI-5",  "HARN/HI"  },
	{                   "HIHP-5F",                "HARN/HI.HI-5F",  "HARN/HI"  },
	{                    "IAHP-N",                 "HARN/IA.IA-N",  "HARN/IA"  },
	{                   "IAHP-NF",                "HARN/IA.IA-NF",  "HARN/IA"  },
	{                    "IAHP-S",                 "HARN/IA.IA-S",  "HARN/IA"  },
	{                   "IAHP-SF",                "HARN/IA.IA-SF",  "HARN/IA"  },
	{                    "IDHP-C",                 "HARN/MT.ID-C",  "HARN/MT"  },
	{                   "IDHP-CF",                "HARN/MT.ID-CF",  "HARN/MT"  },
	{                    "IDHP-E",                 "HARN/MT.ID-E",  "HARN/MT"  },
	{                   "IDHP-EF",                "HARN/MT.ID-EF",  "HARN/MT"  },
	{                    "IDHP-W",                 "HARN/MT.ID-W",  "HARN/MT"  },
	{                   "IDHP-WF",                "HARN/MT.ID-WF",  "HARN/MT"  },
	{                    "ILHP-E",                 "HARN/IL.IL-E",  "HARN/IL"  },
	{                   "ILHP-EF",                "HARN/IL.IL-EF",  "HARN/IL"  },
	{                    "ILHP-W",                 "HARN/IL.IL-W",  "HARN/IL"  },
	{                   "ILHP-WF",                "HARN/IL.IL-WF",  "HARN/IL"  },
	{                    "INHP-E",                 "HARN/IN.IN-E",  "HARN/IN"  },
	{                   "INHP-EF",                "HARN/IN.IN-EF",  "HARN/IN"  },
	{                    "INHP-W",                 "HARN/IN.IN-W",  "HARN/IN"  },
	{                   "INHP-WF",                "HARN/IN.IN-WF",  "HARN/IN"  },
	{                  "IowaWI-F",             "HARN/WI.IowaWI-F",  "HARN/WI"  },
	{                 "IowaWI-IF",            "HARN/WI.IowaWI-IF",  "HARN/WI"  },
	{                  "IowaWI-M",             "HARN/WI.IowaWI-M",  "HARN/WI"  },
	{                  "IronWI-F",             "HARN/WI.IronWI-F",  "HARN/WI"  },
	{                 "IronWI-IF",            "HARN/WI.IronWI-IF",  "HARN/WI"  },
	{                  "IronWI-M",             "HARN/WI.IronWI-M",  "HARN/WI"  },
	{               "JacksonWI-F",          "HARN/WI.JacksonWI-F",  "HARN/WI"  },
	{              "JacksonWI-IF",         "HARN/WI.JacksonWI-IF",  "HARN/WI"  },
	{               "JacksonWI-M",          "HARN/WI.JacksonWI-M",  "HARN/WI"  },
	{             "JeffersonWI-F",        "HARN/WI.JeffersonWI-F",  "HARN/WI"  },
	{            "JeffersonWI-IF",       "HARN/WI.JeffersonWI-IF",  "HARN/WI"  },
	{             "JeffersonWI-M",        "HARN/WI.JeffersonWI-M",  "HARN/WI"  },
	{                "JuneauWI-F",           "HARN/WI.JuneauWI-F",  "HARN/WI"  },
	{               "JuneauWI-IF",          "HARN/WI.JuneauWI-IF",  "HARN/WI"  },
	{                "JuneauWI-M",           "HARN/WI.JuneauWI-M",  "HARN/WI"  },
	{               "KenoshaWI-F",          "HARN/WI.KenoshaWI-F",  "HARN/WI"  },
	{              "KenoshaWI-IF",         "HARN/WI.KenoshaWI-IF",  "HARN/WI"  },
	{               "KenoshaWI-M",          "HARN/WI.KenoshaWI-M",  "HARN/WI"  },
	{              "KewauneeWI-F",         "HARN/WI.KewauneeWI-F",  "HARN/WI"  },
	{             "KewauneeWI-IF",        "HARN/WI.KewauneeWI-IF",  "HARN/WI"  },
	{              "KewauneeWI-M",         "HARN/WI.KewauneeWI-M",  "HARN/WI"  },
	{                    "KSHP-N",                 "HARN/KS.KS-N",  "HARN/KS"  },
	{                   "KSHP-NF",                "HARN/KS.KS-NF",  "HARN/KS"  },
	{                    "KSHP-S",                 "HARN/KS.KS-S",  "HARN/KS"  },
	{                   "KSHP-SF",                "HARN/KS.KS-SF",  "HARN/KS"  },
	{                      "KYHP",                   "HARN/KY.KY",  "HARN/KY"  },
	{                    "KYHP-N",                 "HARN/KY.KY-N",  "HARN/KY"  },
	{                   "KYHP-NF",                "HARN/KY.KY-NF",  "HARN/KY"  },
	{                    "KYHP-S",                 "HARN/KY.KY-S",  "HARN/KY"  },
	{                   "KYHP-SF",                "HARN/KY.KY-SF",  "HARN/KY"  },
	{                     "KYHPF",                  "HARN/KY.KYF",  "HARN/KY"  },
	{              "LaCrosseWI-F",         "HARN/WI.LaCrosseWI-F",  "HARN/WI"  },
	{             "LaCrosseWI-IF",        "HARN/WI.LaCrosseWI-IF",  "HARN/WI"  },
	{              "LaCrosseWI-M",         "HARN/WI.LaCrosseWI-M",  "HARN/WI"  },
	{             "LafayetteWI-F",        "HARN/WI.LafayetteWI-F",  "HARN/WI"  },
	{            "LafayetteWI-IF",       "HARN/WI.LafayetteWI-IF",  "HARN/WI"  },
	{             "LafayetteWI-M",        "HARN/WI.LafayetteWI-M",  "HARN/WI"  },
	{                    "LAHP-N",                 "HARN/LA.LA-N",  "HARN/LA"  },
	{                   "LAHP-NF",                "HARN/LA.LA-NF",  "HARN/LA"  },
	{                    "LAHP-O",                 "HARN/LA.LA-O",  "HARN/LA"  },
	{                   "LAHP-OF",                "HARN/LA.LA-OF",  "HARN/LA"  },
	{                    "LAHP-S",                 "HARN/LA.LA-S",  "HARN/LA"  },
	{                   "LAHP-SF",                "HARN/LA.LA-SF",  "HARN/LA"  },
	{              "LangladeWI-F",         "HARN/WI.LangladeWI-F",  "HARN/WI"  },
	{             "LangladeWI-IF",        "HARN/WI.LangladeWI-IF",  "HARN/WI"  },
	{              "LangladeWI-M",         "HARN/WI.LangladeWI-M",  "HARN/WI"  },
	{               "LincolnWI-F",          "HARN/WI.LincolnWI-F",  "HARN/WI"  },
	{              "LincolnWI-IF",         "HARN/WI.LincolnWI-IF",  "HARN/WI"  },
	{               "LincolnWI-M",          "HARN/WI.LincolnWI-M",  "HARN/WI"  },
	{                      "MAHP",                   "HARN/NE.MA",  "HARN/NE"  },
	{                   "MAHP-IS",                "HARN/NE.MA-IS",  "HARN/NE"  },
	{                  "MAHP-ISF",               "HARN/NE.MA-ISF",  "HARN/NE"  },
	{                     "MAHPF",                  "HARN/NE.MAF",  "HARN/NE"  },
	{             "ManitowocWI-F",        "HARN/WI.ManitowocWI-F",  "HARN/WI"  },
	{            "ManitowocWI-IF",       "HARN/WI.ManitowocWI-IF",  "HARN/WI"  },
	{             "ManitowocWI-M",        "HARN/WI.ManitowocWI-M",  "HARN/WI"  },
	{              "MarathonWI-F",         "HARN/WI.MarathonWI-F",  "HARN/WI"  },
	{             "MarathonWI-IF",        "HARN/WI.MarathonWI-IF",  "HARN/WI"  },
	{              "MarathonWI-M",         "HARN/WI.MarathonWI-M",  "HARN/WI"  },
	{             "MarinetteWI-F",        "HARN/WI.MarinetteWI-F",  "HARN/WI"  },
	{            "MarinetteWI-IF",       "HARN/WI.MarinetteWI-IF",  "HARN/WI"  },
	{             "MarinetteWI-M",        "HARN/WI.MarinetteWI-M",  "HARN/WI"  },
	{             "MarquetteWI-F",        "HARN/WI.MarquetteWI-F",  "HARN/WI"  },
	{            "MarquetteWI-IF",       "HARN/WI.MarquetteWI-IF",  "HARN/WI"  },
	{             "MarquetteWI-M",        "HARN/WI.MarquetteWI-M",  "HARN/WI"  },
	{                      "MDHP",                   "HARN/MD.MD",  "HARN/MD"  },
	{                     "MDHPF",                  "HARN/MD.MDF",  "HARN/MD"  },
	{                    "MEHP-E",                 "HARN/ME.ME-E",  "HARN/ME"  },
	{                   "MEHP-EF",                "HARN/ME.ME-EF",  "HARN/ME"  },
	{                    "MEHP-W",                 "HARN/ME.ME-W",  "HARN/ME"  },
	{                   "MEHP-WF",                "HARN/ME.ME-WF",  "HARN/ME"  },
	{             "MenomineeWI-F",        "HARN/WI.MenomineeWI-F",  "HARN/WI"  },
	{            "MenomineeWI-IF",       "HARN/WI.MenomineeWI-IF",  "HARN/WI"  },
	{             "MenomineeWI-M",        "HARN/WI.MenomineeWI-M",  "HARN/WI"  },
	{          "MichiganGeoRefHP",       "HARN/MI.MichiganGeoRef",  "HARN/MI"  },
	{                    "MIHP-C",                 "HARN/MI.MI-C",  "HARN/MI"  },
	{                   "MIHP-CF",                "HARN/MI.MI-CF",  "HARN/MI"  },
	{                  "MIHP-CIF",               "HARN/MI.MI-CIF",  "HARN/MI"  },
	{                    "MIHP-N",                 "HARN/MI.MI-N",  "HARN/MI"  },
	{                   "MIHP-NF",                "HARN/MI.MI-NF",  "HARN/MI"  },
	{                  "MIHP-NIF",               "HARN/MI.MI-NIF",  "HARN/MI"  },
	{                    "MIHP-S",                 "HARN/MI.MI-S",  "HARN/MI"  },
	{                   "MIHP-SF",                "HARN/MI.MI-SF",  "HARN/MI"  },
	{                  "MIHP-SIF",               "HARN/MI.MI-SIF",  "HARN/MI"  },
	{             "MilwaukeeWI-F",        "HARN/WI.MilwaukeeWI-F",  "HARN/WI"  },
	{            "MilwaukeeWI-IF",       "HARN/WI.MilwaukeeWI-IF",  "HARN/WI"  },
	{             "MilwaukeeWI-M",        "HARN/WI.MilwaukeeWI-M",  "HARN/WI"  },
	{                    "MNHP-C",                 "HARN/MN.MN-C",  "HARN/MN"  },
	{                   "MNHP-CF",                "HARN/MN.MN-CF",  "HARN/MN"  },
	{                    "MNHP-N",                 "HARN/MN.MN-N",  "HARN/MN"  },
	{                   "MNHP-NF",                "HARN/MN.MN-NF",  "HARN/MN"  },
	{                    "MNHP-S",                 "HARN/MN.MN-S",  "HARN/MN"  },
	{                   "MNHP-SF",                "HARN/MN.MN-SF",  "HARN/MN"  },
	{                    "MOHP-C",                 "HARN/MO.MO-C",  "HARN/MO"  },
	{                   "MOHP-CF",                "HARN/MO.MO-CF",  "HARN/MO"  },
	{                    "MOHP-E",                 "HARN/MO.MO-E",  "HARN/MO"  },
	{                   "MOHP-EF",                "HARN/MO.MO-EF",  "HARN/MO"  },
	{                    "MOHP-W",                 "HARN/MO.MO-W",  "HARN/MO"  },
	{                   "MOHP-WF",                "HARN/MO.MO-WF",  "HARN/MO"  },
	{                "MonroeWI-F",           "HARN/WI.MonroeWI-F",  "HARN/WI"  },
	{               "MonroeWI-IF",          "HARN/WI.MonroeWI-IF",  "HARN/WI"  },
	{                "MonroeWI-M",           "HARN/WI.MonroeWI-M",  "HARN/WI"  },
	{                    "MSHP-E",                 "HARN/MS.MS-E",  "HARN/MS"  },
	{                   "MSHP-EF",                "HARN/MS.MS-EF",  "HARN/MS"  },
	{                    "MSHP-W",                 "HARN/MS.MS-W",  "HARN/MS"  },
	{                 "MSHP-WF/a",              "HARN/MS.MS-WF/a",  "HARN/MS"  },
	{                      "MTHP",                   "HARN/MT.MT",  "HARN/MT"  },
	{                     "MTHPF",                  "HARN/MT.MTF",  "HARN/MT"  },
	{                    "MTHPIF",                 "HARN/MT.MTIF",  "HARN/MT"  },
	{                      "NCHP",                   "HARN/NC.NC",  "HARN/NC"  },
	{                     "NCHPF",                  "HARN/NC.NCF",  "HARN/NC"  },
	{                    "NDHP-N",                 "HARN/ND.ND-N",  "HARN/ND"  },
	{                   "NDHP-NF",                "HARN/ND.ND-NF",  "HARN/ND"  },
	{                  "NDHP-NIF",               "HARN/ND.ND-NIF",  "HARN/ND"  },
	{                    "NDHP-S",                 "HARN/ND.ND-S",  "HARN/ND"  },
	{                   "NDHP-SF",                "HARN/ND.ND-SF",  "HARN/ND"  },
	{                  "NDHP-SIF",               "HARN/ND.ND-SIF",  "HARN/ND"  },
	{                     "NE-HP",                   "HARN/NB.NB",  "HARN/NB"  },
	{                    "NE-HPF",                  "HARN/NB.NBF",  "HARN/NB"  },
	{                      "NHHP",                   "HARN/NE.NH",  "HARN/NE"  },
	{                     "NHHPF",                  "HARN/NE.NHF",  "HARN/NE"  },
	{                      "NJHP",                   "HARN/NJ.NJ",  "HARN/NJ"  },
	{                     "NJHPF",                  "HARN/NJ.NJF",  "HARN/NJ"  },
	{                    "NMHP-C",                 "HARN/NM.NM-C",  "HARN/NM"  },
	{                   "NMHP-CF",                "HARN/NM.NM-CF",  "HARN/NM"  },
	{                    "NMHP-E",                 "HARN/NM.NM-E",  "HARN/NM"  },
	{                   "NMHP-EF",                "HARN/NM.NM-EF",  "HARN/NM"  },
	{                    "NMHP-W",                 "HARN/NM.NM-W",  "HARN/NM"  },
	{                   "NMHP-WF",                "HARN/NM.NM-WF",  "HARN/NM"  },
	{                    "NVHP-C",                 "HARN/NV.NV-C",  "HARN/NV"  },
	{                   "NVHP-CF",                "HARN/NV.NV-CF",  "HARN/NV"  },
	{                    "NVHP-E",                 "HARN/NV.NV-E",  "HARN/NV"  },
	{                   "NVHP-EF",                "HARN/NV.NV-EF",  "HARN/NV"  },
	{                    "NVHP-W",                 "HARN/NV.NV-W",  "HARN/NV"  },
	{                   "NVHP-WF",                "HARN/NV.NV-WF",  "HARN/NV"  },
	{                    "NYHP-C",                 "HARN/NY.NY-C",  "HARN/NY"  },
	{                   "NYHP-CF",                "HARN/NY.NY-CF",  "HARN/NY"  },
	{                    "NYHP-E",                 "HARN/NY.NY-E",  "HARN/NY"  },
	{                   "NYHP-EF",                "HARN/NY.NY-EF",  "HARN/NY"  },
	{                   "NYHP-LI",                "HARN/NY.NY-LI",  "HARN/NY"  },
	{                  "NYHP-LIF",               "HARN/NY.NY-LIF",  "HARN/NY"  },
	{                    "NYHP-W",                 "HARN/NY.NY-W",  "HARN/NY"  },
	{                   "NYHP-WF",                "HARN/NY.NY-WF",  "HARN/NY"  },
	{                "OcontoWI-F",           "HARN/WI.OcontoWI-F",  "HARN/WI"  },
	{               "OcontoWI-IF",          "HARN/WI.OcontoWI-IF",  "HARN/WI"  },
	{                "OcontoWI-M",           "HARN/WI.OcontoWI-M",  "HARN/WI"  },
	{                    "OHHP-N",                 "HARN/OH.OH-N",  "HARN/OH"  },
	{                   "OHHP-NF",                "HARN/OH.OH-NF",  "HARN/OH"  },
	{                    "OHHP-S",                 "HARN/OH.OH-S",  "HARN/OH"  },
	{                   "OHHP-SF",                "HARN/OH.OH-SF",  "HARN/OH"  },
	{                    "OKHP-N",                 "HARN/OK.OK-N",  "HARN/OK"  },
	{                   "OKHP-NF",                "HARN/OK.OK-NF",  "HARN/OK"  },
	{                    "OKHP-S",                 "HARN/OK.OK-S",  "HARN/OK"  },
	{                   "OKHP-SF",                "HARN/OK.OK-SF",  "HARN/OK"  },
	{                "OneidaWI-F",           "HARN/WI.OneidaWI-F",  "HARN/WI"  },
	{               "OneidaWI-IF",          "HARN/WI.OneidaWI-IF",  "HARN/WI"  },
	{                "OneidaWI-M",           "HARN/WI.OneidaWI-M",  "HARN/WI"  },
	{                    "ORHP-N",                 "HARN/WO.OR-N",  "HARN/WO"  },
	{                   "ORHP-NF",                "HARN/WO.OR-NF",  "HARN/WO"  },
	{                  "ORHP-NIF",               "HARN/WO.OR-NIF",  "HARN/WO"  },
	{                    "ORHP-S",                 "HARN/WO.OR-S",  "HARN/WO"  },
	{                   "ORHP-SF",                "HARN/WO.OR-SF",  "HARN/WO"  },
	{                  "ORHP-SIF",               "HARN/WO.OR-SIF",  "HARN/WO"  },
	{             "OutagamieWI-F",        "HARN/WI.OutagamieWI-F",  "HARN/WI"  },
	{            "OutagamieWI-IF",       "HARN/WI.OutagamieWI-IF",  "HARN/WI"  },
	{             "OutagamieWI-M",        "HARN/WI.OutagamieWI-M",  "HARN/WI"  },
	{               "OzaukeeWI-F",          "HARN/WI.OzaukeeWI-F",  "HARN/WI"  },
	{              "OzaukeeWI-IF",         "HARN/WI.OzaukeeWI-IF",  "HARN/WI"  },
	{               "OzaukeeWI-M",          "HARN/WI.OzaukeeWI-M",  "HARN/WI"  },
	{                    "PAHP-N",                 "HARN/PA.PA-N",  "HARN/PA"  },
	{                   "PAHP-NF",                "HARN/PA.PA-NF",  "HARN/PA"  },
	{                    "PAHP-S",                 "HARN/PA.PA-S",  "HARN/PA"  },
	{                   "PAHP-SF",                "HARN/PA.PA-SF",  "HARN/PA"  },
	{                 "PepinWI-F",            "HARN/WI.PepinWI-F",  "HARN/WI"  },
	{                "PepinWI-IF",           "HARN/WI.PepinWI-IF",  "HARN/WI"  },
	{                 "PepinWI-M",            "HARN/WI.PepinWI-M",  "HARN/WI"  },
	{                "PierceWI-F",           "HARN/WI.PierceWI-F",  "HARN/WI"  },
	{               "PierceWI-IF",          "HARN/WI.PierceWI-IF",  "HARN/WI"  },
	{                "PierceWI-M",           "HARN/WI.PierceWI-M",  "HARN/WI"  },
	{                  "PolkWI-F",             "HARN/WI.PolkWI-F",  "HARN/WI"  },
	{                 "PolkWI-IF",            "HARN/WI.PolkWI-IF",  "HARN/WI"  },
	{                  "PolkWI-M",             "HARN/WI.PolkWI-M",  "HARN/WI"  },
	{               "PortageWI-F",          "HARN/WI.PortageWI-F",  "HARN/WI"  },
	{              "PortageWI-IF",         "HARN/WI.PortageWI-IF",  "HARN/WI"  },
	{               "PortageWI-M",          "HARN/WI.PortageWI-M",  "HARN/WI"  },
	{                      "PRHP",                 "HARN/PV.PRHP",  "HARN/PV"  },
	{                     "PRHPF",                "HARN/PV.PRHPF",  "HARN/PV"  },
	{                 "PriceWI-F",            "HARN/WI.PriceWI-F",  "HARN/WI"  },
	{                "PriceWI-IF",           "HARN/WI.PriceWI-IF",  "HARN/WI"  },
	{                 "PriceWI-M",            "HARN/WI.PriceWI-M",  "HARN/WI"  },
	{                "RacineWI-F",           "HARN/WI.RacineWI-F",  "HARN/WI"  },
	{               "RacineWI-IF",          "HARN/WI.RacineWI-IF",  "HARN/WI"  },
	{                "RacineWI-M",           "HARN/WI.RacineWI-M",  "HARN/WI"  },
	{              "RichlandWI-F",         "HARN/WI.RichlandWI-F",  "HARN/WI"  },
	{             "RichlandWI-IF",        "HARN/WI.RichlandWI-IF",  "HARN/WI"  },
	{              "RichlandWI-M",         "HARN/WI.RichlandWI-M",  "HARN/WI"  },
	{                      "RIHP",                   "HARN/NE.RI",  "HARN/NE"  },
	{                     "RIHPF",                  "HARN/NE.RIF",  "HARN/NE"  },
	{                  "RockWI-F",             "HARN/WI.RockWI-F",  "HARN/WI"  },
	{                 "RockWI-IF",            "HARN/WI.RockWI-IF",  "HARN/WI"  },
	{                  "RockWI-M",             "HARN/WI.RockWI-M",  "HARN/WI"  },
	{                  "RuskWI-F",             "HARN/WI.RuskWI-F",  "HARN/WI"  },
	{                 "RuskWI-IF",            "HARN/WI.RuskWI-IF",  "HARN/WI"  },
	{                  "RuskWI-M",             "HARN/WI.RuskWI-M",  "HARN/WI"  },
	{                  "SaukWI-F",             "HARN/WI.SaukWI-F",  "HARN/WI"  },
	{                 "SaukWI-IF",            "HARN/WI.SaukWI-IF",  "HARN/WI"  },
	{                  "SaukWI-M",             "HARN/WI.SaukWI-M",  "HARN/WI"  },
	{                "SawyerWI-F",           "HARN/WI.SawyerWI-F",  "HARN/WI"  },
	{               "SawyerWI-IF",          "HARN/WI.SawyerWI-IF",  "HARN/WI"  },
	{                "SawyerWI-M",           "HARN/WI.SawyerWI-M",  "HARN/WI"  },
	{                      "SCHP",                   "HARN/SC.SC",  "HARN/SC"  },
	{                     "SCHPF",                  "HARN/SC.SCF",  "HARN/SC"  },
	{                    "SCHPIF",                 "HARN/SC.SCIF",  "HARN/SC"  },
	{                    "SDHP-N",                 "HARN/SD.SD-N",  "HARN/SD"  },
	{                   "SDHP-NF",                "HARN/SD.SD-NF",  "HARN/SD"  },
	{                    "SDHP-S",                 "HARN/SD.SD-S",  "HARN/SD"  },
	{                   "SDHP-SF",                "HARN/SD.SD-SF",  "HARN/SD"  },
	{               "ShawanoWI-F",          "HARN/WI.ShawanoWI-F",  "HARN/WI"  },
	{              "ShawanoWI-IF",         "HARN/WI.ShawanoWI-IF",  "HARN/WI"  },
	{               "ShawanoWI-M",          "HARN/WI.ShawanoWI-M",  "HARN/WI"  },
	{             "SheboyganWI-F",        "HARN/WI.SheboyganWI-F",  "HARN/WI"  },
	{            "SheboyganWI-IF",       "HARN/WI.SheboyganWI-IF",  "HARN/WI"  },
	{             "SheboyganWI-M",        "HARN/WI.SheboyganWI-M",  "HARN/WI"  },
	{               "StCroixWI-F",          "HARN/WI.StCroixWI-F",  "HARN/WI"  },
	{              "StCroixWI-IF",         "HARN/WI.StCroixWI-IF",  "HARN/WI"  },
	{               "StCroixWI-M",          "HARN/WI.StCroixWI-M",  "HARN/WI"  },
	{                "TaylorWI-F",           "HARN/WI.TaylorWI-F",  "HARN/WI"  },
	{               "TaylorWI-IF",          "HARN/WI.TaylorWI-IF",  "HARN/WI"  },
	{                "TaylorWI-M",           "HARN/WI.TaylorWI-M",  "HARN/WI"  },
	{                      "TNHP",                   "HARN/TN.TN",  "HARN/TN"  },
	{                     "TNHPF",                  "HARN/TN.TNF",  "HARN/TN"  },
	{           "TrempealeauWI-F",      "HARN/WI.TrempealeauWI-F",  "HARN/WI"  },
	{          "TrempealeauWI-IF",      "HARN/WI.TrempealeaWI-IF",  "HARN/WI"  },
	{           "TrempealeauWI-M",      "HARN/WI.TrempealeauWI-M",  "HARN/WI"  },
	{                      "TREX",                 "HARN/CO.TREX",  "HARN/CO"  },
	{                    "TXHP-C",                 "HARN/TX.TX-C",  "HARN/TX"  },
	{                   "TXHP-CF",                "HARN/TX.TX-CF",  "HARN/TX"  },
	{                    "TXHP-N",                 "HARN/TX.TX-N",  "HARN/TX"  },
	{                   "TXHP-NC",                "HARN/TX.TX-NC",  "HARN/TX"  },
	{                  "TXHP-NCF",               "HARN/TX.TX-NCF",  "HARN/TX"  },
	{                   "TXHP-NF",                "HARN/TX.TX-NF",  "HARN/TX"  },
	{                    "TXHP-S",                 "HARN/TX.TX-S",  "HARN/TX"  },
	{                   "TXHP-SC",                "HARN/TX.TX-SC",  "HARN/TX"  },
	{                  "TXHP-SCF",               "HARN/TX.TX-SCF",  "HARN/TX"  },
	{                   "TXHP-SF",                "HARN/TX.TX-SF",  "HARN/TX"  },
	{                    "UTHP-C",                 "HARN/UT.UT-C",  "HARN/UT"  },
	{                   "UTHP-CF",                "HARN/UT.UT-CF",  "HARN/UT"  },
	{                  "UTHP-CIF",               "HARN/UT.UT-CIF",  "HARN/UT"  },
	{                    "UTHP-N",                 "HARN/UT.UT-N",  "HARN/UT"  },
	{                   "UTHP-NF",                "HARN/UT.UT-NF",  "HARN/UT"  },
	{                  "UTHP-NIF",               "HARN/UT.UT-NIF",  "HARN/UT"  },
	{                    "UTHP-S",                 "HARN/UT.UT-S",  "HARN/UT"  },
	{                   "UTHP-SF",                "HARN/UT.UT-SF",  "HARN/UT"  },
	{                  "UTHP-SIF",               "HARN/UT.UT-SIF",  "HARN/UT"  },
	{                  "UTMHP-10",               "HARN/10.UTM-10",  "HARN/10"  },
	{                 "UTMHP-10F",              "HARN/10.UTM-10F",  "HARN/10"  },
	{                "UTMHP-10IF",             "HARN/10.UTM-10IF",  "HARN/10"  },
	{                  "UTMHP-11",               "HARN/11.UTM-11",  "HARN/11"  },
	{                 "UTMHP-11F",              "HARN/11.UTM-11F",  "HARN/11"  },
	{                "UTMHP-11IF",             "HARN/11.UTM-11IF",  "HARN/11"  },
	{                  "UTMHP-12",               "HARN/12.UTM-12",  "HARN/12"  },
	{                 "UTMHP-12F",              "HARN/12.UTM-12F",  "HARN/12"  },
	{                "UTMHP-12IF",             "HARN/12.UTM-12IF",  "HARN/12"  },
	{                  "UTMHP-13",               "HARN/13.UTM-13",  "HARN/13"  },
	{                 "UTMHP-13F",              "HARN/13.UTM-13F",  "HARN/13"  },
	{                "UTMHP-13IF",             "HARN/13.UTM-13IF",  "HARN/13"  },
	{                  "UTMHP-14",               "HARN/14.UTM-14",  "HARN/14"  },
	{                 "UTMHP-14F",              "HARN/14.UTM-14F",  "HARN/14"  },
	{                "UTMHP-14IF",             "HARN/14.UTM-14IF",  "HARN/14"  },
	{                  "UTMHP-15",               "HARN/15.UTM-15",  "HARN/15"  },
	{                 "UTMHP-15F",              "HARN/15.UTM-15F",  "HARN/15"  },
	{                "UTMHP-15IF",             "HARN/15.UTM-15IF",  "HARN/15"  },
	{                  "UTMHP-16",               "HARN/16.UTM-16",  "HARN/16"  },
	{                 "UTMHP-16F",              "HARN/16.UTM-16F",  "HARN/16"  },
	{                "UTMHP-16IF",             "HARN/16.UTM-16IF",  "HARN/16"  },
	{                  "UTMHP-17",               "HARN/17.UTM-17",  "HARN/17"  },
	{                 "UTMHP-17F",              "HARN/17.UTM-17F",  "HARN/17"  },
	{                "UTMHP-17IF",             "HARN/17.UTM-17IF",  "HARN/17"  },
	{                  "UTMHP-18",               "HARN/18.UTM-18",  "HARN/18"  },
	{                 "UTMHP-18F",              "HARN/18.UTM-18F",  "HARN/18"  },
	{                "UTMHP-18IF",             "HARN/18.UTM-18IF",  "HARN/18"  },
	{                    "VAHP-N",                 "HARN/VA.VA-N",  "HARN/VA"  },
	{                   "VAHP-NF",                "HARN/VA.VA-NF",  "HARN/VA"  },
	{                    "VAHP-S",                 "HARN/VA.VA-S",  "HARN/VA"  },
	{                   "VAHP-SF",                "HARN/VA.VA-SF",  "HARN/VA"  },
	{                "VernonWI-F",           "HARN/WI.VernonWI-F",  "HARN/WI"  },
	{               "VernonWI-IF",          "HARN/WI.VernonWI-IF",  "HARN/WI"  },
	{                "VernonWI-M",           "HARN/WI.VernonWI-M",  "HARN/WI"  },
	{                 "VilasWI-F",            "HARN/WI.VilasWI-F",  "HARN/WI"  },
	{                "VilasWI-IF",           "HARN/WI.VilasWI-IF",  "HARN/WI"  },
	{                 "VilasWI-M",            "HARN/WI.VilasWI-M",  "HARN/WI"  },
	{                      "VTHP",                   "HARN/NE.VT",  "HARN/NE"  },
	{                     "VTHPF",                  "HARN/NE.VTF",  "HARN/NE"  },
	{                    "WAHP-N",                 "HARN/WO.WA-N",  "HARN/WO"  },
	{                   "WAHP-NF",                "HARN/WO.WA-NF",  "HARN/WO"  },
	{                    "WAHP-S",                 "HARN/WO.WA-S",  "HARN/WO"  },
	{                   "WAHP-SF",                "HARN/WO.WA-SF",  "HARN/WO"  },
	{              "WalworthWI-F",         "HARN/WI.WalworthWI-F",  "HARN/WI"  },
	{             "WalworthWI-IF",        "HARN/WI.WalworthWI-IF",  "HARN/WI"  },
	{              "WalworthWI-M",         "HARN/WI.WalworthWI-M",  "HARN/WI"  },
	{              "WashburnWI-F",         "HARN/WI.WashburnWI-F",  "HARN/WI"  },
	{             "WashburnWI-IF",        "HARN/WI.WashburnWI-IF",  "HARN/WI"  },
	{              "WashburnWI-M",         "HARN/WI.WashburnWI-M",  "HARN/WI"  },
	{            "WashingtonWI-F",       "HARN/WI.WashingtonWI-F",  "HARN/WI"  },
	{           "WashingtonWI-IF",      "HARN/WI.WashingtonWI-IF",  "HARN/WI"  },
	{            "WashingtonWI-M",       "HARN/WI.WashingtonWI-M",  "HARN/WI"  },
	{              "WaukeshaWI-F",         "HARN/WI.WaukeshaWI-F",  "HARN/WI"  },
	{             "WaukeshaWI-IF",        "HARN/WI.WaukeshaWI-IF",  "HARN/WI"  },
	{              "WaukeshaWI-M",         "HARN/WI.WaukeshaWI-M",  "HARN/WI"  },
	{               "WaupacaWI-F",          "HARN/WI.WaupacaWI-F",  "HARN/WI"  },
	{              "WaupacaWI-IF",         "HARN/WI.WaupacaWI-IF",  "HARN/WI"  },
	{               "WaupacaWI-M",          "HARN/WI.WaupacaWI-M",  "HARN/WI"  },
	{              "WausharaWI-F",         "HARN/WI.WausharaWI-F",  "HARN/WI"  },
	{             "WausharaWI-IF",        "HARN/WI.WausharaWI-IF",  "HARN/WI"  },
	{              "WausharaWI-M",         "HARN/WI.WausharaWI-M",  "HARN/WI"  },
	{                    "WIHP-C",                 "HARN/WI.WI-C",  "HARN/WI"  },
	{                   "WIHP-CF",                "HARN/WI.WI-CF",  "HARN/WI"  },
	{                    "WIHP-N",                 "HARN/WI.WI-N",  "HARN/WI"  },
	{                   "WIHP-NF",                "HARN/WI.WI-NF",  "HARN/WI"  },
	{                    "WIHP-S",                 "HARN/WI.WI-S",  "HARN/WI"  },
	{                   "WIHP-SF",                "HARN/WI.WI-SF",  "HARN/WI"  },
	{             "WinnebagoWI-F",        "HARN/WI.WinnebagoWI-F",  "HARN/WI"  },
	{            "WinnebagoWI-IF",       "HARN/WI.WinnebagoWI-IF",  "HARN/WI"  },
	{             "WinnebagoWI-M",        "HARN/WI.WinnebagoWI-M",  "HARN/WI"  },
	{            "WisconsinTM-HP",          "HARN/WI.WisconsinTM",  "HARN/WI"  },
	{                  "WoodWI-F",             "HARN/WI.WoodWI-F",  "HARN/WI"  },
	{                 "WoodWI-IF",            "HARN/WI.WoodWI-IF",  "HARN/WI"  },
	{                  "WoodWI-M",             "HARN/WI.WoodWI-M",  "HARN/WI"  },
	{                    "WVHP-N",                 "HARN/WV.WV-N",  "HARN/WV"  },
	{                   "WVHP-NF",                "HARN/WV.WV-NF",  "HARN/WV"  },
	{                    "WVHP-S",                 "HARN/WV.WV-S",  "HARN/WV"  },
	{                   "WVHP-SF",                "HARN/WV.WV-SF",  "HARN/WV"  },
	{                    "WYHP-E",                 "HARN/WY.WY-E",  "HARN/WY"  },
	{                   "WYHP-EC",                "HARN/WY.WY-EC",  "HARN/WY"  },
	{                  "WYHP-ECF",               "HARN/WY.WY-ECF",  "HARN/WY"  },
	{                   "WYHP-EF",                "HARN/WY.WY-EF",  "HARN/WY"  },
	{                    "WYHP-W",                 "HARN/WY.WY-W",  "HARN/WY"  },
	{                   "WYHP-WC",                "HARN/WY.WY-WC",  "HARN/WY"  },
	{                  "WYHP-WCF",               "HARN/WY.WY-WCF",  "HARN/WY"  },
	{                   "WYHP-WF",                "HARN/WY.WY-WF",  "HARN/WY"  },
	{                          "",                             "",  ""         }
};

static long32_t testPhiFormat;
static long32_t testLambdaFormat;
static long32_t deltaPhiFormat;
static long32_t deltaLambdaFormat;
static long32_t deltaHeightFormat;

bool csOrgTransformations (const wchar_t* csDictDir,const wchar_t* csDictTrgDir)
{
	bool ok (false);

	char csDtmDictFilePath [MAXPATH];
	char csTrgGxFilePath [MAXPATH];
	char csSrcMregFilePath [MAXPATH];
	char csSrcCopyRightFilePath [MAXPATH];
	wchar_t wrkBufr [256];

	// Initialize CS-MAP, need this for the NameMapper.
	wcstombs (csDtmDictFilePath,csDictDir,sizeof (csDtmDictFilePath));
	int st = CS_altdr (csDtmDictFilePath);
	if (st != 0)
	{
		return ok;
	}

	// Open up the Datum Dictionary file.
	TcsDatumsFile datumDict;

	// Open up the mreg source file.  We'll need that for some things
	// that in the source, but not the resulting mrt files.
	wcstombs (csSrcMregFilePath,csDictDir,sizeof (csSrcMregFilePath));
	strcat (csSrcMregFilePath,"\\mreg.asc");
	TcsDefFile mregAsc (dictTypMreg,csSrcMregFilePath);

	// Open up the gdc files which we will probably need to access.
	gdcAgd66ToGda94 = new TcsGdcFile ("Agd66ToGda94.gdc");
	gdcAgd84ToGda94 = new TcsGdcFile ("Agd84ToGda94.gdc");
	gdcAts77ToCsrs = new TcsGdcFile ("Ats77ToCsrs.gdc");
	gdcCh1903ToPlus = new TcsGdcFile ("Ch1903ToPlus.gdc");
	gdcDhdnToEtrf89 = new TcsGdcFile ("DhdnToEtrf89.gdc");
	gdcEd50ToEtrf89 = new TcsGdcFile ("Ed50ToEtrf89.gdc");
	gdcNad27ToAts77 = new TcsGdcFile ("Nad27ToAts77.gdc");
	gdcNad27ToCsrs = new TcsGdcFile ("Nad27ToCsrs.gdc");
	gdcNad27ToNad83 = new TcsGdcFile ("Nad27ToNad83.gdc");
	gdcNad83ToCsrs = new TcsGdcFile ("Nad83ToCsrs.gdc");
	gdcNad83ToHarn = new TcsGdcFile ("Nad83ToHarn.gdc");
	gdcNzgd49ToNzgd2K = new TcsGdcFile ("Nzgd49ToNzgd2K.gdc");
	gdcRgf93ToNtf = new TcsGdcFile ("Rgf93ToNtf.gdc");
	gdcTokyoToJgd2k = new TcsGdcFile ("TokyoToJgd2k.gdc");

	// Create the geodetic transformation output stream we
	// will use.
	wcstombs (csTrgGxFilePath,csDictTrgDir,sizeof (csTrgGxFilePath));
	strcat (csTrgGxFilePath,"\\GeodeticTransformation.asc");
	std::wofstream gtStrm (csTrgGxFilePath,std::ios_base::out | std::ios_base::trunc);

	// We will also need to write the initial Geodetic Path
	// dictionary.
	wcstombs (csTrgGxFilePath,csDictTrgDir,sizeof (csTrgGxFilePath));
	strcat (csTrgGxFilePath,"\\GeodeticPath.asc");
	std::wofstream gpStrm (csTrgGxFilePath,std::ios_base::out | std::ios_base::trunc);

	// Copy the copyright notice to both output files.
	wcstombs (csSrcCopyRightFilePath,csDictDir,sizeof (csTrgGxFilePath));
	strcat (csSrcCopyRightFilePath,"\\DictionaryCopyright.txt");
	std::wifstream crStrm (csSrcCopyRightFilePath,std::ios_base::in);
	if (crStrm.is_open ())
	{
		while (crStrm.good ())
		{
			crStrm.getline (wrkBufr,256);
			wrkBufr [255] = L'\0';
			gtStrm << wrkBufr << std::endl;
			gpStrm << wrkBufr << std::endl;
		}
		crStrm.close ();
	}

	if (gtStrm.is_open () && gpStrm.is_open ())
	{
		size_t index;
		size_t dtmCount;

		dtmCount = datumDict.GetRecordCount ();
		// Two passes.  First pass, we skip all the LEGACY and 3PARAMETER guys.
		// We add only them on the second pass.
		ok = true;
		for (index = 0;ok && index < dtmCount;index += 1)
		{
			const cs_Dtdef_* dtDefPtr;
			dtDefPtr = datumDict.FetchDatum (index);
			if (!CS_stricmp (dtDefPtr->group,"LEGACY") ||
			     dtDefPtr->to84_via == cs_DTCTYP_3PARM)
			{
				continue;
			}
			ok = csWriteTransformationAsc (gtStrm,gpStrm,dtDefPtr,mregAsc,"WGS84");
			
			if (ok && !CS_stricmp (dtDefPtr->key_nm,"CHTRF95"))
			{
				ok = csWriteTransformationAsc (gtStrm,gpStrm,dtDefPtr,mregAsc,"ETRF89");
			}

			if (ok && !CS_stricmp (dtDefPtr->key_nm,"CH1903Plus_1"))
			{
				ok = csWriteTransformationAsc (gtStrm,gpStrm,dtDefPtr,mregAsc,"CHTRF95");
			}
		}

		// Write the equivalent of "hard coded" stuff.
		if (ok)
		{
			ok = csWriteHardCodedStuff (gtStrm,gpStrm,csDictDir);
		}

		if (ok)
		{
			ok = csNewHpgnDatums (gtStrm,gpStrm,csDictDir,csDictTrgDir);
		}
		
		if (ok)
		{
			for (index = 0;ok && index < dtmCount;index += 1)
			{
				const cs_Dtdef_* dtDefPtr;
				dtDefPtr = datumDict.FetchDatum (index);
				if (!CS_stricmp (dtDefPtr->group,"LEGACY") ||
					 dtDefPtr->to84_via == cs_DTCTYP_3PARM)
				{
					ok = csWriteTransformationAsc (gtStrm,gpStrm,dtDefPtr,mregAsc,"WGS84");
				}
			}
		}
		gtStrm.close ();
		gpStrm.close ();
	}
	return ok;
}
// TODO: This function should be refactored into some more useful components.
// Specifically, single function which given a source and target datum pair,
// produces a cs_GeodewticTransform_ object.  The current state of affairs is
// a result of evolution and scheduling pressures.
bool csWriteTransformationAsc (std::wofstream& gtStrm,std::wofstream& gpStrm,
													  const cs_Dtdef_ *dtDefPtr,
													  const TcsDefFile& mregAsc,
													  const char* dtmPivot)
{
	bool ok (true);

	size_t count;
	size_t index;

	unsigned long epsgVariant;

	char* cp;
	const char* kCp;
	const char* entryPath;
	const TcsGdcEntry* gdcEntryPtr;
	csGeodeticXfromParmsFile_* fileParmPtr;

	double accuracy;
	double usefulRngLngW;
	double usefulRngLngE;
	double usefulRngLatS;
	double usefulRngLatN;

	TcsEpsgCode epsgOpCode;
	TcsEpsgCode srcEpsgId;
	TcsEpsgCode trgEpsgId;
	TcsEpsgCode srcGeoCRS;
	TcsEpsgCode trgGeoCRS;
	TcsEpsgCode epsgAreaCode;

	char gxName [64];
	char srcDatum [cs_KEYNM_DEF];
	char trgDatum [cs_KEYNM_DEF];
	char cTemp [512];

	wchar_t srcDatumW [cs_KEYNM_DEF];
	wchar_t trgDatumW [cs_KEYNM_DEF];

	struct cs_GeodeticTransform_ xform;

	memset ((void*)&xform,'\0',sizeof (xform));

	const TcsEpsgDataSetV6* epsgPtr = GetEpsgObjectPtr ();

	CS_stncp (srcDatum,dtDefPtr->key_nm,sizeof (srcDatum));
	CS_stncp (trgDatum,dtmPivot,sizeof (trgDatum));

	// Adjust the source and target datums based on the existing to84_via
	// value, and if such adjustment is made, need to write a path to the
	// path stream.
	csAdjustSrcAndTrg (gpStrm,srcDatum,trgDatum,dtDefPtr,dtmPivot);

	// Below, we will try to find specific values for the following.
	// We initialize to specific default values in case it doesn't work
	// out for this datum.
	accuracy = cs_Zero;
	epsgOpCode = 0UL;
	epsgVariant = 0UL;
	usefulRngLngW = cs_Zero;
	usefulRngLngE = cs_Zero;
	usefulRngLatS = cs_Zero;
	usefulRngLatN = cs_Zero;

	// See if we have EPSG numbers for both the source and target datums.
	mbstowcs (srcDatumW,srcDatum,wcCount (srcDatumW));
	mbstowcs (trgDatumW,trgDatum,wcCount (trgDatumW));
	srcEpsgId = csMapNameToId (csMapDatumKeyName,csMapFlvrEpsg,csMapFlvrCsMap,srcDatumW);
	trgEpsgId = csMapNameToId (csMapDatumKeyName,csMapFlvrEpsg,csMapFlvrCsMap,trgDatumW);

	// If we have valid numbers, see what we can do with them with regards to
	// the EPSG Parameter Dataset.
	bool epsgOk = (srcEpsgId != KcsNmInvNumber && trgEpsgId != KcsNmInvNumber);
	if (epsgOk)
	{
		// We have an EPSG ID number for both datums.  These doesn't has little
		// value of its own.  We need to convert that to the code for the
		// geographic CRS (usually 2D) which represents that datum.
		TcsEpsgCode srcGeoCRS, trgGeoCRS;
		epsgOk  = epsgPtr->LocateGeographicBase (srcGeoCRS,epsgCrsTypGeographic2D,srcEpsgId);
		epsgOk &= epsgPtr->LocateGeographicBase (trgGeoCRS,epsgCrsTypGeographic2D,trgEpsgId);
		if (epsgOk)
		{
			// Get a list of the variants which will convert from the source
			// datum to the target datum.  This is not trivial and we have,
			// in the past, developed a specific object for this purpose.
			TcsOpVariants dtmOpVariants (*epsgPtr,srcGeoCRS,trgGeoCRS);

			// How many did we find?  Since we'll use the first one, which
			// should be the one with highest accuracy (lowest accuracy
			// value) if there is more than one, we're really only interested
			// in the case where there are more than zero variants at
			// this point.  Maybe later on we'll get more sophisticated.
			unsigned variantCount = dtmOpVariants.GetVariantCount ();
			if (variantCount > 0)
			{
				// Get a pointer to the first variant.  Should be the
				// most accurate.  Until we have time to locate the best
				// match, we'll use this one.
				const TcsOpVariant* varPtr = dtmOpVariants.GetVariantPtr (0);
				accuracy = varPtr->GetAccuracy ();
				epsgOpCode = varPtr->GetOpCodeForCsMap ();
				epsgVariant = varPtr->GetVariantNbr ();
				epsgOk = epsgOpCode.IsValid ();
			}
		}
		if (epsgOk)
		{
			// We should have a valid EPSG Operation code.  We now use it to
			// extracte a useful range from the EPSG database.   Essentially,
			// we need to get the area code from the Operation table, and then
			// get the range from the Area table.
			epsgOk = epsgPtr->GetFieldByCode (epsgAreaCode,epsgTblCoordinateOperation,epsgFldAreaOfUseCode,epsgOpCode);
			if (epsgOk)
			{
				epsgOk = epsgAreaCode.IsValid ();
			}
		}
		if (epsgOk)
		{
			epsgOk  = epsgPtr->GetFieldByCode (usefulRngLngW,epsgTblArea,epsgFldAreaWestBoundLng,epsgAreaCode);
			epsgOk &= epsgPtr->GetFieldByCode (usefulRngLngE,epsgTblArea,epsgFldAreaEastBoundLng,epsgAreaCode);
			epsgOk &= epsgPtr->GetFieldByCode (usefulRngLatS,epsgTblArea,epsgFldAreaSouthBoundLat,epsgAreaCode);
			epsgOk &= epsgPtr->GetFieldByCode (usefulRngLatN,epsgTblArea,epsgFldAreaNorthBoundLat,epsgAreaCode);
		}
		if (epsgOk)
		{
			double rngDelta = fabs (usefulRngLngE - usefulRngLngW) * 0.125;
			usefulRngLngE += rngDelta;
			usefulRngLngW -= rngDelta;
			if (usefulRngLngE >  180.0) usefulRngLngE =  180.0;
			if (usefulRngLngW < -180.0) usefulRngLngE = -180.0;

			rngDelta = fabs (usefulRngLatN - usefulRngLatS) * 0.125;
			usefulRngLatN += rngDelta;
			usefulRngLatS -= rngDelta;
			if (usefulRngLatN >  90.0) usefulRngLatN =  90.0;
			if (usefulRngLatS < -90.0) usefulRngLatS = -90.0;
		}
	}

	// If the method type is multipel regression, we get the useful range
	// from the .MRT file.
	if (dtDefPtr->to84_via == cs_DTCTYP_MREG)
	{
		double rangeLng [2];
		double rangeLat [2];
	
		ok = csExtractMrtRange (rangeLng,rangeLat,dtDefPtr);
		
		// We don't extend the range of this defiunition.
		usefulRngLngW = rangeLng [0];
		usefulRngLngE = rangeLng [1];
		usefulRngLatS = rangeLat [0];
		usefulRngLatN = rangeLat [1];
	}

	if (accuracy == cs_Zero)
	{
		switch (dtDefPtr->to84_via) {
		case cs_DTCTYP_MOLO:
		case cs_DTCTYP_GEOCTR:
		case cs_DTCTYP_3PARM:
		case cs_DTCTYP_4PARM:
			accuracy = 8.0;
			break;
		case cs_DTCTYP_BURS:
		case cs_DTCTYP_6PARM:
			accuracy = 5.0;
			break;
		case cs_DTCTYP_WGS72:
		case cs_DTCTYP_7PARM:
		case cs_DTCTYP_MREG:
			accuracy = 3.0;
			break;
		case cs_DTCTYP_NAD27:
		case cs_DTCTYP_AGD66:
		case cs_DTCTYP_AGD84:
		case cs_DTCTYP_NZGD49:
		case cs_DTCTYP_ATS77:
		case cs_DTCTYP_TOKYO:
		case cs_DTCTYP_RGF93:
		case cs_DTCTYP_ED50:
		case cs_DTCTYP_DHDN:
		case cs_DTCTYP_CHENYX:
			accuracy = 1.0;
			break;
		case cs_DTCTYP_HPGN:
		case cs_DTCTYP_CSRS:
			accuracy = 0.5;
			break;
		case cs_DTCTYP_GDA94:
		case cs_DTCTYP_NZGD2K:
		case cs_DTCTYP_ETRF89:
		case cs_DTCTYP_NAD83:
		case cs_DTCTYP_WGS84:
			accuracy = 0.5;
			break;
		case cs_DTCTYP_NONE:
		default:
			accuracy = 0.0;
			break;
		}
	}

	// Set the maxIteration and convergence values per the to84_via variable.
	// The values were hard coded in CS-MAP prior to this implementation. */
	switch (dtDefPtr->to84_via) {
	// The following values were chosen to match identically to those hard
	// coded into CS-MAP prior to the RFC #2 implementation.
	case cs_DTCTYP_MOLO:
	case cs_DTCTYP_GEOCTR:
	case cs_DTCTYP_BURS:
	case cs_DTCTYP_4PARM:
	case cs_DTCTYP_6PARM:
	case cs_DTCTYP_MREG:
	case cs_DTCTYP_3PARM:		// Deprecated, legacy use only.
		xform.maxIterations = 8;
		xform.cnvrgValue    = 1.0E-09;
		xform.errorValue    = 1.0E-06;
		break;

	case cs_DTCTYP_7PARM:
		xform.maxIterations = 20;
		xform.cnvrgValue    = 1.0E-09;
		xform.errorValue    = 1.0E-06;
		break;

	case cs_DTCTYP_NAD27:
		xform.maxIterations = 10;
		xform.cnvrgValue    = 1.0E-11;
		xform.errorValue    = 5.0E-08;
		break;

	case cs_DTCTYP_AGD66:
	case cs_DTCTYP_AGD84:
	case cs_DTCTYP_NZGD49:
	case cs_DTCTYP_CSRS:
	case cs_DTCTYP_ATS77:
	case cs_DTCTYP_ED50:
	case cs_DTCTYP_DHDN:
	case cs_DTCTYP_CHENYX:
		xform.maxIterations = 10;
		xform.cnvrgValue    = 1.0E-09;
		xform.errorValue    = 5.0E-08;
		break;

	case cs_DTCTYP_RGF93:
		xform.maxIterations = 20;
		xform.cnvrgValue    = 1.0E-09;
		xform.errorValue    = 1.0E-06;
		break;

	case cs_DTCTYP_TOKYO:
		xform.maxIterations = 10;
		xform.cnvrgValue    = 1.0E-09;
		xform.errorValue    = 1.0E-06;
		break;

	case cs_DTCTYP_HPGN:
		xform.maxIterations = 10;
		xform.cnvrgValue    = 1.0E-09;
		xform.errorValue    = 5.0E-08;
		break;

	case cs_DTCTYP_WGS72:		// Not used in this method, assign basic defaults.
	case cs_DTCTYP_GDA94:		// Not used in this method, assign basic defaults.
	case cs_DTCTYP_NZGD2K:		// Not used in this method, assign basic defaults.
	case cs_DTCTYP_ETRF89:		// Not used in this method, assign basic defaults.
	case cs_DTCTYP_NAD83:		// Not used in this method, assign basic defaults.
	case cs_DTCTYP_WGS84:		// Not used in this method, assign basic defaults.
	default:
		xform.maxIterations = 10;
		xform.cnvrgValue    = 1.0E-11;
		xform.errorValue    = 5.0E-08;
	}

	// OK, the rest of this is pretty straight forward.
	sprintf (gxName,"%s_to_%s",srcDatum,trgDatum);
	CS_stncp (xform.xfrmName,gxName,sizeof (xform.xfrmName));
	CS_stncp (xform.srcDatum,srcDatum,sizeof (xform.srcDatum));
	CS_stncp (xform.trgDatum,trgDatum,sizeof (xform.trgDatum));
	if (dtDefPtr->group [0] != '\0')
	{
		CS_stncp (xform.group,dtDefPtr->group,sizeof (xform.group));
	}
	CS_stncp (xform.description,dtDefPtr->name,sizeof (xform.description));
	CS_stncp (xform.source,dtDefPtr->source,sizeof (xform.source));
	if (epsgOpCode.IsValid ())
	{
		xform.epsgCode = static_cast<short>(epsgOpCode);
	}
	if (epsgVariant != 0)
	{
		xform.epsgVariation = static_cast<short>(epsgVariant);
	}
	xform.inverseSupported = TRUE;
	xform.protect = 0;
	xform.accuracy = accuracy;
	if ((usefulRngLngW != cs_Zero || usefulRngLngE != cs_Zero) &&
		(usefulRngLatS != cs_Zero || usefulRngLatN != cs_Zero))
	{
		xform.rangeMinLng = usefulRngLngW;
		xform.rangeMinLat = usefulRngLatS;
		xform.rangeMaxLng = usefulRngLngE;
		xform.rangeMaxLat = usefulRngLatN;
	}

	switch (dtDefPtr->to84_via) {

	case cs_DTCTYP_MOLO:
		xform.methodCode = cs_DTCMTH_MOLOD;
		xform.parameters.geocentricParameters.deltaX = dtDefPtr->delta_X;
		xform.parameters.geocentricParameters.deltaY = dtDefPtr->delta_Y;
		xform.parameters.geocentricParameters.deltaZ = dtDefPtr->delta_Z;
		break;

	case cs_DTCTYP_3PARM:
		xform.methodCode = cs_DTCMTH_3PARM;
		xform.parameters.geocentricParameters.deltaX = dtDefPtr->delta_X;
		xform.parameters.geocentricParameters.deltaY = dtDefPtr->delta_Y;
		xform.parameters.geocentricParameters.deltaZ = dtDefPtr->delta_Z;
		break;

	case cs_DTCTYP_GEOCTR:
		xform.methodCode = cs_DTCMTH_GEOCT;
		xform.parameters.geocentricParameters.deltaX = dtDefPtr->delta_X;
		xform.parameters.geocentricParameters.deltaY = dtDefPtr->delta_Y;
		xform.parameters.geocentricParameters.deltaZ = dtDefPtr->delta_Z;
		break;

	case cs_DTCTYP_4PARM:
		xform.methodCode = cs_DTCMTH_4PARM;
		xform.parameters.geocentricParameters.deltaX = dtDefPtr->delta_X;
		xform.parameters.geocentricParameters.deltaY = dtDefPtr->delta_Y;
		xform.parameters.geocentricParameters.deltaZ = dtDefPtr->delta_Z;
		xform.parameters.geocentricParameters.scale  = dtDefPtr->bwscale;
		break;

	case cs_DTCTYP_6PARM:
		xform.methodCode = cs_DTCMTH_6PARM;
		xform.parameters.geocentricParameters.deltaX  = dtDefPtr->delta_X;
		xform.parameters.geocentricParameters.deltaY  = dtDefPtr->delta_Y;
		xform.parameters.geocentricParameters.deltaZ  = dtDefPtr->delta_Z;
		xform.parameters.geocentricParameters.rotateX = dtDefPtr->rot_X;
		xform.parameters.geocentricParameters.rotateY = dtDefPtr->rot_Y;
		xform.parameters.geocentricParameters.rotateZ = dtDefPtr->rot_Z;
		break;

	case cs_DTCTYP_BURS:
		xform.methodCode = cs_DTCMTH_BURSA;
		xform.parameters.geocentricParameters.deltaX  = dtDefPtr->delta_X;
		xform.parameters.geocentricParameters.deltaY  = dtDefPtr->delta_Y;
		xform.parameters.geocentricParameters.deltaZ  = dtDefPtr->delta_Z;
		xform.parameters.geocentricParameters.rotateX = dtDefPtr->rot_X;
		xform.parameters.geocentricParameters.rotateY = dtDefPtr->rot_Y;
		xform.parameters.geocentricParameters.rotateZ = dtDefPtr->rot_Z;
		xform.parameters.geocentricParameters.scale  = dtDefPtr->bwscale;
		break;

	case cs_DTCTYP_7PARM:
		xform.methodCode = cs_DTCMTH_7PARM;
		xform.parameters.geocentricParameters.deltaX  = dtDefPtr->delta_X;
		xform.parameters.geocentricParameters.deltaY  = dtDefPtr->delta_Y;
		xform.parameters.geocentricParameters.deltaZ  = dtDefPtr->delta_Z;
		xform.parameters.geocentricParameters.rotateX = dtDefPtr->rot_X;
		xform.parameters.geocentricParameters.rotateY = dtDefPtr->rot_Y;
		xform.parameters.geocentricParameters.rotateZ = dtDefPtr->rot_Z;
		xform.parameters.geocentricParameters.scale  = dtDefPtr->bwscale;
		break;

	case cs_DTCTYP_MREG:
		xform.methodCode = cs_DTCMTH_MULRG;
		ok = csConvertMrtFile (&xform.parameters.dmaMulRegParameters,dtDefPtr,mregAsc);
		break;

	case cs_DTCTYP_NAD27:
		xform.methodCode = cs_DTCMTH_GFILE;
		count = gdcNad27ToNad83->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcNad27ToNad83->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcNad27ToNad83->ConvertToRelative (cTemp);
			fileParmPtr = &xform.parameters.fileParameters.fileNames [index];
			fileParmPtr->fileFormat = cs_DTCFRMT_NADCN;
			fileParmPtr->direction = cs_DTCDIR_FWD;
			CS_stncp (fileParmPtr->fileName,cTemp,sizeof (fileParmPtr->fileName));
			xform.parameters.fileParameters.fileReferenceCount += 1;
		}
		kCp = gdcNad27ToNad83->GetFallbackDatum ();
		if (kCp != 0 && *kCp != '\0')
		{
			sprintf (cTemp,"%s_to_WGS84",kCp);
			CS_stncp (xform.parameters.fileParameters.fallback,cTemp,sizeof (xform.parameters.fileParameters.fallback));
		}
		break;

	case cs_DTCTYP_NAD83:
		xform.methodCode = cs_DTCMTH_NULLX;
		break;

	case cs_DTCTYP_WGS84:
		xform.methodCode = cs_DTCMTH_NULLX;
		break;

	case cs_DTCTYP_WGS72:
		xform.methodCode = cs_DTCMTH_WGS72;
		break;

	case cs_DTCTYP_HPGN:
		xform.methodCode = cs_DTCMTH_GFILE;
		count = gdcNad83ToHarn->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcNad83ToHarn->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcNad83ToHarn->ConvertToRelative (cTemp);
			fileParmPtr = &xform.parameters.fileParameters.fileNames [index];
			fileParmPtr->fileFormat = cs_DTCFRMT_NADCN;
			fileParmPtr->direction = cs_DTCDIR_FWD;
			CS_stncp (fileParmPtr->fileName,cTemp,sizeof (fileParmPtr->fileName));
			xform.parameters.fileParameters.fileReferenceCount += 1;
		}
		kCp = gdcNad83ToHarn->GetFallbackDatum ();
		if (kCp != 0 && *kCp != '\0')
		{
			sprintf (cTemp,"%s_to_WGS84",kCp);
			CS_stncp (xform.parameters.fileParameters.fallback,cTemp,sizeof (xform.parameters.fileParameters.fallback));
		}
		break;

	case cs_DTCTYP_AGD66:
		xform.methodCode = cs_DTCMTH_GFILE;
		count = gdcAgd66ToGda94->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcAgd66ToGda94->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcAgd66ToGda94->ConvertToRelative (cTemp);
			fileParmPtr = &xform.parameters.fileParameters.fileNames [index];
			fileParmPtr->fileFormat = cs_DTCFRMT_CNTv2;
			fileParmPtr->direction = cs_DTCDIR_FWD;
			CS_stncp (fileParmPtr->fileName,cTemp,sizeof (fileParmPtr->fileName));
			xform.parameters.fileParameters.fileReferenceCount += 1;
		}
		kCp = gdcAgd66ToGda94->GetFallbackDatum ();
		if (kCp != 0 && *kCp != '\0')
		{
			sprintf (cTemp,"%s_to_WGS84",kCp);
			CS_stncp (xform.parameters.fileParameters.fallback,cTemp,sizeof (xform.parameters.fileParameters.fallback));
		}
		break;

	case cs_DTCTYP_AGD84:
		xform.methodCode = cs_DTCMTH_GFILE;
		count = gdcAgd84ToGda94->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcAgd84ToGda94->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcAgd84ToGda94->ConvertToRelative (cTemp);
			fileParmPtr = &xform.parameters.fileParameters.fileNames [index];
			fileParmPtr->fileFormat = cs_DTCFRMT_CNTv2;
			fileParmPtr->direction = cs_DTCDIR_FWD;
			CS_stncp (fileParmPtr->fileName,cTemp,sizeof (fileParmPtr->fileName));
			xform.parameters.fileParameters.fileReferenceCount += 1;
		}
		kCp = gdcAgd84ToGda94->GetFallbackDatum ();
		if (kCp != 0 && *kCp != '\0')
		{
			sprintf (cTemp,"%s_to_WGS84",kCp);
			CS_stncp (xform.parameters.fileParameters.fallback,cTemp,sizeof (xform.parameters.fileParameters.fallback));
		}
		break;

	case cs_DTCTYP_NZGD49:
		xform.methodCode = cs_DTCMTH_GFILE;
		count = gdcNzgd49ToNzgd2K->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcNzgd49ToNzgd2K->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcNzgd49ToNzgd2K->ConvertToRelative (cTemp);
			fileParmPtr = &xform.parameters.fileParameters.fileNames [index];
			fileParmPtr->fileFormat = cs_DTCFRMT_CNTv2;
			fileParmPtr->direction = cs_DTCDIR_FWD;
			CS_stncp (fileParmPtr->fileName,cTemp,sizeof (fileParmPtr->fileName));
			xform.parameters.fileParameters.fileReferenceCount += 1;
		}
		kCp = gdcNzgd49ToNzgd2K->GetFallbackDatum ();
		if (kCp != 0 && *kCp != '\0')
		{
			sprintf (cTemp,"%s_to_WGS84",kCp);
			CS_stncp (xform.parameters.fileParameters.fallback,cTemp,sizeof (xform.parameters.fileParameters.fallback));
		}
		break;

	case cs_DTCTYP_ATS77:
		xform.methodCode = cs_DTCMTH_GFILE;
		count = gdcAts77ToCsrs->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcAts77ToCsrs->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcAts77ToCsrs->ConvertToRelative (cTemp);
			fileParmPtr = &xform.parameters.fileParameters.fileNames [index];
			fileParmPtr->fileFormat = cs_DTCFRMT_CNTv2;
			fileParmPtr->direction = cs_DTCDIR_FWD;
			CS_stncp (fileParmPtr->fileName,cTemp,sizeof (fileParmPtr->fileName));
			xform.parameters.fileParameters.fileReferenceCount += 1;
		}
		kCp = gdcAts77ToCsrs->GetFallbackDatum ();
		if (kCp != 0 && *kCp != '\0')
		{
			sprintf (cTemp,"%s_to_WGS84",kCp);
			CS_stncp (xform.parameters.fileParameters.fallback,cTemp,sizeof (xform.parameters.fileParameters.fallback));
		}
		break;

	case cs_DTCTYP_GDA94:
		xform.methodCode = cs_DTCMTH_NULLX;
		break;

	case cs_DTCTYP_NZGD2K:
		xform.methodCode = cs_DTCMTH_NULLX;
		break;

	case cs_DTCTYP_CSRS:
		xform.methodCode = cs_DTCMTH_GFILE;
		count = gdcNad83ToCsrs->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcNad83ToCsrs->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcNad83ToCsrs->ConvertToRelative (cTemp);
			fileParmPtr = &xform.parameters.fileParameters.fileNames [index];
			fileParmPtr->fileFormat = cs_DTCFRMT_CNTv2;
			fileParmPtr->direction = cs_DTCDIR_INV;
			CS_stncp (fileParmPtr->fileName,cTemp,sizeof (fileParmPtr->fileName));
			xform.parameters.fileParameters.fileReferenceCount += 1;
		}
		kCp = gdcNad83ToCsrs->GetFallbackDatum ();
		if (kCp != 0 && *kCp != '\0')
		{
			sprintf (cTemp,"%s_to_WGS84",kCp);
			CS_stncp (xform.parameters.fileParameters.fallback,cTemp,sizeof (xform.parameters.fileParameters.fallback));
		}
		break;

	case cs_DTCTYP_TOKYO:
		xform.methodCode = cs_DTCMTH_GFILE;
		count = gdcTokyoToJgd2k->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcTokyoToJgd2k->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcTokyoToJgd2k->ConvertToRelative (cTemp);
			fileParmPtr = &xform.parameters.fileParameters.fileNames [index];
			fileParmPtr->fileFormat = cs_DTCFRMT_JAPAN;
			fileParmPtr->direction = cs_DTCDIR_FWD;
			CS_stncp (fileParmPtr->fileName,cTemp,sizeof (fileParmPtr->fileName));
			xform.parameters.fileParameters.fileReferenceCount += 1;
		}
		kCp = gdcTokyoToJgd2k->GetFallbackDatum ();
		if (kCp != 0 && *kCp != '\0')
		{
			sprintf (cTemp,"%s_to_WGS84",kCp);
			CS_stncp (xform.parameters.fileParameters.fallback,cTemp,sizeof (xform.parameters.fileParameters.fallback));
		}
		break;

	case cs_DTCTYP_RGF93:
		xform.methodCode = cs_DTCMTH_GFILE;
		count = gdcRgf93ToNtf->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcRgf93ToNtf->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcRgf93ToNtf->ConvertToRelative (cTemp);
			cp = strrchr (cTemp,'.');
			fileParmPtr = &xform.parameters.fileParameters.fileNames [index];
			fileParmPtr->fileFormat = cs_DTCFRMT_JAPAN;
			fileParmPtr->direction = cs_DTCDIR_FWD;
			if (!(CS_strnicmp (cp,".gsb",4)))
			{
				fileParmPtr->fileFormat = cs_DTCFRMT_CNTv2;
				fileParmPtr->direction = cs_DTCDIR_INV;
			}
			else
			{
				fileParmPtr->fileFormat = cs_DTCFRMT_FRNCH;
				fileParmPtr->direction = cs_DTCDIR_FWD;
			}
			CS_stncp (fileParmPtr->fileName,cTemp,sizeof (fileParmPtr->fileName));
			xform.parameters.fileParameters.fileReferenceCount += 1;

			kCp = gdcRgf93ToNtf->GetFallbackDatum ();
			if (kCp != 0 && *kCp != '\0')
			{
				sprintf (cTemp,"WGS84_to_%s",kCp);
				CS_stncp (xform.parameters.fileParameters.fallback,cTemp,sizeof (xform.parameters.fileParameters.fallback));
			}
		}
		break;

	case cs_DTCTYP_ED50:
		xform.methodCode = cs_DTCMTH_GFILE;
		count = gdcEd50ToEtrf89->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcEd50ToEtrf89->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcEd50ToEtrf89->ConvertToRelative (cTemp);
			fileParmPtr = &xform.parameters.fileParameters.fileNames [index];
			fileParmPtr->fileFormat = cs_DTCFRMT_CNTv2;
			fileParmPtr->direction = cs_DTCDIR_FWD;
			CS_stncp (fileParmPtr->fileName,cTemp,sizeof (fileParmPtr->fileName));
			xform.parameters.fileParameters.fileReferenceCount += 1;
		}
		kCp = gdcEd50ToEtrf89->GetFallbackDatum ();
		if (kCp != 0 && *kCp != '\0')
		{
			sprintf (cTemp,"%s_to_WGS84",kCp);
			CS_stncp (xform.parameters.fileParameters.fallback,cTemp,sizeof (xform.parameters.fileParameters.fallback));
		}
		break;

	case cs_DTCTYP_DHDN:
		xform.methodCode = cs_DTCMTH_GFILE;
		count = gdcDhdnToEtrf89->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcDhdnToEtrf89->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcDhdnToEtrf89->ConvertToRelative (cTemp);
			fileParmPtr = &xform.parameters.fileParameters.fileNames [index];
			fileParmPtr->fileFormat = cs_DTCFRMT_CNTv2;
			fileParmPtr->direction = cs_DTCDIR_FWD;
			CS_stncp (fileParmPtr->fileName,cTemp,sizeof (fileParmPtr->fileName));
			xform.parameters.fileParameters.fileReferenceCount += 1;
		}
		kCp = gdcDhdnToEtrf89->GetFallbackDatum ();
		if (kCp != 0 && *kCp != '\0')
		{
			sprintf (cTemp,"%s_to_WGS84",kCp);
			CS_stncp (xform.parameters.fileParameters.fallback,cTemp,sizeof (xform.parameters.fileParameters.fallback));
		}
		break;

	case cs_DTCTYP_ETRF89:
		xform.methodCode = cs_DTCMTH_NULLX;
		break;

	case cs_DTCTYP_CHENYX:
		xform.methodCode = cs_DTCMTH_GFILE;
		count = gdcCh1903ToPlus->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcCh1903ToPlus->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcCh1903ToPlus->ConvertToRelative (cTemp);
			fileParmPtr = &xform.parameters.fileParameters.fileNames [index];
			fileParmPtr->fileFormat = cs_DTCFRMT_CNTv2;
			fileParmPtr->direction = cs_DTCDIR_FWD;
			CS_stncp (fileParmPtr->fileName,cTemp,sizeof (fileParmPtr->fileName));
			xform.parameters.fileParameters.fileReferenceCount += 1;
		}
		// The original fallback specification was wrong, so we ignore it here.  The
		// null transformation is a better approximation than the originally specified
		// CH1903.
		break;

	default:
		ok = false;
		break;
	}

/******************************************************************************
*******************************************************************************
**********              Special  One Time Kludge                       ********
*******************************************************************************
******************************************************************************/
if (CS_stricmp (dtDefPtr->key_nm,"Ocotepeque35b") == 0)
{
	// During the same release as this major overhaul, we had an emergency
	// request for this transformation based on the Molodensky-Badekus
	// transformation.
	xform.methodCode = cs_DTCMTH_BDKAS;
	xform.parameters.geocentricParameters.deltaX     = 213.116;
	xform.parameters.geocentricParameters.deltaY     = 9.358;
	xform.parameters.geocentricParameters.deltaZ     = -74.946;
	xform.parameters.geocentricParameters.rotateX    = -2.351418791;
	xform.parameters.geocentricParameters.rotateY    = 0.061466912;
	xform.parameters.geocentricParameters.rotateZ    = -6.394208994;
	xform.parameters.geocentricParameters.scale      = -5.22;
	xform.parameters.geocentricParameters.translateX = 617749.7118;
	xform.parameters.geocentricParameters.translateY = -6250547.7336;
	xform.parameters.geocentricParameters.translateZ = 1102063.6099;
}
	if (ok)
	{
		gtStrm << std::endl;
		ok = csXformToStream (gtStrm,&xform);
	}
	return ok;
}
bool csXformToStream (std::wofstream& gtStrm,cs_GeodeticTransform_* xfrmPtr)
{
	bool ok (true);

	short idx;
	short uuPwr;
	short vvPwr;

	wchar_t* yesPtr;
	wchar_t* dirPtr;
	wchar_t* frmtPtr;
	
	double coefficient;
	
	char cTemp1 [256];
	char cTemp2 [256];

	csGeodeticXfromParmsFile_* fileParmPtr;
	cs_GeodeticTransform_::csGeodeticXformParameters::csGeodeticXformParmsGeocentric* geoCtrPtr;
	cs_GeodeticTransform_::csGeodeticXformParameters::csGeodeticXformParmsGridFiles_* gridFilesPtr;
	cs_GeodeticTransform_::csGeodeticXformParameters::csGeodeticXformParmsDmaMulReg_* mulrgParmPtr;

	wchar_t wcTemp [512];

	yesPtr = (xfrmPtr->inverseSupported != 0) ? L"Yes" : L"No";
	gtStrm << L"GX_NAME: "      << xfrmPtr->xfrmName               << std::endl;
	gtStrm <<   L"\t SRC_DTM: " << xfrmPtr->srcDatum               << std::endl;
	gtStrm <<   L"\t TRG_DTM: " << xfrmPtr->trgDatum               << std::endl;
	if (xfrmPtr->group [0] != '\0')
	{
		gtStrm << L"\t   GROUP: "  << xfrmPtr->group               << std::endl;
	}
	gtStrm <<   L"\t DESC_NM: " << xfrmPtr->description            << std::endl;
	gtStrm <<   L"\t  SOURCE: " << xfrmPtr->source                 << std::endl;
	gtStrm <<   L"\tEPSG_NBR: " << xfrmPtr->epsgCode               << std::endl;
	gtStrm <<   L"\t INVERSE: " << yesPtr                          << std::endl;
	gtStrm <<   L"\t MAX_ITR: " << xfrmPtr->maxIterations          << std::endl;
	gtStrm << L"   CNVRG_VAL: " << xfrmPtr->cnvrgValue             << std::endl;
	gtStrm << L"   ERROR_VAL: " << xfrmPtr->errorValue             << std::endl;

	if (xfrmPtr->accuracy > 0.0)
	{
		gtStrm <<   L"\tACCURACY: " << dbl2wcs (xfrmPtr->accuracy) << std::endl;
	}

	if (xfrmPtr->epsgCode != 0)
	{
		gtStrm <<   L"\tEPSG_NBR: " << static_cast<unsigned long>(xfrmPtr->epsgCode) << std::endl;
	}
	if (xfrmPtr->epsgVariation != 0)
	{
		gtStrm <<   L"\tEPSG_VAR: " << xfrmPtr->epsgVariation << std::endl;
	}
	if ((xfrmPtr->rangeMinLng != cs_Zero || xfrmPtr->rangeMaxLng != cs_Zero) ||
		(xfrmPtr->rangeMinLat != cs_Zero || xfrmPtr->rangeMaxLat != cs_Zero))
	{
		// We have some meaningful useful range data.
		gtStrm <<   L"\t MIN_LNG: " << dbl2wcs (xfrmPtr->rangeMinLng) << std::endl;
		gtStrm <<   L"\t MAX_LNG: " << dbl2wcs (xfrmPtr->rangeMaxLng) << std::endl;
		gtStrm <<   L"\t MIN_LAT: " << dbl2wcs (xfrmPtr->rangeMinLat) << std::endl;
		gtStrm <<   L"\t MAX_LAT: " << dbl2wcs (xfrmPtr->rangeMaxLat) << std::endl;
	}

	switch (xfrmPtr->methodCode){

	case cs_DTCMTH_NULLX:
		gtStrm <<   L"\t  METHOD: " << L"NULL"                  << std::endl;
		break;

	case cs_DTCMTH_WGS72:
		gtStrm <<   L"\t  METHOD: " << L"WGS72"                 << std::endl;
		break;

	case cs_DTCMTH_3PARM:
		gtStrm <<   L"\t  METHOD: " << L"3PARAMETER"                 << std::endl;
		geoCtrPtr = &xfrmPtr->parameters.geocentricParameters;
		gtStrm <<   L"\t DELTA_X: " << dbl2wcs (geoCtrPtr->deltaX)  << std::endl;
		gtStrm <<   L"\t DELTA_Y: " << dbl2wcs (geoCtrPtr->deltaY)  << std::endl;
		gtStrm <<   L"\t DELTA_Z: " << dbl2wcs (geoCtrPtr->deltaZ)  << std::endl;
		break;
		
	case cs_DTCMTH_MOLOD:
		gtStrm <<   L"\t  METHOD: " << L"MOLODENSKY"                << std::endl;
		geoCtrPtr = &xfrmPtr->parameters.geocentricParameters;
		gtStrm <<   L"\t DELTA_X: " << dbl2wcs (geoCtrPtr->deltaX)  << std::endl;
		gtStrm <<   L"\t DELTA_Y: " << dbl2wcs (geoCtrPtr->deltaY)  << std::endl;
		gtStrm <<   L"\t DELTA_Z: " << dbl2wcs (geoCtrPtr->deltaZ)  << std::endl;
		break;

	case cs_DTCMTH_AMOLO:
		gtStrm <<   L"\t  METHOD: " << L"3PARAMETER"                 << std::endl;
		geoCtrPtr = &xfrmPtr->parameters.geocentricParameters;
		gtStrm <<   L"\t DELTA_X: " << dbl2wcs (geoCtrPtr->deltaX)  << std::endl;
		gtStrm <<   L"\t DELTA_Y: " << dbl2wcs (geoCtrPtr->deltaY)  << std::endl;
		gtStrm <<   L"\t DELTA_Z: " << dbl2wcs (geoCtrPtr->deltaZ)  << std::endl;
		break;

	case cs_DTCMTH_GEOCT:
		gtStrm <<   L"\t  METHOD: " << L"GEOCENTRIC"                << std::endl;
		geoCtrPtr = &xfrmPtr->parameters.geocentricParameters;
		gtStrm <<   L"\t DELTA_X: " << dbl2wcs (geoCtrPtr->deltaX)  << std::endl;
		gtStrm <<   L"\t DELTA_Y: " << dbl2wcs (geoCtrPtr->deltaY)  << std::endl;
		gtStrm <<   L"\t DELTA_Z: " << dbl2wcs (geoCtrPtr->deltaZ)  << std::endl;
		break;

	case cs_DTCMTH_4PARM:
		gtStrm <<   L"\t  METHOD: " << L"4PARAMETER"                << std::endl;
		geoCtrPtr = &xfrmPtr->parameters.geocentricParameters;
		gtStrm <<   L"\t DELTA_X: " << dbl2wcs (geoCtrPtr->deltaX)  << std::endl;
		gtStrm <<   L"\t DELTA_Y: " << dbl2wcs (geoCtrPtr->deltaY)  << std::endl;
		gtStrm <<   L"\t DELTA_Z: " << dbl2wcs (geoCtrPtr->deltaZ)  << std::endl;
		gtStrm <<   L"\t BWSCALE: " << dbl2wcs (geoCtrPtr->scale)  << std::endl;
		break;

	case cs_DTCMTH_6PARM:
		gtStrm <<   L"\t  METHOD: " << L"6PARAMETER"                << std::endl;
		geoCtrPtr = &xfrmPtr->parameters.geocentricParameters;
		gtStrm <<   L"\t DELTA_X: " << dbl2wcs (geoCtrPtr->deltaX)  << std::endl;
		gtStrm <<   L"\t DELTA_Y: " << dbl2wcs (geoCtrPtr->deltaY)  << std::endl;
		gtStrm <<   L"\t DELTA_Z: " << dbl2wcs (geoCtrPtr->deltaZ)  << std::endl;
		gtStrm <<   L"\t   ROT_X: " << dbl2wcs (geoCtrPtr->rotateX)    << std::endl;
		gtStrm <<   L"\t   ROT_Y: " << dbl2wcs (geoCtrPtr->rotateY)    << std::endl;
		gtStrm <<   L"\t   ROT_Z: " << dbl2wcs (geoCtrPtr->rotateZ)    << std::endl;
		break;

	case cs_DTCMTH_BURSA:
		gtStrm <<   L"\t  METHOD: " << L"BURSAWOLF"                 << std::endl;
		geoCtrPtr = &xfrmPtr->parameters.geocentricParameters;
		gtStrm <<   L"\t DELTA_X: " << dbl2wcs (geoCtrPtr->deltaX)  << std::endl;
		gtStrm <<   L"\t DELTA_Y: " << dbl2wcs (geoCtrPtr->deltaY)  << std::endl;
		gtStrm <<   L"\t DELTA_Z: " << dbl2wcs (geoCtrPtr->deltaZ)  << std::endl;
		gtStrm <<   L"\t   ROT_X: " << dbl2wcs (geoCtrPtr->rotateX)    << std::endl;
		gtStrm <<   L"\t   ROT_Y: " << dbl2wcs (geoCtrPtr->rotateY)    << std::endl;
		gtStrm <<   L"\t   ROT_Z: " << dbl2wcs (geoCtrPtr->rotateZ)    << std::endl;
		gtStrm <<   L"\t BWSCALE: " << dbl2wcs (geoCtrPtr->scale)  << std::endl;
		break;

	case cs_DTCMTH_FRAME:
		gtStrm <<   L"\t  METHOD: " << L"BURSAFRAME"                 << std::endl;
		geoCtrPtr = &xfrmPtr->parameters.geocentricParameters;
		gtStrm <<   L"\t DELTA_X: " << dbl2wcs (geoCtrPtr->deltaX)  << std::endl;
		gtStrm <<   L"\t DELTA_Y: " << dbl2wcs (geoCtrPtr->deltaY)  << std::endl;
		gtStrm <<   L"\t DELTA_Z: " << dbl2wcs (geoCtrPtr->deltaZ)  << std::endl;
		gtStrm <<   L"\t   ROT_X: " << dbl2wcs (geoCtrPtr->rotateX)    << std::endl;
		gtStrm <<   L"\t   ROT_Y: " << dbl2wcs (geoCtrPtr->rotateY)    << std::endl;
		gtStrm <<   L"\t   ROT_Z: " << dbl2wcs (geoCtrPtr->rotateZ)    << std::endl;
		gtStrm <<   L"\t BWSCALE: " << dbl2wcs (geoCtrPtr->scale)  << std::endl;
		break;

	case cs_DTCMTH_7PARM:
		gtStrm <<   L"\t  METHOD: " << L"7PARAMETER"                << std::endl;
		geoCtrPtr = &xfrmPtr->parameters.geocentricParameters;
		gtStrm <<   L"\t DELTA_X: " << dbl2wcs (geoCtrPtr->deltaX)  << std::endl;
		gtStrm <<   L"\t DELTA_Y: " << dbl2wcs (geoCtrPtr->deltaY)  << std::endl;
		gtStrm <<   L"\t DELTA_Z: " << dbl2wcs (geoCtrPtr->deltaZ)  << std::endl;
		gtStrm <<   L"\t   ROT_X: " << dbl2wcs (geoCtrPtr->rotateX)    << std::endl;
		gtStrm <<   L"\t   ROT_Y: " << dbl2wcs (geoCtrPtr->rotateY)    << std::endl;
		gtStrm <<   L"\t   ROT_Z: " << dbl2wcs (geoCtrPtr->rotateZ)    << std::endl;
		gtStrm <<   L"\t BWSCALE: " << dbl2wcs (geoCtrPtr->scale)  << std::endl;
		break;

	case cs_DTCMTH_BDKAS:
		gtStrm <<   L"\t  METHOD: " << L"MOLOBADEKAS"                   << std::endl;
		geoCtrPtr = &xfrmPtr->parameters.geocentricParameters;
		gtStrm <<   L"\t DELTA_X: " << dbl2wcs (geoCtrPtr->deltaX)     << std::endl;
		gtStrm <<   L"\t DELTA_Y: " << dbl2wcs (geoCtrPtr->deltaY)     << std::endl;
		gtStrm <<   L"\t DELTA_Z: " << dbl2wcs (geoCtrPtr->deltaZ)     << std::endl;
		gtStrm <<   L"\t   ROT_X: " << dbl2wcs (geoCtrPtr->rotateX)       << std::endl;
		gtStrm <<   L"\t   ROT_Y: " << dbl2wcs (geoCtrPtr->rotateY)       << std::endl;
		gtStrm <<   L"\t   ROT_Z: " << dbl2wcs (geoCtrPtr->rotateZ)       << std::endl;
		gtStrm <<   L"\t BWSCALE: " << dbl2wcs (geoCtrPtr->scale)     << std::endl;
		gtStrm <<   L"\t XLATE_X: " << dbl2wcs (geoCtrPtr->translateX)  << std::endl;
		gtStrm <<   L"\t XLATE_Y: " << dbl2wcs (geoCtrPtr->translateY)  << std::endl;
		gtStrm <<   L"\t XLATE_Z: " << dbl2wcs (geoCtrPtr->translateZ)  << std::endl;
		break;

	case cs_DTCMTH_MULRG:
		gtStrm <<   L"\t  METHOD: " << L"MULREG"                                      << std::endl;
		mulrgParmPtr = &xfrmPtr->parameters.dmaMulRegParameters;
		CS_ftoa (cTemp1,sizeof (cTemp1),mulrgParmPtr->testPhi,cs_ATOF_LATDFLT+1);
		CS_ftoa (cTemp2,sizeof (cTemp1),mulrgParmPtr->testLambda,cs_ATOF_LNGDFLT+1);
		gtStrm << L"\t\t  TEST_LAT: " << cTemp1                                       << std::endl;
		gtStrm << L"\t\t  TEST_LNG: " << cTemp2                                       << std::endl;
		gtStrm << L"\tRSLT_DELTA_LAT: " << dbl2wcs (mulrgParmPtr->deltaPhi)           << std::endl;
		gtStrm << L"\tRSLT_DELTA_LNG: " << dbl2wcs (mulrgParmPtr->deltaLambda)        << std::endl;
		gtStrm << L"\tRSLT_DELTA_HGT: " << dbl2wcs (mulrgParmPtr->deltaHeight)        << std::endl;

		gtStrm << L"\t   SRC_LAT_OFF: " << dbl2wcs (mulrgParmPtr->phiOffset)          << std::endl;
		gtStrm << L"\t   SRC_LNG_OFF: " << dbl2wcs (mulrgParmPtr->lambdaOffset)       << std::endl;
		gtStrm << L"\t\t   NRML_KK: "   << dbl2wcs (mulrgParmPtr->normalizationScale) << std::endl;
		gtStrm << L"\t\tVALIDATION: "   << dbl2wcs (mulrgParmPtr->validation)         << std::endl;

		// Add the Longitude complex element.
		for (uuPwr = 0;uuPwr < 10;uuPwr += 1)
		{
			for (vvPwr = 0;vvPwr < 10;vvPwr += 1)
			{
				idx = uuPwr * 10 + vvPwr;
				coefficient = mulrgParmPtr->coeffLambda [idx];
				if (coefficient != 0.0)
				{
					swprintf (wcTemp,128,L"\tLNG_COEF U%d V%d: ",uuPwr,vvPwr);
					gtStrm << wcTemp << dbl2wcs (coefficient) << std::endl;
				}
			}
		}
	
		// Add the Latitude complex element.
		for (uuPwr = 0;uuPwr < 10;uuPwr += 1)
		{
			for (vvPwr = 0;vvPwr < 10;vvPwr += 1)
			{
				idx = uuPwr * 10 + vvPwr;
				coefficient = mulrgParmPtr->coeffPhi [idx];
				if (coefficient != 0.0)
				{
					swprintf (wcTemp,128,L"\tLAT_COEF U%d V%d: ",uuPwr,vvPwr);
					gtStrm << wcTemp << dbl2wcs (coefficient) << std::endl;
				}
			}
		}

		// Add the Height complex element.
		for (uuPwr = 0;uuPwr < 10;uuPwr += 1)
		{
			for (vvPwr = 0;vvPwr < 10;vvPwr += 1)
			{
				idx = uuPwr * 10 + vvPwr;
				coefficient = mulrgParmPtr->coeffHeight [idx];
				if (coefficient != 0.0)
				{
					swprintf (wcTemp,128,L"\tHGT_COEF U%d V%d: ",uuPwr,vvPwr);
					gtStrm << wcTemp << dbl2wcs (coefficient) << std::endl;
				}
			}
		}
		break;

	case cs_DTCMTH_GFILE:
		gtStrm <<   L"\t  METHOD: " << L"GRID_INTERP"             << std::endl;
		gridFilesPtr = &xfrmPtr->parameters.fileParameters;
		for (idx = 0;idx < gridFilesPtr->fileReferenceCount;idx += 1)
		{
			fileParmPtr = &gridFilesPtr->fileNames [idx];
			switch (fileParmPtr->direction) {
			case cs_DTCDIR_FWD:   dirPtr = L"Fwd";   break;
			case cs_DTCDIR_INV:   dirPtr = L"Inv";   break;
			case cs_DTCDIR_NONE:
			case cs_DTCDIR_ERR:
			default:
				dirPtr = L"None";
				break;
			}
			switch (fileParmPtr->fileFormat) {
			case cs_DTCFRMT_CNTv1:   frmtPtr = L"NTv1";     break;
			case cs_DTCFRMT_CNTv2:   frmtPtr = L"NTv2";     break;
			case cs_DTCFRMT_NADCN:   frmtPtr = L"NADCON";   break;
			case cs_DTCFRMT_FRNCH:   frmtPtr = L"FRRGF";    break;
			case cs_DTCFRMT_JAPAN:   frmtPtr = L"JPPAR";    break;
			case cs_DTCFRMT_ATS77:   frmtPtr = L"ATS77";    break;
			case cs_DTCFRMT_OST97:   frmtPtr = L"OSTN97";   break;
			case cs_DTCFRMT_OST02:   frmtPtr = L"OSTN02";   break;
			case cs_DTCFRMT_NONE:
			default:
				ok = false;
				frmtPtr = L"None";
				break;
			}
			mbstowcs (wcTemp,fileParmPtr->fileName,wcCount (wcTemp));		
			gtStrm << L"\t   GRID_FILE: "
				   << frmtPtr
				   << L','
				   << dirPtr
				   << L','
				   << wcTemp
				   << std::endl;
		}
		if (gridFilesPtr->fallback [0] != '\0')
		{
			mbstowcs (wcTemp,gridFilesPtr->fallback,wcCount (wcTemp));		
			gtStrm << L"\t    FALLBACK: "
				   << wcTemp
				   << std::endl;
		}
		break;

	case cs_DTCMTH_PLYNM:
	default:
		ok = false;
		break;
	}
	if (ok)
	{
		ok = gtStrm.good ();
	}	
	return ok;
}
bool csWriteHardCodedStuff (std::wofstream& gtStrm,std::wofstream& gpStrm,const wchar_t* csDictDir)
{
	bool ok (false);

	char srcDir [MAXPATH];
	wchar_t wrkBufr [260];

	// Copy the hard coded transformation info to the transformation stream.
	wcstombs (srcDir,csDictDir,sizeof (srcDir));
	strcat (srcDir,"\\gtHardCoded.asc");
	std::wifstream crStrmT (srcDir,std::ios_base::in);
	if (crStrmT.is_open ())
	{
		ok = true;
		gtStrm << std::endl;
		while (crStrmT.good ())
		{
			crStrmT.getline (wrkBufr,256);
			wrkBufr [255] = L'\0';
			gtStrm << wrkBufr << std::endl;
		}
		crStrmT.close ();
	}

	// Copy the hard coded path info to the path stream.
	if (ok)
	{
		ok = false;
		wcstombs (srcDir,csDictDir,sizeof (srcDir));
		strcat (srcDir,"\\gpHardCoded.asc");
		std::wifstream crStrmP (srcDir,std::ios_base::in);
		if (crStrmP.is_open ())
		{
			ok = true;
			gtStrm << std::endl;
			while (crStrmP.good ())
			{
				crStrmP.getline (wrkBufr,256);
				wrkBufr [255] = L'\0';
				gpStrm << wrkBufr << std::endl;
			}
			crStrmP.close ();
		}
	}
	return ok;
}

// The following uses the old CS-MAP to84_via element of the datum
// definition to determine the appropriate source and target datum
// names.  In the event that they are not the normal thing (key_nm
// and WGS84, respectively), an appropirate path which defines the
// the equivalent of a key_nm to WGS84 conversion is written to
// the provided geodetic path stream.

/******************************************************************************************
	{cs_DTCTYP_NAD27,
		{dtcTypNad27ToNad83,     dtcTypNad83ToWgs84,    dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToNad83,     dtcTypNad83ToNad27,    dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_NAD83,
		{dtcTypNad83ToWgs84,     dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToNad83,     dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_AGD66,
		{dtcTypAgd66ToGda94,     dtcTypGda94ToWgs84,    dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToGda94,     dtcTypGda94ToAgd66,    dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_AGD84,
		{dtcTypAgd84ToGda94,     dtcTypGda94ToWgs84,    dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToGda94,     dtcTypGda94ToAgd84,    dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_ED50,
		{dtcTypEd50ToEtrf89,     dtcTypEtrf89ToWgs84,   dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToEtrf89,    dtcTypEtrf89ToEd50,    dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_DHDN,
		{dtcTypDhdnToEtrf89,     dtcTypEtrf89ToWgs84,   dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToEtrf89,    dtcTypEtrf89ToDhdn,    dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_NZGD49,
		{dtcTypNzgd49ToNzgd2K,   dtcTypNzgd2KToWgs84,   dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToNzgd2K,    dtcTypNzgd2KToNzgd49,  dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_ATS77,
		{dtcTypAts77ToCsrs,      dtcTypCsrsToNad83,     dtcTypNad83ToWgs84,   dtcTypNone  },
		{dtcTypWgs84ToNad83,     dtcTypNad83ToCsrs,     dtcTypCsrsToAts77,    dtcTypNone  }
	},
	{cs_DTCTYP_WGS84,
		{dtcTypNone,             dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypNone,             dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_GDA94,
		{dtcTypGda94ToWgs84,     dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToGda94,     dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_NZGD2K,
		{dtcTypNzgd2KToWgs84,     dtcTypNone,            dtcTypNone,          dtcTypNone  },
		{dtcTypWgs84ToNzgd2K,     dtcTypNone,            dtcTypNone,          dtcTypNone  }
	},
	{cs_DTCTYP_CSRS,
		{dtcTypCsrsToNad83,       dtcTypNad83ToWgs84,    dtcTypNone,          dtcTypNone  },
		{dtcTypWgs84ToNad83,      dtcTypNad83ToCsrs,     dtcTypNone,          dtcTypNone  }
	},
	{cs_DTCTYP_TOKYO,
		{dtcTypTokyoToJgd2k,      dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypJgd2kToTokyo,      dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_WGS72,
		{dtcTypWgs72ToWgs84,     dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToWgs72,     dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_HPGN,
		{dtcTypHarnToNad83,      dtcTypNad83ToWgs84,    dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToNad83,     dtcTypNad83ToHarn,     dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_MOLO,
		{dtcTypMolodensky,       dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypMolodenskyInv,    dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_MREG,
		{dtcTypDMAMulReg,        dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypDMAMulRegInv,     dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_BURS,
		{dtcTypBursaWolf,        dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypBursaWolfInv,     dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_7PARM,
		{dtcTypSevenParm,        dtcTypNone,             dtcTypNone,          dtcTypNone  },
		{dtcTypSevenParmInv,     dtcTypNone,             dtcTypNone,          dtcTypNone  }
	},
	{cs_DTCTYP_6PARM,
		{dtcTypSixParm,          dtcTypNone,             dtcTypNone,          dtcTypNone  },
		{dtcTypSixParmInv,       dtcTypNone,             dtcTypNone,          dtcTypNone  }
	},
	{cs_DTCTYP_GEOCTR,
		{dtcTypGeoCtr,           dtcTypNone,             dtcTypNone,          dtcTypNone  },
		{dtcTypGeoCtrInv,        dtcTypNone,             dtcTypNone,          dtcTypNone  }
	},
	{cs_DTCTYP_4PARM,
		{dtcTypFourParm,         dtcTypNone,             dtcTypNone,          dtcTypNone  },
		{dtcTypFourParmInv,      dtcTypNone,             dtcTypNone,          dtcTypNone  }
	},
	{cs_DTCTYP_RGF93,
		{dtcTypNtfToRgf93,       dtcTypNone,             dtcTypNone,          dtcTypNone  },
		{dtcTypRgf93ToNtf,       dtcTypNone,             dtcTypNone,          dtcTypNone  }
	},
	{ cs_DTCTYP_ETRF89,
		{dtcTypNone,             dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypNone,             dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_3PARM,
		{dtcTypThreeParm,        dtcTypNone,            dtcTypNone,          dtcTypNone  },
		{dtcTypThreeParmInv,     dtcTypNone,            dtcTypNone,          dtcTypNone  }
	},
	{cs_DTCTYP_CHENYX,
		{dtcTypCh1903ToPlus,     dtcTypChPlusToChtrs95, dtcTypChtrs95ToEtrf89, dtcTypEtrf89ToWgs84  },
		{dtcTypWgs84ToEtrf89,    dtcTypEtrf89ToChtrs95, dtcTypChtrs95ToChPlus, dtcTypPlusToCh1903   }
	},
};
********************************************************************************************/
bool csAdjustSrcAndTrg (std::wofstream& gpStrm,char *srcDatum,char *trgDatum,
															  const cs_Dtdef_* dtDefPtr,
															  const char* dtmPivot)
{
	bool ok (true);

	short index;

	char pathName [64];
	char xfrmName [64];

	cs_GeodeticPathElement_* xfrmPtr;

	cs_GeodeticPath_ gPath;

	// Preparations in case we need to write a path.  Avoids
	// lots of duplicate code.
	memset ((void*)&gPath,0,sizeof (gPath));
	gPath.protect = 0;
	gPath.reversible = 1;
	gPath.accuracy = cs_Zero;
	gPath.epsgCode = 0;
	gPath.variant = 0;
	gPath.elementCount = 0;
	sprintf (pathName,"%s_to_%s",dtDefPtr->key_nm,dtmPivot);
	CS_stncp (gPath.pathName,pathName,sizeof (gPath.pathName));
	CS_stncp (gPath.srcDatum,dtDefPtr->key_nm,sizeof (gPath.srcDatum));
	CS_stncp (gPath.trgDatum,dtmPivot,sizeof (gPath.trgDatum));
	CS_stncp (gPath.description,dtDefPtr->name,sizeof (gPath.description));
	CS_stncp (gPath.source,dtDefPtr->source,sizeof (gPath.source));
	if (dtDefPtr->group [0] != '\0')
	{
		CS_stncp (gPath.group,dtDefPtr->group,sizeof (gPath.group));
	}

	switch (dtDefPtr->to84_via) {
	case cs_DTCTYP_MOLO:
	case cs_DTCTYP_GEOCTR:
	case cs_DTCTYP_3PARM:
	case cs_DTCTYP_4PARM:
	case cs_DTCTYP_6PARM:
	case cs_DTCTYP_7PARM:
	case cs_DTCTYP_BURS:
	case cs_DTCTYP_WGS72:
	case cs_DTCTYP_MREG:

	case cs_DTCTYP_GDA94:
	case cs_DTCTYP_NZGD2K:
	case cs_DTCTYP_ETRF89:
	case cs_DTCTYP_NAD83:
	case cs_DTCTYP_WGS84:

		// This is the typical case.
		CS_stncp (srcDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);
		CS_stncp (trgDatum,dtmPivot,cs_KEYNM_DEF);
		// No path required for these.
		ok = true;
		break;

	case cs_DTCTYP_NAD27:
		// This transformation is actually from NAD27 to NAD83.
		CS_stncp (srcDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);
		CS_stncp (trgDatum,"NAD83",cs_KEYNM_DEF);
		// Need a path so that this datum can get converted to WGS84.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","NAD83",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_AGD66:
	case cs_DTCTYP_AGD84:
		// This transformation is actually from AGD66/AGD84 to GDA94.
		CS_stncp (srcDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);
		CS_stncp (trgDatum,"GDA94",cs_KEYNM_DEF);
		// Need a path so that this datum can get converted to WGS84.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","GDA94",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_NZGD49:
		// This transformation is actually from NZGD49 to NZGD2000.
		CS_stncp (srcDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);
		CS_stncp (trgDatum,"NZGD2000",cs_KEYNM_DEF);
		// Need a path so that this datum can get converted to WGS84.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","NZGD2000",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_ATS77:
		// Things start getting painful here.  This transformation is
		// actually from ATS77 to CSRS (i.e. NAD83/1998).  We then need
		// to use CSRS_to_NAD83 to get to NAD83, and then finally
		// NAD83-to_WGS84 to get to WGS84.  This is required to emulate
		// what currently happens.
		//
		// First we set up the transformation which we are now processing.
		CS_stncp (srcDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);
		CS_stncp (trgDatum,"CSRS",cs_KEYNM_DEF);

		// Need a path so that this datum can get converted to WGS84.
		// First element of the path gets us to CSRS.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		// Second element of the path gets us to NAD83
		sprintf (xfrmName,"%s_to_%s","NAD83","CSRS");
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 1;
		gPath.elementCount += 1;

		// Third element of the path gets us to WGS84.
		sprintf (xfrmName,"%s_to_%s","NAD83",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_TOKYO:
		// This transformation is actually from Tokyo to JGD2000.
		CS_stncp (srcDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);
		CS_stncp (trgDatum,"JGD2000",cs_KEYNM_DEF);
		// Need a path so that this datum can get converted to WGS84.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","JGD2000",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_RGF93:
		// This transformation is actually from RGF93 to NTF.
		CS_stncp (srcDatum,"RGF93",cs_KEYNM_DEF);
		CS_stncp (trgDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);

		// Need a path so that this datum can get converted to WGS84.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 1;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","RGF93",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_ED50:
		// This transformation is actually from ED50 to ETRF89.
		CS_stncp (srcDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);
		CS_stncp (trgDatum,"ETRF89",cs_KEYNM_DEF);

		// Need a path so that this datum can get converted to WGS84.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","ETRF89",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_DHDN:
		// This transformation is actually from DHDN to ETRF89.
		CS_stncp (srcDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);
		CS_stncp (trgDatum,"ETRF89",cs_KEYNM_DEF);

		// Need a path so that this datum can get converted to WGS84.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","ETRF89",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_CHENYX:
		// This transformation is actually from CH1903 to CH1903Plus.
		CS_stncp (srcDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);
		CS_stncp (trgDatum,"CH1903Plus_1",cs_KEYNM_DEF);

		// Need a path so that this datum can get converted to WGS84.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","CH1903Plus_1","CHTRF95");
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","CHTrF95","ETRF89");
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","ETRF89",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_HPGN:
		// This transformation is actually from NAD83 to HPGN/HARN.
		CS_stncp (srcDatum,"NAD83",cs_KEYNM_DEF);
		CS_stncp (trgDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);

		// Need a path so that this datum can get converted to WGS84.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 1;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","NAD83",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_CSRS:
		// This transformation is actually from NAD83 to CSRS.
		CS_stncp (srcDatum,"NAD83",cs_KEYNM_DEF);
		CS_stncp (trgDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);

		// Need a path so that this datum can get converted to WGS84.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 1;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","NAD83",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_NONE:
	default:
		ok = false;
		break;
	}
	return ok;
}
bool csWriteGeodeticPath (std::wofstream& gpStrm,const cs_GeodeticPath_ *gpPtr)
{
	bool ok;
	
	short count;
	short index;

	wchar_t* direction;
	const cs_GeodeticPathElement_* xfrmPtr;
	
	gpStrm << std::endl;
	gpStrm << L"GP_NAME: "          << gpPtr->pathName      << std::endl;
	if (gpPtr->group [0] != '\0')
	{
		gpStrm << L"\t    GROUP: "      << gpPtr->group         << std::endl;
	}
	gpStrm << L"\t  SRC_DTM: "      << gpPtr->srcDatum      << std::endl;
	gpStrm << L"\t  TRG_DTM: "      << gpPtr->trgDatum      << std::endl;
	gpStrm << L"\t  DESC_NM: "      << gpPtr->description   << std::endl;
	gpStrm << L"\t   SOURCE: "      << gpPtr->source        << std::endl;
	if (gpPtr->accuracy != 0.0)
	{
	gpStrm << L"\t ACCURACY: "      << gpPtr->accuracy      << std::endl;
	}
	if (gpPtr->epsgCode != 0)
	{
	gpStrm << L"\t     EPSG: "      << gpPtr->epsgCode      << std::endl;
	}
	count = gpPtr->elementCount;
	for (index = 0;index < count;index += 1)
	{
		xfrmPtr = &gpPtr->geodeticPathElements [index];
		direction = (xfrmPtr->direction == 0) ? L"FWD" : L"INV";
		gpStrm << L"\t    XFORM: "
		       << xfrmPtr->geodeticXformName
		       << L','
		       << direction
		       << std::endl;
	}

	ok = gpStrm.good ();
	return ok;
}

bool csConvertMrtFile (cs_GeodeticTransform_::csGeodeticXformParameters::csGeodeticXformParmsDmaMulReg_* mulrgParms,
																		 const cs_Dtdef_ *dtDefPtr,
																		 const TcsDefFile& mregAsc)
{
	bool ok (false);
	short idx, uuPwr, vvPwr;
	double coefficient;
	const TcsAscDefinition* ascDefPtr (0);

	// Set up the name of the file.  We rely on the host application to have
	// properly initialized CS-MAP with a successful call to CS_altdr ();
	CS_stncp (cs_DirP,dtDefPtr->key_nm,CSMAXPATH);
	CS_stncat (cs_DirP,".MRT",CSMAXPATH);
	TcsMrtFile mrtFile ((const char*)cs_Dir);
	ok = mrtFile.IsOk ();

	// Get the definition in ASCII form.  We need this as the test case data
	// is not actually written to the .MRT file.
	if (ok)
	{
		ascDefPtr = mregAsc.GetDefinition (dtDefPtr->key_nm);
		ok = (ascDefPtr != 0);
	}

	// Output the test values which are only available from the mregAsc file.
	if (ok)
	{
		mulrgParms->testPhi     = ascDefPtr->GetValueAsDouble ("TEST_PHI:",testPhiFormat); 
		mulrgParms->testLambda  = ascDefPtr->GetValueAsDouble ("TEST_LAMBDA:",testLambdaFormat); 
		mulrgParms->deltaPhi    = ascDefPtr->GetValueAsDouble ("DELTA_PHI:",deltaPhiFormat); 
		mulrgParms->deltaLambda = ascDefPtr->GetValueAsDouble ("DELTA_LAMBDA:",deltaLambdaFormat); 
		mulrgParms->deltaHeight = ascDefPtr->GetValueAsDouble ("DELTA_HEIGHT:",deltaHeightFormat); 
		ok = (mulrgParms->testPhi     > TcsAscDefinition::InvalidDouble) &&
			 (mulrgParms->testLambda  > TcsAscDefinition::InvalidDouble) &&
			 (mulrgParms->deltaPhi    > TcsAscDefinition::InvalidDouble) &&
			 (mulrgParms->deltaLambda > TcsAscDefinition::InvalidDouble) &&
			 (mulrgParms->deltaHeight > TcsAscDefinition::InvalidDouble);
	}

	// OK, now for the stuff which does reside in the MRT file.  The mrtFile
	// functions simply return doubles and (in the case of coefficients) a
	// hard zero to indicate that the coefficient does not exist.  Thus, there
	// are no error conditions to check for.
	if (ok)
	{
		mulrgParms->phiOffset          = mrtFile.GetPhiOffset ();
		mulrgParms->lambdaOffset       = mrtFile.GetLambdaOffset ();
		mulrgParms->normalizationScale = mrtFile.GetNormalizingScale ();
		mulrgParms->validation         = 1.401;

		// Add the coefficients.
		for (uuPwr = 0;uuPwr < 10;uuPwr += 1)
		{
			for (vvPwr = 0;vvPwr < 10;vvPwr += 1)
			{
				idx = uuPwr * 10 + vvPwr;
				coefficient = mrtFile.GetLambdaCoeff (uuPwr,vvPwr);
				if (fabs (coefficient) > 1.0E-12)		// != 0.0
				{
					mulrgParms->coeffLambda [idx] = coefficient;
				}
				coefficient = mrtFile.GetPhiCoeff (uuPwr,vvPwr);
				if (fabs (coefficient) > 1.0E-12)		// != 0.0
				{
					mulrgParms->coeffPhi [idx] = coefficient;
				}
				coefficient = mrtFile.GetHgtCoeff (uuPwr,vvPwr);
				if (fabs (coefficient) > 1.0E-12)		// != 0.0
				{
					mulrgParms->coeffHeight [idx] = coefficient;
				}
			}
		}
	}
	return ok;
}
bool csExtractMrtRange (double lng [2],double lat [2],const cs_Dtdef_ *dtDefPtr)
{
	bool ok (false);

	// Set up the name of the file.  We rely on the host application to have
	// properly initialized CS-MAP with a successful call to CS_altdr ();
	CS_stncp (cs_DirP,dtDefPtr->key_nm,CSMAXPATH);
	CS_stncat (cs_DirP,".MRT",CSMAXPATH);
	TcsMrtFile mrtFile ((const char*)cs_Dir);
	ok = mrtFile.IsOk ();
	if (!ok)
	{
		return ok;
	}

	double kk = mrtFile.GetNormalizingScale ();
	double lambdaOff = mrtFile.GetLambdaOffset ();
	double phiOff = mrtFile.GetPhiOffset ();

	lng [0] = (-1.0 / kk) - lambdaOff;
	lng [1] = ( 1.0 / kk) - lambdaOff;
	lat [0] = (-1.0 / kk) - phiOff;
	lat [1] = ( 1.0 / kk) - phiOff;

	return ok;
}

bool csNewHpgnDatums (std::wofstream& gtStrm,std::wofstream& gpStrm,const wchar_t* csDictDir,
																	const wchar_t* csDictTrgDir)
{
	bool ok (false);

	char pathDatumsAsc [MAXPATH];
	char pathCoordsysAsc [MAXPATH];
	char pathCategoryAsc [MAXPATH];

	// Open up the datums.asc file.
	wcstombs (pathDatumsAsc,csDictDir,sizeof (pathDatumsAsc));
	strcat (pathDatumsAsc,"\\datums.asc");
	TcsDefFile datumsAsc (dictTypDatum,pathDatumsAsc);

	// Open up the coordsys.asc file.
	wcstombs (pathCoordsysAsc,csDictDir,sizeof (pathCoordsysAsc));
	strcat (pathCoordsysAsc,"\\coordsys.asc");
	TcsDefFile coordsysAsc (dictTypCoordsys,pathCoordsysAsc);

	// Open up the category.asc file.
	wcstombs (pathCategoryAsc,csDictDir,sizeof (pathCategoryAsc));
	strcat (pathCategoryAsc,"\\category.asc");
	std::ifstream catStrm (pathCategoryAsc,std::ios_base::in);
	TcsCategoryFile categoryAsc;
	ok = catStrm.is_open ();
	if (ok)
	{
		ok = categoryAsc.ReadFromStream (catStrm);
		catStrm.close();
	}

	if (ok)
	{
		// Phase One --> Create new datum definitions.
		ok = csNewHpgnDatumsPhaseOne (datumsAsc);
	}
	if (ok)
	{
		// Phase Two --> Create new geodetic transformations.
		ok = csNewHpgnDatumsPhaseTwo (gtStrm);
	}
	if (ok)
	{
		// Phase Three --> Create new Projective Coordinate system definitions.
		ok = csNewHpgnDatumsPhaseThree (coordsysAsc);
	}
	if (ok)
	{
		// Phase Four --> Create new Geographic Coordinate system definitions.
		ok = csNewHpgnDatumsPhaseFour (coordsysAsc);
	}
	if (ok)
	{
		// Phase Five --> Update the Category file.
		ok = csNewHpgnDatumsPhaseFive (categoryAsc,coordsysAsc);
	}

	// If everything is still OK:
	if (ok)
	{
		// Write and close the category file to the provided target directory.
		wcstombs (pathCategoryAsc,csDictTrgDir,sizeof (pathCategoryAsc));
		strcat (pathCategoryAsc,"\\category.asc");
		std::ofstream outStrm (pathCategoryAsc,std::ios_base::out | std::ios_base::trunc);
		ok = outStrm.is_open ();
		if (ok)
		{
			ok = categoryAsc.WriteToStream (outStrm);
			outStrm.close ();
		}
	}
	if (ok)
	{
		// Write and close the coordsys file to the provided target directory.
		wcstombs (pathCoordsysAsc,csDictTrgDir,sizeof (pathCoordsysAsc));
		strcat (pathCoordsysAsc,"\\coordsys.asc");
		std::ofstream outStrm (pathCoordsysAsc,std::ios_base::out | std::ios_base::trunc);
		ok = outStrm.is_open ();
		if (ok)
		{
			ok = coordsysAsc.WriteToStream (outStrm);
			outStrm.close ();
		}
	}
	if (ok)
	{
		// Write and close the datums file to the provided target directory.
		wcstombs (pathDatumsAsc,csDictTrgDir,sizeof (pathDatumsAsc));
		strcat (pathDatumsAsc,"\\datums.asc");
		std::ofstream outStrm (pathDatumsAsc,std::ios_base::out | std::ios_base::trunc);
		ok = outStrm.is_open ();
		if (ok)
		{
			ok = datumsAsc.WriteToStream (outStrm);
			outStrm.close ();
		}
	}
	return ok;
};
// Phase One -->  Create new definitions in the existing Datums.asc file as necessary.
bool csNewHpgnDatumsPhaseOne (TcsDefFile& datumsAsc)
{
	extern struct csHarnDatumTable_ csHarnDatumTable [];

	bool ok (true);
	bool utm (false);
	const char* wrkPtr;
	struct csHarnDatumTable_* tblPtr;
	char workBuffer [256];

	// For each entry in the HPGN table,
	for (tblPtr = csHarnDatumTable;ok && tblPtr->datumCode [0] != '\0';tblPtr += 1)
	{
		TcsAscDefinition* defPtr;
		TcsAscDefinition workDef (dictTypDatum);

		// Is this a UTM zone or a SPCS zone.
		utm = (CS_stricmp (tblPtr->file1,"UTM") == 0);

		// Fetch a copy of the existing HPGN datum definition.  We'll eventually
		// deprecate it, but will use if for now as a base for creating new HARN
		// datum definitions.
		defPtr  = datumsAsc.GetDefinition ("HPGN");
		workDef = *defPtr;

		// Give our new definition a name along the lines of HARN/CO
		sprintf (tblPtr->datumName,"HARN/%s",tblPtr->datumCode);
		ok = workDef.RenameDef (tblPtr->datumName);

		//	* the appropriate ellipsoid (GRS1980)
		if (ok)
		{
			ok = workDef.SetValue ("ELLIPSOID:","GRS1980");
		}

		//  * Description
		if (ok)
		{
			wrkPtr = (utm) ? tblPtr->file2  : tblPtr->datumCode;
			sprintf (workBuffer,"High Accuracy Regional (%s) Network (aka HPGN, NAD83/91)",wrkPtr);
			ok = workDef.SetValue ("DESC_NM:",workBuffer);
		}

		//  * EPSG code is 6152
		if (ok && tblPtr->epsgCode != 0)
		{
			sprintf (workBuffer,"%d",6152);
			ok = workDef.SetValue ("EPSG:",workBuffer);
		}

		//  * A null transformation.
		if (ok)
		{
			ok = workDef.SetValue ("USE:","GEOCENTRIC");
			ok = workDef.SetValue ("DELTA_X:","0.0");
			ok = workDef.SetValue ("DELTA_Y:","0.0");
			ok = workDef.SetValue ("DELTA_Z:","0.0");
		}

		// Insert the new definition definition immediately before the existing HARN entry.
		if (ok)
		{
			ok = datumsAsc.Replace (workDef);
			if (!ok)
			{
				ok = datumsAsc.InsertBefore ("HPGN",workDef);
			}
		}
	}
	
	// Deprecate the existing HPGN datum.
	if (ok)
	{
		ok = datumsAsc.DeprecateDef ("HPGN","Replaced by 45 distinct regional HARN datums.",0);
	}
	return ok;
}
// Phase Two -->  Create new Geodetic Transformation definitions as necessary.
bool csNewHpgnDatumsPhaseTwo (std::wofstream& gtStrm)
{
	extern struct csHarnDatumTable_ csHarnDatumTable [];

	bool ok (true);
	bool utm;
	size_t stIdx;
	const char *stPtr;
	struct csHarnDatumTable_* tblPtr;
	csGeodeticXfromParmsFile_* fileParmPtr;
	char workBuffer [256];

	// For each entry in the HPGN table, for which a new datum name shall now
	// be present:
	for (tblPtr = csHarnDatumTable;ok && tblPtr->datumCode [0] != '\0';tblPtr += 1)
	{
		utm = (CS_stricmp (tblPtr->file1,"UTM") == 0);

		//	* Create a Geodetic Transformation to convert from 'NAD83' to this new datum.
		cs_GeodeticTransform_ xform;
		memset ((void*)&xform,'\0',sizeof (xform));
		sprintf (xform.xfrmName,"NAD83_to_%s",tblPtr->datumName);
		CS_stncp (xform.srcDatum,"NAD83",sizeof (xform.srcDatum));		
		CS_stncp (xform.trgDatum,tblPtr->datumName,sizeof (xform.trgDatum));
		CS_stncp (xform.group,"HARN",sizeof (xform.group));
		stPtr = (utm) ? tblPtr->file2 : tblPtr->datumCode;
		sprintf (workBuffer,"High Accuracy Regional Network (aka HPGN) for region %s",stPtr);
		CS_stncp (xform.description,workBuffer,sizeof (xform.description));
		CS_stncp (xform.source,"Derived by Mentor Software from US NGS NADCON 2.10",sizeof (xform.source));
		xform.epsgCode = static_cast<short>(tblPtr->epsgCode); 
		xform.inverseSupported = TRUE;
		xform.maxIterations = 10;
		xform.cnvrgValue    = 1.0E-09;
		xform.errorValue    = 5.0E-08;
		xform.rangeMinLng   = 0.0;
		xform.rangeMaxLng   = 0.0;
		xform.rangeMinLat   = 0.0;
		xform.rangeMaxLat   = 0.0;
		//  * Method is Grid File Interpolation
		xform.methodCode = cs_DTCMTH_GFILE;
		xform.parameters.fileParameters.fileReferenceCount = 0;			// redundant, to keep lint happy.
		//  * File(s) are NADCON Format, FWD, and use the ./Usa/Harn/??hpgn.l?s
		if (utm)
		{
			// This is a little hoeky, and after thought after the original design.
			for (stIdx = 0;stIdx < 14;stIdx += 1)
			{
				switch (stIdx) {
				case  0: stPtr = tblPtr->state1;  break;
				case  1: stPtr = tblPtr->state2;  break;
				case  2: stPtr = tblPtr->state3;  break;
				case  3: stPtr = tblPtr->state4;  break;
				case  4: stPtr = tblPtr->state5;  break;
				case  5: stPtr = tblPtr->state6;  break;
				case  6: stPtr = tblPtr->state7;  break;
				case  7: stPtr = tblPtr->state8;  break;
				case  8: stPtr = tblPtr->state9;  break;
				case  9: stPtr = tblPtr->state10; break;
				case 10: stPtr = tblPtr->state11; break;
				case 11: stPtr = tblPtr->state12; break;
				case 12: stPtr = tblPtr->state13; break;
				case 13: stPtr = tblPtr->state14; break;
				default:    ok = false;           break;
				}
				
				if (*stPtr != '\0')
				{
					fileParmPtr = &xform.parameters.fileParameters.fileNames [xform.parameters.fileParameters.fileReferenceCount];
					fileParmPtr->fileFormat = cs_DTCFRMT_NADCN;
					fileParmPtr->direction = cs_DTCDIR_FWD;
					sprintf (workBuffer,"./Usa/Harn/%shpgn.l?s",stPtr);
					CS_stncp (fileParmPtr->fileName,workBuffer,sizeof (fileParmPtr->fileName));
					xform.parameters.fileParameters.fileReferenceCount += 1;
				}
			}
		}
		else
		{
			fileParmPtr = &xform.parameters.fileParameters.fileNames [xform.parameters.fileParameters.fileReferenceCount];
			fileParmPtr->fileFormat = cs_DTCFRMT_NADCN;
			fileParmPtr->direction = cs_DTCDIR_FWD;
			sprintf (workBuffer,"./Usa/Harn/%shpgn.l?s",tblPtr->file1);
			CS_stncp (fileParmPtr->fileName,workBuffer,sizeof (fileParmPtr->fileName));
			xform.parameters.fileParameters.fileReferenceCount += 1;
			if (tblPtr->file2[0] != '\0')
			{
				fileParmPtr = &xform.parameters.fileParameters.fileNames [xform.parameters.fileParameters.fileReferenceCount];
				fileParmPtr->fileFormat = cs_DTCFRMT_NADCN;
				fileParmPtr->direction = cs_DTCDIR_FWD;
				sprintf (workBuffer,"./Usa/Harn/%shpgn.l?s",tblPtr->file2);
				CS_stncp (fileParmPtr->fileName,workBuffer,sizeof (fileParmPtr->fileName));
				xform.parameters.fileParameters.fileReferenceCount += 1;
			}
		}
		//	* No fall back.
		xform.parameters.fileParameters.fallback [0] = '\0';
		//  * Each new Geodetic Transformation definition is simply written to the provided stream.
		gtStrm << std::endl;
		ok = csXformToStream (gtStrm,&xform);
		//	* The name given to each new transformation is recorded in the HPGN table.
		CS_stncp (tblPtr->xfrmName,xform.xfrmName,sizeof (tblPtr->xfrmName));
	}
	return ok;
}
bool csNewHpgnDatumsPhaseThree (TcsDefFile& coordsysAsc)
{
	extern csHarnCrsMapTable_ csHarnCrsMapTable [];

	bool ok (true);
	bool fixed;

	const char *wrkPtr;	
	const csHarnCrsMapTable_* tblPtr;
	TcsAscDefinition* defPtr;

	char subBufr [128];
	char wrkBufr [256];
	char description [256];

	for (tblPtr = csHarnCrsMapTable;ok && tblPtr->oldName [0] != '\0';tblPtr += 1)
	{
		// Here once for each CRS system which needs to have the HARN fix
		// applied.  The HARN fix consists of:
		//	1> Make a copy of the existing definition.
		//	2> Append the copy to the end of the coordsys ASC file and
		//	   then deprecate this copy.
		//  3> Change the name of the existing copy to the new name
		//  4> change the datum reference of the existing definition
		//	5> possibly change the description.
		defPtr = coordsysAsc.GetDefinition (tblPtr->oldName);
		if (defPtr == 0)
		{
			ok = false;
			continue;
		}
		TcsAscDefinition copiedDef (*defPtr);

		defPtr->RenameDef (tblPtr->newName);
		defPtr->SetValue ("DT_NAME:",tblPtr->datumName);
		wrkPtr = defPtr->GetValue ("DESC_NM:");
		if (wrkPtr != 0)
		{
			sprintf (subBufr,"%s",tblPtr->datumName);
			CS_stncp (wrkBufr,wrkPtr,sizeof (wrkBufr));
			fixed = CS_strrpl (wrkBufr,sizeof (wrkBufr),"HARN (HPGN)",subBufr);
			if (fixed)
			{
				defPtr->SetValue ("DESC_NM:",wrkBufr);
			}
			else
			{
				sprintf (subBufr,"%s UTM",tblPtr->datumName);
				CS_stncp (wrkBufr,wrkPtr,sizeof (wrkBufr));
				fixed = CS_strrpl (wrkBufr,sizeof (wrkBufr),"UTM with HPGN datum",subBufr);
				if (fixed)
				{
					defPtr->SetValue ("DESC_NM:",wrkBufr);
				}
			}
		}

		wrkPtr = defPtr->GetValue ("SOURCE:");
		if (wrkPtr != 0)
		{
			sprintf (subBufr,"datum is set to %s",tblPtr->datumName);
			CS_stncp (wrkBufr,wrkPtr,sizeof (wrkBufr));
			fixed = CS_strrpl (wrkBufr,sizeof (wrkBufr),"datum is set to HPGN",subBufr);
			if (fixed)
			{
				defPtr->SetValue ("SOURCE:",wrkBufr);
			}
		}

		// Deprecate the old definition.
		ok = coordsysAsc.Append (copiedDef);
		if (ok)
		{
			sprintf (description,"Replaced by %s; referenced to %s",
								 tblPtr->newName,tblPtr->datumName);
			ok = coordsysAsc.DeprecateDef (tblPtr->oldName,description,0);
		}
	}
	return ok;
}
bool csNewHpgnDatumsPhaseFour (TcsDefFile& coordsysAsc)
{
	extern struct csHarnDatumTable_ csHarnDatumTable [];

	bool ok (true);
	struct csHarnDatumTable_* tblPtr;
	TcsAscDefinition* ascDefPtr;
	
	char workBuffer [256];

	// We need to create several coordinate systems of the geographic type.
	// To accomplish this, we get a copy of the current definition of LL-HPGN.
	ascDefPtr = coordsysAsc.GetDefinition ("LL-HPGN");
	TcsAscDefinition baseAscDef (*ascDefPtr);

	// For each entry in the HPGN table, for which a new datum name shall now
	// be present:
	for (tblPtr = csHarnDatumTable;ok && tblPtr->datumCode [0] != '\0';tblPtr += 1)
	{
		TcsAscDefinition nextDef (baseAscDef);
		sprintf (workBuffer,"%s.LL",tblPtr->datumName);
		nextDef.RenameDef (workBuffer);
		sprintf (workBuffer,"HARN (aka HPGN) geographic system for region %s",tblPtr->datumCode);
		nextDef.SetValue ("DESC_NM:",workBuffer);
		nextDef.SetValue ("DT_NAME:",tblPtr->datumName);
		ok = coordsysAsc.Replace (nextDef);
		if (!ok)
		{
			ok = coordsysAsc.InsertBefore ("LL-HPGN",nextDef);
		}
	}
	coordsysAsc.DeprecateDef ("LL-HPGN","Replaced with 42 region specific LL systems/datums.","");
	coordsysAsc.MakeLast ("LL-HPGN");
	return true;
}
bool csNewHpgnDatumsPhaseFive (TcsCategoryFile& categoryAsc,TcsDefFile& coordsysAsc)
{
	bool ok (false);

	size_t itmIdx;
	size_t itmCnt;
	size_t catIdx;
	size_t catCnt;
	
	const char* descPtr (0);
	const char* itmName (0);
	const TcsAscDefinition* csDefPtr (0);

	TcsCategoryItem* itmPtr (0);
	TcsCategory* catPtr (0);
	csHarnDatumTable_* harnDtTblPtr;
	
	char nameBufr [260];
	char descBufr [260];

	catCnt = categoryAsc.GetCategoryCount ();
	for (catIdx = 0;catIdx < catCnt;catIdx += 1)
	{
		catPtr = categoryAsc.FetchCategory (catIdx);
		ok = (catPtr != 0);
		if (ok && !CS_stricmp (catPtr->GetCategoryName (),"Obsolete Coordinate Systems"))
		{
			// This is the Obsolete category.
			const csHarnCrsMapTable_* tblPtr;
			for (tblPtr = csHarnCrsMapTable;tblPtr->oldName [0] != '\0';tblPtr += 1)
			{
				csDefPtr = coordsysAsc.GetDefinition (tblPtr->oldName);
				ok = (csDefPtr != 0);
				if (ok)
				{
					descPtr = csDefPtr->GetValue ("DESC_NM:");
					ok = (descPtr != 0);
				}
				if (ok)
				{
					TcsCategoryItem nextItem (tblPtr->oldName,descPtr);
					catPtr->AddItem (nextItem);
				}					
			}
			itmPtr = catPtr->GetItem ("LL-HPGN");
			if (itmPtr == 0)
			{
				CS_stncp(nameBufr,"LL-HPGN",sizeof (nameBufr));
				csDefPtr = coordsysAsc.GetDefinition (nameBufr);
				ok = (csDefPtr != 0);
				if (ok)
				{
					descPtr = csDefPtr->GetValue ("DESC_NM:");
					ok = (descPtr != 0);
					if (ok)
					{
						CS_stncp (descBufr,descPtr,sizeof (descBufr));
						TcsCategoryItem nextItem (nameBufr,descBufr);
						catPtr->AddItem (nextItem);
					}
				}
			}
			
		}
		else if (ok && !CS_stricmp (catPtr->GetCategoryName (),"Lat Longs"))
		{
			itmPtr = catPtr->GetItem ("LL-HPGN");
			if (itmPtr != 0)
			{
				itmPtr->SetToBeDeleted (true);
			}
			for (harnDtTblPtr = csHarnDatumTable;harnDtTblPtr->datumCode [0] != '\0';harnDtTblPtr += 1)
			{
				sprintf (nameBufr,"%s.LL",harnDtTblPtr->datumName);
				csDefPtr = coordsysAsc.GetDefinition (nameBufr);
				ok = (csDefPtr != 0);
				if (ok)
				{
					descPtr = csDefPtr->GetValue ("DESC_NM:");
					CS_stncp (descBufr,descPtr,sizeof (descBufr));
					TcsCategoryItem nextItem (nameBufr,descBufr);
					catPtr->AddItem (nextItem);
				}
			}
		}
		else
		{
			// This is not the obsolete category AND not the Lat Longs category.
			itmCnt = catPtr->GetItemCount ();
			for (itmIdx = 0;itmIdx < itmCnt;itmIdx += 1)
			{
				itmPtr = catPtr->GetItem (itmIdx);
				ok = (itmPtr != 0);
				if (ok)
				{
					itmName = itmPtr->GetItemName ();
					// Now we need to search the name map table.
					const csHarnCrsMapTable_* tblPtr;
					for (tblPtr = csHarnCrsMapTable;tblPtr->oldName [0] != '\0';tblPtr += 1)
					{
						if (!CS_stricmp (tblPtr->oldName,itmName))
						{
							itmPtr->SetItemName (tblPtr->newName);
							csDefPtr = coordsysAsc.GetDefinition (tblPtr->newName);
							ok = (csDefPtr != 0);
							if (ok)
							{
								descPtr = csDefPtr->GetValue ("DESC_NM:");
								itmPtr->SetDescription (descPtr);
							}
						}
					}
				}
			}
		}
	}
	return ok;
}
