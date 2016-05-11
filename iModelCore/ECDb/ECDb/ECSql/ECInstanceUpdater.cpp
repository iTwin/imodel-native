/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECInstanceUpdater.cpp $
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
struct ECInstanceUpdater::Impl
    {
private:
    ECDbCR m_ecdb;
    ECN::ECClassCR m_ecClass;
    mutable ECSqlStatement m_statement;
    ECValueBindingInfoCollection m_ecValueBindingInfos;
    int m_ecinstanceIdParameterIndex;
    bool m_needsCalculatedPropertyEvaluation;
    bool m_isValid;

    void Initialize(bvector<ECPropertyCP> const& properties, Utf8CP ecsqlOptions) { Initialize(properties, nullptr, ecsqlOptions); }
    void Initialize(bvector<uint32_t> const& propertyIndexes, Utf8CP ecsqlOptions);
    void Initialize(bvector<ECPropertyCP> const& properties, bvector<Utf8CP> const* propertyAccessStrings, Utf8CP ecsqlOptions);

    static void LogFailure(ECN::IECInstanceCR instance, Utf8CP errorMessage) { ECInstanceAdapterHelper::LogFailure("update", instance, errorMessage); }

public:
    Impl(ECDbCR, ECClassCR, Utf8CP ecsqlOptions);
    Impl(ECDbCR, IECInstanceCR, Utf8CP ecsqlOptions);
    Impl(ECDbCR, ECClassCR, bvector<uint32_t> const& propertyIndexesToBind, Utf8CP ecsqlOptions);
    Impl(ECDbCR, ECClassCR, bvector<ECPropertyCP> const& properties, Utf8CP ecsqlOptions);
    ~Impl() {}

    BentleyStatus Update(IECInstanceCR instance) const;
    bool IsValid() const { return m_isValid; }
    };


//*************************************************************************************
// ECInstanceUpdater
//*************************************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                   Carole.MacDonald                   02 / 14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::ECInstanceUpdater(ECDbCR ecdb, ECN::ECClassCR ecClass, Utf8CP ecsqlOptions)
    {
    m_impl = new Impl(ecdb, ecClass, ecsqlOptions);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Carole.MacDonald                   08/14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::ECInstanceUpdater(ECDbCR ecdb, ECN::IECInstanceCR instance, Utf8CP ecsqlOptions)
    {
    m_impl = new Impl(ecdb, instance, ecsqlOptions);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Carole.MacDonald                   09/14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::ECInstanceUpdater(ECDbCR ecdb, ECN::ECClassCR ecClass, bvector<uint32_t> const& propertyIndexesToBind, Utf8CP ecsqlOptions)
    {
    m_impl = new Impl(ecdb, ecClass, propertyIndexesToBind, ecsqlOptions);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Carole.MacDonald                   02/16
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::ECInstanceUpdater(ECDbCR ecdb, ECN::ECClassCR ecClass, bvector<ECN::ECPropertyCP> const& properties, Utf8CP ecsqlOptions)
    {
    m_impl = new Impl(ecdb, ecClass, properties, ecsqlOptions);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::~ECInstanceUpdater()
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
bool ECInstanceUpdater::IsValid() const
    {
    return m_impl->IsValid();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECInstanceUpdater::Update(ECN::IECInstanceCR instance) const
    {
    return m_impl->Update(instance);
    }


//*************************************************************************************
// ECInstanceUpdater::Impl
//*************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::Impl::Impl(ECDbCR ecdb, ECClassCR ecClass, Utf8CP ecsqlOptions)
    : m_ecdb(ecdb), m_ecClass(ecClass), m_isValid(false), m_needsCalculatedPropertyEvaluation(false)
    {
    bvector<ECPropertyCP> properties;
    for (ECPropertyCP prop : m_ecClass.GetProperties(true))
        {
        properties.push_back(prop);
        }

    Initialize(properties, ecsqlOptions);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald                   08/14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::Impl::Impl(ECDbCR ecdb, IECInstanceCR instance, Utf8CP ecsqlOptions)
    : m_ecdb(ecdb), m_ecClass(instance.GetClass()), m_isValid(false), m_needsCalculatedPropertyEvaluation(false)
    {
    bvector<ECPropertyCP> properties;

    ECValuesCollectionPtr collection = ECValuesCollection::Create(instance);
    for (ECPropertyValueCR propertyValue : *collection)
        {
        if (propertyValue.GetValue().IsLoaded())
            properties.push_back(propertyValue.GetValueAccessor().GetECProperty());
        }

    Initialize(properties, ecsqlOptions);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald                   02/16
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::Impl::Impl(ECDbCR ecdb, ECClassCR ecClass, bvector<uint32_t> const& propertyIndexesToBind, Utf8CP ecsqlOptions)
    : m_ecdb(ecdb), m_ecClass(ecClass), m_isValid(false), m_needsCalculatedPropertyEvaluation(false)
    {
    Initialize(propertyIndexesToBind, ecsqlOptions);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald                   09/14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::Impl::Impl(ECDbCR ecdb, ECClassCR ecClass, bvector<ECN::ECPropertyCP> const& propertiesToBind, Utf8CP ecsqlOptions)
    : m_ecdb(ecdb), m_ecClass(ecClass), m_isValid(false), m_needsCalculatedPropertyEvaluation(false)
    {
    Initialize(propertiesToBind, ecsqlOptions);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald                   08/14
//+---------------+---------------+---------------+---------------+---------------+------
void ECInstanceUpdater::Impl::Initialize(bvector<uint32_t> const& propertyIndexesToBind, Utf8CP ecsqlOptions)
    {
    bvector<ECPropertyCP> properties;
    bvector<Utf8CP> propertyAccessStrings;
    ECEnablerP enabler = m_ecClass.GetDefaultStandaloneEnabler();
    for (uint32_t propertyIndex : propertyIndexesToBind)
        {
        ECPropertyCP ecProperty = enabler->LookupECProperty(propertyIndex);
        if (ecProperty == nullptr)
            {
            LOG.errorv("Could not find ECProperty in ECClass '%s' for property index %" PRIu32 ".", m_ecClass.GetFullName(), propertyIndex);
            m_isValid = false;
            return;
            }

        if (ecProperty->GetIsStruct())
            continue;

        properties.push_back(ecProperty);
        Utf8CP accessString = nullptr;
        if (ECObjectsStatus::Success != enabler->GetAccessString(accessString, propertyIndex))
            {
            LOG.errorv("Could not retrieve property access string in ECClass '%s' for property index %" PRIu32 ".", m_ecClass.GetFullName(), propertyIndex);
            m_isValid = false;
            return;
            }

        propertyAccessStrings.push_back(accessString);
        }

    return Initialize(properties, &propertyAccessStrings, ecsqlOptions);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   05/16
//+---------------+---------------+---------------+---------------+---------------+------
void ECInstanceUpdater::Impl::Initialize(bvector<ECPropertyCP> const& properties, bvector<Utf8CP> const* propertyAccessStrings, Utf8CP ecsqlOptions)
    {
    const bool hasAccessStrings = propertyAccessStrings != nullptr && !propertyAccessStrings->empty();
    if (hasAccessStrings && properties.size() != propertyAccessStrings->size())
        {
        BeAssert(false);
        m_isValid = false;
        return;
        }

    Utf8String ecsql("UPDATE ONLY ");
    ecsql.append(m_ecClass.GetECSqlName()).append(" SET ");

    ECPropertyCP currentTimeStampProp = nullptr;
    const bool hasCurrentTimeStampProp = ECInstanceAdapterHelper::TryGetCurrentTimeStampProperty(currentTimeStampProp, m_ecClass);
    const bool readonlyPropsAreUpdatable = ECInstanceAdapterHelper::HasReadonlyPropertiesAreUpdatableOption(m_ecdb, m_ecClass, ecsqlOptions);

    const size_t propCount = properties.size();
    ECEnablerP enabler = m_ecClass.GetDefaultStandaloneEnabler();
    int parameterIndex = 1;
    for (size_t ix = 0; ix < propCount; ix++)
        {
        ECPropertyCP prop = properties[ix];
        //Current time stamp props are populated by SQLite, so ignore them here.
        //Readonly props are ignored if they are not updatable - exception: calc props need to be updated because ECObject's evaluator is not available
        //in ECDb.
        if ((hasCurrentTimeStampProp && prop == currentTimeStampProp) ||
            (!readonlyPropsAreUpdatable && (prop->IsReadOnlyFlagSet() && !prop->IsCalculated())))
            continue;

        if (!m_needsCalculatedPropertyEvaluation)
            m_needsCalculatedPropertyEvaluation = ECInstanceAdapterHelper::IsOrContainsCalculatedProperty(*prop);

        if (parameterIndex != 1)
            ecsql.append(",");

        Utf8CP unescapedAccessString = nullptr;
        Utf8String accessString;
        if (!hasAccessStrings)
            {
            unescapedAccessString = prop->GetName().c_str();
            accessString.append("[").append(unescapedAccessString).append("]");
            }
        else
            {
            //escape each token of the access string
            unescapedAccessString = propertyAccessStrings->at(ix);
            bvector<Utf8String> tokens;
            BeStringUtilities::Split(unescapedAccessString, ".", tokens);
            BeAssert(!tokens.empty());

            bool isFirstToken = true;
            for (Utf8StringCR token : tokens)
                {
                if (!isFirstToken)
                    accessString.append(".");

                accessString.append("[").append(token).append("]");
                isFirstToken = false;
                }
            }

        ecsql.append(accessString).append("=?");

        if (SUCCESS != m_ecValueBindingInfos.AddBindingInfo(*enabler, *prop, unescapedAccessString, parameterIndex))
            {
            m_isValid = false;
            return;
            }

        parameterIndex++;
        }

    if (parameterIndex == 1)
        {
        if (properties.empty())
            LOG.errorv("ECClass '%s' doesn't have any properties. Instances of that class therefore cannot be updated.", m_ecClass.GetFullName());
        else
            LOG.errorv("ECClass '%s' only has read-only properties. Instances of that class therefore cannot be updated.", m_ecClass.GetFullName());

        m_isValid = false;
        return;
        }

    ecsql.append(" WHERE ECInstanceId=?");
    m_ecinstanceIdParameterIndex = parameterIndex;

    if (!Utf8String::IsNullOrEmpty(ecsqlOptions))
        ecsql.append(" ECSQLOPTIONS ").append(ecsqlOptions);

    const ECSqlStatus stat = m_statement.Prepare(m_ecdb, ecsql.c_str());
    m_isValid = stat.IsSuccess();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECInstanceUpdater::Impl::Update(IECInstanceCR instance) const
    {
    if (!ECInstanceAdapterHelper::Equals(instance.GetClass(), m_ecClass))
        {
        Utf8String errorMessage;
        errorMessage.Sprintf("Invalid ECInstance passed to ECInstanceUpdater. ECClass mismatch: Expected ECClass: '%s'. ECInstance's ECClass: '%s'.",
                             m_ecClass.GetFullName(), instance.GetClass().GetFullName());

        LogFailure(instance, Utf8String(errorMessage).c_str());
        return ERROR;
        }

    if (!IsValid())
        {
        LOG.errorv("ECInstanceUpdater for ECClass '%s' is invalid as the ECClass is not mapped or cannot be used for updating.", m_ecClass.GetFullName());
        return ERROR;
        }

    // ECSql does not support modifying the endpoints of a relationship instance.  First need to verify that.
    IECRelationshipInstanceCP relationshipInstance = dynamic_cast<IECRelationshipInstanceCP> (&instance);
    if (nullptr != relationshipInstance)
        {
        ECInstanceId newSourceInstanceId;
        if (SUCCESS != ECInstanceId::FromString(newSourceInstanceId, relationshipInstance->GetSource()->GetInstanceId().c_str()))
            return ERROR;

        ECInstanceId newTargetInstanceId;
        if (SUCCESS != ECInstanceId::FromString(newTargetInstanceId, relationshipInstance->GetTarget()->GetInstanceId().c_str()))
            return ERROR;

        ECClassId newSourceClassId = relationshipInstance->GetSource()->GetClass().GetId();
        ECClassId newTargetClassId = relationshipInstance->GetTarget()->GetClass().GetId();

        Utf8String ecSql("SELECT SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ");
        ecSql.append(m_ecClass.GetECSqlName()).append(" WHERE ECInstanceId=?");
        ECSqlStatement statement;
        ECSqlStatus status = statement.Prepare(m_ecdb, ecSql.c_str());
        if (!status.IsSuccess())
            return ERROR;

        ECInstanceId instanceId;
        if (SUCCESS != ECInstanceId::FromString(instanceId, instance.GetInstanceId().c_str()))
            return ERROR;

        statement.BindId(1, instanceId);
        while (BE_SQLITE_ROW == statement.Step())
            {
            ECInstanceId oldSourceInstanceId = statement.GetValueId<ECInstanceId>(0);
            ECClassId oldSourceClassId = statement.GetValueId<ECClassId>(1);
            ECInstanceId oldTargetInstanceId = statement.GetValueId<ECInstanceId>(2);
            ECClassId oldTargetClassId = statement.GetValueId<ECClassId>(3);

            if (oldSourceInstanceId.GetValue() != newSourceInstanceId.GetValue() && oldSourceClassId != newSourceClassId &&
                oldTargetInstanceId.GetValue() != newTargetInstanceId.GetValue() && oldTargetClassId != newTargetClassId)
                return ERROR;
            }

        }

    //"Pins" the internal memory buffer used by the ECDBuffer such that :
    //a) all calculated property values will be evaluated exactly once, when scope is constructed; and
    //b) addresses of all property values will not change for lifetime of scope.
    //To be used in conjunction with ECValue::SetAllowsPointersIntoInstanceMemory () to ensure
    //pointers remain valid for lifetime of scope.
    ECDBufferScope scope;
    if (m_needsCalculatedPropertyEvaluation)
        scope.Init(instance.GetECDBuffer());
    ECInstanceAdapterHelper::ECInstanceInfo instanceInfo(instance);
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
            return ERROR;
            }
        }

    //now bind ECInstanceId
    ECInstanceId ecinstanceId;
    if (SUCCESS != ECInstanceId::FromString(ecinstanceId, instance.GetInstanceId().c_str()))
        {
        Utf8String errorMessage;
        errorMessage.Sprintf("ECInstanceId '%s' is empty or not a valid ECDb ECInstanceId.", instance.GetInstanceId().c_str());
        LogFailure(instance, errorMessage.c_str());
        return ERROR;
        }

    BeAssert(ecinstanceId.IsValid());
    if (!m_statement.BindId(m_ecinstanceIdParameterIndex, ecinstanceId).IsSuccess())
        return ERROR;

    //now execute statement
    const DbResult stepStatus = m_statement.Step();

    //reset once we are done with executing the statement to put the statement in inactive state (less memory etc)
    m_statement.Reset();
    m_statement.ClearBindings();

    return (stepStatus == BE_SQLITE_DONE) ? SUCCESS : ERROR;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

