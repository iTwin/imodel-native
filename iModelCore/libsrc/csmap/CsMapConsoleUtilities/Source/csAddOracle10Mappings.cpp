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

struct csOracleNbrMap_
{
	long32_t oracleNbr;
	long32_t epsgNbr;
	char msiName [24];
	char oracleName [96];
	unsigned short flags;
};
struct csOracleNbrMap_ csOracleNbrMapCS [] =
{
	{       -1L,    -1L, "WORLD-LL",                   "WORLD-LL",                                                         0 },	// ADSK-HW 01/30/2007
	{     4121L,  4121L, "GGRS87.LL",                  "GGRS87",                                                           0 },	// GGRS87
	{     4122L,  4122L, "LL77",                       "ATS77",                                                            1 },	// ATS77
	{     4123L,  4123L, "KKJ.LL",                     "KKJ",                                                              0 },	// KKJ
	{     4124L,  4124L, "LL-RT90",                    "RT90",                                                             1 },	// RT90
	{     4125L,  4125L, "Samboja.LL",                 "Samboja",                                                          2 },	// Samboja
	{     4126L,  4126L, "Lithuania94.LL",             "LKS94 (ETRS89)",                                                   2 },	// LKS94 (ETRS89)
	{     4127L,  4127L, "Tete.LL",                    "Tete",                                                             0 },	// Tete
	{     4130L,  4130L, "Moznet.LL",                  "Moznet",                                                           0 },	// Moznet
	{     4131L,  4131L, "Indian60.LL",                "Indian 1960",                                                      0 },	// Indian 1960
	{     4132L,  4132L, "Final58.LL",                 "FD58",                                                             0 },	// FD58
	{     4133L,  4133L, "Estonia92",                  "EST92",                                                            0 },	// EST92
	{     4134L,  4134L, "PDOSurvey93.LL",             "PDO Survey Datum 1993",                                            0 },	// PDO Survey Datum 1993
	{     4135L,  4135L, "LL-OLDHI",                   "Old Hawaiian",                                                     1 },	// Old Hawaiian
	{     4139L,  4139L, "PRVI.LL",                    "Puerto Rico",                                                      0 },	// Puerto Rico
	{     4140L,  4140L, "LLCSRS",                     "NAD83(CSRS98)",                                                    3 },	// NAD83(CSRS98)
	{     4141L,  4141L, "Israel.LL",                  "Israel",                                                           0 },	// Israel
	{     4142L,  4142L, "Locodjo65.LL",               "Locodjo 1965",                                                     0 },	// Locodjo 1965
	{     4143L,  4143L, "Abidjan87.LL",               "Abidjan 1987",                                                     0 },	// Abidjan 1987
	{     4144L,  4144L, "Kalianpur37.LL",             "Kalianpur 1937",                                                   0 },	// Kalianpur 1937
	{     4145L,  4145L, "Kalianpur62.LL",             "Kalianpur 1962",                                                   0 },	// Kalianpur 1962
	{     4146L,  4146L, "Kalianpur75.LL",             "Kalianpur 1975",                                                   0 },	// Kalianpur 1975
	{     4147L,  4147L, "Hanoi72.LL",                 "Hanoi 1972",                                                       0 },	// Hanoi 1972
	{     4148L,  4148L, "Hartebeesthoek94.LL",        "Hartebeesthoek94",                                                 0 },	// Hartebeesthoek94
	{     4149L,  4149L, "LLCH1903",                   "CH1903",                                                           1 },	// CH1903
	{     4150L,  4150L, "CH1903Plus.LL",              "CH1903+",                                                          0 },	// CH1903+
	{     4151L,  4151L, "CHTRF95.LL",                 "CHTRF95",                                                          0 },	// CHTRF95
	{     4152L,  4152L, "LL-HPGN",                    "NAD83(HARN)",                                                      1 },	// NAD83(HARN)
	{     4153L,  4153L, "Rassadiran.LL",              "Rassadiran",                                                       0 },	// Rassadiran
	{     4154L,  4154L, "Europ50/77.LL",              "ED50(ED77)",                                                       0 },	// ED50(ED77)
	{     4155L,  4155L, "Dabola81.LL",                "Dabola 1981",                                                      0 },	// Dabola 1981
	{     4156L,  4156L, "S-JTSK.LL",                  "S-JTSK",                                                           1 },	// S-JTSK
	{     4158L,  4158L, "NAPARIMA.LL",                "Naparima 1955",                                                    0 },	// Naparima 1955
	{     4159L,  4159L, "EuropLibyan79.LL",           "ELD79",                                                            0 },	// ELD79
	{     4161L,  4161L, "Castillo.LL",                "Pampa del Castillo",                                               0 },	// Pampa del Castillo
	{     4163L,  4163L, "YemenNtl96.LL",              "Yemen NGN96",                                                      0 },	// Yemen NGN96
	{     4164L,  4164L, "SouthYemen.LL",              "South Yemen",                                                      0 },	// South Yemen
	{     4165L,  4165L, "Bissau.LL",                  "Bissau",                                                           0 },	// Bissau
	{     4166L,  4166L, "Korean95.LL",                "Korean 1995",                                                      0 },	// Korean 1995
	{     4167L,  4167L, "NZGD2000.LL",                "NZGD2000",                                                         0 },	// NZGD2000
	{     4168L,  4168L, "Accra.LL",                   "Accra",                                                            0 },	// Accra
	{     4169L,  4169L, "AmSamoa62.LL",               "American Samoa 1962",                                              0 },	// American Samoa 1962
	{     4170L,  4170L, "GRSSA.LL",                   "SIRGAS",                                                           0 },	// SIRGAS
	{     4171L,  4171L, "LL-RGF93",                   "RGF93",                                                            1 },	// RGF93
	{     4173L,  4173L, "IRENET95.LL",                "IRENET95",                                                         0 },	// IRENET95
	{     4175L,  4175L, "SierraLeone68.LL",           "Sierra Leone 1968",                                                0 },	// Sierra Leone 1968
	{     4176L,  4176L, "Antarctic98.LL",             "Australian Antarctic",                                             0 },	// Australian Antarctic
	{     4178L,  4178L, "Pulkovo42/83.LL",            "Pulkovo 1942(83)",                                                 0 },	// Pulkovo 1942(83)
	{     4179L,  4179L, "Pulkovo42/58.LL",            "Pulkovo 1942(58)",                                                 0 },	// Pulkovo 1942(58)
	{     4180L,  4180L, "Estonia97.LL",               "EST97",                                                            0 },	// EST97
	{     4181L,  4181L, "Luxembourg30.LL",            "Luxembourg 1930",                                                  0 },	// Luxembourg 1930
	{     4182L,  4182L, "OBSRV66.LL",                 "Azores Occidental 1939",                                           1 },	// Azores Occidental 1939
	{     4183L,  4183L, "AZORES.LL",                  "Azores Central 1948",                                              0 },	// Azores Central 1948
	{     4184L,  4184L, "SAOBRAZ.LL",                 "Azores Oriental 1940",                                             0 },	// Azores Oriental 1940
	{     4188L,  4188L, "OSNI52.LL",                  "OSNI 1952",                                                        0 },	// OSNI 1952
	{     4189L,  4189L, "REGVEN.LL",                  "REGVEN",                                                           0 },	// REGVEN
	{     4190L,  4190L, "PGA98.LL",                   "POSGAR 98",                                                        0 },	// POSGAR 98
	{     4192L,  4192L, "Douala48.LL",                "Douala 1948",                                                      0 },	// Douala 1948
	{     4193L,  4193L, "Manoca62.LL",                "Manoca 1962",                                                      0 },	// Manoca 1962
	{     4194L,  4194L, "Qornoq27.LL",                "Qornoq 1927",                                                      0 },	// Qornoq 1927
	{     4195L,  4195L, "Scoresbysund52.LL",          "Scoresbysund 1952",                                                0 },	// Scoresbysund 1952
	{     4196L,  4196L, "Ammassalik58.LL",            "Ammassalik 1958",                                                  0 },	// Ammassalik 1958
	{     4201L,  4201L, "Adindan.LL",                 "Adindan",                                                          0 },	// Adindan
	{     4202L,  4202L, "LL-AGD66",                   "AGD66",                                                            1 },	// AGD66
	{     4203L,  4203L, "LL-AGD84",                   "AGD84",                                                            1 },	// AGD84
	{     4204L,  4204L, "AinElAbd.LL",                "Ain el Abd",                                                       0 },	// Ain el Abd
	{     4205L,  4205L, "Afgooye.LL",                 "Afgooye",                                                          0 },	// Afgooye
	{     4207L,  4207L, "Lisbon37.LL",                "Lisbon",                                                           0 },	// Lisbon
	{     4208L,  4208L, "Aratu.LL",                   "Aratu",                                                            0 },	// Aratu
	{     4209L,  4209L, "Arc1950.LL",                 "Arc 1950",                                                         0 },	// Arc 1950
	{     4210L,  4210L, "Arc1960.LL",                 "Arc 1960",                                                         0 },	// Arc 1960
	{     4211L,  4211L, "Batavia.LL",                 "Batavia",                                                          0 },	// Batavia
	{     4212L,  4212L, "Barbados38.LL",              "Barbados 1938",                                                    0 },	// Barbados 1938
	{     4214L,  4214L, "Beijing54.LL",               "Beijing 1954",                                                     0 },	// Beijing 1954
	{     4216L,  4216L, "Bermuda.LL",                 "Bermuda 1957",                                                     0 },	// Bermuda 1957
	{     4218L,  4218L, "Bogota.LL",                  "Bogota 1975",                                                      0 },	// Bogota 1975
	{     4219L,  4219L, "BukitRimpah.LL",             "Bukit Rimpah",                                                     0 },	// Bukit Rimpah
	{     4220L,  4220L, "Camacupa.LL",                "Camacupa",                                                         0 },	// Camacupa
	{     4221L,  4221L, "Campo.LL",                   "Campo Inchauspe",                                                  0 },	// Campo Inchauspe
	{     4222L,  4222L, "Cape-1.LL",                  "Cape",                                                             0 },	// Cape
	{     4223L,  4223L, "Carthage.LL",                "Carthage",                                                         0 },	// Carthage
	{     4224L,  4224L, "Chua.LL",                    "Chua",                                                             0 },	// Chua
	{  2000016L,  4225L, "Corrego.LL",                 "Corrego Alegre",                                                   0 },	// Corrego Alegre
	{     4227L,  4227L, "DeirEzZor.LL",               "Deir ez Zor",                                                      0 },	// Deir ez Zor
	{     4229L,  4229L, "Old-Egyp.LL",                "Egypt 1907",                                                       0 },	// Egypt 1907
	{     4230L,  4230L, "LL-ERP50",                   "ED50",                                                             1 },	// ED50
	{     4231L,  4231L, "Europ87.LL",                 "ED87",                                                             0 },	// ED87
	{     4232L,  4232L, "Fahud.LL",                   "Fahud",                                                            0 },	// Fahud
	{     4233L,  4233L, "Gandajika.LL",               "Gandajika 1970",                                                   2 },	// Gandajika 1970
	{     4236L,  4236L, "HuTzuShan.LL",               "Hu Tzu Shan",                                                      0 },	// Hu Tzu Shan
	{     4237L,  4237L, "HUN72.LL",                   "HD72",                                                             0 },	// HD72
	{     4238L,  4238L, "Indonesian74.LL",            "ID74",                                                             0 },	// ID74
	{     4239L,  4239L, "Indian54.LL",                "Indian 1954",                                                      0 },	// Indian 1954
	{     4240L,  4240L, "Indian75.LL",                "Indian 1975",                                                      0 },	// Indian 1975
	{     4242L,  4242L, "Jamaica69.LL",               "JAD69",                                                            0 },	// JAD69
	{     4244L,  4244L, "Kandawala.LL",               "Kandawala",                                                        0 },	// Kandawala
	{     4245L,  4245L, "Kertau48.LL",                "Kertau",                                                           0 },	// Kertau 1968
	{     4246L,  4246L, "KuwaitOilCo.LL",             "KOC",                                                              0 },	// KOC
	{     4247L,  4247L, "LaCanoa.LL",                 "La Canoa",                                                         0 },	// La Canoa
	{     4248L,  4248L, "LLPSAD56",                   "PSAD56",                                                           1 },	// PSAD56
	{     4250L,  4250L, "Leigon.LL",                  "Leigon",                                                           0 },	// Leigon
	{     4251L,  4251L, "Liberia.LL",                 "Liberia 1964",                                                     0 },	// Liberia 1964
	{     4253L,  4253L, "Luzon.LL",                   "Luzon 1911",                                                       0 },	// Luzon 1911
	{     4254L,  4254L, "HitoXVIII63.LL",             "Hito XVIII 1963",                                                  0 },	// Hito XVIII 1963
	{     4255L,  4255L, "HeratNorth.LL",              "Herat North",                                                      0 },	// Herat North
	{     4256L,  4256L, "Mahe1971.LL",                "Mahe 1971",                                                        0 },	// Mahe 1971
	{     4257L,  4257L, "Makassar.LL",                "Makassar",                                                         0 },	// Makassar
	{     4258L,  4258L, "LL-ETRS89",                  "ETRS89",                                                           0 },	// ETRS89
	{     4259L,  4259L, "Malongo87.LL",               "Malongo 1987",                                                     0 },	// Malongo 1987
	{     4260L,  4260L, "Manoca.LL",                  "Manoca",                                                           2 },	// Manoca
	{     4261L,  4261L, "Merchich",                   "Merchich",                                                         0 },	// Merchich
	{     4262L,  4262L, "Massawa.LL",                 "Massawa",                                                          0 },	// Massawa
	{     4263L,  4263L, "Minna.LL",                   "Minna",                                                            0 },	// Minna
	{     4264L,  4264L, "Mhast.LL",                   "Mhast",                                                            2 },	// Mhast
	{  2000019L,  4265L, "ROME1940.LL",                "Rome 1940",                                                        1 },	// Monte Mario
	{     4266L,  4266L, "Mporaloko.LL",               "M'poraloko",                                                       0 },	// M'poraloko
	{     4267L,  4267L, "LL27",                       "NAD27",                                                            1 },	// NAD27
	{     4268L,  4268L, "MICHIGAN.LL",                "NAD27 Michigan",                                                   1 },	// NAD27 Michigan
	{     4269L,  4269L, "LL83",                       "NAD83",                                                            1 },	// NAD83
	{     4270L,  4270L, "NHRWN-O.LL",                 "Nahrwan 1967",                                                     0 },	// Nahrwan 1967
	{     4271L,  4271L, "Naparima72.LL",              "Naparima 1972",                                                    0 },	// Naparima 1972
	{     4272L,  4272L, "LL-NZGD49",                  "NZGD49",                                                           1 },	// NZGD49
	{     4273L,  4273L, "NGO48.LL",                   "NGO 1948",                                                         0 },	// NGO 1948
	{     4274L,  4274L, "Datum73.LL",                 "Datum 73",                                                         0 },	// Datum 73
	{     4275L,  4275L, "NTF.LL",                     "NTF",                                                              1 },	// NTF
	{     4277L,  4277L, "OSGB.LL",                    "OSGB 1936",                                                        1 },	// OSGB 1936
	{     4281L,  4281L, "Palestine23.LL",             "Palestine 1923",                                                   0 },	// Palestine 1923
	{     4282L,  4282L, "PointeNoire.LL",             "Pointe Noire",                                                     0 },	// Pointe Noire
	{     5807L,  4283L, "LL-GDA94",                   "GDA94",                                                            1 },	// GDA94
	{  2000005L,  4284L, "Pulkovo42.LL",               "Pulkovo 1942",                                                     0 },	// Pulkovo 1942
	{     4285L,  4285L, "Qatar74.LL",                 "Qatar 1974",                                                       0 },	// Qatar 1974
	{     4286L,  4286L, "Qatar.LL",                   "Qatar 1948",                                                       0 },	// Qatar 1948
	{     4287L,  4287L, "Qornoq.LL",                  "Qornoq",                                                           2 },	// Qornoq
	{     4289L,  4289L, "Amersfoort.LL",              "Amersfoort",                                                       1 },	// Amersfoort
	{     4618L,  4291L, "SAD69.LL",                   "SAD69",                                                            2 },	// SAD69
	{     4292L,  4292L, "Sapper.LL",                  "Sapper Hill 1943",                                                 0 },	// Sapper Hill 1943
	{     4293L,  4293L, "Schwarzk.LL",                "Schwarzeck",                                                       0 },	// Schwarzeck
	{     4294L,  4294L, "Segora.LL",                  "Segora",                                                           2 },	// Segora
	{     4297L,  4297L, "Tananarive25.LL",            "Tananarive",                                                       0 },	// Tananarive
	{     4298L,  4298L, "TIMBALAI.LL",                "Timbalai 1948",                                                    1 },	// Timbalai 1948
	{     4299L,  4299L, "TM65.LL",                    "TM65",                                                             0 },	// TM65
	{  2000003L,  4301L, "Tokyo",                      "Tokyo",                                                            0 },	// Tokyo
	{     4302L,  4302L, "Trinidad03.LL",              "Trinidad 1903",                                                    0 },	// Trinidad 1903
	{     4304L,  4304L, "Voirol1875.LL",              "Voirol 1875",                                                      0 },	// Voirol 1875
	{     4307L,  4307L, "NordSahara59.LL",            "Nord Sahara 1959",                                                 0 },	// Nord Sahara 1959
	{     4309L,  4309L, "Yacare.LL",                  "Yacare",                                                           0 },	// Yacare
	{     4311L,  4311L, "Zanderij.LL",                "Zanderij",                                                         0 },	// Zanderij
	{     4312L,  4312L, "MGI.LL",                     "MGI",                                                              0 },	// MGI
	{     4313L,  4313L, "Belge72.LL",                 "Belge 1972",                                                       0 },	// Belge 1972
	{     4314L,  4314L, "DHDN.LL",                    "DHDN",                                                             0 },	// DHDN
	{     4315L,  4315L, "Conakry05.LL",               "Conakry 1905",                                                     0 },	// Conakry 1905
	{     4316L,  4316L, "DealulPiscului33.LL",        "Dealul Piscului 1933",                                             0 },	// Dealul Piscului 1933
	{     4317L,  4317L, "DealulPiscului70.LL",        "Dealul Piscului 1970",                                             0 },	// Dealul Piscului 1970
	{     4318L,  4318L, "NtlGeodeticNet.LL",          "NGN",                                                              0 },	// NGN
	{     4319L,  4319L, "KuwaitUtility.LL",           "KUDAMS",                                                           0 },	// KUDAMS
	{     4322L,  4322L, "LL72",                       "WGS 72",                                                           1 },	// WGS 72
	{     4324L,  4324L, "WGS72-TBE.LL",               "WGS 72BE",                                                         0 },	// WGS 72BE
	{  2000002L,  4326L, "LL84",                       "WGS 84",                                                           1 },	// WGS 84
	{     4601L,  4601L, "Antigua43.LL",               "Antigua 1943",                                                     0 },	// Antigua 1943
	{     4602L,  4602L, "Dominica45.LL",              "Dominica 1945",                                                    0 },	// Dominica 1945
	{     4603L,  4603L, "Grenada53.LL",               "Grenada 1953",                                                     0 },	// Grenada 1953
	{     4604L,  4604L, "Montserrat58.LL",            "Montserrat 1958",                                                  0 },	// Montserrat 1958
	{     4605L,  4605L, "StKitts55.LL",               "St. Kitts 1955",                                                   0 },	// St. Kitts 1955
	{     4606L,  4606L, "StLucia55.LL",               "St. Lucia 1955",                                                   0 },	// St. Lucia 1955
	{     4607L,  4607L, "StVincent45.LL",             "St. Vincent 1945",                                                 0 },	// St. Vincent 1945
	{     4610L,  4610L, "Xian80.LL",                  "Xian 1980",                                                        0 },	// Xian 1980
	{     4611L,  4611L, "HongKong80.LL",              "Hong Kong 1980",                                                   0 },	// Hong Kong 1980
	{     4612L,  4612L, "JGD2000.LL",                 "JGD2000",                                                          0 },	// JGD2000
	{     4613L,  4613L, "Segara.LL",                  "Segara",                                                           0 },	// Segara
	{     4614L,  4614L, "QatarNtl95.LL",              "QND95",                                                            0 },	// QND95
	{     4615L,  4615L, "MADEIRA.LL",                 "Porto Santo",                                                      1 },	// Porto Santo
	{     4616L,  4616L, "SelvagemGrande.LL",          "Selvagem Grande",                                                  0 },	// Selvagem Grande
	{     4617L,  4617L, "LL-CSRS",                    "NAD83(CSRS)",                                                      1 },	// NAD83(CSRS)
	{     4619L,  4619L, "SWEREF99.LL",                "SWEREF99",                                                         0 },	// SWEREF99
	{     4620L,  4620L, "Point58.LL",                 "Point 58",                                                         0 },	// Point 58
	{     4621L,  4621L, "FortMarigot.LL",             "Fort Marigot",                                                     0 },	// Fort Marigot
	{     4622L,  4622L, "StAnne.LL",                  "Sainte Anne",                                                      0 },	// Guadeloupe 1948
	{     4623L,  4623L, "CSG67.LL",                   "CSG67",                                                            0 },	// CSG67
	{     4624L,  4624L, "Guyane95.LL",                "RGFG95",                                                           0 },	// RGFG95
	{     4625L,  4625L, "FortDesaix.LL",              "Fort Desaix",                                                      0 },	// Martinique 1938
	{     4626L,  4626L, "REUNION.LL",                 "Piton des Neiges",                                                 0 },	// Reunion 1947
	{     4627L,  4627L, "Reunion92.LL",               "RGR92",                                                            0 },	// RGR92
	{     4628L,  4628L, "Tahiti.LL",                  "Tahiti",                                                           0 },	// Tahiti 52
	{     4629L,  4629L, "Tahaa.LL",                   "Tahaa",                                                            0 },	// Tahaa 54
	{     4630L,  4630L, "IGN72NukuHiva.LL",           "IGN72 Nuku Hiva",                                                  0 },	// IGN72 Nuku Hiva
	{     4631L,  4631L, "K01949.LL",                  "K0 1949",                                                          2 },	// K0 1949
	{     4632L,  4632L, "Combani50.LL",               "Combani 1950",                                                     0 },	// Combani 1950
	{     4633L,  4633L, "IGN56Lifou.LL",              "IGN56 Lifou",                                                      0 },	// IGN56 Lifou
	{     4634L,  4634L, "IGN72GrandTerre.LL",         "IGN72 Grand Terre",                                                2 },	// IGN72 Grand Terre
	{     4635L,  4635L, "ST87Ouvea.LL",               "ST87 Ouvea",                                                       2 },	// ST87 Ouvea
	{     4636L,  4636L, "Petrels72.LL",               "Petrels 1972",                                                     0 },	// Petrels 1972
	{     4637L,  4637L, "Perroud50.LL",               "Perroud 1950",                                                     0 },	// Perroud 1950
	{     4638L,  4638L, "Miquelon50.LL",              "Saint Pierre et Miquelon 1950",                                    0 },	// Saint Pierre et Miquelon 1950
	{     4639L,  4639L, "MOP78.LL",                   "MOP78",                                                            0 },	// MOP78
	{     4640L,  4640L, "Antilles91.LL",              "RRAF 1991",                                                        0 },	// RRAF 1991
	{     4641L,  4641L, "IGN53/Mare.LL",              "IGN53 Mare",                                                       0 },	// IGN53 Mare
	{     4642L,  4642L, "ST84IleDesPins.LL",          "ST84 Ile des Pins",                                                0 },	// ST84 Ile des Pins
	{     4643L,  4643L, "ST71Belep.LL",               "ST71 Belep",                                                       0 },	// ST71 Belep
	{     4644L,  4644L, "NEA74Noumea.LL",             "NEA74 Noumea",                                                     0 },	// NEA74 Noumea
	{     4645L,  4645L, "Caledonie91.LL",             "RGNC 1991",                                                        2 },	// RGNC 1991
	{     4657L,  4657L, "Reykjavik.LL",               "Reykjavik 1900",                                                   0 },	// Reykjavik 1900
	{     4658L,  4658L, "HJORSEY.LL",                 "Hjorsey 1955",                                                     1 },	// Hjorsey 1955
	{     4659L,  4659L, "IslandsNet1993.LL",          "ISN93",                                                            0 },	// ISN93
	{     4660L,  4660L, "Helle1954.LL",               "Helle 1954",                                                       0 },	// Helle 1954
	{     4661L,  4661L, "Latvia1992.LL",              "LKS92",                                                            0 },	// LKS92
	{     4662L,  4662L, "IGN72GrandeTerre.LL",        "IGN72 Grande Terre",                                               0 },	// IGN72 Grande Terre
	{     4663L,  4663L, "PortoSanto1995.LL",          "Porto Santo 1995",                                                 0 },	// Porto Santo 1995
	{     4664L,  4664L, "AzoresOriental1995.LL",      "Azores Oriental 1995",                                             0 },	// Azores Oriental 1995
	{     4665L,  4665L, "AzoresCentral1995.LL",       "Azores Central 1995",                                              0 },	// Azores Central 1995
	{     4666L,  4666L, "Lisbon1890.LL",              "Lisbon 1890",                                                      0 },	// Lisbon 1890
	{     4667L,  4667L, "IraqKuwait1992.LL",          "IKBD-92",                                                          0 },	// IKBD-92
	{     4668L,  4668L, "EUROP79.LL",                 "ED79",                                                             1 },	// ED79
	{     4669L,  4669L, "Lithuania94.LL",             "LKS94",                                                            0 },	// LKS94
	{     4670L,  4670L, "IGM1995.LL",                 "IGM95",                                                            0 },	// IGM95
	{  2000031L,  4712L, "ASCENSN.LL",                 "Ascension Island 1958",                                            1 },	// Ascension Island 1958
	{     2001L,  2001L, "Antigua43.BWIgrid",          "Antigua 1943 / British West Indies Grid",                          0 },	// Antigua 1943 / British West Indies Grid
	{     2002L,  2002L, "Dominica45.BWIgrid",         "Dominica 1945 / British West Indies Grid",                         0 },	// Dominica 1945 / British West Indies Grid
	{     2003L,  2003L, "Grenada53.BWIgrid",          "Grenada 1953 / British West Indies Grid",                          0 },	// Grenada 1953 / British West Indies Grid
	{     2004L,  2004L, "Montserrat58.BWIgrid",       "Montserrat 58 / British West Indies Grid",                         0 },	// Montserrat 1958 / British West Indies Grid
	{     2037L,  2037L, "NAD83/98.UTM-19N",           "NAD83(CSRS98) / UTM zone 19N",                                     2 },	// NAD83(CSRS98) / UTM zone 19N
	{     2038L,  2038L, "NAD83/98.UTM-20N",           "NAD83(CSRS98) / UTM zone 20N",                                     2 },	// NAD83(CSRS98) / UTM zone 20N
	{     2039L,  2039L, "Israel.IsraeliGrid",         "Israel / Israeli TM Grid",                                         0 },	// Israel / Israeli TM Grid
	{     2040L,  2040L, "Locodjo65.UTM-30N",          "Locodjo 1965 / UTM zone 30N",                                      0 },	// Locodjo 1965 / UTM zone 30N
	{     2041L,  2041L, "Abidjan87.UTM-30N",          "Abidjan 1987 / UTM zone 30N",                                      0 },	// Abidjan 1987 / UTM zone 30N
	{     2042L,  2042L, "Locodjo65.UTM-29N",          "Locodjo 1965 / UTM zone 29N",                                      0 },	// Locodjo 1965 / UTM zone 29N
	{     2043L,  2043L, "Abidjan87.UTM-29N",          "Abidjan 1987 / UTM zone 29N",                                      0 },	// Abidjan 1987 / UTM zone 29N
	{     2044L,  2044L, "Hanoi72.GK-18",              "Hanoi 1972 / Gauss-Kruger zone 18",                                0 },	// Hanoi 1972 / Gauss-Kruger zone 18
	{     2045L,  2045L, "Hanoi72.GK-19",              "Hanoi 1972 / Gauss-Kruger zone 19",                                0 },	// Hanoi 1972 / Gauss-Kruger zone 19
	{     2046L,  2046L, "Hartebeesthoek94.Lo15",      "Hartebeesthoek94 / Lo15",                                          0 },	// Hartebeesthoek94 / Lo15
	{     2047L,  2047L, "Hartebeesthoek94.Lo17",      "Hartebeesthoek94 / Lo17",                                          0 },	// Hartebeesthoek94 / Lo17
	{     2048L,  2048L, "Hartebeesthoek94.Lo19",      "Hartebeesthoek94 / Lo19",                                          0 },	// Hartebeesthoek94 / Lo19
	{     2049L,  2049L, "Hartebeesthoek94.Lo21",      "Hartebeesthoek94 / Lo21",                                          0 },	// Hartebeesthoek94 / Lo21
	{     2050L,  2050L, "Hartebeesthoek94.Lo23",      "Hartebeesthoek94 / Lo23",                                          0 },	// Hartebeesthoek94 / Lo23
	{     2051L,  2051L, "Hartebeesthoek94.Lo25",      "Hartebeesthoek94 / Lo25",                                          0 },	// Hartebeesthoek94 / Lo25
	{     2052L,  2052L, "Hartebeesthoek94.Lo27",      "Hartebeesthoek94 / Lo27",                                          0 },	// Hartebeesthoek94 / Lo27
	{     2053L,  2053L, "Hartebeesthoek94.Lo29",      "Hartebeesthoek94 / Lo29",                                          0 },	// Hartebeesthoek94 / Lo29
	{     2054L,  2054L, "Hartebeesthoek94.Lo31",      "Hartebeesthoek94 / Lo31",                                          0 },	// Hartebeesthoek94 / Lo31
	{     2055L,  2055L, "Hartebeesthoek94.Lo33",      "Hartebeesthoek94 / Lo33",                                          0 },	// Hartebeesthoek94 / Lo33
	{     2057L,  2057L, "Rassadiran.NakhlTaqi",       "Rassadiran / Nakhl e Taqi",                                        0 },	// Rassadiran / Nakhl e Taqi
	{     2058L,  2058L, "ED50/77.UTM-38N",            "ED50(ED77) / UTM zone 38N",                                        0 },	// ED50(ED77) / UTM zone 38N
	{     2059L,  2059L, "ED50/77.UTM-39N",            "ED50(ED77) / UTM zone 39N",                                        0 },	// ED50(ED77) / UTM zone 39N
	{     2060L,  2060L, "ED50/77.UTM-40N",            "ED50(ED77) / UTM zone 40N",                                        0 },	// ED50(ED77) / UTM zone 40N
	{     2061L,  2061L, "ED50/77.UTM-41N",            "ED50(ED77) / UTM zone 41N",                                        0 },	// ED50(ED77) / UTM zone 41N
	{     2063L,  2063L, "Dabola81.UTM-28N",           "Dabola 1981 / UTM zone 28N",                                       0 },	// Dabola 1981 / UTM zone 28N
	{     2064L,  2064L, "Dabola81.UTM-29N",           "Dabola 1981 / UTM zone 29N",                                       0 },	// Dabola 1981 / UTM zone 29N
	{     2065L,  2065L, "Czech/JTSK.Krovak",          "S-JTSK (Ferro) / Krovak",                                          1 },	// S-JTSK (Ferro) / Krovak
	{     2067L,  2067L, "Naparima55.UTM-20N",         "Naparima 1955 / UTM zone 20N",                                     0 },	// Naparima 1955 / UTM zone 20N
	{     2068L,  2068L, "ELD79.Libya-5",              "ELD79 / Libya zone 5",                                             0 },	// ELD79 / Libya zone 5
	{     2069L,  2069L, "ELD79.Libya-6",              "ELD79 / Libya zone 6",                                             0 },	// ELD79 / Libya zone 6
	{     2070L,  2070L, "ELD79.Libya-7",              "ELD79 / Libya zone 7",                                             0 },	// ELD79 / Libya zone 7
	{     2071L,  2071L, "ELD79.Libya-8",              "ELD79 / Libya zone 8",                                             0 },	// ELD79 / Libya zone 8
	{     2072L,  2072L, "ELD79.Libya-9",              "ELD79 / Libya zone 9",                                             0 },	// ELD79 / Libya zone 9
	{     2073L,  2073L, "ELD79.Libya-10",             "ELD79 / Libya zone 10",                                            0 },	// ELD79 / Libya zone 10
	{     2074L,  2074L, "ELD79.Libya-11",             "ELD79 / Libya zone 11",                                            0 },	// ELD79 / Libya zone 11
	{     2075L,  2075L, "ELD79.Libya-12",             "ELD79 / Libya zone 12",                                            0 },	// ELD79 / Libya zone 12
	{     2076L,  2076L, "ELD79.Libya-13",             "ELD79 / Libya zone 13",                                            0 },	// ELD79 / Libya zone 13
	{     2077L,  2077L, "ELD79.UTM-32N",              "ELD79 / UTM zone 32N",                                             0 },	// ELD79 / UTM zone 32N
	{     2078L,  2078L, "ELD79.UTM-33N",              "ELD79 / UTM zone 33N",                                             0 },	// ELD79 / UTM zone 33N
	{     2079L,  2079L, "ELD79.UTM-34N",              "ELD79 / UTM zone 34N",                                             0 },	// ELD79 / UTM zone 34N
	{     2080L,  2080L, "ELD79.UTM-35N",              "ELD79 / UTM zone 35N",                                             0 },	// ELD79 / UTM zone 35N
	{     2082L,  2082L, "Castillo.Argentina-2",       "Pampa del Castillo / Argentina zone 2",                            0 },	// Pampa del Castillo / Argentina zone 2
	{     2083L,  2083L, "Hito63.Argentina-2",         "Hito XVIII 1963 / Argentina zone 2",                               0 },	// Hito XVIII 1963 / Argentina zone 2
	{     2084L,  2084L, "Hito63.UTM-19S",             "Hito XVIII 1963 / UTM zone 19S",                                   0 },	// Hito XVIII 1963 / UTM zone 19S
	{     2085L,  2085L, "NAD27.CubaNorte",            "NAD27 / Cuba Norte",                                               0 },	// NAD27 / Cuba Norte
	{     2086L,  2086L, "NAD27.CubaSur",              "NAD27 / Cuba Sur",                                                 0 },	// NAD27 / Cuba Sur
	{     2087L,  2087L, "ELD79.TM-12NE",              "ELD79 / TM 12 NE",                                                 0 },	// ELD79 / TM 12 NE
	{     2088L,  2088L, "Carthage.TM-11NE",           "Carthage / TM 11 NE",                                              0 },	// Carthage / TM 11 NE
	{     2089L,  2089L, "Yemen96.UTM-38N",            "Yemen NGN96 / UTM zone 38N",                                       0 },	// Yemen NGN96 / UTM zone 38N
	{     2090L,  2090L, "Yemen96.UTM-39N",            "Yemen NGN96 / UTM zone 39N",                                       0 },	// Yemen NGN96 / UTM zone 39N
	{     2091L,  2091L, "SouthYemen.GK-8",            "South Yemen / Gauss Kruger zone 8",                                2 },	// South Yemen / Gauss Kruger zone 8
	{     2092L,  2092L, "SouthYemen.GK-9",            "South Yemen / Gauss Kruger zone 9",                                2 },	// South Yemen / Gauss Kruger zone 9
	{     2093L,  2093L, "Hanoi72.GK-106NE",           "Hanoi 1972 / GK 106 NE",                                           0 },	// Hanoi 1972 / GK 106 NE
	{     2094L,  2094L, "WGS72be.TM-106NE",           "WGS 72BE / TM 106 NE",                                             0 },	// WGS 72BE / TM 106 NE
	{     2095L,  2095L, "Bissau.UTM-28N",             "Bissau / UTM zone 28N",                                            0 },	// Bissau / UTM zone 28N
	{     2099L,  2099L, "Qatar48.Qatar Grid",         "Qatar 1948 / Qatar Grid",                                          0 },	// Qatar 1948 / Qatar Grid
	{     2100L,  2100L, "GreekGRS87.GreekGrid",       "GGRS87 / Greek Grid",                                              0 },	// GGRS87 / Greek Grid
	{     2105L,  2105L, "NZGD2K.MountEden",           "NZGD2000 / Mount Eden Circuit 2000",                               0 },	// NZGD2000 / Mount Eden Circuit 2000
	{     2106L,  2106L, "NZGD2K.BayofPlenty",         "NZGD2000 / Bay of Plenty Circuit 2000",                            0 },	// NZGD2000 / Bay of Plenty Circuit 2000
	{     2107L,  2107L, "NZGD2K.PovertyBay",          "NZGD2000 / Poverty Bay Circuit 2000",                              0 },	// NZGD2000 / Poverty Bay Circuit 2000
	{     2108L,  2108L, "NZGD2K.HawkesBay",           "NZGD2000 / Hawkes Bay Circuit 2000",                               0 },	// NZGD2000 / Hawkes Bay Circuit 2000
	{     2109L,  2109L, "NZGD2K.Taranaki",            "NZGD2000 / Taranaki Circuit 2000",                                 0 },	// NZGD2000 / Taranaki Circuit 2000
	{     2110L,  2110L, "NZGD2K.Tuhirangi",           "NZGD2000 / Tuhirangi Circuit 2000",                                0 },	// NZGD2000 / Tuhirangi Circuit 2000
	{     2111L,  2111L, "NZGD2K.Wanganui",            "NZGD2000 / Wanganui Circuit 2000",                                 0 },	// NZGD2000 / Wanganui Circuit 2000
	{     2112L,  2112L, "NZGD2K.Wairarapa",           "NZGD2000 / Wairarapa Circuit 2000",                                0 },	// NZGD2000 / Wairarapa Circuit 2000
	{     2113L,  2113L, "NZGD2K.Wellington",          "NZGD2000 / Wellington Circuit 2000",                               0 },	// NZGD2000 / Wellington Circuit 2000
	{     2114L,  2114L, "NZGD2K.Collingwood",         "NZGD2000 / Collingwood Circuit 2000",                              0 },	// NZGD2000 / Collingwood Circuit 2000
	{     2115L,  2115L, "NZGD2K.Nelson",              "NZGD2000 / Nelson Circuit 2000",                                   0 },	// NZGD2000 / Nelson Circuit 2000
	{     2116L,  2116L, "NZGD2K.Karamea",             "NZGD2000 / Karamea Circuit 2000",                                  0 },	// NZGD2000 / Karamea Circuit 2000
	{     2117L,  2117L, "NZGD2K.Buller",              "NZGD2000 / Buller Circuit 2000",                                   0 },	// NZGD2000 / Buller Circuit 2000
	{     2118L,  2118L, "NZGD2K.Grey",                "NZGD2000 / Grey Circuit 2000",                                     0 },	// NZGD2000 / Grey Circuit 2000
	{     2119L,  2119L, "NZGD2K.Amuri",               "NZGD2000 / Amuri Circuit 2000",                                    0 },	// NZGD2000 / Amuri Circuit 2000
	{     2120L,  2120L, "NZGD2K.Marlborough",         "NZGD2000 / Marlborough Circuit 2000",                              0 },	// NZGD2000 / Marlborough Circuit 2000
	{     2121L,  2121L, "NZGD2K.Hokitika",            "NZGD2000 / Hokitika Circuit 2000",                                 0 },	// NZGD2000 / Hokitika Circuit 2000
	{     2122L,  2122L, "NZGD2K.Okarito",             "NZGD2000 / Okarito Circuit 2000",                                  0 },	// NZGD2000 / Okarito Circuit 2000
	{     2123L,  2123L, "NZGD2K.JacksonsBay",         "NZGD2000 / Jacksons Bay Circuit 2000",                             0 },	// NZGD2000 / Jacksons Bay Circuit 2000
	{     2124L,  2124L, "NZGD2K.MountPleasant",       "NZGD2000 / Mount Pleasant Circuit 2000",                           0 },	// NZGD2000 / Mount Pleasant Circuit 2000
	{     2125L,  2125L, "NZGD2K.Gawler",              "NZGD2000 / Gawler Circuit 2000",                                   0 },	// NZGD2000 / Gawler Circuit 2000
	{     2126L,  2126L, "NZGD2K.Timaru",              "NZGD2000 / Timaru Circuit 2000",                                   0 },	// NZGD2000 / Timaru Circuit 2000
	{     2127L,  2127L, "NZGD2K.LindisPeak",          "NZGD2000 / Lindis Peak Circuit 2000",                              0 },	// NZGD2000 / Lindis Peak Circuit 2000
	{     2128L,  2128L, "NZGD2K.MountNicholas",       "NZGD2000 / Mount Nicholas Circuit 2000",                           0 },	// NZGD2000 / Mount Nicholas Circuit 2000
	{     2129L,  2129L, "NZGD2K.MountYork",           "NZGD2000 / Mount York Circuit 2000",                               0 },	// NZGD2000 / Mount York Circuit 2000
	{     2130L,  2130L, "NZGD2K.ObservationPnt",      "NZGD2000 / Observation Point Circuit 2000",                        0 },	// NZGD2000 / Observation Point Circuit 2000
	{     2131L,  2131L, "NZGD2K.NorthTaieri",         "NZGD2000 / North Taieri Circuit 2000",                             0 },	// NZGD2000 / North Taieri Circuit 2000
	{     2132L,  2132L, "NZGD2K.Bluff",               "NZGD2000 / Bluff Circuit 2000",                                    0 },	// NZGD2000 / Bluff Circuit 2000
	{     2133L,  2133L, "NZGD2K.UTM-58S",             "NZGD2000 / UTM zone 58S",                                          0 },	// NZGD2000 / UTM zone 58S
	{     2134L,  2134L, "NZGD2K.UTM-59S",             "NZGD2000 / UTM zone 59S",                                          0 },	// NZGD2000 / UTM zone 59S
	{     2135L,  2135L, "NZGD2K.UTM-60S",             "NZGD2000 / UTM zone 60S",                                          0 },	// NZGD2000 / UTM zone 60S
	{     2136L,  2136L, "Accra.GhanaNational",        "Accra / Ghana National Grid",                                      0 },	// Accra / Ghana National Grid
	{     2137L,  2137L, "Accra.TM-1NW",               "Accra / TM 1 NW",                                                  0 },	// Accra / TM 1 NW
	{     2139L,  2139L, "NAD83/98.SCoPQ-2",           "NAD83(CSRS98) / SCoPQ zone 2",                                     2 },	// NAD83(CSRS98) / SCoPQ zone 2
	{     2140L,  2140L, "NAD83/98.MTM-3",             "NAD83(CSRS98) / MTM zone 3",                                       2 },	// NAD83(CSRS98) / MTM zone 3
	{     2141L,  2141L, "NAD83/98.MTM-4",             "NAD83(CSRS98) / MTM zone 4",                                       2 },	// NAD83(CSRS98) / MTM zone 4
	{     2142L,  2142L, "NAD83/98.MTM-5",             "NAD83(CSRS98) / MTM zone 5",                                       2 },	// NAD83(CSRS98) / MTM zone 5
	{     2143L,  2143L, "NAD83/98.MTM-6",             "NAD83(CSRS98) / MTM zone 6",                                       2 },	// NAD83(CSRS98) / MTM zone 6
	{     2144L,  2144L, "NAD83/98.MTM-7",             "NAD83(CSRS98) / MTM zone 7",                                       2 },	// NAD83(CSRS98) / MTM zone 7
	{     2145L,  2145L, "NAD83/98.MTM-8",             "NAD83(CSRS98) / MTM zone 8",                                       2 },	// NAD83(CSRS98) / MTM zone 8
	{     2146L,  2146L, "NAD83/98.MTM-9",             "NAD83(CSRS98) / MTM zone 9",                                       2 },	// NAD83(CSRS98) / MTM zone 9
	{     2147L,  2147L, "NAD83/98.MTM-10",            "NAD83(CSRS98) / MTM zone 10",                                      2 },	// NAD83(CSRS98) / MTM zone 10
	{     2148L,  2148L, "NAD83/98.UTM-21N",           "NAD83(CSRS98) / UTM zone 21N",                                     2 },	// NAD83(CSRS98) / UTM zone 21N
	{     2149L,  2149L, "NAD83/98.UTM-18N",           "NAD83(CSRS98) / UTM zone 18N",                                     2 },	// NAD83(CSRS98) / UTM zone 18N
	{     2150L,  2150L, "NAD83/98.UTM-17N",           "NAD83(CSRS98) / UTM zone 17N",                                     2 },	// NAD83(CSRS98) / UTM zone 17N
	{     2151L,  2151L, "NAD83/98.UTM-13N",           "NAD83(CSRS98) / UTM zone 13N",                                     2 },	// NAD83(CSRS98) / UTM zone 13N
	{     2152L,  2152L, "NAD83/98.UTM-12N",           "NAD83(CSRS98) / UTM zone 12N",                                     2 },	// NAD83(CSRS98) / UTM zone 12N
	{     2153L,  2153L, "NAD83/98.UTM-11N",           "NAD83(CSRS98) / UTM zone 11N",                                     2 },	// NAD83(CSRS98) / UTM zone 11N
	{     2154L,  2154L, "IGN-RGF93",                  "RGF93 / Lambert-93",                                               1 },	// RGF93 / Lambert-93
	{     2157L,  2157L, "IRENET95.IrishTM",           "IRENET95 / Irish Transverse Mercator",                             0 },	// IRENET95 / Irish Transverse Mercator
	{     2158L,  2158L, "IRENET95.UTM-29N",           "IRENET95 / UTM zone 29N",                                          0 },	// IRENET95 / UTM zone 29N
	{     2161L,  2161L, "SierraLeone68.UTM-28N",      "Sierra Leone 1968 / UTM zone 28N",                                 0 },	// Sierra Leone 1968 / UTM zone 28N
	{     2162L,  2162L, "SierraLeone68.UTM-29N",      "Sierra Leone 1968 / UTM zone 29N",                                 0 },	// Sierra Leone 1968 / UTM zone 29N
	{     2164L,  2164L, "Locodjo65.TM-5NW",           "Locodjo 1965 / TM 5 NW",                                           0 },	// Locodjo 1965 / TM 5 NW
	{     2165L,  2165L, "Abidjan87.TM-5NW",           "Abidjan 1987 / TM 5 NW",                                           0 },	// Abidjan 1987 / TM 5 NW
	{     2166L,  2166L, "Pulkovo/83.GK-3",            "Pulkovo 1942(83) / Gauss Kruger zone 3",                           2 },	// Pulkovo 1942(83) / Gauss Kruger zone 3
	{     2167L,  2167L, "Pulkovo/83.GK-4",            "Pulkovo 1942(83) / Gauss Kruger zone 4",                           2 },	// Pulkovo 1942(83) / Gauss Kruger zone 4
	{     2168L,  2168L, "Pulkovo/83.GK-5",            "Pulkovo 1942(83) / Gauss Kruger zone 5",                           2 },	// Pulkovo 1942(83) / Gauss Kruger zone 5
	{     2169L,  2169L, "Luxembourg30.Gauss",         "Luxembourg 1930 / Gauss",                                          0 },	// Luxembourg 1930 / Gauss
	{     2170L,  2170L, "MGI.Slovenia",               "MGI / Slovenia Grid",                                              0 },	// MGI / Slovenia Grid
	{     2171L,  2171L, "Pulkovo/58.Poland-I",        "Pulkovo 1942(58) / Poland zone I",                                 2 },	// Pulkovo 1942(58) / Poland zone I
	{     2172L,  2172L, "Pulkovo/58.Poland-II",       "Pulkovo 1942(58) / Poland zone II",                                0 },	// Pulkovo 1942(58) / Poland zone II
	{     2173L,  2173L, "Pulkovo/58.Poland-III",      "Pulkovo 1942(58) / Poland zone III",                               0 },	// Pulkovo 1942(58) / Poland zone III
	{     2174L,  2174L, "Pulkovo/58.Poland-IV",       "Pulkovo 1942(58) / Poland zone IV",                                0 },	// Pulkovo 1942(58) / Poland zone IV
	{     2175L,  2175L, "Pulkovo/58.Poland-V",        "Pulkovo 1942(58) / Poland zone V",                                 0 },	// Pulkovo 1942(58) / Poland zone V
	{     2176L,  2176L, "ETRS89.PolandCS2K-5",        "ETRS89 / Poland CS2000 zone 5",                                    0 },	// ETRS89 / Poland CS2000 zone 5
	{     2177L,  2177L, "ETRS89.PolandCS2K-6",        "ETRS89 / Poland CS2000 zone 6",                                    0 },	// ETRS89 / Poland CS2000 zone 6
	{     2178L,  2178L, "ETRS89.PolandCS2K-7",        "ETRS89 / Poland CS2000 zone 7",                                    0 },	// ETRS89 / Poland CS2000 zone 7
	{     2179L,  2179L, "ETRS89.PolandCS2K-8",        "ETRS89 / Poland CS2000 zone 8",                                    0 },	// ETRS89 / Poland CS2000 zone 8
	{     2180L,  2180L, "ETRS89.PolandCS92",          "ETRS89 / Poland CS92",                                             0 },	// ETRS89 / Poland CS92
	{     2188L,  2188L, "AzoresOccdtl39.UTM-25N",     "Azores Occidental 1939 / UTM zone 25N",                            0 },	// Azores Occidental 1939 / UTM zone 25N
	{     2189L,  2189L, "AzoresCntrl48.UTM-26N",      "Azores Central 1948 / UTM zone 26N",                               0 },	// Azores Central 1948 / UTM zone 26N
	{     2190L,  2190L, "AzoresOrntl40.UTM-26N",      "Azores Oriental 1940 / UTM zone 26N",                              0 },	// Azores Oriental 1940 / UTM zone 26N
	{     2192L,  2192L, "ED50.EuroLambert",           "ED50 / France EuroLambert",                                        0 },	// ED50 / France EuroLambert
	{     2193L,  2193L, "NZTM",                       "NZGD2000 / New Zealand Transverse Mercator",                       1 },	// NZGD2000 / New Zealand Transverse Mercator
	{     2194L,  2194L, "AmSamoa62.Lambert",          "American Samoa 1962 / American Samoa Lambert",                     2 },	// American Samoa 1962 / American Samoa Lambert
	{     2195L,  2195L, "HARN.UTM-2S",                "NAD83(HARN) / UTM zone 2S",                                        0 },	// NAD83(HARN) / UTM zone 2S
	{     2196L,  2196L, "ETRS89.Kp2K-Jutland",        "ETRS89 / Kp2000 Jutland",                                          0 },	// ETRS89 / Kp2000 Jutland
	{     2197L,  2197L, "ETRS89.Kp2K-Zealand",        "ETRS89 / Kp2000 Zealand",                                          0 },	// ETRS89 / Kp2000 Zealand
	{     2198L,  2198L, "ETRS89.Kp2K-Bornholm",       "ETRS89 / Kp2000 Bornholm",                                         0 },	// ETRS89 / Kp2000 Bornholm
	{     2200L,  2200L, "NewBrunswick77",             "ATS77 / New Brunswick Stereographic (ATS77)",                      1 },	// ATS77 / New Brunswick Stereographic (ATS77)
	{     2201L,  2201L, "REGVEN.UTM-18N",             "REGVEN / UTM zone 18N",                                            0 },	// REGVEN / UTM zone 18N
	{     2202L,  2202L, "REGVEN.UTM-19N",             "REGVEN / UTM zone 19N",                                            0 },	// REGVEN / UTM zone 19N
	{     2203L,  2203L, "REGVEN.UTM-20N",             "REGVEN / UTM zone 20N",                                            0 },	// REGVEN / UTM zone 20N
	{    32036L,  2204L, "TN",                         "NAD27 / Tennessee",                                                1 },	// NAD27 / Tennessee
	{     2206L,  2206L, "ED50.GK3d-9",                "ED50 / 3-degree Gauss-Kruger zone 9",                              0 },	// ED50 / 3-degree Gauss-Kruger zone 9
	{     2207L,  2207L, "ED50.GK3d-10",               "ED50 / 3-degree Gauss-Kruger zone 10",                             0 },	// ED50 / 3-degree Gauss-Kruger zone 10
	{     2208L,  2208L, "ED50.GK3d-11",               "ED50 / 3-degree Gauss-Kruger zone 11",                             0 },	// ED50 / 3-degree Gauss-Kruger zone 11
	{     2209L,  2209L, "ED50.GK3d-12",               "ED50 / 3-degree Gauss-Kruger zone 12",                             0 },	// ED50 / 3-degree Gauss-Kruger zone 12
	{     2210L,  2210L, "ED50.GK3d-13",               "ED50 / 3-degree Gauss-Kruger zone 13",                             0 },	// ED50 / 3-degree Gauss-Kruger zone 13
	{     2211L,  2211L, "ED50.GK3d-14",               "ED50 / 3-degree Gauss-Kruger zone 14",                             0 },	// ED50 / 3-degree Gauss-Kruger zone 14
	{     2212L,  2212L, "ED50.GK3d-15",               "ED50 / 3-degree Gauss-Kruger zone 15",                             0 },	// ED50 / 3-degree Gauss-Kruger zone 15
	{     2213L,  2213L, "ETRS89.TM-30NE",             "ETRS89 / TM 30 NE",                                                0 },	// ETRS89 / TM 30 NE
	{     2215L,  2215L, "Manoca62.UTM-32N",           "Manoca 1962 / UTM zone 32N",                                       0 },	// Manoca 1962 / UTM zone 32N
	{     2216L,  2216L, "Qornoq27.UTM-22N",           "Qornoq 1927 / UTM zone 22N",                                       0 },	// Qornoq 1927 / UTM zone 22N
	{     2217L,  2217L, "Qornoq27.UTM-23N",           "Qornoq 1927 / UTM zone 23N",                                       0 },	// Qornoq 1927 / UTM zone 23N
	{     2219L,  2219L, "ATS77.UTM-19N",              "ATS77 / UTM zone 19N",                                             0 },	// ATS77 / UTM zone 19N
	{     2220L,  2220L, "ATS77.UTM-20N",              "ATS77 / UTM zone 20N",                                             0 },	// ATS77 / UTM zone 20N
	{     2222L,  2222L, "AZ83-EIF",                   "NAD83 / Arizona East (ft)",                                        1 },	// NAD83 / Arizona East (ft)
	{     2223L,  2223L, "AZ83-CIF",                   "NAD83 / Arizona Central (ft)",                                     1 },	// NAD83 / Arizona Central (ft)
	{     2224L,  2224L, "AZ83-WIF",                   "NAD83 / Arizona West (ft)",                                        1 },	// NAD83 / Arizona West (ft)
	{     2225L,  2225L, "CA83-IF",                    "NAD83 / California zone 1 (ftUS)",                                 1 },	// NAD83 / California zone 1 (ftUS)
	{     2226L,  2226L, "CA83-IIF",                   "NAD83 / California zone 2 (ftUS)",                                 1 },	// NAD83 / California zone 2 (ftUS)
	{     2227L,  2227L, "CA83IIIF",                   "NAD83 / California zone 3 (ftUS)",                                 1 },	// NAD83 / California zone 3 (ftUS)
	{     2228L,  2228L, "CA83-IVF",                   "NAD83 / California zone 4 (ftUS)",                                 1 },	// NAD83 / California zone 4 (ftUS)
	{     2229L,  2229L, "CA83-VF",                    "NAD83 / California zone 5 (ftUS)",                                 1 },	// NAD83 / California zone 5 (ftUS)
	{     2230L,  2230L, "CA83-VIF",                   "NAD83 / California zone 6 (ftUS)",                                 1 },	// NAD83 / California zone 6 (ftUS)
	{     2231L,  2231L, "CO83-NF",                    "NAD83 / Colorado North (ftUS)",                                    1 },	// NAD83 / Colorado North (ftUS)
	{     2232L,  2232L, "CO83-CF",                    "NAD83 / Colorado Central (ftUS)",                                  1 },	// NAD83 / Colorado Central (ftUS)
	{     2233L,  2233L, "CO83-SF",                    "NAD83 / Colorado South (ftUS)",                                    1 },	// NAD83 / Colorado South (ftUS)
	{     2234L,  2234L, "CT83F",                      "NAD83 / Connecticut (ftUS)",                                       1 },	// NAD83 / Connecticut (ftUS)
	{     2235L,  2235L, "DE83F",                      "NAD83 / Delaware (ftUS)",                                          1 },	// NAD83 / Delaware (ftUS)
	{     2236L,  2236L, "FL83-EF",                    "NAD83 / Florida East (ftUS)",                                      1 },	// NAD83 / Florida East (ftUS)
	{     2237L,  2237L, "FL83-WF",                    "NAD83 / Florida West (ftUS)",                                      1 },	// NAD83 / Florida West (ftUS)
	{     2238L,  2238L, "FL83-NF",                    "NAD83 / Florida North (ftUS)",                                     1 },	// NAD83 / Florida North (ftUS)
	{     2239L,  2239L, "GA83-EF",                    "NAD83 / Georgia East (ftUS)",                                      1 },	// NAD83 / Georgia East (ftUS)
	{     2240L,  2240L, "GA83-WF",                    "NAD83 / Georgia West (ftUS)",                                      1 },	// NAD83 / Georgia West (ftUS)
	{     2241L,  2241L, "ID83-EF",                    "NAD83 / Idaho East (ftUS)",                                        1 },	// NAD83 / Idaho East (ftUS)
	{     2242L,  2242L, "ID83-CF",                    "NAD83 / Idaho Central (ftUS)",                                     1 },	// NAD83 / Idaho Central (ftUS)
	{     2243L,  2243L, "ID83-WF",                    "NAD83 / Idaho West (ftUS)",                                        1 },	// NAD83 / Idaho West (ftUS)
	{     2965L,  2244L, "IN83-EF",                    "NAD83 / Indiana East (ftUS)",                                      3 },	// NAD83 / Indiana East (ftUS)
	{     2966L,  2245L, "IN83-WF",                    "NAD83 / Indiana West (ftUS)",                                      3 },	// NAD83 / Indiana West (ftUS)
	{     2246L,  2246L, "KY83-NF",                    "NAD83 / Kentucky North (ftUS)",                                    1 },	// NAD83 / Kentucky North (ftUS)
	{     2247L,  2247L, "KY83-SF",                    "NAD83 / Kentucky South (ftUS)",                                    1 },	// NAD83 / Kentucky South (ftUS)
	{     2248L,  2248L, "MD83F",                      "NAD83 / Maryland (ftUS)",                                          1 },	// NAD83 / Maryland (ftUS)
	{     2249L,  2249L, "MA83F",                      "NAD83 / Massachusetts Mainland (ftUS)",                            1 },	// NAD83 / Massachusetts Mainland (ftUS)
	{     2250L,  2250L, "MA83-ISF",                   "NAD83 / Massachusetts Island (ftUS)",                              1 },	// NAD83 / Massachusetts Island (ftUS)
	{     2251L,  2251L, "MI83-NIF",                   "NAD83 / Michigan North (ft)",                                      1 },	// NAD83 / Michigan North (ft)
	{     2252L,  2252L, "MI83-CIF",                   "NAD83 / Michigan Central (ft)",                                    1 },	// NAD83 / Michigan Central (ft)
	{     2253L,  2253L, "MI83-SIF",                   "NAD83 / Michigan South (ft)",                                      1 },	// NAD83 / Michigan South (ft)
	{     2254L,  2254L, "MS83-EF",                    "NAD83 / Mississippi East (ftUS)",                                  1 },	// NAD83 / Mississippi East (ftUS)
	{     2255L,  2255L, "MS83-WF",                    "NAD83 / Mississippi West (ftUS)",                                  1 },	// NAD83 / Mississippi West (ftUS)
	{     2256L,  2256L, "MT83IF",                     "NAD83 / Montana (ft)",                                             1 },	// NAD83 / Montana (ft)
	{     2257L,  2257L, "NM83-EF",                    "NAD83 / New Mexico East (ftUS)",                                   1 },	// NAD83 / New Mexico East (ftUS)
	{     2258L,  2258L, "NM83-CF",                    "NAD83 / New Mexico Central (ftUS)",                                1 },	// NAD83 / New Mexico Central (ftUS)
	{     2259L,  2259L, "NM83-WF",                    "NAD83 / New Mexico West (ftUS)",                                   1 },	// NAD83 / New Mexico West (ftUS)
	{     2260L,  2260L, "NY83-EF",                    "NAD83 / New York East (ftUS)",                                     1 },	// NAD83 / New York East (ftUS)
	{     2261L,  2261L, "NY83-CF",                    "NAD83 / New York Central (ftUS)",                                  1 },	// NAD83 / New York Central (ftUS)
	{     2262L,  2262L, "NY83-WF",                    "NAD83 / New York West (ftUS)",                                     1 },	// NAD83 / New York West (ftUS)
	{     2263L,  2263L, "NY83-LIF",                   "NAD83 / New York Long Island (ftUS)",                              1 },	// NAD83 / New York Long Island (ftUS)
	{     2264L,  2264L, "NC83F",                      "NAD83 / North Carolina (ftUS)",                                    1 },	// NAD83 / North Carolina (ftUS)
	{     2265L,  2265L, "NAD83.ND-Nft",               "NAD83 / North Dakota North (ft)",                                  0 },	// NAD83 / North Dakota North (ft)
	{     2266L,  2266L, "NAD83.ND-Sft",               "NAD83 / North Dakota South (ft)",                                  0 },	// NAD83 / North Dakota South (ft)
	{     2267L,  2267L, "OK83-NF",                    "NAD83 / Oklahoma North (ftUS)",                                    1 },	// NAD83 / Oklahoma North (ftUS)
	{     2268L,  2268L, "OK83-SF",                    "NAD83 / Oklahoma South (ftUS)",                                    1 },	// NAD83 / Oklahoma South (ftUS)
	{     2269L,  2269L, "OR83-NIF",                   "NAD83 / Oregon North (ft)",                                        1 },	// NAD83 / Oregon North (ft)
	{     2270L,  2270L, "OR83-SIF",                   "NAD83 / Oregon South (ft)",                                        1 },	// NAD83 / Oregon South (ft)
	{     2271L,  2271L, "PA83-NF",                    "NAD83 / Pennsylvania North (ftUS)",                                1 },	// NAD83 / Pennsylvania North (ftUS)
	{     2272L,  2272L, "PA83-SF",                    "NAD83 / Pennsylvania South (ftUS)",                                1 },	// NAD83 / Pennsylvania South (ftUS)
	{     2273L,  2273L, "SC83IF",                     "NAD83 / South Carolina (ft)",                                      1 },	// NAD83 / South Carolina (ft)
	{     2274L,  2274L, "TN83F",                      "NAD83 / Tennessee (ftUS)",                                         1 },	// NAD83 / Tennessee (ftUS)
	{     2275L,  2275L, "TX83-NF",                    "NAD83 / Texas North (ftUS)",                                       1 },	// NAD83 / Texas North (ftUS)
	{     2276L,  2276L, "TX83-NCF",                   "NAD83 / Texas North Central (ftUS)",                               1 },	// NAD83 / Texas North Central (ftUS)
	{     2277L,  2277L, "TX83-CF",                    "NAD83 / Texas Central (ftUS)",                                     1 },	// NAD83 / Texas Central (ftUS)
	{     2278L,  2278L, "TX83-SCF",                   "NAD83 / Texas South Central (ftUS)",                               1 },	// NAD83 / Texas South Central (ftUS)
	{     2279L,  2279L, "TX83-SF",                    "NAD83 / Texas South (ftUS)",                                       1 },	// NAD83 / Texas South (ftUS)
	{     2280L,  2280L, "UT83-NIF",                   "NAD83 / Utah North (ft)",                                          1 },	// NAD83 / Utah North (ft)
	{     2281L,  2281L, "UT83-CIF",                   "NAD83 / Utah Central (ft)",                                        1 },	// NAD83 / Utah Central (ft)
	{     2282L,  2282L, "UT83-SIF",                   "NAD83 / Utah South (ft)",                                          1 },	// NAD83 / Utah South (ft)
	{     2283L,  2283L, "VA83-NF",                    "NAD83 / Virginia North (ftUS)",                                    1 },	// NAD83 / Virginia North (ftUS)
	{     2284L,  2284L, "VA83-SF",                    "NAD83 / Virginia South (ftUS)",                                    1 },	// NAD83 / Virginia South (ftUS)
	{     2285L,  2285L, "WA83-NF",                    "NAD83 / Washington North (ftUS)",                                  1 },	// NAD83 / Washington North (ftUS)
	{     2286L,  2286L, "WA83-SF",                    "NAD83 / Washington South (ftUS)",                                  1 },	// NAD83 / Washington South (ftUS)
	{     2287L,  2287L, "WI83-NF",                    "NAD83 / Wisconsin North (ftUS)",                                   1 },	// NAD83 / Wisconsin North (ftUS)
	{     2288L,  2288L, "WI83-CF",                    "NAD83 / Wisconsin Central (ftUS)",                                 1 },	// NAD83 / Wisconsin Central (ftUS)
	{     2289L,  2289L, "WI83-SF",                    "NAD83 / Wisconsin South (ftUS)",                                   1 },	// NAD83 / Wisconsin South (ftUS)
	{     2290L,  2290L, "PrinceEdwardIsland77",       "ATS77 / Prince Edward Isl. Stereographic (ATS77)",                 1 },	// ATS77 / Prince Edward Isl. Stereographic (ATS77)
	{     2294L,  2294L, "NovaScotia77-4",             "ATS77 / MTM Nova Scotia zone 4",                                   1 },	// ATS77 / MTM Nova Scotia zone 4
	{     2295L,  2295L, "NovaScotia77-5",             "ATS77 / MTM Nova Scotia zone 5",                                   1 },	// ATS77 / MTM Nova Scotia zone 5
	{     2308L,  2308L, "Batavia.TM-109SE",           "Batavia / TM 109 SE",                                              0 },	// Batavia / TM 109 SE
	{     2309L,  2309L, "WGS84.TM-116SE",             "WGS 84 / TM 116 SE",                                               0 },	// WGS 84 / TM 116 SE
	{     2310L,  2310L, "WGS84.TM-132SE",             "WGS 84 / TM 132 SE",                                               0 },	// WGS 84 / TM 132 SE
	{     2311L,  2311L, "WGS84.TM-6NE",               "WGS 84 / TM 6 NE",                                                 0 },	// WGS 84 / TM 6 NE
	{     2314L,  2314L, "Trinidad03.TrinidadGrid",    "Trinidad 1903 / Trinidad Grid (ftCla)",                            0 },	// Trinidad 1903 / Trinidad Grid (ftCla)
	{     2315L,  2315L, "CampoInchauspe.UTM-19S",     "Campo Inchauspe / UTM zone 19S",                                   0 },	// Campo Inchauspe / UTM zone 19S
	{     2316L,  2316L, "CampoInchauspe.UTM-20S",     "Campo Inchauspe / UTM zone 20S",                                   0 },	// Campo Inchauspe / UTM zone 20S
	{     2317L,  2317L, "PSAD56.IcnRegional",         "PSAD56 / ICN Regional",                                            0 },	// PSAD56 / ICN Regional
	{     2318L,  2318L, "AINELABD.AramcoLambert",     "Ain el Abd / Aramco Lambert",                                      0 },	// Ain el Abd / Aramco Lambert
	{     2319L,  2319L, "ED50.TM27",                  "ED50 / TM27",                                                      0 },	// ED50 / TM27
	{     2320L,  2320L, "ED50.TM30",                  "ED50 / TM30",                                                      0 },	// ED50 / TM30
	{     2321L,  2321L, "ED50.TM33",                  "ED50 / TM33",                                                      0 },	// ED50 / TM33
	{     2322L,  2322L, "ED50.TM36",                  "ED50 / TM36",                                                      0 },	// ED50 / TM36
	{     2323L,  2323L, "ED50.TM39",                  "ED50 / TM39",                                                      0 },	// ED50 / TM39
	{     2324L,  2324L, "ED50.TM42",                  "ED50 / TM42",                                                      0 },	// ED50 / TM42
	{     2325L,  2325L, "ED50.TM45",                  "ED50 / TM45",                                                      0 },	// ED50 / TM45
	{     2326L,  2326L, "HongKong80.GridSystem",      "Hong Kong 1980 Grid System",                                       0 },	// Hong Kong 1980 Grid System
	{     2327L,  2327L, "Xian80.GK-13",               "Xian 1980 / Gauss-Kruger zone 13",                                 0 },	// Xian 1980 / Gauss-Kruger zone 13
	{     2328L,  2328L, "Xian80.GK-14",               "Xian 1980 / Gauss-Kruger zone 14",                                 0 },	// Xian 1980 / Gauss-Kruger zone 14
	{     2329L,  2329L, "Xian80.GK-15",               "Xian 1980 / Gauss-Kruger zone 15",                                 0 },	// Xian 1980 / Gauss-Kruger zone 15
	{     2330L,  2330L, "Xian80.GK-16",               "Xian 1980 / Gauss-Kruger zone 16",                                 0 },	// Xian 1980 / Gauss-Kruger zone 16
	{     2331L,  2331L, "Xian80.GK-17",               "Xian 1980 / Gauss-Kruger zone 17",                                 0 },	// Xian 1980 / Gauss-Kruger zone 17
	{     2332L,  2332L, "Xian80.GK-18",               "Xian 1980 / Gauss-Kruger zone 18",                                 0 },	// Xian 1980 / Gauss-Kruger zone 18
	{     2333L,  2333L, "Xian80.GK-19",               "Xian 1980 / Gauss-Kruger zone 19",                                 0 },	// Xian 1980 / Gauss-Kruger zone 19
	{     2334L,  2334L, "Xian80.GK-20",               "Xian 1980 / Gauss-Kruger zone 20",                                 0 },	// Xian 1980 / Gauss-Kruger zone 20
	{     2335L,  2335L, "Xian80.GK-21",               "Xian 1980 / Gauss-Kruger zone 21",                                 0 },	// Xian 1980 / Gauss-Kruger zone 21
	{     2336L,  2336L, "Xian80.GK-22",               "Xian 1980 / Gauss-Kruger zone 22",                                 0 },	// Xian 1980 / Gauss-Kruger zone 22
	{     2337L,  2337L, "Xian80.GK-23",               "Xian 1980 / Gauss-Kruger zone 23",                                 0 },	// Xian 1980 / Gauss-Kruger zone 23
	{     2338L,  2338L, "Xian80.GK/CM-75E",           "Xian 1980 / Gauss-Kruger CM 75E",                                  0 },	// Xian 1980 / Gauss-Kruger CM 75E
	{     2339L,  2339L, "Xian80.GK/CM-81E",           "Xian 1980 / Gauss-Kruger CM 81E",                                  0 },	// Xian 1980 / Gauss-Kruger CM 81E
	{     2340L,  2340L, "Xian80.GK/CM-87E",           "Xian 1980 / Gauss-Kruger CM 87E",                                  0 },	// Xian 1980 / Gauss-Kruger CM 87E
	{     2341L,  2341L, "Xian80.GK/CM-93E",           "Xian 1980 / Gauss-Kruger CM 93E",                                  0 },	// Xian 1980 / Gauss-Kruger CM 93E
	{     2342L,  2342L, "Xian80.GK/CM-99E",           "Xian 1980 / Gauss-Kruger CM 99E",                                  0 },	// Xian 1980 / Gauss-Kruger CM 99E
	{     2343L,  2343L, "Xian80.GK/CM-105E",          "Xian 1980 / Gauss-Kruger CM 105E",                                 0 },	// Xian 1980 / Gauss-Kruger CM 105E
	{     2344L,  2344L, "Xian80.GK/CM-111E",          "Xian 1980 / Gauss-Kruger CM 111E",                                 0 },	// Xian 1980 / Gauss-Kruger CM 111E
	{     2345L,  2345L, "Xian80.GK/CM-117E",          "Xian 1980 / Gauss-Kruger CM 117E",                                 0 },	// Xian 1980 / Gauss-Kruger CM 117E
	{     2346L,  2346L, "Xian80.GK/CM-123E",          "Xian 1980 / Gauss-Kruger CM 123E",                                 0 },	// Xian 1980 / Gauss-Kruger CM 123E
	{     2347L,  2347L, "Xian80.GK/CM-129E",          "Xian 1980 / Gauss-Kruger CM 129E",                                 0 },	// Xian 1980 / Gauss-Kruger CM 129E
	{     2348L,  2348L, "Xian80.GK/CM-135E",          "Xian 1980 / Gauss-Kruger CM 135E",                                 0 },	// Xian 1980 / Gauss-Kruger CM 135E
	{     2349L,  2349L, "Xian80.GK3d-25",             "Xian 1980 / 3-degree Gauss-Kruger zone 25",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger zone 25
	{     2350L,  2350L, "Xian80.GK3d-26",             "Xian 1980 / 3-degree Gauss-Kruger zone 26",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger zone 26
	{     2351L,  2351L, "Xian80.GK3d-27",             "Xian 1980 / 3-degree Gauss-Kruger zone 27",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger zone 27
	{     2352L,  2352L, "Xian80.GK3d-28",             "Xian 1980 / 3-degree Gauss-Kruger zone 28",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger zone 28
	{     2353L,  2353L, "Xian80.GK3d-29",             "Xian 1980 / 3-degree Gauss-Kruger zone 29",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger zone 29
	{     2354L,  2354L, "Xian80.GK3d-30",             "Xian 1980 / 3-degree Gauss-Kruger zone 30",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger zone 30
	{     2355L,  2355L, "Xian80.GK3d-31",             "Xian 1980 / 3-degree Gauss-Kruger zone 31",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger zone 31
	{     2356L,  2356L, "Xian80.GK3d-32",             "Xian 1980 / 3-degree Gauss-Kruger zone 32",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger zone 32
	{     2357L,  2357L, "Xian80.GK3d-33",             "Xian 1980 / 3-degree Gauss-Kruger zone 33",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger zone 33
	{     2358L,  2358L, "Xian80.GK3d-34",             "Xian 1980 / 3-degree Gauss-Kruger zone 34",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger zone 34
	{     2359L,  2359L, "Xian80.GK3d-35",             "Xian 1980 / 3-degree Gauss-Kruger zone 35",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger zone 35
	{     2360L,  2360L, "Xian80.GK3d-36",             "Xian 1980 / 3-degree Gauss-Kruger zone 36",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger zone 36
	{     2361L,  2361L, "Xian80.GK3d-37",             "Xian 1980 / 3-degree Gauss-Kruger zone 37",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger zone 37
	{     2362L,  2362L, "Xian80.GK3d-38",             "Xian 1980 / 3-degree Gauss-Kruger zone 38",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger zone 38
	{     2363L,  2363L, "Xian80.GK3d-39",             "Xian 1980 / 3-degree Gauss-Kruger zone 39",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger zone 39
	{     2364L,  2364L, "Xian80.GK3d-40",             "Xian 1980 / 3-degree Gauss-Kruger zone 40",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger zone 40
	{     2365L,  2365L, "Xian80.GK3d-41",             "Xian 1980 / 3-degree Gauss-Kruger zone 41",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger zone 41
	{     2366L,  2366L, "Xian80.GK3d-42",             "Xian 1980 / 3-degree Gauss-Kruger zone 42",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger zone 42
	{     2367L,  2367L, "Xian80.GK3d-43",             "Xian 1980 / 3-degree Gauss-Kruger zone 43",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger zone 43
	{     2368L,  2368L, "Xian80.GK3d-44",             "Xian 1980 / 3-degree Gauss-Kruger zone 44",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger zone 44
	{     2369L,  2369L, "Xian80.GK3d-45",             "Xian 1980 / 3-degree Gauss-Kruger zone 45",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger zone 45
	{     2370L,  2370L, "Xian80.GK3d/CM-75E",         "Xian 1980 / 3-degree Gauss-Kruger CM 75E",                         0 },	// Xian 1980 / 3-degree Gauss-Kruger CM 75E
	{     2371L,  2371L, "Xian80.GK3d/CM-78E",         "Xian 1980 / 3-degree Gauss-Kruger CM 78E",                         0 },	// Xian 1980 / 3-degree Gauss-Kruger CM 78E
	{     2372L,  2372L, "Xian80.GK3d/CM-81E",         "Xian 1980 / 3-degree Gauss-Kruger CM 81E",                         0 },	// Xian 1980 / 3-degree Gauss-Kruger CM 81E
	{     2373L,  2373L, "Xian80.GK3d/CM-84E",         "Xian 1980 / 3-degree Gauss-Kruger CM 84E",                         0 },	// Xian 1980 / 3-degree Gauss-Kruger CM 84E
	{     2374L,  2374L, "Xian80.GK3d/CM-87E",         "Xian 1980 / 3-degree Gauss-Kruger CM 87E",                         0 },	// Xian 1980 / 3-degree Gauss-Kruger CM 87E
	{     2375L,  2375L, "Xian80.GK3d/CM-90E",         "Xian 1980 / 3-degree Gauss-Kruger CM 90E",                         0 },	// Xian 1980 / 3-degree Gauss-Kruger CM 90E
	{     2376L,  2376L, "Xian80.GK3d/CM-93E",         "Xian 1980 / 3-degree Gauss-Kruger CM 93E",                         0 },	// Xian 1980 / 3-degree Gauss-Kruger CM 93E
	{     2377L,  2377L, "Xian80.GK3d/CM-96E",         "Xian 1980 / 3-degree Gauss-Kruger CM 96E",                         0 },	// Xian 1980 / 3-degree Gauss-Kruger CM 96E
	{     2378L,  2378L, "Xian80.GK3d/CM-99E",         "Xian 1980 / 3-degree Gauss-Kruger CM 99E",                         0 },	// Xian 1980 / 3-degree Gauss-Kruger CM 99E
	{     2379L,  2379L, "Xian80.GK3d/CM-102E",        "Xian 1980 / 3-degree Gauss-Kruger CM 102E",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger CM 102E
	{     2380L,  2380L, "Xian80.GK3d/CM-105E",        "Xian 1980 / 3-degree Gauss-Kruger CM 105E",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger CM 105E
	{     2381L,  2381L, "Xian80.GK3d/CM-108E",        "Xian 1980 / 3-degree Gauss-Kruger CM 108E",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger CM 108E
	{     2382L,  2382L, "Xian80.GK3d/CM-111E",        "Xian 1980 / 3-degree Gauss-Kruger CM 111E",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger CM 111E
	{     2383L,  2383L, "Xian80.GK3d/CM-114E",        "Xian 1980 / 3-degree Gauss-Kruger CM 114E",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger CM 114E
	{     2384L,  2384L, "Xian80.GK3d/CM-117E",        "Xian 1980 / 3-degree Gauss-Kruger CM 117E",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger CM 117E
	{     2385L,  2385L, "Xian80.GK3d/CM-120E",        "Xian 1980 / 3-degree Gauss-Kruger CM 120E",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger CM 120E
	{     2386L,  2386L, "Xian80.GK3d/CM-123E",        "Xian 1980 / 3-degree Gauss-Kruger CM 123E",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger CM 123E
	{     2387L,  2387L, "Xian80.GK3d/CM-126E",        "Xian 1980 / 3-degree Gauss-Kruger CM 126E",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger CM 126E
	{     2388L,  2388L, "Xian80.GK3d/CM-129E",        "Xian 1980 / 3-degree Gauss-Kruger CM 129E",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger CM 129E
	{     2389L,  2389L, "Xian80.GK3d/CM-132E",        "Xian 1980 / 3-degree Gauss-Kruger CM 132E",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger CM 132E
	{     2390L,  2390L, "Xian80.GK3d/CM-135E",        "Xian 1980 / 3-degree Gauss-Kruger CM 135E",                        0 },	// Xian 1980 / 3-degree Gauss-Kruger CM 135E
	{     2391L,  2391L, "KKJ.Finland-1",              "KKJ / Finland zone 1",                                             0 },	// KKJ / Finland zone 1
	{     2392L,  2392L, "KKJ.Finland-2",              "KKJ / Finland zone 2",                                             0 },	// KKJ / Finland zone 2
	{     2393L,  2393L, "KKJ.Finland-UCS",            "KKJ / Finland Uniform Coordinate System",                          0 },	// KKJ / Finland Uniform Coordinate System
	{     2394L,  2394L, "KKJ.Finland-4",              "KKJ / Finland zone 4",                                             0 },	// KKJ / Finland zone 4
	{     2395L,  2395L, "SouthYemen.GK-8",            "South Yemen / Gauss-Kruger zone 8",                                0 },	// South Yemen / Gauss-Kruger zone 8
	{     2396L,  2396L, "SouthYemen.GK-9",            "South Yemen / Gauss-Kruger zone 9",                                0 },	// South Yemen / Gauss-Kruger zone 9
	{     2397L,  2397L, "Pulkovo/83.GK-3",            "Pulkovo 1942(83) / Gauss-Kruger zone 3",                           0 },	// Pulkovo 1942(83) / Gauss-Kruger zone 3
	{     2398L,  2398L, "Pulkovo/83.GK-4",            "Pulkovo 1942(83) / Gauss-Kruger zone 4",                           0 },	// Pulkovo 1942(83) / Gauss-Kruger zone 4
	{     2399L,  2399L, "Pulkovo/83.GK-5",            "Pulkovo 1942(83) / Gauss-Kruger zone 5",                           0 },	// Pulkovo 1942(83) / Gauss-Kruger zone 5
	{     3021L,  2400L, "RT90-2.5V",                  "RT90 2.5 gon V",                                                   3 },	// RT90 2.5 gon W
	{     2401L,  2401L, "Beijing54.GK3d-25",          "Beijing 1954 / 3-degree Gauss-Kruger zone 25",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger zone 25
	{     2402L,  2402L, "Beijing54.GK3d-26",          "Beijing 1954 / 3-degree Gauss-Kruger zone 26",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger zone 26
	{     2403L,  2403L, "Beijing54.GK3d-27",          "Beijing 1954 / 3-degree Gauss-Kruger zone 27",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger zone 27
	{     2404L,  2404L, "Beijing54.GK3d-28",          "Beijing 1954 / 3-degree Gauss-Kruger zone 28",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger zone 28
	{     2405L,  2405L, "Beijing54.GK3d-29",          "Beijing 1954 / 3-degree Gauss-Kruger zone 29",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger zone 29
	{     2406L,  2406L, "Beijing54.GK3d-30",          "Beijing 1954 / 3-degree Gauss-Kruger zone 30",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger zone 30
	{     2407L,  2407L, "Beijing54.GK3d-31",          "Beijing 1954 / 3-degree Gauss-Kruger zone 31",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger zone 31
	{     2408L,  2408L, "Beijing54.GK3d-32",          "Beijing 1954 / 3-degree Gauss-Kruger zone 32",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger zone 32
	{     2409L,  2409L, "Beijing54.GK3d-33",          "Beijing 1954 / 3-degree Gauss-Kruger zone 33",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger zone 33
	{     2410L,  2410L, "Beijing54.GK3d-34",          "Beijing 1954 / 3-degree Gauss-Kruger zone 34",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger zone 34
	{     2411L,  2411L, "Beijing54.GK3d-35",          "Beijing 1954 / 3-degree Gauss-Kruger zone 35",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger zone 35
	{     2412L,  2412L, "Beijing54.GK3d-36",          "Beijing 1954 / 3-degree Gauss-Kruger zone 36",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger zone 36
	{     2413L,  2413L, "Beijing54.GK3d-37",          "Beijing 1954 / 3-degree Gauss-Kruger zone 37",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger zone 37
	{     2414L,  2414L, "Beijing54.GK3d-38",          "Beijing 1954 / 3-degree Gauss-Kruger zone 38",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger zone 38
	{     2415L,  2415L, "Beijing54.GK3d-39",          "Beijing 1954 / 3-degree Gauss-Kruger zone 39",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger zone 39
	{     2416L,  2416L, "Beijing54.GK3d-40",          "Beijing 1954 / 3-degree Gauss-Kruger zone 40",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger zone 40
	{     2417L,  2417L, "Beijing54.GK3d-41",          "Beijing 1954 / 3-degree Gauss-Kruger zone 41",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger zone 41
	{     2418L,  2418L, "Beijing54.GK3d-42",          "Beijing 1954 / 3-degree Gauss-Kruger zone 42",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger zone 42
	{     2419L,  2419L, "Beijing54.GK3d-43",          "Beijing 1954 / 3-degree Gauss-Kruger zone 43",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger zone 43
	{     2420L,  2420L, "Beijing54.GK3d-44",          "Beijing 1954 / 3-degree Gauss-Kruger zone 44",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger zone 44
	{     2421L,  2421L, "Beijing54.GK3d-45",          "Beijing 1954 / 3-degree Gauss-Kruger zone 45",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger zone 45
	{     2422L,  2422L, "Beijing54.GK3d/CM-75E",      "Beijing 1954 / 3-degree Gauss-Kruger CM 75E",                      0 },	// Beijing 1954 / 3-degree Gauss-Kruger CM 75E
	{     2423L,  2423L, "Beijing54.GK3d/CM-78E",      "Beijing 1954 / 3-degree Gauss-Kruger CM 78E",                      0 },	// Beijing 1954 / 3-degree Gauss-Kruger CM 78E
	{     2424L,  2424L, "Beijing54.GK3d/CM-81E",      "Beijing 1954 / 3-degree Gauss-Kruger CM 81E",                      0 },	// Beijing 1954 / 3-degree Gauss-Kruger CM 81E
	{     2425L,  2425L, "Beijing54.GK3d/CM-84E",      "Beijing 1954 / 3-degree Gauss-Kruger CM 84E",                      0 },	// Beijing 1954 / 3-degree Gauss-Kruger CM 84E
	{     2426L,  2426L, "Beijing54.GK3d/CM-87E",      "Beijing 1954 / 3-degree Gauss-Kruger CM 87E",                      0 },	// Beijing 1954 / 3-degree Gauss-Kruger CM 87E
	{     2427L,  2427L, "Beijing54.GK3d/CM-90E",      "Beijing 1954 / 3-degree Gauss-Kruger CM 90E",                      0 },	// Beijing 1954 / 3-degree Gauss-Kruger CM 90E
	{     2428L,  2428L, "Beijing54.GK3d/CM-93E",      "Beijing 1954 / 3-degree Gauss-Kruger CM 93E",                      0 },	// Beijing 1954 / 3-degree Gauss-Kruger CM 93E
	{     2429L,  2429L, "Beijing54.GK3d/CM-96E",      "Beijing 1954 / 3-degree Gauss-Kruger CM 96E",                      0 },	// Beijing 1954 / 3-degree Gauss-Kruger CM 96E
	{     2430L,  2430L, "Beijing54.GK3d/CM-99E",      "Beijing 1954 / 3-degree Gauss-Kruger CM 99E",                      0 },	// Beijing 1954 / 3-degree Gauss-Kruger CM 99E
	{     2431L,  2431L, "Beijing54.GK3d/CM-102E",     "Beijing 1954 / 3-degree Gauss-Kruger CM 102E",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger CM 102E
	{     2432L,  2432L, "Beijing54.GK3d/CM-105E",     "Beijing 1954 / 3-degree Gauss-Kruger CM 105E",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger CM 105E
	{     2433L,  2433L, "Beijing54.GK3d/CM-108E",     "Beijing 1954 / 3-degree Gauss-Kruger CM 108E",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger CM 108E
	{     2434L,  2434L, "Beijing54.GK3d/CM-111E",     "Beijing 1954 / 3-degree Gauss-Kruger CM 111E",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger CM 111E
	{     2435L,  2435L, "Beijing54.GK3d/CM-114E",     "Beijing 1954 / 3-degree Gauss-Kruger CM 114E",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger CM 114E
	{     2436L,  2436L, "Beijing54.GK3d/CM-117E",     "Beijing 1954 / 3-degree Gauss-Kruger CM 117E",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger CM 117E
	{     2437L,  2437L, "Beijing54.GK3d/CM-120E",     "Beijing 1954 / 3-degree Gauss-Kruger CM 120E",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger CM 120E
	{     2438L,  2438L, "Beijing54.GK3d/CM-123E",     "Beijing 1954 / 3-degree Gauss-Kruger CM 123E",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger CM 123E
	{     2439L,  2439L, "Beijing54.GK3d/CM-126E",     "Beijing 1954 / 3-degree Gauss-Kruger CM 126E",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger CM 126E
	{     2440L,  2440L, "Beijing54.GK3d/CM-129E",     "Beijing 1954 / 3-degree Gauss-Kruger CM 129E",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger CM 129E
	{     2441L,  2441L, "Beijing54.GK3d/CM-132E",     "Beijing 1954 / 3-degree Gauss-Kruger CM 132E",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger CM 132E
	{     2442L,  2442L, "Beijing54.GK3d/CM-135E",     "Beijing 1954 / 3-degree Gauss-Kruger CM 135E",                     0 },	// Beijing 1954 / 3-degree Gauss-Kruger CM 135E
	{     2443L,  2443L, "JGD2K.CS-I",                 "JGD2000 / Japan Plane Rectangular CS I",                           0 },	// JGD2000 / Japan Plane Rectangular CS I
	{     2444L,  2444L, "JGD2K.CS-II",                "JGD2000 / Japan Plane Rectangular CS II",                          0 },	// JGD2000 / Japan Plane Rectangular CS II
	{     2445L,  2445L, "JGD2K.CS-III",               "JGD2000 / Japan Plane Rectangular CS III",                         0 },	// JGD2000 / Japan Plane Rectangular CS III
	{     2446L,  2446L, "JGD2K.CS-IV",                "JGD2000 / Japan Plane Rectangular CS IV",                          0 },	// JGD2000 / Japan Plane Rectangular CS IV
	{     2447L,  2447L, "JGD2K.CS-V",                 "JGD2000 / Japan Plane Rectangular CS V",                           0 },	// JGD2000 / Japan Plane Rectangular CS V
	{     2448L,  2448L, "JGD2K.CS-VI",                "JGD2000 / Japan Plane Rectangular CS VI",                          0 },	// JGD2000 / Japan Plane Rectangular CS VI
	{     2449L,  2449L, "JGD2K.CS-VII",               "JGD2000 / Japan Plane Rectangular CS VII",                         0 },	// JGD2000 / Japan Plane Rectangular CS VII
	{     2450L,  2450L, "JGD2K.CS-VIII",              "JGD2000 / Japan Plane Rectangular CS VIII",                        0 },	// JGD2000 / Japan Plane Rectangular CS VIII
	{     2451L,  2451L, "JGD2K.CS-IX",                "JGD2000 / Japan Plane Rectangular CS IX",                          0 },	// JGD2000 / Japan Plane Rectangular CS IX
	{     2452L,  2452L, "JGD2K.CS-X",                 "JGD2000 / Japan Plane Rectangular CS X",                           0 },	// JGD2000 / Japan Plane Rectangular CS X
	{     2453L,  2453L, "JGD2K.CS-XI",                "JGD2000 / Japan Plane Rectangular CS XI",                          0 },	// JGD2000 / Japan Plane Rectangular CS XI
	{     2454L,  2454L, "JGD2K.CS-XII",               "JGD2000 / Japan Plane Rectangular CS XII",                         0 },	// JGD2000 / Japan Plane Rectangular CS XII
	{     2455L,  2455L, "JGD2K.CS-XIII",              "JGD2000 / Japan Plane Rectangular CS XIII",                        0 },	// JGD2000 / Japan Plane Rectangular CS XIII
	{     2456L,  2456L, "JGD2K.CS-XIV",               "JGD2000 / Japan Plane Rectangular CS XIV",                         0 },	// JGD2000 / Japan Plane Rectangular CS XIV
	{     2457L,  2457L, "JGD2K.CS-XV",                "JGD2000 / Japan Plane Rectangular CS XV",                          0 },	// JGD2000 / Japan Plane Rectangular CS XV
	{     2458L,  2458L, "JGD2K.CS-XVI",               "JGD2000 / Japan Plane Rectangular CS XVI",                         0 },	// JGD2000 / Japan Plane Rectangular CS XVI
	{     2459L,  2459L, "JGD2K.CS-XVII",              "JGD2000 / Japan Plane Rectangular CS XVII",                        0 },	// JGD2000 / Japan Plane Rectangular CS XVII
	{     2460L,  2460L, "JGD2K.CS-XVIII",             "JGD2000 / Japan Plane Rectangular CS XVIII",                       0 },	// JGD2000 / Japan Plane Rectangular CS XVIII
	{     2461L,  2461L, "JGD2K.CS-XIX",               "JGD2000 / Japan Plane Rectangular CS XIX",                         0 },	// JGD2000 / Japan Plane Rectangular CS XIX
	{     2492L,  2492L, "Pulkovo42.GK/CM-9E",         "Pulkovo 1942 / Gauss-Kruger CM 9E",                                0 },	// Pulkovo 1942 / Gauss-Kruger CM 9E
	{     2493L,  2493L, "Pulkovo42.GK/CM-15E",        "Pulkovo 1942 / Gauss-Kruger CM 15E",                               0 },	// Pulkovo 1942 / Gauss-Kruger CM 15E
	{     2494L,  2494L, "Pulkovo42.GK/CM-21E",        "Pulkovo 1942 / Gauss-Kruger CM 21E",                               0 },	// Pulkovo 1942 / Gauss-Kruger CM 21E
	{     2495L,  2495L, "Pulkovo42.GK/CM-27E",        "Pulkovo 1942 / Gauss-Kruger CM 27E",                               0 },	// Pulkovo 1942 / Gauss-Kruger CM 27E
	{     2496L,  2496L, "Pulkovo42.GK/CM-33E",        "Pulkovo 1942 / Gauss-Kruger CM 33E",                               0 },	// Pulkovo 1942 / Gauss-Kruger CM 33E
	{     2497L,  2497L, "Pulkovo42.GK/CM-39E",        "Pulkovo 1942 / Gauss-Kruger CM 39E",                               0 },	// Pulkovo 1942 / Gauss-Kruger CM 39E
	{     2498L,  2498L, "Pulkovo42.GK/CM-45E",        "Pulkovo 1942 / Gauss-Kruger CM 45E",                               0 },	// Pulkovo 1942 / Gauss-Kruger CM 45E
	{     2499L,  2499L, "Pulkovo42.GK/CM-51E",        "Pulkovo 1942 / Gauss-Kruger CM 51E",                               0 },	// Pulkovo 1942 / Gauss-Kruger CM 51E
	{     2500L,  2500L, "Pulkovo42.GK/CM-57E",        "Pulkovo 1942 / Gauss-Kruger CM 57E",                               0 },	// Pulkovo 1942 / Gauss-Kruger CM 57E
	{     2501L,  2501L, "Pulkovo42.GK/CM-63E",        "Pulkovo 1942 / Gauss-Kruger CM 63E",                               0 },	// Pulkovo 1942 / Gauss-Kruger CM 63E
	{     2502L,  2502L, "Pulkovo42.GK/CM-69E",        "Pulkovo 1942 / Gauss-Kruger CM 69E",                               0 },	// Pulkovo 1942 / Gauss-Kruger CM 69E
	{     2503L,  2503L, "Pulkovo42.GK/CM-75E",        "Pulkovo 1942 / Gauss-Kruger CM 75E",                               0 },	// Pulkovo 1942 / Gauss-Kruger CM 75E
	{     2504L,  2504L, "Pulkovo42.GK/CM-81E",        "Pulkovo 1942 / Gauss-Kruger CM 81E",                               0 },	// Pulkovo 1942 / Gauss-Kruger CM 81E
	{     2505L,  2505L, "Pulkovo42.GK/CM-87E",        "Pulkovo 1942 / Gauss-Kruger CM 87E",                               0 },	// Pulkovo 1942 / Gauss-Kruger CM 87E
	{     2506L,  2506L, "Pulkovo42.GK/CM-93E",        "Pulkovo 1942 / Gauss-Kruger CM 93E",                               0 },	// Pulkovo 1942 / Gauss-Kruger CM 93E
	{     2507L,  2507L, "Pulkovo42.GK/CM-99E",        "Pulkovo 1942 / Gauss-Kruger CM 99E",                               0 },	// Pulkovo 1942 / Gauss-Kruger CM 99E
	{     2508L,  2508L, "Pulkovo42.GK/CM-105E",       "Pulkovo 1942 / Gauss-Kruger CM 105E",                              0 },	// Pulkovo 1942 / Gauss-Kruger CM 105E
	{     2509L,  2509L, "Pulkovo42.GK/CM-111E",       "Pulkovo 1942 / Gauss-Kruger CM 111E",                              0 },	// Pulkovo 1942 / Gauss-Kruger CM 111E
	{     2510L,  2510L, "Pulkovo42.GK/CM-117E",       "Pulkovo 1942 / Gauss-Kruger CM 117E",                              0 },	// Pulkovo 1942 / Gauss-Kruger CM 117E
	{     2511L,  2511L, "Pulkovo42.GK/CM-123E",       "Pulkovo 1942 / Gauss-Kruger CM 123E",                              0 },	// Pulkovo 1942 / Gauss-Kruger CM 123E
	{     2512L,  2512L, "Pulkovo42.GK/CM-129E",       "Pulkovo 1942 / Gauss-Kruger CM 129E",                              0 },	// Pulkovo 1942 / Gauss-Kruger CM 129E
	{     2513L,  2513L, "Pulkovo42.GK/CM-135E",       "Pulkovo 1942 / Gauss-Kruger CM 135E",                              0 },	// Pulkovo 1942 / Gauss-Kruger CM 135E
	{     2514L,  2514L, "Pulkovo42.GK/CM-141E",       "Pulkovo 1942 / Gauss-Kruger CM 141E",                              0 },	// Pulkovo 1942 / Gauss-Kruger CM 141E
	{     2515L,  2515L, "Pulkovo42.GK/CM-147E",       "Pulkovo 1942 / Gauss-Kruger CM 147E",                              0 },	// Pulkovo 1942 / Gauss-Kruger CM 147E
	{     2516L,  2516L, "Pulkovo42.GK/CM-153E",       "Pulkovo 1942 / Gauss-Kruger CM 153E",                              0 },	// Pulkovo 1942 / Gauss-Kruger CM 153E
	{     2517L,  2517L, "Pulkovo42.GK/CM-159E",       "Pulkovo 1942 / Gauss-Kruger CM 159E",                              0 },	// Pulkovo 1942 / Gauss-Kruger CM 159E
	{     2518L,  2518L, "Pulkovo42.GK/CM-165E",       "Pulkovo 1942 / Gauss-Kruger CM 165E",                              0 },	// Pulkovo 1942 / Gauss-Kruger CM 165E
	{     2519L,  2519L, "Pulkovo42.GK/CM-171E",       "Pulkovo 1942 / Gauss-Kruger CM 171E",                              0 },	// Pulkovo 1942 / Gauss-Kruger CM 171E
	{     2520L,  2520L, "Pulkovo42.GK/CM-177E",       "Pulkovo 1942 / Gauss-Kruger CM 177E",                              0 },	// Pulkovo 1942 / Gauss-Kruger CM 177E
	{     2521L,  2521L, "Pulkovo42.GK/CM-177W",       "Pulkovo 1942 / Gauss-Kruger CM 177W",                              0 },	// Pulkovo 1942 / Gauss-Kruger CM 177W
	{     2522L,  2522L, "Pulkovo42.GK/CM-171W",       "Pulkovo 1942 / Gauss-Kruger CM 171W",                              0 },	// Pulkovo 1942 / Gauss-Kruger CM 171W
	{     2523L,  2523L, "Pulkovo42.GK3d-7",           "Pulkovo 1942 / 3-degree Gauss-Kruger zone 7",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 7
	{     2524L,  2524L, "Pulkovo42.GK3d-8",           "Pulkovo 1942 / 3-degree Gauss-Kruger zone 8",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 8
	{     2525L,  2525L, "Pulkovo42.GK3d-9",           "Pulkovo 1942 / 3-degree Gauss-Kruger zone 9",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 9
	{     2526L,  2526L, "Pulkovo42.GK3d-10",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 10",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 10
	{     2527L,  2527L, "Pulkovo42.GK3d-11",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 11",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 11
	{     2528L,  2528L, "Pulkovo42.GK3d-12",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 12",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 12
	{     2529L,  2529L, "Pulkovo42.GK3d-13",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 13",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 13
	{     2530L,  2530L, "Pulkovo42.GK3d-14",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 14",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 14
	{     2531L,  2531L, "Pulkovo42.GK3d-15",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 15",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 15
	{     2532L,  2532L, "Pulkovo42.GK3d-16",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 16",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 16
	{     2533L,  2533L, "Pulkovo42.GK3d-17",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 17",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 17
	{     2534L,  2534L, "Pulkovo42.GK3d-18",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 18",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 18
	{     2535L,  2535L, "Pulkovo42.GK3d-19",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 19",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 19
	{     2536L,  2536L, "Pulkovo42.GK3d-20",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 20",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 20
	{     2537L,  2537L, "Pulkovo42.GK3d-21",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 21",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 21
	{     2538L,  2538L, "Pulkovo42.GK3d-22",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 22",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 22
	{     2539L,  2539L, "Pulkovo42.GK3d-23",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 23",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 23
	{     2540L,  2540L, "Pulkovo42.GK3d-24",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 24",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 24
	{     2541L,  2541L, "Pulkovo42.GK3d-25",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 25",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 25
	{     2542L,  2542L, "Pulkovo42.GK3d-26",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 26",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 26
	{     2543L,  2543L, "Pulkovo42.GK3d-27",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 27",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 27
	{     2544L,  2544L, "Pulkovo42.GK3d-28",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 28",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 28
	{     2545L,  2545L, "Pulkovo42.GK3d-29",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 29",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 29
	{     2546L,  2546L, "Pulkovo42.GK3d-30",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 30",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 30
	{     2547L,  2547L, "Pulkovo42.GK3d-31",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 31",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 31
	{     2548L,  2548L, "Pulkovo42.GK3d-32",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 32",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 32
	{     2549L,  2549L, "Pulkovo42.GK3d-33",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 33",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 33
	{     2550L,  2550L, "Samboja.UTM-50S",            "Samboja / UTM zone 50S",                                           2 },	// Samboja / UTM zone 50S
	{     2551L,  2551L, "Pulkovo42.GK3d-34",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 34",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 34
	{     2552L,  2552L, "Pulkovo42.GK3d-35",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 35",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 35
	{     2553L,  2553L, "Pulkovo42.GK3d-36",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 36",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 36
	{     2554L,  2554L, "Pulkovo42.GK3d-37",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 37",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 37
	{     2555L,  2555L, "Pulkovo42.GK3d-38",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 38",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 38
	{     2556L,  2556L, "Pulkovo42.GK3d-39",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 39",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 39
	{     2557L,  2557L, "Pulkovo42.GK3d-40",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 40",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 40
	{     2558L,  2558L, "Pulkovo42.GK3d-41",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 41",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 41
	{     2559L,  2559L, "Pulkovo42.GK3d-42",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 42",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 42
	{     2560L,  2560L, "Pulkovo42.GK3d-43",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 43",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 43
	{     2561L,  2561L, "Pulkovo42.GK3d-44",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 44",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 44
	{     2562L,  2562L, "Pulkovo42.GK3d-45",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 45",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 45
	{     2563L,  2563L, "Pulkovo42.GK3d-46",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 46",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 46
	{     2564L,  2564L, "Pulkovo42.GK3d-47",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 47",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 47
	{     2565L,  2565L, "Pulkovo42.GK3d-48",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 48",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 48
	{     2566L,  2566L, "Pulkovo42.GK3d-49",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 49",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 49
	{     2567L,  2567L, "Pulkovo42.GK3d-50",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 50",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 50
	{     2568L,  2568L, "Pulkovo42.GK3d-51",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 51",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 51
	{     2569L,  2569L, "Pulkovo42.GK3d-52",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 52",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 52
	{     2570L,  2570L, "Pulkovo42.GK3d-53",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 53",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 53
	{     2571L,  2571L, "Pulkovo42.GK3d-54",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 54",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 54
	{     2572L,  2572L, "Pulkovo42.GK3d-55",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 55",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 55
	{     2573L,  2573L, "Pulkovo42.GK3d-56",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 56",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 56
	{     2574L,  2574L, "Pulkovo42.GK3d-57",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 57",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 57
	{     2575L,  2575L, "Pulkovo42.GK3d-58",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 58",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 58
	{     2576L,  2576L, "Pulkovo42.GK3d-59",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 59",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 59
	{     2577L,  2577L, "Pulkovo42.GK3d-60",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 60",                     2 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 60
	{     2578L,  2578L, "Pulkovo42.GK3d-61",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 61",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 61
	{     2579L,  2579L, "Pulkovo42.GK3d-62",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 62",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 62
	{     2580L,  2580L, "Pulkovo42.GK3d-63",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 63",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 63
	{     2581L,  2581L, "Pulkovo42.GK3d-64",          "Pulkovo 1942 / 3-degree Gauss-Kruger zone 64",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger zone 64
	{     2582L,  2582L, "Pulkovo42.GK3d/CM-21E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 21E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 21E
	{     2583L,  2583L, "Pulkovo42.GK3d/CM-24E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 24E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 24E
	{     2584L,  2584L, "Pulkovo42.GK3d/CM-27E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 27E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 27E
	{     2585L,  2585L, "Pulkovo42.GK3d/CM-30E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 30E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 30E
	{     2586L,  2586L, "Pulkovo42.GK3d/CM-33E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 33E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 33E
	{     2587L,  2587L, "Pulkovo42.GK3d/CM-36E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 36E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 36E
	{     2588L,  2588L, "Pulkovo42.GK3d/CM-39E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 39E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 39E
	{     2589L,  2589L, "Pulkovo42.GK3d/CM-42E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 42E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 42E
	{     2590L,  2590L, "Pulkovo42.GK3d/CM-45E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 45E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 45E
	{     2591L,  2591L, "Pulkovo42.GK3d/CM-48E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 48E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 48E
	{     2592L,  2592L, "Pulkovo42.GK3d/CM-51E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 51E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 51E
	{     2593L,  2593L, "Pulkovo42.GK3d/CM-54E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 54E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 54E
	{     2594L,  2594L, "Pulkovo42.GK3d/CM-57E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 57E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 57E
	{     2595L,  2595L, "Pulkovo42.GK3d/CM-60E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 60E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 60E
	{     2596L,  2596L, "Pulkovo42.GK3d/CM-63E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 63E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 63E
	{     2597L,  2597L, "Pulkovo42.GK3d/CM-66E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 66E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 66E
	{     2598L,  2598L, "Pulkovo42.GK3d/CM-69E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 69E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 69E
	{     2599L,  2599L, "Pulkovo42.GK3d/CM-72E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 72E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 72E
	{     2600L,  2600L, "Lietuvos1994",               "Lietuvos Koordinoei Sistema 1994",                                 2 },	// Lietuvos Koordinoei Sistema 1994
	{     2601L,  2601L, "Pulkovo42.GK3d/CM-75E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 75E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 75E
	{     2602L,  2602L, "Pulkovo42.GK3d/CM-78E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 78E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 78E
	{     2603L,  2603L, "Pulkovo42.GK3d/CM-81E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 81E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 81E
	{     2604L,  2604L, "Pulkovo42.GK3d/CM-84E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 84E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 84E
	{     2605L,  2605L, "Pulkovo42.GK3d/CM-87E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 87E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 87E
	{     2606L,  2606L, "Pulkovo42.GK3d/CM-90E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 90E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 90E
	{     2607L,  2607L, "Pulkovo42.GK3d/CM-93E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 93E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 93E
	{     2608L,  2608L, "Pulkovo42.GK3d/CM-96E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 96E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 96E
	{     2609L,  2609L, "Pulkovo42.GK3d/CM-99E",      "Pulkovo 1942 / 3-degree Gauss-Kruger CM 99E",                      0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 99E
	{     2610L,  2610L, "Pulkovo42.GK3d/CM-102E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 102E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 102E
	{     2611L,  2611L, "Pulkovo42.GK3d/CM-105E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 105E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 105E
	{     2612L,  2612L, "Pulkovo42.GK3d/CM-108E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 108E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 108E
	{     2613L,  2613L, "Pulkovo42.GK3d/CM-111E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 111E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 111E
	{     2614L,  2614L, "Pulkovo42.GK3d/CM-114E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 114E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 114E
	{     2615L,  2615L, "Pulkovo42.GK3d/CM-117E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 117E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 117E
	{     2616L,  2616L, "Pulkovo42.GK3d/CM-120E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 120E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 120E
	{     2617L,  2617L, "Pulkovo42.GK3d/CM-123E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 123E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 123E
	{     2618L,  2618L, "Pulkovo42.GK3d/CM-126E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 126E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 126E
	{     2619L,  2619L, "Pulkovo42.GK3d/CM-129E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 129E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 129E
	{     2620L,  2620L, "Pulkovo42.GK3d/CM-132E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 132E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 132E
	{     2621L,  2621L, "Pulkovo42.GK3d/CM-135E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 135E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 135E
	{     2622L,  2622L, "Pulkovo42.GK3d/CM-138E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 138E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 138E
	{     2623L,  2623L, "Pulkovo42.GK3d/CM-141E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 141E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 141E
	{     2624L,  2624L, "Pulkovo42.GK3d/CM-144E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 144E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 144E
	{     2625L,  2625L, "Pulkovo42.GK3d/CM-147E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 147E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 147E
	{     2626L,  2626L, "Pulkovo42.GK3d/CM-150E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 150E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 150E
	{     2627L,  2627L, "Pulkovo42.GK3d/CM-153E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 153E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 153E
	{     2628L,  2628L, "Pulkovo42.GK3d/CM-156E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 156E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 156E
	{     2629L,  2629L, "Pulkovo42.GK3d/CM-159E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 159E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 159E
	{     2630L,  2630L, "Pulkovo42.GK3d/CM-162E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 162E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 162E
	{     2631L,  2631L, "Pulkovo42.GK3d/CM-165E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 165E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 165E
	{     2632L,  2632L, "Pulkovo42.GK3d/CM-168E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 168E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 168E
	{     2633L,  2633L, "Pulkovo42.GK3d/CM-171E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 171E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 171E
	{     2634L,  2634L, "Pulkovo42.GK3d/CM-174E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 174E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 174E
	{     2635L,  2635L, "Pulkovo42.GK3d/CM-177E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 177E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 177E
	{     2636L,  2636L, "Pulkovo42.GK3d/CM-180E",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 180E",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 180E
	{     2637L,  2637L, "Pulkovo42.GK3d/CM-177W",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 177W",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 177W
	{     2638L,  2638L, "Pulkovo42.GK3d/CM-174W",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 174W",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 174W
	{     2639L,  2639L, "Pulkovo42.GK3d/CM-171W",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 171W",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 171W
	{     2640L,  2640L, "Pulkovo42.GK3d/CM-168W",     "Pulkovo 1942 / 3-degree Gauss-Kruger CM 168W",                     0 },	// Pulkovo 1942 / 3-degree Gauss-Kruger CM 168W
	{     2641L,  2641L, "Pulkovo95.GK3d-7",           "Pulkovo 1995 / 3-degree Gauss-Kruger zone 7",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 7
	{     2642L,  2642L, "Pulkovo95.GK3d-8",           "Pulkovo 1995 / 3-degree Gauss-Kruger zone 8",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 8
	{     2643L,  2643L, "Pulkovo95.GK3d-9",           "Pulkovo 1995 / 3-degree Gauss-Kruger zone 9",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 9
	{     2644L,  2644L, "Pulkovo95.GK3d-10",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 10",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 10
	{     2645L,  2645L, "Pulkovo95.GK3d-11",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 11",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 11
	{     2646L,  2646L, "Pulkovo95.GK3d-12",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 12",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 12
	{     2647L,  2647L, "Pulkovo95.GK3d-13",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 13",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 13
	{     2648L,  2648L, "Pulkovo95.GK3d-14",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 14",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 14
	{     2649L,  2649L, "Pulkovo95.GK3d-15",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 15",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 15
	{     2650L,  2650L, "Pulkovo95.GK3d-16",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 16",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 16
	{     2651L,  2651L, "Pulkovo95.GK3d-17",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 17",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 17
	{     2652L,  2652L, "Pulkovo95.GK3d-18",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 18",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 18
	{     2653L,  2653L, "Pulkovo95.GK3d-19",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 19",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 19
	{     2654L,  2654L, "Pulkovo95.GK3d-20",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 20",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 20
	{     2655L,  2655L, "Pulkovo95.GK3d-21",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 21",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 21
	{     2656L,  2656L, "Pulkovo95.GK3d-22",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 22",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 22
	{     2657L,  2657L, "Pulkovo95.GK3d-23",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 23",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 23
	{     2658L,  2658L, "Pulkovo95.GK3d-24",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 24",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 24
	{     2659L,  2659L, "Pulkovo95.GK3d-25",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 25",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 25
	{     2660L,  2660L, "Pulkovo95.GK3d-26",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 26",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 26
	{     2661L,  2661L, "Pulkovo95.GK3d-27",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 27",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 27
	{     2662L,  2662L, "Pulkovo95.GK3d-28",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 28",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 28
	{     2663L,  2663L, "Pulkovo95.GK3d-29",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 29",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 29
	{     2664L,  2664L, "Pulkovo95.GK3d-30",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 30",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 30
	{     2665L,  2665L, "Pulkovo95.GK3d-31",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 31",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 31
	{     2666L,  2666L, "Pulkovo95.GK3d-32",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 32",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 32
	{     2667L,  2667L, "Pulkovo95.GK3d-33",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 33",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 33
	{     2668L,  2668L, "Pulkovo95.GK3d-34",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 34",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 34
	{     2669L,  2669L, "Pulkovo95.GK3d-35",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 35",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 35
	{     2670L,  2670L, "Pulkovo95.GK3d-36",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 36",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 36
	{     2671L,  2671L, "Pulkovo95.GK3d-37",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 37",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 37
	{     2672L,  2672L, "Pulkovo95.GK3d-38",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 38",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 38
	{     2673L,  2673L, "Pulkovo95.GK3d-39",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 39",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 39
	{     2674L,  2674L, "Pulkovo95.GK3d-40",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 40",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 40
	{     2675L,  2675L, "Pulkovo95.GK3d-41",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 41",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 41
	{     2676L,  2676L, "Pulkovo95.GK3d-42",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 42",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 42
	{     2677L,  2677L, "Pulkovo95.GK3d-43",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 43",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 43
	{     2678L,  2678L, "Pulkovo95.GK3d-44",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 44",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 44
	{     2679L,  2679L, "Pulkovo95.GK3d-45",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 45",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 45
	{     2680L,  2680L, "Pulkovo95.GK3d-46",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 46",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 46
	{     2681L,  2681L, "Pulkovo95.GK3d-47",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 47",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 47
	{     2682L,  2682L, "Pulkovo95.GK3d-48",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 48",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 48
	{     2683L,  2683L, "Pulkovo95.GK3d-49",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 49",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 49
	{     2684L,  2684L, "Pulkovo95.GK3d-50",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 50",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 50
	{     2685L,  2685L, "Pulkovo95.GK3d-51",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 51",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 51
	{     2686L,  2686L, "Pulkovo95.GK3d-52",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 52",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 52
	{     2687L,  2687L, "Pulkovo95.GK3d-53",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 53",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 53
	{     2688L,  2688L, "Pulkovo95.GK3d-54",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 54",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 54
	{     2689L,  2689L, "Pulkovo95.GK3d-55",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 55",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 55
	{     2690L,  2690L, "Pulkovo95.GK3d-56",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 56",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 56
	{     2691L,  2691L, "Pulkovo95.GK3d-57",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 57",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 57
	{     2692L,  2692L, "Pulkovo95.GK3d-58",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 58",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 58
	{     2693L,  2693L, "Pulkovo95.GK3d-59",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 59",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 59
	{     2694L,  2694L, "Pulkovo95.GK3d-60",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 60",                     2 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 60
	{     2695L,  2695L, "Pulkovo95.GK3d-61",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 61",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 61
	{     2696L,  2696L, "Pulkovo95.GK3d-62",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 62",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 62
	{     2697L,  2697L, "Pulkovo95.GK3d-63",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 63",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 63
	{     2698L,  2698L, "Pulkovo95.GK3d-64",          "Pulkovo 1995 / 3-degree Gauss-Kruger zone 64",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger zone 64
	{     2699L,  2699L, "Pulkovo95.GK3d/CM-21E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 21E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 21E
	{     2700L,  2700L, "Pulkovo95.GK3d/CM-24E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 24E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 24E
	{     2701L,  2701L, "Pulkovo95.GK3d/CM-27E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 27E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 27E
	{     2702L,  2702L, "Pulkovo95.GK3d/CM-30E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 30E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 30E
	{     2703L,  2703L, "Pulkovo95.GK3d/CM-33E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 33E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 33E
	{     2704L,  2704L, "Pulkovo95.GK3d/CM-36E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 36E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 36E
	{     2705L,  2705L, "Pulkovo95.GK3d/CM-39E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 39E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 39E
	{     2706L,  2706L, "Pulkovo95.GK3d/CM-42E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 42E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 42E
	{     2707L,  2707L, "Pulkovo95.GK3d/CM-45E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 45E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 45E
	{     2708L,  2708L, "Pulkovo95.GK3d/CM-48E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 48E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 48E
	{     2709L,  2709L, "Pulkovo95.GK3d/CM-51E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 51E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 51E
	{     2710L,  2710L, "Pulkovo95.GK3d/CM-54E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 54E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 54E
	{     2711L,  2711L, "Pulkovo95.GK3d/CM-57E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 57E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 57E
	{     2712L,  2712L, "Pulkovo95.GK3d/CM-60E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 60E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 60E
	{     2713L,  2713L, "Pulkovo95.GK3d/CM-63E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 63E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 63E
	{     2714L,  2714L, "Pulkovo95.GK3d/CM-66E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 66E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 66E
	{     2715L,  2715L, "Pulkovo95.GK3d/CM-69E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 69E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 69E
	{     2716L,  2716L, "Pulkovo95.GK3d/CM-72E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 72E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 72E
	{     2717L,  2717L, "Pulkovo95.GK3d/CM-75E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 75E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 75E
	{     2718L,  2718L, "Pulkovo95.GK3d/CM-78E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 78E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 78E
	{     2719L,  2719L, "Pulkovo95.GK3d/CM-81E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 81E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 81E
	{     2720L,  2720L, "Pulkovo95.GK3d/CM-84E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 84E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 84E
	{     2721L,  2721L, "Pulkovo95.GK3d/CM-87E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 87E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 87E
	{     2722L,  2722L, "Pulkovo95.GK3d/CM-90E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 90E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 90E
	{     2723L,  2723L, "Pulkovo95.GK3d/CM-93E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 93E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 93E
	{     2724L,  2724L, "Pulkovo95.GK3d/CM-96E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 96E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 96E
	{     2725L,  2725L, "Pulkovo95.GK3d/CM-99E",      "Pulkovo 1995 / 3-degree Gauss-Kruger CM 99E",                      0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 99E
	{     2726L,  2726L, "Pulkovo95.GK3d/CM-102E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 102E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 102E
	{     2727L,  2727L, "Pulkovo95.GK3d/CM-105E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 105E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 105E
	{     2728L,  2728L, "Pulkovo95.GK3d/CM-108E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 108E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 108E
	{     2729L,  2729L, "Pulkovo95.GK3d/CM-111E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 111E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 111E
	{     2730L,  2730L, "Pulkovo95.GK3d/CM-114E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 114E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 114E
	{     2731L,  2731L, "Pulkovo95.GK3d/CM-117E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 117E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 117E
	{     2732L,  2732L, "Pulkovo95.GK3d/CM-120E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 120E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 120E
	{     2733L,  2733L, "Pulkovo95.GK3d/CM-123E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 123E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 123E
	{     2734L,  2734L, "Pulkovo95.GK3d/CM-126E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 126E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 126E
	{     2735L,  2735L, "Pulkovo95.GK3d/CM-129E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 129E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 129E
	{     2736L,  2736L, "Tete.UTM-36S",               "Tete / UTM zone 36S",                                              0 },	// Tete / UTM zone 36S
	{     2737L,  2737L, "Tete.UTM-37S",               "Tete / UTM zone 37S",                                              0 },	// Tete / UTM zone 37S
	{     2738L,  2738L, "Pulkovo95.GK3d/CM-132E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 132E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 132E
	{     2739L,  2739L, "Pulkovo95.GK3d/CM-135E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 135E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 135E
	{     2740L,  2740L, "Pulkovo95.GK3d/CM-138E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 138E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 138E
	{     2741L,  2741L, "Pulkovo95.GK3d/CM-141E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 141E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 141E
	{     2742L,  2742L, "Pulkovo95.GK3d/CM-144E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 144E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 144E
	{     2743L,  2743L, "Pulkovo95.GK3d/CM-147E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 147E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 147E
	{     2744L,  2744L, "Pulkovo95.GK3d/CM-150E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 150E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 150E
	{     2745L,  2745L, "Pulkovo95.GK3d/CM-153E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 153E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 153E
	{     2746L,  2746L, "Pulkovo95.GK3d/CM-156E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 156E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 156E
	{     2747L,  2747L, "Pulkovo95.GK3d/CM-159E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 159E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 159E
	{     2748L,  2748L, "Pulkovo95.GK3d/CM-162E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 162E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 162E
	{     2749L,  2749L, "Pulkovo95.GK3d/CM-165E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 165E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 165E
	{     2750L,  2750L, "Pulkovo95.GK3d/CM-168E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 168E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 168E
	{     2751L,  2751L, "Pulkovo95.GK3d/CM-171E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 171E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 171E
	{     2752L,  2752L, "Pulkovo95.GK3d/CM-174E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 174E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 174E
	{     2753L,  2753L, "Pulkovo95.GK3d/CM-177E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 177E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 177E
	{     2754L,  2754L, "Pulkovo95.GK3d/CM-180E",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 180E",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 180E
	{     2755L,  2755L, "Pulkovo95.GK3d/CM-177W",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 177W",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 177W
	{     2756L,  2756L, "Pulkovo95.GK3d/CM-174W",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 174W",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 174W
	{     2757L,  2757L, "Pulkovo95.GK3d/CM-171W",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 171W",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 171W
	{     2758L,  2758L, "Pulkovo95.GK3d/CM-168W",     "Pulkovo 1995 / 3-degree Gauss-Kruger CM 168W",                     0 },	// Pulkovo 1995 / 3-degree Gauss-Kruger CM 168W
	{     2759L,  2759L, "ALHP-E",                     "NAD83(HARN) / Alabama East",                                       1 },	// NAD83(HARN) / Alabama East
	{     2760L,  2760L, "ALHP-W",                     "NAD83(HARN) / Alabama West",                                       1 },	// NAD83(HARN) / Alabama West
	{     2761L,  2761L, "AZHP-E",                     "NAD83(HARN) / Arizona East",                                       1 },	// NAD83(HARN) / Arizona East
	{     2762L,  2762L, "AZHP-C",                     "NAD83(HARN) / Arizona Central",                                    1 },	// NAD83(HARN) / Arizona Central
	{     2763L,  2763L, "AZHP-W",                     "NAD83(HARN) / Arizona West",                                       1 },	// NAD83(HARN) / Arizona West
	{     2764L,  2764L, "ARHP-N",                     "NAD83(HARN) / Arkansas North",                                     1 },	// NAD83(HARN) / Arkansas North
	{     2765L,  2765L, "ARHP-S",                     "NAD83(HARN) / Arkansas South",                                     1 },	// NAD83(HARN) / Arkansas South
	{     2766L,  2766L, "CAHP-I",                     "NAD83(HARN) / California zone 1",                                  1 },	// NAD83(HARN) / California zone 1
	{     2767L,  2767L, "CAHP-II",                    "NAD83(HARN) / California zone 2",                                  1 },	// NAD83(HARN) / California zone 2
	{     2768L,  2768L, "CAHP-III",                   "NAD83(HARN) / California zone 3",                                  1 },	// NAD83(HARN) / California zone 3
	{     2769L,  2769L, "CAHP-IV",                    "NAD83(HARN) / California zone 4",                                  1 },	// NAD83(HARN) / California zone 4
	{     2770L,  2770L, "CAHP-V",                     "NAD83(HARN) / California zone 5",                                  1 },	// NAD83(HARN) / California zone 5
	{     2771L,  2771L, "CAHP-VI",                    "NAD83(HARN) / California zone 6",                                  1 },	// NAD83(HARN) / California zone 6
	{     2772L,  2772L, "COHP-N",                     "NAD83(HARN) / Colorado North",                                     1 },	// NAD83(HARN) / Colorado North
	{     2773L,  2773L, "COHP-C",                     "NAD83(HARN) / Colorado Central",                                   1 },	// NAD83(HARN) / Colorado Central
	{     2774L,  2774L, "COHP-S",                     "NAD83(HARN) / Colorado South",                                     1 },	// NAD83(HARN) / Colorado South
	{     2775L,  2775L, "CTHP",                       "NAD83(HARN) / Connecticut",                                        1 },	// NAD83(HARN) / Connecticut
	{     2776L,  2776L, "DEHP",                       "NAD83(HARN) / Delaware",                                           1 },	// NAD83(HARN) / Delaware
	{     2777L,  2777L, "FLHP-E",                     "NAD83(HARN) / Florida East",                                       1 },	// NAD83(HARN) / Florida East
	{     2778L,  2778L, "FLHP-W",                     "NAD83(HARN) / Florida West",                                       1 },	// NAD83(HARN) / Florida West
	{     2779L,  2779L, "FLHP-N",                     "NAD83(HARN) / Florida North",                                      1 },	// NAD83(HARN) / Florida North
	{     2780L,  2780L, "GAHP-E",                     "NAD83(HARN) / Georgia East",                                       1 },	// NAD83(HARN) / Georgia East
	{     2781L,  2781L, "GAHP-W",                     "NAD83(HARN) / Georgia West",                                       1 },	// NAD83(HARN) / Georgia West
	{     2782L,  2782L, "HIHP-1",                     "NAD83(HARN) / Hawaii zone 1",                                      0 },	// NAD83(HARN) / Hawaii zone 1
	{     2783L,  2783L, "HIHP-2",                     "NAD83(HARN) / Hawaii zone 2",                                      0 },	// NAD83(HARN) / Hawaii zone 2
	{     2784L,  2784L, "HIHP-3",                     "NAD83(HARN) / Hawaii zone 3",                                      0 },	// NAD83(HARN) / Hawaii zone 3
	{     2785L,  2785L, "HIHP-4",                     "NAD83(HARN) / Hawaii zone 4",                                      0 },	// NAD83(HARN) / Hawaii zone 4
	{     2786L,  2786L, "HIHP-5",                     "NAD83(HARN) / Hawaii zone 5",                                      0 },	// NAD83(HARN) / Hawaii zone 5
	{     2787L,  2787L, "IDHP-E",                     "NAD83(HARN) / Idaho East",                                         1 },	// NAD83(HARN) / Idaho East
	{     2788L,  2788L, "IDHP-C",                     "NAD83(HARN) / Idaho Central",                                      1 },	// NAD83(HARN) / Idaho Central
	{     2789L,  2789L, "IDHP-W",                     "NAD83(HARN) / Idaho West",                                         1 },	// NAD83(HARN) / Idaho West
	{     2790L,  2790L, "ILHP-E",                     "NAD83(HARN) / Illinois East",                                      1 },	// NAD83(HARN) / Illinois East
	{     2791L,  2791L, "ILHP-W",                     "NAD83(HARN) / Illinois West",                                      1 },	// NAD83(HARN) / Illinois West
	{     2792L,  2792L, "INHP-E",                     "NAD83(HARN) / Indiana East",                                       1 },	// NAD83(HARN) / Indiana East
	{     2793L,  2793L, "INHP-W",                     "NAD83(HARN) / Indiana West",                                       1 },	// NAD83(HARN) / Indiana West
	{     2794L,  2794L, "IAHP-N",                     "NAD83(HARN) / Iowa North",                                         1 },	// NAD83(HARN) / Iowa North
	{     2795L,  2795L, "IAHP-S",                     "NAD83(HARN) / Iowa South",                                         1 },	// NAD83(HARN) / Iowa South
	{     2796L,  2796L, "KSHP-N",                     "NAD83(HARN) / Kansas North",                                       1 },	// NAD83(HARN) / Kansas North
	{     2797L,  2797L, "KSHP-S",                     "NAD83(HARN) / Kansas South",                                       1 },	// NAD83(HARN) / Kansas South
	{     2798L,  2798L, "KYHP-N",                     "NAD83(HARN) / Kentucky North",                                     1 },	// NAD83(HARN) / Kentucky North
	{     2799L,  2799L, "KYHP-S",                     "NAD83(HARN) / Kentucky South",                                     1 },	// NAD83(HARN) / Kentucky South
	{     2800L,  2800L, "LAHP-N",                     "NAD83(HARN) / Louisiana North",                                    1 },	// NAD83(HARN) / Louisiana North
	{     2801L,  2801L, "LAHP-S",                     "NAD83(HARN) / Louisiana South",                                    1 },	// NAD83(HARN) / Louisiana South
	{     2802L,  2802L, "MEHP-E",                     "NAD83(HARN) / Maine East",                                         1 },	// NAD83(HARN) / Maine East
	{     2803L,  2803L, "MEHP-W",                     "NAD83(HARN) / Maine West",                                         1 },	// NAD83(HARN) / Maine West
	{     2804L,  2804L, "MDHP",                       "NAD83(HARN) / Maryland",                                           1 },	// NAD83(HARN) / Maryland
	{     2805L,  2805L, "MAHP",                       "NAD83(HARN) / Massachusetts Mainland",                             1 },	// NAD83(HARN) / Massachusetts Mainland
	{     2806L,  2806L, "MAHP-IS",                    "NAD83(HARN) / Massachusetts Island",                               1 },	// NAD83(HARN) / Massachusetts Island
	{     2807L,  2807L, "MIHP-N",                     "NAD83(HARN) / Michigan North",                                     1 },	// NAD83(HARN) / Michigan North
	{     2808L,  2808L, "MIHP-C",                     "NAD83(HARN) / Michigan Central",                                   1 },	// NAD83(HARN) / Michigan Central
	{     2809L,  2809L, "MIHP-S",                     "NAD83(HARN) / Michigan South",                                     1 },	// NAD83(HARN) / Michigan South
	{     2810L,  2810L, "MNHP-N",                     "NAD83(HARN) / Minnesota North",                                    1 },	// NAD83(HARN) / Minnesota North
	{     2811L,  2811L, "MNHP-C",                     "NAD83(HARN) / Minnesota Central",                                  1 },	// NAD83(HARN) / Minnesota Central
	{     2812L,  2812L, "MNHP-S",                     "NAD83(HARN) / Minnesota South",                                    1 },	// NAD83(HARN) / Minnesota South
	{     2813L,  2813L, "MSHP-E",                     "NAD83(HARN) / Mississippi East",                                   1 },	// NAD83(HARN) / Mississippi East
	{     2814L,  2814L, "MSHP-W",                     "NAD83(HARN) / Mississippi West",                                   1 },	// NAD83(HARN) / Mississippi West
	{     2815L,  2815L, "MOHP-E",                     "NAD83(HARN) / Missouri East",                                      1 },	// NAD83(HARN) / Missouri East
	{     2816L,  2816L, "MOHP-C",                     "NAD83(HARN) / Missouri Central",                                   1 },	// NAD83(HARN) / Missouri Central
	{     2817L,  2817L, "MOHP-W",                     "NAD83(HARN) / Missouri West",                                      1 },	// NAD83(HARN) / Missouri West
	{     2818L,  2818L, "MTHP",                       "NAD83(HARN) / Montana",                                            1 },	// NAD83(HARN) / Montana
	{     2819L,  2819L, "NBHP",                       "NAD83(HARN) / Nebraska",                                           1 },	// NAD83(HARN) / Nebraska
	{     2820L,  2820L, "NVHP-E",                     "NAD83(HARN) / Nevada East",                                        1 },	// NAD83(HARN) / Nevada East
	{     2821L,  2821L, "NVHP-C",                     "NAD83(HARN) / Nevada Central",                                     1 },	// NAD83(HARN) / Nevada Central
	{     2822L,  2822L, "NVHP-W",                     "NAD83(HARN) / Nevada West",                                        1 },	// NAD83(HARN) / Nevada West
	{     2823L,  2823L, "NHHP",                       "NAD83(HARN) / New Hampshire",                                      1 },	// NAD83(HARN) / New Hampshire
	{     2824L,  2824L, "NJHP",                       "NAD83(HARN) / New Jersey",                                         1 },	// NAD83(HARN) / New Jersey
	{     2825L,  2825L, "NMHP-E",                     "NAD83(HARN) / New Mexico East",                                    1 },	// NAD83(HARN) / New Mexico East
	{     2826L,  2826L, "NMHP-C",                     "NAD83(HARN) / New Mexico Central",                                 1 },	// NAD83(HARN) / New Mexico Central
	{     2827L,  2827L, "NMHP-W",                     "NAD83(HARN) / New Mexico West",                                    1 },	// NAD83(HARN) / New Mexico West
	{     2828L,  2828L, "NYHP-E",                     "NAD83(HARN) / New York East",                                      1 },	// NAD83(HARN) / New York East
	{     2829L,  2829L, "NYHP-C",                     "NAD83(HARN) / New York Central",                                   1 },	// NAD83(HARN) / New York Central
	{     2830L,  2830L, "NYHP-W",                     "NAD83(HARN) / New York West",                                      1 },	// NAD83(HARN) / New York West
	{     2831L,  2831L, "NYHP-LI",                    "NAD83(HARN) / New York Long Island",                               1 },	// NAD83(HARN) / New York Long Island
	{     2832L,  2832L, "NDHP-N",                     "NAD83(HARN) / North Dakota North",                                 1 },	// NAD83(HARN) / North Dakota North
	{     2833L,  2833L, "NDHP-S",                     "NAD83(HARN) / North Dakota South",                                 1 },	// NAD83(HARN) / North Dakota South
	{     2834L,  2834L, "OHHP-N",                     "NAD83(HARN) / Ohio North",                                         1 },	// NAD83(HARN) / Ohio North
	{     2835L,  2835L, "OHHP-S",                     "NAD83(HARN) / Ohio South",                                         1 },	// NAD83(HARN) / Ohio South
	{     2836L,  2836L, "OKHP-N",                     "NAD83(HARN) / Oklahoma North",                                     1 },	// NAD83(HARN) / Oklahoma North
	{     2837L,  2837L, "OKHP-S",                     "NAD83(HARN) / Oklahoma South",                                     1 },	// NAD83(HARN) / Oklahoma South
	{     2838L,  2838L, "ORHP-N",                     "NAD83(HARN) / Oregon North",                                       1 },	// NAD83(HARN) / Oregon North
	{     2839L,  2839L, "ORHP-S",                     "NAD83(HARN) / Oregon South",                                       1 },	// NAD83(HARN) / Oregon South
	{     2840L,  2840L, "RIHP",                       "NAD83(HARN) / Rhode Island",                                       1 },	// NAD83(HARN) / Rhode Island
	{     2841L,  2841L, "SDHP-N",                     "NAD83(HARN) / South Dakota North",                                 1 },	// NAD83(HARN) / South Dakota North
	{     2842L,  2842L, "SDHP-S",                     "NAD83(HARN) / South Dakota South",                                 1 },	// NAD83(HARN) / South Dakota South
	{     2843L,  2843L, "TNHP",                       "NAD83(HARN) / Tennessee",                                          1 },	// NAD83(HARN) / Tennessee
	{     2844L,  2844L, "TXHP-N",                     "NAD83(HARN) / Texas North",                                        1 },	// NAD83(HARN) / Texas North
	{     2845L,  2845L, "TXHP-NC",                    "NAD83(HARN) / Texas North Central",                                1 },	// NAD83(HARN) / Texas North Central
	{     2846L,  2846L, "TXHP-C",                     "NAD83(HARN) / Texas Central",                                      1 },	// NAD83(HARN) / Texas Central
	{     2847L,  2847L, "TXHP-SC",                    "NAD83(HARN) / Texas South Central",                                1 },	// NAD83(HARN) / Texas South Central
	{     2848L,  2848L, "TXHP-S",                     "NAD83(HARN) / Texas South",                                        1 },	// NAD83(HARN) / Texas South
	{     2849L,  2849L, "UTHP-N",                     "NAD83(HARN) / Utah North",                                         1 },	// NAD83(HARN) / Utah North
	{     2850L,  2850L, "UTHP-C",                     "NAD83(HARN) / Utah Central",                                       1 },	// NAD83(HARN) / Utah Central
	{     2851L,  2851L, "UTHP-S",                     "NAD83(HARN) / Utah South",                                         1 },	// NAD83(HARN) / Utah South
	{     2852L,  2852L, "VTHP",                       "NAD83(HARN) / Vermont",                                            1 },	// NAD83(HARN) / Vermont
	{     2853L,  2853L, "VAHP-N",                     "NAD83(HARN) / Virginia North",                                     1 },	// NAD83(HARN) / Virginia North
	{     2854L,  2854L, "VAHP-S",                     "NAD83(HARN) / Virginia South",                                     1 },	// NAD83(HARN) / Virginia South
	{     2855L,  2855L, "WAHP-N",                     "NAD83(HARN) / Washington North",                                   1 },	// NAD83(HARN) / Washington North
	{     2856L,  2856L, "WAHP-S",                     "NAD83(HARN) / Washington South",                                   1 },	// NAD83(HARN) / Washington South
	{     2857L,  2857L, "WVHP-N",                     "NAD83(HARN) / West Virginia North",                                1 },	// NAD83(HARN) / West Virginia North
	{     2858L,  2858L, "WVHP-S",                     "NAD83(HARN) / West Virginia South",                                1 },	// NAD83(HARN) / West Virginia South
	{     2859L,  2859L, "WIHP-N",                     "NAD83(HARN) / Wisconsin North",                                    1 },	// NAD83(HARN) / Wisconsin North
	{     2860L,  2860L, "WIHP-C",                     "NAD83(HARN) / Wisconsin Central",                                  1 },	// NAD83(HARN) / Wisconsin Central
	{     2861L,  2861L, "WIHP-S",                     "NAD83(HARN) / Wisconsin South",                                    1 },	// NAD83(HARN) / Wisconsin South
	{     2862L,  2862L, "WYHP-E",                     "NAD83(HARN) / Wyoming East",                                       1 },	// NAD83(HARN) / Wyoming East
	{     2863L,  2863L, "WYHP-EC",                    "NAD83(HARN) / Wyoming East Central",                               1 },	// NAD83(HARN) / Wyoming East Central
	{     2864L,  2864L, "WYHP-WC",                    "NAD83(HARN) / Wyoming West Central",                               1 },	// NAD83(HARN) / Wyoming West Central
	{     2865L,  2865L, "WYHP-W",                     "NAD83(HARN) / Wyoming West",                                       1 },	// NAD83(HARN) / Wyoming West
	{     2866L,  2866L, "PRHP",                       "NAD83(HARN) / Puerto Rico & Virgin Is.",                           1 },	// NAD83(HARN) / Puerto Rico & Virgin Is.
	{     2867L,  2867L, "AZHP-EIF",                   "NAD83(HARN) / Arizona East (ft)",                                  1 },	// NAD83(HARN) / Arizona East (ft)
	{     2868L,  2868L, "AZHP-CIF",                   "NAD83(HARN) / Arizona Central (ft)",                               1 },	// NAD83(HARN) / Arizona Central (ft)
	{     2869L,  2869L, "AZHP-WIF",                   "NAD83(HARN) / Arizona West (ft)",                                  1 },	// NAD83(HARN) / Arizona West (ft)
	{     2870L,  2870L, "CAHP-IF",                    "NAD83(HARN) / California zone 1 (ftUS)",                           1 },	// NAD83(HARN) / California zone 1 (ftUS)
	{     2871L,  2871L, "CAHP-IIF",                   "NAD83(HARN) / California zone 2 (ftUS)",                           1 },	// NAD83(HARN) / California zone 2 (ftUS)
	{     2872L,  2872L, "CAHPIIIF",                   "NAD83(HARN) / California zone 3 (ftUS)",                           1 },	// NAD83(HARN) / California zone 3 (ftUS)
	{     2873L,  2873L, "CAHP-IVF",                   "NAD83(HARN) / California zone 4 (ftUS)",                           1 },	// NAD83(HARN) / California zone 4 (ftUS)
	{     2874L,  2874L, "CAHP-VF",                    "NAD83(HARN) / California zone 5 (ftUS)",                           1 },	// NAD83(HARN) / California zone 5 (ftUS)
	{     2875L,  2875L, "CAHP-VIF",                   "NAD83(HARN) / California zone 6 (ftUS)",                           1 },	// NAD83(HARN) / California zone 6 (ftUS)
	{     2876L,  2876L, "COHP-NF",                    "NAD83(HARN) / Colorado North (ftUS)",                              1 },	// NAD83(HARN) / Colorado North (ftUS)
	{     2877L,  2877L, "COHP-CF",                    "NAD83(HARN) / Colorado Central (ftUS)",                            1 },	// NAD83(HARN) / Colorado Central (ftUS)
	{     2878L,  2878L, "COHP-SF",                    "NAD83(HARN) / Colorado South (ftUS)",                              1 },	// NAD83(HARN) / Colorado South (ftUS)
	{     2879L,  2879L, "CTHPF",                      "NAD83(HARN) / Connecticut (ftUS)",                                 1 },	// NAD83(HARN) / Connecticut (ftUS)
	{     2880L,  2880L, "DEHPF",                      "NAD83(HARN) / Delaware (ftUS)",                                    1 },	// NAD83(HARN) / Delaware (ftUS)
	{     2881L,  2881L, "FLHP-EF",                    "NAD83(HARN) / Florida East (ftUS)",                                1 },	// NAD83(HARN) / Florida East (ftUS)
	{     2882L,  2882L, "FLHP-WF",                    "NAD83(HARN) / Florida West (ftUS)",                                1 },	// NAD83(HARN) / Florida West (ftUS)
	{     2883L,  2883L, "FLHP-NF",                    "NAD83(HARN) / Florida North (ftUS)",                               1 },	// NAD83(HARN) / Florida North (ftUS)
	{     2884L,  2884L, "GAHP-EF",                    "NAD83(HARN) / Georgia East (ftUS)",                                1 },	// NAD83(HARN) / Georgia East (ftUS)
	{     2885L,  2885L, "GAHP-WF",                    "NAD83(HARN) / Georgia West (ftUS)",                                1 },	// NAD83(HARN) / Georgia West (ftUS)
	{     2886L,  2886L, "IDHP-EF",                    "NAD83(HARN) / Idaho East (ftUS)",                                  1 },	// NAD83(HARN) / Idaho East (ftUS)
	{     2887L,  2887L, "IDHP-CF",                    "NAD83(HARN) / Idaho Central (ftUS)",                               1 },	// NAD83(HARN) / Idaho Central (ftUS)
	{     2888L,  2888L, "IDHP-WF",                    "NAD83(HARN) / Idaho West (ftUS)",                                  1 },	// NAD83(HARN) / Idaho West (ftUS)
	{     2967L,  2889L, "INHP-EF",                    "NAD83(HARN) / Indiana East (ftUS)",                                3 },	// NAD83(HARN) / Indiana East (ftUS)
	{     2968L,  2890L, "INHP-WF",                    "NAD83(HARN) / Indiana West (ftUS)",                                3 },	// NAD83(HARN) / Indiana West (ftUS)
	{     2891L,  2891L, "KYHP-NF",                    "NAD83(HARN) / Kentucky North (ftUS)",                              1 },	// NAD83(HARN) / Kentucky North (ftUS)
	{     2892L,  2892L, "KYHP-SF",                    "NAD83(HARN) / Kentucky South (ftUS)",                              1 },	// NAD83(HARN) / Kentucky South (ftUS)
	{     2893L,  2893L, "MDHPF",                      "NAD83(HARN) / Maryland (ftUS)",                                    1 },	// NAD83(HARN) / Maryland (ftUS)
	{     2894L,  2894L, "MAHPF",                      "NAD83(HARN) / Massachusetts Mainland (ftUS)",                      1 },	// NAD83(HARN) / Massachusetts Mainland (ftUS)
	{     2895L,  2895L, "MAHP-ISF",                   "NAD83(HARN) / Massachusetts Island (ftUS)",                        1 },	// NAD83(HARN) / Massachusetts Island (ftUS)
	{     2896L,  2896L, "MIHP-NIF",                   "NAD83(HARN) / Michigan North (ft)",                                1 },	// NAD83(HARN) / Michigan North (ft)
	{     2897L,  2897L, "MIHP-CIF",                   "NAD83(HARN) / Michigan Central (ft)",                              1 },	// NAD83(HARN) / Michigan Central (ft)
	{     2898L,  2898L, "MIHP-SIF",                   "NAD83(HARN) / Michigan South (ft)",                                1 },	// NAD83(HARN) / Michigan South (ft)
	{     2899L,  2899L, "MSHP-EF",                    "NAD83(HARN) / Mississippi East (ftUS)",                            1 },	// NAD83(HARN) / Mississippi East (ftUS)
	{     2900L,  2900L, "MSHP-WF",                    "NAD83(HARN) / Mississippi West (ftUS)",                            1 },	// NAD83(HARN) / Mississippi West (ftUS)
	{     2901L,  2901L, "MTHPIF",                     "NAD83(HARN) / Montana (ft)",                                       1 },	// NAD83(HARN) / Montana (ft)
	{     2902L,  2902L, "NMHP-EF",                    "NAD83(HARN) / New Mexico East (ftUS)",                             1 },	// NAD83(HARN) / New Mexico East (ftUS)
	{     2903L,  2903L, "NMHP-CF",                    "NAD83(HARN) / New Mexico Central (ftUS)",                          1 },	// NAD83(HARN) / New Mexico Central (ftUS)
	{     2904L,  2904L, "NMHP-WF",                    "NAD83(HARN) / New Mexico West (ftUS)",                             1 },	// NAD83(HARN) / New Mexico West (ftUS)
	{     2905L,  2905L, "NYHP-EF",                    "NAD83(HARN) / New York East (ftUS)",                               1 },	// NAD83(HARN) / New York East (ftUS)
	{     2906L,  2906L, "NYHP-CF",                    "NAD83(HARN) / New York Central (ftUS)",                            1 },	// NAD83(HARN) / New York Central (ftUS)
	{     2907L,  2907L, "NYHP-WF",                    "NAD83(HARN) / New York West (ftUS)",                               1 },	// NAD83(HARN) / New York West (ftUS)
	{     2908L,  2908L, "NYHP-LIF",                   "NAD83(HARN) / New York Long Island (ftUS)",                        1 },	// NAD83(HARN) / New York Long Island (ftUS)
	{     2909L,  2909L, "NDHP-NIF",                   "NAD83(HARN) / North Dakota North (ft)",                            0 },	// NAD83(HARN) / North Dakota North (ft)
	{     2910L,  2910L, "NDHP-SIF",                   "NAD83(HARN) / North Dakota South (ft)",                            0 },	// NAD83(HARN) / North Dakota South (ft)
	{     2911L,  2911L, "OKHP-NF",                    "NAD83(HARN) / Oklahoma North (ftUS)",                              1 },	// NAD83(HARN) / Oklahoma North (ftUS)
	{     2912L,  2912L, "OKHP-SF",                    "NAD83(HARN) / Oklahoma South (ftUS)",                              1 },	// NAD83(HARN) / Oklahoma South (ftUS)
	{     2913L,  2913L, "ORHP-NIF",                   "NAD83(HARN) / Oregon North (ft)",                                  1 },	// NAD83(HARN) / Oregon North (ft)
	{     2914L,  2914L, "ORHP-SIF",                   "NAD83(HARN) / Oregon South (ft)",                                  1 },	// NAD83(HARN) / Oregon South (ft)
	{     2915L,  2915L, "TNHPF",                      "NAD83(HARN) / Tennessee (ftUS)",                                   1 },	// NAD83(HARN) / Tennessee (ftUS)
	{     2916L,  2916L, "TXHP-NF",                    "NAD83(HARN) / Texas North (ftUS)",                                 1 },	// NAD83(HARN) / Texas North (ftUS)
	{     2917L,  2917L, "TXHP-NCF",                   "NAD83(HARN) / Texas North Central (ftUS)",                         1 },	// NAD83(HARN) / Texas North Central (ftUS)
	{     2918L,  2918L, "TXHP-CF",                    "NAD83(HARN) / Texas Central (ftUS)",                               1 },	// NAD83(HARN) / Texas Central (ftUS)
	{     2919L,  2919L, "TXHP-SCF",                   "NAD83(HARN) / Texas South Central (ftUS)",                         1 },	// NAD83(HARN) / Texas South Central (ftUS)
	{     2920L,  2920L, "TXHP-SF",                    "NAD83(HARN) / Texas South (ftUS)",                                 1 },	// NAD83(HARN) / Texas South (ftUS)
	{     2921L,  2921L, "UTHP-NIF",                   "NAD83(HARN) / Utah North (ft)",                                    1 },	// NAD83(HARN) / Utah North (ft)
	{     2922L,  2922L, "UTHP-CIF",                   "NAD83(HARN) / Utah Central (ft)",                                  1 },	// NAD83(HARN) / Utah Central (ft)
	{     2923L,  2923L, "UTHP-SIF",                   "NAD83(HARN) / Utah South (ft)",                                    1 },	// NAD83(HARN) / Utah South (ft)
	{     2924L,  2924L, "VAHP-NF",                    "NAD83(HARN) / Virginia North (ftUS)",                              1 },	// NAD83(HARN) / Virginia North (ftUS)
	{     2925L,  2925L, "VAHP-SF",                    "NAD83(HARN) / Virginia South (ftUS)",                              1 },	// NAD83(HARN) / Virginia South (ftUS)
	{     2926L,  2926L, "WAHP-NF",                    "NAD83(HARN) / Washington North (ftUS)",                            1 },	// NAD83(HARN) / Washington North (ftUS)
	{     2927L,  2927L, "WAHP-SF",                    "NAD83(HARN) / Washington South (ftUS)",                            1 },	// NAD83(HARN) / Washington South (ftUS)
	{     2928L,  2928L, "WIHP-NF",                    "NAD83(HARN) / Wisconsin North (ftUS)",                             1 },	// NAD83(HARN) / Wisconsin North (ftUS)
	{     2929L,  2929L, "WIHP-CF",                    "NAD83(HARN) / Wisconsin Central (ftUS)",                           1 },	// NAD83(HARN) / Wisconsin Central (ftUS)
	{     2930L,  2930L, "WIHP-SF",                    "NAD83(HARN) / Wisconsin South (ftUS)",                             1 },	// NAD83(HARN) / Wisconsin South (ftUS)
	{     2932L,  2932L, "QND95.QatarNational",        "QND95 / Qatar National Grid",                                      0 },	// QND95 / Qatar National Grid
	{     2933L,  2933L, "Segara.UTM-50S",             "Segara / UTM zone 50S",                                            0 },	// Segara / UTM zone 50S
	{     2935L,  2935L, "Pulkovo42.CS63-A1",          "Pulkovo 1942 / CS63 zone A1",                                      0 },	// Pulkovo 1942 / CS63 zone A1
	{     2936L,  2936L, "Pulkovo42.CS63-A2",          "Pulkovo 1942 / CS63 zone A2",                                      0 },	// Pulkovo 1942 / CS63 zone A2
	{     2937L,  2937L, "Pulkovo42.CS63-A3",          "Pulkovo 1942 / CS63 zone A3",                                      0 },	// Pulkovo 1942 / CS63 zone A3
	{     2938L,  2938L, "Pulkovo42.CS63-A4",          "Pulkovo 1942 / CS63 zone A4",                                      0 },	// Pulkovo 1942 / CS63 zone A4
	{     2939L,  2939L, "Pulkovo42.CS63-K2",          "Pulkovo 1942 / CS63 zone K2",                                      0 },	// Pulkovo 1942 / CS63 zone K2
	{     2940L,  2940L, "Pulkovo42.CS63-K3",          "Pulkovo 1942 / CS63 zone K3",                                      0 },	// Pulkovo 1942 / CS63 zone K3
	{     2941L,  2941L, "Pulkovo42.CS63-K4",          "Pulkovo 1942 / CS63 zone K4",                                      0 },	// Pulkovo 1942 / CS63 zone K4
	{     2942L,  2942L, "PortoSanto.UTM-28N",         "Porto Santo / UTM zone 28N",                                       0 },	// Porto Santo / UTM zone 28N
	{     2943L,  2943L, "SelvagemGrande.UTM-28N",     "Selvagem Grande / UTM zone 28N",                                   0 },	// Selvagem Grande / UTM zone 28N
	{     2944L,  2944L, "CSRS.SCoPQ-2",               "NAD83(CSRS) / SCoPQ zone 2",                                       0 },	// NAD83(CSRS) / SCoPQ zone 2
	{     2945L,  2945L, "CSRS.MTM-3",                 "NAD83(CSRS) / MTM zone 3",                                         0 },	// NAD83(CSRS) / MTM zone 3
	{     2946L,  2946L, "CSRS.MTM-4",                 "NAD83(CSRS) / MTM zone 4",                                         0 },	// NAD83(CSRS) / MTM zone 4
	{     2947L,  2947L, "CSRS.MTM-5",                 "NAD83(CSRS) / MTM zone 5",                                         0 },	// NAD83(CSRS) / MTM zone 5
	{     2948L,  2948L, "CSRS.MTM-6",                 "NAD83(CSRS) / MTM zone 6",                                         0 },	// NAD83(CSRS) / MTM zone 6
	{     2949L,  2949L, "CSRS.MTM-7",                 "NAD83(CSRS) / MTM zone 7",                                         0 },	// NAD83(CSRS) / MTM zone 7
	{     2950L,  2950L, "CSRS.MTM-8",                 "NAD83(CSRS) / MTM zone 8",                                         0 },	// NAD83(CSRS) / MTM zone 8
	{     2951L,  2951L, "CSRS.MTM-9",                 "NAD83(CSRS) / MTM zone 9",                                         0 },	// NAD83(CSRS) / MTM zone 9
	{     2952L,  2952L, "CSRS.MTM-10",                "NAD83(CSRS) / MTM zone 10",                                        0 },	// NAD83(CSRS) / MTM zone 10
	{     2955L,  2955L, "CSRS.UTM-11N",               "NAD83(CSRS) / UTM zone 11N",                                       0 },	// NAD83(CSRS) / UTM zone 11N
	{     2956L,  2956L, "CSRS.UTM-12N",               "NAD83(CSRS) / UTM zone 12N",                                       0 },	// NAD83(CSRS) / UTM zone 12N
	{     2957L,  2957L, "CSRS.UTM-13N",               "NAD83(CSRS) / UTM zone 13N",                                       0 },	// NAD83(CSRS) / UTM zone 13N
	{     2958L,  2958L, "CSRS.UTM-17N",               "NAD83(CSRS) / UTM zone 17N",                                       0 },	// NAD83(CSRS) / UTM zone 17N
	{     2959L,  2959L, "CSRS.UTM-18N",               "NAD83(CSRS) / UTM zone 18N",                                       0 },	// NAD83(CSRS) / UTM zone 18N
	{     2960L,  2960L, "CSRS.UTM-19N",               "NAD83(CSRS) / UTM zone 19N",                                       0 },	// NAD83(CSRS) / UTM zone 19N
	{     2961L,  2961L, "CSRS.UTM-20N",               "NAD83(CSRS) / UTM zone 20N",                                       0 },	// NAD83(CSRS) / UTM zone 20N
	{     2962L,  2962L, "CSRS.UTM-21N",               "NAD83(CSRS) / UTM zone 21N",                                       0 },	// NAD83(CSRS) / UTM zone 21N
	{     2964L,  2964L, "USAK",                       "NAD27 / Alaska Albers",                                            1 },	// NAD27 / Alaska Albers
	{        0L,  2965L, "IN83-EF",                    "NAD83 / Indiana East (ftUS)",                                      1 },	// NAD83 / Indiana East (ftUS)
	{        0L,  2966L, "IN83-WF",                    "NAD83 / Indiana West (ftUS)",                                      1 },	// NAD83 / Indiana West (ftUS)
	{        0L,  2967L, "INHP-EF",                    "NAD83(HARN) / Indiana East (ftUS)",                                1 },	// NAD83(HARN) / Indiana East (ftUS)
	{        0L,  2968L, "INHP-WF",                    "NAD83(HARN) / Indiana West (ftUS)",                                1 },	// NAD83(HARN) / Indiana West (ftUS)
	{     2969L,  2969L, "FortMarigot.UTM-20N",        "Fort Marigot / UTM zone 20N",                                      0 },	// Fort Marigot / UTM zone 20N
	{     2970L,  2970L, "SainteAnne.UTM-20N",         "Sainte Anne / UTM zone 20N",                                       0 },	// Guadeloupe 1948 / UTM zone 20N
	{     2971L,  2971L, "CSG67.UTM-22N",              "CSG67 / UTM zone 22N",                                             0 },	// CSG67 / UTM zone 22N
	{     2972L,  2972L, "RGFG95.UTM-22N",             "RGFG95 / UTM zone 22N",                                            0 },	// RGFG95 / UTM zone 22N
	{     2973L,  2973L, "FortDesaix.UTM-20N",         "Fort Desaix / UTM zone 20N",                                       0 },	// Martinique 1938 / UTM zone 20N
	{     2975L,  2975L, "RGR92.UTM-40S",              "RGR92 / UTM zone 40S",                                             0 },	// RGR92 / UTM zone 40S
	{     2976L,  2976L, "Tahiti.UTM-6S",              "Tahiti / UTM zone 6S",                                             0 },	// Tahiti 52 / UTM zone 6S
	{     2977L,  2977L, "Tahaa.UTM-5S",               "Tahaa / UTM zone 5S",                                              0 },	// Tahaa 54 / UTM zone 5S
	{     2978L,  2978L, "IGN72NukuHiva.UTM-7S",       "IGN72 Nuku Hiva / UTM zone 7S",                                    0 },	// IGN72 Nuku Hiva / UTM zone 7S
	{     2979L,  2979L, "K01949.UTM-42S",             "K0 1949 / UTM zone 42S",                                           2 },	// K0 1949 / UTM zone 42S
	{     2980L,  2980L, "Combani50.UTM-38S",          "Combani 1950 / UTM zone 38S",                                      0 },	// Combani 1950 / UTM zone 38S
	{     2981L,  2981L, "IGN56Lifou.UTM-58S",         "IGN56 Lifou / UTM zone 58S",                                       0 },	// IGN56 Lifou / UTM zone 58S
	{     2982L,  2982L, "IGN72GrandTerre.UTM-58S",    "IGN72 Grand Terre / UTM zone 58S",                                 2 },	// IGN72 Grand Terre / UTM zone 58S
	{     2983L,  2983L, "ST87Ouvea.UTM-58S",          "ST87 Ouvea / UTM zone 58S",                                        2 },	// ST87 Ouvea / UTM zone 58S
	{     2984L,  2984L, "RGNC91.Lambert",             "RGNC 1991 / Lambert New Caledonia",                                2 },	// RGNC 1991 / Lambert New Caledonia
	{     2985L,  2985L, "Petrels72.TerreAdelie",      "Petrels 1972 / Terre Adelie Polar Stereographic",                  0 },	// Petrels 1972 / Terre Adelie Polar Stereographic
	{     2986L,  2986L, "Perroud50.TerreAdelie",      "Perroud 1950 / Terre Adelie Polar Stereographic",                  0 },	// Perroud 1950 / Terre Adelie Polar Stereographic
	{     2987L,  2987L, "Miquelon50.UTM-21N",         "Saint Pierre et Miquelon 1950 / UTM zone 21N",                     0 },	// Saint Pierre et Miquelon 1950 / UTM zone 21N
	{     2988L,  2988L, "MOP78.UTM-1S",               "MOP78 / UTM zone 1S",                                              0 },	// MOP78 / UTM zone 1S
	{     2989L,  2989L, "RRAF91.UTM-20N",             "RRAF 1991 / UTM zone 20N",                                         0 },	// RRAF 1991 / UTM zone 20N
	{     2990L,  2990L, "REUNION.TmReunion",          "Piton des Neiges / TM Reunion",                                    0 },	// Reunion 1947 / TM Reunion
	{     2991L,  2991L, "NAD83.OregonLambert",        "NAD83 / Oregon Lambert",                                           0 },	// NAD83 / Oregon Lambert
	{     2992L,  2992L, "OR83-SSCGIS",                "NAD83 / Oregon Lambert (ft)",                                      1 },	// NAD83 / Oregon Lambert (ft)
	{     2993L,  2993L, "HARN.OregonLambert",         "NAD83(HARN) / Oregon Lambert",                                     0 },	// NAD83(HARN) / Oregon Lambert
	{     2994L,  2994L, "HARN.OregonLambert.ft",      "NAD83(HARN) / Oregon Lambert (ft)",                                0 },	// NAD83(HARN) / Oregon Lambert (ft)
	{     2995L,  2995L, "IGN53/Mare.UTM-58S",         "IGN53 Mare / UTM zone 58S",                                        0 },	// IGN53 Mare / UTM zone 58S
	{     2996L,  2996L, "ST84IledesPins.UTM-58S",     "ST84 Ile des Pins / UTM zone 58S",                                 0 },	// ST84 Ile des Pins / UTM zone 58S
	{     2997L,  2997L, "ST71Belep.UTM-58S",          "ST71 Belep / UTM zone 58S",                                        0 },	// ST71 Belep / UTM zone 58S
	{     2998L,  2998L, "NEA74Noumea.UTM-58S",        "NEA74 Noumea / UTM zone 58S",                                      0 },	// NEA74 Noumea / UTM zone 58S
	{     3000L,  3000L, "Segara.NEIEZ",               "Segara / NEIEZ",                                                   0 },	// Segara / NEIEZ
	{     3001L,  3001L, "Batavia.NEIEZ",              "Batavia / NEIEZ",                                                  0 },	// Batavia / NEIEZ
	{     3002L,  3002L, "Makassar.NEIEZ",             "Makassar / NEIEZ",                                                 0 },	// Makassar / NEIEZ
	{     3003L,  3003L, "MonteMario.Italy-1",         "Monte Mario / Italy zone 1",                                       0 },	// Monte Mario / Italy zone 1
	{     3004L,  3004L, "MonteMario.Italy-2",         "Monte Mario / Italy zone 2",                                       0 },	// Monte Mario / Italy zone 2
	{     3006L,  3006L, "SWEREF 99 TM",               "SWEREF99 TM",                                                      1 },	// SWEREF99 TM
	{     3007L,  3007L, "SWEREF 99 12 00",            "SWEREF99 12 00",                                                   1 },	// SWEREF99 12 00
	{     3008L,  3008L, "SWEREF 99 13 30",            "SWEREF99 13 30",                                                   1 },	// SWEREF99 13 30
	{     3009L,  3009L, "SWEREF 99 15 00",            "SWEREF99 15 00",                                                   1 },	// SWEREF99 15 00
	{     3010L,  3010L, "SWEREF 99 16 30",            "SWEREF99 16 30",                                                   1 },	// SWEREF99 16 30
	{     3011L,  3011L, "SWEREF 99 18 00",            "SWEREF99 18 00",                                                   1 },	// SWEREF99 18 00
	{     3012L,  3012L, "SWEREF 99 14 15",            "SWEREF99 14 15",                                                   1 },	// SWEREF99 14 15
	{     3013L,  3013L, "SWEREF 99 15 45",            "SWEREF99 15 45",                                                   1 },	// SWEREF99 15 45
	{     3014L,  3014L, "SWEREF 99 17 15",            "SWEREF99 17 15",                                                   1 },	// SWEREF99 17 15
	{     3015L,  3015L, "SWEREF 99 18 45",            "SWEREF99 18 45",                                                   1 },	// SWEREF99 18 45
	{     3016L,  3016L, "SWEREF 99 20 15",            "SWEREF99 20 15",                                                   1 },	// SWEREF99 20 15
	{     3017L,  3017L, "SWEREF 99 21 45",            "SWEREF99 21 45",                                                   1 },	// SWEREF99 21 45
	{     3018L,  3018L, "SWEREF 99 23 15",            "SWEREF99 23 15",                                                   1 },	// SWEREF99 23 15
	{     3019L,  3019L, "RT90-7.5V",                  "RT90 7.5 gon V",                                                   1 },	// RT90 7.5 gon V
	{     3020L,  3020L, "RT90-5V",                    "RT90 5 gon V",                                                     1 },	// RT90 5 gon V
	{     3021L,  3021L, "RT90-2.5V",                  "RT90 2.5 gon V",                                                   1 },	// RT90 2.5 gon V
	{     3022L,  3022L, "RT90-0",                     "RT90 0 gon",                                                       1 },	// RT90 0 gon
	{     3023L,  3023L, "RT90-2.5O",                  "RT90 2.5 gon O",                                                   1 },	// RT90 2.5 gon O
	{     3024L,  3024L, "RT90-5O",                    "RT90 5 gon O",                                                     1 },	// RT90 5 gon O
	{     3033L,  3033L, "WGS84.AusAntarctic/LM",      "WGS 84 / Australian Antarctic Lambert",                            0 },	// WGS 84 / Australian Antarctic Lambert
	{     3034L,  3034L, "ETRS89.Europe/Lambert",      "ETRS89 / ETRS-LCC",                                                0 },	// ETRS89 / ETRS-LCC
	{     3036L,  3036L, "Moznet.UTM-36S",             "Moznet / UTM zone 36S",                                            0 },	// Moznet / UTM zone 36S
	{     3037L,  3037L, "Moznet.UTM-37S",             "Moznet / UTM zone 37S",                                            0 },	// Moznet / UTM zone 37S
	{     3038L,  3038L, "ETRS89.TM26",                "ETRS89 / ETRS-TM26",                                               0 },	// ETRS89 / ETRS-TM26
	{     3039L,  3039L, "ETRS89.TM27",                "ETRS89 / ETRS-TM27",                                               0 },	// ETRS89 / ETRS-TM27
	{     3040L,  3040L, "ETRS89.TM28",                "ETRS89 / ETRS-TM28",                                               0 },	// ETRS89 / ETRS-TM28
	{     3041L,  3041L, "ETRS89.TM29",                "ETRS89 / ETRS-TM29",                                               0 },	// ETRS89 / ETRS-TM29
	{     3042L,  3042L, "ETRS89.TM30",                "ETRS89 / ETRS-TM30",                                               0 },	// ETRS89 / ETRS-TM30
	{     3043L,  3043L, "ETRS89.TM31",                "ETRS89 / ETRS-TM31",                                               0 },	// ETRS89 / ETRS-TM31
	{     3044L,  3044L, "ETRS89.TM32",                "ETRS89 / ETRS-TM32",                                               0 },	// ETRS89 / ETRS-TM32
	{     3045L,  3045L, "ETRS89.TM33",                "ETRS89 / ETRS-TM33",                                               0 },	// ETRS89 / ETRS-TM33
	{     3046L,  3046L, "ETRS89.TM34",                "ETRS89 / ETRS-TM34",                                               0 },	// ETRS89 / ETRS-TM34
	{     3047L,  3047L, "ETRS89.TM35",                "ETRS89 / ETRS-TM35",                                               0 },	// ETRS89 / ETRS-TM35
	{     3048L,  3048L, "ETRS89.TM36",                "ETRS89 / ETRS-TM36",                                               0 },	// ETRS89 / ETRS-TM36
	{     3049L,  3049L, "ETRS89.TM37",                "ETRS89 / ETRS-TM37",                                               0 },	// ETRS89 / ETRS-TM37
	{     3050L,  3050L, "ETRS89.TM38",                "ETRS89 / ETRS-TM38",                                               0 },	// ETRS89 / ETRS-TM38
	{     3051L,  3051L, "ETRS89.TM39",                "ETRS89 / ETRS-TM39",                                               0 },	// ETRS89 / ETRS-TM39
	{     3052L,  3052L, "Reykjavik.IcelandGrid",      "Reykjavik 1900 / Lambert 1900",                                    1 },	// Reykjavik 1900 / Lambert 1900
	{     3053L,  3053L, "Hjorsey.IcelandGrid",        "Hjorsey 1955 / Lambert 1955",                                      1 },	// Hjorsey 1955 / Lambert 1955
	{     3054L,  3054L, "Hjorsey.UTM-26N",            "Hjorsey 1955 / UTM zone 26N",                                      0 },	// Hjorsey 1955 / UTM zone 26N
	{     3055L,  3055L, "Hjorsey.UTM-27N",            "Hjorsey 1955 / UTM zone 27N",                                      0 },	// Hjorsey 1955 / UTM zone 27N
	{     3056L,  3056L, "Hjorsey.UTM-28N",            "Hjorsey 1955 / UTM zone 28N",                                      0 },	// Hjorsey 1955 / UTM zone 28N
	{     3057L,  3057L, "ISN93.IcelandGrid",          "ISN93 / Lambert 1993",                                             1 },	// ISN93 / Lambert 1993
	{     3058L,  3058L, "Helle1954.JanMayen",         "Helle 1954 / Jan Mayen Grid",                                      0 },	// Helle 1954 / Jan Mayen Grid
	{     3059L,  3059L, "Latvia1992.TM",              "LKS92 / Latvia TM",                                                0 },	// LKS92 / Latvia TM
	{     3060L,  3060L, "IGN72/NH.UTM-58S",           "IGN72 Grande Terre / UTM zone 58S",                                0 },	// IGN72 Grande Terre / UTM zone 58S
	{     3061L,  3061L, "MADEIRA.UTM-28N",            "Porto Santo 1995 / UTM zone 28N",                                  0 },	// Porto Santo 1995 / UTM zone 28N
	{     3062L,  3062L, "AzoresEast95.UTM-26N",       "Azores Oriental 1995 / UTM zone 26N",                              0 },	// Azores Oriental 1995 / UTM zone 26N
	{     3063L,  3063L, "AzoresCntrl95.UTM-26N",      "Azores Central 1995 / UTM zone 26N",                               0 },	// Azores Central 1995 / UTM zone 26N
	{     3064L,  3064L, "IGM1995.UTM-32N",            "IGM95 / UTM zone 32N",                                             0 },	// IGM95 / UTM zone 32N
	{     3065L,  3065L, "IGM1995.UTM-33N",            "IGM95 / UTM zone 33N",                                             0 },	// IGM95 / UTM zone 33N
	{     3148L,  3148L, "Indian1960.UTM-48N",         "Indian 1960 / UTM zone 48N",                                       0 },	// Indian 1960 / UTM zone 48N
	{     3149L,  3149L, "Indian1960.UTM-49N",         "Indian 1960 / UTM zone 49N",                                       0 },	// Indian 1960 / UTM zone 49N
	{     3176L,  3176L, "Indian1960.TM-106NE",        "Indian 1960 / TM 106 NE",                                          0 },	// Indian 1960 / TM 106 NE
	{     3200L,  3200L, "Final58.Iraq",               "FD58 / Iraq zone",                                                 0 },	// FD58 / Iraq zone
	{     3300L,  3300L, "Estonia92.Estonia",          "Estonian Coordinate System of 1992",                               0 },	// Estonian Coordinate System of 1992
	{     3301L,  3301L, "Estonia97.Estonia",          "Estonian Coordinate System of 1997",                               0 },	// Estonian Coordinate System of 1997
	{     3439L,  3439L, "PSD93.UTM-39N",              "PSD93 / UTM zone 39N",                                             0 },	// PSD93 / UTM zone 39N
	{     3440L,  3440L, "PSD93.UTM-40N",              "PSD93 / UTM zone 40N",                                             0 },	// PSD93 / UTM zone 40N
	{     3561L,  3561L, "OLDHI.Hawaii-1",             "Old Hawaiian / Hawaii zone 1",                                     0 },	// Old Hawaiian / Hawaii zone 1
	{     3562L,  3562L, "OLDHI.Hawaii-2",             "Old Hawaiian / Hawaii zone 2",                                     0 },	// Old Hawaiian / Hawaii zone 2
	{     3563L,  3563L, "OLDHI.Hawaii-3",             "Old Hawaiian / Hawaii zone 3",                                     0 },	// Old Hawaiian / Hawaii zone 3
	{     3564L,  3564L, "OLDHI.Hawaii-4",             "Old Hawaiian / Hawaii zone 4",                                     0 },	// Old Hawaiian / Hawaii zone 4
	{     3565L,  3565L, "OLDHI.Hawaii-5",             "Old Hawaiian / Hawaii zone 5",                                     0 },	// Old Hawaiian / Hawaii zone 5
	{     3920L,  3920L, "PRVI.UTM-20N",               "Puerto Rico / UTM zone 20N",                                       0 },	// Puerto Rico / UTM zone 20N
	{     3991L,  3991L, "PRVI.PR-1",                  "Puerto Rico State Plane CS of 1927",                               0 },	// Puerto Rico State Plane CS of 1927
	{     3992L,  3992L, "PRVI.PR-2",                  "Puerto Rico / St. Croix",                                          0 },	// Puerto Rico / St. Croix
	{    20135L, 20135L, "Adindan.UTM-35N",            "Adindan / UTM zone 35N",                                           0 },	// Adindan / UTM zone 35N
	{    20136L, 20136L, "Adindan.UTM-36N",            "Adindan / UTM zone 36N",                                           0 },	// Adindan / UTM zone 36N
	{    20137L, 20137L, "Adindan.UTM-37N",            "Adindan / UTM zone 37N",                                           0 },	// Adindan / UTM zone 37N
	{    20138L, 20138L, "Adindan.UTM-38N",            "Adindan / UTM zone 38N",                                           0 },	// Adindan / UTM zone 38N
	{    20248L, 20248L, "AMG66-48",                   "AGD66 / AMG zone 48",                                              1 },	// AGD66 / AMG zone 48
	{    20249L, 20249L, "AMG66-49",                   "AGD66 / AMG zone 49",                                              1 },	// AGD66 / AMG zone 49
	{    20250L, 20250L, "AMG66-50",                   "AGD66 / AMG zone 50",                                              1 },	// AGD66 / AMG zone 50
	{    20251L, 20251L, "AMG66-51",                   "AGD66 / AMG zone 51",                                              1 },	// AGD66 / AMG zone 51
	{    20252L, 20252L, "AMG66-52",                   "AGD66 / AMG zone 52",                                              1 },	// AGD66 / AMG zone 52
	{    20253L, 20253L, "AMG66-53",                   "AGD66 / AMG zone 53",                                              1 },	// AGD66 / AMG zone 53
	{    20254L, 20254L, "AMG66-54",                   "AGD66 / AMG zone 54",                                              1 },	// AGD66 / AMG zone 54
	{    20255L, 20255L, "AMG66-55",                   "AGD66 / AMG zone 55",                                              1 },	// AGD66 / AMG zone 55
	{    20256L, 20256L, "AMG66-56",                   "AGD66 / AMG zone 56",                                              1 },	// AGD66 / AMG zone 56
	{    20257L, 20257L, "AMG66-57",                   "AGD66 / AMG zone 57",                                              1 },	// AGD66 / AMG zone 57
	{    20258L, 20258L, "AMG66-58",                   "AGD66 / AMG zone 58",                                              1 },	// AGD66 / AMG zone 58
	{    20348L, 20348L, "AMG84-48",                   "AGD84 / AMG zone 48",                                              1 },	// AGD84 / AMG zone 48
	{    20349L, 20349L, "AMG84-49",                   "AGD84 / AMG zone 49",                                              1 },	// AGD84 / AMG zone 49
	{    20350L, 20350L, "AMG84-50",                   "AGD84 / AMG zone 50",                                              1 },	// AGD84 / AMG zone 50
	{    20351L, 20351L, "AMG84-51",                   "AGD84 / AMG zone 51",                                              1 },	// AGD84 / AMG zone 51
	{    20352L, 20352L, "AMG84-52",                   "AGD84 / AMG zone 52",                                              1 },	// AGD84 / AMG zone 52
	{    20353L, 20353L, "AMG84-53",                   "AGD84 / AMG zone 53",                                              1 },	// AGD84 / AMG zone 53
	{    20354L, 20354L, "AMG84-54",                   "AGD84 / AMG zone 54",                                              1 },	// AGD84 / AMG zone 54
	{    20355L, 20355L, "AMG84-55",                   "AGD84 / AMG zone 55",                                              1 },	// AGD84 / AMG zone 55
	{    20356L, 20356L, "AMG84-56",                   "AGD84 / AMG zone 56",                                              1 },	// AGD84 / AMG zone 56
	{    20357L, 20357L, "AMG84-57",                   "AGD84 / AMG zone 57",                                              1 },	// AGD84 / AMG zone 57
	{    20358L, 20358L, "AMG84-58",                   "AGD84 / AMG zone 58",                                              1 },	// AGD84 / AMG zone 58
	{    20437L, 20437L, "AinElAbd.UTM-37N",           "Ain el Abd / UTM zone 37N",                                        0 },	// Ain el Abd / UTM zone 37N
	{    20438L, 20438L, "AinElAbd.UTM-38N",           "Ain el Abd / UTM zone 38N",                                        0 },	// Ain el Abd / UTM zone 38N
	{    20439L, 20439L, "AinElAbd.UTM-39N",           "Ain el Abd / UTM zone 39N",                                        0 },	// Ain el Abd / UTM zone 39N
	{    20499L, 20499L, "AinElAbd.BahrainGrid",       "Ain el Abd / Bahrain Grid",                                        0 },	// Ain el Abd / Bahrain Grid
	{    20538L, 20538L, "Afgooye.UTM-38N",            "Afgooye / UTM zone 38N",                                           0 },	// Afgooye / UTM zone 38N
	{    20539L, 20539L, "Afgooye.UTM-39N",            "Afgooye / UTM zone 39N",                                           0 },	// Afgooye / UTM zone 39N
	{    20822L, 20822L, "Aratu.UTM-22S",              "Aratu / UTM zone 22S",                                             0 },	// Aratu / UTM zone 22S
	{    20823L, 20823L, "Aratu.UTM-23S",              "Aratu / UTM zone 23S",                                             0 },	// Aratu / UTM zone 23S
	{    20824L, 20824L, "Aratu.UTM-24S",              "Aratu / UTM zone 24S",                                             0 },	// Aratu / UTM zone 24S
	{    20934L, 20934L, "ARC1950.UTM-34S",            "Arc 1950 / UTM zone 34S",                                          0 },	// Arc 1950 / UTM zone 34S
	{    20935L, 20935L, "ARC1950.UTM-35S",            "Arc 1950 / UTM zone 35S",                                          0 },	// Arc 1950 / UTM zone 35S
	{    20936L, 20936L, "ARC1950.UTM-36S",            "Arc 1950 / UTM zone 36S",                                          0 },	// Arc 1950 / UTM zone 36S
	{    21035L, 21035L, "ARC1960.UTM-35S",            "Arc 1960 / UTM zone 35S",                                          0 },	// Arc 1960 / UTM zone 35S
	{    21036L, 21036L, "ARC1960.UTM-36S",            "Arc 1960 / UTM zone 36S",                                          0 },	// Arc 1960 / UTM zone 36S
	{    21037L, 21037L, "ARC1960.UTM-37S",            "Arc 1960 / UTM zone 37S",                                          0 },	// Arc 1960 / UTM zone 37S
	{    21095L, 21095L, "ARC1960.UTM-35N",            "Arc 1960 / UTM zone 35N",                                          0 },	// Arc 1960 / UTM zone 35N
	{    21096L, 21096L, "ARC1960.UTM-36N",            "Arc 1960 / UTM zone 36N",                                          0 },	// Arc 1960 / UTM zone 36N
	{    21097L, 21097L, "ARC1960.UTM-37N",            "Arc 1960 / UTM zone 37N",                                          0 },	// Arc 1960 / UTM zone 37N
	{    21148L, 21148L, "Batavia.UTM-48S",            "Batavia / UTM zone 48S",                                           0 },	// Batavia / UTM zone 48S
	{    21149L, 21149L, "Batavia.UTM-49S",            "Batavia / UTM zone 49S",                                           0 },	// Batavia / UTM zone 49S
	{    21150L, 21150L, "Batavia.UTM-50S",            "Batavia / UTM zone 50S",                                           0 },	// Batavia / UTM zone 50S
	{    21291L, 21291L, "Barbados38.BWIgrid",         "Barbados 1938 / British West Indies Grid",                         0 },	// Barbados 1938 / British West Indies Grid
	{    21292L, 21292L, "Barbados38.NtlGrid",         "Barbados 1938 / Barbados National Grid",                           0 },	// Barbados 1938 / Barbados National Grid
	{    21413L, 21413L, "Beijing54.GK-13",            "Beijing 1954 / Gauss-Kruger zone 13",                              0 },	// Beijing 1954 / Gauss-Kruger zone 13
	{    21414L, 21414L, "Beijing54.GK-14",            "Beijing 1954 / Gauss-Kruger zone 14",                              0 },	// Beijing 1954 / Gauss-Kruger zone 14
	{    21415L, 21415L, "Beijing54.GK-15",            "Beijing 1954 / Gauss-Kruger zone 15",                              0 },	// Beijing 1954 / Gauss-Kruger zone 15
	{    21416L, 21416L, "Beijing54.GK-16",            "Beijing 1954 / Gauss-Kruger zone 16",                              0 },	// Beijing 1954 / Gauss-Kruger zone 16
	{    21417L, 21417L, "Beijing54.GK-17",            "Beijing 1954 / Gauss-Kruger zone 17",                              0 },	// Beijing 1954 / Gauss-Kruger zone 17
	{    21418L, 21418L, "Beijing54.GK-18",            "Beijing 1954 / Gauss-Kruger zone 18",                              0 },	// Beijing 1954 / Gauss-Kruger zone 18
	{    21419L, 21419L, "Beijing54.GK-19",            "Beijing 1954 / Gauss-Kruger zone 19",                              0 },	// Beijing 1954 / Gauss-Kruger zone 19
	{    21420L, 21420L, "Beijing54.GK-20",            "Beijing 1954 / Gauss-Kruger zone 20",                              0 },	// Beijing 1954 / Gauss-Kruger zone 20
	{    21421L, 21421L, "Beijing54.GK-21",            "Beijing 1954 / Gauss-Kruger zone 21",                              0 },	// Beijing 1954 / Gauss-Kruger zone 21
	{    21422L, 21422L, "Beijing54.GK-22",            "Beijing 1954 / Gauss-Kruger zone 22",                              0 },	// Beijing 1954 / Gauss-Kruger zone 22
	{    21423L, 21423L, "Beijing54.GK-23",            "Beijing 1954 / Gauss-Kruger zone 23",                              0 },	// Beijing 1954 / Gauss-Kruger zone 23
	{    21453L, 21453L, "Beijing54.GK/CM-75E",        "Beijing 1954 / Gauss-Kruger CM 75E",                               0 },	// Beijing 1954 / Gauss-Kruger CM 75E
	{    21454L, 21454L, "Beijing54.GK/CM-81E",        "Beijing 1954 / Gauss-Kruger CM 81E",                               0 },	// Beijing 1954 / Gauss-Kruger CM 81E
	{    21455L, 21455L, "Beijing54.GK/CM-87E",        "Beijing 1954 / Gauss-Kruger CM 87E",                               0 },	// Beijing 1954 / Gauss-Kruger CM 87E
	{    21456L, 21456L, "Beijing54.GK/CM-93E",        "Beijing 1954 / Gauss-Kruger CM 93E",                               0 },	// Beijing 1954 / Gauss-Kruger CM 93E
	{    21457L, 21457L, "Beijing54.GK/CM-99E",        "Beijing 1954 / Gauss-Kruger CM 99E",                               0 },	// Beijing 1954 / Gauss-Kruger CM 99E
	{    21458L, 21458L, "Beijing54.GK/CM-105E",       "Beijing 1954 / Gauss-Kruger CM 105E",                              0 },	// Beijing 1954 / Gauss-Kruger CM 105E
	{    21459L, 21459L, "Beijing54.GK/CM-111E",       "Beijing 1954 / Gauss-Kruger CM 111E",                              0 },	// Beijing 1954 / Gauss-Kruger CM 111E
	{    21460L, 21460L, "Beijing54.GK/CM-117E",       "Beijing 1954 / Gauss-Kruger CM 117E",                              0 },	// Beijing 1954 / Gauss-Kruger CM 117E
	{    21461L, 21461L, "Beijing54.GK/CM-123E",       "Beijing 1954 / Gauss-Kruger CM 123E",                              0 },	// Beijing 1954 / Gauss-Kruger CM 123E
	{    21462L, 21462L, "Beijing54.GK/CM-129E",       "Beijing 1954 / Gauss-Kruger CM 129E",                              0 },	// Beijing 1954 / Gauss-Kruger CM 129E
	{    21463L, 21463L, "Beijing54.GK/CM-135E",       "Beijing 1954 / Gauss-Kruger CM 135E",                              0 },	// Beijing 1954 / Gauss-Kruger CM 135E
	{    21473L, 21473L, "Beijing54.GK-13N",           "Beijing 1954 / Gauss-Kruger 13N",                                  2 },	// Beijing 1954 / Gauss-Kruger 13N
	{    21474L, 21474L, "Beijing54.GK-14N",           "Beijing 1954 / Gauss-Kruger 14N",                                  2 },	// Beijing 1954 / Gauss-Kruger 14N
	{    21475L, 21475L, "Beijing54.GK-15N",           "Beijing 1954 / Gauss-Kruger 15N",                                  2 },	// Beijing 1954 / Gauss-Kruger 15N
	{    21476L, 21476L, "Beijing54.GK-16N",           "Beijing 1954 / Gauss-Kruger 16N",                                  2 },	// Beijing 1954 / Gauss-Kruger 16N
	{    21477L, 21477L, "Beijing54.GK-17N",           "Beijing 1954 / Gauss-Kruger 17N",                                  2 },	// Beijing 1954 / Gauss-Kruger 17N
	{    21478L, 21478L, "Beijing54.GK-18N",           "Beijing 1954 / Gauss-Kruger 18N",                                  2 },	// Beijing 1954 / Gauss-Kruger 18N
	{    21479L, 21479L, "Beijing54.GK-19N",           "Beijing 1954 / Gauss-Kruger 19N",                                  2 },	// Beijing 1954 / Gauss-Kruger 19N
	{    21480L, 21480L, "Beijing54.GK-20N",           "Beijing 1954 / Gauss-Kruger 20N",                                  2 },	// Beijing 1954 / Gauss-Kruger 20N
	{    21481L, 21481L, "Beijing54.GK-21N",           "Beijing 1954 / Gauss-Kruger 21N",                                  2 },	// Beijing 1954 / Gauss-Kruger 21N
	{    21482L, 21482L, "Beijing54.GK-22N",           "Beijing 1954 / Gauss-Kruger 22N",                                  2 },	// Beijing 1954 / Gauss-Kruger 22N
	{    21483L, 21483L, "Beijing54.GK-23N",           "Beijing 1954 / Gauss-Kruger 23N",                                  2 },	// Beijing 1954 / Gauss-Kruger 23N
	{    21817L, 21817L, "BOGOTA.UTM-17N",             "Bogota 1975 / UTM zone 17N",                                       2 },	// Bogota 1975 / UTM zone 17N
	{    21818L, 21818L, "BOGOTA.UTM-18N",             "Bogota 1975 / UTM zone 18N",                                       0 },	// Bogota 1975 / UTM zone 18N
	{    21891L, 21896L, "BOGOTA.Colombia-W",          "Bogota 1975 / Colombia West zone",                                 0 },	// Bogota 1975 / Colombia West zone
	{    21892L, 21897L, "BOGOTA.Colombia-Bogota",     "Bogota 1975 / Colombia Bogota zone",                               0 },	// Bogota 1975 / Colombia Bogota zone
	{    21893L, 21898L, "BOGOTA.Colombia-EC",         "Bogota 1975 / Colombia East Central zone",                         0 },	// Bogota 1975 / Colombia East Central zone
	{    21894L, 21899L, "BOGOTA.ColombiaE",           "Bogota 1975 / Colombia East",                                      0 },	// Bogota 1975 / Colombia East
	{    22032L, 22032L, "Camacupa.UTM-32S",           "Camacupa / UTM zone 32S",                                          0 },	// Camacupa / UTM zone 32S
	{    22033L, 22033L, "Camacupa.UTM-33S",           "Camacupa / UTM zone 33S",                                          0 },	// Camacupa / UTM zone 33S
	{    22091L, 22091L, "Camacupa.TM-1130SE",         "Camacupa / TM 11.30 SE",                                           0 },	// Camacupa / TM 11.30 SE
	{    22092L, 22092L, "Camacupa.TM-12SE",           "Camacupa / TM 12 SE",                                              0 },	// Camacupa / TM 12 SE
	{    22191L, 22191L, "Campo.Argentina 1",          "Campo Inchauspe / Argentina 1",                                    0 },	// Campo Inchauspe / Argentina 1
	{    22192L, 22192L, "Campo.Argentina 2",          "Campo Inchauspe / Argentina 2",                                    0 },	// Campo Inchauspe / Argentina 2
	{    22193L, 22193L, "Campo.Argentina 3",          "Campo Inchauspe / Argentina 3",                                    0 },	// Campo Inchauspe / Argentina 3
	{    22194L, 22194L, "Campo.Argentina 4",          "Campo Inchauspe / Argentina 4",                                    0 },	// Campo Inchauspe / Argentina 4
	{    22195L, 22195L, "Campo.Argentina 5",          "Campo Inchauspe / Argentina 5",                                    0 },	// Campo Inchauspe / Argentina 5
	{    22196L, 22196L, "Campo.Argentina 6",          "Campo Inchauspe / Argentina 6",                                    0 },	// Campo Inchauspe / Argentina 6
	{    22197L, 22197L, "Campo.Argentina 7",          "Campo Inchauspe / Argentina 7",                                    0 },	// Campo Inchauspe / Argentina 7
	{    22234L, 22234L, "Cape-1.UTM-34S",             "Cape / UTM zone 34S",                                              0 },	// Cape / UTM zone 34S
	{    22235L, 22235L, "Cape-1.UTM-35S",             "Cape / UTM zone 35S",                                              0 },	// Cape / UTM zone 35S
	{    22236L, 22236L, "Cape-1.UTM-36S",             "Cape / UTM zone 36S",                                              0 },	// Cape / UTM zone 36S
	{    22275L, 22275L, "Cape-1.SACS-15",             "South African Coordinate System zone 15",                          0 },	// South African Coordinate System zone 15
	{    22277L, 22277L, "Cape-1.SACS-17",             "South African Coordinate System zone 17",                          0 },	// South African Coordinate System zone 17
	{    22279L, 22279L, "Cape-1.SACS-19",             "South African Coordinate System zone 19",                          0 },	// South African Coordinate System zone 19
	{    22281L, 22281L, "Cape-1.SACS-21",             "South African Coordinate System zone 21",                          0 },	// South African Coordinate System zone 21
	{    22283L, 22283L, "Cape-1.SACS-23",             "South African Coordinate System zone 23",                          0 },	// South African Coordinate System zone 23
	{    22285L, 22285L, "Cape-1.SACS-25",             "South African Coordinate System zone 25",                          0 },	// South African Coordinate System zone 25
	{    22287L, 22287L, "Cape-1.SACS-27",             "South African Coordinate System zone 27",                          0 },	// South African Coordinate System zone 27
	{    22289L, 22289L, "Cape-1.SACS-29",             "South African Coordinate System zone 29",                          0 },	// South African Coordinate System zone 29
	{    22291L, 22291L, "Cape-1.SACS-31",             "South African Coordinate System zone 31",                          0 },	// South African Coordinate System zone 31
	{    22293L, 22293L, "Cape-1.SACS-33",             "South African Coordinate System zone 33",                          0 },	// South African Coordinate System zone 33
	{    22332L, 22332L, "Carthage.UTM-32N",           "Carthage / UTM zone 32N",                                          0 },	// Carthage / UTM zone 32N
	{    22391L, 22391L, "Carthage.NordTunisie",       "Carthage / Nord Tunisie",                                          0 },	// Carthage / Nord Tunisie
	{    22392L, 22392L, "Carthage.SudTunisie",        "Carthage / Sud Tunisie",                                           0 },	// Carthage / Sud Tunisie
	{    22523L, 22523L, "Corrego.UTM-23S",            "Corrego Alegre / UTM zone 23S",                                    0 },	// Corrego Alegre / UTM zone 23S
	{    22524L, 22524L, "Corrego.UTM-24S",            "Corrego Alegre / UTM zone 24S",                                    0 },	// Corrego Alegre / UTM zone 24S
	{    22700L, 22700L, "DeirEzZor.Levant",           "Deir ez Zor / Levant Zone",                                        0 },	// Deir ez Zor / Levant Zone
	{    22770L, 22770L, "DeirEzZor.Syria",            "Deir ez Zor / Syria Lambert",                                      0 },	// Deir ez Zor / Syria Lambert
	{    22780L, 22780L, "DeirEzZor.Levant",           "Deir ez Zor / Levant Stereographic",                               0 },	// Deir ez Zor / Levant Stereographic
	{    22991L, 22991L, "Old-Egyp.Blue",              "Egypt 1907 / Blue Belt",                                           0 },	// Egypt 1907 / Blue Belt
	{    22992L, 22992L, "Old-Egyp.Red",               "Egypt 1907 / Red Belt",                                            0 },	// Egypt 1907 / Red Belt
	{    22993L, 22993L, "Old-Egyp.Purple",            "Egypt 1907 / Purple Belt",                                         0 },	// Egypt 1907 / Purple Belt
	{    22994L, 22994L, "Old-Egyp.ExPurple",          "Egypt 1907 / Extended Purple Belt",                                0 },	// Egypt 1907 / Extended Purple Belt
	{    23028L, 23028L, "ED50-UTM28",                 "ED50 / UTM zone 28N",                                              1 },	// ED50 / UTM zone 28N
	{    23029L, 23029L, "ED50-UTM29",                 "ED50 / UTM zone 29N",                                              1 },	// ED50 / UTM zone 29N
	{    23030L, 23030L, "ED50-UTM30",                 "ED50 / UTM zone 30N",                                              1 },	// ED50 / UTM zone 30N
	{    23031L, 23031L, "ED50-UTM31",                 "ED50 / UTM zone 31N",                                              1 },	// ED50 / UTM zone 31N
	{    23032L, 23032L, "ED50-UTM32",                 "ED50 / UTM zone 32N",                                              1 },	// ED50 / UTM zone 32N
	{    23033L, 23033L, "ED50-UTM33",                 "ED50 / UTM zone 33N",                                              1 },	// ED50 / UTM zone 33N
	{    23034L, 23034L, "ED50-UTM34",                 "ED50 / UTM zone 34N",                                              1 },	// ED50 / UTM zone 34N
	{    23035L, 23035L, "ED50-UTM35",                 "ED50 / UTM zone 35N",                                              1 },	// ED50 / UTM zone 35N
	{    23036L, 23036L, "ED50-UTM36",                 "ED50 / UTM zone 36N",                                              1 },	// ED50 / UTM zone 36N
	{    23037L, 23037L, "ED50-UTM37",                 "ED50 / UTM zone 37N",                                              1 },	// ED50 / UTM zone 37N
	{    23038L, 23038L, "ED50-UTM38",                 "ED50 / UTM zone 38N",                                              1 },	// ED50 / UTM zone 38N
	{    23090L, 23090L, "Europ50.TM-0N",              "ED50 / TM 0 N",                                                    0 },	// ED50 / TM 0 N
	{    23095L, 23095L, "Europ50.TM-5NE",             "ED50 / TM 5 NE",                                                   0 },	// ED50 / TM 5 NE
	{    23239L, 23239L, "Fahud.UTM-39N",              "Fahud / UTM zone 39N",                                             0 },	// Fahud / UTM zone 39N
	{    23240L, 23240L, "Fahud.UTM-40N",              "Fahud / UTM zone 40N",                                             0 },	// Fahud / UTM zone 40N
	{    23700L, 23700L, "HU-EOV72",                   "HD72 / EOV",                                                       1 },	// HD72 / EOV
	{    23846L, 23846L, "Indonesian74.UTM-46N",       "ID74 / UTM zone 46N",                                              0 },	// ID74 / UTM zone 46N
	{    23847L, 23847L, "Indonesian74.UTM-47N",       "ID74 / UTM zone 47N",                                              0 },	// ID74 / UTM zone 47N
	{    23848L, 23848L, "Indonesian74.UTM-48N",       "ID74 / UTM zone 48N",                                              0 },	// ID74 / UTM zone 48N
	{    23849L, 23849L, "Indonesian74.UTM-49N",       "ID74 / UTM zone 49N",                                              0 },	// ID74 / UTM zone 49N
	{    23850L, 23850L, "Indonesian74.UTM-50N",       "ID74 / UTM zone 50N",                                              0 },	// ID74 / UTM zone 50N
	{    23851L, 23851L, "Indonesian74.UTM-51N",       "ID74 / UTM zone 51N",                                              0 },	// ID74 / UTM zone 51N
	{    23852L, 23852L, "Indonesian74.UTM-52N",       "ID74 / UTM zone 52N",                                              0 },	// ID74 / UTM zone 52N
	{    23853L, 23853L, "Indonesian74.UTM-53N",       "ID74 / UTM zone 53N",                                              2 },	// ID74 / UTM zone 53N
	{    23886L, 23886L, "Indonesian74.UTM-46S",       "ID74 / UTM zone 46S",                                              2 },	// ID74 / UTM zone 46S
	{    23887L, 23887L, "Indonesian74.UTM-47S",       "ID74 / UTM zone 47S",                                              0 },	// ID74 / UTM zone 47S
	{    23888L, 23888L, "Indonesian74.UTM-48S",       "ID74 / UTM zone 48S",                                              0 },	// ID74 / UTM zone 48S
	{    23889L, 23889L, "Indonesian74.UTM-49S",       "ID74 / UTM zone 49S",                                              0 },	// ID74 / UTM zone 49S
	{    23890L, 23890L, "Indonesian74.UTM-50S",       "ID74 / UTM zone 50S",                                              0 },	// ID74 / UTM zone 50S
	{    23891L, 23891L, "Indonesian74.UTM-51S",       "ID74 / UTM zone 51S",                                              0 },	// ID74 / UTM zone 51S
	{    23892L, 23892L, "Indonesian74.UTM-52S",       "ID74 / UTM zone 52S",                                              0 },	// ID74 / UTM zone 52S
	{    23893L, 23893L, "Indonesian74.UTM-53S",       "ID74 / UTM zone 53S",                                              0 },	// ID74 / UTM zone 53S
	{    23894L, 23894L, "Indonesian74.UTM-54S",       "ID74 / UTM zone 54S",                                              0 },	// ID74 / UTM zone 54S
	{    23946L, 23946L, "Indian54.UTM-46N",           "Indian 1954 / UTM zone 46N",                                       0 },	// Indian 1954 / UTM zone 46N
	{    23947L, 23947L, "Indian54.UTM-47N",           "Indian 1954 / UTM zone 47N",                                       0 },	// Indian 1954 / UTM zone 47N
	{    23948L, 23948L, "Indian54.UTM-48N",           "Indian 1954 / UTM zone 48N",                                       0 },	// Indian 1954 / UTM zone 48N
	{    24047L, 24047L, "Indian75.UTM-47N",           "Indian 1975 / UTM zone 47N",                                       0 },	// Indian 1975 / UTM zone 47N
	{    24048L, 24048L, "Indian75.UTM-48N",           "Indian 1975 / UTM zone 48N",                                       0 },	// Indian 1975 / UTM zone 48N
	{    24200L, 24200L, "Jamaica69.NtlGrid",          "JAD69 / Jamaica National Grid",                                    0 },	// JAD69 / Jamaica National Grid
	{    24305L, 24305L, "Kalianpur37.UTM-45N",        "Kalianpur 1937 / UTM zone 45N",                                    0 },	// Kalianpur 1937 / UTM zone 45N
	{    24306L, 24306L, "Kalianpur37.UTM-46N",        "Kalianpur 1937 / UTM zone 46N",                                    0 },	// Kalianpur 1937 / UTM zone 46N
	{    24311L, 24311L, "Kalianpur62.UTM-41N",        "Kalianpur 1962 / UTM zone 41N",                                    0 },	// Kalianpur 1962 / UTM zone 41N
	{    24312L, 24312L, "Kalianpur62.UTM-42N",        "Kalianpur 1962 / UTM zone 42N",                                    0 },	// Kalianpur 1962 / UTM zone 42N
	{    24313L, 24313L, "Kalianpur62.UTM-43N",        "Kalianpur 1962 / UTM zone 43N",                                    0 },	// Kalianpur 1962 / UTM zone 43N
	{    24342L, 24342L, "Kalianpur75.UTM-42N",        "Kalianpur 1975 / UTM zone 42N",                                    0 },	// Kalianpur 1975 / UTM zone 42N
	{    24343L, 24343L, "Kalianpur75.UTM-43N",        "Kalianpur 1975 / UTM zone 43N",                                    0 },	// Kalianpur 1975 / UTM zone 43N
	{    24344L, 24344L, "Kalianpur75.UTM-44N",        "Kalianpur 1975 / UTM zone 44N",                                    0 },	// Kalianpur 1975 / UTM zone 44N
	{    24345L, 24345L, "Kalianpur75.UTM-45N",        "Kalianpur 1975 / UTM zone 45N",                                    0 },	// Kalianpur 1975 / UTM zone 45N
	{    24346L, 24346L, "Kalianpur75.UTM-46N",        "Kalianpur 1975 / UTM zone 46N",                                    0 },	// Kalianpur 1975 / UTM zone 46N
	{    24347L, 24347L, "Kalianpur75.UTM-47N",        "Kalianpur 1975 / UTM zone 47N",                                    0 },	// Kalianpur 1975 / UTM zone 47N
	{    24375L, 24375L, "Kalianpur37.India-IIb",      "Kalianpur 1937 / India zone IIb",                                  0 },	// Kalianpur 1937 / India zone IIb
	{    24376L, 24376L, "Kalianpur62.India-I",        "Kalianpur 1962 / India zone I",                                    0 },	// Kalianpur 1962 / India zone I
	{    24377L, 24377L, "Kalianpur62.India-IIa",      "Kalianpur 1962 / India zone IIa",                                  0 },	// Kalianpur 1962 / India zone IIa
	{    24378L, 24378L, "Kalianpur75.India-I",        "Kalianpur 1975 / India zone I",                                    0 },	// Kalianpur 1975 / India zone I
	{    24379L, 24379L, "Kalianpur75.India-IIa",      "Kalianpur 1975 / India zone IIa",                                  0 },	// Kalianpur 1975 / India zone IIa
	{    24380L, 24380L, "Kalianpur75.India-IIb",      "Kalianpur 1975 / India zone IIb",                                  0 },	// Kalianpur 1975 / India zone IIb
	{    24381L, 24381L, "Kalianpur75.India-III",      "Kalianpur 1975 / India zone III",                                  0 },	// Kalianpur 1975 / India zone III
	{    24383L, 24383L, "Kalianpur75.India-IV",       "Kalianpur 1975 / India zone IV",                                   0 },	// Kalianpur 1975 / India zone IV
	{    24571L, 24571L, "Kertau.MalayaRSO",           "Kertau / R.S.O. Malaya (ch)",                                      2 },	// Kertau / R.S.O. Malaya (ch)
	{    24600L, 24600L, "KuwaitOilCo.Lambert",        "KOC Lambert",                                                      0 },	// KOC Lambert
	{    24718L, 24718L, "LaCanoa.UTM-18N",            "La Canoa / UTM zone 18N",                                          0 },	// La Canoa / UTM zone 18N
	{    24719L, 24719L, "LaCanoa.UTM-19N",            "La Canoa / UTM zone 19N",                                          0 },	// La Canoa / UTM zone 19N
	{    24720L, 24720L, "LaCanoa.UTM-20N",            "La Canoa / UTM zone 20N",                                          0 },	// La Canoa / UTM zone 20N
	{    24818L, 24818L, "UTM56-18N",                  "PSAD56 / UTM zone 18N",                                            1 },	// PSAD56 / UTM zone 18N
	{    24819L, 24819L, "UTM56-19N",                  "PSAD56 / UTM zone 19N",                                            1 },	// PSAD56 / UTM zone 19N
	{    24820L, 24820L, "UTM56-20N",                  "PSAD56 / UTM zone 20N",                                            1 },	// PSAD56 / UTM zone 20N
	{    24821L, 24821L, "UTM56-21N",                  "PSAD56 / UTM zone 21N",                                            1 },	// PSAD56 / UTM zone 21N
	{    24877L, 24877L, "UTM56-17S",                  "PSAD56 / UTM zone 17S",                                            1 },	// PSAD56 / UTM zone 17S
	{    24878L, 24878L, "UTM56-18S",                  "PSAD56 / UTM zone 18S",                                            1 },	// PSAD56 / UTM zone 18S
	{    24879L, 24879L, "UTM56-19S",                  "PSAD56 / UTM zone 19S",                                            1 },	// PSAD56 / UTM zone 19S
	{    24880L, 24880L, "UTM56-20S",                  "PSAD56 / UTM zone 20S",                                            1 },	// PSAD56 / UTM zone 20S
	{    24882L, 24882L, "UTM56-22S",                  "PSAD56 / UTM zone 22S",                                            1 },	// PSAD56 / UTM zone 22S
	{    24891L, 24891L, "PSAD56.PeruWest",            "PSAD56 / Peru west zone",                                          0 },	// PSAD56 / Peru west zone
	{    24892L, 24892L, "PSAD56.PeruCentral",         "PSAD56 / Peru central zone",                                       0 },	// PSAD56 / Peru central zone
	{    24893L, 24893L, "PSAD56.PeruEast",            "PSAD56 / Peru east zone",                                          0 },	// PSAD56 / Peru east zone
	{    25000L, 25000L, "Leigon.GhanaMetreGrid",      "Leigon / Ghana Metre Grid",                                        0 },	// Leigon / Ghana Metre Grid
	{    25391L, 25391L, "Luzon.Philippines-I",        "Luzon 1911 / Philippines zone I",                                  0 },	// Luzon 1911 / Philippines zone I
	{    25392L, 25392L, "Luzon.Philippines-II",       "Luzon 1911 / Philippines zone II",                                 0 },	// Luzon 1911 / Philippines zone II
	{    25393L, 25393L, "Luzon.Philippines-III",      "Luzon 1911 / Philippines zone III",                                0 },	// Luzon 1911 / Philippines zone III
	{    25394L, 25394L, "Luzon.Philippines-IV",       "Luzon 1911 / Philippines zone IV",                                 0 },	// Luzon 1911 / Philippines zone IV
	{    25395L, 25395L, "Luzon.Philippines-V",        "Luzon 1911 / Philippines zone V",                                  0 },	// Luzon 1911 / Philippines zone V
	{    25828L, 25828L, "ETRS89.UTM-28N",             "ETRS89 / UTM zone 28N",                                            0 },	// ETRS89 / UTM zone 28N
	{    25829L, 25829L, "ETRS89.UTM-29N",             "ETRS89 / UTM zone 29N",                                            0 },	// ETRS89 / UTM zone 29N
	{    25830L, 25830L, "ETRS89.UTM-30N",             "ETRS89 / UTM zone 30N",                                            0 },	// ETRS89 / UTM zone 30N
	{    25831L, 25831L, "ETRS89.UTM-31N",             "ETRS89 / UTM zone 31N",                                            0 },	// ETRS89 / UTM zone 31N
	{    25832L, 25832L, "ETRS89.UTM-32N",             "ETRS89 / UTM zone 32N",                                            0 },	// ETRS89 / UTM zone 32N
	{    25833L, 25833L, "ETRS89.UTM-33N",             "ETRS89 / UTM zone 33N",                                            0 },	// ETRS89 / UTM zone 33N
	{    25834L, 25834L, "ETRS89.UTM-34N",             "ETRS89 / UTM zone 34N",                                            0 },	// ETRS89 / UTM zone 34N
	{    25835L, 25835L, "ETRS89.UTM-35N",             "ETRS89 / UTM zone 35N",                                            0 },	// ETRS89 / UTM zone 35N
	{    25836L, 25836L, "ETRS89.UTM-36N",             "ETRS89 / UTM zone 36N",                                            0 },	// ETRS89 / UTM zone 36N
	{    25837L, 25837L, "ETRS89.UTM-37N",             "ETRS89 / UTM zone 37N",                                            0 },	// ETRS89 / UTM zone 37N
	{    25838L, 25838L, "ETRS89.UTM-38N",             "ETRS89 / UTM zone 38N",                                            0 },	// ETRS89 / UTM zone 38N
	{    25884L, 25884L, "ETRS89.TM-Baltic",           "ETRS89 / TM Baltic93",                                             0 },	// ETRS89 / TM Baltic93
	{    25932L, 25932L, "Malongo87.UTM-32S",          "Malongo 1987 / UTM zone 32S",                                      0 },	// Malongo 1987 / UTM zone 32S
	{    26191L, 26191L, "Merchich.NordMaroc",         "Merchich / Nord Maroc",                                            0 },	// Merchich / Nord Maroc
	{    26192L, 26192L, "Merchich.SudMaroc",          "Merchich / Sud Maroc",                                             0 },	// Merchich / Sud Maroc
	{    26193L, 26193L, "Merchich.Sahara",            "Merchich / Sahara",                                                2 },	// Merchich / Sahara
	{    26194L, 26194L, "Merchich.SaharaNord",        "Merchich / Sahara Nord",                                           0 },	// Merchich / Sahara Nord
	{    26195L, 26195L, "Merchich.SaharaSud",         "Merchich / Sahara Sud",                                            0 },	// Merchich / Sahara Sud
	{    26237L, 26237L, "Massawa.UTM-37N",            "Massawa / UTM zone 37N",                                           0 },	// Massawa / UTM zone 37N
	{    26331L, 26331L, "Minna.UTM-31N",              "Minna / UTM zone 31N",                                             0 },	// Minna / UTM zone 31N
	{    26332L, 26332L, "Minna.UTM-32N",              "Minna / UTM zone 32N",                                             0 },	// Minna / UTM zone 32N
	{    26391L, 26391L, "Minna.NigeriaWest",          "Minna / Nigeria West Belt",                                        0 },	// Minna / Nigeria West Belt
	{    26392L, 26392L, "Minna.NigeriaMid",           "Minna / Nigeria Mid Belt",                                         0 },	// Minna / Nigeria Mid Belt
	{    26393L, 26393L, "Minna.NigeriaEast",          "Minna / Nigeria East Belt",                                        0 },	// Minna / Nigeria East Belt
	{    26432L, 26432L, "Mhast.UTM-32S",              "Mhast / UTM zone 32S",                                             2 },	// Mhast / UTM zone 32S
	{    26591L, 26591L, "MonteMario.Italy-1",         "Monte Mario (Rome) / Italy zone 1",                                2 },	// Monte Mario (Rome) / Italy zone 1
	{    26592L, 26592L, "MonteMario.Italy-2",         "Monte Mario (Rome) / Italy zone 2",                                2 },	// Monte Mario (Rome) / Italy zone 2
	{    26632L, 26632L, "Mporaloko.UTM-32N",          "M'poraloko / UTM zone 32N",                                        0 },	// M'poraloko / UTM zone 32N
	{    26692L, 26692L, "Mporaloko.UTM-32S",          "M'poraloko / UTM zone 32S",                                        0 },	// M'poraloko / UTM zone 32S
	{    26703L, 26703L, "UTM27-3",                    "NAD27 / UTM zone 3N",                                              1 },	// NAD27 / UTM zone 3N
	{    26704L, 26704L, "UTM27-4",                    "NAD27 / UTM zone 4N",                                              1 },	// NAD27 / UTM zone 4N
	{    26705L, 26705L, "UTM27-5",                    "NAD27 / UTM zone 5N",                                              1 },	// NAD27 / UTM zone 5N
	{    26706L, 26706L, "UTM27-6",                    "NAD27 / UTM zone 6N",                                              1 },	// NAD27 / UTM zone 6N
	{    26707L, 26707L, "UTM27-7",                    "NAD27 / UTM zone 7N",                                              1 },	// NAD27 / UTM zone 7N
	{    26708L, 26708L, "UTM27-8",                    "NAD27 / UTM zone 8N",                                              1 },	// NAD27 / UTM zone 8N
	{    26709L, 26709L, "UTM27-9",                    "NAD27 / UTM zone 9N",                                              1 },	// NAD27 / UTM zone 9N
	{    26710L, 26710L, "UTM27-10",                   "NAD27 / UTM zone 10N",                                             1 },	// NAD27 / UTM zone 10N
	{    26711L, 26711L, "UTM27-11",                   "NAD27 / UTM zone 11N",                                             1 },	// NAD27 / UTM zone 11N
	{    26712L, 26712L, "UTM27-12",                   "NAD27 / UTM zone 12N",                                             1 },	// NAD27 / UTM zone 12N
	{    26713L, 26713L, "UTM27-13",                   "NAD27 / UTM zone 13N",                                             1 },	// NAD27 / UTM zone 13N
	{    26714L, 26714L, "UTM27-14",                   "NAD27 / UTM zone 14N",                                             1 },	// NAD27 / UTM zone 14N
	{    26715L, 26715L, "UTM27-15",                   "NAD27 / UTM zone 15N",                                             1 },	// NAD27 / UTM zone 15N
	{    26716L, 26716L, "UTM27-16",                   "NAD27 / UTM zone 16N",                                             1 },	// NAD27 / UTM zone 16N
	{    26717L, 26717L, "UTM27-17",                   "NAD27 / UTM zone 17N",                                             1 },	// NAD27 / UTM zone 17N
	{    26718L, 26718L, "UTM27-18",                   "NAD27 / UTM zone 18N",                                             1 },	// NAD27 / UTM zone 18N
	{    26719L, 26719L, "UTM27-19",                   "NAD27 / UTM zone 19N",                                             1 },	// NAD27 / UTM zone 19N
	{    26720L, 26720L, "UTM27-20",                   "NAD27 / UTM zone 20N",                                             1 },	// NAD27 / UTM zone 20N
	{    26721L, 26721L, "UTM27-21",                   "NAD27 / UTM zone 21N",                                             1 },	// NAD27 / UTM zone 21N
	{    26722L, 26722L, "UTM27-22",                   "NAD27 / UTM zone 22N",                                             1 },	// NAD27 / UTM zone 22N
	{    26729L, 26729L, "AL-E",                       "NAD27 / Alabama East",                                             1 },	// NAD27 / Alabama East
	{    26730L, 26730L, "AL-W",                       "NAD27 / Alabama West",                                             1 },	// NAD27 / Alabama West
	{    26731L, 26731L, "AK-1",                       "NAD27 / Alaska zone 1",                                            1 },	// NAD27 / Alaska zone 1
	{    26732L, 26732L, "AK-2",                       "NAD27 / Alaska zone 2",                                            1 },	// NAD27 / Alaska zone 2
	{    26733L, 26733L, "AK-3",                       "NAD27 / Alaska zone 3",                                            1 },	// NAD27 / Alaska zone 3
	{    26734L, 26734L, "AK-4",                       "NAD27 / Alaska zone 4",                                            1 },	// NAD27 / Alaska zone 4
	{    26735L, 26735L, "AK-5",                       "NAD27 / Alaska zone 5",                                            1 },	// NAD27 / Alaska zone 5
	{    26736L, 26736L, "AK-6",                       "NAD27 / Alaska zone 6",                                            1 },	// NAD27 / Alaska zone 6
	{    26737L, 26737L, "AK-7",                       "NAD27 / Alaska zone 7",                                            1 },	// NAD27 / Alaska zone 7
	{    26738L, 26738L, "AK-8",                       "NAD27 / Alaska zone 8",                                            1 },	// NAD27 / Alaska zone 8
	{    26739L, 26739L, "AK-9",                       "NAD27 / Alaska zone 9",                                            1 },	// NAD27 / Alaska zone 9
	{    26740L, 26740L, "AK-10",                      "NAD27 / Alaska zone 10",                                           1 },	// NAD27 / Alaska zone 10
	{    26741L, 26741L, "CA-I",                       "NAD27 / California zone I",                                        1 },	// NAD27 / California zone I
	{    26742L, 26742L, "CA-II",                      "NAD27 / California zone II",                                       1 },	// NAD27 / California zone II
	{    26743L, 26743L, "CA-III",                     "NAD27 / California zone III",                                      1 },	// NAD27 / California zone III
	{    26744L, 26744L, "CA-IV",                      "NAD27 / California zone IV",                                       1 },	// NAD27 / California zone IV
	{    26745L, 26745L, "CA-V",                       "NAD27 / California zone V",                                        1 },	// NAD27 / California zone V
	{    26746L, 26746L, "CA-VI",                      "NAD27 / California zone VI",                                       1 },	// NAD27 / California zone VI
	{    26747L, 26747L, "CA-VII",                     "NAD27 / California zone VII",                                      3 },	// NAD27 / California zone VII
	{    26748L, 26748L, "AZ-E",                       "NAD27 / Arizona East",                                             1 },	// NAD27 / Arizona East
	{    26749L, 26749L, "AZ-C",                       "NAD27 / Arizona Central",                                          1 },	// NAD27 / Arizona Central
	{    26750L, 26750L, "AZ-W",                       "NAD27 / Arizona West",                                             1 },	// NAD27 / Arizona West
	{    26751L, 26751L, "AR-N",                       "NAD27 / Arkansas North",                                           1 },	// NAD27 / Arkansas North
	{    26752L, 26752L, "AR-S",                       "NAD27 / Arkansas South",                                           1 },	// NAD27 / Arkansas South
	{    26753L, 26753L, "CO-N",                       "NAD27 / Colorado North",                                           1 },	// NAD27 / Colorado North
	{    26754L, 26754L, "CO-C",                       "NAD27 / Colorado Central",                                         1 },	// NAD27 / Colorado Central
	{    26755L, 26755L, "CO-S",                       "NAD27 / Colorado South",                                           1 },	// NAD27 / Colorado South
	{    26756L, 26756L, "CT",                         "NAD27 / Connecticut",                                              1 },	// NAD27 / Connecticut
	{    26757L, 26757L, "DE",                         "NAD27 / Delaware",                                                 1 },	// NAD27 / Delaware
	{    26758L, 26758L, "FL-E",                       "NAD27 / Florida East",                                             1 },	// NAD27 / Florida East
	{    26759L, 26759L, "FL-W",                       "NAD27 / Florida West",                                             1 },	// NAD27 / Florida West
	{    26760L, 26760L, "FL-N",                       "NAD27 / Florida North",                                            1 },	// NAD27 / Florida North
	{    26766L, 26766L, "GA-E",                       "NAD27 / Georgia East",                                             1 },	// NAD27 / Georgia East
	{    26767L, 26767L, "GA-W",                       "NAD27 / Georgia West",                                             1 },	// NAD27 / Georgia West
	{    26768L, 26768L, "ID-E",                       "NAD27 / Idaho East",                                               1 },	// NAD27 / Idaho East
	{    26769L, 26769L, "ID-C",                       "NAD27 / Idaho Central",                                            1 },	// NAD27 / Idaho Central
	{    26770L, 26770L, "ID-W",                       "NAD27 / Idaho West",                                               1 },	// NAD27 / Idaho West
	{    26771L, 26771L, "IL-E",                       "NAD27 / Illinois East",                                            1 },	// NAD27 / Illinois East
	{    26772L, 26772L, "IL-W",                       "NAD27 / Illinois West",                                            1 },	// NAD27 / Illinois West
	{    26773L, 26773L, "IN-E",                       "NAD27 / Indiana East",                                             1 },	// NAD27 / Indiana East
	{    26774L, 26774L, "IN-W",                       "NAD27 / Indiana West",                                             1 },	// NAD27 / Indiana West
	{    26775L, 26775L, "IA-N",                       "NAD27 / Iowa North",                                               1 },	// NAD27 / Iowa North
	{    26776L, 26776L, "IA-S",                       "NAD27 / Iowa South",                                               1 },	// NAD27 / Iowa South
	{    26777L, 26777L, "KS-N",                       "NAD27 / Kansas North",                                             1 },	// NAD27 / Kansas North
	{    26778L, 26778L, "KS-S",                       "NAD27 / Kansas South",                                             1 },	// NAD27 / Kansas South
	{    26779L, 26779L, "KY-N",                       "NAD27 / Kentucky North",                                           1 },	// NAD27 / Kentucky North
	{    26780L, 26780L, "KY-S",                       "NAD27 / Kentucky South",                                           1 },	// NAD27 / Kentucky South
	{    26781L, 26781L, "LA-N",                       "NAD27 / Louisiana North",                                          1 },	// NAD27 / Louisiana North
	{    26782L, 26782L, "LA-S",                       "NAD27 / Louisiana South",                                          1 },	// NAD27 / Louisiana South
	{    26783L, 26783L, "ME-E",                       "NAD27 / Maine East",                                               1 },	// NAD27 / Maine East
	{    26784L, 26784L, "ME-W",                       "NAD27 / Maine West",                                               1 },	// NAD27 / Maine West
	{    26785L, 26785L, "MD",                         "NAD27 / Maryland",                                                 1 },	// NAD27 / Maryland
	{    26786L, 26786L, "MA",                         "NAD27 / Massachusetts Mainland",                                   1 },	// NAD27 / Massachusetts Mainland
	{    26787L, 26787L, "MA27-IS",                    "NAD27 / Massachusetts Island",                                     1 },	// NAD27 / Massachusetts Island
	{    26791L, 26791L, "MN-N",                       "NAD27 / Minnesota North",                                          1 },	// NAD27 / Minnesota North
	{    26792L, 26792L, "MN-C",                       "NAD27 / Minnesota Central",                                        1 },	// NAD27 / Minnesota Central
	{    26793L, 26793L, "MN-S",                       "NAD27 / Minnesota South",                                          1 },	// NAD27 / Minnesota South
	{    26794L, 26794L, "MS-E",                       "NAD27 / Mississippi East",                                         1 },	// NAD27 / Mississippi East
	{    26795L, 26795L, "MS-W",                       "NAD27 / Mississippi West",                                         1 },	// NAD27 / Mississippi West
	{    26796L, 26796L, "MO-E",                       "NAD27 / Missouri East",                                            1 },	// NAD27 / Missouri East
	{    26797L, 26797L, "MO-C",                       "NAD27 / Missouri Central",                                         1 },	// NAD27 / Missouri Central
	{    26798L, 26798L, "MO-W",                       "NAD27 / Missouri West",                                            1 },	// NAD27 / Missouri West
	{    26747L, 26799L, "CA-VII",                     "NAD27 / California zone VII",                                      1 },	// NAD27 / California zone VII
	{    26801L, 26801L, "MICHIGAN.East",              "NAD Michigan / Michigan East",                                     0 },	// NAD Michigan / Michigan East
	{    26802L, 26802L, "MICHIGAN.OldCentral",        "NAD Michigan / Michigan Old Central",                              0 },	// NAD Michigan / Michigan Old Central
	{    26803L, 26803L, "MICHIGAN.West",              "NAD Michigan / Michigan West",                                     0 },	// NAD Michigan / Michigan West
	{    26811L, 26811L, "MI27-N",                     "NAD Michigan / Michigan North",                                    1 },	// NAD Michigan / Michigan North
	{    26812L, 26812L, "MI27-C",                     "NAD Michigan / Michigan Central",                                  1 },	// NAD Michigan / Michigan Central
	{    26813L, 26813L, "MI27-S",                     "NAD Michigan / Michigan South",                                    1 },	// NAD Michigan / Michigan South
	{    26903L, 26903L, "UTM83-3",                    "NAD83 / UTM zone 3N",                                              1 },	// NAD83 / UTM zone 3N
	{    26904L, 26904L, "UTM83-4",                    "NAD83 / UTM zone 4N",                                              1 },	// NAD83 / UTM zone 4N
	{    26905L, 26905L, "UTM83-5",                    "NAD83 / UTM zone 5N",                                              1 },	// NAD83 / UTM zone 5N
	{    26906L, 26906L, "UTM83-6",                    "NAD83 / UTM zone 6N",                                              1 },	// NAD83 / UTM zone 6N
	{    26907L, 26907L, "UTM83-7",                    "NAD83 / UTM zone 7N",                                              1 },	// NAD83 / UTM zone 7N
	{    26908L, 26908L, "UTM83-8",                    "NAD83 / UTM zone 8N",                                              1 },	// NAD83 / UTM zone 8N
	{    26909L, 26909L, "UTM83-9",                    "NAD83 / UTM zone 9N",                                              1 },	// NAD83 / UTM zone 9N
	{    26910L, 26910L, "UTM83-10",                   "NAD83 / UTM zone 10N",                                             1 },	// NAD83 / UTM zone 10N
	{    26911L, 26911L, "UTM83-11",                   "NAD83 / UTM zone 11N",                                             1 },	// NAD83 / UTM zone 11N
	{    26912L, 26912L, "UTM83-12",                   "NAD83 / UTM zone 12N",                                             1 },	// NAD83 / UTM zone 12N
	{    26913L, 26913L, "UTM83-13",                   "NAD83 / UTM zone 13N",                                             1 },	// NAD83 / UTM zone 13N
	{    26914L, 26914L, "UTM83-14",                   "NAD83 / UTM zone 14N",                                             1 },	// NAD83 / UTM zone 14N
	{    26915L, 26915L, "UTM83-15",                   "NAD83 / UTM zone 15N",                                             1 },	// NAD83 / UTM zone 15N
	{    26916L, 26916L, "UTM83-16",                   "NAD83 / UTM zone 16N",                                             1 },	// NAD83 / UTM zone 16N
	{    26917L, 26917L, "UTM83-17",                   "NAD83 / UTM zone 17N",                                             1 },	// NAD83 / UTM zone 17N
	{    26918L, 26918L, "UTM83-18",                   "NAD83 / UTM zone 18N",                                             1 },	// NAD83 / UTM zone 18N
	{    26919L, 26919L, "UTM83-19",                   "NAD83 / UTM zone 19N",                                             1 },	// NAD83 / UTM zone 19N
	{    26920L, 26920L, "UTM83-20",                   "NAD83 / UTM zone 20N",                                             1 },	// NAD83 / UTM zone 20N
	{    26921L, 26921L, "UTM83-21",                   "NAD83 / UTM zone 21N",                                             1 },	// NAD83 / UTM zone 21N
	{    26922L, 26922L, "UTM83-22",                   "NAD83 / UTM zone 22N",                                             1 },	// NAD83 / UTM zone 22N
	{    26923L, 26923L, "NAD83.UTM-23N",              "NAD83 / UTM zone 23N",                                             0 },	// NAD83 / UTM zone 23N
	{    26929L, 26929L, "AL83-E",                     "NAD83 / Alabama East",                                             1 },	// NAD83 / Alabama East
	{    26930L, 26930L, "AL83-W",                     "NAD83 / Alabama West",                                             1 },	// NAD83 / Alabama West
	{    26931L, 26931L, "AK83-1",                     "NAD83 / Alaska zone 1",                                            1 },	// NAD83 / Alaska zone 1
	{    26932L, 26932L, "AK83-2",                     "NAD83 / Alaska zone 2",                                            1 },	// NAD83 / Alaska zone 2
	{    26933L, 26933L, "AK83-3",                     "NAD83 / Alaska zone 3",                                            1 },	// NAD83 / Alaska zone 3
	{    26934L, 26934L, "AK83-4",                     "NAD83 / Alaska zone 4",                                            1 },	// NAD83 / Alaska zone 4
	{    26935L, 26935L, "AK83-5",                     "NAD83 / Alaska zone 5",                                            1 },	// NAD83 / Alaska zone 5
	{    26936L, 26936L, "AK83-6",                     "NAD83 / Alaska zone 6",                                            1 },	// NAD83 / Alaska zone 6
	{    26937L, 26937L, "AK83-7",                     "NAD83 / Alaska zone 7",                                            1 },	// NAD83 / Alaska zone 7
	{    26938L, 26938L, "AK83-8",                     "NAD83 / Alaska zone 8",                                            1 },	// NAD83 / Alaska zone 8
	{    26939L, 26939L, "AK83-9",                     "NAD83 / Alaska zone 9",                                            1 },	// NAD83 / Alaska zone 9
	{    26940L, 26940L, "AK83-10",                    "NAD83 / Alaska zone 10",                                           1 },	// NAD83 / Alaska zone 10
	{    26941L, 26941L, "CA83-I",                     "NAD83 / California zone 1",                                        1 },	// NAD83 / California zone 1
	{    26942L, 26942L, "CA83-II",                    "NAD83 / California zone 2",                                        1 },	// NAD83 / California zone 2
	{    26943L, 26943L, "CA83-III",                   "NAD83 / California zone 3",                                        1 },	// NAD83 / California zone 3
	{    26944L, 26944L, "CA83-IV",                    "NAD83 / California zone 4",                                        1 },	// NAD83 / California zone 4
	{    26945L, 26945L, "CA83-V",                     "NAD83 / California zone 5",                                        1 },	// NAD83 / California zone 5
	{    26946L, 26946L, "CA83-VI",                    "NAD83 / California zone 6",                                        1 },	// NAD83 / California zone 6
	{    26948L, 26948L, "AZ83-E",                     "NAD83 / Arizona East",                                             1 },	// NAD83 / Arizona East
	{    26949L, 26949L, "AZ83-C",                     "NAD83 / Arizona Central",                                          1 },	// NAD83 / Arizona Central
	{    26950L, 26950L, "AZ83-W",                     "NAD83 / Arizona West",                                             1 },	// NAD83 / Arizona West
	{    26951L, 26951L, "AR83-N",                     "NAD83 / Arkansas North",                                           1 },	// NAD83 / Arkansas North
	{    26952L, 26952L, "AR83-S",                     "NAD83 / Arkansas South",                                           1 },	// NAD83 / Arkansas South
	{    26953L, 26953L, "CO83-N",                     "NAD83 / Colorado North",                                           1 },	// NAD83 / Colorado North
	{    26954L, 26954L, "CO83-C",                     "NAD83 / Colorado Central",                                         1 },	// NAD83 / Colorado Central
	{    26955L, 26955L, "CO83-S",                     "NAD83 / Colorado South",                                           1 },	// NAD83 / Colorado South
	{    26956L, 26956L, "CT83",                       "NAD83 / Connecticut",                                              1 },	// NAD83 / Connecticut
	{    26957L, 26957L, "DE83",                       "NAD83 / Delaware",                                                 1 },	// NAD83 / Delaware
	{    26958L, 26958L, "FL83-E",                     "NAD83 / Florida East",                                             1 },	// NAD83 / Florida East
	{    26959L, 26959L, "FL83-W",                     "NAD83 / Florida West",                                             1 },	// NAD83 / Florida West
	{    26960L, 26960L, "FL83-N",                     "NAD83 / Florida North",                                            1 },	// NAD83 / Florida North
	{    26961L, 26961L, "HI83-1",                     "NAD83 / Hawaii zone 1",                                            1 },	// NAD83 / Hawaii zone 1
	{    26962L, 26962L, "HI83-2",                     "NAD83 / Hawaii zone 2",                                            1 },	// NAD83 / Hawaii zone 2
	{    26963L, 26963L, "HI83-3",                     "NAD83 / Hawaii zone 3",                                            1 },	// NAD83 / Hawaii zone 3
	{    26964L, 26964L, "HI83-4",                     "NAD83 / Hawaii zone 4",                                            1 },	// NAD83 / Hawaii zone 4
	{    26965L, 26965L, "HI83-5",                     "NAD83 / Hawaii zone 5",                                            1 },	// NAD83 / Hawaii zone 5
	{    26966L, 26966L, "GA83-E",                     "NAD83 / Georgia East",                                             1 },	// NAD83 / Georgia East
	{    26967L, 26967L, "GA83-W",                     "NAD83 / Georgia West",                                             1 },	// NAD83 / Georgia West
	{    26968L, 26968L, "ID83-E",                     "NAD83 / Idaho East",                                               1 },	// NAD83 / Idaho East
	{    26969L, 26969L, "ID83-C",                     "NAD83 / Idaho Central",                                            1 },	// NAD83 / Idaho Central
	{    26970L, 26970L, "ID83-W",                     "NAD83 / Idaho West",                                               1 },	// NAD83 / Idaho West
	{    26971L, 26971L, "IL83-E",                     "NAD83 / Illinois East",                                            1 },	// NAD83 / Illinois East
	{    26972L, 26972L, "IL83-W",                     "NAD83 / Illinois West",                                            1 },	// NAD83 / Illinois West
	{    26973L, 26973L, "IN83-E",                     "NAD83 / Indiana East",                                             1 },	// NAD83 / Indiana East
	{    26974L, 26974L, "IN83-W",                     "NAD83 / Indiana West",                                             1 },	// NAD83 / Indiana West
	{    26975L, 26975L, "IA83-N",                     "NAD83 / Iowa North",                                               1 },	// NAD83 / Iowa North
	{    26976L, 26976L, "IA83-S",                     "NAD83 / Iowa South",                                               1 },	// NAD83 / Iowa South
	{    26977L, 26977L, "KS83-N",                     "NAD83 / Kansas North",                                             1 },	// NAD83 / Kansas North
	{    26978L, 26978L, "KS83-S",                     "NAD83 / Kansas South",                                             1 },	// NAD83 / Kansas South
	{    26979L,  2205L, "KY83-N",                     "NAD83 / Kentucky North",                                           1 },	// NAD83 / Kentucky North
	{    26980L, 26980L, "KY83-S",                     "NAD83 / Kentucky South",                                           1 },	// NAD83 / Kentucky South
	{    26981L, 26981L, "LA83-N",                     "NAD83 / Louisiana North",                                          1 },	// NAD83 / Louisiana North
	{    26982L, 26982L, "LA83-S",                     "NAD83 / Louisiana South",                                          1 },	// NAD83 / Louisiana South
	{    26983L, 26983L, "ME83-E",                     "NAD83 / Maine East",                                               1 },	// NAD83 / Maine East
	{    26984L, 26984L, "ME83-W",                     "NAD83 / Maine West",                                               1 },	// NAD83 / Maine West
	{    26985L, 26985L, "MD83",                       "NAD83 / Maryland",                                                 1 },	// NAD83 / Maryland
	{    26986L, 26986L, "MA83",                       "NAD83 / Massachusetts Mainland",                                   1 },	// NAD83 / Massachusetts Mainland
	{    26987L, 26987L, "MA83-IS",                    "NAD83 / Massachusetts Island",                                     1 },	// NAD83 / Massachusetts Island
	{    26988L, 26988L, "MI83-N",                     "NAD83 / Michigan North",                                           1 },	// NAD83 / Michigan North
	{    26989L, 26989L, "MI83-C",                     "NAD83 / Michigan Central",                                         1 },	// NAD83 / Michigan Central
	{    26990L, 26990L, "MI83-S",                     "NAD83 / Michigan South",                                           1 },	// NAD83 / Michigan South
	{    26991L, 26991L, "MN83-N",                     "NAD83 / Minnesota North",                                          1 },	// NAD83 / Minnesota North
	{    26992L, 26992L, "MN83-C",                     "NAD83 / Minnesota Central",                                        1 },	// NAD83 / Minnesota Central
	{    26993L, 26993L, "MN83-S",                     "NAD83 / Minnesota South",                                          1 },	// NAD83 / Minnesota South
	{    26994L, 26994L, "MS83-E",                     "NAD83 / Mississippi East",                                         1 },	// NAD83 / Mississippi East
	{    26995L, 26995L, "MS83-W",                     "NAD83 / Mississippi West",                                         1 },	// NAD83 / Mississippi West
	{    26996L, 26996L, "MO83-E",                     "NAD83 / Missouri East",                                            1 },	// NAD83 / Missouri East
	{    26997L, 26997L, "MO83-C",                     "NAD83 / Missouri Central",                                         1 },	// NAD83 / Missouri Central
	{    26998L, 26998L, "MO83-W",                     "NAD83 / Missouri West",                                            1 },	// NAD83 / Missouri West
	{    27038L, 27038L, "Nahrwan67.UTM-38N",          "Nahrwan 1967 / UTM zone 38N",                                      0 },	// Nahrwan 1967 / UTM zone 38N
	{    27039L, 27039L, "Nahrwan67.UTM-39N",          "Nahrwan 1967 / UTM zone 39N",                                      0 },	// Nahrwan 1967 / UTM zone 39N
	{    27040L, 27040L, "Nahrwan67.UTM-40N",          "Nahrwan 1967 / UTM zone 40N",                                      0 },	// Nahrwan 1967 / UTM zone 40N
	{    27120L, 27120L, "Naparima72.UTM-20N",         "Naparima 1972 / UTM zone 20N",                                     0 },	// Naparima 1972 / UTM zone 20N
	{    27200L, 27200L, "NZEALAND",                   "NZGD49 / New Zealand Map Grid",                                    1 },	// NZGD49 / New Zealand Map Grid
	{    27205L, 27205L, "NZGD49.MountEden",           "NZGD49 / Mount Eden Circuit",                                      0 },	// NZGD49 / Mount Eden Circuit
	{    27206L, 27206L, "NZGD49.BayOfPlenty",         "NZGD49 / Bay of Plenty Circuit",                                   0 },	// NZGD49 / Bay of Plenty Circuit
	{    27207L, 27207L, "NZGD49.PovertyBay",          "NZGD49 / Poverty Bay Circuit",                                     0 },	// NZGD49 / Poverty Bay Circuit
	{    27208L, 27208L, "NZGD49.HawkesBay",           "NZGD49 / Hawkes Bay Circuit",                                      0 },	// NZGD49 / Hawkes Bay Circuit
	{    27209L, 27209L, "NZGD49.Taranaki",            "NZGD49 / Taranaki Circuit",                                        0 },	// NZGD49 / Taranaki Circuit
	{    27210L, 27210L, "NZGD49.Tuhirangi",           "NZGD49 / Tuhirangi Circuit",                                       0 },	// NZGD49 / Tuhirangi Circuit
	{    27211L, 27211L, "NZGD49.Wanganui",            "NZGD49 / Wanganui Circuit",                                        0 },	// NZGD49 / Wanganui Circuit
	{    27212L, 27212L, "NZGD49.Wairarapa",           "NZGD49 / Wairarapa Circuit",                                       0 },	// NZGD49 / Wairarapa Circuit
	{    27213L, 27213L, "NZGD49.Wellington",          "NZGD49 / Wellington Circuit",                                      0 },	// NZGD49 / Wellington Circuit
	{    27214L, 27214L, "NZGD49.Collingwood",         "NZGD49 / Collingwood Circuit",                                     0 },	// NZGD49 / Collingwood Circuit
	{    27215L, 27215L, "NZGD49.Nelson",              "NZGD49 / Nelson Circuit",                                          0 },	// NZGD49 / Nelson Circuit
	{    27216L, 27216L, "NZGD49.Karamea",             "NZGD49 / Karamea Circuit",                                         0 },	// NZGD49 / Karamea Circuit
	{    27217L, 27217L, "NZGD49.Buller",              "NZGD49 / Buller Circuit",                                          0 },	// NZGD49 / Buller Circuit
	{    27218L, 27218L, "NZGD49.Grey",                "NZGD49 / Grey Circuit",                                            0 },	// NZGD49 / Grey Circuit
	{    27219L, 27219L, "NZGD49.Amuri",               "NZGD49 / Amuri Circuit",                                           0 },	// NZGD49 / Amuri Circuit
	{    27220L, 27220L, "NZGD49.Marlborough",         "NZGD49 / Marlborough Circuit",                                     0 },	// NZGD49 / Marlborough Circuit
	{    27221L, 27221L, "NZGD49.Hokitika",            "NZGD49 / Hokitika Circuit",                                        0 },	// NZGD49 / Hokitika Circuit
	{    27222L, 27222L, "NZGD49.Okarito",             "NZGD49 / Okarito Circuit",                                         0 },	// NZGD49 / Okarito Circuit
	{    27223L, 27223L, "NZGD49.JacksonsBay",         "NZGD49 / Jacksons Bay Circuit",                                    0 },	// NZGD49 / Jacksons Bay Circuit
	{    27224L, 27224L, "NZGD49.MountPleasant",       "NZGD49 / Mount Pleasant Circuit",                                  0 },	// NZGD49 / Mount Pleasant Circuit
	{    27225L, 27225L, "NZGD49.Gawler",              "NZGD49 / Gawler Circuit",                                          0 },	// NZGD49 / Gawler Circuit
	{    27226L, 27226L, "NZGD49.Timaru",              "NZGD49 / Timaru Circuit",                                          0 },	// NZGD49 / Timaru Circuit
	{    27227L, 27227L, "NZGD49.LindisPeak",          "NZGD49 / Lindis Peak Circuit",                                     0 },	// NZGD49 / Lindis Peak Circuit
	{    27228L, 27228L, "NZGD49.MountNicholas",       "NZGD49 / Mount Nicholas Circuit",                                  0 },	// NZGD49 / Mount Nicholas Circuit
	{    27229L, 27229L, "NZGD49.MountYork",           "NZGD49 / Mount York Circuit",                                      0 },	// NZGD49 / Mount York Circuit
	{    27230L, 27230L, "NZGD49.ObservationPnt",      "NZGD49 / Observation Point Circuit",                               0 },	// NZGD49 / Observation Point Circuit
	{    27231L, 27231L, "NZGD49.NorthTaieri",         "NZGD49 / North Taieri Circuit",                                    0 },	// NZGD49 / North Taieri Circuit
	{    27232L, 27232L, "NZGD49.Bluff",               "NZGD49 / Bluff Circuit",                                           0 },	// NZGD49 / Bluff Circuit
	{    27258L, 27258L, "NZGD49.UTM-58S",             "NZGD49 / UTM zone 58S",                                            0 },	// NZGD49 / UTM zone 58S
	{    27259L, 27259L, "NZGD49.UTM-59S",             "NZGD49 / UTM zone 59S",                                            0 },	// NZGD49 / UTM zone 59S
	{    27260L, 27260L, "NZGD49.UTM-60S",             "NZGD49 / UTM zone 60S",                                            0 },	// NZGD49 / UTM zone 60S
	{    27291L, 27291L, "NZGD49.NorthIslandGrid",     "NZGD49 / North Island Grid",                                       0 },	// NZGD49 / North Island Grid
	{    27292L, 27292L, "NZGD49.SouthIslandGrid",     "NZGD49 / South Island Grid",                                       0 },	// NZGD49 / South Island Grid
	{    27429L, 27429L, "Datum73.UTM-29N",            "Datum 73 / UTM zone 29N",                                          0 },	// Datum 73 / UTM zone 29N
	{    27492L, 27492L, "Datum73.ModPortgGrd",        "Datum 73 / Modified Portuguese Grid",                              0 },	// Datum 73 / Modified Portuguese Grid
	{    81989L, 27700L, "BritishNatGrid",             "British National Grid",                                            1 },	// OSGB 1936 / British National Grid
	{    28191L, 28191L, "Palestine23.Grid",           "Palestine 1923 / Palestine Grid",                                  0 },	// Palestine 1923 / Palestine Grid
	{    28192L, 28192L, "Palestine23.Belt",           "Palestine 1923 / Palestine Belt",                                  0 },	// Palestine 1923 / Palestine Belt
	{ 81989001L, 28193L, "Palestine23.IsraeliGrd",     "Palestine 1923 / Israeli CS Grid",                                 0 },	// Palestine 1923 / Israeli CS Grid
	{    28232L, 28232L, "PointeNoire.UTM-32S",        "Pointe Noire / UTM zone 32S",                                      0 },	// Pointe Noire / UTM zone 32S
	{    28348L, 28348L, "MGA94-48",                   "GDA94 / MGA zone 48",                                              1 },	// GDA94 / MGA zone 48
	{    28349L, 28349L, "MGA94-49",                   "GDA94 / MGA zone 49",                                              1 },	// GDA94 / MGA zone 49
	{    28350L, 28350L, "MGA94-50",                   "GDA94 / MGA zone 50",                                              1 },	// GDA94 / MGA zone 50
	{    28351L, 28351L, "MGA94-51",                   "GDA94 / MGA zone 51",                                              1 },	// GDA94 / MGA zone 51
	{    28352L, 28352L, "MGA94-52",                   "GDA94 / MGA zone 52",                                              1 },	// GDA94 / MGA zone 52
	{    28353L, 28353L, "MGA94-53",                   "GDA94 / MGA zone 53",                                              1 },	// GDA94 / MGA zone 53
	{    28354L, 28354L, "MGA94-54",                   "GDA94 / MGA zone 54",                                              1 },	// GDA94 / MGA zone 54
	{    28355L, 28355L, "MGA94-55",                   "GDA94 / MGA zone 55",                                              1 },	// GDA94 / MGA zone 55
	{    28356L, 28356L, "MGA94-56",                   "GDA94 / MGA zone 56",                                              1 },	// GDA94 / MGA zone 56
	{    28357L, 28357L, "MGA94-57",                   "GDA94 / MGA zone 57",                                              1 },	// GDA94 / MGA zone 57
	{    28358L, 28358L, "MGA94-58",                   "GDA94 / MGA zone 58",                                              1 },	// GDA94 / MGA zone 58
	{    28402L, 28402L, "Pulkovo42.GK-2",             "Pulkovo 1942 / Gauss-Kruger zone 2",                               0 },	// Pulkovo 1942 / Gauss-Kruger zone 2
	{    28403L, 28403L, "Pulkovo42.GK-3",             "Pulkovo 1942 / Gauss-Kruger zone 3",                               0 },	// Pulkovo 1942 / Gauss-Kruger zone 3
	{    28404L, 28404L, "GK42-4",                     "Pulkovo 1942 / Gauss-Kruger zone 4",                               1 },	// Pulkovo 1942 / Gauss-Kruger zone 4
	{    28405L, 28405L, "GK42-5",                     "Pulkovo 1942 / Gauss-Kruger zone 5",                               1 },	// Pulkovo 1942 / Gauss-Kruger zone 5
	{    28406L, 28406L, "GK42-6",                     "Pulkovo 1942 / Gauss-Kruger zone 6",                               1 },	// Pulkovo 1942 / Gauss-Kruger zone 6
	{    28407L, 28407L, "GK42-7",                     "Pulkovo 1942 / Gauss-Kruger zone 7",                               1 },	// Pulkovo 1942 / Gauss-Kruger zone 7
	{    28408L, 28408L, "GK42-8",                     "Pulkovo 1942 / Gauss-Kruger zone 8",                               1 },	// Pulkovo 1942 / Gauss-Kruger zone 8
	{    28409L, 28409L, "GK42-9",                     "Pulkovo 1942 / Gauss-Kruger zone 9",                               1 },	// Pulkovo 1942 / Gauss-Kruger zone 9
	{    28410L, 28410L, "GK42-10",                    "Pulkovo 1942 / Gauss-Kruger zone 10",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 10
	{    28411L, 28411L, "GK42-11",                    "Pulkovo 1942 / Gauss-Kruger zone 11",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 11
	{    28412L, 28412L, "GK42-12",                    "Pulkovo 1942 / Gauss-Kruger zone 12",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 12
	{    28413L, 28413L, "GK42-13",                    "Pulkovo 1942 / Gauss-Kruger zone 13",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 13
	{    28414L, 28414L, "GK42-14",                    "Pulkovo 1942 / Gauss-Kruger zone 14",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 14
	{    28415L, 28415L, "GK42-15",                    "Pulkovo 1942 / Gauss-Kruger zone 15",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 15
	{    28416L, 28416L, "GK42-16",                    "Pulkovo 1942 / Gauss-Kruger zone 16",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 16
	{    28417L, 28417L, "GK42-17",                    "Pulkovo 1942 / Gauss-Kruger zone 17",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 17
	{    28418L, 28418L, "GK42-18",                    "Pulkovo 1942 / Gauss-Kruger zone 18",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 18
	{    28419L, 28419L, "GK42-19",                    "Pulkovo 1942 / Gauss-Kruger zone 19",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 19
	{    28420L, 28420L, "GK42-20",                    "Pulkovo 1942 / Gauss-Kruger zone 20",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 20
	{    28421L, 28421L, "GK42-21",                    "Pulkovo 1942 / Gauss-Kruger zone 21",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 21
	{    28422L, 28422L, "GK42-22",                    "Pulkovo 1942 / Gauss-Kruger zone 22",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 22
	{    28423L, 28423L, "GK42-23",                    "Pulkovo 1942 / Gauss-Kruger zone 23",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 23
	{    28424L, 28424L, "GK42-24",                    "Pulkovo 1942 / Gauss-Kruger zone 24",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 24
	{    28425L, 28425L, "GK42-25",                    "Pulkovo 1942 / Gauss-Kruger zone 25",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 25
	{    28426L, 28426L, "GK42-26",                    "Pulkovo 1942 / Gauss-Kruger zone 26",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 26
	{    28427L, 28427L, "GK42-27",                    "Pulkovo 1942 / Gauss-Kruger zone 27",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 27
	{    28428L, 28428L, "GK42-28",                    "Pulkovo 1942 / Gauss-Kruger zone 28",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 28
	{    28429L, 28429L, "GK42-29",                    "Pulkovo 1942 / Gauss-Kruger zone 29",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 29
	{    28430L, 28430L, "GK42-30",                    "Pulkovo 1942 / Gauss-Kruger zone 30",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 30
	{    28431L, 28431L, "GK42-31",                    "Pulkovo 1942 / Gauss-Kruger zone 31",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 31
	{    28432L, 28432L, "GK42-32",                    "Pulkovo 1942 / Gauss-Kruger zone 32",                              1 },	// Pulkovo 1942 / Gauss-Kruger zone 32
	{    28462L, 28462L, "Pulkovo42.GK-2N",            "Pulkovo 1942 / Gauss-Kruger 2N",                                   2 },	// Pulkovo 1942 / Gauss-Kruger 2N
	{    28463L, 28463L, "Pulkovo42.GK-3N",            "Pulkovo 1942 / Gauss-Kruger 3N",                                   2 },	// Pulkovo 1942 / Gauss-Kruger 3N
	{    28464L, 28464L, "GK42-4N",                    "Pulkovo 1942 / Gauss-Kruger 4N",                                   3 },	// Pulkovo 1942 / Gauss-Kruger 4N
	{    28465L, 28465L, "GK42-5N",                    "Pulkovo 1942 / Gauss-Kruger 5N",                                   3 },	// Pulkovo 1942 / Gauss-Kruger 5N
	{    28466L, 28466L, "GK42-6N",                    "Pulkovo 1942 / Gauss-Kruger 6N",                                   3 },	// Pulkovo 1942 / Gauss-Kruger 6N
	{    28467L, 28467L, "GK42-7N",                    "Pulkovo 1942 / Gauss-Kruger 7N",                                   3 },	// Pulkovo 1942 / Gauss-Kruger 7N
	{    28468L, 28468L, "GK42-8N",                    "Pulkovo 1942 / Gauss-Kruger 8N",                                   3 },	// Pulkovo 1942 / Gauss-Kruger 8N
	{    28469L, 28469L, "GK42-9N",                    "Pulkovo 1942 / Gauss-Kruger 9N",                                   3 },	// Pulkovo 1942 / Gauss-Kruger 9N
	{    28470L, 28470L, "GK42-10N",                   "Pulkovo 1942 / Gauss-Kruger 10N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 10N
	{    28471L, 28471L, "GK42-11N",                   "Pulkovo 1942 / Gauss-Kruger 11N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 11N
	{    28472L, 28472L, "GK42-12N",                   "Pulkovo 1942 / Gauss-Kruger 12N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 12N
	{    28473L, 28473L, "GK42-13N",                   "Pulkovo 1942 / Gauss-Kruger 13N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 13N
	{    28474L, 28474L, "GK42-14N",                   "Pulkovo 1942 / Gauss-Kruger 14N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 14N
	{    28475L, 28475L, "GK42-15N",                   "Pulkovo 1942 / Gauss-Kruger 15N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 15N
	{    28476L, 28476L, "GK42-16N",                   "Pulkovo 1942 / Gauss-Kruger 16N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 16N
	{    28477L, 28477L, "GK42-17N",                   "Pulkovo 1942 / Gauss-Kruger 17N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 17N
	{    28478L, 28478L, "GK42-18N",                   "Pulkovo 1942 / Gauss-Kruger 18N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 18N
	{    28479L, 28479L, "GK42-19N",                   "Pulkovo 1942 / Gauss-Kruger 19N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 19N
	{    28480L, 28480L, "GK42-20N",                   "Pulkovo 1942 / Gauss-Kruger 20N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 20N
	{    28481L, 28481L, "GK42-21N",                   "Pulkovo 1942 / Gauss-Kruger 21N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 21N
	{    28482L, 28482L, "GK42-22N",                   "Pulkovo 1942 / Gauss-Kruger 22N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 22N
	{    28483L, 28483L, "GK42-23N",                   "Pulkovo 1942 / Gauss-Kruger 23N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 23N
	{    28484L, 28484L, "GK42-24N",                   "Pulkovo 1942 / Gauss-Kruger 24N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 24N
	{    28485L, 28485L, "GK42-25N",                   "Pulkovo 1942 / Gauss-Kruger 25N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 25N
	{    28486L, 28486L, "GK42-26N",                   "Pulkovo 1942 / Gauss-Kruger 26N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 26N
	{    28487L, 28487L, "GK42-27N",                   "Pulkovo 1942 / Gauss-Kruger 27N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 27N
	{    28488L, 28488L, "GK42-28N",                   "Pulkovo 1942 / Gauss-Kruger 28N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 28N
	{    28489L, 28489L, "GK42-29N",                   "Pulkovo 1942 / Gauss-Kruger 29N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 29N
	{    28490L, 28490L, "GK42-30N",                   "Pulkovo 1942 / Gauss-Kruger 30N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 30N
	{    28491L, 28491L, "GK42-31N",                   "Pulkovo 1942 / Gauss-Kruger 31N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 31N
	{    28492L, 28492L, "GK42-32N",                   "Pulkovo 1942 / Gauss-Kruger 32N",                                  3 },	// Pulkovo 1942 / Gauss-Kruger 32N
	{    28600L, 28600L, "Qatar74.NationalGrid",       "Qatar 1974 / Qatar National Grid",                                 0 },	// Qatar 1974 / Qatar National Grid
	{    28991L, 28991L, "Netherlands-RD-Old",         "Amersfoort / RD Old",                                              1 },	// Amersfoort / RD Old
	{    28992L, 28992L, "Netherlands-RD-New",         "Amersfoort / RD New",                                              1 },	// Amersfoort / RD New
	{    29100L, 29100L, "SA1969.BzPolyconic",         "SAD69 / Brazil Polyconic",                                         2 },	// SAD69 / Brazil Polyconic
	{    29101L, 29101L, "SAD69.BzPolyconic",          "SAD69 / Brazil Polyconic",                                         0 },	// SAD69 / Brazil Polyconic
	{    29168L, 29168L, "SAD69.UTM-18N",              "SAD69 / UTM zone 18N",                                             0 },	// SAD69 / UTM zone 18N
	{    29169L, 29169L, "SAD69.UTM-19N",              "SAD69 / UTM zone 19N",                                             0 },	// SAD69 / UTM zone 19N
	{    29170L, 29170L, "SAD69.UTM-20N",              "SAD69 / UTM zone 20N",                                             0 },	// SAD69 / UTM zone 20N
	{    29171L, 29171L, "SAD69.UTM-21N",              "SAD69 / UTM zone 21N",                                             0 },	// SAD69 / UTM zone 21N
	{    29172L, 29172L, "SAD69.UTM-22N",              "SAD69 / UTM zone 22N",                                             0 },	// SAD69 / UTM zone 22N
	{    29187L, 29187L, "SAD69.UTM-17S",              "SAD69 / UTM zone 17S",                                             0 },	// SAD69 / UTM zone 17S
	{    29188L, 29188L, "SAD69.UTM-18S",              "SAD69 / UTM zone 18S",                                             0 },	// SAD69 / UTM zone 18S
	{    29189L, 29189L, "SAD69.UTM-19S",              "SAD69 / UTM zone 19S",                                             0 },	// SAD69 / UTM zone 19S
	{    29190L, 29190L, "SAD69.UTM-20S",              "SAD69 / UTM zone 20S",                                             0 },	// SAD69 / UTM zone 20S
	{    29191L, 29191L, "SAD69.UTM-21S",              "SAD69 / UTM zone 21S",                                             0 },	// SAD69 / UTM zone 21S
	{    29192L, 29192L, "SAD69.UTM-22S",              "SAD69 / UTM zone 22S",                                             0 },	// SAD69 / UTM zone 22S
	{    29193L, 29193L, "SAD69.UTM-23S",              "SAD69 / UTM zone 23S",                                             0 },	// SAD69 / UTM zone 23S
	{    29194L, 29194L, "SAD69.UTM-24S",              "SAD69 / UTM zone 24S",                                             0 },	// SAD69 / UTM zone 24S
	{    29195L, 29195L, "SAD69.UTM-25S",              "SAD69 / UTM zone 25S",                                             0 },	// SAD69 / UTM zone 25S
	{    29220L, 29220L, "Sapper.UTM-20S",             "Sapper Hill 1943 / UTM zone 20S",                                  0 },	// Sapper Hill 1943 / UTM zone 20S
	{    29221L, 29221L, "Sapper.UTM-21S",             "Sapper Hill 1943 / UTM zone 21S",                                  0 },	// Sapper Hill 1943 / UTM zone 21S
	{    29333L, 29333L, "Schwarzk.UTM-33S",           "Schwarzeck / UTM zone 33S",                                        0 },	// Schwarzeck / UTM zone 33S
	{    29371L, 29371L, "Schwarzk.SWAfrican-11",      "South West African Coord. System zone 11",                         0 },	// South West African Coord. System zone 11
	{    29373L, 29373L, "Schwarzk.SWAfrican-13",      "South West African Coord. System zone 13",                         0 },	// South West African Coord. System zone 13
	{    29375L, 29375L, "Schwarzk.SWAfrican-15",      "South West African Coord. System zone 15",                         0 },	// South West African Coord. System zone 15
	{    29377L, 29377L, "Schwarzk.SWAfrican-17",      "South West African Coord. System zone 17",                         0 },	// South West African Coord. System zone 17
	{    29379L, 29379L, "Schwarzk.SWAfrican-19",      "South West African Coord. System zone 19",                         0 },	// South West African Coord. System zone 19
	{    29381L, 29381L, "Schwarzk.SWAfrican-21",      "South West African Coord. System zone 21",                         0 },	// South West African Coord. System zone 21
	{    29383L, 29383L, "Schwarzk.SWAfrican-23",      "South West African Coord. System zone 23",                         0 },	// South West African Coord. System zone 23
	{    29385L, 29385L, "Schwarzk.SWAfrican-25",      "South West African Coord. System zone 25",                         0 },	// South West African Coord. System zone 25
	{    29738L, 29738L, "Tananarive.UTM-38S",         "Tananarive / UTM zone 38S",                                        0 },	// Tananarive / UTM zone 38S
	{    29739L, 29739L, "Tananarive.UTM-39S",         "Tananarive / UTM zone 39S",                                        0 },	// Tananarive / UTM zone 39S
	{    29849L, 29849L, "TMBLI-B.UTM-49N",            "Timbalai 1948 / UTM zone 49N",                                     0 },	// Timbalai 1948 / UTM zone 49N
	{    29850L, 29850L, "TMBLI-B.UTM-50N",            "Timbalai 1948 / UTM zone 50N",                                     0 },	// Timbalai 1948 / UTM zone 50N
	{    29871L, 29871L, "TMBLI-B.RSOBorneo.ch",       "Timbalai 1948 / R.S.O. Borneo (ch)",                               0 },	// Timbalai 1948 / RSO Borneo (ch)
	{    29872L, 29872L, "TMBLI-B.RSOBorneo.ft",       "Timbalai 1948 / R.S.O. Borneo (ft)",                               0 },	// Timbalai 1948 / RSO Borneo (ft)
	{    29873L, 29873L, "TMBLI-B.RSOBorneo.m",        "Timbalai 1948 / R.S.O. Borneo (m)",                                0 },	// Timbalai 1948 / RSO Borneo (m)
	{    29900L, 29900L, "TM65.IrishNationalGrid",     "TM65 / Irish National Grid",                                       2 },	// TM65 / Irish National Grid
	{    29901L, 29901L, "OSNI52.IrishNtlGrid",        "OSNI 1952 / Irish National Grid",                                  0 },	// OSNI 1952 / Irish National Grid
	{    29902L, 29902L, "TM65.Irish Grid",            "TM65 / Irish Grid",                                                0 },	// TM65 / Irish Grid
	{    30161L, 30161L, "Tokyo.PlnRctCS-I",           "Tokyo / Japan Plane Rectangular CS I",                             0 },	// Tokyo / Japan Plane Rectangular CS I
	{    30162L, 30162L, "Tokyo.PlnRctCS-II",          "Tokyo / Japan Plane Rectangular CS II",                            0 },	// Tokyo / Japan Plane Rectangular CS II
	{    30163L, 30163L, "Tokyo.PlnRctCS-III",         "Tokyo / Japan Plane Rectangular CS III",                           0 },	// Tokyo / Japan Plane Rectangular CS III
	{    30164L, 30164L, "Tokyo.PlnRctCS-IV",          "Tokyo / Japan Plane Rectangular CS IV",                            0 },	// Tokyo / Japan Plane Rectangular CS IV
	{    30165L, 30165L, "Tokyo.PlnRctCS-V",           "Tokyo / Japan Plane Rectangular CS V",                             0 },	// Tokyo / Japan Plane Rectangular CS V
	{    30166L, 30166L, "Tokyo.PlnRctCS-VI",          "Tokyo / Japan Plane Rectangular CS VI",                            0 },	// Tokyo / Japan Plane Rectangular CS VI
	{    30167L, 30167L, "Tokyo.PlnRctCS-VII",         "Tokyo / Japan Plane Rectangular CS VII",                           0 },	// Tokyo / Japan Plane Rectangular CS VII
	{    30168L, 30168L, "Tokyo.PlnRctCS-VIII",        "Tokyo / Japan Plane Rectangular CS VIII",                          0 },	// Tokyo / Japan Plane Rectangular CS VIII
	{    30169L, 30169L, "Tokyo.PlnRctCS-IX",          "Tokyo / Japan Plane Rectangular CS IX",                            0 },	// Tokyo / Japan Plane Rectangular CS IX
	{    30170L, 30170L, "Tokyo.PlnRctCS-X",           "Tokyo / Japan Plane Rectangular CS X",                             0 },	// Tokyo / Japan Plane Rectangular CS X
	{    30171L, 30171L, "Tokyo.PlnRctCS-XI",          "Tokyo / Japan Plane Rectangular CS XI",                            0 },	// Tokyo / Japan Plane Rectangular CS XI
	{    30172L, 30172L, "Tokyo.PlnRctCS-XII",         "Tokyo / Japan Plane Rectangular CS XII",                           0 },	// Tokyo / Japan Plane Rectangular CS XII
	{    30173L, 30173L, "Tokyo.PlnRctCS-XIII",        "Tokyo / Japan Plane Rectangular CS XIII",                          0 },	// Tokyo / Japan Plane Rectangular CS XIII
	{    30174L, 30174L, "Tokyo.PlnRctCS-XIV",         "Tokyo / Japan Plane Rectangular CS XIV",                           0 },	// Tokyo / Japan Plane Rectangular CS XIV
	{    30175L, 30175L, "Tokyo.PlnRctCS-XV",          "Tokyo / Japan Plane Rectangular CS XV",                            0 },	// Tokyo / Japan Plane Rectangular CS XV
	{    30176L, 30176L, "Tokyo.PlnRctCS-XVI",         "Tokyo / Japan Plane Rectangular CS XVI",                           0 },	// Tokyo / Japan Plane Rectangular CS XVI
	{    30177L, 30177L, "Tokyo.PlnRctCS-XVII",        "Tokyo / Japan Plane Rectangular CS XVII",                          0 },	// Tokyo / Japan Plane Rectangular CS XVII
	{    30178L, 30178L, "Tokyo.PlnRctCS-XVIII",       "Tokyo / Japan Plane Rectangular CS XVIII",                         0 },	// Tokyo / Japan Plane Rectangular CS XVIII
	{    30179L, 30179L, "Tokyo.PlnRctCS-XIX",         "Tokyo / Japan Plane Rectangular CS XIX",                           0 },	// Tokyo / Japan Plane Rectangular CS XIX
	{    30200L, 30200L, "Trinidad03.Trinidad",        "Trinidad 1903 / Trinidad Grid",                                    0 },	// Trinidad 1903 / Trinidad Grid
	{    30491L, 30491L, "Voirol1875.NordAlgerie",     "Voirol 1875 / Nord Algerie (ancienne)",                            0 },	// Voirol 1875 / Nord Algerie (ancienne)
	{    30492L, 30492L, "Voirol1875.Sud Algerie",     "Voirol 1875 / Sud Algerie (ancienne)",                             0 },	// Voirol 1875 / Sud Algerie (ancienne)
	{    30729L, 30729L, "NordSahara59.UTM-29N",       "Nord Sahara 1959 / UTM zone 29N",                                  0 },	// Nord Sahara 1959 / UTM zone 29N
	{    30730L, 30730L, "NordSahara59.UTM-30N",       "Nord Sahara 1959 / UTM zone 30N",                                  0 },	// Nord Sahara 1959 / UTM zone 30N
	{    30731L, 30731L, "NordSahara59.UTM-31N",       "Nord Sahara 1959 / UTM zone 31N",                                  0 },	// Nord Sahara 1959 / UTM zone 31N
	{    30732L, 30732L, "NordSahara59.UTM-32N",       "Nord Sahara 1959 / UTM zone 32N",                                  0 },	// Nord Sahara 1959 / UTM zone 32N
	{    30791L, 30791L, "NordSahara59.UnifieN",       "Nord Sahara 1959 / Voirol Unifie Nord",                            0 },	// Nord Sahara 1959 / Voirol Unifie Nord
	{    30792L, 30792L, "NordSahara59.UnifieS",       "Nord Sahara 1959 / Voirol Unifie Sud",                             0 },	// Nord Sahara 1959 / Voirol Unifie Sud
	{    31121L, 31121L, "Zanderij.UTM-21N",           "Zanderij / UTM zone 21N",                                          0 },	// Zanderij / UTM zone 21N
	{    31154L, 31154L, "Zanderij.TM-54NW",           "Zanderij / TM 54 NW",                                              0 },	// Zanderij / TM 54 NW
	{    31170L, 31170L, "Zanderij.SurinameTMOld",     "Zanderij / Suriname Old TM",                                       0 },	// Zanderij / Suriname Old TM
	{    31171L, 31171L, "Zanderij.SurinameTM",        "Zanderij / Suriname TM",                                           0 },	// Zanderij / Suriname TM
	{    31265L, 31265L, "MGI.Gauss3d-5",              "MGI / 3-degree Gauss zone 5",                                      2 },	// MGI / 3-degree Gauss zone 5
	{    31266L, 31266L, "MGI.Gauss3d-6",              "MGI / 3-degree Gauss zone 6",                                      2 },	// MGI / 3-degree Gauss zone 6
	{    31267L, 31267L, "MGI.Gauss3d-7",              "MGI / 3-degree Gauss zone 7",                                      2 },	// MGI / 3-degree Gauss zone 7
	{    31268L, 31268L, "MGI.Gauss3d-8",              "MGI / 3-degree Gauss zone 8",                                      2 },	// MGI / 3-degree Gauss zone 8
	{    31275L, 31275L, "MGI.Balkans-5",              "MGI / Balkans zone 5",                                             0 },	// MGI / Balkans zone 5
	{    31276L, 31276L, "MGI.Balkans-6",              "MGI / Balkans zone 6",                                             0 },	// MGI / Balkans zone 6
	{    31277L, 31277L, "MGI.Balkans-7",              "MGI / Balkans zone 7",                                             0 },	// MGI / Balkans zone 7
	{    31278L, 31278L, "MGI.Balkans-8",              "MGI / Balkans zone 8",                                             2 },	// MGI / Balkans zone 8
	{    31281L, 31281L, "MGI/7P.AustriaWest",         "MGI (Ferro) / Austria West Zone",                                  0 },
	{    31282L, 31282L, "MGI/7P.AustriaCntrl",        "MGI (Ferro) / Austria Central Zone",                               0 },
	{    31283L, 31283L, "MGI/7P.AustriaEast",         "MGI (Ferro) / Austria East Zone",                                  0 },
	{    31284L, 31284L, "MGI/7P.M28",                 "MGI / M28",                                                        0 },	// MGI / M28
	{    31285L, 31285L, "MGI/7P.M31",                 "MGI / M31",                                                        0 },	// MGI / M31
	{    31286L, 31286L, "MGI/7P.M34",                 "MGI / M34",                                                        0 },	// MGI / M34
	{    31297L, 31287L, "MGI/7P.AustriaLambert",      "MGI / Austria Lambert",                                            0 },	// MGI / Austria Lambert
	{    31300L, 31300L, "Belge72.Lambert72",          "Belge 1972 / Belge Lambert 72",                                    0 },	// Belge 1972 / Belge Lambert 72
	{    31370L, 31370L, "Belge72.Lambert72A",         "Belge 1972 / Belgian Lambert 72",                                  0 },	// Belge 1972 / Belgian Lambert 72
	{    31466L, 31466L, "DHDN.Gauss3d-2",             "DHDN / Gauss-Kruger zone 2",                                       0 },	// DHDN / Gauss-Kruger zone 2
	{    31467L, 31467L, "DHDN.Gauss3d-3",             "DHDN / Gauss-Kruger zone 3",                                       0 },	// DHDN / Gauss-Kruger zone 3
	{    31468L, 31468L, "DHDN.Gauss3d-4",             "DHDN / Gauss-Kruger zone 4",                                       0 },	// DHDN / Gauss-Kruger zone 4
	{    31469L, 31469L, "DHDN.Gauss3d-5",             "DHDN / Gauss-Kruger zone 5",                                       0 },	// DHDN / Gauss-Kruger zone 5
	{    31528L, 31528L, "Conakry05.UTM-28N",          "Conakry 1905 / UTM zone 28N",                                      0 },	// Conakry 1905 / UTM zone 28N
	{    31529L, 31529L, "Conakry05.UTM-29N",          "Conakry 1905 / UTM zone 29N",                                      0 },	// Conakry 1905 / UTM zone 29N
	{    31600L, 31600L, "DealulPiscului33.Stero",     "Dealul Piscului 1933/ Stereo 33",                                  0 },	// Dealul Piscului 1933/ Stereo 33
	{    31700L, 31700L, "DealulPiscului70.Stero",     "Dealul Piscului 1970/ Stereo 70",                                  0 },	// Dealul Piscului 1970/ Stereo 70
	{    31838L, 31838L, "NGN.UTM-38N",                "NGN / UTM zone 38N",                                               0 },	// NGN / UTM zone 38N
	{    31839L, 31839L, "NGN.UTM-39N",                "NGN / UTM zone 39N",                                               0 },	// NGN / UTM zone 39N
	{    31986L, 31986L, "GRSSA.UTM-17N",              "SIRGAS / UTM zone 17N",                                            0 },	// SIRGAS / UTM zone 17N
	{    31987L, 31987L, "GRSSA.UTM-18N",              "SIRGAS / UTM zone 18N",                                            0 },	// SIRGAS / UTM zone 18N
	{    31988L, 31988L, "GRSSA.UTM-19N",              "SIRGAS / UTM zone 19N",                                            0 },	// SIRGAS / UTM zone 19N
	{    31989L, 31989L, "GRSSA.UTM-20N",              "SIRGAS / UTM zone 20N",                                            0 },	// SIRGAS / UTM zone 20N
	{    31990L, 31990L, "GRSSA.UTM-21N",              "SIRGAS / UTM zone 21N",                                            0 },	// SIRGAS / UTM zone 21N
	{    31991L, 31991L, "GRSSA.UTM-22N",              "SIRGAS / UTM zone 22N",                                            0 },	// SIRGAS / UTM zone 22N
	{    31992L, 31992L, "GRSSA.UTM-17S",              "SIRGAS / UTM zone 17S",                                            0 },	// SIRGAS / UTM zone 17S
	{    31993L, 31993L, "GRSSA.UTM-18S",              "SIRGAS / UTM zone 18S",                                            0 },	// SIRGAS / UTM zone 18S
	{    31994L, 31994L, "GRSSA.UTM-19S",              "SIRGAS / UTM zone 19S",                                            0 },	// SIRGAS / UTM zone 19S
	{    31995L, 31995L, "GRSSA.UTM-20S",              "SIRGAS / UTM zone 20S",                                            0 },	// SIRGAS / UTM zone 20S
	{    31996L, 31996L, "GRSSA.UTM-21S",              "SIRGAS / UTM zone 21S",                                            0 },	// SIRGAS / UTM zone 21S
	{    31997L, 31997L, "GRSSA.UTM-22S",              "SIRGAS / UTM zone 22S",                                            0 },	// SIRGAS / UTM zone 22S
	{    31998L, 31998L, "GRSSA.UTM-23S",              "SIRGAS / UTM zone 23S",                                            0 },	// SIRGAS / UTM zone 23S
	{    31999L, 31999L, "GRSSA.UTM-24S",              "SIRGAS / UTM zone 24S",                                            0 },	// SIRGAS / UTM zone 24S
	{    32000L, 32000L, "GRSSA.UTM-25S",              "SIRGAS / UTM zone 25S",                                            0 },	// SIRGAS / UTM zone 25S
	{    32001L, 32001L, "MT-N",                       "NAD27 / Montana North",                                            1 },	// NAD27 / Montana North
	{    32002L, 32002L, "MT-C",                       "NAD27 / Montana Central",                                          1 },	// NAD27 / Montana Central
	{    32003L, 32003L, "MT-S",                       "NAD27 / Montana South",                                            1 },	// NAD27 / Montana South
	{    32005L, 32005L, "NB-N",                       "NAD27 / Nebraska North",                                           1 },	// NAD27 / Nebraska North
	{    32006L, 32006L, "NB-S",                       "NAD27 / Nebraska South",                                           1 },	// NAD27 / Nebraska South
	{    32007L, 32007L, "NV-E",                       "NAD27 / Nevada East",                                              1 },	// NAD27 / Nevada East
	{    32008L, 32008L, "NV-C",                       "NAD27 / Nevada Central",                                           1 },	// NAD27 / Nevada Central
	{    32009L, 32009L, "NV-W",                       "NAD27 / Nevada West",                                              1 },	// NAD27 / Nevada West
	{    32010L, 32010L, "NH",                         "NAD27 / New Hampshire",                                            1 },	// NAD27 / New Hampshire
	{    32011L, 32011L, "NJ",                         "NAD27 / New Jersey",                                               1 },	// NAD27 / New Jersey
	{    32012L, 32012L, "NM-E",                       "NAD27 / New Mexico East",                                          1 },	// NAD27 / New Mexico East
	{    32013L, 32013L, "NM-C",                       "NAD27 / New Mexico Central",                                       1 },	// NAD27 / New Mexico Central
	{    32014L, 32014L, "NM-W",                       "NAD27 / New Mexico West",                                          1 },	// NAD27 / New Mexico West
	{    32015L, 32015L, "NY-E",                       "NAD27 / New York East",                                            1 },	// NAD27 / New York East
	{    32016L, 32016L, "NY-C",                       "NAD27 / New York Central",                                         1 },	// NAD27 / New York Central
	{    32017L, 32017L, "NY-W",                       "NAD27 / New York West",                                            1 },	// NAD27 / New York West
	{    32018L, 32018L, "NY-LI",                      "NAD27 / New York Long Island",                                     1 },	// NAD27 / New York Long Island
	{    32019L, 32019L, "NC",                         "NAD27 / North Carolina",                                           1 },	// NAD27 / North Carolina
	{    32020L, 32020L, "ND-N",                       "NAD27 / North Dakota North",                                       1 },	// NAD27 / North Dakota North
	{    32021L, 32021L, "ND-S",                       "NAD27 / North Dakota South",                                       1 },	// NAD27 / North Dakota South
	{    32022L, 32022L, "OH-N",                       "NAD27 / Ohio North",                                               1 },	// NAD27 / Ohio North
	{    32023L, 32023L, "OH-S",                       "NAD27 / Ohio South",                                               1 },	// NAD27 / Ohio South
	{    32024L, 32024L, "OK-N",                       "NAD27 / Oklahoma North",                                           1 },	// NAD27 / Oklahoma North
	{    32025L, 32025L, "OK-S",                       "NAD27 / Oklahoma South",                                           1 },	// NAD27 / Oklahoma South
	{    32026L, 32026L, "OR-N",                       "NAD27 / Oregon North",                                             1 },	// NAD27 / Oregon North
	{    32027L, 32027L, "OR-S",                       "NAD27 / Oregon South",                                             1 },	// NAD27 / Oregon South
	{    32028L, 32028L, "PA-N",                       "NAD27 / Pennsylvania North",                                       1 },	// NAD27 / Pennsylvania North
	{    32029L, 32029L, "PA-S",                       "NAD27 / Pennsylvania South",                                       1 },	// NAD27 / Pennsylvania South
	{    32030L, 32030L, "RI",                         "NAD27 / Rhode Island",                                             1 },	// NAD27 / Rhode Island
	{    32031L, 32031L, "SC-N",                       "NAD27 / South Carolina North",                                     1 },	// NAD27 / South Carolina North
	{    32033L, 32033L, "SC-S",                       "NAD27 / South Carolina South",                                     1 },	// NAD27 / South Carolina South
	{    32034L, 32034L, "SD-N",                       "NAD27 / South Dakota North",                                       1 },	// NAD27 / South Dakota North
	{    32035L, 32035L, "SD-S",                       "NAD27 / South Dakota South",                                       1 },	// NAD27 / South Dakota South
	{    32037L, 32037L, "TX-N",                       "NAD27 / Texas North",                                              1 },	// NAD27 / Texas North
	{    32038L, 32038L, "TX-NC",                      "NAD27 / Texas North Central",                                      1 },	// NAD27 / Texas North Central
	{    32039L, 32039L, "TX-C",                       "NAD27 / Texas Central",                                            1 },	// NAD27 / Texas Central
	{    32040L, 32040L, "TX-SC",                      "NAD27 / Texas South Central",                                      1 },	// NAD27 / Texas South Central
	{    32041L, 32041L, "TX-S",                       "NAD27 / Texas South",                                              1 },	// NAD27 / Texas South
	{    32042L, 32042L, "UT-N",                       "NAD27 / Utah North",                                               1 },	// NAD27 / Utah North
	{    32043L, 32043L, "UT-C",                       "NAD27 / Utah Central",                                             1 },	// NAD27 / Utah Central
	{    32044L, 32044L, "UT-S",                       "NAD27 / Utah South",                                               1 },	// NAD27 / Utah South
	{    32045L, 32045L, "VT",                         "NAD27 / Vermont",                                                  1 },	// NAD27 / Vermont
	{    32046L, 32046L, "VA-N",                       "NAD27 / Virginia North",                                           1 },	// NAD27 / Virginia North
	{    32047L, 32047L, "VA-S",                       "NAD27 / Virginia South",                                           1 },	// NAD27 / Virginia South
	{    32048L, 32048L, "WA-N",                       "NAD27 / Washington North",                                         1 },	// NAD27 / Washington North
	{    32049L, 32049L, "WA-S",                       "NAD27 / Washington South",                                         1 },	// NAD27 / Washington South
	{    32050L, 32050L, "WV-N",                       "NAD27 / West Virginia North",                                      1 },	// NAD27 / West Virginia North
	{    32051L, 32051L, "WV-S",                       "NAD27 / West Virginia South",                                      1 },	// NAD27 / West Virginia South
	{    32052L, 32052L, "WI-N",                       "NAD27 / Wisconsin North",                                          1 },	// NAD27 / Wisconsin North
	{    32053L, 32053L, "WI-C",                       "NAD27 / Wisconsin Central",                                        1 },	// NAD27 / Wisconsin Central
	{    32054L, 32054L, "WI-S",                       "NAD27 / Wisconsin South",                                          1 },	// NAD27 / Wisconsin South
	{    32055L, 32055L, "WY-E",                       "NAD27 / Wyoming East",                                             1 },	// NAD27 / Wyoming East
	{    32056L, 32056L, "WY-EC",                      "NAD27 / Wyoming East Central",                                     1 },	// NAD27 / Wyoming East Central
	{    32057L, 32057L, "WY-WC",                      "NAD27 / Wyoming West Central",                                     1 },	// NAD27 / Wyoming West Central
	{    32058L, 32058L, "WY-W",                       "NAD27 / Wyoming West",                                             1 },	// NAD27 / Wyoming West
	{    32061L, 32061L, "NAD27.GuatemalaN",           "NAD27 / Guatemala Norte",                                          0 },	// NAD27 / Guatemala Norte
	{    32062L, 32062L, "NAD27.GuatemalaS",           "NAD27 / Guatemala Sur",                                            0 },	// NAD27 / Guatemala Sur
	{    32064L, 32064L, "NAD27.BLM-14N.ft",           "NAD27 / BLM 14N (ftUS)",                                           0 },	// NAD27 / BLM 14N (ftUS)
	{    32065L, 32065L, "NAD27.BLM-15N.ft",           "NAD27 / BLM 15N (ftUS)",                                           0 },	// NAD27 / BLM 15N (ftUS)
	{    32066L, 32066L, "NAD27.BLM-16N.ft",           "NAD27 / BLM 16N (ftUS)",                                           0 },	// NAD27 / BLM 16N (ftUS)
	{    32067L, 32067L, "NAD27.BLM-17N.ft",           "NAD27 / BLM 17N (ftUS)",                                           0 },	// NAD27 / BLM 17N (ftUS)
	{    32074L, 32074L, "NAD27.BLM-14N.Ift",          "NAD27 / BLM 14N (feet)",                                           2 },	// NAD27 / BLM 14N (feet)
	{    32075L, 32075L, "NAD27.BLM-15N.Ift",          "NAD27 / BLM 15N (feet)",                                           2 },	// NAD27 / BLM 15N (feet)
	{    32076L, 32076L, "NAD27.BLM-16N.Ift",          "NAD27 / BLM 16N (feet)",                                           2 },	// NAD27 / BLM 16N (feet)
	{    32077L, 32077L, "NAD27.BLM-17N.Ift",          "NAD27 / BLM 17N (feet)",                                           2 },	// NAD27 / BLM 17N (feet)
	{    32081L, 32081L, "NAD27.MTM-1",                "NAD27 / MTM zone 1",                                               0 },	// NAD27 / MTM zone 1
	{    32082L, 32082L, "NAD27.MTM-2",                "NAD27 / MTM zone 2",                                               0 },	// NAD27 / MTM zone 2
	{    32083L, 32083L, "MTM27-3",                    "NAD27 / MTM zone 3",                                               1 },	// NAD27 / MTM zone 3
	{    32084L, 32084L, "MTM27-4",                    "NAD27 / MTM zone 4",                                               1 },	// NAD27 / MTM zone 4
	{    32085L, 32085L, "MTM27-5",                    "NAD27 / MTM zone 5",                                               1 },	// NAD27 / MTM zone 5
	{    32086L, 32086L, "MTM27-6",                    "NAD27 / MTM zone 6",                                               1 },	// NAD27 / MTM zone 6
	{    32098L, 32098L, "NAD27.QuebecLambert",        "NAD27 / Quebec Lambert",                                           0 },	// NAD27 / Quebec Lambert
	{    32100L, 32100L, "MT83",                       "NAD83 / Montana",                                                  1 },	// NAD83 / Montana
	{    32104L, 32104L, "NB83",                       "NAD83 / Nebraska",                                                 1 },	// NAD83 / Nebraska
	{    32107L, 32107L, "NV83-E",                     "NAD83 / Nevada East",                                              1 },	// NAD83 / Nevada East
	{    32108L, 32108L, "NV83-C",                     "NAD83 / Nevada Central",                                           1 },	// NAD83 / Nevada Central
	{    32109L, 32109L, "NV83-W",                     "NAD83 / Nevada West",                                              1 },	// NAD83 / Nevada West
	{    32110L, 32110L, "NH83",                       "NAD83 / New Hampshire",                                            1 },	// NAD83 / New Hampshire
	{    32111L, 32111L, "NJ83",                       "NAD83 / New Jersey",                                               1 },	// NAD83 / New Jersey
	{    32112L, 32112L, "NM83-E",                     "NAD83 / New Mexico East",                                          1 },	// NAD83 / New Mexico East
	{    32113L, 32113L, "NM83-C",                     "NAD83 / New Mexico Central",                                       1 },	// NAD83 / New Mexico Central
	{    32114L, 32114L, "NM83-W",                     "NAD83 / New Mexico West",                                          1 },	// NAD83 / New Mexico West
	{    32115L, 32115L, "NY83-E",                     "NAD83 / New York East",                                            1 },	// NAD83 / New York East
	{    32116L, 32116L, "NY83-C",                     "NAD83 / New York Central",                                         1 },	// NAD83 / New York Central
	{    32117L, 32117L, "NY83-W",                     "NAD83 / New York West",                                            1 },	// NAD83 / New York West
	{    32118L, 32118L, "NY83-LI",                    "NAD83 / New York Long Island",                                     1 },	// NAD83 / New York Long Island
	{    32119L, 32119L, "NC83",                       "NAD83 / North Carolina",                                           1 },	// NAD83 / North Carolina
	{    32120L, 32120L, "ND83-N",                     "NAD83 / North Dakota North",                                       1 },	// NAD83 / North Dakota North
	{    32121L, 32121L, "ND83-S",                     "NAD83 / North Dakota South",                                       1 },	// NAD83 / North Dakota South
	{    32122L, 32122L, "OH83-N",                     "NAD83 / Ohio North",                                               1 },	// NAD83 / Ohio North
	{    32123L, 32123L, "OH83-S",                     "NAD83 / Ohio South",                                               1 },	// NAD83 / Ohio South
	{    32124L, 32124L, "OK83-N",                     "NAD83 / Oklahoma North",                                           1 },	// NAD83 / Oklahoma North
	{    32125L, 32125L, "OK83-S",                     "NAD83 / Oklahoma South",                                           1 },	// NAD83 / Oklahoma South
	{    32126L, 32126L, "OR83-N",                     "NAD83 / Oregon North",                                             1 },	// NAD83 / Oregon North
	{    32127L, 32127L, "OR83-S",                     "NAD83 / Oregon South",                                             1 },	// NAD83 / Oregon South
	{    32128L, 32128L, "PA83-N",                     "NAD83 / Pennsylvania North",                                       1 },	// NAD83 / Pennsylvania North
	{    32129L, 32129L, "PA83-S",                     "NAD83 / Pennsylvania South",                                       1 },	// NAD83 / Pennsylvania South
	{    32130L, 32130L, "RI83",                       "NAD83 / Rhode Island",                                             1 },	// NAD83 / Rhode Island
	{    32133L, 32133L, "SC83",                       "NAD83 / South Carolina",                                           1 },	// NAD83 / South Carolina
	{    32134L, 32134L, "SD83-N",                     "NAD83 / South Dakota North",                                       1 },	// NAD83 / South Dakota North
	{    32135L, 32135L, "SD83-S",                     "NAD83 / South Dakota South",                                       1 },	// NAD83 / South Dakota South
	{    32136L, 32136L, "TN83",                       "NAD83 / Tennessee",                                                1 },	// NAD83 / Tennessee
	{    32137L, 32137L, "TX83-N",                     "NAD83 / Texas North",                                              1 },	// NAD83 / Texas North
	{    32138L, 32138L, "TX83-NC",                    "NAD83 / Texas North Central",                                      1 },	// NAD83 / Texas North Central
	{    32139L, 32139L, "TX83-C",                     "NAD83 / Texas Central",                                            1 },	// NAD83 / Texas Central
	{    32140L, 32140L, "TX83-SC",                    "NAD83 / Texas South Central",                                      1 },	// NAD83 / Texas South Central
	{    32141L, 32141L, "TX83-S",                     "NAD83 / Texas South",                                              1 },	// NAD83 / Texas South
	{    32142L, 32142L, "UT83-N",                     "NAD83 / Utah North",                                               1 },	// NAD83 / Utah North
	{    32143L, 32143L, "UT83-C",                     "NAD83 / Utah Central",                                             1 },	// NAD83 / Utah Central
	{    32144L, 32144L, "UT83-S",                     "NAD83 / Utah South",                                               1 },	// NAD83 / Utah South
	{    32145L, 32145L, "VT83",                       "NAD83 / Vermont",                                                  1 },	// NAD83 / Vermont
	{    32146L, 32146L, "VA83-N",                     "NAD83 / Virginia North",                                           1 },	// NAD83 / Virginia North
	{    32147L, 32147L, "VA83-S",                     "NAD83 / Virginia South",                                           1 },	// NAD83 / Virginia South
	{    32148L, 32148L, "WA83-N",                     "NAD83 / Washington North",                                         1 },	// NAD83 / Washington North
	{    32149L, 32149L, "WA83-S",                     "NAD83 / Washington South",                                         1 },	// NAD83 / Washington South
	{    32150L, 32150L, "WV83-N",                     "NAD83 / West Virginia North",                                      1 },	// NAD83 / West Virginia North
	{    32151L, 32151L, "WV83-S",                     "NAD83 / West Virginia South",                                      1 },	// NAD83 / West Virginia South
	{    32152L, 32152L, "WI83-N",                     "NAD83 / Wisconsin North",                                          1 },	// NAD83 / Wisconsin North
	{    32153L, 32153L, "WI83-C",                     "NAD83 / Wisconsin Central",                                        1 },	// NAD83 / Wisconsin Central
	{    32154L, 32154L, "WI83-S",                     "NAD83 / Wisconsin South",                                          1 },	// NAD83 / Wisconsin South
	{    32155L, 32155L, "WY83-E",                     "NAD83 / Wyoming East",                                             1 },	// NAD83 / Wyoming East
	{    32156L, 32156L, "WY83-EC",                    "NAD83 / Wyoming East Central",                                     1 },	// NAD83 / Wyoming East Central
	{    32157L, 32157L, "WY83-WC",                    "NAD83 / Wyoming West Central",                                     1 },	// NAD83 / Wyoming West Central
	{    32158L, 32158L, "WY83-W",                     "NAD83 / Wyoming West",                                             1 },	// NAD83 / Wyoming West
	{    32161L, 32161L, "PR83",                       "NAD83 / Puerto Rico & Virgin Is.",                                 1 },	// NAD83 / Puerto Rico & Virgin Is.
	{    32180L, 32180L, "NAD83.SCoPQ-2",              "NAD83 / SCoPQ zone 2",                                             0 },	// NAD83 / SCoPQ zone 2
	{    32181L, 32181L, "MTM83-1",                    "NAD83 / MTM zone 1",                                               1 },	// NAD83 / MTM zone 1
	{    32182L, 32182L, "MTM83-2",                    "NAD83 / MTM zone 2",                                               1 },	// NAD83 / MTM zone 2
	{    32183L, 32183L, "MTM83-3",                    "NAD83 / MTM zone 3",                                               1 },	// NAD83 / MTM zone 3
	{    32184L, 32184L, "MTM83-4",                    "NAD83 / MTM zone 4",                                               1 },	// NAD83 / MTM zone 4
	{    32185L, 32185L, "MTM83-5",                    "NAD83 / MTM zone 5",                                               1 },	// NAD83 / MTM zone 5
	{    32186L, 32186L, "MTM83-6",                    "NAD83 / MTM zone 6",                                               1 },	// NAD83 / MTM zone 6
	{    32187L, 32187L, "MTM83-7",                    "NAD83 / MTM zone 7",                                               1 },	// NAD83 / MTM zone 7
	{    32188L, 32188L, "MTM83-8",                    "NAD83 / MTM zone 8",                                               1 },	// NAD83 / MTM zone 8
	{    32189L, 32189L, "MTM83-9",                    "NAD83 / MTM zone 9",                                               1 },	// NAD83 / MTM zone 9
	{    32190L, 32190L, "MTM83-10",                   "NAD83 / MTM zone 10",                                              1 },	// NAD83 / MTM zone 10
	{    32191L, 32191L, "MTM83-11",                   "NAD83 / MTM zone 11",                                              1 },	// NAD83 / MTM zone 11
	{    32192L, 32192L, "MTM83-12",                   "NAD83 / MTM zone 12",                                              1 },	// NAD83 / MTM zone 12
	{    32193L, 32193L, "MTM83-13",                   "NAD83 / MTM zone 13",                                              1 },	// NAD83 / MTM zone 13
	{    32194L, 32194L, "MTM83-14",                   "NAD83 / MTM zone 14",                                              1 },	// NAD83 / MTM zone 14
	{    32195L, 32195L, "MTM83-15",                   "NAD83 / MTM zone 15",                                              1 },	// NAD83 / MTM zone 15
	{    32196L, 32196L, "MTM83-16",                   "NAD83 / MTM zone 16",                                              1 },	// NAD83 / MTM zone 16
	{    32197L, 32197L, "MTM83-17",                   "NAD83 / MTM zone 17",                                              1 },	// NAD83 / MTM zone 17
	{    32198L, 32198L, "NAD83.QuebecLambert",        "NAD83 / Quebec Lambert",                                           0 },	// NAD83 / Quebec Lambert
	{    32201L, 32201L, "WGS72.UTM-1N",               "WGS 72 / UTM zone 1N",                                             0 },	// WGS 72 / UTM zone 1N
	{    32202L, 32202L, "WGS72.UTM-2N",               "WGS 72 / UTM zone 2N",                                             0 },	// WGS 72 / UTM zone 2N
	{    32203L, 32203L, "WGS72.UTM-3N",               "WGS 72 / UTM zone 3N",                                             0 },	// WGS 72 / UTM zone 3N
	{    32204L, 32204L, "WGS72.UTM-4N",               "WGS 72 / UTM zone 4N",                                             0 },	// WGS 72 / UTM zone 4N
	{    32205L, 32205L, "WGS72.UTM-5N",               "WGS 72 / UTM zone 5N",                                             0 },	// WGS 72 / UTM zone 5N
	{    32206L, 32206L, "WGS72.UTM-6N",               "WGS 72 / UTM zone 6N",                                             0 },	// WGS 72 / UTM zone 6N
	{    32207L, 32207L, "WGS72.UTM-7N",               "WGS 72 / UTM zone 7N",                                             0 },	// WGS 72 / UTM zone 7N
	{    32208L, 32208L, "WGS72.UTM-8N",               "WGS 72 / UTM zone 8N",                                             0 },	// WGS 72 / UTM zone 8N
	{    32209L, 32209L, "WGS72.UTM-9N",               "WGS 72 / UTM zone 9N",                                             0 },	// WGS 72 / UTM zone 9N
	{    32210L, 32210L, "WGS72.UTM-10N",              "WGS 72 / UTM zone 10N",                                            0 },	// WGS 72 / UTM zone 10N
	{    32211L, 32211L, "WGS72.UTM-11N",              "WGS 72 / UTM zone 11N",                                            0 },	// WGS 72 / UTM zone 11N
	{    32212L, 32212L, "WGS72.UTM-12N",              "WGS 72 / UTM zone 12N",                                            0 },	// WGS 72 / UTM zone 12N
	{    32213L, 32213L, "WGS72.UTM-13N",              "WGS 72 / UTM zone 13N",                                            0 },	// WGS 72 / UTM zone 13N
	{    32214L, 32214L, "WGS72.UTM-14N",              "WGS 72 / UTM zone 14N",                                            0 },	// WGS 72 / UTM zone 14N
	{    32215L, 32215L, "WGS72.UTM-15N",              "WGS 72 / UTM zone 15N",                                            0 },	// WGS 72 / UTM zone 15N
	{    32216L, 32216L, "WGS72.UTM-16N",              "WGS 72 / UTM zone 16N",                                            0 },	// WGS 72 / UTM zone 16N
	{    32217L, 32217L, "WGS72.UTM-17N",              "WGS 72 / UTM zone 17N",                                            0 },	// WGS 72 / UTM zone 17N
	{    32218L, 32218L, "WGS72.UTM-18N",              "WGS 72 / UTM zone 18N",                                            0 },	// WGS 72 / UTM zone 18N
	{    32219L, 32219L, "WGS72.UTM-19N",              "WGS 72 / UTM zone 19N",                                            0 },	// WGS 72 / UTM zone 19N
	{    32220L, 32220L, "WGS72.UTM-20N",              "WGS 72 / UTM zone 20N",                                            0 },	// WGS 72 / UTM zone 20N
	{    32221L, 32221L, "WGS72.UTM-21N",              "WGS 72 / UTM zone 21N",                                            0 },	// WGS 72 / UTM zone 21N
	{    32222L, 32222L, "WGS72.UTM-22N",              "WGS 72 / UTM zone 22N",                                            0 },	// WGS 72 / UTM zone 22N
	{    32223L, 32223L, "WGS72.UTM-23N",              "WGS 72 / UTM zone 23N",                                            0 },	// WGS 72 / UTM zone 23N
	{    32224L, 32224L, "WGS72.UTM-24N",              "WGS 72 / UTM zone 24N",                                            0 },	// WGS 72 / UTM zone 24N
	{    32225L, 32225L, "WGS72.UTM-25N",              "WGS 72 / UTM zone 25N",                                            0 },	// WGS 72 / UTM zone 25N
	{    32226L, 32226L, "WGS72.UTM-26N",              "WGS 72 / UTM zone 26N",                                            0 },	// WGS 72 / UTM zone 26N
	{    32227L, 32227L, "WGS72.UTM-27N",              "WGS 72 / UTM zone 27N",                                            0 },	// WGS 72 / UTM zone 27N
	{    32228L, 32228L, "WGS72.UTM-28N",              "WGS 72 / UTM zone 28N",                                            0 },	// WGS 72 / UTM zone 28N
	{    32229L, 32229L, "WGS72.UTM-29N",              "WGS 72 / UTM zone 29N",                                            0 },	// WGS 72 / UTM zone 29N
	{    32230L, 32230L, "WGS72.UTM-30N",              "WGS 72 / UTM zone 30N",                                            0 },	// WGS 72 / UTM zone 30N
	{    32231L, 32231L, "WGS72.UTM-31N",              "WGS 72 / UTM zone 31N",                                            0 },	// WGS 72 / UTM zone 31N
	{    32232L, 32232L, "WGS72.UTM-32N",              "WGS 72 / UTM zone 32N",                                            0 },	// WGS 72 / UTM zone 32N
	{    32233L, 32233L, "WGS72.UTM-33N",              "WGS 72 / UTM zone 33N",                                            0 },	// WGS 72 / UTM zone 33N
	{    32234L, 32234L, "WGS72.UTM-34N",              "WGS 72 / UTM zone 34N",                                            0 },	// WGS 72 / UTM zone 34N
	{    32235L, 32235L, "WGS72.UTM-35N",              "WGS 72 / UTM zone 35N",                                            0 },	// WGS 72 / UTM zone 35N
	{    32236L, 32236L, "WGS72.UTM-36N",              "WGS 72 / UTM zone 36N",                                            0 },	// WGS 72 / UTM zone 36N
	{    32237L, 32237L, "WGS72.UTM-37N",              "WGS 72 / UTM zone 37N",                                            0 },	// WGS 72 / UTM zone 37N
	{    32238L, 32238L, "WGS72.UTM-38N",              "WGS 72 / UTM zone 38N",                                            0 },	// WGS 72 / UTM zone 38N
	{    32239L, 32239L, "WGS72.UTM-39N",              "WGS 72 / UTM zone 39N",                                            0 },	// WGS 72 / UTM zone 39N
	{    32240L, 32240L, "WGS72.UTM-40N",              "WGS 72 / UTM zone 40N",                                            0 },	// WGS 72 / UTM zone 40N
	{    32241L, 32241L, "WGS72.UTM-41N",              "WGS 72 / UTM zone 41N",                                            0 },	// WGS 72 / UTM zone 41N
	{    32242L, 32242L, "WGS72.UTM-42N",              "WGS 72 / UTM zone 42N",                                            0 },	// WGS 72 / UTM zone 42N
	{    32243L, 32243L, "WGS72.UTM-43N",              "WGS 72 / UTM zone 43N",                                            0 },	// WGS 72 / UTM zone 43N
	{    32244L, 32244L, "WGS72.UTM-44N",              "WGS 72 / UTM zone 44N",                                            0 },	// WGS 72 / UTM zone 44N
	{    32245L, 32245L, "WGS72.UTM-45N",              "WGS 72 / UTM zone 45N",                                            0 },	// WGS 72 / UTM zone 45N
	{    32246L, 32246L, "WGS72.UTM-46N",              "WGS 72 / UTM zone 46N",                                            0 },	// WGS 72 / UTM zone 46N
	{    32247L, 32247L, "WGS72.UTM-47N",              "WGS 72 / UTM zone 47N",                                            0 },	// WGS 72 / UTM zone 47N
	{    32248L, 32248L, "WGS72.UTM-48N",              "WGS 72 / UTM zone 48N",                                            0 },	// WGS 72 / UTM zone 48N
	{    32249L, 32249L, "WGS72.UTM-49N",              "WGS 72 / UTM zone 49N",                                            0 },	// WGS 72 / UTM zone 49N
	{    32250L, 32250L, "WGS72.UTM-50N",              "WGS 72 / UTM zone 50N",                                            0 },	// WGS 72 / UTM zone 50N
	{    32251L, 32251L, "WGS72.UTM-51N",              "WGS 72 / UTM zone 51N",                                            0 },	// WGS 72 / UTM zone 51N
	{    32252L, 32252L, "WGS72.UTM-52N",              "WGS 72 / UTM zone 52N",                                            0 },	// WGS 72 / UTM zone 52N
	{    32253L, 32253L, "WGS72.UTM-53N",              "WGS 72 / UTM zone 53N",                                            0 },	// WGS 72 / UTM zone 53N
	{    32254L, 32254L, "WGS72.UTM-54N",              "WGS 72 / UTM zone 54N",                                            0 },	// WGS 72 / UTM zone 54N
	{    32255L, 32255L, "WGS72.UTM-55N",              "WGS 72 / UTM zone 55N",                                            0 },	// WGS 72 / UTM zone 55N
	{    32256L, 32256L, "WGS72.UTM-56N",              "WGS 72 / UTM zone 56N",                                            0 },	// WGS 72 / UTM zone 56N
	{    32257L, 32257L, "WGS72.UTM-57N",              "WGS 72 / UTM zone 57N",                                            0 },	// WGS 72 / UTM zone 57N
	{    32258L, 32258L, "WGS72.UTM-58N",              "WGS 72 / UTM zone 58N",                                            0 },	// WGS 72 / UTM zone 58N
	{    32259L, 32259L, "WGS72.UTM-59N",              "WGS 72 / UTM zone 59N",                                            0 },	// WGS 72 / UTM zone 59N
	{    32260L, 32260L, "WGS72.UTM-60N",              "WGS 72 / UTM zone 60N",                                            0 },	// WGS 72 / UTM zone 60N
	{    32301L, 32301L, "WGS72.UTM-1S",               "WGS 72 / UTM zone 1S",                                             0 },	// WGS 72 / UTM zone 1S
	{    32302L, 32302L, "WGS72.UTM-2S",               "WGS 72 / UTM zone 2S",                                             0 },	// WGS 72 / UTM zone 2S
	{    32303L, 32303L, "WGS72.UTM-3S",               "WGS 72 / UTM zone 3S",                                             0 },	// WGS 72 / UTM zone 3S
	{    32304L, 32304L, "WGS72.UTM-4S",               "WGS 72 / UTM zone 4S",                                             0 },	// WGS 72 / UTM zone 4S
	{    32305L, 32305L, "WGS72.UTM-5S",               "WGS 72 / UTM zone 5S",                                             0 },	// WGS 72 / UTM zone 5S
	{    32306L, 32306L, "WGS72.UTM-6S",               "WGS 72 / UTM zone 6S",                                             0 },	// WGS 72 / UTM zone 6S
	{    32307L, 32307L, "WGS72.UTM-7S",               "WGS 72 / UTM zone 7S",                                             0 },	// WGS 72 / UTM zone 7S
	{    32308L, 32308L, "WGS72.UTM-8S",               "WGS 72 / UTM zone 8S",                                             0 },	// WGS 72 / UTM zone 8S
	{    32309L, 32309L, "WGS72.UTM-9S",               "WGS 72 / UTM zone 9S",                                             0 },	// WGS 72 / UTM zone 9S
	{    32310L, 32310L, "WGS72.UTM-10S",              "WGS 72 / UTM zone 10S",                                            0 },	// WGS 72 / UTM zone 10S
	{    32311L, 32311L, "WGS72.UTM-11S",              "WGS 72 / UTM zone 11S",                                            0 },	// WGS 72 / UTM zone 11S
	{    32312L, 32312L, "WGS72.UTM-12S",              "WGS 72 / UTM zone 12S",                                            0 },	// WGS 72 / UTM zone 12S
	{    32313L, 32313L, "WGS72.UTM-13S",              "WGS 72 / UTM zone 13S",                                            0 },	// WGS 72 / UTM zone 13S
	{    32314L, 32314L, "WGS72.UTM-14S",              "WGS 72 / UTM zone 14S",                                            0 },	// WGS 72 / UTM zone 14S
	{    32315L, 32315L, "WGS72.UTM-15S",              "WGS 72 / UTM zone 15S",                                            0 },	// WGS 72 / UTM zone 15S
	{    32316L, 32316L, "WGS72.UTM-16S",              "WGS 72 / UTM zone 16S",                                            0 },	// WGS 72 / UTM zone 16S
	{    32317L, 32317L, "WGS72.UTM-17S",              "WGS 72 / UTM zone 17S",                                            0 },	// WGS 72 / UTM zone 17S
	{    32318L, 32318L, "WGS72.UTM-18S",              "WGS 72 / UTM zone 18S",                                            0 },	// WGS 72 / UTM zone 18S
	{    32319L, 32319L, "WGS72.UTM-19S",              "WGS 72 / UTM zone 19S",                                            0 },	// WGS 72 / UTM zone 19S
	{    32320L, 32320L, "WGS72.UTM-20S",              "WGS 72 / UTM zone 20S",                                            0 },	// WGS 72 / UTM zone 20S
	{    32321L, 32321L, "WGS72.UTM-21S",              "WGS 72 / UTM zone 21S",                                            0 },	// WGS 72 / UTM zone 21S
	{    32322L, 32322L, "WGS72.UTM-22S",              "WGS 72 / UTM zone 22S",                                            0 },	// WGS 72 / UTM zone 22S
	{    32323L, 32323L, "WGS72.UTM-23S",              "WGS 72 / UTM zone 23S",                                            0 },	// WGS 72 / UTM zone 23S
	{    32324L, 32324L, "WGS72.UTM-24S",              "WGS 72 / UTM zone 24S",                                            0 },	// WGS 72 / UTM zone 24S
	{    32325L, 32325L, "WGS72.UTM-25S",              "WGS 72 / UTM zone 25S",                                            0 },	// WGS 72 / UTM zone 25S
	{    32326L, 32326L, "WGS72.UTM-26S",              "WGS 72 / UTM zone 26S",                                            0 },	// WGS 72 / UTM zone 26S
	{    32327L, 32327L, "WGS72.UTM-27S",              "WGS 72 / UTM zone 27S",                                            0 },	// WGS 72 / UTM zone 27S
	{    32328L, 32328L, "WGS72.UTM-28S",              "WGS 72 / UTM zone 28S",                                            0 },	// WGS 72 / UTM zone 28S
	{    32329L, 32329L, "WGS72.UTM-29S",              "WGS 72 / UTM zone 29S",                                            0 },	// WGS 72 / UTM zone 29S
	{    32330L, 32330L, "WGS72.UTM-30S",              "WGS 72 / UTM zone 30S",                                            0 },	// WGS 72 / UTM zone 30S
	{    32331L, 32331L, "WGS72.UTM-31S",              "WGS 72 / UTM zone 31S",                                            0 },	// WGS 72 / UTM zone 31S
	{    32332L, 32332L, "WGS72.UTM-32S",              "WGS 72 / UTM zone 32S",                                            0 },	// WGS 72 / UTM zone 32S
	{    32333L, 32333L, "WGS72.UTM-33S",              "WGS 72 / UTM zone 33S",                                            0 },	// WGS 72 / UTM zone 33S
	{    32334L, 32334L, "WGS72.UTM-34S",              "WGS 72 / UTM zone 34S",                                            0 },	// WGS 72 / UTM zone 34S
	{    32335L, 32335L, "WGS72.UTM-35S",              "WGS 72 / UTM zone 35S",                                            0 },	// WGS 72 / UTM zone 35S
	{    32336L, 32336L, "WGS72.UTM-36S",              "WGS 72 / UTM zone 36S",                                            0 },	// WGS 72 / UTM zone 36S
	{    32337L, 32337L, "WGS72.UTM-37S",              "WGS 72 / UTM zone 37S",                                            0 },	// WGS 72 / UTM zone 37S
	{    32338L, 32338L, "WGS72.UTM-38S",              "WGS 72 / UTM zone 38S",                                            0 },	// WGS 72 / UTM zone 38S
	{    32339L, 32339L, "WGS72.UTM-39S",              "WGS 72 / UTM zone 39S",                                            0 },	// WGS 72 / UTM zone 39S
	{    32340L, 32340L, "WGS72.UTM-40S",              "WGS 72 / UTM zone 40S",                                            0 },	// WGS 72 / UTM zone 40S
	{    32341L, 32341L, "WGS72.UTM-41S",              "WGS 72 / UTM zone 41S",                                            0 },	// WGS 72 / UTM zone 41S
	{    32342L, 32342L, "WGS72.UTM-42S",              "WGS 72 / UTM zone 42S",                                            0 },	// WGS 72 / UTM zone 42S
	{    32343L, 32343L, "WGS72.UTM-43S",              "WGS 72 / UTM zone 43S",                                            0 },	// WGS 72 / UTM zone 43S
	{    32344L, 32344L, "WGS72.UTM-44S",              "WGS 72 / UTM zone 44S",                                            0 },	// WGS 72 / UTM zone 44S
	{    32345L, 32345L, "WGS72.UTM-45S",              "WGS 72 / UTM zone 45S",                                            0 },	// WGS 72 / UTM zone 45S
	{    32346L, 32346L, "WGS72.UTM-46S",              "WGS 72 / UTM zone 46S",                                            0 },	// WGS 72 / UTM zone 46S
	{    32347L, 32347L, "WGS72.UTM-47S",              "WGS 72 / UTM zone 47S",                                            0 },	// WGS 72 / UTM zone 47S
	{    32348L, 32348L, "WGS72.UTM-48S",              "WGS 72 / UTM zone 48S",                                            0 },	// WGS 72 / UTM zone 48S
	{    32349L, 32349L, "WGS72.UTM-49S",              "WGS 72 / UTM zone 49S",                                            0 },	// WGS 72 / UTM zone 49S
	{    32350L, 32350L, "WGS72.UTM-50S",              "WGS 72 / UTM zone 50S",                                            0 },	// WGS 72 / UTM zone 50S
	{    32351L, 32351L, "WGS72.UTM-51S",              "WGS 72 / UTM zone 51S",                                            0 },	// WGS 72 / UTM zone 51S
	{    32352L, 32352L, "WGS72.UTM-52S",              "WGS 72 / UTM zone 52S",                                            0 },	// WGS 72 / UTM zone 52S
	{    32353L, 32353L, "WGS72.UTM-53S",              "WGS 72 / UTM zone 53S",                                            0 },	// WGS 72 / UTM zone 53S
	{    32354L, 32354L, "WGS72.UTM-54S",              "WGS 72 / UTM zone 54S",                                            0 },	// WGS 72 / UTM zone 54S
	{    32355L, 32355L, "WGS72.UTM-55S",              "WGS 72 / UTM zone 55S",                                            0 },	// WGS 72 / UTM zone 55S
	{    32356L, 32356L, "WGS72.UTM-56S",              "WGS 72 / UTM zone 56S",                                            0 },	// WGS 72 / UTM zone 56S
	{    32357L, 32357L, "WGS72.UTM-57S",              "WGS 72 / UTM zone 57S",                                            0 },	// WGS 72 / UTM zone 57S
	{    32358L, 32358L, "WGS72.UTM-58S",              "WGS 72 / UTM zone 58S",                                            0 },	// WGS 72 / UTM zone 58S
	{    32359L, 32359L, "WGS72.UTM-59S",              "WGS 72 / UTM zone 59S",                                            0 },	// WGS 72 / UTM zone 59S
	{    32360L, 32360L, "WGS72.UTM-60S",              "WGS 72 / UTM zone 60S",                                            0 },	// WGS 72 / UTM zone 60S
	{    32401L, 32401L, "WGS72be.UTM-1N",             "WGS 72BE / UTM zone 1N",                                           0 },	// WGS 72BE / UTM zone 1N
	{    32402L, 32402L, "WGS72be.UTM-2N",             "WGS 72BE / UTM zone 2N",                                           0 },	// WGS 72BE / UTM zone 2N
	{    32403L, 32403L, "WGS72be.UTM-3N",             "WGS 72BE / UTM zone 3N",                                           0 },	// WGS 72BE / UTM zone 3N
	{    32404L, 32404L, "WGS72be.UTM-4N",             "WGS 72BE / UTM zone 4N",                                           0 },	// WGS 72BE / UTM zone 4N
	{    32405L, 32405L, "WGS72be.UTM-5N",             "WGS 72BE / UTM zone 5N",                                           0 },	// WGS 72BE / UTM zone 5N
	{    32406L, 32406L, "WGS72be.UTM-6N",             "WGS 72BE / UTM zone 6N",                                           0 },	// WGS 72BE / UTM zone 6N
	{    32407L, 32407L, "WGS72be.UTM-7N",             "WGS 72BE / UTM zone 7N",                                           0 },	// WGS 72BE / UTM zone 7N
	{    32408L, 32408L, "WGS72be.UTM-8N",             "WGS 72BE / UTM zone 8N",                                           0 },	// WGS 72BE / UTM zone 8N
	{    32409L, 32409L, "WGS72be.UTM-9N",             "WGS 72BE / UTM zone 9N",                                           0 },	// WGS 72BE / UTM zone 9N
	{    32410L, 32410L, "WGS72be.UTM-10N",            "WGS 72BE / UTM zone 10N",                                          0 },	// WGS 72BE / UTM zone 10N
	{    32411L, 32411L, "WGS72be.UTM-11N",            "WGS 72BE / UTM zone 11N",                                          0 },	// WGS 72BE / UTM zone 11N
	{    32412L, 32412L, "WGS72be.UTM-12N",            "WGS 72BE / UTM zone 12N",                                          0 },	// WGS 72BE / UTM zone 12N
	{    32413L, 32413L, "WGS72be.UTM-13N",            "WGS 72BE / UTM zone 13N",                                          0 },	// WGS 72BE / UTM zone 13N
	{    32414L, 32414L, "WGS72be.UTM-14N",            "WGS 72BE / UTM zone 14N",                                          0 },	// WGS 72BE / UTM zone 14N
	{    32415L, 32415L, "WGS72be.UTM-15N",            "WGS 72BE / UTM zone 15N",                                          0 },	// WGS 72BE / UTM zone 15N
	{    32416L, 32416L, "WGS72be.UTM-16N",            "WGS 72BE / UTM zone 16N",                                          0 },	// WGS 72BE / UTM zone 16N
	{    32417L, 32417L, "WGS72be.UTM-17N",            "WGS 72BE / UTM zone 17N",                                          0 },	// WGS 72BE / UTM zone 17N
	{    32418L, 32418L, "WGS72be.UTM-18N",            "WGS 72BE / UTM zone 18N",                                          0 },	// WGS 72BE / UTM zone 18N
	{    32419L, 32419L, "WGS72be.UTM-19N",            "WGS 72BE / UTM zone 19N",                                          0 },	// WGS 72BE / UTM zone 19N
	{    32420L, 32420L, "WGS72be.UTM-20N",            "WGS 72BE / UTM zone 20N",                                          0 },	// WGS 72BE / UTM zone 20N
	{    32421L, 32421L, "WGS72be.UTM-21N",            "WGS 72BE / UTM zone 21N",                                          0 },	// WGS 72BE / UTM zone 21N
	{    32422L, 32422L, "WGS72be.UTM-22N",            "WGS 72BE / UTM zone 22N",                                          0 },	// WGS 72BE / UTM zone 22N
	{    32423L, 32423L, "WGS72be.UTM-23N",            "WGS 72BE / UTM zone 23N",                                          0 },	// WGS 72BE / UTM zone 23N
	{    32424L, 32424L, "WGS72be.UTM-24N",            "WGS 72BE / UTM zone 24N",                                          0 },	// WGS 72BE / UTM zone 24N
	{    32425L, 32425L, "WGS72be.UTM-25N",            "WGS 72BE / UTM zone 25N",                                          0 },	// WGS 72BE / UTM zone 25N
	{    32426L, 32426L, "WGS72be.UTM-26N",            "WGS 72BE / UTM zone 26N",                                          0 },	// WGS 72BE / UTM zone 26N
	{    32427L, 32427L, "WGS72be.UTM-27N",            "WGS 72BE / UTM zone 27N",                                          0 },	// WGS 72BE / UTM zone 27N
	{    32428L, 32428L, "WGS72be.UTM-28N",            "WGS 72BE / UTM zone 28N",                                          0 },	// WGS 72BE / UTM zone 28N
	{    32429L, 32429L, "WGS72be.UTM-29N",            "WGS 72BE / UTM zone 29N",                                          0 },	// WGS 72BE / UTM zone 29N
	{    32430L, 32430L, "WGS72be.UTM-30N",            "WGS 72BE / UTM zone 30N",                                          0 },	// WGS 72BE / UTM zone 30N
	{    32431L, 32431L, "WGS72be.UTM-31N",            "WGS 72BE / UTM zone 31N",                                          0 },	// WGS 72BE / UTM zone 31N
	{    32432L, 32432L, "WGS72be.UTM-32N",            "WGS 72BE / UTM zone 32N",                                          0 },	// WGS 72BE / UTM zone 32N
	{    32433L, 32433L, "WGS72be.UTM-33N",            "WGS 72BE / UTM zone 33N",                                          0 },	// WGS 72BE / UTM zone 33N
	{    32434L, 32434L, "WGS72be.UTM-34N",            "WGS 72BE / UTM zone 34N",                                          0 },	// WGS 72BE / UTM zone 34N
	{    32435L, 32435L, "WGS72be.UTM-35N",            "WGS 72BE / UTM zone 35N",                                          0 },	// WGS 72BE / UTM zone 35N
	{    32436L, 32436L, "WGS72be.UTM-36N",            "WGS 72BE / UTM zone 36N",                                          0 },	// WGS 72BE / UTM zone 36N
	{    32437L, 32437L, "WGS72be.UTM-37N",            "WGS 72BE / UTM zone 37N",                                          0 },	// WGS 72BE / UTM zone 37N
	{    32438L, 32438L, "WGS72be.UTM-38N",            "WGS 72BE / UTM zone 38N",                                          0 },	// WGS 72BE / UTM zone 38N
	{    32439L, 32439L, "WGS72be.UTM-39N",            "WGS 72BE / UTM zone 39N",                                          0 },	// WGS 72BE / UTM zone 39N
	{    32440L, 32440L, "WGS72be.UTM-40N",            "WGS 72BE / UTM zone 40N",                                          0 },	// WGS 72BE / UTM zone 40N
	{    32441L, 32441L, "WGS72be.UTM-41N",            "WGS 72BE / UTM zone 41N",                                          0 },	// WGS 72BE / UTM zone 41N
	{    32442L, 32442L, "WGS72be.UTM-42N",            "WGS 72BE / UTM zone 42N",                                          0 },	// WGS 72BE / UTM zone 42N
	{    32443L, 32443L, "WGS72be.UTM-43N",            "WGS 72BE / UTM zone 43N",                                          0 },	// WGS 72BE / UTM zone 43N
	{    32444L, 32444L, "WGS72be.UTM-44N",            "WGS 72BE / UTM zone 44N",                                          0 },	// WGS 72BE / UTM zone 44N
	{    32445L, 32445L, "WGS72be.UTM-45N",            "WGS 72BE / UTM zone 45N",                                          0 },	// WGS 72BE / UTM zone 45N
	{    32446L, 32446L, "WGS72be.UTM-46N",            "WGS 72BE / UTM zone 46N",                                          0 },	// WGS 72BE / UTM zone 46N
	{    32447L, 32447L, "WGS72be.UTM-47N",            "WGS 72BE / UTM zone 47N",                                          0 },	// WGS 72BE / UTM zone 47N
	{    32448L, 32448L, "WGS72be.UTM-48N",            "WGS 72BE / UTM zone 48N",                                          0 },	// WGS 72BE / UTM zone 48N
	{    32449L, 32449L, "WGS72be.UTM-49N",            "WGS 72BE / UTM zone 49N",                                          0 },	// WGS 72BE / UTM zone 49N
	{    32450L, 32450L, "WGS72be.UTM-50N",            "WGS 72BE / UTM zone 50N",                                          0 },	// WGS 72BE / UTM zone 50N
	{    32451L, 32451L, "WGS72be.UTM-51N",            "WGS 72BE / UTM zone 51N",                                          0 },	// WGS 72BE / UTM zone 51N
	{    32452L, 32452L, "WGS72be.UTM-52N",            "WGS 72BE / UTM zone 52N",                                          0 },	// WGS 72BE / UTM zone 52N
	{    32453L, 32453L, "WGS72be.UTM-53N",            "WGS 72BE / UTM zone 53N",                                          0 },	// WGS 72BE / UTM zone 53N
	{    32454L, 32454L, "WGS72be.UTM-54N",            "WGS 72BE / UTM zone 54N",                                          0 },	// WGS 72BE / UTM zone 54N
	{    32455L, 32455L, "WGS72be.UTM-55N",            "WGS 72BE / UTM zone 55N",                                          0 },	// WGS 72BE / UTM zone 55N
	{    32456L, 32456L, "WGS72be.UTM-56N",            "WGS 72BE / UTM zone 56N",                                          0 },	// WGS 72BE / UTM zone 56N
	{    32457L, 32457L, "WGS72be.UTM-57N",            "WGS 72BE / UTM zone 57N",                                          0 },	// WGS 72BE / UTM zone 57N
	{    32458L, 32458L, "WGS72be.UTM-58N",            "WGS 72BE / UTM zone 58N",                                          0 },	// WGS 72BE / UTM zone 58N
	{    32459L, 32459L, "WGS72be.UTM-59N",            "WGS 72BE / UTM zone 59N",                                          0 },	// WGS 72BE / UTM zone 59N
	{    32460L, 32460L, "WGS72be.UTM-60N",            "WGS 72BE / UTM zone 60N",                                          0 },	// WGS 72BE / UTM zone 60N
	{    32501L, 32501L, "WGS72be.UTM-1S",             "WGS 72BE / UTM zone 1S",                                           0 },	// WGS 72BE / UTM zone 1S
	{    32502L, 32502L, "WGS72be.UTM-2S",             "WGS 72BE / UTM zone 2S",                                           0 },	// WGS 72BE / UTM zone 2S
	{    32503L, 32503L, "WGS72be.UTM-3S",             "WGS 72BE / UTM zone 3S",                                           0 },	// WGS 72BE / UTM zone 3S
	{    32504L, 32504L, "WGS72be.UTM-4S",             "WGS 72BE / UTM zone 4S",                                           0 },	// WGS 72BE / UTM zone 4S
	{    32505L, 32505L, "WGS72be.UTM-5S",             "WGS 72BE / UTM zone 5S",                                           0 },	// WGS 72BE / UTM zone 5S
	{    32506L, 32506L, "WGS72be.UTM-6S",             "WGS 72BE / UTM zone 6S",                                           0 },	// WGS 72BE / UTM zone 6S
	{    32507L, 32507L, "WGS72be.UTM-7S",             "WGS 72BE / UTM zone 7S",                                           0 },	// WGS 72BE / UTM zone 7S
	{    32508L, 32508L, "WGS72be.UTM-8S",             "WGS 72BE / UTM zone 8S",                                           0 },	// WGS 72BE / UTM zone 8S
	{    32509L, 32509L, "WGS72be.UTM-9S",             "WGS 72BE / UTM zone 9S",                                           0 },	// WGS 72BE / UTM zone 9S
	{    32510L, 32510L, "WGS72be.UTM-10S",            "WGS 72BE / UTM zone 10S",                                          0 },	// WGS 72BE / UTM zone 10S
	{    32511L, 32511L, "WGS72be.UTM-11S",            "WGS 72BE / UTM zone 11S",                                          0 },	// WGS 72BE / UTM zone 11S
	{    32512L, 32512L, "WGS72be.UTM-12S",            "WGS 72BE / UTM zone 12S",                                          0 },	// WGS 72BE / UTM zone 12S
	{    32513L, 32513L, "WGS72be.UTM-13S",            "WGS 72BE / UTM zone 13S",                                          0 },	// WGS 72BE / UTM zone 13S
	{    32514L, 32514L, "WGS72be.UTM-14S",            "WGS 72BE / UTM zone 14S",                                          0 },	// WGS 72BE / UTM zone 14S
	{    32515L, 32515L, "WGS72be.UTM-15S",            "WGS 72BE / UTM zone 15S",                                          0 },	// WGS 72BE / UTM zone 15S
	{    32516L, 32516L, "WGS72be.UTM-16S",            "WGS 72BE / UTM zone 16S",                                          0 },	// WGS 72BE / UTM zone 16S
	{    32517L, 32517L, "WGS72be.UTM-17S",            "WGS 72BE / UTM zone 17S",                                          0 },	// WGS 72BE / UTM zone 17S
	{    32518L, 32518L, "WGS72be.UTM-18S",            "WGS 72BE / UTM zone 18S",                                          0 },	// WGS 72BE / UTM zone 18S
	{    32519L, 32519L, "WGS72be.UTM-19S",            "WGS 72BE / UTM zone 19S",                                          0 },	// WGS 72BE / UTM zone 19S
	{    32520L, 32520L, "WGS72be.UTM-20S",            "WGS 72BE / UTM zone 20S",                                          0 },	// WGS 72BE / UTM zone 20S
	{    32521L, 32521L, "WGS72be.UTM-21S",            "WGS 72BE / UTM zone 21S",                                          0 },	// WGS 72BE / UTM zone 21S
	{    32522L, 32522L, "WGS72be.UTM-22S",            "WGS 72BE / UTM zone 22S",                                          0 },	// WGS 72BE / UTM zone 22S
	{    32523L, 32523L, "WGS72be.UTM-23S",            "WGS 72BE / UTM zone 23S",                                          0 },	// WGS 72BE / UTM zone 23S
	{    32524L, 32524L, "WGS72be.UTM-24S",            "WGS 72BE / UTM zone 24S",                                          0 },	// WGS 72BE / UTM zone 24S
	{    32525L, 32525L, "WGS72be.UTM-25S",            "WGS 72BE / UTM zone 25S",                                          0 },	// WGS 72BE / UTM zone 25S
	{    32526L, 32526L, "WGS72be.UTM-26S",            "WGS 72BE / UTM zone 26S",                                          0 },	// WGS 72BE / UTM zone 26S
	{    32527L, 32527L, "WGS72be.UTM-27S",            "WGS 72BE / UTM zone 27S",                                          0 },	// WGS 72BE / UTM zone 27S
	{    32528L, 32528L, "WGS72be.UTM-28S",            "WGS 72BE / UTM zone 28S",                                          0 },	// WGS 72BE / UTM zone 28S
	{    32529L, 32529L, "WGS72be.UTM-29S",            "WGS 72BE / UTM zone 29S",                                          0 },	// WGS 72BE / UTM zone 29S
	{    32530L, 32530L, "WGS72be.UTM-30S",            "WGS 72BE / UTM zone 30S",                                          0 },	// WGS 72BE / UTM zone 30S
	{    32531L, 32531L, "WGS72be.UTM-31S",            "WGS 72BE / UTM zone 31S",                                          0 },	// WGS 72BE / UTM zone 31S
	{    32532L, 32532L, "WGS72be.UTM-32S",            "WGS 72BE / UTM zone 32S",                                          0 },	// WGS 72BE / UTM zone 32S
	{    32533L, 32533L, "WGS72be.UTM-33S",            "WGS 72BE / UTM zone 33S",                                          0 },	// WGS 72BE / UTM zone 33S
	{    32534L, 32534L, "WGS72be.UTM-34S",            "WGS 72BE / UTM zone 34S",                                          0 },	// WGS 72BE / UTM zone 34S
	{    32535L, 32535L, "WGS72be.UTM-35S",            "WGS 72BE / UTM zone 35S",                                          0 },	// WGS 72BE / UTM zone 35S
	{    32536L, 32536L, "WGS72be.UTM-36S",            "WGS 72BE / UTM zone 36S",                                          0 },	// WGS 72BE / UTM zone 36S
	{    32537L, 32537L, "WGS72be.UTM-37S",            "WGS 72BE / UTM zone 37S",                                          0 },	// WGS 72BE / UTM zone 37S
	{    32538L, 32538L, "WGS72be.UTM-38S",            "WGS 72BE / UTM zone 38S",                                          0 },	// WGS 72BE / UTM zone 38S
	{    32539L, 32539L, "WGS72be.UTM-39S",            "WGS 72BE / UTM zone 39S",                                          0 },	// WGS 72BE / UTM zone 39S
	{    32540L, 32540L, "WGS72be.UTM-40S",            "WGS 72BE / UTM zone 40S",                                          0 },	// WGS 72BE / UTM zone 40S
	{    32541L, 32541L, "WGS72be.UTM-41S",            "WGS 72BE / UTM zone 41S",                                          0 },	// WGS 72BE / UTM zone 41S
	{    32542L, 32542L, "WGS72be.UTM-42S",            "WGS 72BE / UTM zone 42S",                                          0 },	// WGS 72BE / UTM zone 42S
	{    32543L, 32543L, "WGS72be.UTM-43S",            "WGS 72BE / UTM zone 43S",                                          0 },	// WGS 72BE / UTM zone 43S
	{    32544L, 32544L, "WGS72be.UTM-44S",            "WGS 72BE / UTM zone 44S",                                          0 },	// WGS 72BE / UTM zone 44S
	{    32545L, 32545L, "WGS72be.UTM-45S",            "WGS 72BE / UTM zone 45S",                                          0 },	// WGS 72BE / UTM zone 45S
	{    32546L, 32546L, "WGS72be.UTM-46S",            "WGS 72BE / UTM zone 46S",                                          0 },	// WGS 72BE / UTM zone 46S
	{    32547L, 32547L, "WGS72be.UTM-47S",            "WGS 72BE / UTM zone 47S",                                          0 },	// WGS 72BE / UTM zone 47S
	{    32548L, 32548L, "WGS72be.UTM-48S",            "WGS 72BE / UTM zone 48S",                                          0 },	// WGS 72BE / UTM zone 48S
	{    32549L, 32549L, "WGS72be.UTM-49S",            "WGS 72BE / UTM zone 49S",                                          0 },	// WGS 72BE / UTM zone 49S
	{    32550L, 32550L, "WGS72be.UTM-50S",            "WGS 72BE / UTM zone 50S",                                          0 },	// WGS 72BE / UTM zone 50S
	{    32551L, 32551L, "WGS72be.UTM-51S",            "WGS 72BE / UTM zone 51S",                                          0 },	// WGS 72BE / UTM zone 51S
	{    32552L, 32552L, "WGS72be.UTM-52S",            "WGS 72BE / UTM zone 52S",                                          0 },	// WGS 72BE / UTM zone 52S
	{    32553L, 32553L, "WGS72be.UTM-53S",            "WGS 72BE / UTM zone 53S",                                          0 },	// WGS 72BE / UTM zone 53S
	{    32554L, 32554L, "WGS72be.UTM-54S",            "WGS 72BE / UTM zone 54S",                                          0 },	// WGS 72BE / UTM zone 54S
	{    32555L, 32555L, "WGS72be.UTM-55S",            "WGS 72BE / UTM zone 55S",                                          0 },	// WGS 72BE / UTM zone 55S
	{    32556L, 32556L, "WGS72be.UTM-56S",            "WGS 72BE / UTM zone 56S",                                          0 },	// WGS 72BE / UTM zone 56S
	{    32557L, 32557L, "WGS72be.UTM-57S",            "WGS 72BE / UTM zone 57S",                                          0 },	// WGS 72BE / UTM zone 57S
	{    32558L, 32558L, "WGS72be.UTM-58S",            "WGS 72BE / UTM zone 58S",                                          0 },	// WGS 72BE / UTM zone 58S
	{    32559L, 32559L, "WGS72be.UTM-59S",            "WGS 72BE / UTM zone 59S",                                          0 },	// WGS 72BE / UTM zone 59S
	{    32560L, 32560L, "WGS72be.UTM-60S",            "WGS 72BE / UTM zone 60S",                                          0 },	// WGS 72BE / UTM zone 60S
	{    32601L, 32601L, "UTM84-1N",                   "WGS 84 / UTM zone 1N",                                             1 },	// WGS 84 / UTM zone 1N
	{    32602L, 32602L, "UTM84-2N",                   "WGS 84 / UTM zone 2N",                                             1 },	// WGS 84 / UTM zone 2N
	{    32603L, 32603L, "UTM84-3N",                   "WGS 84 / UTM zone 3N",                                             1 },	// WGS 84 / UTM zone 3N
	{    32604L, 32604L, "UTM84-4N",                   "WGS 84 / UTM zone 4N",                                             1 },	// WGS 84 / UTM zone 4N
	{    32605L, 32605L, "UTM84-5N",                   "WGS 84 / UTM zone 5N",                                             1 },	// WGS 84 / UTM zone 5N
	{    32606L, 32606L, "UTM84-6N",                   "WGS 84 / UTM zone 6N",                                             1 },	// WGS 84 / UTM zone 6N
	{    32607L, 32607L, "UTM84-7N",                   "WGS 84 / UTM zone 7N",                                             1 },	// WGS 84 / UTM zone 7N
	{    32608L, 32608L, "UTM84-8N",                   "WGS 84 / UTM zone 8N",                                             1 },	// WGS 84 / UTM zone 8N
	{    32609L, 32609L, "UTM84-9N",                   "WGS 84 / UTM zone 9N",                                             1 },	// WGS 84 / UTM zone 9N
	{    32610L, 32610L, "UTM84-10N",                  "WGS 84 / UTM zone 10N",                                            1 },	// WGS 84 / UTM zone 10N
	{    32611L, 32611L, "UTM84-11N",                  "WGS 84 / UTM zone 11N",                                            1 },	// WGS 84 / UTM zone 11N
	{    32612L, 32612L, "UTM84-12N",                  "WGS 84 / UTM zone 12N",                                            1 },	// WGS 84 / UTM zone 12N
	{    32613L, 32613L, "UTM84-13N",                  "WGS 84 / UTM zone 13N",                                            1 },	// WGS 84 / UTM zone 13N
	{    32614L, 32614L, "UTM84-14N",                  "WGS 84 / UTM zone 14N",                                            1 },	// WGS 84 / UTM zone 14N
	{    32615L, 32615L, "UTM84-15N",                  "WGS 84 / UTM zone 15N",                                            1 },	// WGS 84 / UTM zone 15N
	{    32616L, 32616L, "UTM84-16N",                  "WGS 84 / UTM zone 16N",                                            1 },	// WGS 84 / UTM zone 16N
	{    32617L, 32617L, "UTM84-17N",                  "WGS 84 / UTM zone 17N",                                            1 },	// WGS 84 / UTM zone 17N
	{    32618L, 32618L, "UTM84-18N",                  "WGS 84 / UTM zone 18N",                                            1 },	// WGS 84 / UTM zone 18N
	{    32619L, 32619L, "UTM84-19N",                  "WGS 84 / UTM zone 19N",                                            1 },	// WGS 84 / UTM zone 19N
	{    32620L, 32620L, "UTM84-20N",                  "WGS 84 / UTM zone 20N",                                            1 },	// WGS 84 / UTM zone 20N
	{    32621L, 32621L, "UTM84-21N",                  "WGS 84 / UTM zone 21N",                                            1 },	// WGS 84 / UTM zone 21N
	{    32622L, 32622L, "UTM84-22N",                  "WGS 84 / UTM zone 22N",                                            1 },	// WGS 84 / UTM zone 22N
	{    32623L, 32623L, "UTM84-23N",                  "WGS 84 / UTM zone 23N",                                            1 },	// WGS 84 / UTM zone 23N
	{    32624L, 32624L, "UTM84-24N",                  "WGS 84 / UTM zone 24N",                                            1 },	// WGS 84 / UTM zone 24N
	{    32625L, 32625L, "UTM84-25N",                  "WGS 84 / UTM zone 25N",                                            1 },	// WGS 84 / UTM zone 25N
	{    32626L, 32626L, "UTM84-26N",                  "WGS 84 / UTM zone 26N",                                            1 },	// WGS 84 / UTM zone 26N
	{    32627L, 32627L, "UTM84-27N",                  "WGS 84 / UTM zone 27N",                                            1 },	// WGS 84 / UTM zone 27N
	{    32628L, 32628L, "UTM84-28N",                  "WGS 84 / UTM zone 28N",                                            1 },	// WGS 84 / UTM zone 28N
	{    32629L, 32629L, "UTM84-29N",                  "WGS 84 / UTM zone 29N",                                            1 },	// WGS 84 / UTM zone 29N
	{    32630L, 32630L, "UTM84-30N",                  "WGS 84 / UTM zone 30N",                                            1 },	// WGS 84 / UTM zone 30N
	{    32631L, 32631L, "UTM84-31N",                  "WGS 84 / UTM zone 31N",                                            1 },	// WGS 84 / UTM zone 31N
	{    32632L, 32632L, "UTM84-32N",                  "WGS 84 / UTM zone 32N",                                            1 },	// WGS 84 / UTM zone 32N
	{    32633L, 32633L, "UTM84-33N",                  "WGS 84 / UTM zone 33N",                                            1 },	// WGS 84 / UTM zone 33N
	{    32634L, 32634L, "UTM84-34N",                  "WGS 84 / UTM zone 34N",                                            1 },	// WGS 84 / UTM zone 34N
	{    32635L, 32635L, "UTM84-35N",                  "WGS 84 / UTM zone 35N",                                            1 },	// WGS 84 / UTM zone 35N
	{    32636L, 32636L, "UTM84-36N",                  "WGS 84 / UTM zone 36N",                                            1 },	// WGS 84 / UTM zone 36N
	{    32637L, 32637L, "UTM84-37N",                  "WGS 84 / UTM zone 37N",                                            1 },	// WGS 84 / UTM zone 37N
	{    32638L, 32638L, "UTM84-38N",                  "WGS 84 / UTM zone 38N",                                            1 },	// WGS 84 / UTM zone 38N
	{    32639L, 32639L, "UTM84-39N",                  "WGS 84 / UTM zone 39N",                                            1 },	// WGS 84 / UTM zone 39N
	{    32640L, 32640L, "UTM84-40N",                  "WGS 84 / UTM zone 40N",                                            1 },	// WGS 84 / UTM zone 40N
	{    32641L, 32641L, "UTM84-41N",                  "WGS 84 / UTM zone 41N",                                            1 },	// WGS 84 / UTM zone 41N
	{    32642L, 32642L, "UTM84-42N",                  "WGS 84 / UTM zone 42N",                                            1 },	// WGS 84 / UTM zone 42N
	{    32643L, 32643L, "UTM84-43N",                  "WGS 84 / UTM zone 43N",                                            1 },	// WGS 84 / UTM zone 43N
	{    32644L, 32644L, "UTM84-44N",                  "WGS 84 / UTM zone 44N",                                            1 },	// WGS 84 / UTM zone 44N
	{    32645L, 32645L, "UTM84-45N",                  "WGS 84 / UTM zone 45N",                                            1 },	// WGS 84 / UTM zone 45N
	{    32646L, 32646L, "UTM84-46N",                  "WGS 84 / UTM zone 46N",                                            1 },	// WGS 84 / UTM zone 46N
	{    32647L, 32647L, "UTM84-47N",                  "WGS 84 / UTM zone 47N",                                            1 },	// WGS 84 / UTM zone 47N
	{    32648L, 32648L, "UTM84-48N",                  "WGS 84 / UTM zone 48N",                                            1 },	// WGS 84 / UTM zone 48N
	{    32649L, 32649L, "UTM84-49N",                  "WGS 84 / UTM zone 49N",                                            1 },	// WGS 84 / UTM zone 49N
	{    32650L, 32650L, "UTM84-50N",                  "WGS 84 / UTM zone 50N",                                            1 },	// WGS 84 / UTM zone 50N
	{    32651L, 32651L, "UTM84-51N",                  "WGS 84 / UTM zone 51N",                                            1 },	// WGS 84 / UTM zone 51N
	{    32652L, 32652L, "UTM84-52N",                  "WGS 84 / UTM zone 52N",                                            1 },	// WGS 84 / UTM zone 52N
	{    32653L, 32653L, "UTM84-53N",                  "WGS 84 / UTM zone 53N",                                            1 },	// WGS 84 / UTM zone 53N
	{    32654L, 32654L, "UTM84-54N",                  "WGS 84 / UTM zone 54N",                                            1 },	// WGS 84 / UTM zone 54N
	{    32655L, 32655L, "UTM84-55N",                  "WGS 84 / UTM zone 55N",                                            1 },	// WGS 84 / UTM zone 55N
	{    32656L, 32656L, "UTM84-56N",                  "WGS 84 / UTM zone 56N",                                            1 },	// WGS 84 / UTM zone 56N
	{    32657L, 32657L, "UTM84-57N",                  "WGS 84 / UTM zone 57N",                                            1 },	// WGS 84 / UTM zone 57N
	{    32658L, 32658L, "UTM84-58N",                  "WGS 84 / UTM zone 58N",                                            1 },	// WGS 84 / UTM zone 58N
	{    32659L, 32659L, "UTM84-59N",                  "WGS 84 / UTM zone 59N",                                            1 },	// WGS 84 / UTM zone 59N
	{    32660L, 32660L, "UTM84-60N",                  "WGS 84 / UTM zone 60N",                                            1 },	// WGS 84 / UTM zone 60N
	{    32661L, 32661L, "WGS84.UPSNorth",             "WGS 84 / UPS North",                                               0 },	// WGS 84 / UPS North
	{    32662L, 32662L, "WGS84.PlateCarree",          "WGS 84 / Plate Carree",                                            0 },	// WGS 84 / Plate Carree
	{    32701L, 32701L, "UTM84-1S",                   "WGS 84 / UTM zone 1S",                                             1 },	// WGS 84 / UTM zone 1S
	{    32702L, 32702L, "UTM84-2S",                   "WGS 84 / UTM zone 2S",                                             1 },	// WGS 84 / UTM zone 2S
	{    32703L, 32703L, "UTM84-3S",                   "WGS 84 / UTM zone 3S",                                             1 },	// WGS 84 / UTM zone 3S
	{    32704L, 32704L, "UTM84-4S",                   "WGS 84 / UTM zone 4S",                                             1 },	// WGS 84 / UTM zone 4S
	{    32705L, 32705L, "UTM84-5S",                   "WGS 84 / UTM zone 5S",                                             1 },	// WGS 84 / UTM zone 5S
	{    32706L, 32706L, "UTM84-6S",                   "WGS 84 / UTM zone 6S",                                             1 },	// WGS 84 / UTM zone 6S
	{    32707L, 32707L, "UTM84-7S",                   "WGS 84 / UTM zone 7S",                                             1 },	// WGS 84 / UTM zone 7S
	{    32708L, 32708L, "UTM84-8S",                   "WGS 84 / UTM zone 8S",                                             1 },	// WGS 84 / UTM zone 8S
	{    32709L, 32709L, "UTM84-9S",                   "WGS 84 / UTM zone 9S",                                             1 },	// WGS 84 / UTM zone 9S
	{    32710L, 32710L, "UTM84-10S",                  "WGS 84 / UTM zone 10S",                                            1 },	// WGS 84 / UTM zone 10S
	{    32711L, 32711L, "UTM84-11S",                  "WGS 84 / UTM zone 11S",                                            1 },	// WGS 84 / UTM zone 11S
	{    32712L, 32712L, "UTM84-12S",                  "WGS 84 / UTM zone 12S",                                            1 },	// WGS 84 / UTM zone 12S
	{    32713L, 32713L, "UTM84-13S",                  "WGS 84 / UTM zone 13S",                                            1 },	// WGS 84 / UTM zone 13S
	{    32714L, 32714L, "UTM84-14S",                  "WGS 84 / UTM zone 14S",                                            1 },	// WGS 84 / UTM zone 14S
	{    32715L, 32715L, "UTM84-15S",                  "WGS 84 / UTM zone 15S",                                            1 },	// WGS 84 / UTM zone 15S
	{    32716L, 32716L, "UTM84-16S",                  "WGS 84 / UTM zone 16S",                                            1 },	// WGS 84 / UTM zone 16S
	{    32717L, 32717L, "UTM84-17S",                  "WGS 84 / UTM zone 17S",                                            1 },	// WGS 84 / UTM zone 17S
	{    32718L, 32718L, "UTM84-18S",                  "WGS 84 / UTM zone 18S",                                            1 },	// WGS 84 / UTM zone 18S
	{    32719L, 32719L, "UTM84-19S",                  "WGS 84 / UTM zone 19S",                                            1 },	// WGS 84 / UTM zone 19S
	{    32720L, 32720L, "UTM84-20S",                  "WGS 84 / UTM zone 20S",                                            1 },	// WGS 84 / UTM zone 20S
	{    32721L, 32721L, "UTM84-21S",                  "WGS 84 / UTM zone 21S",                                            1 },	// WGS 84 / UTM zone 21S
	{    32722L, 32722L, "UTM84-22S",                  "WGS 84 / UTM zone 22S",                                            1 },	// WGS 84 / UTM zone 22S
	{    32723L, 32723L, "UTM84-23S",                  "WGS 84 / UTM zone 23S",                                            1 },	// WGS 84 / UTM zone 23S
	{    32724L, 32724L, "UTM84-24S",                  "WGS 84 / UTM zone 24S",                                            1 },	// WGS 84 / UTM zone 24S
	{    32725L, 32725L, "UTM84-25S",                  "WGS 84 / UTM zone 25S",                                            1 },	// WGS 84 / UTM zone 25S
	{    32726L, 32726L, "UTM84-26S",                  "WGS 84 / UTM zone 26S",                                            1 },	// WGS 84 / UTM zone 26S
	{    32727L, 32727L, "UTM84-27S",                  "WGS 84 / UTM zone 27S",                                            1 },	// WGS 84 / UTM zone 27S
	{    32728L, 32728L, "UTM84-28S",                  "WGS 84 / UTM zone 28S",                                            1 },	// WGS 84 / UTM zone 28S
	{    32729L, 32729L, "UTM84-29S",                  "WGS 84 / UTM zone 29S",                                            1 },	// WGS 84 / UTM zone 29S
	{    32730L, 32730L, "UTM84-30S",                  "WGS 84 / UTM zone 30S",                                            1 },	// WGS 84 / UTM zone 30S
	{    32731L, 32731L, "UTM84-31S",                  "WGS 84 / UTM zone 31S",                                            1 },	// WGS 84 / UTM zone 31S
	{    32732L, 32732L, "UTM84-32S",                  "WGS 84 / UTM zone 32S",                                            1 },	// WGS 84 / UTM zone 32S
	{    32733L, 32733L, "UTM84-33S",                  "WGS 84 / UTM zone 33S",                                            1 },	// WGS 84 / UTM zone 33S
	{    32734L, 32734L, "UTM84-34S",                  "WGS 84 / UTM zone 34S",                                            1 },	// WGS 84 / UTM zone 34S
	{    32735L, 32735L, "UTM84-35S",                  "WGS 84 / UTM zone 35S",                                            1 },	// WGS 84 / UTM zone 35S
	{    32736L, 32736L, "UTM84-36S",                  "WGS 84 / UTM zone 36S",                                            1 },	// WGS 84 / UTM zone 36S
	{    32737L, 32737L, "UTM84-37S",                  "WGS 84 / UTM zone 37S",                                            1 },	// WGS 84 / UTM zone 37S
	{    32738L, 32738L, "UTM84-38S",                  "WGS 84 / UTM zone 38S",                                            1 },	// WGS 84 / UTM zone 38S
	{    32739L, 32739L, "UTM84-39S",                  "WGS 84 / UTM zone 39S",                                            1 },	// WGS 84 / UTM zone 39S
	{    32740L, 32740L, "UTM84-40S",                  "WGS 84 / UTM zone 40S",                                            1 },	// WGS 84 / UTM zone 40S
	{    32741L, 32741L, "UTM84-41S",                  "WGS 84 / UTM zone 41S",                                            1 },	// WGS 84 / UTM zone 41S
	{    32742L, 32742L, "UTM84-42S",                  "WGS 84 / UTM zone 42S",                                            1 },	// WGS 84 / UTM zone 42S
	{    32743L, 32743L, "UTM84-43S",                  "WGS 84 / UTM zone 43S",                                            1 },	// WGS 84 / UTM zone 43S
	{    32744L, 32744L, "UTM84-44S",                  "WGS 84 / UTM zone 44S",                                            1 },	// WGS 84 / UTM zone 44S
	{    32745L, 32745L, "UTM84-45S",                  "WGS 84 / UTM zone 45S",                                            1 },	// WGS 84 / UTM zone 45S
	{    32746L, 32746L, "UTM84-46S",                  "WGS 84 / UTM zone 46S",                                            1 },	// WGS 84 / UTM zone 46S
	{    32747L, 32747L, "UTM84-47S",                  "WGS 84 / UTM zone 47S",                                            1 },	// WGS 84 / UTM zone 47S
	{    32748L, 32748L, "UTM84-48S",                  "WGS 84 / UTM zone 48S",                                            1 },	// WGS 84 / UTM zone 48S
	{    32749L, 32749L, "UTM84-49S",                  "WGS 84 / UTM zone 49S",                                            1 },	// WGS 84 / UTM zone 49S
	{    32750L, 32750L, "UTM84-50S",                  "WGS 84 / UTM zone 50S",                                            1 },	// WGS 84 / UTM zone 50S
	{    32751L, 32751L, "UTM84-51S",                  "WGS 84 / UTM zone 51S",                                            1 },	// WGS 84 / UTM zone 51S
	{    32752L, 32752L, "UTM84-52S",                  "WGS 84 / UTM zone 52S",                                            1 },	// WGS 84 / UTM zone 52S
	{    32753L, 32753L, "UTM84-53S",                  "WGS 84 / UTM zone 53S",                                            1 },	// WGS 84 / UTM zone 53S
	{    32754L, 32754L, "UTM84-54S",                  "WGS 84 / UTM zone 54S",                                            1 },	// WGS 84 / UTM zone 54S
	{    32755L, 32755L, "UTM84-55S",                  "WGS 84 / UTM zone 55S",                                            1 },	// WGS 84 / UTM zone 55S
	{    32756L, 32756L, "UTM84-56S",                  "WGS 84 / UTM zone 56S",                                            1 },	// WGS 84 / UTM zone 56S
	{    32757L, 32757L, "UTM84-57S",                  "WGS 84 / UTM zone 57S",                                            1 },	// WGS 84 / UTM zone 57S
	{    32758L, 32758L, "UTM84-58S",                  "WGS 84 / UTM zone 58S",                                            1 },	// WGS 84 / UTM zone 58S
	{    32759L, 32759L, "UTM84-59S",                  "WGS 84 / UTM zone 59S",                                            1 },	// WGS 84 / UTM zone 59S
	{    32760L, 32760L, "UTM84-60S",                  "WGS 84 / UTM zone 60S",                                            1 },	// WGS 84 / UTM zone 60S
	{    32761L, 32761L, "WGS84.UPSSouth",             "WGS 84 / UPS South",                                               0 },	// WGS 84 / UPS South
	{    32766L, 32766L, "WGS84.TM-36SE",              "WGS 84 / TM 36 SE",                                                0 },	// WGS 84 / TM 36 SE
	{        0L,     0L, "",                           "",                                                                 0 },	// End of table marker
};

bool AddOracle10Mappings (const wchar_t* csDataDir,TcsCsvStatus& status)
{
	bool ok (true);
	short count (0);
	short dummy;
	unsigned recNbr;
	wchar_t* wcPtr;
	wchar_t wcTemp [512];
	wchar_t filePath [260];
	struct csOracleNbrMap_* tblPtr;
	TcsNameMapper nameMapper;
	std::wstring fieldValue;

	wcsncpy (filePath,csDataDir,wcCount (filePath));
	wcPtr = filePath + wcslen (filePath) - 1;
	if (*wcPtr != L'\\' && *wcPtr != L'/')
	{
		++wcPtr;
		*wcPtr++ = L'\\';
	}

	wcscpy (wcPtr,L"ProjectiveKeyNameMap.csv");
	TcsKeyNameMapFile prjMap (filePath,27);
	wcscpy (wcPtr,L"GeographicKeyNameMap.csv");
	TcsKeyNameMapFile geoMap (filePath,27);

	// For each Oracle10 entry in the table, we use the MSI name to locate the
	// appropriate record in the table and insert the Oracle9 name and number.
	tblPtr = csOracleNbrMapCS;
	while (ok && tblPtr->msiName [0] != '\0' && tblPtr->oracleName [0] != '\0')
	{
		mbstowcs (wcTemp,tblPtr->msiName,wcCount (wcTemp));
		if (tblPtr->epsgNbr == -1 || (tblPtr->epsgNbr > 4000 && tblPtr->epsgNbr < 4800))
		{
			// Do the geographic names here.
			// First we need to locate the appropriate record.
			bool locOk = geoMap.Locate (recNbr,L"CsMapName",wcTemp);
			if (locOk)
			{
				mbstowcs (wcTemp,tblPtr->oracleName,wcCount (wcTemp));
				ok = geoMap.SetCurrentRecord (recNbr);
				if (ok)
				{
					// Get the Oracle name.  If it matches the table name, we continue.
					ok = geoMap.GetField (fieldValue,csMapFldOracleName);
					if (ok && fieldValue.empty ())
					{
						std::wstring newValue (wcTemp);
						ok = geoMap.ReplaceField (csMapFldOracleName,newValue);
						if (ok && tblPtr->oracleNbr > 0)
						{
							swprintf (wcTemp,wcCount (wcTemp),L"%ld",tblPtr->oracleNbr);
							std::wstring newValue1 (wcTemp);
							ok = geoMap.ReplaceField (csMapFldOracleNbr,newValue1);
						}
					}
					else
					{
						if (wcsicmp (fieldValue.c_str (),wcTemp))
						{
							dummy = 0;
						}
					}
				}
			}
		}
		else
		{
			// Its a projective key name.
			bool locOk = prjMap.Locate (recNbr,L"CsMapName",wcTemp);
			if (locOk)
			{
				mbstowcs (wcTemp,tblPtr->oracleName,wcCount (wcTemp));
				ok = prjMap.SetCurrentRecord (recNbr);
				if (ok)
				{
					// Get the Oracle name.  If it matches the table name, we continue.
					ok = prjMap.GetField (fieldValue,csMapFldOracleName);
					if (ok && fieldValue.empty ())
					{
						mbstowcs (wcTemp,tblPtr->oracleName,wcCount (wcTemp));
						std::wstring newValue (wcTemp);
						count++;
						ok = prjMap.ReplaceField (csMapFldOracleName,newValue);
						if (ok && tblPtr->oracleNbr > 0)
						{
							swprintf (wcTemp,wcCount (wcTemp),L"%ld",tblPtr->oracleNbr);
							std::wstring newValue1 (wcTemp);
							ok = prjMap.ReplaceField (csMapFldOracleNbr,newValue1);
						}
					}
					else
					{			
						if (wcsicmp (fieldValue.c_str (),wcTemp))
						{
							dummy = 0;
						}
					}
				}
			}
		}
		tblPtr++;
	}

	// Write out the modified tables.
	wcscpy (wcPtr,L"GeographicKeyNameMap10.csv");	
	std::wofstream oStrm (filePath,std::ios_base::out | std::ios_base::trunc);
	if (oStrm.is_open ())
	{
		geoMap.WriteToStream (oStrm,true,status);
		oStrm.close ();
	}

	wcscpy (wcPtr,L"ProjectiveKeyNameMap10.csv");	
	oStrm.open (filePath,std::ios_base::out | std::ios_base::trunc);
	if (oStrm.is_open ())
	{
		prjMap.WriteToStream (oStrm,true,status);
		oStrm.close ();
	}
	return ok;
}
