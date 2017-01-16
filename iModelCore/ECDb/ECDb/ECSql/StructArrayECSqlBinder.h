/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructArrayECSqlBinder.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECSqlBinder.h"
#include <BeJsonCpp/BeJsonUtilities.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      01/2014
//+===============+===============+===============+===============+===============+======
struct StructArrayECSqlBinder : public ECSqlBinder
    {
private:

    struct JsonValueBinder : IECSqlBinder
        {
        private:
            ECDb const* m_ecdb = nullptr;
            ECSqlTypeInfo m_typeInfo;

            Json::Value* m_json = nullptr;
            //only relevant if binder is a struct binder
            std::map<ECN::ECPropertyId, JsonValueBinder> m_structMemberBinders;
            //only relevant if binder is an array binder
            std::unique_ptr<JsonValueBinder> m_currentArrayElementBinder;
            
            //only relevant if binder is a struct binder
            IECSqlBinder& CreateStructMemberBinder(ECN::ECPropertyCR);
            //only relevant if binder is an array binder
            IECSqlBinder& MoveCurrentArrayElementBinder(ECDbCR, ECSqlTypeInfo const& arrayTypeInfo, Json::Value& newArrayElementJson);
            
            ECSqlStatus FailIfTypeMismatch(ECN::PrimitiveType boundType) const;
            ECSqlStatus FailIfInvalid() const;

        public:
            JsonValueBinder() : IECSqlBinder() {}
            JsonValueBinder(ECDbCR ecdb, ECSqlTypeInfo const& typeInfo, Json::Value& json) : IECSqlBinder(), m_ecdb(&ecdb), m_typeInfo(typeInfo), m_json(&json), m_currentArrayElementBinder(nullptr) {}

            JsonValueBinder(JsonValueBinder&&);
            JsonValueBinder& operator=(JsonValueBinder&&);

            virtual ECSqlStatus _BindNull() override;
            virtual ECSqlStatus _BindBoolean(bool value) override;
            virtual ECSqlStatus _BindBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy) override;
            virtual ECSqlStatus _BindZeroBlob(int blobSize) override;
            virtual ECSqlStatus _BindDateTime(uint64_t julianDayMsec, DateTime::Info const&) override;
            virtual ECSqlStatus _BindDateTime(double julianDay, DateTime::Info const&) override;
            virtual ECSqlStatus _BindDouble(double value) override;
            virtual ECSqlStatus _BindInt(int value) override;
            virtual ECSqlStatus _BindInt64(int64_t value) override;
            virtual ECSqlStatus _BindPoint2d(DPoint2dCR) override;
            virtual ECSqlStatus _BindPoint3d(DPoint3dCR) override;
            virtual ECSqlStatus _BindText(Utf8CP stringValue, IECSqlBinder::MakeCopy makeCopy, int byteCount) override;

            virtual IECSqlBinder& _BindStructMember(Utf8CP structMemberPropertyName) override;
            virtual IECSqlBinder& _BindStructMember(ECN::ECPropertyId structMemberPropertyId) override;

            virtual IECSqlBinder& _AddArrayElement() override;

        };

    int m_sqliteIndex;
    Json::Value m_json;
    JsonValueBinder m_rootBinder;

    void Initialize();

    virtual void _SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteParameterIndex) override { m_sqliteIndex = (int) sqliteParameterIndex; }
    virtual void _OnClearBindings() override { Initialize(); }
    virtual ECSqlStatus _OnBeforeStep() override;

    virtual ECSqlStatus _BindNull() override { _OnClearBindings(); return ECSqlStatus::Success; }
    virtual ECSqlStatus _BindBoolean(bool value) override { return m_rootBinder.BindBoolean(value); }
    virtual ECSqlStatus _BindBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy) override { return m_rootBinder.BindBlob(value, blobSize, makeCopy); }
    virtual ECSqlStatus _BindZeroBlob(int blobSize) override { return m_rootBinder.BindZeroBlob(blobSize); }
    virtual ECSqlStatus _BindDateTime(uint64_t julianDayMsec, DateTime::Info const& dtInfo) override { return m_rootBinder.BindDateTime(julianDayMsec, dtInfo); }
    virtual ECSqlStatus _BindDateTime(double julianDay, DateTime::Info const& dtInfo) override { return m_rootBinder.BindDateTime(julianDay, dtInfo); }
    virtual ECSqlStatus _BindDouble(double value) override { return m_rootBinder.BindDouble(value); }
    virtual ECSqlStatus _BindInt(int value) override { return m_rootBinder.BindInt(value); }
    virtual ECSqlStatus _BindInt64(int64_t value) override { return m_rootBinder.BindInt64(value); }
    virtual ECSqlStatus _BindPoint2d(DPoint2dCR value) override { return m_rootBinder.BindPoint2d(value); }
    virtual ECSqlStatus _BindPoint3d(DPoint3dCR value) override { return m_rootBinder.BindPoint3d(value); }
    virtual ECSqlStatus _BindText(Utf8CP stringValue, IECSqlBinder::MakeCopy makeCopy, int byteCount) override { return m_rootBinder.BindText(stringValue, makeCopy, byteCount); }

    virtual IECSqlBinder& _BindStructMember(Utf8CP structMemberPropertyName) override { return m_rootBinder[structMemberPropertyName]; }
    virtual IECSqlBinder& _BindStructMember(ECN::ECPropertyId structMemberPropertyId) override { return m_rootBinder[structMemberPropertyId]; }

    virtual IECSqlBinder& _AddArrayElement() override { return m_rootBinder.AddArrayElement(); }

public:
    StructArrayECSqlBinder(ECSqlStatementBase&, ECSqlTypeInfo const&);
    ~StructArrayECSqlBinder() {}
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
