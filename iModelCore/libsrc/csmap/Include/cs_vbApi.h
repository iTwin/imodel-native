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

/* This module contains function prototypes for the Visual Basic interface
   modules declared in CS_vbApi.c.  It is the intent that these functions
   be compiled into CsMapDll.dll and not be used anywhere else.  Thus, a
   special header file for these function prototypes.

   Probably do even need a header file as these functions woould/should not
   be referenced anywhere else; except perhaps in a testing routine or
   something similar. */

#ifdef __cplusplus
extern "C" {
#endif

/* Variations of the specifically for Visual Basic.  They could work elsewhere,
   but they have been tested in Visual Basic. */
int					EXP_LVL1	CS_csEnumVb (int index,char *key_name,int size);
int					EXP_LVL1	CS_csRangeEnumVb (int index,char *key_name,int size);
int					EXP_LVL1	CS_dtEnumVb (int index,char *key_name,int size);
int					EXP_LVL1	CS_elEnumVb (int index,char *key_name,int size);
void				EXP_LVL1	CS_errmsgVb (char *user_bufr,int buf_size);
long32_t			EXP_LVL1	CS_ftoaVb (char *bufr,int size,double value,long frmt);
int					EXP_LVL1	CS_getDataDirectoryVb (char *data_dir,int dir_sz);
int					EXP_LVL1	CS_getDatumOfVb (Const char *csKeyName,char *datumName,int size);
int					EXP_LVL1	CS_getDescriptionOfVb (Const char *csKeyName,char *description,int size);
int					EXP_LVL1	CS_getEllipsoidOfVb (Const char *csKeyName,char *ellipsoidName,int size);
int					EXP_LVL1	CS_getReferenceOfVb (Const char *csKeyName,char *reference,int size);
int					EXP_LVL1	CS_getSourceOfVb (Const char *csKeyName,char *source,int size);
int					EXP_LVL1	CS_getUnitsOfVb (Const char *csKeyName,char *unitName,int size);
int					EXP_LVL1	CS_mgrsFromLlVb (char* result,int rdlt_size,double latLng [2],int prec);
int					EXP_LVL1	CS_prjEnumVb (int index,ulong32_t *prj_flags,char *prj_keynm,int keynm_sz,char *prj_descr,int descr_sz);
int					EXP_LVL1	CS_unEnumVb (int index,int type,char *un_name,int un_size);
int					EXP_LVL1	CS_unEnumPluralVb (int index,int type,char *un_name,int un_size);
long32_t			EXP_LVL1	CS_mapIdToIdVb (enum EcsMapObjType type,enum EcsNameFlavor trgFlavor,
																		enum EcsNameFlavor srcFlavor,
																		long32_t srcId);
long32_t			EXP_LVL1	CS_mapNameToIdVb (enum EcsMapObjType type,enum EcsNameFlavor trgFlavor,
																		  enum EcsNameFlavor srcFlavor,
																		  const char* srcName);
long32_t			EXP_LVL1	CS_mapNameToNameVb (enum EcsMapObjType type,char* trgName,
																			size_t trgSize,
																			enum EcsNameFlavor trgFlavor,
																			enum EcsNameFlavor srcFlavor,
																			const char* srcName);
long32_t			EXP_LVL1	CS_mapIdToNameVb (enum EcsMapObjType type,char* trgName,
																		  size_t trgSize,
																		  enum EcsNameFlavor trgFlavor,
																		  enum EcsNameFlavor srcFlavor,
																		  long32_t srcId);
#ifdef __cplusplus
}
#endif

