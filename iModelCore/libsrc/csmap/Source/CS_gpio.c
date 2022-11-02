/*
 * Copyright (c) 2008, Autodesk, Inc.
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

#include "cs_map.h"
#include "cs_ioUtil.h"

/**********************************************************************
**	Calling Sequence:	strm = CS_gpopn (mode);
**
**	int mode;					mode of the file open, ala fcntl.h
**	csFILE *strm;				returns a file descriptor to the open file.
**
**	Will return NULL if a problem was encountered.
**
**	File is positioned past the magic number on the front.
**********************************************************************/
csFILE * EXP_LVL5 CS_gpopn (Const char *mode)
{
	return CS_gpFileOpen(mode);
}

/**********************************************************************
**	flag = CS_gprd (strm,gp_rec);
**
**	csFILE strm;				file/device from which the possibly encrypted
**								ellipsoid definition is to be read.
**	struct cs_GeodeticPathDef_ *gp_rec;
**								next geodetic path definition record is returned here.
**	int flag;					returns +1 for successful read, 0 for EOF,
**								-1 for error.
**********************************************************************/
int EXP_LVL5 CS_gprd (csFILE *strm,struct cs_GeodeticPath_ *gp_def)
{
    char tempPathKeyName[64] = { '\0' };
    char tempDatumKeyname[cs_KEYNM_DEF] = { '\0' };

    int result = CS_gpRead(strm, gp_def);
    if (result <= 0)
        return result;

	/* Check the result. The name must always meet the criteria
	   set by the CS_nmpp64 function.  At least so far, the criteria
	   established by CS_nampp over the years has always been
	   expanded, never restricted.  Thus, any definition which
	   was legitimate in a previous release would always be
	   legitimate in subsequent releases. */
	CS_stncp (tempPathKeyName,gp_def->pathName,sizeof (tempPathKeyName));
	if (CS_nampp64 (tempPathKeyName) != 0)
	{
		/* Replace the error condition reported by CS_nampp with
		   and Invalid File indication. */
		CS_erpt (cs_INV_FILE);
		return (-1);
	}
	CS_stncp (tempDatumKeyname,gp_def->srcDatum,sizeof (tempDatumKeyname));
	if (CS_nampp (tempDatumKeyname) != 0)
	{
		/* Replace the error condition reported by CS_nampp with
		   and Invalid File indication. */
		CS_erpt (cs_INV_FILE);
		return (-1);
	}
	CS_stncp (tempDatumKeyname,gp_def->trgDatum,sizeof (tempDatumKeyname));
	if (CS_nampp (tempDatumKeyname) != 0)
	{
		/* Replace the error condition reported by CS_nampp with
		   and Invalid File indication. */
		CS_erpt (cs_INV_FILE);
		return (-1);
	}

	return result;
}
/**********************************************************************
**	st = CS_gpwr (strm,gp_rec);
**
**	csFILE *strm;				file/device to which the 
**								geodetic path definition is to be written.
**	struct cs_GeodeticPathDef_ *gp_rec;
**								the geodetic path definition record which is
**								to be written.
**	int st;						returns FALSE if write was completed successfully,
**								else returns TRUE.
**********************************************************************/
int EXP_LVL5 CS_gpwr (csFILE *strm,Const struct cs_GeodeticPath_ *gp_def)
{
	return CS_gpWrite(strm, gp_def);
}

/**********************************************************************
**	st = CS_gpdel (gpdef);
**
**	struct cs_GeodeticPath_ *gpdef;
**								a pointer to the geodetic path definition
**								structure which is to be deleted from the
**								Geodetic Path Dictionary.
**	int st;						returns a zero if the delete was successfully
**								completed, else returns -1.
**********************************************************************/
int EXP_LVL3 CS_gpdel (struct cs_GeodeticPath_ *gp_def)
{
	return CS_gpDelete(gp_def);
}

/******************************************************************************
**	flag = CS_gpupd (eldef,crypt);
**
**	struct cs_GeodeticPath_ *gpdef;
**								a pointer to the ellipsoid definition
**								structure which is to be written to the
**								Geodetic Path Dictionary.
**	int flag;					returns TRUE (+1) if the indicated geodetic
**								path previously existed and was simply
**								updated, returns FALSE (0) if the geodetic
**								path had to be added as a new path, returns
**								-1 if the update process failed.
**
**	If the Geodetic Path Dictionary does not already contain an entry
**	with the indicated key name, the entry is added.
*******************************************************************************/
int EXP_LVL5 CS_gpupd (struct cs_GeodeticPath_ *gp_def)
{
	return CS_gpUpdate(gp_def);
}

/**********************************************************************
**	cmp_val = CS_gpcmp (pp,qq);
**
**	struct cs_GeodeticPathDef_ *pp;
**	struct cs_GeodeticPathDef_ *qq;
**								The geodetic path definition records which
**								are to be compared.
**	int cmp_val;				return is negative if pp follows qq in the
**								collating sequence, positive if pp precedes
**								qq, and zero if they are equivalent.
**
** Compares only the three elements required to make the key value of
** each record unique.  Thus, rcrdCnt and protect are not included
** in the comparison.
**********************************************************************/
int EXP_LVL7 CS_gpcmp (Const struct cs_GeodeticPath_ *pp,
					   Const struct cs_GeodeticPath_ *qq)
{
	int st;

	/* OK, now we can compare the two structures.  For sorting
	   and binary search purposes, we only look at the source and
	   target key names and the record number. */

	st = CS_stricmp (pp->pathName,qq->pathName);
	return (st);
}
/******************************************************************************
**	gp_ptr = CS_gpdef (Const char* pathName);
**
**	char *pathName;				key name of the geodetic path definition
**								which is to be fetched.
**	struct cs_GeodeticPath_ *gp_ptr;
**								returns a pointer to a malloc'ed geodetic path
**								definition structure.  Calling module owns the
**								returned structure and is responsible for
**								freeing it.
**
**	This function only fetches the geodetic path definition as recorded in
**	the geodetic path dictionary.
**********************************************************************/
struct cs_GeodeticPath_ * EXP_LVL3 CS_gpdef (Const char *pathName)
{
	return CS_gpdef2 (pathName, NULL);
}

struct cs_GeodeticPath_ * EXP_LVL5 CS_gpdef2 (Const char *pathName, char* pszDirPath)
{
	return CS_gpDefinition(pathName, pszDirPath);
}


/**********************************************************************
**	gp_ptr = CS_gpdefEx (Const char* srcDatum,Const char* trgDatum);
**
**	char *srcDatum;				key name of the source datum of the path
**								definition which is to be fetched.
**	char *trgDatum;				key name of the target datum of the path
**								definition which is to be fetched.
**	struct cs_GeodeticPath_ *gp_ptr;
**								returns a pointer to a malloc'ed
**								geodetic path definition structure.
**********************************************************************/
/*lint -esym(550,globalFoundForward)  Variable set, but not used. */
struct cs_GeodeticPath_ * EXP_LVL3 CS_gpdefEx (int* direction,
											   Const char *srcDatum,
											   Const char *trgDatum)
{
	extern char csErrnam [];
	extern char cs_UserDir[];

	extern int cs_Error;

	char currentDir[MAXPATH] = { '\0' };
	char targetPaths[2][MAXPATH] = { { '\0'}, {'\0'} };
	char const* pTargetPath;
	
	int st;
	size_t i;

	long curFpos;
	long fwdFpos;
	long invFpos;

	int globalFoundForward;
	int globalFoundBackward;

	csFILE *strm;
	struct cs_GeodeticPath_ *gp_def;

	char tmpKeyName [64];
	char errMsg [256];
	
	__ALIGNMENT__1				/* For some versions of Sun compiler. */
	struct cs_GeodeticPath_ gp_rec;

	/* Prepare for the potential error condition. */
	strm = NULL;
	gp_def = NULL;
	*direction = cs_DTCDIR_NONE;

	globalFoundForward = 0;
	globalFoundBackward = 0;

	/* Make sure the provided names are OK. */
	CS_stncp (tmpKeyName,srcDatum,sizeof (tmpKeyName));
	st = CS_nampp (tmpKeyName);
	if (st != 0) goto error;
	CS_stncp (tmpKeyName,trgDatum,sizeof (tmpKeyName));
	st = CS_nampp (tmpKeyName);
	if (st != 0) goto error;

	st = CS_getdr(currentDir);
	if (st != 0) goto error;

	CS_stncp(targetPaths[0], cs_UserDir, sizeof(targetPaths[0]));
	CS_stncp(targetPaths[1], currentDir, sizeof(targetPaths[1]));

	/* go through all directories we possibly have.  Search the user
	   dictionary first, than the distribution directory. */
	for (i = 0; i < (sizeof(targetPaths) / sizeof(targetPaths[0])); ++i)
	{
		if (NULL != strm)
		{
			CS_fclose (strm);
			strm = NULL;
		}

		/* Search the file for the requested coordinate system definition. */
		pTargetPath = targetPaths[i];
		if ('\0' == *pTargetPath)
			continue;

		//switch [cs_dir] and [cs_DirP] to whatever the current target directory is
		if (CS_setdr(pTargetPath, NULL))
			goto error;

		/* Open up the geodetic path dictionary file. */
		strm = CS_gpopn (_STRM_BINRD);
		if (strm == NULL)
		{
			cs_Error = 0;
			continue;
		}

		/* Search the file for the requested path definition. */
		fwdFpos = 0L;
		invFpos = 0L;
		for (;;)
		{
			curFpos = CS_ftell (strm);
			st = CS_gprd (strm,&gp_rec);
			if (st < 0)
			{
				goto error;	
			} 
			if (st == 0)
			{
				break;
			}

			/* See if we have a match in the forward direction. */
			if (!CS_stricmp (gp_rec.srcDatum,srcDatum) &&
				!CS_stricmp (gp_rec.trgDatum,trgDatum))
			{
				//we must not have yet hit a "forward path"
				fwdFpos = curFpos;
				globalFoundForward = 1;
                // Optimization, stop search when a forward path is found. 
                // The forward paths are preferred over the backward ones.
                break;
			}
			/* See if we have a match in the inverse direction, but only if the path
			   is marked as reversible. */
			else if (gp_rec.reversible != 0)
			{
				if (!CS_stricmp (gp_rec.srcDatum,trgDatum) &&
					!CS_stricmp (gp_rec.trgDatum,srcDatum))
				{
					if (invFpos == 0L && 0 == globalFoundBackward) // When double inverse entry. Be tolerant and continue.
					{
						invFpos = curFpos;
						globalFoundBackward = 1;
					}
				}
				// Continue search, the forward paths are preferred over the backward ones.
			}
		} //for (;;)

		/* We always return the forward definition if we found one. */
		if (fwdFpos != 0L)
		{
			if (!gp_def) //we might have allocated that in the first round (inverse)
				gp_def = (struct cs_GeodeticPath_ *)CS_malc (sizeof (struct cs_GeodeticPath_));

			if (gp_def == NULL)
			{
				CS_erpt (cs_NO_MEM);
				goto error;
			}

			memset ((void*)gp_def,0,sizeof (*gp_def));
			st = CS_fseek (strm,fwdFpos,SEEK_SET);
			if (st != 0)
			{
				CS_erpt (cs_IOERR);
				goto error;
			}
			if (!CS_gprd (strm,gp_def))
			{
				goto error;
			}
			else
			{
				*direction = cs_DTCDIR_FWD;
			}
		}
		else if (invFpos != 0L && !gp_def) //we prefer the forward paths over the backward ones
		{
			gp_def = (struct cs_GeodeticPath_ *)CS_malc (sizeof (struct cs_GeodeticPath_));
			if (gp_def == NULL)
			{
				CS_erpt (cs_NO_MEM);
				goto error;
			}

			memset ((void*)gp_def,0,sizeof (*gp_def));
			st = CS_fseek (strm,invFpos,SEEK_SET);
			if (st != 0)
			{
				CS_erpt (cs_IOERR);
				goto error;
			}
			if (!CS_gprd (strm,gp_def))
			{
				goto error;
			}
			else
			{
				*direction = cs_DTCDIR_INV;
			}
		}
		
		if (strm != NULL)			/*lint !e774  always evaluates to true */
		{
			CS_fclose (strm);
			strm = NULL;
		}
        if (fwdFpos != 0L && NULL != gp_def) 
        {
            // Optimization, stop search when a forward path is found. 
            // The forward paths are preferred over the backward ones.
            // Continue searching when no or only a backward path is found.
            break;
        }
	}

	/* report a failure after we've gone through the 2 directories and we still haven't found
	   any good entry */
	if (NULL == gp_def)
	{
		sprintf (errMsg,"'%s' to '%s'",srcDatum,trgDatum);
		CS_stncp (csErrnam,errMsg,MAXPATH);
		CS_erpt (cs_GP_NOPATH);

		goto error;
	}

	st = CS_setdr(currentDir, NULL);
	if (st != 0)
	{
		goto error;
	}

	/* Return a pointer to the malloc'ed geodetic path definition to the
	   user. */
	return (gp_def);

error:
	if (strm != NULL) CS_fclose (strm);
	if (gp_def != NULL)
	{
		CS_free (gp_def);
		gp_def = NULL;
	}

	CS_setdr(currentDir, NULL);		/*lint !e534  ignoring return value, not much we can do about it here */

	return NULL;
}

#ifdef GEOCOORD_ENHANCEMENT
/**********************************************************************
**	int CS_gpdefFrom (struct cs_GeodeticTransform_ *array[], int numArray, Const char *srcDatum);
**
**  Discovers and extracts all transform paths that are explicitely defined
**  in the transform path dictionary to or from the given datum.
**
**  array                       An array of transform path pointer that will receive
**                              newly allocated transform paths. These paths must
**                              be explicitely freed when not needed anymore.
**
**  numArray                    Indicates the maximum number of entries in the array.
**                              after this number no more paths will be returned.
**
**	char *srcDatum;				key name of the source datum of the path
**								definition which is to be fetched.
**  
**  returns                     The total number of transform paths that were discovered.
**                              This number may be greater than the number of transform paths
**                              returned. It is possible to call this method a first time
**                              to obtain the total number of entries that would be returned
**                              then call it a second time with a properly sized array.
**********************************************************************/
/*lint -esym(550,globalFoundForward)  Variable set, but not used. */
int EXP_LVL3 CS_gpdefFrom(struct cs_GeodeticPath_ *array[], int numArray, Const char *srcDatum)
{
	extern char csErrnam [];
	extern char cs_UserDir[];

	extern int cs_Error;

	char currentDir[MAXPATH] = { '\0' };
	char targetPaths[2][MAXPATH] = { { '\0'}, {'\0'} };
	char const* pTargetPath;
	
	int st;
	size_t i;

	long curFpos;

	csFILE *strm;
	struct cs_GeodeticPath_ *gp_def;

	char tmpKeyName [64];
	
	__ALIGNMENT__1				/* For some versions of Sun compiler. */
	struct cs_GeodeticPath_ gp_rec;

	/* Prepare for the potential error condition. */
	strm = NULL;
	gp_def = NULL;

    /* Clear return array */
    for (int i = 0 ; i < numArray; ++i)
        array[i] = NULL;
    int numInArray = 0;

	/* Make sure the provided names are OK. */
	CS_stncp (tmpKeyName,srcDatum,sizeof (tmpKeyName));
	st = CS_nampp (tmpKeyName);
	if (st != 0) goto error;

	st = CS_getdr(currentDir);
	if (st != 0) goto error;

	CS_stncp(targetPaths[0], cs_UserDir, sizeof(targetPaths[0]));
	CS_stncp(targetPaths[1], currentDir, sizeof(targetPaths[1]));

	/* go through all directories we possibly have.  Search the user
	   dictionary first, than the distribution directory. */
	for (i = 0; i < (sizeof(targetPaths) / sizeof(targetPaths[0])); ++i)
	{
		if (NULL != strm)
		{
			CS_fclose (strm);
			strm = NULL;
		}

		/* Search the file for the requested coordinate system definition. */
		pTargetPath = targetPaths[i];
		if ('\0' == *pTargetPath)
			continue;

		//switch [cs_dir] and [cs_DirP] to whatever the current target directory is
		if (CS_setdr(pTargetPath, NULL))
			goto error;

		/* Open up the geodetic path dictionary file. */
		strm = CS_gpopn (_STRM_BINRD);
		if (strm == NULL)
		{
			cs_Error = 0;
			continue;
		}

		/* Search the file for the requested path definition. */
		for (;;)
		{
			curFpos = CS_ftell (strm);
			st = CS_gprd (strm,&gp_rec);
			if (st < 0)
			{
				goto error;	
			} 
			if (st == 0)
			{
				break;
			}

			/* See if we have a match in the forward direction. */
			if (!CS_stricmp (gp_rec.srcDatum,srcDatum))
			{
                gp_def = (struct cs_GeodeticPath_ *)CS_malc (sizeof (struct cs_GeodeticPath_));

                if (gp_def == NULL)
                {
                    CS_erpt (cs_NO_MEM);
                    goto error;
                }

                memset ((void*)gp_def,0,sizeof (*gp_def));
                st = CS_fseek (strm, curFpos,SEEK_SET);
                if (st != 0)
                {
                    CS_erpt (cs_IOERR);
                    goto error;
                }
                if (!CS_gprd (strm,gp_def))
                {
                    goto error;
                }
                if (numArray > numInArray)
                {
                    array[numInArray] = gp_def;
                    gp_def = NULL;
                }
                else if (gp_def != NULL)
                {
                    CS_free(gp_def);
                    gp_def = NULL;
                }

                numInArray++;
			}
			/* See if we have a match in the inverse direction, but only if the path
			   is marked as reversible. */
			else if (gp_rec.reversible != 0)
			{
				if (!CS_stricmp (gp_rec.trgDatum, srcDatum))
				{
                    gp_def = (struct cs_GeodeticPath_ *)CS_malc (sizeof (struct cs_GeodeticPath_));

                    if (gp_def == NULL)
                    {
                        CS_erpt (cs_NO_MEM);
                        goto error;
                    }

                    memset ((void*)gp_def,0,sizeof (*gp_def));
                    st = CS_fseek (strm, curFpos,SEEK_SET);
                    if (st != 0)
                    {
                        CS_erpt (cs_IOERR);
                        goto error;
                    }
                    if (!CS_gprd (strm,gp_def))
                    {
                        goto error;
                    }
                    if (numArray > numInArray)
                    {
                        array[numInArray] = gp_def;
                        gp_def = NULL;
                    }
                    else if (gp_def != NULL)
                    {
                        CS_free(gp_def);
                        gp_def = NULL;
                    }

                    numInArray++;
				}
				// Continue search, the forward paths are preferred over the backward ones.
			}
		} //for (;;)

		if (strm != NULL)			/*lint !e774  always evaluates to true */
		{
			CS_fclose (strm);
			strm = NULL;
		}
	}

	st = CS_setdr(currentDir, NULL);
	if (st != 0)
	{
		goto error;
	}

	/* Return a pointer to the malloc'ed geodetic path definition to the
	   user. */
	return (numInArray);

error:
	if (strm != NULL) CS_fclose (strm);
	if (gp_def != NULL)
	{
		CS_free (gp_def);
		gp_def = NULL;
	}

	CS_setdr(currentDir, NULL);		/*lint !e534  ignoring return value, not much we can do about it here */

	return 0;
}
#endif


int EXP_LVL1 CS_gpchk (Const struct cs_GeodeticPath_ *gpPath,unsigned short gpChkFlg,int err_list [],int list_sz)
{
	extern char csErrnam [MAXPATH];

	short gpIdx;

	int st;
	int ii;
	int gxIndex;
	int err_cnt;

	struct cs_GeodeticPathElement_* gpPtr;
	struct cs_GxIndex_* gxPtr;

	char datumName [cs_KEYNM_DEF];

	err_cnt = -1;			/* We'lll be returning err_cnt + 1 */

	if (CS_nampp64 (gpPath->pathName))
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_GPQ_NAME;
	}

	/* Checking the datum names is optional as this code will always check
	   against the datum dictionary in the standard location.  This may not
	   be appropriate in certain environments such as the dictionary
	   compiler. */
	if ((gpChkFlg & cs_GPCHK_DATUM) != 0)
	{
		/* Check the datums. */
		if (!CS_dtIsValid (gpPath->srcDatum))
		{
			if (++err_cnt < list_sz) err_list [err_cnt] = cs_GPQ_GPDTM;
		}
		if (!CS_dtIsValid (gpPath->trgDatum))
		{
			if (++err_cnt < list_sz) err_list [err_cnt] = cs_GPQ_GPDTM;
		}
	}

	/* Make sure the elementCount is sane before we go producing a memory
	   exception. */
	if (gpPath->elementCount <= 0 || gpPath->elementCount > csPATH_MAXXFRM)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_GPQ_COUNT;
	}
	else
	{
		for (gpIdx = 0;gpIdx < gpPath->elementCount;gpIdx += 1)
		{
			gpPtr = &gpPath->geodeticPathElements [gpIdx];
			
			/* Check for a valid direction indication. */
			if (gpPtr->direction != cs_DTCDIR_FWD && gpPtr->direction != cs_DTCDIR_INV)
			{
				if (++err_cnt < list_sz) err_list [err_cnt] = cs_GPQ_GXDIR;
			}

			/* Check for a valid transformation name. */
			st = CS_nampp64 (gpPtr->geodeticXformName);
			if (st != 0)
			{
				if (++err_cnt < list_sz) err_list [err_cnt] = cs_GPQ_INVNM;
			}

			/* The following flag check enables us to use this function in the
			   dictionary compiler.  In the compiler, it may be, or may not be,
			   appropriate to check the transformation dictionary for a valid
			   name as the dictionaries being generated may not be the same ones
			   that standard CS_gx??? functions will access. */
			if ((gpChkFlg & cs_GPCHK_XFORM) != 0)
			{
				/* OK, we have been instructed to check the dictionary for the
				   existence of the transformation name. */
				gxIndex = CS_locateGxByName (gpPtr->geodeticXformName);
				if (gxIndex < 0)
				{
					if (++err_cnt < list_sz) err_list [err_cnt] = cs_GPQ_NOXFRM;
				}
			}
		}
	}

	/* If we're good so far, check that the path is consistent.  That is,
	   the chain of datum names is proper.  We'll need a GX index for this,
	   so this is not always appropriate. */
	if (err_cnt < 0 && (gpChkFlg & cs_GPCHK_XFORM) != 0)
	{
		/* We start with the source datum of the path. */
		CS_stncp (datumName,gpPath->srcDatum,sizeof (datumName));

		/* Check that the chain of transformations is valid. */
		for (gpIdx = 0;gpIdx < gpPath->elementCount;gpIdx += 1)
		{
			gpPtr = &gpPath->geodeticPathElements [gpIdx];

			/* Determine the source datum for the current transformation. */
			gxIndex = CS_locateGxByName (gpPtr->geodeticXformName);
			if (gxIndex < 0)			/* Defensive */
			{
				if (++err_cnt < list_sz) err_list [err_cnt] = cs_GPQ_NOXFRM;
				break;
			}
			gxPtr = CS_getGxIndexEntry (gxIndex);
			if (gpPtr->direction == cs_DTCDIR_FWD)
			{
				if (CS_stricmp (datumName,gxPtr->srcDatum))
				{
					/* Oops! Chain is broken. */
					if (++err_cnt < list_sz) err_list [err_cnt] = cs_GPQ_CHAIN;
				}
				else
				{
					CS_stncp (datumName,gxPtr->trgDatum,sizeof (datumName));
				}
			}
			else if (gpPtr->direction == cs_DTCDIR_INV)
			{
				if (!gxPtr->inverseSupported)
				{
					/* Path says use the inverse, but the transformation does
					   not support the inverse. */
					if (++err_cnt < list_sz) err_list [err_cnt] = cs_GPQ_NOINV;
				}
				else if (CS_stricmp (datumName,gxPtr->trgDatum))
				{
					/* Oops! Chain is broken. */
					if (++err_cnt < list_sz) err_list [err_cnt] = cs_GPQ_CHAIN;
					break;
				}
				else
				{
					/* Chain is continuous, so far; check next transformation in
					   the path. */
					CS_stncp (datumName,gxPtr->srcDatum,sizeof (datumName));
				}
			}

			/* No sense trying to follow a broken chain. */
			if (err_cnt >= 0)
			{
				break;
			}
		}
		if (err_cnt < 0)
		{
			/* Verify that the path did end up at the target datum. */
			if (CS_stricmp (datumName,gpPath->trgDatum))
			{
				if (++err_cnt < list_sz) err_list [err_cnt] = cs_GPQ_CHAIN;
			}
		}
	}

	/* If so instructed, report all of the errors detected so far. */
	if ((gpChkFlg & cs_GPCHK_REPORT) != 0)
	{
		CS_stncp (csErrnam,gpPath->pathName,MAXPATH);
		for (ii = 0;ii <= err_cnt && ii < list_sz;ii++)
		{
			CS_erpt (err_list [ii]);
		}
	}

	return (err_cnt + 1);
}

/**********************************************************************
**	count = CS_gpdefAll (pDefArray);
**
**	cs_GeodeticPath_ **pDefArray[];	A pointer to an array of cs_GeodeticPath_ instances.
**	
**	This function reads all available cs_GeodeticPath_ definitions from all available
**	dictionary files. [pDefArray] will be set to a malloc'ed array
**	of cs_GeodeticPath_ pointers where both, i.e. the array and the pointers contained,
**	will have to be CS_free'd() by the caller. The number
**	of pointers contained in the array is returned by [count].
**	That is, count will be >= 0 in case of success and a negative value
**	otherwise in which case [pDefArray] must not be examined by the caller.
**********************************************************************/
int EXP_LVL3 CS_gpdefAll (struct cs_GeodeticPath_ **pDefArray[])
{
	return CS_gpDefinitionAll(pDefArray);
}

/**********************************************************************
**	CS_gpfnm (new_name);
**
**	char *new_name;		the name of the geodetic path dictionary file.
**
**	This function specifies the name only.  The directory,
**	and possibly drive, are specified using CS_altdr.
**********************************************************************/
void EXP_LVL1 CS_gpfnm (Const char *new_name)
{
	extern char cs_Gpname [];

	(void)CS_stncp (cs_Gpname,new_name,cs_FNM_MAXLEN);
	return;
}
