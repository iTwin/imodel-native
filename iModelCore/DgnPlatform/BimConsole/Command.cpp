/*--------------------------------------------------------------------------------------+
|
|     $Source: BimConsole/Command.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECDb/ECDbApi.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeTextFile.h>
#include "Command.h"
#include "BimConsole.h"
#include <numeric>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

//******************************* Command ******************

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void Command::Run(Session& session, Utf8StringCR args) const
    {
    session.GetIssues().Reset();
    return _Run(session, args);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus Command::TokenizeString(std::vector<Utf8String>& tokens, WStringCR inputString, WChar delimiter, WChar delimiterEscapeChar)
    {
    const size_t inputStrLength = inputString.size();
    Utf8String token;
    size_t nextStartIndex = 0;
    while (nextStartIndex < inputStrLength)
        {
        nextStartIndex = BimConsole::FindNextToken(token, inputString, nextStartIndex, delimiter, delimiterEscapeChar);
        tokens.push_back(token);
        token.resize(0);
        }

    return SUCCESS;
    }

//******************************* HelpCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void HelpCommand::_Run(Session& session, Utf8StringCR args) const
    {
    BeAssert(m_commandMap.size() == 23 && "Command was added or removed, please update the HelpCommand accordingly.");
    BimConsole::WriteLine(m_commandMap.at(".help")->GetUsage().c_str());
    BimConsole::WriteLine();
    BimConsole::WriteLine(m_commandMap.at(".open")->GetUsage().c_str());
    BimConsole::WriteLine(m_commandMap.at(".close")->GetUsage().c_str());
    BimConsole::WriteLine(m_commandMap.at(".create")->GetUsage().c_str());
    BimConsole::WriteLine(m_commandMap.at(".fileinfo")->GetUsage().c_str());
    BimConsole::WriteLine();
    BimConsole::WriteLine(m_commandMap.at(".ecsql")->GetUsage().c_str());
    BimConsole::WriteLine(m_commandMap.at(".metadata")->GetUsage().c_str());
    BimConsole::WriteLine();
    BimConsole::WriteLine(m_commandMap.at(".createclassviews")->GetUsage().c_str());
    BimConsole::WriteLine();
    BimConsole::WriteLine(m_commandMap.at(".commit")->GetUsage().c_str());
    BimConsole::WriteLine(m_commandMap.at(".rollback")->GetUsage().c_str());
    BimConsole::WriteLine();
    BimConsole::WriteLine(m_commandMap.at(".import")->GetUsage().c_str());
    BimConsole::WriteLine(m_commandMap.at(".export")->GetUsage().c_str());
    BimConsole::WriteLine();
    BimConsole::WriteLine(m_commandMap.at(".parse")->GetUsage().c_str());
    BimConsole::WriteLine(m_commandMap.at(".dbschema")->GetUsage().c_str());
    BimConsole::WriteLine();
    BimConsole::WriteLine(m_commandMap.at(".sqlite")->GetUsage().c_str());
    BimConsole::WriteLine();
    BimConsole::WriteLine(m_commandMap.at(".schemastats")->GetUsage().c_str());
    BimConsole::WriteLine();
    BimConsole::WriteLine(m_commandMap.at(".exit")->GetUsage().c_str());
    }

//******************************* OpenCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
//static
Utf8CP const OpenCommand::READONLY_SWITCH = "readonly";
Utf8CP const OpenCommand::READWRITE_SWITCH = "readwrite";

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String OpenCommand::_GetUsage() const
    {
    return " .open [readonly|readwrite] <BIM/ECDb/BeSQLite file>\r\n"
        COMMAND_USAGE_IDENT "Opens a BIM, ECDb, or BeSQLite file. Default open mode: read-only.\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void OpenCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    std::vector<Utf8String> args = TokenizeArgs(argsUnparsed);

    const size_t argCount = args.size();
    if (argCount == 0 || argCount > 3)
        {
        BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (session.IsFileLoaded())
        {
        BimConsole::WriteErrorLine("%s file '%s' already open. Close it first before opening another file.", session.GetFile().TypeToString(), session.GetFile().GetPath());
        return;
        }

    //default mode: read-only
    Db::OpenMode openMode = Db::OpenMode::Readonly;
    bool openAsECDb = false;

    const size_t switchCount = argCount - 1;
    if (switchCount > 0)
        {
        for (size_t i = 0; i < switchCount; i++)
            {
            Utf8String const& arg = args[i];

            if (arg.EqualsI(READWRITE_SWITCH))
                openMode = Db::OpenMode::ReadWrite;
            else if (arg.EqualsI("asecdb"))
                openAsECDb = true;
            }
        }

    BeFileName filePath;
    filePath.SetNameUtf8(args[argCount - 1].c_str());
    filePath.Trim(L"\"");
    if (!filePath.DoesPathExist())
        {
        BimConsole::WriteErrorLine("The path '%s' does not exist.", filePath.GetNameUtf8().c_str());
        return;
        }

    Utf8CP openModeStr = openMode == Db::OpenMode::Readonly ? "read-only" : "read-write";

    //open as plain BeSQlite file first to retrieve profile infos. If file is ECDb or BIM file, we close it
    //again and use respective API to open it higher-level
    std::unique_ptr<BeSQLiteFile> sqliteFile = std::make_unique<BeSQLiteFile>();
    if (BE_SQLITE_OK != sqliteFile->GetHandleR().OpenBeSQLiteDb(filePath, Db::OpenParams(openMode)))
        {
        sqliteFile->GetHandleR().CloseDb();//seems that open errors do not automatically close the handle again
        BimConsole::WriteErrorLine("Could not open file '%s'. File might not be a BIM file, ECDb file, or BeSQLite file.", filePath.GetNameUtf8().c_str());
        return;
        }

    bmap<SessionFile::ProfileInfo::Type, SessionFile::ProfileInfo> profileInfos;
    if (!sqliteFile->TryRetrieveProfileInfos(profileInfos))
        {
        sqliteFile->GetHandleR().CloseDb();//seems that open errors do not automatically close the handle again
        BimConsole::WriteErrorLine("Could not retrieve profiles from file '%s'. Closing file again.", filePath.GetNameUtf8().c_str());
        return;
        }

    const bool isBimFile = profileInfos.find(SessionFile::ProfileInfo::Type::DgnDb) != profileInfos.end();
    if (isBimFile && !openAsECDb)
        {
        sqliteFile->GetHandleR().CloseDb();

        DbResult bimStat;
        Dgn::DgnDb::OpenParams params(openMode);
        Dgn::DgnDbPtr bim = Dgn::DgnDb::OpenDgnDb(&bimStat, filePath, params);
        if (BE_SQLITE_OK == bimStat)
            {
            session.SetFile(std::unique_ptr<SessionFile>(new BimFile(bim)));
            BimConsole::WriteLine("Opened BIM file '%s' in %s mode.", filePath.GetNameUtf8().c_str(), openModeStr);
            return;
            }

        BimConsole::WriteErrorLine("Could not open file '%s'.", filePath.GetNameUtf8().c_str());
        return;
        }

    if (profileInfos.find(SessionFile::ProfileInfo::Type::ECDb) != profileInfos.end())
        {
        sqliteFile->GetHandleR().CloseDb();

        std::unique_ptr<ECDbFile> ecdbFile = std::make_unique<ECDbFile>();
        if (BE_SQLITE_OK == ecdbFile->GetECDbHandleP()->OpenBeSQLiteDb(filePath, Db::OpenParams(openMode)))
            {
            session.SetFile(std::move(ecdbFile));
            if (isBimFile)
                BimConsole::WriteLine("Opened BIM file as ECDb file '%s' in %s mode. This can damage the file as BIM validation logic is bypassed.", filePath.GetNameUtf8().c_str(), openModeStr);
            else
                BimConsole::WriteLine("Opened ECDb file '%s' in %s mode.", filePath.GetNameUtf8().c_str(), openModeStr);
            return;
            }


        ecdbFile->GetHandleR().CloseDb();//seems that open errors do not automatically close the handle again
        BimConsole::WriteErrorLine("Could not open file '%s'.", filePath.GetNameUtf8().c_str());
        return;
        }

    session.SetFile(std::move(sqliteFile));
    BimConsole::WriteLine("Opened BeSQLite file '%s' in %s mode.", filePath.GetNameUtf8().c_str(), openModeStr);
    }


//******************************* CloseCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void CloseCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    if (session.IsFileLoaded(true))
        {
        //need to get path before closing, because afterwards it is not available on the ECDb object anymore
        Utf8String path(session.GetFile().GetPath());
        session.GetFile().GetHandleR().CloseDb();
        BimConsole::WriteLine("Closed '%s'.", path.c_str());
        }
    }

//******************************* CreateCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String CreateCommand::_GetUsage() const
    {
    return  " .create [bim|ecdb|besqlite] [<BIM root subject label>] <file path>\r\n"
        COMMAND_USAGE_IDENT "Creates a new BIM (default), ECDb file, or BeSQLite file.\r\n"
        COMMAND_USAGE_IDENT "When creating a BIM file, passing a root subject label is mandatory.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void CreateCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    std::vector<Utf8String> args = TokenizeArgs(argsUnparsed);

    if (args.empty())
        {
        BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (session.IsFileLoaded())
        {
        BimConsole::WriteErrorLine("%s file %s already loaded. Please unload it first.", session.GetFile().TypeToString(), session.GetFile().GetPath());
        return;
        }

    BeFileName filePath;
    SessionFile::Type fileType = SessionFile::Type::Bim;
    Utf8CP rootSubjectName = nullptr;
    if (args[0].EqualsIAscii("ecdb") || args[0].EqualsIAscii("besqlite"))
        {
        if (args.size() == 1)
            {
            BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
            return;
            }

        if (args.size() > 2)
            {
            BimConsole::WriteErrorLine("You can only specify a root subject label for BIM files. Usage: %s", GetUsage().c_str());
            return;
            }

        fileType = args[0].EqualsIAscii("ecdb") ? SessionFile::Type::ECDb : SessionFile::Type::BeSQLite;
        filePath.AssignUtf8(args[1].c_str());
        }
    else
        {
        size_t rootSubjectNameIndex = 0;
        size_t filePathIndex = 0;
        if (args[0].EqualsIAscii("bim"))
            {
            if (args.size() != 3)
                {
                BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
                return;
                }

            rootSubjectNameIndex = 1;
            filePathIndex = 2;
            }
        else
            {
            if (args.size() != 2)
                {
                BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
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
        BimConsole::WriteErrorLine("Cannot create %s file %s as it already exists.", SessionFile::TypeToString(fileType), filePath.GetNameUtf8().c_str());
        return;
        }

    switch (fileType)
        {
            case SessionFile::Type::Bim:
            {
            Dgn::CreateDgnDbParams createParams(rootSubjectName);
            createParams.SetOverwriteExisting(true);

            DbResult fileStatus;
            Dgn::DgnDbPtr bim = Dgn::DgnDb::CreateDgnDb(&fileStatus, filePath, createParams);
            if (BE_SQLITE_OK != fileStatus)
                {
                BimConsole::WriteErrorLine("Failed to create BIM file %s.", filePath.GetNameUtf8().c_str());
                return;
                }

            session.SetFile(std::unique_ptr<SessionFile>(new BimFile(bim)));
            BimConsole::WriteLine("Successfully created BIM file %s", filePath.GetNameUtf8().c_str());
            return;
            }

        case SessionFile::Type::ECDb:
            {
            std::unique_ptr<ECDbFile> ecdbFile = std::make_unique<ECDbFile>();
            const DbResult stat = ecdbFile->GetECDbHandleP()->CreateNewDb(filePath);
            if (BE_SQLITE_OK != stat)
                {
                BimConsole::WriteErrorLine("Failed to create ECDb file %s. See log for details.", filePath.GetNameUtf8().c_str());
                ecdbFile->GetECDbHandleP()->AbandonChanges();
                return;
                }

            BimConsole::WriteLine("Successfully created ECDb file %s and loaded it in read/write mode", filePath.GetNameUtf8().c_str());
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
            BimConsole::WriteErrorLine("Failed to create BeSQLite file %s. See log for details.", filePath.GetNameUtf8().c_str());
            sqliteFile->GetHandleR().AbandonChanges();
            return;
            }

        BimConsole::WriteLine("Successfully created BeSQLite file %s and loaded it in read/write mode", filePath.GetNameUtf8().c_str());
        sqliteFile->GetHandleR().SaveChanges();
        session.SetFile(std::move(sqliteFile));
        return;
        }
        }
    }

//******************************* FileInfoCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void FileInfoCommand::_Run(Session& session, Utf8StringCR args) const
    {
    if (!session.IsFileLoaded(true))
        return;

    BimConsole::WriteLine("Current file: ");
    BimConsole::WriteLine("  %s", session.GetFile().GetPath());

    ProfileVersion initialECDbProfileVersion(0, 0, 0, 0);
    if (session.GetFile().GetType() != SessionFile::Type::BeSQLite)
        {
        BimConsole::WriteLine("  BriefcaseId: %" PRIu32, session.GetFile().GetECDbHandle()->GetBriefcaseId().GetValue());

        if (session.GetFile().GetType() == SessionFile::Type::Bim)
            {
            Dgn::DgnDbCR bimFile = session.GetFile().GetAs<BimFile>().GetDgnDbHandle();
            BimConsole::WriteLine("  Root subject: %s", bimFile.Elements().GetRootSubject()->GetUserLabel());
            }

        Statement stmt;
        if (BE_SQLITE_OK != stmt.Prepare(session.GetFile().GetHandle(), "SELECT StrData FROM be_Prop WHERE Namespace='ec_Db' AND Name='InitialSchemaVersion'"))
            {
            BimConsole::WriteErrorLine("Could not execute SQL to retrieve profile versions.");
            return;
            }

        if (BE_SQLITE_ROW == stmt.Step())
            initialECDbProfileVersion = ProfileVersion(stmt.GetValueText(0));
        }


    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(session.GetFile().GetHandle(), "SELECT sqlite_version()"))
        {
        BimConsole::WriteErrorLine("Could not execute SQL to retrieve SQLite version.");
        return;
        }

    if (BE_SQLITE_ROW == stmt.Step())
        BimConsole::WriteLine("  SQLite version: %s", stmt.GetValueText(0));

    stmt.Finalize();

    bmap<SessionFile::ProfileInfo::Type, SessionFile::ProfileInfo> profileInfos;
    if (!session.GetFile().TryRetrieveProfileInfos(profileInfos))
        {
        BimConsole::WriteErrorLine("Could not retrieve profile infos for the current file");
        return;
        }

    BimConsole::WriteLine("  Profiles:");
    auto it = profileInfos.find(SessionFile::ProfileInfo::Type::DgnDb);
    if (it != profileInfos.end())
        BimConsole::WriteLine("    %s: %s", it->second.m_name.c_str(), it->second.m_version.ToString().c_str());

    it = profileInfos.find(SessionFile::ProfileInfo::Type::ECDb);
    if (it != profileInfos.end())
        {
        if (!initialECDbProfileVersion.IsEmpty())
            BimConsole::WriteLine("    %s: %s (Creation version: %s)", it->second.m_name.c_str(), it->second.m_version.ToString().c_str(), initialECDbProfileVersion.ToString().c_str());
        else
            BimConsole::WriteLine("    %s: %s", it->second.m_name.c_str(), it->second.m_version.ToString().c_str());
        }

    it = profileInfos.find(SessionFile::ProfileInfo::Type::BeSQLite);
    if (it != profileInfos.end())
        BimConsole::WriteLine("    %s: %s", it->second.m_name.c_str(), it->second.m_version.ToString().c_str());

    for (bpair<SessionFile::ProfileInfo::Type, SessionFile::ProfileInfo> const& profileInfo : profileInfos)
        {
        if (profileInfo.first == SessionFile::ProfileInfo::Type::Unknown)
            BimConsole::WriteLine("    %s: %s", profileInfo.second.m_name.c_str(), profileInfo.second.m_version.ToString().c_str());
        }

    }

//******************************* CommitCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     03/2014
//---------------------------------------------------------------------------------------
void CommitCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    if (!argsUnparsed.empty())
        {
        BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        return;

    if (session.GetFile().GetHandle().IsReadonly())
        {
        BimConsole::WriteErrorLine("File must be editable. Please close the file and re-open it in read-write mode.");
        return;
        }

    if (!session.GetFile().GetHandle().IsTransactionActive())
        {
        BimConsole::WriteErrorLine("Cannot commit because no transaction is active.");
        return;
        }

    DbResult stat = session.GetFile().GetHandleR().SaveChanges();
    if (stat != BE_SQLITE_OK)
        BimConsole::WriteErrorLine("Commit failed: %s", session.GetFile().GetHandle().GetLastError().c_str());
    else
        BimConsole::WriteLine("Committed current transaction and restarted it.");
    }

//******************************* RollbackCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     03/2014
//---------------------------------------------------------------------------------------
Utf8String RollbackCommand::_GetUsage() const
    {
    return " .rollback                      Rolls back the current transaction and restarts it.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     03/2014
//---------------------------------------------------------------------------------------
void RollbackCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    if (!argsUnparsed.empty())
        {
        BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        return;

    if (session.GetFile().GetHandle().IsReadonly())
        {
        BimConsole::WriteErrorLine("File must be editable. Please close the file and re-open it in read-write mode.");
        return;
        }

    if (!session.GetFile().GetHandle().IsTransactionActive())
        {
        BimConsole::WriteErrorLine("Cannot roll back because no transaction is active.");
        return;
        }

    DbResult stat = session.GetFile().GetHandleR().AbandonChanges();
    if (stat != BE_SQLITE_OK)
        BimConsole::WriteErrorLine("Rollback failed: %s", session.GetFile().GetHandle().GetLastError().c_str());
    else
        BimConsole::WriteLine("Rolled current transaction back and restarted it.");
    }

//******************************* ImportCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
//static
Utf8CP const ImportCommand::ECSCHEMA_SWITCH = "schema";
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
//static
Utf8CP const ImportCommand::CSV_SWITCH = "csv";

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
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
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void ImportCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    std::vector<Utf8String> args = TokenizeArgs(argsUnparsed);

    const size_t switchArgIndex = 0;
    if (args.size() < 2 || (!args[switchArgIndex].EqualsIAscii(ECSCHEMA_SWITCH) && !args[switchArgIndex].EqualsIAscii(CSV_SWITCH)))
        {
        BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        return;

    if (session.GetFile().GetHandle().IsReadonly())
        {
        BimConsole::WriteErrorLine("File must be editable. Please close the file and re-open it in read-write mode.");
        return;
        }

    BeFileName path(args[1]);
    path.Trim(L"\"");
    if (!path.DoesPathExist())
        {
        BimConsole::WriteErrorLine("Import failed. Specified path '%s' does not exist.", path.GetNameUtf8().c_str());
        return;
        }

    Utf8StringCR commandSwitch = args[switchArgIndex];
    if (commandSwitch.EqualsIAscii(ECSCHEMA_SWITCH))
        {
        if (!session.IsECDbFileLoaded(true))
            return;

        RunImportSchema(session, path);
        return;
        }

    BeAssert(commandSwitch.EqualsIAscii(CSV_SWITCH));
    RunImportCsv(session, path, args);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void ImportCommand::RunImportSchema(Session& session, BeFileNameCR ecschemaPath) const
    {
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(session.GetFile().GetECDbHandle()->GetSchemaLocater());

    bvector<BeFileName> ecschemaFilePaths;

    const bool isFolder = (const_cast<BeFileNameR> (ecschemaPath)).IsDirectory();
    if (isFolder)
        {
        context->AddSchemaPath(ecschemaPath);
        BeDirectoryIterator::WalkDirsAndMatch(ecschemaFilePaths, ecschemaPath, L"*.ecschema.xml", false);
        if (ecschemaFilePaths.empty())
            {
            BimConsole::WriteErrorLine("Import failed. Folder '%s' does not contain ECSchema XML files.", ecschemaPath.GetNameUtf8().c_str());
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
        BimConsole::WriteLine("Reading ECSchema ... %s", ecschemaFilePath.GetNameUtf8().c_str());
        if (SUCCESS != DeserializeECSchema(*context, ecschemaFilePath))
            {
            BimConsole::WriteErrorLine("Import failed. Could not read ECSchema '%s' into memory.", ecschemaFilePath.GetNameUtf8().c_str());
            return;
            }
        }

    Utf8CP schemaStr = isFolder ? "ECSchemas in folder" : "ECSchema";

    if (BE_SQLITE_OK != session.GetFile().GetHandleR().SaveChanges())
        {
        BimConsole::WriteLine("Saving outstanding changes in the file failed: %s", session.GetFile().GetHandle().GetLastError().c_str());
        return;
        }

    if (SUCCESS == session.GetFile().GetECDbHandle()->Schemas().ImportSchemas(context->GetCache().GetSchemas()))
        {
        session.GetFile().GetHandleR().SaveChanges();
        BimConsole::WriteLine("Successfully imported %s '%s'.", schemaStr, ecschemaPath.GetNameUtf8().c_str());
        return;
        }

    session.GetFile().GetHandleR().AbandonChanges();
    if (session.GetIssues().HasIssue())
        BimConsole::WriteErrorLine("Failed to import %s '%s': %s", schemaStr, ecschemaPath.GetNameUtf8().c_str(), session.GetIssues().GetIssue());
    else
        BimConsole::WriteErrorLine("Failed to import %s '%s'.", schemaStr, ecschemaPath.GetNameUtf8().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Affan.Khan     03/2014
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
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
void ImportCommand::RunImportCsv(Session& session, BeFileNameCR csvFilePath, std::vector<Utf8String> const& args) const
    {
    if(args.size() < 3)
        {
        BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
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
        BimConsole::WriteErrorLine("Could not open CSV file %s.", csvFilePath.GetNameUtf8().c_str());
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
            BimConsole::WriteErrorLine("Error when parsing line #d in CSV file %s.", lineNo, csvFilePath.GetNameUtf8().c_str());
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

    BimConsole::WriteLine("Successfully imported %d rows into table %s from CSV file %s", rowCount, tableName.c_str(), csvFilePath.GetNameUtf8().c_str());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
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
            BimConsole::WriteErrorLine("Table '%s' already exists but its column count differs from the column count in CSV file '%s'.",
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
            BimConsole::WriteErrorLine("Could not create table '%s' for CSV file: %s",
                                       tableName.c_str(), session.GetFile().GetHandle().GetLastError().c_str());
            return ERROR;
            }
        }

    if (BE_SQLITE_OK != stmt.Prepare(session.GetFile().GetHandle(), insertSql.c_str()))
        {
        BimConsole::WriteErrorLine("Could not prepare CSV insert statement '%s': %s",
                               insertSql.c_str(), session.GetFile().GetHandle().GetLastError().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
BentleyStatus ImportCommand::InsertCsvRow(Session& session, Statement& stmt, int columnCount, std::vector<Utf8String> const& tokens, int rowNumber) const
    {
    for (int i = 0; i < (int) tokens.size(); i++)
        {
        if (i >= columnCount)
            continue; //ignore excess tokens

        if (BE_SQLITE_OK != stmt.BindText(i + 1, tokens[i], Statement::MakeCopy::Yes))
            {
            BimConsole::WriteErrorLine("Could not bind cell value [column %d, row %d] to insert statement: %s",
                                    i + 1, rowNumber, session.GetFile().GetHandle().GetLastError().c_str());
            return ERROR;
            }
        }

    if (BE_SQLITE_DONE != stmt.Step())
        {
        BimConsole::WriteErrorLine("Could not insert row %d into table: %s",
                                rowNumber, session.GetFile().GetHandle().GetLastError().c_str());
        return ERROR;
        }

    stmt.ClearBindings();
    stmt.Reset();
    return SUCCESS;
    }


//******************************* ExportCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
//static
Utf8CP const ExportCommand::ECSCHEMA_SWITCH = "schema";
//static
Utf8CP const ExportCommand::TABLES_SWITCH = "tables";

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String ExportCommand::_GetUsage() const
    {
    return " .export schema [v2] <out folder>    Exports all ECSchemas of the file to disk. If 'v2' is specified, ECXML v2 is used.\r\n"
           COMMAND_USAGE_IDENT "Otherwise ECXML v3 is used.\r\n"
           "         tables <JSON file>     Exports the data in all tables of the file into a JSON file\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void ExportCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    std::vector<Utf8String> args = TokenizeArgs(argsUnparsed);
    const size_t argCount = args.size();
    if (!(argCount == 2 || (argCount == 2 && args[1].EqualsI("v2"))))
        {
        BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        return;

    if (args[0].EqualsI(ECSCHEMA_SWITCH))
        {
        if (!session.IsECDbFileLoaded(true))
            return;

        Utf8CP outFolder = nullptr;
        bool useECXmlV2 = false;
        if (argCount == 2)
            outFolder = args[1].c_str();
        else
            {
            useECXmlV2 = true;
            outFolder = args[2].c_str();
            }

        RunExportSchema(session, outFolder, useECXmlV2);
        return;
        }

    if (args[0].EqualsI(TABLES_SWITCH))
        {
        RunExportTables(session, args[1].c_str());
        return;
        }

    BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Affan.Khan        04/2015
//---------------------------------------------------------------------------------------
void ExportCommand::RunExportTables(Session& session, Utf8CP jsonFile) const
    {
    ExportTables(session, jsonFile);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void ExportCommand::RunExportSchema(Session& session, Utf8CP outFolderStr, bool useECXmlV2) const
    {
    bvector<ECN::ECSchemaCP> schemas = session.GetFile().GetECDbHandle()->Schemas().GetSchemas(true);
    if (schemas.empty())
        {
        BimConsole::WriteErrorLine("Failed to load schemas from file.");
        return;
        }

    BeFileName outFolder;
    outFolder.SetNameUtf8(outFolderStr);
    if (BeFileName::IsDirectory(outFolder.GetName()))
        {
        BimConsole::WriteErrorLine("Folder %s already exists. Please delete it or specify and another folder.", outFolder.GetNameUtf8().c_str());
        return;
        }
    else
        BeFileName::CreateNewDirectory(outFolder.GetName());

    ECN::ECVersion ecxmlVersion = useECXmlV2 ? ECN::ECVersion::V2_0 : ECN::ECVersion::V3_1;
    for (auto schema : schemas)
        {
        WString fileName;
        fileName.AssignUtf8(schema->GetFullSchemaName().c_str());
        fileName.append(L".ecschema.xml");

        BeFileName outPath(outFolder);
        outPath.AppendToPath(fileName.c_str());
        schema->WriteToXmlFile(outPath.GetName(), ecxmlVersion);
        BimConsole::WriteLine("Saved ECSchema '%s' to disk", outPath.GetNameUtf8().c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Affan.Khan        04/2015
//---------------------------------------------------------------------------------------
void ExportCommand::ExportTables(Session& session, Utf8CP jsonFile) const
    {
    BeFile file;
    if (file.Create(jsonFile, true) != BeFileStatus::Success)
        {
        BimConsole::WriteErrorLine("Failed to create JSON file %s", jsonFile);
        return;
        }

    Statement stmt;
    stmt.Prepare(session.GetFile().GetHandle(), "SELECT name FROM sqlite_master WHERE type ='table'");
    Json::Value tableData(Json::ValueType::arrayValue);

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ExportTable(session, tableData, stmt.GetValueText(0));
        }

    auto jsonString = tableData.toStyledString();
    if (file.Write(nullptr, jsonString.c_str(), static_cast<uint32_t>(jsonString.size())) != BeFileStatus::Success)
        {
        BimConsole::WriteErrorLine("Failed to write to JSON file %s", jsonFile);
        return;
        }

    file.Flush();
    BimConsole::WriteLine("Exported tables to '%s'", jsonFile);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Affan.Khan        04/2015
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
                    if (SUCCESS != ECJsonUtilities::BinaryToJson(row[stmt.GetColumnName(i)], (Byte const*) stmt.GetValueBlob(i), stmt.GetColumnBytes(i)))
                        {
                        BimConsole::WriteErrorLine("Failed to export table %s", tableName);
                        return;
                        }
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

//******************************* CreateClassViewsCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     12/2015
//---------------------------------------------------------------------------------------
Utf8String CreateClassViewsCommand::_GetUsage() const
    {
    return " .createclassviews <classes>    Creates or updates views in the file to visualize the EC content as ECClasses and\r\n"
        COMMAND_USAGE_IDENT "ECProperties rather than tables and columns.\r\n"
        COMMAND_USAGE_IDENT "If <classes> is specified, will create views for only those classes.  This is a space-delimited list of\r\n"
        COMMAND_USAGE_IDENT "ECClassId and/or SchemaName:ClassName.\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     12/2015
//---------------------------------------------------------------------------------------
void CreateClassViewsCommand::_Run(Session& session, Utf8StringCR args) const
    {
    if (!session.IsECDbFileLoaded(true))
        return;

    if (session.GetFile().GetHandle().IsReadonly())
        {
        BimConsole::WriteErrorLine("File must be editable. Please close the file and re-open it in read-write mode.");
        return;
        }

    bvector<ECClassId> classIds;
    if (!args.empty())
        {
        bvector<Utf8String> classTokens;
        BeStringUtilities::Split(args.c_str(), " ", classTokens);
        for (Utf8String classId : classTokens)
            {
            ECClassId id;
            if (ECClassId::FromString(id, classId.c_str()))
                classIds.push_back(id);
            else
                {
                bvector<Utf8String> components;
                BeStringUtilities::Split(classId.c_str(), ":.", components);
                if (components.size() != 2)
                    {
                    BimConsole::WriteErrorLine("Unable to convert %s to either an ECClassId or <SchemaName>.<ClassName>\n", classId.c_str());
                    }
                else
                    {
                    ECClassId id = session.GetFile().GetECDbHandle()->Schemas().GetClassId(components[0], components[1]);
                    if (id.IsValid())
                        classIds.push_back(id);
                    else
                        BimConsole::WriteErrorLine("Unable to convert %s to an ECClassId\n", classId.c_str());
                    }
                }
            }
        }
    if (0 != classIds.size())
        {
        if (SUCCESS != session.GetFile().GetECDbHandle()->Schemas().CreateClassViewsInDb(classIds))
            BimConsole::WriteErrorLine("Failed to create ECClass views in the file.");
        else
            BimConsole::WriteLine("Created or updated ECClass views in the file.");
        }
    else
        {
        if (SUCCESS != session.GetFile().GetECDbHandle()->Schemas().CreateClassViewsInDb())
            BimConsole::WriteErrorLine("Failed to create ECClass views in the file.");
        else
            BimConsole::WriteLine("Created or updated ECClass views in the file.");
        }
    }


//******************************* MetadataCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String MetadataCommand::_GetUsage() const
    {
    return " .metadata <ecsql>              Executes ECSQL and displays result column metadata";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void MetadataCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    if (argsUnparsed.empty())
        {
        BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
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
            BimConsole::WriteErrorLine("Failed to prepare ECSQL statement. %s", session.GetIssues().GetIssue());
        else
            BimConsole::WriteErrorLine("Failed to prepare ECSQL statement.");

        return;
        }

    BimConsole::WriteLine();
    BimConsole::WriteLine("Column metadata");
    BimConsole::WriteLine("===============");
    BimConsole::WriteLine("Index   Name/PropertyPath                   DisplayLabel                        Type           Root class                     Root class alias");
    BimConsole::WriteLine("----------------------------------------------------------------------------------------------------------------------------------------------");
    const int columnCount = stmt.GetColumnCount();
    for (int i = 0; i < columnCount; i++)
        {
        ECSqlColumnInfo const& columnInfo = stmt.GetColumnInfo(i);
        bool isGeneratedProp = columnInfo.IsGeneratedProperty();
        ECN::ECPropertyCP prop = columnInfo.GetProperty();
        ECSqlPropertyPathCR propPath = columnInfo.GetPropertyPath();
        Utf8String propPathStr = isGeneratedProp ? prop->GetDisplayLabel() : propPath.ToString();

        Utf8String typeName(prop->GetTypeName());
        if (prop->GetIsArray())
            typeName.append("[]");

        Utf8CP rootClassName = isGeneratedProp ? "generated" : columnInfo.GetRootClass().GetFullName();
        Utf8CP rootClassAlias = columnInfo.GetRootClassAlias();

        BimConsole::WriteLine("%3d     %-35s %-35s %-14s %-30s %s", i, propPathStr.c_str(), prop->GetDisplayLabel().c_str(), typeName.c_str(), rootClassName, rootClassAlias);
        }

    BimConsole::WriteLine();
    }

//******************************* ParseCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String ParseCommand::_GetUsage() const
    {
    return " .parse [sql|exp|token] <ecsql> Parses ECSQL. Options: sql (default): the resulting SQL is displayed.\r\n"
        COMMAND_USAGE_IDENT                "                       exp: the parsed expression tree is displayed.\r\n"
        COMMAND_USAGE_IDENT                "                       token: the parsed raw token tree is displayed.\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void ParseCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    if (argsUnparsed.empty())
        {
        BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsECDbFileLoaded(true))
        return;

    Utf8String commandSwitch;
    const size_t nextStartIndex = BimConsole::FindNextToken(commandSwitch, WString(argsUnparsed.c_str(), BentleyCharEncoding::Utf8), 0, L' ');

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
        BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
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
                    BimConsole::WriteErrorLine("Failed to parse ECSQL: %s", session.GetIssues().GetIssue());
                else
                    BimConsole::WriteErrorLine("Failed to parse ECSQL.");

                return;
                }

            BimConsole::WriteLine("ECSQL from expression tree: %s", ecsqlFromExpTree.c_str());
            BimConsole::WriteLine();
            BimConsole::WriteLine("ECSQL expression tree:");

            Utf8String expTreeStr;
            ExpTreeToString(expTreeStr, expTree, 0);
            BimConsole::WriteLine("%s", expTreeStr.c_str());
            return;
            }

            case ParseMode::Token:
            {
            Utf8String parseTree;
            if (SUCCESS != ECSqlParseTreeFormatter::ParseAndFormatECSqlParseNodeTree(parseTree, *session.GetFile().GetECDbHandle(), ecsql.c_str()))
                {
                if (session.GetIssues().HasIssue())
                    BimConsole::WriteErrorLine("Failed to parse ECSQL: %s", session.GetIssues().GetIssue());
                else
                    BimConsole::WriteErrorLine("Failed to parse ECSQL.");

                return;
                }

            BimConsole::WriteLine("Raw ECSQL parse tree:");
            BimConsole::WriteLine("%s", parseTree.c_str());
            return;
            }

            default:
            {
            ECSqlStatement stmt;
            ECSqlStatus stat = stmt.Prepare(*session.GetFile().GetECDbHandle(), ecsql.c_str());
            if (!stat.IsSuccess())
                if (session.GetIssues().HasIssue())
                    BimConsole::WriteErrorLine("Failed to parse ECSQL: %s", session.GetIssues().GetIssue());
                else
                    BimConsole::WriteErrorLine("Failed to parse ECSQL.");

            BimConsole::WriteLine("SQLite SQL: %s", stmt.GetNativeSql());
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      04/2016
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
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String ExitCommand::_GetUsage() const
    {
    return " .exit, .quit, .q               Exits the BimConsole";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void ExitCommand::_Run(Session& session, Utf8StringCR args) const { exit(0); }

//******************************* SqliteCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String SqliteCommand::_GetUsage() const
    {
    return " .sqlite <SQLite SQL>           Executes a SQLite SQL statement";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     07/2015
//---------------------------------------------------------------------------------------
void SqliteCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    Utf8StringCR sql = argsUnparsed;
    if (sql.empty())
        {
        BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        return;

    Statement stmt;
    DbResult status = stmt.Prepare(session.GetFile().GetHandle(), sql.c_str());
    if (status != BE_SQLITE_OK)
        {
        BimConsole::WriteErrorLine("Failed to prepare SQLite SQL statement %s: %s", sql.c_str(), session.GetFile().GetHandle().GetLastError().c_str());
        return;
        }

     ExecuteSelect(stmt, session);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     07/2015
//---------------------------------------------------------------------------------------
void SqliteCommand::ExecuteSelect(Statement& statement, Session& session) const
    {
    const int columnCount = statement.GetColumnCount();
    if (columnCount == 0)
        {
        if (statement.Step() != BE_SQLITE_DONE)
            {
            BimConsole::WriteErrorLine("Failed to execute SQLite SQL statement %s: %s", statement.GetSql(), session.GetFile().GetHandle().GetLastError().c_str());
            return;
            }
        }

    for (int i = 0; i < columnCount; i++)
        {
        BimConsole::Write("%s\t", statement.GetColumnName(i));
        }

    BimConsole::WriteLine();
    BimConsole::WriteLine("-------------------------------------------------------------");


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

        BimConsole::WriteLine(out.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     07/2015
//---------------------------------------------------------------------------------------
void SqliteCommand::ExecuteNonSelect(Session& session, Statement& statement) const
    {
    if (statement.Step() != BE_SQLITE_DONE)
        {
        BimConsole::WriteErrorLine("Failed to execute SQLite SQL statement %s: %s", statement.GetSql(), session.GetFile().GetHandle().GetLastError().c_str());
        return;
        }
    }

//******************************* DbSchemaCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     04/2016
//---------------------------------------------------------------------------------------
Utf8String DbSchemaCommand::_GetUsage() const
    {
    return " .dbschema search <search term> [<folder> <file extension>]\r\n"
        COMMAND_USAGE_IDENT "Searches the DDL of all DB schema elements in the current file or in all SQLite files\r\n"
        COMMAND_USAGE_IDENT "in the specified folder for the specified search term.\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     04/2016
//---------------------------------------------------------------------------------------
void DbSchemaCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    std::vector<Utf8String> args = TokenizeArgs(argsUnparsed);
    if (args.size() < 2)
        {
        BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
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

    BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     04/2016
//---------------------------------------------------------------------------------------
void DbSchemaCommand::Search(Session& session, std::vector<Utf8String> const& searchArgs) const
    {
    const size_t argSize = searchArgs.size();
    Utf8CP searchTerm = nullptr;
    const bool isFileOpen = session.IsFileLoaded(false);

    if (isFileOpen && argSize != 1)
        {
        BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }
    else if (!isFileOpen && argSize != 3)
        {
        BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
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
        BimConsole::WriteErrorLine("Command failed. Folder '%s' does not contain files with extension *.%s.",
                                folder.c_str(), fileFilter.c_str());
        return;
        }

    for (BeFileNameCR path : filePaths)
        {
        Db db;
        if (BE_SQLITE_OK != db.OpenBeSQLiteDb(path, Db::OpenParams(Db::OpenMode::Readonly)))
            {
            BimConsole::WriteErrorLine("Skipping file '%s', because it could not be opened.",
                                    path.GetNameUtf8().c_str());
            continue;
            }

        Search(db, searchTerm);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     04/2016
//---------------------------------------------------------------------------------------
void DbSchemaCommand::Search(Db const& db, Utf8CP searchTerm) const
    {
    Utf8CP sql = "SELECT name, type FROM sqlite_master WHERE sql LIKE ?";
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(db, sql))
        {
        BimConsole::WriteErrorLine("Failed to prepare SQLite SQL statement %s: %s", sql, db.GetLastError().c_str());
        return;
        }

    Utf8String searchTermWithWildcards;
    searchTermWithWildcards.Sprintf("%%%s%%", searchTerm);

    if (BE_SQLITE_OK != stmt.BindText(1, searchTermWithWildcards, Statement::MakeCopy::No))
        {
        BimConsole::WriteErrorLine("Failed to bind search term '%s' to SQLite SQL statement %s: %s", searchTermWithWildcards.c_str(), sql, db.GetLastError().c_str());
        return;
        }

    if (BE_SQLITE_ROW != stmt.Step())
        {
        BimConsole::WriteLine("The search term '%s' was not found in the DB schema elements of the file '%s'", searchTerm, db.GetDbFileName());
        return;
        }

    BimConsole::WriteLine("In the file '%s' the following DB schema elements contain the search term %s:", db.GetDbFileName(), searchTerm);
    do
        {
        BimConsole::WriteLine(" %s [%s]", stmt.GetValueText(0), stmt.GetValueText(1));
        } while (BE_SQLITE_ROW == stmt.Step());
    }

//******************************* SchemaStatsCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     03/2017
//---------------------------------------------------------------------------------------
Utf8String SchemaStatsCommand::_GetUsage() const
    {
    return " .schemastats classhierarchy rootclass [csvoutputfilepath]\r\n"
        COMMAND_USAGE_IDENT "Computes statistics for a class hierarchy or a branch of it\r\n";
        COMMAND_USAGE_IDENT "rootclass: Fully specified class name of the root class of the hierarchy\r\n";
        COMMAND_USAGE_IDENT "csvoutputfilepath: Path to CSV file containing the class column count distribution\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     03/2017
//---------------------------------------------------------------------------------------
void SchemaStatsCommand::_Run(Session& session, Utf8StringCR argsUnparsed) const
    {
    if (!session.IsECDbFileLoaded(true))
        return;
    
    std::vector<Utf8String> args = TokenizeArgs(argsUnparsed);
    if (args.empty())
        {
        BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    Utf8StringCR switchArg = args[0];

    if (switchArg.EqualsIAscii("classhierarchy"))
        {
        ComputeClassHierarchyStats(session, args);
        return;
        }

    BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
    return;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     03/2017
//---------------------------------------------------------------------------------------
void SchemaStatsCommand::ComputeClassHierarchyStats(Session& session, std::vector<Utf8String> const& args) const
    {
    if (args.size() != 2 && args.size() != 3)
        {
        BimConsole::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    Utf8StringCR rootClassName = args[1];
    bvector<Utf8String> parsedRootClassName;
    BeStringUtilities::Split(rootClassName.c_str(), ".:", parsedRootClassName);
    if (parsedRootClassName.size() != 2)
        {
        BimConsole::WriteErrorLine("Invalid root class name %s. Format must be: <schema name>.<class name> or <schema name>:<class name>", rootClassName.c_str());
        return;
        }

    ECDbCR ecdb = *session.GetFile().GetECDbHandle();
    ECClassCP rootClass = ecdb.Schemas().GetClass(parsedRootClassName[0], parsedRootClassName[1], SchemaLookupMode::AutoDetect);
    if (rootClass == nullptr || !rootClass->IsEntityClass())
        {
        BimConsole::WriteErrorLine("Root class %s is not an entity class or does not exist in the current file.", rootClass->GetFullName());
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
        BimConsole::WriteErrorLine("Preparing stats SQL failed: %s", ecdb.GetLastError().c_str());
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
        BimConsole::WriteErrorLine("Gathering Class Hierarchy stats for %s failed.", rootClass->GetFullName());
        return;
        }

    if (classStats.IsEmpty())
        {
        BimConsole::WriteLine("No class hierarchy stats available for root class %s. The class hierarchy only consists of abstract classes.", rootClass->GetFullName());
        return;
        }

    //compute stats metrics
    classStats.Sort();


    BimConsole::WriteLine("Mapped column count stats for class hierarchy of class %s", rootClass->GetFullName());
    BimConsole::WriteLine("Minimum: %" PRIu32, classStats.GetList().front().GetTotalColumnCount());
    BimConsole::WriteLine("Maximum: %" PRIu32, classStats.GetList().back().GetTotalColumnCount());
    BimConsole::WriteLine("Median: %.1f", ComputeQuantile(classStats, .5));
    BimConsole::WriteLine("80%% quantile: %.1f:", ComputeQuantile(classStats, .8));
    //Mean
    const double mean = std::accumulate(classStats.GetList().begin(), classStats.GetList().end(), 0, [] (double sum, ClassColumnStats const& stat) { return sum + stat.GetTotalColumnCount();}) / (1.0 * classStats.GetSize());
    BimConsole::WriteLine("Mean: %.1f:", mean);

    //stddev
    const double variance = std::accumulate(classStats.GetList().begin(), classStats.GetList().end(), 0.0,
                                          [mean] (double sum, ClassColumnStats const& stat) { return sum + std::pow((mean - stat.GetTotalColumnCount()), 2); }) / (1.0 * classStats.GetSize());
    BimConsole::WriteLine("Standard Deviation: %.1f:", std::sqrt(variance));

    if (args.size() == 2)
        return;

    BeFileName csvOutFilePath(args[2]);
    BeFileStatus stat = BeFileStatus::Success;
    BeTextFilePtr csvFile = BeTextFile::Open(stat, csvOutFilePath, TextFileOpenType::Write, TextFileOptions::KeepNewLine, TextFileEncoding::Utf8);
    if (BeFileStatus::Success != stat)
        {
        BimConsole::WriteErrorLine("Failed to create output CSV file %s", csvOutFilePath.GetNameUtf8().c_str());
        return;
        }

    BeAssert(csvFile != nullptr);
    //write header line
    csvFile->PutLine(L"ECClass,ClassColCount,FirstTable,FirstTableColCount,SecondTable,SecondTableColCount", true);
    for (ClassColumnStats const& stats : classStats.GetList())
        {
        Utf8String line;

        Utf8String snippet;
        snippet.Sprintf("%s,%" PRIu32, stats.GetClass().GetFullName(), stats.GetTotalColumnCount());
        line.append(snippet);
        for (std::pair<Utf8String,uint32_t> const& tableColCount : stats.GetColCountPerTable())
            {
            Utf8String snippet;
            snippet.Sprintf("%s,%" PRIu32, tableColCount.first.c_str(), tableColCount.second);
            line.append(",").append(snippet);
            }

        csvFile->PutLine(WString(line.c_str(), BentleyCharEncoding::Utf8).c_str(), true);
        }

    BimConsole::WriteLine("Saved class column count distribution to CSV file %s.", csvOutFilePath.GetNameUtf8().c_str());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     03/2017
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
// @bsimethod                                                  Krischan.Eberle     10/2016
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