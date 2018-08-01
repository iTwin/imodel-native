/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourceCreatorWorker.h $
|    $RCSfile: ScalableMeshSourceCreator.h,v $
|   $Revision: 1.45 $
|       $Date: 2015/07/15 11:02:24 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

#include "ScalableMeshSourceCreator.h"
#include <ScalableMesh/IScalableMeshSourceCreatorWorker.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Mathieu.St-Pierre    06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct IScalableMeshSourceCreatorWorker::Impl : public IScalableMeshSourceCreator::Impl
    {
    private:

        uint32_t m_nbWorkers;            

        HFCPtr<MeshIndexType> m_pDataIndex;

        HFCPtr<MeshIndexType> GetDataIndex();

        void GetTaskPlanFileName(BeFileName& taskPlanFileName) const;

        void GetSisterMainLockFileName(BeFileName& lockFileName) const;

        StatusInt CreateFilterTasks(uint32_t resolutionInd);

        StatusInt CreateStitchTasks(uint32_t resolutionInd);

        uint32_t GetNbNodesPerTask(size_t nbNodes) const;
        
      
    protected:
    
    public:
        explicit                            Impl(const WChar*            scmFileName, uint32_t nbWorkers);
        explicit                            Impl(const IScalableMeshPtr& iDTMFilePtr, uint32_t nbWorkers);

        

        virtual                             ~Impl();
        
        StatusInt                    CreateMeshTasks();

        StatusInt                    CreateTaskPlan();

        StatusInt                    ExecuteNextTaskInTaskPlan();

        StatusInt                    ProcessMeshTask(BeXmlNodeP pXmlTaskNode);         

        StatusInt                    ProcessStitchTask(BeXmlNodeP pXmlTaskNode);

        StatusInt                    ProcessFilterTask(BeXmlNodeP pXmlTaskNode);        
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE