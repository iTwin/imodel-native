/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "JsonECSqlValue.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct ArrayECSqlField final : public ECSqlField
    {
private:
    int m_sqliteColumnIndex = -1;
    mutable rapidjson::Document m_json = rapidjson::Document(rapidjson::kArrayType);
    mutable std::unique_ptr<JsonECSqlValue> m_value = nullptr;

    //IECSqlValue
    bool _IsNull() const override { return GetSqliteStatement().IsColumnNull(m_sqliteColumnIndex); }

    void const* _GetBlob(int* blobSize) const override { return GetValue().GetBlob(blobSize); }
    bool _GetBoolean() const override { return GetValue().GetBoolean(); }
    uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override { return GetValue().GetDateTimeJulianDaysMsec(metadata); }
    double _GetDateTimeJulianDays(DateTime::Info& metadata) const override { return GetValue().GetDateTimeJulianDays(metadata); }
    double _GetDouble() const override { return GetValue().GetDouble(); }
    int _GetInt() const override { return GetValue().GetInt(); }
    int64_t _GetInt64() const override { return GetValue().GetInt64(); }
    Utf8CP _GetText() const override { return GetValue().GetText(); }
    DPoint2d _GetPoint2d() const override { return GetValue().GetPoint2d(); }
    DPoint3d _GetPoint3d() const override { return GetValue().GetPoint3d(); }
    IGeometryPtr _GetGeometry() const override { return GetValue().GetGeometry(); }

    IECSqlValue const& _GetStructMemberValue(Utf8CP memberName) const override { return GetValue()[memberName]; }
    IECSqlValueIterable const& _GetStructIterable() const override { return GetValue().GetStructIterable(); }

    int _GetArrayLength() const override { return GetValue().GetArrayLength(); }
    IECSqlValueIterable const& _GetArrayIterable() const override { return GetValue().GetArrayIterable(); }

    //ECSqlField
    ECSqlStatus _OnAfterReset() override;
    ECSqlStatus _OnAfterStep() override;

    void DoReset() const;

    JsonECSqlValue const& GetValue() const { BeAssert(m_value != nullptr); return *m_value; }
public:
    ArrayECSqlField(ECSqlSelectPreparedStatement& stmt, ECSqlColumnInfo const& colInfo, int sqliteColumnIndex) : ECSqlField(stmt, colInfo, true, true), m_sqliteColumnIndex(sqliteColumnIndex) {}
    ~ArrayECSqlField() {}
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
