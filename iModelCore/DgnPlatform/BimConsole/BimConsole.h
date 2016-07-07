/*--------------------------------------------------------------------------------------+
|
|     $Source: BimConsole/BimConsole.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DesktopTools/WindowsKnownLocationsAdmin.h>

#include "Command.h"

USING_NAMESPACE_BENTLEY
//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle     07/2016
//---------------------------------------------------------------------------------------
struct SessionFile
    {
    public:
        enum class Type
            {
            Bim,
            ECDb
            };

    private:
        Type m_type;

        virtual bool _IsOpen() const = 0;
        virtual BeSQLite::EC::ECDb& _GetHandle() const = 0;

    protected:
        explicit SessionFile(Type type) : m_type(type) {}

    public:
        virtual ~SessionFile() {}

        bool IsOpen() const { return _IsOpen(); }
        BeSQLite::EC::ECDb const& GetHandle() const { return _GetHandle(); }
        BeSQLite::EC::ECDb& GetHandleR() const { return _GetHandle(); }
        Utf8CP GetPath() const { return IsOpen() ? GetHandle().GetDbFileName() : nullptr; }
        Utf8CP TypeToString() const { return TypeToString(m_type); }
        static Utf8CP TypeToString(Type type) { return type == Type::Bim ? "BIM" : "ECDb"; }
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle     07/2016
//---------------------------------------------------------------------------------------
struct BimFile : SessionFile
    {
    private:
        Dgn::DgnDbPtr m_file;

        virtual bool _IsOpen() const override { return m_file != nullptr && m_file->IsDbOpen(); }
        virtual BeSQLite::EC::ECDb& _GetHandle() const override { BeAssert(IsOpen()); return *m_file; }

    public:
        explicit BimFile(Dgn::DgnDbPtr bim) : SessionFile(Type::Bim), m_file(bim) {}
        ~BimFile() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle     07/2016
//---------------------------------------------------------------------------------------
struct ECDbFile : SessionFile
    {
    private:
        mutable BeSQLite::EC::ECDb m_file;

        virtual bool _IsOpen() const override { return m_file.IsDbOpen(); }
        virtual BeSQLite::EC::ECDb& _GetHandle() const override { return m_file; }

    public:
        ECDbFile() : SessionFile(Type::ECDb) {}
        ~ECDbFile() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
struct Session
    {
    struct ECDbIssueListener : BeSQLite::EC::ECDb::IIssueListener
        {
        private:
            mutable BeSQLite::EC::ECDbIssueSeverity m_severity;
            mutable Utf8String m_issue;

            virtual void _OnIssueReported(BeSQLite::EC::ECDbIssueSeverity severity, Utf8CP message) const override
                {
                m_severity = severity;
                m_issue = message;
                }

        public:
            ECDbIssueListener() : BeSQLite::EC::ECDb::IIssueListener() {}

            void Reset() const { m_issue.clear(); }
            bool HasIssue() const { return !m_issue.empty(); }
            BeSQLite::EC::ECDbIssueSeverity GetSeverity() const { return m_severity; }
            Utf8CP GetIssue() const { return m_issue.c_str(); }
        };

    private:
        std::unique_ptr<SessionFile> m_file;
        ECDbIssueListener m_issueListener;

    public:
        Session() : m_file(nullptr) {}

        bool IsFileLoaded(bool printMessageIfFalse = false) const;
        SessionFile const& GetFile() const { BeAssert(IsFileLoaded()); return *m_file; }
        BentleyStatus SetFile(std::unique_ptr<SessionFile>);
        ECDbIssueListener const& GetIssues() const { return m_issueListener; }
    };


//=======================================================================================
// @bsiclass                                    BentleySystems 
//=======================================================================================
struct BimConsole : Dgn::DgnPlatformLib::Host
    {
    private:
        const static Utf8Char ECSQLSTATEMENT_DELIMITER = ';';
        const static Utf8Char COMMAND_PREFIX = '.';

        Session m_session;
        char m_readBuffer[5000];
        std::vector<Utf8String> m_commandHistory;
        std::map<Utf8String, std::shared_ptr<Command>> m_commands;

        virtual void _SupplyProductName(Utf8StringR name) override { name.assign("BimConsole"); }
        virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new Dgn::WindowsKnownLocationsAdmin(); }
        virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override;

        void Setup();
        void AddCommand(Utf8CP commandName, std::shared_ptr<Command> const&);
        void AddCommand(std::shared_ptr<Command> const& command);
        Command const* GetCommand(Utf8CP commandName) const;

        void WritePrompt();
        int WaitForUserInput(int argc, WCharP argv[]);
        void RunCommand(Utf8CP cmd);
        bool TokenizeCommandline(std::vector<Utf8String>& tokens, Utf8StringCR cmd);
        bool ReadLine(Utf8StringR stmt);

        bool StringEndsWith(Utf8StringCR stmt, Utf8Char ch, bool skipSpaces);
        void AddToHistory(Utf8CP command) { return m_commandHistory.push_back(command); }
        std::vector<Utf8String> const& GetCommandHistory() const { return m_commandHistory; }

    public:
        BimConsole() {}
        int Run(int argc, WCharP argv[]);
    };


//=======================================================================================
// @bsiclass                                    BentleySystems 
//=======================================================================================
struct Console
    {
    private:
        static void Write(FILE* stream, Utf8CP format, va_list args);

        static FILE* GetOut();
        static FILE* GetErr();

    public:
        static FILE* GetIn();
        static void Write(Utf8CP format, ...);
        static void WriteLine(Utf8CP format, ...);
        static void WriteError(Utf8CP format, ...);
        static void WriteErrorLine(Utf8CP format, ...);
        static void WriteLine();
    };


