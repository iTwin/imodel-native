/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#define ELLIPSOID_NAME_LENGTH_LIMIT 50
#define ELLIPSOID_MAP_NB_COLS       6

char EllipsoidMap[][ELLIPSOID_MAP_NB_COLS][ELLIPSOID_NAME_LENGTH_LIMIT] =
{       //  CST                     Oracle(Legacy)                          EPSG            ESRI-ID          ESRI-WKT                                     Additional    Name
	{   "APL4-5"                 , ""                                   , ""            , ""             , ""                                  , "" },              //  A.P.L./4.5
	{   "AIRY30"                 , "Oracle:Airy 1830"                   , "EPSG:7001"   , ""             , "Airy_1830"                         , "" },              //          Airy - 1830
	{   "AIRY49"                 , ""                                   , ""            , ""             , ""                                  , "" },              //  Airy - 1848
	{   "AIRY-MOD"               , "Oracle:Airy 1830(Ireland 1965)"     , "EPSG:7002"   , ""             , "Airy_Modified"                     , "" },              //              Airy Modified 1849
	{   "ANS66"                  , "Oracle:Australian"                  , "EPSG:7003"   , ""             , "Australian"                        , "" },              //           Australian National Spheroid of 1966
	{   "ATS77"                  , ""                                   , "EPSG:7041"   , ""             , "ATS_1977"                          , "" },              //         Average Terrestrial System 1977
	{   "AUSSIE52"               , ""                                   , ""            , ""             , ""                                  , "" },              //  Australian - AIG - 1952
	{   "AUSSIE"                 , ""                                   , ""            , ""             , ""                                  , "" },              //  Obsolete, use ANS66
	{   "BESSEL"                 , "Oracle:Bessel 1841"                 , "EPSG:7004"   , ""             , "Bessel_1841"                       , "Bessel 1841" },   //            Bessel - 1841
	{   "Bessel-Norway"          , "Oracle:Bessel 1841 (NGO 1948)"      , ""            , ""             , ""                                  , "" },              // Norwegian National Ellipsoid (NGO)
	{   "BESL-MOD"               , ""                                   , "EPSG:7005"   , ""             , "Bessel_Modified"                   , "" },              //                Bessel Modified
	{   "BESL-NMB"               , "Oracle:Bessel 1841 (Schwarzeck)"    , "EPSG:7006"   , ""             , "Bessel_Namibia"                    , "" },              //               Bessel - Namibia
	{   "BESL-TRI"               , ""                                   , ""            , ""             , ""                                  , "" },              //  Bessel Triangulation - Sweden
	{   "BeslNmb-GLM"            , ""                                   , "EPSG:7046"   , ""             , ""                                  , "" },              // Bessel Namibia (GLM)
	{   "Borneo"                 , ""                                   , ""            , ""             , ""                                  , "" },              //  Borneo, 4mm larger than EVRST-TM
	{   "BPCNC"                  , ""                                   , ""            , ""             , ""                                  , "" },              //  Sphere having volume equal to International ellipsoid
	{   "Clarke1866/IntnlFt"     , ""                                   , "EPSG:7055"   , ""             , ""                                  , "" },              // Clarke 1880 (international foot) (Differs from meter version)
	{   "Clarke66AuthalicSphere" , "Oracle:Sphere (6370997m)"           , "EPSG:7052"   , ""             , "Sphere_Clarke_1866_Authalic"       , "" },              //                Clarke 1866 Authalic Sphere
	{   "CLRK22"                 , "Oracle:Clarke 1880 (Merchich)"      , "EPSG:7014"   , ""             , "Clarke_1880_SGA"                   , "" },              //     Clarke 1922 - IGN / Clarke 1880 (SGA 1922) / Mermich
	{   "CLRK58"                 , "Oracle:Clarke 1858"                 , ""            , ""             , "Clarke_1858"                       , "Clarke 1858" },   // Clarke - 1858
	{   ""                       , ""                                   , "EPSG:7007"   , ""             , ""                                  , "" },              //    Clarke - 1858 (Varies from previous)
	{   ""                       , ""                                   , ""            , ""             , ""                                  , "" },              // 
	{   "CLRK66"                 , "Oracle:Clarke 1866"                 , "EPSG:7008"   , ""             , "Clarke_1866"                       , "Clarke 1866" },   //                   Clarke - 1866
	{   "CLRK80"                 , "Oracle:Clarke 1880"                 , "EPSG:7055"   , ""             , "Clarke_1880"                       , "Clarke 1880" },              //                   Clarke - 1880
	{   "Clrk80-Strasser"        , "Oracle:Clarke 1880 (Jamaica)"       , "EPSG:7034"   , ""             , ""                                  , "" },              //   Clarke 1880 (a little different from previous)
	{   "CLRK85"                   ""                                   , ""            , ""             , ""                                  , "" },              //  Clarke 1885
	{   "CLRK-ARC"               , "Oracle:Clarke 1880 (Arc 1950)"      , "EPSG:7013"   , ""             , "Clarke_1880_Arc"                   , "" },              //                      Clarke ARC
	{   "CLRK-IGN"               , "Oracle:Clarke 1880 (IGN)"           , "EPSG:7011"   , ""             , "Clarke_1880_IGN"                   , "Clarke 1880 (IGN)" },   //                      Institut Geographique National (France), Clarke 1880
	{   "CLRK-PAL"               , "Oracle:Clarke 1880 (Palestine)"     , "EPSG:7010"   , ""             , "Clarke_1880_Benoit"                , "" },              // Clarke Palestine / Clarke 1880 (Benoit)
	{   "CLRK-RGS"               , ""                                   , "EPSG:7012"   , ""             , "Clarke_1880_RGS"                   , "" },              //               Clark 1880 - RGS
	{   "CLRKS"                  , ""                                   , ""            , ""             , ""                                  , "" },              // Clarke - 1866, Spherical
	{   "DANEMARK"               , ""                                   , "EPSG:7051"   , ""             , "Danish_1876"                       , "" },              //            Danemark
	{   "DELA1810"               , ""                                   , ""            , ""             , ""                                  , "" },              //       Delambre, 1810
	{   "DELA-MOD"               , ""                                   , ""            , ""             , ""                                  , "" },              // Delambre Modified - Hydro
	{   "EVEREST"                , "Oracle:Everest"                     , "EPSG:7015"   , ""             , "Everest_Adjustment_1937"           , "" },              // Everest Indian - 1830 / Everest 1830 (1937 Adjustment)
	{   "Everest1830"            , ""                                   , "EPSG:7042"   , ""             , "Everest_1830"                      , "" },              // Everest (1830 Definition)
	{   "Everest1830Def62"       , "Oracle:Everest (Kalianpur)"         , "EPSG:7044"   , ""             , "Everest_Definition_1962"           , "" },              // Everest 1830 (1962 Definition)
	{   "Everest1830/67"         , ""                                   , "EPSG:7016"   , ""             , "Everest_Definition_1967"           , "" },              // Everest 1830 (1967 Definition)
	{   "Everest1830Def75"       , ""                                   , "EPSG:7045"   , ""             , "Everest_Definition_1975"           , "" },              // Everest 1830 (1975 Definition)
	{   "Everest1969/RSO"        , ""                                   , "EPSG:7056"   , "EPSG:107006"  , "Everest_Modified_1969"             , "" },              // Everest 1830 (RSO 1969)
	{   "EVRST-IM"               , ""                                   , ""            , ""             , ""                                  , "" },              // Everest Imperial - 1830
	{   "EVRST-MD"               , "Oracle:Everest (Kertau)"            , "EPSG:7018"   , ""             , "Everest_1830_Modified"             , "" },              // Everest - Modified
	{   "EVRST-TM"               , "Oracle:Everest (Timbalai)"          , ""            , ""             , ""                                  , "" },              // Everest - Timbalai (same as Everest Imperial)
	{   "FSHR1960"               , "Oracle:Fischer 1960 (Mercury)"      , ""            , "EPSG:107002"  , ""                                  , "" },              // Fischer - 1960 (Mercury)
	{   "FSHR60MD"               , "Oracle:Fischer 1960 (South Asia)"   , ""            , "EPSG:107004"  , ""                                  , "" },              // Modified Fischer - 1960 (South Asia)
	{   "FSHR1968"               , "Oracle:Fischer 1968"                , ""            , "EPSG:107003"  , ""                                  , "" },              // Fischer - 1968
	{   "GEM-10C"                , ""                                   , "EPSG:7031"   , ""             , "GEM_10C"                           , "" },              // GEM 10C
	{   "GHANA-WO"               , ""                                   , ""            , ""             , ""                                  , "" },              // Ghana -- Labeled "War Office", given in feet, assumed IFOOT
	{   "GRS1967"                , "Oracle:GRS 67"                      , "EPSG:7036"   , ""             , "GRS_1967"                          , "" },              // Geodetic Reference System, 1967
	{   "GRS1980"                , "Oracle:GRS 80"                      , "EPSG:7019"   , "GRS 1980"     , "GRS_1980"                          , "GRS 80" },  // Geodetic Reference System of 1980
	{   "GRS80SphereAuthalic"    , ""                                   , "EPSG:7048"   , "EPSG:7047"    , "GRS 1980 Authalic Sphere (EPSG ID 7047)"  , "GRS 1980 Authalic Sphere" },              // GRS 1980 Authalic Sphere
	{   "HEIS-29"                , ""                                   , ""            , ""             , ""                                  , "" },              // Heiskanen, 1929
	{   "HLMRT06"                , "Oracle:Helmert 1906"                , "EPSG:7020"   , ""             , "Helmert_1906"                      , "" },              // Helmert - 1906 (aka 1907)
	{   "HOUGH"                  , "Oracle:Hough"                       , "EPSG:7053"   , "EPSG:107005"  , "Hough_1960"                        , "" },              // Hough 1960
	{   "HAYFORD"                , "Oracle:Hayford"                     , ""            , ""             , ""                                  , "" },              // Hayford - 1924 (aka 1909), same as International 1924
	{   "HOLLAND"                , ""                                   , ""            , ""             , ""                                  , "" },              // Holland
	{   ""                       , ""                                   , "EPSG:7058"   , ""             , ""                                  , "" },              // Hughes 1980
	{   ""                       , "Oracle:IAG 75"                      , ""            , ""             , ""                                  , "" },              //
	{   "IndonesianNtl"          , "Oracle:Indonesian"                  , "EPSG:7021"   , ""             , "Indonesian"                        , "" },              // Indonesian National Spheroid
	{   ""                       , ""                                   , "EPSG:7057"   , ""             , ""                                  , "" },              // International Authalic Sphere
	{   "INTNL"                  , "Oracle:International 1924"          , "EPSG:7022"   , ""             , "International_1924"                , "International 1924" },              // International - 1924
	{   ""                       , ""                                   , "EPSG:7023"   , ""             , ""                                  , "" },              // International - 1967
	{   "IUGG-67"                , ""                                   , ""            , ""             , ""                                  , "" },              // IUGG Reference Ellipsoid, 1967
	{   "IUGG-75"                , ""                                   , ""            , ""             , ""                                  , "" },              // IUGG Reference Ellipsoid, 1975
	{   "JEFF-48"                , ""                                   , ""            , ""             , ""                                  , "" },              // Jeffreys, 1948
	{   "KAULA1961"              , ""                                   , ""            , ""             , ""                                  , "" },              // Kaula 1961 Ellipsoid
	{   "KRASOV"                 , "Oracle:Krassovsky"                  , "EPSG:7024"   , ""             , "Krasovsky_1940"                    , "" },              // Krassovsky - 1940/1948
	{   " "                      , "Oracle:Mars"                        , ""            , ""             , ""                                  , "" },              // Ellipsoid for planet Mars
	{   " "                      , "Oracle:MERIT 83"                    , ""            , ""             , ""                                  , "" },              //
	{   "MICHIGAN"               , "Oracle:Clarke 1866 (Michigan)"      , "EPSG:7009"   , "Clarke 1866 (Michigan)"   , "Clarke_1866_Michigan"  , "Clarke 1866 Michigan" },   // Michigan - Based on Clarke 1866 + 800 feet.
	{   ""                       , "Oracle:New International 1967"      , ""            , ""             , ""                                  , "" },              //
	{   "NWL-10D"                , "Oracle:NWL 10D"                     , ""            , ""             , ""                                  , "" },              // NWL-10D
	{   "NWL-9D"                 , ""                                   , ""            , ""             , "NWL_9D"                            , "" },              // NWL-9D
	{   ""                       , "Oracle:NWL 9D"                      , "EPSG:7025"   , ""             , ""                                  , "" },              // NWL-9D (EPSG/Oracle version differs significantly)
	{   "NZGD49"                 , ""                                   , ""            , ""             , ""                                  , "" },              // New Zealand Geodetic Datum of 1949 (aka International 1924)
	{   "OSU86F"                 , "Oracle:OSU86F"                      , "EPSG:7032"   , ""             , "OSU_86F"                           , "" },              // OSU86F
	{   "OSU91A"                 , "Oracle:OSU91A"                      , "EPSG:7033"   , ""             , "OSU_91A"                           , "" },              // OSU91A
	{   "PLESSIS"                , "Oracle:Plessis 1817"                , "EPSG:7027"   , ""             , "Plessis_1817"                      , "" },              // Plessis, 1817
	{   "PZ-90"                  , ""                                   , "EPSG:7054"   , ""             , ""                                  , "" },              // PZ-90
	{   "SA1969"                 , "Oracle:South American 1969"         , "EPSG:7050"   , ""             , "GRS_1967_Truncated"                , "" },              // South American 1969 / GRS67 Truncated
	{   "SPHERE"                 , ""                                   , ""            , ""             , ""                                  , "" },              // Sphere of radius 6370997
	{   "SPHERE-1"               , ""                                   , "EPSG:7035"   , ""             , "Sphere"                            , "" },              // Sphere of radius 6371000.0
	{   "STRU1860"               , "Oracle:Struve 1860"                 , "EPSG:7028"   , ""             , "Struve_1860"                       , "" },              // Struve, 1860
	{   "SVANBERG"               , ""                                   , ""            , ""             , ""                                  , "" },              // Svanberg
	{   "UNITE"                  , ""                                   , ""            , ""             , ""                                  , "" },              // Unit ellipsoid, testing only.  Eccentricity same as Clarke 66
	{   "UNITS"                  , "Oracle:UnitSphere"                  , ""            , ""             , ""                                  , "" },              // Unit sphere, testing only
	{   "UNITS3"                 , ""                                   , ""            , ""             , ""                                  , "" },              // 3.0 Unit sphere, testing only
	{   "T-BPCNC"                , ""                                   , ""            , ""             , ""                                  , "" },              // Sphere for testing Bipolar Oblique Conformal COnic
	{   "WALB"                   , "Oracle:Walbeck"                     , ""            , "EPSG:107007"  , ""                                  , "" },              // Walbeck
	{   "WAR-OFC"                , "Oracle:War Office"                  , ""            , ""             , "War_Office"                        , "" },              // War Office, McCaw
	{   "WarOffice"              , ""                                   , "EPSG:7029"   , ""             , ""                                  , "" },              // War Office
	{   "WGS60"                  , "Oracle:WGS 60"                      , ""            , ""             , ""                                  , "" },              // World Geodetic System of 1960
	{   "WGS66"                  , "Oracle:WGS 66"                      , ""            , "EPSG:107001"  , ""                                  , "" },              // World Geodetic System of 1966 / NWL 8D
	{   "WGS67"                  , ""                                   , ""            , ""             , ""                                  , "" },              // World Geodetic System of 1967, Lucerne
	{   "WGS72"                  , "Oracle:WGS 72"                      , "EPSG:7043"   , ""             , "WGS_1972"                          , "" },              // World Geodetic System of 1972
	{   "WGS84"                  , "Oracle:WGS 84"                      , "EPSG:7030"   , "WGS 84"       , "WGS_1984"                          , "WGS 84 (MAPINFO Datum 0)"},              // World Geodetic System of 1984
	{   "Xian80"                 , ""                                   , "EPSG:7049"   , ""             , "Xian_1980"                         , "" }               // Xian 1980
};



char ListOfKnownInvalidDatums[][30] = 
{
    "EPSG:6001",       // Not specified (based on Airy 1830 ellipsoid)
    "EPSG:6002",       // Not specified (based on Airy Modified 1849 ellipsoid)
    "EPSG:6003",       // Not specified (based on Australian National Spheroid)
    "EPSG:6004",       // Not specified (based on Bessel 1841 ellipsoid)
    "EPSG:6005",       // Not specified (based on Bessel Modified ellipsoid)
    "EPSG:6006",       // Not specified (based on Bessel Namibia ellipsoid)
    "EPSG:6007",       // Not specified (based on Clarke 1858 ellipsoid)
    "EPSG:6008",       // Not specified (based on Clarke 1866 ellipsoid)
    "EPSG:6009",       // Not specified (based on Clarke 1866 Michigan ellipsoid)
    "EPSG:6010",       // Not specified (based on Clarke 1880 (Benoit) ellipsoid)
    "EPSG:6011",       // Not specified (based on Clarke 1880 (IGN) ellipsoid)
    "EPSG:6012",       // Not specified (based on Clarke 1880 (RGS) ellipsoid)
    "EPSG:6013",       // Not specified (based on Clarke 1880 (Arc) ellipsoid)
    "EPSG:6014",       // Not specified (based on Clarke 1880 (SGA 1922) ellipsoid)
    "EPSG:6015",       // Not specified (based on Everest 1830 (1937 Adjustment) ellipsoid)
    "EPSG:6016",       // Not specified (based on Everest 1830 (1967 Definition) ellipsoid)
    "EPSG:6018",       // Not specified (based on Everest 1830 Modified ellipsoid)
    "EPSG:6019",       // Not specified (based on GRS 1980 ellipsoid)
    "EPSG:6020",       // Not specified (based on Helmert 1906 ellipsoid)
    "EPSG:6021",       // Not specified (based on Indonesian National Spheroid)
    "EPSG:6022",       // Not specified (based on International 1924 ellipsoid)
    "EPSG:6024",       // Not specified (based on Krassowsky 1940 ellipsoid)
    "EPSG:6025",       // Not specified (based on NWL 9D ellipsoid)
    "EPSG:6027",       // Not specified (based on Plessis 1817 ellipsoid)
    "EPSG:6028",       // Not specified (based on Struve 1860 ellipsoid)
    "EPSG:6029",       // Not specified (based on War Office ellipsoid)
    "EPSG:6030",       // Not specified (based on WGS 84 ellipsoid)
    "EPSG:6031",       // Not specified (based on GEM 10C ellipsoid)
    "EPSG:6032",       // Not specified (based on OSU86F ellipsoid)
    "EPSG:6033",       // Not specified (based on OSU91A ellipsoid)
    "EPSG:6034",       // Not specified (based on Clarke 1880 ellipsoid)
    "EPSG:6035",       // Not specified (based on Authalic Sphere)
    "EPSG:6036",       // Not specified (based on GRS 1967 ellipsoid)
    "EPSG:6041",       // Not specified (based on Average Terrestrial System 1977 ellipsoid)
    "EPSG:6042",       // Not specified (based on Everest (1830 Definition) ellipsoid)
    "EPSG:6043",       // Not specified (based on WGS 72 ellipsoid)
    "EPSG:6044",       // Not specified (based on Everest 1830 (1962 Definition) ellipsoid)
    "EPSG:6045",       // Not specified (based on Everest 1830 (1975 Definition) ellipsoid)
    "EPSG:6047",       // Not specified (based on GRS 1980 Authalic Sphere)
    "EPSG:6052",       // Not specified (based on Clarke 1866 Authalic Sphere)
    "EPSG:6053",       // Not specified (based on International 1924 Authalic Sphere)
    "EPSG:6054",       // Not specified (based on Hughes 1980 ellipsoid)
    "EPSG:6157",       // Mount Dillon
    "EPSG:6162",       // Korean Datum 1985
    "EPSG:6174",       // Sierra Leone Colony
    "EPSG:6185",       // Madeira 1936
    "EPSG:6197",       // Garoua
    "EPSG:6198",       // Kousseri
    "EPSG:6199",       // Egypt 1930
    "EPSG:6200",       // Pulkovo 1995
    "EPSG:6206",       // Agadez
    "EPSG:6215",       // Reseau National Belge 1950
    "EPSG:6235",       // Guyanne Francaise
    "EPSG:6241",       // Jamaica 1875
    "EPSG:6243",       // Kalianpur 1880
    "EPSG:6249",       // Lake
    "EPSG:6252",       // Lome
    "EPSG:6276",       // NSWC 9Z-2
    "EPSG:6278",       // OSGB 1970 SN
    "EPSG:6279",       // OS (SN) 1980
    "EPSG:6280",       // Padang
    "EPSG:6286",       // Qatar 1948
    "EPSG:6288",       // Loma Quintana
    "EPSG:6295",       // Serindung
    "EPSG:6303",       // Trucial Coast 1948
    "EPSG:6306",       // Bern 1938
    "EPSG:6308",       // Stockholm 1938
    "EPSG:6600",       // Anguilla 1957
    "EPSG:6671",       // Voirol 1879
    "EPSG:6695",       // Katanga 1955
    "EPSG:6696",       // Kasai 1953 
    "EPSG:6697",       // IGC 1962 Arc of the 6th Parallel South
    "EPSG:6700",       // IGN Astro 1960
    "EPSG:6703",       // Missao Hidrografico Angola y Sao Tome (Mhast) 1951
    "EPSG:6704",       // Mhast (onshore)
    "EPSG:6704",       // Mhast (offshore)
    "EPSG:6738",       // Hong Kong 1963
    "EPSG:6741",       // Faroe Datum 1954
    "EPSG:6742",       // Geodetic Datum Malaysia 2000
    "EPSG:6744",       // Nahrwan 1934
    "EPSG:6756",       // Vietnam 2000
    "EPSG:6757",       // SVY21
    "MIF 0"        // MAPINFO Datum 0
};


#define DATUM_NAME_LENGTH_LIMIT 65
#define DATUM_MAP_NB_COLS       10

char DatumMap[][DATUM_MAP_NB_COLS][DATUM_NAME_LENGTH_LIMIT] =
{       //  CST                     MapInfoWKT      ECW                 OGR                                                     MapInfo          EPSG            ESRI-WKT                                            Oracle(Legacy)                                Additional 1                           Additional 2                                  Name
    {   ""                       , ""           , "USAIRY"      ,   "Airy_1830"                                         ,   ""         ,  "EPSG:6001"  ,   "D_Airy_1830"                                  ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Airy 1830 ellipsoid)
    {   ""                       , ""           , "USAIRMOD"    ,   "Airy_Modified"                                     ,   ""         ,  "EPSG:6002"  ,   "D_Airy_Modified"                              ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Airy Modified 1849 ellipsoid)
    {   ""                       , ""           , ""            ,   "Australian"                                        ,   ""         ,  "EPSG:6003"  ,   "D_Australian"                                 ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Australian National Spheroid)
    {   ""                       , ""           , "USBESS"      ,   "Bessel_1841"                                       ,   ""         ,  "EPSG:6004"  ,   "D_Bessel_1841"                                ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Bessel 1841 ellipsoid)
    {   ""                       , ""           , "USBESMOD"    ,   "Bessel_Modified"                                   ,   ""         ,  "EPSG:6005"  ,   "D_Bessel_Modified"                            ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Bessel Modified ellipsoid)
    {   ""                       , ""           , "NAMIBIA"     ,   "Bessel_Namibia"                                    ,   ""         ,  "EPSG:6006"  ,   "D_Bessel_Namibia"                             ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Bessel Namibia ellipsoid)
    {   ""                       , ""           , "USC58MTR"    ,   "Clarke_1858"                                       ,   ""         ,  "EPSG:6007"  ,   "D_Clarke_1858"                                ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Clarke 1858 ellipsoid)
    {   ""                       , ""           , "CLRK1866"    ,   "Clarke_1866"                                       ,   ""         ,  "EPSG:6008"  ,   "D_Clarke_1866"                                ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Clarke 1866 ellipsoid)
    {   ""                       , ""           , ""            ,   "Clarke_1866_Michigan"                              ,   ""         ,  "EPSG:6009"  ,   "D_Clarke_1866_Michigan"                       ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Clarke 1866 Michigan ellipsoid)
    {   ""                       , ""           , ""            ,   "Clarke_1880_Benoit"                                ,   ""         ,  "EPSG:6010"  ,   "D_Clarke_1880_Benoit"                         ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Clarke 1880 (Benoit) ellipsoid)
    {   ""                       , ""           , "USC80IGN"    ,   "Clarke_1880_IGN"                                   ,   ""         ,  "EPSG:6011"  ,   "D_Clarke_1880_IGN"                            ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Clarke 1880 (IGN) ellipsoid)
    {   ""                       , ""           , ""            ,   "Clarke_1880_RGS"                                   ,   ""         ,  "EPSG:6012"  ,   "D_Clarke_1880_RGS"                            ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Clarke 1880 (RGS) ellipsoid)
    {   ""                       , ""           , ""            ,   "Clarke_1880_Arc"                                   ,   ""         ,  "EPSG:6013"  ,   "D_Clarke_1880_Arc"                            ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Clarke 1880 (Arc) ellipsoid)
    {   ""                       , ""           , ""            ,   "Clarke_1880_SGA"                                   ,   ""         ,  "EPSG:6014"  ,   "D_Clarke_1880_SGA"                            ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Clarke 1880 (SGA 1922) ellipsoid)
    {   ""                       , ""           , "USEV37AD"    ,   ""                                                  ,   ""         ,  "EPSG:6015"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Everest 1830 (1937 Adjustment) ellipsoid)
    {   ""                       , ""           , "USEV67"      ,   "Everest_Def_1967"                                  ,   ""         ,  "EPSG:6016"  ,   "D_Everest_Def_1967"                           ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Everest 1830 (1967 Definition) ellipsoid)
    {   ""                       , ""           , ""            ,   "Everest_Modified"                                  ,   ""         ,  "EPSG:6018"  ,   "D_Everest_Modified"                           ,    ""                                       , "Everest_Modified_1969"             , "D_Everest_Modified_1969"             },   // Not specified (based on Everest 1830 Modified ellipsoid)
    {   ""                       , "MIF 33"     , "USGRS80"     ,   "GRS_80"                                            ,   "MIF:33"   ,  "EPSG:6019"  ,   "D_GRS_1980"                                   ,    "GRS 80"                                 , "GRS_1980"                          , ""                                    },   // Not specified (based on GRS 1980 ellipsoid)
    {   ""                       , ""           , "USHELM"      ,   "Helmert_1906"                                      ,   ""         ,  "EPSG:6020"  ,   "D_Helmert_1906"                               ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Helmert 1906 ellipsoid)
    {   ""                       , ""           , ""            ,   "Indonesian"                                        ,   ""         ,  "EPSG:6021"  ,   "D_Indonesian"                                 ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Indonesian National Spheroid)
    {   ""                       , ""           , "INT24"       ,   "International_1924"                                ,   ""         ,  "EPSG:6022"  ,   "D_International_1924"                         ,    ""                                       , "USINT24"                           , ""                                    },   // Not specified (based on International 1924 ellipsoid)
    {   ""                       , ""           , "USKRAS40"    ,   "Krasovsky_1940"                                    ,   ""         ,  "EPSG:6024"  ,   "D_Krasovsky_1940"                             ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Krassowsky 1940 ellipsoid)
    {   ""                       , ""           , "NWL9D"       ,   "NWL_9D"                                            ,   ""         ,  "EPSG:6025"  ,   "D_NWL_9D"                                     ,    ""                                       , "USNWL9D"                           , ""                                    },   // Not specified (based on NWL 9D ellipsoid)
    {   ""                       , ""           , "PLESSIS"     ,   "Plessis_1817"                                      ,   ""         ,  "EPSG:6027"  ,   "D_Plessis_1817"                               ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Plessis 1817 ellipsoid)
    {   ""                       , ""           , ""            ,   "Struve_1860"                                       ,   ""         ,  "EPSG:6028"  ,   "D_Struve_1860"                                ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Struve 1860 ellipsoid)
    {   ""                       , ""           , ""            ,   "War_Office"                                        ,   ""         ,  "EPSG:6029"  ,   "D_War_Office"                                 ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on War Office ellipsoid)
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6030"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on WGS 84 ellipsoid)
    {   ""                       , ""           , ""            ,   "GEM_10C"                                           ,   ""         ,  "EPSG:6031"  ,   "D_GEM_10C"                                    ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on GEM 10C ellipsoid)
    {   ""                       , ""           , ""            ,   "OSU_86F"                                           ,   ""         ,  "EPSG:6032"  ,   "D_OSU_86F"                                    ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on OSU86F ellipsoid)
    {   ""                       , ""           , ""            ,   "OSU_91A"                                           ,   ""         ,  "EPSG:6033"  ,   "D_OSU_91A"                                    ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on OSU91A ellipsoid)
    {   ""                       , ""           , ""            ,   "Clarke_1880"                                       ,   ""         ,  "EPSG:6034"  ,   "D_Clarke_1880"                                ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Clarke 1880 ellipsoid)
    {   ""                       , ""           , ""            ,   "Sphere"                                            ,   ""         ,  "EPSG:6035"  ,   "D_Sphere"                                     ,    ""                                       , "Sphere_ARC_INFO"                   , ""                                    },   // Not specified (based on Authalic Sphere)
    {   ""                       , "MIF 32"     , ""            ,   "GRS_67"                                            ,   "MIF:32"   ,  "EPSG:6036"  ,   "D_GRS_1967"                                   ,    "GRS 67"                                 , "USINT67"                           , ""                                    },   // Not specified (based on GRS 1967 ellipsoid)
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6041"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Average Terrestrial System 1977 ellipsoid)
    {   ""                       , ""           , ""            ,   "Everest_1830"                                      ,   ""         ,  "EPSG:6042"  ,   "D_Everest_1830"                               ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Everest (1830 Definition) ellipsoid)
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6043"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on WGS 72 ellipsoid)
    {   ""                       , ""           , ""            ,   "Everest_Def_1962"                                  ,   ""         ,  "EPSG:6044"  ,   "D_Everest_Def_1962"                           ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Everest 1830 (1962 Definition) ellipsoid)
    {   ""                       , ""           , ""            ,   "Everest_Def_1975"                                  ,   ""         ,  "EPSG:6045"  ,   "D_Everest_Def_1975"                           ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Everest 1830 (1975 Definition) ellipsoid)
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6047"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on GRS 1980 Authalic Sphere)
    {   ""                       , ""           , ""            ,   "Sphere_Clarke_1866_Authalic"                       ,   ""         ,  "EPSG:6052"  ,   "D_Sphere_Clarke_1866_Authalic"                ,    ""                                       , "Clarke_1866_Authalic_Sphere"       , ""                                    },   // Not specified (based on Clarke 1866 Authalic Sphere)
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6053"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on International 1924 Authalic Sphere)
    {   ""                       , ""           , ""            ,   "Sphere_International_1924_Authalic"                ,   ""         ,  "EPSG:6054"  ,   "D_Sphere_International_1924_Authalic"         ,    ""                                       , ""                                  , ""                                    },   // Not specified (based on Hughes 1980 ellipsoid)
    {   "SphereWGS84"            , ""           , "SPHERE"      ,   "Popular_Visualisation_Datum"                       ,   ""         ,  "EPSG:6055"  ,   "D_Popular_Visualisation_Datum"                ,    ""                                       , "Popular_Visualization_Datum"       , "D_Popular_Visualization_Datum"       },   // Not specified (Popular Visualization Datum)
    {   "SphereWGS84"            , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   "D_WGS_1984_Major_Auxiliary_Sphere"            ,    "Auxiliary Sphere WGS84"                 , "WGS_1984_Major_Auxiliary_Sphere"   , "D_Auxiliary Sphere WGS84"            },   // Additional line for Popular Visualization Datum above
    {   "Abidjan1987"            , ""           , ""            ,   "Abidjan_1987"                                      ,   ""         ,  "EPSG:6143"  ,   "D_Abidjan_1987"                               ,    ""                                       , "D_Abidjan 1987"                    , "Abidjan87"                           },   // Abidjan1987
    {   "Accra1929"              , ""           , "ACCRA"       ,   "Accra"                                             ,   ""         ,  "EPSG:6168"  ,   "D_Accra"                                      ,    ""                                       , ""                                  , ""                                    },   // Accra
    {   "ADINDAN"                , "MIF 1"      , "ADINDAN"     ,   "Adindan"                                           ,   "MIF:1"    ,  "EPSG:6201"  ,   "D_Adindan"                                    ,    "Adindan"                                , ""                                  , ""                                    },   // ADINBAN
    {   "ADOS714"                , "MIF 10"     , ""            ,   "Astro_Dos_71_4"                                    ,   "MIF:10"   ,  ""           ,   "D_DOS_71_4"                                   ,    "Astro DOS 71/4"                         , "DOS_71_4"                          , ""                                    },   // ADOS714 
    {   "AFGOOYE"                , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // AFGOOYE
    {   ""                       , "MIF 2"      , ""            ,   "Afgooye"                                           ,   "MIF:2"    ,  "EPSG:6205"  ,   "D_Afgooye"                                    ,    "Afgooye"                                , ""                                  , ""                                    },   // A variant of AFGOOYE
    {   ""                       , ""           , ""            ,   "Agadez"                                            ,   ""         ,  "EPSG:6206"  ,   "D_Agadez"                                     ,    ""                                       , ""                                  , ""                                    },   // Agadez INVALID DATUM NO TRANSFORM DEFINED
    {   "AGD66"                  , "MIF 12"     , "AGD66"       ,   "Australian_Geodetic_Datum_66"                      ,   "MIF:12"   ,  "EPSG:6202"  ,   "D_Australian_1966"                            ,    "Australian Geodetic Datum 1966"         , "Australian Geodetic 1966"          , "Australian_1966"                     },   // AGD66 Additional (both) are GDAL
    {   "ASTRLA66-ACT-7P"        , "MIF 1007"   , ""            ,   "AGD66_7_Param_ACT"                                 ,   "MIF:1007" ,  ""           ,   ""                                             ,    "AGD 66 ACT"                             , ""                                  , ""                                    },   // AGD66 Australian Geodetic 1966 7 parameter variation (AGD66 fallback solution)
    {   "AGD66-Tas"              , "MIF 1008"   , ""            ,   "AGD66_7_Param_TAS"                                 ,   "MIF:1008" ,  ""           ,   ""                                             ,    "AGD 66 TAS"                             , ""                                  , ""                                    },   // AGD66 Australian Geodetic 1966 7 parameter variation of AGD66 for Tasmania only
    {   "AGD66-Vic/NSW"          , "MIF 1009"   , ""            ,   "AGD66_7_Param_VIC_NSW"                             ,   "MIF:1009" ,  ""           ,   ""                                             ,    "AGD 66 VIC NSW"                         , "ASTRLA66-VicNsw"                   , ""                                    },   // AGD66-Vic/NSW The extra is a CSMAP dupplicate
    {   "AGD84"                  , "MIF 13"     , "AGD84"       ,   "Australian_Geodetic_Datum_84"                      ,   "MIF:13"   ,  "EPSG:6203"  ,   "D_Australian_1984"                            ,    "Australian Geodetic 1984"               , "Australian Geodetic 1984"          , "New AGD 84"                          },   // AGD84 Additional is GDAL
    {   "AGD84"                  , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    "Australian_1984"                        , "Australian Geodetic Datum 1984"    , "Australian 1984"                     },   // AGD84 second line
    {   "AGD84-P7"               , "MIF 1006"   , ""            ,   "AGD84_7_Param_Aust"                                ,   "MIF:1006" ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // AGD84-P7 Duplicate of AGD84 using 7 parameter variation datum transformation
    {   "AINELABD"               , "MIF 3"      , "AINABD70"    ,   "Ain_el_Abd_1970"                                   ,   "MIF:3"    ,  "EPSG:6204"  ,   "D_Ain_el_Abd_1970"                            ,    "Ain el Abd 1970"                        , ""                                  , ""                                    },   // AINELABD
    {   "Albanian1987"           , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6191"  ,   "D_Albanian_1987"                              ,    ""                                       , ""                                  , "Albanian_1987"                       },   // Albanian 1987 INVALID DATUM NO TRANSFORM DEFINED
    {   "Amersfoort"             , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Amersfoort variant A see DutchRd for variation
    {   "Amersfoort1"            , "MIF 109"    , "RD"          ,   "Netherlands_Bessel"                                ,   "MIF:109"  ,  "EPSG:6289"  ,   "D_Amersfoort"                                 ,    "Netherlands Bessel"                     , ""                                  , "Amersfoort"                          },   // Amersfoort variant (one of the most useable)
    {   "Ammassalik58b"          , ""           , ""            ,   "Ammassalik_1958"                                   ,   ""         ,  "EPSG:6196"  ,   "D_Ammassalik_1958"                            ,    ""                                       , "Ammassalik 1958"                   , "Ammassalik58b"                       },   // Ammassalik58
    {   "Samoa1962"              , "MIF 118"    , ""            ,   "American_Samoa_1962"                               ,   "MIF:118"  ,  "EPSG:6169"  ,   "D_American_Samoa_1962"                        ,    ""                                       , "EPSG:106252"                       , "Samoa_1962"                          },   // AmSamoa62 Alternate introduced by ESRI
    {   "Samoa1962"              , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , "AmSamoa62"                         , "America_Samoa"                       },   // AmSamoa62 Additional Add2 is OGC variant
    {   ""                       , ""           , ""            ,   "Anguilla_1957"                                     ,   ""         ,  "EPSG:6600"  ,   "D_Anguilla_1957"                              ,    "Anguilla 1957"                          , ""                                  , ""                                    },   // Anguilla 1957 INVALID DATUM NO TRANSFORM DEFINED
    {   "ANNA65"                 , "MIF 4"      , ""            ,   "Anna_1_Astro_1965"                                 ,   "MIF:4"    ,  ""           ,   "D_Anna_1_1965"                                ,    "Anna 1 Astro 1965"                      , ""                                  , "Anna_1_1965"                         },   // ANNA65 
    {   "Antarctic1998"          , ""           , ""            ,   "Australian_Antarctic_1998"                         ,   ""         ,  "EPSG:6176"  ,   "D_Australian_Antarctic_1998"                  ,    "Antarctic98"                            , "Australian Antarctic Datum 1998"   , "Australian Antarctic 1998"           },   // Antartic98 Australian Antarctic Datum 1998
    {   "Antigua1943"            , ""           , ""            ,   "Antigua_1943"                                      ,   ""         ,  "EPSG:6601"  ,   "D_Antigua_1943"                               ,    ""                                       , ""                                  , "ANTIGUA"                             },   // Antigua43
    {   ""                       , "MIF 119"    , ""            ,   "Antigua_Astro_1965"                                ,   "MIF:119"  ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Antigua Astro 1965
    {   "Antilles91"             , ""           , ""            ,   "RRAF_1991"                                         ,   ""         ,  "EPSG:6640"  ,   "D_RRAF_1991"                                  ,    ""                                       , ""                                  , "Reseau de Reference des Antilles Francaises 1991" },   // Antilles91
    {   "Aratu_1"                , ""           , ""            ,   "Aratu"                                             ,   ""         ,  "EPSG:6208"  ,   "D_Aratu"                                      ,    ""                                       , ""                                  , ""                                    },   // Aratu
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6208-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Aratu variant A
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6208-2",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Aratu variant B
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6208-3",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Aratu variant C
    {   "ARC1950-01"             , "MIF 5"      , "ARC1950"     ,   "Arc_1950"                                          ,   "MIF:5"    ,  ""           ,   "D_Arc_1950"                                   ,    "Arc 1950"                               , ""                                  , ""                                    },   // ARC1950
    {   ""                       , ""           , "ARC1960"     ,   ""                                                  ,   ""         ,  "EPSG:6209"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // ARC1950 variant
    {   "ARC1960"                , "MIF 6"      , ""            ,   "Arc_1960"                                          ,   "MIF:6"    ,  ""           ,   "D_Arc_1960"                                   ,    "Arc 1960"                               , ""                                  , ""                                    },   // ARC1960
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6210"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // ARC1960 variant
    {   "Ascension58"            , "MIF 7"      , ""            ,   "Ascension_Islands"                                 ,   "MIF:7"    ,  "EPSG:6712"  ,   "D_Ascension_Island_1958"                      ,    "Ascension Island 1958"                  , "Ascension_Island_1958"             , ""                                    },   // ASCENSN
    {   "ASCENSN"                , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // ASCENSN Variant
    {   "ASTATN52"               , "MIF 11"     , ""            ,   "Astronomic_Station_1952"                           ,   "MIF:11"   ,  ""           ,   "D_Astro_1952"                                 ,    "Astronomic Station 1952"                , "Astro_1952"                        , ""                                    },   // ASTATN52 
    {   "ASTRLA66"               , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // ASTRLA66 Australian Geodetic 1966 Variation of AGD66 which uses Multiple regression files
    {   "ASTRLA66-P7"            , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // ASTRLA66 Australian Geodetic 1966 7 parameter variation of AGD66
    {   "ASTRLA66-Tasmania"      , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // ASTRLA66 Australian Geodetic 1966 variation of AGD66-Tas which is also a variation of AGD66
    {   "ASTRLA66-ASTRLA66-VicNsw", ""          , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // ASTRLA66-VicNsw is an exact duplicate of AGD66-Vic/NSW
    {   "ASTRLA84"               , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // ASTRLA84 Australian Geodetic 1984 Variation of AGD84 which uses Multiple regression files
    {   "ATS77"                  , "MIF 151"    , ""            ,   "ATS77"                                             ,   "MIF:151"  ,  "EPSG:6122"  ,   "D_ATS_1977"                                   ,    ""                                       , "ATS_1977"                          , "Average Terrestrial System 1977"     },   // ATS1977
    {   "AyabelleLH"             , "MIF 120"    , ""            ,   "Ayabelle"                                          ,   "MIF:120"  ,  "EPSG:6713"  ,   "D_Ayabelle"                                   ,    ""                                       , "Ayabelle_Lighthouse"               , "DJIBOUTI"                            },   // Ayabelle (Add1 is OGC variant)
    {   "AZORES-G"               , ""           , ""            ,   "Azores_Central_Islands_1948"                       ,   ""         ,  ""           ,   "D_Azores_Central_Islands_1948"                ,    ""                                       , ""                                  , "AZORES"                              },   // AZORES (Azores Central Islands 1948)
    {   "AZORES-G"               , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6183"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // AZORES Variant
    {   "AZORES-G"               , "MIF 126"    , ""            ,   "Gracias_Base_SW_1948"                              ,   "MIF:126"  ,  "EPSG:106241",   "D_Graciosa_Base_SW_1948"                      ,    ""                                       , "Graciosa_base_1948"                , "Graciosa_Base_SW_1948"               },   // Graciosa Base SW 1948 Introduced by ESRI Add1 is OGC variant also named SouthWest and identical to AZORES
    {   "AZORES-G"               , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , "Southwest Base"                    , ""                                    },   // Graciosa Base (SouthWest) second line
    {   "AzoresEast1995"         , ""           , ""            ,   "Azores_Oriental_Islands_1995"                      ,   ""         ,  "EPSG:6664"  ,   "D_Azores_Oriental_Islands_1995"               ,    ""                                       , ""                                  , "AzoresEast95"                        },   // AzoresEast95
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6664-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // AzoresEast95 variant
    {   "AzoresCntrl1995"        , ""           , ""            ,   "Azores_Central_Islands_1995"                       ,   ""         ,  "EPSG:6665"  ,   "D_Azores_Central_Islands_1995"                ,    ""                                       , ""                                  , "AzoresCntrl95"                       },   // AzoresCntrl95
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6665-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // AzoresCntrl95 variant
    {   "AzoresWest1939"         , ""           , ""            ,   "Azores_Occidental_Islands_1939"                    ,   ""         ,  "EPSG:6182"  ,   "D_Azores_Occidental_Islands_1939"             ,    ""                                       , ""                                  , "AzoresWest39"                        },   // AzoresWest39
    {   "AzoresWest1939"         , "MIF 140"    , ""            ,   "Observ_Meteorologico_1939"                         ,   "MIF:140"  ,  "EPSG:106245",   "D_Observ_Meteorologico_1939"                  ,    ""                                       , "Observatorio_Met_1939"             , "Observ_Meteorologico_1939"           },   // Observ Meteorologico 1939 Introduced by ESRI (Add1 is OGC variant)
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6182-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // AzoresWest39 variant
    {   "BAH-SAN"                , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , "NAD 27 (San Salvador)"             , ""                                    },   // Nad 27 (San Salvador)
    {   "BAHAMAS"                , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , "NAD 27 (Bahamas)"                  , ""                                    },   // Nad 27 (Bahamas)
    {   "Barbados1938"           , ""           , ""            ,   "Barbados_1938"                                     ,   ""         ,  "EPSG:6212"  ,   "D_Barbados_1938"                              ,    ""                                       , ""                                  , "Barbados38"                          },   // Barbados38
    {   ""                       , ""           , "BEDUARAM"    ,   "Beduaram"                                          ,   ""         ,  "EPSG:6213"  ,   "D_Beduaram"                                   ,    ""                                       , "BEDUARAM"                          , ""                                    },   // Beduaram
    {   "Beijing1954/a"          , ""           , ""            ,   "Beijing_1954"                                      ,   ""         ,  ""           ,   "D_Beijing_1954"                               ,    "Beijing1954"                            , "Beijing54"                         , "BEIJING"                             },   // Beijing54
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6214"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Beijing54 variant
    {   ""                       , ""           , ""            ,   "Bern_1938"                                         ,   ""         ,  "EPSG:6306"  ,   "D_Bern_1938"                                  ,    ""                                       , ""                                  , ""                                    },   // Bern 1938 INVALID DATUM NO TRANSFORM DEFINED
    {   "Belge72/a"              , "MIF 110"    , ""            ,   "Belge_1972"                                        ,   ""         ,  ""           ,   "D_Belge_1972"                                 ,    "Belgium Hayford"                        , "Reseau_National_Belge_1972"        , "Belgium_Hayford"                     },   // BELGIUM72 Add2 is OGR variant
    {   "Belge72/a"              , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , "Reseau National Belge 1972"        , ""                                    },   // BELGIUM72 Add2 is OGR variant
    {   "Belge72/b"              , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Belge72 variant
    {   "Belge72"                , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6313"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Belge72 variant
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6313-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Belge72 variant
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6313-2",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Belge72 variant
    {   "BELLEVUE"               , "MIF 14"     , ""            ,   "Bellevue_Ign"                                      ,   "MIF:14"   ,  ""           ,   "D_Bellevue_IGN"                               ,    "Bellevue (IGN)"                         , "Bellevue_IGN"                      , ""                                    },   // BELLEVUE
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6714"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // BELLEVUE variant
    {   "BERMUDA"                , "MIF 15"     , ""            ,   "Bermuda_1957"                                      ,   "MIF:15"   ,  ""           ,   "D_Bermuda_1957"                               ,    "Bermuda 1957"                           , ""                                  , ""                                    },   // BERMUDA
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6216"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // BERMUDA variant
    {   "BhutanNtl"              , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:1058"  ,   "D_Bhutan National Geodetic Datum"             ,    ""                                       , "Bhutan National Geodetic Datum"    , ""                                    },   // Bhutan national
    {   "BDA2000"                , ""           , ""            ,   "Bermuda_2000"                                      ,   ""         ,  "EPSG:6762"  ,   "D_Bermuda_2000"                               ,    ""                                       , "D_Bermuda 2000"                    , ""                                    },   // Bermuda 2000
    {   "Bissau_1"               , ""           , ""            ,   "Bissau"                                            ,   ""         ,  "EPSG:6165"  ,   "D_Bissau"                                     ,    ""                                       , ""                                  , "Bissau"                              },   // Bissau
    {   "BOGOTA"                 , "MIF 16"     , "BOGOTA"      ,   "Bogota"                                            ,   "MIF:16"   ,  ""           ,   "D_Bogota"                                     ,    "Bogota Observatory"                     , "Bogota 1975"                       , "D_Bogota_1975_(Bogota)"              },   // BOGOTA
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6218"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // BOGOTA variant
    {   "BukitRimpah_1"          , "MIF 121"    , ""            ,   "Bukit_Rimpah"                                      ,   "MIF:121"  ,  "EPSG:6219"  ,   "D_Bukit_Rimpah"                               ,    ""                                       , "Bukit Rimpah"                      , "BukitRimpah"                         },   // BukitRimpah
    {   "Caledonie91"            , ""           , ""            ,   "RGNC_1991"                                         ,   ""         ,  "EPSG:6645"  ,   "D_RGNC_1991"                                  ,    ""                                       , "D_RGNC9193"                        , ""                                    },   // Caledonie91
    {   "Camacupa_1"             , ""           , "CAMACUPA"    ,   "Camacupa"                                          ,   ""         ,  "EPSG:6220"  ,   "D_Camacupa"                                   ,    ""                                       , ""                                  , ""                                    },   // Camacupa
    {   "CampAreaAstro_1"        , ""           , ""            ,   "Camp_Area"                                         ,   ""         ,  "EPSG:6715"  ,   "D_Camp_Area"                                  ,    ""                                       , ""                                  , "CampAreaAstro"                       },   // CampAreaAstro
    {   "CAMPO"                  , "MIF 17"     , "CMPOINCH"    ,   "Campo_Inchauspe"                                   ,   "MIF:17"   ,  "EPSG:6221"  ,   "D_Campo_Inchauspe"                            ,    "Campo Inchauspe"                        , ""                                  , ""                                    },   // CAMPO
    {   "PicoDeLasNieves"        , ""           , ""            ,   "Pico_de_Las_Nieves"                                ,   ""         ,  "EPSG:6728"  ,   "D_Pico_de_Las_Nieves"                         ,    "Pico de las Nieves"                     , "Pico de la Nieves"                 , "CANARY"                              },   // CANARY (PicoDeLasNieves)
    {   "CANAVRL"                , "MIF 20"     , ""            ,   "Cape_Canaveral"                                    ,   "MIF:20"   ,  "EPSG:6717"  ,   "D_Cape_Canaveral"                             ,    "Cape Canaveral"                         , ""                                  , ""                                    },   // CANAVRL
    {   "CANTON"                 , "MIF 18"     , ""            ,   "Canton_Astro_1966"                                 ,   "MIF:18"   ,  ""           ,   "D_Canton_1966"                                ,    "Canton Astro 1966"                      , "Canton_1966"                       , ""                                    },   // CANTON 
    {   "CAPE"                   , "MIF 19"     , "CAPE"        ,   "Cape"                                              ,   "MIF:19"   ,  "EPSG:6222"  ,   "D_Cape"                                       ,    "Cape"                                   , ""                                  , ""                                    },   // CAPE
    {   "CAPE-1"                 , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // CAPE-1 EPSG variation of CAPE
    {   ""                       , "MIF 1005"   , ""            ,   "Cape_7_Parameter"                                  ,   "MIF:1005" ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // CAPE 7 parameters us almost identical to CAPE-1 but with 1ppm scale additional
    {   "CARTHAGE-MOD"           , "MIF 21"     , ""            ,   "Carthage"                                          ,   "MIF:21"   ,  "EPSG:6223"  ,   "D_Carthage"                                   ,    "Carthage"                               , ""                                  , ""                                    },   // CARTHAGE
    {   "PampaCastillo"          , ""           , ""            ,   "Pampa_del_Castillo"                                ,   ""         ,  "EPSG:6161"  ,   "D_Pampa_del_Castillo"                         ,    ""                                       , ""                                  , "Castillo"                            },   // Castillo
    {   "CH1903/GSB"             , "MIF 1003"   , ""            ,   "Switzerland_CH_1903"                               ,   ""         ,  "EPSG:6149"  ,   "D_CH1903"                                     ,    "CH 1903 (Switzerland)"                  , "CH1903"                            , "D_Bern_1898"                         },   // CH-1903 The alternate is a CSMAP duplicate
    {   "CH1903/GSB"             , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , "CH1903 (Bern)"                     , "CH-1903"                             },   // CH-1903 second line
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6149-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // CH-1903 variant
    {   "CH1903Plus_1"           , ""           , ""            ,   "CH1903+"                                           ,   ""         ,  "EPSG:6150"  ,   "D_CH1903+"                                    ,    ""                                       , ""                                  , "CH1903Plus"                          },   // CH-1903PLUS
    {   "CHATHAM"                , "MIF 22"     , ""            ,   "Chatham_1971"                                      ,   "MIF:22"   ,  "EPSG:6672"  ,   "D_Chatham_Island_1971"                        ,    "Chatham 1971"                           , "Chatham_Island_1971"               , "Chatham Islands Datum 1971"          },   // CHATAM
    {   "Chatham1979"            , ""           , ""            ,   "Chatham_Islands_1979"                              ,   ""         ,  "EPSG:6673"  ,   "D_Chatham_Islands_1979"                       ,    ""                                       , ""                                  , "Chatham Islands Datum 1979"          },   // Chatham1979
    {   "CHUA"                   , "MIF 23"     , "CHUA"        ,   "Chua"                                              ,   "MIF:23"   ,  "EPSG:6224"  ,   "D_Chua"                                       ,    "Chua Astro"                             , "CHUA"                              , "CHAU"                                },   // CHUA or CHAU ??? Both are the same but a typo was introduced somewhere
    {   ""                       , ""           , ""            ,   "Chos_Malal_1914"                                   ,   ""         ,  "EPSG:6160"  ,   "D_Chos_Malal_1914"                            ,    ""                                       , ""                                  , ""                                    },   // Chos Malal 1914
    {   "CocosIsl1965"           , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6708"  ,   "D_CocosIslands1965"                           ,    ""                                       , "D_Cocos_Islands_1965"              , "Cocos1965"                           },   // Cocos1965
    {   "Combani1950"            , ""           , ""            ,   "Combani_1950"                                      ,   ""         ,  "EPSG:6632"  ,   "D_Combani_1950"                               ,    ""                                       , ""                                  , "Combani50"                           },   // Combani50
    {   "Conakry1905"            , ""           , ""            ,   "Conakry_1905"                                      ,   ""         ,  "EPSG:6315"  ,   "D_Conakry_1905"                               ,    ""                                       , ""                                  , "Conakry05"                           },   // Conakry05
    {   "CongoBelge1955"         , ""           , ""            ,   "Institut_Geographique_du_Congo_Belge_1955"         ,   ""         ,  "EPSG:6701"  ,   "D_Institut_Geographique_du_Congo_Belge_1955"  ,    ""                                       , ""                                  , ""                                    },   // CongoBelge1955
    {   "CORREGO"                , "MIF 24"     , "CORRALEG"    ,   "Corrego_Alegre"                                    ,   "MIF:24"   ,  "EPSG:6225"  ,   "D_Corrego_Alegre"                             ,    "Corrego Alegre"                         , ""                                  , ""                                    },   // CORREGO
    {   "HTRS96"                 , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6761"  ,   "D_Croatian_Terrestrial_Reference_System"      ,    ""                                       , "D_HTRS96"                          , "D_Croatian Terrestrial Reference System"},   // Croatian Terrestrial Reference System
    {   "CR05"                   , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   "D_Costa Rica 2005"                            ,    "Costa Rica 2005"                        , ""                                  , ""                                    },   // Costa Rica 05
    {   "CSG1967"                , ""           , ""            ,   "CSG_1967"                                          ,   ""         ,  "EPSG:6623"  ,   "D_CSG_1967"                                   ,    ""                                       , "D_Centre_Spatial_Guyanais_1967"    , "CSG67"                               },   // CSG67 Centre Spatial Guyanais 1967
    {   "CSRS"                   , ""           , ""            ,   "North_American_1983_CSRS98"                        ,   ""         ,  "EPSG:6140"  ,   "D_North_American_1983_CSRS98"                 ,    ""                                       , "D_North_American_1983_CSRS"        , "North_American_1983_CSRS"            },   // NAD83-CSRS98   THE ADDITIONAL ENTRY COMES FROM THE FACT ESRI IS INCONSISTENT AND USES BOTH VALUES DEPENDING IF GENERATED OR FROM PROVIDED PRJ FILES
    {   "CSRS"                   , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , "NAD83 Canadian Spatial Reference System", "NAD83(CSRS98)"                  },   // CSRS98 second line
    {   "CUBA"                   , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , "NAD 27 (Cuba)"                     , ""                                    },   // NAD27 - Cuba
    {   "Czech/JTSK"             , ""           , ""            ,   "S_JTSK"                                            ,   ""         ,  "EPSG:6156"  ,   "D_S_JTSK"                                     ,    ""                                       , "Jednotne"                          , "Jednotne_Trigonometricke_Site_Katastralni"   },   // SJTSK Alternate is a CSMAP duplicate of slightly less precision second alternate is GDAL ECW
    {   "Czech/JTSK"             , "MIF 146"    , ""            ,   "S_JTSK_Ferro"                                      ,   "MIF:146"  ,  ""           ,   ""                                             ,    ""                                       , "Czech S-JTSK"                      , "Jednotne Trigonometricke Site Katastralni"   },   // SJTSK_Ferro ... The same as JTSK but with Prime Meridian at Ferro which we do not really support ... changed to JTSK
    {   "Dabola1981"             , "MIF 123"    , ""            ,   "Dabola_1981"                                       ,   "MIF:123"  ,  "EPSG:6155"  ,   "D_Dabola_1981"                                ,    "D_Dabola"                               , "Dabola"                            , "Dabola81"                            },   // Dabola81 Add1 is OGC variant
    {   "Datum73"                , ""           , "MELRICA"     ,   "Datum_73"                                          ,   ""         ,  "EPSG:6274"  ,   "D_Datum_73"                                   ,    "Melrica 1973 (D73)"                     , "Datum_73"                          , "Melrica_1973_D73"                    },   // Datum73 (Datum73 and EPSG:6274 are very slightly different) Add2 is OGC variant
    {   "DB_REF"                 , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   "D_Deutsche_Bahn_Reference_System"             ,    ""                                       , ""                                  , "Deutsche_Bahn_Reference_System"      },   // DB_REF
    {   "DealulPiscului70"       , ""           , ""            ,   "Dealul_Piscului_1970"                              ,   ""         ,  "EPSG:6317"  ,   "D_Dealul_Piscului_1970"                       ,    ""                                       , ""                                  , ""                                    },   // DealulPiscului70
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6317-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // DealulPiscului70 variant
    {   "DealulPiscului1933"     , ""           , ""            ,   "Dealul_Piscului_1933"                              ,   ""         ,  "EPSG:6316"  ,   "D_Dealul_Piscului_1933"                       ,    ""                                       , ""                                  , "DealulPiscului33"                    },   // DealulPiscului33
    {   "DeceptionIsland_1"      , "MIF 124"    , ""            ,   "Deception_Island"                                  ,   "MIF:124"  ,  "EPSG:6736"  ,   "D_Deception_Island"                           ,    ""                                       , ""                                  , "DeceptionIsland"                     },   // DeceptionIsland
    {   "DeirEzZor_2"            , ""           , "DEIR"        ,   "Deir_ez_Zor"                                       ,   ""         ,  "EPSG:6227"  ,   "D_Deir_ez_Zor"                                ,    ""                                       , ""                                  , "DeirEzZor"                           },   // DeirEzZor
    {   "DeirEzZor-7P"           , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6227-1",   ""                                             ,    ""                                       , ""                                  , "DeirEzZor-7P"                        },   // DeirEzZor variant
    {   "DGN95"                  , ""           , ""            ,   "Datum_Geodesi_Nasional_1995"                       ,   ""         ,  "EPSG:6755"  ,   "D_Datum_Geodesi_Nasional_1995"                ,    ""                                       , "Datum Geodesi Nasional 1995"       , ""                                    },   // DGN95
    {   "DiegoGarcia1969"        , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6724"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // DiegoGarcia1969
    {   "Batavia_1"              , "MIF 25"     , "BATAVIA"     ,   "Batavia"                                           ,   "MIF:25"   ,  "EPSG:6211"  ,   "D_Batavia"                                    ,    "Djakarta (Batavia)"                     , "D_BATAVIA_MSDQ"                    , "DJAKRTA"                                    },   // DJAKRTA (The alternate is a duplicate entry in csmap)
    {   "DHDN"                   , "MIF 1000"   , ""            ,   "DHDN_Potsdam_Rauenberg"                            ,   "MIF:1000" ,  "EPSG:6314"  ,   "D_Deutsches_Hauptdreiecksnetz"                ,    "DHDN (Potsdam/Rauenberg)"               , "D_Deutsche_Hauptdreiecksnetz"      , "Deutsches_Hauptdreiecksnetz"         },   // DHDN Alternate is missspell of other ESRI
    {   "DHDN"                   , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , "Deutsches Hauptdreiecksnetz"       , ""                                    },   // DHDN second line
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6314-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // DHDN variant
    {   "DLX-7P"                 , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    "Lisboa (DLx)"                                       , ""                                  , ""                                               },   // Lisboa DLX
    {   "Dominica1945"           , ""           , ""            ,   "Dominica_1945"                                     ,   ""         ,  "EPSG:6602"  ,   "D_Dominica_1945"                              ,    ""                                       , "DOMINICA"                          , "Dominica45"                          },   // Dominica45
    {   "DOS1968"                , "MIF 26"     , ""            ,   "Dos_1968"                                          ,   "MIF:26"   ,  ""           ,   "D_DOS_1968"                                   ,    "DOS 1968"                               , "DOS_1968"                          , ""                                    },   // DOS1968 UNKNOWN
    {   "Douala1948"             , ""           , ""            ,   "Douala_1948"                                       ,   ""         ,  "EPSG:6192"  ,   "D_Douala_1948"                                ,    ""                                       , ""                                  , "Douala48"                            },   // Douala48 
    {   "DutchRd"                , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Variation of Amersfoort ... should be identical to EPSG:6289 ... verify!
    {   "EASTER"                 , "MIF 27"     , ""            ,   "Easter_Island_1967"                                ,   "MIF:27"   ,  "EPSG:6719"  ,   "D_Easter_Island_1967"                         ,    "Easter Island 1967"                     , ""                                  , ""                                    },   // EASTER UNKNOWN
    {   "ED50"                   , ""           , "ED50"        ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // ED50 Older 7 parameter variation of EUROP 50 For Spain only use ED50-IGN.ES
    {   "ED50-IGN.ES"            , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // ED50-IGN.ES Official Spain transformation from Europe 50 to ETRF 89
    {   "ED50-DK34"              , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    "European 1950 (Denmark)"                , ""                                  , ""                                    },   // ED50-DK34 UNKNOWN
    {   "ED50COMOFF"             , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // ED50COMOFF
    {   ""                       , ""           , "EGYPT24"     ,   "Egypt_1930"                                        ,   ""         ,  "EPSG:6199"  ,   "D_Egypt_1930"                                 ,    ""                                       , ""                                  , ""                                    },   // Egypt 1930 INVALID DATUM NO TRANSFORM DEFINED
    {   "EGYP-GS"                , ""           , ""            ,   "Egypt_Gulf_of_Suez_S-650_TL"                       ,   ""         ,  "EPSG:6706"  ,   "D_Egypt_Gulf_of_Suez_S-650_TL"                ,    ""                                       , ""                                  , ""                                    },   // Egypt Gulf of Suez S-650 TL
    {   "ERP50-CY"               , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // ERP50-CY Variation of EUROP50 for Cyprus only
    {   "ERP50-EG"               , ""           , "ED50EGYT"    ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // ERP50-EG Variation of EUROP50 for Egypt only
    {   "ERP50-GB"               , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // ERP50-GB Variation of EUROP50 for Great Britain only
    {   "ERP50-IR"               , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // ERP50-IR Variation of EUROP50 for Iran only
    {   "ERP50-UK"               , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // ERP50-UK Variation of EUROP50 for Great Britain and Ireland only
    {   "ERP50-W"                , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // ERP50-UK Variation of EUROP50 for Western Europe only
    {   "Estonia1937"            , "MIF 122"    , ""            ,   "Estonia_1937"                                      ,   "MIF:122"  ,  ""           ,   "D_Estonia_1937"                               ,    ""                                       , ""                                  , "EST-1937"                            },   // Estonia 1937 
    {   "Estonia92b"             , ""           , ""            ,   "Estonia_1992"                                      ,   ""         ,  "EPSG:6133"  ,   "D_Estonia_1992"                               ,    ""                                       , "Estonia 1992"                      , "Estonia92"                           },   // Estonia92 
    {   "Estonia1997"            , ""           , ""            ,   "Estonia_1997"                                      ,   ""         ,  "EPSG:6180"  ,   "D_Estonia_1997"                               ,    ""                                       , "Estonia 1997"                      , "Estonia97"                           },   // Estonia97 
    {   "ETRF89"                 , "MIF 115"    , ""            ,   "Euref_89"                                          ,   "MIF:115"  ,  "EPSG:6258"  ,   "D_ETRF_1989"                                  ,    "ETRS 89"                                , "Euref_98"                          , "ETRS89"                              },   // ETRF89             Note: Euref_98 in additional has added for compatibility with OGR, We think that Frank Warmerdam has done a typing error (see mitab_spatialref.cpp, line 305, Euref_98). ETRS89 is added as a duplicate in many systems
    {   "ETRF89"                 , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   "D_ETRS_1989"                                  ,    "European_Terrestrial_Reference_System_1989", "European Terrestrial Reference System 1989" , "ETRS 1989"               },   // ETRF89 Second line ... for additional additional
    {   "ETRF89"                 , ""           , ""            ,   ""                                                  ,   ""         ,  "D_ETRS89"   ,   "D_ETRS89/01"                                  ,    "D_ETRS_89"                              , "D_EUREF_1989"                      , "ETRF1989"                            },   // ETRF89 Third line ... for additional additional
    {   "EUR50-7P"               , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // EUR50-7P Variation of EUROP50 using 7 Parameters
    {   "Europ50/1977"           , ""           , ""            ,   "European_1950_ED77"                                ,   ""         ,  "EPSG:6154"  ,   "D_European_1950_ED77"                         ,    "D_European_Datum_1950(1977)"            , "D_European Datum 1950(1977)"       , "Europ50/77"                          },   // EUROP50/77
    {   "ERP50-IR-7P"            , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6154-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // EUROP50/77 variant for Iran
    {   "EUROP50-Mean"           , "MIF 28"     , ""            ,   "European_Datum_1950"                               ,   "MIF:28"   ,  "EPSG:6230"  ,   "D_European_1950"                              ,    "European 1950"                          , "European_1950"                     , "European 1950"                       },   // EUROP50 Additional is generated by OGR, second additional is GDAL
    {   "EUROP50-Mean"           , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   "D_European 1950 - Mean"                       ,    ""                                       , "D_European Datum 1950"             , ""                                    },   // EUROP50 2nd line
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6230-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // EUROP50 variant
    {   "EUROP79"                , "MIF 29"     , ""            ,   "European_Datum_1979"                               ,   "MIF:29"   ,  "EPSG:6668"  ,   "D_European_1979"                              ,    "European 1979"                          , "European_1979"                     , "European Datum 1979"                 },   // EUROP79
    {   "ELD1979"                , ""           , ""            ,   "European_Libyan_1979"                              ,   ""         ,  "EPSG:6159"  ,   "D_European_Libyan_1979"                       ,    ""                                       , "D_European_Libyan_Datum_1979"      , "EuropLibyan79"                       },   // EuropLibyan79
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6159-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // EuropLibyan79 variant
    {   "Europ87"                , "MIF 108"    , ""            ,   "European_Datum_1987"                               ,   "MIF:108"  ,  "EPSG:6231"  ,   "D_European_1987"                              ,    "European 1987"                          , "European_1987"                     , "European Datum 1987"                 },   // EUROP87  Note: The additional entry is that for some unforsaken reason OGR can sometime provide additional value
    {   "OMAN"                   , ""           , "FAHUD"       ,   "Fahud"                                             ,   ""         ,  "EPSG:6232"  ,   "D_Fahud"                                      ,    ""                                       , ""                                  , "Fahud_1"                             },   // Fahud
    {   "OMAN"                   , "MIF 78"     , ""            ,   "Oman"                                              ,   "MIF:78"   ,  ""           ,   "D_Oman"                                       ,    "Oman"                                   , ""                                  , ""                                    },   // OMAN 
    {   ""                       , ""           , ""            ,   "Faroe_Datum_1954"                                  ,   ""         ,  "EPSG:6741"  ,   "D_Faroe_Datum_1954"                           ,    ""                                       , ""                                  , ""                                    },   // Faroe Datum 1954 INVALID DATUM NO TRANSFORM DEFINED
    {   "FatuIva/72"             , ""           , ""            ,   "Fatu_Iva_1972"                                     ,   ""         ,  "EPSG:6688"  ,   "D_Fatu_Iva_1972"                              ,    ""                                       , "Fatu Iva 72"                       , ""                                    },   // FatuIva/72
    {   "Fiji1956"               , ""           , ""            ,   "Fiji_1956"                                         ,   ""         ,  "EPSG:6721"  ,   "D_Fiji_1956"                                  ,    ""                                       , "Fiji Geodetic Datum 1956"          , ""                                    },   // Fiji1956
    {   "Fiji1986"               , ""           , ""            ,   "Fiji_1986"                                         ,   ""         ,  "EPSG:6720"  ,   "D_Fiji_1986"                                  ,    ""                                       , "Fiji Geodetic Datum 1986"          , ""                                    },   // Fiji1986
    {   "Final1958"              , ""           , ""            ,   "FD_1958"                                           ,   ""         ,  "EPSG:6132"  ,   "D_FD_1958"                                    ,    ""                                       , "FD58"                              , ""                                    },   // FD1958
    {   "Martinique38"           , ""           , ""            ,   "Fort_Desaix"                                       ,   ""         ,  "EPSG:6625"  ,   "D_Fort_Desaix"                                ,    ""                                       , "D_Martinique_1938"                 , "FortDesaix"                          },   // FortDesaix
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6625-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // FortDesaix variant
    {   "FortMarigot_1"          , ""           , ""            ,   "Fort_Marigot"                                      ,   ""         ,  "EPSG:6621"  ,   "D_Fort_Marigot"                               ,    ""                                       , ""                                  , "FortMarigot"                         },   // FortMarigot
    {   "FortThomas1955"         , "MIF 125"    , ""            ,   "Fort_Thomas_1955"                                  ,   "MIF:125"  ,  ""           ,   "D_Fort_Thomas_1955"                           ,    ""                                       , "D_Fort_Thomas"                     , "FORT-TH"                             },   // Fort Thomas 1955
    {   "Gan70"                  , "MIF 30"     , "Gan_1970"    ,   "Gandajika_1970"                                    ,   "MIF:30"   ,  "EPSG:6233"  ,   "D_Gan_1970"                                   ,    "D_Gandajika"                            , "Gandajika 1970"                    , "Gan1970"                             },   // GNDAJIKA
    {   "Gan70"                  , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6284"  ,   "D_Gandajika_1970"                             ,    "Gandajika Base"                         , "D_Gandajika_1970"                  , "GNDAJIKA"                            },   // GNDAJIKA second line
    {   ""                       , ""           , ""            ,   "Garoua"                                            ,   ""         ,  "EPSG:6197"  ,   "D_Garoua"                                     ,    ""                                       , ""                                  , ""                                    },   // Garoua
    {   "GDA94"                  , "MIF 116"    , "GDA94"       ,   "GDA94"                                             ,   "MIF:116"  ,  "EPSG:6283"  ,   "D_GDA_1994"                                   ,    "GDA 94"                                 , "GDA_1994"                          , "Geocentric Datum of Australia 1994"  },   // GDA94
    {   "GDA94"                  , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , "Geocentric_Datum_of_Australia_1994"  },   // GDA94 second line
    {   "GDA2020"                , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:1168"  ,   "D_GDA_2020"                                   ,    "GDA 2020"                               , "GDA_2020"                          , "Geocentric Datum of Australia 2020"  },   // GDA2020
    {   "GDA2020"                , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   "D_GDA2020"                                    ,    "GDA2020"                                , "GDA2020"                           , "Geocentric_Datum_of_Australia_2020"  },   // GDA2020 second line
    {   ""                       , ""           , ""            ,   "GDM_2000"                                          ,   ""         ,  "EPSG:6742"  ,   "D_GDM_2000"                                   ,    ""                                       , ""                                  , ""                                    },   // Geodetic Datum of Malaysia 2000 INVALID DATUM NO TRANSFORM DEFINED
    {   "GrandCayman1959"        , ""           , ""            ,   "Grand_Cayman_1959"                                 ,   ""         ,  "EPSG:6723"  ,   "D_Grand_Cayman_1959"                          ,    ""                                       , "Grand Cayman 1959"                 , ""                                    },   // GrandCayman1959
    {   "GrandComoros"           , ""           , ""            ,   "Grand_Comoros"                                     ,   ""         ,  "EPSG:6646"  ,   "D_Grand_Comoros"                              ,    ""                                       , ""                                  , ""                                    },   // Grand Comoros INVALID DATUM NO TRANSFORM DEFINED
    {   "Greek-GRS87"            , ""           , "EGSA87"      ,   "GGRS_1987"                                         ,   ""         ,  "EPSG:6121"  ,   "D_GGRS_1987"                                  ,    ""                                       , "HGRS87"                            , "Greek Geodetic Reference System 1987"},   // GGRS1987
    {   "GRSSA"                  , ""           , ""            ,   "SIRGAS"                                            ,   ""         ,  "EPSG:6170"  ,   "D_SIRGAS"                                     ,    ""                                       , "Sistema de Referencia Geocentrico para America del Sur"      , "Sistema de Referencia Geocentrico para America del Sur 1995"          },   // GRSSA Sistema de Referencia Geocentrico para America del Sur
    {   "GHANA-WO"               , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // GHANA-WO
    {   ""                       , ""           , ""            ,   "Greek"                                             ,   ""         ,  "EPSG:6120"  ,   "D_Greek"                                      ,    ""                                       , ""                                  , ""                                    },   // GREEK
    {   "GR96"                   , ""           , ""            ,   "Greenland_1996"                                    ,   ""         ,  "EPSG:6747"  ,   "D_Greenland_1996"                             ,    ""                                       , "Greenland 1996"                    , ""                                    },   // GR96 (Greenland 1996)
    {   "Grenada1953"            , ""           , ""            ,   "Grenada_1953"                                      ,   ""         ,  "EPSG:6603"  ,   "D_Grenada_1953"                               ,    ""                                       , ""                                  , "Grenada53"                           },   // Grenada53
    {   "Gulshan303"             , ""           , ""            ,   "Gulshan_303"                                       ,   ""         ,  "EPSG:6682"  ,   "D_Gulshan_303"                                ,    ""                                       , ""                                  , "Gulshan"                             },   // Gulshan
    {   "GUAM63"                 , "MIF 34"     , ""            ,   "Guam_1963"                                         ,   "MIF:34"   ,  "EPSG:6675"  ,   "D_Guam_1963"                                  ,    "Guam 1963"                              , ""                                  , ""                                    },   // GUAM63 
    {   "GunungSegara"           , "MIF 88"     , ""            ,   "Samboja"                                           ,   "MIF:88"   ,  "EPSG:6125"  ,   "D_Samboja"                                    ,    ""                                       , "Samboja_1"                         , ""                                    },   // SAMBOJA became GunundSegara
    {   "GunungSegara"           , ""           , ""            ,   "Gunung_Segara"                                     ,   ""         ,  "EPSG:6613"  ,   "D_Gunung_Segara"                              ,    ""                                       , "Segara"                            , ""                                    },   // Segara became GunundSegara
    {   "GUX1"                   , "MIF 35"     , ""            ,   "Gux_1_Astro"                                       ,   "MIF:35"   ,  ""           ,   "D_GUX_1"                                      ,    "GUX 1 Astro"                            , "GUX_1"                             , ""                                    },   // GUX1
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6635"  ,   "D_Guyanne_Francaise"                          ,    ""                                       , ""                                  , ""                                    },   // Guyane Francaise
    {   "Guyane95"               , ""           , ""            ,   "RGFG_1995"                                         ,   ""         ,  "EPSG:6624"  ,   "D_RGFG_1995"                                  ,    ""                                       , "Reseau Geodesique Francais Guyane 1995"  , ""                              },   // Guyane95
    {   "Hanoi1972"              , ""           , ""            ,   "Hanoi_1972"                                        ,   ""         ,  "EPSG:6147"  ,   "D_Hanoi_1972"                                 ,    ""                                       , ""                                  , "Hanoi72"                             },   // Hanoi1972
    {   "HeratNorth_1"           , "MIF 127"    , ""            ,   "Herat_North"                                       ,   "MIF:127"  ,  "EPSG:6255"  ,   "D_Herat_North"                                ,    ""                                       , ""                                  , "HeratNorth"                          },   // HeratNorth
    {   "NAD83/HARN-A"           , ""           , ""            ,   "North_American_1983_HARN"                          ,   ""         ,  "EPSG:6152"  ,   "D_North_American_1983_HARN"                   ,    ""                                       , "HARN"                              , "NAD83_High_Accuracy_Regional_Network"},   // HARN / HPGN Additional is GDAL
    {   "NAD83/HARN-A"           , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    "HPGN"                                   , "D_NAD83_High_Accuracy_Reference_Network" , "NAD83 (High Accuracy Regional Network)"}, // HARN / HPGN Additional is CSMAP WKT Variation
    {   "Hartebeesthoek94"       , "MIF 150"    , ""            ,   "Hartebeesthoek94"                                  ,   "MIF:150"  ,  "EPSG:6148"  ,   "D_Hartebeesthoek_1994"                        ,    ""                                       , "Hartebeesthoek_1994"               , ""                                    },   // Hartebeesthoek1994
    {   "HD1909"                 , ""           , ""            ,   "Hungarian_1909"                                    ,   ""         ,  "EPSG:1024"  ,   "D_Hungarian_1909"                             ,    ""                                       , "Hungarian_Datum_1909"              , "D_Hungarian Datum 1909"              },   // Hungarian Datum of 1909
    {   "HD72/7Pa"               , "MIF 1004"   , ""            ,   "Hungarian_1972"                                    ,   "MIF:1004" ,  "EPSG:6237"  ,   "D_Hungarian_1972"                             ,    "D_Hungarian_Datum_1972"                 , "Hungarian_Datum_1972"              , "HD-72"                               },   // HD-72 Hungarian Datum of 1972 (Add1 is OGC variant
    {   "Helle1954"              , ""           , ""            ,   "Helle_1954"                                        ,   ""         ,  "EPSG:6660"  ,   "D_Helle_1954"                                 ,    ""                                       , "Helle 1954"                        , ""                                    },   // Helle1954
    {   "HitoXVIII63b"           , "MIF 36"     , ""            ,   "Hito_XVIII_1963"                                   ,   "MIF:36"   ,  "EPSG:6254"  ,   "D_Hito_XVIII_1963"                            ,    "Hito XVIII 1963"                        , ""                                  , "HitoXVIII63b"                        },   // HITO
    {   "HJORSEY"                , "MIF 37"     , ""            ,   "Hjorsey_1955"                                      ,   "MIF:37"   ,  "EPSG:6658"  ,   "D_Hjorsey_1955"                               ,    "Hjorsey 1955"                           , ""                                  , ""                                    },   // HJORSEY 
    {   "HOLLAND"                , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Amersfoort variant B
    {   "HONGKONG"               , "MIF 38"     , ""            ,   "Hong_Kong_1963"                                    ,   "MIF:38"   ,  "EPSG:6738"  ,   "D_Hong_Kong_1963"                             ,    "Hong Kong 1963"                         , ""                                  , ""                                    },   // HONGKONG 1963
    {   "HongKong1963/67"        , ""           , ""            ,   "Hong_Kong_1963_67"                                 ,   ""         ,  "EPSG:6739"  ,   "D_Hong_Kong_1963_67"                          ,    ""                                       , "HONGKONG"                          , ""                                    },   // HongKong1963/67 Alternate is a csmap duplicate
    {   "HongKong80"             , ""           , "HK80"        ,   "Hong_Kong_1980"                                    ,   ""         ,  "EPSG:6611"  ,   "D_Hong_Kong_1980"                             ,    "Hong Kong 1980"                         , "EPSG:106261"                       , ""                                    },   // HONGKONG 1980 Alternate is introduced by ESRI
    {   "HS2SD_2002"             , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   "D_HS2SD_2012"                                 ,    "HS2TN02 datum"                          , "HS2TN02"                           , "D_HS2TN02"                           },   // High Speed 2 - 2002
    {   "HS2SD_2002"             , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , "HS2SD_2002"                        , ""                                    },   // High Speed 2 - 2002
    {   "HS2SD_2015"             , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   "D_HS2SD_2015"                                 ,    "HS2TN15 datum"                          , "HS2TN15"                           , "D_HS2TN15"                           },   // High Speed 2 - 2015
    {   "HS2SD_2015"             , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   "D_HS2SD"                                      ,    "HS2SD"                                  , "HS2SD_2015"                        , ""                                    },   // High Speed 2 - 2015
    {   "HuTzuShan_1"            , "MIF 39"     , ""            ,   "Hu_Tzu_Shan"                                       ,   "MIF:39"   ,  "EPSG:6236"  ,   "D_Hu_Tzu_Shan"                                ,    "Hu-Tzu-Shan"                            , "D_Hu Tzu Shan 1950"                , "HuTzuShan"                           },   // HUTZUSHAN
    {   "HuTzuShan_1"            , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   "D_Hu_Tzu_Shan_1950"                           ,    "Hu-Tzu-Shan"                            , "D_Hu Tzu Shan 1950"                , "HuTzuShan"                           },   // HUTZUSHAN second line
    {   ""                       , ""           , ""            ,   "IGC_1962_Arc_of_the_6th_Parallel_South"            ,   ""         ,  "EPSG:6697"  ,   "D_IGC_1962_Arc_of_the_6th_Parallel_South"     ,    ""                                       , ""                                  , ""                                    },   // IGC 1962 Arc of the 6th Parallel South INVALID DATUM NO TRANSFORM DEFINED
    {   "IGM1995"                , ""           , ""            ,   "IGM_1995"                                          ,   ""         ,  "EPSG:6670"  ,   "D_IGM_1995"                                   ,    ""                                       , "Istituto Geografico Militaire 1995", ""                                    },   // IGM1995
    {   "IGN53Mare"              , ""           , ""            ,   "IGN53_Mare"                                        ,   ""         ,  "EPSG:6641"  ,   "D_IGN53_Mare"                                 ,    ""                                       , "IGN53/Mare"                        , "IGN53 Mare"                          },   // IGN53Mare Alternate is a CSMAP duplicate
    {   "IGN56/Lifou"            , ""           , ""            ,   "IGN56_Lifou"                                       ,   ""         ,  "EPSG:6633"  ,   "D_IGN56_Lifou"                                ,    ""                                       , ""                                  , "IGN56Lifou"                          },   // IGN56Lifou
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6633-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // IGN56Lifou variant
    {   "IGN63/Hiva Oa"          , ""           , ""            ,   "IGN63_Hiva_Oa"                                     ,   ""         ,  "EPSG:6689"  ,   "D_IGN63_Hiva_Oa"                              ,    ""                                       , "IGN63 Hiva Oa"                     , ""                                    },   // IGN63/Hiva Oa
    {   "IGN72/GrandeTerre"      , ""           , ""            ,   "IGN72_Grande_Terre"                                ,   ""         ,  "EPSG:6634"  ,   "D_IGN72_Grande_Terre"                         ,    ""                                       , ""                                  , "IGN72GrandeTerre"                    },   // IGN72GrandeTerre
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6634-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // IGN72GrandeTerre variant
    {   "IGN72/NukuHiva"         , ""           , ""            ,   "IGN72_Nuku_Hiva"                                   ,   ""         ,  "EPSG:6630"  ,   "D_IGN72_Nuku_Hiva"                            ,    "IGN72/NH_1"                             , "IGN72/NH"                          , "IGN72NukuHiva"                       },   // IGN72NukuHiva Alternate is a CSMAP duplicate
    {   ""                       , ""           , ""            ,   "IGN_Astro_1960"                                    ,   ""         ,  "EPSG:6700"  ,   "D_IGN_Astro_1960"                             ,    ""                                       , ""                                  , ""                                    },   // IGN Astro 1960
    {   "INDIAN"                 , "MIF 41"     , ""            ,   "Indian_Bangladesh"                                 ,   "MIF:41"   ,  ""           ,   ""                                             ,    "Indian (Bangladesh, etc.)"              , ""                                  , ""                                    },   // INDIAN Important note: The OGC "Indian" is not the same ... Unable to match OGC Indian with anything else
    {   "Indian1954"             , "MIF 130"    , "INDIAN54"    ,   "Indian_1954"                                       ,   "MIF:130"  ,  "EPSG:6239"  ,   "D_Indian_1954"                                ,    ""                                       , ""                                  , "Indian54"                            },   // Indian54
    {   "Indian1960/E"           , "MIF 131"    , "INDIAN60"    ,   "Indian_1960"                                       ,   "MIF:131"  ,  "EPSG:6131"  ,   "D_Indian_1960"                                ,    ""                                       , ""                                  , "D_Indian 1960"                       },   // INDIAN1960
    {   "Indian75/E"             , "MIF 132"    , "INDIAN75"    ,   "Indian_1975"                                       ,   "MIF:132"  ,  "EPSG:6240"  ,   "D_Indian_1975"                                ,    ""                                       , ""                                  , ""                                    },   // Indian75
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6240-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Indian75 variant 
    {   "INDIANTV"               , "MIF 40"     , ""            ,   "Indian_Thailand_Vietnam"                           ,   "MIF:40"   ,  ""           ,   ""                                             ,    "Indian (Thailand/Vietnam)"              , ""                                  , ""                                    },   // INDIANTV 
    {   "Indonesian1974"         , "MIF 133"    , "IND74"       ,   "Indonesian_1974"                                   ,   "MIF:133"  ,  "EPSG:6238"  ,   "D_Indonesian_1974"                            ,    "D_Indonesian_Datum_1974"                , "Indonesian_Datum_1974"             , "Indonesian74"                        },   // Indonesian74 Add1 is OGC variant
    {   "ID74-7P"                , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6238-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Indonesian74 variant
    {   "IraqKuwait1992"         , ""           , ""            ,   "Iraq_Kuwait_Boundary_1992"                         ,   ""         ,  "EPSG:6667"  ,   "D_Iraq_Kuwait_Boundary_1992"                  ,    ""                                       , "Iraq-Kuwait Boundary Datum 1992"   , ""                                    },   // IraqKuwait1992
    {   "IRELND65"               , "MIF 42"     , ""            ,   "Ireland_1965"                                      ,   "MIF:42"   ,  ""           ,   ""                                             ,    "Ireland 1965"                           , ""                                  , ""                                    },   // IRELND65 
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6300-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // IRELND variant 
    {   "IRENET95"               , ""           , ""            ,   "IRENET95"                                          ,   ""         ,  "EPSG:6173"  ,   "D_IRENET95"                                   ,    ""                                       , ""                                  , ""                                    },   // IRENET95
    {   "ISN93"                  , ""           , ""            ,   "Islands_Network_1993"                              ,   ""         ,  "EPSG:6659"  ,   "D_Islands_Network_1993"                       ,    ""                                       , "Islands Network 1993"              , ""                                    },   // ISN93 Islenska Grunnstoovanetid 1993 (Iceland 1993)
    {   "Israel_1"               , ""           , ""            ,   "Israel"                                            ,   ""         ,  "EPSG:6141"  ,   "D_Israel"                                     ,    ""                                       , ""                                  , ""                                    },   // Israel
    {   "ISTS69"                 , "MIF 43"     , ""            ,   "ISTS_073_Astro_1969"                               ,   "MIF:43"   ,  ""           ,   "D_ISTS_073_1969"                              ,    "ISTS 073 Astro 1969"                    , "ISTS_073_1969"                     , ""                                    },   // ISTS69 UNKNOWN
    {   "IwoJima45"              , "MIF 8"      , ""            ,   "Astro_Beacon_E"                                    ,   "MIF:8"    ,  "EPSG:6709"  ,   ""                                             ,    "Astro Beacon E"                         , "Beacon_E_1945"                     , "IwoJima1945"                         },   // IWOJIMA variant A
    {   "IwoJima45"              , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , "IWOJIMA"                             },   // IWOJIMA variant B
    {   ""                       , ""           , "JA1875"      ,   "Jamaica_1875"                                      ,   ""         ,  "EPSG:6241"  ,   "D_Jamaica_1875"                               ,    ""                                       , ""                                  , ""                                    },   // Jamaica 1875 INVALID DATUM NO TRANSFORM DEFINED
    {   "Jamaica2001"            , ""           , ""            ,   "Jamaica_2001"                                      ,   ""         ,  "EPSG:6758"  ,   "D_Jamaica_2001"                               ,    "D_Jamaica 2001"                                       , "JAD2001"                           , "D_JAD2001"                           },   // Jamaica 2001
    {   "Jamaica1969"            , ""           , "JAD69"       ,   "Jamaica_1969"                                      ,   ""         ,  "EPSG:6242"  ,   "D_Jamaica_1969"                               ,    ""                                       , ""                                  , "Jamaica69"                           },   // Jamaica69
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6242-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Jamaica69 variant 
    {   "JGD2000"                , ""           , ""            ,   "JGD_2000"                                          ,   ""         ,  "EPSG:6612"  ,   "D_JGD_2000"                                   ,    ""                                       , "Tokyo-Grid"                        , "Japanese Geodetic Datum 2000"        },   // JGD2000 Alternate is CSMAP duplicate
    {   "JGD2011"                , ""           , ""            ,   "JGD_2011"                                          ,   ""         ,  "EPSG:1128"  ,   "D_JGD_2011"                                   ,    ""                                       , ""                                  , "Japanese Geodetic Datum 2011"        },   // JGD2011
    {   "JHNSTN"                 , "MIF 44"     , ""            ,   "Johnston_Island_1961"                              ,   "MIF:44"   ,  "EPSG:6725"  ,   "D_Johnston_Island_1961"                       ,    "Johnston Island 1961"                   , ""                                  , ""                                    },   // JHNSTN 
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6725"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Johnston variant B 
    {   "Jouik61"                , ""           , ""            ,   "Jouik_1961"                                        ,   ""         ,  "EPSG:6679"  ,   "D_Jouik_1961"                                 ,    ""                                       , ""                                  , "Jouik1961"                           },   // Jouik1961
    {   "K01949"                 , ""           , ""            ,   "K0_1949"                                           ,   ""         ,  "EPSG:6631"  ,   "D_K0_1949"                                    ,    ""                                       , ""                                  , ""                                    },   // K01949
    {   ""                       , ""           , ""            ,   "Kalianpur_1880"                                    ,   ""         ,  "EPSG:6243"  ,   "D_Kalianpur_1880"                             ,    ""                                       , ""                                  , ""                                    },   // Kalianpur1880 INVALID DATUM NO TRANSFORM DEFINED
    {   "Kalianpur1937"          , ""           , ""            ,   "Kalianpur_1937"                                    ,   ""         ,  "EPSG:6144"  ,   "D_Kalianpur_1937"                             ,    ""                                       , "D_Kalianpur 1937"                  , "Kalianpur37"                         },   // Kalianpur1937
    {   "Kalianpur1962"          , ""           , ""            ,   "Kalianpur_1962"                                    ,   ""         ,  "EPSG:6145"  ,   "D_Kalianpur_1962"                             ,    ""                                       , "D_Kalianpur 1962"                  , "Kalianpur62"                         },   // Kalianpur1962
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6145-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Kalianpur62 variant 
    {   "Kalianpur1975"          , ""           , ""            ,   "Kalianpur_1975"                                    ,   ""         ,  "EPSG:6146"  ,   "D_Kalianpur_1975"                             ,    ""                                       , "D_Kalianpur 1975"                                  , "Kalianpur75"                         },   // Kalianpur1975
    {   "KANDWALA"               , "MIF 45"     , ""            ,   "Kandawala"                                         ,   "MIF:45"   ,  "EPSG:6244"  ,   "D_Kandawala"                                  ,    "Kandawala"                              , ""                                  , ""                                    },   // KANDWALA
    {   "Karbala79/P"            , ""           , "KARBALA"     ,   "Karbala_1979_Polservice"                           ,   ""         ,  "EPSG:6743"  ,   "D_Karbala_1979_Polservice"                    ,    "D_Karbala 1979 Polservice"              , "D_Karbala1979"                     , "Karbala1979/P"                       },   // Karbala1979/P
    {   "Kerguelen"              , "MIF 46"     , ""            ,   "Kerguyelen_Island"                                 ,   "MIF:46"   ,  "EPSG:6698"  ,   "D_Kerguelen_Island_1949"                      ,    "Kerguelen Island"                       , "IGN 1962 Kerguelen"                , "Kerguelen_Island_1949"               },   // KERGUELN 
    {   "Kerguelen"              , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   "D_Kerguelen Island 1949"                      ,    ""                                       , ""                                  , "KERGUELN"                            },   // KERGUELN  second line 
    {   "KERTAU48"               , "MIF 47"     , "KERTAU"      ,   "Kertau"                                            ,   "MIF:47"   ,  "EPSG:6245"  ,   "D_Kertau"                                     ,    "Kertau 1948"                            , "Kertau 1968"                       , ""                                    },   // KERTAU48
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6751"  ,   "D_Kertau_RSO"                                 ,    "D_Kertau RSO"                           , "D_Kertau (RSO)"                    , ""                                    },   // KERTAU (RSO) NOTE: This datum and all KERTAU48 apply the same datum transformation so this is a functional duplicate yet EPSG introduces two entries ... to be studied
    {   "KertauDsmm"             , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // KertauDsmm Variation of KERTAU48
    {   "Kartasto66a"            , "MIF 1016"   , "FINKKJ"      ,   "KKJ"                                               ,   "MIF:1016" ,  "EPSG:6123"  ,   "D_KKJ"                                        ,    ""                                       , "Kartasto Koordinaati Jarjestelma 1966" , "Kartastokoordinaattijarjestelma (1966)"   },   // KKJ
    {   "Kartasto66a"            , ""           , ""            ,   "Finnish_KKJ"                                       ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , "D_Kartastokoordinaattijarjestelma_1966"     },   // KKJ second line
    {   "Kartasto66b"            , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6123-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // KKJ Variant
    {   "Kartasto66"             , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // KKJ Legacy
    {   "Final1958"              , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6132"  ,   "D_Final_Datum_1958"                           ,    ""                                       , "D_Final Datum 1958"                , ""                                    },   // KKJ Legacy
    {   "FINKKJ"                 , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // KKJ Legacy
    {   ""                       , ""           , ""            ,   "Kasai_1953"                                        ,   ""         ,  "EPSG:6696"  ,   "D_Kasai_1953"                                 ,    "D_Kasai 1953"                           , "D_Kasai_1955"                      , ""                                    },   // Kasai 1953 INVALID DATUM NO TRANSFORM DEFINED The alternate appears to be an ESRI typo encountered on the spatial-reference.org web site
    {   ""                       , ""           , ""            ,   "Katanga_1955"                                      ,   ""         ,  "EPSG:6695"  ,   "D_Katanga_1955"                               ,    ""                                       , "D_Katanga 1955"                    , ""                                    },   // Katanga 1955 INVALID DATUM NO TRANSFORM DEFINED
    {   ""                       , ""           , ""            ,   "Korean_Datum_1985"                                 ,   ""         ,  "EPSG:6162"  ,   "D_Korean_Datum_1985"                          ,    ""                                       , "D_Korean Datum 1985"               , ""                                    },   // Korean 1985 INVALID DATUM NO TRANSFORM DEFINED
    {   "Korean1995"             , ""           , ""            ,   "Korean_Datum_1995"                                 ,   ""         ,  "EPSG:6166"  ,   "D_Korean_Datum_1995"                          ,    "D_Korean Datum 1995"                    , "Korean Datum 1995"                 , "Korean95"                            },   // Korean95
    {   "Korean2000"             , ""           , ""            ,   "Korea_2000"                                        ,   ""         ,  "EPSG:6737"  ,   "D_Korea_2000"                                 ,    "D_Korea 2000"                           , "Geocentric datum of Korea"         , "D_Geocentric datum of Korea"         },   // Korean2000
    {   ""                       , ""           , ""            ,   "Kousseri"                                          ,   ""         ,  "EPSG:6198"  ,   "D_Kousseri"                                   ,    ""                                       , ""                                  , ""                                    },   // Kousseri INVALID DATUM NO TRANSFORM DEFINED
    {   "Kusaie51"               , "MIF 135"    , ""            ,   "Kusaie_1951"                                       ,   "MIF:135"  ,  "EPSG:6735"  ,   "D_Kusaie_1951"                                ,    "KUSAIE"                                 , "Kusaie_Astro_1951"                 , "Kusaie1951"                          },   // Kusaie1951 Add1 is OGC variant
    {   "Kusaie51"               , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   "D_Kusaie 1951"                                ,    ""                                       , ""                                  , ""                                    },   // Kusaie1951 second line
    {   "KuwaitOil"              , ""           , "KOC"         ,   "Kuwait_Oil_Company"                                ,   ""         ,  "EPSG:6246"  ,   "D_Kuwait_Oil_Company"                         ,    ""                                       , "D_Kuwait Oil Company"              , "KuwaitOilCo"                         },   // KuwaitOilCo
    {   "KuwaitUtility"          , ""           , ""            ,   "Kuwait_Utility"                                    ,   ""         ,  "EPSG:6319"  ,   "D_Kuwait_Utility"                             ,    ""                                       , "Kuwait Utility"                    , "D_Kuwait Utility"                    },   // KuwaitUtility
    {   "L-C5"                   , "MIF 48"     , ""            ,   "L_C_5_Astro"                                       ,   "MIF:48"   ,  ""           ,   "D_LC5_1961"                                   ,    "L.C. 5 Astro"                           , "LC5_1961"                          , "D_LC5 1961"                          },   // L-C5 UNKNOWN
    {   "LaCanoa/E"              , ""           , "LACANOA"     ,   "La_Canoa"                                          ,   ""         ,  "EPSG:6247"  ,   "D_La_Canoa"                                   ,    ""                                       , ""                                  , ""                                    },   // LaCanoa
    {   ""                       , ""           , ""            ,   "Lake"                                              ,   ""         ,  "EPSG:6249"  ,   "D_Lake"                                       ,    ""                                       , ""                                  , ""                                    },   // Lake INVALID DATUM NO TRANSFORM DEFINED
    {   ""                       , ""           , ""            ,   "Lao_1993"                                          ,   ""         ,  "EPSG:6677"  ,   "D_Lao_1993"                                   ,    ""                                       , ""                                  , ""                                    },   // Lao1993
    {   "Lao97"                  , ""           , ""            ,   "Lao_National_Datum_1997"                           ,   ""         ,  "EPSG:6678"  ,   "D_Lao_National_Datum_1997"                    ,    ""                                       , "D_Lao National Datum 1997"         , "Lao1997"                             },   // Lao1997
    {   "Latvia1992_1"           , ""           , ""            ,   "Latvia_1992"                                       ,   ""         ,  "EPSG:6661"  ,   "D_Latvia_1992"                                ,    ""                                       , "Latvia 1992"                       , "Latvia_1992"                         },   // Latvia1992
    {   "Leigon_1"               , "MIF 136"    , "LEIGON"      ,   "Leigon"                                            ,   "MIF:136"  ,  "EPSG:6250"  ,   "D_Leigon"                                     ,    ""                                       , ""                                  , ""                                    },   // Leigon
    {   "LePouce1934"            , ""           , ""            ,   "Le_Pouce_1934"                                     ,   ""         ,  "EPSG:6699"  ,   "D_Le_Pouce_1934"                              ,    ""                                       , ""                                  , ""                                    },   // LePouce1934
    {   "LIBERIA"                , "MIF 49"     , ""            ,   "Liberia_1964"                                      ,   "MIF:49"   ,  "EPSG:6251"  ,   "D_Liberia_1964"                               ,    "Liberia 1964"                           , ""                                  , ""                                    },   // LIBERIA
    {   "LISBOA-7P"              , ""           , "LISBONBESSEL",   "Lisbon_1890"                                       ,   ""         ,  "EPSG:6666"  ,   "D_Lisbon_1890"                                ,    ""                                       , "EPSG:106262"                       , "D_Datum_Lisboa_Bessel"               },   // Lisbon1890 Alternate is an ESRI introduction having name Lisboa Bessel
    {   "LISBOA-7P"              , ""           , ""            ,   "Lisbon1890_1"                                      ,   ""         ,  ""           ,   ""                                             ,    ""                                       , "Lisbon1890"                        , "Datum_Lisboa_Bessel"                 },   // Lisbon1890 Alternate is an ESRI introduction having name Lisboa Bessel
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:106263",   ""                                             ,    ""                                       , "D_Datum_Lisboa_Hayford"            , "Datum_Lisboa_Hayford"                },   // Lisbon Hayford Introduced by ESRI
    {   "Lisbon37/b"             , ""           , ""            ,   "Lisbon"                                            ,   ""         ,  "EPSG:6207"  ,   "D_Lisbon"                                     ,    "D_Lisbon_1937_(Lisbon)"                 , "Lisbon 1937"                       , "Lisbon37"                            },   // Lisbon37
    {   "Lithuania94"            , ""           , ""            ,   "Lithuania_1994"                                    ,   ""         ,  "EPSG:6126"  ,   "D_Lithuania_1994"                             ,    ""                                       , "Lithuania 1994 (ETRS89)"           , ""                                    },   // Lithuania94 
    {   "LittleCayman1961"       , ""           , ""            ,   "Little_Cayman_1961"                                ,   ""         ,  "EPSG:6726"  ,   "D_Little_Cayman_1961"                         ,    ""                                       , "Little Cayman 1961"                , ""                                    },   // LittleCayman1961
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6726-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // LittleCayman1961 variant
    {   "Locodjo1965"            , ""           , ""            ,   "Locodjo_1965"                                      ,   ""         ,  "EPSG:6142"  ,   "D_Locodjo_1965"                               ,    ""                                       , "D_Locodjo 1965"                    , "Locodjo65"                           },   // Locodjo
    {   ""                       , ""           , ""            ,   "Loma_Quintana"                                     ,   ""         ,  "EPSG:6288"  ,   "D_Loma_Quintana"                              ,    ""                                       , ""                                  , ""                                    },   // Loma Quintana INVALID DATUM NO TRANSFORM DEFINED
    {   ""                       , ""           , ""            ,   "Lome"                                              ,   ""         ,  "EPSG:6252"  ,   "D_Lome"                                       ,    ""                                       , ""                                  , ""                                    },   // Lome INVALID DATUM NO TRANSFORM DEFINED
    {   "Luxembourg30b"          , ""           , ""            ,   "Luxembourg_1930"                                   ,   ""         ,  "EPSG:6181"  ,   "D_Luxembourg_1930"                            ,    ""                                       , "Luxembourg 1930"                   , "Luxembourg30b"                       },   // Luxembourg30
    {   "LUZON"                  , "MIF 50"     , "LUZON11"     ,   "Luzon_Phillippines"                                ,   "MIF:50"   ,  "EPSG:6253"  ,   "D_Luzon_1911"                                 ,    "Luzon (Philippines)"                    , "Luzon_1911"                        , "Luzon 1911"                          },   // LUZON
    {   "LUZON-MI"               , "MIF 51"     , ""            ,   "Luzon_Mindanao_Island"                             ,   "MIF:51"   ,  ""           ,   ""                                             ,    "Luzon (Mindanao Island)"                , ""                                  , ""                                    },   // LUZON-MI UNKNOWN
    {   "Libyan2006_1"           , ""           , ""            ,   "Libyan_Geodetic_Datum_2006"                        ,   ""         ,  "EPSG:6754"  ,   "D_Libyan_Geodetic_Datum_2006"                 ,    "LGD2006"                                , "D_LGD2006"                         , "Libyan2006"                          },   // Libyan2006
    {   "Libyan2006_1"           , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   "D_Libyan Geodetic Datum 2006"                 ,    ""                                       , ""                                  , ""                                    },   // Libyan2006 second line
    {   "MADEIRA"                , "MIF 94"     , ""            ,   "Southeast_Base"                                    ,   "MIF:94"   ,  ""           ,   ""                                             ,    "Southeast Base"                         , ""                                  , ""                                    },   // MADEIRA 
    {   ""                       , ""           , ""            ,   "Madeira_1936"                                      ,   ""         ,  "EPSG:6185"  ,   "D_Madeira_1936"                               ,    ""                                       , ""                                  , ""                                    },   // Madeira 1936 INVALID DATUM NO TRANSFORM DEFINED
    {   ""                       , ""           , ""            ,   "Madzansua"                                         ,   ""         ,  "EPSG:6128"  ,   "D_Madzansua"                                  ,    ""                                       , ""                                  , ""                                    },   // Madzansua
    {   "MAGNA"                  , ""           , ""            ,   "MAGNA"                                             ,   ""         ,  "EPSG:6686"  ,   "D_MAGNA"                                      ,    "D_Marco Geocentrico Nacional de Referencia"  , "Marco Geocentrico Nacional de Referencia" , ""                             },   // MAGNA
    {   "MAHE1971"               , "MIF 52"     , "MAHE71"      ,   "Mahe_1971"                                         ,   "MIF:52"   ,  "EPSG:6256"  ,   "D_Mahe_1971"                                  ,    "Mahe 1971"                              , ""                                  , ""                                    },   // MAHE1971 
    {   "Makassar/E"             , ""           , "MAKASSAR"    ,   "Makassar"                                          ,   ""         ,  "EPSG:6257"  ,   "D_Makassar"                                   ,    ""                                       , "D_Makassar_(Jakarta)"              , ""                                    },   // Makassar
    {   "Malongo1987"            , ""           , ""            ,   "Malongo_1987"                                      ,   ""         ,  "EPSG:6259"  ,   "D_Malongo_1987"                               ,    ""                                       , ""                                  , "Malongo87"                           },   // Malongo87
    {   "Manoca"                 , ""           , "MANOKA"      ,   "Manoca"                                            ,   ""         ,  "EPSG:6260"  ,   "D_Manoca"                                     ,    ""                                       , ""                                  , ""                                    },   // Manoca
    {   "Manoca1962"             , ""           , ""            ,   "Manoca_1962"                                       ,   ""         ,  "EPSG:6193"  ,   "D_Manoca_1962"                                ,    ""                                       , ""                                  , "Manoca62"                            },   // Manoca62
    {   "Marshalls1960"          , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6732"  ,   "D_MarshallIslands1960"                        ,    ""                                       , "D_Marshall_Islands_1960"           , "Marshalls1960"                       },   // Marshalls1960
    {   "Marcus52"               , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6711"  ,   "D_MarcusIsland1952"                           ,    ""                                       , "D_Marcus_Island_1952"              , "Marcus1952"                          },   // Marcus1952
    {   "MARCO"                  , "MIF 53"     , ""            ,   "Marco_Astro"                                       ,   "MIF:53"   ,  ""           ,   ""                                             ,    "Marco Astro"                            , ""                                  , ""                                    },   // MARCO UNKNOWN
    {   "Mars_2000"              , ""           , ""            ,   "Mars_2000"                                         ,   ""         ,  ""           ,   "D_Mars_2000"                                  ,    ""                                       , ""                                  , ""                                    },   // Mars_2000 for demo only
    {   "Mars_2000_Sphere"       , ""           , ""            ,   "Mars_2000_Sphere"                                  ,   ""         ,  ""           ,   "D_Mars_2000_Sphere"                           ,    ""                                       , ""                                  , ""                                    },   // Mars_2000 Sphere (equatorial radius) for demo only
    {   "MASSAWA"                , "MIF 54"     , ""            ,   "Massawa"                                           ,   "MIF:54"   ,  ""           ,   "D_Massawa"                                    ,    "Massawa"                                , ""                                  , ""                                    },   // MASSAWA
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6262"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // MASSAWA variant
    {   "Maupiti83"              , ""           , ""            ,   "Maupiti_1983"                                      ,   ""         ,  "EPSG:6692"  ,   "D_Maupiti_1983"                               ,    ""                                       , ""                                  , ""                                    },   // Maupiti83
    {   "Mauritania1999"         , ""           , ""            ,   "Mauritania_1999"                                   ,   ""         ,  "EPSG:6702"  ,   "D_Mauritania_1999"                            ,    ""                                       , "D_Mauritania 1999"                 , ""                                    },   // Mauritania1999
    {   "MERCHICH"               , "MIF 55"     , ""            ,   "Merchich"                                          ,   "MIF:55"   ,  "EPSG:6261"  ,   "D_Merchich"                                   ,    "Merchich"                               , ""                                  , "MERCHICH-01"                         },   // MERCHICH
    {   "MGI"                    , "MIF 128"    , "MGIBESS"     ,   "MGI"                                               ,   "MIF:128"  ,  "EPSG:6312"  ,   "D_MGI"                                        ,    "Hermanns_Kogel"                         , "D_Militar-Geographische_Institut_(Ferro)", "D_Hermannskogel"               },   // MGI Add1 is OGC variant
    {   "MGI"                    , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , "D_Militar-Geographische Institut" , "D_MGI 1901"                           },   // MGI variant
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6312-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // MGI variant
    {   "Mhast"                  , ""           , ""            ,   "Mhast"                                             ,   ""         ,  "EPSG:6264"  ,   "D_Mhast"                                      ,    ""                                       , ""                                  , ""                                    },   // Mhast
    {   ""                       , ""           , ""            ,   "Mhast_Onshore"                                     ,   ""         ,  "EPSG:6704"  ,   "D_Mhast_Onshore"                              ,    ""                                       , ""                                  , ""                                    },   // Mhast onshore INVALID DATUM NO TRANSFORM DEFINED
    {   ""                       , ""           , ""            ,   "Mhast_Offshore"                                    ,   ""         ,  "EPSG:6705"  ,   "D_Mhast_Offshore"                             ,    ""                                       , ""                                  , ""                                    },   // Mhast offshore INVALID DATUM NO TRANSFORM DEFINED
    {   "MICHIGAN"               , "MIF 73"     , ""            ,   "NAD_27_Michigan"                                   ,   "MIF:73"   ,  "EPSG:6268"  ,   "D_North_American_Michigan"                    ,    "NAD 27 (Michigan)"                      , "NAD Michigan"                      , "North_American_Michigan"             },   // MICHIGAN Additional is GDAL
    {   "MIDWAY"                 , "MIF 56"     , ""            ,   "Midway_Astro_1961"                                 ,   "MIF:56"   ,  ""           ,   "D_Midway_1961"                                ,    "Midway Astro 1961"                      , "Midway_1961"                       , ""                                    },   // MIDWAY 
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6727"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // MIDWAY variant
    {   "MINNA"                  , "MIF 57"     , ""            ,   "Minna"                                             ,   "MIF:57"   ,  "EPSG:6263"  ,   "D_Minna"                                      ,    "Minna"                                  , ""                                  , ""                                    },   // MINNA 
    {   "Miquelon1950"           , ""           , ""            ,   "Saint_Pierre_et_Miquelon_1950"                     ,   ""         ,  "EPSG:6638"  ,   "D_Saint_Pierre_et_Miquelon_1950"              ,    ""                                       , ""                                  , "Miquelon50"                          },   // Miquelon50
    {   ""                       , ""           , ""            ,   "Mhast_1951"                                        ,   ""         ,  "EPSG:6703"  ,   "D_Mhast_1951"                                 ,    ""                                       , ""                                  , ""                                    },   // Missao Hidrografico Angola y Sao Tome (Mhast) 1951 INVALID DATUM NO TRANSFORM DEFINED
    {   "Montserrat1958"         , "MIF 137"    , ""            ,   "Montserrat_1958"                                   ,   "MIF:137"  ,  "EPSG:6604"  ,   "D_Montserrat_1958"                            ,    "Montserrat_1958"                        , "Montserrat_Astro_1958"             , "Montserrat58"                        },   // Montserrat58 Add1 is OGC variant
    {   "Moorea87"               , ""           , ""            ,   "Moorea_1987"                                       ,   ""         ,  "EPSG:6691"  ,   "D_Moorea_1987"                                ,    ""                                       , ""                                  , ""                                    },   // Moorea87
    {   "MOP78"                  , ""           , ""            ,   "MOP78"                                             ,   ""         ,  "EPSG:6639"  ,   "D_MOP78"                                      ,    ""                                       , ""                                  , ""                                    },   // MOP78
    {   ""                       , ""           , ""            ,   "Mount_Dillon"                                      ,   ""         ,  "EPSG:6157"  ,   "D_Mount_Dillon"                               ,    ""                                       , ""                                  , ""                                    },   // Mount Dillon INVALID DATUM NO TRANSFORM DEFINED
    {   "Moznet"                 , ""           , ""            ,   "Moznet"                                            ,   ""         ,  "EPSG:6130"  ,   "D_Moznet"                                     ,    ""                                       , "Moznet (ITRF94)"                   , ""                                    },   // Moznet
    {   "Mporaloko_1"            , "MIF 138"    , "MPORO"       ,   "Mporaloko"                                         ,   "MIF:138"  ,  "EPSG:6266"  ,   "D_Mporaloko"                                  ,    ""                                       , "D_M'poraloko"                      , ""                                    },   // Mporaloko
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6266-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Mporaloko variant
    {   "NAD27"                  , "MIF 62"     , ""            ,   "NAD_1927"                                          ,   "MIF:62"   ,  "EPSG:6267"  ,   "D_North_American_1927"                        ,    "NAD 27 (Continental US)"                , "NAD27-48"                          , "NAD 1927"                            },   // NAD27 The additional is a variation of CSMAP using multiple regression, second additional is unknown added by Mathieu St-Pierre
    {   "NAD27"                  , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , "North_American_Datum_1927"         , "North_American_1927"                 },   // NAD27 The additional is a variation of CSMAP using multiple regression, second additional is unknown added by Mathieu St-Pierre
    {   "NAD27"                  , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , "North American Datum 1927"         , ""                                    },   // NAD27 The additional is a variation of CSMAP WKT Generation
    {   "NAD27"                  , "MIF 66"     , ""            ,   "NAD_27_Canada"                                     ,   "MIF:66"   ,  ""           ,   ""                                             ,    "NAD 27 (Canada)"                        , ""                                  , ""                                    },   // NAD27 for Canada is mapped to NAD27 as it is the prefered choice.
    {   "NAD27"                  , "MIF 63"     , ""            ,   "NAD_27_Alaska"                                     ,   "MIF:63"   ,  ""           ,   ""                                             ,    "NAD 27 (Alaska)"                        , ""                                  , ""                                    },   // NAD27 for Alaska (we remap to NAD 27 ordinary)
    {   "NAD27-AK"               , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // NAD27-AK
    {   "NAD27-CN"               , "MIF 66"     , ""            ,   "NAD_27_Canada"                                     ,   "MIF:66"   ,  ""           ,   ""                                             ,    "NAD 27 (Canada)"                        , ""                                  , ""                                    },   // NAD27-CN 
    {   "NAD27-CZ"               , "MIF 67"     , ""            ,   "NAD_27_Canal_Zone"                                 ,   "MIF:67"   ,  ""           ,   ""                                             ,    "NAD 27 (Canal Zone)"                    , ""                                  , ""                                    },   // NAD27-CZ 
    {   "NAD27-CB"               , "MIF 68"     , ""            ,   "NAD_27_Caribbean"                                  ,   "MIF:68"   ,  ""           ,   ""                                             ,    "NAD 27 (Caribbean)"                     , ""                                  , ""                                    },   // NAD27-CB 
    {   "NAD27-CA"               , "MIF 69"     , ""            ,   "NAD_27_Central_America"                            ,   "MIF:69"   ,  ""           ,   ""                                             ,    "NAD 27 (Central America)"               , ""                                  , ""                                    },   // NAD27-CA 
    {   "NAD27-GR"               , "MIF 71"     , ""            ,   "NAD_27_Greenland"                                  ,   "MIF:71"   ,  ""           ,   ""                                             ,    "NAD 27 (Greenland)"                     , ""                                  , ""                                    },   // NAD27-GR 
    {   "NAD27-MX"               , "MIF 72"     , ""            ,   "NAD_27_Mexico"                                     ,   "MIF:72"   ,  ""           ,   ""                                             ,    "NAD 27 (Mexico)"                        , ""                                  , ""                                    },   // NAD27-MX 
    {   "NAD27HII"               , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // NAD27HII NAD27 with International Ellipsoid for HI UTM.
    {   ""                       , ""           , "NAD27A76"    ,   "NAD_1927_Definition_1976"                          ,   ""         ,  "EPSG:6608"  ,   "D_NAD_1927_Definition_1976"                   ,    ""                                       , "D_North_American_Datum_1927_(1976)", ""                                    },   // NAD27/76 ... implemented in CSMAP as NAD27 using Ontario tables
    {   ""                       , ""           , ""            ,   "NAD_1927_CGQ77"                                    ,   ""         ,  "EPSG:6609"  ,   "D_NAD_1927_CGQ77"                             ,    ""                                       , "D_North_American_Datum_1927_(CGQ77)", ""                                   },   // NAD27/CGQ77 ... implemented in CSMAP as NAD27 using specific tables
    {   "NAD83"                  , "MIF 74"     , ""            ,   "North_American_Datum_1983"                         ,   "MIF:74"   ,  "EPSG:6269"  ,   "D_North_American_1983"                        ,    "NAD 83"                                 , "NAD 1983"                          , "NAD 83 (Continental US)"             },   // NAD83 Additional1 unknown source Additional 2 Oracle variant
    {   "NAD83"                  , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    "NAD 83 (Alaska)"                        , "North American Datum 1983"         , ""                                    },   // NAD83 Additional line since Oracle can be very creative
    {   "NAD83/2011"             , ""           , ""            ,   "NAD_1983_2011"                                     ,   ""         ,  "EPSG:1116"  ,   "D_NAD_1983_2011"                              ,    ""                                       , "D_NAD83_National_Spatial_Reference_System_2011", "NAD83_National_Spatial_Reference_System_2011" },   // NAD83/2011
    {   "NAD83/2011"             , ""           , ""            ,   "NAD 1983 NSRS2011"                                 ,   ""         ,  ""           ,   "D_NAD 1983 NSRS2011"                          ,    ""                                       , "NAD83 (National Spatial Reference System 2011)", ""                                    },   // NAD83/2011 line 2
    {   "NSRS07"                 , ""           , ""            ,   "NAD_1983_2007"                                     ,   ""         ,  "EPSG:6759"  ,   "D_NAD_1983_2007"                              ,    ""                                       , "D_NAD83_National_Spatial_Reference_System_2007", "NAD83_National_Spatial_Reference_System_2007"   },   // NAD83/2007 aka NSRS2007
    {   "NSRS07"                 , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   "D_NAD83 (National Spatial Reference System 2007)", "D_NAD_1983_NSRS2007"                    , "D_NAD83_National_Spatial_Reference_System_2007", "NAD83_National_Spatial_Reference_System_2007"   },   // NAD83/2007 aka NSRS2007
    {   "NSRS07"                 , ""           , ""            ,   "NAD 1983 NSRS2007"                                 ,   ""         ,  ""           ,   "D_NAD 1983 NSRS2007"                          ,    "D_NSRS2007"                             , "NAD83(NSRS2007)"                   , "NSRS2007"                            },   // NAD83/2007 aka NSRS2007 line 2
    {   "NAD83/CORS96"           , ""           , ""            ,   "NAD_1983_CORS96"                                   ,   ""         ,  ""           ,   "D_NAD_1983_CORS96"                            ,    ""                                       , ""                                  , ""                                    },   // NAD83/CORS96
    {   "Nahrwan1934"            , ""           , ""            ,   "Nahrwan_1934"                                      ,   ""         ,  "EPSG:6744"  ,   "D_Nahrwan_1934"                               ,    ""                                       , ""                                  , "D_Nahrwan_1934"                      },   // Nahrwan 1934 
    {   "Nakhl-eGhanem"          , ""           , ""            ,   "Nakhl-e_Ghanem"                                    ,   ""         ,  "EPSG:6693"  ,   "D_Nakhl-e_Ghanem"                             ,    ""                                       , ""                                  , ""                                    },   // Nakhl-eGhanem
    {   "Naparima1955"           , ""           , ""            ,   "Naparima_1955"                                     ,   ""         ,  "EPSG:6158"  ,   "D_Naparima_1955"                              ,    "Naparima, BWI"                          , ""                                  , "NAPARIMA-O"                          },   // NAPARIMA55
    {   "Naparima1972"           , "MIF 61"     , ""            ,   "Naparima_1972"                                     ,   "MIF:61"   ,  "EPSG:6271"  ,   "D_Naparima_1972"                              ,    ""                                       , ""                                  , ""                                    },   // NAPARIMA72
    {   "NAPARIMA"               , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // NAPARIMA is a variant of Naparima72 using a different parameter values 
    {   "NEA74Noumea"            , ""           , ""            ,   "NEA74_Noumea"                                      ,   ""         ,  "EPSG:6644"  ,   "D_NEA74_Noumea"                               ,    ""                                       , ""                                  , ""                                    },   // NEA74Noumea
    {   "NGO-I"                  , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // NGO-I Norway Zone 1
    {   "NGO-II"                 , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // NGO-I Norway Zone 2
    {   "NGO-III"                , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // NGO-I Norway Zone 3
    {   "NGO-IV"                 , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // NGO-I Norway Zone 4
    {   "NGO-V"                  , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // NGO-I Norway Zone 5
    {   "NGO-VI"                 , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // NGO-I Norway Zone 6
    {   "NGO-VII"                , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // NGO-I Norway Zone 7
    {   "NGO-VIII"               , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // NGO-I Norway Zone 8
    {   "NGO48b"                 , ""           , ""            ,   "NGO_1948"                                          ,   ""         ,  "EPSG:6273"  ,   "D_NGO_1948"                                   ,    ""                                       , "NGO_1948"                          , "NGO48"                               },   // NGO 1948
    {   "NHRWN-O"                , "MIF 58"     , ""            ,   "Nahrwan_Masirah_Island"                            ,   "MIF:58"   ,  ""           ,   ""                                             ,    "Nahrwan (Masirah Island)"               , ""                                  , "Nahrwan 1967"                        },   // NHRWN-O 
    {   "Nahrwan67.UAE-1"        , "MIF 59"     , "NAHRWAN"     ,   "Nahrwan_Un_Arab_Emirates"                          ,   "MIF:59"   ,  ""           ,   "D_Nahrwan_1967"                               ,    "Nahrwan (Un. Arab Emirates)"            , ""                                  , "NHRWN-U"                             },   // NHRWN-U 
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6270"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // NHRWN-U variant
    {   "Nahrwan67.SA"           , "MIF 60"     , ""            ,   "Nahrwan_Saudi_Arabia"                              ,   "MIF:60"   ,  ""           ,   ""                                             ,    "Nahrwan (Saudi Arabia)"                 , ""                                  , "NHRWN-S"                             },   // NHRWN-S 
    {   "NordSahara1959"         , "MIF 139"    , ""            ,   "Nord_Sahara_1959"                                  ,   "MIF:139"  ,  "EPSG:6307"  ,   "D_Nord_Sahara_1959"                           ,    ""                                       , "NORD-SAHARA"                       , "NordSahara59"                        },   // NordSahara59
    {   "Nouakchott65"           , ""           , ""            ,   "Nouakchott_1965"                                   ,   ""         ,  "EPSG:6680"  ,   "D_Nouakchott_1965"                            ,    ""                                       , ""                                  , "Nouakchott1965"                      },   // Nouakchott1965
    {   ""                       , ""           , ""            ,   "NSWC_9Z_2"                                         ,   ""         ,  "EPSG:6276"  ,   "D_NSWC_9Z_2"                                  ,    ""                                       , ""                                  , ""                                    },   // NSWC 9Z 2 INVALID DATUM NO TRANSFORM DEFINED
    {   "NTF"                    , "MIF 107"    , "NTFPARG"     ,   "NTF"                                               ,   "MIF:107"  ,  "EPSG:6275"  ,   "D_NTF"                                        ,    "NTF"                                    , "NTF (Greenwich meridian)"          , "Nouvelle_Triangulation_Francaise"    },   // NTF
    {   "NTF"                    , ""           , "NTFPARIS"    ,   ""                                                  ,   ""         ,  ""           ,   "D_Nouvelle Triangulation Francaise"           ,    ""                                       , ""                                  , ""                                    },   // NTF second line
    {   "NTF-3P"                 , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // NTF-3P 3 Parameter variation of NTF
    {   "NTF"                    , "MIF 1002"   , ""            ,   "NTF_Paris_Meridian"                                ,   "MIF:1002" ,  "EPSG:6807"  ,   ""                                             ,    "NTF (Paris meridian)"                   , ""                                  , ""                                    },   // NTF-PARIS (we use NTF anyway ass the prime meridian is set in projection not datum
    {   "NtlGeodeticNet"         , ""           , ""            ,   "NGN"                                               ,   ""         ,  "EPSG:6318"  ,   "D_NGN"                                        ,    ""                                       , "D_National_Geodetic_Network"       , ""                                    },   // NtlGeodeticNet
    {   "NZGD2000"               , "MIF 117"    , "NZGD2000"    ,   "NZGD_2000"                                         ,   "MIF:117"  ,  "EPSG:6167"  ,   "D_NZGD_2000"                                  ,    ""                                       , "EPSG:106265"                       , "New Zealand Geodetic Datum 2000"     },   // NZGD2000 Alternate is introduced by ESRI
    {   "NZGD49"                 , "MIF 31"     , "NZGD49"      ,   "New_Zealand_GD49"                                  ,   "MIF:31"   ,  "EPSG:6272"  ,   "D_New_Zealand_1949"                           ,    "Geodetic Datum 1949"                    , "GD1949"                            , "New Zealand Geodetic Datum 1949"     },   // NZGD49
    {   "NZGD49-7P"              , "MIF 1010"   , ""            ,   "NZGD_7_Param_49"                                   ,   "MIF:1010" ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // NZGD49-7P 7 Parameters variation of NZGD49
    {   "Observatario07"         , "MIF 75"     , ""            ,   "Observatorio_1966"                                 ,   "MIF:75"   ,  "EPSG:6129"  ,   "D_Observatario"                               ,    "Observatorio 1966"                      , "Observatorio"                      , "OBSRV66"                             },   // OBSRV66
    {   "Observatorio65"         , ""           , ""            ,   "Observatorio_1965"                                 ,   ""         ,  ""           ,   "D_Observatorio_Meteorologico_1965"            ,    ""                                       , ""                                  , ""                                    },   // 
    {   "Ocotepeque35c"          , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   "D_Ocotepeque_1935"                            ,    ""                                       , ""                                  , ""                                    },   // 
    {   "OLD-EGYP"               , "MIF 76"     , "EGYPT07"     ,   "Old_Egyptian"                                      ,   "MIF:76"   ,  "EPSG:6229"  ,   "D_Egypt_1907"                                 ,    "Old Egyptian"                           , "Egypt_1907"                        , "OLD_EGYP"                            },   // OLD-EGYP
    {   "OldHawaiian"            , "MIF 77"     , ""            ,   "Old_Hawaiian"                                      ,   "MIF:77"   ,  "EPSG:6135"  ,   "D_Old_Hawaiian"                               ,    "Old Hawaiian"                           , ""                                  , "OLDHI"                               },   // OLDHI
    {   "OSGB/OSTN15"            , "MIF 79"     , ""            ,   "OSGB_1936"                                         ,   "MIF:79"   ,  "EPSG:6277"  ,   "D_OSGB_1936"                                  ,    "Ordnance Survey Great Brit"             , "Ordinance Survey Great Brit"       , "Ord. Survey G. Britain 1936"         },   // OSGB 1936
    {   "OSGB/OSTN15"            , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    "OSGB"                                   , "OSGB-7P-2"                         , "OSGB-MOD"                            },   // OSGB 1936 second line
    {   "OSGB/OSTN02"            , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // OSGB 1936 (line for 2002 variation)
    {   ""                       , ""           , "OSGB70"      ,   "OSGB_1970_SN"                                      ,   ""         ,  "EPSG:6278"  ,   "D_OSGB_1970_SN"                               ,    "Ordnance Survey Great Brit"             , ""                                  , ""                                    },   // OSGB 1970 (SN)
    {   ""                       , ""           , "OSSN80"      ,   "OS_SN_1980"                                        ,   ""         ,  "EPSG:6279"  ,   "D_OS_SN_1980"                                 ,    ""                                       , ""                                  , ""                                    },   // OS (SN) 1980 INVALID DATUM NO TRANSFORM DEFINED
    {   "OSGB-7P"                , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // OSGB variant 2
    {   "OSNI52"                 , ""           , ""            ,   "OSNI_1952"                                         ,   ""         ,  "EPSG:6188"  ,   "D_OSNI_1952"                                  ,    ""                                       , ""                                  , ""                                    },   // OSNI52
    {   "Padang1884"             , ""           , "PADANG"      ,   "Padang_1884"                                       ,   ""         ,  "EPSG:6280"  ,   "D_Padang_1884"                                ,    ""                                       , "D_Padang_1884_(Jakarta)"           , "D_Padang"                            },   // Padang
    {   "Palestine23"            , ""           , "PALEST23"    ,   "Palestine_1923"                                    ,   ""         ,  "EPSG:6281"  ,   "D_Palestine_1923"                             ,    "Palestine 1923"                         , ""                                  , ""                                    },   // Palestine1923
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6281-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Palestine1923 variant
    {   "ParametropZemp1990"     , ""           , ""            ,   "Parametrop_Zemp_1990"                              ,   ""         ,  "EPSG:6740"  ,   "D_Parametrop_Zemp_1990"                       ,    ""                                       , "D_Parametrop Zemp 1990"            , ""                                    },   // ParametropZemp1990
    {   ""                       , "MIF 1012"   , ""            ,   "Russia_PZ90"                                       ,   "MIF:1012" ,  "EPSG:6740-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // ParametropZemp1990 variant
    {   "PDOSurvey93b"           , ""           , ""            ,   "PDO_1993"                                          ,   ""         ,  "EPSG:6134"  ,   "D_PDO_1993"                                   ,    "D_PDO_Survey_Datum_1993"                , "D_PDO Survey Datum 1993"           , "PDOSurvey93"                         },   // PDO1993
    {   "Perroud1950"            , ""           , ""            ,   "Pointe_Geologie_Perroud_1950"                      ,   ""         ,  "EPSG:6637"  ,   "D_Pointe_Geologie_Perroud_1950"               ,    ""                                       , ""                                  , "Perroud50"                           },   // Perroud50
    {   "Petrels1972"            , ""           , ""            ,   "Petrels_1972"                                      ,   ""         ,  "EPSG:6636"  ,   "D_Petrels_1972"                               ,    ""                                       , ""                                  , "Petrels72"                           },   // Petrels72
    {   "PhoenixIs66"            , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6716"  ,   "D_PhoenixIslands1966"                         ,    ""                                       , "D_Phoenix_Islands_1966"            , "Phoenix Is1966"                      },   // Phoenix Is1966
    {   "PITCAIRN"               , "MIF 81"     , ""            ,   "Pitcairn_Astro_1967"                               ,   "MIF:81"   ,  ""           ,   "D_Pitcairn_1967"                              ,    "Pitcairn Astro 1967"                    , "Pitcairn_1967"                     , ""                                    },   // PITCAIRN
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6729"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // PITCAIRN variant
    {   ""                       , ""           , ""            ,   "Pitcairn_2006"                                     ,   ""         ,  "EPSG:6763"  ,   "D_Pitcairn_2006"                              ,    ""                                       , ""                                  , ""                                    },   // Pitcairn 2006
    {   "Point1958"              , "MIF 141"    , ""            ,   "Point_58"                                          ,   "MIF:141"  ,  "EPSG:6620"  ,   "D_Point_58"                                   ,    ""                                       , "POINT-58"                          , "Point58"                             },   // Point58
    {   "PointeNoire60"          , "MIF 142"    , "PTNOIRE"     ,   "Pointe_Noire"                                      ,   "MIF:142"  ,  "EPSG:6282"  ,   "D_Pointe_Noire"                               ,    "D_Congo_1960_Pointe_Noire"              , "PTE-NOIR"                          , "PointeNoire"                         },   // PointeNoire
    {   "PortoSanto"             , "MIF 143"    , ""            ,   "Porto_Santo_1936"                                  ,   "MIF:143"  ,  "EPSG:6615"  ,   "D_Porto_Santo_1936"                           ,    ""                                       , "EPSG:106247"                       , ""                                    },   // PortoSanto Alternate is an ESRI introduction
    {   "PortoSanto95"           , ""           , ""            ,   "Porto_Santo_1995"                                  ,   ""         ,  "EPSG:6663"  ,   "D_Porto_Santo_1995"                           ,    ""                                       , ""                                  , "PortoSanto95"                        },   // PortoSanto1995
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6663-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // PortoSanto1995 variant
    {   "PosGAr"                 , ""           , ""            ,   "POSGAR"                                            ,   ""         ,  "EPSG:6172"  ,   "D_POSGAR"                                     ,    ""                                       , ""                                  , ""                                    },   // PosGAr Posiciones Geodesicas Argentinas
    {   "PosGAr94"               , ""           , ""            ,   "POSGAR_1994"                                       ,   ""         ,  "EPSG:6694"  ,   "D_POSGAR_1994"                                ,    ""                                       , "D_Posiciones Geodesicas Argentinas 1994", "D_Posiciones_Geodesicas_Argentinas_1994"},   // PosGar94 
    {   "PosGar1998"             , ""           , ""            ,   "POSGAR_1998"                                       ,   ""         ,  "EPSG:6190"  ,   "D_POSGAR_1998"                                ,    "D_Posiciones Geodesicas Argentinas 1998", "PGA98"                             , "PosGAr98"                            },   // PosGAr98 Posiciones Geodesicas Argentinas 1998 identical or PGA98 a CSMAP alternate
    {   ""                       , ""           , "POTSDAM"     ,   "Potsdam_1983"                                      ,   ""         ,  "EPSG:6746"  ,   "D_Potsdam_1983"                               ,    ""                                       , ""                                  , ""                                    },   // Potsdam Datum/83
    {   "PRS92"                  , ""           , "PRS92"       ,   "Philippine_Reference_System_1992"                  ,   ""         ,  "EPSG:6683"  ,   "D_Philippine_Reference_System_1992"           ,    ""                                       , "D_Phillipine_Reference_System_1992", "D_Philippine Reference System 1992"  },   // PRS92 The alternate is a typo introduced by spatial-reference.org
    {   "PSAD56"                 , "MIF 82"     , "PSAD56"      ,   "Provisional_South_American"                        ,   ""         ,  "EPSG:6248"  ,   "D_Provisional_S_American_1956"                ,    "Provisional South American"             , "Provisional South American Datum 1956", ""                                 },   // PSAD56
    {   "PSC63"                  , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // PSC63 Variation of HITO
    {   "PuertoRico"             , "MIF 83"     , ""            ,   "Puerto_Rico"                                       ,   ""         ,  "EPSG:6139"  ,   "D_Puerto_Rico"                                ,    "Puerto Rico"                            , ""                                  , "PRVI"                                },   // PRVI (grid shift files NADCON)
    {   "Pulkovo1942"            , "MIF 1001"   , "PULKOVO"     ,   "Pulkovo_1942"                                      ,   "MIF:1001" ,  ""           ,   "D_Pulkovo_1942"                               ,    "Pulkovo 1942"                           , ""                                  , ""                                    },   // Pulkovo1942
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6284"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Pulkovo1942 variant 1 
    {   "Pulkovo42/2"            , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Pulkovo1942 Variant 2 using operation EPSG:1679
    {   ""                       , "MIF 1013"   , ""            ,   "Russia_SK42"                                       ,   "MIF:1013" ,  "EPSG:6284-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Pulkovo1942 Variant 3 using operation EPSG:1267
    {   "Pulkovo42/58b"          , ""           , ""            ,   "Pulkovo_1942_Adj_1958"                             ,   ""         ,  "EPSG:6179"  ,   "D_Pulkovo_1942_Adj_1958"                      ,    "D_Pulkovo_1942(58)"                     , "D_Pulkovo 1942(58)"                , "Pulkovo42/58"                        },   // Pulkovo42/58
    {   "Pulkovo42/58b"          , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , "D_Pulkovo 1942/58"                 , ""                                    },   // Pulkovo42/58 line 2
    {   "Pulkovo42/83b"          , ""           , ""            ,   "Pulkovo_1942_Adj_1983"                             ,   ""         ,  "EPSG:6178"  ,   "D_Pulkovo_1942_Adj_1983"                      ,    "D_Pulkovo_1942(83)"                     , "D_Pulkovo 1942(83)"                , "Pulkovo42/83"                        },   // Pulkovo42/83
    {   "Pulkovo42/83b"          , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , "D_Pulkovo 1942/83"                 , ""                                    },   // Pulkovo42/83 line 2
    {   ""                       , "MIF 1014"   , ""            ,   "Pulkovo_1995"                                      ,   "MIF:1014" ,  "EPSG:6200"  ,   "D_Pulkovo_1995"                               ,    ""                                       , "Russia_SK95"                       , ""                                    },   // Pulkovo 1995 Add 1 is OGC variant
    {   "QatarNtl95"             , "MIF 84"     , ""            ,   "Qatar_National"                                    ,   "MIF:84"   ,  "EPSG:6614"  ,   "D_QND_1995"                                   ,    "Qatar National"                         , "QND_1995"                          , "D_Qatar_National_Datum_1995"         },   // QatarNtl95
    {   "Qatar1974"              , ""           , "QATAR"       ,   "Qatar"                                             ,   ""         ,  "EPSG:6285"  ,   "D_Qatar"                                      ,    ""                                       , "D_Qatar_1974"                      , "Qatar1974"                           },   // Qatar74 
    {   "QATAR"                  , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // QATAR Legacy 
    {   "QATAR48"                , ""           , "QATAR51"     ,   "Qatar_1948"                                        ,   ""         ,  "EPSG:6286"  ,   "D_Qatar_1948"                                 ,    ""                                       , "Qatar 1948"                        , ""                                    },   // QATAR48 
    {   "QORNOQ"                 , "MIF 85"     , "QORNOQ"      ,   "Qornoq"                                            ,   "MIF:85"   ,  ""           ,   "D_Qornoq"                                     ,    "Qornoq"                                 , ""                                  , ""                                    },   // QORNOQ 
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6287"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // QORNOQ variant
    {   "Qornoq1927"             , ""           , ""            ,   "Qornoq_1927"                                       ,   ""         ,  "EPSG:6194"  ,   "D_Qornoq_1927"                                ,    ""                                       , ""                                  , "Qornoq27"                            },   // Qornoq27
    {   "Qornoq1927-7P"          , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6194-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Qornoq27 variant
    {   "Rassadiran_1"           , ""           , ""            ,   "Rassadiran"                                        ,   ""         ,  "EPSG:6153"  ,   "D_Rassadiran"                                 ,    ""                                       , ""                                  , ""                                    },   // Rassadiran
    {   ""                       , ""           , ""            ,   "Rauenberg_1983"                                    ,   ""         ,  "EPSG:6745"  ,   "D_Rauenberg_1983"                             ,    ""                                       , ""                                  , ""                                    },   // Rauenberg Datum/83
    {   "RDN2008"                , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:1132"  ,   "D_RDN2008"                                    ,    ""                                       , ""                                  , "Rete Dinamica Nazionale 2008"        },   // RDN2008 (Italy)
    {   "REGVEN-New"             , ""           , ""            ,   "REGVEN"                                            ,   ""         ,  "EPSG:6189"  ,   "D_REGVEN"                                     ,    ""                                       , "D_Red Geodesica Venezolana"        , "D_Red_Geodesica_Venezolana"          },   // REGVEN
    {   ""                       , ""           , "BELG50"      ,   "Belge_1950"                                        ,   ""         ,  "EPSG:6215"  ,   "D_Belge_1950"                                 ,    ""                                       , "BELG50"                            , ""                                    },   // Reseau National Belge 1950 INVALID DATUM NO TRANSFORM DEFINED
    {   "REUNION"                , "MIF 86"     , ""            ,   "Reunion"                                           ,   "MIF:86"   ,  ""           ,   "D_Reunion_1947"                               ,    "Reunion"                                , "D_Reunion"                         , "D_Piton_des_Neiges"                  },   // REUNION 1947 also named "Piton des Neiges"
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6626"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // REUNION variant
    {   "Reunion1992"            , ""           , ""            ,   "RGR_1992"                                          ,   ""         ,  "EPSG:6627"  ,   "D_RGR_1992"                                   ,    ""                                       , "D_Reseau Geodesique de la Reunion 1992", "Reunion92"                       },   // Reunion92
    {   "Reykjavik"              , ""           , ""            ,   "Reykjavik_1900"                                    ,   ""         ,  ""           ,   "D_Reykjavik_1900"                             ,    ""                                       , ""                                  , ""                                    },   // Reykjavik
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6657"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Reykjavik variant
    {   "RGF93"                  , ""           , "RGF93"       ,   "RGF_1993"                                          ,   ""         ,  "EPSG:6171"  ,   "D_RGF_1993"                                   ,    ""                                       , "EPSG:106264"                       , "RGF_1993"                            },   // RGF93 Alternate introduce by ESRI
    {   "RGF93"                  , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , "D_Reseau geodesique francais 1993" , "Reseau Geodesique Francais 1993"     },   // RGF93 second line
    {   "EPSG:1036"              , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , "D_Reseau_Geodesique_de_Mayotte_2004" , "Reseau Geodesique de Mayotte 2004"     },   // Mayotte   
    {   "RGN/91-93"              , ""           , ""            ,   "Reseau_Geodesique_de_Nouvelle_Caledonie_1991-93"   ,   ""         ,  "EPSG:6749"  ,   "D_Reseau_Geodesique_de_Nouvelle_Caledonie_1991-93", "D_Reseau Geodesique de Nouvelle Caledonie 91-93"                           , "D_RGNC91-93"                       , "Reseau Geodesique Nouvelle Caledonie 1991"  },   // RGN/91-93
    {   "RGP-Francaise"          , ""           , ""            ,   "Reseau_Geodesique_de_la_Polynesie_Francaise"       ,   ""         ,  "EPSG:6687"  ,   "D_Reseau_Geodesique_de_la_Polynesie_Francaise",    ""                                       , "D_Reseau Geodesique de la Polynesie Francaise", "RGP-Francaise/a"          },   // RGP-Francaise
    {   "ROME1940-1"             , "MIF 87"     , "MONTEMAR"    ,   "Monte_Mario"                                       ,   "MIF:87"   ,  "EPSG:6265"  ,   "D_Roma_1940"                                  ,    "Rome 1940"                              , "Roma_1940"                         , "D_Monte_Mario"                       },   // ROME1940 variant (Recommended)
    {   "ROME1940-1"             , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , "D_Monte_Mario_(Rome)"              , ""                                    },   // ROME1940 second line
    {   "MonteMario_1"           , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // ROME1940 variant 1
    {   "ROME1940"               , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // ROME1940 variant 2
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6265-1",   ""                                             ,    ""                                       , ""                                  , "MonteMario"                          },   // ROME1940 variant 3
    {   "RSRGD2000"              , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6764"  ,   "D_Ross_Sea_Region_Geodetic_Datum_2000"        ,    ""                                       , "D_Ross Sea Region Geodetic Datum 2000", ""                                 },   // RSRGD2000
    {   "RT90"                   , "MIF 1011"   , ""            ,   "Rikets_Tri_7_Param_1990"                           ,   "MIF:1011" ,  "EPSG:6124"  ,   "D_RT_1990"                                    ,    "RT 90 (Sweden)"                         , "RT_1990"                           , "RT_90"                               },   // RT90
    {   "RT90"                   , "MIF 112"    , ""            ,   "Rikets_koordinatsystem_1990"                       ,   "MIF:112"  ,  ""           ,   ""                                             ,    ""                                       , "Rikets koordinatsystem 1990"       , ""                                    },   // RT90 variant (apparently)
    {   ""                       , "MIF 1013"   , ""            ,   "Russia_SK42"                                       ,   "MIF:1013" ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Russia SK42
    {   "S-JTSK"                 , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // S-JTSK Legacy use Czech/JTSK
    {   "S-JTSK95"               , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // S-JTSK95 Legacy use Czech/JTSK
    {   "SAD69"                  , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6291"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // SAD69 LEGACY DEPRECATED USE SA1969
    {   "SA1969"                 , "MIF 92"     , "SAD69"       ,   "South_American_Datum_1969"                         ,   "MIF:92"   ,  "EPSG:6618"  ,   "D_South_American_1969"                        ,    "South American 1969"                    , "D_South_American_Datum_1969"       , "D_South American 1969 - Argentina"   },   // SA1969
    {   "SA1969-BZ"              , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // SA1969-BZ Variation of SA1969 for Brazil only
    {   "Santo65"                , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , "SANTO"                               },   // SANTO 
    {   "Santo65"                , ""           , ""            ,   "Santo_Dos"                                         ,   ""         ,  "EPSG:6730"  ,   "D_Santo_DOS_1965"                             ,    "Santo (DOS)"                            , "Santo_DOS_1965"                    , "Santo1965"                           },   // SANTO1965 (Variant of SANTO that uses 3P instead of MOLODENSKY)
    {   "SAOBRAZ"                , "MIF 89"     , ""            ,   "Sao_Braz"                                          ,   "MIF:89"   ,  ""           ,   "D_Azores_Oriental_Islands_1940"               ,    "Sao Braz"                               , "EPSG:106249"                       , "D_Sao_Braz"                          },   // SAOBRAZ (Azores Oriental Islands 1940 / Sao Miguel & Santa Maria Islands (Azores)) Alternate is ESRI but is slightly different in definition in CSMAP dictionary
    {   "SAOBRAZ"                , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , "Azores Oriental Islands 1940"        },   // SAOBRAZ Line 2
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6184"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // SAOBRAZ variant
    {   "SAPPER"                 , "MIF 90"     , ""            ,   "Sapper_Hill_1943"                                  ,   "MIF:90"   ,  ""           ,   "D_Sapper_Hill_1943"                           ,    "Sapper Hill 1943"                       , ""                                  , ""                                    },   // SAPPER
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6292"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // SAPPER variant
    {   "SCHWARZK"               , "MIF 91"     , ""            ,   "Schwarzeck"                                        ,   "MIF:91"   ,  ""           ,   "D_Schwarzeck"                                 ,    "Schwarzeck"                             , ""                                  , ""                                    },   // SCHWARZK
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6293"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // SCHWARZK variant
    {   "Scoresbysund52b"        , ""           , ""            ,   "Scoresbysund_1952"                                 ,   ""         ,  "EPSG:6195"  ,   "D_Scoresbysund_1952"                          ,    ""                                       , ""                                  , "Scoresbysund52"                      },   // Scoresbysund52
    {   "Segora"                 , ""           , ""            ,   "Segora"                                            ,   ""         ,  "EPSG:6294"  ,   "D_Segora"                                     ,    ""                                       , ""                                  , ""                                    },   // Segora DEPRECATED USE SEGARA
    {   "Selvagem"               , "MIF 144"    , ""            ,   "Selvagem_Grande_1938"                              ,   "MIF:144"  ,  "EPSG:6616"  ,   "D_Selvagem_Grande_1938"                       ,    "MARCO"                                  , "EPSG:106250"                       , "SelvagemGrande"                      },   // SelvagemGrande
    {   "Selvagem"               , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   "D_Selvagem_Grande"                            ,    ""                                       , ""                                  , ""                                    },   // Selvagem Grande second line
    {   ""                       , ""           , ""            ,   "Serindung"                                         ,   ""         ,  "EPSG:6295"  ,   "D_Serindung"                                  ,    ""                                       , ""                                  , ""                                    },   // Serindung INVALID DATUM NO TRANSFORM DEFINED
    {   "SierraLeone1968"        , "MIF 145"    , ""            ,   "Sierra_Leone_1968"                                 ,   "MIF:145"  ,  "EPSG:6175"  ,   "D_Sierra_Leone_1968"                          ,    ""                                       , "Sierra_Leone_1960"                 , "SierraLeone68"                       },   // SierraLeone68 Add1 appears to be an OGC Typo (Parameters are those of 1968)
    {   ""                       , ""           , ""            ,   "Sierra_Leone_1924"                                 ,   ""         ,  "EPSG:6174"  ,   "D_Sierra_Leone_1924"                          ,    ""                                       , ""                                  , ""                                    },   // SierraLeoneColony INVALID DATUM NO TRANSFORM DEFINED
    {   "SINGAPR"                , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // SINGAPR
    {   "SIRGAS2000"             , ""           , ""            ,   "SIRGAS_2000"                                       ,   ""         ,  "EPSG:6674"  ,   "D_SIRGAS_2000"                                ,    ""                                       , "D_Sistema de Referencia Geocentrico para America del Sur 2000" , "D_Sistema_de_Referencia_Geocentrico_para_America_del_Sur_2000"                                    },   // SIRGAS2000
    {   "SIRGAS2000"             , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , "" , "D_Sistema_de_Referencia_Geocentrico_para_las_AmericaS_2000"           },   // SIRGAS2000 second line
    {   "EPSG:1053"              , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   "D_Sri_Lanka_Datum_1999"                       ,    ""                                       , ""                                  , "Sri_Lanka_Datum_1999"                },   // Sri Lanka 1999
    {   "Slovenia1996"           , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6765"  ,   "D_Slovenia_Geodetic_Datum_1996"               ,    ""                                       , "D_Slovenia Geodetic Datum 1996"    , "D_Slovenia_1996"                     },   // Slovenia1996 == ETRS89
    {   "Slov/JTSK03"            , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:1201"  ,   ""                                             ,    ""                                       , ""                                  , "System of the Unified Trigonometrical Cadastral Network [JTSK03]"},   // SJTSK03
    {   "Solomon1968"            , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6718"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Solomon1968
    {   "Tern61"                 , "MIF 9"      , ""            ,   "Astro_B4_Sorol_Atoll"                              ,   "MIF:9"    ,  ""           ,   ""                                             ,    "Astro B4 Sorol Atoll"                   , ""                                  , "SOROL"                               },   // SOROL 
    {   "Tern61"                 , ""           , ""            ,   "Tern_Island_1961"                                  ,   ""         ,  "EPSG:6707"  ,   "D_Tern_Island_1961"                           ,    ""                                       , ""                                  , "Tern1961"                            },   // Tern1961
    {   "SouthGeorgia1968"       , "MIF 134"    , ""            ,   "ISTS061_Astro_1968"                                ,   "MIF:134"  ,  "EPSG:6722"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // SouthGeorgia1968 (Named ISTS061_Astro_1968 by OGC!)
    {   "SouthYemen_1"           , ""           , ""            ,   "South_Yemen"                                       ,   ""         ,  "EPSG:6164"  ,   "D_South_Yemen"                                ,    ""                                       , ""                                  , "SouthYemen"                          },   // SouthYemen
    {   "SphereWGS84"            , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // SphereWGS84 Virtual Earth pseudo datum
    {   "ST71Belep"              , ""           , ""            ,   "ST71_Belep"                                        ,   ""         ,  "EPSG:6643"  ,   "D_ST71_Belep"                                 ,    ""                                       , ""                                  , ""                                    },   // ST71Belep
    {   "ST84IleDesPins"         , ""           , ""            ,   "ST84_Ile_des_Pins"                                 ,   ""         ,  "EPSG:6642"  ,   "D_ST84_Ile_des_Pins"                          ,    ""                                       , ""                                  , ""                                    },   // ST84IleDesPins
    {   "ST87Ouvea"              , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6635"  ,   "                "                             ,    ""                                       , ""                                  , ""                                    },   // ST87Ouvea // Deprecated
    {   "ST87/Ouvea_1"           , ""           , ""            ,   "ST87_Ouvea"                                        ,   ""         ,  "EPSG:6750"  ,   "D_ST87_Ouvea"                                 ,    ""                                       , ""                                  , "ST87/Ouvea"                          },   // ST87/Ouvea variant of ST87Ouvea
    {   "Guadeloupe48"           , ""           , ""            ,   "Sainte_Anne"                                       ,   ""         ,  "EPSG:6622"  ,   "D_Sainte_Anne"                                ,    ""                                       , "D_Guadeloupe 1948"                 , "StAnne"                              },   // StAnne
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6622-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // StAnne variant
    {   ""                       , ""           , ""            ,   "St_George_Island"                                  ,   ""         ,  "EPSG:6138"  ,   "D_St_George_Island"                           ,    ""                                       , "D_St. George Island"               , ""                                    },   // StGeorge
    {   "StHelena1971"           , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6710"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // StHelena1971
    {   "StKitts1955"            , ""           , ""            ,   "St_Kitts_1955"                                     ,   ""         ,  "EPSG:6605"  ,   "D_St_Kitts_1955"                              ,    "D_St._Kitts_1955"                       , "KITTS"                             , "StKitts55"                           },   // StKitts55
    {   ""                       , ""           , ""            ,   "St_Lawrence_Island"                                ,   ""         ,  "EPSG:6136"  ,   "D_St_Lawrence_Island"                         ,    ""                                       , "D_St. Lawrence Island"             , "D_St._Lawrence_Island"               },   // StLawrence
    {   "StLucia1955"            , ""           , ""            ,   "St_Lucia_1955"                                     ,   ""         ,  "EPSG:6606"  ,   "D_St_Lucia_1955"                              ,    "D_St._Lucia_1955"                       , "LUCIA"                             , "StLucia55"                           },   // StLucia55
    {   ""                       , ""           , ""            ,   "Stockholm_1938"                                    ,   ""         ,  "EPSG:6308"  ,   "D_Stockholm_1938"                             ,    ""                                       , ""                                  , ""                                    },   // Stockholm_1938 INVALID DATUM NO TRANSFORM DEFINED
    {   ""                       , ""           , ""            ,   "St_Paul_Island"                                    ,   ""         ,  "EPSG:6137"  ,   "D_St_Paul_Island"                             ,    ""                                       , "D_St. Paul Island"                 , ""                                    },   // StPaul
    {   "StVincent1945"          , ""           , ""            ,   "St_Vincent_1945"                                   ,   ""         ,  "EPSG:6607"  ,   "D_St_Vincent_1945"                            ,    ""                                       , "D_St._Vincent_1945"                , "StVincent45"                         },   // StVincent45
    {   ""                       , ""           , ""            ,   "SVY21"                                             ,   ""         ,  "EPSG:6757"  ,   "D_SVY21"                                      ,    ""                                       , ""                                  , ""                                    },   // SVY21
    {   "SWEREF99"               , ""           , ""            ,   "SWEREF99"                                          ,   ""         ,  "EPSG:6619"  ,   "D_SWEREF99"                                   ,    ""                                       , ""                                  , ""                                    },   // SWEREF99
    {   "CHTRF95"                , ""           , ""            ,   "Swiss_TRF_1995"                                    ,   ""         ,  "EPSG:6151"  ,   "D_Swiss_TRF_1995"                             ,    ""                                       , "Swiss Terrestrial Reference Frame 1995", ""                                },   // SwissTRF1995
    {   "Tete"                   , ""           , ""            ,   "Tete"                                              ,   ""         ,  "EPSG:6127"  ,   "D_Tete"                                       ,    ""                                       , ""                                  , ""                                    },   // TETE
    {   "Tahaa54"                , ""           , ""            ,   "Tahaa"                                             ,   ""         ,  "EPSG:6629"  ,   "D_Tahaa"                                      ,    ""                                       , "D_Tahaa_1954"                      , "D_Tahaa_54"                          },   // TAHAA  // ESRI INCONSISTENCE WITH IDENTIFIER IN WKT
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6629-1",   ""                                             ,    ""                                       , ""                                  , ""                                    },   // TAHAA variant
    {   "Tahiti52"               , ""           , ""            ,   "Tahiti"                                            ,   ""         ,  "EPSG:6628"  ,   "D_Tahiti"                                     ,    ""                                       , "D_Tahiti_1952"                     , "D_Tahiti_52"                         },   // TAHITI  // ESRI INCONSISTENCE WITH IDENTIFIER IN WKT
    {   ""                       , ""           , ""            ,   "Tahiti_1979"                                       ,   ""         ,  "EPSG:6690"  ,   "D_Tahiti_1979"                                ,    ""                                       , ""                                  , ""                                    },   // TAHITI 79
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:1025"  ,   "D_Taiwan Datum 1967"                          ,    ""                                       , "TWD67"                             , "D_TWD67"                             },   // Taiwan 1967 (no ops defined)
    {   "Taiwan1997"             , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:1026"  ,   "D_Taiwan Datum 1997"                          ,    "TWD_1997"                               , "TWD97"                             , "D_TWD97"                             },   // Taiwan 1997
    {   "Tananarive1925"         , "MIF 147"    , "TANANAR"     ,   "Tananarive_1925"                                   ,   "MIF:147"  ,  "EPSG:6297"  ,   "D_Tananarive_1925"                            ,    ""                                       , "Tananarive Observatory 1925"       , "TANANPAR"                            },   // Tananarive25 additional is GDAL
    {   "Tananarive1925"         , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , "TANANARIVE"                        , "Tananarive1925"                      },   // Tananarive25 additional line
    {   "TMBLI-A"                , "MIF 96"     , "TIMBALAI"    ,   "Timbalai_1948"                                     ,   "MIF:96"   ,  ""           ,   "D_Timbalai_1948"                              ,    "Timbalai 1948"                          , "Timbalai 1948"                     , ""                                    },   // TIMBALAI The TMBLIA fits with ESRI and EPSG definitions
    {   "TIMBALAI"               , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // TIMBALAI Variation of TIMBALAI This version uses the Everest 1830 definition while officially the 1967 definition or imperial definition is to be used
    {   "TMBLI-B"                , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // TMBLI-B Variation of TIMBALAI
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6298"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // TMBLI-B variant
    {   "Timbalai68Sabah"        , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Timbalai68Sabah variation of Timbalai
    {   "Timbalai68Sarawak"      , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // Timbalai68Sarawak variation of Timbalai
    {   "TM65"                   , ""           , "TM65"        ,   "TM65"                                              ,   ""         ,  "EPSG:6299"  ,   "D_TM65"                                       ,    "TM65"                                   , ""                                  , ""                                    },   // TM65
    {   "TOKYO"                  , "MIF 97"     , "TOKYO"       ,   "Tokyo"                                             ,   "MIF:97"   ,  ""           ,   "D_Tokyo"                                      ,    "Tokyo"                                  , "Tokyo Datum"                       , ""                                    },   // TOKYO 
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6301"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // TOKYO variant
    {   ""                       , "MIF 1015"   , ""            ,   ""                                                  ,   "MIF:1015" ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // other TOKYO variant
    {   "Trinidad03"             , ""           , ""            ,   "Trinidad_1903"                                     ,   ""         ,  "EPSG:6302"  ,   "D_Trinidad_1903"                              ,    ""                                       , ""                                  , ""                                    },   // Trinidad03
    {   "TRISTAN"                , "MIF 98"     , ""            ,   "Tristan_Astro_1968"                                ,   "MIF:98"   ,  ""           ,   "D_Tristan_1968"                               ,    "Tristan Astro 1968"                     , "Tristan_1968"                      , ""                                    },   // TRISTAN 
    {   ""                       , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6734"  ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // TRISTAN variant
    {   ""                       , ""           , "TRUCIAL"     ,   "Trucial_Coast_1948"                                ,   ""         ,  "EPSG:6303"  ,   "D_Trucial_Coast_1948"                         ,    ""                                       , ""                                  , ""                                    },   // Trucial Coast 1948 INVALID DATUM NO TRANSFORM DEFINED
    {   "UNITS"                  , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   "D_Sphere"                                     ,    "Local Datum"                            , "UnitSphere"                        , ""                                    },   // UNITS (Not really but local coordinate systems are different)
    {   "VanuaLv1915"            , ""           , ""            ,   ""                                                  ,   ""         ,  "EPSG:6748"  ,   "D_Vanua Levu"                                 ,    "D_VanuaLevu1915"                        , "D_Vanua_Levu"                      , "VanuaLv15"                           },   // VanuaLv15
    {   "Vietnam2000"            , ""           , ""            ,   "Vietnam_2000"                                      ,   ""         ,  "EPSG:6756"  ,   "D_Vietnam_2000"                               ,    "D_VN2000"                               , "VN2000"                            , "D_VN-2000"                           },   // Vietnam2000
    {   ""                       , ""           , ""            ,   "Vientiane_1982"                                    ,   ""         ,  "EPSG:6676"  ,   "D_Vientiane_1982"                             ,    ""                                       , ""                                  , ""                                    },   // Vientiane 1982
    {   "VITI"                   , "MIF 99"     , ""            ,   "Viti_Levu_1916"                                    ,   "MIF:99"   ,  "EPSG:6752"  ,   "D_Viti_Levu_1916"                             ,    "Viti Levu 1916"                         , "VitiLevu12"                        , ""                                    },   // VITI Appears identical to Vanua Levu 1912 ... check ESRI appears to be 1912 instead of 1916 ???
    {   "Voirol1875_1"           , "MIF 148"    , ""            ,   "Voirol_1875"                                       ,   "MIF:148"  ,  "EPSG:6304"  ,   "D_Voirol_1875"                                ,    "VOIR1875"                               , "Voirol_1874"                       , "Voirol1875"                          },   // Voirol1875 Add1 appears to be an OGC Typo (same parameters)
    {   ""                       , ""           , ""            ,   "Voirol_1879"                                       ,   ""         ,  "EPSG:6671"  ,   "D_Voirol_1879"                                ,    ""                                       , ""                                  , ""                                    },   // Voirol1879 INVALID DATUM NO TRANSFORM DEFINED
    {   ""                       , "NIF 149"    , ""            ,   "Voirol_Unifie_1960"                                ,   "MIF:149"  ,  "EPSG:6305"  ,   "D_Voirol_Unifie_1960"                         ,    ""                                       , "EPSG:106305"                       , "Virol_1960"                          },   // Voirol Unifie 1960 Add2 is OGC variant with Typo (parameters are the same)
    {   "WAKE"                   , "MIF 100"    , ""            ,   "Wake_Eniwetok_1960"                                ,   "MIF:100"  ,  ""           ,   "D_Wake_Eniwetok_1960"                         ,    "Wake-Eniwetok 1960"                     , ""                                  , ""                                    },   // WAKE 
    {   "WakeIs1952"             , ""           , ""            ,   "Wake_Island_1952"                                  ,   ""         ,  "EPSG:6733"  ,   "D_Wake_Island_1952"                           ,    ""                                       , ""                                  , "WakeIs52"                            },   // Wake 1952
    {   "WGS72"                  , "MIF 103"    , ""            ,   "WGS_1972"                                          ,   "MIF:103"  ,  "EPSG:6322"  ,   "D_WGS_1972"                                   ,    "WGS 72"                                 , "WGS 72"                            , "World Geodetic System 1972"          },   // WGS72
    {   "WGS72-BW"               , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   ""                                             ,    ""                                       , ""                                  , ""                                    },   // WGS72-BW Variation of WGS72 using a Bursa-Wolf transformation
    {   "WGS72-TBE"              , ""           , ""            ,   "WGS_1972_BE"                                       ,   ""         ,  "EPSG:6324"  ,   "D_WGS_1972_BE"                                ,    "WGS72BE"                                , "D_WGS 72 Transit Broadcast Ephemeris", "D_WGS_72_Transit_Broadcast_Ephemeris"                                    },   // WGS72-TBE (WGS 72 Transit Broadcast Ephemeris)
    {   "WGS84"                  , "MIF 104"    , ""            ,   "WGS_1984"                                          ,   "MIF:104"  ,  "EPSG:6326"  ,   "D_WGS_1984"                                   ,    "WGS_84"                                 , "WGS 84"                            , "World Geodetic System 1984"          },   // WGS84
    {   "WGS84"                  , ""           , ""            ,   ""                                                  ,   ""         ,  ""           ,   "D_World_Geodetic_System_1984"                 ,    "D_WGS_84"                               , ""                                  , "World_Geodetic_System_1984"          },   // WGS84
    {   "Xian80"                 , ""           , "XIAN80"      ,   "Xian_1980"                                         ,   ""         ,  "EPSG:6610"  ,   "D_Xian_1980"                                  ,    ""                                       , "Xian 1980"                         , ""                                    },   // Xian80
    {   "Yacare/E"               , "MIF 105"    , ""            ,   "Yacare"                                            ,   "MIF:105"  ,  "EPSG:6309"  ,   "D_Yacare"                                     ,    "Yacare"                                 , ""                                  , ""                                    },   // YACARE
    {   "YemenNtl1996"           , ""           , ""            ,   "Yemen_NGN_1996"                                    ,   ""         ,  "EPSG:6163"  ,   "D_Yemen_NGN_1996"                             ,    "D_Yemen_National_Geodetic_Network_1996" , "D_Yemen National Geodetic Network 1996" , "YemenNtl96"                     },   // YemenNtl96 
    {   ""                       , ""           , "YOFF2000"    ,   "Yoff"                                              ,   ""         ,  "EPSG:6310"  ,   "D_Yoff"                                       ,    ""                                       , ""                                  , ""                                    },   // Yoff
    {   "ZANDERIJ"               , "MIF 106"    , ""            ,   "Zanderij"                                          ,   "MIF:106"  ,  "EPSG:6311"  ,   "D_Zanderij"                                   ,    "Zanderij"                               , ""                                  , ""                                    },   // ZANDERIJ
                                                                                                                                                                                                                                                    
                                                                                                                                                                                                                                                    
                                                                                                                                                                                                                                                    
                                                                                                                                                                                                                                                    
};                                                                                                                                                                                                                                                  




int ProjectionMap[][2] =
{
    {3811 , 9802},
    {3813 , 9807},
    {3818 , 9807},
    {3820 , 9807},
    {3831 , 9804},
    {3853 , 9807},
    {3860 , 9807},
    {3861 , 9807},
    {3862 , 9807},
    {3863 , 9807},
    {3864 , 9807},
    {3865 , 9807},
    {3866 , 9807},
    {3867 , 9807},
    {3868 , 9807},
    {3869 , 9807},
    {3870 , 9807},
    {3871 , 9807},
    {3872 , 9807},
    {3967 , 9802},
    {3977 , 9802},
    {3980 , 9802},
    {3981 , 9807},
    {3982 , 9807},
    {3983 , 9807},
    {3984 , 9807},
    {3999 , 9807},
    {4089 , 9807},
    {4090 , 9807},
    {4091 , 9807},
    {4092 , 9807},
    {4101 , 9807},
    {4102 , 9807},
    {4103 , 9807},
    {4104 , 9807},
    {4105 , 9807},
    {4106 , 9807},
    {4107 , 9807},
    {4108 , 9807},
    {4109 , 9807},
    {4110 , 9807},
    {4111 , 9807},
    {4112 , 9807},
    {4113 , 9807},
    {4114 , 9806},
    {4115 , 9806},
    {4116 , 9806},
    {4117 , 9806},
    {4118 , 9807},
    {4119 , 9807},
    {4177 , 9806},
    {4186 , 9807},
    {4187 , 9807},
    {4305 , 9806},
    {4320 , 9806},
    {4321 , 9806},
    {4323 , 9806},
    {4325 , 9807},
    {4416 , 9802},
    {4436 , 9802},
    {4454 , 9802},
    {4460 , 9802},
    {4648 , 9807},
    {4825 , 9802},
    {4838 , 9802},
    {4841 , 9807},
    {4842 , 9807},
    {4843 , 9807},
    {4844 , 9807},
    {4845 , 9807},
    {4846 , 9807},
    {4847 , 9807},
    {4848 , 9807},
    {4849 , 9807},
    {4850 , 9807},
    {4851 , 9807},
    {4852 , 9807},
    {4853 , 9807},
    {4854 , 9807},
    {4881 , 9807},
    {5000 , 9807},
    {5001 , 9807},
    {5002 , 9807},
    {5003 , 9807},
    {5004 , 9807},
    {5005 , 9807},
    {5006 , 9807},
    {5007 , 9807},
    {5008 , 9807},
    {5009 , 9807},
    {5010 , 9807},
    {5019 , 9828},
    {5020 , 9807},
    {5049 , 9807},
    {5068 , 9822},
    {5100 , 9807},
    {5101 , 9807},
    {5102 , 9807},
    {5103 , 9807},
    {5104 , 9807},
    {5131 , 9807},
    {5135 , 9807},
    {5136 , 9807},
    {5137 , 9807},
    {5138 , 9807},
    {5139 , 9807},
    {5140 , 9807},
    {5141 , 9807},
    {5142 , 9807},
    {5143 , 9807},
    {5144 , 9807},
    {5145 , 9807},
    {5146 , 9807},
    {5147 , 9807},
    {5148 , 9807},
    {5149 , 9807},
    {5150 , 9807},
    {5151 , 9807},
    {5152 , 9807},
    {5153 , 9807},
    {5154 , 9807},
    {5155 , 9807},
    {5156 , 9807},
    {5157 , 9807},
    {5158 , 9807},
    {5159 , 9807},
    {5160 , 9807},
    {5161 , 9807},
    {5162 , 9807},
    {5163 , 9807},
    {5164 , 9807},
    {5165 , 9807},
    {5222 , 9807},
    {5231 , 9807},
    {5232 , 9807},
    {5265 , 9807},
    {5268 , 9807},
    {5276 , 9807},
    {5277 , 9807},
    {5278 , 9807},
    {5279 , 9807},
    {5280 , 9807},
    {5281 , 9807},
    {5282 , 9807},
    {5283 , 9807},
    {5284 , 9807},
    {5285 , 9807},
    {5286 , 9807},
    {5287 , 9807},
    {5288 , 9807},
    {5289 , 9807},
    {5290 , 9807},
    {5291 , 9807},
    {5312 , 9807},
    {5313 , 9807},
    {5314 , 9807},
    {5315 , 9807},
    {5319 , 9802},
    {5326 , 9802},
    {5328 , 9804},
    {5366 , 9807},
    {5394 , 9801},
    {5397 , 9801},
    {5398 , 9801},
    {5399 , 9801},
    {5439 , 9801},
    {5444 , 9801},
    {5465 , 9807},
    {5468 , 9801},
    {5471 , 9818},
    {5475 , 9802},
    {5476 , 9802},
    {5477 , 9802},
    {5478 , 9810},
    {5509 , 9819},
    {5517 , 9807},
    {5522 , 9807},
    {5547 , 9807},
    {5548 , 9807},
    {5549 , 9807},
    {5587 , 9809},
    {5595 , 9807},
    {5640 , 9805},
    {5642 , 9802},
    {5645 , 9807},
    {5647 , 9807},
    {5648 , 9807},
    {5658 , 9807},
    {5824 , 9807},
    {5883 , 9807},
    {5889 , 9829},
    {5892 , 9807},
    {5893 , 9807},
    {5894 , 9807},
    {5895 , 9807},
    {5901 , 9810},
    {5902 , 9810},
    {5903 , 9810},
    {5904 , 9810},
    {5905 , 9810},
    {5906 , 9802},
    {5907 , 9802},
    {5908 , 9802},
    {5909 , 9802},
    {5910 , 9802},
    {5911 , 9802},
    {5912 , 9802},
    {5913 , 9802},
    {5914 , 9802},
    {5915 , 9802},
    {5916 , 9802},
    {5917 , 9802},
    {5918 , 9802},
    {5919 , 9802},
    {5920 , 9802},
    {5943 , 9802},
    {5944 , 9802},
    {5977 , 9802},
    {5978 , 9802},
    {5979 , 9802},
    {5980 , 9802},
    {5981 , 9802},
    {5982 , 9802},
    {5983 , 9802},
    {5984 , 9802},
    {5985 , 9802},
    {5986 , 9802},
    {5987 , 9802},
    {5988 , 9802},
    {5989 , 9802},
    {5990 , 9802},
    {5991 , 9802},
    {5992 , 9802},
    {5993 , 9802},
    {5994 , 9802},
    {5995 , 9802},
    {5996 , 9802},
    {5997 , 9802},
    {5998 , 9802},
    {5999 , 9802},
    {6000 , 9802},
    {6001 , 9802},
    {6002 , 9802},
    {6003 , 9802},
    {6004 , 9802},
    {6005 , 9802},
    {6006 , 9802},
    {6007 , 9802},
    {6008 , 9802},
    {6009 , 9802},
    {6010 , 9802},
    {6011 , 9802},
    {6012 , 9802},
    {6013 , 9802},
    {6014 , 9802},
    {6015 , 9802},
    {6016 , 9802},
    {6017 , 9802},
    {6018 , 9802},
    {6019 , 9802},
    {6020 , 9802},
    {6021 , 9802},
    {6022 , 9802},
    {6023 , 9802},
    {6024 , 9802},
    {6025 , 9802},
    {6026 , 9802},
    {6027 , 9802},
    {6028 , 9802},
    {6029 , 9802},
    {6030 , 9802},
    {6031 , 9802},
    {6032 , 9802},
    {6033 , 9802},
    {6034 , 9802},
    {6035 , 9802},
    {6036 , 9802},
    {6037 , 9802},
    {6038 , 9802},
    {6039 , 9802},
    {6040 , 9802},
    {6041 , 9802},
    {6042 , 9802},
    {6043 , 9802},
    {6044 , 9802},
    {6045 , 9802},
    {6046 , 9802},
    {6047 , 9802},
    {6048 , 9802},
    {6049 , 9802},
    {6126 , 9802},
    {6127 , 9807},
    {6203 , 9807},
    {6308 , 9807},
    {6361 , 9802},
    {6374 , 9807},
    {6375 , 9807},
    {6376 , 9807},
    {6377 , 9807},
    {6378 , 9807},
    {6379 , 9807},
    {6380 , 9807},
    {6390 , 9802},
    {6645 , 9822},
    {6702 , 9807},
    {6716 , 9807},
    {6717 , 9807},
    {6718 , 9807},
    {6719 , 9807},
    {6725 , 9807},
    {6726 , 9807},
    {6727 , 9807},
    {6728 , 9807},
    {6729 , 9807},
    {6730 , 9807},
    {6731 , 9807},
    {6741 , 9807},
    {6742 , 9807},
    {6743 , 9807},
    {6744 , 9807},
    {6745 , 9801},
    {6746 , 9801},
    {6747 , 9801},
    {6748 , 9801},
    {6749 , 9807},
    {6750 , 9807},
    {6751 , 9801},
    {6752 , 9801},
    {6753 , 9812},
    {6754 , 9812},
    {6755 , 9807},
    {6756 , 9807},
    {6757 , 9807},
    {6758 , 9807},
    {6759 , 9807},
    {6760 , 9807},
    {6761 , 9807},
    {6762 , 9807},
    {6763 , 9807},
    {6764 , 9807},
    {6765 , 9807},
    {6766 , 9807},
    {6767 , 9807},
    {6768 , 9807},
    {6769 , 9812},
    {6770 , 9812},
    {6771 , 9807},
    {6772 , 9807},
    {6773 , 9807},
    {6774 , 9807},
    {6775 , 9801},
    {6776 , 9801},
    {6777 , 9807},
    {6778 , 9807},
    {6779 , 9807},
    {6780 , 9807},
    {6869 , 9807},
    {6877 , 9807},
    {6878 , 9807},
    {6920 , 9802},
    {6921 , 9802},
    {6928 , 9835},
    {6929 , 9820},
    {6930 , 9820},
    {6952 , 9807},
    {6953 , 9807},
    {6954 , 9807},
    {6955 , 9807},
    {6961 , 9802},
    {6994 , 9807},
    {6995 , 9807},
    {7043 , 9801},
    {7044 , 9801},
    {7045 , 9807},
    {7046 , 9801},
    {7047 , 9801},
    {7048 , 9807},
    {7049 , 9807},
    {7050 , 9807},
    {7051 , 9807},
    {7052 , 9801},
    {7053 , 9807},
    {7054 , 9801},
    {7055 , 9807},
    {7056 , 9807},
    {7089 , 9807},
    {7090 , 9807},
    {7091 , 9807},
    {7092 , 9807},
    {7093 , 9801},
    {7094 , 9801},
    {7095 , 9801},
    {7096 , 9801},
    {7097 , 9801},
    {7098 , 9801},
    {7099 , 9801},
    {7100 , 9801},
    {7101 , 9807},
    {7102 , 9807},
    {7103 , 9801},
    {7104 , 9801},
    {7105 , 9801},
    {7106 , 9801},
    {7107 , 9807},
    {7108 , 9807},
    {7129 , 9807},
    {7130 , 9807},
    {7141 , 9807},
    {7143 , 9807},
    {7144 , 9807},
    {7145 , 9807},
    {7146 , 9807},
    {7147 , 9807},
    {7148 , 9807},
    {7149 , 9807},
    {7150 , 9807},
    {7151 , 9807},
    {7152 , 9807},
    {7153 , 9807},
    {7154 , 9807},
    {7155 , 9807},
    {7156 , 9807},
    {7157 , 9807},
    {7158 , 9807},
    {7159 , 9807},
    {7160 , 9807},
    {7161 , 9807},
    {7162 , 9807},
    {7163 , 9807},
    {7164 , 9807},
    {7165 , 9807},
    {7166 , 9807},
    {7167 , 9807},
    {7168 , 9807},
    {7169 , 9807},
    {7170 , 9807},
    {7171 , 9807},
    {7172 , 9807},
    {7173 , 9807},
    {7174 , 9807},
    {7175 , 9807},
    {7176 , 9807},
    {7177 , 9807},
    {7178 , 9807},
    {7179 , 9807},
    {7180 , 9807},
    {7181 , 9807},
    {7182 , 9807},
    {7183 , 9807},
    {7184 , 9807},
    {7185 , 9807},
    {7186 , 9807},
    {7187 , 9807},
    {7188 , 9807},
    {7189 , 9807},
    {7190 , 9807},
    {7191 , 9807},
    {7192 , 9807},
    {7193 , 9807},
    {7194 , 9807},
    {7195 , 9807},
    {7196 , 9807},
    {7197 , 9807},
    {7198 , 9807},
    {7199 , 9807},
    {7200 , 9807},
    {7201 , 9807},
    {7202 , 9807},
    {7203 , 9807},
    {7204 , 9807},
    {7205 , 9807},
    {7206 , 9807},
    {7207 , 9807},
    {7208 , 9807},
    {7209 , 9807},
    {7210 , 9807},
    {7211 , 9807},
    {7212 , 9807},
    {7213 , 9807},
    {7214 , 9807},
    {7215 , 9807},
    {7216 , 9807},
    {7217 , 9807},
    {7218 , 9807},
    {7219 , 9807},
    {7220 , 9807},
    {7221 , 9807},
    {7222 , 9807},
    {7223 , 9807},
    {7224 , 9807},
    {7225 , 9807},
    {7226 , 9807},
    {7227 , 9807},
    {7228 , 9807},
    {7229 , 9807},
    {7230 , 9807},
    {7231 , 9807},
    {7232 , 9807},
    {7233 , 9807},
    {7234 , 9807},
    {7235 , 9807},
    {7236 , 9807},
    {7237 , 9807},
    {7238 , 9807},
    {7239 , 9807},
    {7240 , 9807},
    {7241 , 9807},
    {7242 , 9807},
    {7243 , 9807},
    {7244 , 9807},
    {7245 , 9807},
    {7246 , 9807},
    {7247 , 9807},
    {7248 , 9807},
    {7249 , 9807},
    {7250 , 9807},
    {7251 , 9807},
    {7252 , 9807},
    {7253 , 9807},
    {7254 , 9807},
    {7255 , 9807},
    {7256 , 9807},
    {7378 , 9807},
    {7379 , 9807},
    {7380 , 9801},
    {7381 , 9801},
    {7382 , 9801},
    {7383 , 9801},
    {7384 , 9807},
    {7385 , 9807},
    {7386 , 9807},
    {7387 , 9807},
    {7388 , 9807},
    {7389 , 9807},
    {7390 , 9807},
    {7391 , 9807},
    {7392 , 9801},
    {7393 , 9801},
    {7394 , 9807},
    {7395 , 9807},
    {7396 , 9801},
    {7397 , 9801},
    {7398 , 9801},
    {7399 , 9801},
    {7424 , 9801},
    {7425 , 9801},
    {7426 , 9807},
    {7427 , 9807},
    {7428 , 9807},
    {7429 , 9807},
    {7430 , 9807},
    {7431 , 9807},
    {7432 , 9801},
    {7433 , 9801},
    {7434 , 9807},
    {7435 , 9807},
    {7436 , 9807},
    {7437 , 9807},
    {7438 , 9807},
    {7439 , 9807},
    {7440 , 9801},
    {7441 , 9801},
    {7450 , 9807},
    {7451 , 9807},
    {7452 , 9801},
    {7453 , 9801},
    {7454 , 9807},
    {7455 , 9807},
    {7456 , 9801},
    {7457 , 9801},
    {7458 , 9807},
    {7459 , 9807},
    {7460 , 9807},
    {7461 , 9807},
    {7462 , 9807},
    {7463 , 9807},
    {7464 , 9801},
    {7465 , 9801},
    {7466 , 9807},
    {7467 , 9807},
    {7468 , 9801},
    {7469 , 9801},
    {7470 , 9807},
    {7471 , 9807},
    {7472 , 9807},
    {7473 , 9807},
    {7474 , 9807},
    {7475 , 9807},
    {7476 , 9801},
    {7477 , 9801},
    {7478 , 9807},
    {7479 , 9807},
    {7480 , 9807},
    {7481 , 9807},
    {7482 , 9801},
    {7483 , 9801},
    {7484 , 9807},
    {7485 , 9807},
    {7486 , 9807},
    {7487 , 9807},
    {7488 , 9801},
    {7489 , 9801},
    {7490 , 9801},
    {7491 , 9801},
    {7492 , 9801},
    {7493 , 9801},
    {7494 , 9807},
    {7495 , 9807},
    {7496 , 9807},
    {7497 , 9807},
    {7498 , 9801},
    {7499 , 9801},
    {7500 , 9801},
    {7501 , 9801},
    {7502 , 9807},
    {7503 , 9807},
    {7504 , 9807},
    {7505 , 9807},
    {7506 , 9807},
    {7507 , 9807},
    {7508 , 9807},
    {7509 , 9807},
    {7510 , 9801},
    {7511 , 9801},
    {7512 , 9801},
    {7513 , 9801},
    {7514 , 9807},
    {7515 , 9807},
    {7516 , 9807},
    {7517 , 9807},
    {7518 , 9801},
    {7519 , 9801},
    {7520 , 9801},
    {7521 , 9801},
    {7522 , 9807},
    {7523 , 9807},
    {7524 , 9807},
    {7525 , 9807},
    {7526 , 9801},
    {7527 , 9801},
    {7687 , 9807},
    {7688 , 9807},
    {7689 , 9807},
    {7690 , 9807},
    {7691 , 9807},
    {7722 , 9802},
    {7723 , 9802},
    {7724 , 9802},
    {7725 , 9802},
    {7726 , 9802},
    {7727 , 9802},
    {7728 , 9802},
    {7729 , 9802},
    {7730 , 9802},
    {7731 , 9802},
    {7732 , 9802},
    {7733 , 9802},
    {7734 , 9802},
    {7735 , 9802},
    {7736 , 9802},
    {7737 , 9802},
    {7738 , 9802},
    {7739 , 9802},
    {7740 , 9802},
    {7741 , 9802},
    {7742 , 9802},
    {7743 , 9802},
    {7744 , 9807},
    {7745 , 9807},
    {7746 , 9807},
    {7747 , 9807},
    {7748 , 9807},
    {7749 , 9807},
    {7750 , 9807},
    {7751 , 9807},
    {7752 , 9807},
    {7753 , 9807},
    {7754 , 9807},
    {7802 , 9802},
    {7818 , 9807},
    {7819 , 9807},
    {7820 , 9807},
    {7821 , 9807},
    {7822 , 9807},
    {7823 , 9807},
    {7824 , 9807},
    {7875 , 9807},
    {7876 , 9807},
    {7993 , 9807},
    {7994 , 9807},
    {7995 , 9807},
    {7996 , 9807},
    {7997 , 9807},
    {7998 , 9807},
    {7999 , 9807},
    {8000 , 9807},
    {8001 , 9807},
    {8002 , 9807},
    {8003 , 9807},
    {8004 , 9807},
    {8005 , 9807},
    {8006 , 9807},
    {8007 , 9807},
    {8008 , 9807},
    {8009 , 9807},
    {8010 , 9807},
    {8011 , 9807},
    {8012 , 9807},
    {8033 , 9807},
    {8034 , 9807},
    {8040 , 9806},
    {8041 , 9806},
    {8061 , 9815},
    {8062 , 9807},
    {8063 , 9807},
    {8064 , 9801},
    {8080 , 9807},
    {8081 , 9807},
    {8087 , 9802},
    {8273 , 9807},
    {8274 , 9807},
    {8275 , 9807},
    {8276 , 9807},
    {8277 , 9801},
    {8278 , 9801},
    {8279 , 9807},
    {8280 , 9807},
    {8281 , 9807},
    {8282 , 9807},
    {8283 , 9801},
    {8284 , 9801},
    {8285 , 9801},
    {8286 , 9801},
    {8287 , 9801},
    {8288 , 9801},
    {8289 , 9801},
    {8290 , 9801},
    {8291 , 9801},
    {8292 , 9801},
    {8293 , 9807},
    {8294 , 9807},
    {8295 , 9801},
    {8296 , 9801},
    {8297 , 9801},
    {8298 , 9801},
    {8299 , 9807},
    {8300 , 9807},
    {8301 , 9801},
    {8302 , 9801},
    {8303 , 9801},
    {8304 , 9801},
    {8305 , 9807},
    {8306 , 9807},
    {8307 , 9801},
    {8308 , 9801},
    {8309 , 9807},
    {8310 , 9807},
    {8373 , 9807},
    {8374 , 9807},
    {8375 , 9807},
    {8376 , 9807},
    {8389 , 9807},
    {8432 , 9807},
    {8440 , 9813},
    {8458 , 9807},
    {8459 , 9807},
    {8490 , 9807},
    {8491 , 9807},
    {8492 , 9807},
    {8493 , 9807},
    {8494 , 9807},
    {8495 , 9801},
    {8498 , 9801},
    {8499 , 9801},
    {8500 , 9801},
    {8501 , 9807},
    {8502 , 9807},
    {8503 , 9807},
    {8504 , 9807},
    {8505 , 9807},
    {8506 , 9801},
    {8507 , 9801},
    {8515 , 9807},
    {8516 , 9807},
    {10101, 9807},
    {10102, 9807},
    {10131, 9807},
    {10132, 9807},
    {10201, 9807},
    {10202, 9807},
    {10203, 9807},
    {10231, 9807},
    {10232, 9807},
    {10233, 9807},
    {10301, 9802},
    {10302, 9802},
    {10331, 9802},
    {10332, 9802},
    {10401, 9802},
    {10402, 9802},
    {10403, 9802},
    {10404, 9802},
    {10405, 9802},
    {10406, 9802},
    {10407, 9802},
    {10408, 9802},
    {10420, 9822},
    {10431, 9802},
    {10432, 9802},
    {10433, 9802},
    {10434, 9802},
    {10435, 9802},
    {10436, 9802},
    {10501, 9802},
    {10502, 9802},
    {10503, 9802},
    {10531, 9802},
    {10532, 9802},
    {10533, 9802},
    {10600, 9802},
    {10630, 9802},
    {10700, 9807},
    {10730, 9807},
    {10901, 9807},
    {10902, 9807},
    {10903, 9802},
    {10931, 9807},
    {10932, 9807},
    {10933, 9802},
    {10934, 9822},
    {11001, 9807},
    {11002, 9807},
    {11031, 9807},
    {11032, 9807},
    {11101, 9807},
    {11102, 9807},
    {11103, 9807},
    {11131, 9807},
    {11132, 9807},
    {11133, 9807},
    {11201, 9807},
    {11202, 9807},
    {11231, 9807},
    {11232, 9807},
    {11301, 9807},
    {11302, 9807},
    {11331, 9807},
    {11332, 9807},
    {11401, 9802},
    {11402, 9802},
    {11431, 9802},
    {11432, 9802},
    {11501, 9802},
    {11502, 9802},
    {11531, 9802},
    {11532, 9802},
    {11601, 9802},
    {11602, 9802},
    {11630, 9802},
    {11631, 9802},
    {11632, 9802},
    {11701, 9802},
    {11702, 9802},
    {11703, 9802},
    {11731, 9802},
    {11732, 9802},
    {11733, 9802},
    {11801, 9807},
    {11802, 9807},
    {11831, 9807},
    {11832, 9807},
    {11833, 9807},
    {11834, 9807},
    {11851, 9807},
    {11852, 9807},
    {11853, 9807},
    {11854, 9807},
    {11900, 9802},
    {11930, 9802},
    {12001, 9802},
    {12002, 9802},
    {12031, 9802},
    {12032, 9802},
    {12101, 9807},
    {12102, 9807},
    {12103, 9807},
    {12111, 9802},
    {12112, 9802},
    {12113, 9802},
    {12141, 9802},
    {12142, 9802},
    {12143, 9802},
    {12150, 9812},
    {12201, 9802},
    {12202, 9802},
    {12203, 9802},
    {12231, 9802},
    {12232, 9802},
    {12233, 9802},
    {12234, 9802},
    {12235, 9802},
    {12236, 9802},
    {12301, 9807},
    {12302, 9807},
    {12331, 9807},
    {12332, 9807},
    {12401, 9807},
    {12402, 9807},
    {12403, 9807},
    {12431, 9807},
    {12432, 9807},
    {12433, 9807},
    {12501, 9802},
    {12502, 9802},
    {12503, 9802},
    {12530, 9802},
    {12601, 9802},
    {12602, 9802},
    {12630, 9802},
    {12701, 9807},
    {12702, 9807},
    {12703, 9807},
    {12731, 9807},
    {12732, 9807},
    {12733, 9807},
    {12800, 9807},
    {12830, 9807},
    {12900, 9807},
    {12930, 9807},
    {13001, 9807},
    {13002, 9807},
    {13003, 9807},
    {13031, 9807},
    {13032, 9807},
    {13033, 9807},
    {13101, 9807},
    {13102, 9807},
    {13103, 9807},
    {13104, 9802},
    {13131, 9807},
    {13132, 9807},
    {13133, 9807},
    {13134, 9802},
    {13200, 9802},
    {13230, 9802},
    {13301, 9802},
    {13302, 9802},
    {13331, 9802},
    {13332, 9802},
    {13401, 9802},
    {13402, 9802},
    {13431, 9802},
    {13432, 9802},
    {13433, 9802},
    {13434, 9802},
    {13501, 9802},
    {13502, 9802},
    {13531, 9802},
    {13532, 9802},
    {13601, 9802},
    {13602, 9802},
    {13631, 9802},
    {13632, 9802},
    {13633, 9802},
    {13701, 9802},
    {13702, 9802},
    {13731, 9802},
    {13732, 9802},
    {13800, 9807},
    {13830, 9807},
    {13901, 9802},
    {13902, 9802},
    {13930, 9802},
    {14001, 9802},
    {14002, 9802},
    {14031, 9802},
    {14032, 9802},
    {14100, 9802},
    {14130, 9802},
    {14201, 9802},
    {14202, 9802},
    {14203, 9802},
    {14204, 9802},
    {14205, 9802},
    {14231, 9802},
    {14232, 9802},
    {14233, 9802},
    {14234, 9802},
    {14235, 9802},
    {14251, 9802},
    {14252, 9802},
    {14253, 9802},
    {14254, 9822},
    {14301, 9802},
    {14302, 9802},
    {14303, 9802},
    {14331, 9802},
    {14332, 9802},
    {14333, 9802},
    {14400, 9807},
    {14430, 9807},
    {14501, 9802},
    {14502, 9802},
    {14531, 9802},
    {14532, 9802},
    {14601, 9802},
    {14602, 9802},
    {14631, 9802},
    {14632, 9802},
    {14701, 9802},
    {14702, 9802},
    {14731, 9802},
    {14732, 9802},
    {14733, 9802},
    {14734, 9802},
    {14735, 9802},
    {14736, 9802},
    {14801, 9802},
    {14802, 9802},
    {14803, 9802},
    {14811, 9807},
    {14831, 9802},
    {14832, 9802},
    {14833, 9802},
    {14841, 9807},
    {14901, 9807},
    {14902, 9807},
    {14903, 9807},
    {14904, 9807},
    {14931, 9807},
    {14932, 9807},
    {14933, 9807},
    {14934, 9807},
    {14935, 9807},
    {14936, 9807},
    {14937, 9807},
    {14938, 9807},
    {15001, 9812},
    {15002, 9807},
    {15003, 9807},
    {15004, 9807},
    {15005, 9807},
    {15006, 9807},
    {15007, 9807},
    {15008, 9807},
    {15009, 9807},
    {15010, 9802},
    {15020, 9822},
    {15021, 9822},
    {15031, 9812},
    {15032, 9807},
    {15033, 9807},
    {15034, 9807},
    {15035, 9807},
    {15036, 9807},
    {15037, 9807},
    {15038, 9807},
    {15039, 9807},
    {15040, 9802},
    {15101, 9807},
    {15102, 9807},
    {15103, 9807},
    {15104, 9807},
    {15105, 9807},
    {15131, 9807},
    {15132, 9807},
    {15133, 9807},
    {15134, 9807},
    {15135, 9807},
    {15138, 9807},
    {15201, 9802},
    {15202, 9802},
    {15230, 9802},
    {15297, 9802},
    {15298, 9802},
    {15299, 9802},
    {15300, 9801},
    {15301, 9801},
    {15302, 9802},
    {15303, 9802},
    {15304, 9807},
    {15305, 9807},
    {15306, 9807},
    {15307, 9802},
    {15308, 9802},
    {15309, 9802},
    {15310, 9802},
    {15311, 9802},
    {15312, 9802},
    {15313, 9802},
    {15314, 9802},
    {15315, 9802},
    {15316, 9802},
    {15317, 9807},
    {15318, 9807},
    {15319, 9807},
    {15320, 9802},
    {15321, 9807},
    {15322, 9807},
    {15323, 9807},
    {15324, 9807},
    {15325, 9807},
    {15326, 9807},
    {15327, 9807},
    {15328, 9802},
    {15329, 9802},
    {15330, 9802},
    {15331, 9802},
    {15332, 9802},
    {15333, 9802},
    {15334, 9802},
    {15335, 9802},
    {15336, 9807},
    {15337, 9807},
    {15338, 9802},
    {15339, 9807},
    {15340, 9807},
    {15341, 9807},
    {15342, 9807},
    {15343, 9807},
    {15344, 9807},
    {15345, 9802},
    {15346, 9802},
    {15347, 9802},
    {15348, 9802},
    {15349, 9802},
    {15350, 9802},
    {15351, 9802},
    {15352, 9802},
    {15353, 9802},
    {15354, 9802},
    {15355, 9802},
    {15356, 9802},
    {15357, 9802},
    {15358, 9802},
    {15359, 9802},
    {15360, 9802},
    {15361, 9802},
    {15362, 9802},
    {15363, 9802},
    {15364, 9802},
    {15365, 9802},
    {15366, 9802},
    {15367, 9802},
    {15368, 9802},
    {15369, 9802},
    {15370, 9802},
    {15371, 9802},
    {15372, 9807},
    {15373, 9807},
    {15374, 9802},
    {15375, 9802},
    {15376, 9801},
    {15377, 9802},
    {15378, 9802},
    {15379, 9802},
    {15380, 9802},
    {15381, 9807},
    {15382, 9807},
    {15383, 9807},
    {15384, 9807},
    {15385, 9802},
    {15386, 9802},
    {15387, 9807},
    {15388, 9807},
    {15389, 9807},
    {15390, 9807},
    {15391, 9802},
    {15392, 9802},
    {15393, 9802},
    {15394, 9802},
    {15395, 9802},
    {15396, 9802},
    {15397, 9822},
    {15398, 9822},
    {15399, 9832},
    {15400, 9831},
    {15498, 9843},
    {15499, 9844},
    {15594, 9837},
    {15595, 9836},
    {15914, 9807},
    {15915, 9807},
    {15916, 9807},
    {15917, 9807},
    {16000, 9824},
    {16001, 9807},
    {16002, 9807},
    {16003, 9807},
    {16004, 9807},
    {16005, 9807},
    {16006, 9807},
    {16007, 9807},
    {16008, 9807},
    {16009, 9807},
    {16010, 9807},
    {16011, 9807},
    {16012, 9807},
    {16013, 9807},
    {16014, 9807},
    {16015, 9807},
    {16016, 9807},
    {16017, 9807},
    {16018, 9807},
    {16019, 9807},
    {16020, 9807},
    {16021, 9807},
    {16022, 9807},
    {16023, 9807},
    {16024, 9807},
    {16025, 9807},
    {16026, 9807},
    {16027, 9807},
    {16028, 9807},
    {16029, 9807},
    {16030, 9807},
    {16031, 9807},
    {16032, 9807},
    {16033, 9807},
    {16034, 9807},
    {16035, 9807},
    {16036, 9807},
    {16037, 9807},
    {16038, 9807},
    {16039, 9807},
    {16040, 9807},
    {16041, 9807},
    {16042, 9807},
    {16043, 9807},
    {16044, 9807},
    {16045, 9807},
    {16046, 9807},
    {16047, 9807},
    {16048, 9807},
    {16049, 9807},
    {16050, 9807},
    {16051, 9807},
    {16052, 9807},
    {16053, 9807},
    {16054, 9807},
    {16055, 9807},
    {16056, 9807},
    {16057, 9807},
    {16058, 9807},
    {16059, 9807},
    {16060, 9807},
    {16061, 9810},
    {16065, 9807},
    {16070, 9807},
    {16071, 9807},
    {16072, 9807},
    {16073, 9807},
    {16074, 9807},
    {16075, 9807},
    {16076, 9807},
    {16077, 9807},
    {16078, 9807},
    {16079, 9807},
    {16080, 9807},
    {16081, 9807},
    {16082, 9807},
    {16083, 9807},
    {16084, 9807},
    {16085, 9807},
    {16086, 9807},
    {16087, 9807},
    {16088, 9807},
    {16089, 9807},
    {16090, 9807},
    {16091, 9807},
    {16092, 9807},
    {16093, 9807},
    {16094, 9807},
    {16099, 9807},
    {16100, 9824},
    {16101, 9807},
    {16102, 9807},
    {16103, 9807},
    {16104, 9807},
    {16105, 9807},
    {16106, 9807},
    {16107, 9807},
    {16108, 9807},
    {16109, 9807},
    {16110, 9807},
    {16111, 9807},
    {16112, 9807},
    {16113, 9807},
    {16114, 9807},
    {16115, 9807},
    {16116, 9807},
    {16117, 9807},
    {16118, 9807},
    {16119, 9807},
    {16120, 9807},
    {16121, 9807},
    {16122, 9807},
    {16123, 9807},
    {16124, 9807},
    {16125, 9807},
    {16126, 9807},
    {16127, 9807},
    {16128, 9807},
    {16129, 9807},
    {16130, 9807},
    {16131, 9807},
    {16132, 9807},
    {16133, 9807},
    {16134, 9807},
    {16135, 9807},
    {16136, 9807},
    {16137, 9807},
    {16138, 9807},
    {16139, 9807},
    {16140, 9807},
    {16141, 9807},
    {16142, 9807},
    {16143, 9807},
    {16144, 9807},
    {16145, 9807},
    {16146, 9807},
    {16147, 9807},
    {16148, 9807},
    {16149, 9807},
    {16150, 9807},
    {16151, 9807},
    {16152, 9807},
    {16153, 9807},
    {16154, 9807},
    {16155, 9807},
    {16156, 9807},
    {16157, 9807},
    {16158, 9807},
    {16159, 9807},
    {16160, 9807},
    {16161, 9810},
    {16170, 9807},
    {16171, 9807},
    {16172, 9807},
    {16173, 9807},
    {16174, 9807},
    {16175, 9807},
    {16176, 9807},
    {16177, 9807},
    {16178, 9807},
    {16179, 9807},
    {16180, 9807},
    {16181, 9807},
    {16182, 9807},
    {16183, 9807},
    {16184, 9807},
    {16185, 9807},
    {16186, 9807},
    {16187, 9807},
    {16188, 9807},
    {16189, 9807},
    {16190, 9807},
    {16191, 9807},
    {16192, 9807},
    {16193, 9807},
    {16194, 9807},
    {16201, 9807},
    {16202, 9807},
    {16203, 9807},
    {16204, 9807},
    {16205, 9807},
    {16206, 9807},
    {16207, 9807},
    {16208, 9807},
    {16209, 9807},
    {16210, 9807},
    {16211, 9807},
    {16212, 9807},
    {16213, 9807},
    {16214, 9807},
    {16215, 9807},
    {16216, 9807},
    {16217, 9807},
    {16218, 9807},
    {16219, 9807},
    {16220, 9807},
    {16221, 9807},
    {16222, 9807},
    {16223, 9807},
    {16224, 9807},
    {16225, 9807},
    {16226, 9807},
    {16227, 9807},
    {16228, 9807},
    {16229, 9807},
    {16230, 9807},
    {16231, 9807},
    {16232, 9807},
    {16233, 9807},
    {16234, 9807},
    {16235, 9807},
    {16236, 9807},
    {16237, 9807},
    {16238, 9807},
    {16239, 9807},
    {16240, 9807},
    {16241, 9807},
    {16242, 9807},
    {16243, 9807},
    {16244, 9807},
    {16245, 9807},
    {16246, 9807},
    {16247, 9807},
    {16248, 9807},
    {16249, 9807},
    {16250, 9807},
    {16251, 9807},
    {16252, 9807},
    {16253, 9807},
    {16254, 9807},
    {16255, 9807},
    {16256, 9807},
    {16257, 9807},
    {16258, 9807},
    {16259, 9807},
    {16260, 9807},
    {16261, 9807},
    {16262, 9807},
    {16263, 9807},
    {16264, 9807},
    {16265, 9807},
    {16266, 9807},
    {16267, 9807},
    {16268, 9807},
    {16269, 9807},
    {16270, 9807},
    {16271, 9807},
    {16272, 9807},
    {16273, 9807},
    {16274, 9807},
    {16275, 9807},
    {16276, 9807},
    {16277, 9807},
    {16278, 9807},
    {16279, 9807},
    {16280, 9807},
    {16281, 9807},
    {16282, 9807},
    {16283, 9807},
    {16284, 9807},
    {16285, 9807},
    {16286, 9807},
    {16287, 9807},
    {16288, 9807},
    {16289, 9807},
    {16290, 9807},
    {16291, 9807},
    {16292, 9807},
    {16293, 9807},
    {16294, 9807},
    {16295, 9807},
    {16296, 9807},
    {16297, 9807},
    {16298, 9807},
    {16299, 9807},
    {16301, 9807},
    {16302, 9807},
    {16303, 9807},
    {16304, 9807},
    {16305, 9807},
    {16306, 9807},
    {16307, 9807},
    {16308, 9807},
    {16309, 9807},
    {16310, 9807},
    {16311, 9807},
    {16312, 9807},
    {16313, 9807},
    {16314, 9807},
    {16315, 9807},
    {16316, 9807},
    {16317, 9807},
    {16318, 9807},
    {16319, 9807},
    {16320, 9807},
    {16321, 9807},
    {16322, 9807},
    {16323, 9807},
    {16324, 9807},
    {16325, 9807},
    {16326, 9807},
    {16327, 9807},
    {16328, 9807},
    {16329, 9807},
    {16330, 9807},
    {16331, 9807},
    {16332, 9807},
    {16333, 9807},
    {16334, 9807},
    {16335, 9807},
    {16336, 9807},
    {16337, 9807},
    {16338, 9807},
    {16339, 9807},
    {16340, 9807},
    {16341, 9807},
    {16342, 9807},
    {16343, 9807},
    {16344, 9807},
    {16345, 9807},
    {16346, 9807},
    {16347, 9807},
    {16348, 9807},
    {16349, 9807},
    {16350, 9807},
    {16351, 9807},
    {16352, 9807},
    {16353, 9807},
    {16354, 9807},
    {16355, 9807},
    {16356, 9807},
    {16357, 9807},
    {16358, 9807},
    {16359, 9807},
    {16360, 9807},
    {16361, 9807},
    {16362, 9807},
    {16363, 9807},
    {16364, 9807},
    {16365, 9807},
    {16366, 9807},
    {16367, 9807},
    {16368, 9807},
    {16369, 9807},
    {16370, 9807},
    {16371, 9807},
    {16372, 9807},
    {16373, 9807},
    {16374, 9807},
    {16375, 9807},
    {16376, 9807},
    {16377, 9807},
    {16378, 9807},
    {16379, 9807},
    {16380, 9807},
    {16381, 9807},
    {16382, 9807},
    {16383, 9807},
    {16384, 9807},
    {16385, 9807},
    {16386, 9807},
    {16387, 9807},
    {16388, 9807},
    {16389, 9807},
    {16390, 9807},
    {16391, 9807},
    {16392, 9807},
    {16393, 9807},
    {16394, 9807},
    {16395, 9807},
    {16396, 9807},
    {16397, 9807},
    {16398, 9807},
    {16399, 9807},
    {16400, 9807},
    {16405, 9807},
    {16406, 9807},
    {16411, 9807},
    {16412, 9807},
    {16413, 9807},
    {16430, 9807},
    {16490, 9807},
    {16506, 9807},
    {16586, 9807},
    {16611, 9807},
    {16612, 9807},
    {16636, 9807},
    {16709, 9807},
    {16716, 9807},
    {16732, 9807},
    {17001, 9807},
    {17005, 9807},
    {17054, 9807},
    {17204, 9802},
    {17205, 9802},
    {17206, 9802},
    {17207, 9802},
    {17208, 9802},
    {17209, 9802},
    {17210, 9802},
    {17211, 9802},
    {17212, 9802},
    {17213, 9802},
    {17214, 9802},
    {17215, 9802},
    {17216, 9802},
    {17217, 9802},
    {17218, 9802},
    {17219, 9802},
    {17220, 9802},
    {17221, 9802},
    {17222, 9802},
    {17223, 9802},
    {17224, 9802},
    {17225, 9802},
    {17226, 9802},
    {17227, 9802},
    {17228, 9802},
    {17229, 9802},
    {17230, 9802},
    {17231, 9802},
    {17232, 9802},
    {17233, 9802},
    {17234, 9802},
    {17235, 9802},
    {17236, 9802},
    {17237, 9802},
    {17238, 9802},
    {17239, 9802},
    {17240, 9802},
    {17241, 9802},
    {17242, 9802},
    {17243, 9802},
    {17244, 9802},
    {17245, 9802},
    {17246, 9802},
    {17247, 9802},
    {17248, 9802},
    {17249, 9802},
    {17250, 9802},
    {17251, 9802},
    {17252, 9802},
    {17253, 9802},
    {17254, 9802},
    {17255, 9802},
    {17256, 9802},
    {17257, 9802},
    {17258, 9802},
    {17259, 9802},
    {17260, 9802},
    {17261, 9802},
    {17262, 9802},
    {17263, 9802},
    {17264, 9802},
    {17265, 9802},
    {17266, 9802},
    {17267, 9802},
    {17268, 9802},
    {17269, 9802},
    {17270, 9802},
    {17271, 9802},
    {17272, 9802},
    {17273, 9802},
    {17274, 9802},
    {17275, 9829},
    {17276, 9829},
    {17277, 9829},
    {17278, 9829},
    {17279, 9829},
    {17280, 9829},
    {17281, 9829},
    {17282, 9829},
    {17283, 9829},
    {17284, 9829},
    {17285, 9829},
    {17286, 9829},
    {17287, 9829},
    {17288, 9829},
    {17289, 9829},
    {17290, 9829},
    {17291, 9829},
    {17292, 9829},
    {17293, 9829},
    {17294, 9802},
    {17295, 9820},
    {17296, 9820},
    {17297, 9820},
    {17298, 9820},
    {17299, 9820},
    {17300, 9820},
    {17321, 9807},
    {17322, 9807},
    {17323, 9807},
    {17324, 9807},
    {17325, 9807},
    {17326, 9807},
    {17327, 9807},
    {17328, 9807},
    {17329, 9807},
    {17330, 9807},
    {17331, 9807},
    {17332, 9807},
    {17333, 9807},
    {17334, 9807},
    {17335, 9807},
    {17336, 9807},
    {17337, 9807},
    {17338, 9807},
    {17339, 9807},
    {17340, 9807},
    {17341, 9807},
    {17342, 9807},
    {17343, 9807},
    {17344, 9807},
    {17348, 9807},
    {17349, 9807},
    {17350, 9807},
    {17351, 9807},
    {17352, 9807},
    {17353, 9807},
    {17354, 9807},
    {17355, 9807},
    {17356, 9807},
    {17357, 9807},
    {17358, 9807},
    {17359, 9802},
    {17360, 9802},
    {17361, 9802},
    {17362, 9802},
    {17363, 9807},
    {17364, 9802},
    {17365, 9822},
    {17401, 9802},
    {17402, 9807},
    {17412, 9807},
    {17414, 9807},
    {17416, 9807},
    {17418, 9807},
    {17420, 9807},
    {17422, 9807},
    {17424, 9807},
    {17426, 9807},
    {17428, 9807},
    {17430, 9807},
    {17432, 9807},
    {17433, 9807},
    {17434, 9807},
    {17435, 9807},
    {17436, 9807},
    {17437, 9807},
    {17438, 9807},
    {17439, 9807},
    {17440, 9807},
    {17441, 9807},
    {17442, 9807},
    {17443, 9807},
    {17444, 9807},
    {17445, 9807},
    {17446, 9807},
    {17447, 9807},
    {17448, 9807},
    {17449, 9807},
    {17450, 9807},
    {17451, 9807},
    {17452, 9807},
    {17453, 9807},
    {17454, 9807},
    {17455, 9807},
    {17456, 9807},
    {17457, 9807},
    {17458, 9807},
    {17515, 9808},
    {17517, 9808},
    {17519, 9808},
    {17521, 9808},
    {17523, 9808},
    {17525, 9808},
    {17527, 9808},
    {17529, 9808},
    {17531, 9808},
    {17533, 9808},
    {17611, 9808},
    {17613, 9808},
    {17615, 9808},
    {17617, 9808},
    {17619, 9808},
    {17621, 9808},
    {17623, 9808},
    {17625, 9808},
    {17700, 9807},
    {17701, 9807},
    {17702, 9807},
    {17703, 9807},
    {17704, 9807},
    {17705, 9807},
    {17706, 9807},
    {17707, 9807},
    {17708, 9807},
    {17709, 9807},
    {17710, 9807},
    {17711, 9807},
    {17712, 9807},
    {17713, 9807},
    {17714, 9807},
    {17715, 9807},
    {17716, 9807},
    {17717, 9807},
    {17722, 9807},
    {17723, 9807},
    {17724, 9807},
    {17725, 9807},
    {17726, 9807},
    {17794, 9807},
    {17795, 9807},
    {17801, 9807},
    {17802, 9807},
    {17803, 9807},
    {17804, 9807},
    {17805, 9807},
    {17806, 9807},
    {17807, 9807},
    {17808, 9807},
    {17809, 9807},
    {17810, 9807},
    {17811, 9807},
    {17812, 9807},
    {17813, 9807},
    {17814, 9807},
    {17815, 9807},
    {17816, 9807},
    {17817, 9807},
    {17818, 9807},
    {17819, 9807},
    {17901, 9807},
    {17902, 9807},
    {17903, 9807},
    {17904, 9807},
    {17905, 9807},
    {17906, 9807},
    {17907, 9807},
    {17908, 9807},
    {17909, 9807},
    {17910, 9807},
    {17911, 9807},
    {17912, 9807},
    {17913, 9807},
    {17914, 9807},
    {17915, 9807},
    {17916, 9807},
    {17917, 9807},
    {17918, 9807},
    {17919, 9807},
    {17920, 9807},
    {17921, 9807},
    {17922, 9807},
    {17923, 9807},
    {17924, 9807},
    {17925, 9807},
    {17926, 9807},
    {17927, 9807},
    {17928, 9807},
    {17931, 9807},
    {17932, 9807},
    {17933, 9807},
    {17934, 9807},
    {17935, 9807},
    {17936, 9807},
    {17937, 9807},
    {17938, 9807},
    {17939, 9807},
    {17940, 9807},
    {17941, 9807},
    {17942, 9807},
    {17943, 9807},
    {17944, 9807},
    {17945, 9807},
    {17946, 9807},
    {17947, 9807},
    {17948, 9807},
    {17949, 9807},
    {17950, 9807},
    {17951, 9807},
    {17952, 9807},
    {17953, 9807},
    {17954, 9807},
    {17955, 9807},
    {17956, 9807},
    {17957, 9807},
    {17958, 9807},
    {17959, 9807},
    {17960, 9807},
    {17961, 9807},
    {17962, 9807},
    {17963, 9807},
    {17964, 9802},
    {17965, 9807},
    {17966, 9802},
    {18001, 9807},
    {18002, 9807},
    {18003, 9807},
    {18004, 9807},
    {18005, 9807},
    {18006, 9807},
    {18007, 9807},
    {18008, 9807},
    {18009, 9807},
    {18011, 9801},
    {18012, 9801},
    {18021, 9801},
    {18022, 9801},
    {18031, 9807},
    {18032, 9807},
    {18033, 9807},
    {18034, 9807},
    {18035, 9807},
    {18036, 9807},
    {18037, 9807},
    {18041, 9807},
    {18042, 9807},
    {18043, 9807},
    {18044, 9807},
    {18045, 9807},
    {18046, 9807},
    {18047, 9807},
    {18048, 9807},
    {18049, 9807},
    {18051, 9807},
    {18052, 9807},
    {18053, 9807},
    {18054, 9807},
    {18055, 9807},
    {18056, 9807},
    {18057, 9807},
    {18058, 9807},
    {18059, 9807},
    {18061, 9801},
    {18062, 9801},
    {18063, 9802},
    {18064, 9802},
    {18071, 9807},
    {18072, 9807},
    {18073, 9807},
    {18074, 9807},
    {18081, 9801},
    {18082, 9801},
    {18083, 9801},
    {18084, 9801},
    {18085, 9802},
    {18086, 9801},
    {18091, 9801},
    {18092, 9801},
    {18093, 9801},
    {18094, 9801},
    {18101, 9802},
    {18102, 9802},
    {18103, 9802},
    {18104, 9802},
    {18105, 9802},
    {18106, 9802},
    {18107, 9802},
    {18108, 9802},
    {18109, 9802},
    {18110, 9801},
    {18111, 9801},
    {18112, 9801},
    {18113, 9801},
    {18114, 9801},
    {18115, 9801},
    {18116, 9801},
    {18117, 9801},
    {18121, 9807},
    {18122, 9807},
    {18131, 9801},
    {18132, 9801},
    {18133, 9801},
    {18134, 9801},
    {18135, 9801},
    {18141, 9807},
    {18142, 9807},
    {18151, 9807},
    {18152, 9807},
    {18153, 9807},
    {18161, 9807},
    {18162, 9807},
    {18163, 9807},
    {18171, 9807},
    {18172, 9807},
    {18173, 9807},
    {18174, 9807},
    {18175, 9807},
    {18180, 9807},
    {18181, 9801},
    {18182, 9801},
    {18183, 9807},
    {18184, 9807},
    {18185, 9807},
    {18186, 9807},
    {18187, 9807},
    {18188, 9807},
    {18189, 9807},
    {18190, 9807},
    {18191, 9807},
    {18192, 9807},
    {18193, 9807},
    {18194, 9807},
    {18195, 9807},
    {18196, 9807},
    {18197, 9807},
    {18198, 9807},
    {18199, 9807},
    {18201, 9806},
    {18202, 9807},
    {18203, 9806},
    {18204, 9807},
    {18205, 9807},
    {18211, 9801},
    {18212, 9801},
    {18221, 9807},
    {18222, 9807},
    {18223, 9807},
    {18224, 9807},
    {18225, 9807},
    {18226, 9807},
    {18227, 9807},
    {18228, 9807},
    {18231, 9801},
    {18232, 9801},
    {18233, 9801},
    {18234, 9801},
    {18235, 9801},
    {18236, 9801},
    {18237, 9801},
    {18238, 9801},
    {18240, 9807},
    {18241, 9807},
    {18242, 9807},
    {18243, 9807},
    {18244, 9807},
    {18245, 9807},
    {18246, 9807},
    {18247, 9807},
    {18248, 9807},
    {18251, 9807},
    {18252, 9807},
    {18253, 9807},
    {18260, 9801},
    {18261, 9801},
    {18262, 9801},
    {18263, 9801},
    {18275, 9807},
    {18276, 9807},
    {18277, 9807},
    {18278, 9807},
    {18280, 9809},
    {18281, 9809},
    {18282, 9809},
    {18283, 9809},
    {18284, 9809},
    {18285, 9807},
    {18286, 9809},
    {18300, 9807},
    {18305, 9807},
    {18306, 9807},
    {18307, 9807},
    {18308, 9807},
    {18310, 9807},
    {18311, 9807},
    {18312, 9807},
    {18313, 9807},
    {18314, 9807},
    {18315, 9807},
    {18316, 9807},
    {18317, 9807},
    {18318, 9807},
    {18319, 9807},
    {18401, 9807},
    {18402, 9807},
    {18403, 9807},
    {18411, 9807},
    {18412, 9807},
    {18413, 9807},
    {18414, 9807},
    {18415, 9807},
    {18416, 9807},
    {18417, 9807},
    {18421, 9826},
    {18422, 9826},
    {18423, 9826},
    {18424, 9826},
    {18425, 9826},
    {18426, 9826},
    {18427, 9826},
    {18428, 9826},
    {18432, 9826},
    {18433, 9826},
    {18434, 9826},
    {18435, 9826},
    {18436, 9826},
    {18437, 9826},
    {18441, 9807},
    {18442, 9807},
    {18443, 9807},
    {18444, 9807},
    {18446, 9807},
    {18447, 9807},
    {18448, 9807},
    {18450, 9807},
    {18451, 9807},
    {18452, 9807},
    {19839, 9807},
    {19840, 9829},
    {19841, 9815},
    {19842, 9829},
    {19843, 9805},
    {19844, 9802},
    {19845, 9807},
    {19846, 9842},
    {19847, 9841},
    {19848, 9807},
    {19849, 9807},
    {19850, 9838},
    {19851, 9807},
    {19852, 9802},
    {19853, 9807},
    {19854, 9802},
    {19855, 9804},
    {19856, 9807},
    {19857, 9802},
    {19858, 9822},
    {19859, 9807},
    {19860, 9801},
    {19861, 9813},
    {19862, 9802},
    {19863, 9802},
    {19864, 9807},
    {19865, 9829},
    {19866, 9829},
    {19867, 9821},
    {19868, 9821},
    {19869, 9834},
    {19870, 9826},
    {19871, 9812},
    {19872, 9812},
    {19873, 9802},
    {19874, 9802},
    {19875, 9802},
    {19876, 9807},
    {19877, 9826},
    {19878, 9833},
    {19879, 9806},
    {19880, 9807},
    {19881, 9807},
    {19882, 9807},
    {19883, 9804},
    {19884, 9805},
    {19885, 9806},
    {19886, 9806},
    {19887, 9806},
    {19888, 9806},
    {19889, 9806},
    {19890, 9806},
    {19891, 9806},
    {19892, 9806},
    {19893, 9806},
    {19894, 9812},
    {19895, 9812},
    {19896, 9806},
    {19897, 9802},
    {19898, 9804},
    {19899, 9801},
    {19900, 9807},
    {19901, 9802},
    {19902, 9803},
    {19903, 9801},
    {19904, 9807},
    {19905, 9804},
    {19906, 9801},
    {19907, 9807},
    {19908, 9807},
    {19909, 9801},
    {19910, 9801},
    {19911, 9815},
    {19913, 9809},
    {19914, 9809},
    {19915, 9801},
    {19916, 9807},
    {19917, 9811},
    {19919, 9807},
    {19920, 9806},
    {19921, 9801},
    {19922, 9815},
    {19923, 9815},
    {19924, 9806},
    {19925, 9806},
    {19926, 9809},
    {19927, 9809},
    {19928, 9807},
    {19929, 9807},
    {19930, 9807},
    {19931, 9815},
    {19933, 9809},
    {19934, 9807},
    {19935, 9812},
    {19936, 9807},
    {19937, 9816},
    {19938, 9802},
    {19939, 9807},
    {19940, 9817},
    {19941, 9818},
    {19942, 9807},
    {19943, 9807},
    {19944, 9802},
    {19945, 9809},
    {19946, 9809},
    {19947, 9802},
    {19948, 9801},
    {19949, 9809},
    {19950, 9815},
    {19951, 9815},
    {19952, 9819},
    {19953, 9806},
    {19954, 9807},
    {19955, 9807},
    {19956, 9815},
    {19957, 9815},
    {19958, 9815},
    {19959, 9807},
    {19960, 9809},
    {19961, 9802},
    {19962, 9807},
    {19963, 9807},
    {19964, 9807},
    {19965, 9821},
    {19966, 9807},
    {19967, 9807},
    {19968, 9823},
    {19969, 9807},
    {19971, 9807},
    {19972, 9807},
    {19973, 9807},
    {19974, 9807},
    {19975, 9806},
    {19976, 9802},
    {19977, 9802},
    {19978, 9807},
    {19979, 9828},
    {19981, 9802},
    {19982, 9807},
    {19983, 9830},
    {19984, 9822},
    {19985, 9802},
    {19986, 9820},
    {19987, 9826},
    {19988, 9826},
    {19989, 9802},
    {19990, 9807},
    {19991, 9807},
    {19992, 9829},
    {19993, 9829},
    {19994, 9802},
    {19995, 9807},
    {19996, 9806},
    {19997, 9807},
    {19998, 9807},
    {19999, 9807}
};

bool CS_wktDatumLookUp (const char* datumNameInWkt, char* csDatumName)
{
    bool found = FALSE;    
    int  mapTypeIndex = 7; // Default is Additional
    int  colInd;
    int  rowInd;

    int  numRows = sizeof (DatumMap) / sizeof (DatumMap[0]);

    
    for (rowInd = 0 ; rowInd < numRows; ++rowInd)
    {        
        if (CS_stricmp(DatumMap[rowInd][mapTypeIndex], datumNameInWkt) == 0)
        {    
            colInd = mapTypeIndex;
            break;
        }
        else
        {        
            for (colInd = 1; colInd < DATUM_MAP_NB_COLS; colInd++)
            {
                if (colInd == mapTypeIndex) continue;

                if (CS_stricmp(DatumMap[rowInd][colInd], datumNameInWkt) == 0)
                {            
                    break;
                }
            }

            //The datum name has been found in the map.
            if (colInd < DATUM_MAP_NB_COLS)
                break;
        }
    }

    if (rowInd < numRows)
    {
        // If the CSMAP value is empty ... we use the EPSG value
        if (CS_stricmp(DatumMap[rowInd][0], "") == 0) 
            CS_stncp(csDatumName, DatumMap[rowInd][5], DATUM_NAME_LENGTH_LIMIT);
        else
            CS_stncp(csDatumName, DatumMap[rowInd][0], DATUM_NAME_LENGTH_LIMIT);
        found = TRUE;
    }

    return found;
}

bool CS_wktProjectionMethodEPSGLookUp(int projectionCode, int* projectionMethodCode)
{
    bool found = FALSE;
    int  rowInd;

    int  numRows = sizeof(ProjectionMap) / sizeof(ProjectionMap[0]);

    for (rowInd = 0; rowInd < numRows; ++rowInd)
    {
        if (projectionCode == ProjectionMap[rowInd][0])
        {
            *projectionMethodCode = ProjectionMap[rowInd][1];
            found = true;
            break;
        }
    }

    return found;
}

bool CS_wktEllipsoidLookUp (const char* ellipsoidNameInWkt, char* csEllipsoidName)
{
    bool found = FALSE;    
    int  mapTypeIndex = 5; // Default is Additional
    int  colInd;
    int  rowInd;

    int  numRows = sizeof (EllipsoidMap) / sizeof (EllipsoidMap[0]);
    
    for (rowInd = 0 ; rowInd < numRows; ++rowInd)
    {        
        if (CS_stricmp(EllipsoidMap[rowInd][mapTypeIndex], ellipsoidNameInWkt) == 0)
        {    
            colInd = mapTypeIndex;
            break;
        }
        else
        {        
            for (colInd = 1; colInd < ELLIPSOID_MAP_NB_COLS; colInd++)
            {
                if (colInd == mapTypeIndex) continue;

                if (CS_stricmp(EllipsoidMap[rowInd][colInd], ellipsoidNameInWkt) == 0)
                {            
                    break;
                }
            }

            //The datum name has been found in the map.
            if (colInd < ELLIPSOID_MAP_NB_COLS)
                break;
        }
    }

    if (rowInd < numRows)
    {
        // If the CSMAP value is empty ... we use the EPSG value
        if (CS_stricmp(EllipsoidMap[rowInd][0], "") == 0) 
            CS_stncp(csEllipsoidName, EllipsoidMap[rowInd][2], ELLIPSOID_NAME_LENGTH_LIMIT);
        else
            CS_stncp(csEllipsoidName, EllipsoidMap[rowInd][0], ELLIPSOID_NAME_LENGTH_LIMIT);
        found = TRUE;
    }

    return found;
}



