/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/GTests/SMUnitTestUtil.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "SMUnitTestUtil.h"
#include <Bentley/BeDirectoryIterator.h>

using namespace ScalableMeshGTestUtil;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                   10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScalableMeshGTestUtil::GetDataPath(BeFileName& dataPath)
    {
#ifdef OVERRIDE_TEST_DATA_DIR
    BeFileName dataDir = dataPath;
#else
    BeFileName dataDir = ScalableMeshGTestUtil::GetModuleFileDirectory();
    dataDir.AppendToPath(dataPath);
#endif
    dataPath = dataDir;
    return BeFileName::DoesPathExist(dataDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                   10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<BeFileName> ScalableMeshGTestUtil::GetFiles(BeFileName dataPath)
    {
    bool pathExists = GetDataPath(dataPath);

    bvector<BeFileName> fileList;
    if (pathExists)
        {
        BeFileName entryName;
        bool        isDir;
        for (BeDirectoryIterator dirIter(dataPath); dirIter.GetCurrentEntry(entryName, isDir) == SUCCESS; dirIter.ToNext())
            {
            if (FilterEntry(entryName, isDir)) fileList.push_back(entryName);
            }
        }
    return fileList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                   10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SMMeshType ScalableMeshGTestUtil::GetFileType(BeFileName file)
    {
    SMMeshType type = SMMeshType::TYPE_UNKNOWN;
    auto extension = BeFileName::GetExtension(file);
    if (extension == L"3sm")
        {
        type = SMMeshType::TYPE_3SM;
        }
    else if (extension == L"json")
        {
        type = SMMeshType::TYPE_3DTILES;
        }
    else
        {
        BeAssert(!"Unknown file type for ScalableMesh");
        }
    return type;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                   10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScalableMeshGTestUtil::FilterEntry(BeFileName& entry, bool isDir)
    {
    if (isDir)
        {
        auto filename = BeFileName::GetFileNameWithoutExtension(entry.c_str());
        entry.AppendToPath(filename.c_str());
        entry += L".json";
        if (BeFileName::DoesPathExist(entry)) return true;
        }
    else
        {
        if (BeFileName::DoesPathExist(entry) && SMMeshType::TYPE_3SM == GetFileType(entry))
            {
            return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                   10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName ScalableMeshGTestUtil::GetModuleFilePath()
    {
    WCHAR wccwd[FILENAME_MAX];
    GetModuleFileNameW(nullptr, &wccwd[0], (DWORD)FILENAME_MAX);
    return BeFileName(wccwd);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                   10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName ScalableMeshGTestUtil::GetModuleFileDirectory()
    {
    return BeFileName(BeFileName::GetDirectoryName(GetModuleFilePath()).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                   10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName ScalableMeshGTestUtil::GetUserSMTempDir()
    {
    BeFileName tempPath;
    BeFileName::BeGetTempPath(tempPath);
    tempPath.AppendToPath(L"SMGTestOutput");

    return tempPath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                   10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScalableMeshGTestUtil::InitScalableMesh()
    {
    // Do nothing if Scalable Mesh was already initialized
    static bool bInitialized = false;
    if (!bInitialized)
        {
        ScalableMeshModule().Initialize();
        bInitialized = true;
        }

    return true;
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                   10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
VIEWMANAGER& ScalableMeshModule::_SupplyViewManager()
    {
    struct ExeViewManager : VIEWMANAGER
        {
        protected:
#ifndef VANCOUVER_API   
            virtual DgnDisplay::QvSystemContextP _GetQvSystemContext() override { return nullptr; }
#else
            virtual Bentley::DgnPlatform::DgnDisplayCoreTypes::WindowP _GetTopWindow(int) override { return nullptr; }
            virtual int                                                _GetCurrentViewNumber() override { return 0; }
            virtual HUDManager*                                        _GetHUDManager() { return nullptr; }
#endif

            virtual bool                _DoesHostHaveFocus()        override { return true; }
            virtual IndexedViewSetR     _GetActiveViewSet()         override { return *(IndexedViewSetP)nullptr; }
            virtual int                 _GetDynamicsStopInterval()  override { return 200; }

        public:
            ExeViewManager() {}
            ~ExeViewManager() {}
        };
    return *new ExeViewManager();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                   10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatformLib::Host::GeoCoordinationAdmin& ScalableMeshModule::_SupplyGeoCoordinationAdmin()
    {
    BeFileName geocoordinateDataPath = GetModuleFileDirectory();
    geocoordinateDataPath.AppendToPath(L"GeoCoordinateData");
#ifndef VANCOUVER_API  
    return *DgnGeoCoordinationAdmin::Create(geocoordinateDataPath);
#else
    return *DgnGeoCoordinationAdmin::Create(geocoordinateDataPath, IACSManager::GetManager());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                   10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ScalableMeshModule::Initialize()
    {
    struct SMHost : public Bentley::ScalableMesh::ScalableMeshLib::Host
        {
        SMHost()
            {}
        protected:
            ScalableMesh::ScalableMeshAdmin& _SupplyScalableMeshAdmin()
                {
                return *new ScalableMesh::ScalableMeshAdmin(); // delete will be hopefully called by ScalableMeshAdmin::_OnHostTermination
                };
        };
    ScalableMesh::ScalableMeshLib::Initialize(*new SMHost());

    DgnViewLib::Initialize(*this, true); // this initializes the DgnDb libraries

                                         // Initialize RasterLib
                                         //DgnDomains::RegisterDomain(RasterSchema::RasterDomain::GetDomain());
                                         //ImagePP::ImageppLib::Initialize(*new MyImageppLibHost());
    _SupplyGeoCoordinationAdmin()._GetServices();
    setlocale(LC_CTYPE, "");
    return SUCCESS;
    }

