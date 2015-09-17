/*--------------------------------------------------------------------------------------+
|
|     $Source: ECSqlConsole/ECSqlConsole.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDbApi.h>
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
public:
    struct ECDbIssueListener : BeSQLite::EC::ECDb::IIssueListener
        {
    private:
        mutable BeSQLite::EC::ECDb::IssueSeverity m_severity;
        mutable Utf8String m_issue;
        
        virtual void _OnIssueReported(BeSQLite::EC::ECDb::IssueSeverity severity, Utf8CP message) const override;

    public:
        ECDbIssueListener() : BeSQLite::EC::ECDb::IIssueListener() {}

        void Reset() const { m_issue.clear(); }
        bool HasIssue() const { return !m_issue.empty(); }
        BeSQLite::EC::ECDb::IssueSeverity GetSeverity() const { return m_severity; }
        Utf8CP GetIssue() const { return m_issue.c_str(); }
        };

private:
    BeSQLite::EC::ECDb m_ecdb;
    ECDbIssueListener m_issueListener;
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

    ECDbIssueListener const& GetIssues() const { return m_issueListener; }

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
