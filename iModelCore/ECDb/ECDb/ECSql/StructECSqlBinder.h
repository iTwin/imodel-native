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
        virtual void _SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteParameterIndex) override;
        virtual void _OnClearBindings() override;
        virtual ECSqlStatus _OnBeforeStep() override;

        virtual ECSqlStatus _BindNull() override;
        virtual ECSqlStatus _BindBoolean(bool value) override;
        virtual ECSqlStatus _BindBlob(const void* value, int binarySize, IECSqlBinder::MakeCopy) override;
        virtual ECSqlStatus _BindZeroBlob(int blobSize) override;
        virtual ECSqlStatus _BindDateTime(uint64_t julianDayMsec, DateTime::Info const&) override;
        virtual ECSqlStatus _BindDateTime(double julianDay, DateTime::Info const&) override;
        virtual ECSqlStatus _BindDouble(double value) override;
        virtual ECSqlStatus _BindInt(int value) override;
        virtual ECSqlStatus _BindInt64(int64_t value) override;
        virtual ECSqlStatus _BindPoint2d(DPoint2dCR) override;
        virtual ECSqlStatus _BindPoint3d(DPoint3dCR) override;
        virtual ECSqlStatus _BindText(Utf8CP stringValue, IECSqlBinder::MakeCopy makeCopy, int byteCount) override;

        virtual IECSqlBinder& _BindStructMember(Utf8CP structMemberPropertyName) override;
        virtual IECSqlBinder& _BindStructMember(ECN::ECPropertyId structMemberPropertyId) override;

        virtual IECSqlBinder& _AddArrayElement() override;

    public:
        ~StructECSqlBinder() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
