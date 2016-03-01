/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStepTask.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECDb/ECInstanceFinder.h>
#include "EmbeddedECSqlStatement.h"
using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*************************************ECSqlStepTask::Collection********************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         07/2014
//---------------------------------------------------------------------------------------
EmbeddedECSqlStatement* ECSqlStepTask::Collection::GetSelector(bool create) 
    { 
    if (create && m_selector == nullptr)
        m_selector = unique_ptr<EmbeddedECSqlStatement> (new EmbeddedECSqlStatement());

    return m_selector.get(); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
DbResult ECSqlStepTask::Collection::ExecuteBeforeStepTaskList()
    {
    if (IsEmpty() ||  m_selector == nullptr)
        return BE_SQLITE_OK;

    BeAssert(m_selector->GetPreparedStatementP<ECSqlSelectPreparedStatement>() != nullptr && "m_selector statement is null");

    while (BE_SQLITE_ROW == m_selector->Step())
        {
        const ECInstanceId iId = m_selector->GetValue(0).GetId<ECInstanceId>();
        DbResult stat = Execute(ExecutionCategory::ExecuteBeforeParentStep, iId);
        if (BE_SQLITE_OK != stat)
            return stat;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
DbResult ECSqlStepTask::Collection::ExecuteAfterStepTaskList(ECInstanceId const& instanceId)
    {
    if (!IsEmpty())
        {
        DbResult stat = Execute(ExecutionCategory::ExecuteAfterParentStep, instanceId);
        if (BE_SQLITE_OK != stat)
            return stat;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
void ECSqlStepTask::Collection::ResetSelector()
    {
    if (m_selector != nullptr)
        {
        m_selector->Reset();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
void ECSqlStepTask::Collection::Add(std::unique_ptr<ECSqlStepTask> stepTask)
    {
    m_stepTasks.push_back(std::move(stepTask));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
DbResult ECSqlStepTask::Collection::Execute(ExecutionCategory category, ECInstanceId const& instanceId)
    {
    for (std::unique_ptr<ECSqlStepTask>& stepTask : m_stepTasks)
        {
        if (stepTask->GetExecutionCategory() == category)
            {
            DbResult status = stepTask->Execute(instanceId);
            if (BE_SQLITE_OK != status)
                {
                BeAssert(false && "Step failed");
                return status;
                }
            }
        }

    return BE_SQLITE_OK;
    }

//*************************************UpdateStructArrayECSqlStepTask********************************
//***************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
UpdateStructArrayStepTask::UpdateStructArrayStepTask(unique_ptr<InsertStructArrayStepTask>& insertStepTask, unique_ptr<DeleteStructArrayStepTask>& deleteStepTask)
    :ParametericStepTask(ExecutionCategory::ExecuteBeforeParentStep, insertStepTask->GetPropertyMap(), insertStepTask->GetClassMap()), m_insertStepTask(std::move(insertStepTask)), m_deleteStepTask(std::move(deleteStepTask))
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
DbResult UpdateStructArrayStepTask::_Execute(ECInstanceId const& instanceId)
    {
    auto deleteStatus = m_deleteStepTask->Execute(instanceId);
    if (BE_SQLITE_OK != deleteStatus)
        return deleteStatus;

    return m_insertStepTask->Execute(instanceId);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTaskCreateStatus UpdateStructArrayStepTask::Create(unique_ptr<UpdateStructArrayStepTask>& updateStepTask, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, Utf8CP propAccessString)
    {
    unique_ptr<InsertStructArrayStepTask> insertStepTask;
    unique_ptr<DeleteStructArrayStepTask> deleteStepTask;

    ECSqlStepTaskCreateStatus status = InsertStructArrayStepTask::Create(insertStepTask, preparedContext, ecdb, classMap, propAccessString);
    if (status != ECSqlStepTaskCreateStatus::Success)
        return status;

    status = DeleteStructArrayStepTask::Create(deleteStepTask, preparedContext, ecdb, classMap, propAccessString);
    if (status != ECSqlStepTaskCreateStatus::Success)
        return status;

    updateStepTask = unique_ptr<UpdateStructArrayStepTask> (new UpdateStructArrayStepTask(insertStepTask, deleteStepTask));
    return ECSqlStepTaskCreateStatus::Success;
    }

//*************************************InsertStructArrayStepTask********************************
//***************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
InsertStructArrayStepTask::InsertStructArrayStepTask(PropertyMapStructArrayCR propertyMap, IClassMap const& classMap)
: ParametericStepTask(ExecutionCategory::ExecuteAfterParentStep, propertyMap, classMap), m_insertStmt(new EmbeddedECSqlStatement()), m_parameterValue(nullptr), m_propertyPathId(0)
    {}


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
DbResult InsertStructArrayStepTask::_Execute(ECInstanceId const& instanceId)
    {
    ECSqlInsertPreparedStatement* preparedStmt = m_insertStmt->GetPreparedStatementP <ECSqlInsertPreparedStatement >();
    ECSqlParameterValue* structArrayParameterValue = GetParameter();
    if (structArrayParameterValue == nullptr)
        {
        BeAssert(false && "Struct array parameter value is nullptr.");
        return BE_SQLITE_ERROR;
        }
    
    if (structArrayParameterValue->IsNull())
        return BE_SQLITE_OK;
    
    ECClassCP structType = GetPropertyMap().GetProperty().GetAsStructArrayProperty()->GetStructElementType();
    ECDb const& ecdb = preparedStmt->GetECDb();
    IClassMap const* structClassMap = ecdb.GetECDbImplR().GetECDbMap().GetClassMap(*structType);
    PropertyMapCollection const& structMemberPropMaps = structClassMap->GetPropertyMaps();
    int nArrayElementIndex = 0;
   
    for (IECSqlValue const* arrayElement : structArrayParameterValue->GetArray())
        {       
        ECInstanceId generatedECInstanceId;
        DbResult stat = ecdb.GetECDbImplR().GetECInstanceIdSequence().GetNextValue<ECInstanceId>(generatedECInstanceId);
        if (BE_SQLITE_OK != stat)
            return stat;
       
        m_insertStmt->Reset();
        m_insertStmt->ClearBindings();
        m_insertStmt->GetBinder(PARAMETER_ECINSTANCEID).BindId(generatedECInstanceId);
        m_insertStmt->GetBinder(PARAMETER_OWNERECINSTANCEID).BindId(instanceId);
        m_insertStmt->GetBinder(PARAMETER_ECPROPERTYPATHID).BindInt64(m_propertyPathId);
        m_insertStmt->GetBinder(PARAMETER_ECARRAYINDEX).BindInt64(nArrayElementIndex++);   
       
        StructECSqlParameterValue const* structArrayElementValue = static_cast<StructECSqlParameterValue const*> (&arrayElement->GetStruct());

        int parameterIndex = PARAMETER_STRUCTARRAY - 1;
        for (PropertyMap const* propertyMap : structMemberPropMaps)
            {
            if (propertyMap->IsSystemPropertyMap())
                continue;

            parameterIndex++;

            auto& v = structArrayElementValue->GetValue(propertyMap->GetProperty().GetId());
            if (v.IsNull())
                continue;

            ECSqlParameterValue const* value = static_cast<ECSqlParameterValue const*>(&v);
            IECSqlBinder& binder = m_insertStmt->GetBinder(parameterIndex);
            if (value->BindTo(binder) != ECSqlStatus::Success)
                {
                BeAssert(false && "Failed to bind struct array element");
                return BE_SQLITE_ERROR;
                }
            }
            
        preparedStmt->SetECInstanceKeyInfo(ECSqlInsertPreparedStatement::ECInstanceKeyInfo(structClassMap->GetClass().GetId(),
                                                                                            generatedECInstanceId));
        DbResult stepStatus = m_insertStmt->Step();
        if (BE_SQLITE_DONE != stepStatus)
            {
            BeAssert(false && "Failed to insert struct array element");
            return stepStatus;
            }
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTaskCreateStatus InsertStructArrayStepTask::Create(unique_ptr<InsertStructArrayStepTask>& insertStepTask, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, Utf8CP propAccessString)
    {
    PropertyMapCP propertyMap = classMap.GetPropertyMap(propAccessString);
    if (propertyMap == nullptr)
        return ECSqlStepTaskCreateStatus::PropertyNotFound;

    PropertyMapStructArrayCP structArrayPropertyMap = propertyMap->GetAsPropertyMapStructArray();
    if (structArrayPropertyMap == nullptr)
        return ECSqlStepTaskCreateStatus::PropertyNotStructArrayType;

    auto& arrayElementType = structArrayPropertyMap->GetElementType();
    auto arrayElementTypeMap = ecdb.GetECDbImplR().GetECDbMap().GetClassMap(arrayElementType);
    if (!arrayElementTypeMap)
        {
        return ECSqlStepTaskCreateStatus::ArrayElementTypeMapNotFound;
        }

    if (arrayElementTypeMap->GetMapStrategy().IsNotMapped())
        {
        return ECSqlStepTaskCreateStatus::ArrayElementTypeIsUnmapped;
        }

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO %s(%s,%s,%s,%s", arrayElementType.GetECSqlName().c_str(),
                  ECDbSystemSchemaHelper::ECINSTANCEID_PROPNAME, ECDbSystemSchemaHelper::PARENTECINSTANCEID_PROPNAME,
                  ECDbSystemSchemaHelper::ECPROPERTYPATHID_PROPNAME, ECDbSystemSchemaHelper::ECARRAYINDEX_PROPNAME);
    Utf8String ecsqlValuesClause(") VALUES(?,?,?,?");

    for (PropertyMapCP propertyMap : arrayElementTypeMap->GetPropertyMaps())
        {
        if (propertyMap->IsSystemPropertyMap())
            continue;

        ecsql.append(",[").append(propertyMap->GetProperty().GetName()).append("]");
        ecsqlValuesClause.append(",?");
        }

    ecsql.append(ecsqlValuesClause).append(")");

    unique_ptr<InsertStructArrayStepTask> aInsertStepTask = unique_ptr<InsertStructArrayStepTask> (new InsertStructArrayStepTask(*structArrayPropertyMap, classMap));
    aInsertStepTask->GetStatement().Initialize(preparedContext, propertyMap->GetProperty().GetAsArrayProperty(), nullptr);
    ECSqlStatus ecsqlStatus = aInsertStepTask->GetStatement().Prepare(ecdb, ecsql.c_str());
    if (ecsqlStatus != ECSqlStatus::Success)
        return ECSqlStepTaskCreateStatus::ECSqlError;

    aInsertStepTask->m_propertyPathId = structArrayPropertyMap->GetPropertyPathId();

    insertStepTask = std::move(aInsertStepTask);
    return ECSqlStepTaskCreateStatus::Success;
    }

//*************************************DeleteStructArrayStepTask********************************
//***************************************************************************************************


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
DeleteStructArrayStepTask::DeleteStructArrayStepTask(PropertyMapStructArrayCR propertyMap, IClassMap const& classMap)
: ECSqlStepTask(ExecutionCategory::ExecuteBeforeParentStep, propertyMap, classMap), m_deleteStmt(new EmbeddedECSqlStatement())
    {}


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
DbResult DeleteStructArrayStepTask::_Execute(ECInstanceId const& instanceId)
    {
    m_deleteStmt->Reset();
    m_deleteStmt->ClearBindings();
    m_deleteStmt->GetBinder(PARAMETER_OWNERECINSTANCEID).BindId(instanceId);

    const DbResult status = m_deleteStmt->Step();
    return status == BE_SQLITE_DONE ? BE_SQLITE_OK : status;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTaskCreateStatus DeleteStructArrayStepTask::Create(unique_ptr<DeleteStructArrayStepTask>& deleteStepTask, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, Utf8CP propAccessString)
    {
    PropertyMapCP propertyMap = classMap.GetPropertyMap(propAccessString);
    if (propertyMap == nullptr)
        return ECSqlStepTaskCreateStatus::PropertyNotFound;

    PropertyMapStructArrayCP structArrayPropertyMap = propertyMap->GetAsPropertyMapStructArray();
    if (structArrayPropertyMap == nullptr)
        return ECSqlStepTaskCreateStatus::PropertyNotStructArrayType;

    ECClassCR arrayElementType = structArrayPropertyMap->GetElementType();
    ClassMap const* arrayElementTypeMap = ecdb.GetECDbImplR().GetECDbMap().GetClassMap(arrayElementType);
    if (arrayElementTypeMap == nullptr)
        return ECSqlStepTaskCreateStatus::ArrayElementTypeMapNotFound;

    if (arrayElementTypeMap->GetMapStrategy().IsNotMapped())
        return ECSqlStepTaskCreateStatus::ArrayElementTypeIsUnmapped;

    Utf8String ecsql; 
    ecsql.Sprintf("DELETE FROM ONLY [%s].[%s] WHERE " ECDB_COL_ParentECInstanceId "=? AND " 
                  ECDB_COL_ECPropertyPathId "=%llu AND " ECDB_COL_ECArrayIndex " IS NOT NULL", 
                  arrayElementType.GetSchema().GetName().c_str(), arrayElementType.GetName().c_str(), propertyMap->GetPropertyPathId());

    unique_ptr<DeleteStructArrayStepTask> stepTask = unique_ptr<DeleteStructArrayStepTask> (new DeleteStructArrayStepTask(*structArrayPropertyMap, classMap));
    stepTask->GetStatement().Initialize(preparedContext, propertyMap->GetProperty().GetAsArrayProperty(), nullptr);
    ECSqlStatus stat = stepTask->GetStatement().Prepare(ecdb, ecsql.c_str());
    if (!stat.IsSuccess())
        return ECSqlStepTaskCreateStatus::ECSqlError;

    deleteStepTask = std::move(stepTask);
    return ECSqlStepTaskCreateStatus::Success;
    }

//*************************************ECSqlStepTaskFactory*******************************************
//***************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTaskCreateStatus ECSqlStepTaskFactory::CreatePropertyStepTask(std::unique_ptr<ECSqlStepTask>& stepTask, StepTaskType taskType, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, Utf8CP propertyAccessPath)
    {
    switch (taskType)
        {
        case StepTaskType::Insert:
            {
            auto task = std::unique_ptr<InsertStructArrayStepTask> ();
            auto status = InsertStructArrayStepTask::Create(task, preparedContext, ecdb, classMap, propertyAccessPath);
            stepTask = std::move(task);
            return status;
            }
        case StepTaskType::Update:
            {
            auto task = std::unique_ptr<UpdateStructArrayStepTask> ();
            auto status = UpdateStructArrayStepTask::Create(task, preparedContext, ecdb, classMap, propertyAccessPath);
            stepTask = std::move(task);
            return status;
            }
        case StepTaskType::Delete:
            {
            auto task = std::unique_ptr<DeleteStructArrayStepTask> ();
            auto status = DeleteStructArrayStepTask::Create(task, preparedContext, ecdb, classMap, propertyAccessPath);
            stepTask = std::move(task);
            return status;
            }
        }
    BeAssert(false && "Case is not supported");
    return ECSqlStepTaskCreateStatus::NothingToDo;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTaskCreateStatus ECSqlStepTaskFactory::CreateClassStepTask(ECSqlStepTask::Collection& taskList, StepTaskType taskType, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, bool isPolymorphicStatement)
    {
    switch (taskType)
        {
        case StepTaskType::Insert:
            {
            return CreateInsertStepTask(taskList, preparedContext, ecdb, classMap);
            }
        case StepTaskType::Update:
            {
            return CreateUpdateStepTask(taskList, preparedContext, ecdb, classMap, isPolymorphicStatement);
            }
        case StepTaskType::Delete:
            {
            return CreateDeleteStepTask(taskList, preparedContext, ecdb, classMap, isPolymorphicStatement);
            }
        }

    BeAssert(false && "Case is not supported");
    return ECSqlStepTaskCreateStatus::NothingToDo;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
void ECSqlStepTaskFactory::GetSubclasses(ECSqlParseContext::ClassListById& classes, ECClassCR ecClass, ECDbSchemaManagerCR schemaManager)
    {
    for (auto derivedClass : schemaManager.GetDerivedECClasses(const_cast<ECClassR>(ecClass)))
        {
        if (classes.find(derivedClass->GetId()) == classes.end())
            {
            classes[derivedClass->GetId()] = derivedClass;
            GetSubclasses(classes, *derivedClass, schemaManager);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
void ECSqlStepTaskFactory::GetConstraintClasses(ECSqlParseContext::ClassListById& classes, ECRelationshipConstraintCR constraintEnd, ECDbSchemaManagerCR schemaManager, bool* containAnyClass)
    {
    if (containAnyClass)
        *containAnyClass = false;
    for (auto ecClass : constraintEnd.GetClasses())
        {
        if (containAnyClass && !(*containAnyClass) && ecClass->GetName() == "AnyClass" && ecClass->GetSchema().GetName() == "Bentley_Standard_Classes")
            *containAnyClass = true;

        if (classes.find(ecClass->GetId()) == classes.end())
            {
            classes[ecClass->GetId()] = ecClass;
            if (constraintEnd.GetIsPolymorphic())
                GetSubclasses(classes, *ecClass, schemaManager);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTaskCreateStatus ECSqlStepTaskFactory::CreateDeleteStepTask(ECSqlStepTask::Collection& taskList, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, bool isPolymorphicStatement)
    {
    ECSqlStepTaskCreateStatus status = CreateStepTaskList(taskList, preparedContext, StepTaskType::Delete, ecdb, classMap, isPolymorphicStatement);
    if (status != ECSqlStepTaskCreateStatus::Success)
        {
        BeAssert (false && "Failed to create delete step task list");
        return status;
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTaskCreateStatus ECSqlStepTaskFactory::CreateInsertStepTask(ECSqlStepTask::Collection& taskList, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap)
    {
    ECSqlStepTaskCreateStatus status = CreateStepTaskList(taskList, preparedContext, StepTaskType::Insert, ecdb, classMap, false);
    if (status != ECSqlStepTaskCreateStatus::Success)
        {
        BeAssert(false && "Failed to create delete step task list");
        return status;
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTaskCreateStatus ECSqlStepTaskFactory::CreateUpdateStepTask(ECSqlStepTask::Collection& taskList, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, bool isPolymorphicStatement)
    {
    ECSqlStepTaskCreateStatus status = CreateStepTaskList(taskList, preparedContext, StepTaskType::Update, ecdb, classMap, isPolymorphicStatement);
    if (status != ECSqlStepTaskCreateStatus::Success)
        {
        BeAssert(false && "Failed to create delete step task list");
        return status;
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTaskCreateStatus ECSqlStepTaskFactory::CreateStepTaskList(ECSqlStepTask::Collection& taskList, ECSqlPrepareContext& preparedContext, StepTaskType taskType, ECDbR ecdb, IClassMap const& classMap, bool isPolymorphicStatement)
    {
    ECSqlStepTaskCreateStatus status = ECSqlStepTaskCreateStatus::Success;
    bset<ECDbPropertyPathId> ids;
    auto processStructArrayProperties = [&taskList, &preparedContext, &taskType, &ecdb, &classMap, &status, &ids] (TraversalFeedback& feedback, PropertyMapCP propMap)
        {
        if (propMap->GetAsPropertyMapStructArray() == nullptr)
            {
            feedback = TraversalFeedback::Next;
            return;
            }

        ECDbPropertyPathId propPathId = propMap->GetPropertyPathId();
        if (ids.find(propPathId) != ids.end())
            {
            feedback = TraversalFeedback::NextSibling;
            return;
            }

        unique_ptr<ECSqlStepTask> task = nullptr;
        status = ECSqlStepTaskFactory::CreatePropertyStepTask(task, taskType, preparedContext, ecdb, classMap, propMap->GetPropertyAccessString());
        if (status != ECSqlStepTaskCreateStatus::Success)
            {
            BeAssert(false && "Failed to create property step task");
            feedback = TraversalFeedback::Cancel;
            return;
            }

        ids.insert(propPathId);
        taskList.Add(move(task));
        feedback = TraversalFeedback::NextSibling;
        };

    classMap.GetPropertyMaps().Traverse(processStructArrayProperties, false);
    if (!isPolymorphicStatement || status != ECSqlStepTaskCreateStatus::Success)
        return status;

    vector<IClassMap const*> derivedClassMaps = classMap.GetDerivedClassMaps();
    for (IClassMap const* derivedClassMap : derivedClassMaps)
        {
        if (derivedClassMap == nullptr)
            {
            BeAssert(false && "classMap.GetDerivedClassMaps() should't return nullptr derived class maps");
            return ECSqlStepTaskCreateStatus::Error;
            }

        status = CreateStepTaskList(taskList, preparedContext, taskType, ecdb, *derivedClassMap, isPolymorphicStatement);
        if (status != ECSqlStepTaskCreateStatus::Success)
            return status;
        }

    return ECSqlStepTaskCreateStatus::Success;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE