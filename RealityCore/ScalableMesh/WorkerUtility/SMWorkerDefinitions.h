/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

/*__PUBLISH_SECTION_START__*/
#pragma once

#include <Bentley/Bentley.h>
#include <ScalableMesh/ScalableMeshDefs.h>

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
    TEXTURE,
    CREATETEXTURE,
    TASK_TYPE_QTY,
    };

END_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE
