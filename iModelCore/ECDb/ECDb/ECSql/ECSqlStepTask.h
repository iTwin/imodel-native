/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStepTask.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECDb/IECSqlBinder.h>
#include "ECSqlParameterValue.h"
#include <ECDb/ECInstanceId.h>
#include <ECDb/ECSqlStatement.h>

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
            std::vector<std::unique_ptr<ECSqlStepTask>> m_stepTasks;
            std::unique_ptr<EmbeddedECSqlStatement> m_selector;
            DbResult Execute(ExecutionCategory, ECInstanceId const&);

        public:
            Collection() {}
            virtual ~Collection() {}
            DbResult ExecuteBeforeStepTaskList();
            DbResult ExecuteAfterStepTaskList(ECInstanceId const&);

            bool HasSelector() const { return m_selector != nullptr; }
            EmbeddedECSqlStatement* GetSelector(bool create = false);
            void Add(std::unique_ptr<ECSqlStepTask>);
            size_t Size() const { return m_stepTasks.size(); }
            bool IsEmpty() const { return m_stepTasks.empty(); }
            void Clear() { m_stepTasks.clear(); }
            void ResetSelector();
        };

private:
    ExecutionCategory m_category;
    PropertyMapStructArrayCR m_propertyMap;
    IClassMap const& m_classMap;

    virtual DbResult _Execute(ECInstanceId const& instanceId) = 0;

protected:
    ECSqlStepTask(ExecutionCategory category, PropertyMapStructArrayCR propertyMap, IClassMap const& classMap)
        : m_category(category), m_propertyMap(propertyMap), m_classMap(classMap) {}

public:
    virtual ~ECSqlStepTask() {};

    ExecutionCategory GetExecutionCategory() const { return m_category; }

    DbResult Execute(ECInstanceId const& instanceId) { return _Execute(instanceId); }

    IClassMap const& GetClassMap() const { return m_classMap; }
    PropertyMapStructArrayCR GetPropertyMap() const { return m_propertyMap; }
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2014
//+===============+===============+===============+===============+===============+======
struct ECSqlStepTaskFactory
    {
private:
    ECSqlStepTaskFactory ();
    ~ECSqlStepTaskFactory ();

    static ECSqlStepTaskCreateStatus CreateDeleteStepTask(ECSqlStepTask::Collection& taskList, ECSqlPrepareContext&, ECDbR, IClassMap const&, bool isPolymorphicStatement);
    static ECSqlStepTaskCreateStatus CreateInsertStepTask (ECSqlStepTask::Collection& taskList, ECSqlPrepareContext&, ECDbR, IClassMap const&);
    static ECSqlStepTaskCreateStatus CreateUpdateStepTask(ECSqlStepTask::Collection& taskList, ECSqlPrepareContext&, ECDbR, IClassMap const&, bool isPolymorphicStatement);

    static ECSqlStepTaskCreateStatus CreateStepTaskList(ECSqlStepTask::Collection& taskList, ECSqlPrepareContext&, StepTaskType, ECDbR, IClassMap const&, bool isPolymorphicStatement);
    static void GetSubclasses (ECSqlParseContext::ClassListById& classes, ECClassCR, ECDbSchemaManagerCR);
    static void GetConstraintClasses (ECSqlParseContext::ClassListById& classes, ECRelationshipConstraintCR, ECDbSchemaManagerCR, bool* containAnyClass);

public:
    static ECSqlStepTaskCreateStatus CreatePropertyStepTask (std::unique_ptr<ECSqlStepTask>&, StepTaskType, ECSqlPrepareContext&, ECDbR, IClassMap const&, Utf8CP propertyAccessPath);
    static ECSqlStepTaskCreateStatus CreateClassStepTask(ECSqlStepTask::Collection& taskList, StepTaskType, ECSqlPrepareContext&, ECDbR, IClassMap const&, bool isPolymorphicStatement);
    };

//=======================================================================================
//! ParametericStepTask Accepts a paramter source
//! @bsiclass                                                Affan.Khan      04/2014
//+===============+===============+===============+===============+===============+=====
struct ParametericStepTask : public ECSqlStepTask
    {
private: 
    virtual void _SetParameterSource (ECSqlParameterValue& parameterSource) = 0;
    virtual ECSqlParameterValue* _GetParameter () = 0;

protected:
    ParametericStepTask (ExecutionCategory category, PropertyMapStructArrayCR propertyMap, IClassMap const& classMap)
        :ECSqlStepTask(category, propertyMap, classMap) {}

public:
    virtual ~ParametericStepTask (){}

    void SetParameterSource (ECSqlParameterValue& parameterSource) { _SetParameterSource (parameterSource); }
    ECSqlParameterValue* GetParameter () { return  _GetParameter (); }
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
    static const int PARAMETER_ECPROPERTYPATHID = 3;
    static const int PARAMETER_ECARRAYINDEX = 4;
    static const int PARAMETER_STRUCTARRAY = 5;

    std::unique_ptr<EmbeddedECSqlStatement> m_insertStmt;
    ECSqlParameterValue* m_parameterValue;
    ECPropertyId m_propertyPathId;
private:
    InsertStructArrayStepTask (PropertyMapStructArrayCR, IClassMap const&);
    virtual DbResult _Execute (ECInstanceId const& instanceId) override;

    virtual void _SetParameterSource (ECSqlParameterValue& parameterSource) override { m_parameterValue = &parameterSource; }
    virtual ECSqlParameterValue* _GetParameter () override { return m_parameterValue; }

public:
    virtual ~InsertStructArrayStepTask (){}

    EmbeddedECSqlStatement& GetStatement () { return *m_insertStmt; }
    static ECSqlStepTaskCreateStatus Create (std::unique_ptr<InsertStructArrayStepTask>& insertStepTask, ECSqlPrepareContext&, ECDbR, IClassMap const&, Utf8CP propAccessString);
    };

//=======================================================================================
//! DeleteStructArrayECSqlStepTask delete struct array elements
//! @bsiclass                                                Affan.Khan      02/2014
//+===============+===============+===============+===============+===============+======
struct DeleteStructArrayStepTask : public ECSqlStepTask
    {
private:
    static const int PARAMETER_OWNERECINSTANCEID = 1;
    std::unique_ptr<EmbeddedECSqlStatement> m_deleteStmt;

private:
    DeleteStructArrayStepTask (PropertyMapStructArrayCR, IClassMap const&);
    virtual DbResult _Execute (ECInstanceId const& instanceId) override;

    public:
    ~DeleteStructArrayStepTask (){}

    EmbeddedECSqlStatement& GetStatement () { return *m_deleteStmt; }
    static ECSqlStepTaskCreateStatus Create (std::unique_ptr<DeleteStructArrayStepTask>& deleteStepTask, ECSqlPrepareContext&, ECDbR, IClassMap const&, Utf8CP propAccessString);
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
    UpdateStructArrayStepTask (std::unique_ptr<InsertStructArrayStepTask>& insertStepTask, std::unique_ptr<DeleteStructArrayStepTask>& deleteStepTask);
    virtual DbResult _Execute (ECInstanceId const& instanceId) override;
    virtual void _SetParameterSource (ECSqlParameterValue& parameterSource) override { m_insertStepTask->SetParameterSource (parameterSource); }
    virtual ECSqlParameterValue* _GetParameter () override { return m_insertStepTask->GetParameter (); }
public:
    ~UpdateStructArrayStepTask (){};

    static ECSqlStepTaskCreateStatus Create(std::unique_ptr<UpdateStructArrayStepTask>& updateStepTask, ECSqlPrepareContext&, ECDbR, IClassMap const&, Utf8CP propAccessString);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE