/*--------------------------------------------------------------------------------------+
|
|     $Source: ECSqlConsole/ConsoleCommand.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECDb/ECDbApi.h>
#include "ConsoleCommand.h"
#include "Console.h"
#include "ECSqlConsole.h"
#include "RandomECInstanceGenerator.h"
#include <Bentley/BeDirectoryIterator.h>
#include <json/json.h>

using namespace std;
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

//******************************* ConsoleCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String ConsoleCommand::GetName() const
    {
    return _GetName();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String ConsoleCommand::GetUsage() const
    {
    return _GetUsage();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void ConsoleCommand::Run(ECSqlConsoleSession& session, vector<Utf8String> const& args) const
    {
    session.GetIssues().Reset();
    return _Run(session, args);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
//static
Utf8String ConsoleCommand::ConcatArgs(size_t startIndex, std::vector<Utf8String> const& args)
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
Utf8String HelpCommand::_GetName() const
    {
    return ".help";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String HelpCommand::_GetUsage() const
    {
    return " .help, .h                      Displays all available commands";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void HelpCommand::_Run(ECSqlConsoleSession& session, vector<Utf8String> const& args) const
    {
    BeAssert(m_commandMap.size() == 28 && "Command was added or removed, please update the HelpCommand accordingly.");
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
    Console::WriteLine(m_commandMap.at(".commit")->GetUsage().c_str());
    Console::WriteLine(m_commandMap.at(".rollback")->GetUsage().c_str());
    Console::WriteLine();
    Console::WriteLine(m_commandMap.at(".import")->GetUsage().c_str());
    Console::WriteLine(m_commandMap.at(".export")->GetUsage().c_str());
    Console::WriteLine();
    Console::WriteLine(m_commandMap.at(".diff")->GetUsage().c_str());
    Console::WriteLine();
    Console::WriteLine(m_commandMap.at(".classmapping")->GetUsage().c_str());
    Console::WriteLine();
    Console::WriteLine(m_commandMap.at(".set")->GetUsage().c_str());
    Console::WriteLine();
    Console::WriteLine(m_commandMap.at(".sql")->GetUsage().c_str());
    Console::WriteLine(m_commandMap.at(".parse")->GetUsage().c_str());
    Console::WriteLine(m_commandMap.at(".dbschema")->GetUsage().c_str());
    Console::WriteLine();
    Console::WriteLine(m_commandMap.at(".populate")->GetUsage().c_str());
    Console::WriteLine();
    Console::WriteLine(m_commandMap.at(".createecclassviews")->GetUsage().c_str());
    Console::WriteLine();
    Console::WriteLine(m_commandMap.at(".sqlite")->GetUsage().c_str());
    Console::WriteLine();
    Console::WriteLine(m_commandMap.at(".history")->GetUsage().c_str());
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
Utf8String OpenCommand::_GetName() const
    {
    return ".open";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String OpenCommand::_GetUsage() const
    {
    return " .open [readonly|readwrite] <ecdb file>\r\n"
           "                                Opens an ECDb file. If no open mode is specified,\r\n"
           "                                the file will be opened in read-only mode.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void OpenCommand::_Run(ECSqlConsoleSession& session, vector<Utf8String> const& args) const
    {
    const auto argCount = args.size();
    if (argCount != 2 && argCount != 3)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (session.HasECDb(false))
        {
        Console::WriteErrorLine("ECDb file '%s' already open. Close it first before opening another ECDb file.", session.GetECDbPath());
        return;
        }

    auto const& firstArg = args[1];
    //default mode: read-only
    auto openMode = ECDb::OpenMode::Readonly;
    size_t filePathIndex = 1;
    bool isReadWrite = false;
    if ((isReadWrite = firstArg.EqualsI (READWRITE_SWITCH)) || firstArg.EqualsI (READONLY_SWITCH))
        {
        if (isReadWrite)
            openMode = ECDb::OpenMode::ReadWrite;

        filePathIndex = 2;
        }

    Utf8CP ecdbPath = args[filePathIndex].c_str();
    BeFileName ecdbFile;
    ecdbFile.SetNameUtf8(ecdbPath);

    if (!ecdbFile.DoesPathExist())
        {
        Console::WriteErrorLine("The path '%s' does not exist.", ecdbPath);
        return;
        }

    auto stat = session.GetECDbR().OpenBeSQLiteDb(ecdbFile, ECDb::OpenParams(openMode));
    if (stat != BE_SQLITE_OK)
        {
        session.GetECDbR().CloseDb();//seems that open errors do not automatically close the handle again
        Console::WriteErrorLine("Could not open ECDb file '%s'.", ecdbPath);
        return;
        }

    Utf8CP openModeStr = openMode == ECDb::OpenMode::Readonly? "read-only" : "read-write";
    Console::WriteLine("Opened ECDb file '%s' in %s mode.", ecdbPath, openModeStr);
    }


//******************************* CloseCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String CloseCommand::_GetName() const
    {
    return ".close";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String CloseCommand::_GetUsage() const
    {
    return " .close                         Closes the currently open ECDb file";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void CloseCommand::_Run(ECSqlConsoleSession& session, vector<Utf8String> const& args) const
    {
    if (session.HasECDb(true))
        {
        //need to get path before closing, because afterwards it is not available on the ECDb object anymore
        Utf8String path = session.GetECDbPath();
        session.GetECDbR ().CloseDb();
        Console::WriteLine("Closed '%s'.", path.c_str());
        }
    }

//******************************* CreateCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String CreateCommand::_GetName() const
    {
    return ".create";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String CreateCommand::_GetUsage() const
    {
    return  " .create <ecdb file>            Creates a new ECDb file.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void CreateCommand::_Run(ECSqlConsoleSession& session, vector<Utf8String> const& args) const
    {
    if (args.size() < 2)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (session.HasECDb(false))
        {
        Console::WriteErrorLine("ECDb file %s already loaded. Unload it first.", session.GetECDbPath());
        return;
        }

    BeFileName ecdbFileName(args[1].c_str(), true);
    if (ecdbFileName.DoesPathExist())
        {
        Console::WriteErrorLine("Cannot create ECDb file %s as it already exists.", ecdbFileName.GetNameUtf8().c_str());
        return;
        }
    
    const DbResult stat = session.GetECDbR().CreateNewDb(ecdbFileName);
    if (BE_SQLITE_OK != stat)
        {
        Console::WriteErrorLine("Failed to create ECDb file %s. See log for details.", ecdbFileName.GetNameUtf8().c_str());
        session.GetECDbR().AbandonChanges();
        return;
        }

    Console::WriteLine("Successfully created ECDb file %s and loaded it in read/write mode", ecdbFileName.GetNameUtf8().c_str());
    session.GetECDbR ().SaveChanges();
    }

//******************************* FileInfoCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String FileInfoCommand::_GetName() const
    {
    return ".fileinfo";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String FileInfoCommand::_GetUsage() const
    {
    return " .fileinfo                      Displays information about the open file";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void FileInfoCommand::_Run(ECSqlConsoleSession& session, vector<Utf8String> const& args) const
    {
    if (!session.HasECDb(true))
        return;

    Console::WriteLine("Current file: ");
    Console::WriteLine("  %s", session.GetECDbPath());
    Console::WriteLine("  BriefcaseId: %" PRIu32, session.GetECDb().GetBriefcaseId().GetValue());

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(session.GetECDb(), "SELECT StrData FROM be_Prop WHERE Namespace='ec_Db' AND Name='InitialSchemaVersion'"))
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

    if (BE_SQLITE_OK != stmt.Prepare(session.GetECDb(), "SELECT Namespace, StrData FROM be_Prop WHERE Name='SchemaVersion' ORDER BY Namespace"))
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
Utf8String CommitCommand::_GetName() const
    {
    return ".commit";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     03/2014
//---------------------------------------------------------------------------------------
Utf8String CommitCommand::_GetUsage() const
    {
    return " .commit                        Commits the current transaction and restarts it.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     03/2014
//---------------------------------------------------------------------------------------
void CommitCommand::_Run(ECSqlConsoleSession& session, vector<Utf8String> const& args) const
    {
    const auto argCount = args.size();
    if (argCount != 1)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.HasECDb(true))
        {
        return;
        }

    if (session.GetECDb().IsReadonly())
        {
        Console::WriteErrorLine("ECDb file must be editable. Please close the file and re-open it in read-write mode.");
        return;
        }

    if (!session.GetECDb().IsTransactionActive())
        {
        Console::WriteErrorLine("Cannot commit because no transaction is active.");
        return;
        }

    DbResult stat = session.GetECDbR ().SaveChanges();
    if (stat != BE_SQLITE_OK)
        Console::WriteErrorLine("Commit failed: %s", session.GetECDb().GetLastError().c_str());
    else
        Console::WriteLine("Committed current transaction and restarted it.");
    }

//******************************* RollbackCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     03/2014
//---------------------------------------------------------------------------------------
Utf8String RollbackCommand::_GetName() const
    {
    return ".rollback";
    }

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
void RollbackCommand::_Run(ECSqlConsoleSession& session, vector<Utf8String> const& args) const
    {
    const size_t argCount = args.size();
    if (argCount != 1)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.HasECDb(true))
        return;

    if (session.GetECDb().IsReadonly())
        {
        Console::WriteErrorLine("ECDb file must be editable. Please close the file and re-open it in read-write mode.");
        return;
        }

    if (!session.GetECDb().IsTransactionActive())
        {
        Console::WriteErrorLine("Cannot roll back because no transaction is active.");
        return;
        }

    DbResult stat = session.GetECDbR ().AbandonChanges();
    if (stat != BE_SQLITE_OK)
        Console::WriteErrorLine("Rollback failed: %s", session.GetECDb().GetLastError().c_str());
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
Utf8String ImportCommand::_GetName() const
    {
    return ".import";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String ImportCommand::_GetUsage() const
    {
    return " .import ecschema <ecschema xml file|folder>\r\n"
           "                                Imports the specified ECSchema XML file into the ECDb file.\r\n"
           "                                If a folder was specified, all ECSchemas in the folder\r\n"
           "                                are imported.\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void ImportCommand::_Run(ECSqlConsoleSession& session, vector<Utf8String> const& args) const
    {
    if (args.size() < 3 || !args[1].EqualsI(ECSCHEMA_SWITCH))
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.HasECDb(true))
        return;

    if (session.GetECDb().IsReadonly())
        {
        Console::WriteErrorLine("ECDb file must be editable. Please close the file and re-open it in read-write mode.");
        return;
        }

    BeFileName ecschemaPath(args[2]);
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
void ImportCommand::RunImportSchema(ECSqlConsoleSession& session, BeFileNameCR ecschemaPath) const
    {
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(session.GetECDb().GetSchemaLocater());

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

    for (auto const& ecschemaFilePath : ecschemaFilePaths)
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

    Savepoint savepoint(session.GetECDbR(), "import ecschema");
    if (SUCCESS == session.GetECDb().Schemas().ImportECSchemas(context->GetCache()))
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
Utf8String ExportCommand::_GetName() const
    {
    return ".export";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String ExportCommand::_GetUsage() const
    {
    return " .export ecschema [v2] <out folder>  Exports all ECSchemas of the ECDb file to disk. If 'v2' is specified,\r\n"
           "                                ECXML v2 is used. Otherwise ECXML v3 is used.\r\n"
           "         tables <JSON file>     Exports the data in all tables of the ECDb file into a JSON file";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void ExportCommand::_Run(ECSqlConsoleSession& session, vector<Utf8String> const& args) const
    {
    const size_t argCount = args.size();
    if (!(argCount == 3 || (argCount == 4 && args[2].EqualsI("v2"))))
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.HasECDb(true))
        {
        return;
        }

    if (args[1].EqualsI (ECSCHEMA_SWITCH))
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

    if (args[1].EqualsI (TABLES_SWITCH))
        {
        RunExportTables(session, args[2].c_str());
        return;
        }

    Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Affan.Khan        04/2015
//---------------------------------------------------------------------------------------
void ExportCommand::RunExportTables(ECSqlConsoleSession& session, Utf8CP jsonFile) const
    {
    ExportTables(session, jsonFile);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void ExportCommand::RunExportSchema(ECSqlConsoleSession& session, Utf8CP outFolderStr, bool useECXmlV2) const
    {
    bvector<ECN::ECSchemaCP> schemas = session.GetECDbR().Schemas().GetECSchemas(true);
    if (schemas.empty())
        {
        Console::WriteErrorLine("Failed to load schemas from ECdb file.");
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
void ExportCommand::ExportTables(ECSqlConsoleSession& session, Utf8CP jsonFile) const
    {
    BeFile file;
    if (file.Create(jsonFile, true) != BeFileStatus::Success)
        {
        Console::WriteErrorLine("Failed to create JSON file %s", jsonFile);
        return;
        }

    Statement stmt;
    stmt.Prepare(session.GetECDb(), "SELECT name FROM sqlite_master WHERE type ='table'");
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
void ExportCommand::ExportTable(ECSqlConsoleSession& session, Json::Value& out, Utf8CP tableName) const
    {
    auto& tableObj = out.append(Json::ValueType::objectValue);
    tableObj["Name"] = tableName;
    tableObj["Rows"] = Json::Value(Json::ValueType::arrayValue);
    auto& rows = tableObj["Rows"];
    rows.clear();
    Statement stmt;
    stmt.Prepare(session.GetECDb(), SqlPrintfString("SELECT * FROM %s", tableName));
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
//******************************* PopulateCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String PopulateCommand::_GetName() const
    {
    return ".populate";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String PopulateCommand::_GetUsage() const
    {
    return " .populate [<ecschema name>]    Randomly populates the ECDb file with ECInstances.\r\n"
           "                                If an ECSchema is specified (without version), only its classes are populated.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void PopulateCommand::_Run(ECSqlConsoleSession& session, vector<Utf8String> const& args) const
    {
    const size_t argCount = args.size();
    if (argCount != 1 && argCount != 2)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.HasECDb(true))
        return;

    if (session.GetECDb().IsReadonly())
        {
        Console::WriteErrorLine("ECDb file must be editable. Please close the file and re-open it in read-write mode.");
        return;
        }

    bvector<ECN::ECSchemaCP> schemaList;
    if (argCount == 2)
        {
        Utf8CP schemaName = args[1].c_str();
        ECSchemaCP schema = session.GetECDbR().Schemas().GetECSchema(schemaName, true);
        if (schema == nullptr)
            {
            Console::WriteErrorLine("Could not find ECSchema '%s' in ECDb file.", schemaName);
            return;
            }

        schemaList.push_back(schema);
        }
    else
        {
        schemaList = session.GetECDbR().Schemas().GetECSchemas(true);
        if (schemaList.empty())
            {
            Console::WriteErrorLine("Could not retrieve the ECSchemas from the ECDb file.");
            return;
            }
        }

    //generate class list from all schemas (except standard and system schemas)
    vector<ECClassCP> classList;
    for (ECSchemaCP schema : schemaList)
        {
        if (ECSchema::IsStandardSchema(schema->GetName().c_str()) || schema->IsSystemSchema())
            return;

        for (ECClassCP ecClass : schema->GetClasses())
            {
            if (!ecClass->IsCustomAttributeClass())
                classList.push_back(ecClass);
            };
        };

    RandomECInstanceGenerator rig(classList);
    if (SUCCESS != rig.Generate(true))
        {
        Console::WriteErrorLine("Failed to generate random instances.");
        return;
        }

    auto insertECInstanceDelegate = [] (ECDbR ecdb, ECClassCP ecClass, vector<IECInstancePtr> const& instances)
        {
        ECInstanceInserter inserter(ecdb, *ecClass);
        for (IECInstancePtr const& instance : instances)
            {
            Savepoint savepoint(ecdb, "Populate");
            ECInstanceKey key;
            if (SUCCESS != inserter.Insert(key, *instance))
                {
                Console::WriteErrorLine("Could not insert ECInstance of ECClass %s into ECDb file.", Utf8String(ecClass->GetFullName()).c_str());
                savepoint.Cancel();
                }

            savepoint.Commit(nullptr);
            };
        };

    for (bpair<ECClassCP, vector<IECInstancePtr>> const& kvPair : rig.GetGeneratedInstances())
        {
        insertECInstanceDelegate(session.GetECDbR(), kvPair.first, kvPair.second);
        }

    //relationship instances can only be inserted after the regular instances
    for (bpair<ECClassCP, vector<IECInstancePtr>> const& kvPair : rig.GetGeneratedRelationshipInstances())
        {
        insertECInstanceDelegate(session.GetECDbR(), kvPair.first, kvPair.second);
        }
    }


//******************************* CreateECClassViewsCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     12/2015
//---------------------------------------------------------------------------------------
Utf8String CreateECClassViewsCommand::_GetName() const { return ".createecclassviews"; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     12/2015
//---------------------------------------------------------------------------------------
Utf8String CreateECClassViewsCommand::_GetUsage() const
    {
    return " .createecclassviews            Creates or updates views in the ECDb file to visualize the EC content\r\n"
           "                                as ECClasses and ECProperties rather than tables and columns.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     12/2015
//---------------------------------------------------------------------------------------
void CreateECClassViewsCommand::_Run(ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const
    {
    if (!session.HasECDb(true))
        return;

    if (session.GetECDb().IsReadonly())
        {
        Console::WriteErrorLine("ECDb file must be editable. Please close the file and re-open it in read-write mode.");
        return;
        }

    if (SUCCESS != session.GetECDb().Schemas().CreateECClassViewsInDb())
        Console::WriteErrorLine("Failed to create EC database views in the ECDb file.");
    else
        Console::WriteLine("Created or updated EC database views in the ECDb file.");
    }


//******************************* MetadataCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String MetadataCommand::_GetName() const
    {
    return ".metadata";
    }

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
void MetadataCommand::_Run(ECSqlConsoleSession& session, vector<Utf8String> const& args) const
    {
    const size_t argSize = args.size();
    if (argSize <= 1)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.HasECDb(true))
        return;

    Utf8String ecsql;
    for (size_t i = 1; i < argSize; i++)
        {
        ecsql.append(args[i]);
        
        if (i != argSize - 1)
            ecsql.append(" ");
        }


    ECSqlStatement stmt;
    ECSqlStatus status = stmt.Prepare(session.GetECDbR (), ecsql.c_str());
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

        Utf8CP rootClassName = isGeneratedProp? "generated" : columnInfo.GetRootClass().GetFullName();
        Utf8CP rootClassAlias = columnInfo.GetRootClassAlias();

        Console::WriteLine("%3d     %-35s %-35s %-14s %-30s %s", i, propPathStr.c_str(), prop->GetDisplayLabel(), typeName.c_str(), rootClassName, rootClassAlias);
        }

    Console::WriteLine();
    }

//******************************* SqlCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String SqlCommand::_GetName() const
    {
    return ".sql";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String SqlCommand::_GetUsage() const
    {
    return " .sql <ecsql>                   Parses ECSQL and displays resulting SQLite SQL";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void SqlCommand::_Run(ECSqlConsoleSession& session, vector<Utf8String> const& args) const
    {
    const size_t argSize = args.size();
    if (argSize <= 1)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.HasECDb(true))
        return;

    Utf8String ecsql = ConcatArgs(1, args);

    ECSqlStatement stmt;
    ECSqlStatus stat = stmt.Prepare(session.GetECDbR (), ecsql.c_str());
    if (!stat.IsSuccess())
        {
        if (session.GetIssues().HasIssue())
            Console::WriteErrorLine("Failed to parse ECSQL: %s", session.GetIssues().GetIssue());
        else
            Console::WriteErrorLine("Failed to parse ECSQL.");

        return;
        }

    Console::WriteLine("SQLite SQL: %s", stmt.GetNativeSql());
    return;
    }

//******************************* ParseCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
//static
Utf8CP const ParseCommand::RAW_SWITCH = "raw";

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String ParseCommand::_GetName() const
    {
    return ".parse";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String ParseCommand::_GetUsage() const
    {
    return " .parse [raw] <ecsql>           Parses ECSQL and displays parse tree. If 'raw' is specified \r\n"
           "                                the raw parse tree before resolving / validating tokens\r\n"
           "                                against the ECSchema is displayed.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void ParseCommand::_Run(ECSqlConsoleSession& session, vector<Utf8String> const& args) const
    {
    const size_t argCount = args.size();
    if (argCount < 2)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.HasECDb(true))
        return;

    if (args[1].EqualsI (RAW_SWITCH))
        {
        if (argCount < 3)
            {
            Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
            return;
            }

        auto ecsql = ConcatArgs(2, args);

        Utf8String parseTree;
        if (SUCCESS != ECSqlParseTreeFormatter::ParseAndFormatECSqlParseNodeTree(parseTree, session.GetECDb(), ecsql.c_str()))
            {
            if (session.GetIssues().HasIssue())
                Console::WriteErrorLine("Failed to parse ECSQL: %s", session.GetIssues().GetIssue());
            else
                Console::WriteErrorLine("Failed to parse ECSQL.");

            return;
            }

        Console::WriteLine("Raw ECSQL parse tree:");
        Console::WriteLine("%s", parseTree.c_str());
        }
    else
        {
        Utf8String ecsql = ConcatArgs(1, args);
        Utf8String ecsqlFromExpTree;
        Json::Value expTree;
        if (SUCCESS != ECSqlParseTreeFormatter::ParseAndFormatECSqlExpTree(expTree, ecsqlFromExpTree, session.GetECDbR(), ecsql.c_str()))
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

//******************************* SetCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
//static
Utf8CP const SetCommand::OUTPUT_SWITCH = "output";
Utf8CP const SetCommand::OUTPUT_TABLE_SWITCH = "table";
Utf8CP const SetCommand::OUTPUT_LIST_SWITCH = "list";

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String SetCommand::_GetName() const
    {
    return ".set";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String SetCommand::_GetUsage() const
    {
    return " .set output [table|list]       Sets the output format for ECSQL results";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void SetCommand::_Run(ECSqlConsoleSession& session, vector<Utf8String> const& args) const
    {
    const size_t argCount = args.size();
    if (argCount < 2 || !args[1].EqualsI (OUTPUT_SWITCH))
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (argCount == 3)
        {
        auto const& outputFormatStr = args[2];
        if (outputFormatStr.EqualsI (OUTPUT_TABLE_SWITCH))
            {
            session.SetOutputFormat(OutputFormat::Table);
            }
        else if (outputFormatStr.EqualsI (OUTPUT_LIST_SWITCH))
            {
            session.SetOutputFormat(OutputFormat::List);
            }
        else
            {
            Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
            return;
            }
        }

    switch (session.GetOutputFormat())
        {
        case OutputFormat::List:
            Console::WriteLine("ECSQL result output format: List");
            break;
        case OutputFormat::Table:
            Console::WriteLine("ECSQL result output format: Tabular");
            break;
        default:
            BeAssert(false);
            break;
        }
    }

//******************************* HistoryCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String HistoryCommand::_GetName() const
    {
    return ".history";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String HistoryCommand::_GetUsage() const
    {
    return " .history                       Displays command history";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void HistoryCommand::_Run(ECSqlConsoleSession& session, vector<Utf8String> const& args) const
    {
    auto const& commandHistory = session.GetCommandHistory();
    int i = 0;
    for_each(commandHistory.rbegin(), commandHistory.rend(), [&i] (Utf8StringCR command)
        {
        i++;
        Console::WriteLine("%3d  %s", i, command.c_str());
        });
    }

//******************************* ExitCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String ExitCommand::_GetName() const
    {
    return ".exit";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String ExitCommand::_GetUsage() const
    {
    return " .exit, .quit, .q               Exits the ECSQL Console";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void ExitCommand::_Run(ECSqlConsoleSession& session, vector<Utf8String> const& args) const
    {
    exit(0);
    }

//******************************* ECSchemaDiffCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String ECSchemaDiffCommand::_GetName() const
    {
    return ".diff";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String ECSchemaDiffCommand::_GetUsage() const
    {
    return " .diff <schemafile1> <schemafile2> [<output-file>]\r\n"           
           "                                Find differences between two ecschemas and optionally save\r\n"
           "                                the output to a outputfile";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void ECSchemaDiffCommand::_Run(ECSqlConsoleSession& session, vector<Utf8String> const& args) const
    {
    if (args.size() < 3)
        {
        Console::WriteLine(GetUsage().c_str());
        return;
        }
    BeFileName leftECSchemaFilePath = BeFileName(args.at(1).c_str(), true);
    BeFileName rightECSchemaFilePath = BeFileName(args.at(2).c_str(), true);
    BeFileName out;
    if (args.size() > 3)
        out = BeFileName(args.at(3).c_str(), true);

    if (!leftECSchemaFilePath.DoesPathExist())
        {
        Console::WriteErrorLine("ECSchema file '%s' does not exist", leftECSchemaFilePath.GetNameUtf8().c_str());
        return;
        }
    if (!rightECSchemaFilePath.DoesPathExist())
        {
        Console::WriteErrorLine("ECSchema file '%s' does not exist", rightECSchemaFilePath.GetNameUtf8().c_str());
        return;
        }

    ECSchemaPtr left, right;
    auto contextL = ECSchemaReadContext::CreateContext();
    auto contextR = ECSchemaReadContext::CreateContext();

    auto status = ECSchema::ReadFromXmlFile(left,  leftECSchemaFilePath.GetName(), *contextL);
    if (status != SchemaReadStatus::Success)
        {
        Console::WriteErrorLine("Failed to load ECSchema file '%s'", leftECSchemaFilePath.GetNameUtf8().c_str());
        return;
        }

    status = ECSchema::ReadFromXmlFile(right,  rightECSchemaFilePath.GetName(), *contextR);
    if (status != SchemaReadStatus::Success)
        {
        Console::WriteErrorLine("Failed to load ECSchema file '%s'", rightECSchemaFilePath.GetNameUtf8().c_str());
        return;
        }

    auto diff = ECDiff::Diff(*left, *right);
    if (diff->GetStatus() != DiffStatus::Success)
        {
        Console::WriteErrorLine("Failed to diff schemas");
        return;
        }

    Utf8String diffText;
    if (diff->WriteToString(diffText,2) != DiffStatus::Success)
        {
        Console::WriteErrorLine("Failed to convert diff into textual representation");
        return;
        }
    if (out.empty())
        Console::WriteLine( diffText.c_str());
    else
        {
        BeFile file;
        Utf8String outData = Utf8String(diffText.c_str());
        if (file.Create(out.c_str()) != BeFileStatus::Success)
            {
            Console::WriteLine("Failed to create file '%s'.", out.GetNameUtf8());
            return;
            }

        file.Write(nullptr, outData.c_str(), static_cast<uint32_t>(outData.size()));
        file.Close();
        Console::WriteLine("Written diff to '%s'.", out.GetNameUtf8());
        }
    }

//******************************* SqliteCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     07/2015
//---------------------------------------------------------------------------------------
Utf8String SqliteCommand::_GetName() const
    {
    return ".sqlite";
    }

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
void SqliteCommand::_Run(ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const
    {
    const size_t argSize = args.size();
    if (argSize <= 1)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    if (!session.HasECDb(true))
        return;

    Utf8String sql = ConcatArgs(1, args);

    Statement stmt;
    DbResult status = stmt.Prepare(session.GetECDbR(), sql.c_str());
    if (status != BE_SQLITE_OK)
        {
        Console::WriteErrorLine("Failed to prepare SQLite SQL statement %s: %s", sql.c_str(), session.GetECDb().GetLastError().c_str());
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
void SqliteCommand::ExecuteNonSelect(ECSqlConsoleSession& session, Statement& statement) const
    {
    if (statement.Step() != BE_SQLITE_DONE)
        {
        Console::WriteErrorLine("Failed to execute SQLite SQL statement %s: %s", statement.GetSql (), session.GetECDb ().GetLastError ().c_str());
        return;
        }
    }

//******************************* DbSchemaCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     04/2016
//---------------------------------------------------------------------------------------
Utf8String DbSchemaCommand::_GetName() const
    {
    return ".dbschema";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     04/2016
//---------------------------------------------------------------------------------------
Utf8String DbSchemaCommand::_GetUsage() const
    {
    return " .dbschema search <search term> [<folder> <file extension>]\r\n"
           "                                Searches the DB schemas in the current ECDb file or\r\n"
           "                                in all ECDb files in the specified folder for the specified\r\n"
           "                                search term.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     04/2016
//---------------------------------------------------------------------------------------
void DbSchemaCommand::_Run(ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const
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
void DbSchemaCommand::Search(ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const
    {
    const size_t argSize = args.size();
    Utf8CP searchTerm = nullptr;
    const bool isECDbOpen = session.HasECDb(false);

    if (isECDbOpen && argSize != 3)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }
    else if (!isECDbOpen && argSize != 5)
        {
        Console::WriteErrorLine("Usage: %s", GetUsage().c_str());
        return;
        }

    searchTerm = args[2].c_str();

    if (isECDbOpen)
        {
        Search(session.GetECDb(), searchTerm);
        return;
        }

    bvector<BeFileName> ecdbFilePaths;
    Utf8String ecdbFolder(args[3]);
    ecdbFolder.Trim("\"");
    
    Utf8String fileExtension(args[4]);
    Utf8String fileFilter;
    if (fileExtension.StartsWith("*."))
        fileFilter = fileExtension;
    else if (fileExtension.StartsWith("."))
        fileFilter.Sprintf("*%s", fileExtension.c_str());
    else 
        fileFilter.Sprintf("*.%s", fileExtension.c_str());

    BeDirectoryIterator::WalkDirsAndMatch(ecdbFilePaths, BeFileName(ecdbFolder), WString(fileFilter.c_str(), BentleyCharEncoding::Utf8).c_str(), true);
    
    if (ecdbFilePaths.empty())
        {
        Console::WriteErrorLine("Command failed. Folder '%s' does not contain ECDb files with extension *.%s.",
                   ecdbFolder, fileFilter.c_str());
        return;
        }

    for (BeFileNameCR ecdbPath : ecdbFilePaths)
        {
        ECDb ecdb;
        if (BE_SQLITE_OK != ecdb.OpenBeSQLiteDb(ecdbPath, ECDb::OpenParams(Db::OpenMode::Readonly)))
            {
            Console::WriteErrorLine("Skipping ECDb file '%s', because it could not be opened.",
                                    ecdbPath.GetNameUtf8().c_str());
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
        Console::WriteLine("The search term '%s' was not found in the DB schema elements of the ECDb file '%s'", searchTerm, ecdb.GetDbFileName());
        return;
        }

    Console::WriteLine("In the ECDb file '%s' the following DB schema elements contain the search term %s:", ecdb.GetDbFileName(), searchTerm);
    do
        {
        Console::WriteLine(" %s [%s]", stmt.GetValueText(0), stmt.GetValueText(1));
        }
    while (BE_SQLITE_ROW == stmt.Step());
    }


//******************************* ClassMappingCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     04/2016
//---------------------------------------------------------------------------------------
Utf8String ClassMappingCommand::_GetName() const
    {
    return ".classmapping";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     04/2016
//---------------------------------------------------------------------------------------
Utf8String ClassMappingCommand::_GetUsage() const
    {
    return " .classmapping, .cm [<ECSchemaName>|<ECSchemaName> <ECClassName>]\r\n"
           "                                Returns ECClass mapping information";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     04/2016
//---------------------------------------------------------------------------------------
void ClassMappingCommand::_Run(ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const
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
        if (SUCCESS != ClassMappingInfoHelper::GetInfos(json, session.GetECDb(), false))
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

        if (SUCCESS != ClassMappingInfoHelper::GetInfos(json, session.GetECDb(), args[2].c_str(), false))
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

        if (SUCCESS != ClassMappingInfoHelper::GetInfo(json, session.GetECDb(), args[2].c_str(), args[3].c_str()))
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
