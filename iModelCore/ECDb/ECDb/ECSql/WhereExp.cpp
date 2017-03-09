/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/WhereExp.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
WhereExp::WhereExp(std::unique_ptr<BooleanExp> expression) : Exp(Type::Where)
    {
    AddChild(std::move(expression));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BooleanExp const* WhereExp::GetSearchConditionExp() const
    {
    return GetChild<BooleanExp>(0);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    09/2015
//+---------------+---------------+---------------+---------------+---------------+--------
std::set<DbTable const*> WhereExp::GetReferencedTables() const
    {
    std::set<DbTable const*> tmp;
    if (!IsComplete())
        {
        BeAssert(false && "This operation is supported on resolved expressions");
        return tmp;
        }

    std::vector<Exp const*> expList = Find(Type::PropertyName, true);
    for (Exp const* exp : expList)
        {
        PropertyNameExp const& propertyNameExp = exp->GetAs<PropertyNameExp>();
        if (propertyNameExp.IsPropertyRef())
            continue;

        PropertyMap const* propertyMap = propertyNameExp.GetTypeInfo().GetPropertyMap();
        if (propertyMap->IsSystem())
            tmp.insert(&propertyMap->GetClassMap().GetJoinedTable());
        else
            tmp.insert(&propertyMap->GetAs<DataPropertyMap>().GetTable());
        }

    return tmp;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
