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

// Now comes May 21, 2014 
// The following list, and the order of their listing, has been optimized for
// the use of pre-compiled headers.  Some of these files are unreferenced in
// this module, a small price paid for the efficiency afforded by pre-compiled
// headers.
/*lint -e766 */		/* Disable PC-Lint's warning of unreferenced headers */

#include "cs_map.h"
#include "cs_NameMapper.hpp"
//  cs_NameMapper.hpp includes cs_CsvFileSupport.hpp
//  cs_NameMapper.hpp includes csNameMapperSupport.hpp
#include "cs_WktObject.hpp"
#include "cs_wkt.h"

#include "cs_Legacy.h"
#include "cs_EpsgStuff.h"
#include "cs_OsGeoTest.hpp"

/* These includes moved here to give the cs_map.h header control over some
   debug defines. */
#include <ctype.h>
#include <time.h>
#include <locale.h>

// A structure used in several tests.
struct tst_lst_
{
	char name [24];
	struct cs_Csdef_ *cs_ptr;
	struct cs_Dtdef_ *dt_ptr;
	struct cs_Eldef_ *el_ptr;
};

// Supporting functions.
void usage (bool batch);
void CS_reset (void);
double CStestRN (double low, double high);
int CStestXYZ (double xyz [3],double falseOrg [2],unsigned* sequencer);
int CStestLLH (double llh [2],double cntrlMer,unsigned* sequencer);
double CStstsclk (struct cs_Csprm_ *csprm,double ll [2]);
double CStstsclh (struct cs_Csprm_ *csprm,double ll [2]);
double CStstcnvrg (struct cs_Csprm_ *csprm,double ll [2]);

// Individual test function prototypes.
int CStest1 (bool verbose,bool crypt);
int CStest2 (bool verbose,bool crypt);
int CStest3 (bool verbose,bool crypt);
int CStest4 (bool verbose,char *test_file);
int CStest5 (bool verbose,long32_t duration);
int CStest6 (bool verbose,bool crypt); 
int CStest7 (bool verbose,bool crypt); 
int CStest8 (bool verbose,bool crypt); 
int CStest9 (bool verbose);
int CStestA (bool verbose,char *test_file);
int CStestB (bool verbose,long32_t duration);
int CStestC (bool verbose,long32_t duration);
int CStestD (bool verbose,long32_t duration);
int CStestE (bool verbose,long32_t duration);
int CStestF (bool verbose,long32_t duration);
int CStestG (bool verbose,long32_t duration);
int CStestH (bool verbose,long32_t duration);
int CStestI (bool verbose,long32_t duration);
int CStestJ (bool verbose,long32_t duration);
int CStestK (bool verbose,long32_t duration);
int CStestL (bool verbose,long32_t duration);
int CStestM (const TcsEpsgDataSetV6& epsgV6,bool verbose,long32_t duration);
int CStestN (const TcsEpsgDataSetV6& epsgV6,bool verbose,long32_t duration);
int CStestQ (bool verbose,long32_t duration,char *test_file,int revision);
int CStestR (bool verbose,char *test_file);
int CStestS (bool verbose);
int CStestT (bool verbose,long32_t duration);

