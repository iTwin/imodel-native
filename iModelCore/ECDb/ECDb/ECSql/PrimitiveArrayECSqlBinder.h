/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PrimitiveArrayECSqlBinder.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlBinder.h"
#include "PrimitiveECSqlBinder.h"
#include "ECSqlStatementNoopImpls.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      01/2014
//+===============+===============+===============+===============+===============+======
struct PrimitiveArrayECSqlBinder : public ECSqlBinder
    {
private:
    struct ElementBinder : IECSqlBinder
        {
        private:
            const uint32_t ARRAY_PROPERTY_INDEX = 1;
            ECDbCR m_ecdb;
            ECSqlTypeInfo const& m_arrayTypeInfo;
            ECN::StandaloneECInstancePtr m_arrayInstance = nullptr;
            int m_currentArrayIndex = -1;

            virtual ECSqlStatus _BindNull() override { return SetValue(ECN::ECValue()); }
            virtual ECSqlStatus _BindBoolean(bool value) override;
            virtual ECSqlStatus _BindBlob(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy) override;
            virtual ECSqlStatus _BindZeroBlob(int blobSize) override;
            virtual ECSqlStatus _BindDateTime(double julianDay, DateTime::Info const&) override;
            virtual ECSqlStatus _BindDateTime(uint64_t julianDayMsec, DateTime::Info const&) override;
            virtual ECSqlStatus _BindDouble(double value) override;
            virtual ECSqlStatus _BindInt(int value) override;
            virtual ECSqlStatus _BindInt64(int64_t value) override;
            virtual ECSqlStatus _BindPoint2d(DPoint2dCR value) override;
            virtual ECSqlStatus _BindPoint3d(DPoint3dCR value) override;
            virtual ECSqlStatus _BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount) override;

            virtual IECSqlBinder& _BindStructMember(Utf8CP structMemberPropertyName) override;
            virtual IECSqlBinder& _BindStructMember(ECN::ECPropertyId structMemberPropertyId) override;

            virtual IECSqlBinder& _AddArrayElement() override;

            ECSqlStatus FailAndLogTypeMismatchError(ECN::PrimitiveType) const;
            ECSqlStatus SetValue(ECN::ECValueCR);

        public:
            ElementBinder(ECDbCR ecdb, ECSqlTypeInfo const& arrayTypeInfo) : m_ecdb(ecdb), m_arrayTypeInfo(arrayTypeInfo) {}

            ECSqlStatus MoveNext();
            void Clear();
            ECN::StandaloneECInstance& GetArrayInstance();

            uint32_t GetArrayLength() const { return (uint32_t) (m_currentArrayIndex + 1); }
        };


    int m_sqliteIndex;
    ElementBinder m_currentElementBinder;

    virtual void _SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteParameterIndex) override { m_sqliteIndex = (int) sqliteParameterIndex; }
    virtual void _OnClearBindings() override { return m_currentElementBinder.Clear(); }
    virtual ECSqlStatus _OnBeforeStep() override;

    virtual IECSqlBinder& _AddArrayElement() override;
    virtual ECSqlStatus _BindNull() override { _OnClearBindings(); return ECSqlStatus::Success; }

    virtual ECSqlStatus _BindBoolean(bool value) override { LOG.error("Type mismatch. Cannot bind bool value to primitive array parameter."); return ECSqlStatus::Error; }
    virtual ECSqlStatus _BindBlob(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy) override { LOG.error("Type mismatch. Cannot bind BLOB value to primitive array parameter."); return ECSqlStatus::Error; }
    virtual ECSqlStatus _BindZeroBlob(int blobSize) override { LOG.error("Type mismatch. Cannot bind Zeroblob value to primitive array parameter."); return ECSqlStatus::Error; }
    virtual ECSqlStatus _BindDateTime(double julianDay, DateTime::Info const&) override { LOG.error("Type mismatch. Cannot bind DateTime value to primitive array parameter."); return ECSqlStatus::Error; }
    virtual ECSqlStatus _BindDateTime(uint64_t julianDayMsec, DateTime::Info const&) override { LOG.error("Type mismatch. Cannot bind DateTime value to primitive array parameter."); return ECSqlStatus::Error; }
    virtual ECSqlStatus _BindDouble(double value) override { LOG.error("Type mismatch. Cannot bind double value to primitive array parameter."); return ECSqlStatus::Error; }
    virtual ECSqlStatus _BindInt(int value) override { LOG.error("Type mismatch. Cannot bind Int32 value to primitive array parameter."); return ECSqlStatus::Error; }
    virtual ECSqlStatus _BindInt64(int64_t value) override { LOG.error("Type mismatch. Cannot bind Int64 value to primitive array parameter."); return ECSqlStatus::Error; }
    virtual ECSqlStatus _BindPoint2d(DPoint2dCR value) override { LOG.error("Type mismatch. Cannot bind Point2d value to primitive array parameter."); return ECSqlStatus::Error; }
    virtual ECSqlStatus _BindPoint3d(DPoint3dCR value) override { LOG.error("Type mismatch. Cannot bind Point3d value to primitive array parameter."); return ECSqlStatus::Error; }
    virtual ECSqlStatus _BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount) override { LOG.error("Type mismatch. Cannot bind string value to primitive array parameter."); return ECSqlStatus::Error; }
    virtual IECSqlBinder& _BindStructMember(Utf8CP structMemberPropertyName) override { LOG.error("Type mismatch. Cannot bind struct to primitive array parameter."); return NoopECSqlBinder::Get(); }
    virtual IECSqlBinder& _BindStructMember(ECN::ECPropertyId structMemberPropertyId) override { LOG.error("Type mismatch. Cannot bind struct to primitive array parameter."); return NoopECSqlBinder::Get(); }


public:
    PrimitiveArrayECSqlBinder(ECSqlStatementBase&, ECSqlTypeInfo const&);
    ~PrimitiveArrayECSqlBinder() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
