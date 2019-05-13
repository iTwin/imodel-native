/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#ifndef winNT
#error "w32tools.h is not portable"
#endif

#include <Bentley/Bentley.h>
#include <Bentley/Bentley.r.h>
#include <Bentley/BeTextFile.h>
#include <stdio.h>
BEGIN_EXTERN_C

#if ! defined (___)
#  define ___       (0)
#endif

/*======================================================================+
|                                                                       |
|   Functions                                                           |
|                                                                       |
+======================================================================*/
int32_t win32Util_getCRuntimeMemFuncs (struct CRuntimeMemFuncs* memFuncsP);

DGNPLATFORM_EXPORT WCharCP win32Tools_exceptionToString
(
uint32_t exceptionCode
);

#if defined(_WINBASE_)
DGNPLATFORM_EXPORT void    win32Tools_dumpExceptionCallStack           // WIP - Must implement
(
BeTextFile*                      stream,
EXCEPTION_POINTERS const * const exceptionInfoP,
int32_t                    const debugLevel
);

int32_t   win32Tools_executeExternalProbe
(
FILE              * const fpStream,
WCharCP                   szProbeExeName,
STARTUPINFO const * const psiOption                 // => Optional STARTUPINFO for probe window visibility
);


#endif  // defined(_WINBASE_)

DGNPLATFORM_EXPORT int32_t    win32tools_processBSIExceptionLog        // WIP - Must implement
(
BeTextFile*             stream,                                 // => Optional
WChar   const * const   szDumpFile                              // => Optional
);

DGNPLATFORM_EXPORT void    win32Tools_generateMiniDump
(
BeTextFile*              stream,
EXCEPTION_POINTERS const * const exceptionInfoP,
WCharCP                  dmpFilePath,
bool*                    wantFullMemoryDump
);

DGNPLATFORM_EXPORT uint32_t win32Tools_resetFloatingPointExceptions
(
uint32_t newFpuMask
);

DGNPLATFORM_EXPORT Public bool    win32Tools_isRunningWindowsTerminalServer
(
void
);

DGNPLATFORM_EXPORT void     toolSubsystem_setThreadName     // Set thread name for VC6 debugger.
(
char const * const  sz9CharacterThreadName      // => Thread name, 9 characters + null max  // WIP_CHAR_OK thread names are what they are.
);

#if defined (INCLUDE_win32tools_recordDelayLoadHookFailure)
#  include    <delayimp.h>
DGNPLATFORM_EXPORT    void win32tools_recordDelayLoadHookFailure (uint32_t dliNotify,  PDelayLoadInfo pdli);
#endif

END_EXTERN_C
