/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include <ECDb/ECInstanceId.h>
#include <ECDb/IECSqlValue.h>
#include <ECDb/IECSqlBinder.h>
#include <ECDb/ECSqlStatement.h>
#include <ECDb/SchemaManager.h>
#include <list>
#include <json/json.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @internal Allow fast reading of full instance by its classid and instanceId
//=======================================================================================
struct InstanceWriter final {

    enum class Abortable : bool {
        Continue = false,
        Abort = true,
    };

    struct BaseInsertOrUpdateOptions {
        using UnknownJsPropertyHandler = std::function<Abortable(Utf8CP, BeJsConst)>;
        protected:
            bool m_useJsName;
            UnknownJsPropertyHandler m_unknownJsPropertyHandler;
        public:
            BaseInsertOrUpdateOptions():m_useJsName(false){}
            bool GetUseJsName() const {return m_useJsName; }
            BaseInsertOrUpdateOptions& UseJsName(bool v) {
                m_useJsName = v;
                return *this;
            }
            BaseInsertOrUpdateOptions& SetUnknownJsPropertyHandler(UnknownJsPropertyHandler handler) {
                m_unknownJsPropertyHandler = handler;
                return *this;
            }
            UnknownJsPropertyHandler GetUnknownJsPropertyHandler() const {return m_unknownJsPropertyHandler;}
    };

    struct UpdateOptions final: public BaseInsertOrUpdateOptions {
        private:
        public:
            UpdateOptions():BaseInsertOrUpdateOptions(){}

    };

    struct InsertOptions final: public BaseInsertOrUpdateOptions {
        private:
            ECInstanceId m_newInstanceId;
        public:
            InsertOptions(): BaseInsertOrUpdateOptions(){}
            ECInstanceId GetNewInstanceId() const {return m_newInstanceId;}
            InsertOptions& SetNewInstanceId(ECInstanceId id) {
                m_newInstanceId = id;
                return *this;
            }
    };

    struct DeleteOptions final {};
    struct Impl;
private:


    Impl* m_pImpl;
public:
    ECDB_EXPORT InstanceWriter(ECDbCR ecdb, uint32_t cacheSize = 40);
    ECDB_EXPORT ~InstanceWriter();
    ECDB_EXPORT Utf8StringCR GetLastError() const;
    ECDB_EXPORT DbResult Insert(BeJsConst inst, InsertOptions const& options);
    ECDB_EXPORT DbResult Update(BeJsConst inst, UpdateOptions const& options);
    ECDB_EXPORT DbResult Delete(BeJsConst inst, DeleteOptions const& options);
    ECDB_EXPORT void Reset();
};

END_BENTLEY_SQLITE_EC_NAMESPACE
