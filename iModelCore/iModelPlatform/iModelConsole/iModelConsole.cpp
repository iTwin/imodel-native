/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "iModelConsole.h"
#include <Bentley/BeTimeUtilities.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DGN


void Session::ECIssueListener::_OnIssueReported(ECN::IssueSeverity severity, ECN::IssueCategory category, ECN::IssueType type, ECN::IssueId id, Utf8CP message) const
    {
    IModelConsole::WriteErrorLine("ISSUE: %s", message);
    m_issue = message;
    }
//******************** Session ***********************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool Session::IsFileLoaded(bool printMessageIfFalse) const
    {
    const bool isOpen = m_file != nullptr && m_file->IsOpen();

    if (!isOpen && printMessageIfFalse)
        IModelConsole::WriteErrorLine("No file loaded.");

    return isOpen;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool Session::IsECDbFileLoaded(bool printMessageIfFalse) const
    {
    if (!IsFileLoaded(printMessageIfFalse))
        return false;

    const bool isECDbFile = m_file->GetECDbHandle() != nullptr;
    if (!isECDbFile && printMessageIfFalse)
        IModelConsole::WriteErrorLine("Command requires iModel or ECDb, but currently opened file is only a BeSQLite file.");

    return isECDbFile;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
//static
Utf8CP SessionFile::TypeToString(Type type)
    {
    switch (type)
        {
            case Type::IModel:
                return "iModel";
            case Type::ECDb:
                return "ECDb";
            case Type::BeSQLite:
                return "BeSQLite";

            default:
                BeAssert(false);
                return "error";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool SessionFile::EnableTracking(bool enable)
    {
    if (m_changeTracker == nullptr)
        m_changeTracker = std::make_unique<IModelConsoleChangeTracker>(GetHandleR());

    return m_changeTracker->EnableTracking(enable);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool SessionFile::TryRetrieveProfileInfos(bmap<ProfileInfo::Type, ProfileInfo>& profileInfos) const
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(GetHandle(), "SELECT Namespace, StrData FROM be_Prop WHERE Name='SchemaVersion'"))
        {
        IModelConsole::WriteErrorLine("Could not execute SQL to retrieve profile versions.");
        return false;
        }

    while (BE_SQLITE_ROW == stmt.Step())
        {
        Utf8CP profileNamespace = stmt.GetValueText(0);

        ProfileInfo profileInfo;
        if (BeStringUtilities::StricmpAscii(profileNamespace, "be_Db") == 0)
            {
            profileInfo.m_type = ProfileInfo::Type::BeSQLite;
            profileInfo.m_name.assign("BeSQlite");
            }
        else if (BeStringUtilities::StricmpAscii(profileNamespace, "ec_Db") == 0)
            {
            profileInfo.m_type = ProfileInfo::Type::ECDb;
            profileInfo.m_name.assign("ECDb");
            }
        else if (BeStringUtilities::StricmpAscii(profileNamespace, "dgn_Db") == 0)
            {
            profileInfo.m_type = ProfileInfo::Type::IModel;
            profileInfo.m_name.assign("iModel");
            }
        else
            {
            profileInfo.m_type = ProfileInfo::Type::Unknown;
            profileInfo.m_name.assign(profileNamespace);
            }

        profileInfo.m_version = ProfileVersion(stmt.GetValueText(1));

        profileInfos.insert(bpair<ProfileInfo::Type, ProfileInfo>(profileInfo.m_type, profileInfo));
        }

    return !profileInfos.empty();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool SessionFile::IsAttached(Utf8StringCR tableSpaceName) const
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(GetHandle(), "pragma database_list"))
        {
        IModelConsole::WriteErrorLine("Could not execute SQL 'pragma database_list' to retrieve tablespaces.");
        return false;
        }

    while (BE_SQLITE_ROW == stmt.Step())
        {
        if (tableSpaceName.EqualsIAscii(stmt.GetValueText(1)))
            return true;
        }

    return false;
    }
//******************** IModelConsole ***********************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
IModelConsole* IModelConsole::s_singleton = new IModelConsole();

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void IModelConsole::Setup()
    {
    WriteLine(" --------------------------------------------------------------------------- ");
    WriteLine(" iModelConsole v1.0");
    WriteLine(" Copyright (c) Bentley Systems. All rights reserved. www.Bentley.com.");
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
    AddCommand(std::make_shared<AttachCommand>());
    AddCommand(std::make_shared<DetachCommand>());
    AddCommand(std::make_shared<ChangeCommand>());
    AddCommand(std::make_shared<ImportCommand>());
    AddCommand(std::make_shared<MergeCommand>());
    AddCommand(std::make_shared<ExportCommand>());
    AddCommand(std::make_shared<DropCommand>());
    AddCommand(std::make_shared<ParseCommand>());
    AddCommand(std::make_shared<MergeExternalCommand>());
    AddCommand(std::make_shared<CreateClassViewsCommand>());
    AddCommand(std::make_shared<SqliteCommand>());
    AddCommand(std::make_shared<JsonCommand>());
    AddCommand(std::make_shared<DbSchemaCommand>());
    AddCommand(std::make_shared<SchemaStatsCommand>());
    AddCommand(std::make_shared<DebugCommand>());
    AddCommand(std::make_shared<ExplainCommand>());
    AddCommand(std::make_shared<SyncCommand>());
    auto exitCommand = std::make_shared<ExitCommand>();
    AddCommand(exitCommand);
    AddCommand(".quit", exitCommand); //add same command with alternative command name
    AddCommand(".q", exitCommand); //add same command with alternative command name
    AddCommand(".bye", exitCommand); //add same command with alternative command name
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int IModelConsole::Run(int argc, WCharCP argv[])
    {
    //Initialize iModelConsole and print out banner
    Setup();

    if (argc > 1)
        {
        Command const* openCommand = GetCommand(".open");
        BeAssert(openCommand != nullptr);
        Utf8String argsUnparsed;
        //ignore first arg as it is the path to the iModelConsole exe
        for (size_t i = 1; i < (size_t) argc; i++)
            {
            if (i > 1)
                argsUnparsed.append(" ");

            argsUnparsed.append(Utf8String(argv[i]));
            }

        openCommand->Run(m_session, argsUnparsed);
        }

    return WaitForUserInput();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int IModelConsole::WaitForUserInput()
    {
    Utf8String cmd;

    //Run the Read-Execute command loop
    while (ReadLine(cmd))
        {
        cmd.Trim();
        if (!cmd.empty())
            RunCommand(cmd);
        }

    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
void IModelConsole::RunCommand(Utf8StringCR cmd)
    {
    bool isECSqlCommand = cmd[0] != COMMAND_PREFIX;

    Command const* command = nullptr;

    Utf8String args;
    if (isECSqlCommand)
        {
        args.assign(cmd);
        command = GetCommand(".ecsql");
        }
    else
        {
        Utf8String commandName;
        const size_t nextStartIndex = FindNextToken(commandName, WString(cmd.c_str(), BentleyCharEncoding::Utf8), 0, L' ', L'"');
        if (commandName.empty())
            {
            WriteErrorLine("Syntax error in command");
            return;
            }

        command = GetCommand(commandName);
        if (nextStartIndex < cmd.size())
            args.assign(cmd.substr(nextStartIndex));
        }

    //Unsupported command, use help command to display available commands
    if (command == nullptr)
        command = GetCommand(".help");

    BeAssert(command != nullptr);

    StopWatch executionTimer(true);
    command->Run(m_session, args);
    executionTimer.Stop();
    WriteLine();
    WriteLine("[Executed in %.4f s]", executionTimer.GetElapsedSeconds());
    WriteLine();

    //Add command to history
    AddToHistory(cmd);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool IModelConsole::ReadLine(Utf8StringR cmd)
    {
    WritePrompt();
    cmd.clear();
    while (true)
        {
        fgets(m_readBuffer, sizeof(m_readBuffer), GetIn());
        if (feof(GetIn()))
            return false;

        cmd.append(m_readBuffer);
        cmd.TrimEnd();
        if (!cmd.empty() && (cmd[0] == COMMAND_PREFIX || cmd.EndsWithIAscii(";")))
            return true;

        Write("   > ");
        cmd.append(" ");
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Command const* IModelConsole::GetCommand(Utf8StringCR commandName) const
    {
    auto it = m_commands.find(commandName);
    if (it == m_commands.end())
        return nullptr;

    return it->second.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
size_t IModelConsole::FindNextToken(Utf8String& token, WStringCR inputString, size_t startIndex, WChar delimiter, WChar delimiterEscapeChar)
    {
    enum class State
        {
        IsDelimiter,
        NotInEscapeSequence,
        InEscapeSequence,
        EscapeCharInEscapeSequence
        };

    const size_t inputStrLength = inputString.size();
    const bool doEscapeDelimiter = delimiterEscapeChar != L'\0';
    State state = State::NotInEscapeSequence;
    WString currentToken;
    for (size_t i = startIndex; i < inputStrLength; i++)
        {
        WChar c = inputString[i];
        switch (state)
            {
                case State::IsDelimiter:
                {
                if (c == delimiter)
                    break;

                currentToken.Trim();
                token = Utf8String(currentToken);
                return i;
                }
                case State::NotInEscapeSequence:
                {
                if (c == delimiter)
                    {
                    state = State::IsDelimiter;
                    break;
                    }

                if (doEscapeDelimiter && c == delimiterEscapeChar)
                    state = State::InEscapeSequence;
                else
                    currentToken.append(1, c);

                break;
                }

                case State::InEscapeSequence:
                {
                if (c == delimiterEscapeChar)
                    state = State::EscapeCharInEscapeSequence;
                else
                    currentToken.append(1, c);

                break;
                }

                case State::EscapeCharInEscapeSequence:
                {
                if (c == delimiterEscapeChar)
                    {
                    //two subsequent double-quotes in an escape sequence mean to escape the escape char
                    currentToken.append(1, c);
                    state = State::InEscapeSequence;
                    }
                else if (c == delimiter)
                    state = State::IsDelimiter;
                else
                    {
                    currentToken.append(1, c);
                    state = State::NotInEscapeSequence;
                    }

                break;
                }

                default:
                    BeAssert(false);
                    return Utf8String::npos;
            }
        }

    currentToken.Trim();
    token = Utf8String(currentToken);
    return inputStrLength;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
FILE* IModelConsole::GetIn() { return stdin; }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
FILE* IModelConsole::GetOut() { return stdout; }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
FILE* IModelConsole::GetErr() { return stderr; }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void IModelConsole::Write(Utf8CP format, ...)
    {
    va_list args;
    va_start(args, format);
    Write(GetOut(), format, args);
    va_end(args);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void IModelConsole::WriteLine(Utf8CP format, ...)
    {
    va_list args;
    va_start(args, format);
    Write(GetOut(), format, args);
    va_end(args);
    WriteLine();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void IModelConsole::WriteLine() { Write("%s", "\r\n"); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void IModelConsole::WriteError(Utf8CP format, ...)
    {
    va_list args;
    va_start(args, format);
    Write(GetErr(), format, args);
    va_end(args);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void IModelConsole::WriteErrorLine(Utf8CP format, ...)
    {
    va_list args;
    va_start(args, format);
    Write(GetErr(), format, args);
    va_end(args);
    WriteLine();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void IModelConsole::Write(FILE* stream, Utf8CP format, va_list args) { vfprintf(stream, format, args); }
