/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
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
struct InstanceReader final {
    struct JsonParams {
        private:
            bool m_abbreviateBlobs:1;
            bool m_classIdToClassNames:2;
            bool m_useJsName:3;
            bool m_indent:4;
        public:
            JsonParams():m_abbreviateBlobs(true),m_classIdToClassNames(true), m_useJsName(false), m_indent(false){}
            bool GetAbbreviateBlobs() const { return m_abbreviateBlobs;}
            bool GetClassIdToClassNames() const {return m_classIdToClassNames;}
            bool GetUseJsName() const {return m_useJsName; }
            bool GetIndent() const {return m_indent;}
            JsonParams& SetAbbreviateBlobs(bool v){ m_abbreviateBlobs = v; return *this; }
            JsonParams& SetClassIdToClassNames(bool v){ m_classIdToClassNames = v; return *this; }
            JsonParams& SetUseJsName(bool v){ m_useJsName = v; return *this; }
            JsonParams& SetIndent(bool v){ m_indent = v; return *this; }
            bool operator == (JsonParams const& rhs) const{
                if (this == &rhs) {
                    return true;
                }
                return
                    this->m_abbreviateBlobs == rhs.m_abbreviateBlobs &&
                    this->m_classIdToClassNames == rhs.m_classIdToClassNames &&
                    this->m_indent == rhs.m_indent &&
                    this->m_useJsName == rhs.m_useJsName;
            }
    };
    struct IRowContext : IECSqlRow {
       public:
            virtual BeJsValue GetJson(JsonParams const& param = JsonParams()) const = 0;
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
    using RowCallback = std::function<void(IRowContext const&)>;
    struct Impl;
    private:
        Impl* m_pImpl;
    public:
        InstanceReader(InstanceReader const&) = delete;
        InstanceReader& operator = (InstanceReader&) = delete;
        ECDB_EXPORT explicit InstanceReader(ECDbCR);
        ECDB_EXPORT ~InstanceReader();
        ECDB_EXPORT bool Seek(Position const&, RowCallback) const;
};

END_BENTLEY_SQLITE_EC_NAMESPACE
