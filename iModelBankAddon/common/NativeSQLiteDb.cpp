/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "NativeSQLiteDb.h"
#include <Bentley/BeFileName.h>
#include <Bentley/bvector.h>
#include <BeSQLite/RevisionChangesFile.h>
#include "DgnSqlFuncsForTriggers.h"

using namespace IModelBank;
USING_NAMESPACE_BENTLEY_SQLITE

Napi::FunctionReference NativeSQLiteDb::s_constructor;

namespace
{
static bool isElementTable(Utf8CP n)
{
    return (0 == strcmp(n, "bis_Element"));
}

static bool isModelTable(Utf8CP n)
{
    return (0 == strcmp(n, "bis_Model"));
}

static bool isAspectTable(Utf8CP n)
{
    return ((0 == strcmp(n, "bis_ElementMultiAspect")) || (0 == strcmp(n, "bis_ElementUniqueAspect")));
}

static void countChanges(NativeSQLiteDb::ChangeSetApplyStats &stats, Changes const &changes)
{
    for (auto change : changes)
    {
        Utf8CP tn;
        int nCols, indirect;
        DbOpcode opcode;
        if (BE_SQLITE_OK == change.GetOperation(&tn, &nCols, &opcode, &indirect))
        {
            NativeSQLiteDb::ChangeSetCounts *counts = isElementTable(tn) ? &stats.element : isModelTable(tn) ? &stats.model : isAspectTable(tn) ? &stats.aspect : nullptr;
            if (nullptr == counts)
                continue;
            switch (opcode)
            {
            case DbOpcode::Delete:
                ++counts->deletes;
                break;
            case DbOpcode::Insert:
                ++counts->inserts;
                break;
            default:
                ++counts->updates;
                break;
            }
        }
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                       07/2018
//---------------------------------------------------------------------------------------
static DbResult applyChangeSet(NativeSQLiteDb::ChangeSetApplyStats &stats, Db &db, bvector<BeFileName> const &blockFileNames)
{
    if (blockFileNames.empty())
        return DbResult::BE_SQLITE_EMPTY;

    RevisionChangesFileReaderBase changesetReader(blockFileNames, db);

    // Apply DDL, if any
    DbSchemaChangeSet dbSchemaChanges;
    bool containsSchemaChanges;
    DbResult result = changesetReader.GetSchemaChanges(containsSchemaChanges, dbSchemaChanges);
    if (result != BE_SQLITE_OK)
    {
        //        BeAssert(false);
        return result;
    }

    if (containsSchemaChanges && !dbSchemaChanges.IsEmpty())
    {
        stats.containsSchemaChanges = true;

        DbResult status = db.ExecuteSql(dbSchemaChanges.ToString().c_str());
        if (BE_SQLITE_OK != status)
            return status;
    }

    countChanges(stats, changesetReader.GetChanges());

    // Apply normal/data changes (including ec_ table data changes)
    return changesetReader.ApplyChanges(db);
}

} // namespace

bool NativeSQLiteDb::InstanceOf(Napi::Value val)
{
    if (!val.IsObject())
        return false;

    Napi::HandleScope scope(val.Env());
    return val.As<Napi::Object>().InstanceOf(s_constructor.Value());
}
Db &NativeSQLiteDb::GetDb()
{
    if (m_db == nullptr)
        m_db = std::make_unique<Db>();

    return *m_db;
}

Napi::Value NativeSQLiteDb::CreateDb(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENT_STRING(0, dbName);
    REQUIRE_ARGUMENT_INTEGER(1, defaultTxn);
    RETURN_IF_HAD_EXCEPTION
    BeSQLite::Db::CreateParams params;
    params.SetStartDefaultTxn((BeSQLite::DefaultTxn)defaultTxn);
    DbResult status = GetDb().CreateNewDb(BeFileName(dbName.c_str(), true), BeGuid(), params);
    return Napi::Number::New(Env(), (int)status);
}

Napi::Value NativeSQLiteDb::OpenDb(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENT_STRING(0, dbName);
    REQUIRE_ARGUMENT_INTEGER(1, mode);
    REQUIRE_ARGUMENT_INTEGER(2, defaultTxn);
    RETURN_IF_HAD_EXCEPTION
    DbResult status = GetDb().OpenBeSQLiteDb(BeFileName(dbName.c_str(), true), BeSQLite::Db::OpenParams((Db::OpenMode)mode, (BeSQLite::DefaultTxn)defaultTxn));
    if ((BeSQLite::BE_SQLITE_OK == status) && GetDb().IsDbOpen())
        DgnSqlFuncsForTriggers::Register(GetDb());
    return Napi::Number::New(Env(), (int)status);
}

Napi::Value NativeSQLiteDb::CloseDb(const Napi::CallbackInfo &info)
{
    if (m_db != nullptr)
    {
        m_db->CloseDb();
        m_db = nullptr;
    }
    return Napi::Number::New(Env(), (int)BE_SQLITE_OK);
}

Napi::Value NativeSQLiteDb::SaveChanges(const Napi::CallbackInfo &info)
{
    OPTIONAL_ARGUMENT_STRING(0, changeSetName);
    RETURN_IF_HAD_EXCEPTION
    const DbResult status = GetDb().SaveChanges(changeSetName.empty() ? nullptr : changeSetName.c_str());
    return Napi::Number::New(Env(), (int)status);
}

Napi::Value NativeSQLiteDb::AbandonChanges(const Napi::CallbackInfo &info)
{
    DbResult status = GetDb().AbandonChanges();
    return Napi::Number::New(Env(), (int)status);
}

Napi::Value NativeSQLiteDb::GetDbGuid(const Napi::CallbackInfo &info)
{
    auto dbGuid = GetDb().GetDbGuid();
    return Napi::String::New(Env(), dbGuid.ToString().c_str());
}

Napi::Value NativeSQLiteDb::CreateTable(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENT_STRING(0, tableName);
    REQUIRE_ARGUMENT_STRING(1, ddl);
    DbResult status = GetDb().CreateTable(tableName.c_str(), ddl.c_str());
    return Napi::Number::New(Env(), (int)status);
}

Napi::Value NativeSQLiteDb::IsOpen(const Napi::CallbackInfo &info) { return Napi::Boolean::New(Env(), GetDb().IsDbOpen()); }

// To catch any SQLite error when it happens, put a BP on sqlite3ErrorMsg

/*
    Prereqs:
    Define SQLite functions: DGN_bbox, DGN_bbox_value, DGN_placement_aabb, DGN_placement, DGN_point, DGN_angles
*/

Napi::Object NativeSQLiteDb::countsToJs(ChangeSetCounts &counts)
{
    auto countsJs = Napi::Object::New(Env());
    countsJs["inserts"] = Napi::Number::New(Env(), counts.inserts);
    countsJs["deletes"] = Napi::Number::New(Env(), counts.deletes);
    countsJs["updates"] = Napi::Number::New(Env(), counts.updates);
    return countsJs;
}

Napi::Value NativeSQLiteDb::ApplyChangeset(const Napi::CallbackInfo &info)
{
    if (info.Length() < 1 || !info[0].IsArray())
        Napi::TypeError::New(info.Env(), "Argument 1 must be a string[]").ThrowAsJavaScriptException();
    auto blockFileNamesJs = info[0].As<Napi::Array>();
    bvector<BeFileName> blockFileNames;
    for (uint32_t i = 0; i < blockFileNamesJs.Length(); ++i)
    {
        Napi::Value item = blockFileNamesJs[i];
        blockFileNames.push_back(BeFileName(item.ToString().Utf8Value().c_str(), true));
    }

    ChangeSetApplyStats stats;
    auto status = applyChangeSet(stats, GetDb(), blockFileNames);

    auto statsJs = Napi::Object::New(Env());
    statsJs["containsSchemaChanges"] = Napi::Boolean::New(Env(), stats.containsSchemaChanges);
    statsJs["status"] = Napi::Number::New(Env(), (int)status);
    statsJs["element"] = countsToJs(stats.element);
    statsJs["model"] = countsToJs(stats.model);
    statsJs["aspect"] = countsToJs(stats.aspect);
    return statsJs;
}

void NativeSQLiteDb::Init(Napi::Env env, Napi::Object exports)
{
    // ***
    // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
    // ***
    Napi::HandleScope scope(env);
    Napi::Function t = DefineClass(env, "NativeSQLiteDb", {InstanceMethod("getDbGuid", &NativeSQLiteDb::GetDbGuid), InstanceMethod("applyChangeSet", &NativeSQLiteDb::ApplyChangeset), InstanceMethod("createTable", &NativeSQLiteDb::CreateTable), InstanceMethod("createDb", &NativeSQLiteDb::CreateDb), InstanceMethod("openDb", &NativeSQLiteDb::OpenDb), InstanceMethod("closeDb", &NativeSQLiteDb::CloseDb), InstanceMethod("saveChanges", &NativeSQLiteDb::SaveChanges), InstanceMethod("abandonChanges", &NativeSQLiteDb::AbandonChanges), InstanceMethod("isOpen", &NativeSQLiteDb::IsOpen)});

    exports.Set("NativeSQLiteDb", t);

    s_constructor = Napi::Persistent(t);
    // Per N-API docs: Call this on a reference that is declared as static data, to prevent its destructor
    // from running at program shutdown time, which would attempt to reset the reference when
    // the environment is no longer valid.
    s_constructor.SuppressDestruct();
}
