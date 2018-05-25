//#include "ScalableMeshATPPch.h"
#include "ScalableMeshWorker.h"
#include "SMWorkerTaskScheduler.h"

#include <Bentley\BeDirectoryIterator.h>
#include <BeXml\BeXml.h>

#include <ImagePP/all/h/HFCMacros.h>
#include <ImagePP\all\h\HRFFileFormats.h>
#include <ImagePP/all/h/HRFRasterFileFactory.h>
#include "Initialize.h"

#include <ScalableMesh\IScalableMeshCreator.h>
#include <ScalableMesh\IScalableMeshSources.h>
#include <ScalableMesh\IScalableMeshSourceCollection.h>
#include <ScalableMesh\Import\ScalableMeshData.h>
#include <ScalableMesh\IScalableMeshSourceImportConfig.h>
#include <ScalableMesh\IScalableMeshPolicy.h>


#include "SMWorkerDefinitions.h"


USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT

USING_NAMESPACE_BENTLEY_TERRAINMODEL
USING_NAMESPACE_IMAGEPP



BEGIN_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE


/*---------------------------------------------------NEEDS_WORK_MST : Duplicate from ATP Code-----------------------------------------------------------*/


void ParseDTMFeatureType(WString& name, DTMFeatureType& type)
{
    if (0 == _wcsicmp(L"Void", name.c_str())) type = DTMFeatureType::Void;
    else if (0 == _wcsicmp(L"DrapeVoid", name.c_str())) type = DTMFeatureType::DrapeVoid;
    else if (0 == _wcsicmp(L"Breakline", name.c_str())) type = DTMFeatureType::Breakline;
    else if (0 == _wcsicmp(L"Hole", name.c_str())) type = DTMFeatureType::Hole;
    else if (0 == _wcsicmp(L"Island", name.c_str())) type = DTMFeatureType::Island;
    else if (0 == _wcsicmp(L"ContourLine", name.c_str())) type = DTMFeatureType::ContourLine;
}


bool AddOptionToSource(IDTMSourcePtr srcPtr, BeXmlNodeP pTestChildNode)
{
    WString datasetIs3D;
    WString datasetIsGround;

    StatusInt status = pTestChildNode->GetAttributeStringValue(datasetIs3D, "is3D");

    if (status == BEXML_Success)
    {
        if (datasetIs3D.Equals(L"1"))
        {
            SourceImportConfig& sourceImportConfig = srcPtr->EditConfig();
            ScalableMeshData data = sourceImportConfig.GetReplacementSMData();

            data.SetRepresenting3dData(true);

            sourceImportConfig.SetReplacementSMData(data);
        }
        else
        {
            SourceImportConfig& sourceImportConfig = srcPtr->EditConfig();
            ScalableMeshData data = sourceImportConfig.GetReplacementSMData();

            data.SetRepresenting3dData(false);

            sourceImportConfig.SetReplacementSMData(data);
        }
    }

    status = pTestChildNode->GetAttributeStringValue(datasetIsGround, "grounddetection");


    if (status == BEXML_Success)
    {
        if (datasetIsGround.Equals(L"1"))
        {
            SourceImportConfig& sourceImportConfig = srcPtr->EditConfig();
            ScalableMeshData data = sourceImportConfig.GetReplacementSMData();

            data.SetIsGroundDetection(true);
            data.SetRepresenting3dData(false);

            sourceImportConfig.SetReplacementSMData(data);
        }
    }
    WString propertyName;

    status = pTestChildNode->GetAttributeStringValue(propertyName, "elevationProperty");

    if (status == BEXML_Success)
    {
        SourceImportConfig& sourceImportConfig = srcPtr->EditConfig();
        ScalableMeshData data = sourceImportConfig.GetReplacementSMData();

        data.SetElevationPropertyName(propertyName);

        sourceImportConfig.SetReplacementSMData(data);
    }
    WString importType;
    status = pTestChildNode->GetAttributeStringValue(importType, "polygonImport");

    if (status == BEXML_Success)
    {
        DTMFeatureType type = DTMFeatureType::Breakline;
        ParseDTMFeatureType(importType, type);
        SourceImportConfig& sourceImportConfig = srcPtr->EditConfig();
        ScalableMeshData data = sourceImportConfig.GetReplacementSMData();

        data.SetPolygonFeatureType(type);

        sourceImportConfig.SetReplacementSMData(data);
    }

    WString linearImportType;
    status = pTestChildNode->GetAttributeStringValue(importType, "linearImport");

    if (status == BEXML_Success)
    {
        DTMFeatureType type = DTMFeatureType::ContourLine;
        ParseDTMFeatureType(linearImportType, type);
        SourceImportConfig& sourceImportConfig = srcPtr->EditConfig();
        ScalableMeshData data = sourceImportConfig.GetReplacementSMData();

        data.SetLinearFeatureType(type);

        sourceImportConfig.SetReplacementSMData(data);
    }

    WString gcsKeyName;
    status = pTestChildNode->GetAttributeStringValue(gcsKeyName, "gcsKeyName");

    if (status == BEXML_Success)
    {
        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr baseGCSPtr(BaseGCS::CreateGCS(gcsKeyName.c_str()));

        GeoCoords::GCS gcs(GetGCSFactory().Create(baseGCSPtr));

        SourceImportConfig& sourceImportConfig = srcPtr->EditConfig();

        sourceImportConfig.SetReplacementGCS(gcs);
    }

    return true;
}


IDTMSourcePtr CreateSourceFor(const WString&          sourcePath,
    DTMSourceDataType importedType,
    BeXmlNodeP        pTestChildNode = 0)
{
    if (0 == _wcsicmp(L"dgn", BeFileName::GetExtension(sourcePath.c_str()).c_str()))
    {
        assert(pTestChildNode != 0);

        WString model = L"Default";
        WString level = L"Default";

        StatusInt status = pTestChildNode->GetAttributeStringValue(model, "model");

        assert(status == SUCCESS);


        status = pTestChildNode->GetAttributeStringValue(level, "level");

        assert(status == SUCCESS);


        return IDTMDgnLevelSource::Create(importedType, sourcePath.c_str(), 0, model.c_str(), 0, level.c_str()).get();
    }

    return IDTMLocalFileSource::Create(importedType, sourcePath.c_str()).get();
}


void GetSourceDataType(DTMSourceDataType& dataType, BeXmlNodeP pSourceNode)
{
    WString dataTypeStr;

    StatusInt status = pSourceNode->GetAttributeStringValue(dataTypeStr, "dataType");

    if (status == BEXML_Success)
    {
        if (dataTypeStr.CompareTo(L"POINT") == 0)
        {
            dataType = DTM_SOURCE_DATA_POINT;
        }
        else
            if (dataTypeStr.CompareTo(L"DTM") == 0)
            {
                dataType = DTM_SOURCE_DATA_DTM;
            }
            else
                if (dataTypeStr.CompareTo(L"BREAKLINE") == 0)
                {
                    dataType = DTM_SOURCE_DATA_BREAKLINE;
                }
                else
                    if (dataTypeStr.CompareTo(L"RASTER") == 0)
                    {
                        dataType = DTM_SOURCE_DATA_IMAGE;
                    }
                    else
                        if (dataTypeStr.CompareTo(L"CLIP") == 0)
                        {
                            dataType = DTM_SOURCE_DATA_CLIP;
                        }
                        else
                            if (dataTypeStr.CompareTo(L"MESH") == 0)
                            {
                                dataType = DTM_SOURCE_DATA_MESH;
                            }
                            else
                            {
                                printf("Unsupporter/unknown data type");
                            }
    }
}


bool ParseSourceSubNodes(IDTMSourceCollection& sourceCollection, BeXmlNodeP pTestNode)
{
    bool isSuccess = true;

    BeXmlNodeP pTestChildNode = pTestNode->GetFirstChild();

    while ((0 != pTestChildNode) && (isSuccess == true))
    {
        if (0 == BeStringUtilities::Stricmp(pTestChildNode->GetName(), "source"))
        {
            WString datasetPath;
            WString datasetIs3D;
            WString datasetIsGround;

            StatusInt status = pTestChildNode->GetAttributeStringValue(datasetPath, "path");

            if (status == BEXML_Success)
            {
                DTMSourceDataType dataType = DTM_SOURCE_DATA_POINT;

                GetSourceDataType(dataType, pTestChildNode);

                if ((datasetPath.c_str()[datasetPath.size() - 1] != L'\\') &&
                    (datasetPath.c_str()[datasetPath.size() - 1] != L'/'))
                {
                    IDTMSourcePtr srcPtr = CreateSourceFor(datasetPath.c_str(), dataType, pTestChildNode);
                    AddOptionToSource(srcPtr, pTestChildNode);
                    if (BSISUCCESS != sourceCollection.Add(srcPtr))
                    {
                        isSuccess = false;
                        wprintf(L"ERROR : cannot add %s\r\n", datasetPath.c_str());
                        break;
                    }
                }
                else
                {
                    assert(!"Not implemented yet");
#if 0 
                    ATPFileFinder fileFinder;

                    WString filePaths;
                    WString filter;
                    status = pTestChildNode->GetAttributeStringValue(filter, "filter");
                    WString ext;
                    status = pTestChildNode->GetAttributeStringValue(ext, "extension");
                    fileFinder.FindFiles(datasetPath, filePaths, true);

                    WString firstPath;

                    while (fileFinder.ParseFilePaths(filePaths, firstPath))
                    {
                        BeFileName name(firstPath.c_str());
                        WString extension;
                        name.ParseName(NULL, NULL, NULL, &extension);
                        if (0 == BeStringUtilities::Wcsicmp(extension.c_str(), L"classif")) continue;
                        if (!ext.empty() && 0 != BeStringUtilities::Wcsicmp(extension.c_str(), ext.c_str())) continue;
                        if (!filter.empty() && !name.ContainsI(filter)) continue;
                        IDTMSourcePtr srcPtr = IDTMLocalFileSource::Create(dataType, firstPath.c_str());
                        AddOptionToSource(srcPtr, pTestChildNode);
                        if (BSISUCCESS != sourceCollection.Add(srcPtr))
                        {
                            isSuccess = false;
                            wprintf(L"ERROR : cannot add %s\r\n", firstPath.c_str());
                            break;
                        }
                    }
#endif
                }
            }
            else
            {
                printf("ERROR : attribute path for mesher node not found\r\n");
            }
        }
        else
        {
            //printf("ERROR : unknown child node for test node\r\n");
        }

        pTestChildNode = pTestChildNode->GetNextSibling();
    }

    return isSuccess;
}
/*---------------------------------------------------NEEDS_WORK_MST : Duplicate from ATP Code - END-----------------------------------------------------------*/


    TaskScheduler::TaskScheduler(BeFileName& taskFolderName)
        {
        m_taskFolderName = taskFolderName;
        }

    TaskScheduler::~TaskScheduler()
        {
        }

    void TaskScheduler::Start()
        {

/*
        BENTLEYDLL_EXPORT StatusInt GetCurrentEntry(BeFileName& name, bool& isDir, bool fullPath = true);

        //! Move to the next directory entry
        BENTLEYDLL_EXPORT StatusInt ToNext();
*/            
        bool isThereTaskAvaialble = true;
    
        while (isThereTaskAvaialble)
            { 
            isThereTaskAvaialble = false;

            BeDirectoryIterator dirIter(m_taskFolderName);
            
            while (dirIter.ToNext() == SUCCESS)
                {
                isThereTaskAvaialble = true; 

                BeFileName name;
                bool isDir;

                StatusInt status = dirIter.GetCurrentEntry(name, isDir);

                if (status == SUCCESS && isDir == false)
                    {
                    FILE* file = _wfopen(name, L"rb+");

                    if (file == nullptr) continue;

                    ProcessTask(file);
                    }
                }                   
            }
        }



    bool TaskScheduler::ParseWorkerTaskType(BeXmlNodeP pXmlTaskNode, WorkerTaskType& t)
    {
        WString testType;
        StatusInt status = pXmlTaskNode->GetAttributeStringValue(testType, "type");

        if (status == BEXML_Success)
            {          
            if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"cut"))
                t = WorkerTaskType::CUT;
            else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"filter"))
                t = WorkerTaskType::FILTER;
            else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"index"))
                t = WorkerTaskType::INDEX;
            else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"mesh"))
                t = WorkerTaskType::MESH;
            else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"stitch"))
                t = WorkerTaskType::STITCH;            
            else return false;
        }
        else return false;
        return true;
    }


    bool TaskScheduler::ProcessTask(FILE* file)
        {
        assert(file != nullptr);

        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);

        BeXmlStatus status;
        WString     errorMsg;

        bvector<char> xmlFileContent(size);
        size_t readSize = fread(&xmlFileContent[0], size, 1, file);
        assert(readSize == size);

        //BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(status, testPlanPath.c_str(), &errorMsg); 
        BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromMemory(status, &xmlFileContent[0], xmlFileContent.size(), &errorMsg);

        if (pXmlDom == 0)
            {
            //assert(false && "Invalid test plan filename");
            return false;
            }

        BeXmlNodeP pXmlTaskNode(pXmlDom->GetRootElement());
        if (0 != BeStringUtilities::Stricmp(pXmlTaskNode->GetName(), "workerTask"))
            {
            //assert(false && "Invalid test plan format");
            return false;
            }

/*
        FILE*          pResultFile = 0;
        if (!OpenResultFile(pXmlTaskNode, pResultFile)) return false;
*/
        WorkerTaskType t;
        if (!ParseWorkerTaskType(pXmlTaskNode, t))
            {
            assert(false && "Invalid worker task type");
            //fclose(pResultFile);
            return false;
            }
        //fwprintf(pResultFile, GetHeaderForTestType(t).c_str());
        //BeXmlNodeP pTaskNode = pXmlTaskNode->GetFirstChild();
/*
        while (0 != pXmlTaskNode)
            {
*/
            switch (t)
                {
                case WorkerTaskType::CUT:
                    PerformCutTask(pXmlTaskNode/*, pResultFile*/);
                    break;
                case WorkerTaskType::FILTER:
                    PerformFilterTask(pXmlTaskNode/*, pResultFile*/);
                    break;
                case WorkerTaskType::INDEX:
                    PerformIndexTask(pXmlTaskNode/*, pResultFile*/);
                    break;
                case WorkerTaskType::MESH:
                    PerformMeshTask(pXmlTaskNode/*, pResultFile*/);
                    break;
                case WorkerTaskType::STITCH:
                    PerformStitchTask(pXmlTaskNode/*, pResultFile*/);
                    break;
                default: break;
                }
/*
            pTestNode = pTestNode->GetNextSibling();
            }

        if (pResultFile != 0)
            {
            fclose(pResultFile);
            }
*/
        return true;

        }


    void TaskScheduler::PerformCutTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/)
        {

        }

    void TaskScheduler::PerformFilterTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/)
        {

        }

    void TaskScheduler::PerformIndexTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/)
        {
        IDTMSourceCollection sourceCollection;

        bool result = ParseSourceSubNodes(sourceCollection, pXmlTaskNode);
        assert(result == true);
    
        }

    void TaskScheduler::PerformMeshTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/)
        {

        }

    void TaskScheduler::PerformStitchTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/)
        {

        }
    
END_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE