/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/SystemColumnPreparer.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "../RelationshipClassMap.h"
#include "ECSqlPrepareContext.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      05/2015
//+===============+===============+===============+===============+===============+======
struct SystemColumnPreparer
    {
private:
    static std::map<IClassMap::Type, std::unique_ptr<SystemColumnPreparer>> s_flyweights;

    virtual ECSqlStatus _GetWhereClause(ECSqlPrepareContext&, NativeSqlBuilder& whereClauseBuilder, IClassMap const&, ECSqlType, bool isPolymorphicClassExp, Utf8CP tableAlias) const = 0;

    static SystemColumnPreparer const& GetFor(IClassMap::Type);

protected:
    SystemColumnPreparer() {}

    static SystemColumnPreparer const& GetDefault();

public:
    virtual ~SystemColumnPreparer() {}

    ECSqlStatus GetWhereClause(ECSqlPrepareContext&, NativeSqlBuilder& whereClauseBuilder, IClassMap const&, ECSqlType, bool isPolymorphicClassExp, Utf8CP tableAlias) const;

    static SystemColumnPreparer const& GetFor(IClassMap const& classMap);
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      05/2015
//+===============+===============+===============+===============+===============+======
struct RegularClassSystemColumnPreparer : SystemColumnPreparer
    {
private:
    virtual ECSqlStatus _GetWhereClause(ECSqlPrepareContext&, NativeSqlBuilder& whereClauseBuilder, IClassMap const&, ECSqlType, bool isPolymorphicClassExp, Utf8CP tableAlias) const override;

public:
    RegularClassSystemColumnPreparer() : SystemColumnPreparer() {}
    ~RegularClassSystemColumnPreparer() {}
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      05/2015
//+===============+===============+===============+===============+===============+======
struct EndTableSystemColumnPreparer : SystemColumnPreparer
    {
private:
    virtual ECSqlStatus _GetWhereClause(ECSqlPrepareContext&, NativeSqlBuilder& whereClauseBuilder, IClassMap const&, ECSqlType, bool isPolymorphicClassExp, Utf8CP tableAlias) const override;

public:
    EndTableSystemColumnPreparer() : SystemColumnPreparer() {}
    ~EndTableSystemColumnPreparer() {}
    };


//=======================================================================================
// @bsiclass                                                Krischan.Eberle      05/2015
//+===============+===============+===============+===============+===============+======
struct SecondaryTableSystemColumnPreparer : SystemColumnPreparer
    {
private:
    virtual ECSqlStatus _GetWhereClause(ECSqlPrepareContext&, NativeSqlBuilder& whereClauseBuilder, IClassMap const&, ECSqlType, bool isPolymorphicClassExp, Utf8CP tableAlias) const override;

public:
    SecondaryTableSystemColumnPreparer() : SystemColumnPreparer() {}
    ~SecondaryTableSystemColumnPreparer() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE