/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlBinderFactory.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

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
        ECSqlSystemPropertyInfo const& sysPropInfo = propNameExp->GetSystemPropertyInfo();
        if (sysPropInfo.IsSystemProperty() && sysPropInfo.IsId())
            return CreateIdBinder(ctx, propNameExp->GetPropertyMap(), sysPropInfo);
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
                        return std::unique_ptr<ECSqlBinder>(new PrimitiveECSqlBinder(ctx.GetECSqlStatementR(), typeInfo));

                    case ECN::PRIMITIVETYPE_Point2d:
                        return std::unique_ptr<ECSqlBinder>(new PointECSqlBinder(ctx.GetECSqlStatementR(), typeInfo, false));

                    case ECN::PRIMITIVETYPE_Point3d:
                        return std::unique_ptr<ECSqlBinder>(new PointECSqlBinder(ctx.GetECSqlStatementR(), typeInfo, true));

                    default:
                        BeAssert(false && "Could not create parameter mapping for the given parameter exp.");
                        return nullptr;
                }
            break;
            }
            //the rare case of expressions like this: NULL IS ?
            case ECSqlTypeInfo::Kind::Null:
                return std::unique_ptr<ECSqlBinder>(new PrimitiveECSqlBinder(ctx.GetECSqlStatementR(), typeInfo));

            case ECSqlTypeInfo::Kind::Struct:
            {
            std::unique_ptr<StructECSqlBinder> structBinder(new StructECSqlBinder(ctx.GetECSqlStatementR(), typeInfo));
            if (SUCCESS != structBinder->Initialize(ctx))
                return nullptr;

            return std::move(structBinder);
            }

            case ECSqlTypeInfo::Kind::PrimitiveArray:
            case ECSqlTypeInfo::Kind::StructArray:
                return std::unique_ptr<ECSqlBinder>(new ArrayECSqlBinder(ctx.GetECSqlStatementR(), typeInfo));
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
std::unique_ptr<IdECSqlBinder> ECSqlBinderFactory::CreateIdBinder(ECSqlPrepareContext& ctx, PropertyMap const& propMap, ECSqlSystemPropertyInfo const& sysPropertyInfo)
    {
    if (!sysPropertyInfo.IsId())
        {
        BeAssert(false);
        return nullptr;
        }

    const bool isNoopBinder = RequiresNoopBinder(ctx, propMap, sysPropertyInfo);
    return std::unique_ptr<IdECSqlBinder>(new IdECSqlBinder(ctx.GetECSqlStatementR(), ECSqlTypeInfo(propMap), isNoopBinder));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
bool ECSqlBinderFactory::RequiresNoopBinder(ECSqlPrepareContext& ctx, PropertyMap const& propMap, ECSqlSystemPropertyInfo const& sysPropertyInfo)
    {
    const ECSqlType ecsqlType = ctx.GetCurrentScope().GetECSqlType();
    if (ecsqlType == ECSqlType::Select || ecsqlType == ECSqlType::Delete ||
        (ecsqlType == ECSqlType::Update && ctx.GetCurrentScope().GetExp().GetType() != Exp::Type::AssignmentList))
        return false;


    //only INSERT and UPDATE SET clauses require no-op binders because they directly translate to columns. All other expressions
    //can use constant values for virtual columns and therefore don't need no-op binders.
    
    BeAssert((sysPropertyInfo.GetType() != ECSqlSystemPropertyInfo::Type::Class || sysPropertyInfo.GetClass() != ECSqlSystemPropertyInfo::Class::ECClassId) && "Inserting/updating ECClassId is not supported and should have been caught before");
    BeAssert(propMap.GetType() != PropertyMap::Type::ECClassId && "Inserting/updating ECClassId is not supported and should have been caught before");

    if (propMap.GetClassMap().GetType() == ClassMap::Type::RelationshipEndTable)
        {
        //for end table relationships we ignore 
        //* the user provided ECInstanceId as end table relationships don't have their own ECInstanceId
        //* this end's class id (foreign end class id) as it is the same the end's class ECClassId. It cannot be set through
        //an ECSQL INSERT INTO ECRel.
        RelationshipClassEndTableMap const& relClassMap = static_cast<RelationshipClassEndTableMap const&> (propMap.GetClassMap());
        if (sysPropertyInfo == ECSqlSystemPropertyInfo::ECInstanceId() || 
            sysPropertyInfo == (relClassMap.GetForeignEnd() == ECN::ECRelationshipEnd_Source ? ECSqlSystemPropertyInfo::SourceECClassId() : ECSqlSystemPropertyInfo::TargetECClassId()))
            return true;
        }

    switch (propMap.GetType())
        {
            case PropertyMap::Type::ConstraintECClassId:
            {
            ConstraintECClassIdPropertyMap const& constraintClassIdPropMap = *propMap.GetAs<ConstraintECClassIdPropertyMap>();
            if (nullptr != ConstraintECClassIdJoinInfo::RequiresJoinTo(constraintClassIdPropMap, true /*ignoreVirtualColumnCheck*/))
                return true;

            BeAssert(propMap.GetClassMap().GetTables().size() == 1 && constraintClassIdPropMap.GetTables().size() == 1);
            DbTable const* contextTable = &propMap.GetClassMap().GetJoinedTable();
            return constraintClassIdPropMap.IsVirtual(*contextTable);
            }
            case PropertyMap::Type::NavigationRelECClassId:
                return propMap.GetAs<NavigationPropertyMap::RelECClassIdPropertyMap>()->IsVirtual();

            default:
                return false;
        }
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
