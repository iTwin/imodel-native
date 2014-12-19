/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStepTask.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlStatusContext.h"
#include <BeSQLite/ECDb/IECSqlBinder.h>
#include "ECSqlParameterValue.h"
#include <BeSQLite/ECDb/ECInstanceId.h>
#include <BeSQLite/ECDb/ECSqlStatement.h>
#include "ECSqlEventManager.h"
//#include "EmbeddedECSqlStatement.h"


BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct EmbeddedECSqlStatement;

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2014
//+===============+===============+===============+===============+===============+======
enum class ECSqlStepTaskCreateStatus
    {
    Success, 
    PropertyNotFound, 
    PropertyNotStructArrayType, 
    ArrayElementTypeMapNotFound, 
    ArrayElementTypeIsUnmapped, 
    ECSqlError, 
    NothingToDo,
    Error
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2014
//+===============+===============+===============+===============+===============+======
enum class ExecutionCategory
    {
    ExecuteBeforeParentStep,
    ExecuteAfterParentStep
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2014
//+===============+===============+===============+===============+===============+======
enum class StepTaskType
    {
    Insert,
    Update,
    Delete,
    ///Select
    };

//=======================================================================================
//! ECSqlStepTask is the base class for child action performed by statement
//! @bsiclass                                                Affan.Khan      02/2014
//+===============+===============+===============+===============+===============+======
struct ECSqlStepTask : NonCopyableClass
    {
public:
    struct Collection
        {
    private:
        ECSqlEventManager& m_eventManager;
        std::map<WCharCP, std::unique_ptr<ECSqlStepTask>, CompareWChar> m_stepTasks;
        std::unique_ptr<EmbeddedECSqlStatement> m_selector;
        ECSqlStepStatus Execute (ExecutionCategory category, ECInstanceId const& instanceId);

    public:
        explicit Collection (ECSqlEventManager& eventManager);
        virtual ~Collection (){}
        ECSqlStepStatus ExecuteBeforeStepTaskList ();
        ECSqlStepStatus ExecuteAfterStepTaskList (ECInstanceId const& instanceId);

        ECSqlStepTask const* Find (WCharCP name) const;
        bool HasSelector () const { return m_selector != nullptr;  }
        EmbeddedECSqlStatement* GetSelector (bool create = false);
        bool Add (std::unique_ptr<ECSqlStepTask> stepTask);
        bool Delete (WCharCP name);
        size_t Size () const;
        bool HasAnyTask () const { return !m_stepTasks.empty (); }
        void Clear ();
        void ResetSelector ();

        ECSqlEventManager& GetEventManagerR () const { return m_eventManager; }
        };

private:
    ExecutionCategory m_category;
    ECSqlStatusContext& m_statusContext;
    WString m_name;

    virtual ECSqlStepStatus _Execute (ECInstanceId const& instanceId) = 0;

protected:
    ECSqlStepTask (ExecutionCategory category, ECSqlStatusContext& statusContext, WCharCP name)
        : m_category(category), m_statusContext(statusContext), m_name(name)
        {}

    ECSqlStatusContext& GetStatusContext () { return m_statusContext; }

public:
    virtual ~ECSqlStepTask () {};

    WStringCR GetName () const { return m_name; }
    ExecutionCategory GetExecutionCategory () const { return m_category; }

    ECSqlStepStatus Execute (ECInstanceId const& instanceId)
        {
        return _Execute (instanceId);
        }
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2014
//+===============+===============+===============+===============+===============+======
struct ECSqlStepTaskFactory
    {
private:
    ECSqlStepTaskFactory ();
    ~ECSqlStepTaskFactory ();

    static ECSqlStepTaskCreateStatus CreateDeleteStepTask (ECSqlStepTask::Collection& taskList, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap);
    static ECSqlStepTaskCreateStatus CreateInsertStepTask (ECSqlStepTask::Collection& taskList, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap);
    static ECSqlStepTaskCreateStatus CreateUpdateStepTask (ECSqlStepTask::Collection& taskList, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap);

    static ECSqlStepTaskCreateStatus CreateStepTaskList (ECSqlStepTask::Collection& taskList, ECSqlPrepareContext& preparedContext, StepTaskType taskType, ECDbR ecdb, IClassMap const& classMap);
    static void GetSubclasses (ECSqlParseContext::ClassListById& classes, ECClassCR ecClass, ECDbSchemaManagerCR schemaManager);
    static void GetConstraintClasses (ECSqlParseContext::ClassListById& classes, ECRelationshipConstraintCR constraintEnd, ECDbSchemaManagerCR schemaManager, bool* containAnyClass);

public:
    static ECSqlStepTaskCreateStatus CreatePropertyStepTask (std::unique_ptr<ECSqlStepTask>& stepTask, StepTaskType taskType, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, WCharCP property);
    static ECSqlStepTaskCreateStatus CreateClassStepTask (ECSqlStepTask::Collection& taskList, StepTaskType taskType, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap);
    };

//=======================================================================================
//! ECSqlPropertyStepTask Base class for a task that handle a single property
//! @bsiclass                                                Affan.Khan      02/2014
//+===============+===============+===============+===============+===============+======
struct ECSqlPropertyStepTask : public ECSqlStepTask
    {
private:
    PropertyMapToTableCR m_property;
    IClassMap const& m_classMap;

protected:
    ECSqlPropertyStepTask (ExecutionCategory category, ECSqlStatusContext& statusContext, PropertyMapToTableCR property, IClassMap const& classMap)
        :ECSqlStepTask (category, statusContext, property.GetPropertyAccessString ()), m_property (property), m_classMap (classMap)
        {
        }

public:
    virtual ~ECSqlPropertyStepTask (){}

    PropertyMapToTableCR aInsertStepTaskGetPropertyMap () const { return m_property; }
    IClassMap const& GetClassMap () const { return m_classMap; }
    PropertyMapToTableCR GetPropertyMap () const { return m_property; }
    PropertyMapToTableCR GetPropertyMapR () const { return const_cast<PropertyMapToTableR>(m_property); }
    };

//=======================================================================================
//! ParametericStepTask Accepts a paramter source
//! @bsiclass                                                Affan.Khan      04/2014
//+===============+===============+===============+===============+===============+=====
struct ParametericStepTask : public ECSqlPropertyStepTask
    {
private: 
    virtual void _SetParameterSource (ECSqlParameterValue& parameterSource) = 0;
    virtual ECSqlParameterValue* _GetParameter () = 0;

protected:
    ParametericStepTask (ExecutionCategory category, ECSqlStatusContext& statusContext, PropertyMapToTableCR property, IClassMap const& classMap)
        :ECSqlPropertyStepTask (category, statusContext, property, classMap)
        {}

public:
    virtual ~ParametericStepTask (){}

    void SetParameterSource (ECSqlParameterValue& parameterSource)
        {
        _SetParameterSource (parameterSource);
        }

    ECSqlParameterValue* GetParameter ()
        {
        return  _GetParameter ();
        }
    };

//=======================================================================================
//! InsertStructArrayECSqlStepTask Insert struct array elements
//! @bsiclass                                                Affan.Khan      02/2014
//+===============+===============+===============+===============+===============+======
struct InsertStructArrayStepTask : public ParametericStepTask
    {
private:
    static const int PARAMETER_ECINSTANCEID = 1;
    static const int PARAMETER_OWNERECINSTANCEID = 2;
    static const int PARAMETER_ECPROPERTYID = 3;
    static const int PARAMETER_ECARRAYINDEX = 4;
    static const int PARAMETER_STRUCTARRAY = 5;

    std::unique_ptr<EmbeddedECSqlStatement> m_insertStmt;
    ECSqlParameterValue* m_parameterValue;
    ECPropertyId m_persistenceECPropertyId;
private:
    InsertStructArrayStepTask (ECSqlStatusContext& statusContext, PropertyMapToTableCR property, IClassMap const& classMap);
    virtual ECSqlStepStatus _Execute (ECInstanceId const& instanceId) override;

    virtual void _SetParameterSource (ECSqlParameterValue& parameterSource) override
        {
        m_parameterValue = &parameterSource;
        }
    virtual ECSqlParameterValue* _GetParameter () override
        {
        return m_parameterValue;
        }

public:
    virtual ~InsertStructArrayStepTask (){}

    EmbeddedECSqlStatement& GetStatement () { return *m_insertStmt; }
    static ECSqlStepTaskCreateStatus Create (std::unique_ptr<InsertStructArrayStepTask>& insertStepTask, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, WCharCP property);
    };

//=======================================================================================
//! DeleteStructArrayECSqlStepTask delete struct array elements
//! @bsiclass                                                Affan.Khan      02/2014
//+===============+===============+===============+===============+===============+======
struct DeleteStructArrayStepTask : public ECSqlPropertyStepTask
    {
private:
    static const int PARAMETER_OWNERECINSTANCEID = 1;
    std::unique_ptr<EmbeddedECSqlStatement> m_deleteStmt;

private:
    DeleteStructArrayStepTask (ECSqlStatusContext& statusContext, PropertyMapToTableCR property, IClassMap const& classMap);
    virtual ECSqlStepStatus _Execute (ECInstanceId const& instanceId) override;

    public:
    ~DeleteStructArrayStepTask (){}

    EmbeddedECSqlStatement& GetStatement () { return *m_deleteStmt; }
    static ECSqlStepTaskCreateStatus Create (std::unique_ptr<DeleteStructArrayStepTask>& deleteStepTask, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, WCharCP property);
    };

//=======================================================================================
//! DeleteRelatedInstancesECSqlStepTask delete relationship instances
//! @bsiclass                                                Affan.Khan      02/2014
//+===============+===============+===============+===============+===============+======
struct DeleteRelatedInstancesECSqlStepTask : public ECSqlStepTask
    {
private:
    struct ECDbStepTaskDeleteHandler : ECDbDeleteHandler
        {
        private:
            ECSqlEventArgs::ECInstanceKeyList& m_instanceKeyList;

        private:
            virtual void _OnBeforeDelete(ECClassCR ecClass, ECInstanceId ecInstanceId, ECDbR ecDb) override;

        public:
            ECDbStepTaskDeleteHandler(ECSqlEventArgs::ECInstanceKeyList &owner) :m_instanceKeyList(owner)
                {}
        };
    private:
    ECDbStepTaskDeleteHandler m_deleteHander;
    ECDbR m_ecdb;
    ECSqlEventManager& m_eventManager;
    ECInstanceFinder m_orphanInstanceFinder;
    ECClassId m_ecClassId;

private:
    DeleteRelatedInstancesECSqlStepTask (ECDbR ecdb, ECSqlEventManager& eventManager, ECSqlStatusContext& statusContext, WCharCP name, ECClassId classId);

    BentleyStatus FindOrphanedInstances
        (
        ECInstanceKeyMultiMap& orphanedRelationshipInstances,
        ECInstanceKeyMultiMap& orphanedInstances,
        ECClassId deletedClassId,
        ECInstanceId deletedInstanceId,
        ECDbR ecDb);

    BentleyStatus DeleteDependentInstances (int& numDeleted, const ECInstanceIdSet& deletedInstanceIds);
    BentleyStatus DeleteInstances (int& numDeleted, const ECInstanceKeyMultiMap& instanceMap, ECDbR ecDb);

    virtual ECSqlStepStatus _Execute (ECInstanceId const& instanceId) override;

public:
    ~DeleteRelatedInstancesECSqlStepTask (){}
    static ECSqlStepTaskCreateStatus Create (std::unique_ptr<DeleteRelatedInstancesECSqlStepTask>& deleteStepTask, ECDbR ecdb, ECSqlEventManager& eventManager, ECSqlPrepareContext& preparedContext, IClassMap const& classMap);
    };



//=======================================================================================
//! UpdateStructArrayECSqlStepTask Update struct array element
//! @bsiclass                                                Affan.Khan      02/2014
//+===============+===============+===============+===============+===============+=====
struct UpdateStructArrayStepTask : public ParametericStepTask
    {
private:
    std::unique_ptr<InsertStructArrayStepTask> m_insertStepTask;
    std::unique_ptr<DeleteStructArrayStepTask> m_deleteStepTask;

private:
    UpdateStructArrayStepTask (ECSqlStatusContext& statusContext, std::unique_ptr<InsertStructArrayStepTask>& insertStepTask, std::unique_ptr<DeleteStructArrayStepTask>& deleteStepTask);
    virtual ECSqlStepStatus _Execute (ECInstanceId const& instanceId) override;
    virtual void _SetParameterSource (ECSqlParameterValue& parameterSource) override
        {
        m_insertStepTask->SetParameterSource (parameterSource);
        }
    virtual ECSqlParameterValue* _GetParameter () override
        {
        return m_insertStepTask->GetParameter ();
        }
public:
    ~UpdateStructArrayStepTask (){};

    static ECSqlStepTaskCreateStatus Create (std::unique_ptr<UpdateStructArrayStepTask>& updateStepTask, ECSqlPrepareContext& preparedContext, ECDbR ecdb, IClassMap const& classMap, WCharCP property);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE