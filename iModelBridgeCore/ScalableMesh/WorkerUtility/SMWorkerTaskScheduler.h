#pragma once

#include <list>
#include <iostream>
#include <windows.h> //used for pipes
#include <DgnPlatform/DgnPlatform.h>
#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh/IScalableMeshSourceCreatorWorker.h>
#include <ImagePP/h/ImageppAPI.h>
#include <Bentley/BeFileListIterator.h>
#include <DgnPlatform/DgnPlatformLib.h>

#include <BeXml/BeXml.h>
USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SCALABLEMESH

BEGIN_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE    


struct TaskProcessingStat
    {
    double m_durationInSeconds;
    WorkerTaskType m_taskType;
    };

struct JobProcessingStat
    {

    JobProcessingStat()
        {        
        time(&m_startTime);
        time(&m_stopTime);
        }

    time_t m_startTime;
    time_t m_stopTime;

    bvector<TaskProcessingStat> m_processedTasks;
    };

struct TaskScheduler
    {

    private:
    
        BeFileName m_taskFolderName;
        uint32_t   m_nbWorkers;
        bool       m_useGroupingStrategy;        
        uint32_t   m_groupingSize; 
        bool       m_startAsService;
        BeFileName m_resultFolderName;

        bmap<WString, JobProcessingStat> m_jobProcessingStat;         
        IScalableMeshSourceCreatorWorkerPtr m_sourceCreatorWorkerPtr;
        
        JobProcessingStat& GetJobStat(const WString& jobName);

        void OutputJobStat();

        void GetScalableMeshFileName(BeFileName& smFileNameAbsolutePath, const BeFileName& smFileName) const;

        IScalableMeshSourceCreatorWorkerPtr GetSourceCreatorWorker(const BeFileName& smFileName);

        bool ParseWorkerTaskType(BeXmlNodeP pXmlTaskNode, WorkerTaskType& t);

        StatusInt ExecuteTaskPlanNextTask(const BeFileName& taskPlanFileName);
                
        bool ProcessTask(FILE* file);

        void PerformCutTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/);
                        
        void PerformFilterTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/);
                
        void PerformIndexTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/);
        
        void PerformMeshTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/);

        void PerformStitchTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/);

        void PerformGenerateTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/);
        
    
    public:
    
        TaskScheduler(BeFileName& taskFolderName, uint32_t nbWorkers, bool useGroupingStrategy, uint32_t groupingSize, bool startAsService, const BeFileName& resultFolderName);

        virtual ~TaskScheduler();

        void Start();

    };

END_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE