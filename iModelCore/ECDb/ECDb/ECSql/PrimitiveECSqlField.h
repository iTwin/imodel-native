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

    bool _IsNull() const override { return GetSqliteValue(m_sqliteColumnIndex).IsNull(); }

    void const* _GetBlob(int* blobSize) const override;
    bool _GetBoolean() const override { return  GetSqliteValue(m_sqliteColumnIndex).GetValueInt(); }
    uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override;
    double _GetDateTimeJulianDays(DateTime::Info& metadata) const override;
    double _GetDouble() const override { return GetSqliteValue(m_sqliteColumnIndex).GetValueDouble(); }
    int _GetInt() const override { return GetSqliteValue(m_sqliteColumnIndex).GetValueInt(); }
    int64_t _GetInt64() const override { return GetSqliteValue(m_sqliteColumnIndex).GetValueInt64(); }
    Utf8CP _GetText() const override { return GetSqliteValue(m_sqliteColumnIndex).GetValueText(); }
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
    PrimitiveECSqlField(ECDbCR ecdb, Changes::Change const& change, Changes::Change::Stage const& stage, ECSqlColumnInfo const& columnInfo, int columnIndex);
    ~PrimitiveECSqlField() {}
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
