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

bool ThreeParameterDatum (const char* csDictSrcDir,const char* csDictTrgDir);
bool ThreeParameterCoordsys (const char* csDictSrcDir,const char* csDictTrgDir);
bool ThreeParameterCategory (const char* csDictSrcDir,const char* csDictTrgDir);
bool ThreeParameterNameMapper (const char* csDictSrcDir,const char* csDictTrgDir);

/* The following are datums who method is Geocentric and the delta X, Y, Z's are
   zero.  These need to be:
   1> changed back to 3PARAMETER
   2> deprecated
   3> Replaced with a new definition which is referenced to GEOCENTRIC.

   Them all coordinsys which reference these datum definitions must be deprecated,
   and replaced by a coordinate system which references the new datum definition.

 1 Lithuania94
 2 Hartebeesthoek94
 3 CHTRF95-MOD
 4 YemenNtl96
 5 Korean95
 6 GRSSA
 7 PosGAr94
 8 IRENET95
 9 Antarctic98
10 Estonia97
11 REGVEN
12 PosGAr98
13 SWEREF99
14 Reunion92
15 Antilles91
16 ETRF89
17 PGA98
18 Latvia1992
19 SIRGAS2000
20 MAGNA
21 Mauritania1999
22 Korean2000
23 GR96
24 RGN/91-93
25 ETRS89/01
26 PosGAr-MOD
27 Caledonie91

*/

// These two tables define the renaming.  The changes are pretty standard and
// are hard coded into the module below.
struct csDtRename_
{
	char oldName [24];
	unsigned long epsgNbr;
	char newName  [24];
	char newNameCS [12];
};

struct csDtRename_ csDtRename [] =
{
/*                                EPSG                             Short
       Old Name                   Nbr    New Name                  Name                        EPSG Name from mapping table */
	{ "Abidjan87",                6143, "Abidjan1987",             "*"            },		// Abidjan 1987
	{ "Accra-MOD",                6168, "Accra1929",               "*"            },		// Accra
	{ "AmSamoa62",                6169, "Samoa1962",               "*"            },		// American Samoa 1962
	{ "Antigua43",                6601, "Antigua1943",             "*"            },		// Antigua 1943
	{ "Aratu",                    6208, "Aratu_1",                 "*"            },		// Aratu
	{ "Ayabelle",                 6713, "AyabelleLH",              "*"            },		// Ayabelle Lighthouse
	{ "AzoresCntrl95",            6665, "AzoresCntrl1995",         "*"            },		// Azores Central Islands 1995
	{ "AzoresEast95",             6664, "AzoresEast1995",          "*"            },		// Azores Oriental Islands 1995
	{ "AzoresWest39",             6182, "AzoresWest1939",          "*"            },		// Azores Occidental Islands 1939 +
	{ "Barbados38",               6212, "Barbados1938",            "*"            },		// Barbados 1938
	{ "Batavia",                  6211, "Batavia_1",               "*"            },		// Batavia
	{ "Beijing54",                6214, "Beijing1954",             "Bjing54"      },		// Beijing 1954
	{ "Bissau-MOD",               6165, "Bissau_1",                "*"            },		// Bissau
	{ "BukitRimpah",              6219, "BukitRimpah_1",           "*"            },		// Bukit Rimpah
	{ "Camacupa",                 6220, "Camacupa_1",              "*"            },		// Camacupa
	{ "CampAreaAstro",            6715, "CampAreaAstro_1",         "*"            },		// Camp Area Astro
	{ "Castillo",                 6161, "PampaCastillo",           "pCastillo"    },		// Pampa del Castillo
	{ "CH1903Plus",               6150, "CH1903Plus_1",            "*"            },		// CH1903+
	{ "Cocos1965",                6708, "CocosIsl1965",            "*"            },		// Cocos Islands 1965
	{ "Combani50",                6632, "Combani1950",             "*"            },		// Combani 1950
	{ "Conakry05",                6315, "Conakry1905",             "*"            },		// Conakry 1905
	{ "CongoBelge1955",           6701, "CongoBelge55",            "*"            },		// Institut Geographique du Congo Belge 1955
	{ "CSG67",                    6623, "CSG1967",                 "*"            },		// Centre Spatial Guyanais 1967
	{ "Dabola81",                 6155, "Dabola1981",              "*"            },		// Dabola 1981
	{ "DealulPiscului33",         6316, "DealulPiscului1933",      "Piscului33"   },		// Dealul Piscului 1933
	{ "DealulPiscului70",         6317, "DealulPiscului1970",      "Piscului70"   },		// Dealul Piscului 1970
	{ "DeceptionIsland",          6736, "DeceptionIsland_1",       "*"            },		// Deception Island
	{ "DeirEzZor",                6227, "DeirEzZor_1",             "*"            },		// Deir ez Zor
	{ "DiegoGarcia1969",          6724, "DiegoGarcia69",           "*"            },		// Diego Garcia 1969
	{ "Dominica45",               6602, "Dominica1945",            "*"            },		// Dominica 1945
	{ "Douala48",                 6192, "Douala1948",              "*"            },		// Douala 1948
	{ "Europ50/77",               6154, "Europ50/1977",            "*"            },		// European Datum 1950(1977)
	{ "EuropLibyan79",            6159, "ELD1979",                 "*"            },		// European Libyan Datum 1979
	{ "Fahud",                    6232, "Fahud_1",                 "*"            },		// Fahud
	{ "Fiji1956",                 6721, "Fiji56",                  "*"            },		// Fiji 1956
	{ "Final58",                  6132, "Final1958",               "*"            },		// Final Datum 1958
	{ "FortDesaix",               6625, "Martinique38",            "*"            },		// Martinique 1938
	{ "FortMarigot",              6621, "FortMarigot_1",           "*"            },		// Fort Marigot
	{ "Gan1970",                  6684, "Gan70",                   "*"            },		// Gan 1970
	{ "GrandCayman1959",          6723, "GrandCayman59",           "*"            },		// Grand Cayman 1959
	{ "Grenada53",                6603, "Grenada1953",             "*"            },		// Grenada 1953
	{ "Gulshan",                  6682, "Gulshan303",              "Glshn303"     },		// Gulshan 303
	{ "Hanoi72",                  6147, "Hanoi1972",               "*"            },		// Hanoi 1972
	{ "HeratNorth",               6255, "HeratNorth_1",            "*"            },		// Herat North
	{ "HongKong1963/67",          6739, "HongKong63/1967",         "*"            },		// Hong Kong 1963(67)
	{ "HuTzuShan",                6236, "HuTzuShan_1",             "*"            },		// Hu Tzu Shan
	{ "IGN56Lifou",               6633, "IGN56/Lifou",             "*"            },		// IGN56 Lifou
	{ "IGN72/NH",                 6630, "IGN72/NH_1",              "*"            },		// IGN72 Nuku Hiva
	{ "IGN72GrandeTerre",         6634, "IGN72/GrandeTerre",       "IGN72/GT"     },		// IGN72 Grande Terre
	{ "IGN72NukuHiva",            6630, "IGN72/NukuHiva",          "*"            },		// IGN72 Nuku Hiva
	{ "Indian54",                 6239, "Indian1954",              "*"            },		// Indian 1954
	{ "Indian60-MOD",             6131, "Indian1960/E",            "*"            },		// Indian 1960
	{ "Indian75-MOD",             6240, "Indian75/E",              "*"            },		// Indian 1975
	{ "Indonesian74",             6238, "Indonesian1974",          "*"            },		// Indonesian Datum 1974 +
	{ "IwoJima1945",              6709, "IwoJima45",               "*"            },		// Iwo Jima 1945
	{ "Jamaica69",                6242, "Jamaica1969",             "*"            },		// Jamaica 1969
	{ "Jouik1961",                6679, "Jouik61",                 "*"            },		// Jouik 1961
	{ "K01949",                   6631, "K0/1949",                 "*"            },		// K0 1949 -
	{ "Kalianpur37",              6144, "Kalianpur1937",           "*"            },		// Kalianpur 1937
	{ "Kalianpur62",              6145, "Kalianpur1962",           "*"            },		// Kalianpur 1962
	{ "Kalianpur75",              6146, "Kalianpur1975",           "*"            },		// Kalianpur 1975
	{ "Karbala1979/P",            6743, "Karbala79/P",             "*"            },		// Karbala 1979 (Polservice)
	{ "Kusaie1951",               6735, "Kusaie51",                "*"            },		// Kusaie 1951
	{ "KuwaitOilCo",              6246, "KuwaitOil",               "*"            },		// Kuwait Oil Company
	{ "LaCanoa-MOD",              6247, "LaCanoa/E",               "*"            },		// La Canoa
	{ "Lao1997",                  6678, "Lao97",                   "*"            },		// Lao National Datum 1997
	{ "Leigon-MOD",               6250, "Leigon_1",                "*"            },		// Leigon
	{ "LePouce1934",              6699, "LePouce34",               "*"            },		// Le Pouce 1934
	{ "Libyan2006",               6754, "Libyan2006_1",            "LBY2006"      },		// Libyan Geodetic Datum 2006
	{ "Lisbon1890",               6666, "Lisbon1890_1",            "*"            },		// Lisbon 1890
	{ "LittleCayman1961",         6726, "LittleCayman61",          "*"            },		// Little Cayman 1961
	{ "Locodjo65",                6142, "Locodjo1965",             "*"            },		// Locodjo 1965
	{ "Makassar-MOD",             6257, "Makassar/E",              "*"            },		// Makassar
	{ "Malongo87",                6259, "Malongo1987",             "*"            },		// Malongo 1987
	{ "Manoca-MOD",               6260, "Manoca/E",                "*"            },		// Manoca
	{ "Manoca62",                 6193, "Manoca1962",              "*"            },		// Manoca 1962
	{ "Marcus1952",               6711, "Marcus52",                "*"            },		// Marcus Island 1952
	{ "Marshalls1960",            6732, "Marshalls60",             "*"            },		// Marshall Islands 1960
	{ "Maupiti83",                6692, "Maupiti1983",             "*"            },		// Maupiti 83
	{ "MGI-MOD",                  6312, "MGI/gc",                  "*"            },		// Militar-Geographische Institut +
	{ "Mhast-MOD",                6264, "Mhast/gc",                "*"            },		// Mhast -
	{ "Miquelon50",               6638, "Miquelon1950",            "*"            },		// Saint Pierre et Miquelon 1950
	{ "MonteMario",               6265, "MonteMario_1",            "*"            },		// Monte Mario
	{ "Montserrat58",             6604, "Montserrat1958",          "*"            },		// Montserrat 1958
	{ "MOP78",                    6639, "MOP1978",                 "*"            },		// MOP78
	{ "Mporaloko",                6266, "Mporaloko_1",             "*"            },		// M'poraloko
	{ "Naparima72",               6271, "Naparima1972",            "*"            },		// Naparima 1972
	{ "NordSahara59",             6307, "NordSahara1959",          "*"            },		// Nord Sahara 1959
	{ "Nouakchott1965",           6680, "Nouakchott65",            "*"            },		// Nouakchott 1965
	{ "NTF-3P",                      0, "NTF-3P/gc",               "*"            },		// 
	{ "Perroud50",                6637, "Perroud1950",             "*"            },		// Pointe Geologie Perroud 1950
	{ "Petrels72",                6636, "Petrels1972",             "*"            },		// Petrels 1972
	{ "Phoenix Is1966",           6716, "PhoenixIs66",             "*"            },		// Phoenix Islands 1966
	{ "Point58",                  6620, "Point1958",               "*"            },		// Point 58
	{ "PointeNoire",              6282, "PointeNoire60",           "*"            },		// Congo 1960 Pointe Noire
	{ "PortoSanto",               6615, "PortoSanto36",            "*"            },		// Porto Santo 1936
	{ "PortoSanto1995",           6663, "PortoSanto95",            "*"            },		// Porto Santo 1995
	{ "Qatar74",                  6285, "Qatar1974",               "*"            },		// Qatar 1974
	{ "Qornoq27",                 6194, "Qornoq1927",              "*"            },		// Qornoq 1927
	{ "Rassadiran",               6153, "Rassadiran_1",            "*"            },		// Rassadiran
	{ "SAD69/01",                 6618, "SAD1969",                 "*"            },		// South American Datum 1969
	{ "Samboja-MOD",              6125, "Samboja_1",               "*"            },		// Samboja
	{ "Santo1965",                6730, "Santo65",                 "*"            },		// Santo 1965
	{ "Segara-MOD",               6613, "GunungSegara",            "*"            },		// Gunung Segara +
	{ "Segora",                   6294, "Segora_1",                "*"            },		// Segora
	{ "SelvagemGrande",           6616, "Selvagem",                "Selvagem"     },		// Selvagem Grande
	{ "SierraLeone68",            6175, "SierraLeone1968",         "*"            },		// Sierra Leone 1968
	{ "Solomon1968",              6718, "Solomon68",               "*"            },		// Solomon 1968
	{ "SouthGeorgia1968",         6722, "SouthGeorgia68",          "*"            },		// South Georgia 1968
	{ "SouthYemen",               6164, "SouthYemen_1",            "*"            },		// South Yemen
	{ "StAnne",                   6622, "Guadeloupe48",            "*"            },		// Guadeloupe 1948
	{ "StHelena1971",             6710, "StHelena71",              "*"            },		// St. Helena 1971
	{ "StKitts55",                6605, "StKitts1955",             "*"            },		// St. Kitts 1955
	{ "StLucia55",                6606, "StLucia1955",             "*"            },		// St. Lucia 1955
	{ "StVincent45",              6607, "StVincent1945",           "*"            },		// St. Vincent 1945
	{ "Tahaa",                    6629, "Tahaa54",                 "*"            },		// Tahaa 54
	{ "Tahiti",                   6628, "Tahiti52",                "*"            },		// Tahiti 52
	{ "Tananarive25",             6297, "Tananarive1925",          "*"            },		// Tananarive 1925
	{ "Tern1961",                 6707, "Tern61",                  "*"            },		// Tern Island 1961
	{ "TM65",                     6299, "TM1965",                  "TM65g"        },		// TM65
	{ "Trinidad03",               6302, "Trinidad1903",            "Trndd1903"    },		// Trinidad 1903
	{ "VanuaLv15",                6748, "VanuaLv1915",             "*"            },		// Vanua Levu 1915
	{ "VitiLevu12",               6752, "VitiLevu1912",            "VitiLevu12"   },		// Viti Levu 1912
	{ "Voirol1875",               6304, "Voirol1875_1",            "V1875"        },		// Voirol 1875
	{ "Wake1952",                 6733, "WakeIs1952",              "*"            },		// Wake Island 1952
	{ "Yacare-MOD",               6309, "Yacare/E",                "*"            },		// Yacare
	{ "",                            0, "",                        "*"            }			// End of Table
};

struct csCsRename_
{
	char oldName [24];
	unsigned long epsgNbr;
	char newName [24];
};

struct csCsRename_ csCsRename [] =
{
/*                                EPSG
       Old Name                   Nbr    New Name                              EPSG Name from mapping table */
	{ "Abidjan87.LL",             4143, "Abidjan1987.LL"          },		// Abidjan 1987
	{ "Abidjan87.TM-5NW",         2165, "Abidjan1987.TM-5NW"      },		// Abidjan 1987 / TM 5 NW
	{ "Abidjan87.UTM-29N",        2043, "Abidjan1987.UTM-29N"     },		// Abidjan 1987 / UTM zone 29N
	{ "Abidjan87.UTM-30N",        2041, "Abidjan1987.UTM-30N"     },		// Abidjan 1987 / UTM zone 30N
	{ "Accra.GhanaNational",      2136, "Accra1929.GhanaNational" },		// Accra / Ghana National Grid
	{ "Accra.LL",                 4168, "Accra1929.LL"            },		// Accra
	{ "Accra.TM-1NW",             2137, "Accra1929.TM-1NW"        },		// Accra / TM 1 NW
	{ "AmSamoa62.LL",             4169, "Samoa1962.LL"            },		// American Samoa 1962
	{ "AmSamoa62.Samoa/Lambert",     0, "Samoa1962.Samoa/Lambert" },		// 
	{ "Antigua43.BWIgrid",        2001, "Antigua1943.BWIgrid"     },		// Antigua 1943 / British West Indies Grid
	{ "Antigua43.LL",             4601, "Antigua1943.LL"          },		// Antigua 1943
	{ "Aratu.LL",                 4208, "Aratu_1.LL"              },		// Aratu
	{ "Aratu.UTM-22S",           20822, "Aratu_1.UTM-22S"         },		// Aratu / UTM zone 22S
	{ "Aratu.UTM-23S",           20823, "Aratu_1.UTM-23S"         },		// Aratu / UTM zone 23S
	{ "Aratu.UTM-24S",           20824, "Aratu_1.UTM-24S"         },		// Aratu / UTM zone 24S
	{ "Ayabelle.LL",              4713, "AyabelleLH.LL"           },		// Ayabelle Lighthouse
	{ "AzoresCentral1995.LL",     4665, "AzoresCntrl1995.LL"      },		// Azores Central 1995
	{ "AzoresCntrl95.UTM-26N",    3063, "AzoresCntrl1995.UTM-26N" },		// Azores Central 1995 / UTM zone 26N
	{ "AzoresEast95.UTM-26N",     3062, "AzoresEast1995.UTM-26N"  },		// Azores Oriental 1995 / UTM zone 26N
	{ "AzoresOccdtl39.LL",           0, "AzoresWest1939.LL"       },		// 
	{ "AzoresOccdtl39.UTM-25N",   2188, "AzoresWest1939.UTM-25N"  },		// Azores Occidental 1939 / UTM zone 25N
	{ "AzoresOriental1995.LL",    4664, "AzoresEast1995.LL"       },		// Azores Oriental 1995
	{ "Barbados38.BWIgrid",      21291, "Barbados1938.BWIgrid"    },		// Barbados 1938 / British West Indies Grid
	{ "Barbados38.LL",            4212, "Barbados1938.LL"         },		// Barbados 1938
	{ "Barbados38.NtlGrid",      21292, "Barbados1938.NtlGrid"    },		// Barbados 1938 / Barbados National Grid
	{ "Batavia.LL",               4211, "Batavia_1.LL"            },		// Batavia
	{ "Batavia.NEIEZ/01",            0, "Batavia_1.NEIEZ/01"      },		// 
	{ "Batavia.TM-109SE",         2308, "Batavia_1.TM-109SE"      },		// Batavia / TM 109 SE
	{ "Batavia.UTM-48S",         21148, "Batavia_1.UTM-48S"       },		// Batavia / UTM zone 48S
	{ "Batavia.UTM-49S",         21149, "Batavia_1.UTM-49S"       },		// Batavia / UTM zone 49S
	{ "Batavia.UTM-50S",         21150, "Batavia_1.UTM-50S"       },		// Batavia / UTM zone 50S
	{ "Beijing54.GK-13",         21413, "Beijing1954.GK-13"       },		// Beijing 1954 / Gauss-Kruger zone 13
	{ "Beijing54.GK-13N",        21473, "Beijing1954.GK-13N"      },		// Beijing 1954 / Gauss-Kruger 13N
	{ "Beijing54.GK-14",         21414, "Beijing1954.GK-14"       },		// Beijing 1954 / Gauss-Kruger zone 14
	{ "Beijing54.GK-14N",        21474, "Beijing1954.GK-14N"      },		// Beijing 1954 / Gauss-Kruger 14N
	{ "Beijing54.GK-15",         21415, "Beijing1954.GK-15"       },		// Beijing 1954 / Gauss-Kruger zone 15
	{ "Beijing54.GK-15N",        21475, "Beijing1954.GK-15N"      },		// Beijing 1954 / Gauss-Kruger 15N
	{ "Beijing54.GK-16",         21416, "Beijing1954.GK-16"       },		// Beijing 1954 / Gauss-Kruger zone 16
	{ "Beijing54.GK-16N",        21476, "Beijing1954.GK-16N"      },		// Beijing 1954 / Gauss-Kruger 16N
	{ "Beijing54.GK-17",         21417, "Beijing1954.GK-17"       },		// Beijing 1954 / Gauss-Kruger zone 17
	{ "Beijing54.GK-17N",        21477, "Beijing1954.GK-17N"      },		// Beijing 1954 / Gauss-Kruger 17N
	{ "Beijing54.GK-18",         21418, "Beijing1954.GK-18"       },		// Beijing 1954 / Gauss-Kruger zone 18
	{ "Beijing54.GK-18N",        21478, "Beijing1954.GK-18N"      },		// Beijing 1954 / Gauss-Kruger 18N
	{ "Beijing54.GK-19",         21419, "Beijing1954.GK-19"       },		// Beijing 1954 / Gauss-Kruger zone 19
	{ "Beijing54.GK-19N",        21479, "Beijing1954.GK-19N"      },		// Beijing 1954 / Gauss-Kruger 19N
	{ "Beijing54.GK-20",         21420, "Beijing1954.GK-20"       },		// Beijing 1954 / Gauss-Kruger zone 20
	{ "Beijing54.GK-20N",        21480, "Beijing1954.GK-20N"      },		// Beijing 1954 / Gauss-Kruger 20N
	{ "Beijing54.GK-21",         21421, "Beijing1954.GK-21"       },		// Beijing 1954 / Gauss-Kruger zone 21
	{ "Beijing54.GK-21N",        21481, "Beijing1954.GK-21N"      },		// Beijing 1954 / Gauss-Kruger 21N
	{ "Beijing54.GK-22",         21422, "Beijing1954.GK-22"       },		// Beijing 1954 / Gauss-Kruger zone 22
	{ "Beijing54.GK-22N",        21482, "Beijing1954.GK-22N"      },		// Beijing 1954 / Gauss-Kruger 22N
	{ "Beijing54.GK-23",         21423, "Beijing1954.GK-23"       },		// Beijing 1954 / Gauss-Kruger zone 23
	{ "Beijing54.GK-23N",        21483, "Beijing1954.GK-23N"      },		// Beijing 1954 / Gauss-Kruger 23N
	{ "Beijing54.GK/CM-105E",    21458, "Beijing1954.GK/CM-105E"  },		// Beijing 1954 / Gauss-Kruger CM 105E
	{ "Beijing54.GK/CM-111E",    21459, "Beijing1954.GK/CM-111E"  },		// Beijing 1954 / Gauss-Kruger CM 111E
	{ "Beijing54.GK/CM-117E",    21460, "Beijing1954.GK/CM-117E"  },		// Beijing 1954 / Gauss-Kruger CM 117E
	{ "Beijing54.GK/CM-123E",    21461, "Beijing1954.GK/CM-123E"  },		// Beijing 1954 / Gauss-Kruger CM 123E
	{ "Beijing54.GK/CM-129E",    21462, "Beijing1954.GK/CM-129E"  },		// Beijing 1954 / Gauss-Kruger CM 129E
	{ "Beijing54.GK/CM-135E",    21463, "Beijing1954.GK/CM-135E"  },		// Beijing 1954 / Gauss-Kruger CM 135E
	{ "Beijing54.GK/CM-75E",     21453, "Beijing1954.GK/CM-75E"   },		// Beijing 1954 / Gauss-Kruger CM 75E
	{ "Beijing54.GK/CM-81E",     21454, "Beijing1954.GK/CM-81E"   },		// Beijing 1954 / Gauss-Kruger CM 81E
	{ "Beijing54.GK/CM-87E",     21455, "Beijing1954.GK/CM-87E"   },		// Beijing 1954 / Gauss-Kruger CM 87E
	{ "Beijing54.GK/CM-93E",     21456, "Beijing1954.GK/CM-93E"   },		// Beijing 1954 / Gauss-Kruger CM 93E
	{ "Beijing54.GK/CM-99E",     21457, "Beijing1954.GK/CM-99E"   },		// Beijing 1954 / Gauss-Kruger CM 99E
	{ "Beijing54.GK3d-25",        2401, "Beijing1954.GK3d-25"     },		// Beijing 1954 / 3-degree Gauss-Kruger zone 25
	{ "Beijing54.GK3d-26",        2402, "Beijing1954.GK3d-26"     },		// Beijing 1954 / 3-degree Gauss-Kruger zone 26
	{ "Beijing54.GK3d-27",        2403, "Beijing1954.GK3d-27"     },		// Beijing 1954 / 3-degree Gauss-Kruger zone 27
	{ "Beijing54.GK3d-28",        2404, "Beijing1954.GK3d-28"     },		// Beijing 1954 / 3-degree Gauss-Kruger zone 28
	{ "Beijing54.GK3d-29",        2405, "Beijing1954.GK3d-29"     },		// Beijing 1954 / 3-degree Gauss-Kruger zone 29
	{ "Beijing54.GK3d-30",        2406, "Beijing1954.GK3d-30"     },		// Beijing 1954 / 3-degree Gauss-Kruger zone 30
	{ "Beijing54.GK3d-31",        2407, "Beijing1954.GK3d-31"     },		// Beijing 1954 / 3-degree Gauss-Kruger zone 31
	{ "Beijing54.GK3d-32",        2408, "Beijing1954.GK3d-32"     },		// Beijing 1954 / 3-degree Gauss-Kruger zone 32
	{ "Beijing54.GK3d-33",        2409, "Beijing1954.GK3d-33"     },		// Beijing 1954 / 3-degree Gauss-Kruger zone 33
	{ "Beijing54.GK3d-34",        2410, "Beijing1954.GK3d-34"     },		// Beijing 1954 / 3-degree Gauss-Kruger zone 34
	{ "Beijing54.GK3d-35",        2411, "Beijing1954.GK3d-35"     },		// Beijing 1954 / 3-degree Gauss-Kruger zone 35
	{ "Beijing54.GK3d-36",        2412, "Beijing1954.GK3d-36"     },		// Beijing 1954 / 3-degree Gauss-Kruger zone 36
	{ "Beijing54.GK3d-37",        2413, "Beijing1954.GK3d-37"     },		// Beijing 1954 / 3-degree Gauss-Kruger zone 37
	{ "Beijing54.GK3d-38",        2414, "Beijing1954.GK3d-38"     },		// Beijing 1954 / 3-degree Gauss-Kruger zone 38
	{ "Beijing54.GK3d-39",        2415, "Beijing1954.GK3d-39"     },		// Beijing 1954 / 3-degree Gauss-Kruger zone 39
	{ "Beijing54.GK3d-40",        2416, "Beijing1954.GK3d-40"     },		// Beijing 1954 / 3-degree Gauss-Kruger zone 40
	{ "Beijing54.GK3d-41",        2417, "Beijing1954.GK3d-41"     },		// Beijing 1954 / 3-degree Gauss-Kruger zone 41
	{ "Beijing54.GK3d-42",        2418, "Beijing1954.GK3d-42"     },		// Beijing 1954 / 3-degree Gauss-Kruger zone 42
	{ "Beijing54.GK3d-43",        2419, "Beijing1954.GK3d-43"     },		// Beijing 1954 / 3-degree Gauss-Kruger zone 43
	{ "Beijing54.GK3d-44",        2420, "Beijing1954.GK3d-44"     },		// Beijing 1954 / 3-degree Gauss-Kruger zone 44
	{ "Beijing54.GK3d-45",        2421, "Beijing1954.GK3d-45"     },		// Beijing 1954 / 3-degree Gauss-Kruger zone 45
	{ "Beijing54.GK3d/CM-102E",   2431, "Bjing54.GK3d/CM-102E"    },		// Beijing 1954 / 3-degree Gauss-Kruger CM 102E
	{ "Beijing54.GK3d/CM-105E",   2432, "Bjing54.GK3d/CM-105E"    },		// Beijing 1954 / 3-degree Gauss-Kruger CM 105E
	{ "Beijing54.GK3d/CM-108E",   2433, "Bjing54.GK3d/CM-108E"    },		// Beijing 1954 / 3-degree Gauss-Kruger CM 108E
	{ "Beijing54.GK3d/CM-111E",   2434, "Bjing54.GK3d/CM-111E"    },		// Beijing 1954 / 3-degree Gauss-Kruger CM 111E
	{ "Beijing54.GK3d/CM-114E",   2435, "Bjing54.GK3d/CM-114E"    },		// Beijing 1954 / 3-degree Gauss-Kruger CM 114E
	{ "Beijing54.GK3d/CM-117E",   2436, "Bjing54.GK3d/CM-117E"    },		// Beijing 1954 / 3-degree Gauss-Kruger CM 117E
	{ "Beijing54.GK3d/CM-120E",   2437, "Bjing54.GK3d/CM-120E"    },		// Beijing 1954 / 3-degree Gauss-Kruger CM 120E
	{ "Beijing54.GK3d/CM-123E",   2438, "Bjing54.GK3d/CM-123E"    },		// Beijing 1954 / 3-degree Gauss-Kruger CM 123E
	{ "Beijing54.GK3d/CM-126E",   2439, "Bjing54.GK3d/CM-126E"    },		// Beijing 1954 / 3-degree Gauss-Kruger CM 126E
	{ "Beijing54.GK3d/CM-129E",   2440, "Bjing54.GK3d/CM-129E"    },		// Beijing 1954 / 3-degree Gauss-Kruger CM 129E
	{ "Beijing54.GK3d/CM-132E",   2441, "Bjing54.GK3d/CM-132E"    },		// Beijing 1954 / 3-degree Gauss-Kruger CM 132E
	{ "Beijing54.GK3d/CM-135E",   2442, "Bjing54.GK3d/CM-135E"    },		// Beijing 1954 / 3-degree Gauss-Kruger CM 135E
	{ "Beijing54.GK3d/CM-75E",    2422, "Beijing1954.GK3d/CM-75E" },		// Beijing 1954 / 3-degree Gauss-Kruger CM 75E
	{ "Beijing54.GK3d/CM-78E",    2423, "Beijing1954.GK3d/CM-78E" },		// Beijing 1954 / 3-degree Gauss-Kruger CM 78E
	{ "Beijing54.GK3d/CM-81E",    2424, "Beijing1954.GK3d/CM-81E" },		// Beijing 1954 / 3-degree Gauss-Kruger CM 81E
	{ "Beijing54.GK3d/CM-84E",    2425, "Beijing1954.GK3d/CM-84E" },		// Beijing 1954 / 3-degree Gauss-Kruger CM 84E
	{ "Beijing54.GK3d/CM-87E",    2426, "Beijing1954.GK3d/CM-87E" },		// Beijing 1954 / 3-degree Gauss-Kruger CM 87E
	{ "Beijing54.GK3d/CM-90E",    2427, "Beijing1954.GK3d/CM-90E" },		// Beijing 1954 / 3-degree Gauss-Kruger CM 90E
	{ "Beijing54.GK3d/CM-93E",    2428, "Beijing1954.GK3d/CM-93E" },		// Beijing 1954 / 3-degree Gauss-Kruger CM 93E
	{ "Beijing54.GK3d/CM-96E",    2429, "Beijing1954.GK3d/CM-96E" },		// Beijing 1954 / 3-degree Gauss-Kruger CM 96E
	{ "Beijing54.GK3d/CM-99E",    2430, "Beijing1954.GK3d/CM-99E" },		// Beijing 1954 / 3-degree Gauss-Kruger CM 99E
	{ "Beijing54.LL",             4214, "Beijing1954.LL"          },		// Beijing 1954
	{ "Bissau.LL",                4165, "Bissau_1.LL"             },		// Bissau
	{ "Bissau.UTM-28N",           2095, "Bissau_1.UTM-28N"        },		// Bissau / UTM zone 28N
	{ "BukitRimpah.LL",           4219, "BukitRimpah_1.LL"        },		// Bukit Rimpah
	{ "Camacupa.LL",              4220, "Camacupa_1.LL"           },		// Camacupa
	{ "Camacupa.TM-1130SE",      22091, "Camacupa_1.TM-1130SE"    },		// Camacupa / TM 11.30 SE
	{ "Camacupa.TM-12SE",        22092, "Camacupa_1.TM-12SE"      },		// Camacupa / TM 12 SE
	{ "Camacupa.UTM-32S",        22032, "Camacupa_1.UTM-32S"      },		// Camacupa / UTM zone 32S
	{ "Camacupa.UTM-33S",        22033, "Camacupa_1.UTM-33S"      },		// Camacupa / UTM zone 33S
	{ "CampAreaAstro.LL",         4715, "CampAreaAstro_1.LL"      },		// Camp Area Astro
	{ "Castillo.Argentina-2",     2082, "pCastillo.Argentina-2"   },		// Pampa del Castillo / Argentina zone 2
	{ "Castillo.LL",              4161, "PampaCastillo.LL"        },		// Pampa del Castillo
	{ "CH1903Plus.LL",            4150, "CH1903Plus_1.LL"         },		// CH1903+
	{ "CH1903Plus.LV95/01",       2056, "CH1903Plus_1.LV95/01"    },		// CH1903+ / LV95
	{ "Cocos1965.LL",             4708, "CocosIsl1965.LL"         },		// Cocos Islands 1965
	{ "Combani50.LL",             4632, "Combani1950.LL"          },		// Combani 1950
	{ "Combani50.UTM-38S",        2980, "Combani1950.UTM-38S"     },		// Combani 1950 / UTM zone 38S
	{ "Conakry05.LL",             4315, "Conakry1905.LL"          },		// Conakry 1905
	{ "Conakry05.UTM-28N",       31528, "Conakry1905.UTM-28N"     },		// Conakry 1905 / UTM zone 28N
	{ "Conakry05.UTM-29N",       31529, "Conakry1905.UTM-29N"     },		// Conakry 1905 / UTM zone 29N
	{ "CongoBelge1955.LL",        4701, "CongoBelge55.LL"         },		// IGCB 1955
	{ "CSG67.LL",                 4623, "CSG1967.LL"              },		// CSG67
	{ "CSG67.UTM-22N",            2971, "CSG1967.UTM-22N"         },		// CSG67 / UTM zone 22N
	{ "Dabola81.LL",              4155, "Dabola1981.LL"           },		// Dabola 1981
	{ "Dabola81.UTM-28N/01",      2063, "Dabola1981.UTM-28N/01"   },		// Dabola 1981 / UTM zone 28N
	{ "Dabola81.UTM-29N/01",      2064, "Dabola1981.UTM-29N/01"   },		// Dabola 1981 / UTM zone 29N
	{ "DealulPiscului33.LL",      4316, "DealulPiscului1933.LL"   },		// Dealul Piscului 1933
	{ "DealulPiscului33.Stero",  31600, "Piscului33.Stero"        },		// Dealul Piscului 1933/ Stereo 33
	{ "DealulPiscului70.LL",      4317, "DealulPiscului1970.LL"   },		// Dealul Piscului 1970
	{ "DealulPiscului70.Stero",  31700, "Piscului70.Stero"        },		// Dealul Piscului 1970/ Stereo 70
	{ "DeceptionIsland.LL",       4736, "DeceptionIsland_1.LL"    },		// Deception Island
	{ "DeirEzZor.Levant",        22780, "DeirEzZor_1.Levant"      },		// Deir ez Zor / Levant Stereographic
	{ "DeirEzZor.LL",             4227, "DeirEzZor_1.LL"          },		// Deir ez Zor
	{ "DeirEzZor.Syria",         22770, "DeirEzZor_1.Syria"       },		// Deir ez Zor / Syria Lambert
	{ "DiegoGarcia1969.LL",       4724, "DiegoGarcia69.LL"        },		// Diego Garcia 1969
	{ "Dominica45.BWIgrid",       2002, "Dominica1945.BWIgrid"    },		// Dominica 1945 / British West Indies Grid
	{ "Dominica45.LL",            4602, "Dominica1945.LL"         },		// Dominica 1945
	{ "Douala48.AEF-West",        3119, "Douala1948.AEF-West"     },		// Douala 1948 / AEF west
	{ "Douala48.LL",              4192, "Douala1948.LL"           },		// Douala 1948
	{ "ED50/77.UTM-38N",          2058, "Europ50/1977.UTM-38N"    },		// ED50(ED77) / UTM zone 38N
	{ "ED50/77.UTM-39N",          2059, "Europ50/1977.UTM-39N"    },		// ED50(ED77) / UTM zone 39N
	{ "ED50/77.UTM-40N",          2060, "Europ50/1977.UTM-40N"    },		// ED50(ED77) / UTM zone 40N
	{ "ED50/77.UTM-41N",          2061, "Europ50/1977.UTM-41N"    },		// ED50(ED77) / UTM zone 41N
	{ "ELD79.Libya-10",           2073, "ELD1979.Libya-10"        },		// ELD79 / Libya zone 10
	{ "ELD79.Libya-11",           2074, "ELD1979.Libya-11"        },		// ELD79 / Libya zone 11
	{ "ELD79.Libya-12",           2075, "ELD1979.Libya-12"        },		// ELD79 / Libya zone 12
	{ "ELD79.Libya-13",           2076, "ELD1979.Libya-13"        },		// ELD79 / Libya zone 13
	{ "ELD79.Libya-5",            2068, "ELD1979.Libya-5"         },		// ELD79 / Libya zone 5
	{ "ELD79.Libya-6",            2069, "ELD1979.Libya-6"         },		// ELD79 / Libya zone 6
	{ "ELD79.Libya-7",            2070, "ELD1979.Libya-7"         },		// ELD79 / Libya zone 7
	{ "ELD79.Libya-8",            2071, "ELD1979.Libya-8"         },		// ELD79 / Libya zone 8
	{ "ELD79.Libya-9",            2072, "ELD1979.Libya-9"         },		// ELD79 / Libya zone 9
	{ "ELD79.TM-12NE",            2087, "ELD1979.TM-12NE"         },		// ELD79 / TM 12 NE
	{ "ELD79.UTM-32N",            2077, "ELD1979.UTM-32N"         },		// ELD79 / UTM zone 32N
	{ "ELD79.UTM-33N",            2078, "ELD1979.UTM-33N"         },		// ELD79 / UTM zone 33N
	{ "ELD79.UTM-34N",            2079, "ELD1979.UTM-34N"         },		// ELD79 / UTM zone 34N
	{ "ELD79.UTM-35N",            2080, "ELD1979.UTM-35N"         },		// ELD79 / UTM zone 35N
	{ "Europ50/77.LL",            4154, "Europ50/1977.LL"         },		// ED50(ED77)
	{ "EuropLibyan79.LL",         4159, "ELD1979.LL"              },		// ELD79
	{ "Fahud.LL",                 4232, "Fahud_1.LL"              },		// Fahud
	{ "Fahud.UTM-39N",           23239, "Fahud_1.UTM-39N"         },		// Fahud / UTM zone 39N
	{ "Fahud.UTM-40N",           23240, "Fahud_1.UTM-40N"         },		// Fahud / UTM zone 40N
	{ "Fiji1956.LL",              4721, "Fiji56.LL"               },		// Fiji 1956
	{ "Fiji1956.UTM-1S",          3142, "Fiji56.UTM-1S"           },		// Fiji 1956 / UTM zone 1S
	{ "Fiji1956.UTM-60S",         3141, "Fiji56.UTM-60S"          },		// Fiji 1956 / UTM zone 60S
	{ "Final58.Iraq",             3200, "Final1958.Iraq"          },		// FD58 / Iraq zone
	{ "Final58.LL",               4132, "Final1958.LL"            },		// FD58
	{ "FortDesaix.LL",            4625, "Martinique38.LL"         },		// Martinique 1938
	{ "FortDesaix.UTM-20N",       2973, "Martinique38.UTM-20N"    },		// Martinique 1938 / UTM zone 20N
	{ "FortMarigot.LL",           4621, "FortMarigot_1.LL"        },		// Fort Marigot
	{ "FortMarigot.UTM-20N",      2969, "FortMarigot_1.UTM-20N"   },		// Fort Marigot / UTM zone 20N
	{ "Gan1970.LL",               4684, "Gan70.LL"                },		// Gan 1970
	{ "GrandCayman1959.LL",       4723, "GrandCayman59.LL"        },		// Grand Cayman 1959
	{ "Grenada53.BWIgrid",        2003, "Grenada1953.BWIgrid"     },		// Grenada 1953 / British West Indies Grid
	{ "Grenada53.LL",             4603, "Grenada1953.LL"          },		// Grenada 1953
	{ "Gulshan-303.LL",           4682, "Gulshan303.LL"           },		// Gulshan 303
	{ "Gulshan.Bangladesh/TM",    3106, "Glshn303.Bangladesh/TM"  },		// Gulshan 303 / Bangladesh Transverse Mercator
	{ "Hanoi72.GK-106NE",         2093, "Hanoi1972.GK-106NE"      },		// Hanoi 1972 / GK 106 NE
	{ "Hanoi72.GK-18",            2044, "Hanoi1972.GK-18"         },		// Hanoi 1972 / Gauss-Kruger zone 18
	{ "Hanoi72.GK-19",            2045, "Hanoi1972.GK-19"         },		// Hanoi 1972 / Gauss-Kruger zone 19
	{ "Hanoi72.LL",               4147, "Hanoi1972.LL"            },		// Hanoi 1972
	{ "HeratNorth.LL",            4255, "HeratNorth_1.LL"         },		// Herat North
	{ "HongKong1963/67.LL",       4739, "HongKong63/1967.LL"      },		// Hong Kong 1963(67)
	{ "HuTzuShan.LL",             4236, "HuTzuShan_1.LL"          },		// Hu Tzu Shan
	{ "IGN56Lifou.LL",            4633, "IGN56/Lifou.LL"          },		// IGN56 Lifou
	{ "IGN56Lifou.UTM-58S",       2981, "IGN56/Lifou.UTM-58S"     },		// IGN56 Lifou / UTM zone 58S
	{ "IGN72/NH.UTM-58S",         3060, "IGN72/GT.UTM-58S"        },		// IGN72 Grande Terre / UTM zone 58S
	{ "IGN72GrandeTerre.LL",      4662, "IGN72/GrandeTerre.LL"    },		// IGN72 Grande Terre
//	{ "IGN72GrandTerre.LL",       4662, "IGN72/GrandeTerre.LL"    },		// IGN72 Grande Terre   --- Deprecated
//	{ "IGN72GrandTerre.UTM-58S",  2982, "IGN72/GT.UTM-58S"        },		// IGN72 Grand Terre / UTM zone 58S --- Deprecated
	{ "IGN72NukuHiva.LL",         4630, "IGN72/NukuHiva.LL"       },		// IGN72 Nuku Hiva
	{ "IGN72NukuHiva.UTM-7S",     2978, "IGN72/NukuHiva.UTM-7S"   },		// IGN72 Nuku Hiva / UTM zone 7S
	{ "Indian1960.TM-106NE",      3176, "Indian1960/E.TM-106NE"   },		// Indian 1960 / TM 106 NE
	{ "Indian1960.UTM-48N",       3148, "Indian1960/E.UTM-48N"    },		// Indian 1960 / UTM zone 48N
	{ "Indian1960.UTM-49N",       3149, "Indian1960/E.UTM-49N"    },		// Indian 1960 / UTM zone 49N
	{ "Indian54.LL",              4239, "Indian1954.LL"           },		// Indian 1954
	{ "Indian54.UTM-46N",        23946, "Indian1954.UTM-46N"      },		// Indian 1954 / UTM zone 46N
	{ "Indian54.UTM-47N",        23947, "Indian1954.UTM-47N"      },		// Indian 1954 / UTM zone 47N
	{ "Indian54.UTM-48N",        23948, "Indian1954.UTM-48N"      },		// Indian 1954 / UTM zone 48N
	{ "Indian60.LL",              4131, "Indian1960/E.LL"         },		// Indian 1960
	{ "Indian75.LL",              4240, "Indian75/E.LL"           },		// Indian 1975
	{ "Indian75.UTM-47N",        24047, "Indian75/E.UTM-47N"      },		// Indian 1975 / UTM zone 47N
	{ "Indian75.UTM-48N",        24048, "Indian75/E.UTM-48N"      },		// Indian 1975 / UTM zone 48N
	{ "Indonesian74.LL",          4238, "Indonesian1974.LL"       },		// ID74
	{ "Indonesian74.UTM-46N",    23846, "Indonesian1974.UTM-46N"  },		// ID74 / UTM zone 46N
	{ "Indonesian74.UTM-46S",    23886, "Indonesian1974.UTM-46S"  },		// ID74 / UTM zone 46S
	{ "Indonesian74.UTM-47N",    23847, "Indonesian1974.UTM-47N"  },		// ID74 / UTM zone 47N
	{ "Indonesian74.UTM-47S",    23887, "Indonesian1974.UTM-47S"  },		// ID74 / UTM zone 47S
	{ "Indonesian74.UTM-48N",    23848, "Indonesian1974.UTM-48N"  },		// ID74 / UTM zone 48N
	{ "Indonesian74.UTM-48S",    23888, "Indonesian1974.UTM-48S"  },		// ID74 / UTM zone 48S
	{ "Indonesian74.UTM-49N",    23849, "Indonesian1974.UTM-49N"  },		// ID74 / UTM zone 49N
	{ "Indonesian74.UTM-49S",    23889, "Indonesian1974.UTM-49S"  },		// ID74 / UTM zone 49S
	{ "Indonesian74.UTM-50N",    23850, "Indonesian1974.UTM-50N"  },		// ID74 / UTM zone 50N
	{ "Indonesian74.UTM-50S",    23890, "Indonesian1974.UTM-50S"  },		// ID74 / UTM zone 50S
	{ "Indonesian74.UTM-51N",    23851, "Indonesian1974.UTM-51N"  },		// ID74 / UTM zone 51N
	{ "Indonesian74.UTM-51S",    23891, "Indonesian1974.UTM-51S"  },		// ID74 / UTM zone 51S
	{ "Indonesian74.UTM-52N",    23852, "Indonesian1974.UTM-52N"  },		// ID74 / UTM zone 52N
	{ "Indonesian74.UTM-52S",    23892, "Indonesian1974.UTM-52S"  },		// ID74 / UTM zone 52S
	{ "Indonesian74.UTM-53N",    23853, "Indonesian1974.UTM-53N"  },		// ID74 / UTM zone 53N
	{ "Indonesian74.UTM-53S",    23893, "Indonesian1974.UTM-53S"  },		// ID74 / UTM zone 53S
	{ "Indonesian74.UTM-54S",    23894, "Indonesian1974.UTM-54S"  },		// ID74 / UTM zone 54S
	{ "IwoJima1945.LL",           4709, "IwoJima45.LL"            },		// Iwo Jima 1945
	{ "Jamaica69.LL",             4242, "Jamaica1969.LL"          },		// JAD69
	{ "Jamaica69.NtlGrid",       24200, "Jamaica1969.NtlGrid"     },		// JAD69 / Jamaica National Grid
	{ "Jouik1961.LL",             4679, "Jouik61.LL"              },		// Jouik 1961
	{ "K01949.LL",                4631, "K0/1949.LL"              },		// K0 1949
	{ "K01949.UTM-42S",           2979, "K0/1949.UTM-42S"         },		// K0 1949 / UTM zone 42S
	{ "Kalianpur37.India-IIb",   24375, "Kalianpur1937.India-IIb" },		// Kalianpur 1937 / India zone IIb
	{ "Kalianpur37.LL",           4144, "Kalianpur1937.LL"        },		// Kalianpur 1937
	{ "Kalianpur37.UTM-45N",     24305, "Kalianpur1937.UTM-45N"   },		// Kalianpur 1937 / UTM zone 45N
	{ "Kalianpur37.UTM-46N",     24306, "Kalianpur1937.UTM-46N"   },		// Kalianpur 1937 / UTM zone 46N
	{ "Kalianpur62.India-I",     24376, "Kalianpur1962.India-I"   },		// Kalianpur 1962 / India zone I
	{ "Kalianpur62.India-IIa",   24377, "Kalianpur1962.India-IIa" },		// Kalianpur 1962 / India zone IIa
	{ "Kalianpur62.LL",           4145, "Kalianpur1962.LL"        },		// Kalianpur 1962
	{ "Kalianpur62.UTM-41N",     24311, "Kalianpur1962.UTM-41N"   },		// Kalianpur 1962 / UTM zone 41N
	{ "Kalianpur62.UTM-42N",     24312, "Kalianpur1962.UTM-42N"   },		// Kalianpur 1962 / UTM zone 42N
	{ "Kalianpur62.UTM-43N",     24313, "Kalianpur1962.UTM-43N"   },		// Kalianpur 1962 / UTM zone 43N
	{ "Kalianpur75.India-I",     24378, "Kalianpur1975.India-I"   },		// Kalianpur 1975 / India zone I
	{ "Kalianpur75.India-IIa",   24379, "Kalianpur1975.India-IIa" },		// Kalianpur 1975 / India zone IIa
	{ "Kalianpur75.India-IIb",   24380, "Kalianpur1975.India-IIb" },		// Kalianpur 1975 / India zone IIb
	{ "Kalianpur75.India-III",   24381, "Kalianpur1975.India-III" },		// Kalianpur 1975 / India zone III
	{ "Kalianpur75.India-IV",    24383, "Kalianpur1975.India-IV"  },		// Kalianpur 1975 / India zone IV
	{ "Kalianpur75.LL",           4146, "Kalianpur1975.LL"        },		// Kalianpur 1975
	{ "Kalianpur75.UTM-42N",     24342, "Kalianpur1975.UTM-42N"   },		// Kalianpur 1975 / UTM zone 42N
	{ "Kalianpur75.UTM-43N",     24343, "Kalianpur1975.UTM-43N"   },		// Kalianpur 1975 / UTM zone 43N
	{ "Kalianpur75.UTM-44N",     24344, "Kalianpur1975.UTM-44N"   },		// Kalianpur 1975 / UTM zone 44N
	{ "Kalianpur75.UTM-45N",     24345, "Kalianpur1975.UTM-45N"   },		// Kalianpur 1975 / UTM zone 45N
	{ "Kalianpur75.UTM-46N",     24346, "Kalianpur1975.UTM-46N"   },		// Kalianpur 1975 / UTM zone 46N
	{ "Kalianpur75.UTM-47N",     24347, "Kalianpur1975.UTM-47N"   },		// Kalianpur 1975 / UTM zone 47N
	{ "Karbala1979/P.LL",         4743, "Karbala79/P.LL"          },		// Karbala 1979 (Polservice)
	{ "Kusaie1951.LL",            4735, "Kusaie51.LL"             },		// Kusaie 1951
	{ "KuwaitOilCo.Lambert",     24600, "KuwaitOil.Lambert"       },		// KOC Lambert
	{ "KuwaitOilCo.LL",           4246, "KuwaitOil.LL"            },		// KOC
	{ "LaCanoa.LL",               4247, "LaCanoa/E.LL"            },		// La Canoa
	{ "LaCanoa.UTM-18N",         24718, "LaCanoa/E.UTM-18N"       },		// La Canoa / UTM zone 18N
	{ "LaCanoa.UTM-19N",         24719, "LaCanoa/E.UTM-19N"       },		// La Canoa / UTM zone 19N
	{ "LaCanoa.UTM-20N",         24720, "LaCanoa/E.UTM-20N"       },		// La Canoa / UTM zone 20N
	{ "Lao1997.LL",               4678, "Lao97.LL"                },		// Lao 1997
	{ "Leigon.GhanaMetreGrid",   25000, "Leigon_1.GhanaMetreGrid" },		// Leigon / Ghana Metre Grid
	{ "Leigon.LL",                4250, "Leigon_1.LL"             },		// Leigon
	{ "LePouce1934.LL",           4699, "LePouce34.LL"            },		// Le Pouce 1934
	{ "Libyan2006.Libya/TM",      3177, "Libyan2006_1.Libya/TM"   },		// LGD2006 / Libya TM
	{ "Libyan2006.Libya/TM-10",   3195, "LBY2006.Libya/TM-10"     },		// LGD2006 / Libya TM zone 10
	{ "Libyan2006.Libya/TM-11",   3196, "LBY2006.Libya/TM-11"     },		// LGD2006 / Libya TM zone 11
	{ "Libyan2006.Libya/TM-12",   3197, "LBY2006.Libya/TM-12"     },		// LGD2006 / Libya TM zone 12
	{ "Libyan2006.Libya/TM-13",   3198, "LBY2006.Libya/TM-13"     },		// LGD2006 / Libya TM zone 13
	{ "Libyan2006.Libya/TM-5",    3190, "Libyan2006_1.Libya/TM-5" },		// LGD2006 / Libya TM zone 5
	{ "Libyan2006.Libya/TM-6",    3191, "Libyan2006_1.Libya/TM-6" },		// LGD2006 / Libya TM zone 6
	{ "Libyan2006.Libya/TM-7",    3192, "Libyan2006_1.Libya/TM-7" },		// LGD2006 / Libya TM zone 7
	{ "Libyan2006.Libya/TM-8",    3193, "Libyan2006_1.Libya/TM-8" },		// LGD2006 / Libya TM zone 8
	{ "Libyan2006.Libya/TM-9",    3194, "Libyan2006_1.Libya/TM-9" },		// LGD2006 / Libya TM zone 9
	{ "Libyan2006.LL",            4754, "Libyan2006_1.LL"         },		// LGD2006
	{ "Libyan2006.UTM-32N",       3199, "Libyan2006_1.UTM-32N"    },		// LGD2006 / UTM zone 32N
	{ "Libyan2006.UTM-33N",       3201, "Libyan2006_1.UTM-33N"    },		// LGD2006 / UTM zone 33N
	{ "Libyan2006.UTM-34N",       3202, "Libyan2006_1.UTM-34N"    },		// LGD2006 / UTM zone 34N
	{ "Libyan2006.UTM-35N",       3203, "Libyan2006_1.UTM-35N"    },		// LGD2006 / UTM zone 35N
	{ "Lisbon1890.LL",            4666, "Lisbon1890_1.LL"         },		// Lisbon 1890
	{ "LittleCayman1961.LL",      4726, "LittleCayman61.LL"       },		// Little Cayman 1961
	{ "Locodjo65.LL",             4142, "Locodjo1965.LL"          },		// Locodjo 1965
	{ "Locodjo65.TM-5NW",         2164, "Locodjo1965.TM-5NW"      },		// Locodjo 1965 / TM 5 NW
	{ "Locodjo65.UTM-29N",        2042, "Locodjo1965.UTM-29N"     },		// Locodjo 1965 / UTM zone 29N
	{ "Locodjo65.UTM-30N",        2040, "Locodjo1965.UTM-30N"     },		// Locodjo 1965 / UTM zone 30N
	{ "MADEIRA.UTM-28N",          3061, "PortoSanto95.UTM-28N"    },		// Porto Santo 1995 / UTM zone 28N
	{ "Makassar.LL",              4257, "Makassar/E.LL"           },		// Makassar
	{ "Makassar.NEIEZ",           3002, "Makassar/E.NEIEZ"        },		// Makassar / NEIEZ
	{ "Malongo87.LL",             4259, "Malongo1987.LL"          },		// Malongo 1987
	{ "Malongo87.UTM-32S",       25932, "Malongo1987.UTM-32S"     },		// Malongo 1987 / UTM zone 32S
	{ "Manoca.LL",                4260, "Manoca/E.LL"             },		// Manoca
	{ "Manoca62.LL",              4193, "Manoca1962.LL"           },		// Manoca 1962
	{ "Manoca62.UTM-32N",         2215, "Manoca1962.UTM-32N"      },		// Manoca 1962 / UTM zone 32N
	{ "Marcus1952.LL",            4711, "Marcus52.LL"             },		// Marcus Island 1952
	{ "Marshalls1960.LL",         4732, "Marshalls60.LL"          },		// Marshall Islands 1960
	{ "Maupiti83.LL",             4692, "Maupiti1983.LL"          },		// Maupiti 83
	{ "MGI.AustriaLambert",      31287, "MGI/gc.AustriaLambert"   },		// MGI / Austria Lambert
	{ "MGI.Balkans-5",           31275, "MGI/gc.Balkans-5"        },		// MGI / Balkans zone 5
	{ "MGI.Balkans-6",           31276, "MGI/gc.Balkans-6"        },		// MGI / Balkans zone 6
	{ "MGI.Balkans-7",           31277, "MGI/gc.Balkans-7"        },		// MGI / Balkans zone 7
	{ "MGI.Balkans-8",           31278, "MGI/gc.Balkans-8"        },		// MGI / Balkans zone 8
	{ "MGI.Gauss3d-5",           31265, "MGI/gc.Gauss3d-5"        },		// MGI / 3-degree Gauss zone 5
	{ "MGI.Gauss3d-6",           31266, "MGI/gc.Gauss3d-6"        },		// MGI / 3-degree Gauss zone 6
	{ "MGI.Gauss3d-7",           31267, "MGI/gc.Gauss3d-7"        },		// MGI / 3-degree Gauss zone 7
	{ "MGI.Gauss3d-8",           31268, "MGI/gc.Gauss3d-8"        },		// MGI / 3-degree Gauss zone 8
	{ "MGI.LL",                      0, "MGI/gc.LL"               },		// 
	{ "MGI.M28",                 31294, "MGI/gc.M28"              },		// MGI / M28
	{ "MGI.M31",                 31295, "MGI/gc.M31"              },		// MGI / M31
	{ "MGI.M34",                 31296, "MGI/gc.M34"              },		// MGI / M34
	{ "MGI.Slovenia",             2170, "MGI/gc.Slovenia"         },		// MGI / Slovenia Grid
	{ "Mhast.LL",                 4264, "Mhast/gc.LL"             },		// Mhast
	{ "Mhast.UTM-32S",           26432, "Mhast/gc.UTM-32S"        },		// Mhast / UTM zone 32S
	{ "Miquelon50.LL",            4638, "Miquelon1950.LL"         },		// Saint Pierre et Miquelon 1950
	{ "Miquelon50.UTM-21N",       2987, "Miquelon1950.UTM-21N"    },		// Saint Pierre et Miquelon 1950 / UTM zone 21N
	{ "MonteMario.Italy-1",       3003, "MonteMario_1.Italy-1"    },		// Monte Mario / Italy zone 1
	{ "MonteMario.Italy-2",       3004, "MonteMario_1.Italy-2"    },		// Monte Mario / Italy zone 2
	{ "MonteMario.LL",               0, "MonteMario_1.LL"         },		// 
	{ "Montserrat58.BWIgrid",     2004, "Montserrat1958.BWIgrid"  },		// Montserrat 1958 / British West Indies Grid
	{ "Montserrat58.LL",          4604, "Montserrat1958.LL"       },		// Montserrat 1958
	{ "MOP78.LL",                 4639, "MOP1978.LL"              },		// MOP78
	{ "MOP78.UTM-1S",             2988, "MOP1978.UTM-1S"          },		// MOP78 / UTM zone 1S
	{ "Mporaloko.LL",             4266, "Mporaloko_1.LL"          },		// M'poraloko
	{ "Mporaloko.UTM-32N",       26632, "Mporaloko_1.UTM-32N"     },		// M'poraloko / UTM zone 32N
	{ "Mporaloko.UTM-32S",       26692, "Mporaloko_1.UTM-32S"     },		// M'poraloko / UTM zone 32S
	{ "Naparima72.LL",            4271, "Naparima1972.LL"         },		// Naparima 1972
	{ "Naparima72.UTM-20N",      27120, "Naparima1972.UTM-20N"    },		// Naparima 1972 / UTM zone 20N
	{ "NordSahara59.LL",          4307, "NordSahara1959.LL"       },		// Nord Sahara 1959
	{ "NordSahara59.UnifieN",    30791, "NordSahara1959.UnifieN"  },		// Nord Sahara 1959 / Voirol Unifie Nord
	{ "NordSahara59.UnifieS",    30792, "NordSahara1959.UnifieS"  },		// Nord Sahara 1959 / Voirol Unifie Sud
	{ "NordSahara59.UTM-29N",    30729, "NordSahara1959.UTM-29N"  },		// Nord Sahara 1959 / UTM zone 29N
	{ "NordSahara59.UTM-30N",    30730, "NordSahara1959.UTM-30N"  },		// Nord Sahara 1959 / UTM zone 30N
	{ "NordSahara59.UTM-31N",    30731, "NordSahara1959.UTM-31N"  },		// Nord Sahara 1959 / UTM zone 31N
	{ "NordSahara59.UTM-32N",    30732, "NordSahara1959.UTM-32N"  },		// Nord Sahara 1959 / UTM zone 32N
	{ "Nouakchott1965.LL",        4680, "Nouakchott65.LL"         },		// Nouakchott 1965
	{ "NTF-3P.Lambert-1",            0, "NTF-3P/gc.Lambert-1"     },		// 
	{ "NTF-3P.Lambert-1C",           0, "NTF-3P/gc.Lambert-1C"    },		// 
	{ "NTF-3P.Lambert-2",            0, "NTF-3P/gc.Lambert-2"     },		// 
	{ "NTF-3P.Lambert-2C",           0, "NTF-3P/gc.Lambert-2C"    },		// 
	{ "NTF-3P.Lambert-3",            0, "NTF-3P/gc.Lambert-3"     },		// 
	{ "NTF-3P.Lambert-3C",           0, "NTF-3P/gc.Lambert-3C"    },		// 
	{ "NTF-3P.Lambert-4",            0, "NTF-3P/gc.Lambert-4"     },		// 
	{ "NTF-3P.Lambert-4C",           0, "NTF-3P/gc.Lambert-4C"    },		// 
	{ "NTF-3P.Lambert-E",            0, "NTF-3P/gc.Lambert-E"     },		// 
	{ "NTF-3P.LL",                   0, "NTF-3P/gc.LL"            },		// 
	{ "Perroud50.LL",             4637, "Perroud1950.LL"          },		// Perroud 1950
	{ "Petrels72.LL",             4636, "Petrels1972.LL"          },		// Petrels 1972
	{ "Phoenix Is1966.LL",        4716, "PhoenixIs66.LL"          },		// Phoenix Islands 1966
	{ "Point58.LL",               4620, "Point1958.LL"            },		// Point 58
	{ "PointeNoire.LL",           4282, "PointeNoire60.LL"        },		// Pointe Noire
	{ "PointeNoire.UTM-32S",     28232, "PointeNoire60.UTM-32S"   },		// Pointe Noire / UTM zone 32S
	{ "PortoSanto.LL",               0, "PortoSanto36.LL"         },		// 
	{ "PortoSanto.UTM-28N",       2942, "PortoSanto36.UTM-28N"    },		// Porto Santo / UTM zone 28N
	{ "PortoSanto1995.LL",        4663, "PortoSanto95.LL"         },		// Porto Santo 1995
	{ "Qatar74.LL",               4285, "Qatar1974.LL"            },		// Qatar 1974
	{ "Qatar74.NationalGrid",    28600, "Qatar1974.NationalGrid"  },		// Qatar 1974 / Qatar National Grid
	{ "Qornoq27.LL",              4194, "Qornoq1927.LL"           },		// Qornoq 1927
	{ "Qornoq27.UTM-22N",         2216, "Qornoq1927.UTM-22N"      },		// Qornoq 1927 / UTM zone 22N
	{ "Qornoq27.UTM-23N",         2217, "Qornoq1927.UTM-23N"      },		// Qornoq 1927 / UTM zone 23N
	{ "Rassadiran.LL",            4153, "Rassadiran_1.LL"         },		// Rassadiran
	{ "Rassadiran.NakhlTaqi",     2057, "Rassadiran_1.NakhlTaqi"  },		// Rassadiran / Nakhl e Taqi
	{ "SAD69.BzPolyconic/01",        0, "SAD1969.BzPolyconic/01"  },		// 
	{ "SAD69.LL/01",              4291, "SAD1969.LL/01"           },		// SAD69
	{ "SainteAnne.UTM-20N",       2970, "Guadeloupe48.UTM-20N"    },		// Guadeloupe 1948 / UTM zone 20N
	{ "Samboja.LL",               4125, "Samboja_1.LL"            },		// Samboja
	{ "Samboja.UTM-50S",          2550, "Samboja_1.UTM-50S"       },		// Samboja / UTM zone 50S
	{ "Santo1965.LL",             4730, "Santo65.LL"              },		// Santo 1965
	{ "Segara.LL",                4613, "GunungSegara.LL"         },		// Segara
	{ "Segara.NEIEZ/01",             0, "GunungSegara.NEIEZ/01"   },		// 
	{ "Segara.UTM-50S",           2933, "GunungSegara.UTM-50S"    },		// Segara / UTM zone 50S
	{ "Segora.LL",                4294, "Segora_1.LL"             },		// Segora
	{ "SelvagemGrande.LL",        4616, "Selvagem.LL"             },		// Selvagem Grande
	{ "SelvagemGrande.UTM-28N",   2943, "Selvagem.UTM-28N"        },		// Selvagem Grande / UTM zone 28N
	{ "SierraLeone68.LL",         4175, "SierraLeone1968.LL"      },		// Sierra Leone 1968
	{ "SierraLeone68.UTM-28N",    2161, "SierraLeone1968.UTM-28N" },		// Sierra Leone 1968 / UTM zone 28N
	{ "SierraLeone68.UTM-29N",    2162, "SierraLeone1968.UTM-29N" },		// Sierra Leone 1968 / UTM zone 29N
	{ "Solomon1968.LL",           4718, "Solomon68.LL"            },		// Solomon 1968
	{ "SouthGeorgia1968.LL",      4722, "SouthGeorgia68.LL"       },		// South Georgia 1968
	{ "SouthYemen.GK-8",          2091, "SouthYemen_1.GK-8"       },		// South Yemen / Gauss Kruger zone 8
	{ "SouthYemen.GK-9",          2092, "SouthYemen_1.GK-9"       },		// South Yemen / Gauss Kruger zone 9
	{ "SouthYemen.LL",            4164, "SouthYemen_1.LL"         },		// South Yemen
	{ "StAnne.LL",                4622, "Guadeloupe48.LL"         },		// Guadeloupe 1948
	{ "StHelena1971.LL",          4710, "StHelena71.LL"           },		// St. Helena 1971
	{ "StKitts55.BWIgrid",        2005, "StKitts1955.BWIgrid"     },		// St. Kitts 1955 / British West Indies Grid
	{ "StKitts55.LL",             4605, "StKitts1955.LL"          },		// St. Kitts 1955
	{ "StLucia55.BWIgrid",        2006, "StLucia1955.BWIgrid"     },		// St. Lucia 1955 / British West Indies Grid
	{ "StLucia55.LL",             4606, "StLucia1955.LL"          },		// St. Lucia 1955
	{ "StVincent45.BWIgrid",      2007, "StVincent1945.BWIgrid"   },		// St. Vincent 45 / British West Indies Grid
	{ "StVincent45.LL",           4607, "StVincent1945.LL"        },		// St. Vincent 1945
	{ "Tahaa.LL",                 4629, "Tahaa54.LL"              },		// Tahaa 54
	{ "Tahaa.UTM-5S",             2977, "Tahaa54.UTM-5S"          },		// Tahaa 54 / UTM zone 5S
	{ "Tahiti.LL",                4628, "Tahiti52.LL"             },		// Tahiti 52
	{ "Tahiti.UTM-6S",            2976, "Tahiti52.UTM-6S"         },		// Tahiti 52 / UTM zone 6S
	{ "Tananarive.UTM-38S",      29738, "Tananarive1925.UTM-38S"  },		// Tananarive / UTM zone 38S
	{ "Tananarive.UTM-39S",      29739, "Tananarive1925.UTM-39S"  },		// Tananarive / UTM zone 39S
	{ "Tananarive25.LL",          4297, "Tananarive1925.LL"       },		// Tananarive
	{ "Tern1961.LL",              4707, "Tern61.LL"               },		// Tern Island 1961
	{ "TM65.IrishGrid",          29902, "TM1965.IrishGrid"        },		// TM65 / Irish Grid
	{ "TM65.IrishNationalGrid",  29900, "TM65g.IrishNationalGrid" },		// TM65 / Irish National Grid
	{ "TM65.LL",                  4299, "TM1965.LL"               },		// TM65
	{ "Trinidad03.LL",            4302, "Trinidad1903.LL"         },		// Trinidad 1903
	{ "Trinidad03.Trinidad",     30200, "Trinidad1903.Trinidad"   },		// Trinidad 1903 / Trinidad Grid
	{ "Trinidad03.TrinidadGrid",  2314, "Trndd1903.TrinidadGrid"  },		// Trinidad 1903 / Trinidad Grid (ftCla)
	{ "VanuaLevu1915.LL",         4748, "VanuaLv1915.LL"          },		// Vanua Levu 1915
	{ "VitiLevu1912.LL",          4752, "VitiLevu12.LL"           },		// Viti Levu 1912
	{ "Voirol1875.LL",            4304, "Voirol1875_1.LL"         },		// Voirol 1875
	{ "Voirol1875.NordAlgerie",  30491, "V1875.NordAlgerie"       },		// Voirol 1875 / Nord Algerie (ancienne)
	{ "Voirol1875.SudAlgerie",   30492, "Voirol1875_1.SudAlgerie" },		// Voirol 1875 / Sud Algerie (ancienne)
	{ "Wake1952.LL",              4733, "WakeIs1952.LL"           },		// Wake Island 1952
	{ "Yacare.LL",                4309, "Yacare/E.LL"             },		// Yacare
	{ "",                            0, ""                        }		// End of Table
};

bool ThreeParameterFixer (const char* csDictSrcDir,const char* csDictTrgDir)
{
	bool ok (false);

	ok = ThreeParameterDatum (csDictSrcDir,csDictTrgDir);
	if (ok)
	{
		ok = ThreeParameterCoordsys (csDictSrcDir,csDictTrgDir);
	}
	if (ok)
	{
		ok = ThreeParameterCategory (csDictSrcDir,csDictTrgDir);
	}
	if (ok)
	{
		ok = ThreeParameterNameMapper (csDictSrcDir,csDictTrgDir);
	}
	return ok;
}
bool ThreeParameterDatum (const char* csDictSrcDir,const char* csDictTrgDir)
{
	bool ok (false);
	std::ofstream oStrm;
	
	struct csDtRename_* dtTblPtr;
	TcsAscDefinition* ascDefPtr;

	char comment [128];
	char datumPathName [512];

	CS_stncp (datumPathName,csDictSrcDir,sizeof (datumPathName));
	strcat (datumPathName,"\\datums.asc");
	
	TcsDefFile datums (dictTypDatum,datumPathName);

	// Do our thing here.

	// For each entry in the above defined "ReName" table:
	//
	// 1> Locate the old definition
	// 2> Make a copy of it.
	// 3> Deprecate the copy.
	//		a> change group to LEGACY
	//		b> change the description
	//		c> change the source
	//		d> Prepend a comment to the entry using the old and new names.
	// 4> Change the name of the original definition.
	// 5> Change the "USE:" specification to "GEOCENTRIC"
	// 6> Append the copy to the end of the definition file.  We do this last
	//	  as it invalidates all pointers in the vector used to maintain the
	//	  definitions inside of a TcsDefFile object.
	ok = true;
	for (dtTblPtr = csDtRename;ok && dtTblPtr->oldName [0] != '\0';dtTblPtr++)
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
		
		// Step 3: Deprecate the copy.  If there is no GROUP element, we need to
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

		// Step 4: Change the name of the original definition.
		ascDefPtr->RenameDef (dtTblPtr->newName);
		
		// Step 5: Change the USE: specification to GEOCENTRIC
		ascDefPtr->SetValue ("USE:","GEOCENTRIC");

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
bool ThreeParameterCoordsys (const char* csDictSrcDir,const char* csDictTrgDir)
{
	bool ok (false);
	std::ofstream oStrm;
	
	struct csDtRename_* dtTblPtr;
	struct csCsRename_* csTblPtr;
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
	for (csTblPtr = csCsRename;ok && csTblPtr->oldName [0] != '\0';csTblPtr++)
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
		for (dtTblPtr = csDtRename;ok && dtTblPtr->oldName [0] != '\0';dtTblPtr++)
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
bool ThreeParameterCategory (const char* csDictSrcDir,const char* csDictTrgDir)
{
	bool ok (false);
	std::ifstream inStrm;
	std::ofstream outStrm;

	char* cPtr;
	struct csCsRename_* csTblPtr;

	char catName [128];
	char itmName [128];
	char lineBufr [256];
	char wrkBufr [256];
	char categoryPathName [512];

	enum lineType {typCatName, typItmName, typComment, typBlank, typBogus} lineType;

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
					for (csTblPtr = csCsRename;csTblPtr->oldName [0] != '\0';csTblPtr++)
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
					// Loop through the enitre rename table and add the old neame fo each
					// entry to the output file.
					for (csTblPtr = csCsRename;ok && csTblPtr->oldName [0] != '\0';csTblPtr++)
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
bool ThreeParameterNameMapper (const char* csDictSrcDir,const char* csDictTrgDir)
{
	bool ok (false);
	std::ofstream oStrm;
	
	unsigned recNbr;

	struct csDtRename_* dtTblPtr;
	struct csCsRename_* csTblPtr;

	std::wstring field;
	TcsCsvStatus csvStatus;
	
	char csvDefName [128];
	char csvPathName [512];
	wchar_t wCsvDefName [128];

	{
		std::wifstream inStrm;
		std::wofstream outStrm;
		CS_stncp (csvPathName,csDictSrcDir,sizeof (csvPathName));
		strcat (csvPathName,"\\..\\..\\Data\\DatumKeyNameMap.csv");
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
						for (dtTblPtr = csDtRename;dtTblPtr->oldName [0] != '\0';dtTblPtr++)
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
		strcat (csvPathName,"\\..\\..\\Data\\ProjectiveKeyNameMap.csv");
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
						for (csTblPtr = csCsRename;csTblPtr->oldName [0] != '\0';csTblPtr++)
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
		strcat (csvPathName,"\\..\\..\\Data\\GeographicKeyNameMap.csv");
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
						for (csTblPtr = csCsRename;csTblPtr->oldName [0] != '\0';csTblPtr++)
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
		strcat (csvPathName,"\\..\\..\\Data\\Geographic3DKeyNameMap.csv");
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
						for (csTblPtr = csCsRename;csTblPtr->oldName [0] != '\0';csTblPtr++)
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
