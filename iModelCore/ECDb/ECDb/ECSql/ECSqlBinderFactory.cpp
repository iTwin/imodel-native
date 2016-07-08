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
#include "SystemPropertyECSqlBinder.h"
#include "ECSqlStatementBase.h"
#include "ECSqlBinder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//****************** ECSqlBinderFactory **************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSqlBinder> ECSqlBinderFactory::CreateBinder(ECSqlStatementBase& ecsqlStatement, ParameterExp const& parameterExp,
                                                         bool targetIsVirtual, bool enforceConstraints)
    {
    ECSqlTypeInfo const& typeInfo = parameterExp.GetTypeInfo();
    ComputedExp const* targetExp = parameterExp.GetTargetExp();
    if (targetExp != nullptr && targetExp->GetType() == Exp::Type::PropertyName)
        {
        BeAssert(dynamic_cast<PropertyNameExp const*> (targetExp) != nullptr);
        PropertyNameExp const* propNameExp = static_cast<PropertyNameExp const*> (targetExp);
        if (propNameExp->IsSystemProperty())
            return std::unique_ptr<ECSqlBinder>(new SystemPropertyECSqlBinder(ecsqlStatement, typeInfo, *propNameExp, targetIsVirtual, enforceConstraints));
        }

    return CreateBinder(ecsqlStatement, typeInfo);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSqlBinder> ECSqlBinderFactory::CreateBinder(ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& typeInfo)
    {
    auto typeKind = typeInfo.GetKind();
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

                    case ECN::PRIMITIVETYPE_Point2D:
                        return std::unique_ptr<ECSqlBinder>(new PointToColumnsECSqlBinder(ecsqlStatement, typeInfo, false));

                    case ECN::PRIMITIVETYPE_Point3D:
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

            default:
                BeAssert(false && "ECSqlBinderFactory::CreateBinder> Unhandled ECSqlTypeInfo::Kind value.");
                return nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSqlBinder> ECSqlBinderFactory::CreateBinder(ECSqlStatementBase& ecsqlStatement, PropertyMapCR propMap)
    {
    return CreateBinder(ecsqlStatement, ECSqlTypeInfo(propMap));
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
