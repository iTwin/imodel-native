/*--------------------------------------------------------------------------------------+
|
|     $Source: BimConsole/BimConsole.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <windows.h>

#include "BimConsole.h"
#include <Bentley/BeTimeUtilities.h>

using namespace std;
USING_NAMESPACE_BENTLEY_SQLITE_EC


//******************** Session ***********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
bool Session::IsFileLoaded(bool printMessageIfFalse) const
    {
    const bool isOpen = m_file != nullptr && m_file->IsOpen();

    if (!isOpen && printMessageIfFalse)
        Console::WriteErrorLine("No file loaded.");

    return isOpen;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle    07/2016
//---------------------------------------------------------------------------------------
BentleyStatus Session::SetFile(std::unique_ptr<SessionFile> file)
    {
    if (file == nullptr)
        return ERROR;

    m_file = std::move(file); 
    m_file->GetHandleR().AddIssueListener(m_issueListener);
    return SUCCESS;
    }

//******************** BimConsole ***********************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void BimConsole::Setup()
    {
    Console::WriteLine(" --------------------------------------------------------------------------- ");
    Console::WriteLine(" BimConsole.exe v1.0");
    Console::WriteLine(" Copyright (c) Bentley Systems 2016. All rights reserved. www.Bentley.com.");
    Console::WriteLine(" ----------------------------------------------------------------------------");
    Console::WriteLine();
    Console::WriteLine("    .help for help, .exit to exit program");
    Console::WriteLine();

    auto helpCommand = make_shared<HelpCommand>(m_commands);
    AddCommand(helpCommand);
    AddCommand(".h", helpCommand); //add same command with alternative command name

    AddCommand(make_shared<OpenCommand>());
    AddCommand(make_shared<CloseCommand>());
    AddCommand(make_shared<CreateCommand>());
    AddCommand(make_shared<FileInfoCommand>());

    AddCommand(make_shared<ECSqlCommand>());

    auto metadataCommand = make_shared<MetadataCommand>();
    AddCommand(metadataCommand);
    AddCommand(".meta", metadataCommand); //add same command with alternative command name

    AddCommand(make_shared<CommitCommand>());
    AddCommand(make_shared<RollbackCommand>());

    AddCommand(make_shared<ImportCommand>());
    AddCommand(make_shared<ExportCommand>());

    AddCommand(make_shared<ParseCommand>());
    AddCommand(make_shared<CreateECClassViewsCommand>());

    AddCommand(make_shared<SqliteCommand>());
    AddCommand(make_shared<DbSchemaCommand>());
    auto classMappingCommand = make_shared<ClassMappingCommand>();
    AddCommand(classMappingCommand);
    AddCommand(".cm", classMappingCommand); //add same command with alternative command name

    auto exitCommand = make_shared<ExitCommand>();
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
        vector<Utf8String> args;
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
void BimConsole::WritePrompt()
    {
    Console::Write("DGNDB> ");
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
        RunCommand(cmd.c_str());
        }

    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.eberle     09/2013
//---------------------------------------------------------------------------------------
//static
void BimConsole::RunCommand(Utf8CP cmd)
    {
    bool isECSqlCommand = cmd[0] != COMMAND_PREFIX;

    Command const* command = nullptr;

    vector<Utf8String> args;
    if (isECSqlCommand)
        {
        args.push_back(cmd);
        command = GetCommand(".ecsql");
        }
    else
        {
        if (!TokenizeCommandline(args, cmd) || args.empty())
            {
            Console::WriteErrorLine("Syntax error in command");
            return;
            }

        command = GetCommand(args[0].c_str());
        }

    //Unsupported command, use help command to display available commands
    if (command == nullptr)
        command = GetCommand(".help");

    BeAssert(command != nullptr);

    StopWatch executionTimer(true);
    command->Run(m_session, args);
    executionTimer.Stop();
    Console::WriteLine("[Execution Time: %.4f seconds]", executionTimer.GetElapsedSeconds());

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
        fgets(m_readBuffer, sizeof(m_readBuffer), Console::GetIn());
        if (feof(Console::GetIn()))
            return false;
        cmd.append(m_readBuffer);
        if (cmd.size() > 0)
            {
            if (cmd[0] == COMMAND_PREFIX || StringEndsWith(cmd, ECSQLSTATEMENT_DELIMITER, true))
                return true;
            else
                {
                Console::Write(" > ");
                cmd.append("\t");
                }
            }
        }
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
bool BimConsole::TokenizeCommandline(vector<Utf8String>& tokens, Utf8StringCR cmd)
    {
    Utf8String cur;
    int state = 0;
    int n = -1;
    while (n < (int) cmd.size())
        {
        n++;
        switch (state)
            {
                case 0:
                {
                if (cmd[n] == '"')
                    {
                    state = 1;
                    cur.append(&cmd[n], 1);
                    }
                else if (cmd[n] == '\'')
                    {
                    state = 2;
                    cur.append(&cmd[n], 1);
                    }
                else if (isspace(cmd[n]))
                    {
                    if (cur.empty())
                        continue;

                    tokens.push_back(cur);
                    cur.clear();
                    }
                else
                    cur.append(&cmd[n], 1);
                break;
                }
                case 1:
                {
                if (cmd[n] == '"')
                    {
                    state = 0;
                    cur.append(&cmd[n], 1);
                    cur.Trim();
                    tokens.push_back(cur);
                    cur.clear();
                    }
                else
                    cur.append(&cmd[n], 1);
                break;
                }
                case 2:
                {
                if (cmd[n] == '\'')
                    {
                    state = 0;
                    cur.append(&cmd[n], 1);
                    cur.Trim();
                    tokens.push_back(cur);
                    cur.clear();
                    }
                else
                    cur.append(&cmd[n], 1);
                break;
                }
            }
        }

    if (state != 0)
        return false; //syntax error

    cur.Trim();
    if (!cur.empty())
        tokens.push_back(cur);
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
bool BimConsole::StringEndsWith(Utf8StringCR stmt, Utf8Char ch, bool skipSpaces)
    {
    if (stmt.empty())
        return false;
    for (size_t i = stmt.size() - 1; i >= 0; i--)
        {
        if (skipSpaces)
            if (isspace(stmt[i]))
                continue;
        if (stmt[i] == ch)
            return true;
        return false;
        }
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2013
//---------------------------------------------------------------------------------------
Command const* BimConsole::GetCommand(Utf8CP commandName) const
    {
    auto it = m_commands.find(commandName);
    if (it == m_commands.end())
        return nullptr;

    return it->second.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2013
//---------------------------------------------------------------------------------------
void BimConsole::AddCommand(std::shared_ptr<Command> const& command)
    {
    AddCommand(command->GetName().c_str(), command);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2013
//---------------------------------------------------------------------------------------
//static
void BimConsole::AddCommand(Utf8CP commandName, std::shared_ptr<Command> const& command)
    {
    m_commands[commandName] = command;
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


//*************** Console ****

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
FILE* Console::GetIn() { return stdin; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
FILE* Console::GetOut() { return stdout; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
FILE* Console::GetErr() { return stderr; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void Console::Write(Utf8CP format, ...)
    {
    va_list args;
    va_start(args, format);
    Write(GetOut(), format, args);
    va_end(args);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void Console::WriteLine(Utf8CP format, ...)
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
void Console::WriteLine()
    {
    Write("%s", "\r\n");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void Console::WriteError(Utf8CP format, ...)
    {
    va_list args;
    va_start(args, format);
    Write(GetErr(), format, args);
    va_end(args);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void Console::WriteErrorLine(Utf8CP format, ...)
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
void Console::Write(FILE* stream, Utf8CP format, va_list args)
    {
    vfprintf(stream, format, args);
    }

