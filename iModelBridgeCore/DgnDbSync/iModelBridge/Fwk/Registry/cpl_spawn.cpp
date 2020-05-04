/**********************************************************************
 * $Id: cpl_spawn.cpp 27722 2014-09-22 15:37:31Z goatbar $
 *
 * Project:  CPL - Common Portability Library
 * Purpose:  Implement CPLSystem().
 * Author:   Even Rouault, <even dot rouault at mines dash paris dot org>
 *
 **********************************************************************
 * Copyright (c) 2012-2013, Even Rouault <even dot rouault at mines-paris dot org>
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

#include "cpl_spawn.h"

// ********************
// *** BENTLEY CHANGES: Pull in the few CPL macros that we actually need from various cpl_ header files
// ********************
#define CPL_UNUSED
#define CPL_STDCALL
#define CPLString WString
#define CPLFree free
#define CPLStrdup _wcsdup

#define CPL_FRMT_GIB     L"%" CPL_FRMT_GB_WITHOUT_PREFIX L"d"
#define CPL_FRMT_GUIB    L"%" CPL_FRMT_GB_WITHOUT_PREFIX L"u"

#define CPL_FRMT_GB_WITHOUT_PREFIX     L"I64"

#define CPLE_None                       0
#define CPLE_AppDefined                 1
#define CPLE_OutOfMemory                2
#define CPLE_FileIO                     3
#define CPLE_OpenFailed                 4
#define CPLE_IllegalArg                 5
#define CPLE_NotSupported               6
#define CPLE_AssertionFailed            7
#define CPLE_NoWriteAccess              8
#define CPLE_UserInterrupt              9
#define CPLE_ObjectNull                 10

#define PIPE_BUFFER_SIZE    4096

#define IN_FOR_PARENT   0
#define OUT_FOR_PARENT  1

typedef enum
{
    CE_None = 0,
    CE_Debug = 1,
    CE_Warning = 2,
    CE_Failure = 3,
    CE_Fatal = 4
} CPLErr;


static void FillFileFromPipe(CPL_FILE_HANDLE pipe_fd, VSILFILE* fout);

// ********************
// *** BENTLEY CHANGES: Pull in the few CPL utility functions that we actually need from various cpl_ header files
// ********************

void CPL_DLL CPLError(CPLErr eErrClass, int err_no, const wchar_t *fmt, ...)
    {
    BeAssert(false);
    }


uint64_t CPLGetPID()
    {
    return (uint64_t) GetCurrentProcessId();
    }


int CSLCount(wchar_t **papszStrList)
{
    int nItems=0;

    if (papszStrList)
    {
        while(*papszStrList != NULL)
        {
            nItems++;
            papszStrList++;
        }
    }

    return nItems;
}

void CPL_STDCALL CSLDestroy(wchar_t **papszStrList)
{
    wchar_t **papszPtr;

    if (papszStrList)
    {
        papszPtr = papszStrList;
        while(*papszPtr != NULL)
        {
            CPLFree(*papszPtr);
            papszPtr++;
        }

        CPLFree(papszStrList);
    }
}

wchar_t **CSLDuplicate(wchar_t **papszStrList)
{
    wchar_t **papszNewList, **papszSrc, **papszDst;
    int  nLines;

    nLines = CSLCount(papszStrList);

    if (nLines == 0)
        return NULL;

    papszNewList = (wchar_t **)CPLMalloc((nLines+1)*sizeof(wchar_t*));
    papszSrc = papszStrList;
    papszDst = papszNewList;

    while(*papszSrc != NULL)
    {
        *papszDst = CPLStrdup(*papszSrc);

        papszSrc++;
        papszDst++;
    }
    *papszDst = NULL;

    return papszNewList;
}

// ********************
// *** BENTLEY CHANGES: Convenience functions
// ********************

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CPLPipeReadLine(Utf8String& line, CPL_FILE_HANDLE h)
    {
    char ch;
    for(;;)
        {
        if (!CPLPipeRead(h, &ch, 1)) // NB: CPLPipeRead takes a count of bytes
            return BSIERROR;
        if (ch == '\n')
            break;
        if (ch == '\r')
            continue;
        line.append(1, ch);
        }
    return BSISUCCESS;
    }

/************************************************************************/
/*                          CPLPipeRead()                               */
/************************************************************************/

int CPLPipeRead(CPL_FILE_HANDLE fin, void* data, int length)
{
    GByte* pabyData = (GByte*)data;
    int nRemain = length;
    while( nRemain > 0 )
    {
        DWORD nRead = 0;
        if (!ReadFile( fin, pabyData, nRemain, &nRead, NULL))
            return FALSE;
        pabyData += nRead;
        nRemain -= nRead;
    }
    return TRUE;
}

/************************************************************************/
/*                         CPLPipeWrite()                               */
/************************************************************************/

int CPLPipeWrite(CPL_FILE_HANDLE fout, const void* data, int length)
{
    const GByte* pabyData = (const GByte*)data;
    int nRemain = length;
    while( nRemain > 0 )
    {
        DWORD nWritten = 0;
        if (!WriteFile(fout, pabyData, nRemain, &nWritten, NULL))
            return FALSE;
        pabyData += nWritten;
        nRemain -= nWritten;
    }
    return TRUE;
}

struct _CPLSpawnedProcess
{
    HANDLE hProcess;
    DWORD  nProcessId;
    HANDLE hThread;
    CPL_FILE_HANDLE fin;
    CPL_FILE_HANDLE fout;
    CPL_FILE_HANDLE ferr;
};

/************************************************************************/
/*                            CPLSpawnAsync()                           */
/************************************************************************/

CPLSpawnedProcess* CPLSpawnAsync(int (*pfnMain)(CPL_FILE_HANDLE, CPL_FILE_HANDLE),
                                 const wchar_t * const papszArgv[],
                                 int bCreateInputPipe,
                                 int bCreateOutputPipe,
                                 int bCreateErrorPipe)
{
    HANDLE pipe_in[2] = {NULL, NULL};
    HANDLE pipe_out[2] = {NULL, NULL};
    HANDLE pipe_err[2] = {NULL, NULL};
    SECURITY_ATTRIBUTES saAttr;
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFOW siStartInfo;
    CPLString osCommandLine;
    int i;
    CPLSpawnedProcess* p = NULL;

    if( papszArgv == NULL )
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 L"On Windows, papszArgv argument must not be NULL");
        return NULL;
    }

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if( bCreateInputPipe )
    {
        if (!CreatePipe(&pipe_in[IN_FOR_PARENT],&pipe_in[OUT_FOR_PARENT],&saAttr, 0))
            goto err_pipe;
        /* The child must not inherit from the write side of the pipe_in */
        if (!SetHandleInformation(pipe_in[OUT_FOR_PARENT],HANDLE_FLAG_INHERIT,0))
            goto err_pipe;
    }

    if( bCreateOutputPipe )
    {
        if (!CreatePipe(&pipe_out[IN_FOR_PARENT],&pipe_out[OUT_FOR_PARENT],&saAttr, 0))
            goto err_pipe;
        /* The child must not inherit from the read side of the pipe_out */
        if (!SetHandleInformation(pipe_out[IN_FOR_PARENT],HANDLE_FLAG_INHERIT,0))
            goto err_pipe;
    }

    if( bCreateErrorPipe )
    {
        if (!CreatePipe(&pipe_err[IN_FOR_PARENT],&pipe_err[OUT_FOR_PARENT],&saAttr, 0))
            goto err_pipe;
        /* The child must not inherit from the read side of the pipe_err */
        if (!SetHandleInformation(pipe_err[IN_FOR_PARENT],HANDLE_FLAG_INHERIT,0))
            goto err_pipe;
    }

    memset(&piProcInfo, 0, sizeof(PROCESS_INFORMATION));
    memset(&siStartInfo, 0, sizeof(STARTUPINFOW));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdInput = (bCreateInputPipe) ? pipe_in[IN_FOR_PARENT] : GetStdHandle(STD_INPUT_HANDLE);
    siStartInfo.hStdOutput = (bCreateOutputPipe) ? pipe_out[OUT_FOR_PARENT] : GetStdHandle(STD_OUTPUT_HANDLE);
    siStartInfo.hStdError = (bCreateErrorPipe) ? pipe_err[OUT_FOR_PARENT] : GetStdHandle(STD_ERROR_HANDLE);
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    for(i=0;papszArgv[i] != NULL;i++)
    {
        if (i > 0)
            osCommandLine += L" ";
        /* We need to quote arguments with spaces in them (if not already done) */
        if( wcschr(papszArgv[i], L' ') != NULL &&
            papszArgv[i][0] != L'"' )
        {
            osCommandLine += L"\"";
            osCommandLine += papszArgv[i];
            osCommandLine += L"\"";
        }
        else
            osCommandLine += papszArgv[i];
    }

    if (!CreateProcessW(NULL,
                       (wchar_t*)osCommandLine.c_str(),
                       NULL,          // process security attributes
                       NULL,          // primary thread security attributes
                       TRUE,          // handles are inherited
                       CREATE_NO_WINDOW|NORMAL_PRIORITY_CLASS,             // creation flags
                       NULL,          // use parent's environment
                       NULL,          // use parent's current directory
                       &siStartInfo,
                       &piProcInfo))
    {
        CPLError(CE_Failure, CPLE_AppDefined, L"Could not create process %ls",
                 osCommandLine.c_str());
        goto err;
    }

    /* Close unused end of pipe */
    if( bCreateInputPipe )
        CloseHandle(pipe_in[IN_FOR_PARENT]);
    if( bCreateOutputPipe )
        CloseHandle(pipe_out[OUT_FOR_PARENT]);
    if( bCreateErrorPipe )
        CloseHandle(pipe_err[OUT_FOR_PARENT]);

    p = (CPLSpawnedProcess*)CPLMalloc(sizeof(CPLSpawnedProcess));
    p->hProcess = piProcInfo.hProcess;
    p->nProcessId = piProcInfo.dwProcessId;
    p->hThread = piProcInfo.hThread;
    p->fin = pipe_out[IN_FOR_PARENT];
    p->fout = pipe_in[OUT_FOR_PARENT];
    p->ferr = pipe_err[IN_FOR_PARENT];
    return p;

err_pipe:
    CPLError(CE_Failure, CPLE_AppDefined, L"Could not create pipe");
err:
    for(i=0;i<2;i++)
    {
        if (pipe_in[i] != NULL)
            CloseHandle(pipe_in[i]);
        if (pipe_out[i] != NULL)
            CloseHandle(pipe_out[i]);
        if (pipe_err[i] != NULL)
            CloseHandle(pipe_err[i]);
    }

    return NULL;
}

/************************************************************************/
/*                        CPLSpawnAsyncFinish()                         */
/************************************************************************/

int CPLSpawnAsyncFinish(CPLSpawnedProcess* p, int bWait, int bKill)
{
    // Get the exit code.
    DWORD exitCode = -1;

    if( bWait )
    {
        WaitForSingleObject( p->hProcess, INFINITE );
        GetExitCodeProcess(p->hProcess, &exitCode);
    }
    else
        exitCode = 0;

    CloseHandle(p->hProcess);
    CloseHandle(p->hThread);

    CPLSpawnAsyncCloseInputFileHandle(p);
    CPLSpawnAsyncCloseOutputFileHandle(p);
    CPLSpawnAsyncCloseErrorFileHandle(p);
    CPLFree(p);

    return (int)exitCode;
}

/************************************************************************/
/*                 CPLSpawnAsyncCloseInputFileHandle()                  */
/************************************************************************/

void CPLSpawnAsyncCloseInputFileHandle(CPLSpawnedProcess* p)
{
    if( p->fin != NULL )
        CloseHandle(p->fin);
    p->fin = NULL;
}

/************************************************************************/
/*                 CPLSpawnAsyncCloseOutputFileHandle()                 */
/************************************************************************/

void CPLSpawnAsyncCloseOutputFileHandle(CPLSpawnedProcess* p)
{
    if( p->fout != NULL )
        CloseHandle(p->fout);
    p->fout = NULL;
}

/************************************************************************/
/*                 CPLSpawnAsyncCloseErrorFileHandle()                  */
/************************************************************************/

void CPLSpawnAsyncCloseErrorFileHandle(CPLSpawnedProcess* p)
{
    if( p->ferr != NULL )
        CloseHandle(p->ferr);
    p->ferr = NULL;
}

/************************************************************************/
/*                    CPLSpawnAsyncGetInputFileHandle()                 */
/************************************************************************/

/**
 * Return the file handle of the standard output of the forked process
 * from which to read.
 *
 * @param p handle returned by CPLSpawnAsync().
 *
 * @return the file handle.
 *
 * @since GDAL 1.10.0
 */
CPL_FILE_HANDLE CPLSpawnAsyncGetInputFileHandle(CPLSpawnedProcess* p)
{
    return p->fin;
}

/************************************************************************/
/*                   CPLSpawnAsyncGetOutputFileHandle()                 */
/************************************************************************/

/**
 * Return the file handle of the standard input of the forked process
 * into which to write
 *
 * @param p handle returned by CPLSpawnAsync().
 *
 * @return the file handle.
 *
 * @since GDAL 1.10.0
 */
CPL_FILE_HANDLE CPLSpawnAsyncGetOutputFileHandle(CPLSpawnedProcess* p)
{
    return p->fout;
}

/************************************************************************/
/*                    CPLSpawnAsyncGetErrorFileHandle()                 */
/************************************************************************/

/**
 * Return the file handle of the standard error of the forked process
 * from which to read.
 *
 * @param p handle returned by CPLSpawnAsync().
 *
 * @return the file handle
 *
 * @since GDAL 1.10.0
 */
CPL_FILE_HANDLE CPLSpawnAsyncGetErrorFileHandle(CPLSpawnedProcess* p)
{
    return p->ferr;
}
