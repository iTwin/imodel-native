/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PrimitiveMappedToSingleColumnECSqlField.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
struct PrimitiveMappedToSingleColumnECSqlField : public ECSqlField, public IECSqlPrimitiveValue
    {
private:
    int m_sqliteColumnIndex;
    DateTime::Info m_datetimeMetadata;

    bool CanGetValue(ECN::PrimitiveType requestedType) const;
    bool CanGetValue(ECN::PrimitiveType testType, ECN::PrimitiveType requestedType, Utf8CP typeName, Utf8CP getValueMethodName) const;

    virtual ECSqlStatus _Init() override;

    virtual bool _IsNull() const override;

    virtual IECSqlPrimitiveValue const& _GetPrimitive() const override;
    virtual IECSqlStructValue const& _GetStruct() const override;
    virtual IECSqlArrayValue const& _GetArray() const override;

    virtual void const* _GetBinary(int* binarySize) const override;
    virtual bool _GetBoolean() const override;
    virtual uint64_t _GetDateTimeJulianDaysHns(DateTime::Info& metadata) const override;
    virtual double _GetDateTimeJulianDays(DateTime::Info& metadata) const override;
    virtual double _GetDouble() const override;
    virtual int _GetInt() const override;
    virtual int64_t _GetInt64() const override;
    virtual Utf8CP _GetText() const override;
    virtual DPoint2d _GetPoint2D() const override;
    virtual DPoint3d _GetPoint3D() const override;
    virtual IGeometryPtr _GetGeometry() const override;
    virtual void const* _GetGeometryBlob(int* blobSize) const override;

    int GetSqliteColumnIndex() const;

public:
    PrimitiveMappedToSingleColumnECSqlField(ECSqlStatementBase& ecsqlStatement, ECSqlColumnInfo&& ecsqlColumnInfo, int ecsqlColumnIndex);
    ~PrimitiveMappedToSingleColumnECSqlField() {}
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
