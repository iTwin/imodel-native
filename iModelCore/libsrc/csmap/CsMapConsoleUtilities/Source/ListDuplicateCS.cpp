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

bool GenerateDuplicateListCS (std::wostream& listStrm);

bool GenerateDuplicateLists (const wchar_t* listPath,const wchar_t* dictDir)
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
		ok = GenerateDuplicateListCS (listStrm);
	}
	//if (ok)
	//{
	//	ok = GenerateDuplicateListDT (listStrm);
	//}
	//if (ok)
	//{
	//	ok = GenerateDuplicateListEL (listStrm);
	//}
	listStrm.close ();
	return ok;
}
bool GenerateDuplicateListCS (std::wostream& listStrm)
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
	wchar_t wcTemp [1024];
	const cs_Csdef_* duplicatesCS [64];
	unsigned long duplicatesEPSG [64];

	typedef std::pair<TcsKeyName,TcsKeyName> TcsKeyNmPair;
	std::vector<TcsKeyNmPair> DUplicates;

	TcsCoordsysFile csFile;
	if (csFile.IsOk ())
	{
		size_t ii, jj, kk, csCount;
		csCount = csFile.GetRecordCount ();

		for (ii = 0;ii < (csCount - 1);ii +=1 )
		{
			dupCount = 0;
			csPtrII = csFile.FetchCoordinateSystem (ii);
			if (csPtrII == 0) break;
			if (!CS_stricmp (csPtrII->group,"LEGACY")) continue;
			if (!CS_stricmp (csPtrII->prj_knm,"LM-WCCS")) continue;
			if (!CS_stricmp (csPtrII->prj_knm,"LM-MNDOT")) continue;
			if (!CS_stricmp (csPtrII->prj_knm,"TM-WCCS")) continue;
			if (!CS_stricmp (csPtrII->prj_knm,"TM-MNDOT")) continue;
			epsgCodeII = csMapNameToIdC (csMapProjGeoCSys,csMapFlvrEpsg,csMapFlvrAutodesk,csPtrII->key_nm);

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
					epsgCodeJJ = csMapNameToIdC (csMapProjGeoCSys,csMapFlvrEpsg,csMapFlvrAutodesk,csPtrJJ->key_nm);
					if (epsgCodeJJ == KcsNmInvNumber) epsgCodeJJ = 0UL;
					duplicatesCS [dupCount] = csPtrJJ;
					duplicatesEPSG [dupCount++] = epsgCodeJJ;
				}
			}

			if (dupCount > 0)
			{
				mbstowcs (wcTemp,csPtrII->key_nm,32);
				listStrm << "CS" << ','
					  << wcTemp << ','
					  << epsgCodeII;
				for (kk = 0;kk < dupCount;kk += 1)
				{
					csPtrJJ = duplicatesCS [kk];
					mbstowcs (wcTemp,csPtrJJ->key_nm,32);
					listStrm << ',';
					listStrm << wcTemp << ','
						  << duplicatesEPSG [kk];
				}
				listStrm << std::endl;
			}
		}
		ok = true;
	}
	return ok;
}
