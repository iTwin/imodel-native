/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/InstanceInserter.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************* Binding ****************************************************
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Binding::Binding (ECEnablerCR enabler, PropertyMapCR propertyMap, uint32_t propertyIndex, uint16_t componentMask, int sqlIndex, DbColumnCP column)
: m_enabler (enabler), m_propertyMap (propertyMap), m_propertyIndex (propertyIndex), m_componentMask (componentMask),
m_sqlIndex (sqlIndex), m_column (column)
    {
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Binding::Binding (Binding const & other) :
m_enabler (other.m_enabler),
m_propertyMap (other.m_propertyMap),
m_propertyIndex (other.m_propertyIndex),
m_componentMask (other.m_componentMask),
m_sqlIndex (other.m_sqlIndex),
m_column (other.m_column)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
BindingCR Binding::operator= (BindingCR other)
    {
    memcpy (this, &other, sizeof(Binding));
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
DbResult Binding::Bind (BeSQLiteStatementR statement, ECN::IECInstanceR ecInstance, BindingsCR parameterBindings)
    {
    for (int iBinding = 0; iBinding < (int) parameterBindings.size ();)
        {
        BindingCR binding = parameterBindings[iBinding];
        // binding.m_propertyMap->Bind will increment iBinding by 1 or more
        DbResult r = binding.m_propertyMap.Bind (iBinding, parameterBindings, statement, ecInstance);
        if (BE_SQLITE_OK != r)
            return r;
        }

    return BE_SQLITE_OK;
    }

//************* PrimaryKeyGenerator ****************************************************
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PrimaryKeyGenerator::PrimaryKeyGenerator (int& nextParameterIndex, DbColumnR primaryKeyColumn, BeRepositoryBasedIdSequenceR ecInstanceIdSequence) 
: /*m_primaryKeyColumn (primaryKeyColumn),*/ m_parameterIndex (nextParameterIndex), m_ecInstanceIdSequence (ecInstanceIdSequence)
    {
    //nextParameterIndex++;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PrimaryKeyGeneratorPtr PrimaryKeyGenerator::Create (int& nextParameterIndex, DbColumnR primaryKeyColumn, BeRepositoryBasedIdSequenceR ecInstanceIdSequence)
    {
    return new PrimaryKeyGenerator (nextParameterIndex, primaryKeyColumn, ecInstanceIdSequence);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult PrimaryKeyGenerator::_Generate (ECInstanceId* instanceId, BeSQLite::Statement& statement, ECN::IECInstanceR instance, bool useSuppliedECInstanceId) const 
    {
    ECInstanceId newInstanceId;
    if (useSuppliedECInstanceId) 
        {
        if (!EXPECTED_CONDITION (instanceId != nullptr))
            return BE_SQLITE_ERROR; 
        newInstanceId = *instanceId;
        }
    else
        {
        DbResult stat = m_ecInstanceIdSequence.GetNextValue<ECInstanceId> (newInstanceId);
        POSTCONDITION (stat == BE_SQLITE_OK, stat);
        }

    ECInstanceAdapterHelper::SetECInstanceId (instance, newInstanceId);
    if (instanceId != nullptr)
        *instanceId = newInstanceId;

    return statement.BindId (m_parameterIndex, newInstanceId);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult PrimaryKeyGenerator::Generate (ECInstanceId* ecInstanceId, BeSQLite::Statement& statement, ECN::IECInstanceR instance, bool useSuppliedECInstanceId) const
    {
    return _Generate (ecInstanceId, statement, instance, useSuppliedECInstanceId);
    }


//************* InstanceInserter ****************************************************

#define NULL_ARRAY_INDEX 0xffffffff  //Use to tell if an instance is not a array element

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceInserterPtr InstanceInserter::Create (InsertStatus& status, ECDbMapCR ecDbMap, ECN::ECClassCR ecClass, BeRepositoryBasedIdSequenceR ecInstanceIdSequence, InstanceInserterCP parentInserter)
    {
    auto classMap = ecDbMap.GetClassMap (ecClass);
    BeAssert (classMap != nullptr && !classMap->IsUnmapped());

    ECRelationshipClassCP relationshipClass = ecClass.GetRelationshipClassCP();
    InstanceInserterPtr inserter;
    if (!relationshipClass)
        inserter = new InstanceInserter (ecDbMap, ecClass, ecInstanceIdSequence, parentInserter);
    else
        {
        if (classMap->GetClassMapType () == ClassMap::Type::RelationshipLinkTable)
            inserter = new RelationshipInstanceLinkTableInserter (ecDbMap, *relationshipClass, ecInstanceIdSequence);
        else
            inserter = new RelationshipInstanceEndTableInserter (ecDbMap, *relationshipClass, ecInstanceIdSequence);
        }

    status = inserter->_Initialize();

    return (status == INSERT_Success) ? inserter : nullptr;
    }
    
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceInserter::InstanceInserter (ECDbMapCR ecDbMap, ECN::ECClassCR ecClass, BeRepositoryBasedIdSequenceR ecInstanceIdSequence, InstanceInserterCP parentInserter)
: m_primaryKeyGenerator (nullptr), m_ecDbMap (ecDbMap), m_ecClass (ecClass), m_ecInstanceIdSequence (ecInstanceIdSequence), m_isPropertyToTableInsertersSetup (false), m_parentInserter (parentInserter)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceInserter::AppendSqlToInsertIntoTable (DbTableCR table) 
    {
    m_insertBuilder.SetTable(table.GetName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    affan.khan        03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
InsertStatus  InstanceInserter::AppendSqlToInsertPrimaryKeyOfInstance (int& nextParameterIndex, DbTableCR table, bool setupPrimaryKeyGenerator)
    {
    bvector<DbColumnP> primaryKeyColumns;
    table.GetPrimaryKeyTableConstraint().GetColumns(primaryKeyColumns); //WIP_ECDB: What if one of the properties was part of the PrimaryKey

    if (m_ecClass.GetIsStruct()) 
        PRECONDITION(primaryKeyColumns.size() > 1, INSERT_MapError)
    else
        PRECONDITION(primaryKeyColumns.size() == 1, INSERT_MapError);

    if (setupPrimaryKeyGenerator)
        {
        DbColumnP primaryKeyColumn = primaryKeyColumns.front();
        m_primaryKeyGenerator = PrimaryKeyGenerator::Create (nextParameterIndex, *primaryKeyColumn, m_ecInstanceIdSequence);
        }

    FOR_EACH (DbColumnP primaryKeyColumn, primaryKeyColumns)
        {
        // WIP_ECDB: if it is ECPropertyId column, we can hardcode it here! Grep for HARDCODE_ECPROPERTYID
        m_insertBuilder.AddInsertColumn(primaryKeyColumn->GetName(), "?");
        nextParameterIndex++;
        }

    return INSERT_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceInserter::AppendSqlToInsertECClassId (ECClassId classId, DbTableCR table)
    {
    auto classIdColumn = table.GetClassIdColumn(); //TODO: needswork: deal with this??
    if (classIdColumn != nullptr)
        m_insertBuilder.AddInsertColumn(classIdColumn->GetName(), SqlPrintfString("%lld", classId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    affan.khan        03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
InsertStatus InstanceInserter::AppendSqlToInsertPropertyValues (int& nextParameterIndex, IClassMap const& classMap)
    {
    auto classMap2 = dynamic_cast<ClassMapCP> (&classMap);
    BeAssert (classMap2 != nullptr);
    classMap2->GenerateParameterBindings (m_parameterBindings, nextParameterIndex); // WIP_ECD

    FOR_EACH (BindingCR binding, m_parameterBindings)
        {
        if (binding.m_column)
            m_insertBuilder.AddInsertColumn(binding.m_column->GetName(), "?");
        }

    return INSERT_Success;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan        03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
InsertStatus InstanceInserter::TrySetupPropertyToTableInserters (IClassMap const& classMap)
    {
    InsertStatus status = INSERT_Success;
    if (m_isPropertyToTableInsertersSetup)
        return status;

    classMap.GetPropertyMaps ().Traverse ([&status, this] (TraversalFeedback& feedback, PropertyMapCP propMap)
        {
        BeAssert (propMap != nullptr);
        PropertyMapToTableCP propertyMapToTable = propMap->GetAsPropertyMapToTable ();
        if (propertyMapToTable != nullptr)
            {
            InstancePropertyToTableInserterPtr inserterPtr = InstancePropertyToTableInserter::Create (status, m_ecDbMap, *propertyMapToTable, GetECInstanceIdSequence (), this);
            if (status != INSERT_Success)
                {
                feedback = TraversalFeedback::Cancel;
                return;
                }

            BeAssert (inserterPtr != nullptr);
            m_propertyToTableInserters.push_back (inserterPtr);
            }
        }, true);

    m_isPropertyToTableInsertersSetup = true;
    return VerifyExpectedNumberOfBindings (classMap);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
InsertStatus InstanceInserter::_Initialize()
    {
    auto classMap = m_ecDbMap.GetClassMap (m_ecClass);
    PRECONDITION (!classMap->IsUnmapped(), INSERT_MapError);
    DbTableR table = classMap->GetTable();
    PRECONDITION (m_ecDbMap.GetECDbR().TableExists (table.GetName()), INSERT_MapError);

    int nextParameterIndex = 1; // Parameter indices are 1 based rather than 0 based
    AppendSqlToInsertIntoTable (table);

    InsertStatus status = AppendSqlToInsertPrimaryKeyOfInstance (nextParameterIndex, table, true);
    if (status != INSERT_Success)
        return status;

    AppendSqlToInsertECClassId (classMap->GetClass ().GetId(), table);

    status = AppendSqlToInsertPropertyValues (nextParameterIndex, *classMap);
    if (status != INSERT_Success)
        return status;

    if (!m_insertBuilder.GetSql(m_sqlString))
        return INSERT_FailedToGenerateSql;
       
    return status;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan        03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult InstanceInserter::BindPrimaryKeysOfInstance (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance, bool useSuppliedECInstanceId)
    {
    DbResult r = m_primaryKeyGenerator->Generate (ecInstanceId, m_statement, ecInstance, useSuppliedECInstanceId);
    POSTCONDITION (r == BE_SQLITE_OK, r);

    if (m_ecClass.GetIsStruct()) // ClassMappingRule: yet another place assuming that if it is a struct, it is mapped to an array of structs
        {
        m_statement.BindNull (2); // WIP_ECDB: this is ECPropertyId column, we should have hardcoded it! Grep for HARDCODE_ECPROPERTYID
        m_statement.BindNull (3);
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult InstanceInserter::BindPropertyValues (ECN::IECInstanceR ecInstance)
    {
#if 0
    FOR_EACH (BinderPtr propertyBinder, m_propertyBinders)
        {
        DbResult r = propertyBinder->Bind (m_statement, ecInstance);
        if (BE_SQLITE_OK != r)
            return r;
        }
#endif

    return Binding::Bind(m_statement, ecInstance, m_parameterBindings);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult InstanceInserter::_Bind (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance, bool useSuppliedECInstanceId)
    {
    m_statement.ClearBindings();
    DbResult r = BindPrimaryKeysOfInstance (ecInstanceId, ecInstance, useSuppliedECInstanceId);
    POSTCONDITION (BE_SQLITE_OK == r, r);

    return BindPropertyValues (ecInstance);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InsertStatus InstanceInserter::Insert (ECN::IECInstanceR ecInstance, ECInstanceId ecInstanceId)
    {
    InsertStatus status = _Insert (&ecInstanceId, ecInstance, true);
    return status;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InsertStatus InstanceInserter::Insert (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance)
    {
    return _Insert (ecInstanceId, ecInstance, false);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InsertStatus InstanceInserter::_Insert (ECInstanceId* ecInstanceId, IECInstanceR ecInstance, bool useSuppliedECInstanceId)
    {
    if (!m_statement.IsPrepared())
        {
        DbResult r = m_statement.Prepare (m_ecDbMap.GetECDbR(), m_sqlString.c_str());
        POSTCONDITION (BE_SQLITE_OK == r, INSERT_SqliteError);
        }
    else
        m_statement.Reset(); 

    DbResult r = _Bind (ecInstanceId, ecInstance, useSuppliedECInstanceId);
    POSTCONDITION (BE_SQLITE_OK == r, INSERT_SqliteError);
    r =  m_statement.Step();    
    if (BE_SQLITE_DONE != r)
        return IsConstraintDbResult(r) ? INSERT_ConstraintViolation : INSERT_SqliteError;

    DoInsertionsInSecondaryTables (ecInstanceId, ecInstance);

    return INSERT_Success;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan        03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
InsertStatus InstanceInserter::DoInsertionsInSecondaryTables (ECInstanceId* ecInstanceLuid, IECInstanceR ecInstance)
    {
    InsertStatus status;
    ECObjectsStatus ecStatus;    
    //======================================================================================================
    //We have delayed preparing properties that are mapped to secondary tables until insert actually happens.
    //This is done for cyclic struct classes that refer directly or indirectly to itself. If we do that in
    //_Inititalize2() it would have infinit recursion. - affan
    if (!m_isPropertyToTableInsertersSetup)
        {
        auto classMap = m_ecDbMap.GetClassMap (m_ecClass);
        status = TrySetupPropertyToTableInserters (*classMap);
        if (status != InsertStatus::INSERT_Success)
            {
            LOG.error ("Failed to prepare properties that are map to table for insert");
            BeAssert (false && "Failed to prepare properties that are map to table for insert");
            return status;
            }
        }
    //======================================================================================================
    for (InstancePropertyToTableInserterPtr inserter : m_propertyToTableInserters)
        { 
        ECPropertyCR ecProperty = inserter->GetECProperty();
        WStringCR ecPropertyAccessString = inserter->GetECPropertyAccessString();
        ECValue value;
        uint32_t propIndex;
        //WIP_ECDB: make these bindings... where the call to "bind" does the secondary insertion... still must wait until the main insertion has happened... do these in a second pass. That will also help in case where caller has skipped the insert/update of that property!
        ecStatus = ecInstance.GetEnabler().GetPropertyIndex (propIndex, ecPropertyAccessString.c_str());
        if (ecStatus == ECOBJECTS_STATUS_PropertyNotFound)
            return INSERT_ECError;

        ecStatus = ecInstance.GetValue (value, propIndex); 
        POSTCONDITION (ecStatus == ECOBJECTS_STATUS_Success, INSERT_ECError);

        if (ECDbUtility::IsECValueEmpty (value))
            continue; // There is nothing to insert.

        if (ecProperty.GetIsStruct())  // ClassMappingRule: Handling case where an embedded struct or struct array is mapped to a separate table
            {
            //WIP_ECDB: this seems very inefficient... in simple cases we can directly look up the ECValue... 
            ECValuesCollectionPtr collection = ECValuesCollection::Create (ecInstance);
            for (ECPropertyValueCR propertyValue : *collection)
                {                
                if (propertyValue.HasChildValues() && 
                    propertyValue.GetValueAccessor().GetPropertyName().EqualsI (ecProperty.GetName()))
                    { 
                    IECInstancePtr structInstance = ecProperty.GetAsStructProperty()->GetType().GetDefaultStandaloneEnabler()->CreateInstance();
                    if (CopyStruct (*structInstance, propertyValue) != ECOBJECTS_STATUS_Success)
                        return INSERT_ECError;

                    status  = inserter->Insert (ecInstanceLuid, *structInstance, NULL_ARRAY_INDEX);

                    if (INSERT_Success != status)
                        return status;
                    break;
                    }
                }
            }
        else if (ecProperty.GetIsArray())
            {
            uint32_t count = value.GetArrayInfo().GetCount();

            ArrayECPropertyCP arrayProperty = ecProperty.GetAsArrayProperty();
            if (arrayProperty->GetKind() == ARRAYKIND_Primitive)
                { 
                BeAssert (false && "This should never happen"); // WIP_ECDB: we should be able to eliminate this code path
                PrimitiveType primitiveType = arrayProperty->GetPrimitiveElementType();
                ECClassCR ecPrimitiveClass = m_ecDbMap.GetClassForPrimitiveArrayPersistence (primitiveType);
                IECInstancePtr inst = ecPrimitiveClass.GetDefaultStandaloneEnabler()->CreateInstance();

                for (uint32_t i = 0; i < count; i++)   
                    {
                    ecStatus = ecInstance.GetValue (value, propIndex, i);
                    if (ecStatus != ECOBJECTS_STATUS_Success)
                        return INSERT_ECError;

                    switch (primitiveType)
                        {
                        case PRIMITIVETYPE_String  : 
                        case PRIMITIVETYPE_Integer : 
                        case PRIMITIVETYPE_Long    : 
                        case PRIMITIVETYPE_Double  : 
                        case PRIMITIVETYPE_DateTime: 
                        case PRIMITIVETYPE_Binary  : 
                        case PRIMITIVETYPE_Boolean : 
                            inst->SetValue (L"ElementValue", value);
                            break;
                        case PRIMITIVETYPE_Point2D : 
                            {
                            DPoint2d point2d = value.GetPoint2D();
                            inst->SetValue (L"ElementValue_X", ECValue (point2d.x));
                            inst->SetValue (L"ElementValue_Y", ECValue (point2d.y));
                            break;
                            }
                        case PRIMITIVETYPE_Point3D :
                            {
                            DPoint3d point3d = value.GetPoint3D();
                            inst->SetValue (L"ElementValue_X", ECValue (point3d.x));
                            inst->SetValue (L"ElementValue_Y", ECValue (point3d.y));
                            inst->SetValue (L"ElementValue_Z", ECValue (point3d.z));
                            break;
                            }
                        }

                    status = inserter->Insert (ecInstanceLuid, *inst, i);
                    if (status != INSERT_Success)
                        return status;
                    }
                }
            else if (arrayProperty->GetKind() == ARRAYKIND_Struct)
                {
                for (uint32_t i = 0; i < count ; i++)   
                    {
                    ecStatus = ecInstance.GetValue (value, propIndex, i);
                    if (ecStatus != ECOBJECTS_STATUS_Success)
                        return INSERT_ECError;
                    //Null array element is not stored
                    if (!value.IsNull())
                        {
                        status = inserter->Insert (ecInstanceLuid, *value.GetStruct(), i);
                        if (status != INSERT_Success)
                            return status;
                        }
                    }
                }
            }
        }

    return INSERT_Success;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan        03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus InstanceInserter::CopyStruct (IECInstanceR ecInstance, ECPropertyValueCR propertyValue)
    {
    if (!propertyValue.HasChildValues())
        return ECOBJECTS_STATUS_Success;

    ECValuesCollectionPtr propertyValues = propertyValue.GetChildValues();
    ECObjectsStatus r;
    FOR_EACH (ECPropertyValueCR childPropertyValue, *propertyValues)
        {
        if (childPropertyValue.HasChildValues())
            {
             r = CopyStruct (ecInstance, childPropertyValue); 
             if (r != ECOBJECTS_STATUS_Success)
                 return r;
            }
        else
            {
            ECValueAccessorCR accessor  = childPropertyValue.GetValueAccessor();
            WString orignalAcccessString = accessor.GetAccessString();
            WString accessString= orignalAcccessString.substr (orignalAcccessString.find_first_of (L".") + 1);
            ECValueCR v = childPropertyValue.GetValue();
            if (v.IsNull())
                continue;
            r = ecInstance.SetValue (accessString.c_str(), v);
            if (r != ECOBJECTS_STATUS_Success && r != ECOBJECTS_STATUS_PropertyValueMatchesNoChange)
                return r;
            }
        }
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan        03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BeRepositoryBasedIdSequenceR InstanceInserter::GetECInstanceIdSequence () const
    {
    return m_ecInstanceIdSequence;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
InsertStatus InstanceInserter::VerifyExpectedNumberOfBindings (IClassMap const& classMap)
    {
    if (m_parameterBindings.size() == 0)
        return INSERT_Success;

    int nParametersToBind = -1;
    for (int i = (int)m_parameterBindings.size() - 1; i >= 0; i--)
        {
        if (m_parameterBindings[i].m_sqlIndex != BINDING_NotBound)
            {
            nParametersToBind = m_parameterBindings[i].m_sqlIndex;
            break;
            }
        }

    // nParametersToBind == -1 means we are not binding any columns to parameters, and we don't have enough information to 
    // do the verification... we don't know how many key columns there are
    if (nParametersToBind == -1)
        return INSERT_Success;

    int nPlaceholders = m_insertBuilder.GetParameterCount();
    if (nParametersToBind != nPlaceholders)  
        {
        LOG.errorv(L"InstanceInserter has %d parameter placeholders, but %d bindings", nPlaceholders, nParametersToBind);
        BeAssert (false && "Mismatch in VerifyExpectedNumberOfBindings");
        return INSERT_MapError;
        }

    return INSERT_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
InstancePropertyToTableInserter::InstancePropertyToTableInserter 
(
ECDbMapCR ecDbMap,
PropertyMapToTableCR propertyMapToTable,
BeRepositoryBasedIdSequenceR ecInstanceIdSequence,
InstanceInserterCP parentInserter)
 : InstanceInserter (ecDbMap, propertyMapToTable.GetElementType(), ecInstanceIdSequence, parentInserter), m_ecProperty (propertyMapToTable.GetProperty()), m_ecPropertyAccessString (propertyMapToTable.GetPropertyAccessString())
    {
    // TODO: Can we just store a reference to the property map itself, instead of the various fields?? 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
InsertStatus InstancePropertyToTableInserter::_Initialize()
    {
    auto classMap = m_ecDbMap.GetClassMap (m_ecClass);
    PRECONDITION (classMap != nullptr && !classMap->IsUnmapped(), INSERT_MapError);
    DbTableR table = classMap->GetTable();
    PRECONDITION (m_ecDbMap.GetECDbR().TableExists (table.GetName()), INSERT_MapError);

    int nextParameterIndex = 1; // Parameter indices are 1 based rather than 0 based
    AppendSqlToInsertIntoTable (table);
    InsertStatus status = AppendSqlToInsertPrimaryKeyOfInstance (nextParameterIndex, table, true);
    if (status != INSERT_Success)
        return status;

    AppendSqlToInsertECClassId (classMap->GetClass().GetId(), table);

    status = AppendSqlToInsertPropertyValues (nextParameterIndex, *classMap);
    if (status != INSERT_Success)
        return status;

    if (!m_insertBuilder.GetSql(m_sqlString))
        return INSERT_FailedToGenerateSql;

    BeAssert (GetParent () != nullptr);
    ECClassId rootClassId = GetParent ()->GetClass().GetId ();
    Utf8String accessString = Utf8String (m_ecPropertyAccessString);
    m_propertyIdForPersistence = ECDbSchemaPersistence::GetECPropertyAlias (rootClassId, accessString.c_str (), m_ecDbMap.GetECDbR ());
    if (m_propertyIdForPersistence == 0)
        m_propertyIdForPersistence = m_ecProperty.GetId ();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult InstancePropertyToTableInserter::_Bind (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance, uint32_t arrayIndex)
    {
    m_statement.ClearBindings();
    BindPrimaryKeysForProperty (ecInstanceId, ecInstance, arrayIndex);
    DbResult r = BindPropertyValues (ecInstance);
    return r;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult InstancePropertyToTableInserter::BindPrimaryKeysForProperty (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance, uint32_t arrayIndex)
    {    
    ECInstanceId generatedECId;
    DbResult stat = m_ecInstanceIdSequence.GetNextValue (generatedECId);
    POSTCONDITION (stat == BE_SQLITE_OK, stat);

    stat = m_statement.BindId (1, generatedECId);
    POSTCONDITION (stat == BE_SQLITE_OK, stat);

    stat = m_statement.BindId (2, *ecInstanceId);
    POSTCONDITION (stat == BE_SQLITE_OK, stat);

    *ecInstanceId = generatedECId;

    stat = m_statement.BindInt64 (3, m_propertyIdForPersistence); // WIP_ECDB: we should have been able to HARDCODE_ECPROPERTYID when we constructed the inserter's sql, m_ecProperty will not change
    POSTCONDITION (stat == BE_SQLITE_OK, stat);

    if (arrayIndex == NULL_ARRAY_INDEX) 
        {
        stat = m_statement.BindNull (4);
        POSTCONDITION (stat == BE_SQLITE_OK, stat);
        }
    else
        {
        stat = m_statement.BindInt (4, arrayIndex);
        POSTCONDITION (stat == BE_SQLITE_OK, stat);
        }
    
    return BE_SQLITE_OK;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
InsertStatus InstancePropertyToTableInserter::_Insert (ECInstanceId* ecInstanceLuid, ECN::IECInstanceR ecInstance, uint32_t arrayIndex)
    {
    InsertStatus status = INSERT_Success;
    if (!m_statement.IsPrepared())
        {
        DbResult r = m_statement.Prepare (m_ecDbMap.GetECDbR(), m_sqlString.c_str());
        if (BE_SQLITE_OK != r)
            return INSERT_SqliteError;
        }
    else
        m_statement.Reset(); 

    BeAssert (ecInstanceLuid != nullptr);
    ECInstanceId generatedECId = *ecInstanceLuid;

    DbResult r = _Bind (&generatedECId, ecInstance, arrayIndex);
    POSTCONDITION (BE_SQLITE_OK == r, INSERT_SqliteError);

    r =  m_statement.Step();
    if (BE_SQLITE_DONE != r)
        return IsConstraintDbResult(r) ? INSERT_ConstraintViolation : INSERT_SqliteError;

    // process struct/array properties
    DoInsertionsInSecondaryTables (&generatedECId, ecInstance);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
InstancePropertyToTableInserterPtr InstancePropertyToTableInserter::Create 
(
InsertStatus& status, 
ECDbMapCR ecDbMap,
PropertyMapToTableCR propertyMapToTable,
BeRepositoryBasedIdSequenceR ecInstanceIdSequence,
InstanceInserterCP parentInserter
)
    {
    InstancePropertyToTableInserterPtr inserter = new InstancePropertyToTableInserter (ecDbMap, propertyMapToTable, ecInstanceIdSequence, parentInserter);
    status = inserter->_Initialize();
    return (status == INSERT_Success) ? inserter : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
InsertStatus InstancePropertyToTableInserter::Insert (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance, uint32_t arrayIndex)
    {
    return _Insert (ecInstanceId, ecInstance, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipInstanceInserter::RelationshipInstanceInserter (ECDbMapCR ecDbMap, ECRelationshipClassCR relClass, BeRepositoryBasedIdSequenceR ecInstanceIdSequence) 
: InstanceInserter (ecDbMap, relClass, ecInstanceIdSequence, nullptr) 
    {
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipInstanceEndTableInserter::RelationshipInstanceEndTableInserter (ECDbMapCR ecDbMap, ECRelationshipClassCR relClass, BeRepositoryBasedIdSequenceR ecInstanceIdSequence)  
: RelationshipInstanceInserter (ecDbMap, relClass, ecInstanceIdSequence), m_classMap (nullptr), m_primaryKeyBinder (nullptr)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipInstanceEndTableInserter::AppendSqlToUpdateRelationshipOtherEnd
(
int& nextParameterIndex, 
RelationshipClassEndTableMapCR relationshipClassMap
)
    {
    // Get the non-persisted "other" end of the relationship
    MapStrategy mapStrategy = relationshipClassMap.GetMapStrategy();
    ECRelationshipEnd otherEnd = (mapStrategy == MapStrategy::RelationshipSourceTable) ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;

    auto rkColumn = relationshipClassMap.GetOtherEndECInstanceIdPropMap()->GetFirstColumn();
    BeAssert (rkColumn != nullptr);
    m_updateBuilder.AddUpdateColumn(rkColumn->GetName(), "?");
    m_relationshipOtherEndBinders.push_back (new BindRelationshipEndInstanceId (nextParameterIndex, otherEnd));

    auto rcColumn = relationshipClassMap.GetOtherEndECClassIdPropMap()->GetFirstColumn();
    BeAssert (rcColumn != nullptr);
    if (!rcColumn->IsVirtual ())
        {
        m_updateBuilder.AddUpdateColumn(rcColumn->GetName(), "?");
        m_relationshipOtherEndBinders.push_back (new BindRelationshipEndClassId (nextParameterIndex, otherEnd));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
InsertStatus RelationshipInstanceEndTableInserter::AppendSqlToUpdatePropertyValues 
(
int& nextParameterIndex, 
IClassMap const& classMap
)
    {
    ClassMapCP classMap2 = dynamic_cast<ClassMapCP> (&classMap);
    BeAssert (classMap2 != nullptr);
    BentleyStatus s = classMap2->GenerateParameterBindings(m_parameterBindings, nextParameterIndex);
    POSTCONDITION (s == SUCCESS, INSERT_MapError);
    FOR_EACH (BindingR binding, m_parameterBindings)
        {
        if (!binding.m_column)
            continue; // WIP_ECDB: When we start allowing update of only selected properties, this should really be an error... the caller is trying to update something that does not exist

        m_updateBuilder.AddUpdateColumn (binding.m_column->GetName (), "?");
        nextParameterIndex++;
        }

    return INSERT_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
InsertStatus RelationshipInstanceEndTableInserter::AppendSqlForPrimaryKeyWhereClause
(
int& nextParameterIndex, 
DbTableCR table
)
    {
    // Get the persisted end of the relationship
    MapStrategy mapStrategy = m_classMap->GetMapStrategy();
    ECRelationshipEnd persistenceEnd = (mapStrategy == MapStrategy::RelationshipSourceTable) ?  ECRelationshipEnd_Source : ECRelationshipEnd_Target;

    bvector<DbColumnP> primaryKeyColumns;
    table.GetPrimaryKeyTableConstraint().GetColumns (primaryKeyColumns);
    if (primaryKeyColumns.size() == 0)
        return INSERT_FailedToGenerateSql;

    BeAssert (primaryKeyColumns.size() > 0);
    DbColumnP primaryKeyColumn = primaryKeyColumns.front();

    auto rkColumn = m_classMap->GetOtherEndECInstanceIdPropMap ()->GetFirstColumn ();
    BeAssert (rkColumn != nullptr);

    //Check for other end ecinstanceid column being null. This allows us to detect whether 
    //the client tries to insert a second instance on the 0..1 end which would be a constraint violation
    Utf8String whereClause (primaryKeyColumn->GetName ());
    whereClause.append ("= ? AND ").append (rkColumn->GetName ()).append (" IS NULL");
    m_updateBuilder.AddWhere (whereClause.c_str ());
    m_primaryKeyBinder = new BindRelationshipEndInstanceId (nextParameterIndex, persistenceEnd);

    return INSERT_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
InsertStatus RelationshipInstanceEndTableInserter::_Initialize()
    {
    auto classMap = m_ecDbMap.GetClassMap (m_ecClass);
    PRECONDITION (!classMap->IsUnmapped(), INSERT_MapError);
    DbTableCR endTable = classMap->GetTable ();
    auto relationshipClassMap = dynamic_cast<RelationshipClassEndTableMap const*> (classMap);
    BeAssert (relationshipClassMap != nullptr);
    m_classMap = relationshipClassMap;

    m_updateBuilder.SetTable(endTable.GetName());

    int nextParameterIndex = 1; // Parameter indices are 1 based rather than 0 based
    AppendSqlToUpdateRelationshipOtherEnd (nextParameterIndex, *relationshipClassMap);
    
    InsertStatus status = AppendSqlToUpdatePropertyValues (nextParameterIndex, *classMap);
    if (status != INSERT_Success)
        return status;

    status = AppendSqlForPrimaryKeyWhereClause (nextParameterIndex, endTable);
    if (status != INSERT_Success)
        return status;

    m_updateBuilder.GetSql(m_sqlString);

    status = TrySetupPropertyToTableInserters (*classMap);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult RelationshipInstanceEndTableInserter::BindRelationshipOtherEnd (IECRelationshipInstanceCR relationshipInstance)
    {
    for (auto relationshipBinder : m_relationshipOtherEndBinders)
        {
        DbResult r = relationshipBinder->Bind (m_statement, relationshipInstance);
        if (BE_SQLITE_OK != r)
            return r;
        }
    return BE_SQLITE_OK;
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult RelationshipInstanceEndTableInserter::BindPrimaryKeyWhereClause (ECInstanceId* ecInstanceId, IECRelationshipInstanceR relationshipInstance, bool useSuppliedECInstanceId)
    {
    // Set the relationship instance id from the end instance
    ECInstanceId endInstanceId;
    if (!m_primaryKeyBinder->GetEndInstanceId (endInstanceId, relationshipInstance))
        return BE_SQLITE_ERROR;

    // Bind the parameter in the where clause
    DbResult r = m_primaryKeyBinder->Bind (m_statement, relationshipInstance);
    if (r != BE_SQLITE_OK)
        return r;
    
    // Setup instance id of the relationship instance
    if (useSuppliedECInstanceId)
        {
        if (!EXPECTED_CONDITION (ecInstanceId != nullptr))
            return BE_SQLITE_ERROR;
        if (*ecInstanceId != endInstanceId)
            {
            LOG.errorv (L"Supplied instance id does not match the id of the relationship end, and cannot be setup");
            return BE_SQLITE_ERROR;
            }
        }
    else
        {
        if (nullptr != ecInstanceId)
            *ecInstanceId = endInstanceId;
        }

    ECInstanceAdapterHelper::SetECInstanceId (relationshipInstance, endInstanceId);
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult RelationshipInstanceEndTableInserter::_Bind (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance, bool useSuppliedECInstanceId)
    {
    DbResult r;
    IECRelationshipInstance* relationshipInstance = dynamic_cast<IECRelationshipInstance*> (&ecInstance);
    BeAssert (nullptr != relationshipInstance);

    m_statement.ClearBindings();
    r = BindRelationshipOtherEnd (*relationshipInstance);
    if (r != BE_SQLITE_OK)
        return r;
    r = BindPropertyValues (*relationshipInstance);
    if (r != BE_SQLITE_OK)
        return r;
    r = BindPrimaryKeyWhereClause (ecInstanceId, *relationshipInstance, useSuppliedECInstanceId);
    return r;
    }

//------------------------------------------------------------------------------------------------
//@bsimethod                                   Krischan.Eberle                   06 / 14
//+---------------+---------------+---------------+--------------+---------------+----------------
InsertStatus RelationshipInstanceEndTableInserter::_Insert (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance, bool useSuppliedECInstanceId)
    {
    auto stat = RelationshipInstanceInserter::_Insert (ecInstanceId, ecInstance, useSuppliedECInstanceId);
    if (INSERT_Success != stat)
        return stat;

    //do additional check that insert did not violate cardinality
    if (0 == m_ecDbMap.GetECDbR ().GetModifiedRowCount ())
        {
        IECRelationshipInstanceP relationshipInstance = dynamic_cast<IECRelationshipInstanceP> (&ecInstance);
        BeAssert (nullptr != relationshipInstance);

        WCharCP sourceEndName = L"source";
        WCharCP targetEndName = L"target";
        WCharCP thisEndName = nullptr;
        WCharCP otherEndName = nullptr;
        if (m_classMap->GetThisEnd () == ECN::ECRelationshipEnd_Source)
            {
            thisEndName = sourceEndName;
            otherEndName = targetEndName;
            }
        else
            {
            thisEndName = targetEndName;
            otherEndName = sourceEndName;
            }

        auto sourceInstance = relationshipInstance->GetSource ();
        auto targetInstance = relationshipInstance->GetTarget ();
        LOG.errorv (L"Inserting %ls ECRelationship (Source: %ls:%ls, Target: %ls:%ls) failed. "
                    L"Either %ls instance does not exist or cardinality constraint is violated as %ls instance already has a relationship to a %ls instance.",
                  m_ecClass.GetFullName (), sourceInstance->GetClass ().GetFullName (), sourceInstance->GetInstanceId ().c_str (),
                  targetInstance->GetClass ().GetFullName (), targetInstance->GetInstanceId ().c_str (),
                  thisEndName, otherEndName, thisEndName);

        return INSERT_InvalidInputInstance;
        }

    return INSERT_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipInstanceLinkTableInserter::RelationshipInstanceLinkTableInserter (ECDbMapCR ecDbMap, ECRelationshipClassCR relClass, BeRepositoryBasedIdSequenceR ecInstanceIdSequence)  
: RelationshipInstanceInserter (ecDbMap, relClass, ecInstanceIdSequence) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipInstanceLinkTableInserter::AppendSqlForRelationshipEnd 
(
int& nextParameterIndex,
ECRelationshipEnd relationEnd, 
RelationshipClassLinkTableMapCR relationshipClassMap
)
    {
    auto rkColumn = relationshipClassMap.GetConstraintECInstanceIdPropMap (relationEnd)->GetFirstColumn();
    BeAssert (rkColumn != nullptr);
    m_insertBuilder.AddInsertColumn (rkColumn->GetName (), "?");
    m_relationshipEndBinders.push_back (new BindRelationshipEndInstanceId (nextParameterIndex, relationEnd));

    auto rcColumn = relationshipClassMap.GetConstraintECClassIdPropMap (relationEnd)->GetFirstColumn();
    BeAssert (rcColumn != nullptr);
    if (!rcColumn->IsVirtual ())
        {
        m_insertBuilder.AddInsertColumn (rcColumn->GetName (), "?");
        m_relationshipEndBinders.push_back (new BindRelationshipEndClassId (nextParameterIndex, relationEnd));
        }

    // TODO: Binders elsewhere are with the maps, and get added by the maps. This pattern seems better? Make it consistent.
    // TODO: This logic of checking for rel columns is in different places. 
    // WIP_ECDB: switch to "bindings"
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipInstanceLinkTableInserter::AppendSqlToInsertRelationships (int& nextParameterIndex)
    {
    auto classMap = m_ecDbMap.GetClassMap (m_ecClass);
    auto relationshipClassMap = dynamic_cast<RelationshipClassLinkTableMapCP> (classMap);
    BeAssert (relationshipClassMap != nullptr);

    AppendSqlForRelationshipEnd (nextParameterIndex, ECRelationshipEnd_Source, *relationshipClassMap);
    AppendSqlForRelationshipEnd (nextParameterIndex, ECRelationshipEnd_Target, *relationshipClassMap);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
InsertStatus RelationshipInstanceLinkTableInserter::_Initialize()
    {
    auto classMap = m_ecDbMap.GetClassMap (m_ecClass);
    PRECONDITION (!classMap->IsUnmapped(), INSERT_MapError);
    DbTableR table = classMap->GetTable();
    PRECONDITION (m_ecDbMap.GetECDbR().TableExists (table.GetName()), INSERT_MapError);

    AppendSqlToInsertIntoTable (table);

    int nextParameterIndex = 1; // Parameter indices are 1 based rather than 0 based
    InsertStatus status = AppendSqlToInsertPrimaryKeyOfInstance (nextParameterIndex, table, true);
    if (status != INSERT_Success)
        return status;

    AppendSqlToInsertECClassId (classMap->GetClass().GetId(), table);
    AppendSqlToInsertRelationships (nextParameterIndex);
    
    status = AppendSqlToInsertPropertyValues (nextParameterIndex, *classMap);
    if (status != INSERT_Success)
        return status;

    m_insertBuilder.GetSql(m_sqlString);
    LOG.tracev ("Generated insert for link table: %s", m_sqlString.c_str());
    
    status = TrySetupPropertyToTableInserters (*classMap);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult RelationshipInstanceLinkTableInserter::BindRelationshipEnds (IECRelationshipInstanceCR relationshipInstance)
    {
    for (auto relationshipBinder : m_relationshipEndBinders)
        {
        DbResult r = relationshipBinder->Bind (m_statement, relationshipInstance);
        if (BE_SQLITE_OK != r)
            return r;
        }
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult RelationshipInstanceLinkTableInserter::_Bind (ECInstanceId* ecInstanceId, ECN::IECInstanceR ecInstance, bool useSuppliedECInstanceId)
    {
    DbResult r;
    IECRelationshipInstance* relationshipInstance = dynamic_cast<IECRelationshipInstance*> (&ecInstance);
    BeAssert (nullptr != relationshipInstance);

    m_statement.ClearBindings();
    r = BindPrimaryKeysOfInstance (ecInstanceId, *relationshipInstance, useSuppliedECInstanceId);
    if (r != BE_SQLITE_OK)
        return r;
    r = BindPropertyValues (*relationshipInstance);
    if (r != BE_SQLITE_OK)
        return r;
    r = BindRelationshipEnds (*relationshipInstance);
    return r;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
