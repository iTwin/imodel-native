/*--------------------------------------------------------------------------------------+
|
|     $Source: BimConsole/Command.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECDb/ECDbApi.h>
#include "Command.h"
#include "BimConsole.h"
#include <Bentley/BeDirectoryIterator.h>

using namespace std;
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DGN;

//******************************* Command ******************

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void Command::Run(Session& session, std::vector<Utf8String> const& args) const
    {
    session.GetIssues().Reset();
    return _Run(session, args);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
//static
Utf8String Command::ConcatArgs(size_t startIndex, std::vector<Utf8String> const& args)
    {
    const size_t count = args.size();
    if (startIndex > count)
        {
        BeAssert(false);
        return "";
        }

    Utf8String argStr;
    for (size_t i = startIndex; i < count; i++)
        {
        argStr.append(args[i]);

        if (i != count - 1)
            argStr.append(" ");
        }

    return argStr;
    }

//******************************* HelpCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void HelpCommand::_Run(Session& session, vector<Utf8String> const& args) const
    {
    BeAssert(m_commandMap.size() == 23 && "Command was added or removed, please update the HelpCommand accordingly.");
    Console::WriteLine(m_commandMap.at(".help")->GetUsage().c_str());
    Console::WriteLine();
    Console::WriteLine(m_commandMap.at(".open")->GetUsage().c_str());
    Console::WriteLine(m_commandMap.at(".close")->GetUsage().c_str());
    Console::WriteLine(m_commandMap.at(".create")->GetUsage().c_str());
    Console::WriteLine(m_commandMap.at(".fileinfo")->GetUsage().c_str());
    Console::WriteLine();
    Console::WriteLine(m_commandMap.at(".ecsql")->GetUsage().c_str());
    Console::WriteLine(m_commandMap.at(".metadata")->GetUsage().c_str());
    Console::WriteLine();
    Console::WriteLine(m_commandMap.at(".createecclassviews")->GetUsage().c_str());
    Console::WriteLine();
    Console::WriteLine(m_commandMap.at(".commit")->GetUsage().c_str());
    Console::WriteLine(m_commandMap.at(".rollback")->GetUsage().c_str());
    Console::WriteLine();
    Console::WriteLine(m_commandMap.at(".import")->GetUsage().c_str());
    Console::WriteLine(m_commandMap.at(".export")->GetUsage().c_str());
    Console::WriteLine();
    Console::WriteLine(m_commandMap.at(".classmapping")->GetUsage().c_str());
    Console::WriteLine();
    Console::WriteLine(m_commandMap.at(".parse")->GetUsage().c_str());
    Console::WriteLine(m_commandMap.at(".dbschema")->GetUsage().c_str());
    Console::WriteLine();
    Console::WriteLine(m_commandMap.at(".sqlite")->GetUsage().c_str());
    Console::WriteLine();
    Console::WriteLine(m_commandMap.at(".exit")->GetUsage().c_str());
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
    return " .open [readonly|readwrite] <BIM/ECDb file>\r\n"
        COMMAND_USAGE_IDENT "Opens a BIM or ECDb file. Default open mode: read-only.\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void OpenCommand::_Run(Session& session, vector<Utf8String> const& args) const
    {
    const auto argCount = args.size();
    if (argCount != 2 && argCount != 3)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (session.IsFileLoaded())
        {
        Console::WriteErrorLine("%s file '%s' already open. Close it first before opening another file.", session.GetFile().GetPath());
        return;
        }

    Utf8String const& firstArg = args[1];
    //default mode: read-only
    ECDb::OpenMode openMode = ECDb::OpenMode::Readonly;
    size_t filePathIndex = 1;
    bool isReadWrite = false;
    if ((isReadWrite = firstArg.EqualsI(READWRITE_SWITCH)) || firstArg.EqualsI(READONLY_SWITCH))
        {
        if (isReadWrite)
            openMode = ECDb::OpenMode::ReadWrite;

        filePathIndex = 2;
        }


    BeFileName filePath;
    filePath.SetNameUtf8(args[filePathIndex].c_str());
    filePath.Trim(L"\"");
    if (!filePath.DoesPathExist())
        {
        Console::WriteErrorLine("The path '%s' does not exist.", filePath.GetNameUtf8().c_str());
        return;
        }

    Utf8CP openModeStr = openMode == ECDb::OpenMode::Readonly ? "read-only" : "read-write";

    DbResult bimStat;
    DgnDb::OpenParams params(openMode);
    DgnDbPtr bim = DgnDb::OpenDgnDb(&bimStat, filePath, params);
    if (BE_SQLITE_OK == bimStat)
        {
        session.SetFile(std::unique_ptr<SessionFile>(new BimFile(bim)));
        Console::WriteLine("Opened BIM file '%s' in %s mode.", filePath.GetNameUtf8().c_str(), openModeStr);
        return;
        }

    std::unique_ptr<ECDbFile> ecdbFile(new ECDbFile());
    if (BE_SQLITE_OK == ecdbFile->GetHandleR().OpenBeSQLiteDb(filePath, ECDb::OpenParams(openMode)))
        {
        session.SetFile(std::move(ecdbFile));
        Console::WriteLine("Opened ECDb file '%s' in %s mode.", filePath.GetNameUtf8().c_str(), openModeStr);
        return;
        }
    else
        ecdbFile->GetHandleR().CloseDb();//seems that open errors do not automatically close the handle again

    Console::WriteErrorLine("Could not open file '%s'.", filePath.GetNameUtf8().c_str());
    }


//******************************* CloseCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void CloseCommand::_Run(Session& session, vector<Utf8String> const& args) const
    {

    if (session.IsFileLoaded(true))
        {
        //need to get path before closing, because afterwards it is not available on the ECDb object anymore
        Utf8String path(session.GetFile().GetPath());
        session.GetFile().GetHandleR().CloseDb();
        Console::WriteLine("Closed '%s'.", path.c_str());
        }
    }

//******************************* CreateCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String CreateCommand::_GetUsage() const
    {
    return  " .create [bim|ecdb] <file path> Creates a new BIM (default) or ECDb file.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void CreateCommand::_Run(Session& session, vector<Utf8String> const& args) const
    {
    if (args.size() < 2)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (session.IsFileLoaded())
        {
        Console::WriteErrorLine("%s file %s already loaded. Please unload it first.", session.GetFile().TypeToString(), session.GetFile().GetPath());
        return;
        }

    BeFileName filePath;
    SessionFile::Type fileType = SessionFile::Type::Bim;
    if (args.size() == 3)
        {
        if (args[1].EqualsIAscii("ecdb"))
            fileType = SessionFile::Type::ECDb;

        filePath.AssignUtf8(args[2].c_str());
        }
    else
        filePath.AssignUtf8(args[1].c_str());

    filePath.Trim(L"\"");
    if (filePath.DoesPathExist())
        {
        Console::WriteErrorLine("Cannot create %s file %s as it already exists.", SessionFile::TypeToString(fileType), filePath.GetNameUtf8().c_str());
        return;
        }

    if (fileType == SessionFile::Type::Bim)
        {
        CreateDgnDbParams createParams;
        createParams.SetOverwriteExisting(true);

        DbResult fileStatus;
        DgnDbPtr bim = DgnDb::CreateDgnDb(&fileStatus, filePath, createParams);
        if (BE_SQLITE_OK != fileStatus)
            {
            Console::WriteErrorLine("Failed to create BIM file %s.", filePath.GetNameUtf8().c_str());
            return;
            }

        session.SetFile(std::unique_ptr<SessionFile>(new BimFile(bim)));
        Console::WriteLine("Successfully created BIM file %s", filePath.GetNameUtf8().c_str());
        return;
        }

    //plain ECDb file
    std::unique_ptr<ECDbFile> ecdbFile(new ECDbFile());
    const DbResult stat = ecdbFile->GetHandleR().CreateNewDb(filePath);
    if (BE_SQLITE_OK != stat)
        {
        Console::WriteErrorLine("Failed to create ECDb file %s. See log for details.", filePath.GetNameUtf8().c_str());
        ecdbFile->GetHandleR().AbandonChanges();
        return;
        }

    Console::WriteLine("Successfully created ECDb file %s and loaded it in read/write mode", filePath.GetNameUtf8().c_str());
    ecdbFile->GetHandleR().SaveChanges();
    session.SetFile(std::move(ecdbFile));
    }

//******************************* FileInfoCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void FileInfoCommand::_Run(Session& session, vector<Utf8String> const& args) const
    {
    if (!session.IsFileLoaded(true))
        return;

    Console::WriteLine("Current file: ");
    Console::WriteLine("  %s", session.GetFile().GetPath());
    Console::WriteLine("  BriefcaseId: %" PRIu32, session.GetFile().GetHandle().GetBriefcaseId().GetValue());

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(session.GetFile().GetHandle(), "SELECT StrData FROM be_Prop WHERE Namespace='ec_Db' AND Name='InitialSchemaVersion'"))
        {
        Console::WriteErrorLine("Could not execute SQL to retrieve profile versions.");
        return;
        }

    SchemaVersion initialECDbProfileVersion(0, 0, 0, 0);
    if (BE_SQLITE_ROW == stmt.Step())
        {
        initialECDbProfileVersion = SchemaVersion(stmt.GetValueText(0));
        }

    stmt.Finalize();

    if (BE_SQLITE_OK != stmt.Prepare(session.GetFile().GetHandle(), "SELECT Namespace, StrData FROM be_Prop WHERE Name='SchemaVersion' ORDER BY Namespace"))
        {
        Console::WriteErrorLine("Could not execute SQL to retrieve profile versions.");
        return;
        }

    bmap<KnownProfile, Utf8String> knownProfileInfos;
    std::vector<Utf8String> otherProfileInfos;
    Utf8CP profileFormat = "    %s: %s";
    while (BE_SQLITE_ROW == stmt.Step())
        {
        SchemaVersion profileVersion(stmt.GetValueText(1));
        Utf8CP profileName = stmt.GetValueText(0);

        KnownProfile knownProfile = KnownProfile::Unknown;

        Utf8String profileInfo;
        Utf8String addendum;

        //use human readable names for core profiles
        if (BeStringUtilities::StricmpAscii(profileName, "be_Db") == 0)
            {
            profileName = "BeSQLite";
            knownProfile = KnownProfile::BeSQLite;
            }
        else if (BeStringUtilities::StricmpAscii(profileName, "ec_Db") == 0)
            {
            profileName = "ECDb";
            knownProfile = KnownProfile::ECDb;

            if (!initialECDbProfileVersion.IsEmpty())
                addendum.Sprintf(" (Creation version: %s)", initialECDbProfileVersion.ToString().c_str());
            }
        else if (BeStringUtilities::StricmpAscii(profileName, "dgn_Proj") == 0)
            {
            profileName = "DgnDb";
            knownProfile = KnownProfile::DgnDb;
            }

        profileInfo.Sprintf(profileFormat, profileName, profileVersion.ToString().c_str());
        if (!addendum.empty())
            profileInfo.append(addendum);

        if (knownProfile == KnownProfile::Unknown)
            otherProfileInfos.push_back(profileInfo);
        else
            knownProfileInfos[knownProfile] = profileInfo;
        }

    Console::WriteLine("  Profiles:");
    Console::WriteLine(knownProfileInfos[KnownProfile::BeSQLite].c_str());
    Console::WriteLine(knownProfileInfos[KnownProfile::ECDb].c_str());
    Console::WriteLine(knownProfileInfos[KnownProfile::DgnDb].c_str());
    for (Utf8StringCR otherProfileInfo : otherProfileInfos)
        {
        Console::WriteLine(otherProfileInfo.c_str());
        }
    }

//******************************* CommitCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     03/2014
//---------------------------------------------------------------------------------------
void CommitCommand::_Run(Session& session, vector<Utf8String> const& args) const
    {
    const auto argCount = args.size();
    if (argCount != 1)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        return;

    if (session.GetFile().GetHandle().IsReadonly())
        {
        Console::WriteErrorLine("File must be editable. Please close the file and re-open it in read-write mode.");
        return;
        }

    if (!session.GetFile().GetHandle().IsTransactionActive())
        {
        Console::WriteErrorLine("Cannot commit because no transaction is active.");
        return;
        }

    DbResult stat = session.GetFile().GetHandleR().SaveChanges();
    if (stat != BE_SQLITE_OK)
        Console::WriteErrorLine("Commit failed: %s", session.GetFile().GetHandle().GetLastError().c_str());
    else
        Console::WriteLine("Committed current transaction and restarted it.");
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
void RollbackCommand::_Run(Session& session, vector<Utf8String> const& args) const
    {
    const size_t argCount = args.size();
    if (argCount != 1)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        return;

    if (session.GetFile().GetHandle().IsReadonly())
        {
        Console::WriteErrorLine("File must be editable. Please close the file and re-open it in read-write mode.");
        return;
        }

    if (!session.GetFile().GetHandle().IsTransactionActive())
        {
        Console::WriteErrorLine("Cannot roll back because no transaction is active.");
        return;
        }

    DbResult stat = session.GetFile().GetHandleR().AbandonChanges();
    if (stat != BE_SQLITE_OK)
        Console::WriteErrorLine("Rollback failed: %s", session.GetFile().GetHandle().GetLastError().c_str());
    else
        Console::WriteLine("Rolled current transaction back and restarted it.");
    }

//******************************* ImportCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
//static
Utf8CP const ImportCommand::ECSCHEMA_SWITCH = "ecschema";

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String ImportCommand::_GetUsage() const
    {
    return " .import ecschema <ecschema xml file|folder>\r\n"
        COMMAND_USAGE_IDENT "Imports the specified ECSchema XML file into the file. If a folder was specified, all ECSchemas\r\n"
        COMMAND_USAGE_IDENT "in the folder are imported.\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void ImportCommand::_Run(Session& session, vector<Utf8String> const& args) const
    {
    if (args.size() < 3 || !args[1].EqualsI(ECSCHEMA_SWITCH))
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        return;

    if (session.GetFile().GetHandle().IsReadonly())
        {
        Console::WriteErrorLine("File must be editable. Please close the file and re-open it in read-write mode.");
        return;
        }

    BeFileName ecschemaPath(args[2]);
    ecschemaPath.Trim(L"\"");
    if (!ecschemaPath.DoesPathExist())
        {
        Console::WriteErrorLine("Import failed. Specified path '%s' does not exist.", ecschemaPath.GetNameUtf8().c_str());
        return;
        }

    RunImportSchema(session, ecschemaPath);
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void ImportCommand::RunImportSchema(Session& session, BeFileNameCR ecschemaPath) const
    {
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(session.GetFile().GetHandle().GetSchemaLocater());

    bvector<BeFileName> ecschemaFilePaths;

    const bool isFolder = (const_cast<BeFileNameR> (ecschemaPath)).IsDirectory();
    if (isFolder)
        {
        context->AddSchemaPath(ecschemaPath);
        BeDirectoryIterator::WalkDirsAndMatch(ecschemaFilePaths, ecschemaPath, L"*.ecschema.xml", false);
        if (ecschemaFilePaths.empty())
            {
            Console::WriteErrorLine("Import failed. Folder '%s' does not contain ECSchema XML files.", ecschemaPath.GetNameUtf8().c_str());
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
        Console::WriteLine("Reading ECSchema ... %s", ecschemaFilePath.GetNameUtf8());
        if (SUCCESS != DeserializeECSchema(*context, ecschemaFilePath))
            {
            Console::WriteErrorLine("Import failed. Could not read ECSchema '%s' into memory.", ecschemaFilePath.GetNameUtf8());
            return;
            }
        }
    Console::WriteLine("Preparing to import ecschema. Press any key to continue ...");


    Utf8CP schemaStr = isFolder ? "ECSchemas in folder" : "ECSchema";

    Savepoint savepoint(session.GetFile().GetHandleR(), "import ecschema");
    if (SUCCESS == session.GetFile().GetHandle().Schemas().ImportECSchemas(context->GetCache()))
        {
        savepoint.Commit(nullptr);
        Console::WriteLine("Successfully imported %s '%s'.", schemaStr, ecschemaPath.GetNameUtf8().c_str());
        return;
        }

    savepoint.Cancel();
    if (session.GetIssues().HasIssue())
        Console::WriteErrorLine("Failed to import %s '%s': %s", schemaStr, ecschemaPath.GetNameUtf8().c_str(), session.GetIssues().GetIssue());
    else
        Console::WriteErrorLine("Failed to import %s '%s'.", schemaStr, ecschemaPath.GetNameUtf8().c_str());
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


//******************************* ExportCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
//static
Utf8CP const ExportCommand::ECSCHEMA_SWITCH = "ecschema";
//static
Utf8CP const ExportCommand::TABLES_SWITCH = "tables";

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String ExportCommand::_GetUsage() const
    {
    return " .export ecschema [v2] <out folder>  Exports all ECSchemas of the file to disk. If 'v2' is specified, ECXML v2 is used.\r\n"
           COMMAND_USAGE_IDENT "Otherwise ECXML v3 is used.\r\n"
           "         tables <JSON file>     Exports the data in all tables of the file into a JSON file\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void ExportCommand::_Run(Session& session, vector<Utf8String> const& args) const
    {
    const size_t argCount = args.size();
    if (!(argCount == 3 || (argCount == 4 && args[2].EqualsI("v2"))))
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        {
        return;
        }

    if (args[1].EqualsI(ECSCHEMA_SWITCH))
        {
        Utf8CP outFolder = nullptr;
        bool useECXmlV2 = false;
        if (argCount == 3)
            outFolder = args[2].c_str();
        else
            {
            useECXmlV2 = true;
            outFolder = args[3].c_str();
            }

        RunExportSchema(session, outFolder, useECXmlV2);
        return;
        }

    if (args[1].EqualsI(TABLES_SWITCH))
        {
        RunExportTables(session, args[2].c_str());
        return;
        }

    Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
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
    bvector<ECN::ECSchemaCP> schemas = session.GetFile().GetHandle().Schemas().GetECSchemas(true);
    if (schemas.empty())
        {
        Console::WriteErrorLine("Failed to load schemas from file.");
        return;
        }

    BeFileName outFolder;
    outFolder.SetNameUtf8(outFolderStr);
    if (BeFileName::IsDirectory(outFolder.GetName()))
        {
        Console::WriteErrorLine("Folder %s already exists. Please delete it or specify and another folder.", outFolder.GetNameUtf8().c_str());
        return;
        }
    else
        BeFileName::CreateNewDirectory(outFolder.GetName());

    int ecxmlMajorVersion = useECXmlV2 ? 2 : 3;
    for (auto schema : schemas)
        {
        WString fileName;
        fileName.AssignUtf8(schema->GetFullSchemaName().c_str());
        fileName.append(L".ecschema.xml");

        BeFileName outPath(outFolder);
        outPath.AppendToPath(fileName.c_str());
        schema->WriteToXmlFile(outPath.GetName(), ecxmlMajorVersion);
        Console::WriteLine("Saved ECSchema '%s' to disk", outPath.GetNameUtf8().c_str());
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
        Console::WriteErrorLine("Failed to create JSON file %s", jsonFile);
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
        Console::WriteErrorLine("Failed to write to JSON file %s", jsonFile);
        return;
        }

    file.Flush();
    Console::WriteLine("Exported tables to '%s'", jsonFile);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Affan.Khan        04/2015
//---------------------------------------------------------------------------------------
void ExportCommand::ExportTable(Session& session, Json::Value& out, Utf8CP tableName) const
    {
    auto& tableObj = out.append(Json::ValueType::objectValue);
    tableObj["Name"] = tableName;
    tableObj["Rows"] = Json::Value(Json::ValueType::arrayValue);
    auto& rows = tableObj["Rows"];
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
                        Console::WriteErrorLine("Failed to export table %s", tableName);
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

//******************************* CreateECClassViewsCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     12/2015
//---------------------------------------------------------------------------------------
Utf8String CreateECClassViewsCommand::_GetUsage() const
    {
    return " .createecclassviews            Creates or updates views in the file to visualize the EC content as ECClasses and\r\n"
        COMMAND_USAGE_IDENT "ECProperties rather than tables and columns.\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     12/2015
//---------------------------------------------------------------------------------------
void CreateECClassViewsCommand::_Run(Session& session, std::vector<Utf8String> const& args) const
    {
    if (!session.IsFileLoaded(true))
        return;

    if (session.GetFile().GetHandle().IsReadonly())
        {
        Console::WriteErrorLine("File must be editable. Please close the file and re-open it in read-write mode.");
        return;
        }

    if (SUCCESS != session.GetFile().GetHandle().Schemas().CreateECClassViewsInDb())
        Console::WriteErrorLine("Failed to create ECClass views in the file.");
    else
        Console::WriteLine("Created or updated ECClass views in the file.");
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
void MetadataCommand::_Run(Session& session, vector<Utf8String> const& args) const
    {
    const size_t argSize = args.size();
    if (argSize <= 1)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        return;

    Utf8String ecsql;
    for (size_t i = 1; i < argSize; i++)
        {
        ecsql.append(args[i]);

        if (i != argSize - 1)
            ecsql.append(" ");
        }


    ECSqlStatement stmt;
    ECSqlStatus status = stmt.Prepare(session.GetFile().GetHandle(), ecsql.c_str());
    if (!status.IsSuccess())
        {
        if (session.GetIssues().HasIssue())
            Console::WriteErrorLine("Failed to prepare ECSQL statement. %s", session.GetIssues().GetIssue());
        else
            Console::WriteErrorLine("Failed to prepare ECSQL statement.");

        return;
        }

    Console::WriteLine();
    Console::WriteLine("Column metadata");
    Console::WriteLine("===============");
    Console::WriteLine("Index   Name/PropertyPath                   DisplayLabel                        Type           Root class                     Root class alias");
    Console::WriteLine("----------------------------------------------------------------------------------------------------------------------------------------------");
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

        Console::WriteLine("%3d     %-35s %-35s %-14s %-30s %s", i, propPathStr.c_str(), prop->GetDisplayLabel(), typeName.c_str(), rootClassName, rootClassAlias);
        }

    Console::WriteLine();
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
void ParseCommand::_Run(Session& session, vector<Utf8String> const& args) const
    {
    const size_t argCount = args.size();
    if (argCount < 2)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        return;

    Utf8StringCR firstArg = args[1];
    if (firstArg.EqualsIAscii("exp"))
        {
        if (argCount < 3)
            {
            Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
            return;
            }

        Utf8String ecsql = ConcatArgs(2, args);
        Utf8String ecsqlFromExpTree;
        Json::Value expTree;
        if (SUCCESS != ECSqlParseTreeFormatter::ParseAndFormatECSqlExpTree(expTree, ecsqlFromExpTree, session.GetFile().GetHandle(), ecsql.c_str()))
            {
            if (session.GetIssues().HasIssue())
                Console::WriteErrorLine("Failed to parse ECSQL: %s", session.GetIssues().GetIssue());
            else
                Console::WriteErrorLine("Failed to parse ECSQL.");
            return;
            }

        Console::WriteLine("ECSQL from expression tree: %s", ecsqlFromExpTree.c_str());
        Console::WriteLine();
        Console::WriteLine("ECSQL expression tree:");

        Utf8String expTreeStr;
        ExpTreeToString(expTreeStr, expTree, 0);
        Console::WriteLine("%s", expTreeStr.c_str());
        return;
        }

    if (firstArg.EqualsIAscii("token"))
        {
        if (argCount < 3)
            {
            Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
            return;
            }

        Utf8String ecsql = ConcatArgs(2, args);

        Utf8String parseTree;
        if (SUCCESS != ECSqlParseTreeFormatter::ParseAndFormatECSqlParseNodeTree(parseTree, session.GetFile().GetHandle(), ecsql.c_str()))
            {
            if (session.GetIssues().HasIssue())
                Console::WriteErrorLine("Failed to parse ECSQL: %s", session.GetIssues().GetIssue());
            else
                Console::WriteErrorLine("Failed to parse ECSQL.");

            return;
            }

        Console::WriteLine("Raw ECSQL parse tree:");
        Console::WriteLine("%s", parseTree.c_str());
        return;
        }

    Utf8String ecsql;
    if (firstArg.EqualsIAscii("sql"))
        {
        if (argCount < 3)
            {
            Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
            return;
            }

        ecsql = ConcatArgs(2, args);
        }

    //default mode ('sql' was not specified)
    ecsql = ConcatArgs(1, args);

    ECSqlStatement stmt;
    ECSqlStatus stat = stmt.Prepare(session.GetFile().GetHandle(), ecsql.c_str());
    if (!stat.IsSuccess())
        if (session.GetIssues().HasIssue())
            Console::WriteErrorLine("Failed to parse ECSQL: %s", session.GetIssues().GetIssue());
        else
            Console::WriteErrorLine("Failed to parse ECSQL.");

    Console::WriteLine("SQLite SQL: %s", stmt.GetNativeSql());
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
void ExitCommand::_Run(Session& session, vector<Utf8String> const& args) const
    {
    exit(0);
    }

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
void SqliteCommand::_Run(Session& session, std::vector<Utf8String> const& args) const
    {
    const size_t argSize = args.size();
    if (argSize <= 1)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.IsFileLoaded(true))
        return;

    Utf8String sql = ConcatArgs(1, args);

    Statement stmt;
    DbResult status = stmt.Prepare(session.GetFile().GetHandle(), sql.c_str());
    if (status != BE_SQLITE_OK)
        {
        Console::WriteErrorLine("Failed to prepare SQLite SQL statement %s: %s", sql.c_str(), session.GetFile().GetHandle().GetLastError().c_str());
        return;
        }

    if (strnicmp(sql.c_str(), "SELECT", 6) == 0)
        ExecuteSelect(stmt);
    else
        ExecuteNonSelect(session, stmt);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     07/2015
//---------------------------------------------------------------------------------------
void SqliteCommand::ExecuteSelect(Statement& statement) const
    {
    const int columnCount = statement.GetColumnCount();
    for (int i = 0; i < columnCount; i++)
        {
        Console::Write("%s\t", statement.GetColumnName(i));
        }

    Console::WriteLine();
    Console::WriteLine("-------------------------------------------------------------");

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

        Console::WriteLine(out.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     07/2015
//---------------------------------------------------------------------------------------
void SqliteCommand::ExecuteNonSelect(Session& session, Statement& statement) const
    {
    if (statement.Step() != BE_SQLITE_DONE)
        {
        Console::WriteErrorLine("Failed to execute SQLite SQL statement %s: %s", statement.GetSql(), session.GetFile().GetHandle().GetLastError().c_str());
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
void DbSchemaCommand::_Run(Session& session, std::vector<Utf8String> const& args) const
    {
    if (args.size() < 2)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    Utf8StringCR switchArg = args[1];

    if (switchArg.EqualsI("search"))
        {
        Search(session, args);
        return;
        }

    Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     04/2016
//---------------------------------------------------------------------------------------
void DbSchemaCommand::Search(Session& session, std::vector<Utf8String> const& args) const
    {
    const size_t argSize = args.size();
    Utf8CP searchTerm = nullptr;
    const bool isFileOpen = session.IsFileLoaded(false);

    if (isFileOpen && argSize != 3)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }
    else if (!isFileOpen && argSize != 5)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    searchTerm = args[2].c_str();

    if (isFileOpen)
        {
        Search(session.GetFile().GetHandle(), searchTerm);
        return;
        }

    bvector<BeFileName> filePaths;
    Utf8String folder(args[3]);
    folder.Trim("\"");

    Utf8String fileExtension(args[4]);
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
        Console::WriteErrorLine("Command failed. Folder '%s' does not contain files with extension *.%s.",
                                folder, fileFilter.c_str());
        return;
        }

    for (BeFileNameCR path : filePaths)
        {
        ECDb ecdb;
        if (BE_SQLITE_OK != ecdb.OpenBeSQLiteDb(path, ECDb::OpenParams(Db::OpenMode::Readonly)))
            {
            Console::WriteErrorLine("Skipping file '%s', because it could not be opened.",
                                    path.GetNameUtf8().c_str());
            continue;
            }

        Search(ecdb, searchTerm);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     04/2016
//---------------------------------------------------------------------------------------
void DbSchemaCommand::Search(ECDbCR ecdb, Utf8CP searchTerm) const
    {
    Utf8CP sql = "SELECT name, type FROM sqlite_master WHERE sql LIKE ?";
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, sql))
        {
        Console::WriteErrorLine("Failed to prepare SQLite SQL statement %s: %s", sql, ecdb.GetLastError().c_str());
        return;
        }

    Utf8String searchTermWithWildcards;
    searchTermWithWildcards.Sprintf("%%%s%%", searchTerm);

    if (BE_SQLITE_OK != stmt.BindText(1, searchTermWithWildcards, Statement::MakeCopy::No))
        {
        Console::WriteErrorLine("Failed to bind search term '%s' to SQLite SQL statement %s: %s", searchTermWithWildcards.c_str(), sql, ecdb.GetLastError().c_str());
        return;
        }

    if (BE_SQLITE_ROW != stmt.Step())
        {
        Console::WriteLine("The search term '%s' was not found in the DB schema elements of the file '%s'", searchTerm, ecdb.GetDbFileName());
        return;
        }

    Console::WriteLine("In the file '%s' the following DB schema elements contain the search term %s:", ecdb.GetDbFileName(), searchTerm);
    do
        {
        Console::WriteLine(" %s [%s]", stmt.GetValueText(0), stmt.GetValueText(1));
        } while (BE_SQLITE_ROW == stmt.Step());
    }


//******************************* ClassMappingCommand ******************

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     04/2016
//---------------------------------------------------------------------------------------
Utf8String ClassMappingCommand::_GetUsage() const
    {
    return " .classmapping, .cm [<ECSchemaName>|<ECSchemaName> <ECClassName>]\r\n"
        COMMAND_USAGE_IDENT "Returns ECClass mapping information for the file, the specified ECSchema, or the specified ECClass.\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     04/2016
//---------------------------------------------------------------------------------------
void ClassMappingCommand::_Run(Session& session, std::vector<Utf8String> const& args) const
    {
    if (args.size() < 2)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    Utf8StringCR switchArg = args[1];

    Json::Value json;
    if (switchArg.EqualsI("*"))
        {
        if (SUCCESS != ClassMappingInfoHelper::GetInfos(json, session.GetFile().GetHandle(), false))
            {
            Console::WriteErrorLine("Retrieving ECClass mapping failed.");
            return;
            }
        }
    else if (switchArg.EqualsI("ecschema"))
        {
        if (args.size() < 3)
            {
            Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
            return;
            }

        if (SUCCESS != ClassMappingInfoHelper::GetInfos(json, session.GetFile().GetHandle(), args[2].c_str(), false))
            {
            Console::WriteErrorLine("Retrieving ECClass mapping failed.");
            return;
            }
        }
    else if (switchArg.EqualsI("ecclass"))
        {
        if (args.size() < 4)
            {
            Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
            return;
            }

        if (SUCCESS != ClassMappingInfoHelper::GetInfo(json, session.GetFile().GetHandle(), args[2].c_str(), args[3].c_str()))
            {
            Console::WriteErrorLine("Retrieving ECClass mapping failed.");
            return;
            }
        }
    else
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());

    Json::FastWriter writer;
    Utf8String infoStr = writer.write(json);
    Console::WriteLine(infoStr.c_str());
    }
