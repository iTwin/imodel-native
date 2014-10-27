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

void CStestSa (const char *swp_name)
{
	printf ("Swapping contents of %s now.\n",swp_name);
	return;
}
int CStestS (int verbose)
{
	extern char cs_Dir [];
	extern char *cs_DirP;
	extern char cs_Csname [];
	extern union cs_Bswap_ cs_BswapU;

	int st = 0;
#ifdef __SKIP__
	size_t rdCnt;

	csFILE *strm;

	char magic [sizeof (cs_magic_t)];
	
	printf ("Switching byte order in all files for subsequent tests.\n");

	/* Now we force CS_swap into swap mode. */
	cs_BswapU.llll = 0x010203L;

	st = 0;	
#if _RUN_TIME < _rt_UNIXPCC
	/* It appears that there are problems with the directory tree scan stuff
	   under UNIX.  So, we simply comment this out for now. */
	/* Swap all the files. */
	if (verbose)
	{
		st = CS_swpal (CStestSa);
	}
	else
	{
		st = CS_swpal (NULL);
	}
#endif
	
	/* Now we set CS_bswap to operate correctly with the results;
	   regardless of what type of machine we are on. In order to do
	   this, we determine the current state of the binary files.
	   This may be the same as before if CS_swpal had some sort of
	   problem. */
	(void)strcpy (cs_DirP,cs_Csname);
	strm = CS_fopen (cs_Dir,_STRM_BINRD);
	if (strm == NULL)
	{
		st = cs_CSDICT;
	}
	else
	{
		rdCnt = CS_fread (&magic,1,sizeof (magic),strm);
		CS_fclose (strm);
		if (rdCnt != sizeof (magic))
		{
			st = CS_ferror (strm) ? cs_IOERR : cs_INV_FILE;
		}
		else
		{
			if (magic [0] != '\200')
			{
				/* Little endian order. */
				cs_BswapU.cccc [0] = 0x00;
				cs_BswapU.cccc [1] = 0x01;
				cs_BswapU.cccc [2] = 0x02;
				cs_BswapU.cccc [3] = 0x03;
			}
			else
			{
				/* Little endian order. */
				cs_BswapU.cccc [0] = 0x03;
				cs_BswapU.cccc [1] = 0x02;
				cs_BswapU.cccc [2] = 0x01;
				cs_BswapU.cccc [3] = 0x00;
			}
		}
	}
#endif
	return (st);
}
