//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------

   
#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"

/*----------------------------------------------------------------------+
| Include MicroStation SDK header files
+----------------------------------------------------------------------*/
//#include <toolsubs.h>
/*----------------------------------------------------------------------+
| Include Imagepp header files                                          |
+----------------------------------------------------------------------*/

#include "ScalableMesh.h"
#include "ScalableMeshQuadTreeBCLIBFilters.h"
#include "ScalableMeshQuadTreeBCLIBFilters.hpp"


template class ScalableMeshQuadTreeBCLIBFilter1<DPoint3d, DRange3d>;

template class ScalableMeshQuadTreeBCLIBMeshFilter1<DPoint3d, DRange3d>;

template class ScalableMeshQuadTreeBCLIBProgressiveFilter1<DPoint3d, DRange3d>;

template class ScalableMeshQuadTreeBCLIB_CGALMeshFilter<DPoint3d, DRange3d>;

//#ifdef WIP_MESH_IMPORT
template class ScalableMeshQuadTreeBCLIB_UserMeshFilter<DPoint3d, DRange3d>;
//#endif