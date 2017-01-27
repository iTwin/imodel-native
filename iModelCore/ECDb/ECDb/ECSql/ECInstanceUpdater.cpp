/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECInstanceUpdater.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
    struct ECSqlBuilder;
    struct CompositeECSqlStatement
        {
    public:
        struct Leaf
            {
            private:
                ECDbCR m_ecdb;
                ECSqlStatement m_statement;
                int m_ecinstanceIdParameterIndex;
                ECValueBindingInfoCollection m_ecValueBindingInfos;

            public:
                explicit Leaf(ECDbCR ecdb) : m_ecdb(ecdb), m_ecinstanceIdParameterIndex(-1) {}

                BentleyStatus Prepare(ECSqlBuilder const&, ECCrudWriteToken const*);

                ECSqlStatement& GetStatement() { return m_statement; }
                ECValueBindingInfoCollection const& GetBindingInfos() const { return m_ecValueBindingInfos; }
                ECValueBindingInfoCollection& GetBindingInfosR() { return m_ecValueBindingInfos; }

                int GetECInstanceIdParameterIndex() const { return m_ecinstanceIdParameterIndex; }
            };

    private:
        ECDbCR m_ecdb;
        std::vector<std::unique_ptr<Leaf>> m_statements;

    public:
        explicit CompositeECSqlStatement(ECDbCR ecdb) : m_ecdb(ecdb) {}

        Leaf& GetCurrent();
        void AddNext() { m_statements.push_back(std::make_unique<Leaf>(m_ecdb)); }
        std::vector<std::unique_ptr<Leaf>> const& GetStatements() const { return m_statements; }
        };

    struct ECSqlBuilder
        {
    private:
        CompositeECSqlStatement& m_compositeStatement;
        ECN::ECClassCR m_ecClass;
        ECN::ECEnablerCR m_enabler;
        Utf8CP m_ecsqlOptions;

        Utf8String m_setClause;
        uint32_t m_parameterIndex = 1;
        uint32_t m_usedOverflowColumnCount = 0;

    public:
        explicit ECSqlBuilder(CompositeECSqlStatement& compositeStatement, ECN::ECClassCR ecClass, Utf8CP ecsqlOptions) 
            : m_compositeStatement(compositeStatement), m_ecClass(ecClass), m_enabler(*ecClass.GetDefaultStandaloneEnabler()), m_ecsqlOptions(ecsqlOptions) {}

        BentleyStatus Append(ECN::ECPropertyCR prop, Utf8CP accessString, bool splitAccessString);

        uint32_t GetParameterIndex() const { return m_parameterIndex; }
        uint32_t GetUsedOverflowColumnCount() const { return m_usedOverflowColumnCount; }
        void IncrementUsedOverflowColumnCount(uint32_t increment) { m_usedOverflowColumnCount += increment; }
        Utf8String ToString() const;
        void Reset();
        };

    static const uint32_t MAX_COLUMNS_IN_OVERFLOW_FOR_UPDATE = 63;
    ECDbCR m_ecdb;
    ECN::ECClassCR m_ecClass;
    CompositeECSqlStatement m_compositeStatement;
    bool m_needsCalculatedPropertyEvaluation;
    bool m_isValid;

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
    : m_ecdb(ecdb), m_ecClass(ecClass), m_compositeStatement(ecdb), m_isValid(false), m_needsCalculatedPropertyEvaluation(false)
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
    : m_ecdb(ecdb), m_ecClass(instance.GetClass()), m_compositeStatement(ecdb), m_isValid(false), m_needsCalculatedPropertyEvaluation(false)
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
    : m_ecdb(ecdb), m_ecClass(ecClass), m_compositeStatement(ecdb), m_isValid(false), m_needsCalculatedPropertyEvaluation(false)
    {
    Initialize(writeToken, propertyIndexesToBind, ecsqlOptions);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald                   09/14
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::Impl::Impl(ECDbCR ecdb, ECClassCR ecClass, ECCrudWriteToken const* writeToken, bvector<ECN::ECPropertyCP> const& propertiesToBind, Utf8CP ecsqlOptions)
    : m_ecdb(ecdb), m_ecClass(ecClass), m_compositeStatement(ecdb), m_isValid(false), m_needsCalculatedPropertyEvaluation(false)
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

    ClassMap const* classMap = m_ecdb.Schemas().GetDbMap().GetClassMap(m_ecClass);
    BeAssert(classMap != nullptr);
    if (classMap->GetType() == ClassMap::Type::NotMapped || classMap->GetType() == ClassMap::Type::RelationshipEndTable)
        {
        Utf8CP errorDetails = nullptr;
        if (classMap->GetType() == ClassMap::Type::NotMapped)
            errorDetails = "the ECClass is not mapped to the database.";
        else
            errorDetails = "the ECClass is a foreign key type ECRelationshipClass. Instances of it cannot be updated.";
        LOG.errorv("ECClass '%s' cannot be used with ECInstanceUpdater: %s", m_ecClass.GetFullName(), errorDetails);
        return;
        }

    ECPropertyCP currentTimeStampProp = nullptr;
    const bool hasCurrentTimeStampProp = ECInstanceAdapterHelper::TryGetCurrentTimeStampProperty(currentTimeStampProp, m_ecClass);
    const bool readonlyPropsAreUpdatable = ECInstanceAdapterHelper::HasReadonlyPropertiesAreUpdatableOption(m_ecdb, m_ecClass, ecsqlOptions);

    ECSqlBuilder ecsqlBuilder(m_compositeStatement, m_ecClass, ecsqlOptions);
    const size_t propCount = properties.size();
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

        Utf8CP accessString = !hasAccessStrings ? prop->GetName().c_str() : propertyAccessStrings->at(ix);

        PropertyMap const* propMap = classMap->GetPropertyMaps().Find(accessString);
        if (propMap == nullptr)
            {
            LOG.errorv("Failed to create ECInstanceUpdater. ECProperty '%s' is not defined in ECClass %s or is not mapped to the database.", accessString, m_ecClass.GetFullName());
            return;
            }

        GetColumnsPropertyMapVisitor getColumnsVisitor(PropertyMap::Type::All, true);
        if (SUCCESS != propMap->AcceptVisitor(getColumnsVisitor))
            return;
        
        if (getColumnsVisitor.GetOverflowColumnCount() > MAX_COLUMNS_IN_OVERFLOW_FOR_UPDATE)
            {
            //if single property is mapped to more than the max, we error out. 
            LOG.errorv("Failed to create ECInstanceUpdater. ECProperty '%s' in ECClass '%s' is mapped to %" PRIu32 " virtual columns in the overflow column. "
                       "In ECSQL UPDATE an ECProperty  must not be mapped to more than %" PRIu32 " virtual columns in the overflow column. "
                       "Try breaking down the ECProperty into its member properties when constructing the ECInstanceUpdater.",
                       accessString, m_ecClass.GetFullName(), getColumnsVisitor.GetOverflowColumnCount(), MAX_COLUMNS_IN_OVERFLOW_FOR_UPDATE);
            return;
            }

        if ((ecsqlBuilder.GetUsedOverflowColumnCount() + getColumnsVisitor.GetOverflowColumnCount()) > MAX_COLUMNS_IN_OVERFLOW_FOR_UPDATE)
            {
            if (SUCCESS != m_compositeStatement.GetCurrent().Prepare(ecsqlBuilder, writeToken))
                return;

            m_compositeStatement.AddNext();
            ecsqlBuilder.Reset();
            }
        
        ecsqlBuilder.IncrementUsedOverflowColumnCount(getColumnsVisitor.GetOverflowColumnCount());

        if (SUCCESS != ecsqlBuilder.Append(*prop, accessString, hasAccessStrings))
            return;
        }

    if (m_compositeStatement.GetStatements().empty())
        {
        if (properties.empty())
            LOG.errorv("ECClass '%s' doesn't have any properties. Instances of that class therefore cannot be updated.", m_ecClass.GetFullName());
        else
            LOG.errorv("ECClass '%s' only has read-only properties. Instances of that class therefore cannot be updated.", m_ecClass.GetFullName());

        return;
        }

    m_isValid = (SUCCESS == m_compositeStatement.GetCurrent().Prepare(ecsqlBuilder, writeToken));
    ecsqlBuilder.Reset();
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
    if (nullptr != relationshipInstance)
        {
        ECInstanceId newSourceInstanceId;
        if (SUCCESS != ECInstanceId::FromString(newSourceInstanceId, relationshipInstance->GetSource()->GetInstanceId().c_str()))
            return BE_SQLITE_ERROR;

        ECInstanceId newTargetInstanceId;
        if (SUCCESS != ECInstanceId::FromString(newTargetInstanceId, relationshipInstance->GetTarget()->GetInstanceId().c_str()))
            return BE_SQLITE_ERROR;

        ECClassId newSourceClassId = relationshipInstance->GetSource()->GetClass().GetId();
        ECClassId newTargetClassId = relationshipInstance->GetTarget()->GetClass().GetId();

        Utf8String ecSql("SELECT SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ");
        ecSql.append(m_ecClass.GetECSqlName()).append(" WHERE ECInstanceId=?");
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

            if (oldSourceInstanceId.GetValue() != newSourceInstanceId.GetValue() && oldSourceClassId != newSourceClassId &&
                oldTargetInstanceId.GetValue() != newTargetInstanceId.GetValue() && oldTargetClassId != newTargetClassId)
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
        errorMessage.Sprintf("ECInstanceId '%s' is empty or not a valid ECDb ECInstanceId.", instance.GetInstanceId().c_str());
        ECInstanceAdapterHelper::LogFailure("update", instance, errorMessage.c_str());
        return BE_SQLITE_ERROR;
        }

    BeAssert(ecinstanceId.IsValid());
    ECInstanceAdapterHelper::ECInstanceInfo instanceInfo(instance, ecinstanceId, true);

    for (std::unique_ptr<CompositeECSqlStatement::Leaf> const& leafStatement : m_compositeStatement.GetStatements())
        {
        ECSqlStatement& stmt = leafStatement->GetStatement();

        //now add parameter values for regular properties
        for (auto const& bindingInfo : leafStatement->GetBindingInfos())
            {
            BeAssert(bindingInfo->HasECSqlParameterIndex());
            if (SUCCESS != ECInstanceAdapterHelper::BindValue(stmt.GetBinder(bindingInfo->GetECSqlParameterIndex()), instanceInfo, *bindingInfo))
                {
                Utf8String errorMessage;
                errorMessage.Sprintf("Could not bind value to ECSQL parameter %d [ECSQL: '%s'].", bindingInfo->GetECSqlParameterIndex(),
                                     stmt.GetECSql());
                ECInstanceAdapterHelper::LogFailure("update", instance, errorMessage.c_str());
                return BE_SQLITE_ERROR;
                }
            }

        //now bind ECInstanceId
        if (!stmt.BindId(leafStatement->GetECInstanceIdParameterIndex(), ecinstanceId).IsSuccess())
            return BE_SQLITE_ERROR;

        //now execute statement
        const DbResult stepStatus = stmt.Step();

        //reset once we are done with executing the statement to put the statement in inactive state (less memory etc)
        stmt.Reset();
        stmt.ClearBindings();

        if (BE_SQLITE_DONE != stepStatus)
            return stepStatus;
        }

    return BE_SQLITE_OK;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   01/17
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceUpdater::Impl::CompositeECSqlStatement::Leaf& ECInstanceUpdater::Impl::CompositeECSqlStatement::GetCurrent()
    {
    if (m_statements.empty())
        AddNext();

    return *m_statements.back();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   01/17
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECInstanceUpdater::Impl::CompositeECSqlStatement::Leaf::Prepare(ECSqlBuilder const& ecsqlBuilder, ECCrudWriteToken const* writeToken)
    {
    Utf8String ecsql = ecsqlBuilder.ToString();
    if (ECSqlStatus::Success != m_statement.Prepare(m_ecdb, ecsql.c_str(), writeToken))
        return ERROR;

    m_ecinstanceIdParameterIndex = ecsqlBuilder.GetParameterIndex();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   01/17
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECInstanceUpdater::Impl::ECSqlBuilder::Append(ECN::ECPropertyCR prop, Utf8CP accessString, bool splitAccessString)
    {
    if (!m_setClause.empty())
        m_setClause.append(",");

    Utf8String escapedAccessString;
    if (!splitAccessString)
        escapedAccessString.append("[").append(accessString).append("]");
    else
        {
        //escape each token of the access string
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

    m_setClause.append(escapedAccessString).append("=?");

    if (SUCCESS != m_compositeStatement.GetCurrent().GetBindingInfosR().AddBindingInfo(m_enabler, prop, accessString, (int) m_parameterIndex))
        return ERROR;

    m_parameterIndex++;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   01/17
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECInstanceUpdater::Impl::ECSqlBuilder::ToString() const
    {
    Utf8String ecsql;
    ecsql.Sprintf("UPDATE ONLY %s SET %s WHERE ECInstanceId=?", m_ecClass.GetECSqlName().c_str(),
                  m_setClause.c_str());

    if (!Utf8String::IsNullOrEmpty(m_ecsqlOptions))
        ecsql.append(" ECSQLOPTIONS ").append(m_ecsqlOptions);

    return ecsql;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   01/17
//+---------------+---------------+---------------+---------------+---------------+------
void ECInstanceUpdater::Impl::ECSqlBuilder::Reset()
    {
    m_setClause.clear();
    m_parameterIndex = 1;
    m_usedOverflowColumnCount = 0;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
