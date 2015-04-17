/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DesktopTools/w32tools.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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

DESKTOP_TOOLS_EXPORT WCharCP win32Tools_exceptionToString
(
uint32_t exceptionCode
);

#if defined(_WINBASE_)
void    win32Tools_dumpExceptionCallStack           // WIP - Must implement
(
BeTextFilePtr                    stream,
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

DESKTOP_TOOLS_EXPORT int32_t    win32tools_processBSIExceptionLog        // WIP - Must implement
(
BeTextFilePtr           stream,                                 // => Optional
WChar   const * const   szDumpFile                              // => Optional
);

DESKTOP_TOOLS_EXPORT void    win32Tools_generateMiniDump
(
FILE                     * const stream,
EXCEPTION_POINTERS const * const exceptionInfoP,
WCharCP                  dmpFilePath
);

DESKTOP_TOOLS_EXPORT uint32_t win32Tools_resetFloatingPointExceptions
(
uint32_t newFpuMask
);

DESKTOP_TOOLS_EXPORT Public bool    win32Tools_isRunningWindowsTerminalServer
(
void
);

DESKTOP_TOOLS_EXPORT void     toolSubsystem_setThreadName     // Set thread name for VC6 debugger.
(
char const * const  sz9CharacterThreadName      // => Thread name, 9 characters + null max  // WIP_CHAR_OK thread names are what they are.
);

#if defined (INCLUDE_win32tools_recordDelayLoadHookFailure)
#  include    <delayimp.h>
DESKTOP_TOOLS_EXPORT    void win32tools_recordDelayLoadHookFailure (uint32_t dliNotify,  PDelayLoadInfo pdli);
#endif

END_EXTERN_C
