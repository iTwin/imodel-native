/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

/*
* MonotoneClock.cpp - A SQLite VFS shim that overrides the time functions to guarantee
* strictly monotonically increasing values across all calls, regardless of system clock
* resolution. This is useful in tests where SQLite's millisecond-resolution clock may
* return the same value for two consecutive operations.
*
* Based on the vfstrace shim pattern from SQLite's ext/misc/vfstrace.c: all VFS calls
* are forwarded verbatim to the underlying (root) VFS except xCurrentTime and
* xCurrentTimeInt64, which are intercepted to enforce strict monotonicity.
*/

#include <BeSQLite/BeSQLite.h>
#include "SQLite/sqlite3.h"
#include <Bentley/BeThread.h>

USING_NAMESPACE_BENTLEY

BEGIN_BENTLEY_SQLITE_NAMESPACE

//=======================================================================================
//! State carried in the monotone-clock VFS shim's pAppData pointer.
//=======================================================================================
struct MonotoneClockInfo
    {
    sqlite3_vfs*  m_rootVfs;         //!< The real underlying VFS we wrap.
    BeMutex       m_mutex;           //!< Guards m_lastTime64 for thread safety.
    sqlite3_int64 m_lastTime64 = 0;  //!< Last value returned by xCurrentTimeInt64.
    };

/*---------------------------------------------------------------------------------**//**
* Forward xOpen to the root VFS unchanged. Because szOsFile matches the root VFS, the
* root will initialise pFile->pMethods with its own io methods, so subsequent file
* operations bypass this shim entirely.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int MonotoneOpen(sqlite3_vfs* pVfs, const char* zName, sqlite3_file* pFile, int flags, int* pOutFlags)
    {
    auto* info = (MonotoneClockInfo*) pVfs->pAppData;
    return info->m_rootVfs->xOpen(info->m_rootVfs, zName, pFile, flags, pOutFlags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int MonotoneDelete(sqlite3_vfs* pVfs, const char* zName, int syncDir)
    {
    auto* info = (MonotoneClockInfo*) pVfs->pAppData;
    return info->m_rootVfs->xDelete(info->m_rootVfs, zName, syncDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int MonotoneAccess(sqlite3_vfs* pVfs, const char* zName, int flags, int* pResOut)
    {
    auto* info = (MonotoneClockInfo*) pVfs->pAppData;
    return info->m_rootVfs->xAccess(info->m_rootVfs, zName, flags, pResOut);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int MonotoneFullPathname(sqlite3_vfs* pVfs, const char* zName, int nOut, char* zOut)
    {
    auto* info = (MonotoneClockInfo*) pVfs->pAppData;
    return info->m_rootVfs->xFullPathname(info->m_rootVfs, zName, nOut, zOut);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void* MonotoneDlOpen(sqlite3_vfs* pVfs, const char* zFilename)
    {
    auto* info = (MonotoneClockInfo*) pVfs->pAppData;
    return info->m_rootVfs->xDlOpen(info->m_rootVfs, zFilename);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void MonotoneDlError(sqlite3_vfs* pVfs, int nByte, char* zErrMsg)
    {
    auto* info = (MonotoneClockInfo*) pVfs->pAppData;
    info->m_rootVfs->xDlError(info->m_rootVfs, nByte, zErrMsg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void (*MonotoneDlSym(sqlite3_vfs* pVfs, void* pHdl, const char* zSym))(void)
    {
    auto* info = (MonotoneClockInfo*) pVfs->pAppData;
    return info->m_rootVfs->xDlSym(info->m_rootVfs, pHdl, zSym);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void MonotoneDlClose(sqlite3_vfs* pVfs, void* pHdl)
    {
    auto* info = (MonotoneClockInfo*) pVfs->pAppData;
    info->m_rootVfs->xDlClose(info->m_rootVfs, pHdl);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int MonotoneRandomness(sqlite3_vfs* pVfs, int nByte, char* zOut)
    {
    auto* info = (MonotoneClockInfo*) pVfs->pAppData;
    return info->m_rootVfs->xRandomness(info->m_rootVfs, nByte, zOut);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int MonotoneSleep(sqlite3_vfs* pVfs, int microseconds)
    {
    auto* info = (MonotoneClockInfo*) pVfs->pAppData;
    return info->m_rootVfs->xSleep(info->m_rootVfs, microseconds);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int MonotoneGetLastError(sqlite3_vfs* pVfs, int n, char* zBuf)
    {
    auto* info = (MonotoneClockInfo*) pVfs->pAppData;
    return info->m_rootVfs->xGetLastError(info->m_rootVfs, n, zBuf);
    }

/*---------------------------------------------------------------------------------**//**
* Returns the current time as a Julian Day Number (× 86400000) in milliseconds.
* Guarantees that successive calls never return the same value: if the real clock has
* not advanced since the previous call, the returned value is simply incremented by one
* millisecond rather than spinning/sleeping, keeping tests fast.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int MonotoneCurrentTimeInt64(sqlite3_vfs* pVfs, sqlite3_int64* pOut)
    {
    auto* info = (MonotoneClockInfo*) pVfs->pAppData;
    BeMutexHolder lock(info->m_mutex);

    sqlite3_int64 t = 0;
    int rc = info->m_rootVfs->xCurrentTimeInt64(info->m_rootVfs, &t);
    if (rc != SQLITE_OK)
        return rc;

    // Ensure strict monotonicity without sleeping — just advance by 1 ms if needed.
    if (t <= info->m_lastTime64)
        t = info->m_lastTime64 + 1;

    info->m_lastTime64 = t;
    *pOut = t;
    return SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* Returns the current time as a Julian Day Number (double). Delegates to
* MonotoneCurrentTimeInt64 so both functions share the same monotone counter.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int MonotoneCurrentTime(sqlite3_vfs* pVfs, double* pOut)
    {
    sqlite3_int64 t = 0;
    int rc = MonotoneCurrentTimeInt64(pVfs, &t);
    if (rc == SQLITE_OK)
        *pOut = t / 86400000.0;  // Convert ms-per-Julian-day back to Julian day double
    return rc;
    }

//=======================================================================================
//! VFS template; szOsFile and mxPathname are filled in from the root VFS at creation time.
//=======================================================================================
static sqlite3_vfs const s_monotoneVfsTemplate =
    {
    2,                           /* iVersion (must be >= 2 for xCurrentTimeInt64) */
    0,                           /* szOsFile – filled in from root VFS */
    0,                           /* mxPathname – filled in from root VFS */
    nullptr,                     /* pNext (managed by SQLite) */
    "besqlite_monotone",         /* zName */
    nullptr,                     /* pAppData – filled in at creation */
    MonotoneOpen,
    MonotoneDelete,
    MonotoneAccess,
    MonotoneFullPathname,
    MonotoneDlOpen,
    MonotoneDlError,
    MonotoneDlSym,
    MonotoneDlClose,
    MonotoneRandomness,
    MonotoneSleep,
    MonotoneCurrentTime,
    MonotoneGetLastError,
    MonotoneCurrentTimeInt64,
    nullptr,                     /* xSetSystemCall – not needed */
    nullptr,                     /* xGetSystemCall – not needed */
    nullptr,                     /* xNextSystemCall – not needed */
    };

// The single registered instance, accessed atomically so concurrent Enable/Disable
// calls are serialised without a static mutex (whose destruction order is unspecified
// on Mac/Linux and can cause crashes at process exit).
// The sentinel value (1) indicates that a transition is in progress; a second thread
// seeing the sentinel bails out immediately without waiting.
static sqlite3_vfs* const s_vfsTransitionSentinel = reinterpret_cast<sqlite3_vfs*>(1);
static std::atomic<sqlite3_vfs*> s_monotoneVfs{nullptr};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BE_SQLITE_EXPORT void BeSQLiteLib::EnableMonotoneClock(bool enable)
    {
    if (enable)
        {
        // Atomically claim the "not active" state by swapping in the sentinel.
        // If the current value is anything other than null (already active or another
        // thread is transitioning), do nothing.
        sqlite3_vfs* expected = nullptr;
        if (!s_monotoneVfs.compare_exchange_strong(expected, s_vfsTransitionSentinel,
                                                   std::memory_order_acquire,
                                                   std::memory_order_relaxed))
            return;  // already active or being activated concurrently

        sqlite3_vfs* rootVfs = sqlite3_vfs_find(nullptr); // current default VFS
        if (rootVfs == nullptr)
            {
            s_monotoneVfs.store(nullptr, std::memory_order_release);
            return;
            }

        auto* info = new MonotoneClockInfo();
        info->m_rootVfs = rootVfs;

        auto* vfs = new sqlite3_vfs(s_monotoneVfsTemplate);
        vfs->szOsFile   = rootVfs->szOsFile;
        vfs->mxPathname = rootVfs->mxPathname;
        vfs->pAppData   = info;

        if (SQLITE_OK != sqlite3_vfs_register(vfs, 1 /* makeDefault */))
            {
            // Registration failed; clean up and restore inactive state.
            delete info;
            delete vfs;
            s_monotoneVfs.store(nullptr, std::memory_order_release);
            return;
            }

        s_monotoneVfs.store(vfs, std::memory_order_release);
        }
    else
        {
        // Atomically claim whatever valid VFS pointer is stored, replacing it with
        // the sentinel so no other thread can race with our unregistration.
        sqlite3_vfs* vfs = s_monotoneVfs.load(std::memory_order_acquire);
        for (;;)
            {
            if (vfs == nullptr || vfs == s_vfsTransitionSentinel)
                return;  // not active, or another thread is transitioning
            if (s_monotoneVfs.compare_exchange_weak(vfs, s_vfsTransitionSentinel,
                                                    std::memory_order_acquire,
                                                    std::memory_order_relaxed))
                break;
            }

        // We exclusively own vfs now.
        auto* info = (MonotoneClockInfo*) vfs->pAppData;
        sqlite3_vfs* rootVfs = info->m_rootVfs;

        if (SQLITE_OK != sqlite3_vfs_unregister(vfs))
            {
            // Unregistration failed; restore the pointer and leave state unchanged.
            s_monotoneVfs.store(vfs, std::memory_order_release);
            return;
            }

        // Re-promote the original root VFS as the default.
        // Even if this fails the monotone shim is already gone, so we still clean up.
        (void) sqlite3_vfs_register(rootVfs, 1 /* makeDefault */);

        delete info;
        delete vfs;
        s_monotoneVfs.store(nullptr, std::memory_order_release);
        }
    }

END_BENTLEY_SQLITE_NAMESPACE
