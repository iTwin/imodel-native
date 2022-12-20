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
struct IdECSqlBinder final : public ECSqlBinder
    {
private:
    bool m_isNoop;

    int GetSqlParamIndex() const 
        { 
        if (m_isNoop) 
            return -1; 
        
        BeAssert(GetMappedSqlParameterNames().size() == 1 && !GetMappedSqlParameterNames()[0].empty());
        return GetSqliteStatement().GetParameterIndex(GetMappedSqlParameterNames()[0].c_str());
        }
    std::shared_ptr<VirtualSet> m_pVirtualSet;

    void _OnClearBindings() override { m_pVirtualSet.reset(); }

public:
    ECSqlStatus _BindNull() override;
    ECSqlStatus _BindBoolean(bool value) override;
    ECSqlStatus _BindBlob(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy) override;
    ECSqlStatus _BindZeroBlob(int blobSize) override;
    ECSqlStatus _BindDateTime(double julianDay, DateTime::Info const&) override;
    ECSqlStatus _BindDateTime(uint64_t julianDayMsec, DateTime::Info const&) override;
    ECSqlStatus _BindDouble(double value) override;
    ECSqlStatus _BindInt(int value) override;
    ECSqlStatus _BindInt64(int64_t value) override;
    ECSqlStatus _BindPoint2d(DPoint2dCR value) override;
    ECSqlStatus _BindPoint3d(DPoint3dCR value) override;
    ECSqlStatus _BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount) override;
    ECSqlStatus _BindIdSet(IdSet<BeInt64Id> const& idSet) override;
    ECSqlStatus _BindVirtualSet(std::shared_ptr<VirtualSet>) override;

    IECSqlBinder& _BindStructMember(Utf8CP structMemberPropertyName) override;
    IECSqlBinder& _BindStructMember(ECN::ECPropertyId structMemberPropertyId) override;

    IECSqlBinder& _AddArrayElement() override;

    public:
        IdECSqlBinder(ECSqlPrepareContext&, ECSqlTypeInfo const&, bool isNoop, SqlParamNameGenerator&);
        ~IdECSqlBinder() { OnClearBindings(); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE