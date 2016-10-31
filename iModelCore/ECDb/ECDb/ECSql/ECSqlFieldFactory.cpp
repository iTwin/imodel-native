/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlFieldFactory.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

    ECSqlSelectPreparedStatement* selectPreparedState = ctx.GetECSqlStatementR().GetPreparedStatementP <ECSqlSelectPreparedStatement>();

    ValueExp const* valueExp = derivedProperty->GetExpression();
    PropertyNameExp const* propNameExp = nullptr;
    if (valueExp->GetType() == Exp::Type::PropertyName)
        propNameExp = static_cast<PropertyNameExp const*>(valueExp);

    ECPropertyCP generatedProperty = nullptr;
    ECSqlStatus stat = selectPreparedState->GetDynamicSelectClauseECClassR().GeneratePropertyIfRequired(generatedProperty, ctx, *derivedProperty, propNameExp, selectPreparedState->GetECDb());
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
                stat = CreatePrimitiveArrayField(field, startColumnIndex, ctx, ecsqlColumnInfo, valueTypeInfo.GetPrimitiveType());
                break;

            case ECSqlTypeInfo::Kind::StructArray:
                stat = CreateStructArrayField(field, startColumnIndex, ctx, ecsqlColumnInfo);
                break;

            default:
                BeAssert(false && "Unhandled property type in value reader factory");
                return ECSqlStatus::Error;
        }

    if (stat.IsSuccess())
        selectPreparedState->AddField(move(field));

    return stat;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreatePrimitiveField(std::unique_ptr<ECSqlField>& field, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECSqlColumnInfo const& ecsqlColumnInfo, PrimitiveType primitiveType)
    {
    ECSqlStatementBase& ecsqlStmt = ctx.GetECSqlStatementR();

    switch (primitiveType)
        {
            case PRIMITIVETYPE_Point2d:
            {
            int xColumnIndex = sqlColumnIndex++;
            int yColumnIndex = sqlColumnIndex++;

            field = std::unique_ptr<ECSqlField>(new PointMappedToColumnsECSqlField(ecsqlStmt, ecsqlColumnInfo, xColumnIndex, yColumnIndex));
            break;
            }
            case PRIMITIVETYPE_Point3d:
            {
            int xColumnIndex = sqlColumnIndex++;
            int yColumnIndex = sqlColumnIndex++;
            int zColumnIndex = sqlColumnIndex++;

            field = std::unique_ptr<ECSqlField>(new PointMappedToColumnsECSqlField(ecsqlStmt, ecsqlColumnInfo, xColumnIndex, yColumnIndex, zColumnIndex));
            break;
            }
            default:
                field = std::unique_ptr<ECSqlField>(new PrimitiveMappedToSingleColumnECSqlField(ecsqlStmt, ecsqlColumnInfo, sqlColumnIndex++));
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
ECSqlStatus ECSqlFieldFactory::CreatePrimitiveArrayField(std::unique_ptr<ECSqlField>& field, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECSqlColumnInfo const& ecsqlColumnInfo, PrimitiveType primitiveType)
    {
    ECClassCP primArraySystemClass = ECDbSystemSchemaHelper::GetClassForPrimitiveArrayPersistence(ctx.GetECDb(), primitiveType);
    if (primArraySystemClass == nullptr)
        {
        BeAssert(false);
        return ECSqlStatus::Error;
        }

    field = std::unique_ptr<ECSqlField>(new PrimitiveArrayMappedToSingleColumnECSqlField(ctx.GetECSqlStatementR(), ecsqlColumnInfo, sqlColumnIndex++, *primArraySystemClass));
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreateStructArrayField(std::unique_ptr<ECSqlField>& field, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECSqlColumnInfo const& ecsqlColumnInfo)
    {
    field = std::unique_ptr<ECSqlField>(new StructArrayJsonECSqlField(ctx.GetECSqlStatementR(), ecsqlColumnInfo, sqlColumnIndex++));
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreateStructMemberFields(std::unique_ptr<ECSqlField>& structField, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECN::ECStructClassCR structType, ECSqlColumnInfo const& structFieldColumnInfo)
    {
    std::unique_ptr<StructMappedToColumnsECSqlField> newStructField(new StructMappedToColumnsECSqlField(ctx.GetECSqlStatementR(), structFieldColumnInfo));

    for (ECPropertyCP prop : structType.GetProperties())
        {
        std::unique_ptr<ECSqlField> memberField = nullptr;
        ECSqlStatus status = CreateStructMemberField(memberField, ctx, sqlColumnIndex, newStructField->GetColumnInfo(), *prop);
        if (!status.IsSuccess())
            return status;

        newStructField->AppendField(std::move(memberField));
        }

    structField = std::move(newStructField);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreateStructMemberField(std::unique_ptr<ECSqlField>& memberField, ECSqlPrepareContext& ctx, int& sqlColumnIndex, ECSqlColumnInfo const& structFieldColumnInfo, ECN::ECPropertyCR structMemberProperty)
    {
    ECSqlColumnInfo columnInfo = ECSqlColumnInfo::CreateChild(structFieldColumnInfo, structMemberProperty);

    if (structMemberProperty.GetIsStruct())
        {
        ECStructClassCR childStructType = structMemberProperty.GetAsStructProperty()->GetType();
        return CreateStructMemberFields(memberField, sqlColumnIndex, ctx, childStructType, columnInfo);
        }

    if (structMemberProperty.GetIsPrimitive())
        {
        PrimitiveType primitiveType = structMemberProperty.GetAsPrimitiveProperty()->GetType();
        return CreatePrimitiveField(memberField, sqlColumnIndex, ctx, columnInfo, primitiveType);
        }

    if (structMemberProperty.GetIsArray())
        {
        if (structMemberProperty.GetIsPrimitiveArray())
            {
            PrimitiveType primitiveType = structMemberProperty.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();
            return CreatePrimitiveArrayField(memberField, sqlColumnIndex, ctx, columnInfo, primitiveType);
            }

        return CreateStructArrayField(memberField, sqlColumnIndex, ctx, columnInfo);
        }

    if (structMemberProperty.GetIsNavigation())
        {
        NavigationECPropertyCP navProp = structMemberProperty.GetAsNavigationProperty();
        PrimitiveType navPropIdType = navProp->GetType();
        if (!navProp->IsMultiple())
            return CreatePrimitiveField(memberField, sqlColumnIndex, ctx, columnInfo, navPropIdType);

        return CreatePrimitiveArrayField(memberField, sqlColumnIndex, ctx, columnInfo, navPropIdType);
        }

    BeAssert(false && "No ECSqlField instantiated");
    return ECSqlStatus::Error;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlColumnInfo ECSqlFieldFactory::CreateECSqlColumnInfoFromPropertyNameExp(ECSqlPrepareContext const& ctx, PropertyNameExp const& propertyNameExp)
    {
    ECSqlPropertyPath ecsqlPropPath;

    if (ctx.GetParentColumnInfo() != nullptr)
        {
        ECSqlPropertyPathCR parentPropPath = ctx.GetParentColumnInfo()->GetPropertyPath();
        ecsqlPropPath.InsertEntriesAtBeginning(parentPropPath);
        }

    PropertyPath const& internalPropPath = propertyNameExp.GetPropertyPath();
    size_t entryCount = internalPropPath.Size();
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
    if (ctx.GetParentColumnInfo() != nullptr)
        {
        auto const& parentPropPath = ctx.GetParentColumnInfo()->GetPropertyPath();
        propertyPath.InsertEntriesAtBeginning(parentPropPath);
        }

    propertyPath.AddEntry(generatedProperty);

    return ECSqlColumnInfo::CreateTopLevel(true, std::move(propertyPath), generatedProperty.GetClass(), nullptr);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
