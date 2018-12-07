/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Util/ECDbAdapter.cpp $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    m_navigationProperties.clear();
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
    return m_ecDb->Schemas().GetSchema(schemaName);
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
    return m_ecDb->Schemas().GetClass(schemaName.c_str(), className.c_str());
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
    return m_ecDb->Schemas().GetClass(classId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECDbAdapter::GetECClass(ECInstanceKeyCR instanceKey)
    {
    return GetECClass(instanceKey.GetClassId());
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
    return GetECRelationshipClass(instanceKey.GetClassId());
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
    for (auto constraintClass : constraint.GetConstraintClasses())
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
        Utf8CP sql = R"(
        WITH RECURSIVE 
            RelationshipConstraintClasses (RelationshipClassId, RelationshipEnd, IsPolymorphic, ClassId, NestingLevel) AS
            (
            SELECT RC.RelationshipClassId, RC.RelationshipEnd, RC.IsPolymorphic, RCC.ClassId, 0
                FROM ec_RelationshipConstraint RC
                INNER JOIN ec_RelationshipConstraintClass RCC
                    ON  RC.Id = RCC.ConstraintId
            UNION
            SELECT RCC.RelationshipClassId, RCC.RelationshipEnd, RCC.IsPolymorphic, BC.ClassId, NestingLevel + 1
                FROM RelationshipConstraintClasses RCC
                INNER JOIN ec_ClassHasBaseClasses BC ON BC.BaseClassId = RCC.ClassId
                WHERE RCC.IsPolymorphic = 1
                ORDER BY 2 DESC
            )
        SELECT DISTINCT SRC.RelationshipClassId
            FROM RelationshipConstraintClasses SRC
            INNER JOIN RelationshipConstraintClasses TRG 
                ON SRC.RelationshipClassId = TRG.RelationshipClassId
            WHERE SRC.ClassId = ? AND TRG.ClassId = ? )";

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

    m_findRelationshipClassesStatement->BindId(1, sourceClassId);
    m_findRelationshipClassesStatement->BindId(2, targetClassId);

    BeSQLite::DbResult status;
    while (BeSQLite::DbResult::BE_SQLITE_ROW == (status = m_findRelationshipClassesStatement->Step()))
        {
        classes.push_back(GetECRelationshipClass(m_findRelationshipClassesStatement->GetValueId<ECClassId>(0)));
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
        if (candidateRelClass->GetStrengthDirection() == ECRelatedInstanceDirection::Forward
            && DoesConstraintSupportECClass(candidateRelClass->GetSource(), *sourceClass, false)
            || candidateRelClass->GetStrengthDirection() == ECRelatedInstanceDirection::Backward
            && DoesConstraintSupportECClass(candidateRelClass->GetTarget(), *sourceClass, false))
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
        if (candidateRelClass->GetStrengthDirection() == ECRelatedInstanceDirection::Forward
            && DoesConstraintSupportECClass(candidateRelClass->GetTarget(), *targetClass, false)
            || candidateRelClass->GetStrengthDirection() == ECRelatedInstanceDirection::Backward
            && DoesConstraintSupportECClass(candidateRelClass->GetSource(), *targetClass, false))
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
        ECRelationshipConstraintClassList candidateClasses;

        if (DoesConstraintSupportECClass(candidateRelClass->GetTarget(), *sourceClass, true))
            {
            candidateClasses = candidateRelClass->GetSource().GetConstraintClasses();
            }
        else if (DoesConstraintSupportECClass(candidateRelClass->GetSource(), *sourceClass, true))
            {
            candidateClasses = candidateRelClass->GetTarget().GetConstraintClasses();
            }

        for (auto candClass : candidateClasses)
            {
            //Find closest distance from inherited class
            int dist = FindDistanceFromBaseClass(targetClass, candClass);
            if (dist >= 0 && dist < relClassDist)
                {
                relClass = candidateRelClass;
                relClassDist = dist;
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
    ECClassCP ecClass = GetECClass(ecInstanceJson[ECJsonUtilities::json_className()].asString());
    if (nullptr == ecClass)
        {
        return ECInstanceKey();
        }
    ECInstanceId ecId = ECDbHelper::ECInstanceIdFromJsonInstance(ecInstanceJson);
    return ECInstanceKey(ecClass->GetId(), ecId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Petras.Sukys    11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey ECDbAdapter::GetInstanceKeyFromJsonInstance(const rapidjson::Value& ecInstanceJson)
    {
    ECClassCP ecClass = GetECClass(ecInstanceJson[ECJsonUtilities::json_className().c_str()].GetString());
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
    if (nullptr == ecClass)
        return ERROR;

    if (ct && ct->IsCanceled())
        return ERROR;

    Utf8String className(ecClass->GetName());

    JsonECSqlSelectAdapter adapter(statement);

    DbResult status;
    while (BE_SQLITE_ROW == (status = statement.Step()))
        {
        if (ct && ct->IsCanceled())
            {
            return ERROR;
            }

        JsonValueR currentObj = jsonInstancesArrayOut.append(Json::objectValue);

        if (SUCCESS != adapter.GetRowInstance(currentObj, ecClass->GetId()))
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
        jsonInstanceOut = Json::Value::GetNull();
        return ERROR;
        }

    JsonECSqlSelectAdapter adapter(statement);
    if (SUCCESS != adapter.GetRowInstance(jsonInstanceOut, ecClass->GetId()))
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
            return ERROR;
        ecIdsOut.push_back(statement.GetValueId<ECInstanceId>(ecInstanceIdcolumn));
        }

    if (BE_SQLITE_DONE != status)
        return ERROR;

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
            return ERROR;

        keysOut.Insert(classId, statement.GetValueId<ECInstanceId>(ecInstanceIdcolumn));
        }

    if (BE_SQLITE_DONE != status)
        return ERROR;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::ExtractECInstanceKeys
(
ECSqlStatement& statement,
ECInstanceKeyMultiMap& keysOut,
ICancellationTokenPtr ct,
int ecClassIdColumn,
int ecInstanceIdcolumn
)
    {
    DbResult status;
    while (BE_SQLITE_ROW == (status = statement.Step()))
        {
        if (ct && ct->IsCanceled())
            return ERROR;

        auto ecClassId = statement.GetValueId<ECClassId>(ecClassIdColumn);
        auto ecInstanceId = statement.GetValueId<ECInstanceId>(ecInstanceIdcolumn);
        keysOut.Insert(ecClassId, ecInstanceId);
        }

    if (BE_SQLITE_DONE != status)
        return ERROR;

    return SUCCESS;
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
    if (nullptr == ecClass)
        {
        return 0;
        }

    Utf8String ecsql = "SELECT NULL FROM ONLY " + ecClass->GetECSqlName();

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

    Utf8String ecsql = "SELECT ECInstanceId FROM ONLY " + ecClass->GetECSqlName() + " ";
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

    if (nullptr == ecClass)
        return ids;

    Utf8String ecsql = "SELECT ECInstanceId FROM ONLY " + ecClass->GetECSqlName() + " ";
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
    return GetJsonInstance(objectOut, GetECClass(instanceKey.GetClassId()), instanceKey.GetInstanceId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::GetJsonInstance(JsonValueR jsonOut, ECClassCP ecClass, ECInstanceId ecId)
    {
    if (nullptr == ecClass)
        return ERROR;

    Utf8String ecsql = "SELECT * FROM ONLY " + ecClass->GetECSqlName() + " WHERE ECInstanceId = ? LIMIT 1 ";

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
    if (nullptr == ecClass)
        return ERROR;

    Utf8String ecsql;
    if (nullptr != select)
        {
        ecsql = "SELECT " + Utf8String(select) + " ";
        }
    else
        {
        ecsql = "SELECT * ";
        }

    ecsql += "FROM ONLY " + ecClass->GetECSqlName() + " ";

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
    if (nullptr == ecClass)
        return ERROR;

    Utf8String ecsql = "SELECT * FROM ONLY " + ecClass->GetECSqlName() + " ";
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

    Utf8PrintfString key("RelateInstances:%llu", relClass->GetId().GetValue());
    auto statement = m_statementCache.GetPreparedStatement(key, [&]
        {
        //WIP_NEEDS_REFACTOR Cannot insert into fk relationship. Needs to insert via its nav prop
        return Utf8PrintfString(
            "INSERT INTO %s (SourceECClassId, SourceECInstanceId, TargetECClassId, TargetECInstanceId) VALUES (?,?,?,?)",
            relClass->GetECSqlName().c_str()
            );
        });

    if (statement == nullptr || !statement->IsPrepared())
        return ECInstanceKey();

    statement->BindId(1, source.GetClassId());
    statement->BindId(2, source.GetInstanceId());
    statement->BindId(3, target.GetClassId());
    statement->BindId(4, target.GetInstanceId());

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

    Utf8PrintfString key("FindRelationship:%llu", relClass->GetId().GetValue());
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

    statement->BindId(1, source.GetClassId());
    statement->BindId(2, source.GetInstanceId());
    statement->BindId(3, target.GetClassId());
    statement->BindId(4, target.GetInstanceId());

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
        // Nothing to delete
        return SUCCESS;
        }

    ECInstanceKeyMultiMap instances;
    instances.Insert(relationship.GetClassId(), relationship.GetInstanceId());

    return DeleteInstances(instances);
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
        "FROM ONLY " + targetClass->GetECSqlName() + " t "
        "JOIN ONLY " + sourceClass->GetECSqlName() + " s "
        "USING " + relClass->GetECSqlName() + " BACKWARD "
        "WHERE s.ECInstanceId = ? ";

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, ecsql))
        {
        return ERROR;
        }

    statement.BindId(1, source.GetInstanceId());

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
        "FROM ONLY " + sourceClass->GetECSqlName() + " s "
        "JOIN ONLY " + targetClass->GetECSqlName() + " t "
        "USING " + relClass->GetECSqlName() + " FORWARD "
        "WHERE t.ECInstanceId = ? ";

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, ecsql))
        {
        return ERROR;
        }

    statement.BindId(1, target.GetInstanceId());

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
        "FROM ONLY " + sourceClass->GetECSqlName() + " s "
        "JOIN ONLY " + targetClass->GetECSqlName() + " t "
        "USING " + relClass->GetECSqlName() + " FORWARD "
        "WHERE t.ECInstanceId = ? ";

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, ecsql))
        {
        return ERROR;
        }

    statement.BindId(1, target.GetInstanceId());

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
        "FROM ONLY " + targetClass->GetECSqlName() + " t "
        "JOIN ONLY " + sourceClass->GetECSqlName() + " s "
        "USING " + relClass->GetECSqlName() + " BACKWARD "
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

    statement.BindId(1, source.GetInstanceId());

    return ExtractJsonInstanceArrayFromStatement(statement, targetClass, arrayOut);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::GetRelatedTargetKeys(ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyMultiMap& keysOut)
    {
    Utf8PrintfString ecsql(
        "SELECT rel.TargetECClassId, rel.TargetECInstanceId "
        "FROM ONLY %s rel "
        "WHERE rel.SourceECClassId = ? AND rel.SourceECInstanceId = ?",
        relClass->GetECSqlName().c_str()
        );

    return GetRelatedKeys(relClass, source, ecsql, keysOut);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::GetRelatedSourceKeys(ECRelationshipClassCP relClass, ECInstanceKeyCR target, ECInstanceKeyMultiMap& keysOut)
    {
    Utf8PrintfString ecsql(
        "SELECT rel.SourceECClassId, rel.SourceECInstanceId "
        "FROM ONLY %s rel "
        "WHERE rel.TargetECClassId = ? AND rel.TargetECInstanceId = ?",
        relClass->GetECSqlName().c_str()
        );

    return GetRelatedKeys(relClass, target, ecsql, keysOut);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas    09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::GetRelatedKeys(ECRelationshipClassCP relClass, ECInstanceKeyCR instance, Utf8PrintfString ecsql, ECInstanceKeyMultiMap& keysOut)
    {
    if (nullptr == relClass || !instance.IsValid())
        {
        return ERROR;
        }

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, ecsql))
        {
        return ERROR;
        }

    statement.BindId(1, instance.GetClassId());
    statement.BindId(2, instance.GetInstanceId());

    DbResult status;
    while (BE_SQLITE_ROW == (status = statement.Step()))
        {
        ECClassId relatedClassId = statement.GetValueId<ECClassId>(0);
        ECInstanceId relatedInstanceId = statement.GetValueId<ECInstanceId>(1);
        keysOut.insert({relatedClassId, relatedInstanceId});
        }

    if (BE_SQLITE_DONE != status)
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::OnBeforeDelete(ECClassCR ecClass, ECInstanceId instanceId, bset<ECInstanceKey>& additionalToDeleteOut)
    {
    if (ecClass.IsRelationshipClass())
        {
        return SUCCESS;
        }

    for (auto listener : m_deleteListeners)
        {
        if (SUCCESS != listener->OnBeforeDelete(ecClass, instanceId, additionalToDeleteOut))
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::DeleteInstance(ECInstanceKeyCR instanceKey)
    {
    ECInstanceKeyMultiMap instances;
    instances.Insert(instanceKey.GetClassId(), instanceKey.GetInstanceId());
    return DeleteInstances(instances);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::DeleteInstances(const ECInstanceKeyMultiMap& instances)
    {
    bset<ECInstanceKey> deleted;
    return DeleteInstances(instances, deleted);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::DeleteInstances(const ECInstanceKeyMultiMap& instances, bset<ECInstanceKey>& deleted)
    {
    if (instances.empty())
        return SUCCESS;

    bset<ECInstanceKey> allInstancesBeingDeleted;
    if (SUCCESS != FindInstancesBeingDeleted(instances, allInstancesBeingDeleted))
        return ERROR;

    bset<ECInstanceKey> additionalInstancesSet;
    ECInstanceKeyMultiMap allInstancesBeingDeletedMap;
    ECInstanceKeyMultiMap allRelationshipsBeingDeletedMap;

    for (ECInstanceKeyCR key : allInstancesBeingDeleted)
        {
        ECClassCP ecClass = GetECClass(key);
        if (nullptr == ecClass)
            return ERROR;

        if (ecClass->IsRelationshipClass())
            allRelationshipsBeingDeletedMap.insert({key.GetClassId(), key.GetInstanceId()});
        else
            allInstancesBeingDeletedMap.insert({key.GetClassId(), key.GetInstanceId()});

        for (auto listener : m_deleteListeners)
            {
            if (SUCCESS != listener->OnBeforeDelete(*ecClass, key.GetInstanceId(), additionalInstancesSet))
                return ERROR;
            }
        }

    if (SUCCESS != DeleteInstancesDirectly(allInstancesBeingDeletedMap, deleted))
        return ERROR;
    if (SUCCESS != DeleteInstancesDirectly(allRelationshipsBeingDeletedMap, deleted))
        return ERROR;

    ECInstanceKeyMultiMap additionalInstancesMap;
    for (auto& key : additionalInstancesSet)
        {
        if (deleted.find(key) != deleted.end())
            continue;

        additionalInstancesMap.insert({key.GetClassId(), key.GetInstanceId()});
        }

    return DeleteInstances(additionalInstancesMap, deleted);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::DeleteInstancesDirectly(const ECInstanceKeyMultiMap& instances, bset<ECInstanceKey>& deleted)
    {
    for (auto it = instances.begin(); it != instances.end();)
        {
        ECClassId ecClassId = it->first;
        auto classInstances = instances.equal_range(ecClassId);
        it = classInstances.second;

        ECInstanceIdSet ids;
        for (auto ciit = classInstances.first; ciit != classInstances.second; ++ciit)
            ids.insert(ciit->second);

        ECClassCP ecClass = GetECClass(ecClassId);
        if (ecClass->IsRelationshipClass())
            {
            if (SUCCESS != DeleteRelationshipInstancesUsingECSQL(ecClass->GetRelationshipClassCP(), ids))
                return ERROR;
            }
        else
            {
            if (SUCCESS != DeleteInstancesUsingECSQL(ecClass, ids))
                return ERROR;
            }

        for (auto id : ids)
            deleted.insert({ecClassId, id});
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::DeleteRelationshipInstancesUsingECSQL(ECRelationshipClassCP ecClass, const ECInstanceIdSet& ids)
    {
    if (nullptr == ecClass)
        return ERROR;

    ECPropertyId navPropertyId;
    Utf8String navPropertyName;
    ECClassId constraintClassId;
    if (SUCCESS != GetNavigationProperty(ecClass, navPropertyId, navPropertyName, constraintClassId))
        return ERROR;

    if (!navPropertyId.IsValid())
        return DeleteInstancesUsingECSQL(ecClass, ids);

    Utf8PrintfString key("DeleteRelationshipInstancesUsingECSQL:%llu", navPropertyId.GetValue());
    auto statement = m_statementCache.GetPreparedStatement(key, [&]
        {
        ECClassCP constraintClass = GetECClass(constraintClassId);
        if (nullptr == constraintClass)
            return bastring();
        return "UPDATE " + constraintClass->GetECSqlName() + " SET " + navPropertyName + " = NULL WHERE InVirtualSet(?, ECInstanceId) ";
        });

    statement->BindVirtualSet(1, ids);
    DbResult result = statement->Step();
    if (BE_SQLITE_DONE != result)
        return ERROR;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::GetNavigationProperty
(
ECRelationshipClassCP relClass, 
ECPropertyId& propertyIdOut, 
Utf8String& propertyNameOut, 
ECClassId& classIdOut
)
    {
    auto it = m_navigationProperties.find(relClass->GetId());
    if (it == m_navigationProperties.end())
        {
        auto statement = m_statementCache.GetPreparedStatement("GetNavigationProperty", [&]
            {
            return "SELECT ECInstanceId, Name, Class.Id FROM meta.ECPropertyDef WHERE NavigationRelationshipClass.Id = ? ";
            });

        statement->BindId(1, relClass->GetId());
        DbResult result = statement->Step();

        std::tuple<ECPropertyId, Utf8String, ECClassId> tuple;

        if (BE_SQLITE_ROW == result)
            {            
            std::get<0>(tuple) = statement->GetValueId<ECPropertyId>(0);
            std::get<1>(tuple) = statement->GetValueText(1);
            std::get<2>(tuple) = statement->GetValueId<ECClassId>(2);
            }
        else if (BE_SQLITE_DONE != result)
            {
            return ERROR;
            }

        it = m_navigationProperties.insert({relClass->GetId(), tuple}).first;
        }

    propertyIdOut = std::get<0>(it->second);
    propertyNameOut = std::get<1>(it->second);
    classIdOut = std::get<2>(it->second);
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::DeleteInstancesUsingECSQL(ECClassCP ecClass, const ECInstanceIdSet& ids)
    {
    if (nullptr == ecClass)
        return ERROR;

    Utf8PrintfString key("DeleteInstancesUsingECSQL:%llu", ecClass->GetId().GetValue());
    auto statement = m_statementCache.GetPreparedStatement(key, [&]
        {
        return "DELETE FROM ONLY " + ecClass->GetECSqlName() + " WHERE InVirtualSet(?, ECInstanceId) ";
        });

    statement->BindVirtualSet(1, ids);
    DbResult result = result = statement->Step();
    if (BE_SQLITE_DONE != result)
        return ERROR;
    
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::FindInstancesBeingDeleted
(
const ECInstanceKeyMultiMap& seedInstancesBeingDeleted,
bset<ECInstanceKey>& allInstancesBeingDeletedOut
)
    {
    for (auto& pair : seedInstancesBeingDeleted)
        {
        ECInstanceKey key(pair.first, pair.second);
        if (SUCCESS != FindInstancesBeingDeleted(key, allInstancesBeingDeletedOut))
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::FindInstancesBeingDeleted
(
ECInstanceKeyCR instanceToDelete,
bset<ECInstanceKey>& allInstancesBeingDeletedOut
)
    {
    if (!instanceToDelete.IsValid())
        {
        BeAssert(false && "ECInstanceKey is invalid");
        return ERROR;
        }

    auto it = allInstancesBeingDeletedOut.find(instanceToDelete);
    if (it != allInstancesBeingDeletedOut.end())
        {
        // Already found instances being deleted
        return SUCCESS;
        }

    allInstancesBeingDeletedOut.insert(instanceToDelete);

    ECRelationshipClassCP instanceRelClass = GetECRelationshipClass(instanceToDelete);
    if (nullptr != instanceRelClass)
        {
        return FindInstancesBeingDeletedForRelationship(*instanceRelClass, instanceToDelete, allInstancesBeingDeletedOut);
        }

    ECInstanceFinder& finder = GetECInstanceFinder();

    ECInstanceKeyMultiMap embeddedChildren, heldChildren, relationships;
    if (SUCCESS != finder.FindRelatedInstances(&embeddedChildren, nullptr, instanceToDelete, ECInstanceFinder::RelatedDirection_EmbeddedChildren) ||
        SUCCESS != finder.FindRelatedInstances(&heldChildren, nullptr, instanceToDelete, ECInstanceFinder::RelatedDirection_HeldChildren) ||
        SUCCESS != finder.FindRelatedInstances(nullptr, &relationships, instanceToDelete, ECInstanceFinder::RelatedDirection_All))
        {
        return ERROR;
        }

    for (auto& pair : relationships)
        {
        allInstancesBeingDeletedOut.insert(ECInstanceKey(pair.first, pair.second));
        }

    if (SUCCESS != FindInstancesBeingDeleted(embeddedChildren, allInstancesBeingDeletedOut))
        {
        return ERROR;
        }

    bset<ECInstanceKey> childrenBeingDeleted;
    for (auto& pair : heldChildren)
        {
        ECInstanceKey childKey(pair.first, pair.second);
        if (0 == CountHoldingParents(childKey, &allInstancesBeingDeletedOut))
            {
            childrenBeingDeleted.insert(childKey);
            }
        }

    for (auto& childKey : childrenBeingDeleted)
        {
        if (SUCCESS != FindInstancesBeingDeleted(childKey, allInstancesBeingDeletedOut))
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ECDbAdapter::CountHoldingParents(ECInstanceKeyCR instanceKey, const bset<ECInstanceKey>* parentsToIgnore)
    {
    ECInstanceFinder& finder = GetECInstanceFinder();

    ECInstanceKeyMultiMap holdingParents;
    if (SUCCESS != finder.FindRelatedInstances(&holdingParents, nullptr, instanceKey, ECInstanceFinder::RelatedDirection_HoldingParents))
        {
        BeAssert(false);
        return 0;
        }

    if (nullptr == parentsToIgnore)
        {
        return !holdingParents.empty();
        }

    size_t count = 0;
    for (auto& pair : holdingParents)
        {
        ECInstanceKey parentKey(pair.first, pair.second);
        auto it = parentsToIgnore->find(parentKey);
        if (it == parentsToIgnore->end())
            {
            count++;
            }
        }

    return count;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::FindInstancesBeingDeletedForRelationship
(
ECRelationshipClassCR relClass,
ECInstanceKeyCR instanceToDelete,
bset<ECInstanceKey>& allInstancesBeingDeletedOut
)
    {
    if (relClass.GetId() != instanceToDelete.GetClassId())
        {
        return ERROR;
        }

    if (StrengthType::Referencing == relClass.GetStrength())
        {
        return SUCCESS;
        }

    Utf8PrintfString key("FindInstancesBeingDeletedForRelationship:%llu", relClass.GetId().GetValue());
    auto statement = m_statementCache.GetPreparedStatement(key, [&]
        {
        Utf8String ecsql = "SELECT ";
        ECRelatedInstanceDirection direction = relClass.GetStrengthDirection();
        if (ECRelatedInstanceDirection::Forward == direction)
            {
            ecsql += "TargetECClassId, TargetECInstanceId ";
            }
        else if (ECRelatedInstanceDirection::Backward == direction)
            {
            ecsql += "SourceECClassId, SourceECInstanceId ";
            }

        ecsql += "FROM ONLY " + relClass.GetECSqlName() + " WHERE ECInstanceId = ? ";
        return ecsql;
        });

    statement->BindId(1, instanceToDelete.GetInstanceId());

    auto status = statement->Step();
    if (BE_SQLITE_DONE == status)
        return SUCCESS; // Relationship not found

    if (BE_SQLITE_ROW != status)
        return ERROR;

    ECInstanceKey child(statement->GetValueId<ECClassId>(0), statement->GetValueId<ECInstanceId>(1));

    if (StrengthType::Embedding == relClass.GetStrength() ||
        (StrengthType::Holding == relClass.GetStrength() && (CountHoldingParents(child, &allInstancesBeingDeletedOut) <= 1)))
        {
        return FindInstancesBeingDeleted(child, allInstancesBeingDeletedOut);
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbAdapter::RegisterDeleteListener(DeleteListener* listener)
    {
    m_deleteListeners.insert(listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbAdapter::UnRegisterDeleteListener(DeleteListener* listener)
    {
    m_deleteListeners.erase(listener);
    }
