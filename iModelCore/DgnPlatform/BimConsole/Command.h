/*--------------------------------------------------------------------------------------+
|
|     $Source: BimConsole/Command.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDbApi.h>
#include <vector>
#include <map>

USING_NAMESPACE_BENTLEY

//forward declaration
struct Session;

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct Command
    {
    private:
        virtual Utf8String _GetName() const = 0;
        virtual Utf8String _GetUsage() const = 0;
        virtual void _Run(Session&, std::vector<Utf8String> const& args) const = 0;

    protected:
        Command() {}

        static Utf8String ConcatArgs(size_t startIndex, std::vector<Utf8String> const& args);

    public:
        virtual ~Command() {}

        Utf8String GetName() const { return _GetName(); }
        Utf8String GetUsage() const { return _GetUsage(); }
        void Run(Session& session, std::vector<Utf8String> const& args) const;
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct HelpCommand : public Command
    {
    private:
        std::map<Utf8String, std::shared_ptr<Command>> const& m_commandMap;

        virtual Utf8String _GetName() const override { return ".help"; }
        virtual Utf8String _GetUsage() const override { return " .help, .h                      Displays all available commands"; }
        virtual void _Run(Session&, std::vector<Utf8String> const& args) const override;

    public:
        explicit HelpCommand(std::map<Utf8String, std::shared_ptr<Command>> const& commandMap)
            : Command(), m_commandMap(commandMap)
            {}

        ~HelpCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct OpenCommand : public Command
    {
    private:
        static Utf8CP const READONLY_SWITCH;
        static Utf8CP const READWRITE_SWITCH;

        virtual Utf8String _GetName() const override { return ".open"; }
        virtual Utf8String _GetUsage() const override;
        virtual void _Run(Session&, std::vector<Utf8String> const& args) const override;

    public:
        OpenCommand() : Command() {}
        ~OpenCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct CloseCommand : public Command
    {
    private:
        virtual Utf8String _GetName() const override { return ".close"; }
        virtual Utf8String _GetUsage() const override { return " .close                         Closes the currently open file"; }

        virtual void _Run(Session&, std::vector<Utf8String> const& args) const override;

    public:
        CloseCommand()
            : Command()
            {}

        ~CloseCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct CreateCommand : public Command
    {
    private:
        virtual Utf8String _GetName() const override { return ".create"; }
        virtual Utf8String _GetUsage() const override;
        virtual void _Run(Session&, std::vector<Utf8String> const& args) const override;

    public:
        CreateCommand() : Command() {}
        ~CreateCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct FileInfoCommand : public Command
    {
    private:
        enum class KnownProfile
            {
            BeSQLite,
            ECDb,
            DgnDb,
            Unknown
            };

        virtual Utf8String _GetName() const override { return ".fileinfo"; }
        virtual Utf8String _GetUsage() const override { return " .fileinfo                      Displays information about the open file"; }

        virtual void _Run(Session&, std::vector<Utf8String> const& args) const override;

    public:
        FileInfoCommand() : Command() {}
        ~FileInfoCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct ImportCommand : public Command
    {
    private:
        static Utf8CP const ECSCHEMA_SWITCH;

        virtual Utf8String _GetName() const override { return ".import"; }
        virtual Utf8String _GetUsage() const override;
        virtual void _Run(Session&, std::vector<Utf8String> const& args) const override;

        void RunImportSchema(Session&, BeFileNameCR ecschemaPath) const;
        static BentleyStatus DeserializeECSchema(ECN::ECSchemaReadContextR readContext, BeFileNameCR ecschemaFilePath);
    public:
        ImportCommand() : Command() {}
        ~ImportCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct ExportCommand : public Command
    {
    private:
        static Utf8CP const ECSCHEMA_SWITCH;
        static Utf8CP const TABLES_SWITCH;
        virtual Utf8String _GetName() const override { return ".export"; }
        virtual Utf8String _GetUsage() const override;
        virtual void _Run(Session&, std::vector<Utf8String> const& args) const override;

        void RunExportSchema(Session&, Utf8CP outFolder, bool useECXmlV2) const;
        void RunExportTables(Session&, Utf8CP jsonFile) const;

        void ExportTables(Session&, Utf8CP jsonFile) const;
        void ExportTable(Session&, Json::Value& out, Utf8CP tableName) const;
    public:
        ExportCommand() : Command() {}
        ~ExportCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    12/2015
//---------------------------------------------------------------------------------------
struct CreateECClassViewsCommand : public Command
    {
    private:
        virtual Utf8String _GetName() const override;
        virtual Utf8String _GetUsage() const override;
        virtual void _Run(Session&, std::vector<Utf8String> const& args) const override;
    public:
        CreateECClassViewsCommand() : Command() {}
        ~CreateECClassViewsCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct ECSqlCommand : public Command
    {
    private:
        virtual Utf8String _GetName() const override;
        virtual Utf8String _GetUsage() const override;
        virtual void _Run(Session&, std::vector<Utf8String> const& args) const override;

        void ExecuteSelect(Session&, BeSQLite::EC::ECSqlStatement&) const;
        void ExecuteInsert(Session&, BeSQLite::EC::ECSqlStatement&) const;
        void ExecuteUpdateOrDelete(Session&, BeSQLite::EC::ECSqlStatement&) const;

        static Utf8String PrimitiveToString(BeSQLite::EC::IECSqlValue const&, ECN::PrimitiveType);
        static Utf8String PrimitiveToString(BeSQLite::EC::IECSqlValue const&);
        static Utf8String ArrayToString(BeSQLite::EC::IECSqlValue const& arrayValue, ECN::ECPropertyCP);
        static Utf8String StructToString(BeSQLite::EC::IECSqlValue const& structValue);

    public:
        ECSqlCommand() : Command() {}
        ~ECSqlCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct SqliteCommand : public Command
    {
    private:
        virtual Utf8String _GetName() const override;
        virtual Utf8String _GetUsage() const override;
        virtual void _Run(Session&, std::vector<Utf8String> const& args) const override;

        void ExecuteSelect(BeSQLite::Statement&) const;
        void ExecuteNonSelect(Session&, BeSQLite::Statement&) const;

    public:
        SqliteCommand() : Command() {}
        ~SqliteCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    03/2014
//---------------------------------------------------------------------------------------
struct CommitCommand : public Command
    {
    private:
        virtual Utf8String _GetName() const override { return ".commit"; }
        virtual Utf8String _GetUsage() const override { return " .commit                        Commits the current transaction and restarts it."; }
        virtual void _Run(Session&, std::vector<Utf8String> const& args) const override;

    public:
        CommitCommand() : Command() {}
        ~CommitCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    03/2014
//---------------------------------------------------------------------------------------
struct RollbackCommand : public Command
    {
    private:
        virtual Utf8String _GetName() const override;
        virtual Utf8String _GetUsage() const override;
        virtual void _Run(Session&, std::vector<Utf8String> const& args) const override;

    public:
        RollbackCommand() : Command() {}
        ~RollbackCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct MetadataCommand : public Command
    {
    private:
        virtual Utf8String _GetName() const override;
        virtual Utf8String _GetUsage() const override;
        virtual void _Run(Session&, std::vector<Utf8String> const& args) const override;

    public:
        MetadataCommand() : Command() {}
        ~MetadataCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct SqlCommand : public Command
    {
    private:
        virtual Utf8String _GetName() const override;
        virtual Utf8String _GetUsage() const override;
        virtual void _Run(Session&, std::vector<Utf8String> const& args) const override;

    public:
        SqlCommand() : Command() {}
        ~SqlCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct ParseCommand : public Command
    {
    private:
        static Utf8CP const RAW_SWITCH;

        virtual Utf8String _GetName() const override;
        virtual Utf8String _GetUsage() const override;
        virtual void _Run(Session&, std::vector<Utf8String> const& args) const override;

        static void ExpTreeToString(Utf8StringR expTree, JsonValueCR exp, int indentLevel);

    public:
        ParseCommand() : Command() {}
        ~ParseCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct ExitCommand : public Command
    {
    private:
        virtual Utf8String _GetName() const override;
        virtual Utf8String _GetUsage() const override;
        virtual void _Run(Session&, std::vector<Utf8String> const& args) const override;

    public:
        ExitCommand() : Command() {}
        ~ExitCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct DbSchemaCommand : public Command
    {
    private:
        virtual Utf8String _GetName() const override;
        virtual Utf8String _GetUsage() const override;
        virtual void _Run(Session&, std::vector<Utf8String> const& args) const override;

        void Search(Session&, std::vector<Utf8String> const& args) const;
        void Search(BeSQLite::EC::ECDb const&, Utf8CP searchTerm) const;

    public:
        DbSchemaCommand() : Command() {}
        ~DbSchemaCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Affan.Khan    10/2013
//---------------------------------------------------------------------------------------
struct ClassMappingCommand : public Command
    {
    private:
        virtual Utf8String _GetName() const override;
        virtual Utf8String _GetUsage() const override;
        virtual void _Run(Session&, std::vector<Utf8String> const& args) const override;

    public:
        ClassMappingCommand() : Command() {}
        ~ClassMappingCommand() {}
    };
