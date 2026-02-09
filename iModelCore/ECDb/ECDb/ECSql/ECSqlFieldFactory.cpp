/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "ECSqlFieldFactory.h"

#include "ECDbPch.h"
#include "ECSqlPreparer.h"
#include "ECSqlStatementNoopImpls.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

template <typename TResult>
TResult const* GetParentOfType(Exp const* exp, Exp::Type type) {
    if (exp == nullptr)
        return nullptr;

    auto* parent = exp->FindParent(type);
    if (parent != nullptr)
        return parent->GetAsCP<TResult>();

    return nullptr;
}

ClassNameExp const* TryGetOutmostView(PropertyNameExp const* propNameExp) {
    if (propNameExp == nullptr)
        return nullptr;

    auto* propRef = propNameExp->GetPropertyRef();
    if (propRef == nullptr)
        return nullptr;

    auto& derivedProperty = propNameExp->GetPropertyRef()->GetEndPointDerivedProperty();  // This is the innermost property referenced by the propRef

    // Now we navigate up and find the outmost view class which wraps this derivd property
    ClassNameExp const* outmostViewClass = nullptr;
    SubqueryRefExp const* subqueryRef = nullptr;
    Exp const* currentExp = derivedProperty.GetParent();

    do {
        subqueryRef = GetParentOfType<SubqueryRefExp>(currentExp, Exp::Type::SubqueryRef);
        if (subqueryRef != nullptr) {
            auto* viewClass = subqueryRef->GetViewClass();
            if (viewClass != nullptr) {
                outmostViewClass = viewClass;
            }

            currentExp = subqueryRef->GetParent();
        }
    } while (subqueryRef != nullptr);

    return outmostViewClass;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// static
// Before calling this method please do check a flag using ctx.GetCreateField() getter
ECSqlStatus ECSqlFieldFactory::CreateField(ECSqlPrepareContext& ctx, DerivedPropertyExp const* derivedProperty, int startColumnIndex) {
    BeAssert(derivedProperty != nullptr && derivedProperty->IsComplete());

    bool isDynamic = false;
    ExtractPropertyValueExp const* extractPropExp = derivedProperty->TryGetExtractPropExp();
    if (extractPropExp) {
        // replace anchor for extract function with actual parameter.
        auto& anchorName = extractPropExp->GetSqlAnchor([&](Utf8CP name) {
            return ctx.GetAnchors().CreateAnchor(name);
        });
        ctx.GetAnchors().QueueReplacementForAnchor(anchorName, SqlPrintfString(",%s,%d", ctx.GetThisStmtPtrParamDecl(), startColumnIndex));
        isDynamic = true;
    }

    ValueExp const* valueExp = derivedProperty->GetExpression();
    PropertyNameExp const* propNameExp = nullptr;
    if (valueExp->GetType() == Exp::Type::PropertyName && !isDynamic)
        propNameExp = valueExp->GetAsCP<PropertyNameExp>();

    ClassNameExp const* viewClassNameExp = TryGetOutmostView(propNameExp);  // If we are in a view we have to resolve the propNameExp to the actual property in the view
    if (viewClassNameExp != nullptr && propNameExp != nullptr) {
        return CreateFieldForView(ctx, *propNameExp, *viewClassNameExp, *derivedProperty, startColumnIndex, isDynamic);
    }

    ECPropertyCP generatedProperty = nullptr;
    ECSqlSelectPreparedStatement& selectPreparedStatement = GetPreparedStatement(ctx);
    ECSqlStatus stat = selectPreparedStatement.GetDynamicSelectClauseECClassR().GeneratePropertyIfRequired(generatedProperty, ctx, *derivedProperty, propNameExp, isDynamic);
    if (!stat.IsSuccess())
        return stat;

    if (generatedProperty == nullptr && propNameExp == nullptr)
        return ECSqlStatus::Error;

    ECSqlColumnInfo ecsqlColumnInfo = CreateColumnInfoForProperty(ctx, generatedProperty, propNameExp, isDynamic);

    return CreateField(ctx, selectPreparedStatement, startColumnIndex, ecsqlColumnInfo, *valueExp);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
// static
ECSqlStatus ECSqlFieldFactory::CreateField(ECSqlPrepareContext& ctx, ECSqlSelectPreparedStatement& preparedStatement, int sqlColumnIndex, ECSqlColumnInfo const& ecsqlColumnInfo, ValueExp const& valueExp) {
    ECSqlTypeInfo const& valueTypeInfo = valueExp.GetTypeInfo();
    BeAssert(valueTypeInfo.GetKind() != ECSqlTypeInfo::Kind::Unset);

    std::unique_ptr<ECSqlField> field = nullptr;
    ECSqlStatus stat = ECSqlStatus::Success;
    switch (valueTypeInfo.GetKind()) {
        case ECSqlTypeInfo::Kind::Null:
            stat = CreateNullField(field, sqlColumnIndex, ctx, ecsqlColumnInfo);
            break;

        case ECSqlTypeInfo::Kind::Primitive:
            stat = CreatePrimitiveField(field, sqlColumnIndex, ctx, ecsqlColumnInfo, valueTypeInfo.GetPrimitiveType());
            break;

        case ECSqlTypeInfo::Kind::Struct:
            stat = CreateStructField(field, sqlColumnIndex, ctx, ecsqlColumnInfo, valueTypeInfo.GetStructType());
            break;

        case ECSqlTypeInfo::Kind::PrimitiveArray:
        case ECSqlTypeInfo::Kind::StructArray:
            stat = CreateArrayField(field, sqlColumnIndex, ctx, ecsqlColumnInfo);
            break;

        case ECSqlTypeInfo::Kind::Navigation:
            stat = CreateNavigationPropertyField(field, sqlColumnIndex, ctx, ecsqlColumnInfo);
            break;

        default:
            BeAssert(false && "Unhandled property type in value reader factory");
            return ECSqlStatus::Error;
    }

    if (stat.IsSuccess())
        preparedStatement.AddField(std::move(field));

    return stat;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
// static
ECSqlStatus ECSqlFieldFactory::CreateFieldForView(ECSqlPrepareContext& ctx, PropertyNameExp const& propNameExp, ClassNameExp const& viewClassNameExp, DerivedPropertyExp const& derivedProperty, int startColumnIndex, bool isDynamic) {
    PropertyPath const& propertyPath = propNameExp.GetResolvedPropertyPath();
    ECPropertyCP ecProperty = propertyPath.First().GetProperty();
    Utf8StringCR propertyName = propertyPath.First().GetName();
    Utf8String columnName = derivedProperty.GetName();
    Utf8StringCR columnAlias = derivedProperty.GetColumnAlias();

    ClassMap const* classMap = propertyPath.GetClassMap();
    ECClassCP rootECClass = ctx.GetECDb().Schemas().GetClass(viewClassNameExp.GetSchemaName(), viewClassNameExp.GetClassName());
    if (classMap != nullptr && rootECClass == nullptr) {
        rootECClass = &classMap->GetClass();
    }

    if (ecProperty == nullptr || ecProperty->GetClass().GetId() != rootECClass->GetId() || !ecProperty->GetName().EqualsI(propertyName)) {
        if (propertyName.EqualsI(ECDBSYS_PROP_ECClassId)) {
            ecProperty = ctx.GetECDb().Schemas().Main().GetSystemSchemaHelper().GetSystemProperty(ECSqlSystemPropertyInfo::ECClassId());
        } else if (propertyName.EqualsI(ECDBSYS_PROP_ECInstanceId)) {
            ecProperty = ctx.GetECDb().Schemas().Main().GetSystemSchemaHelper().GetSystemProperty(ECSqlSystemPropertyInfo::ECInstanceId());
        } else if (rootECClass != nullptr) {
            ecProperty = rootECClass->GetPropertyP(propertyName);
        }
    }

    ECPropertyCP originalProperty = ecProperty;
    ECSqlSelectPreparedStatement& selectPreparedStatement = GetPreparedStatement(ctx);
    bool isDuplicate = false;
    if (ecProperty != nullptr) {
        auto dupStat = selectPreparedStatement.GetDynamicSelectClauseECClassR().CheckForDuplicateName(columnName, columnAlias, isDuplicate, ctx);
        if (dupStat != ECSqlStatus::Success)
            return dupStat;
    }

    bool isGeneratedProperty = false;
    if (derivedProperty.HasAlias() || propertyPath.Size() > 1 || isDuplicate || ecProperty == nullptr) {
        ECPropertyCP generatedProperty;
        ECSqlStatus stat = selectPreparedStatement.GetDynamicSelectClauseECClassR().GeneratePropertyIfRequired(generatedProperty, ctx, derivedProperty, &propNameExp, isDynamic);
        if (!stat.IsSuccess())
            return stat;

        if (generatedProperty != nullptr) {
            isGeneratedProperty = true;
            ecProperty = generatedProperty;
        }
    } else {
        selectPreparedStatement.GetDynamicSelectClauseECClassR().RegisterSelectClauseItem(columnName, derivedProperty);
    }

    if (ecProperty == nullptr) {
        return ECSqlStatus::Error;
    }

    ECSqlColumnInfo::RootClass rootClass(rootECClass != nullptr ? *rootECClass : ecProperty->GetClass(), nullptr);
    const bool isSystemProperty = ecProperty ? (ExtendedTypeHelper::FromProperty(*ecProperty) != ExtendedTypeHelper::ExtendedType::Unknown) : false;
    ECSqlPropertyPath resultPropertyPath;
    resultPropertyPath.AddEntry(*ecProperty);

    DateTime::Info dateTimeInfo;
    ECStructClassCP structType = nullptr;
    ECTypeDescriptor typeDescriptor = DetermineDataType(dateTimeInfo, structType, ctx.Issues(), *ecProperty);
    ECSqlColumnInfo ecsqlColumnInfo(typeDescriptor, dateTimeInfo, structType, ecProperty, originalProperty, isSystemProperty, isGeneratedProperty, resultPropertyPath, rootClass, isDynamic);
    ValueExp const* valueExp = derivedProperty.GetExpression();
    return CreateField(ctx, selectPreparedStatement, startColumnIndex, ecsqlColumnInfo, *valueExp);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
// static
ECSqlColumnInfo ECSqlFieldFactory::CreateColumnInfoForProperty(ECSqlPrepareContext const& ctx, ECPropertyCP generatedProperty, PropertyNameExp const* propertyNameExp, bool isDynamic) {
    bool isGenerated = generatedProperty != nullptr;
    bool isSystem = false;
    ECSqlPropertyPath propertyPath;
    ECClassCP rootClass = nullptr;
    PropertyNameExp const* resolvedPropertyName = propertyNameExp;
    bool propertyNameExpRefersToProperty = propertyNameExp != nullptr;
    // in this bool we perform additional checks below and may set it to false if the target is not a property
    if (propertyNameExpRefersToProperty && propertyNameExp->IsPropertyRef()) {
        auto* propRef = propertyNameExp->GetPropertyRef();
        propertyNameExpRefersToProperty = propRef->IsPure() &&
                                          propRef->GetEndPointDerivedProperty().GetExpression()->GetType() == Exp::Type::PropertyName;

        if (propertyNameExpRefersToProperty) {
            const auto virtualProp = propRef->TryGetVirtualProperty();
            if (virtualProp || !propRef->TryGetPropertyMap())
                resolvedPropertyName = propRef->GetEndPointDerivedProperty().GetExpression()->GetAsCP<PropertyNameExp>();
        }
    }

    if (isGenerated) {
        isSystem = ExtendedTypeHelper::FromProperty(*generatedProperty) != ExtendedTypeHelper::ExtendedType::Unknown;
        propertyPath.AddEntry(*generatedProperty);
        rootClass = &generatedProperty->GetClass();

        if (!propertyNameExpRefersToProperty) {
            return CreateTopLevelColumnInfo(ctx.Issues(), isSystem, true, std::move(propertyPath), ECSqlColumnInfo::RootClass(*rootClass, nullptr), nullptr, isDynamic);
        }
    }
    if (resolvedPropertyName == nullptr) {
        throw std::runtime_error("Error in program logic. Resolved property name must not be null.");
    }
    PropertyPath const& internalPropPath = resolvedPropertyName->GetResolvedPropertyPath();
    size_t entryCount = internalPropPath.Size();
    ECSqlPropertyPath ecsqlPropPath;
    for (size_t i = 0; i < entryCount; i++) {
        PropertyPath::Location const& internalEntry = internalPropPath[i];
        ecsqlPropPath.AddEntry(*internalEntry.GetProperty());
    }

    BeAssert(ecsqlPropPath.Size() > 0 && "Error in program logic. Property path must not be empty.");
    if (propertyNameExp == nullptr) {
        throw std::runtime_error("Error in program logic. Property path must not be empty.");
    }
    const auto isVirtualProperty = propertyNameExp->IsVirtualProperty();
    if (isVirtualProperty) {
        if (isGenerated) {
            return CreateTopLevelColumnInfo(ctx.Issues(), isSystem, true, std::move(propertyPath), ECSqlColumnInfo::RootClass(*rootClass, nullptr), ecsqlPropPath.GetLeafEntry().GetProperty(), isDynamic);
        }

        ECPropertyCP origProp = ecsqlPropPath.GetLeafEntry().GetProperty();
        ECPropertyCP leafProp = internalPropPath.Last().GetProperty();
        rootClass = &leafProp->GetClass();
        return CreateTopLevelColumnInfo(ctx.Issues(), false, false, std::move(ecsqlPropPath), ECSqlColumnInfo::RootClass(*rootClass, "", rootClass->GetName().c_str()), origProp, isDynamic);
    }

    ECPropertyCP leafProp = ecsqlPropPath.GetLeafEntry().GetProperty();
    if (isGenerated) {
        return CreateTopLevelColumnInfo(ctx.Issues(), isSystem, true, std::move(propertyPath), ECSqlColumnInfo::RootClass(*rootClass, nullptr), ecsqlPropPath.GetLeafEntry().GetProperty(), isDynamic);
    }

    BeAssert((internalPropPath.GetClassMap() != nullptr) && "Error in program logic. PropertyPath must have been resolved.");
    ECClassCR ecClass = internalPropPath.GetClassMap()->GetClass();
    isSystem = leafProp != nullptr && ExtendedTypeHelper::FromProperty(*leafProp) != ExtendedTypeHelper::ExtendedType::Unknown;
    Utf8CP tableSpace = resolvedPropertyName->GetPropertyMap()->GetClassMap().GetSchemaManager().GetTableSpace().GetName().c_str();
    return CreateTopLevelColumnInfo(ctx.Issues(), isSystem, false, std::move(ecsqlPropPath), ECSqlColumnInfo::RootClass(ecClass, tableSpace, resolvedPropertyName->GetClassName()), leafProp, isDynamic);
}

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECSqlColumnInfo ECSqlFieldFactory::CreateTopLevelColumnInfo(IssueDataSource const& issues, bool isSystemProperty, bool isGeneratedProperty, ECSqlPropertyPath const& propertyPath, ECSqlColumnInfo::RootClass const& rootClass, ECPropertyCP originalProperty, bool isDynamic) {
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
// static
ECSqlStatus ECSqlFieldFactory::CreateChildField(std::unique_ptr<ECSqlField>& childField, ECSqlPrepareContext& ctx, int& sqlColumnIndex, ECSqlColumnInfo const& parentFieldColumnInfo, ECN::ECPropertyCR childProperty) {
    ECSqlColumnInfo columnInfo = CreateChildColumnInfo(ctx.Issues(), parentFieldColumnInfo, childProperty, ctx.GetECDb().Schemas().Main().GetSystemSchemaHelper().GetSystemPropertyInfo(childProperty).IsSystemProperty());

    if (childProperty.GetIsStruct()) {
        ECStructClassCR childStructType = childProperty.GetAsStructProperty()->GetType();
        return CreateStructMemberFields(childField, sqlColumnIndex, ctx, childStructType, columnInfo);
    }

    if (childProperty.GetIsPrimitive()) {
        PrimitiveType primitiveType = childProperty.GetAsPrimitiveProperty()->GetType();
        return CreatePrimitiveField(childField, sqlColumnIndex, ctx, columnInfo, primitiveType);
    }

    if (childProperty.GetIsArray())
        return CreateArrayField(childField, sqlColumnIndex, ctx, columnInfo);

    if (childProperty.GetIsNavigation()) {
        BeAssert(false && "Navigation properties cannot be used in an ECStruct");
        return ECSqlStatus::Error;
    }

    BeAssert(false && "No ECSqlField instantiated");
    return ECSqlStatus::Error;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECSqlStatus ECSqlFieldFactory::CreateNullField(std::unique_ptr<ECSqlField>& field, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECSqlColumnInfo const& ecsqlColumnInfo) {
    ECSqlSelectPreparedStatement& selectPreparedStatement = GetPreparedStatement(ctx);
    field = std::make_unique<PrimitiveECSqlField>(selectPreparedStatement, ecsqlColumnInfo, sqlColumnIndex++);
    return ECSqlStatus::Success;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECSqlStatus ECSqlFieldFactory::CreatePrimitiveField(std::unique_ptr<ECSqlField>& field, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECSqlColumnInfo const& ecsqlColumnInfo, PrimitiveType primitiveType) {
    ECSqlSelectPreparedStatement& selectPreparedStatement = GetPreparedStatement(ctx);

    switch (primitiveType) {
        case PRIMITIVETYPE_Point2d: {
            int xColumnIndex = sqlColumnIndex++;
            int yColumnIndex = sqlColumnIndex++;

            field = std::make_unique<PointECSqlField>(selectPreparedStatement, ecsqlColumnInfo, xColumnIndex, yColumnIndex);
            break;
        }
        case PRIMITIVETYPE_Point3d: {
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
// static
ECSqlStatus ECSqlFieldFactory::CreateStructField(std::unique_ptr<ECSqlField>& field, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECSqlColumnInfo const& ecsqlColumnInfo, ECN::ECStructClassCR structType) {
    return CreateStructMemberFields(field, sqlColumnIndex, ctx, structType, ecsqlColumnInfo);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECSqlStatus ECSqlFieldFactory::CreateNavigationPropertyField(std::unique_ptr<ECSqlField>& field, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECSqlColumnInfo const& ecsqlColumnInfo) {
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
// static
ECSqlStatus ECSqlFieldFactory::CreateArrayField(std::unique_ptr<ECSqlField>& field, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECSqlColumnInfo const& ecsqlColumnInfo) {
    field = std::make_unique<ArrayECSqlField>(GetPreparedStatement(ctx), ecsqlColumnInfo, sqlColumnIndex++);
    return ECSqlStatus::Success;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECSqlStatus ECSqlFieldFactory::CreateStructMemberFields(std::unique_ptr<ECSqlField>& structField, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECN::ECStructClassCR structType, ECSqlColumnInfo const& structFieldColumnInfo) {
    std::unique_ptr<StructECSqlField> newStructField = std::make_unique<StructECSqlField>(GetPreparedStatement(ctx), structFieldColumnInfo);

    for (ECPropertyCP prop : structType.GetProperties(true)) {
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
// static
ECSqlSelectPreparedStatement& ECSqlFieldFactory::GetPreparedStatement(ECSqlPrepareContext& ctx) {
    return ctx.GetPreparedStatement<ECSqlSelectPreparedStatement>();
}

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECSqlColumnInfo ECSqlFieldFactory::CreateChildColumnInfo(IssueDataSource const& issues, ECSqlColumnInfo const& parent, ECPropertyCR childProperty, bool isSystemProperty) {
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
// static
ECSqlColumnInfo ECSqlFieldFactory::CreateColumnInfoForArrayElement(ECSqlColumnInfo const& parent, int arrayIndex) {
    ECTypeDescriptor arrayElementDataType;
    DateTime::Info dateTimeInfo;
    ECStructClassCP structType = nullptr;
    if (parent.GetDataType().IsPrimitiveArray()) {
        arrayElementDataType = ECTypeDescriptor::CreatePrimitiveTypeDescriptor(parent.GetDataType().GetPrimitiveType());
        if (arrayElementDataType == PRIMITIVETYPE_DateTime)
            dateTimeInfo = parent.GetDateTimeInfo();
    } else {
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
// static
ECTypeDescriptor ECSqlFieldFactory::DetermineDataType(DateTime::Info& dateTimeInfo, ECN::ECStructClassCP& structType, IssueDataSource const& issues, ECPropertyCR ecProperty) {
    if (ecProperty.GetIsPrimitive()) {
        const PrimitiveType primType = ecProperty.GetAsPrimitiveProperty()->GetType();
        if (primType == PRIMITIVETYPE_DateTime) {
            if (ECObjectsStatus::Success != CoreCustomAttributeHelper::GetDateTimeInfo(dateTimeInfo, ecProperty)) {
                issues.ReportV(
                    IssueSeverity::Error,
                    IssueCategory::BusinessProperties,
                    IssueType::ECSQL,
                    ECDbIssueId::ECDb_0469,
                    "Could not read DateTimeInfo custom attribute from the primitive ECProperty %s:%s.",
                    ecProperty.GetClass().GetFullName(),
                    ecProperty.GetName().c_str());
            }
        }

        return ECTypeDescriptor::CreatePrimitiveTypeDescriptor(primType);
    }

    if (ecProperty.GetIsStruct()) {
        structType = &ecProperty.GetAsStructProperty()->GetType();
        return ECTypeDescriptor::CreateStructTypeDescriptor();
    }

    if (ecProperty.GetIsPrimitiveArray()) {
        const PrimitiveType primType = ecProperty.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();
        if (primType == PRIMITIVETYPE_DateTime) {
            if (ECObjectsStatus::Success != CoreCustomAttributeHelper::GetDateTimeInfo(dateTimeInfo, ecProperty)) {
                issues.ReportV(
                    IssueSeverity::Error,
                    IssueCategory::BusinessProperties,
                    IssueType::ECSQL,
                    ECDbIssueId::ECDb_0470,
                    "Could not read DateTimeInfo custom attribute from the primitive array ECProperty %s:%s.",
                    ecProperty.GetClass().GetFullName(),
                    ecProperty.GetName().c_str());
            }
        }

        return ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor(primType);
    }

    if (ecProperty.GetIsStructArray()) {
        structType = &ecProperty.GetAsStructArrayProperty()->GetStructElementType();
        return ECTypeDescriptor::CreateStructArrayTypeDescriptor();
    }

    if (ecProperty.GetIsNavigation()) {
        NavigationECPropertyCP navProp = ecProperty.GetAsNavigationProperty();
        return ECTypeDescriptor::CreateNavigationTypeDescriptor(navProp->GetType(), navProp->IsMultiple());
    }

    BeAssert(false && "Unhandled ECProperty type. Adjust code to new ECProperty type");
    return ECTypeDescriptor();
}

END_BENTLEY_SQLITE_EC_NAMESPACE