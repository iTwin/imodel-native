/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <BeSQLite/BeSQLite.h>
#include "SQLite/sqlite3.h"
#include <limits>

BEGIN_BENTLEY_SQLITE_NAMESPACE

struct CloudSqliteQueryStats {
    int64_t m_waitUs = 0;
    int64_t m_blocks = 0;
    int64_t m_bytes = 0;

    static BE_SQLITE_EXPORT int Read(sqlite3* db, CloudSqliteQueryStats& out);

    static bool Delta(CloudSqliteQueryStats const& before, CloudSqliteQueryStats const& after, CloudSqliteQueryStats& out) {
        if (before.m_waitUs < 0 || before.m_blocks < 0 || before.m_bytes < 0 ||
            after.m_waitUs < before.m_waitUs || after.m_blocks < before.m_blocks || after.m_bytes < before.m_bytes)
            return false;
        out = {after.m_waitUs - before.m_waitUs, after.m_blocks - before.m_blocks, after.m_bytes - before.m_bytes};
        return true;
    }

    bool Accumulate(CloudSqliteQueryStats const& delta) {
        auto canAdd = [](int64_t value, int64_t increment) {
            return value >= 0 && increment >= 0 && value <= std::numeric_limits<int64_t>::max() - increment;
        };
        if (!canAdd(m_waitUs, delta.m_waitUs) || !canAdd(m_blocks, delta.m_blocks) || !canAdd(m_bytes, delta.m_bytes))
            return false;
        m_waitUs += delta.m_waitUs;
        m_blocks += delta.m_blocks;
        m_bytes += delta.m_bytes;
        return true;
    }
};

END_BENTLEY_SQLITE_NAMESPACE
