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
    m_findRelationshipClassesWithSourceStatement = nullptr;
    m_findRelationshipClassesInSchemaStatement = nullptr;
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
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECRelationshipClassCP> ECDbAdapter::FindRelationshipClasses(ECClassId sourceClassId, ECClassId targetClassId)
    {
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
bvector<ECRelationshipClassCP> ECDbAdapter::FindRelationshipClassesWithSource(ECClassId sourceClassId, Utf8String schemaName)
    {
    bvector<ECRelationshipClassCP> classes;

    if (nullptr == m_findRelationshipClassesWithSourceStatement)
        {
        Utf8CP sql = R"(WITH RECURSIVE
           RelationshipConstraintClasses (ECClassId, RelationECClassId) AS
           (
           SELECT  RCC.ECClassId, RCC.RelationECClassId
               FROM ec_RelationshipConstraintClass RCC JOIN ec_Class EC ON RCC.ECClassId = EC.ECClassId JOIN ec_Schema ES ON EC.ECSchemaId = ES.ECSchemaId WHERE ES.Name = ?
           UNION
           SELECT RCC.ECClassId, BC.ECClassId
               FROM RelationshipConstraintClasses RCC
                   INNER JOIN ec_BaseClass BC ON BC.BaseECClassId = RCC.RelationECClassId JOIN ec_Class EC ON RCC.ECClassId = EC.ECClassId JOIN ec_Schema ES ON EC.ECSchemaId = ES.ECSchemaId WHERE ES.Name = ?
           )
                SELECT DISTINCT SRC.ECClassId
           FROM RelationshipConstraintClasses SRC
           INNER JOIN   RelationshipConstraintClasses TRG ON SRC.ECClassId = TRG.ECClassId
                WHERE SRC.RelationECClassId = ? OR TRG.RelationECClassId = ?)";

        m_findRelationshipClassesWithSourceStatement = std::make_shared<Statement>();

        BeSQLite::DbResult status;
        if (BeSQLite::DbResult::BE_SQLITE_OK != (status = m_findRelationshipClassesWithSourceStatement->Prepare(*m_ecDb, sql)))
            {
            BeAssert(false);
            return classes;
            }
        }
    else
        {
        m_findRelationshipClassesWithSourceStatement->Reset();
        m_findRelationshipClassesWithSourceStatement->ClearBindings();
        }

    m_findRelationshipClassesWithSourceStatement->BindText(1, schemaName, Statement::MAKE_COPY_Yes);
    m_findRelationshipClassesWithSourceStatement->BindText(2, schemaName, Statement::MAKE_COPY_Yes);
    m_findRelationshipClassesWithSourceStatement->BindInt64(3, sourceClassId);
    m_findRelationshipClassesWithSourceStatement->BindInt64(4, sourceClassId);

    BeSQLite::DbResult status;
    while (BeSQLite::DbResult::BE_SQLITE_ROW == (status = m_findRelationshipClassesWithSourceStatement->Step()))
        {
        classes.push_back(GetECRelationshipClass(m_findRelationshipClassesWithSourceStatement->GetValueInt64(0)));
        }

    return classes;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mark.Uvari      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECRelationshipClassCP> ECDbAdapter::FindRelationshipClassesInSchema(ECClassId sourceClassId, ECClassId targetClassId, Utf8String schemaName)
    {
    bvector<ECRelationshipClassCP> classes;

    if (nullptr == m_findRelationshipClassesInSchemaStatement)
        {
        Utf8CP sql = R"(WITH RECURSIVE
           RelationshipConstraintClasses (ECClassId, ECRelationshipEnd, IsPolymorphic, RelationECClassId, NestingLevel, SchemaName) AS
           (
           SELECT  RC.ECClassId, RCC.ECRelationshipEnd, RC.IsPolymorphic, RCC.RelationECClassId, 0, ES.Name
               FROM ec_RelationshipConstraint RC
                   INNER JOIN ec_RelationshipConstraintClass RCC ON RC.ECClassId = RCC.ECClassId  AND RC.[ECRelationshipEnd] = RCC.[ECRelationshipEnd] JOIN ec_Class EC ON RCC.ECClassId = EC.ECClassId JOIN ec_Schema ES ON EC.ECSchemaId = ES.ECSchemaId
           UNION
           SELECT RCC.ECClassId, RCC.ECRelationshipEnd, RCC.IsPolymorphic, BC.ECClassId, NestingLevel + 1, ES.Name
               FROM RelationshipConstraintClasses RCC
                   INNER JOIN ec_BaseClass BC ON BC.BaseECClassId = RCC.RelationECClassId JOIN ec_Class EC ON RCC.ECClassId = EC.ECClassId JOIN ec_Schema ES ON EC.ECSchemaId = ES.ECSchemaId
               WHERE RCC.IsPolymorphic = 1
               ORDER BY 2 DESC
           )
        SELECT SRC.ECClassId
           FROM RelationshipConstraintClasses SRC
           INNER JOIN   RelationshipConstraintClasses TRG ON SRC.ECClassId = TRG.ECClassId
                WHERE SRC.ECRelationshipEnd = 0 AND TRG.ECRelationshipEnd = 1
                      AND ((SRC.RelationECClassId = ? AND TRG.RelationECClassId = ?)
                          OR (TRG.RelationECClassId = ? AND SRC.RelationECClassId = ?))
                      AND SRC.SchemaName = ?)";

        m_findRelationshipClassesInSchemaStatement = std::make_shared<Statement>();

        BeSQLite::DbResult status;
        if (BeSQLite::DbResult::BE_SQLITE_OK != (status = m_findRelationshipClassesInSchemaStatement->Prepare(*m_ecDb, sql)))
            {
            BeAssert(false);
            return classes;
            }
        }
    else
        {
        m_findRelationshipClassesInSchemaStatement->Reset();
        m_findRelationshipClassesInSchemaStatement->ClearBindings();
        }

    m_findRelationshipClassesInSchemaStatement->BindInt64(1, sourceClassId);
    m_findRelationshipClassesInSchemaStatement->BindInt64(2, targetClassId);
    m_findRelationshipClassesInSchemaStatement->BindInt64(3, sourceClassId);
    m_findRelationshipClassesInSchemaStatement->BindInt64(4, targetClassId);
    m_findRelationshipClassesInSchemaStatement->BindText(5, schemaName, Statement::MakeCopy::Yes); //TODO: Temporarily hard-coded schema

    BeSQLite::DbResult status;
    while (BeSQLite::DbResult::BE_SQLITE_ROW == (status = m_findRelationshipClassesInSchemaStatement->Step()))
        {
        classes.push_back(GetECRelationshipClass(m_findRelationshipClassesInSchemaStatement->GetValueInt64(0)));
        }

    return classes;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP ECDbAdapter::FindRelationshipClassWithSource(ECClassId sourceClassId, ECClassId targetClassId)
    {
    ECRelationshipClassCP relClass = nullptr;
    for (ECRelationshipClassCP candidateRelClass : FindRelationshipClasses(sourceClassId, targetClassId))
        {
        for (ECClassCP sourceClass : candidateRelClass->GetSource().GetClasses())
            {
            if (sourceClass->GetId() != sourceClassId)
                {
                continue;
                }

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
    for (ECRelationshipClassCP candidateRelClass : FindRelationshipClasses(sourceClassId, targetClassId))
        {
        for (ECClassCP targetClass : candidateRelClass->GetTarget().GetClasses())
            {
            if (targetClass->GetId() != targetClassId)
                {
                continue;
                }

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
BentleyStatus ECDbAdapter::ExtractJsonInstanceArrayFromStatement(ECSqlStatement& statement, ECClassCP ecClass, JsonValueR jsonInstancesArrayOut, ICancellationTokenPtr cancellationToken)
    {
    if (cancellationToken && cancellationToken->IsCanceled())
        {
        return ERROR;
        }

    Utf8String className(ecClass->GetName());

    JsonECSqlSelectAdapter adapter(statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));

    ECSqlStepStatus status;
    while (ECSqlStepStatus::HasRow == (status = statement.Step()))
        {
        if (cancellationToken && cancellationToken->IsCanceled())
            {
            return ERROR;
            }

        JsonValueR currentObj = jsonInstancesArrayOut.append(Json::objectValue);

        if (!adapter.GetRowInstance(currentObj, ecClass->GetId()))
            {
            return ERROR;
            }
        }

    if (ECSqlStepStatus::Done != status)
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

    if (ECSqlStepStatus::HasRow != statement.Step())
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
ICancellationTokenPtr cancellationToken
)
    {
    ECSqlStepStatus status;
    while (ECSqlStepStatus::HasRow == (status = statement.Step()))
        {
        if (cancellationToken && cancellationToken->IsCanceled())
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
ICancellationTokenPtr cancellationToken
)
    {
    ECSqlStepStatus status;
    while (ECSqlStepStatus::HasRow == (status = statement.Step()))
        {
        if (cancellationToken && cancellationToken->IsCanceled())
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
    ECSqlSelectBuilder sqlBuilder;
    sqlBuilder.Select("NULL").From(*ecClass, false);

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, sqlBuilder))
        {
        return 0;
        }

    int count = 0;
    while (ECSqlStepStatus::HasRow == statement.Step())
        {
        count++;
        }
    return count;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceId ECDbAdapter::FindInstance(ECClassCP ecClass, Utf8CP whereQuery)
    {
    if (nullptr == ecClass)
        {
        return ECInstanceId();
        }

    ECSqlSelectBuilder sqlBuilder;
    sqlBuilder.Select("ECInstanceId").From(*ecClass, false).Limit("1");

    if (nullptr != whereQuery)
        {
        sqlBuilder.Where(whereQuery);
        }

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, sqlBuilder))
        {
        return ECInstanceId();
        }

    if (ECSqlStepStatus::HasRow != statement.Step())
        {
        return ECInstanceId();
        }

    return statement.GetValueId<ECInstanceId>(0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bset<ECInstanceId> ECDbAdapter::FindInstances(ECClassCP ecClass, Utf8CP whereQuery)
    {
    bset<ECInstanceId> ids;

    ECSqlSelectBuilder sqlBuilder;
    sqlBuilder.Select("ECInstanceId").From(*ecClass, false);

    if (nullptr != whereQuery)
        {
        sqlBuilder.Where(whereQuery);
        }

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, sqlBuilder))
        {
        return ids;
        }

    while (ECSqlStepStatus::HasRow == statement.Step())
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
    ECSqlSelectBuilder sqlBuilder;
    sqlBuilder.SelectAll().From(*ecClass, false).Where("ECInstanceId = ?");

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, sqlBuilder))
        {
        return ERROR;
        }

    statement.BindId(1, ecId);

    return ExtractJsonInstanceFromStatement(statement, ecClass, jsonOut);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::GetJsonInstance(JsonValueR objectOut, ECClassCP ecClass, Utf8CP optionalWhereQuery, Utf8CP optionalSelect)
    {
    ECSqlSelectBuilder sqlBuilder;
    sqlBuilder.From(*ecClass, false).Limit("1");

    if (nullptr != optionalWhereQuery)
        {
        sqlBuilder.Where(optionalWhereQuery);
        }

    if (nullptr != optionalSelect)
        {
        sqlBuilder.Select(optionalSelect);
        }
    else
        {
        sqlBuilder.SelectAll();
        }

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, sqlBuilder))
        {
        return ERROR;
        }

    return ExtractJsonInstanceFromStatement(statement, ecClass, objectOut);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::GetJsonInstances(JsonValueR jsonOut, ECClassCP ecClass, Utf8CP whereQuery, ICancellationTokenPtr cancellationToken)
    {
    ECSqlSelectBuilder sqlBuilder;
    sqlBuilder.Select("*");
    sqlBuilder.From(*ecClass, false);

    if (whereQuery != nullptr)
        {
        sqlBuilder.Where(whereQuery);
        }

    ECSqlStatement statement;
    PrepareStatement(statement, sqlBuilder);

    return GetJsonInstances(jsonOut, ecClass, statement, cancellationToken);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::GetJsonInstances
(
JsonValueR jsonOut,
ECClassCP ecClass,
ECSqlStatement& statement,
ICancellationTokenPtr cancellationToken
)
    {
    return ExtractJsonInstanceArrayFromStatement(statement, ecClass, jsonOut, cancellationToken);
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
            ECSqlBuilder::ToECSqlSnippet(*relClass).c_str()
            );
        });

    statement->BindInt64(1, source.GetECClassId());
    statement->BindId(2, source.GetECInstanceId());
    statement->BindInt64(3, target.GetECClassId());
    statement->BindId(4, target.GetECInstanceId());

    if (ECSqlStepStatus::Done != statement->Step(relationshipKey))
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
            ECSqlBuilder::ToECSqlSnippet(*relClass).c_str()
            );
        });

    statement->BindInt64(1, source.GetECClassId());
    statement->BindId(2, source.GetECInstanceId());
    statement->BindInt64(3, target.GetECClassId());
    statement->BindId(4, target.GetECInstanceId());

    if (ECSqlStepStatus::HasRow != statement->Step())
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

    ECSqlSelectBuilder sqlBuilder;
    sqlBuilder
        .Select("t.ECInstanceId")
        .From(*targetClass, "t", false)
        .Join(*sourceClass, "s", false)
        .Using(*relClass, JoinDirection::Reverse)
        .Where("s.ECInstanceId = ?");

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, sqlBuilder))
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

    ECSqlSelectBuilder sqlBuilder;
    sqlBuilder
        .Select("s.ECInstanceId")
        .From(*sourceClass, "s", false)
        .Join(*targetClass, "t", false)
        .Using(*relClass, JoinDirection::Forward)
        .Where("t.ECInstanceId = ?");

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, sqlBuilder))
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

    ECSqlSelectBuilder sqlBuilder;
    sqlBuilder
        .Select("s.*")
        .From(*sourceClass, "s", false)
        .Join(*targetClass, "t", false)
        .Using(*relClass, JoinDirection::Forward)
        .Where("t.ECInstanceId = ?");

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, sqlBuilder))
        {
        return ERROR;
        }

    statement.BindId(1, target.GetECInstanceId());

    return ExtractJsonInstanceArrayFromStatement(statement, sourceClass, arrayOut);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbAdapter::GetJsonRelatedTargets(JsonValueR arrayOut, ECRelationshipClassCP relClass, ECClassCP targetClass, ECInstanceKeyCR source, Utf8CP optionalOrderBy)
    {
    ECClassCP sourceClass = GetECClass(source);

    if (nullptr == sourceClass || nullptr == targetClass || nullptr == relClass || !source.IsValid())
        {
        return ERROR;
        }

    ECSqlSelectBuilder sqlBuilder;
    sqlBuilder
        .Select("t.*")
        .From(*targetClass, "t", false)
        .Join(*sourceClass, "s", false)
        .Using(*relClass, JoinDirection::Reverse)
        .Where("s.ECInstanceId = ?");

    if (nullptr != optionalOrderBy)
        {
        sqlBuilder.OrderBy(optionalOrderBy);
        }

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, sqlBuilder))
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
        ECSqlBuilder::ToECSqlSnippet(*relClass).c_str()
        );

    ECSqlStatement statement;
    if (SUCCESS != PrepareStatement(statement, ecsql))
        {
        return ERROR;
        }

    statement.BindInt64(1, source.GetECClassId());
    statement.BindId(2, source.GetECInstanceId());

    ECSqlStepStatus status;
    while (ECSqlStepStatus::HasRow == (status = statement.Step()))
        {
        ECClassId targetClassId = statement.GetValueId<ECClassId>(0);
        ECInstanceId targetInstanceId = statement.GetValueId<ECInstanceId>(1);
        keysOut.insert({targetClassId, targetInstanceId});
        }

    if (ECSqlStepStatus::Done != status)
        {
        return ERROR;
        }

    return SUCCESS;
    }
