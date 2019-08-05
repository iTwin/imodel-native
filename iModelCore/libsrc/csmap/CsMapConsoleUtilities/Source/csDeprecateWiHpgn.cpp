/*
 * Copyright (c) 2008, Autodesk, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Autodesk, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Autodesk, Inc. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Autodesk, Inc. OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "csConsoleUtilities.hpp"
#include "cs_NameMapper.hpp"

	extern "C" char cs_DirsepC;
	extern "C" char cs_NameMapperName [];

bool csDeprecateWiHpgn (const wchar_t* trgDictDir,const wchar_t* srcDictDir)
{
	bool ok;
	bool nmMprOk;
	int status;
	size_t idx;
	size_t ascCount = 0;

	const char* valPtr;
	const char* crsName;
	char* ascNamePtr = 0;
	char* trgNamePtr = 0;
	TcsAscDefinition* ascDefPtr;

	std::ifstream iStrm;
	std::ofstream oStrm;
	std::wofstream woStrm;

	char chrBufr [MAXPATH + MAXPATH];
	char ascPathName [1024];
	char trgPathName [1024];

	wchar_t nmMprName [256];
	wchar_t wChrBufr [MAXPATH + MAXPATH];

	TcsDefFile coordsysAsc (dictTypCoordsys);
	TcsCategoryFile categoryFile;
	// Activate the name mapper.  Query operations do not need the
	// pointer, but updating operations do need it.
	TcsNameMap oldNameMap;
	TcsNameMap newNameMap;
	TcsGenericId oldCrsNmMprID;
	TcsGenericId newCrsNmMprID;

	// There are some technical flaws in the following, but it is expected that this
	// function will only be used once after its been debugged.
	wcstombs (chrBufr,srcDictDir,sizeof (chrBufr));
	status = CS_altdr (chrBufr);
	ok = (status == 0);
	if (ok)
	{
		ascNamePtr = CS_stncp (ascPathName,chrBufr,sizeof (ascPathName));
		*ascNamePtr++ = cs_DirsepC;
		*ascNamePtr = '\0';

		wcstombs (chrBufr,trgDictDir,sizeof (chrBufr));
		trgNamePtr = CS_stncp (trgPathName,chrBufr,sizeof (trgPathName));
		*trgNamePtr++ = cs_DirsepC;
		*trgNamePtr = '\0';
		
		CS_stncp (ascNamePtr,"category.asc",sizeof (ascPathName));
		iStrm.open (ascPathName,std::ios_base::in);
		ok = iStrm.is_open ();
	}
	if (ok)
	{
		ok = categoryFile.ReadFromStream (iStrm);
		iStrm.close ();
	}

	if (ok)
	{
		CS_stncp (ascNamePtr,"coordsys.asc",sizeof (ascPathName));
		ok = coordsysAsc.InitializeFromFile (ascPathName);
		if (ok)
		{
			ascCount = coordsysAsc.GetDefinitionCount ();
		}
	}

	// Get the in-memory rendition of the NameMapper
	TcsNameMapper* nmMprPtr = cmGetNameMapperPtr (false);

	// Loop through the coordinate systems in the .asc file.  For each definition,
	// we see if:
	//	* The datum name is HPGN, and
	//	* The projection is TM-WCCS, and
	//	* The goup is "OTHR-US".
	//
	// For each such coordinate system, we deprecate it giving the new
	// description as "OBSOLETE:  use HARN/WI.???? where ??? is replaced
	// with the name of the coordinate system being deprecated.
	// The category file is then updated by removing the deprecated CRS
	// name from the Wisconsin category, and adding it to the Obsolete
	// catgeory using the new descriptive name.
	for (idx = 0;ok && idx < ascCount;idx++)
	{
		crsName = coordsysAsc.GetDefinitionName (idx);
		ascDefPtr = coordsysAsc.GetDefinition (crsName);

		// Is this one to be deprecated???
		valPtr = ascDefPtr->GetValue ("DT_NAME:");
		if (valPtr == NULL)
		{
			// No datum, can't be "HPGN"
			continue;
		}
		if (stricmp (valPtr,"HPGN"))
		{
			continue;
		}
		valPtr = ascDefPtr->GetValue ("PROJ:");
		if (stricmp ((valPtr+1),"M-WCCS"))
		{
			continue;
		}
		valPtr = ascDefPtr->GetValue ("GROUP:");
		if (stricmp (valPtr,"OTHR-US"))
		{
			continue;
		}

		// OK, it is one of the ~221 systems we need to deal with.
		// First, we deprecate the definition.  This requires a new description
		// and a new source.
		char newDescr [256];
		char newSource [256];
		char newName [256];

		sprintf (newName,"HARN/WI.%s",crsName);
		sprintf (newDescr,"OBSOLETE: HPGN deprecated, use %s",newName);
		sprintf (newSource,"Per Wisconsin Department of Transportation");
		coordsysAsc.DeprecateDef (crsName,newDescr,newSource);

		// Depreacte in the category dictionary
		categoryFile.DeprecateCrsDel (crsName,newName,"OBSOLETE");
		
		// Deal with the NameMapper.  Locate the record for this system
		// and mark it as deprecated and replaced by the system whose
		// name is currently in 'newName".  We do this twice.  Once for
		// the CsMap flavor, and once for the AutoDesk flavor.

		// In this specific case, the genericID which is used to deprecate
		// the old system is likely to be the same as that of the system
		// being deprecated.  However, the fact that the deprecated field
		// is not zero will still indicate that the use of the name is
		// deprecated.

		// Get the generic ID of the "new system", i.e. the system which
		// is replacing the system being deprecated.  As of this writing,
		// the system could be of either the CS_MAP or Autodesk flavor.
		// If either one, or both, are there, then the generic ID will
		// be the same.
		mbstowcs (nmMprName,newName,wcCount(nmMprName));
		newCrsNmMprID = nmMprPtr->Locate (csMapProjectedCSysKeyName,
										  csMapFlvrCsMap,
										  nmMprName);
		if (newCrsNmMprID.IsNotKnown ())
		{
			newCrsNmMprID = nmMprPtr->Locate (csMapProjectedCSysKeyName,
											  csMapFlvrAutodesk,
											  nmMprName);
		}

		// Deprecate any reference to the system being deprecated which has
		// a CS-MAP flavor.
		mbstowcs (nmMprName,crsName,wcCount(nmMprName));
		oldCrsNmMprID = nmMprPtr->Locate (csMapProjectedCSysKeyName,
										  csMapFlvrCsMap,
										  nmMprName);
		if (oldCrsNmMprID.IsKnown ())
		{
			nmMprOk = nmMprPtr->ExtractSpecificName (oldNameMap,csMapProjectedCSysKeyName,
																oldCrsNmMprID,
																csMapFlvrCsMap,
																nmMprName);
			if (nmMprOk)
			{
				newNameMap = oldNameMap;
				newNameMap.SetDeprecated (newCrsNmMprID);
				newNameMap.SetComments (L"System remains valid, but use of this name is deprecated.");
				nmMprPtr->Replace (newNameMap,oldNameMap);
			}
		}

		// If there a name mapper entry to the old system with the
		// Autodesk flavor, deprecate it too.
		oldCrsNmMprID = nmMprPtr->Locate (csMapProjectedCSysKeyName,
										  csMapFlvrAutodesk,
										  nmMprName);
		if (oldCrsNmMprID.IsKnown ())
		{
			nmMprOk = nmMprPtr->ExtractSpecificName (oldNameMap,csMapProjectedCSysKeyName,
																oldCrsNmMprID,
																csMapFlvrAutodesk,
																nmMprName);
			if (nmMprOk)
			{
				newNameMap = oldNameMap;
				newNameMap.SetDeprecated (newCrsNmMprID);
				newNameMap.SetComments (L"System remains valid, but use of this name is deprecated.");
				nmMprPtr->Replace (newNameMap,oldNameMap);
			}
		}
	}
	if (ok)
	{
		CS_stncp (trgNamePtr,"coordsys.asc",sizeof (trgPathName));
		oStrm.open (trgPathName,std::ios_base::out | std::ios_base::trunc);
		ok = oStrm.is_open ();
		if (ok)
		{
			coordsysAsc.WriteToStream (oStrm);
			oStrm.close ();
		}
	}
	if (ok)
	{
		CS_stncp (trgNamePtr,"category.asc",sizeof (trgPathName));
		oStrm.open (trgPathName,std::ios_base::out | std::ios_base::trunc);
		ok = oStrm.is_open ();
		if (ok)
		{
			categoryFile.WriteToStream (oStrm);
			oStrm.close ();
		}
	}
	if (ok)
	{
		CS_stncp (trgNamePtr,cs_NameMapperName,sizeof (trgPathName));
		mbstowcs (wChrBufr,trgPathName,wcCount (wChrBufr));
		woStrm.open (wChrBufr,std::ios_base::out | std::ios_base::trunc);
		ok = woStrm.is_open ();
		if (ok)
		{
			nmMprPtr->WriteAsCsv (woStrm,true);
			woStrm.close ();
		}
	}

	// Release the NameMapper memory.
	nmMprPtr = cmGetNameMapperPtr (true);

	return ok;
}

