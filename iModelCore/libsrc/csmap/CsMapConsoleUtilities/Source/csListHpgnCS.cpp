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


// This module was written to generate a list of HPGN systems, a proposed
// new name in the "HARN/??" scheme of things, along with the isolated
// datum name.  This list was not intended to be used as is, only as a base
// which woul be 956% correct, and have the minor changes added separately.

// THUS, this module is of even more questionable long term value than many
// of the other modules in this project.  Nevertheless, we save the code as
// something similar may be beeded in the future and its a shame to throw
// away code the works.

// Turns out that the MInnesota COunty systems are based on NAD83 rather
// than HARN; so all that code related to Minnesota is useless.

#ifdef __SKIP__

// One entry per new datum, and a list of the existing ??HP systems which need
// to be created and reference the new datum.

struct csHarnDatumTable_
{
	char datumCode [8];
	char file1 [8];
	char file2 [8];
	long epsgCode;
	char state1 [8];
	char state2 [8];
	char state3 [8];
	char state4 [8];
	char state5 [8];
	char datumName [64];
	char geogName [64];
	char xfrmName [64];
}
	csHarnDatumTable [] =
{
	{ "AL",  "al" ,    "",  1474L,  "AL",    "",    "",    "",    ""  },
	{ "AR",  "ar" ,    "",  1704L,  "AR",    "",    "",    "",    ""  },
	{ "AZ",  "az" ,    "",  1475L,  "AZ",    "",    "",    "",    ""  },
//	{ "CA",  "ca" ,    "",     0L,  "CA",    "",    "",    "",    ""  },
	{ "CA",  "cn" ,  "cs",  1476L,  "CA",    "",    "",    "",    ""  },	// CN = Califormia North, CS = California South
	{ "CO",  "co" ,    "",  1478L,  "CO",    "",    "",    "",    ""  },
	{ "MT",  "em" ,  "wm",  1481L,  "MT",  "ID",    "",    "",    ""  },
	{ "SA",  "es" ,  "ws",  1579L,  "SA",    "",    "",    "",    ""  },	// EASTERN/WESTERN SAMOA
	{ "TX",  "et" ,  "wt",  1498L,  "TX",    "",    "",    "",    ""  },
	{ "FL",  "fl" ,    "",  1480L,  "FL",    "",    "",    "",    ""  },
	{ "GA",  "ga" ,    "",  1479L,  "GA",    "",    "",    "",    ""  },
	{ "GU",  "gu" ,    "",  1068L,  "GU",    "",    "",    "",    ""  },	// GUAM
	{ "HI",  "hi" ,    "",  1520L,  "HI",    "",    "",    "",    ""  },
	{ "IA",  "ia" ,    "",  1705L,  "IA",    "",    "",    "",    ""  },
	{ "IL",  "il" ,    "",  1553L,  "IL",    "",    "",    "",    ""  },
	{ "IN",  "in" ,    "",  1521L,  "IN",    "",    "",    "",    ""  },
	{ "KS",  "ks" ,    "",  1522L,  "KS",    "",    "",    "",    ""  },
	{ "KY",  "ky" ,    "",  1483L,  "KY",    "",    "",    "",    ""  },
	{ "LA",  "la" ,    "",  1484L,  "LA",    "",    "",    "",    ""  },
	{ "MD",  "md" ,    "",  1485L,  "MD",  "DE",    "",    "",    ""  },
	{ "ME",  "me" ,    "",  1486L,  "ME",    "",    "",    "",    ""  },
	{ "MI",  "mi" ,    "",  1487L,  "MI",    "",    "",    "",    ""  },
	{ "MN",  "mn" ,    "",  1706L,  "MN",    "",    "",    "",    ""  },
	{ "MO",  "mo" ,    "",  1707L,  "MO",    "",    "",    "",    ""  },
	{ "MS",  "ms" ,    "",  1488L,  "MS",    "",    "",    "",    ""  },
	{ "NB",  "nb" ,    "",  1489L,  "NB",    "",    "",    "",    ""  },
	{ "NC",  "nc" ,    "", 15835L,  "NC",    "",    "",    "",    ""  },
	{ "ND",  "nd" ,    "",  1493L,  "ND",    "",    "",    "",    ""  },
	{ "NE",  "ne" ,    "",  1490L,  "NH",  "VT",  "MA",  "CT",  "RI"  },	// NE == New England
	{ "NJ",  "nj" ,    "",  1554L,  "NJ",    "",    "",    "",    ""  },
	{ "NM",  "nm" ,    "",  1491L,  "NM",    "",    "",    "",    ""  },
	{ "NV",  "nv" ,    "",  1523L,  "NV",    "",    "",    "",    ""  },
	{ "NY",  "ny" ,    "",  1492L,  "NY",    "",    "",    "",    ""  },
	{ "OH",  "oh" ,    "",  1524L,  "OH",    "",    "",    "",    ""  },
	{ "OK",  "ok" ,    "",  1494L,  "OK",    "",    "",    "",    ""  },
	{ "PA",  "pa" ,    "", 15838L,  "PA",    "",    "",    "",    ""  },
	{ "PV",  "pv" ,    "",  1495L,  "PV",    "",    "",    "",    ""  },
	{ "SC",  "sc" ,    "", 15836L,  "SC",    "",    "",    "",    ""  },
	{ "SD",  "sd" ,    "",  1496L,  "SD",    "",    "",    "",    ""  },
	{ "TN",  "tn" ,    "",  1497L,  "TN",    "",    "",    "",    ""  },
	{ "UT",  "ut" ,    "",  1525L,  "UT",    "",    "",    "",    ""  },
	{ "VA",  "va" ,    "",  1500L,  "VA",    "",    "",    "",    ""  },
	{ "WI",  "wi" ,    "",  1502L,  "WI",    "",    "",    "",    ""  },
	{ "WO",  "wo" ,    "",  1501L,  "WA",  "OR",    "",    "",    ""  },
	{ "WV",  "wv" ,    "",  1526L,  "WV",    "",    "",    "",    ""  },
	{ "WY",  "wy" ,    "",  1503L,  "WY",    "",    "",    "",    ""  },
	{   "",    "" ,    "",     0L,    "",    "",    "",    "",    ""  },
};

csHarnDatumTable_* csHpgnLocateState (const char* stateCode)
{
	csHarnDatumTable_* hpgnTblPtr;
	
	for (hpgnTblPtr = csHarnDatumTable;hpgnTblPtr->datumCode [0] != '\0';hpgnTblPtr += 1)
	{
		if (hpgnTblPtr->state1 [0] != '\0' && !CS_stricmp (hpgnTblPtr->state1,stateCode)) break;
		if (hpgnTblPtr->state2 [0] != '\0' && !CS_stricmp (hpgnTblPtr->state2,stateCode)) break;
		if (hpgnTblPtr->state3 [0] != '\0' && !CS_stricmp (hpgnTblPtr->state3,stateCode)) break;
		if (hpgnTblPtr->state4 [0] != '\0' && !CS_stricmp (hpgnTblPtr->state4,stateCode)) break;
		if (hpgnTblPtr->state5 [0] != '\0' && !CS_stricmp (hpgnTblPtr->state5,stateCode)) break;
	}
	if (hpgnTblPtr->datumCode [0] == '\0')
	{
		hpgnTblPtr = 0;
	}
	return hpgnTblPtr;
}
bool csListHpgnCS (std::wostream& tableStrm);

bool csGenerateHpgnTable (const wchar_t* tablePath,const wchar_t* dictDir)
{
	bool ok (false);
	int st;

	wchar_t filePath [260];

	if (wcslen (tablePath) > (wcCount (filePath) - 40))
	{
		return ok;
	}
	wcscpy (filePath,tablePath);
	wcscat (filePath,L"\\HpgnNameTable.cpp");
	std::wofstream tableStrm (filePath,std::ios_base::out | std::ios_base::trunc);
	ok = tableStrm.is_open ();
	if (ok)
	{
		char dictDirC [1024];
		wcstombs (dictDirC,dictDir,sizeof (dictDirC));
		st = CS_altdr (dictDirC);
		ok = (st == 0);
	}
	if (ok)
	{
		ok = csListHpgnCS (tableStrm);
	}
	tableStrm.close ();
	return ok;
}

bool csListHpgnCS (std::wostream& tableStrm)
{
	bool ok (false);
	char *cp1;
	char *cp2;
	const char* kCp;
	const char* kNamePtr;
	const cs_Csdef_* csPtr;
	csHarnDatumTable_* hpgnTblPtr;

	size_t nameLen;

	char stateCode [12];
	char hpCode [12];
	char oldCsName [64];	
	char newCsName [64];
	char datumName [64];
	char field1 [64];
	char field2 [64];
	char field3 [64];

	TcsCoordsysFile csFile;
	ok = csFile.IsOk ();
	if (ok)
	{
		size_t ii, csCount;
		csCount = csFile.GetRecordCount ();

		for (ii = 0;ii < (csCount - 1);ii += 1)
		{
			CS_stncp (datumName,"HPGN/??",sizeof (datumName));
			csPtr = csFile.FetchCoordinateSystem (ii);
			if (csPtr == 0) break;
			if (CS_stricmp (csPtr->dat_knm,"HPGN"))
			{
				continue;
			}
			if (!CS_stricmp (csPtr->group,"LEGACY"))
			{
				continue;
			}

			// Separate the UTM zones, this is fairly simple.
			newCsName [0] = '\0';
			kNamePtr = csPtr->key_nm;
			nameLen = strlen (kNamePtr);
			if (!CS_strnicmp (kNamePtr,"UTM",3))
			{
				CS_stncp (oldCsName,csPtr->key_nm,sizeof (oldCsName));
				cp1 = (char*)CS_stristr (oldCsName,"HP");
				if (cp1 != 0)
				{
					*cp1 = '\0';
					cp2 = cp1 + 2;
					sprintf (newCsName,"HARN/10.%s%s",oldCsName,cp2);
				}
				else
				{
					sprintf (newCsName,"HARN/??.%s",oldCsName);
				}
				CS_stncp (datumName,"HARN/10",sizeof (datumName));
			}
			// If the names are long enough to justify, see if this is one of the
			// county coordinate systems in Wisconsin or Minnesota.
			else if (nameLen >= 6)
			{
				kCp = kNamePtr + nameLen - 4;
				if (!CS_stricmp (kCp,"WI-M") ||
					!CS_stricmp (kCp,"WI-F") ||
					!CS_stricmp ((kCp-1),"WI-IF")
				   )
				{
					sprintf (newCsName,"HARN/WI.%s",csPtr->key_nm);
					CS_stncp (datumName,"HARN/WI",sizeof (datumName));
				}	
				else if (!CS_stricmp (kCp,"MN-M") ||
						 !CS_stricmp (kCp,"MN-F") ||
						 !CS_stricmp ((kCp-1),"MN-IF")
				        )
				{
					sprintf (newCsName,"HARN/MN.%s",csPtr->key_nm);
					CS_stncp (datumName,"HARN/MN",sizeof (datumName));
				}
				else
				{
					stateCode [0] = *kNamePtr;
					stateCode [1] = *(kNamePtr + 1);
					stateCode [2] = '\0';

					hpCode [0] = *(kNamePtr + 2);
					hpCode [1] = *(kNamePtr + 3);
					hpCode [2] = '\0';
					if (!CS_stricmp (hpCode,"HP"))
					{
						cp1 = (char*)kNamePtr + 4;
						hpgnTblPtr = csHpgnLocateState (stateCode);
						if (hpgnTblPtr != 0)
						{
							if (*cp1 == '\0')
							{
								sprintf (newCsName,"HARN/%s.%s",hpgnTblPtr->datumCode,stateCode);
							}
							else
							{
								sprintf (newCsName,"HARN/%s.%s%s",hpgnTblPtr->datumCode,stateCode,cp1);
							}
							sprintf (datumName,"HARN/%s",hpgnTblPtr->datumCode);
						}
						else
						{
							sprintf (newCsName,"HARN/??.%s",kNamePtr);
						}
					}
					else
					{
						sprintf (newCsName,"HARN/??.%s",kNamePtr);
					}
				}
			}
			else
			{
				stateCode [0] = *kNamePtr;
				stateCode [1] = *(kNamePtr + 1);
				stateCode [2] = '\0';

				hpCode [0] = *(kNamePtr + 2);
				hpCode [1] = *(kNamePtr + 3);
				hpCode [2] = '\0';

				cp1 = (char*)kNamePtr + 4;
				hpgnTblPtr = csHpgnLocateState (stateCode);
				if (!CS_stricmp (hpCode,"HP") && hpgnTblPtr != 0)
				{
					if (*cp1 == '\0')
					{
						sprintf (newCsName,"HARN/%s.%s",hpgnTblPtr->datumCode,stateCode);
					}
					else
					{
						sprintf (newCsName,"HARN/%s.%s%s",hpgnTblPtr->datumCode,stateCode,cp1);
					}
					sprintf (datumName,"HARN/%s",hpgnTblPtr->datumCode);
				}
				else
				{
					sprintf (newCsName,"HARN/??.%s",csPtr->key_nm);
				}
			}

			sprintf (field1,"\"%s\"",csPtr->key_nm);
			sprintf (field2,"\"%s\"",newCsName);
			sprintf (field3,"\"%s\"",datumName);
			tableStrm	<< L"\t{  "
						<< std::setiosflags (std::ios::right)
						<< std::setw(26)
						<< field1
						<< L",   "
						<< std::setiosflags (std::ios::left)
						<< std::setw(28)
						<< field2
						<< L", "
						<< std::setw(10)
						<< field3
						<< L"  },";
			nameLen = strlen (newCsName);
			if (nameLen > 23)
			{
				tableStrm	<< L"\t\t// " << nameLen;
			}
			tableStrm << std::endl;
		}
		ok = tableStrm.good ();
	}
	return ok;
}

#endif