/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/IECSqlBinder.h>
#include <ECDb/IECSqlValue.h>
#include "ECSqlField.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//**************** NoopECSqlBinder **************************************
//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct NoopECSqlBinder final : public IECSqlBinder
    {
    private:
        ECSqlStatus m_errorStatus;
        static NoopECSqlBinder* s_singleton;

        NoopECSqlBinder() : m_errorStatus(ECSqlStatus::Error) {}

        ECSqlStatus _BindNull() override { return m_errorStatus; }
        ECSqlStatus _BindBoolean(bool value) override { return m_errorStatus; }
        ECSqlStatus _BindBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy) override { return m_errorStatus; }
        ECSqlStatus _BindZeroBlob(int blobSize) override { return m_errorStatus; }
        ECSqlStatus _BindDateTime(uint64_t julianDayTicksHns, DateTime::Info const&) override { return m_errorStatus; }
        ECSqlStatus _BindDateTime(double julianDay, DateTime::Info const&) override { return m_errorStatus; }
        ECSqlStatus _BindDouble(double value) override { return m_errorStatus; }
        ECSqlStatus _BindInt(int value) override { return m_errorStatus; }
        ECSqlStatus _BindInt64(int64_t value) override { return m_errorStatus; }
        ECSqlStatus _BindPoint2d(DPoint2dCR value) override { return m_errorStatus; }
        ECSqlStatus _BindPoint3d(DPoint3dCR value) override { return m_errorStatus; }
        ECSqlStatus _BindText(Utf8CP stringValue, IECSqlBinder::MakeCopy, int byteCount) override { return m_errorStatus; }
        ECSqlStatus _BindIdSet(IdSet<BeInt64Id> const& idSet) override { return m_errorStatus; }

        IECSqlBinder& _BindStructMember(Utf8CP structMemberPropertyName) override { return *this; }
        IECSqlBinder& _BindStructMember(ECN::ECPropertyId structMemberPropertyId) override { return *this; }

        IECSqlBinder& _AddArrayElement() override { return *this; }

    public:
        static NoopECSqlBinder& Get();
    };


//**************** NoopECSqlValue **************************************
//=======================================================================================
//! No-op implementation for IECSqlValue
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct NoopECSqlValue final : public IECSqlValue, IECSqlValueIterable
    {
    private:
        ECSqlColumnInfo m_dummyColumnInfo;

        static NoopECSqlValue const* s_singleton;

        NoopECSqlValue() : IECSqlValue() {}
        ~NoopECSqlValue() {}

        ECSqlColumnInfoCR _GetColumnInfo() const override { return m_dummyColumnInfo; }
        bool _IsNull() const override { return true; }

        //primitive
        void const* _GetBlob(int* blobSize) const override;
        bool _GetBoolean() const override { return false; }
        uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override { return INT64_C(0); }
        double _GetDateTimeJulianDays(DateTime::Info& metadata) const override { return 0.0; }
        double _GetDouble() const override { return 0.0; }
        int _GetInt() const override { return 0; }
        int64_t _GetInt64() const override { return INT64_C(0); }
        Utf8CP _GetText() const override { return nullptr; }
        DPoint2d _GetPoint2d() const override { return DPoint2d::From(0.0, 0.0); }
        DPoint3d _GetPoint3d() const override { return DPoint3d::From(0.0, 0.0, 0.0); }
        IGeometryPtr _GetGeometry() const override { return nullptr; }

        //struct
        IECSqlValue const& _GetStructMemberValue(Utf8CP memberName) const override { return *this; }
        IECSqlValueIterable const& _GetStructIterable() const override { return *this; }
        //array
        int _GetArrayLength() const override { return -1; }
        IECSqlValueIterable const& _GetArrayIterable() const override { return *this; }

        const_iterator _CreateIterator() const override { return end(); }

    public:
        static NoopECSqlValue const& GetSingleton();
    };



END_BENTLEY_SQLITE_EC_NAMESPACE
