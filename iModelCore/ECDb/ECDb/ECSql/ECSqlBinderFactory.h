/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlBinderFactory.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECSqlTypeInfo.h"
#include "../PropertyMap.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECSqlBinder;
struct IdECSqlBinder;
struct ECSqlPrepareContext;

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      08/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlBinderFactory
    {
    private:
        ECSqlBinderFactory();
        ~ECSqlBinderFactory();

        static bool RequiresNoopBinder(ECSqlPrepareContext&, PropertyMap const&, ECSqlSystemPropertyInfo const& sysPropertyInfo);
    public:
        static std::unique_ptr<ECSqlBinder> CreateBinder(ECSqlPrepareContext&, ECSqlTypeInfo const& typeInfo);
        static std::unique_ptr<ECSqlBinder> CreateBinder(ECSqlPrepareContext&, ParameterExp const& parameterExp);
        static std::unique_ptr<ECSqlBinder> CreateBinder(ECSqlPrepareContext& ctx, PropertyMap const& propMap) { return CreateBinder(ctx, ECSqlTypeInfo(propMap)); }

        static std::unique_ptr<IdECSqlBinder> CreateIdBinder(ECSqlPrepareContext&, PropertyMap const&, ECSqlSystemPropertyInfo const& sysPropertyInfo);

    };

END_BENTLEY_SQLITE_EC_NAMESPACE
