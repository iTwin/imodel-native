/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECDb/ECSqlStatement.h>
#include "ECSqlStatementImpl.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//******************* ECSqlStatement ************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle   06/2013
//---------------------------------------------------------------------------------------
ECSqlStatement::ECSqlStatement() : m_pimpl(new ECSqlStatement::Impl()) {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      06/2013
//---------------------------------------------------------------------------------------
ECSqlStatement::~ECSqlStatement()
    {
    if (m_pimpl != nullptr)
        {
        delete m_pimpl;
        m_pimpl = nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle   06/2013
//---------------------------------------------------------------------------------------
ECSqlStatement::ECSqlStatement(ECSqlStatement&& rhs)
    : m_pimpl(std::move(rhs.m_pimpl))
    {
    //nulling out the pimpl on the RHS to avoid that rhs' destructor tries to delete it
    rhs.m_pimpl = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle   06/2013
//---------------------------------------------------------------------------------------
ECSqlStatement& ECSqlStatement::operator=(ECSqlStatement&& rhs)
    {
    if (this != &rhs)
        {
        m_pimpl = std::move(rhs.m_pimpl);

        //nulling out the pimpl on the RHS to avoid that rhs' destructor tries to delete it
        rhs.m_pimpl = nullptr;
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle   06/2013
//---------------------------------------------------------------------------------------
void ECSqlStatement::Finalize() { m_pimpl->Finalize(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      06/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::Prepare(ECDbCR ecdb, Utf8CP ecsql, ECCrudWriteToken const* token, bool logErrors) { return m_pimpl->Prepare(ecdb, nullptr, ecsql, token, logErrors); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      06/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::Prepare(SchemaManagerCR schemaManager, DbCR dataSourceECDb, Utf8CP selectECSql) { return m_pimpl->Prepare(schemaManager.Main().GetECDb(), &dataSourceECDb, selectECSql, nullptr, true); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      06/2013
//---------------------------------------------------------------------------------------
bool ECSqlStatement::IsPrepared() const { return m_pimpl->IsPrepared(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     07/2013
//---------------------------------------------------------------------------------------
IECSqlBinder& ECSqlStatement::GetBinder(int parameterIndex) { return m_pimpl->GetBinder(parameterIndex); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     07/2013
//---------------------------------------------------------------------------------------
int ECSqlStatement::GetParameterIndex(Utf8CP parameterName) const { return m_pimpl->GetParameterIndex(parameterName); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Shaun.Sewall   10/2019
//---------------------------------------------------------------------------------------
int ECSqlStatement::TryGetParameterIndex(Utf8CP parameterName) const { return m_pimpl->TryGetParameterIndex(parameterName); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      06/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::ClearBindings() { return m_pimpl->ClearBindings(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      06/2013
//---------------------------------------------------------------------------------------
DbResult ECSqlStatement::Step() { return m_pimpl->Step(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      06/2013
//---------------------------------------------------------------------------------------
DbResult ECSqlStatement::Step(ECInstanceKey& ecInstanceKey) const { return m_pimpl->Step(ecInstanceKey); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      06/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::Reset() { return m_pimpl->Reset(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      06/2013
//---------------------------------------------------------------------------------------
int ECSqlStatement::GetColumnCount() const { return m_pimpl->GetColumnCount(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     03/2014
//---------------------------------------------------------------------------------------
IECSqlValue const& ECSqlStatement::GetValue(int columnIndex) const
    {
    //Reports errors (not prepared yet, index out of bounds) and uses no-op field in case of error
    return m_pimpl->GetValue(columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2014
//---------------------------------------------------------------------------------------
Utf8CP ECSqlStatement::GetECSql() const { return m_pimpl->GetECSql(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
Utf8CP ECSqlStatement::GetNativeSql() const { return m_pimpl->GetNativeSql(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2014
//---------------------------------------------------------------------------------------
ECDb const* ECSqlStatement::GetECDb() const { return m_pimpl->GetECDb(); }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan        07/2018
//---------------------------------------------------------------------------------------
uint64_t ECSqlStatement::GetHashCode() const { return m_pimpl->GetHashCode(); }

END_BENTLEY_SQLITE_EC_NAMESPACE
