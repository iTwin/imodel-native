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
struct StructECSqlBinder final : public ECSqlBinder
    {
    friend struct ECSqlBinderFactory;

    private:
        std::map<ECN::ECPropertyId, std::unique_ptr<ECSqlBinder>> m_memberBinders;

        StructECSqlBinder(ECSqlPrepareContext&, ECSqlTypeInfo const&, SqlParamNameGenerator&);
        BentleyStatus Initialize(ECSqlPrepareContext&, SqlParamNameGenerator&);

        void _OnClearBindings() override;
        ECSqlStatus _OnBeforeStep() override;

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
        ECSqlStatus _BindIdSet(std::shared_ptr<VirtualSet> virtualSet) override { return ECSqlStatus::Error; }
        ECSqlStatus _BindVirtualSet(std::shared_ptr<VirtualSet> virtualSet) override { return ECSqlStatus::Error; }

        IECSqlBinder& _BindStructMember(Utf8CP structMemberPropertyName) override;
        IECSqlBinder& _BindStructMember(ECN::ECPropertyId structMemberPropertyId) override;

        IECSqlBinder& _AddArrayElement() override;

    public:
        ~StructECSqlBinder() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
