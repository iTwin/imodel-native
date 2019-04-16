/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlBinder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      08/2013
//+===============+===============+===============+===============+===============+======
struct PrimitiveECSqlBinder final : public ECSqlBinder
    {
private:
    ECSqlStatus CanBind(ECN::PrimitiveType requestedType) const;

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

    int GetSqlParameterIndex() const 
        { 
        BeAssert(GetMappedSqlParameterNames().size() == 1); 
        BeAssert(!GetMappedSqlParameterNames()[0].empty());
        return GetSqliteStatement().GetParameterIndex(GetMappedSqlParameterNames()[0].c_str());
        }

public:
    PrimitiveECSqlBinder(ECSqlPrepareContext& ctx, ECSqlTypeInfo const& typeInfo, SqlParamNameGenerator& paramNameGen) : ECSqlBinder(ctx, typeInfo, paramNameGen, 1, false, false) {}
    ~PrimitiveECSqlBinder() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
