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
 *       contr butors may be used to endorse or promote products derived
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

/* This module carries a dozen or so of stub functions, specifically designed
   to make access to the DLL easy for VB/VBA users.

   Each of the functions defined here has a corresponding function in the
   normal library.  What each of these stubs do, is instead of returing a
   null terminated 'C' string in the provided buffer, these stubs return
   a left justified field, with space padding on the right, which is _NOT_
   null terminated.  The size argument specifies how wide the "field" is.

	Thus, VB/VBA users (and perhaps users of other high level languages such
	as Delphi, Optima, etc.) can conveniently access the core functions with
	code similar to the following:
	
	Dim format As Long
	Dim buffer As String * 32
	Dim value As Double
	Dim stringValue As String
	.
	format -= cs_ATOF_LNGDFLT;
	value - 1234.56
	format = CS_ftoa (buffer,LEN (buffer),value,format);
	stringValue = Trim (buffer)
	.
	.
	.

	Further, most versions of VB and VBA use 8 bit characters.  It could be
	that future versions use something else.  Localizing this action in this
	module makes it easy to switch between the two; and the modules are
	designed to facilitate that.  Note that CS_MAP is strictly an 8 bit
	character library.
*/

#include "cs_map.h"
#include "cs_NameMapperSupport.h"
#include "cs_vbApi.h"

const long32_t csNmMaprSt_Ok      =  0;	/* Normal completion */
const long32_t csNmMaprSt_NoName  =  1;	/* Source entry was found, no name defined with target flavor */
const long32_t csNmMaprSt_NoNbr   =  2;	/* Source flavored entry was found, no number defined with target flavor */
const long32_t csNmMaprSt_NoMatch =  4;	/* Source entry could not defined in Name Mapper */
const long32_t csNmMaprSt_NoMap   = -1;	/* Name mapper initialization failed, use CS_errmsg to get reason */

void EXP_LVL5 CSstrToVba (char *vbaStr,int vbaLen,Const char *csMapStr)
{
	int idx;
	int len;

	len = (int)strlen (csMapStr);
	for (idx = 0;idx < vbaLen;++idx)
	{
		if (idx < len)
		{
			*(vbaStr + idx) = *(csMapStr + idx);
		}
		else
		{
			*(vbaStr + idx) = ' ';
		}
	}
	return;
}

long32_t EXP_LVL5 csNmMapStToInt (enum EcsMapSt nmMapSt)
{
	long32_t rtnValue;

	switch (nmMapSt) {
	case csMapOk:      rtnValue =  0; break;
    case csMapNoName:  rtnValue =  1; break;
    case csMapNoNbr:   rtnValue =  2; break;
    case csMapNoMatch: rtnValue =  4; break;
	default:           rtnValue = -1; break;
	}
	return rtnValue;
}

int EXP_LVL1 CS_csEnumVb (int index,char *key_name,int size)
{
	int rtnValue;
	char csMapBufr [32];

	csMapBufr [0] = '\0';
	rtnValue = CS_csEnum (index,csMapBufr,(int)sizeof (csMapBufr));
	CSstrToVba (key_name,size,csMapBufr);
	return rtnValue;
}
int EXP_LVL1 CS_csRangeEnumVb (int index,char *key_name,int size)
{
	int rtnValue;
	char csMapBufr [32];

	csMapBufr [0] = '\0';
	rtnValue = CS_csRangeEnum (index,csMapBufr,(int)sizeof (csMapBufr));
	CSstrToVba (key_name,size,csMapBufr);
	return rtnValue;
}
int EXP_LVL1 CS_dtEnumVb (int index,char *key_name,int size)
{
	int rtnValue;
	char csMapBufr [32];

	csMapBufr [0] = '\0';
	rtnValue = CS_dtEnum (index,csMapBufr,(int)sizeof (csMapBufr));
	CSstrToVba (key_name,size,csMapBufr);
	return rtnValue;
}
int EXP_LVL1 CS_elEnumVb (int index,char *key_name,int size)
{
	int rtnValue;
	char csMapBufr [32];

	csMapBufr [0] = '\0';
	rtnValue = CS_elEnum (index,csMapBufr,(int)sizeof (csMapBufr));
	CSstrToVba (key_name,size,csMapBufr);
	return rtnValue;
}
void EXP_LVL1 CS_errmsgVb (char *user_bufr,int buf_size)
{
	char csMapBufr [256];

	csMapBufr [0] = '\0';
	CS_errmsg (csMapBufr,(int)sizeof (csMapBufr));
	CSstrToVba (user_bufr,buf_size,csMapBufr);
	return;
}
long32_t EXP_LVL1 CS_ftoaVb (char *bufr,int size,double value,long frmt)
{
	long32_t rtnValue;
	char csMapBufr [64];

	csMapBufr [0] = '\0';
	rtnValue = CS_ftoa (csMapBufr,(int)sizeof (csMapBufr),value,frmt);
	CSstrToVba (bufr,size,csMapBufr);
	return rtnValue;
}
int EXP_LVL1 CS_getDataDirectoryVb (char *data_dir,int dir_sz)
{
	int rtnValue;
	char csMapBufr [256];

	csMapBufr [0] = '\0';
	rtnValue = CS_getDataDirectory (csMapBufr,(int)sizeof (csMapBufr));
	CSstrToVba (data_dir,dir_sz,csMapBufr);
	return rtnValue;
}
int EXP_LVL1 CS_getDatumOfVb (Const char *csKeyName,char *datumName,int size)
{
	int rtnValue;
	char csMapBufr [32];

	csMapBufr [0] = '\0';
	rtnValue = CS_getDatumOf (csKeyName,csMapBufr,(int)sizeof (csMapBufr));
	CSstrToVba (datumName,size,csMapBufr);
	return rtnValue;
}
int EXP_LVL1 CS_getDescriptionOfVb (Const char *csKeyName,char *description,int size)
{
	int rtnValue;
	char csMapBufr [128];

	csMapBufr [0] = '\0';
	rtnValue = CS_getDescriptionOf (csKeyName,csMapBufr,(int)sizeof (csMapBufr));
	CSstrToVba (description,size,csMapBufr);
	return rtnValue;
}
int EXP_LVL1 CS_getEllipsoidOfVb (Const char *csKeyName,char *ellipsoidName,int size)
{
	int rtnValue;
	char csMapBufr [32];

	csMapBufr [0] = '\0';
	rtnValue = CS_getEllipsoidOf (csKeyName,csMapBufr,(int)sizeof (csMapBufr));
	CSstrToVba (ellipsoidName,size,csMapBufr);
	return rtnValue;
}
int EXP_LVL1 CS_getReferenceOfVb (Const char *csKeyName,char *reference,int size)
{
	int rtnValue;
	char csMapBufr [128];

	csMapBufr [0] = '\0';
	rtnValue = CS_getReferenceOf (csKeyName,csMapBufr,(int)sizeof (csMapBufr));
	CSstrToVba (reference,size,csMapBufr);
	return rtnValue;
}
int EXP_LVL1 CS_getSourceOfVb (Const char *csKeyName,char *source,int size)
{
	int rtnValue;
	char csMapBufr [128];

	csMapBufr [0] = '\0';
	rtnValue = CS_getSourceOf (csKeyName,csMapBufr,(int)sizeof (csMapBufr));
	CSstrToVba (source,size,csMapBufr);
	return rtnValue;
}
int EXP_LVL1 CS_getUnitsOfVb (Const char *csKeyName,char *unitName,int size)
{
	int rtnValue;
	char csMapBufr [32];

	csMapBufr [0] = '\0';
	rtnValue = CS_getUnitsOf (csKeyName,csMapBufr,(int)sizeof (csMapBufr));
	CSstrToVba (unitName,size,csMapBufr);
	return rtnValue;
}
int EXP_LVL1 CS_mgrsFromLlVb (char* result,int rslt_size,double latLng [2],int prec)
{
	int rtnValue;
	char csMapBufr [64];

	csMapBufr [0] = '\0';
	rtnValue = CS_mgrsFromLl (csMapBufr,latLng,(int)sizeof (csMapBufr));
	CSstrToVba (result,rslt_size,csMapBufr);
	return rtnValue;
}
int EXP_LVL1 CS_prjEnumVb (int index,ulong32_t *prj_flags,char *prj_keynm,int keynm_sz,char *prj_descr,int descr_sz)
{
	int rtnValue;
	char csMapBufr32 [32];
	char csMapBufr128 [128];

	csMapBufr32 [0] = '\0';
	csMapBufr128 [0] = '\0';
	rtnValue = CS_prjEnum (index,prj_flags,csMapBufr32,(int)sizeof (csMapBufr32),csMapBufr128,(int)sizeof (csMapBufr128));
	CSstrToVba (prj_keynm,keynm_sz,csMapBufr32);
	CSstrToVba (prj_descr,descr_sz,csMapBufr128);
	return rtnValue;
}
int EXP_LVL1 CS_unEnumVb (int index,int type,char *un_name,int un_size)
{
	int rtnValue;
	char csMapBufr [32];

	csMapBufr [0] = '\0';
	rtnValue = CS_unEnum (index,type,csMapBufr,(int)sizeof (csMapBufr));
	CSstrToVba (un_name,un_size,csMapBufr);
	return rtnValue;
}
int EXP_LVL1 CS_unEnumPluralVb (int index,int type,char *un_name,int un_size)
{
	int rtnValue;
	char csMapBufr [32];

	csMapBufr [0] = '\0';
	rtnValue = CS_unEnumPlural (index,type,csMapBufr,(int)sizeof (csMapBufr));
	CSstrToVba (un_name,un_size,csMapBufr);
	return rtnValue;
}
/* Bit of a wobble here.  If the return value is +4 or less, the value is
   actually a status value.  A value greater than +4 is the requested
   ID number. */
long32_t EXP_LVL1 CS_mapIdToIdVb (enum EcsMapObjType type,enum EcsNameFlavor trgFlavor,
														  enum EcsNameFlavor srcFlavor,
														  long32_t srcId)
{
	extern const unsigned long KcsNmInvNumber;
	extern const unsigned long KcsNmMapNoNumber;

	long32_t rtnValue;
	unsigned long myId;
	unsigned long mySrcId;

	mySrcId = (ulong32_t)srcId;
	myId = csMapIdToId (type,trgFlavor,srcFlavor,mySrcId);
	if (myId == KcsNmInvNumber)
	{
		rtnValue = csNmMaprSt_NoMatch;
	}
	else if (myId == KcsNmMapNoNumber)
	{
		rtnValue = csNmMaprSt_NoNbr;
	}
	else
	{
		rtnValue = (long32_t) myId;
	}
	return rtnValue;
}
/* Bit of a wobble here.  If the return value is +4 or less, the value is
   actually a status value.  A value greater than +4 is the requested
   ID number. */
long32_t EXP_LVL1 CS_mapNameToIdVb (enum EcsMapObjType type,enum EcsNameFlavor trgFlavor,
															enum EcsNameFlavor srcFlavor,
															const char* srcName)
{
	extern const unsigned long KcsNmInvNumber;
	extern const unsigned long KcsNmMapNoNumber;

	long32_t rtnValue;
	unsigned long myId;

	myId = csMapNameToIdC (type,trgFlavor,srcFlavor,srcName);
	if (myId == KcsNmInvNumber)
	{
		rtnValue = csNmMaprSt_NoMatch;
	}
	else if (myId == KcsNmMapNoNumber)
	{
		rtnValue = csNmMaprSt_NoNbr;
	}
	else
	{
		rtnValue = (long32_t) myId;
	}
	return rtnValue;
}
long32_t EXP_LVL1 CS_mapNameToNameVb (enum EcsMapObjType type,char* trgName,
															  size_t trgSize,
															  enum EcsNameFlavor trgFlavor,
															  enum EcsNameFlavor srcFlavor,
															  const char* srcName)
{
	enum EcsMapSt nmMapSt;
	
	char nameBufr [1024];

	nmMapSt = csMapNameToNameC (type,nameBufr,sizeof (nameBufr),trgFlavor,srcFlavor,srcName);
	CSstrToVba (trgName,(int)trgSize,nameBufr);
	return csNmMapStToInt (nmMapSt);
}
long32_t EXP_LVL1 CS_mapIdToNameVb (enum EcsMapObjType type,char* trgName,
															size_t trgSize,
															enum EcsNameFlavor trgFlavor,
															enum EcsNameFlavor srcFlavor,
															long32_t srcId)
{
	enum EcsMapSt nmMapSt;
	unsigned long mySrcId;

	char nameBufr [1024];

	mySrcId = (ulong32_t)srcId;
	nmMapSt = csMapIdToNameC (type,nameBufr,sizeof (nameBufr),trgFlavor,srcFlavor,mySrcId);
	CSstrToVba (trgName,(int)trgSize,nameBufr);
	return csNmMapStToInt (nmMapSt);
}
