/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/JsonReader.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
//@bsimethod                                    Ramanujam.Raman                 9 / 2013
//+---------------+---------------+---------------+---------------+---------------+------
JsonReader::JsonReader(ECDbCR ecdb, ECN::ECClassId ecClassId) : m_ecDb(ecdb), m_statementCache(50, "JsonReader ECSqlStatement Cache")
    {
    m_ecClass = m_ecDb.Schemas().GetECClass(ecClassId);
    BeAssert(m_ecClass != nullptr && "Could not retrieve class with specified id");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 09 / 2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::Read(JsonValueR jsonInstances, JsonValueR jsonDisplayInfo, ECInstanceId ecInstanceId, JsonECSqlSelectAdapter::FormatOptions formatOptions) const
    {
    jsonInstances = Json::arrayValue;
    jsonDisplayInfo = Json::objectValue;

    if (!IsValid())
        return ERROR;

    // Add any instances of the specified class itself
    ECRelationshipPath trivialPathToClass;
    BentleyStatus status = GetTrivialPathToSelf(trivialPathToClass, *m_ecClass);
    if (SUCCESS == status)
        status = AddInstancesFromSpecifiedClassPath(jsonInstances, jsonDisplayInfo, trivialPathToClass, ecInstanceId, formatOptions, false /*isPolymorphic*/);

    // Add any related instances according to the "RelatedItemsDisplaySpecification" custom attribute
    if (SUCCESS == status)
        status = AddInstancesFromRelatedItems(jsonInstances, jsonDisplayInfo, *m_ecClass, trivialPathToClass, ecInstanceId, formatOptions);

    if (status != SUCCESS)
        {
        jsonInstances = Json::nullValue;
        jsonDisplayInfo = Json::nullValue;
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 09 / 2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::ReadInstance(Json::Value& jsonValue, ECInstanceId ecInstanceId, JsonECSqlSelectAdapter::FormatOptions formatOptions) const
    {
    if (!IsValid())
        return ERROR;

    ECRelationshipPath emptyPath;
    if (SUCCESS != GetTrivialPathToSelf(emptyPath, *m_ecClass))
        return ERROR;

    Utf8String ecSql;
    if (SUCCESS != GenerateECSql(ecSql, emptyPath, false/*selectInstanceKeyOnly*/, false/*isPolymorphic*/))
        return ERROR;

    CachedECSqlStatementPtr statement = nullptr;
    if (SUCCESS != PrepareAndBindStatement(statement, ecSql, ecInstanceId))
        return ERROR;

    DbResult stepStatus = statement->Step();
    if (stepStatus != BE_SQLITE_ROW)
        {
        BeAssert(false && "Instance not found");
        return ERROR;
        }

    JsonECSqlSelectAdapter jsonAdapter(*statement, formatOptions);
    if (!jsonAdapter.GetRowInstance(jsonValue, m_ecClass->GetId()))
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 05 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::AddInstancesFromRelatedItems(JsonValueR allInstances, JsonValueR allDisplayInfo, ECClassCR parentClass,
                                                       ECRelationshipPath const& pathFromParent, ECInstanceId ecInstanceId, JsonECSqlSelectAdapter::FormatOptions const& formatOptions) const
    {
    bvector<ECRelationshipPath> appendPathsFromParent;
    JsonReader::ECRelatedItemsDisplaySpecificationsCache* appData = JsonReader::ECRelatedItemsDisplaySpecificationsCache::Get(m_ecDb);
    if (appData == nullptr)
        return ERROR;

    if (!appData->TryGetRelatedPaths(appendPathsFromParent, parentClass))
        return SUCCESS;

    ECRelationshipPath pathToParent;
    pathFromParent.Reverse(pathToParent);

    for (ECRelationshipPath const& appendPath : appendPathsFromParent)
        {
        ECRelationshipPath pathToRelatedClass = pathToParent;
        pathToRelatedClass.Combine(appendPath);

        ECRelationshipPath pathFromRelatedClass;
        pathToRelatedClass.Reverse(pathFromRelatedClass);

        if (SUCCESS != AddInstancesFromSpecifiedClassPath(allInstances, allDisplayInfo, pathFromRelatedClass, ecInstanceId, formatOptions, true /*isPolymorphic*/))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::AddInstancesFromSpecifiedClassPath(JsonValueR allInstances, JsonValueR allDisplayInfo, ECRelationshipPath const& pathFromRelatedClass, ECInstanceId ecInstanceId, JsonECSqlSelectAdapter::FormatOptions const& formatOptions, bool isPolymorphic) const
    {
    // Note: We just do this in two stages to accommodate polymorphic cases - get the instance keys, and make a subsequent query with the classes that were found
    Utf8String ecSqlKey;
    if (SUCCESS != GenerateECSql(ecSqlKey, pathFromRelatedClass, true /* selectInstanceKeyOnly*/, isPolymorphic))
        return ERROR;

    CachedECSqlStatementPtr statementKey = nullptr;
    if (SUCCESS != PrepareAndBindStatement(statementKey, ecSqlKey, ecInstanceId))
        return ERROR;

    while (BE_SQLITE_ROW == statementKey->Step())
        {
        ECClassId selectClassId = statementKey->GetValueId<ECClassId>(0);
        ECInstanceId selectInstanceId = statementKey->GetValueId<ECInstanceId>(1);

        ECClassCP selectClass = m_ecDb.Schemas().GetECClass(selectClassId);
        BeAssert(selectClass != nullptr);

        Utf8PrintfString ecSql("SELECT * FROM ONLY %s AS el WHERE el.ECInstanceId=?", selectClass->GetECSqlName().c_str());

        CachedECSqlStatementPtr statement = nullptr;
        if (SUCCESS != PrepareAndBindStatement(statement, ecSql, selectInstanceId))
            return ERROR;

        ECRelationshipPath pathFromDerivedClass = pathFromRelatedClass;
        pathFromDerivedClass.SetEndClass(*selectClass, ECRelationshipPath::End::Root);

        if (SUCCESS != AddInstancesFromPreparedStatement(allInstances, allDisplayInfo, *statement, formatOptions, pathFromDerivedClass.ToString()))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 05 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::GetTrivialPathToSelf(ECRelationshipPath& emptyPath, ECClassCR ecClass) const
    {
    if (SUCCESS != emptyPath.InitFromString(ecClass.GetFullName(), m_ecDb.GetClassLocater(), &(ecClass.GetSchema())))
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 04/ 2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::GenerateECSql(Utf8StringR ecSql, ECRelationshipPath const& pathFromRelatedClass, bool selectInstanceKeyOnly, bool isPolymorphic) const
    {
    Utf8String fromClause, joinClause;
    ECRelationshipPath::GeneratedEndInfo rootInfo, leafInfo;
    if (SUCCESS != pathFromRelatedClass.GenerateECSql(fromClause, joinClause, rootInfo, leafInfo, isPolymorphic))
        return ERROR;
    BeAssert(m_ecClass == pathFromRelatedClass.GetEndClass(ECRelationshipPath::End::Leaf));

    Utf8String selectClause;
    if (selectInstanceKeyOnly)
        selectClause.Sprintf("SELECT %s, %s", rootInfo.GetClassIdExpression().c_str(), rootInfo.GetInstanceIdExpression().c_str());
    else
        selectClause.Sprintf("SELECT [%s].*", rootInfo.GetAlias().c_str());

    ecSql.Sprintf("%s %s %s WHERE %s.ECInstanceId=?", selectClause.c_str(), fromClause.c_str(), joinClause.c_str(), leafInfo.GetAlias().c_str());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// Prepare ECSQL to retrieve the instance specified by the path
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::PrepareAndBindStatement(CachedECSqlStatementPtr& statement, Utf8StringCR ecSql, ECInstanceId ecInstanceId) const
    {
    statement = m_statementCache.GetPreparedStatement(m_ecDb, ecSql.c_str());
    if (statement == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    ECSqlStatus bindStatus = statement->BindId(1, ecInstanceId);
    if (!bindStatus.IsSuccess())
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
void JsonReader::SetRelationshipPath(JsonValueR addClasses, Utf8StringCR pathFromRelatedClassStr)
    {
    for (Utf8StringCR key : addClasses.getMemberNames())
        addClasses[key]["RelationshipPath"] = pathFromRelatedClassStr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
void JsonReader::SetInstanceIndex(JsonValueR addCategories, int currentInstanceIndex)
    {
    for (int categoryIndex = 0; categoryIndex < (int) addCategories.size(); categoryIndex++)
        {
        Json::Value& addCategory = addCategories[categoryIndex];
        if (!addCategory.isMember("Properties"))
            continue; // empty category
        Json::Value& addProperties = addCategory["Properties"];
        for (int propertyIndex = 0; propertyIndex < (int) addProperties.size(); propertyIndex++)
            {
            Json::Value& addProperty = addProperties[propertyIndex];
            addProperty["InstanceIndex"] = currentInstanceIndex;
            if (addProperty.isMember("Categories"))
                SetInstanceIndex(addProperty["Categories"], currentInstanceIndex);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
void JsonReader::AddClasses(JsonValueR allClasses, JsonValueR addClasses)
    {
    for (Utf8StringCR key : addClasses.getMemberNames())
        {
        if (allClasses.isMember(key))
            continue;
        allClasses[key] = addClasses[key];
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
void JsonReader::AddCategories(JsonValueR allCategories, JsonValueR addCategories, int currentInstanceIndex)
    {
    // Recursively setup the instanceIndex in all the properties within addCategories
    SetInstanceIndex(addCategories, currentInstanceIndex);

    // Consolidate the categories
    for (int categoryIndex = 0; categoryIndex < (int) addCategories.size(); categoryIndex++)
        {
        // Consolidate the categories
        Json::Value& addCategory = addCategories[categoryIndex];
        if (!addCategory.isMember("Properties"))
            continue; // empty category

        Utf8String categoryName = addCategory["CategoryName"].asString();
        int existingCategoryIndex;
        for (existingCategoryIndex = 0; existingCategoryIndex < (int) allCategories.size(); existingCategoryIndex++)
            {
            if (allCategories[existingCategoryIndex]["CategoryName"].asString() == categoryName)
                break;
            }
        if (existingCategoryIndex >= (int) allCategories.size()) // Category not found
            {
            allCategories.append(addCategory);
            continue;
            }

        // Category found, consolidate the properties
        Json::Value& allProperties = allCategories[existingCategoryIndex]["Properties"];
        Json::Value& addProperties = addCategory["Properties"];
        for (int propertyIndex = 0; propertyIndex < (int) addProperties.size(); propertyIndex++)
            allProperties.append(addProperties[propertyIndex]);

        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
void JsonReader::AddInstances(JsonValueR allInstances, JsonValueR addInstances, int currentInstanceIndex)
    {
    // Consolidate the categories with previous instances
    for (int ii = 0; ii < (int) addInstances.size(); ii++)
        allInstances[currentInstanceIndex + ii] = addInstances[ii];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus JsonReader::AddInstancesFromPreparedStatement(JsonValueR allInstances, JsonValueR allDisplayInfo, ECSqlStatement& statement, JsonECSqlSelectAdapter::FormatOptions const& formatOptions, Utf8StringCR pathFromRelatedClassStr)
    {
    statement.Reset();
    JsonECSqlSelectAdapter jsonAdapter(statement, formatOptions);

    int currentInstanceIndex = (int) allInstances.size();

    while (BE_SQLITE_ROW == statement.Step())
        {
        if (currentInstanceIndex == 0)
            {
            // Setup instance the first time
            if (!jsonAdapter.GetRow(allInstances))
                return ERROR;

            // Setup display info the first time
            jsonAdapter.GetRowDisplayInfo(allDisplayInfo);
            SetRelationshipPath(allDisplayInfo["Classes"], pathFromRelatedClassStr);
            }
        else
            {
            /*
            * Consolidate instances
            */
            Json::Value addInstances;
            if (!jsonAdapter.GetRow(addInstances))
                return ERROR;

            AddInstances(allInstances, addInstances, currentInstanceIndex);

            /*
            * Consolidate display info
            */
            Json::Value addDisplayInfo;
            jsonAdapter.GetRowDisplayInfo(addDisplayInfo); // TODO: Setup move constructors in Json::Value
            SetRelationshipPath(addDisplayInfo["Classes"], pathFromRelatedClassStr);

            // Consolidate the categories
            if (!allDisplayInfo.isMember("Categories"))
                allDisplayInfo["Categories"] = Json::Value(Json::arrayValue);
            AddCategories(allDisplayInfo["Categories"], addDisplayInfo["Categories"], currentInstanceIndex);

            // Consolidate the classes
            AddClasses(allDisplayInfo["Classes"], addDisplayInfo["Classes"]);
            }

        currentInstanceIndex++;
        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 05 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
JsonReader::ECRelatedItemsDisplaySpecificationsCache* JsonReader::ECRelatedItemsDisplaySpecificationsCache::Get(ECDbCR ecdb)
    {
    ECRelatedItemsDisplaySpecificationsCache* appData = reinterpret_cast <ECRelatedItemsDisplaySpecificationsCache*> (ecdb.FindAppData(ECRelatedItemsDisplaySpecificationsCache::GetKey()));
    if (appData)
        return appData;

    bvector<ECN::ECSchemaCP> allSchemas;
    if (SUCCESS != ecdb.Schemas().GetECSchemas(allSchemas, false))
        {
        BeAssert(false);
        return nullptr;
        }

    appData = new ECRelatedItemsDisplaySpecificationsCache(ecdb);
    if (SUCCESS != appData->Initialize(allSchemas, ecdb.GetClassLocater()))
        {
        delete appData;
        return nullptr;
        }

    ecdb.AddAppData(ECRelatedItemsDisplaySpecificationsCache::GetKey(), appData, true);
    return appData;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 05 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::ECRelatedItemsDisplaySpecificationsCache::Initialize(bvector<ECSchemaCP> const& schemaList, IECClassLocater& classLocater)
    {
    ECClassCP ecClass = classLocater.LocateClass("Bentley_Standard_CustomAttributes", "RelatedItemsDisplaySpecifications");
    if (!ecClass)
        {
        BeAssert(false);
        return ERROR;
        }

    ECCustomAttributeClassCP relatedItemDisplaySpecCA = ecClass->GetCustomAttributeClassCP();
    if (!relatedItemDisplaySpecCA)
        {
        BeAssert(false);
        return ERROR;
        }

    BentleyStatus status = SUCCESS;
    for (ECSchemaCP schema : schemaList)
        {
        IECInstancePtr customAttribute = schema->GetCustomAttribute(*relatedItemDisplaySpecCA);
        if (customAttribute.IsNull())
            continue;

        ECValue specificationsValue;
        if (ECObjectsStatus::Success != customAttribute->GetValue(specificationsValue, "Specifications"))
            continue;

        ArrayInfo arrayInfo = specificationsValue.GetArrayInfo();
        for (int ii = 0; ii < (int) arrayInfo.GetCount(); ii++)
            {
            ECValue specificationValue;
            customAttribute->GetValue(specificationValue, "Specifications", ii);
            IECInstancePtr specificationInstance = specificationValue.GetStruct();
            if (specificationInstance.IsNull())
                continue;

            if (SUCCESS != ExtractFromCustomAttribute(*specificationInstance, classLocater, *schema))
                {
                status = ERROR;
                continue; // best effort
                }
            }
        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
ECClassCP ResolveClass(Utf8StringCR possiblyQualifiedClassName, IECClassLocaterR classLocater, ECSchemaCP defaultSchema)
    {
    Utf8String schemaName, className;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(schemaName, className, possiblyQualifiedClassName))
        return nullptr;

    if (!schemaName.empty())
        return classLocater.LocateClass(schemaName.c_str(), className.c_str());

    return classLocater.LocateClass(defaultSchema->GetName().c_str(), className.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::ECRelatedItemsDisplaySpecificationsCache::ExtractFromCustomAttribute(IECInstanceCR customAttributeSpecification, IECClassLocater& classLocater, ECSchemaCR customAttributeContainerSchema)
    {
    // Construct "end-to-end" relationship paths that prepends the ParentClass to the specified RelationshipPath. 
    Utf8String relationshipPathString;

    // Find parent or "root" class
    ECValue ecValueParentClass;
    ECObjectsStatus ecStatus = customAttributeSpecification.GetValue(ecValueParentClass, "ParentClass");
    PRECONDITION(ECObjectsStatus::Success == ecStatus && !ecValueParentClass.IsNull(), ERROR);

    // Append parent or "root" class
    relationshipPathString.append(ecValueParentClass.GetUtf8CP());

    // Append relationship path string
    ECValue ecValueRelationshipPath;
    ecStatus = customAttributeSpecification.GetValue(ecValueRelationshipPath, "RelationshipPath");
    PRECONDITION(ECObjectsStatus::Success == ecStatus && !ecValueRelationshipPath.IsNull(), ERROR);
    relationshipPathString.append(".");
    relationshipPathString.append(ecValueRelationshipPath.GetUtf8CP());

    // Create relationship path from string (contains the base class as the leaf class)
    ECRelationshipPath basePath;
    if (SUCCESS != basePath.InitFromString(relationshipPathString, classLocater, &customAttributeContainerSchema))
        return ERROR;
    if (!basePath.Validate())
        return ERROR;
    
    // Create a relationship path for every DerivedClass specified
    BentleyStatus status = SUCCESS;
    bool hasDerivedClasses = false;
    ECValue derivedClassesValue;
    ecStatus = customAttributeSpecification.GetValue(derivedClassesValue, "DerivedClasses");
    if (ecStatus == ECObjectsStatus::Success)
        {
        ArrayInfo arrayInfo = derivedClassesValue.GetArrayInfo();
        for (int ii = 0; ii < (int) arrayInfo.GetCount(); ii++)
            {
            ECValue val;
            customAttributeSpecification.GetValue(val, "DerivedClasses", ii);
            if (val.IsNull())
                continue;

            Utf8String derivedClassName(val.GetUtf8CP());
            ECClassCP derivedClass = ResolveClass(derivedClassName, classLocater, &customAttributeContainerSchema);
            if (derivedClass == nullptr)
                continue;

            ECRelationshipPath derivedPath = basePath;
            derivedPath.SetEndClass(*derivedClass, ECRelationshipPath::End::Leaf);

            if (!derivedPath.Validate())
                {
                status = ERROR;
                continue;
                }

            AddPathToCache(derivedPath);
            hasDerivedClasses = true;
            }
        }
    
    if (!hasDerivedClasses)
        AddPathToCache(basePath); // Use the base path if there aren't any derived classes specified. 

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 05 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
void JsonReader::ECRelatedItemsDisplaySpecificationsCache::AddPathToCache(ECRelationshipPath const& path)
    {
    ECClassCP parentClass = path.GetEndClass(ECRelationshipPath::End::Root);

    auto it = m_pathsByClass.find(parentClass->GetId());
    bvector<ECRelationshipPath>* pathVector = nullptr;
    if (it != m_pathsByClass.end())
        pathVector = &it->second;
    else
        pathVector = &m_pathsByClass[parentClass->GetId()];

    BeAssert(pathVector != nullptr);

    // Check for duplicate entries!!
    for (ECRelationshipPath& existingPath : *pathVector)
        {
        Utf8String existingPathStr = existingPath.ToString();
        Utf8String currentPathStr = path.ToString();
        if (existingPathStr.Equals(currentPathStr))
            return;
        }

    pathVector->push_back(path);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 05 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
bool JsonReader::ECRelatedItemsDisplaySpecificationsCache::TryGetRelatedPaths(bvector<ECRelationshipPath>& relationshipPathVec, ECClassCR ecClass) const
    {
    for (bmap<ECClassId, bvector<ECRelationshipPath>>::const_iterator iter = m_pathsByClass.begin(); iter != m_pathsByClass.end(); iter++)
        {
        ECClassId classId = iter->first;
        ECClassCP pathClass = m_ecDb.Schemas().GetECClass(classId);
        if (ecClass.Is(pathClass))
            {
            bvector<ECRelationshipPath> const& pathVector = iter->second;
            relationshipPathVec.insert(relationshipPathVec.end(), pathVector.begin(), pathVector.end());
            }
        }

    return !relationshipPathVec.empty();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
