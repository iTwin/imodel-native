/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStepTask.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    //even if no tasks exist, but if event handlers are registered
    //we need to execute the selector so that event can return the correct instances affected.
    if ((!HasAnyTask() ) ||  m_selector == nullptr)
        return BE_SQLITE_OK;

    auto stmt = m_selector->GetPreparedStatementP<ECSqlSelectPreparedStatement> ();
    BeAssert(stmt != nullptr && "m_selector statement is null");
    while (BE_SQLITE_ROW == m_selector->Step())
        {
        ECInstanceId iId = stmt->GetValue(0).GetId<ECInstanceId>();
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
    if (HasAnyTask())
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
ECSqlStepTask const* ECSqlStepTask::Collection::Find(Utf8CP name) const
    {
    auto stepTask = m_stepTasks.find(name);
    if (stepTask != m_stepTasks.end())
        return stepTask->second.get();

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
bool ECSqlStepTask::Collection::Add(std::unique_ptr<ECSqlStepTask> stepTask)
    {
    auto name = stepTask->GetName().c_str();
    if (Find(name) != nullptr)
        {
        BeAssert(false && "Already have a property with same name");
        return false;
        }

    m_stepTasks[name] = std::move(stepTask);
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
size_t ECSqlStepTask::Collection::Size() const
    {
    return m_stepTasks.size();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
void ECSqlStepTask::Collection::Clear()
    {
    m_stepTasks.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
DbResult ECSqlStepTask::Collection::Execute(ExecutionCategory category, ECInstanceId const& instanceId)
    {
    for (auto& pair : m_stepTasks)
        {
        auto stepTask = pair.second.get();
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
ECSqlStepTaskCreateStatus UpdateStructArrayStepTask::Create(unique_ptr<UpdateStructArrayStepTask>& updateStepTask, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, Utf8CP property)
    {
    unique_ptr<InsertStructArrayStepTask> insertStepTask;
    unique_ptr<DeleteStructArrayStepTask> deleteStepTask;

    auto status = InsertStructArrayStepTask::Create(insertStepTask, preparedContext, ecdb, classMap, property);
    if (status != ECSqlStepTaskCreateStatus::Success)
        return status;

    status = DeleteStructArrayStepTask::Create(deleteStepTask, preparedContext, ecdb, classMap, property);
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
InsertStructArrayStepTask::InsertStructArrayStepTask(PropertyMapToTableCR property, IClassMap const& classMap)
: ParametericStepTask(ExecutionCategory::ExecuteAfterParentStep, property, classMap), m_insertStmt(new EmbeddedECSqlStatement()), m_parameterValue(nullptr), m_propertyPathId(0)
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
    
    ECClassCP structType = GetPropertyMapR ().GetProperty().GetAsArrayProperty()->GetStructElementType();
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
            if (propertyMap->IsSystemPropertyMap() || propertyMap->IsUnmapped())
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
ECSqlStepTaskCreateStatus InsertStructArrayStepTask::Create(unique_ptr<InsertStructArrayStepTask>& insertStepTask, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, Utf8CP property)
    {
    auto propertyMap = classMap.GetPropertyMap(property);
    if (propertyMap == nullptr)
        {
        return ECSqlStepTaskCreateStatus::PropertyNotFound;
        }

    auto structArrayPropertyMap = dynamic_cast<PropertyMapToTableCP>(propertyMap);
    if (structArrayPropertyMap == nullptr)
        {
        return ECSqlStepTaskCreateStatus::PropertyNotStructArrayType;
        }

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

    ECSqlInsertBuilder builder;
    builder.InsertInto(arrayElementType);

    //ECSQL_Todo: Change following to constants defines
    builder.AddValue(ECDbSystemSchemaHelper::ECINSTANCEID_PROPNAME, "?");
    builder.AddValue(ECDbSystemSchemaHelper::PARENTECINSTANCEID_PROPNAME, "?");
    builder.AddValue(ECDbSystemSchemaHelper::ECPROPERTYPATHID_PROPNAME, "?");
    builder.AddValue(ECDbSystemSchemaHelper::ECARRAYINDEX_PROPNAME, "?");

    for (auto& propertyMap : arrayElementTypeMap->GetPropertyMaps())
        {
        if (propertyMap->IsSystemPropertyMap() || propertyMap->IsUnmapped())
            continue;

        Utf8String propName("[");
        propName.append(Utf8String(propertyMap->GetProperty().GetName())).append("]");

        builder.AddValue(propName.c_str(), "?");
        }


    auto aInsertStepTask = unique_ptr<InsertStructArrayStepTask> (new InsertStructArrayStepTask(*structArrayPropertyMap, classMap));
    aInsertStepTask->GetStatement().Initialize(preparedContext, propertyMap->GetProperty().GetAsArrayProperty(), nullptr);
    auto ecsqlStatus = aInsertStepTask->GetStatement().Prepare(ecdb, builder.ToString().c_str());
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
DeleteStructArrayStepTask::DeleteStructArrayStepTask(PropertyMapToTableCR property, IClassMap const& classMap)
: ECSqlPropertyStepTask(ExecutionCategory::ExecuteBeforeParentStep, property, classMap), m_deleteStmt(new EmbeddedECSqlStatement())
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
ECSqlStepTaskCreateStatus DeleteStructArrayStepTask::Create(unique_ptr<DeleteStructArrayStepTask>& deleteStepTask, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, Utf8CP property)
    {
    auto propertyMap = classMap.GetPropertyMap(property);
    if (propertyMap == nullptr)
        {
        return ECSqlStepTaskCreateStatus::PropertyNotFound;
        }

    auto structArrayPropertyMap = dynamic_cast<PropertyMapToTableCP>(propertyMap);
    if (structArrayPropertyMap == nullptr)
        {
        return ECSqlStepTaskCreateStatus::PropertyNotStructArrayType;
        }

    auto& arrayElementType = structArrayPropertyMap->GetElementType();
    auto arrayElementTypeMap = ecdb.GetECDbImplR().GetECDbMap().GetClassMap(arrayElementType);
    if (arrayElementTypeMap == nullptr)
        {
        return ECSqlStepTaskCreateStatus::ArrayElementTypeMapNotFound;
        }

    if (arrayElementTypeMap->GetMapStrategy().IsNotMapped())
        {
        return ECSqlStepTaskCreateStatus::ArrayElementTypeIsUnmapped;
        }

    Utf8String schemaName = Utf8String(arrayElementType.GetSchema().GetName().c_str());
    Utf8String structName = Utf8String(arrayElementType.GetName().c_str());
    auto persistenceECPropertyId = propertyMap->GetPropertyPathId();
 
    Utf8String ecsql; 
    ecsql.Sprintf("DELETE FROM ONLY [%s].[%s] WHERE " ECDB_COL_ParentECInstanceId " = ? AND " ECDB_COL_ECPropertyPathId " = %llu AND " ECDB_COL_ECArrayIndex " IS NOT NULL", schemaName.c_str(), structName.c_str(), persistenceECPropertyId);

    unique_ptr<DeleteStructArrayStepTask>  aDeleteStepTask = unique_ptr<DeleteStructArrayStepTask> (new DeleteStructArrayStepTask(*structArrayPropertyMap, classMap));
    aDeleteStepTask->GetStatement().Initialize(preparedContext, propertyMap->GetProperty().GetAsArrayProperty(), nullptr);
    auto ecsqlStatus = aDeleteStepTask->GetStatement().Prepare(ecdb, ecsql.c_str());
    if (ecsqlStatus != ECSqlStatus::Success)
        return ECSqlStepTaskCreateStatus::ECSqlError;

    deleteStepTask = std::move(aDeleteStepTask);
    return ECSqlStepTaskCreateStatus::Success;
    }

//*************************************ECSqlStepTaskFactory*******************************************
//***************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTaskCreateStatus ECSqlStepTaskFactory::CreatePropertyStepTask(std::unique_ptr<ECSqlStepTask>& stepTask, StepTaskType taskType, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, Utf8CP property)
    {
    switch (taskType)
        {
        case StepTaskType::Insert:
            {
            auto task = std::unique_ptr<InsertStructArrayStepTask> ();
            auto status = InsertStructArrayStepTask::Create(task, preparedContext, ecdb, classMap, property);
            stepTask = std::move(task);
            return status;
            }
        case StepTaskType::Update:
            {
            auto task = std::unique_ptr<UpdateStructArrayStepTask> ();
            auto status = UpdateStructArrayStepTask::Create(task, preparedContext, ecdb, classMap, property);
            stepTask = std::move(task);
            return status;
            }
        case StepTaskType::Delete:
            {
            auto task = std::unique_ptr<DeleteStructArrayStepTask> ();
            auto status = DeleteStructArrayStepTask::Create(task, preparedContext, ecdb, classMap, property);
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
    auto processStructArrayProperties = [&] (TraversalFeedback& feedback, PropertyMapCP propMap)
        {
        if (propMap->GetAsPropertyMapToTable())
            {
            unique_ptr<ECSqlStepTask> task;
            status = ECSqlStepTaskFactory::CreatePropertyStepTask(task, taskType, preparedContext, ecdb, classMap, propMap->GetPropertyAccessString());
            if (status == ECSqlStepTaskCreateStatus::Success)
                taskList.Add(move(task));
            else
                {
                BeAssert(false && "Failed to create property step task");
                feedback = TraversalFeedback::Cancel;
                }

            feedback = TraversalFeedback::NextSibling;
            }
        };

    classMap.GetPropertyMaps().Traverse(processStructArrayProperties, false);
    return status;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE