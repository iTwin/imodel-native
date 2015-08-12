/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ClassRefExp.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

#include "ClassRefExp.h"
#include "SelectStatementExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

using namespace std;

//****************************** ClassNameExp *****************************************

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ClassNameExp::_CreatePropertyNameExpList (ECSqlParseContext& ctx, std::function<void (std::unique_ptr<PropertyNameExp>&)> addDelegate) const 
    {
    BeAssert (m_info != nullptr);
    auto& classMap = m_info->GetMap();

    for(auto propertyMap : classMap.GetPropertyMaps ())
        {
        auto newExp = unique_ptr<PropertyNameExp>(new PropertyNameExp(propertyMap->GetPropertyAccessString(), *this, classMap));
        addDelegate (newExp);
        }

    return ECSqlStatus::Success;
    }


//****************************** RangeClassRefExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus RangeClassRefExp::CreatePropertyNameExpList (ECSqlParseContext& ctx, std::function<void (std::unique_ptr<PropertyNameExp>&)> addDelegate) const
    {
    return _CreatePropertyNameExpList (ctx, addDelegate);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool RangeClassRefExp::ContainProperty(Utf8CP propertyName) const
    {
    return _ContainProperty(propertyName);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
