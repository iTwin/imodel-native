#ifndef __checkHndH__
#define __checkHndH__
/*__BENTLEY_INTERNAL_ONLY__*/
/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DesktopTools/checkhnd.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|       Definitions                                                     |
|                                                                       |
+----------------------------------------------------------------------*/
#ifdef _MSC_VER

#ifndef CHECKHND_GLE
#define CHECKHND_GLE uint32_t lastError = GetLastError()
#endif

#ifndef CHECKHND_OUTPUT
#define CHECKHND_OUTPUT(text) \
    WChar outMsg[512];\
    WChar errMsg[512];\
    memset(errMsg, 0, sizeof(errMsg));\
    memset(outMsg, 0, sizeof(outMsg));\
    FormatMessageW(0x1000, NULL, lastError, 0, errMsg, _countof(errMsg), NULL);\
    wsprintfW(outMsg, L"%ls (error=%d)\n%ls\n", text, lastError, errMsg); \
    OutputDebugStringW(outMsg)
#endif

#ifndef CHECKHND_DEBUG_BREAK
#if DEBUG_BREAK_ON_ERROR
#define CHECKHND_DEBUG_BREAK    if (2!=lastError) DebugBreak()
#else
#define CHECKHND_DEBUG_BREAK
#endif
#endif

#else /* POSIX */

#ifndef CHECKHND_GLE
#define CHECKHND_GLE extern int32_t errno; int32_t lastError = errno
#endif

#ifndef CHECKHND_OUTPUT
#define CHECKHND_OUTPUT(text) perror(text)
#endif

#ifndef CHECKHND_DEBUG_BREAK
#if defined (__GNUC__) && defined (__i386__) && DEBUG_BREAK_ON_ERROR
#define CHECKHND_DEBUG_BREAK    __asm__ __volatile__ ("int3")
#else
#define CHECKHND_DEBUG_BREAK
#endif
#endif

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)(-1))
#endif /* INVALID_HANDLE_VALUE */
#include <errno.h>

#endif /* POSIX */

#define CHECK_HANDLE(handle, text, status) if ((NULL == (handle)) || (INVALID_HANDLE_VALUE == (handle)))\
    {\
    CHECKHND_GLE; \
    CHECKHND_OUTPUT(text); \
    CHECKHND_DEBUG_BREAK; \
    if (appStatusP)\
        *appStatusP = status;\
    if (sysStatusP)\
        *sysStatusP = lastError;\
    }

#define CHECK_HANDLE_RETURN_NULL(handle, text, status) if ((NULL == (handle)) || (INVALID_HANDLE_VALUE == (handle)))\
    {\
    CHECKHND_GLE; \
    CHECKHND_OUTPUT(text); \
    CHECKHND_DEBUG_BREAK; \
    if (appStatusP)\
        *appStatusP = status;\
    if (sysStatusP)\
        *sysStatusP = lastError;\
    return NULL;\
    }

#define CHECK_HANDLE_RETURN_ZERO(handle, text, status) if ((NULL == (handle)) || (INVALID_HANDLE_VALUE == (handle)))\
    {\
    CHECKHND_GLE; \
    CHECKHND_OUTPUT(text); \
    CHECKHND_DEBUG_BREAK; \
    if (appStatusP)\
        *appStatusP = status;\
    if (sysStatusP)\
        *sysStatusP = lastError;\
    return 0;\
    }

#define CHECK_HANDLE_RETURN(handle, text, status) if ((NULL == (handle)) || (INVALID_HANDLE_VALUE == (handle)))\
    {\
    CHECKHND_GLE; \
    CHECKHND_OUTPUT(text); \
    CHECKHND_DEBUG_BREAK; \
    if (appStatusP)\
        *appStatusP = status;\
    if (sysStatusP)\
        *sysStatusP = lastError;\
    return status;\
    }

#define CHECK_HANDLE_LEAVE(handle, text, status) if ((NULL == (handle)) || (INVALID_HANDLE_VALUE == (handle)))\
    {\
    CHECKHND_GLE; \
    CHECKHND_OUTPUT(text); \
    CHECKHND_DEBUG_BREAK; \
    if (appStatusP)\
        *appStatusP = status;\
    if (sysStatusP)\
        *sysStatusP = lastError;\
    goto mycheck_leave;\
    }

#define CHECK_BOOL_LEAVE(ok, text, status) if (!(ok))\
    {\
    CHECKHND_GLE; \
    CHECKHND_OUTPUT(text); \
    CHECKHND_DEBUG_BREAK; \
    if (appStatusP)\
        *appStatusP = status;\
    if (sysStatusP)\
        *sysStatusP = lastError;\
    goto mycheck_leave;\
    }

#endif /* __checkHndH__ */
