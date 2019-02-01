#include "ScalableMeshWorker.h"


#include <DgnPlatform/DesktopTools/ConfigurationManager.h>
#include <ImagePP/all/h/HFCMacros.h>
#include <ImagePP\all\h\HRFFileFormats.h>
#include <ImagePP/all/h/HRFRasterFileFactory.h>
#include "Initialize.h"
#include "SMWorkerDefinitions.h"
#include "SMWorkerTaskScheduler.h"





USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_TERRAINMODEL
USING_NAMESPACE_IMAGEPP


//#define TRACE_ON


BEGIN_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE

DgnPlatformLib::Host::GeoCoordinationAdmin&      ScalableMeshWorker::_SupplyGeoCoordinationAdmin()
    {
    BeFileName geocoordinateDataPath(L".\\GeoCoordinateData\\");
#ifndef VANCOUVER_API  
    return *DgnGeoCoordinationAdmin::Create(geocoordinateDataPath);
#else
    return *DgnGeoCoordinationAdmin::Create(geocoordinateDataPath, IACSManager::GetManager());
#endif
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

#ifndef VANCOUVER_API
    virtual BentleyStatus                           _GetDefaultTempDirectory(BeFileName& tempFileName) const override;        
    virtual BentleyStatus                           _GetGDalDataPath(BeFileNameR gdalDataPath) const override;
    virtual BentleyStatus                           _GetECWDataPath(BeFileNameR ecwDataPath) const override;    
#else
    virtual IRasterGeoCoordinateServices*           _GetIRasterGeoCoordinateServicesImpl() const override;
#endif

    virtual BentleyStatus                           _GetLocalCacheDirPath(BeFileName& tempPath, bool checkForChange = false) const override;        
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
#ifndef VANCOUVER_API
BentleyStatus MyImageppLibAdmin::_GetDefaultTempDirectory(BeFileName& tempFileName) const
    {
    // Return the temp directory name. The directory is created if it does not exist.
    T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(tempFileName, L"Raster");
    return BSISUCCESS;
    }
#endif
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
#ifndef VANCOUVER_API
BentleyStatus MyImageppLibAdmin::_GetGDalDataPath(BeFileNameR gdalDataPath) const
    {
    BeFileName path = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    path.AppendToPath(L"GDal_Data");

    // Convert BeFileName to WString
    BeFileName::BuildName(gdalDataPath, path.GetDevice().c_str(), path.GetDirectoryWithoutDevice().c_str(), path.GetFileNameWithoutExtension().c_str(), path.GetExtension().c_str());
    BeFileName::AppendSeparator(gdalDataPath);

    return BSISUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         07/2015
//-----------------------------------------------------------------------------------------
BentleyStatus MyImageppLibAdmin::_GetECWDataPath(BeFileNameR ecwDataPath) const
    {
    BeFileName path = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();

    // Convert BeFileName to WString
    BeFileName::BuildName(ecwDataPath, path.GetDevice().c_str(), path.GetDirectoryWithoutDevice().c_str(), path.GetFileNameWithoutExtension().c_str(), path.GetExtension().c_str());
    BeFileName::AppendSeparator(ecwDataPath);

    return BSISUCCESS;
    }

#endif

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre   08/2017
//-----------------------------------------------------------------------------------------
#ifdef VANCOUVER_API
IRasterGeoCoordinateServices* MyImageppLibAdmin::_GetIRasterGeoCoordinateServicesImpl() const 
    {
    if (GeoCoordinationManager::GetServices() != NULL)
        return ImageppLib::GetDefaultIRasterGeoCoordinateServicesImpl();

    return NULL;
    }
#endif

BentleyStatus ScalableMeshWorker::Initialize(int argc, WCharP argv[])
    {
#ifdef VANCOUVER_API
    DgnViewLib::Initialize(*this, true); // this initializes the DgnDb libraries
#else
    DgnPlatformLib::Initialize(*this, true);    
#endif	

    // Initialize RasterLib
    //DgnDomains::RegisterDomain(RasterSchema::RasterDomain::GetDomain());
    ImagePP::ImageppLib::Initialize(*new MyImageppLibHost());

    InitializeWorker(*this);
    setlocale(LC_CTYPE, "");
    return SUCCESS;
    }


int ScalableMeshWorker::PrintUsage(WCharCP programName)
    {
    WString exeName = BeFileName::GetFileNameAndExtension(programName);

    fwprintf(stderr,
             L"\n\
                 ScalableMesh Worker Application For Cloud Computing.\n\
                 \n Usage: \n\
                %ls -taskFolder=|tf= -startingIndexTask=|it= -nbExtraWorkers=|ew= -h|-help\n\
                -taskFolder=  (required) path to folder containing the task definitions. \n\
                -startingIndexTask=  (optional) absolute path to xml file containing the indexing task definition. \n\
                                     When defined the taskFolder will be cleanup and the index task xml file copy to the taskFolder prior to any task processing\n\
                -nbExtraWorkers=  (optional) spawn extra workers when setup. \n\
                --help  (optional) print usage. \n\
                ", programName);

    return 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
WString ScalableMeshWorker::GetArgValueW(WCharCP arg)
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
Utf8String ScalableMeshWorker::GetArgValue(WCharCP arg)
    {
    return Utf8String(GetArgValueW(arg));
    }

int ScalableMeshWorker::ParseCommandLine(int argc, WCharP argv[]) 
    {
    if (argc < 2)
        return PrintUsage(argv[0]);
            
    for (int iArg = 1; iArg < argc; ++iArg)
        {
        if (argv[iArg] == wcsstr(argv[iArg], L"-help") || argv[iArg] == wcsstr(argv[iArg], L"-h"))
            return PrintUsage(argv[0]);        
        if (argv[iArg] == wcsstr(argv[iArg], L"-taskFolder=") || argv[iArg] == wcsstr(argv[iArg], L"-tf="))
            {
            BeFileName::FixPathName(m_taskFolderName, GetArgValueW(argv[iArg]).c_str());
            if (!BeFileName::DoesPathExist(m_taskFolderName.c_str()))
                {
                fwprintf(stderr, L"%ls is not an existing folder\n", m_taskFolderName.c_str());
                return PrintUsage(argv[0]);
                }

            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"-nbExtraWorkers=") || argv[iArg] == wcsstr(argv[iArg], L"-ew="))
            {
            m_nbExtraWorkers = BeStringUtilities::Wtoi(GetArgValueW(argv[iArg]).c_str());
            m_workerProcessName = Utf8String(argv[0]);
            continue;
            }
        
        if (argv[iArg] == wcsstr(argv[iArg], L"-startingIndexTask=") || argv[iArg] == wcsstr(argv[iArg], L"-it="))
            {
            BeFileName::FixPathName(m_startingIndexTask, GetArgValueW(argv[iArg]).c_str());
            if (!BeFileName::DoesPathExist(m_startingIndexTask.c_str()))
                {
                fwprintf(stderr, L"%ls is not an existing file\n", m_startingIndexTask.c_str());
                return PrintUsage(argv[0]);
                }

            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"-useGroupingStrategy") || argv[iArg] == wcsstr(argv[iArg], L"-gr"))
            {
            m_useGroupingStrategy = true;            
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"-groupingSize=") || argv[iArg] == wcsstr(argv[iArg], L"-gs="))
            {
            m_groupingSize = BeStringUtilities::Wtoi(GetArgValueW(argv[iArg]).c_str());            
            continue;
            }                        

        if (argv[iArg] == wcsstr(argv[iArg], L"-startAsService") || argv[iArg] == wcsstr(argv[iArg], L"-ss"))
            {
            m_startAsService = true;            
            continue;
            }

        if (argv[iArg] == wcsstr(argv[iArg], L"-resultFolder=") || argv[iArg] == wcsstr(argv[iArg], L"-rf="))
            {
            BeFileName::FixPathName(m_resultFolderName, GetArgValueW(argv[iArg]).c_str());
            if (!BeFileName::DoesPathExist(m_resultFolderName.c_str()))
                {
                fwprintf(stderr, L"%ls is not an existing folder\n", m_resultFolderName.c_str());
                return PrintUsage(argv[0]);
                }

            continue;
            }

               
        fwprintf(stderr, L"Unrecognized command line option: %ls\n", argv[iArg]);
        return PrintUsage(argv[0]);
        }

    return SUCCESS;
    }

//ScalableMeshStep

void ScalableMeshWorker::Start()
    {

#ifdef TRACE_ON	
    BeFileNameStatus fileStatus = BeFileName::EmptyDirectory(L"D:\\MyDoc\\RMA - July\\CloudWorker\\Log\\");
    assert(fileStatus == BeFileNameStatus::Success);
#endif	    

    if (m_startingIndexTask.size() > 0)
        {
        BeFileNameStatus fileStatus = BeFileName::EmptyDirectory(m_taskFolderName.c_str());
        assert(fileStatus == BeFileNameStatus::Success);

        BeFileName newFileName(m_taskFolderName);
        newFileName.AppendString(m_startingIndexTask.GetFileNameAndExtension().c_str());

        fileStatus = BeFileName::BeCopyFile(m_startingIndexTask, newFileName, true);
        assert(fileStatus == BeFileNameStatus::Success);
        }

    if (m_nbExtraWorkers > 0)
        {        
        Utf8String cmdStr(m_workerProcessName);                
        cmdStr.append(Utf8PrintfString(" -taskFolder=\"%s\\\"", Utf8String(m_taskFolderName.c_str()).c_str()));        
        cmdStr.append(Utf8PrintfString(" -rf=\"%s\\\"", Utf8String(m_resultFolderName.c_str()).c_str()));
        
        for (uint16_t taskId = 0; taskId < m_nbExtraWorkers; taskId++)
            {
            STARTUPINFOA info;
            PROCESS_INFORMATION processInfo;
            ZeroMemory(&info, sizeof(info));
            info.cb = sizeof(info);
            ZeroMemory(&processInfo, sizeof(processInfo));

            if (!CreateProcessA(NULL, (LPSTR)cmdStr.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, /*(void*)env.c_str()*/ NULL, NULL, &info, &processInfo))
                {
                assert(!"Cannot create process");
                }
            }
        }


    WString cfgValue;
    uint16_t nbWorkers = 1;

    if (SUCCESS == ConfigurationManager::GetVariable(cfgValue, L"SM_WORKER_NB_WORKERS"))
        {
        uint64_t value;

        BentleyStatus status = BeStringUtilities::ParseUInt64(value, Utf8String(cfgValue.c_str()).c_str());

        if (status == SUCCESS)
            {
            nbWorkers = (uint16_t)value;
            }
        }    

    TaskScheduler taskScheduler(m_taskFolderName, nbWorkers, m_useGroupingStrategy, m_groupingSize, m_startAsService, m_resultFolderName);

    taskScheduler.Start();
    }

END_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE


USING_NAMESPACE_BENTLEY_SCALABLEMESH_WORKER

int wmain(int argc, wchar_t* argv[])
    {
    _set_error_mode(_OUT_TO_MSGBOX);


    ScalableMeshWorker app;

    if (SUCCESS != app.ParseCommandLine(argc, argv))
        return 1;

    app.Initialize(argc, argv);
    app.Start();

    //ScalableMeshWorker::CloseATP();

    return 0;
    }