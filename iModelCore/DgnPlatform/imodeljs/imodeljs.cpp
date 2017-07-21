/*--------------------------------------------------------------------------------------+
|
|     $Source: imodeljs/imodeljs.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include "imodeljs.h"
#include <Bentley/Desktop/FileSystem.h>

static Utf8String s_lastEcdbIssue;
static BeFileName s_addonDllDir;

namespace { // Ensure that none of the classes defined here end up with the same name as classes defined in DgnPlatform. 
            // If that happened, the Windows linker would silently select one of the implementations to use everywhere.

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  05/17
//---------------------------------------------------------------------------------------
struct ECDbIssueListener : BeSQLite::EC::ECDb::IIssueListener
    {
    void _OnIssueReported(BentleyApi::Utf8CP message) const override {s_lastEcdbIssue = message;}
    };

static ECDbIssueListener s_listener;

/*=================================================================================**//**
* An implementation of IKnownLocationsAdmin that is useful for desktop applications.
* This implementation works for Windows, Linux, and MacOS.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct KnownAddonLocationsAdmin : DgnPlatformLib::Host::IKnownLocationsAdmin
{
    BeFileName m_tempDirectory;
    BeFileName m_assetsDirectory;

    BeFileNameCR _GetLocalTempDirectoryBaseName() override {return m_tempDirectory;}
    BeFileNameCR _GetDgnPlatformAssetsDirectory() override {return m_assetsDirectory;}

    //! Construct an instance of the KnownDesktopLocationsAdmin
    KnownAddonLocationsAdmin()
        {
        Desktop::FileSystem::BeGetTempPath(m_tempDirectory);
        m_assetsDirectory = s_addonDllDir;
        m_assetsDirectory.AppendToPath(L"Assets");
        }
};

//=======================================================================================
// @bsistruct                                   Sam.Wilson                  05/17
//=======================================================================================
struct AddonHost : DgnPlatformLib::Host
{
private:
    void _SupplyProductName(Utf8StringR name) override { name.assign("imodeljs"); }
    IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new KnownAddonLocationsAdmin(); }
    BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override
        {
        BeFileName sqlang(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
        sqlang.AppendToPath(L"sqlang/DgnPlatform_en.sqlang.db3");
        return BeSQLite::L10N::SqlangFiles(sqlang);
        }

    static void OnAssert(WCharCP msg, WCharCP file, unsigned line, BeAssertFunctions::AssertType type)
        {
        printf("Assertion Failure: %ls (%ls:%d)\n", msg, file, line);
        //DebugBreak();
        }
public:
    AddonHost() { BeAssertFunctions::SetBeAssertHandler(&AddonHost::OnAssert); }
};

}; // end anonymous namespace

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  05/17
//---------------------------------------------------------------------------------------
void imodeljs::Initialize(BeFileNameCR addonDllDir)
    {
    s_addonDllDir = addonDllDir;

    static std::once_flag s_initFlag;
    std::call_once(s_initFlag, []() 
        {
        DgnPlatformLib::Initialize(*new AddonHost, true);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  05/17
//---------------------------------------------------------------------------------------
Utf8String imodeljs::GetLastEcdbIssue() 
    {
    // It's up to the caller to serialize access to this.
    return s_lastEcdbIssue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  05/17
//---------------------------------------------------------------------------------------
DgnDbPtr imodeljs::GetDbByName(DbResult& dbres, Utf8String& errmsg, BeFileNameCR fn, DgnDb::OpenMode mode)
    {
    static bmap<BeFileName, DgnDbPtr>* s_dbs;
    
    // *** 
    // ***  TBD: sort out readonly vs. readwrite 
    // ***

    BeSystemMutexHolder threadSafeInScope;

    if (nullptr == s_dbs)
        s_dbs = new bmap<BeFileName, DgnDbPtr>();

    auto found = s_dbs->find(fn);
    if (found != s_dbs->end())
        return found->second;

    // *** TBD: keep some kind of last-used-time for each db
    // *** TBD: if we have too many Dbs open, then close some that have not been accessed for a while.

    BeFileName dbfilename;
    if (!fn.GetDirectoryName().empty())
        {
        dbfilename = fn;
        }
    else
        {
        // *** NEEDS WORK: To save the user the trouble of typing in a full path name, we'll let him
        //                  define an envvar that defines the directory.
        BeFileName dbDir;
        Utf8CP dbdirenv = getenv("NODE_DGNDB_DIR");
        if (nullptr != dbdirenv)
            dbDir.SetNameUtf8(dbdirenv);
        else
            {
            Desktop::FileSystem::GetCwd(dbDir);
            dbDir.AppendToPath(L"briefcases");

            if (!dbDir.DoesPathExist())
                {
                dbres = DbResult::BE_SQLITE_NOTFOUND;
                errmsg = Utf8String(dbDir);
                errmsg.append(" - directory not found. Where are the bim files? You can put them in a subdirectory called 'briefcases', or you can set NODE_DGNDB_DIR in your shell to point to a directory that contains them.");
                return nullptr;
                }
            }
        dbfilename = dbDir;
        dbfilename.AppendToPath(fn.c_str());
        }

    auto db = DgnDb::OpenDgnDb(&dbres, dbfilename, DgnDb::OpenParams(mode));
    if (!db.IsValid())
        {
        errmsg = DgnDb::InterpretDbResult(dbres);
        return nullptr;
        }

    db->AddIssueListener(s_listener); 

    s_dbs->insert(make_bpair(fn, db));
    return db;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
Json::Value& imodeljs::GetRowAsRawJson(Json::Value& rowAsJson, ECSqlStatement& stmt)
    {
    if (rowAsJson.isNull())
        {
        JsonECSqlSelectAdapter adapter(stmt);
        adapter.GetRowAsIs(rowAsJson);
        }
    return rowAsJson;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
void imodeljs::GetRowAsJson(Json::Value& rowJson, ECSqlStatement& stmt) 
    {
    int cols = stmt.GetColumnCount();

    Json::Value rawRowJson;

    for (int i = 0; i < cols; i++)
        {
        IECSqlValue const& value = stmt.GetValue(i);
        ECSqlColumnInfoCR info = value.GetColumnInfo();
        BeAssert(info.IsValid());
    
        Utf8String name = info.GetProperty()->GetName();
        
        name[0] = tolower(name[0]);
        
        if (value.IsNull())
            {
            rowJson[name] = Json::Value();
            continue;
            }

        ECN::ECTypeDescriptor typedesc = info.GetDataType();
        if (typedesc.IsNavigation())
            {
            ECN::ECClassId relClassId;
            auto eid = value.GetNavigation<DgnElementId>(&relClassId);
            ECN::ECValue value(eid, relClassId);
            JsonUtils::NavigationPropertyToJson(rowJson[name], value.GetNavigationInfo());
            continue;
            }

        if (!typedesc.IsPrimitive())
            {
            rowJson[name] = GetRowAsRawJson(rawRowJson, stmt)[i];     // *** WIP_NODE_ADDON
            continue;
            }

        switch (typedesc.GetPrimitiveType())
            {
            case ECN::PRIMITIVETYPE_Long:
                rowJson[name] = value.GetUInt64();
                break;
            case ECN::PRIMITIVETYPE_Integer:
                rowJson[name] = Json::Value(value.GetInt());
                break;
            case ECN::PRIMITIVETYPE_Double:
                rowJson[name] = Json::Value(value.GetDouble());
                break;
            case ECN::PRIMITIVETYPE_String: 
                rowJson[name] = Json::Value(value.GetText());
                break;
            case ECN::PRIMITIVETYPE_Binary:
            #ifdef NEEDS_WORK
                {
                int length;
                const void* blob = value.GetBlob(&length);
                row->push_back(new Values::Blob(name, length, blob));
                break;
                }
            #else
                rowJson[name] = Json::Value();
                break;
            #endif
            case ECN::PRIMITIVETYPE_Point2d: 
                JsonUtils::DPoint2dToJson(rowJson[name], value.GetPoint2d());                 // *** WIP_NODE_ADDON
                break;
            case ECN::PRIMITIVETYPE_Point3d:
                JsonUtils::DPoint3dToJson(rowJson[name], value.GetPoint3d());                 // *** WIP_NODE_ADDON
                break;
            case ECN::PRIMITIVETYPE_DateTime:
                rowJson[name] = Json::Value(value.GetDateTime().ToString().c_str());          // *** WIP_NODE_ADDON
                break;
            default: 
                {
                BeAssert(false && "TBD");
                rowJson[name] = GetRowAsRawJson(rawRowJson, stmt)[i];     // *** WIP_NODE_ADDON
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
void imodeljs::GetECValuesCollectionAsJson(Json::Value& json, ECN::ECValuesCollectionCR props) 
    {
    for (ECN::ECPropertyValue const& prop : props)
        {
        JsonValueR pvalue = json[prop.GetValueAccessor().GetAccessString(prop.GetValueAccessor().GetDepth()-1)];

        if (prop.HasChildValues())
            {
            DgnDbStatus status;
            GetECValuesCollectionAsJson(pvalue, *prop.GetChildValues());
            }
        else 
            {
            ECUtils::ConvertECValueToJson(pvalue, prop.GetValue());
            }
        }
    }