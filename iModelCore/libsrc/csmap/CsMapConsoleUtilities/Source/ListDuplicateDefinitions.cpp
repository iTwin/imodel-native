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

bool ListDuplicateCS (std::wostream& listStrm);

bool ListDuplicateDefinitions (const wchar_t* listPath,const wchar_t* dictDir)
{
	bool ok (false);
	int st;

	std::wofstream listStrm (listPath,std::ios_base::out | std::ios_base::trunc);
	ok = listStrm.is_open ();
	if (ok)
	{
		char dictDirC [1024];
		wcstombs (dictDirC,dictDir,sizeof (dictDirC));
		st = CS_altdr (dictDirC);
		ok = (st == 0);
	}
	if (ok)
	{
		ok = ListDuplicateCS (listStrm);
	}
	//if (ok)
	//{
	//	ok = ListDuplicateDT (listStrm);
	//}
	//if (ok)
	//{
	//	ok = ListDuplicateEL (listStrm);
	//}
	listStrm.close ();
	return ok;
}
bool ListDuplicateCS (std::wostream& listStrm)
{
	bool ok (false);
	int st;
	unsigned dupCount;
	unsigned long epsgCodeII;
	unsigned long epsgCodeJJ;
	const cs_Csdef_ *csPtrII;
	const cs_Csdef_ *csPtrJJ;
	double qValue;
	char message [1024];

	std::vector<TcsKeyName> duplicateNameVector;
	std::vector<TcsKeyNameList> duplicateListVector;
	std::vector<TcsKeyNameList>::iterator itrDuplicateListVctr;

	TcsCoordsysFile csFile;
	if (csFile.IsOk ())
	{
		size_t ii, jj, csCount;
		csCount = csFile.GetRecordCount ();
		duplicateNameVector.clear ();
		duplicateNameVector.reserve (1024);

		for (ii = 0;ii < (csCount - 1);ii +=1 )
		{
			csPtrII = csFile.FetchCoordinateSystem (ii);
			if (csPtrII == 0) break;
			if (!CS_stricmp (csPtrII->group,"LEGACY")) continue;
			if (!CS_stricmp (csPtrII->prj_knm,"LM-WCCS")) continue;
			if (!CS_stricmp (csPtrII->prj_knm,"LM-MNDOT")) continue;
			if (!CS_stricmp (csPtrII->prj_knm,"TM-WCCS")) continue;
			if (!CS_stricmp (csPtrII->prj_knm,"TM-MNDOT")) continue;
			epsgCodeII = csMapNameToIdC (csMapProjGeoCSys,csMapFlvrEpsg,csMapFlvrAutodesk,csPtrII->key_nm);

			TcsKeyName primaryName (csKyNmTypeCRS,csPtrII->key_nm,epsgCodeII);
			primaryName.SetQuality (csPtrII);
			TcsKeyNameList duplicateList (primaryName);
			dupCount = 0;

			// If this coordinate system name already appears in our duplicate
			// name vector, we ignore it.  It's already a secondary name and we
			// don't want it to appear as also as a primary name.
			if (binary_search (duplicateNameVector.begin (),duplicateNameVector.end (),primaryName))
			{
				continue;
			}

			for (jj = (ii + 1);jj < csCount;jj += 1)
			{
				csPtrJJ = csFile.FetchCoordinateSystem (jj);
				if (csPtrJJ == 0) break;
				if (!CS_stricmp (csPtrJJ->group,"LEGACY")) continue;
				if (!CS_stricmp (csPtrJJ->prj_knm,"LM-WCCS")) continue;
				if (!CS_stricmp (csPtrJJ->prj_knm,"LM-MNDOT")) continue;
				if (!CS_stricmp (csPtrJJ->prj_knm,"TM-WCCS")) continue;
				if (!CS_stricmp (csPtrII->prj_knm,"TM-MNDOT")) continue;
				if (CS_stricmp (csPtrII->dat_knm,csPtrJJ->dat_knm)) continue;
				message [0] = '\0';
				st = CS_csDefCmpEx (&qValue,csPtrII,csPtrJJ,message,sizeof (message));
				if ((st == 0) && (qValue < 3.0))
				{
					dupCount += 1;
					epsgCodeJJ = csMapNameToIdC (csMapProjGeoCSys,csMapFlvrEpsg,csMapFlvrAutodesk,csPtrJJ->key_nm);
					if (epsgCodeJJ == KcsNmInvNumber) epsgCodeJJ = 0UL;
					TcsKeyName duplicateName (csKyNmTypeCRS,csPtrJJ->key_nm,epsgCodeJJ);
					duplicateName.SetQuality (csPtrJJ);
					duplicateNameVector.push_back (duplicateName);
					duplicateList += duplicateName;
				}
			}

			if (dupCount > 0)
			{
				duplicateListVector.push_back (duplicateList);
				std::stable_sort (duplicateNameVector.begin (),duplicateNameVector.end ());
			}
		}

		// OK, we have a vector of TcsKeyNameList objects which represent the definitions
		// which are candidates for drepecation as duplicates.  We do some arranging
		// and print the results out in a form ammenable to inserting as a data object
		// initialization in a C++ source code module.
		for (itrDuplicateListVctr  = duplicateListVector.begin ();
			 itrDuplicateListVctr != duplicateListVector.end ();
			 itrDuplicateListVctr++)
		{
			// Here once for each instance of duplicate deifnitions.  First we
			// "Arrange" the names so the most preferred name is the "Primary" name.
			// Then we write them out in a ofrm suitable for the C++ compiler.
			TcsKeyNameList& keyNameList = *itrDuplicateListVctr;
			keyNameList.Arrange ();
			keyNameList.WriteToStream (listStrm);
		}
		ok = true;
	}
	return ok;
}
