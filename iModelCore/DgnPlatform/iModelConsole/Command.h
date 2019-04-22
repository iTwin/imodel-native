/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDbApi.h>
#include <vector>
#include <map>
#include <Bentley/BeTextFile.h>

USING_NAMESPACE_BENTLEY

struct Session;

#define COMMAND_USAGE_IDENT "                                "

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct Command
    {
    private:
        virtual Utf8String _GetName() const = 0;
        virtual Utf8String _GetUsage() const = 0;
        virtual void _Run(Session&, Utf8StringCR args) const = 0;

    protected:
        Command() {}

        static BentleyStatus TokenizeString(std::vector<Utf8String>& tokens, WStringCR inputString, WChar delimiter, WChar delimiterEscapeChar = L'\0');
        
        std::vector<Utf8String> TokenizeArgs(Utf8StringCR argsUnparsed) const { std::vector<Utf8String> tokens; TokenizeString(tokens, WString(argsUnparsed.c_str(), BentleyCharEncoding::Utf8), L' ', L'"'); return tokens; }

        static bool GetArgAsBool(Utf8StringCR arg) { return arg.EqualsIAscii("true") || arg.EqualsIAscii("1") || arg.EqualsIAscii("yes"); }

    public:
        virtual ~Command() {}

        Utf8String GetName() const { return _GetName(); }
        Utf8String GetUsage() const { return _GetUsage(); }
        void Run(Session& session, Utf8StringCR args) const;
    };


//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct HelpCommand final : public Command
    {
    private:
        std::map<Utf8String, std::shared_ptr<Command>> const& m_commandMap;

        Utf8String _GetName() const override { return ".help"; }
        Utf8String _GetUsage() const override { return " .help, .h                      Displays all available commands"; }
        void _Run(Session&, Utf8StringCR args) const override;

    public:
        explicit HelpCommand(std::map<Utf8String, std::shared_ptr<Command>> const& commandMap)
            : Command(), m_commandMap(commandMap) {}

        ~HelpCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct OpenCommand final : public Command
    {
    private:
        Utf8String _GetName() const override { return ".open"; }
        Utf8String _GetUsage() const override;
        void _Run(Session&, Utf8StringCR args) const override;

    public:
        OpenCommand() : Command() {}
        ~OpenCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct CloseCommand final : public Command
    {
    private:
        Utf8String _GetName() const override { return ".close"; }
        Utf8String _GetUsage() const override { return " .close                         Closes the currently open file"; }

        void _Run(Session&, Utf8StringCR args) const override;

    public:
        CloseCommand() : Command() {}
        ~CloseCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct CreateCommand final : public Command
    {
    private:
        Utf8String _GetName() const override { return ".create"; }
        Utf8String _GetUsage() const override;
        void _Run(Session&, Utf8StringCR args) const override;

    public:
        CreateCommand() : Command() {}
        ~CreateCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct FileInfoCommand final : public Command
    {
    private:
        

        Utf8String _GetName() const override { return ".fileinfo"; }
        Utf8String _GetUsage() const override { return " .fileinfo                      Displays information about the open file"; }

        void _Run(Session&, Utf8StringCR args) const override;

    public:
        FileInfoCommand() : Command() {}
        ~FileInfoCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct ImportCommand final : public Command
    {
    private:
        static Utf8CP const ECSCHEMA_SWITCH;
        static Utf8CP const CSV_SWITCH;

        Utf8String _GetName() const override { return ".import"; }
        Utf8String _GetUsage() const override;
        void _Run(Session&, Utf8StringCR args) const override;

        void RunImportSchema(Session&, std::vector<Utf8String> const& args) const;
        static BentleyStatus DeserializeECSchema(ECN::ECSchemaReadContextR readContext, BeFileNameCR ecschemaFilePath);

        void RunImportCsv(Session&, std::vector<Utf8String> const& args) const;
        BentleyStatus SetupCsvImport(Session&, BeSQLite::Statement& insertStmt, Utf8StringCR tableName, uint32_t columnCount, std::vector<Utf8String> const* header) const;
        BentleyStatus InsertCsvRow(Session&, BeSQLite::Statement&, int columnCount, std::vector<Utf8String> const& tokens, int rowNumber) const;

    public:
        ImportCommand() : Command() {}
        ~ImportCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct ExportCommand final : public Command
    {
    private:
        static constexpr Utf8CP s_schemaSwitch = "schema";
        static constexpr Utf8CP s_tablesSwitch = "tables";
        static constexpr Utf8CP s_changeSummarySwitch = "changesummary";

        struct ChangeSummaryExportContext final
            {
            struct Timer final
                {
            private:
                ChangeSummaryExportContext& m_ctx;
                Utf8String m_message;
                Utf8CP m_ecsql = nullptr;
                Utf8CP m_nativeSql = nullptr;
                uint64_t m_startTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
                bool m_isDiposed = false;

            public:
                Timer(ChangeSummaryExportContext& ctx, Utf8StringCR message, Utf8CP ecsql = nullptr, Utf8CP nativeSql = nullptr) : m_ctx(ctx), m_message(message), m_ecsql(ecsql), m_nativeSql(nativeSql) {}
                ~Timer() { Dispose(); }
                void Dispose();
                };

            BeSQLite::EC::ECDbCR m_ecdb;
            BeSQLite::EC::ECInstanceId m_summaryId;
            Utf8String m_summaryIdString;
            BeSQLite::EC::ECSqlStatement m_accessStringStmt;
            BeSQLite::EC::ECSqlStatementCache m_changedInstanceStmtCache;
            BeFileName m_jsonOutputPath;

            Json::Value m_outputJson = Json::Value(Json::arrayValue);
            BeFile m_outputFile;
            BeFile m_diagnosticsFile;

            ChangeSummaryExportContext(BeSQLite::EC::ECDbCR ecdb, BeSQLite::EC::ECInstanceId summaryId) : m_ecdb(ecdb), m_summaryId(summaryId), m_summaryIdString(summaryId.ToHexStr()), m_changedInstanceStmtCache(50) {}
            BentleyStatus InitializeOutput(Utf8StringCR jsonOutputPath);
            BentleyStatus WriteOutput();
            };

        Utf8String _GetName() const override { return ".export"; }
        Utf8String _GetUsage() const override;
        void _Run(Session&, Utf8StringCR args) const override;

        void RunExportSchema(Session&, Utf8StringCR outFolder, bool useECXmlV2) const;
        void RunExportTables(Session&, Utf8StringCR jsonFile) const;
        void ExportTable(Session&, Json::Value& out, Utf8CP tableName) const;
        void RunExportChangeSummary(Session&, BeSQLite::EC::ECInstanceId changeSummaryId, Utf8StringCR jsonFile) const;
        BentleyStatus PropertyValueChangesToJson(Json::Value& propValueJson, ChangeSummaryExportContext&, BeSQLite::EC::ECInstanceId instanceChangeId, BeSQLite::EC::ECInstanceId changedInstanceId, Utf8StringCR changedInstanceClassName, BeSQLite::EC::ChangedValueState) const;

        static Utf8CP ToString(BeSQLite::EC::ChangedValueState);

    public:
        ExportCommand() : Command() {}
        ~ExportCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    12/2015
//---------------------------------------------------------------------------------------
struct CreateClassViewsCommand final : public Command
    {
    private:
        Utf8String _GetName() const override { return ".createclassviews"; }
        Utf8String _GetUsage() const override;
        void _Run(Session&, Utf8StringCR args) const override;
    public:
        CreateClassViewsCommand() : Command() {}
        ~CreateClassViewsCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct ECSqlCommand final : public Command
    {
    private:
        Utf8String _GetName() const override { return ".ecsql"; }
        Utf8String _GetUsage() const override;
        void _Run(Session&, Utf8StringCR args) const override;

        void ExecuteSelect(Session&, BeSQLite::EC::ECSqlStatement&) const;
        void ExecuteInsert(Session&, BeSQLite::EC::ECSqlStatement&) const;
        void ExecuteUpdateOrDelete(Session&, BeSQLite::EC::ECSqlStatement&) const;

        static int ComputeColumnSize(BeSQLite::EC::ECSqlColumnInfo const&);
        static Utf8String ValueToString(BeSQLite::EC::IECSqlValue const&);
        static Utf8String PrimitiveToString(BeSQLite::EC::IECSqlValue const&, ECN::PrimitiveType);
        static Utf8String PrimitiveToString(BeSQLite::EC::IECSqlValue const&, ECN::ECEnumerationCR);
        static Utf8String ArrayToString(BeSQLite::EC::IECSqlValue const& arrayValue);
        static Utf8String StructToString(BeSQLite::EC::IECSqlValue const& structValue);

    public:
        ECSqlCommand() : Command() {}
        ~ECSqlCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct SqliteCommand final : public Command
    {
    private:
        Utf8String _GetName() const override { return ".sqlite"; }
        Utf8String _GetUsage() const override;
        void _Run(Session&, Utf8StringCR args) const override;

        void ExecuteSelect(BeSQLite::Statement&, Session& session) const;
        void ExecuteNonSelect(Session&, BeSQLite::Statement&) const;

    public:
        SqliteCommand() : Command() {}
        ~SqliteCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    09/2017
//---------------------------------------------------------------------------------------
struct JsonCommand final : public Command
    {
    private:
        Utf8String _GetName() const override { return ".json"; }
        Utf8String _GetUsage() const override;
        void _Run(Session&, Utf8StringCR args) const override;

    public:
        JsonCommand() : Command() {}
        ~JsonCommand() {}
    };


//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    03/2014
//---------------------------------------------------------------------------------------
struct CommitCommand final : public Command
    {
    private:
        Utf8String _GetName() const override { return ".commit"; }
        Utf8String _GetUsage() const override { return " .commit                        Commits the current transaction and restarts it."; }
        void _Run(Session&, Utf8StringCR args) const override;

    public:
        CommitCommand() : Command() {}
        ~CommitCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    03/2014
//---------------------------------------------------------------------------------------
struct RollbackCommand final : public Command
    {
    private:
        Utf8String _GetName() const override { return ".rollback"; }
        Utf8String _GetUsage() const override;
        void _Run(Session&, Utf8StringCR args) const override;

    public:
        RollbackCommand() : Command() {}
        ~RollbackCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    12/2017
//---------------------------------------------------------------------------------------
struct AttachCommand final : public Command
    {
    private:
        Utf8String _GetName() const override { return ".attach"; }
        Utf8String _GetUsage() const override;
        void _Run(Session&, Utf8StringCR args) const override;
    public:
        AttachCommand() : Command() {}
        ~AttachCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    12/2017
//---------------------------------------------------------------------------------------
struct DetachCommand final : public Command
    {
    private:
        Utf8String _GetName() const override { return ".detach"; }
        Utf8String _GetUsage() const override;
        void _Run(Session&, Utf8StringCR args) const override;
    public:
        DetachCommand() : Command() {}
        ~DetachCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Affan.Khan    11/2017
//---------------------------------------------------------------------------------------
struct ChangeCommand final : public Command
    {
    private:
        Utf8String _GetName() const override { return ".change"; }
        Utf8String _GetUsage() const override;
        void _Run(Session&, Utf8StringCR args) const override;
    public:
        ChangeCommand(): Command() {}
        ~ChangeCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct MetadataCommand final : public Command
    {
    private:
        Utf8String _GetName() const override { return ".metadata"; }
        Utf8String _GetUsage() const override;
        void _Run(Session&, Utf8StringCR args) const override;

        static Utf8String GetPropertyTypeName(ECN::ECPropertyCR);

    public:
        MetadataCommand() : Command() {}
        ~MetadataCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct ParseCommand final : public Command
    {
    private:
        Utf8String _GetName() const override { return ".parse"; }
        Utf8String _GetUsage() const override;
        void _Run(Session&, Utf8StringCR args) const override;

        static void ExpTreeToString(Utf8StringR expTree, JsonValueCR exp, int indentLevel);

    public:
        ParseCommand() : Command() {}
        ~ParseCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct ExitCommand final : public Command
    {
    private:
        Utf8String _GetName() const override { return ".exit"; }
        Utf8String _GetUsage() const override;
        void _Run(Session&, Utf8StringCR args) const override;

    public:
        ExitCommand() : Command() {}
        ~ExitCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct DbSchemaCommand final : public Command
    {
    private:
        Utf8String _GetName() const override { return ".dbschema"; }
        Utf8String _GetUsage() const override;
        void _Run(Session&, Utf8StringCR args) const override;

        void Search(Session&, std::vector<Utf8String> const& searchArgs) const;
        void Search(BeSQLite::Db const&, Utf8CP searchTerm) const;

    public:
        DbSchemaCommand() : Command() {}
        ~DbSchemaCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    03/2017
//---------------------------------------------------------------------------------------
struct SchemaStatsCommand final : public Command
    {
    private:
        struct ClassColumnStats final
            {
        private:
            ECN::ECClassCP m_class = nullptr;
            std::vector<std::pair<Utf8String, uint32_t>> m_columnCountPerTable;
            uint32_t m_totalColumnCount = 0;

        public:
            explicit ClassColumnStats(ECN::ECClassCR ecClass) : m_class(&ecClass) {}

            void Add(Utf8CP tableName, uint32_t colCount)
                { 
                m_columnCountPerTable.push_back(std::make_pair(Utf8String(tableName), colCount));
                m_totalColumnCount += colCount;
                }

            ECN::ECClassCR GetClass() const { return *m_class; }
            uint32_t GetTotalColumnCount() const { return m_totalColumnCount; }
            std::vector<std::pair<Utf8String, uint32_t>> const& GetColCountPerTable() const { return m_columnCountPerTable; }
            };

        struct ClassColumnStatsCollection final
            {
        private:
            std::vector<ClassColumnStats> m_stats;

        public:
            ClassColumnStatsCollection() {}

            void Add(ClassColumnStats const& stat) { m_stats.push_back(stat); }

            void Sort() 
                {
                std::sort(m_stats.begin(), m_stats.end(),
                    [] (ClassColumnStats const& lhs, ClassColumnStats const& rhs) { return lhs.GetColCountPerTable() < rhs.GetColCountPerTable(); });
                }

            bool IsEmpty() const { return m_stats.empty(); }
            uint32_t GetSize() const { return (uint32_t) m_stats.size(); }
            std::vector<ClassColumnStats> const& GetList() const { return m_stats; }
            };

        Utf8String _GetName() const override { return ".schemastats"; }
        Utf8String _GetUsage() const override;
        
        void _Run(Session&, Utf8StringCR args) const override;
        void ComputeClassHierarchyStats(Session&, std::vector<Utf8String> const& args) const;

        static double ComputeQuantile(ClassColumnStatsCollection const&, double p);

    public:
        SchemaStatsCommand() : Command() {}
        ~SchemaStatsCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle 10/2016
//---------------------------------------------------------------------------------------
struct DebugCommand final : public Command
    {
    private:
        Utf8String _GetName() const override { return ".debug"; }
        Utf8String _GetUsage() const override { return "debug"; }
        void _Run(Session&, Utf8StringCR args) const override;

    public:
        DebugCommand() : Command() {}
        ~DebugCommand() {}
    };
