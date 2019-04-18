/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECSqlField.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      06/2013
//+===============+===============+===============+===============+===============+======
struct PointECSqlField final : public ECSqlField
    {
private:
    int m_xColumnIndex;
    int m_yColumnIndex;
    int m_zColumnIndex;

    bool _IsNull() const override;

    void const* _GetBlob(int* blobSize) const override;
    bool _GetBoolean() const override;
    uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override;
    double _GetDateTimeJulianDays(DateTime::Info& metadata) const override;
    double _GetDouble() const override;
    int _GetInt() const override;
    int64_t _GetInt64() const override;
    Utf8CP _GetText() const override;
    DPoint2d _GetPoint2d() const override;
    DPoint3d _GetPoint3d() const override;
    IGeometryPtr _GetGeometry() const override;

    IECSqlValue const& _GetStructMemberValue(Utf8CP memberName) const override;
    IECSqlValueIterable const& _GetStructIterable() const override;

    int _GetArrayLength() const override;
    IECSqlValueIterable const& _GetArrayIterable() const override;

    bool IsPoint3d() const { return m_zColumnIndex >= 0; }

public:
    PointECSqlField(ECSqlSelectPreparedStatement& stmt, ECSqlColumnInfo const& colInfo, int xColumnIndex, int yColumnIndex, int zColumnIndex)
        : ECSqlField(stmt, colInfo, false, false), m_xColumnIndex(xColumnIndex), m_yColumnIndex(yColumnIndex), m_zColumnIndex(zColumnIndex)
        {}
    PointECSqlField(ECSqlSelectPreparedStatement& stmt, ECSqlColumnInfo const& colInfo, int xColumnIndex, int yColumnIndex) : PointECSqlField (stmt, colInfo, xColumnIndex, yColumnIndex, -1) {}
    ~PointECSqlField() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
