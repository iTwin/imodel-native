/*
 * Copyright (c) 2012, Autodesk, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *	* Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 *	* Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
 *	* Neither the name of the Autodesk, Inc. nor the names of its
 *	  contributors may be used to endorse or promote products derived
 *	  from this software without specific prior written permission.
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

#ifdef _WIN32

	#pragma warning( push )
	#pragma warning( disable: 4127 ) //conditional expression is constant

		#include "csIoUtil.hpp"

	#pragma warning ( pop )

	#pragma warning( push )
	#pragma warning( disable: 4702 ) //unreachable code detected

#else

	#define _ASSERT(x) void(0)
	#include "csIoUtil.hpp"

#endif

#define DECL_RETURN(x) x toReturn
#define DO_RETURN(x) toReturn = x; return toReturn
#define Q_RETURN(x,defError,y) \
	try \
	{ DECL_RETURN(x); DO_RETURN(y); } \
	catch(std::bad_alloc&) \
	{ CS_erpt(cs_NO_MEM); } \
	return defError

extern "C"
{

/**********************************************************************
**	flag = CS_setdr (newTargetDir, currentDir);
**
**	char *newTargetDir;			the directory where to locate CSD files in.
**	char *currentDir;			An optional buffer into which the
**								current directory will be stored.
**
**	Return 0 if successfull. -1 otherwise.
**********************************************************************/
int CS_setdr(Const char* newTargetDir, char* currentDir /* = NULL*/)
{
	CS_CHECK_NULL_ARG(newTargetDir, 1);
	if (strlen(newTargetDir) >= MAXPATH)
	{
		CS_erpt(cs_INV_ARG2);
		return -1;
	}

	//store the current value of [cs_Dir] into [currentDir]
	//if the caller wanted us to save it
	if (currentDir)
	{
		int result = CS_getdr(currentDir);
		if (result < 0)
			return result;
	}

	//now set [cs_Dir] to contain the new string and make
	//also [cs_DirP] point to the end of it
	cs_DirP = CS_stcpy(cs_Dir, newTargetDir);
	if (*(cs_DirP - 1) != cs_DirsepC) //make sure, the separator is preceding [cs_DirP]
	{
		*cs_DirP = cs_DirsepC;
		++cs_DirP;
	}

	return 0;
}

/**********************************************************************
**	flag = CS_getdr (currentDir);
**
**	char *currentDir;			the directory where to all CSD files in.
								The provided buffer must have at least
								a size count of CS_CSDIR_SIZE.
** return 0 if successfull; -1 otherwise
**********************************************************************/
int CS_getdr(char* currentDir)
{
	CS_CHECK_NULL_ARG(currentDir, 1);

	if (NULL == cs_DirP)
	{
		CS_stncp (csErrnam,"CS_DirP must not be NULL", MAXPATH);
		CS_erpt(cs_ISER);

		return -1;
	}

	//copy the current dir into the buffer provided by the caller
	//note, that [cs_Dir] is required to point to the end of the current
	//directory name
	char currentChar = *cs_DirP;
	*cs_DirP = '\0';
	CS_stncp(currentDir, cs_Dir, MAXPATH);
	*cs_DirP = currentChar;

	return 0;
}

/*************************************/
/* Coordinate System proxy functions */
/*************************************/
csFILE* CS_csFileOpen(Const char* mode)
{
	Q_RETURN(csFILE*, NULL,CS_FileOpen<cs_CSDEF_MAGIC>(cs_Csname, mode));
}

int CS_csRead (csFILE *strm, struct cs_Csdef_ *cs_def, int* crypt)
{
	Q_RETURN(int, -1, (CS_DefinitionRead<cs_Csdef_>(strm, cs_def, cs_def->key_nm, cs_BSWP_CSDEF, (unsigned char*)&cs_def->fill, crypt)));
}

int CS_csWrite(csFILE *strm, Const struct cs_Csdef_ *cs_def, int crypt)
{
	cs_Csdef_ localDef = *cs_def;
	cs_Csdef_ *pLocalDef = &localDef;
	Q_RETURN(int, -1, (CS_DefinitionWrite<cs_Csdef_>(strm, pLocalDef, cs_BSWP_CSDEF, (unsigned char*)(crypt ? localDef.fill : NULL))));
}

int CS_csDelete(struct cs_Csdef_ *cs_def)
{
	CS_CHECK_NULL_ARG(cs_def, 1);
	Q_RETURN(int, -1, (CS_DefinitionDelete<cs_Csdef_, cs_CSDEF_MAGIC, cs_CS_PROT, cs_CS_UPROT>(cs_def, cs_def->key_nm,
		CS_csopn,
		CS_csdef2,
		NULL,
		CS_csrd,
		NULL,
		CS_cswr,
		CS_cscmp)));
}

int CS_csUpdate(struct cs_Csdef_ *cs_def, int)
{
	CS_CHECK_NULL_ARG(cs_def, 1);
	return CS_DefinitionUpdate<cs_Csdef_, cs_CSDEF_MAGIC, cs_CS_PROT, cs_CS_UPROT, cs_Csname>(cs_def, cs_def->key_nm,
		CS_csopn, NULL, CS_csrd, NULL, CS_cswr, CS_cscmp, NULL);
}

struct cs_Csdef_* CS_csDefinition(Const char *csName, char* pszDirPath, int* isUsrDef)
{
	cs_Csdef_ source;
	memset(&source, 0x0, sizeof(source));
	CS_stncp(source.key_nm, csName, sizeof(source.key_nm));

	Q_RETURN(struct cs_Csdef_*, NULL, (DefinitionGet<cs_Csdef_, cs_CS_NOT_FND>(source, source.key_nm, pszDirPath, CS_csopn, NULL, CS_csrd, CS_cscmp, CS_usrCsDefPtr, isUsrDef)));
}

int CS_csDefinitionAll(struct cs_Csdef_** pAllDefs[])
{
	Q_RETURN(int, -1, (DefinitionGetAll<cs_Csdef_>(pAllDefs, CS_csopn, NULL, CS_csrd)));
}

/*************************/
/* Datum proxy functions */
/*************************/
csFILE* CS_dtFileOpen(Const char* mode)
{
	Q_RETURN(csFILE*, NULL,(CS_FileOpen<cs_DTDEF_MAGIC>(cs_Dtname, mode)));
}

int CS_dtRead (csFILE *strm, struct cs_Dtdef_ *dt_def, int* crypt)
{
	Q_RETURN(int, -1, (CS_DefinitionRead<cs_Dtdef_>(strm, dt_def, dt_def->key_nm, cs_BSWP_DTDEF, (unsigned char*)&dt_def->fill, crypt)));
}

int CS_dtWrite(csFILE *strm, Const struct cs_Dtdef_ *dt_def, int crypt)
{
	cs_Dtdef_ localDef = *dt_def;
	cs_Dtdef_ *pLocalDef = &localDef;
	Q_RETURN(int, -1, (CS_DefinitionWrite<cs_Dtdef_>(strm, pLocalDef, cs_BSWP_DTDEF, (unsigned char*)(crypt ? localDef.fill : NULL))));
}

int CS_dtDelete(struct cs_Dtdef_ *dt_def)
{
	CS_CHECK_NULL_ARG(dt_def, 1);
	Q_RETURN(int, -1, (CS_DefinitionDelete<cs_Dtdef_, cs_DTDEF_MAGIC, cs_DT_PROT, cs_DT_UPROT>(dt_def, dt_def->key_nm,
		CS_dtopn,
		CS_dtdef2,
		NULL,
		CS_dtrd,
		NULL,
		CS_dtwr,
		CS_dtcmp)));
}

int CS_dtUpdate(struct cs_Dtdef_ *dt_def, int)
{
	CS_CHECK_NULL_ARG(dt_def, 1);
	Q_RETURN(int, -1, (CS_DefinitionUpdate<cs_Dtdef_, cs_DTDEF_MAGIC, cs_DT_PROT, cs_DT_UPROT, cs_Dtname>(dt_def, dt_def->key_nm,
		CS_dtopn, NULL, CS_dtrd, NULL, CS_dtwr, CS_dtcmp, NULL)));
}

struct cs_Dtdef_* CS_dtDefinition(Const char *dtName, char* pszDirPath, int* isUsrDef)
{
	cs_Dtdef_ source;
	memset(&source, 0x0, sizeof(source));
	CS_stncp(source.key_nm, dtName, sizeof(source.key_nm));

	Q_RETURN(struct cs_Dtdef_*, NULL, (DefinitionGet<cs_Dtdef_, cs_DT_NOT_FND>(source, source.key_nm, pszDirPath, CS_dtopn, NULL, CS_dtrd, CS_dtcmp, CS_usrDtDefPtr, isUsrDef)));
}

int CS_dtDefinitionAll(struct cs_Dtdef_** pAllDefs[])
{
	Q_RETURN(int, -1, (DefinitionGetAll<cs_Dtdef_>(pAllDefs, CS_dtopn, NULL, CS_dtrd)));
}

/*************************/
/* Ellipsoid proxy functions */
/*************************/
csFILE* CS_elFileOpen(Const char* mode)
{
	Q_RETURN(csFILE*, NULL,(CS_FileOpen<cs_ELDEF_MAGIC>(cs_Elname, mode)));
}

int CS_elRead (csFILE *strm, struct cs_Eldef_ *el_def, int* crypt)
{
	Q_RETURN(int, -1, (CS_DefinitionRead<cs_Eldef_>(strm, el_def, el_def->key_nm, cs_BSWP_ELDEF, (unsigned char*)&el_def->fill, crypt)));
}

int CS_elWrite(csFILE *strm, Const struct cs_Eldef_ *el_def, int crypt)
{
	cs_Eldef_ localDef = *el_def;
	cs_Eldef_ *pLocalDef = &localDef;
	Q_RETURN(int, -1, (CS_DefinitionWrite<cs_Eldef_>(strm, pLocalDef, cs_BSWP_ELDEF, (unsigned char*)(crypt ? localDef.fill : NULL))));
}

int CS_elDelete(struct cs_Eldef_ *el_def)
{
	CS_CHECK_NULL_ARG(el_def, 1);
	Q_RETURN(int, -1, (CS_DefinitionDelete<cs_Eldef_, cs_ELDEF_MAGIC, cs_EL_PROT, cs_EL_UPROT>(el_def, el_def->key_nm, CS_elopn,
		CS_eldef2,
		NULL,
		CS_elrd,
		NULL,
		CS_elwr,
		CS_elcmp)));
}

int CS_elUpdate(struct cs_Eldef_ *el_def, int)
{
	CS_CHECK_NULL_ARG(el_def, 1);
	Q_RETURN(int, -1, (CS_DefinitionUpdate<cs_Eldef_, cs_ELDEF_MAGIC, cs_EL_PROT, cs_EL_UPROT, cs_Elname>(el_def, el_def->key_nm,
		CS_elopn, NULL, CS_elrd, NULL, CS_elwr, CS_elcmp, NULL)));
}

struct cs_Eldef_* CS_elDefinition(Const char *elName, char* pszDirPath, int* isUsrDef)
{
	cs_Eldef_ source;
	memset(&source, 0x0, sizeof(source));
	CS_stncp(source.key_nm, elName, sizeof(source.key_nm));

	Q_RETURN(struct cs_Eldef_*, NULL, (DefinitionGet<cs_Eldef_, cs_EL_NOT_FND>(source, source.key_nm, pszDirPath, CS_elopn, NULL, CS_elrd, CS_elcmp, CS_usrElDefPtr, isUsrDef)));
}

int CS_elDefinitionAll(struct cs_Eldef_** pAllDefs[])
{
	Q_RETURN(int, -1, (DefinitionGetAll<cs_Eldef_>(pAllDefs, CS_elopn, NULL, CS_elrd)));
}

/*********************************/
/* Geodetic Path proxy functions */
/*********************************/
csFILE* CS_gpFileOpen(Const char* mode)
{
	Q_RETURN(csFILE*, NULL,(CS_FileOpen<cs_GPDEF_MAGIC>(cs_Gpname, mode)));
}

int CS_gpRead (csFILE *strm, struct cs_GeodeticPath_ * gp_def)
{
	Q_RETURN(int, -1, (CS_DefinitionRead<cs_GeodeticPath_>(strm, gp_def, gp_def->pathName, cs_BSWP_GPDEF)));
}

int CS_gpWrite(csFILE *strm, Const struct cs_GeodeticPath_ * gp_def)
{
	cs_GeodeticPath_ localDef = *gp_def;
	cs_GeodeticPath_ *pLocalDef = &localDef;
	Q_RETURN(int, -1, (CS_DefinitionWrite<cs_GeodeticPath_>(strm, pLocalDef, cs_BSWP_GPDEF)));
}

int CS_gpDelete(struct cs_GeodeticPath_ * gp_def)
{
	CS_CHECK_NULL_ARG(gp_def, 1);
	Q_RETURN(int, -1, (CS_DefinitionDelete<cs_GeodeticPath_, cs_GPDEF_MAGIC, cs_GP_PROT, cs_GP_UPROT>(gp_def, gp_def->pathName,
		CS_gpopn,
		CS_gpdef2,
		CS_gprd,
		NULL,
		CS_gpwr,
		NULL,
		CS_gpcmp)));
}

int CS_gpUpdate(struct cs_GeodeticPath_ *gp_def)
{
	CS_CHECK_NULL_ARG(gp_def, 1);
	Q_RETURN(int, -1, (CS_DefinitionUpdate<cs_GeodeticPath_, cs_GPDEF_MAGIC, cs_GP_PROT, cs_GP_UPROT, cs_Gpname>(gp_def, gp_def->pathName,
		CS_gpopn, CS_gprd, NULL, CS_gpwr, NULL, CS_gpcmp, NULL)));
}

struct cs_GeodeticPath_* CS_gpDefinition(Const char *gpName, char* pszDirPath)
{
	cs_GeodeticPath_ source;
	memset(&source, 0x0, sizeof(cs_GeodeticPath_));
	CS_stncp(source.pathName, gpName, sizeof(source.pathName));

	Q_RETURN(struct cs_GeodeticPath_*, NULL, (DefinitionGet<cs_GeodeticPath_, cs_GP_NOT_FND>(source, source.pathName, pszDirPath, CS_gpopn, CS_gprd, NULL, CS_gpcmp)));
}

int CS_gpDefinitionAll(struct cs_GeodeticPath_** pAllDefs[])
{
	Q_RETURN(int, -1, (DefinitionGetAll<cs_GeodeticPath_>(pAllDefs, CS_gpopn, CS_gprd, NULL)));
}

/**********************************/
/* Transformation proxy functions */
/**********************************/
csFILE* CS_gxFileOpen(Const char* mode)
{
	Q_RETURN(csFILE*, NULL,(CS_FileOpen<cs_GXDEF_MAGIC>(cs_Gxname, mode)));
}

int CS_gxRead (csFILE *strm, struct cs_GeodeticTransform_ *gx_def)
{
	Q_RETURN(int, -1, (CS_DefinitionRead<cs_GeodeticTransform_>(strm, gx_def, gx_def->xfrmName, NULL, NULL, NULL, CS_gxswp)));
}

int CS_gxWrite(csFILE *strm, Const struct cs_GeodeticTransform_ * gx_def)
{
	cs_GeodeticTransform_ localDef = *gx_def;
	cs_GeodeticTransform_ *pLocalDef = &localDef;
	Q_RETURN(int, -1, (CS_DefinitionWrite<cs_GeodeticTransform_>(strm, pLocalDef, NULL, NULL, CS_gxswp)));
}

int CS_gxDelete(struct cs_GeodeticTransform_ *gx_def)
{
	CS_CHECK_NULL_ARG(gx_def, 1);
	Q_RETURN(int, -1, (CS_DefinitionDelete<cs_GeodeticTransform_, cs_GXDEF_MAGIC, cs_GX_PROT, cs_GX_UPROT>(gx_def, gx_def->xfrmName,
		CS_gxopn,
		CS_gxdef2,
		CS_gxrd,
		NULL,
		CS_gxwr,
		NULL,
		CS_gxcmp)));
}

cs_GeodeticTransform_* CS_gxDefinition(Const char *xfrmName, char* pszDirPath)
{
	cs_GeodeticTransform_ source;
	memset(&source, 0x0, sizeof(cs_GeodeticTransform_));
	CS_stncp(source.xfrmName, xfrmName, sizeof(source.xfrmName));

	Q_RETURN(cs_GeodeticTransform_*, NULL, (DefinitionGet<cs_GeodeticTransform_, cs_GX_NOT_FND>(source, source.xfrmName, pszDirPath, CS_gxopn, CS_gxrd, NULL, CS_gxcmp)));
}

int CS_gxwrtchk(struct cs_GeodeticTransform_ *gx_target, Const struct cs_GeodeticTransform_ *gx_source, int* isProtected);
int CS_gxIsWriteProtected(cs_GeodeticTransform_* pToUpdate, cs_GeodeticTransform_ const* pDictionaryDef, bool, int& gxIsProtectedByDefaultType)
{
	//this is the 'override' function which checks, whether a geodetic transformation definition can
	//be updated, even if it's a 'system' definition
	int isProtected = (DEF_PROTECTED_NONE == gxIsProtectedByDefaultType) ? FALSE : TRUE;
	int error = CS_gxwrtchk(pToUpdate, pDictionaryDef, &isProtected);
	if (error)
		return error;

	if (FALSE == isProtected)
	{
		gxIsProtectedByDefaultType = DEF_PROTECTED_NONE;
	}
	else if (DEF_PROTECTED_NONE == gxIsProtectedByDefaultType)
	{
		gxIsProtectedByDefaultType = DEF_PROTECTED_SYSTEM_DEF;
	}
	//else: leave the value of [gxIsProtectedByDefaultType] unchanged, i.e. either [DEF_PROTECTED_SYSTEM_DEF] or [DEF_PROTECTED_USER_DEF]
	return 0;
}

int CS_gxUpdate(struct cs_GeodeticTransform_* gx_def)
{
	CS_CHECK_NULL_ARG(gx_def, 1);
	Q_RETURN(int, -1, (CS_DefinitionUpdate<cs_GeodeticTransform_, cs_GXDEF_MAGIC, cs_GX_PROT, cs_GX_UPROT, cs_Gxname>(gx_def, gx_def->xfrmName,
		CS_gxopn, CS_gxrd, NULL, CS_gxwr, NULL, CS_gxcmp, CS_gxIsWriteProtected)));
}

int CS_gxGridOvrly(struct cs_GeodeticTransform_ *gx_target, Const struct cs_GeodeticTransform_ *gx_source);
char Const* CS_gxkey(Const struct cs_GeodeticTransform_ *gx_def);
int CS_gxDefinitionAll(struct cs_GeodeticTransform_** pAllDefs[])
{
	int newDefCount = -1;
	typedef std::map<char const*, std::vector<struct cs_GeodeticTransform_ *>, CsMapKeyCompare> GxDuplicatesMap;
	GxDuplicatesMap gxDuplicates;
	int readStatus = DefinitionGetAll<cs_GeodeticTransform_>(pAllDefs, CS_gxopn, CS_gxrd, NULL, CS_gxkey, &gxDuplicates);
	if (readStatus < 0)
		return readStatus;

	std::vector<struct cs_GeodeticTransform_ *> duplicateDefs;
	//The geodetic transformations are a bit specific - in principle, system definitions cannot be updated,
	//but grid-file-based definitions can, because that was always possible by altering the GDC files; see als
	//the implementation in [CS_gxwrtchk]. Still, we add these to the user dictionaries thus 
	//getting the same ID back twice. These we now have to filter out and return the
	//system ones overlaid with the ones from the user dictionaries
	if (gxDuplicates.size() > 0)
	{
		for(GxDuplicatesMap::iterator gxIterator = gxDuplicates.begin();
			gxIterator != gxDuplicates.end(); ++gxIterator)
		{
			std::vector<struct cs_GeodeticTransform_ *>& gxDefVector = gxIterator->second;
			_ASSERT(2 == gxDefVector.size()); //what else could have happened

			if (gxDefVector.size() < 2)
				continue;

			struct cs_GeodeticTransform_ *gx_target = gxDefVector[gxDefVector.size() - 1]; //the definition from the system dictionaries is always read last
			struct cs_GeodeticTransform_ *gx_source = gxDefVector[0]; //the definition from the user dictionary will always be at index 0

			if (CS_gxGridOvrly(gx_target, gx_source))
			{
				//this *MUST* be a grid file based transformation;
				//if it wasn't, i.e. [CS_gxGridOvrly] returned something different than 0,
				//we have duplicated IDs
				CS_erpt(cs_DICT_DUP_IDS);
				goto error;
			}

			duplicateDefs.push_back(gx_target);
			for(size_t i = 0; i < (gxDefVector.size() - 1); /* don't CS_free [gx_target] */ ++i)
			{
				CS_free(gxDefVector[i]);
			}

			gxDefVector.clear();
		}
	}

	newDefCount = (int)duplicateDefs.size();
	if (0 != newDefCount)
	{
		//increase the array we're returning to the caller...
		struct cs_GeodeticTransform_** pAllDefsR;
		pAllDefsR = (cs_GeodeticTransform_**)CS_ralc(*pAllDefs, sizeof(struct cs_GeodeticTransform_ *) * (readStatus + newDefCount));
		if (NULL == pAllDefsR)
		{
			CS_erpt(cs_NO_MEM);
			goto error;
		}

		//...and copy our items beyond those we have got from [DefinitionGetAll]
		*pAllDefs = pAllDefsR;
		struct cs_GeodeticTransform_ ** pStart = &(*pAllDefs)[readStatus];
		memset(pStart, 0x0, (newDefCount * sizeof(struct cs_GeodeticTransform_ *)));
		memcpy(pStart, &duplicateDefs[0], newDefCount * sizeof(struct cs_GeodeticTransform_ *));
		duplicateDefs.clear();
	}

	return (readStatus + newDefCount);

error:

	if (pAllDefs)
	{
		for(int i = 0; i < readStatus; ++i)
		{
			CS_free((*pAllDefs)[i]);
		}
		CS_free(*pAllDefs);
		*pAllDefs = NULL;
	}

	for(GxDuplicatesMap::iterator duplicatesIterator = gxDuplicates.begin(); duplicatesIterator != gxDuplicates.end(); ++duplicatesIterator)
	{
		std::vector<struct cs_GeodeticTransform_ *> const& defDuplicates = duplicatesIterator->second;
		for(size_t i = 0; i < defDuplicates.size(); ++i)
		{
			CS_free(defDuplicates[i]);
		}
	}

	for(size_t i = 0; i < duplicateDefs.size(); ++i)
	{
		CS_free(duplicateDefs[i]);
	}

	return -1;
}

/****************************/
/* Category proxy functions */
/****************************/

csFILE* CS_ctFileOpen(Const char* mode)
{
	Q_RETURN(csFILE*, NULL,CS_FileOpen<cs_CTDEF_MAGIC>(cs_Ctname, mode));
}

int CS_ctRead (csFILE *strm, struct cs_Ctdef_ *ct_def)
{
	Q_RETURN(int, -1, CSrdCategoryEx(strm, ct_def));
}

int CS_ctWrite(csFILE *strm, Const struct cs_Ctdef_ *ct_def)
{
	int writeStatus = CSwrtCategory(strm, ct_def);
	return writeStatus ?
		TRUE //--> failure!
		: FALSE; // --> success!
}

int CS_ctDelete(struct cs_Ctdef_ *ct_def)
{
	CS_CHECK_NULL_ARG(ct_def, 1);
	Q_RETURN(int, -1, CSdelCategory(ct_def->ctName));
}

int CS_ctUpdate(Const struct cs_Ctdef_ *ct_def)
{
	CS_CHECK_NULL_ARG(ct_def, 1);
	Q_RETURN(int, -1, CSupdCategory(ct_def));
}

struct cs_Ctdef_* CS_ctDefinition(Const char *ctName)
{
	Q_RETURN(cs_Ctdef_*, NULL, CSgetCtDef(ctName));
}

int CS_ctDefinitionAll(struct cs_Ctdef_** pAllDefs[])
{
	Q_RETURN(int, -1, CSgetCtDefAll(pAllDefs));
}

} //extern "C"

#ifdef _WIN32

	#pragma warning ( pop )

#endif
