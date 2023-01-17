/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECDb/ECSqlStatement.h>
#include "ECSqlStatementImpl.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//******************* ECSqlStatement ************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatement::ECSqlStatement() : m_pimpl(new ECSqlStatement::Impl()) {}

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatement::ECSqlStatement(ECSqlStatement&& rhs)
    : m_pimpl(std::move(rhs.m_pimpl))
    {
    //nulling out the pimpl on the RHS to avoid that rhs' destructor tries to delete it
    rhs.m_pimpl = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
bool ECSqlStatement::IsReadonly() const { return m_pimpl->IsReadonly(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ECSqlStatement::Finalize() { m_pimpl->Finalize(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::Prepare(ECDbCR ecdb, Utf8CP ecsql, ECCrudWriteToken const* token, bool logErrors) { return m_pimpl->Prepare(ecdb, nullptr, ecsql, token, logErrors); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::Prepare(SchemaManagerCR schemaManager, DbCR dataSourceECDb, Utf8CP selectECSql, bool logErrors) { return m_pimpl->Prepare(schemaManager.Main().GetECDb(), &dataSourceECDb, selectECSql, nullptr, logErrors); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ECSqlStatement::IsPrepared() const { return m_pimpl->IsPrepared(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlBinder& ECSqlStatement::GetBinder(int parameterIndex) { return m_pimpl->GetBinder(parameterIndex); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int ECSqlStatement::GetParameterIndex(Utf8CP parameterName) const { return m_pimpl->GetParameterIndex(parameterName); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int ECSqlStatement::TryGetParameterIndex(Utf8CP parameterName) const { return m_pimpl->TryGetParameterIndex(parameterName); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::ClearBindings() { return m_pimpl->ClearBindings(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ECSqlStatement::Step() { return m_pimpl->Step(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ECSqlStatement::Step(ECInstanceKey& ecInstanceKey) const { return m_pimpl->Step(ecInstanceKey); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::Reset() { return m_pimpl->Reset(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int ECSqlStatement::GetColumnCount() const { return m_pimpl->GetColumnCount(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlValue const& ECSqlStatement::GetValue(int columnIndex) const
    {
    //Reports errors (not prepared yet, index out of bounds) and uses no-op field in case of error
    return m_pimpl->GetValue(columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8CP ECSqlStatement::GetECSql() const { return m_pimpl->GetECSql(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8CP ECSqlStatement::GetNativeSql() const { return m_pimpl->GetNativeSql(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECDb const* ECSqlStatement::GetECDb() const { return m_pimpl->GetECDb(); }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Db const* ECSqlStatement::GetDataSourceDb() const { return m_pimpl->GetDataSourceDb(); }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
uint64_t ECSqlStatement::GetHashCode() const { return m_pimpl->GetHashCode(); }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
uint64_t ECSqlStatement::GetHashCode(Utf8CP ecsql) { return ECSqlStatement::Impl::GetHashCode(ecsql); }

END_BENTLEY_SQLITE_EC_NAMESPACE
