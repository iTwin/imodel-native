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
struct ECSqlStatementBase;

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      08/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlBinderFactory
    {
    private:
        ECSqlBinderFactory();
        ~ECSqlBinderFactory();


    public:
        static std::unique_ptr<ECSqlBinder> CreateBinder(ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& typeInfo);
        static std::unique_ptr<ECSqlBinder> CreateBinder(ECSqlStatementBase& ecsqlStatement, ParameterExp const& parameterExp);
        static std::unique_ptr<ECSqlBinder> CreateBinder(ECSqlStatementBase& ecsqlStatement, PropertyMap const& propMap) { return CreateBinder(ecsqlStatement, ECSqlTypeInfo(propMap)); }

        static std::unique_ptr<ECSqlBinder> CreateIdBinder(ECSqlStatementBase& ecsqlStatement, PropertyMap const&, ECSqlSystemPropertyKind);

    };

END_BENTLEY_SQLITE_EC_NAMESPACE
