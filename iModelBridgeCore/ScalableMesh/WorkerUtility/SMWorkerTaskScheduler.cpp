//#include "ScalableMeshATPPch.h"
#include "ScalableMeshWorker.h"
#include "SMWorkerTaskScheduler.h"

#include <ctime> 
#include <process.h>
#include <string.h>

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

void ParseJobInfo(WString& jobName, BeFileName& smFileName, BeXmlNodeP pXmlTaskNode)
    {    
    StatusInt status = pXmlTaskNode->GetAttributeStringValue(jobName, "jobName");    
    assert(status == BEXML_Success);

    status = pXmlTaskNode->GetAttributeStringValue(smFileName, "smName");
    assert(status == BEXML_Success);   
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


TaskScheduler::TaskScheduler(BeFileName& taskFolderName, uint32_t nbWorkers, bool useGroupingStrategy, uint32_t groupingSize, bool startAsService, const BeFileName& resultFolderName)
    {
    m_taskFolderName = taskFolderName;
    m_nbWorkers = nbWorkers;
    m_useGroupingStrategy = useGroupingStrategy;
    m_startAsService = startAsService;
    m_groupingSize = groupingSize; 
    m_resultFolderName = resultFolderName;
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

    BeDuration sleeper(BeDuration::FromSeconds(0.1));
    BeDuration listenSleeper(BeDuration::FromSeconds(5));

    bool listenForNewTask = true;    
    
    while (listenForNewTask)
        {                                
        bool isThereTaskAvailable = true;
    
        while (isThereTaskAvailable)
            { 
            isThereTaskAvailable = false;

            BeDirectoryIterator dirIter(m_taskFolderName);

            BeFileName name;
            bool isDir;
            int  fileOpReturnCode = 0; 
        
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

                    if (_wstat(name.c_str(), &buffer) != 0 || buffer.st_size == 0)
                        {
                        fileOpReturnCode = fclose(lockFile);
                        assert(fileOpReturnCode == 0);
                        continue;
                        }
                                
                    FILE* file = nullptr;
                
                    //errno_t err = _wfopen_s(&file, name, L"abN+");
                    errno_t err = 0;
                    file = _wfsopen(name, L"ab+", _SH_DENYRW);
                
                    if (file == nullptr) 
                        {                        
                        fileOpReturnCode = fclose(lockFile);
                        assert(fileOpReturnCode == 0);
                        continue;
                        }

                    if (err != 0)
                        {
                        fileOpReturnCode = fclose(lockFile);
                        assert(fileOpReturnCode == 0);
                        assert(file == nullptr);
                        continue;
                        }

                    //If first char is equal to 0 the file has already been process
                    char startingChar = 0;
                    size_t readSize = fread(&startingChar, 1, 1, file);
                    //assert(readSize == 1);

                    if (startingChar == 0 || readSize == 0)
                        {
                        fileOpReturnCode = fclose(file);
                        assert(fileOpReturnCode == 0);
                        fileOpReturnCode = fclose(lockFile);
                        assert(fileOpReturnCode == 0);
                        continue;
                        }

                    ProcessTask(file);
             

                    fclose(file);

                    while (0 != _wremove(name))
                        {                    
					    }

                    while (0 != fclose(lockFile))
                        {
                        }

                    int retRemoveCode; 

                    while (0 != (retRemoveCode = _wremove(lockFileName)))
                        {
                        retRemoveCode = retRemoveCode;

                        char buffer[1024];
                        strerror_s(buffer, 1024, errno);
                        sleeper.Sleep();
                        }                                
                    }

                }          
        
            if (!isThereTaskAvailable)
                {
                BeFileName taskPlanFileName(m_taskFolderName);

                taskPlanFileName.AppendString(L"\\Tasks.xml.Plan");

                struct _stat64i32 buffer;

                //No task plan, no more task to execute
                if (_wstat(taskPlanFileName.c_str(), &buffer) != 0 || buffer.st_size == 0) 
                    continue;

                BeFileName taskPlanLockFile(m_taskFolderName);

                taskPlanLockFile.AppendString(L"\\Tasks.xml.Plan.lock");

                FILE* lockFile = _wfsopen(taskPlanLockFile.c_str(), L"ab+", _SH_DENYRW);

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
                    {        
                    status = ExecuteTaskPlanNextTask(taskPlanFileName);
                    }
                   
                fclose(lockFile);
            
                if (status == SUCCESS_TASK_PLAN_COMPLETE)
                    break;           

                isThereTaskAvailable = true;
                }

            sleeper.Sleep();
            }

        m_sourceCreatorWorkerPtr = nullptr;

        OutputJobStat();
        
        if (m_startAsService)
            {
            listenSleeper.Sleep();
            }
        else
            {
            listenForNewTask = false;
            }        
        }    
    }

void TaskScheduler::OutputJobStat()
    {                   
    if (m_resultFolderName.empty())
        return;
    
    for (auto& jobStat : m_jobProcessingStat)
        {        
        BeFileName durationFileName(m_resultFolderName);    
        durationFileName.AppendString(jobStat.first.c_str());
        durationFileName.AppendString(L"_");
    
        char *temp = 0;
        std::string computerName;

#if defined(_WIN32)
        temp = getenv("COMPUTERNAME");
        if (temp != 0) 
            {
            computerName = temp;
            temp = 0;
            }
#else
        temp = getenv("HOSTNAME");

        if (temp != 0) {
            computerName = temp;
            temp = 0;
        } else {
            temp = new char[512];
            if (gethostname(temp, 512) == 0) { // success = 0, failure = -1
                computerName = temp;
            }
            delete []temp;
            temp = 0;
        }
#endif

        if (!computerName.empty())
            {
            durationFileName.AppendString(WString(computerName.c_str(), false).c_str());        
            durationFileName.AppendString(L"_");
            }

        durationFileName.AppendString(std::to_wstring(::_getpid()).c_str());
        durationFileName.AppendString(L"_");
        durationFileName.AppendString(L".csv");

        FILE* durationFile = _wfsopen(durationFileName.c_str(), L"ab+", _SH_DENYRW);

        assert(durationFile != nullptr);

        if (durationFile != nullptr)
            {
            time_t duration = jobStat.second.m_stopTime - jobStat.second.m_startTime;
            fwprintf(durationFile, L"StartTime,StopTime,Duration (seconds)\r\n");
            fwprintf(durationFile, L"%lli,%lli,%lli\r\n\r\n", jobStat.second.m_startTime, jobStat.second.m_stopTime, duration);

            fwprintf(durationFile, L"TaskType,Duration (seconds)\r\n");

            for (auto& task : jobStat.second.m_processedTasks)
                {
                WString taskTypeStr;

                switch (task.m_taskType)
                    {
                    case WorkerTaskType::CUT:
                        taskTypeStr.AssignOrClear(L"CUT");
                        break;
                    case WorkerTaskType::FILTER:
                        taskTypeStr.AssignOrClear(L"FILTER");
                        break;
                    case WorkerTaskType::INDEX:
                        taskTypeStr.AssignOrClear(L"INDEX");
                        break;
                    case WorkerTaskType::MESH:
                        taskTypeStr.AssignOrClear(L"MESH");
                        break;
                    case WorkerTaskType::STITCH:
                        taskTypeStr.AssignOrClear(L"STITCH");
                        break;
                    case WorkerTaskType::GENERATE:
                        taskTypeStr.AssignOrClear(L"GENERATE");
                        break;
                    case WorkerTaskType::TEXTURE:
                        taskTypeStr.AssignOrClear(L"TEXTURE");
                        break;
                    case WorkerTaskType::CREATETEXTURE:
                        taskTypeStr.AssignOrClear(L"CREATETEXTURE");
                        break;
                    default : 
                        assert(!"Unknown task type");
                        break;
                    }

                fwprintf(durationFile, L"%s,%.5f\r\n", taskTypeStr.c_str(), task.m_durationInSeconds);                
                }           
            }
           
        fclose(durationFile);
        }

    m_jobProcessingStat.clear();
    }
        
void TaskScheduler::GetScalableMeshFileName(BeFileName& smFileNameAbsolutePath, const BeFileName& smFileName) const
    {
    smFileNameAbsolutePath.AppendString(m_taskFolderName.c_str());

    if (!smFileName.empty())
        {
        smFileNameAbsolutePath.AppendString(smFileName.c_str());
        }
    else
        {
        smFileNameAbsolutePath.AppendString(L"\\generated3sm.3sm");
        }
    }

IScalableMeshSourceCreatorWorkerPtr TaskScheduler::GetSourceCreatorWorker(const BeFileName& smFileName)
    {
    if (!m_sourceCreatorWorkerPtr.IsValid())
        {                
        BeFileName smFileNameAbsolutePath;

        GetScalableMeshFileName(smFileNameAbsolutePath, smFileName);

        StatusInt status;
        
        m_sourceCreatorWorkerPtr = IScalableMeshSourceCreatorWorker::GetFor(smFileNameAbsolutePath.c_str(), m_nbWorkers, status);        

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
            else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"texture"))
                t = WorkerTaskType::TEXTURE;
            else if (0 == BeStringUtilities::Wcsicmp(testType.c_str(), L"createTexture"))
                t = WorkerTaskType::CREATETEXTURE;
            else return false;
        }
        else return false;
        return true;
    }


JobProcessingStat& TaskScheduler::GetJobStat(const WString& jobName)
    {    
    bmap<WString, JobProcessingStat>::iterator jobIter;
    jobIter = m_jobProcessingStat.find(jobName);

    if (jobIter == m_jobProcessingStat.end())
        {
        JobProcessingStat jobStat;
        bpair<bmap<WString, JobProcessingStat>::iterator,bool> ret = m_jobProcessingStat.insert(bpair<WString, JobProcessingStat>(jobName,jobStat));
        assert(ret.second == true);
        jobIter = ret.first;
        }
    
    return jobIter->second;
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
        assert(false && "Invalid task filename");
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
        WString jobName;
        BeFileName smFileName;

        ParseJobInfo(jobName, smFileName, pXmlTaskNode);

        JobProcessingStat& jobStat = GetJobStat(jobName);
        
        TaskProcessingStat taskStat;

        taskStat.m_taskType = t;
        clock_t startTime = clock();        

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
            case WorkerTaskType::TEXTURE:
                PerformTextureTask(pXmlTaskNode/*, pResultFile*/);
                break;
            case WorkerTaskType::CREATETEXTURE:
                PerformCreateTextureTask(pXmlTaskNode/*, pResultFile*/);
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

    taskStat.m_durationInSeconds = (double)(clock() - startTime) / CLOCKS_PER_SEC;

    jobStat.m_processedTasks.push_back(taskStat);
    time(&jobStat.m_stopTime);

    //Write 0 at the beginning of the task file to ensue no other worker processes this task.
    fseek(file, 0, SEEK_SET);

    xmlFileContent[0] = 0;
    size_t writeSize = fwrite(&xmlFileContent[0], 1, 1, file);
    assert(writeSize == 1);    
    fflush(file);

    return true;
    }
        

StatusInt TaskScheduler::ExecuteTaskPlanNextTask(const BeFileName& taskPlanFileName)
    {
    struct _stat64i32 buffer;

    //If task plan doesn't exist. 
    if (_wstat(taskPlanFileName.c_str(), &buffer) != 0)
        return SUCCESS_TASK_PLAN_COMPLETE;    

    FILE* file = nullptr;

    errno_t err = 0;
    file = _wfsopen(taskPlanFileName.c_str(), L"ab+", _SH_DENYRW);
                    
    assert(file != nullptr);

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    BeXmlStatus xmlStatus;
    WString     errorMsg;

    bvector<char> xmlFileContent(size);
    size_t readSize = fread(&xmlFileContent[0], 1, size, file);
    assert(readSize == size);
    
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromMemory(xmlStatus, &xmlFileContent[0], xmlFileContent.size(), &errorMsg);

    if (pXmlDom == 0)
        {
        assert(!"Invalid task plan filename");
        return SUCCESS_TASK_PLAN_COMPLETE;
        }

    BeXmlNodeP pXmlTaskNode(pXmlDom->GetRootElement());
    if (0 != BeStringUtilities::Stricmp(pXmlTaskNode->GetName(), "taskPlan"))
        {
        assert("Invalid task plan format");
        return SUCCESS_TASK_PLAN_COMPLETE;    
        }

    WString jobName;
    BeFileName smFileName;

    ParseJobInfo(jobName, smFileName, pXmlTaskNode);

    IScalableMeshSourceCreatorWorkerPtr creatorWorkerPtr(GetSourceCreatorWorker(smFileName));               

    fclose(file);

    StatusInt status = creatorWorkerPtr->ExecuteNextTaskInTaskPlan(); 
    
    return status;
    }


void TaskScheduler::PerformCutTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/)
    {

    }

void TaskScheduler::PerformFilterTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/)
    {    
    WString    jobName;
    BeFileName smFileName;

    ParseJobInfo(jobName, smFileName, pXmlTaskNode);
        
    IScalableMeshSourceCreatorWorkerPtr creatorWorkerPtr(GetSourceCreatorWorker(smFileName));

    StatusInt status = creatorWorkerPtr->ProcessFilterTask(pXmlTaskNode);

    assert(status == SUCCESS);
    }

void TaskScheduler::PerformIndexTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/)
    {        
    WString    jobName;
    BeFileName smFileName;

    ParseJobInfo(jobName, smFileName, pXmlTaskNode);
                 
    BeFileName smFileNameAbsolutePath;

    GetScalableMeshFileName(smFileNameAbsolutePath, smFileName);

    _wremove(smFileNameAbsolutePath.c_str());
    
    StatusInt createStatus;
    BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreatorPtr creatorPtr(BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreator::GetFor(smFileNameAbsolutePath.c_str(), createStatus));

    if (creatorPtr == 0)
        {
        //printf("ERROR : cannot create STM file\r\n");
        return;
        }
   
    creatorPtr->SetShareable(true);
    ScalableMeshCreationMethod creationMethod = SCM_CREATION_METHOD_ONE_SPLIT;

    creatorPtr->SetCreationMethod(creationMethod);
    creatorPtr->SetCreationCompleteness(SCM_CREATION_COMPLETENESS_INDEX_ONLY);

    uint32_t splitThreshold;
    BeXmlStatus xmlStatus = pXmlTaskNode->GetAttributeUInt32Value(splitThreshold, "splitThreshold");    

    if (xmlStatus == BEXML_Success && splitThreshold >= 100)
        {
        creatorPtr->SetSplitThreshold(splitThreshold);
        }        

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
            

            IScalableMeshSourceCreatorWorkerPtr creatorWorkerPtr(GetSourceCreatorWorker(smFileName));

            if (m_useGroupingStrategy)
                {                                
                status = creatorWorkerPtr->CreateGenerationTasks(m_groupingSize, jobName, smFileName);
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
    WString    jobName;
    BeFileName smFileName;

    ParseJobInfo(jobName, smFileName, pXmlTaskNode);

    IScalableMeshSourceCreatorWorkerPtr creatorWorkerPtr(GetSourceCreatorWorker(smFileName));               

    StatusInt status = creatorWorkerPtr->ProcessMeshTask(pXmlTaskNode);

    assert(status == SUCCESS);
    }

void TaskScheduler::PerformStitchTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/)
    {
    WString    jobName;
    BeFileName smFileName;

    ParseJobInfo(jobName, smFileName, pXmlTaskNode);

    IScalableMeshSourceCreatorWorkerPtr creatorWorkerPtr(GetSourceCreatorWorker(smFileName));

    StatusInt status = creatorWorkerPtr->ProcessStitchTask(pXmlTaskNode);

    assert(status == SUCCESS);    
    }

void TaskScheduler::PerformGenerateTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/)
    {
    WString    jobName;
    BeFileName smFileName;

    ParseJobInfo(jobName, smFileName, pXmlTaskNode);

    IScalableMeshSourceCreatorWorkerPtr creatorWorkerPtr(GetSourceCreatorWorker(smFileName));               

    StatusInt status = creatorWorkerPtr->ProcessGenerateTask(pXmlTaskNode);

    assert(status == SUCCESS);
    }

void TaskScheduler::PerformTextureTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/)
    {
    WString    jobName;
    BeFileName smFileName;

    ParseJobInfo(jobName, smFileName, pXmlTaskNode);

    IScalableMeshSourceCreatorWorkerPtr creatorWorkerPtr(GetSourceCreatorWorker(smFileName));
    StatusInt status = SUCCESS;
    if (ParseSourceSubNodes(creatorWorkerPtr->EditSources(), pXmlTaskNode) == true)
    {
        status = creatorWorkerPtr->ProcessTextureTask(pXmlTaskNode);
    }

    assert(status == SUCCESS);
    }

void TaskScheduler::PerformCreateTextureTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/)
{
    WString    jobName;
    BeFileName smFileName;

    ParseJobInfo(jobName, smFileName, pXmlTaskNode);

    IScalableMeshSourceCreatorWorkerPtr creatorWorkerPtr(GetSourceCreatorWorker(smFileName));
    StatusInt status = SUCCESS;
    if (ParseSourceSubNodes(creatorWorkerPtr->EditSources(), pXmlTaskNode) == true)
    {
        status = creatorWorkerPtr->CreateTextureTasks(m_groupingSize, jobName, smFileName);
    }

    assert(status == SUCCESS);
}


END_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE