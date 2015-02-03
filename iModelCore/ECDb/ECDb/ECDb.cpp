/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDb.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbImpl.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2012
//---------------+---------------+---------------+---------------+---------------+------
ECDb::ECDb ()
    : Db (), m_pimpl (nullptr)
    {
    m_pimpl = new Impl (*this);
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2012
//---------------+---------------+---------------+---------------+---------------+------
ECDb::~ECDb ()
    {
    CloseDb ();

    if (m_pimpl != nullptr)
        {
        delete m_pimpl;
        m_pimpl = nullptr;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2012
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ECDb::Initialize (BeFileNameCR ecdbTempDir, BeFileNameCP hostAssetsDir, BeSQLiteLib::LogErrors logSqliteErrors)
    {
    return Impl::Initialize (ecdbTempDir, hostAssetsDir, logSqliteErrors);
    }
//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2014
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::_OnDbOpened ()
    {
    DbResult stat = Db::_OnDbOpened ();
    if (stat != BE_SQLITE_OK)
        return stat;

    return m_pimpl->OnDbOpened ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2012
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::_OnDbCreated (CreateParams const& params)
    {
    DbResult stat = Db::_OnDbCreated (params);
    if (stat != BE_SQLITE_OK)
        return stat;

    BeAssert (!IsReadonly ());

    return m_pimpl->OnDbCreated (*this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                12/2012
//---------------+---------------+---------------+---------------+---------------+------
//override
DbResult ECDb::_OnRepositoryIdChanged
(
BeRepositoryId newRepositoryId
)
    {
    DbResult stat = Db::_OnRepositoryIdChanged (newRepositoryId);
    if (stat != BE_SQLITE_OK)
        return stat;

    return m_pimpl->OnRepositoryIdChanged (*this, newRepositoryId);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2012
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::_OnDbClose ()
    {
    BeAssert (m_pimpl != nullptr && "DbClose was called in destructor after pimpl was deleted.");
    if (m_pimpl != nullptr)
        {
        delete m_pimpl;
        m_pimpl = new Impl (*this);
        }

    Db::_OnDbClose ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                01/2015
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::_OnDbChangedByOtherConnection ()
    {
    Db::_OnDbChangedByOtherConnection ();
    m_pimpl->OnDbChangedByOtherConnection ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::_VerifySchemaVersion (Db::OpenParams const& params)
    {
    auto stat = Db::_VerifySchemaVersion (params);
    if (stat != BE_SQLITE_OK)
        return stat;

    return m_pimpl->VerifySchemaVersion (*this, params);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
ECDbSchemaManagerCR ECDb::GetSchemaManager () const
    {
    return m_pimpl->GetSchemaManager ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
ECN::IECSchemaLocaterR ECDb::GetSchemaLocater () const
    {
    return m_pimpl->GetSchemaLocater ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
ECN::IECClassLocaterR ECDb::GetClassLocater () const
    {
    return m_pimpl->GetClassLocater ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Raman.Ramanujam                09/2012
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::ClearCache () const
    {
    m_pimpl->ClearCache ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Raman.Ramanujam                09/2012
//---------------+---------------+---------------+---------------+---------------+------
ECDb::Impl& ECDb::GetImplR () const
    {
    BeAssert (m_pimpl != nullptr);
    return *m_pimpl;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
