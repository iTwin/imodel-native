/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include <ECDb/ECInstanceId.h>
#include <ECDb/ECSqlStatement.h>
#include <ECDb/IECSqlBinder.h>
#include <ECDb/IECSqlValue.h>
#include <ECDb/InstanceWriter.h>
#include <vector>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass
//=======================================================================================
struct InstanceRepository final {
private:
    ECDbCR m_ecdb;
    mutable BeMutex m_mutex;
    mutable Utf8String m_lastError;
public:
    InstanceRepository(ECDbCR ecdb) : m_ecdb(ecdb) {}
    ~InstanceRepository() = default;
    InstanceRepository(InstanceRepository const&) = delete;
    InstanceRepository(InstanceRepository&&) = delete;
    InstanceRepository& operator=(InstanceRepository const&) = delete;
    InstanceRepository& operator=(InstanceRepository&&) = delete;
    ECDB_EXPORT DbResult Insert(BeJsValue instance, BeJsConst userOptions, JsFormat inFmt, ECInstanceKeyR key) const;
    ECDB_EXPORT DbResult Update(BeJsValue instance, BeJsConst userOptions, JsFormat inFmt) const;
    ECDB_EXPORT DbResult Delete(BeJsConst key, BeJsConst userOptions, JsFormat inFmt) const;
    ECDB_EXPORT DbResult Delete(ECInstanceKeyCR key, BeJsConst userOptions, JsFormat inFmt) const;
    ECDB_EXPORT DbResult Read(BeJsConst key, BeJsValue out, BeJsConst userOptions, JsFormat fmt) const;
    ECDB_EXPORT DbResult Read(ECInstanceKeyCR key, BeJsValue out, BeJsConst userOptions, JsFormat fmt) const;

    //! Bulk write a JSON array of instances in a single call.
    //! The batch is all-or-nothing: if any row fails, no row of the batch is applied and
    //! @p failedIndex reports the index of the offending row (-1 if the failure was not row specific).
    //! Update and delete report the number of supplied instances that matched a row via @p affectedRows.
    ECDB_EXPORT DbResult BulkInsert(BeJsConst instances, BeJsConst userOptions, JsFormat inFmt, std::vector<ECInstanceKey>& keys, int& failedIndex) const;
    ECDB_EXPORT DbResult BulkUpdate(BeJsConst instances, BeJsConst userOptions, JsFormat inFmt, uint64_t& affectedRows, int& failedIndex) const;
    ECDB_EXPORT DbResult BulkDelete(BeJsConst keys, BeJsConst userOptions, JsFormat inFmt, uint64_t& affectedRows, int& failedIndex) const;

    Utf8StringCR GetLastError() const { return m_lastError; }

};


END_BENTLEY_SQLITE_EC_NAMESPACE
