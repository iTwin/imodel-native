/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlBinder.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlBinder.h"
#include "ECSqlStatementBase.h"

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
    if (m_hasToCallOnBeforeStep)
        return _OnBeforeStep ();

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlBinder::_OnBeforeStep ()
    {
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
void ECSqlBinder::OnClearBindings ()
    {
    if (m_hasToCallOnClearBindings)
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
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatusContext& ECSqlBinder::GetStatusContext () const
    {
    return m_ecsqlStatement.GetStatusContextR ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlBinder::SetError (DbResult sqliteStat, Utf8CP errorMessageHeader) const
    {
    BeAssert (m_ecsqlStatement.IsPrepared ());
    return GetStatusContext ().SetError (&m_ecsqlStatement.GetPreparedStatementP ()->GetECDbR (), sqliteStat, errorMessageHeader);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlBinder::ResetStatus () const
    {
    return GetStatusContext ().Reset ();
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
Statement::BindMakeCopy ECSqlBinder::ToBeSQliteBindMakeCopy (IECSqlBinder::MakeCopy makeCopy)
    {
    switch (makeCopy)
        {
            case IECSqlBinder::MakeCopy::No:
                return Statement::MAKE_COPY_No;

            case IECSqlBinder::MakeCopy::Yes:
            default:
                return Statement::MAKE_COPY_Yes;
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
        return ECSqlStatus::IndexOutOfBounds;

    //parameter indices are 1-based, but stored in a 0-based vector.
    binder = m_binders[static_cast<size_t> (ecsqlParameterIndex - 1)].get ();
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlParameterMap::TryGetInternalBinder (ECSqlBinder*& binder, size_t internalBinderIndex) const
    {
    if (internalBinderIndex >= m_internalSqlParameterBinders.size ())
        return ECSqlStatus::IndexOutOfBounds;

    binder = m_internalSqlParameterBinders[internalBinderIndex].get ();
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
    m_binders.push_back (std::move (binder));

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
    m_internalSqlParameterBinders.push_back (std::move (binder));
    index = m_internalSqlParameterBinders.size () - 1;
    return binderP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlParameterMap::OnBeforeStep ()
    {
    for (auto& binder : m_binders)
        {
        auto stat = binder->OnBeforeStep ();
        if (stat != ECSqlStatus::Success)
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
void ECSqlParameterMap::OnClearBindings ()
    {
    for (auto& binder : m_binders)
        {
        binder->OnClearBindings ();
        }
    }

//****************** ArrayConstraintValidator **************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2014
//---------------------------------------------------------------------------------------
ECSqlStatus ArrayConstraintValidator::Validate (ECSqlStatusContext& statusContext, ECSqlTypeInfo const& expected, uint32_t actualArrayLength)
    {
    const uint32_t expectedMinOccurs = expected.GetArrayMinOccurs ();
    const uint32_t expectedMaxOccurs = expected.GetArrayMaxOccurs ();
    if (actualArrayLength < expectedMinOccurs)
        {
        LOG.warningv ("Array to be bound to the array parameter must at least have %d element(s) as defined in the respective ECProperty.", expectedMinOccurs);
        //return statusContext.SetError (ECSqlStatus::UserError, true, "Array to be bound to the array parameter must at least have %d element(s) as defined in the respective ECProperty.", expectedMinOccurs);
        }
    else if (actualArrayLength > expectedMaxOccurs)
        {
        LOG.warningv ("Array to be bound to the array parameter must at most have %d element(s) as defined in the respective ECProperty.", expectedMaxOccurs);
        //return statusContext.SetError (ECSqlStatus::UserError, true, "Array to be bound to the array parameter must at most have %d element(s) as defined in the respective ECProperty.", expectedMaxOccurs);
        }

    return ECSqlStatus::Success;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
