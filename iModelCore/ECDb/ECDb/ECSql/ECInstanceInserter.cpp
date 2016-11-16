/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECInstanceInserter.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECInstanceAdapterHelper.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//======================================================================================
// @bsiclass                                                 Krischan.Eberle      06/2014
//+===============+===============+===============+===============+===============+======
struct ECInstanceInserter::Impl : NonCopyableClass
    {
private:
    ECDbCR m_ecdb;
    ECN::ECClassCR m_ecClass;
    mutable ECSqlStatement m_statement;
    ECValueBindingInfoCollection m_ecValueBindingInfos;
    ECSqlSystemPropertyBindingInfo* m_ecinstanceIdBindingInfo;
    bool m_needsCalculatedPropertyEvaluation;
    bool m_isValid;

    void Initialize(ECSqlWriteToken const*);

    DbResult InsertRelationship(ECInstanceKey& newInstanceKey, ECN::IECRelationshipInstanceCR, bool autogenerateECInstanceId, ECInstanceId const* userProvidedECInstanceId) const;

    static void LogFailure(ECN::IECInstanceCR instance, Utf8CP errorMessage) { ECInstanceAdapterHelper::LogFailure("insert", instance, errorMessage); }

public:
    Impl(ECDbCR ecdb, ECClassCR ecClass, ECSqlWriteToken const* writeToken);

    DbResult Insert(ECInstanceKey& newInstanceKey, IECInstanceCR, bool autogenerateECInstanceId = true, ECInstanceId const* userprovidedECInstanceId = nullptr) const;
    DbResult Insert(ECN::IECInstanceR instance, bool autogenerateECInstanceId = true) const;
    DbResult InsertRelationship(ECInstanceKey& newInstanceKey, ECInstanceKey const& sourceKey, ECInstanceKey const& targetKey, ECN::IECRelationshipInstanceCP relationshipProperties = nullptr, bool autogenerateECInstanceId = true, ECInstanceId const* userProvidedECInstanceId = nullptr) const;
    bool IsValid() const { return m_isValid; }
    };


//*************************************************************************************
// ECInstanceInserter
//*************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceInserter::ECInstanceInserter(ECDbCR ecdb, ECN::ECClassCR ecClass, ECSqlWriteToken const* writeToken)
    : m_impl(new ECInstanceInserter::Impl(ecdb, ecClass, writeToken))
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
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
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
bool ECInstanceInserter::IsValid() const { return m_impl->IsValid(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECInstanceInserter::Insert(ECInstanceKey& newInstanceKey, ECN::IECInstanceCR instance, bool autogenerateECInstanceId, ECInstanceId const* userprovidedECInstanceId) const
    {
    return m_impl->Insert(newInstanceKey, instance, autogenerateECInstanceId, userprovidedECInstanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/16
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECInstanceInserter::InsertRelationship(ECInstanceKey& newInstanceKey, ECInstanceKey const& sourceKey, ECInstanceKey const& targetKey, ECN::IECRelationshipInstanceCP relationshipProperties, bool autogenerateECInstanceId, ECInstanceId const* userProvidedECInstanceId) const
    {
    return m_impl->InsertRelationship(newInstanceKey, sourceKey, targetKey, relationshipProperties, autogenerateECInstanceId, userProvidedECInstanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECInstanceInserter::Insert(ECN::IECInstanceR instance, bool autogenerateECInstanceId) const
    {
    return m_impl->Insert(instance, autogenerateECInstanceId);
    }

//*************************************************************************************
// ECInstanceInserter::Impl
//*************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle      06/2014
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceInserter::Impl::Impl(ECDbCR ecdb, ECClassCR ecClass, ECSqlWriteToken const* writeToken)
    : m_ecdb(ecdb), m_ecClass(ecClass), m_ecinstanceIdBindingInfo(nullptr), m_needsCalculatedPropertyEvaluation(false), m_isValid(false)
    {
    Initialize(writeToken);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
void ECInstanceInserter::Impl::Initialize(ECSqlWriteToken const* writeToken)
    {
    Utf8String ecsql("INSERT INTO ");
    //add ECInstanceId. If NULL is bound to it, ECDb will auto-generate one
    ecsql.append(m_ecClass.GetECSqlName()).append("(ECInstanceId");
    Utf8String valuesClause(") VALUES(?");

    int parameterIndex = 1;
    //cache the binding info as we later need to set the user provided instance id (if available)
    m_ecinstanceIdBindingInfo = m_ecValueBindingInfos.AddBindingInfo(ECValueBindingInfo::SystemPropertyKind::ECInstanceId, parameterIndex);
    parameterIndex++;

    ECPropertyCP currentTimeStampProp = nullptr;
    bool hasCurrentTimeStampProp = ECInstanceAdapterHelper::TryGetCurrentTimeStampProperty(currentTimeStampProp, m_ecClass);

    for (ECPropertyCP ecProperty : m_ecClass.GetProperties(true))
        {
        //Current time stamp props are populated by SQLite, so ignore them here.
        if (hasCurrentTimeStampProp && ecProperty == currentTimeStampProp)
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
        ecsql.append(",").append(ECDbSystemSchemaHelper::ToString(ECSqlSystemProperty::SourceECInstanceId));
        valuesClause.append(",?");
        m_ecValueBindingInfos.AddBindingInfo(ECValueBindingInfo::SystemPropertyKind::SourceECInstanceId, parameterIndex);

        parameterIndex++;
        ecsql.append(",").append(ECDbSystemSchemaHelper::ToString(ECSqlSystemProperty::SourceECClassId));
        valuesClause.append(",?");
        m_ecValueBindingInfos.AddBindingInfo(ECValueBindingInfo::SystemPropertyKind::SourceECClassId, parameterIndex);

        parameterIndex++;
        ecsql.append(",").append(ECDbSystemSchemaHelper::ToString(ECSqlSystemProperty::TargetECInstanceId));
        valuesClause.append(",?");
        m_ecValueBindingInfos.AddBindingInfo(ECValueBindingInfo::SystemPropertyKind::TargetECInstanceId, parameterIndex);

        parameterIndex++;
        ecsql.append(",").append(ECDbSystemSchemaHelper::ToString(ECSqlSystemProperty::TargetECClassId));
        valuesClause.append(",?");
        m_ecValueBindingInfos.AddBindingInfo(ECValueBindingInfo::SystemPropertyKind::TargetECClassId, parameterIndex);
        }

    ecsql.append(valuesClause).append(")");
    m_isValid = ECSqlStatus::Success == m_statement.Prepare(m_ecdb, ecsql.c_str(), writeToken);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
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
                errorMessage.Sprintf("Invalid parameter for ECInstanceInserter::Insert. Parameter userprovidedECInstanceId is not a valid ECInstanceId.",
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
                errorMessage.Sprintf("Invalid ECInstance passed to ECInstanceInserter. %s ECInstance's instance id must be set when ECInstanceId auto-generation is disabled and no user provided ECInstanceId was given explicitly.",
                                     m_ecClass.GetFullName());

                LogFailure(instance, errorMessage.c_str());
                return BE_SQLITE_ERROR;

                }

            if (SUCCESS != ECInstanceId::FromString(actualUserProvidedInstanceId, instanceIdStr.c_str()))
                {
                Utf8String errorMessage;
                errorMessage.Sprintf("Invalid ECInstance passed to ECInstanceInserter. %s ECInstance's instance id '%s' must be of type ECInstanceId when ECInstanceId auto-gneration is disabled and no user provided ECInstanceId was given explicitly.",
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

    ECInstanceAdapterHelper::ECInstanceInfo instanceInfo(instance, actualUserProvidedInstanceId);
    //now add parameter values for regular properties
    for (auto const& bindingInfo : m_ecValueBindingInfos)
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

    return stepStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/16
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

    return InsertRelationship(newInstanceKey, ECInstanceKey(sourceInstance->GetClass().GetId(), sourceId), ECInstanceKey(targetInstance->GetClass().GetId(), targetId),
                  &relInstance, autogenerateECInstanceId, userProvidedECInstanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/16
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECInstanceInserter::Impl::InsertRelationship(ECInstanceKey& newInstanceKey, ECInstanceKey const& sourceKey, ECInstanceKey const& targetKey, ECN::IECRelationshipInstanceCP relationshipProperties, bool autogenerateECInstanceId, ECInstanceId const* userProvidedECInstanceId) const
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
                LOG.error("Failed to insert RelationshipECInstance: Parameter 'userprovidedECInstanceId' is not a valid ECInstanceId.");
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

    ECInstanceAdapterHelper::ECInstanceInfo instanceInfo(actualUserProvidedInstanceId, sourceKey, targetKey, relationshipProperties);

    //now add parameter values
    for (auto const& bindingInfo : m_ecValueBindingInfos)
        {
        BeAssert(bindingInfo->HasECSqlParameterIndex());
        auto stat = ECInstanceAdapterHelper::BindValue(m_statement.GetBinder(bindingInfo->GetECSqlParameterIndex()), instanceInfo, *bindingInfo);
        if (stat != SUCCESS)
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

    return stepStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECInstanceInserter::Impl::Insert(ECN::IECInstanceR instance, bool autogenerateECInstanceId) const
    {
    ECInstanceKey newInstanceKey;
    const DbResult stat = Insert(newInstanceKey, instance, autogenerateECInstanceId, nullptr);
    if (BE_SQLITE_DONE != stat)
        return stat;

    //only set instance id in ECInstance if it was auto-generated by ECDb. IF not auto-generated it hasn't changed
    //in the input ECInstance, and hence doesn't need to be set
    if (!autogenerateECInstanceId)
        return BE_SQLITE_DONE;

    return SUCCESS == ECInstanceAdapterHelper::SetECInstanceId(instance, newInstanceKey.GetECInstanceId()) ? BE_SQLITE_DONE : BE_SQLITE_ERROR;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

