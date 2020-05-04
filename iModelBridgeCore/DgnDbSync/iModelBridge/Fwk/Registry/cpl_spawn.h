/**********************************************************************
 * $Id: cpl_spawn.h 27044 2014-03-16 23:41:27Z rouault $
 *
 * Project:  CPL - Common Portability Library
 * Purpose:  Implement CPLSystem().
 * Author:   Even Rouault, <even dot rouault at mines dash paris dot org>
 *
 **********************************************************************
 * Copyright (c) 2013, Even Rouault <even dot rouault at mines-paris dot org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

#ifndef CPL_SPAWN_H_INCLUDED
#define CPL_SPAWN_H_INCLUDED

//#include "cpl_vsi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>

#define CPL_C_START
#define CPL_C_END

// ********************
// *** BENTLEY CHANGES: Pull in the few CPL macros that we actually need from various cpl_ header files
// ********************
#ifndef TRUE
#  define TRUE 1
#  define FALSE 0
#endif

#ifndef EQUAL
#if defined(WIN32) || defined(_WIN32) || defined(_WINDOWS)
#  define EQUALN(a,b,n)           (strnicmp(a,b,n)==0)
#  define EQUAL(a,b)              (stricmp(a,b)==0)
#else
#  define EQUALN(a,b,n)           (strncasecmp(a,b,n)==0)
#  define EQUAL(a,b)              (strcasecmp(a,b)==0)
#endif
#endif

#ifndef ABS
#define ABS(x)  (((x)>=0) ? (x) : -(x))
#endif

#ifndef MIN
#define MIN(x,y) (((x)<(y)) ? (x) : (y))
#endif

#define VSIFTellL ftell
#define VSIFOpenL _wfopen
#define VSIFCloseL fclose
#define VSIFReadL fread
#define VSIFWriteL fwrite
#define CPLMalloc malloc
#define CPLCalloc calloc
#define CPLFree free
#define GInt16  short
#define GByte   unsigned char
#define VSIFSeekL fseek
#define CPLAssert assert
#define VSIStrdup strdup

//CPL_C_START
#define CPL_DLL
typedef FILE VSILFILE;

/* -------------------------------------------------------------------- */
/*      Spawn a process.                                                */
/* -------------------------------------------------------------------- */

int CPL_DLL CPLSpawn( const wchar_t * const papszArgv[], VSILFILE* fin, VSILFILE* fout,
                      int bDisplayErr );

#include <windows.h>
typedef HANDLE CPL_FILE_HANDLE;
#define CPL_FILE_INVALID_HANDLE NULL
typedef DWORD  CPL_PID;

typedef struct _CPLSpawnedProcess CPLSpawnedProcess;

CPLSpawnedProcess CPL_DLL* CPLSpawnAsync( int (*pfnMain)(CPL_FILE_HANDLE, CPL_FILE_HANDLE),
                                          const wchar_t * const papszArgv[],
                                          int bCreateInputPipe,
                                          int bCreateOutputPipe,
                                          int bCreateErrorPipe);
int CPL_DLL CPLSpawnAsyncFinish(CPLSpawnedProcess* p, int bWait, int bKill);
CPL_FILE_HANDLE CPL_DLL CPLSpawnAsyncGetInputFileHandle(CPLSpawnedProcess* p);
CPL_FILE_HANDLE CPL_DLL CPLSpawnAsyncGetOutputFileHandle(CPLSpawnedProcess* p);
CPL_FILE_HANDLE CPL_DLL CPLSpawnAsyncGetErrorFileHandle(CPLSpawnedProcess* p);
void CPL_DLL CPLSpawnAsyncCloseInputFileHandle(CPLSpawnedProcess* p);
void CPL_DLL CPLSpawnAsyncCloseOutputFileHandle(CPLSpawnedProcess* p);
void CPL_DLL CPLSpawnAsyncCloseErrorFileHandle(CPLSpawnedProcess* p);

int CPL_DLL CPLPipeRead(CPL_FILE_HANDLE fin, void* data, int length);
int CPL_DLL CPLPipeWrite(CPL_FILE_HANDLE fout, const void* data, int length);


// *** BENTLEY CHANGES: Convenience functions
BentleyStatus CPL_DLL CPLPipeReadLine(Utf8String&, CPL_FILE_HANDLE);

//CPL_C_END

#endif // CPL_SPAWN_H_INCLUDED
