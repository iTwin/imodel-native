/*=================================================================================**//**
*
* !!!Describe Class Here!!!
*
* @bsiclass                                     		Thomas.Butzbach 11/2013
+===============+===============+===============+===============+===============+======*/
#pragma  once

//#include "PointCloudUtilities.h"
#include "PointCloudQuadTree.h"
#include "AutomaticGroundDetectionInternalConfig.h"
//#include <HFCThread.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

typedef RefCountedPtr<BcDTMMeshFace> IBcDTMMeshFacePtr;

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Thomas.Butzbach                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
class GroundDetection
{
    /*-----------------------------------------------------------------------------------
        Private Member variables
    -----------------------------------------------------------------------------------*/
private:
    static size_t       s_minimalTriangulableTilePointNum; 
    static float        s_minimalAcceptableTriangulationVerticeNumRatioVersusTileVerticeNum; 
    static int          s_maxRefinementLoop;   
    static unsigned int s_MaxNumberOfThreads;  
    
    // Mutex for multithreading
    static std::mutex   s_mutexamp;


    PointCloudQuadTree* m_pQuadTree;
    EditElementHandleR  m_elHandle;
    QuadSeedPtr         m_cloudSeed;
    bool                m_debugWriteInfo;
    GroundDetectionParametersPtr m_pParams;

    /*-----------------------------------------------------------------------------------
        Protected Member functions and variables
    -----------------------------------------------------------------------------------*/
protected:
    static bool isPointInsideExtent2D(DPoint3dCP pt, DPoint3dCP minCorner, DPoint3dCP maxCorner);
    
    //evaluate parameters
    bool        isPointWithinSlopeOfTin(double allowedSlope, DPoint3d& point, DPoint3d& projectedPoint, std::vector<DPoint3d>& tri);
    bool        getClosestTriangleToPoint(BC_DTM_OBJ* dtmObject, DPoint3d& point, DPoint3d& pointInFacePlane, std::vector<DPoint3d>& oTriangle, DVec3d& groundNormal, double tolerance, double angle);
   
    //triangulation utilities
    DPoint3d    nearest_vertex(DPoint3d& evaluatedPts, BcDTMMesh& pMesh);
    StatusInt   setTriangulation(std::vector<DPoint3d>& pPt, BC_DTM_OBJ* dtmP);
    void makeTriangulationForExtent(BC_DTM_OBJ* dtmObj, DPoint3d& minCorner, DPoint3d& maxCorner, std::vector<int>& remainingTriIndices, std::vector<DPoint3d>& insertedPointsPtr, PointCloudQuadNode* quadNode);
    void makeTriangulation(BC_DTM_OBJ* dtmObj, std::vector<DPoint3d>& ptsTriangulate);
    void        makeQuadTreeFromTriangulation(std::vector<DPoint3d>& points, std::vector<int>& indices);
    void        transformDataForSeedsTriangulation(std::vector<DPoint3d>& pPt);
    long        triangulation(DTM_TIN_POINT**& dtmpPtsPP, BC_DTM_OBJ* dtmObject);

    //compute statistical parameters for a triangulation
    void        computeStatisticsForTIN(double& allowedSlope, double& allowedHeight, double& heightgrad, BcDTMPtr& bcDtmObjPtr, double percentile);

    /*-----------------------------------------------------------------------------------
        Public Member functions
    -----------------------------------------------------------------------------------*/
public:
    GroundDetection(EditElementHandleR elHandle, PointCloudQuadTree* tree, GroundDetectionParametersCR params);
    ~GroundDetection();

    static std::mutex   s_mutexTriangle;

    void filterGroundForTile(QuadSeedPtr currentTile, BcDTMPtr dtmObj,int threadId = -1);
    StatusInt filterGround(std::vector<QuadSeedPtr>& seeds, EditElementHandleR elHandle, ProgressReportR report);
    void setDebugInfo(bool debug) {m_debugWriteInfo = debug;}

}; // GroundDetection

END_BENTLEY_SCALABLEMESH_NAMESPACE
