/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PrimitiveECSqlBinder.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlBinder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      08/2013
//+===============+===============+===============+===============+===============+======
struct PrimitiveECSqlBinder : public ECSqlBinder
    {
private:
    int m_sqliteIndex;

    ECSqlStatus CanBind(ECN::PrimitiveType requestedType) const;

    void _SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteParameterIndex) override;

    ECSqlStatus _BindNull() override;
    ECSqlStatus _BindBoolean(bool value) override;
    ECSqlStatus _BindBlob(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy) override;
    ECSqlStatus _BindZeroBlob(int blobSize) override;
    ECSqlStatus _BindDateTime(double julianDay, DateTime::Info const&) override;
    ECSqlStatus _BindDateTime(uint64_t julianDayMsec, DateTime::Info const&) override;
    ECSqlStatus _BindDouble(double value) override;
    ECSqlStatus _BindInt(int value) override;
    ECSqlStatus _BindInt64(int64_t value) override;
    ECSqlStatus _BindPoint2d (DPoint2dCR value) override;
    ECSqlStatus _BindPoint3d (DPoint3dCR value) override;
    ECSqlStatus _BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount) override;

    IECSqlBinder& _BindStructMember(Utf8CP structMemberPropertyName) override;
    IECSqlBinder& _BindStructMember(ECN::ECPropertyId structMemberPropertyId) override;

    IECSqlBinder& _AddArrayElement() override;

public:
    PrimitiveECSqlBinder(ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& typeInfo)
        : ECSqlBinder(ecsqlStatement, typeInfo, 1, false, false), m_sqliteIndex(-1) {}

    ~PrimitiveECSqlBinder() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
