//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/ScalableMeshQuadTreeBCLIBFilters.cpp $
//:>    $RCSfile: ScalableMeshQuadTreeBCLIBFilters.cpp,v $
//:>   $Revision: 1.13 $
//:>       $Date: 2011/06/27 14:53:05 $
//:>     $Author: Alain.Robert $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

template class ScalableMeshQuadTreeBCLIBFilter1<DPoint3d, IDTMFile::Extent3d64f>;

template class ScalableMeshQuadTreeBCLIBMeshFilter1<DPoint3d, IDTMFile::Extent3d64f>;

template class ScalableMeshQuadTreeBCLIBProgressiveFilter1<DPoint3d, IDTMFile::Extent3d64f>;

template class ScalableMeshQuadTreeBCLIB_CGALMeshFilter<DPoint3d, IDTMFile::Extent3d64f>;
