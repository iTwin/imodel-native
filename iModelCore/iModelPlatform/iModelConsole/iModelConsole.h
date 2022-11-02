/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/PlatformLib.h>
#include <Bentley/PerformanceLogger.h>
#include <Bentley/Desktop/FileSystem.h>
#include "Command.h"

BEGIN_BENTLEY_DGN_NAMESPACE

/*=================================================================================**//**
* An implementation of IKnownLocationsAdmin that is useful for desktop applications.
* This implementation works for Windows, Linux, and MacOS.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct KnownDesktopLocationsAdmin : PlatformLib::Host::IKnownLocationsAdmin
{
    BeFileName m_tempDirectory;
    BeFileName m_executableDirectory;
    BeFileName m_assetsDirectory;

    BeFileNameCR _GetLocalTempDirectoryBaseName() override {return m_tempDirectory;}
    BeFileNameCR _GetDgnPlatformAssetsDirectory() override {return m_assetsDirectory;}

    //! Construct an instance of the KnownDesktopLocationsAdmin
    KnownDesktopLocationsAdmin()
        {
        Desktop::FileSystem::BeGetTempPath(m_tempDirectory);
        m_executableDirectory = Desktop::FileSystem::GetExecutableDir();
        m_assetsDirectory = m_executableDirectory;
        m_assetsDirectory.AppendToPath(L"Assets");
        }
};

END_BENTLEY_DGN_NAMESPACE

USING_NAMESPACE_BENTLEY

//---------------------------------------------------------------------------------------
// @bsiclass
//---------------------------------------------------------------------------------------
struct IModelConsoleChangeTracker final : BeSQLite::ChangeTracker
    {
    explicit IModelConsoleChangeTracker(BeSQLite::DbR db) : BeSQLite::ChangeTracker() { SetDb(&db); }
    OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override { return OnCommitStatus::Commit; }
    };

//---------------------------------------------------------------------------------------
// @bsiclass
//---------------------------------------------------------------------------------------
struct IModelConsoleChangeSet final : BeSQLite::ChangeSet
    {
    IModelConsoleChangeSet() : BeSQLite::ChangeSet() {}
    ConflictResolution _OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) override { BeAssert(false && "Unexpected conflict"); return ConflictResolution::Skip; }
    };

//---------------------------------------------------------------------------------------
// @bsiclass
//---------------------------------------------------------------------------------------
struct SessionFile
    {
    public:
        enum class Type
            {
            IModel,
            ECDb,
            BeSQLite
            };

        struct ProfileInfo final
            {
            enum class Type
                {
                BeSQLite,
                ECDb,
                IModel,
                Unknown
                };

            Type m_type = Type::Unknown;
            Utf8String m_name;
            BeSQLite::ProfileVersion m_version;

            ProfileInfo() : m_version(BeSQLite::ProfileVersion(0, 0, 0, 0)) {}
            };

    private:
        Type m_type;
        std::unique_ptr<IModelConsoleChangeTracker> m_changeTracker;

        virtual BeSQLite::EC::ECDb* _GetECDbHandle() const = 0;
        virtual BeSQLite::Db& _GetBeSqliteHandle() const { BeAssert(_GetECDbHandle() != nullptr); return *_GetECDbHandle(); }

    protected:
        explicit SessionFile(Type type) : m_type(type) {}

        void Finalize()
            {
            if (m_changeTracker != nullptr)
                m_changeTracker->EndTracking();
            }
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
        bool IsAttached(Utf8StringCR tableSpaceName) const;
        bool EnableTracking(bool enable);
        bool IsTracking() const { return m_changeTracker != nullptr && m_changeTracker->IsTracking(); }
        IModelConsoleChangeTracker* GetTracker() const { return m_changeTracker.get(); }
        Type GetType() const { return m_type; }
        Utf8CP TypeToString() const { return TypeToString(m_type); }
        static Utf8CP TypeToString(Type type);

    };

//---------------------------------------------------------------------------------------
// @bsiclass
//---------------------------------------------------------------------------------------
struct IModelFile final : SessionFile
    {
    private:
        mutable Dgn::DgnDbPtr m_file;

        BeSQLite::EC::ECDb* _GetECDbHandle() const override { BeAssert(m_file != nullptr); return m_file.get(); }

    public:
        explicit IModelFile(Dgn::DgnDbPtr iModel) : SessionFile(Type::IModel), m_file(iModel) {}
        ~IModelFile() { Finalize(); }
        Dgn::DgnDbCR GetDgnDbHandle() const { BeAssert(IsOpen()); return *m_file; }
        Dgn::DgnDbR GetDgnDbHandleR() const { BeAssert(IsOpen()); return *m_file; }
    };

//---------------------------------------------------------------------------------------
// @bsiclass
//---------------------------------------------------------------------------------------
struct ECDbFile final : SessionFile
    {
    private:
        mutable BeSQLite::EC::ECDb m_file;

        BeSQLite::EC::ECDb* _GetECDbHandle() const override { return &m_file; }

    public:
        ECDbFile() : SessionFile(Type::ECDb) {}
        ~ECDbFile() { Finalize(); }
    };

//---------------------------------------------------------------------------------------
// @bsiclass
//---------------------------------------------------------------------------------------
struct BeSQLiteFile final : SessionFile
    {
    private:
        mutable BeSQLite::Db m_file;

        BeSQLite::Db& _GetBeSqliteHandle() const override { return m_file; }
        BeSQLite::EC::ECDb* _GetECDbHandle() const override { return nullptr; }

    public:
        BeSQLiteFile() : SessionFile(Type::BeSQLite) {}
        ~BeSQLiteFile() { Finalize(); }
    };

//---------------------------------------------------------------------------------------
// @bsiclass
//---------------------------------------------------------------------------------------
struct Session final
    {
    struct ECIssueListener : ECN::IIssueListener
        {
        private:
            mutable Utf8String m_issue;

            void _OnIssueReported(ECN::IssueSeverity severity, ECN::IssueCategory category, ECN::IssueType type, Utf8CP message) const override { m_issue = message; }

        public:
            ECIssueListener() : ECN::IIssueListener() {}

            void Reset() const { m_issue.clear(); }
            bool HasIssue() const { return !m_issue.empty(); }
            Utf8CP GetIssue() const { return m_issue.c_str(); }
        };

    private:
        std::unique_ptr<SessionFile> m_file;
        ECIssueListener m_issueListener;

    public:
        Session() {}

        bool IsFileLoaded(bool printMessageIfFalse = false) const;
        bool IsECDbFileLoaded(bool printMessageIfFalse = false) const;
        SessionFile const& GetFile() const { BeAssert(IsFileLoaded()); return *m_file; }
        SessionFile& GetFileR() { BeAssert(IsFileLoaded()); return *m_file; }
        BentleyStatus SetFile(std::unique_ptr<SessionFile>);
        void Reset() { m_file = nullptr; m_issueListener.Reset(); }
        ECIssueListener const& GetIssues() const { return m_issueListener; }
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct IModelConsole final : Dgn::PlatformLib::Host
    {
    private:
        static const Utf8Char COMMAND_PREFIX = '.';

        static IModelConsole* s_singleton;

        Session m_session;
        Utf8Char m_readBuffer[5000];
        std::vector<Utf8String> m_commandHistory;
        std::map<Utf8String, std::shared_ptr<Command>> m_commands;

        IModelConsole(): Dgn::PlatformLib::Host() {}

        IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new Dgn::KnownDesktopLocationsAdmin; }

        void Setup();
        void AddCommand(Utf8StringCR commandName, std::shared_ptr<Command> const& command) { m_commands[commandName] = command; }
        void AddCommand(std::shared_ptr<Command> const& command) { AddCommand(command->GetName(), command); }
        Command const* GetCommand(Utf8StringCR commandName) const;

        int WaitForUserInput();
        void RunCommand(Utf8StringCR cmd);
        bool ReadLine(Utf8StringR stmt);

        void AddToHistory(Utf8StringCR command) { return m_commandHistory.push_back(command); }
        std::vector<Utf8String> const& GetCommandHistory() const { return m_commandHistory; }

        static void WritePrompt() { Write("iModelConsole> "); }
        static void Write(FILE* stream, Utf8CP format, va_list args);
        static FILE* GetIn();
        static FILE* GetOut();
        static FILE* GetErr();

    public:
        static IModelConsole& Singleton() { return *s_singleton; }

        int Run(int argc, WCharCP argv[]);

        static size_t FindNextToken(Utf8String& token, WStringCR inputString, size_t startIndex, WChar delimiter, WChar delimiterEscapeChar = L'\0');

        static void Write(Utf8CP format, ...);
        static void WriteLine(Utf8CP format, ...);
        static void WriteError(Utf8CP format, ...);
        static void WriteErrorLine(Utf8CP format, ...);
        static void WriteLine();

        static Utf8String FormatId(BeInt64Id id) { return id.IsValid()? id.ToHexStr() : Utf8String(); }
    };



