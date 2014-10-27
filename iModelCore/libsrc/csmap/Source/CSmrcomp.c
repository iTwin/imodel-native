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

/*lint -e661				*/			/* out-of-bounds pointer */
/*lint -esym(715,err_func)	*/			/* function parameter not currently used */
/*lint -e702                */			/* right shift of signed quantity */

/*??*/		/* bit-nbr should probably be changed to an unsigned type.  This
			   is not a problem, though, as bit_nbr is never negative. */

#include "cs_map.h"
#include "cs_Legacy.h"

#ifndef HGT
#	define HGT 2
#endif

#define MR_NAME   1
#define TEST_LNG  2
#define TEST_LAT  3
#define DELT_LNG  4
#define DELT_LAT  5
#define DELT_HGT  6
#define UU_OFF    7
#define VV_OFF    8
#define KK        9
#define LAT_COEF 10
#define LNG_COEF 11
#define HGT_COEF 12

struct cs_CscmpT_
{
	char label [24];
	int type;
};

__ALIGNMENT__1		/* For some versions of Sun compiler. */
static struct cs_CscmpT_ cs_CscmpT [] =
{
	{"\015DELTA_LAMBDA:"  ,DELT_LNG},
	{"\015DELTA_HEIGHT:"  ,DELT_HGT},
	{"\014TEST_LAMBDA:"   ,TEST_LNG},
	{"\013DATUM_NAME:"    ,MR_NAME},
	{"\013LAMBDA_OFF:"    ,VV_OFF},
	{"\012DELTA_PHI:"     ,DELT_LAT},
	{"\011TEST_PHI:"      ,TEST_LAT},
	{"\010PHI_OFF:"       ,UU_OFF},
	{"\006LAMBDA"         ,LNG_COEF},
	{"\006HEIGHT"         ,HGT_COEF},
	{"\003PHI"            ,LAT_COEF},
	{"\003KK:"            ,KK},
	{"\0"                 ,0}
};


__ALIGNMENT__2		/* For some versions of Sun compiler. */
static struct csDmaMReg_ csDmaMReg;
static double test_lng;
static double test_lat;
static double delt_lng;
static double delt_lat;
static double delt_hgt;
static double lng [10][10];
static double lat [10][10];
static double hgt [10][10];
int line_nbr;
static char dup_fndr [500][10];

int CSmrcmpC (struct csDmaMReg_ *mr_ptr,char *mr_name,int (*err_func)(char *mesg));
int CSmrcmpA (struct csDmaMReg_ *mr_ptr,int (*err_func)(char *mesg));
int CSmrdefwr (	struct csDmaMReg_ *mr_ptr,char *mr_name,Const char *outp,csFILE *dtStrm,int (*err_func)(char *err_msg));

/**********************************************************************
**	err_cnt = CSmrcomp (inpt,outp,flags,datums,err_func);
**
**	char *inpt;					full path to the ASCII source file.
**	char *outp;					full path to the output DIRECTORY.
**	int flags;					bit map of option flags, only needed to access the
**								datum file correctly.
**	char *datums;				full path name to the datums dictionary which is to
**								be used to verify integrity of names, else null.
**	int (*err_func)(char *mesg);function used to report error messages.  Returns
**								non-zero to indicate cancel.
**	int err_cnt;				returns the number of errors encountered.
**********************************************************************/

int EXP_LVL9 CSmrcomp (	Const char *inpt,
						Const char *outp,
						int flags,
						Const char *datum,
						int (*err_func)(char *mesg)
					  )
{
	int ii;
	int jj;
	int st;
	int cnt;
	int demo;
	int cancel;
	int err_cnt;

	int degs;
	int mins;
	int minus;

	size_t rdCnt;

	cs_magic_t magic;

	char *cp;
	csFILE *dtStrm;
	csFILE *inStrm;
	struct cs_CscmpT_ *tp;

	double tmp_d;
	double secs;
	double coef;

	char mr_name [16];
	char err_seg [18];
	char buff [128];
	char err_msg [128];

	demo = ((flags & cs_CMPLR_DEMO) != 0);
	memset (dup_fndr,'\0',sizeof (dup_fndr));
				
	/* Open the source file. */
	inStrm = CS_fopen (inpt,_STRM_TXTRD);
	if (inStrm == NULL)
	{
		sprintf (err_msg,"Couldn't open %s for input.\n",inpt);
		cancel = (*err_func)(err_msg);
		return (1);
	}

	/* If we have been given a datum file name, we open it now. */
	if (datum != NULL && *datum != '\0')
	{
		/* CS_dtopn uses a canned file name, so we can't use it here. */
		dtStrm = CS_fopen (datum,_STRM_BINRD);
		if (dtStrm == NULL)
		{
			sprintf (err_msg,"Couldn't open %s as a datum file.",datum);
			cancel = (*err_func)(err_msg);
			CS_fclose (inStrm);
			return (1);
		}
		rdCnt = CS_fread (&magic,1,sizeof (magic),dtStrm);
		CS_bswap (&magic,"l");
		if (rdCnt != sizeof (magic) ||
			(demo && magic != cs_DTDEF_MAGIC) ||
			(!demo && magic != cs_DTDEF_MAGIC))
		{
			sprintf (err_msg,"%s is not a Datum Dictionary file.",datum);
			cancel = (*err_func)(err_msg);
			CS_fclose (inStrm);
			CS_fclose (dtStrm);
			return (1);
		}
	}
	else
	{
		dtStrm = NULL;
	}

	/* Initialize the csDmaMReg structure for the first multiple
	   regression coefficient definition. */
	mr_name [0] = '\0';
	(void)memset (&csDmaMReg,'\0',sizeof (csDmaMReg));
	csDmaMReg.magic = cs_MULREG_MAGIC;
	(void)memset (lng,'\0',sizeof (lng));
	(void)memset (lat,'\0',sizeof (lat));
	(void)memset (hgt,'\0',sizeof (hgt));

	/* Process each line in the source file. */
	line_nbr = 0;
	err_cnt = 0;
	cancel = FALSE;
	while (CS_fgets (buff,sizeof (buff),inStrm) != NULL)
	{
		if (cancel) break;
		line_nbr += 1;

		/* Ignore comments and blank lines. */
		CS_trim (buff);
		if (buff [0] == '#' || buff [0] == '\0')
		{
			continue;
		}
		cp = buff;
		while ((cp = strchr (cp,'#')) != NULL)
		{
			if (*(cp + 1) != '#' &&
				*(cp - 1) != '\\')
			{
				*cp = '\0';
				break;
			}
		}

		/* Run the line through the table parser.  We
		   see what type of line we have. */
		cp = buff;
		tp = (struct cs_CscmpT_ *)CS_tpars (&cp,cs_CscmpT,sizeof (struct cs_CscmpT_));

		if (tp == NULL)
		{
			CS_stncp (err_seg,buff,sizeof (err_seg));
			sprintf (err_msg,"Invalid keyword at line %d.\n",line_nbr);
			cancel = (*err_func)(err_msg);
			err_cnt += 1;
			mr_name [0] = '\0';
			continue;
		}

		/* Here when we have a valid line. Cp points to the first
		   character after the colon. */
		CS_trim (cp);
		switch (tp->type) {

		case MR_NAME:
			/* Here each time we encounter a new multiple
			   regression.  We see if there is an existing
			   multiple regression which must be written. */
			if (mr_name [0] != '\0')
			{
				/* See if we have one with this name already. */
				for (ii = 0;dup_fndr [ii][0] != '\0';ii++)
				{
					if (ii > 499) break;
					if (!CS_stricmp (mr_name,dup_fndr [ii])) break;
				}
				if (dup_fndr [ii][0] != '\0')					/*lint !e661 */
				{
					sprintf (err_msg,"Datum name %s appears more than once.",mr_name);
					cancel = (*err_func)(err_msg);
					err_cnt += 1;
				}
				else
				{
					CS_stncp (dup_fndr [ii],mr_name,10);
				}
					
				st = CSmrdefwr (&csDmaMReg,mr_name,outp,dtStrm,err_func);
				if (st >= 0) err_cnt += st;
				else
				{
					err_cnt += -st;
					cancel = TRUE;
				}
			}
				
			/* Save the name of the new datum. */
			if (strlen (cp) > 15)
			{
				sprintf (err_msg,"Datum name %s is too long; truncating to 15 characters.",cp);
				cancel = (*err_func)(err_msg);
				err_cnt += 1;
				*(cp + 15) = '\0';
			}
			CS_stncp (mr_name,cp,sizeof (mr_name));

			/* Initialize for a new datum. */
			memset (&csDmaMReg,'\0',sizeof (csDmaMReg));
			csDmaMReg.magic = cs_MULREG_MAGIC;
			memset (lng,'\0',sizeof (lng));
			memset (lat,'\0',sizeof (lat));
			memset (hgt,'\0',sizeof (hgt));
			break;

		case KK:
			csDmaMReg.kk = atof (cp);
			break;

		case UU_OFF:
			csDmaMReg.uu_off = atof (cp);
			break;

		case VV_OFF:
			tmp_d = atof (cp);
			if (tmp_d <= -180.0) tmp_d += 360;
			csDmaMReg.vv_off = tmp_d;
			break;

		case DELT_LNG:
			delt_lng = atof (cp);
			break;

		case DELT_LAT:
			delt_lat = atof (cp);
			break;

		case DELT_HGT:
			delt_hgt = atof (cp);
			break;

		case TEST_LNG:
			cnt = sscanf (cp,"%d %d %lf",&degs,&mins,&secs);
			if (cnt != 3 || degs < 0 || degs > 360 ||
							mins < 0 || mins >= 60 ||
							secs < 0.0 || secs >= 60.0)
			{
				sprintf (err_msg,"At line %d, invalid longitude format.\n",line_nbr);
				cancel = (*err_func)(err_msg);
				err_cnt += 1;
			}
			else
			{
				test_lng = (double)degs + ((double)mins / 60.0) + (secs / 3600.00);
				if (test_lng > 180.0) test_lng -= 360.0;
			}
			break;

		case TEST_LAT:
			cnt = sscanf (cp,"%d %d %lf",&degs,&mins,&secs);
			if (cnt != 3 || degs <= -90  || degs >= 90 ||
							mins < 0 || mins >= 60 ||
							secs < 0.0 || secs >= 60.0)
			{
				sprintf (err_msg,"At line %d, invalid latitude format.\n",line_nbr);
				cancel = (*err_func)(err_msg);
				err_cnt += 1;
			}
			else
			{
				if (degs < 0.0)
				{
					minus = TRUE;
					degs = -degs;
				}
				else
				{
					minus = FALSE;
				}
				test_lat = (double)degs + ((double)mins / 60.0) + (secs / 3600.00);
				if (minus)
				{
					test_lat = -test_lat;
				}
			}
			break;

		case LNG_COEF:
			cnt = sscanf (cp,"U%d V%d: %lf",&ii,&jj,&coef);
			if (cnt != 3 ||	jj < 0 || jj > 9 ||	ii < 0 || ii > 9)
			{
				sprintf (err_msg,"At line %d, invalid coefficient specification.",line_nbr);
				cancel = (*err_func)(err_msg);
				err_cnt += 1;
			}
			else
			{
				lng [jj][ii] = coef;
			}
			break;

		case LAT_COEF:
			cnt = sscanf (cp,"U%d V%d: %lf",&ii,&jj,&coef);
			if (cnt != 3 || jj < 0 || jj > 9 || ii < 0 || ii > 9)
			{
				sprintf (err_msg,"At line %d, invalid coefficient specification.",line_nbr);
				cancel = (*err_func)(err_msg);
				err_cnt += 1;
			}
			else
			{
				lat [jj][ii] = coef;
			}
			break;

		case HGT_COEF:
			cnt = sscanf (cp,"U%d V%d: %lf",&ii,&jj,&coef);
			if (cnt != 3 || jj < 0 || jj > 9 || ii < 0 || ii > 9)
			{
				sprintf (err_msg,"At line %d, invalid coefficient specification.",line_nbr);
				cancel = (*err_func)(err_msg);
				err_cnt += 1;
			}
			else
			{
				hgt [jj][ii] = coef;
			}
			break;

		default:
			sprintf (err_msg,"Software error: %s at line %d.",__FILE__,__LINE__);
			(void)(*err_func)(err_msg);
			cancel = TRUE;
			break;
		}
		/* On to the next input line. */
	}
	CS_fclose (inStrm);
	if (cancel)
	{
		if (dtStrm != NULL) CS_fclose (dtStrm);
		return (err_cnt);
	}

	/* Write out the last datum, if it's OK. */
	if (err_cnt == 0 && mr_name [0] != '\0')
	{
		st = CSmrdefwr (&csDmaMReg,mr_name,outp,dtStrm,err_func);
		if (st >= 0) err_cnt += st;
		else
		{
			err_cnt += -st;
			cancel = TRUE;
		}
	}

	/* All done. */
	if (dtStrm != NULL) CS_fclose (dtStrm);
	return (err_cnt);
}

int CSmrcmpA (	struct csDmaMReg_ *mr_ptr,
				int (*err_func)(char *mesg)		/* Not used, currently */
			 )
{
	extern double lng [10][10];
	extern double lat [10][10];
	extern double hgt [10][10];

	int ii;
	int jj;
	int wrd;
	int idx;
	int bit_nbr;

	ulong32_t mask;

	/* Initialize the max exponent variables.  These limit
	   the amount of computation when calculating
	   transformations. */
	mr_ptr->max_uu = 0;
	mr_ptr->max_vv = 0;

	/* Initialize the bit maps for each of the calculations.
	   Look at these bit maps as 10 by 10 arrays.  A bit is
	   set if the corresponding coeficient is present which
	   implies that it is non-zero. */
	for (ii = 0;ii < 4;ii++)
	{
		mr_ptr->lng_map [ii] = 0L;
		mr_ptr->lat_map [ii] = 0L;
		mr_ptr->hgt_map [ii] = 0L;
	}

	/* Initialize index.  We use a variable number of
	   coeficients for each calculation, rather than a separate
	   array for each.  This keeps the size of the structure
	   down to a minimum. */
	idx = 0;

	/* Now, we transfer each individual non-zero coefficient
	   to the coeficient array.  While doing so, we must keep
	   the max exponent, bit map, and idx items up to date.
	   We do the longitude coefficients first. */
	for (ii = 0;ii < 10;ii++)
	{
		for (jj = 0;jj < 10;jj++)
		{
			if (lng [ii][jj] == 0.0)
			{
				continue;
			}
			bit_nbr = (ii * 10) + jj;
			wrd = bit_nbr >> 5;
			mask = (ulong32_t)0x80000000L >> (bit_nbr & 0x1F);
			mr_ptr->lng_map [wrd] |= mask;
			if ((ii + 1) > mr_ptr->max_vv) mr_ptr->max_vv = (unsigned char)(ii + 1);
			if ((jj + 1) > mr_ptr->max_uu) mr_ptr->max_uu = (unsigned char)(jj + 1);
			mr_ptr->coefs [idx++] = lng [ii][jj];
		}
	}

	/* Save the idex value for the first latitude coefficient. */
	mr_ptr->lat_idx = (unsigned char)idx;

	/* Now do the latitude coefficients. */
	for (ii = 0;ii < 10;ii++)
	{
		for (jj = 0;jj < 10;jj++)
		{
			if (lat [ii][jj] == 0.0)
			{
				continue;
			}
			bit_nbr = (ii * 10) + jj;
			wrd = bit_nbr >> 5;
			mask = (ulong32_t)0x80000000L >> (bit_nbr & 0x1F);
			mr_ptr->lat_map [wrd] |= mask;
			if ((ii + 1) > mr_ptr->max_vv) mr_ptr->max_vv = (unsigned char)(ii + 1);
			if ((jj + 1) > mr_ptr->max_uu) mr_ptr->max_uu = (unsigned char)(jj + 1);
			mr_ptr->coefs [idx++] = lat [ii][jj];
		}
	}

	/* Save the index to the first height coefficient. */
	mr_ptr->hgt_idx = (unsigned char)idx;

	/* Now we do the height coefficients. */
	for (ii = 0;ii < 10;ii++)
	{
		for (jj = 0;jj < 10;jj++)
		{
			if (hgt [ii][jj] == 0.0)
			{
				continue;
			}
			bit_nbr = (ii * 10) + jj;
			wrd = bit_nbr >> 5;
			mask = (ulong32_t)0x80000000L >> (bit_nbr & 0x1F);
			mr_ptr->hgt_map [wrd] |= mask;
			if ((ii + 1) > mr_ptr->max_vv) mr_ptr->max_vv = (unsigned char)(ii + 1);
			if ((jj + 1) > mr_ptr->max_uu) mr_ptr->max_uu = (unsigned char)(jj + 1);
			mr_ptr->coefs [idx++] = hgt [ii][jj];
		}
	}

	/* Compute the minimum size of this thing and stuff
	   it in the structure.  This is the amount of data
	   which we write.  Since this value is stored in the
	   file, modules which read this data should use this
	   value to determine if they got everything that they
	   were supposed to. */
	mr_ptr->mr_size = (long32_t)(sizeof (csDmaMReg) - sizeof (csDmaMReg.coefs) +
									 (idx * sizeof (double)));

	/* That's that. */
	return (0);
}

#ifdef __SKIP__
int CSmrcmpC (struct csDmaMReg_ *mr_ptr,char *mr_name,int (*err_func)(char *mesg))
{
	extern double delt_lng;
	extern double delt_lat;
	extern double test_lng;
	extern double test_lat;

	int cancel;
	int err_cnt;

	double tmp_lng;
	double tmp_lat;

	double ll_lcl [3];
	double ll_84 [3];

	char err_msg [128];

	struct cs_DmaMReg_ newMReg;

	err_cnt = 0;
	cancel = FALSE;

	ll_lcl [LNG] = test_lng;
	ll_lcl [LAT] = test_lat;
	ll_lcl [HGT] = 0;

	memcpy (&newMReg.mReg,mr_ptr,sizeof (newMReg.mReg));
	newMReg.fallback = dtcTypNone;
	newMReg.fallbackXfrm.molo = NULL;
	if (CS_dmaMr3dFowrd (ll_84,ll_lcl,&newMReg) != 0)
	{
		sprintf (err_msg,"Multiple Regession test point out of range, datum is %s.",mr_name);
		cancel = (*err_func)(err_msg);
		err_cnt += 1;
	}
	else
	{
		tmp_lng = (ll_84 [LNG] - ll_lcl [LNG]) * 3600.0;
		tmp_lat = (ll_84 [LAT] - ll_lcl [LAT]) * 3600.0;

		if (fabs (tmp_lng - delt_lng) > 5.0E-04)
		{
			sprintf (err_msg,"Delta Longitude (lambda) error, datum %s; is %f, should be %f.",mr_name,tmp_lng,delt_lng);
			cancel = (*err_func)(err_msg);
			err_cnt += 1;
		}
		if (fabs (tmp_lat - delt_lat) > 5.0E-04)
		{
			sprintf (err_msg,"Delta Latitude (phi) error, datum %s; is %f, should be %f.",mr_name,tmp_lat,delt_lat);
			cancel = (*err_func)(err_msg);
			err_cnt += 1;
		}
		if (fabs (ll_84 [HGT] - delt_hgt) > 5.0E-03)
		{
			sprintf (err_msg,"Delta Height error, datum %s; is %f, should be %f.",mr_name,ll_84 [HGT],delt_hgt);
			cancel = (*err_func)(err_msg);
			err_cnt += 1;
		}
	}

	return (cancel ? -err_cnt : err_cnt);
}
#endif

int CSmrdefwr (	struct csDmaMReg_ *mr_ptr,
				char *mr_name,
				Const char *outp,
				csFILE *dtStrm,
				int (*err_func)(char *err_msg)
			  )
{
	extern char cs_DirsepC;
	extern char cs_ExtsepC;

	int st;
	int flag;

	int err_cnt;
	int cancel;

	size_t mrSize;
	size_t wrCnt;

	char *cp;
	csFILE *mrStrm;

	char mr_path [MAXPATH];
	char err_msg [MAXPATH];

	__ALIGNMENT__3		/* For some versions of Sun compiler. */
	struct cs_Dtdef_ dt_def;

	err_cnt = 0;
	cancel = FALSE;

	/* Construct the name for the file. */
	cp = CS_stncp (mr_path,outp,sizeof (mr_path) - 16);
	if (*(cp - 1) != cs_DirsepC)
	{
		*cp++ = cs_DirsepC;
		*cp = '\0';
	}
	cp = CS_stcpy (cp,mr_name);
	*cp++ = cs_ExtsepC;
	CS_stcpy (cp,cs_MULREG_EXT);

	/* Compose the structure. */
	st = CSmrcmpA (mr_ptr,err_func);
	if (st >= 0) err_cnt += st;
	else
	{
		err_cnt += -st;
		cancel = TRUE;
	}

	/* TODO -->
	   Check the current definition.
	st = CSmrcmpC (mr_ptr,mr_name,err_func);
	if (st >= 0) err_cnt += st;
	else
	{
		err_cnt += -st;
		cancel = TRUE;
	}  */

	/* Verify that the datum dictionary has a corresponding entry. */
	if (dtStrm != NULL)
	{
		CS_stncp (dt_def.key_nm,mr_name,sizeof (dt_def.key_nm));
		dt_def.fill [0] = '\0';
		dt_def.fill [1] = '\0';
		flag = CS_bins (dtStrm,(long32_t)sizeof (cs_magic_t),(long32_t)-1,sizeof (dt_def),&dt_def,(CMPFUNC_CAST)CS_dtcmp);
		if (!flag)
		{
			sprintf (err_msg,"No Datum Dictionary entry for %s (line %d).",mr_name,line_nbr);
			cancel = (*err_func)(err_msg);
			err_cnt += 1;
		}
	}

	/* If it passed all tests, write it out. */
	if (err_cnt == 0)
	{
		/* Write it out to a file with the datum name. */
		mrStrm = CS_fopen (mr_path,_STRM_BINWR);
		if (mrStrm == NULL)
		{
			sprintf (err_msg,"Couldn't create/truncate %s.",mr_path);
			cancel = (*err_func)(err_msg);
			err_cnt += 1;
		}
		else
		{
			/* We only write as much as necessary.  Save mr_size
			   before writing so it survives the possible byte
			   swap. */
			mrSize = (size_t)mr_ptr->mr_size;
			CS_bswap (mr_ptr,cs_BSWP_DMAMREG);
			wrCnt = CS_fwrite (mr_ptr,1,mrSize,mrStrm);
			CS_fclose (mrStrm);
			if (wrCnt != mrSize)
			{
				sprintf (err_msg,"Write to %s failed.",mr_path);
				cancel = (*err_func)(err_msg);
				err_cnt += 1;
			}
		}
	}
	return (cancel ? -err_cnt : err_cnt);
}
