/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECInstanceInserter.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
protected:
    ECDbCR m_ecdb;
    ECN::ECClassCR m_ecClass;
    mutable ECSqlStatement m_statement;
    ECValueBindingInfoCollection m_ecValueBindingInfos;
    ECSqlSystemPropertyBindingInfo* m_ecinstanceIdBindingInfo;

    bool m_needsCalculatedPropertyEvaluation;
    mutable bool m_isValid;

    void Initialize ();

    static void LogFailure (ECN::IECInstanceCR instance, Utf8CP errorMessage);

public:
    Impl (ECDbCR ecdb, ECClassCR ecClass);

    BentleyStatus Insert (ECInstanceKey& newInstanceKey, IECInstanceCR instance, bool autogenerateECInstanceId = true, ECInstanceId const* userprovidedECInstanceId = nullptr) const;
    BentleyStatus Insert (ECN::IECInstanceR instance, bool autogenerateECInstanceId = true) const;
    bool IsValid () const { return m_isValid; }
    };


//*************************************************************************************
// ECInstanceInserter
//*************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceInserter::ECInstanceInserter (ECDbCR ecdb, ECN::ECClassCR ecClass)
    {
    m_impl = new ECInstanceInserter::Impl (ecdb, ecClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceInserter::~ECInstanceInserter ()
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
bool ECInstanceInserter::IsValid () const
    {
    return m_impl->IsValid ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECInstanceInserter::Insert (ECInstanceKey& newInstanceKey, ECN::IECInstanceCR instance, bool autogenerateECInstanceId, ECInstanceId const* userprovidedECInstanceId) const
    {
    return m_impl->Insert (newInstanceKey, instance, autogenerateECInstanceId, userprovidedECInstanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECInstanceInserter::Insert (ECN::IECInstanceR instance, bool autogenerateECInstanceId) const
    {
    return m_impl->Insert (instance, autogenerateECInstanceId);
    }

//*************************************************************************************
// ECInstanceInserter::Impl
//*************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle      06/2014
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceInserter::Impl::Impl (ECDbCR ecdb, ECClassCR ecClass)
    : m_ecdb (ecdb), m_ecClass (ecClass), m_ecinstanceIdBindingInfo (nullptr), m_needsCalculatedPropertyEvaluation (false), m_isValid (false)
    {
    Initialize ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
void ECInstanceInserter::Impl::Initialize ()
    {
    ECSqlInsertBuilder builder;
    builder.InsertInto (m_ecClass);

    int parameterIndex = 1;

    //add ECInstanceId. If NULL is bound to it, ECDb will auto-generate one
    builder.AddValue (ECDbSystemSchemaHelper::ToString (ECSqlSystemProperty::ECInstanceId), "?");
    //cache the binding info as we later need to set the user provided instance id (if available)
    m_ecinstanceIdBindingInfo = m_ecValueBindingInfos.AddBindingInfo (ECValueBindingInfo::SystemPropertyKind::ECInstanceId, parameterIndex);
    parameterIndex++;

    ECPropertyCP currentTimeStampProp = nullptr;
    bool hasCurrentTimeStampProp = ECInstanceAdapterHelper::TryGetCurrentTimeStampProperty (currentTimeStampProp, m_ecClass);

    for (ECPropertyCP ecProperty : m_ecClass.GetProperties (true))
        {
        //Current time stamp props are populated by SQLite, so ignore them here.
        if (hasCurrentTimeStampProp && ecProperty == currentTimeStampProp)
            continue;

        if (!m_needsCalculatedPropertyEvaluation)
            m_needsCalculatedPropertyEvaluation = ECInstanceAdapterHelper::IsOrContainsCalculatedProperty (*ecProperty);

        Utf8String propNameSnippet ("[");
        propNameSnippet.append (ecProperty->GetName ()).append ("]");
        builder.AddValue (propNameSnippet.c_str (), "?");
        if (SUCCESS != m_ecValueBindingInfos.AddBindingInfo (m_ecClass, *ecProperty, parameterIndex))
            {
            m_isValid = false;
            return;
            }

        parameterIndex++;
        }

    const bool isRelationshipClass = m_ecClass.GetRelationshipClassCP () != nullptr;
    if (isRelationshipClass)
        {
        builder.AddValue (ECDbSystemSchemaHelper::ToString (ECSqlSystemProperty::SourceECInstanceId), "?");
        m_ecValueBindingInfos.AddBindingInfo (ECValueBindingInfo::SystemPropertyKind::SourceECInstanceId, parameterIndex);

        parameterIndex++;
        builder.AddValue (ECDbSystemSchemaHelper::ToString (ECSqlSystemProperty::SourceECClassId), "?");
        m_ecValueBindingInfos.AddBindingInfo (ECValueBindingInfo::SystemPropertyKind::SourceECClassId, parameterIndex);

        parameterIndex++;
        builder.AddValue (ECDbSystemSchemaHelper::ToString (ECSqlSystemProperty::TargetECInstanceId), "?");
        m_ecValueBindingInfos.AddBindingInfo (ECValueBindingInfo::SystemPropertyKind::TargetECInstanceId, parameterIndex);

        parameterIndex++;
        builder.AddValue (ECDbSystemSchemaHelper::ToString (ECSqlSystemProperty::TargetECClassId), "?");
        m_ecValueBindingInfos.AddBindingInfo (ECValueBindingInfo::SystemPropertyKind::TargetECClassId, parameterIndex);
        }

    const auto stat = m_statement.Prepare (m_ecdb, builder.ToString ().c_str ());
    m_isValid = stat.IsSuccess();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECInstanceInserter::Impl::Insert (ECInstanceKey& newInstanceKey, IECInstanceCR instance, bool autogenerateECInstanceId, ECInstanceId const* userProvidedECInstanceId) const
    {
    if (!IsValid ())
        {
        LOG.errorv ("ECInstanceInserter for ECClass '%s' is invalid as the ECClass is not mapped or not instantiable.", m_ecClass.GetFullName ());
        return ERROR;
        }

    if (instance.GetClass () != m_ecClass)
        {
        Utf8String errorMessage;
        errorMessage.Sprintf ("Invalid ECInstance passed to ECInstanceInserter. ECClass mismatch: Expected ECClass: '%s'. ECInstance's ECClass: '%s'.",
            m_ecClass.GetFullName (), instance.GetClass ().GetFullName ());

        LogFailure (instance, Utf8String (errorMessage).c_str ());
        return ERROR;
        }

    if (autogenerateECInstanceId && userProvidedECInstanceId != nullptr)
        {
        LogFailure (instance, "Wrong usage of ECInstanceInserter::Insert. When passing true for autogenerateECInstanceId, userprovidedECInstanceId must be nullptr.");
        return ERROR;
        }

    ECInstanceId actualUserProvidedInstanceId;
    //try to retrieve a user provided ECInstanceId as auto-generation is not wanted
    if (!autogenerateECInstanceId)
        {
        if (userProvidedECInstanceId != nullptr)
            {
            if (!userProvidedECInstanceId->IsValid ())
                {
                Utf8String errorMessage;
                errorMessage.Sprintf ("Invalid parameter for ECInstanceInserter::Insert. Parameter userprovidedECInstanceId is not a valid ECInstanceId.",
                                      m_ecClass.GetFullName ());

                LogFailure (instance, errorMessage.c_str ());
                return ERROR;
                }

            actualUserProvidedInstanceId = *userProvidedECInstanceId;
            }
        else
            {
            //user provided ECInstanceId is null -> try to retrieve it from ECInstance
            Utf8String instanceIdStr = instance.GetInstanceId ();
            if (instanceIdStr.empty ())
                {
                Utf8String errorMessage;
                errorMessage.Sprintf ("Invalid ECInstance passed to ECInstanceInserter. %s ECInstance's instance id must be set when ECInstanceId auto-generation is disabled and no user provided ECInstanceId was given explicitly.",
                                      m_ecClass.GetFullName ());

                LogFailure (instance, errorMessage.c_str ());
                return ERROR;

                }

            if (!ECInstanceIdHelper::FromString (actualUserProvidedInstanceId, instanceIdStr.c_str ()))
                {
                Utf8String errorMessage;
                errorMessage.Sprintf ("Invalid ECInstance passed to ECInstanceInserter. %s ECInstance's instance id '%s' must be of type ECInstanceId when ECInstanceId auto-gneration is disabled and no user provided ECInstanceId was given explicitly.",
                                      m_ecClass.GetFullName (), instanceIdStr.c_str ());

                LogFailure (instance, errorMessage.c_str ());
                return ERROR;
                }
            }

        BeAssert (actualUserProvidedInstanceId.IsValid ());
        }

    //"Pins" the internal memory buffer used by the ECDBuffer such that :
    //a) all calculated property values will be evaluated exactly once, when scope is constructed; and
    //b) addresses of all property values will not change for lifetime of scope.
    //To be used in conjunction with ECValue::SetAllowsPointersIntoInstanceMemory () to ensure
    //pointers remain valid for lifetime of scope.
    ECDBufferScope scope;
    if (m_needsCalculatedPropertyEvaluation)
        scope.Init (instance.GetECDBuffer ());

    ECInstanceAdapterHelper::ECInstanceInfo instanceInfo (instance, actualUserProvidedInstanceId);
    //now add parameter values for regular properties
    for (auto const& bindingInfo : m_ecValueBindingInfos)
        {
        BeAssert (bindingInfo->HasECSqlParameterIndex ());
        auto stat = ECInstanceAdapterHelper::BindValue (m_statement.GetBinder (bindingInfo->GetECSqlParameterIndex ()), instanceInfo, *bindingInfo);
        if (stat != SUCCESS)
            {
            Utf8String errorMessage;
            errorMessage.Sprintf ("Could not bind value to ECSQL parameter %d [ECSQL: '%s'].", bindingInfo->GetECSqlParameterIndex (),
                m_statement.GetECSql ());
            LogFailure (instance, errorMessage.c_str ());
            return ERROR;
            }
        }

    //now execute statement
    const DbResult stepStatus = m_statement.Step (newInstanceKey);

    //reset once we are done with executing the statement to put the statement in inactive state (less memory etc)
    m_statement.Reset();
    m_statement.ClearBindings();

    return (BE_SQLITE_DONE == stepStatus && newInstanceKey.IsValid ()) ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECInstanceInserter::Impl::Insert (ECN::IECInstanceR instance, bool autogenerateECInstanceId) const
    {
    ECInstanceKey newInstanceKey;
    auto stat = Insert (newInstanceKey, instance, autogenerateECInstanceId, nullptr);
    if (stat != SUCCESS)
        return ERROR;

    //only set instance id in ECInstance if it was auto-generated by ECDb. IF not auto-generated it hasn't changed
    //in the input ECInstance, and hence doesn't need to be set
    if (autogenerateECInstanceId)
        return ECInstanceAdapterHelper::SetECInstanceId (instance, newInstanceKey.GetECInstanceId ());
    else
        return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
void ECInstanceInserter::Impl::LogFailure (ECN::IECInstanceCR instance, Utf8CP errorMessage)
    {
    ECInstanceAdapterHelper::LogFailure ("insert", instance, errorMessage);
    }


END_BENTLEY_SQLITE_EC_NAMESPACE

