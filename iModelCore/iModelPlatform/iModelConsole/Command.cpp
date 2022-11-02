/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECDb/ECDbApi.h>
#include <ECObjects/ECJsonUtilities.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/Nullable.h>
#include "Command.h"
#include "iModelConsole.h"
#include <numeric>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

//******************************* Command ******************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void Command::Run(Session& session, Utf8StringCR args) const
    {
    session.GetIssues().Reset();
    return _Run(session, args);
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus Command::TokenizeString(std::vector<Utf8String>& tokens, WStringCR inputString, WChar delimiter, WChar delimiterEscapeChar)
    {
    const size_t inputStrLength = inputString.size();
    Utf8String token;
    size_t nextStartIndex = 0;
    while (nextStartIndex < inputStrLength)
        {
        nextStartIndex = IModelConsole::FindNextToken(token, inputString, nextStartIndex, delimiter, delimiterEscapeChar);
        tokens.push_back(token);
        token.resize(0);
        }

    return SUCCESS;
    }

//******************************* HelpCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void HelpCommand::_Run(Session& session, Utf8StringCR args) const
    {
    BeAssert(m_commandMap.size() == 27 && "Command was added or removed, please update the HelpCommand accordingly.");
    IModelConsole::WriteLine(m_commandMap.at(".help")->GetUsage().c_str());
    IModelConsole::WriteLine();
    IModelConsole::WriteLine(m_commandMap.at(".open")->GetUsage().c_str());
    IModelConsole::WriteLine(m_commandMap.at(".close")->GetUsage().c_str());
    IModelConsole::WriteLine(m_commandMap.at(".create")->GetUsage().c_str());
    IModelConsole::WriteLine(m_commandMap.at(".fileinfo")->GetUsage().c_str());
    IModelConsole::WriteLine();
    IModelConsole::WriteLine(m_commandMap.at(".ecsql")->GetUsage().c_str());
    IModelConsole::WriteLine(m_commandMap.at(".metadata")->GetUsage().c_str());
    IModelConsole::WriteLine();
    IModelConsole::WriteLine(m_commandMap.at(".createclassviews")->GetUsage().c_str());
    IModelConsole::WriteLine();
    IModelConsole::WriteLine(m_commandMap.at(".commit")->GetUsage().c_str());
    IModelConsole::WriteLine(m_commandMap.at(".rollback")->GetUsage().c_str());
    IModelConsole::WriteLine(m_commandMap.at(".attach")->GetUsage().c_str());
    IModelConsole::WriteLine(m_commandMap.at(".detach")->GetUsage().c_str());
    IModelConsole::WriteLine(m_commandMap.at(".change")->GetUsage().c_str());
    IModelConsole::WriteLine();
    IModelConsole::WriteLine(m_commandMap.at(".import")->GetUsage().c_str());
    IModelConsole::WriteLine(m_commandMap.at(".export")->GetUsage().c_str());
    IModelConsole::WriteLine(m_commandMap.at(".drop")->GetUsage().c_str());
    IModelConsole::WriteLine();
    IModelConsole::WriteLine(m_commandMap.at(".parse")->GetUsage().c_str());
    IModelConsole::WriteLine(m_commandMap.at(".dbschema")->GetUsage().c_str());
    IModelConsole::WriteLine();
    IModelConsole::WriteLine(m_commandMap.at(".sqlite")->GetUsage().c_str());
    IModelConsole::WriteLine(m_commandMap.at(".json")->GetUsage().c_str());
    IModelConsole::WriteLine();
    IModelConsole::WriteLine(m_commandMap.at(".schemastats")->GetUsage().c_str());
    IModelConsole::WriteLine();
    IModelConsole::WriteLine(m_commandMap.at(".exit")->GetUsage().c_str());
    }

//******************************* OpenCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String OpenCommand::_GetUsage() const
    {
    return " .open [readonly|readwrite|upgrade] [attachchanges] <iModel/ECDb/BeSQLite file>\r\n"
        COMMAND_USAGE_IDENT "Opens iModel, ECDb, or BeSQLite file. Default open mode: read-only.\r\n"
        COMMAND_USAGE_IDENT "if upgrade is specified, the file is upgraded if necessary.\r\n"
        COMMAND_USAGE_IDENT "if attachchanges is specified, the Change Cache file is attached (and created if necessary).\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void OpenCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    std::vector<Utf8String> args = TokenizeArgs(argsUnparsed);

    const size_t argCount = args.size();
    if (argCount == 0 || argCount > 3)
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (session.IsFileLoaded())
        {
        IModelConsole::WriteErrorLine("%s file '%s' already open. Close it first before opening another file.", session.GetFile().TypeToString(), session.GetFile().GetPath());
        return;
        }

    //default mode: read-only
    Db::OpenMode openMode = Db::OpenMode::Readonly;
    Db::ProfileUpgradeOptions profileUpgradeOptions = Db::ProfileUpgradeOptions::None;
    bool attachChangeCache = false;
    bool openAsECDb = false;

    const size_t switchCount = argCount - 1;
    if (switchCount > 0)
        {
        for (size_t i = 0; i < switchCount; i++)
            {
            Utf8String const& arg = args[i];

            if (arg.EqualsI("readwrite"))
                {
                openMode = Db::OpenMode::ReadWrite;
                profileUpgradeOptions = Db::ProfileUpgradeOptions::None;
                }
            else if (arg.EqualsI("upgrade"))
                {
                openMode = Db::OpenMode::ReadWrite;
                profileUpgradeOptions = Db::ProfileUpgradeOptions::Upgrade;
                }
            else if (arg.EqualsI("asecdb"))
                {
                openAsECDb = true;
                openMode = Db::OpenMode::Readonly;
                profileUpgradeOptions = Db::ProfileUpgradeOptions::None;
                }
            else if (arg.EqualsI("attachchanges"))
                attachChangeCache = true;
            }
        }

    BeFileName filePath;
    filePath.SetNameUtf8(args[argCount - 1].c_str());
    filePath.Trim(L"\"");
    if (!filePath.DoesPathExist())
        {
        IModelConsole::WriteErrorLine("The path '%s' does not exist.", filePath.GetNameUtf8().c_str());
        return;
        }

    Utf8CP openModeStr = nullptr;
    if (openMode == Db::OpenMode::Readonly)
        openModeStr = "read-only";
    else if (profileUpgradeOptions == Db::ProfileUpgradeOptions::None)
        openModeStr = "read-write";
    else
        openModeStr = "read-write with profile upgrade";

    Utf8CP attachChangeMessage = attachChangeCache ? " and attached Change Cache file" : "";
    //open as plain BeSQlite file first to retrieve profile infos.
    std::unique_ptr<BeSQLiteFile> sqliteFile = std::make_unique<BeSQLiteFile>();
    if (BE_SQLITE_OK != sqliteFile->GetHandleR().OpenBeSQLiteDb(filePath, Db::OpenParams(Db::OpenMode::Readonly)))
        {
        sqliteFile->GetHandleR().CloseDb();//seems that open errors do not automatically close the handle again
        IModelConsole::WriteErrorLine("Could not open file '%s'. File might not be a iModel file, ECDb file, or BeSQLite file.", filePath.GetNameUtf8().c_str());
        return;
        }

    bmap<SessionFile::ProfileInfo::Type, SessionFile::ProfileInfo> profileInfos;
    if (!sqliteFile->TryRetrieveProfileInfos(profileInfos))
        {
        sqliteFile->GetHandleR().CloseDb();//seems that open errors do not automatically close the handle again
        IModelConsole::WriteErrorLine("Could not retrieve profiles from file '%s'. Closing file again.", filePath.GetNameUtf8().c_str());
        return;
        }

    sqliteFile->GetHandleR().CloseDb();

    const bool isIModelFile = profileInfos.find(SessionFile::ProfileInfo::Type::IModel) != profileInfos.end();
    if (isIModelFile && !openAsECDb)
        {
        DbResult stat;
        Dgn::DgnDb::OpenParams params(openMode);
        params.SetProfileUpgradeOptions(profileUpgradeOptions);
        Dgn::DgnDbPtr iModel = Dgn::DgnDb::OpenDgnDb(&stat, filePath, params);
        if (BE_SQLITE_OK != stat)
            {
            IModelConsole::WriteErrorLine("Could not open file '%s'.", filePath.GetNameUtf8().c_str());
            return;
            }

        if (attachChangeCache)
            {
            if (BE_SQLITE_OK != iModel->AttachChangeCache(ECDb::GetDefaultChangeCachePath(filePath.GetNameUtf8().c_str())))
                {
                IModelConsole::WriteErrorLine("Could not attach Change Cache file to '%s'.", filePath.GetNameUtf8().c_str());
                return;
                }
            }

        session.SetFile(std::unique_ptr<SessionFile>(new IModelFile(iModel)));
        IModelConsole::WriteLine("Opened iModel file '%s' in %s mode%s.", filePath.GetNameUtf8().c_str(), openModeStr, attachChangeMessage);
        return;
        }

    if (profileInfos.find(SessionFile::ProfileInfo::Type::ECDb) != profileInfos.end())
        {
        std::unique_ptr<ECDbFile> ecdbFile = std::make_unique<ECDbFile>();
        if (BE_SQLITE_OK != ecdbFile->GetECDbHandleP()->OpenBeSQLiteDb(filePath, ECDb::OpenParams(openMode, profileUpgradeOptions)))
            {
            ecdbFile->GetHandleR().CloseDb();//seems that open errors do not automatically close the handle again
            IModelConsole::WriteErrorLine("Could not open file '%s'.", filePath.GetNameUtf8().c_str());
            return;
            }

        if (attachChangeCache)
            {
            if (BE_SQLITE_OK != ecdbFile->GetECDbHandle()->AttachChangeCache(ECDb::GetDefaultChangeCachePath(filePath.GetNameUtf8().c_str())))
                {
                IModelConsole::WriteErrorLine("Could not attach Change Cache file to '%s'.", filePath.GetNameUtf8().c_str());
                return;
                }
            }

        session.SetFile(std::move(ecdbFile));
        if (isIModelFile)
            IModelConsole::WriteLine("Opened iModel file as ECDb file '%s' in %s mode. This can damage the file as iModel validation logic is bypassed.", filePath.GetNameUtf8().c_str(), openModeStr);
        else
            IModelConsole::WriteLine("Opened ECDb file '%s' in %s mode%s.", filePath.GetNameUtf8().c_str(), openModeStr, attachChangeMessage);
        return;
        }

    if (BE_SQLITE_OK != sqliteFile->GetHandleR().OpenBeSQLiteDb(filePath, Db::OpenParams(openMode, profileUpgradeOptions)))
        {
        IModelConsole::WriteErrorLine("Could not open file '%s': %s", filePath.GetNameUtf8().c_str(), sqliteFile->GetHandle().GetLastError().c_str());
        sqliteFile->GetHandleR().CloseDb();//seems that open errors do not automatically close the handle again
        return;
        }

    session.SetFile(std::move(sqliteFile));
    IModelConsole::WriteLine("Opened BeSQLite file '%s' in %s mode.", filePath.GetNameUtf8().c_str(), openModeStr);
    }


//******************************* CloseCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void CloseCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    if (session.IsFileLoaded(true))
        {
        //need to get path before closing, because afterwards it is not available on the ECDb object anymore
        Utf8String path(session.GetFile().GetPath());
        session.Reset();
        IModelConsole::WriteLine("Closed '%s'.", path.c_str());
        }
    }

//******************************* CreateCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String CreateCommand::_GetUsage() const
    {
    return  " .create [iModel|ecdb|besqlite] [<iModel root subject label>] <file path>\r\n"
        COMMAND_USAGE_IDENT "Creates a new iModel (default), ECDb file, or BeSQLite file.\r\n"
        COMMAND_USAGE_IDENT "When creating a iModel file, passing a root subject label is mandatory.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void CreateCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    std::vector<Utf8String> args = TokenizeArgs(argsUnparsed);

    if (args.empty())
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (session.IsFileLoaded())
        {
        IModelConsole::WriteErrorLine("%s file %s already loaded. Please unload it first.", session.GetFile().TypeToString(), session.GetFile().GetPath());
        return;
        }

    BeFileName filePath;
    SessionFile::Type fileType = SessionFile::Type::IModel;
    Utf8CP rootSubjectName = nullptr;
    if (args[0].EqualsIAscii("ecdb") || args[0].EqualsIAscii("besqlite"))
        {
        if (args.size() == 1)
            {
            IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
            return;
            }

        if (args.size() > 2)
            {
            IModelConsole::WriteErrorLine("You can only specify a root subject label for iModel files. Usage: %s", GetUsage().c_str());
            return;
            }

        fileType = args[0].EqualsIAscii("ecdb") ? SessionFile::Type::ECDb : SessionFile::Type::BeSQLite;
        filePath.AssignUtf8(args[1].c_str());
        }
    else
        {
        size_t rootSubjectNameIndex = 0;
        size_t filePathIndex = 0;
        if (args[0].EqualsIAscii("iModel"))
            {
            if (args.size() != 3)
                {
                IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
                return;
                }

            rootSubjectNameIndex = 1;
            filePathIndex = 2;
            }
        else
            {
            if (args.size() != 2)
                {
                IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
                return;
                }

            rootSubjectNameIndex = 0;
            filePathIndex = 1;
            }

        rootSubjectName = args[rootSubjectNameIndex].c_str();
        filePath.AssignUtf8(args[filePathIndex].c_str());
        }

    filePath.Trim(L"\"");
    if (filePath.DoesPathExist())
        {
        IModelConsole::WriteErrorLine("Cannot create %s file %s as it already exists.", SessionFile::TypeToString(fileType), filePath.GetNameUtf8().c_str());
        return;
        }

    switch (fileType)
        {
            case SessionFile::Type::IModel:
            {
            Dgn::CreateDgnDbParams createParams(rootSubjectName);
            createParams.SetOverwriteExisting(true);

            DbResult fileStatus;
            Dgn::DgnDbPtr iModel = Dgn::DgnDb::CreateDgnDb(&fileStatus, filePath, createParams);
            if (BE_SQLITE_OK != fileStatus)
                {
                IModelConsole::WriteErrorLine("Failed to create iModel file '%s'.", filePath.GetNameUtf8().c_str());
                return;
                }

            session.SetFile(std::unique_ptr<SessionFile>(new IModelFile(iModel)));
            IModelConsole::WriteLine("Successfully created iModel file '%s' with root subject '%s'.", filePath.GetNameUtf8().c_str(), rootSubjectName);
            return;
            }

        case SessionFile::Type::ECDb:
            {
            std::unique_ptr<ECDbFile> ecdbFile = std::make_unique<ECDbFile>();
            const DbResult stat = ecdbFile->GetECDbHandleP()->CreateNewDb(filePath);
            if (BE_SQLITE_OK != stat)
                {
                IModelConsole::WriteErrorLine("Failed to create ECDb file %s. See log for details.", filePath.GetNameUtf8().c_str());
                ecdbFile->GetECDbHandleP()->AbandonChanges();
                return;
                }

            IModelConsole::WriteLine("Successfully created ECDb file %s and loaded it in read/write mode", filePath.GetNameUtf8().c_str());
            ecdbFile->GetECDbHandleP()->SaveChanges();
            session.SetFile(std::move(ecdbFile));
            return;
            }

        case SessionFile::Type::BeSQLite:
        {
        std::unique_ptr<BeSQLiteFile> sqliteFile = std::make_unique<BeSQLiteFile>();
        const DbResult stat = sqliteFile->GetHandleR().CreateNewDb(filePath);
        if (BE_SQLITE_OK != stat)
            {
            IModelConsole::WriteErrorLine("Failed to create BeSQLite file %s. See log for details.", filePath.GetNameUtf8().c_str());
            sqliteFile->GetHandleR().AbandonChanges();
            return;
            }

        IModelConsole::WriteLine("Successfully created BeSQLite file %s and loaded it in read/write mode", filePath.GetNameUtf8().c_str());
        sqliteFile->GetHandleR().SaveChanges();
        session.SetFile(std::move(sqliteFile));
        return;
        }
        }
    }

//******************************* FileInfoCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void FileInfoCommand::_Run(Session& session, Utf8StringCR args) const
    {
    if (!session.IsFileLoaded(true))
        return;

    IModelConsole::WriteLine("Current file: ");
    IModelConsole::WriteLine("  Path: %s", session.GetFile().GetPath());

    ProfileVersion initialECDbProfileVersion(0, 0, 0, 0);
    if (session.GetFile().GetType() != SessionFile::Type::BeSQLite)
        {
        IModelConsole::WriteLine("  BriefcaseId: %" PRIu32, session.GetFile().GetECDbHandle()->GetBriefcaseId().GetValue());

        if (session.GetFile().GetType() == SessionFile::Type::IModel)
            {
            Dgn::DgnDbCR iModelFile = session.GetFile().GetAs<IModelFile>().GetDgnDbHandle();
            IModelConsole::WriteLine("  Root subject: %s", iModelFile.Elements().GetRootSubject()->GetUserLabel());
            }

        Statement stmt;
        if (BE_SQLITE_OK != stmt.Prepare(session.GetFile().GetHandle(), "SELECT StrData FROM be_Prop WHERE Namespace='ec_Db' AND Name='InitialSchemaVersion'"))
            {
            IModelConsole::WriteErrorLine("Could not execute SQL to retrieve profile versions.");
            return;
            }

        if (BE_SQLITE_ROW == stmt.Step())
            initialECDbProfileVersion = ProfileVersion(stmt.GetValueText(0));
        }


    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(session.GetFile().GetHandle(), "SELECT sqlite_version()"))
        {
        IModelConsole::WriteErrorLine("Could not execute SQL to retrieve SQLite version.");
        return;
        }

    if (BE_SQLITE_ROW == stmt.Step())
        IModelConsole::WriteLine("  SQLite version: %s", stmt.GetValueText(0));

    stmt.Finalize();

    bmap<SessionFile::ProfileInfo::Type, SessionFile::ProfileInfo> profileInfos;
    if (!session.GetFile().TryRetrieveProfileInfos(profileInfos))
        {
        IModelConsole::WriteErrorLine("Could not retrieve profile infos for the current file");
        return;
        }

    IModelConsole::WriteLine("  Profiles:");
    auto it = profileInfos.find(SessionFile::ProfileInfo::Type::IModel);
    if (it != profileInfos.end())
        IModelConsole::WriteLine("    %s: %s", it->second.m_name.c_str(), it->second.m_version.ToString().c_str());

    it = profileInfos.find(SessionFile::ProfileInfo::Type::ECDb);
    if (it != profileInfos.end())
        {
        if (!initialECDbProfileVersion.IsEmpty())
            IModelConsole::WriteLine("    %s: %s (Creation version: %s)", it->second.m_name.c_str(), it->second.m_version.ToString().c_str(), initialECDbProfileVersion.ToString().c_str());
        else
            IModelConsole::WriteLine("    %s: %s", it->second.m_name.c_str(), it->second.m_version.ToString().c_str());
        }

    it = profileInfos.find(SessionFile::ProfileInfo::Type::BeSQLite);
    if (it != profileInfos.end())
        IModelConsole::WriteLine("    %s: %s", it->second.m_name.c_str(), it->second.m_version.ToString().c_str());

    for (bpair<SessionFile::ProfileInfo::Type, SessionFile::ProfileInfo> const& profileInfo : profileInfos)
        {
        if (profileInfo.first == SessionFile::ProfileInfo::Type::Unknown)
            IModelConsole::WriteLine("    %s: %s", profileInfo.second.m_name.c_str(), profileInfo.second.m_version.ToString().c_str());
        }

    // Table spaces
    IModelConsole::WriteLine("  Tablespaces:");
    if (BE_SQLITE_OK != stmt.Prepare(session.GetFile().GetHandle(), "pragma database_list"))
        {
        IModelConsole::WriteErrorLine("Could not execute SQL 'pragma database_list' to retrieve table spaces.");
        return;
        }

    while (BE_SQLITE_ROW == stmt.Step())
        {
        Utf8CP tableSpace = stmt.GetValueText(1);
        Utf8CP path = stmt.GetValueText(2);
        if (Utf8String::IsNullOrEmpty(path))
            IModelConsole::WriteLine("    %s", tableSpace);
        else
            IModelConsole::WriteLine("    %s \t(%s)", tableSpace, path);
        }

    stmt.Finalize();
    }

//******************************* CommitCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void CommitCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    if (!argsUnparsed.empty())
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        return;

    if (session.GetFile().GetHandle().IsReadonly())
        {
        IModelConsole::WriteErrorLine("File must be editable. Please close the file and re-open it in read-write mode.");
        return;
        }

    if (!session.GetFile().GetHandle().IsTransactionActive())
        {
        IModelConsole::WriteErrorLine("Cannot commit because no transaction is active.");
        return;
        }

    DbResult stat = session.GetFile().GetHandleR().SaveChanges();
    if (stat != BE_SQLITE_OK)
        IModelConsole::WriteErrorLine("Commit failed: %s", session.GetFile().GetHandle().GetLastError().c_str());
    else
        IModelConsole::WriteLine("Committed current transaction and restarted it.");
    }

//******************************* RollbackCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String RollbackCommand::_GetUsage() const
    {
    return " .rollback                      Rolls back the current transaction and restarts it.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void RollbackCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    if (!argsUnparsed.empty())
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        return;

    if (session.GetFile().GetHandle().IsReadonly())
        {
        IModelConsole::WriteErrorLine("File must be editable. Please close the file and re-open it in read-write mode.");
        return;
        }

    if (!session.GetFile().GetHandle().IsTransactionActive())
        {
        IModelConsole::WriteErrorLine("Cannot roll back because no transaction is active.");
        return;
        }

    DbResult stat = session.GetFile().GetHandleR().AbandonChanges();
    if (stat != BE_SQLITE_OK)
        IModelConsole::WriteErrorLine("Rollback failed: %s", session.GetFile().GetHandle().GetLastError().c_str());
    else
        IModelConsole::WriteLine("Rolled current transaction back and restarted it.");
    }



//******************************* AttachCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String AttachCommand::_GetUsage() const
    {
    return " .attach <file path> <table space>     Attaches the specified file to the currently open file.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void AttachCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    std::vector<Utf8String> args = TokenizeArgs(argsUnparsed);

    // unused - const size_t switchArgIndex = 0;
    if (args.size() != 2)
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        return;

    Utf8CP filePath = args[0].c_str();
    if (!BeFileName::DoesPathExist(WString(filePath, BentleyCharEncoding::Utf8).c_str()))
        {
        IModelConsole::WriteErrorLine("File to attach does not exist: %s", filePath);
        return;
        }

    Utf8StringCR tableSpaceName = args[1];
    if (session.GetFile().IsAttached(tableSpaceName))
        {
        IModelConsole::WriteErrorLine("Table space %s is already in use.", tableSpaceName.c_str());
        return;
        }

    if (BE_SQLITE_OK != session.GetFile().GetHandle().AttachDb(filePath, tableSpaceName.c_str()))
        {
        IModelConsole::WriteErrorLine("Failed to attach file %s as table space %s: %s", filePath, args[1].c_str(),
                                   session.GetFile().GetHandle().GetLastError().c_str());
        return;
        }

    IModelConsole::WriteLine("Attached file %s as table space %s.", filePath, args[1].c_str());
    }


//******************************* DetachCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String DetachCommand::_GetUsage() const
    {
    return " .detach <table space>          Detaches the table space and its backing file";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void DetachCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    std::vector<Utf8String> args = TokenizeArgs(argsUnparsed);

    // unused - const size_t switchArgIndex = 0;
    if (args.size() != 1)
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        return;

    Utf8StringCR tableSpaceName = args[0];
    if (!session.GetFile().IsAttached(tableSpaceName))
        {
        IModelConsole::WriteErrorLine("Table space %s does not exist.", tableSpaceName.c_str());
        return;
        }

    if (BE_SQLITE_OK != session.GetFile().GetHandle().DetachDb(tableSpaceName.c_str()))
        {
        IModelConsole::WriteErrorLine("Failed to detach table space %s and backing file: %s", args[0].c_str(), session.GetFile().GetHandle().GetLastError().c_str());
        return;
        }

    IModelConsole::WriteLine("Detached table space %s and backing file.", args[0].c_str());
    }

//******************************* ChangeCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String ChangeCommand::_GetUsage() const
    {
    return " .change tracking [on|off]      Enable / pause change tracking. Pausing does not create a revision.\r\n"
           "         attachcache [cache path] attaches (and creates if necessary) the Change Cache file.\r\n"
           "         detachcache            Detaches the Change Cache file, if it was attached.\r\n"
           "         extractsummary [changeset file] if a changeset file is specified, a change summary is created from it.\r\n"
           "                                Otherwise a revision from the current local changes is created and the summary extracted from it.\r\n"
           "         logchangeset changeset file    Logs the contents of the changeset file (to iModelConsole logs under category: 'Changeset').\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ChangeCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    if (!session.IsECDbFileLoaded(true))
        return;

    const std::vector<Utf8String> args = TokenizeArgs(argsUnparsed);
    if (args.size() != 1 && args.size() != 2)
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (args[0].EqualsIAscii("tracking"))
        {
        if (args.size() == 1)
            {
            IModelConsole::WriteLine("Change tracking is %s.", session.GetFile().IsTracking() ? "on" : "off");
            return;
            }

        const bool on = args[1].EqualsIAscii("on");
        if (on && session.GetFile().IsTracking())
            {
            IModelConsole::WriteLine("Change tracking is already on.");
            return;
            }

        if (!on && !session.GetFile().IsTracking())
            {
            IModelConsole::WriteLine("Change tracking is already off.");
            return;
            }

        session.GetFileR().EnableTracking(on);
        return;
        }

    if (args[0].EqualsIAscii("attachcache"))
        {
        if (session.GetFileR().GetECDbHandle()->IsChangeCacheAttached())
            {
            IModelConsole::WriteErrorLine("A Change Cache file has already been attached.");
            return;
            }

        BeFileName cachePath;
        if (args.size() == 1)
            cachePath = ECDb::GetDefaultChangeCachePath(session.GetFileR().GetECDbHandle()->GetDbFileName());
        else
            cachePath = BeFileName(args[1]);

        if (BE_SQLITE_OK != session.GetFileR().GetECDbHandle()->AttachChangeCache(cachePath))
            {
            IModelConsole::WriteErrorLine("Failed to attach Change Cache file %s.", cachePath.GetNameUtf8().c_str());
            return;
            }

        IModelConsole::WriteLine("Attached Change Cache file %s.", cachePath.GetNameUtf8().c_str());
        return;
        }

    if (args[0].EqualsIAscii("detachcache"))
        {
        if (!session.GetFileR().GetECDbHandle()->IsChangeCacheAttached())
            {
            IModelConsole::WriteErrorLine("No Change Cache file has been attached.");
            return;
            }

        if (BE_SQLITE_OK != session.GetFileR().GetECDbHandle()->DetachChangeCache())
            {
            IModelConsole::WriteErrorLine("Failed to detach Change Cache file.");
            return;
            }

        IModelConsole::WriteLine("Detached Change Cache file.");
        return;
        }

    if (args[0].EqualsIAscii("extractsummary"))
        {
        if (!session.GetFileR().GetECDbHandle()->IsChangeCacheAttached())
            {
            IModelConsole::WriteErrorLine("Failed to extract change summary. No Change Cache file is attached. Make sure to attach it first or specify a path to the Change Cache file.");
            return;
            }

        const bool fromFile = args.size() == 2;
        if (fromFile)
            {
            BeFileName changesetFilePath(args[1].c_str(), true);
            if (!changesetFilePath.DoesPathExist())
                {
                IModelConsole::WriteErrorLine("Failed to extract change summary. ChangeSet file %s does not exist.", changesetFilePath.GetNameUtf8().c_str());
                return;
                }

            if (session.GetFile().GetType() != SessionFile::Type::IModel)
                {
                IModelConsole::WriteErrorLine("No iModel open. Extracting change summaries from changeset file is not supported for ECDb files");
                return;
                }

            Dgn::RevisionChangesFileReader changeStream(changesetFilePath, session.GetFile().GetAs<IModelFile>().GetDgnDbHandle());
            PERFLOG_START("iModelConsole", "ExtractChangeSummary>ECDb::ExtractChangeSummary");
            ECInstanceKey changeSummaryKey;
            if (session.GetFileR().GetECDbHandle()->ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeStream)) != SUCCESS)
                {
                IModelConsole::WriteErrorLine("Failed to extract change summary.");
                return;
                }
            PERFLOG_FINISH("iModelConsole", "ExtractChangeSummary>ECDb::ExtractChangeSummary");
            IModelConsole::WriteLine("Successfully extracted ChangeSummary (Id: %s) from ChangeSet file.", changeSummaryKey.GetInstanceId().ToString().c_str());
            return;
            }

        IModelConsoleChangeSet changeset;
        IModelConsoleChangeTracker* tracker = session.GetFileR().GetTracker();
        if (tracker == nullptr)
            {
            IModelConsole::WriteErrorLine("No changes tracked so far. Make sure to enable change tracking before extracting a change summary.");
            return;
            }

        if (!tracker->HasChanges())
            {
            IModelConsole::WriteErrorLine("No changes tracked.");
            return;
            }

        if (changeset.FromChangeTrack(*tracker) != BE_SQLITE_OK)
            {
            IModelConsole::WriteErrorLine("Failed to retrieve changeset from local changes.");
            return;
            }

        PERFLOG_START("iModelConsole", "ExtractChangeSummary>ECDb::ExtractChangeSummary");
        ECInstanceKey changeSummaryKey;
        if (session.GetFileR().GetECDbHandle()->ExtractChangeSummary(changeSummaryKey, ChangeSetArg(changeset)) != SUCCESS)
            {
            IModelConsole::WriteErrorLine("Failed to extract change summary.");
            return;
            }
        PERFLOG_FINISH("iModelConsole", "ExtractChangeSummary>ECDb::ExtractChangeSummary");

        const bool trackingWasOn = tracker->IsTracking();
        tracker->EndTracking();//end the changeset
        IModelConsole::WriteLine("Successfully created revision and extracted ChangeSummary (Id: %s) from it.", changeSummaryKey.GetInstanceId().ToString().c_str());
        if (trackingWasOn)
            tracker->EnableTracking(true);

        return;
        }

    if (args[0].EqualsIAscii("logchangeset"))
        {
        if (args.size() < 2)
            {
            IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
            return;
            }

        BeFileName changesetFilePath(args[1].c_str(), true);
        if (!changesetFilePath.DoesPathExist())
            {
            IModelConsole::WriteErrorLine("Failed to log changeset content. ChangeSet file %s does not exist.", changesetFilePath.GetNameUtf8().c_str());
            return;
            }

        if (session.GetFile().GetType() != SessionFile::Type::IModel)
            {
            IModelConsole::WriteErrorLine("No iModel open. Logging the content if a changeset file is not supported for ECDb files.");
            return;
            }

        Dgn::RevisionChangesFileReader changeStream(changesetFilePath, session.GetFile().GetAs<IModelFile>().GetDgnDbHandle());
        changeStream.Dump(Utf8String(changesetFilePath.GetFileNameWithoutExtension().c_str()).c_str(), session.GetFile().GetAs<IModelFile>().GetHandle());
        IModelConsole::WriteLine("Successfully logged the content of the changeset file. See logging category 'Changeset' in the iModelConsole logs.");
        return;
        }

    IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
    }


//******************************* ImportCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
Utf8CP const ImportCommand::ECSCHEMA_SWITCH = "schema";
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
Utf8CP const ImportCommand::CSV_SWITCH = "csv";

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String ImportCommand::_GetUsage() const
    {
    return " .import schema <ecschema xml file|folder>\r\n"
        COMMAND_USAGE_IDENT "Imports the specified ECSchema XML file into the file. If a folder was specified, all ECSchemas\r\n"
        COMMAND_USAGE_IDENT "in the folder are imported.\r\n"
        COMMAND_USAGE_IDENT "Note: Outstanding changes are committed before starting the import.\r\n"
        "         csv <csv file path> <table name> [<hascolumnheader> <delimiter> <delimiterescapechar>]\r\n"
        COMMAND_USAGE_IDENT "Imports the specified CSV file into a plain table.\r\n"
        COMMAND_USAGE_IDENT "hascolumnheader: if true, the first line's values become the column names of the new table (default: false)\r\n"
        COMMAND_USAGE_IDENT "delimiter: token delimiter in the CSV file (default: comma)\r\n"
        COMMAND_USAGE_IDENT "nodelimiterescaping: By default, delimiters are escaped if they are enclosed by double-quotes. If this parameter\r\n"
        COMMAND_USAGE_IDENT "is set, delimiters are not escaped. Default: false\r\n";
        COMMAND_USAGE_IDENT "If the specified table already exists, rows are inserted into the existing table\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ImportCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    std::vector<Utf8String> args = TokenizeArgs(argsUnparsed);

    const size_t switchArgIndex = 0;
    if (args.size() < 2 || (!args[switchArgIndex].EqualsIAscii(ECSCHEMA_SWITCH) && !args[switchArgIndex].EqualsIAscii(CSV_SWITCH)))
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        return;

    if (session.GetFile().GetHandle().IsReadonly())
        {
        IModelConsole::WriteErrorLine("File must be editable. Please close the file and re-open it in read-write mode.");
        return;
        }


    Utf8StringCR commandSwitch = args[switchArgIndex];
    if (commandSwitch.EqualsIAscii(ECSCHEMA_SWITCH))
        {
        if (!session.IsECDbFileLoaded(true))
            return;

        RunImportSchema(session, args);
        return;
        }

    BeAssert(commandSwitch.EqualsIAscii(CSV_SWITCH));
    RunImportCsv(session, args);
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ImportCommand::RunImportSchema(Session& session, std::vector<Utf8String> const& args) const
    {
    Utf8StringCR firstArg = args[1];
    size_t pathIx = 1;
    SchemaManager::SchemaImportOptions options = SchemaManager::SchemaImportOptions::None;
    if (firstArg.EqualsIAscii("legacy"))
        {
        options = SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues;
        pathIx = 2;
        }

    BeFileName ecschemaPath(args[pathIx]);
    ecschemaPath.Trim(L"\"");
    if (!ecschemaPath.DoesPathExist())
        {
        IModelConsole::WriteErrorLine("Schema Import failed. Specified path '%s' does not exist.", ecschemaPath.GetNameUtf8().c_str());
        return;
        }

    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext(false, true);
    context->AddSchemaLocater(session.GetFile().GetECDbHandle()->GetSchemaLocater());

    BeFileName schemaAssetsFolder = Dgn::PlatformLib::GetHost().GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();

    BeFileName schemaSearchPath(schemaAssetsFolder);
    schemaSearchPath.AppendToPath(L"ECSchemas").AppendToPath(L"Domain");
    context->AddSchemaPath(schemaSearchPath);

    bvector<BeFileName> ecschemaFilePaths;

    const bool isFolder = (const_cast<BeFileNameR> (ecschemaPath)).IsDirectory();
    if (isFolder)
        {
        context->AddSchemaPath(ecschemaPath);
        BeDirectoryIterator::WalkDirsAndMatch(ecschemaFilePaths, ecschemaPath, L"*.ecschema.xml", false);
        if (ecschemaFilePaths.empty())
            {
            IModelConsole::WriteErrorLine("Import failed. Folder '%s' does not contain ECSchema XML files.", ecschemaPath.GetNameUtf8().c_str());
            return;
            }
        }
    else
        {
        context->AddSchemaPath(ecschemaPath.GetDirectoryName().GetName());
        ecschemaFilePaths.push_back(ecschemaPath);
        }

    for (BeFileName const& ecschemaFilePath : ecschemaFilePaths)
        {
        IModelConsole::WriteLine("Reading ECSchema ... %s", ecschemaFilePath.GetNameUtf8().c_str());
        if (SUCCESS != DeserializeECSchema(*context, ecschemaFilePath))
            {
            IModelConsole::WriteErrorLine("Import failed. Could not read ECSchema '%s' into memory.", ecschemaFilePath.GetNameUtf8().c_str());
            return;
            }
        }

    Utf8CP schemaStr = isFolder ? "ECSchemas in folder" : "ECSchema";

    if (BE_SQLITE_OK != session.GetFile().GetHandleR().SaveChanges())
        {
        IModelConsole::WriteLine("Saving outstanding changes in the file failed: %s", session.GetFile().GetHandle().GetLastError().c_str());
        return;
        }

    bool schemaImportSuccessful = false;
    if (session.GetFile().GetType() == SessionFile::Type::IModel)
        {
        if (options == SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues)
            schemaImportSuccessful = Dgn::SchemaStatus::Success == session.GetFile().GetAs<IModelFile>().GetDgnDbHandleR().ImportV8LegacySchemas(context->GetCache().GetSchemas());
        else
            schemaImportSuccessful = Dgn::SchemaStatus::Success == session.GetFile().GetAs<IModelFile>().GetDgnDbHandleR().ImportSchemas(context->GetCache().GetSchemas());
        }
    else
        schemaImportSuccessful = SUCCESS == session.GetFile().GetECDbHandle()->Schemas().ImportSchemas(context->GetCache().GetSchemas(), options);

    if (schemaImportSuccessful)
        {
        session.GetFile().GetHandleR().SaveChanges();
        IModelConsole::WriteLine("Successfully imported %s '%s'.", schemaStr, ecschemaPath.GetNameUtf8().c_str());
        return;
        }

    session.GetFile().GetHandleR().AbandonChanges();
    if (session.GetIssues().HasIssue())
        IModelConsole::WriteErrorLine("Failed to import %s '%s': %s", schemaStr, ecschemaPath.GetNameUtf8().c_str(), session.GetIssues().GetIssue());
    else
        IModelConsole::WriteErrorLine("Failed to import %s '%s'.", schemaStr, ecschemaPath.GetNameUtf8().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus ImportCommand::DeserializeECSchema(ECSchemaReadContextR readContext, BeFileNameCR ecschemaXmlFile)
    {
    ECN::ECSchemaPtr ecSchema = nullptr;
    const auto stat = ECN::ECSchema::ReadFromXmlFile(ecSchema, ecschemaXmlFile.GetName(), readContext);
    //duplicate schema error is ok, as the ReadFromXmlFile reads schema references implicitly.
    return stat == ECN::SchemaReadStatus::Success || stat == ECN::SchemaReadStatus::DuplicateSchema ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ImportCommand::RunImportCsv(Session& session, std::vector<Utf8String> const& args) const
    {
    if(args.size() < 3)
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    BeFileName csvFilePath(args[1]);
    csvFilePath.Trim(L"\"");
    if (!csvFilePath.DoesPathExist())
        {
        IModelConsole::WriteErrorLine("CSV Import failed. Specified path '%s' does not exist.", csvFilePath.GetNameUtf8().c_str());
        return;
        }

    Utf8StringCR tableName = args[2];
    const bool hasColumnHeader = args.size() >= 4 ? GetArgAsBool(args[3]) : false;
    WChar delimiter = args.size() >= 5 ? WString(args[4].c_str(), BentleyCharEncoding::Utf8)[0] : L',';
    WChar escapeChar = args.size() >= 6 && GetArgAsBool(args[5]) ? L'\0' : L'"';

    BeFileStatus stat;
    BeTextFilePtr file = BeTextFile::Open(stat, csvFilePath.GetName(), TextFileOpenType::Read, TextFileOptions::KeepNewLine);
    if (BeFileStatus::Success != stat)
        {
        IModelConsole::WriteErrorLine("Could not open CSV file %s.", csvFilePath.GetNameUtf8().c_str());
        return;
        }

    WString line;
    Statement stmt;
    int lineNo = 0, rowCount = 0;
    bool isFirstLine = true;
    int columnCount = -1;
    std::vector<Utf8String> tokens;
    while (TextFileReadStatus::Success == file->GetLine(line))
        {
        lineNo++;
        tokens.resize(0);
        if (SUCCESS != TokenizeString(tokens, line, delimiter, escapeChar))
            {
            IModelConsole::WriteErrorLine("Error when parsing line #d in CSV file %s.", lineNo, csvFilePath.GetNameUtf8().c_str());
            return;
            }

        if (isFirstLine)
            {
            columnCount = (int) tokens.size();
            if (SUCCESS != SetupCsvImport(session, stmt, tableName, (uint32_t) tokens.size(), hasColumnHeader ? &tokens : nullptr))
                return;

            isFirstLine = false;

            if (hasColumnHeader)
                continue;
            }

        rowCount++;
        if (SUCCESS != InsertCsvRow(session, stmt, columnCount, tokens, rowCount))
            return;
        }

    IModelConsole::WriteLine("Successfully imported %d rows into table %s from CSV file %s", rowCount, tableName.c_str(), csvFilePath.GetNameUtf8().c_str());
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ImportCommand::SetupCsvImport(Session& session, Statement& stmt, Utf8StringCR tableName, uint32_t columnCount, std::vector<Utf8String> const* header) const
    {
    if (header != nullptr && (uint32_t) header->size() != columnCount)
        return ERROR;

    Utf8String insertSql("INSERT INTO [");
    insertSql.append(tableName).append("](");
    Utf8String insertValuesClauseSql("VALUES(");

    Utf8String createTableSql;
    bvector<Utf8String> existingTableColNames;
    {
    if (session.GetFile().GetHandle().GetColumns(existingTableColNames, tableName.c_str()))
        {
        //table exists. Check whether col count matches with CSV file
        if ((uint32_t) existingTableColNames.size() != columnCount)
            {
            IModelConsole::WriteErrorLine("Table '%s' already exists but its column count differs from the column count in CSV file '%s'.",
                                       tableName.c_str(), session.GetFile().GetHandle().GetLastError().c_str());
            return ERROR;
            }
        }
    else
        {
        //table does not exists
        createTableSql.assign("CREATE TABLE [").append(tableName).append("] (");
        }
    }

    const bool tableExists = createTableSql.empty();

    bool isFirstCol = true;
    for (uint32_t i = 0; i < columnCount; i++)
        {
        if (!isFirstCol)
            {
            if (!tableExists)
                createTableSql.append(",");

            insertSql.append(",");
            insertValuesClauseSql.append(",");
            }

        if (tableExists)
            {
            insertSql.append(existingTableColNames[(size_t) i]);
            }
        else
            {
            if (header == nullptr)
                {
                Utf8String colName;
                colName.Sprintf("Column%" PRIu32, i + 1);
                createTableSql.append(colName);
                insertSql.append(colName);
                }
            else
                {
                Utf8StringCR colName = header->operator[](static_cast<size_t>(i));
                createTableSql.append("[").append(colName).append("]");
                insertSql.append("[").append(colName).append("]");
                }
            }

        insertValuesClauseSql.append("?");
        isFirstCol = false;
        }

    if (!tableExists)
        createTableSql.append(")");

    insertValuesClauseSql.append(")");
    insertSql.append(") ").append(insertValuesClauseSql);

    if (!tableExists)
        {
        if (BE_SQLITE_OK != session.GetFile().GetHandle().ExecuteSql(createTableSql.c_str()))
            {
            IModelConsole::WriteErrorLine("Could not create table '%s' for CSV file: %s",
                                       tableName.c_str(), session.GetFile().GetHandle().GetLastError().c_str());
            return ERROR;
            }
        }

    if (BE_SQLITE_OK != stmt.Prepare(session.GetFile().GetHandle(), insertSql.c_str()))
        {
        IModelConsole::WriteErrorLine("Could not prepare CSV insert statement '%s': %s",
                               insertSql.c_str(), session.GetFile().GetHandle().GetLastError().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ImportCommand::InsertCsvRow(Session& session, Statement& stmt, int columnCount, std::vector<Utf8String> const& tokens, int rowNumber) const
    {
    for (int i = 0; i < (int) tokens.size(); i++)
        {
        if (i >= columnCount)
            continue; //ignore excess tokens

        if (BE_SQLITE_OK != stmt.BindText(i + 1, tokens[i], Statement::MakeCopy::Yes))
            {
            IModelConsole::WriteErrorLine("Could not bind cell value [column %d, row %d] to insert statement: %s",
                                    i + 1, rowNumber, session.GetFile().GetHandle().GetLastError().c_str());
            return ERROR;
            }
        }

    if (BE_SQLITE_DONE != stmt.Step())
        {
        IModelConsole::WriteErrorLine("Could not insert row %d into table: %s",
                                rowNumber, session.GetFile().GetHandle().GetLastError().c_str());
        return ERROR;
        }

    stmt.ClearBindings();
    stmt.Reset();
    return SUCCESS;
    }


//******************************* ExportCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String ExportCommand::_GetUsage() const
    {
    return " .export schema [v2] <out folder>    Exports all ECSchemas of the file to disk. If 'v2' is specified, ECXML v2 is used.\r\n"
           COMMAND_USAGE_IDENT "Otherwise ECXML v3 is used.\r\n"
           "         tables <JSON file>     Exports the data in all tables of the file into a JSON file\r\n"
           "         changesummary <change summary id> <JSON file> Exports the content of the specified change summary into a JSON file\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ExportCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    std::vector<Utf8String> args = TokenizeArgs(argsUnparsed);
    const size_t argCount = args.size();
    if (argCount == 0)
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        return;

    Utf8StringCR switchName = args[0];
    if (switchName.EqualsIAscii(s_schemaSwitch))
        {
        if (!session.IsECDbFileLoaded(true))
            return;

        if (argCount < 2 || argCount > 3 || (argCount == 3 && !args[1].EqualsIAscii("v2")))
            {
            IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
            return;
            }

        Utf8StringCP outFolder = nullptr;
        bool useECXmlV2 = false;
        if (argCount == 2)
            outFolder = &args[1];
        else
            {
            useECXmlV2 = true;
            outFolder = &args[2];
            }

        RunExportSchema(session, *outFolder, useECXmlV2);
        return;
        }

    if (switchName.EqualsIAscii(s_tablesSwitch))
        {
        if (argCount != 2)
            {
            IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
            return;
            }

        RunExportTables(session, args[1]);
        return;
        }

    if (switchName.EqualsIAscii(s_changeSummarySwitch))
        {
        if (!session.IsECDbFileLoaded(true))
            return;

        if (argCount != 3)
            {
            IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
            return;
            }

        Utf8StringCR changeSummaryIdStr = args[1];
        ECInstanceId changeSummaryId;
        if (SUCCESS != changeSummaryId.FromString(changeSummaryId, changeSummaryIdStr.c_str()))
            {
            IModelConsole::WriteErrorLine("Invalid change summary id '%s'. The id must either be specified as decimal or hexadecimal number.", changeSummaryIdStr.c_str());
            return;
            }

        RunExportChangeSummary(session, changeSummaryId, args[2]);
        return;
        }

    IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ExportCommand::RunExportSchema(Session& session, Utf8StringCR outFolderStr, bool useECXmlV2) const
    {
    bvector<ECN::ECSchemaCP> schemas = session.GetFile().GetECDbHandle()->Schemas().GetSchemas(true);
    if (schemas.empty())
        {
        IModelConsole::WriteErrorLine("Failed to load schemas from file.");
        return;
        }

    BeFileName outFolder(outFolderStr);
    if (outFolder.IsDirectory())
        {
        IModelConsole::WriteErrorLine("Folder %s already exists. Please delete it or specify and another folder.", outFolder.GetNameUtf8().c_str());
        return;
        }
    else
        BeFileName::CreateNewDirectory(outFolder.GetName());

    ECN::ECVersion ecxmlVersion = useECXmlV2 ? ECN::ECVersion::V2_0 : ECN::ECVersion::V3_1;
    for (ECSchemaCP schema : schemas)
        {
        WString fileName;
        fileName.AssignUtf8(schema->GetFullSchemaName().c_str());
        fileName.append(L".ecschema.xml");

        BeFileName outPath(outFolder);
        outPath.AppendToPath(fileName.c_str());
        schema->WriteToXmlFile(outPath.GetName(), ecxmlVersion);
        IModelConsole::WriteLine("Saved ECSchema '%s' to disk", outPath.GetNameUtf8().c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ExportCommand::RunExportChangeSummary(Session& session, ECInstanceId changeSummaryId, Utf8StringCR jsonFilePath) const
    {
    BeAssert(session.GetFile().GetECDbHandle() != nullptr);
    ChangeSummaryExportContext ctx(*session.GetFile().GetECDbHandle(), changeSummaryId);
    if (SUCCESS != ctx.InitializeOutput(jsonFilePath))
        return;

    ECSqlStatement stmt;
    {
    ChangeSummaryExportContext::Timer perf(ctx, "InstanceChanges ECSQL (Prepare)");
    if (ECSqlStatus::Success != stmt.Prepare(ctx.m_ecdb,
                "SELECT ic.ECInstanceId, s.Name changedInstanceSchemaName, c.Name changedInstanceClassName, ic.ChangedInstance.Id changedInstanceId,"
                "ic.OpCode, ic.IsIndirect FROM ecchange.change.InstanceChange ic JOIN main.meta.ECClassDef c ON c.ECInstanceId = ic.ChangedInstance.ClassId "
                "JOIN main.meta.ECSchemaDef s ON c.Schema.Id = s.ECInstanceId WHERE ic.Summary.Id=?") ||
        ECSqlStatus::Success != stmt.BindId(1, changeSummaryId))
        {
        IModelConsole::WriteErrorLine("Failed to retrieve instance changes for change summary id %s.", ctx.m_summaryIdString.c_str());
        return;
        }
    }

    const int icIdIx = 0;
    const int changedInstanceSchemaNameIx = 1;
    const int changedInstanceClassNameIx = 2;
    const int changedInstanceIdIx = 3;
    const int opCodeIx = 4;
    const int isIndirectIx = 5;

    if (ECSqlStatus::Success != ctx.m_accessStringStmt.Prepare(ctx.m_ecdb, "SELECT AccessString FROM ecchange.change.PropertyValueChange WHERE InstanceChange.Id=?"))
        {
        IModelConsole::WriteErrorLine("Failed to prepare ECSQL to retrieve property value changes for instance changes.");
        return;
        }

    uint64_t instanceChangeCount = 0;
    ChangeSummaryExportContext::Timer perf(ctx, "Process InstanceChanges");
    while (BE_SQLITE_ROW == stmt.Step())
        {
        instanceChangeCount++;

        ECInstanceId icId = stmt.GetValueId<ECInstanceId>(icIdIx);
        ECInstanceId changedInstanceId = stmt.GetValueId<ECInstanceId>(changedInstanceIdIx);
        Utf8PrintfString changedInstanceClassName("[%s].[%s]", stmt.GetValueText(changedInstanceSchemaNameIx), stmt.GetValueText(changedInstanceClassNameIx));

        Json::Value& instanceChangeJson = ctx.m_outputJson.append(Json::ValueType::objectValue);
        instanceChangeJson["id"] = icId.ToHexStr();
        instanceChangeJson["summaryId"] = ctx.m_summaryIdString;

        instanceChangeJson["changedInstance"]["id"] = changedInstanceId.ToHexStr();
        instanceChangeJson["changedInstance"]["className"] = changedInstanceClassName;

        ChangeOpCode opCode = (ChangeOpCode) stmt.GetValueInt(opCodeIx);
        instanceChangeJson["opCode"] = (int) opCode;
        instanceChangeJson["isIndirect"] = stmt.GetValueBoolean(isIndirectIx);

        Json::Value& changedPropsJson = instanceChangeJson["changedProperties"];
        switch (opCode)
            {
                case ChangeOpCode::Insert:
                    if (SUCCESS != PropertyValueChangesToJson(changedPropsJson["after"], ctx, icId, changedInstanceId, changedInstanceClassName, ChangedValueState::AfterInsert))
                        return;

                    break;
                case ChangeOpCode::Update:
                    if (SUCCESS != PropertyValueChangesToJson(changedPropsJson["before"], ctx, icId, changedInstanceId, changedInstanceClassName, ChangedValueState::BeforeUpdate) ||
                        SUCCESS != PropertyValueChangesToJson(changedPropsJson["after"], ctx, icId, changedInstanceId, changedInstanceClassName, ChangedValueState::AfterUpdate))
                        return;

                    break;
                case ChangeOpCode::Delete:
                    if (SUCCESS != PropertyValueChangesToJson(changedPropsJson["before"], ctx, icId, changedInstanceId, changedInstanceClassName, ChangedValueState::BeforeDelete))
                        return;

                    break;
                default:
                    BeAssert(false && "Need to adjust unhandled ChangedOpCode");
                    IModelConsole::WriteErrorLine("Programmer error. Need to adjust unhandled ChangedOpCode");
                    return;
            }
        }

    perf.Dispose();

    if (SUCCESS != ctx.WriteOutput())
        return;

    IModelConsole::WriteLine("Exported ChangeSummary %s with %" PRIu64 " instance changes to '%s'", ctx.m_summaryIdString.c_str(), instanceChangeCount, ctx.m_jsonOutputPath.GetNameUtf8().c_str());
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ExportCommand::PropertyValueChangesToJson(Json::Value& propValJson, ChangeSummaryExportContext& ctx, BeSQLite::EC::ECInstanceId instanceChangeId, BeSQLite::EC::ECInstanceId changedInstanceId, Utf8StringCR changedInstanceClassName, BeSQLite::EC::ChangedValueState changedValueState) const
    {
    Utf8String changedInstanceLabel;
    changedInstanceLabel.Sprintf("%s:%s (%s)", changedInstanceClassName.c_str(), changedInstanceId.ToHexStr().c_str(), ToString(changedValueState));

    if (ECSqlStatus::Success != ctx.m_accessStringStmt.BindId(1, instanceChangeId))
        {
        IModelConsole::WriteErrorLine("Failed to retrieve property value changes for %s.", changedInstanceLabel.c_str());
        return ERROR;
        }

    ChangeSummaryExportContext::Timer perf(ctx, Utf8PrintfString("Process Changed %s", changedInstanceLabel.c_str()));

    Utf8String propValECSql("SELECT ");
    bool isFirstRow = true;
    while (BE_SQLITE_ROW == ctx.m_accessStringStmt.Step())
        {
        if (!isFirstRow)
            propValECSql.append(",");

        Utf8CP accessString = ctx.m_accessStringStmt.GetValueText(0);
        // access string tokens need to be escaped as they might collide with reserved words in ECSQL or SQLite
        bvector<Utf8String> accessStringTokens;
        BeStringUtilities::Split(accessString, ".", accessStringTokens);
        bool isFirstToken = true;
        for (Utf8StringCR token :  accessStringTokens)
            {
            if (!isFirstToken)
                propValECSql.append(".");

            propValECSql.append("[").append(token).append("]");
            isFirstToken = false;
            }

        isFirstRow = false;
        }

    ctx.m_accessStringStmt.Reset();
    ctx.m_accessStringStmt.ClearBindings();

    // Avoiding parameters in the Changes function speeds up performance because ECDb can do optimizations
    // if it knows the function args at prepare time
    propValECSql.append(" FROM main.").append(changedInstanceClassName).append(".Changes(").append(ctx.m_summaryIdString).append(",");
    propValECSql.append(Utf8PrintfString("%d) WHERE ECInstanceId=?", (int) changedValueState));
    CachedECSqlStatementPtr stmt = ctx.m_changedInstanceStmtCache.GetPreparedStatement(ctx.m_ecdb, propValECSql.c_str());
    if (stmt == nullptr || ECSqlStatus::Success != stmt->BindId(1, changedInstanceId))
        {
        IModelConsole::WriteErrorLine("Failed to retrieve property value changes for %s. Failed to prepare ECSQL '%s'", changedInstanceLabel.c_str(), propValECSql.c_str());
        return ERROR;
        }

    if (BE_SQLITE_ROW != stmt->Step())
        return SUCCESS;

    JsonECSqlSelectAdapter adapter(*stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsNumber));
    return adapter.GetRow(propValJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8CP ExportCommand::ToString(ChangedValueState state)
    {
    switch (state)
        {
            case ChangedValueState::AfterInsert:
                return "ChangedValueState::AfterInsert";
            case ChangedValueState::BeforeUpdate:
                return "ChangedValueState::BeforeUpdate";
            case ChangedValueState::AfterUpdate:
                return "ChangedValueState::AfterUpdate";
            case ChangedValueState::BeforeDelete:
                return "ChangedValueState::BeforeDelete";
            default:
                BeAssert(false && "Unhandled ChangedValueState enum value. Code needs to be adjusted");
                return "<programmer error>";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ExportCommand::RunExportTables(Session& session, Utf8StringCR jsonFile) const
    {
    BeFile file;
    if (file.Create(jsonFile, true) != BeFileStatus::Success)
        {
        IModelConsole::WriteErrorLine("Failed to create JSON file %s", jsonFile.c_str());
        return;
        }

    Statement stmt;
    stmt.Prepare(session.GetFile().GetHandle(), "SELECT name FROM sqlite_master WHERE type ='table'");
    Json::Value tableData(Json::ValueType::arrayValue);

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ExportTable(session, tableData, stmt.GetValueText(0));
        }

    Utf8String jsonString = tableData.ToString();
    if (file.Write(nullptr, jsonString.c_str(), static_cast<uint32_t>(jsonString.size())) != BeFileStatus::Success)
        {
        IModelConsole::WriteErrorLine("Failed to write to JSON file %s", jsonFile.c_str());
        return;
        }

    file.Flush();
    IModelConsole::WriteLine("Exported tables to '%s'", jsonFile.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ExportCommand::ExportTable(Session& session, Json::Value& out, Utf8CP tableName) const
    {
    Json::Value& tableObj = out.append(Json::ValueType::objectValue);
    tableObj["Name"] = tableName;
    tableObj["Rows"] = Json::Value(Json::ValueType::arrayValue);
    Json::Value& rows = tableObj["Rows"];
    rows.clear();
    Statement stmt;
    stmt.Prepare(session.GetFile().GetHandle(), SqlPrintfString("SELECT * FROM %s", tableName));
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        auto& row = rows.append(Json::ValueType::objectValue);
        row.clear();
        for (auto i = 0; i < stmt.GetColumnCount(); i++)
            {
            switch (stmt.GetColumnType(i))
                {
                    case DbValueType::BlobVal:
                    {
                    Utf8String base64Str;
                    Base64Utilities::Encode(base64Str, (Byte const*) stmt.GetValueBlob(i), stmt.GetColumnBytes(i));
                    row[stmt.GetColumnName(i)] = base64Str;
                    break;
                    }
                    case DbValueType::FloatVal:
                        row[stmt.GetColumnName(i)] = Json::Value(stmt.GetValueDouble(i)); break;
                    case DbValueType::IntegerVal:
                        row[stmt.GetColumnName(i)] = Json::Value(stmt.GetValueInt64(i)); break;
                    case DbValueType::NullVal:
                        row[stmt.GetColumnName(i)] = Json::Value(Json::nullValue); break;
                    case DbValueType::TextVal:
                        row[stmt.GetColumnName(i)] = Json::Value(stmt.GetValueText(i)); break;
                }
            }
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ExportCommand::ChangeSummaryExportContext::InitializeOutput(Utf8StringCR jsonOutputPath)
    {
    m_jsonOutputPath = BeFileName(jsonOutputPath);

    if (m_outputFile.Create(m_jsonOutputPath, true) != BeFileStatus::Success)
        {
        IModelConsole::WriteErrorLine("Failed to create JSON file %s", jsonOutputPath.c_str());
        return ERROR;
        }

    BeFileName diagnosticsFilePath = m_jsonOutputPath.GetDirectoryName();
    diagnosticsFilePath.AppendToPath(m_jsonOutputPath.GetFileNameWithoutExtension().c_str());
    diagnosticsFilePath.AppendExtension(L"diag.csv");
    if (m_diagnosticsFile.Create(diagnosticsFilePath, true) != BeFileStatus::Success)
        {
        IModelConsole::WriteErrorLine("Failed to create diagnostics file %s", diagnosticsFilePath.GetNameUtf8().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ExportCommand::ChangeSummaryExportContext::WriteOutput()
    {
    Utf8String jsonString = m_outputJson.ToString();
    if (m_outputFile.Write(nullptr, jsonString.c_str(), (uint32_t) jsonString.size()) != BeFileStatus::Success ||
        BeFileStatus::Success != m_outputFile.Flush())
        {
        IModelConsole::WriteErrorLine("Failed to write ChangeSummary content to JSON file %s", m_jsonOutputPath.GetNameUtf8().c_str());
        return ERROR;
        }

    if (m_diagnosticsFile.Flush() != BeFileStatus::Success)
        {
        IModelConsole::WriteErrorLine("Failed to write diagnostics file.");
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ExportCommand::ChangeSummaryExportContext::Timer::Dispose()
    {
    if (m_isDiposed)
        return;

    m_isDiposed = true;
    Utf8String line;
    if (Utf8String::IsNullOrEmpty(m_ecsql))
        line.Sprintf("%s|%" PRIu64 "\r\n", m_message.c_str(), BeTimeUtilities::GetCurrentTimeAsUnixMillis() - m_startTime);
    else
        line.Sprintf("%s|%" PRIu64 "|%s|%s\r\n", m_message.c_str(), BeTimeUtilities::GetCurrentTimeAsUnixMillis() - m_startTime, m_ecsql, m_nativeSql);

    if (m_ctx.m_diagnosticsFile.Write(nullptr, line.c_str(), (uint32_t) line.size()) != BeFileStatus::Success)
        {
        IModelConsole::WriteErrorLine("Failed to write diagnostics to diagnostics file.");
        return;
        }
    }


//******************************* CreateClassViewsCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String CreateClassViewsCommand::_GetUsage() const
    {
    return " .createclassviews <classes>    Creates or updates views in the file to visualize the EC content as ECClasses and\r\n"
        COMMAND_USAGE_IDENT "ECProperties rather than tables and columns.\r\n"
        COMMAND_USAGE_IDENT "If <classes> is specified, will create views for only those classes.  This is a space-delimited list of\r\n"
        COMMAND_USAGE_IDENT "ECClassId and/or SchemaName:ClassName.\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void CreateClassViewsCommand::_Run(Session& session, Utf8StringCR args) const
    {
    if (!session.IsECDbFileLoaded(true))
        return;

    if (session.GetFile().GetHandle().IsReadonly())
        {
        IModelConsole::WriteErrorLine("File must be editable. Please close the file and re-open it in read-write mode.");
        return;
        }

    bvector<ECClassId> classIds;
    if (!args.empty())
        {
        bvector<Utf8String> classTokens;
        BeStringUtilities::Split(args.c_str(), " ", classTokens);
        for (Utf8String classToken : classTokens)
            {
            ECClassId parsedId;
            if (SUCCESS == ECClassId::FromString(parsedId, classToken.c_str()))
                classIds.push_back(parsedId);
            else
                {
                bvector<Utf8String> components;
                BeStringUtilities::Split(classToken.c_str(), ":.", components);
                if (components.size() != 2)
                    {
                    IModelConsole::WriteErrorLine("Unable to convert %s to either an ECClassId or <SchemaName>.<ClassName>\n", classToken.c_str());
                    }
                else
                    {
                    ECClassId foundId = session.GetFile().GetECDbHandle()->Schemas().GetClassId(components[0], components[1]);
                    if (foundId.IsValid())
                        classIds.push_back(foundId);
                    else
                        IModelConsole::WriteErrorLine("Unable to convert %s to an ECClassId\n", classToken.c_str());
                    }
                }
            }
        }
    if (0 != classIds.size())
        {
        if (SUCCESS != session.GetFile().GetECDbHandle()->Schemas().CreateClassViewsInDb(classIds))
            IModelConsole::WriteErrorLine("Failed to create ECClass views in the file.");
        else
            IModelConsole::WriteLine("Created or updated ECClass views in the file.");
        }
    else if (args.empty()) // If user passed in classNames that failed to parse, don't default to creating classviews for everything
        {
        if (SUCCESS != session.GetFile().GetECDbHandle()->Schemas().CreateClassViewsInDb())
            IModelConsole::WriteErrorLine("Failed to create ECClass views in the file.");
        else
            IModelConsole::WriteLine("Created or updated ECClass views in the file.");
        }
    }


//******************************* MetadataCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String MetadataCommand::_GetUsage() const
    {
    return " .metadata <ecsql>              Executes ECSQL and displays result column metadata";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void MetadataCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    if (argsUnparsed.empty())
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsECDbFileLoaded(true))
        return;

    Utf8CP ecsql = argsUnparsed.c_str();

    ECSqlStatement stmt;
    ECSqlStatus status = stmt.Prepare(*session.GetFile().GetECDbHandle(), ecsql);
    if (!status.IsSuccess())
        {
        if (session.GetIssues().HasIssue())
            IModelConsole::WriteErrorLine("Failed to prepare ECSQL statement. %s", session.GetIssues().GetIssue());
        else
            IModelConsole::WriteErrorLine("Failed to prepare ECSQL statement.");

        return;
        }

    IModelConsole::WriteLine();
    IModelConsole::WriteLine("Column metadata");
    IModelConsole::WriteLine("===============");
    IModelConsole::WriteLine("Index   Name/PropertyPath                   DisplayLabel                        System   Type                      Root class                     Root class alias");
    IModelConsole::WriteLine("------------------------------------------------------------------------------------------------------------------------------------------------------------------");
    const int columnCount = stmt.GetColumnCount();
    for (int i = 0; i < columnCount; i++)
        {
        ECSqlColumnInfo const& columnInfo = stmt.GetColumnInfo(i);
        bool isSystemProp = columnInfo.IsSystemProperty();
        bool isGeneratedProp = columnInfo.IsGeneratedProperty();
        ECN::ECPropertyCP prop = columnInfo.GetProperty();
        ECSqlPropertyPathCR propPath = columnInfo.GetPropertyPath();
        Utf8String propPathStr = isGeneratedProp ? prop->GetDisplayLabel() : propPath.ToString();
        Utf8String typeName = GetPropertyTypeName(*prop);

        ECSqlColumnInfo::RootClass const& rootClass = columnInfo.GetRootClass();
        Utf8String rootClassName;
        if (isGeneratedProp)
            rootClassName = "generated";
        else
            {
            //system properties have a different schema, so they must be excluded and never get a full class name
            if (!isSystemProp && rootClass.GetClass().GetSchema().GetId() != prop->GetClass().GetSchema().GetId())
                {
                if (!rootClass.GetTableSpace().EqualsIAscii("main"))
                    rootClassName.assign(rootClass.GetTableSpace()).append(".");

                rootClassName.append(rootClass.GetClass().GetFullName());
                }
            else
                rootClassName = rootClass.GetClass().GetName();
            }

        IModelConsole::WriteLine("%3d     %-35s %-35s %-8s %-25s %-30s %s", i, propPathStr.c_str(), prop->GetDisplayLabel().c_str(), isSystemProp? "yes":"no", typeName.c_str(), rootClassName.c_str(), rootClass.GetAlias().c_str());
        }

    IModelConsole::WriteLine();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
Utf8String MetadataCommand::GetPropertyTypeName(ECN::ECPropertyCR prop)
    {
    Utf8CP extendedTypeName = nullptr;
    if (prop.GetIsPrimitive() || prop.GetIsPrimitiveArray())
        {
        ECEnumerationCP ecEnum = nullptr;
        Nullable<PrimitiveType> primType;
        PrimitiveECPropertyCP primProp = prop.GetAsPrimitiveProperty();
        bool isArray = false;
        if (primProp != nullptr)
            {
            ecEnum = primProp->GetEnumeration();
            if (ecEnum == nullptr)
                primType = primProp->GetType();

            if (primProp->HasExtendedType())
                extendedTypeName = primProp->GetExtendedTypeName().c_str();
            }
        else
            {
            PrimitiveArrayECPropertyCP primArrayProp = prop.GetAsPrimitiveArrayProperty();
            BeAssert(primArrayProp != nullptr);
            isArray = true;
            ecEnum = primArrayProp->GetEnumeration();
            if (ecEnum == nullptr)
                primType = primArrayProp->GetPrimitiveElementType();

            if (primArrayProp->HasExtendedType())
                extendedTypeName = primArrayProp->GetExtendedTypeName().c_str();
            }

        Utf8String typeName;
        if (ecEnum != nullptr)
            {
            if (ecEnum->GetSchema().GetId() != prop.GetClass().GetSchema().GetId())
                typeName = ecEnum->GetFullName();
            else
                typeName = ecEnum->GetName();
            }
        else
            {
            BeAssert(!primType.IsNull());
            switch (primType.Value())
                {
                    case PRIMITIVETYPE_Binary:
                        typeName = "Blob";
                        break;
                    case PRIMITIVETYPE_Boolean:
                        typeName = "Boolean";
                        break;
                    case PRIMITIVETYPE_DateTime:
                        typeName = "DateTime";
                        break;
                    case PRIMITIVETYPE_Double:
                        typeName = "Double";
                        break;
                    case PRIMITIVETYPE_IGeometry:
                        typeName = "Geometry";
                        break;
                    case PRIMITIVETYPE_Integer:
                        typeName = "Integer";
                        break;
                    case PRIMITIVETYPE_Long:
                        typeName = "Long";
                        break;
                    case PRIMITIVETYPE_Point2d:
                        typeName = "Point2d";
                        break;
                    case PRIMITIVETYPE_Point3d:
                        typeName = "Point3d";
                        break;
                    case PRIMITIVETYPE_String:
                        typeName = "String";
                        break;
                    default:
                        typeName = "<unknown>";
                        BeAssert(false && "Adjust code to new value in ECN::PrimitiveType enum");
                        break;
                }
            }

        if (isArray)
            typeName.append("[]");

        if (!Utf8String::IsNullOrEmpty(extendedTypeName))
            {
            _Analysis_assume_(extendedTypeName != nullptr);
            typeName.append(" | Extended Type: ").append(extendedTypeName);
            }

        return typeName;
        }

    if (prop.GetIsStruct() || prop.GetIsStructArray())
        {
        ECStructClassCR structType = prop.GetIsStruct() ? prop.GetAsStructProperty()->GetType() : prop.GetAsStructArrayProperty()->GetStructElementType();
        Utf8String typeName;
        if (structType.GetSchema().GetId() != prop.GetClass().GetSchema().GetId())
            typeName = structType.GetFullName();
        else
            typeName = structType.GetName();

        if (prop.GetIsStructArray())
            typeName.append("[]");

        return typeName;
        }

    if (prop.GetIsNavigation())
        {
        NavigationECPropertyCP navProp = prop.GetAsNavigationProperty();
        Utf8String typeName("Navigation(");

        if (navProp->GetRelationshipClass()->GetSchema().GetId() != prop.GetClass().GetSchema().GetId())
            typeName.append(navProp->GetRelationshipClass()->GetFullName());
        else
            typeName.append(navProp->GetRelationshipClass()->GetName());

        if (navProp->GetDirection() == ECRelatedInstanceDirection::Forward)
            typeName.append(",forward)");
        else
            typeName.append(",backward)");

        return typeName;
        }

    BeAssert(false && "Adjust code to new ECProperty type");
    return Utf8String("<unknown>");
    }

//******************************* ParseCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String ParseCommand::_GetUsage() const
    {
    return " .parse [sql|exp|token] <ecsql> Parses ECSQL. Options: sql (default): the resulting SQL is displayed.\r\n"
        COMMAND_USAGE_IDENT                "                       exp: the parsed expression tree is displayed.\r\n"
        COMMAND_USAGE_IDENT                "                       token: the parsed raw token tree is displayed.\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ParseCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    if (argsUnparsed.empty())
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsECDbFileLoaded(true))
        return;

    Utf8String commandSwitch;
    const size_t nextStartIndex = IModelConsole::FindNextToken(commandSwitch, WString(argsUnparsed.c_str(), BentleyCharEncoding::Utf8), 0, L' ');

    enum class ParseMode
        {
        Default,
        Exp,
        Token,
        Sql
        };

    ParseMode parseMode = ParseMode::Default;
    if (commandSwitch.EqualsIAscii("exp"))
        parseMode = ParseMode::Exp;
    else if (commandSwitch.EqualsIAscii("token"))
        parseMode = ParseMode::Token;
    else if (commandSwitch.EqualsIAscii("sql"))
        parseMode = ParseMode::Sql;

    Utf8String ecsql;
    if (parseMode == ParseMode::Default)
        ecsql.assign(argsUnparsed);
    else
        {
        if (nextStartIndex < argsUnparsed.size())
            ecsql.assign(argsUnparsed.substr(nextStartIndex));
        }

    if (ecsql.empty())
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    switch (parseMode)
        {
            case ParseMode::Exp:
            {
            Utf8String ecsqlFromExpTree;
            Json::Value expTree;
            if (SUCCESS != ECSqlParseTreeFormatter::ParseAndFormatECSqlExpTree(expTree, ecsqlFromExpTree, *session.GetFile().GetECDbHandle(), ecsql.c_str()))
                {
                if (session.GetIssues().HasIssue())
                    IModelConsole::WriteErrorLine("Failed to parse ECSQL: %s", session.GetIssues().GetIssue());
                else
                    IModelConsole::WriteErrorLine("Failed to parse ECSQL.");

                return;
                }

            IModelConsole::WriteLine("ECSQL from expression tree: %s", ecsqlFromExpTree.c_str());
            IModelConsole::WriteLine();
            IModelConsole::WriteLine("ECSQL expression tree:");

            Utf8String expTreeStr;
            ExpTreeToString(expTreeStr, expTree, 0);
            IModelConsole::WriteLine("%s", expTreeStr.c_str());
            return;
            }

            case ParseMode::Token:
            {
            Utf8String parseTree;
            if (SUCCESS != ECSqlParseTreeFormatter::ParseAndFormatECSqlParseNodeTree(parseTree, *session.GetFile().GetECDbHandle(), ecsql.c_str()))
                {
                if (session.GetIssues().HasIssue())
                    IModelConsole::WriteErrorLine("Failed to parse ECSQL: %s", session.GetIssues().GetIssue());
                else
                    IModelConsole::WriteErrorLine("Failed to parse ECSQL.");

                return;
                }

            IModelConsole::WriteLine("Raw ECSQL parse tree:");
            IModelConsole::WriteLine("%s", parseTree.c_str());
            return;
            }

            default:
            {
            ECSqlStatement stmt;
            ECSqlStatus stat = stmt.Prepare(*session.GetFile().GetECDbHandle(), ecsql.c_str());
            if (!stat.IsSuccess())
                if (session.GetIssues().HasIssue())
                    IModelConsole::WriteErrorLine("Failed to parse ECSQL: %s", session.GetIssues().GetIssue());
                else
                    IModelConsole::WriteErrorLine("Failed to parse ECSQL.");

            IModelConsole::WriteLine("SQLite SQL: %s", stmt.GetNativeSql());
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
void ParseCommand::ExpTreeToString(Utf8StringR expTreeStr, JsonValueCR expTree, int indentLevel)
    {
    for (int i = 0; i < indentLevel; i++)
        expTreeStr.append("   ");

    expTreeStr.append(expTree["Exp"].asCString()).append("\r\n");

    if (!expTree.isMember("Children"))
        return;

    indentLevel++;
    for (JsonValueCR child : expTree["Children"])
        {
        ExpTreeToString(expTreeStr, child, indentLevel);
        }
    }

//******************************* ExitCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String ExitCommand::_GetUsage() const
    {
    return " .exit, .quit, .q               Exits the iModelConsole";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ExitCommand::_Run(Session& session, Utf8StringCR args) const { exit(0); }

//******************************* SqliteCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String SqliteCommand::_GetUsage() const
    {
    return " .sqlite <SQLite SQL>           Executes a SQLite SQL statement";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void SqliteCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    Utf8StringCR sql = argsUnparsed;
    if (sql.empty())
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        return;

    Statement stmt;
    DbResult status = stmt.Prepare(session.GetFile().GetHandle(), sql.c_str());
    if (status != BE_SQLITE_OK)
        {
        IModelConsole::WriteErrorLine("Failed to prepare SQLite SQL statement %s: %s", sql.c_str(), session.GetFile().GetHandle().GetLastError().c_str());
        return;
        }

    ExecuteSelect(stmt, session);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void SqliteCommand::ExecuteSelect(Statement& statement, Session& session) const
    {
    const int columnCount = statement.GetColumnCount();
    if (columnCount == 0)
        {
        if (statement.Step() != BE_SQLITE_DONE)
            {
            IModelConsole::WriteErrorLine("Failed to execute SQLite SQL statement %s: %s", statement.GetSql(), session.GetFile().GetHandle().GetLastError().c_str());
            return;
            }
        }

    for (int i = 0; i < columnCount; i++)
        {
        IModelConsole::Write("%s\t", statement.GetColumnName(i));
        }

    IModelConsole::WriteLine();
    IModelConsole::WriteLine("-------------------------------------------------------------");


    while (statement.Step() == BE_SQLITE_ROW)
        {
        Utf8String out;
        for (int i = 0; i < columnCount; i++)
            {
            if (statement.IsColumnNull(i))
                out.append("NULL");
            else
                out.append(statement.GetValueText(i));

            out.append("\t");
            }

        IModelConsole::WriteLine(out.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void SqliteCommand::ExecuteNonSelect(Session& session, Statement& statement) const
    {
    if (statement.Step() != BE_SQLITE_DONE)
        {
        IModelConsole::WriteErrorLine("Failed to execute SQLite SQL statement %s: %s", statement.GetSql(), session.GetFile().GetHandle().GetLastError().c_str());
        return;
        }
    }

//******************************* JsonCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String JsonCommand::_GetUsage() const
    {
    return " .json <ECSQL SELECT>           Returns the results of the ECSQL SELECT as JSON";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void JsonCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    Utf8StringCR ecsql = argsUnparsed;
    if (ecsql.empty() )
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        return;

    ECSqlStatement stmt;
    ECSqlStatus status = stmt.Prepare(*session.GetFile().GetECDbHandle(), ecsql.c_str());
    if (status != ECSqlStatus::Success)
        {
        IModelConsole::WriteErrorLine("Failed to prepare ECSQL statement %s: %s", ecsql.c_str(), session.GetIssues().GetIssue());
        return;
        }

    IModelConsole::WriteLine("Row Count | Row JSON");
    IModelConsole::WriteLine("--------------------");
    JsonECSqlSelectAdapter adapter(stmt);
    int rowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        rowCount++;
        Json::Value json;
        if (SUCCESS != adapter.GetRow(json))
            {
            IModelConsole::WriteErrorLine("Failed to retrieve the result of the ECSQL as JSON.");
            return;
            }

        IModelConsole::WriteLine("%d | %s", rowCount, json.ToString().c_str());
        }

    }

//******************************* DbSchemaCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String DbSchemaCommand::_GetUsage() const
    {
    return " .dbschema search <search term> [<folder> <file extension>]\r\n"
        COMMAND_USAGE_IDENT "Searches the DDL of all DB schema elements in the current file or in all SQLite files\r\n"
        COMMAND_USAGE_IDENT "in the specified folder for the specified search term.\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void DbSchemaCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    std::vector<Utf8String> args = TokenizeArgs(argsUnparsed);
    if (args.size() < 2)
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    Utf8StringCR switchArg = args[0];

    if (switchArg.EqualsI("search"))
        {
        //delete the switch arg
        args.erase(args.begin());
        Search(session, args);
        return;
        }

    IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void DbSchemaCommand::Search(Session& session, std::vector<Utf8String> const& searchArgs) const
    {
    const size_t argSize = searchArgs.size();
    Utf8CP searchTerm = nullptr;
    const bool isFileOpen = session.IsFileLoaded(false);

    if (isFileOpen && argSize != 1)
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }
    else if (!isFileOpen && argSize != 3)
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    searchTerm = searchArgs[0].c_str();

    if (isFileOpen)
        {
        Search(session.GetFile().GetHandle(), searchTerm);
        return;
        }

    bvector<BeFileName> filePaths;
    Utf8String folder(searchArgs[1]);
    folder.Trim("\"");

    Utf8String fileExtension(searchArgs[2]);
    Utf8String fileFilter;
    if (fileExtension.StartsWith("*."))
        fileFilter = fileExtension;
    else if (fileExtension.StartsWith("."))
        fileFilter.Sprintf("*%s", fileExtension.c_str());
    else
        fileFilter.Sprintf("*.%s", fileExtension.c_str());

    BeDirectoryIterator::WalkDirsAndMatch(filePaths, BeFileName(folder), WString(fileFilter.c_str(), BentleyCharEncoding::Utf8).c_str(), true);

    if (filePaths.empty())
        {
        IModelConsole::WriteErrorLine("Command failed. Folder '%s' does not contain files with extension *.%s.",
                                folder.c_str(), fileFilter.c_str());
        return;
        }

    for (BeFileNameCR path : filePaths)
        {
        Db db;
        if (BE_SQLITE_OK != db.OpenBeSQLiteDb(path, Db::OpenParams(Db::OpenMode::Readonly)))
            {
            IModelConsole::WriteErrorLine("Skipping file '%s', because it could not be opened.",
                                    path.GetNameUtf8().c_str());
            continue;
            }

        Search(db, searchTerm);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void DbSchemaCommand::Search(Db const& db, Utf8CP searchTerm) const
    {
    Utf8CP sql = "SELECT name, type FROM sqlite_master WHERE sql LIKE ?";
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(db, sql))
        {
        IModelConsole::WriteErrorLine("Failed to prepare SQLite SQL statement %s: %s", sql, db.GetLastError().c_str());
        return;
        }

    Utf8String searchTermWithWildcards;
    searchTermWithWildcards.Sprintf("%%%s%%", searchTerm);

    if (BE_SQLITE_OK != stmt.BindText(1, searchTermWithWildcards, Statement::MakeCopy::No))
        {
        IModelConsole::WriteErrorLine("Failed to bind search term '%s' to SQLite SQL statement %s: %s", searchTermWithWildcards.c_str(), sql, db.GetLastError().c_str());
        return;
        }

    if (BE_SQLITE_ROW != stmt.Step())
        {
        IModelConsole::WriteLine("The search term '%s' was not found in the DB schema elements of the file '%s'", searchTerm, db.GetDbFileName());
        return;
        }

    IModelConsole::WriteLine("In the file '%s' the following DB schema elements contain the search term %s:", db.GetDbFileName(), searchTerm);
    do
        {
        IModelConsole::WriteLine(" %s [%s]", stmt.GetValueText(0), stmt.GetValueText(1));
        } while (BE_SQLITE_ROW == stmt.Step());
    }




//******************************* SchemaStatsCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String SchemaStatsCommand::_GetUsage() const
    {
    return " .schemastats classhierarchy rootclass [csvoutputfilepath]\r\n"
        COMMAND_USAGE_IDENT "Computes statistics for a class hierarchy or a branch of it\r\n";
        COMMAND_USAGE_IDENT "rootclass: Fully specified class name of the root class of the hierarchy\r\n";
        COMMAND_USAGE_IDENT "csvoutputfilepath: Path to CSV file containing the class column count distribution\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void SchemaStatsCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    if (!session.IsECDbFileLoaded(true))
        return;

    std::vector<Utf8String> args = TokenizeArgs(argsUnparsed);
    if (args.empty())
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    Utf8StringCR switchArg = args[0];

    if (switchArg.EqualsIAscii("classhierarchy"))
        {
        ComputeClassHierarchyStats(session, args);
        return;
        }

    IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
    return;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void SchemaStatsCommand::ComputeClassHierarchyStats(Session& session, std::vector<Utf8String> const& args) const
    {
    if (args.size() != 2 && args.size() != 3)
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    Utf8StringCR rootClassName = args[1];
    bvector<Utf8String> parsedRootClassName;
    BeStringUtilities::Split(rootClassName.c_str(), ".:", parsedRootClassName);
    if (parsedRootClassName.size() != 2)
        {
        IModelConsole::WriteErrorLine("Invalid root class name %s. Format must be: <schema name>.<class name> or <schema name>:<class name>", rootClassName.c_str());
        return;
        }

    ECDbCR ecdb = *session.GetFile().GetECDbHandle();
    ECClassCP rootClass = ecdb.Schemas().GetClass(parsedRootClassName[0], parsedRootClassName[1], SchemaLookupMode::AutoDetect);
    if (rootClass == nullptr || !rootClass->IsEntityClass())
        {
        IModelConsole::WriteErrorLine("Root class %s:%s is not an entity class or does not exist in the current file.", parsedRootClassName[0].c_str(), parsedRootClassName[1].c_str());
        return;
        }

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, R"sql(  SELECT t.Name, count(*) FROM ec_Class c
                        JOIN ec_PropertyMap pm ON c.Id=pm.ClassId
                        JOIN ec_Column col ON col.Id=pm.ColumnId
                        JOIN ec_Table t ON t.Id=col.TableId
                        WHERE col.IsVirtual=0 AND c.Id=?
                        group by t.Id order by t.Id)sql"))
        {
        IModelConsole::WriteErrorLine("Preparing stats SQL failed: %s", ecdb.GetLastError().c_str());
        return;
        }

    /*
    SELECT c.Name, rc.RelationshipClassId, rc.RelationshipEnd, c.Modifier from ec_RelationshipConstraint rc
 INNER JOIN ec_ClassMap cm ON cm.ClassId=rc.RelationshipClassId
 INNER JOIN ec_Class c ON c.Id=cm.ClassId
 INNER JOIN ec_RelationshipConstraintClass rcc ON rc.Id=rcc.ConstraintId
 LEFT JOIN ec_cache_ClassHierarchy CH ON CH.BaseClassId = RCC.ClassId
 Where ((cm.MapStrategy=10 AND rc.RelationshipEnd=1) OR (cm.MapStrategy=11 AND rc.RelationshipEnd=0)) AND
 ((rc.IsPolymorphic=0 AND rcc.ClassId=:rootclass) OR (rc.IsPolymorphic<>0 and ch.ClassId=:rootclass))*/

    ClassColumnStatsCollection classStats;
    std::function<BentleyStatus(ClassColumnStatsCollection& classStats, ECDbCR ecdb, ECClassCR ecClass, Statement& stmt)> gatherClassStats;
    gatherClassStats = [&gatherClassStats] (ClassColumnStatsCollection& classStats, ECDbCR ecdb, ECClassCR ecClass, Statement& stmt)
        {
        if (ecClass.GetClassModifier() != ECClassModifier::Abstract)
            {
            if (BE_SQLITE_OK != stmt.BindId(1, ecClass.GetId()))
                return ERROR;

            ClassColumnStats stats(ecClass);
            while (BE_SQLITE_ROW == stmt.Step())
                {
                stats.Add(stmt.GetValueText(0), (uint32_t) stmt.GetValueInt(1));
                }

            stmt.Reset();
            stmt.ClearBindings();

            if (!stats.GetColCountPerTable().empty())
                classStats.Add(stats);
            }

        for (ECClassCP subclass : ecdb.Schemas().GetDerivedClasses(ecClass))
            {
            if (SUCCESS != gatherClassStats(classStats, ecdb, *subclass, stmt))
                return ERROR;
            }

        return SUCCESS;
        };

    if (SUCCESS != gatherClassStats(classStats, ecdb, *rootClass, stmt))
        {
        IModelConsole::WriteErrorLine("Gathering Class Hierarchy stats for %s failed.", rootClass->GetFullName());
        return;
        }

    if (classStats.IsEmpty())
        {
        IModelConsole::WriteLine("No class hierarchy stats available for root class %s. The class hierarchy only consists of abstract classes.", rootClass->GetFullName());
        return;
        }

    //compute stats metrics
    classStats.Sort();


    IModelConsole::WriteLine("Mapped column count stats for class hierarchy of class %s", rootClass->GetFullName());
    IModelConsole::WriteLine("Minimum: %" PRIu32, classStats.GetList().front().GetTotalColumnCount());
    IModelConsole::WriteLine("Maximum: %" PRIu32, classStats.GetList().back().GetTotalColumnCount());
    IModelConsole::WriteLine("Median: %.1f", ComputeQuantile(classStats, .5));
    IModelConsole::WriteLine("80%% quantile: %.1f:", ComputeQuantile(classStats, .8));
    //Mean
    const double mean = std::accumulate(classStats.GetList().begin(), classStats.GetList().end(), 0.0, [] (double sum, ClassColumnStats const& stat) { return sum + stat.GetTotalColumnCount();}) / (1.0 * classStats.GetSize());
    IModelConsole::WriteLine("Mean: %.1f:", mean);

    //stddev
    const double variance = std::accumulate(classStats.GetList().begin(), classStats.GetList().end(), 0.0,
                                          [mean] (double sum, ClassColumnStats const& stat) { return sum + std::pow((mean - stat.GetTotalColumnCount()), 2); }) / (1.0 * classStats.GetSize());
    IModelConsole::WriteLine("Standard Deviation: %.1f:", std::sqrt(variance));

    if (args.size() == 2)
        return;

    BeFileName csvOutFilePath(args[2]);
    BeFileStatus stat = BeFileStatus::Success;
    BeTextFilePtr csvFile = BeTextFile::Open(stat, csvOutFilePath, TextFileOpenType::Write, TextFileOptions::KeepNewLine, TextFileEncoding::Utf8);
    if (BeFileStatus::Success != stat)
        {
        IModelConsole::WriteErrorLine("Failed to create output CSV file %s", csvOutFilePath.GetNameUtf8().c_str());
        return;
        }

    BeAssert(csvFile != nullptr);
    //write header line
    csvFile->PutLine(L"ECClass,ClassColCount,FirstTable,FirstTableColCount,SecondTable,SecondTableColCount", true);
    for (ClassColumnStats const& stats : classStats.GetList())
        {
        Utf8String line;
            {
            Utf8String snippet;
            snippet.Sprintf("%s,%" PRIu32, stats.GetClass().GetFullName(), stats.GetTotalColumnCount());
            line.append(snippet);
            }
        for (std::pair<Utf8String,uint32_t> const& tableColCount : stats.GetColCountPerTable())
            {
            Utf8String snippet;
            snippet.Sprintf("%s,%" PRIu32, tableColCount.first.c_str(), tableColCount.second);
            line.append(",").append(snippet);
            }

        csvFile->PutLine(WString(line.c_str(), BentleyCharEncoding::Utf8).c_str(), true);
        }

    IModelConsole::WriteLine("Saved class column count distribution to CSV file %s.", csvOutFilePath.GetNameUtf8().c_str());
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
double SchemaStatsCommand::ComputeQuantile(ClassColumnStatsCollection const& stats, double p)
    {
    if (p < 0 || p > 1)
        {
        BeAssert(false);
        return -1;
        }

    std::vector<ClassColumnStats> const& orderedStats = stats.GetList();

    const double np = p * stats.GetSize();
    const double np_floor = std::floor(np);

    if ((np - np_floor) < std::numeric_limits<double>::epsilon())
        {
        //np is not integral
        const uint32_t index = (uint32_t) std::floor(np + 1);
        BeAssert(index - 1 < stats.GetSize());
        return (double) orderedStats[(size_t) (index - 1)].GetTotalColumnCount();
        }

    //np is integral
    const uint32_t index1 = (uint32_t) np_floor;
    BeAssert(index1 - 1 < stats.GetSize());
    const uint32_t index2 = (uint32_t) (np_floor + 1);
    BeAssert(index2 - 1 < stats.GetSize());
    return .5 * (orderedStats[(size_t) (index1 - 1)].GetTotalColumnCount() + orderedStats[(size_t) (index2 - 1)].GetTotalColumnCount());
    }

//******************************* DebugCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void DebugCommand::_Run(Session& session, Utf8StringCR args) const
    {
    if (!session.IsFileLoaded(true))
        return;

    bvector<BeFileName> diegoSchemas;
    diegoSchemas.push_back(BeFileName(L"D:\\temp\\diego\\LinearReferencing.01.00.00.ecschema.xml"));
    diegoSchemas.push_back(BeFileName(L"D:\\temp\\diego\\RoadRailAlignment.01.00.00.ecschema.xml"));
    diegoSchemas.push_back(BeFileName(L"D:\\temp\\diego\\Costing.01.00.00.ecschema.xml"));
    diegoSchemas.push_back(BeFileName(L"D:\\temp\\diego\\BridgePhysical.01.00.00.ecschema.xml"));
    diegoSchemas.push_back(BeFileName(L"D:\\temp\\diego\\RoadRailPhysical.01.00.00.ecschema.xml"));

    ImportCommand importCmd;
    for (BeFileNameCR schemaFile : diegoSchemas)
        {
        Utf8String cmdArgs;
        cmdArgs.Sprintf("ecschema %s", schemaFile.GetNameUtf8().c_str());
        importCmd.Run(session, cmdArgs);
        }
    }



//******************************* Explain ******************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String ExplainCommand::_GetUsage() const
    {
    return " .explain                       Explain query plan of ECSQL";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ExplainCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    if (!session.IsECDbFileLoaded(true))
        return;

    if (!session.IsFileLoaded(true))
        return;

    std::vector<Utf8String> args = TokenizeArgs(argsUnparsed);

    if (args.empty()) {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
    }

    ECSqlStatement stmt;
    if (stmt.Prepare(*session.GetFile().GetECDbHandle(), argsUnparsed.c_str()) != ECSqlStatus::Success) {
        IModelConsole::WriteErrorLine("Failed to prepare ecsql");
        return;
    }

    Utf8String sql = Utf8String("EXPLAIN QUERY PLAN ") + stmt.GetNativeSql();
    SqliteCommand cmd;
    cmd.Run(session, sql.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String DropCommand::_GetUsage() const
    {
    return " .drop schema <schema name>     Drops the given schema if it is unused and no other schema references and uses any classes in the schema.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void DropCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    std::vector<Utf8String> args = TokenizeArgs(argsUnparsed);

    if (args.size() != 2)
        {
        IModelConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        return;

    if (session.GetFile().GetHandle().IsReadonly())
        {
        IModelConsole::WriteErrorLine("File must be editable. Please close the file and re-open it in read-write mode.");
        return;
        }

    if (!session.GetFile().GetECDbHandle()->Schemas().ContainsSchema(args[1]))
        {
        IModelConsole::WriteErrorLine("Could not find schema %s in the db", args[1].c_str());
        return;
        }

    if (session.GetFile().GetAs<IModelFile>().GetDgnDbHandleR().DropSchema(args[1]).IsSuccess())
        IModelConsole::WriteLine("Successfully dropped schema %s", args[1].c_str());
    else
        IModelConsole::WriteErrorLine("Failed to drop schema %s", args[1].c_str());
    }