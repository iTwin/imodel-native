/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ECSqlBinder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct PointECSqlBinder final : public ECSqlBinder
    {
    private:
        enum class Coordinate
            {
            X = 0,
            Y = 1,
            Z = 2
            };

        bool m_isPoint3d;

        ECSqlStatus _BindNull() override;
        ECSqlStatus _BindBoolean(bool value) override;
        ECSqlStatus _BindBlob(const void* value, int binarySize, IECSqlBinder::MakeCopy) override;
        ECSqlStatus _BindZeroBlob(int blobSize) override;
        ECSqlStatus _BindDateTime(uint64_t julianDayMsec, DateTime::Info const&) override;
        ECSqlStatus _BindDateTime(double julianDay, DateTime::Info const&) override;
        ECSqlStatus _BindDouble(double value) override;
        ECSqlStatus _BindInt(int value) override;
        ECSqlStatus _BindInt64(int64_t value) override;
        ECSqlStatus _BindPoint2d(DPoint2dCR) override;
        ECSqlStatus _BindPoint3d(DPoint3dCR) override;
        ECSqlStatus _BindText(Utf8CP stringValue, IECSqlBinder::MakeCopy makeCopy, int byteCount) override;
        ECSqlStatus _BindIdSet(IdSet<BeInt64Id> const& idSet) override { return ECSqlStatus::Error; }

        IECSqlBinder& _BindStructMember(Utf8CP structMemberPropertyName) override;
        IECSqlBinder& _BindStructMember(ECN::ECPropertyId structMemberPropertyId) override;

        IECSqlBinder& _AddArrayElement() override;

        int GetCoordSqlParamIndex(Coordinate coord) const
            {
            BeAssert(GetMappedSqlParameterNames().size() == (m_isPoint3d ? 3 : 2));
            BeAssert(Enum::ToInt(coord) <= 3);
            BeAssert(!GetMappedSqlParameterNames()[(size_t) Enum::ToInt(coord)].empty());

            Utf8StringCR paramName = GetMappedSqlParameterNames()[(size_t) Enum::ToInt(coord)];
            return GetSqliteStatement().GetParameterIndex(paramName.c_str());
            }

    public:
        PointECSqlBinder(ECSqlPrepareContext& ctx, ECSqlTypeInfo const& typeInfo, bool isPoint3d, SqlParamNameGenerator& paramNameGen)
            : ECSqlBinder(ctx, typeInfo, paramNameGen, isPoint3d ? 3 : 2, false, false), m_isPoint3d(isPoint3d)
            {}

        ~PointECSqlBinder() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE