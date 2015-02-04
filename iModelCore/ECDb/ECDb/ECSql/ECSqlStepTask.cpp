/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStepTask.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <BeSQLite/ECDb/ECInstanceFinder.h>
#include "EmbeddedECSqlStatement.h"
using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*************************************ECSqlStepTask::Collection********************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTask::Collection::Collection (ECSqlEventManager& eventManager)
: m_eventManager (eventManager)
    {    
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         07/2014
//---------------------------------------------------------------------------------------
EmbeddedECSqlStatement* ECSqlStepTask::Collection::GetSelector (bool create) 
    { 
    if (create && m_selector == nullptr)
        m_selector = unique_ptr<EmbeddedECSqlStatement> (new EmbeddedECSqlStatement ());

    return m_selector.get (); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepStatus ECSqlStepTask::Collection::ExecuteBeforeStepTaskList ()
    {
    //even if no tasks exist, but if event handlers are registered
    //we need to execute the selector so that event can return the correct instances affected.
    if ((!HasAnyTask () && !m_eventManager.HasEventHandlers ()) ||  m_selector == nullptr)
        return ECSqlStepStatus::Done;

    auto stmt = m_selector->GetPreparedStatementP<ECSqlSelectPreparedStatement> ();
    BeAssert (stmt != nullptr && "m_selector statement is null");
    while (m_selector->Step () == ECSqlStepStatus::HasRow)
        {
        auto iId = stmt->GetValue (0).GetId<ECInstanceId> ();
        m_eventManager.GetEventArgsR ().GetInstanceKeysR ().push_back (ECInstanceKey (stmt->GetValue (1).GetInt64 (), iId));

        if (Execute (ExecutionCategory::ExecuteBeforeParentStep, iId) == ECSqlStepStatus::Error)
            return ECSqlStepStatus::Error;
        }

    return ECSqlStepStatus::Done;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepStatus ECSqlStepTask::Collection::ExecuteAfterStepTaskList (ECInstanceId const& instanceId)
    {
    if (HasAnyTask ())
        {
        if (Execute (ExecutionCategory::ExecuteAfterParentStep, instanceId) == ECSqlStepStatus::Error)
            return ECSqlStepStatus::Error;
        }

    return ECSqlStepStatus::Done;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
void ECSqlStepTask::Collection::ResetSelector ()
    {
    if (m_selector != nullptr)
        {
        m_selector->Reset ();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTask const* ECSqlStepTask::Collection::Find (WCharCP name) const
    {
    auto stepTask = m_stepTasks.find (name);
    if (stepTask != m_stepTasks.end ())
        return stepTask->second.get ();

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
bool ECSqlStepTask::Collection::Add (std::unique_ptr<ECSqlStepTask> stepTask)
    {
    auto name = stepTask->GetName ().c_str ();
    if (Find (name) != nullptr)
        {
        BeAssert (false && "Already have a property with same name");
        return false;
        }

    m_stepTasks[name] = std::move (stepTask);
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
size_t ECSqlStepTask::Collection::Size () const
    {
    return m_stepTasks.size ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
void ECSqlStepTask::Collection::Clear ()
    {
    m_stepTasks.clear ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepStatus ECSqlStepTask::Collection::Execute (ExecutionCategory category, ECInstanceId const& instanceId)
    {
    for (auto& pair : m_stepTasks)
        {
        auto stepTask = pair.second.get ();
        if (stepTask->GetExecutionCategory () == category)
            {
            auto status = stepTask->Execute (instanceId);
            if (status != ECSqlStepStatus::Done)
                {
                BeAssert (false && "Step failed");
                return status;
                }
            }
        }

    return ECSqlStepStatus::Done;
    }

//*************************************UpdateStructArrayECSqlStepTask********************************
//***************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
UpdateStructArrayStepTask::UpdateStructArrayStepTask (ECSqlStatusContext& statusContext, unique_ptr<InsertStructArrayStepTask>& insertStepTask, unique_ptr<DeleteStructArrayStepTask>& deleteStepTask)
    :ParametericStepTask (ExecutionCategory::ExecuteBeforeParentStep, statusContext, insertStepTask->GetPropertyMap (), insertStepTask->GetClassMap()), m_insertStepTask (std::move (insertStepTask)), m_deleteStepTask (std::move (deleteStepTask))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepStatus UpdateStructArrayStepTask::_Execute (ECInstanceId const& instanceId)
    {
    auto deleteStatus = m_deleteStepTask->Execute (instanceId);
    if (deleteStatus != ECSqlStepStatus::Done)
        return deleteStatus;

    return m_insertStepTask->Execute (instanceId);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTaskCreateStatus UpdateStructArrayStepTask::Create (unique_ptr<UpdateStructArrayStepTask>& updateStepTask, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, WCharCP property)
    {
    unique_ptr<InsertStructArrayStepTask> insertStepTask;
    unique_ptr<DeleteStructArrayStepTask> deleteStepTask;

    auto status = InsertStructArrayStepTask::Create (insertStepTask, preparedContext, ecdb, classMap, property);
    if (status != ECSqlStepTaskCreateStatus::Success)
        return status;

    status = DeleteStructArrayStepTask::Create (deleteStepTask, preparedContext, ecdb, classMap, property);
    if (status != ECSqlStepTaskCreateStatus::Success)
        return status;

    updateStepTask = unique_ptr<UpdateStructArrayStepTask> (new UpdateStructArrayStepTask (preparedContext.GetECSqlStatementR().GetStatusContextR(), insertStepTask, deleteStepTask));
    return ECSqlStepTaskCreateStatus::Success;
    }

//*************************************InsertStructArrayStepTask********************************
//***************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
InsertStructArrayStepTask::InsertStructArrayStepTask (ECSqlStatusContext& statusContext, PropertyMapToTableCR property, IClassMap const& classMap)
: ParametericStepTask (ExecutionCategory::ExecuteAfterParentStep, statusContext, property, classMap), m_insertStmt (new EmbeddedECSqlStatement ()), m_parameterValue (nullptr), m_propertyPathId (0)
    {
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepStatus InsertStructArrayStepTask::_Execute (ECInstanceId const& instanceId)
    {
    ECSqlStepStatus stepStatus = ECSqlStepStatus::Error;
    auto preparedStmt = m_insertStmt->GetPreparedStatementP <ECSqlInsertPreparedStatement > ();
    auto structArrayParameterValue = GetParameter ();
    if (structArrayParameterValue == nullptr)
        {
        BeAssert (false && "Struct array parameter value is nullptr.");
        return ECSqlStepStatus::Error;
        }
    
    if (structArrayParameterValue->IsNull ())
        {
        return ECSqlStepStatus::Done;
        }
    
    auto structType = GetPropertyMapR ().GetProperty ().GetAsArrayProperty ()->GetStructElementType ();
    auto& ecdb = preparedStmt->GetECDbR ();
    auto structClassMap = ecdb.GetImplR ().GetECDbMap ().GetClassMap (*structType);
    auto const& structMemberPropMaps = structClassMap->GetPropertyMaps ();
    int nArrayElementIndex = 0;
   
    for (auto arrayElement : structArrayParameterValue->GetArray ())
        {       
        ECInstanceId generatedECInstanceId;
        auto dbStat = ecdb.GetImplR ().GetECInstanceIdSequence ().GetNextValue<ECInstanceId> (generatedECInstanceId);
        if (dbStat != BE_SQLITE_OK)
            {
            //GetStatusContext ().SetStatus (&ecdb, dbStat, true, "InsertStructArrayStepTask::_Execute (ECSqlStepTaskArgs& args) failed: Could not generate an OwnerECInstanceId.");
            return ECSqlStepStatus::Error;
            }
       
        m_insertStmt->Reset ();
        m_insertStmt->ClearBindings ();
        m_insertStmt->GetBinder (PARAMETER_ECINSTANCEID).BindId (generatedECInstanceId);
        m_insertStmt->GetBinder (PARAMETER_OWNERECINSTANCEID).BindId (instanceId);
        m_insertStmt->GetBinder (PARAMETER_ECPROPERTYPATHID).BindInt64 (m_propertyPathId);
        m_insertStmt->GetBinder (PARAMETER_ECARRAYINDEX).BindInt64 (nArrayElementIndex++);   
       
        auto structArrayElementValue = static_cast<StructECSqlParameterValue const*> (&arrayElement->GetStruct ());

        int parameterIndex = PARAMETER_STRUCTARRAY - 1;
        for (auto& propertyMap : structMemberPropMaps)
            {
            if (propertyMap->IsSystemPropertyMap () || propertyMap->IsUnmapped())
                continue;

            parameterIndex++;

            auto& v = structArrayElementValue->GetValue (propertyMap->GetProperty ().GetId ());
            if (v.IsNull ())
                continue;

            auto value = static_cast<ECSqlParameterValue const*>(&v);
            auto& binder = m_insertStmt->GetBinder (parameterIndex);
            if (value->BindTo (binder) != ECSqlStatus::Success)
                {
                BeAssert (false && "Failed to bind struct array element");
                return ECSqlStepStatus::Error;
                }
            }
            
        preparedStmt->SetECInstanceKeyInfo (ECSqlInsertPreparedStatement::ECInstanceKeyInfo (structClassMap->GetClass ().GetId (),
                                                                                            generatedECInstanceId));
        stepStatus = m_insertStmt->Step ();
        if (stepStatus != ECSqlStepStatus::Done)
            {
            BeAssert (false && "Failed to insert struct array element");
            return stepStatus;
            }
        }

    return stepStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTaskCreateStatus InsertStructArrayStepTask::Create (unique_ptr<InsertStructArrayStepTask>& insertStepTask, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, WCharCP property)
    {
    auto propertyMap = classMap.GetPropertyMap (property);
    if (propertyMap == nullptr)
        {
        return ECSqlStepTaskCreateStatus::PropertyNotFound;
        }

    auto structArrayPropertyMap = dynamic_cast<PropertyMapToTableCP>(propertyMap);
    if (structArrayPropertyMap == nullptr)
        {
        return ECSqlStepTaskCreateStatus::PropertyNotStructArrayType;
        }

    auto& arrayElementType = structArrayPropertyMap->GetElementType ();
    auto arrayElementTypeMap = ecdb.GetImplR ().GetECDbMap ().GetClassMap (arrayElementType);
    if (!arrayElementTypeMap)
        {
        return ECSqlStepTaskCreateStatus::ArrayElementTypeMapNotFound;
        }

    if (arrayElementTypeMap->IsUnmapped ())
        {
        return ECSqlStepTaskCreateStatus::ArrayElementTypeIsUnmapped;
        }

    ECSqlInsertBuilder builder;
    builder.InsertInto (arrayElementType);

    //ECSQL_Todo: Change following to constants defines
    builder.AddValue (ECDbSystemSchemaHelper::ECINSTANCEID_PROPNAME, "?");
    builder.AddValue (ECDbSystemSchemaHelper::OWNERECINSTANCEID_PROPNAME, "?");
    builder.AddValue (ECDbSystemSchemaHelper::ECPROPERTYPATHID_PROPNAME, "?");
    builder.AddValue (ECDbSystemSchemaHelper::ECARRAYINDEX_PROPNAME, "?");

    for (auto& propertyMap : arrayElementTypeMap->GetPropertyMaps ())
        {
        if (propertyMap->IsSystemPropertyMap () || propertyMap->IsUnmapped ())
            continue;

        Utf8String propName ("[");
        propName.append (Utf8String (propertyMap->GetProperty ().GetName ())).append ("]");

        builder.AddValue (propName.c_str (), "?");
        }


    auto aInsertStepTask = unique_ptr<InsertStructArrayStepTask> (new InsertStructArrayStepTask (preparedContext.GetECSqlStatementR ().GetStatusContextR (), *structArrayPropertyMap, classMap));
    aInsertStepTask->GetStatement ().Initialize (preparedContext, propertyMap->GetProperty ().GetAsArrayProperty (), nullptr);
    auto ecsqlStatus = aInsertStepTask->GetStatement().Prepare (ecdb, builder.ToString ().c_str ());
    if (ecsqlStatus != ECSqlStatus::Success)
        return ECSqlStepTaskCreateStatus::ECSqlError;

    aInsertStepTask->m_propertyPathId = structArrayPropertyMap->GetPropertyPathId ();

    insertStepTask = std::move (aInsertStepTask);
    return ECSqlStepTaskCreateStatus::Success;
    }

//*************************************DeleteStructArrayStepTask********************************
//***************************************************************************************************


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
DeleteStructArrayStepTask::DeleteStructArrayStepTask (ECSqlStatusContext& statusContext, PropertyMapToTableCR property, IClassMap const& classMap)
: ECSqlPropertyStepTask (ExecutionCategory::ExecuteBeforeParentStep, statusContext, property, classMap), m_deleteStmt (new EmbeddedECSqlStatement ())
    {
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepStatus DeleteStructArrayStepTask::_Execute (ECInstanceId const& instanceId)
    {
    m_deleteStmt->Reset ();
    m_deleteStmt->ClearBindings ();
    m_deleteStmt->GetBinder (PARAMETER_OWNERECINSTANCEID).BindId (instanceId);

    auto status = m_deleteStmt->Step ();
    return status;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTaskCreateStatus DeleteStructArrayStepTask::Create (unique_ptr<DeleteStructArrayStepTask>& deleteStepTask, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, WCharCP property)
    {
    auto propertyMap = classMap.GetPropertyMap (property);
    if (propertyMap == nullptr)
        {
        return ECSqlStepTaskCreateStatus::PropertyNotFound;
        }

    auto structArrayPropertyMap = dynamic_cast<PropertyMapToTableCP>(propertyMap);
    if (structArrayPropertyMap == nullptr)
        {
        return ECSqlStepTaskCreateStatus::PropertyNotStructArrayType;
        }

    auto& arrayElementType = structArrayPropertyMap->GetElementType ();
    auto arrayElementTypeMap = ecdb.GetImplR ().GetECDbMap ().GetClassMap (arrayElementType);
    if (arrayElementTypeMap == nullptr)
        {
        return ECSqlStepTaskCreateStatus::ArrayElementTypeMapNotFound;
        }

    if (arrayElementTypeMap->IsUnmapped ())
        {
        return ECSqlStepTaskCreateStatus::ArrayElementTypeIsUnmapped;
        }

    Utf8String schemaName = Utf8String (arrayElementType.GetSchema ().GetName ().c_str ());
    Utf8String structName = Utf8String (arrayElementType.GetName ().c_str ());
    auto persistenceECPropertyId = propertyMap->GetPropertyPathId ();
 
    Utf8String ecsql; 
    ecsql.Sprintf ("DELETE FROM ONLY [%s].[%s] WHERE OwnerECInstanceId = ? AND ECPropertyPathId = %llu AND ECArrayIndex IS NOT NULL", schemaName.c_str (), structName.c_str (), persistenceECPropertyId);

    unique_ptr<DeleteStructArrayStepTask>  aDeleteStepTask = unique_ptr<DeleteStructArrayStepTask> (new DeleteStructArrayStepTask (preparedContext.GetECSqlStatementR ().GetStatusContextR (), *structArrayPropertyMap, classMap));
    aDeleteStepTask->GetStatement ().Initialize (preparedContext, propertyMap->GetProperty ().GetAsArrayProperty (), nullptr);
    auto ecsqlStatus = aDeleteStepTask->GetStatement().Prepare (ecdb, ecsql.c_str ());
    if (ecsqlStatus != ECSqlStatus::Success)
        return ECSqlStepTaskCreateStatus::ECSqlError;

    deleteStepTask = std::move (aDeleteStepTask);
    return ECSqlStepTaskCreateStatus::Success;
    }


//*************************************DeleteRelatedInstancesECSqlStepTask********************************
//****************************************************************************************************


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
DeleteRelatedInstancesECSqlStepTask::DeleteRelatedInstancesECSqlStepTask (ECDbR ecdb, ECSqlEventManager& eventManager, ECSqlStatusContext& statusContext, WCharCP name, ECClassId classId)
: ECSqlStepTask(ExecutionCategory::ExecuteBeforeParentStep, statusContext, name), m_ecdb(ecdb), m_eventManager(eventManager), m_orphanInstanceFinder(ecdb), m_ecClassId(classId), m_deleteHander(eventManager.GetEventArgsR().GetInstanceKeysR())
    {
    }
//---------------------------------------------------------------------------------------
// @bsimethod                        Muhammad.zaighum                                 02/2014
//---------------------------------------------------------------------------------------
void DeleteRelatedInstancesECSqlStepTask::ECDbStepTaskDeleteHandler::_OnBeforeDelete(ECClassCR ecClass, ECInstanceId ecInstanceId, ECDbR ecDb) 
    {
    ECInstanceKey key = ECInstanceKey(ecClass.GetId(), ecInstanceId);
    m_instanceKeyList.push_back(key);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepStatus DeleteRelatedInstancesECSqlStepTask::_Execute (ECInstanceId const& instanceId)
    {
    ECInstanceIdSet set;
    set.insert (instanceId);
    int deletedCount; 
    if (DeleteDependentInstances (deletedCount, set) != BentleyStatus::SUCCESS)
        return ECSqlStepStatus::Error;
   
    return ECSqlStepStatus::Done;

    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTaskCreateStatus DeleteRelatedInstancesECSqlStepTask::Create (unique_ptr<DeleteRelatedInstancesECSqlStepTask>& deleteStepTask, ECDbR ecdb, ECSqlEventManager& eventManager, ECSqlPrepareContext& preparedContext, IClassMap const& classMap)
    {
    auto aDeleteStepTask = unique_ptr<DeleteRelatedInstancesECSqlStepTask> (new DeleteRelatedInstancesECSqlStepTask (ecdb, eventManager, preparedContext.GetECSqlStatementR ().GetStatusContextR (), L"$DeleteRelatedStepTask", classMap.GetClass ().GetId ()));
    deleteStepTask = std::move (aDeleteStepTask);
    return ECSqlStepTaskCreateStatus::Success;
    }
    

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
BentleyStatus DeleteRelatedInstancesECSqlStepTask::DeleteDependentInstances (int& numDeleted, const ECInstanceIdSet& deletedInstanceIds)
    {
    numDeleted = 0;

    for (ECInstanceId deletedInstanceId : deletedInstanceIds)
        {
        ECInstanceKeyMultiMap orphanedRelationshipInstances;
        ECInstanceKeyMultiMap orphanedInstances;

        BentleyStatus status = FindOrphanedInstances (orphanedRelationshipInstances, orphanedInstances, m_ecClassId, deletedInstanceId, m_ecdb);
        if (status != SUCCESS)
            return status;

        if (m_eventManager.HasEventHandlers ())
            {
            auto& ecInstanceKeyList = m_eventManager.GetEventArgsR().GetInstanceKeysR();
            ecInstanceKeyList.reserve(ecInstanceKeyList.size() + orphanedInstances.size() + orphanedRelationshipInstances.size());
            }

        /*
        * Note: Delete relationship instances first. Deleting instances that hold relationships as
        * FKeys causes the relationship to get deleted automatically, messing up the count of
        * number of items deleted.
        */
        int numRelationshipsDeleted;
        status = DeleteInstances (numRelationshipsDeleted, orphanedRelationshipInstances, m_ecdb);
        if (status != SUCCESS)
            return status;
        numDeleted += numRelationshipsDeleted;

        int numInstancesDeleted;
        status = DeleteInstances (numInstancesDeleted, orphanedInstances, m_ecdb);
        if (status != SUCCESS)
            return status;

        numDeleted += numInstancesDeleted;
        }

    return SUCCESS;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
BentleyStatus DeleteRelatedInstancesECSqlStepTask::DeleteInstances (int& numDeleted, const ECInstanceKeyMultiMap& instanceMap, ECDbR ecDb)
    {
    numDeleted = 0;
    ECInstanceKeyMultiMapConstIterator instanceIdIter;
    for (ECInstanceKeyMultiMapConstIterator classIdIter = instanceMap.begin (); classIdIter != instanceMap.end (); classIdIter = instanceIdIter)
        {
        ECClassId classId = classIdIter->first;
        ECClassCP ecClass = ecDb.GetSchemaManager ().GetECClass (classId);
        if (ecClass == nullptr)
            {
            LOG.errorv ("Failed to get ECClass for ECClassId %x", classId);
            return ERROR;
            }

        ECPersistencePtr persistence = ecDb.GetImplR ().GetECPersistence (*ecClass);
        if (!persistence.IsValid ())
            return ERROR;

        ECInstanceIdSet ecInstanceIdSet;
        bpair<ECInstanceKeyMultiMapConstIterator, ECInstanceKeyMultiMapConstIterator> keyRange = instanceMap.equal_range (classId);
        for (instanceIdIter = keyRange.first; instanceIdIter != keyRange.second; instanceIdIter++)
            ecInstanceIdSet.insert (instanceIdIter->second);

        int numChildDeleted;
        DeleteStatus deleteStatus = persistence->Delete (&numChildDeleted, ecInstanceIdSet, &m_deleteHander);
        if (deleteStatus != DELETE_Success)
            return ERROR;
        numDeleted += numChildDeleted;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
BentleyStatus DeleteRelatedInstancesECSqlStepTask::FindOrphanedInstances
(
ECInstanceKeyMultiMap& orphanedRelationshipInstances,
ECInstanceKeyMultiMap& orphanedInstances,
ECClassId deletedClassId,
ECInstanceId deletedInstanceId,
ECDbR ecDb
)
    {
    ECInstanceKey seedInstanceKey (deletedClassId, deletedInstanceId);

    ECInstanceFinder* instanceFinder = &m_orphanInstanceFinder;

    // Add *all* relationship instances that relate the deleted instance (at either end)
    ECInstanceKeyMultiMap allRelationshipInstances;
    BentleyStatus status = instanceFinder->FindRelatedInstances (nullptr, &allRelationshipInstances, seedInstanceKey, ECInstanceFinder::RelatedDirection_All);
    POSTCONDITION (status == SUCCESS, status);
    orphanedRelationshipInstances.insert (allRelationshipInstances.begin (), allRelationshipInstances.end ());

    // Add embedded children
    ECInstanceKeyMultiMap embeddedChildren;
    status = instanceFinder->FindRelatedInstances (&embeddedChildren, nullptr, seedInstanceKey, ECInstanceFinder::RelatedDirection_EmbeddedChildren);
    POSTCONDITION (status == SUCCESS, status);
    orphanedInstances.insert (embeddedChildren.begin (), embeddedChildren.end ());

    // Add held relationship instances, and child instances (only if they don't have any other parents left)
    ECInstanceKeyMultiMap heldChildren;
    status = instanceFinder->FindRelatedInstances (&heldChildren, nullptr, seedInstanceKey, ECInstanceFinder::RelatedDirection_HeldChildren);
    POSTCONDITION (status == SUCCESS, status);
    for (ECInstanceKeyMultiMapConstIterator iter = heldChildren.begin (); iter != heldChildren.end (); iter++)
        {
        ECClassId relatedClassId = iter->first;
        ECInstanceId relatedInstanceId = iter->second;
        seedInstanceKey = ECInstanceKey (iter->first, iter->second);

        ECInstanceKeyMultiMap holdingParentRelationships;
        status = instanceFinder->FindRelatedInstances (nullptr, &holdingParentRelationships, seedInstanceKey, ECInstanceFinder::RelatedDirection_HoldingParents);
        POSTCONDITION (status == SUCCESS, status);
        if (holdingParentRelationships.size () < 2)
            {
            ECInstanceKeyMultiMapPair mapEntry (relatedClassId, relatedInstanceId);
            orphanedInstances.insert (mapEntry);
            }
        }

    return SUCCESS;
    }

//*************************************ECSqlStepTaskFactory*******************************************
//***************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTaskCreateStatus ECSqlStepTaskFactory::CreatePropertyStepTask (std::unique_ptr<ECSqlStepTask>& stepTask, StepTaskType taskType, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, WCharCP property)
    {
    switch (taskType)
        {
        case StepTaskType::Insert:
            {
            auto task = std::unique_ptr<InsertStructArrayStepTask> ();
            auto status = InsertStructArrayStepTask::Create (task, preparedContext, ecdb, classMap, property);
            stepTask = std::move (task);
            return status;
            }
        case StepTaskType::Update:
            {
            auto task = std::unique_ptr<UpdateStructArrayStepTask> ();
            auto status = UpdateStructArrayStepTask::Create (task, preparedContext, ecdb, classMap, property);
            stepTask = std::move (task);
            return status;
            }
        case StepTaskType::Delete:
            {
            auto task = std::unique_ptr<DeleteStructArrayStepTask> ();
            auto status = DeleteStructArrayStepTask::Create (task, preparedContext, ecdb, classMap, property);
            stepTask = std::move (task);
            return status;
            }
        }
    BeAssert (false && "Case is not supported");
    return ECSqlStepTaskCreateStatus::NothingToDo;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTaskCreateStatus ECSqlStepTaskFactory::CreateClassStepTask (ECSqlStepTask::Collection& taskList, StepTaskType taskType, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap)
    {
    switch (taskType)
        {
        case StepTaskType::Insert:
            {
            return CreateInsertStepTask (taskList, preparedContext, ecdb, classMap);
            }
        case StepTaskType::Update:
            {
            return CreateUpdateStepTask (taskList, preparedContext, ecdb, classMap);
            }
        case StepTaskType::Delete:
            {
            return CreateDeleteStepTask (taskList, preparedContext, ecdb, classMap);
            }
        }

    BeAssert (false && "Case is not supported");
    return ECSqlStepTaskCreateStatus::NothingToDo;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
void ECSqlStepTaskFactory::GetSubclasses (ECSqlParseContext::ClassListById& classes, ECClassCR ecClass, ECDbSchemaManagerCR schemaManager)
    {
    for (auto derivedClass : schemaManager.GetDerivedECClasses (const_cast<ECClassR>(ecClass)))
        {
        if (classes.find (derivedClass->GetId ()) == classes.end ())
            {
            classes[derivedClass->GetId ()] = derivedClass;
            GetSubclasses (classes, *derivedClass, schemaManager);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
void ECSqlStepTaskFactory::GetConstraintClasses (ECSqlParseContext::ClassListById& classes, ECRelationshipConstraintCR constraintEnd, ECDbSchemaManagerCR schemaManager, bool* containAnyClass)
    {
    if (containAnyClass)
        *containAnyClass = false;
    for (auto ecClass : constraintEnd.GetClasses ())
        {
        if (containAnyClass && !(*containAnyClass) && ecClass->GetName () == L"AnyClass" && ecClass->GetSchema ().GetName () == L"Bentley_Standard_Classes")
            *containAnyClass = true;

        if (classes.find (ecClass->GetId ()) == classes.end ())
            {
            classes[ecClass->GetId ()] = ecClass;
            if (constraintEnd.GetIsPolymorphic ())
                GetSubclasses (classes, *ecClass, schemaManager);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTaskCreateStatus ECSqlStepTaskFactory::CreateDeleteStepTask (ECSqlStepTask::Collection& taskList, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap)
    {
    //1. Delete all struct array properties.
    ECSqlStepTaskCreateStatus status = CreateStepTaskList (taskList, preparedContext, StepTaskType::Delete, ecdb, classMap);
    if (status != ECSqlStepTaskCreateStatus::Success)
        {
        BeAssert (false && "Failed to create delete step task list");
        return status;
        }
    //2. Delete all relationships
    //ECSQL_TODO ensure that all relevant relationships are loaded
    unique_ptr<DeleteRelatedInstancesECSqlStepTask> task;
    status = DeleteRelatedInstancesECSqlStepTask::Create (task, ecdb, taskList.GetEventManagerR (), preparedContext, classMap);
    if (status == ECSqlStepTaskCreateStatus::Success)
        taskList.Add (move (task));
    else
        {
        BeAssert (false && "Failed to create relationship step task");
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTaskCreateStatus ECSqlStepTaskFactory::CreateInsertStepTask (ECSqlStepTask::Collection& taskList, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap)
    {
    //1. Delete all struct array properties.
    ECSqlStepTaskCreateStatus status = CreateStepTaskList (taskList, preparedContext, StepTaskType::Insert, ecdb, classMap);
    if (status != ECSqlStepTaskCreateStatus::Success)
        {
        BeAssert (false && "Failed to create delete step task list");
        return status;
        }
    //2. Delete all relationships
    //ECSQL_TODO ensure that all relevant relationships are loaded

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTaskCreateStatus ECSqlStepTaskFactory::CreateUpdateStepTask (ECSqlStepTask::Collection& taskList, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap)
    {
    //1. Delete all struct array properties.
    ECSqlStepTaskCreateStatus status = CreateStepTaskList (taskList, preparedContext, StepTaskType::Update, ecdb, classMap);
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
ECSqlStepTaskCreateStatus ECSqlStepTaskFactory::CreateStepTaskList (ECSqlStepTask::Collection& taskList, ECSqlPrepareContext& preparedContext, StepTaskType taskType, ECDbR ecdb, IClassMap const& classMap)
    {
    //1. Delete all struct array properties.
    ECSqlStepTaskCreateStatus status = ECSqlStepTaskCreateStatus::Success;
    auto processStructArrayProperties = [&] (TraversalFeedback& feedback, PropertyMapCP propMap)
        {
        if (propMap->GetAsPropertyMapToTable ())
            {
            unique_ptr<ECSqlStepTask> task;
            status = ECSqlStepTaskFactory::CreatePropertyStepTask (task, taskType, preparedContext, ecdb, classMap, propMap->GetPropertyAccessString ());
            if (status == ECSqlStepTaskCreateStatus::Success)
                taskList.Add (move (task));
            else
                {
                BeAssert (false && "Failed to create property step task");
                feedback = TraversalFeedback::Cancel;
                }

            feedback = TraversalFeedback::NextSibling;
            }
        };

    classMap.GetPropertyMaps ().Traverse (processStructArrayProperties, false);
    return status;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE