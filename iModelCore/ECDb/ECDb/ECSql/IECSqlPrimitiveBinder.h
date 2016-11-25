/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/IECSqlPrimitiveBinder.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <ECDb/ECInstanceId.h>

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

    virtual ECSqlStatus _BindBoolean(bool) = 0;
    virtual ECSqlStatus _BindBinary(const void* value, int binarySize, IECSqlBinder::MakeCopy) = 0;
    virtual ECSqlStatus _BindDateTime(uint64_t julianDayMsec, DateTime::Info const&) = 0;
    virtual ECSqlStatus _BindDateTime(double julianDay, DateTime::Info const&) = 0;
    virtual ECSqlStatus _BindDouble(double) = 0;
    virtual ECSqlStatus _BindGeometryBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy) = 0;
    virtual ECSqlStatus _BindInt(int ) = 0;
    virtual ECSqlStatus _BindInt64(int64_t) = 0;
    virtual ECSqlStatus _BindPoint2d (DPoint2dCR) = 0;
    virtual ECSqlStatus _BindPoint3d (DPoint3dCR) = 0;
    virtual ECSqlStatus _BindText(Utf8CP, IECSqlBinder::MakeCopy, int byteCount) = 0;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE