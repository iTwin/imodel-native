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

struct csOracleNbrMap_ csOracleNbrMap [] =
{
	{     8288L,  4124L, "LL-RT90",                    "Longitude / Latitude (RT 90)",                                     5 },	// RT90
	{     8277L,  4135L, "LL-OLDHI",                   "Longitude / Latitude (Old Hawaiian)",                              5 },	// Old Hawaiian
	{     8284L,  4139L, "PRVI.LL",                    "Longitude / Latitude (Puerto Rico)",                               4 },	// Puerto Rico
	{     8223L,  4154L, "Europ50/77.LL",              "Longitude / Latitude (ED 50)",                                     4 },	// ED50(ED77)
	{     8272L,  4158L, "NAPARIMA.LL",                "Longitude / Latitude (Naparima, BWI)",                             4 },	// Naparima 1955
	{     8275L,  4182L, "OBSRV66.LL",                 "Longitude / Latitude (Observatorio 1966)",                         5 },	// Azores Occidental 1939
	{     8299L,  4183L, "AZORES.LL",                  "Longitude / Latitude (Southwest Base)",                            4 },	// Azores Central 1948
	{     8294L,  4184L, "SAOBRAZ.LL",                 "Longitude / Latitude (Sao Braz)",                                  4 },	// Azores Oriental 1940
	{     8195L,  4201L, "Adindan.LL",                 "Longitude / Latitude (Adindan)",                                   4 },	// Adindan
	{     8193L,  4202L, "LL-AGD66",                   "Longitude / Latitude (AGD 66)",                                    5 },	// AGD66
	{     8194L,  4203L, "LL-AGD84",                   "Longitude / Latitude (AGD 84)",                                    5 },	// AGD84
	{     8197L,  4204L, "AinElAbd.LL",                "Longitude / Latitude (Ain el Abd 1970)",                           4 },	// Ain el Abd
	{     8196L,  4205L, "Afgooye.LL",                 "Longitude / Latitude (Afgooye)",                                   4 },	// Afgooye
	{     8199L,  4209L, "Arc1950.LL",                 "Longitude / Latitude (Arc 1950)",                                  4 },	// Arc 1950
	{     8200L,  4210L, "Arc1960.LL",                 "Longitude / Latitude (Arc 1960)",                                  4 },	// Arc 1960
	{     8206L,  4215L, "Belge50.LL",                 "Longitude / Latitude (Belgium)",                                   4 },	// Belge 1950
	{     8208L,  4216L, "Bermuda.LL",                 "Longitude / Latitude (Bermuda 1957)",                              4 },	// Bermuda 1957
	{     8209L,  4218L, "Bogota.LL",                  "Longitude / Latitude (Bogota Observatory)",                        4 },	// Bogota 1975
	{     8211L,  4221L, "Campo.LL",                   "Longitude / Latitude (Campo Inchauspe)",                           4 },	// Campo Inchauspe
	{     8214L,  4222L, "Cape-1.LL",                  "Longitude / Latitude (Cape)",                                      4 },	// Cape
	{     8215L,  4223L, "Carthage.LL",                "Longitude / Latitude (Carthage)",                                  4 },	// Carthage
	{     8217L,  4224L, "Chua.LL",                    "Longitude / Latitude (Chua Astro)",                                4 },	// Chua
	{     8218L,  4225L, "Corrego.LL",                 "Longitude / Latitude (Corrego Alegre)",                            4 },	// Corrego Alegre
	{     8276L,  4229L, "Old-Egyp.LL",                "Longitude / Latitude (Old Egyptian)",                              4 },	// Egypt 1907
	{     8223L,  4230L, "LL-ERP50",                   "Longitude / Latitude (ED 50)",                                     5 },	// ED50
	{     8225L,  4231L, "Europ87.LL",                 "Longitude / Latitude (ED 87)",                                     4 },	// ED87
	{     8278L,  4232L, "Fahud.LL",                   "Longitude / Latitude (Oman)",                                      4 },	// Fahud
	{     8228L,  4233L, "Gandajika.LL",               "Longitude / Latitude (Gandajika Base)",                            6 },	// Gandajika 1970
	{     8233L,  4236L, "HuTzuShan.LL",               "Longitude / Latitude (Hu-Tzu-Shan)",                               4 },	// Hu Tzu Shan
	{     8239L,  4244L, "Kandawala.LL",               "Longitude / Latitude (Kandawala)",                                 4 },	// Kandawala
	{     8241L,  4245L, "Kertau48.LL",                "Longitude / Latitude (Kertau 1948)",                               4 },	// Kertau 1968
	{     8282L,  4248L, "LLPSAD56",                   "Longitude / Latitude (Provisional South American 1956)",           5 },	// PSAD56
	{     8243L,  4251L, "Liberia.LL",                 "Longitude / Latitude (Liberia 1964)",                              4 },	// Liberia 1964
	{     8246L,  4253L, "Luzon.LL",                   "Longitude / Latitude (Luzon for Philippines)",                     4 },	// Luzon 1911
	{     8231L,  4254L, "HitoXVIII63.LL",             "Longitude / Latitude (Hito XVIII 1963)",                           4 },	// Hito XVIII 1963
	{     8247L,  4256L, "Mahe1971.LL",                "Longitude / Latitude (Mahe 1971)",                                 4 },	// Mahe 1971
	{     8251L,  4261L, "Merchich",                   "Longitude / Latitude (Merchich)",                                  4 },	// Merchich
	{     8249L,  4262L, "Massawa.LL",                 "Longitude / Latitude (Massawa)",                                   4 },	// Massawa
	{     8253L,  4263L, "Minna.LL",                   "Longitude / Latitude (Minna)",                                     4 },	// Minna
	{     8291L,  4265L, "ROME1940.LL",                "Longitude / Latitude (Rome 1940)",                                 5 },	// Monte Mario
	{     8260L,  4267L, "LL27",                       "Longitude / Latitude (NAD 27 for Continental US)",                 5 },	// NAD27
	{     8265L,  4269L, "LL83",                       "Longitude / Latitude (NAD 83)",                                    5 },	// NAD83
	{     8269L,  4270L, "NHRWN-O.LL",                 "Longitude / Latitude (Nahrwan for Masirah Island)",                4 },	// Nahrwan 1967
	{     8229L,  4272L, "LL-NZGD49",                  "Longitude / Latitude (Geodetic Datum 1949)",                       5 },	// NZGD49
	{     8219L,  4274L, "Datum73.LL",                 "Longitude / Latitude (D73)",                                       4 },	// Datum 73
	{     8266L,  4275L, "NTF.LL",                     "Longitude / Latitude (NTF with Greenwich prime meridian)",         5 },	// NTF
	{     8274L,  4277L, "OSGB.LL",                    "Longitude / Latitude (OSGB 36)",                                   5 },	// OSGB 1936
	{     8311L,  4283L, "LL-GDA94",                   "Longitude / Latitude (GDA 94)",                                    5 },	// GDA94
	{     8285L,  4284L, "Pulkovo42.LL",               "Longitude / Latitude (Pulkovo 1942)",                              4 },	// Pulkovo 1942
	{     8286L,  4286L, "Qatar.LL",                   "Longitude / Latitude (Qatar National)",                            4 },	// Qatar 1948
	{     8287L,  4287L, "Qornoq.LL",                  "Longitude / Latitude (Qornoq)",                                    6 },	// Qornoq
	{     8273L,  4289L, "Amersfoort.LL",              "Longitude / Latitude (Netherlands)",                               5 },	// Amersfoort
	{     8292L,  4291L, "SAD69.LL",                   "Longitude / Latitude (SAD 69)",                                    6 },	// SAD69
	{     8295L,  4292L, "Sapper.LL",                  "Longitude / Latitude (Sapper Hill 1943)",                          4 },	// Sapper Hill 1943
	{     8296L,  4293L, "Schwarzk.LL",                "Longitude / Latitude (Schwarzeck)",                                4 },	// Schwarzeck
	{     8300L,  4298L, "TIMBALAI.LL",                "Longitude / Latitude (Timbalai 1948)",                             5 },	// Timbalai 1948
	{     8237L,  4299L, "TM65.LL",                    "Longitude / Latitude (Ireland 1965)",                              4 },	// TM65
	{     8301L,  4301L, "Tokyo",                      "Longitude / Latitude (Tokyo)",                                     4 },	// Tokyo
	{     8309L,  4309L, "Yacare.LL",                  "Longitude / Latitude (Yacare)",                                    4 },	// Yacare
	{     8310L,  4311L, "Zanderij.LL",                "Longitude / Latitude (Zanderij)",                                  4 },	// Zanderij
	{     8220L,  4314L, "DHDN.LL",                    "Longitude / Latitude (DHDN)",                                      4 },	// DHDN
	{     8306L,  4322L, "LL72",                       "Longitude / Latitude (WGS 72)",                                    5 },	// WGS 72
	{     8192L,  4326L, "LL84",                       "Longitude / Latitude (WGS 84)",                                    5 },	// WGS 84
	{     8229L,  4347L, "GDA94.LL",                   "Longitude / Latitude (Geodetic Datum 1949)",                       6 },	// GDA94 (3D)
	{     8298L,  4615L, "MADEIRA.LL",                 "Longitude / Latitude (Southeast Base)",                            5 },	// Porto Santo
	{     8292L,  4618L, "SAD69.LL",                   "Longitude / Latitude (SAD 69)",                                    4 },	// SAD69
	{     8290L,  4626L, "REUNION.LL",                 "Longitude / Latitude (Reunion)",                                   4 },	// Reunion 1947
	{     8224L,  4668L, "EUROP79.LL",                 "Longitude / Latitude (ED 79)",                                     5 },	// ED79
	{     8216L,  4672L, "CHATHAM.LL",                 "Longitude / Latitude (Chatham 1971)",                              5 },	// CI1971
	{     8230L,  4675L, "GUAM63.LL",                  "Longitude / Latitude (Guam 1963)",                                 5 },	// Guam 1963
	{     8240L,  4698L, "KERGUELN.LL",                "Longitude / Latitude (Kerguelen Island)",                          5 },	// IGN 1962 Kerguelen
	{     8201L,  4712L, "ASCENSN.LL",                 "Longitude / Latitude (Ascension Island 1958)",                     5 },	// Ascension Island 1958
	{     8207L,  4714L, "BELLEVUE.LL",                "Longitude / Latitude (Bellevue)",                                  5 },	// Bellevue
	{     8213L,  4717L, "CANAVRL.LL",                 "Longitude / Latitude (Cape Canaveral)",                            5 },	// Cape Canaveral
	{     8226L,  4719L, "EASTER.LL",                  "Longitude / Latitude (Easter Island 1967)",                        5 },	// Easter Island 1967
	{     8238L,  4725L, "JHNSTN.LL",                  "Longitude / Latitude (Johnston Island 1961)",                      5 },	// Johnston Island 1961
	{     8252L,  4727L, "MIDWAY.LL",                  "Longitude / Latitude (Midway Astro 1961)",                         5 },	// Midway 1961
	{     8280L,  4729L, "PITCAIRN.LL",                "Longitude / Latitude (Pitcairn Astro 1967)",                       5 },	// Pitcairn 1967
	{     8303L,  4731L, "VITI.LL",                    "Longitude / Latitude (Viti Levu 1916)",                            7 },	// Viti Levu 1916
	{     8302L,  4734L, "TRISTAN.LL",                 "Longitude / Latitude (Tristan Astro 1968)",                        5 },	// Tristan 1968
	{     8232L,  4738L, "HONGKONG.LL",                "Longitude / Latitude (Hong Kong 1963)",                            5 },	// Hong Kong 1963
	{     8289L,  4745L, "Rauenberg83.LL",             "Longitude / Latitude (Rauenberg)",                                 4 },	// RD/83
	{     8281L,  4746L, "Potsdam83.LL",               "Longitude / Latitude (Potsdam)",                                   4 },	// PD/83
	{     8267L,  4807L, "NTF.LL.Paris.Grad",          "Longitude / Latitude (NTF with Paris prime meridian)",             4 },	// NTF (Paris)
	{     8224L,     0L, "EUROP79.LL",                 "Longitude / Latitude (ED 79)",                                     5 },	// 
	{     8297L,     0L, "SINGAPR.LL",                 "Longitude / Latitude (South Asia)",                                5 },	// 
	{     8205L,     0L, "ASTATN52.LL",                "Longitude / Latitude (Astronomic Station 1952)",                   5 },	// 
	{     8207L,     0L, "BELLEVUE.LL",                "Longitude / Latitude (Bellevue)",                                  5 },	// 
	{     8212L,     0L, "CANTON.LL",                  "Longitude / Latitude (Canton Astro 1966)",                         5 },	// 
	{     8216L,     0L, "CHATHAM.LL",                 "Longitude / Latitude (Chatham 1971)",                              5 },	// 
	{     8222L,     0L, "DOS1968.LL",                 "Longitude / Latitude (DOS 1968)",                                  5 },	// 
	{     8227L,     0L, "GUX1.LL",                    "Longitude / Latitude (GUX 1 Astro)",                               5 },	// 
	{     8198L,     0L, "ANNA65.LL",                  "Longitude / Latitude (Anna 1 Astro 1965)",                         5 },	// 
	{     8234L,     0L, "ISTS69.LL",                  "Longitude / Latitude (ISTS 073 Astro 1969)",                       5 },	// 
	{     8240L,     0L, "KERGUELN.LL",                "Longitude / Latitude (Kerguelen Island)",                          5 },	// 
	{     8242L,     0L, "L-C5.LL",                    "Longitude / Latitude (LC 5 Astro)",                                5 },	// 
	{    82183L,  2008L, "NADq77.SCoPQ-2",             "Quebec MTM Zone 2 (NAD 27)",                                       4 },	// NAD27(CGQ77) / SCoPQ zone 2
	{    82185L,  2009L, "NADq77.SCoPQ-3",             "Quebec MTM Zone 3 (NAD 27)",                                       4 },	// NAD27(CGQ77) / SCoPQ zone 3
	{    82187L,  2010L, "NADq77.SCoPQ-4",             "Quebec MTM Zone 4 (NAD 27)",                                       4 },	// NAD27(CGQ77) / SCoPQ zone 4
	{    82189L,  2011L, "NADq77.SCoPQ-5",             "Quebec MTM Zone 5 (NAD 27)",                                       4 },	// NAD27(CGQ77) / SCoPQ zone 5
	{    82191L,  2012L, "NADq77.SCoPQ-6",             "Quebec MTM Zone 6 (NAD 27)",                                       4 },	// NAD27(CGQ77) / SCoPQ zone 6
	{    82193L,  2013L, "NADq77.SCoPQ-7",             "Quebec MTM Zone 7 (NAD 27)",                                       4 },	// NAD27(CGQ77) / SCoPQ zone 7
	{    82195L,  2014L, "NADq77.SCoPQ-8",             "Quebec MTM Zone 8 (NAD 27)",                                       4 },	// NAD27(CGQ77) / SCoPQ zone 8
	{    82197L,  2015L, "NADq77.SCoPQ-9",             "Quebec MTM Zone 9 (NAD 27)",                                       4 },	// NAD27(CGQ77) / SCoPQ zone 9
	{    82181L,  2016L, "NADq77.SCoPQ-10",            "Quebec MTM Zone 10 (NAD 27)",                                      4 },	// NAD27(CGQ77) / SCoPQ zone 10
	{    41140L,  2204L, "TN",                         "Tennessee 4100 (1927)",                                            5 },	// NAD27 / Tennessee
	{    81977L,  2222L, "AZ83-EIF",                   "Arizona 0201, Eastern Zone (1983, feet)",                          5 },	// NAD83 / Arizona East (ft)
	{    81981L,  2223L, "AZ83-CIF",                   "Arizona 0202, Central Zone (1983, feet)",                          5 },	// NAD83 / Arizona Central (ft)
	{    81985L,  2224L, "AZ83-WIF",                   "Arizona 0203, Western Zone (1983, feet)",                          5 },	// NAD83 / Arizona West (ft)
	{    40971L,  2225L, "CA83-IF",                    "California 0401, Zone I (1983, US Survey feet)",                   5 },	// NAD83 / California zone 1 (ftUS)
	{    40974L,  2226L, "CA83-IIF",                   "California 0402, Zone II (1983, US Survey feet)",                  5 },	// NAD83 / California zone 2 (ftUS)
	{    40977L,  2227L, "CA83IIIF",                   "California 0403, Zone III (1983, US Survey feet)",                 5 },	// NAD83 / California zone 3 (ftUS)
	{    40980L,  2228L, "CA83-IVF",                   "California 0404, Zone IV (1983, US Survey feet)",                  5 },	// NAD83 / California zone 4 (ftUS)
	{    40983L,  2229L, "CA83-VF",                    "California 0405, Zone V (1983, US Survey feet)",                   5 },	// NAD83 / California zone 5 (ftUS)
	{    40986L,  2230L, "CA83-VIF",                   "California 0406, Zone VI (1983, US Survey feet)",                  5 },	// NAD83 / California zone 6 (ftUS)
	{    40990L,  2231L, "CO83-NF",                    "Colorado 0501, Northern Zone (1983, US Survey feet)",              5 },	// NAD83 / Colorado North (ftUS)
	{    40993L,  2232L, "CO83-CF",                    "Colorado 0502, Central Zone (1983, US Survey feet)",               5 },	// NAD83 / Colorado Central (ftUS)
	{    40996L,  2233L, "CO83-SF",                    "Colorado 0503, Southern Zone (1983, US Survey feet)",              5 },	// NAD83 / Colorado South (ftUS)
	{    41006L,  2234L, "CT83F",                      "Connecticut 0600 (1983, US Survey feet)",                          5 },	// NAD83 / Connecticut (ftUS)
	{    81995L,  2235L, "DE83F",                      "Delaware 0700 (1983, US Survey feet)",                             5 },	// NAD83 / Delaware (ftUS)
	{    81998L,  2236L, "FL83-EF",                    "Florida 0901, Eastern Zone (1983, US Survey feet)",                5 },	// NAD83 / Florida East (ftUS)
	{    82001L,  2237L, "FL83-WF",                    "Florida 0902, Western Zone (1983, US Survey feet)",                5 },	// NAD83 / Florida West (ftUS)
	{    41010L,  2238L, "FL83-NF",                    "Florida 0903, Northern Zone (1983, US Survey feet)",               5 },	// NAD83 / Florida North (ftUS)
	{    82042L,  2239L, "GA83-EF",                    "Georgia 1001, Eastern Zone (1983, US Survey feet)",                5 },	// NAD83 / Georgia East (ftUS)
	{    82045L,  2240L, "GA83-WF",                    "Georgia 1002, Western Zone (1983, US Survey feet)",                5 },	// NAD83 / Georgia West (ftUS)
	{    82066L,  2241L, "ID83-EF",                    "Idaho 1101, Eastern Zone (1983, US Survey feet)",                  5 },	// NAD83 / Idaho East (ftUS)
	{    82069L,  2242L, "ID83-CF",                    "Idaho 1102, Central Zone (1983, US Survey feet)",                  5 },	// NAD83 / Idaho Central (ftUS)
	{    82072L,  2243L, "ID83-WF",                    "Idaho 1103, Western Zone (1983, US Survey feet)",                  5 },	// NAD83 / Idaho West (ftUS)
	{    82081L,  2244L, "IN83-EF",                    "Indiana 1301, Eastern Zone (1983, US Survey feet)",                7 },	// NAD83 / Indiana East (ftUS)
	{    82084L,  2245L, "IN83-WF",                    "Indiana 1302, Western Zone (1983, US Survey feet)",                7 },	// NAD83 / Indiana West (ftUS)
	{    41033L,  2246L, "KY83-NF",                    "Kentucky 1601, Northern Zone (1983, US Survey feet)",              5 },	// NAD83 / Kentucky North (ftUS)
	{    41036L,  2247L, "KY83-SF",                    "Kentucky 1602, Southern Zone (1983, US Survey feet)",              5 },	// NAD83 / Kentucky South (ftUS)
	{    41048L,  2248L, "MD83F",                      "Maryland 1900 (1983, US Survey feet)",                             5 },	// NAD83 / Maryland (ftUS)
	{    41051L,  2249L, "MA83F",                      "Massachusetts 2001, Mainland Zone (1983, US Survey feet)",         5 },	// NAD83 / Massachusetts Mainland (ftUS)
	{    41054L,  2250L, "MA83-ISF",                   "Massachusetts 2002, Island Zone (1983, US Survey feet)",           5 },	// NAD83 / Massachusetts Island (ftUS)
	{    41058L,  2251L, "MI83-NIF",                   "Michigan 2111, Northern Zone (1983, feet)",                        5 },	// NAD83 / Michigan North (ft)
	{    41062L,  2252L, "MI83-CIF",                   "Michigan 2112, Central Zone (1983, feet)",                         5 },	// NAD83 / Michigan Central (ft)
	{    41066L,  2253L, "MI83-SIF",                   "Michigan 2113, Southern Zone (1983, feet)",                        5 },	// NAD83 / Michigan South (ft)
	{    82119L,  2254L, "MS83-EF",                    "Mississippi 2301, Eastern Zone (1983, US Survey feet)",            5 },	// NAD83 / Mississippi East (ftUS)
	{    82122L,  2255L, "MS83-WF",                    "Mississippi 2302, Western Zone (1983, US Survey feet)",            5 },	// NAD83 / Mississippi West (ftUS)
	{    41078L,  2256L, "MT83IF",                     "Montana 2500 (1983, feet)",                                        5 },	// NAD83 / Montana (ft)
	{    82155L,  2257L, "NM83-EF",                    "New Mexico 3001 , Eastern Zone (1983, US Survey feet)",            5 },	// NAD83 / New Mexico East (ftUS)
	{    82159L,  2258L, "NM83-CF",                    "New Mexico 3002, Central Zone (1983, US Survey feet)",             5 },	// NAD83 / New Mexico Central (ftUS)
	{    82162L,  2259L, "NM83-WF",                    "New Mexico 3003, Western Zone (1983, US Survey feet)",             5 },	// NAD83 / New Mexico West (ftUS)
	{    82165L,  2260L, "NY83-EF",                    "New York 3101, Eastern Zone (1983, US Survey feet)",               5 },	// NAD83 / New York East (ftUS)
	{    82168L,  2261L, "NY83-CF",                    "New York 3102, Central Zone (1983, US Survey feet)",               5 },	// NAD83 / New York Central (ftUS)
	{    82171L,  2262L, "NY83-WF",                    "New York 3103, Western Zone (1983, US Survey feet)",               5 },	// NAD83 / New York West (ftUS)
	{    41088L,  2263L, "NY83-LIF",                   "New York 3104, Long Island Zone (1983, US Survey feet)",           5 },	// NAD83 / New York Long Island (ftUS)
	{    41091L,  2264L, "NC83F",                      "North Carolina 3200 (1983, US Survey feet)",                       5 },	// NAD83 / North Carolina (ftUS)
	{    41106L,  2267L, "OK83-NF",                    "Oklahoma 3501, Northern Zone (1983, US Survey feet)",              5 },	// NAD83 / Oklahoma North (ftUS)
	{    41109L,  2268L, "OK83-SF",                    "Oklahoma 3502, Southern Zone (1983, US Survey feet)",              5 },	// NAD83 / Oklahoma South (ftUS)
	{    41113L,  2269L, "OR83-NIF",                   "Oregon 3601, Northern Zone (1983, feet)",                          5 },	// NAD83 / Oregon North (ft)
	{    41117L,  2270L, "OR83-SIF",                   "Oregon 3602, Southern Zone (1983, feet)",                          5 },	// NAD83 / Oregon South (ft)
	{    41120L,  2271L, "PA83-NF",                    "Pennsylvania 3701, Northern Zone (1983, US Survey feet)",          5 },	// NAD83 / Pennsylvania North (ftUS)
	{    41123L,  2272L, "PA83-SF",                    "Pennsylvania 3702, Southern Zone (1983, US Survey feet)",          5 },	// NAD83 / Pennsylvania South (ftUS)
	{    41130L,  2273L, "SC83IF",                     "South Carolina 3900 (1983, feet)",                                 5 },	// NAD83 / South Carolina (ft)
	{    41141L,  2274L, "TN83F",                      "Tennessee 4100 (1983, US Survey feet)",                            5 },	// NAD83 / Tennessee (ftUS)
	{    41144L,  2275L, "TX83-NF",                    "Texas 4201, Northern Zone (1983, US Survey feet)",                 5 },	// NAD83 / Texas North (ftUS)
	{    41147L,  2276L, "TX83-NCF",                   "Texas 4202, North Central Zone (1983, US Survey feet)",            5 },	// NAD83 / Texas North Central (ftUS)
	{    41150L,  2277L, "TX83-CF",                    "Texas 4203, Central Zone (1983, US Survey feet)",                  5 },	// NAD83 / Texas Central (ftUS)
	{    41153L,  2278L, "TX83-SCF",                   "Texas 4204, South Central Zone (1983, US Survey feet)",            5 },	// NAD83 / Texas South Central (ftUS)
	{    41156L,  2279L, "TX83-SF",                    "Texas 4205, Southern Zone (1983, US Survey feet)",                 5 },	// NAD83 / Texas South (ftUS)
	{    41160L,  2280L, "UT83-NIF",                   "Utah 4301, Northern Zone (1983, feet)",                            5 },	// NAD83 / Utah North (ft)
	{    41164L,  2281L, "UT83-CIF",                   "Utah 4302, Central Zone (1983, feet)",                             5 },	// NAD83 / Utah Central (ft)
	{    41168L,  2282L, "UT83-SIF",                   "Utah 4303, Southern Zone (1983, feet)",                            5 },	// NAD83 / Utah South (ft)
	{    41171L,  2283L, "VA83-NF",                    "Virginia 4501, Northern Zone (1983, US Survey feet)",              5 },	// NAD83 / Virginia North (ftUS)
	{    41174L,  2284L, "VA83-SF",                    "Virginia 4502, Southern Zone (1983, US Survey feet)",              5 },	// NAD83 / Virginia South (ftUS)
	{    41177L,  2285L, "WA83-NF",                    "Washington 4601, Northern Zone (1983, US Survey feet)",            5 },	// NAD83 / Washington North (ftUS)
	{    41180L,  2286L, "WA83-SF",                    "Washington 4602, Southern Zone (1983, US Survey feet)",            5 },	// NAD83 / Washington South (ftUS)
	{    41189L,  2287L, "WI83-NF",                    "Wisconsin 4801, Northern Zone (1983, US Survey feet)",             5 },	// NAD83 / Wisconsin North (ftUS)
	{    41192L,  2288L, "WI83-CF",                    "Wisconsin 4802, Central Zone (1983, US Survey feet)",              5 },	// NAD83 / Wisconsin Central (ftUS)
	{    41195L,  2289L, "WI83-SF",                    "Wisconsin 4803, Southern Zone (1983, US Survey feet)",             5 },	// NAD83 / Wisconsin South (ftUS)
	{   294912L,  2391L, "KKJ.Finland-1",              "Finnish KKJ Zone 1",                                               4 },	// KKJ / Finland zone 1
	{   294913L,  2392L, "KKJ.Finland-2",              "Finnish KKJ Zone 2",                                               4 },	// KKJ / Finland zone 2
	{   294915L,  2394L, "KKJ.Finland-4",              "Finnish KKJ Zone 4",                                               4 },	// KKJ / Finland zone 4
	{    82090L,  2443L, "JGD2K.CS-I",                 "Japanese Zone I",                                                  4 },	// JGD2000 / Japan Plane Rectangular CS I
	{    82091L,  2444L, "JGD2K.CS-II",                "Japanese Zone II",                                                 4 },	// JGD2000 / Japan Plane Rectangular CS II
	{    82092L,  2445L, "JGD2K.CS-III",               "Japanese Zone III",                                                4 },	// JGD2000 / Japan Plane Rectangular CS III
	{    82093L,  2446L, "JGD2K.CS-IV",                "Japanese Zone IV",                                                 4 },	// JGD2000 / Japan Plane Rectangular CS IV
	{    82095L,  2447L, "JGD2K.CS-V",                 "Japanese Zone V",                                                  4 },	// JGD2000 / Japan Plane Rectangular CS V
	{    82096L,  2448L, "JGD2K.CS-VI",                "Japanese Zone VI",                                                 4 },	// JGD2000 / Japan Plane Rectangular CS VI
	{    82097L,  2449L, "JGD2K.CS-VII",               "Japanese Zone VII",                                                4 },	// JGD2000 / Japan Plane Rectangular CS VII
	{    82098L,  2450L, "JGD2K.CS-VIII",              "Japanese Zone VIII",                                               4 },	// JGD2000 / Japan Plane Rectangular CS VIII
	{    82094L,  2451L, "JGD2K.CS-IX",                "Japanese Zone IX",                                                 4 },	// JGD2000 / Japan Plane Rectangular CS IX
	{    82099L,  2452L, "JGD2K.CS-X",                 "Japanese Zone X",                                                  4 },	// JGD2000 / Japan Plane Rectangular CS X
	{    82100L,  2453L, "JGD2K.CS-XI",                "Japanese Zone XI",                                                 4 },	// JGD2000 / Japan Plane Rectangular CS XI
	{    82101L,  2454L, "JGD2K.CS-XII",               "Japanese Zone XII",                                                4 },	// JGD2000 / Japan Plane Rectangular CS XII
	{    82102L,  2455L, "JGD2K.CS-XIII",              "Japanese Zone XIII",                                               4 },	// JGD2000 / Japan Plane Rectangular CS XIII
	{    82103L,  2456L, "JGD2K.CS-XIV",               "Japanese Zone XIV",                                                4 },	// JGD2000 / Japan Plane Rectangular CS XIV
	{    82105L,  2457L, "JGD2K.CS-XV",                "Japanese Zone XV",                                                 4 },	// JGD2000 / Japan Plane Rectangular CS XV
	{    82106L,  2458L, "JGD2K.CS-XVI",               "Japanese Zone XVI",                                                4 },	// JGD2000 / Japan Plane Rectangular CS XVI
	{    82107L,  2459L, "JGD2K.CS-XVII",              "Japanese Zone XVII",                                               4 },	// JGD2000 / Japan Plane Rectangular CS XVII
	{    82108L,  2460L, "JGD2K.CS-XVIII",             "Japanese Zone XVIII",                                              4 },	// JGD2000 / Japan Plane Rectangular CS XVIII
	{    82104L,  2461L, "JGD2K.CS-XIX",               "Japanese Zone XIX",                                                4 },	// JGD2000 / Japan Plane Rectangular CS XIX
	{    82396L,  3092L, "TOKYO.UTM-51N",              "UTM Zone 51 (Tokyo)",                                              4 },	// Tokyo / UTM zone 51N
	{    82399L,  3093L, "TOKYO.UTM-52N",              "UTM Zone 52 (Tokyo)",                                              4 },	// Tokyo / UTM zone 52N
	{    82402L,  3094L, "TOKYO.UTM-53N",              "UTM Zone 53 (Tokyo)",                                              4 },	// Tokyo / UTM zone 53N
	{    82405L,  3095L, "TOKYO.UTM-54N",              "UTM Zone 54 (Tokyo)",                                              4 },	// Tokyo / UTM zone 54N
	{    82408L,  3096L, "TOKYO.UTM-55N",              "UTM Zone 55 (Tokyo)",                                              4 },	// Tokyo / UTM zone 55N
	{    81922L, 20248L, "AMG66-48",                   "AMG Zone 48 (AGD 66)",                                             5 },	// AGD66 / AMG zone 48
	{    81924L, 20249L, "AMG66-49",                   "AMG Zone 49 (AGD 66)",                                             5 },	// AGD66 / AMG zone 49
	{    81926L, 20250L, "AMG66-50",                   "AMG Zone 50 (AGD 66)",                                             5 },	// AGD66 / AMG zone 50
	{    81928L, 20251L, "AMG66-51",                   "AMG Zone 51 (AGD 66)",                                             5 },	// AGD66 / AMG zone 51
	{    81930L, 20252L, "AMG66-52",                   "AMG Zone 52 (AGD 66)",                                             5 },	// AGD66 / AMG zone 52
	{    81932L, 20253L, "AMG66-53",                   "AMG Zone 53 (AGD 66)",                                             5 },	// AGD66 / AMG zone 53
	{    81934L, 20254L, "AMG66-54",                   "AMG Zone 54 (AGD 66)",                                             5 },	// AGD66 / AMG zone 54
	{    81936L, 20255L, "AMG66-55",                   "AMG Zone 55 (AGD 66)",                                             5 },	// AGD66 / AMG zone 55
	{    81938L, 20256L, "AMG66-56",                   "AMG Zone 56 (AGD 66)",                                             5 },	// AGD66 / AMG zone 56
	{    81940L, 20257L, "AMG66-57",                   "AMG Zone 57 (AGD 66)",                                             5 },	// AGD66 / AMG zone 57
	{    81942L, 20258L, "AMG66-58",                   "AMG Zone 58 (AGD 66)",                                             5 },	// AGD66 / AMG zone 58
	{    81923L, 20348L, "AMG84-48",                   "AMG Zone 48 (AGD 84)",                                             5 },	// AGD84 / AMG zone 48
	{    81925L, 20349L, "AMG84-49",                   "AMG Zone 49 (AGD 84)",                                             5 },	// AGD84 / AMG zone 49
	{    81927L, 20350L, "AMG84-50",                   "AMG Zone 50 (AGD 84)",                                             5 },	// AGD84 / AMG zone 50
	{    81929L, 20351L, "AMG84-51",                   "AMG Zone 51 (AGD 84)",                                             5 },	// AGD84 / AMG zone 51
	{    81931L, 20352L, "AMG84-52",                   "AMG Zone 52 (AGD 84)",                                             5 },	// AGD84 / AMG zone 52
	{    81933L, 20353L, "AMG84-53",                   "AMG Zone 53 (AGD 84)",                                             5 },	// AGD84 / AMG zone 53
	{    81935L, 20354L, "AMG84-54",                   "AMG Zone 54 (AGD 84)",                                             5 },	// AGD84 / AMG zone 54
	{    81937L, 20355L, "AMG84-55",                   "AMG Zone 55 (AGD 84)",                                             5 },	// AGD84 / AMG zone 55
	{    81939L, 20356L, "AMG84-56",                   "AMG Zone 56 (AGD 84)",                                             5 },	// AGD84 / AMG zone 56
	{    81941L, 20357L, "AMG84-57",                   "AMG Zone 57 (AGD 84)",                                             5 },	// AGD84 / AMG zone 57
	{    81943L, 20358L, "AMG84-58",                   "AMG Zone 58 (AGD 84)",                                             5 },	// AGD84 / AMG zone 58
	{    82286L, 22521L, "Corrego.UTM-21S",            "UTM Zone 21, Southern Hemisphere (Corrego Alegre)",                4 },	// Corrego Alegre / UTM zone 21S
	{    82294L, 22522L, "Corrego.UTM-22S",            "UTM Zone 22, Southern Hemisphere (Corrego Alegre)",                4 },	// Corrego Alegre / UTM zone 22S
	{    82300L, 22523L, "Corrego.UTM-23S",            "UTM Zone 23, Southern Hemisphere (Corrego Alegre)",                4 },	// Corrego Alegre / UTM zone 23S
	{    82306L, 22524L, "Corrego.UTM-24S",            "UTM Zone 24, Southern Hemisphere (Corrego Alegre)",                4 },	// Corrego Alegre / UTM zone 24S
	{    82312L, 22525L, "Corrego.UTM-25S",            "UTM Zone 25, Southern Hemisphere (Corrego Alegre)",                4 },	// Corrego Alegre / UTM zone 25S
	{    82323L, 23028L, "ED50-UTM28",                 "UTM Zone 28 (ED 50)",                                              5 },	// ED50 / UTM zone 28N
	{    82328L, 23029L, "ED50-UTM29",                 "UTM Zone 29 (ED 50)",                                              5 },	// ED50 / UTM zone 29N
	{    82337L, 23030L, "ED50-UTM30",                 "UTM Zone 30 (ED 50)",                                              5 },	// ED50 / UTM zone 30N
	{    82340L, 23031L, "ED50-UTM31",                 "UTM Zone 31 (ED 50)",                                              5 },	// ED50 / UTM zone 31N
	{    82343L, 23032L, "ED50-UTM32",                 "UTM Zone 32 (ED 50)",                                              5 },	// ED50 / UTM zone 32N
	{    82346L, 23033L, "ED50-UTM33",                 "UTM Zone 33 (ED 50)",                                              5 },	// ED50 / UTM zone 33N
	{    82349L, 23034L, "ED50-UTM34",                 "UTM Zone 34 (ED 50)",                                              5 },	// ED50 / UTM zone 34N
	{    82352L, 23035L, "ED50-UTM35",                 "UTM Zone 35 (ED 50)",                                              5 },	// ED50 / UTM zone 35N
	{    82355L, 23036L, "ED50-UTM36",                 "UTM Zone 36 (ED 50)",                                              5 },	// ED50 / UTM zone 36N
	{    82358L, 23037L, "ED50-UTM37",                 "UTM Zone 37 (ED 50)",                                              5 },	// ED50 / UTM zone 37N
	{    82361L, 23038L, "ED50-UTM38",                 "UTM Zone 38 (ED 50)",                                              5 },	// ED50 / UTM zone 38N
	{    82366L, 26704L, "UTM27-4",                    "UTM Zone 4 (NAD 27 for Alaska)",                                   5 },	// NAD27 / UTM zone 4N
	{    82390L, 26705L, "UTM27-5",                    "UTM Zone 5 (NAD 27 for Alaska)",                                   5 },	// NAD27 / UTM zone 5N
	{    82421L, 26706L, "UTM27-6",                    "UTM Zone 6 (NAD 27 for Alaska)",                                   5 },	// NAD27 / UTM zone 6N
	{    82427L, 26707L, "UTM27-7",                    "UTM Zone 7 (NAD 27 for Alaska)",                                   5 },	// NAD27 / UTM zone 7N
	{    82432L, 26708L, "UTM27-8",                    "UTM Zone 8 (NAD 27 for Alaska)",                                   5 },	// NAD27 / UTM zone 8N
	{    82437L, 26709L, "UTM27-9",                    "UTM Zone 9 (NAD 27 for Alaska)",                                   5 },	// NAD27 / UTM zone 9N
	{    82211L, 26710L, "UTM27-10",                   "UTM Zone 10 (NAD 27 for US)",                                      5 },	// NAD27 / UTM zone 10N
	{    82216L, 26711L, "UTM27-11",                   "UTM Zone 11 (NAD 27 for US)",                                      5 },	// NAD27 / UTM zone 11N
	{    82221L, 26712L, "UTM27-12",                   "UTM Zone 12 (NAD 27 for US)",                                      5 },	// NAD27 / UTM zone 12N
	{    82226L, 26713L, "UTM27-13",                   "UTM Zone 13 (NAD 27 for US)",                                      5 },	// NAD27 / UTM zone 13N
	{    82231L, 26714L, "UTM27-14",                   "UTM Zone 14 (NAD 27 for US)",                                      5 },	// NAD27 / UTM zone 14N
	{    82236L, 26715L, "UTM27-15",                   "UTM Zone 15 (NAD 27 for US)",                                      5 },	// NAD27 / UTM zone 15N
	{    82241L, 26716L, "UTM27-16",                   "UTM Zone 16 (NAD 27 for US)",                                      5 },	// NAD27 / UTM zone 16N
	{    82246L, 26717L, "UTM27-17",                   "UTM Zone 17 (NAD 27 for US)",                                      5 },	// NAD27 / UTM zone 17N
	{    82253L, 26718L, "UTM27-18",                   "UTM Zone 18 (NAD 27 for US)",                                      5 },	// NAD27 / UTM zone 18N
	{    82261L, 26719L, "UTM27-19",                   "UTM Zone 19 (NAD 27 for US)",                                      5 },	// NAD27 / UTM zone 19N
	{    82273L, 26720L, "UTM27-20",                   "UTM Zone 20 (NAD 27 for Canada)",                                  5 },	// NAD27 / UTM zone 20N
	{    82281L, 26721L, "UTM27-21",                   "UTM Zone 21 (NAD 27 for Canada)",                                  5 },	// NAD27 / UTM zone 21N
	{    82289L, 26722L, "UTM27-22",                   "UTM Zone 22 (NAD 27 for Canada)",                                  5 },	// NAD27 / UTM zone 22N
	{    81944L, 26729L, "AL-E",                       "Alabama 0101, Eastern Zone (1927)",                                5 },	// NAD27 / Alabama East
	{    81947L, 26730L, "AL-W",                       "Alabama 0102, Western Zone (1927)",                                5 },	// NAD27 / Alabama West
	{   172032L, 26731L, "AK-1",                       "Alaska 5001, Zone 1 (1927)",                                       5 },	// NAD27 / Alaska zone 1
	{    81950L, 26732L, "AK-2",                       "Alaska 5002, Zone 2 (1927)",                                       5 },	// NAD27 / Alaska zone 2
	{    81953L, 26733L, "AK-3",                       "Alaska 5003, Zone 3 (1927)",                                       5 },	// NAD27 / Alaska zone 3
	{    81956L, 26734L, "AK-4",                       "Alaska 5004, Zone 4 (1927)",                                       5 },	// NAD27 / Alaska zone 4
	{    81959L, 26735L, "AK-5",                       "Alaska 5005, Zone 5 (1927)",                                       5 },	// NAD27 / Alaska zone 5
	{    81962L, 26736L, "AK-6",                       "Alaska 5006, Zone 6 (1927)",                                       5 },	// NAD27 / Alaska zone 6
	{    81965L, 26737L, "AK-7",                       "Alaska 5007, Zone 7 (1927)",                                       5 },	// NAD27 / Alaska zone 7
	{    81968L, 26738L, "AK-8",                       "Alaska 5008, Zone 8 (1927)",                                       5 },	// NAD27 / Alaska zone 8
	{    81971L, 26739L, "AK-9",                       "Alaska 5009, Zone 9 (1927)",                                       5 },	// NAD27 / Alaska zone 9
	{    40960L, 26740L, "AK-10",                      "Alaska 5010, Zone 10 (1927)",                                      5 },	// NAD27 / Alaska zone 10
	{    40970L, 26741L, "CA-I",                       "California 0401, Zone I (1927)",                                   5 },	// NAD27 / California zone I
	{    40973L, 26742L, "CA-II",                      "California 0402, Zone II (1927)",                                  5 },	// NAD27 / California zone II
	{    40976L, 26743L, "CA-III",                     "California 0403, Zone III (1927)",                                 5 },	// NAD27 / California zone III
	{    40979L, 26744L, "CA-IV",                      "California 0404, Zone IV (1927)",                                  5 },	// NAD27 / California zone IV
	{    40982L, 26745L, "CA-V",                       "California 0405, Zone V (1927)",                                   5 },	// NAD27 / California zone V
	{    40985L, 26746L, "CA-VI",                      "California 0406, Zone VI (1927)",                                  5 },	// NAD27 / California zone VI
	{    40988L, 26747L, "CA-VII",                     "California 0407, Zone VII (1927)",                                 7 },	// NAD27 / California zone VII
	{    81975L, 26748L, "AZ-E",                       "Arizona 0201, Eastern Zone (1927)",                                5 },	// NAD27 / Arizona East
	{    81979L, 26749L, "AZ-C",                       "Arizona 0202, Central Zone (1927)",                                5 },	// NAD27 / Arizona Central
	{    81983L, 26750L, "AZ-W",                       "Arizona 0203, Western Zone (1927)",                                5 },	// NAD27 / Arizona West
	{    40963L, 26751L, "AR-N",                       "Arkansas 0301, Northern Zone (1927)",                              5 },	// NAD27 / Arkansas North
	{    40966L, 26752L, "AR-S",                       "Arkansas 0302, Southern Zone (1927)",                              5 },	// NAD27 / Arkansas South
	{    40989L, 26753L, "CO-N",                       "Colorado 0501, Northern Zone (1927)",                              5 },	// NAD27 / Colorado North
	{    40992L, 26754L, "CO-C",                       "Colorado 0502, Central Zone (1927)",                               5 },	// NAD27 / Colorado Central
	{    40995L, 26755L, "CO-S",                       "Colorado 0503, Southern Zone (1927)",                              5 },	// NAD27 / Colorado South
	{    41005L, 26756L, "CT",                         "Connecticut 0600 (1927)",                                          5 },	// NAD27 / Connecticut
	{    81994L, 26757L, "DE",                         "Delaware 0700 (1927)",                                             5 },	// NAD27 / Delaware
	{    81997L, 26758L, "FL-E",                       "Florida 0901, Eastern Zone (1927)",                                5 },	// NAD27 / Florida East
	{    82000L, 26759L, "FL-W",                       "Florida 0902, Western Zone (1927)",                                5 },	// NAD27 / Florida West
	{    41009L, 26760L, "FL-N",                       "Florida 0903, Northern Zone (1927)",                               5 },	// NAD27 / Florida North
	{    82041L, 26766L, "GA-E",                       "Georgia 1001, Eastern Zone (1927)",                                5 },	// NAD27 / Georgia East
	{    82044L, 26767L, "GA-W",                       "Georgia 1002, Western Zone (1927)",                                5 },	// NAD27 / Georgia West
	{    82065L, 26768L, "ID-E",                       "Idaho 1101, Eastern Zone (1927)",                                  5 },	// NAD27 / Idaho East
	{    82068L, 26769L, "ID-C",                       "Idaho 1102, Central Zone (1927)",                                  5 },	// NAD27 / Idaho Central
	{    82071L, 26770L, "ID-W",                       "Idaho 1103, Western Zone (1927)",                                  5 },	// NAD27 / Idaho West
	{    82074L, 26771L, "IL-E",                       "Illinois 1201, Eastern Zone (1927)",                               5 },	// NAD27 / Illinois East
	{    82077L, 26772L, "IL-W",                       "Illinois 1202, Western Zone (1927)",                               5 },	// NAD27 / Illinois West
	{    82080L, 26773L, "IN-E",                       "Indiana 1301, Eastern Zone (1927)",                                5 },	// NAD27 / Indiana East
	{    82083L, 26774L, "IN-W",                       "Indiana 1302, Western Zone (1927)",                                5 },	// NAD27 / Indiana West
	{    41020L, 26775L, "IA-N",                       "Iowa 1401, Northern Zone (1927)",                                  5 },	// NAD27 / Iowa North
	{    41023L, 26776L, "IA-S",                       "Iowa 1402, Southern Zone (1927)",                                  5 },	// NAD27 / Iowa South
	{    41026L, 26777L, "KS-N",                       "Kansas 1501, Northern Zone (1927)",                                5 },	// NAD27 / Kansas North
	{    41029L, 26778L, "KS-S",                       "Kansas 1502, Southern Zone (1927)",                                5 },	// NAD27 / Kansas South
	{    41032L, 26779L, "KY-N",                       "Kentucky 1601, Northern Zone (1927)",                              5 },	// NAD27 / Kentucky North
	{    41035L, 26780L, "KY-S",                       "Kentucky 1602, Southern Zone (1927)",                              5 },	// NAD27 / Kentucky South
	{    41038L, 26781L, "LA-N",                       "Louisiana 1701, Northern Zone (1927)",                             5 },	// NAD27 / Louisiana North
	{    41041L, 26782L, "LA-S",                       "Louisiana 1702, Southern Zone (1927)",                             5 },	// NAD27 / Louisiana South
	{    82112L, 26783L, "ME-E",                       "Maine 1801, Eastern Zone (1927)",                                  5 },	// NAD27 / Maine East
	{    82115L, 26784L, "ME-W",                       "Maine 1802, Western Zone (1927)",                                  5 },	// NAD27 / Maine West
	{    41047L, 26785L, "MD",                         "Maryland 1900 (1927)",                                             5 },	// NAD27 / Maryland
	{    41050L, 26786L, "MA",                         "Massachusetts 2001, Mainland Zone (1927)",                         5 },	// NAD27 / Massachusetts Mainland
	{    41053L, 26787L, "MA27-IS",                    "Massachusetts 2002, Island Zone (1927)",                           5 },	// NAD27 / Massachusetts Island
	{    41068L, 26791L, "MN-N",                       "Minnesota 2201, Northern Zone (1927)",                             5 },	// NAD27 / Minnesota North
	{    41071L, 26792L, "MN-C",                       "Minnesota 2202, Central Zone (1927)",                              5 },	// NAD27 / Minnesota Central
	{    41074L, 26793L, "MN-S",                       "Minnesota 2203, South Zone (1927)",                                5 },	// NAD27 / Minnesota South
	{    82118L, 26794L, "MS-E",                       "Mississippi 2301, Eastern Zone (1927)",                            5 },	// NAD27 / Mississippi East
	{    82121L, 26795L, "MS-W",                       "Mississippi 2302, Western Zone (1927)",                            5 },	// NAD27 / Mississippi West
	{    82124L, 26796L, "MO-E",                       "Missouri 2401, Eastern Zone (1927)",                               5 },	// NAD27 / Missouri East
	{    82127L, 26797L, "MO-C",                       "Missouri 2402, Central Zone (1927)",                               5 },	// NAD27 / Missouri Central
	{    82130L, 26798L, "MO-W",                       "Missouri 2403, Western Zone (1927)",                               5 },	// NAD27 / Missouri West
	{    40988L, 26799L, "CA-VII",                     "California 0407, Zone VII (1927)",                                 5 },	// NAD27 / California zone VII
	{    41056L, 26811L, "MI27-N",                     "Michigan 2111, Northern Zone (1927)",                              5 },	// NAD Michigan / Michigan North
	{    41060L, 26812L, "MI27-C",                     "Michigan 2112, Central Zone (1927)",                               5 },	// NAD Michigan / Michigan Central
	{    41064L, 26813L, "MI27-S",                     "Michigan 2113, Southern Zone (1927)",                              5 },	// NAD Michigan / Michigan South
	{    82207L, 26901L, "UTM83-1",                    "UTM Zone 1 (NAD 83)",                                              5 },	// NAD83 / UTM zone 1N
	{    82270L, 26902L, "UTM83-2",                    "UTM Zone 2 (NAD 83)",                                              5 },	// NAD83 / UTM zone 2N
//ADSK-HW 05/09/07 entry missing from Mentor 11.15 fix #2
	{    0L, 26903L,     "UTM83-3",                    "UTM Zone 3 (NAD 83)",                                              5 },	
	{    82367L, 26904L, "UTM83-4",                    "UTM Zone 4 (NAD 83)",                                              5 },	// NAD83 / UTM zone 4N
	{    82391L, 26905L, "UTM83-5",                    "UTM Zone 5 (NAD 83)",                                              5 },	// NAD83 / UTM zone 5N
	{    82422L, 26906L, "UTM83-6",                    "UTM Zone 6 (NAD 83)",                                              5 },	// NAD83 / UTM zone 6N
	{    82429L, 26907L, "UTM83-7",                    "UTM Zone 7 (NAD 83)",                                              5 },	// NAD83 / UTM zone 7N
	{    82434L, 26908L, "UTM83-8",                    "UTM Zone 8 (NAD 83)",                                              5 },	// NAD83 / UTM zone 8N
	{    82439L, 26909L, "UTM83-9",                    "UTM Zone 9 (NAD 83)",                                              5 },	// NAD83 / UTM zone 9N
	{    82212L, 26910L, "UTM83-10",                   "UTM Zone 10 (NAD 83)",                                             5 },	// NAD83 / UTM zone 10N
	{    82217L, 26911L, "UTM83-11",                   "UTM Zone 11 (NAD 83)",                                             5 },	// NAD83 / UTM zone 11N
	{    82222L, 26912L, "UTM83-12",                   "UTM Zone 12 (NAD 83)",                                             5 },	// NAD83 / UTM zone 12N
	{    82227L, 26913L, "UTM83-13",                   "UTM Zone 13 (NAD 83)",                                             5 },	// NAD83 / UTM zone 13N
	{    82232L, 26914L, "UTM83-14",                   "UTM Zone 14 (NAD 83)",                                             5 },	// NAD83 / UTM zone 14N
	{    82237L, 26915L, "UTM83-15",                   "UTM Zone 15 (NAD 83)",                                             5 },	// NAD83 / UTM zone 15N
	{    82242L, 26916L, "UTM83-16",                   "UTM Zone 16 (NAD 83)",                                             5 },	// NAD83 / UTM zone 16N
	{    82247L, 26917L, "UTM83-17",                   "UTM Zone 17 (NAD 83)",                                             5 },	// NAD83 / UTM zone 17N
	{    82254L, 26918L, "UTM83-18",                   "UTM Zone 18 (NAD 83)",                                             5 },	// NAD83 / UTM zone 18N
	{    82262L, 26919L, "UTM83-19",                   "UTM Zone 19 (NAD 83)",                                             5 },	// NAD83 / UTM zone 19N
	{    82274L, 26920L, "UTM83-20",                   "UTM Zone 20 (NAD 83)",                                             5 },	// NAD83 / UTM zone 20N
	{    82282L, 26921L, "UTM83-21",                   "UTM Zone 21 (NAD 83)",                                             5 },	// NAD83 / UTM zone 21N
	{    82290L, 26922L, "UTM83-22",                   "UTM Zone 22 (NAD 83)",                                             5 },	// NAD83 / UTM zone 22N
	{    81946L, 26929L, "AL83-E",                     "Alabama 0101, Eastern Zone (1983, meters)",                        5 },	// NAD83 / Alabama East
	{    81949L, 26930L, "AL83-W",                     "Alabama 0102, Western Zone (1983, meters)",                        5 },	// NAD83 / Alabama West
	{   172034L, 26931L, "AK83-1",                     "Alaska 5001, Zone 1 (1983, meters)",                               5 },	// NAD83 / Alaska zone 1
	{    81952L, 26932L, "AK83-2",                     "Alaska 5002, Zone 2 (1983, meters)",                               5 },	// NAD83 / Alaska zone 2
	{    81955L, 26933L, "AK83-3",                     "Alaska 5003, Zone 3 (1983, meters)",                               5 },	// NAD83 / Alaska zone 3
	{    81958L, 26934L, "AK83-4",                     "Alaska 5004, Zone 4 (1983, meters)",                               5 },	// NAD83 / Alaska zone 4
	{    81961L, 26935L, "AK83-5",                     "Alaska 5005, Zone 5 (1983, meters)",                               5 },	// NAD83 / Alaska zone 5
	{    81964L, 26936L, "AK83-6",                     "Alaska 5006, Zone 6 (1983, meters)",                               5 },	// NAD83 / Alaska zone 6
	{    81967L, 26937L, "AK83-7",                     "Alaska 5007, Zone 7 (1983, meters)",                               5 },	// NAD83 / Alaska zone 7
	{    81970L, 26938L, "AK83-8",                     "Alaska 5008, Zone 8 (1983, meters)",                               5 },	// NAD83 / Alaska zone 8
	{    81973L, 26939L, "AK83-9",                     "Alaska 5009, Zone 9 (1983, meters)",                               5 },	// NAD83 / Alaska zone 9
	{    40962L, 26940L, "AK83-10",                    "Alaska 5010, Zone 10 (1983, meters)",                              5 },	// NAD83 / Alaska zone 10
	{    40972L, 26941L, "CA83-I",                     "California 0401, Zone I (1983, meters)",                           5 },	// NAD83 / California zone 1
	{    40975L, 26942L, "CA83-II",                    "California 0402, Zone II (1983, meters)",                          5 },	// NAD83 / California zone 2
	{    40978L, 26943L, "CA83-III",                   "California 0403, Zone III (1983, meters)",                         5 },	// NAD83 / California zone 3
	{    40981L, 26944L, "CA83-IV",                    "California 0404, Zone IV (1983, meters)",                          5 },	// NAD83 / California zone 4
	{    40984L, 26945L, "CA83-V",                     "California 0405, Zone V (1983, meters)",                           5 },	// NAD83 / California zone 5
	{    40987L, 26946L, "CA83-VI",                    "California 0406, Zone VI (1983, meters)",                          5 },	// NAD83 / California zone 6
	{    81978L, 26948L, "AZ83-E",                     "Arizona 0201, Eastern Zone (1983, meters)",                        5 },	// NAD83 / Arizona East
	{    81982L, 26949L, "AZ83-C",                     "Arizona 0202, Central Zone (1983, meters)",                        5 },	// NAD83 / Arizona Central
	{    81986L, 26950L, "AZ83-W",                     "Arizona 0203, Western Zone (1983, meters)",                        5 },	// NAD83 / Arizona West
	{    40965L, 26951L, "AR83-N",                     "Arkansas 0301, Northern Zone (1983, meters)",                      5 },	// NAD83 / Arkansas North
	{    40968L, 26952L, "AR83-S",                     "Arkansas 0302, Southern Zone (1983, meters)",                      5 },	// NAD83 / Arkansas South
	{    40991L, 26953L, "CO83-N",                     "Colorado 0501, Northern Zone (1983, meters)",                      5 },	// NAD83 / Colorado North
	{    40994L, 26954L, "CO83-C",                     "Colorado 0502, Central Zone (1983, meters)",                       5 },	// NAD83 / Colorado Central
	{    40997L, 26955L, "CO83-S",                     "Colorado 0503, Southern Zone (1983, meters)",                      5 },	// NAD83 / Colorado South
	{    41007L, 26956L, "CT83",                       "Connecticut 0600 (1983, meters)",                                  5 },	// NAD83 / Connecticut
	{    81996L, 26957L, "DE83",                       "Delaware 0700 (1983, meters)",                                     5 },	// NAD83 / Delaware
	{    81999L, 26958L, "FL83-E",                     "Florida 0901, Eastern Zone (1983, meters)",                        5 },	// NAD83 / Florida East
	{    82002L, 26959L, "FL83-W",                     "Florida 0902, Western Zone (1983, meters)",                        5 },	// NAD83 / Florida West
	{    41011L, 26960L, "FL83-N",                     "Florida 0903, Northern Zone (1983, meters)",                       5 },	// NAD83 / Florida North
	{    82050L, 26961L, "HI83-1",                     "Hawaii 5101, Zone 1 (1983, meters)",                               5 },	// NAD83 / Hawaii zone 1
	{    82053L, 26962L, "HI83-2",                     "Hawaii 5102, Zone 2 (1983, meters)",                               5 },	// NAD83 / Hawaii zone 2
	{    82056L, 26963L, "HI83-3",                     "Hawaii 5103, Zone 3 (1983, meters)",                               5 },	// NAD83 / Hawaii zone 3
	{    82059L, 26964L, "HI83-4",                     "Hawaii 5104, Zone 4 (1983, meters)",                               5 },	// NAD83 / Hawaii zone 4
	{    82062L, 26965L, "HI83-5",                     "Hawaii 5105, Zone 5 (1983, meters)",                               5 },	// NAD83 / Hawaii zone 5
	{    82043L, 26966L, "GA83-E",                     "Georgia 1001, Eastern Zone (1983, meters)",                        5 },	// NAD83 / Georgia East
	{    82046L, 26967L, "GA83-W",                     "Georgia 1002, Western Zone (1983, meters)",                        5 },	// NAD83 / Georgia West
	{    82067L, 26968L, "ID83-E",                     "Idaho 1101, Eastern Zone (1983, meters)",                          5 },	// NAD83 / Idaho East
	{    82070L, 26969L, "ID83-C",                     "Idaho 1102, Central Zone (1983, meters)",                          5 },	// NAD83 / Idaho Central
	{    82073L, 26970L, "ID83-W",                     "Idaho 1103, Western Zone (1983, meters)",                          5 },	// NAD83 / Idaho West
	{    82076L, 26971L, "IL83-E",                     "Illinois 1201, Eastern Zone (1983, meters)",                       5 },	// NAD83 / Illinois East
	{    82079L, 26972L, "IL83-W",                     "Illinois 1202, Western Zone (1983, meters)",                       5 },	// NAD83 / Illinois West
	{    82082L, 26973L, "IN83-E",                     "Indiana 1301, Eastern Zone (1983, meters)",                        5 },	// NAD83 / Indiana East
	{    82085L, 26974L, "IN83-W",                     "Indiana 1302, Western Zone (1983, meters)",                        5 },	// NAD83 / Indiana West
	{    41022L, 26975L, "IA83-N",                     "Iowa 1401, Northern Zone (1983, meters)",                          5 },	// NAD83 / Iowa North
	{    41025L, 26976L, "IA83-S",                     "Iowa 1402, Southern Zone (1983, meters)",                          5 },	// NAD83 / Iowa South
	{    41028L, 26977L, "KS83-N",                     "Kansas 1501, Northern Zone (1983, meters)",                        5 },	// NAD83 / Kansas North
	{    41031L, 26978L, "KS83-S",                     "Kansas 1502, Southern Zone (1983, meters)",                        5 },	// NAD83 / Kansas South
	{    41034L,  2205L, "KY83-N",                     "Kentucky 1601, Northern Zone (1983, meters)",                      5 },	// NAD83 / Kentucky North
	{    41037L, 26980L, "KY83-S",                     "Kentucky 1602, Southern Zone (1983, meters)",                      5 },	// NAD83 / Kentucky South
	{    41040L, 26981L, "LA83-N",                     "Louisiana 1701, Northern Zone (1983, meters)",                     5 },	// NAD83 / Louisiana North
	{    41043L, 26982L, "LA83-S",                     "Louisiana 1702, Southern Zone (1983, meters)",                     5 },	// NAD83 / Louisiana South
	{    82114L, 26983L, "ME83-E",                     "Maine 1801, Eastern Zone (1983, meters)",                          5 },	// NAD83 / Maine East
	{    82116L, 26984L, "ME83-W",                     "Maine 1802, Western Zone (1983, meters)",                          5 },	// NAD83 / Maine West
	{    41049L, 26985L, "MD83",                       "Maryland 1900 (1983, meters)",                                     5 },	// NAD83 / Maryland
	{    41052L, 26986L, "MA83",                       "Massachusetts 2001, Mainland Zone (1983, meters)",                 5 },	// NAD83 / Massachusetts Mainland
	{    41055L, 26987L, "MA83-IS",                    "Massachusetts 2002, Island Zone (1983, meters)",                   5 },	// NAD83 / Massachusetts Island
	{    41059L, 26988L, "MI83-N",                     "Michigan 2111, Northern Zone (1983, meters)",                      5 },	// NAD83 / Michigan North
	{    41063L, 26989L, "MI83-C",                     "Michigan 2112, Central Zone (1983, meters)",                       5 },	// NAD83 / Michigan Central
	{    41067L, 26990L, "MI83-S",                     "Michigan 2113, Southern Zone (1983, meters)",                      5 },	// NAD83 / Michigan South
	{    41070L, 26991L, "MN83-N",                     "Minnesota 2201, Northern Zone (1983, meters)",                     5 },	// NAD83 / Minnesota North
	{    41073L, 26992L, "MN83-C",                     "Minnesota 2202, Central Zone (1983, meters)",                      5 },	// NAD83 / Minnesota Central
	{    41076L, 26993L, "MN83-S",                     "Minnesota 2203, South Zone (1983, meters)",                        5 },	// NAD83 / Minnesota South
	{    82120L, 26994L, "MS83-E",                     "Mississippi 2301, Eastern Zone (1983, meters)",                    5 },	// NAD83 / Mississippi East
	{    82123L, 26995L, "MS83-W",                     "Mississippi 2302, Western Zone (1983, meters)",                    5 },	// NAD83 / Mississippi West
	{    82126L, 26996L, "MO83-E",                     "Missouri 2401, Eastern Zone (1983, meters)",                       5 },	// NAD83 / Missouri East
	{    82129L, 26997L, "MO83-C",                     "Missouri 2402, Central Zone (1983, meters)",                       5 },	// NAD83 / Missouri Central
	{    82132L, 26998L, "MO83-W",                     "Missouri 2403, Western Zone (1983, meters)",                       5 },	// NAD83 / Missouri West
	{   335872L, 27200L, "NZGD49.NewZealandGrid",      "New Zealand Map Grid",                                             5 },	// NZGD49 / New Zealand Map Grid
	{    82133L, 27205L, "NZGD49.MountEden",           "Mt. Eden Circuit",                                                 4 },	// NZGD49 / Mount Eden Circuit
	{    81987L, 27206L, "NZGD49.BayOfPlenty",         "Bay of Plenty Circuit",                                            4 },	// NZGD49 / Bay of Plenty Circuit
	{    82179L, 27207L, "NZGD49.PovertyBay",          "Poverty Bay Circuit",                                              4 },	// NZGD49 / Poverty Bay Circuit
	{    82063L, 27208L, "NZGD49.HawkesBay",           "Hawkes Bay Circuit",                                               4 },	// NZGD49 / Hawkes Bay Circuit
	{    82203L, 27209L, "NZGD49.Taranaki",            "Taranaki Circuit",                                                 4 },	// NZGD49 / Taranaki Circuit
	{    82205L, 27210L, "NZGD49.Tuhirangi",           "Tuhirangi Circuit",                                                4 },	// NZGD49 / Tuhirangi Circuit
	{    82446L, 27211L, "NZGD49.Wanganui",            "Wanganui Circuit",                                                 4 },	// NZGD49 / Wanganui Circuit
	{    82445L, 27212L, "NZGD49.Wairarapa",           "Wairarapa Circuit",                                                4 },	// NZGD49 / Wairarapa Circuit
	{    82447L, 27213L, "NZGD49.Wellington",          "Wellington Circuit",                                               4 },	// NZGD49 / Wellington Circuit
	{    81992L, 27214L, "NZGD49.Collingwood",         "Collingwood Circuit",                                              4 },	// NZGD49 / Collingwood Circuit
	{    82139L, 27215L, "NZGD49.Nelson",              "Nelson Circuit",                                                   4 },	// NZGD49 / Nelson Circuit
	{    82109L, 27216L, "NZGD49.Karamea",             "Karamea Circuit",                                                  4 },	// NZGD49 / Karamea Circuit
	{    81991L, 27217L, "NZGD49.Buller",              "Buller Circuit",                                                   4 },	// NZGD49 / Buller Circuit
	{    82047L, 27218L, "NZGD49.Grey",                "Grey Circuit",                                                     4 },	// NZGD49 / Grey Circuit
	{    81974L, 27219L, "NZGD49.Amuri",               "Amuri Circuit",                                                    4 },	// NZGD49 / Amuri Circuit
	{    82117L, 27220L, "NZGD49.Marlborough",         "Marlborough Circuit",                                              4 },	// NZGD49 / Marlborough Circuit
	{    82064L, 27221L, "NZGD49.Hokitika",            "Hokitika Circuit",                                                 4 },	// NZGD49 / Hokitika Circuit
	{    82175L, 27222L, "NZGD49.Okarito",             "Okarito Circuit",                                                  4 },	// NZGD49 / Okarito Circuit
	{    82089L, 27223L, "NZGD49.JacksonsBay",         "Jacksons Bay Circuit",                                             4 },	// NZGD49 / Jacksons Bay Circuit
	{    82135L, 27224L, "NZGD49.MountPleasant",       "Mt. Pleasant Circuit",                                             4 },	// NZGD49 / Mount Pleasant Circuit
	{    82040L, 27225L, "NZGD49.Gawler",              "Gawler Circuit",                                                   4 },	// NZGD49 / Gawler Circuit
	{    82204L, 27226L, "NZGD49.Timaru",              "Timaru Circuit",                                                   4 },	// NZGD49 / Timaru Circuit
	{    82110L, 27227L, "NZGD49.LindisPeak",          "Lindis Peak Circuit",                                              4 },	// NZGD49 / Lindis Peak Circuit
	{    82134L, 27228L, "NZGD49.MountNicholas",       "Mt. Nicholas Circuit",                                             4 },	// NZGD49 / Mount Nicholas Circuit
	{    82136L, 27229L, "NZGD49.MountYork",           "Mt. York Circuit",                                                 4 },	// NZGD49 / Mount York Circuit
	{    82174L, 27230L, "NZGD49.ObservationPnt",      "Observation Pt. Circuit",                                          4 },	// NZGD49 / Observation Point Circuit
	{    82173L, 27231L, "NZGD49.NorthTaieri",         "North Taieri Circuit",                                             4 },	// NZGD49 / North Taieri Circuit
	{    81988L, 27232L, "NZGD49.Bluff",               "Bluff Circuit",                                                    4 },	// NZGD49 / Bluff Circuit
	{    82176L, 27492L, "Datum73.ModPortgGrd",        "Portuguese National System (D73)",                                 4 },	// Datum 73 / Modified Portuguese Grid
//ADSK-HW mapped to the 8 newly created systems that use the CLRK-IGN ellipsoidinstead of the CLRK80. 
	{    41013L, 27561L, "NTF.Lambert-1-ClrkIGN",      "French Lambert I Nord",                                            5 },	// NTF (Paris) / Lambert Nord France
	{    41015L, 27562L, "NTF.Lambert-2-ClrkIGN",      "French Lambert II Centre",                                         5 },	// NTF (Paris) / Lambert Centre France
	{    41017L, 27563L, "NTF.Lambert-3-ClrkIGN",      "French Lambert III Sud",                                           5 },	// NTF (Paris) / Lambert Sud France
	{    41019L, 27564L, "NTF.Lambert-4-ClrkIGN",      "French Lambert IV Corse",                                          5 },	// NTF (Paris) / Lambert Corse
	{    41012L, 27571L, "NTF.Lambert-1C-ClrkIGN",     "French Lambert I Carto",                                           5 },	// NTF (Paris) / Lambert zone I
	{    41014L, 27572L, "NTF.Lambert-2C-ClrkIGN",     "French Lambert II Carto",                                          5 },	// NTF (Paris) / Lambert zone II
	{    41016L, 27573L, "NTF.Lambert-3C-ClrkIGN",     "French Lambert III Carto",                                         5 },	// NTF (Paris) / Lambert zone III
	{    41018L, 27574L, "NTF.Lambert-4C-ClrkIGN",     "French Lambert IV Carto",                                          5 },	// NTF (Paris) / Lambert zone IV
	{    81989L, 27700L, "BritishNatGrid",             "British National Grid",                                            5 },	// OSGB 1936 / British National Grid
	{    82461L, 28348L, "MGA94-48",                   "MGA94 Zone 48",                                                    5 },	// GDA94 / MGA zone 48
	{    82462L, 28349L, "MGA94-49",                   "MGA94 Zone 49",                                                    5 },	// GDA94 / MGA zone 49
	{    82463L, 28350L, "MGA94-50",                   "MGA94 Zone 50",                                                    5 },	// GDA94 / MGA zone 50
	{    82464L, 28351L, "MGA94-51",                   "MGA94 Zone 51",                                                    5 },	// GDA94 / MGA zone 51
	{    82465L, 28352L, "MGA94-52",                   "MGA94 Zone 52",                                                    5 },	// GDA94 / MGA zone 52
	{    82466L, 28353L, "MGA94-53",                   "MGA94 Zone 53",                                                    5 },	// GDA94 / MGA zone 53
	{    82467L, 28354L, "MGA94-54",                   "MGA94 Zone 54",                                                    5 },	// GDA94 / MGA zone 54
	{    82468L, 28355L, "MGA94-55",                   "MGA94 Zone 55",                                                    5 },	// GDA94 / MGA zone 55
	{    82469L, 28356L, "MGA94-56",                   "MGA94 Zone 56",                                                    5 },	// GDA94 / MGA zone 56
	{    82470L, 28357L, "MGA94-57",                   "MGA94 Zone 57",                                                    5 },	// GDA94 / MGA zone 57
	{    82471L, 28358L, "MGA94-58",                   "MGA94 Zone 58",                                                    5 },	// GDA94 / MGA zone 58
	{    82033L, 28404L, "GK42-4",                     "GK Zone 4 (Pulkovo 1942)",                                         5 },	// Pulkovo 1942 / Gauss-Kruger zone 4
	{    82035L, 28405L, "GK42-5",                     "GK Zone 5 (Pulkovo 1942)",                                         5 },	// Pulkovo 1942 / Gauss-Kruger zone 5
	{    82036L, 28406L, "GK42-6",                     "GK Zone 6 (Pulkovo 1942)",                                         5 },	// Pulkovo 1942 / Gauss-Kruger zone 6
	{    82037L, 28407L, "GK42-7",                     "GK Zone 7 (Pulkovo 1942)",                                         5 },	// Pulkovo 1942 / Gauss-Kruger zone 7
	{    82038L, 28408L, "GK42-8",                     "GK Zone 8 (Pulkovo 1942)",                                         5 },	// Pulkovo 1942 / Gauss-Kruger zone 8
	{    82039L, 28409L, "GK42-9",                     "GK Zone 9 (Pulkovo 1942)",                                         5 },	// Pulkovo 1942 / Gauss-Kruger zone 9
	{    82005L, 28410L, "GK42-10",                    "GK Zone 10 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 10
	{    82006L, 28411L, "GK42-11",                    "GK Zone 11 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 11
	{    82007L, 28412L, "GK42-12",                    "GK Zone 12 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 12
	{    82008L, 28413L, "GK42-13",                    "GK Zone 13 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 13
	{    82009L, 28414L, "GK42-14",                    "GK Zone 14 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 14
	{    82010L, 28415L, "GK42-15",                    "GK Zone 15 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 15
	{    82011L, 28416L, "GK42-16",                    "GK Zone 16 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 16
	{    82012L, 28417L, "GK42-17",                    "GK Zone 17 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 17
	{    82013L, 28418L, "GK42-18",                    "GK Zone 18 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 18
	{    82014L, 28419L, "GK42-19",                    "GK Zone 19 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 19
	{    82017L, 28420L, "GK42-20",                    "GK Zone 20 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 20
	{    82018L, 28421L, "GK42-21",                    "GK Zone 21 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 21
	{    82019L, 28422L, "GK42-22",                    "GK Zone 22 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 22
	{    82020L, 28423L, "GK42-23",                    "GK Zone 23 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 23
	{    82021L, 28424L, "GK42-24",                    "GK Zone 24 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 24
	{    82022L, 28425L, "GK42-25",                    "GK Zone 25 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 25
	{    82023L, 28426L, "GK42-26",                    "GK Zone 26 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 26
	{    82024L, 28427L, "GK42-27",                    "GK Zone 27 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 27
	{    82025L, 28428L, "GK42-28",                    "GK Zone 28 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 28
	{    82026L, 28429L, "GK42-29",                    "GK Zone 29 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 29
	{    82029L, 28430L, "GK42-30",                    "GK Zone 30 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 30
	{    82030L, 28431L, "GK42-31",                    "GK Zone 31 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 31
	{    82031L, 28432L, "GK42-32",                    "GK Zone 32 (Pulkovo 1942)",                                        5 },	// Pulkovo 1942 / Gauss-Kruger zone 32
	{    90112L, 28992L, "Netherlands-RD-New",         "Netherlands National System",                                      5 },	// Amersfoort / RD New
	{    82255L, 29168L, "SAD69.UTM-18N",              "UTM Zone 18, Northern Hemisphere (SAD 69)",                        4 },	// SAD69 / UTM zone 18N
	{    82264L, 29169L, "SAD69.UTM-19N",              "UTM Zone 19, Northern Hemisphere (SAD 69)",                        4 },	// SAD69 / UTM zone 19N
	{    82276L, 29170L, "SAD69.UTM-20N",              "UTM Zone 20, Northern Hemisphere (SAD 69)",                        4 },	// SAD69 / UTM zone 20N
	{    82284L, 29171L, "SAD69.UTM-21N",              "UTM Zone 21, Northern Hemisphere (SAD 69)",                        4 },	// SAD69 / UTM zone 21N
	{    82292L, 29172L, "SAD69.UTM-22N",              "UTM Zone 22, Northern Hemisphere (SAD 69)",                        4 },	// SAD69 / UTM zone 22N
	{    82250L, 29177L, "SAD69.UTM-17S",              "UTM Zone 17, Southern Hemisphere (SAD 69)",                        6 },	// SAD69 / UTM zone 17S
	{    82258L, 29178L, "SAD69.UTM-18S",              "UTM Zone 18, Southern Hemisphere (SAD 69)",                        6 },	// SAD69 / UTM zone 18S
	{    82267L, 29179L, "SAD69.UTM-19S",              "UTM Zone 19, Southern Hemisphere (SAD 69)",                        6 },	// SAD69 / UTM zone 19S
	{    82279L, 29180L, "SAD69.UTM-20S",              "UTM Zone 20, Southern Hemisphere (SAD 69)",                        6 },	// SAD69 / UTM zone 20S
	{    82287L, 29181L, "SAD69.UTM-21S",              "UTM Zone 21, Southern Hemisphere (SAD 69)",                        6 },	// SAD69 / UTM zone 21S
	{    82295L, 29182L, "SAD69.UTM-22S",              "UTM Zone 22, Southern Hemisphere (SAD 69)",                        6 },	// SAD69 / UTM zone 22S
	{    82301L, 29183L, "SAD69.UTM-23S",              "UTM Zone 23, Southern Hemisphere (SAD 69)",                        6 },	// SAD69 / UTM zone 23S
	{    82307L, 29184L, "SAD69.UTM-24S",              "UTM Zone 24, Southern Hemisphere (SAD 69)",                        6 },	// SAD69 / UTM zone 24S
	{    82313L, 29185L, "SAD69.UTM-25S",              "UTM Zone 25, Southern Hemisphere (SAD 69)",                        6 },	// SAD69 / UTM zone 25S
	{    82015L, 31466L, "DHDN.Gauss3d-2",             "GK Zone 2 (DHDN)",                                                 4 },	// DHDN / Gauss-Kruger zone 2
	{    82027L, 31467L, "DHDN.Gauss3d-3",             "GK Zone 3 (DHDN)",                                                 4 },	// DHDN / Gauss-Kruger zone 3
	{    82032L, 31468L, "DHDN.Gauss3d-4",             "GK Zone 4 (DHDN)",                                                 4 },	// DHDN / Gauss-Kruger zone 4
	{    82034L, 31469L, "DHDN.Gauss3d-5",             "GK Zone 5 (DHDN)",                                                 4 },	// DHDN / Gauss-Kruger zone 5
	{    41080L, 32001L, "MT-N",                       "Montana 2501, Northern Zone (1927)",                               5 },	// NAD27 / Montana North
	{    41081L, 32002L, "MT-C",                       "Montana 2502, Central Zone (1927)",                                5 },	// NAD27 / Montana Central
	{    41082L, 32003L, "MT-S",                       "Montana 2503, Southern Zone (1927)",                               5 },	// NAD27 / Montana South
	{    41085L, 32005L, "NB-N",                       "Nebraska 2601, Northern Zone (1927)",                              5 },	// NAD27 / Nebraska North
	{    41086L, 32006L, "NB-S",                       "Nebraska 2602, Southern Zone (1927)",                              5 },	// NAD27 / Nebraska South
	{    82141L, 32007L, "NV-E",                       "Nevada 2701, Eastern Zone (1927)",                                 5 },	// NAD27 / Nevada East
	{    82143L, 32008L, "NV-C",                       "Nevada 2702, Central Zone (1927)",                                 5 },	// NAD27 / Nevada Central
	{    82146L, 32009L, "NV-W",                       "Nevada 2703, Western Zone (1927)",                                 5 },	// NAD27 / Nevada West
	{    82149L, 32010L, "NH",                         "New Hampshire 2800 (1927)",                                        5 },	// NAD27 / New Hampshire
	{    82152L, 32011L, "NJ",                         "New Jersey 2900 (1927)",                                           5 },	// NAD27 / New Jersey
	{    82156L, 32012L, "NM-E",                       "New Mexico 3001, Eastern Zone (1927)",                             5 },	// NAD27 / New Mexico East
	{    82158L, 32013L, "NM-C",                       "New Mexico 3002, Central Zone (1927)",                             5 },	// NAD27 / New Mexico Central
	{    82161L, 32014L, "NM-W",                       "New Mexico 3003, Western Zone (1927)",                             5 },	// NAD27 / New Mexico West
	{    82164L, 32015L, "NY-E",                       "New York 3101, Eastern Zone (1927)",                               5 },	// NAD27 / New York East
	{    82167L, 32016L, "NY-C",                       "New York 3102, Central Zone (1927)",                               5 },	// NAD27 / New York Central
	{    82170L, 32017L, "NY-W",                       "New York 3103, Western Zone (1927)",                               5 },	// NAD27 / New York West
	{    41087L, 32018L, "NY-LI",                      "New York 3104, Long Island (1927)",                                5 },	// NAD27 / New York Long Island
	{    41090L, 32019L, "NC",                         "North Carolina 3200 (1927)",                                       5 },	// NAD27 / North Carolina
	{    41093L, 32020L, "ND-N",                       "North Dakota 3301, Northern Zone (1927)",                          5 },	// NAD27 / North Dakota North
	{    41096L, 32021L, "ND-S",                       "North Dakota 3302, Southern Zone (1927)",                          5 },	// NAD27 / North Dakota South
	{    41099L, 32022L, "OH-N",                       "Ohio 3401, Northern Zone (1927)",                                  5 },	// NAD27 / Ohio North
	{    41102L, 32023L, "OH-S",                       "Ohio 3402, Southern Zone (1927)",                                  5 },	// NAD27 / Ohio South
	{    41105L, 32024L, "OK-N",                       "Oklahoma 3501, Northern Zone (1927)",                              5 },	// NAD27 / Oklahoma North
	{    41108L, 32025L, "OK-S",                       "Oklahoma 3502, Southern Zone (1927)",                              5 },	// NAD27 / Oklahoma South
	{    41111L, 32026L, "OR-N",                       "Oregon 3601, Northern Zone (1927)",                                5 },	// NAD27 / Oregon North
	{    41115L, 32027L, "OR-S",                       "Oregon 3602, Southern Zone (1927)",                                5 },	// NAD27 / Oregon South
	{    41119L, 32028L, "PA-N",                       "Pennsylvania 3701, Northern Zone (1927)",                          5 },	// NAD27 / Pennsylvania North
	{    41122L, 32029L, "PA-S",                       "Pennsylvania 3702, Southern Zone (1927)",                          5 },	// NAD27 / Pennsylvania South
	{    82199L, 32030L, "RI",                         "Rhode Island 3800 (1927)",                                         5 },	// NAD27 / Rhode Island
	{    41132L, 32031L, "SC-N",                       "South Carolina 3901, Northern Zone (1927)",                        5 },	// NAD27 / South Carolina North
	{    41133L, 32033L, "SC-S",                       "South Carolina 3902, Southern Zone (1927)",                        5 },	// NAD27 / South Carolina South
	{    41134L, 32034L, "SD-N",                       "South Dakota 4001, Northern Zone (1927)",                          5 },	// NAD27 / South Dakota North
	{    41137L, 32035L, "SD-S",                       "South Dakota 4002, Southern Zone (1927)",                          5 },	// NAD27 / South Dakota South
	{    41143L, 32037L, "TX-N",                       "Texas 4201, Northern Zone (1927)",                                 5 },	// NAD27 / Texas North
	{    41146L, 32038L, "TX-NC",                      "Texas 4202, North Central Zone (1927)",                            5 },	// NAD27 / Texas North Central
	{    41149L, 32039L, "TX-C",                       "Texas 4203, Central Zone (1927)",                                  5 },	// NAD27 / Texas Central
	{    41152L, 32040L, "TX-SC",                      "Texas 4204, South Central Zone (1927)",                            5 },	// NAD27 / Texas South Central
	{    41155L, 32041L, "TX-S",                       "Texas 4205, Southern Zone (1927)",                                 5 },	// NAD27 / Texas South
	{    41158L, 32042L, "UT-N",                       "Utah 4301, Northern Zone (1927)",                                  5 },	// NAD27 / Utah North
	{    41162L, 32043L, "UT-C",                       "Utah 4302, Central Zone (1927)",                                   5 },	// NAD27 / Utah Central
	{    41166L, 32044L, "UT-S",                       "Utah 4303, Southern Zone (1927)",                                  5 },	// NAD27 / Utah South
	{    82442L, 32045L, "VT",                         "Vermont 4400 (1927)",                                              5 },	// NAD27 / Vermont
	{    41170L, 32046L, "VA-N",                       "Virginia 4501, Northern Zone (1927)",                              5 },	// NAD27 / Virginia North
	{    41173L, 32047L, "VA-S",                       "Virginia 4502, Southern Zone (1927)",                              5 },	// NAD27 / Virginia South
	{    41176L, 32048L, "WA-N",                       "Washington 4601, Northern Zone (1927)",                            5 },	// NAD27 / Washington North
	{    41179L, 32049L, "WA-S",                       "Washington 4602, Southern Zone (1927)",                            5 },	// NAD27 / Washington South
	{    41182L, 32050L, "WV-N",                       "West Virginia 4701, Northern Zone (1927)",                         5 },	// NAD27 / West Virginia North
	{    41185L, 32051L, "WV-S",                       "West Virginia 4702, Southern Zone (1927)",                         5 },	// NAD27 / West Virginia South
	{    41188L, 32052L, "WI-N",                       "Wisconsin 4801, Northern Zone (1927)",                             5 },	// NAD27 / Wisconsin North
	{    41191L, 32053L, "WI-C",                       "Wisconsin 4802, Central Zone (1927)",                              5 },	// NAD27 / Wisconsin Central
	{    41194L, 32054L, "WI-S",                       "Wisconsin 4803, Southern Zone (1927)",                             5 },	// NAD27 / Wisconsin South
	{    82448L, 32055L, "WY-E",                       "Wyoming 4901, Eastern Zone (1927)",                                5 },	// NAD27 / Wyoming East
	{    82451L, 32056L, "WY-EC",                      "Wyoming 4902, East Central Zone (1927)",                           5 },	// NAD27 / Wyoming East Central
	{    82454L, 32057L, "WY-WC",                      "Wyoming 4903, West Central Zone (1927)",                           5 },	// NAD27 / Wyoming West Central
	{    82457L, 32058L, "WY-W",                       "Wyoming 4904, Western Zone (1927)",                                5 },	// NAD27 / Wyoming West
	{    41044L, 32099L, "LA-O",                       "Louisiana 1703, Offshore Zone (1927)",                             5 },	// NAD27 / Louisiana Offshore
	{    41079L, 32100L, "MT83",                       "Montana 2500 (1983, meters)",                                      5 },	// NAD83 / Montana
	{    41084L, 32104L, "NB83",                       "Nebraska 2600 (1983, meters)",                                     5 },	// NAD83 / Nebraska
	{    82140L, 32107L, "NV83-E",                     "Nevada 2701 , Eastern Zone (1983, meters)",                        5 },	// NAD83 / Nevada East
	{    82145L, 32108L, "NV83-C",                     "Nevada 2702, Central Zone (1983, meters)",                         5 },	// NAD83 / Nevada Central
	{    82148L, 32109L, "NV83-W",                     "Nevada 2703, Western Zone (1983, meters)",                         5 },	// NAD83 / Nevada West
	{    82151L, 32110L, "NH83",                       "New Hampshire 2800 (1983, meters)",                                5 },	// NAD83 / New Hampshire
	{    82154L, 32111L, "NJ83",                       "New Jersey 2900 (1983, meters)",                                   5 },	// NAD83 / New Jersey
	{    82157L, 32112L, "NM83-E",                     "New Mexico 3001, Eastern Zone (1983, meters)",                     5 },	// NAD83 / New Mexico East
	{    82160L, 32113L, "NM83-C",                     "New Mexico 3002, Central Zone (1983, meters)",                     5 },	// NAD83 / New Mexico Central
	{    82163L, 32114L, "NM83-W",                     "New Mexico 3003, Western Zone (1983, meters)",                     5 },	// NAD83 / New Mexico West
	{    82166L, 32115L, "NY83-E",                     "New York 3101, Eastern Zone (1983, meters)",                       5 },	// NAD83 / New York East
	{    82169L, 32116L, "NY83-C",                     "New York 3102, Central Zone (1983, meters)",                       5 },	// NAD83 / New York Central
	{    82172L, 32117L, "NY83-W",                     "New York 3103, Western Zone (1983, meters)",                       5 },	// NAD83 / New York West
	{    41089L, 32118L, "NY83-LI",                    "New York 3104, Long Island Zone (1983, meters)",                   5 },	// NAD83 / New York Long Island
	{    41092L, 32119L, "NC83",                       "North Carolina 3200 (1983, meters)",                               5 },	// NAD83 / North Carolina
	{    41095L, 32120L, "ND83-N",                     "North Dakota 3301, Northern Zone (1983, meters)",                  5 },	// NAD83 / North Dakota North
	{    41098L, 32121L, "ND83-S",                     "North Dakota 3302, Southern Zone (1983, meters)",                  5 },	// NAD83 / North Dakota South
	{    41101L, 32122L, "OH83-N",                     "Ohio 3401, Northern Zone (1983, meters)",                          5 },	// NAD83 / Ohio North
	{    41104L, 32123L, "OH83-S",                     "Ohio 3402, Southern Zone (1983, meters)",                          5 },	// NAD83 / Ohio South
	{    41107L, 32124L, "OK83-N",                     "Oklahoma 3501, Northern Zone (1983, meters)",                      5 },	// NAD83 / Oklahoma North
	{    41110L, 32125L, "OK83-S",                     "Oklahoma 3502, Southern Zone (1983, meters)",                      5 },	// NAD83 / Oklahoma South
	{    41114L, 32126L, "OR83-N",                     "Oregon 3601, Northern Zone (1983, meters)",                        5 },	// NAD83 / Oregon North
	{    41118L, 32127L, "OR83-S",                     "Oregon 3602, Southern Zone (1983, meters)",                        5 },	// NAD83 / Oregon South
	{    41121L, 32128L, "PA83-N",                     "Pennsylvania 3701, Northern Zone (1983, meters)",                  5 },	// NAD83 / Pennsylvania North
	{    41124L, 32129L, "PA83-S",                     "Pennsylvania 3702, Southern Zone (1983, meters)",                  5 },	// NAD83 / Pennsylvania South
	{    82201L, 32130L, "RI83",                       "Rhode Island 3800 (1983, meters)",                                 5 },	// NAD83 / Rhode Island
	{    41131L, 32133L, "SC83",                       "South Carolina 3900 (1983, meters)",                               5 },	// NAD83 / South Carolina
	{    41136L, 32134L, "SD83-N",                     "South Dakota 4001, Northern Zone (1983, meters)",                  5 },	// NAD83 / South Dakota North
	{    41139L, 32135L, "SD83-S",                     "South Dakota 4002, Southern Zone (1983, meters)",                  5 },	// NAD83 / South Dakota South
	{    41142L, 32136L, "TN83",                       "Tennessee 4100 (1983, meters)",                                    5 },	// NAD83 / Tennessee
	{    41145L, 32137L, "TX83-N",                     "Texas 4201, Northern Zone (1983, meters)",                         5 },	// NAD83 / Texas North
	{    41148L, 32138L, "TX83-NC",                    "Texas 4202, North Central Zone (1983, meters)",                    5 },	// NAD83 / Texas North Central
	{    41151L, 32139L, "TX83-C",                     "Texas 4203, Central Zone (1983, meters)",                          5 },	// NAD83 / Texas Central
	{    41154L, 32140L, "TX83-SC",                    "Texas 4204, South Central Zone (1983, meters)",                    5 },	// NAD83 / Texas South Central
	{    41157L, 32141L, "TX83-S",                     "Texas 4205, Southern Zone (1983, meters)",                         5 },	// NAD83 / Texas South
	{    41161L, 32142L, "UT83-N",                     "Utah 4301, Northern Zone (1983, meters)",                          5 },	// NAD83 / Utah North
	{    41165L, 32143L, "UT83-C",                     "Utah 4302, Central Zone (1983, meters)",                           5 },	// NAD83 / Utah Central
	{    41169L, 32144L, "UT83-S",                     "Utah 4303, Southern Zone (1983, meters)",                          5 },	// NAD83 / Utah South
	{    82444L, 32145L, "VT83",                       "Vermont 4400 (1983, meters)",                                      5 },	// NAD83 / Vermont
	{    41172L, 32146L, "VA83-N",                     "Virginia 4501, Northern Zone (1983, meters)",                      5 },	// NAD83 / Virginia North
	{    41175L, 32147L, "VA83-S",                     "Virginia 4502, Southern Zone (1983, meters)",                      5 },	// NAD83 / Virginia South
	{    41178L, 32148L, "WA83-N",                     "Washington 4601, Northern Zone (1983, meters)",                    5 },	// NAD83 / Washington North
	{    41181L, 32149L, "WA83-S",                     "Washington 4602, Southern Zone (1983, meters)",                    5 },	// NAD83 / Washington South
	{    41184L, 32150L, "WV83-N",                     "West Virginia 4701, Northern Zone (1983, meters)",                 5 },	// NAD83 / West Virginia North
	{    41187L, 32151L, "WV83-S",                     "West Virginia 4702, Southern Zone (1983, meters)",                 5 },	// NAD83 / West Virginia South
	{    41190L, 32152L, "WI83-N",                     "Wisconsin 4801, Northern Zone (1983, meters)",                     5 },	// NAD83 / Wisconsin North
	{    41193L, 32153L, "WI83-C",                     "Wisconsin 4802, Central Zone (1983, meters)",                      5 },	// NAD83 / Wisconsin Central
	{    41196L, 32154L, "WI83-S",                     "Wisconsin 4803, Southern Zone (1983, meters)",                     5 },	// NAD83 / Wisconsin South
	{    82450L, 32155L, "WY83-E",                     "Wyoming 4901, Eastern Zone (1983, meters)",                        5 },	// NAD83 / Wyoming East
	{    82453L, 32156L, "WY83-EC",                    "Wyoming 4902, East Central Zone (1983, meters)",                   5 },	// NAD83 / Wyoming East Central
	{    82456L, 32157L, "WY83-WC",                    "Wyoming 4903, West Central Zone (1983, meters)",                   5 },	// NAD83 / Wyoming West Central
	{    82459L, 32158L, "WY83-W",                     "Wyoming 4904, Western Zone (1983, meters)",                        5 },	// NAD83 / Wyoming West
	{    41126L, 32161L, "PR83",                       "Puerto Rico and Virgin Islands 5200 (1983, meters)",               5 },	// NAD83 / Puerto Rico & Virgin Is.
	{    41046L, 32199L, "LA83-O",                     "Louisiana 1703, Offshore Zone (1983, meters)",                     5 },	// NAD83 / Louisiana Offshore
	{    82208L, 32601L, "UTM84-1N",                   "UTM Zone 1, Northern Hemisphere (WGS 84)",                         5 },	// WGS 84 / UTM zone 1N
	{    82271L, 32602L, "UTM84-2N",                   "UTM Zone 2, Northern Hemisphere (WGS 84)",                         5 },	// WGS 84 / UTM zone 2N
	{    82335L, 32603L, "UTM84-3N",                   "UTM Zone 3, Northern Hemisphere (WGS 84)",                         5 },	// WGS 84 / UTM zone 3N
	{    82368L, 32604L, "UTM84-4N",                   "UTM Zone 4, Northern Hemisphere (WGS 84)",                         5 },	// WGS 84 / UTM zone 4N
	{    82392L, 32605L, "UTM84-5N",                   "UTM Zone 5, Northern Hemisphere (WGS 84)",                         5 },	// WGS 84 / UTM zone 5N
	{    82423L, 32606L, "UTM84-6N",                   "UTM Zone 6, Northern Hemisphere (WGS 84)",                         5 },	// WGS 84 / UTM zone 6N
	{    82430L, 32607L, "UTM84-7N",                   "UTM Zone 7, Northern Hemisphere (WGS 84)",                         5 },	// WGS 84 / UTM zone 7N
	{    82435L, 32608L, "UTM84-8N",                   "UTM Zone 8, Northern Hemisphere (WGS 84)",                         5 },	// WGS 84 / UTM zone 8N
	{    82440L, 32609L, "UTM84-9N",                   "UTM Zone 9, Northern Hemisphere (WGS 84)",                         5 },	// WGS 84 / UTM zone 9N
	{    82213L, 32610L, "UTM84-10N",                  "UTM Zone 10, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 10N
	{    82218L, 32611L, "UTM84-11N",                  "UTM Zone 11, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 11N
	{    82223L, 32612L, "UTM84-12N",                  "UTM Zone 12, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 12N
	{    82228L, 32613L, "UTM84-13N",                  "UTM Zone 13, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 13N
	{    82233L, 32614L, "UTM84-14N",                  "UTM Zone 14, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 14N
	{    82238L, 32615L, "UTM84-15N",                  "UTM Zone 15, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 15N
	{    82243L, 32616L, "UTM84-16N",                  "UTM Zone 16, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 16N
	{    82249L, 32617L, "UTM84-17N",                  "UTM Zone 17, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 17N
	{    82256L, 32618L, "UTM84-18N",                  "UTM Zone 18, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 18N
	{    82265L, 32619L, "UTM84-19N",                  "UTM Zone 19, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 19N
	{    82277L, 32620L, "UTM84-20N",                  "UTM Zone 20, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 20N
	{    82285L, 32621L, "UTM84-21N",                  "UTM Zone 21, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 21N
	{    82293L, 32622L, "UTM84-22N",                  "UTM Zone 22, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 22N
	{    82299L, 32623L, "UTM84-23N",                  "UTM Zone 23, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 23N
	{    82305L, 32624L, "UTM84-24N",                  "UTM Zone 24, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 24N
	{    82311L, 32625L, "UTM84-25N",                  "UTM Zone 25, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 25N
	{    82317L, 32626L, "UTM84-26N",                  "UTM Zone 26, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 26N
	{    82321L, 32627L, "UTM84-27N",                  "UTM Zone 27, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 27N
	{    82326L, 32628L, "UTM84-28N",                  "UTM Zone 28, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 28N
	{    82331L, 32629L, "UTM84-29N",                  "UTM Zone 29, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 29N
	{    82338L, 32630L, "UTM84-30N",                  "UTM Zone 30, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 30N
	{    82341L, 32631L, "UTM84-31N",                  "UTM Zone 31, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 31N
	{    82344L, 32632L, "UTM84-32N",                  "UTM Zone 32, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 32N
	{    82347L, 32633L, "UTM84-33N",                  "UTM Zone 33, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 33N
	{    82350L, 32634L, "UTM84-34N",                  "UTM Zone 34, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 34N
	{    82353L, 32635L, "UTM84-35N",                  "UTM Zone 35, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 35N
	{    82356L, 32636L, "UTM84-36N",                  "UTM Zone 36, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 36N
	{    82359L, 32637L, "UTM84-37N",                  "UTM Zone 37, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 37N
	{    82362L, 32638L, "UTM84-38N",                  "UTM Zone 38, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 38N
	{    82364L, 32639L, "UTM84-39N",                  "UTM Zone 39, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 39N
	{    82370L, 32640L, "UTM84-40N",                  "UTM Zone 40, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 40N
	{    82372L, 32641L, "UTM84-41N",                  "UTM Zone 41, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 41N
	{    82374L, 32642L, "UTM84-42N",                  "UTM Zone 42, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 42N
	{    82376L, 32643L, "UTM84-43N",                  "UTM Zone 43, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 43N
	{    82378L, 32644L, "UTM84-44N",                  "UTM Zone 44, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 44N
	{    82380L, 32645L, "UTM84-45N",                  "UTM Zone 45, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 45N
	{    82382L, 32646L, "UTM84-46N",                  "UTM Zone 46, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 46N
	{    82384L, 32647L, "UTM84-47N",                  "UTM Zone 47, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 47N
	{    82386L, 32648L, "UTM84-48N",                  "UTM Zone 48, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 48N
	{    82388L, 32649L, "UTM84-49N",                  "UTM Zone 49, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 49N
	{    82394L, 32650L, "UTM84-50N",                  "UTM Zone 50, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 50N
	{    82397L, 32651L, "UTM84-51N",                  "UTM Zone 51, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 51N
	{    82400L, 32652L, "UTM84-52N",                  "UTM Zone 52, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 52N
	{    82403L, 32653L, "UTM84-53N",                  "UTM Zone 53, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 53N
	{    82406L, 32654L, "UTM84-54N",                  "UTM Zone 54, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 54N
	{    82409L, 32655L, "UTM84-55N",                  "UTM Zone 55, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 55N
	{    82412L, 32656L, "UTM84-56N",                  "UTM Zone 56, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 56N
	{    82415L, 32657L, "UTM84-57N",                  "UTM Zone 57, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 57N
	{    82417L, 32658L, "UTM84-58N",                  "UTM Zone 58, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 58N
	{    82419L, 32659L, "UTM84-59N",                  "UTM Zone 59, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 59N
	{    82425L, 32660L, "UTM84-60N",                  "UTM Zone 60, Northern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 60N
	{    90113L, 32661L, "WGS84.UPSNorth",             "UPS North Zone",                                                   4 },	// WGS 84 / UPS North
	{    82209L, 32701L, "UTM84-1S",                   "UTM Zone 1, Southern Hemisphere (WGS 84)",                         5 },	// WGS 84 / UTM zone 1S
	{    82272L, 32702L, "UTM84-2S",                   "UTM Zone 2, Southern Hemisphere (WGS 84)",                         5 },	// WGS 84 / UTM zone 2S
	{    82336L, 32703L, "UTM84-3S",                   "UTM Zone 3, Southern Hemisphere (WGS 84)",                         5 },	// WGS 84 / UTM zone 3S
	{    82369L, 32704L, "UTM84-4S",                   "UTM Zone 4, Southern Hemisphere (WGS 84)",                         5 },	// WGS 84 / UTM zone 4S
	{    82393L, 32705L, "UTM84-5S",                   "UTM Zone 5, Southern Hemisphere (WGS 84)",                         5 },	// WGS 84 / UTM zone 5S
	{    82424L, 32706L, "UTM84-6S",                   "UTM Zone 6, Southern Hemisphere (WGS 84)",                         5 },	// WGS 84 / UTM zone 6S
	{    82431L, 32707L, "UTM84-7S",                   "UTM Zone 7, Southern Hemisphere (WGS 84)",                         5 },	// WGS 84 / UTM zone 7S
	{    82436L, 32708L, "UTM84-8S",                   "UTM Zone 8, Southern Hemisphere (WGS 84)",                         5 },	// WGS 84 / UTM zone 8S
	{    82441L, 32709L, "UTM84-9S",                   "UTM Zone 9, Southern Hemisphere (WGS 84)",                         5 },	// WGS 84 / UTM zone 9S
	{    82214L, 32710L, "UTM84-10S",                  "UTM Zone 10, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 10S
	{    82219L, 32711L, "UTM84-11S",                  "UTM Zone 11, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 11S
	{    82224L, 32712L, "UTM84-12S",                  "UTM Zone 12, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 12S
	{    82229L, 32713L, "UTM84-13S",                  "UTM Zone 13, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 13S
	{    82234L, 32714L, "UTM84-14S",                  "UTM Zone 14, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 14S
	{    82239L, 32715L, "UTM84-15S",                  "UTM Zone 15, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 15S
	{    82244L, 32716L, "UTM84-16S",                  "UTM Zone 16, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 16S
	{    82251L, 32717L, "UTM84-17S",                  "UTM Zone 17, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 17S
	{    82259L, 32718L, "UTM84-18S",                  "UTM Zone 18, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 18S
	{    82268L, 32719L, "UTM84-19S",                  "UTM Zone 19, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 19S
	{    82280L, 32720L, "UTM84-20S",                  "UTM Zone 20, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 20S
	{    82288L, 32721L, "UTM84-21S",                  "UTM Zone 21, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 21S
	{    82296L, 32722L, "UTM84-22S",                  "UTM Zone 22, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 22S
	{    82302L, 32723L, "UTM84-23S",                  "UTM Zone 23, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 23S
	{    82308L, 32724L, "UTM84-24S",                  "UTM Zone 24, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 24S
	{    82314L, 32725L, "UTM84-25S",                  "UTM Zone 25, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 25S
	{    82318L, 32726L, "UTM84-26S",                  "UTM Zone 26, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 26S
	{    82322L, 32727L, "UTM84-27S",                  "UTM Zone 27, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 27S
	{    82327L, 32728L, "UTM84-28S",                  "UTM Zone 28, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 28S
	{    82332L, 32729L, "UTM84-29S",                  "UTM Zone 29, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 29S
	{    82339L, 32730L, "UTM84-30S",                  "UTM Zone 30, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 30S
	{    82342L, 32731L, "UTM84-31S",                  "UTM Zone 31, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 31S
	{    82345L, 32732L, "UTM84-32S",                  "UTM Zone 32, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 32S
	{    82348L, 32733L, "UTM84-33S",                  "UTM Zone 33, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 33S
	{    82351L, 32734L, "UTM84-34S",                  "UTM Zone 34, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 34S
	{    82354L, 32735L, "UTM84-35S",                  "UTM Zone 35, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 35S
	{    82357L, 32736L, "UTM84-36S",                  "UTM Zone 36, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 36S
	{    82360L, 32737L, "UTM84-37S",                  "UTM Zone 37, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 37S
	{    82363L, 32738L, "UTM84-38S",                  "UTM Zone 38, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 38S
	{    82365L, 32739L, "UTM84-39S",                  "UTM Zone 39, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 39S
	{    82371L, 32740L, "UTM84-40S",                  "UTM Zone 40, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 40S
	{    82373L, 32741L, "UTM84-41S",                  "UTM Zone 41, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 41S
	{    82375L, 32742L, "UTM84-42S",                  "UTM Zone 42, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 42S
	{    82377L, 32743L, "UTM84-43S",                  "UTM Zone 43, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 43S
	{    82379L, 32744L, "UTM84-44S",                  "UTM Zone 44, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 44S
	{    82381L, 32745L, "UTM84-45S",                  "UTM Zone 45, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 45S
	{    82383L, 32746L, "UTM84-46S",                  "UTM Zone 46, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 46S
	{    82385L, 32747L, "UTM84-47S",                  "UTM Zone 47, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 47S
	{    82387L, 32748L, "UTM84-48S",                  "UTM Zone 48, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 48S
	{    82389L, 32749L, "UTM84-49S",                  "UTM Zone 49, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 49S
	{    82395L, 32750L, "UTM84-50S",                  "UTM Zone 50, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 50S
	{    82398L, 32751L, "UTM84-51S",                  "UTM Zone 51, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 51S
	{    82401L, 32752L, "UTM84-52S",                  "UTM Zone 52, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 52S
	{    82404L, 32753L, "UTM84-53S",                  "UTM Zone 53, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 53S
	{    82407L, 32754L, "UTM84-54S",                  "UTM Zone 54, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 54S
	{    82410L, 32755L, "UTM84-55S",                  "UTM Zone 55, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 55S
	{    82413L, 32756L, "UTM84-56S",                  "UTM Zone 56, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 56S
	{    82416L, 32757L, "UTM84-57S",                  "UTM Zone 57, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 57S
	{    82418L, 32758L, "UTM84-58S",                  "UTM Zone 58, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 58S
	{    82420L, 32759L, "UTM84-59S",                  "UTM Zone 59, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 59S
	{    82426L, 32760L, "UTM84-60S",                  "UTM Zone 60, Southern Hemisphere (WGS 84)",                        5 },	// WGS 84 / UTM zone 60S
	{    90114L, 32761L, "WGS84.UPSSouth",             "UPS South Zone",                                                   4 },	// WGS 84 / UPS South
	{    82048L,     0L, "HI-1",                       "Hawaii 5101, Zone 1 (1927)",                                       5 },	// 
	{    82051L,     0L, "HI-2",                       "Hawaii 5102, Zone 2 (1927)",                                       5 },	// 
	{    82054L,     0L, "HI-3",                       "Hawaii 5103, Zone 3 (1927)",                                       5 },	// 
	{    82057L,     0L, "HI-4",                       "Hawaii 5104, Zone 4 (1927)",                                       5 },	// 
	{    82060L,     0L, "HI-5",                       "Hawaii 5105, Zone 5 (1927)",                                       5 },	// 
	{    41127L,     0L, "PR-1",                       "Puerto Rico and Virgin Islands 5201, Zone 1 (1927)",               5 },	// 
	{    41128L,     0L, "PR-2",                       "Puerto Rico, St. Croix, Virgin Islands 5202, Zone 2 (1927)",       5 },	// 
	{        1L,     0L, "",                           "Sinusoidal (WGS 84)",                                              4 },	// 
	{    32775L,     0L, "",                           "Equal-Area Projection (United States)",                            4 },	// 
	{   106496L,     0L, "",                           "Azimuthal Equidistant (North Pole)",                               4 },	// 
	{    98304L,     0L, "",                           "Lambert Azimuthal Equal-Area (North Pole)",                        4 },	// 
	{   106497L,     0L, "",                           "Azimuthal Equidistant (South Pole)",                               4 },	// 
	{    98305L,     0L, "",                           "Lambert Azimuthal Equal-Area (South Pole)",                        4 },	// 
	{    81945L,     0L, "AL83-EF",                    "Alabama 0101, Eastern Zone (1983, US Survey feet)",                5 },	// 
	{    81948L,     0L, "AL83-WF",                    "Alabama 0102, Western Zone (1983, US Survey feet)",                5 },	// 
	{   172033L,     0L, "AK83-1F",                    "Alaska 5001, Zone 1 (1983, US Survey feet)",                       5 },	// 
	{    81951L,     0L, "AK83-2F",                    "Alaska 5002, Zone 2 (1983, US Survey feet)",                       5 },	// 
	{    81954L,     0L, "AK83-3F",                    "Alaska 5003, Zone 3 (1983, US Survey feet)",                       5 },	// 
	{    81957L,     0L, "AK83-4F",                    "Alaska 5004, Zone 4 (1983, US Survey feet)",                       5 },	// 
	{    81960L,     0L, "AK83-5F",                    "Alaska 5005, Zone 5 (1983, US Survey feet)",                       5 },	// 
	{    81963L,     0L, "AK83-6F",                    "Alaska 5006, Zone 6 (1983, US Survey feet)",                       5 },	// 
	{    81966L,     0L, "AK83-7F",                    "Alaska 5007, Zone 7 (1983, US Survey feet)",                       5 },	// 
	{    81969L,     0L, "AK83-8F",                    "Alaska 5008, Zone 8 (1983, US Survey feet)",                       5 },	// 
	{    81972L,     0L, "AK83-9F",                    "Alaska 5009, Zone 9 (1983, US Survey feet)",                       5 },	// 
	{    40961L,     0L, "AK83-10F",                   "Alaska 5010, Zone 10 (1983, US Survey feet)",                      5 },	// 
	{    81976L,     0L, "AZ83-EF",                    "Arizona 0201, Eastern Zone (1983, US Survey feet)",                5 },	// 
	{    81980L,     0L, "AZ83-CF",                    "Arizona 0202, Central Zone (1983, US Survey feet)",                5 },	// 
	{    81984L,     0L, "AZ83-WF",                    "Arizona 0203, Western Zone (1983, US Survey feet)",                5 },	// 
	{    40964L,     0L, "AR83-NF",                    "Arkansas 0301, Northern Zone (1983, US Survey feet)",              5 },	// 
	{    40967L,     0L, "AR83-SF",                    "Arkansas 0302, Southern Zone (1983, US Survey feet)",              5 },	// 
	{    82049L,     0L, "HI83-1F",                    "Hawaii 5101, Zone 1 (1983, US Survey feet)",                       5 },	// 
	{    82052L,     0L, "HI83-2F",                    "Hawaii 5102, Zone 2 (1983, US Survey feet)",                       5 },	// 
	{    82055L,     0L, "HI83-3F",                    "Hawaii 5103, Zone 3 (1983, US Survey feet)",                       5 },	// 
	{    82058L,     0L, "HI83-4F",                    "Hawaii 5104, Zone 4 (1983, US Survey feet)",                       5 },	// 
	{    82061L,     0L, "HI83-5F",                    "Hawaii 5105, Zone 5 (1983, US Survey feet)",                       5 },	// 
	{    82075L,     0L, "IL83-EF",                    "Illinois 1201, Eastern Zone (1983, US Survey feet)",               5 },	// 
	{    82078L,     0L, "IL83-WF",                    "Illinois 1202, Western Zone (1983, US Survey feet)",               5 },	// 
	{    41021L,     0L, "IA83-NF",                    "Iowa 1401, Northern Zone (1983, US Survey feet)",                  5 },	// 
	{    41024L,     0L, "IA83-SF",                    "Iowa 1402, Southern Zone (1983, US Survey feet)",                  5 },	// 
	{    41027L,     0L, "KS83-NF",                    "Kansas 1501, Northern Zone (1983, US Survey feet)",                5 },	// 
	{    41030L,     0L, "KS83-SF",                    "Kansas 1502, Southern Zone (1983, US Survey feet)",                5 },	// 
	{    41039L,     0L, "LA83-NF",                    "Louisiana 1701, Northern Zone (1983, US Survey feet)",             5 },	// 
	{    41042L,     0L, "LA83-SF",                    "Louisiana 1702, Southern Zone (1983, US Survey feet)",             5 },	// 
	{    82113L,     0L, "ME83-EF",                    "Maine 1801, Eastern Zone (1983, US Survey feet)",                  5 },	// 
	{    82111L,     0L, "ME83-WF",                    "Maine 102, Western Zone (1983, US Survey feet)",                   5 },	// 
	{    41057L,     0L, "MI83-NF",                    "Michigan 2111, Northern Zone (1983, US Survey feet)",              5 },	// 
	{    41061L,     0L, "MI83-CF",                    "Michigan 2112, Central Zone (1983, US Survey feet)",               5 },	// 
	{    41065L,     0L, "MI83-SF",                    "Michigan 2113, Southern Zone (1983, US Survey feet)",              5 },	// 
	{    41069L,     0L, "MN83-NF",                    "Minnesota 2201, Northern Zone (1983, US Survey feet)",             5 },	// 
	{    41072L,     0L, "MN83-CF",                    "Minnesota 2202, Central Zone (1983, US Survey feet)",              5 },	// 
	{    41075L,     0L, "MN83-SF",                    "Minnesota 2203, South Zone (1983, US Survey feet)",                5 },	// 
	{    82125L,     0L, "MO83-EF",                    "Missouri 2401, Eastern Zone (1983, US Survey feet)",               5 },	// 
	{    82128L,     0L, "MO83-CF",                    "Missouri 2402, Central Zone (1983, US Survey feet)",               5 },	// 
	{    82131L,     0L, "MO83-WF",                    "Missouri 2403, Western Zone (1983, US Survey feet)",               5 },	// 
	{    41077L,     0L, "MT83F",                      "Montana 2500 (1983, US Survey feet)",                              5 },	// 
	{    41083L,     0L, "NB83F",                      "Nebraska 2600 (1983, US Survey feet)",                             5 },	// 
	{    82142L,     0L, "NV83-EF",                    "Nevada 2701, Eastern Zone (1983, US Survey feet)",                 5 },	// 
	{    82144L,     0L, "NV83-CF",                    "Nevada 2702, Central Zone (1983, US Survey feet)",                 5 },	// 
	{    82147L,     0L, "NV83-WF",                    "Nevada 2703, Western Zone (1983, US Survey feet)",                 5 },	// 
	{    82150L,     0L, "NH83F",                      "New Hampshire 2800 (1983, US Survey feet)",                        5 },	// 
	{    82153L,     0L, "NJ83F",                      "New Jersey 2900 (1983, US Survey feet)",                           5 },	// 
	{    41094L,     0L, "ND83-NF",                    "North Dakota 3301, Northern Zone (1983, US Survey feet)",          5 },	// 
	{    41097L,     0L, "ND83-SF",                    "North Dakota 3302, Southern Zone (1983, US Survey feet)",          5 },	// 
	{    41100L,     0L, "OH83-NF",                    "Ohio 3401, Northern Zone (1983, US Survey feet)",                  5 },	// 
	{    41103L,     0L, "OH83-SF",                    "Ohio 3402, Southern Zone (1983, US Survey feet)",                  5 },	// 
	{    41112L,     0L, "OR83-NF",                    "Oregon 3601, Northern Zone (1983, US Survey feet)",                5 },	// 
	{    41116L,     0L, "OR83-SF",                    "Oregon 3602, Southern Zone (1983, US Survey feet)",                5 },	// 
	{    82200L,     0L, "RI83F",                      "Rhode Island 3800 (1983, US Survey feet)",                         5 },	// 
	{    41129L,     0L, "SC83F",                      "South Carolina 3900 (1983, US Survey feet)",                       5 },	// 
	{    41135L,     0L, "SD83-NF",                    "South Dakota 4001, Northern Zone (1983, US Survey feet)",          5 },	// 
	{    41138L,     0L, "SD83-SF",                    "South Dakota 4002, Southern Zone (1983, US Survey feet)",          5 },	// 
	{    41159L,     0L, "UT83-NF",                    "Utah 4301, Northern Zone (1983, US Survey feet)",                  5 },	// 
	{    41163L,     0L, "UT83-CF",                    "Utah 4302, Central Zone (1983, US Survey feet)",                   5 },	// 
	{    41167L,     0L, "UT83-SF",                    "Utah 4303, Southern Zone (1983, US Survey feet)",                  5 },	// 
	{    82443L,     0L, "VT83F",                      "Vermont 4400 (1983, US Survey feet)",                              5 },	// 
	{    41183L,     0L, "WV83-NF",                    "West Virginia 4701, Northern Zone (1983, US Survey feet)",         5 },	// 
	{    41186L,     0L, "WV83-SF",                    "West Virginia 4702, Southern Zone (1983, US Survey feet)",         5 },	// 
	{    82449L,     0L, "WY83-EF",                    "Wyoming 4901, Eastern Zone (1983, US Survey feet)",                5 },	// 
	{    82452L,     0L, "WY83-ECF",                   "Wyoming 4902, East Central Zone (1983, US Survey feet)",           5 },	// 
	{    82455L,     0L, "WY83-WCF",                   "Wyoming 4903, West Central Zone (1983, US Survey feet)",           5 },	// 
	{    82458L,     0L, "WY83-WF",                    "Wyoming 4904, Western Zone (1983, US Survey feet)",                5 },	// 
	{    41125L,     0L, "PR83F",                      "Puerto Rico and Virgin Islands 5200 (1983, US Survey feet)",       5 },	// 

//ADSK-HW adding here the Mentor 11.13 entries that are still missing from Mentor 11.15's fix from April 2007.
//this is trickier than for the datums below because in Mentor 11.13 we did not have a direct mapping MSI - Oracle
//it was done through the EPSG mapping in a different file: MSI - PSG - Oracle
//I will manually map what can be map thanks to the EPSG file
//The ones with an EPSG value that is 0 will be commented out, as well as the double occurences
	{   8210L,   4149L, "LLCH1903",                    "Longitude / Latitude (CH 1903)",                                    5 },
	{   8250L,   4274L, "Datum73.LL",                  "Longitude / Latitude (Melrica 1973)",                               4 },
	{   8220L,   4314L, "Datum73.LL",                  "Longitude / Latitude (DHDN)",                                       4 },
	{   8281L,   4314L, "Datum73.LL",                  "Longitude / Latitude (Potsdam)",                                    4 },
	{   8289L,   4314L, "Datum73.LL",                  "Longitude / Latitude (Rauenberg)",                                  4 },
	{ 352257L,  21781L, "CH1903.LV03",                 "Swiss National System",                                             5 },
	{ 352256L,  21780L, "CH1903.LV03C",                "Liechtenstein National System",                                     5 },

	{        0L,     0L, "",                           "",                                                                 0 },	// End of table marker
};


struct csOracleDtmMap_
{
	long32_t epsgNbr;
	char msiName [24];
	char oracleName [96];
	unsigned short flags;		/* currently unused */
};
struct csOracleDtmMap_ csOracleDtmMap [] =
{
	{   6124L,  "RT90",                     "RT 90 (Sweden)",                                                                           5 },	// Rikets koordinatsystem 1990
	{   6135L,  "OLDHI",                    "Old Hawaiian",                                                                             5 },	// Old Hawaiian
	{   6139L,  "PRVI",                     "Puerto Rico",                                                                              5 },	// Puerto Rico
	{   6149L,  "CH1903",                   "CH 1903 (Switzerland)",                                                                    4 },	// CH1903
	{   6158L,  "NAPARIMA",                 "Naparima, BWI",                                                                            5 },	// Naparima 1955
	{   6182L,  "OBSRV66",                  "Observatorio 1966",                                                                        5 },	// Azores Occidental Islands 1939
	{   6183L,  "AZORES",                   "Southwest Base",                                                                           5 },	// Azores Central Islands 1948
	{   6184L,  "SAOBRAZ",                  "Sao Braz",                                                                                 5 },	// Azores Oriental Islands 1940
	{   6194L,  "Qornoq27",                 "Qornoq",                                                                                   4 },	// Qornoq 1927
	{   6201L,  "ADINDAN",                  "Adindan",                                                                                  5 },	// Adindan
	{   6202L,  "AGD66",                    "Australian Geodetic 1966",                                                                 5 },	// Australian Geodetic Datum 1966
	{   6203L,  "AGD84",                    "Australian Geodetic 1984",                                                                 5 },	// Australian Geodetic Datum 1984
	{   6204L,  "AINELABD",                 "Ain el Abd 1970",                                                                          5 },	// Ain el Abd 1970
	{   6205L,  "AFGOOYE",                  "Afgooye",                                                                                  5 },	// Afgooye
	{   6209L,  "ARC1950",                  "Arc 1950",                                                                                 5 },	// Arc 1950
	{   6210L,  "ARC1960",                  "Arc 1960",                                                                                 5 },	// Arc 1960
	{   6211L,  "Batavia",                  "Djakarta (Batavia)",                                                                       4 },	// Batavia
	{   6216L,  "BERMUDA",                  "Bermuda 1957",                                                                             5 },	// Bermuda 1957
	{   6218L,  "BOGOTA",                   "Bogota Observatory",                                                                       5 },	// Bogota 1975
	{   6221L,  "CAMPO",                    "Campo Inchauspe",                                                                          5 },	// Campo Inchauspe
	{   6222L,  "CAPE-1",                   "Cape",                                                                                     5 },	// Cape
	{   6223L,  "CARTHAGE",                 "Carthage",                                                                                 5 },	// Carthage
	{   6224L,  "CHAU",                     "Chua Astro",                                                                               5 },	// Chua
	{   6225L,  "CORREGO",                  "Corrego Alegre",                                                                           5 },	// Corrego Alegre
	{   6229L,  "OLD-EGYP",                 "Old Egyptian",                                                                             5 },	// Egypt 1907
	{   6230L,  "ED50",                     "European 1950",                                                                            5 },	// European Datum 1950
	{   6231L,  "Europ87",                  "European 1987",                                                                            4 },	// European Datum 1987
	{   6232L,  "Fahud",                    "Oman",                                                                                     4 },	// Fahud
	{   6233L,  "GNDAJIKA",                 "Gandajika Base",                                                                           7 },	// Gandajika 1970
	{   6236L,  "HuTzuShan",                "Hu-Tzu-Shan",                                                                              4 },	// Hu Tzu Shan
	{   6244L,  "KANDWALA",                 "Kandawala",                                                                                5 },	// Kandawala
	{   6245L,  "KERTAU48",                 "Kertau 1948",                                                                              5 },	// Kertau 1968
	{   6248L,  "PSAD56",                   "Provisional South American",                                                               5 },	// Provisional South American Datum 1956
	{   6251L,  "LIBERIA",                  "Liberia 1964",                                                                             5 },	// Liberia 1964
	{   6253L,  "LUZON",                    "Luzon (Philippines)",                                                                      5 },	// Luzon 1911
	{   6254L,  "HitoXVIII63",              "Hito XVIII 1963",                                                                          4 },	// Hito XVIII 1963
	{   6256L,  "MAHE1971",                 "Mahe 1971",                                                                                5 },	// Mahe 1971
	{   6261L,  "MERCHICH",                 "Merchich",                                                                                 5 },	// Merchich
	{   6262L,  "MASSAWA",                  "Massawa",                                                                                  5 },	// Massawa
	{   6263L,  "MINNA",                    "Minna",                                                                                    5 },	// Minna
	{   6265L,  "ROME1940",                 "Rome 1940",                                                                                5 },	// Monte Mario
	{   6267L,  "NAD27",                    "NAD 27 (Continental US)",                                                                  5 },	// North American Datum 1927
	{   6268L,  "MICHIGAN",                 "NAD 27 (Michigan)",                                                                        5 },	// NAD Michigan
	{   6269L,  "NAD83",                    "NAD 83 (Continental US)",                                                                  5 },	// North American Datum 1983
	{   6270L,  "NHRWN-O",                  "Nahrwan (Masirah Island)",                                                                 5 },	// Nahrwan 1967
	{   6272L,  "NZGD49",                   "Geodetic Datum 1949",                                                                      5 },	// New Zealand Geodetic Datum 1949
	{   6274L,  "Datum73",                  "Melrica 1973 (D73)",                                                                       4 },	// Datum 73
	{   6275L,  "NTF",                      "NTF",                                                                                      5 },	// Nouvelle Triangulation Francaise
	{   6277L,  "OSGB",                     "Ordnance Survey Great Brit",                                                               5 },	// OSGB 1936
	{   6283L,  "GDA94",                    "GDA 94",                                                                                   5 },	// Geocentric Datum of Australia 1994
	{   6284L,  "Pulkovo1942",              "Pulkovo 1942",                                                                             5 },	// Pulkovo 1942
	{   6286L,  "QATAR",                    "Qatar National",                                                                           5 },	// Qatar 1948
	{   6292L,  "SAPPER",                   "Sapper Hill 1943",                                                                         5 },	// Sapper Hill 1943
	{   6293L,  "SCHWARZK",                 "Schwarzeck",                                                                               5 },	// Schwarzeck
	{   6298L,  "TMBLI-B",                  "Timbalai 1948",                                                                            5 },	// Timbalai 1948
	{   6299L,  "TM65",                     "Ireland 1965",                                                                             4 },	// TM65
	{   6301L,  "TOKYO",                    "Tokyo",                                                                                    5 },	// Tokyo
	{   6309L,  "Yacare",                   "Yacare",                                                                                   4 },	// Yacare
	{   6311L,  "ZANDERIJ",                 "Zanderij",                                                                                 5 },	// Zanderij
	{   6314L,  "DHDN",                     "DHDN (Potsdam/Rauenberg)",                                                                 4 },	// Deutsches Hauptdreiecksnetz
	{   6322L,  "WGS72",                    "WGS 72",                                                                                   5 },	// World Geodetic System 1972
	{   6326L,  "WGS84",                    "WGS 84",                                                                                   5 },	// World Geodetic System 1984
	{   6615L,  "MADEIRA",                  "Southeast Base",                                                                           5 },	// Porto Santo 1936
	{   6618L,  "SA1969",                   "South American 1969",                                                                      5 },	// South American Datum 1969
	{   6626L,  "REUNION",                  "Reunion",                                                                                  5 },	// Reunion 1947
	{   6658L,  "HJORSEY",                  "Hjorsey 1955",                                                                             5 },	// Hjorsey 1955
	{   6668L,  "EUROP79",                  "European 1979",                                                                            5 },	// European Datum 1979
	{   6672L,  "CHATHAM",                  "Chatham 1971",                                                                             5 },	// Chatham Islands Datum 1971
	{   6675L,  "GUAM63",                   "Guam 1963",                                                                                5 },	// Guam 1963
	{   6698L,  "KERGUELN",                 "Kerguelen Island",                                                                         5 },	// IGN 1962 Kerguelen
	{   6712L,  "ASCENSN",                  "Ascension Island 1958",                                                                    5 },	// Ascension Island 1958
	{   6714L,  "BELLEVUE",                 "Bellevue (IGN)",                                                                           5 },	// Bellevue
	{   6717L,  "CANAVRL",                  "Cape Canaveral",                                                                           5 },	// Cape Canaveral
	{   6719L,  "EASTER",                   "Easter Island 1967",                                                                       5 },	// Easter Island 1967
	{   6725L,  "JHNSTN",                   "Johnston Island 1961",                                                                     5 },	// Johnston Island 1961
	{   6727L,  "MIDWAY",                   "Midway Astro 1961",                                                                        5 },	// Midway 1961
	{   6728L,  "CANARY",                   "Pico de las Nieves",                                                                       5 },	// Pico de la Nieves
	{   6729L,  "PITCAIRN",                 "Pitcairn Astro 1967",                                                                      5 },	// Pitcairn 1967
	{   6731L,  "VITI",                     "Viti Levu 1916",                                                                           7 },	// Viti Levu 1916
	{   6734L,  "TRISTAN",                  "Tristan Astro 1968",                                                                       5 },	// Tristan 1968
	{   6738L,  "HONGKONG",                 "Hong Kong 1963",                                                                           5 },	// Hong Kong 1963
	{      0L,"ANNA65"                  ,"Anna 1 Astro 1965",               4 },
	{      0L,"ADOS714"                 ,"Astro DOS 71/4",                  4 },
	{      0L,"ASTATN52"                ,"Astronomic Station 1952",         4 },
	{      0L,"SOROL"                   ,"Astro B4 Sorol Atoll",            4 },
	{      0L,"CANTON"                  ,"Canton Astro 1966",               4 },
	{      0L,"DOS1968"                 ,"DOS 1968",                        4 },
	{      0L,"DutchRD"                 ,"Netherlands Bessel",              4 },
	{      0L,"SANTO"                   ,"Santo (DOS)",                     4 },
	{      0L,"GUX1"                    ,"GUX 1 Astro",                     4 },
	{      0L,"INDIAN"                  ,"Indian (Bangladesh, etc.)",       4 },
	{      0L,"INDIANTV"                ,"Indian (Thailand/Vietnam)",       4 },
	{      0L,"ISTS69"                  ,"ISTS 073 Astro 1969",             4 },
	{      0L,"IWOJIMA"                 ,"Astro Beacon E",                  4 },
	{      0L,"L-C5"                    ,"L.C. 5 Astro",                    4 },
	{      0L,"LUZON-MI"                ,"Luzon (Mindanao Island)",         4 },
	{      0L,"MARCO"                   ,"Marco Astro",                     4 },
	{      0L,"NHRWN-S"                 ,"Nahrwan (Saudi Arabia)",          4 },
	{      0L,"NHRWN-U"                 ,"Nahrwan (Un. Arab Emirates)",     4 },
	{      0L,"SINGAPR"                 ,"South Asia",                      4 },
	{      0L,"WAKE"                    ,"Wake-Eniwetok 1960",              4 },
	{   6202L,"AGD66"                   ,"AGD 66 ACT",                      4 },
	{   6202L,"AGD66"                   ,"AGD 66 NT",                       4 },
	{   6202L,"AGD66"                   ,"AGD 66 TAS",                      4 },
	{   6202L,"AGD66"                   ,"AGD 66 VIC NSW",                  4 },
	{   6230L,"EUROP50"                 ,"European 1950",                   4 },
	{   6267L,"NAD27"                   ,"NAD 27 (Alaska)",                 4 },
	{   6267L,"NAD27"                   ,"NAD 27 (Bahamas)",                4 },
	{   6267L,"NAD27"                   ,"NAD 27 (Canada)",                 4 },
	{   6267L,"NAD27"                   ,"NAD 27 (Canal Zone)",             4 },
	{   6267L,"NAD27"                   ,"NAD 27 (Caribbean)",              4 },
	{   6267L,"NAD27"                   ,"NAD 27 (Central America)",        4 },
	{   6267L,"NAD27"                   ,"NAD 27 (Cuba)",                   4 },
	{   6267L,"NAD27"                   ,"NAD 27 (Greenland)",              4 },
	{   6267L,"NAD27"                   ,"NAD 27 (San Salvador)",           4 },
	{   6269L,"NAD83"                   ,"NAD 83",                          4 },
	{   6269L,"NAD83"                   ,"NAD 83 (Alaska)",                 4 },
	{   6269L,"NAD83"                   ,"GRS 80",                          4 },
	{   6275L,"NTF"                     ,"NTF (Greenwich meridian)",        4 },
	{   6275L,"NTF-G-Grid-ClrkIGN"      ,"NTF (Paris meridian)",            4 },
	{   6277L,"OSGB-7P"                 ,"Ordnance Survey Great Brit",      4 },
	{     0L,"",                         "",                                0 },	// End of table marker
};

bool AddOracle9Mappings (const wchar_t* csDataDir,TcsCsvStatus& status)
{
	bool ok (true);
	unsigned recNbr;
	wchar_t* wcPtr;
	wchar_t wcTemp [256];
	wchar_t filePath [260];
	struct csOracleNbrMap_* tblPtr;
	TcsNameMapper nameMapper;

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

	// Get a pointer to the first Oracle9 table entry.
	for (tblPtr = csOracleNbrMap;tblPtr->oracleNbr != 8288;tblPtr++) {}

	// For each Oracle9 entry in the table, we use the MSI name to locate the
	// appropriate record in the table and insert the Oracle9 name and number.

	while (ok && tblPtr->flags != 0)
	{
		mbstowcs (wcTemp,tblPtr->msiName,wcCount (wcTemp));
		if (tblPtr->oracleNbr < 10000)
		{
			// Do the geographic names here.
			// First we need to locate the appropriate record.
			bool locOk = geoMap.Locate (recNbr,L"CsMapName",wcTemp);
			if (locOk)
			{
				ok = geoMap.SetCurrentRecord (recNbr);
				if (ok)
				{
					mbstowcs (wcTemp,tblPtr->oracleName,wcCount (wcTemp));
					std::wstring newValue (wcTemp);
					ok = geoMap.ReplaceField (csMapFldOracle9Name,newValue);
				}
				if (ok)
				{
					swprintf (wcTemp,wcCount (wcTemp),L"%ld",tblPtr->oracleNbr);
					std::wstring newValue1 (wcTemp);
					ok = geoMap.ReplaceField (csMapFldOracle9Nbr,newValue1);
				}
			}
		}
		else
		{
			// Its a projective key name.
			bool locOk = prjMap.Locate (recNbr,L"CsMapName",wcTemp);
			if (locOk)
			{
				ok = prjMap.SetCurrentRecord (recNbr);
				if (ok)
				{
					mbstowcs (wcTemp,tblPtr->oracleName,wcCount (wcTemp));
					std::wstring newValue (wcTemp);
					ok = prjMap.ReplaceField (csMapFldOracle9Name,newValue);
				}
				if (ok)
				{
					swprintf (wcTemp,wcCount (wcTemp),L"%ld",tblPtr->oracleNbr);
					std::wstring newValue1 (wcTemp);
					ok = prjMap.ReplaceField (csMapFldOracle9Nbr,newValue1);
				}
			}
		}
		tblPtr++;
	}

	// Write out the modified tables.
	wcscpy (wcPtr,L"GeographicKeyNameMapO9.csv");	
	std::wofstream oStrm (filePath,std::ios_base::out | std::ios_base::trunc);
	if (oStrm.is_open ())
	{
//		geoMap.WriteToStream (oStrm,true,status);
		oStrm.close ();
	}

	wcscpy (wcPtr,L"ProjectiveKeyNameMapO9.csv");	
	oStrm.open (filePath,std::ios_base::out | std::ios_base::trunc);
	if (oStrm.is_open ())
	{
//		prjMap.WriteToStream (oStrm,true,status);
		oStrm.close ();
	}
	return ok;
}
