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
bool ECSqlStatement::IsInsertStatement() const { return m_pimpl->IsInsertStatement(); }

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

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::BindNavigationValue(int parameterIndex, BeInt64Id relatedInstanceId, ECN::ECClassId relationshipECClassId) {
    // Validate the relationship class ID if the ECSQL is an INSERT statement and validation is enabled
    if (const auto status = IsNavigationPropertyValid(GetBinder(parameterIndex).GetBinderInfo().GetPropertyName(), relationshipECClassId); status != ECSqlStatus::Success)
        return status;

    // Bind the navigation value
    return GetBinder(parameterIndex).BindNavigation(relatedInstanceId, relationshipECClassId);
}

ECSqlStatus ECSqlStatement::IsNavigationPropertyValid(Utf8StringCR propertyName, const ECN::ECClassId& relationshipECClassId)
    {
    if (IsInsertStatement() && GetECDb()->GetECSqlConfig().IsInsertValueValidationEnabled())
        {
        const auto relClassToValidate = GetECDb()->Schemas().GetClass(relationshipECClassId);
        if (!relClassToValidate)
            {
            LOG.errorv("The ECSql INSERT statement contains a relationship class ID '%s' which does not correspond to any EC class.", 
                relationshipECClassId.ToString().c_str());
            return ECSqlStatus::InvalidECSql;
            }

        if (!relClassToValidate->IsRelationshipClass())
            {
            LOG.errorv("The ECSql INSERT statement contains a relationship class ID '%s' which does not correspond to a valid ECRelationship class.", 
                relationshipECClassId.ToString().c_str());
            return ECSqlStatus::InvalidECSql;
            }
        
        if (Utf8String::IsNullOrEmpty(propertyName.c_str()))
            {
            LOG.errorv("The ECSql INSERT statement contains an invalid/empty navigation property name.", propertyName.c_str());
            return ECSqlStatus::InvalidECSql;
            }

        // Try to get the rel class for the nav prop
        ECSqlStatement stmt;
        auto query = SqlPrintfString(
            "SELECT c.ECInstanceId "
            "FROM meta.ECClassDef c "
            "JOIN meta.ECPropertyDef p ON c.ECInstanceId = p.NavigationRelationshipClass.Id "
            "WHERE p.Name='%s'", propertyName.c_str());

        if (stmt.Prepare(*GetECDb(), query.GetUtf8CP(), nullptr, false) != ECSqlStatus::Success || stmt.Step() != BE_SQLITE_ROW)
            {
            LOG.errorv("The ECSql INSERT statement contains an invalid navigation property '%s'. The property does not correspond to any valid navigation property.", propertyName.c_str());
            return ECSqlStatus::InvalidECSql;
            }

        // Check if the relationship class matches or is a base of the provided class ID
        const auto relClass = GetECDb()->Schemas().GetClass(stmt.GetValueId<ECClassId>(0));
        if ( relClass == nullptr)
            {
            LOG.errorv("The ECSql INSERT statement contains a relationship class id '%s' which does not correspond to any valid class in the schema.",
                relationshipECClassId.ToString().c_str());
            return ECSqlStatus::InvalidECSql;
            }

        if (relClass->GetId() == relationshipECClassId)
            return ECSqlStatus::Success;

        const auto derivedClasses = GetECDb()->Schemas().GetAllDerivedClassesInternal(*relClass);
        if (!derivedClasses.IsValid()
            || std::none_of(derivedClasses.Value().begin(), derivedClasses.Value().end(), [&relationshipECClassId](const auto& checkClass) { return checkClass->GetId() == relationshipECClassId; }))
            {
            LOG.errorv("The ECSql INSERT statement contains a relationship class id '%s' which does not correspond to any valid ECRelationship class in the schema.",
                relationshipECClassId.ToString().c_str());
            return ECSqlStatus::InvalidECSql;
            }
        }

    return ECSqlStatus::Success;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
