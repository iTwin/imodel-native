/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECInstanceAdapterHelper.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECInstanceInserter::Impl
    {
private:
    ECDbCR m_ecdb;
    ECN::ECClassCR m_ecClass;
    mutable ECSqlStatement m_statement;
    ECValueBindingInfoCollection m_ecValueBindingInfos;
    ECSqlSystemPropertyBindingInfo* m_ecinstanceIdBindingInfo;
    bool m_needsCalculatedPropertyEvaluation;
    bool m_isValid;

    //not copyable
    Impl(Impl const&) = delete;
    Impl& operator=(Impl const&) = delete;

    void Initialize(ECCrudWriteToken const*);

    DbResult InsertRelationship(ECInstanceKey& newInstanceKey, ECN::IECRelationshipInstanceCR, bool autogenerateECInstanceId, ECInstanceId const* userProvidedECInstanceId) const;

    static void LogFailure(ECN::IECInstanceCR instance, Utf8CP errorMessage) { ECInstanceAdapterHelper::LogFailure("insert", instance, errorMessage); }

public:
    Impl(ECDbCR ecdb, ECClassCR ecClass, ECCrudWriteToken const* writeToken);

    DbResult Insert(ECInstanceKey& newInstanceKey, IECInstanceCR, bool autogenerateECInstanceId = true, ECInstanceId const* userprovidedECInstanceId = nullptr) const;
    DbResult Insert(ECN::IECInstanceR instance, bool autogenerateECInstanceId = true) const;
    DbResult InsertRelationship(ECInstanceKey& newInstanceKey, ECInstanceId sourceId, ECInstanceId targetId, ECN::IECRelationshipInstanceCP relationshipProperties = nullptr, bool autogenerateECInstanceId = true, ECInstanceId const* userProvidedECInstanceId = nullptr) const;
    bool IsValid() const { return m_isValid; }
    };


//*************************************************************************************
// ECInstanceInserter
//*************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceInserter::ECInstanceInserter(ECDbCR ecdb, ECN::ECClassCR ecClass, ECCrudWriteToken const* writeToken)
    : m_impl(new ECInstanceInserter::Impl(ecdb, ecClass, writeToken))
    {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceInserter::~ECInstanceInserter()
    {
    if (m_impl != nullptr)
        {
        delete m_impl;
        m_impl = nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ECInstanceInserter::IsValid() const { return m_impl->IsValid(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECInstanceInserter::Insert(ECInstanceKey& newInstanceKey, ECN::IECInstanceCR instance, bool autogenerateECInstanceId, ECInstanceId const* userprovidedECInstanceId) const
    {
    return m_impl->Insert(newInstanceKey, instance, autogenerateECInstanceId, userprovidedECInstanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECInstanceInserter::InsertRelationship(ECInstanceKey& newInstanceKey, ECInstanceId sourceId, ECInstanceId targetId, ECN::IECRelationshipInstanceCP relationshipProperties, bool autogenerateECInstanceId, ECInstanceId const* userProvidedECInstanceId) const
    {
    return m_impl->InsertRelationship(newInstanceKey, sourceId, targetId, relationshipProperties, autogenerateECInstanceId, userProvidedECInstanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECInstanceInserter::Insert(ECN::IECInstanceR instance, bool autogenerateECInstanceId) const
    {
    return m_impl->Insert(instance, autogenerateECInstanceId);
    }

//*************************************************************************************
// ECInstanceInserter::Impl
//*************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceInserter::Impl::Impl(ECDbCR ecdb, ECClassCR ecClass, ECCrudWriteToken const* writeToken)
    : m_ecdb(ecdb), m_ecClass(ecClass), m_ecinstanceIdBindingInfo(nullptr), m_needsCalculatedPropertyEvaluation(false), m_isValid(false)
    {
    Initialize(writeToken);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECInstanceInserter::Impl::Initialize(ECCrudWriteToken const* writeToken)
    {
    Utf8String ecsql("INSERT INTO ");
    //add ECInstanceId. If NULL is bound to it, ECDb will auto-generate one
    ecsql.append(m_ecClass.GetECSqlName()).append("(" ECDBSYS_PROP_ECInstanceId);
    Utf8String valuesClause(") VALUES(?");

    int parameterIndex = 1;
    //cache the binding info as we later need to set the user provided instance id (if available)
    m_ecinstanceIdBindingInfo = m_ecValueBindingInfos.AddBindingInfo(ECValueBindingInfo::SystemPropertyKind::ECInstanceId, parameterIndex);
    parameterIndex++;

    PrimitiveECPropertyCP currentTimeStampProp = nullptr;
    if (SUCCESS != CoreCustomAttributeHelper::GetCurrentTimeStampProperty(currentTimeStampProp, m_ecClass))
        {
        LOG.errorv("ECInstanceInserter failure: Could not retrieve the 'ClassHasCurrentTimeStampProperty' custom attribute from the ECClass '%s'.",
                   m_ecClass.GetFullName());

        m_isValid = false;
        return;
        }

    for (ECPropertyCP ecProperty : m_ecClass.GetProperties(true))
        {
        //Current time stamp props are populated by SQLite, so ignore them here.
        if (currentTimeStampProp != nullptr && ecProperty == currentTimeStampProp)
            continue;

        if (!m_needsCalculatedPropertyEvaluation)
            m_needsCalculatedPropertyEvaluation = ECInstanceAdapterHelper::IsOrContainsCalculatedProperty(*ecProperty);

        ecsql.append(",[").append(ecProperty->GetName()).append("]");
        valuesClause.append(",?");
        if (SUCCESS != m_ecValueBindingInfos.AddBindingInfo(m_ecClass, *ecProperty, parameterIndex))
            {
            m_isValid = false;
            return;
            }

        parameterIndex++;
        }

    if (m_ecClass.IsRelationshipClass())
        {
        //SourceECClassId and TargetECClassId are not needed during insert
        ecsql.append("," ECDBSYS_PROP_SourceECInstanceId);
        valuesClause.append(",?");
        m_ecValueBindingInfos.AddBindingInfo(ECValueBindingInfo::SystemPropertyKind::SourceECInstanceId, parameterIndex);

        parameterIndex++;
        ecsql.append("," ECDBSYS_PROP_TargetECInstanceId);
        valuesClause.append(",?");
        m_ecValueBindingInfos.AddBindingInfo(ECValueBindingInfo::SystemPropertyKind::TargetECInstanceId, parameterIndex);
        }

    ecsql.append(valuesClause).append(")");
    m_isValid = ECSqlStatus::Success == m_statement.Prepare(m_ecdb, ecsql.c_str(), writeToken);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECInstanceInserter::Impl::Insert(ECInstanceKey& newInstanceKey, IECInstanceCR instance, bool autogenerateECInstanceId, ECInstanceId const* userProvidedECInstanceId) const
    {
    if (!IsValid())
        {
        LOG.errorv("ECInstanceInserter for ECClass '%s' is invalid as the ECClass is not mapped or not instantiable.", m_ecClass.GetFullName());
        return BE_SQLITE_ERROR;
        }

    if (!ECInstanceAdapterHelper::Equals(instance.GetClass(), m_ecClass))
        {
        LOG.errorv("Failed to insert RelationshipECInstance: Invalid ECInstance passed to ECInstanceInserter. ECClass mismatch: Expected ECClass: '%s'. ECInstance's ECClass: '%s'.",
                             m_ecClass.GetFullName(), instance.GetClass().GetFullName());
        return BE_SQLITE_ERROR;
        }

    if (m_ecClass.IsRelationshipClass())
        {
        IECRelationshipInstanceCP relInstance = dynamic_cast<IECRelationshipInstanceCP> (&instance);
        BeAssert(relInstance != nullptr && "Was checked before");
        return InsertRelationship(newInstanceKey, *relInstance, autogenerateECInstanceId, userProvidedECInstanceId);
        }

    if (autogenerateECInstanceId && userProvidedECInstanceId != nullptr)
        {
        LogFailure(instance, "Wrong usage of ECInstanceInserter::Insert. When passing true for autogenerateECInstanceId, userprovidedECInstanceId must be nullptr.");
        return BE_SQLITE_ERROR;
        }

    ECInstanceId actualUserProvidedInstanceId;
    //try to retrieve a user provided ECInstanceId as auto-generation is not wanted
    if (!autogenerateECInstanceId)
        {
        if (userProvidedECInstanceId != nullptr)
            {
            if (!userProvidedECInstanceId->IsValid())
                {
                Utf8String errorMessage;
                errorMessage.Sprintf("Invalid parameter for ECInstanceInserter::Insert. Parameter userprovidedECInstanceId is not a valid " ECDBSYS_PROP_ECInstanceId ".",
                                     m_ecClass.GetFullName());

                LogFailure(instance, errorMessage.c_str());
                return BE_SQLITE_ERROR;
                }

            actualUserProvidedInstanceId = *userProvidedECInstanceId;
            }
        else
            {
            //user provided ECInstanceId is null -> try to retrieve it from ECInstance
            Utf8String instanceIdStr = instance.GetInstanceId();
            if (instanceIdStr.empty())
                {
                Utf8String errorMessage;
                errorMessage.Sprintf("Invalid ECInstance passed to ECInstanceInserter. %s ECInstance's instance id must be set when " ECDBSYS_PROP_ECInstanceId " auto-generation is disabled and no user provided " ECDBSYS_PROP_ECInstanceId " was given explicitly.",
                                     m_ecClass.GetFullName());

                LogFailure(instance, errorMessage.c_str());
                return BE_SQLITE_ERROR;

                }

            if (SUCCESS != ECInstanceId::FromString(actualUserProvidedInstanceId, instanceIdStr.c_str()))
                {
                Utf8String errorMessage;
                errorMessage.Sprintf("Invalid ECInstance passed to ECInstanceInserter. %s ECInstance's instance id '%s' must be of type " ECDBSYS_PROP_ECInstanceId " when " ECDBSYS_PROP_ECInstanceId " auto-generation is disabled and no user provided " ECDBSYS_PROP_ECInstanceId " was given explicitly.",
                                     m_ecClass.GetFullName(), instanceIdStr.c_str());

                LogFailure(instance, errorMessage.c_str());
                return BE_SQLITE_ERROR;
                }
            }

        BeAssert(actualUserProvidedInstanceId.IsValid());
        }

    //"Pins" the internal memory buffer used by the ECDBuffer such that :
    //a) all calculated property values will be evaluated exactly once, when scope is constructed; and
    //b) addresses of all property values will not change for lifetime of scope.
    //To be used in conjunction with ECValue::SetAllowsPointersIntoInstanceMemory () to ensure
    //pointers remain valid for lifetime of scope.
    ECDBufferScope scope;
    if (m_needsCalculatedPropertyEvaluation)
        scope.Init(instance.GetECDBuffer());

    ECInstanceAdapterHelper::ECInstanceInfo instanceInfo(instance, actualUserProvidedInstanceId, true);
    //now add parameter values for regular properties
    for (ECValueBindingInfo const* bindingInfo : m_ecValueBindingInfos)
        {
        BeAssert(bindingInfo->HasECSqlParameterIndex());
        auto stat = ECInstanceAdapterHelper::BindValue(m_statement.GetBinder(bindingInfo->GetECSqlParameterIndex()), instanceInfo, *bindingInfo);
        if (stat != SUCCESS)
            {
            Utf8String errorMessage;
            errorMessage.Sprintf("Could not bind value to ECSQL parameter %d [ECSQL: '%s'].", bindingInfo->GetECSqlParameterIndex(),
                                 m_statement.GetECSql());
            LogFailure(instance, errorMessage.c_str());
            return BE_SQLITE_ERROR;
            }
        }

    //now execute statement
    const DbResult stepStatus = m_statement.Step(newInstanceKey);

    //reset once we are done with executing the statement to put the statement in inactive state (less memory etc)
    m_statement.Reset();
    m_statement.ClearBindings();

    return BE_SQLITE_DONE == stepStatus ? BE_SQLITE_OK : stepStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECInstanceInserter::Impl::InsertRelationship(ECInstanceKey& newInstanceKey, IECRelationshipInstanceCR relInstance, bool autogenerateECInstanceId, ECInstanceId const* userProvidedECInstanceId) const
    {
    IECInstancePtr sourceInstance = relInstance.GetSource();
    IECInstancePtr targetInstance = relInstance.GetTarget();
    if (sourceInstance == nullptr || targetInstance == nullptr)
        {
        LOG.error("Failed to insert RelationshipECInstance: Source and Target must be set in the RelationshipECInstance.");
        return BE_SQLITE_ERROR;
        }

    ECInstanceId sourceId, targetId;
    if (SUCCESS != ECInstanceId::FromString(sourceId, sourceInstance->GetInstanceId().c_str()))
        {
        LOG.error("Failed to insert RelationshipECInstance: Source instance of the RelationshipECInstance must have a valid InstanceId.");
        return BE_SQLITE_ERROR;
        }

    if (SUCCESS != ECInstanceId::FromString(targetId, targetInstance->GetInstanceId().c_str()))
        {
        LOG.error("Failed to insert RelationshipECInstance: Target instance of the RelationshipECInstance must have a valid InstanceId.");
        return BE_SQLITE_ERROR;
        }

    return InsertRelationship(newInstanceKey, sourceId, targetId, &relInstance, autogenerateECInstanceId, userProvidedECInstanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECInstanceInserter::Impl::InsertRelationship(ECInstanceKey& newInstanceKey, ECInstanceId sourceId, ECInstanceId targetId, ECN::IECRelationshipInstanceCP relationshipProperties, bool autogenerateECInstanceId, ECInstanceId const* userProvidedECInstanceId) const
    {
    if (!IsValid())
        {
        LOG.errorv("ECInstanceInserter for ECClass '%s' is invalid as the ECClass is not mapped or not instantiable.", m_ecClass.GetFullName());
        return BE_SQLITE_ERROR;
        }

    if (relationshipProperties != nullptr && !ECInstanceAdapterHelper::Equals(relationshipProperties->GetClass(), m_ecClass))
        {
        LOG.errorv("Failed to insert RelationshipECInstance: Invalid RelationshipECInstance passed to ECInstanceInserter. "
                   "ECClass mismatch: Expected ECClass: '%s'. ECInstance's ECClass: '%s'.", m_ecClass.GetFullName(), relationshipProperties->GetClass().GetFullName());
        return BE_SQLITE_ERROR;
        }

    if (autogenerateECInstanceId && userProvidedECInstanceId != nullptr)
        {
        LOG.error("Failed to insert RelationshipECInstance: Wrong usage of ECInstanceInserter::Insert.When passing true for autogenerateECInstanceId, userprovidedECInstanceId must be nullptr.");
        return BE_SQLITE_ERROR;
        }

    ECInstanceId actualUserProvidedInstanceId;
    //try to retrieve a user provided ECInstanceId as auto-generation is not wanted
    if (!autogenerateECInstanceId)
        {
        if (userProvidedECInstanceId != nullptr)
            {
            if (!userProvidedECInstanceId->IsValid())
                {
                LOG.error("Failed to insert RelationshipECInstance: Parameter 'userprovidedECInstanceId' is not a valid " ECDBSYS_PROP_ECInstanceId ".");
                return BE_SQLITE_ERROR;
                }

            actualUserProvidedInstanceId = *userProvidedECInstanceId;
            }
        else
            {
            LOG.error("Failed to insert RelationshipECInstance: Parameter 'userprovidedECInstanceId' must be set if parameter 'autogenerateECInstanceId' is false.");
            return BE_SQLITE_ERROR;
            }
        }

    //"Pins" the internal memory buffer used by the ECDBuffer such that :
    //a) all calculated property values will be evaluated exactly once, when scope is constructed; and
    //b) addresses of all property values will not change for lifetime of scope.
    //To be used in conjunction with ECValue::SetAllowsPointersIntoInstanceMemory () to ensure
    //pointers remain valid for lifetime of scope.
    ECDBufferScope scope;
    if (m_needsCalculatedPropertyEvaluation && relationshipProperties != nullptr)
        scope.Init(relationshipProperties->GetECDBuffer());

    ECInstanceAdapterHelper::ECInstanceInfo instanceInfo(actualUserProvidedInstanceId, sourceId, targetId, relationshipProperties);

    //now add parameter values
    for (auto const* bindingInfo : m_ecValueBindingInfos)
        {
        BeAssert(bindingInfo->HasECSqlParameterIndex());
        if (SUCCESS != ECInstanceAdapterHelper::BindValue(m_statement.GetBinder(bindingInfo->GetECSqlParameterIndex()), instanceInfo, *bindingInfo))
            {
            LOG.errorv("Failed to insert RelationshipECInstance: Could not bind value to ECSQL parameter %d [ECSQL: '%s'].", bindingInfo->GetECSqlParameterIndex(),
                                 m_statement.GetECSql());
            return BE_SQLITE_ERROR;
            }
        }

    //now execute statement
    const DbResult stepStatus = m_statement.Step(newInstanceKey);

    //reset once we are done with executing the statement to put the statement in inactive state (less memory etc)
    m_statement.Reset();
    m_statement.ClearBindings();

    return BE_SQLITE_DONE == stepStatus ? BE_SQLITE_OK : stepStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECInstanceInserter::Impl::Insert(ECN::IECInstanceR instance, bool autogenerateECInstanceId) const
    {
    ECInstanceKey newInstanceKey;
    const DbResult stat = Insert(newInstanceKey, instance, autogenerateECInstanceId, nullptr);
    if (BE_SQLITE_OK != stat)
        return stat;

    //only set instance id in ECInstance if it was auto-generated by ECDb. IF not auto-generated it hasn't changed
    //in the input ECInstance, and hence doesn't need to be set
    if (!autogenerateECInstanceId)
        return BE_SQLITE_OK;

    return SUCCESS == ECInstanceAdapterHelper::SetECInstanceId(instance, newInstanceKey.GetInstanceId()) ? BE_SQLITE_OK : BE_SQLITE_ERROR;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

