/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/NavigationPropertyECSqlBinder.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECSqlBinder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      11/2016
//+===============+===============+===============+===============+===============+======
struct NavigationPropertyECSqlBinder : public ECSqlBinder, IECSqlStructBinder
    {
    private:
        static const size_t ID_MEMBER_INDEX = 0;
        static const size_t RELECCLASSID_MEMBER_INDEX = 1;

        std::vector<std::unique_ptr<ECSqlBinder>> m_memberBinders;

        //only needed at prepare time to set up the binder
        virtual void _SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteParameterIndex) override;

        //these are needed by the actual binding API
        virtual IECSqlBinder& _GetMember(Utf8CP navPropMemberPropertyName) override;
        virtual IECSqlBinder& _GetMember(ECN::ECPropertyId navPropMemberPropertyId) override;
        virtual ECSqlStatus _BindNull() override;
        virtual IECSqlPrimitiveBinder& _BindPrimitive() override;
        virtual IECSqlStructBinder& _BindStruct() override { return *this; }
        virtual IECSqlArrayBinder& _BindArray(uint32_t initialCapacity) override;

        void Initialize();
    public:
        NavigationPropertyECSqlBinder(ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& ecsqlTypeInfo);

        ~NavigationPropertyECSqlBinder() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

