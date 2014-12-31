/*--------------------------------------------------------------------------------------+
|
|     $Source: ECSqlConsole/ECSqlConsole.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <BeSQLite/ECDb/ECDbApi.h>
#include <Bentley/NonCopyableClass.h>
#include "ConsoleCommand.h"

USING_NAMESPACE_BENTLEY

#define ECSQLCONSOLE_MAJ_VER 1
#define ECSQLCONSOLE_MIN_VER 0

enum class OutputFormat
    {
    Table,
    List
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
struct ECSqlConsoleSession : NonCopyableClass
    {
private:
    BeSQLite::EC::ECDb m_ecdb;
    BeFileName m_ecdbPath;
    OutputFormat m_outputFormat;
    std::vector<Utf8String> m_commandHistory;

public:
    ECSqlConsoleSession ();

    bool HasECDb (bool printMessageIfFalse) const;

    BeSQLite::EC::ECDb& GetECDbR () { return m_ecdb; }
    BeSQLite::EC::ECDb const& GetECDb () { return m_ecdb; }
    Utf8CP GetECDbPath () const;

    OutputFormat GetOutputFormat () const {return m_outputFormat;}
    void SetOutputFormat (OutputFormat newFormat) {m_outputFormat = newFormat;}

    void AddToHistory (Utf8CP command);
    std::vector<Utf8String> const& GetCommandHistory () const {return m_commandHistory;}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
struct ECSqlConsole
    {
private:
    const static size_t MaxReadBufferLineSize = 5000;
    const static Utf8Char ECSqlStatementDelimiterChar = ';';
    const static Utf8Char CommandPrefix = '.';

    static ECSqlConsoleSession s_session;
    static ConsoleCommandMap s_commands;
    static char s_readBuffer [MaxReadBufferLineSize];

    ECSqlConsole ();
    ~ECSqlConsole ();

    static void Setup();
    static void AddCommand (std::shared_ptr<ConsoleCommand> const& command);
    static void AddCommand (Utf8CP commandName, std::shared_ptr<ConsoleCommand> const& command);
    static ConsoleCommand const* GetCommand (Utf8CP commandName);

    static int WaitForUserInput(int argc, WCharP argv[]);

    static void RunCommand (Utf8CP cmd);

    static bool TokenizeCommandline (std::vector<Utf8String>& tokens, Utf8StringCR cmd);


    static void WritePrompt();
    static bool ReadLine (Utf8StringR stmt);

    static bool StringEndsWith(Utf8StringCR stmt, Utf8Char ch, bool skipSpaces);

    static ECSqlConsoleSession& GetSession ();

public:
    static int Run(int argc, WCharP argv[]);
    };
