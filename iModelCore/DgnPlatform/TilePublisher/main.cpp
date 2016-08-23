/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/main.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TilePublisher.h"
#include <DgnPlatform/DesktopTools/WindowsKnownLocationsAdmin.h>
#include <ThreeMx/ThreeMxApi.h>


USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RENDER
using namespace BentleyApi::Dgn::Render::Tile3d;

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
enum class ParamId
{
    Input = 0,
    View,
    Output,
    Name,
    Compress,
    Invalid
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct CommandParam
{
    WCharCP     m_abbrev;
    WCharCP     m_verbose;
    WCharCP     m_descr;
    bool        m_required;
    bool        m_boolean;  // True => arg is true if param specified. False => expect 'arg=value'.
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
static CommandParam s_paramTable[] =
    {
        { L"i", L"input", L"Name of the .bim file to publish", true },
        { L"v", L"view", L"Name of the view to publish. If omitted, the default view is used", false },
        { L"o", L"output", L"Directory in which to place the output .html file. If omitted, the output is placed in the .bim file's directory", false },
        { L"n", L"name", L"Name of the .html file and root name of the tileset .json and .b3dm files. If omitted, uses the name of the .bim file", false },
        { L"c", L"compress", L"If specified, .b3dm files will be compressed using gzip.", false, true },
    };

static const size_t s_paramTableSize = _countof(s_paramTable);

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct CommandArg
{
    ParamId     m_paramId;
    WString     m_value;

    explicit CommandArg(WCharCP raw) : m_paramId(ParamId::Invalid)
        {
        if (WString::IsNullOrEmpty(raw) || '-' != *raw)
            return;

        ++raw;
        bool verboseParamName = *raw == '-';
        if (verboseParamName)
            ++raw;

        WCharCP equalPos = wcschr(raw, '=');
        bool haveArgValue = nullptr != equalPos;
        WCharCP argValue = haveArgValue ? equalPos+1 : nullptr;
        auto paramNameLen = haveArgValue ? equalPos - raw : wcslen(raw);

        if (0 == paramNameLen || (!verboseParamName && 1 != paramNameLen))
            return;

        for (size_t i = 0; i < s_paramTableSize; i++)
            {
            auto const& param = s_paramTable[i];
            WCharCP paramName = verboseParamName ? param.m_verbose : param.m_abbrev;
            if (0 != wcsncmp(raw, paramName, paramNameLen) || paramNameLen != wcslen(paramName))
                continue;

            if ((nullptr == equalPos) != (param.m_boolean))
                return;

            m_paramId = static_cast<ParamId>(i);

            if (!param.m_boolean)
                {
                m_value = argValue;
                m_value.Trim(L"\"");
                m_value.Trim();
                }

            break;
            }
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct PublisherParams
{
private:
    BeFileName      m_inputFileName;    //!< Path to the .bim
    Utf8String      m_viewName;         //!< Name of the view definition from which to publish
    BeFileName      m_outputDir;        //!< Directory in which to place the output
    WString         m_tilesetName;      //!< Root name of the output tileset files
    bool            m_compress=false;   //!< If true, compress .b3dm files

    DgnViewId GetViewId(DgnDbR db) const;
public:
    BeFileNameCR GetInputFileName() const { return m_inputFileName; }
    BeFileNameCR GetOutputDirectory() const { return m_outputDir; }
    WStringCR GetTilesetName() const { return m_tilesetName; }
    Utf8StringCR GetViewName() const { return m_viewName; }
    bool WantCompressed() const { return m_compress; }

    bool ParseArgs(int ac, wchar_t const** av);
    DgnDbPtr OpenDgnDb() const;
    ViewControllerPtr LoadViewController(DgnDbR db);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId PublisherParams::GetViewId(DgnDbR db) const
    {
    if (!m_viewName.empty())
        return ViewDefinition::QueryViewId(m_viewName, db);

    // Try default view
    DgnViewId viewId;
    if (BeSQLite::BE_SQLITE_ROW == db.QueryProperty(&viewId, sizeof(viewId), DgnViewProperty::DefaultView()) && viewId.IsValid())
        return viewId;

    // Try first spatial view
    for (auto const& entry : ViewDefinition::MakeIterator(db))
        {
        auto view = ViewDefinition::QueryView(entry.GetId(), db);
        if (view.IsValid() && view->IsSpatialView())
            {
            viewId = view->GetViewId();
            break;
            }
        }

    return viewId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr PublisherParams::LoadViewController(DgnDbR db)
    {
    DgnViewId viewId = GetViewId(db);
    ViewDefinitionCPtr view = ViewDefinition::QueryView(viewId, db);
    if (view.IsNull())
        {
        printf("View not found\n");
        return false;
        }

    m_viewName = view->GetName();

    ViewControllerPtr controller = view->LoadViewController();
    if (controller.IsNull())
        {
        printf("Failed to load view %hs\n", view->GetName().c_str());
        return false;
        }

    return controller;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr PublisherParams::OpenDgnDb() const
    {
    DgnDb::OpenParams openParams(DgnDb::OpenMode::Readonly);
    DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, m_inputFileName, openParams);
    if (db.IsNull())
        printf("Failed to open file %ls\n", m_inputFileName.c_str());

    return db;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool PublisherParams::ParseArgs(int ac, wchar_t const** av)
    {
    if (ac < 2)
        return false;

    bool haveInput = false;
    for (int i = 1; i < ac; i++)
        {
        CommandArg arg(av[i]);
        switch (arg.m_paramId)
            {
            case ParamId::Input:
                haveInput = true;
                BeFileName::FixPathName(m_inputFileName, arg.m_value.c_str());
                break;
            case ParamId::View:
                m_viewName = Utf8String(arg.m_value.c_str());
                break;
            case ParamId::Output:
                BeFileName::FixPathName(m_outputDir, arg.m_value.c_str());
                break;
            case ParamId::Name:
                m_tilesetName = arg.m_value.c_str();
                break;
            case ParamId::Compress:
                m_compress = true;
                break;
            default:
                printf("Unrecognized command option %ls\n", av[i]);
                return false;
            }
        }

    if (!haveInput)
        {
        printf("Input filename is required\n");
        return false;
        }

    if (m_outputDir.empty())
        m_outputDir = m_inputFileName.GetDirectoryName();

    if (m_tilesetName.empty())
        m_tilesetName = m_inputFileName.GetFileNameWithoutExtension().c_str();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void printUsage(WCharCP exePath)
    {
    WString exeName = BeFileName::GetFileNameAndExtension(exePath);

    printf("Publish the contents of a DgnDb view as a Cesium tileset viewable in a web browser.\n\n");
    printf("Usage: %ls -i|--input= [OPTIONS...]\n\n", exeName.c_str());
    
    for (auto const& cmdArg : s_paramTable)
        printf("  --%ls=|-%ls=\t(%ls)\t%ls\n", cmdArg.m_verbose, cmdArg.m_abbrev, cmdArg.m_required ? L"required" : L"optional", cmdArg.m_descr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void printStatus(TilesetPublisher::Status status)
    {
    static const Utf8CP s_msg[] =
        {
        "Publishing succeeded",
        "No geometry to publish",
        "Publishing aborted",
        "Failed to write to base directory",
        "Failed to create subdirectory",
        "Failed to write scene",
        "Failed to write node"
        };

    auto index = static_cast<uint32_t>(status);
    Utf8CP msg = index < _countof(s_msg) ? s_msg[index] : "Unrecognized error";
    printf("Result: %hs.\n", msg);
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct Host : DgnPlatformLib::Host
{
private:
    virtual void _SupplyProductName(Utf8StringR name) override { name.assign("TilePublisher"); }
    virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new WindowsKnownLocationsAdmin(); }
    virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override
        {
        BeFileName sqlang(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
        sqlang.AppendToPath(L"sqlang/DgnPlatform_en.sqlang.db3");
        return BeSQLite::L10N::SqlangFiles(sqlang);
        }

    static void OnAssert(WCharCP msg, WCharCP file, unsigned line, BeAssertFunctions::AssertType type)
        {
        printf("Assertion Failure: %ls (%ls:%d)\n", msg, file, line);
        }
public:
    Host() { BeAssertFunctions::SetBeAssertHandler(&Host::OnAssert); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
int wmain(int ac, wchar_t const** av)
    {
    PublisherParams createParams;
    if (!createParams.ParseArgs(ac, av))
        {
        printUsage(av[0]);
        return 1;
        }

    Host host;
    DgnPlatformLib::Initialize(host, false);

    DgnDomains::RegisterDomain(ThreeMx::ThreeMxDomain::GetDomain());

    DgnDbPtr db = createParams.OpenDgnDb();
    if (db.IsNull())
        return 1;


    ViewControllerPtr viewController = createParams.LoadViewController(*db);
    if (viewController.IsNull())
        return 1;

    TilesetPublisher publisher(*viewController, createParams.GetOutputDirectory(), createParams.GetTilesetName());

    printf("Publishing:\n"
           "\tInput: View %s from %ls\n"
           "\tOutput: %ls%ls.html\n"
           "\tData: %ls\n"
           "\tCompressed: %s\n",
            createParams.GetViewName().c_str(), createParams.GetInputFileName().c_str(), publisher.GetOutputDirectory().c_str(), publisher.GetRootName().c_str(), publisher.GetDataDirectory().c_str(), createParams.WantCompressed() ? "true" : "false");
            
    auto status = publisher.Publish();

    printStatus(status);
    return static_cast<int>(status);
    }

