/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructECSqlBinder.h $
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
struct StructECSqlBinder : public ECSqlBinder
    {
    friend struct ECSqlBinderFactory;

    private:
        //=======================================================================================
        //! @bsiclass                                                Krischan.Eberle      03/2014
        //+===============+===============+===============+===============+===============+======
        struct MemberBinderInfo
            {
            private:
                ECSqlBinder* m_memberBinder;
                int m_ecsqlComponentIndexOffset;

            public:
                MemberBinderInfo(ECSqlBinder& memberBinder, int ecsqlComponentIndexOffset)
                    : m_memberBinder(&memberBinder), m_ecsqlComponentIndexOffset(ecsqlComponentIndexOffset)
                    {}
                //use compiler generated dtor/copy ctor/copy assignment op

                ECSqlBinder& GetMemberBinder() const { return *m_memberBinder; }
                //! Gets the component index of the parent struct binder that maps to the first component
                //! of this member's binder.
                int GetECSqlComponentIndexOffset() const { return m_ecsqlComponentIndexOffset; }
            };

        std::map<ECN::ECPropertyId, std::unique_ptr<ECSqlBinder>> m_memberBinders;
        std::vector<MemberBinderInfo> m_ecsqlComponentIndexToMemberBinderMapping;

        StructECSqlBinder(ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& ecsqlTypeInfo);
        BentleyStatus Initialize(ECSqlPrepareContext&);

        //only needed at prepare time to set up the binder
        void _SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteParameterIndex) override;
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

        IECSqlBinder& _BindStructMember(Utf8CP structMemberPropertyName) override;
        IECSqlBinder& _BindStructMember(ECN::ECPropertyId structMemberPropertyId) override;

        IECSqlBinder& _AddArrayElement() override;

    public:
        ~StructECSqlBinder() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
