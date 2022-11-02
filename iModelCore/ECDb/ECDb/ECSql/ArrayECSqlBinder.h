/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlBinder.h"
#include <BeRapidJson/BeRapidJson.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct ArrayECSqlBinder final : public ECSqlBinder
    {
private:

    struct JsonValueBinder final : IECSqlBinder
        {
        private:
            ECDb const* m_ecdb = nullptr;
            ECSqlTypeInfo m_typeInfo;

            rapidjson::Value* m_json = nullptr;
            rapidjson::MemoryPoolAllocator<>* m_jsonAllocator = nullptr;
            //only relevant if binder is a struct binder
            std::map<ECN::ECPropertyId, JsonValueBinder> m_structMemberBinders;
            //only relevant if binder is an array binder
            std::unique_ptr<JsonValueBinder> m_currentArrayElementBinder;
            
            //only relevant if binder is a struct binder
            IECSqlBinder& CreateStructMemberBinder(ECN::ECPropertyCR);
            //only relevant if binder is an array binder
            IECSqlBinder& MoveCurrentArrayElementBinder(ECDbCR, ECSqlTypeInfo const& arrayTypeInfo);
            
            ECSqlStatus FailIfTypeMismatch(ECN::PrimitiveType boundType) const;
            ECSqlStatus FailIfInvalid() const;

            void Reset(rapidjson::Value& newJsonValue) { m_json = &newJsonValue; Reset(); }
            void Reset();

        public:
            JsonValueBinder(ECDbCR ecdb, ECSqlTypeInfo const& typeInfo, rapidjson::Value& json, rapidjson::MemoryPoolAllocator<>& jsonAllocator);

            JsonValueBinder(JsonValueBinder&&);
            JsonValueBinder& operator=(JsonValueBinder&&);

            ECSqlStatus _BindNull() override;
            ECSqlStatus _BindBoolean(bool value) override;
            ECSqlStatus _BindBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy) override;
            ECSqlStatus _BindZeroBlob(int blobSize) override;
            ECSqlStatus _BindDateTime(uint64_t julianDayMsec, DateTime::Info const&) override;
            ECSqlStatus _BindDateTime(double julianDay, DateTime::Info const&) override;
            ECSqlStatus _BindDouble(double value) override;
            ECSqlStatus _BindInt(int value) override;
            ECSqlStatus _BindInt64(int64_t value) override;
            ECSqlStatus _BindPoint2d(DPoint2dCR) override;
            ECSqlStatus _BindPoint3d(DPoint3dCR) override;
            ECSqlStatus _BindText(Utf8CP stringValue, IECSqlBinder::MakeCopy makeCopy, int byteCount) override;
            ECSqlStatus _BindIdSet(IdSet<BeInt64Id> const& idSet) override { return ECSqlStatus::Error; }

            IECSqlBinder& _BindStructMember(Utf8CP structMemberPropertyName) override;
            IECSqlBinder& _BindStructMember(ECN::ECPropertyId structMemberPropertyId) override;

            IECSqlBinder& _AddArrayElement() override;

        };

    rapidjson::Document m_json;
    std::unique_ptr<JsonValueBinder> m_rootBinder = nullptr;

    void Initialize();

    void _OnClearBindings() override { Initialize(); }
    ECSqlStatus _OnBeforeStep() override;

    ECSqlStatus _BindNull() override { _OnClearBindings(); return ECSqlStatus::Success; }
    ECSqlStatus _BindBoolean(bool value) override { return m_rootBinder->BindBoolean(value); }
    ECSqlStatus _BindBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy) override { return m_rootBinder->BindBlob(value, blobSize, makeCopy); }
    ECSqlStatus _BindZeroBlob(int blobSize) override { return m_rootBinder->BindZeroBlob(blobSize); }
    ECSqlStatus _BindDateTime(uint64_t julianDayMsec, DateTime::Info const& dtInfo) override { return m_rootBinder->BindDateTime(julianDayMsec, dtInfo); }
    ECSqlStatus _BindDateTime(double julianDay, DateTime::Info const& dtInfo) override { return m_rootBinder->BindDateTime(julianDay, dtInfo); }
    ECSqlStatus _BindDouble(double value) override { return m_rootBinder->BindDouble(value); }
    ECSqlStatus _BindInt(int value) override { return m_rootBinder->BindInt(value); }
    ECSqlStatus _BindInt64(int64_t value) override { return m_rootBinder->BindInt64(value); }
    ECSqlStatus _BindPoint2d(DPoint2dCR value) override { return m_rootBinder->BindPoint2d(value); }
    ECSqlStatus _BindPoint3d(DPoint3dCR value) override { return m_rootBinder->BindPoint3d(value); }
    ECSqlStatus _BindText(Utf8CP stringValue, IECSqlBinder::MakeCopy makeCopy, int byteCount) override { return m_rootBinder->BindText(stringValue, makeCopy, byteCount); }
    ECSqlStatus _BindIdSet(IdSet<BeInt64Id> const& idSet) override { return ECSqlStatus::Error; }

    IECSqlBinder& _BindStructMember(Utf8CP structMemberPropertyName) override { return m_rootBinder->operator[](structMemberPropertyName); }
    IECSqlBinder& _BindStructMember(ECN::ECPropertyId structMemberPropertyId) override { return m_rootBinder->operator[](structMemberPropertyId); }

    IECSqlBinder& _AddArrayElement() override { return m_rootBinder->AddArrayElement(); }

public:
    ArrayECSqlBinder(ECSqlPrepareContext&, ECSqlTypeInfo const&, SqlParamNameGenerator&);
    ~ArrayECSqlBinder() {}
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
