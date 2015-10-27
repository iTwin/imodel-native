/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlBinder.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlBinder.h"
#include "ECSqlStatementBase.h"
#include "PrimitiveToSingleColumnECSqlBinder.h"
using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************** ECSqlBinder **************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
int ECSqlBinder::GetMappedSqlParameterCount() const
    {
    return m_mappedSqlParameterCount;
    }
ECSqlStatus ECSqlBinder::SetOnBindEventHandler(IECSqlBinder& binder)
    {
    BeAssert(m_onBindEventHandler == nullptr);
    if (dynamic_cast<PrimitiveToSingleColumnECSqlBinder const*>(&binder) == nullptr)
        {
        BeAssert(dynamic_cast<PrimitiveToSingleColumnECSqlBinder const*>(&binder) != nullptr && "Only PrimitiveToSingleColumnECSqlBinder is supported as EventHandlers");
        return ECSqlStatus::Error;
        }
    
    m_onBindEventHandler = &binder;
    return ECSqlStatus::Success;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
void ECSqlBinder::SetSqliteIndex (size_t sqliteIndex)
    {
    SetSqliteIndex (-1, sqliteIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
void ECSqlBinder::SetSqliteIndex (int ecsqlParameterComponentIndex, size_t sqliteIndex)
    {
    _SetSqliteIndex (ecsqlParameterComponentIndex, sqliteIndex);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlBinder::OnBeforeStep ()
    {
    return _OnBeforeStep ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
void ECSqlBinder::OnClearBindings ()
    {
    _OnClearBindings ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
Statement& ECSqlBinder::GetSqliteStatementR () const
    {
    BeAssert (m_ecsqlStatement.GetPreparedStatementP () != nullptr);
    return m_ecsqlStatement.GetPreparedStatementP ()->GetSqliteStatementR ();
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
ECSqlStatus ECSqlBinder::ReportError (DbResult sqliteStat, Utf8CP errorMessageHeader) const
    {
    BeAssert (m_ecsqlStatement.IsPrepared ());
    GetECDb().GetECDbImplR().GetIssueReporter().ReportSqliteIssue(ECDbIssueSeverity::Error, sqliteStat, errorMessageHeader);
    return ECSqlStatus(sqliteStat);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
NoopECSqlBinder& ECSqlBinder::GetNoopBinder (ECSqlStatus status)
    {
    return NoopECSqlBinderFactory::GetBinder (status);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
//static
Statement::MakeCopy ECSqlBinder::ToBeSQliteBindMakeCopy (IECSqlBinder::MakeCopy makeCopy)
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
bool ECSqlParameterMap::Contains (int& ecsqlParameterIndex, Utf8CP ecsqlParameterName) const
    {
    return ecsqlParameterIndex = GetIndexForName (ecsqlParameterName) > 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
bool ECSqlParameterMap::TryGetBinder (ECSqlBinder*& binder, Utf8CP ecsqlParameterName) const
    {
    int ecsqlParameterIndex = -1;
    if (!Contains (ecsqlParameterIndex, ecsqlParameterName))
        return false;

    BeAssert (ecsqlParameterIndex > 0);
    return TryGetBinder (binder, ecsqlParameterIndex) == ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlParameterMap::TryGetBinder (ECSqlBinder*& binder, int ecsqlParameterIndex) const
    {
    if (ecsqlParameterIndex <= 0 || ecsqlParameterIndex > static_cast<int> (m_binders.size ()))
        return ECSqlStatus::Error;

    //parameter indices are 1-based, but stored in a 0-based vector.
    binder = m_binders[static_cast<size_t> (ecsqlParameterIndex - 1)];
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlParameterMap::TryGetInternalBinder (ECSqlBinder*& binder, size_t internalBinderIndex) const
    {
    if (internalBinderIndex >= m_internalSqlParameterBinders.size ())
        return ECSqlStatus::Error;

    binder = m_internalSqlParameterBinders[internalBinderIndex];
    return ECSqlStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
int ECSqlParameterMap::GetIndexForName (Utf8CP ecsqlParameterName) const
    {
    auto it = m_nameToIndexMapping.find (ecsqlParameterName);
    if (it != m_nameToIndexMapping.end ())
        return it->second;
    else
        return -1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          10/2015
//---------------------------------------------------------------------------------------
ECSqlBinder* ECSqlParameterMap::AddProxyBinder(int ecsqlParameterIndex, ECSqlBinder& binder)
    {
    BeAssert(ecsqlParameterIndex != 0);
    if (ecsqlParameterIndex == 0)
        return nullptr;
    
    m_binders.insert(m_binders.begin() + (ecsqlParameterIndex - 1), &binder);
    for (auto& binder : m_nameToIndexMapping)
        {
        if (binder.second >= ecsqlParameterIndex)
            {
            binder.second = binder.second + 1;
            }
        }

    return &binder;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          10/2015
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlParameterMap::RemapForJoinTable(ECSqlPrepareContext& ctx)
    {
    ECSqlStatus st = ECSqlStatus::Success;
    auto joinInfo = ctx.GetJoinTableInfo();
    if (joinInfo == nullptr)
        return st;

    auto baseStmt = ctx.GetECSqlStatementR().GetPreparedStatementP()->GetBaseECSqlStatement();
    if (baseStmt == nullptr)
        return st;

    auto& baseParameterMap = baseStmt->GetPreparedStatementP()->GetParameterMapR();
    auto& primaryMap = joinInfo->GetParameterMap().GetPrimary();
    for (auto oi = primaryMap.First(); oi != primaryMap.Last(); oi++)
        {
        auto param = primaryMap.Find(oi);
        if (auto orignalParam = param->GetOrignalParameter())
            {
            ECSqlBinder* binder = nullptr;
            if (param->IsShared())
                {
                ECSqlBinder* cbinder = nullptr;
                if (param->IsNamed())
                    {
                    auto status = baseParameterMap.TryGetBinder(binder, param->GetName());
                    if (!status)
                        {
                        BeAssert(false && "Programmer Error: Failed to find named parameter in base");
                        return ECSqlStatus::Error;
                        }

                    if (!TryGetBinder(cbinder, param->GetName()))
                        {
                        BeAssert(false && "Programmer Error: Failed to find named parameter in secondary");
                        return ECSqlStatus::Error;
                        }

                    st = cbinder->SetOnBindEventHandler(*binder);
                    if (st != ECSqlStatus::Success)
                        return st;
                    }
                else
                    {
                    st = baseParameterMap.TryGetBinder(binder, static_cast<int>(param->GetIndex()));
                    if (st != ECSqlStatus::Success)
                        {
                        BeAssert(false && "Programmer Error: Pararameter order is not correct.");
                        return st;
                        }

                    st = TryGetBinder(cbinder, static_cast<int>(orignalParam->GetIndex()));
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
                st = baseParameterMap.TryGetBinder(binder, static_cast<int>(param->GetIndex()));
                if (st != ECSqlStatus::Success)
                    {
                    BeAssert(false && "Programmer Error: Pararameter order is not correct.");
                    return st;
                    }

                AddProxyBinder(static_cast<int>(orignalParam->GetIndex()), *binder);
                }
            }
        }

    return st;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlBinder* ECSqlParameterMap::AddBinder (ECSqlStatementBase& ecsqlStatement, ParameterExp const& parameterExp, bool targetIsVirtual, bool enforceConstraints)
    {
    int ecsqlParameterIndex = 0;
    //unnamed parameters don't have an identity, therefore always add a new binder in that case
    if (parameterExp.IsNamedParameter () && Contains (ecsqlParameterIndex, parameterExp.GetParameterName ()))
        {
        BeAssert (false && "ECSqlParameterMap::AddBinder: mapping already exists");
        return nullptr;
        }

    auto binder = ECSqlBinderFactory::CreateBinder (ecsqlStatement, parameterExp, targetIsVirtual, enforceConstraints);
    if (binder == nullptr)
        return nullptr;

    auto binderP = binder.get (); //cache raw pointer as return value as the unique_ptr will be moved into the list
    m_ownedBinders.push_back (std::move (binder));
    m_binders.push_back(binderP);

    if (binderP->HasToCallOnBeforeStep())
        m_bindersToCallOnStep.push_back(binderP);

    if (binderP->HasToCallOnClearBindings())
        m_bindersToCallOnClearBindings.push_back(binderP);

    BeAssert (static_cast<int> (m_binders.size ()) == parameterExp.GetParameterIndex ()); //Parameter indices are 1-based

    //insert name to index mapping. 
    if (parameterExp.IsNamedParameter ())
        m_nameToIndexMapping[parameterExp.GetParameterName ()] = parameterExp.GetParameterIndex ();
   
    return binderP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlBinder* ECSqlParameterMap::AddInternalBinder (size_t& index, ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& typeInfo)
    {
    auto binder = ECSqlBinderFactory::CreateBinder (ecsqlStatement, typeInfo);
    if (binder == nullptr)
        return nullptr;

    auto binderP = binder.get (); //cache raw pointer as return value as the unique_ptr will be moved into the list
    m_ownedBinders.push_back(std::move(binder));
    m_internalSqlParameterBinders.push_back (binderP);
    
    if (binderP->HasToCallOnBeforeStep())
        m_bindersToCallOnStep.push_back(binderP);

    if (binderP->HasToCallOnClearBindings())
        m_bindersToCallOnClearBindings.push_back(binderP);

    index = m_internalSqlParameterBinders.size () - 1;

    return binderP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlParameterMap::OnBeforeStep ()
    {
    for (ECSqlBinder* binder : m_bindersToCallOnStep)
        {
        ECSqlStatus stat = binder->OnBeforeStep ();
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
void ECSqlParameterMap::OnClearBindings ()
    {
    for (ECSqlBinder* binder : m_bindersToCallOnClearBindings)
        {
        binder->OnClearBindings ();
        }
    }

//****************** ArrayConstraintValidator **************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2014
//---------------------------------------------------------------------------------------
//static
ECSqlStatus ArrayConstraintValidator::Validate (ECDbCR ecdb, ECSqlTypeInfo const& expected, uint32_t actualArrayLength)
    {
    const uint32_t expectedMinOccurs = expected.GetArrayMinOccurs ();
    if (actualArrayLength < expectedMinOccurs)
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Array to be bound to the array parameter must at least have %d element(s) as defined in the respective ECProperty.", expectedMinOccurs);
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
        ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Array to be bound to the array parameter must at most have %d element(s) as defined in the respective ECProperty.", expectedMaxOccurs);
        return ECSqlStatus::Error;
        }

    return ECSqlStatus::Success;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
