/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshSourceCreatorWorker.h $
|    $RCSfile: IScalableMeshSourceCreator.h,v $
|   $Revision: 1.39 $
|       $Date: 2015/07/15 10:35:02 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/IScalableMeshSourceCreator.h>
#include <ScalableMesh/IScalableMeshSourceCreatorWorker.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IScalableMeshSourceCreatorWorker;
typedef RefCountedPtr<IScalableMeshSourceCreatorWorker>            IScalableMeshSourceCreatorWorkerPtr;

#define SUCCESS_TASK_PLAN_COMPLETE SUCCESS + 1

struct WorkerTimer
    {
    WorkerTimer()
        {
        m_indexingTime = 0;
        m_meshingTime = 0;
        m_stitchingTime = 0;
        m_filteringTime = 0;
        }
        

    double m_indexingTime;
    double m_meshingTime;
    double m_stitchingTime;
    double m_filteringTime;
    };

//This is the creator interface to use when providing a series of source files to import data to the Scalable Mesh. All details of indexing, etc are handled
//automatically. At the moment, it is not possible to import data from source files and also manually create nodes in the index.
struct IScalableMeshSourceCreatorWorker : public IScalableMeshSourceCreator
    {
    private:
        /*__PUBLISH_SECTION_END__*/
        friend struct                       IScalableMeshCreator;
        friend struct                       IScalableMeshSourceCreator;
        struct                              Impl;
        //std::auto_ptr<Impl>                 m_implP;

        explicit                            IScalableMeshSourceCreatorWorker(Impl*                       implP);

        /*__PUBLISH_SECTION_START__*/

    public:
        BENTLEY_SM_IMPORT_EXPORT virtual                 ~IScalableMeshSourceCreatorWorker();

        BENTLEY_SM_IMPORT_EXPORT static IScalableMeshSourceCreatorWorkerPtr GetFor(const WChar*              filePath,
                                                                             StatusInt&                status);

        BENTLEY_SM_IMPORT_EXPORT static IScalableMeshSourceCreatorWorkerPtr GetFor(const IScalableMeshPtr&     scmPtr,
                                                                             StatusInt&                  status);
        
        BENTLEY_SM_IMPORT_EXPORT StatusInt                    CreateTaskPlan() const;

        BENTLEY_SM_IMPORT_EXPORT StatusInt                    CreateMeshTasks() const;

        BENTLEY_SM_IMPORT_EXPORT StatusInt                    ExecuteNextTaskInTaskPlan() const;

        BENTLEY_SM_IMPORT_EXPORT StatusInt                    ProcessMeshTask(BeXmlNodeP pXmlTaskNode) const;

        BENTLEY_SM_IMPORT_EXPORT StatusInt                    ProcessStitchTask(BeXmlNodeP pXmlTaskNode) const;

        BENTLEY_SM_IMPORT_EXPORT StatusInt                    ProcessFilterTask(BeXmlNodeP pXmlTaskNode) const;        
                
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE