//#include "PointCloudEditPCH.h" //always first
#include "ScalableMeshPCH.h"
#include "../STM/ImagePPHeaders.h"
#include "AMPKernels.h"
using namespace concurrency;
#include "PointCloudClassification.h"
//#include "PointCloudEditChannelUndoRedoManager.h"
#include "PointCloudEditGroundDetection.h"
#include "TileLoaderQueue.h"
#include <DgnPlatform\DelegatedElementECEnabler.h>
#include <DgnPlatform\ECQuery.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

#include <DgnPlatform\DgnPlatformLib.h>
#include <DgnPlatform\PointCloudHandler.h>
#include <DgnPlatform\Tools\ConfigurationManager.h>
#include <PCLWrapper\INormalCalculator.h>
#include <eigen\Eigen\Dense>
#include <TerrainModel/Core/DTMIterators.h>


#ifdef SCALABLE_MESH_ATP
#include <ScalableMesh/IScalableMeshATP.h>
enum {
    ACCELERATOR_CPU = 0,
    ACCELERATOR_GPU
    };
#endif



//Boost includes
#pragma warning( disable : 4003 )
#pragma warning( disable : 4189 )

//https://connect.microsoft.com/VisualStudio/feedback/details/740466/doubted-bug-in-c-amp-library
#pragma warning( disable : 4505 )
#pragma warning( disable : 4872 )

#undef F1
#undef F2
#undef round

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/adapted/boost_polygon.hpp>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/algorithms/make.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/strategies/strategies.hpp>

using namespace boost::geometry;

BOOST_GEOMETRY_REGISTER_POINT_3D(DPoint3d, double, cs::cartesian, x, y, z)

#define GROUND_CHANNEL_NUMBER (2)

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
bool s_useFilteringOfSeedsBasedOnSlope = false;
bool s_useEstimatedParameters = true;
bool s_useEstimatedTileParameters = false;


/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard        11/2014
+===============+===============+===============+===============+===============+======*/
class  ProgressBarMonitor
    {
    public:
        ProgressBarMonitor(ProgressReport& report, size_t processSize, bool useMultiThread) :m_progressReport(report), m_processSize(processSize - 1), m_useMutiThread(useMultiThread)
            {
            s_process_canceled = false;
            s_NbTileProcessed = 0;
            s_pTimer = new StopWatch();
            s_pTimer->Start();
            }

        ~ProgressBarMonitor()
            {
            if (s_pTimer)
                delete s_pTimer;
            }

        //return true if process should continue (was not canceled)
        static bool UpdateProgressInThread()
            {
            std::unique_lock<std::mutex> lk(s_mutexProgress);
            bool shouldContinue = UpdateProgress();
            // Manual unlocking is done before notifying, to avoid waking up
            // the waiting thread only to block again
            lk.unlock();
            s_cvProgress.notify_one();
            return shouldContinue;
            }

        static void SignalErrorInThread()
            {
            std::unique_lock<std::mutex> lk(s_mutexProgress);
            SignalError();
            lk.unlock();
            s_cvProgress.notify_one();
            }

        //return true if process should continue (was not canceled)
        static bool UpdateProgress()
            {
            //Always increment
            s_NbTileProcessed++;
            s_currentNbSecondsAtLastUpdate = s_pTimer->GetCurrentSeconds();
            bool shouldContinue(!s_process_canceled);
            return shouldContinue;
            }

        static void SignalError()
            {
            s_process_canceled = true;
            s_process_errorSignaled = true;
            }

        void FinalStageProcessing()
            {
            m_progressReport.SetWorkDone(1.0);
            s_process_canceled = !m_progressReport.CheckContinueOnProgress();
            }


        //return true if process should continue (was not canceled)
        bool InProgress() const
            {
            if (m_useMutiThread)
                {
                //Don't want to update estimated time too often as it make it blink...
                std::unique_lock<std::mutex> lk(s_mutexProgress);
                cv_status waitStatus = s_cvProgress.wait_for(lk, chrono::milliseconds(2000));

                //Update progress bar only if an update was signaled, otherwise we only check if process was canceled
                //if (waitStatus == cv_status::no_timeout)
                    {
                    ShowProgress();
                    }
                bool isCanceled = CheckCancel();
                return isCanceled;
                }
            ShowProgress();

            return CheckCancel();
            }
    private:
        static std::mutex               s_mutexProgress;
        static std::condition_variable  s_cvProgress;

        static StopWatch*   s_pTimer;
        static double       s_currentNbSecondsAtLastUpdate;
        static size_t       s_NbTileProcessed;
        static bool         s_process_canceled;
        static bool         s_process_errorSignaled;

        bool                             m_useMutiThread;
        size_t                           m_processSize;
        ProgressReport&                  m_progressReport;

        //return true if process should continue (was not canceled)
        bool ShowProgress() const
            {
            if (s_process_errorSignaled)
                {
                m_progressReport.OnSignalError();
                return false;
                }

            size_t nbTileProcessed(s_NbTileProcessed);
            double currentNbSeconds(s_currentNbSecondsAtLastUpdate);

            double accomplishmentRatio = (static_cast<double>(nbTileProcessed) / static_cast<double>(m_processSize));
            double remainingSeconds(0);
            if (accomplishmentRatio != 0.0)
                remainingSeconds = ((1.0 / accomplishmentRatio) * currentNbSeconds) - currentNbSeconds;

            m_progressReport.SetEstimatedRemainingTime(true, remainingSeconds);
            m_progressReport.SetWorkDone(accomplishmentRatio);
            s_process_canceled = !m_progressReport.CheckContinueOnProgress();

            return (nbTileProcessed < m_processSize && !s_process_canceled);
            }

        //return true if process should continue (was not canceled)
        bool CheckCancel() const
            {
            //Progress and cancellation check
            if (!s_process_canceled)
                s_process_canceled = !m_progressReport.CheckContinueOnProgress();
            return (s_NbTileProcessed < m_processSize && !s_process_canceled);
            }
    }; // ProgressBarMonitor


//Static variable initialization
size_t      GroundDetection::s_minimalTriangulableTilePointNum = 300;
float       GroundDetection::s_minimalAcceptableTriangulationVerticeNumRatioVersusTileVerticeNum = 0.05; 
int         GroundDetection::s_maxRefinementLoop = 1;//20;  
unsigned int GroundDetection::s_MaxNumberOfThreads = 8; 
std::mutex  GroundDetection::s_mutexTriangle;
std::mutex  GroundDetection::s_mutexamp;

StopWatch*  ProgressBarMonitor::s_pTimer = NULL;
double      ProgressBarMonitor::s_currentNbSecondsAtLastUpdate = 0;
bool        ProgressBarMonitor::s_process_canceled = (false);
bool        ProgressBarMonitor::s_process_errorSignaled = (false);
size_t      ProgressBarMonitor::s_NbTileProcessed = 0;
std::mutex  ProgressBarMonitor::s_mutexProgress;
std::condition_variable ProgressBarMonitor::s_cvProgress;



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline double sign(DPoint3d p1, DPoint3d p2, DPoint3d p3)
    {
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool pointWithinTriangle3D(const DPoint3d v1, const DPoint3d v2, const DPoint3d v3, DPoint3d& query)
    {
    bool b1, b2, b3;

    b1 = sign(query, v1, v2) < 0.0f;
    b2 = sign(query, v2, v3) < 0.0f;
    b3 = sign(query, v3, v1) < 0.0f;

    return ((b1 == b2) && (b2 == b3));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline double pointDistance(DPoint3d& p1, DPoint3d& p2)
    {
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2) + pow(p1.z - p2.z, 2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
GroundDetection::GroundDetection(EditElementHandleR elHandle, PointCloudQuadTree* tree, GroundDetectionParametersCR params) : m_elHandle(elHandle), m_pQuadTree(tree), m_pParams(GroundDetectionParameters::Clone(params))
    {

    s_minimalTriangulableTilePointNum = 300; 
    s_minimalAcceptableTriangulationVerticeNumRatioVersusTileVerticeNum = 0.05; 
    s_maxRefinementLoop = 20; 
    s_MaxNumberOfThreads = 8; 

    WString cfgVarValueStr;
    if (BSISUCCESS == ConfigurationManager::GetVariable(cfgVarValueStr, L"POINTCLOUDADV_MIN_TILE_PT_NUM"))
        {
        size_t  minimalTriangulableTilePointNum;
        if (1 == swscanf(cfgVarValueStr.c_str(), L"%ld", &minimalTriangulableTilePointNum))
            {
            s_minimalTriangulableTilePointNum = minimalTriangulableTilePointNum;
            }
        }
    if (BSISUCCESS == ConfigurationManager::GetVariable(cfgVarValueStr, L"POINTCLOUDADV_MIN_VERTICE_RATIO"))
        {
        float minimalAcceptableTriangulationVerticeNumRatioVersusTileVerticeNum;
        if (1 == swscanf(cfgVarValueStr.c_str(), L"%f", &minimalAcceptableTriangulationVerticeNumRatioVersusTileVerticeNum))
            {
            s_minimalAcceptableTriangulationVerticeNumRatioVersusTileVerticeNum = minimalAcceptableTriangulationVerticeNumRatioVersusTileVerticeNum;
            }
        }
    if (BSISUCCESS == ConfigurationManager::GetVariable(cfgVarValueStr, L"POINTCLOUDADV_MAX_REFINEMENT_LOOP"))
        {
        int maxRefinementLoop;
        if (1 == swscanf(cfgVarValueStr.c_str(), L"%d", &maxRefinementLoop))
            {
            s_maxRefinementLoop = maxRefinementLoop;
            }
        }
    if (BSISUCCESS == ConfigurationManager::GetVariable(cfgVarValueStr, L"POINTCLOUDADV_MAX_THREAD"))
        {
        int maxNumberOfThreads;
        if (1 == swscanf(cfgVarValueStr.c_str(), L"%d", &maxNumberOfThreads))
            {
            s_MaxNumberOfThreads = maxNumberOfThreads;
            }
        }
#ifdef SCALABLE_MESH_ATP
    int64_t useCpuValue = 0;
    IScalableMeshATP::GetInt(ATP_GROUNDDETECTION_FORCE_USE_CPU, useCpuValue);
    if (useCpuValue > 0) m_pParams->SetProcessingStrategy(GroundDetectionParameters::ProcessingStrategy::USE_CPU_ONLY);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
GroundDetection::~GroundDetection()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void GroundDetection::transformDataForSeedsTriangulation(std::vector<DPoint3d>& pPt)
    {
    for (std::vector<DPoint3d>::iterator it = m_cloudSeed->seedPoints.begin(); it != m_cloudSeed->seedPoints.end(); it++)
        {
        DPoint3d p = { it->x, it->y, it->z };
        pPt.push_back(p);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
// Initialize data for triangulation (initialize dtmP)
StatusInt GroundDetection::setTriangulation(std::vector<DPoint3d>& pPt, BC_DTM_OBJ* dtmP)
    {
    static long   edgeOption = 2;         // ==> Triangulation Edge Option <1,2,3> Usually 2
    static double maxSideLength = 250;    // ==> Max Triangle Side Length For External Triangles ** Required For Edge Option = 3
    static double p2pTolerance = 0.0001;  // ==> Triangulation Point to Point Tolerance Usually 0.0001

    int status;

    //Triangulation code
    status = bcdtmObject_initialiseDtmObject(dtmP);

    BeAssert(status == DTM_SUCCESS);
    if (status != DTM_SUCCESS) return status;

    status = bcdtmObject_storeDtmFeatureInDtmObject(dtmP, DTMFeatureType::RandomSpots, DTM_NULL_USER_TAG, 1, &(dtmP)->nullFeatureId, &pPt[0], (long) pPt.size());

    BeAssert(status == DTM_SUCCESS);
    if (status != DTM_SUCCESS) return status;

    status = bcdtmObject_setTriangulationParametersDtmObject(dtmP, p2pTolerance, p2pTolerance, edgeOption, maxSideLength);

    BeAssert(status == DTM_SUCCESS);
    if (status != DTM_SUCCESS) return status;

    status = bcdtmObject_triangulateDtmObject(dtmP);

    BeAssert(status == DTM_SUCCESS);
    if (status != DTM_SUCCESS) return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool GroundDetection::isPointInsideExtent2D(DPoint3dCP pt, DPoint3dCP minCorner, DPoint3dCP maxCorner)
    {
    DRange2d range;
    range.initFrom(minCorner->x, minCorner->y, maxCorner->x, maxCorner->y);
    return range.contains(pt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void GroundDetection::makeQuadTreeFromTriangulation(std::vector<DPoint3d>& points, std::vector<int>& indices)
    {
    m_pQuadTree->setTriangles(points, indices);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void GroundDetection::makeTriangulation(BC_DTM_OBJ* dtmObj, std::vector<DPoint3d>& ptsTriangulate)
    {
    /*std::vector<DPoint3d> pPt;

    for (size_t i = 0; i < ptsTriangulate.size(); i++)
        {
        DPoint3d p = { ptsTriangulate[i].x, ptsTriangulate[i].y, ptsTriangulate[i].z };
        pPt.push_back(p);
        }*/

    StatusInt status = setTriangulation(ptsTriangulate, dtmObj);
    BeAssert(status == SUCCESS);

  //  return dtmObject;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void GroundDetection::makeTriangulationForExtent(BC_DTM_OBJ* dtmObj, DPoint3d& minCorner, DPoint3d& maxCorner, std::vector<int>& remainingTriIndices, std::vector<DPoint3d>& insertedPointsPtr, PointCloudQuadNode* quadNode)
    {
    // Calculate extent
    std::vector<size_t> triangleIDs;
    DPoint3d middle;
    middle.x = (maxCorner.x - minCorner.x) / 2.0 + minCorner.x;
    middle.y = (maxCorner.y - minCorner.y) / 2.0 + minCorner.y;
    middle.z = (maxCorner.z - minCorner.z) / 2.0 + minCorner.z;

    // All triangles in this extent
    triangleIDs = quadNode->findTries(middle);
    BeAssert(triangleIDs.size() != 0);

    // triangulation
    std::vector<bool> triangleToTestCornersAgainst(triangleIDs.size());

    int remainingIndicesIndex = 0;
    remainingTriIndices.resize(triangleIDs.size() * 3);

    std::set<int> insideIndiceSet;

    for (size_t i = 0; i < triangleIDs.size(); i++)
        {
        bool indiceInside[3];
        indiceInside[0] = indiceInside[1] = indiceInside[2] = false;
        int indices[3];

        indices[0] = m_pQuadTree->GetTriangleIndexe(triangleIDs[i]);
        indices[1] = m_pQuadTree->GetTriangleIndexe(triangleIDs[i] + 1);
        indices[2] = m_pQuadTree->GetTriangleIndexe(triangleIDs[i] + 2);

        int nbIn = 0;
        DPoint3d pt = m_pQuadTree->GetPoint(indices[0]);
        if (isPointInsideExtent2D(&pt, &minCorner, &maxCorner))
            {
            indiceInside[0] = true;
            nbIn++;
            }
        else
            indiceInside[0] = false;

        pt = m_pQuadTree->GetPoint(indices[1]);
        if (isPointInsideExtent2D(&pt, &minCorner, &maxCorner))
            {
            indiceInside[1] = true;
            nbIn++;
            }
        else
            indiceInside[1] = false;

        pt = m_pQuadTree->GetPoint(indices[2]);
        if (isPointInsideExtent2D(&pt, &minCorner, &maxCorner))
            {
            indiceInside[2] = true;
            nbIn++;
            }
        else
            indiceInside[2] = false;

        BeAssert((indices[0] != indices[1]) && (indices[1] != indices[2]) && (indices[2] != indices[0]));

        insideIndiceSet.insert(indices[0]);
        insideIndiceSet.insert(indices[1]);
        insideIndiceSet.insert(indices[2]);

        indiceInside[0] = true;
        indiceInside[1] = true;
        indiceInside[2] = true;

        if (nbIn < 3)
            {
            remainingTriIndices[3 * i] = indices[0];
            remainingTriIndices[(3 * i) + 1] = indices[1];
            remainingTriIndices[(3 * i) + 2] = indices[2];

            remainingIndicesIndex += 3;
            }
        }
    remainingTriIndices.resize(remainingIndicesIndex);

    // Triangulation delaunay
    std::vector<DPoint3d> pPt;
    for (std::set<int>::iterator it = insideIndiceSet.begin(); it != insideIndiceSet.end(); it++)
        {
        DPoint3d pt(m_pQuadTree->GetPoint(*it));
        insertedPointsPtr.push_back(pt);
        pPt.push_back(pt);
        }

    StatusInt status = setTriangulation(pPt, dtmObj);
    BeAssert(status == SUCCESS);

    //return dtmObject;
    }

bool getClosestTriangleToPointPCL(DPoint3d& point, DPoint3d& pointInFacePlane, std::vector<DPoint3d>& oTriangle, void* pclTree, DPoint3d* allPoints)
    {
    int indices[10];
    size_t ct;
    Eigen::Vector3f pt(point.x, point.y, point.z);
    PCLUtility::INormalCalculator::NearestKSearch(indices,
                                                  &ct,
                                                  pclTree,
                                                  pt,
                                                  FLT_MAX,
                                                  3);
    if (ct < 3) return false;
    oTriangle.resize(3);
    oTriangle[0] = allPoints[indices[0]];
    oTriangle[1] = allPoints[indices[1]];
    oTriangle[2] = allPoints[indices[2]];
    DPoint3d bary;
    if (!bsiDPoint3d_barycentricFromDPoint3dTriangle(&bary, &pointInFacePlane, &oTriangle[0], &oTriangle[1], &oTriangle[2]) && bary.x >= 0.0 && bary.x <= 1.0 && bary.y >= 0.0 && bary.y <= 1.0 && bary.z >= 0.0 && bary.z <= 1.0)
        return false;
    DPlane3d planeFromFace2;
    planeFromFace2.initFrom3Points(&oTriangle[0], &oTriangle[1], &oTriangle[2]);
    planeFromFace2.projectPoint(&pointInFacePlane, &point);
    return true;
    } 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool GroundDetection::getClosestTriangleToPoint(BC_DTM_OBJ* dtmObject, DPoint3d& point, DPoint3d& pointInFacePlane, std::vector<DPoint3d>& oTriangle, DVec3d& groundNormal, double tolerance, double angle)
    {
    std::map<double, std::vector<DPoint3d>> triangles;

    // Found the nearest vertex of triangle in DTM from point
    long cPointP;
    int test = bcdtmFind_closestPointDtmObject(dtmObject, point.x, point.y, &cPointP);
    DPoint3d nearestVertex;
    if (test != SUCCESS)
        {
        nearestVertex.x = pointAddrP(dtmObject, cPointP)->x;
        nearestVertex.y = pointAddrP(dtmObject, cPointP)->y;
        nearestVertex.z = pointAddrP(dtmObject, cPointP)->z;
        }

    long fndTypeP, pnt1P, pnt2P, pnt3P;
    double Zs;
    StatusInt status = bcdtmFind_triangleForPointDtmObject(dtmObject, point.x, point.y, &Zs, &fndTypeP, &pnt1P, &pnt2P, &pnt3P);
    BeAssert(status == SUCCESS);
    if (fndTypeP < 4)
        {
        return false;
        }
    else
        {
        DPoint3d p1;
        p1.x = pointAddrP(dtmObject, pnt1P)->x;
        p1.y = pointAddrP(dtmObject, pnt1P)->y;
        p1.z = pointAddrP(dtmObject, pnt1P)->z;

        DPoint3d p2;
        p2.x = pointAddrP(dtmObject, pnt2P)->x;
        p2.y = pointAddrP(dtmObject, pnt2P)->y;
        p2.z = pointAddrP(dtmObject, pnt2P)->z;

        DPoint3d p3;
        p3.x = pointAddrP(dtmObject, pnt3P)->x;
        p3.y = pointAddrP(dtmObject, pnt3P)->y;
        p3.z = pointAddrP(dtmObject, pnt3P)->z;

        std::vector<DPoint3d> points(3);
        points[0] = p1;
        points[1] = p2;
        points[2] = p3;

        DPlane3d planeFromFace;
        planeFromFace.initFrom3Points(&p1, &p2, &p3);
        DPoint4d eq;
        planeFromFace.getDPoint4d(&eq);
        planeFromFace.projectPoint(&pointInFacePlane, &point);

        double normalHeight = pointDistance(point, pointInFacePlane);

        DVec3d normal, flatPlane;
        DPoint3d normTmp, flatTmp;
        normTmp = pointInFacePlane;
        normTmp.subtract(&point);
        normal.init(&normTmp);
        flatTmp = pointInFacePlane;
        flatTmp.subtract(&pointInFacePlane);
        flatTmp.subtract(&groundNormal);
        flatPlane.init(&flatTmp);


        double angleCos = (normal.dotProduct(&flatPlane)) / (sqrt(normal.magnitudeSquared())*sqrt(flatPlane.magnitudeSquared()));
        double pi50 = 50 * PI / 180;
        //if (acos(abs(angleCos)) < pi50)
        DPoint3d bary;
        if (bsiDPoint3d_barycentricFromDPoint3dTriangle(&bary, &pointInFacePlane, &p1, &p2, &p3) && bary.x >= 0.0 && bary.x <= 1.0 && bary.y >= 0.0 && bary.y <= 1.0 && bary.z >= 0.0 && bary.z <= 1.0)
            triangles.insert(make_pair(normalHeight, points));
        }

    if (triangles.size() == 0)
        return false;
    oTriangle = triangles.begin()->second;

    triangles.erase(triangles.begin(), triangles.end());
    DPlane3d planeFromFace2;
    planeFromFace2.initFrom3Points(&oTriangle[0], &oTriangle[1], &oTriangle[2]);
    planeFromFace2.projectPoint(&pointInFacePlane, &point);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool GroundDetection::isPointWithinSlopeOfTin(double allowedSlope, DPoint3d& point, DPoint3d& projectedPoint, std::vector<DPoint3d>& tri)
    {
    for (int i = 0; i < 3; i++)
        {
        DVec3d triPoint;
        DVec3d triProj;

        triPoint.init(tri[i].x - point.x, tri[i].y - point.y, tri[i].z - point.z);
        triProj.init(tri[i].x - projectedPoint.x, tri[i].y - projectedPoint.y, tri[i].z - projectedPoint.z);

        double angle1 = triProj.angleTo(&triPoint);

        double angleDeg1 = angle1 * 180 / PI;

        BeAssert(angleDeg1 >= 0);
        if ((angleDeg1) > allowedSlope)
            return false;

        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
long GroundDetection::triangulation(DPoint3d**& dtmPtsPP, BC_DTM_OBJ* dtmObject)
    {
    long numTriangles = 0;
    assert(false);
   /* long* trianglesP = 0;
    int status = bcdtmCheck_tinComponentDtmObject(dtmObject);

    BeAssert(status == SUCCESS);

    status = bcdtmLoad_tinTrianglesFromDtmObject(dtmObject,
        FALSE,
        DTMFenceType::Block,
        0,
        0,
        &numTriangles,
        &trianglesP,
        &dtmPtsPP);

    BeAssert(status == SUCCESS);

    delete trianglesP;*/

    return numTriangles;
    }

void addFaceToEdge(bvector<bvector<bpair<int, bvector<int>>>>& edgeMap, int idx1, int idx2, int i)
    {
    auto idx1_it = std::find_if(edgeMap[idx1].begin(), edgeMap[idx1].end(), [idx2] (bpair<int, bvector<int>>& edge) { return edge.first == idx2; });
    if (idx1_it == edgeMap[idx1].end())
        {
        edgeMap[idx1].push_back(make_bpair(idx2, bvector<int>()));
        idx1_it = edgeMap[idx1].end() - 1;
        }
    idx1_it->second.push_back(i);
    }

void filterInitialTriangulation(std::vector<DPoint3d>& insertedPoints, std::vector<DPoint3d>& toDeletePts,const DPoint3d* dtmP, BcDTMPtr& bcDtmObjPtr, double angle, double height)
    {

    //BcDTMMeshPtr mesh = bcDtmObjPtr->GetMesh(TRUE, bcDtmObjPtr->GetTrianglesCount(), NULL, 0);
    DTMMeshEnumeratorPtr enumerator = DTMMeshEnumerator::Create (*bcDtmObjPtr);
    enumerator->SetMaxTriangles (100000000); // Otherwise it could return more than one section.

    //std::vector<DPoint3d> points;
    //std::vector<int> indices;
    for (auto pf : *enumerator)
        {
        bvector<bvector<int>> listOfFaces(pf->GetPointCount());
        bvector<bvector<bpair<int, bvector<int>>>> edgeMap(pf->GetPointCount());
        for (size_t i = 0; i < pf->GetPointIndexCount(); i+=3)
            {
            //auto face = mesh->GetFace((long)i);
            int idx1 = pf->GetPointIndexCP()[i] - 1, idx2 = pf->GetPointIndexCP()[i+1] - 1, idx3 = pf->GetPointIndexCP()[i+2] - 1;
            listOfFaces[idx1].push_back((int)i);
            listOfFaces[idx2].push_back((int)i);
            listOfFaces[idx3].push_back((int)i);
            addFaceToEdge(edgeMap, idx1, idx2, idx3);
            addFaceToEdge(edgeMap, idx1, idx3, idx2);
            addFaceToEdge(edgeMap, idx2, idx1, idx3);
            addFaceToEdge(edgeMap, idx2, idx3, idx1);
            addFaceToEdge(edgeMap, idx3, idx1, idx2);
            addFaceToEdge(edgeMap, idx3, idx2, idx1);
            }
        for (size_t i = 0; i < listOfFaces.size(); i++)
            {
            bvector<DVec3d> normals;
            for (size_t j = 0; j < listOfFaces[i].size(); j++)
                {
                //auto face = mesh->GetFace((long)listOfFaces[i][j]);
                DPoint3d pt1 = pf->GetPointCP()[(long)pf->GetPointIndexCP()[listOfFaces[i][j]] - 1], pt2 = pf->GetPointCP()[pf->GetPointIndexCP()[listOfFaces[i][j]+1] - 1];
                DPoint3d pt3 = pf->GetPointCP()[(long)pf->GetPointIndexCP()[listOfFaces[i][j]+2] - 1];
                DPlane3d pl = DPlane3d::From3Points(pt1, pt2, pt3);
                normals.push_back(pl.normal);
                }
            size_t nOutlierNormals = 0;
            for (auto& normal : normals)
                {
                for (auto& normal2 : normals)
                    {
                    if (normal.SmallerUnorientedAngleTo(normal2) > angle*PI / 180)
                        {
                        nOutlierNormals++;
                        break;
                        }
                    }
                }
            if (nOutlierNormals >= 3)
                {
                toDeletePts.push_back(dtmP[i]);
                }
            }
        bvector<size_t> mapOfThirdPoints(pf->GetPointCount(),0);
        for (size_t i = 0; i <edgeMap.size(); ++i)
            {
            for (size_t j = 0; j <edgeMap[i].size(); ++j)
                {
                if (edgeMap[i][j].second.size() <= 1) continue;
                DPoint3d pt1 = pf->GetPointCP()[(long)i], pt2 = pf->GetPointCP()[(long)edgeMap[i][j].first];
                DPoint3d pt3 = pf->GetPointCP()[(long)edgeMap[i][j].second[0]], pt4 = pf->GetPointCP()[(long)edgeMap[i][j].second[1]];
                DVec3d vec1 = DPlane3d::From3Points(pt1, pt2, pt3).normal, vec2 = DPlane3d::From3Points(pt1, pt2, pt4).normal;
                DPlane3d pl1 = DPlane3d::From3Points(pt1, pt2, pt3), pl2 = DPlane3d::From3Points(pt1, pt2, pt4);
                DPoint3d proj1, proj2;
                pl1.ProjectPoint(proj1, pt4);
                pl2.ProjectPoint(proj2, pt3);
                if (DVec3d::FromStartEnd(proj1, pt4).Magnitude() > height)mapOfThirdPoints[edgeMap[i][j].second[1]]++;
                if (DVec3d::FromStartEnd(proj2, pt3).Magnitude() > height) mapOfThirdPoints[edgeMap[i][j].second[0]]++;
                }
            }
        for (size_t i = 0; i < mapOfThirdPoints.size(); i++)
            {
            if (mapOfThirdPoints[i] > 1) toDeletePts.push_back(dtmP[i]);
            }
        for (size_t i = 0; i < toDeletePts.size(); i++)
            {
            DPoint3d toDelete = toDeletePts[i];
            auto it = std::find_if(insertedPoints.begin(), insertedPoints.end(), [&toDelete] (DPoint3d& pt) { return fabs(toDelete.x - pt.x) <= 0.001 && fabs(toDelete.y - pt.y) <= 0.001 && fabs(toDelete.z - pt.z) <= 0.001; });
            if (it != insertedPoints.end()) insertedPoints.erase(it);
            }
        }
    }


void GroundDetection::computeStatisticsForTIN(double& allowedSlope, double& allowedHeight, double& heightgrad, BcDTMPtr& bcDtmObjPtr, double percentile)
    {
    std::set<double> heightStats;
    std::set<double> heightGradStats;
    std::set<double> slopeStats;
    // get all triangles
    //BcDTMMeshPtr mesh = bcDtmObjPtr->GetMesh(TRUE, bcDtmObjPtr->GetTrianglesCount(), NULL, 0);
    DTMMeshEnumeratorPtr enumerator = DTMMeshEnumerator::Create (*bcDtmObjPtr);
    enumerator->SetMaxTriangles (100000000); // Otherwise it could return more than one section.

    //std::vector<DPoint3d> points;
    //std::vector<int> indices;
    for (auto pf : *enumerator)
        {
        bvector<bvector<bpair<int, bvector<int>>>> edgeMap(pf->GetPointCount());
        bvector<bvector<double>> distMap(pf->GetPointCount());
        double avgDistance = 0;
        for (size_t i = 0; i < pf->GetPointIndexCount(); i+=3)
            {
            //auto face = pf->GetPointIndexCP()[(long)i];
            int idx1 = pf->GetPointIndexCP()[(long)i] - 1, idx2 = pf->GetPointIndexCP()[(long)i+1] - 1, idx3 = pf->GetPointIndexCP()[(long)i+2] - 1;
            assert(idx1 >= 0 && idx2 >= 0 && idx3 >= 0);
            addFaceToEdge(edgeMap, idx1, idx2, idx3);
            addFaceToEdge(edgeMap, idx1, idx3, idx2);
            addFaceToEdge(edgeMap, idx2, idx1, idx3);
            addFaceToEdge(edgeMap, idx2, idx3, idx1);
            addFaceToEdge(edgeMap, idx3, idx1, idx2);
            addFaceToEdge(edgeMap, idx3, idx2, idx1);
            DPoint3d pt1 = pf->GetPointCP()[(long)idx1], pt2 = pf->GetPointCP()[(long)idx2];
            DPoint3d pt3 = pf->GetPointCP()[(long)idx3];
            double dist12 = DVec3d::FromStartEnd(pt1, pt2).Magnitude(), dist13 = DVec3d::FromStartEnd(pt1, pt3).Magnitude(),
                dist23 = DVec3d::FromStartEnd(pt3, pt2).Magnitude();
            distMap[idx1].push_back((dist12 + dist13) / 2);
            distMap[idx2].push_back((dist12 + dist23) / 2);
            distMap[idx3].push_back((dist23 + dist13) / 2);
            avgDistance += dist12 + dist13 + dist23;
            }
        avgDistance /= pf->GetPointIndexCount() * 3;
        for (size_t i = 0; i < pf->GetPointCount(); ++i)
            {
            DPoint3d pt = pf->GetPointCP()[(long)i];
            heightGradStats.insert(pt.z);
            }

        for (size_t i = 0; i <edgeMap.size(); ++i)
            {
            for (size_t j = 0; j <edgeMap[i].size(); ++j)
                {
                if (edgeMap[i][j].second.size() <= 1) continue;
                double dist1 = std::accumulate(distMap[i].begin(), distMap[i].end(), 0.0) / distMap[i].size();
                double dist2 = std::accumulate(distMap[edgeMap[i][j].first].begin(), distMap[edgeMap[i][j].first].end(), 0.0) / distMap[edgeMap[i][j].first].size();
                if (dist1 > avgDistance && dist2 > avgDistance) continue;
                DPoint3d pt1 = pf->GetPointCP()[(long)i], pt2 = pf->GetPointCP()[(long)edgeMap[i][j].first];
                DPoint3d pt3 = pf->GetPointCP()[(long)edgeMap[i][j].second[0]], pt4 = pf->GetPointCP()[(long)edgeMap[i][j].second[1]];
                DVec3d vec1 = DPlane3d::From3Points(pt1, pt2, pt3).normal, vec2 = DPlane3d::From3Points(pt1, pt2, pt4).normal;
                slopeStats.insert(vec1.SmallerUnorientedAngleTo(vec2));
                DPlane3d pl1 = DPlane3d::From3Points(pt1, pt2, pt3), pl2 = DPlane3d::From3Points(pt1, pt2, pt4);
                DPoint3d proj1, proj2;
                pl1.ProjectPoint(proj1, pt4);
                pl2.ProjectPoint(proj2, pt3);
                heightStats.insert(DVec3d::FromStartEnd(proj1, pt4).Magnitude());
                heightStats.insert(DVec3d::FromStartEnd(proj2, pt3).Magnitude());
                }
            }
        allowedSlope = *std::next(slopeStats.begin(), (int)slopeStats.size()* percentile/100);
        allowedHeight = *std::next(heightStats.begin(), (int)heightStats.size()*percentile/100);
        heightgrad = *std::next(heightGradStats.begin(), (int)heightGradStats.size()*percentile / 100);
        heightgrad -= *heightGradStats.begin();
        allowedSlope *= 180 / PI;
        }
    }


void populateTriangleList(std::vector<FPoint3d>& allTri, BcDTMPtr& bcDtmObjPtr, DPoint3d& minPt)
    {
    //BcDTMMeshPtr mesh;
    PolyfaceQueryP pf;
    DTMMeshEnumeratorPtr enumerator = DTMMeshEnumerator::Create (*bcDtmObjPtr);
    enumerator->SetMaxTriangles (100000000); 
        {
        std::lock_guard<std::mutex> lock(GroundDetection::s_mutexTriangle);
        //mesh = bcDtmObjPtr->GetMesh(TRUE, bcDtmObjPtr->GetTrianglesCount(), NULL, 0);
        pf = *(enumerator->begin());
        }

    for (size_t i = 0; pf!= NULL && i < pf->GetPointIndexCount(); i+=3)
        {
        //auto face = mesh->GetFace((long)i);
        int idx1 = pf->GetPointIndexCP()[i] - 1, idx2 = pf->GetPointIndexCP()[i+1] - 1, idx3 = pf->GetPointIndexCP()[i+2] - 1;
        DPoint3d pt1 = pf->GetPointCP()[(long)idx1], pt2 = pf->GetPointCP()[(long)idx2];
        DPoint3d pt3 = pf->GetPointCP()[(long)idx3];
        pt1.DifferenceOf(pt1, minPt);
        pt2.DifferenceOf(pt2, minPt);
        pt3.DifferenceOf(pt3, minPt);
        FPoint3d fpt1, fpt2, fpt3;
        bsiFPoint3d_initFromDPoint3d(&fpt1, &pt1);
        bsiFPoint3d_initFromDPoint3d(&fpt2, &pt2);
        bsiFPoint3d_initFromDPoint3d(&fpt3, &pt3);
        allTri.push_back(fpt1);
        allTri.push_back(fpt2);
        allTri.push_back(fpt3);
        }
    }



bool pick_accelerator(GroundDetectionParameters::ProcessingStrategy useCpu)
    {
    std::vector<accelerator> accs = accelerator::get_all();
    accelerator chosen_one;
    accelerator cpu_acc(accelerator::cpu_accelerator); //for filtering out
    auto result =
        std::find_if(accs.begin(), accs.end(), [cpu_acc] (const accelerator& acc)
        {
        return !acc.is_emulated &&
            acc.device_path.compare(cpu_acc.device_path) != 0 &&
            !acc.has_display;
        });

    if (result != accs.end())
        chosen_one = *(result); //pick the first dedicated GPU

    else if (useCpu == GroundDetectionParameters::ProcessingStrategy::USE_ANY_GPU)
        {
        result =
            std::find_if(accs.begin(), accs.end(), [cpu_acc] (const accelerator& acc)
            {
            return !acc.is_emulated &&
                acc.device_path.compare(cpu_acc.device_path) !=0 ; //pick GPU used for display
            });
        if (result != accs.end()) chosen_one = *(result);
        else chosen_one = accelerator(accelerator::direct3d_warp); //pick CPU (emulated)
        }

    bool success = accelerator::set_default(useCpu == GroundDetectionParameters::ProcessingStrategy::USE_CPU_ONLY ? accelerator::direct3d_warp : chosen_one.device_path);
    return success;
    }

double s_ticksToLoadData = 0;
double s_ticksToTriangulate = 0;
double s_ticksToTriangulate2 = 0;
double s_ticksToFilterGround = 0;
double s_ticksToWriteToChannelBuffer = 0;
double s_ticksToUpdateClassifFile = 0;
double s_ticksForAllTile = 0;
double s_ticksForPopulateList = 0;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void GroundDetection::filterGroundForTile(QuadSeedPtr currentTile, BcDTMPtr dtmObj, int threadId)
    {
    std::vector<int> remainingTriIndices;
    clock_t timer1 = clock();
    PointCloudQuadNode* node = currentTile->tile;

    size_t numOfPts;
    UChar* channelBuffer;
#ifdef SCALABLE_MESH_ATP
    int64_t nSeedPoints = 0;
    IScalableMeshATP::GetInt(ATP_GROUNDDETECTION_ALL_SEED_POINTS_NUMBER, nSeedPoints);
    nSeedPoints += currentTile->seedPoints.size();
    IScalableMeshATP::StoreInt(ATP_GROUNDDETECTION_ALL_SEED_POINTS_NUMBER, nSeedPoints);
#endif
    clock_t timer = clock();
    if (threadId == -1 && node->m_pChannelBuffer == nullptr) node->loadData(true);
    channelBuffer = node->m_pChannelBuffer;
    numOfPts = node->m_nOfPointsRead;
    clock_t endT = clock();
    if (endT > 0 && timer > 0) s_ticksToLoadData += (float)(endT - timer) / CLOCKS_PER_SEC;

    if (numOfPts == 0)
        {
        return; //tile has no points
        }
    DPoint3d minPt(node->m_corner[0]);
    DPoint3d maxPt(node->m_corner[1]);

    std::vector<int> isPointProcessedVector(numOfPts, 0);


    long numTriangles = 0;
   // DPoint3d **dtmPtsPP = 0; /* <== Pointer To Dtm Points Array ( Dpoint3d )       */
    std::vector<DPoint3d> removedPts;
    std::vector<DPoint3d> insertedPoints;
        {
        timer = clock();
        makeTriangulationForExtent(dtmObj->GetTinHandle(), minPt, maxPt, remainingTriIndices, insertedPoints, node);
        numTriangles = dtmObj->GetTrianglesCount();//triangulation(dtmPtsPP, dtmObj->GetTinHandle());
        endT = clock();
        if (endT > 0 && timer > 0) s_ticksToTriangulate += (float)(endT - timer) / CLOCKS_PER_SEC;
        }
    if (insertedPoints.size() == 0)
        {
        return; //tile has no points
        }

    // initial triangulation vertices
    /*int maxTripointNum = (int) ((tinpercentage * (float) numTriangles * 3) / 100.0);

    if (maxTripointNum < (s_minimalTriangulableTilePointNum * s_minimalAcceptableTriangulationVerticeNumRatioVersusTileVerticeNum))
    maxTripointNum = (int) (s_minimalTriangulableTilePointNum * s_minimalAcceptableTriangulationVerticeNumRatioVersusTileVerticeNum);*/

    bool keepClassifying = true;

    int loop = 0;
    double allowedHeight = m_pParams->GetHeightThreshold();
    allowedHeight = TINGrowingParams::acceptableHeightOfGroundPointsInPoints * 
        min(TINGrowingParams::maxTrustworthinessFactor, max(TINGrowingParams::minTrustworthinessFactor, currentTile->trustworthiness)) / currentTile->density;

    int nSlopeNotValid = 0;
    int nTriangleNotFound = 0;
    int nAddedPts = 0;
    int nHeightNotValid = 0;

#ifdef SCALABLE_MESH_ATP
    int64_t nLoops = 0;
    IScalableMeshATP::GetInt(ATP_GROUNDDETECTION_ALL_LOOPS_NUMBER, nLoops);
#endif
    std::vector<FPoint3d> allTriangles;

        {
        timer = clock();
        makeTriangulation(dtmObj->GetTinHandle(), insertedPoints);
        numTriangles = dtmObj->GetTrianglesCount();//triangulation(dtmPtsPP, dtmObj->GetTinHandle());
        s_ticksToTriangulate2 += (float)(clock() - timer) / CLOCKS_PER_SEC;
        }
    timer = clock();
    if (dtmObj->GetTrianglesCount() > 0) populateTriangleList(allTriangles, dtmObj, minPt);
    endT = clock();
    if (endT > 0 && timer > 0) s_ticksForPopulateList += (float)(endT - timer) / CLOCKS_PER_SEC;
    timer = clock();
    DPoint3d* pEvaluatedPoint(node->m_pXyzBuffer);
    UChar*    pFilterBuffer(node->m_pFilterBuffer);
    if (numOfPts > 50000) numOfPts = 50000;
    size_t beginIdx = 0;
    bool useCpuAccelerator = false;
    while (beginIdx < node->m_nOfPointsRead)
        {
        //std::vector<bool>::iterator isPointProcessedItr = isPointProcessedVector.begin();
        std::vector<int> isPointAddedVector((int)numOfPts, -1);
        std::vector<FPoint3d> pointData;
        for (size_t pointIndx = beginIdx; pointIndx < numOfPts + beginIdx; pointIndx++, pEvaluatedPoint++)
            {
            FPoint3d fpt;
            DPoint3d dpt = *pEvaluatedPoint;
            dpt.x -= minPt.x;
            dpt.y -= minPt.y;
            dpt.z -= minPt.z;
            bsiFPoint3d_initFromDPoint3d(&fpt, &dpt);
            pointData.push_back(fpt);
            }
        std::vector<int>::iterator isPointProcessedItr = isPointProcessedVector.begin();
        array_view<const float> a((int)numOfPts * 3, (const float*)&pointData[0]);
        array_view<int> c((int)numOfPts, isPointProcessedVector);
        array_view<int> d((int)numOfPts, isPointAddedVector);
        d.discard_data();
        array_view<const float> triList((int)allTriangles.size() * 3, (const float*)&allTriangles[0]);
        concurrency::extent<1> e((int)numOfPts);
        const float density = currentTile->density;
        const float minZ = minPt.z;
        const float slope = m_pParams->GetSlopeThreshold();
        const float height = allowedHeight;
        numTriangles = (int)allTriangles.size();
        accelerator acc;
        if (allTriangles.size() > 500000 || useCpuAccelerator) acc = accelerator(accelerator::direct3d_warp);
        try
            {
            std::lock_guard<std::mutex> lock(s_mutexamp);
            parallel_for_each(acc.default_view,e, [=] (index<1> idx) restrict(amp)
                {
                index<1> ptIdx(idx[0] * 3);
                index<1> ptIdx1(idx[0] * 3 + 1);
                index<1> ptIdx2(idx[0] * 3 + 2);
                float testPtX = a[ptIdx];
                float testPtY = a[ptIdx1];
                float testPtZ = a[ptIdx2];
                FPoint3d tri[3];
                FPoint3d testPt;
                testPt.x = testPtX;
                testPt.y = testPtY;
                testPt.z = testPtZ;
                if (c[idx] || !findTriangleInTriList(tri, triList, numTriangles * 3, testPt)) d[idx] = -1;
                else
                    {
                    c[idx] = 1;
                    FPoint3d proj = projectPtOnTriPlane(tri, testPt);
                    if (!evaluateSlopeCondition(tri, proj, testPt, slope)) d[idx] = -2;
                    else if (!evaluateHeightCondition(testPt, proj, height, density, minZ))  d[idx] = -3;
                    else d[idx] = pointDistanceSquared(testPt, proj);
                    }
                });
            c.synchronize();
            d.synchronize();
            }
        catch (accelerator_view_removed)
            {
            //we got a TDR
            useCpuAccelerator = true;
            numOfPts = 0;
            }
        catch (runtime_exception& ex)
            {
            std::string s;
            s += ex.what();
            }
        endT = clock();
        if (endT > 0 && timer > 0) s_ticksToFilterGround += (float)(endT - timer) / CLOCKS_PER_SEC;
        timer = clock();
        for (size_t pointIndx = beginIdx; pointIndx < numOfPts+beginIdx; pointIndx++, pFilterBuffer++)
            {
            if (isPointAddedVector[(int)(pointIndx - beginIdx)] >= 0 && PointCloudChannels_Is_Point_Visible(*pFilterBuffer))
                {
                //if (isPointAddedVector[(int)pointIndx] >= (allowedHeight / 10) * (allowedHeight / 10)) insertedPoints.push_back(node->m_pXyzBuffer[pointIndx]);
                    {
                    channelBuffer[pointIndx] = GROUND_CHANNEL_NUMBER;
                    }
                node->m_ptsGround++;
                nAddedPts++;
                }
            else if (isPointAddedVector[(int)(pointIndx - beginIdx)] == -2) nSlopeNotValid++;
            else if (isPointAddedVector[(int)(pointIndx - beginIdx)] == -3) nHeightNotValid++;
            else if (isPointAddedVector[(int)(pointIndx - beginIdx)] == -1) nTriangleNotFound++;
            for (auto it = insertedPoints.begin(); it != insertedPoints.end(); it++)
                {
                if (it->x == node->m_pXyzBuffer[pointIndx].x && it->y == node->m_pXyzBuffer[pointIndx].y && it->z == node->m_pXyzBuffer[pointIndx].z)
                    {
                    channelBuffer[pointIndx] = GROUND_CHANNEL_NUMBER;
                    insertedPoints.erase(it);
                    break;
                    }
                }
            }
        beginIdx += numOfPts;
        if (beginIdx + 50000 < node->m_nOfPointsRead) numOfPts = 50000;
        else numOfPts = node->m_nOfPointsRead - beginIdx;
        }
    endT = clock();
    if (endT > 0 && timer > 0) s_ticksToWriteToChannelBuffer += (float)(endT - timer) / CLOCKS_PER_SEC;
#if 0
    for (size_t pointIndx = 0; pointIndx < numOfPts; pointIndx++, pEvaluatedPoint++, pFilterBuffer++, isPointProcessedItr++)
        {
        // If point not visible, we can just mask it as processed, we don't need to test it next time            
        if (*isPointProcessedItr || !PointCloudChannels_Is_Point_Visible(*pFilterBuffer))
            {
            *isPointProcessedItr = true;
            continue;
            }

        DPoint3d evaluatedPoint = *pEvaluatedPoint;

        std::vector<DPoint3d> tri(3);
        DPoint3d projectedPoint;

        /* if (!getClosestTriangleToPoint(dtmObject, evaluatedPoint, projectedPoint, tri, groundNormal, allowedHeight, allowedSlope))
             {
             continue;
             }*/
        if (!getClosestTriangleToPointPCL(evaluatedPoint, projectedPoint, tri, pclTree, &insertedPoints[0]))
            {
            continue;
            }

        double angle = params.slopeThreshold;

        if (!isPointWithinSlopeOfTin(angle, evaluatedPoint, projectedPoint, tri))
            {
            continue;
            }
        double height = pointDistance(evaluatedPoint, projectedPoint);

        if (height < allowedHeight)
            {

            // Add point in dtm.                                                            
            insertedPoints.push_back(evaluatedPoint);

                {
                // std::lock_guard<std::mutex> lock(s_mutexData);
                channelBuffer[pointIndx] = GROUND_CHANNEL_NUMBER;
                }
            node->m_ptsGround++;
            *isPointProcessedItr = true;

            keepClassifying = true;

            continue;
            }
        }
    PCLUtility::INormalCalculator::ReleaseOctree(pclTree);
    }
#endif

/*    int nSlopeNotValid = 0;
int nTriangleNotFound = 0;
int nAddedPts = 0;
int nHeightNotValid = 0;*/
#ifdef SCALABLE_MESH_ATP
nLoops+=loop;
IScalableMeshATP::StoreInt(ATP_GROUNDDETECTION_ALL_LOOPS_NUMBER, nLoops);
#endif
if (minPt.x <= 6850350000 && maxPt.x >= 6850350000 && minPt.y <= 720130000 && maxPt.y >= 720130000)
    {
    std::string s;
    s += "TRIANGLES NOT FOUND " + std::to_string(nTriangleNotFound) + " SLOPE NOT VALID " + std::to_string(nSlopeNotValid) + "(SLOPE IS " + std::to_string(m_pParams->GetSlopeThreshold()) + ") HEIGHT NOT VALID " +
        std::to_string(nHeightNotValid) + "(HEIGHT IS " + std::to_string(allowedHeight) + ") POINTS ADDED " + std::to_string(nAddedPts) + " OUT OF " + std::to_string(numOfPts);
    }

if (threadId == -1)
    {
    timer = clock();
    node->saveClassification();
    node->unloadData();
    endT = clock();
    if (endT > 0 && timer > 0) s_ticksToUpdateClassifFile += (float)(endT - timer) / CLOCKS_PER_SEC;
    }
endT = clock();
if (endT > 0 && timer1 > 0) s_ticksForAllTile += (float)(endT - timer1) / CLOCKS_PER_SEC;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
// For multithreading
void doTileFiltering(DgnPlatformLib::Host* hostToAdopt, GroundDetection* ground, std::vector<QuadSeedPtr>& tiles, TileLoaderQueue* queue, std::atomic<bool>* threadStop, std::vector<BcDTMPtr>& dtmArray, int threadId, int NUM_THREADS)
    {
    //According to Keith Bentley the AdoptHost function could be use the set the host created in the main thread
    //to the other working thread (see UstationLibHost.h for more information). 
    //DgnPlatformLib::AdoptHost(*hostToAdopt);
    size_t ntiles = 0;
    try
        {
        for (size_t i = 0; i < tiles.size(); i++)
            {
            if (i % NUM_THREADS == threadId)
                {
                ntiles++;
                if (!tiles[i]->tile->isReady){
                    TileLoadMessage msgLoad(TileLoadMessage::MessageTag::MESSAGE_LOAD_TILE, tiles[i]->tile);
                    queue->Send(msgLoad);
                    }
                int timeout = 0;
                while (!tiles[i]->tile->isReady)
                    {
                    this_thread::sleep_for(std::chrono::microseconds(250)); 
                    if (++timeout > NUM_THREADS)
                        {
                        TileLoadMessage msgLoad(TileLoadMessage::MessageTag::MESSAGE_LOAD_TILE, tiles[i]->tile);
                        queue->Send(msgLoad);
                        timeout = 0;
                        }
                    }
                ground->filterGroundForTile(tiles[i], dtmArray[threadId],threadId);
                TileLoadMessage msgSave(TileLoadMessage::MessageTag::MESSAGE_SAVE_TILE, tiles[i]->tile);
                queue->Send(msgSave);
                //Progress update and cancellation check
                if (!ProgressBarMonitor::UpdateProgressInThread())
                    i = tiles.size();//process was cancelled; force to exit the loop
                }
            }
        }
    catch (...)
        {
        ProgressBarMonitor::SignalErrorInThread();
        }
    threadStop[threadId] = true;
    //DgnPlatformLib::ForgetHost();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GroundDetection::filterGround(std::vector<QuadSeedPtr>& seeds, EditElementHandleR elHandle, ProgressReportR report)
    {
#ifndef DISABLELOGGER //Don't want logger during ATP
    StopWatch timer;
    timer.Start();

    // Write some stats in files like time elapsed
    GROUNDDLOG->trace(L"START - filterGround");
#endif
    report.SetCurrentStep(ProgressReport::STATE_FILTER_GROUND);
    report.SetCurrentPhase(3);
    report.SetWorkDone(0.0);
    if (!report.CheckContinueOnProgress())
        return ERROR;//User abort

    QuadSeedPtr cloudSeed(quadSeed::Create());
    for (size_t j = 0; j < seeds.size(); j++)
        {
        cloudSeed->seedPoints.insert(cloudSeed->seedPoints.end(), seeds[j]->seedPoints.begin(), seeds[j]->seedPoints.end());
        }
    m_cloudSeed = cloudSeed;

#ifndef DISABLELOGGER //Don't want logger during ATP
    GROUNDDLOG->tracev(L"Initial Nb of seed points=%ld", m_cloudSeed->seedPoints.size());
#endif

    // Seed Triangulation delaunay
    RefCountedPtr<BcDTM> bcDtmObjPtr = BcDTM::Create();
    std::vector<DPoint3d> pPt;
    transformDataForSeedsTriangulation(pPt);
    if (setTriangulation(pPt, bcDtmObjPtr->GetTinHandle()) == ERROR)
        {
        return ERROR;
        }
    //DPoint3d **dtmPtsPP = 0; /* <== Pointer To Dtm Points Array ( Dpoint3d )       */
    //triangulation(dtmPtsPP, bcDtmObjPtr->GetTinHandle());
    // get all triangles
#ifdef SCALABLE_MESH_ATP
    double nTimeToEstimateParams = 0.0;
    double timerT = clock();
#endif
    //estimate parameters
    if (s_useEstimatedParameters && (GroundDetectionParameters::IsAutoDetect(m_pParams->GetSlopeThreshold()) || 
                                     GroundDetectionParameters::IsAutoDetect(m_pParams->GetHeightThreshold())) )
        {
        double slope = 0;
        double height = 0;
        double heightgrad = 0;
        computeStatisticsForTIN(slope, height, heightgrad, bcDtmObjPtr, 95); //essentially this assumes 5% of seeds are outliers. 
        if (GroundDetectionParameters::IsAutoDetect(m_pParams->GetSlopeThreshold())) 
            m_pParams->SetSlopeThreshold(slope);
        if (GroundDetectionParameters::IsAutoDetect(m_pParams->GetHeightThreshold())) 
            m_pParams->SetHeightThreshold(height);
        }

#ifndef DISABLELOGGER //Don't want logger during ATP
    GROUNDDLOG->tracev(L"Angle=%lf Height=%lf Largest Struct=%lf", m_pParams->GetSlopeThreshold(), m_pParams->GetHeightThreshold(), m_pParams->GetLargestStructureSize());
#endif

#ifdef SCALABLE_MESH_ATP
    nTimeToEstimateParams = (clock() - timerT)/CLOCKS_PER_SEC;
    IScalableMeshATP::StoreDouble(ATP_GROUNDDETECTION_TIMINGS_PARAM_ESTIMATION, nTimeToEstimateParams);
#endif
    //create index and vertex buffer
   // BcDTMMeshPtr mesh = bcDtmObjPtr->GetMesh(TRUE, bcDtmObjPtr->GetTrianglesCount(), NULL, 0);
    DTMMeshEnumeratorPtr enumerator = DTMMeshEnumerator::Create (*bcDtmObjPtr);
    enumerator->SetMaxTriangles (100000000); // Otherwise it could return more than one section.

    auto pf = *(enumerator->begin());
    //std::vector<BcDTMMeshFacePtr> VectorFaces;
    //const int numFaces =pf->GetPointIndexCount()/3;
    //VectorFaces.resize(numFaces);

    const size_t numPoints = pf->GetPointCount();
    std::vector<DPoint3d> points;
    std::vector<int> indices;
    for (size_t i = 0; i < numPoints; i++)
        points.push_back(pf->GetPointCP()[(int) i]);

    for (int i = 0; i < pf->GetPointIndexCount(); i+=3)
        {
        //VectorFaces[i] = mesh->GetFace(i);

        indices.push_back(pf->GetPointIndexCP()[i] - 1);
        indices.push_back(pf->GetPointIndexCP()[i+1]  - 1);
        indices.push_back(pf->GetPointIndexCP()[i+2]  - 1);
        }

    //VectorFaces.clear();

    // Now first triangulation is ready (seeds are triangulated)
    // Spatial partition of triangles
    makeQuadTreeFromTriangulation(points, indices);
    std::vector<QuadSeedPtr> unmarkedExtent;

    m_pQuadTree->visitUnmarkedNodes(unmarkedExtent);
#ifndef DISABLELOGGER //Don't want logger during ATP
    GROUNDDLOG->tracev(L"Number of Tiles=%ld", seeds.size());
#endif
    bool useMultiThread = m_pParams->GetUseMultiThread();
#ifdef SCALABLE_MESH_ATP
    int64_t useMultiThreadValue = 0;
    if(IScalableMeshATP::GetInt(ATP_GROUNDDETECTION_SHOULD_USE_MULTITHREAD, useMultiThreadValue) == SUCCESS && useMultiThreadValue > 0) useMultiThread = true;
    double nTimeToFilterGround = 0.0;
    timerT = clock();
#endif
#ifdef SCALABLE_MESH_ATP
    int64_t nSeeds = seeds.size();
    IScalableMeshATP::StoreInt(ATP_GROUNDDETECTION_SEEDS_NUMBER, nSeeds);
#endif
    pick_accelerator(m_pParams->GetProcessingStrategy());
#ifdef SCALABLE_MESH_ATP
    accelerator default_acc;
    int64_t acceleratorUseCpu = default_acc.device_path.compare(accelerator::direct3d_warp) == 0 ? ACCELERATOR_CPU : ACCELERATOR_GPU;
    IScalableMeshATP::StoreInt(ATP_GROUNDDETECTION_CHOSEN_ACCELERATOR_TYPE, acceleratorUseCpu);
#endif
    if (useMultiThread/*GetPublishedGlobals()->groundDetection.enableMultiThread*/)
        {
        //how many  threads can be created, zero means is meaningless for this platform
        unsigned int num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0 || num_threads > s_MaxNumberOfThreads)
            num_threads = s_MaxNumberOfThreads;  //Arbitrary number

        vector<std::thread> tgroup(num_threads);
        ProgressBarMonitor tileFilteringProgressBar(report, seeds.size(), true);

        std::atomic<bool>* threadFutures = new std::atomic<bool>[num_threads];
        std::vector<BcDTMPtr> dtmThreads(num_threads);
        for (size_t i = 0; i < num_threads; i++)
            {
            threadFutures[i] = false;
            dtmThreads[i] = BcDTM::Create();
            }
        TileLoaderQueue q;
        q.SetStop([&threadFutures, num_threads]() {
            for (size_t i = 0; i < num_threads; i++)
                {
                if (!threadFutures[i]) 
                    {
                    return false;
                    }
                }
            return true;
            });
        //we pre-load a random subset of seeds to minimize contention in the early steps of the algorithm.
        //The main thread does all point cloud queries.
        size_t nInitialLoad = min((size_t)100, seeds.size());
        std::vector<QuadSeedPtr> initSeeds;
        initSeeds.insert(initSeeds.begin(),seeds.begin(), seeds.begin() + min((size_t)500, seeds.size()));
        std::random_shuffle(initSeeds.begin(), initSeeds.end());
        for (size_t n = 0; n < nInitialLoad; ++n)
            {
            QuadSeedPtr& seed = initSeeds[n];
            seed->tile->loadData(true);
            seed->tile->isReady = true;
            }
        //Launch a group of threads
        for (unsigned int i = 0; i < num_threads; ++i)
            {
            tgroup[i] = std::thread(doTileFiltering, DgnPlatformLib::QueryHost(), this, seeds, &q, threadFutures, dtmThreads, i, num_threads);
            }
        q.Run(); //this will run until there is no more work for the main thread (loading tiles, saving tiles, etc)

        //Join the threads with the main thread
        for (unsigned int i = 0; i < num_threads; ++i)
            {
            tgroup[i].join();
            }
        tileFilteringProgressBar.FinalStageProcessing();
        for (auto& seed : seeds) //make sure all processed tiles are saved correctly
            {
            seed->tile->saveClassification();
            seed->tile->unloadData();
            }
        delete[] threadFutures;
        }
    else
        {
        BcDTMPtr dtm = BcDTM::Create();
        ProgressBarMonitor tileFilteringProgressBar(report, seeds.size(), false);
        for (size_t i = 0; i < seeds.size(); ++i)
            {
            //Display progress
            if (!ProgressBarMonitor::UpdateProgress())
                break;//process was cancelled; force to exit the loop
            tileFilteringProgressBar.InProgress();
            
            filterGroundForTile(seeds[i], dtm, -1);
//             if (i % 10 == 0)
//                 {
//                 report.SetWorkDone(i / seeds.size());
//                 if (!report.CheckContinueOnProgress())
//                     return ERROR;//User abort
//                 }
            }
        tileFilteringProgressBar.FinalStageProcessing();
        }
#ifdef SCALABLE_MESH_ATP
    nTimeToFilterGround = (float)(clock() - timerT)/CLOCKS_PER_SEC;
    IScalableMeshATP::StoreDouble(ATP_GROUNDDETECTION_TIMINGS_FILTER_GROUND, nTimeToFilterGround);
#endif

#ifndef DISABLELOGGER //Don't want logger during ATP
    timer.Stop();
    GroundDetectionLogger::OutputTimerToLogger(timer);
    GROUNDDLOG->trace(L"END - filterGround");
#endif

    return SUCCESS;
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE
