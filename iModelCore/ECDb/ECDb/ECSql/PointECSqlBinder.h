/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PointECSqlBinder.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlBinder.h"
#include "IECSqlPrimitiveBinder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      08/2013
//+===============+===============+===============+===============+===============+======
struct PointToColumnsECSqlBinder : public ECSqlBinder, IECSqlPrimitiveBinder
    {
private:
    bool m_isPoint3D;
    int m_xSqliteIndex;
    int m_ySqliteIndex;
    int m_zSqliteIndex;

    virtual void _SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteParameterIndex) override;

    virtual IECSqlPrimitiveBinder& _BindPrimitive() override;
    virtual IECSqlStructBinder& _BindStruct() override;
    virtual IECSqlArrayBinder& _BindArray(uint32_t initialCapacity) override;

    virtual ECSqlStatus _BindNull() override;
    virtual ECSqlStatus _BindBoolean(bool value) override;
    virtual ECSqlStatus _BindBinary(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy) override;
    virtual ECSqlStatus _BindDateTime(uint64_t julianDayTicksHns, DateTime::Info const* metadata) override;
    virtual ECSqlStatus _BindDouble(double value) override;
    virtual ECSqlStatus _BindGeometryBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy) override;
    virtual ECSqlStatus _BindInt(int value) override;
    virtual ECSqlStatus _BindInt64(int64_t value) override;
    virtual ECSqlStatus _BindPoint2D (DPoint2dCR value) override;
    virtual ECSqlStatus _BindPoint3D (DPoint3dCR value) override;
    virtual ECSqlStatus _BindText(Utf8CP stringValue, IECSqlBinder::MakeCopy makeCopy, int byteCount) override;

    bool IsPoint3D () const {return m_isPoint3D;}
public:
    PointToColumnsECSqlBinder(ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& typeInfo, bool isPoint3D)
        : ECSqlBinder(ecsqlStatement, typeInfo, isPoint3D ? 3 : 2, false, false), m_isPoint3D (isPoint3D), m_xSqliteIndex(-1), m_ySqliteIndex(-1), m_zSqliteIndex(-1)
        {}

    ~PointToColumnsECSqlBinder() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE