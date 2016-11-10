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
struct IdECSqlBinder;

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      11/2016
//+===============+===============+===============+===============+===============+======
struct NavigationPropertyECSqlBinder : public ECSqlBinder, IECSqlStructBinder
    {
    friend struct ECSqlBinderFactory;

    private:
        std::unique_ptr<IdECSqlBinder> m_idBinder;
        std::unique_ptr<IdECSqlBinder> m_relECClassIdBinder;

        NavigationPropertyECSqlBinder(ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& ecsqlTypeInfo);
        BentleyStatus Initialize(ECSqlPrepareContext&);

        //only needed at prepare time to set up the binder
        virtual void _SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteParameterIndex) override;

        //these are needed by the actual binding API
        virtual IECSqlBinder& _GetMember(Utf8CP navPropMemberPropertyName) override;
        virtual IECSqlBinder& _GetMember(ECN::ECPropertyId navPropMemberPropertyId) override;
        virtual ECSqlStatus _BindNull() override;
        virtual IECSqlPrimitiveBinder& _BindPrimitive() override;
        virtual IECSqlStructBinder& _BindStruct() override { return *this; }
        virtual IECSqlArrayBinder& _BindArray(uint32_t initialCapacity) override;

    public:
        ~NavigationPropertyECSqlBinder() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

