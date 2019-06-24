/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshContourExtractor.h $
|    $RCSfile: ScalableMeshContourExtractor.h,v $
|   $Revision: 1.17 $
|       $Date: 2019/03/25 15:30:53 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <ScalableMesh/IScalableMeshQuery.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE



class ScalableMeshContourExtractor
{
public:
    ScalableMeshContourExtractor();
    ScalableMeshContourExtractor(ContoursParameters p);

    void GetMajorContours(bvector<bvector<DPoint3d>>& contoursMajor, IScalableMeshMeshPtr& meshP, double lowZ, double highZ);
    void GetMinorContours(bvector<bvector<DPoint3d>>& contoursMinor, IScalableMeshMeshPtr& meshP, double lowZ, double highZ);

private:
    ContoursParameters m_params;

    void GetContours(bvector<bvector<DPoint3d>>& contours, double spacing, IScalableMeshMeshPtr& meshP, double begin, double end);
};

END_BENTLEY_SCALABLEMESH_NAMESPACE
