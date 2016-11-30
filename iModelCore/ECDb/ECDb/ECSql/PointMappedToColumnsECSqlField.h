/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PointMappedToColumnsECSqlField.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECSqlField.h"
#include "IECSqlPrimitiveValue.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      06/2013
//+===============+===============+===============+===============+===============+======
struct PointMappedToColumnsECSqlField : public ECSqlField, IECSqlPrimitiveValue
    {
private:
    int m_xColumnIndex;
    int m_yColumnIndex;
    int m_zColumnIndex;

    virtual bool _IsNull() const override;

    virtual IECSqlPrimitiveValue const& _GetPrimitive() const override;
    virtual IECSqlStructValue const& _GetStruct() const override;
    virtual IECSqlArrayValue const& _GetArray() const override;

    virtual void const* _GetBinary(int* binarySize) const override;
    virtual bool _GetBoolean() const override;
    virtual uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override;
    virtual double _GetDateTimeJulianDays(DateTime::Info& metadata) const override;
    virtual double _GetDouble() const override;
    virtual int _GetInt() const override;
    virtual int64_t _GetInt64() const override;
    virtual Utf8CP _GetText() const override;
    virtual DPoint2d _GetPoint2d() const override;
    virtual DPoint3d _GetPoint3d() const override;
    virtual IGeometryPtr _GetGeometry() const override;
    virtual void const* _GetGeometryBlob(int* blobSize) const override;

    bool IsPoint3d() const { return m_zColumnIndex >= 0; }

public:
    PointMappedToColumnsECSqlField(ECSqlStatementBase&, ECSqlColumnInfo const&, int xColumnIndex, int yColumnIndex, int zColumnIndex);
    PointMappedToColumnsECSqlField(ECSqlStatementBase& stmt, ECSqlColumnInfo const& colInfo, int xColumnIndex, int yColumnIndex) : PointMappedToColumnsECSqlField (stmt, colInfo, xColumnIndex, yColumnIndex, -1) {}
    ~PointMappedToColumnsECSqlField() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
