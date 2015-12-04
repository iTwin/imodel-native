/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Util/ECDbAdapter.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Util/ECDbAdapter.h>
#include <WebServices/Cache/Util/ECDbHelper.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbAdapter::ECDbAdapter(ObservableECDb& ecDb) :
m_ecDb(&ecDb),
m_inserters(ecDb),
m_statementCache(ecDb)
    {
    m_ecDb->RegisterSchemaChangeListener(this);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbAdapter::~ECDbAdapter()
    {
    m_ecDb->UnRegisterSchemaChangeListener(this);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbAdapter::OnSchemaChanged()
    {
    m_inserters = ECSqlAdapterCache<ECInstanceInserter>(*m_ecDb);
    m_findRelationshipClassesStatement = nullptr;
    m_finder = nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ObservableECDb& ECDbAdapter::GetECDb()
    {
    return *m_ecDb;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceFinder& ECDbAdapter::GetECInstanceFinder()
    {
    if (nullptr == m_finder)
        {
        m_finder = std::make_shared<ECInstanceFinder>(*m_ecDb);
        }
    return *m_finder;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECDbAdapter::GetECSchema(Utf8StringCR schemaName)
    {
    return m_ecDb->Schemas().GetECSchema(schemaName.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbAdapter::HasECSchema(Utf8StringCR schemaName)
    {
    return nullptr != GetECSchema(schemaName);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECDbAdapter::GetECClass(Utf8StringCR schemaName, Utf8StringCR className)
    {
    return m_ecDb->Schemas().GetECClass(schemaName.c_str(), className.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECDbAdapter::GetECClass(Utf8StringCR classKey)
    {
    Utf8String className;
    Utf8String schemaName;

    ECDbHelper::ParseECClassKey(classKey, schemaName, className);

    return GetECClass(schemaName, className);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECDbAdapter::GetECClass(ECClassId classId)
    {
    return m_ecDb->Schemas().GetECClass(classId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECDbAdapter::GetECClass(ECInstanceKeyCR instanceKey)
    {
    return GetECClass(instanceKey.GetECClassId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECDbAdapter::GetECClass(ObjectIdCR objectId)
    {
    return GetECClass(objectId.schemaName, objectId.className);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECClassCP> ECDbAdapter::GetECClasses(const ECInstanceKeyMultiMap& instanceMultiMap)
    {
    bset<ECClassCP> ecClasses;
    for (auto& pair : instanceMultiMap)
        {
        ECClassId ecClassId = pair.first;
        ECClassCP ecClass = GetECClass(ecClassId);
        BeAssert(ecClass != nullptr);
        ecClasses.insert(ecClass);
        }
    return bvector<ECClassCP>(ecClasses.begin(), ecClasses.end());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP ECDbAdapter::GetECRelationshipClass(Utf8StringCR classKey)
    {
    ECClassCP ecClass = GetECClass(classKey);
    if (nullptr == ecClass)
        {
        return nullptr;
        }
    return ecClass->GetRelationshipClassCP();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP ECDbAdapter::GetECRelationshipClass(ECClassId classId)
    {
    ECClassCP ecClass = GetECClass(classId);
    if (nullptr == ecClass)
        {
        return nullptr;
        }
    return ecClass->GetRelationshipClassCP();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP ECDbAdapter::GetECRelationshipClass(Utf8StringCR schemaName, Utf8StringCR className)
    {
    ECClassCP ecClass = GetECClass(schemaName, className);
    if (nullptr == ecClass)
        {
        return nullptr;
        }
    return ecClass->GetRelationshipClassCP();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP ECDbAdapter::GetECRelationshipClass(ECInstanceKeyCR instanceKey)
    {
    return GetECRelationshipClass(instanceKey.GetECClassId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP ECDbAdapter::GetECRelationshipClass(ObjectIdCR objectId)
    {
    ECClassCP ecClass = GetECClass(objectId);
    if (nullptr == ecClass)
        {
        return nullptr;
        }
    return ecClass->GetRelationshipClassCP();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbAdapter::DoesConstraintSupportECClass(ECRelationshipConstraintCR constraint, ECClassCR ecClass, bool allowPolymorphic)
    {
    for (auto constraintClass : constraint.GetClasses())
        {
        if ((!allowPolymorphic || !constraint.GetIsPolymorphic()) && &ecClass == constraintClass)
            {
            return true;
            }

        if (allowPolymorphic && constraint.GetIsPolymorphic() && ecClass.Is(constraintClass))
            {
            return true;
            }
        }
    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mark.Uvari      10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
int ECDbAdapter::FindDistanceFromBaseClass(ECClassCP ecClass, ECClassCP targetClass, int dist)
    {
    if (ecClass->GetName().Equals(targetClass->GetName()))
        {
        return dist; //Match found
        }
    else if (!ecClass->HasBaseClasses())
        {
        return -1; //Error: No match found
        }
    else
        {
        return FindDistanceFromBaseClass((*ecClass->GetBaseClasses().begin()), targetClass, ++dist);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECRelationshipClassCP> ECDbAdapter::FindRelationshipClasses(ECClassId sourceClassId, ECClassId targetClassId)
    {
    // Doing ECDb query is 3-6 times faster than loading schema to memory. However, schema loading is done once.
    // If this method needs to be called multiple times, then its faster to use schema approach.

    bvector<ECRelationshipClassCP> classes;

    if (nullptr == m_findRelationshipClassesStatement)
        {
        // SQL supplied by Affan Khan
        Utf8CP sql = R"(WITH RECURSIVE
           RelationshipConstraintClasses (ECClassId, ECRelationshipEnd, IsPolymorphic, RelationECClassId, NestingLevel) AS
           (
           SELECT  RC.ECClassId, RCC.ECRelationshipEnd, RC.IsPolymorphic, RCC.RelationECClassId, 0
               FROM ec_RelationshipConstraint RC
                   INNER JOIN ec_RelationshipConstraintClass RCC ON RC.ECClassId = RCC.ECClassId  AND RC.[ECRelationshipEnd] = RCC.[ECRelationshipEnd]
           UNION
           SELECT RCC.ECClassId, RCC.ECRelationshipEnd, RCC.IsPolymorphic, BC.ECClassId, NestingLevel + 1
               FROM RelationshipConstraintClasses RCC
                   INNER JOIN ec_BaseClass BC ON BC.BaseECClassId = RCC.RelationECClassId
               WHERE RCC.IsPolymorphic = 1
               ORDER BY 2 DESC
           )
        SELECT SRC.ECClassId
           FROM RelationshipConstraintClasses SRC
           INNER JOIN   RelationshipConstraintClasses TRG ON SRC.ECClassId = TRG.ECClassId
                WHERE SRC.ECRelationshipEnd = 0 AND SRC.RelationECClassId = ?
                      AND TRG.ECRelationshipEnd = 1 AND TRG.RelationECClassId = ? )";

        m_findRelationshipClassesStatement = std::make_shared<Statement>();

        BeSQLite::DbResult status;
        if (BeSQLite::DbResult::BE_SQLITE_OK != (status = m_findRelationshipClassesStatement->Prepare(*m_ecDb, sql)))
            {
            BeAssert(false);
            return classes;
            }
        }
    else
        {
        m_findRelationshipClassesStatement->Reset();
        m_findRelationshipClassesStatement->ClearBindings();
        }

    m_findRelationshipClassesStatement->BindInt64(1, sourceClassId);
    m_findRelationshipClassesStatement->BindInt64(2, targetClassId);

    BeSQLite::DbResult status;
    while (BeSQLite::DbResult::BE_SQLITE_ROW == (status = m_findRelationshipClassesStatement->Step()))
        {
        classes.push_back(GetECRelationshipClass(m_findRelationshipClassesStatement->GetValueInt64(0)));
        }

    return classes;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mark.Uvari      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECRelationshipClassCP> ECDbAdapter::FindRelationshipClassesWithSource(ECClassId endClassId, Utf8String schemaName)
    {
    bvector<ECRelationshipClassCP> classes;

    ECClassCP endClass = GetECClass(endClassId);
    ECSchemaCP schema = GetECSchema(schemaName);

    if (schema == nullptr || endClass == nullptr)
        {
        return classes;
        }

    for (ECClassCP ecClass : schema->GetClasses())
        {
        ECRelationshipClassCP relClass = ecClass->GetRelationshipClassCP();
        if (nullptr == relClass)
            continue;

        if (DoesConstraintSupportECClass(relClass->GetSource(), *endClass, true) ||
            DoesConstraintSupportECClass(relClass->GetTarget(), *endClass, true))
            {
            classes.push_back(relClass);
            }
        }

    return classes;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mark.Uvari      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECRelationshipClassCP> ECDbAdapter::FindRelationshipClassesInSchema(ECClassId sourceClassId, ECClassId targetClassId, Utf8String schemaName)
    {
    bvector<ECRelationshipClassCP> classes;

    ECClassCP sourceClass = GetECClass(sourceClassId);
    ECClassCP targetClass = GetECClass(targetClassId);
    ECSchemaCP schema = GetECSchema(schemaName);

    if (schema == nullptr || sourceClass == nullptr || targetClass == nullptr)
        {
        return classes;
        }

    for (ECClassCP ecClass : schema->GetClasses())
        {
        ECRelationshipClassCP relClass = ecClass->GetRelationshipClassCP();
        if (nullptr == relClass)
            continue;

        if ((DoesConstraintSupportECClass(relClass->GetSource(), *sourceClass, true) &&
             DoesConstraintSupportECClass(relClass->GetTarget(), *targetClass, true)) ||
            (DoesConstraintSupportECClass(relClass->GetSource(), *targetClass, true) &&
             DoesConstraintSupportECClass(relClass->GetTarget(), *sourceClass, true)))
            {
            classes.push_back(relClass);
            }
        }

    return classes;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP ECDbAdapter::FindRelationshipClassWithSource(ECClassId sourceClassId, ECClassId targetClassId)
    {
    ECRelationshipClassCP relClass = nullptr;

    ECClassCP sourceClass = GetECClass(sourceClassId);
    if (sourceClass == nullptr)
        {
        return nullptr;
        }

    for (ECRelationshipClassCP candidateRelClass : FindRelationshipClasses(sourceClassId, targetClassId))
        {
        if (DoesConstraintSupportECClass(candidateRelClass->GetSource(), *sourceClass, false))
            {
            if (relClass != nullptr)
                {
                // Duplicate found
                return nullptr;
                }
            relClass = candidateRelClass;
            }
        }

    return relClass;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jeremy.Fisher   03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP ECDbAdapter::FindRelationshipClassWithTarget(ECClassId sourceClassId, ECClassId targetClassId)
    {
    ECRelationshipClassCP relClass = nullptr;

    ECClassCP targetClass = GetECClass(targetClassId);
    if (targetClass == nullptr)
        {
        return nullptr;
        }

    for (ECRelationshipClassCP candidateRelClass : FindRelationshipClasses(sourceClassId, targetClassId))
        {
        if (DoesConstraintSupportECClass(candidateRelClass->GetTarget(), *targetClass, false))
            {
            if (relClass != nullptr)
                {
                // Duplicate found
                return nullptr;
                }
            relClass = candidateRelClass;
            }
        }

    return relClass;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mark.Uvari    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP ECDbAdapter::FindClosestRelationshipClassWithSource(ECClassId sourceClassId, ECClassId targetClassId)
    {
    ECRelationshipClassCP relClass = nullptr;

    ECClassCP sourceClass = GetECClass(sourceClassId);
    ECClassCP targetClass = GetECClass(targetClassId);
    if (sourceClass == nullptr || targetClass == nullptr)
        {
        return nullptr;
        }

    int relClassDist = INT_MAX;
    for (ECRelationshipClassCP candidateRelClass : FindRelationshipClasses(sourceClassId, targetClassId))
        {
        if (DoesConstraintSupportECClass(candidateRelClass->GetSource(), *sourceClass, false))
            {
            for (auto candTargClass : candidateRelClass->GetTarget().GetClasses())
                {
                //Find closest distance from inherited class
                int dist = FindDistanceFromBaseClass(targetClass, candTargClass);
                if (dist >= 0 && dist < relClassDist)
                    {
                    relClass = candidateRelClass;
                    relClassDist = dist;
                    }
                }
            }
        }

    return relClass;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey ECDbAdapter::GetInstanceKeyFromJsonInstance(JsonValueCR ecInstanceJson)
    {
    ECClassCP ecClass = GetECClass(ecInstanceJson["$ECClassKey"].asString());
    if (nullptr == ecClass)
        {
        return ECInstanceKey();
        }
    ECInstanceId ecId = ECDbHelper::ECInstanceIdFromJsonInstance(ecInstanceJson);
    return ECInstanceKey(ecClass->GetId(), ecId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::ExtractJsonInstanceArrayFromStatement(ECSqlStatement& statement, ECClassCP ecClass, JsonValueR jsonInstancesArrayOut, ICancellationTokenPtr ct)
    {
    if (ct && ct->IsCanceled())
        {
        return ERROR;
        }

    Utf8String className(ecClass->GetName());

    JsonECSqlSelectAdapter adapter(statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));

    DbResult status;
    while (BE_SQLITE_ROW == (status = statement.Step()))
        {
        if (ct && ct->IsCanceled())
            {
            return ERROR;
            }

        JsonValueR currentObj = jsonInstancesArrayOut.append(Json::objectValue);

        if (!adapter.GetRowInstance(currentObj, ecClass->GetId()))
            {
            return ERROR;
            }
        }

    if (BE_SQLITE_DONE != status)
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::ExtractJsonInstanceFromStatement(ECSqlStatement& statement, ECClassCP ecClass, JsonValueR jsonInstanceOut)
    {
    Utf8String className(ecClass->GetName());

    if (BE_SQLITE_ROW != statement.Step())
        {
        jsonInstanceOut = Json::Value::null;
        return ERROR;
        }

    JsonECSqlSelectAdapter adapter(statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
    if (!adapter.GetRowInstance(jsonInstanceOut, ecClass->GetId()))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::ExtractECIdsFromStatement
(
ECSqlStatement& statement,
int ecInstanceIdcolumn,
bvector<ECInstanceId>& ecIdsOut,
ICancellationTokenPtr ct
)
    {
    DbResult status;
    while (BE_SQLITE_ROW == (status = statement.Step()))
        {
        if (ct && ct->IsCanceled())
            {
            return ERROR;
            }
        ecIdsOut.push_back(statement.GetValueId<ECInstanceId>(ecInstanceIdcolumn));
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytenis.Navalinskas    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::ExtractECInstanceKeyMultiMapFromStatement
(
ECSqlStatement& statement,
int ecInstanceIdcolumn,
ECClassId classId,
ECInstanceKeyMultiMap& keysOut,
ICancellationTokenPtr ct
)
    {
    DbResult status;
    while (BE_SQLITE_ROW == (status = statement.Step()))
        {
        if (ct && ct->IsCanceled())
            {
            return ERROR;
            }
        keysOut.Insert(classId, statement.GetValueId<ECInstanceId>(ecInstanceIdcolumn));
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::PrepareStatement(ECSqlStatement& statement, ECSqlBuilderCR builder)
    {
    return PrepareStatement(statement, builder.ToString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::PrepareStatement(ECSqlStatement& statement, Utf8StringCR ecsql)
    {
    ECSqlStatus status = statement.Prepare(*m_ecDb, ecsql.c_str());
    if (ECSqlStatus::Success != status)
        {
        BeAssert(false && "Failed to prepare statement");
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::BindParameters(ECSqlStatement& statement, const bvector<Utf8String>& parameters, IECSqlBinder::MakeCopy makeCopy)
    {
    int parameterIndex = 0;
    for (Utf8StringCR parameter : parameters)
        {
        ++parameterIndex;
        ECSqlStatus status = statement.BindText(parameterIndex, parameter.c_str(), makeCopy);
        if (ECSqlStatus::Success != status)
            {
            BeAssert(false);
            return ERROR;
            }
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
int ECDbAdapter::CountClassInstances(ECClassCP ecClass)
    {
    Utf8String ecsql = "SELECT NULL FROM ONLY " + ECSqlBuilder::ToECSqlSnippet(*ecClass);

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, ecsql))
        {
        return 0;
        }

    int count = 0;
    while (BE_SQLITE_ROW == statement.Step())
        {
        count++;
        }
    return count;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceId ECDbAdapter::FindInstance(ECClassCP ecClass, Utf8CP whereClause)
    {
    if (nullptr == ecClass)
        {
        return ECInstanceId();
        }

    Utf8String ecsql = "SELECT ECInstanceId FROM ONLY " + ECSqlBuilder::ToECSqlSnippet(*ecClass) + " ";
    if (nullptr != whereClause)
        {
        ecsql += "WHERE " + Utf8String(whereClause) + " ";
        }
    ecsql += "LIMIT 1 ";

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, ecsql))
        {
        return ECInstanceId();
        }

    if (BE_SQLITE_ROW != statement.Step())
        {
        return ECInstanceId();
        }

    return statement.GetValueId<ECInstanceId>(0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bset<ECInstanceId> ECDbAdapter::FindInstances(ECClassCP ecClass, Utf8CP whereClause)
    {
    bset<ECInstanceId> ids;

    Utf8String ecsql = "SELECT ECInstanceId FROM ONLY " + ECSqlBuilder::ToECSqlSnippet(*ecClass) + " ";
    if (nullptr != whereClause)
        {
        ecsql += "WHERE " + Utf8String(whereClause);
        }

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, ecsql))
        {
        return ids;
        }

    while (BE_SQLITE_ROW == statement.Step())
        {
        ids.insert(statement.GetValueId<ECInstanceId>(0));
        }

    return ids;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::GetJsonInstance(JsonValueR objectOut, ECInstanceKeyCR instanceKey)
    {
    return GetJsonInstance(objectOut, GetECClass(instanceKey.GetECClassId()), instanceKey.GetECInstanceId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::GetJsonInstance(JsonValueR jsonOut, ECClassCP ecClass, ECInstanceId ecId)
    {
    Utf8String ecsql = "SELECT * FROM ONLY " + ECSqlBuilder::ToECSqlSnippet(*ecClass) + " WHERE ECInstanceId = ? LIMIT 1 ";

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, ecsql))
        {
        return ERROR;
        }

    statement.BindId(1, ecId);

    return ExtractJsonInstanceFromStatement(statement, ecClass, jsonOut);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::GetJsonInstance(JsonValueR objectOut, ECClassCP ecClass, Utf8CP whereClause, Utf8CP select)
    {
    Utf8String ecsql;
    if (nullptr != select)
        {
        ecsql = "SELECT " + Utf8String(select) + " ";
        }
    else
        {
        ecsql = "SELECT * ";
        }

    ecsql += "FROM ONLY " + ECSqlBuilder::ToECSqlSnippet(*ecClass) + " ";

    if (nullptr != whereClause)
        {
        ecsql += "WHERE " + Utf8String(whereClause);
        }

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, ecsql))
        {
        return ERROR;
        }

    return ExtractJsonInstanceFromStatement(statement, ecClass, objectOut);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::GetJsonInstances(JsonValueR jsonOut, ECClassCP ecClass, Utf8CP whereClause, ICancellationTokenPtr ct)
    {
    Utf8String ecsql = "SELECT * FROM ONLY " + ECSqlBuilder::ToECSqlSnippet(*ecClass) + " ";
    if (whereClause != nullptr)
        {
        ecsql += "WHERE " + Utf8String(whereClause);
        }

    ECSqlStatement statement;
    PrepareStatement(statement, ecsql);

    return GetJsonInstances(jsonOut, ecClass, statement, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::GetJsonInstances
(
JsonValueR jsonOut,
ECClassCP ecClass,
ECSqlStatement& statement,
ICancellationTokenPtr ct
)
    {
    return ExtractJsonInstanceArrayFromStatement(statement, ecClass, jsonOut, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey ECDbAdapter::RelateInstances(ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target)
    {
    if (nullptr == relClass)
        {
        return ECInstanceKey();
        }

    if (!source.IsValid() || !target.IsValid())
        {
        return ECInstanceKey(relClass->GetId(), ECInstanceId());
        }

    // EC Preffered way for checking existing relationship (don't rely on INSERT_ConstraintViolation)
    ECInstanceKey relationshipKey = FindRelationship(relClass, source, target);
    if (relationshipKey.IsValid())
        {
        return relationshipKey;
        }

    Utf8PrintfString key("RelateInstances:%lld", relClass->GetId());
    auto statement = m_statementCache.GetPreparedStatement(key, [&]
        {
        return Utf8PrintfString(
            "INSERT INTO %s (SourceECClassId, SourceECInstanceId, TargetECClassId, TargetECInstanceId) VALUES (?,?,?,?)",
            relClass->GetECSqlName().c_str()
            );
        });

    statement->BindInt64(1, source.GetECClassId());
    statement->BindId(2, source.GetECInstanceId());
    statement->BindInt64(3, target.GetECClassId());
    statement->BindId(4, target.GetECInstanceId());

    if (BE_SQLITE_DONE != statement->Step(relationshipKey))
        {
        return ECInstanceKey(relClass->GetId(), ECInstanceId());
        }

    return relationshipKey;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey ECDbAdapter::FindRelationship(ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target)
    {
    if (nullptr == relClass)
        {
        return ECInstanceKey();
        }

    if (!source.IsValid() || !target.IsValid())
        {
        return ECInstanceKey(relClass->GetId(), ECInstanceId());
        }

    Utf8PrintfString key("FindRelationship:%lld", relClass->GetId());
    auto statement = m_statementCache.GetPreparedStatement(key, [&]
        {
        return  Utf8PrintfString(
            "SELECT ECInstanceId "
            "FROM ONLY %s "
            "WHERE SourceECClassId = ? AND SourceECInstanceId = ? AND TargetECClassId = ? AND TargetECInstanceId = ? "
            "LIMIT 1 ",
            relClass->GetECSqlName().c_str()
            );
        });

    statement->BindInt64(1, source.GetECClassId());
    statement->BindId(2, source.GetECInstanceId());
    statement->BindInt64(3, target.GetECClassId());
    statement->BindId(4, target.GetECInstanceId());

    if (BE_SQLITE_ROW != statement->Step())
        {
        return ECInstanceKey(relClass->GetId(), ECInstanceId());
        }

    return ECInstanceKey(relClass->GetId(), statement->GetValueId<ECInstanceId>(0));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbAdapter::HasRelationship(ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target)
    {
    return FindRelationship(relClass, source, target).IsValid();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::DeleteRelationship(ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target)
    {
    // TODO: not optmized, deletes not direclty
    ECInstanceKey relationship = FindRelationship(relClass, source, target);
    if (!relationship.IsValid())
        {
        return ERROR;
        }
#if defined (NEEDS_WORK_PORT_GRA06) // Port 0504 to 06,
    ECPersistencePtr persistence = m_ecDb->GetECPersistence(nullptr, *relClass);
    if (persistence.IsNull() ||
        DELETE_Success != persistence->Delete(relationship.GetECInstanceId()))
        {
        return ERROR;
        }
#endif
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::GetRelatedTargetIds(ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECClassCP targetClass, bvector<ECInstanceId>& ecIdsOut)
    {
    ECClassCP sourceClass = GetECClass(source);

    if (nullptr == sourceClass || nullptr == targetClass || nullptr == relClass || !source.IsValid())
        {
        return ERROR;
        }

    Utf8String ecsql =
        "SELECT t.ECInstanceId "
        "FROM ONLY " + ECSqlBuilder::ToECSqlSnippet(*targetClass) + " t "
        "JOIN ONLY " + ECSqlBuilder::ToECSqlSnippet(*sourceClass) + " s "
        "USING " + ECSqlBuilder::ToECSqlSnippet(*relClass) + " REVERSE "
        "WHERE s.ECInstanceId = ? ";

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, ecsql))
        {
        return ERROR;
        }

    statement.BindId(1, source.GetECInstanceId());

    return ExtractECIdsFromStatement(statement, 0, ecIdsOut);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::GetRelatedSourceIds(ECRelationshipClassCP relClass, ECClassCP sourceClass, bvector<ECInstanceId>& ecIdsOut, ECInstanceKeyCR target)
    {
    ECClassCP targetClass = GetECClass(target);

    if (nullptr == sourceClass || nullptr == targetClass || nullptr == relClass || !target.IsValid())
        {
        return ERROR;
        }

    Utf8String ecsql =
        "SELECT s.ECInstanceId "
        "FROM ONLY " + ECSqlBuilder::ToECSqlSnippet(*sourceClass) + " s "
        "JOIN ONLY " + ECSqlBuilder::ToECSqlSnippet(*targetClass) + " t "
        "USING " + ECSqlBuilder::ToECSqlSnippet(*relClass) + " FORWARD "
        "WHERE t.ECInstanceId = ? ";

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, ecsql))
        {
        return ERROR;
        }

    statement.BindId(1, target.GetECInstanceId());

    return ExtractECIdsFromStatement(statement, 0, ecIdsOut);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::GetJsonRelatedSources(JsonValueR arrayOut, ECRelationshipClassCP relClass, ECClassCP sourceClass, ECInstanceKeyCR target)
    {
    ECClassCP targetClass = GetECClass(target);

    if (nullptr == sourceClass || nullptr == targetClass || nullptr == relClass || !target.IsValid())
        {
        return ERROR;
        }

    Utf8String ecsql =
        "SELECT s.* "
        "FROM ONLY " + ECSqlBuilder::ToECSqlSnippet(*sourceClass) + " s "
        "JOIN ONLY " + ECSqlBuilder::ToECSqlSnippet(*targetClass) + " t "
        "USING " + ECSqlBuilder::ToECSqlSnippet(*relClass) + " FORWARD "
        "WHERE t.ECInstanceId = ? ";

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, ecsql))
        {
        return ERROR;
        }

    statement.BindId(1, target.GetECInstanceId());

    return ExtractJsonInstanceArrayFromStatement(statement, sourceClass, arrayOut);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::GetJsonRelatedTargets(JsonValueR arrayOut, ECRelationshipClassCP relClass, ECClassCP targetClass, ECInstanceKeyCR source, Utf8CP orderBy)
    {
    ECClassCP sourceClass = GetECClass(source);

    if (nullptr == sourceClass || nullptr == targetClass || nullptr == relClass || !source.IsValid())
        {
        return ERROR;
        }

    Utf8String ecsql =
        "SELECT t.* "
        "FROM ONLY " + ECSqlBuilder::ToECSqlSnippet(*targetClass) + " t "
        "JOIN ONLY " + ECSqlBuilder::ToECSqlSnippet(*sourceClass) + " s "
        "USING " + ECSqlBuilder::ToECSqlSnippet(*relClass) + " REVERSE "
        "WHERE s.ECInstanceId = ? ";

    if (nullptr != orderBy)
        {
        ecsql += " ORDER BY " + Utf8String(orderBy);
        }

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, ecsql))
        {
        return ERROR;
        }

    statement.BindId(1, source.GetECInstanceId());

    return ExtractJsonInstanceArrayFromStatement(statement, targetClass, arrayOut);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::GetRelatedTargetKeys(ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyMultiMap& keysOut)
    {
    if (nullptr == relClass || !source.IsValid())
        {
        return ERROR;
        }

    Utf8PrintfString ecsql(
        "SELECT rel.TargetECClassId, rel.TargetECInstanceId "
        "FROM ONLY %s rel "
        "WHERE rel.SourceECClassId = ? AND rel.SourceECInstanceId = ?",
        relClass->GetECSqlName().c_str()
        );

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, ecsql))
        {
        return ERROR;
        }

    statement.BindInt64(1, source.GetECClassId());
    statement.BindId(2, source.GetECInstanceId());

    DbResult status;
    while (BE_SQLITE_ROW == (status = statement.Step()))
        {
        ECClassId targetClassId = statement.GetValueId<ECClassId>(0);
        ECInstanceId targetInstanceId = statement.GetValueId<ECInstanceId>(1);
        keysOut.insert({targetClassId, targetInstanceId});
        }

    if (BE_SQLITE_DONE != status)
        {
        return ERROR;
        }

    return SUCCESS;
    }
