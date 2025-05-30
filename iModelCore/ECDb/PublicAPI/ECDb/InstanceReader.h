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

struct PropertyReader final {
    using Finder = std::function<std::optional<PropertyReader>(Utf8CP)>;

private:
    const IECSqlValue* m_reader;

public:
    PropertyReader(const IECSqlValue& reader) : m_reader(&reader) {}
    PropertyReader(PropertyReader const&) = default;
    PropertyReader& operator=(PropertyReader const&) = default;
    ECN::ECPropertyCR GetProperty() const { return *(m_reader->GetColumnInfo().GetProperty()); }
    const IECSqlValue& GetReader() const { return *m_reader; }
};

//=======================================================================================
//! @internal Allow fast reading of full instance by its classid and instanceId
//=======================================================================================
struct InstanceReader final {
    public:
    constexpr static unsigned FLAGS_UseJsPropertyNames = 0x1u;
    constexpr static unsigned FLAGS_DoNotTruncateBlobs = 0x2u;

    struct IRowContext : IECSqlRow {
       public:
            virtual BeJsValue GetJson(JsReadOptions const& param = JsReadOptions()) const = 0;
    };

    struct Position final {
        private:
            ECN::ECClassId m_classId;
            ECInstanceId m_instanceId;
            Utf8CP m_accessString;
            Utf8CP m_classFullName;
        public:
            Position(ECInstanceId instanceId,  ECN::ECClassId classId, Utf8CP accessString = nullptr):
                m_instanceId(instanceId), m_classId(classId), m_accessString(accessString),m_classFullName(nullptr){}
            Position(ECInstanceId instanceId, Utf8CP classFullName, Utf8CP accessString = nullptr):
                m_instanceId(instanceId), m_classFullName(classFullName), m_accessString(accessString){}
            ECN::ECClassId GetClassId() const { return m_classId; }
            ECInstanceId GetInstanceId() const { return m_instanceId; }
            Utf8CP GetAccessString() const { return m_accessString; }
            Utf8CP GetClassFullName() const { return m_classFullName; }
            Position Resolve(ECN::ECClassId classId) const {
                return Position(m_instanceId, classId, m_accessString) ;
            }
    };

    struct Options {
        private:
            bool m_forceSeek:1;
        public:
            Options() : m_forceSeek(false) {}
            bool GetForceSeek() const { return m_forceSeek; }
            void SetForceSeek(bool v) { m_forceSeek = v; }
    };

    using RowCallback = std::function<void(IRowContext const&, PropertyReader::Finder)>;
    struct Impl;
    private:
        Impl* m_pImpl;
    public:
        InstanceReader(InstanceReader const&) = delete;
        InstanceReader& operator = (InstanceReader&) = delete;
        ECDB_EXPORT explicit InstanceReader(ECDbCR);
        ECDB_EXPORT ~InstanceReader();
        ECDB_EXPORT bool Seek(Position const&, RowCallback, Options const& = Options()) const;
        ECDB_EXPORT void Reset();
        ECDB_EXPORT void InvalidateSeekPos(ECInstanceKey const& key = ECInstanceKey());
};

END_BENTLEY_SQLITE_EC_NAMESPACE
