/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECInstanceUpdater.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    mutable ECSqlStatement m_ecsqlStatement;
    ECValueBindingInfoCollection m_ecValueBindingInfos;
    int m_ecinstanceIdParameterIndex = -1;

    bool m_needsCalculatedPropertyEvaluation = false;
    bool m_isValid = false;

    void Initialize(ECCrudWriteToken const* writeToken, bvector<ECPropertyCP> const& properties, Utf8CP ecsqlOptions) { Initialize(writeToken, properties, nullptr, ecsqlOptions); }
    void Initialize(ECCrudWriteToken const*, bvector<uint32_t> const& propertyIndexes, Utf8CP ecsqlOptions);
    void Initialize(ECCrudWriteToken const*, bvector<ECPropertyCP> const& properties, bvector<Utf8CP> const* propertyAccessStrings, Utf8CP ecsqlOptions);

public:
    Impl(ECDbCR, ECClassCR, ECCrudWriteToken const*, Utf8CP ecsqlOptions);
    Impl(ECDbCR, IECInstanceCR, ECCrudWriteToken const*, Utf8CP ecsqlOptions);
    Impl(ECDbCR, ECClassCR, ECCrudWriteToken const*, bvector<uint32_t> const& propertyIndexesToBind, Utf8CP ecsqlOptions);
    Impl(ECDbCR, ECClassCR, ECCrudWriteToken const*, bvector<ECPropertyCP> const& properties, Utf8CP ecsqlOptions);
    ~Impl() {}

    DbResult Update(IECInstanceCR instance) const;
    bool IsValid() const { return m_isValid; }
    };

  
//*************************************************************************************
// ECInstanceUpdater
//*************************************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                   Carole.MacDonald                   02 / 14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::ECInstanceUpdater(ECDbCR ecdb, ECN::ECClassCR ecClass, ECCrudWriteToken const* writeToken, Utf8CP ecsqlOptions)
    : m_impl(new Impl(ecdb, ecClass, writeToken, ecsqlOptions))
    {}

//---------------------------------------------------------------------------------------
//@bsimethod                                   Carole.MacDonald                   08/14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::ECInstanceUpdater(ECDbCR ecdb, ECN::IECInstanceCR instance, ECCrudWriteToken const* writeToken, Utf8CP ecsqlOptions)
    : m_impl(new Impl(ecdb, instance, writeToken, ecsqlOptions))
    {}

//---------------------------------------------------------------------------------------
//@bsimethod                                   Carole.MacDonald                   09/14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::ECInstanceUpdater(ECDbCR ecdb, ECN::ECClassCR ecClass, ECCrudWriteToken const* writeToken, bvector<uint32_t> const& propertyIndexesToBind, Utf8CP ecsqlOptions)
    : m_impl(new Impl(ecdb, ecClass, writeToken, propertyIndexesToBind, ecsqlOptions))
    {}

//---------------------------------------------------------------------------------------
//@bsimethod                                   Carole.MacDonald                   02/16
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::ECInstanceUpdater(ECDbCR ecdb, ECN::ECClassCR ecClass, ECCrudWriteToken const* writeToken, bvector<ECN::ECPropertyCP> const& properties, Utf8CP ecsqlOptions)
    : m_impl(new Impl(ecdb, ecClass, writeToken, properties, ecsqlOptions))
    {}

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
bool ECInstanceUpdater::IsValid() const { return m_impl->IsValid(); }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECInstanceUpdater::Update(ECN::IECInstanceCR instance) const { return m_impl->Update(instance); }


//*************************************************************************************
// ECInstanceUpdater::Impl
//*************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::Impl::Impl(ECDbCR ecdb, ECClassCR ecClass, ECCrudWriteToken const* writeToken, Utf8CP ecsqlOptions)
    : m_ecdb(ecdb), m_ecClass(ecClass)
    {
    bvector<ECPropertyCP> properties;
    for (ECPropertyCP prop : m_ecClass.GetProperties(true))
        {
        properties.push_back(prop);
        }

    Initialize(writeToken, properties, ecsqlOptions);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald                   08/14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::Impl::Impl(ECDbCR ecdb, IECInstanceCR instance, ECCrudWriteToken const* writeToken, Utf8CP ecsqlOptions)
    : m_ecdb(ecdb), m_ecClass(instance.GetClass())
    {
    bvector<ECPropertyCP> properties;

    ECValuesCollectionPtr collection = ECValuesCollection::Create(instance);
    for (ECPropertyValueCR propertyValue : *collection)
        {
        if (propertyValue.GetValue().IsLoaded())
            properties.push_back(propertyValue.GetValueAccessor().GetECProperty());
        }

    Initialize(writeToken, properties, ecsqlOptions);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald                   02/16
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::Impl::Impl(ECDbCR ecdb, ECClassCR ecClass, ECCrudWriteToken const* writeToken, bvector<uint32_t> const& propertyIndexesToBind, Utf8CP ecsqlOptions)
    : m_ecdb(ecdb), m_ecClass(ecClass)
    {
    Initialize(writeToken, propertyIndexesToBind, ecsqlOptions);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald                   09/14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::Impl::Impl(ECDbCR ecdb, ECClassCR ecClass, ECCrudWriteToken const* writeToken, bvector<ECN::ECPropertyCP> const& propertiesToBind, Utf8CP ecsqlOptions)
    : m_ecdb(ecdb), m_ecClass(ecClass)
    {
    Initialize(writeToken, propertiesToBind, ecsqlOptions);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald                   08/14
//+---------------+---------------+---------------+---------------+---------------+------
void ECInstanceUpdater::Impl::Initialize(ECCrudWriteToken const* writeToken, bvector<uint32_t> const& propertyIndexesToBind, Utf8CP ecsqlOptions)
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
            return;
            }

        if (ecProperty->GetIsStruct())
            continue;

        properties.push_back(ecProperty);
        Utf8CP accessString = nullptr;
        if (ECObjectsStatus::Success != enabler->GetAccessString(accessString, propertyIndex))
            {
            LOG.errorv("Could not retrieve property access string in ECClass '%s' for property index %" PRIu32 ".", m_ecClass.GetFullName(), propertyIndex);
            return;
            }

        propertyAccessStrings.push_back(accessString);
        }

    return Initialize(writeToken, properties, &propertyAccessStrings, ecsqlOptions);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   05/16
//+---------------+---------------+---------------+---------------+---------------+------
void ECInstanceUpdater::Impl::Initialize(ECCrudWriteToken const* writeToken, bvector<ECPropertyCP> const& properties, bvector<Utf8CP> const* propertyAccessStrings, Utf8CP ecsqlOptions)
    {
    const bool hasAccessStrings = propertyAccessStrings != nullptr && !propertyAccessStrings->empty();
    if (hasAccessStrings && properties.size() != propertyAccessStrings->size())
        {
        BeAssert(false);
        return;
        }

    PrimitiveECPropertyCP currentTimeStampProp = nullptr;
    if (SUCCESS != CoreCustomAttributeHelper::GetCurrentTimeStampProperty(currentTimeStampProp, m_ecClass))
        {
        LOG.errorv("ECInstanceUpdater failure: Could not retrieve the 'ClassHasCurrentTimeStampProperty' custom attribute from the ECClass '%s'.",
                   m_ecClass.GetFullName());

        m_isValid = false;
        return;
        }

    const bool readonlyPropsAreUpdatable = ECInstanceAdapterHelper::HasReadonlyPropertiesAreUpdatableOption(m_ecdb, m_ecClass, ecsqlOptions);

    Utf8String ecsql("UPDATE ONLY ");
    ecsql.append(m_ecClass.GetECSqlName()).append(" SET ");

    const size_t propCount = properties.size();
    ECEnablerP enabler = m_ecClass.GetDefaultStandaloneEnabler();
    int parameterIndex = 1;
    for (size_t ix = 0; ix < propCount; ix++)
        {
        ECPropertyCP prop = properties[ix];
        //Current time stamp props are populated by SQLite, so ignore them here.
        //Readonly props are ignored if they are not updatable - exception: calc props need to be updated because ECObject's evaluator is not available
        //in ECDb.
        if ((currentTimeStampProp != nullptr && prop == currentTimeStampProp) ||
            (!readonlyPropsAreUpdatable && (prop->IsReadOnlyFlagSet() && !prop->IsCalculated())))
            continue;

        if (!m_needsCalculatedPropertyEvaluation)
            m_needsCalculatedPropertyEvaluation = ECInstanceAdapterHelper::IsOrContainsCalculatedProperty(*prop);

        if (parameterIndex != 1)
            ecsql.append(",");

        Utf8CP accessString = nullptr;
        Utf8String escapedAccessString;
        if (!hasAccessStrings)
            {
            accessString = prop->GetName().c_str();
            escapedAccessString.append("[").append(accessString).append("]");
            }
        else
            {
            accessString = propertyAccessStrings->at(ix);
            bvector<Utf8String> tokens;
            BeStringUtilities::Split(accessString, ".", tokens);
            BeAssert(!tokens.empty());

            bool isFirstToken = true;
            for (Utf8StringCR token : tokens)
                {
                if (!isFirstToken)
                    escapedAccessString.append(".");

                escapedAccessString.append("[").append(token).append("]");
                isFirstToken = false;
                }
            }


        ecsql.append(escapedAccessString).append("=?");

        if(SUCCESS != m_ecValueBindingInfos.AddBindingInfo(*enabler, *prop, accessString, parameterIndex))
            return;

        parameterIndex++;

        }

    if (parameterIndex == 1)
        {
        if (properties.empty())
            LOG.errorv("ECClass '%s' doesn't have any properties. Instances of that class therefore cannot be updated.", m_ecClass.GetFullName());
        else
            LOG.errorv("ECClass '%s' only has read-only properties. Instances of that class therefore cannot be updated.", m_ecClass.GetFullName());

        return;
        }

    ecsql.append(" WHERE " ECDBSYS_PROP_ECInstanceId "=?");
    m_ecinstanceIdParameterIndex = parameterIndex;

    if (!Utf8String::IsNullOrEmpty(ecsqlOptions))
        ecsql.append(" ECSQLOPTIONS ").append(ecsqlOptions);

    m_isValid = (ECSqlStatus::Success == m_ecsqlStatement.Prepare(m_ecdb, ecsql.c_str(), writeToken));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECInstanceUpdater::Impl::Update(IECInstanceCR instance) const
    {
    if (!ECInstanceAdapterHelper::Equals(instance.GetClass(), m_ecClass))
        {
        Utf8String errorMessage;
        errorMessage.Sprintf("Invalid ECInstance passed to ECInstanceUpdater. ECClass mismatch: Expected ECClass: '%s'. ECInstance's ECClass: '%s'.",
                             m_ecClass.GetFullName(), instance.GetClass().GetFullName());

        ECInstanceAdapterHelper::LogFailure("update", instance, errorMessage.c_str());
        return BE_SQLITE_ERROR;
        }

    if (!IsValid())
        return BE_SQLITE_ERROR;

    // ECSql does not support modifying the endpoints of a relationship instance.  First need to verify that.
    IECRelationshipInstanceCP relationshipInstance = dynamic_cast<IECRelationshipInstanceCP> (&instance);
    if ((nullptr != relationshipInstance) && (relationshipInstance->GetSource().IsValid()))
        {
        ECInstanceId newSourceInstanceId;
        if (SUCCESS != ECInstanceId::FromString(newSourceInstanceId, relationshipInstance->GetSource()->GetInstanceId().c_str()))
            return BE_SQLITE_ERROR;

        ECInstanceId newTargetInstanceId;
        if (SUCCESS != ECInstanceId::FromString(newTargetInstanceId, relationshipInstance->GetTarget()->GetInstanceId().c_str()))
            return BE_SQLITE_ERROR;

        ECClassId newSourceClassId = relationshipInstance->GetSource()->GetClass().GetId();
        ECClassId newTargetClassId = relationshipInstance->GetTarget()->GetClass().GetId();

        Utf8String ecSql("SELECT " ECDBSYS_PROP_SourceECInstanceId "," ECDBSYS_PROP_SourceECClassId "," 
                         ECDBSYS_PROP_TargetECInstanceId "," ECDBSYS_PROP_TargetECClassId " FROM ");
        ecSql.append(m_ecClass.GetECSqlName()).append(" WHERE " ECDBSYS_PROP_ECInstanceId "=?");
        ECSqlStatement statement;
        ECSqlStatus status = statement.Prepare(m_ecdb, ecSql.c_str());
        if (!status.IsSuccess())
            return BE_SQLITE_ERROR;

        ECInstanceId instanceId;
        if (SUCCESS != ECInstanceId::FromString(instanceId, instance.GetInstanceId().c_str()))
            return BE_SQLITE_ERROR;

        statement.BindId(1, instanceId);
        while (BE_SQLITE_ROW == statement.Step())
            {
            ECInstanceId oldSourceInstanceId = statement.GetValueId<ECInstanceId>(0);
            ECClassId oldSourceClassId = statement.GetValueId<ECClassId>(1);
            ECInstanceId oldTargetInstanceId = statement.GetValueId<ECInstanceId>(2);
            ECClassId oldTargetClassId = statement.GetValueId<ECClassId>(3);

            if (oldSourceInstanceId.GetValue() != newSourceInstanceId.GetValue() || oldSourceClassId != newSourceClassId ||
                oldTargetInstanceId.GetValue() != newTargetInstanceId.GetValue() || oldTargetClassId != newTargetClassId)
                return BE_SQLITE_ERROR;
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

    ECInstanceId ecinstanceId;
    if (SUCCESS != ECInstanceId::FromString(ecinstanceId, instance.GetInstanceId().c_str()))
        {
        Utf8String errorMessage;
        errorMessage.Sprintf(ECDBSYS_PROP_ECInstanceId " '%s' is empty or not a valid ECDb " ECDBSYS_PROP_ECInstanceId ".", instance.GetInstanceId().c_str());
        ECInstanceAdapterHelper::LogFailure("update", instance, errorMessage.c_str());
        return BE_SQLITE_ERROR;
        }

    BeAssert(ecinstanceId.IsValid());
    ECInstanceAdapterHelper::ECInstanceInfo instanceInfo(instance, ecinstanceId, true);

    for (auto const& bindingInfo : m_ecValueBindingInfos)
        {
        BeAssert(bindingInfo->HasECSqlParameterIndex());
        if (SUCCESS != ECInstanceAdapterHelper::BindValue(m_ecsqlStatement.GetBinder(bindingInfo->GetECSqlParameterIndex()), instanceInfo, *bindingInfo))
            {
            Utf8String errorMessage;
            errorMessage.Sprintf("Could not bind value to ECSQL parameter %d [ECSQL: '%s'].", bindingInfo->GetECSqlParameterIndex(),
                                 m_ecsqlStatement.GetECSql());
            ECInstanceAdapterHelper::LogFailure("update", instance, errorMessage.c_str());
            return BE_SQLITE_ERROR;
            }
        }

    //now bind ECInstanceId
    if (!m_ecsqlStatement.BindId(m_ecinstanceIdParameterIndex, ecinstanceId).IsSuccess())
        return BE_SQLITE_ERROR;

    //now execute statement
    const DbResult stepStatus = m_ecsqlStatement.Step();

    //reset once we are done with executing the statement to put the statement in inactive state (less memory etc)
    m_ecsqlStatement.Reset();
    m_ecsqlStatement.ClearBindings();

    if (BE_SQLITE_DONE != stepStatus)
        return stepStatus;

    return BE_SQLITE_OK;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
