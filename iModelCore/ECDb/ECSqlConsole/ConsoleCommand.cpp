/*--------------------------------------------------------------------------------------+
|
|     $Source: ECSqlConsole/ConsoleCommand.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";


static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
    }

static Utf8String base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
    Utf8String ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i <4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
            }
        }

    if (i)
        {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            ret += '=';

        }

    return ret;

    }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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
    BeAssert(m_commandMap.size() == 24 && "Command was added or removed, please update the HelpCommand accordingly.");
    Console::WriteLine(m_commandMap.at(".help")->GetUsage().c_str());
    Console::WriteLine();
    Console::WriteLine(m_commandMap.at(".open")->GetUsage().c_str());
    Console::WriteLine(m_commandMap.at(".close")->GetUsage().c_str());
    Console::WriteLine(m_commandMap.at(".create")->GetUsage().c_str());
    Console::WriteLine(m_commandMap.at(".path")->GetUsage().c_str());
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
    Console::WriteLine(m_commandMap.at(".set")->GetUsage().c_str());
    Console::WriteLine();
    Console::WriteLine(m_commandMap.at(".sql")->GetUsage().c_str());
    Console::WriteLine(m_commandMap.at(".parse")->GetUsage().c_str());
    Console::WriteLine();
    Console::WriteLine(m_commandMap.at(".populate")->GetUsage().c_str());
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

    auto stat = session.GetECDbR ().OpenBeSQLiteDb(ecdbFile, ECDb::OpenParams(openMode));
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

    auto const& ecdbFileName = args.at(1);
    BeFileName beECDbFile(ecdbFileName.c_str(), true);
    if (beECDbFile.DoesPathExist())
        {
        Console::WriteErrorLine("Cannot create ECDb file %s as it already exists.", beECDbFile.GetNameUtf8().c_str());
        return;
        }
    auto createNewDbStatus = session.GetECDbR ().CreateNewDb(ecdbFileName.c_str());
    if (createNewDbStatus != BE_SQLITE_OK, BentleyStatus::SUCCESS)
        {
        Console::WriteErrorLine("Failed to create ECDb file %s", ecdbFileName.c_str());
        }

    Console::WriteLine("Successfully created ECDb file %s and loaded it in read/write mode", ecdbFileName.c_str());
    session.GetECDbR ().SaveChanges();
    }

//******************************* PathCommand ******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String PathCommand::_GetName() const
    {
    return ".path";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String PathCommand::_GetUsage() const
    {
    return " .path                          Displays full path of the open ECDb file";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void PathCommand::_Run(ECSqlConsoleSession& session, vector<Utf8String> const& args) const
    {
    if (session.HasECDb(true))
        Console::WriteLine("Current ECDb file is '%s'", session.GetECDbPath());
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

    auto stat = session.GetECDbR ().SaveChanges();
    if (stat != BE_SQLITE_OK)
        Console::WriteErrorLine("Commit failed: %s", session.GetECDb().GetLastError());
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
        Console::WriteErrorLine("Cannot roll back because no transaction is active.");
        return;
        }

    auto stat = session.GetECDbR ().AbandonChanges();
    if (stat != BE_SQLITE_OK)
        Console::WriteErrorLine("Rollback failed: %s", session.GetECDb().GetLastError());
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
        {
        return;
        }

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
    auto context = ECN::ECSchemaReadContext::CreateContext();

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
            Console::WriteErrorLine("Import failed. Could not read ECSchema '%s' into memory.", Utf8String(ecschemaFilePath).c_str());
            return;
            }
        }
    Console::WriteLine("Preparing to import ecschema. Press any key to continue ...");


    Savepoint savepoint(session.GetECDbR (), "import ecschema");
    ECDbSchemaManager::ImportOptions options(true, true);
    auto status = session.GetECDbR ().Schemas().ImportECSchemas(context->GetCache(), options);

    Utf8CP schemaStr = isFolder ? "ECSchemas in folder" : "ECSchema";

    if (status != SUCCESS)
        {
        savepoint.Cancel();

        if (session.GetIssues().HasIssue())
            Console::WriteErrorLine("Failed to import %s '%s': %s", schemaStr, ecschemaPath.GetNameUtf8().c_str(), session.GetIssues().GetIssue());
        else
            Console::WriteErrorLine("Failed to import %s '%s'.", schemaStr, ecschemaPath.GetNameUtf8().c_str());

        return;
        }

    savepoint.Commit(nullptr);
    Console::WriteLine("Successfully imported %s '%s'.", schemaStr, ecschemaPath.GetNameUtf8().c_str());
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
    return stat == ECN::SCHEMA_READ_STATUS_Success || stat == ECN::SCHEMA_READ_STATUS_DuplicateSchema ? SUCCESS : ERROR;
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
    return " .export ecschema <out folder>  Exports all ECSchemas of the ECDb file to disk\r\n"
           "         tables <JSON file>     Exports the data in all tables of the ECDb file into a JSON file";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void ExportCommand::_Run(ECSqlConsoleSession& session, vector<Utf8String> const& args) const
    {
    if (args.size() < 3)
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
        RunExportSchema(session, args[2].c_str());
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
void ExportCommand::RunExportSchema(ECSqlConsoleSession& session, Utf8CP outFolderStr) const
    {
    auto const& schemaManager = session.GetECDbR ().Schemas();
    ECSchemaList schemas;
    auto status = schemaManager.GetECSchemas(schemas, true);
    if (status != SUCCESS)
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

    for (auto schema : schemas)
        {
        WString fileName;
        fileName.AssignUtf8(schema->GetFullSchemaName().c_str());
        fileName.append(L".ecschema.xml");

        BeFileName outPath(outFolder);
        outPath.AppendToPath(fileName.c_str());
        schema->WriteToXmlFile(outPath.GetName());
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
                    row[stmt.GetColumnName(i)] = Json::Value(base64_encode((unsigned char const*)stmt.GetValueBlob(i), stmt.GetColumnBytes(i))); break;
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
    const auto argCount = args.size();
    if (argCount != 1 && argCount != 2)
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

    ECSchemaList schemaList;
    if (argCount == 2)
        {
        auto schemaName = args[1].c_str();
        ECSchemaCP schema = session.GetECDbR ().Schemas().GetECSchema(schemaName, true);
        if (schema == nullptr)
            {
            Console::WriteErrorLine("Could not find ECSchema '%s' in ECDb file.", schemaName);
            return;
            }

        schemaList.push_back(schema);
        }
    else
        {
        auto stat = session.GetECDbR ().Schemas().GetECSchemas(schemaList,true);
        if (stat != SUCCESS)
            {
            Console::WriteErrorLine("Could not retrieve the ECSchemas from the ECDb file.");
            return;
            }
        }

    //generate class list from all schemas (except standard and system schemas)
    vector<ECClassCP> classList;
    for (auto schema : schemaList)
        {
        if (ECSchema::IsStandardSchema(schema->GetName().c_str()) || schema->IsSystemSchema())
            return;

        auto const& classes = schema->GetClasses();
        for (auto ecClass : classes)
            {
            if (!ecClass->GetIsCustomAttributeClass())
                classList.push_back(ecClass);
            };
        };

    RandomECInstanceGenerator rig(classList);
    auto status = rig.Generate (true);

    if (status != BentleyStatus::SUCCESS)
        {
        Console::WriteErrorLine("Failed to generate random instances.");
        return;
        }

    auto& ecdb = session.GetECDbR ();
    auto insertECInstanceDelegate = [&ecdb] (bpair<ECClassCP, vector<IECInstancePtr>> entry)
        {
        auto ecClass = entry.first;
        auto const& instanceList = entry.second;
        ECInstanceInserter inserter(ecdb, *ecClass);
        for (auto const& instance : instanceList)
            {
            Savepoint savepoint(ecdb, "Populate");
            ECInstanceKey instanceKey;
            auto insertStatus = inserter.Insert(instanceKey, *instance);
            Utf8Char id[ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH] = "";
            ECInstanceIdHelper::ToString (id, ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH, instanceKey.GetECInstanceId ());
            instance->SetInstanceId (id);

            if (insertStatus != SUCCESS)
                {
                Console::WriteErrorLine("Could not insert ECInstance of ECClass %s into ECDb file.", Utf8String(ecClass->GetFullName()).c_str());
                savepoint.Cancel();
                }
            else
                savepoint.Commit(nullptr);
            };
        };

    auto const& generatedInstances = rig.GetGeneratedInstances();
    for_each(generatedInstances.begin(), generatedInstances.end(), insertECInstanceDelegate);

    //relationship instances can only be inserted after the regular instances
    auto const& generatedRelationshipInstances = rig.GetGeneratedRelationshipInstances();
    for_each(generatedRelationshipInstances.begin(), generatedRelationshipInstances.end(), insertECInstanceDelegate);
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
    if (status != ECSqlStatus::Success)
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
    auto columnCount = stmt.GetColumnCount();
    for (int i = 0; i < columnCount; i++)
        {
        auto const& columnInfo = stmt.GetColumnInfo(i);
        bool isGeneratedProp = columnInfo.IsGeneratedProperty();
        ECN::ECPropertyCP prop = columnInfo.GetProperty();
        ECSqlPropertyPathCR propPath = columnInfo.GetPropertyPath();
        Utf8String propPathStr = isGeneratedProp ? Utf8String(prop->GetDisplayLabel()) : propPath.ToString();
        Utf8String displayLabel(prop->GetDisplayLabel());

        Utf8String typeName(prop->GetTypeName());
        if (prop->GetIsArray())
            typeName.append("[]");

        Utf8String rootClassName = isGeneratedProp? "generated" : Utf8String(columnInfo.GetRootClass().GetFullName());
        Utf8CP rootClassAlias = columnInfo.GetRootClassAlias();

        Console::WriteLine("%3d     %-35s %-35s %-14s %-30s %s", i, propPathStr.c_str(), displayLabel.c_str(), typeName.c_str(), rootClassName.c_str(), rootClassAlias);
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
    auto stat = stmt.Prepare(session.GetECDbR (), ecsql.c_str());
    if (stat != ECSqlStatus::Success)
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
        auto ecsql = ConcatArgs(1, args);
        Utf8String expTree, ecsqlFromExpTree;
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
        Console::WriteLine("%s", expTree.c_str());
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
    if (status != ECOBJECTS_STATUS_Success)
        {
        Console::WriteErrorLine("Failed to load ECSchema file '%s'", leftECSchemaFilePath.GetNameUtf8().c_str());
        return;
        }

    status = ECSchema::ReadFromXmlFile(right,  rightECSchemaFilePath.GetName(), *contextR);
    if (status != ECOBJECTS_STATUS_Success)
        {
        Console::WriteErrorLine("Failed to load ECSchema file '%s'", rightECSchemaFilePath.GetNameUtf8().c_str());
        return;
        }

    auto diff = ECDiff::Diff(*left, *right);
    if (diff->GetStatus() != DiffStatus::DIFFSTATUS_Success)
        {
        Console::WriteErrorLine("Failed to diff schemas");
        return;
        }

    Utf8String diffText;
    if (diff->WriteToString(diffText,2) != DiffStatus::DIFFSTATUS_Success)
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

//******************************* ECSchemaDiffCommand ******************
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
    return " .sqlite <SQLite SQL>        Executes a SQLite SQL statement";
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
        Console::WriteErrorLine("Failed to prepare SQLite SQL statement %s: %s", sql.c_str(), session.GetECDb().GetLastError());
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
        Console::WriteErrorLine("Failed to execute SQLite SQL statement %s: %s", statement.GetSql (), session.GetECDb ().GetLastError ());
        return;
        }
    }