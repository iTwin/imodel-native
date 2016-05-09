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
        ECN::ECClassCR m_ecClass;

        virtual BentleyStatus _Update(IECInstanceCR instance) const = 0;
        virtual bool _IsValid() const = 0;

    protected:
        explicit Impl(ECN::ECClassCR ecClass)
            : m_ecClass(ecClass)
            {}

        ECN::ECClassCR GetECClass() const { return m_ecClass; }

        static void LogFailure(ECN::IECInstanceCR instance, Utf8CP errorMessage);

    public:
        virtual ~Impl()
            {}

        BentleyStatus Update(IECInstanceCR instance) const;
        bool IsValid() const;
    };

//======================================================================================
// @bsiclass                                                 Krischan.Eberle      06/2014
//+===============+===============+===============+===============+===============+======
struct ClassUpdaterImpl : ECInstanceUpdater::Impl
    {
    private:
        ECDbCR m_ecdb;
        mutable ECSqlStatement m_statement;
        ECValueBindingInfoCollection m_ecValueBindingInfos;
        int m_ecinstanceIdParameterIndex;
        bool m_needsCalculatedPropertyEvaluation;
        bool m_isValid;

        void Initialize(bvector<ECPropertyCP>& propertiesToBind);
        void Initialize(bvector<uint32_t>& propertiesToBind);

        virtual BentleyStatus _Update(IECInstanceCR instance) const override;
        virtual bool _IsValid() const override { return m_isValid; }

    public:
        ClassUpdaterImpl(ECDbCR ecdb, ECClassCR ecClass);
        ClassUpdaterImpl(ECDbCR ecdb, IECInstanceCR instance);
        ClassUpdaterImpl(ECDbCR ecdb, ECClassCR ecClass, bvector<uint32_t>& propertiesToBind);
        ClassUpdaterImpl(ECDbCR ecdb, ECClassCR ecClass, bvector<ECPropertyCP>& propertiesToBind);

        ~ClassUpdaterImpl()
            {}
    };


//************************************** Implementation part **************************

//*************************************************************************************
// ECInstanceUpdater
//*************************************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                   Carole.MacDonald                   02 / 14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::ECInstanceUpdater(ECDbCR ecdb, ECN::ECClassCR ecClass)
    {
    m_impl = new ClassUpdaterImpl(ecdb, ecClass);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Carole.MacDonald                   08/14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::ECInstanceUpdater(ECDbCR ecdb, ECN::IECInstanceCR instance)
    {
    m_impl = new ClassUpdaterImpl(ecdb, instance);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Carole.MacDonald                   09/14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::ECInstanceUpdater(ECDbCR ecdb, ECN::ECClassCR ecClass, bvector<uint32_t>& propertiesToBind)
    {
    m_impl = new ClassUpdaterImpl(ecdb, ecClass, propertiesToBind);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Carole.MacDonald                   02/16
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::ECInstanceUpdater(ECDbCR ecdb, ECN::ECClassCR ecClass, bvector<ECN::ECPropertyCP>& propertiesToBind)
    {
    m_impl = new ClassUpdaterImpl(ecdb, ecClass, propertiesToBind);
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

    return _Update(instance);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
bool ECInstanceUpdater::Impl::IsValid() const
    {
    return _IsValid();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
void ECInstanceUpdater::Impl::LogFailure(ECN::IECInstanceCR instance, Utf8CP errorMessage)
    {
    ECInstanceAdapterHelper::LogFailure("update", instance, errorMessage);
    }

//*************************************************************************************
// ClassUpdaterImpl
//*************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
ClassUpdaterImpl::ClassUpdaterImpl(ECDbCR ecdb, ECClassCR ecClass)
    : Impl(ecClass), m_ecdb(ecdb), m_isValid(false), m_needsCalculatedPropertyEvaluation(false)
    {
    bvector<ECPropertyCP> propertiesToBind;
    for (ECPropertyCP ecProperty : GetECClass().GetProperties(true))
        {
        if (ecProperty->IsReadOnlyFlagSet() && ecProperty->GetIsReadOnly() && !ecProperty->IsCalculated())
            {
            LOG.warningv("ECProperty '%s.%s' is marked as 'Readonly' and therefore will be skipped during update operations", ecProperty->GetClass().GetFullName(), ecProperty->GetName().c_str());
            continue;
            }

        propertiesToBind.push_back(ecProperty);
        }

    Initialize(propertiesToBind);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald                   08/14
//+---------------+---------------+---------------+---------------+---------------+------
ClassUpdaterImpl::ClassUpdaterImpl(ECDbCR ecdb, IECInstanceCR instance)
    : Impl(instance.GetClass()), m_ecdb(ecdb), m_isValid(false), m_needsCalculatedPropertyEvaluation(false)
    {
    bvector<ECPropertyCP> propertiesToBind;

    ECValuesCollectionPtr collection = ECValuesCollection::Create(instance);
    for (ECPropertyValueCR propertyValue : *collection)
        {
        if (propertyValue.GetValue().IsLoaded())
            propertiesToBind.push_back(propertyValue.GetValueAccessor().GetECProperty());
        }

    Initialize(propertiesToBind);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald                   02/16
//+---------------+---------------+---------------+---------------+---------------+------
ClassUpdaterImpl::ClassUpdaterImpl(ECDbCR ecdb, ECClassCR ecClass, bvector<uint32_t>& propertiesToBind)
    : Impl(ecClass), m_ecdb(ecdb), m_isValid(false), m_needsCalculatedPropertyEvaluation(false)
    {
    Initialize(propertiesToBind);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald                   09/14
//+---------------+---------------+---------------+---------------+---------------+------
ClassUpdaterImpl::ClassUpdaterImpl(ECDbCR ecdb, ECClassCR ecClass, bvector<ECN::ECPropertyCP>& propertiesToBind)
    : Impl(ecClass), m_ecdb(ecdb), m_isValid(false), m_needsCalculatedPropertyEvaluation(false)
    {
    Initialize(propertiesToBind);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald                   08/14
//+---------------+---------------+---------------+---------------+---------------+------
void ClassUpdaterImpl::Initialize(bvector<uint32_t>& propertiesToBind)
    {
    if (propertiesToBind.size() < 1)
        {
        LOG.errorv("ECClass '%s' doesn't have any properties. Instances of that class therefore cannot be updated.",
                   GetECClass().GetFullName());

        m_isValid = false;
        return;
        }

    Utf8String ecsql("UPDATE ONLY ");
    ecsql.append(GetECClass().GetECSqlName()).append(" SET ");

    ECPropertyCP currentTimeStampProp = nullptr;
    bool hasCurrentTimeStampProp = ECInstanceAdapterHelper::TryGetCurrentTimeStampProperty(currentTimeStampProp, GetECClass());

    int parameterIndex = 1;
    ECEnablerP enabler = GetECClass().GetDefaultStandaloneEnabler();
    for (uint32_t propertyIndex : propertiesToBind)
        {
        ECPropertyCP ecProperty = enabler->LookupECProperty(propertyIndex);

        //Current time stamp props are populated by SQLite, so ignore them here.
        if (hasCurrentTimeStampProp && ecProperty == currentTimeStampProp)
            continue;

        if (ecProperty->GetIsStruct())
            continue;

        if (!m_needsCalculatedPropertyEvaluation)
            m_needsCalculatedPropertyEvaluation = ECInstanceAdapterHelper::IsOrContainsCalculatedProperty(*ecProperty);

        Utf8String propNameSnippet("[");
        Utf8CP accessString;
        enabler->GetAccessString(accessString, propertyIndex);
        size_t offset = 0;
        Utf8String utfAccessString(accessString);
        Utf8String token;
        bool firstEntry = true;
        while ((offset = utfAccessString.GetNextToken(token, ".", offset)) != Utf8String::npos)
            {
            if (!firstEntry)
                propNameSnippet.append(".[");
            propNameSnippet.append(token).append("]");
            firstEntry = false;
            }

        if (parameterIndex != 1)
            ecsql.append(",");

        ecsql.append(propNameSnippet).append("=?");

        if (SUCCESS != m_ecValueBindingInfos.AddBindingInfo(*enabler, *ecProperty, accessString, parameterIndex))
            {
            m_isValid = false;
            return;
            }

        parameterIndex++;
        }

    ecsql.append(" WHERE ECInstanceId=?");
    m_ecinstanceIdParameterIndex = parameterIndex;

    ECSqlStatus stat = m_statement.Prepare(m_ecdb, ecsql.c_str());
    m_isValid = (stat.IsSuccess());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald                   08/14
//+---------------+---------------+---------------+---------------+---------------+------
void ClassUpdaterImpl::Initialize(bvector<ECPropertyCP>& propertiesToBind)
    {
    if (propertiesToBind.size() < 1)
        {
        LOG.errorv("ECClass '%s' doesn't have any properties. Instances of that class therefore cannot be updated.",
                   GetECClass().GetFullName());

        m_isValid = false;
        return;
        }

    Utf8String ecsql("UPDATE ONLY ");
    ecsql.append(GetECClass().GetECSqlName()).append(" SET ");

    ECPropertyCP currentTimeStampProp = nullptr;
    bool hasCurrentTimeStampProp = ECInstanceAdapterHelper::TryGetCurrentTimeStampProperty(currentTimeStampProp, GetECClass());

    int parameterIndex = 1;
    for (ECPropertyCP ecProperty : propertiesToBind)
        {
        //Current time stamp props are populated by SQLite, so ignore them here.
        if (hasCurrentTimeStampProp && ecProperty == currentTimeStampProp)
            continue;

        if (!m_needsCalculatedPropertyEvaluation)
            m_needsCalculatedPropertyEvaluation = ECInstanceAdapterHelper::IsOrContainsCalculatedProperty(*ecProperty);

        if (parameterIndex != 1)
            ecsql.append(",");

        ecsql.append("[").append(ecProperty->GetName()).append("]=?");

        if (SUCCESS != m_ecValueBindingInfos.AddBindingInfo(GetECClass(), *ecProperty, parameterIndex))
            {
            m_isValid = false;
            return;
            }

        parameterIndex++;
        }

    ecsql.append(" WHERE ECInstanceId=?");
    m_ecinstanceIdParameterIndex = parameterIndex;

    ECSqlStatus stat = m_statement.Prepare(m_ecdb, ecsql.c_str());
    m_isValid = (stat.IsSuccess());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassUpdaterImpl::_Update(IECInstanceCR instance) const
    {
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
        ecSql.append(GetECClass().GetECSqlName()).append(" WHERE ECInstanceId=?");
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

