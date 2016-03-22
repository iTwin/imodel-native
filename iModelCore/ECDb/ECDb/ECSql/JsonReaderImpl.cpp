/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/JsonReaderImpl.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "JsonReaderImpl.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
//@bsimethod                                    Ramanujam.Raman                 9 / 2013
//+---------------+---------------+---------------+---------------+---------------+------
JsonReader::Impl::Impl(ECDbCR ecdb, ECN::ECClassId ecClassId)
    : m_ecDb(ecdb), m_statementCache(50, "JsonReader ECSqlStatement Cache")
    {
    m_ecClass = m_ecDb.Schemas().GetECClass(ecClassId);
    BeAssert(m_ecClass != nullptr && "Could not retrieve class with specified id");
    m_isValid = m_ecClass != nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 09 / 2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::Impl::Read(JsonValueR jsonInstances, JsonValueR jsonDisplayInfo, ECInstanceId ecInstanceId, JsonECSqlSelectAdapter::FormatOptions formatOptions)
    {
    jsonInstances = Json::arrayValue;
    jsonDisplayInfo = Json::objectValue;

    if (!IsValid ())
        return ERROR;

    // Add any instances of the specified class itself
    ECRelationshipPath trivialPathToClass;
    auto status = GetTrivialPathToSelf (trivialPathToClass, *m_ecClass);
    if (status == SUCCESS)
        status = AddInstancesFromSpecifiedClassPath (jsonInstances, jsonDisplayInfo, trivialPathToClass, ecInstanceId, formatOptions);

    //WIP_FOR_RAMAN: Method AddInstancesFromRelatedItems does not exist
    // Add any related instances according to the "RelatedItemsDisplaySpecification" custom attribute
    //if (status == SUCCESS)
        //status = AddInstancesFromRelatedItems (jsonInstances, jsonDisplayInfo, *m_ecClass, trivialPathToClass, ecInstanceId, formatOptions);

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
BentleyStatus JsonReader::Impl::ReadInstance(Json::Value& jsonValue, ECInstanceId ecInstanceId, JsonECSqlSelectAdapter::FormatOptions formatOptions)
    {
    if (!IsValid ())
        return ERROR;

    ECRelationshipPath emptyPath;
    if (SUCCESS != GetTrivialPathToSelf (emptyPath, *m_ecClass))
        return ERROR;

    CachedECSqlStatementPtr statement = nullptr;
    if (SUCCESS != PrepareECSql (statement, emptyPath, ecInstanceId, false/*selectInstanceKeyOnly*/, false/*isPolymorphic*/))
        return ERROR;

    DbResult stepStatus = statement->Step ();
    POSTCONDITION (stepStatus == BE_SQLITE_ROW && "Instance not found", ERROR);

    JsonECSqlSelectAdapter jsonAdapter (*statement, formatOptions);
    if (!jsonAdapter.GetRowInstance (jsonValue, m_ecClass->GetId ()))
        return ERROR;

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::Impl::PrepareECSql (CachedECSqlStatementPtr& statement, Utf8StringCR ecSql) const
    {
    statement = m_statementCache.GetPreparedStatement (m_ecDb, ecSql.c_str ());
    POSTCONDITION (statement != nullptr, ERROR);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// Prepare ECSQL to retrieve the instance specified by the path
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::Impl::PrepareECSql(CachedECSqlStatementPtr& statement, ECRelationshipPath const& pathFromRelatedClass, ECInstanceId ecInstanceId, bool selectInstanceKeyOnly, bool isPolymorphic) const
    {
    Utf8String fromClause, joinClause;
    ECRelationshipPath::GeneratedEndInfo rootInfo, leafInfo;
    if (SUCCESS != pathFromRelatedClass.GenerateECSql (fromClause, joinClause, rootInfo, leafInfo, isPolymorphic))
        return ERROR;
    BeAssert (m_ecClass == pathFromRelatedClass.GetEndClass (ECRelationshipPath::End::Leaf));
    
    Utf8String selectClause;
    if (selectInstanceKeyOnly)
        selectClause.Sprintf ("SELECT %s, %s", rootInfo.GetClassIdExpression().c_str(), rootInfo.GetInstanceIdExpression().c_str());
    else
        selectClause.Sprintf ("SELECT [%s].*", rootInfo.GetAlias().c_str());

    Utf8PrintfString ecSqlRelated ("%s %s %s WHERE %s.ECInstanceId = ?", 
        selectClause.c_str(), fromClause.c_str(), joinClause.c_str(), leafInfo.GetAlias().c_str ());
    if (SUCCESS != PrepareECSql (statement, ecSqlRelated))
        return ERROR;

    ECSqlStatus bindStatus = statement->BindId (1, ecInstanceId);
    POSTCONDITION (bindStatus.IsSuccess(), ERROR);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::Impl::GetRelatedInstanceKeys(ECInstanceKeyMultiMap& instanceKeys, ECRelationshipPath const& pathFromRelatedClass, ECInstanceId ecInstanceId)
    {
    instanceKeys.clear();

    CachedECSqlStatementPtr statement = nullptr;
    if (SUCCESS != PrepareECSql(statement, pathFromRelatedClass, ecInstanceId, true /* selectInstanceKeyOnly*/, false/*isPolymorphic*/))
        return ERROR;

    while (BE_SQLITE_ROW == statement->Step())
        {
        ECInstanceKey instanceKey(statement->GetValueId<ECClassId>(0), statement->GetValueId<ECInstanceId>(1));
        if (!instanceKey.IsValid())
            continue; // TODO: In the existing DgnDb, you can have ECClassId, ECInstanceId set to 0,0

        ECInstanceKeyMultiMapPair instanceEntry(instanceKey.GetECClassId(), instanceKey.GetECInstanceId());
        instanceKeys.insert(instanceEntry);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
void JsonReader::Impl::SetRelationshipPath(JsonValueR addClasses, Utf8StringCR pathFromRelatedClassStr)
    {
    for (Utf8StringCR key : addClasses.getMemberNames())
        addClasses[key]["RelationshipPath"] = pathFromRelatedClassStr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
void JsonReader::Impl::SetInstanceIndex(JsonValueR addCategories, int currentInstanceIndex)
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
void JsonReader::Impl::AddClasses(JsonValueR allClasses, JsonValueR addClasses)
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
void JsonReader::Impl::AddCategories(JsonValueR allCategories, JsonValueR addCategories, int currentInstanceIndex)
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
void JsonReader::Impl::AddInstances(JsonValueR allInstances, JsonValueR addInstances, int currentInstanceIndex)
    {
    // Consolidate the categories with previous instances
    for (int ii = 0; ii < (int) addInstances.size(); ii++)
        allInstances[currentInstanceIndex + ii] = addInstances[ii];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::Impl::AddInstancesFromPreparedStatement(JsonValueR allInstances, JsonValueR allDisplayInfo, ECSqlStatement& statement,
                                                                  JsonECSqlSelectAdapter::FormatOptions const& formatOptions, Utf8StringCR pathFromRelatedClassStr)
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
BentleyStatus JsonReader::Impl::AddInstancesFromRelatedItems(JsonValueR allInstances,JsonValueR allDisplayInfo,ECClassCR parentClass,
ECRelationshipPath const& pathFromParent,ECInstanceId ecInstanceId,JsonECSqlSelectAdapter::FormatOptions const& formatOptions)
    {
    bvector<ECRelationshipPath> appendPathsFromParent;
    ECRelatedItemsDisplaySpecCacheAppData* appData = ECRelatedItemsDisplaySpecCacheAppData::Get(m_ecDb);
    if (appData == nullptr)
        return ERROR;

    if (!appData->GetCache().TryGetRelatedPaths(appendPathsFromParent, parentClass))
        return SUCCESS;

    ECRelationshipPath pathToParent;
    pathFromParent.Reverse(pathToParent);

    for (ECRelationshipPath const& appendPath : appendPathsFromParent)
        {
        ECRelationshipPath pathToRelatedClass = pathToParent;
        pathToRelatedClass.Combine(appendPath);

        ECRelationshipPath pathFromRelatedClass;
        pathToRelatedClass.Reverse(pathFromRelatedClass);

        if (SUCCESS != AddInstancesFromSpecifiedClassPath(allInstances, allDisplayInfo, pathFromRelatedClass, ecInstanceId, formatOptions))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::Impl::AddInstancesFromSpecifiedClassPath(JsonValueR allInstances, JsonValueR allDisplayInfo, ECRelationshipPath const& pathFromRelatedClass, ECInstanceId ecInstanceId, JsonECSqlSelectAdapter::FormatOptions const& formatOptions)
    {
    CachedECSqlStatementPtr statement = nullptr;
    if (SUCCESS != PrepareECSql(statement, pathFromRelatedClass, ecInstanceId, false /* selectInstanceKeyOnly*/, false/*isPolymorphic*/))
        return ERROR;

    Utf8String pathFromRelatedClassStr = pathFromRelatedClass.ToString();
    return AddInstancesFromPreparedStatement(allInstances, allDisplayInfo, *statement, formatOptions, pathFromRelatedClassStr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 05 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::Impl::GetTrivialPathToSelf (ECRelationshipPath& emptyPath, ECClassCR ecClass)
    {
    auto status = emptyPath.InitFromString (ecClass.GetFullName(), m_ecDb.GetClassLocater(), &(ecClass.GetSchema ()));
    POSTCONDITION (status == SUCCESS, ERROR);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 05 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
JsonReader::Impl::ECRelatedItemsDisplaySpecCacheAppData* JsonReader::Impl::ECRelatedItemsDisplaySpecCacheAppData::Get(ECDbCR ecdb)
    {
    //WIP_FOR_RAMAN It is dangerous to keep this cache together with the ECDb with pointers to ECSchema entities, because whenever
    //ClearCache is called (or a schema import is done), the pointers are invalid.
    ECRelatedItemsDisplaySpecCacheAppData* appData = reinterpret_cast <ECRelatedItemsDisplaySpecCacheAppData*> (ecdb.FindAppData(ECRelatedItemsDisplaySpecCacheAppData::GetKey()));

    if (nullptr == appData)
        {
        ECSchemaList allSchemas;
        if (SUCCESS != ecdb.Schemas().GetECSchemas(allSchemas, false))
            {
            BeAssert(false);
            return nullptr;
            }

        appData = new ECRelatedItemsDisplaySpecCacheAppData(ecdb);
        if (SUCCESS != appData->m_cache.Initialize(allSchemas, ecdb.GetClassLocater()))
            {
            delete appData;
            return nullptr;
            }

        ecdb.AddAppData(ECRelatedItemsDisplaySpecCacheAppData::GetKey(), appData);
        }

    return appData;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE