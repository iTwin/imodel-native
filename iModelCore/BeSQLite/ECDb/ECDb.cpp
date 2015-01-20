/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDb.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2012
//---------------+---------------+---------------+---------------+---------------+------
//static
DbResult ECDb::Initialize (BeFileNameCR ecdbTempDir, BeFileNameCP hostAssetsDir, BeSQLiteLib::LogErrors logSqliteErrors)
    {
    const auto stat = BeSQLiteLib::Initialize (ecdbTempDir, logSqliteErrors);
    if (stat != BE_SQLITE_OK)
        return stat;

    if (hostAssetsDir != nullptr)
        { 
        if (!hostAssetsDir->DoesPathExist ())
            {
            LOG.warningv ("ECDb::Initialize: host assets dir '%s' does not exist.", hostAssetsDir->GetNameUtf8 ().c_str ());
            BeAssert (false && "ECDb::Initialize: host assets dir does not exist!");
            }

        ECSchemaReadContext::Initialize (*hostAssetsDir);
        }

    return BE_SQLITE_OK;
    }

#if defined (_MSC_VER)
    #pragma warning (push)
    #pragma warning (disable:4355) 
#endif // defined (_MSC_VER)

// Note: It's OK to just store a reference to the ECDb base class even if it's the derived class
// is not completely defined yet, which is the point of the warning. 
//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2012
//---------------+---------------+---------------+---------------+---------------+------
ECDb::ECDb ()
 : Db (), m_ecDbStore (ECDbStore::Create (*this))
    {}

#if defined (_MSC_VER)
    #pragma warning (pop)
#endif // defined (_MSC_VER)

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2012
//---------------+---------------+---------------+---------------+---------------+------
ECDb::ECDb 
(
ECDbStorePtr sharedECDbStore
) : Db (), m_ecDbStore (sharedECDbStore)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                09/2012
//---------------+---------------+---------------+---------------+---------------+------
ECDb::~ECDb ()
    {
    CloseDb();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2014
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::_OnDbOpened ()
    {
    DbResult stat = T_Super::_OnDbOpened ();
    if (stat != BE_SQLITE_OK)
        return stat;

    return GetECR ().OnDbOpened ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2012
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::_OnDbCreated (CreateParams const& params)
    {
    DbResult stat = T_Super::_OnDbCreated (params);
    if (stat != BE_SQLITE_OK)
        return stat;

    //creates the EC profile
    BeAssert (m_ecDbStore.IsValid ());
    BeAssert (!IsReadonly ());

    return GetECR ().OnDbCreated ();
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
    DbResult stat = T_Super::_OnRepositoryIdChanged (newRepositoryId);
    if (stat != BE_SQLITE_OK)
        return stat;

    BeAssert (m_ecDbStore.IsValid ());
    return GetECR ().OnRepositoryIdChanged (newRepositoryId);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2012
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::_OnDbClose ()
    {
    BeAssert (m_ecDbStore.IsValid ());
    m_ecDbStore->Close ();
    T_Super::_OnDbClose();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                01/2015
//---------------+---------------+---------------+---------------+---------------+------
void ECDb::_OnDbChangedByOtherConnection ()
    {
    T_Super::_OnDbChangedByOtherConnection ();
    BeAssert (m_ecDbStore.IsValid ());
    m_ecDbStore->OnDbChangedByOtherConnection ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2013
//---------------+---------------+---------------+---------------+---------------+------
DbResult ECDb::_VerifySchemaVersion (Db::OpenParams const& params)
    {
    auto stat = T_Super::_VerifySchemaVersion (params);
    if (stat != BE_SQLITE_OK)
        return stat;
   
    return ECDbProfileManager::UpgradeECProfile(*this, params);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2012
//---------------+---------------+---------------+---------------+---------------+------
ECDbStorePtr ECDb::GetECDbStore () const
    {
    return m_ecDbStore;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Raman.Ramanujam                09/2012
//---------------+---------------+---------------+---------------+---------------+------
ECDbStoreCR ECDb::GetEC () const
    {
    return *m_ecDbStore;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle        12/2013
//---------------+---------------+---------------+---------------+---------------+------
ECDbStore& ECDb::GetECR () const
    {
    return *m_ecDbStore;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
