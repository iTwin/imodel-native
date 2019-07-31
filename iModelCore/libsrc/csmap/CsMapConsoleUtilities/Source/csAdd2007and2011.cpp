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

struct cs_Add2007And2011_
{
	char harnKeyName     [cs_KEYNM_DEF];
	char harnDtmName     [cs_KEYNM_DEF];
	ulong32_t harnEpsgCode;
	char nsrs07KeyName   [cs_KEYNM_DEF];
	char nsrs07Desc      [64];
	char nsrs07Source    [64];
	ulong32_t nsrs07EpsgCode;
	char nsrs11KeyName   [cs_KEYNM_DEF];
	char nsrs11Desc      [64];
	char nsrs11Source    [64];
	ulong32_t nsrs11EpsgCode;
	ulong32_t nsrs11EsriCode;			// newly added 8 Nov 2014
	wchar_t nsrs11EpsgName [128];		// EPSG database size is 80
} cs_Add2007And2011 [] =
{
	{               "HARN/AL.AL-E",    "HARN/AL",   2759,
					               "NSRS07.AL-E",  "NSRS 2007 Alabama State Planes, East Zone, Meter",                   "CRS HARN/AL.AL-E with datum modified to NSRS07",                      3465,
					               "NSRS11.AL-E",  "NSRS 2011 Alabama State Planes, East Zone, Meter",                   "CRS HARN/AL.AL-E with datum modified to NSRS11",                         0   },
	{               "HARN/AL.AL-W",    "HARN/AL",   2760,
					               "NSRS07.AL-W",  "NSRS 2007 Alabama State Planes, West Zone, Meter",                   "CRS HARN/AL.AL-W with datum modified to NSRS07",                      3466,
					               "NSRS11.AL-W",  "NSRS 2011 Alabama State Planes, West Zone, Meter",                   "CRS HARN/AL.AL-W with datum modified to NSRS11",                         0   },
	// CS-MAP had HARN definitions for the ALabama systems based onUS Foot, but no one else had HARN definitions for these in US Feet.
	//{              "HARN/AL.AL-EF",    "HARN/AL",      0,
	//				              "NSRS07.AL-EF",  "NSRS 2007 Alabama State Planes, East Zone, US Foot",                 "CRS HARN/AL.AL-EF with datum modified to NSRS07",                        0,
	//				              "NSRS11.AL-EF",  "NSRS 2011 Alabama State Planes, East Zone, US Foot",                 "CRS HARN/AL.AL-EF with datum modified to NSRS11",                        0   },
	//{              "HARN/AL.AL-WF",    "HARN/AL",      0,
	//				              "NSRS07.AL-WF",  "NSRS 2007 Alabama State Planes, West Zone, US Foot",                 "CRS HARN/AL.AL-WF with datum modified to NSRS07",                        0,
	//				              "NSRS11.AL-WF",  "NSRS 2011 Alabama State Planes, West Zone, US Foot",                 "CRS HARN/AL.AL-WF with datum modified to NSRS11",                        0   },
	{               "HARN/AZ.AZ-C",    "HARN/AZ",   2762,
					               "NSRS07.AZ-C",  "NSRS 2007 Arizona State Planes, Central Zone, Meter",                "CRS HARN/AZ.AZ-C with datum modified to NSRS07",                      3478,
					               "NSRS11.AZ-C",  "NSRS 2011 Arizona State Planes, Central Zone, Meter",                "CRS HARN/AZ.AZ-C with datum modified to NSRS11",                         0   },
	{             "HARN/AZ.AZ-CIF",    "HARN/AZ",   2868,
					             "NSRS07.AZ-CIF",  "NSRS 2007 Arizona State Planes, Central Zone, Internat'l Foot",      "CRS HARN/AZ.AZ-CIF with datum modified to NSRS07",                    3479,
					             "NSRS11.AZ-CIF",  "NSRS 2011 Arizona State Planes, Central Zone, Internat'l Foot",      "CRS HARN/AZ.AZ-CIF with datum modified to NSRS11",                       0   },
	{               "HARN/AZ.AZ-E",    "HARN/AZ",   2761,
					               "NSRS07.AZ-E",  "NSRS 2007 Arizona State Planes, East Zone, Meter",                   "CRS HARN/AZ.AZ-E with datum modified to NSRS07",                      3480,
					               "NSRS11.AZ-E",  "NSRS 2011 Arizona State Planes, East Zone, Meter",                   "CRS HARN/AZ.AZ-E with datum modified to NSRS11",                         0   },
	{             "HARN/AZ.AZ-EIF",    "HARN/AZ",   2867,
					             "NSRS07.AZ-EIF",  "NSRS 2007 Arizona State Planes, East Zone, International Foot",      "CRS HARN/AZ.AZ-EIF with datum modified to NSRS07",                    3481,
					             "NSRS11.AZ-EIF",  "NSRS 2011 Arizona State Planes, East Zone, International Foot",      "CRS HARN/AZ.AZ-EIF with datum modified to NSRS11",                       0   },
	{               "HARN/AZ.AZ-W",    "HARN/AZ",   2763,
					               "NSRS07.AZ-W",  "NSRS 2007 Arizona State Planes, West Zone, Meter",                   "CRS HARN/AZ.AZ-W with datum modified to NSRS07",                      3482,
					               "NSRS11.AZ-W",  "NSRS 2011 Arizona State Planes, West Zone, Meter",                   "CRS HARN/AZ.AZ-W with datum modified to NSRS11",                         0   },
	{             "HARN/AZ.AZ-WIF",    "HARN/AZ",   2869,
					             "NSRS07.AZ-WIF",  "NSRS 2007 Arizona State Planes, West Zone, International Foot",      "CRS HARN/AZ.AZ-WIF with datum modified to NSRS07",                    3483,
					             "NSRS11.AZ-WIF",  "NSRS 2011 Arizona State Planes, West Zone, International Foot",      "CRS HARN/AZ.AZ-WIF with datum modified to NSRS11",                       0   },
	// CS-MAP had HARN definitions for the Arizona systems based onUS Foot, but no one else had HARN definitions for these in US Feet.
	//{              "HARN/AZ.AZ-CF",    "HARN/AZ",      0,
	//				              "NSRS07.AZ-CF",  "NSRS 2007 Arizona State Planes, Central Zone, US Foot",              "CRS HARN/AZ.AZ-CF with datum modified to NSRS07",                        0,
	//				              "NSRS11.AZ-CF",  "NSRS 2011 Arizona State Planes, Central Zone, US Foot",              "CRS HARN/AZ.AZ-CF with datum modified to NSRS11",                        0   },
	//{              "HARN/AZ.AZ-EF",    "HARN/AZ",      0,
	//				              "NSRS07.AZ-EF",  "NSRS 2007 Arizona State Planes, East Zone, US Foot",                 "CRS HARN/AZ.AZ-EF with datum modified to NSRS07",                        0,
	//				              "NSRS11.AZ-EF",  "NSRS 2011 Arizona State Planes, East Zone, US Foot",                 "CRS HARN/AZ.AZ-EF with datum modified to NSRS11",                        0   },
	//{              "HARN/AZ.AZ-WF",    "HARN/AZ",      0,
	//				              "NSRS07.AZ-WF",  "NSRS 2007 Arizona State Planes, West Zone, US Foot",                 "CRS HARN/AZ.AZ-WF with datum modified to NSRS07",                        0,
	//				              "NSRS11.AZ-WF",  "NSRS 2011 Arizona State Planes, West Zone, US Foot",                 "CRS HARN/AZ.AZ-WF with datum modified to NSRS11",                        0   },
	{               "HARN/CA.CA-I",    "HARN/CA",   2766,
					               "NSRS07.CA-I",  "NSRS 2007 California State Planes, Zone I, Meter",                   "CRS HARN/CA.CA-I with datum modified to NSRS07",                      3489,
					               "NSRS11.CA-I",  "NSRS 2011 California State Planes, Zone I, Meter",                   "CRS HARN/CA.CA-I with datum modified to NSRS11",                         0   },
	{              "HARN/CA.CA-IF",    "HARN/CA",   2870,
					              "NSRS07.CA-IF",  "NSRS 2007 California State Planes, Zone I, US Foot",                 "CRS HARN/CA.CA-IF with datum modified to NSRS07",                     3490,
					              "NSRS11.CA-IF",  "NSRS 2011 California State Planes, Zone I, US Foot",                 "CRS HARN/CA.CA-IF with datum modified to NSRS11",                        0   },
	{              "HARN/CA.CA-II",    "HARN/CA",   2767,
					              "NSRS07.CA-II",  "NSRS 2007 California State Planes, Zone II, Meter",                  "CRS HARN/CA.CA-II with datum modified to NSRS07",                     3491,
					              "NSRS11.CA-II",  "NSRS 2011 California State Planes, Zone II, Meter",                  "CRS HARN/CA.CA-II with datum modified to NSRS11",                        0   },
	{             "HARN/CA.CA-IIF",    "HARN/CA",   2871,
					             "NSRS07.CA-IIF",  "NSRS 2007 California State Planes, Zone II, US Foot",                "CRS HARN/CA.CA-IIF with datum modified to NSRS07",                    3492,
					             "NSRS11.CA-IIF",  "NSRS 2011 California State Planes, Zone II, US Foot",                "CRS HARN/CA.CA-IIF with datum modified to NSRS11",                       0   },
	{             "HARN/CA.CA-III",    "HARN/CA",   2768,
					             "NSRS07.CA-III",  "NSRS 2007 California State Planes, Zone III, Meter",                 "CRS HARN/CA.CA-III with datum modified to NSRS07",                    3493,
					             "NSRS11.CA-III",  "NSRS 2011 California State Planes, Zone III, Meter",                 "CRS HARN/CA.CA-III with datum modified to NSRS11",                       0   },
	{              "HARN/CA.CA-IV",    "HARN/CA",   2769,
					              "NSRS07.CA-IV",  "NSRS 2007 California State Planes, Zone IV, Meter",                  "CRS HARN/CA.CA-IV with datum modified to NSRS07",                     3495,
					              "NSRS11.CA-IV",  "NSRS 2011 California State Planes, Zone IV, Meter",                  "CRS HARN/CA.CA-IV with datum modified to NSRS11",                        0   },
	{             "HARN/CA.CA-IVF",    "HARN/CA",   2873,
					             "NSRS07.CA-IVF",  "NSRS 2007 California State Planes, Zone IV, US Foot",                "CRS HARN/CA.CA-IVF with datum modified to NSRS07",                    3496,
					             "NSRS11.CA-IVF",  "NSRS 2011 California State Planes, Zone IV, US Foot",                "CRS HARN/CA.CA-IVF with datum modified to NSRS11",                       0   },
	{               "HARN/CA.CA-V",    "HARN/CA",   2770,
					               "NSRS07.CA-V",  "NSRS 2007 California State Planes, Zone V, Meter",                   "CRS HARN/CA.CA-V with datum modified to NSRS07",                      3497,
					               "NSRS11.CA-V",  "NSRS 2011 California State Planes, Zone V, Meter",                   "CRS HARN/CA.CA-V with datum modified to NSRS11",                         0   },
	{              "HARN/CA.CA-VF",    "HARN/CA",   2874,
					              "NSRS07.CA-VF",  "NSRS 2007 California State Planes, Zone V, US Foot",                 "CRS HARN/CA.CA-VF with datum modified to NSRS07",                     3498,
					              "NSRS11.CA-VF",  "NSRS 2011 California State Planes, Zone V, US Foot",                 "CRS HARN/CA.CA-VF with datum modified to NSRS11",                        0   },
	{             "HARN/CA.CAIIIF",    "HARN/CA",   2872,
					             "NSRS07.CAIIIF",  "NSRS 2007 California State Planes, Zone III, US Foot",               "CRS HARN/CA.CAIIIF with datum modified to NSRS07",                    3494,
					             "NSRS11.CAIIIF",  "NSRS 2011 California State Planes, Zone III, US Foot",               "CRS HARN/CA.CAIIIF with datum modified to NSRS11",                       0   },
	{               "HARN/CO.CO-C",    "HARN/CO",   2773,
					               "NSRS07.CO-C",  "NSRS 2007 Colorado State Planes, Central Zone, Meter",               "CRS HARN/CO.CO-C with datum modified to NSRS07",                      3501,
					               "NSRS11.CO-C",  "NSRS 2011 Colorado State Planes, Central Zone, Meter",               "CRS HARN/CO.CO-C with datum modified to NSRS11",                         0   },
	{               "HARN/CO.CO-N",    "HARN/CO",   2772,
					               "NSRS07.CO-N",  "NSRS 2007 Colorado State Planes, North Zone, Meter",                 "CRS HARN/CO.CO-N with datum modified to NSRS07",                      3503,
					               "NSRS11.CO-N",  "NSRS 2011 Colorado State Planes, North Zone, Meter",                 "CRS HARN/CO.CO-N with datum modified to NSRS11",                         0   },
	{              "HARN/CO.CO-NF",    "HARN/CO",   2876,
					              "NSRS07.CO-NF",  "NSRS 2007 Colorado State Planes, North Zone, US Foot",               "CRS HARN/CO.CO-NF with datum modified to NSRS07",                     3504,
					              "NSRS11.CO-NF",  "NSRS 2011 Colorado State Planes, North Zone, US Foot",               "CRS HARN/CO.CO-NF with datum modified to NSRS11",                        0   },
	{               "HARN/CO.CO-S",    "HARN/CO",   2774,
					               "NSRS07.CO-S",  "NSRS 2007 Colorado State Planes, South Zone, Meter",                 "CRS HARN/CO.CO-S with datum modified to NSRS07",                      3505,
					               "NSRS11.CO-S",  "NSRS 2011 Colorado State Planes, South Zone, Meter",                 "CRS HARN/CO.CO-S with datum modified to NSRS11",                         0   },
	{                 "HARN/NE.CT",    "HARN/NE",   2775,
					                 "NSRS07.CT",  "NSRS 2007 Connecticut State Plane Zone, Meter",                      "CRS HARN/NE.CT with datum modified to NSRS07",                        3507,
					                 "NSRS11.CT",  "NSRS 2011 Connecticut State Plane Zone, Meter",                      "CRS HARN/NE.CT with datum modified to NSRS11",                           0   },
	{                "HARN/NE.CTF",    "HARN/NE",   2879,
					                "NSRS07.CTF",  "NSRS 2007 Connecticut State Plane Zone, US Foot",                    "CRS HARN/NE.CTF with datum modified to NSRS07",                       3508,
					                "NSRS11.CTF",  "NSRS 2011 Connecticut State Plane Zone, US Foot",                    "CRS HARN/NE.CTF with datum modified to NSRS11",                          0   },
	{                 "HARN/MD.DE",    "HARN/MD",   2776,
					                 "NSRS07.DE",  "NSRS 2007 Delaware State Planes, Meter",                             "CRS HARN/MD.DE with datum modified to NSRS07",                        3509,
					                 "NSRS11.DE",  "NSRS 2011 Delaware State Planes, Meter",                             "CRS HARN/MD.DE with datum modified to NSRS11",                           0   },
	{                "HARN/MD.DEF",    "HARN/MD",   2880,
					                "NSRS07.DEF",  "NSRS 2007 Delaware State Planes, US Foot",                           "CRS HARN/MD.DEF with datum modified to NSRS07",                       3510,
					                "NSRS11.DEF",  "NSRS 2011 Delaware State Planes, US Foot",                           "CRS HARN/MD.DEF with datum modified to NSRS11",                          0   },
	{               "HARN/FL.FL-E",    "HARN/FL",   2777,
					               "NSRS07.FL-E",  "NSRS 2007 Florida State Planes, East Zone, Meter",                   "CRS HARN/FL.FL-E with datum modified to NSRS07",                      3511,
					               "NSRS11.FL-E",  "NSRS 2011 Florida State Planes, East Zone, Meter",                   "CRS HARN/FL.FL-E with datum modified to NSRS11",                         0   },
	{              "HARN/FL.FL-EF",    "HARN/FL",   2881,
					              "NSRS07.FL-EF",  "NSRS 2007 Florida State Planes, East Zone, US Foot",                 "CRS HARN/FL.FL-EF with datum modified to NSRS07",                     3512,
					              "NSRS11.FL-EF",  "NSRS 2011 Florida State Planes, East Zone, US Foot",                 "CRS HARN/FL.FL-EF with datum modified to NSRS11",                        0   },
	{               "HARN/FL.FL-N",    "HARN/FL",   2779,
					               "NSRS07.FL-N",  "NSRS 2007 Florida State Planes, North Zone, Meter",                  "CRS HARN/FL.FL-N with datum modified to NSRS07",                      3514,
					               "NSRS11.FL-N",  "NSRS 2011 Florida State Planes, North Zone, Meter",                  "CRS HARN/FL.FL-N with datum modified to NSRS11",                         0   },
	{              "HARN/FL.FL-NF",    "HARN/FL",   2883,
					              "NSRS07.FL-NF",  "NSRS 2007 Florida State Planes, North Zone, US Foot",                "CRS HARN/FL.FL-NF with datum modified to NSRS07",                     3515,
					              "NSRS11.FL-NF",  "NSRS 2011 Florida State Planes, North Zone, US Foot",                "CRS HARN/FL.FL-NF with datum modified to NSRS11",                        0   },
	{               "HARN/FL.FL-W",    "HARN/FL",   2778,
					               "NSRS07.FL-W",  "NSRS 2007 Florida State Planes, West Zone, Meter",                   "CRS HARN/FL.FL-W with datum modified to NSRS07",                      3516,
					               "NSRS11.FL-W",  "NSRS 2011 Florida State Planes, West Zone, Meter",                   "CRS HARN/FL.FL-W with datum modified to NSRS11",                         0   },
	{              "HARN/FL.FL-WF",    "HARN/FL",   2882,
					              "NSRS07.FL-WF",  "NSRS 2007 Florida State Planes, West Zone, US Foot",                 "CRS HARN/FL.FL-WF with datum modified to NSRS07",                     3517,
					              "NSRS11.FL-WF",  "NSRS 2011 Florida State Planes, West Zone, US Foot",                 "CRS HARN/FL.FL-WF with datum modified to NSRS11",                        0   },
	{               "HARN/KY.KY-N",    "HARN/KY",   2798,
					               "NSRS07.KY-N",  "NSRS 2007 Kentucky State Planes, North Zone, Meter",                 "CRS HARN/KY.KY-N with datum modified to NSRS07",                      3544,
					               "NSRS11.KY-N",  "NSRS 2011 Kentucky State Planes, North Zone, Meter",                 "CRS HARN/KY.KY-N with datum modified to NSRS11",                         0   },
	{              "HARN/KY.KY-NF",    "HARN/KY",   2891,
					              "NSRS07.KY-NF",  "NSRS 2007 Kentucky State Planes, North Zone, US Foot",               "CRS HARN/KY.KY-NF with datum modified to NSRS07",                     3545,
					              "NSRS11.KY-NF",  "NSRS 2011 Kentucky State Planes, North Zone, US Foot",               "CRS HARN/KY.KY-NF with datum modified to NSRS11",                        0   },
	{               "HARN/KY.KY-S",    "HARN/KY",   2799,
					               "NSRS07.KY-S",  "NSRS 2007 Kentucky State Planes, South Zone, Meter",                 "CRS HARN/KY.KY-S with datum modified to NSRS07",                      3548,
					               "NSRS11.KY-S",  "NSRS 2011 Kentucky State Planes, South Zone, Meter",                 "CRS HARN/KY.KY-S with datum modified to NSRS11",                         0   },
	{              "HARN/KY.KY-SF",    "HARN/KY",   2892,
					              "NSRS07.KY-SF",  "NSRS 2007 Kentucky State Planes, South Zone, US Foot",               "CRS HARN/KY.KY-SF with datum modified to NSRS07",                     3549,
					              "NSRS11.KY-SF",  "NSRS 2011 Kentucky State Planes, South Zone, US Foot",               "CRS HARN/KY.KY-SF with datum modified to NSRS11",                        0   },
	{               "HARN/LA.LA-N",    "HARN/LA",   2800,
					               "NSRS07.LA-N",  "NSRS 2007 Louisiana State Planes, North Zone, Meter",                "CRS HARN/LA.LA-N with datum modified to NSRS07",                      3550,
					               "NSRS11.LA-N",  "NSRS 2011 Louisiana State Planes, North Zone, Meter",                "CRS HARN/LA.LA-N with datum modified to NSRS11",                         0   },
	{              "HARN/LA.LA-NF",    "HARN/LA",   3456,
					              "NSRS07.LA-NF",  "NSRS 2007 Louisiana State Planes, North Zone, US Foot",              "CRS HARN/LA.LA-NF with datum modified to NSRS07",                     3551,
					              "NSRS11.LA-NF",  "NSRS 2011 Louisiana State Planes, North Zone, US Foot",              "CRS HARN/LA.LA-NF with datum modified to NSRS11",                        0   },
	// CS_MAP had Louisiana offshore for HARN, but nobody else has.
	//{               "HARN/LA.LA-O",    "HARN/LA",      0,
	//				               "NSRS07.LA-O",  "NSRS 2007 Louisiana State Planes, Offshore, Meter",                  "CRS HARN/LA.LA-O with datum modified to NSRS07",                         0,
	//				               "NSRS11.LA-O",  "NSRS 2011 Louisiana State Planes, Offshore, Meter",                  "CRS HARN/LA.LA-O with datum modified to NSRS11",                         0   },
	//{              "HARN/LA.LA-OF",    "HARN/LA",      0,
	//				              "NSRS07.LA-OF",  "NSRS 2007 Louisiana State Planes, Offshore, US Foot",                "CRS HARN/LA.LA-OF with datum modified to NSRS07",                        0,
	//				              "NSRS11.LA-OF",  "NSRS 2011 Louisiana State Planes, Offshore, US Foot",                "CRS HARN/LA.LA-OF with datum modified to NSRS11",                        0   },
	{               "HARN/LA.LA-S",    "HARN/LA",   2801,
					               "NSRS07.LA-S",  "NSRS 2007 Louisiana State Planes, South Zone, Meter",                "CRS HARN/LA.LA-S with datum modified to NSRS07",                      3552,
					               "NSRS11.LA-S",  "NSRS 2011 Louisiana State Planes, South Zone, Meter",                "CRS HARN/LA.LA-S with datum modified to NSRS11",                         0   },
	{              "HARN/LA.LA-SF",    "HARN/LA",   3457,
					              "NSRS07.LA-SF",  "NSRS 2007 Louisiana State Planes, South Zone, US Foot",              "CRS HARN/LA.LA-SF with datum modified to NSRS07",                     3553,
					              "NSRS11.LA-SF",  "NSRS 2011 Louisiana State Planes, South Zone, US Foot",              "CRS HARN/LA.LA-SF with datum modified to NSRS11",                        0   },
	{                    "LL-HPGN",       "HPGN",   4152,
					                 "NSRS07.LL",  "NSRS 2007 Geographic Coordinates (degrees/Greenwich)",               "CRS LL-HPGN with datum modified to NSRS07",                           4759,
					                 "NSRS11.LL",  "NSRS 2011 Geographic Coordinates (degrees/Greenwich)",               "CRS LL-HPGN with datum modified to NSRS11",                              0   },
	//{                 "HARN/AR.LL",    "HARN/AR",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region AR",               "CRS HARN/AR.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region AR",               "CRS HARN/AR.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/AZ.LL",    "HARN/AZ",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region AZ",               "CRS HARN/AZ.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region AZ",               "CRS HARN/AZ.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/CA.LL",    "HARN/CA",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region CA",               "CRS HARN/CA.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region CA",               "CRS HARN/CA.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/CO.LL",    "HARN/CO",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region CO",               "CRS HARN/CO.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region CO",               "CRS HARN/CO.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/MT.LL",    "HARN/MT",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region MT",               "CRS HARN/MT.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region MT",               "CRS HARN/MT.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/SA.LL",    "HARN/SA",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region SA",               "CRS HARN/SA.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region SA",               "CRS HARN/SA.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/TX.LL",    "HARN/TX",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region TX",               "CRS HARN/TX.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region TX",               "CRS HARN/TX.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/FL.LL",    "HARN/FL",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region FL",               "CRS HARN/FL.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region FL",               "CRS HARN/FL.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/GA.LL",    "HARN/GA",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region GA",               "CRS HARN/GA.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region GA",               "CRS HARN/GA.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/GU.LL",    "HARN/GU",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region GU",               "CRS HARN/GU.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region GU",               "CRS HARN/GU.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/IA.LL",    "HARN/IA",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region IA",               "CRS HARN/IA.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region IA",               "CRS HARN/IA.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/IL.LL",    "HARN/IL",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region IL",               "CRS HARN/IL.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region IL",               "CRS HARN/IL.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/IN.LL",    "HARN/IN",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region IN",               "CRS HARN/IN.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region IN",               "CRS HARN/IN.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/KS.LL",    "HARN/KS",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region KS",               "CRS HARN/KS.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region KS",               "CRS HARN/KS.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/KY.LL",    "HARN/KY",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region KY",               "CRS HARN/KY.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region KY",               "CRS HARN/KY.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/LA.LL",    "HARN/LA",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region LA",               "CRS HARN/LA.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region LA",               "CRS HARN/LA.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/MD.LL",    "HARN/MD",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region MD",               "CRS HARN/MD.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region MD",               "CRS HARN/MD.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/ME.LL",    "HARN/ME",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region ME",               "CRS HARN/ME.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region ME",               "CRS HARN/ME.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/MI.LL",    "HARN/MI",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region MI",               "CRS HARN/MI.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region MI",               "CRS HARN/MI.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/MN.LL",    "HARN/MN",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region MN",               "CRS HARN/MN.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region MN",               "CRS HARN/MN.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/MO.LL",    "HARN/MO",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region MO",               "CRS HARN/MO.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region MO",               "CRS HARN/MO.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/MS.LL",    "HARN/MS",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region MS",               "CRS HARN/MS.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region MS",               "CRS HARN/MS.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/NB.LL",    "HARN/NB",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region NB",               "CRS HARN/NB.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region NB",               "CRS HARN/NB.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/NC.LL",    "HARN/NC",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region NC",               "CRS HARN/NC.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region NC",               "CRS HARN/NC.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/ND.LL",    "HARN/ND",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region ND",               "CRS HARN/ND.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region ND",               "CRS HARN/ND.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/NE.LL",    "HARN/NE",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region NE",               "CRS HARN/NE.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region NE",               "CRS HARN/NE.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/NJ.LL",    "HARN/NJ",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region NJ",               "CRS HARN/NJ.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region NJ",               "CRS HARN/NJ.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/NM.LL",    "HARN/NM",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region NM",               "CRS HARN/NM.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region NM",               "CRS HARN/NM.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/NV.LL",    "HARN/NV",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region NV",               "CRS HARN/NV.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region NV",               "CRS HARN/NV.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/NY.LL",    "HARN/NY",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region NY",               "CRS HARN/NY.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region NY",               "CRS HARN/NY.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/OH.LL",    "HARN/OH",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region OH",               "CRS HARN/OH.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region OH",               "CRS HARN/OH.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/OK.LL",    "HARN/OK",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region OK",               "CRS HARN/OK.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region OK",               "CRS HARN/OK.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/PA.LL",    "HARN/PA",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region PA",               "CRS HARN/PA.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region PA",               "CRS HARN/PA.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/PV.LL",    "HARN/PV",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region PV",               "CRS HARN/PV.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region PV",               "CRS HARN/PV.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/SC.LL",    "HARN/SC",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region SC",               "CRS HARN/SC.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region SC",               "CRS HARN/SC.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/SD.LL",    "HARN/SD",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region SD",               "CRS HARN/SD.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region SD",               "CRS HARN/SD.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/TN.LL",    "HARN/TN",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region TN",               "CRS HARN/TN.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region TN",               "CRS HARN/TN.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/UT.LL",    "HARN/UT",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region UT",               "CRS HARN/UT.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region UT",               "CRS HARN/UT.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/VA.LL",    "HARN/VA",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region VA",               "CRS HARN/VA.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region VA",               "CRS HARN/VA.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/WI.LL",    "HARN/WI",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region WI",               "CRS HARN/WI.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region WI",               "CRS HARN/WI.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/WO.LL",    "HARN/WO",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region WO",               "CRS HARN/WO.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region WO",               "CRS HARN/WO.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/WV.LL",    "HARN/WV",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region WV",               "CRS HARN/WV.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region WV",               "CRS HARN/WV.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/WY.LL",    "HARN/WY",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region WY",               "CRS HARN/WY.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region WY",               "CRS HARN/WY.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/02.LL",    "HARN/02",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region 02",               "CRS HARN/02.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region 02",               "CRS HARN/02.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/10.LL",    "HARN/10",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region 10",               "CRS HARN/10.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region 10",               "CRS HARN/10.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/11.LL",    "HARN/11",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region 11",               "CRS HARN/11.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region 11",               "CRS HARN/11.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/12.LL",    "HARN/12",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region 12",               "CRS HARN/12.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region 12",               "CRS HARN/12.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/13.LL",    "HARN/13",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region 13",               "CRS HARN/13.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region 13",               "CRS HARN/13.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/14.LL",    "HARN/14",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region 14",               "CRS HARN/14.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region 14",               "CRS HARN/14.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/15.LL",    "HARN/15",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region 15",               "CRS HARN/15.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region 15",               "CRS HARN/15.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/16.LL",    "HARN/16",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region 16",               "CRS HARN/16.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region 16",               "CRS HARN/16.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/17.LL",    "HARN/17",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region 17",               "CRS HARN/17.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region 17",               "CRS HARN/17.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/18.LL",    "HARN/18",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region 18",               "CRS HARN/18.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region 18",               "CRS HARN/18.LL with datum modified to NSRS11",                           0   },
	//{                 "HARN/19.LL",    "HARN/19",   4152,
	//				                 "NSRS07.LL",  "HARN (aka NSRS 2007) geographic system for region 19",               "CRS HARN/19.LL with datum modified to NSRS07",                        4759,
	//				                 "NSRS11.LL",  "HARN (aka NSRS 2011) geographic system for region 19",               "CRS HARN/19.LL with datum modified to NSRS11",                           0   },
	{                 "HARN/NE.MA",    "HARN/NE",   2805,
					                 "NSRS07.MA",  "NSRS 2007 Massachusetts State Planes, Mainland Zone, Meter",         "CRS HARN/NE.MA with datum modified to NSRS07",                        3585,
					                 "NSRS11.MA",  "NSRS 2011 Massachusetts State Planes, Mainland Zone, Meter",         "CRS HARN/NE.MA with datum modified to NSRS11",                           0   },
	{              "HARN/NE.MA-IS",    "HARN/NE",   2806,
					              "NSRS07.MA-IS",  "NSRS 2007 Massachusetts State Planes, Island Zone, Meter",           "CRS HARN/NE.MA-IS with datum modified to NSRS07",                     3583,
					              "NSRS11.MA-IS",  "NSRS 2011 Massachusetts State Planes, Island Zone, Meter",           "CRS HARN/NE.MA-IS with datum modified to NSRS11",                        0   },
	{             "HARN/NE.MA-ISF",    "HARN/NE",   2895,
					             "NSRS07.MA-ISF",  "NSRS 2007 Massachusetts State Planes, Island Zone, US Foot",         "CRS HARN/NE.MA-ISF with datum modified to NSRS07",                    3584,
					             "NSRS11.MA-ISF",  "NSRS 2011 Massachusetts State Planes, Island Zone, US Foot",         "CRS HARN/NE.MA-ISF with datum modified to NSRS11",                       0   },
	{                "HARN/NE.MAF",    "HARN/NE",   2894,
					                "NSRS07.MAF",  "NSRS 2007 Massachusetts State Planes, Mainland Zone, US Foot",       "CRS HARN/NE.MAF with datum modified to NSRS07",                       3586,
					                "NSRS11.MAF",  "NSRS 2011 Massachusetts State Planes, Mainland Zone, US Foot",       "CRS HARN/NE.MAF with datum modified to NSRS11",                          0   },
	{                 "HARN/MD.MD",    "HARN/MD",   2804,
					                 "NSRS07.MD",  "NSRS 2007 Maryland State Plane Zone, Meter",                         "CRS HARN/MD.MD with datum modified to NSRS07",                        3559,
					                 "NSRS11.MD",  "NSRS 2011 Maryland State Plane Zone, Meter",                         "CRS HARN/MD.MD with datum modified to NSRS11",                           0   },
	{                "HARN/MD.MDF",    "HARN/MD",   2893,
					                "NSRS07.MDF",  "NSRS 2007 Maryland State Plane Zone, US Foot",                       "CRS HARN/MD.MDF with datum modified to NSRS07",                       3582,
					                "NSRS11.MDF",  "NSRS 2011 Maryland State Plane Zone, US Foot",                       "CRS HARN/MD.MDF with datum modified to NSRS11",                          0   },
	{               "HARN/ME.ME-E",    "HARN/ME",   2802,
					               "NSRS07.ME-E",  "NSRS 2007 Maine State Planes, East Zone, Meter",                     "CRS HARN/ME.ME-E with datum modified to NSRS07",                      3557,
					               "NSRS11.ME-E",  "NSRS 2011 Maine State Planes, East Zone, Meter",                     "CRS HARN/ME.ME-E with datum modified to NSRS11",                         0   },
	{              "HARN/ME.ME-EF",    "HARN/ME",  26855,
					              "NSRS07.ME-EF",  "NSRS 2007 Maine State Planes, East Zone, US Foot",                   "CRS HARN/ME.ME-EF with datum modified to NSRS07",                    26863,
					              "NSRS11.ME-EF",  "NSRS 2011 Maine State Planes, East Zone, US Foot",                   "CRS HARN/ME.ME-EF with datum modified to NSRS11",                        0   },
	{               "HARN/ME.ME-W",    "HARN/ME",   2803,
					               "NSRS07.ME-W",  "NSRS 2007 Maine State Planes, West Zone, Meter",                     "CRS HARN/ME.ME-W with datum modified to NSRS07",                      3558,
					               "NSRS11.ME-W",  "NSRS 2011 Maine State Planes, West Zone, Meter",                     "CRS HARN/ME.ME-W with datum modified to NSRS11",                         0   },
	{              "HARN/ME.ME-WF",    "HARN/ME",  26856,
					              "NSRS07.ME-WF",  "NSRS 2007 Maine State Planes, West Zone, US Foot",                   "CRS HARN/ME.ME-WF with datum modified to NSRS07",                    26864,
					              "NSRS11.ME-WF",  "NSRS 2011 Maine State Planes, West Zone, US Foot",                   "CRS HARN/ME.ME-WF with datum modified to NSRS11",                        0   },
	{               "HARN/MS.MS-E",    "HARN/MS",   2813,
					               "NSRS07.MS-E",  "NSRS 2007 Mississippi State Planes, East Zone, Meter",               "CRS HARN/MS.MS-E with datum modified to NSRS07",                      3597,
					               "NSRS11.MS-E",  "NSRS 2011 Mississippi State Planes, East Zone, Meter",               "CRS HARN/MS.MS-E with datum modified to NSRS11",                         0   },
	{              "HARN/MS.MS-EF",    "HARN/MS",   2899,
					              "NSRS07.MS-EF",  "NSRS 2007 Mississippi State Planes, East Zone, US Foot",             "CRS HARN/MS.MS-EF with datum modified to NSRS07",                     3598,
					              "NSRS11.MS-EF",  "NSRS 2011 Mississippi State Planes, East Zone, US Foot",             "CRS HARN/MS.MS-EF with datum modified to NSRS11",                        0   },
	{               "HARN/MS.MS-W",    "HARN/MS",   2814,
					               "NSRS07.MS-W",  "NSRS 2007 Mississippi State Planes, West Zone, Meter",               "CRS HARN/MS.MS-W with datum modified to NSRS07",                      3599,
					               "NSRS11.MS-W",  "NSRS 2011 Mississippi State Planes, West Zone, Meter",               "CRS HARN/MS.MS-W with datum modified to NSRS11",                         0   },
	{                 "HARN/MT.MT",    "HARN/MT",   2818,
					                 "NSRS07.MT",  "NSRS 2007 Montana State Plane Zone, Meter",                          "CRS HARN/MT.MT with datum modified to NSRS07",                        3604,
					                 "NSRS11.MT",  "NSRS 2011 Montana State Plane Zone, Meter",                          "CRS HARN/MT.MT with datum modified to NSRS11",                           0   },
	// CS-MAP has a HARN version of Montana based on US Survey Feet, but nobody else does.
	//{                "HARN/MT.MTF",    "HARN/MT",      0,
	//				                "NSRS07.MTF",  "NSRS 2007 Montana State Plane Zone, US Foot",                        "CRS HARN/MT.MTF with datum modified to NSRS07",                          0,
	//				                "NSRS11.MTF",  "NSRS 2011 Montana State Plane Zone, US Foot",                        "CRS HARN/MT.MTF with datum modified to NSRS11",                          0   },
	{               "HARN/MT.MTIF",    "HARN/MT",   2901,
					               "NSRS07.MTIF",  "NSRS 2007 Montana State Planes, International Foot",                 "CRS HARN/MT.MTIF with datum modified to NSRS07",                      3605,
					               "NSRS11.MTIF",  "NSRS 2011 Montana State Planes, International Foot",                 "CRS HARN/MT.MTIF with datum modified to NSRS11",                         0   },
	{                 "HARN/NB.NB",    "HARN/NB",   2819,
					                 "NSRS07.NB",  "NSRS 2007 Nebraska State Planes, Meter",                             "CRS HARN/NB.NB with datum modified to NSRS07",                        3606,
					                 "NSRS11.NB",  "NSRS 2011 Nebraska State Planes, Meter",                             "CRS HARN/NB.NB with datum modified to NSRS11",                           0   },
	{                "HARN/NB.NBF",    "HARN/NB",  26860,
					                "NSRS07.NBF",  "NSRS 2007 Nebraska State Planes, US Foot",                           "CRS HARN/NB.NBF with datum modified to NSRS07",                      26868,
					                "NSRS11.NBF",  "NSRS 2011 Nebraska State Planes, US Foot",                           "CRS HARN/NB.NBF with datum modified to NSRS11",                          0   },
	{                 "HARN/NE.NH",    "HARN/NE",   2823,
					                 "NSRS07.NH",  "NSRS 2007 New Hampshire State Planes, Meter",                        "CRS HARN/NE.NH with datum modified to NSRS07",                        3613,
					                 "NSRS11.NH",  "NSRS 2011 New Hampshire State Planes, Meter",                        "CRS HARN/NE.NH with datum modified to NSRS11",                           0   },
	{                "HARN/NE.NHF",    "HARN/NE",   3445,
					                "NSRS07.NHF",  "NSRS 2007 New Hampshire State Planes, US Foot",                      "CRS HARN/NE.NHF with datum modified to NSRS07",                       3614,
					                "NSRS11.NHF",  "NSRS 2011 New Hampshire State Planes, US Foot",                      "CRS HARN/NE.NHF with datum modified to NSRS11",                          0   },
	{               "HARN/NM.NM-C",    "HARN/NM",   2826,
					               "NSRS07.NM-C",  "NSRS 2007 New Mexico State Planes, Central Zone, Meter",             "CRS HARN/NM.NM-C with datum modified to NSRS07",                      3617,
					               "NSRS11.NM-C",  "NSRS 2011 New Mexico State Planes, Central Zone, Meter",             "CRS HARN/NM.NM-C with datum modified to NSRS11",                         0   },
	{              "HARN/NM.NM-CF",    "HARN/NM",   2903,
					              "NSRS07.NM-CF",  "NSRS 2007 New Mexico State Planes, Central Zone, US Foot",           "CRS HARN/NM.NM-CF with datum modified to NSRS07",                     3618,
					              "NSRS11.NM-CF",  "NSRS 2011 New Mexico State Planes, Central Zone, US Foot",           "CRS HARN/NM.NM-CF with datum modified to NSRS11",                        0   },
	{               "HARN/NM.NM-E",    "HARN/NM",   2825,
					               "NSRS07.NM-E",  "NSRS 2007 New Mexico State Planes, East Zone, Meter",                "CRS HARN/NM.NM-E with datum modified to NSRS07",                      3619,
					               "NSRS11.NM-E",  "NSRS 2011 New Mexico State Planes, East Zone, Meter",                "CRS HARN/NM.NM-E with datum modified to NSRS11",                         0   },
	{              "HARN/NM.NM-EF",    "HARN/NM",   2902,
					              "NSRS07.NM-EF",  "NSRS 2007 New Mexico State Planes, East Zone, US Foot",              "CRS HARN/NM.NM-EF with datum modified to NSRS07",                     3620,
					              "NSRS11.NM-EF",  "NSRS 2011 New Mexico State Planes, East Zone, US Foot",              "CRS HARN/NM.NM-EF with datum modified to NSRS11",                        0   },
	{               "HARN/NM.NM-W",    "HARN/NM",   2827,
					               "NSRS07.NM-W",  "NSRS 2007 New Mexico State Planes, West Zone, Meter",                "CRS HARN/NM.NM-W with datum modified to NSRS07",                      3621,
					               "NSRS11.NM-W",  "NSRS 2011 New Mexico State Planes, West Zone, Meter",                "CRS HARN/NM.NM-W with datum modified to NSRS11",                         0   },
	{              "HARN/NM.NM-WF",    "HARN/NM",   2904,
					              "NSRS07.NM-WF",  "NSRS 2007 New Mexico State Planes, West Zone, US Foot",              "CRS HARN/NM.NM-WF with datum modified to NSRS07",                     3622,
					              "NSRS11.NM-WF",  "NSRS 2011 New Mexico State Planes, West Zone, US Foot",              "CRS HARN/NM.NM-WF with datum modified to NSRS11",                        0   },
	{               "HARN/OK.OK-N",    "HARN/OK",   2836,
					               "NSRS07.OK-N",  "NSRS 2007 Oklahoma State Planes, North Zone, Meter",                 "CRS HARN/OK.OK-N with datum modified to NSRS07",                      3639,
					               "NSRS11.OK-N",  "NSRS 2011 Oklahoma State Planes, North Zone, Meter",                 "CRS HARN/OK.OK-N with datum modified to NSRS11",                         0   },
	{              "HARN/OK.OK-NF",    "HARN/OK",   2911,
					              "NSRS07.OK-NF",  "NSRS 2007 Oklahoma State Planes, North Zone, US Foot",               "CRS HARN/OK.OK-NF with datum modified to NSRS07",                     3640,
					              "NSRS11.OK-NF",  "NSRS 2011 Oklahoma State Planes, North Zone, US Foot",               "CRS HARN/OK.OK-NF with datum modified to NSRS11",                        0   },
	{               "HARN/OK.OK-S",    "HARN/OK",   2837,
					               "NSRS07.OK-S",  "NSRS 2007 Oklahoma State Planes, South Zone, Meter",                 "CRS HARN/OK.OK-S with datum modified to NSRS07",                      3641,
					               "NSRS11.OK-S",  "NSRS 2011 Oklahoma State Planes, South Zone, Meter",                 "CRS HARN/OK.OK-S with datum modified to NSRS11",                         0   },
	{              "HARN/OK.OK-SF",    "HARN/OK",   2912,
					              "NSRS07.OK-SF",  "NSRS 2007 Oklahoma State Planes, South Zone, US Foot",               "CRS HARN/OK.OK-SF with datum modified to NSRS07",                     3642,
					              "NSRS11.OK-SF",  "NSRS 2011 Oklahoma State Planes, South Zone, US Foot",               "CRS HARN/OK.OK-SF with datum modified to NSRS11",                        0   },
	{               "HARN/WO.OR-N",    "HARN/WO",   2838,
					               "NSRS07.OR-N",  "NSRS 2007 Oregon State Planes (Polyconic), North Zone, Meter",       "CRS HARN/WO.OR-N with datum modified to NSRS07",                      3645,
					               "NSRS11.OR-N",  "NSRS 2011 Oregon State Planes (Polyconic), North Zone, Meter",       "CRS HARN/WO.OR-N with datum modified to NSRS11",                         0   },
	{             "HARN/WO.OR-NIF",    "HARN/WO",   2913,
					             "NSRS07.OR-NIF",  "NSRS 2007 Oregon State Planes, North Zone, International Foot",      "CRS HARN/WO.OR-NIF with datum modified to NSRS07",                    3646,
					             "NSRS11.OR-NIF",  "NSRS 2011 Oregon State Planes, North Zone, International Foot",      "CRS HARN/WO.OR-NIF with datum modified to NSRS11",                       0   },
	{               "HARN/WO.OR-S",    "HARN/WO",   2839,
					               "NSRS07.OR-S",  "NSRS 2007 Oregon State Planes (Polyconic), South Zone, Meter",       "CRS HARN/WO.OR-S with datum modified to NSRS07",                      3647,
					               "NSRS11.OR-S",  "NSRS 2011 Oregon State Planes (Polyconic), South Zone, Meter",       "CRS HARN/WO.OR-S with datum modified to NSRS11",                         0   },
	{             "HARN/WO.OR-SIF",    "HARN/WO",   2914,
					             "NSRS07.OR-SIF",  "NSRS 2007 Oregon State Planes, South Zone, International Foot",      "CRS HARN/WO.OR-SIF with datum modified to NSRS07",                    3648,
					             "NSRS11.OR-SIF",  "NSRS 2011 Oregon State Planes, South Zone, International Foot",      "CRS HARN/WO.OR-SIF with datum modified to NSRS11",                       0   },
	// CS-MAP has HARN versions of Oregon based on US Survey Feet, but nobody else does.
	//{              "HARN/WO.OR-SF",    "HARN/WO",      0,
	//				              "NSRS07.OR-SF",  "NSRS 2007 Oregon State Planes, South Zone, US Foot",                 "CRS HARN/WO.OR-SF with datum modified to NSRS07",                        0,
	//				              "NSRS11.OR-SF",  "NSRS 2011 Oregon State Planes, South Zone, US Foot",                 "CRS HARN/WO.OR-SF with datum modified to NSRS11",                        0   },
	//{              "HARN/WO.OR-NF",    "HARN/WO",      0,
	//				              "NSRS07.OR-NF",  "NSRS 2007 Oregon State Planes, North Zone, US Foot",                 "CRS HARN/WO.OR-NF with datum modified to NSRS07",                        0,
	//				              "NSRS11.OR-NF",  "NSRS 2011 Oregon State Planes, North Zone, US Foot",                 "CRS HARN/WO.OR-NF with datum modified to NSRS11",                        0   },
	{               "HARN/PV.PRHP",    "HARN/PV",   2866,
					               "NSRS07.PRHP",  "NSRS 2007 Puerto Rico and Virgin Islands, Meter",                    "CRS HARN/PV.PRHP with datum modified to NSRS07",                      4437,
					               "NSRS11.PRHP",  "NSRS 2011 Puerto Rico and Virgin Islands, Meter",                    "CRS HARN/PV.PRHP with datum modified to NSRS11",                         0   },
	// CS-MAP has a system for HARN Puerto Rico in US Survey Foot, but nobody else does.
	//{              "HARN/PV.PRHPF",    "HARN/PV",      0,
	//				              "NSRS07.PRHPF",  "NSRS 2007 Puerto Rico and Virgin Islands, US Foot",                  "CRS HARN/PV.PRHPF with datum modified to NSRS07",                        0,
	//				              "NSRS11.PRHPF",  "NSRS 2011 Puerto Rico and Virgin Islands, US Foot",                  "CRS HARN/PV.PRHPF with datum modified to NSRS11",                        0   },
	{                 "HARN/NE.RI",    "HARN/NE",   2840,
					                 "NSRS07.RI",  "NSRS 2007 Rhode Island State Planes, Meter",                         "CRS HARN/NE.RI with datum modified to NSRS07",                        3653,
					                 "NSRS11.RI",  "NSRS 2011 Rhode Island State Planes, Meter",                         "CRS HARN/NE.RI with datum modified to NSRS11",                           0   },
	{                "HARN/NE.RIF",    "HARN/NE",   3446,
					                "NSRS07.RIF",  "NSRS 2007 Rhode Island State Planes, US Foot",                       "CRS HARN/NE.RIF with datum modified to NSRS07",                       3654,
					                "NSRS11.RIF",  "NSRS 2011 Rhode Island State Planes, US Foot",                       "CRS HARN/NE.RIF with datum modified to NSRS11",                          0   },
	{                 "HARN/TN.TN",    "HARN/TN",   2843,
					                 "NSRS07.TN",  "NSRS 2007 Tennessee State Plane Zone, Meter",                        "CRS HARN/TN.TN with datum modified to NSRS07",                        3661,
					                 "NSRS11.TN",  "NSRS 2011 Tennessee State Plane Zone, Meter",                        "CRS HARN/TN.TN with datum modified to NSRS11",                           0   },
	{                "HARN/TN.TNF",    "HARN/TN",   2915,
					                "NSRS07.TNF",  "NSRS 2007 Tennessee State Plane Zone, US Foot",                      "CRS HARN/TN.TNF with datum modified to NSRS07",                       3662,
					                "NSRS11.TNF",  "NSRS 2011 Tennessee State Plane Zone, US Foot",                      "CRS HARN/TN.TNF with datum modified to NSRS11",                          0   },
	{               "HARN/TX.TX-C",    "HARN/TX",   2846,
					               "NSRS07.TX-C",  "NSRS 2007 Texas State Planes, Central Zone, Meter",                  "CRS HARN/TX.TX-C with datum modified to NSRS07",                      3663,
					               "NSRS11.TX-C",  "NSRS 2011 Texas State Planes, Central Zone, Meter",                  "CRS HARN/TX.TX-C with datum modified to NSRS11",                         0   },
	{              "HARN/TX.TX-CF",    "HARN/TX",   2918,
					              "NSRS07.TX-CF",  "NSRS 2007 Texas State Planes, Central Zone, US Foot",                "CRS HARN/TX.TX-CF with datum modified to NSRS07",                     3664,
					              "NSRS11.TX-CF",  "NSRS 2011 Texas State Planes, Central Zone, US Foot",                "CRS HARN/TX.TX-CF with datum modified to NSRS11",                        0   },
	{               "HARN/TX.TX-N",    "HARN/TX",   2844,
					               "NSRS07.TX-N",  "NSRS 2007 Texas State Planes, North Zone, Meter",                    "CRS HARN/TX.TX-N with datum modified to NSRS07",                      3667,
					               "NSRS11.TX-N",  "NSRS 2011 Texas State Planes, North Zone, Meter",                    "CRS HARN/TX.TX-N with datum modified to NSRS11",                         0   },
	{              "HARN/TX.TX-NC",    "HARN/TX",   2845,
					              "NSRS07.TX-NC",  "NSRS 2007 Texas State Planes, North Central Zone, Meter",            "CRS HARN/TX.TX-NC with datum modified to NSRS07",                     3669,
					              "NSRS11.TX-NC",  "NSRS 2011 Texas State Planes, North Central Zone, Meter",            "CRS HARN/TX.TX-NC with datum modified to NSRS11",                        0   },
	{             "HARN/TX.TX-NCF",    "HARN/TX",   2917,
					             "NSRS07.TX-NCF",  "NSRS 2007 Texas State Planes, North Central Zone, US Foot",          "CRS HARN/TX.TX-NCF with datum modified to NSRS07",                    3670,
					             "NSRS11.TX-NCF",  "NSRS 2011 Texas State Planes, North Central Zone, US Foot",          "CRS HARN/TX.TX-NCF with datum modified to NSRS11",                       0   },
	{              "HARN/TX.TX-NF",    "HARN/TX",   2916,
					              "NSRS07.TX-NF",  "NSRS 2007 Texas State Planes, North Zone, US Foot",                  "CRS HARN/TX.TX-NF with datum modified to NSRS07",                     3668,
					              "NSRS11.TX-NF",  "NSRS 2011 Texas State Planes, North Zone, US Foot",                  "CRS HARN/TX.TX-NF with datum modified to NSRS11",                        0   },
	{               "HARN/TX.TX-S",    "HARN/TX",   2848,
					               "NSRS07.TX-S",  "NSRS 2007 Texas State Planes, South Zone, Meter",                    "CRS HARN/TX.TX-S with datum modified to NSRS07",                      3671,
					               "NSRS11.TX-S",  "NSRS 2011 Texas State Planes, South Zone, Meter",                    "CRS HARN/TX.TX-S with datum modified to NSRS11",                         0   },
	{              "HARN/TX.TX-SC",    "HARN/TX",   2847,
					              "NSRS07.TX-SC",  "NSRS 2007 Texas State Planes, South Central Zone, Meter",            "CRS HARN/TX.TX-SC with datum modified to NSRS07",                     3673,
					              "NSRS11.TX-SC",  "NSRS 2011 Texas State Planes, South Central Zone, Meter",            "CRS HARN/TX.TX-SC with datum modified to NSRS11",                        0   },
	{             "HARN/TX.TX-SCF",    "HARN/TX",   2919,
					             "NSRS07.TX-SCF",  "NSRS 2007 Texas State Planes, South Central Zone, US Foot",          "CRS HARN/TX.TX-SCF with datum modified to NSRS07",                    3674,
					             "NSRS11.TX-SCF",  "NSRS 2011 Texas State Planes, South Central Zone, US Foot",          "CRS HARN/TX.TX-SCF with datum modified to NSRS11",                       0   },
	{              "HARN/TX.TX-SF",    "HARN/TX",   2920,
					              "NSRS07.TX-SF",  "NSRS 2007 Texas State Planes, South Zone, US Foot",                  "CRS HARN/TX.TX-SF with datum modified to NSRS07",                     3672,
					              "NSRS11.TX-SF",  "NSRS 2011 Texas State Planes, South Zone, US Foot",                  "CRS HARN/TX.TX-SF with datum modified to NSRS11",                        0   },
	{             "HARN/10.UTM-10",    "HARN/10",   3740,
					             "NSRS07.UTM-10",  "NSRS 2007 UTM, Zone 10, Meter; Central Meridian 123d W",             "CRS HARN/10.UTM-10 with datum modified to NSRS07",                    3717,
					             "NSRS11.UTM-10",  "NSRS 2011 UTM, Zone 10, Meter; Central Meridian 123d W",             "CRS HARN/10.UTM-10 with datum modified to NSRS11",                       0   },
	{            "HARN/10.UTM-10F",    "HARN/10",      0,
					            "NSRS07.UTM-10F",  "NSRS 2007 UTM, Zone 10,US Foot; Central Meridian 123d W",            "CRS HARN/10.UTM-10F with datum modified to NSRS07",                      0,
					            "NSRS11.UTM-10F",  "NSRS 2011 UTM, Zone 10,US Foot; Central Meridian 123d W",            "CRS HARN/10.UTM-10F with datum modified to NSRS11",                      0   },
	{           "HARN/10.UTM-10IF",    "HARN/10",      0,
					           "NSRS07.UTM-10IF",  "NSRS 2007 UTM, Zone 10,Int.Foot;Central Meridian 123d W",            "CRS HARN/10.UTM-10IF with datum modified to NSRS07",                     0,
					           "NSRS11.UTM-10IF",  "NSRS 2011 UTM, Zone 10,Int.Foot;Central Meridian 123d W",            "CRS HARN/10.UTM-10IF with datum modified to NSRS11",                     0   },
	{             "HARN/11.UTM-11",    "HARN/11",   3741,
					             "NSRS07.UTM-11",  "NSRS 2007 UTM, Zone 11, Meter; Central Meridian 117d W",             "CRS HARN/11.UTM-11 with datum modified to NSRS07",                    3718,
					             "NSRS11.UTM-11",  "NSRS 2011 UTM, Zone 11, Meter; Central Meridian 117d W",             "CRS HARN/11.UTM-11 with datum modified to NSRS11",                       0   },
	{            "HARN/11.UTM-11F",    "HARN/11",      0,
					            "NSRS07.UTM-11F",  "NSRS 2007 UTM, Zone 11,US Foot; Central Meridian 117d W",            "CRS HARN/11.UTM-11F with datum modified to NSRS07",                      0,
					            "NSRS11.UTM-11F",  "NSRS 2011 UTM, Zone 11,US Foot; Central Meridian 117d W",            "CRS HARN/11.UTM-11F with datum modified to NSRS11",                      0   },
	{           "HARN/11.UTM-11IF",    "HARN/11",      0,
					           "NSRS07.UTM-11IF",  "NSRS 2007 UTM, Zone 11,Int.Foot;Central Meridian 117d W",            "CRS HARN/11.UTM-11IF with datum modified to NSRS07",                     0,
					           "NSRS11.UTM-11IF",  "NSRS 2011 UTM, Zone 11,Int.Foot;Central Meridian 117d W",            "CRS HARN/11.UTM-11IF with datum modified to NSRS11",                     0   },
	{             "HARN/12.UTM-12",    "HARN/12",   3742,
					             "NSRS07.UTM-12",  "NSRS 2007 UTM, Zone 12, Meter; Central Meridian 111d W",             "CRS HARN/12.UTM-12 with datum modified to NSRS07",                    3719,
					             "NSRS11.UTM-12",  "NSRS 2011 UTM, Zone 12, Meter; Central Meridian 111d W",             "CRS HARN/12.UTM-12 with datum modified to NSRS11",                       0   },
	{            "HARN/12.UTM-12F",    "HARN/12",      0,
					            "NSRS07.UTM-12F",  "NSRS 2007 UTM, Zone 12,US Foot; Central Meridian 111d W",            "CRS HARN/12.UTM-12F with datum modified to NSRS07",                      0,
					            "NSRS11.UTM-12F",  "NSRS 2011 UTM, Zone 12,US Foot; Central Meridian 111d W",            "CRS HARN/12.UTM-12F with datum modified to NSRS11",                      0   },
	{           "HARN/12.UTM-12IF",    "HARN/12",      0,
					           "NSRS07.UTM-12IF",  "NSRS 2007 UTM, Zone 12,Int.Foot;Central Meridian 111d W",            "CRS HARN/12.UTM-12IF with datum modified to NSRS07",                     0,
					           "NSRS11.UTM-12IF",  "NSRS 2011 UTM, Zone 12,Int.Foot;Central Meridian 111d W",            "CRS HARN/12.UTM-12IF with datum modified to NSRS11",                     0   },
	{             "HARN/13.UTM-13",    "HARN/13",   3743,
					             "NSRS07.UTM-13",  "NSRS 2007 UTM, Zone 13, Meter; Central Meridian 105d W",             "CRS HARN/13.UTM-13 with datum modified to NSRS07",                    3720,
					             "NSRS11.UTM-13",  "NSRS 2011 UTM, Zone 13, Meter; Central Meridian 105d W",             "CRS HARN/13.UTM-13 with datum modified to NSRS11",                       0   },
	{            "HARN/13.UTM-13F",    "HARN/13",      0,
					            "NSRS07.UTM-13F",  "NSRS 2007 UTM, Zone 13,US Foot; Central Meridian 105d W",            "CRS HARN/13.UTM-13F with datum modified to NSRS07",                      0,
					            "NSRS11.UTM-13F",  "NSRS 2011 UTM, Zone 13,US Foot; Central Meridian 105d W",            "CRS HARN/13.UTM-13F with datum modified to NSRS11",                      0   },
	{           "HARN/13.UTM-13IF",    "HARN/13",      0,
					           "NSRS07.UTM-13IF",  "NSRS 2007 UTM, Zone 13,Int.Foot;Central Meridian 105d W",            "CRS HARN/13.UTM-13IF with datum modified to NSRS07",                     0,
					           "NSRS11.UTM-13IF",  "NSRS 2011 UTM, Zone 13,Int.Foot;Central Meridian 105d W",            "CRS HARN/13.UTM-13IF with datum modified to NSRS11",                     0   },
	{             "HARN/14.UTM-14",    "HARN/14",   3744,
					             "NSRS07.UTM-14",  "NSRS 2007 UTM, Zone 14, Meter; Central Meridian 99d W",              "CRS HARN/14.UTM-14 with datum modified to NSRS07",                    3721,
					             "NSRS11.UTM-14",  "NSRS 2011 UTM, Zone 14, Meter; Central Meridian 99d W",              "CRS HARN/14.UTM-14 with datum modified to NSRS11",                       0   },
	{            "HARN/14.UTM-14F",    "HARN/14",      0,
					            "NSRS07.UTM-14F",  "NSRS 2007 UTM, Zone 14, US Foot; Central Meridian 99d W",            "CRS HARN/14.UTM-14F with datum modified to NSRS07",                      0,
					            "NSRS11.UTM-14F",  "NSRS 2011 UTM, Zone 14, US Foot; Central Meridian 99d W",            "CRS HARN/14.UTM-14F with datum modified to NSRS11",                      0   },
	{           "HARN/14.UTM-14IF",    "HARN/14",      0,
					           "NSRS07.UTM-14IF",  "NSRS 2007 UTM, Zone 14,Int.Foot; Central Meridian 99d W",            "CRS HARN/14.UTM-14IF with datum modified to NSRS07",                     0,
					           "NSRS11.UTM-14IF",  "NSRS 2011 UTM, Zone 14,Int.Foot; Central Meridian 99d W",            "CRS HARN/14.UTM-14IF with datum modified to NSRS11",                     0   },
	{             "HARN/15.UTM-15",    "HARN/15",   3745,
					             "NSRS07.UTM-15",  "NSRS 2007 UTM, Zone 15, Meter; Central Meridian 93d W",              "CRS HARN/15.UTM-15 with datum modified to NSRS07",                    3722,
					             "NSRS11.UTM-15",  "NSRS 2011 UTM, Zone 15, Meter; Central Meridian 93d W",              "CRS HARN/15.UTM-15 with datum modified to NSRS11",                       0   },
	{            "HARN/15.UTM-15F",    "HARN/15",      0,
					            "NSRS07.UTM-15F",  "NSRS 2007 UTM, Zone 15, US Foot; Central Meridian 93d W",            "CRS HARN/15.UTM-15F with datum modified to NSRS07",                      0,
					            "NSRS11.UTM-15F",  "NSRS 2011 UTM, Zone 15, US Foot; Central Meridian 93d W",            "CRS HARN/15.UTM-15F with datum modified to NSRS11",                      0   },
	{           "HARN/15.UTM-15IF",    "HARN/15",      0,
					           "NSRS07.UTM-15IF",  "NSRS 2007 UTM, Zone 15,Int.Foot; Central Meridian 93d W",            "CRS HARN/15.UTM-15IF with datum modified to NSRS07",                     0,
					           "NSRS11.UTM-15IF",  "NSRS 2011 UTM, Zone 15,Int.Foot; Central Meridian 93d W",            "CRS HARN/15.UTM-15IF with datum modified to NSRS11",                     0   },
	{             "HARN/16.UTM-16",    "HARN/16",   3746,
					             "NSRS07.UTM-16",  "NSRS 2007 UTM, Zone 16, Meter; Central Meridian 87d W",              "CRS HARN/16.UTM-16 with datum modified to NSRS07",                    3723,
					             "NSRS11.UTM-16",  "NSRS 2011 UTM, Zone 16, Meter; Central Meridian 87d W",              "CRS HARN/16.UTM-16 with datum modified to NSRS11",                       0   },
	{            "HARN/16.UTM-16F",    "HARN/16",      0,
					            "NSRS07.UTM-16F",  "NSRS 2007 UTM, Zone 16, US Foot; Central Meridian 87d W",            "CRS HARN/16.UTM-16F with datum modified to NSRS07",                      0,
					            "NSRS11.UTM-16F",  "NSRS 2011 UTM, Zone 16, US Foot; Central Meridian 87d W",            "CRS HARN/16.UTM-16F with datum modified to NSRS11",                      0   },
	{           "HARN/16.UTM-16IF",    "HARN/16",      0,
					           "NSRS07.UTM-16IF",  "NSRS 2007 UTM, Zone 16,Int.Foot; Central Meridian 87d W",            "CRS HARN/16.UTM-16IF with datum modified to NSRS07",                     0,
					           "NSRS11.UTM-16IF",  "NSRS 2011 UTM, Zone 16,Int.Foot; Central Meridian 87d W",            "CRS HARN/16.UTM-16IF with datum modified to NSRS11",                     0   },
	{             "HARN/17.UTM-17",    "HARN/17",   3747,
					             "NSRS07.UTM-17",  "NSRS 2007 UTM, Zone 17, Meter; Central Meridian 81d W",              "CRS HARN/17.UTM-17 with datum modified to NSRS07",                    3724,
					             "NSRS11.UTM-17",  "NSRS 2011 UTM, Zone 17, Meter; Central Meridian 81d W",              "CRS HARN/17.UTM-17 with datum modified to NSRS11",                       0   },
	{            "HARN/17.UTM-17F",    "HARN/17",      0,
					            "NSRS07.UTM-17F",  "NSRS 2007 UTM, Zone 17, US Foot; Central Meridian 81d W",            "CRS HARN/17.UTM-17F with datum modified to NSRS07",                      0,
					            "NSRS11.UTM-17F",  "NSRS 2011 UTM, Zone 17, US Foot; Central Meridian 81d W",            "CRS HARN/17.UTM-17F with datum modified to NSRS11",                      0   },
	{           "HARN/17.UTM-17IF",    "HARN/17",      0,
					           "NSRS07.UTM-17IF",  "NSRS 2007 UTM, Zone 17,Int.Foot; Central Meridian 81d W",            "CRS HARN/17.UTM-17IF with datum modified to NSRS07",                     0,
					           "NSRS11.UTM-17IF",  "NSRS 2011 UTM, Zone 17,Int.Foot; Central Meridian 81d W",            "CRS HARN/17.UTM-17IF with datum modified to NSRS11",                     0   },
	{             "HARN/18.UTM-18",    "HARN/18",   3748,
					             "NSRS07.UTM-18",  "NSRS 2007 UTM, Zone 18, Meter; Central Meridian 75d W",              "CRS HARN/18.UTM-18 with datum modified to NSRS07",                    3725,
					             "NSRS11.UTM-18",  "NSRS 2011 UTM, Zone 18, Meter; Central Meridian 75d W",              "CRS HARN/18.UTM-18 with datum modified to NSRS11",                       0   },
	{            "HARN/18.UTM-18F",    "HARN/18",      0,
					            "NSRS07.UTM-18F",  "NSRS 2007 UTM, Zone 18, US Foot; Central Meridian 75d W",            "CRS HARN/18.UTM-18F with datum modified to NSRS07",                      0,
					            "NSRS11.UTM-18F",  "NSRS 2011 UTM, Zone 18, US Foot; Central Meridian 75d W",            "CRS HARN/18.UTM-18F with datum modified to NSRS11",                      0   },
	{           "HARN/18.UTM-18IF",    "HARN/18",      0,
					           "NSRS07.UTM-18IF",  "NSRS 2007 UTM, Zone 18,Int.Foot; Central Meridian 75d W",            "CRS HARN/18.UTM-18IF with datum modified to NSRS07",                     0,
					           "NSRS11.UTM-18IF",  "NSRS 2011 UTM, Zone 18,Int.Foot; Central Meridian 75d W",            "CRS HARN/18.UTM-18IF with datum modified to NSRS11",                     0   },
	{               "HARN/VA.VA-N",    "HARN/VA",   2853,
					               "NSRS07.VA-N",  "NSRS 2007 Virginia State Planes, Northern Zone, Meter",              "CRS HARN/VA.VA-N with datum modified to NSRS07",                      3685,
					               "NSRS11.VA-N",  "NSRS 2011 Virginia State Planes, Northern Zone, Meter",              "CRS HARN/VA.VA-N with datum modified to NSRS11",                         0   },
	{              "HARN/VA.VA-NF",    "HARN/VA",   2924,
					              "NSRS07.VA-NF",  "NSRS 2007 Virginia State Planes, North Zone, US Foot",               "CRS HARN/VA.VA-NF with datum modified to NSRS07",                     3686,
					              "NSRS11.VA-NF",  "NSRS 2011 Virginia State Planes, North Zone, US Foot",               "CRS HARN/VA.VA-NF with datum modified to NSRS11",                        0   },
	{               "HARN/VA.VA-S",    "HARN/VA",   2854,
					               "NSRS07.VA-S",  "NSRS 2007 Virginia State Planes, South Zone, Meter",                 "CRS HARN/VA.VA-S with datum modified to NSRS07",                      3687,
					               "NSRS11.VA-S",  "NSRS 2011 Virginia State Planes, South Zone, Meter",                 "CRS HARN/VA.VA-S with datum modified to NSRS11",                         0   },
	{              "HARN/VA.VA-SF",    "HARN/VA",   2925,
					              "NSRS07.VA-SF",  "NSRS 2007 Virginia State Planes, South Zone, US Foot",               "CRS HARN/VA.VA-SF with datum modified to NSRS07",                     3688,
					              "NSRS11.VA-SF",  "NSRS 2011 Virginia State Planes, South Zone, US Foot",               "CRS HARN/VA.VA-SF with datum modified to NSRS11",                        0   },
	{                 "HARN/NE.VT",    "HARN/NE",   2852,
					                 "NSRS07.VT",  "NSRS 2007 Vermont State Planes, Meter",                              "CRS HARN/NE.VT with datum modified to NSRS07",                        3684,
					                 "NSRS11.VT",  "NSRS 2011 Vermont State Planes, Meter",                              "CRS HARN/NE.VT with datum modified to NSRS11",                           0   },
	{                "HARN/NE.VTF",    "HARN/NE",   5654,
					                "NSRS07.VTF",  "NSRS 2007 Vermont State Planes, US Foot",                            "CRS HARN/NE.VTF with datum modified to NSRS07",                       5655,
					                "NSRS11.VTF",  "NSRS 2011 Vermont State Planes, US Foot",                            "CRS HARN/NE.VTF with datum modified to NSRS11",                       6590   },	// There is no ESRI 2007 euivalent for this system, so this EPSG code has been hard coded.
	{               "HARN/WO.WA-N",    "HARN/WO",   2855,
					               "NSRS07.WA-N",  "NSRS 2007 Washington State Planes, North Zone, Meter",               "CRS HARN/WO.WA-N with datum modified to NSRS07",                      3689,
					               "NSRS11.WA-N",  "NSRS 2011 Washington State Planes, North Zone, Meter",               "CRS HARN/WO.WA-N with datum modified to NSRS11",                         0   },
	{              "HARN/WO.WA-NF",    "HARN/WO",   2926,
					              "NSRS07.WA-NF",  "NSRS 2007 Washington State Planes, North Zone, US Foot",             "CRS HARN/WO.WA-NF with datum modified to NSRS07",                     3690,
					              "NSRS11.WA-NF",  "NSRS 2011 Washington State Planes, North Zone, US Foot",             "CRS HARN/WO.WA-NF with datum modified to NSRS11",                        0   },
	{               "HARN/WO.WA-S",    "HARN/WO",   2856,
					               "NSRS07.WA-S",  "NSRS 2007 Washington State Planes, South Zone, Meter",               "CRS HARN/WO.WA-S with datum modified to NSRS07",                      3691,
					               "NSRS11.WA-S",  "NSRS 2011 Washington State Planes, South Zone, Meter",               "CRS HARN/WO.WA-S with datum modified to NSRS11",                         0   },
	{              "HARN/WO.WA-SF",    "HARN/WO",   2927,
					              "NSRS07.WA-SF",  "NSRS 2007 Washington State Planes, South Zone, US Foot",             "CRS HARN/WO.WA-SF with datum modified to NSRS07",                     3692,
					              "NSRS11.WA-SF",  "NSRS 2011 Washington State Planes, South Zone, US Foot",             "CRS HARN/WO.WA-SF with datum modified to NSRS11",                        0   },
	{               "HARN/WI.WI-C",    "HARN/WI",   2860,
					               "NSRS07.WI-C",  "NSRS 2007 Wisconsin State Planes, Central Zone, Meter",              "CRS HARN/WI.WI-C with datum modified to NSRS07",                      3695,
					               "NSRS11.WI-C",  "NSRS 2011 Wisconsin State Planes, Central Zone, Meter",              "CRS HARN/WI.WI-C with datum modified to NSRS11",                         0   },
	{              "HARN/WI.WI-CF",    "HARN/WI",   2929,
					              "NSRS07.WI-CF",  "NSRS 2007 Wisconsin State Planes, Central Zone, US Foot",            "CRS HARN/WI.WI-CF with datum modified to NSRS07",                     3696,
					              "NSRS11.WI-CF",  "NSRS 2011 Wisconsin State Planes, Central Zone, US Foot",            "CRS HARN/WI.WI-CF with datum modified to NSRS11",                        0   },
	{               "HARN/WI.WI-N",    "HARN/WI",   2859,
					               "NSRS07.WI-N",  "NSRS 2007 Wisconsin State Planes, North Zone, Meter",                "CRS HARN/WI.WI-N with datum modified to NSRS07",                      3697,
					               "NSRS11.WI-N",  "NSRS 2011 Wisconsin State Planes, North Zone, Meter",                "CRS HARN/WI.WI-N with datum modified to NSRS11",                         0   },
	{              "HARN/WI.WI-NF",    "HARN/WI",   2928,
					              "NSRS07.WI-NF",  "NSRS 2007 Wisconsin State Planes, North Zone, US Foot",              "CRS HARN/WI.WI-NF with datum modified to NSRS07",                     3698,
					              "NSRS11.WI-NF",  "NSRS 2011 Wisconsin State Planes, North Zone, US Foot",              "CRS HARN/WI.WI-NF with datum modified to NSRS11",                        0   },
	{               "HARN/WI.WI-S",    "HARN/WI",   2861,
					               "NSRS07.WI-S",  "NSRS 2007 Wisconsin State Planes, South Zone, Meter",                "CRS HARN/WI.WI-S with datum modified to NSRS07",                      3699,
					               "NSRS11.WI-S",  "NSRS 2011 Wisconsin State Planes, South Zone, Meter",                "CRS HARN/WI.WI-S with datum modified to NSRS11",                         0   },
	{              "HARN/WI.WI-SF",    "HARN/WI",   2930,
					              "NSRS07.WI-SF",  "NSRS 2007 Wisconsin State Planes, South Zone, US Foot",              "CRS HARN/WI.WI-SF with datum modified to NSRS07",                     3700,
					              "NSRS11.WI-SF",  "NSRS 2011 Wisconsin State Planes, South Zone, US Foot",              "CRS HARN/WI.WI-SF with datum modified to NSRS11",                        0   },
	{               "HARN/WY.WY-E",    "HARN/WY",   2862,
					               "NSRS07.WY-E",  "NSRS 2007 Wyoming State Planes, East Zone, Meter",                   "CRS HARN/WY.WY-E with datum modified to NSRS07",                      3702,
					               "NSRS11.WY-E",  "NSRS 2011 Wyoming State Planes, East Zone, Meter",                   "CRS HARN/WY.WY-E with datum modified to NSRS11",                         0   },
	{              "HARN/WY.WY-EC",    "HARN/WY",   2863,
					              "NSRS07.WY-EC",  "NSRS 2007 Wyoming State Planes, East Central Zone, Meter",           "CRS HARN/WY.WY-EC with datum modified to NSRS07",                     3703,
					              "NSRS11.WY-EC",  "NSRS 2011 Wyoming State Planes, East Central Zone, Meter",           "CRS HARN/WY.WY-EC with datum modified to NSRS11",                        0   },
	{             "HARN/WY.WY-ECF",    "HARN/WY",   3756,
					             "NSRS07.WY-ECF",  "NSRS 2007 Wyoming State Planes, East Central Zone, US Foot",         "CRS HARN/WY.WY-ECF with datum modified to NSRS07",                    3731,
					             "NSRS11.WY-ECF",  "NSRS 2011 Wyoming State Planes, East Central Zone, US Foot",         "CRS HARN/WY.WY-ECF with datum modified to NSRS11",                       0   },
	{              "HARN/WY.WY-EF",    "HARN/WY",   3755,
					              "NSRS07.WY-EF",  "NSRS 2007 Wyoming State Planes, East Zone, US Foot",                 "CRS HARN/WY.WY-EF with datum modified to NSRS07",                     3730,
					              "NSRS11.WY-EF",  "NSRS 2011 Wyoming State Planes, East Zone, US Foot",                 "CRS HARN/WY.WY-EF with datum modified to NSRS11",                        0   },
	{               "HARN/WY.WY-W",    "HARN/WY",   2865,
					               "NSRS07.WY-W",  "NSRS 2007 Wyoming State Planes, West Zone, Meter",                   "CRS HARN/WY.WY-W with datum modified to NSRS07",                      3705,
					               "NSRS11.WY-W",  "NSRS 2011 Wyoming State Planes, West Zone, Meter",                   "CRS HARN/WY.WY-W with datum modified to NSRS11",                         0   },
	{              "HARN/WY.WY-WC",    "HARN/WY",   2864,
					              "NSRS07.WY-WC",  "NSRS 2007 Wyoming State Planes, West Central Zone, Meter",           "CRS HARN/WY.WY-WC with datum modified to NSRS07",                     3704,
					              "NSRS11.WY-WC",  "NSRS 2011 Wyoming State Planes, West Central Zone, Meter",           "CRS HARN/WY.WY-WC with datum modified to NSRS11",                        0   },
	{             "HARN/WY.WY-WCF",    "HARN/WY",   3757,
					             "NSRS07.WY-WCF",  "NSRS 2007 Wyoming State Planes, West Central Zone, US Foot",         "CRS HARN/WY.WY-WCF with datum modified to NSRS07",                    3732,
					             "NSRS11.WY-WCF",  "NSRS 2011 Wyoming State Planes, West Central Zone, US Foot",         "CRS HARN/WY.WY-WCF with datum modified to NSRS11",                       0   },
	{              "HARN/WY.WY-WF",    "HARN/WY",   3758,
					              "NSRS07.WY-WF",  "NSRS 2007 Wyoming State Planes, West Zone, US Foot",                 "CRS HARN/WY.WY-WF with datum modified to NSRS07",                     3733,
					              "NSRS11.WY-WF",  "NSRS 2011 Wyoming State Planes, West Zone, US Foot",                 "CRS HARN/WY.WY-WF with datum modified to NSRS11",                        0   },
	//{          "HARN/WI.AdamsWI-F",    "HARN/WI",      0,
	//				          "NSRS07.AdamsWI-F",  "",                                                                   "CRS HARN/WI.AdamsWI-F with datum modified to NSRS07",                    0,
	//				          "NSRS11.AdamsWI-F",  "",                                                                   "CRS HARN/WI.AdamsWI-F with datum modified to NSRS11",                    0   },
	//{         "HARN/WI.AdamsWI-IF",    "HARN/WI",      0,
	//				         "NSRS07.AdamsWI-IF",  "",                                                                   "CRS HARN/WI.AdamsWI-IF with datum modified to NSRS07",                   0,
	//				         "NSRS11.AdamsWI-IF",  "",                                                                   "CRS HARN/WI.AdamsWI-IF with datum modified to NSRS11",                   0   },
	//{          "HARN/WI.AdamsWI-M",    "HARN/WI",      0,
	//				          "NSRS07.AdamsWI-M",  "",                                                                   "CRS HARN/WI.AdamsWI-M with datum modified to NSRS07",                    0,
	//				          "NSRS11.AdamsWI-M",  "",                                                                   "CRS HARN/WI.AdamsWI-M with datum modified to NSRS11",                    0   },
	//{        "HARN/WI.AshlandWI-F",    "HARN/WI",      0,
	//				        "NSRS07.AshlandWI-F",  "",                                                                   "CRS HARN/WI.AshlandWI-F with datum modified to NSRS07",                  0,
	//				        "NSRS11.AshlandWI-F",  "",                                                                   "CRS HARN/WI.AshlandWI-F with datum modified to NSRS11",                  0   },
	//{       "HARN/WI.AshlandWI-IF",    "HARN/WI",      0,
	//				       "NSRS07.AshlandWI-IF",  "",                                                                   "CRS HARN/WI.AshlandWI-IF with datum modified to NSRS07",                 0,
	//				       "NSRS11.AshlandWI-IF",  "",                                                                   "CRS HARN/WI.AshlandWI-IF with datum modified to NSRS11",                 0   },
	//{        "HARN/WI.AshlandWI-M",    "HARN/WI",      0,
	//				        "NSRS07.AshlandWI-M",  "",                                                                   "CRS HARN/WI.AshlandWI-M with datum modified to NSRS07",                  0,
	//				        "NSRS11.AshlandWI-M",  "",                                                                   "CRS HARN/WI.AshlandWI-M with datum modified to NSRS11",                  0   },
	//{         "HARN/WI.BarronWI-F",    "HARN/WI",      0,
	//				         "NSRS07.BarronWI-F",  "",                                                                   "CRS HARN/WI.BarronWI-F with datum modified to NSRS07",                   0,
	//				         "NSRS11.BarronWI-F",  "",                                                                   "CRS HARN/WI.BarronWI-F with datum modified to NSRS11",                   0   },
	//{        "HARN/WI.BarronWI-IF",    "HARN/WI",      0,
	//				        "NSRS07.BarronWI-IF",  "",                                                                   "CRS HARN/WI.BarronWI-IF with datum modified to NSRS07",                  0,
	//				        "NSRS11.BarronWI-IF",  "",                                                                   "CRS HARN/WI.BarronWI-IF with datum modified to NSRS11",                  0   },
	//{         "HARN/WI.BarronWI-M",    "HARN/WI",      0,
	//				         "NSRS07.BarronWI-M",  "",                                                                   "CRS HARN/WI.BarronWI-M with datum modified to NSRS07",                   0,
	//				         "NSRS11.BarronWI-M",  "",                                                                   "CRS HARN/WI.BarronWI-M with datum modified to NSRS11",                   0   },
	//{       "HARN/WI.BayfieldWI-F",    "HARN/WI",      0,
	//				       "NSRS07.BayfieldWI-F",  "",                                                                   "CRS HARN/WI.BayfieldWI-F with datum modified to NSRS07",                 0,
	//				       "NSRS11.BayfieldWI-F",  "",                                                                   "CRS HARN/WI.BayfieldWI-F with datum modified to NSRS11",                 0   },
	//{      "HARN/WI.BayfieldWI-IF",    "HARN/WI",      0,
	//				      "NSRS07.BayfieldWI-IF",  "",                                                                   "CRS HARN/WI.BayfieldWI-IF with datum modified to NSRS07",                0,
	//				      "NSRS11.BayfieldWI-IF",  "",                                                                   "CRS HARN/WI.BayfieldWI-IF with datum modified to NSRS11",                0   },
	//{       "HARN/WI.BayfieldWI-M",    "HARN/WI",      0,
	//				       "NSRS07.BayfieldWI-M",  "",                                                                   "CRS HARN/WI.BayfieldWI-M with datum modified to NSRS07",                 0,
	//				       "NSRS11.BayfieldWI-M",  "",                                                                   "CRS HARN/WI.BayfieldWI-M with datum modified to NSRS11",                 0   },
	//{          "HARN/WI.BrownWI-F",    "HARN/WI",      0,
	//				          "NSRS07.BrownWI-F",  "",                                                                   "CRS HARN/WI.BrownWI-F with datum modified to NSRS07",                    0,
	//				          "NSRS11.BrownWI-F",  "",                                                                   "CRS HARN/WI.BrownWI-F with datum modified to NSRS11",                    0   },
	//{         "HARN/WI.BrownWI-IF",    "HARN/WI",      0,
	//				         "NSRS07.BrownWI-IF",  "",                                                                   "CRS HARN/WI.BrownWI-IF with datum modified to NSRS07",                   0,
	//				         "NSRS11.BrownWI-IF",  "",                                                                   "CRS HARN/WI.BrownWI-IF with datum modified to NSRS11",                   0   },
	//{          "HARN/WI.BrownWI-M",    "HARN/WI",      0,
	//				          "NSRS07.BrownWI-M",  "",                                                                   "CRS HARN/WI.BrownWI-M with datum modified to NSRS07",                    0,
	//				          "NSRS11.BrownWI-M",  "",                                                                   "CRS HARN/WI.BrownWI-M with datum modified to NSRS11",                    0   },
	//{        "HARN/WI.BuffaloWI-F",    "HARN/WI",      0,
	//				        "NSRS07.BuffaloWI-F",  "",                                                                   "CRS HARN/WI.BuffaloWI-F with datum modified to NSRS07",                  0,
	//				        "NSRS11.BuffaloWI-F",  "",                                                                   "CRS HARN/WI.BuffaloWI-F with datum modified to NSRS11",                  0   },
	//{       "HARN/WI.BuffaloWI-IF",    "HARN/WI",      0,
	//				       "NSRS07.BuffaloWI-IF",  "",                                                                   "CRS HARN/WI.BuffaloWI-IF with datum modified to NSRS07",                 0,
	//				       "NSRS11.BuffaloWI-IF",  "",                                                                   "CRS HARN/WI.BuffaloWI-IF with datum modified to NSRS11",                 0   },
	//{        "HARN/WI.BuffaloWI-M",    "HARN/WI",      0,
	//				        "NSRS07.BuffaloWI-M",  "",                                                                   "CRS HARN/WI.BuffaloWI-M with datum modified to NSRS07",                  0,
	//				        "NSRS11.BuffaloWI-M",  "",                                                                   "CRS HARN/WI.BuffaloWI-M with datum modified to NSRS11",                  0   },
	//{        "HARN/WI.BurnettWI-F",    "HARN/WI",      0,
	//				        "NSRS07.BurnettWI-F",  "",                                                                   "CRS HARN/WI.BurnettWI-F with datum modified to NSRS07",                  0,
	//				        "NSRS11.BurnettWI-F",  "",                                                                   "CRS HARN/WI.BurnettWI-F with datum modified to NSRS11",                  0   },
	//{       "HARN/WI.BurnettWI-IF",    "HARN/WI",      0,
	//				       "NSRS07.BurnettWI-IF",  "",                                                                   "CRS HARN/WI.BurnettWI-IF with datum modified to NSRS07",                 0,
	//				       "NSRS11.BurnettWI-IF",  "",                                                                   "CRS HARN/WI.BurnettWI-IF with datum modified to NSRS11",                 0   },
	//{        "HARN/WI.BurnettWI-M",    "HARN/WI",      0,
	//				        "NSRS07.BurnettWI-M",  "",                                                                   "CRS HARN/WI.BurnettWI-M with datum modified to NSRS07",                  0,
	//				        "NSRS11.BurnettWI-M",  "",                                                                   "CRS HARN/WI.BurnettWI-M with datum modified to NSRS11",                  0   },
	//{        "HARN/WI.CalumetWI-F",    "HARN/WI",      0,
	//				        "NSRS07.CalumetWI-F",  "",                                                                   "CRS HARN/WI.CalumetWI-F with datum modified to NSRS07",                  0,
	//				        "NSRS11.CalumetWI-F",  "",                                                                   "CRS HARN/WI.CalumetWI-F with datum modified to NSRS11",                  0   },
	//{       "HARN/WI.CalumetWI-IF",    "HARN/WI",      0,
	//				       "NSRS07.CalumetWI-IF",  "",                                                                   "CRS HARN/WI.CalumetWI-IF with datum modified to NSRS07",                 0,
	//				       "NSRS11.CalumetWI-IF",  "",                                                                   "CRS HARN/WI.CalumetWI-IF with datum modified to NSRS11",                 0   },
	//{        "HARN/WI.CalumetWI-M",    "HARN/WI",      0,
	//				        "NSRS07.CalumetWI-M",  "",                                                                   "CRS HARN/WI.CalumetWI-M with datum modified to NSRS07",                  0,
	//				        "NSRS11.CalumetWI-M",  "",                                                                   "CRS HARN/WI.CalumetWI-M with datum modified to NSRS11",                  0   },
	//{       "HARN/WI.ChippewaWI-F",    "HARN/WI",      0,
	//				       "NSRS07.ChippewaWI-F",  "",                                                                   "CRS HARN/WI.ChippewaWI-F with datum modified to NSRS07",                 0,
	//				       "NSRS11.ChippewaWI-F",  "",                                                                   "CRS HARN/WI.ChippewaWI-F with datum modified to NSRS11",                 0   },
	//{      "HARN/WI.ChippewaWI-IF",    "HARN/WI",      0,
	//				      "NSRS07.ChippewaWI-IF",  "",                                                                   "CRS HARN/WI.ChippewaWI-IF with datum modified to NSRS07",                0,
	//				      "NSRS11.ChippewaWI-IF",  "",                                                                   "CRS HARN/WI.ChippewaWI-IF with datum modified to NSRS11",                0   },
	//{       "HARN/WI.ChippewaWI-M",    "HARN/WI",      0,
	//				       "NSRS07.ChippewaWI-M",  "",                                                                   "CRS HARN/WI.ChippewaWI-M with datum modified to NSRS07",                 0,
	//				       "NSRS11.ChippewaWI-M",  "",                                                                   "CRS HARN/WI.ChippewaWI-M with datum modified to NSRS11",                 0   },
	//{          "HARN/WI.ClarkWI-F",    "HARN/WI",      0,
	//				          "NSRS07.ClarkWI-F",  "",                                                                   "CRS HARN/WI.ClarkWI-F with datum modified to NSRS07",                    0,
	//				          "NSRS11.ClarkWI-F",  "",                                                                   "CRS HARN/WI.ClarkWI-F with datum modified to NSRS11",                    0   },
	//{         "HARN/WI.ClarkWI-IF",    "HARN/WI",      0,
	//				         "NSRS07.ClarkWI-IF",  "",                                                                   "CRS HARN/WI.ClarkWI-IF with datum modified to NSRS07",                   0,
	//				         "NSRS11.ClarkWI-IF",  "",                                                                   "CRS HARN/WI.ClarkWI-IF with datum modified to NSRS11",                   0   },
	//{          "HARN/WI.ClarkWI-M",    "HARN/WI",      0,
	//				          "NSRS07.ClarkWI-M",  "",                                                                   "CRS HARN/WI.ClarkWI-M with datum modified to NSRS07",                    0,
	//				          "NSRS11.ClarkWI-M",  "",                                                                   "CRS HARN/WI.ClarkWI-M with datum modified to NSRS11",                    0   },
	//{       "HARN/WI.ColumbiaWI-F",    "HARN/WI",      0,
	//				       "NSRS07.ColumbiaWI-F",  "",                                                                   "CRS HARN/WI.ColumbiaWI-F with datum modified to NSRS07",                 0,
	//				       "NSRS11.ColumbiaWI-F",  "",                                                                   "CRS HARN/WI.ColumbiaWI-F with datum modified to NSRS11",                 0   },
	//{      "HARN/WI.ColumbiaWI-IF",    "HARN/WI",      0,
	//				      "NSRS07.ColumbiaWI-IF",  "",                                                                   "CRS HARN/WI.ColumbiaWI-IF with datum modified to NSRS07",                0,
	//				      "NSRS11.ColumbiaWI-IF",  "",                                                                   "CRS HARN/WI.ColumbiaWI-IF with datum modified to NSRS11",                0   },
	//{       "HARN/WI.ColumbiaWI-M",    "HARN/WI",      0,
	//				       "NSRS07.ColumbiaWI-M",  "",                                                                   "CRS HARN/WI.ColumbiaWI-M with datum modified to NSRS07",                 0,
	//				       "NSRS11.ColumbiaWI-M",  "",                                                                   "CRS HARN/WI.ColumbiaWI-M with datum modified to NSRS11",                 0   },
	//{     "HARN/WI.CrawfordWI-F/a",    "HARN/WI",      0,
	//				       "NSRS07.CrawfordWI-F",  "",                                                                   "CRS HARN/WI.CrawfordWI-F/a with datum modified to NSRS07",               0,
	//				       "NSRS11.CrawfordWI-F",  "",                                                                   "CRS HARN/WI.CrawfordWI-F/a with datum modified to NSRS11",               0   },
	//{    "HARN/WI.CrawfordWI-IF/a",    "HARN/WI",      0,
	//				      "NSRS07.CrawfordWI-IF",  "",                                                                   "CRS HARN/WI.CrawfordWI-IF/a with datum modified to NSRS07",              0,
	//				      "NSRS11.CrawfordWI-IF",  "",                                                                   "CRS HARN/WI.CrawfordWI-IF/a with datum modified to NSRS11",              0   },
	//{     "HARN/WI.CrawfordWI-M/a",    "HARN/WI",      0,
	//				       "NSRS07.CrawfordWI-M",  "",                                                                   "CRS HARN/WI.CrawfordWI-M/a with datum modified to NSRS07",               0,
	//				       "NSRS11.CrawfordWI-M",  "",                                                                   "CRS HARN/WI.CrawfordWI-M/a with datum modified to NSRS11",               0   },
	//{           "HARN/WI.DaneWI-F",    "HARN/WI",      0,
	//				           "NSRS07.DaneWI-F",  "",                                                                   "CRS HARN/WI.DaneWI-F with datum modified to NSRS07",                     0,
	//				           "NSRS11.DaneWI-F",  "",                                                                   "CRS HARN/WI.DaneWI-F with datum modified to NSRS11",                     0   },
	//{          "HARN/WI.DaneWI-IF",    "HARN/WI",      0,
	//				          "NSRS07.DaneWI-IF",  "",                                                                   "CRS HARN/WI.DaneWI-IF with datum modified to NSRS07",                    0,
	//				          "NSRS11.DaneWI-IF",  "",                                                                   "CRS HARN/WI.DaneWI-IF with datum modified to NSRS11",                    0   },
	//{           "HARN/WI.DaneWI-M",    "HARN/WI",      0,
	//				           "NSRS07.DaneWI-M",  "",                                                                   "CRS HARN/WI.DaneWI-M with datum modified to NSRS07",                     0,
	//				           "NSRS11.DaneWI-M",  "",                                                                   "CRS HARN/WI.DaneWI-M with datum modified to NSRS11",                     0   },
	//{          "HARN/WI.DodgeWI-F",    "HARN/WI",      0,
	//				          "NSRS07.DodgeWI-F",  "",                                                                   "CRS HARN/WI.DodgeWI-F with datum modified to NSRS07",                    0,
	//				          "NSRS11.DodgeWI-F",  "",                                                                   "CRS HARN/WI.DodgeWI-F with datum modified to NSRS11",                    0   },
	//{         "HARN/WI.DodgeWI-IF",    "HARN/WI",      0,
	//				         "NSRS07.DodgeWI-IF",  "",                                                                   "CRS HARN/WI.DodgeWI-IF with datum modified to NSRS07",                   0,
	//				         "NSRS11.DodgeWI-IF",  "",                                                                   "CRS HARN/WI.DodgeWI-IF with datum modified to NSRS11",                   0   },
	//{          "HARN/WI.DodgeWI-M",    "HARN/WI",      0,
	//				          "NSRS07.DodgeWI-M",  "",                                                                   "CRS HARN/WI.DodgeWI-M with datum modified to NSRS07",                    0,
	//				          "NSRS11.DodgeWI-M",  "",                                                                   "CRS HARN/WI.DodgeWI-M with datum modified to NSRS11",                    0   },
	//{           "HARN/WI.DoorWI-F",    "HARN/WI",      0,
	//				           "NSRS07.DoorWI-F",  "",                                                                   "CRS HARN/WI.DoorWI-F with datum modified to NSRS07",                     0,
	//				           "NSRS11.DoorWI-F",  "",                                                                   "CRS HARN/WI.DoorWI-F with datum modified to NSRS11",                     0   },
	//{          "HARN/WI.DoorWI-IF",    "HARN/WI",      0,
	//				          "NSRS07.DoorWI-IF",  "",                                                                   "CRS HARN/WI.DoorWI-IF with datum modified to NSRS07",                    0,
	//				          "NSRS11.DoorWI-IF",  "",                                                                   "CRS HARN/WI.DoorWI-IF with datum modified to NSRS11",                    0   },
	//{           "HARN/WI.DoorWI-M",    "HARN/WI",      0,
	//				           "NSRS07.DoorWI-M",  "",                                                                   "CRS HARN/WI.DoorWI-M with datum modified to NSRS07",                     0,
	//				           "NSRS11.DoorWI-M",  "",                                                                   "CRS HARN/WI.DoorWI-M with datum modified to NSRS11",                     0   },
	//{        "HARN/WI.DouglasWI-F",    "HARN/WI",      0,
	//				        "NSRS07.DouglasWI-F",  "",                                                                   "CRS HARN/WI.DouglasWI-F with datum modified to NSRS07",                  0,
	//				        "NSRS11.DouglasWI-F",  "",                                                                   "CRS HARN/WI.DouglasWI-F with datum modified to NSRS11",                  0   },
	//{       "HARN/WI.DouglasWI-IF",    "HARN/WI",      0,
	//				       "NSRS07.DouglasWI-IF",  "",                                                                   "CRS HARN/WI.DouglasWI-IF with datum modified to NSRS07",                 0,
	//				       "NSRS11.DouglasWI-IF",  "",                                                                   "CRS HARN/WI.DouglasWI-IF with datum modified to NSRS11",                 0   },
	//{        "HARN/WI.DouglasWI-M",    "HARN/WI",      0,
	//				        "NSRS07.DouglasWI-M",  "",                                                                   "CRS HARN/WI.DouglasWI-M with datum modified to NSRS07",                  0,
	//				        "NSRS11.DouglasWI-M",  "",                                                                   "CRS HARN/WI.DouglasWI-M with datum modified to NSRS11",                  0   },
	//{           "HARN/WI.DunnWI-F",    "HARN/WI",      0,
	//				           "NSRS07.DunnWI-F",  "",                                                                   "CRS HARN/WI.DunnWI-F with datum modified to NSRS07",                     0,
	//				           "NSRS11.DunnWI-F",  "",                                                                   "CRS HARN/WI.DunnWI-F with datum modified to NSRS11",                     0   },
	//{          "HARN/WI.DunnWI-IF",    "HARN/WI",      0,
	//				          "NSRS07.DunnWI-IF",  "",                                                                   "CRS HARN/WI.DunnWI-IF with datum modified to NSRS07",                    0,
	//				          "NSRS11.DunnWI-IF",  "",                                                                   "CRS HARN/WI.DunnWI-IF with datum modified to NSRS11",                    0   },
	//{           "HARN/WI.DunnWI-M",    "HARN/WI",      0,
	//				           "NSRS07.DunnWI-M",  "",                                                                   "CRS HARN/WI.DunnWI-M with datum modified to NSRS07",                     0,
	//				           "NSRS11.DunnWI-M",  "",                                                                   "CRS HARN/WI.DunnWI-M with datum modified to NSRS11",                     0   },
	//{      "HARN/WI.EauClaireWI-F",    "HARN/WI",      0,
	//				      "NSRS07.EauClaireWI-F",  "",                                                                   "CRS HARN/WI.EauClaireWI-F with datum modified to NSRS07",                0,
	//				      "NSRS11.EauClaireWI-F",  "",                                                                   "CRS HARN/WI.EauClaireWI-F with datum modified to NSRS11",                0   },
	//{     "HARN/WI.EauClaireWI-IF",    "HARN/WI",      0,
	//				     "NSRS07.EauClaireWI-IF",  "",                                                                   "CRS HARN/WI.EauClaireWI-IF with datum modified to NSRS07",               0,
	//				     "NSRS11.EauClaireWI-IF",  "",                                                                   "CRS HARN/WI.EauClaireWI-IF with datum modified to NSRS11",               0   },
	//{      "HARN/WI.EauClaireWI-M",    "HARN/WI",      0,
	//				      "NSRS07.EauClaireWI-M",  "",                                                                   "CRS HARN/WI.EauClaireWI-M with datum modified to NSRS07",                0,
	//				      "NSRS11.EauClaireWI-M",  "",                                                                   "CRS HARN/WI.EauClaireWI-M with datum modified to NSRS11",                0   },
	//{       "HARN/WI.FlorenceWI-F",    "HARN/WI",      0,
	//				       "NSRS07.FlorenceWI-F",  "",                                                                   "CRS HARN/WI.FlorenceWI-F with datum modified to NSRS07",                 0,
	//				       "NSRS11.FlorenceWI-F",  "",                                                                   "CRS HARN/WI.FlorenceWI-F with datum modified to NSRS11",                 0   },
	//{      "HARN/WI.FlorenceWI-IF",    "HARN/WI",      0,
	//				      "NSRS07.FlorenceWI-IF",  "",                                                                   "CRS HARN/WI.FlorenceWI-IF with datum modified to NSRS07",                0,
	//				      "NSRS11.FlorenceWI-IF",  "",                                                                   "CRS HARN/WI.FlorenceWI-IF with datum modified to NSRS11",                0   },
	//{       "HARN/WI.FlorenceWI-M",    "HARN/WI",      0,
	//				       "NSRS07.FlorenceWI-M",  "",                                                                   "CRS HARN/WI.FlorenceWI-M with datum modified to NSRS07",                 0,
	//				       "NSRS11.FlorenceWI-M",  "",                                                                   "CRS HARN/WI.FlorenceWI-M with datum modified to NSRS11",                 0   },
	//{      "HARN/WI.FondDuLacWI-F",    "HARN/WI",      0,
	//				      "NSRS07.FondDuLacWI-F",  "",                                                                   "CRS HARN/WI.FondDuLacWI-F with datum modified to NSRS07",                0,
	//				      "NSRS11.FondDuLacWI-F",  "",                                                                   "CRS HARN/WI.FondDuLacWI-F with datum modified to NSRS11",                0   },
	//{     "HARN/WI.FondDuLacWI-IF",    "HARN/WI",      0,
	//				     "NSRS07.FondDuLacWI-IF",  "",                                                                   "CRS HARN/WI.FondDuLacWI-IF with datum modified to NSRS07",               0,
	//				     "NSRS11.FondDuLacWI-IF",  "",                                                                   "CRS HARN/WI.FondDuLacWI-IF with datum modified to NSRS11",               0   },
	//{      "HARN/WI.FondDuLacWI-M",    "HARN/WI",      0,
	//				      "NSRS07.FondDuLacWI-M",  "",                                                                   "CRS HARN/WI.FondDuLacWI-M with datum modified to NSRS07",                0,
	//				      "NSRS11.FondDuLacWI-M",  "",                                                                   "CRS HARN/WI.FondDuLacWI-M with datum modified to NSRS11",                0   },
	//{         "HARN/WI.ForestWI-F",    "HARN/WI",      0,
	//				         "NSRS07.ForestWI-F",  "",                                                                   "CRS HARN/WI.ForestWI-F with datum modified to NSRS07",                   0,
	//				         "NSRS11.ForestWI-F",  "",                                                                   "CRS HARN/WI.ForestWI-F with datum modified to NSRS11",                   0   },
	//{        "HARN/WI.ForestWI-IF",    "HARN/WI",      0,
	//				        "NSRS07.ForestWI-IF",  "",                                                                   "CRS HARN/WI.ForestWI-IF with datum modified to NSRS07",                  0,
	//				        "NSRS11.ForestWI-IF",  "",                                                                   "CRS HARN/WI.ForestWI-IF with datum modified to NSRS11",                  0   },
	//{         "HARN/WI.ForestWI-M",    "HARN/WI",      0,
	//				         "NSRS07.ForestWI-M",  "",                                                                   "CRS HARN/WI.ForestWI-M with datum modified to NSRS07",                   0,
	//				         "NSRS11.ForestWI-M",  "",                                                                   "CRS HARN/WI.ForestWI-M with datum modified to NSRS11",                   0   },
	//{          "HARN/WI.GrantWI-F",    "HARN/WI",      0,
	//				          "NSRS07.GrantWI-F",  "",                                                                   "CRS HARN/WI.GrantWI-F with datum modified to NSRS07",                    0,
	//				          "NSRS11.GrantWI-F",  "",                                                                   "CRS HARN/WI.GrantWI-F with datum modified to NSRS11",                    0   },
	//{         "HARN/WI.GrantWI-IF",    "HARN/WI",      0,
	//				         "NSRS07.GrantWI-IF",  "",                                                                   "CRS HARN/WI.GrantWI-IF with datum modified to NSRS07",                   0,
	//				         "NSRS11.GrantWI-IF",  "",                                                                   "CRS HARN/WI.GrantWI-IF with datum modified to NSRS11",                   0   },
	//{          "HARN/WI.GrantWI-M",    "HARN/WI",      0,
	//				          "NSRS07.GrantWI-M",  "",                                                                   "CRS HARN/WI.GrantWI-M with datum modified to NSRS07",                    0,
	//				          "NSRS11.GrantWI-M",  "",                                                                   "CRS HARN/WI.GrantWI-M with datum modified to NSRS11",                    0   },
	//{      "HARN/WI.GreenLakeWI-F",    "HARN/WI",      0,
	//				      "NSRS07.GreenLakeWI-F",  "",                                                                   "CRS HARN/WI.GreenLakeWI-F with datum modified to NSRS07",                0,
	//				      "NSRS11.GreenLakeWI-F",  "",                                                                   "CRS HARN/WI.GreenLakeWI-F with datum modified to NSRS11",                0   },
	//{     "HARN/WI.GreenLakeWI-IF",    "HARN/WI",      0,
	//				     "NSRS07.GreenLakeWI-IF",  "",                                                                   "CRS HARN/WI.GreenLakeWI-IF with datum modified to NSRS07",               0,
	//				     "NSRS11.GreenLakeWI-IF",  "",                                                                   "CRS HARN/WI.GreenLakeWI-IF with datum modified to NSRS11",               0   },
	//{      "HARN/WI.GreenLakeWI-M",    "HARN/WI",      0,
	//				      "NSRS07.GreenLakeWI-M",  "",                                                                   "CRS HARN/WI.GreenLakeWI-M with datum modified to NSRS07",                0,
	//				      "NSRS11.GreenLakeWI-M",  "",                                                                   "CRS HARN/WI.GreenLakeWI-M with datum modified to NSRS11",                0   },
	//{          "HARN/WI.GreenWI-F",    "HARN/WI",      0,
	//				          "NSRS07.GreenWI-F",  "",                                                                   "CRS HARN/WI.GreenWI-F with datum modified to NSRS07",                    0,
	//				          "NSRS11.GreenWI-F",  "",                                                                   "CRS HARN/WI.GreenWI-F with datum modified to NSRS11",                    0   },
	//{         "HARN/WI.GreenWI-IF",    "HARN/WI",      0,
	//				         "NSRS07.GreenWI-IF",  "",                                                                   "CRS HARN/WI.GreenWI-IF with datum modified to NSRS07",                   0,
	//				         "NSRS11.GreenWI-IF",  "",                                                                   "CRS HARN/WI.GreenWI-IF with datum modified to NSRS11",                   0   },
	//{          "HARN/WI.GreenWI-M",    "HARN/WI",      0,
	//				          "NSRS07.GreenWI-M",  "",                                                                   "CRS HARN/WI.GreenWI-M with datum modified to NSRS07",                    0,
	//				          "NSRS11.GreenWI-M",  "",                                                                   "CRS HARN/WI.GreenWI-M with datum modified to NSRS11",                    0   },
	//{           "HARN/WI.IowaWI-F",    "HARN/WI",      0,
	//				           "NSRS07.IowaWI-F",  "",                                                                   "CRS HARN/WI.IowaWI-F with datum modified to NSRS07",                     0,
	//				           "NSRS11.IowaWI-F",  "",                                                                   "CRS HARN/WI.IowaWI-F with datum modified to NSRS11",                     0   },
	//{          "HARN/WI.IowaWI-IF",    "HARN/WI",      0,
	//				          "NSRS07.IowaWI-IF",  "",                                                                   "CRS HARN/WI.IowaWI-IF with datum modified to NSRS07",                    0,
	//				          "NSRS11.IowaWI-IF",  "",                                                                   "CRS HARN/WI.IowaWI-IF with datum modified to NSRS11",                    0   },
	//{           "HARN/WI.IowaWI-M",    "HARN/WI",      0,
	//				           "NSRS07.IowaWI-M",  "",                                                                   "CRS HARN/WI.IowaWI-M with datum modified to NSRS07",                     0,
	//				           "NSRS11.IowaWI-M",  "",                                                                   "CRS HARN/WI.IowaWI-M with datum modified to NSRS11",                     0   },
	//{           "HARN/WI.IronWI-F",    "HARN/WI",      0,
	//				           "NSRS07.IronWI-F",  "",                                                                   "CRS HARN/WI.IronWI-F with datum modified to NSRS07",                     0,
	//				           "NSRS11.IronWI-F",  "",                                                                   "CRS HARN/WI.IronWI-F with datum modified to NSRS11",                     0   },
	//{          "HARN/WI.IronWI-IF",    "HARN/WI",      0,
	//				          "NSRS07.IronWI-IF",  "",                                                                   "CRS HARN/WI.IronWI-IF with datum modified to NSRS07",                    0,
	//				          "NSRS11.IronWI-IF",  "",                                                                   "CRS HARN/WI.IronWI-IF with datum modified to NSRS11",                    0   },
	//{           "HARN/WI.IronWI-M",    "HARN/WI",      0,
	//				           "NSRS07.IronWI-M",  "",                                                                   "CRS HARN/WI.IronWI-M with datum modified to NSRS07",                     0,
	//				           "NSRS11.IronWI-M",  "",                                                                   "CRS HARN/WI.IronWI-M with datum modified to NSRS11",                     0   },
	//{        "HARN/WI.JacksonWI-F",    "HARN/WI",      0,
	//				        "NSRS07.JacksonWI-F",  "",                                                                   "CRS HARN/WI.JacksonWI-F with datum modified to NSRS07",                  0,
	//				        "NSRS11.JacksonWI-F",  "",                                                                   "CRS HARN/WI.JacksonWI-F with datum modified to NSRS11",                  0   },
	//{       "HARN/WI.JacksonWI-IF",    "HARN/WI",      0,
	//				       "NSRS07.JacksonWI-IF",  "",                                                                   "CRS HARN/WI.JacksonWI-IF with datum modified to NSRS07",                 0,
	//				       "NSRS11.JacksonWI-IF",  "",                                                                   "CRS HARN/WI.JacksonWI-IF with datum modified to NSRS11",                 0   },
	//{        "HARN/WI.JacksonWI-M",    "HARN/WI",      0,
	//				        "NSRS07.JacksonWI-M",  "",                                                                   "CRS HARN/WI.JacksonWI-M with datum modified to NSRS07",                  0,
	//				        "NSRS11.JacksonWI-M",  "",                                                                   "CRS HARN/WI.JacksonWI-M with datum modified to NSRS11",                  0   },
	//{      "HARN/WI.JeffersonWI-F",    "HARN/WI",      0,
	//				      "NSRS07.JeffersonWI-F",  "",                                                                   "CRS HARN/WI.JeffersonWI-F with datum modified to NSRS07",                0,
	//				      "NSRS11.JeffersonWI-F",  "",                                                                   "CRS HARN/WI.JeffersonWI-F with datum modified to NSRS11",                0   },
	//{     "HARN/WI.JeffersonWI-IF",    "HARN/WI",      0,
	//				     "NSRS07.JeffersonWI-IF",  "",                                                                   "CRS HARN/WI.JeffersonWI-IF with datum modified to NSRS07",               0,
	//				     "NSRS11.JeffersonWI-IF",  "",                                                                   "CRS HARN/WI.JeffersonWI-IF with datum modified to NSRS11",               0   },
	//{      "HARN/WI.JeffersonWI-M",    "HARN/WI",      0,
	//				      "NSRS07.JeffersonWI-M",  "",                                                                   "CRS HARN/WI.JeffersonWI-M with datum modified to NSRS07",                0,
	//				      "NSRS11.JeffersonWI-M",  "",                                                                   "CRS HARN/WI.JeffersonWI-M with datum modified to NSRS11",                0   },
	//{         "HARN/WI.JuneauWI-F",    "HARN/WI",      0,
	//				         "NSRS07.JuneauWI-F",  "",                                                                   "CRS HARN/WI.JuneauWI-F with datum modified to NSRS07",                   0,
	//				         "NSRS11.JuneauWI-F",  "",                                                                   "CRS HARN/WI.JuneauWI-F with datum modified to NSRS11",                   0   },
	//{        "HARN/WI.JuneauWI-IF",    "HARN/WI",      0,
	//				        "NSRS07.JuneauWI-IF",  "",                                                                   "CRS HARN/WI.JuneauWI-IF with datum modified to NSRS07",                  0,
	//				        "NSRS11.JuneauWI-IF",  "",                                                                   "CRS HARN/WI.JuneauWI-IF with datum modified to NSRS11",                  0   },
	//{         "HARN/WI.JuneauWI-M",    "HARN/WI",      0,
	//				         "NSRS07.JuneauWI-M",  "",                                                                   "CRS HARN/WI.JuneauWI-M with datum modified to NSRS07",                   0,
	//				         "NSRS11.JuneauWI-M",  "",                                                                   "CRS HARN/WI.JuneauWI-M with datum modified to NSRS11",                   0   },
	//{        "HARN/WI.KenoshaWI-F",    "HARN/WI",      0,
	//				        "NSRS07.KenoshaWI-F",  "",                                                                   "CRS HARN/WI.KenoshaWI-F with datum modified to NSRS07",                  0,
	//				        "NSRS11.KenoshaWI-F",  "",                                                                   "CRS HARN/WI.KenoshaWI-F with datum modified to NSRS11",                  0   },
	//{       "HARN/WI.KenoshaWI-IF",    "HARN/WI",      0,
	//				       "NSRS07.KenoshaWI-IF",  "",                                                                   "CRS HARN/WI.KenoshaWI-IF with datum modified to NSRS07",                 0,
	//				       "NSRS11.KenoshaWI-IF",  "",                                                                   "CRS HARN/WI.KenoshaWI-IF with datum modified to NSRS11",                 0   },
	//{        "HARN/WI.KenoshaWI-M",    "HARN/WI",      0,
	//				        "NSRS07.KenoshaWI-M",  "",                                                                   "CRS HARN/WI.KenoshaWI-M with datum modified to NSRS07",                  0,
	//				        "NSRS11.KenoshaWI-M",  "",                                                                   "CRS HARN/WI.KenoshaWI-M with datum modified to NSRS11",                  0   },
	//{       "HARN/WI.KewauneeWI-F",    "HARN/WI",      0,
	//				       "NSRS07.KewauneeWI-F",  "",                                                                   "CRS HARN/WI.KewauneeWI-F with datum modified to NSRS07",                 0,
	//				       "NSRS11.KewauneeWI-F",  "",                                                                   "CRS HARN/WI.KewauneeWI-F with datum modified to NSRS11",                 0   },
	//{      "HARN/WI.KewauneeWI-IF",    "HARN/WI",      0,
	//				      "NSRS07.KewauneeWI-IF",  "",                                                                   "CRS HARN/WI.KewauneeWI-IF with datum modified to NSRS07",                0,
	//				      "NSRS11.KewauneeWI-IF",  "",                                                                   "CRS HARN/WI.KewauneeWI-IF with datum modified to NSRS11",                0   },
	//{       "HARN/WI.KewauneeWI-M",    "HARN/WI",      0,
	//				       "NSRS07.KewauneeWI-M",  "",                                                                   "CRS HARN/WI.KewauneeWI-M with datum modified to NSRS07",                 0,
	//				       "NSRS11.KewauneeWI-M",  "",                                                                   "CRS HARN/WI.KewauneeWI-M with datum modified to NSRS11",                 0   },
	//{       "HARN/WI.LangladeWI-F",    "HARN/WI",      0,
	//				       "NSRS07.LangladeWI-F",  "",                                                                   "CRS HARN/WI.LangladeWI-F with datum modified to NSRS07",                 0,
	//				       "NSRS11.LangladeWI-F",  "",                                                                   "CRS HARN/WI.LangladeWI-F with datum modified to NSRS11",                 0   },
	//{      "HARN/WI.LangladeWI-IF",    "HARN/WI",      0,
	//				      "NSRS07.LangladeWI-IF",  "",                                                                   "CRS HARN/WI.LangladeWI-IF with datum modified to NSRS07",                0,
	//				      "NSRS11.LangladeWI-IF",  "",                                                                   "CRS HARN/WI.LangladeWI-IF with datum modified to NSRS11",                0   },
	//{       "HARN/WI.LangladeWI-M",    "HARN/WI",      0,
	//				       "NSRS07.LangladeWI-M",  "",                                                                   "CRS HARN/WI.LangladeWI-M with datum modified to NSRS07",                 0,
	//				       "NSRS11.LangladeWI-M",  "",                                                                   "CRS HARN/WI.LangladeWI-M with datum modified to NSRS11",                 0   },
	//{       "HARN/WI.LaCrosseWI-F",    "HARN/WI",      0,
	//				       "NSRS07.LaCrosseWI-F",  "",                                                                   "CRS HARN/WI.LaCrosseWI-F with datum modified to NSRS07",                 0,
	//				       "NSRS11.LaCrosseWI-F",  "",                                                                   "CRS HARN/WI.LaCrosseWI-F with datum modified to NSRS11",                 0   },
	//{      "HARN/WI.LaCrosseWI-IF",    "HARN/WI",      0,
	//				      "NSRS07.LaCrosseWI-IF",  "",                                                                   "CRS HARN/WI.LaCrosseWI-IF with datum modified to NSRS07",                0,
	//				      "NSRS11.LaCrosseWI-IF",  "",                                                                   "CRS HARN/WI.LaCrosseWI-IF with datum modified to NSRS11",                0   },
	//{       "HARN/WI.LaCrosseWI-M",    "HARN/WI",      0,
	//				       "NSRS07.LaCrosseWI-M",  "",                                                                   "CRS HARN/WI.LaCrosseWI-M with datum modified to NSRS07",                 0,
	//				       "NSRS11.LaCrosseWI-M",  "",                                                                   "CRS HARN/WI.LaCrosseWI-M with datum modified to NSRS11",                 0   },
	//{      "HARN/WI.LafayetteWI-F",    "HARN/WI",      0,
	//				      "NSRS07.LafayetteWI-F",  "",                                                                   "CRS HARN/WI.LafayetteWI-F with datum modified to NSRS07",                0,
	//				      "NSRS11.LafayetteWI-F",  "",                                                                   "CRS HARN/WI.LafayetteWI-F with datum modified to NSRS11",                0   },
	//{     "HARN/WI.LafayetteWI-IF",    "HARN/WI",      0,
	//				     "NSRS07.LafayetteWI-IF",  "",                                                                   "CRS HARN/WI.LafayetteWI-IF with datum modified to NSRS07",               0,
	//				     "NSRS11.LafayetteWI-IF",  "",                                                                   "CRS HARN/WI.LafayetteWI-IF with datum modified to NSRS11",               0   },
	//{      "HARN/WI.LafayetteWI-M",    "HARN/WI",      0,
	//				      "NSRS07.LafayetteWI-M",  "",                                                                   "CRS HARN/WI.LafayetteWI-M with datum modified to NSRS07",                0,
	//				      "NSRS11.LafayetteWI-M",  "",                                                                   "CRS HARN/WI.LafayetteWI-M with datum modified to NSRS11",                0   },
	//{        "HARN/WI.LincolnWI-F",    "HARN/WI",      0,
	//				        "NSRS07.LincolnWI-F",  "",                                                                   "CRS HARN/WI.LincolnWI-F with datum modified to NSRS07",                  0,
	//				        "NSRS11.LincolnWI-F",  "",                                                                   "CRS HARN/WI.LincolnWI-F with datum modified to NSRS11",                  0   },
	//{       "HARN/WI.LincolnWI-IF",    "HARN/WI",      0,
	//				       "NSRS07.LincolnWI-IF",  "",                                                                   "CRS HARN/WI.LincolnWI-IF with datum modified to NSRS07",                 0,
	//				       "NSRS11.LincolnWI-IF",  "",                                                                   "CRS HARN/WI.LincolnWI-IF with datum modified to NSRS11",                 0   },
	//{        "HARN/WI.LincolnWI-M",    "HARN/WI",      0,
	//				        "NSRS07.LincolnWI-M",  "",                                                                   "CRS HARN/WI.LincolnWI-M with datum modified to NSRS07",                  0,
	//				        "NSRS11.LincolnWI-M",  "",                                                                   "CRS HARN/WI.LincolnWI-M with datum modified to NSRS11",                  0   },
	//{      "HARN/WI.MilwaukeeWI-F",    "HARN/WI",      0,
	//				      "NSRS07.MilwaukeeWI-F",  "",                                                                   "CRS HARN/WI.MilwaukeeWI-F with datum modified to NSRS07",                0,
	//				      "NSRS11.MilwaukeeWI-F",  "",                                                                   "CRS HARN/WI.MilwaukeeWI-F with datum modified to NSRS11",                0   },
	//{     "HARN/WI.MilwaukeeWI-IF",    "HARN/WI",      0,
	//				     "NSRS07.MilwaukeeWI-IF",  "",                                                                   "CRS HARN/WI.MilwaukeeWI-IF with datum modified to NSRS07",               0,
	//				     "NSRS11.MilwaukeeWI-IF",  "",                                                                   "CRS HARN/WI.MilwaukeeWI-IF with datum modified to NSRS11",               0   },
	//{      "HARN/WI.MilwaukeeWI-M",    "HARN/WI",      0,
	//				      "NSRS07.MilwaukeeWI-M",  "",                                                                   "CRS HARN/WI.MilwaukeeWI-M with datum modified to NSRS07",                0,
	//				      "NSRS11.MilwaukeeWI-M",  "",                                                                   "CRS HARN/WI.MilwaukeeWI-M with datum modified to NSRS11",                0   },
	//{      "HARN/WI.ManitowocWI-F",    "HARN/WI",      0,
	//				      "NSRS07.ManitowocWI-F",  "",                                                                   "CRS HARN/WI.ManitowocWI-F with datum modified to NSRS07",                0,
	//				      "NSRS11.ManitowocWI-F",  "",                                                                   "CRS HARN/WI.ManitowocWI-F with datum modified to NSRS11",                0   },
	//{     "HARN/WI.ManitowocWI-IF",    "HARN/WI",      0,
	//				     "NSRS07.ManitowocWI-IF",  "",                                                                   "CRS HARN/WI.ManitowocWI-IF with datum modified to NSRS07",               0,
	//				     "NSRS11.ManitowocWI-IF",  "",                                                                   "CRS HARN/WI.ManitowocWI-IF with datum modified to NSRS11",               0   },
	//{      "HARN/WI.ManitowocWI-M",    "HARN/WI",      0,
	//				      "NSRS07.ManitowocWI-M",  "",                                                                   "CRS HARN/WI.ManitowocWI-M with datum modified to NSRS07",                0,
	//				      "NSRS11.ManitowocWI-M",  "",                                                                   "CRS HARN/WI.ManitowocWI-M with datum modified to NSRS11",                0   },
	//{       "HARN/WI.MarathonWI-F",    "HARN/WI",      0,
	//				       "NSRS07.MarathonWI-F",  "",                                                                   "CRS HARN/WI.MarathonWI-F with datum modified to NSRS07",                 0,
	//				       "NSRS11.MarathonWI-F",  "",                                                                   "CRS HARN/WI.MarathonWI-F with datum modified to NSRS11",                 0   },
	//{      "HARN/WI.MarathonWI-IF",    "HARN/WI",      0,
	//				      "NSRS07.MarathonWI-IF",  "",                                                                   "CRS HARN/WI.MarathonWI-IF with datum modified to NSRS07",                0,
	//				      "NSRS11.MarathonWI-IF",  "",                                                                   "CRS HARN/WI.MarathonWI-IF with datum modified to NSRS11",                0   },
	//{       "HARN/WI.MarathonWI-M",    "HARN/WI",      0,
	//				       "NSRS07.MarathonWI-M",  "",                                                                   "CRS HARN/WI.MarathonWI-M with datum modified to NSRS07",                 0,
	//				       "NSRS11.MarathonWI-M",  "",                                                                   "CRS HARN/WI.MarathonWI-M with datum modified to NSRS11",                 0   },
	//{      "HARN/WI.MarinetteWI-F",    "HARN/WI",      0,
	//				      "NSRS07.MarinetteWI-F",  "",                                                                   "CRS HARN/WI.MarinetteWI-F with datum modified to NSRS07",                0,
	//				      "NSRS11.MarinetteWI-F",  "",                                                                   "CRS HARN/WI.MarinetteWI-F with datum modified to NSRS11",                0   },
	//{     "HARN/WI.MarinetteWI-IF",    "HARN/WI",      0,
	//				     "NSRS07.MarinetteWI-IF",  "",                                                                   "CRS HARN/WI.MarinetteWI-IF with datum modified to NSRS07",               0,
	//				     "NSRS11.MarinetteWI-IF",  "",                                                                   "CRS HARN/WI.MarinetteWI-IF with datum modified to NSRS11",               0   },
	//{      "HARN/WI.MarinetteWI-M",    "HARN/WI",      0,
	//				      "NSRS07.MarinetteWI-M",  "",                                                                   "CRS HARN/WI.MarinetteWI-M with datum modified to NSRS07",                0,
	//				      "NSRS11.MarinetteWI-M",  "",                                                                   "CRS HARN/WI.MarinetteWI-M with datum modified to NSRS11",                0   },
	//{      "HARN/WI.MarquetteWI-F",    "HARN/WI",      0,
	//				      "NSRS07.MarquetteWI-F",  "",                                                                   "CRS HARN/WI.MarquetteWI-F with datum modified to NSRS07",                0,
	//				      "NSRS11.MarquetteWI-F",  "",                                                                   "CRS HARN/WI.MarquetteWI-F with datum modified to NSRS11",                0   },
	//{     "HARN/WI.MarquetteWI-IF",    "HARN/WI",      0,
	//				     "NSRS07.MarquetteWI-IF",  "",                                                                   "CRS HARN/WI.MarquetteWI-IF with datum modified to NSRS07",               0,
	//				     "NSRS11.MarquetteWI-IF",  "",                                                                   "CRS HARN/WI.MarquetteWI-IF with datum modified to NSRS11",               0   },
	//{      "HARN/WI.MarquetteWI-M",    "HARN/WI",      0,
	//				      "NSRS07.MarquetteWI-M",  "",                                                                   "CRS HARN/WI.MarquetteWI-M with datum modified to NSRS07",                0,
	//				      "NSRS11.MarquetteWI-M",  "",                                                                   "CRS HARN/WI.MarquetteWI-M with datum modified to NSRS11",                0   },
	//{      "HARN/WI.MenomineeWI-F",    "HARN/WI",      0,
	//				      "NSRS07.MenomineeWI-F",  "",                                                                   "CRS HARN/WI.MenomineeWI-F with datum modified to NSRS07",                0,
	//				      "NSRS11.MenomineeWI-F",  "",                                                                   "CRS HARN/WI.MenomineeWI-F with datum modified to NSRS11",                0   },
	//{     "HARN/WI.MenomineeWI-IF",    "HARN/WI",      0,
	//				     "NSRS07.MenomineeWI-IF",  "",                                                                   "CRS HARN/WI.MenomineeWI-IF with datum modified to NSRS07",               0,
	//				     "NSRS11.MenomineeWI-IF",  "",                                                                   "CRS HARN/WI.MenomineeWI-IF with datum modified to NSRS11",               0   },
	//{      "HARN/WI.MenomineeWI-M",    "HARN/WI",      0,
	//				      "NSRS07.MenomineeWI-M",  "",                                                                   "CRS HARN/WI.MenomineeWI-M with datum modified to NSRS07",                0,
	//				      "NSRS11.MenomineeWI-M",  "",                                                                   "CRS HARN/WI.MenomineeWI-M with datum modified to NSRS11",                0   },
	//{         "HARN/WI.MonroeWI-F",    "HARN/WI",      0,
	//				         "NSRS07.MonroeWI-F",  "",                                                                   "CRS HARN/WI.MonroeWI-F with datum modified to NSRS07",                   0,
	//				         "NSRS11.MonroeWI-F",  "",                                                                   "CRS HARN/WI.MonroeWI-F with datum modified to NSRS11",                   0   },
	//{        "HARN/WI.MonroeWI-IF",    "HARN/WI",      0,
	//				        "NSRS07.MonroeWI-IF",  "",                                                                   "CRS HARN/WI.MonroeWI-IF with datum modified to NSRS07",                  0,
	//				        "NSRS11.MonroeWI-IF",  "",                                                                   "CRS HARN/WI.MonroeWI-IF with datum modified to NSRS11",                  0   },
	//{         "HARN/WI.MonroeWI-M",    "HARN/WI",      0,
	//				         "NSRS07.MonroeWI-M",  "",                                                                   "CRS HARN/WI.MonroeWI-M with datum modified to NSRS07",                   0,
	//				         "NSRS11.MonroeWI-M",  "",                                                                   "CRS HARN/WI.MonroeWI-M with datum modified to NSRS11",                   0   },
	//{         "HARN/WI.OcontoWI-F",    "HARN/WI",      0,
	//				         "NSRS07.OcontoWI-F",  "",                                                                   "CRS HARN/WI.OcontoWI-F with datum modified to NSRS07",                   0,
	//				         "NSRS11.OcontoWI-F",  "",                                                                   "CRS HARN/WI.OcontoWI-F with datum modified to NSRS11",                   0   },
	//{        "HARN/WI.OcontoWI-IF",    "HARN/WI",      0,
	//				        "NSRS07.OcontoWI-IF",  "",                                                                   "CRS HARN/WI.OcontoWI-IF with datum modified to NSRS07",                  0,
	//				        "NSRS11.OcontoWI-IF",  "",                                                                   "CRS HARN/WI.OcontoWI-IF with datum modified to NSRS11",                  0   },
	//{         "HARN/WI.OcontoWI-M",    "HARN/WI",      0,
	//				         "NSRS07.OcontoWI-M",  "",                                                                   "CRS HARN/WI.OcontoWI-M with datum modified to NSRS07",                   0,
	//				         "NSRS11.OcontoWI-M",  "",                                                                   "CRS HARN/WI.OcontoWI-M with datum modified to NSRS11",                   0   },
	//{         "HARN/WI.OneidaWI-F",    "HARN/WI",      0,
	//				         "NSRS07.OneidaWI-F",  "",                                                                   "CRS HARN/WI.OneidaWI-F with datum modified to NSRS07",                   0,
	//				         "NSRS11.OneidaWI-F",  "",                                                                   "CRS HARN/WI.OneidaWI-F with datum modified to NSRS11",                   0   },
	//{        "HARN/WI.OneidaWI-IF",    "HARN/WI",      0,
	//				        "NSRS07.OneidaWI-IF",  "",                                                                   "CRS HARN/WI.OneidaWI-IF with datum modified to NSRS07",                  0,
	//				        "NSRS11.OneidaWI-IF",  "",                                                                   "CRS HARN/WI.OneidaWI-IF with datum modified to NSRS11",                  0   },
	//{         "HARN/WI.OneidaWI-M",    "HARN/WI",      0,
	//				         "NSRS07.OneidaWI-M",  "",                                                                   "CRS HARN/WI.OneidaWI-M with datum modified to NSRS07",                   0,
	//				         "NSRS11.OneidaWI-M",  "",                                                                   "CRS HARN/WI.OneidaWI-M with datum modified to NSRS11",                   0   },
	//{      "HARN/WI.OutagamieWI-F",    "HARN/WI",      0,
	//				      "NSRS07.OutagamieWI-F",  "",                                                                   "CRS HARN/WI.OutagamieWI-F with datum modified to NSRS07",                0,
	//				      "NSRS11.OutagamieWI-F",  "",                                                                   "CRS HARN/WI.OutagamieWI-F with datum modified to NSRS11",                0   },
	//{     "HARN/WI.OutagamieWI-IF",    "HARN/WI",      0,
	//				     "NSRS07.OutagamieWI-IF",  "",                                                                   "CRS HARN/WI.OutagamieWI-IF with datum modified to NSRS07",               0,
	//				     "NSRS11.OutagamieWI-IF",  "",                                                                   "CRS HARN/WI.OutagamieWI-IF with datum modified to NSRS11",               0   },
	//{      "HARN/WI.OutagamieWI-M",    "HARN/WI",      0,
	//				      "NSRS07.OutagamieWI-M",  "",                                                                   "CRS HARN/WI.OutagamieWI-M with datum modified to NSRS07",                0,
	//				      "NSRS11.OutagamieWI-M",  "",                                                                   "CRS HARN/WI.OutagamieWI-M with datum modified to NSRS11",                0   },
	//{        "HARN/WI.OzaukeeWI-F",    "HARN/WI",      0,
	//				        "NSRS07.OzaukeeWI-F",  "",                                                                   "CRS HARN/WI.OzaukeeWI-F with datum modified to NSRS07",                  0,
	//				        "NSRS11.OzaukeeWI-F",  "",                                                                   "CRS HARN/WI.OzaukeeWI-F with datum modified to NSRS11",                  0   },
	//{       "HARN/WI.OzaukeeWI-IF",    "HARN/WI",      0,
	//				       "NSRS07.OzaukeeWI-IF",  "",                                                                   "CRS HARN/WI.OzaukeeWI-IF with datum modified to NSRS07",                 0,
	//				       "NSRS11.OzaukeeWI-IF",  "",                                                                   "CRS HARN/WI.OzaukeeWI-IF with datum modified to NSRS11",                 0   },
	//{        "HARN/WI.OzaukeeWI-M",    "HARN/WI",      0,
	//				        "NSRS07.OzaukeeWI-M",  "",                                                                   "CRS HARN/WI.OzaukeeWI-M with datum modified to NSRS07",                  0,
	//				        "NSRS11.OzaukeeWI-M",  "",                                                                   "CRS HARN/WI.OzaukeeWI-M with datum modified to NSRS11",                  0   },
	//{          "HARN/WI.PepinWI-F",    "HARN/WI",      0,
	//				          "NSRS07.PepinWI-F",  "",                                                                   "CRS HARN/WI.PepinWI-F with datum modified to NSRS07",                    0,
	//				          "NSRS11.PepinWI-F",  "",                                                                   "CRS HARN/WI.PepinWI-F with datum modified to NSRS11",                    0   },
	//{         "HARN/WI.PepinWI-IF",    "HARN/WI",      0,
	//				         "NSRS07.PepinWI-IF",  "",                                                                   "CRS HARN/WI.PepinWI-IF with datum modified to NSRS07",                   0,
	//				         "NSRS11.PepinWI-IF",  "",                                                                   "CRS HARN/WI.PepinWI-IF with datum modified to NSRS11",                   0   },
	//{          "HARN/WI.PepinWI-M",    "HARN/WI",      0,
	//				          "NSRS07.PepinWI-M",  "",                                                                   "CRS HARN/WI.PepinWI-M with datum modified to NSRS07",                    0,
	//				          "NSRS11.PepinWI-M",  "",                                                                   "CRS HARN/WI.PepinWI-M with datum modified to NSRS11",                    0   },
	//{         "HARN/WI.PierceWI-F",    "HARN/WI",      0,
	//				         "NSRS07.PierceWI-F",  "",                                                                   "CRS HARN/WI.PierceWI-F with datum modified to NSRS07",                   0,
	//				         "NSRS11.PierceWI-F",  "",                                                                   "CRS HARN/WI.PierceWI-F with datum modified to NSRS11",                   0   },
	//{        "HARN/WI.PierceWI-IF",    "HARN/WI",      0,
	//				        "NSRS07.PierceWI-IF",  "",                                                                   "CRS HARN/WI.PierceWI-IF with datum modified to NSRS07",                  0,
	//				        "NSRS11.PierceWI-IF",  "",                                                                   "CRS HARN/WI.PierceWI-IF with datum modified to NSRS11",                  0   },
	//{         "HARN/WI.PierceWI-M",    "HARN/WI",      0,
	//				         "NSRS07.PierceWI-M",  "",                                                                   "CRS HARN/WI.PierceWI-M with datum modified to NSRS07",                   0,
	//				         "NSRS11.PierceWI-M",  "",                                                                   "CRS HARN/WI.PierceWI-M with datum modified to NSRS11",                   0   },
	//{           "HARN/WI.PolkWI-F",    "HARN/WI",      0,
	//				           "NSRS07.PolkWI-F",  "",                                                                   "CRS HARN/WI.PolkWI-F with datum modified to NSRS07",                     0,
	//				           "NSRS11.PolkWI-F",  "",                                                                   "CRS HARN/WI.PolkWI-F with datum modified to NSRS11",                     0   },
	//{          "HARN/WI.PolkWI-IF",    "HARN/WI",      0,
	//				          "NSRS07.PolkWI-IF",  "",                                                                   "CRS HARN/WI.PolkWI-IF with datum modified to NSRS07",                    0,
	//				          "NSRS11.PolkWI-IF",  "",                                                                   "CRS HARN/WI.PolkWI-IF with datum modified to NSRS11",                    0   },
	//{           "HARN/WI.PolkWI-M",    "HARN/WI",      0,
	//				           "NSRS07.PolkWI-M",  "",                                                                   "CRS HARN/WI.PolkWI-M with datum modified to NSRS07",                     0,
	//				           "NSRS11.PolkWI-M",  "",                                                                   "CRS HARN/WI.PolkWI-M with datum modified to NSRS11",                     0   },
	//{        "HARN/WI.PortageWI-F",    "HARN/WI",      0,
	//				        "NSRS07.PortageWI-F",  "",                                                                   "CRS HARN/WI.PortageWI-F with datum modified to NSRS07",                  0,
	//				        "NSRS11.PortageWI-F",  "",                                                                   "CRS HARN/WI.PortageWI-F with datum modified to NSRS11",                  0   },
	//{       "HARN/WI.PortageWI-IF",    "HARN/WI",      0,
	//				       "NSRS07.PortageWI-IF",  "",                                                                   "CRS HARN/WI.PortageWI-IF with datum modified to NSRS07",                 0,
	//				       "NSRS11.PortageWI-IF",  "",                                                                   "CRS HARN/WI.PortageWI-IF with datum modified to NSRS11",                 0   },
	//{        "HARN/WI.PortageWI-M",    "HARN/WI",      0,
	//				        "NSRS07.PortageWI-M",  "",                                                                   "CRS HARN/WI.PortageWI-M with datum modified to NSRS07",                  0,
	//				        "NSRS11.PortageWI-M",  "",                                                                   "CRS HARN/WI.PortageWI-M with datum modified to NSRS11",                  0   },
	//{          "HARN/WI.PriceWI-F",    "HARN/WI",      0,
	//				          "NSRS07.PriceWI-F",  "",                                                                   "CRS HARN/WI.PriceWI-F with datum modified to NSRS07",                    0,
	//				          "NSRS11.PriceWI-F",  "",                                                                   "CRS HARN/WI.PriceWI-F with datum modified to NSRS11",                    0   },
	//{         "HARN/WI.PriceWI-IF",    "HARN/WI",      0,
	//				         "NSRS07.PriceWI-IF",  "",                                                                   "CRS HARN/WI.PriceWI-IF with datum modified to NSRS07",                   0,
	//				         "NSRS11.PriceWI-IF",  "",                                                                   "CRS HARN/WI.PriceWI-IF with datum modified to NSRS11",                   0   },
	//{          "HARN/WI.PriceWI-M",    "HARN/WI",      0,
	//				          "NSRS07.PriceWI-M",  "",                                                                   "CRS HARN/WI.PriceWI-M with datum modified to NSRS07",                    0,
	//				          "NSRS11.PriceWI-M",  "",                                                                   "CRS HARN/WI.PriceWI-M with datum modified to NSRS11",                    0   },
	//{         "HARN/WI.RacineWI-F",    "HARN/WI",      0,
	//				         "NSRS07.RacineWI-F",  "",                                                                   "CRS HARN/WI.RacineWI-F with datum modified to NSRS07",                   0,
	//				         "NSRS11.RacineWI-F",  "",                                                                   "CRS HARN/WI.RacineWI-F with datum modified to NSRS11",                   0   },
	//{        "HARN/WI.RacineWI-IF",    "HARN/WI",      0,
	//				        "NSRS07.RacineWI-IF",  "",                                                                   "CRS HARN/WI.RacineWI-IF with datum modified to NSRS07",                  0,
	//				        "NSRS11.RacineWI-IF",  "",                                                                   "CRS HARN/WI.RacineWI-IF with datum modified to NSRS11",                  0   },
	//{         "HARN/WI.RacineWI-M",    "HARN/WI",      0,
	//				         "NSRS07.RacineWI-M",  "",                                                                   "CRS HARN/WI.RacineWI-M with datum modified to NSRS07",                   0,
	//				         "NSRS11.RacineWI-M",  "",                                                                   "CRS HARN/WI.RacineWI-M with datum modified to NSRS11",                   0   },
	//{       "HARN/WI.RichlandWI-F",    "HARN/WI",      0,
	//				       "NSRS07.RichlandWI-F",  "",                                                                   "CRS HARN/WI.RichlandWI-F with datum modified to NSRS07",                 0,
	//				       "NSRS11.RichlandWI-F",  "",                                                                   "CRS HARN/WI.RichlandWI-F with datum modified to NSRS11",                 0   },
	//{      "HARN/WI.RichlandWI-IF",    "HARN/WI",      0,
	//				      "NSRS07.RichlandWI-IF",  "",                                                                   "CRS HARN/WI.RichlandWI-IF with datum modified to NSRS07",                0,
	//				      "NSRS11.RichlandWI-IF",  "",                                                                   "CRS HARN/WI.RichlandWI-IF with datum modified to NSRS11",                0   },
	//{       "HARN/WI.RichlandWI-M",    "HARN/WI",      0,
	//				       "NSRS07.RichlandWI-M",  "",                                                                   "CRS HARN/WI.RichlandWI-M with datum modified to NSRS07",                 0,
	//				       "NSRS11.RichlandWI-M",  "",                                                                   "CRS HARN/WI.RichlandWI-M with datum modified to NSRS11",                 0   },
	//{           "HARN/WI.RockWI-F",    "HARN/WI",      0,
	//				           "NSRS07.RockWI-F",  "",                                                                   "CRS HARN/WI.RockWI-F with datum modified to NSRS07",                     0,
	//				           "NSRS11.RockWI-F",  "",                                                                   "CRS HARN/WI.RockWI-F with datum modified to NSRS11",                     0   },
	//{          "HARN/WI.RockWI-IF",    "HARN/WI",      0,
	//				          "NSRS07.RockWI-IF",  "",                                                                   "CRS HARN/WI.RockWI-IF with datum modified to NSRS07",                    0,
	//				          "NSRS11.RockWI-IF",  "",                                                                   "CRS HARN/WI.RockWI-IF with datum modified to NSRS11",                    0   },
	//{           "HARN/WI.RockWI-M",    "HARN/WI",      0,
	//				           "NSRS07.RockWI-M",  "",                                                                   "CRS HARN/WI.RockWI-M with datum modified to NSRS07",                     0,
	//				           "NSRS11.RockWI-M",  "",                                                                   "CRS HARN/WI.RockWI-M with datum modified to NSRS11",                     0   },
	//{           "HARN/WI.RuskWI-F",    "HARN/WI",      0,
	//				           "NSRS07.RuskWI-F",  "",                                                                   "CRS HARN/WI.RuskWI-F with datum modified to NSRS07",                     0,
	//				           "NSRS11.RuskWI-F",  "",                                                                   "CRS HARN/WI.RuskWI-F with datum modified to NSRS11",                     0   },
	//{          "HARN/WI.RuskWI-IF",    "HARN/WI",      0,
	//				          "NSRS07.RuskWI-IF",  "",                                                                   "CRS HARN/WI.RuskWI-IF with datum modified to NSRS07",                    0,
	//				          "NSRS11.RuskWI-IF",  "",                                                                   "CRS HARN/WI.RuskWI-IF with datum modified to NSRS11",                    0   },
	//{           "HARN/WI.RuskWI-M",    "HARN/WI",      0,
	//				           "NSRS07.RuskWI-M",  "",                                                                   "CRS HARN/WI.RuskWI-M with datum modified to NSRS07",                     0,
	//				           "NSRS11.RuskWI-M",  "",                                                                   "CRS HARN/WI.RuskWI-M with datum modified to NSRS11",                     0   },
	//{           "HARN/WI.SaukWI-F",    "HARN/WI",      0,
	//				           "NSRS07.SaukWI-F",  "",                                                                   "CRS HARN/WI.SaukWI-F with datum modified to NSRS07",                     0,
	//				           "NSRS11.SaukWI-F",  "",                                                                   "CRS HARN/WI.SaukWI-F with datum modified to NSRS11",                     0   },
	//{          "HARN/WI.SaukWI-IF",    "HARN/WI",      0,
	//				          "NSRS07.SaukWI-IF",  "",                                                                   "CRS HARN/WI.SaukWI-IF with datum modified to NSRS07",                    0,
	//				          "NSRS11.SaukWI-IF",  "",                                                                   "CRS HARN/WI.SaukWI-IF with datum modified to NSRS11",                    0   },
	//{           "HARN/WI.SaukWI-M",    "HARN/WI",      0,
	//				           "NSRS07.SaukWI-M",  "",                                                                   "CRS HARN/WI.SaukWI-M with datum modified to NSRS07",                     0,
	//				           "NSRS11.SaukWI-M",  "",                                                                   "CRS HARN/WI.SaukWI-M with datum modified to NSRS11",                     0   },
	//{         "HARN/WI.SawyerWI-F",    "HARN/WI",      0,
	//				         "NSRS07.SawyerWI-F",  "",                                                                   "CRS HARN/WI.SawyerWI-F with datum modified to NSRS07",                   0,
	//				         "NSRS11.SawyerWI-F",  "",                                                                   "CRS HARN/WI.SawyerWI-F with datum modified to NSRS11",                   0   },
	//{        "HARN/WI.SawyerWI-IF",    "HARN/WI",      0,
	//				        "NSRS07.SawyerWI-IF",  "",                                                                   "CRS HARN/WI.SawyerWI-IF with datum modified to NSRS07",                  0,
	//				        "NSRS11.SawyerWI-IF",  "",                                                                   "CRS HARN/WI.SawyerWI-IF with datum modified to NSRS11",                  0   },
	//{         "HARN/WI.SawyerWI-M",    "HARN/WI",      0,
	//				         "NSRS07.SawyerWI-M",  "",                                                                   "CRS HARN/WI.SawyerWI-M with datum modified to NSRS07",                   0,
	//				         "NSRS11.SawyerWI-M",  "",                                                                   "CRS HARN/WI.SawyerWI-M with datum modified to NSRS11",                   0   },
	//{        "HARN/WI.ShawanoWI-F",    "HARN/WI",      0,
	//				        "NSRS07.ShawanoWI-F",  "",                                                                   "CRS HARN/WI.ShawanoWI-F with datum modified to NSRS07",                  0,
	//				        "NSRS11.ShawanoWI-F",  "",                                                                   "CRS HARN/WI.ShawanoWI-F with datum modified to NSRS11",                  0   },
	//{       "HARN/WI.ShawanoWI-IF",    "HARN/WI",      0,
	//				       "NSRS07.ShawanoWI-IF",  "",                                                                   "CRS HARN/WI.ShawanoWI-IF with datum modified to NSRS07",                 0,
	//				       "NSRS11.ShawanoWI-IF",  "",                                                                   "CRS HARN/WI.ShawanoWI-IF with datum modified to NSRS11",                 0   },
	//{        "HARN/WI.ShawanoWI-M",    "HARN/WI",      0,
	//				        "NSRS07.ShawanoWI-M",  "",                                                                   "CRS HARN/WI.ShawanoWI-M with datum modified to NSRS07",                  0,
	//				        "NSRS11.ShawanoWI-M",  "",                                                                   "CRS HARN/WI.ShawanoWI-M with datum modified to NSRS11",                  0   },
	//{      "HARN/WI.SheboyganWI-F",    "HARN/WI",      0,
	//				      "NSRS07.SheboyganWI-F",  "",                                                                   "CRS HARN/WI.SheboyganWI-F with datum modified to NSRS07",                0,
	//				      "NSRS11.SheboyganWI-F",  "",                                                                   "CRS HARN/WI.SheboyganWI-F with datum modified to NSRS11",                0   },
	//{     "HARN/WI.SheboyganWI-IF",    "HARN/WI",      0,
	//				     "NSRS07.SheboyganWI-IF",  "",                                                                   "CRS HARN/WI.SheboyganWI-IF with datum modified to NSRS07",               0,
	//				     "NSRS11.SheboyganWI-IF",  "",                                                                   "CRS HARN/WI.SheboyganWI-IF with datum modified to NSRS11",               0   },
	//{      "HARN/WI.SheboyganWI-M",    "HARN/WI",      0,
	//				      "NSRS07.SheboyganWI-M",  "",                                                                   "CRS HARN/WI.SheboyganWI-M with datum modified to NSRS07",                0,
	//				      "NSRS11.SheboyganWI-M",  "",                                                                   "CRS HARN/WI.SheboyganWI-M with datum modified to NSRS11",                0   },
	//{        "HARN/WI.StCroixWI-F",    "HARN/WI",      0,
	//				        "NSRS07.StCroixWI-F",  "",                                                                   "CRS HARN/WI.StCroixWI-F with datum modified to NSRS07",                  0,
	//				        "NSRS11.StCroixWI-F",  "",                                                                   "CRS HARN/WI.StCroixWI-F with datum modified to NSRS11",                  0   },
	//{       "HARN/WI.StCroixWI-IF",    "HARN/WI",      0,
	//				       "NSRS07.StCroixWI-IF",  "",                                                                   "CRS HARN/WI.StCroixWI-IF with datum modified to NSRS07",                 0,
	//				       "NSRS11.StCroixWI-IF",  "",                                                                   "CRS HARN/WI.StCroixWI-IF with datum modified to NSRS11",                 0   },
	//{        "HARN/WI.StCroixWI-M",    "HARN/WI",      0,
	//				        "NSRS07.StCroixWI-M",  "",                                                                   "CRS HARN/WI.StCroixWI-M with datum modified to NSRS07",                  0,
	//				        "NSRS11.StCroixWI-M",  "",                                                                   "CRS HARN/WI.StCroixWI-M with datum modified to NSRS11",                  0   },
	//{         "HARN/WI.TaylorWI-F",    "HARN/WI",      0,
	//				         "NSRS07.TaylorWI-F",  "",                                                                   "CRS HARN/WI.TaylorWI-F with datum modified to NSRS07",                   0,
	//				         "NSRS11.TaylorWI-F",  "",                                                                   "CRS HARN/WI.TaylorWI-F with datum modified to NSRS11",                   0   },
	//{        "HARN/WI.TaylorWI-IF",    "HARN/WI",      0,
	//				        "NSRS07.TaylorWI-IF",  "",                                                                   "CRS HARN/WI.TaylorWI-IF with datum modified to NSRS07",                  0,
	//				        "NSRS11.TaylorWI-IF",  "",                                                                   "CRS HARN/WI.TaylorWI-IF with datum modified to NSRS11",                  0   },
	//{         "HARN/WI.TaylorWI-M",    "HARN/WI",      0,
	//				         "NSRS07.TaylorWI-M",  "",                                                                   "CRS HARN/WI.TaylorWI-M with datum modified to NSRS07",                   0,
	//				         "NSRS11.TaylorWI-M",  "",                                                                   "CRS HARN/WI.TaylorWI-M with datum modified to NSRS11",                   0   },
	//{    "HARN/WI.TrempealeauWI-F",    "HARN/WI",      0,
	//				    "NSRS07.TrempealeauWI-F",  "",                                                                   "CRS HARN/WI.TrempealeauWI-F with datum modified to NSRS07",              0,
	//				    "NSRS11.TrempealeauWI-F",  "",                                                                   "CRS HARN/WI.TrempealeauWI-F with datum modified to NSRS11",              0   },
	//{    "HARN/WI.TrempealeaWI-IF",    "HARN/WI",      0,
	//				    "NSRS07.TrempealeaWI-IF",  "",                                                                   "CRS HARN/WI.TrempealeaWI-IF with datum modified to NSRS07",              0,
	//				    "NSRS11.TrempealeaWI-IF",  "",                                                                   "CRS HARN/WI.TrempealeaWI-IF with datum modified to NSRS11",              0   },
	//{    "HARN/WI.TrempealeauWI-M",    "HARN/WI",      0,
	//				    "NSRS07.TrempealeauWI-M",  "",                                                                   "CRS HARN/WI.TrempealeauWI-M with datum modified to NSRS07",              0,
	//				    "NSRS11.TrempealeauWI-M",  "",                                                                   "CRS HARN/WI.TrempealeauWI-M with datum modified to NSRS11",              0   },
	//{         "HARN/WI.VernonWI-F",    "HARN/WI",      0,
	//				         "NSRS07.VernonWI-F",  "",                                                                   "CRS HARN/WI.VernonWI-F with datum modified to NSRS07",                   0,
	//				         "NSRS11.VernonWI-F",  "",                                                                   "CRS HARN/WI.VernonWI-F with datum modified to NSRS11",                   0   },
	//{        "HARN/WI.VernonWI-IF",    "HARN/WI",      0,
	//				        "NSRS07.VernonWI-IF",  "",                                                                   "CRS HARN/WI.VernonWI-IF with datum modified to NSRS07",                  0,
	//				        "NSRS11.VernonWI-IF",  "",                                                                   "CRS HARN/WI.VernonWI-IF with datum modified to NSRS11",                  0   },
	//{         "HARN/WI.VernonWI-M",    "HARN/WI",      0,
	//				         "NSRS07.VernonWI-M",  "",                                                                   "CRS HARN/WI.VernonWI-M with datum modified to NSRS07",                   0,
	//				         "NSRS11.VernonWI-M",  "",                                                                   "CRS HARN/WI.VernonWI-M with datum modified to NSRS11",                   0   },
	//{          "HARN/WI.VilasWI-F",    "HARN/WI",      0,
	//				          "NSRS07.VilasWI-F",  "",                                                                   "CRS HARN/WI.VilasWI-F with datum modified to NSRS07",                    0,
	//				          "NSRS11.VilasWI-F",  "",                                                                   "CRS HARN/WI.VilasWI-F with datum modified to NSRS11",                    0   },
	//{         "HARN/WI.VilasWI-IF",    "HARN/WI",      0,
	//				         "NSRS07.VilasWI-IF",  "",                                                                   "CRS HARN/WI.VilasWI-IF with datum modified to NSRS07",                   0,
	//				         "NSRS11.VilasWI-IF",  "",                                                                   "CRS HARN/WI.VilasWI-IF with datum modified to NSRS11",                   0   },
	//{          "HARN/WI.VilasWI-M",    "HARN/WI",      0,
	//				          "NSRS07.VilasWI-M",  "",                                                                   "CRS HARN/WI.VilasWI-M with datum modified to NSRS07",                    0,
	//				          "NSRS11.VilasWI-M",  "",                                                                   "CRS HARN/WI.VilasWI-M with datum modified to NSRS11",                    0   },
	//{       "HARN/WI.WalworthWI-F",    "HARN/WI",      0,
	//				       "NSRS07.WalworthWI-F",  "",                                                                   "CRS HARN/WI.WalworthWI-F with datum modified to NSRS07",                 0,
	//				       "NSRS11.WalworthWI-F",  "",                                                                   "CRS HARN/WI.WalworthWI-F with datum modified to NSRS11",                 0   },
	//{      "HARN/WI.WalworthWI-IF",    "HARN/WI",      0,
	//				      "NSRS07.WalworthWI-IF",  "",                                                                   "CRS HARN/WI.WalworthWI-IF with datum modified to NSRS07",                0,
	//				      "NSRS11.WalworthWI-IF",  "",                                                                   "CRS HARN/WI.WalworthWI-IF with datum modified to NSRS11",                0   },
	//{       "HARN/WI.WalworthWI-M",    "HARN/WI",      0,
	//				       "NSRS07.WalworthWI-M",  "",                                                                   "CRS HARN/WI.WalworthWI-M with datum modified to NSRS07",                 0,
	//				       "NSRS11.WalworthWI-M",  "",                                                                   "CRS HARN/WI.WalworthWI-M with datum modified to NSRS11",                 0   },
	//{       "HARN/WI.WashburnWI-F",    "HARN/WI",      0,
	//				       "NSRS07.WashburnWI-F",  "",                                                                   "CRS HARN/WI.WashburnWI-F with datum modified to NSRS07",                 0,
	//				       "NSRS11.WashburnWI-F",  "",                                                                   "CRS HARN/WI.WashburnWI-F with datum modified to NSRS11",                 0   },
	//{      "HARN/WI.WashburnWI-IF",    "HARN/WI",      0,
	//				      "NSRS07.WashburnWI-IF",  "",                                                                   "CRS HARN/WI.WashburnWI-IF with datum modified to NSRS07",                0,
	//				      "NSRS11.WashburnWI-IF",  "",                                                                   "CRS HARN/WI.WashburnWI-IF with datum modified to NSRS11",                0   },
	//{       "HARN/WI.WashburnWI-M",    "HARN/WI",      0,
	//				       "NSRS07.WashburnWI-M",  "",                                                                   "CRS HARN/WI.WashburnWI-M with datum modified to NSRS07",                 0,
	//				       "NSRS11.WashburnWI-M",  "",                                                                   "CRS HARN/WI.WashburnWI-M with datum modified to NSRS11",                 0   },
	//{     "HARN/WI.WashingtonWI-F",    "HARN/WI",      0,
	//				     "NSRS07.WashingtonWI-F",  "",                                                                   "CRS HARN/WI.WashingtonWI-F with datum modified to NSRS07",               0,
	//				     "NSRS11.WashingtonWI-F",  "",                                                                   "CRS HARN/WI.WashingtonWI-F with datum modified to NSRS11",               0   },
	//{    "HARN/WI.WashingtonWI-IF",    "HARN/WI",      0,
	//				    "NSRS07.WashingtonWI-IF",  "",                                                                   "CRS HARN/WI.WashingtonWI-IF with datum modified to NSRS07",              0,
	//				    "NSRS11.WashingtonWI-IF",  "",                                                                   "CRS HARN/WI.WashingtonWI-IF with datum modified to NSRS11",              0   },
	//{     "HARN/WI.WashingtonWI-M",    "HARN/WI",      0,
	//				     "NSRS07.WashingtonWI-M",  "",                                                                   "CRS HARN/WI.WashingtonWI-M with datum modified to NSRS07",               0,
	//				     "NSRS11.WashingtonWI-M",  "",                                                                   "CRS HARN/WI.WashingtonWI-M with datum modified to NSRS11",               0   },
	//{       "HARN/WI.WaukeshaWI-F",    "HARN/WI",      0,
	//				       "NSRS07.WaukeshaWI-F",  "",                                                                   "CRS HARN/WI.WaukeshaWI-F with datum modified to NSRS07",                 0,
	//				       "NSRS11.WaukeshaWI-F",  "",                                                                   "CRS HARN/WI.WaukeshaWI-F with datum modified to NSRS11",                 0   },
	//{      "HARN/WI.WaukeshaWI-IF",    "HARN/WI",      0,
	//				      "NSRS07.WaukeshaWI-IF",  "",                                                                   "CRS HARN/WI.WaukeshaWI-IF with datum modified to NSRS07",                0,
	//				      "NSRS11.WaukeshaWI-IF",  "",                                                                   "CRS HARN/WI.WaukeshaWI-IF with datum modified to NSRS11",                0   },
	//{       "HARN/WI.WaukeshaWI-M",    "HARN/WI",      0,
	//				       "NSRS07.WaukeshaWI-M",  "",                                                                   "CRS HARN/WI.WaukeshaWI-M with datum modified to NSRS07",                 0,
	//				       "NSRS11.WaukeshaWI-M",  "",                                                                   "CRS HARN/WI.WaukeshaWI-M with datum modified to NSRS11",                 0   },
	//{        "HARN/WI.WaupacaWI-F",    "HARN/WI",      0,
	//				        "NSRS07.WaupacaWI-F",  "",                                                                   "CRS HARN/WI.WaupacaWI-F with datum modified to NSRS07",                  0,
	//				        "NSRS11.WaupacaWI-F",  "",                                                                   "CRS HARN/WI.WaupacaWI-F with datum modified to NSRS11",                  0   },
	//{       "HARN/WI.WaupacaWI-IF",    "HARN/WI",      0,
	//				       "NSRS07.WaupacaWI-IF",  "",                                                                   "CRS HARN/WI.WaupacaWI-IF with datum modified to NSRS07",                 0,
	//				       "NSRS11.WaupacaWI-IF",  "",                                                                   "CRS HARN/WI.WaupacaWI-IF with datum modified to NSRS11",                 0   },
	//{        "HARN/WI.WaupacaWI-M",    "HARN/WI",      0,
	//				        "NSRS07.WaupacaWI-M",  "",                                                                   "CRS HARN/WI.WaupacaWI-M with datum modified to NSRS07",                  0,
	//				        "NSRS11.WaupacaWI-M",  "",                                                                   "CRS HARN/WI.WaupacaWI-M with datum modified to NSRS11",                  0   },
	//{       "HARN/WI.WausharaWI-F",    "HARN/WI",      0,
	//				       "NSRS07.WausharaWI-F",  "",                                                                   "CRS HARN/WI.WausharaWI-F with datum modified to NSRS07",                 0,
	//				       "NSRS11.WausharaWI-F",  "",                                                                   "CRS HARN/WI.WausharaWI-F with datum modified to NSRS11",                 0   },
	//{      "HARN/WI.WausharaWI-IF",    "HARN/WI",      0,
	//				      "NSRS07.WausharaWI-IF",  "",                                                                   "CRS HARN/WI.WausharaWI-IF with datum modified to NSRS07",                0,
	//				      "NSRS11.WausharaWI-IF",  "",                                                                   "CRS HARN/WI.WausharaWI-IF with datum modified to NSRS11",                0   },
	//{       "HARN/WI.WausharaWI-M",    "HARN/WI",      0,
	//				       "NSRS07.WausharaWI-M",  "",                                                                   "CRS HARN/WI.WausharaWI-M with datum modified to NSRS07",                 0,
	//				       "NSRS11.WausharaWI-M",  "",                                                                   "CRS HARN/WI.WausharaWI-M with datum modified to NSRS11",                 0   },
	//{      "HARN/WI.WinnebagoWI-F",    "HARN/WI",      0,
	//				      "NSRS07.WinnebagoWI-F",  "",                                                                   "CRS HARN/WI.WinnebagoWI-F with datum modified to NSRS07",                0,
	//				      "NSRS11.WinnebagoWI-F",  "",                                                                   "CRS HARN/WI.WinnebagoWI-F with datum modified to NSRS11",                0   },
	//{     "HARN/WI.WinnebagoWI-IF",    "HARN/WI",      0,
	//				     "NSRS07.WinnebagoWI-IF",  "",                                                                   "CRS HARN/WI.WinnebagoWI-IF with datum modified to NSRS07",               0,
	//				     "NSRS11.WinnebagoWI-IF",  "",                                                                   "CRS HARN/WI.WinnebagoWI-IF with datum modified to NSRS11",               0   },
	//{      "HARN/WI.WinnebagoWI-M",    "HARN/WI",      0,
	//				      "NSRS07.WinnebagoWI-M",  "",                                                                   "CRS HARN/WI.WinnebagoWI-M with datum modified to NSRS07",                0,
	//				      "NSRS11.WinnebagoWI-M",  "",                                                                   "CRS HARN/WI.WinnebagoWI-M with datum modified to NSRS11",                0   },
	//{           "HARN/WI.WoodWI-F",    "HARN/WI",      0,
	//				           "NSRS07.WoodWI-F",  "",                                                                   "CRS HARN/WI.WoodWI-F with datum modified to NSRS07",                     0,
	//				           "NSRS11.WoodWI-F",  "",                                                                   "CRS HARN/WI.WoodWI-F with datum modified to NSRS11",                     0   },
	//{          "HARN/WI.WoodWI-IF",    "HARN/WI",      0,
	//				          "NSRS07.WoodWI-IF",  "",                                                                   "CRS HARN/WI.WoodWI-IF with datum modified to NSRS07",                    0,
	//				          "NSRS11.WoodWI-IF",  "",                                                                   "CRS HARN/WI.WoodWI-IF with datum modified to NSRS11",                    0   },
	//{           "HARN/WI.WoodWI-M",    "HARN/WI",      0,
	//				           "NSRS07.WoodWI-M",  "",                                                                   "CRS HARN/WI.WoodWI-M with datum modified to NSRS07",                     0,
	//				           "NSRS11.WoodWI-M",  "",                                                                   "CRS HARN/WI.WoodWI-M with datum modified to NSRS11",                     0   },
	{               "HARN/AR.AR-N",    "HARN/AR",   2764,
					               "NSRS07.AR-N",  "NSRS 2007 Arkansas State Planes, North Zone, Meter",                 "CRS HARN/AR.AR-N with datum modified to NSRS07",                      3484,
					               "NSRS11.AR-N",  "NSRS 2011 Arkansas State Planes, North Zone, Meter",                 "CRS HARN/AR.AR-N with datum modified to NSRS11",                         0   },
	{              "HARN/AR.AR-NF",    "HARN/AR",   3441,
					              "NSRS07.AR-NF",  "NSRS 2007 Arkansas State Planes, North Zone, US Foot",               "CRS HARN/AR.AR-NF with datum modified to NSRS07",                     3485,
					              "NSRS11.AR-NF",  "NSRS 2011 Arkansas State Planes, North Zone, US Foot",               "CRS HARN/AR.AR-NF with datum modified to NSRS11",                        0   },
	{               "HARN/AR.AR-S",    "HARN/AR",   2765,
					               "NSRS07.AR-S",  "NSRS 2007 Arkansas State Planes, South Zone, Meter",                 "CRS HARN/AR.AR-S with datum modified to NSRS07",                      3486,
					               "NSRS11.AR-S",  "NSRS 2011 Arkansas State Planes, South Zone, Meter",                 "CRS HARN/AR.AR-S with datum modified to NSRS11",                         0   },
	{              "HARN/AR.AR-SF",    "HARN/AR",   3442,
					              "NSRS07.AR-SF",  "NSRS 2007 Arkansas State Planes, South Zone, US Foot",               "CRS HARN/AR.AR-SF with datum modified to NSRS07",                     3487,
					              "NSRS11.AR-SF",  "NSRS 2011 Arkansas State Planes, South Zone, US Foot",               "CRS HARN/AR.AR-SF with datum modified to NSRS11",                        0   },
	{               "HARN/CA.CAVI",    "HARN/CA",   2771,
					               "NSRS07.CAVI",  "NSRS 2007 California State Planes, Zone VI, Meter",                  "CRS HARN/CA.CAVI with datum modified to NSRS07",                      3499,
					               "NSRS11.CAVI",  "NSRS 2011 California State Planes, Zone VI, Meter",                  "CRS HARN/CA.CAVI with datum modified to NSRS11",                         0   },
	{              "HARN/CA.CAVIF",    "HARN/CA",   2875,
					              "NSRS07.CAVIF",  "NSRS 2007 California State Planes, Zone VI, US Foot",                "CRS HARN/CA.CAVIF with datum modified to NSRS07",                     3500,
					              "NSRS11.CAVIF",  "NSRS 2011 California State Planes, Zone VI, US Foot",                "CRS HARN/CA.CAVIF with datum modified to NSRS11",                        0   },
	{               "HARN/CO.COCF",    "HARN/CO",   2877,
					               "NSRS07.COCF",  "NSRS 2007 Colorado State Planes, Central Zone, US Foot",             "CRS HARN/CO.COCF with datum modified to NSRS07",                      3502,
					               "NSRS11.COCF",  "NSRS 2011 Colorado State Planes, Central Zone, US Foot",             "CRS HARN/CO.COCF with datum modified to NSRS11",                         0   },
	{               "HARN/GA.GA-E",    "HARN/GA",   2780,
					               "NSRS07.GA-E",  "NSRS 2007 Georgia State Planes, East Zone, Meter",                   "CRS HARN/GA.GA-E with datum modified to NSRS07",                      3518,
					               "NSRS11.GA-E",  "NSRS 2011 Georgia State Planes, East Zone, Meter",                   "CRS HARN/GA.GA-E with datum modified to NSRS11",                         0   },
	{              "HARN/GA.GA-EF",    "HARN/GA",   2884,
					              "NSRS07.GA-EF",  "NSRS 2007 Georgia State Planes, East Zone, US Foot",                 "CRS HARN/GA.GA-EF with datum modified to NSRS07",                     3519,
					              "NSRS11.GA-EF",  "NSRS 2011 Georgia State Planes, East Zone, US Foot",                 "CRS HARN/GA.GA-EF with datum modified to NSRS11",                        0   },
	{               "HARN/GA.GA-W",    "HARN/GA",   2781,
					               "NSRS07.GA-W",  "NSRS 2007 Georgia State Planes, West Zone, Meter",                   "CRS HARN/GA.GA-W with datum modified to NSRS07",                      3520,
					               "NSRS11.GA-W",  "NSRS 2011 Georgia State Planes, West Zone, Meter",                   "CRS HARN/GA.GA-W with datum modified to NSRS11",                         0   },
	{              "HARN/GA.GA-WF",    "HARN/GA",   2885,
					              "NSRS07.GA-WF",  "NSRS 2007 Georgia State Planes, West Zone, US Foot",                 "CRS HARN/GA.GA-WF with datum modified to NSRS07",                     3521,
					              "NSRS11.GA-WF",  "NSRS 2011 Georgia State Planes, West Zone, US Foot",                 "CRS HARN/GA.GA-WF with datum modified to NSRS11",                        0   },
	{               "HARN/MT.ID-C",    "HARN/MT",   2788,
					               "NSRS07.ID-C",  "NSRS 2007 Idaho State Planes, Central Zone, Meter",                  "CRS HARN/MT.ID-C with datum modified to NSRS07",                      3522,
					               "NSRS11.ID-C",  "NSRS 2011 Idaho State Planes, Central Zone, Meter",                  "CRS HARN/MT.ID-C with datum modified to NSRS11",                         0   },
	{              "HARN/MT.ID-CF",    "HARN/MT",   2887,
					              "NSRS07.ID-CF",  "NSRS 2007 Idaho State Planes, Central Zone, US Foot",                "CRS HARN/MT.ID-CF with datum modified to NSRS07",                     3523,
					              "NSRS11.ID-CF",  "NSRS 2011 Idaho State Planes, Central Zone, US Foot",                "CRS HARN/MT.ID-CF with datum modified to NSRS11",                        0   },
	{               "HARN/MT.ID-E",    "HARN/MT",   2787,
					               "NSRS07.ID-E",  "NSRS 2007 Idaho State Planes, East Zone, Meter",                     "CRS HARN/MT.ID-E with datum modified to NSRS07",                      3524,
					               "NSRS11.ID-E",  "NSRS 2011 Idaho State Planes, East Zone, Meter",                     "CRS HARN/MT.ID-E with datum modified to NSRS11",                         0   },
	{              "HARN/MT.ID-EF",    "HARN/MT",   2886,
					              "NSRS07.ID-EF",  "NSRS 2007 Idaho State Planes, East Zone, US Foot",                   "CRS HARN/MT.ID-EF with datum modified to NSRS07",                     3525,
					              "NSRS11.ID-EF",  "NSRS 2011 Idaho State Planes, East Zone, US Foot",                   "CRS HARN/MT.ID-EF with datum modified to NSRS11",                        0   },
	{               "HARN/MT.ID-W",    "HARN/MT",   2789,
					               "NSRS07.ID-W",  "NSRS 2007 Idaho State Planes, West Zone, Meter",                     "CRS HARN/MT.ID-W with datum modified to NSRS07",                      3526,
					               "NSRS11.ID-W",  "NSRS 2011 Idaho State Planes, West Zone, Meter",                     "CRS HARN/MT.ID-W with datum modified to NSRS11",                         0   },
	{              "HARN/MT.ID-WF",    "HARN/MT",   2888,
					              "NSRS07.ID-WF",  "NSRS 2007 Idaho State Planes, West Zone, US Foot",                   "CRS HARN/MT.ID-WF with datum modified to NSRS07",                     3527,
					              "NSRS11.ID-WF",  "NSRS 2011 Idaho State Planes, West Zone, US Foot",                   "CRS HARN/MT.ID-WF with datum modified to NSRS11",                        0   },
	{               "HARN/IA.IA-N",    "HARN/IA",   2794,
					               "NSRS07.IA-N",  "NSRS 2007 Iowa State Planes, North Zone, Meter",                     "CRS HARN/IA.IA-N with datum modified to NSRS07",                      3536,
					               "NSRS11.IA-N",  "NSRS 2011 Iowa State Planes, North Zone, Meter",                     "CRS HARN/IA.IA-N with datum modified to NSRS11",                         0   },
	{              "HARN/IA.IA-NF",    "HARN/IA",   3425,
					              "NSRS07.IA-NF",  "NSRS 2007 Iowa State Planes, North Zone, US Foot",                   "CRS HARN/IA.IA-NF with datum modified to NSRS07",                     3537,
					              "NSRS11.IA-NF",  "NSRS 2011 Iowa State Planes, North Zone, US Foot",                   "CRS HARN/IA.IA-NF with datum modified to NSRS11",                        0   },
	{               "HARN/IA.IA-S",    "HARN/IA",   2795,
					               "NSRS07.IA-S",  "NSRS 2007 Iowa State Planes, South Zone, Meter",                     "CRS HARN/IA.IA-S with datum modified to NSRS07",                      3538,
					               "NSRS11.IA-S",  "NSRS 2011 Iowa State Planes, South Zone, Meter",                     "CRS HARN/IA.IA-S with datum modified to NSRS11",                         0   },
	{              "HARN/IA.IA-SF",    "HARN/IA",   3426,
					              "NSRS07.IA-SF",  "NSRS 2007 Iowa State Planes, South Zone, US Foot",                   "CRS HARN/IA.IA-SF with datum modified to NSRS07",                     3539,
					              "NSRS11.IA-SF",  "NSRS 2011 Iowa State Planes, South Zone, US Foot",                   "CRS HARN/IA.IA-SF with datum modified to NSRS11",                        0   },
	{               "HARN/IL.IL-E",    "HARN/IL",   2790,
					               "NSRS07.IL-E",  "NSRS 2007 Illinois State Planes, East Zone, Meter",                  "CRS HARN/IL.IL-E with datum modified to NSRS07",                      3528,
					               "NSRS11.IL-E",  "NSRS 2011 Illinois State Planes, East Zone, Meter",                  "CRS HARN/IL.IL-E with datum modified to NSRS11",                         0   },
	{              "HARN/IL.IL-EF",    "HARN/IL",   3443,
					              "NSRS07.IL-EF",  "NSRS 2007 Illinois State Planes, East Zone, US Foot",                "CRS HARN/IL.IL-EF with datum modified to NSRS07",                     3529,
					              "NSRS11.IL-EF",  "NSRS 2011 Illinois State Planes, East Zone, US Foot",                "CRS HARN/IL.IL-EF with datum modified to NSRS11",                        0   },
	{               "HARN/IL.IL-W",    "HARN/IL",   2791,
					               "NSRS07.IL-W",  "NSRS 2007 Illinois State Planes, West Zone, Meter",                  "CRS HARN/IL.IL-W with datum modified to NSRS07",                      3530,
					               "NSRS11.IL-W",  "NSRS 2011 Illinois State Planes, West Zone, Meter",                  "CRS HARN/IL.IL-W with datum modified to NSRS11",                         0   },
	{              "HARN/IL.IL-WF",    "HARN/IL",   3444,
					              "NSRS07.IL-WF",  "NSRS 2007 Illinois State Planes, West Zone, US Foot",                "CRS HARN/IL.IL-WF with datum modified to NSRS07",                     3531,
					              "NSRS11.IL-WF",  "NSRS 2011 Illinois State Planes, West Zone, US Foot",                "CRS HARN/IL.IL-WF with datum modified to NSRS11",                        0   },
	{               "HARN/IN.IN-E",    "HARN/IN",   2792,
					               "NSRS07.IN-E",  "NSRS 2007 Indiana State Planes, East Zone, Meter",                   "CRS HARN/IN.IN-E with datum modified to NSRS07",                      3532,
					               "NSRS11.IN-E",  "NSRS 2011 Indiana State Planes, East Zone, Meter",                   "CRS HARN/IN.IN-E with datum modified to NSRS11",                         0   },
	{              "HARN/IN.IN-EF",    "HARN/IN",   2967,
					              "NSRS07.IN-EF",  "NSRS 2007 Indiana State Planes, East Zone, US Foot",                 "CRS HARN/IN.IN-EF with datum modified to NSRS07",                     3533,
					              "NSRS11.IN-EF",  "NSRS 2011 Indiana State Planes, East Zone, US Foot",                 "CRS HARN/IN.IN-EF with datum modified to NSRS11",                        0   },
	{               "HARN/IN.IN-W",    "HARN/IN",   2793,
					               "NSRS07.IN-W",  "NSRS 2007 Indiana State Planes, West Zone, Meter",                   "CRS HARN/IN.IN-W with datum modified to NSRS07",                      3534,
					               "NSRS11.IN-W",  "NSRS 2011 Indiana State Planes, West Zone, Meter",                   "CRS HARN/IN.IN-W with datum modified to NSRS11",                         0   },
	{              "HARN/IN.IN-WF",    "HARN/IN",   2968,
					              "NSRS07.IN-WF",  "NSRS 2007 Indiana State Planes, West Zone, US Foot",                 "CRS HARN/IN.IN-WF with datum modified to NSRS07",                     3535,
					              "NSRS11.IN-WF",  "NSRS 2011 Indiana State Planes, West Zone, US Foot",                 "CRS HARN/IN.IN-WF with datum modified to NSRS11",                        0   },
	{               "HARN/KS.KS-N",    "HARN/KS",   2796,
					               "NSRS07.KS-N",  "NSRS 2007 Kansas State Planes, North Zone, Meter",                   "CRS HARN/KS.KS-N with datum modified to NSRS07",                      3540,
					               "NSRS11.KS-N",  "NSRS 2011 Kansas State Planes, North Zone, Meter",                   "CRS HARN/KS.KS-N with datum modified to NSRS11",                         0   },
	{              "HARN/KS.KS-NF",    "HARN/KS",   3427,
					              "NSRS07.KS-NF",  "NSRS 2007 Kansas State Planes, North Zone, US Foot",                 "CRS HARN/KS.KS-NF with datum modified to NSRS07",                     3541,
					              "NSRS11.KS-NF",  "NSRS 2011 Kansas State Planes, North Zone, US Foot",                 "CRS HARN/KS.KS-NF with datum modified to NSRS11",                        0   },
	{               "HARN/KS.KS-S",    "HARN/KS",   2797,
					               "NSRS07.KS-S",  "NSRS 2007 Kansas State Planes, South Zone, Meter",                   "CRS HARN/KS.KS-S with datum modified to NSRS07",                      3542,
					               "NSRS11.KS-S",  "NSRS 2011 Kansas State Planes, South Zone, Meter",                   "CRS HARN/KS.KS-S with datum modified to NSRS11",                         0   },
	{              "HARN/KS.KS-SF",    "HARN/KS",   3428,
					              "NSRS07.KS-SF",  "NSRS 2007 Kansas State Planes, South Zone, US Foot",                 "CRS HARN/KS.KS-SF with datum modified to NSRS07",                     3543,
					              "NSRS11.KS-SF",  "NSRS 2011 Kansas State Planes, South Zone, US Foot",                 "CRS HARN/KS.KS-SF with datum modified to NSRS11",                        0   },
	{               "HARN/MI.MI-C",    "HARN/MI",   2808,
					               "NSRS07.MI-C",  "NSRS 2007 Michigan State Planes, Central Zone, Meter",               "CRS HARN/MI.MI-C with datum modified to NSRS07",                      3587,
					               "NSRS11.MI-C",  "NSRS 2011 Michigan State Planes, Central Zone, Meter",               "CRS HARN/MI.MI-C with datum modified to NSRS11",                         0   },
	{              "HARN/MI.MI-CF",    "HARN/MI",      0,
					              "NSRS07.MI-CF",  "NSRS 2007 Michigan State Planes, Central Zone, US Foot",             "CRS HARN/MI.MI-CF with datum modified to NSRS07",                        0,
					              "NSRS11.MI-CF",  "NSRS 2011 Michigan State Planes, Central Zone, US Foot",             "CRS HARN/MI.MI-CF with datum modified to NSRS11",                        0   },
	{             "HARN/MI.MI-CIF",    "HARN/MI",   2897,
					             "NSRS07.MI-CIF",  "NSRS 2007 Michigan State Planes, Central Zone, Intnl Foot",          "CRS HARN/MI.MI-CIF with datum modified to NSRS07",                    3588,
					             "NSRS11.MI-CIF",  "NSRS 2011 Michigan State Planes, Central Zone, Intnl Foot",          "CRS HARN/MI.MI-CIF with datum modified to NSRS11",                       0   },
	{               "HARN/MI.MI-N",    "HARN/MI",   2807,
					               "NSRS07.MI-N",  "NSRS 2007 Michigan State Planes, North Zone, Meter",                 "CRS HARN/MI.MI-N with datum modified to NSRS07",                      3589,
					               "NSRS11.MI-N",  "NSRS 2011 Michigan State Planes, North Zone, Meter",                 "CRS HARN/MI.MI-N with datum modified to NSRS11",                         0   },
	{              "HARN/MI.MI-NF",    "HARN/MI",      0,
					              "NSRS07.MI-NF",  "NSRS 2007 Michigan State Planes, North Zone, US Foot",               "CRS HARN/MI.MI-NF with datum modified to NSRS07",                        0,
					              "NSRS11.MI-NF",  "NSRS 2011 Michigan State Planes, North Zone, US Foot",               "CRS HARN/MI.MI-NF with datum modified to NSRS11",                        0   },
	{             "HARN/MI.MI-NIF",    "HARN/MI",   2896,
					             "NSRS07.MI-NIF",  "NSRS 2007 Michigan State Planes, North Zone, Intnl Foot",            "CRS HARN/MI.MI-NIF with datum modified to NSRS07",                    3590,
					             "NSRS11.MI-NIF",  "NSRS 2011 Michigan State Planes, North Zone, Intnl Foot",            "CRS HARN/MI.MI-NIF with datum modified to NSRS11",                       0   },
	{               "HARN/MI.MI-S",    "HARN/MI",   2809,
					               "NSRS07.MI-S",  "NSRS 2007 Michigan State Planes, South Zone, Meter",                 "CRS HARN/MI.MI-S with datum modified to NSRS07",                      3592,
					               "NSRS11.MI-S",  "NSRS 2011 Michigan State Planes, South Zone, Meter",                 "CRS HARN/MI.MI-S with datum modified to NSRS11",                         0   },
	{              "HARN/MI.MI-SF",    "HARN/MI",      0,
					              "NSRS07.MI-SF",  "NSRS 2007 Michigan State Planes, Southern Zone, US Foot",            "CRS HARN/MI.MI-SF with datum modified to NSRS07",                        0,
					              "NSRS11.MI-SF",  "NSRS 2011 Michigan State Planes, Southern Zone, US Foot",            "CRS HARN/MI.MI-SF with datum modified to NSRS11",                        0   },
	{             "HARN/MI.MI-SIF",    "HARN/MI",   2898,
					             "NSRS07.MI-SIF",  "NSRS 2007 Michigan State Planes, South Zone, Intnl Foot",            "CRS HARN/MI.MI-SIF with datum modified to NSRS07",                    3593,
					             "NSRS11.MI-SIF",  "NSRS 2011 Michigan State Planes, South Zone, Intnl Foot",            "CRS HARN/MI.MI-SIF with datum modified to NSRS11",                       0   },
	{               "HARN/MN.MN-C",    "HARN/MN",   2811,
					               "NSRS07.MN-C",  "NSRS 2007 Minnesota State Planes, Central Zone, Meter",              "CRS HARN/MN.MN-C with datum modified to NSRS07",                      3594,
					               "NSRS11.MN-C",  "NSRS 2011 Minnesota State Planes, Central Zone, Meter",              "CRS HARN/MN.MN-C with datum modified to NSRS11",                         0   },
	{              "HARN/MN.MN-CF",    "HARN/MN",  26858,
					              "NSRS07.MN-CF",  "NSRS 2007 Minnesota State Planes, Central Zone, US Foot",            "CRS HARN/MN.MN-CF with datum modified to NSRS07",                    26866,
					              "NSRS11.MN-CF",  "NSRS 2011 Minnesota State Planes, Central Zone, US Foot",            "CRS HARN/MN.MN-CF with datum modified to NSRS11",                        0   },
	{               "HARN/MN.MN-N",    "HARN/MN",   2810,
					               "NSRS07.MN-N",  "NSRS 2007 Minnesota State Planes, North Zone, Meter",                "CRS HARN/MN.MN-N with datum modified to NSRS07",                      3595,
					               "NSRS11.MN-N",  "NSRS 2011 Minnesota State Planes, North Zone, Meter",                "CRS HARN/MN.MN-N with datum modified to NSRS11",                         0   },
	{              "HARN/MN.MN-NF",    "HARN/MN",  26857,
					              "NSRS07.MN-NF",  "NSRS 2007 Minnesota State Planes, North Zone, US Foot",              "CRS HARN/MN.MN-NF with datum modified to NSRS07",                    26865,
					              "NSRS11.MN-NF",  "NSRS 2011 Minnesota State Planes, North Zone, US Foot",              "CRS HARN/MN.MN-NF with datum modified to NSRS11",                        0   },
	{               "HARN/MN.MN-S",    "HARN/MN",   2812,
					               "NSRS07.MN-S",  "NSRS 2007 Minnesota State Planes, South Zone, Meter",                "CRS HARN/MN.MN-S with datum modified to NSRS07",                      3596,
					               "NSRS11.MN-S",  "NSRS 2011 Minnesota State Planes, South Zone, Meter",                "CRS HARN/MN.MN-S with datum modified to NSRS11",                         0   },
	{              "HARN/MN.MN-SF",    "HARN/MN",  26859,
					              "NSRS07.MN-SF",  "NSRS 2007 Minnesota State Planes, South Zone, US Foot",              "CRS HARN/MN.MN-SF with datum modified to NSRS07",                    26867,
					              "NSRS11.MN-SF",  "NSRS 2011 Minnesota State Planes, South Zone, US Foot",              "CRS HARN/MN.MN-SF with datum modified to NSRS11",                        0   },
	{            "HARN/MS.MS-WF/a",    "HARN/MS",   2900,
					              "NSRS07.MS-WF",  "NSRS 2007 Mississippi State Planes, West Zone, US Foot",             "CRS HARN/MS.MS-WF/a with datum modified to NSRS07",                   3600,
					              "NSRS11.MS-WF",  "NSRS 2011 Mississippi State Planes, West Zone, US Foot",             "CRS HARN/MS.MS-WF/a with datum modified to NSRS11",                      0   },
	{               "HARN/MO.MO-C",    "HARN/MO",   2816,
					               "NSRS07.MO-C",  "NSRS 2007 Missouri State Planes, Central Zone, Meter",               "CRS HARN/MO.MO-C with datum modified to NSRS07",                      3601,
					               "NSRS11.MO-C",  "NSRS 2011 Missouri State Planes, Central Zone, Meter",               "CRS HARN/MO.MO-C with datum modified to NSRS11",                         0   },
	{              "HARN/MO.MO-CF",    "HARN/MO",      0,
					              "NSRS07.MO-CF",  "NSRS 2007 Missouri State Planes, Central Zone, US Foot",             "CRS HARN/MO.MO-CF with datum modified to NSRS07",                        0,
					              "NSRS11.MO-CF",  "NSRS 2011 Missouri State Planes, Central Zone, US Foot",             "CRS HARN/MO.MO-CF with datum modified to NSRS11",                        0   },
	{               "HARN/MO.MO-E",    "HARN/MO",   2815,
					               "NSRS07.MO-E",  "NSRS 2007 Missouri State Planes, East Zone, Meter",                  "CRS HARN/MO.MO-E with datum modified to NSRS07",                      3602,
					               "NSRS11.MO-E",  "NSRS 2011 Missouri State Planes, East Zone, Meter",                  "CRS HARN/MO.MO-E with datum modified to NSRS11",                         0   },
	{              "HARN/MO.MO-EF",    "HARN/MO",      0,
					              "NSRS07.MO-EF",  "NSRS 2007 Missouri State Planes, East Zone, US Foot",                "CRS HARN/MO.MO-EF with datum modified to NSRS07",                        0,
					              "NSRS11.MO-EF",  "NSRS 2011 Missouri State Planes, East Zone, US Foot",                "CRS HARN/MO.MO-EF with datum modified to NSRS11",                        0   },
	{               "HARN/MO.MO-W",    "HARN/MO",   2817,
					               "NSRS07.MO-W",  "NSRS 2007 Missouri State Planes, West Zone, Meter",                  "CRS HARN/MO.MO-W with datum modified to NSRS07",                      3603,
					               "NSRS11.MO-W",  "NSRS 2011 Missouri State Planes, West Zone, Meter",                  "CRS HARN/MO.MO-W with datum modified to NSRS11",                         0   },
	{              "HARN/MO.MO-WF",    "HARN/MO",      0,
					              "NSRS07.MO-WF",  "NSRS 2007 Missouri State Planes, West Zone, US Foot",                "CRS HARN/MO.MO-WF with datum modified to NSRS07",                        0,
					              "NSRS11.MO-WF",  "NSRS 2011 Missouri State Planes, West Zone, US Foot",                "CRS HARN/MO.MO-WF with datum modified to NSRS11",                        0   },
	{                 "HARN/NC.NC",    "HARN/NC",   3358,
					                 "NSRS07.NC",  "NSRS 2007 North Carolina State Planes, Meter",                       "CRS HARN/NC.NC with datum modified to NSRS07",                        3631,
					                 "NSRS11.NC",  "NSRS 2011 North Carolina State Planes, Meter",                       "CRS HARN/NC.NC with datum modified to NSRS11",                           0   },
	{                "HARN/NC.NCF",    "HARN/NC",   3404,
					                "NSRS07.NCF",  "NSRS 2007 North Carolina State Planes, US Foot",                     "CRS HARN/NC.NCF with datum modified to NSRS07",                       3632,
					                "NSRS11.NCF",  "NSRS 2011 North Carolina State Planes, US Foot",                     "CRS HARN/NC.NCF with datum modified to NSRS11",                          0   },
	{               "HARN/ND.ND-N",    "HARN/ND",   2832,
					               "NSRS07.ND-N",  "NSRS 2007 North Dakota State Planes, North Zone, Meter",             "CRS HARN/ND.ND-N with datum modified to NSRS07",                      3633,
					               "NSRS11.ND-N",  "NSRS 2011 North Dakota State Planes, North Zone, Meter",             "CRS HARN/ND.ND-N with datum modified to NSRS11",                         0   },
	{              "HARN/ND.ND-NF",    "HARN/ND",      0,
					              "NSRS07.ND-NF",  "NSRS 2007 North Dakota State Planes, North Zone, US Foot",           "CRS HARN/ND.ND-NF with datum modified to NSRS07",                        0,
					              "NSRS11.ND-NF",  "NSRS 2011 North Dakota State Planes, North Zone, US Foot",           "CRS HARN/ND.ND-NF with datum modified to NSRS11",                        0   },
	{               "HARN/ND.ND-S",    "HARN/ND",   2833,
					               "NSRS07.ND-S",  "NSRS 2007 North Dakota State Planes, South Zone, Meter",             "CRS HARN/ND.ND-S with datum modified to NSRS07",                      3635,
					               "NSRS11.ND-S",  "NSRS 2011 North Dakota State Planes, South Zone, Meter",             "CRS HARN/ND.ND-S with datum modified to NSRS11",                         0   },
	{              "HARN/ND.ND-SF",    "HARN/ND",      0,
					              "NSRS07.ND-SF",  "NSRS 2007 North Dakota State Planes, South Zone, US Foot",           "CRS HARN/ND.ND-SF with datum modified to NSRS07",                        0,
					              "NSRS11.ND-SF",  "NSRS 2011 North Dakota State Planes, South Zone, US Foot",           "CRS HARN/ND.ND-SF with datum modified to NSRS11",                        0   },
	{               "HARN/NV.NV-C",    "HARN/NV",   2821,
					               "NSRS07.NV-C",  "NSRS 2007 Nevada State Planes, Central Zone, Meter",                 "CRS HARN/NV.NV-C with datum modified to NSRS07",                      3607,
					               "NSRS11.NV-C",  "NSRS 2011 Nevada State Planes, Central Zone, Meter",                 "CRS HARN/NV.NV-C with datum modified to NSRS11",                         0   },
	{              "HARN/NV.NV-CF",    "HARN/NV",   3430,
					              "NSRS07.NV-CF",  "NSRS 2007 Nevada State Planes, Central Zone, US Foot",               "CRS HARN/NV.NV-CF with datum modified to NSRS07",                     3608,
					              "NSRS11.NV-CF",  "NSRS 2011 Nevada State Planes, Central Zone, US Foot",               "CRS HARN/NV.NV-CF with datum modified to NSRS11",                        0   },
	{               "HARN/NV.NV-E",    "HARN/NV",   2820,
					               "NSRS07.NV-E",  "NSRS 2007 Nevada State Planes, East Zone, Meter",                    "CRS HARN/NV.NV-E with datum modified to NSRS07",                      3609,
					               "NSRS11.NV-E",  "NSRS 2011 Nevada State Planes, East Zone, Meter",                    "CRS HARN/NV.NV-E with datum modified to NSRS11",                         0   },
	{              "HARN/NV.NV-EF",    "HARN/NV",   3429,
					              "NSRS07.NV-EF",  "NSRS 2007 Nevada State Planes, East Zone, US Foot",                  "CRS HARN/NV.NV-EF with datum modified to NSRS07",                     3610,
					              "NSRS11.NV-EF",  "NSRS 2011 Nevada State Planes, East Zone, US Foot",                  "CRS HARN/NV.NV-EF with datum modified to NSRS11",                        0   },
	{               "HARN/NV.NV-W",    "HARN/NV",   2822,
					               "NSRS07.NV-W",  "NSRS 2007 Nevada State Planes, West Zone, Meter",                    "CRS HARN/NV.NV-W with datum modified to NSRS07",                      3611,
					               "NSRS11.NV-W",  "NSRS 2011 Nevada State Planes, West Zone, Meter",                    "CRS HARN/NV.NV-W with datum modified to NSRS11",                         0   },
	{              "HARN/NV.NV-WF",    "HARN/NV",   3431,
					              "NSRS07.NV-WF",  "NSRS 2007 Nevada State Planes, West Zone, US Foot",                  "CRS HARN/NV.NV-WF with datum modified to NSRS07",                     3612,
					              "NSRS11.NV-WF",  "NSRS 2011 Nevada State Planes, West Zone, US Foot",                  "CRS HARN/NV.NV-WF with datum modified to NSRS11",                        0   },
	{                 "HARN/NJ.NJ",    "HARN/NJ",   2824,
					                 "NSRS07.NJ",  "NSRS 2007 New Jersey State Planes, Meter",                           "CRS HARN/NJ.NJ with datum modified to NSRS07",                        3615,
					                 "NSRS11.NJ",  "NSRS 2011 New Jersey State Planes, Meter",                           "CRS HARN/NJ.NJ with datum modified to NSRS11",                           0   },
	{                "HARN/NJ.NJF",    "HARN/NJ",   3432,
					                "NSRS07.NJF",  "NSRS 2007 New Jersey State Planes, US Foot",                         "CRS HARN/NJ.NJF with datum modified to NSRS07",                       3616,
					                "NSRS11.NJF",  "NSRS 2011 New Jersey State Planes, US Foot",                         "CRS HARN/NJ.NJF with datum modified to NSRS11",                          0   },
	{               "HARN/NY.NY-C",    "HARN/NY",   2829,
					               "NSRS07.NY-C",  "NSRS 2007 New York State Planes, Central Zone, Meter",               "CRS HARN/NY.NY-C with datum modified to NSRS07",                      3623,
					               "NSRS11.NY-C",  "NSRS 2011 New York State Planes, Central Zone, Meter",               "CRS HARN/NY.NY-C with datum modified to NSRS11",                         0   },
	{              "HARN/NY.NY-CF",    "HARN/NY",   2906,
					              "NSRS07.NY-CF",  "NSRS 2007 New York State Planes, Central Zone, US Foot",             "CRS HARN/NY.NY-CF with datum modified to NSRS07",                     3624,
					              "NSRS11.NY-CF",  "NSRS 2011 New York State Planes, Central Zone, US Foot",             "CRS HARN/NY.NY-CF with datum modified to NSRS11",                        0   },
	{               "HARN/NY.NY-E",    "HARN/NY",   2828,
					               "NSRS07.NY-E",  "NSRS 2007 New York State Planes, East Zone, Meter",                  "CRS HARN/NY.NY-E with datum modified to NSRS07",                      3625,
					               "NSRS11.NY-E",  "NSRS 2011 New York State Planes, East Zone, Meter",                  "CRS HARN/NY.NY-E with datum modified to NSRS11",                         0   },
	{              "HARN/NY.NY-EF",    "HARN/NY",   2905,
					              "NSRS07.NY-EF",  "NSRS 2007 New York State Planes, East Zone, US Foot",                "CRS HARN/NY.NY-EF with datum modified to NSRS07",                     3626,
					              "NSRS11.NY-EF",  "NSRS 2011 New York State Planes, East Zone, US Foot",                "CRS HARN/NY.NY-EF with datum modified to NSRS11",                        0   },
	{              "HARN/NY.NY-LI",    "HARN/NY",   2831,
					              "NSRS07.NY-LI",  "NSRS 2007 New York State Planes, Long Island, Meter",                "CRS HARN/NY.NY-LI with datum modified to NSRS07",                     3627,
					              "NSRS11.NY-LI",  "NSRS 2011 New York State Planes, Long Island, Meter",                "CRS HARN/NY.NY-LI with datum modified to NSRS11",                        0   },
	{             "HARN/NY.NY-LIF",    "HARN/NY",   2908,
					             "NSRS07.NY-LIF",  "NSRS 2007 New York State Planes, Long Island, US Foot",              "CRS HARN/NY.NY-LIF with datum modified to NSRS07",                    3628,
					             "NSRS11.NY-LIF",  "NSRS 2011 New York State Planes, Long Island, US Foot",              "CRS HARN/NY.NY-LIF with datum modified to NSRS11",                       0   },
	{               "HARN/NY.NY-W",    "HARN/NY",   2830,
					               "NSRS07.NY-W",  "NSRS 2007 New York State Planes, West Zone, Meter",                  "CRS HARN/NY.NY-W with datum modified to NSRS07",                      3629,
					               "NSRS11.NY-W",  "NSRS 2011 New York State Planes, West Zone, Meter",                  "CRS HARN/NY.NY-W with datum modified to NSRS11",                         0   },
	{              "HARN/NY.NY-WF",    "HARN/NY",   2907,
					              "NSRS07.NY-WF",  "NSRS 2007 New York State Planes, West Zone, US Foot",                "CRS HARN/NY.NY-WF with datum modified to NSRS07",                     3630,
					              "NSRS11.NY-WF",  "NSRS 2011 New York State Planes, West Zone, US Foot",                "CRS HARN/NY.NY-WF with datum modified to NSRS11",                        0   },
	{               "HARN/OH.OH-N",    "HARN/OH",   2834,
					               "NSRS07.OH-N",  "NSRS 2007 Ohio State Planes, North Zone, Meter",                     "CRS HARN/OH.OH-N with datum modified to NSRS07",                      3637,
					               "NSRS11.OH-N",  "NSRS 2011 Ohio State Planes, North Zone, Meter",                     "CRS HARN/OH.OH-N with datum modified to NSRS11",                         0   },
	{              "HARN/OH.OH-NF",    "HARN/OH",   3753,
					              "NSRS07.OH-NF",  "NSRS 2007 Ohio State Planes, North Zone, US Foot",                   "CRS HARN/OH.OH-NF with datum modified to NSRS07",                     3728,
					              "NSRS11.OH-NF",  "NSRS 2011 Ohio State Planes, North Zone, US Foot",                   "CRS HARN/OH.OH-NF with datum modified to NSRS11",                        0   },
	{               "HARN/OH.OH-S",    "HARN/OH",   2835,
					               "NSRS07.OH-S",  "NSRS 2007 Ohio State Planes, South Zone, Meter",                     "CRS HARN/OH.OH-S with datum modified to NSRS07",                      3638,
					               "NSRS11.OH-S",  "NSRS 2011 Ohio State Planes, South Zone, Meter",                     "CRS HARN/OH.OH-S with datum modified to NSRS11",                         0   },
	{              "HARN/OH.OH-SF",    "HARN/OH",   3754,
					              "NSRS07.OH-SF",  "NSRS 2007 Ohio State Planes, South Zone, US Foot",                   "CRS HARN/OH.OH-SF with datum modified to NSRS07",                     3729,
					              "NSRS11.OH-SF",  "NSRS 2011 Ohio State Planes, South Zone, US Foot",                   "CRS HARN/OH.OH-SF with datum modified to NSRS11",                        0   },
	{               "HARN/PA.PA-N",    "HARN/PA",   3362,
					               "NSRS07.PA-N",  "NSRS 2007 Pennsylvania State Planes, North Zone, Meter",             "CRS HARN/PA.PA-N with datum modified to NSRS07",                      3649,
					               "NSRS11.PA-N",  "NSRS 2011 Pennsylvania State Planes, North Zone, Meter",             "CRS HARN/PA.PA-N with datum modified to NSRS11",                         0   },
	{              "HARN/PA.PA-NF",    "HARN/PA",   3363,
					              "NSRS07.PA-NF",  "NSRS 2007 Pennsylvania State Planes, North Zone, US Foot",           "CRS HARN/PA.PA-NF with datum modified to NSRS07",                     3650,
					              "NSRS11.PA-NF",  "NSRS 2011 Pennsylvania State Planes, North Zone, US Foot",           "CRS HARN/PA.PA-NF with datum modified to NSRS11",                        0   },
	{               "HARN/PA.PA-S",    "HARN/PA",   3364,
					               "NSRS07.PA-S",  "NSRS 2007 Pennsylvania State Planes, South Zone, Meter",             "CRS HARN/PA.PA-S with datum modified to NSRS07",                      3651,
					               "NSRS11.PA-S",  "NSRS 2011 Pennsylvania State Planes, South Zone, Meter",             "CRS HARN/PA.PA-S with datum modified to NSRS11",                         0   },
	{              "HARN/PA.PA-SF",    "HARN/PA",   3365,
					              "NSRS07.PA-SF",  "NSRS 2007 Pennsylvania State Planes, South Zone, US Foot",           "CRS HARN/PA.PA-SF with datum modified to NSRS07",                     3652,
					              "NSRS11.PA-SF",  "NSRS 2011 Pennsylvania State Planes, South Zone, US Foot",           "CRS HARN/PA.PA-SF with datum modified to NSRS11",                        0   },
	{                 "HARN/SC.SC",    "HARN/SC",   3360,
					                 "NSRS07.SC",  "NSRS 2007 South Carolina State Planes, Meter",                       "CRS HARN/SC.SC with datum modified to NSRS07",                        3655,
					                 "NSRS11.SC",  "NSRS 2011 South Carolina State Planes, Meter",                       "CRS HARN/SC.SC with datum modified to NSRS11",                           0   },
	{                "HARN/SC.SCF",    "HARN/SC",      0,
					                "NSRS07.SCF",  "NSRS 2007 South Carolina State Planes, US Foot",                     "CRS HARN/SC.SCF with datum modified to NSRS07",                          0,
					                "NSRS11.SCF",  "NSRS 2011 South Carolina State Planes, US Foot",                     "CRS HARN/SC.SCF with datum modified to NSRS11",                          0   },
	{               "HARN/SC.SCIF",    "HARN/SC",   3361,
					               "NSRS07.SCIF",  "NSRS 2007 South Carolina State Planes, Intnl Foot",                  "CRS HARN/SC.SCIF with datum modified to NSRS07",                      3656,
					               "NSRS11.SCIF",  "NSRS 2011 South Carolina State Planes, Intnl Foot",                  "CRS HARN/SC.SCIF with datum modified to NSRS11",                         0   },
	{               "HARN/SD.SD-N",    "HARN/SD",   2841,
					               "NSRS07.SD-N",  "NSRS 2007 South Dakota State Planes, North Zone, Meter",             "CRS HARN/SD.SD-N with datum modified to NSRS07",                      3657,
					               "NSRS11.SD-N",  "NSRS 2011 South Dakota State Planes, North Zone, Meter",             "CRS HARN/SD.SD-N with datum modified to NSRS11",                         0   },
	{              "HARN/SD.SD-NF",    "HARN/SD",   3458,
					              "NSRS07.SD-NF",  "NSRS 2007 South Dakota State Planes, North Zone, US Foot",           "CRS HARN/SD.SD-NF with datum modified to NSRS07",                     3658,
					              "NSRS11.SD-NF",  "NSRS 2011 South Dakota State Planes, North Zone, US Foot",           "CRS HARN/SD.SD-NF with datum modified to NSRS11",                        0   },
	{               "HARN/SD.SD-S",    "HARN/SD",   2842,
					               "NSRS07.SD-S",  "NSRS 2007 South Dakota State Planes, South Zone, Meter",             "CRS HARN/SD.SD-S with datum modified to NSRS07",                      3659,
					               "NSRS11.SD-S",  "NSRS 2011 South Dakota State Planes, South Zone, Meter",             "CRS HARN/SD.SD-S with datum modified to NSRS11",                         0   },
	{              "HARN/SD.SD-SF",    "HARN/SD",   3459,
					              "NSRS07.SD-SF",  "NSRS 2007 South Dakota State Planes, South Zone, US Foot",           "CRS HARN/SD.SD-SF with datum modified to NSRS07",                     3660,
					              "NSRS11.SD-SF",  "NSRS 2011 South Dakota State Planes, South Zone, US Foot",           "CRS HARN/SD.SD-SF with datum modified to NSRS11",                        0   },
	{               "HARN/UT.UT-C",    "HARN/UT",   2850,
					               "NSRS07.UT-C",  "NSRS 2007 Utah State Planes, Central Zone, Meter",                   "CRS HARN/UT.UT-C with datum modified to NSRS07",                      3675,
					               "NSRS11.UT-C",  "NSRS 2011 Utah State Planes, Central Zone, Meter",                   "CRS HARN/UT.UT-C with datum modified to NSRS11",                         0   },
	{              "HARN/UT.UT-CF",    "HARN/UT",   3569,
					              "NSRS07.UT-CF",  "NSRS 2007 Utah State Planes, Central Zone, US Foot",                 "CRS HARN/UT.UT-CF with datum modified to NSRS07",                     3677,
					              "NSRS11.UT-CF",  "NSRS 2011 Utah State Planes, Central Zone, US Foot",                 "CRS HARN/UT.UT-CF with datum modified to NSRS11",                        0   },
	{             "HARN/UT.UT-CIF",    "HARN/UT",   2922,
					             "NSRS07.UT-CIF",  "NSRS 2007 Utah State Planes, Central Zone, Intnl Foot",              "CRS HARN/UT.UT-CIF with datum modified to NSRS07",                    3676,
					             "NSRS11.UT-CIF",  "NSRS 2011 Utah State Planes, Central Zone, Intnl Foot",              "CRS HARN/UT.UT-CIF with datum modified to NSRS11",                       1   },	// 1 here means a hard coded invalid value.  In this case, EPSG has not, as yet, assigned a code for this system (NSRS 2011).
	{               "HARN/UT.UT-N",    "HARN/UT",   2849,
					               "NSRS07.UT-N",  "NSRS 2007 Utah State Planes, North Zone, Meter",                     "CRS HARN/UT.UT-N with datum modified to NSRS07",                      3678,
					               "NSRS11.UT-N",  "NSRS 2011 Utah State Planes, North Zone, Meter",                     "CRS HARN/UT.UT-N with datum modified to NSRS11",                         0   },
	{              "HARN/UT.UT-NF",    "HARN/UT",   3568,
					              "NSRS07.UT-NF",  "NSRS 2007 Utah State Planes, North Zone, US Foot",                   "CRS HARN/UT.UT-NF with datum modified to NSRS07",                     3680,
					              "NSRS11.UT-NF",  "NSRS 2011 Utah State Planes, North Zone, US Foot",                   "CRS HARN/UT.UT-NF with datum modified to NSRS11",                        0   },
	{             "HARN/UT.UT-NIF",    "HARN/UT",   2921,
					             "NSRS07.UT-NIF",  "NSRS 2007 Utah State Planes, North Zone, Intnl Foot",                "CRS HARN/UT.UT-NIF with datum modified to NSRS07",                    3679,
					             "NSRS11.UT-NIF",  "NSRS 2011 Utah State Planes, North Zone, Intnl Foot",                "CRS HARN/UT.UT-NIF with datum modified to NSRS11",                       1   },	// 1 here means a hard coded invalid value.  In this case, EPSG has not, as yet, assigned a code for this system (NSRS 2011).
	{               "HARN/UT.UT-S",    "HARN/UT",   2851,
					               "NSRS07.UT-S",  "NSRS 2007 Utah State Planes, Southern Zone, Meter",                  "CRS HARN/UT.UT-S with datum modified to NSRS07",                      3681,
					               "NSRS11.UT-S",  "NSRS 2011 Utah State Planes, Southern Zone, Meter",                  "CRS HARN/UT.UT-S with datum modified to NSRS11",                         0   },
	{              "HARN/UT.UT-SF",    "HARN/UT",   3570,
					              "NSRS07.UT-SF",  "NSRS 2007 Utah State Planes, South Zone, US Foot",                   "CRS HARN/UT.UT-SF with datum modified to NSRS07",                     3683,
					              "NSRS11.UT-SF",  "NSRS 2011 Utah State Planes, South Zone, US Foot",                   "CRS HARN/UT.UT-SF with datum modified to NSRS11",                        0   },
	{             "HARN/UT.UT-SIF",    "HARN/UT",   2923,
					             "NSRS07.UT-SIF",  "NSRS 2007 Utah State Planes, South Zone, Intnl Foot",                "CRS HARN/UT.UT-SIF with datum modified to NSRS07",                    3682,
					             "NSRS11.UT-SIF",  "NSRS 2011 Utah State Planes, South Zone, Intnl Foot",                "CRS HARN/UT.UT-SIF with datum modified to NSRS11",                       1   },	// 1 here means a hard coded invalid value.  In this case, EPSG has not, as yet, assigned a code for this system (NSRS 2011).
	{               "HARN/WV.WV-N",    "HARN/WV",   2857,
					               "NSRS07.WV-N",  "NSRS 2007 West Virginia State Planes, North Zone, Meter",            "CRS HARN/WV.WV-N with datum modified to NSRS07",                      3693,
					               "NSRS11.WV-N",  "NSRS 2011 West Virginia State Planes, North Zone, Meter",            "CRS HARN/WV.WV-N with datum modified to NSRS11",                         0   },
	{              "HARN/WV.WV-NF",    "HARN/WV",  26861,
					              "NSRS07.WV-NF",  "NSRS 2007 West Virginia State Planes, North Zone, US Foot",          "CRS HARN/WV.WV-NF with datum modified to NSRS07",                    26869,
					              "NSRS11.WV-NF",  "NSRS 2011 West Virginia State Planes, North Zone, US Foot",          "CRS HARN/WV.WV-NF with datum modified to NSRS11",                        0   },
	{               "HARN/WV.WV-S",    "HARN/WV",   2858,
					               "NSRS07.WV-S",  "NSRS 2007 West Virginia State Planes, South Zone, Meter",            "CRS HARN/WV.WV-S with datum modified to NSRS07",                      3694,
					               "NSRS11.WV-S",  "NSRS 2011 West Virginia State Planes, South Zone, Meter",            "CRS HARN/WV.WV-S with datum modified to NSRS11",                         0   },
	{              "HARN/WV.WV-SF",    "HARN/WV",  26862,
					              "NSRS07.WV-SF",  "NSRS 2007 West Virginia State Planes, South Zone, US Foot",          "CRS HARN/WV.WV-SF with datum modified to NSRS07",                    26870,
					              "NSRS11.WV-SF",  "NSRS 2011 West Virginia State Planes, South Zone, US Foot",          "CRS HARN/WV.WV-SF with datum modified to NSRS11",                        0   },
	{          "HARN/CO.CO-SF-MOD",    "HARN/CO",   2878,
					              "NSRS07.CO-SF",  "NSRS 2007 Colorado State Planes, Southern Zone, US Foot",            "CRS HARN/CO.CO-SF-MOD with datum modified to NSRS07",                 3506,
					              "NSRS11.CO-SF",  "NSRS 2011 Colorado State Planes, Southern Zone, US Foot",            "CRS HARN/CO.CO-SF-MOD with datum modified to NSRS11",                    0   },
	//{             "HARN/02.UTM-2S",    "HARN/02",   2195,
	//				             "NSRS07.UTM-2S",  "NSRS07 UTM zone 2S",                                                 "CRS HARN/02.UTM-2S with datum modified to NSRS07",                       0,
	//				             "NSRS11.UTM-2S",  "NSRS 2011 UTM zone 2S",                                              "CRS HARN/02.UTM-2S with datum modified to NSRS11",                       0   },
	{             "HARN/ND.ND-NIF",    "HARN/ND",   2909,
					             "NSRS07.ND-NIF",  "NSRS07 North Dakota North (ft)",                                     "CRS HARN/ND.ND-NIF with datum modified to NSRS07",                    3634,
					             "NSRS11.ND-NIF",  "NSRS 2011 North Dakota North (ft)",                                  "CRS HARN/ND.ND-NIF with datum modified to NSRS11",                       0   },
	{             "HARN/ND.ND-SIF",    "HARN/ND",   2910,
					             "NSRS07.ND-SIF",  "NSRS07 North Dakota South (ft)",                                     "CRS HARN/ND.ND-SIF with datum modified to NSRS07",                    3636,
					             "NSRS11.ND-SIF",  "NSRS 2011 North Dakota South (ft)",                                  "CRS HARN/ND.ND-SIF with datum modified to NSRS11",                       0   },
	{      "HARN/WO.OregonLambert",    "HARN/WO",   2993,
					      "NSRS07.OregonLambert",  "NSRS07 Oregon Lambert",                                              "CRS HARN/WO.OregonLambert with datum modified to NSRS07",             3643,
					      "NSRS11.OregonLambert",  "NSRS 2011 Oregon Lambert",                                           "CRS HARN/WO.OregonLambert with datum modified to NSRS11",                0   },
	{    "HARN/WO.OregonLambert-F",    "HARN/WO",   2994,
					    "NSRS07.OregonLambert-F",  "NSRS07 Oregon Lambert (ft)",                                         "CRS HARN/WO.OregonLambert-F with datum modified to NSRS07",           3644,
					    "NSRS11.OregonLambert-F",  "NSRS 2011 Oregon Lambert (ft)",                                      "CRS HARN/WO.OregonLambert-F with datum modified to NSRS11",              0   },
	//{               "HARN/CO.TREX",    "HARN/CO",      0,
	//				               "NSRS07.TREX",  "",                                                                   "CRS HARN/CO.TREX with datum modified to NSRS07",                         0,
	//				               "NSRS11.TREX",  "",                                                                   "CRS HARN/CO.TREX with datum modified to NSRS11",                         0   },
	{        "HARN/WI.WisconsinTM",    "HARN/WI",   3071,
					        "NSRS07.WisconsinTM",  "NSRS07 Wisconsin Transverse Mercator",                               "CRS HARN/WI.WisconsinTM with datum modified to NSRS07",               3701,
					        "NSRS11.WisconsinTM",  "NSRS 2011 Wisconsin Transverse Mercator",                            "CRS HARN/WI.WisconsinTM with datum modified to NSRS11",               6610   },
	{             "HARN/ME.ME2K-E",    "HARN/ME",   3075,
					             "NSRS07.ME2K-E",  "NSRS07 Maine CS2000 East",                                           "CRS HARN/ME.ME2K-E with datum modified to NSRS07",                    3555,
					             "NSRS11.ME2K-E",  "NSRS 2011 Maine CS2000 East",                                        "CRS HARN/ME.ME2K-E with datum modified to NSRS11",                       0   },
	{           "HARN/ME.ME2K-C/a",    "HARN/ME",   3464,
					             "NSRS07.ME2K-C",  "NSRS07 Maine CS2000 Central",                                        "CRS HARN/ME.ME2K-C/a with datum modified to NSRS07",                  3554,
					             "NSRS11.ME2K-C",  "NSRS 2011 Maine CS2000 Central",                                     "CRS HARN/ME.ME2K-C/a with datum modified to NSRS11",                     0   },
	{             "HARN/ME.ME2K-W",    "HARN/ME",   3077,
					             "NSRS07.ME2K-W",  "NSRS07 Maine CS2000 West",                                           "CRS HARN/ME.ME2K-W with datum modified to NSRS07",                    3556,
					             "NSRS11.ME2K-W",  "NSRS 2011 Maine CS2000 West",                                        "CRS HARN/ME.ME2K-W with datum modified to NSRS11",                       0   },
	{     "HARN/MI.MichiganGeoRef",    "HARN/MI",   3079,
					     "NSRS07.MichiganGeoRef",  "NSRS07 Michigan Oblique Mercator",                                   "CRS HARN/MI.MichiganGeoRef with datum modified to NSRS07",            3591,
					     "NSRS11.MichiganGeoRef",  "NSRS 2011 Michigan Oblique Mercator",                                "CRS HARN/MI.MichiganGeoRef with datum modified to NSRS11",            6497   },	// There is no ESRI 2007 euivalent for this system, so this EPSG code has been hard coded.
	{      "HARN/TX.Texas/Lambert",    "HARN/TX",   3084,
					      "NSRS07.Texas/Lambert",  "NSRS07 Texas Centric Lambert Conformal",                             "CRS HARN/TX.Texas/Lambert with datum modified to NSRS07",             3666,
					      "NSRS11.Texas/Lambert",  "NSRS 2011 Texas Centric Lambert Conformal",                          "CRS HARN/TX.Texas/Lambert with datum modified to NSRS11",             6580   },	// There is no ESRI 2007 euivalent for this system, so this EPSG code has been hard coded.
	{       "HARN/TX.Texas/Albers",    "HARN/TX",   3085,
					       "NSRS07.Texas/Albers",  "NSRS07 Texas Centric Albers Equal Area",                             "CRS HARN/TX.Texas/Albers with datum modified to NSRS07",              3665,
					       "NSRS11.Texas/Albers",  "NSRS 2011 Texas Centric Albers Equal Area",                          "CRS HARN/TX.Texas/Albers with datum modified to NSRS11",              6579   },	// There is no ESRI 2007 euivalent for this system, so this EPSG code has been hard coded.
	{    "HARN/FL.FloridaGDL/Albr",    "HARN/FL",   3087,
					    "NSRS07.FloridaGDL/Albr",  "NSRS07 Florida GDL Albers",                                          "CRS HARN/FL.FloridaGDL/Albr with datum modified to NSRS07",           3513,
					    "NSRS11.FloridaGDL/Albr",  "NSRS 2011 Florida GDL Albers",                                       "CRS HARN/FL.FloridaGDL/Albr with datum modified to NSRS11",           6439   },	// There is no ESRI 2007 euivalent for this system, so this EPSG code has been hard coded.
	{                 "HARN/KY.KY",    "HARN/KY",   3090,
					                 "NSRS07.KY",  "NSRS07 Kentucky Single Zone",                                        "CRS HARN/KY.KY with datum modified to NSRS07",                        3546,
					                 "NSRS11.KY",  "NSRS 2011 Kentucky Single Zone",                                     "CRS HARN/KY.KY with datum modified to NSRS11",                           0   },
	{                "HARN/KY.KYF",    "HARN/KY",   3091,
					                "NSRS07.KYF",  "NSRS07 Kentucky Single Zone (ftUS)",                                 "CRS HARN/KY.KYF with datum modified to NSRS07",                       3547,
					                "NSRS11.KYF",  "NSRS 2011 Kentucky Single Zone (ftUS)",                              "CRS HARN/KY.KYF with datum modified to NSRS11",                          0   },
	{           "HARN/CA.CA/Teale",    "HARN/CA",   3311,
					           "NSRS07.CA/Teale",  "",                                                                   "CRS HARN/CA.CA/Teale with datum modified to NSRS07",                  3488,
					           "NSRS11.CA/Teale",  "",                                                                   "CRS HARN/CA.CA/Teale with datum modified to NSRS11",                  6414  },	// There is no ESRI 2007 equivalent definition for this system; so the 2011 EPSG code is hard coded.
																																																			// Of course, there is no ESRI 2011 euivalent either, but we still need to associate the EPSG code with the CS-MAP definition.
	//{              "NVHRN.NCRS-LV",    "HARN/NV",      0,
	//				             "NVHRN.NCRS-LV",  "NSRS 2007 NCRS, Las Vegas Zone, Meter",                              "CRS NVHRN.NCRS-LV with datum modified to NSRS07",                        0,
	//				             "NVHRN.NCRS-LV",  "NSRS 2011 NCRS, Las Vegas Zone, Meter",                              "CRS NVHRN.NCRS-LV with datum modified to NSRS11",                        0   },
	//{             "NVHRN.NCRS-LVF",    "HARN/NV",      0,
	//				            "NVHRN.NCRS-LVF",  "NSRS 2007 NCRS, Las Vegas Zone, US Foot",                            "CRS NVHRN.NCRS-LVF with datum modified to NSRS07",                       0,
	//				            "NVHRN.NCRS-LVF",  "NSRS 2011 NCRS, Las Vegas Zone, US Foot",                            "CRS NVHRN.NCRS-LVF with datum modified to NSRS11",                       0   },
	//{            "NVHRN.NCRS-LVHE",    "HARN/NV",      0,
	//				           "NVHRN.NCRS-LVHE",  "NSRS 2007 NCRS, Las Vegas High Elev Zone, Meter",                    "CRS NVHRN.NCRS-LVHE with datum modified to NSRS07",                      0,
	//				           "NVHRN.NCRS-LVHE",  "NSRS 2011 NCRS, Las Vegas High Elev Zone, Meter",                    "CRS NVHRN.NCRS-LVHE with datum modified to NSRS11",                      0   },
	//{           "NVHRN.NCRS-LVHEF",    "HARN/NV",      0,
	//				          "NVHRN.NCRS-LVHEF",  "NSRS 2007 NCRS, Las Vegas High Elev Zone, US Foot",                  "CRS NVHRN.NCRS-LVHEF with datum modified to NSRS07",                     0,
	//				          "NVHRN.NCRS-LVHEF",  "NSRS 2011 NCRS, Las Vegas High Elev Zone, US Foot",                  "CRS NVHRN.NCRS-LVHEF with datum modified to NSRS11",                     0   },
	{                           "",           "",      0,
								              "",                                                   "",                  "",                                                                       0,
								              "",                                                   "",                  "",                                                                       0   }
};

struct cs_RplTable_
{
	char harnFind  [64];
	char nsrs07Rpl [64];
	char nsrs11Rpl [64];
};
cs_RplTable_ cs_GrpRplTable [] = 
{
	{        "LL",         "LL",  "LL"           },
	{    "SPCSHP",   "SPNSRS07",  "SPNSRS11"     },
	{   "SPCSHPF",  "SPNSRS07F",  "SPNSRS11F"    },
	{   "SPCSHPI",  "SPNSRS07I",  "SPNSRS11I"    },
	{     "UTMHP",  "UTMNSRS07",  "UTMNSRS11"    },
	{    "UTMHPF", "UTMNSRS07F",  "UTMNSRS11F"   },
	{    "UTMHPI", "UTMNSRS07I",  "UTMNSRS11I"   },
	{   "OTHR-US",    "OTHR-US",  "OTHR-US"      },
	{   "EPSGPRJ",    "EPSGPRJ",  "EPSGPRJ"      },
	{    "SPCS83",   "SPNSRS07",  "SPNSRS11"     },	// To convert several busts in the current coordsys.asc file.
	{   "SPCS83F",  "SPNSRS07F",  "SPNSRS11F"    },	// To convert several busts in the current coordsys.asc file.
	{   "SPCS83I",  "SPNSRS07I",  "SPNSRS11I"    },	// To convert several busts in the current coordsys.asc file.
	{          "",           "",  ""             },
};

cs_RplTable_ cs_DescRplTable [] = 
{
	{       "HPGN (HARN)",  "NSRS 2007",   "NSRS 2011"    },
	{   "HPGN (aka HPGN)",  "NSRS 2007",   "NSRS 2011"    },
	{ "HARN (HPGN datum)",  "NSRS 2007",   "NSRS 2011"    },
	{              "HPGN",  "NSRS 2007",   "NSRS 2011"    },
	{     "NAD83(HARN) /",     "NSRS07",   "NSRS 2011"    },
	{                  "",           "",            ""    }
};

// Turns out, we don't really need this; not yet anyway.
cs_RplTable_ cs_SourceRplTable [] = 
{
	{                  "",           "",            ""    }
};


bool CS_epsgHarnToNsrs (const TcsEpsgDataSetV6* epsgPtr,ulong32_t& epsg2007,ulong32_t& epsg2011,ulong32_t epsgHarn);
bool csAddCrsToCategoryFile (TcsCategoryFile& categoryFile,const cs_Add2007And2011_* addTblPtr);
bool csAddToNameMapper (TcsNameMapper& nameMapper,const cs_Add2007And2011_* addTblPtr,bool isGeographic);
bool csAddNameMap (TcsNameMapper& nameMapper,bool isGeographic,unsigned long epsgCode,EcsNameFlavor flavor,const char* crsName);
bool CS_epsgNsrs07Name (const TcsEpsgDataSetV6* epsgPtr,char* nsrs07Name,size_t nameSize,ulong32_t& epsg2007);
TcsGenericId csGetGenericIdFoot (TcsNameMapper& nameMapper,EcsMapObjType objType,const char* crsName);

bool csAddNsrs07Nsrs11 (const wchar_t* csDictSrcDir,const wchar_t* csDictTrgDir)
{
	bool ok (false);
	bool isGeographic;
	
	EcsDictType dictType;
	EcsCsvStatus csvReadStatus;

	char workBuffer [MAXPATH];
	char insertAfter [MAXPATH];
	char insertBefore [MAXPATH];
	char csSrcAscFilePath [MAXPATH];
	char csTrgAscFilePath [MAXPATH];
	
	const char *chPtr;
	cs_RplTable_* rplTblPtr;
	TcsAscDefinition *harnDefPtr;
	struct cs_Add2007And2011_* addTblPtr;

	std::wifstream iStream;
	
	TcsCategoryFile categoryAsc;
	TcsCsvStatus csvStatus;
	TcsNameMapper nameMapper;

	// We insert all NSRS 2007 definitions after "JGD2011-19", and insert all
	// NSRS 2011 definitions before "Petrels72.TerreAdelie/1".  This keeps
	// the various definitions in a logical placce, in a logical order.
	CS_stncp (insertAfter,"JGD2011-19",sizeof (insertAfter));
	CS_stncp (insertBefore,"Petrels72.TerreAdelie/1",sizeof (insertBefore));

	// Open up the Coordinate System ASCII definition file.
	wcstombs (csSrcAscFilePath,csDictSrcDir,sizeof (csSrcAscFilePath));
	CS_envsub (csSrcAscFilePath,sizeof (csSrcAscFilePath));
	CS_stncat (csSrcAscFilePath,"\\coordsys.asc",sizeof (csSrcAscFilePath));
	TcsDefFile coordsysAsc (dictTypCoordsys,csSrcAscFilePath);

	// Set up the path for the Target Coordinate System ASCII definition file

	// If the above fails for any reason, the Dictionary Type will not be
	// valid.
	dictType = coordsysAsc.GetDictType ();
	if (dictType != dictTypCoordsys)
	{
		return false;
	}

	// Eventually we'll need access to the Category data file.
	wcstombs (csSrcAscFilePath,csDictSrcDir,sizeof (csSrcAscFilePath));
	CS_envsub (csSrcAscFilePath,sizeof (csSrcAscFilePath));
	CS_stncat (csSrcAscFilePath,"\\category.asc",sizeof (csSrcAscFilePath));
	ok = categoryAsc.InitializeFromFile (csSrcAscFilePath);
	if (!ok)
	{
		return ok;
	}

	// Eventually, we'll need to twiddle the NameMapper file.
	wcstombs (csSrcAscFilePath,csDictSrcDir,sizeof (csSrcAscFilePath));
	CS_envsub (csSrcAscFilePath,sizeof (csSrcAscFilePath));
	CS_stncat (csSrcAscFilePath,"\\NameMapper.csv",sizeof (csSrcAscFilePath));
	iStream.open (csSrcAscFilePath,std::ios_base::in);
	ok = iStream.is_open ();
	if (ok)
	{
		csvReadStatus = nameMapper.ReadFromStream (iStream,csvStatus);
		ok = (csvReadStatus == csvOk);
		iStream.close ();
	}
	if (!ok)
	{
		return false;
	}

	// The nsrs11EsriCode element in the above structure was added added after
	// the original purpose of this module was completed.  We do the following
	// simply make sure it gets initialized to an appropriate value.
	for (addTblPtr = cs_Add2007And2011;ok && addTblPtr->harnKeyName[0] != '\0';addTblPtr += 1)
	{
		addTblPtr->nsrs11EsriCode = 0UL;
	}

	ok = true;					// Until we know differently.
	for (addTblPtr = cs_Add2007And2011;ok && addTblPtr->harnKeyName[0] != '\0';addTblPtr += 1)
	{
		// We declare this here to insure that the memory allocated by the
		// creation of a new definition below is deleted.  Thi8s, of course,
		// works as the std::vector.insert () function causes a call to the
		// copy constructor.  (This iis fine as the performance of this module
		// is not an issue.
		TcsAscDefinition* nsrs07DefPtr = 0; 
		TcsAscDefinition* nsrs11DefPtr = 0; 
		isGeographic = false;

		// After much agony and pain, it is decided to skip all entries for
		// which there is no HARN EPSG code.  Such entries in the above table
		// are usually the result of CS-MAP supporting US Survey feet for a
		// system for which there is no official definition.  For example, use
		// of International Feet for NAD83 (and later) datum is sanctioned by
		// the state legislature in Arizona while US foot is snactioned for use
		// only with NAD27.  That CS-MAP supported both US Foot and
		// International foot in Arizona (NAD83 and HARN) was an arbitrary
		// decision by the CS-MAP developers.  NOthing wrong with it, but a
		// system definition which is not "officially" sanctioned.
		//
		// Anyway, we skip these definitionsin this module as there are no
		// EPSG, ESRI, or Oracle mappings; so we leave them out.
		if (addTblPtr->harnEpsgCode == 0UL)
		{
			continue;
		}

		harnDefPtr = coordsysAsc.GetDefinition (addTblPtr->harnKeyName);
		ok = (harnDefPtr != 0);
		if (ok)
		{
			// We may need to know this below:
			chPtr = harnDefPtr->GetValue ("PROJ:");
			ok = (chPtr != 0);
			if (ok)
			{
				isGeographic = !CS_stricmp (chPtr,"LL");
			}
		}
		if (ok)
		{
			// We need to duplicate this entry, modify it as appropriate,
			// and insert it before a specific definition.  The choice of
			// the "insert before" system is rather arbitrary.  I have
			// chosen "Petrels72.TerreAdelie/1" as it seems to be the first
			// of a long list of deprecated systems which appear at the
			// end of the .asc source file.
			nsrs07DefPtr = new TcsAscDefinition (*harnDefPtr);
			nsrs11DefPtr = new TcsAscDefinition (*harnDefPtr);

			// Trim any line comments in the beginning or the end.  These
			// will usually apply to the existing definition and NOT
			// to the new definition which we will be creating.
			nsrs07DefPtr->TrimLineComments ();
			nsrs11DefPtr->TrimLineComments ();

			// If one does not exist, we add an EPSG: line to each definition.
			// We assume that such exists below.
			TcsDefLine* defLinePtr;
			defLinePtr = nsrs07DefPtr->GetLine ("EPSG:");
			if (defLinePtr == 0)
			{
				TcsDefLine newLine (dictTypCoordsys,"EPSG:","0",NULL);
				nsrs07DefPtr->InsertAfter ("SOURCE:",newLine);
			}
			defLinePtr = nsrs11DefPtr->GetLine ("EPSG:");
			if (defLinePtr == 0)
			{
				TcsDefLine newLine (dictTypCoordsys,"EPSG:","0",NULL);
				nsrs11DefPtr->InsertAfter ("SOURCE:",newLine);
			}

			// We need to make four changes to each of these definitions:
			//	1> The CS_NAME of the system.
			//	2> The DESC_NM of the system
			//	3> The SOURCE of the system.
			//	4> The GROUP of the system.
			//	5> The DT_NAME of the system.
			//	6> The EPSG code of the system.
			//	7> AND MAYBE the Oracle SRID code, assuming it will be the same as the EPSG code.
			
			ok = nsrs07DefPtr->RenameDef (addTblPtr->nsrs07KeyName);
			if (ok)
			{
				ok = nsrs11DefPtr->RenameDef (addTblPtr->nsrs11KeyName);
			}
		}
		
		// Most importantly, set the new datum name.
		if (ok)
		{
			ok = nsrs07DefPtr->SetValue ("DT_NAME:","NSRS07");
		}
		if (ok)
		{
			ok = nsrs11DefPtr->SetValue ("DT_NAME:","NSRS11");
		}

		// We get rather verbose here, but it makes debugging much easier.
		if (ok)
		{
			chPtr = harnDefPtr->GetValue ("GROUP:");
			ok = (chPtr != 0);
			if (ok)
			{
				CS_stncp (workBuffer,chPtr,sizeof (workBuffer));
				// Locate the proper group entry in the group replace table.
				for (rplTblPtr = cs_GrpRplTable;ok && rplTblPtr->harnFind[0]!='\0';rplTblPtr += 1)
				{
					if (!CS_stricmp (workBuffer,rplTblPtr->harnFind))
					{
						ok = nsrs07DefPtr->SetValue ("GROUP:",rplTblPtr->nsrs07Rpl);
						if (ok)
						{
							ok = nsrs11DefPtr->SetValue ("GROUP:",rplTblPtr->nsrs11Rpl);
						}
						break;
					}
				}
				if (rplTblPtr->harnFind [0] == '\0')
				{
					// Should these values end up in the result, they will be flagged by
					// the compiler and be properly fixed.
					nsrs07DefPtr->SetValue ("GROUP:","XXXXX");
					nsrs11DefPtr->SetValue ("GROUP:","XXXXX");
				}
			}
		}
		if (ok)
		{
			ok = nsrs07DefPtr->SetValue ("DESC_NM:",addTblPtr->nsrs07Desc);
		}
		if (ok)
		{
			ok = nsrs11DefPtr->SetValue ("DESC_NM:",addTblPtr->nsrs11Desc);
		}
		if (ok)
		{
			ok = nsrs07DefPtr->SetValue ("SOURCE:",addTblPtr->nsrs07Source);
		}
		if (ok)
		{
			ok = nsrs11DefPtr->SetValue ("SOURCE:",addTblPtr->nsrs11Source);
		}
		if (ok)
		{
			if (addTblPtr->nsrs07EpsgCode != 0)
			{
				sprintf (workBuffer,"%ld",addTblPtr->nsrs07EpsgCode);
				ok = nsrs07DefPtr->SetValue ("EPSG:",workBuffer);
			}
			else
			{
				// We ignore status here, the SRID line may be there, it might
				// not be.  We don't really care; we just want it out of there.
				nsrs07DefPtr->RemoveLine ("EPSG:");
			}
			if (ok)
			{
				// We ignore status here, the SRID line may be there, it might
				// not be.
				nsrs07DefPtr->RemoveLine ("SRID:");
			}
		}
		if (ok)
		{
			if (addTblPtr->nsrs11EpsgCode != 0)
			{
				sprintf (workBuffer,"%ld",addTblPtr->nsrs11EpsgCode);
				ok = nsrs11DefPtr->SetValue ("EPSG:",workBuffer);
			}
			else
			{
				// We ignore status here, the EPSG line may be there, it might
				// not be.
				nsrs11DefPtr->RemoveLine ("EPSG:");
			}
			if (ok)
			{
				// We ignore status here, the SRID line may be there, it might
				// not be.
				nsrs11DefPtr->RemoveLine ("SRID:");
			}
		}

		// Need to insert both entries into the file.
		if (ok)
		{
			ok = coordsysAsc.InsertAfter (insertAfter,(*nsrs07DefPtr));
			CS_stncp (insertAfter,addTblPtr->nsrs07KeyName,sizeof (insertAfter));
		}
		if (ok)
		{
			ok = coordsysAsc.InsertBefore (insertBefore,(*nsrs11DefPtr));
		}
		if (!ok)
		{
			// For debugging purposes.
			ok = false;
		}

		// Update the category file to account for the two new coordinate systems.
		if (ok)
		{
			ok = csAddCrsToCategoryFile (categoryAsc,addTblPtr);
		}

		// OK, we need to add several records to the Namemapper.
		if (ok)
		{
			ok = csAddToNameMapper (nameMapper,addTblPtr,isGeographic);
			
			// The following statement was added after rigorous testing.
			// There are five known failures at this point, the failures being
			// related to special "whole state" coordinate systems for states
			// which have multiple stat plane zones.  The failures occur
			// because there is no 2011 euivalent in EPSG or ESRI, while there
			// is an EPSG and ESRI equivalent for 2007.  So, rather then
			// devising some radical kludge, we simply ignore these 5 errors
			// since no harm comes from doing so.
			ok = true;
		}

		// For debugging convenience.
		if (!ok)
		{
			ok = false;
		}
	}
	if (ok)
	{
		wcstombs (csTrgAscFilePath,csDictTrgDir,sizeof (csTrgAscFilePath));
		CS_envsub (csTrgAscFilePath,sizeof (csTrgAscFilePath));
		CS_stncat (csTrgAscFilePath,"\\coordsys.asc",sizeof (csTrgAscFilePath));
		ok = coordsysAsc.WriteToFile (csTrgAscFilePath);
	}
	if (ok)
	{
		wcstombs (csTrgAscFilePath,csDictTrgDir,sizeof (csTrgAscFilePath));
		CS_envsub (csTrgAscFilePath,sizeof (csTrgAscFilePath));
		CS_stncat (csTrgAscFilePath,"\\category.asc",sizeof (csTrgAscFilePath));
		ok = categoryAsc.WriteToFile (csTrgAscFilePath);
	}
	if (ok)
	{
		wcstombs (csTrgAscFilePath,csDictTrgDir,sizeof (csTrgAscFilePath));
		CS_envsub (csTrgAscFilePath,sizeof (csTrgAscFilePath));
		CS_stncat (csTrgAscFilePath,"\\NameMapper.csv",sizeof (csTrgAscFilePath));
		std::wofstream oStream (csTrgAscFilePath,std::ios_base::out | std::ios_base::trunc);
		if (oStream.is_open ())
		{
			nameMapper.WriteAsCsv (oStream,true);
			oStream.close ();
		}
	}
	return ok;
}
// This module writes in 'C' table syntax a listing all non-deprecated HARN
// based CRS systems including:
//		1> CS-MAP CRS Name,
//		2> Current Datum reference
//		3> Current EPSG code
//		4> Proposed name for the new 2007 version.
//		5> Proposed name for the new 2011 version.

bool csWriteNsrsList (const wchar_t* csDictSrcDir,const wchar_t* csTrgDataFileDir)
{
	bool ok (false);
	bool descOk;
	
	unsigned long harnEpsgCode (0UL);
	unsigned long nsrs07EpsgCode (0UL);
	unsigned long nsrs11EpsgCode (0UL);

	EcsDictType dictType;

	char csSrcAscFilePath [MAXPATH];
	char csTrgDataFilePath [MAXPATH];
	
	char ccBuffer  [MAXPATH];
	char ccBuffer1 [MAXPATH];
	char ccBuffer2 [MAXPATH];
	char ccBuffer3 [MAXPATH];
	char ccBuffer4 [MAXPATH];
	char ccBuffer5 [MAXPATH];
	char ccBuffer6 [MAXPATH];
	char ccBuffer7 [MAXPATH];
	char ccBuffer8 [MAXPATH];
	char workBuffer [MAXPATH];
	wchar_t wcBuffer [MAXPATH];

	char *dummy;
	const char *chPtr;
	TcsDefLine* defLinePtr;
	TcsAscDefinition* ascDefPtr;
	cs_RplTable_* descRplTblPtr;

	std::wofstream oStream;

	char defKeyName     [cs_KEYNM_DEF];
	char datumKeyName   [cs_KEYNM_DEF];
	char defKeyName2007 [cs_KEYNM_DEF];
	char nsrs07DescName [64];
	char nsrs07SrcName  [64];
	char defKeyName2011 [cs_KEYNM_DEF];
	char nsrs11DescName [64];
	char nsrs11SrcName  [64];

	const TcsEpsgDataSetV6* epsgPtr;
	
	epsgPtr = GetEpsgObjectPtr ();
	ok = (epsgPtr != 0);

	// Open up the Coordinate System ASCII definition file.
	wcstombs (csSrcAscFilePath,csDictSrcDir,sizeof (csSrcAscFilePath));
	CS_stncat (csSrcAscFilePath,"\\coordsys.asc",sizeof (csSrcAscFilePath));
	TcsDefFile coordsysAsc (dictTypCoordsys,csSrcAscFilePath);

	// If the above fails for any reason, the Dictionary Type will not be
	// valid.
	dictType = coordsysAsc.GetDictType ();
	if (dictType != dictTypCoordsys)
	{
		ok = false;
	}

	// Prepare the output file.  Note, the resulting file wioll require some
	// manual tweaking.  This is fully anticipated.
	wcstombs (csTrgDataFilePath,csTrgDataFileDir,sizeof (csTrgDataFilePath));
	CS_stncat (csTrgDataFilePath,"\\Add2011.c",sizeof (csTrgDataFilePath));
	oStream.open (csTrgDataFilePath,std::ios_base::out | std::ios_base::trunc);
	if (!oStream.is_open ())
	{
		ok = false;
	}
	
	if (!ok)
	{
		return ok;
	}

	ok = true;									// Until we know differently.
	size_t defCount = coordsysAsc.GetDefinitionCount ();
	for (unsigned index = 0;ok && index < defCount;index += 1)
	{
		// Here once for each definition in the file.
		ascDefPtr = &(*coordsysAsc [index]);
		if (ascDefPtr == 0)
		{
			ok = false;
			break;
		}

		// Get the definition name.
		chPtr = ascDefPtr->GetDefinitionName ();
		if (chPtr == 0)
		{
			// Should only be the last enry in the file.  Maybe this is bug in
			// the TcsDefFile object.
			continue;
		}
		CS_stncp (defKeyName,chPtr,sizeof (defKeyName));

		// Extract the Datum code. If it is something other than "HARN", we
		// don't need to do anything which this entry.
		defLinePtr = ascDefPtr->GetLine ("DT_NAME:");
		if (defLinePtr == 0)
		{
			continue;
		}
		chPtr = defLinePtr->GetValue ();
		if (chPtr != 0)
		{
			CS_stncp (datumKeyName,chPtr,sizeof (datumKeyName));
			if (CS_strnicmp (datumKeyName,"HARN/",5))
			{
				// On to the next definition if this definition is not 
				continue;
			}
			if (!CS_stricmp (datumKeyName,"HARN/HI"))
			{
				// 2007 and/or 2011 not defined for Hawaii
				continue;
			}
		}

		// Get the current EPSG code value if one is there.
		harnEpsgCode = 0UL;
		defLinePtr = ascDefPtr->GetLine ("EPSG:");
		if (defLinePtr != 0)
		{
			// There is an EPSG label, see if the value is correct.
			chPtr = defLinePtr->GetValue ();
			if (chPtr != 0)
			{
				harnEpsgCode = strtoul (chPtr,&dummy,10);
			}
		}

		// Propose new names for the 2007 and 2011 version.
		CS_stncp (defKeyName2007,defKeyName,sizeof (defKeyName2007));
		CS_stncp (defKeyName2011,defKeyName,sizeof (defKeyName2011));
		CS_strrpl (defKeyName2007,sizeof (defKeyName2007),datumKeyName,"NSRS07");
		CS_strrpl (defKeyName2011,sizeof (defKeyName2011),datumKeyName,"NSRS11");
		
		// Propose new descriptions.
		descOk = false;
		memset (nsrs07DescName,'\0',sizeof (nsrs07DescName));
		chPtr = ascDefPtr->GetValue ("DESC_NM:");
		ok = (chPtr != 0);
		if (ok)
		{
			CS_stncp (workBuffer,chPtr,sizeof (workBuffer));
			descOk = CS_strrpl (workBuffer,sizeof (workBuffer),datumKeyName,"NSRS 2007");
			if (descOk)
			{
				CS_stncp (nsrs07DescName,workBuffer,sizeof (nsrs07DescName));
			}
			else
			{
				for (descRplTblPtr = cs_DescRplTable;descRplTblPtr->harnFind[0] != '\0';descRplTblPtr += 1)
				{
					if (CS_strrpl (workBuffer,sizeof (workBuffer),descRplTblPtr->harnFind,descRplTblPtr->nsrs07Rpl))
					{
						CS_stncp (nsrs07DescName,workBuffer,sizeof (nsrs07DescName));
						descOk = true;
						break;
					}
				}
			}
		}

		descOk = false;
		memset (nsrs11DescName,'\0',sizeof (nsrs11DescName));
		chPtr = ascDefPtr->GetValue ("DESC_NM:");
		ok = (chPtr != 0);
		if (ok)
		{
			CS_stncp (workBuffer,chPtr,sizeof (workBuffer));
			descOk = CS_strrpl (workBuffer,sizeof (workBuffer),datumKeyName,"NSRS 2011");
			if (descOk)
			{
				CS_stncp (nsrs11DescName,workBuffer,sizeof (nsrs07DescName));
			}
			else
			{
				CS_stncp (workBuffer,chPtr,sizeof (workBuffer));
				for (descRplTblPtr = cs_DescRplTable;descRplTblPtr->harnFind[0] != '\0';descRplTblPtr += 1)
				{
					if (CS_strrpl (workBuffer,sizeof (workBuffer),descRplTblPtr->harnFind,descRplTblPtr->nsrs11Rpl))
					{
						CS_stncp (nsrs11DescName,workBuffer,sizeof (nsrs11DescName));
						descOk = true;
						break;
					}
				}
			}
		}

		// Propose new "Source" values.
		memset (nsrs07SrcName,'\0',sizeof (nsrs07SrcName));
		chPtr = ascDefPtr->GetValue ("SOURCE:");
		if (chPtr != 0)
		{
			sprintf (nsrs07SrcName,"Modified %s; datum set to NSRS07",defKeyName);
			//CS_stncp (workBuffer,chPtr,sizeof (workBuffer));
			//for (srcRplTblPtr = cs_SourceRplTable;srcRplTblPtr->harnFind[0] != '\0';srcRplTblPtr += 1)
			//{
			//	if (CS_strrpl (workBuffer,sizeof (workBuffer),srcRplTblPtr->harnFind,srcRplTblPtr->nsrs07Rpl))
			//	{
			//		CS_stncp (nsrs07SrcName,workBuffer,sizeof (nsrs07SrcName));
			//		break;
			//	}
			//} 
		}
		memset (nsrs11SrcName,'\0',sizeof (nsrs11SrcName));
		chPtr = ascDefPtr->GetValue ("SOURCE:");
		if (chPtr != 0)
		{
			sprintf (nsrs11SrcName,"Modified %s; datum set to NSRS07",defKeyName);
			//CS_stncp (workBuffer,chPtr,sizeof (workBuffer));
			//for (srcRplTblPtr = cs_SourceRplTable;srcRplTblPtr->harnFind[0] != '\0';srcRplTblPtr += 1)
			//{
			//	if (CS_strrpl (workBuffer,sizeof (workBuffer),srcRplTblPtr->harnFind,srcRplTblPtr->nsrs11Rpl))
			//	{
			//		CS_stncp (nsrs11SrcName,workBuffer,sizeof (nsrs11SrcName));
			//		break;
			//	}
			//} 
		}
		
		// Get the EPSG codes for the NSRS 2007 definitions.  EPSG has not
		// assigned code values for NSRS 2011 yet.
		nsrs07EpsgCode = 0UL;
		nsrs11EpsgCode = 0UL;
		if (harnEpsgCode != 0UL)
		{
			ok = CS_epsgHarnToNsrs (epsgPtr,nsrs07EpsgCode,nsrs11EpsgCode,harnEpsgCode);
		}

		if (ok)
		{
			// The C code table needs quoted strings, a bit of a pain.
			sprintf (ccBuffer1,"\"%s\",",defKeyName);
			sprintf (ccBuffer2,"\"%s\",",datumKeyName);
			sprintf (ccBuffer3,"\"%s\",",defKeyName2007);
			sprintf (ccBuffer4,"\"%s\",",defKeyName2011);
			sprintf (ccBuffer5,"\"%s\",",nsrs07DescName);
			sprintf (ccBuffer6,"\"%s\",",nsrs07SrcName);
			sprintf (ccBuffer7,"\"%s\",",nsrs11DescName);
			sprintf (ccBuffer8,"\"%s\",",nsrs11SrcName);

			// Write the output.
			sprintf (ccBuffer,"\t{  %28s  %12s  %5u,",
												ccBuffer1,
												ccBuffer2,
												harnEpsgCode);
			mbstowcs (wcBuffer,ccBuffer,wcCount (wcBuffer));
			oStream << wcBuffer << std::endl;

			sprintf (ccBuffer,"\t\t\t\t\t %28s  %-68s  %-68s  %5u,",
												ccBuffer3,
												ccBuffer5,
												ccBuffer6,
												nsrs07EpsgCode);
			mbstowcs (wcBuffer,ccBuffer,wcCount (wcBuffer));
			oStream << wcBuffer << std::endl;

			sprintf (ccBuffer,"\t\t\t\t\t %28s  %-68s  %-68s  %5u   },",
												ccBuffer4,
												ccBuffer7,
												ccBuffer8,
												nsrs11EpsgCode);
			mbstowcs (wcBuffer,ccBuffer,wcCount (wcBuffer));
			oStream << wcBuffer << std::endl;
		}
	}
	oStream.close ();
	return ok;
}
bool CS_epsgHarnToNsrs (const TcsEpsgDataSetV6* epsgPtr,ulong32_t& epsg2007,ulong32_t& epsg2011,ulong32_t epsgHarn)
{
	bool ok (false);

	TcsEpsgCode epsgCode;

	char ccHarnName [128];
	wchar_t wcSrchName [128];
	std::wstring wsHarnName;

	epsg2007 = 0UL;
	epsg2011 = 0UL;

	// Extract the name of the definition associated with the provided EPSG code.
	ok = epsgPtr->GetFieldByCode (wsHarnName,epsgTblReferenceSystem,epsgFldCoordRefSysName,epsgHarn);
	if (!ok)
	{
		return ok;
	}

	// Manipulate the name so as to get the name of the NSRS 2007 equivalent.
	wcstombs (ccHarnName,wsHarnName.c_str (),sizeof (ccHarnName));
	ok = CS_strrpl (ccHarnName,sizeof (ccHarnName),"NAD83(HARN)","NAD83(NSRS2007)");
	if (ok)
	{
		// Locate a non-deprecated entry with this name and extract the
		// EPSG code value.
		mbstowcs (wcSrchName,ccHarnName,wcCount (wcSrchName));
		const TcsEpsgTable* crsTblPtr = epsgPtr->GetTablePtr (epsgTblReferenceSystem);
		ok = (crsTblPtr != 0);
		if (ok)
		{
			bool lclOk = crsTblPtr->EpsgLocateCode (epsgCode,epsgFldCoordRefSysName,wcSrchName);
			if (lclOk)
			{
				epsg2007 = epsgCode;
			}
		}
	}
	if (ok)
	{
		wcstombs (ccHarnName,wsHarnName.c_str (),sizeof (ccHarnName));
		ok = CS_strrpl (ccHarnName,sizeof (ccHarnName),"NAD83(HARN)","NAD83(2011)");
		if (ok)
		{
			// Locate a non-deprecated entry with this name and extract the
			// EPSG code value.
			mbstowcs (wcSrchName,ccHarnName,wcCount (wcSrchName));
			const TcsEpsgTable* crsTblPtr = epsgPtr->GetTablePtr (epsgTblReferenceSystem);
			ok = (crsTblPtr != 0);
			if (ok)
			{
				bool lclOk = crsTblPtr->EpsgLocateCode (epsgCode,epsgFldCoordRefSysName,wcSrchName);
				if (lclOk)
				{
					epsg2011 = epsgCode;
				}
			}
		}
	}
	return ok;
}
bool CS_epsgNsrs07Name (const TcsEpsgDataSetV6* epsgPtr,char* nsrs07Name,size_t nameSize,ulong32_t& epsg2007)
{
	bool ok (false);

	TcsEpsgCode epsgCode;

	std::wstring wsNsrs07Name;

	// Extract the name of the definition associated with the provided EPSG code.
	ok = epsgPtr->GetFieldByCode (wsNsrs07Name,epsgTblReferenceSystem,epsgFldCoordRefSysName,epsg2007);
	if (ok)
	{
		wcstombs (nsrs07Name,wsNsrs07Name.c_str (),nameSize);
	}
	return ok;
}
// The following function searches the category file looking for categories
// which contain an entry with the oldKeyName.  Upon locatioin of any such
// category, it adds a new entry composed of the provided 'new' key name and
// description.
bool csAddCrsToCategoryFile (TcsCategoryFile& categoryFile,const cs_Add2007And2011_* addTblPtr)
{
	bool ok (true);				// until we know differently

	size_t categoryIdx;
	size_t itemIdx;

	// Create the two category items which we intend to add.
	TcsCategoryItem nsrs07Item (addTblPtr->nsrs07KeyName,addTblPtr->nsrs07Desc);
	TcsCategoryItem nsrs11Item (addTblPtr->nsrs11KeyName,addTblPtr->nsrs11Desc);

	// Start a loop which will search the entire category file for entries with
	// the 'harn' name.  For each such category which has an entry with the
	// 'harn' name, we add both of the new entries.
	for (categoryIdx = 0;ok && categoryIdx < categoryFile.GetCategoryCount ();categoryIdx += 1)
	{
		TcsCategory* categoryPtr = categoryFile.FetchCategory (categoryIdx);

		// Here oncce for each category in the category file.
		bool addToThisCategory = false;
		for (itemIdx = 0;ok && itemIdx < categoryPtr->GetItemCount ();itemIdx += 1)
		{
			// Here once for each item in each category in the file.
			TcsCategoryItem* itemPtr = categoryPtr->GetItem (itemIdx);
			if (!itemPtr->IsToBeDeleted ())
			{
				if (!CS_stricmp (itemPtr->GetItemName (),addTblPtr->harnKeyName))
				{
					addToThisCategory = true;
					break;
				}
			}
		}
		
		// If an entry for the 'harn' name existed in this category, we add the
		// two new entries to this category as well.
		if (ok && addToThisCategory)
		{
			categoryPtr->AddItem (nsrs07Item);
			categoryPtr->AddItem (nsrs11Item);
		}
	}
	return ok;
}
// We need to insert a rather significant number of entries into the NameMapper
// file for each of the systems we are adding.  Using other data sources and
// utilities, the EPSG and ESRI records for all new systems have already been
// added to the NameMapper.  Thus, we only need to add name entries for the
// CS-MAP and Autodesk flavors.  However, the trick will be to properly
// associate the new entries with the corresponding existing entries.
bool csAddToNameMapper (TcsNameMapper& nameMapper,const cs_Add2007And2011_* addTblPtr,bool isGeographic)
{
	bool ok (true);
	bool maFlag (false);
	bool prFlag (false);
	bool wyFlag (false);
	bool nyFlag (false);
	bool ftFlag (false);

	EcsMapObjType objType;

	const char *ccPtr1;
	const char *ccPtr2;
	const wchar_t* wcPtrK;

	TcsGenericId idHarn;
	TcsGenericId esri2007Id;
	TcsGenericId esri2011Id;

	char esriHarnName [256];
	char esri2007Name [256];
	char esri2011Name [256];
	wchar_t wrkBufr [256];
	
	// Job 1: Given the inofrmation provided by the addTblPtr, we need to
	// determine the appropriate ID with which to associate the names
	// associated with the current definitions.
	
	// We have most, ut not all, 2007 EPSG codes.  SInce we don't have all. we
	// can't rely on that.  So, what we will try to do here is:
	//  1> Using the HARN name from the table, extract from the NameMapper the
	//	   ESRI HARN name for the system.
	//	2> Using CS_strrpl, we change the datum portion of the HARN name from
	//     HARN to NSRS2007/_2011_.
	//	3> Use the resulting names to extract from the NameMapper the generic
	//     ID of the appropriate item, and then
	//  4> Insert the appropirate names using the extracted generic ID's
	//     as appropriate.

	// Start with getting the generic ID of the HARN name equivalent for this
	// system:
	objType = isGeographic ? csMapGeographicCSysKeyName : csMapProjectedCSysKeyName;
	mbstowcs (wrkBufr,addTblPtr->harnKeyName,wcCount (wrkBufr));
	idHarn = nameMapper.Locate (objType,csMapFlvrAutodesk,wrkBufr);
	if (idHarn.IsNotKnown ())
	{
		idHarn = nameMapper.Locate (objType,csMapFlvrCsMap,wrkBufr);
	}
	if (idHarn.IsNotKnown ())
	{
		// We expect a few of these.  These are systems in US Foot that are not
		// supported by EPSG and/or ESRI.  We rep[ort this condition so that we
		// can visually verify that there are not others.  But we simple return
		// a true value and keep on trucking.
		std::wcerr << L"No NameMapper entry for HARN System named \""
				   << wrkBufr
				   << L"\"."
				   << std::endl;
		return true;
	}

	if (ok)
	{
		// Use this generic ID to get the ESRI HARN name for this system.
		wcPtrK = nameMapper.LocateName (objType,csMapFlvrEsri,idHarn);
		if (wcPtrK == 0)
		{
			std::wcerr << L"Generic ID "
					   << static_cast<unsigned long>(idHarn)
					   << L" failed to produce a ESRI name."
					   << std::endl;
			ok = false;
		}
		wcstombs (esriHarnName,wcPtrK,sizeof (esriHarnName));

		// Need to do some special stuff for Massachusetts and Puerto Rico.
		ccPtr1 = CS_stristr (esriHarnName,"_Massachusetts_");
		maFlag = (ccPtr1 != 0);		
		ccPtr1 = CS_stristr (esriHarnName,"_Puerto_Rico_");
		prFlag = (ccPtr1 != 0);
		ccPtr1 = CS_stristr (esriHarnName,"_Wyoming_");
		wyFlag = (ccPtr1 != 0);		
		ccPtr1 = CS_stristr (esriHarnName,"_New_York_");
		nyFlag = (ccPtr1 != 0);		
		ccPtr1 = CS_stristr (esriHarnName,"_Feet");
		ccPtr2 = CS_stristr (esriHarnName,"_Feet_Intl");
		ftFlag = (ccPtr1 != 0 || ccPtr2 != 0);

		// Convert the esriHarn name to a esri2007Name and extract the genericID
		// for the item which is supposed to exist.
		CS_stncp (esri2007Name,esriHarnName,sizeof (esri2007Name));
		CS_strrpl (esri2007Name,sizeof (esri2007Name),"NAD_1983_HARN_","NAD_1983_NSRS2007_");
		if (maFlag && ftFlag)
		{
			// Here for Massachussets 
			CS_strrpl (esri2007Name,sizeof (esri2007Name),"_Mainland_","_Mnld_");
			CS_strrpl (esri2007Name,sizeof (esri2007Name),"_Island_","_Isl_");
		}
		if (wyFlag && ftFlag)
		{
			// Here for Massachussets 
			CS_strrpl (esri2007Name,sizeof (esri2007Name),"_East_Central_","_E_Central_");
			CS_strrpl (esri2007Name,sizeof (esri2007Name),"_West_Central_","_W_Central_");
		}
		if (nyFlag && ftFlag)
		{
			// Here for New York
			CS_strrpl (esri2007Name,sizeof (esri2007Name),"_Island_","_Isl_");
		}
		if (prFlag)
		{
			// Here for Peurto Rico and Virgin Islands
			CS_strrpl (esri2007Name,sizeof (esri2007Name),"_Islands_","_Isls_");
		}
		
		// We have a special version of NameMapp.Locate which will try the
		// all the variations used to encode the "foot" unit into the name.
		esri2007Id = csGetGenericIdFoot (nameMapper,objType,esri2007Name);
		ok = esri2007Id.IsKnown ();

		// Kludge Time!  If the 2011 epsgCode value is set to 1, we
		// skip this entry as there is no ESRI equivalent in 2011.
		// Of course, there is no EPSG equivalent for anything in 
		// 2011 as we wriute this kludge.
		if (addTblPtr->nsrs11EpsgCode != 1)
		{
			// Convert the esriHarn name to a esri2011Name and extract the generidID
			// for the item which is supposed to exist.
			CS_stncp (esri2011Name,esriHarnName,sizeof (esri2011Name));
			CS_strrpl (esri2011Name,sizeof (esri2011Name),"NAD_1983_HARN_","NAD_1983_2011_");
			if (maFlag && ftFlag)
			{
				// Here for Massachussets US Survey Foot variations.
				CS_strrpl (esri2011Name,sizeof (esri2011Name),"_Mainland_","_Mnld_");
				CS_strrpl (esri2011Name,sizeof (esri2011Name),"_Island_","_Isl_");
				CS_strrpl (esri2011Name,sizeof (esri2011Name),"_Feet","_FtUS");
			}
			if (wyFlag && ftFlag)
			{
				// Here for Massachussets 
				CS_strrpl (esri2011Name,sizeof (esri2011Name),"_East_Central_","_E_Central_");
				CS_strrpl (esri2011Name,sizeof (esri2011Name),"_West_Central_","_W_Central_");
			}
			if (nyFlag && ftFlag)
			{
				// Here for New York
				CS_strrpl (esri2011Name,sizeof (esri2011Name),"_Island_","_Isl_");
			}
			if (prFlag)
			{
				// Here for Peurto Rico and Virgin Islands
				CS_strrpl (esri2011Name,sizeof (esri2011Name),"_Islands_","_Isls_");
			}
			esri2011Id = csGetGenericIdFoot (nameMapper,objType,esri2011Name);
			ok = esri2011Id.IsKnown ();
		}
	}

	// Add the 2007 CS-MAP and Autodesk entries to the name mapper.
	if (ok && esri2007Id.IsKnown ())
	{
		mbstowcs (wrkBufr,addTblPtr->nsrs07KeyName,wcCount (wrkBufr));
		TcsNameMap newNameMap (esri2007Id,objType,csMapFlvrCsMap,0UL,wrkBufr);
		ok = nameMapper.Add (newNameMap);
	}
	if (ok && esri2007Id.IsKnown ())
	{
		mbstowcs (wrkBufr,addTblPtr->nsrs07KeyName,wcCount (wrkBufr));
		TcsNameMap newNameMap (esri2007Id,objType,csMapFlvrAutodesk,0UL,wrkBufr);
		ok = nameMapper.Add (newNameMap);
	}

	// Add the 2011 CS-MAP and Autodesk entries to the name mapper.
	if (ok && esri2011Id.IsKnown ())
	{
		mbstowcs (wrkBufr,addTblPtr->nsrs11KeyName,wcCount (wrkBufr));
		TcsNameMap newNameMap (esri2011Id,objType,csMapFlvrCsMap,0UL,wrkBufr);
		ok = nameMapper.Add (newNameMap);
	}
	if (ok && esri2011Id.IsKnown ())
	{
		mbstowcs (wrkBufr,addTblPtr->nsrs11KeyName,wcCount (wrkBufr));
		TcsNameMap newNameMap (esri2011Id,objType,csMapFlvrAutodesk,0UL,wrkBufr);
		ok = nameMapper.Add (newNameMap);
	}
	return ok;
}
TcsGenericId csGetGenericIdFoot (TcsNameMapper& nameMapper,EcsMapObjType objType,const char* crsName)
{
	bool usFtFlag (false);
	bool intlFtFlag (false);

	TcsGenericId genericId;
	char nameBufr [256];
	wchar_t wrkBufr [256];

	// A special version of the NameMapper Locate function which is smart enough to deal
	// with the changes in ESRI names with regard to how feet (US and INternational) are
	// encoded in the name.  The representation in the HARN names is pretty standard.
	// We determine if the name indicates one of the two foor units before we get
	// started.
	CS_stncp (nameBufr,crsName,sizeof (nameBufr));
	usFtFlag = (CS_stristr (nameBufr,"_Feet") != 0);
	intlFtFlag = (CS_stristr (nameBufr,"Feet_Intl") != 0);
	
	// OK, we try the normal case where no special kludges are arequried.
	mbstowcs (wrkBufr,nameBufr,wcCount (wrkBufr));
	genericId = nameMapper.Locate (objType,csMapFlvrEsri,wrkBufr);
	if (genericId.IsNotKnown () && usFtFlag)
	{
		// This definition name contains a reference to US Survey Foot.
		// We try the alternatives to this in search of success.
		CS_stncp (nameBufr,crsName,sizeof (nameBufr));
		CS_strrpl (nameBufr,sizeof (nameBufr),"_Feet","_Ft_US");
		mbstowcs (wrkBufr,nameBufr,wcCount (wrkBufr));
		genericId = nameMapper.Locate (objType,csMapFlvrEsri,wrkBufr);
		if (genericId.IsNotKnown ())
		{
			CS_stncp (nameBufr,crsName,sizeof (nameBufr));
			CS_strrpl (nameBufr,sizeof (nameBufr),"_Feet","_FtUS");
			mbstowcs (wrkBufr,nameBufr,wcCount (wrkBufr));
			genericId = nameMapper.Locate (objType,csMapFlvrEsri,wrkBufr);
		}
	}
	if (genericId.IsNotKnown () && intlFtFlag)
	{
		// This definition name contains a reference to US Survey Foot.
		// We try the alternatives to this in search of success.
		CS_stncp (nameBufr,crsName,sizeof (nameBufr));
		CS_strrpl (nameBufr,sizeof (nameBufr),"_Feet_Intl","_Ft_Intl");
		mbstowcs (wrkBufr,nameBufr,wcCount (wrkBufr));
		genericId = nameMapper.Locate (objType,csMapFlvrEsri,wrkBufr);
		if (genericId.IsNotKnown ())
		{
			CS_stncp (nameBufr,crsName,sizeof (nameBufr));
			CS_strrpl (nameBufr,sizeof (nameBufr),"_Feet_Intl","_FtI");
			mbstowcs (wrkBufr,nameBufr,wcCount (wrkBufr));
			genericId = nameMapper.Locate (objType,csMapFlvrEsri,wrkBufr);
		}
	}
	if (genericId.IsNotKnown ())
	{
		mbstowcs (wrkBufr,crsName,wcCount (wrkBufr));
		std::wcerr << L"ESRI Name \""
				   << wrkBufr
				   << L"\" failed to produce a generic ID."
				   << std::endl;
	}
	return genericId;
}
bool csAddNameMap (TcsNameMapper& nameMapper,bool isGeographic,unsigned long epsgCode,EcsNameFlavor flavor,const char* crsName)
{
	bool ok (false);
	wchar_t keyName [256];
	EcsMapObjType objectType;

	objectType = (isGeographic) ? csMapGeographicCSysKeyName : csMapProjectedCSysKeyName;
	mbstowcs (keyName,crsName,wcCount (keyName));
	TcsGenericId genericId (epsgCode);
	TcsNameMap nameMap (genericId,objectType,flavor,0UL,keyName);
	ok = nameMapper.Add (nameMap,true);
	return ok;
}
bool csWriteEsriWktTestFile (const wchar_t* csDataSrcDir,const wchar_t* csDataTrgDir)
{
	bool ok (false);

	char *chPtr;
	char *envKeyPtr;
	char *envNamPtr;
	char *envValPtr;
	const char *chPtrK;

	std::ifstream iStream;
	std::ofstream oStream;
	std::ifstream prjStream;

	char prjListFilePath [MAXPATH];			//we use char here, seems Linux prefers it
	char esriPrjTestFilePath [MAXPATH];		//we use char here, seems Linux prefers it
	char prjFilePath [MAXPATH + MAXPATH];
	char prjFileBuffer [1024];


	envKeyPtr = "%OPEN_SOURCE%";
	envNamPtr = "OPEN_SOURCE";
	envValPtr = getenv (envNamPtr);
	// Open up the file which lists each individual ESRI .prj file.
	// This is an ordinary text file with one file path specification
	// per line of text.  This was produced by executing the command
	//		"DIR /B /S >EsriPrjFileList.txt" 
	// in the Coordinate system directory produced be unzipping the
	// target file.  See the README.txt file in the root folder of
	// all these .prj files.  NOTE: the above command includes the parent
	// folder of all .prj files listed, which have been manually editted out.
	// The code writen below would ignoire these lines if they were
	// left in the file.
	wcstombs (prjListFilePath,csDataSrcDir,sizeof (prjListFilePath));
	if (envValPtr != 0)
	{
		CS_strrpl (prjListFilePath,sizeof (prjListFilePath),envKeyPtr,envValPtr);
	}
	CS_stncat (prjListFilePath,"\\EsriPrjFileList.txt",sizeof (prjListFilePath));
	iStream.open (prjListFilePath,std::ios_base::in);
	ok = iStream.is_open ();

	// Prepare the output file.
	if (ok)
	{
		wcstombs (esriPrjTestFilePath,csDataTrgDir,sizeof (esriPrjTestFilePath));
		if (envValPtr != 0)
		{
			CS_strrpl (esriPrjTestFilePath,sizeof (esriPrjTestFilePath),envKeyPtr,envValPtr);
		}
		CS_stncat (esriPrjTestFilePath,"\\EsriWktTest.txt",sizeof (esriPrjTestFilePath));
		oStream.open (esriPrjTestFilePath,std::ios_base::out | std::ios_base::trunc);
		ok = oStream.is_open ();
	}
	
	// Loop until we see the end-of-file on the input file.
	while (ok && !iStream.eof())
	{
		iStream.getline (prjFilePath,sizeof (prjFilePath));
		if (iStream.good ())
		{
			chPtr = prjFilePath + strlen (prjFilePath) - 4;
			if (CS_stricmp (chPtr,".prj"))
			{
				// Must be a folder path, perhaps a comment.
				continue;
			}

			chPtrK = CS_stristr (prjFilePath,"Solar System");
			if (chPtrK != 0)
			{
				// We're not interested in Juptier or its moons.
				continue;
			}

			// Here once for each interesting entry in the list file.
			// We open the file, read on line of content, copy that line of
			// contentn tot eh output file, close the file we opened, and
			// proceed to the next file.
			prjStream.open (prjFilePath,std::ios_base::in);
			if (prjStream.is_open ())
			{
				prjStream.getline (prjFileBuffer,sizeof (prjFileBuffer));
				if (prjStream.good ())
				{
					// Skip the VERTCS stuff.
					if (!CS_strnicmp (prjFileBuffer,"VERTCS",6))
					{
						// We just do nothing.
						ok = true;
					}
					else
					{
						bool isEpsg;
						EcsNameFlavor wktFlvrEsri = csMapFlvrEsri;
						unsigned long esriNbr;
						unsigned long epsgNbr;
						char wrkBuffer [256];
						char* dummy;

						isEpsg = false;		// until we know differently
						esriNbr = 0UL;		// until we know differently
						epsgNbr = 0UL;		// until we know differently

						// We're going to write this line.  Before we do,
						// we extract the Authority element on the end, and
						// extract the type of number (EPSG/ESRI) and the
						// number itself.
						chPtrK = CS_stristr (prjFileBuffer,"AUTHORITY");
						if (chPtrK != 0)
						{
							// We should always find an AUTHORITY element.
							CS_stncp (wrkBuffer,chPtrK,sizeof (wrkBuffer));
							chPtrK = CS_stristr (wrkBuffer,"EPSG");
							isEpsg = (chPtrK != 0);
							chPtrK = strrchr (wrkBuffer,',');
							esriNbr = strtoul ((chPtrK+1),&dummy,10);
							if (isEpsg)
							{
								epsgNbr = esriNbr;
							}
						}
						oStream << static_cast<int>(wktFlvrEsri) << '|'
								<< epsgNbr						 << '|'
								<< esriNbr						 << '|'
								<< prjFileBuffer				 << std::endl;
					}
				}
				else
				{
					ok = false;
				}
				prjStream.close ();
			}
			else
			{
				ok = false;
			}
		}
	}
	iStream.close ();
	oStream.close ();
	return ok;
}
// This module writes in 'C' table syntax a listing all non-deprecated HARN
// based CRS systems including:
//		1> CS-MAP CRS Name,
//		2> Current Datum reference
//		3> Current EPSG code
//		4> Proposed name for the new 2007 version.
//		5> Proposed name for the new 2011 version.
bool csWriteNsrsAudit (const wchar_t* csDictSrcDir,const wchar_t* csTrgDataFileDir)
{
	bool ok (false);

	unsigned long harnEpsgCode (0UL);
	unsigned long nsrs07EpsgCode (0UL);
	unsigned long nsrs11EpsgCode (0UL);

	char *dummy;
	const char *chPtr;
	TcsDefLine* defLinePtr;
	TcsAscDefinition* ascDefPtr;

	EcsDictType dictType;

	char defKeyName [MAXPATH];
	char datumKeyName [MAXPATH];
	char csSrcAscFilePath [MAXPATH];
	char csTrgDataFilePath [MAXPATH];
	char nsrs07Name [MAXPATH];

	wchar_t wcBuffer [MAXPATH + MAXPATH];

	std::wofstream oStream;

	const TcsEpsgDataSetV6* epsgPtr;
	
	epsgPtr = GetEpsgObjectPtr ();
	ok = (epsgPtr != 0);

	// Open up the Coordinate System ASCII definition file.
	wcstombs (csSrcAscFilePath,csDictSrcDir,sizeof (csSrcAscFilePath));
	CS_stncat (csSrcAscFilePath,"\\coordsys.asc",sizeof (csSrcAscFilePath));
	TcsDefFile coordsysAsc (dictTypCoordsys,csSrcAscFilePath);

	// If the above fails for any reason, the Dictionary Type will not be
	// valid.
	dictType = coordsysAsc.GetDictType ();
	if (dictType != dictTypCoordsys)
	{
		ok = false;
	}

	// Prepare the output file.  Note, the resulting file wioll require some
	// manual tweaking.  This is fully anticipated.
	wcstombs (csTrgDataFilePath,csTrgDataFileDir,sizeof (csTrgDataFilePath));
	CS_stncat (csTrgDataFilePath,"\\Nsrs07Audit.csv",sizeof (csTrgDataFilePath));
	oStream.open (csTrgDataFilePath,std::ios_base::out | std::ios_base::trunc);
	if (!oStream.is_open ())
	{
		ok = false;
	}
	
	if (!ok)
	{
		return ok;
	}

	ok = true;
	size_t defCount = coordsysAsc.GetDefinitionCount ();
	for (unsigned index = 0;ok && index < defCount;index += 1)
	{
		// Here once for each definition in the file.
		ascDefPtr = &(*coordsysAsc [index]);
		if (ascDefPtr == 0)
		{
			ok = false;
			break;
		}

		// Get the definition name.
		chPtr = ascDefPtr->GetDefinitionName ();
		if (chPtr == 0)
		{
			// Should only be the last enry in the file.  Maybe this is bug in
			// the TcsDefFile object.
			continue;
		}
		CS_stncp (defKeyName,chPtr,sizeof (defKeyName));

		// Extract the Datum code. If it is something other than "HARN", we
		// don't need to do anything which this entry.
		defLinePtr = ascDefPtr->GetLine ("DT_NAME:");
		if (defLinePtr == 0)
		{
			continue;
		}
		chPtr = defLinePtr->GetValue ();
		if (chPtr != 0)
		{
			CS_stncp (datumKeyName,chPtr,sizeof (datumKeyName));
			if (CS_strnicmp (datumKeyName,"HARN/",5))
			{
				// On to the next definition if this definition is not 
				continue;
			}
			if (!CS_stricmp (datumKeyName,"HARN/HI"))
			{
				// 2007 and/or 2011 not defined for Hawaii
				continue;
			}
		}

		// Get the current EPSG code value if one is there.
		harnEpsgCode = 0UL;
		defLinePtr = ascDefPtr->GetLine ("EPSG:");
		if (defLinePtr != 0)
		{
			// There is an EPSG label, see if the value is correct.
			chPtr = defLinePtr->GetValue ();
			if (chPtr != 0)
			{
				harnEpsgCode = strtoul (chPtr,&dummy,10);
			}
		}

		// Get the EPSG codes for the NSRS 2007 definitions.  EPSG has not
		// assigned code values for NSRS 2011 yet.
		nsrs07EpsgCode = 0UL;
		if (harnEpsgCode != 0UL)
		{
			ok = CS_epsgHarnToNsrs (epsgPtr,nsrs07EpsgCode,nsrs11EpsgCode,harnEpsgCode);
			if (nsrs07EpsgCode != 0)
			{
				// Get the EPSG NSRS 2007 Name.
				ok = CS_epsgNsrs07Name (epsgPtr,nsrs07Name,sizeof (nsrs07Name),nsrs07EpsgCode);
			}
		}

		if (ok)
		{
			swprintf (wcBuffer,wcCount (wcBuffer),L"%d,\"%S\",\"%S\"",nsrs07EpsgCode,nsrs07Name,defKeyName);
			oStream << wcBuffer << std::endl;
		}
	}
	oStream.close ();
	return ok;
}
//
// The following uses the current EPSG dataset to populate the
// nsrs11EpsgCode element of the cs_Add2007And2011 table.  No
// attempt is made to preserve these values.  Such will not be
// worth much once we complete populating the Coordinate 
// System Dictionary and the NameMapper.  Note this function
// does not use the NameMapper, only the EPSG database.
bool csGetNsrs2011EpsgCodes (const TcsEpsgDataSetV6* epsgPtr)
{
	bool ok;

	TcsEpsgCode epsgCode;
	TcsEpsgCode epsgCode2007;

	struct cs_Add2007And2011_* addTblPtr;

	char ccName [128];
	wchar_t wc2011Name [128];
	std::wstring ws2007Name;
	std::wstring wsEpsgName;

	ok = true;							// Until we know differently.
	
	// Loop once for each entry in the table.
	for (addTblPtr = cs_Add2007And2011;ok && addTblPtr->harnKeyName[0] != '\0';addTblPtr += 1)
	{
		// If the 2011 EPSG code has been hard coded (as we did is a few very strange,
		// cases) we have nothing to do here.  We need to do this for a few codes where
		// there exists an ESRI 2011 system, but no corresponding 2007 System.  Looking
		// up two codes and hard coding them was not a big deal.
		if (addTblPtr->nsrs11EpsgCode >= 3UL && addTblPtr->nsrs11EpsgCode <= 32767UL)
		{
			// Already know the code, just need to get the EPSG name.
			epsgCode = addTblPtr->nsrs11EpsgCode;
			const TcsEpsgTable* crsTblPtr = epsgPtr->GetTablePtr (epsgTblReferenceSystem);
			ok = (crsTblPtr != 0);
			if (ok)
			{
				ok = epsgPtr->GetFieldByCode (wsEpsgName,epsgTblReferenceSystem,epsgFldCoordRefSysName,epsgCode);
				if (ok)
				{
					wcsncpy (addTblPtr->nsrs11EpsgName,wsEpsgName.c_str (),wcCount (addTblPtr->nsrs11EpsgName));
				}
			}
			continue;
		}

		// We use the otherwise invalie EPSG code value of 1 to indicate a system
		// for which we know there is no EPSG code.  There are couple of them.
		// We skip these so that they do not cause an error report.  We want to
		// get through this without any reported errors to boost our confidence.
		if (addTblPtr->nsrs11EpsgCode >= 1UL)
		{
			// A 2011 EPSG code is known not to exist, so no sense to trying to
			// determine it.
			continue;
		}

		// Extract the existing 2007 EPSG Code from the table.
		epsgCode2007 = addTblPtr->nsrs07EpsgCode;
		if (epsgCode2007.IsNotValid ())
		{
			continue;
		}

		// Extract the EPSG name of the definition associated with the 2007 EPSG code.
		ok = epsgPtr->GetFieldByCode (ws2007Name,epsgTblReferenceSystem,epsgFldCoordRefSysName,epsgCode2007);
		if (!ok)
		{
			return ok;
		}

		// Manipulate the name so as to get the name of the NSRS 2011 equivalent.
		wcstombs (ccName,ws2007Name.c_str (),sizeof (ccName));
		ok = CS_strrpl (ccName,sizeof (ccName),"NAD83(NSRS2007)","NAD83(2011)");
		if (ok)
		{
			// Locate a non-deprecated EPSG entry with this name and extract
			// the EPSG code value.
			mbstowcs (wc2011Name,ccName,wcCount (wc2011Name));
			const TcsEpsgTable* crsTblPtr = epsgPtr->GetTablePtr (epsgTblReferenceSystem);
			ok = (crsTblPtr != 0);
			if (ok)
			{
				bool lclOk = crsTblPtr->EpsgLocateCode (epsgCode,epsgFldCoordRefSysName,wc2011Name);
				if (lclOk)
				{
					addTblPtr->nsrs11EpsgCode = epsgCode;
					wcsncpy (addTblPtr->nsrs11EpsgName,wc2011Name,wcCount (addTblPtr->nsrs11EpsgName));
				}
				else
				{
					std::wcout << L"No 2011 EPSG entry for 2007 EPSG entry named "
							   << ws2007Name
							   << L", Code:"
							   << addTblPtr->nsrs07EpsgCode
							   << L"."
							   << std::endl;
				}
			}
		}
	}
	return ok;
}
//
// The following uses the current NameMapper to populate the newly added
// nsrs11EsriCode element of the cs_Add2007And2011 table.  We'll need this
// code as it is our ticket to finding existing NameMapper entries which
// need to be deprecated.  No  attempt is made to preserve these values.
// Such will not be worth much once we complete populating the Coordinate
// System Dictionary and the NameMapper.
//
// Note that this module uses the normal NameMapper support functions which
// implies that the information is coming from a constant in memory copy
/// of the NameMapper produced by the implied cmGetNameMapperPtr function
// call in these modules.  To remove any possible contention, we release
// the internal copy of the name mapper.  Performance wise, rather painful.
// But after the final successful run, this code will never be used again.
//
bool csGetNsrs2011EsriCodes (void)
{
	bool ok;

	unsigned long esri2011Code;
	TcsEpsgCode epsgCode2007;

	const wchar_t* esri2007NmPtr;
	struct cs_Add2007And2011_* addTblPtr;

	char ccName [256];
	wchar_t esri2011Name [256];

	ok = true;							// Until we know differently.

	// Loop once for each entry in the table.
	for (addTblPtr = cs_Add2007And2011;ok && addTblPtr->harnKeyName[0] != '\0';addTblPtr += 1)
	{
		// Using the existing 2007 EPSG Code in the table; extract the ESRI 2007 
		// name from the NameMapper.  These are all known to exist.  If not,
		// 'ok' will go false and we'll know right away.
		epsgCode2007 = addTblPtr->nsrs07EpsgCode;
		if (epsgCode2007.IsNotValid ())
		{
			continue;
		}

		// There are two systems for which we know that a ESRI 2007 definition
		// does not exist.  We skip these:
		//		EPSG::5655  --> NSRS07.VTF
		
		esri2007NmPtr = csMapIdToName (csMapProjGeoCSys,csMapFlvrEsri,csMapFlvrEpsg,epsgCode2007);
		ok = (esri2007NmPtr != 0);
		if (!ok)
		{
			std::wcout << L"No ESRI NameMapper entry for EPSG code "
					   << static_cast<unsigned long>(epsgCode2007)
					   << L"."
					   << std::endl;
			ok = true;
			continue;
		}

		// Convert the ESRI name to the 2011 form.
		if (ok)
		{
			wcstombs (ccName,esri2007NmPtr,sizeof (ccName));
			ok = CS_strrpl (ccName,sizeof (ccName),"NAD_1983_NSRS2007","NAD_1983_2011");
		}
	
		if (ok)
		{
			bool lclOk;

			mbstowcs (esri2011Name,ccName,wcCount (esri2011Name));

			// Extract the 2011 ESRI code (assigned by ESRI prior to EPSG's
			// assignment of code values.
			esri2011Code = csMapNameToId (csMapProjGeoCSys,csMapFlvrEsri,csMapFlvrEsri,esri2011Name);
			lclOk = (esri2011Code != KcsNmInvNumber) && (esri2011Code != KcsNmMapNoNumber);
			if (lclOk)
			{
				addTblPtr->nsrs11EsriCode = esri2011Code;
			}
			else
			{
				std::wcout << L"No NameMapper entry for ESRI (2011) name '"
						   << esri2011Name
						   << L"."
						   << std::endl;
			}
		}
	}

	// Release the NameMapper copy we used so as not to interfere or
	// obfuscgate the editting operation which follows this function
	// call.

	cmGetNameMapperPtr (true);
	return ok;
}
//
// The following function uses the dynamically added information now in the
// cs_Add2007And2011 table to update the Coordinate System Dictionary and the
// Name Mapper.
bool csFixNsrs2011 (const wchar_t* csDictDir,const wchar_t* csTempDir)
{
	bool ok;

	wchar_t* wcPtr;
	const wchar_t* wcNmPtr;
	TcsNameMapper* nmMapPtr;
	struct cs_Add2007And2011_* addTblPtr;
	const TcsEpsgDataSetV6* epsgPtr;

	char ccWork [256];
	char crsAscFilePath [260];

	wchar_t nmMapFilePath [260];

	EcsCsvStatus csvStatus;

	TcsNameMap existingEsriNameMap;
	TcsNameMap newEsriNameMap;
	TcsNameMap newEpsgNameMap;

	TcsNameMapper nameMapper;

	// BUild a path to the NameMapper we will be modifying.
	wcsncpy (nmMapFilePath,csDictDir,wcCount (nmMapFilePath));
	wcPtr = nmMapFilePath + wcslen (nmMapFilePath) - 1;
	if (*wcPtr != L'\\' && *wcPtr != L'/')
	{
		++wcPtr;
		*wcPtr++ = L'\\';
	}
	wcscpy (wcPtr,L"NameMapper.csv");

	// Read it in, so we have direct access to the NameMapper itself.
	// We activate the Duplicate feature, as when we started this
	// project, there were a buch of duplicates which needed to be
	// dealt with such that a "diff" aftetr this operation would
	// provide results which could be properly evaluated for success.
	nameMapper.SetRecordDuplicates (true);
	std::wifstream iStrm (nmMapFilePath,std::ios_base::in);
	ok = iStrm.is_open ();
	if (ok)
	{
		csvStatus = nameMapper.ReadFromStream (iStrm);
		ok = (csvStatus == csvOk);
	}
	nmMapPtr = ok ? &nameMapper : 0;

	// Make sure we have a valid dictionary directory which CS-MAP can
	// access.  We'll be updating the "official" NameMapper copy which
	// essentially resides there.
	if (ok)
	{
		wcstombs (ccWork,csDictDir,sizeof (ccWork));
		int status = CS_altdr (ccWork);
		ok = (status == 0);
	}

	// We'll need accesss to the EPSG database.
	epsgPtr = 0;				// Keep lint happy
	if (ok)
	{
		epsgPtr = GetEpsgObjectPtr ();
		ok = (epsgPtr != 0);
	}

	if (ok)
	{
		// Populate the "cs_Add2007And2011" table with the 2011 EPSG codes
		// extracted from the latest EPSG dataset (8.05 as I write).
		ok = csGetNsrs2011EpsgCodes (epsgPtr);
	}
	if (ok)
	{
		// ESRI assigned it own codes to the 2011 definitions as EPSG had
		// not assigned codes to them.  We will need to know these code
		// values in order to properly update the NameMapper.  These codes
		// are known to be in the NameMapper.
		ok = csGetNsrs2011EsriCodes ();
	}
	if (!ok)
	{
		return ok;
	}

	// We need to add the new EPSG code for the 2011 systems to the Coordinate
	// System Dictionary. We need access to this object once created, so we
	// can't do this within an if(ok)code block 
	wcstombs (crsAscFilePath,csDictDir,sizeof (crsAscFilePath));
	CS_stncat (crsAscFilePath,"\\coordsys.asc",sizeof (crsAscFilePath));
	TcsDefFile coordsysAsc (dictTypCoordsys,crsAscFilePath);
	ok = (coordsysAsc.GetDictType () == dictTypCoordsys);  // Valid dictionary type indicates success

	for (addTblPtr = cs_Add2007And2011;ok && addTblPtr->harnKeyName[0] != '\0';addTblPtr += 1)
	{
		TcsAscDefinition * ascDefPtr;

		TcsEpsgCode epsgCode2007 (addTblPtr->nsrs07EpsgCode);
		if (epsgCode2007.IsNotValid ())
		{
			
			continue;
		}

		// Set the NSRS 2011 EPSG code into the coordinate system definition.
		ascDefPtr = coordsysAsc.GetDefinition (addTblPtr->nsrs11KeyName);
		ok = (ascDefPtr != NULL);
		if (ok)
		{
			bool lclOk;

			sprintf (ccWork,"%ld",addTblPtr->nsrs11EpsgCode);
			lclOk = ascDefPtr->SetValue ("EPSG:",ccWork);
			if (!lclOk)
			{
				// Need to add a new line.
				TcsDefLine newEpsgLine (dictTypCoordsys,"EPSG:",ccWork,0);
				ok = ascDefPtr->InsertAfter ("SOURCE:",newEpsgLine);
			}
		}
		if (!ok)
		{
			continue;
		}

		// While we're here, lets add the Oracle SRID number for the 2007
		// version of this system.  THese are the same as the EPSG codes.
		// Oracle now appears to support the 2007 set of systems, they
		// didn't back when this was originally done.  At the time of this
		// writing (Nov 8, 2014) there is no indication of Oracle support for
		// the 2011 set of systems.  Note: at this time, the Oracle 2007
		// NameMapper entries were already updated.
		ascDefPtr = coordsysAsc.GetDefinition (addTblPtr->nsrs07KeyName);
		ok = (ascDefPtr != NULL);
		if (ok)
		{
			bool lclOk;

			sprintf (ccWork,"%ld",addTblPtr->nsrs07EpsgCode);
			lclOk = ascDefPtr->SetValue ("SRID:",ccWork);
			if (!lclOk)
			{
				bool addLineOk;

				// Need to add a new line.
				TcsDefLine newSridLine (dictTypCoordsys,"SRID:",ccWork,0);
				addLineOk = ascDefPtr->InsertAfter ("EPSG:",newSridLine);
				if (!addLineOk)
				{
					ok = ascDefPtr->InsertAfter ("SOURCE:",newSridLine);
				}
			}
		}
		if (!ok)
		{
			continue;
		}

		// Get the ESRI name of the 2011 system of the current driving
		// table entry.  If the nsrs11EsriCode value is zero, that indicates
		// that there is no ESRI 2011 system for this table entry.
		if (addTblPtr->nsrs11EsriCode == 0UL)
		{
			// The names of the half dozen of these systems were priinted
			// out above, no need to repeat here.
			continue;
		}
		wcNmPtr = 0;		// keep lint  happy
		
		if (ok)
		{
			// Note that this will fetch the name from a distinct copy of
			// the name mapper other than that which we are editting.
			wcNmPtr = csMapIdToName (csMapProjGeoCSys,csMapFlvrEsri,csMapFlvrEsri,addTblPtr->nsrs11EsriCode);
			ok = (wcNmPtr != 0);
		}

		if (ok)
		{
			bool lclOk;

			// Extract and remove the current 2011 ESRI entry for this system.
			// In this case, we cannot use the csMapProjGeoCSys as a type as
			// the locate function used in the actual std::set<> code does not
			// have the special magic necessary to make the switch between the
			// two possibilites.  Therefore, we do the search using both types,
			// giving "projected" the priority, and whatever type is located will
			// be conveyed to the fix it up code below in the extracted object.
			lclOk = nmMapPtr->ExtractAndRemove (existingEsriNameMap,csMapProjectedCSysKeyName,
																	csMapFlvrEsri,
																	wcNmPtr,
																	0,
																	0);
			if (!lclOk)
			{
				lclOk = nmMapPtr->ExtractAndRemove (existingEsriNameMap,csMapGeographicCSysKeyName,
																		csMapFlvrEsri,
																		wcNmPtr,
																		0,
																		0);
			}
			if (!lclOk)
			{
				lclOk = nmMapPtr->ExtractAndRemove (existingEsriNameMap,csMapGeographic3DKeyName,
																		csMapFlvrEsri,
																		wcNmPtr,
																		0,
																		0);
			}
			if (!lclOk)
			{
				std::wcout << L"ESRI 2011 name not found in NameMapper: "
						   << wcNmPtr
						   << std::endl;
				ok = false;
			}
		}

		if (ok)
		{
			// Copy the existing entry, such copy to become the new primary
			// 2011 ESRI entry for this system.  This provides us with the
			// two new entries we need with the appropriate GenericId
			// value to retain the proper association of the records.
			TcsGenericId esri2011GenericId = existingEsriNameMap.GetGenericId ();
			newEsriNameMap = existingEsriNameMap;
			newEpsgNameMap = existingEsriNameMap;

			// Deprecate the old entry.  The name does not change; but there
			// will be two records with the same name, so we need to set a
			// value for dup_sort.
			existingEsriNameMap.SetDeprecated (esri2011GenericId);
			existingEsriNameMap.SetDupSort (1);		// Name, flavor, and type are the same, this distinguishes between the two records.
			existingEsriNameMap.SetComments (L"ESRI used this ID before EPSG assigned an ID.");

			// The newEsriNameMap will become the active ESRI reference.
			// The flavor, name, and other stuff should remain unchanged.
			newEsriNameMap.SetNumericId (addTblPtr->nsrs11EpsgCode);
			newEsriNameMap.SetDupSort (0);			// Defensive
			newEsriNameMap.SetAliasFlag (0);		// Defensive
			newEsriNameMap.SetDeprecated (0UL);		// Defensive

			// Since we now have an EPSG number, we need to add an EPSG
			// record to this sequence.  Again, must not screw around
			// with the generic ID.  Type should remain unchanged.
			newEpsgNameMap.SetFlavor (csMapFlvrEpsg);
			newEpsgNameMap.SetNumericId (addTblPtr->nsrs11EpsgCode);
			newEpsgNameMap.SetNameId (addTblPtr->nsrs11EpsgName);
			newEpsgNameMap.SetDupSort (0);			// Defensive
			newEpsgNameMap.SetAliasFlag (0);		// Defensive
			newEpsgNameMap.SetDeprecated (0UL);		// Defensive
			newEpsgNameMap.SetRemarks (L"");
			newEpsgNameMap.SetComments (L"");

			// Put them both back into the NameMapper.
			ok = nmMapPtr->Add (newEpsgNameMap,false,0);
			if (ok)
			{
				ok = nmMapPtr->Add (newEsriNameMap,false,0);
				if (ok)
				{
					ok = nmMapPtr->Add (existingEsriNameMap,true,0);
				}
			}
		}
		if (!ok)
		{
			ok = true;
		}
	}

	// Write the modified dictionary and NameMapper out, if appropriate.
	if (ok)
	{
		wcstombs (crsAscFilePath,csTempDir,sizeof (crsAscFilePath));
		CS_stncat (crsAscFilePath,"\\ModifiedCoordsys.asc",sizeof (crsAscFilePath));
		ok = coordsysAsc.WriteToFile (crsAscFilePath);
	}
	if (ok)
	{
		wcsncpy (nmMapFilePath,csTempDir,wcCount (nmMapFilePath));
		wcPtr = nmMapFilePath + wcslen (nmMapFilePath) - 1;
		if (*wcPtr != L'\\' && *wcPtr != L'/')
		{
			++wcPtr;
			*wcPtr++ = L'\\';
		}
		wcscpy (wcPtr,L"ModifiedNameMapper.csv");	

		std::wofstream oStrm (nmMapFilePath,std::ios_base::out | std::ios_base::trunc);
		if (oStrm.is_open ())
		{
			nmMapPtr->WriteAsCsv (oStrm,true);
			oStrm.close ();

			wcscpy (wcPtr,L"ModifiedNameMapperDuplicates.csv");
			oStrm.open (nmMapFilePath,std::ios_base::out | std::ios_base::trunc);
			if (oStrm.is_open ())
			{
				nmMapPtr->WriteDuplicates (oStrm);
				oStrm.close ();
			}
		}
	}
	return ok;
}
