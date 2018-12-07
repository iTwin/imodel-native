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

//lint -esym(534,CS_cs3ll,CS_ll3cs)
//lint -esym(715,verbose,duration)
//lint -esym(752,cs_Doserr,cs_Error,cs_Errno,cs_ErrSup,csErrlng,csErrlat)

#include "cs_map.h"
#include "cs_Test.h"
#include "csNameMapperSupport.h"

extern int cs_Error;
extern int cs_Errno;
extern int csErrlng;
extern int csErrlat;
extern unsigned short cs_ErrSup;
#if _RUN_TIME <= _rt_UNIXPCC
extern ulong32_t cs_Doserr;
#endif

int CStestL (int verbose,long32_t duration)
{
	enum EcsMapSt csMapSt;

	int err_cnt;

	struct cs_Csprm_* xyMeter;
	struct cs_Csprm_* xyCentimeter;
	
	double src [3];
	double ll [3];
	double trg [3];
	
	char cTemp [1024];

	err_cnt = 0;
	
	csMapSt = csMapNameToNameC (csMapProjectionKeyName,cTemp,sizeof (cTemp),
                                                        csMapFlvrOCR,
												        csMapFlvrCsMap,
														"LMTAN");
	if (csMapSt != csMapOk)
	{
		printf ("Name Mapper failed to map projection CS-MAP projection name \"LMTAN\".\n");
		err_cnt += 1;
	}

	src [0] = src [1] = 10.0;
	xyMeter = CS_csloc ("XY-M");
	if (xyMeter != 0)
	{
		xyCentimeter = CS_csloc ("XY-CM");
		if (xyCentimeter != 0)
		{
			CS_cs3ll (xyMeter,ll,src);
			CS_ll3cs (xyCentimeter,trg,ll);
			CS_free (xyCentimeter);
		}
		else
		{
			printf ("NERTH test failed on \"XY-CM\"\n");
			err_cnt += 1;
		}
		CS_free (xyMeter);
	}
	else
	{
		printf ("NERTH test failed on \"XY-M\"\n");
		err_cnt += 1;
	}
	
    return err_cnt;
}
