//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/ScalableMeshMesher.cpp $
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
#include "ScalableMeshMesher.h"
#include "ScalableMeshMesher.hpp"
 
//template class ScalableMesh2DDelaunayMesher<DPoint3d, IDTMFile::Extent3d64f>;

//template class ScalableMeshAPSSOutOfCoreMesher<DPoint3d, IDTMFile::Extent3d64f>;

//template class ScalableMesh3DDelaunayMesher<DPoint3d, IDTMFile::Extent3d64f>;

template class ScalableMesh2DDelaunayMesher<DPoint3d, DRange3d>;

void ProcessFeatureDefinitions(bvector<bvector<DPoint3d>>& voidFeatures, bvector<DTMFeatureType>& types, bvector<bvector<DPoint3d>>& islandFeatures, const std::vector<DPoint3d>& nodePoints, BC_DTM_OBJ* dtmObjP, const std::vector<HPMStoredPooledVector<int32_t>>& featureDefs)
    {
    for (size_t i = 0; i < featureDefs.size(); ++i)
        {
        bvector<DPoint3d> feature;
        for (size_t j = 1; j < featureDefs[i].size(); ++j)
            {
            if (featureDefs[i][j] < nodePoints.size()) feature.push_back(nodePoints[featureDefs[i][j]]);
            }
        if ((DTMFeatureType)featureDefs[i][0] == DTMFeatureType::Void || (DTMFeatureType)featureDefs[i][0] == DTMFeatureType::Hole
            || (DTMFeatureType)featureDefs[i][0] == DTMFeatureType::BreakVoid || (DTMFeatureType)featureDefs[i][0] == DTMFeatureType::DrapeVoid)
            {

            if (featureDefs[i].size() > 4)
                voidFeatures.push_back(feature);
            types.push_back((DTMFeatureType)featureDefs[i][0]);
            }
        else if ((DTMFeatureType)featureDefs[i][0] == DTMFeatureType::Island)
            islandFeatures.push_back(feature);
        else bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, (DTMFeatureType)featureDefs[i][0], dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &feature[0], (long)feature.size());
        }
    }

int AddPolygonsToDTMObject(bvector<bvector<DPoint3d>>& polygons, DTMFeatureType type, BC_DTM_OBJ* dtmObjP)
    {
    int status = DTM_SUCCESS;
    for (auto& polygon : polygons)
        {
        if (polygon.size() > 2)
            {
            if (!bsiDPoint3d_pointEqualTolerance(&polygon.front(), &polygon.back(), 1e-8)) polygon.push_back(polygon.front());
            if (type == DTMFeatureType::Void)
                {
                status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::GraphicBreak, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(polygon[0]), (long)polygon.size());
                assert(status == SUCCESS);
                }
            status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, type, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(polygon[0]), (long)polygon.size());
            assert(status == SUCCESS);
            }
        }
    return status;
    }

int AddIslandsToDTMObject(bvector<bvector<DPoint3d>>& islandFeatures, bvector<bvector<DPoint3d>>& voidFeatures, BC_DTM_OBJ* dtmObjP)
    {
    VuPolygonClassifier vu(1e-8, 0);
    int status = DTM_SUCCESS;
    for (auto& island : islandFeatures)
        {
        bool intersect = false;
        for (auto& hole : voidFeatures)
            {
            vu.ClassifyAIntersectB(island, hole);
            bvector<DPoint3d> xyz;
            for (; vu.GetFace(xyz);)
                {
                intersect = true;
                status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::Island, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(xyz[0]), (long)xyz.size());
                assert(status == SUCCESS);
                }

            }
        if (!intersect)
            {
            status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::Island, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(island[0]), (long)island.size());
            
            }
        }
    return status;
    }