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
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTask::Collection::Collection ()

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
    if ((!HasAnyTask () ) ||  m_selector == nullptr)
        return ECSqlStepStatus::Done;

    auto stmt = m_selector->GetPreparedStatementP<ECSqlSelectPreparedStatement> ();
    BeAssert (stmt != nullptr && "m_selector statement is null");
    while (m_selector->Step () == ECSqlStepStatus::HasRow)
        {
        auto iId = stmt->GetValue (0).GetId<ECInstanceId> ();
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
    {}


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepStatus InsertStructArrayStepTask::_Execute (ECInstanceId const& instanceId)
    {
    ECSqlStepStatus stepStatus = ECSqlStepStatus::Error;
    ECSqlInsertPreparedStatement* preparedStmt = m_insertStmt->GetPreparedStatementP <ECSqlInsertPreparedStatement >();
    ECSqlParameterValue* structArrayParameterValue = GetParameter ();
    if (structArrayParameterValue == nullptr)
        {
        BeAssert (false && "Struct array parameter value is nullptr.");
        return ECSqlStepStatus::Error;
        }
    
    if (structArrayParameterValue->IsNull ())
        return ECSqlStepStatus::Done;
    
    ECClassCP structType = GetPropertyMapR ().GetProperty ().GetAsArrayProperty ()->GetStructElementType ();
    ECDb const& ecdb = preparedStmt->GetECDb ();
    IClassMap const* structClassMap = ecdb.GetECDbImplR().GetECDbMap ().GetClassMap (*structType);
    PropertyMapCollection const& structMemberPropMaps = structClassMap->GetPropertyMaps ();
    int nArrayElementIndex = 0;
   
    for (IECSqlValue const* arrayElement : structArrayParameterValue->GetArray ())
        {       
        ECInstanceId generatedECInstanceId;
        if (BE_SQLITE_OK != ecdb.GetECDbImplR().GetECInstanceIdSequence ().GetNextValue<ECInstanceId> (generatedECInstanceId))
            return ECSqlStepStatus::Error;
       
        m_insertStmt->Reset ();
        m_insertStmt->ClearBindings ();
        m_insertStmt->GetBinder (PARAMETER_ECINSTANCEID).BindId (generatedECInstanceId);
        m_insertStmt->GetBinder (PARAMETER_OWNERECINSTANCEID).BindId (instanceId);
        m_insertStmt->GetBinder (PARAMETER_ECPROPERTYPATHID).BindInt64 (m_propertyPathId);
        m_insertStmt->GetBinder (PARAMETER_ECARRAYINDEX).BindInt64 (nArrayElementIndex++);   
       
        StructECSqlParameterValue const* structArrayElementValue = static_cast<StructECSqlParameterValue const*> (&arrayElement->GetStruct());

        int parameterIndex = PARAMETER_STRUCTARRAY - 1;
        for (PropertyMap const* propertyMap : structMemberPropMaps)
            {
            if (propertyMap->IsSystemPropertyMap () || propertyMap->IsUnmapped())
                continue;

            parameterIndex++;

            auto& v = structArrayElementValue->GetValue (propertyMap->GetProperty ().GetId ());
            if (v.IsNull ())
                continue;

            ECSqlParameterValue const* value = static_cast<ECSqlParameterValue const*>(&v);
            IECSqlBinder& binder = m_insertStmt->GetBinder (parameterIndex);
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
    auto arrayElementTypeMap = ecdb.GetECDbImplR().GetECDbMap ().GetClassMap (arrayElementType);
    if (!arrayElementTypeMap)
        {
        return ECSqlStepTaskCreateStatus::ArrayElementTypeMapNotFound;
        }

    if (arrayElementTypeMap->GetMapStrategy().IsNotMapped())
        {
        return ECSqlStepTaskCreateStatus::ArrayElementTypeIsUnmapped;
        }

    ECSqlInsertBuilder builder;
    builder.InsertInto (arrayElementType);

    //ECSQL_Todo: Change following to constants defines
    builder.AddValue (ECDbSystemSchemaHelper::ECINSTANCEID_PROPNAME, "?");
    builder.AddValue (ECDbSystemSchemaHelper::PARENTECINSTANCEID_PROPNAME, "?");
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
    auto arrayElementTypeMap = ecdb.GetECDbImplR().GetECDbMap ().GetClassMap (arrayElementType);
    if (arrayElementTypeMap == nullptr)
        {
        return ECSqlStepTaskCreateStatus::ArrayElementTypeMapNotFound;
        }

    if (arrayElementTypeMap->GetMapStrategy().IsNotMapped ())
        {
        return ECSqlStepTaskCreateStatus::ArrayElementTypeIsUnmapped;
        }

    Utf8String schemaName = Utf8String (arrayElementType.GetSchema ().GetName ().c_str ());
    Utf8String structName = Utf8String (arrayElementType.GetName ().c_str ());
    auto persistenceECPropertyId = propertyMap->GetPropertyPathId ();
 
    Utf8String ecsql; 
    ecsql.Sprintf ("DELETE FROM ONLY [%s].[%s] WHERE " ECDB_COL_ParentECInstanceId " = ? AND " ECDB_COL_ECPropertyPathId " = %llu AND " ECDB_COL_ECArrayIndex " IS NOT NULL", schemaName.c_str (), structName.c_str (), persistenceECPropertyId);

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
DeleteRelatedInstancesECSqlStepTask::DeleteRelatedInstancesECSqlStepTask (ECDbR ecdb, ECSqlStatusContext& statusContext, WCharCP name, ECClassId classId)
    : ECSqlStepTask (ExecutionCategory::ExecuteBeforeParentStep, statusContext, name), m_ecdb (ecdb), m_orphanInstanceFinder (ecdb), m_ecClassId (classId)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepStatus DeleteRelatedInstancesECSqlStepTask::_Execute (ECInstanceId const& seedInstanceId)
    {
    if (DeleteDependentInstances (seedInstanceId) != BentleyStatus::SUCCESS)
        return ECSqlStepStatus::Error;
   
    return ECSqlStepStatus::Done;

    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
ECSqlStepTaskCreateStatus DeleteRelatedInstancesECSqlStepTask::Create (unique_ptr<DeleteRelatedInstancesECSqlStepTask>& deleteStepTask, ECDbR ecdb, ECSqlPrepareContext& preparedContext, IClassMap const& classMap)
    {
    deleteStepTask = unique_ptr<DeleteRelatedInstancesECSqlStepTask> (new DeleteRelatedInstancesECSqlStepTask (ecdb, preparedContext.GetECSqlStatementR ().GetStatusContextR (), L"$DeleteRelatedStepTask", classMap.GetClass ().GetId ()));
    return ECSqlStepTaskCreateStatus::Success;
    }
    

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
BentleyStatus DeleteRelatedInstancesECSqlStepTask::DeleteDependentInstances (ECInstanceId const& seedInstanceId) const
    {
    ECInstanceKeyMultiMap orphanedRelationshipInstances;
    ECInstanceKeyMultiMap orphanedInstances;

    BentleyStatus status = FindOrphanedInstances (orphanedRelationshipInstances, orphanedInstances, m_ecdb, m_ecClassId, seedInstanceId);
    if (status != SUCCESS)
        return status;



    status = DeleteInstances (m_ecdb, orphanedRelationshipInstances);
    if (status != SUCCESS)
        return status;

    status = DeleteInstances (m_ecdb, orphanedInstances);
    if (status != SUCCESS)
        return status;


    return SUCCESS;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                 Affan.Khan         02/2014
//---------------------------------------------------------------------------------------
BentleyStatus DeleteRelatedInstancesECSqlStepTask::DeleteInstances (ECDbR ecdb, ECInstanceKeyMultiMap const& candidateKeyMap) const
    {
    if (candidateKeyMap.empty ())
        return SUCCESS;

    std::vector<ECInstanceKey> keyList;
    ECClassId previousClassId = -1LL;
    bool isFirstItem = true;
    for (auto const& kvPair : candidateKeyMap)
        {
        //keys are ordered by class id. Do bulk delete per class, so gather all keys for the current class first before deleting.
        ECClassId classId = kvPair.first;
        if (isFirstItem)
            previousClassId = classId;

        isFirstItem = false;

        if (classId == previousClassId)
            {
            keyList.push_back (ECInstanceKey (classId, kvPair.second));
            continue;
            }

        //Sequence of keys with same class id has ended. So delete now
        if (SUCCESS != DeleteInstances (ecdb, previousClassId, keyList))
            return ERROR;

        keyList.clear ();
        keyList.push_back (ECInstanceKey (classId, kvPair.second));
        previousClassId = classId;
        }

    //Last sequence keys with same class id needs to be deleted now.
    if (!keyList.empty ())
        return DeleteInstances (ecdb, previousClassId, keyList);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle         01/2015
//---------------------------------------------------------------------------------------
BentleyStatus DeleteRelatedInstancesECSqlStepTask::DeleteInstances (ECDbR ecDb, ECClassId classId, std::vector<ECInstanceKey> const& keyList) const
    {
    ECClassCP ecClass = ecDb.Schemas ().GetECClass (classId);
    if (ecClass == nullptr)
        {
        LOG.errorv ("ECSQL cascade delete of related instances with ECClassId %lld failed: Could not retrieve ECClass for the ECClassId.", classId);
        return ERROR;
        }

    //if this is a relationship class with end table mapping and cardinality on other end is 1:1, we don't need to delete
    //it as it gets implicitly deleted with the end instance.
    //This prevents us from running into null constraint violations if the foreign key column is not nullable
    //(See TFS#168619)
    auto relationshipClass = ecClass->GetRelationshipClassCP ();
    if (relationshipClass != nullptr)
        {
        auto classMap = ecDb.GetECDbImplR().GetECDbMap ().GetClassMapCP (*relationshipClass);
        if (classMap == nullptr)
            {
            LOG.errorv (L"ECSQL cascade delete of related %ls instances failed: Cound not retrieve class map.", relationshipClass->GetFullName ());
            return ERROR;
            }

        if (classMap->GetClassMapType () == IClassMap::Type::RelationshipEndTable)
            {
            BeAssert (dynamic_cast<RelationshipClassEndTableMapCP> (classMap) != nullptr);
            auto relClassMap = static_cast<RelationshipClassEndTableMapCP> (classMap);
            auto const& otherEndConstraint = relClassMap->GetThisEnd () == ECRelationshipEnd_Source ? relationshipClass->GetTarget () : relationshipClass->GetSource ();
            if (otherEndConstraint.GetCardinality ().GetLowerLimit () == 1)
                {

                return SUCCESS;
                }
            }
        }

    auto statement = GetDeleteStatement (*ecClass);
    if (statement == nullptr)
        return ERROR; //error logging already done in GetDeleteStatement

    //in order to reuse the prepared statement, the number of parameters in the ECSQL must be constant
    //We therefore split the delete in chunks of MAX_PARAMETER_COUNT instance ids
    const int keyCount = (int) keyList.size ();
    for (int i = 0; i < keyCount; i++)
        {
        const int parameterIndex = (i % MAX_PARAMETER_COUNT) + 1;

        if (ECSqlStatus::Success != statement->BindId (parameterIndex, keyList[i].GetECInstanceId ()))
            {
            LOG.errorv (L"ECSQL cascade delete of related %ls instances failed: Binding to nested ECSQL DELETE failed.", ecClass->GetFullName ());
            return ERROR;
            }

        //once the chunk of MAX_PARAMETER_COUNT keys has been bound (or the last key has been reached),
        //we execute the statement
        const bool isLastKey = (i + 1) == keyCount;
        if (parameterIndex == MAX_PARAMETER_COUNT || isLastKey)
            {
            if (ECSqlStepStatus::Done != statement->Step ())
                {
                LOG.errorv (L"ECSQL cascade delete of related %ls instances failed: Execution of nested ECSQL DELETE failed.", ecClass->GetFullName ());
                return ERROR;
                }

            //no need to reset the statement if this is the last execution
            if (!isLastKey)
                {
                statement->ClearBindings ();
                statement->Reset ();
                }
            }
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
ECDbR ecdb,
ECClassId deletedClassId,
ECInstanceId seedInstanceId
) const
    {
    ECInstanceKey seedInstanceKey (deletedClassId, seedInstanceId);

    // Add relationship instances that that point to the deleted instance
    // except for embedded relationships pointing to embedded children (i.e. where this instance is the parent), 
    // because deleting the embedded related instances
    // will find and delete those relationships themselves.
    if (SUCCESS != m_orphanInstanceFinder.FindRelatedInstances (nullptr, &orphanedRelationshipInstances, seedInstanceKey, ECInstanceFinder::RelatedDirection::RelatedDirection_EmbeddingParent |
        ECInstanceFinder::RelatedDirection::RelatedDirection_HeldChildren | ECInstanceFinder::RelatedDirection::RelatedDirection_HoldingParents | ECInstanceFinder::RelatedDirection::RelatedDirection_Referencing))
        return ERROR;

    // Add embedded children
    if (SUCCESS != m_orphanInstanceFinder.FindRelatedInstances (&orphanedInstances, nullptr, seedInstanceKey, ECInstanceFinder::RelatedDirection_EmbeddedChildren))
        return ERROR;

    // Add held child instances (only if they don't have any other parents left)
    // Step 1: first find all held child instances
    ECInstanceKeyMultiMap heldChildren;
    if (SUCCESS != m_orphanInstanceFinder.FindRelatedInstances (&heldChildren, nullptr, seedInstanceKey, ECInstanceFinder::RelatedDirection_HeldChildren))
        return ERROR;

    // Step 2: only add those held instances for who this instance is the only parent. Which means they will 
    // be deleted if this instance is deleted
    for (ECInstanceKeyMultiMapConstIterator iter = heldChildren.begin (); iter != heldChildren.end (); iter++)
        {
        ECClassId relatedClassId = iter->first;
        ECInstanceId relatedInstanceId = iter->second;
        seedInstanceKey = ECInstanceKey (iter->first, iter->second);

        ECInstanceKeyMultiMap holdingParentRelationships;
        if (SUCCESS != m_orphanInstanceFinder.FindRelatedInstances (nullptr, &holdingParentRelationships, seedInstanceKey, ECInstanceFinder::RelatedDirection_HoldingParents))
            return ERROR;

        if (holdingParentRelationships.size () < 2)
            {
            ECInstanceKeyMultiMapPair mapEntry (relatedClassId, relatedInstanceId);
            orphanedInstances.insert (mapEntry);
            }
        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle         01/2015
//---------------------------------------------------------------------------------------
ECSqlStatement* DeleteRelatedInstancesECSqlStepTask::GetDeleteStatement (ECN::ECClassCR ecClass) const
    {
    auto it = m_statementCache.find (ecClass.GetId ());
    if (it == m_statementCache.end ())
        {
        Utf8String ecsql ("DELETE FROM ONLY ");
        ecsql.append (ECSqlBuilder::ToECSqlSnippet (ecClass)).append (" WHERE ECInstanceId IN (");
        bool isFirstParameter = true;
        for (int i = 0; i < MAX_PARAMETER_COUNT; i++)
            {
            if (!isFirstParameter)
                ecsql.append (",");

            ecsql.append ("?");
            isFirstParameter = false;
            }
        ecsql.append (")");

        auto statement = std::unique_ptr<ECSqlStatement> (new ECSqlStatement ());
        if (statement->Prepare (m_ecdb, ecsql.c_str ()) != ECSqlStatus::Success)
            {
            LOG.errorv (L"ECSQL cascade delete of related %ls instances failed: Preparation of nested ECSQL DELETE failed.", ecClass.GetFullName ());
            return nullptr;
            }

        auto statementP = statement.get ();

        m_statementCache[ecClass.GetId ()] = std::move (statement);
        return statementP;
        }

    auto statement = it->second.get ();
    BeAssert (statement != nullptr);

    statement->Reset ();
    statement->ClearBindings ();
    return statement;
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
ECSqlStepTaskCreateStatus ECSqlStepTaskFactory::CreateClassStepTask (ECSqlStepTask::Collection& taskList, StepTaskType taskType, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, bool isPolymorphicStatement)
    {
    switch (taskType)
        {
        case StepTaskType::Insert:
            {
            return CreateInsertStepTask (taskList, preparedContext, ecdb, classMap);
            }
        case StepTaskType::Update:
            {
            return CreateUpdateStepTask(taskList, preparedContext, ecdb, classMap, isPolymorphicStatement);
            }
        case StepTaskType::Delete:
            {
            return CreateDeleteStepTask (taskList, preparedContext, ecdb, classMap, isPolymorphicStatement);
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
ECSqlStepTaskCreateStatus ECSqlStepTaskFactory::CreateDeleteStepTask(ECSqlStepTask::Collection& taskList, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, bool isPolymorphicStatement)
    {
    //1. Delete all struct array properties.
    ECSqlStepTaskCreateStatus status = CreateStepTaskList(taskList, preparedContext, StepTaskType::Delete, ecdb, classMap, isPolymorphicStatement);
    if (status != ECSqlStepTaskCreateStatus::Success)
        {
        BeAssert (false && "Failed to create delete step task list");
        return status;
        }
    //2. Delete all relationships
    //ECSQL_TODO ensure that all relevant relationships are loaded
    unique_ptr<DeleteRelatedInstancesECSqlStepTask> task;
    status = DeleteRelatedInstancesECSqlStepTask::Create (task, ecdb, preparedContext, classMap);
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
    ECSqlStepTaskCreateStatus status = CreateStepTaskList (taskList, preparedContext, StepTaskType::Insert, ecdb, classMap, false);
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
ECSqlStepTaskCreateStatus ECSqlStepTaskFactory::CreateUpdateStepTask(ECSqlStepTask::Collection& taskList, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, bool isPolymorphicStatement)
    {
    ECSqlStepTaskCreateStatus status = CreateStepTaskList (taskList, preparedContext, StepTaskType::Update, ecdb, classMap, isPolymorphicStatement);
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
ECSqlStepTaskCreateStatus ECSqlStepTaskFactory::CreateStepTaskList(ECSqlStepTask::Collection& taskList, ECSqlPrepareContext& preparedContext, StepTaskType taskType, ECDbR ecdb, IClassMap const& classMap, bool isPolymorphicStatement)
    {
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