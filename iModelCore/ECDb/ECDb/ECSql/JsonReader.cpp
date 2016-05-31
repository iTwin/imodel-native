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
    jsonInstances = Json::nullValue;
    jsonDisplayInfo = Json::nullValue;

    if (!IsValid())
        return ERROR;

    // Add any instances of the specified class itself
    ECRelationshipPath trivialPathToClass;
    if (SUCCESS != GetTrivialPathToSelf(trivialPathToClass, *m_ecClass))
        return ERROR;

    jsonInstances = Json::arrayValue;
    jsonDisplayInfo = Json::objectValue;

    BentleyStatus status = AddInstancesFromSpecifiedClassPath(jsonInstances, jsonDisplayInfo, trivialPathToClass, ecInstanceId, formatOptions, false /*isPolymorphic*/);

    // Add any related instances according to the "RelatedItemsDisplaySpecification" custom attribute
    if (SUCCESS != AddInstancesFromRelatedItems(jsonInstances, jsonDisplayInfo, *m_ecClass, trivialPathToClass, ecInstanceId, formatOptions))
        status = ERROR;

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
    JsonReader::RelatedItemsDisplaySpecificationsCache* appData = JsonReader::RelatedItemsDisplaySpecificationsCache::Get(m_ecDb);
    if (appData == nullptr)
        return ERROR;

    if (!appData->TryGetRelatedPaths(appendPathsFromParent, parentClass))
        return SUCCESS;

    ECRelationshipPath pathToParent;
    pathFromParent.Reverse(pathToParent);

    BentleyStatus status = SUCCESS;
    for (ECRelationshipPath const& appendPath : appendPathsFromParent)
        {
        ECRelationshipPath pathToRelatedClass = pathToParent;
        pathToRelatedClass.Combine(appendPath);

        ECRelationshipPath pathFromRelatedClass;
        pathToRelatedClass.Reverse(pathFromRelatedClass);

        if (SUCCESS != AddInstancesFromSpecifiedClassPath(allInstances, allDisplayInfo, pathFromRelatedClass, ecInstanceId, formatOptions, true /*isPolymorphic*/))
            status = ERROR;
        }

    return status;
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

    BentleyStatus status = SUCCESS;
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
            {
            status = ERROR;
            continue;
            }
        }

    return status;
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
        allInstances[ii + currentInstanceIndex] = addInstances[ii];
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

    BentleyStatus status = SUCCESS;

    while (BE_SQLITE_ROW == statement.Step())
        {
        if (currentInstanceIndex == 0)
            {
            // Setup instance the first time
            if (!jsonAdapter.GetRow(allInstances))
                status = ERROR;
            
            if (allInstances.size() != 1)
                {
                status = ERROR;
                BeAssert(false);
                }

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
                status = ERROR;

            if (addInstances.size() != 1)
                {
                status = ERROR;
                BeAssert(false);
                }

            allInstances[currentInstanceIndex] = addInstances[0];

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

        currentInstanceIndex = allInstances.size();
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
ECClassCP JsonReader::RelatedItemsDisplaySpecificationsCache::ResolveClass(Utf8StringCR possiblyQualifiedClassName, ECSchemaCR defaultSchema) const
    {
    Utf8String schemaName, className;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(schemaName, className, possiblyQualifiedClassName))
        return nullptr;

    if (schemaName.empty())
       schemaName = defaultSchema.GetName();

    ECClassCP ecClass = m_ecDb.Schemas().GetECClass(schemaName.c_str(), className.c_str());
    BeAssert(ecClass != nullptr);

    return ecClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 05 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
JsonReader::RelatedItemsDisplaySpecificationsCache* JsonReader::RelatedItemsDisplaySpecificationsCache::Get(ECDbCR ecdb)
    {
    RelatedItemsDisplaySpecificationsCache* appData = reinterpret_cast <RelatedItemsDisplaySpecificationsCache*> (ecdb.FindAppData(RelatedItemsDisplaySpecificationsCache::GetKey()));
    if (appData)
        return appData;

    appData = new RelatedItemsDisplaySpecificationsCache(ecdb);
    if (SUCCESS != appData->Initialize())
        {
        delete appData;
        return nullptr;
        }

    ecdb.AddAppData(RelatedItemsDisplaySpecificationsCache::GetKey(), appData, true);
    return appData;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 05 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::RelatedItemsDisplaySpecificationsCache::Initialize()
    {
    RelationshipPathInfosByClass pathInfosByClass;
    if (SUCCESS != GatherRelationshipPathInfos(pathInfosByClass))
        return ERROR;

    RemoveDuplicates(pathInfosByClass);
    
    if (SUCCESS != ExtractRelationshipPaths(pathInfosByClass))
        return ERROR;

    SortRelationshipPaths(); // Only needed for ATPs to maintain stable order

    if (LOG.isSeverityEnabled(NativeLogging::LOG_TRACE))
        DumpCache();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------------------
// Gathers information on relationship paths from RelatedDisplayCustomAttributes in schemas in the Db.
// @bsimethod                                Ramanujam.Raman                           05 / 2016
//+---------------+---------------+---------------+---------------+---------------+------------------
BentleyStatus JsonReader::RelatedItemsDisplaySpecificationsCache::GatherRelationshipPathInfos(RelationshipPathInfosByClass& pathInfosByClass) const
    {
    ECClassCP ridsClass = m_ecDb.Schemas().GetECClass("Bentley_Standard_CustomAttributes", "RelatedItemsDisplaySpecifications");
    if (!ridsClass)
        {
        BeAssert(false);
        return ERROR;
        }

    ECCustomAttributeClassCP ridsCAClass = ridsClass->GetCustomAttributeClassCP();
    if (!ridsCAClass)
        {
        BeAssert(false);
        return ERROR;
        }

    bvector<ECN::ECSchemaCP> allSchemas;
    m_ecDb.Schemas().GetECSchemas(allSchemas, false);
    BeAssert(allSchemas.size() > 0);

    BentleyStatus status = SUCCESS;
    for (ECSchemaCP schema : allSchemas)
        {
        IECInstancePtr ridsCA = schema->GetCustomAttribute(*ridsCAClass);
        if (ridsCA.IsNull())
            continue;

        ECValue ridsSpecValue;
        if (ECObjectsStatus::Success != ridsCA->GetValue(ridsSpecValue, "Specifications"))
            continue;

        ArrayInfo arrayInfo = ridsSpecValue.GetArrayInfo();
        for (int ii = 0; ii < (int) arrayInfo.GetCount(); ii++)
            {
            ridsCA->GetValue(ridsSpecValue, "Specifications", ii);

            IECInstancePtr ridsSpecInstance = ridsSpecValue.GetStruct();
            if (ridsSpecInstance.IsNull())
                continue;

            if (SUCCESS != GatherRelationshipPathInfos(pathInfosByClass, *schema, *ridsSpecInstance))
                {
                status = ERROR;
                continue; // best effort
                }
            }
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                              Ramanujam.Raman                 05 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::RelatedItemsDisplaySpecificationsCache::GatherRelationshipPathInfos(RelationshipPathInfosByClass& pathInfosByClass, ECSchemaCR defaultSchema, IECInstanceCR caSpec) const
    {
    ECValue parentClassECV;
    ECObjectsStatus ecStatus = caSpec.GetValue(parentClassECV, "ParentClass");
    PRECONDITION(ECObjectsStatus::Success == ecStatus && !parentClassECV.IsNull(), ERROR);

    ECClassCP parentClass = ResolveClass(parentClassECV.GetUtf8CP(), defaultSchema);
    PRECONDITION(parentClass != nullptr, ERROR);

    ECValue pathECV;
    ecStatus = caSpec.GetValue(pathECV, "RelationshipPath");
    PRECONDITION(ECObjectsStatus::Success == ecStatus && !pathECV.IsNull(), ERROR);

    Utf8CP path = pathECV.GetUtf8CP();
    PRECONDITION(!Utf8String::IsNullOrEmpty(path), ERROR);

    RelationshipPathInfo& pathInfo = AddEntryToRelationshipPathInfos(pathInfosByClass, *parentClass, path, defaultSchema);
   
    // Create a relationship path for every DerivedClass specified
    ECValue derivedClassesValue;
    ecStatus = caSpec.GetValue(derivedClassesValue, "DerivedClasses");
    if (ecStatus == ECObjectsStatus::Success)
        {
        ArrayInfo arrayInfo = derivedClassesValue.GetArrayInfo();
        for (int ii = 0; ii < (int) arrayInfo.GetCount(); ii++)
            {
            ECValue val;
            caSpec.GetValue(val, "DerivedClasses", ii);
            if (val.IsNull())
                continue;

            Utf8String derivedClassName(val.GetUtf8CP());
            ECClassCP derivedClass = ResolveClass(derivedClassName, defaultSchema);
            if (derivedClass == nullptr)
                continue;

            pathInfo.InsertDerivedClass(*derivedClass);
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                 06 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
JsonReader::RelationshipPathInfo& JsonReader::RelatedItemsDisplaySpecificationsCache::AddEntryToRelationshipPathInfos(RelationshipPathInfosByClass& pathInfosByClass, ECClassCR parentClass, Utf8CP path, ECSchemaCR defaultSchema) const
    {
    auto it = pathInfosByClass.find(&parentClass);
    bvector<JsonReader::RelationshipPathInfo>& pathInfoVector = (it == pathInfosByClass.end()) ? pathInfosByClass[&parentClass] : it->second;

    RelationshipPathInfo pathInfo(path, defaultSchema);
    pathInfoVector.push_back(pathInfo);

    return pathInfoVector.back();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                 06 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
void JsonReader::RelatedItemsDisplaySpecificationsCache::SortRelationshipPaths()
    {
    for (RelationshipPathsByClassId::iterator iter = m_pathsByClass.begin(); iter != m_pathsByClass.end(); iter++)
        {
        bvector<ECRelationshipPath>& infos = iter->second;
        std::sort(infos.begin(), infos.end(), [] (ECRelationshipPath const& path1, ECRelationshipPath const& path2) -> bool
            {
            Utf8String path1Str = path1.ToString();
            Utf8String path2Str = path2.ToString();
            return ::strcmp(path1Str.c_str(), path2Str.c_str()) > 0;
            });
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                 06 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
void JsonReader::RelatedItemsDisplaySpecificationsCache::RemoveDuplicates(RelationshipPathInfosByClass& pathInfosByClass) const
    {
    /*
    * Note: There may be duplicate relationship paths specified starting from parent/root classes. This typically happens in IFC cases where the same paths
    * are specified as starting from the base classes and derived classes.
    *
    * e.g., Consider one IFC hierarchy: Element -> GeometricElement -> GeometricElement3d -> SpatialElement -> PhysicalElement -> PhysicalObject ->
    *       -> IfcRoot -> IfcObjectDefinition -> IfcObject -> IfcProduct -> IfcElement -> IfcBuildingElement -> IfcBuildingElementProxy
    *
    * As specified by the IFC schemas, these classes have duplicate paths: IfcObject, IfcProduct, IfcElement, IfcBuildingElement, IfcBuildingElementProxy
    *
    * We compact the cache to remove these duplicates so that the derived classes *only* contain the paths specific to them. When we later TryGetRelatedPaths() for
    * a specific parent/root from the cache, we get the paths associated with the entire hierarchy of base classes starting with the requested parent/root class.
    *
    */
    for (RelationshipPathInfosByClass::const_iterator baseIter = pathInfosByClass.begin(); baseIter != pathInfosByClass.end(); baseIter++)
        {
        ECClassCP baseClass = baseIter->first;
        bvector<RelationshipPathInfo> const& baseInfos = baseIter->second;

        for (RelationshipPathInfosByClass::iterator derivedIter = pathInfosByClass.begin(); derivedIter != pathInfosByClass.end(); derivedIter++)
            {
            if (baseIter == derivedIter)
                continue;

            ECClassCP derivedClass = derivedIter->first;
            if (!derivedClass->Is(baseClass))
                continue;

            bvector<RelationshipPathInfo>& derivedInfos = derivedIter->second;

            for (bvector<RelationshipPathInfo>::iterator derivedInfoIter = derivedInfos.begin(); derivedInfoIter != derivedInfos.end(); /* no increment */)
                {
                RelationshipPathInfo const& derivedInfo = *derivedInfoIter;
                bvector<RelationshipPathInfo>::const_iterator it = std::find_if(baseInfos.begin(), baseInfos.end(), [&derivedInfo] (RelationshipPathInfo const& baseInfo)
                    {
                    return (0 == BeStringUtilities::StricmpAscii(derivedInfo.m_path.c_str(), baseInfo.m_path.c_str()));
                    });

                if (it != baseInfos.end())
                    derivedInfoIter = derivedInfos.erase(derivedInfoIter);
                else
                    derivedInfoIter++;
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::RelatedItemsDisplaySpecificationsCache::ExtractRelationshipPaths(RelationshipPathInfosByClass const& pathInfosByClass)
    {
    BentleyStatus status = SUCCESS;

    for (RelationshipPathInfosByClass::const_iterator iter = pathInfosByClass.begin(); iter != pathInfosByClass.end(); iter++)
        {
        ECClassCP parentClass = iter->first;

        // Construct "end-to-end" relationship paths : ParentClass + RelationshipPath (specified in schema) + DerivedClass (if specified)
        Utf8PrintfString relationshipPathStr("%s:%s", parentClass->GetSchema().GetName().c_str(), parentClass->GetName().c_str());

        for (RelationshipPathInfo const& pathInfo : iter->second)
            {
            pathInfo.m_derivedClasses;

            Utf8String basePathStr = relationshipPathStr;
            
            basePathStr.append(".");
            basePathStr.append(pathInfo.m_path);

            ECRelationshipPath basePath;
            if (SUCCESS != basePath.InitFromString(basePathStr, m_ecDb.GetClassLocater(), &pathInfo.m_defaultSchema) || !basePath.Validate())
                {
                status = ERROR;
                continue;
                }

            if (pathInfo.m_derivedClasses.empty())
                {
                // If no derived classes are specified, only add the base path
                AddEntryToCache(*parentClass, basePath);
                continue;
                }

            for (ECClassCP derivedClass : pathInfo.m_derivedClasses)
                {
                ECRelationshipPath derivedPath = basePath;
                
                derivedPath.SetEndClass(*derivedClass, ECRelationshipPath::End::Leaf);

                if (!derivedPath.Validate())
                    {
                    status = ERROR;
                    continue;
                    }

                AddEntryToCache(*parentClass, derivedPath);
                }
            }
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                 06 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
void JsonReader::RelatedItemsDisplaySpecificationsCache::AddEntryToCache(ECN::ECClassCR parentClass, ECN::ECRelationshipPath const& path)
    {
    ECClassId parentClassId = parentClass.GetId();

    auto it = m_pathsByClass.find(parentClassId);
    bvector<ECRelationshipPath>& pathVector = (it == m_pathsByClass.end()) ? m_pathsByClass[parentClassId] : it->second;

    pathVector.push_back(path);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                 06 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
void JsonReader::RelatedItemsDisplaySpecificationsCache::DumpCache(ECClassCP ecClass /*=nullptr*/) const
    {
    if (ecClass)
        LOG.tracev("ECRelationshipPath Cache (Filtered By Class %s)", ecClass->GetECSqlName().c_str());
    else
        LOG.tracev("ECRelationshipPath Cache:");

    for (RelationshipPathsByClassId::const_iterator iter = m_pathsByClass.begin(); iter != m_pathsByClass.end(); iter++)
        {
        ECClassId classId = iter->first;
        ECClassCP pathClass = m_ecDb.Schemas().GetECClass(classId);

        if (ecClass != nullptr && !ecClass->Is(pathClass))
            continue;

        LOG.infov("\tClass %s:", pathClass->GetECSqlName());

        bvector<ECRelationshipPath> const& pathVector = iter->second;
        for (ECRelationshipPath const& path : pathVector)
            {
            LOG.infov("\t\t%s", path.ToString().c_str());
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 05 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
bool JsonReader::RelatedItemsDisplaySpecificationsCache::TryGetRelatedPaths(bvector<ECRelationshipPath>& relationshipPathVec, ECClassCR ecClass) const
    {
    for (RelationshipPathsByClassId::const_iterator iter = m_pathsByClass.begin(); iter != m_pathsByClass.end(); iter++)
        {
        ECClassId classId = iter->first;
        ECClassCP pathClass = m_ecDb.Schemas().GetECClass(classId);

        if (!ecClass.Is(pathClass))
            continue;

        bvector<ECRelationshipPath> const& pathVector = iter->second;
        relationshipPathVec.insert(relationshipPathVec.end(), pathVector.begin(), pathVector.end());
        }

    if (LOG.isSeverityEnabled(NativeLogging::LOG_TRACE))
        DumpCache(&ecClass);

    return !relationshipPathVec.empty();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
