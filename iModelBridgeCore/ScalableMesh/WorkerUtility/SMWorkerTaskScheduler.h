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

struct TaskScheduler
    {

    private:
    
        BeFileName m_taskFolderName;
        uint32_t   m_nbWorkers;

        IScalableMeshSourceCreatorWorkerPtr m_sourceCreatorWorkerPtr;

        void GetScalableMeshFileName(BeFileName& smFileName) const;

        IScalableMeshSourceCreatorWorkerPtr GetSourceCreatorWorker();

        bool ParseWorkerTaskType(BeXmlNodeP pXmlTaskNode, WorkerTaskType& t);

        bool ProcessTask(FILE* file);

        void PerformCutTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/);
                        
        void PerformFilterTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/);
                
        void PerformIndexTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/);
        
        void PerformMeshTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/);

        void PerformStitchTask(BeXmlNodeP pXmlTaskNode/*, pResultFile*/);
    
    public:
    
        TaskScheduler(BeFileName& taskFolderName, uint32_t nbWorkers);

        virtual ~TaskScheduler();

        void Start();

    };

END_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE