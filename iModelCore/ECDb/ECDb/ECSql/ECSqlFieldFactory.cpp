/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlFieldFactory.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    auto valueExp = derivedProperty->GetExpression();

    auto selectPreparedState = ctx.GetECSqlStatementR ().GetPreparedStatementP <ECSqlSelectPreparedStatement> ();

    ECSqlColumnInfo ecsqlColumnInfo;
    PropertyNameExp const* propertyNameExp = nullptr;
    if (valueExp->GetType() == Exp::Type::PropertyName)
        propertyNameExp = static_cast<PropertyNameExp const*>(valueExp);          

    const bool isPropertyNameExp = propertyNameExp != nullptr;
    //if a column alias was specified always create a dynamic property for it, even if expression is a property name expression
    if (isPropertyNameExp && derivedProperty->GetColumnAlias ().empty () && !propertyNameExp->IsPropertyRef ())
        ecsqlColumnInfo = CreateECSqlColumnInfoFromPropertyNameExp (ctx, *propertyNameExp);
    else
        {
        ECPropertyCP generatedProperty = nullptr;
        auto stat = selectPreparedState->GetDynamicSelectClauseECClassR ().AddProperty (generatedProperty, *derivedProperty, selectPreparedState->GetECDb ());
        if (stat != ECSqlStatus::Success)
            {
            BeAssert (false && "");
            return ctx.SetError (ECSqlStatus::ProgrammerError, "Could not create dynamic ECProperty for computed select clause item.");
            }

        ecsqlColumnInfo = CreateECSqlColumnInfoFromGeneratedProperty (ctx, *generatedProperty);
        }
    
    auto const& valueTypeInfo = valueExp->GetTypeInfo ();
    BeAssert (valueTypeInfo.GetKind () != ECSqlTypeInfo::Kind::Unset);

    unique_ptr<ECSqlField> field = nullptr;
    ECSqlStatus stat = ECSqlStatus::Success;
    switch (valueTypeInfo.GetKind ())
        {
        case ECSqlTypeInfo::Kind::Primitive:
        case ECSqlTypeInfo::Kind::Null:
            stat = CreatePrimitiveField (field, startColumnIndex, ctx, move (ecsqlColumnInfo), propertyNameExp, valueTypeInfo.GetPrimitiveType ());
            break;

        case ECSqlTypeInfo::Kind::Struct:
            stat =  CreateStructField (field, startColumnIndex, ctx, move (ecsqlColumnInfo), propertyNameExp);
            break;

        case ECSqlTypeInfo::Kind::PrimitiveArray:
            stat =  CreatePrimitiveArrayField(field, startColumnIndex, ctx, move (ecsqlColumnInfo), propertyNameExp, valueTypeInfo.GetPrimitiveType ());
            break;

        case ECSqlTypeInfo::Kind::StructArray:
            {
            if (!isPropertyNameExp)
                {
                BeAssert(false && "Operations with struct array properties not supported in the select clause. This should have been caught by the parser already.");
                return ctx.SetError(ECSqlStatus::InvalidECSql, "Operations with struct array properties not supported in the select clause.");
                }

            stat = CreateStructArrayField(field, startColumnIndex, ctx, move (ecsqlColumnInfo), propertyNameExp->GetPropertyMap ());
            break;
            }

        default:
            BeAssert (false && "Unhandled property type in value reader factory");
            return ECSqlStatus::ProgrammerError;
        }

    if (stat == ECSqlStatus::Success)
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
    PRECONDITION(propertyName != nullptr && "We donot expect computed expression in case of struct", ECSqlStatus::ProgrammerError);

    if (propertyName->GetClassRefExp()->GetType() == Exp::Type::ClassName)
        {
        auto classNameExp = static_cast<ClassNameExp const*>(propertyName->GetClassRefExp());
        PRECONDITION(classNameExp != nullptr, ECSqlStatus::ProgrammerError);
        auto& propertyMap = propertyName->GetPropertyMap();
        if(auto structPropertyMap = dynamic_cast<PropertyMapToInLineStructCP>(&propertyMap))
            return CreateStructMemberFields (field, sqlColumnIndex, ctx, *structPropertyMap, move (ecsqlColumnInfo));
        else
            return ctx.SetError(ECSqlStatus::ProgrammerError, "For struct properties we only support inline mapping %s", Utf8String (propertyMap.GetPropertyAccessString()).c_str ());
        }
    if (propertyName->GetClassRefExp ()->GetType () == Exp::Type::SubqueryRef)
        {        
        auto& propertyMap = propertyName->GetPropertyMap ();
        if (auto structPropertyMap = dynamic_cast<PropertyMapToInLineStructCP>(&propertyMap))
            return CreateStructMemberFields (field, sqlColumnIndex, ctx, *structPropertyMap, move (ecsqlColumnInfo));
        else
            return ctx.SetError (ECSqlStatus::ProgrammerError, "For struct properties we only support inline mapping %s", Utf8String (propertyMap.GetPropertyAccessString ()).c_str ());
        }
    return ctx.SetError(ECSqlStatus::InvalidECSql, "Nested SELECTs are not supported yet.");
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
    auto const& primArraySystemClass = ctx.GetECSqlStatementR().GetPreparedStatementP ()->GetECDb ().GetECDbImplR().GetECDbMap ().GetClassForPrimitiveArrayPersistence(primitiveType);
    field = unique_ptr<ECSqlField> (
        new PrimitiveArrayMappedToSingleColumnECSqlField (ctx.GetECSqlStatementR (), move (ecsqlColumnInfo), sqlColumnIndex++, primArraySystemClass));  
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
    auto arrayProperty = propertyMap.GetProperty().GetAsArrayProperty();
    if (!arrayProperty)
        {
        BeAssert(false && "Expecting array property");
        return ctx.SetError(ECSqlStatus::ProgrammerError, "Expecting array property");
        }

    if (arrayProperty->GetKind() != ARRAYKIND_Struct)
        {
        BeAssert(false && "Expecting struct array property");
        return ctx.SetError(ECSqlStatus::ProgrammerError, "Expecting struct array property");
        }

    auto structType = arrayProperty->GetStructElementType();
    auto structTypeMap = ecdb.GetECDbImplR().GetECDbMap ().GetClassMap (*structType);
    //1. Generate ECSQL statement to read nested struct array.
    ECSqlSelectBuilder innerECSql;
    Utf8String innerECSqlSelectClause;
    bool isFirstProp = true;
    int selectColumnCount = 0;
    for (auto propertyMap : structTypeMap->GetPropertyMaps ())
        {
        if (propertyMap->IsECInstanceIdPropertyMap ())
            continue;

        if (!isFirstProp)
            innerECSqlSelectClause.append(", ");

        innerECSqlSelectClause.append("[");
        innerECSqlSelectClause.append(Utf8String(propertyMap->GetProperty().GetName().c_str()));
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

    auto structArrayField = unique_ptr<StructArrayMappedToSecondaryTableECSqlField>(
        new StructArrayMappedToSecondaryTableECSqlField (ctx, *arrayProperty, move (ecsqlColumnInfo)));

    //2. Create and prepare the nested ECSqlStatement.
   
    auto& secondaryECSqlStatement = structArrayField->GetSecondaryECSqlStatement();
    
    auto status = secondaryECSqlStatement.Prepare (ecdb, innerECSql.ToString ().c_str());
    if (status != ECSqlStatus::Success)
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
PropertyMapToInLineStructCR structPropertyMap,
ECSqlColumnInfo&& structFieldColumnInfo
)
    {
    auto const& childPropertyMaps = structPropertyMap.GetChildren ();
    if (childPropertyMaps.IsEmpty ())
        return ECSqlStatus::Success;


    auto newStructField = unique_ptr<StructMappedToColumnsECSqlField>(new StructMappedToColumnsECSqlField(ctx.GetECSqlStatementR (), move(structFieldColumnInfo)));

    ECSqlStatus status = ECSqlStatus::Success;
    for(auto childPropertyMap : childPropertyMaps)
        {
        if (childPropertyMap->IsUnmapped ())
            continue;

        ECSqlColumnInfo childColumnInfo = ECSqlColumnInfo::CreateChild (newStructField->GetColumnInfo (), childPropertyMap->GetProperty ());

        std::unique_ptr<ECSqlField> childField = nullptr;
        if (auto childStructPropMap = dynamic_cast<PropertyMapToInLineStructCP>(childPropertyMap))
            {
            status = CreateStructMemberFields (childField, sqlColumnIndex, ctx, *childStructPropMap, move (childColumnInfo));
            if ( status != ECSqlStatus::Success)
                return status;
            }
        else
            {           
            if (childPropertyMap->GetProperty().GetIsPrimitive())
                {          
                auto primitiveType = childPropertyMap->GetProperty().GetAsPrimitiveProperty()->GetType();
                status = CreatePrimitiveField(childField, sqlColumnIndex, ctx, move (childColumnInfo), nullptr, primitiveType);
                }
            if (childPropertyMap->GetProperty().GetIsStruct())
                {
                BeAssert(false && "Struct is always mapped inline so control should not drop here");
                }
            else if (childPropertyMap->GetProperty().GetIsArray())
                {          
                auto arrayProperty = childPropertyMap->GetProperty().GetAsArrayProperty();
                if (arrayProperty->GetKind() == ArrayKind::ARRAYKIND_Primitive)
                    {
                    auto primitiveType = arrayProperty->GetPrimitiveElementType();
                    status = CreatePrimitiveArrayField(childField, sqlColumnIndex, ctx, move (childColumnInfo), nullptr, primitiveType);
                    }
                else
                    {
                    status = CreateStructArrayField(childField, sqlColumnIndex, ctx, move(childColumnInfo), *childPropertyMap);
                    }
                }
            }

        if (childField == nullptr)
            {
            BeAssert (false && "No ECSqlField instantiated");
            return ctx.SetError (ECSqlStatus::ProgrammerError, "No ECSqlField instantiated");
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
