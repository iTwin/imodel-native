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
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlStatus ECSqlFieldFactory::CreateField
(
ECSqlPrepareContext& ctx, 
DerivedPropertyExp const* derivedProperty, 
int startColumnIndex
)
    {
    BeAssert (derivedProperty != nullptr && derivedProperty->IsComplete ());

    ECSqlSelectPreparedStatement* selectPreparedState = ctx.GetECSqlStatementR ().GetPreparedStatementP <ECSqlSelectPreparedStatement> ();

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
    
    auto const& valueTypeInfo = valueExp->GetTypeInfo ();
    BeAssert (valueTypeInfo.GetKind () != ECSqlTypeInfo::Kind::Unset);

    unique_ptr<ECSqlField> field = nullptr;
    stat = ECSqlStatus::Success;
    switch (valueTypeInfo.GetKind ())
        {
        case ECSqlTypeInfo::Kind::Primitive:
        case ECSqlTypeInfo::Kind::Null:
            stat = CreatePrimitiveField(field, startColumnIndex, ctx, move(ecsqlColumnInfo), propNameExp, valueTypeInfo.GetPrimitiveType());
            break;

        case ECSqlTypeInfo::Kind::Struct:
            stat = CreateStructField(field, startColumnIndex, ctx, move(ecsqlColumnInfo), propNameExp);
            break;

        case ECSqlTypeInfo::Kind::PrimitiveArray:
            stat = CreatePrimitiveArrayField(field, startColumnIndex, ctx, move(ecsqlColumnInfo), propNameExp, valueTypeInfo.GetPrimitiveType());
            break;

        case ECSqlTypeInfo::Kind::StructArray:
            {
            if (propNameExp == nullptr)
                {
                BeAssert(false && "Operations with struct array properties not supported in the select clause. This should have been caught by the parser already.");
                ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Operations with struct array properties not supported in the select clause.");
                return ECSqlStatus::InvalidECSql;
                }

            stat = CreateStructArrayField(field, startColumnIndex, ctx, move(ecsqlColumnInfo), propNameExp->GetPropertyMap());
            break;
            }

        default:
            BeAssert (false && "Unhandled property type in value reader factory");
            return ECSqlStatus::Error;
        }

    if (stat.IsSuccess())
        selectPreparedState->AddField (move (field));

    return stat;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlFieldFactory::CreatePrimitiveField
(
std::unique_ptr<ECSqlField>& field,
int& sqlColumnIndex,
ECSqlPrepareContext& ctx,
ECSqlColumnInfo&& ecsqlColumnInfo, 
PropertyNameExp const* propertyName, //NOT USED
PrimitiveType primitiveType
)
    {
    auto& ecsqlStmt = ctx.GetECSqlStatementR ();

    switch (primitiveType)
        {
        case PRIMITIVETYPE_Point2D:
            {
            auto xColumnIndex = sqlColumnIndex++;
            auto yColumnIndex = sqlColumnIndex++;

            field = unique_ptr<ECSqlField> (
                new PointMappedToColumnsECSqlField (ecsqlStmt, move (ecsqlColumnInfo), 
                xColumnIndex, yColumnIndex));
            break;
            }
        case PRIMITIVETYPE_Point3D:
            {
            auto xColumnIndex = sqlColumnIndex++;
            auto yColumnIndex = sqlColumnIndex++;
            auto zColumnIndex = sqlColumnIndex++;

            field = unique_ptr<ECSqlField> (
                new PointMappedToColumnsECSqlField (ecsqlStmt, move (ecsqlColumnInfo), 
                xColumnIndex, yColumnIndex, zColumnIndex));
            break;
            }
        default:
            field = unique_ptr<ECSqlField> (
                new PrimitiveMappedToSingleColumnECSqlField (ecsqlStmt, move (ecsqlColumnInfo), 
                sqlColumnIndex++));
            break;
        }

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2013
// Struct can be mapped to blob or inline. We are only handling inline case here
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlFieldFactory::CreateStructField
(
std::unique_ptr<ECSqlField>& field,
int& sqlColumnIndex,
ECSqlPrepareContext& ctx, 
ECSqlColumnInfo&& ecsqlColumnInfo, 
PropertyNameExp const* propertyName
)
    {
    PRECONDITION(propertyName != nullptr && "We donot expect computed expression in case of struct", ECSqlStatus::Error);

    if (propertyName->GetClassRefExp()->GetType() == Exp::Type::ClassName)
        {
        auto classNameExp = static_cast<ClassNameExp const*>(propertyName->GetClassRefExp());
        PRECONDITION(classNameExp != nullptr, ECSqlStatus::Error);
        auto& propertyMap = propertyName->GetPropertyMap();
        if(auto structPropertyMap = dynamic_cast<PropertyMapStructCP>(&propertyMap))
            return CreateStructMemberFields (field, sqlColumnIndex, ctx, *structPropertyMap, move (ecsqlColumnInfo));

        BeAssert (false && "For struct properties we only support inline mapping %s");
        return ECSqlStatus::Error;
        }
    if (propertyName->GetClassRefExp ()->GetType () == Exp::Type::SubqueryRef)
        {        
        auto& propertyMap = propertyName->GetPropertyMap ();
        if (auto structPropertyMap = dynamic_cast<PropertyMapStructCP>(&propertyMap))
            return CreateStructMemberFields (field, sqlColumnIndex, ctx, *structPropertyMap, move (ecsqlColumnInfo));
        
        BeAssert(false && "For struct properties we only support inline mapping %s");
        return ECSqlStatus::Error;
        }

    BeAssert(false);
    return ECSqlStatus::Error;
    }

//-----------------------------------------------------------------------------------------
//! Arrays are not necessarily mapped to blob all the time. But this the case right now.
//! If array is mapped to a different table we need to update this function to take care of that case.
// @bsimethod                                    Affan.Khan                       09/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlFieldFactory::CreatePrimitiveArrayField
(
std::unique_ptr<ECSqlField>& field,
int& sqlColumnIndex,
ECSqlPrepareContext& ctx, 
ECSqlColumnInfo&& ecsqlColumnInfo, 
PropertyNameExp const* propertyName, //NOT USED
PrimitiveType primitiveType
)
    {
    ECClassCP primArraySystemClass = ECDbSystemSchemaHelper::GetClassForPrimitiveArrayPersistence(ctx.GetECDb(), primitiveType);
    if (primArraySystemClass == nullptr)
        {
        BeAssert(false);
        return ECSqlStatus::Error;
        }

    field = unique_ptr<ECSqlField> (
        new PrimitiveArrayMappedToSingleColumnECSqlField (ctx.GetECSqlStatementR (), move (ecsqlColumnInfo), sqlColumnIndex++, *primArraySystemClass));  
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlFieldFactory::CreateStructArrayField
(
std::unique_ptr<ECSqlField>& field,
int& sqlColumnIndex,
ECSqlPrepareContext& ctx, 
ECSqlColumnInfo&& ecsqlColumnInfo, 
PropertyMapCR propertyMap
)
    {
    auto const& ecdb = ctx.GetECSqlStatementR ().GetPreparedStatementP ()->GetECDb ();
    auto structArrayProperty = propertyMap.GetProperty().GetAsStructArrayProperty();
    if (!structArrayProperty)
        {
        BeAssert(false && "Expecting struct array property");
        return ECSqlStatus::Error;
        }

    ECClassCP structType = structArrayProperty->GetStructElementType();
    ClassMap const* structTypeMap = ecdb.GetECDbImplR().GetECDbMap ().GetClassMap (*structType);
    if (structTypeMap == nullptr)
        {
        BeAssert(false);
        return ECSqlStatus::Error;
        }

    //1. Generate ECSQL statement to read nested struct array.
    ECSqlSelectBuilder innerECSql;
    Utf8String innerECSqlSelectClause;
    bool isFirstProp = true;
    int selectColumnCount = 0;
    for (PropertyMap const* propertyMap : structTypeMap->GetPropertyMaps ())
        {
        if (propertyMap->IsECInstanceIdPropertyMap ())
            continue;

        if (!isFirstProp)
            innerECSqlSelectClause.append(", ");

        innerECSqlSelectClause.append("[");
        innerECSqlSelectClause.append(propertyMap->GetProperty().GetName());
        innerECSqlSelectClause.append("]");
        selectColumnCount++;
        isFirstProp = false;
        }

    if (isFirstProp)
        innerECSqlSelectClause.append (ECDB_COL_ECInstanceId);
    else
        innerECSqlSelectClause.append (", " ECDB_COL_ECInstanceId);

    Utf8String whereClause;

    ECPropertyId  persistedPropertyId = propertyMap.GetPropertyPathId ();
    whereClause.Sprintf (ECDB_COL_ParentECInstanceId " = ? AND " ECDB_COL_ECPropertyPathId " = %d", persistedPropertyId);
    innerECSql.Select (innerECSqlSelectClause.c_str ()).From (*structType, false).Where (whereClause.c_str()).OrderBy (ECDB_COL_ECArrayIndex);

    unique_ptr<StructArrayMappedToSecondaryTableECSqlField> structArrayField = unique_ptr<StructArrayMappedToSecondaryTableECSqlField>(
        new StructArrayMappedToSecondaryTableECSqlField (ctx, *structArrayProperty, move (ecsqlColumnInfo)));

    //2. Create and prepare the nested ECSqlStatement.
   
    auto& secondaryECSqlStatement = structArrayField->GetSecondaryECSqlStatement();
    
    auto status = secondaryECSqlStatement.Prepare (ecdb, innerECSql.ToString ().c_str());
    if (!status.IsSuccess())
        return status;

    //Make sure we hide ECInstnaceId from user
    structArrayField->SetHiddenMemberStartIndex (selectColumnCount);

    //3. Set binding information to bind ECInstanceId from parent statement to nested statement.
    //   Everytime parent do Step() the nested statement is rerun with new value of parent ECInstanceId.
    structArrayField->GetBinder().SetSourcePropertyPath (ECDbSystemSchemaHelper::ECINSTANCEID_PROPNAME);

    // The SourcePropertyIndex would be set by Prepare later when it complete parsing parent SELECT-list. It would either map it to 
    // existing selected ECInstanceId property or it will select ECInstanceId property automatically in sqlite statement that it generate
    // and update the SourcePropertyIndex according to point to it.
    structArrayField->GetBinder().SetTargetParamterIndex (1);    

    field = move (structArrayField);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       09/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlFieldFactory::CreateStructMemberFields
(
std::unique_ptr<ECSqlField>& structField, 
int& sqlColumnIndex, 
ECSqlPrepareContext& ctx, 
PropertyMapStructCR structPropertyMap,
ECSqlColumnInfo&& structFieldColumnInfo
)
    {
    auto const& childPropertyMaps = structPropertyMap.GetChildren ();
    if (childPropertyMaps.IsEmpty ())
        return ECSqlStatus::Success;

    auto newStructField = unique_ptr<StructMappedToColumnsECSqlField>(new StructMappedToColumnsECSqlField(ctx.GetECSqlStatementR (), move(structFieldColumnInfo)));

    ECSqlStatus status = ECSqlStatus::Success;
    for(PropertyMapCP childPropertyMap : childPropertyMaps)
        {
        ECSqlColumnInfo childColumnInfo = ECSqlColumnInfo::CreateChild (newStructField->GetColumnInfo (), childPropertyMap->GetProperty ());

        std::unique_ptr<ECSqlField> childField = nullptr;
        if (PropertyMapStructCP childStructPropMap = dynamic_cast<PropertyMapStructCP>(childPropertyMap))
            {
            status = CreateStructMemberFields (childField, sqlColumnIndex, ctx, *childStructPropMap, move (childColumnInfo));
            if ( !status.IsSuccess())
                return status;
            }
        else
            {           
            if (childPropertyMap->GetProperty().GetIsPrimitive())
                {          
                PrimitiveType primitiveType = childPropertyMap->GetProperty().GetAsPrimitiveProperty()->GetType();
                status = CreatePrimitiveField(childField, sqlColumnIndex, ctx, move (childColumnInfo), nullptr, primitiveType);
                }
            if (childPropertyMap->GetProperty().GetIsStruct())
                {
                BeAssert(false && "Struct is always mapped inline so control should not drop here");
                }
            else if (childPropertyMap->GetProperty().GetIsArray())
                {          
                ArrayECPropertyCP arrayProperty = childPropertyMap->GetProperty().GetAsArrayProperty();
                if (arrayProperty->GetKind() == ArrayKind::ARRAYKIND_Primitive)
                    {
                    PrimitiveType primitiveType = arrayProperty->GetPrimitiveElementType();
                    status = CreatePrimitiveArrayField(childField, sqlColumnIndex, ctx, move (childColumnInfo), nullptr, primitiveType);
                    }
                else
                    status = CreateStructArrayField(childField, sqlColumnIndex, ctx, move(childColumnInfo), *childPropertyMap);
                }
            else if (childPropertyMap->GetProperty().GetIsNavigation())
                {
                //WIP_NAVPROP Not implemented yet
                BeAssert(false && "NavProps not implemented yet.");
                return ECSqlStatus::Error;
                }
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
