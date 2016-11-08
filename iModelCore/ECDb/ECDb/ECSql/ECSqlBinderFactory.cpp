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
std::unique_ptr<ECSqlBinder> ECSqlBinderFactory::CreateBinder(ECSqlPrepareContext& ctx, ParameterExp const& parameterExp)
    {
    ComputedExp const* targetExp = parameterExp.GetTargetExp();
    if (targetExp != nullptr && targetExp->GetType() == Exp::Type::PropertyName)
        {
        BeAssert(dynamic_cast<PropertyNameExp const*> (targetExp) != nullptr);
        PropertyNameExp const* propNameExp = static_cast<PropertyNameExp const*> (targetExp);
        ECSqlSystemPropertyKind sysPropKind;
        if (propNameExp->TryGetSystemProperty(sysPropKind) && Enum::Contains(ECSqlSystemPropertyKind::IsId, sysPropKind))
            return CreateIdBinder(ctx, propNameExp->GetPropertyMap(), sysPropKind);
        }

    return CreateBinder(ctx, parameterExp.GetTypeInfo());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSqlBinder> ECSqlBinderFactory::CreateBinder(ECSqlPrepareContext& ctx, ECSqlTypeInfo const& typeInfo)
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
                        return std::unique_ptr<ECSqlBinder>(new PrimitiveToSingleColumnECSqlBinder(ctx.GetECSqlStatementR(), typeInfo));

                    case ECN::PRIMITIVETYPE_Point2d:
                        return std::unique_ptr<ECSqlBinder>(new PointToColumnsECSqlBinder(ctx.GetECSqlStatementR(), typeInfo, false));

                    case ECN::PRIMITIVETYPE_Point3d:
                        return std::unique_ptr<ECSqlBinder>(new PointToColumnsECSqlBinder(ctx.GetECSqlStatementR(), typeInfo, true));

                    default:
                        BeAssert(false && "Could not create parameter mapping for the given parameter exp.");
                        return nullptr;
                }
            break;
            }
            //the rare case of expressions like this: NULL IS ?
            case ECSqlTypeInfo::Kind::Null:
                return std::unique_ptr<ECSqlBinder>(new PrimitiveToSingleColumnECSqlBinder(ctx.GetECSqlStatementR(), typeInfo));

            case ECSqlTypeInfo::Kind::Struct:
            {
            std::unique_ptr<StructToColumnsECSqlBinder> structBinder(new StructToColumnsECSqlBinder(ctx.GetECSqlStatementR(), typeInfo));
            if (SUCCESS != structBinder->Initialize(ctx))
                return nullptr;

            return std::move(structBinder);
            }

            case ECSqlTypeInfo::Kind::PrimitiveArray:
                return std::unique_ptr<ECSqlBinder>(new PrimitiveArrayToColumnECSqlBinder(ctx.GetECSqlStatementR(), typeInfo));
            case ECSqlTypeInfo::Kind::StructArray:
                return std::unique_ptr<ECSqlBinder>(new StructArrayJsonECSqlBinder(ctx.GetECSqlStatementR(), typeInfo));
            case ECSqlTypeInfo::Kind::Navigation:
            {
            std::unique_ptr<NavigationPropertyECSqlBinder> navPropBinder(new NavigationPropertyECSqlBinder(ctx.GetECSqlStatementR(), typeInfo));
            if (SUCCESS != navPropBinder->Initialize(ctx))
                return nullptr;

            return std::move(navPropBinder);
            }

            default:
                BeAssert(false && "ECSqlBinderFactory::CreateBinder> Unhandled ECSqlTypeInfo::Kind value.");
                return nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
std::unique_ptr<IdECSqlBinder> ECSqlBinderFactory::CreateIdBinder(ECSqlPrepareContext& ctx, PropertyMap const& propMap, ECSqlSystemPropertyKind sysPropertyKind)
    {
    if (!Enum::Contains(ECSqlSystemPropertyKind::IsId, sysPropertyKind))
        {
        BeAssert(false);
        return nullptr;
        }

    const bool isNoopBinder = RequiresNoopBinder(ctx, propMap, sysPropertyKind);
    return std::unique_ptr<IdECSqlBinder>(new IdECSqlBinder(ctx.GetECSqlStatementR(), ECSqlTypeInfo(propMap), isNoopBinder));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
bool ECSqlBinderFactory::RequiresNoopBinder(ECSqlPrepareContext& ctx, PropertyMap const& propMap, ECSqlSystemPropertyKind sysPropertyKind)
    {
    //noop binder is only needed for INSERT as in that case we don't translate the parameter into a native SQL token.
    //The ECSQL user however might still bind to the ECSQL parameter which then is a no-op.
    if (ctx.GetCurrentScope().GetECSqlType() != ECSqlType::Insert)
        return false;

    BeAssert(ctx.GetCurrentScope().IsRootScope() && "Nested ECSQL INSERT is not expected to be supported by this code.");

    BeAssert(sysPropertyKind != ECSqlSystemPropertyKind::ECClassId && "Inserting into ECClassId is not supported and should have been caught before");

    if (propMap.GetClassMap().GetType() == ClassMap::Type::RelationshipEndTable)
        {
        //for end table relationships we ignore 
        //* the user provided ECInstanceId as end table relationships don't have their own ECInstanceId
        //* this end's class id (foreign end class id) as it is the same the end's class ECClassId. It cannot be set through
        //an ECSQL INSERT INTO ECRel.
        RelationshipClassEndTableMap const& relClassMap = static_cast<RelationshipClassEndTableMap const&> (propMap.GetClassMap());
        const ECSqlSystemPropertyKind foreignEndClassId = relClassMap.GetForeignEnd() == ECN::ECRelationshipEnd_Source ? ECSqlSystemPropertyKind::SourceECClassId : ECSqlSystemPropertyKind::TargetECClassId;
        if (sysPropertyKind == ECSqlSystemPropertyKind::ECInstanceId || sysPropertyKind == foreignEndClassId)
            return true;
        }

    switch (propMap.GetType())
        {
            case PropertyMap::Type::ConstraintECClassId:
            {
            ConstraintECClassIdPropertyMap const& constraintClassIdPropMap = static_cast<ConstraintECClassIdPropertyMap const&>(propMap);
            if (nullptr != ConstraintECClassIdJoinInfo::RequiresJoinTo(constraintClassIdPropMap, true /*ignoreVirtualColumnCheck*/))
                return true;

            BeAssert(propMap.GetClassMap().GetTables().size() == 1 && constraintClassIdPropMap.GetTables().size() == 1);
            DbTable const* contextTable = &propMap.GetClassMap().GetJoinedTable();
            return constraintClassIdPropMap.IsVirtual(*contextTable);
            }
            case PropertyMap::Type::ECClassId:
                //WIP_TABLECONTEXT
                return static_cast<ECClassIdPropertyMap const&>(propMap).IsVirtual(propMap.GetClassMap().GetJoinedTable());
            case PropertyMap::Type::NavigationRelECClassId:
                return static_cast<NavigationPropertyMap::RelECClassIdPropertyMap const&>(propMap).IsVirtual();

            default:
                return false;;
        }
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
