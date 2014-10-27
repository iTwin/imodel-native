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

struct Tcs7ParmFlipNames
{
	char                   dtName1986 [24];
	char                   dtName1968 [24];
	char                   newDtName  [24];
	csSevenParameterAction action;
	char                   updtDtName [24];
	unsigned               epsgOprCode;
	unsigned               epsgVariant;
}
Kcs7ParmFlipNames [] =
{
//    Existing              Name prior to          Proposed new			 Assigned			  Fetch Parameter    EPSG       EPSG
//    Name (1986)           EPSG synch (1968)      Name                  Action				  Values this def    Op Code    Variant
	{"Kartasto66a",        "Kartasto66",          "Kartasto66b",         cs7pActnRevert,     ""                                    },
	{"RT90-3/7P",          "",                    "RT90-3/7Pa",          cs7pActnFlipSigns,  "RT-90-7P",         1896,      2      },
	{"TETE/a",             "TETE",                "TETE/b",              cs7pActnRevert,     "",                    0,      0      },
	{"Estonia92a",         "Estonia92",           "Estonia92b",          cs7pActnRevert,     "",                    0,      0      },
	{"PDOSurvey93a",       "PDOSurvey93",         "PDOSurvey93b",        cs7pActnRevert,     "",                    0,      0      },
	{"CzechJTSK/5",        "CzechJTSK",           "CzechJTSK/5b",        cs7pActnFlipSigns,  "",                    0,      0      },
	{"Pulkovo42/83a",      "Pulkovo42/83",        "Pulkovo42/83b",       cs7pActnRevert,     "",                    0,      0      },
	{"Pulkovo42/58a",      "Pulkovo42/58",        "Pulkovo42/58b",       cs7pActnRevert,     "",                    0,      0      },
	{"Luxembourg30a",      "Luxembourg30",        "Luxembourg30b",       cs7pActnRevert,     "",                    0,      0      },
	{"OSNI52",             "OSNI52",              "OSNI52/b",            cs7pActnFlipSigns,  "",                    0,      0      },
	{"Scoresbysund52a",    "Scoresbysund52",      "Scoresbysund52b",     cs7pActnRevert,     "",                    0,      0      },
	{"Ammassalik58a",      "Ammassalik58",        "Ammassalik58b",       cs7pActnRevert,     "",                    0,      0      },
	{"Lisbon37a",          "Lisbon37",            "Lisbon37/b",          cs7pActnFlipSigns,  "",                 1988,      4      },
	{"Europ87",            "Europ87",             "Europ87/a",           cs7pActnFlipSigns,  "",                    0,      0      },
	{"HD72/7P",            "",                    "HD72/7Pa",            cs7pActnFlipSigns,  "",                 1448,      3      },
	{"HD72-7P-CORR",       "",                    "",                    cs7pActnRestore,    "",                 1829,      1      },
	{"HitoXVIII63a",       "HitoXVIII63",         "HitoXVIII63b",        cs7pActnRevert,     "",                    0,      0      },
	{"NGO48a",             "NGO48",               "NGO48b",              cs7pActnRevert,     "",                    0,      0      },
	{"Datum73-Mod/a",      "Datum73-Mod",         "Datum73-Mod/b",       cs7pActnFlipSigns,  "",                    0,      0      },
	{"Palestine23",        "Palestine23",         "Palestine23a",        cs7pActnFlipSigns,  "",                    0,      0      },
	{"TM1965/a",           "TM1965",              "TM1965/b",            cs7pActnFlipSigns,  "",                    0,      0      },
	{"MGI-AT",             "MGI-AT",              "MGI-AT/a",            cs7pActnFlipSigns,  "",                    0,      0      },
	{"Belge72a",           "Belge72",             "Belge72/b",           cs7pActnFlipSigns,  "",                    0,      0      },
//	{"DHDN/2",             "DHDN",                "DHDN/3",              cs7pActnFlipSigns,  "",                    0,      0      }, // Already done, submission 2269 
	{"WGS72-TBE/a",        "WGS72-TBE",           "WGS72-TBE/b",         cs7pActnRevert,     "",                    0,      0      },
	{"HongKong80a",        "HongKong80",          "HongKong80b",         cs7pActnRevert,     "",                    0,      0      },
	{"QatarNtl95a",        "QatarNtl95",          "QatarNtl95b",         cs7pActnRevert,     "",                    0,      0      },
	{"IGN53/Mare",         "IGN53/Mare",          "IGN53/Mare.a",        cs7pActnFlipSigns,  "",                 1928,      1      },
	{"ST71Belep/a",        "ST71Belep",           "ST71Belep/b",         cs7pActnRevert,     "",                    0,      0      },
	{"Helle1954",          "Helle1954",           "Helle1954a",          cs7pActnFlipSigns,  "",                    0,      0      },
	{"Chatham1979",        "Chatham1979",         "Chatham1979a",        cs7pActnFlipSigns,  "",                    0,      0      },
	{"PRS92/02",           "PRS92/02",            "PRS92/03",            cs7pActnFlipSigns,  "",                    0,      0      },
	{"RGP-Francaise",      "RGP-Francaise",       "RGP-Francaise/a",     cs7pActnFlipSigns,  "",                    0,      0      },
	{"FatuIva/72",         "FatuIva/72",          "FatuIva/72a",         cs7pActnFlipSigns,  "",                    0,      0      },
	{"IGN63/Hiva Oa",      "IGN63/Hiva O",        "IGN63/HivaOb",        cs7pActnFlipSigns,  "",                    0,      0      },
	{"Moorea87",           "Moorea87",            "Moorea87a",           cs7pActnFlipSigns,  "",                    0,      0      },
	{"Fiji1986",           "Fiji1986",            "Fiji1986a",           cs7pActnFlipSigns,  "",                    0,      0      },
	{"ParametropZemp1990", "ParametropZemp1990",  "ParametropZemp1990a", cs7pActnFlipSigns,  "",                    0,      0      },
	{"MGI-AT/F",           "MGI-AT/F",            "MGI-AT/Fa",           cs7pActnFlipSigns,  "",                    0,      0      },
//	{"CH1903",             "CH1903",              "CH1903a",             cs7pActnFlipSigns,  "",                    0,      0      },	// Already done, saubmission 2270
	{"",                   "",                    "",                    cs7pActnError,      "",                    0,      0      }
};

bool SevenParameterFlipList (std::wostream& listStrm,const wchar_t* dictDir)
{
	bool ok;
	
	int st;

	unsigned index1;
	unsigned index2;
	unsigned index3;

	const char* crsNamePtr;
	const char* dtmNamePtr;
	
	char newCrsName [128];
	char newDtmName [128];

	TcsDefLine* defLinePtr;
	TcsAscDefinition* ascDefPtr;

	char dictDirC [1024];
	char lineBufr [1024];

	wcstombs (dictDirC,dictDir,sizeof (dictDirC));
	st = CS_altdr (dictDirC);
	if (st != 0)
	{
		std::wcout << L"Dictionary directory specification is in error." << std::endl;
		return false;
	}

	// Open up the Coordinate System ASCII definition file.
	strcat (dictDirC,"\\coordsys.asc");
	TcsDefFile coordsysAsc (dictTypCoordsys,dictDirC);

	ok = true;
	for (index1 = 0;Kcs7ParmFlipNames[index1].dtName1986 [0] != '\0';index1 += 1)
	{
		// Here once for each datum which needs adjusting.
		index2 = 0;
		size_t defCount = coordsysAsc.GetDefinitionCount ();
		for (index3 = 0;ok && index3 < defCount;index3 += 1)
		{
			defLinePtr = 0;
			ascDefPtr = &(*coordsysAsc [index3]);
			ok = (ascDefPtr != 0);
			if (ok)
			{
				defLinePtr = ascDefPtr->GetLine ("DT_NAME:");
			}
			if (defLinePtr != 0)
			{
				// Here once for each coordinate system which references a datum.
				//Should have done// skip any datum which has alrady been deprecated, i.e. GROUP == "LEGACY" 
				char *namePtr = defLinePtr->GetValue ();			// points to referenced datum name
				if (!CS_stricmp (namePtr,Kcs7ParmFlipNames[index1].dtName1986))
				{
					// We have a match, add the name of this CRS to the list.

					crsNamePtr = ascDefPtr->GetDefinitionName ();	
					dtmNamePtr = Kcs7ParmFlipNames[index1].dtName1986;
					
					CS_stncp (newCrsName,crsNamePtr,sizeof (newCrsName));
					CS_stncp (newDtmName,Kcs7ParmFlipNames[index1].newDtName,sizeof (newDtmName));

					unsigned actionCode = static_cast<unsigned>(Kcs7ParmFlipNames[index1].action);
					unsigned epsgOprCode = Kcs7ParmFlipNames[index1].epsgOprCode;
					unsigned epsgVariant = Kcs7ParmFlipNames[index1].epsgVariant;

					if (!CS_strnicmp (newCrsName,dtmNamePtr,strlen (dtmNamePtr)))
					{
						// The CRS definition name is prefixed with the old datumn name.
						// Therefore, we get a perfectly good new CRS name by replacing
						// the old datum name prefix with the new datum name.  Note, the
						// CS_strrpl function is case INsenitive during the locate [hase.
						CS_strrpl (newCrsName,sizeof (newCrsName),dtmNamePtr,newDtmName);
					}
					else
					{
						// We play a simple game to give us a new CRS name.  Probably will
						// need a manual edit.
						char *cp = newCrsName + strlen (newCrsName) - 1;
						if (*(cp - 1) == '/' && *cp == 'a') *cp = 'b';
						else if (*(cp - 1) == '/' && *cp == 'b') *cp = 'c';
						else if (*(cp - 1) == '.' && *cp == 'a') *cp = 'b';
						else if (*(cp - 1) == '.' && *cp == 'b') *cp = 'c';
						else
						{
							CS_stncat (newCrsName,"/a",sizeof (newCrsName));
						}
					}
					
					// OK, we have all the informatyion we need.  Generate a line of C++
					// code and write it to the output stream.  We use sprintf as a way
					// to have all elements properly padded for a nice looking table in
					// C++.
					unsigned newCrsNameLen;
					char field1 [64];
					char field2 [64];
					char field3 [64];
					char field4 [64];
					char field5 [64];
					char field6 [64];
					char field7 [64];
					char field8 [64];
					char field9 [64];
					sprintf (field1,"%3d,",index2);
					sprintf (field2,"%2d,",actionCode);
					sprintf (field3,"\"%s\",",dtmNamePtr);
					sprintf (field4,"\"%s\",",crsNamePtr);
					sprintf (field5,"\"%s\",",newDtmName);
					sprintf (field6,"\"%s\",",newCrsName);
					sprintf (field7,"%4d,",epsgOprCode);
					sprintf (field8,"%2d",epsgVariant);
					sprintf (field9,"// ok");
					if (strlen (newCrsName) > 23)
					{
						sprintf (field9,"// %d",strlen (newCrsName)); 
					}
					sprintf (lineBufr,"\t{  %6s %6s %-28s %-28s %-28s %-28s %6s %3s   },  %s",field1,
																							  field2,
																							  field3,
																							  field4,
																							  field5,
																							  field6,
																							  field7,
																							  field8,
																							  field9);
					listStrm << lineBufr << std::endl;
					index2 += 1;
				}
			}
		}
	}
	return ok;
}
