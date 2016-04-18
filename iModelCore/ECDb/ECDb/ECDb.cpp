/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDb.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbImpl.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2012
//---------------+---------------+---------------+---------------+---------------+------
ECDb::ECDb() : Db(), m_pimpl(nullptr) { m_pimpl = new Impl(*this); }


//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2012
//---------------+---------------+---------------+---------------+---------------+------
ECDb::~ECDb()
    {
    Destroy();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2012
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::Destroy()
    {
    //any active statements need to be finalized before attempting to close the Db.
    if (m_pimpl != nullptr)
        {
        m_pimpl->GetStatementRegistry().FinalizeStatements();

        delete m_pimpl;
        m_pimpl = nullptr;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2012
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ECDb::Initialize(BeFileNameCR ecdbTempDir, BeFileNameCP hostAssetsDir, BeSQLiteLib::LogErrors logSqliteErrors)
    {
    return Impl::Initialize(ecdbTempDir, hostAssetsDir, logSqliteErrors);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2012
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::_OnDbOpening()
    {
    DbResult stat = Db::_OnDbOpening();
    if (stat != BE_SQLITE_OK)
        return stat;

    return m_pimpl->OnDbOpening();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2012
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::_OnDbCreated(CreateParams const& params)
    {
    DbResult stat = Db::_OnDbCreated(params);
    if (stat != BE_SQLITE_OK)
        return stat;

    BeAssert(!IsReadonly());
    return m_pimpl->OnDbCreated();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                12/2012
//---------------+---------------+---------------+---------------+---------------+------
//override
DbResult ECDb::_OnBriefcaseIdChanged(BeBriefcaseId newBriefcaseId)
    {
    DbResult stat = Db::_OnBriefcaseIdChanged(newBriefcaseId);
    if (stat != BE_SQLITE_OK)
        return stat;

    return m_pimpl->OnBriefcaseIdChanged(newBriefcaseId);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2012
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::_OnDbClose()
    {
    BeAssert(m_pimpl != nullptr && "DbClose was called in destructor after pimpl was deleted.");
    Destroy();
    m_pimpl = new Impl(*this);
    Db::_OnDbClose();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                01/2015
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::_OnDbChangedByOtherConnection()
    {
    Db::_OnDbChangedByOtherConnection();
    m_pimpl->OnDbChangedByOtherConnection();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::_VerifySchemaVersion(Db::OpenParams const& params)
    {
    DbResult stat = Db::_VerifySchemaVersion(params);
    if (stat != BE_SQLITE_OK)
        return stat;

    return m_pimpl->VerifySchemaVersion(params);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                03/2014
//---------------+---------------+---------------+---------------+---------------+------
int ECDb::_OnAddFunction(DbFunction& func) const
    {
    return m_pimpl->OnAddFunction(func) == SUCCESS ? 0 : 1;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                03/2014
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::_OnRemoveFunction(DbFunction& func) const
    {
    m_pimpl->OnRemoveFunction(func);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
ECDbSchemaManagerCR ECDb::Schemas() const
    {
    return m_pimpl->Schemas();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
ECN::IECSchemaLocaterR ECDb::GetSchemaLocater() const
    {
    return m_pimpl->GetSchemaLocater();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
ECN::IECClassLocaterR ECDb::GetClassLocater() const
    {
    return m_pimpl->GetClassLocater();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2015
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::Purge(PurgeMode mode) const
    {
    return m_pimpl->Purge(mode);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2015
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDb::AddIssueListener(IIssueListener const& issueListener)
    {
    return m_pimpl->AddIssueListener(issueListener);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2015
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::RemoveIssueListener()
    {
    m_pimpl->RemoveIssueListener();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                03/2016
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::AddAppData(AppData::Key const& key, AppData* appData, bool deleteOnClearCache) const
    {
    m_pimpl->AddAppData(key, appData, deleteOnClearCache);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Raman.Ramanujam                09/2012
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::ClearECDbCache() const
    {
    m_pimpl->ClearECDbCache();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Raman.Ramanujam                09/2012
//---------------+---------------+---------------+---------------+---------------+------
ECDb::Impl& ECDb::GetECDbImplR() const
    {
    BeAssert(m_pimpl != nullptr);
    return *m_pimpl;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   09/2015
//---------------------------------------------------------------------------------------
void ECDb::IIssueListener::ReportIssue(ECDbIssueSeverity severity, Utf8CP message) const
    {
    _OnIssueReported(severity, message);
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
