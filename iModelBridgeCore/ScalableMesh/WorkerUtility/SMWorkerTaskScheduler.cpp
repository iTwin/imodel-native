//#include "ScalableMeshATPPch.h"
#include "ScalableMeshWorker.h"
#include "SMWorkerTaskScheduler.h"

#include <process.h>

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
#include <ScalableMesh\IScalableMeshSourceCreator.h>
#include <ScalableMesh\IScalableMeshSourceCreatorWorker.h>

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


bool ParseSourceSubNodes(IDTMSourceCollection& sourceCollection, BeXmlNodeP pXmlTaskNode)
{
    bool isSuccess = true;

    BeXmlNodeP pTestChildNode = pXmlTaskNode->GetFirstChild();

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


TaskScheduler::TaskScheduler(BeFileName& taskFolderName, uint32_t nbWorkers, bool useGroupingStrategy, uint32_t groupingSize)
    {
    m_taskFolderName = taskFolderName;
    m_nbWorkers = nbWorkers;
    m_useGroupingStrategy = useGroupingStrategy;
    m_groupingSize = groupingSize; 
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

    clock_t duration = clock();
    
    BeDuration sleeper(BeDuration::FromSeconds(0.1));
    
    
    bool isThereTaskAvailable = true;

    while (isThereTaskAvailable)
        { 
        isThereTaskAvailable = false;

        BeDirectoryIterator dirIter(m_taskFolderName);

        BeFileName name;
        bool isDir;
        
        for (; SUCCESS == dirIter.GetCurrentEntry(name, isDir); dirIter.ToNext())        
            {                                
            if (isDir == false && 0 == name.GetExtension().CompareTo(L"xml"))
                {                
                isThereTaskAvailable = true;
                
                struct _stat64i32 buffer;

                if (_wstat(name.c_str(), &buffer) != 0 || buffer.st_size == 0) continue;
                                
                BeFileName lockFileName(name);
                lockFileName.AppendString(L".lock");

                FILE* lockFile = _wfsopen(lockFileName, L"ab+", _SH_DENYRW);

                if (lockFile == nullptr)
                    continue;
                                
                //struct _stat64i32 buffer;

                if (_wstat(name.c_str(), &buffer) != 0 || buffer.st_size == 0) continue;
                                
                FILE* file = nullptr;
                
                //errno_t err = _wfopen_s(&file, name, L"abN+");
                errno_t err = 0;
                file = _wfsopen(name, L"ab+", _SH_DENYRW);
                
                if (file == nullptr) continue;

                if (err != 0)
                    {
                    assert(file == nullptr);
                    continue;
                    }

                //If first char is equal to 0 the file has already been process
                char startingChar = 0;
                size_t readSize = fread(&startingChar, 1, 1, file);
                //assert(readSize == 1);

                if (startingChar == 0 || readSize == 0)
                    {
                    fclose(file);
                    continue;
                    }

                ProcessTask(file);
             

                fclose(file);

                while (0 != _wremove(name))
                    {                    
					}

                fclose(lockFile);

                while (0 != _wremove(lockFileName))
                    {
			        sleeper.Sleep();
                    }                                
                }

            }          
        
        if (!isThereTaskAvailable)
            {
            BeFileName testPlanLockFile(m_taskFolderName);

            testPlanLockFile.AppendString(L"\\Tasks.xml.Plan.lock");

            FILE* lockFile = _wfsopen(testPlanLockFile.c_str(), L"ab+", _SH_DENYRW);

            if (lockFile == nullptr)
                {
                isThereTaskAvailable = true;
                continue;
                }

            BeDirectoryIterator dirIter(m_taskFolderName);

            BeFileName name;
            bool isDir;

            bool needExecuteTaskPlan = true;

            for (; SUCCESS == dirIter.GetCurrentEntry(name, isDir); dirIter.ToNext())
                {
                if (isDir == false && 0 == name.GetExtension().CompareTo(L"xml"))
                    {
                    needExecuteTaskPlan = false;
                    break;
                    }
                }

            StatusInt status = SUCCESS;
            
            if (needExecuteTaskPlan)            
                status = GetSourceCreatorWorker()->ExecuteNextTaskInTaskPlan();        

            fclose(lockFile);
            
            if (status == SUCCESS_TASK_PLAN_COMPLETE)
                break;           

            isThereTaskAvailable = true;
            }

        sleeper.Sleep();
        }


    double totalDuration = (double)(clock() - duration) / CLOCKS_PER_SEC;

    m_sourceCreatorWorkerPtr = nullptr;
        
    BeFileName durationFileName(L"D:\\MyDoc\\RMA - July\\CloudWorker\\Log\\duration");    
    durationFileName.AppendString(std::to_wstring(::_getpid()).c_str());
    durationFileName.AppendString(L".csv");

    FILE* durationFile = _wfsopen(durationFileName.c_str(), L"ab+", _SH_DENYRW);

        /*
        fwprintf(pResultFile,
            L"%s,%s,%s,%s,%I64d,%I64d,%.5f%%,%.5f,%s,%.5f,%.5f,%.5f,%.5f,%.5f%%,%.5f%%,%.5f%%,%.5f%%,%.5f%%,%.5f%%,%.5f%%,%.5f%%,%.5f%%,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%s\n",
            */

    fwprintf(durationFile, L"Duration : %.5f\n", totalDuration);
    fclose(durationFile);
    }
    
void TaskScheduler::GetScalableMeshFileName(BeFileName& smFileName) const
    {
    smFileName.AppendString(m_taskFolderName.c_str());    
    smFileName.AppendString(L"\\generated3sm.3sm");
    }

IScalableMeshSourceCreatorWorkerPtr TaskScheduler::GetSourceCreatorWorker()
    {
    if (!m_sourceCreatorWorkerPtr.IsValid())
        {
        BeFileName smFileName;

        GetScalableMeshFileName(smFileName);

        StatusInt status;
        
        m_sourceCreatorWorkerPtr = IScalableMeshSourceCreatorWorker::GetFor(smFileName.c_str(), m_nbWorkers, status);        

        assert(m_sourceCreatorWorkerPtr.IsValid());            
        }

    return m_sourceCreatorWorkerPtr;
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
            else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"generate"))
                t = WorkerTaskType::GENERATE;                        
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
    size_t readSize = fread(&xmlFileContent[0], 1, size, file);
    assert(readSize == size);

    //BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(status, testPlanPath.c_str(), &errorMsg); 
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromMemory(status, &xmlFileContent[0], xmlFileContent.size(), &errorMsg);

    if (pXmlDom == 0)
        {
        assert(false && "Invalid test plan filename");
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
            case WorkerTaskType::GENERATE:
                PerformGenerateTask(pXmlTaskNode/*, pResultFile*/);
                break;                

            default: break;
            }
/*
        pXmlTaskNode = pXmlTaskNode->GetNextSibling();
            }

        if (pResultFile != 0)
            {
            fclose(pResultFile);
            }
*/

    //Write 0 at the beginning of the task file to ensue no other worker processes this task.
    fseek(file, 0, SEEK_SET);

    xmlFileContent[0] = 0;
    size_t writeSize = fwrite(&xmlFileContent[0], 1, 1, file);
    assert(writeSize == 1);    
    fflush(file);

    return true;

    }


void TaskScheduler::PerformCutTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/)
    {

    }

void TaskScheduler::PerformFilterTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/)
    {
    IScalableMeshSourceCreatorWorkerPtr creatorWorkerPtr(GetSourceCreatorWorker());

    StatusInt status = creatorWorkerPtr->ProcessFilterTask(pXmlTaskNode);

    assert(status == SUCCESS);
    }

void TaskScheduler::PerformIndexTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/)
    {        
    BeFileName smFileName;

    GetScalableMeshFileName(smFileName);    

    _wremove(smFileName.c_str());
    
    StatusInt createStatus;
    BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreatorPtr creatorPtr(BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreator::GetFor(smFileName.c_str(), createStatus));

    if (creatorPtr == 0)
        {
        //printf("ERROR : cannot create STM file\r\n");
        return;
        }
   
    creatorPtr->SetShareable(true);
    ScalableMeshCreationMethod creationMethod = SCM_CREATION_METHOD_ONE_SPLIT;

    creatorPtr->SetCreationMethod(creationMethod);
    creatorPtr->SetCreationCompleteness(SCM_CREATION_COMPLETENESS_INDEX_ONLY);
    

/*
    double nTimeToCreateSeeds = 0, nTimeToEstimateParams = 0, nTimeToFilterGround = 0;*/

    WString gcsKeyName;//

    auto status = pXmlTaskNode->GetAttributeStringValue(gcsKeyName, "gcsKeyName");

    if (status == BEXML_Success)
        {
        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr baseGCSPtr(BaseGCS::CreateGCS(gcsKeyName.c_str()));
        StatusInt status = creatorPtr->SetBaseGCS(baseGCSPtr);
        assert(status == SUCCESS);
        }//

    bool streamFromMapBox = false;
    bool streamFromBingMap = false;

    WString streamAttr;
    auto readStatus = pXmlTaskNode->GetAttributeStringValue(streamAttr, "textureStreaming");

    if (readStatus == BEXML_Success)
        {
        if (0 == BeStringUtilities::Wcsicmp(streamAttr.c_str(), L"mapbox"))
            {
                streamFromMapBox = true;
            }
            else
                if (0 == BeStringUtilities::Wcsicmp(streamAttr.c_str(), L"bingmap"))
                {
                    streamFromBingMap = true;
                }
                else
                {
                    //assert(!"Unknown textureStreaming value");
                }
            }

    if (ParseSourceSubNodes(creatorPtr->EditSources(), pXmlTaskNode) == true)
            {
#if 0
                SetGroundDetectionDuration(0.0);
                clock_t t = clock();

                bool importInProgress = true;
                std::thread mythread([&importInProgress, &creatorPtr]()
                {
                    float lastProgress = 0;
                    int lastStep = 0;
                    while (importInProgress)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));

                        {
                            float progress = 0;
                            int step = 0;
                            int nSteps = 0;
                            if (!creatorPtr.IsValid() || creatorPtr->GetProgress() == nullptr) continue;

                            progress = creatorPtr->GetProgress()->GetProgress();
                            step = creatorPtr->GetProgress()->GetProgressStep();
                            nSteps = creatorPtr->GetProgress()->GetTotalNumberOfSteps();
                            if ((fabs(progress - lastProgress) > 0.01 || abs(step - lastStep) > 0))
                            {
                                std::cout << " PROGRESS: " << progress * 100.0 << " % ON STEP " << step << " OUT OF " << nSteps << std::endl;
                                lastProgress = progress;
                                lastStep = step;
                            }
                        }
                    }
                });
#endif

            bool isSingleFile = true;
            creatorPtr->SetShareable(true);
            StatusInt status = creatorPtr->Create(isSingleFile);

#if 0
            importInProgress = false;
            mythread.join();
#endif

            creatorPtr->SaveToFile();
            creatorPtr = nullptr;            
            

            IScalableMeshSourceCreatorWorkerPtr creatorWorkerPtr(GetSourceCreatorWorker());

            if (m_useGroupingStrategy)
                {
                status = creatorWorkerPtr->CreateGenerationTasks(m_groupingSize);
                }
            else
                {
                status = creatorWorkerPtr->CreateMeshTasks();

                assert(status == SUCCESS);

                status = creatorWorkerPtr->CreateTaskPlan();

                assert(status == SUCCESS);
                }
            
/*
smFileName

            
            creatorPtr->SaveToFile();
*/

/*
            IDTMSourceCollection sourceCollection;

            bool result = ParseSourceSubNodes(sourceCollection, pXmlTaskNode);
            assert(result == true);*/
            }
        
    
        }

void TaskScheduler::PerformMeshTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/)
    {
    IScalableMeshSourceCreatorWorkerPtr creatorWorkerPtr(GetSourceCreatorWorker());               

    StatusInt status = creatorWorkerPtr->ProcessMeshTask(pXmlTaskNode);

    assert(status == SUCCESS);
    }

void TaskScheduler::PerformStitchTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/)
    {
    IScalableMeshSourceCreatorWorkerPtr creatorWorkerPtr(GetSourceCreatorWorker());

    StatusInt status = creatorWorkerPtr->ProcessStitchTask(pXmlTaskNode);

    assert(status == SUCCESS);    
    }

void TaskScheduler::PerformGenerateTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/)
    {
    IScalableMeshSourceCreatorWorkerPtr creatorWorkerPtr(GetSourceCreatorWorker());               

    StatusInt status = creatorWorkerPtr->ProcessGenerateTask(pXmlTaskNode);

    assert(status == SUCCESS);
    }


END_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE