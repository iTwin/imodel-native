/*--------------------------------------------------------------------------------------+
|
|     $Source: WorkerUtility/SMWorkerDefinitions.h $
|    $RCSfile: ScalableMeshDefs.h,v $
|   $Revision: 1.9 $
|       $Date: 2011/10/26 17:55:44 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*__PUBLISH_SECTION_START__*/
#pragma once

#include <Bentley/Bentley.h>
#include <ScalableMesh\ScalableMeshDefs.h>

#define BEGIN_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE    BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE namespace Worker {
#define END_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE     }}}
#define USING_NAMESPACE_BENTLEY_SCALABLEMESH_WORKER    using namespace BENTLEY_NAMESPACE_NAME::ScalableMesh::Worker;


BEGIN_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE

enum class WorkerTaskType
    {
    CUT = 0,
    FILTER,
    INDEX,
    MESH,
    STITCH,
    GENERATE,
    TASK_TYPE_QTY,
    };

END_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE
