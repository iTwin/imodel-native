/*--------------------------------------------------------------------------------------+
|
|     $Source: ECSqlConsole/ConsoleCommand.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/WString.h>
#include <Bentley/NonCopyableClass.h>
#include <vector>
#include <map>
#include <ECDb/ECSqlStatement.h>
#include <ECDb/ECDbApi.h>

#include <ECObjects/ECObjectsAPI.h>
#include <json/json.h>
USING_NAMESPACE_BENTLEY

//forward declaration
struct ECSqlConsoleSession;

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct ConsoleCommand
    {
private:
    virtual Utf8String _GetName () const = 0;
    virtual Utf8String _GetUsage () const = 0;
    virtual void _Run (ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const = 0;

protected:
    ConsoleCommand () {}

    static Utf8String ConcatArgs (size_t startIndex, std::vector<Utf8String> const& args);

public:
    virtual ~ConsoleCommand () {}

    Utf8String GetName () const;
    Utf8String GetUsage () const;
    void Run (ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const;
    };

typedef std::map<Utf8String, std::shared_ptr<ConsoleCommand>> ConsoleCommandMap;

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct HelpCommand : public ConsoleCommand, NonCopyableClass
    {
private:
    ConsoleCommandMap const& m_commandMap;

    virtual Utf8String _GetName () const override;
    virtual Utf8String _GetUsage () const override;
    virtual void _Run (ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const override;

public:
    explicit HelpCommand (ConsoleCommandMap const& commandMap) 
        : ConsoleCommand (), m_commandMap (commandMap)
        {}

    ~HelpCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct OpenCommand : public ConsoleCommand, NonCopyableClass
    {
private:
    static Utf8CP const READONLY_SWITCH;
    static Utf8CP const READWRITE_SWITCH;

    virtual Utf8String _GetName () const override;
    virtual Utf8String _GetUsage () const override;
    virtual void _Run (ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const override;

public:
    OpenCommand () 
        : ConsoleCommand () 
        {}

    ~OpenCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct CloseCommand : public ConsoleCommand, NonCopyableClass
    {
private:
    virtual Utf8String _GetName () const override;
    virtual Utf8String _GetUsage () const override;
    virtual void _Run (ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const override;

public:
    CloseCommand () 
        : ConsoleCommand () 
        {}

    ~CloseCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct CreateCommand : public ConsoleCommand, NonCopyableClass
    {
private:
    virtual Utf8String _GetName () const override;
    virtual Utf8String _GetUsage () const override;
    virtual void _Run (ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const override;

public:
    CreateCommand () 
        : ConsoleCommand () 
        {}

    ~CreateCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct PathCommand : public ConsoleCommand, NonCopyableClass
    {
private:
    virtual Utf8String _GetName () const override;
    virtual Utf8String _GetUsage () const override;
    virtual void _Run (ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const override;

public:
    PathCommand () 
        : ConsoleCommand () 
        {}

    ~PathCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct ImportCommand : public ConsoleCommand, NonCopyableClass
    {
private:
    static Utf8CP const ECSCHEMA_SWITCH;

    virtual Utf8String _GetName () const override;
    virtual Utf8String _GetUsage () const override;
    virtual void _Run (ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const override;
    
    void RunImportSchema (ECSqlConsoleSession& session, BeFileNameCR ecschemaPath) const;
    static BentleyStatus DeserializeECSchema (ECN::ECSchemaReadContextR readContext, BeFileNameCR ecschemaFilePath);
public:
    ImportCommand () : ConsoleCommand () {}
    ~ImportCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct ExportCommand : public ConsoleCommand, NonCopyableClass
    {
private:
    static Utf8CP const ECSCHEMA_SWITCH;
    static Utf8CP const TABLES_SWITCH;
    virtual Utf8String _GetName () const override;
    virtual Utf8String _GetUsage () const override;
    virtual void _Run (ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const override;

    void RunExportSchema (ECSqlConsoleSession& session, Utf8CP outFolder) const;
    void RunExportTables (ECSqlConsoleSession& session, Utf8CP jsonFile) const;

    void ExportTables (ECSqlConsoleSession& session, Utf8CP jsonFile) const;
    void ExportTable (ECSqlConsoleSession& session, Json::Value& out, Utf8CP tableName) const;
public:
    ExportCommand () : ConsoleCommand ()  {}
    ~ExportCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct PopulateCommand : public ConsoleCommand, NonCopyableClass
    {
private:
    virtual Utf8String _GetName () const override;
    virtual Utf8String _GetUsage () const override;
    virtual void _Run (ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const override;

public:
    PopulateCommand () : ConsoleCommand () {}

    ~PopulateCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct ECSqlCommand : public ConsoleCommand, NonCopyableClass
    {
private:
    virtual Utf8String _GetName () const override;
    virtual Utf8String _GetUsage () const override;
    virtual void _Run (ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const override;

    void ExecuteSelect (ECSqlConsoleSession& session, BeSQLite::EC::ECSqlStatement& statement) const;
    void ExecuteInsert (BeSQLite::EC::ECSqlStatement& statement) const;
    void ExecuteUpdateOrDelete (BeSQLite::EC::ECSqlStatement& statement) const;

    static Utf8String PrimitiveToString (BeSQLite::EC::IECSqlValue const& value, ECN::PrimitiveType type);
    static Utf8String PrimitiveToString (BeSQLite::EC::IECSqlValue const& value);
    static Utf8String ArrayToString (BeSQLite::EC::IECSqlValue const& arrayValue, ECN::ECPropertyCP property);
    static Utf8String StructToString (BeSQLite::EC::IECSqlValue const& structValue);

public:
    ECSqlCommand ()  : ConsoleCommand () {}
    ~ECSqlCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct SqliteCommand : public ConsoleCommand, NonCopyableClass
    {
private:
    virtual Utf8String _GetName() const override;
    virtual Utf8String _GetUsage() const override;
    virtual void _Run(ECSqlConsoleSession&, std::vector<Utf8String> const& args) const override;

    void ExecuteSelect(BeSQLite::Statement&) const;
    void ExecuteNonSelect(ECSqlConsoleSession&, BeSQLite::Statement&) const;

public:
    SqliteCommand() : ConsoleCommand() {}
    ~SqliteCommand() {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    03/2014
//---------------------------------------------------------------------------------------
struct CommitCommand : public ConsoleCommand, NonCopyableClass
    {
private:
    virtual Utf8String _GetName () const override;
    virtual Utf8String _GetUsage () const override;
    virtual void _Run (ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const override;

public:
    CommitCommand () : ConsoleCommand () {}
    ~CommitCommand () {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    03/2014
//---------------------------------------------------------------------------------------
struct RollbackCommand : public ConsoleCommand, NonCopyableClass
    {
private:
    virtual Utf8String _GetName () const override;
    virtual Utf8String _GetUsage () const override;
    virtual void _Run (ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const override;

public:
    RollbackCommand ()
        : ConsoleCommand ()
        {}

    ~RollbackCommand () {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct MetadataCommand : public ConsoleCommand, NonCopyableClass
    {
private:
    virtual Utf8String _GetName () const override;
    virtual Utf8String _GetUsage () const override;
    virtual void _Run (ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const override;

public:
    MetadataCommand () 
        : ConsoleCommand ()
        {}

    ~MetadataCommand () {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct SqlCommand : public ConsoleCommand, NonCopyableClass
    {
private:
    virtual Utf8String _GetName () const override;
    virtual Utf8String _GetUsage () const override;
    virtual void _Run (ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const override;

public:
    SqlCommand () 
        : ConsoleCommand ()
        {}

    ~SqlCommand () {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct ParseCommand : public ConsoleCommand, NonCopyableClass
    {
private:
    static Utf8CP const RAW_SWITCH;

    virtual Utf8String _GetName () const override;
    virtual Utf8String _GetUsage () const override;
    virtual void _Run (ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const override;

public:
    ParseCommand () 
        : ConsoleCommand ()
        {}

    ~ParseCommand () {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct SetCommand : public ConsoleCommand, NonCopyableClass
    {
private:
    static Utf8CP const OUTPUT_SWITCH;
    static Utf8CP const OUTPUT_TABLE_SWITCH;
    static Utf8CP const OUTPUT_LIST_SWITCH;

    virtual Utf8String _GetName () const override;
    virtual Utf8String _GetUsage () const override;
    virtual void _Run (ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const override;

public:
    SetCommand () 
        : ConsoleCommand ()
        {}

    ~SetCommand () {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct HistoryCommand : public ConsoleCommand, NonCopyableClass
    {
private:
    virtual Utf8String _GetName () const override;
    virtual Utf8String _GetUsage () const override;
    virtual void _Run (ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const override;

public:
    HistoryCommand () 
        : ConsoleCommand ()
        {}

    ~HistoryCommand () {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct ExitCommand : public ConsoleCommand, NonCopyableClass
    {
private:
    virtual Utf8String _GetName () const override;
    virtual Utf8String _GetUsage () const override;
    virtual void _Run (ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const override;

public:
    ExitCommand () 
        : ConsoleCommand ()
        {}

    ~ExitCommand () {}
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                                   Krischan.Eberle    10/2013
//---------------------------------------------------------------------------------------
struct ECSchemaDiffCommand : public ConsoleCommand, NonCopyableClass
    {
private:
    virtual Utf8String _GetName () const override;
    virtual Utf8String _GetUsage () const override;
    virtual void _Run (ECSqlConsoleSession& session, std::vector<Utf8String> const& args) const override;

public:
    ECSchemaDiffCommand () 
        : ConsoleCommand () 
        {}

    ~ECSchemaDiffCommand() {}
    };
