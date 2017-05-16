/*--------------------------------------------------------------------------------------+
|
|     $Source: BimConsole/BimConsole.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>
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
            ECDb,
            BeSQLite
            };

        struct ProfileInfo final
            {
            enum class Type
                {
                BeSQLite,
                ECDb,
                DgnDb,
                Unknown
                };

            Type m_type = Type::Unknown;
            Utf8String m_name;
            BeSQLite::ProfileVersion m_version;

            ProfileInfo() : m_version(BeSQLite::ProfileVersion(0, 0, 0, 0)) {}
            };

    private:
        Type m_type;

        virtual BeSQLite::EC::ECDb* _GetECDbHandle() const = 0;
        virtual BeSQLite::Db& _GetBeSqliteHandle() const { BeAssert(_GetECDbHandle() != nullptr); return *_GetECDbHandle(); }

    protected:
        explicit SessionFile(Type type) : m_type(type) {}

    public:
        virtual ~SessionFile() {}

        template<typename TSessionFile>
        TSessionFile const& GetAs() const
            {
            BeAssert(dynamic_cast<TSessionFile const*> (this) != nullptr);
            return static_cast<TSessionFile const&> (*this);
            }

        bool IsOpen() const { return GetHandle().IsDbOpen(); }
        Utf8CP GetPath() const { BeAssert(IsOpen()); return GetHandle().GetDbFileName(); }

        BeSQLite::EC::ECDb const* GetECDbHandle() const { return _GetECDbHandle(); }
        BeSQLite::EC::ECDb* GetECDbHandleP() const { return _GetECDbHandle(); }
        BeSQLite::Db const& GetHandle() const { return _GetBeSqliteHandle(); }
        BeSQLite::Db& GetHandleR() const { return _GetBeSqliteHandle(); }

        bool TryRetrieveProfileInfos(bmap<ProfileInfo::Type, ProfileInfo>&) const;

        Type GetType() const { return m_type; }
        Utf8CP TypeToString() const { return TypeToString(m_type); }
        static Utf8CP TypeToString(Type type);

    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle     07/2016
//---------------------------------------------------------------------------------------
struct BimFile final : SessionFile
    {
    private:
        mutable Dgn::DgnDbPtr m_file;

        BeSQLite::EC::ECDb* _GetECDbHandle() const override { BeAssert(m_file != nullptr); return m_file.get(); }

    public:
        explicit BimFile(Dgn::DgnDbPtr bim) : SessionFile(Type::Bim), m_file(bim) {}
        ~BimFile() {}
        Dgn::DgnDbCR GetDgnDbHandle() const { BeAssert(IsOpen()); return *m_file; }
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle     07/2016
//---------------------------------------------------------------------------------------
struct ECDbFile final : SessionFile
    {
    private:
        mutable BeSQLite::EC::ECDb m_file;

        BeSQLite::EC::ECDb* _GetECDbHandle() const override { return &m_file; }

    public:
        ECDbFile() : SessionFile(Type::ECDb) {}
        ~ECDbFile() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle     01/2017
//---------------------------------------------------------------------------------------
struct BeSQLiteFile final : SessionFile
    {
    private:
        mutable BeSQLite::Db m_file;

        BeSQLite::Db& _GetBeSqliteHandle() const override { return m_file; }
        BeSQLite::EC::ECDb* _GetECDbHandle() const override { return nullptr; }

    public:
        BeSQLiteFile() : SessionFile(Type::BeSQLite) {}
        ~BeSQLiteFile() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
struct Session final
    {
    struct ECDbIssueListener : BeSQLite::EC::ECDb::IIssueListener
        {
        private:
            mutable Utf8String m_issue;

            void _OnIssueReported(Utf8CP message) const override { m_issue = message; }

        public:
            ECDbIssueListener() : BeSQLite::EC::ECDb::IIssueListener() {}

            void Reset() const { m_issue.clear(); }
            bool HasIssue() const { return !m_issue.empty(); }
            Utf8CP GetIssue() const { return m_issue.c_str(); }
        };

    private:
        std::unique_ptr<SessionFile> m_file;
        ECDbIssueListener m_issueListener;

    public:
        Session() : m_file(nullptr) {}

        bool IsFileLoaded(bool printMessageIfFalse = false) const;
        bool IsECDbFileLoaded(bool printMessageIfFalse = false) const;
        SessionFile const& GetFile() const { BeAssert(IsFileLoaded()); return *m_file; }
        BentleyStatus SetFile(std::unique_ptr<SessionFile>);
        ECDbIssueListener const& GetIssues() const { return m_issueListener; }
    };


//=======================================================================================
// @bsiclass                                    BentleySystems 
//=======================================================================================
struct BimConsole final : Dgn::DgnPlatformLib::Host
    {
    private:
        static const Utf8Char COMMAND_PREFIX = '.';

        Session m_session;
        Utf8Char m_readBuffer[5000];
        std::vector<Utf8String> m_commandHistory;
        std::map<Utf8String, std::shared_ptr<Command>> m_commands;

        void _SupplyProductName(Utf8StringR name) override { name.assign("BimConsole"); }
        IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new Dgn::KnownDesktopLocationsAdmin; }
        BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override;

        void Setup();
        void AddCommand(Utf8StringCR commandName, std::shared_ptr<Command> const& command) { m_commands[commandName] = command; }
        void AddCommand(std::shared_ptr<Command> const& command) { AddCommand(command->GetName(), command); }
        Command const* GetCommand(Utf8StringCR commandName) const;

        int WaitForUserInput();
        void RunCommand(Utf8StringCR cmd);
        bool ReadLine(Utf8StringR stmt);


        void AddToHistory(Utf8StringCR command) { return m_commandHistory.push_back(command); }
        std::vector<Utf8String> const& GetCommandHistory() const { return m_commandHistory; }

        static void WritePrompt() { Write("BIM> "); }
        static void Write(FILE* stream, Utf8CP format, va_list args);
        static FILE* GetIn();
        static FILE* GetOut();
        static FILE* GetErr();


    public:
        BimConsole() {}
        int Run(int argc, WCharCP argv[]);

        static size_t FindNextToken(Utf8String& token, WStringCR inputString, size_t startIndex, WChar delimiter, WChar delimiterEscapeChar = L'\0');

        static void Write(Utf8CP format, ...);
        static void WriteLine(Utf8CP format, ...);
        static void WriteError(Utf8CP format, ...);
        static void WriteErrorLine(Utf8CP format, ...);
        static void WriteLine();
    };



