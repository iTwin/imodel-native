/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/IECSqlPrimitiveValue.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <ECDb/ECDbTypes.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Represents the primitive value of a specific ECSQL column in the current
//! row of the result set of an ECSqlStatement.
//! @ingroup ECDbGroup
// @bsiclass                                                 Krischan.Eberle    03/2014
//+===============+===============+===============+===============+===============+======
struct IECSqlPrimitiveValue
    {
public:
    virtual ~IECSqlPrimitiveValue () {}

    virtual void const* _GetBinary (int* binarySize) const = 0;
    virtual bool _GetBoolean () const = 0;
    virtual uint64_t _GetDateTimeJulianDays (DateTime::Info& metadata) const = 0;
    virtual double _GetDouble () const = 0;
    virtual int _GetInt () const = 0;
    virtual int64_t _GetInt64 () const = 0;
    virtual IGeometryPtr _GetGeometry() const = 0;
    virtual void const* _GetGeometryBlob(int* blobSize) const = 0;
    virtual DPoint2d _GetPoint2D() const = 0;
    virtual DPoint3d _GetPoint3D () const = 0;
    virtual Utf8CP _GetText () const = 0;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE