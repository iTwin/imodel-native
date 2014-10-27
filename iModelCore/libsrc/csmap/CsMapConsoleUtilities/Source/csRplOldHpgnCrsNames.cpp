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
#include "csNameMapper.hpp"

extern "C" unsigned long KcsNmInvNumber;
extern "C" unsigned long KcsNmMapNoNumber;

// The definition of the table declared next is contained in the
// csOrgTransformations.cpp module.
struct csHarnCrsMapTable_
{
	char oldName [24];
	char newName [24];
	char datumName [24];
};
extern struct csHarnCrsMapTable_  csHarnCrsMapTable [];

bool ReplaceOldHpgnCrsNamesInCsv (const wchar_t* dataDir,const wchar_t* csvFileName,EcsMapTableFields fiedlId,const wchar_t* resultDir);

bool ReplaceOldHpgnCrsNames (const wchar_t* dataDir,const wchar_t* resultDir)
{
	bool ok (false);

    // In case we have more than one .csv file to do, we have a function to
    // deal with one .csv file.
    ok = ReplaceOldHpgnCrsNamesInCsv (dataDir,L"ProjectiveKeyNameMap",csMapFldAdskName,resultDir);  
 //   if (ok)
 //   {
	//	ok = ReplaceOldHpgnCrsNamesInCsv (dataDir,L"ProjectiveKeyNameMap",csMapFldCsMapName,resultDir);  
	//}
    return ok;
}
bool ReplaceOldHpgnCrsNamesInCsv (const wchar_t* dataDir,const wchar_t* csvFileName,EcsMapTableFields fieldId,const wchar_t* resultDir)
{
	bool ok;
	bool dummy;
	bool notEofYet;

	short fldNbr;
	
	unsigned barCount;
	unsigned rplCount;

	char *chrPtr;
	char *strPtr;

	char cTheField [1000];
	char cTheField1 [1000];
	char cTheField2 [1000];
	char cTheField3 [1000];
	char cTheField4 [1000];
	char cTheFieldTmp [1000];
	wchar_t wTheField [1000];
	wchar_t csvPathName [1000];
	
	std::wstring fieldData;

	TcsCsvStatus status;

	wcscpy (csvPathName,dataDir);
	wcscat (csvPathName,L"\\");
	wcscat (csvPathName,csvFileName);
	wcscat (csvPathName,L".csv");
	TcsKeyNameMapFile csvFile (csvPathName,28);

	// Get the field number of the field of interest.
	fldNbr = csvFile.GetFldNbr (L"AdskName",status);

	// The natural inclination is to work through the file, and then the
	// mapping table.  However, there exists the possibility of multiple
	// names in the mapping field.  This is due to the alias feature of the
	// NameMapper.  A CRS definition (for example) can have two different
	// names.  In the NameMapper, the additional names are flagged as
	// being aliases.  Thuhs they get used on input, but will be ignored
	// on output.  In the KeyNameMapFile environment, aliases are appended
	// to the official name, separated from the official name and each
	// other (if mnoore than one alias) by the vertical bar character.
	
	// Thus there can be several names in a s[ecific name field.  To address
	// that, we work through our name mapping table and for each entry scan
	// the entire Key Name Map File.  A bit slower, but then again we do not
	// expect to use this program more than once.  Also, the TcsKeyNameMapFile
	// object will read the whol file into memory, so the perfromance hit is
	// hardly noticeable.
	
	ok = true;				// until we know different
	csHarnCrsMapTable_* tblPtr;
	for (tblPtr = csHarnCrsMapTable;ok && tblPtr->oldName[0] != '\0';tblPtr += 1)
	{
		// For each entry in the table, we search the specified field in the
		// specified TcsKeyNameMapFile object and replace all occurences of
		// the old name with the new name.  As a cautionary measure, we count
		// the replacements for each table entry.  A zero count is interesting,
		// a count more than 1 wouold be very interesting.  We'll just use the
		// debugger to see if these situations arise.
		csvFile.Rewind ();
		notEofYet = true;
		rplCount = 0;
		while (ok && notEofYet)
		{
			// Extract the target field of interest.
			ok = csvFile.GetField (fieldData,fieldId);
			if (ok)
			{
				// Our table consists of 8 bit names, so we choose to work in that environment.
				wcstombs (cTheField,fieldData.c_str (),sizeof (cTheField));
			
				// Nothing to do if the field is empty.
				if (cTheField [0] == '\0')
				{
					notEofYet = csvFile.NextRecord ();
					continue;
				}

				// Life is easy if there are no vertical bars in the field.
				chrPtr = strchr (cTheField,'|');
				if (chrPtr == NULL)
				{
					if (!CS_stricmp (cTheField,tblPtr->oldName))
					{
						CS_stncp (cTheField,tblPtr->newName,sizeof (cTheField));
						mbstowcs (wTheField,cTheField,wcCount (wTheField));
						fieldData = wTheField;
						ok = csvFile.ReplaceField (fieldId,fieldData);
	 					rplCount += 1;
					}
				}
				else
				{
					cTheField1 [0] = cTheField2 [0] = cTheField3 [0]  = cTheField4 [0] = '\0';
					CS_stncp (cTheFieldTmp,cTheField,sizeof (cTheFieldTmp));
					strPtr = cTheFieldTmp;
					chrPtr = strchr (strPtr,'|');
					if (chrPtr != NULL)
					{
						*chrPtr++ = '\0';
						CS_stncp (cTheField1,strPtr,sizeof (cTheField1));
						strPtr = chrPtr;
						chrPtr = strchr (strPtr,'|');
						if (chrPtr != NULL)
						{
							*chrPtr++ = '\0';
							CS_stncp (cTheField2,strPtr,sizeof (cTheField2));
							strPtr = chrPtr;
							chrPtr = strchr (strPtr,'|');
							if (chrPtr != NULL)
							{
								*chrPtr++ = '\0';
								CS_stncp (cTheField3,strPtr,sizeof (cTheField3));
								strPtr = chrPtr;
								chrPtr = strchr (strPtr,'|');
								if (chrPtr == NULL)
								{
									CS_stncp (cTheField4,strPtr,sizeof (cTheField4));
								}
								else
								{
									ok = false;
								}
							}
							else
							{
								CS_stncp (cTheField2,strPtr,sizeof (cTheField2));
							}
						}
						else
						{
							CS_stncp (cTheField2,strPtr,sizeof (cTheField2));
						}
					}
					else
					{
						CS_stncp (cTheField1,strPtr,sizeof (cTheField1));
					}
					barCount = 0;
					if (ok)
					{
						if (cTheField1 [0] != '\0')
						{
							if (!CS_stricmp (cTheField1,tblPtr->oldName))
							{
								CS_stncp (cTheField1,tblPtr->newName,sizeof (cTheField1));
								barCount += 1;
							}
						}
						if (cTheField2 [0] != '\0')
						{
							if (!CS_stricmp (cTheField2,tblPtr->oldName))
							{
								CS_stncp (cTheField2,tblPtr->newName,sizeof (cTheField2));
								barCount += 1;
							}
						}
						if (cTheField3 [0] != '\0')
						{
							if (!CS_stricmp (cTheField3,tblPtr->oldName))
							{
								CS_stncp (cTheField3,tblPtr->newName,sizeof (cTheField3));
								barCount += 1;
							}
						}
						if (cTheField4 [0] != '\0')
						{
							if (!CS_stricmp (cTheField4,tblPtr->oldName))
							{
								CS_stncp (cTheField4,tblPtr->newName,sizeof (cTheField4));
								barCount += 1;
							}
						}
					}
					if (barCount != 0)
					{
						cTheField [0] = '\0';
						if (cTheField1 [0] != '\0')
						{
							if (cTheField [0] != '\0')
							{
								CS_stncat (cTheField,"|",sizeof (cTheField));
							}
							CS_stncat (cTheField,cTheField1,sizeof (cTheField));
						}
						if (cTheField2 [0] != '\0')
						{
							if (cTheField [0] != '\0')
							{
								CS_stncat (cTheField,"|",sizeof (cTheField));
							}
							CS_stncat (cTheField,cTheField2,sizeof (cTheField));
						}
						if (cTheField3 [0] != '\0')
						{
							if (cTheField [0] != '\0')
							{
								CS_stncat (cTheField,"|",sizeof (cTheField));
							}
							CS_stncat (cTheField,cTheField3,sizeof (cTheField));
						}
						if (cTheField4 [0] != '\0')
						{
							if (cTheField [0] != '\0')
							{
								CS_stncat (cTheField,"|",sizeof (cTheField));
							}
							CS_stncat (cTheField,cTheField4,sizeof (cTheField));
						}
						mbstowcs (wTheField,cTheField,wcCount (wTheField));
						fieldData = wTheField;
						ok = csvFile.ReplaceField (fieldId,fieldData);
	 					rplCount += 1;
					}
	 			}
			}
			notEofYet = csvFile.NextRecord ();
		}
		if (rplCount > 1)
		{
			// This could be a situation which requires more research.
			dummy = ok;					// Need some code to hang a breakpoint on.
		}
		if (rplCount == 0)
		{
			// This could be a situation which requires more research.
			dummy = ok;					// Need some code to hang a breakpoint on.
		}
	}

	if (ok && resultDir != 0 && *resultDir != L'\0')
	{
		wcscpy (csvPathName,resultDir);
		wcscat (csvPathName,L"\\");
		wcscat (csvPathName,csvFileName);
		wcscat (csvPathName,L".csv");
		std::wofstream csvStrm (csvPathName,std::ios_base::out | std::ios_base::trunc);
		if (csvStrm.is_open ())
		{
			ok = csvFile.WriteToStream (csvStrm,true,status);
		}
		else
		{
			ok = false;
		}
	}
	return ok;
}
