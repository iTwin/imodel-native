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

bool GeocentricFixDatum (const char* csDictSrcDir,const char* csDictTrgDir);
bool GeocentricFixCoordsys (const char* csDictSrcDir,const char* csDictTrgDir);
bool GeocentricFixCategory (const char* csDictSrcDir,const char* csDictTrgDir);
bool GeocentricFixNameMapper (const char* csDictSrcDir,const char* csDictTrgDir);

// These two tables define the renaming.  The changes are pretty standard and
// are hard coded into the module below.
struct csGcDtRename_
{
	char oldName [24];
	unsigned long epsgNbr;
	char newName  [24];
	char newNameCS [12];
};

struct csGcDtRename_ csGcDtRename [] =
{
/*                                EPSG                             Short
	   Old Name                   Nbr    New Name                  Name                        EPSG Name from mapping table */
	{ "Antarctic98",              6176,  "Antarctic1998",          "*"          },          // Australian Antarctic Datum 1998
	{ "Antilles91",               6640,  "Antilles1991",           "RRAF91a"    },          // Reseau de Reference des Antilles Francaises 1991
	{ "Caledonie91",              6645,  "Caledonie1991",          "*"          },  /*d*/   // Reseau Geodesique Nouvelle Caledonie 1991
	{ "CHTRF95-MOD",              6151,  "CHTRF1995-MOD",          "*"          },          // Swiss Terrestrial Reference Frame 1995
	{ "Estonia97",                6180,  "Estonia1997",            "*"          },          // Estonia 1997
	{ "ETRF89",                   6258,  "ETRF1989",               "*"          },          // European Terrestrial Reference System 1989
	{ "ETRS89/01",                   0,  "ETRS1989/01",            "*"          },
	{ "GR96",                     6747,  "GR1996",                 "*"          },          // Greenland 1996
	{ "Greek-GRS87",              6121,  "Greek-GRS1987",          "*"          },          // Greek Geodetic Reference System 1987
	{ "GRSSA",                    6170,  "GRSSA1995",              "*"          },          // Sistema de Referencia Geocentrico para America del Sur 1995
	{ "Guyane95",                 6624,  "Guyane1995",             "*"          },          // Reseau Geodesique Francais Guyane 1995
	{ "Hartebeesthoek94",         6148,  "Hartebeesthoek-94",      "*"          },          // Hartebeesthoek94
	{ "IRENET95",                 6173,  "IRENET-1995",            "*"          },          // IRENET95
	{ "Israel",                   6141,  "Israel_1",               "*"          },          // Israel
	{ "Korean2000",               6737,  "Korean2000_1",           "*"          },          // Geocentric datum of Korea
	{ "Korean95",                 6166,  "Korean1995",             "*"          },          // Korean Datum 1995
	{ "KuwaitUtility",            6319,  "KuwaitUtility_1",        "*"          },          // Kuwait Utility
	{ "Latvia1992",               6661,  "Latvia92",               "*"          },          // Latvia 1992
	{ "Lithuania94",              6126,  "Lithuania1994",          "*"          },          // Lithuania 1994 (ETRS89)
	{ "MAGNA",                    6686,  "MGNdR",                  "*"          },          // Marco Geocentrico Nacional de Referencia
	{ "Mauritania1999",           6702,  "Mauritania99",           "*"          },          // Mauritania 1999
	{ "Nakhl-eGhanem",            6693,  "Nakhl-Ghanem",           "*"          },          // Nakhl-e Ghanem
	{ "NtlGeodeticNet",           6318,  "NationalGeodeticNet",    "NGNa"       },          // National Geodetic Network
	{ "PGA98",                    6190,  "PGA1998",                "*"          },          // Posiciones Geodesicas Argentinas 1998
	{ "PosGAr94",                 6694,  "PosGAr1994",             "*"          },          // Posiciones Geodesicas Argentinas 1994
	{ "PosGAr98",                 6190,  "PosGAr1998",             "*"          },          // Posiciones Geodesicas Argentinas 1998
	{ "PosGAr-MOD",               6172,  "PosGAr94-MOD",           "*"          },  /*d*/   // Posiciones Geodesicas Argentinas
	{ "REGVEN",                   6189,  "REGVEN_1",               "*"          },          // Red Geodesica Venezolana
	{ "Reunion92",                6627,  "Reunion1992",            "*"          },          // Reseau Geodesique de la Reunion 1992
	{ "RGN/91-93",                6749,  "RGN/1991-93",            "*"          },          // Reseau Geodesique de Nouvelle Caledonie 91-93
	{ "SIRGAS2000",               6674,  "SIRGAS2000_1",           "*"          },          // Sistema de Referencia Geocentrico para America del Sur 2000
	{ "ST87/Ouvea",               6750,  "ST1987/Ouvea",           "*"          },          // ST87 Ouvea
	{ "SWEREF99",                 6619,  "SWEREF99_1",             "*"          },          // SWEREF99
	{ "YemenNtl96",               6163,  "YemenNtl1996",           "YmnNtl96"   },          // Yemen National Geodetic Network 1996
	{ "",                         6100,  "",                       "*"          }           // end of table
};

struct csGcCsRename_
{
	char oldName [24];
	unsigned long epsgNbr;
	char newName [24];
};

struct csGcCsRename_ csGcCsRename [] =
{
/*                                EPSG
       Old Name                   Nbr    New Name                              EPSG Name from mapping table */
	{ "Antarctic98.LL",             4176,  "Antarctic1998.LL"          },        // Australian Antarctic
	{ "Antilles91.LL",              4640,  "Antilles1991.LL"           },        // RRAF 1991
	{ "Caledonie91.LL",             4645,  "Caledonie1991.LL"          },        // RGNC 1991
	{ "CHTRF95.LL",                 4151,  "CHTRF95/m.LL"               },        // CHTRF95
	{ "Estonia97.Estonia",          3301,  "Estonia1997.Estonia"       },        // Estonian Coordinate System of 1997
	{ "Estonia97.LL",               4180,  "Estonia1997.LL"            },        // EST97
	{ "ETRF89.Europe/EqArea",       3035,  "ETRF1989.Europe/EqArea"    },        // ETRS89 / ETRS-LAEA
	{ "ETRF89.Europe/Lambert",      3034,  "ETRF1989.Europe/Lambert"   },        // ETRS89 / ETRS-LCC
	{ "ETRF89.FinlandGK-19",        3126,  "ETRF1989.FinlandGK-19"     },        // ETRS89 / ETRS-GK19FIN
	{ "ETRF89.FinlandGK-20",        3127,  "ETRF1989.FinlandGK-20"     },        // ETRS89 / ETRS-GK20FIN
	{ "ETRF89.FinlandGK-21",        3128,  "ETRF1989.FinlandGK-21"     },        // ETRS89 / ETRS-GK21FIN
	{ "ETRF89.FinlandGK-22",        3129,  "ETRF1989.FinlandGK-22"     },        // ETRS89 / ETRS-GK22FIN
	{ "ETRF89.FinlandGK-23",        3130,  "ETRF1989.FinlandGK-23"     },        // ETRS89 / ETRS-GK23FIN
	{ "ETRF89.FinlandGK-24",        3131,  "ETRF1989.FinlandGK-24"     },        // ETRS89 / ETRS-GK24FIN
	{ "ETRF89.FinlandGK-25",        3132,  "ETRF1989.FinlandGK-25"     },        // ETRS89 / ETRS-GK25FIN
	{ "ETRF89.FinlandGK-26",        3133,  "ETRF1989.FinlandGK-26"     },        // ETRS89 / ETRS-GK26FIN
	{ "ETRF89.FinlandGK-27",        3134,  "ETRF1989.FinlandGK-27"     },        // ETRS89 / ETRS-GK27FIN
	{ "ETRF89.FinlandGK-28",        3135,  "ETRF1989.FinlandGK-28"     },        // ETRS89 / ETRS-GK28FIN
	{ "ETRF89.FinlandGK-29",        3136,  "ETRF1989.FinlandGK-29"     },        // ETRS89 / ETRS-GK29FIN
	{ "ETRF89.FinlandGK-30",        3137,  "ETRF1989.FinlandGK-30"     },        // ETRS89 / ETRS-GK30FIN
	{ "ETRF89.FinlandGK-31",        3138,  "ETRF1989.FinlandGK-31"     },        // ETRS89 / ETRS-GK31FIN
	{ "ETRF89.GuernseyGrid",        3108,  "ETRF1989.GuernseyGrid"     },        // ETRS89 / Guernsey Grid
	{ "ETRF89.Jersey/TM",           3109,  "ETRF1989.Jersey/TM"        },        // ETRS89 / Jersey Transverse Mercator
	{ "ETRF89.TM-35/Fin",           3067,  "ETRF1989.TM-35/Fin"        },        // ETRS89 / ETRS-TM35FIN
	{ "ETRF89.TM26",                3038,  "ETRF1989.TM26"             },        // ETRS89 / ETRS-TM26
	{ "ETRF89.TM27",                3039,  "ETRF1989.TM27"             },        // ETRS89 / ETRS-TM27
	{ "ETRF89.TM28",                3040,  "ETRF1989.TM28"             },        // ETRS89 / ETRS-TM28
	{ "ETRF89.TM29",                3041,  "ETRF1989.TM29"             },        // ETRS89 / ETRS-TM29
	{ "ETRF89.TM30",                3042,  "ETRF1989.TM30"             },        // ETRS89 / ETRS-TM30
	{ "ETRF89.TM31",                3043,  "ETRF1989.TM31"             },        // ETRS89 / ETRS-TM31
	{ "ETRF89.TM32",                3044,  "ETRF1989.TM32"             },        // ETRS89 / ETRS-TM32
	{ "ETRF89.TM33",                3045,  "ETRF1989.TM33"             },        // ETRS89 / ETRS-TM33
	{ "ETRF89.TM34",                3046,  "ETRF1989.TM34"             },        // ETRS89 / ETRS-TM34
	{ "ETRF89.TM35",                3047,  "ETRF1989.TM35"             },        // ETRS89 / ETRS-TM35
	{ "ETRF89.TM36",                3048,  "ETRF1989.TM36"             },        // ETRS89 / ETRS-TM36
	{ "ETRF89.TM37",                3049,  "ETRF1989.TM37"             },        // ETRS89 / ETRS-TM37
	{ "ETRF89.TM38",                3050,  "ETRF1989.TM38"             },        // ETRS89 / ETRS-TM38
	{ "ETRF89.TM39",                3051,  "ETRF1989.TM39"             },        // ETRS89 / ETRS-TM39
	{ "ETRS89.Europe/EqArea",          0,  "ETRS1989.Europe/EqArea"    },        // 
	{ "ETRS89.Europe/Lambert",         0,  "ETRS1989.Europe/Lambert"   },        // 
	{ "ETRS89.FinlandGK-19",           0,  "ETRS1989.FinlandGK-19"     },        // 
	{ "ETRS89.FinlandGK-20",           0,  "ETRS1989.FinlandGK-20"     },        // 
	{ "ETRS89.FinlandGK-21",           0,  "ETRS1989.FinlandGK-21"     },        // 
	{ "ETRS89.FinlandGK-22",           0,  "ETRS1989.FinlandGK-22"     },        // 
	{ "ETRS89.FinlandGK-23",           0,  "ETRS1989.FinlandGK-23"     },        // 
	{ "ETRS89.FinlandGK-24",           0,  "ETRS1989.FinlandGK-24"     },        // 
	{ "ETRS89.FinlandGK-25",           0,  "ETRS1989.FinlandGK-25"     },        // 
	{ "ETRS89.FinlandGK-26",           0,  "ETRS1989.FinlandGK-26"     },        // 
	{ "ETRS89.FinlandGK-27",           0,  "ETRS1989.FinlandGK-27"     },        // 
	{ "ETRS89.FinlandGK-28",           0,  "ETRS1989.FinlandGK-28"     },        // 
	{ "ETRS89.FinlandGK-29",           0,  "ETRS1989.FinlandGK-29"     },        // 
	{ "ETRS89.FinlandGK-30",           0,  "ETRS1989.FinlandGK-30"     },        // 
	{ "ETRS89.FinlandGK-31",           0,  "ETRS1989.FinlandGK-31"     },        // 
	{ "ETRS89.GuernseyGrid",           0,  "ETRS1989.GuernseyGrid"     },        // 
	{ "ETRS89.Jersey/TM",              0,  "ETRS1989.Jersey/TM"        },        // 
	{ "ETRS89.Kp2K-Bornholm",       2198,  "ETRS1989.Kp2K-Bornholm"    },        // ETRS89 / Kp2000 Bornholm
	{ "ETRS89.Kp2K-Jutland",        2196,  "ETRS1989.Kp2K-Jutland"     },        // ETRS89 / Kp2000 Jutland
	{ "ETRS89.Kp2K-Zealand",        2197,  "ETRS1989.Kp2K-Zealand"     },        // ETRS89 / Kp2000 Zealand
	{ "ETRS89.PolandCS2K-5",        2176,  "ETRS1989.PolandCS2K-5"     },        // ETRS89 / Poland CS2000 zone 5
	{ "ETRS89.PolandCS2K-6",        2177,  "ETRS1989.PolandCS2K-6"     },        // ETRS89 / Poland CS2000 zone 6
	{ "ETRS89.PolandCS2K-7",        2178,  "ETRS1989.PolandCS2K-7"     },        // ETRS89 / Poland CS2000 zone 7
	{ "ETRS89.PolandCS2K-8",        2179,  "ETRS1989.PolandCS2K-8"     },        // ETRS89 / Poland CS2000 zone 8
	{ "ETRS89.PolandCS92",          2180,  "ETRS1989.PolandCS92"       },        // ETRS89 / Poland CS92
	{ "ETRS89.TM-30NE",             2213,  "ETRS1989.TM-30NE"          },        // ETRS89 / TM 30 NE
	{ "ETRS89.TM-35/Fin",              0,  "ETRS1989.TM-35/Fin"        },        // 
	{ "ETRS89.TM-Baltic",          25884,  "ETRS1989.TM-Baltic"        },        // ETRS89 / TM Baltic93
	{ "ETRS89.TM26",                   0,  "ETRS1989.TM26"             },        // 
	{ "ETRS89.TM27",                   0,  "ETRS1989.TM27"             },        // 
	{ "ETRS89.TM28",                   0,  "ETRS1989.TM28"             },        // 
	{ "ETRS89.TM29",                   0,  "ETRS1989.TM29"             },        // 
	{ "ETRS89.TM30",                   0,  "ETRS1989.TM30"             },        // 
	{ "ETRS89.TM31",                   0,  "ETRS1989.TM31"             },        // 
	{ "ETRS89.TM32",                   0,  "ETRS1989.TM32"             },        // 
	{ "ETRS89.TM33",                   0,  "ETRS1989.TM33"             },        // 
	{ "ETRS89.TM34",                   0,  "ETRS1989.TM34"             },        // 
	{ "ETRS89.TM35",                   0,  "ETRS1989.TM35"             },        // 
	{ "ETRS89.TM36",                   0,  "ETRS1989.TM36"             },        // 
	{ "ETRS89.TM37",                   0,  "ETRS1989.TM37"             },        // 
	{ "ETRS89.TM38",                   0,  "ETRS1989.TM38"             },        // 
	{ "ETRS89.TM39",                   0,  "ETRS1989.TM39"             },        // 
	{ "ETRS89.UTM-28N",            25828,  "ETRS1989.UTM-28N"          },        // ETRS89 / UTM zone 28N
	{ "ETRS89.UTM-29N",            25829,  "ETRS1989.UTM-29N"          },        // ETRS89 / UTM zone 29N
	{ "ETRS89.UTM-30N",            25830,  "ETRS1989.UTM-30N"          },        // ETRS89 / UTM zone 30N
	{ "ETRS89.UTM-31N",            25831,  "ETRS1989.UTM-31N"          },        // ETRS89 / UTM zone 31N
	{ "ETRS89.UTM-32N",            25832,  "ETRS1989.UTM-32N"          },        // ETRS89 / UTM zone 32N
	{ "ETRS89.UTM-33N",            25833,  "ETRS1989.UTM-33N"          },        // ETRS89 / UTM zone 33N
	{ "ETRS89.UTM-34N",            25834,  "ETRS1989.UTM-34N"          },        // ETRS89 / UTM zone 34N
	{ "ETRS89.UTM-35N",            25835,  "ETRS1989.UTM-35N"          },        // ETRS89 / UTM zone 35N
	{ "ETRS89.UTM-36N",            25836,  "ETRS1989.UTM-36N"          },        // ETRS89 / UTM zone 36N
	{ "ETRS89.UTM-37N",            25837,  "ETRS1989.UTM-37N"          },        // ETRS89 / UTM zone 37N
	{ "ETRS89.UTM-38N",            25838,  "ETRS1989.UTM-38N"          },        // ETRS89 / UTM zone 38N
	{ "GGRS87.LL",                  4121,  "Greek-GRS1987.LL"          },        // GGRS87
	{ "GR96.UTM-18N",               3178,  "GR1996.UTM-18N"            },        // GR96 / UTM zone 18N
	{ "GR96.UTM-19N",               3179,  "GR1996.UTM-19N"            },        // GR96 / UTM zone 19N
	{ "GR96.UTM-20N",               3180,  "GR1996.UTM-20N"            },        // GR96 / UTM zone 20N
	{ "GR96.UTM-21N",               3181,  "GR1996.UTM-21N"            },        // GR96 / UTM zone 21N
	{ "GR96.UTM-22N",               3182,  "GR1996.UTM-22N"            },        // GR96 / UTM zone 22N
	{ "GR96.UTM-23N",               3183,  "GR1996.UTM-23N"            },        // GR96 / UTM zone 23N
	{ "GR96.UTM-24N",               3184,  "GR1996.UTM-24N"            },        // GR96 / UTM zone 24N
	{ "GR96.UTM-25N",               3185,  "GR1996.UTM-25N"            },        // GR96 / UTM zone 25N
	{ "GR96.UTM-26N",               3186,  "GR1996.UTM-26N"            },        // GR96 / UTM zone 26N
	{ "GR96.UTM-27N",               3187,  "GR1996.UTM-27N"            },        // GR96 / UTM zone 27N
	{ "GR96.UTM-28N",               3188,  "GR1996.UTM-28N"            },        // GR96 / UTM zone 28N
	{ "GR96.UTM-29N",               3189,  "GR1996.UTM-29N"            },        // GR96 / UTM zone 29N
	{ "GreekGRS87.GreekGrid",       2100,  "Greek-GRS1987.GreekGrid"   },        // GGRS87 / Greek Grid
	{ "Greenland1996.LL",           4747,  "GR1996.LL"                 },        // GR96
	{ "GRSSA.LL",                   4170,  "GRSSA1995.LL"              },        // SIRGAS 1995
	{ "GRSSA.UTM-17N",             31986,  "GRSSA1995.UTM-17N"         },        // SIRGAS 1995 / UTM zone 17N
	{ "GRSSA.UTM-17S",             31992,  "GRSSA1995.UTM-17S"         },        // SIRGAS 1995 / UTM zone 17S
	{ "GRSSA.UTM-18N",             31987,  "GRSSA1995.UTM-18N"         },        // SIRGAS 1995 / UTM zone 18N
	{ "GRSSA.UTM-18S",             31993,  "GRSSA1995.UTM-18S"         },        // SIRGAS 1995 / UTM zone 18S
	{ "GRSSA.UTM-19N",             31988,  "GRSSA1995.UTM-19N"         },        // SIRGAS 1995 / UTM zone 19N
	{ "GRSSA.UTM-19S",             31994,  "GRSSA1995.UTM-19S"         },        // SIRGAS 1995 / UTM zone 19S
	{ "GRSSA.UTM-20N",             31989,  "GRSSA1995.UTM-20N"         },        // SIRGAS 1995 / UTM zone 20N
	{ "GRSSA.UTM-20S",             31995,  "GRSSA1995.UTM-20S"         },        // SIRGAS 1995 / UTM zone 20S
	{ "GRSSA.UTM-21N",             31990,  "GRSSA1995.UTM-21N"         },        // SIRGAS 1995 / UTM zone 21N
	{ "GRSSA.UTM-21S",             31996,  "GRSSA1995.UTM-21S"         },        // SIRGAS 1995 / UTM zone 21S
	{ "GRSSA.UTM-22N",             31991,  "GRSSA1995.UTM-22N"         },        // SIRGAS 1995 / UTM zone 22N
	{ "GRSSA.UTM-22S",             31997,  "GRSSA1995.UTM-22S"         },        // SIRGAS 1995 / UTM zone 22S
	{ "GRSSA.UTM-23S",             31998,  "GRSSA1995.UTM-23S"         },        // SIRGAS 1995 / UTM zone 23S
	{ "GRSSA.UTM-24S",             31999,  "GRSSA1995.UTM-24S"         },        // SIRGAS 1995 / UTM zone 24S
	{ "GRSSA.UTM-25S",             32000,  "GRSSA1995.UTM-25S"         },        // SIRGAS 1995 / UTM zone 25S
	{ "Guyane95.LL",                4624,  "Guyane1995.LL"             },        // RGFG95
	{ "Hartebeesthoek94.LL",        4148,  "Hartebeesthoek-94.LL"      },        // Hartebeesthoek94
	{ "Hartebeesthoek94.Lo15",      2046,  "Hartebeesthoek-94.Lo15"    },        // Hartebeesthoek94 / Lo15
	{ "Hartebeesthoek94.Lo17",      2047,  "Hartebeesthoek-94.Lo17"    },        // Hartebeesthoek94 / Lo17
	{ "Hartebeesthoek94.Lo19",      2048,  "Hartebeesthoek-94.Lo19"    },        // Hartebeesthoek94 / Lo19
	{ "Hartebeesthoek94.Lo21",      2049,  "Hartebeesthoek-94.Lo21"    },        // Hartebeesthoek94 / Lo21
	{ "Hartebeesthoek94.Lo23",      2050,  "Hartebeesthoek-94.Lo23"    },        // Hartebeesthoek94 / Lo23
	{ "Hartebeesthoek94.Lo25",      2051,  "Hartebeesthoek-94.Lo25"    },        // Hartebeesthoek94 / Lo25
	{ "Hartebeesthoek94.Lo27",      2052,  "Hartebeesthoek-94.Lo27"    },        // Hartebeesthoek94 / Lo27
	{ "Hartebeesthoek94.Lo29",      2053,  "Hartebeesthoek-94.Lo29"    },        // Hartebeesthoek94 / Lo29
	{ "Hartebeesthoek94.Lo31",      2054,  "Hartebeesthoek-94.Lo31"    },        // Hartebeesthoek94 / Lo31
	{ "Hartebeesthoek94.Lo33",      2055,  "Hartebeesthoek-94.Lo33"    },        // Hartebeesthoek94 / Lo33
	{ "IRENET95.IrishTM",           2157,  "IRENET-1995.IrishTM"       },        // IRENET95 / Irish Transverse Mercator
	{ "IRENET95.LL",                4173,  "IRENET-1995.LL"            },        // IRENET95
	{ "IRENET95.UTM-29N",           2158,  "IRENET-1995.UTM-29N"       },        // IRENET95 / UTM zone 29N
	{ "Israel.IsraeliGrid",         2039,  "Israel_1.IsraeliGrid"      },        // Israel / Israeli TM Grid
	{ "Israel.LL",                  4141,  "Israel_1.LL"               },        // Israel
	{ "Korean2000.LL",              4737,  "Korean2000_1.LL"           },        // Korea 2000
	{ "Korean95.LL",                4166,  "Korean1995.LL"             },        // Korean 1995
	{ "KuwaitUtility.KTM",             0,  "KuwaitUtility_1.KTM"       },        // Pulkovo 1942 / Gauss-Kruger CM 15E
	{ "KuwaitUtility.LL",           4319,  "KuwaitUtility_1.LL"        },        // KUDAMS
	{ "Latvia1992.LL",              4661,  "Latvia92.LL"               },        // LKS92
	{ "Latvia1992.TM",              3059,  "Latvia92.TM"               },        // LKS92 / Latvia TM
	{ "Lietuvos1994",               3346,  "Lietuvos94.TM"             },        // LKS94 / Lithuania TM
	{ "Lithuania94.LL",             4669,  "Lithuania1994.LL"          },        // LKS94
	{ "LL-ETRF89",                  4258,  "LL-ETRF1989"               },        // ETRS89
	{ "LL-ETRS89/01",                  0,  "LL-ETRS1989/01"            },        // 
	{ "MAGNA.Columbia-Bogota",      3116,  "MGNdR.Columbia-Bogota"     },        // MAGNA-SIRGAS / Colombia Bogota zone
	{ "MAGNA.Columbia-East",        3118,  "MGNdR.Columbia-East"       },        // MAGNA-SIRGAS / Colombia East zone
	{ "MAGNA.Columbia-EastCtrl",    3117,  "MGNdR.Columbia-EastCtrl"   },        // MAGNA-SIRGAS / Colombia East Central zone
	{ "MAGNA.Columbia-FarWest",     3114,  "MGNdR.Columbia-FarWest"    },        // MAGNA-SIRGAS / Colombia Far West zone
	{ "MAGNA.Columbia-West",        3115,  "MGNdR.Columbia-West"       },        // MAGNA-SIRGAS / Colombia West zone
	{ "MarcoGNR.LL",                4686,  "MGNdR.LL"                  },        // MAGNA-SIRGAS
	{ "Mauritania1999.LL",          4702,  "Mauritania99.LL"           },        // Mauritania 1999
	{ "Nakhl-eGhanem.LL",           4693,  "Nakhl-Ghanem.LL"           },        // Nakhl-e Ghanem
	{ "NGN.UTM-38N",               31838,  "NGNa.UTM-38N"              },        // NGN / UTM zone 38N
	{ "NGN.UTM-39N",               31839,  "NGNa.UTM-39N"              },        // NGN / UTM zone 39N
	{ "NtlGeodeticNet.LL",          4318,  "NationalGeodeticNet.LL"    },        // NGN
	{ "PGA98.Argentina-1",         22171,  "PGA1998.Argentina-1"       },        // POSGAR 98 / Argentina 1
	{ "PGA98.Argentina-2",         22172,  "PGA1998.Argentina-2"       },        // POSGAR 98 / Argentina 2
	{ "PGA98.Argentina-3",         22173,  "PGA1998.Argentina-3"       },        // POSGAR 98 / Argentina 3
	{ "PGA98.Argentina-4",         22174,  "PGA1998.Argentina-4"       },        // POSGAR 98 / Argentina 4
	{ "PGA98.Argentina-5",         22175,  "PGA1998.Argentina-5"       },        // POSGAR 98 / Argentina 5
	{ "PGA98.Argentina-6",         22176,  "PGA1998.Argentina-6"       },        // POSGAR 98 / Argentina 6
	{ "PGA98.Argentina-7",         22177,  "PGA1998.Argentina-7"       },        // POSGAR 98 / Argentina 7
	{ "PGA98.LL",                   4190,  "PGA1998.LL"                },        // POSGAR 98
	{ "PosGAr.LL",                     0,  "PosGAr94m.LL"              },        // 
	{ "PosGAr94.LL",                   0,  "PosGAr1994.LL"             },        // 
	{ "PosGAr98.LL",                4190,  "PosGAr1998.LL"             },        // POSGAR 98
	{ "REGVEN.LL",                  4189,  "REGVEN_1.LL"               },        // REGVEN
	{ "REGVEN.UTM-18N",             2201,  "REGVEN_1.UTM-18N"          },        // REGVEN / UTM zone 18N
	{ "REGVEN.UTM-19N",             2202,  "REGVEN_1.UTM-19N"          },        // REGVEN / UTM zone 19N
	{ "REGVEN.UTM-20N",             2203,  "REGVEN_1.UTM-20N"          },        // REGVEN / UTM zone 20N
	{ "Reunion92.LL",               4627,  "Reunion1992.LL"            },        // RGR92
	{ "RGFG95.UTM-22N",             2972,  "Guyane1995.UTM-22N"        },        // RGFG95 / UTM zone 22N
	{ "RGN-Caledonie/91-93.LL",     4749,  "RGN/1991-93.LL"            },        // RGNC91-93
	{ "RGN/91-93.NewCaledonia",     3163,  "RGN/1991-3.NewCaledonia"   },        // RGNC91-93 / Lambert New Caledonia
	{ "RGN/91-93.UTM-57S",          3169,  "RGN/1991-93.UTM-57S"       },        // RGNC91-93 / UTM zone 57S
	{ "RGN/91-93.UTM-58S",          3170,  "RGN/1991-93.UTM-58S"       },        // RGNC91-93 / UTM zone 58S
	{ "RGN/91-93.UTM-59S",          3171,  "RGN/1991-93.UTM-59S"       },        // RGNC91-93 / UTM zone 59S
	{ "RGNC91.Lambert",                0,  "Caledonie1991.Lambert"     },        // 
	{ "RGR92.UTM-40S",              2975,  "Reunion1992.UTM-40S"       },        // RGR92 / UTM zone 40S
	{ "RRAF91.UTM-20N",             2989,  "RRAF91a.UTM-20N"           },        // RRAF 1991 / UTM zone 20N
	{ "RT90_2.5V_SWEREF99",            0,  "RT90_2.5V_SWEREF99_1"      },        // 
	{ "RT90_2.5V_SWEREF99/01",      3847,  "RT90_2.5V_SWEREF99_1/01"   },        // SWEREF99 / RT90 2.5 gon V emulation
	{ "SIRGAS2000.UTM-11N",        31965,  "SIRGAS2000_1.UTM-11N"      },        // SIRGAS 2000 / UTM zone 11N
	{ "SIRGAS2000.UTM-12N",        31966,  "SIRGAS2000_1.UTM-12N"      },        // SIRGAS 2000 / UTM zone 12N
	{ "SIRGAS2000.UTM-13N",        31967,  "SIRGAS2000_1.UTM-13N"      },        // SIRGAS 2000 / UTM zone 13N
	{ "SIRGAS2000.UTM-14N",        31968,  "SIRGAS2000_1.UTM-14N"      },        // SIRGAS 2000 / UTM zone 14N
	{ "SIRGAS2000.UTM-15N",        31969,  "SIRGAS2000_1.UTM-15N"      },        // SIRGAS 2000 / UTM zone 15N
	{ "SIRGAS2000.UTM-16N",        31970,  "SIRGAS2000_1.UTM-16N"      },        // SIRGAS 2000 / UTM zone 16N
	{ "SIRGAS2000.UTM-17N",        31971,  "SIRGAS2000_1.UTM-17N"      },        // SIRGAS 2000 / UTM zone 17N
	{ "SIRGAS2000.UTM-17S",        31977,  "SIRGAS2000_1.UTM-17S"      },        // SIRGAS 2000 / UTM zone 17S
	{ "SIRGAS2000.UTM-18N",        31972,  "SIRGAS2000_1.UTM-18N"      },        // SIRGAS 2000 / UTM zone 18N
	{ "SIRGAS2000.UTM-18S",        31978,  "SIRGAS2000_1.UTM-18S"      },        // SIRGAS 2000 / UTM zone 18S
	{ "SIRGAS2000.UTM-19N",        31973,  "SIRGAS2000_1.UTM-19N"      },        // SIRGAS 2000 / UTM zone 19N
	{ "SIRGAS2000.UTM-19S",        31979,  "SIRGAS2000_1.UTM-19S"      },        // SIRGAS 2000 / UTM zone 19S
	{ "SIRGAS2000.UTM-20N",        31974,  "SIRGAS2000_1.UTM-20N"      },        // SIRGAS 2000 / UTM zone 20N
	{ "SIRGAS2000.UTM-20S",        31980,  "SIRGAS2000_1.UTM-20S"      },        // SIRGAS 2000 / UTM zone 20S
	{ "SIRGAS2000.UTM-21N",        31975,  "SIRGAS2000_1.UTM-21N"      },        // SIRGAS 2000 / UTM zone 21N
	{ "SIRGAS2000.UTM-21S",        31981,  "SIRGAS2000_1.UTM-21S"      },        // SIRGAS 2000 / UTM zone 21S
	{ "SIRGAS2000.UTM-22N",        31976,  "SIRGAS2000_1.UTM-22N"      },        // SIRGAS 2000 / UTM zone 22N
	{ "SIRGAS2000.UTM-22S",        31982,  "SIRGAS2000_1.UTM-22S"      },        // SIRGAS 2000 / UTM zone 22S
	{ "SIRGAS2000.UTM-23S",        31983,  "SIRGAS2000_1.UTM-23S"      },        // SIRGAS 2000 / UTM zone 23S
	{ "SIRGAS2000.UTM-24S",        31984,  "SIRGAS2000_1.UTM-24S"      },        // SIRGAS 2000 / UTM zone 24S
	{ "SIRGAS2000.UTM-25S",        31985,  "SIRGAS2000_1.UTM-25S"      },        // SIRGAS 2000 / UTM zone 25S
	{ "SRG-SA/2000.LL",             4674,  "SIRGAS2000_1.LL"           },        // SIRGAS 2000
	{ "ST87/Ouvea.LL",                 0,  "ST1987/Ouvea.LL"           },        // Pulkovo 1942 / Gauss-Kruger CM 15E
	{ "ST87/Ouvea.UTM-58S",         3164,  "ST1987/Ouvea.UTM-58S"      },        // ST87 Ouvea / UTM zone 58S
	{ "SWEREF-99-12-00",            3007,  "SWEREF99-12-00"            },        // SWEREF99 12 00
	{ "SWEREF-99-13-30",            3008,  "SWEREF99-13-30"            },        // SWEREF99 13 30
	{ "SWEREF-99-14-15",            3012,  "SWEREF99-14-15"            },        // SWEREF99 14 15
	{ "SWEREF-99-15-00",            3009,  "SWEREF99-15-00"            },        // SWEREF99 15 00
	{ "SWEREF-99-15-45",            3013,  "SWEREF99-15-45"            },        // SWEREF99 15 45
	{ "SWEREF-99-16-30",            3010,  "SWEREF99-16-30"            },        // SWEREF99 16 30
	{ "SWEREF-99-17-15",            3014,  "SWEREF99-17-15"            },        // SWEREF99 17 15
	{ "SWEREF-99-18-00",            3011,  "SWEREF99-18-00"            },        // SWEREF99 18 00
	{ "SWEREF-99-18-45",            3015,  "SWEREF99-18-45"            },        // SWEREF99 18 45
	{ "SWEREF-99-20-15",            3016,  "SWEREF99-20-15"            },        // SWEREF99 20 15
	{ "SWEREF-99-21-45",            3017,  "SWEREF99-21-45"            },        // SWEREF99 21 45
	{ "SWEREF-99-23-15",            3018,  "SWEREF99-23-15"            },        // SWEREF99 23 15
	{ "SWEREF-99-TM",               3006,  "SWEREF99-TM"               },        // SWEREF99 TM
	{ "SWEREF99.LL",                4619,  "SWEREF99_1.LL"             },        // SWEREF99
	{ "SWEREF99.ST74",              3152,  "SWEREF99_1.ST74"           },        // ST74
	{ "Yemen96.UTM-38N",            2089,  "Yemen1996.UTM-38N"         },        // Yemen NGN96 / UTM zone 38N
	{ "Yemen96.UTM-39N",            2090,  "Yemen1996.UTM-39N"         },        // Yemen NGN96 / UTM zone 39N
	{ "YemenNtl96.LL",              4163,  "YemenNtl1996.LL"           },        // Yemen NGN96
	{ "",                            0, ""                             }         // End of Table
};

#ifdef __SKIP__
bool GeocentricFixCrsList3P (const wchar_t* dictDir)
{
	bool ok (false);
	int myCrypt;

	enum EcsMapObjType objType;
	unsigned long epsgCode;

	csFILE *csStrm (0);
	struct csGcDtRename_* tblPtr;
	FILE* oStream;

	char epsgDescr [1024];
	char dictDirC [1024];
	char oldName [64];
	char newName [64];
	char chWork [512];

	struct cs_Csdef_ cs_def;

	objType = csMapProjGeoCSys;

	wcstombs (dictDirC,dictDir,sizeof (dictDirC));
	ok = (CS_altdr (dictDirC) == 0);

	if (ok)
	{
		ok = false;
		oStream = fopen ("C:\\Temp\\3ParamWork.txt","wt");
		if (oStream != NULL)
		{
			csStrm = CS_csopn (_STRM_BINUP);
			if (csStrm != NULL)
			{
				while (CS_csrd (csStrm,&cs_def,&myCrypt))
				{
					// See if this definition references one of the datums on our list.
					for (tblPtr = csGcDtRename;tblPtr->oldName[0] != '\0';tblPtr += 1)
					{
						if (CS_stricmp (cs_def.dat_knm,tblPtr->oldName) == 0)
						{
							// Yup!!! We have a definition which references one of the datums
							// which we will rename.
							sprintf (oldName,"\"%s\",",cs_def.key_nm);
							
							// See if we can devise a new name.
							CS_stncp (chWork,cs_def.key_nm,sizeof (chWork));
							ok = CS_strrpl (chWork,sizeof (chWork),tblPtr->oldName,tblPtr->newName);
							if (ok)
							{
								if (tblPtr->newNameCS [0] != '\0' && strlen (chWork) >= 24)
								{
									CS_stncp (chWork,cs_def.key_nm,sizeof (chWork));
									ok = CS_strrpl (chWork,sizeof (chWork),tblPtr->oldName,tblPtr->newNameCS);
								}
							}
							else
							{
								if (!strncmp (cs_def.key_nm,"ETRS89.",7))
								{
									ok = CS_strrpl (chWork,sizeof (chWork),"ETRS89","ETRS1989");
								}
								else
								{
									sprintf (chWork,"!%s",cs_def.key_nm);
								}
							}
							sprintf (newName,"\"%s\"",chWork);
							epsgDescr [0] = '\0';
							epsgCode = csMapNameToIdC (objType,csMapFlvrEpsg,csMapFlvrAutodesk,cs_def.key_nm);
							if (epsgCode != KcsNmInvNumber)
							{
								csMapIdToNameC (objType,epsgDescr,sizeof (epsgDescr),csMapFlvrEpsg,csMapFlvrEpsg,epsgCode);
							}
							else
							{
								epsgCode = 0;
							}
							fprintf (oStream,"{ %-26s   %5d,  %-26s  },        // %s\n",oldName,
																					    epsgCode,
																					    newName,
																					    epsgDescr);
							break;
						}
					}
				}
				CS_fclose (csStrm);
				ok = true;
			}
			fclose (oStream);
		}
	}
	return ok;
}

#else

bool GeocentricFixer (const char* csDictSrcDir,const char* csDictTrgDir)
{
	bool ok (false);

	ok = GeocentricFixDatum (csDictSrcDir,csDictTrgDir);
	if (ok)
	{
		ok = GeocentricFixCoordsys (csDictSrcDir,csDictTrgDir);
	}
	if (ok)
	{
		ok = GeocentricFixCategory (csDictSrcDir,csDictTrgDir);
	}
	if (ok)
	{
		ok = GeocentricFixNameMapper (csDictSrcDir,csDictTrgDir);
	}
	return ok;
}
bool GeocentricFixDatum (const char* csDictSrcDir,const char* csDictTrgDir)
{
	bool ok (false);
	std::ofstream oStrm;
	
	struct csGcDtRename_* dtTblPtr;
	TcsAscDefinition* ascDefPtr;

	char comment [128];
	char datumPathName [512];

	CS_stncp (datumPathName,csDictSrcDir,sizeof (datumPathName));
	strcat (datumPathName,"\\datums.asc");
	
	TcsDefFile datums (dictTypDatum,datumPathName);

	// Do our thing here.

	// For each entry in the above defined "ReName" table:
	//
	// 1> Locate the existing definition
	// 2> Make a copy of it.
	// 3> Change the "USE:" element of the copy back to 3PARAMETER.
	// 4> Deprecate the copy.
	//		a> change group to LEGACY
	//		b> change the description
	//		c> change the source
	//		d> Prepend a comment to the entry using the old and new names.
	// 5> Change the name of the existing definition.
	// 6> Append the copy to the end of the definition file.  We do this last
	//	  as it invalidates all pointers in the vector used to maintain the
	//	  definitions inside of a TcsDefFile object.
	ok = true;
	for (dtTblPtr = csGcDtRename;ok && dtTblPtr->oldName [0] != '\0';dtTblPtr++)
	{
		// Step 1: Locate the definition.
		ascDefPtr = datums.GetDefinition (dtTblPtr->oldName);
		if (ascDefPtr == 0)
		{
			ok = false;
			break;
		}

		// Step 2: Make a copy of it.
		TcsAscDefinition originalCopy (*ascDefPtr);
		
		// Step 3: Change the USE: specification back to 3PARAMETER
		originalCopy.SetValue ("USE:","3PARAMETER");
		// Step 4: Deprecate the copy.  If there is no GROUP element, we need to
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
		sprintf (comment,"Replaced by %s which uses GEOCENTRIC transformation",dtTblPtr->newName);
		originalCopy.SetValue ("DESC_NM:",comment);

		// Step 5: Change the name of the original definition to the new name.
		ascDefPtr->RenameDef (dtTblPtr->newName);

		// Step 6: Append it to the end of the definition file.
		// Note that the original blanks lines, and comments are still there.
		// We may want to "StripLeadingComments" and then prepend a different
		// comment.  These methods will need to be added to the TcsAscDefinition
		// object.
		datums.Append (originalCopy);

		// That's it for that one.
	}

	// If we are still OK, we write the results out to the provided target directory.
	if (ok)
	{
		CS_stncp (datumPathName,csDictTrgDir,sizeof (datumPathName));
		strcat (datumPathName,"\\datums.asc");

		oStrm.open (datumPathName,std::ios_base::out | std::ios_base::trunc);
		if (oStrm.is_open ())
		{
			datums.WriteToStream (oStrm);
			oStrm.close ();
		}
	}
	return ok;
}
bool GeocentricFixCoordsys (const char* csDictSrcDir,const char* csDictTrgDir)
{
	bool ok (false);
	std::ofstream oStrm;
	
	struct csGcDtRename_* dtTblPtr;
	struct csGcCsRename_* csTblPtr;
	TcsAscDefinition* ascDefPtr;

	char comment [128];
	char coordsysPathName [512];

	CS_stncp (coordsysPathName,csDictSrcDir,sizeof (coordsysPathName));
	strcat (coordsysPathName,"\\coordsys.asc");
	
	TcsDefFile coordsys (dictTypCoordsys,coordsysPathName);

	// What we do here is, for each entry in the above defined "ReName" table:
	//
	// 1> Locate the old definition
	// 2> Extract the original datum reference and locate in the
	//	  datum rename table.
	// 3> Make a copy of the original definition.
	// 4> Deprecate the copy of the original definition.
	//		a> change group to LEGACY
	//		b> change the description
	//		c> change the source(?)
	// 5> Change the name of the original definition.
	// 6> Change the Datum reference:
	// 7> Append the deprecated copy to the end of the definition file.
	ok = true;
	for (csTblPtr = csGcCsRename;ok && csTblPtr->oldName [0] != '\0';csTblPtr++)
	{
		// Step 1: Locate the definition.
		ascDefPtr = coordsys.GetDefinition (csTblPtr->oldName);
		if (ascDefPtr == 0)
		{
			ok = false;
			break;
		}

		// Step2: Extract the original datum reference and locate it in the
		// datum rename table.
		const char* dtNamePtr = ascDefPtr->GetValue ("DT_NAME:");
		if (dtNamePtr == 0)
		{
			ok = false;
			break;
		}
		for (dtTblPtr = csGcDtRename;ok && dtTblPtr->oldName [0] != '\0';dtTblPtr++)
		{
			if (!stricmp (dtTblPtr->oldName,dtNamePtr))
			{
				break;
			}
		}
		if (dtTblPtr->oldName [0] == '\0')
		{
			ok = false;
			break;
		}

		// Step 3: Make a copy of the original coordsys definition.
		TcsAscDefinition originalCopy (*ascDefPtr);
		
		// Step 4: Deprecate the copy.  There should be a GROUP element, but
		// just in case.
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
		sprintf (comment,"Replaced by %s. New datum %s",csTblPtr->newName,dtTblPtr->newName);
		originalCopy.SetValue ("DESC_NM:",comment);
		
		// Step 5: Change the name of the original definition.
		ascDefPtr->RenameDef (csTblPtr->newName);
		
		// Step 6: Change the datum reference.
		ascDefPtr->SetValue ("DT_NAME:",dtTblPtr->newName);

		// Step 7: Append the deprecated copy to the end of the definition file.
		coordsys.Append (originalCopy);
	}

	// If we are still OK, we write the results out to the provide target
	// directory.
	if (ok)
	{
		CS_stncp (coordsysPathName,csDictTrgDir,sizeof (coordsysPathName));
		strcat (coordsysPathName,"\\coordsys.asc");

		oStrm.open (coordsysPathName,std::ios_base::out | std::ios_base::trunc);
		if (oStrm.is_open ())
		{
			coordsys.WriteToStream (oStrm);
			oStrm.close ();
		}
	}
	return ok;
}
bool GeocentricFixCategory (const char* csDictSrcDir,const char* csDictTrgDir)
{
	bool ok (false);
	std::ifstream inStrm;
	std::ofstream outStrm;

	char* cPtr;
	struct csGcCsRename_* csTblPtr;

	char catName [128];
	char itmName [128];
	char lineBufr [256];
	char wrkBufr [256];
	char categoryPathName [512];

	enum lineType {typCatName, typItmName, typComment, typBlank, typBogus} lineType;

	// What we do here is, for each entry in the category.asc file:
	//
	// 1> Determine the type of category entry.
	// 2> If it is a coordinate system listing,
	//		a> Extract the coordinate system name
	//		b> Determine if the name is on the rename list.
	//		c> If so replace the name.
	//	3> If it is a category record,
	//		a> determine if it is the Obsolete Category
	//			i> copy all old names from the rename table

	CS_stncp (categoryPathName,csDictSrcDir,sizeof (categoryPathName));
	strcat (categoryPathName,"\\category.asc");
	inStrm.open (categoryPathName,std::ios_base::in);
	if (inStrm.is_open ())
	{
		CS_stncp (categoryPathName,csDictTrgDir,sizeof (categoryPathName));
		strcat (categoryPathName,"\\category.asc");
		outStrm.open (categoryPathName,std::ios_base::out | std::ios_base::trunc);
		if (outStrm.is_open ())
		{
			while (inStrm.good ())
			{
				inStrm.getline (lineBufr,sizeof (lineBufr),'\n');	//lint !e534
				
				// Get a copy we can play aournd with with out disturbing the
				// original which we read from the input file.  We trim leading
				// and traling white space, and then determine the line type.
				CS_stncp (wrkBufr,lineBufr,sizeof (wrkBufr));
				CS_trim (wrkBufr);
				
				// The first character essentially determines the line type.
				if (wrkBufr [0] == '[')
				{
					cPtr = strrchr (wrkBufr,']');
					if (cPtr != 0)
					{
						lineType = typCatName;
					}
					else
					{
						lineType = typBogus;
					}
				}
				else if (wrkBufr [0] == ';')  lineType = typComment;
				else if (wrkBufr [0] == '\0') lineType = typBlank;
				else
				{
					cPtr = strchr (wrkBufr,'=');
					if (cPtr != 0) lineType = typItmName;
					else           lineType = typBogus;
				}

				// If we have a category line, we extract and save the name of the
				// category.  The assumption here is that for all lines of type
				// typeItmName, there exists a current category name.
				if (lineType == typCatName)
				{
					cPtr = strrchr (wrkBufr,']');
					*cPtr = '\0';
					CS_stncp (catName,&wrkBufr [1],sizeof (catName));
				}
				else if (lineType == typItmName && stricmp (catName,"Obsolete Coordinate Systems"))
				{
					// Isolate the name.
					cPtr = strchr (wrkBufr,'=');
					*cPtr = '\0';
					CS_trim (wrkBufr);
					CS_stncp (itmName,wrkBufr,sizeof (itmName));

					// Locate the name in the rename table.
					for (csTblPtr = csGcCsRename;csTblPtr->oldName [0] != '\0';csTblPtr++)
					{
						if (!stricmp (csTblPtr->oldName,itmName))
						{
							break;
						}
					}

					// Did we find a match?
					if (csTblPtr->oldName [0] != '\0')
					{
						// Yes we did.  So, we simply replace the old name with
						// the new name in the actual string we read from the
						// file, and which we will write to the output file.
						ok = CS_strrpl (lineBufr,sizeof (lineBufr),csTblPtr->oldName,csTblPtr->newName);
					}
				}
				else if (lineType == typBlank && !stricmp (catName,"Obsolete Coordinate Systems"))
				{
					// Loop through the enitre rename table and add the old name for each
					// entry to the output file.
					for (csTblPtr = csGcCsRename;ok && csTblPtr->oldName [0] != '\0';csTblPtr++)
					{
						sprintf (wrkBufr," %s = DEPRECATED: References deprecated datum definition (3PARAMETER).",csTblPtr->oldName);
						outStrm << wrkBufr << std::endl;
					}
				}

				// Write the possibly modified line out.
				outStrm << lineBufr << std::endl;
			}
			outStrm.close ();
		}
		inStrm.close ();
	}
	return ok;
}
bool GeocentricFixNameMapper (const char* csDictSrcDir,const char* csDictTrgDir)
{
	bool ok (false);
	std::ofstream oStrm;
	
	unsigned recNbr;

	struct csGcDtRename_* dtTblPtr;
	struct csGcCsRename_* csTblPtr;

	std::wstring field;
	TcsCsvStatus csvStatus;
	
	char csvDefName [128];
	char csvPathName [512];
	wchar_t wCsvDefName [128];

	{		// desire to have a separate name space for each table
		std::wifstream inStrm;
		std::wofstream outStrm;
		CS_stncp (csvPathName,csDictSrcDir,sizeof (csvPathName));
		strcat (csvPathName,"\\..\\Data\\DatumKeyNameMap.csv");
		inStrm.open (csvPathName,std::ios_base::in);
		ok = inStrm.is_open ();
		if (ok)
		{
			TcsCsvFileBase csvDatums (true,28,28);
			ok = csvDatums.ReadFromStream (inStrm,true,csvStatus);
			if (ok)
			{
				for (recNbr = 0;ok && recNbr < csvDatums.RecordCount ();recNbr++)
				{
					ok = csvDatums.GetField (field,recNbr,L"AdskName",csvStatus);
					if (ok)
					{
						wcstombs (csvDefName,field.c_str (),sizeof (csvDefName));
						for (dtTblPtr = csGcDtRename;dtTblPtr->oldName [0] != '\0';dtTblPtr++)
						{
							if (!stricmp (dtTblPtr->oldName,csvDefName))
							{
								mbstowcs (wCsvDefName,dtTblPtr->newName,wcCount (wCsvDefName));
								field = wCsvDefName;
								ok = csvDatums.ReplaceField (field,recNbr,L"AdskName",csvStatus);
								break;
							}
						}
					}
				}
			}

			// If all is still OK, we create an output stream and write the
			// modified .csv file to that stream.
			if (ok)
			{
				CS_stncp (csvPathName,csDictTrgDir,sizeof (csvPathName));
				strcat (csvPathName,"\\DatumKeyNameMap.csv");
				outStrm.open (csvPathName,std::ios_base::out | std::ios_base::trunc);
				if (outStrm.is_open ())
				{
					ok = csvDatums.WriteToStream (outStrm,true,csvStatus);
					outStrm.close ();
				}
			}
			inStrm.close ();
		}
	}
	if (!ok)
	{
		return ok;
	}

	// Now for the Coordinate System table.  Essentially the same thing, just
	// some different names.
	{
		std::wifstream inStrm;
		std::wofstream outStrm;
		CS_stncp (csvPathName,csDictSrcDir,sizeof (csvPathName));
		strcat (csvPathName,"\\..\\Data\\ProjectiveKeyNameMap.csv");
		inStrm.open (csvPathName,std::ios_base::in);
		ok = inStrm.is_open ();
		if (ok)
		{
			TcsCsvFileBase csvCoordsys (true,28,28);
			ok = csvCoordsys.ReadFromStream (inStrm,true,csvStatus);
			if (ok)
			{
				for (recNbr = 0;ok && recNbr < csvCoordsys.RecordCount ();recNbr++)
				{
					ok = csvCoordsys.GetField (field,recNbr,L"AdskName",csvStatus);
					if (ok)
					{
						wcstombs (csvDefName,field.c_str (),sizeof (csvDefName));
						for (csTblPtr = csGcCsRename;csTblPtr->oldName [0] != '\0';csTblPtr++)
						{
							if (!stricmp (csTblPtr->oldName,csvDefName))
							{
								mbstowcs (wCsvDefName,csTblPtr->newName,wcCount (wCsvDefName));
								field = wCsvDefName;
								ok = csvCoordsys.ReplaceField (field,recNbr,L"AdskName",csvStatus);
								break;
							}
						}
					}
				}
			}

			// If all is still OK, we create an output stream and write the
			// modified .csv file to that stream.
			if (ok)
			{
				CS_stncp (csvPathName,csDictTrgDir,sizeof (csvPathName));
				strcat (csvPathName,"\\ProjectiveKeyNameMap.csv");
				outStrm.open (csvPathName,std::ios_base::out | std::ios_base::trunc);
				if (outStrm.is_open ())
				{
					ok = csvCoordsys.WriteToStream (outStrm,true,csvStatus);
					outStrm.close ();
				}
			}
			inStrm.close ();
		}
	}
	if (!ok)
	{
		return ok;
	}

	// Now for the Geographic System table.  Essentially the same thing, just
	// some different names.
	{
		std::wifstream inStrm;
		std::wofstream outStrm;
		CS_stncp (csvPathName,csDictSrcDir,sizeof (csvPathName));
		strcat (csvPathName,"\\..\\Data\\GeographicKeyNameMap.csv");
		inStrm.open (csvPathName,std::ios_base::in);
		ok = inStrm.is_open ();
		if (ok)
		{
			TcsCsvFileBase csvCoordsys (true,28,28);
			ok = csvCoordsys.ReadFromStream (inStrm,true,csvStatus);
			if (ok)
			{
				for (recNbr = 0;ok && recNbr < csvCoordsys.RecordCount ();recNbr++)
				{
					ok = csvCoordsys.GetField (field,recNbr,L"AdskName",csvStatus);
					if (ok)
					{
						wcstombs (csvDefName,field.c_str (),sizeof (csvDefName));
						for (csTblPtr = csGcCsRename;csTblPtr->oldName [0] != '\0';csTblPtr++)
						{
							if (!stricmp (csTblPtr->oldName,csvDefName))
							{
								mbstowcs (wCsvDefName,csTblPtr->newName,wcCount (wCsvDefName));
								field = wCsvDefName;
								ok = csvCoordsys.ReplaceField (field,recNbr,L"AdskName",csvStatus);
								break;
							}
						}
					}
				}
			}

			// If all is still OK, we create an output stream and write the
			// modified .csv file to that stream.
			if (ok)
			{
				CS_stncp (csvPathName,csDictTrgDir,sizeof (csvPathName));
				strcat (csvPathName,"\\GeographicKeyNameMap.csv");
				outStrm.open (csvPathName,std::ios_base::out | std::ios_base::trunc);
				if (outStrm.is_open ())
				{
					ok = csvCoordsys.WriteToStream (outStrm,true,csvStatus);
					outStrm.close ();
				}
			}
			inStrm.close ();
		}
	}
	if (!ok)
	{
		return ok;
	}

	// Now for the Geographic3D System table.  Essentially the same thing, just
	// some different names.
	{
		std::wifstream inStrm;
		std::wofstream outStrm;
		CS_stncp (csvPathName,csDictSrcDir,sizeof (csvPathName));
		strcat (csvPathName,"\\..\\Data\\Geographic3DKeyNameMap.csv");
		inStrm.open (csvPathName,std::ios_base::in);
		ok = inStrm.is_open ();
		if (ok)
		{
			TcsCsvFileBase csvCoordsys (true,28,28);
			ok = csvCoordsys.ReadFromStream (inStrm,true,csvStatus);
			if (ok)
			{
				for (recNbr = 0;ok && recNbr < csvCoordsys.RecordCount ();recNbr++)
				{
					ok = csvCoordsys.GetField (field,recNbr,L"AdskName",csvStatus);
					if (ok)
					{
						wcstombs (csvDefName,field.c_str (),sizeof (csvDefName));
						for (csTblPtr = csGcCsRename;csTblPtr->oldName [0] != '\0';csTblPtr++)
						{
							if (!stricmp (csTblPtr->oldName,csvDefName))
							{
								mbstowcs (wCsvDefName,csTblPtr->newName,wcCount (wCsvDefName));
								field = wCsvDefName;
								ok = csvCoordsys.ReplaceField (field,recNbr,L"AdskName",csvStatus);
								break;
							}
						}
					}
				}
			}

			// If all is still OK, we create an output stream and write the
			// modified .csv file to that stream.
			if (ok)
			{
				CS_stncp (csvPathName,csDictTrgDir,sizeof (csvPathName));
				strcat (csvPathName,"\\Geographic3DKeyNameMap.csv");
				outStrm.open (csvPathName,std::ios_base::out | std::ios_base::trunc);
				if (outStrm.is_open ())
				{
					ok = csvCoordsys.WriteToStream (outStrm,true,csvStatus);
					outStrm.close ();
				}
			}
			inStrm.close ();
		}
	}
	return ok;
}
#endif