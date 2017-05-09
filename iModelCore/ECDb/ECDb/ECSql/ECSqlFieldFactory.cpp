/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlFieldFactory.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlFieldFactory.h"
#include "ECSqlStatementNoopImpls.h"
#include "ECSqlPreparer.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreateField(ECSqlPrepareContext& ctx, DerivedPropertyExp const* derivedProperty, int startColumnIndex)
    {
    BeAssert(derivedProperty != nullptr && derivedProperty->IsComplete());

    ECSqlSelectPreparedStatement& selectPreparedStatement = GetPreparedStatement(ctx);

    ValueExp const* valueExp = derivedProperty->GetExpression();
    PropertyNameExp const* propNameExp = nullptr;
    if (valueExp->GetType() == Exp::Type::PropertyName)
        propNameExp = valueExp->GetAsCP<PropertyNameExp>();

    ECPropertyCP generatedProperty = nullptr;
    ECSqlStatus stat = selectPreparedStatement.GetDynamicSelectClauseECClassR().GeneratePropertyIfRequired(generatedProperty, ctx, *derivedProperty, propNameExp);
    if (!stat.IsSuccess())
        return stat;

    ECSqlColumnInfo ecsqlColumnInfo = generatedProperty != nullptr ? 
                CreateECSqlColumnInfoFromGeneratedProperty(ctx, *generatedProperty) :
                CreateECSqlColumnInfoFromPropertyNameExp(ctx, *propNameExp);

    ECSqlTypeInfo const& valueTypeInfo = valueExp->GetTypeInfo();
    BeAssert(valueTypeInfo.GetKind() != ECSqlTypeInfo::Kind::Unset);

    std::unique_ptr<ECSqlField> field = nullptr;
    stat = ECSqlStatus::Success;
    switch (valueTypeInfo.GetKind())
        {
            case ECSqlTypeInfo::Kind::Primitive:
            case ECSqlTypeInfo::Kind::Null:
                stat = CreatePrimitiveField(field, startColumnIndex, ctx, ecsqlColumnInfo, valueTypeInfo.GetPrimitiveType());
                break;

            case ECSqlTypeInfo::Kind::Struct:
                stat = CreateStructField(field, startColumnIndex, ctx, ecsqlColumnInfo, valueTypeInfo.GetStructType());
                break;

            case ECSqlTypeInfo::Kind::PrimitiveArray:
            case ECSqlTypeInfo::Kind::StructArray:
                stat = CreateArrayField(field, startColumnIndex, ctx, ecsqlColumnInfo);
                break;

            case ECSqlTypeInfo::Kind::Navigation:
                stat = CreateNavigationPropertyField(field, startColumnIndex, ctx, ecsqlColumnInfo);
                break;

            default:
                BeAssert(false && "Unhandled property type in value reader factory");
                return ECSqlStatus::Error;
        }

    if (stat.IsSuccess())
        selectPreparedStatement.AddField(move(field));

    return stat;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreateChildField(std::unique_ptr<ECSqlField>& childField, ECSqlPrepareContext& ctx, int& sqlColumnIndex, ECSqlColumnInfo const& parentFieldColumnInfo, ECN::ECPropertyCR childProperty)
    {
    ECSqlColumnInfo columnInfo = ECSqlColumnInfo::CreateChild(parentFieldColumnInfo, childProperty);

    if (childProperty.GetIsStruct())
        {
        ECStructClassCR childStructType = childProperty.GetAsStructProperty()->GetType();
        return CreateStructMemberFields(childField, sqlColumnIndex, ctx, childStructType, columnInfo);
        }

    if (childProperty.GetIsPrimitive())
        {
        PrimitiveType primitiveType = childProperty.GetAsPrimitiveProperty()->GetType();
        return CreatePrimitiveField(childField, sqlColumnIndex, ctx, columnInfo, primitiveType);
        }

    if (childProperty.GetIsArray())
        return CreateArrayField(childField, sqlColumnIndex, ctx, columnInfo);

    if (childProperty.GetIsNavigation())
        {
        BeAssert(false && "Navigation properties cannot be used in an ECStruct");
        return ECSqlStatus::Error;
        }

    BeAssert(false && "No ECSqlField instantiated");
    return ECSqlStatus::Error;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreatePrimitiveField(std::unique_ptr<ECSqlField>& field, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECSqlColumnInfo const& ecsqlColumnInfo, PrimitiveType primitiveType)
    {
    ECSqlSelectPreparedStatement& selectPreparedStatement = GetPreparedStatement(ctx);

    switch (primitiveType)
        {
            case PRIMITIVETYPE_Point2d:
            {
            int xColumnIndex = sqlColumnIndex++;
            int yColumnIndex = sqlColumnIndex++;

            field = std::unique_ptr<ECSqlField>(new PointECSqlField(selectPreparedStatement, ecsqlColumnInfo, xColumnIndex, yColumnIndex));
            break;
            }
            case PRIMITIVETYPE_Point3d:
            {
            int xColumnIndex = sqlColumnIndex++;
            int yColumnIndex = sqlColumnIndex++;
            int zColumnIndex = sqlColumnIndex++;

            field = std::unique_ptr<ECSqlField>(new PointECSqlField(selectPreparedStatement, ecsqlColumnInfo, xColumnIndex, yColumnIndex, zColumnIndex));
            break;
            }
            default:
                field = std::unique_ptr<ECSqlField>(new PrimitiveECSqlField(selectPreparedStatement, ecsqlColumnInfo, sqlColumnIndex++));
                break;
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreateStructField(std::unique_ptr<ECSqlField>& field, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECSqlColumnInfo const& ecsqlColumnInfo, ECN::ECStructClassCR structType)
    {
    return CreateStructMemberFields(field, sqlColumnIndex, ctx, structType, ecsqlColumnInfo);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreateNavigationPropertyField(std::unique_ptr<ECSqlField>& field, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECSqlColumnInfo const& ecsqlColumnInfo)
    {
    ECSqlSelectPreparedStatement& selectPreparedStatement = GetPreparedStatement(ctx);

    std::unique_ptr<NavigationPropertyECSqlField> newField(new NavigationPropertyECSqlField(selectPreparedStatement, ecsqlColumnInfo));

    std::unique_ptr<ECSqlField> idField = nullptr;
    ECSqlStatus stat = CreateChildField(idField, ctx, sqlColumnIndex, newField->GetColumnInfo(), *ctx.GetECDb().Schemas().GetReader().GetSystemSchemaHelper().GetSystemProperty(ECSqlSystemPropertyInfo::NavigationId()));
    if (!stat.IsSuccess())
        return stat;
    
    std::unique_ptr<ECSqlField> relClassIdField = nullptr;
    stat = CreateChildField(relClassIdField, ctx, sqlColumnIndex, newField->GetColumnInfo(), *ctx.GetECDb().Schemas().GetReader().GetSystemSchemaHelper().GetSystemProperty(ECSqlSystemPropertyInfo::NavigationRelECClassId()));
    if (!stat.IsSuccess())
        return stat;

    newField->SetMembers(std::move(idField), std::move(relClassIdField));
    field = std::move(newField);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreateArrayField(std::unique_ptr<ECSqlField>& field, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECSqlColumnInfo const& ecsqlColumnInfo)
    {
    field = std::unique_ptr<ECSqlField>(new ArrayECSqlField(GetPreparedStatement(ctx), ecsqlColumnInfo, sqlColumnIndex++));
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreateStructMemberFields(std::unique_ptr<ECSqlField>& structField, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECN::ECStructClassCR structType, ECSqlColumnInfo const& structFieldColumnInfo)
    {
    std::unique_ptr<StructECSqlField> newStructField(new StructECSqlField(GetPreparedStatement(ctx), structFieldColumnInfo));

    for (ECPropertyCP prop : structType.GetProperties())
        {
        std::unique_ptr<ECSqlField> memberField = nullptr;
        ECSqlStatus status = CreateChildField(memberField, ctx, sqlColumnIndex, newStructField->GetColumnInfo(), *prop);
        if (!status.IsSuccess())
            return status;

        newStructField->AppendField(std::move(memberField));
        }

    structField = std::move(newStructField);
    return ECSqlStatus::Success;
    }



//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlColumnInfo ECSqlFieldFactory::CreateECSqlColumnInfoFromPropertyNameExp(ECSqlPrepareContext const& ctx, PropertyNameExp const& propertyNameExp)
    {
    PropertyPath const& internalPropPath = propertyNameExp.GetPropertyPath();
    size_t entryCount = internalPropPath.Size();
    ECSqlPropertyPath ecsqlPropPath;
    for (size_t i = 0; i < entryCount; i++)
        {
        PropertyPath::Location const& internalEntry = internalPropPath[i];
        ecsqlPropPath.AddEntry(*internalEntry.GetProperty());
        }

    BeAssert(ecsqlPropPath.Size() > 0 && "Error in program logic. Property path must not be empty.");

    ECClassCR& rootClass = internalPropPath.GetClassMap()->GetClass();
    return ECSqlColumnInfo::CreateTopLevel(false, std::move(ecsqlPropPath), rootClass, propertyNameExp.GetClassAlias());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlColumnInfo ECSqlFieldFactory::CreateECSqlColumnInfoFromGeneratedProperty(ECSqlPrepareContext const& ctx, ECPropertyCR generatedProperty)
    {
    ECSqlPropertyPath propertyPath;
    propertyPath.AddEntry(generatedProperty);
    return ECSqlColumnInfo::CreateTopLevel(true, std::move(propertyPath), generatedProperty.GetClass(), nullptr);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlSelectPreparedStatement& ECSqlFieldFactory::GetPreparedStatement(ECSqlPrepareContext& ctx) { return ctx.GetPreparedStatement<ECSqlSelectPreparedStatement>(); }

END_BENTLEY_SQLITE_EC_NAMESPACE
