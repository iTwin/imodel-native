/*--------------------------------------------------------------------------------------+
|
|     $Source: BimConsole/BimConsole.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <windows.h>

#include "BimConsole.h"
#include <Bentley/BeTimeUtilities.h>

USING_NAMESPACE_BENTLEY_SQLITE_EC


//******************** Session ***********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
bool Session::IsFileLoaded(bool printMessageIfFalse) const
    {
    const bool isOpen = m_file != nullptr && m_file->IsOpen();

    if (!isOpen && printMessageIfFalse)
        BimConsole::WriteErrorLine("No file loaded.");

    return isOpen;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle    01/2017
//---------------------------------------------------------------------------------------
bool Session::IsECDbFileLoaded(bool printMessageIfFalse) const
    {
    if (!IsFileLoaded(printMessageIfFalse))
        return false;

    const bool isECDbFile = m_file->GetECDbHandle() != nullptr;
    if (!isECDbFile && printMessageIfFalse)
        BimConsole::WriteErrorLine("Command requires BIM or ECDb, but currently opened file is only a BeSQLite file.");

    return isECDbFile;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle    07/2016
//---------------------------------------------------------------------------------------
BentleyStatus Session::SetFile(std::unique_ptr<SessionFile> file)
    {
    if (file == nullptr)
        return ERROR;

    m_file = std::move(file);
    ECDb* ecdb = m_file->GetECDbHandleP();
    if (ecdb != nullptr)
        ecdb->AddIssueListener(m_issueListener);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle    01/2017
//---------------------------------------------------------------------------------------
//static
Utf8CP SessionFile::TypeToString(Type type)
    {
    switch (type)
        {
            case Type::Bim:
                return "BIM";
            case Type::ECDb:
                return "ECDb";
            case Type::BeSQLite:
                return "BeSQLite";

            default:
                BeAssert(false);
                return "error";
        }
    }

//******************** BimConsole ***********************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void BimConsole::Setup()
    {
    WriteLine(" --------------------------------------------------------------------------- ");
    WriteLine(" BimConsole.exe v1.0");
    WriteLine(" Copyright (c) Bentley Systems 2017. All rights reserved. www.Bentley.com.");
    WriteLine(" ----------------------------------------------------------------------------");
    WriteLine();
    WriteLine("    .help for help, .exit to exit program");
    WriteLine();

    auto helpCommand = std::make_shared<HelpCommand>(m_commands);
    AddCommand(helpCommand);
    AddCommand(".h", helpCommand); //add same command with alternative command name

    AddCommand(std::make_shared<OpenCommand>());
    AddCommand(std::make_shared<CloseCommand>());
    AddCommand(std::make_shared<CreateCommand>());
    AddCommand(std::make_shared<FileInfoCommand>());

    AddCommand(std::make_shared<ECSqlCommand>());

    auto metadataCommand = std::make_shared<MetadataCommand>();
    AddCommand(metadataCommand);
    AddCommand(".meta", metadataCommand); //add same command with alternative command name

    AddCommand(std::make_shared<CommitCommand>());
    AddCommand(std::make_shared<RollbackCommand>());

    AddCommand(std::make_shared<ImportCommand>());
    AddCommand(std::make_shared<ExportCommand>());

    AddCommand(std::make_shared<ParseCommand>());
    AddCommand(std::make_shared<CreateECClassViewsCommand>());

    AddCommand(std::make_shared<SqliteCommand>());
    AddCommand(std::make_shared<DbSchemaCommand>());

    AddCommand(std::make_shared<DebugCommand>());

    auto exitCommand = std::make_shared<ExitCommand>();
    AddCommand(exitCommand);
    AddCommand(".quit", exitCommand); //add same command with alternative command name
    AddCommand(".q", exitCommand); //add same command with alternative command name
    AddCommand(".bye", exitCommand); //add same command with alternative command name
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
int BimConsole::Run(int argc, WCharP argv[])
    {
    //Initialize BimConsole and Print out banner 
    Setup();

    if (argc > 1)
        {
        Command const* openCommand = GetCommand(".open");
        BeAssert(openCommand != nullptr);
        std::vector<Utf8String> args;
        args.push_back(openCommand->GetName());
        args.push_back(Utf8String(argv[1]));

        if (argc == 3)
            args.push_back(Utf8String(argv[2]));

        openCommand->Run(m_session, args);
        }

    return WaitForUserInput(argc, argv);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
int BimConsole::WaitForUserInput(int argc, WCharP argv[])
    {
    Utf8String cmd;

    //Run the Read-Execute command loop
    while (ReadLine(cmd))
        {
        cmd.Trim();
        RunCommand(cmd);
        }

    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.eberle     09/2013
//---------------------------------------------------------------------------------------
//static
void BimConsole::RunCommand(Utf8StringCR cmd)
    {
    bool isECSqlCommand = cmd[0] != COMMAND_PREFIX;

    Command const* command = nullptr;

    std::vector<Utf8String> args;
    if (isECSqlCommand)
        {
        args.push_back(cmd);
        command = GetCommand(".ecsql");
        }
    else
        {
        Command::Tokenize(args, WString(cmd.c_str(), BentleyCharEncoding::Utf8), L' ', L'"');
        if (args.empty())
            {
            WriteErrorLine("Syntax error in command");
            return;
            }

        command = GetCommand(args[0]);
        }

    //Unsupported command, use help command to display available commands
    if (command == nullptr)
        command = GetCommand(".help");

    BeAssert(command != nullptr);

    StopWatch executionTimer(true);
    command->Run(m_session, args);
    executionTimer.Stop();
    WriteLine("[Execution Time: %.4f seconds]", executionTimer.GetElapsedSeconds());

    //Add command to history
    AddToHistory(cmd);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
bool BimConsole::ReadLine(Utf8StringR cmd)
    {
    WritePrompt();
    cmd.clear();
    while (true)
        {
        fgets(m_readBuffer, sizeof(m_readBuffer), GetIn());
        if (feof(GetIn()))
            return false;
        cmd.append(m_readBuffer);
        if (cmd.size() > 0)
            {
            if (cmd[0] == COMMAND_PREFIX || cmd[cmd.size() -1] == ECSQLSTATEMENT_DELIMITER)
                return true;
            else
                {
                Write(" > ");
                cmd.append("\t");
                }
            }
        }
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2013
//---------------------------------------------------------------------------------------
Command const* BimConsole::GetCommand(Utf8StringCR commandName) const
    {
    auto it = m_commands.find(commandName);
    if (it == m_commands.end())
        return nullptr;

    return it->second.get();
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle    07/2016
//---------------------------------------------------------------------------------------
BeSQLite::L10N::SqlangFiles BimConsole::_SupplySqlangFiles()
    {
    BeFileName defaultSqlang(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    defaultSqlang.AppendToPath(L"sqlang/DgnPlatform_en.sqlang.db3");
    return BeSQLite::L10N::SqlangFiles(defaultSqlang);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
FILE* BimConsole::GetIn() { return stdin; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
FILE* BimConsole::GetOut() { return stdout; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
FILE* BimConsole::GetErr() { return stderr; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void BimConsole::Write(Utf8CP format, ...)
    {
    va_list args;
    va_start(args, format);
    Write(GetOut(), format, args);
    va_end(args);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void BimConsole::WriteLine(Utf8CP format, ...)
    {
    va_list args;
    va_start(args, format);
    Write(GetOut(), format, args);
    va_end(args);
    WriteLine();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void BimConsole::WriteLine()
    {
    Write("%s", "\r\n");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void BimConsole::WriteError(Utf8CP format, ...)
    {
    va_list args;
    va_start(args, format);
    Write(GetErr(), format, args);
    va_end(args);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void BimConsole::WriteErrorLine(Utf8CP format, ...)
    {
    va_list args;
    va_start(args, format);
    Write(GetErr(), format, args);
    va_end(args);
    WriteLine();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void BimConsole::Write(FILE* stream, Utf8CP format, va_list args)
    {
    vfprintf(stream, format, args);
    }

