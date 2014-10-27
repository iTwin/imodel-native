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

enum csSevenParameterAction {
								cs7pActnNone      = 0,
								cs7pActnRevert,
								cs7pActnFlipSigns,
								cs7pActnUpdate,
								cs7pActnRestore,
								cs7pActnError
							};
struct Tcs7ParmFixNames
{
	unsigned               dtIndex;
	csSevenParameterAction action;
	char                   dtmName1986 [24];
	char                   crsName1986 [24];
	char                   newDtmName  [24];
	char                   newCrsName  [24];
	unsigned               epsgOprCode;
	unsigned               epsgVariant;
}
Kcs7ParmFixNames [] =
{
	{  0,     cs7pActnRevert, "Kartasto66a",          "KKJa.Finland-1",            "Kartasto66b",          "KKJb.Finland-1",                0,   0   },  // ok
	{  1,     cs7pActnRevert, "Kartasto66a",          "KKJa.Finland-2",            "Kartasto66b",          "KKJb.Finland-2",                0,   0   },  // ok
	{  2,     cs7pActnRevert, "Kartasto66a",          "KKJa.Finland-UCS",          "Kartasto66b",          "KKJb.Finland-UCS",              0,   0   },  // ok
	{  3,     cs7pActnRevert, "Kartasto66a",          "KKJa.Finland-4",            "Kartasto66b",          "KKJb.Finland-4",                0,   0   },  // ok
	{  4,     cs7pActnRevert, "Kartasto66a",          "KKJa.LL",                   "Kartasto66b",          "KKJb.LL",                       0,   0   },  // ok
	{  0,  cs7pActnFlipSigns, "RT90-3/7P",            "RT90-3/7P.SW-75GONV",       "RT90-3/7Pa",           "RT90-3/7Pa.SW-75GONV",       1896,   2   },  // ok
	{  1,  cs7pActnFlipSigns, "RT90-3/7P",            "RT90-3/7P.SW-5GONV",        "RT90-3/7Pa",           "RT90-3/7Pa.SW-5GONV",        1896,   2   },  // ok
	{  2,  cs7pActnFlipSigns, "RT90-3/7P",            "RT90-3/7P.SW-25GONV",       "RT90-3/7Pa",           "RT90-3/7Pa.SW-25GONV",       1896,   2   },  // ok
	{  3,  cs7pActnFlipSigns, "RT90-3/7P",            "RT90-3/7P.SW-0GONV",        "RT90-3/7Pa",           "RT90-3/7Pa.SW-0GONV",        1896,   2   },  // ok
	{  4,  cs7pActnFlipSigns, "RT90-3/7P",            "RT90-3/7P.SW-25GONO",       "RT90-3/7Pa",           "RT90-3/7Pa.SW-25GONO",       1896,   2   },  // ok
	{  5,  cs7pActnFlipSigns, "RT90-3/7P",            "RT90-3/7P.SW-5GONO",        "RT90-3/7Pa",           "RT90-3/7Pa.SW-5GONO",        1896,   2   },  // ok
	{  6,  cs7pActnFlipSigns, "RT90-3/7P",            "RT90-3/7P.SW-NAT90",        "RT90-3/7Pa",           "RT90-3/7Pa.SW-NAT90",        1896,   2   },  // ok
	{  7,  cs7pActnFlipSigns, "RT90-3/7P",            "RT90-3/7P.LL",              "RT90-3/7Pa",           "RT90-3/7Pa.LL",              1896,   2   },  // ok
	{  8,  cs7pActnFlipSigns, "RT90-3/7P",            "RT90-3/7P.SW-7.5O",         "RT90-3/7Pa",           "RT90-3/7Pa.SW-7.5O",         1896,   2   },  // ok
	{  0,     cs7pActnRevert, "TETE/a",               "TETE/a.MOZ-T-W",            "TETE/b",               "TETE/b.MOZ-T-W",                0,   0   },  // ok
	{  1,     cs7pActnRevert, "TETE/a",               "TETE/a.MOZ-T-E",            "TETE/b",               "TETE/b.MOZ-T-E",                0,   0   },  // ok
	{  2,     cs7pActnRevert, "TETE/a",               "Tete/a.UTM-36S",            "TETE/b",               "TETE/b.UTM-36S",                0,   0   },  // ok
	{  3,     cs7pActnRevert, "TETE/a",               "Tete/a.UTM-37S",            "TETE/b",               "TETE/b.UTM-37S",                0,   0   },  // ok
	{  4,     cs7pActnRevert, "TETE/a",               "Tete/a.LL",                 "TETE/b",               "TETE/b.LL",                     0,   0   },  // ok
	{  0,     cs7pActnRevert, "Estonia92a",           "Estonia92a.Estonia",        "Estonia92b",           "Estonia92b.Estonia",            0,   0   },  // ok
	{  1,     cs7pActnRevert, "Estonia92a",           "Estonia92a.LL",             "Estonia92b",           "Estonia92b.LL",                 0,   0   },  // ok
	{  0,     cs7pActnRevert, "PDOSurvey93a",         "PSD93a.UTM-39N",            "PDOSurvey93b",         "PSD93b.UTM-39N",                0,   0   },  // ok
	{  1,     cs7pActnRevert, "PDOSurvey93a",         "PSD93a.UTM-40N",            "PDOSurvey93b",         "PSD93b.UTM-40N",                0,   0   },  // ok
	{  2,     cs7pActnRevert, "PDOSurvey93a",         "PDOSurvey93a.LL",           "PDOSurvey93b",         "PDOSurvey93b.LL",               0,   0   },  // ok
	{  0,  cs7pActnFlipSigns, "CzechJTSK/5",          "CzechJTSK/5.Krovak",        "CzechJTSK/5b",         "CzechJTSK/5b.Krovak",           0,   0   },  // ok
	{  1,  cs7pActnFlipSigns, "CzechJTSK/5",          "CzechJTSK/5.LL",            "CzechJTSK/5b",         "CzechJTSK/5b.LL",               0,   0   },  // ok
	{  0,     cs7pActnRevert, "Pulkovo42/83a",        "Pulkovo42/83a.GK-3",        "Pulkovo42/83b",        "Pulkovo42/83b.GK-3",            0,   0   },  // ok
	{  1,     cs7pActnRevert, "Pulkovo42/83a",        "Pulkovo42/83a.GK-4",        "Pulkovo42/83b",        "Pulkovo42/83b.GK-4",            0,   0   },  // ok
	{  2,     cs7pActnRevert, "Pulkovo42/83a",        "Pulkovo42/83a.GK-5",        "Pulkovo42/83b",        "Pulkovo42/83b.GK-5",            0,   0   },  // ok
	{  3,     cs7pActnRevert, "Pulkovo42/83a",        "Pulkovo42/83a.LL",          "Pulkovo42/83b",        "Pulkovo42/83b.LL",              0,   0   },  // ok
	{  0,     cs7pActnRevert, "Pulkovo42/58a",        "Pulkovo/58a.Poland-II",     "Pulkovo42/58b",        "Pulkovo/58b.Poland-II",         0,   0   },  // ok
	{  1,     cs7pActnRevert, "Pulkovo42/58a",        "Pulkovo/58a.Poland-III",    "Pulkovo42/58b",        "Pulkovo/58b.Poland-III",        0,   0   },  // ok
	{  2,     cs7pActnRevert, "Pulkovo42/58a",        "Pulkovo/58a.Poland-IV",     "Pulkovo42/58b",        "Pulkovo/58b.Poland-IV",         0,   0   },  // ok
	{  3,     cs7pActnRevert, "Pulkovo42/58a",        "Pulkovo/58a.Poland-V",      "Pulkovo42/58b",        "Pulkovo/58b.Poland-V",          0,   0   },  // ok
	{  4,     cs7pActnRevert, "Pulkovo42/58a",        "Pulkovo42/58a.LL",          "Pulkovo42/58b",        "Pulkovo42/58b.LL",              0,   0   },  // ok
	{  5,     cs7pActnRevert, "Pulkovo42/58a",        "Pulkovo42/58a.Poland-1",    "Pulkovo42/58b",        "Pulkovo42/58b.Poland-1",        0,   0   },  // ok
	{  6,     cs7pActnRevert, "Pulkovo42/58a",        "Pulkovo42/58a.Stereo70",    "Pulkovo42/58b",        "Pulkovo42/58b.Stereo70",        0,   0   },  // ok
	{  0,     cs7pActnRevert, "Luxembourg30a",        "Luxembourg30a.Gauss",       "Luxembourg30b",        "Luxembourg30b.Gauss",           0,   0   },  // ok
	{  1,     cs7pActnRevert, "Luxembourg30a",        "Luxembourg30a.LL",          "Luxembourg30b",        "Luxembourg30b.LL",              0,   0   },  // ok
	{  0,  cs7pActnFlipSigns, "OSNI52",               "OSNI52.LL",                 "OSNI52/b",             "OSNI52/b.LL",                   0,   0   },  // ok
	{  1,  cs7pActnFlipSigns, "OSNI52",               "OSNI52.IrishNtlGrid",       "OSNI52/b",             "OSNI52/b.IrishNtlGrid",         0,   0   },  // ok
	{  0,     cs7pActnRevert, "Scoresbysund52a",      "Scoresbysund52a.LL",        "Scoresbysund52b",      "Scoresbysund52b.LL",            0,   0   },  // ok
	{  0,     cs7pActnRevert, "Ammassalik58a",        "Ammassalik58a.LL",          "Ammassalik58b",        "Ammassalik58b.LL",              0,   0   },  // ok
	{  0,  cs7pActnFlipSigns, "Lisbon37a",            "Lisbon37a.LL",              "Lisbon37/b",           "Lisbon37/b.LL",              1988,   4   },  // ok
	{  0,  cs7pActnFlipSigns, "Europ87",              "Europ87.LL",                "Europ87/a",            "Europ87/a.LL",                  0,   0   },  // ok
	{  0,  cs7pActnFlipSigns, "HD72/7P",              "HD72/7P.EOV",               "HD72/7Pa",             "HD72/7Pa.EOV",               1448,   3   },  // ok
	{  1,  cs7pActnFlipSigns, "HD72/7P",              "HD72/7P.LL",                "HD72/7Pa",             "HD72/7Pa.LL",                1448,   3   },  // ok
	{  0,    cs7pActnRestore, "HD72-7P-CORR",         "HD-72.LL",                  "",                     "HD-72.LL",                   1829,   1   },  // ok
	{  1,    cs7pActnRestore, "HD72-7P-CORR",         "HUN-EOV72-7P",              "",                     "HUN-EOV72-7P",               1829,   1   },  // ok
	{  0,     cs7pActnRevert, "HitoXVIII63a",         "Hito63a.Argentina-2",       "HitoXVIII63b",         "Hito63b.Argentina-2",           0,   0   },  // ok
	{  1,     cs7pActnRevert, "HitoXVIII63a",         "Hito63a.UTM-19S",           "HitoXVIII63b",         "Hito63b.UTM-19S",               0,   0   },  // ok
	{  2,     cs7pActnRevert, "HitoXVIII63a",         "HitoXVIII63a.LL",           "HitoXVIII63b",         "HitoXVIII63b.LL",               0,   0   },  // ok
	{  0,     cs7pActnRevert, "NGO48a",               "NGO48a.LL",                 "NGO48b",               "NGO48b.LL",                     0,   0   },  // ok
	{  0,  cs7pActnFlipSigns, "Datum73-Mod/a",        "Datum73a.LL",               "Datum73-Mod/b",        "Datum73b.LL",                   0,   0   },  // ok
	{  1,  cs7pActnFlipSigns, "Datum73-Mod/a",        "Datum73a.UTM-29N",          "Datum73-Mod/b",        "Datum73b.UTM-29N",              0,   0   },  // ok
	{  2,  cs7pActnFlipSigns, "Datum73-Mod/a",        "Datum73a.ModPortgGrd",      "Datum73-Mod/b",        "Datum73b.ModPortgGrd",          0,   0   },  // ok
	{  0,  cs7pActnFlipSigns, "Palestine23",          "Palestine23.LL",            "Palestine23a",         "Palestine23a.LL",               0,   0   },  // ok
	{  1,  cs7pActnFlipSigns, "Palestine23",          "Palestine23.Grid",          "Palestine23a",         "Palestine23a.Grid",             0,   0   },  // ok
	{  2,  cs7pActnFlipSigns, "Palestine23",          "Palestine23.Belt",          "Palestine23a",         "Palestine23a.Belt",             0,   0   },  // ok
	{  3,  cs7pActnFlipSigns, "Palestine23",          "Palestine23.IsraeliGrd",    "Palestine23a",         "Palestine23a.IsraeliGrd",       0,   0   },  // ok
	{  0,  cs7pActnFlipSigns, "TM1965/a",             "TM1965/a.LL",               "TM1965/b",             "TM1965/b.LL",                   0,   0   },  // ok
	{  1,  cs7pActnFlipSigns, "TM1965/a",             "TM1965/a.IrishGrid",        "TM1965/b",             "TM1965/b.IrishGrid",            0,   0   },  // ok
	{  0,  cs7pActnFlipSigns, "MGI-AT",               "MGI-AT.LL",                 "MGI-AT/a",             "MGI-AT/a.LL",                   0,   0   },  // ok
	{  1,  cs7pActnFlipSigns, "MGI-AT",               "MGI-AT.AUT-West/GK",        "MGI-AT/a",             "MGI-AT/a.AUT-West/GK",          0,   0   },  // ok
	{  2,  cs7pActnFlipSigns, "MGI-AT",               "MGI-AT.AUT-Central/GK",     "MGI-AT/a",             "MGI-AT/a.AUT-Central/GK",       0,   0   },  // ok
	{  3,  cs7pActnFlipSigns, "MGI-AT",               "MGI-AT.AUT-East/GK",        "MGI-AT/a",             "MGI-AT/a.AUT-East/GK",          0,   0   },  // ok
	{  4,  cs7pActnFlipSigns, "MGI-AT",               "MGI-AT.M28/GKa",            "MGI-AT/a",             "MGI-AT/a.M28/GKa",              0,   0   },  // ok
	{  5,  cs7pActnFlipSigns, "MGI-AT",               "MGI-AT.M31/GKa",            "MGI-AT/a",             "MGI-AT/a.M31/GKa",              0,   0   },  // ok
	{  6,  cs7pActnFlipSigns, "MGI-AT",               "MGI-AT.M34/GKa",            "MGI-AT/a",             "MGI-AT/a.M34/GKa",              0,   0   },  // ok
	{  7,  cs7pActnFlipSigns, "MGI-AT",               "MGI-AT.M28",                "MGI-AT/a",             "MGI-AT/a.M28",                  0,   0   },  // ok
	{  8,  cs7pActnFlipSigns, "MGI-AT",               "MGI-AT.M31",                "MGI-AT/a",             "MGI-AT/a.M31",                  0,   0   },  // ok
	{  9,  cs7pActnFlipSigns, "MGI-AT",               "MGI-AT.M34",                "MGI-AT/a",             "MGI-AT/a.M34",                  0,   0   },  // ok
	{ 10,  cs7pActnFlipSigns, "MGI-AT",               "MGI-AT.AustriaLambert",     "MGI-AT/a",             "MGI-AT/a.AustriaLambert",       0,   0   },  // ok
//	{ 11,  cs7pActnFlipSigns, "MGI-AT",               "MGI-AT.M28/GK",             "MGI-AT/a",             "MGI-AT/a.M28/GK",               0,   0   },  // Already deprecated
//	{ 12,  cs7pActnFlipSigns, "MGI-AT",               "MGI-AT.M31/GK",             "MGI-AT/a",             "MGI-AT/a.M31/GK",               0,   0   },  // Already deprecated
//	{ 13,  cs7pActnFlipSigns, "MGI-AT",               "MGI-AT.M34/GK",             "MGI-AT/a",             "MGI-AT/a.M34/GK",               0,   0   },  // Already deprecated
	{  0,  cs7pActnFlipSigns, "Belge72a",             "Belge72a.LL",               "Belge72/b",            "Belge72/b.LL",                  0,   0   },  // ok
	{  1,  cs7pActnFlipSigns, "Belge72a",             "Belge72a.Lambert72",        "Belge72/b",            "Belge72/b.Lambert72",           0,   0   },  // ok
	{  2,  cs7pActnFlipSigns, "Belge72a",             "Belge72a.Lambert72A",       "Belge72/b",            "Belge72/b.Lambert72A",          0,   0   },  // ok
//	{  0,  cs7pActnFlipSigns, "DHDN/2",               "DHDN/2.LL",                 "DHDN/3",               "DHDN/3.LL",                     0,   0   },  // Already done, submision 2269
//	{  1,  cs7pActnFlipSigns, "DHDN/2",               "DHDN/2.Gauss3d-1",          "DHDN/3",               "DHDN/3.Gauss3d-1",              0,   0   },  // Already done, submision 2269
//	{  2,  cs7pActnFlipSigns, "DHDN/2",               "DHDN/2.Gauss3d-2",          "DHDN/3",               "DHDN/3.Gauss3d-2",              0,   0   },  // Already done, submision 2269
//	{  3,  cs7pActnFlipSigns, "DHDN/2",               "DHDN/2.Gauss3d-3",          "DHDN/3",               "DHDN/3.Gauss3d-3",              0,   0   },  // Already done, submision 2269
//	{  4,  cs7pActnFlipSigns, "DHDN/2",               "DHDN/2.Gauss3d-4",          "DHDN/3",               "DHDN/3.Gauss3d-4",              0,   0   },  // Already done, submision 2269
//	{  5,  cs7pActnFlipSigns, "DHDN/2",               "DHDN/2.Gauss3d-5",          "DHDN/3",               "DHDN/3.Gauss3d-5",              0,   0   },  // Already done, submision 2269
//	{  6,  cs7pActnFlipSigns, "DHDN/2",               "DHDN/2.Berlin/Cassini",     "DHDN/3",               "DHDN/3.Berlin/Cassini",         0,   0   },  // Already done, submision 2269
	{  0,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.TM-106NE",        "WGS72-TBE/b",          "WGS72be/b.TM-106NE/a",          0,   0   },  // ok
	{  1,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72-TBE/a.LL",            "WGS72-TBE/b",          "WGS72-TBE/b.LL",                0,   0   },  // ok
	{  2,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-1N",          "WGS72-TBE/b",          "WGS72be/b.UTM-1N",              0,   0   },  // ok
	{  3,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-2N",          "WGS72-TBE/b",          "WGS72be/b.UTM-2N",              0,   0   },  // ok
	{  4,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-3N",          "WGS72-TBE/b",          "WGS72be/b.UTM-3N",              0,   0   },  // ok
	{  5,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-4N",          "WGS72-TBE/b",          "WGS72be/b.UTM-4N",              0,   0   },  // ok
	{  6,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-5N",          "WGS72-TBE/b",          "WGS72be/b.UTM-5N",              0,   0   },  // ok
	{  7,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-6N",          "WGS72-TBE/b",          "WGS72be/b.UTM-6N",              0,   0   },  // ok
	{  8,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-7N",          "WGS72-TBE/b",          "WGS72be/b.UTM-7N",              0,   0   },  // ok
	{  9,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-8N",          "WGS72-TBE/b",          "WGS72be/b.UTM-8N",              0,   0   },  // ok
	{ 10,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-9N",          "WGS72-TBE/b",          "WGS72be/b.UTM-9N",              0,   0   },  // ok
	{ 11,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-10N",         "WGS72-TBE/b",          "WGS72be/b.UTM-10N",             0,   0   },  // ok
	{ 12,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-11N",         "WGS72-TBE/b",          "WGS72be/b.UTM-11N",             0,   0   },  // ok
	{ 13,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-12N",         "WGS72-TBE/b",          "WGS72be/b.UTM-12N",             0,   0   },  // ok
	{ 14,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-13N",         "WGS72-TBE/b",          "WGS72be/b.UTM-13N",             0,   0   },  // ok
	{ 15,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-14N",         "WGS72-TBE/b",          "WGS72be/b.UTM-14N",             0,   0   },  // ok
	{ 16,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-15N",         "WGS72-TBE/b",          "WGS72be/b.UTM-15N",             0,   0   },  // ok
	{ 17,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-16N",         "WGS72-TBE/b",          "WGS72be/b.UTM-16N",             0,   0   },  // ok
	{ 18,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-17N",         "WGS72-TBE/b",          "WGS72be/b.UTM-17N",             0,   0   },  // ok
	{ 19,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-18N",         "WGS72-TBE/b",          "WGS72be/b.UTM-18N",             0,   0   },  // ok
	{ 20,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-19N",         "WGS72-TBE/b",          "WGS72be/b.UTM-19N",             0,   0   },  // ok
	{ 21,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-20N",         "WGS72-TBE/b",          "WGS72be/b.UTM-20N",             0,   0   },  // ok
	{ 22,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-21N",         "WGS72-TBE/b",          "WGS72be/b.UTM-21N",             0,   0   },  // ok
	{ 23,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-22N",         "WGS72-TBE/b",          "WGS72be/b.UTM-22N",             0,   0   },  // ok
	{ 24,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-23N",         "WGS72-TBE/b",          "WGS72be/b.UTM-23N",             0,   0   },  // ok
	{ 25,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-24N",         "WGS72-TBE/b",          "WGS72be/b.UTM-24N",             0,   0   },  // ok
	{ 26,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-25N",         "WGS72-TBE/b",          "WGS72be/b.UTM-25N",             0,   0   },  // ok
	{ 27,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-26N",         "WGS72-TBE/b",          "WGS72be/b.UTM-26N",             0,   0   },  // ok
	{ 28,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-27N",         "WGS72-TBE/b",          "WGS72be/b.UTM-27N",             0,   0   },  // ok
	{ 29,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-28N",         "WGS72-TBE/b",          "WGS72be/b.UTM-28N",             0,   0   },  // ok
	{ 30,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-29N",         "WGS72-TBE/b",          "WGS72be/b.UTM-29N",             0,   0   },  // ok
	{ 31,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-30N",         "WGS72-TBE/b",          "WGS72be/b.UTM-30N",             0,   0   },  // ok
	{ 32,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-31N",         "WGS72-TBE/b",          "WGS72be/b.UTM-31N",             0,   0   },  // ok
	{ 33,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-32N",         "WGS72-TBE/b",          "WGS72be/b.UTM-32N",             0,   0   },  // ok
	{ 34,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-33N",         "WGS72-TBE/b",          "WGS72be/b.UTM-33N",             0,   0   },  // ok
	{ 35,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-34N",         "WGS72-TBE/b",          "WGS72be/b.UTM-34N",             0,   0   },  // ok
	{ 36,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-35N",         "WGS72-TBE/b",          "WGS72be/b.UTM-35N",             0,   0   },  // ok
	{ 37,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-36N",         "WGS72-TBE/b",          "WGS72be/b.UTM-36N",             0,   0   },  // ok
	{ 38,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-37N",         "WGS72-TBE/b",          "WGS72be/b.UTM-37N",             0,   0   },  // ok
	{ 39,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-38N",         "WGS72-TBE/b",          "WGS72be/b.UTM-38N",             0,   0   },  // ok
	{ 40,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-39N",         "WGS72-TBE/b",          "WGS72be/b.UTM-39N",             0,   0   },  // ok
	{ 41,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-40N",         "WGS72-TBE/b",          "WGS72be/b.UTM-40N",             0,   0   },  // ok
	{ 42,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-41N",         "WGS72-TBE/b",          "WGS72be/b.UTM-41N",             0,   0   },  // ok
	{ 43,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-42N",         "WGS72-TBE/b",          "WGS72be/b.UTM-42N",             0,   0   },  // ok
	{ 44,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-43N",         "WGS72-TBE/b",          "WGS72be/b.UTM-43N",             0,   0   },  // ok
	{ 45,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-44N",         "WGS72-TBE/b",          "WGS72be/b.UTM-44N",             0,   0   },  // ok
	{ 46,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-45N",         "WGS72-TBE/b",          "WGS72be/b.UTM-45N",             0,   0   },  // ok
	{ 47,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-46N",         "WGS72-TBE/b",          "WGS72be/b.UTM-46N",             0,   0   },  // ok
	{ 48,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-47N",         "WGS72-TBE/b",          "WGS72be/b.UTM-47N",             0,   0   },  // ok
	{ 49,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-48N",         "WGS72-TBE/b",          "WGS72be/b.UTM-48N",             0,   0   },  // ok
	{ 50,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-49N",         "WGS72-TBE/b",          "WGS72be/b.UTM-49N",             0,   0   },  // ok
	{ 51,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-50N",         "WGS72-TBE/b",          "WGS72be/b.UTM-50N",             0,   0   },  // ok
	{ 52,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-51N",         "WGS72-TBE/b",          "WGS72be/b.UTM-51N",             0,   0   },  // ok
	{ 53,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-52N",         "WGS72-TBE/b",          "WGS72be/b.UTM-52N",             0,   0   },  // ok
	{ 54,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-53N",         "WGS72-TBE/b",          "WGS72be/b.UTM-53N",             0,   0   },  // ok
	{ 55,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-54N",         "WGS72-TBE/b",          "WGS72be/b.UTM-54N",             0,   0   },  // ok
	{ 56,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-55N",         "WGS72-TBE/b",          "WGS72be/b.UTM-55N",             0,   0   },  // ok
	{ 57,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-56N",         "WGS72-TBE/b",          "WGS72be/b.UTM-56N",             0,   0   },  // ok
	{ 58,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-57N",         "WGS72-TBE/b",          "WGS72be/b.UTM-57N",             0,   0   },  // ok
	{ 59,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-58N",         "WGS72-TBE/b",          "WGS72be/b.UTM-58N",             0,   0   },  // ok
	{ 60,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-59N",         "WGS72-TBE/b",          "WGS72be/b.UTM-59N",             0,   0   },  // ok
	{ 61,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-60N",         "WGS72-TBE/b",          "WGS72be/b.UTM-60N",             0,   0   },  // ok
	{ 62,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-1S",          "WGS72-TBE/b",          "WGS72be/b.UTM-1S",              0,   0   },  // ok
	{ 63,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-2S",          "WGS72-TBE/b",          "WGS72be/b.UTM-2S",              0,   0   },  // ok
	{ 64,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-3S",          "WGS72-TBE/b",          "WGS72be/b.UTM-3S",              0,   0   },  // ok
	{ 65,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-4S",          "WGS72-TBE/b",          "WGS72be/b.UTM-4S",              0,   0   },  // ok
	{ 66,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-5S",          "WGS72-TBE/b",          "WGS72be/b.UTM-5S",              0,   0   },  // ok
	{ 67,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-6S",          "WGS72-TBE/b",          "WGS72be/b.UTM-6S",              0,   0   },  // ok
	{ 68,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-7S",          "WGS72-TBE/b",          "WGS72be/b.UTM-7S",              0,   0   },  // ok
	{ 69,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-8S",          "WGS72-TBE/b",          "WGS72be/b.UTM-8S",              0,   0   },  // ok
	{ 70,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-9S",          "WGS72-TBE/b",          "WGS72be/b.UTM-9S",              0,   0   },  // ok
	{ 71,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-10S",         "WGS72-TBE/b",          "WGS72be/b.UTM-10S",             0,   0   },  // ok
	{ 72,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-11S",         "WGS72-TBE/b",          "WGS72be/b.UTM-11S",             0,   0   },  // ok
	{ 73,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-12S",         "WGS72-TBE/b",          "WGS72be/b.UTM-12S",             0,   0   },  // ok
	{ 74,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-13S",         "WGS72-TBE/b",          "WGS72be/b.UTM-13S",             0,   0   },  // ok
	{ 75,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-14S",         "WGS72-TBE/b",          "WGS72be/b.UTM-14S",             0,   0   },  // ok
	{ 76,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-15S",         "WGS72-TBE/b",          "WGS72be/b.UTM-15S",             0,   0   },  // ok
	{ 77,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-16S",         "WGS72-TBE/b",          "WGS72be/b.UTM-16S",             0,   0   },  // ok
	{ 78,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-17S",         "WGS72-TBE/b",          "WGS72be/b.UTM-17S",             0,   0   },  // ok
	{ 79,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-18S",         "WGS72-TBE/b",          "WGS72be/b.UTM-18S",             0,   0   },  // ok
	{ 80,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-19S",         "WGS72-TBE/b",          "WGS72be/b.UTM-19S",             0,   0   },  // ok
	{ 81,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-20S",         "WGS72-TBE/b",          "WGS72be/b.UTM-20S",             0,   0   },  // ok
	{ 82,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-21S",         "WGS72-TBE/b",          "WGS72be/b.UTM-21S",             0,   0   },  // ok
	{ 83,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-22S",         "WGS72-TBE/b",          "WGS72be/b.UTM-22S",             0,   0   },  // ok
	{ 84,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-23S",         "WGS72-TBE/b",          "WGS72be/b.UTM-23S",             0,   0   },  // ok
	{ 85,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-24S",         "WGS72-TBE/b",          "WGS72be/b.UTM-24S",             0,   0   },  // ok
	{ 86,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-25S",         "WGS72-TBE/b",          "WGS72be/b.UTM-25S",             0,   0   },  // ok
	{ 87,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-26S",         "WGS72-TBE/b",          "WGS72be/b.UTM-26S",             0,   0   },  // ok
	{ 88,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-27S",         "WGS72-TBE/b",          "WGS72be/b.UTM-27S",             0,   0   },  // ok
	{ 89,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-28S",         "WGS72-TBE/b",          "WGS72be/b.UTM-28S",             0,   0   },  // ok
	{ 90,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-29S",         "WGS72-TBE/b",          "WGS72be/b.UTM-29S",             0,   0   },  // ok
	{ 91,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-30S",         "WGS72-TBE/b",          "WGS72be/b.UTM-30S",             0,   0   },  // ok
	{ 92,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-31S",         "WGS72-TBE/b",          "WGS72be/b.UTM-31S",             0,   0   },  // ok
	{ 93,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-32S",         "WGS72-TBE/b",          "WGS72be/b.UTM-32S",             0,   0   },  // ok
	{ 94,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-33S",         "WGS72-TBE/b",          "WGS72be/b.UTM-33S",             0,   0   },  // ok
	{ 95,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-34S",         "WGS72-TBE/b",          "WGS72be/b.UTM-34S",             0,   0   },  // ok
	{ 96,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-35S",         "WGS72-TBE/b",          "WGS72be/b.UTM-35S",             0,   0   },  // ok
	{ 97,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-36S",         "WGS72-TBE/b",          "WGS72be/b.UTM-36S",             0,   0   },  // ok
	{ 98,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-37S",         "WGS72-TBE/b",          "WGS72be/b.UTM-37S",             0,   0   },  // ok
	{ 99,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-38S",         "WGS72-TBE/b",          "WGS72be/b.UTM-38S",             0,   0   },  // ok
	{100,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-39S",         "WGS72-TBE/b",          "WGS72be/b.UTM-39S",             0,   0   },  // ok
	{101,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-40S",         "WGS72-TBE/b",          "WGS72be/b.UTM-40S",             0,   0   },  // ok
	{102,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-41S",         "WGS72-TBE/b",          "WGS72be/b.UTM-41S",             0,   0   },  // ok
	{103,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-42S",         "WGS72-TBE/b",          "WGS72be/b.UTM-42S",             0,   0   },  // ok
	{104,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-43S",         "WGS72-TBE/b",          "WGS72be/b.UTM-43S",             0,   0   },  // ok
	{105,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-44S",         "WGS72-TBE/b",          "WGS72be/b.UTM-44S",             0,   0   },  // ok
	{106,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-45S",         "WGS72-TBE/b",          "WGS72be/b.UTM-45S",             0,   0   },  // ok
	{107,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-46S",         "WGS72-TBE/b",          "WGS72be/b.UTM-46S",             0,   0   },  // ok
	{108,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-47S",         "WGS72-TBE/b",          "WGS72be/b.UTM-47S",             0,   0   },  // ok
	{109,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-48S",         "WGS72-TBE/b",          "WGS72be/b.UTM-48S",             0,   0   },  // ok
	{110,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-49S",         "WGS72-TBE/b",          "WGS72be/b.UTM-49S",             0,   0   },  // ok
	{111,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-50S",         "WGS72-TBE/b",          "WGS72be/b.UTM-50S",             0,   0   },  // ok
	{112,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-51S",         "WGS72-TBE/b",          "WGS72be/b.UTM-51S",             0,   0   },  // ok
	{113,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-52S",         "WGS72-TBE/b",          "WGS72be/b.UTM-52S",             0,   0   },  // ok
	{114,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-53S",         "WGS72-TBE/b",          "WGS72be/b.UTM-53S",             0,   0   },  // ok
	{115,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-54S",         "WGS72-TBE/b",          "WGS72be/b.UTM-54S",             0,   0   },  // ok
	{116,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-55S",         "WGS72-TBE/b",          "WGS72be/b.UTM-55S",             0,   0   },  // ok
	{117,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-56S",         "WGS72-TBE/b",          "WGS72be/b.UTM-56S",             0,   0   },  // ok
	{118,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-57S",         "WGS72-TBE/b",          "WGS72be/b.UTM-57S",             0,   0   },  // ok
	{119,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-58S",         "WGS72-TBE/b",          "WGS72be/b.UTM-58S",             0,   0   },  // ok
	{120,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-59S",         "WGS72-TBE/b",          "WGS72be/b.UTM-59S",             0,   0   },  // ok
	{121,     cs7pActnRevert, "WGS72-TBE/a",          "WGS72be/a.UTM-60S",         "WGS72-TBE/b",          "WGS72be/b.UTM-60S",             0,   0   },  // ok
	{  0,     cs7pActnRevert, "HongKong80a",          "HongKong80a.GridSystem",    "HongKong80b",          "HongKong80b.GridSystem",        0,   0   },  // ok
	{  1,     cs7pActnRevert, "HongKong80a",          "HongKong80a.LL",            "HongKong80b",          "HongKong80b.LL",                0,   0   },  // ok
	{  0,     cs7pActnRevert, "QatarNtl95a",          "QND95a.QatarNational",      "QatarNtl95b",          "QND95b.QatarNational",          0,   0   },  // ok
	{  1,     cs7pActnRevert, "QatarNtl95a",          "QatarNtl95a.LL",            "QatarNtl95b",          "QatarNtl95b.LL",                0,   0   },  // ok
	{  0,  cs7pActnFlipSigns, "IGN53/Mare",           "IGN53/Mare.LL",             "IGN53/Mare.a",         "IGN53/Mare.a.LL",            1928,   1   },  // ok
	{  1,  cs7pActnFlipSigns, "IGN53/Mare",           "IGN53/Mare.UTM-58S",        "IGN53/Mare.a",         "IGN53/Mare.a.UTM-58S",       1928,   1   },  // ok
	{  2,  cs7pActnFlipSigns, "IGN53/Mare",           "IGN53/Mare.UTM-59S",        "IGN53/Mare.a",         "IGN53/Mare.a.UTM-59S",       1928,   1   },  // ok
	{  0,     cs7pActnRevert, "ST71Belep/a",          "ST71Belep/a.UTM-58S",       "ST71Belep/b",          "ST71Belep/b.UTM-58S",           0,   0   },  // ok
	{  1,     cs7pActnRevert, "ST71Belep/a",          "ST71Belep/a.LL",            "ST71Belep/b",          "ST71Belep/b.LL",                0,   0   },  // ok
	{  0,  cs7pActnFlipSigns, "Helle1954",            "Helle1954.LL",              "Helle1954a",           "Helle1954a.LL",                 0,   0   },  // ok
	{  1,  cs7pActnFlipSigns, "Helle1954",            "Helle1954.JanMayen",        "Helle1954a",           "Helle1954a.JanMayen",           0,   0   },  // ok
	{  0,  cs7pActnFlipSigns, "Chatham1979",          "Chatham1979.LL",            "Chatham1979a",         "Chatham1979a.LL",               0,   0   },  // ok
	{  0,  cs7pActnFlipSigns, "PRS92/02",             "Philippine1992.LL",         "PRS92/03",             "PRS92/03.LL",                   0,   0   },  // ok
	{  1,  cs7pActnFlipSigns, "PRS92/02",             "PRS92.Philippines-1",       "PRS92/03",             "PRS92/03.Philippines-1",        0,   0   },  // ok
	{  2,  cs7pActnFlipSigns, "PRS92/02",             "PRS92.Philippines-2",       "PRS92/03",             "PRS92/03.Philippines-2",        0,   0   },  // ok
	{  3,  cs7pActnFlipSigns, "PRS92/02",             "PRS92.Philippines-3",       "PRS92/03",             "PRS92/03.Philippines-3",        0,   0   },  // ok
	{  4,  cs7pActnFlipSigns, "PRS92/02",             "PRS92.Philippines-4",       "PRS92/03",             "PRS92/03.Philippines-4",        0,   0   },  // ok
	{  5,  cs7pActnFlipSigns, "PRS92/02",             "PRS92.Philippines-5",       "PRS92/03",             "PRS92/03.Philippines-5",        0,   0   },  // ok
	{  0,  cs7pActnFlipSigns, "RGP-Francaise",        "RGP-Francaise.LL",          "RGP-Francaise/a",      "RGP-Francaise/a.LL",            0,   0   },  // ok
	{  1,  cs7pActnFlipSigns, "RGP-Francaise",        "RGP-Francaise.UTM-5S",      "RGP-Francaise/a",      "RGP-Francaise/a.UTM-5S",        0,   0   },  // ok
	{  2,  cs7pActnFlipSigns, "RGP-Francaise",        "RGP-Francaise.UTM-6S",      "RGP-Francaise/a",      "RGP-Francaise/a.UTM-6S",        0,   0   },  // ok
	{  3,  cs7pActnFlipSigns, "RGP-Francaise",        "RGP-Francaise.UTM-7S",      "RGP-Francaise/a",      "RGP-Francaise/a.UTM-7S",        0,   0   },  // ok
	{  4,  cs7pActnFlipSigns, "RGP-Francaise",        "RGP-Francaise.UTM-8S",      "RGP-Francaise/a",      "RGP-Francaise/a.UTM-8S",        0,   0   },  // ok
	{  0,  cs7pActnFlipSigns, "FatuIva/72",           "FatuIva/72.LL",             "FatuIva/72a",          "FatuIva/72a.LL",                0,   0   },  // ok
	{  0,  cs7pActnFlipSigns, "IGN63/Hiva Oa",        "IGN63/Hiva Oa.LL",          "IGN63/Hiva Oa/a",      "IGN63/Hiva Oa/a.LL",            0,   0   },  // ok
	{  0,  cs7pActnFlipSigns, "Moorea87",             "Moorea87.LL",               "Moorea87a",            "Moorea87a.LL",                  0,   0   },  // ok
	{  0,  cs7pActnFlipSigns, "Fiji1986",             "Fiji1986.LL",               "Fiji1986a",            "Fiji1986a.LL",                  0,   0   },  // ok
	{  1,  cs7pActnFlipSigns, "Fiji1986",             "Fiji1956.FijiMapGrid",      "Fiji1986a",            "Fiji1986a.FijiMapGrid",         0,   0   },  // ok
	{  0,  cs7pActnFlipSigns, "ParametropZemp1990",   "ParametropZemp1990.LL",     "ParametropZemp1990a",  "ParametropZemp1990a.LL",        0,   0   },  // ok
	{  0,  cs7pActnFlipSigns, "MGI-AT/F",             "MGI-AT/F.LL/a",             "MGI-AT/Fa",            "MGI-AT/Fa.LL/a",                0,   0   },  // ok
	{  1,  cs7pActnFlipSigns, "MGI-AT/F",             "MGI-AT/F.AUT-West/GK",      "MGI-AT/Fa",            "MGI-AT/Fa.AUT-West/GK",         0,   0   },  // ok
	{  2,  cs7pActnFlipSigns, "MGI-AT/F",             "MGI-AT/F.AUT-Central/GK",   "MGI-AT/Fa",            "MGI-AT/Fa.AUT-Cntrl/GK",        0,   0   },  // 24
	{  3,  cs7pActnFlipSigns, "MGI-AT/F",             "MGI-AT/F.AUT-East/GK",      "MGI-AT/Fa",            "MGI-AT/Fa.AUT-East/GK",         0,   0   },  // ok
	{  4,  cs7pActnFlipSigns, "MGI-AT/F",             "MGI-AT/F.AUT-West",         "MGI-AT/Fa",            "MGI-AT/Fa.AUT-West",            0,   0   },  // ok
	{  5,  cs7pActnFlipSigns, "MGI-AT/F",             "MGI-AT/F.AUT-Central",      "MGI-AT/Fa",            "MGI-AT/Fa.AUT-Cntrl",           0,   0   },  // ok
	{  6,  cs7pActnFlipSigns, "MGI-AT/F",             "MGI-AT/F.AUT-East",         "MGI-AT/Fa",            "MGI-AT/Fa.AUT-East",            0,   0   },  // ok
	{  7,  cs7pActnFlipSigns, "MGI-AT/F",             "MGI-AT/F.M28",              "MGI-AT/Fa",            "MGI-AT/Fa.M28",                 0,   0   },  // ok
	{  8,  cs7pActnFlipSigns, "MGI-AT/F",             "MGI-AT/F.M31",              "MGI-AT/Fa",            "MGI-AT/Fa.M31",                 0,   0   },  // ok
	{  9,  cs7pActnFlipSigns, "MGI-AT/F",             "MGI-AT/F.M34",              "MGI-AT/Fa",            "MGI-AT/Fa.M34",                 0,   0   },  // ok
//	{ 10,  cs7pActnFlipSigns, "MGI-AT/F",             "MGI-AT/F.LL",               "MGI-AT/Fa",            "MGI-AT/Fa.LL",                  0,   0   },  // Already deprecated
	{  0,       cs7pActnNone, "",                     "",                          "",                     "",                              0,   0   }   // end of table marker
};

bool SevenParameterRevert (const Tcs7ParmFixNames* tblPtr);
bool SevenParameterFlip (const Tcs7ParmFixNames* tblPtr);
bool SevenParameterUpdate (const Tcs7ParmFixNames* tblPtr);
bool SevenParameterRestore (const Tcs7ParmFixNames* tblPtr);

bool csDatumFlip7P (const char* newDtmName,const char* dtmName1986);
bool csXformFlip7P (const char* newDtmName,const char* dtmName1986);
bool csCrsFlip7P (const char* newCrsName,const char* newDtmName,const char* crsName1986);

bool csDatumUpdate7P (const char* toBeRestored,const char* newDef);
bool csXformUpdate7P (const char* toBeRestored,const char* newDef);
bool csCoordsysUpdate7P (const char* toBeRestored,const char* newDef);

bool csDatumRestore7P (const char* toBeRestored);
bool csXformRestore7P (const char* toBeRestored);
bool csCoordsysRestore7P (const char* toBeRestored);

bool RotationFlip (TcsAscDefinition* defPtr,const char* rotLabel);


// We make the following global variables for convenience.

TcsDefFile VcsDatumsAsc (dictTypDatum);
TcsDefFile VcsXformAsc (dictTypXform);
TcsDefFile VcsCoordsysAsc (dictTypCoordsys);
TcsCategoryFile VcsCategoryAsc; 

TcsNameMapper VcsNameMapper;

bool SevenParameterFix (const wchar_t* csDictTrgDir,const wchar_t* csDataTrgDir,const wchar_t* csDictSrcDir,const wchar_t* csDataSrcDir)
{
	bool ok (true);

	const Tcs7ParmFixNames* tblPtr;

	char srcPathName [512];
	char srcDataPath [512];
	char trgPathName [512];
	char trgDataPath [512];
	char categoryPathName [512];
	char datumPathName [512];
	char transformPathName [512];
	char coordsysPathName [512];
	//char datumCsvPathName [512];
	//char projectiveCsvPathName [512];
	//char geographicCsvPathName [512];
	char nameMapperCsvPathName [512];

	std::wifstream winStrm;
	std::wofstream woutStrm;

	TcsCsvStatus csvStatus;

	wcstombs (srcPathName,csDictSrcDir,sizeof (srcPathName));
	wcstombs (srcDataPath,csDataSrcDir,sizeof (srcDataPath));
	wcstombs (trgPathName,csDictTrgDir,sizeof (trgPathName));
	wcstombs (trgDataPath,csDataTrgDir,sizeof (trgDataPath));

	CS_stncp (categoryPathName,srcPathName,sizeof (categoryPathName));
	CS_stncat (categoryPathName,"\\Category.asc",sizeof (categoryPathName));
	VcsCategoryAsc.InitializeFromFile (categoryPathName);

	CS_stncp (datumPathName,srcPathName,sizeof (datumPathName));
	CS_stncat (datumPathName,"\\Datums.asc",sizeof (datumPathName));
	VcsDatumsAsc.InitializeFromFile (datumPathName);

	CS_stncp (transformPathName,srcPathName,sizeof (transformPathName));
	CS_stncat (transformPathName,"\\GeodeticTransformation.asc",sizeof (transformPathName));
	VcsXformAsc.InitializeFromFile (transformPathName);

	CS_stncp (coordsysPathName,srcPathName,sizeof (coordsysPathName));
	CS_stncat (coordsysPathName,"\\coordsys.asc",sizeof (coordsysPathName));
	VcsCoordsysAsc.InitializeFromFile (coordsysPathName);

	// Populate the name mapper with the current data file.
	CS_stncp (nameMapperCsvPathName,srcPathName,sizeof (nameMapperCsvPathName));
	strcat (nameMapperCsvPathName,"\\NameMapper.csv");
	winStrm.open (nameMapperCsvPathName,std::ios_base::in);
	ok = winStrm.is_open ();
	if (ok)
	{
		EcsCsvStatus csvStatus;
		csvStatus = VcsNameMapper.ReadFromStream (winStrm);
		winStrm.close ();
		ok = (csvStatus == csvOk);
		if (ok)
		{
			ok = VcsNameMapper.IsInitialized ();
		}
	}

	// For each entry in the table:
	for (tblPtr = Kcs7ParmFixNames;ok && tblPtr->action != cs7pActnNone;tblPtr += 1)
	{
		switch (tblPtr->action) {
		
		case cs7pActnRevert:
			ok = SevenParameterRevert (tblPtr);
			break;
		case cs7pActnFlipSigns:
			ok = SevenParameterFlip (tblPtr);
			break;
		case cs7pActnUpdate:
			ok = SevenParameterUpdate (tblPtr);
			break;
		case cs7pActnRestore:
			ok = SevenParameterRestore (tblPtr);
			break;
		case cs7pActnError:
			ok = false;
			break;
		case cs7pActnNone:
		default:
			ok = true;
			break;
		}
	}

	// If all is OK, we need to write out the results.  Nothing complicated,
	// but rather laborious.
	if (ok)
	{
		CS_stncp (categoryPathName,trgPathName,sizeof (categoryPathName));
		CS_stncat (categoryPathName,"\\category.asc",sizeof (categoryPathName));
		ok = VcsCategoryAsc.WriteToFile (categoryPathName);
	}

	if (ok)
	{
		CS_stncp (datumPathName,trgPathName,sizeof (datumPathName));
		CS_stncat (datumPathName,"\\Datums.asc",sizeof (datumPathName));
		ok = VcsDatumsAsc.WriteToFile (datumPathName);
	}

	if (ok)
	{
		CS_stncp (transformPathName,trgPathName,sizeof (transformPathName));
		CS_stncat (transformPathName,"\\GeodeticTransformation.asc",sizeof (transformPathName));
		ok = VcsXformAsc.WriteToFile (transformPathName);
	}

	if (ok)
	{
		CS_stncp (coordsysPathName,trgPathName,sizeof (coordsysPathName));
		CS_stncat (coordsysPathName,"\\coordsys.asc",sizeof (coordsysPathName));
		VcsCoordsysAsc.WriteToFile (coordsysPathName);
	}

	if (ok)
	{
		CS_stncp (nameMapperCsvPathName,trgDataPath,sizeof (nameMapperCsvPathName));
		strcat (nameMapperCsvPathName,"\\NameMapper.csv");
		woutStrm.open (nameMapperCsvPathName,std::ios_base::out | std::ios_base::trunc);
 		ok = woutStrm.is_open ();
		if (ok)
		{
			VcsNameMapper.WriteAsCsv (woutStrm,true);
			woutStrm.close ();
		}
	}
	
	return ok;
}
bool SevenParameterRevert (const Tcs7ParmFixNames* tblPtr)
{
	bool ok (false);

	// Originally, the idea here was to revert to the previously existing definitions which
	// were correct but deprecated by the EPSG stnch project.  However, this got rather dicey
	// confidence in the robustness oif the result waned.  Thus, we now somply replace the
	// existing definition with a new definition which has had the signs flipped.  Esentially,
	// this means that the operation becomes the same as the Flip operation.
	
	ok = SevenParameterFlip (tblPtr);
	return ok;
}
bool SevenParameterFlip (const Tcs7ParmFixNames* tblPtr)
{
	bool ok (true);

	// If the dtIndex element is zero, we edit the datum and geodetic transformation
	// dictionaries. This prevents us from doing that operation more than once.  This
	// operation includes dinking with the datum name mapper file.  AT this point
	// in time, we do not have a transformation name mapping file.
	
	// In any case, we then perform the edit on the coordinate system dictionary, the
	// category dictionary, and then either the projective name mapping file or the
	// geographic name mapping file as is appropriate.

	// Ok.  Lets get started with the datum dictionary and the geodetic transformation
	// dictionary.
	if (tblPtr->dtIndex == 0)
	{	
		// This datum flip function also deals with the datum name mapping file.
		ok = csDatumFlip7P (tblPtr->newDtmName,tblPtr->dtmName1986);
		// The Geodetic Transformations
		if (ok)
		{
			ok = csXformFlip7P (tblPtr->newDtmName,tblPtr->dtmName1986);
		}
	}

	// The coordinate system dictionary.  Need a new definition with the new datum
	// reference in it.  This function also twiddles the appropriate name mapping
	// data files as well as the category dictionary.
	if (ok)
	{
		ok = csCrsFlip7P (tblPtr->newCrsName,tblPtr->newDtmName,tblPtr->crsName1986);
	}
	return ok;
}
bool SevenParameterUpdate (const Tcs7ParmFixNames* tblPtr)
{
	bool ok (false);
	
	// Revert the datums.
	ok = csDatumUpdate7P (tblPtr->newDtmName,tblPtr->dtmName1986);
	
	// The Geodetic Transformations
	if (ok)
	{
		ok = csXformUpdate7P (tblPtr->newDtmName,tblPtr->dtmName1986);
	}

	// Finally, the coordinate systems.
	if (ok)
	{
		ok = csCoordsysUpdate7P (tblPtr->newDtmName,tblPtr->dtmName1986);
	}

	return ok;
}
bool SevenParameterRestore (const Tcs7ParmFixNames* tblPtr)
{
	bool ok (false);
	
	// Revert the datums.
	ok = csDatumRestore7P (tblPtr->dtmName1986);
	
	// The Geodetic Transformations
	if (ok)
	{
		ok = csXformRestore7P (tblPtr->dtmName1986);
	}

	// Finally, the coordinate systems.
	if (ok)
	{
		ok = csCoordsysRestore7P (tblPtr->dtmName1986);
	}
	return ok;
}

bool csDatumFlip7P (const char* newDtmName,const char* dtmName1986)
{
	bool ok;

	// We use two definitions here.  modified DefPtr points to the definition
	// in the dictionary which is the current definition which we will modify.
	// defOriginal is a complete copy of the original definition which we will
	// deprecate by changing the group and description elements, and then
	// append to the end of the dictionary. 

	TcsAscDefinition* modifiedDefPtr (0);	// New definition which is tobecome active with new name.
	TcsAscDefinition defOriginal;			// Original definitions which is to be deprecated.

	// We get two copies of the definition we are to manipulate.
	ok = VcsDatumsAsc.ExtractDefinition (defOriginal,dtmName1986);
	if (ok)
	{
		modifiedDefPtr = VcsDatumsAsc.GetDefinition (dtmName1986);
		ok = (modifiedDefPtr != 0);
	}

	// Make the desired modifications to the current definition.
	if (ok)
	{
		ok = modifiedDefPtr->RenameDef (newDtmName);
	}
	if (ok)
	{
		// Flip the sign of the rotation values.
		ok = RotationFlip (modifiedDefPtr,"ROT_X:");
		if (ok) RotationFlip (modifiedDefPtr,"ROT_Y:");
		if (ok) RotationFlip (modifiedDefPtr,"ROT_Z:");
	}

	// Deprecate the old definition definition.
	if (ok)
	{
		char chrBufr [512];

		defOriginal.SetValue ("GROUP:","LEGACY");
		sprintf (chrBufr,"Deprecated (rotation sign), replaced by %s.",newDtmName);
		defOriginal.SetValue ("DESC_NM:",chrBufr);

		// Append the deprecated definition to the end of the dictionary file.
		ok = VcsDatumsAsc.Append (defOriginal);
	}

	// OK, need to fix up the name mapping file.
	if (ok)
	{
		wchar_t wDtmName1986 [256];
		wchar_t wNewDtmName  [256];
		
		TcsNameMap extractedNameMap;

		mbstowcs (wDtmName1986,dtmName1986,wcCount (wDtmName1986));
		mbstowcs (wNewDtmName,newDtmName,wcCount (wNewDtmName));

		ok = VcsNameMapper.ExtractAndRemove (extractedNameMap,csMapDatumKeyName,
															  csMapFlvrAutodesk,
															  wDtmName1986);
		if (ok)
		{
			
			extractedNameMap.SetNameId (wNewDtmName);
			ok = VcsNameMapper.Add (extractedNameMap);
		}
		else
		{
			// There was no entry for the old name in the name mapper.  We could
			// add the new name, but what would we map it to?  Perhaps just having
			// the name in the mapper is sufficient?  Lets try it and see how it
			// works out.
			TcsNameMap newNameMap (csMapDatumKeyName,csMapFlvrAutodesk,0UL,wNewDtmName);
			ok = VcsNameMapper.Add (newNameMap);
		}
	}
	return ok;
}
bool csXformFlip7P (const char* newDtmName,const char* dtmName1986)
{
	bool ok;
	char originalXfrmName [512];
	char newXfrmName [512];
	char chrBufr [512];
	
	TcsAscDefinition* modifiedDefPtr (0);	// New definition which is to become active with new name.
	TcsAscDefinition defOriginal;			// Original definitions which is to be deprecated.

	sprintf (originalXfrmName,"%s_to_WGS84",dtmName1986);
	sprintf (newXfrmName,"%s_to_WGS84",newDtmName);

	ok = VcsXformAsc.ExtractDefinition (defOriginal,originalXfrmName);
	if (ok)
	{
		modifiedDefPtr = VcsXformAsc.GetDefinition (originalXfrmName);
		ok = (modifiedDefPtr != 0);
	}

	// Make the desired modifications to the new definition.
	if (ok)
	{
		modifiedDefPtr->RenameDef (newXfrmName);
	}
	
	// Change the source datum name.
	if (ok)
	{
		ok = modifiedDefPtr->SetValue ("SRC_DTM:",newDtmName);
	}
	if (ok)
	{
		// Flip the sign of the rotation values.
		ok = RotationFlip (modifiedDefPtr,"ROT_X:");		
		if (ok) ok = RotationFlip (modifiedDefPtr,"ROT_Y:");		
		if (ok) ok = RotationFlip (modifiedDefPtr,"ROT_Z:");
	}

	// Deprecate the old definition.
	if (ok)
	{
		ok = defOriginal.SetValue ("GROUP:","LEGACY");
		if (!ok)
		{
			TcsDefLine newGroupLine (dictTypXform,"GROUP:","LEGACY",0);
			ok = defOriginal.InsertAfter ("TRG_DTM:",newGroupLine);
		}
		if (ok)
		{
			sprintf (chrBufr,"Deprecated (rotations); replaced by %s.",newXfrmName);
			ok = defOriginal.SetValue ("DESC_NM:",chrBufr);
		}

		// Append the deprecated definition to the end of the dictionary file.
		if (ok)
		{
			ok = VcsXformAsc.Append (defOriginal);
		}
		
		// As of yet, there are no name mappings for geodetic transformations.
	}
	return ok;
}
bool csCrsFlip7P (const char* newCrsName,const char* newDtmName,const char* crsName1986)
{
	bool ok (true);
	char chrBufr [512];

	TcsAscDefinition* modifiedDefPtr (0);	// New definition which is to become active with new name.
	TcsAscDefinition defOriginal;			// Original definitions which is to be deprecated.

	// Here to correct the CRS definition.  That is:
	// 1> Get a pointer to the current definition.
	// 2> Get a copy of the current definition.
	// 3> Change the name of the current definition.
	// 4> Change the datum referenced by the current definition.
	// 5> Change the group on the copy of the original.
	// 6> Change the description on the copy of the original.
	// 7> Append the decorecated copy of the original to the end of the dictionary.
	// 8> Locate the categorydictionary entry for this definition.
	// 9> Make a copy of this category definition.
	//10> Change the name of this definition in the category dictionary.
	//11> Change the description of the original category item copy.
	//12> Append the modified copy to the OBSOLETE category.
	//13> Locate the name mapper entry for this definition in the name mapper object.
	//14> Replace the previous CRS name with the new CRS name in the name mapper object.
	//15> Smoke 'em if you've got 'em.

	ok = VcsCoordsysAsc.ExtractDefinition (defOriginal,crsName1986);
	if (ok)
	{
		modifiedDefPtr = VcsCoordsysAsc.GetDefinition (crsName1986);
		ok = (modifiedDefPtr != 0);
	}
	if (ok)
	{
	
		modifiedDefPtr->RenameDef (newCrsName);
	}
	if (ok)
	{
		ok = modifiedDefPtr->SetValue ("DT_NAME:",newDtmName);
	}
	if (ok)
	{
		ok = defOriginal.SetValue ("GROUP:","LEGACY");
		if (!ok)
		{
			TcsDefLine newGroupLine (dictTypXform,"GROUP:","LEGACY",0);
			ok = defOriginal.InsertBefore ("DESC_NM:",newGroupLine);
		}
		if (ok)
		{
			sprintf (chrBufr,"Deprecated (datum); replaced by %s.",newCrsName);
			ok = defOriginal.SetValue ("DESC_NM:",chrBufr);
		}
		if (ok)
		{
			ok = VcsCoordsysAsc.Append (defOriginal);
		}
	}
	
	// Deal with the name mapper.  There are two possibilities here.  If the
	// definition is a geographic coordinate system (i.e. Lat/Long), we need to
	// work with a Geographic name mapping entry.  Otherwise, we work with the
	// Projective name mapping entry.
	if (ok)
	{
		wchar_t wCrsName1986 [256];
		wchar_t wNewCrsName  [256];
		EcsMapObjType objType;
		TcsNameMap extractedNameMap;

		mbstowcs (wCrsName1986,crsName1986,wcCount (wCrsName1986));
		mbstowcs (wNewCrsName,newCrsName,wcCount (wNewCrsName));

		const char* projCode = modifiedDefPtr->GetValue ("PROJ:");
		objType = (CS_stricmp (projCode,"LL")) ?  csMapProjectedCSysKeyName : csMapGeographicCSysKeyName;

		ok = VcsNameMapper.ExtractAndRemove (extractedNameMap,objType,csMapFlvrAutodesk,
																	  wCrsName1986);
		if (ok)
		{
			extractedNameMap.SetNameId (wNewCrsName);
			ok = VcsNameMapper.Add (extractedNameMap);
		}
		else
		{
			// There was no entry for the old name in the name mapper.  We could
			// add the new name, but what would we map it to?  Perhaps just having
			// the name in the mapper is sufficient?  Lets try it and see how it
			// works out.
			TcsNameMap newNameMap (objType,csMapFlvrAutodesk,0UL,wNewCrsName);
			ok = VcsNameMapper.Add (newNameMap);
		}
	}

	// OK, need to deal with the category file now.
	if (ok)
	{
		bool changeOk (false);
		bool obsoleteOk (false);

		size_t categoryIdx;
		size_t categoryCount;
		TcsCategory* categoryPtr;
		TcsCategoryItem* catItemPtr;

		sprintf (chrBufr,"Deprecated, new datum reference; replaced by %s.",newCrsName);
		TcsCategoryItem obsoleteItem (crsName1986,chrBufr);

		ok = false;
		categoryCount = VcsCategoryAsc.GetCategoryCount ();
		for (categoryIdx = 0;categoryIdx < categoryCount;categoryIdx += 1)
		{
			categoryPtr = VcsCategoryAsc.FetchCategory (categoryIdx);
			if (categoryPtr != 0)
			{
				if (CS_stricmp (categoryPtr->GetCategoryName (),"Obsolete Coordinate Systems"))
				{
					catItemPtr = categoryPtr->GetItem (crsName1986);
					if (catItemPtr != 0)
					{
						catItemPtr->SetItemName (newCrsName);
						changeOk = true;
					}
				}
				else
				{
					categoryPtr->AddItem (obsoleteItem);
					obsoleteOk = true;
				}
			}
		}
		ok = changeOk & obsoleteOk;
	}
	return ok;
}

bool csDatumUpdate7P (const char* toBeRestored,const char* newDef)
{
	bool ok (true);

	return ok;
}
bool csXformUpdate7P (const char* toBeRestored,const char* newDef)
{
	bool ok (true);

	return ok;
}
bool csCoordsysUpdate7P (const char* toBeRestored,const char* newDef)
{
	bool ok (true);

	return ok;
}

bool csDatumRestore7P (const char* toBeRestored)
{
	bool ok (true);

	return ok;
}
bool csXformRestore7P (const char* toBeRestored)
{
	bool ok (true);

	return ok;
}
bool csCoordsysRestore7P (const char* toBeRestored)
{
	bool ok (true);

	return ok;
}
bool RotationFlip (TcsAscDefinition* defPtr,const char* rotLabel)
{
	bool ok (false);

	long32_t format;
	double dblTmp;
	
	const char* valuePtr;
	TcsDefLine* linePtr;

	char rotValue [128];

	// Flip the sign of a definition parameter value, usually used for rotation values.

	linePtr = defPtr->GetLine (rotLabel);
	if (linePtr != 0)
	{
		ok = true;
		dblTmp = linePtr->GetValueAsDouble (format);
		if (fabs (dblTmp) > 1.0E-07)
		{
			valuePtr = linePtr->GetValue ();
			if (*valuePtr == '-')
			{
				CS_stncp (rotValue,(valuePtr + 1),sizeof (rotValue));
			}
			else if (*valuePtr == '+')
			{
				CS_stncp (rotValue,valuePtr,sizeof (rotValue));
				rotValue [0] = '-';
			}
			else
			{
				rotValue [0] = '-';
				rotValue [1] = '\0';
				CS_stncat (rotValue,valuePtr,sizeof (rotValue));
			}
			linePtr->SetValue (rotValue);
		}
	}
	return ok;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//bool csDatumRevert7P (TcsDefFile& datumsAsc,const char* toBeRestored,const char* toBeDeprecated)
//{
//	bool ok;
//	TcsDefLine* linePtr;
//	char chrBufr [512];
//
//	TcsAscDefinition defToBeRestored;		// What was deprecated, is now to become the valid definition.
//	TcsAscDefinition defToBeDeprecated;		// What is currently active, and to become the deprecated definition.
//
//	ok = datumsAsc.ExtractDefinition (defToBeRestored,toBeRestored);
//	if (ok)
//	{
//		ok = datumsAsc.ExtractDefinition (defToBeDeprecated,toBeDeprecated);
//	}
//
//	// Fix up the definitions.
//	//if (ok)
//	//{
//	//	// Swap the definition names.
//	//	ok = ValueSwap (defToBeDeprecated,defToBeRestored,"DT_NAME:");
//	//}
//	if (ok)
//	{
//		// Swap the group names.
//		ok = ValueSwap (defToBeDeprecated,defToBeRestored,"GROUP:");
//	}
//	if (ok)
//	{	
//		// Twiddle the descriptions.  Swap them first, less work for me here.
//		ok = ValueSwap (defToBeDeprecated,defToBeRestored,"DESC_NM:");
//		
//		// The to be restored definition should be fine now.  Need to doctor
//		// up the toBeDeprecated message.
//		if (ok)
//		{
//			sprintf (chrBufr,"Deprecated; previously deprecated system %s has been restored.",toBeRestored);
//			linePtr = defToBeDeprecated.GetLine ("DESC_NM:");
//			ok = (linePtr != 0);
//			if (ok)
//			{
//				linePtr->SetValue (chrBufr);
//			}
//		}
//	}
//
//	// Replace the definitions. Note that we have swapped the definitions.  that is,
//	// defToBeRestored is now that which represnts the definition wqhich we want to
//	// deprecate; and vice versa.
//	if (ok)
//	{
//		size_t idxToBeRestored (0);			// Initialization to keep lint happy.
//		size_t idxToBeDeprecated (0);		// Initialization to keep lint happy.
//		
//		ok = datumsAsc.GetIndexOf (idxToBeDeprecated,toBeRestored);
//		if (ok)
//		{
//			ok = datumsAsc.GetIndexOf (idxToBeRestored,toBeDeprecated);
//		}
//		if (ok)
//		{
//			ok = datumsAsc.ReplaceAt (idxToBeRestored,defToBeRestored);
//		}
//		if (ok)
//		{
//			ok = datumsAsc.ReplaceAt (idxToBeDeprecated,defToBeDeprecated);
//		}
//	}
//	return ok;
//}
//bool csXformRevert7P (TcsDefFile& xformAsc,const char* toBeRestored,const char* toBeDeprecated)
//{
//	bool ok;
//
//	TcsDefLine* linePtr;
//
//	char xfmToBeRestored [512];
//	char xfmToBeDeprecated [512];
//	char chrBufr [512];
//
//	TcsAscDefinition defToBeRestored;		// What was deprecated, is now to be the valid definition.
//	TcsAscDefinition defToBeDeprecated;		// What is currently active, and to be deprecated.
//
//	// This is currently valid.  In future would probably need to search the
//	// dictionary looking for a SrC_DTM of the gioven name and a TRG_DTM of
//	// WGS84.
//	sprintf (xfmToBeRestored,"%s_to_WGS84",toBeRestored);
//	sprintf (xfmToBeDeprecated,"%s_to_WGS84",toBeDeprecated);
//
//	ok = xformAsc.ExtractDefinition (defToBeRestored,xfmToBeRestored);
//	if (ok)
//	{
//		ok = xformAsc.ExtractDefinition (defToBeDeprecated,xfmToBeDeprecated);
//	}
//
//	// Fix up the definitions.
//	//if (ok)
//	//{
//	//	// Swap the definition names.
//	//	ok = ValueSwap (defToBeDeprecated,defToBeRestored,"GX_NAME:");
//	//}
//	if (ok)
//	{
//		// Swap the group names.
//		ok = ValueSwap (defToBeDeprecated,defToBeRestored,"GROUP:");
//	}
//	if (ok)
//	{	
//		// Twiddle the descriptions.  Swap them first, less work for me here.
//		ok = ValueSwap (defToBeDeprecated,defToBeRestored,"DESC_NM:");
//		
//		// The to be restored definition should be fine now.  Need to doctor
//		// up the toBeDeprecated message.
//		if (ok)
//		{
//			sprintf (chrBufr,"Deprecated; originally deprecated transformation %s has been restored.",xfmToBeRestored);
//			linePtr = defToBeDeprecated.GetLine ("DESC_NM:");
//			ok = (linePtr != 0);
//			if (ok)
//			{
//				linePtr->SetValue (chrBufr);
//			}
//		}
//	}
//
//	// Replace the definitions. Note that we have swapped the definitions.  that is,
//	// defToBeRestored is now that which represnts the definition wqhich we want to
//	// deprecate; and vice versa.
//	if (ok)
//	{
//		size_t idxToBeRestored (0);			// Initialization to keep lint happy.
//		size_t idxToBeDeprecated (0);		// Initialization to keep lint happy.
//		
//		ok = xformAsc.GetIndexOf (idxToBeDeprecated,xfmToBeRestored);
//		if (ok)
//		{
//			ok = xformAsc.GetIndexOf (idxToBeRestored,xfmToBeDeprecated);
//		}
//		if (ok)
//		{
//			ok = xformAsc.ReplaceAt (idxToBeDeprecated,defToBeDeprecated);
//		}
//		if (ok)
//		{
//			ok = xformAsc.ReplaceAt (idxToBeRestored,defToBeRestored);
//		}
//	}	return ok;
//}
//bool csCoordsysRevert7P (TcsDefFile& coordsysAsc,const char* toBeRestored,const char* toBeDeprecated)
//{
//	bool ok;
//
//	size_t index;
//	size_t defCount;
//
//	const char* valuePtr;
//	const TcsDefLine* linePtr;
//	const TcsAscDefinition* defPtr;
//
//	defCount = coordsysAsc.GetDefinitionCount ();
//	ok = (defCount > 0);
//	for (index = 0;ok && index < defCount;index += 1)
//	{
//		defPtr = coordsysAsc.GetDefinition (index);
//		ok = (defPtr != 0);
//		if (defPtr->IsDefinition ())
//		{
//			linePtr = defPtr->GetLine ("DT_NAME:");
//			ok = (linePtr != 0);
//			if (ok)
//			{
//				valuePtr = linePtr->GetValue ();
//				
//				// See if there is something to do with this defionition.
//				if (!CS_stricmp (toBeRestored,valuePtr))
//				{
//					// Need to restore this definition.
//				}
//				if (!CS_stricmp (toBeDeprecated,valuePtr))
//				{
//					// Need to deprecate this definition.
//				}
//			}
//			else
//			{
//				// Some defensive prograqmming here.
//				linePtr = defPtr->GetLine ("EL_NAME:");
//				ok = (linePtr != 0);
//				if (!ok)
//				{
//					linePtr = defPtr->GetLine ("PROJ:");
//					ok = (linePtr != 0);
//					if (ok)
//					{
//						valuePtr = linePtr->GetValue ();
//						if (valuePtr != 0 &&
//							!CS_strnicmp (valuePtr,"NERTH",5))
//						{
//							ok = true;
//						}
//					}
//				}
//			}
//		}
//	}
//	return ok;
//}
