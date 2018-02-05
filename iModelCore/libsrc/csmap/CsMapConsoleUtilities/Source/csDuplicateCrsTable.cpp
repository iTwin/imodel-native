/* The following table was produced by the ListDUplicateDefinitions
   utility function.  This result was editted manually, primarily by
   commenting out several definitions which, though technically a
   mathematical duplicate, were determined to be left undprecacted.

   This decision was based on the concept that if an end user can
   be reasonably expected to be looking for a definition, it should
   not be deprecated even if it is a duplicate.
   
   Put another way, what we ant to do here is remove duplicates which
   are the result of several people maintaining the same database
   independent of each other. */

struct TcsDulicateCrsNames
{
    char Primary    [32];
    char Duplicate1 [32];
    char Duplicate2 [32];
    char Duplicate3 [32];
    char Duplicate4 [32];
    char Duplicate5 [32];
    char Duplicate6 [32];
    char Duplicate7 [32];
    char Duplicate8 [32];
} KcsDuplicateCrsName [] =
{
    /* Primary Name (?)             Duplicate Name(s)              */
    {  "Adindan.UTM-37N",          "ETSU_W",                       },
    {  "Adindan.UTM-38N",          "ETSU_E",                       },
    {  "Afgooye.UTM-38N",          "SOMAL-W",                      },
    {  "Afgooye.UTM-39N",          "SOMAL-E",                      },
    {  "ARC1950.UTM-34S/01",       "AFR50-34S/01",                 },
    {  "ARC1950.UTM-35S/01",       "AFR50-35S/01",
                                   "ZIM-35/01",                    },
    {  "ARC1950.UTM-36S/01",       "AFR50-36S/01",
                                   "ZIM-36/01",                    },
    {  "ARC1960.UTM-35N",          "AFR60-35N",                    },
    {  "ARC1960.UTM-35S",          "AFR60-35S",                    },
    {  "ARC1960.UTM-36N",          "AFR60-36N",
                                   "KEN-N-W",                      },
    {  "ARC1960.UTM-36S",          "AFR60-36S",
                                   "KEN-S-W",                      },
    {  "ARC1960.UTM-37N",          "AFR60-37N",
                                   "KEN-N-E",                      },
    {  "ARC1960.UTM-37S",          "AFR60-37S",
                                   "KEN-S-E",                      },
    {  "ASTRLA66-VicNsw.LL",       "AGD66-Vic/NSW.LL",             },
    {  "AinElAbd.BahrainGrid",     "AinElAbd.UTM-39N",
                                   "BAH-SG",
                                   "SAUD-E",                       },
    {  "AinElAbd.UTM-37N",         "SAUD-W",                       },
    {  "AinElAbd.UTM-38N",         "KU-AEA",
                                   "SAUD-C",                       },
    {  "Arc1950.LL/01",            "LL-ARC1950/01",                },
    {  "Arc1960.LL",               "LL-ARC1960",                   },
    {  "ASTRLA66.LL",              "LL-ASTRLA66",                  },
    {  "ASTRLA84.LL",              "LL-AGD84",                     },
    {  "ATS77.UTM-19N",            "CANNB-19N-ATS",                },
    {  "ATS77.UTM-20N",            "CANNB-20N-ATS",                },
    {  "AzoresOrntl40.UTM-26N",    "F26-SB",                       },
/*===================================================================
   With respect to the following, there exists a set of six degree
   zones and a set of three degree zones; with the same scale
   reduction.  Thus, every other one of the three degree zones will
   duplicate one of the six degree zones.  We're leaving these as
   they have distinct EPSG code numbers.  The remainder are indeed
   duplicates */
    {  "Beijing1954.GK/CM-75E",    "Beijing1954.GK-13N"            },
    {  "Beijing1954.GK/CM-81E",    "Beijing1954.GK-14N"            },
    {  "Beijing1954.GK/CM-87E",    "Beijing1954.GK-15N"            },
    {  "Beijing1954.GK/CM-93E",    "Beijing1954.GK-16N"            },
    {  "Beijing1954.GK/CM-99E",    "Beijing1954.GK-17N"            },
    {  "Beijing1954.GK/CM-105E",   "Beijing1954.GK-18N"            },
    {  "Beijing1954.GK/CM-111E",   "Beijing1954.GK-19N"            },
    {  "Beijing1954.GK/CM-117E",   "Beijing1954.GK-20N"            },
    {  "Beijing1954.GK/CM-123E",   "Beijing1954.GK-21N"            },
    {  "Beijing1954.GK/CM-129E",   "Beijing1954.GK-22N"            },
    {  "Beijing1954.GK/CM-135E",   "Beijing1954.GK-23N"            },
/*=================================================================*/
    {  "UTM-31N",                  "SPAIN-TM31-I",
                                   "BELGE-U31"                     },
    {  "UTM-32N",                  "DANE-32",
                                   "NORGE-32",
                                   "BELGE-U32"                     },
    {  "NAD83.BLM-14N",            "BLM-14"                        },
    {  "NAD83.BLM-15NF",           "BLM-15"                        },
    {  "NAD83.BLM-16NF",           "BLM-16"                        },
    {  "NAD83.BLM-17NF",           "BLM-17"                        },
    {  "BOGOTA.Colombia-Bogota",   "COL-B-B",                      },
    {  "BOGOTA.Colombia-EC",       "COL-EC-B",                     },
    {  "BOGOTA.Colombia-W",        "COL-O-B",                      },
    {  "BOGOTA.ColombiaE",         "COL-EE-B",                     },
    {  "BORNEO",                   "TIMBALAI",                     },
//  {  "BOSNIA-5M",                "BOSNIA-5M-S9",
//                                 "CROATIA-5M",                   },	// Same, but entered by the same person.
//  {  "BOSNIA-6M",                "BOSNIA-6M-S9",
//                                 "CROATIA-6M",
//                                 "JUGO-6M",                      },	// Same, but entered by the same person.
    {  "SA1969.BzPolyconic",       "BRA-P",                        },
    {  "Corrego.UTM-23S",          "BRAC-NE1",                     },
    {  "Corrego.UTM-24S",          "BRAC-NE2",                     },
    {  "BritishNatGrid",           "GB_ORD1-MOD",                  },
//  {  "BUL-34",                   "CZ-S42-4",
//                                 "EST-P42-34",
//                                 "GK-4",
//                                 "HUN-34",
//                                 "LIT-GK4",
//                                 "ROM-34",
//                                 "SK-S42-4",                     },	// Same, but entered by the same person.
//  {  "BUL-35",                   "EST-P42-35",
//                                 "GK-5",
//                                 "LIT-GK5",
//                                 "ROM-35",                       },	// Same, but entered by the same person.
    {  "Campo.Argentina1",         "FAJA-1",                       },
    {  "Campo.Argentina2",         "FAJA-2",                       },
    {  "Campo.Argentina3",         "FAJA-3",                       },
    {  "Campo.Argentina4",         "FAJA-4",                       },
    {  "Campo.Argentina5",         "FAJA-5",                       },
    {  "Campo.Argentina6",         "FAJA-6",                       },
    {  "Campo.Argentina7",         "FAJA-7",                       },
//  {  "CAN27-10",                 "CANQ27-M10",                   },	// Numerically the same, but for different provinces.
//  {  "CAN27-11",                 "CANQ27-M11M",                  },	// Numerically the same, but for different provinces.
//  {  "CAN27-12",                 "CANQ27-M12",                   },	// Numerically the same, but for different provinces.
//  {  "CAN27-13",                 "CANQ27-M13",                   },	// Numerically the same, but for different provinces.
//  {  "CAN27-14",                 "CAN83-14",                     },	// CRS Bust!!!
//  {  "CAN27-14",                 "CANQ27-M14",                   },	// Numerically the same, but for different provinces.
//  {  "CAN27-15",                 "CANQ27-M15M",                  },	// Numerically the same, but for different provinces.
//  {  "CAN27-16",                 "CANQ27-M16",                   },	// Numerically the same, but for different provinces.
//  {  "CAN27-17",                 "CANQ27-M17",                   },	// Numerically the same, but for different provinces.
//  {  "CAN27-8",                  "CANQ27-M8M",                   },	// Numerically the same, but for different provinces.
//  {  "CAN27-9",                  "CANQ27-M9",                    },	// Numerically the same, but for different provinces.
//  {  "UTM27-15",                 "CAN27-Ont15"                   },	// Numerically the same, but for different provinces.
//  {  "UTM27-16",                 "CAN27-Ont16"                   },	// Numerically the same, but for different provinces.
//  {  "UTM27-17",                 "CAN27-Ont17"                   },	// Numerically the same, but for different provinces.
//  {  "UTM27-18",                 "CAN27-Ont18"                   },	// Numerically the same, but for different provinces.

//  {  "CAN83-10",                 "CANQ-M10",                     },	// Numerically the same, but for different provinces.
//  {  "CAN83-11",                 "CANQ-M11",                     },	// Numerically the same, but for different provinces.
//  {  "CAN83-12",                 "CANQ-M12",                     },	// Numerically the same, but for different provinces.
//  {  "CAN83-13",                 "CANQ-M13",                     },	// Numerically the same, but for different provinces.
//  {  "CAN83-15",                 "CANQ-M15",                     },	// Numerically the same, but for different provinces.
//                                 "CANQ27-M15",                   },	// CRS Bust !!!!
//  {  "CAN83-16",                 "CANQ-M16",                     },	// Numerically the same, but for different provinces.
//  {  "CAN83-17",                 "CANQ-M17",                     },	// Numerically the same, but for different provinces.
//  {  "CAN83-8",                  "CANQ-M8",                      },	// Numerically the same, but for different provinces.
//  {  "CAN83-9",                  "CANQ-M9",                      },	// Numerically the same, but for different provinces.
    {  "NAD83.QuebecLambert",      "CANQ-LCC-83",                  },
    {  "MTM83-2",                  "CANQ-M2N",                      },
    {  "NAD27.MTM-1",              "CANQ27-M1",                    },
    {  "NAD27.MTM-2",              "CANQ27-M2N",                   },
    {  "CAPE.LL",                  "LL-CAPE",                      },
    {  "Carthage.UTM-32N",         "TUNIS-O-MOD",                  },
//  {  "CH-13-P",                  "GK-13",                        },	// Same, but 1 for China and 1 for Russia
//  {  "CH-14-P",                  "GK-14",                        },	// Same, but 1 for China and 1 for Russia
//  {  "CH-15-P",                  "GK-15",                        },	// Same, but 1 for China and 1 for Russia
//  {  "CH-16-P",                  "GK-16",                        },	// Same, but 1 for China and 1 for Russia
//  {  "CH-17-P",                  "GK-17",                        },	// Same, but 1 for China and 1 for Russia
//  {  "CH-18-P",                  "GK-18",                        },	// Same, but 1 for China and 1 for Russia
//  {  "CH-19-P",                  "GK-19",                        },	// Same, but 1 for China and 1 for Russia
//  {  "CH-20-P",                  "GK-20",                        },	// Same, but 1 for China and 1 for Russia
//  {  "CH-21-P",                  "GK-21",                        },	// Same, but 1 for China and 1 for Russia
//  {  "CH-22-P",                  "GK-22",                        },	// Same, but 1 for China and 1 for Russia
//  {  "CH-23-P",                  "GK-23",                        },	// Same, but 1 for China and 1 for Russia
    {  "CH1903.LV03/01",           "SWISS",                        },
//  {  "CO83-SF",                  "COHP-SF",                      },	/* CRS Bust!!! */
//  {  "CROATIA-5",                "CROATIA-5-S9",						// Suspect a CRS Bust on Croatia-5-S9
//                                 "GK-S5",
//                                 "GRMNY-S5",
//                                 "SERBIA-5",
//                                 "SLOVENIA-5",
//                                 "SLOVENIA-5-S9",                },	// Suspect a CRS Bust on Slovenia-5-S9
//  {  "CROATIA-6",                "CROATIA-6-S9",                 },	// Suspect a CRS Bust on Croatia-6-S9
    {  "Pulkovo42.CS63-A1",        "CS63-A1",                      },
    {  "Pulkovo42.CS63-A2",        "CS63-A2",                      },
    {  "Pulkovo42.CS63-A3",        "CS63-A3",                      },
    {  "Pulkovo42.CS63-A4",        "CS63-A4",                      },
    {  "Pulkovo42.CS63-K2",        "CS63-K2",                      },
    {  "Pulkovo42.CS63-K3",        "CS63-K3",                      },
    {  "Pulkovo42.CS63-K4",        "CS63-K4",                      },
    {  "CSRS.MTM-10",              "NAD83/98.MTM-10",              },	// NAD83/98 version deprecated by EPSG, name change to CSRS98
    {  "CSRS.MTM-3",               "NAD83/98.MTM-3",               },	// NAD83/98 version deprecated by EPSG, name change to CSRS98
    {  "CSRS.MTM-4",               "NAD83/98.MTM-4",               },	// NAD83/98 version deprecated by EPSG, name change to CSRS98
    {  "CSRS.MTM-5",               "NAD83/98.MTM-5",               },	// NAD83/98 version deprecated by EPSG, name change to CSRS98
    {  "CSRS.MTM-6",               "NAD83/98.MTM-6",               },	// NAD83/98 version deprecated by EPSG, name change to CSRS98
    {  "CSRS.MTM-7",               "NAD83/98.MTM-7",               },	// NAD83/98 version deprecated by EPSG, name change to CSRS98
    {  "CSRS.MTM-8",               "NAD83/98.MTM-8",               },	// NAD83/98 version deprecated by EPSG, name change to CSRS98
    {  "CSRS.MTM-9",               "NAD83/98.MTM-9",               },	// NAD83/98 version deprecated by EPSG, name change to CSRS98
    {  "CSRS.SCoPQ-2",             "NAD83/98.SCoPQ-2",             },	// NAD83/98 version deprecated by EPSG, name change to CSRS98
    {  "CSRS.UTM-11N",             "NAD83/98.UTM-11N",             },	// NAD83/98 version deprecated by EPSG, name change to CSRS98
    {  "CSRS.UTM-12N",             "NAD83/98.UTM-12N",             },	// NAD83/98 version deprecated by EPSG, name change to CSRS98
    {  "CSRS.UTM-13N",             "NAD83/98.UTM-13N",             },	// NAD83/98 version deprecated by EPSG, name change to CSRS98
    {  "CSRS.UTM-17N",             "NAD83/98.UTM-17N",             },	// NAD83/98 version deprecated by EPSG, name change to CSRS98
    {  "CSRS.UTM-18N",             "NAD83/98.UTM-18N",             },	// NAD83/98 version deprecated by EPSG, name change to CSRS98
    {  "CSRS.UTM-19N",             "NAD83/98.UTM-19N",             },	// NAD83/98 version deprecated by EPSG, name change to CSRS98
    {  "CSRS.UTM-20N",             "NAD83/98.UTM-20N",             },	// NAD83/98 version deprecated by EPSG, name change to CSRS98
    {  "CSRS.UTM-21N",             "NAD83/98.UTM-21N",             },	// NAD83/98 version deprecated by EPSG, name change to CSRS98
//  {  "Pulkovo42.GK-3",           "CZ-S42-3",
//                                 "HUN-33",
//                                 "SK-S42-3",                     },	// Same, but different countries.
//  {  "DANE-33",                  "NORGE-33",
//                                 "UTM-33N",                      },	// Same, but different countries.
//  {  "DUB-OLD40",                "UTM-40N",                      },	// Slightly different ellipsoids
//  {  "ED50-UTM28",               "SPAIN-HU28",                   },	// Same, but secondary is country specific???
//  {  "ED50-UTM29",               "SPAIN-HU29",                   },	// Same, but secondary is country specific???
//  {  "ED50-UTM30",               "SPAIN-UTM",                    },	// Same, but secondary is country specific???
//  {  "ED50-UTM31",               "SPAIN-HU31",                   },	// Same, but secondary is country specific???
    {  "Massawa.UTM-37N",          "ERITREA",                      },
//  {  "ETRF89.TM-35/Fin",         "ETRF89.TM35",
//                                 "ETRS89.UTM-35N",               },	// Country specific and distinct EPSG codes.
//  {  "ETRF89.TM28",              "ETRS89.UTM-28N",               },	// Distinct EPSG Codes
//  {  "ETRF89.TM29",              "ETRS89.UTM-29N",               },	// Distinct EPSG Codes
//  {  "ETRF89.TM30",              "ETRS89.UTM-30N",               },	// Distinct EPSG Codes
//  {  "ETRF89.TM31",              "ETRS89.UTM-31N",               },	// Distinct EPSG Codes
//  {  "ETRF89.TM32",              "ETRS89.UTM-32N",               },	// Distinct EPSG Codes
//  {  "ETRF89.TM33",              "ETRS89.UTM-33N",               },	// Distinct EPSG Codes
//  {  "ETRF89.TM34",              "ETRS89.UTM-34N",               },	// Distinct EPSG Codes
//  {  "ETRF89.TM36",              "ETRS89.UTM-36N",               },	// Distinct EPSG Codes
//  {  "ETRF89.TM37",              "ETRS89.UTM-37N",               },	// Distinct EPSG Codes
//  {  "ETRF89.TM38",              "ETRS89.UTM-38N",               },	// Distinct EPSG Codes
//  {  "ETRS89.TM-35/Fin",         "ETRS89.TM35",                  },	// Distinct EPSG Codes
//  {  "EU-29",                    "F29-ED50",
//                                 "FAR-TM",                       },	// Same, but different countries.
    {  "EU-30",                    "FX-30",                        },
    {  "EU-31",                    "FX-31",                        },
    {  "EU-32",                    "FX-32",                        },
    {  "EU-37",                    "YEM-U37",                      },	// Same, but different countries.
    {  "EU-38",                    "YEM-U38",                      },	// Same, but different countries.
    {  "EUET-32",                  "NOR-32",                       },
//  {  "EUET-33",                  "G-BR",
//                                 "NOR-33",                       },
//  {  "EUET-35",                  "NOR-35",                       },
//  {  "Sapper.UTM-21S",           "FALK-E",                       },
//  {  "Sapper.UTM-20S",           "FALK-W",                       },
//  {  "FINL-KKJ3",                "FINL-YHT",                     },
//  {  "FJ-U1S",                   "UTM-1S",                       },
//  {  "FJ-U60S",                  "UTM-60S",                      },
    {  "HARN.FloridaGDL/Albers",   "FLHP",                         },
    {  "OrdinalSurvey",            "GB_GRS80",                     },
//  {  "GIBR-TM",                  "SPAIN-TM30-I",
//                                 "UTM-30N",                      },	// Same system, different geography
//  {  "Pulkovo42.GK/CM-15E",      "GK-3N",
//                                 "SK-3N",                        },	// Same sytem, different countries
//  {  "Pulkovo42.GK/CM-21E",      "Pulkovo42.GK3d/CM-21E"         },	// Different width zones
//                                 "Pulkovo95.GK3d/CM-21E",				// CRS Bust!!!
//                                 "GK-4N",								// Different country
//                                 "SK-4N",                        },	// Different country
//  {  "GK-7",                     "YEM-GK7",                      },	// Different countries
//  {  "GK-8",                     "YEM-GK8",                      },	// Different countries
//  {  "GK-9",                     "YEM-GK9",                      },	// Different countries
    {  "GRMNY-S2",                 "GK-S2",                     },
    {  "GRMNY-S3",                 "GK-S3",                     },
    {  "GRMNY-S4",                 "GK-S4",                     },
    {  "Pulkovo42.GK/CM-153E",     // "Pulkovo42.GK3d/CM-153E",			// Zone width distinction
                                   // "Pulkovo95.GK3d/CM-153E",			// CRS Bust!!!
                                   "GK42-26N",                     },
    {  "HARN.UTM-2S",              "SAMOA-HARN",                   },
    {  "HIHP-1",                   "HIHP-1-MOD",                   },
    {  "HIHP-2",                   "HIHP-2-MOD",                   },
    {  "HIHP-3",                   "HIHP-3-MOD",                   },
    {  "HIHP-4",                   "HIHP-4-MOD",                   },
    {  "HIHP-5",                   "HIHP-5-MOD",                   },
    {  "Hjorsey.UTM-26N",          "ICE-26",                       },
    {  "Hjorsey.UTM-27N",          "ICE-27",                       },
    {  "Hjorsey.UTM-28N",          "ICE-28",                       },
    {  "UTM84-49N",                "HK-49",                        },
    {  "UTM84-50N",                "HK-50",                        },
    {  "NTF.Lambert-1",            "IGN-I-Grid",                   },
    {  "NTF.Lambert-2",            "IGN-II-Grid",                  },
//  {  "IGN-IIE",                  "IGNC-II",                      },	// One is a zone, the other "whole country"
    {  "NTF.Lambert-E",            "IGN-IIE-Grid",                 },
    {  "NTF.Lambert-3",            "IGN-III-Grid",                 },
    {  "NTF.Lambert-4-Mod",        "IGN-IV-Grid",                  },
//  {  "IND-I/a",                  "PAK-I/a",                      },	// Same system, different countries
//  {  "IND-IIA/a",                "PAK-IIA/a",                    },	// Same system, different countries
    {  "IslandsNet1993.LL",        "ISN93.LL",                     },
//  {  "ISRAEL",                   "JORDAN",                       },	// Same system, different countries
    {  "JGD2K.PlnRctCS-I",         "JGD2K-01",                     },
    {  "JGD2K.PlnRctCS-II",        "JGD2K-02",                     },
    {  "JGD2K.PlnRctCS-III",       "JGD2K-03",                     },
    {  "JGD2K.PlnRctCS-IV",        "JGD2K-04",                     },
    {  "JGD2K.PlnRctCS-V",         "JGD2K-05",                     },
    {  "JGD2K.PlnRctCS-VI",        "JGD2K-06",                     },
    {  "JGD2K.PlnRctCS-VII",       "JGD2K-07",                     },
    {  "JGD2K.PlnRctCS-VIII",      "JGD2K-08",                     },
    {  "JGD2K.PlnRctCS-IX",        "JGD2K-09",                     },
    {  "JGD2K.PlnRctCS-X",         "JGD2K-10",                     },
    {  "JGD2K.PlnRctCS-XI",        "JGD2K-11",                     },
    {  "JGD2K.PlnRctCS-XII",       "JGD2K-12",                     },
    {  "JGD2K.PlnRctCS-XIII",      "JGD2K-13",                     },
    {  "JGD2K.PlnRctCS-XIV",       "JGD2K-14",                     },
    {  "JGD2K.PlnRctCS-XV",        "JGD2K-15",                     },
    {  "JGD2K.PlnRctCS-XVI",       "JGD2K-16",                     },
    {  "JGD2K.PlnRctCS-XVII",      "JGD2K-17",                     },
    {  "JGD2K.PlnRctCS-XVIII",     "JGD2K-18",                     },
    {  "JGD2K.PlnRctCS-XIX",       "JGD2K-19",                     },
//  {  "JORDAN-U36",               "TURK84-36N",
//                                 "UTM84-36N",                    },	// Same system, different countries
//  {  "JUGO-7M",                  "MACED-7M",							// Same System, different countries
//                                 "MACED-7M-S9",                  },	// Susp[ect a CRS Bust!!!
//  {  "JUGO-8M",                  "MACED-8M",							// Same System, different countries
//                                 "MACED-8M-S9",                  },	// Suspect a CRS Bust!!!
    {  "Kertau.SingaporeGrid",     "SING-G",                       },
    {  "Kertau.UTM-47N",           "MAL-W_W",                      },
    {  "Kertau.UTM-48N",           "MAL-W_E",                      },
    {  "KY83F",                    "KY1Z-FT",                        },
    {  "KY83",                     "KY1Z-M",                         },
    {  "Les-25",                   "Lo-25",                        },
    {  "Les-25-WGS",               "Lo-25-WGS",                    },
    {  "Les-27",                   "Lo-27",                        },
    {  "Les-27-WGS",               "Lo-27-WGS",                    },
//  {  "LL",                       "LL-270",
//                                 "LL-360",                       },	// Result range difference only. (DOes the CsDefCmp look at these???)
    {  "OSGB.LL",                  "LL-OSGB-MOD",                  },
    {  "LL-PSAD56",                "LLPSAD56",                     },
    {  "Pulkovo42.LL",             "LL-Pulkovo42",                 },
    {  "LL84",                     "WORLD-LL",                     },
//  {  "Lo-31",                    "Swazi-CP",                     },
//  {  "Lo-31-WGS",                "Swazi-WGS",                    },
    {  "Luzon.Philippines-I",      "PHIL-I",                       },
    {  "Luzon.Philippines-II",     "PHIL-II",                      },
    {  "Luzon.Philippines-III",    "PHIL-III",                     },
    {  "Luzon.Philippines-IV",     "PHIL-IV",                      },
    {  "Luzon.Philippines-V",      "PHIL-V",                       },
    {  "Merchich.SudMaroc/01",     "Morocco-S/01",                 },
    {  "MICHIGAN.OldCentral",      "MI27-OC",                      },
    {  "MICHIGAN.East",            "MI27-OE",                      },
    {  "MICHIGAN.West",            "MI27-OW",                      },
    {  "Minna.NigeriaEast",        "NGRA-E",                       },
    {  "Minna.NigeriaMid",         "NGRA-M",                       },
    {  "Minna.NigeriaWest",        "NGRA-W",                       },
//  {  "MYAN-E54",                 "THAI-W54",                     },
//  {  "NAD27.BLM-14N.ft",         "UTM27-14F",                    },
//  {  "NAD27.BLM-15N.ft",         "UTM27-15F",                    },
//  {  "NAD27.BLM-16N.ft",         "UTM27-16F",                    },
//  {  "NAD27.BLM-17N.ft",         "UTM27-17F",                    },
    {  "NAD83.OregonLambert",      "OR-GIS83-M",                   },
    {  "Nahrwan67.UTM-37N",        "Nahrwan67.UTM-38N",            },	// CRS Bust !!!!
    {  "Schwarzk.UTM-33S",         "NAM-U33",                      },
//  {  "Naparima55.UTM-20N",       "TRIN_TOB",                     },	// Slight datum difference
//  {  "NJ83",                     "NY83-E",                       },	// Same system, different states
    {  "NJ83F",                    "NY83-EF",                      },	// Same system, different states
    {  "NJHP",                     "NYHP-E",                       },	// Same system, different states
    {  "NJHPF",                    "NYHP-EF",                      },
//  {  "NORGE-34",                 "UTM-34N",                      },	// Defaulted ellipsoid
//  {  "NORGE-35",                 "UTM-35N",                      },	// Defaulted ellipsoid
//  {  "NTF-3P/gc.Lambert-2C",     "NTF-3P/gc.Lambert-E",          },	// One is a zone, the other the whole country
    {  "NZGD2K.Amuri",             "NZ-AM-2000",                   },
    {  "NZGD2K.Bluff",             "NZ-BLF-2000",                  },
    {  "NZGD2K.Buller",            "NZ-BUL-2000",                  },
    {  "NZGD2K.Collingwood",       "NZ-COL-2000",                  },
    {  "NZGD2K.Gawler",            "NZ-GAW-2000",                  },
    {  "NZGD2K.Grey",              "NZ-GRY-2000",                  },
    {  "NZGD2K.HawkesBay",         "NZ-HB-2000",                   },
    {  "NZGD2K.Hokitika",          "NZ-HO-2000",                   },
    {  "NZGD2K.JacksonsBay",       "NZ-JB-2000",                   },
    {  "NZGD2K.Karamea",           "NZ-KA-2000",                   },
    {  "NZGD2K.LindisPeak",        "NZ-LP-2000",                   },
    {  "NZGD2K.Marlborough",       "NZ-MA-2000",                   },
    {  "NZGD2K.MountEden",         "NZ-MTE-2000",                  },
    {  "NZGD2K.MountNicholas",     "NZ-MTN-2000",                  },
    {  "NZGD2K.MountPleasant",     "NZ-MTP-2000",                  },
    {  "NZGD2K.MountYork",         "NZ-MTY-2000",                  },
    {  "NZGD2K.Nelson",            "NZ-NL-2000",                   },
    {  "NZGD2K.NorthTaieri",       "NZ-NT-2000",                   },
    {  "NZGD2K.ObservationPnt",    "NZ-OBP-2000",                  },
    {  "NZGD2K.Okarito",           "NZ-OK-2000",                   },
    {  "NZGD2K.PovertyBay",        "NZ-PB-2000",                   },
    {  "NZGD2K.Timaru",            "NZ-TI-2000",                   },
    {  "NZTM",                     "NZ-TM",                        },
    {  "NZGD2K.Tuhirangi",         "NZ-TU-2000",                   },
    {  "NZGD2K.Wairarapa",         "NZ-WA-2000",                   },
    {  "NZGD2K.Wanganui",          "NZ-WAN-2000",                  },
    {  "OSGB-GPS-1997",            "OSGB-OSTN97",                  },
    {  "OSGB-GPS-2002",            "OSGB-OSTN02",                  },
    {  "Pulkovo42/58.Poland-1",    "Pulkovo/58.Poland-I/a",        },
/*===================================================================
   Two issues apply to the following:
   1> All of the "Pulkovo95." systems refer to the same datum as the
      "Pulkovo42." references.  Problem is that the real Pulkovo95 is
      different from Pulkovo42.  The difference is small, but there is
      a difference. [There is an interesting history behind this, so
      it is not surpizing.]
   2> In the case of the Pulkovo42 duplicates, there exists a six degree
      wide series of coordinate systems and a series of 3 degree wide
      coordinate systems.  Thus, every other one of the three degree
      series is a legimate duplicate of one of the six degree wide
      definitions.
      
      All of the Pulkovo95 definitions will need to deprecated and
      a new definition which references the actual Pulkovo 1995
      datum definition added.  This is different from the deprecation
      process which is coded below and based on this table.
===================================================================*/
//  {  "Pulkovo42.GK/CM-105E",     "Pulkovo42.GK3d/CM-105E",
//                                 "Pulkovo95.GK3d/CM-105E",       },
//  {  "Pulkovo42.GK/CM-111E",     "Pulkovo42.GK3d/CM-111E",
//                                 "Pulkovo95.GK3d/CM-111E",       },
//  {  "Pulkovo42.GK/CM-117E",     "Pulkovo42.GK3d/CM-117E",
//                                 "Pulkovo95.GK3d/CM-111E",       },
//                               "Pulkovo95.GK3d/CM-117E",       },
//  {  "Pulkovo42.GK/CM-123E",     "Pulkovo42.GK3d/CM-123E",
//                                 "Pulkovo95.GK3d/CM-123E",       },
//  {  "Pulkovo42.GK/CM-129E",     "Pulkovo42.GK3d/CM-129E",
//                                 "Pulkovo95.GK3d/CM-129E",       },
//  {  "Pulkovo42.GK/CM-135E",     "Pulkovo42.GK3d/CM-135E",
//                                 "Pulkovo95.GK3d/CM-135E",       },
//  {  "Pulkovo42.GK/CM-141E",     "Pulkovo42.GK3d/CM-141E",
//                                 "Pulkovo95.GK3d/CM-141E",       },
//  {  "Pulkovo42.GK/CM-147E",     "Pulkovo42.GK3d/CM-147E",
//                                 "Pulkovo95.GK3d/CM-147E",       },
//  {  "Pulkovo42.GK/CM-159E",     "Pulkovo42.GK3d/CM-159E",
//                                 "Pulkovo95.GK3d/CM-159E",       },
//  {  "Pulkovo42.GK/CM-165E",     "Pulkovo42.GK3d/CM-165E",
//                                 "Pulkovo95.GK3d/CM-165E",       },
//  {  "Pulkovo42.GK/CM-171E",     "Pulkovo42.GK3d/CM-171E",
//                                 "Pulkovo95.GK3d/CM-171E",       },
//  {  "Pulkovo42.GK/CM-171W",     "Pulkovo42.GK3d/CM-171W",
//                                 "Pulkovo95.GK3d/CM-171W",       },
//  {  "Pulkovo42.GK/CM-177E",     "Pulkovo42.GK3d/CM-177E",
//                                 "Pulkovo95.GK3d/CM-177E",       },
//  {  "Pulkovo42.GK/CM-177W",     "Pulkovo42.GK3d/CM-177W",
//                                 "Pulkovo95.GK3d/CM-177W",       },
//  {  "Pulkovo42.GK/CM-27E",      "Pulkovo42.GK3d/CM-27E",
//                                 "Pulkovo95.GK3d/CM-27E",        },
//  {  "Pulkovo42.GK/CM-33E",      "Pulkovo42.GK3d/CM-33E",
//                                 "Pulkovo95.GK3d/CM-33E",        },
//  {  "Pulkovo42.GK/CM-39E",      "Pulkovo42.GK3d/CM-39E",
//                                 "Pulkovo95.GK3d/CM-39E",        },
//  {  "Pulkovo42.GK/CM-45E",      "Pulkovo42.GK3d/CM-45E",
//                                 "Pulkovo95.GK3d/CM-45E",        },
//  {  "Pulkovo42.GK/CM-51E",      "Pulkovo42.GK3d/CM-51E",
//                                 "Pulkovo95.GK3d/CM-51E",        },
//  {  "Pulkovo42.GK/CM-57E",      "Pulkovo42.GK3d/CM-57E",
//                                 "Pulkovo95.GK3d/CM-57E",        },
//  {  "Pulkovo42.GK/CM-63E",      "Pulkovo42.GK3d/CM-63E",
//                                 "Pulkovo95.GK3d/CM-63E",        },
//  {  "Pulkovo42.GK/CM-69E",      "Pulkovo42.GK3d/CM-69E",
//                                 "Pulkovo95.GK3d/CM-69E",        },
//  {  "Pulkovo42.GK/CM-75E",      "Pulkovo42.GK3d/CM-75E",
//                                 "Pulkovo95.GK3d/CM-75E",        },
//  {  "Pulkovo42.GK/CM-81E",      "Pulkovo42.GK3d/CM-81E",
//                                 "Pulkovo95.GK3d/CM-81E",        },
//  {  "Pulkovo42.GK/CM-87E",      "Pulkovo42.GK3d/CM-87E",
//                                 "Pulkovo95.GK3d/CM-87E",        },
//  {  "Pulkovo42.GK/CM-93E",      "Pulkovo42.GK3d/CM-93E",
//                                 "Pulkovo95.GK3d/CM-93E",        },
//  {  "Pulkovo42.GK/CM-99E",      "Pulkovo42.GK3d/CM-99E",
//                                 "Pulkovo95.GK3d/CM-99E",        },
//  {  "Pulkovo42.GK3d-10",        "Pulkovo95.GK3d-10",            },
//  {  "Pulkovo42.GK3d-11",        "Pulkovo95.GK3d-11",            },
//  {  "Pulkovo42.GK3d-12",        "Pulkovo95.GK3d-12",            },
//  {  "Pulkovo42.GK3d-13",        "Pulkovo95.GK3d-13",            },
//  {  "Pulkovo42.GK3d-14",        "Pulkovo95.GK3d-14",            },
//  {  "Pulkovo42.GK3d-15",        "Pulkovo95.GK3d-15",            },
//  {  "Pulkovo42.GK3d-16",        "Pulkovo95.GK3d-16",            },
//  {  "Pulkovo42.GK3d-17",        "Pulkovo95.GK3d-17",            },
//  {  "Pulkovo42.GK3d-18",        "Pulkovo95.GK3d-18",            },
//  {  "Pulkovo42.GK3d-19",        "Pulkovo95.GK3d-19",            },
//  {  "Pulkovo42.GK3d-20",        "Pulkovo95.GK3d-20",            },
//  {  "Pulkovo42.GK3d-21",        "Pulkovo95.GK3d-21",            },
//  {  "Pulkovo42.GK3d-22",        "Pulkovo95.GK3d-22",            },
//  {  "Pulkovo42.GK3d-23",        "Pulkovo95.GK3d-23",            },
//  {  "Pulkovo42.GK3d-24",        "Pulkovo95.GK3d-24",            },
//  {  "Pulkovo42.GK3d-25",        "Pulkovo95.GK3d-25",            },
//  {  "Pulkovo42.GK3d-26",        "Pulkovo95.GK3d-26",            },
//  {  "Pulkovo42.GK3d-27",        "Pulkovo95.GK3d-27",            },
//  {  "Pulkovo42.GK3d-28",        "Pulkovo95.GK3d-28",            },
//  {  "Pulkovo42.GK3d-29",        "Pulkovo95.GK3d-29",            },
//  {  "Pulkovo42.GK3d-30",        "Pulkovo95.GK3d-30",            },
//  {  "Pulkovo42.GK3d-31",        "Pulkovo95.GK3d-31",            },
//  {  "Pulkovo42.GK3d-32",        "Pulkovo95.GK3d-32",            },
//  {  "Pulkovo42.GK3d-33",        "Pulkovo95.GK3d-33",            },
//  {  "Pulkovo42.GK3d-34",        "Pulkovo95.GK3d-34",            },
//  {  "Pulkovo42.GK3d-35",        "Pulkovo95.GK3d-35",            },
//  {  "Pulkovo42.GK3d-36",        "Pulkovo95.GK3d-36",            },
//  {  "Pulkovo42.GK3d-37",        "Pulkovo95.GK3d-37",            },
//  {  "Pulkovo42.GK3d-38",        "Pulkovo95.GK3d-38",            },
//  {  "Pulkovo42.GK3d-39",        "Pulkovo95.GK3d-39",            },
//  {  "Pulkovo42.GK3d-40",        "Pulkovo95.GK3d-40",            },
//  {  "Pulkovo42.GK3d-41",        "Pulkovo95.GK3d-41",            },
//  {  "Pulkovo42.GK3d-42",        "Pulkovo95.GK3d-42",            },
//  {  "Pulkovo42.GK3d-43",        "Pulkovo95.GK3d-43",            },
//  {  "Pulkovo42.GK3d-44",        "Pulkovo95.GK3d-44",            },
//  {  "Pulkovo42.GK3d-45",        "Pulkovo95.GK3d-45",            },
//  {  "Pulkovo42.GK3d-46",        "Pulkovo95.GK3d-46",            },
//  {  "Pulkovo42.GK3d-47",        "Pulkovo95.GK3d-47",            },
//  {  "Pulkovo42.GK3d-48",        "Pulkovo95.GK3d-48",            },
//  {  "Pulkovo42.GK3d-49",        "Pulkovo95.GK3d-49",            },
//  {  "Pulkovo42.GK3d-50",        "Pulkovo95.GK3d-50",            },
//  {  "Pulkovo42.GK3d-51",        "Pulkovo95.GK3d-51",            },
//  {  "Pulkovo42.GK3d-52",        "Pulkovo95.GK3d-52",            },
//  {  "Pulkovo42.GK3d-53",        "Pulkovo95.GK3d-53",            },
//  {  "Pulkovo42.GK3d-54",        "Pulkovo95.GK3d-54",            },
//  {  "Pulkovo42.GK3d-55",        "Pulkovo95.GK3d-55",            },
//  {  "Pulkovo42.GK3d-56",        "Pulkovo95.GK3d-56",            },
//  {  "Pulkovo42.GK3d-57",        "Pulkovo95.GK3d-57",            },
//  {  "Pulkovo42.GK3d-58",        "Pulkovo95.GK3d-58",            },
//  {  "Pulkovo42.GK3d-59",        "Pulkovo95.GK3d-59",            },
//  {  "Pulkovo42.GK3d-61",        "Pulkovo95.GK3d-61",            },
//  {  "Pulkovo42.GK3d-62",        "Pulkovo95.GK3d-62",            },
//  {  "Pulkovo42.GK3d-63",        "Pulkovo95.GK3d-63",            },
//  {  "Pulkovo42.GK3d-64",        "Pulkovo95.GK3d-64",            },
//  {  "Pulkovo42.GK3d-7",         "Pulkovo95.GK3d-7",             },
//  {  "Pulkovo42.GK3d-8",         "Pulkovo95.GK3d-8",             },
//  {  "Pulkovo42.GK3d-9",         "Pulkovo95.GK3d-9",             },
//  {  "Pulkovo42.GK3d/CM-102E",   "Pulkovo95.GK3d/CM-102E",       },
//  {  "Pulkovo42.GK3d/CM-108E",   "Pulkovo95.GK3d/CM-108E",       },
//  {  "Pulkovo42.GK3d/CM-114E",   "Pulkovo95.GK3d/CM-114E",       },
//  {  "Pulkovo42.GK3d/CM-120E",   "Pulkovo95.GK3d/CM-120E",       },
//  {  "Pulkovo42.GK3d/CM-126E",   "Pulkovo95.GK3d/CM-126E",       },
//  {  "Pulkovo42.GK3d/CM-132E",   "Pulkovo95.GK3d/CM-132E",       },
//  {  "Pulkovo42.GK3d/CM-138E",   "Pulkovo95.GK3d/CM-138E",       },
//  {  "Pulkovo42.GK3d/CM-144E",   "Pulkovo95.GK3d/CM-144E",       },
//  {  "Pulkovo42.GK3d/CM-150E",   "Pulkovo95.GK3d/CM-150E",       },
//  {  "Pulkovo42.GK3d/CM-156E",   "Pulkovo95.GK3d/CM-156E",       },
//  {  "Pulkovo42.GK3d/CM-162E",   "Pulkovo95.GK3d/CM-162E",       },
//  {  "Pulkovo42.GK3d/CM-168E",   "Pulkovo95.GK3d/CM-168E",       },
//  {  "Pulkovo42.GK3d/CM-168W",   "Pulkovo95.GK3d/CM-168W",       },
//  {  "Pulkovo42.GK3d/CM-174E",   "Pulkovo95.GK3d/CM-174E",       },
//  {  "Pulkovo42.GK3d/CM-174W",   "Pulkovo95.GK3d/CM-174W",       },
//  {  "Pulkovo42.GK3d/CM-180E",   "Pulkovo95.GK3d/CM-180E",       },
//  {  "Pulkovo42.GK3d/CM-24E",    "Pulkovo95.GK3d/CM-24E",        },
//  {  "Pulkovo42.GK3d/CM-30E",    "Pulkovo95.GK3d/CM-30E",        },
//  {  "Pulkovo42.GK3d/CM-36E",    "Pulkovo95.GK3d/CM-36E",        },
//  {  "Pulkovo42.GK3d/CM-42E",    "Pulkovo95.GK3d/CM-42E",        },
//  {  "Pulkovo42.GK3d/CM-48E",    "Pulkovo95.GK3d/CM-48E",        },
//  {  "Pulkovo42.GK3d/CM-54E",    "Pulkovo95.GK3d/CM-54E",        },
//  {  "Pulkovo42.GK3d/CM-60E",    "Pulkovo95.GK3d/CM-60E",        },
//  {  "Pulkovo42.GK3d/CM-66E",    "Pulkovo95.GK3d/CM-66E",        },
//  {  "Pulkovo42.GK3d/CM-72E",    "Pulkovo95.GK3d/CM-72E",        },
//  {  "Pulkovo42.GK3d/CM-78E",    "Pulkovo95.GK3d/CM-78E",        },
//  {  "Pulkovo42.GK3d/CM-84E",    "Pulkovo95.GK3d/CM-84E",        },
//  {  "Pulkovo42.GK3d/CM-90E",    "Pulkovo95.GK3d/CM-90E",        },
//  {  "Pulkovo42.GK3d/CM-96E",    "Pulkovo95.GK3d/CM-96E",        },
    {  "SAD69.UTM-17S",            "SA69-17S",                     },
    {  "SAD69.UTM-18N",            "SA69-18N",                     },
    {  "SAD69.UTM-18S",            "SA69-18S",                     },
    {  "SAD69.UTM-19N",            "SA69-19N",                     },
    {  "SAD69.UTM-19S",            "SA69-19S",
                                   "SGB-19",
                                   "UTM-SGB",                      },
    {  "SAD69.UTM-20N",            "SA69-20N",                     },
    {  "SAD69.UTM-20S",            "SA69-20S",
                                   "SGB-20",                       },
    {  "SAD69.UTM-21N",            "SA69-21N",                     },
    {  "SAD69.UTM-21S",            "SA69-21S",
                                   "SGB-21",                       },
    {  "SAD69.UTM-22N",            "SA69-22N",                     },
    {  "SAD69.UTM-22S",            "SA69-22S",
                                   "SGB-22",                       },
    {  "SAD69.UTM-23S",            "SA69-23S",
                                   "SGB-23",                       },
    {  "SAD69.UTM-24S",            "SA69-24S",
                                   "SGB-24",                       },
    {  "SAD69.UTM-25S",            "SA69-25S",
                                   "SGB-25",                       },
//  {  "SPAIN-TM28-I",             "UTM-28N",                      },	// Slight difference in ellipsoid reference.
    {  "Zanderij.UTM-21N",         "SUR",                          },
    {  "Zanderij.SurinameTM",      "SUR-NEW",                      },
    {  "Zanderij.SurinameTMOld",   "SUR-OLD",                      },
//  {  "SW-25GONV",                "SW-NAT90",                     },	// Slight difference in the datum
    {  "SW-25GONW",                "SW-NAT",                       },
    {  "TM1965.IrishGrid",         "TM65g.IrishNationalGrid",      },	// Same, the TM65g entry was deprecfated by EPSG.
    {  "TURK84-35N",               "UTM84-35N",                    },
    {  "TURK84-37N",               "UTM84-37N",                    },
    {  "TURK84-38N",               "UTM84-38N",                    },
    {  "WGS84.UPSNorth",           "UPS-N",                        },
    {  "WGS84.UPSSouth",           "UPS-S",                        },
//  {  "UTM-58S",                  "VAN-58",                       },	// Same, but a user could concievably look for Vanuatu!
//  {  "UTM-59S",                  "VAN-59",                       },	// Same, but a user could concievably look for Vanuatu!
//  {  "UTM-5HI",                  "UTM-5N",                       },	// Slight difference in the ellipsoid
//  {  "UTM-6HI",                  "UTM-6N",                       },	// Slight difference in the ellipsoid
    {  "UTM27-1",                  "UTM27-1N",                     },	// UTM27-1 is consistent with other UTM names.  Not in name mapper.
    {  "UTM27-2",                  "UTM27-2N",                     },	// UTM27-2 is consistent with other UTM names. Not in name mapper.
    {  "UTM84-18N",                "VEN-18N",                      },
    {  "UTM84-19N",                "VEN-19N",                      },
    {  "UTM84-20N",                "VEN-20N",                      },
//  {  "VEN-18",                   "VEN-19",                       },	// CRS Bust!!! VEN-19 is wrong.
    {  "WGS84.PlateCarree",        "WORLD-EQDIST-CYL",             },
//  {  "Xian80.GK/CM-105E",        "Xian80.GK3d/CM-105E",          },	// The GK3d defs are from a three degree wide
//  {  "Xian80.GK/CM-111E",        "Xian80.GK3d/CM-111E",          },	// series of coordinate systems covering the
//  {  "Xian80.GK/CM-117E",        "Xian80.GK3d/CM-117E",          },	// same geography as the 6 degree wide zones.
//  {  "Xian80.GK/CM-123E",        "Xian80.GK3d/CM-123E",          },
//  {  "Xian80.GK/CM-129E",        "Xian80.GK3d/CM-129E",          },
//  {  "Xian80.GK/CM-135E",        "Xian80.GK3d/CM-135E",          },
//  {  "Xian80.GK/CM-75E",         "Xian80.GK3d/CM-75E",           },
//  {  "Xian80.GK/CM-81E",         "Xian80.GK3d/CM-81E",           },
//  {  "Xian80.GK/CM-87E",         "Xian80.GK3d/CM-87E",           },
//  {  "Xian80.GK/CM-93E",         "Xian80.GK3d/CM-93E",           },
//  {  "Xian80.GK/CM-99E",         "Xian80.GK3d/CM-99E",           },
    {  "XY-CA",                    "XY-CF",                        },
    {  "XY-DAM",                   "XY-DK",                        },	// Same, different spelling of the unit name.
//  {  "XY-IIN",                   "XY-IN",                        },	// CRS Bust? NO, the difference is very small but they are different.
    {  "XY-NM",                    "XY-KT",                        },	// KT is the knot, not really a linear unit.
//  {  "XY-PE",                    "XY-PO",
//                                 "XY-RD",                        },	// Three different names for three units which are identical.
    {  "",                         "",                             }
};
