/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourceCreatorWorker.h $
|    $RCSfile: ScalableMeshSourceCreator.h,v $
|   $Revision: 1.45 $
|       $Date: 2015/07/15 11:02:24 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

#include "ScalableMeshSourceCreator.h"
#include <ScalableMesh/IScalableMeshSourceCreatorWorker.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct NodeTask;
typedef RefCountedPtr<NodeTask> NodeTaskPtr;

struct GenerationTask;
typedef RefCountedPtr<GenerationTask> GenerationTaskPtr;

struct TextureTask;
typedef RefCountedPtr<TextureTask> TextureTaskPtr;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Mathieu.St-Pierre    06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct IScalableMeshSourceCreatorWorker::Impl : public IScalableMeshSourceCreator::Impl
    {

    friend struct IScalableMeshSourceCreatorWorker;

    private:

        

        ScalableMeshDb* m_smDb;
        ScalableMeshDb* m_smSisterDb;
        SMSQLiteFilePtr m_mainFilePtr;
        SMSQLiteFilePtr m_sisterFilePtr;
        FILE*           m_scalableMeshFileLock; 


        uint32_t m_nbWorkers;            
        BeDuration m_lockSleeper;  

        HFCPtr<MeshIndexType> GetDataIndex();

        void FreeDataIndex();        

        void GetScalableMeshFileLock(bool readOnly);

        void ReleaseScalableMeshFileLock();
                
        void GetGenerationTasks(bvector<NodeTaskPtr>& toExecuteTasks, uint32_t maxGroupSize);

        void GetTextureTasks(bvector<NodeTaskPtr>& toExecuteTasks, uint32_t maxGroupSize);
        
        void GetTaskPlanFileName(BeFileName& taskPlanFileName) const;

        void CreateTaskPlanForTaskGrouping(uint32_t maxPriority, const WString& jobName, const BeFileName& smFileName);

        void GetSisterMainLockFileName(BeFileName& lockFileName) const;

        StatusInt CreateFilterTasks(uint32_t resolutionInd);

        StatusInt CreateStitchTasks(uint32_t resolutionInd);

        StatusInt CopyNextPriorityTasks(uint32_t priority);

        uint32_t GetNbNodesPerTask(size_t nbNodes) const;

        /*
        StatusInt CloseSqlFiles();

        StatusInt OpenSqlFiles(bool readOnly, bool needSisterMainLockFile);
        */

        
      
    protected:


    
    public:


        virtual void SetShareable(bool isShareable) override;

        explicit                            Impl(const WChar*            scmFileName, uint32_t nbWorkers, bool isSharable);
        explicit                            Impl(const IScalableMeshPtr& iDTMFilePtr, uint32_t nbWorkers, bool isSharable);

        

        virtual                             ~Impl();

        
        StatusInt                    CreateGenerationTasks(uint32_t maxGroupSize, const WString& jobName, const BeFileName& smFileName);

        StatusInt                    CreateTextureTasks(uint32_t maxGroupSize, const WString& jobName, const BeFileName& smFileName, int basePriority = 0);
        
        StatusInt                    CreateMeshTasks();        

        StatusInt                    CreateTaskPlan();
        
        StatusInt                    ExecuteNextTaskInTaskPlan();

        StatusInt                    ProcessMeshTask(BeXmlNodeP pXmlTaskNode);         

        StatusInt                    ProcessStitchTask(BeXmlNodeP pXmlTaskNode);

        StatusInt                    ProcessFilterTask(BeXmlNodeP pXmlTaskNode);       

        StatusInt                    ProcessGenerateTask(BeXmlNodeP pXmlTaskNode);  

        StatusInt                    ProcessTextureTask(BeXmlNodeP pXmlTaskNode);


        StatusInt                    OpenSqlFiles(bool readOnly, bool needSisterMainLockFile);

        StatusInt                    CloseSqlFiles();        
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE