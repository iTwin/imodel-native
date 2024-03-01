/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlFieldFactory.h"
#include "ECSqlStatementNoopImpls.h"
#include "ECSqlPreparer.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreateField(ECSqlPrepareContext& ctx, DerivedPropertyExp const* derivedProperty, int startColumnIndex)
    {
    BeAssert(derivedProperty != nullptr && derivedProperty->IsComplete());

    ECSqlSelectPreparedStatement& selectPreparedStatement = GetPreparedStatement(ctx);
    ExtractPropertyValueExp const* extractPropExp = derivedProperty->TryGetExtractPropExp();
    if (extractPropExp) {
        // replace anchor for extract function with actual parameter.
        auto& anchorName = extractPropExp->GetSqlAnchor([&](Utf8CP name) {
            return ctx.GetAnchors().CreateAnchor(name);
        });
        ctx.GetAnchors().QueueReplacementForAnchor(anchorName, SqlPrintfString(",%s,%d", ctx.GetThisStmtPtrParamDecl(), startColumnIndex));
    }

    ValueExp const* valueExp = derivedProperty->GetExpression();
    PropertyNameExp const* propNameExp = nullptr;
    if (valueExp->GetType() == Exp::Type::PropertyName && !extractPropExp)
        propNameExp = valueExp->GetAsCP<PropertyNameExp>();


    ECPropertyCP generatedProperty = nullptr;
    ECSqlStatus stat = selectPreparedStatement.GetDynamicSelectClauseECClassR().GeneratePropertyIfRequired(generatedProperty, ctx, *derivedProperty, propNameExp, extractPropExp != nullptr);
    if (!stat.IsSuccess())
        return stat;

    if(generatedProperty == nullptr && propNameExp == nullptr)
        return ECSqlStatus::Error;

    ECSqlColumnInfo ecsqlColumnInfo = CreateColumnInfoForProperty(ctx, generatedProperty, propNameExp,  extractPropExp != nullptr);

    ECSqlTypeInfo const& valueTypeInfo = valueExp->GetTypeInfo();
    BeAssert(valueTypeInfo.GetKind() != ECSqlTypeInfo::Kind::Unset);

    std::unique_ptr<ECSqlField> field = nullptr;
    stat = ECSqlStatus::Success;
    switch (valueTypeInfo.GetKind())
        {
            case ECSqlTypeInfo::Kind::Null:
                stat = CreateNullField(field, startColumnIndex, ctx, ecsqlColumnInfo);
                break;

            case ECSqlTypeInfo::Kind::Primitive:
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
        selectPreparedStatement.AddField(std::move(field));

    return stat;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlColumnInfo ECSqlFieldFactory::CreateColumnInfoForProperty(ECSqlPrepareContext const& ctx, ECPropertyCP generatedProperty, PropertyNameExp const* propertyNameExp, bool isDynamic)
    {
    bool isGenerated = generatedProperty != nullptr;
    bool isSystem = false;
    ECSqlPropertyPath propertyPath;
    ECClassCP rootClass = nullptr;
    PropertyNameExp const* resolvedPropertyName = propertyNameExp;
    bool propertyNameExpRefersToProperty = propertyNameExp != nullptr;
    //in this bool we perform additional checks below and may set it to false if the target is not a property
    if(propertyNameExpRefersToProperty && propertyNameExp->IsPropertyRef())
        {
        auto* propRef = propertyNameExp->GetPropertyRef();
        propertyNameExpRefersToProperty = propRef->IsPure() &&
                                          propRef->GetEndPointDerivedProperty().GetExpression()->GetType() == Exp::Type::PropertyName;

        if(propertyNameExpRefersToProperty)
            {
            const auto virtualProp = propRef->TryGetVirtualProperty();
            if (virtualProp || !propRef->TryGetPropertyMap())
                resolvedPropertyName = propRef->GetEndPointDerivedProperty().GetExpression()->GetAsCP<PropertyNameExp>();
            }
        }

    if(isGenerated)
        {
        isSystem = generatedProperty->HasId() && ctx.GetECDb().Schemas().Main().GetSystemSchemaHelper().GetSystemPropertyInfo(*generatedProperty).IsSystemProperty();
        propertyPath.AddEntry(*generatedProperty);
        rootClass = &generatedProperty->GetClass();

        if(!propertyNameExpRefersToProperty)
            {
            return CreateTopLevelColumnInfo(ctx.Issues(), isSystem, true, std::move(propertyPath), ECSqlColumnInfo::RootClass(*rootClass, nullptr), nullptr, isDynamic);
            }
        }

    PropertyPath const& internalPropPath = resolvedPropertyName->GetPropertyPath();
    size_t entryCount = internalPropPath.Size();
    ECSqlPropertyPath ecsqlPropPath;
    for (size_t i = 0; i < entryCount; i++)
        {
        PropertyPath::Location const& internalEntry = internalPropPath[i];
        ecsqlPropPath.AddEntry(*internalEntry.GetProperty());
        }

    BeAssert(ecsqlPropPath.Size() > 0 && "Error in program logic. Property path must not be empty.");
    const auto isVirtualProperty = propertyNameExp->IsVirtualProperty();
    if (isVirtualProperty)
        {
        if(isGenerated)
            {
            return CreateTopLevelColumnInfo(ctx.Issues(), isSystem, true, std::move(propertyPath), ECSqlColumnInfo::RootClass(*rootClass, nullptr), ecsqlPropPath.GetLeafEntry().GetProperty(), isDynamic);
            }

        ECPropertyCP origProp = ecsqlPropPath.GetLeafEntry().GetProperty();
        ECPropertyCP leafProp = internalPropPath.Last().GetProperty();
        rootClass = &leafProp->GetClass();
        return CreateTopLevelColumnInfo(ctx.Issues(), false, false, std::move(ecsqlPropPath), ECSqlColumnInfo::RootClass(*rootClass, "", rootClass->GetName().c_str()), origProp, isDynamic);
        }

    ECPropertyCP leafProp = ecsqlPropPath.GetLeafEntry().GetProperty();
    if(isGenerated)
        {
        return CreateTopLevelColumnInfo(ctx.Issues(), isSystem, true, std::move(propertyPath), ECSqlColumnInfo::RootClass(*rootClass, nullptr), ecsqlPropPath.GetLeafEntry().GetProperty(), isDynamic);
        }

    BeAssert((internalPropPath.GetClassMap() != nullptr) && "Error in program logic. PropertyPath must have been resolved.");
    ECClassCR ecClass = internalPropPath.GetClassMap()->GetClass();
    isSystem = leafProp != nullptr && ctx.GetECDb().Schemas().Main().GetSystemSchemaHelper().GetSystemPropertyInfo(*leafProp).IsSystemProperty();
    Utf8CP tableSpace = resolvedPropertyName->GetPropertyMap()->GetClassMap().GetSchemaManager().GetTableSpace().GetName().c_str();
    return CreateTopLevelColumnInfo(ctx.Issues(), isSystem, false, std::move(ecsqlPropPath), ECSqlColumnInfo::RootClass(ecClass, tableSpace, resolvedPropertyName->GetClassName()), leafProp, isDynamic);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlColumnInfo ECSqlFieldFactory::CreateTopLevelColumnInfo(IssueDataSource const& issues, bool isSystemProperty, bool isGeneratedProperty, ECSqlPropertyPath const& propertyPath, ECSqlColumnInfo::RootClass const& rootClass, ECPropertyCP originalProperty, bool isDynamic)
    {
    BeAssert(propertyPath.Size() > 0);
    ECPropertyCP ecProperty = propertyPath.GetLeafEntry().GetProperty();
    BeAssert(ecProperty != nullptr);
    DateTime::Info dateTimeInfo;
    ECStructClassCP structType = nullptr;
    ECTypeDescriptor typeDescriptor = DetermineDataType(dateTimeInfo, structType, issues, *ecProperty);
    return ECSqlColumnInfo(typeDescriptor, dateTimeInfo, structType, ecProperty, originalProperty, isSystemProperty, isGeneratedProperty, propertyPath, rootClass, isDynamic);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreateChildField(std::unique_ptr<ECSqlField>& childField, ECSqlPrepareContext& ctx, int& sqlColumnIndex, ECSqlColumnInfo const& parentFieldColumnInfo, ECN::ECPropertyCR childProperty)
    {
    ECSqlColumnInfo columnInfo = CreateChildColumnInfo(ctx.Issues(), parentFieldColumnInfo, childProperty, ctx.GetECDb().Schemas().Main().GetSystemSchemaHelper().GetSystemPropertyInfo(childProperty).IsSystemProperty());

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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreateNullField(std::unique_ptr<ECSqlField>& field, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECSqlColumnInfo const& ecsqlColumnInfo)
    {
    ECSqlSelectPreparedStatement& selectPreparedStatement = GetPreparedStatement(ctx);
    field = std::make_unique<PrimitiveECSqlField>(selectPreparedStatement, ecsqlColumnInfo, sqlColumnIndex++);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
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

            field = std::make_unique<PointECSqlField>(selectPreparedStatement, ecsqlColumnInfo, xColumnIndex, yColumnIndex);
            break;
            }
            case PRIMITIVETYPE_Point3d:
            {
            int xColumnIndex = sqlColumnIndex++;
            int yColumnIndex = sqlColumnIndex++;
            int zColumnIndex = sqlColumnIndex++;

            field = std::make_unique<PointECSqlField>(selectPreparedStatement, ecsqlColumnInfo, xColumnIndex, yColumnIndex, zColumnIndex);
            break;
            }
            default:
                field = std::make_unique<PrimitiveECSqlField>(selectPreparedStatement, ecsqlColumnInfo, sqlColumnIndex++);
                break;
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreateStructField(std::unique_ptr<ECSqlField>& field, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECSqlColumnInfo const& ecsqlColumnInfo, ECN::ECStructClassCR structType)
    {
    return CreateStructMemberFields(field, sqlColumnIndex, ctx, structType, ecsqlColumnInfo);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreateNavigationPropertyField(std::unique_ptr<ECSqlField>& field, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECSqlColumnInfo const& ecsqlColumnInfo)
    {
    ECSqlSelectPreparedStatement& selectPreparedStatement = GetPreparedStatement(ctx);

    std::unique_ptr<NavigationPropertyECSqlField> newField = std::make_unique<NavigationPropertyECSqlField>(selectPreparedStatement, ecsqlColumnInfo);

    std::unique_ptr<ECSqlField> idField = nullptr;
    ECSqlStatus stat = CreateChildField(idField, ctx, sqlColumnIndex, newField->GetColumnInfo(), *ctx.GetECDb().Schemas().Main().GetSystemSchemaHelper().GetSystemProperty(ECSqlSystemPropertyInfo::NavigationId()));
    if (!stat.IsSuccess())
        return stat;

    std::unique_ptr<ECSqlField> relClassIdField = nullptr;
    stat = CreateChildField(relClassIdField, ctx, sqlColumnIndex, newField->GetColumnInfo(), *ctx.GetECDb().Schemas().Main().GetSystemSchemaHelper().GetSystemProperty(ECSqlSystemPropertyInfo::NavigationRelECClassId()));
    if (!stat.IsSuccess())
        return stat;

    newField->SetMembers(std::move(idField), std::move(relClassIdField));
    field = std::move(newField);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreateArrayField(std::unique_ptr<ECSqlField>& field, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECSqlColumnInfo const& ecsqlColumnInfo)
    {
    field = std::make_unique<ArrayECSqlField>(GetPreparedStatement(ctx), ecsqlColumnInfo, sqlColumnIndex++);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreateStructMemberFields(std::unique_ptr<ECSqlField>& structField, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECN::ECStructClassCR structType, ECSqlColumnInfo const& structFieldColumnInfo)
    {
    std::unique_ptr<StructECSqlField> newStructField = std::make_unique<StructECSqlField>(GetPreparedStatement(ctx), structFieldColumnInfo);

    for (ECPropertyCP prop : structType.GetProperties(true))
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlSelectPreparedStatement& ECSqlFieldFactory::GetPreparedStatement(ECSqlPrepareContext& ctx) { return ctx.GetPreparedStatement<ECSqlSelectPreparedStatement>(); }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlColumnInfo ECSqlFieldFactory::CreateChildColumnInfo(IssueDataSource const& issues, ECSqlColumnInfo const& parent, ECPropertyCR childProperty, bool isSystemProperty)
    {
    DateTime::Info dateTimeInfo;
    ECStructClassCP structType = nullptr;
    ECTypeDescriptor dataType = DetermineDataType(dateTimeInfo, structType, issues, childProperty);

    ECSqlPropertyPath childPropPath;
    childPropPath.InsertEntriesAtBeginning(parent.GetPropertyPath());
    childPropPath.AddEntry(childProperty);

    return ECSqlColumnInfo(dataType, dateTimeInfo, structType, &childProperty, parent.GetOriginProperty(), isSystemProperty, parent.IsGeneratedProperty(), childPropPath, parent.GetRootClass());
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlColumnInfo ECSqlFieldFactory::CreateColumnInfoForArrayElement(ECSqlColumnInfo const& parent, int arrayIndex)
    {
    ECTypeDescriptor arrayElementDataType;
    DateTime::Info dateTimeInfo;
    ECStructClassCP structType = nullptr;
    if (parent.GetDataType().IsPrimitiveArray())
        {
        arrayElementDataType = ECTypeDescriptor::CreatePrimitiveTypeDescriptor(parent.GetDataType().GetPrimitiveType());
        if (arrayElementDataType == PRIMITIVETYPE_DateTime)
            dateTimeInfo = parent.GetDateTimeInfo();
        }
    else
        {
        BeAssert(parent.GetDataType().IsStructArray());
        structType = parent.GetStructType();
        arrayElementDataType = ECTypeDescriptor::CreateStructTypeDescriptor();
        }

    ECSqlPropertyPath childPropPath;
    childPropPath.InsertEntriesAtBeginning(parent.GetPropertyPath());
    childPropPath.AddEntry(arrayIndex);

    return ECSqlColumnInfo(arrayElementDataType, dateTimeInfo, structType, nullptr, parent.GetOriginProperty(), false, parent.IsGeneratedProperty(), childPropPath, parent.GetRootClass());
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECTypeDescriptor ECSqlFieldFactory::DetermineDataType(DateTime::Info& dateTimeInfo, ECN::ECStructClassCP& structType, IssueDataSource const& issues, ECPropertyCR ecProperty)
    {
    if (ecProperty.GetIsPrimitive())
        {
        const PrimitiveType primType = ecProperty.GetAsPrimitiveProperty()->GetType();
        if (primType == PRIMITIVETYPE_DateTime)
            {
            if (ECObjectsStatus::Success != CoreCustomAttributeHelper::GetDateTimeInfo(dateTimeInfo, ecProperty))
                {
                issues.ReportV(
                    IssueSeverity::Error,
                    IssueCategory::BusinessProperties,
                    IssueType::ECSQL,
                    ECDbIssueId::ECDb_0469,
                    "Could not read DateTimeInfo custom attribute from the primitive ECProperty %s:%s.",
                    ecProperty.GetClass().GetFullName(),
                    ecProperty.GetName().c_str()
                );
                }
            }

        return ECTypeDescriptor::CreatePrimitiveTypeDescriptor(primType);
        }

    if (ecProperty.GetIsStruct())
        {
        structType = &ecProperty.GetAsStructProperty()->GetType();
        return ECTypeDescriptor::CreateStructTypeDescriptor();
        }

    if (ecProperty.GetIsPrimitiveArray())
        {
        const PrimitiveType primType = ecProperty.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();
        if (primType == PRIMITIVETYPE_DateTime)
            {
            if (ECObjectsStatus::Success != CoreCustomAttributeHelper::GetDateTimeInfo(dateTimeInfo, ecProperty))
                {
                issues.ReportV(
                    IssueSeverity::Error,
                    IssueCategory::BusinessProperties,
                    IssueType::ECSQL,
                    ECDbIssueId::ECDb_0470,
                    "Could not read DateTimeInfo custom attribute from the primitive array ECProperty %s:%s.",
                    ecProperty.GetClass().GetFullName(),
                    ecProperty.GetName().c_str()
                );
                }
            }

        return ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor(primType);
        }

    if (ecProperty.GetIsStructArray())
        {
        structType = &ecProperty.GetAsStructArrayProperty()->GetStructElementType();
        return ECTypeDescriptor::CreateStructArrayTypeDescriptor();
        }


    if (ecProperty.GetIsNavigation())
        {
        NavigationECPropertyCP navProp = ecProperty.GetAsNavigationProperty();
        return ECTypeDescriptor::CreateNavigationTypeDescriptor(navProp->GetType(), navProp->IsMultiple());
        }

    BeAssert(false && "Unhandled ECProperty type. Adjust code to new ECProperty type");
    return ECTypeDescriptor();
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
