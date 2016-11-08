/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlBinderFactory.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlBinderFactory.h"
#include "PrimitiveToSingleColumnECSqlBinder.h"
#include "PointECSqlBinder.h"
#include "StructToColumnsECSqlBinder.h"
#include "PrimitiveArrayToColumnECSqlBinder.h"
#include "StructArrayJsonECSqlBinder.h"
#include "NavigationPropertyECSqlBinder.h"
#include "IdECSqlBinder.h"
#include "ECSqlStatementBase.h"
#include "ECSqlBinder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//****************** ECSqlBinderFactory **************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSqlBinder> ECSqlBinderFactory::CreateBinder(ECSqlStatementBase& ecsqlStatement, ParameterExp const& parameterExp)
    {
    ComputedExp const* targetExp = parameterExp.GetTargetExp();
    if (targetExp != nullptr && targetExp->GetType() == Exp::Type::PropertyName)
        {
        BeAssert(dynamic_cast<PropertyNameExp const*> (targetExp) != nullptr);
        PropertyNameExp const* propNameExp = static_cast<PropertyNameExp const*> (targetExp);
        ECSqlSystemPropertyKind sysPropKind;
        if (propNameExp->TryGetSystemProperty(sysPropKind) && Enum::Contains(ECSqlSystemPropertyKind::IsId, sysPropKind))
            return CreateIdBinder(ecsqlStatement, propNameExp->GetPropertyMap(), sysPropKind);
        }

    return CreateBinder(ecsqlStatement, parameterExp.GetTypeInfo());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSqlBinder> ECSqlBinderFactory::CreateBinder(ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& typeInfo)
    {
    ECSqlTypeInfo::Kind typeKind = typeInfo.GetKind();
    BeAssert(typeKind != ECSqlTypeInfo::Kind::Unset);

    switch (typeKind)
        {
            case ECSqlTypeInfo::Kind::Primitive:
            {
            switch (typeInfo.GetPrimitiveType())
                {
                    case ECN::PRIMITIVETYPE_Binary:
                    case ECN::PRIMITIVETYPE_Boolean:
                    case ECN::PRIMITIVETYPE_DateTime:
                    case ECN::PRIMITIVETYPE_Double:
                    case ECN::PRIMITIVETYPE_IGeometry:
                    case ECN::PRIMITIVETYPE_Integer:
                    case ECN::PRIMITIVETYPE_Long:
                    case ECN::PRIMITIVETYPE_String:
                        return std::unique_ptr<ECSqlBinder>(new PrimitiveToSingleColumnECSqlBinder(ecsqlStatement, typeInfo));

                    case ECN::PRIMITIVETYPE_Point2d:
                        return std::unique_ptr<ECSqlBinder>(new PointToColumnsECSqlBinder(ecsqlStatement, typeInfo, false));

                    case ECN::PRIMITIVETYPE_Point3d:
                        return std::unique_ptr<ECSqlBinder>(new PointToColumnsECSqlBinder(ecsqlStatement, typeInfo, true));

                    default:
                        BeAssert(false && "Could not create parameter mapping for the given parameter exp.");
                        return nullptr;
                }
            break;
            }
            //the rare case of expressions like this: NULL IS ?
            case ECSqlTypeInfo::Kind::Null:
                return std::unique_ptr<ECSqlBinder>(new PrimitiveToSingleColumnECSqlBinder(ecsqlStatement, typeInfo));

            case ECSqlTypeInfo::Kind::Struct:
                return std::unique_ptr<ECSqlBinder>(new StructToColumnsECSqlBinder(ecsqlStatement, typeInfo));
            case ECSqlTypeInfo::Kind::PrimitiveArray:
                return std::unique_ptr<ECSqlBinder>(new PrimitiveArrayToColumnECSqlBinder(ecsqlStatement, typeInfo));
            case ECSqlTypeInfo::Kind::StructArray:
                return std::unique_ptr<ECSqlBinder>(new StructArrayJsonECSqlBinder(ecsqlStatement, typeInfo));
            case ECSqlTypeInfo::Kind::Navigation:
                return std::unique_ptr<ECSqlBinder>(new NavigationPropertyECSqlBinder(ecsqlStatement, typeInfo));

            default:
                BeAssert(false && "ECSqlBinderFactory::CreateBinder> Unhandled ECSqlTypeInfo::Kind value.");
                return nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
std::unique_ptr<IdECSqlBinder> ECSqlBinderFactory::CreateIdBinder(ECSqlStatementBase& ecsqlStatement, PropertyMap const& propMap, ECSqlSystemPropertyKind sysPropertyKind)
    {
   
    if (!Enum::Contains(ECSqlSystemPropertyKind::IsId, sysPropertyKind))
        {
        BeAssert(false);
        return nullptr;
        }

    bool isNoopBinder = false;
    if (propMap.GetClassMap().GetType() == ClassMap::Type::RelationshipEndTable && sysPropertyKind == ECSqlSystemPropertyKind::ECInstanceId)
        {
        //for end table relationships we ignore the user provided ECInstanceId
        //as end table relationships don't have their own ECInstanceId
        isNoopBinder = true;
        }
    else
        {
        switch (propMap.GetType())
            {
                case PropertyMap::Type::ConstraintECClassId:
                {
                //WIP_TABLECONTEXT
                ConstraintECClassIdPropertyMap const& m = static_cast<ConstraintECClassIdPropertyMap const&>(propMap);
                
                if (DbTable const* table = ConstraintECClassIdJoinInfo::RequiresJoinTo(m, true /*ignoreVirtualColumnCheck*/))
                    {
                    isNoopBinder = m.IsVirtual(*table);
                    }
                else
                    {
                    isNoopBinder = static_cast<ConstraintECClassIdPropertyMap const&>(propMap).IsVirtual(propMap.GetClassMap().GetJoinedTable());
                    }

                break;
                }
                case PropertyMap::Type::ECClassId:
                    //WIP_TABLECONTEXT
                    isNoopBinder = static_cast<ECClassIdPropertyMap const&>(propMap).IsVirtual(propMap.GetClassMap().GetJoinedTable());
                    break;
                case PropertyMap::Type::NavigationRelECClassId:
                    isNoopBinder = static_cast<NavigationPropertyMap::RelECClassIdPropertyMap const&>(propMap).IsVirtual();
                    break;

                default:
                    break;
            }
        }

    return std::unique_ptr<IdECSqlBinder>(new IdECSqlBinder(ecsqlStatement, ECSqlTypeInfo(propMap), isNoopBinder));
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
