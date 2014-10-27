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

struct tst_lst_
{
	char name [24];
	struct cs_Csdef_ *cs_ptr;
	struct cs_Dtdef_ *dt_ptr;
	struct cs_Eldef_ *el_ptr;
};

void usage (int batch);
void CS_reset (void);
double CStestRN (double low, double high);
int CStestXYZ (double xyz [3],double falseOrg [2],unsigned* sequencer);
int CStestLLH (double llh [2],double cntrlMer,unsigned* sequencer);
double CStstsclh (struct cs_Csprm_ *csprm,double ll [2]);
double CStstcnvrg (struct cs_Csprm_ *csprm,double ll [2]);

int CStest1 (int verbose,int crypt);
int CStest2 (int verbose,int crypt);
int CStest3 (int verbose,int crypt);
int CStest4 (int verbose,char *test_file);
int CStest5 (int verbose,long32_t duration);
int CStest6 (int verbose,int crypt); 
int CStest7 (int verbose,int crypt); 
int CStest8 (int verbose,int crypt); 
int CStest9 (int verbose);
int CStestA (int verbose,char *test_file);
int CStestB (int verbose,long32_t duration);
int CStestC (int verbose,long32_t duration);
int CStestD (int verbose,long32_t duration);
int CStestE (int verbose,long32_t duration);
int CStestF (int verbose,long32_t duration);
int CStestG (int verbose,long32_t duration);
int CStestH (int verbose,long32_t duration);
int CStestI (int verbose,long32_t duration);
int CStestJ (int verbose,long32_t duration);
int CStestK (int verbose,long32_t duration);
int CStestL (int verbose,long32_t duration);
int CStestS (int verbose);
int CStestT (int verbose,long32_t duration);
