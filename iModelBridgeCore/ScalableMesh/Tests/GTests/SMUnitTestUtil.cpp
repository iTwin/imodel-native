/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/GTests/SMUnitTestUtil.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "SMUnitTestUtil.h"
#include <Bentley/BeFile.h>
#include <Bentley/BeDirectoryIterator.h>
#ifndef VANCOUVER_API
#include <Bentley/Desktop/FileSystem.h>
#endif

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
bvector<BeFileName> ScalableMeshGTestUtil::GetFiles(BeFileName dataPath, bool wantDataSource)
    {
    bool pathExists = GetDataPath(dataPath);

    bvector<BeFileName> fileList;
    if (pathExists)
        {
        BeFileName entryName;
        bool        isDir;
        for (BeDirectoryIterator dirIter(dataPath); dirIter.GetCurrentEntry(entryName, isDir) == SUCCESS; dirIter.ToNext())
            {
            if (FilterEntry(entryName, isDir, wantDataSource)) fileList.push_back(entryName);
            }
        }
    return fileList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                   12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ScalableMeshGTestUtil::GetFileCount(BeFileName dataPath)
    {
    size_t fileCount = 0;
    if (BeFileName::DoesPathExist(dataPath))
        {
        BeFileName entryName;
        bool        isDir;
        for (BeDirectoryIterator dirIter(dataPath); dirIter.GetCurrentEntry(entryName, isDir) == SUCCESS; dirIter.ToNext())
            {
            if (!isDir && BeFileName::DoesPathExist(entryName))
                {
                auto const& fileType = GetFileType(entryName);
                if (SMMeshType::TYPE_3DTILES_TILESET == fileType || SMMeshType::TYPE_3DTILES_B3DM == fileType)
                    {
                    ++fileCount;
                    }
                }
            }
        }
    return fileCount;
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
        type = SMMeshType::TYPE_3DTILES_TILESET;
        }
    else if (extension == L"b3dm")
        {
        type = SMMeshType::TYPE_3DTILES_B3DM;
        }
    else if (extension == L"xyz")
        {
        type = SMMeshType::TYPE_3SM_SOURCE;
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
bool ScalableMeshGTestUtil::FilterEntry(BeFileName& entry, bool isDir, bool wantDataSource)
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
        if (!BeFileName::DoesPathExist(entry))
            {
            return false;                        
            }

        SMMeshType fileType = GetFileType(entry);

        if (SMMeshType::TYPE_3SM == fileType && !wantDataSource)
            { 
            return true;
            }

        if (SMMeshType::TYPE_3SM_SOURCE == fileType && wantDataSource)
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
#ifdef VANCOUVER_API
    BeFileName::BeGetTempPath(tempPath);
#else
    Desktop::FileSystem::BeGetTempPath(tempPath);
#endif
    tempPath.AppendToPath(L"SMGTestOutput");

    return tempPath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                   10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName ScalableMeshGTestUtil::GetTempPathFromProjectPath(const BeFileName& path)
    {
    BeFileName extraFileDir;

#ifndef VANCOUVER_API
    Desktop::FileSystem::BeGetTempPath(extraFileDir);
#else
    BeFileName::BeGetTempPath(extraFileDir);
#endif

    WString substrFile = path.c_str();
    substrFile.ReplaceAll(L"/", L"_");
    substrFile.ReplaceAll(L"\\", L"_");
    substrFile.ReplaceAll(L":", L"_");
    substrFile.ReplaceAll(L"\"", L"_");
    substrFile.ReplaceAll(L"'", L"_");

    extraFileDir.AppendToPath(substrFile.c_str());
    return extraFileDir;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Elenie.Godzaridis                     11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<std::tuple<BeFileName, DMatrix4d, bvector<DPoint3d>, bvector<DPoint3d>>> ScalableMeshGTestUtil::GetListOfValues(BeFileName listingFile)
{
    std::ifstream f;
    bvector<std::tuple<BeFileName, DMatrix4d, bvector<DPoint3d>, bvector<DPoint3d>>> resultList;
    if (!ScalableMeshGTestUtil::GetDataPath(listingFile))
        return resultList;
    f.open(listingFile.c_str());
    if (f.fail())
        return resultList;
    while (!f.eof())
    {
        std::string nameStr;
        f >> nameStr;

        DMatrix4d mat;
        mat.InitIdentity();
        for (size_t i = 0; i < 3; ++i)
            for (size_t j = 0; j < 4; ++j)
                f >> mat.coff[i][j];

        int nOfSourcePts;
        f >> nOfSourcePts;
        bvector<DPoint3d> sourcePts;
        for (size_t i = 0; i < nOfSourcePts; ++i)
        {
            DPoint3d pt;
            f >> pt.x;
            f >> pt.y;
            f >> pt.z;
            sourcePts.push_back(pt);
        }

        int nOfResultPts;
        f >> nOfResultPts;
        bvector<DPoint3d> resultPts;
        for (size_t i = 0; i < nOfResultPts; ++i)
        {
            DPoint3d pt;
            f >> pt.x;
            f >> pt.y;
            f >> pt.z;
            resultPts.push_back(pt);
        }

        BeFileName name;
        ScalableMeshGTestUtil::GetDataPath(name);
        name.AppendToPath(SM_DATA_PATH);
        name.AppendToPath(WString(nameStr.c_str(), true).c_str());
        std::tuple<BeFileName, DMatrix4d, bvector<DPoint3d>, bvector<DPoint3d>> entries(name, mat, sourcePts, resultPts);
        resultList.push_back(entries);
    }
    return resultList;
}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Richard.Bois                     12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value ScalableMeshGTestUtil::GetGroundTruthJsonFile(BeFileName jsonFile)
    {
    Json::Value result;

    BeFile f;
#ifdef VANCOUVER_API
    if (BeFileStatus::Success != f.Open(jsonFile.c_str(), BeFileAccess::Read, BeFileSharing::None))
#else
    if (BeFileStatus::Success != f.Open(jsonFile.c_str(), BeFileAccess::Read))
#endif
        return Json::Value();

    bvector<byte> fileBuffer;
    if (BeFileStatus::Success != f.ReadEntireFile(fileBuffer))
        return Json::Value();

    if (!Json::Reader().parse(reinterpret_cast<char *>(fileBuffer.data()), reinterpret_cast<char *>(fileBuffer.data() + fileBuffer.size()), result))
        return Json::Value();

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Mathieu.St-Pierre                       11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<std::tuple<BeFileName, DMatrix4d, bvector<DPoint4d>, bvector<double>>> ScalableMeshGTestUtil::GetListOfDisplayQueryValues(BeFileName listingFile)
    {
    std::ifstream f;
    bvector<std::tuple<BeFileName, DMatrix4d, bvector<DPoint4d>, bvector<double>>> resultList;
    if (!ScalableMeshGTestUtil::GetDataPath(listingFile))
        return resultList;
    f.open(listingFile.c_str());
    if (f.fail())
        return resultList;
    while (!f.eof())
        {
        std::string nameStr;
        f >> nameStr;

        DMatrix4d mat;
        mat.InitIdentity();
        for (size_t i = 0; i <4; ++i)
            for (size_t j = 0; j <4; ++j)
                f >> mat.coff[i][j];

        int nOfClipPlanes;
        f >> nOfClipPlanes;

        bvector<DPoint4d> clipPlanes;
        for (size_t i = 0; i < nOfClipPlanes; ++i)
            {
            DPoint4d pt;
            f >> pt.x;
            f >> pt.y;
            f >> pt.z;
            f >> pt.w;
            clipPlanes.push_back(pt);
            }

        
        /*
        int nOfResultPts;
        f >> nOfResultPts;
        */
        bvector<double> results(1);
        //bvector<DPoint3d> resultPts;
        for (size_t i = 0; i < 1; ++i)
            {
            f >> results[i];
            /*
            DPoint3d pt;
            f >> pt.x;  
            f >> pt.y;
            f >> pt.z;
            resultPts.push_back(pt);*/
            }

        BeFileName name;
        ScalableMeshGTestUtil::GetDataPath(name);
        name.AppendToPath(SM_DATA_PATH);
        name.AppendToPath(WString(nameStr.c_str(), true).c_str());
        std::tuple<BeFileName, DMatrix4d, bvector<DPoint4d>, bvector<double>> entries(name, mat, clipPlanes, results);
        resultList.push_back(entries);
        }
    return resultList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Mathieu.St-Pierre                       03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<std::tuple<BeFileName, bvector<DPoint3d>, uint64_t>> ScalableMeshGTestUtil::GetListOfGroundDetectionValues(BeFileName listingFile)
    {   
    std::ifstream f;
    bvector<std::tuple<BeFileName, bvector<DPoint3d>, uint64_t>> resultList;
    if (!ScalableMeshGTestUtil::GetDataPath(listingFile))
        return resultList;
    f.open(listingFile.c_str());
    if (f.fail())
        return resultList;
    while (!f.eof())
        {
        std::string nameStr;
        f >> nameStr;

        int nOfAreaPts;
        f >> nOfAreaPts;

        bvector<DPoint3d> groundArea;
        for (size_t i = 0; i < nOfAreaPts; ++i)
            {
            DPoint3d pt;
            f >> pt.x;
            f >> pt.y;
            f >> pt.z;            
            groundArea.push_back(pt);
            }    

        uint64_t expectedGroundPts;
        f >> expectedGroundPts;

        BeFileName name;
        ScalableMeshGTestUtil::GetDataPath(name);
        name.AppendToPath(SM_GROUND_DETECTION_PATH);
        name.AppendToPath(WString(nameStr.c_str(), true).c_str());
        std::tuple<BeFileName, bvector<DPoint3d>, uint64_t> entries(name, groundArea, expectedGroundPts);
        resultList.push_back(entries);
        }

    return resultList;
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
        static ScalableMeshModule smApp;
        if (SUCCESS == smApp.Initialize())
            bInitialized = true;
        }

    return bInitialized;
    };


#ifdef VANCOUVER_API
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
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                   10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatformLib::Host::GeoCoordinationAdmin& ScalableMeshModule::_SupplyGeoCoordinationAdmin()
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                   10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ScalableMeshModule::Initialize()
    {
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

#ifdef VANCOUVER_API
    DgnViewLib::Initialize(*this, true); // this initializes the DgnDb libraries
#else
    DgnPlatformLib::Initialize(*this, true);
#endif	
    SMHost* smHost = new SMHost();
    ScalableMesh::ScalableMeshLib::Initialize(*smHost);

    if (smHost->m_isScalableMeshAdminSupplied == false)
        return ERROR;

                                         // Initialize RasterLib
                                         //DgnDomains::RegisterDomain(RasterSchema::RasterDomain::GetDomain());
                                         //ImagePP::ImageppLib::Initialize(*new MyImageppLibHost());
    _SupplyGeoCoordinationAdmin()._GetServices();
    setlocale(LC_CTYPE, "");
    return SUCCESS;
    }

