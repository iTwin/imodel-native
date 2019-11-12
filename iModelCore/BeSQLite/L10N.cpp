/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BeSQLite/L10N.h>
#include <Logging/bentleylogging.h>

#define RUNONCE_CHECK(var)      {if (var) return; var=true;}

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE

DEFINE_KEY_METHOD (L10NLookup::SQLangDb)

//---------------------------------------------------------------------------------------
// @bsimethod                                            Shaun.Sewall     05/2013
//---------------------------------------------------------------------------------------
Utf8String L10NLookup::SQLangDb::GetString(L10N::NameSpace scope, L10N::StringId name, bool& hasString)
    {
    if (IsDbOpen())
        {
        CachedStatementPtr stmt;
        DbResult rc = GetCachedStatement(stmt, "SELECT Value FROM l10n_strings WHERE Namespace=? AND ResName=?");
        BeAssert(rc == BE_SQLITE_OK);

        stmt->BindText(1, scope.m_namespace, Statement::MakeCopy::No);
        stmt->BindText(2, name.m_str, Statement::MakeCopy::No);

        rc = stmt->Step();
        if (BE_SQLITE_ROW == rc)
            {
            hasString = true;
            return stmt->GetValueText(0);
            }
        }

    hasString = false;
    return "";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult L10NLookup::SQLangDb::Open()
    {
    if (!HasName())
        return  BE_SQLITE_ERROR;

    Db::OpenParams params(Db::OpenMode::Readonly);
    params.SetRawSQLite();

    return OpenBeSQLiteDb(m_fileName.c_str(), params);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bern.McCarty     02/2013
//---------------------------------------------------------------------------------------
L10NLookup::L10NLookup(L10N::SqlangFiles const & files) :
m_lastResortDb(files.m_default), 
m_cultureNeutralDb(files.m_cultureNeutral),
m_cultureSpecificDb(files.m_cultureSpecific)
    {
    m_initialized = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bern.McCarty     02/2013
//---------------------------------------------------------------------------------------
void L10NLookup::Initialize()
    {
    RUNONCE_CHECK (m_initialized);

    int rc = m_lastResortDb.Open();
    if (rc != BE_SQLITE_OK)
        {
        NativeLogging::LoggingManager::GetLogger(L"BeSQLite.L10N")->errorv("failed with error %x trying to open last resort sqlang file: %s", rc, m_lastResortDb.m_fileName.c_str());
        BeAssert(rc == BE_SQLITE_OK || !m_lastResortDb.HasName());
        }

    if (m_cultureNeutralDb.HasName())
        {
        rc = m_cultureNeutralDb.Open();
        if (BE_SQLITE_OK != rc)
            {
            NativeLogging::LoggingManager::GetLogger(L"BeSQLite.L10N")->errorv("failed with error %x trying to open culture neutral sqlang file: %s", rc, m_cultureNeutralDb.m_fileName.c_str());
            BeAssert(false);
            }
        }

    if (m_cultureSpecificDb.HasName())
        {
        rc = m_cultureSpecificDb.Open();
        if (BE_SQLITE_OK != rc)
            {
            NativeLogging::LoggingManager::GetLogger(L"BeSQLite.L10N")->errorv("failed with error %x trying to open culture specific sqlang file: %s", rc, m_cultureSpecificDb.m_fileName.c_str());
            BeAssert(false);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void L10NLookup::ResuspendIfNeeded()
    {
    // Addition to original fix with Suspend()/Resume(): http://graphite5-5r6.hgbranches.bentley.com/dgndb/mobiledgn/rev/0efb55a16beb
    // iOS app crashes after getting localized strings while app is in background mode. String retrieval re-initializes "suspended" L10N API.
    // Database connections lock app-bundle files, and this causes "Message from debugger: Terminated due to signal 9" crash after app 
    // finishes its background execution with endBackgroundTask.
    if (!m_initialized || !m_suspended)
        return;
    Suspend();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2014
//---------------------------------------------------------------------------------------
void L10NLookup::Suspend()
    {
    BeMutexHolder lock(m_mutex);
    
    // On iOS 8.0 on older iPads (pre Air), suspending the app crashes with the message "was suspended with locked system files" listing our app's own .sqlang.db3 files.
    // We do not understand why our personal sqlang files are considered "system" files (e.g. the address book database), but closing the files on suspend seems to be the best workaround.

    m_cultureSpecificDb.CloseDb();
    m_cultureNeutralDb.CloseDb();
    m_lastResortDb.CloseDb();

    m_initialized = false;
    m_suspended = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2014
//---------------------------------------------------------------------------------------
void L10NLookup::Resume()
    {
    BeMutexHolder lock(m_mutex);
    m_suspended = false;
    // Let the next call to Initialize() take care of starting up again.
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                            Shaun.Sewall     05/2013
//---------------------------------------------------------------------------------------
Utf8String L10NLookup::GetString(L10N::NameSpace scope, L10N::StringId name, bool* outHasString)
    {
    BeMutexHolder lock(m_mutex);
    Initialize();

    bool ALLOW_NULL_OUTPUT (hasString, outHasString);
    Utf8String  message = m_cultureSpecificDb.GetString(scope, name, hasString);
    if (!hasString)
        message = m_cultureNeutralDb.GetString(scope, name, hasString);

    if (!hasString)
        message = m_lastResortDb.GetString(scope, name, hasString);

    if (!hasString && (NULL == outHasString))
        {
        NativeLogging::LoggingManager::GetLogger(L"BeSQLite.L10N")->errorv("Attempt to lookup localized string %s %s failed.", scope, name);

#if defined (MOBILEDGN_TESTS_FAIL)
        BeAssert(false && "Attempt to lookup localized string failed.");
#endif
        }

    ResuspendIfNeeded();
    return message;
    }

BeMutex s_lookupMutex;
static L10NLookup* s_lookup = NULL;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void L10N::Shutdown()
    {
    BeMutexHolder lock(s_lookupMutex);
    DELETE_AND_CLEAR(s_lookup);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2014
//---------------------------------------------------------------------------------------
void L10N::Suspend()
    {
    BeMutexHolder lock(s_lookupMutex);
    if (NULL == s_lookup)
        {
        BeAssert(false && "Call L10N::Initialize() first");
        return;
        }

    s_lookup->Suspend();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2014
//---------------------------------------------------------------------------------------
void L10N::Resume()
    {
    BeMutexHolder lock(s_lookupMutex);
    if (NULL == s_lookup)
        {
        BeAssert(false && "Call L10N::Initialize() first");
        return;
        }

    s_lookup->Resume();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/2013
//---------------------------------------------------------------------------------------
static void checkFileExists(BeFileNameCR filename)
    {
    if (filename.IsEmpty() || filename.DoesPathExist())
        return;

    NativeLogging::LoggingManager::GetLogger(L"BeSQLite.L10N")->errorv("sqlang file does not exist: %s", Utf8String(filename.c_str()).c_str());
    BeAssert(false && "sqlang file does not exist");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bern.McCarty     02/2013
//---------------------------------------------------------------------------------------
BentleyStatus L10N::Initialize(SqlangFiles const& files)
    {
    BeMutexHolder lock(s_lookupMutex);
    if (NULL != s_lookup)  // First caller wins 
        return BSISUCCESS;

    checkFileExists(files.m_default);
    checkFileExists(files.m_cultureNeutral);
    checkFileExists(files.m_cultureSpecific);

    s_lookup = new L10NLookup(files);
    return !files.m_default.IsEmpty() ? BSISUCCESS : BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                            Shaun.Sewall     05/2013
//---------------------------------------------------------------------------------------
Utf8String L10N::GetString(NameSpace scope, StringId name, bool* hasString) 
    {
    BeMutexHolder lock(s_lookupMutex);
    if (NULL != s_lookup)
        return s_lookup->GetString(scope, name, hasString);

    BeAssert(false && "Call L10N::Initialize() first");
    if (hasString)
        *hasString = false;

    return "";
    }
