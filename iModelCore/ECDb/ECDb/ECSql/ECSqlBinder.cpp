/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlBinder.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************** ECSqlBinder **************************
#ifdef ECSQLPREPAREDSTATEMENT_REFACTOR
//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlBinder::ECSqlBinder(ECSqlPrepareContext& ctx, ECSqlTypeInfo const& typeInfo, SqlParamNameGenerator& nameGen, int mappedSqlParameterCount, bool hasToCallOnBeforeStep, bool hasToCallOnClearBindings)
    : m_preparedStatement(ctx.GetPreparedStatement()), m_typeInfo(typeInfo), m_hasToCallOnBeforeStep(hasToCallOnBeforeStep), m_hasToCallOnClearBindings(hasToCallOnClearBindings)
    {
    for (int i = 0; i < mappedSqlParameterCount; i++)
        {
        m_mappedSqlParameterNames.push_back(nameGen.GetNextName());
        }
    }

#else
//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlBinder::ECSqlBinder(ECSqlPrepareContext& ctx, ECSqlTypeInfo const& typeInfo, SqlParamNameGenerator& nameGen, int mappedSqlParameterCount, bool hasToCallOnBeforeStep, bool hasToCallOnClearBindings)
    : m_preparedStatement(*ctx.GetECSqlStatementR().GetPreparedStatementP()), m_typeInfo(typeInfo), m_hasToCallOnBeforeStep(hasToCallOnBeforeStep), m_hasToCallOnClearBindings(hasToCallOnClearBindings)
    {
    for (int i = 0; i < mappedSqlParameterCount; i++)
        {
        m_mappedSqlParameterNames.push_back(nameGen.GetNextName());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlBinder::SetOnBindEventHandler(IECSqlBinder& binder)
    {
    if (m_onBindEventHandlers == nullptr)
        m_onBindEventHandlers = std::unique_ptr<std::vector<IECSqlBinder*>>(new std::vector<IECSqlBinder*>());

    BeAssert(std::find(m_onBindEventHandlers->begin(), m_onBindEventHandlers->end(), &binder) == m_onBindEventHandlers->end());

    m_onBindEventHandlers->push_back(&binder);
    return ECSqlStatus::Success;
    }
#endif
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
Statement& ECSqlBinder::GetSqliteStatement() const { return m_preparedStatement.GetSqliteStatement(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2015
//---------------------------------------------------------------------------------------
ECDbCR ECSqlBinder::GetECDb() const { return m_preparedStatement.GetECDb(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlBinder::LogSqliteError(DbResult sqliteStat, Utf8CP errorMessageHeader) const
    {
    ECDbLogger::LogSqliteError(GetECDb(), sqliteStat, errorMessageHeader);
    return ECSqlStatus(sqliteStat);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
//static
Statement::MakeCopy ECSqlBinder::ToBeSQliteBindMakeCopy(IECSqlBinder::MakeCopy makeCopy)
    {
    switch (makeCopy)
        {
            case IECSqlBinder::MakeCopy::No:
                return Statement::MakeCopy::No;

            case IECSqlBinder::MakeCopy::Yes:
            default:
                return Statement::MakeCopy::Yes;
        }
    }


//****************** ECSqlBinderFactory **************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSqlBinder> ECSqlBinderFactory::CreateBinder(ECSqlPrepareContext& ctx, ParameterExp const& parameterExp)
    {
    ECSqlBinder::SqlParamNameGenerator paramNameGen(ctx, parameterExp.GetParameterName().c_str());

    ComputedExp const* targetExp = parameterExp.GetTargetExp();
    if (targetExp != nullptr && targetExp->GetType() == Exp::Type::PropertyName)
        {
        PropertyNameExp const& propNameExp = targetExp->GetAs<PropertyNameExp>();
        ECSqlSystemPropertyInfo const& sysPropInfo = propNameExp.GetSystemPropertyInfo();
        if (sysPropInfo.IsSystemProperty() && sysPropInfo.IsId())
            return CreateIdBinder(ctx, propNameExp.GetPropertyMap(), sysPropInfo, paramNameGen);
        }

    return CreateBinder(ctx, parameterExp.GetTypeInfo(), paramNameGen);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSqlBinder> ECSqlBinderFactory::CreateBinder(ECSqlPrepareContext& ctx, ECSqlTypeInfo const& typeInfo, ECSqlBinder::SqlParamNameGenerator& nameGen)
    {
    ECSqlTypeInfo::Kind typeKind = typeInfo.GetKind();
    BeAssert(typeKind != ECSqlTypeInfo::Kind::Unset);

    switch (typeKind)
        {
            case ECSqlTypeInfo::Kind::Primitive:
            {
            switch (typeInfo.GetPrimitiveType())
                {
                    case ECN::PRIMITIVETYPE_Binary:
                    case ECN::PRIMITIVETYPE_Boolean:
                    case ECN::PRIMITIVETYPE_DateTime:
                    case ECN::PRIMITIVETYPE_Double:
                    case ECN::PRIMITIVETYPE_IGeometry:
                    case ECN::PRIMITIVETYPE_Integer:
                    case ECN::PRIMITIVETYPE_Long:
                    case ECN::PRIMITIVETYPE_String:
                        return std::unique_ptr<ECSqlBinder>(new PrimitiveECSqlBinder(ctx, typeInfo, nameGen));

                    case ECN::PRIMITIVETYPE_Point2d:
                        return std::unique_ptr<ECSqlBinder>(new PointECSqlBinder(ctx, typeInfo, false, nameGen));

                    case ECN::PRIMITIVETYPE_Point3d:
                        return std::unique_ptr<ECSqlBinder>(new PointECSqlBinder(ctx, typeInfo, true, nameGen));

                    default:
                        BeAssert(false && "Could not create parameter mapping for the given parameter exp.");
                        return nullptr;
                }
            break;
            }
            //the rare case of expressions like this: NULL IS ?
            case ECSqlTypeInfo::Kind::Null:
                return std::unique_ptr<ECSqlBinder>(new PrimitiveECSqlBinder(ctx, typeInfo, nameGen));

            case ECSqlTypeInfo::Kind::Struct:
                return std::unique_ptr<ECSqlBinder>(new StructECSqlBinder(ctx, typeInfo, nameGen));

            case ECSqlTypeInfo::Kind::PrimitiveArray:
            case ECSqlTypeInfo::Kind::StructArray:
                return std::unique_ptr<ECSqlBinder>(new ArrayECSqlBinder(ctx, typeInfo, nameGen));
            case ECSqlTypeInfo::Kind::Navigation:
                return std::unique_ptr<ECSqlBinder>(new NavigationPropertyECSqlBinder(ctx, typeInfo, nameGen));

            default:
                BeAssert(false && "ECSqlBinderFactory::CreateBinder> Unhandled ECSqlTypeInfo::Kind value.");
                return nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
std::unique_ptr<IdECSqlBinder> ECSqlBinderFactory::CreateIdBinder(ECSqlPrepareContext& ctx, PropertyMap const& propMap, ECSqlSystemPropertyInfo const& sysPropertyInfo, ECSqlBinder::SqlParamNameGenerator& paramNameGen)
    {
    if (!sysPropertyInfo.IsId())
        {
        BeAssert(false);
        return nullptr;
        }

    const bool isNoopBinder = RequiresNoopBinder(ctx, propMap, sysPropertyInfo);
    return std::unique_ptr<IdECSqlBinder>(new IdECSqlBinder(ctx, ECSqlTypeInfo(propMap), isNoopBinder, paramNameGen));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
bool ECSqlBinderFactory::RequiresNoopBinder(ECSqlPrepareContext& ctx, PropertyMap const& propMap, ECSqlSystemPropertyInfo const& sysPropertyInfo)
    {
    const ECSqlType ecsqlType = ctx.GetCurrentScope().GetECSqlType();
    if (ecsqlType == ECSqlType::Select || ecsqlType == ECSqlType::Delete ||
        (ecsqlType == ECSqlType::Update && ctx.GetCurrentScope().GetExp().GetType() != Exp::Type::AssignmentList))
        return false;


    //only INSERT and UPDATE SET clauses require no-op binders because they directly translate to columns. All other expressions
    //can use constant values for virtual columns and therefore don't need no-op binders.

    BeAssert((sysPropertyInfo.GetType() != ECSqlSystemPropertyInfo::Type::Class || sysPropertyInfo.GetClass() != ECSqlSystemPropertyInfo::Class::ECClassId) && "Inserting/updating ECClassId is not supported and should have been caught before");
    BeAssert(propMap.GetType() != PropertyMap::Type::ECClassId && "Inserting/updating ECClassId is not supported and should have been caught before");

    if (propMap.GetClassMap().GetType() == ClassMap::Type::RelationshipEndTable)
        {
        //for end table relationships we ignore 
        //* the user provided ECInstanceId as end table relationships don't have their own ECInstanceId
        //* this end's class id (foreign end class id) as it is the same the end's class ECClassId. It cannot be set through
        //an ECSQL INSERT INTO ECRel.
        RelationshipClassEndTableMap const& relClassMap = propMap.GetClassMap().GetAs<RelationshipClassEndTableMap>();
        if (sysPropertyInfo == ECSqlSystemPropertyInfo::ECInstanceId() ||
            sysPropertyInfo == (relClassMap.GetForeignEnd() == ECN::ECRelationshipEnd_Source ? ECSqlSystemPropertyInfo::SourceECClassId() : ECSqlSystemPropertyInfo::TargetECClassId()))
            return true;
        }

    ClassMap const& classMap = propMap.GetClassMap();
    switch (propMap.GetType())
        {
            case PropertyMap::Type::ConstraintECClassId:
            {
            ConstraintECClassIdPropertyMap const& constraintClassIdPropMap = propMap.GetAs<ConstraintECClassIdPropertyMap>();
            if (nullptr != ConstraintECClassIdJoinInfo::RequiresJoinTo(constraintClassIdPropMap, true /*ignoreVirtualColumnCheck*/))
                return true;

            BeAssert(classMap.GetTables().size() == 1 && constraintClassIdPropMap.GetTables().size() == 1);
            DbTable const& contextTable = classMap.GetJoinedOrPrimaryTable();
            if (!classMap.GetUpdatableViewInfo().HasView() && contextTable.GetPersistenceType() == PersistenceType::Virtual)
                return true; //if table is virtual and there is no alternative table to use this is a noop.

            ConstraintECClassIdPropertyMap::PerTableIdPropertyMap const* contextConstraintClassIdPropMap = constraintClassIdPropMap.FindDataPropertyMap(contextTable);
            BeAssert(contextConstraintClassIdPropMap != nullptr);
            return contextConstraintClassIdPropMap->GetColumn().GetPersistenceType() == PersistenceType::Virtual;
            }
            case PropertyMap::Type::NavigationRelECClassId:
            {
            DbColumn const& col = propMap.GetAs<NavigationPropertyMap::RelECClassIdPropertyMap>().GetColumn();
            if (!classMap.GetUpdatableViewInfo().HasView() && col.GetTable().GetPersistenceType() == PersistenceType::Virtual)
                return true;

            return col.GetPersistenceType() == PersistenceType::Virtual;
            }
            default:
                return false;
        }
    }



//****************** ECSqlParameterMap **************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
bool ECSqlParameterMap::Contains(int& ecsqlParameterIndex, Utf8StringCR ecsqlParameterName) const
    {
    ecsqlParameterIndex = GetIndexForName(ecsqlParameterName);
    return ecsqlParameterIndex > 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
bool ECSqlParameterMap::TryGetBinder(ECSqlBinder*& binder, Utf8StringCR ecsqlParameterName) const
    {
    int ecsqlParameterIndex = -1;
    if (!Contains(ecsqlParameterIndex, ecsqlParameterName))
        return false;

    BeAssert(ecsqlParameterIndex > 0);
    return TryGetBinder(binder, ecsqlParameterIndex) == ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlParameterMap::TryGetBinder(ECSqlBinder*& binder, int ecsqlParameterIndex) const
    {
    if (ecsqlParameterIndex <= 0 || ecsqlParameterIndex > (int) (m_binders.size()))
        return ECSqlStatus::Error;

    //parameter indices are 1-based, but stored in a 0-based vector.
    binder = m_binders[(size_t) (ecsqlParameterIndex - 1)];
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
int ECSqlParameterMap::GetIndexForName(Utf8StringCR ecsqlParameterName) const
    {
    auto it = m_nameToIndexMapping.find(ecsqlParameterName);
    if (it != m_nameToIndexMapping.end())
        return it->second;
    else
        return -1;
    }

#ifndef ECSQLPREPAREDSTATEMENT_REFACTOR
//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          10/2015
//---------------------------------------------------------------------------------------
ECSqlBinder* ECSqlParameterMap::AddProxyBinder(int ecsqlParameterIndex, ECSqlBinder& binder, Utf8StringCR parameterName)
    {
    BeAssert(ecsqlParameterIndex != 0);
    if (ecsqlParameterIndex == 0)
        return nullptr;

    ECSqlBinder* binderExist = nullptr;
    if (!parameterName.empty() && TryGetBinder(binderExist, parameterName))
        {
        BeAssert(false && "Binder with name already exist");
        return nullptr;
        }

    m_binders.insert(m_binders.begin() + (ecsqlParameterIndex - 1), &binder);
    for (auto& binder : m_nameToIndexMapping)
        {
        if (binder.second >= ecsqlParameterIndex)
            {
            binder.second = binder.second + 1;
            }
        }

    if (!parameterName.empty())
        m_nameToIndexMapping[parameterName] = ecsqlParameterIndex;

    return &binder;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          10/2015
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlParameterMap::RemapForJoinTable(ECSqlPrepareContext& ctx)
    {
    ECSqlPrepareContext::JoinedTableInfo const* joinInfo = ctx.GetJoinedTableInfo();
    if (joinInfo == nullptr)
        return ECSqlStatus::Success;

    ParentOfJoinedTableECSqlStatement* joinedTableStmt = ctx.GetECSqlStatementR().GetPreparedStatementP()->GetParentOfJoinedTableECSqlStatement();
    if (joinedTableStmt == nullptr)
        return ECSqlStatus::Success;

    auto& baseParameterMap = joinedTableStmt->GetPreparedStatementP()->GetParameterMapR();
    ECSqlPrepareContext::JoinedTableInfo::ParameterSet const& primaryMap = joinInfo->GetParameterMap().GetPrimary();
    if (!primaryMap.Empty())
        {
        for (size_t oi = primaryMap.First(); oi <= primaryMap.Last(); oi++)
            {
            ECSqlPrepareContext::JoinedTableInfo::Parameter const* param = primaryMap.Find(oi);
            if (ECSqlPrepareContext::JoinedTableInfo::Parameter const* orignalParam = param->GetOrignalParameter())
                {
                ECSqlBinder* binder = nullptr;
                if (param->IsShared())
                    {
                    ECSqlBinder* cbinder = nullptr;
                    if (param->IsNamed())
                        {
                        if (!baseParameterMap.TryGetBinder(binder, param->GetName()))
                            {
                            BeAssert(false && "Programmer Error: Failed to find named parameter in base");
                            return ECSqlStatus::Error;
                            }

                        if (!TryGetBinder(cbinder, param->GetName()))
                            {
                            BeAssert(false && "Programmer Error: Failed to find named parameter in secondary");
                            return ECSqlStatus::Error;
                            }

                        ECSqlStatus st = cbinder->SetOnBindEventHandler(*binder);
                        if (st != ECSqlStatus::Success)
                            return st;
                        }
                    else
                        {
                        ECSqlStatus st = baseParameterMap.TryGetBinder(binder, (int) (param->GetIndex()));
                        if (st != ECSqlStatus::Success)
                            {
                            BeAssert(false && "Programmer Error: Parameter order is not correct.");
                            return st;
                            }

                        st = TryGetBinder(cbinder, (int) (orignalParam->GetIndex()));
                        if (st != ECSqlStatus::Success)
                            {
                            BeAssert(false && "Programmer Error: Failed to find named parameter in secondary");
                            return st;
                            }

                        st = cbinder->SetOnBindEventHandler(*binder);
                        if (st != ECSqlStatus::Success)
                            return st;
                        }
                    }
                else
                    {
                    ECSqlStatus st = baseParameterMap.TryGetBinder(binder, (int) (param->GetIndex()));
                    if (st != ECSqlStatus::Success)
                        {
                        BeAssert(false && "Programmer Error: Parameter order is not correct.");
                        return st;
                        }

                    AddProxyBinder((int) (orignalParam->GetIndex()), *binder, orignalParam->GetName());
                    }
                }
            }
        }

    auto& orignalMap = joinInfo->GetParameterMap().GetOrignal();
    if (!orignalMap.Empty())
        {
        for (auto oi = orignalMap.First(); oi <= orignalMap.Last(); oi++)
            {
            ECSqlPrepareContext::JoinedTableInfo::Parameter const* param = orignalMap.Find(oi);
            if (param == nullptr || !param->IsNamed())
                continue;

            ECSqlBinder* abinder = nullptr;
            ECSqlBinder* bbinder = nullptr;

            baseParameterMap.TryGetBinder(abinder, param->GetName());
            TryGetBinder(bbinder, param->GetName());

            if (abinder == nullptr && bbinder == nullptr)
                {
                BeAssert(false && "Binding is not valid for joined table");
                return ECSqlStatus::Error;
                }
            if (abinder != nullptr && bbinder == nullptr)
                AddProxyBinder((int) (param->GetIndex()), *abinder, param->GetName());
            }
        }

    return ECSqlStatus::Success;
    }

#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlBinder* ECSqlParameterMap::AddBinder(ECSqlPrepareContext& ctx, ParameterExp const& parameterExp)
    {
    int ecsqlParameterIndex = 0;
    //unnamed parameters don't have an identity, therefore always add a new binder in that case
    if (parameterExp.IsNamedParameter() && Contains(ecsqlParameterIndex, parameterExp.GetParameterName()))
        {
        BeAssert(false && "ECSqlParameterMap::AddBinder: mapping already exists");
        return nullptr;
        }

    std::unique_ptr<ECSqlBinder> binder = ECSqlBinderFactory::CreateBinder(ctx, parameterExp);
    if (binder == nullptr)
        return nullptr;

    ECSqlBinder* binderP = binder.get(); //cache raw pointer as return value as the unique_ptr will be moved into the list
    m_ownedBinders.push_back(std::move(binder));
    m_binders.push_back(binderP);

    BeAssert(((int) m_binders.size()) == parameterExp.GetParameterIndex()); //Parameter indices are 1-based

    if (binderP->HasToCallOnBeforeStep())
        m_bindersToCallOnStep.push_back(binderP);

    if (binderP->HasToCallOnClearBindings())
        m_bindersToCallOnClearBindings.push_back(binderP);

    //insert name to index mapping. 
    if (parameterExp.IsNamedParameter())
        m_nameToIndexMapping[parameterExp.GetParameterName()] = parameterExp.GetParameterIndex();

    return binderP;
    }

#ifndef ECSQLPREPAREDSTATEMENT_REFACTOR
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlBinder* ECSqlParameterMap::AddInternalECInstanceIdBinder(ECSqlPrepareContext& ctx)
    {
    ECSqlBinder::SqlParamNameGenerator paramNameGen(ctx, ECSQLSYS_SQLPARAM_Id);
    std::unique_ptr<ECSqlBinder> binder = ECSqlBinderFactory::CreateBinder(ctx, ECSqlTypeInfo(ECN::PRIMITIVETYPE_Long), paramNameGen);
    if (binder == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    ECSqlBinder* binderP = binder.get(); //cache raw pointer as return value as the unique_ptr will be moved into the list
    m_ownedBinders.push_back(std::move(binder));
    m_internalECInstanceIdBinder = binderP;

    if (binderP->HasToCallOnBeforeStep())
        m_bindersToCallOnStep.push_back(binderP);

    if (binderP->HasToCallOnClearBindings())
        m_bindersToCallOnClearBindings.push_back(binderP);

    return binderP;
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlParameterMap::OnBeforeStep()
    {
    for (ECSqlBinder* binder : m_bindersToCallOnStep)
        {
        ECSqlStatus stat = binder->OnBeforeStep();
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
void ECSqlParameterMap::OnClearBindings()
    {
    for (ECSqlBinder* binder : m_bindersToCallOnClearBindings)
        {
        binder->OnClearBindings();
        }
    }

//****************** ArrayConstraintValidator **************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2014
//---------------------------------------------------------------------------------------
//static
ECSqlStatus ArrayConstraintValidator::Validate(ECDbCR ecdb, ECSqlTypeInfo const& expected, uint32_t actualArrayLength)
    {
    const uint32_t expectedMinOccurs = expected.GetArrayMinOccurs();
    if (actualArrayLength < expectedMinOccurs)
        {
        LOG.errorv("Array to be bound to the array parameter must at least have %" PRIu32 " element(s) as defined in the respective ECProperty.", expectedMinOccurs);
        return ECSqlStatus::Error;
        }

    return ValidateMaximum(ecdb, expected, actualArrayLength);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2015
//---------------------------------------------------------------------------------------
//static
ECSqlStatus ArrayConstraintValidator::ValidateMaximum(ECDbCR ecdb, ECSqlTypeInfo const& expected, uint32_t actualArrayLength)
    {
    const uint32_t expectedMaxOccurs = expected.GetArrayMaxOccurs();
    if (actualArrayLength > expectedMaxOccurs)
        {
        LOG.errorv("Array to be bound to the array parameter must at most have %" PRIu32 " element(s) as defined in the respective ECProperty.", expectedMaxOccurs);
        return ECSqlStatus::Error;
        }

    return ECSqlStatus::Success;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
