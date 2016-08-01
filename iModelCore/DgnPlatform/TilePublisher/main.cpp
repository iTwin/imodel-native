/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/main.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TilePublisher.h"

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
        if (nullptr == equalPos)
            return;

        WCharCP argValue = equalPos+1;

        auto paramNameLen = equalPos - raw;
        if (0 == paramNameLen || (!verboseParamName && 1 != paramNameLen))
            return;

        for (size_t i = 0; i < s_paramTableSize; i++)
            {
            auto const& param = s_paramTable[i];
            WCharCP paramName = verboseParamName ? param.m_verbose : param.m_abbrev;
            if (0 != wcsncmp(raw, paramName, paramNameLen) || paramNameLen != wcslen(paramName))
                continue;

            m_paramId = static_cast<ParamId>(i);
            m_value = argValue;
            m_value.Trim(L"\"");
            m_value.Trim();
            break;
            }
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
static bool parseArgs(Publisher::CreateParams& params, int ac, wchar_t const** av)
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
                BeFileName::FixPathName(params.m_inputFileName, arg.m_value.c_str());
                break;
            case ParamId::View:
                params.m_viewName = Utf8String(arg.m_value.c_str());
                break;
            case ParamId::Output:
                BeFileName::FixPathName(params.m_outputDir, arg.m_value.c_str());
                break;
            case ParamId::Name:
                params.m_tilesetName = Utf8String(arg.m_value.c_str());
                break;
            default:
                return false;
            }
        }

    if (!haveInput)
        return false;

    if (params.m_outputDir.empty())
        params.m_outputDir = params.m_inputFileName.GetDirectoryName();

    if (params.m_tilesetName.empty())
        params.m_tilesetName = Utf8String(params.m_inputFileName.GetFileNameWithoutExtension().c_str());

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
static void printStatus(Publisher::Status status)
    {
    static const Utf8CP s_msg[] =
        {
        "Publishing succeeded",
        "No geometry to publish",
        "Publishing aborted",
        "Failed to open input .bim",
        "Failed to open view",
        "Failed to write to base directory",
        "Failed to create subdirectory",
        "Failed to write scene",
        "Failed to write node"
        };

    auto index = static_cast<uint32_t>(status);
    Utf8CP msg = index < _countof(s_msg) ? s_msg[index] : "Unrecognized error";
    printf("Result: %hs.\n", msg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
int wmain(int ac, wchar_t const** av)
    {
    Publisher::CreateParams createParams;
    if (!parseArgs(createParams, ac, av))
        {
        printUsage(av[0]);
        return 1;
        }

    // ###TODO: stuff...
    printf("Publishing view %hs from file %ls to %ls%hs.html\n", createParams.m_viewName.c_str(), createParams.m_inputFileName.c_str(), createParams.m_outputDir.c_str(), createParams.m_tilesetName.c_str());

    Publisher publisher(createParams);
    auto status = publisher.Publish();

    printStatus(status);
    return static_cast<int>(status);
    }

