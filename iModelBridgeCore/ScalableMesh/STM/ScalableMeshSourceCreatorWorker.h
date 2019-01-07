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

struct GenerationTask;
typedef RefCountedPtr<GenerationTask> GenerationTaskPtr;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Mathieu.St-Pierre    06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct IScalableMeshSourceCreatorWorker::Impl : public IScalableMeshSourceCreator::Impl
    {
    private:

        ScalableMeshDb* m_smDb;
        ScalableMeshDb* m_smSisterDb;
        SMSQLiteFilePtr m_mainFilePtr;
        SMSQLiteFilePtr m_sisterFilePtr;


        uint32_t m_nbWorkers;            
        BeDuration m_lockSleeper;  

        HFCPtr<MeshIndexType> m_pDataIndex;

        HFCPtr<MeshIndexType> GetDataIndex();

        
        void GetGenerationTasks(bvector<GenerationTaskPtr>& toExecuteTasks);
        
        void GetTaskPlanFileName(BeFileName& taskPlanFileName) const;

        void GetSisterMainLockFileName(BeFileName& lockFileName) const;

        StatusInt CreateFilterTasks(uint32_t resolutionInd);

        StatusInt CreateStitchTasks(uint32_t resolutionInd);

        uint32_t GetNbNodesPerTask(size_t nbNodes) const;

        StatusInt CloseSqlFiles();

        StatusInt OpenSqlFiles(bool readOnly, bool needSisterMainLockFile);


        
      
    protected:
    
    public:
        explicit                            Impl(const WChar*            scmFileName, uint32_t nbWorkers);
        explicit                            Impl(const IScalableMeshPtr& iDTMFilePtr, uint32_t nbWorkers);

        

        virtual                             ~Impl();

        
        StatusInt                    CreateGenerationTasks();
        
        StatusInt                    CreateMeshTasks();        

        StatusInt                    CreateTaskPlan();

        StatusInt                    ExecuteNextTaskInTaskPlan();

        StatusInt                    ProcessMeshTask(BeXmlNodeP pXmlTaskNode);         

        StatusInt                    ProcessStitchTask(BeXmlNodeP pXmlTaskNode);

        StatusInt                    ProcessFilterTask(BeXmlNodeP pXmlTaskNode);       

        StatusInt                    ProcessGenerateTask(BeXmlNodeP pXmlTaskNode);         
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE