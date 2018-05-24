/*--------------------------------------------------------------------------------------+
|
|     $Source: WorkerUtility/SMWorkerDefinitions.h $
|    $RCSfile: ScalableMeshDefs.h,v $
|   $Revision: 1.9 $
|       $Date: 2011/10/26 17:55:44 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*__PUBLISH_SECTION_START__*/
#pragma once

#include <Bentley/Bentley.h>

#define BEGIN_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE    BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE namespace ScalableMesh {
#define END_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE     }}}
#define USING_NAMESPACE_BENTLEY_SCALABLEMESH_WORKER    using namespace BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE::Worker;

BEGIN_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE

enum class WorkerTask
    {
    SMW_TASK_GENERATE = 0,
    SCM_FILTER_PROGRESSIVE_DUMB = 1,
    SCM_FILTER_DUMB_MESH = 2,
    SCM_FILTER_CGAL_SIMPLIFIER = 3,
    SCM_FILTER_QTY,
    };

END_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE
