/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PrimitiveToSingleColumnECSqlBinder.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
struct PrimitiveToSingleColumnECSqlBinder : public ECSqlBinder, public IECSqlPrimitiveBinder
    {
private:
    int m_sqliteIndex;

    ECSqlStatus CanBind (ECN::PrimitiveType requestedType) const;

    virtual void _SetSqliteIndex (int ecsqlParameterComponentIndex, size_t sqliteParameterIndex) override;

    virtual IECSqlPrimitiveBinder& _BindPrimitive () override;
    virtual IECSqlStructBinder& _BindStruct () override;
    virtual IECSqlArrayBinder& _BindArray (UInt32 initialCapacity) override;

    virtual ECSqlStatus _BindNull () override;
    virtual ECSqlStatus _BindBoolean (bool value) override;
    virtual ECSqlStatus _BindBinary (const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy) override;
    virtual ECSqlStatus _BindDateTime (UInt64 julianDayTicksHns, DateTime::Info const* metadata) override;
    virtual ECSqlStatus _BindDouble (double value) override;
    virtual ECSqlStatus _BindGeometryBlob (const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy) override;
    virtual ECSqlStatus _BindInt (int value) override;
    virtual ECSqlStatus _BindInt64 (Int64 value) override;
    virtual ECSqlStatus _BindPoint2D (DPoint2dCR value) override;
    virtual ECSqlStatus _BindPoint3D (DPoint3dCR value) override;
    virtual ECSqlStatus _BindText (Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount) override;
    virtual ECSqlStatus _BindId (BeRepositoryBasedId value) override;

public:
    PrimitiveToSingleColumnECSqlBinder (ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& typeInfo)
        : ECSqlBinder (ecsqlStatement, typeInfo, 1, false, false), m_sqliteIndex (-1)
        {
        }

    ~PrimitiveToSingleColumnECSqlBinder () {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
