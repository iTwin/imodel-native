/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <BeSQLite/BeSQLite.h>
#include "SQLite/sqlite3.h"
#include <cstring>
#include <limits>

BEGIN_BENTLEY_SQLITE_NAMESPACE

struct CloudSqliteQueryStats {
    int64_t m_waitUs = 0;
    int64_t m_blocks = 0;
    int64_t m_bytes = 0;

    static int Read(sqlite3* db, CloudSqliteQueryStats& out) {
        Statement stmt;
        auto rc = sqlite3_prepare_v2(db,
            "SELECT name,value FROM main.bcv_stat WHERE name IN ('nqueryus','nqueryblock','nquerybyte')",
            -1, &stmt.GetStmtR(), nullptr);
        if (rc != SQLITE_OK)
            return rc;

        CloudSqliteQueryStats snapshot;
        unsigned seen = 0;
        while ((rc = sqlite3_step(stmt.GetStmtR())) == SQLITE_ROW) {
            auto name = stmt.GetValueText(0);
            if (!name)
                return SQLITE_NOMEM;
            unsigned bit = 0;
            int64_t* value = nullptr;
            if (strcmp(name, "nqueryus") == 0) {
                bit = 1;
                value = &snapshot.m_waitUs;
            } else if (strcmp(name, "nqueryblock") == 0) {
                bit = 2;
                value = &snapshot.m_blocks;
            } else if (strcmp(name, "nquerybyte") == 0) {
                bit = 4;
                value = &snapshot.m_bytes;
            }
            if (!value || (seen & bit) || sqlite3_column_type(stmt.GetStmtR(), 1) != SQLITE_INTEGER)
                return SQLITE_CORRUPT;
            *value = stmt.GetValueInt64(1);
            if (*value < 0)
                return SQLITE_CORRUPT;
            seen |= bit;
        }
        if (rc != SQLITE_DONE)
            return rc;
        if (seen != 7)
            return SQLITE_NOTFOUND; // Older daemons only supply nqueryms.
        out = snapshot;
        return SQLITE_OK;
    }

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
