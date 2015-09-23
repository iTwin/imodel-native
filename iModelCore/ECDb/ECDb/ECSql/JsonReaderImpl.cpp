/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/JsonReaderImpl.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "JsonReaderImpl.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
//@bsimethod                                    Ramanujam.Raman                 9 / 2013
//+---------------+---------------+---------------+---------------+---------------+------
JsonReader::Impl::Impl (ECDbCR ecdb, ECN::ECClassId ecClassId)
: m_ecDb (ecdb), m_statementCache (50, "JsonReader ECSqlStatement Cache")
    {
    m_ecClass = m_ecDb.Schemas ().GetECClass (ecClassId);
    BeAssert (m_ecClass != nullptr && "Could not retrieve class with specified id");
    m_isValid = m_ecClass != nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 09 / 2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::Impl::Read
(
JsonValueR jsonInstances,
JsonValueR jsonDisplayInfo,
ECInstanceId const& ecInstanceId,
JsonECSqlSelectAdapter::FormatOptions formatOptions
)
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

    // Add any related instances according to the "RelatedItemsDisplaySpecification" custom attribute
    if (status == SUCCESS)
        status = AddInstancesFromRelatedItems (jsonInstances, jsonDisplayInfo, *m_ecClass, trivialPathToClass, ecInstanceId, formatOptions);

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
BentleyStatus JsonReader::Impl::ReadInstance
(
Json::Value& jsonValue,
ECInstanceId const& ecInstanceId,
JsonECSqlSelectAdapter::FormatOptions formatOptions
)
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
BentleyStatus JsonReader::Impl::PrepareECSql (CachedECSqlStatementPtr& statement, Utf8StringCR ecSql)
    {
    statement = m_statementCache.GetPreparedStatement (m_ecDb, ecSql.c_str ());
    POSTCONDITION (statement != nullptr, ERROR);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// Prepare ECSQL to retrieve the instance specified by the path
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::Impl::PrepareECSql 
(
CachedECSqlStatementPtr& statement,
const ECRelationshipPath& pathFromRelatedClass, 
const ECInstanceId& ecInstanceId, 
bool selectInstanceKeyOnly,
bool isPolymorphic
)
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
BentleyStatus JsonReader::Impl::GetRelatedInstanceKeys (ECInstanceKeyMultiMap& instanceKeys, const ECRelationshipPath& pathFromRelatedClass, const ECInstanceId& ecInstanceId)
    {
    instanceKeys.clear ();

    CachedECSqlStatementPtr statement = nullptr;
    if (SUCCESS != PrepareECSql (statement, pathFromRelatedClass, ecInstanceId, true /* selectInstanceKeyOnly*/,false/*isPolymorphic*/))
        return ERROR;

    while (BE_SQLITE_ROW == statement->Step ())
        {
        ECInstanceKey instanceKey (statement->GetValueInt64 (0), statement->GetValueId<ECInstanceId> (1));
        if (!instanceKey.IsValid ())
            continue; // TODO: In the existing DgnDb, you can have ECClassId, ECInstanceId set to 0,0

        ECInstanceKeyMultiMapPair instanceEntry (instanceKey.GetECClassId (), instanceKey.GetECInstanceId ());
        instanceKeys.insert (instanceEntry);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
void JsonReader::Impl::SetRelationshipPath (JsonValueR addClasses, Utf8StringCR pathFromRelatedClassStr)
    {
    for (Utf8StringCR key : addClasses.getMemberNames ())
        addClasses[key]["RelationshipPath"] = pathFromRelatedClassStr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
void JsonReader::Impl::SetInstanceIndex (JsonValueR addCategories, int currentInstanceIndex)
    {
    for (int categoryIndex = 0; categoryIndex < (int) addCategories.size (); categoryIndex++)
        {
        Json::Value& addCategory = addCategories[categoryIndex];
        if (!addCategory.isMember ("Properties"))
            continue; // empty category
        Json::Value& addProperties = addCategory["Properties"];
        for (int propertyIndex = 0; propertyIndex < (int) addProperties.size (); propertyIndex++)
            {
            Json::Value& addProperty = addProperties[propertyIndex];
            addProperty["InstanceIndex"] = currentInstanceIndex;
            if (addProperty.isMember ("Categories"))
                SetInstanceIndex (addProperty["Categories"], currentInstanceIndex);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
void JsonReader::Impl::AddClasses (JsonValueR allClasses, JsonValueR addClasses)
    {
    for (Utf8StringCR key : addClasses.getMemberNames ())
        {
        if (allClasses.isMember (key))
            continue;
        allClasses[key] = addClasses[key];
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
void JsonReader::Impl::AddCategories (JsonValueR allCategories, JsonValueR addCategories, int currentInstanceIndex)
    {
    // Recursively setup the instanceIndex in all the properties within addCategories
    SetInstanceIndex (addCategories, currentInstanceIndex);

    // Consolidate the categories
    for (int categoryIndex = 0; categoryIndex < (int) addCategories.size (); categoryIndex++)
        {
        // Consolidate the categories
        Json::Value& addCategory = addCategories[categoryIndex];
        if (!addCategory.isMember ("Properties"))
            continue; // empty category

        Utf8String categoryName = addCategory["CategoryName"].asString ();
        int existingCategoryIndex;
        for (existingCategoryIndex = 0; existingCategoryIndex < (int) allCategories.size (); existingCategoryIndex++)
            {
            if (allCategories[existingCategoryIndex]["CategoryName"].asString () == categoryName)
                break;
            }
        if (existingCategoryIndex >= (int) allCategories.size ()) // Category not found
            {
            allCategories.append (addCategory);
            continue;
            }

        // Category found, consolidate the properties
        Json::Value& allProperties = allCategories[existingCategoryIndex]["Properties"];
        Json::Value& addProperties = addCategory["Properties"];
        for (int propertyIndex = 0; propertyIndex < (int) addProperties.size (); propertyIndex++)
            allProperties.append (addProperties[propertyIndex]);

        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
void JsonReader::Impl::AddInstances (JsonValueR allInstances, JsonValueR addInstances, int currentInstanceIndex)
    {
    // Consolidate the categories with previous instances
    for (int ii = 0; ii < (int) addInstances.size (); ii++)
        allInstances[currentInstanceIndex + ii] = addInstances[ii];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::Impl::AddInstancesFromPreparedStatement
(
JsonValueR allInstances,
JsonValueR allDisplayInfo,
ECSqlStatement& statement,
const JsonECSqlSelectAdapter::FormatOptions& formatOptions,
Utf8StringCR pathFromRelatedClassStr
)
    {
    statement.Reset ();
    JsonECSqlSelectAdapter jsonAdapter (statement, formatOptions);

    int currentInstanceIndex = (int) allInstances.size ();

    while (BE_SQLITE_ROW == statement.Step ())
        {
        if (currentInstanceIndex == 0)
            {
            // Setup instance the first time
            if (!jsonAdapter.GetRow (allInstances))
                return ERROR;

            // Setup display info the first time
            jsonAdapter.GetRowDisplayInfo (allDisplayInfo);
            SetRelationshipPath (allDisplayInfo["Classes"], pathFromRelatedClassStr);
            }
        else
            {
            /*
            * Consolidate instances
            */
            Json::Value addInstances;
            if (!jsonAdapter.GetRow (addInstances))
                return ERROR;

            AddInstances (allInstances, addInstances, currentInstanceIndex);

            /*
            * Consolidate display info
            */
            Json::Value addDisplayInfo;
            jsonAdapter.GetRowDisplayInfo (addDisplayInfo); // TODO: Setup move constructors in Json::Value
            SetRelationshipPath (addDisplayInfo["Classes"], pathFromRelatedClassStr);

            // Consolidate the categories
            if (!allDisplayInfo.isMember ("Categories"))
                allDisplayInfo["Categories"] = Json::Value (Json::arrayValue);
            AddCategories (allDisplayInfo["Categories"], addDisplayInfo["Categories"], currentInstanceIndex);

            // Consolidate the classes
            AddClasses (allDisplayInfo["Classes"], addDisplayInfo["Classes"]);
            }

        currentInstanceIndex++;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::Impl::AddInstancesFromAnyClassPath
(
JsonValueR allInstances,
JsonValueR allDisplayInfo,
const ECRelationshipPath& pathFromAnyClass,
const ECInstanceId& ecInstanceId,
const JsonECSqlSelectAdapter::FormatOptions& formatOptions
)
    {
    BeAssert (pathFromAnyClass.IsAnyClassAtEnd (ECRelationshipPath::End::Root));

    ECInstanceKeyMultiMap instanceKeys;
    if (SUCCESS != GetRelatedInstanceKeys (instanceKeys, pathFromAnyClass, ecInstanceId))
        return ERROR;

    // See note on "hack" below
    ECClassCP dgnElementClass = m_ecDb.Schemas ().GetECClass ("dgn", "Element");
    bool isRootClassDgnElement = (m_ecClass == dgnElementClass);

    ECInstanceKeyMultiMapConstIterator instanceIdIter;
    for (ECInstanceKeyMultiMapConstIterator classIdIter = instanceKeys.begin (); classIdIter != instanceKeys.end (); classIdIter = instanceIdIter)
        {
        ECClassId ecRelatedClassId = classIdIter->first;
        ECClassCP ecRelatedClass = m_ecDb.Schemas ().GetECClass (ecRelatedClassId);
        POSTCONDITION (ecRelatedClass != nullptr, ERROR);

        ECSqlSelectBuilder builder;
        builder.From (*ecRelatedClass).SelectAll ().Where ("ECInstanceId = ?");
        CachedECSqlStatementPtr statement = nullptr;
        if (SUCCESS != PrepareECSql (statement, builder.ToString ()))
            return ERROR;

        // Determine the fully specified path to the class found - this is recorded in the output JSON. 
        ECRelationshipPath pathFromRelatedClass = pathFromAnyClass;
        pathFromRelatedClass.ReplaceAnyClassAtEnd (*ecRelatedClass, ECRelationshipPath::End::Root);
        Utf8String pathFromRelatedClassStr = pathFromRelatedClass.ToString ();

        bpair<ECInstanceKeyMultiMapConstIterator, ECInstanceKeyMultiMapConstIterator> keyRange = instanceKeys.equal_range (ecRelatedClassId);
        for (instanceIdIter = keyRange.first; instanceIdIter != keyRange.second; instanceIdIter++)
            {
            statement->Reset ();
            ECInstanceId instanceId = instanceIdIter->second;
            statement->BindId (1, instanceId);
            if (SUCCESS != AddInstancesFromPreparedStatement (allInstances, allDisplayInfo, *statement, formatOptions, pathFromRelatedClassStr))
                return ERROR;
            }

        statement = nullptr;

        // Traverse any related items display specifications on the resolved classes
        // 
        // Note: A hard coded check for dgn:Element here looks positively ugly, but seemed like the best of options. 
        // We are intentionally making a very special exception for ElementHasPrimaryInstance relationship with an AnyClass constraint at the 
        // end here. 
        //
        // In the typical case we will not look further at (indirectly) related items that are resolved after traversing these AnyClass relationships, 
        // just like we won't look further when it's *not* a AnyClass relationship. 
        // 
        // The user may not want to see the entire hierarchy of items. If they do, they can specify all the RelatedItemsDisplaySPecifications 
        // they want exhaustively. 
        // 
        // Also, nobody else should be using "AnyClass" in their relationship path. ONLY we can!!! 
        // That's our justification and we are sticking with it!!!
        if (isRootClassDgnElement)
            {
            if (SUCCESS != AddInstancesFromRelatedItems (allInstances, allDisplayInfo, *ecRelatedClass, pathFromRelatedClass, ecInstanceId, formatOptions))
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 05 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::Impl::AddInstancesFromRelatedItems
(
JsonValueR allInstances,
JsonValueR allDisplayInfo,
ECClassCR parentClass,
ECRelationshipPathCR pathFromParent,
const ECInstanceId& ecInstanceId,
const JsonECSqlSelectAdapter::FormatOptions& formatOptions
)
    {
    ECRelationshipPathVector appendPathsFromParent;
    if (!ECRelatedItemsDisplaySpecificationsCache::GetCache(m_ecDb)->TryGetRelatedPathsFromClass (appendPathsFromParent, parentClass))
        return SUCCESS;

    ECRelationshipPath pathToParent;
    pathFromParent.Reverse (pathToParent);

    for (ECRelationshipPathCR appendPath : appendPathsFromParent)
        {
        ECRelationshipPath pathToRelatedClass = pathToParent;
        pathToRelatedClass.Combine (appendPath);

        ECRelationshipPath pathFromRelatedClass;
        pathToRelatedClass.Reverse (pathFromRelatedClass);

        BentleyStatus status = SUCCESS;
        if (pathFromRelatedClass.IsAnyClassAtEnd (ECRelationshipPath::End::Root))
            status = AddInstancesFromAnyClassPath (allInstances, allDisplayInfo, pathFromRelatedClass, ecInstanceId, formatOptions);
        else
            status = AddInstancesFromSpecifiedClassPath (allInstances, allDisplayInfo, pathFromRelatedClass, ecInstanceId, formatOptions);

        if (SUCCESS != status)
            return status;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::Impl::AddInstancesFromSpecifiedClassPath
(
JsonValueR allInstances,
JsonValueR allDisplayInfo,
const ECRelationshipPath& pathFromRelatedClass,
const ECInstanceId& ecInstanceId,
const JsonECSqlSelectAdapter::FormatOptions& formatOptions
)
    {
    BeAssert (!pathFromRelatedClass.IsAnyClassAtEnd (ECRelationshipPath::End::Root));

    CachedECSqlStatementPtr statement = nullptr;
    if (SUCCESS != PrepareECSql (statement, pathFromRelatedClass, ecInstanceId, false /* selectInstanceKeyOnly*/,false/*isPolymorphic*/))
        return ERROR;

    Utf8String pathFromRelatedClassStr = pathFromRelatedClass.ToString ();
    return AddInstancesFromPreparedStatement (allInstances, allDisplayInfo, *statement, formatOptions, pathFromRelatedClassStr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 05 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::Impl::GetTrivialPathToSelf (ECRelationshipPathR emptyPath, ECClassCR ecClass)
    {
    Utf8String schemaName (ecClass.GetSchema ().GetName ().c_str ());
    Utf8String className (ecClass.GetName ().c_str ());
    Utf8String qualifiedClassName = schemaName + ":" + className;

    auto status = emptyPath.InitFromString (qualifiedClassName, m_ecDb, &(ecClass.GetSchema ()));
    POSTCONDITION (status == SUCCESS, ERROR);

    return SUCCESS;
    }

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECClassHelper::GetUnqualifiedName (Utf8StringCR qualifiedClassName, Utf8Char delimiter)
    {
    return qualifiedClassName.substr (qualifiedClassName.find (delimiter) + 1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
void ECClassHelper::ParseQualifiedName (Utf8StringR schemaPrefixOrName, Utf8StringR className, Utf8StringCR qualifiedClassName, Utf8Char delimiter)
    {
    // [SchemaPrefix[.:]]<ClassName>
    className = qualifiedClassName;
    size_t splitIndex;
    if (Utf8String::npos != (splitIndex = className.find (delimiter)))
        {
        schemaPrefixOrName = className.substr (0, splitIndex);
        className = className.substr (splitIndex + 1);
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Ramanujam.Raman                 01 / 2014
// @param schemaName [in] Can be nullptr, in which case the class is resolved in the default schema (assuming that's supplied).
//+---------------+---------------+---------------+---------------+---------------+------
ECClassCP ECClassHelper::ResolveClass (Utf8CP schemaName, Utf8StringCR className, ECDbCR ecDb, ECSchemaCP defaultSchema)
    {
    ECClassCP ecClass = nullptr;
    if (schemaName != nullptr)
        ecClass = ecDb.Schemas ().GetECClass (schemaName, className.c_str ());
    else
        {
        PRECONDITION (defaultSchema != nullptr && "Cannot resolve class without a specified namespace. or a default schema", nullptr);
        Utf8String defaultSchemaName (defaultSchema->GetName ().c_str ());
        ecClass = ecDb.Schemas ().GetECClass (defaultSchemaName.c_str (), className.c_str ());
        }

    POSTCONDITION (ecClass != nullptr, nullptr);
    return ecClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
ECClassCP ECClassHelper::ResolveClass (Utf8StringCR possiblyQualifiedClassName, ECDbCR ecDb, ECSchemaCP defaultSchema)
    {
    Utf8String schemaName, className;
    ECClassHelper::ParseQualifiedECObjectsName (schemaName, className, possiblyQualifiedClassName);

    return ResolveClass ((schemaName.empty ()) ? nullptr : schemaName.c_str (), className, ecDb, defaultSchema);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
bool ECClassHelper::IsAnyClass (ECClassCR ecClass)
    {
    return ClassMap::IsAnyClass (ecClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECClassHelper::GetName (ECClassCR ecClass)
    {
    Utf8String className (ecClass.GetName ().c_str ());
    return className;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECClassHelper::GetQualifiedECObjectsName (ECClassCR ecClass)
    {
    Utf8PrintfString qualifiedName ("%s:%s", Utf8String (ecClass.GetSchema ().GetName ().c_str ()).c_str (), Utf8String (ecClass.GetName ().c_str ()).c_str ());
    return qualifiedName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECClassHelper::GetQualifiedECSqlName (ECClassCR ecClass)
    {
    Utf8PrintfString qualifiedName ("[%s].[%s]", Utf8String (ecClass.GetSchema ().GetName ().c_str ()).c_str (), Utf8String (ecClass.GetName ().c_str ()).c_str ());
    return qualifiedName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
void ECClassHelper::ParseQualifiedECObjectsName (Utf8StringR schemaPrefixOrName, Utf8StringR className, Utf8StringCR qualifiedClassName)
    {
    ParseQualifiedName (schemaPrefixOrName, className, qualifiedClassName, ':');
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
void ECClassHelper::ParseQualifiedECSqlName (Utf8StringR schemaPrefixOrName, Utf8StringR className, Utf8StringCR qualifiedClassName)
    {
    ParseQualifiedName (schemaPrefixOrName, className, qualifiedClassName, '.');
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECClassHelper::GetUnqualifiedECObjectsName (Utf8StringCR qualifiedClassName)
    {
    return GetUnqualifiedName (qualifiedClassName, ':');
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECClassHelper::GetUnqualifiedECSqlName (Utf8StringCR qualifiedClassName)
    {
    return GetUnqualifiedName (qualifiedClassName, '.');
    }

END_BENTLEY_SQLITE_EC_NAMESPACE