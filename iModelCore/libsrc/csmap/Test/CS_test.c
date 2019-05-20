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

/*lint -esym(534,CSrgf93Init) */
/*lint -esym(534,_filbuf) */
/*lint -esym(534,CS_atof,CS_ftoa) */
/*lint -esym(578,cs_TestDir) */

#define _CRT_SECURE_NO_DEPRECATE

#include "cs_map.h"
#include "cs_wkt.h"
#include <ctype.h>
#include <time.h>
#include <locale.h>
#include "csNameMapperSupport.h"
#include "cs_Test.h"

const char* MsiToAdskMapCS (const char* msiName);
const char* MsiToAdskMapDT (const char* msiName);
const char* MsiToAdskMapEL (const char* msiName);

#if _RUN_TIME < _rt_UNIXPCC
#	include <crtdbg.h>
#	include <conio.h>
#endif

#ifndef CLK_TCK
#	define CLK_TCK CLOCKS_PER_SECOND
#endif

#ifdef __HEAP__
	static ulong32_t mem_used;
#	if defined (_MSC_VER)
#		define CRTDBG_MAP_ALLOC			// This is really _CRTDBG_MAP_ALLOC
#		define _CRTDBG_MAP_ALLOC		// This doesn't work.
#		include <crtdbg.h>
#		include <malloc.h>
#		if (_MSC_VER >= 800 && _MSC_VER < 1600)
#			define heapwalk _heapwalk
#		endif
#	endif
#	if defined (__WATCOMC__)
#		define heapwalk _heapwalk
#	endif
#endif

extern int cs_Error;
extern int cs_Errno;
extern char cs_DirsepC;

int cs_MeFlag = 0;
char cs_MeKynm [128];
char cs_MeFunc [128];
double cs_Coords [3];
int cs_InitialRandomValue;
char cs_TestFile [MAXPATH];
char cs_TestDir [MAXPATH + MAXPATH];
char* cs_TestDirP;

/**********************************************************************
**	CS_Test [/p30] [/ddirectory] [/t12345] [/llocale] [test_file]
**
**	/t				use the t option to indicate the test
**					numbers, and the specific sequence
**					they are to be run.
**	/d				use the /d argument to indicate the directory
**					which is to be searched for the dictionary
**					and datum conversion files.
**	/v				use the v option to indicate that the
**					program is to produce verbose output
**					concerning what it is doing.
**	/p				use the p variable to extent or reduce the
**					number of iterations in a test.  The base
**					value is 30.
**	/l				the value of this option is use to set the current
**					locale for the operation of the test.
**	test_file		if present, the one positional argument
**					is the name of the test file which is
**					to be used for test number 4.
**
**	No arguments are required, but this module expects
**	the release supplied Coordinate System Dictionary
**	and Datum Dictionary to be in the location defined
**	in CSdata.c.
**
**	The heap checker built into this module is only
**	activated by having the -D__HEAP__ on the command
**	line when this module is compiled.  The heap calls
**	included are available in the Borland 4.5 library;
**	maybe some others.  In any case, the test always
**	fails the first time through.  I don't know why.
**	However, if you repeat whatever the first test
**	is, it passes just fine.  The run-time library
**	must be allocating memory somewhere, for some
**	reason, but I can't figure out what that reason
**	may be.
**
**	This test program assumes that there are at least
**	two entries in each of the dictionary files.
**
**********************************************************************/

int main (int argc,char *argv [])
{
	extern char cs_Dir [];
	extern char *cs_DirP;
	extern char cs_OptchrC;
	extern union cs_Bswap_ cs_BswapU;

	extern short cs_Protect;
	extern char cs_Unique;

	extern struct csNad27ToNad83_* csNad27ToNad83;
	extern struct csNad83ToHarn_* csNad83ToHarn;
	extern struct csAgd66ToGda94_* csAgd66ToGda94;
	extern struct csAgd84ToGda94_* csAgd84ToGda94;
	extern struct csNzgd49ToNzgd2K_* csNzgd49ToNzgd2K;

	extern struct csAts77ToCsrs_* csAts77ToCsrs;
	extern struct csNad83ToCsrs_* csNad83ToCsrs;
	extern struct csVertconUS_* csVertconUS;
	extern struct cs_Osgm91_ *cs_Osgm91Ptr;
	extern struct csTokyoToJgd2k_ *csTokyoToJgd2k;
	extern struct csRgf93ToNtf_* csRgf93ToNtf;
    extern struct csDhdnToEtrf89_* csDhdnToEtrf89;

	extern struct cs_Ostn97_ *cs_Ostn97Ptr;
	extern struct cs_Ostn02_ *cs_Ostn02Ptr;

	short protectSave;
	char uniqueSave;

	int ii;
	int st;
	int ok_cnt;
	int bad_cnt;
	int crypt;
	int batch;
	int verbose;
	int test_st;
	int initial_fd;
	int final_fd;

	unsigned seed;

	long32_t duration;
	time_t startTime;

	char *cp;
	char *cp1;
	csFILE *chk_fs;
	csFILE *csStrm;

	char locale [128];
	char tests [128];
	char alt_dir [MAXPATH];
	char cTemp [MAXPATH + MAXPATH];
	
	struct tm* tmPtr;


///////////////////////////////////////////////////////////////////////////////
// The following activates the memory leak detector on MS Visual C++ 7.01
#if defined(__HEAP__) && defined (_MSC_VER)
	int memDbgState;
	char *leakTest;
	//	The following are useful in the Microsoft Windows environment.  As it appears
	//	below, it will report on any memory leaks detected during the evaluation.
	memDbgState = _CrtSetDbgFlag (_CRTDBG_REPORT_FLAG);
	memDbgState |= _CRTDBG_ALLOC_MEM_DF;
	memDbgState |= _CRTDBG_LEAK_CHECK_DF;
//	memDbgState |= _CRTDBG_DELAY_FREE_MEM_DF;	// useful only when hunting for a leak.
//	memDbgState |= _CRTDBG_CHECK_ALWAYS_DF;		// A real dog, hope we never need it.
//	memDbgState |= _CRTDBG_CHECK_CRT_DF;		// CRT allocates environment, but
												// doesn't free it.  So this option
												// will always produce some memory leaks.
	_CrtSetDbgFlag (memDbgState);

	// I uncomment the following, every once in a while, to verify that the
	// leak detection feature is working properly since we rarely have seen
	// a memory leak report.
	leakTest = malloc (139);
	strcpy (leakTest,"Leak detector working");
#endif

	/* Capture the time and date of the start of this test. */
	time (&startTime);
	
	/* Save the distribution values used to activate the dictionary
	   protection system.  We use these to restore the protection
	   mode back to normal after each test. */
	protectSave = cs_Protect;
	uniqueSave = cs_Unique;

	/* Set up the current directory as the possible source for
	   the dictionaries and other data files.  This only works
	   under MS-DOS unless they've changed UNIX since this
	   old fart last worked on a UNIX system.  The /d option
	   overrides this selection if present on the command
	   line. */

#if _OPR_SYSTEM != _os_UNIX
	strncpy (alt_dir,argv [0],sizeof (alt_dir));
	alt_dir [sizeof (alt_dir) - 1] = '\0';
	cp = strrchr (alt_dir,cs_DirsepC);
	if (cp != NULL)
	{
		*cp = '\0';
		CS_altdr (alt_dir);
	}
#endif

	/* Analyze the arguments and extract all options. */

	verbose = FALSE;
	batch = FALSE;
	crypt = FALSE;
	duration = 0L;
	seed = 0;
	locale [0] = '\0';
	tests [0] = '\0';
	alt_dir [0] = '\0';
	cs_TestFile [0] = '\0';
	cs_TestDir [0] = '\0';
	cs_TestDirP = cs_TestDir;
	cs_DirP = CS_stcpy (cs_Dir,"C:\\Program Files\\Common Files\\GeodeticData\\");

	for (ii = 1;ii < argc;ii++)
	{
		/* Here once for each argument. */
		cp = argv [ii];
		if (*cp == cs_OptchrC)
		{
			cp += 1;
			switch (*cp) {

			case 'b':
			case 'B':
				batch = TRUE;
				break;

			case 'c':
			case 'C':
				crypt = TRUE;
				break;

			case 'd':
			case 'D':
				if (alt_dir [0] != '\0')
				{
					printf ("Multiple alternate directory options.\n");
					usage (batch);
				}
				cp += 1;
				if (*cp == '\0')
				{
					CS_stcpy (alt_dir,".");
				}
				else
				{
					CS_stncp (alt_dir,cp,sizeof (alt_dir));
				}
				break;

			case 'l':
			case 'L':
				if (locale [0] != '\0')
				{
					printf ("Multiple locale specifications.\n");
					usage (batch);
				}
				cp += 1;
				if (*cp == '\0')
				{
					CS_stcpy (locale,"C");
				}
				else
				{
					CS_stncp (locale,cp,sizeof (locale));
				}
				break;
				
			case 'p':
			case 'P':

				if (duration != 0)
				{
					printf ("Multiple duration arguments.\n");
					usage (batch);
				}
				cp += 1;
				duration = atol (cp);
				break;

			case 'r':
			case 'R':
				cp += 1;
				seed = atoi (cp) & 0x3FFF;
				break;

			case 's':
			case 'S':
				/* Force CS_bswap to think that it is running on a big
				   endian machine. */
				cs_BswapU.cccc [0] = 0x03;
				cs_BswapU.cccc [1] = 0x02;
				cs_BswapU.cccc [2] = 0x01;
				cs_BswapU.cccc [3] = 0x00;
				break;

			case 't':
			case 'T':
				if (tests [0] != '\0')
				{
					printf ("Multiple test specifications.\n");
					usage (batch);
				}
				cp += 1;
				CS_stncp (tests,cp,sizeof (tests));
				break;

			case 'v':
			case 'V':
				verbose = TRUE;
				break;

			default:
				printf ("Unknown option letter - %c\n",*cp);
				usage (batch);
				break;
			}
		}
		else
		{
			/* It must be the file name. */
			if (cs_TestFile [0] != '\0')
			{
				printf ("Too many positional arguments.\n");
				usage (batch);
			}
			cp1 = strrchr (cp,'\\');
			if (cp1 == 0)
			{
				cp1 = strrchr (cp,'/');
			}
			if (cp1 == 0)
			{
				cs_TestDir [0] = '.';
				cs_TestDir [1] = cs_DirsepC;
				cs_TestDir [2] = '\0';
				cs_TestDirP = &cs_TestDir [2];
				CS_stncp (cs_TestFile,cp,sizeof (cs_TestFile));
			}
			else
			{
				CS_stncp (cs_TestDir,cp,sizeof (cs_TestDir));
				cs_TestDirP = strrchr (cs_TestDir,'\\');
				if (cs_TestDirP == 0)
				{
					cs_TestDirP = strrchr (cs_TestDir,'/');
				}
				if (cs_TestDirP == 0)
				{
					printf ("Internal coding error at line %d.\n",__LINE__);
					usage (batch);
				}
				else
				{
					cs_TestDirP += 1;
					CS_stncp (cs_TestFile,cs_TestDirP,sizeof (cs_TestFile));
					*cs_TestDirP = '\0';
				}
			}
		}
	}

	/* Set up the defaults. */
	if (locale [0] != '\0')
	{
		cp = setlocale (LC_ALL,locale);
		if (cp == NULL)
		{
			printf ("Setting specified locale [%s] failed.\n",locale);
			usage (batch);
		}
		else if (CS_stricmp (locale,cp))
		{
			printf ("WARNING: Specified locale [%s] was interpreted to be %s.\n",locale,cp);
		}
		if (verbose)
		{
			printf ("Locale sensitive rendition of 1234567.89 = \"%.2f\"\n",1234567.89);
			tmPtr = gmtime (&startTime);
			strftime (cTemp,sizeof (cTemp),"%#x",tmPtr);
			printf ("Locale rendition of start time (GMT) and date = \"%s\"\n",cTemp);
		}
	}
	if (cs_TestFile [0] == '\0')
	{
		CS_stncp (cs_TestFile,"TEST.DAT",sizeof (cs_TestFile));
	}
	if (cs_TestDir [0] == '\0')
	{
		cs_TestDir [0] = '.';
		cs_TestDir [1] = cs_DirsepC;
		cs_TestDir [2] = '\0';
		cs_TestDirP = &cs_TestDir [2];
	}
	if (alt_dir [0] != '\0')
	{
		st = CS_altdr (alt_dir);
		if (st != 0)
		{
			printf ("CS_altdr did not succeed.  Either CS_altdr is broke, or\n");
			printf ("the path %s is not a valid data directory path.\n",alt_dir);
			usage (batch);
		}
	}
	if (tests [0] == '\0')
	{
		CS_stcpy (tests,"123456789ABCDEFGHIJKLKJIHGFEDCBA987654321");
	}
	if (duration <= 0L)
	{
		duration = 30L;
	}
	if (seed == 0)
	{
		seed = (unsigned)CS_time ((cs_Time_ *)0) & 0x3FFF;
	}
	*cs_DirP = '\0';
	printf ("Using data files in the %s directory.\n",cs_Dir);
	printf ("Random number seed = %d.\n",seed);
	srand (seed);
	cs_InitialRandomValue = rand ();

	/* Force test values to upper case. */
	for (cp = tests;*cp != '\0';cp += 1)
	{
		*cp = (char)toupper (*cp);
	}

	/* Open a file with and save the file descriptor.
	   This is used to see if any of the modules in
	   CS_MAP leave a file descriptor open.  This is
	   admittedly a poor test, but it has found some
	   bugs, so we leave it in. */
	initial_fd = -1;
	csStrm = CS_csopn (_STRM_BINRD);
	if (csStrm != 0)
	{	
		initial_fd = _fileno (csStrm);
		CS_csDictCls (csStrm);
	}

	/* If the test sequence includes any of the tests which
	   use the test file, we make sure it exists before
	   plunging into the entire thing. */

	cp = strchr (tests,'4');
	if (cp == NULL) cp = strchr (tests,'A');
	if (cp == NULL) cp = strchr (tests,'a');
	if (cp != NULL)
	{
		CS_stncp (cs_TestDirP,cs_TestFile,MAXPATH);
		chk_fs = fopen (cs_TestDir,_STRM_TXTRD);
		if (chk_fs != NULL)
		{
			fclose (chk_fs);
		}
		else
		{
			printf ("Can't open the test data file %s.\n",cs_TestDir);
			return (1);
		}
	}
	
	/* Close/delete any remnants. */
	
	CS_reset ();

	/* If the heap check feature is activated, we walk the
	   current heap and accumulate the amount of heap memory
	   currently in use.  This is used later on to verify
	   that all heap memory used in any test has been
	   successfully returned.  This is conditionally compiled
	   as not all compilers support heap walking, nor is it
	   supported in a standard manner.
	   
	   This test usually fails for the first test.  I believe
	   this is due to the fact that whatever run time library is
	   in use malloc's some memory on the first call to some
	   function.  I feel confident about this as the failure
	   always happens with the first test, no matter which one
	   is chosen.  The specific test which fails always passes
	   on subsequent tries, and always passes if it is not the
	   first test.  So, don't get upset if this test fails
	   on the first test.  */

#ifdef __HEAP__

{	/* start a block so we can declare some automatics */

	int hi_st;
	_HEAPINFO hi;

	hi._pentry = NULL;
	mem_used = 0;
	for (;;)
	{
		hi_st = _heapwalk (&hi);
		if (hi_st != _HEAPOK) break;
		if (hi._useflag == _USEDENTRY)
		{
			mem_used += hi._size;
		}
	}				 
	if (hi_st != _HEAPEND && hi_st != _HEAPEMPTY)
	{
		printf ("Initial heap is corrupt!!!\n");
		return (1);
	}
}
#endif

	/* OK, we now cycle through the list of tests we have been
	   given. */

	ok_cnt = 0;
	bad_cnt = 0;
	for (cp = tests;*cp != '\0';cp += 1)
	{
		/* Restart the entire sequence if we encounter a 'Z' */ 
		if (*cp == 'Z')
		{
			printf ("Recyling through test sequence.\n");
			seed = (unsigned)clock () & 0x3FFF;
			printf ("Random number seed = %d.\n",seed);
			srand (seed);
			cp = tests;
		}

		/* Turn on the dictionary protection system prior to each test.
		   We use the standard distribution values. */

		cs_Protect = protectSave;
		cs_Unique = uniqueSave;

		/* Do the current test. */

		switch (*cp) {
		
		case '0':
			/* Special case: perform a reset. */
			CS_reset ();
			test_st = 0;
			break;

		case '1':

			/* Perform test 1.  This test checks the
			   set of functions which manipulate the
			   coordinate system dictionary. */

			test_st = CStest1 (verbose,crypt);
			break;

		case '2':

			/* Perform test 2.  This test checks the
			   set of functions which manipulate the
			   datum dictionary. */

			test_st = CStest2 (verbose,crypt);
			break;

		case '3':

			/* Perform test 3.  This test checks the
			   set of functions which manipulate the
			   datum dictionary. */

			test_st = CStest3 (verbose,crypt);
			break;

		case '4':

			/* Perform test 4.  This test processes all
			   conversions in the provided test file
			   and reports any discrepancies. */

			test_st = CStest4 (verbose,cs_TestFile);
			break;

		case '5':

			/* Performs a perfomance test on certain
			   conversions. */

			test_st = CStest5 (verbose,duration);
			break;

		case '6':

			/* Tests the sort and binary search functions
			   by reversing the order of the Coordinate
			   System Dictionary file. */

			test_st = CStest6 (verbose,crypt);
			break;

		case '7':

			/* Tests the group list function and actual
			   group data in the Coordinate System
			   Dictionary file. */

			test_st = CStest7 (verbose,crypt);
			break;

		case '8':

			/* Assures that every coordinate system
			   definition is exercised. */

			test_st = CStest8 (verbose,crypt);
			break;

		case '9':

			/* Excercises the auxilliary latitude
			   series functions, assuring correct
			   results and thath the inverse always
			   matches the forward. */

			test_st = CStest9 (verbose);
			break;

		case 'A':
		case 'a':

			/* Perform test A. Same as test 4 but uses
			   the single function interface. */

			test_st = CStestA (verbose,cs_TestFile);
			break;

		case 'B':
		case 'b':

			/* Perform test B. Checks grid scale and
			   convergence angle functions. */

			test_st = CStestB (verbose,duration);
			break;

		case 'C':
		case 'c':

			/* Perform test C. Tries to produce a
			   floating point exception error. */

			test_st = CStestC (verbose,duration);
			break;

		case 'D':
		case 'd':

			/* Perform test D. Test's each coordinate
			   system for reversibility. */

			test_st = CStestD (verbose,duration);
			break;

		case 'E':
		case 'e':

			/* Perform test E. Test's each coordinate
			   system for reversibility over a smaller
			   region, but with a smaller tolerance. */

			test_st = CStestE (verbose,duration);
			break;

		case 'F':
		case 'f':

			/* Perform test F. Exercises the CS_atof and
			   CS_ftoa functions, using one to test the
			   other. */

			test_st = CStestF (verbose,duration);
			break;

		case 'G':
		case 'g':

			/* Perform test G. Creep test on selected
			   projections. */

			test_st = CStestG (verbose,duration);
			break;

		case 'H':
		case 'h':

			/* Perform test H. Test miscellaneous functions. */

			test_st = CStestH (verbose,duration);
			break;

		case 'I':
		case 'i':

			/* Perform test I. Test WKT functions. */

			test_st = CStestI (verbose,duration);
			break;

		case 'J':
		case 'j':

			/* Perform test J. Test EPSG conversions. */

			test_st = CStestJ (verbose,duration);
			break;

		case 'K':
		case 'k':

			/* Perform test K. */
			test_st = CStestK (verbose,duration);
			break;

		case 'L':
		case 'l':

			/* Perform test L. */
			test_st = CStestL (verbose,duration);
			break;

		case 'R':
		case 'r':

			/* Not really a test.  Encountering this character in the test sequence
			   simply resets the environemnt.  That is, releases all memory and
			   file descriptors, and deletes all pre-parsed binary files. */
			CS_reset ();
			test_st = 0;
			break;

		case 'S':
		case 's':

			/* Swaps the byte order of binary data files,
			   then adjusts CS_bswap appropriately.  Not
			   a test in itself, but sets up the system
			   for running all other tests in the swapped
			   environment. */

			test_st = CStestS (verbose);
			if (test_st != 0)
			{
				printf ("Swap operation failed; status of binary files questionable!!!\n");
			}
			break;

		case 'T':
		case 't':

			/* Perform test T.  I.e. temporary code module. */
			test_st = CStestT (verbose,duration);
			break;

        /* Not really a test.  Encountering this character in the test sequence
           simply toggles the verbose flag on or off. */
		case 'V':
		case 'v':
			test_st = 0;
			verbose = ~verbose;
			break;

		default:

			test_st = 1;
			printf ("Test case %c not known.\n",*cp);
			break;
		}

		if (test_st != 0)
		{
			if (*cp == 'S')
			{
				printf ("File swap failed!!!\n");
			}
			else
			{
				bad_cnt += 1;
				printf ("Test %c failed (%d failures detected)!\n",*cp,test_st);
			}
		}
		else if (*cp != 'S' && *cp != 's' &&
		         *cp != 'V' && *cp != 'v' &&
		         *cp != 'R' && *cp != 'r')
		{
			ok_cnt += 1;
			if (verbose)
			{
				printf ("Test %c succeeded!!!\n",*cp);
			}
		}

		/* On to the next test. */

	}

	/* Delete any datum shift stuff which may be around.  Note that
	   CS_dtcls releases resources, but doesn't actually delete
	   the objects. */

	CS_recvr ();
	CS_reset ();

	/* See if a file open doesn't produce the same file
	   descriptor that we got the first time we did this. */

	if (initial_fd >= 0)
	{
		csStrm = CS_csopn (_STRM_BINRD);
		final_fd = _fileno (csStrm);
		CS_csDictCls (csStrm);
		if (final_fd != initial_fd)
		{
			printf ("There's a file descriptor leak somewhere.\n");
		}
	}

	printf ("%d tests completed successfully.\n",ok_cnt);
	if (bad_cnt != 0)
	{
		printf ("%d tests failed!!!!\n",bad_cnt);
	}

	if (!batch)
	{
		printf ("\rPress any key to continue: ");
		getchar ();
		printf ("\n");
	}
	return (0);
}
//newPage//
/* I thought matherrr was ANSI standard.  Maybe it isn't, as the code below
   does not appear to compile in all Unix environments. */
#if _RUN_TIME < _rt_UNIXPCC
#	if defined (_MSC_VER)
		int matherr (struct _exception *except)
#	else
		int matherr (struct exception *except)
#	endif
{
	extern int cs_MeFlag;
	extern char cs_MeKynm [];
	extern char cs_MeFunc [];
	extern double cs_Coords [3];

	char *type;

	cs_MeFlag = 1;
	switch (except->type) {
	case DOMAIN:
		type = "Domain";
		break;
	case SING:
		type = "Singularity";
		break;
	case OVERFLOW:
		type = "Overflow";
		break;
	case PLOSS:
		type = "Precision loss";
		break;
	case TLOSS:
		type = "Precision loss";
		break;
	case UNDERFLOW:
		type = "Underflow";
		break;
	default:
		type = "Unknown";
		break;
	}
	printf ("Math Error produced by %s :: %s %s(%g) [%s(%g:%g:%g)].\n",
				cs_MeKynm,
				type,
				except->name,
				except->arg1,
				cs_MeFunc,
				cs_Coords [0],
				cs_Coords [1],
				cs_Coords [2]);
	return (1);
}
#endif
