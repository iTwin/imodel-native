/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <windows.h> // For GetModuleFileNameW
#include <DgnPlatform/DgnPlatformLib.h>
#include <ScalableMesh/ScalableMeshLib.h>

#ifndef VANCOUVER_API   
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>
#else
#include <DgnGeoCoord/DgnGeoCoord.h>

using namespace Bentley::GeoCoordinates;
#endif

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SCALABLEMESH

#ifndef VANCOUVER_API
#define BEFILENAME(function, filename) filename.function()
#else
#define BEFILENAME(function, filename) BeFileName::function(filename.c_str())
#endif

#ifdef WIN32
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                   10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName GetModuleFilePath()
    {
    wchar_t wccwd[FILENAME_MAX];
    GetModuleFileNameW(nullptr, &wccwd[0], (DWORD)FILENAME_MAX);
    return BeFileName(wccwd);
    }

#else
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nicolas.Beland                   07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName GetModuleFilePath()
    {
    char path[PATH_MAX + 1];
    char dest[PATH_MAX + 1];
    memset(dest, 0, sizeof(dest)); // readlink does not null terminate!
    struct stat info;
    pid_t pid = getpid();
    sprintf(path, "/proc/%d/exe", pid);
    size_t num_bytes = readlink(path, dest, PATH_MAX);
    if(num_bytes == -1 || num_bytes == 0 || num_bytes > PATH_MAX)
        {
        std::cout << "num_bytes == 0" << std::endl;
        return BeFileName(L"");
        }
    else
        {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        std::wstring path_str = conv.from_bytes(dest);
        return BeFileName(path_str.c_str());
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                   10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName GetModuleFileDirectory()
    {
    return BeFileName(BeFileName::GetDirectoryName(GetModuleFilePath()).c_str());
    }

struct ScalableMeshApp : DgnPlatformLib::Host
    {
    protected:
        enum class ParseStatus {
            Success, Error, NotRecognized
            };

        virtual DgnPlatformLib::Host::GeoCoordinationAdmin& _SupplyGeoCoordinationAdmin()
            {
            BeFileName geocoordinateDataPath = GetModuleFileDirectory();
#ifndef VANCOUVER_API  
            geocoordinateDataPath.AppendToPath(L"Assets/DgnGeoCoord");
            return *DgnGeoCoordinationAdmin::Create(geocoordinateDataPath);
#else
            geocoordinateDataPath.AppendToPath(L"GeoCoordinateData");
            return *DgnGeoCoordinationAdmin::Create(geocoordinateDataPath, IACSManager::GetManager());
#endif
            }

#ifndef VANCOUVER_API   
        virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin()
            override
            {
            return *new BentleyApi::Dgn::KnownDesktopLocationsAdmin();
            }
        virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() {
            return BeSQLite::L10N::SqlangFiles(BeFileName());
            }
        virtual void _SupplyProductName(Utf8StringR name) override {
            name.assign("ScalableMeshUnitTests");
            }
#else
        virtual void _SupplyProductName(WStringR name) override {
            name.assign(L"ScalableMeshUnitTests");
            }
#endif


        BeFileName          m_inputFileName;
        BeFileName          m_outputFileName;

        WString GetArgValueW(WCharCP arg)
            {
            WString argValue(arg);
            argValue = argValue.substr(argValue.find_first_of('=', 0) + 1);
            argValue.Trim(L"\"");
            argValue.Trim();
            return argValue;
            }

        Utf8String GetArgValue(WCharCP arg)
            {
            return Utf8String(GetArgValueW(arg));
            }


    public:

        int PrintUsage(WCharCP programName)
            {
            WString exeName = BeFileName::GetFileNameAndExtension(programName);

            fwprintf(stderr,
                     L"\n\
                 Run 3MX to 3SM Conversion.\n\
                 \n Usage: \n\
                %ls -i|--input= -o|--output=\n\
                --input=                (required)  path to input 3MX file. \n\
                --output=               (optional)  path to output 3sm file. \n\
                ", programName);

            return 1;
            }

        int ParseCommandLine(int argc, WCharP argv[])
            {
            if(argc < 1)
                return PrintUsage(argv[0]);
            for(int iArg = 1; iArg < argc; ++iArg)
                {
                if(argv[iArg] == wcsstr(argv[iArg], L"--help") || argv[iArg] == wcsstr(argv[iArg], L"-h"))
                    return PrintUsage(argv[0]);
                if(argv[iArg] == wcsstr(argv[iArg], L"--input=") || argv[iArg] == wcsstr(argv[iArg], L"-i="))
                    {
                    BeFileName::FixPathName(m_inputFileName, GetArgValueW(argv[iArg]).c_str());
                    if(!BeFileName::DoesPathExist(m_inputFileName.c_str()))
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

        BentleyStatus Initialize(int argc, WCharP argv[])
            {
#ifdef IMODEL02
            DgnPlatformLib::Initialize(*this);
#elif defined(VANCOUVER_API)
            DgnViewLib::Initialize(*this, true); // this initializes the DgnDb libraries
#else
            DgnPlatformLib::Initialize(*this, true);
#endif	

            struct SMHost : public BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshLib::Host
                {
                bool m_isScalableMeshAdminSupplied = false;
                SMHost()
                    {}
                protected:
                    ScalableMesh::ScalableMeshAdmin& _SupplyScalableMeshAdmin()
                        {
                        m_isScalableMeshAdminSupplied = true;
                        return *new ScalableMesh::ScalableMeshAdmin(); // delete will be hopefully called by ScalableMeshAdmin::_OnHostTermination
                        };
                };

            SMHost* smHost = new SMHost();
            ScalableMesh::ScalableMeshLib::Initialize(*smHost);

            if(smHost->m_isScalableMeshAdminSupplied == false)
                return ERROR;

            _SupplyGeoCoordinationAdmin()._GetServices();

            setlocale(LC_CTYPE, "");

            if(SUCCESS != ParseCommandLine(argc, argv))
                return ERROR;

            return SUCCESS;
            }

        void Start()
            {
            bool readOnly = true;
            bool shareable = false;
            StatusInt status;
            ScalableMesh::IScalableMeshPtr myScalableMesh = ScalableMesh::IScalableMesh::GetFor(m_inputFileName, readOnly, shareable, status);
            BeAssert(status == SUCCESS);
            std::cout << "Start IntersectRay" << std::endl;
            auto drapePoint = myScalableMesh->GetDTMInterface(DTMAnalysisType::ViewOnly)->GetDTMDraping();
            DPoint3d startPt = DPoint3d::From(538315.036730, 4702307.270700, 345.137888);
            DVec3d direction = DVec3d::From(0.407905, -0.768806, -0.492495);

            DPoint3d result;
            bool succeeded = drapePoint->IntersectRay(result, direction, startPt);
            BeAssert(succeeded); // No intersection!
            std::cout << "End IntersectRay. Succeeded? " <<(succeeded? "YES" : "NO")<< std::endl;
            std::cout << "Point found: " << result.x << ", " << result.y << ", " << result.z;
            }
    };


int wmain(int argc, wchar_t* argv[])
    {
    ScalableMeshApp app;
    if(SUCCESS != app.Initialize(argc, argv))
        return 1;

    app.Start();
    return 0;
    }
