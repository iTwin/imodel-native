/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/IECSqlPrimitiveBinder.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <ECDb/ECDbTypes.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! IECSqlPrimitiveBinder is used to bind a primitive value to a binding parameter in an ECSqlStatement.
//! @ingroup ECDbGroup
// @bsiclass                                                 Krischan.Eberle    05/2013
//+===============+===============+===============+===============+===============+======
struct IECSqlPrimitiveBinder
    {
public:
    virtual ~IECSqlPrimitiveBinder() {}

    virtual ECSqlStatus _BindBoolean(bool value) = 0;
    virtual ECSqlStatus _BindBinary(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy) = 0;
    virtual ECSqlStatus _BindDateTime(uint64_t julianDayTicksHns, DateTime::Info const* metadata) = 0;
    virtual ECSqlStatus _BindDouble(double value) = 0;
    virtual ECSqlStatus _BindGeometryBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy) = 0;
    virtual ECSqlStatus _BindInt(int value) = 0;
    virtual ECSqlStatus _BindInt64(int64_t value) = 0;
    virtual ECSqlStatus _BindPoint2D (DPoint2dCR value) = 0;
    virtual ECSqlStatus _BindPoint3D (DPoint3dCR value) = 0;
    virtual ECSqlStatus _BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount) = 0;
    virtual ECSqlStatus _BindId(ECInstanceId value) = 0;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE