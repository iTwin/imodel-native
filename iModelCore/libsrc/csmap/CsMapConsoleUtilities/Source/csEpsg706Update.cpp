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

/*
	The following table was produced manually with the help of csapisample1.
	
	Essentially, it provides a list of the new and old names of the datums
	that have changed, and also the new and old names of all the coordinate
	systems which reference those datums.

	For each datum in the list, being careful to note that each datum will
	appear more than once:

	1>  Find the old datum name in datum dictionary.
	2>  Duplicate the datum definition at the end of the dictionary.
	3>  Deprecate the old datum, adding an appropriate description includiing "...replaced by <new name>".
	4>  Change the name on the orginal defnition.
	5>  Change the EPSG code value in the new definition.
	5>  Change the technique and parameter values as indicated in the table.
	6>  Locate the old name in the Datum Key Name ampping .csv file.
	7>  Replace the old name with the new name.
	8>  Locate the original definition in the Geodetic Transformation dictionary.
	9>  Duplicate this definition at the end of the file.
	10> Deprecate this copy, adding an appropriate description: replaced by <new name>
	11> CHange the name of the original definition.
	12> Change the source datum name of the original definition.
	13> Change the technique and parameter values of the new definition.
	14> Change the EPSG operation variant and operation code values.

	This module assumes that the datum definitions have been changed manually
	and all this module needs to do is, for each coordinate system in the list:

	1>  Locate the definition in the coordinate system dictionary.
	2>  Duplicate the coordinate system definition at the end of the dictionary.
	3>  Deprecate the duplicated definition.
	4>  Add an appropriate description which indicates the system is deprecated and
	    replaced by --> the new name.
	5>  Change the name of the original definition to the provided new name.
	6>  Change the the name of the datum reference in the orginal definition.
	7>  Locate the original definition in the category dictionary.
	8>  Add a copy of that definition to the Obsolete category.
	9>  Change the copy description to "Deprecated, replaced by <new Name>"
	10> Change the name of the original category entry to the new name.
	11> Locate the original name in the projected key name .csv file.
	12> Change that name to the new name.

*/

struct TcsEpsg706DtmUpdateTbl
{
	char OldDtName [32];
	char NewDtName [32];
	unsigned long NewEpsgDtmCode;
	unsigned long NewEpsgVariant;
	unsigned long NewEpsgOperationCode;
	char NewTechnique [32];
	double NewDeltaX;
	double NewDeltaY;
	double NewDeltaZ;
	double NewRotX;
	double NewRotY;
	double NewRotZ;
	double NewBwScale;
}
	KcsEpsg706DtmUpdateTbl [] =
{
/*                                                         New     New      New
                                                           Epsg    Epsg     Epsg     New           New          New          New         New        New        New        New
     OldDtName                   NewDtName                 Code    Variant  OprCode  Technique     DeltaX       DeltaY       DeltaZ      RotX       RotY       RotZ       BwScale */
//{ "123456789012345678902123", "12345678901234567890123", 3276UL, 3276UL,  3276UL, "7PARAMETER",  999.999999,  999.999999,  999.999999, 59.999999, 59.999999, 59.999999,  -1.234567 },
  { "Kartasto66",               "Kartasto66a",             6123UL,    2UL, 10099UL, "7PARAMETER",  -96.062000,  -82.428000, -121.753000,  4.801000,  0.345000, -1.376000,   1.496000 },
  { "TETE",                     "TETE/a",                  6127UL,    1UL,  1683UL, "7PARAMETER", -115.064000,  -87.390000, -101.716000, -0.058000,  4.001000, -2.062000,   9.366000 },
  { "Estonia92",                "Estonia92a",              6133UL,    1UL,  1333UL, "7PARAMETER",    0.055000,   -0.541000,   -0.185000,  0.018300, -0.000300, -0.007000,  -0.014000 },
  { "PDOSurvey93",              "PDOSurvey93a",            6134UL,    1UL,  1439UL, "7PARAMETER", -180.624000, -225.516000,  173.919000, -0.810000, -1.898000,  8.336000,  16.710060 },
  { "Czech/JTSK",               "CzechJTSK/5",             6156UL,    5UL,  5239UL, "7PARAMETER",  572.213000,   85.334000,  461.940000,  4.973200,  1.529000,  5.248400,   3.537800 },
  { "Pulkovo42/83",             "Pulkovo42/83a",           6178UL,    1UL,  1675UL, "7PARAMETER",   24.000000, -123.000000,  -94.000000,  0.020000, -0.250000, -0.130000,   1.100000 },
  { "Pulkovo42/58",             "Pulkovo42/58a",           6179UL,    1UL,  1645UL, "7PARAMETER",   33.400000, -146.600000,  -76.300000, -0.359000, -0.053000,  0.844000,  -0.840000 },
  { "Luxembourg30",             "Luxembourg30a",           6181UL,    1UL,  1643UL, "7PARAMETER", -193.000000,   13.700000,  -39.300000, -0.410000, -2.933000,  2.688000,   0.430000 },
  { "Scoresbysund52",           "Scoresbysund52a",         6195UL,    1UL,  1799UL, "7PARAMETER",  105.000000,  326.000000, -102.500000,  0.000000,  0.000000,  0.814000,  -0.600000 },
  { "Ammassalik58",             "Ammassalik58a",           6196UL,    1UL,  1800UL, "7PARAMETER",  -45.000000,  417.000000,   -3.500000,  0.000000,  0.000000,  0.814000,  -0.600000 },
  { "Lisbon37",                 "Lisbon37a",               6207UL,    4UL,  1988UL, "7PARAMETER", -288.885000,  -91.744000,  126.244000, -1.691000,  0.410000, -0.211000,  -4.598000 },
  { "Beijing1954",              "Beijing1954/a",           6214UL,    3UL, 15920UL, "BURSA",        31.400000, -144.300000,  -74.800000,  0.000000,  0.000000,  0.814000,  -0.380000 },
  { "DeirEzZor_1",              "DeirEzZor_2",             6227UL,    2UL, 15741UL, "GEOCENTRIC", -187.500000,   14.100000,  237.600000,  0.000000,  0.000000,  0.000000,   0.000000 },
  { "HD72-7P-CORR",             "HD72/7P",                 6237UL,    3UL,  1448UL, "7PARAMETER",   52.684000,  -71.194000,  -13.975000, -0.312000, -0.106300, -0.372900,   1.019100 },
  { "HitoXVIII63",              "HitoXVIII63a",            6254UL,    1UL,  1529UL, "7PARAMETER",   18.380000,  192.450000,   96.820000,  0.056000, -0.142000, -0.200000,  -0.001300 },
  { "NGO48",                    "NGO48a",                  6273UL,    1UL,  1654UL, "7PARAMETER",  278.300000,   93.000000,  474.500000,  7.889000,  0.050000, -6.610000,   6.210000 },
  { "Datum73-MOD",              "Datum73-Mod/a",           6274UL,    4UL,  1987UL, "7PARAMETER", -239.749000,   88.181000,   30.488000,  0.263000,  0.082000,  1.211000,   2.229000 },
  { "Amersfoort",               "Amersfoort/a",            6289UL,    2UL,  1672UL, "BURSA",       565.040000,   49.910000,  465.840000, -0.409394,  0.359705, -1.868491,   4.077200 },	// Rotations in EPSG are micro-radians!!!
  { "TM1965",                   "TM1965/a",                6299UL,    1UL,  1641UL, "7PARAMETER",  482.500000, -130.600000,  564.600000, -1.042000, -0.214000, -0.631000,   8.150000 },
  { "Belge72",                  "Belge72a",                6313UL,    1UL,  1609UL, "7PARAMETER",  -99.059000,   53.322000, -112.486000,  0.419000, -0.830000,  1.885000,  -1.000000 },
  { "DHDN",                     "DHDN/2",                  6314UL,    2UL,  1777UL, "7PARAMETER",  598.100000,   73.700000,  418.200000,  0.202000,  0.045000, -2.455000,   6.700000 },
  { "RT90-3-7P",                "RT90-3/7P",               6124UL,    2UL,  1896UL, "7PARAMETER",  414.100000,   41.300000,  603.100000, -0.855000,  2.141000, -7.023000,   0.000000 },
  { "WGS72-TBE",                "WGS72-TBE/a",             6324UL,    1UL,  1240UL, "7PARAMETER",    0.000000,    0.000000,    1.900000,  0.000000,  0.000000,  0.814000,  -0.380000 },
  { "HongKong80",               "HongKong80a",             6611UL,    1UL,  1825UL, "7PARAMETER", -162.619000, -276.959000, -161.764000,  0.067753, -2.243649, -1.158827,  -1.094246 },
  { "QatarNtl95",               "QatarNtl95a",             6614UL,    1UL,  1840UL, "7PARAMETER", -119.424800, -303.658720,  -11.000610,  1.164298,  0.174458,  1.096259,   3.657065 },
  { "Guyane95",                 "Guyane95a",               6624UL,    2UL,  4840UL, "GEOCENTRIC",    0.000000,    0.000000,    0.000000,  0.000000,  0.000000,  0.000000,   0.000000 },
  { "MOP1978",                  "MOP1978a",                6639UL,    2UL, 15847UL, "GEOCENTRIC",  253.000000, -132.000000, -127.000000,  0.000000,  0.000000,  0.000000,   0.000000 },
  { "ST84IleDesPins",           "ST84IleDesPins/a",        6642UL,    2UL, 15848UL, "GEOCENTRIC",  -13.000000, -348.000000,  292.000000,  0.000000,  0.000000,  0.000000,   0.000000 },
  { "ST71Belep",                "ST71Belep/a",             6643UL,    1UL,  1931UL, "7PARAMETER", -480.260000, -438.320000, -643.429000, 16.311900, 20.172100, -4.034900,-111.700200 },
  { "NEA74Noumea",              "Noumea74",                6644UL,    2UL, 15904UL, "GEOCENTRIC",  -10.180000, -350.430000,  291.370000,  0.000000,  0.000000,  0.000000,   0.000000 },
  { "",                         "",                           0UL,    0UL,    0UL,  "",              0.000000,    0.000000,    0.000000,  0.000000,  0.000000,  0.000000,   0.000000 }
};

struct TcsEpsg706CrsUpdateTbl
{
	char OldDtmName [32];
	char NewDtmName [32];
	char OldCrsName [32];
	char NewCrsName [32];
	unsigned long OldEpsgCode;
	unsigned long NewEpsgCode;
	bool geographic;
	char NewCrsDescription [128];
}
	KcsEpsg706CrsUpdateTbl [] =
{
/*                                                                                                                   Old     New       
                                                                                                                     Epsg    Epsg    Is   
     OldDtName                   NewDtName                   OldCsName                   NewCsName                   Code    Code    Geog    New CRS Description */
//{ "123456789012345678902123", "12345678901234567890123",  "12345678901234567890123",  "12345678901234567890123",   3276UL, 3276UL, false, "123456789012345678901234567890"     },
  { "Kartasto66",               "Kartasto66a",              "KKJ.Finland-1",            "KKJa.Finland-1",               0UL,    0UL, false, ""                                   }, 
  { "Kartasto66",               "Kartasto66a",              "KKJ.Finland-2",            "KKJa.Finland-2",               0UL,    0UL, false, ""                                   }, 
  { "Kartasto66",               "Kartasto66a",              "KKJ.Finland-4",            "KKJa.Finland-4",               0UL,    0UL, false, ""                                   }, 
  { "Kartasto66",               "Kartasto66a",              "KKJ.Finland-UCS",          "KKJa.Finland-UCS",             0UL,    0UL, false, ""                                   }, 
  { "Kartasto66",               "Kartasto66a",              "KKJ.LL",                   "KKJa.LL",                      0UL,    0UL, true,  ""                                   }, 
  { "TETE",                     "TETE/a",                   "MOZ-T-E",                  "TETE/a.MOZ-T-E",               0UL,    0UL, false, ""                                   }, 
  { "TETE",                     "TETE/a",                   "MOZ-T-W",                  "TETE/a.MOZ-T-W",               0UL,    0UL, false, ""                                   }, 
  { "TETE",                     "TETE/a",                   "Tete.LL",                  "Tete/a.LL",                    0UL,    0UL, true,  ""                                   }, 
  { "TETE",                     "TETE/a",                   "Tete.UTM-36S",             "Tete/a.UTM-36S",               0UL,    0UL, false, ""                                   }, 
  { "TETE",                     "TETE/a",                   "Tete.UTM-37S",             "Tete/a.UTM-37S",               0UL,    0UL, false, ""                                   }, 
  { "Estonia92",                "Estonia92a",               "Estonia92",                "Estonia92a.LL",                0UL,    0UL, true,  ""                                   }, 
  { "Estonia92",                "Estonia92a",               "Estonia92.Estonia",        "Estonia92a.Estonia",           0UL,    0UL, false, ""                                   }, 
  { "PDOSurvey93",              "PDOSurvey93a",             "PDOSurvey93.LL",           "PDOSurvey93a.LL",              0UL,    0UL, true,  ""                                   }, 
  { "PDOSurvey93",              "PDOSurvey93a",             "PSD93.UTM-39N",            "PSD93a.UTM-39N",               0UL,    0UL, false, ""                                   }, 
  { "PDOSurvey93",              "PDOSurvey93a",             "PSD93.UTM-40N",            "PSD93a.UTM-40N",               0UL,    0UL, false, ""                                   }, 
  { "Czech/JTSK",               "CzechJTSK/5",              "Czech/JTSK.Krovak",        "CzechJTSK/5.Krovak",           0UL,    0UL, false, ""                                   }, 
  { "Czech/JTSK",               "CzechJTSK/5",              "Czech/JTSK.LL",            "CzechJTSK/5.LL",               0UL,    0UL, true,  ""                                   }, 
  { "Pulkovo42/83",             "Pulkovo42/83a",            "Pulkovo/83.GK-3",          "Pulkovo42/83a.GK-3",           0UL,    0UL, false, ""                                   }, 
  { "Pulkovo42/83",             "Pulkovo42/83a",            "Pulkovo/83.GK-4",          "Pulkovo42/83a.GK-4",           0UL,    0UL, false, ""                                   }, 
  { "Pulkovo42/83",             "Pulkovo42/83a",            "Pulkovo/83.GK-5",          "Pulkovo42/83a.GK-5",           0UL,    0UL, false, ""                                   }, 
  { "Pulkovo42/83",             "Pulkovo42/83a",            "Pulkovo42/83.LL",          "Pulkovo42/83a.LL",             0UL,    0UL, true,  ""                                   }, 
//{ "Pulkovo42/58",             "Pulkovo42/58a",            "Pulkovo/58.Poland-I",      "Pulkovo/58a.Poland-I",         0UL,    0UL, false, ""                                   },  Already deprecated, shouldn't be in this list.
//{ "Pulkovo42/58",             "Pulkovo42/58a",            "Pulkovo/58.Poland-I/a",    "Pulkovo/58a.Poland-I/a",       0UL,    0UL, false, ""                                   },  Already deprecated, shouldn't be in this list.
  { "Pulkovo42/58",             "Pulkovo42/58a",            "Pulkovo/58.Poland-II",     "Pulkovo/58a.Poland-II",        0UL,    0UL, false, ""                                   }, 
  { "Pulkovo42/58",             "Pulkovo42/58a",            "Pulkovo/58.Poland-III",    "Pulkovo/58a.Poland-III",       0UL,    0UL, false, ""                                   }, 
  { "Pulkovo42/58",             "Pulkovo42/58a",            "Pulkovo/58.Poland-IV",     "Pulkovo/58a.Poland-IV",        0UL,    0UL, false, ""                                   }, 
  { "Pulkovo42/58",             "Pulkovo42/58a",            "Pulkovo/58.Poland-V",      "Pulkovo/58a.Poland-V",         0UL,    0UL, false, ""                                   }, 
  { "Pulkovo42/58",             "Pulkovo42/58a",            "Pulkovo42/58.LL",          "Pulkovo42/58a.LL",             0UL,    0UL, true,  ""                                   }, 
  { "Pulkovo42/58",             "Pulkovo42/58a",            "Pulkovo42/58.Poland-1",    "Pulkovo42/58a.Poland-1",       0UL,    0UL, false, ""                                   }, 
  { "Pulkovo42/58",             "Pulkovo42/58a",            "Pulkovo42/58.Stereo70",    "Pulkovo42/58a.Stereo70",       0UL,    0UL, false, ""                                   }, 
  { "Luxembourg30",             "Luxembourg30a",            "Luxembourg30.Gauss",       "Luxembourg30a.Gauss",          0UL,    0UL, false, ""                                   }, 
  { "Luxembourg30",             "Luxembourg30a",            "Luxembourg30.LL",          "Luxembourg30a.LL",             0UL,    0UL, true,  ""                                   }, 
  { "Scoresbysund52",           "Scoresbysund52a",          "Scoresbysund52.LL",        "Scoresbysund52a.LL",           0UL,    0UL, true,  ""                                   }, 
  { "Ammassalik58",             "Ammassalik58a",            "Ammassalik58.LL",          "Ammassalik58a.LL",             0UL,    0UL, true,  ""                                   }, 
  { "Lisbon37",                 "Lisbon37a",                "Lisbon37.LL",              "Lisbon37a.LL",                 0UL,    0UL, true,  ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-13",        "Beijing1954/a.GK-13",          0UL,    0UL, false, ""                                   }, 
//{ "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-13N",       "Beijing1954/a.GK-13N",         0UL,    0UL, false, ""                                   },   Already deprecated, shouldn't be in this list.
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-14",        "Beijing1954/a.GK-14",          0UL,    0UL, false, ""                                   }, 
//{ "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-14N",       "Beijing1954/a.GK-14N",         0UL,    0UL, false, ""                                   },   Already deprecated, shouldn't be in this list.
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-15",        "Beijing1954/a.GK-15",          0UL,    0UL, false, ""                                   }, 
//{ "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-15N",       "Beijing1954/a.GK-15N",         0UL,    0UL, false, ""                                   },   Already deprecated, shouldn't be in this list.
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-16",        "Beijing1954/a.GK-16",          0UL,    0UL, false, ""                                   }, 
//{ "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-16N",       "Beijing1954/a.GK-16N",         0UL,    0UL, false, ""                                   },   Already deprecated, shouldn't be in this list.
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-17",        "Beijing1954/a.GK-17",          0UL,    0UL, false, ""                                   }, 
//{ "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-17N",       "Beijing1954/a.GK-17N",         0UL,    0UL, false, ""                                   },   Already deprecated, shouldn't be in this list.
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-18",        "Beijing1954/a.GK-18",          0UL,    0UL, false, ""                                   }, 
//{ "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-18N",       "Beijing1954/a.GK-18N",         0UL,    0UL, false, ""                                   },   Already deprecated, shouldn't be in this list.
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-19",        "Beijing1954/a.GK-19",          0UL,    0UL, false, ""                                   }, 
//{ "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-19N",       "Beijing1954/a.GK-19N",         0UL,    0UL, false, ""                                   },   Already deprecated, shouldn't be in this list.
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-20",        "Beijing1954/a.GK-20",          0UL,    0UL, false, ""                                   }, 
//{ "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-20N",       "Beijing1954/a.GK-20N",         0UL,    0UL, false, ""                                   },   Already deprecated, shouldn't be in this list.
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-21",        "Beijing1954/a.GK-21",          0UL,    0UL, false, ""                                   }, 
//{ "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-21N",       "Beijing1954/a.GK-21N",         0UL,    0UL, false, ""                                   },   Already deprecated, shouldn't be in this list.
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-22",        "Beijing1954/a.GK-22",          0UL,    0UL, false, ""                                   }, 
//{ "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-22N",       "Beijing1954/a.GK-22N",         0UL,    0UL, false, ""                                   },   Already deprecated, shouldn't be in this list.
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-23",        "Beijing1954/a.GK-23",          0UL,    0UL, false, ""                                   }, 
//{ "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK-23N",       "Beijing1954/a.GK-23N",         0UL,    0UL, false, ""                                   },   Already deprecated, shouldn't be in this list.
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK/CM-105E",   "Bjing54/a.GK/CM-105E",         0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK/CM-111E",   "Bjing54/a.GK/CM-111E",         0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK/CM-117E",   "Bjing54/a.GK/CM-117E",         0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK/CM-123E",   "Bjing54/a.GK/CM-123E",         0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK/CM-129E",   "Bjing54/a.GK/CM-129E",         0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK/CM-135E",   "Bjing54/a.GK/CM-135E",         0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK/CM-75E",    "Beijing1954/a.GK/CM-75E",      0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK/CM-81E",    "Beijing1954/a.GK/CM-81E",      0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK/CM-87E",    "Beijing1954/a.GK/CM-87E",      0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK/CM-93E",    "Beijing1954/a.GK/CM-93E",      0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK/CM-99E",    "Beijing1954/a.GK/CM-99E",      0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d-25",      "Beijing1954/a.GK3d-25",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d-26",      "Beijing1954/a.GK3d-26",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d-27",      "Beijing1954/a.GK3d-27",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d-28",      "Beijing1954/a.GK3d-28",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d-29",      "Beijing1954/a.GK3d-29",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d-30",      "Beijing1954/a.GK3d-30",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d-31",      "Beijing1954/a.GK3d-31",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d-32",      "Beijing1954/a.GK3d-32",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d-33",      "Beijing1954/a.GK3d-33",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d-34",      "Beijing1954/a.GK3d-34",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d-35",      "Beijing1954/a.GK3d-35",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d-36",      "Beijing1954/a.GK3d-36",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d-37",      "Beijing1954/a.GK3d-37",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d-38",      "Beijing1954/a.GK3d-38",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d-39",      "Beijing1954/a.GK3d-39",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d-40",      "Beijing1954/a.GK3d-40",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d-41",      "Beijing1954/a.GK3d-41",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d-42",      "Beijing1954/a.GK3d-42",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d-43",      "Beijing1954/a.GK3d-43",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d-44",      "Beijing1954/a.GK3d-44",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d-45",      "Beijing1954/a.GK3d-45",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d/CM-75E",  "Bjing54/a.GK3d/CM-75E",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d/CM-78E",  "Bjing54/a.GK3d/CM-78E",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d/CM-81E",  "Bjing54/a.GK3d/CM-81E",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d/CM-84E",  "Bjing54/a.GK3d/CM-84E",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d/CM-87E",  "Bjing54/a.GK3d/CM-87E",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d/CM-90E",  "Bjing54/a.GK3d/CM-90E",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d/CM-93E",  "Bjing54/a.GK3d/CM-93E",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d/CM-96E",  "Bjing54/a.GK3d/CM-96E",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.GK3d/CM-99E",  "Bjing54/a.GK3d/CM-99E",        0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Beijing1954.LL",           "Beijing1954/a.LL",             0UL,    0UL, true,  ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Bjing54.GK3d/CM-102E",     "Bjing54/a.GK3d/CM-102E",       0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Bjing54.GK3d/CM-105E",     "Bjing54/a.GK3d/CM-105E",       0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Bjing54.GK3d/CM-108E",     "Bjing54/a.GK3d/CM-108E",       0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Bjing54.GK3d/CM-111E",     "Bjing54/a.GK3d/CM-111E",       0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Bjing54.GK3d/CM-114E",     "Bjing54/a.GK3d/CM-114E",       0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Bjing54.GK3d/CM-117E",     "Bjing54/a.GK3d/CM-117E",       0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Bjing54.GK3d/CM-120E",     "Bjing54/a.GK3d/CM-120E",       0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Bjing54.GK3d/CM-123E",     "Bjing54/a.GK3d/CM-123E",       0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Bjing54.GK3d/CM-126E",     "Bjing54/a.GK3d/CM-126E",       0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Bjing54.GK3d/CM-129E",     "Bjing54/a.GK3d/CM-129E",       0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Bjing54.GK3d/CM-132E",     "Bjing54/a.GK3d/CM-132E",       0UL,    0UL, false, ""                                   }, 
  { "Beijing1954",              "Beijing1954/a",            "Bjing54.GK3d/CM-135E",     "Bjing54/a.GK3d/CM-135E",       0UL,    0UL, false, ""                                   }, 
  { "DeirEzZor_1",              "DeirEzZor_2",              "DeirEzZor_1.Levant",       "DeirEzZor_2.Levant",           0UL,    0UL, false, ""                                   }, 
  { "DeirEzZor_1",              "DeirEzZor_2",              "DeirEzZor_1.LL",           "DeirEzZor_2.LL",               0UL,    0UL, true,  ""                                   }, 
  { "DeirEzZor_1",              "DeirEzZor_2",              "DeirEzZor_1.Syria",        "DeirEzZor_2.Syria",            0UL,    0UL, false, ""                                   }, 
  { "HD72-7P-CORR",             "HD72/7P",                  "HD-72.LL",                 "HD72/7P.LL",                   0UL,    0UL, true,  ""                                   }, 
  { "HD72-7P-CORR",             "HD72/7P",                  "HUN-EOV72-7P",             "HD72/7P.EOV",                  0UL,    0UL, false, ""                                   }, 
  { "HitoXVIII63",              "HitoXVIII63a",             "Hito63.Argentina-2",       "Hito63a.Argentina-2",          0UL,    0UL, false, ""                                   }, 
  { "HitoXVIII63",              "HitoXVIII63a",             "Hito63.UTM-19S",           "Hito63a.UTM-19S",              0UL,    0UL, false, ""                                   }, 
  { "HitoXVIII63",              "HitoXVIII63a",             "HitoXVIII63.LL",           "HitoXVIII63a.LL",              0UL,    0UL, true,  ""                                   }, 
  { "NGO48",                    "NGO48a",                   "NGO48.LL",                 "NGO48a.LL",                    0UL,    0UL, true,  ""                                   }, 
  { "Datum73-MOD",              "Datum73-Mod/a",            "Datum73.LL",               "Datum73a.LL",                  0UL,    0UL, true,  ""                                   }, 
  { "Datum73-MOD",              "Datum73-Mod/a",            "Datum73.ModPortgGrd",      "Datum73a.ModPortgGrd",         0UL,    0UL, false, ""                                   }, 
  { "Datum73-MOD",              "Datum73-Mod/a",            "Datum73.UTM-29N",          "Datum73a.UTM-29N",             0UL,    0UL, false, ""                                   }, 
  { "Amersfoort",               "Amersfoort/a",             "Amersfoort.LL",            "Amersfoort/a.LL",              0UL,    0UL, true,  ""                                   }, 
  { "TM1965",                   "TM1965/a",                 "TM1965.IrishGrid",         "TM1965/a.IrishGrid",           0UL,    0UL, false, ""                                   }, 
  { "TM1965",                   "TM1965/a",                 "TM1965.LL",                "TM1965/a.LL",                  0UL,    0UL, true,  ""                                   }, 
//{ "TM1965",                   "TM1965/a",                 "TM65g.IrishNationalGrid",  "TM65g/a.IrishNtlGrid",         0UL,    0UL, false, ""                                   },   Already deprecated, shouldn't be in this list.
  { "Belge72",                  "Belge72a",                 "Belge72.Lambert72",        "Belge72a.Lambert72",           0UL,    0UL, false, ""                                   }, 
  { "Belge72",                  "Belge72a",                 "Belge72.Lambert72A",       "Belge72a.Lambert72A",          0UL,    0UL, false, ""                                   }, 
  { "Belge72",                  "Belge72a",                 "Belge72.LL",               "Belge72a.LL",                  0UL,    0UL, true,  ""                                   }, 
  { "DHDN",                     "DHDN/2",                   "DHDN.Berlin/Cassini",      "DHDN/2.Berlin/Cassini",        0UL,    0UL, false, ""                                   }, 
  { "DHDN",                     "DHDN/2",                   "DHDN.Gauss3d-1",           "DHDN/2.Gauss3d-1",             0UL,    0UL, false, ""                                   }, 
  { "DHDN",                     "DHDN/2",                   "DHDN.Gauss3d-2",           "DHDN/2.Gauss3d-2",             0UL,    0UL, false, ""                                   }, 
  { "DHDN",                     "DHDN/2",                   "DHDN.Gauss3d-3",           "DHDN/2.Gauss3d-3",             0UL,    0UL, false, ""                                   }, 
  { "DHDN",                     "DHDN/2",                   "DHDN.Gauss3d-4",           "DHDN/2.Gauss3d-4",             0UL,    0UL, false, ""                                   }, 
  { "DHDN",                     "DHDN/2",                   "DHDN.Gauss3d-5",           "DHDN/2.Gauss3d-5",             0UL,    0UL, false, ""                                   }, 
  { "DHDN",                     "DHDN/2",                   "DHDN.LL",                  "DHDN/2.LL",                    0UL,    0UL, true,  ""                                   }, 
  { "RT90-3-7P",                "RT90-3/7P",                "SW-75GONV",                "RT90-3/7P.SW-75GONV",          0UL,    0UL, false, ""                                   }, 
  { "RT90-3-7P",                "RT90-3/7P",                "SW-5GONV",                 "RT90-3/7P.SW-5GONV",           0UL,    0UL, false, ""                                   }, 
  { "RT90-3-7P",                "RT90-3/7P",                "SW-25GONV",                "RT90-3/7P.SW-25GONV",          0UL,    0UL, false, ""                                   }, 
  { "RT90-3-7P",                "RT90-3/7P",                "SW-0GONV",                 "RT90-3/7P.SW-0GONV",           0UL,    0UL, false, ""                                   }, 
  { "RT90-3-7P",                "RT90-3/7P",                "SW-25GONO",                "RT90-3/7P.SW-25GONO",          0UL,    0UL, false, ""                                   }, 
  { "RT90-3-7P",                "RT90-3/7P",                "SW-5GONO",                 "RT90-3/7P.SW-5GONO",           0UL,    0UL, false, ""                                   }, 
  { "RT90-3-7P",                "RT90-3/7P",                "SW-NAT90",                 "RT90-3/7P.SW-NAT90",           0UL,    0UL, false, ""                                   }, 
  { "RT90-3-7P",                "RT90-3/7P",                "LL-RT90-7P",               "RT90-3/7P.LL",                 0UL,    0UL, true,  ""                                   }, 
  { "RT90-3-7P",                "RT90-3/7P",                "RT90-7.5O",                "RT90-3/7P.SW-7.5O",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72-TBE.LL",             "WGS72-TBE/a.LL",               0UL,    0UL, true, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.TM-106NE",         "WGS72be/a.TM-106NE",           0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-10N",          "WGS72be/a.UTM-10N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-10S",          "WGS72be/a.UTM-10S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-11N",          "WGS72be/a.UTM-11N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-11S",          "WGS72be/a.UTM-11S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-12N",          "WGS72be/a.UTM-12N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-12S",          "WGS72be/a.UTM-12S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-13N",          "WGS72be/a.UTM-13N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-13S",          "WGS72be/a.UTM-13S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-14N",          "WGS72be/a.UTM-14N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-14S",          "WGS72be/a.UTM-14S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-15N",          "WGS72be/a.UTM-15N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-15S",          "WGS72be/a.UTM-15S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-16N",          "WGS72be/a.UTM-16N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-16S",          "WGS72be/a.UTM-16S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-17N",          "WGS72be/a.UTM-17N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-17S",          "WGS72be/a.UTM-17S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-18N",          "WGS72be/a.UTM-18N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-18S",          "WGS72be/a.UTM-18S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-19N",          "WGS72be/a.UTM-19N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-19S",          "WGS72be/a.UTM-19S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-1N",           "WGS72be/a.UTM-1N",             0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-1S",           "WGS72be/a.UTM-1S",             0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-20N",          "WGS72be/a.UTM-20N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-20S",          "WGS72be/a.UTM-20S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-21N",          "WGS72be/a.UTM-21N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-21S",          "WGS72be/a.UTM-21S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-22N",          "WGS72be/a.UTM-22N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-22S",          "WGS72be/a.UTM-22S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-23N",          "WGS72be/a.UTM-23N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-23S",          "WGS72be/a.UTM-23S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-24N",          "WGS72be/a.UTM-24N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-24S",          "WGS72be/a.UTM-24S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-25N",          "WGS72be/a.UTM-25N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-25S",          "WGS72be/a.UTM-25S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-26N",          "WGS72be/a.UTM-26N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-26S",          "WGS72be/a.UTM-26S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-27N",          "WGS72be/a.UTM-27N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-27S",          "WGS72be/a.UTM-27S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-28N",          "WGS72be/a.UTM-28N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-28S",          "WGS72be/a.UTM-28S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-29N",          "WGS72be/a.UTM-29N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-29S",          "WGS72be/a.UTM-29S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-2N",           "WGS72be/a.UTM-2N",             0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-2S",           "WGS72be/a.UTM-2S",             0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-30N",          "WGS72be/a.UTM-30N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-30S",          "WGS72be/a.UTM-30S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-31N",          "WGS72be/a.UTM-31N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-31S",          "WGS72be/a.UTM-31S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-32N",          "WGS72be/a.UTM-32N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-32S",          "WGS72be/a.UTM-32S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-33N",          "WGS72be/a.UTM-33N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-33S",          "WGS72be/a.UTM-33S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-34N",          "WGS72be/a.UTM-34N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-34S",          "WGS72be/a.UTM-34S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-35N",          "WGS72be/a.UTM-35N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-35S",          "WGS72be/a.UTM-35S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-36N",          "WGS72be/a.UTM-36N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-36S",          "WGS72be/a.UTM-36S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-37N",          "WGS72be/a.UTM-37N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-37S",          "WGS72be/a.UTM-37S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-38N",          "WGS72be/a.UTM-38N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-38S",          "WGS72be/a.UTM-38S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-39N",          "WGS72be/a.UTM-39N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-39S",          "WGS72be/a.UTM-39S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-3N",           "WGS72be/a.UTM-3N",             0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-3S",           "WGS72be/a.UTM-3S",             0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-40N",          "WGS72be/a.UTM-40N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-40S",          "WGS72be/a.UTM-40S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-41N",          "WGS72be/a.UTM-41N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-41S",          "WGS72be/a.UTM-41S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-42N",          "WGS72be/a.UTM-42N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-42S",          "WGS72be/a.UTM-42S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-43N",          "WGS72be/a.UTM-43N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-43S",          "WGS72be/a.UTM-43S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-44N",          "WGS72be/a.UTM-44N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-44S",          "WGS72be/a.UTM-44S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-45N",          "WGS72be/a.UTM-45N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-45S",          "WGS72be/a.UTM-45S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-46N",          "WGS72be/a.UTM-46N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-46S",          "WGS72be/a.UTM-46S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-47N",          "WGS72be/a.UTM-47N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-47S",          "WGS72be/a.UTM-47S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-48N",          "WGS72be/a.UTM-48N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-48S",          "WGS72be/a.UTM-48S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-49N",          "WGS72be/a.UTM-49N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-49S",          "WGS72be/a.UTM-49S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-4N",           "WGS72be/a.UTM-4N",             0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-4S",           "WGS72be/a.UTM-4S",             0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-50N",          "WGS72be/a.UTM-50N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-50S",          "WGS72be/a.UTM-50S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-51N",          "WGS72be/a.UTM-51N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-51S",          "WGS72be/a.UTM-51S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-52N",          "WGS72be/a.UTM-52N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-52S",          "WGS72be/a.UTM-52S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-53N",          "WGS72be/a.UTM-53N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-53S",          "WGS72be/a.UTM-53S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-54N",          "WGS72be/a.UTM-54N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-54S",          "WGS72be/a.UTM-54S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-55N",          "WGS72be/a.UTM-55N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-55S",          "WGS72be/a.UTM-55S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-56N",          "WGS72be/a.UTM-56N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-56S",          "WGS72be/a.UTM-56S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-57N",          "WGS72be/a.UTM-57N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-57S",          "WGS72be/a.UTM-57S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-58N",          "WGS72be/a.UTM-58N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-58S",          "WGS72be/a.UTM-58S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-59N",          "WGS72be/a.UTM-59N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-59S",          "WGS72be/a.UTM-59S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-5N",           "WGS72be/a.UTM-5N",             0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-5S",           "WGS72be/a.UTM-5S",             0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-60N",          "WGS72be/a.UTM-60N",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-60S",          "WGS72be/a.UTM-60S",            0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-6N",           "WGS72be/a.UTM-6N",             0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-6S",           "WGS72be/a.UTM-6S",             0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-7N",           "WGS72be/a.UTM-7N",             0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-7S",           "WGS72be/a.UTM-7S",             0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-8N",           "WGS72be/a.UTM-8N",             0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-8S",           "WGS72be/a.UTM-8S",             0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-9N",           "WGS72be/a.UTM-9N",             0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "WGS72-TBE/a",              "WGS72be.UTM-9S",           "WGS72be/a.UTM-9S",             0UL,    0UL, false, ""                                   }, 
  { "WGS72-TBE",                "HongKong80a",              "HongKong80.GridSystem",    "HongKong80a.GridSystem",       0UL,    0UL, false, ""                                   }, 
  { "HongKong80",               "HongKong80a",              "HongKong80.LL",            "HongKong80a.LL",               0UL,    0UL, true,  ""                                   }, 
  { "HongKong80",               "QatarNtl95a",              "QatarNtl95.LL",            "QatarNtl95a.LL",               0UL,    0UL, true,  ""                                   }, 
  { "QatarNtl95",               "QatarNtl95a",              "QND95.QatarNational",      "QND95a.QatarNational",         0UL,    0UL, false, ""                                   }, 
  { "QatarNtl95",               "Guyane95a",                "Guyane95.LL",              "Guyane95a.LL",                 0UL,    0UL, true,  ""                                   }, 
  { "Guyane95",                 "Guyane95a",                "RGFG95.UTM-22N",           "RGFG95a.UTM-22N",              0UL,    0UL, false, ""                                   }, 
  { "Guyane95",                 "MOP1978a",                 "MOP1978.LL",               "MOP1978a.LL",                  0UL,    0UL, true,  ""                                   }, 
  { "MOP1978",                  "MOP1978a",                 "MOP1978.UTM-1S",           "MOP1978a.UTM-1S",              0UL,    0UL, false, ""                                   }, 
  { "MOP1978",                  "ST84IleDesPins/a",         "ST84IleDesPins.LL",        "ST84IlePins/a.LL",             0UL,    0UL, true,  ""                                   }, 
  { "ST84IleDesPins",           "ST84IleDesPins/a",         "ST84IledesPins.UTM-58S",   "ST84IlePins/a.UTM-58S",        0UL,    0UL, false, ""                                   }, 
  { "ST84IleDesPins",           "ST71Belep/a",              "ST71Belep.LL",             "ST71Belep/a.LL",               0UL,    0UL, true,  ""                                   }, 
  { "ST71Belep",                "ST71Belep/a",              "ST71Belep.UTM-58S",        "ST71Belep/a.UTM-58S",          0UL,    0UL, false, ""                                   }, 
  { "ST71Belep",                "Noumea74",                 "NEA74Noumea.LL",           "Noumea74.LL",                  0UL,    0UL, true,  ""                                   }, 
  { "NEA74Noumea",              "Noumea74",                 "NEA74Noumea.Noumea",       "Noumea74.Noumea",              0UL,    0UL, false, ""                                   }, 
  { "NEA74Noumea",              "Noumea74",                 "NEA74Noumea.Noumea-2",     "Noumea74.Noumea-2",            0UL,    0UL, false, ""                                   }, 
  { "NEA74Noumea",              "Noumea74",                 "NEA74Noumea.UTM-58S",      "Noumea74.UTM-58S",             0UL,    0UL, false, ""                                   }, 
  {"",                          "",                         "",                         "",                             0UL,    0UL, false, ""                                   }
};

struct TcsEpsg706DtmReMapTbl
{
	char OldDtmName [32];
	char NewDtmName [32];
}
	KcsEpsg706DtmReMapTbl [] =
{
	{ "NAPARIMA",                 "NAPARIMA-O"              },
	{ "SAPPER",                   "SAPPER-97"               },
	{ "Timbalai",                 "TMBLI-B"                 },
	{ "",                         ""                        }
};

bool Epsg706UpdateDatums (const wchar_t* srcDictDir,const wchar_t* dataDir,const wchar_t* trgDictDir);
bool Epsg706UpdateXforms (const wchar_t* srcDictDir,const wchar_t* dataDir,const wchar_t* trgDictDir);
bool Epsg706UpdatePaths (const wchar_t* srcDictDir,const wchar_t* dataDir,const wchar_t* trgDictDir);
bool Epsg706UpdateCrs (const wchar_t* srcDictDir,const wchar_t* dataDir,const wchar_t* trgDictDir);

bool Epsg706Updates (const wchar_t* srcDictDir,const wchar_t* dataDir,const wchar_t* trgDictDir)
{
	bool ok (true);

	if (ok)
	{
		ok = Epsg706UpdateDatums (srcDictDir,dataDir,trgDictDir);
	}
	if (ok)
	{
		ok = Epsg706UpdateXforms (srcDictDir,dataDir,trgDictDir);
	}
	if (ok)
	{
		ok = Epsg706UpdatePaths (srcDictDir,dataDir,trgDictDir);
	}
	if (ok)
	{
		ok = Epsg706UpdateCrs (srcDictDir,dataDir,trgDictDir);
	}
	return ok;
}

bool Epsg706UpdateDatums (const wchar_t* srcDictDir,const wchar_t* dataDir,const wchar_t* trgDictDir)
{
	bool ok;
	unsigned recordNbr;

	TcsDefLine* linePtr;
	TcsAscDefinition* ascDefPtr;
	TcsEpsg706DtmUpdateTbl* tblPtr;

	char chrBufr [256];
	char nullString [2] = {'\0','\0'};
	char pathName [1024];
	wchar_t wcBufr [256];
	
	TcsCsvStatus csvStatus;

	wcstombs (pathName,srcDictDir,sizeof (pathName));
	strcat (pathName,"\\Datums.asc");
	TcsDefFile datumDefs (dictTypDatum,pathName);

	short aDskFldNbr;
	unsigned invalidRecordNbr;
	TcsCsvFileBase datumNameMap (true,28,28);
	{
		std::wifstream wiStrm;
		wcstombs (pathName,dataDir,sizeof (pathName));
		strcat (pathName,"\\DatumKeyNameMap.csv");
		wiStrm.open (pathName,std::ios_base::in);
		ok = wiStrm.is_open ();
		if (ok)
		{
			ok = datumNameMap.ReadFromStream (wiStrm,true,csvStatus);
			wiStrm.close ();
		}
	}
	if (!ok) return ok;
	aDskFldNbr = datumNameMap.GetFldNbr (L"AdskName",csvStatus);
	invalidRecordNbr = datumNameMap.GetInvalidRecordNbr();

	// OK, we're ready to go.  We do eight steps for each datum in the table.
	ok = true;				// until we know different.
	for (tblPtr = KcsEpsg706DtmUpdateTbl;ok && tblPtr->OldDtName [0] != '\0';tblPtr += 1)
	{
		// Step 1: Locate the original definition.
		ascDefPtr = datumDefs.GetDefinition (tblPtr->OldDtName);
		if (ascDefPtr == 0)
		{
			ok = false;
			break;
		}

		// Step 2: Make a copy of it.
		TcsAscDefinition originalCopy (*ascDefPtr);
		
		// Step 6: Change the name of the original definition to the new name.
		ascDefPtr->RenameDef (tblPtr->NewDtName);

		// Step 4: Update the definition
		sprintf (chrBufr,"%lu",tblPtr->NewEpsgDtmCode);
		linePtr = originalCopy.GetLine ("EPSG:");
		if (linePtr == 0)
		{
			TcsDefLine newEpsgLine (dictTypDatum,"EPSG:",chrBufr,nullString);
			originalCopy.InsertAfter ("SOURCE:",newEpsgLine);
		}
		else
		{
			ascDefPtr->SetValue ("EPSG:",chrBufr);
		}
		ascDefPtr->SetValue ("USE:",tblPtr->NewTechnique);
		
		// Deal with the differences between 3 parameter and 7 parameter methods.
		if (!CS_stricmp (tblPtr->NewTechnique,"GEOCENTRIC") ||
			!CS_stricmp (tblPtr->NewTechnique,"MOLODENSKY")
		   )
		{
			// Delta X,Y,Z should wlways be there.
			sprintf (chrBufr,"%.3f",tblPtr->NewDeltaX);
			ascDefPtr->SetValue ("DELTA_X:",chrBufr);
			sprintf (chrBufr,"%.3f",tblPtr->NewDeltaY);
			ascDefPtr->SetValue ("DELTA_Y:",chrBufr);
			sprintf (chrBufr,"%.3f",tblPtr->NewDeltaZ);
			ascDefPtr->SetValue ("DELTA_Z:",chrBufr);
			sprintf (chrBufr,"%.5f",tblPtr->NewBwScale);
			
			// Remove the following if they happen to be there.
			ascDefPtr->RemoveLine ("BWSCALE:");
			ascDefPtr->RemoveLine ("ROT_X:");
			ascDefPtr->RemoveLine ("ROT_Y:");
			ascDefPtr->RemoveLine ("ROT_Z:");
		}
		else if (!CS_stricmp (tblPtr->NewTechnique,"BURSA") ||
				 !CS_stricmp (tblPtr->NewTechnique,"7PARAMETER")
				)
		{
			// Delta X,Y,Z should wlways be there.
			sprintf (chrBufr,"%.3f",tblPtr->NewDeltaX);
			ascDefPtr->SetValue ("DELTA_X:",chrBufr);
			sprintf (chrBufr,"%.3f",tblPtr->NewDeltaY);
			ascDefPtr->SetValue ("DELTA_Y:",chrBufr);
			sprintf (chrBufr,"%.3f",tblPtr->NewDeltaZ);
			ascDefPtr->SetValue ("DELTA_Z:",chrBufr);

			// The four extra parameters might not be there, yet.
			sprintf (chrBufr,"%.6f",tblPtr->NewBwScale);
			linePtr = ascDefPtr->GetLine ("BWSCALE:");
			if (linePtr != 0)
			{
				ascDefPtr->SetValue ("BWSCALE:",chrBufr);
			}
			else
			{
				TcsDefLine newGrpLine (dictTypDatum,"BWSCALE:",chrBufr,nullString);
				ascDefPtr->InsertAfter ("DELTA_Z:",newGrpLine);
			}

			sprintf (chrBufr,"%.6f",tblPtr->NewRotX);
			linePtr = ascDefPtr->GetLine ("ROT_X:");
			if (linePtr != 0)
			{
				ascDefPtr->SetValue ("ROT_X:",chrBufr);
			}
			else
			{
				TcsDefLine newGrpLine (dictTypDatum,"ROT_X:",chrBufr,nullString);
				ascDefPtr->InsertAfter ("BWSCALE:",newGrpLine);
			}

			sprintf (chrBufr,"%.6f",tblPtr->NewRotY);
			linePtr = ascDefPtr->GetLine ("ROT_Y:");
			if (linePtr != 0)
			{
				ascDefPtr->SetValue ("ROT_Y:",chrBufr);
			}
			else
			{
				TcsDefLine newGrpLine (dictTypDatum,"ROT_Y:",chrBufr,nullString);
				ascDefPtr->InsertAfter ("ROT_X:",newGrpLine);
			}

			sprintf (chrBufr,"%.6f",tblPtr->NewRotZ);
			linePtr = ascDefPtr->GetLine ("ROT_Z:");
			if (linePtr != 0)
			{
				ascDefPtr->SetValue ("ROT_Z:",chrBufr);
			}
			else
			{
				TcsDefLine newGrpLine (dictTypDatum,"ROT_Z:",chrBufr,nullString);
				ascDefPtr->InsertAfter ("ROT_Y:",newGrpLine);
			}
		}
		else
		{
			ok = false;
		}

		// Step 5: Deprecate the copy.  If there is no GROUP element, we need to
		// insert one.
		linePtr = originalCopy.GetLine ("GROUP:");
		if (linePtr == 0)
		{
			TcsDefLine newGrpLine (dictTypDatum,"GROUP:","LEGACY","");
			originalCopy.InsertAfter ("DESC_NM:",newGrpLine);
		}
		else
		{
			originalCopy.SetValue ("GROUP:","LEGACY");
		}
		sprintf (chrBufr,"Deprecated by EPSG synch. Replaced by %s.",tblPtr->NewDtName);
		originalCopy.SetValue ("DESC_NM:",chrBufr);

		// Step 6: Append the deprecated version to the end of the definition file.
		// Note that the original blanks lines, and comments are still there.
		// We may want to "StripLeadingComments" and then prepend a different
		// comment.  These methods would need to be added to the TcsAscDefinition
		// object.
		datumDefs.Append (originalCopy);

		// Step 7:  Locate the old name in the Datum Key Name Map file.
		mbstowcs (wcBufr,tblPtr->OldDtName,wcCount(wcBufr));
		ok = datumNameMap.Locate (recordNbr,aDskFldNbr,wcBufr);
		if (ok)
		{
			 ok = (recordNbr != invalidRecordNbr);
		}

		// Step 8:  Change the name to the new datum name.
		if (ok)
		{
			mbstowcs (wcBufr,tblPtr->NewDtName,wcCount(wcBufr));
			datumNameMap.ReplaceField (wcBufr,recordNbr,aDskFldNbr,csvStatus);
		}
		// That's it for this one.
	}
	
	if (ok)
	{
		bool epsgOk;
		const char* kCp; 
		char epsgValue [32];

		epsgOk = false;
		ascDefPtr = datumDefs.GetDefinition ("NAPARIMA");
		if (ascDefPtr != 0)
		{
			kCp = ascDefPtr->GetValue ("EPSG:");
			CS_stncp (epsgValue,kCp,sizeof (epsgValue));
			ascDefPtr->RemoveLine ("EPSG:");
			epsgOk = true;
		}
		if (epsgOk)
		{
			ascDefPtr = datumDefs.GetDefinition ("NAPARIMA-O");
			if (ascDefPtr != 0)
			{
				if (!ascDefPtr->SetValue ("EPSG:",epsgValue))
				{
					TcsDefLine newEpsgLine (dictTypDatum,"EPSG:",epsgValue,nullString);
					ascDefPtr->InsertAfter ("SOURCE:",newEpsgLine);
				}
			}
		}

		epsgOk = false;
		ascDefPtr = datumDefs.GetDefinition ("SAPPER");
		if (ascDefPtr != 0)
		{
			kCp = ascDefPtr->GetValue ("EPSG:");
			CS_stncp (epsgValue,kCp,sizeof (epsgValue));
			ascDefPtr->RemoveLine ("EPSG:");
			epsgOk = true;
		}
		if (epsgOk)
		{
			ascDefPtr = datumDefs.GetDefinition ("SAPPER-97");
			if (ascDefPtr != 0)
			{
				if (!ascDefPtr->SetValue ("EPSG:",epsgValue))
				{
					TcsDefLine newEpsgLine (dictTypDatum,"EPSG:",epsgValue,nullString);
					ascDefPtr->InsertAfter ("SOURCE:",newEpsgLine);
				}
			}
		}
	
		epsgOk = false;
		ascDefPtr = datumDefs.GetDefinition ("Timbalai");
		if (ascDefPtr != 0)
		{
			kCp = ascDefPtr->GetValue ("EPSG:");
			CS_stncp (epsgValue,kCp,sizeof (epsgValue));
			ascDefPtr->RemoveLine ("EPSG:");
			epsgOk = true;
		}
		if (epsgOk)
		{
			ascDefPtr = datumDefs.GetDefinition ("TMBLI-B");
			if (ascDefPtr != 0)
			{
				if (!ascDefPtr->SetValue ("EPSG:",epsgValue))
				{
					TcsDefLine newEpsgLine (dictTypDatum,"EPSG:",epsgValue,nullString);
					ascDefPtr->InsertAfter ("SOURCE:",newEpsgLine);
				}
			}
		}
	}

	if (ok)
	{
		bool dtmOk;
		TcsEpsg706DtmReMapTbl* dtmTblPtr;
		
		for (dtmTblPtr = KcsEpsg706DtmReMapTbl;ok && dtmTblPtr->OldDtmName [0] != '\0';dtmTblPtr += 1)
		{
			mbstowcs (wcBufr,dtmTblPtr->OldDtmName,wcCount(wcBufr));
			dtmOk = datumNameMap.Locate (recordNbr,aDskFldNbr,wcBufr);
			if (dtmOk && (recordNbr != invalidRecordNbr))
			{
				mbstowcs (wcBufr,dtmTblPtr->NewDtmName,wcCount(wcBufr));
				ok = datumNameMap.ReplaceField (wcBufr,recordNbr,aDskFldNbr,csvStatus);
			}
		}
	}

	// If we are still OK, we write the results out to the provided target directory.
	if (ok)
	{
		std::ofstream oStrm;
		wcstombs (pathName,trgDictDir,sizeof (pathName));
		strcat (pathName,"\\Datums.asc");
		oStrm.open (pathName,std::ios_base::out | std::ios_base::trunc);
		if (oStrm.is_open ())
		{
			ok = datumDefs.WriteToStream (oStrm);
			oStrm.close ();
		}

		if (ok)
		{
			std::wofstream woStrm;
			wcstombs (pathName,dataDir,sizeof (pathName));
			strcat (pathName,"\\DatumKeyNameMap.csv");
			woStrm.open (pathName,std::ios_base::out | std::ios_base::trunc);
			if (woStrm.is_open ())
			{
				datumNameMap.WriteToStream (woStrm,true,csvStatus);
				woStrm.close ();
			}
		}
	}
	return ok;
}
bool Epsg706UpdateXforms (const wchar_t* srcDictDir,const wchar_t* dataDir,const wchar_t* trgDictDir)
{
	bool ok;
	std::ofstream oStrm;

	TcsDefLine* linePtr;
	TcsAscDefinition* ascDefPtr;
	TcsEpsg706DtmUpdateTbl* tblPtr;

	char chrBufr [256];
	char oldXformName [256];
	char newXformName [256];
	char pathName [1024];
	char nullString [2] = {'\0','\0'};

	TcsCsvStatus status;

	wcstombs (pathName,srcDictDir,sizeof (pathName));
	strcat (pathName,"\\GeodeticTransformation.asc");
	TcsDefFile xformDefs (dictTypXform,pathName);

	// OK, we're ready to go.  We do six steps for each datum in the table.
	ok = true;				// until we know different.
	for (tblPtr = KcsEpsg706DtmUpdateTbl;ok && tblPtr->OldDtName [0] != '\0';tblPtr += 1)
	{
		// Step 1: Locate the original definition.
		sprintf (oldXformName,"%s_to_WGS84",tblPtr->OldDtName);
		ascDefPtr = xformDefs.GetDefinition (oldXformName);
		if (ascDefPtr == 0)
		{
			ok = false;
			break;
		}

		// Step 2: Make a copy of it.
		TcsAscDefinition originalCopy (*ascDefPtr);

		// Step 6: Change the name of the original definition to the new name.
		sprintf (newXformName,"%s_to_WGS84",tblPtr->NewDtName);
		ascDefPtr->RenameDef (newXformName);

		// Step 4: Update the definition
		ascDefPtr->SetValue ("SRC_DTM:",tblPtr->NewDtName);

		sprintf (chrBufr,"%lu",tblPtr->NewEpsgOperationCode);
		linePtr = originalCopy.GetLine ("EPSG_NBR:");
		if (linePtr == 0)
		{
			TcsDefLine newEpsgLine (dictTypDatum,"EPSG_NBR:",chrBufr,nullString);
			originalCopy.InsertAfter ("SOURCE:",newEpsgLine);
		}
		else
		{
			ascDefPtr->SetValue ("EPSG_NBR:",chrBufr);
		}

		sprintf (chrBufr,"%lu",tblPtr->NewEpsgVariant);
		linePtr = originalCopy.GetLine ("EPSG_VAR:");
		if (linePtr == 0)
		{
			TcsDefLine newEpsgLine (dictTypDatum,"EPSG_VAR:",chrBufr,nullString);
			originalCopy.InsertAfter ("SOURCE:",newEpsgLine);
		}
		else
		{
			ascDefPtr->SetValue ("EPSG_VAR:",chrBufr);
		}

		if (!CS_stricmp (tblPtr->NewTechnique,"BURSA"))
		{
			ascDefPtr->SetValue ("METHOD:","BURSAWOLF");
		}
		else
		{
			ascDefPtr->SetValue ("METHOD:",tblPtr->NewTechnique);
		}

		// Deal with the differences between 3 parameter and 7 parameter methods.
		if (!CS_stricmp (tblPtr->NewTechnique,"GEOCENTRIC") ||
			!CS_stricmp (tblPtr->NewTechnique,"MOLODENSKY")
		   )
		{
			// Delta X,Y,Z should wlways be there.
			sprintf (chrBufr,"%.3f",tblPtr->NewDeltaX);
			ascDefPtr->SetValue ("DELTA_X:",chrBufr);
			sprintf (chrBufr,"%.3f",tblPtr->NewDeltaY);
			ascDefPtr->SetValue ("DELTA_Y:",chrBufr);
			sprintf (chrBufr,"%.3f",tblPtr->NewDeltaZ);
			ascDefPtr->SetValue ("DELTA_Z:",chrBufr);
			sprintf (chrBufr,"%.5f",tblPtr->NewBwScale);
			
			// Remove the following if they happen to be there.
			ascDefPtr->RemoveLine ("BWSCALE:");
			ascDefPtr->RemoveLine ("ROT_X:");
			ascDefPtr->RemoveLine ("ROT_Y:");
			ascDefPtr->RemoveLine ("ROT_Z:");
		}
		else if (!CS_stricmp (tblPtr->NewTechnique,"BURSA") ||
				 !CS_stricmp (tblPtr->NewTechnique,"7PARAMETER")
				)
		{
			// Delta X,Y,Z should wlways be there.
			sprintf (chrBufr,"%.3f",tblPtr->NewDeltaX);
			ascDefPtr->SetValue ("DELTA_X:",chrBufr);
			sprintf (chrBufr,"%.3f",tblPtr->NewDeltaY);
			ascDefPtr->SetValue ("DELTA_Y:",chrBufr);
			sprintf (chrBufr,"%.3f",tblPtr->NewDeltaZ);
			ascDefPtr->SetValue ("DELTA_Z:",chrBufr);

			// The four extra parameters might not be there, yet.
			sprintf (chrBufr,"%.6f",tblPtr->NewBwScale);
			linePtr = ascDefPtr->GetLine ("BWSCALE:");
			if (linePtr != 0)
			{
				ascDefPtr->SetValue ("BWSCALE:",chrBufr);
			}
			else
			{
				TcsDefLine newGrpLine (dictTypDatum,"BWSCALE:",chrBufr,nullString);
				ascDefPtr->InsertAfter ("DELTA_Z:",newGrpLine);
			}

			sprintf (chrBufr,"%.6f",tblPtr->NewRotX);
			linePtr = ascDefPtr->GetLine ("ROT_X:");
			if (linePtr != 0)
			{
				ascDefPtr->SetValue ("ROT_X:",chrBufr);
			}
			else
			{
				TcsDefLine newGrpLine (dictTypDatum,"ROT_X:",chrBufr,nullString);
				ascDefPtr->InsertAfter ("BWSCALE:",newGrpLine);
			}

			sprintf (chrBufr,"%.6f",tblPtr->NewRotY);
			linePtr = ascDefPtr->GetLine ("ROT_Y:");
			if (linePtr != 0)
			{
				ascDefPtr->SetValue ("ROT_Y:",chrBufr);
			}
			else
			{
				TcsDefLine newGrpLine (dictTypDatum,"ROT_Y:",chrBufr,nullString);
				ascDefPtr->InsertAfter ("ROT_X:",newGrpLine);
			}

			sprintf (chrBufr,"%.6f",tblPtr->NewRotZ);
			linePtr = ascDefPtr->GetLine ("ROT_Z:");
			if (linePtr != 0)
			{
				ascDefPtr->SetValue ("ROT_Z:",chrBufr);
			}
			else
			{
				TcsDefLine newGrpLine (dictTypDatum,"ROT_Z:",chrBufr,nullString);
				ascDefPtr->InsertAfter ("ROT_Y:",newGrpLine);
			}
		}
		else
		{
			ok = false;
		}

		// Step 5: Deprecate the copy.  If there is no GROUP element, we need to
		// insert one.
		TcsDefLine* grpLinePtr = originalCopy.GetLine ("GROUP:");
		if (grpLinePtr == 0)
		{
			TcsDefLine newGrpLine (dictTypDatum,"GROUP:","LEGACY","");
			originalCopy.InsertAfter ("DESC_NM:",newGrpLine);
		}
		else
		{
			originalCopy.SetValue ("GROUP:","LEGACY");
		}
		sprintf (chrBufr,"Deprecated by EPSG synchronization. Replaced by %s.",newXformName);
		originalCopy.SetValue ("DESC_NM:",chrBufr);

		// Step 6: Append the deprecated version to the end of the definition file.
		// Note that the original blanks lines, and comments are still there.
		// We may want to "StripLeadingComments" and then prepend a different
		// comment.  These methods would need to be added to the TcsAscDefinition
		// object.
		xformDefs.Append (originalCopy);

		// That's it for this one.
	}

	// If we are still OK, we write the results out to the provided target directory.
	if (ok)
	{
		wcstombs (pathName,trgDictDir,sizeof (pathName));
		strcat (pathName,"\\GeodeticTransformation.asc");
		oStrm.open (pathName,std::ios_base::out | std::ios_base::trunc);
		if (oStrm.is_open ())
		{
			ok = xformDefs.WriteToStream (oStrm);
			oStrm.close ();
		}
	}
	return ok;
}
bool Epsg706UpdatePaths (const wchar_t* srcDictDir,const wchar_t* dataDir,const wchar_t* trgDictDir)
{
	// For EPSG 7.06, there are no changes required to the Path dictioanry.
	// We save a lot of code by this condition being true.
	return true;
}
bool Epsg706UpdateCrs (const wchar_t* srcDictDir,const wchar_t* dataDir,const wchar_t* trgDictDir)
{
	bool ok;

	TcsAscDefinition* ascDefPtr;
	TcsEpsg706CrsUpdateTbl* tblPtr;

	char chrBufr [256];
	char nullString [2] = {'\0','\0'};
	char pathName [1024];

	TcsCsvStatus csvStatus;

	wcstombs (pathName,srcDictDir,sizeof (pathName));
	strcat (pathName,"\\coordsys.asc");
	TcsDefFile crsDefs (dictTypCoordsys,pathName);

	TcsCategoryFile categoryDefs;
	{
		std::ifstream iStrm;	
		wcstombs (pathName,srcDictDir,sizeof (pathName));
		strcat (pathName,"\\category.asc");
		iStrm.open (pathName,std::ios_base::in);
		ok = iStrm.is_open ();
		if (ok)
		{
			ok = categoryDefs.ReadFromStream (iStrm);
		}
	}
	if (!ok) return ok;

	short crsAdskFldNbr;
	unsigned invalidRecordNbr;
	TcsCsvFileBase crsNameMap (true,28,28);
	{
		std::wifstream wiStrm;
		wcstombs (pathName,dataDir,sizeof (pathName));
		strcat (pathName,"\\ProjectiveKeyNameMap.csv");
		wiStrm.open (pathName,std::ios_base::in);
		ok = wiStrm.is_open ();
		if (ok)
		{
			ok = crsNameMap.ReadFromStream (wiStrm,true,csvStatus);
			wiStrm.close ();
		}
		crsAdskFldNbr = crsNameMap.GetFldNbr (L"AdskName",csvStatus);
		invalidRecordNbr = crsNameMap.GetInvalidRecordNbr();
	}
	if (!ok) return ok;

	short geogAdskFldNbr;
	TcsCsvFileBase geogNameMap (true,28,28);
	{
		std::wifstream wiStrm;
		wcstombs (pathName,dataDir,sizeof (pathName));
		strcat (pathName,"\\GeographicKeyNameMap.csv");
		wiStrm.open (pathName,std::ios_base::in);
		ok = wiStrm.is_open ();
		if (ok)
		{
			ok = geogNameMap.ReadFromStream (wiStrm,true,csvStatus);
			wiStrm.close ();
		}
		geogAdskFldNbr = crsNameMap.GetFldNbr (L"AdskName",csvStatus);
	}
	if (!ok) return ok;

	// OK, we're ready to go.  We do nine steps for each CRS in the table.
	ok = true;				// until we know different.
	for (tblPtr = KcsEpsg706CrsUpdateTbl;ok && tblPtr->OldDtmName [0] != '\0';tblPtr += 1)
	{
		// Step 1: Locate the original definition.
		ascDefPtr = crsDefs.GetDefinition (tblPtr->OldCrsName);
		if (ascDefPtr == 0)
		{
			ok = false;
			break;
		}

		// Step 2: Make a copy of it.
		TcsAscDefinition originalCopy (*ascDefPtr);
		
		// Step 2.5:  Verify that this deifnition is not in the legacy group.
		// We don't want to edit a Legacy definition.
		const char* grpValuePtr = originalCopy.GetValue ("GROUP:");
		if (grpValuePtr != 0)
		{
			if (!CS_stricmp (grpValuePtr,"LEGACY"))
			{
				printf ("%s has already been deprecated, should not be on this list.\n",tblPtr->OldCrsName);
				continue;
			}
		}

		// Step 6: Change the name of the active definition to the new name.
		ascDefPtr->RenameDef (tblPtr->NewCrsName);

		// Step 4: Update the definition
		ascDefPtr->SetValue ("DT_NAME:",tblPtr->NewDtmName);

		// Step 5: Deprecate the copy.  If there is no GROUP element, we need to
		// insert one.
		TcsDefLine* grpLinePtr = originalCopy.GetLine ("GROUP:");
		if (grpLinePtr == 0)
		{
			TcsDefLine newGrpLine (dictTypDatum,"GROUP:","LEGACY",nullString);
			originalCopy.InsertAfter ("DESC_NM:",newGrpLine);
		}
		else
		{
			originalCopy.SetValue ("GROUP:","LEGACY");
		}
		sprintf (chrBufr,"Deprecated by EPSG synch. Replaced by %s.",tblPtr->NewCrsName);
		originalCopy.SetValue ("DESC_NM:",chrBufr);

		// Step 6: Append the deprecated version to the end of the definition file.
		crsDefs.Append (originalCopy);

		// Step 7:  Locate the old name in the Projective Key Name Map file.
		if (ok)
		{
			if (!tblPtr->geographic)
			{
				int rplStatus = CS_nmMprRplName (crsNameMap,crsAdskFldNbr,tblPtr->OldCrsName,tblPtr->NewCrsName,true);
				ok = (rplStatus >= 0);
				if (rplStatus == 0)
				{
					printf ("Replacement of %s with %s in ProjectiveKeyNameMap failed.\n",tblPtr->OldCrsName,tblPtr->NewCrsName);
				}
			}
			else
			{
				int rplStatus = CS_nmMprRplName (geogNameMap,geogAdskFldNbr,tblPtr->OldCrsName,tblPtr->NewCrsName,true);
				ok = (rplStatus >= 0);
				if (rplStatus == 0)
				{
					printf ("Replacement of %s with %s in GeographicKeyNameMap failed.\n",tblPtr->OldCrsName,tblPtr->NewCrsName);
				}
			}
		}

		// Step 9  Change all references to the Old CRS name in the category
		// dictionary to the new CRS name, and then add a deprecated copy of a
		// copy of the original to the Obsolete category.  This is a lot of
		// work so we buried all of this code in the TcsCategoryFile object.
		if (ok)
		{
			ok = categoryDefs.DeprecateCrs (tblPtr->OldCrsName,tblPtr->NewCrsName,
															   "Deprecated by EPSG 7.06 synchronization.",
															   tblPtr->NewCrsDescription);
		}
		// That's it for this one.
	}

	// If we are still OK, we write the results out to the provided target directory.
	if (ok)
	{
		std::ofstream oStrm;
		wcstombs (pathName,trgDictDir,sizeof (pathName));
		strcat (pathName,"\\Coordsys.asc");
		oStrm.open (pathName,std::ios_base::out | std::ios_base::trunc);
		if (oStrm.is_open ())
		{
			ok = crsDefs.WriteToStream (oStrm);
			oStrm.close ();
		}
	}
	if (ok)
	{
		std::ofstream oStrm;
		wcstombs (pathName,trgDictDir,sizeof (pathName));
		strcat (pathName,"\\Category.asc");
		oStrm.open (pathName,std::ios_base::out | std::ios_base::trunc);
		if (oStrm.is_open ())
		{
			ok = categoryDefs.WriteToStream (oStrm);
			oStrm.close ();
		}
	}
	if (ok)
	{
		std::wofstream woStrm;
		wcstombs (pathName,dataDir,sizeof (pathName));
		strcat (pathName,"\\ProjectiveKeyNameMap.csv");
		woStrm.open (pathName,std::ios_base::out | std::ios_base::trunc);
		if (woStrm.is_open ())
		{
			crsNameMap.WriteToStream (woStrm,true,csvStatus);
			woStrm.close ();
		}
	}
	if (ok)
	{
		std::wofstream woStrm;
		wcstombs (pathName,dataDir,sizeof (pathName));
		strcat (pathName,"\\GeographicKeyNameMap.csv");
		woStrm.open (pathName,std::ios_base::out | std::ios_base::trunc);
		if (woStrm.is_open ())
		{
			geogNameMap.WriteToStream (woStrm,true,csvStatus);
			woStrm.close ();
		}
	}

	return ok;
}
