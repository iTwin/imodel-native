/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlField.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct PrimitiveECSqlField final : public ECSqlField
    {
private:
    int m_sqliteColumnIndex;
    DateTime::Info m_datetimeMetadata;

    bool _IsNull() const override { return GetSqliteStatement().IsColumnNull(m_sqliteColumnIndex); }

    void const* _GetBlob(int* blobSize) const override;
    bool _GetBoolean() const override { return  GetSqliteStatement().GetValueBoolean(m_sqliteColumnIndex); }
    uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override;
    double _GetDateTimeJulianDays(DateTime::Info& metadata) const override;
    double _GetDouble() const override { return GetSqliteStatement().GetValueDouble(m_sqliteColumnIndex); }
    int _GetInt() const override { return GetSqliteStatement().GetValueInt(m_sqliteColumnIndex); }
    int64_t _GetInt64() const override { return GetSqliteStatement().GetValueInt64(m_sqliteColumnIndex); }
    Utf8CP _GetText() const override { return GetSqliteStatement().GetValueText(m_sqliteColumnIndex); }
    DPoint2d _GetPoint2d() const override;
    DPoint3d _GetPoint3d() const override;
    IGeometryPtr _GetGeometry() const override;

    IECSqlValue const& _GetStructMemberValue(Utf8CP memberName) const override;
    IECSqlValueIterable const& _GetStructIterable() const override;

    int _GetArrayLength() const override;
    IECSqlValueIterable const& _GetArrayIterable() const override;
    void _OnDynamicPropertyUpdated() override;
    void UpdateDateTimeMetaData();

public:
    PrimitiveECSqlField(ECSqlSelectPreparedStatement&, ECSqlColumnInfo const&, int ecsqlColumnIndex);
    ~PrimitiveECSqlField() {}
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
