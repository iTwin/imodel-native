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

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
Statement& ECSqlBinder::GetSqliteStatementR() const
    {
    BeAssert(m_ecsqlStatement.GetPreparedStatementP() != nullptr);
    return m_ecsqlStatement.GetPreparedStatementP()->GetSqliteStatementR();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2015
//---------------------------------------------------------------------------------------
ECDbCR ECSqlBinder::GetECDb() const
    {
    BeAssert(m_ecsqlStatement.GetECDb() != nullptr);
    return *m_ecsqlStatement.GetECDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlBinder::LogSqliteError(DbResult sqliteStat, Utf8CP errorMessageHeader) const
    {
    BeAssert(m_ecsqlStatement.IsPrepared());
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
ECSqlStatus ECSqlParameterMap::TryGetInternalBinder(ECSqlBinder*& binder, size_t internalBinderIndex) const
    {
    if (internalBinderIndex >= m_internalSqlParameterBinders.size())
        return ECSqlStatus::Error;

    binder = m_internalSqlParameterBinders[internalBinderIndex];
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
                    ECSqlStatus st = baseParameterMap.TryGetBinder(binder, (int)(param->GetIndex()));
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

    auto& orignalMap = joinInfo->GetParameterMap().GetOrignal();
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

    return ECSqlStatus::Success;
    }
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

    auto binder = ECSqlBinderFactory::CreateBinder(ctx, parameterExp);
    if (binder == nullptr)
        return nullptr;

    auto binderP = binder.get(); //cache raw pointer as return value as the unique_ptr will be moved into the list
    m_ownedBinders.push_back(std::move(binder));
    m_binders.push_back(binderP);

    if (binderP->HasToCallOnBeforeStep())
        m_bindersToCallOnStep.push_back(binderP);

    if (binderP->HasToCallOnClearBindings())
        m_bindersToCallOnClearBindings.push_back(binderP);

    BeAssert(static_cast<int> (m_binders.size()) == parameterExp.GetParameterIndex()); //Parameter indices are 1-based

    //insert name to index mapping. 
    if (parameterExp.IsNamedParameter())
        m_nameToIndexMapping[parameterExp.GetParameterName()] = parameterExp.GetParameterIndex();

    return binderP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlBinder* ECSqlParameterMap::AddInternalBinder(size_t& index, ECSqlPrepareContext& ctx, ECSqlTypeInfo const& typeInfo)
    {
    auto binder = ECSqlBinderFactory::CreateBinder(ctx, typeInfo);
    if (binder == nullptr)
        return nullptr;

    auto binderP = binder.get(); //cache raw pointer as return value as the unique_ptr will be moved into the list
    m_ownedBinders.push_back(std::move(binder));
    m_internalSqlParameterBinders.push_back(binderP);

    if (binderP->HasToCallOnBeforeStep())
        m_bindersToCallOnStep.push_back(binderP);

    if (binderP->HasToCallOnClearBindings())
        m_bindersToCallOnClearBindings.push_back(binderP);

    index = m_internalSqlParameterBinders.size() - 1;

    return binderP;
    }

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
