/*--------------------------------------------------------------------------------------+
|
|     $Source: ECSqlConsole/ECSqlConsole.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <windows.h>

#include "ECSqlConsole.h"
#include "Console.h"
#include "ECSqlStatementIterator.h"
#include <Bentley/BeTimeUtilities.h>
using namespace std;
USING_NAMESPACE_BENTLEY_SQLITE_EC

//******************** ECSqlConsoleSession ***********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
ECSqlConsoleSession::ECSqlConsoleSession ()
    : m_outputFormat (OutputFormat::Table)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
bool ECSqlConsoleSession::HasECDb( bool printMessageIfFalse ) const
    {
    const auto isOpen = m_ecdb.IsDbOpen();

    if (!isOpen && printMessageIfFalse)
        {
        Console::WriteErrorLine ("No ECDb loaded.");
        }

    return isOpen;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
Utf8CP ECSqlConsoleSession::GetECDbPath() const
    {
    return m_ecdb.GetDbFileName ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
void ECSqlConsoleSession::AddToHistory( Utf8CP command )
    {
    m_commandHistory.push_back (command);
    }



//******************** ECSqlConsole ***********************
//static
ECSqlConsoleSession ECSqlConsole::s_session;
ConsoleCommandMap ECSqlConsole::s_commands;
char ECSqlConsole::s_readBuffer [ECSqlConsole::MaxReadBufferLineSize];

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
//static
void ECSqlConsole::Setup ()
    {
    Console::WriteLine (" --------------------------------------------------------------------------- ");
    Console::WriteLine (" ECSqlConsole.exe v%d.%d", ECSQLCONSOLE_MAJ_VER, ECSQLCONSOLE_MIN_VER); 
    Console::WriteLine (" Copyright (c) Bentley Systems 2013. All rights reserved. www.Bentley.com.");
    Console::WriteLine (" ----------------------------------------------------------------------------");
    Console::WriteLine ();
    Console::WriteLine ("    .help for help, .exit to quit program");
    Console::WriteLine ();

    auto helpCommand = make_shared<HelpCommand> (s_commands);
    AddCommand (helpCommand);
    AddCommand (".h", helpCommand); //add same command with alternative command name
    
    AddCommand (make_shared<OpenCommand> ());
    AddCommand (make_shared<CloseCommand> ());
    AddCommand (make_shared<CreateCommand> ());
    AddCommand (make_shared<PathCommand> ());

    AddCommand (make_shared<ECSqlCommand> ());

    auto metadataCommand = make_shared<MetadataCommand> ();
    AddCommand (metadataCommand);
    AddCommand (".meta", metadataCommand); //add same command with alternative command name

    AddCommand (make_shared<CommitCommand> ());
    AddCommand (make_shared<RollbackCommand> ());

    AddCommand (make_shared<ImportCommand> ());
    AddCommand (make_shared<ExportCommand> ());
    AddCommand (make_shared<ECSchemaDiffCommand> ());

    AddCommand (make_shared<SqlCommand> ());
    AddCommand (make_shared<ParseCommand> ());
    AddCommand (make_shared<SetCommand> ());
    AddCommand (make_shared<PopulateCommand> ());

    AddCommand (make_shared<HistoryCommand> ());
    auto exitCommand = make_shared<ExitCommand> ();
    AddCommand (exitCommand);
    AddCommand (".quit", exitCommand); //add same command with alternative command name
    AddCommand (".q", exitCommand); //add same command with alternative command name
    AddCommand (".bye", exitCommand); //add same command with alternative command name
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
//static
int ECSqlConsole::Run (int argc, WCharP argv[])
    {
    //Initialize ECSqlConsole and Print out banner 
    Setup ();

    if (argc > 1)
        {
        auto openCommand = GetCommand (".open");
        BeAssert (openCommand != nullptr);
        vector<Utf8String> args;
        args.push_back (openCommand->GetName ());
        args.push_back (Utf8String (argv[1]));

        if (argc == 3)
            args.push_back (Utf8String (argv[2]));

        openCommand->Run (GetSession (), args);
        }

    return WaitForUserInput(argc, argv);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
//static
int ECSqlConsole::WaitForUserInput (int argc, WCharP argv[])
    {
    Utf8String cmd;

    //Run the Read-Execute command loop
    while (ReadLine(cmd))
        {
        cmd.Trim ();

        RunCommand (cmd.c_str ());

        }

    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.eberle     09/2013
//---------------------------------------------------------------------------------------
//static
void ECSqlConsole::RunCommand (Utf8CP cmd)
    {
    bool isECSqlCommand = cmd[0] != CommandPrefix;

    ConsoleCommand const* command = nullptr;

    vector<Utf8String> args;
    if (isECSqlCommand)
        {
        args.push_back (cmd);
        command = GetCommand (".ecsql");
        }
    else
        {
        if (!TokenizeCommandline (args, cmd) || args.empty())
            {
            Console::WriteErrorLine ("Syntax error in command");
            return;
            }

        command = GetCommand (args[0].c_str ());
        }

    //Unsupported command, use help command to display available commands
    if (command == nullptr)
        command = GetCommand (".help");

    BeAssert (command != nullptr);

    StopWatch executionTimer (true);
    command->Run (GetSession (), args);
    executionTimer.Stop ();
    if (GetSession ().GetECDb ().IsDbOpen ())
        {
        Console::WriteLine ("\r\n[Sqlite rows modified: %d]", GetSession ().GetECDb ().GetModifiedRowCount ());
        Console::WriteLine ("[Execution Time: %.4f seconds]", executionTimer.GetElapsedSeconds ());
        }
    //Add command to history (except history command itself)
    if (dynamic_cast<HistoryCommand const*> (command) == nullptr)
        GetSession ().AddToHistory (cmd);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
//static
bool ECSqlConsole::ReadLine (Utf8StringR cmd)
    {
    WritePrompt();
    cmd.clear();
    while (true)
        {
        fgets(s_readBuffer, sizeof(s_readBuffer), Console::GetIn());
        if (feof(Console::GetIn()))
            return false;
        cmd.append (s_readBuffer);
        if (cmd.size() > 0)
            {
            if (cmd[0] == CommandPrefix || StringEndsWith(cmd, ECSqlStatementDelimiterChar, true))
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
//static
bool ECSqlConsole::TokenizeCommandline (vector<Utf8String>& tokens, Utf8StringCR cmd)
    {
    Utf8String cur;
    int state = 0;
    int n = -1;
    while ( n < (int)cmd.size())
        {
        n++;
        switch(state)
            {
            case 0:
                {
                if (cmd[n] == '"')
                    {
                    state = 1;
                    cur.append (&cmd[n], 1);
                    }
                else if (cmd[n] == '\'')
                    {
                    state = 2;
                    cur.append (&cmd[n], 1);
                    }
                else if (isspace (cmd[n]))
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
                    cur.append (&cmd[n], 1);
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
                    cur.append (&cmd[n], 1);
                    cur.Trim ();
                    tokens.push_back (cur);
                    cur.clear ();
                    }
                else
                    cur.append (&cmd[n], 1);
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
//static
void ECSqlConsole::WritePrompt()
    {
    Console::Write("ECSQL> ");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
//static
bool ECSqlConsole::StringEndsWith (Utf8StringCR stmt, Utf8Char ch, bool skipSpaces)
    {
    if (stmt.empty())
        return false;
    for(size_t i=stmt.size()-1; i>=0; i--)
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
//static
void ECSqlConsole::AddCommand (std::shared_ptr<ConsoleCommand> const& command)
    {
    AddCommand (command->GetName ().c_str (), command);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2013
//---------------------------------------------------------------------------------------
//static
void ECSqlConsole::AddCommand (Utf8CP commandName, std::shared_ptr<ConsoleCommand> const& command)
    {
    s_commands[commandName] = command;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2013
//---------------------------------------------------------------------------------------
//static
ConsoleCommand const* ECSqlConsole::GetCommand (Utf8CP commandName)
    {
    if (s_commands.find (commandName) == s_commands.end ())
        return nullptr;

    shared_ptr<ConsoleCommand> const& command = s_commands[commandName];
    return command.get ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle     09/2013
//---------------------------------------------------------------------------------------
//static
ECSqlConsoleSession& ECSqlConsole::GetSession()
    {
    return s_session;
    }



