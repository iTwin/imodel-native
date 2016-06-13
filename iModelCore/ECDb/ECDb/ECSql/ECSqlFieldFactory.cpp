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

using namespace std;
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

    ECSqlColumnInfo ecsqlColumnInfo;
    if (generatedProperty != nullptr)
        ecsqlColumnInfo = CreateECSqlColumnInfoFromGeneratedProperty(ctx, *generatedProperty);
    else
        ecsqlColumnInfo = CreateECSqlColumnInfoFromPropertyNameExp(ctx, *propNameExp);

    ECSqlTypeInfo const& valueTypeInfo = valueExp->GetTypeInfo();
    BeAssert(valueTypeInfo.GetKind() != ECSqlTypeInfo::Kind::Unset);

    unique_ptr<ECSqlField> field = nullptr;
    stat = ECSqlStatus::Success;
    switch (valueTypeInfo.GetKind())
        {
            case ECSqlTypeInfo::Kind::Primitive:
            case ECSqlTypeInfo::Kind::Null:
                stat = CreatePrimitiveField(field, startColumnIndex, ctx, move(ecsqlColumnInfo), valueTypeInfo.GetPrimitiveType());
                break;

            case ECSqlTypeInfo::Kind::Struct:
                stat = CreateStructField(field, startColumnIndex, ctx, move(ecsqlColumnInfo), propNameExp);
                break;

            case ECSqlTypeInfo::Kind::PrimitiveArray:
                stat = CreatePrimitiveArrayField(field, startColumnIndex, ctx, move(ecsqlColumnInfo), valueTypeInfo.GetPrimitiveType());
                break;

            case ECSqlTypeInfo::Kind::StructArray:
                stat = CreateStructArrayField(field, startColumnIndex, ctx, move(ecsqlColumnInfo));
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
ECSqlStatus ECSqlFieldFactory::CreatePrimitiveField(unique_ptr<ECSqlField>& field, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECSqlColumnInfo&& ecsqlColumnInfo, PrimitiveType primitiveType)
    {
    auto& ecsqlStmt = ctx.GetECSqlStatementR ();

    switch (primitiveType)
        {
        case PRIMITIVETYPE_Point2D:
            {
            auto xColumnIndex = sqlColumnIndex++;
            auto yColumnIndex = sqlColumnIndex++;

            field = unique_ptr<ECSqlField> (new PointMappedToColumnsECSqlField (ecsqlStmt, move (ecsqlColumnInfo), xColumnIndex, yColumnIndex));
            break;
            }
        case PRIMITIVETYPE_Point3D:
            {
            auto xColumnIndex = sqlColumnIndex++;
            auto yColumnIndex = sqlColumnIndex++;
            auto zColumnIndex = sqlColumnIndex++;

            field = unique_ptr<ECSqlField> (new PointMappedToColumnsECSqlField (ecsqlStmt, move (ecsqlColumnInfo), xColumnIndex, yColumnIndex, zColumnIndex));
            break;
            }
        default:
            field = unique_ptr<ECSqlField> (new PrimitiveMappedToSingleColumnECSqlField (ecsqlStmt, move (ecsqlColumnInfo), sqlColumnIndex++));
            break;
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreateStructField(unique_ptr<ECSqlField>& field, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECSqlColumnInfo&& ecsqlColumnInfo, PropertyNameExp const* propertyName)
    {
    PRECONDITION(propertyName != nullptr && "We donot expect computed expression in case of struct", ECSqlStatus::Error);

    if (propertyName->GetClassRefExp()->GetType() == Exp::Type::ClassName)
        {
        auto classNameExp = static_cast<ClassNameExp const*>(propertyName->GetClassRefExp());
        PRECONDITION(classNameExp != nullptr, ECSqlStatus::Error);
        auto& propertyMap = propertyName->GetPropertyMap();
        if(StructPropertyMap const* structPropertyMap = dynamic_cast<StructPropertyMap const*>(&propertyMap))
            return CreateStructMemberFields (field, sqlColumnIndex, ctx, *structPropertyMap, move (ecsqlColumnInfo));

        BeAssert (false && "For struct properties we only support inline mapping %s");
        return ECSqlStatus::Error;
        }
    if (propertyName->GetClassRefExp ()->GetType () == Exp::Type::SubqueryRef)
        {        
        auto& propertyMap = propertyName->GetPropertyMap ();
        if (StructPropertyMap const* structPropertyMap = dynamic_cast<StructPropertyMap const*>(&propertyMap))
            return CreateStructMemberFields (field, sqlColumnIndex, ctx, *structPropertyMap, move (ecsqlColumnInfo));
        
        BeAssert(false && "For struct properties we only support inline mapping %s");
        return ECSqlStatus::Error;
        }

    BeAssert(false);
    return ECSqlStatus::Error;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreatePrimitiveArrayField(unique_ptr<ECSqlField>& field, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECSqlColumnInfo&& ecsqlColumnInfo, PrimitiveType primitiveType)
    {
    ECClassCP primArraySystemClass = ECDbSystemSchemaHelper::GetClassForPrimitiveArrayPersistence(ctx.GetECDb(), primitiveType);
    if (primArraySystemClass == nullptr)
        {
        BeAssert(false);
        return ECSqlStatus::Error;
        }

    field = unique_ptr<ECSqlField> (new PrimitiveArrayMappedToSingleColumnECSqlField (ctx.GetECSqlStatementR (), move (ecsqlColumnInfo), sqlColumnIndex++, *primArraySystemClass));  
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreateStructArrayField(unique_ptr<ECSqlField>& field, int& sqlColumnIndex, ECSqlPrepareContext& ctx, ECSqlColumnInfo&& ecsqlColumnInfo)
    {
    field = unique_ptr<ECSqlField>(new StructArrayJsonECSqlField(ctx.GetECSqlStatementR(), move(ecsqlColumnInfo), sqlColumnIndex++));
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreateStructMemberFields(unique_ptr<ECSqlField>& structField, int& sqlColumnIndex, ECSqlPrepareContext& ctx, StructPropertyMap const& structPropertyMap, ECSqlColumnInfo&& structFieldColumnInfo)
    {
    PropertyMapCollection const& childPropertyMaps = structPropertyMap.GetChildren ();
    if (childPropertyMaps.IsEmpty ())
        return ECSqlStatus::Success;

    unique_ptr<StructMappedToColumnsECSqlField> newStructField = unique_ptr<StructMappedToColumnsECSqlField>(new StructMappedToColumnsECSqlField(ctx.GetECSqlStatementR (), move(structFieldColumnInfo)));

    ECSqlStatus status = ECSqlStatus::Success;
    for (PropertyMapCP childPropertyMap : childPropertyMaps)
        {
        ECSqlColumnInfo childColumnInfo = ECSqlColumnInfo::CreateChild(newStructField->GetColumnInfo(), childPropertyMap->GetProperty());

        unique_ptr<ECSqlField> childField = nullptr;
        if (StructPropertyMap const* childStructPropMap = dynamic_cast<StructPropertyMap const*>(childPropertyMap))
            {
            status = CreateStructMemberFields(childField, sqlColumnIndex, ctx, *childStructPropMap, move(childColumnInfo));
            if (!status.IsSuccess())
                return status;
            }
        else if (childPropertyMap->GetProperty().GetIsPrimitive())
            {
            PrimitiveType primitiveType = childPropertyMap->GetProperty().GetAsPrimitiveProperty()->GetType();
            status = CreatePrimitiveField(childField, sqlColumnIndex, ctx, move(childColumnInfo), primitiveType);
            }
        else if (childPropertyMap->GetProperty().GetIsArray())
            {
            ArrayECPropertyCP arrayProperty = childPropertyMap->GetProperty().GetAsArrayProperty();
            if (arrayProperty->GetKind() == ArrayKind::ARRAYKIND_Primitive)
                {
                PrimitiveType primitiveType = arrayProperty->GetPrimitiveElementType();
                status = CreatePrimitiveArrayField(childField, sqlColumnIndex, ctx, move(childColumnInfo), primitiveType);
                }
            else
                status = CreateStructArrayField(childField, sqlColumnIndex, ctx, move(childColumnInfo));
            }
        else if (childPropertyMap->GetProperty().GetIsNavigation())
            {
            NavigationECPropertyCP navProp = childPropertyMap->GetProperty().GetAsNavigationProperty();
            PrimitiveType navPropIdType = navProp->GetType();
            if (!navProp->IsMultiple())
                status = CreatePrimitiveField(childField, sqlColumnIndex, ctx, move(childColumnInfo), navPropIdType);
            else
                status = CreatePrimitiveArrayField(childField, sqlColumnIndex, ctx, move(childColumnInfo), navPropIdType);
            }

        if (childField == nullptr)
            {
            BeAssert (false && "No ECSqlField instantiated");
            return ECSqlStatus::Error;
            }

        newStructField->AppendField(move (childField));
        }

    structField = move (newStructField);
    return status;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlColumnInfo ECSqlFieldFactory::CreateECSqlColumnInfoFromPropertyNameExp (ECSqlPrepareContext const& ctx, PropertyNameExp const& propertyNameExp)
    {
    ECSqlPropertyPath ecsqlPropPath;

    if (ctx.GetParentColumnInfo () != nullptr)
        {
        ECSqlPropertyPathCR parentPropPath = ctx.GetParentColumnInfo ()->GetPropertyPath ();
        ecsqlPropPath.InsertEntriesAtBeginning (parentPropPath);
        }

    PropertyPath const& internalPropPath = propertyNameExp.GetPropertyPath ();
    size_t entryCount = internalPropPath.Size ();
    for (size_t i = 0; i < entryCount; i++)
        {
        PropertyPath::Location const& internalEntry = internalPropPath[i];
        ecsqlPropPath.AddEntry (*internalEntry.GetProperty ());
        }

    BeAssert (ecsqlPropPath.Size () > 0 && "Error in program logic. Property path must not be empty.");

    ECClassCR& rootClass = internalPropPath.GetClassMap ()->GetClass ();
    return ECSqlColumnInfo::CreateTopLevel (false, move (ecsqlPropPath), rootClass, propertyNameExp.GetClassAlias ());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSqlColumnInfo ECSqlFieldFactory::CreateECSqlColumnInfoFromGeneratedProperty (ECSqlPrepareContext const& ctx, ECPropertyCR generatedProperty)
    {
    ECSqlPropertyPath propertyPath;
    if (ctx.GetParentColumnInfo () != nullptr)
        {
        auto const& parentPropPath = ctx.GetParentColumnInfo ()->GetPropertyPath ();
        propertyPath.InsertEntriesAtBeginning (parentPropPath);
        }

    propertyPath.AddEntry (generatedProperty);

    return ECSqlColumnInfo::CreateTopLevel (true, move (propertyPath), generatedProperty.GetClass (), nullptr);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
