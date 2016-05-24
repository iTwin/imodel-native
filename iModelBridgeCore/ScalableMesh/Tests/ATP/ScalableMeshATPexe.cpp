//#include "ScalableMeshATPPch.h"
#include "ScalableMeshATPexe.h"
#include <RasterSchema/RasterSchemaApi.h>
#include <RasterSchema/RasterFileHandler.h>
#include <RasterSchema/WmsHandler.h>
#include <ImagePP/all/h/HFCMacros.h>
#include <ImagePP\all\h\HRFFileFormats.h>
#include <ImagePP/all/h/HRFRasterFileFactory.h>
#include <ImagePP/IppImaging/HRFInternetImagingFile.h>
#include "Initialize.h"
#include "Common/ATPUtils.h"
#include "Common/ATPFileFinder.h"


USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_TERRAINMODEL
USING_NAMESPACE_IMAGEPP


namespace ScalableMeshATPexe
{
DgnPlatformLib::Host::GeoCoordinationAdmin&      ScalableMeshATPexe::_SupplyGeoCoordinationAdmin()
    {
    BeFileName geocoordinateDataPath(L".\\GeoCoordinateData\\");

    return *DgnGeoCoordinationAdmin::Create(geocoordinateDataPath);
    }





struct MyImageppLibHost : ImagePP::ImageppLib::Host
    {
    MyImageppLibHost();

    virtual ImagePP::ImageppLibAdmin&               _SupplyImageppLibAdmin() override;
    virtual void                                    _RegisterFileFormat() override;
    };

struct MyImageppLibAdmin : ImagePP::ImageppLibAdmin
    {
    DEFINE_T_SUPER(ImagePP::ImageppLibAdmin)

        virtual BentleyStatus                           _GetDefaultTempDirectory(BeFileName& tempFileName) const override;
    virtual BentleyStatus                           _GetLocalCacheDirPath(BeFileName& tempPath, bool checkForChange = false) const override;
    virtual BentleyStatus                           _GetGDalDataPath(WStringR gdalDataPath) const override;
    virtual BentleyStatus                           _GetECWDataPath(WStringR ecwDataPath) const override;
    virtual                                         ~MyImageppLibAdmin() {}
    };


static void callHostOnAssert(WCharCP _Message, WCharCP _File, unsigned _Line, BeAssertFunctions::AssertType)
    {
    BeAssertFunctions::DefaultAssertionFailureHandler(_Message, _File, _Line);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         05/2015
//-----------------------------------------------------------------------------------------
MyImageppLibHost::MyImageppLibHost()
    {
    BeAssertFunctions::SetBeAssertHandler(callHostOnAssert);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         05/2015
//-----------------------------------------------------------------------------------------
ImagePP::ImageppLibAdmin& MyImageppLibHost::_SupplyImageppLibAdmin()
    {
    return *new MyImageppLibAdmin();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         05/2015
//-----------------------------------------------------------------------------------------
void MyImageppLibHost::_RegisterFileFormat()
    {
    REGISTER_SUPPORTED_FILEFORMAT
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         07/2015
//-----------------------------------------------------------------------------------------
BentleyStatus MyImageppLibAdmin::_GetDefaultTempDirectory(BeFileName& tempFileName) const
    {
    // Return the temp directory name. The directory is created if it does not exist.
    T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(tempFileName, L"Raster");
    return BSISUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         07/2015
//-----------------------------------------------------------------------------------------
BentleyStatus MyImageppLibAdmin::_GetLocalCacheDirPath(BeFileName& tempPath, bool checkForChange) const
    {
    //If not empty and it exist, return it 
    if (!checkForChange && !m_localDirPath.IsEmpty() && BeFileName::IsDirectory(m_localDirPath.GetName()))
        {
        tempPath = m_localDirPath;
        return BSISUCCESS;
        }

    //Use default cache folder,m_localDirPath will be set by this call
    return T_Super::_GetLocalCacheDirPath(tempPath);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         07/2015
//-----------------------------------------------------------------------------------------
BentleyStatus MyImageppLibAdmin::_GetGDalDataPath(WStringR gdalDataPath) const
    {
    BeFileName path = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    path.AppendToPath(L"GDalData");

    // Convert BeFileName to WString
    BeFileName::BuildName(gdalDataPath, path.GetDevice().c_str(), path.GetDirectoryWithoutDevice().c_str(), path.GetFileNameWithoutExtension().c_str(), path.GetExtension().c_str());
    BeFileName::AppendSeparator(gdalDataPath);

    return BSISUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         07/2015
//-----------------------------------------------------------------------------------------
BentleyStatus MyImageppLibAdmin::_GetECWDataPath(WStringR ecwDataPath) const
    {
    BeFileName path = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();

    // Convert BeFileName to WString
    BeFileName::BuildName(ecwDataPath, path.GetDevice().c_str(), path.GetDirectoryWithoutDevice().c_str(), path.GetFileNameWithoutExtension().c_str(), path.GetExtension().c_str());
    BeFileName::AppendSeparator(ecwDataPath);

    return BSISUCCESS;
    }













BentleyStatus ScalableMeshATPexe::Initialize(int argc, WCharP argv[])
    {
    DgnViewLib::Initialize(*this, true); // this initializes the DgnDb libraries

    // Initialize RasterLib
    //DgnDomains::RegisterDomain(RasterSchema::RasterDomain::GetDomain());
    ImagePP::ImageppLib::Initialize(*new MyImageppLibHost());

    InitializeATP(*this);
    setlocale(LC_CTYPE, "");
    return SUCCESS;
    }


int ScalableMeshATPexe::PrintUsage(WCharCP programName)
    {
    WString exeName = BeFileName::GetFileNameAndExtension(programName);

    fwprintf(stderr,
             L"\n\
                 Run ATP for ScalableMesh.\n\
                 \n Usage: \n\
                %ls runatp -i|--input= -c|--clean\n\
                --input=                (required)  path to XML file for ATP or directory with xml files for ATP. \n\
                --clean                 (optional)  Delete stm file (if -i is a directory delete all stm files). \n\
                ", programName);

    return 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
WString ScalableMeshATPexe::GetArgValueW(WCharCP arg)
    {
    WString argValue(arg);
    argValue = argValue.substr(argValue.find_first_of('=', 0) + 1);
    argValue.Trim(L"\"");
    argValue.Trim();
    return argValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ScalableMeshATPexe::GetArgValue(WCharCP arg)
    {
    return Utf8String(GetArgValueW(arg));
    }

int ScalableMeshATPexe::ParseCommandLine(int argc, WCharP argv[])
    {
    if (argc < 2)
        return PrintUsage(argv[0]);
    bool isTestPlanMode = false;
    m_optionClean = false;
    if (0 == wcscmp(argv[1], L"runatp"))
        {
        isTestPlanMode = true;
        }
    if (!isTestPlanMode)
        {
        fwprintf(stderr, L"Unrecognized command: %ls\nTry 'runatp'\n.", argv[1]);
        return PrintUsage(argv[0]);
        }
    for (int iArg = 2; iArg < argc; ++iArg)
        {
        if (argv[iArg] == wcsstr(argv[iArg], L"--help") || argv[iArg] == wcsstr(argv[iArg], L"-h"))
            return PrintUsage(argv[0]);
        if (argv[iArg] == wcsstr(argv[iArg], L"--clean") || argv[iArg] == wcsstr(argv[iArg], L"-c"))
            {
            m_optionClean = true;
            continue;
            }
        if (argv[iArg] == wcsstr(argv[iArg], L"--input=") || argv[iArg] == wcsstr(argv[iArg], L"-i="))
            {
            BeFileName::FixPathName(m_inputFileName, GetArgValueW(argv[iArg]).c_str());
            if (!BeFileName::DoesPathExist(m_inputFileName.c_str()))
                {
                fwprintf(stderr, L"%ls is not an existing path\n", m_inputFileName.c_str());
                return PrintUsage(argv[0]);
                }

            continue;
            }

        fwprintf(stderr, L"Unrecognized command line option: %ls\n", argv[iArg]);
        return PrintUsage(argv[0]);
        }

    return SUCCESS;
    }

void ScalableMeshATPexe::Start()
    {
    // Run ATP
    if (BeFileName::IsDirectory(m_inputFileName.c_str()))
        {
        ATPFileFinder fileFinder;

        WString filePaths;

        fileFinder.FindFiles(m_inputFileName, filePaths, true);

        WString firstPath;

        while (fileFinder.ParseFilePaths(filePaths, firstPath))
            {
            BeFileName name(firstPath.c_str());
            WString extension;
            name.ParseName(NULL, NULL, NULL, &extension);
            if (0 == BeStringUtilities::Wcsicmp(extension.c_str(), L"xml"))
                {
                if (m_optionClean)
                    RemoveStmFiles(name);
                RunTestPlan(name);
                }

            }
        }
    else
        {
        if (m_optionClean)
            RemoveStmFiles(m_inputFileName);
        RunTestPlan(m_inputFileName);
        }
    }
}

int wmain(int argc, wchar_t* argv[])
    {
    _set_error_mode(_OUT_TO_MSGBOX);
    ScalableMeshATPexe::ScalableMeshATPexe app;
    if (SUCCESS != app.ParseCommandLine(argc, argv))
        return 1;

    app.Initialize(argc, argv);
    app.Start();

    ScalableMeshATPexe::CloseATP();

    return 0;
    }