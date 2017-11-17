/*--------------------------------------------------------------------------------------+
|
|     $Source: BimConsole/BimConsole.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BimConsole.h"
#include <Bentley/BeTimeUtilities.h>

USING_NAMESPACE_BENTLEY_SQLITE
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle    03/2017
//---------------------------------------------------------------------------------------
bool SessionFile::TryRetrieveProfileInfos(bmap<ProfileInfo::Type, ProfileInfo>& profileInfos) const
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(GetHandle(), "SELECT Namespace, StrData FROM be_Prop WHERE Name='SchemaVersion'"))
        {
        BimConsole::WriteErrorLine("Could not execute SQL to retrieve profile versions.");
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
            profileInfo.m_type = ProfileInfo::Type::DgnDb;
            profileInfo.m_name.assign("DgnDb");
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
    AddCommand(std::make_shared<CreateClassViewsCommand>());
    AddCommand(std::make_shared<SqliteCommand>());
    AddCommand(std::make_shared<JsonCommand>());
    AddCommand(std::make_shared<DbSchemaCommand>());
    AddCommand(std::make_shared<SchemaStatsCommand>());
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
int BimConsole::Run(int argc, WCharCP argv[])
    {
    //Initialize BimConsole and Print out banner 
    Setup();

    if (argc > 1)
        {
        Command const* openCommand = GetCommand(".open");
        BeAssert(openCommand != nullptr);
        Utf8String argsUnparsed;
        //ignore first arg as it is the path to the bimconsole exe
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
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
int BimConsole::WaitForUserInput()
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
// @bsimethod                                                  Krischan.eberle     09/2013
//---------------------------------------------------------------------------------------
//static
void BimConsole::RunCommand(Utf8StringCR cmd)
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
        cmd.TrimEnd();
        if (!cmd.empty() && (cmd[0] == COMMAND_PREFIX || cmd.EndsWithIAscii(";")))
            return true;

        Write("   > ");
        cmd.append(" ");
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
// @bsimethod                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
//static
size_t BimConsole::FindNextToken(Utf8String& token, WStringCR inputString, size_t startIndex, WChar delimiter, WChar delimiterEscapeChar)
    {
#ifdef COMMENT_OUT_UNUSED_VARIABLE
    auto postProcessToken = [&token] (WStringR tokenW, size_t currentIndex)
        {
        tokenW.Trim();
        token.Assign(tokenW.c_str());
        return currentIndex + 1;
        };
#endif

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

