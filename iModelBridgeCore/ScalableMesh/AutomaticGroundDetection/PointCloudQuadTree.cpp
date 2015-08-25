/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/PointCloudQuadTree.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ScalableMeshPCH.h"
#include "AutomaticGroundDetectionInternalConfig.h"
//#include "PointCloudUtilities.h"
#include "PointCloudQuadTree.h"
//#include "PointCloudEditChannelUndoRedoManager.h"
//#include "PointCloudEditRasterColoringTool.h"

#include "PointCloudClassification.h"

typedef RgbColorDef* RgbColorDefP;

#include <RmgrTools\Tools\rscdefs.r.h>
#include <DgnView\IViewManager.h>
#include <PCLWrapper\IDefines.h>
#include <PCLWrapper\IRansacUtility.h>
#define GROUND_CHANNEL_NUMBER 2

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_LOGGING


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

const int       PointCloudQuadTree::BITVECTOR[] = { 0x1, 0x2, 0x4, 0x8 };
const double    PointCloudQuadTree::D_EPSILON_2 = 1e-6;

#define MAX_NB_VIEWS 8

/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard        11/2014
+===============+===============+===============+===============+===============+======*/
class  ProgressBarMonitorForTree
    {
    public:
        ProgressBarMonitorForTree(ProgressReport& report, double processSize) :m_progressReport(report), m_processSize(processSize)
            {
            s_process_canceled = false;
            s_currentSize = 0;
            s_pTimer = new StopWatch();
            s_pTimer->Start();
            }

        ~ProgressBarMonitorForTree()
            {
            if (s_pTimer)
                delete s_pTimer;
            }

        //return true if process should continue (was not canceled)
        static bool UpdateProgress(size_t currentProgress)
            {
            //Always increment
            s_currentSize = currentProgress;
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
        bool InProgress()
            {
            ShowProgress();

            return CheckCancel();
            }

        bool WasCanceled()  {return s_process_canceled;}

    private:
        static size_t       s_currentSize;
        static StopWatch*   s_pTimer;
        static double       s_currentNbSecondsAtLastUpdate;
        static bool         s_process_canceled;
        static bool         s_process_errorSignaled;


        double              m_processSize;
        ProgressReport&     m_progressReport;


        //return true if process should continue (was not canceled)
        bool ShowProgress()
            {
            if (s_process_errorSignaled)
                {
                m_progressReport.OnSignalError();
                return false;
                }

            double currentNbSeconds(s_currentNbSecondsAtLastUpdate);

            double accomplishmentRatio = (static_cast<double>(s_currentSize) / m_processSize);
            double remainingSeconds(0);
            if (accomplishmentRatio!=0.0)
                remainingSeconds = ((1.0 / accomplishmentRatio) * currentNbSeconds) - currentNbSeconds;

            m_progressReport.SetEstimatedRemainingTime(true, remainingSeconds);
            m_progressReport.SetWorkDone(accomplishmentRatio);
            s_process_canceled = !m_progressReport.CheckContinueOnProgress();

            return (!s_process_canceled);
            }

        //return true if process should continue (was not canceled)
        bool CheckCancel()
            {
            //Progress and cancellation check
            if (!s_process_canceled)
                s_process_canceled = !m_progressReport.CheckContinueOnProgress();
            return (!s_process_canceled);
            }
    }; // ProgressBarMonitorForTree

size_t ProgressBarMonitorForTree::s_currentSize = 0;

StopWatch*   ProgressBarMonitorForTree::s_pTimer;
double       ProgressBarMonitorForTree::s_currentNbSecondsAtLastUpdate=0;
bool         ProgressBarMonitorForTree::s_process_canceled=false;
bool         ProgressBarMonitorForTree::s_process_errorSignaled = (false);


quadSeed::quadSeed(std::vector<DPoint3d> seedPoints_, PointCloudQuadNode* tile_)
    {
    seedPoints = seedPoints_;
    tile = tile_;
    }

quadSeed::quadSeed()
    {
    tile = NULL;
    seedPoints = vector<DPoint3d>();
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudQuadTreePtr PointCloudQuadTree::Create(PointCloudQuadTreeData* data, std::vector<QuadSeedPtr>& seeds, ProgressReportR report, float density)
    {
    return new PointCloudQuadTree(data, seeds, report, density);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudQuadTree::PointCloudQuadTree(PointCloudQuadTreeData* data, std::vector<QuadSeedPtr>& seeds, ProgressReportR report, float density) : m_triangleIndexes(NULL), m_dataSize(0), m_nbNodeFetched(0)
    {
    m_densityVal = density;
    createTreeAndSeeds(data, seeds, report);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudQuadTreePtr PointCloudQuadTree::Create(PointCloudQuadTreeData* data, ProgressReportR report, float density)
    {
    return new PointCloudQuadTree(data, report, density);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudQuadTree::PointCloudQuadTree(PointCloudQuadTreeData* data, ProgressReportR report, float density) : m_triangleIndexes(NULL), m_dataSize(0), m_nbNodeFetched(0)
    {
    m_densityVal = density;
    createTree(data, report);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudQuadTree::~PointCloudQuadTree()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudQuadTree::createTreeAndSeeds(PointCloudQuadTreeData* data, std::vector<QuadSeedPtr>& seeds, ProgressReportR report)
    {
#ifndef DISABLELOGGER //Don't want logger during ATP
    GROUNDDLOG->trace(L"START - PointCloudQuadTree");
    StopWatch t;
    t.Start();
#endif

    m_maxDepth = data->m_maxDepth;
    m_maxSize = data->m_sizeBound;
    m_elHandle = data->m_elHandle;
    m_maxDataSize = data->m_maxDataSize;
    if(data->m_points!=NULL)
        m_points = *data->m_points;
    m_root = PointCloudQuadNode::Create(data->m_bb, this, NULL, 0, true);
    int currentDepth = 0;
    
    m_root->loadData();
    size_t pointsRead = m_root->m_nOfPointsRead;

    if (pointsRead > this->getMaxSize())
        {
        m_root->unloadData();
        size_t idx = 0;
        m_maxDepth = m_root->createSubRegion(currentDepth++, 0, seeds, idx);
#pragma omp parallel for schedule(dynamic)
        for (int i = (int)idx; i < (int)seeds.size(); ++i)
            {
            seeds[i]->tile->fetchSeedsFromQuadTree(seeds[i]);
            }
        for (int i = (int)idx; i < (int)seeds.size(); ++i)
            {
            seeds[i]->tile->unloadData();
            }
        }
    else
        {
        m_root->m_dataSize = pointsRead;
        QuadSeedPtr seed(quadSeed::Create());
        seed->tile = m_root.get();
        m_root->fetchSeedsFromQuadTree(seed);
        m_root->unloadData();
        seeds.push_back(seed);
        }

    m_dataSize = m_dataSize + m_root->GetDataSize();
#ifndef DISABLELOGGER //Don't want logger during ATP
    t.Stop();
    GroundDetectionLogger::OutputTimerToLogger(t);
    GROUNDDLOG->trace(L"END - PointCloudQuadTree");
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudQuadTree::createTree(PointCloudQuadTreeData* data, ProgressReportR report)
    {
#ifndef DISABLELOGGER //Don't want logger during ATP
    GROUNDDLOG->trace(L"START - PointCloudQuadTree");
    StopWatch t;
    t.Start();
#endif
    
    m_maxDepth = data->m_maxDepth;
    m_maxSize = data->m_sizeBound;
    m_elHandle = data->m_elHandle;
    m_maxDataSize = data->m_maxDataSize;
    if(data->m_points!=NULL)
        m_points = *data->m_points;
    m_root = PointCloudQuadNode::Create(data->m_bb, this, NULL, 0, true);
    int currentDepth = 0;
    
    m_root->loadData();
    size_t pointsRead = m_root->m_nOfPointsRead;

    if (pointsRead > this->getMaxSize())
        {
        m_root->unloadData();
        m_maxDepth = m_root->createSubRegion(currentDepth++, 0);
        }
    else
        {
        m_root->m_dataSize = pointsRead;
        }

    m_dataSize = m_dataSize + m_root->GetDataSize();
#ifndef DISABLELOGGER //Don't want logger during ATP
    t.Stop();
    GroundDetectionLogger::OutputTimerToLogger(t);
    GROUNDDLOG->trace(L"END - PointCloudQuadTree");
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elenie.Godzaridis                 5/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudQuadTree::excludeOutlierSeeds(std::vector<QuadSeedPtr>& seeds)
    {
    double avgDensity = 0.0;
    std::for_each(seeds.begin(), seeds.end(), [&avgDensity] (QuadSeedPtr& seed) { avgDensity+=seed->density; });
    avgDensity /= seeds.size();
    std::vector<std::vector<DPoint3d>> toErase(seeds.size());
    std::vector<float> percentageOfInliers(seeds.size());
#pragma omp parallel for schedule(dynamic)
    for (int it = 0; it < seeds.size(); it++)
        {
        std::vector<DPoint3d> pointsAroundSeedNode;
        QuadSeedPtr& seed = seeds[it];
        if (seed->density < avgDensity/3)
            {
            seed->seedPoints.clear();
            }
        else
            {
            pointsAroundSeedNode.insert(pointsAroundSeedNode.end(), seed->seedPoints.begin(), seed->seedPoints.end());
            size_t i = 0;
            
            PointCloudQuadNode* parent = seed->tile->m_parent;
            bool stopSeeds = parent->m_corner[0].z < seed->tile->m_corner[0].z - (seed->tile->m_corner[1].z - seed->tile->m_corner[0].z)*0.2;
            for (i = 0; !stopSeeds && parent->m_parent != NULL; ++i)
                {
                parent = parent->m_parent;
                stopSeeds = parent->m_corner[0].z < seed->tile->m_corner[0].z - (seed->tile->m_corner[1].z - seed->tile->m_corner[0].z)*0.2;
                }
            for (auto& seed2 : seeds)
                {
                PointCloudQuadNode* seedAncestor = seed2->tile->m_parent;
                while (seedAncestor != parent && seedAncestor->m_parent != NULL)  seedAncestor = seedAncestor->m_parent;
                if (parent == seedAncestor) pointsAroundSeedNode.insert(pointsAroundSeedNode.end(), seed2->seedPoints.begin(), seed2->seedPoints.end());
                if (!stopSeeds && pointsAroundSeedNode.size() > 500) break;
                }
            DPoint3d* inliers = nullptr;
            DPoint3d* outliers = nullptr;
            size_t nOutliers = 0;
            size_t nInliers = 0;
            Bentley::PCLUtility::IRansacUtility::GetOutliersFromBestPlaneFit(outliers, inliers, nOutliers, nInliers, &pointsAroundSeedNode[0], pointsAroundSeedNode.size(), 2 / avgDensity);
            percentageOfInliers[it] = (float)nInliers / pointsAroundSeedNode.size();
            for (size_t i = 0; i < nOutliers && (float)nInliers / pointsAroundSeedNode.size() > SeedExclusionParams::thresholdOfRansacInliersForExclusion +
                                                        SeedExclusionParams::trustworthinessCorrectionFactorForInlierThreshold*seed->trustworthiness; i++)
                {
                toErase[it].push_back(outliers[i]);
                }
            delete[] outliers;
            delete[] inliers;
            }
        }
    for (auto& listPoints : toErase)
        {
        auto& seed = seeds[&listPoints - &toErase[0]];
        if (SeedExclusionParams::dontExcludeSeedsIfHighlyTrustworthy && seed->trustworthiness > SeedExclusionParams::trustworthinessThresholdForExclusion) continue;
        for (DPoint3d i : listPoints)
            {
            auto it2 = std::find_if(seed->seedPoints.begin(), seed->seedPoints.end(), [&i] (DPoint3d& pt) { return i.x == pt.x && i.y == pt.y && i.z == pt.z; });
            if (it2 != seed->seedPoints.end()) seed->seedPoints.erase(it2);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudQuadTree::createSeeds(std::vector<QuadSeedPtr>& seeds, double distanceThreshold, double minDistance, ProgressReportR report)
    {
#ifndef DISABLELOGGER //Don't want logger during ATP
    GROUNDDLOG->trace(L"START - createSeeds");
    StopWatch createSeedsTimer;
    createSeedsTimer.Start();
#endif

    report.SetCurrentStep(ProgressReport::STATE_FILTER_SEEDS);
    report.SetCurrentPhase(2);
    report.SetWorkDone(0.0);
    if (!report.CheckContinueOnProgress())
        return false;//User abort
    excludeOutlierSeeds(seeds);
    report.SetWorkDone(1.0);
    if (!report.CheckContinueOnProgress())
        return false;//User abort

#ifndef DISABLELOGGER //Don't want logger during ATP
    createSeedsTimer.Stop();
    GroundDetectionLogger::OutputTimerToLogger(createSeedsTimer);
    GROUNDDLOG->trace(L"END - createSeeds");
#endif

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
#if 0 //NEEDS_WORK_SM_IMPORTER Not required?
void PointCloudQuadTree::drawTree(EditElementHandleR elHandle)
    {
    m_root->drawNode(elHandle);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudQuadTree::visitUnmarkedNodes(std::vector<QuadSeedPtr>& seeds)
    {
    // visit All node which are not already visited
    m_root->visitUnmarkedNodes(seeds);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
//spatial partition for triangles 
void PointCloudQuadTree::setTriangles(std::vector<DPoint3d>& points, std::vector<int>& indices)
    {
    this->m_points = points;
    this->m_triangleIndexes = &indices;
    size_t size = m_triangleIndexes->size();
    for(size_t i = 0; i < size; i+=3)
        {
        m_root->m_tries.push_back(i);
        }
    if(m_root->GetDataSize() > getMaxSize())
        m_root->createTrianglesRegions();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
// Find all triangles in cell of the searchPoint
std::vector<size_t>& PointCloudQuadTree::findTries(DPoint3d& searchPoint)
    {
    return m_root->findTries(searchPoint);
    }

PointCloudQuadNode* PointCloudQuadTree::findNode(DPoint3dCR searchPoint)
    {
    return m_root->findNode(searchPoint);
    }

void PointCloudQuadTree::setSeedsPoints(std::vector<DPoint3d>& seedsPoints, std::vector<QuadSeedPtr>& seeds)
    {
    typedef std::map<size_t, PointCloudQuadNode*> MapNode;
    typedef std::map<size_t, std::vector<DPoint3d>> MapPts;
    MapNode mapIdNode;
    MapPts mapIdVecPt3D;
    for (std::vector<DPoint3d>::const_iterator itr = seedsPoints.begin(); itr!=seedsPoints.end(); ++itr)
        {
        PointCloudQuadNode* node = m_root->findNode(*itr);
        mapIdNode[node->uid] = node;
        mapIdVecPt3D[node->uid].push_back(*itr);
        }
    MapPts::iterator itPts = mapIdVecPt3D.begin();
    for(MapNode::iterator itNode = mapIdNode.begin(); itNode != mapIdNode.end(); itNode++, itPts++)
        {
        QuadSeedPtr seed(quadSeed::Create());
        seed->tile = itNode->second;
        seed->trustworthiness = 1.0;
        seed->seedPoints = itPts->second;
        seeds.push_back(seed);
        }
    }

int PointCloudQuadNode::newID = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudQuadNodePtr PointCloudQuadNode::Create(DPoint3d* bb, PointCloudQuadTree* tree, PointCloudQuadNode* parent, int depth, bool init)
    {
    return new  PointCloudQuadNode(bb,tree,parent,depth,init);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudQuadNode::PointCloudQuadNode(DPoint3d* bb, PointCloudQuadTree* tree, PointCloudQuadNode* parent, int depth, bool init)
    : uid(newID++)
    {
    setMarked(false);
    m_pivot.x = (bb[1].x + bb[0].x) / 2;
    m_pivot.y = (bb[1].y + bb[0].y) / 2;
    m_pivot.z = (bb[1].z + bb[0].z) / 2;  //We ignore z and work only on X-Y plan
    m_corner[0] = bb[0];
    m_corner[1] = bb[1];
    m_tree = tree;
    m_parent = parent;
    m_bitRegions = 0;
    m_depth = depth;
    m_dataSize = 0;
    m_ptsError = 0;
    m_ptsGood = 0;
    m_ptsGround = 0;
    m_pXyzBuffer = nullptr;
    m_pFilterBuffer = nullptr;
    m_pChannelBuffer = nullptr;
    isReady = false;
    for (int i = 0; i < 4; i++)
        m_region[i] = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudQuadNode::~PointCloudQuadNode()
    {
    for(int i=0; i<4; i++)
        m_region[i]=NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
int PointCloudQuadNode::createSubRegion(int currentDepth, size_t dataSize, std::vector<QuadSeedPtr>& seeds, size_t& lastLoadedSeedIdx/*, std::vector<DPoint3d>* points*/)
    {
    int depth[4] = {0,0,0,0};
    // create 4 new boudnigBox, one per subRegion
    DPoint3d boundingBox[4][2];

    // 1. region south west
    boundingBox[SW][0].x = m_corner[0].x;
    boundingBox[SW][0].y = m_corner[0].y;
    boundingBox[SW][1].x = m_pivot.x;
    boundingBox[SW][1].y = m_pivot.y;

    // 2. region south east
    boundingBox[SE][0].x = m_pivot.x;
    boundingBox[SE][0].y = m_corner[0].y;
    boundingBox[SE][1].x = m_corner[1].x;
    boundingBox[SE][1].y = m_pivot.y;

    // 3. region north west
    boundingBox[NW][0].x = m_corner[0].x;
    boundingBox[NW][0].y = m_pivot.y;
    boundingBox[NW][1].x = m_pivot.x;
    boundingBox[NW][1].y = m_corner[1].y;

    // 4. region north west
    boundingBox[NE][0].x = m_pivot.x;
    boundingBox[NE][0].y = m_pivot.y;
    boundingBox[NE][1].x = m_corner[1].x;
    boundingBox[NE][1].y = m_corner[1].y;

    for(int i=0; i<4; i++)
        {
        boundingBox[i][0].z = m_corner[0].z;
        boundingBox[i][1].z = m_corner[1].z;

        m_region[i] = PointCloudQuadNode::Create(boundingBox[i], m_tree, this, m_depth+1);
        m_region[i]->loadData();
        size_t pointsRead = m_region[i]->m_nOfPointsRead;
        if (pointsRead >= m_tree->getMaxSize())
            {
            m_region[i]->unloadData();
            int tmpDepth = m_region[i]->createSubRegion(currentDepth + 1, pointsRead, seeds, lastLoadedSeedIdx);
            depth[i] += max(m_depth, tmpDepth);
            }
        else
            {
            m_region[i]->SetDataSize(pointsRead);
            depth[i] = m_region[i]->GetDepth();
            QuadSeedPtr seed(quadSeed::Create());
            seed->tile = m_region[i].get();
            seeds.push_back(seed);
            if (seeds.size() - lastLoadedSeedIdx >= MAX_N_LOADED_TILES)
                {
#pragma omp parallel for schedule(dynamic)
                for (int j = (int)lastLoadedSeedIdx;j < (int)(lastLoadedSeedIdx + MAX_N_LOADED_TILES); ++j)
                    {
                    seeds[j]->tile->fetchSeedsFromQuadTree(seeds[j]);
                    }
                for (int j = (int)lastLoadedSeedIdx; j < (int)(lastLoadedSeedIdx + MAX_N_LOADED_TILES); ++j)
                    {
                    seeds[j]->tile->unloadData();
                    }
                lastLoadedSeedIdx = lastLoadedSeedIdx + MAX_N_LOADED_TILES;
                }
            }
        m_dataSize += m_region[i]->GetDataSize();
        }
    return *std::max_element(depth, depth+4);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
int PointCloudQuadNode::createSubRegion(int currentDepth, size_t dataSize)
    {
    int depth[4] = {0,0,0,0};
    // create 4 new boudnigBox, one per subRegion
    DPoint3d boundingBox[4][2];

    // 1. region south west
    boundingBox[SW][0].x = m_corner[0].x;
    boundingBox[SW][0].y = m_corner[0].y;
    boundingBox[SW][1].x = m_pivot.x;
    boundingBox[SW][1].y = m_pivot.y;

    // 2. region south east
    boundingBox[SE][0].x = m_pivot.x;
    boundingBox[SE][0].y = m_corner[0].y;
    boundingBox[SE][1].x = m_corner[1].x;
    boundingBox[SE][1].y = m_pivot.y;

    // 3. region north west
    boundingBox[NW][0].x = m_corner[0].x;
    boundingBox[NW][0].y = m_pivot.y;
    boundingBox[NW][1].x = m_pivot.x;
    boundingBox[NW][1].y = m_corner[1].y;

    // 4. region north west
    boundingBox[NE][0].x = m_pivot.x;
    boundingBox[NE][0].y = m_pivot.y;
    boundingBox[NE][1].x = m_corner[1].x;
    boundingBox[NE][1].y = m_corner[1].y;

    for(int i=0; i<4; i++)
        {
        boundingBox[i][0].z = m_corner[0].z;
        boundingBox[i][1].z = m_corner[1].z;

        m_region[i] = PointCloudQuadNode::Create(boundingBox[i], m_tree, this, m_depth+1);
        m_region[i]->loadData();
        size_t pointsRead = m_region[i]->m_nOfPointsRead;
        if (pointsRead >= m_tree->getMaxSize())
            {
            m_region[i]->unloadData();
            int tmpDepth = m_region[i]->createSubRegion(currentDepth + 1, pointsRead);
            depth[i] += max(m_depth, tmpDepth);
            }
        else
            {
            m_region[i]->SetDataSize(pointsRead);
            depth[i] = m_region[i]->GetDepth();
            }
        m_dataSize += m_region[i]->GetDataSize();
        }
    return *std::max_element(depth, depth+4);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudQuadNode::createTrianglesRegions()
    {
    size_t startIdx;
    size_t size = m_tries.size();
    for(size_t i = 0; i<size; i++)
        {
        startIdx = m_tries[i];
        DPoint3d triangle[3];

        triangle[0] = m_tree->GetPoint(m_tree->GetTriangleIndexe(startIdx));
        triangle[1] = m_tree->GetPoint(m_tree->GetTriangleIndexe(startIdx + 1));
        triangle[2] = m_tree->GetPoint(m_tree->GetTriangleIndexe(startIdx + 2));
        addTriangleToRegion(triangle, startIdx);
        }
    for(size_t i = 0; i<4; i++)
        {
        if(m_region[i]->m_dataSize > m_tree->getMaxSize())
            m_region[i]->createTrianglesRegions();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool isPointInTriangle(DPoint3d& vecIn3, DPoint3d& vec13, DPoint3d& vec23, DPoint3d& vec33)
    {
    DPoint2d vecIn = {vecIn3.x, vecIn3.y};
    DPoint2d vec1 = {vec13.x, vec13.y};
    DPoint2d vec2 = {vec23.x, vec23.y};
    DPoint2d vec3 = {vec33.x, vec33.y};
    DPoint2d diff1;
    diff1.differenceOf(&vec2, &vec1); //vec2 - vec1;
    DPoint2d diff2;
    diff2.differenceOf(&vec3, &vec1);
    DPoint2d diff3;
    diff3.differenceOf(&vecIn, &vec1);

    DPoint2d cross1, cross2;
    cross1.x = -diff1.y;
    cross1.y = diff1.x;

    cross2.x = diff2.y;
    cross2.y = diff2.x;

    double deno = diff1.dotProduct(&cross2);
    if(deno==0)
        return false;
    double s = diff3.dotProduct(&cross2) / deno;

    deno = diff2.dotProduct(&cross1);
    if(deno==0)
        return false;
    double v = diff3.dotProduct(&cross1) / deno;

    if(s >= 0 && v >= 0 && ((s + v) <= 1))
        // point is inside or on the edge of this triangle
        return true;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudQuadNode::addTriangleToRegion(DPoint3d* triangle, size_t startIdx)
    {
    m_bitRegions = 0;
    if(isPointInTriangle(m_pivot, triangle[0], triangle[1], triangle[2]))
        {
        addToRegion(startIdx, SW);
        addToRegion(startIdx, SE);
        addToRegion(startIdx, NW);
        addToRegion(startIdx, NE);
        return;
        }
    findTriangleIntersection(triangle, startIdx);

    
    if(!m_bitRegions)
        {
        // we didn't find any intersection so we add this triangle to a point's region
        int region = findRegion(triangle[0]);
        m_region[region]->m_tries.push_back(startIdx);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
size_t PointCloudQuadNode::getAllDataPoints(std::vector<DPoint3d>& pointsBuffer)
    {    
    IPointCloudDataQueryPtr query = IPointCloudDataQuery::CreateBoundingBoxQuery (*m_tree->getElHandle(), m_corner[0], m_corner[1]);
    
    query->SetMode(IPointCloudDataQuery::QUERY_MODE_FILE, -1);            

    IPointCloudQueryBuffersPtr queryBuffer(query->CreateBuffers((uint32_t)m_tree->GetMaxDataSize(), (uint32_t)PointCloudChannelId::Xyz | (uint32_t)PointCloudChannelId::Filter));

    size_t pointsReadTmp = query->GetPoints (*queryBuffer.get());
    size_t pointsRead = 0;

    unsigned char* pFilterBuffer(queryBuffer->GetFilterBuffer());
    DPoint3d* pXyzBuffer(queryBuffer->GetXyzBuffer());

    for (size_t i = 0; i<pointsReadTmp; i++, pFilterBuffer++, pXyzBuffer++)
        {            
        if (!PointCloudChannels_Is_Point_Visible(*pFilterBuffer))
            continue;
        pointsRead++;
        pointsBuffer.push_back(*pXyzBuffer);
        }

    return pointsRead;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
size_t PointCloudQuadNode::getAllDataPoints(IPointCloudDataQueryPtr& query, IPointCloudQueryBuffersPtr& queryBuffer, IPointCloudChannelVector& queryChannels)
    {    
    query = IPointCloudDataQuery::CreateBoundingBoxQuery (*m_tree->getElHandle(), m_corner[0], m_corner[1]);        
    
    query->SetMode(IPointCloudDataQuery::QUERY_MODE_FILE, -1);            

    queryBuffer = query->CreateBuffers((uint32_t)m_tree->GetMaxDataSize(), (uint32_t)PointCloudChannelId::Xyz | (uint32_t)PointCloudChannelId::Classification | (uint32_t)PointCloudChannelId::Filter, queryChannels);
            
    return query->GetPoints (*queryBuffer.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
// For debug
#if 0 //NEEDS_WORK_SM_IMPORTER Not required?
void PointCloudQuadNode::drawNode(EditElementHandleR elHandle)
    {
    std::vector<DPoint3d> verticesAxes(5);

    verticesAxes[0].x	= m_corner[0].x;
    verticesAxes[0].y	= m_corner[0].y;
    verticesAxes[0].z   = m_corner[1].z;

    verticesAxes[1].x	= m_corner[1].x;
    verticesAxes[1].y	= m_corner[0].y;
    verticesAxes[1].z   = m_corner[1].z;

    verticesAxes[2].x	= m_corner[1].x;
    verticesAxes[2].y	= m_corner[1].y;
    verticesAxes[2].z   = m_corner[1].z;

    verticesAxes[3].x	= m_corner[0].x;
    verticesAxes[3].y	= m_corner[1].y;
    verticesAxes[3].z   = m_corner[1].z;

    verticesAxes[4].x	= m_corner[0].x;
    verticesAxes[4].y	= m_corner[0].y;
    verticesAxes[4].z   = m_corner[1].z;

    DPoint3d linePoints[2];
    MSElement lineElm;
    MSElementDescrP lineElmdscrP;

    for(int i = 0; i<4; i++)
        {
        linePoints[0] = verticesAxes[i];
        linePoints[1] = verticesAxes[i+1];
        mdlLineString_create(&lineElm, NULL, linePoints, 2);
        mdlElmdscr_new (&lineElmdscrP, NULL, &lineElm);
        EditElementHandle lineElement(lineElmdscrP, true, true, elHandle.GetModelRef());
        uint32_t color = 6+m_depth*16;
        if(color > 246)
            color=246;
        uint32_t weight = m_tree->getMaxDepth()-m_depth;
        Int32 style = 0;
        mdlElement_setSymbology(lineElement.GetElementP(), &color, &weight, &style);
        lineElement.AddToModel();
        }


    if(m_dataSize > m_tree->getMaxSize())
        for(size_t i = 0; i < 4; i++)
            m_region[i]->drawNode(elHandle);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
// Search seeds in QuadTree, and add it.
bool PointCloudQuadNode::seedSearch(std::vector<QuadSeedPtr>& seeds)
    {
    if (m_dataSize == 0)
        {
        double nodesFetched = pow(4, (m_tree->getMaxDepth() - m_depth));
        m_tree->m_nbNodeFetched += nodesFetched;
        }
    /*else if (minDistance != 0 && (m_corner[1].x - m_corner[0].x) < distanceThreshold && (m_corner[1].x - m_corner[0].x) < minDistance)
        {
        double nodesFetched = pow(4, (m_tree->getMaxDepth()-m_depth));
        m_tree->m_nbNodeFetched += nodesFetched;

        //progress.UpdateProgress(m_tree->m_nbNodeFetched);
       //m_parent->fetchSeedsFromQuadTree(seeds);
        QuadSeedPtr seed(quadSeed::Create());
        seed->tile = m_parent;
        seeds.push_back(seed);
        }*/
    else if (m_dataSize < m_tree->getMaxSize())
        {
        double nodesFetched = pow(4, (m_tree->getMaxDepth() - m_depth));
        m_tree->m_nbNodeFetched += nodesFetched;
        QuadSeedPtr seed(quadSeed::Create());
        seed->tile = this;
        seeds.push_back(seed);
        }
    else
        {
        for(size_t i = 0; i < 4; i++)
            {
            //return true if process should continue (was not canceled)
            if (!m_region[i]->seedSearch(seeds))
                return false;//Was canceled;
            }
        }

    //return true if process should continue (was not canceled)
    return true; //progress.InProgress();        
    }

void PointCloudQuadNode::loadData(bool loadClassif)
    {
    ClassificationChannelHandler* pData = ClassificationChannelHandler::GetChannelHandler(*m_tree->getElHandle());
    IPointCloudChannelPtr channel = pData->GetChannel();

    IPointCloudChannelVector queryChannels;
    queryChannels.push_back(channel.get());

    m_query = IPointCloudDataQuery::CreateBoundingBoxQuery(*m_tree->getElHandle(), m_corner[0], m_corner[1]);
    m_query->SetDensity(IPointCloudDataQuery::QUERY_DENSITY::QUERY_DENSITY_FULL, m_tree->m_densityVal);

    m_query->SetMode(IPointCloudDataQuery::QUERY_MODE_FILE, -1);

    uint32_t channelFlags = (uint32_t)PointCloudChannelId::Xyz | (uint32_t)PointCloudChannelId::Classification | (uint32_t)PointCloudChannelId::Filter;

    m_buffers = m_query->CreateBuffers((uint32_t)m_tree->GetMaxDataSize(), channelFlags, queryChannels);

    m_nOfPointsRead = m_query->GetPoints(*m_buffers.get());
    m_pFilterBuffer = new unsigned char[m_nOfPointsRead];
    m_pXyzBuffer = new DPoint3d[m_nOfPointsRead];
    if (loadClassif) m_pChannelBuffer = new unsigned char[m_nOfPointsRead];
    memcpy(m_pFilterBuffer, m_buffers->GetFilterBuffer(), m_nOfPointsRead*sizeof(unsigned char));
    memcpy(m_pXyzBuffer, m_buffers->GetXyzBuffer(), m_nOfPointsRead*sizeof(DPoint3d));
    if (loadClassif) memcpy(m_pChannelBuffer, (unsigned char*)m_buffers->GetChannelBuffer(queryChannels[0]), m_nOfPointsRead*sizeof(unsigned char));
    }

void PointCloudQuadNode::saveClassification()
    {
    if (m_pChannelBuffer == nullptr) return;
    ClassificationChannelHandler* pData = ClassificationChannelHandler::GetChannelHandler(*m_tree->getElHandle());
    IPointCloudChannelPtr channel = pData->GetChannel();

    IPointCloudChannelVector queryChannels;
    queryChannels.push_back(channel.get());
    memcpy((unsigned char*)m_buffers->GetChannelBuffer(queryChannels[0]), m_pChannelBuffer, m_nOfPointsRead*sizeof(unsigned char));
    m_query->SubmitUpdate(queryChannels[0]);
    pData->SetHasPendingChange(true);
    }

void PointCloudQuadNode::unloadData()
    {
    if (m_pXyzBuffer != nullptr)
        {
        delete[] m_pFilterBuffer;
        m_pFilterBuffer = nullptr;
        delete[] m_pXyzBuffer;
        m_pXyzBuffer = nullptr;
        m_query = nullptr;
        m_buffers = nullptr;
        }
    if (m_pChannelBuffer != nullptr)
        {
        delete[] m_pChannelBuffer;
        m_pChannelBuffer = nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
// Search all seeds in this Node and Add it
void PointCloudQuadNode::fetchSeedsFromQuadTree(QuadSeedPtr& seed)
    {        
    if (nullptr == m_pXyzBuffer) loadData();
    std::vector<DPoint3d> lowestPointsInTile;
    if (m_nOfPointsRead == 0)
        {
        seed->density = seed->trustworthiness = 0;
        return;
        }
    double seedFactor = SeedSelectionParams::deeperTilesHaveFewerSeedPoints && m_depth > SeedSelectionParams::relativeDepthThresholdToPickFewerSeedPoints * m_tree->getMaxDepth() ? SeedSelectionParams::correctiveFactorForNumberOfSeedPoints : 1;

    //how many seeds are being picked for a given tile. This is a heuristic, but basically higher density(depth) nodes have less
    //because they have many neighbors to provide points. Nodes should have 3 points at least. 
    //Because a tile has 0..10 000 points, the number of seed points taken is 3 to 50. 
    size_t maxLowestPoints = max((size_t)ceil((double)m_nOfPointsRead / (SeedSelectionParams::numberOfPointsInTilePerSeedPoint * seedFactor)), (size_t)3);
    lowestPointsInTile.reserve(maxLowestPoints);

    std::vector<size_t> indexPts;
    auto dPoint3dCompare = [] (const DPoint3d& a, const DPoint3d& b)
        {
        if (fabs(a.x - b.x) <= 0.0001 && fabs(a.y - b.y) <= 0.0001 && fabs(a.z - b.z) <= 0.0001) return false;
        else if (a.z < b.z || (a.z == b.z && a.y < b.y) || ((a.z == b.z && a.y == b.y) && a.x < b.x)) return true;
        else return false;
        };
    DPoint3d* pXyzBuffer = m_pXyzBuffer;
    UChar* pFilterBuffer = m_pFilterBuffer;
    std::set<DPoint3d, std::function<bool(const DPoint3d&, const DPoint3d&)>> pointSet(dPoint3dCompare);
    double density = m_nOfPointsRead / DVec3d::FromStartEnd(m_corner[0], m_corner[1]).Magnitude();
    size_t numberOfBins = TileProfilingParams::numberOfBinsPerAxisInTileHistogram * TileProfilingParams::numberOfBinsPerAxisInTileHistogram;
    std::vector<DRange1d> bins(numberOfBins, DRange1d::NullRange());
    std::vector<DRange3d> extents(numberOfBins);
    std::vector<DPoint3d> lowestPts(numberOfBins);
    for (size_t i = 0; i < TileProfilingParams::numberOfBinsPerAxisInTileHistogram; i++)
        {
        for (size_t j = 0; j < TileProfilingParams::numberOfBinsPerAxisInTileHistogram; j++)
            {
            extents[i * TileProfilingParams::numberOfBinsPerAxisInTileHistogram + j] = DRange3d::From(m_corner[0], m_corner[1]);
            extents[i * TileProfilingParams::numberOfBinsPerAxisInTileHistogram + j].low.x = m_corner[0].x + (double)i / TileProfilingParams::numberOfBinsPerAxisInTileHistogram * (m_corner[1].x - m_corner[0].x);
            extents[i * TileProfilingParams::numberOfBinsPerAxisInTileHistogram + j].low.y = m_corner[0].y + (double)j / TileProfilingParams::numberOfBinsPerAxisInTileHistogram * (m_corner[1].y - m_corner[0].y);
            extents[i * TileProfilingParams::numberOfBinsPerAxisInTileHistogram + j].high.x = extents[i * TileProfilingParams::numberOfBinsPerAxisInTileHistogram + j].low.x + (double)(m_corner[1].x - m_corner[0].x) / TileProfilingParams::numberOfBinsPerAxisInTileHistogram;
            extents[i * TileProfilingParams::numberOfBinsPerAxisInTileHistogram + j].high.y = extents[i * TileProfilingParams::numberOfBinsPerAxisInTileHistogram + j].low.y + (double)(m_corner[1].y - m_corner[0].y) / TileProfilingParams::numberOfBinsPerAxisInTileHistogram;
            }
        }

    //for all points in the tile, compute the histogram of lowest and highest points, and order points from lowest to highest
    for (size_t i = 0; i < m_nOfPointsRead; i++, pFilterBuffer++, pXyzBuffer++)
        {        
        if (!PointCloudChannels_Is_Point_Visible(*pFilterBuffer))
            continue;
        DPoint3d currentPoint = *pXyzBuffer;
        pointSet.insert(currentPoint);
        for (size_t ext = 0; ext < extents.size(); ext++)
            {
            if (extents[ext].IsContainedXY(currentPoint))
                {
                bins[ext].Extend(currentPoint.z);
                if (lowestPts[ext].x == 0 && lowestPts[ext].y == 0 && lowestPts[ext].z == 0) lowestPts[ext] = currentPoint;
                else if (bins[ext].low == currentPoint.z) lowestPts[ext] = currentPoint;
                break;
                }
            }
        }

    double varHighestPt = 0;
    double varLowestPt = 0;
    std::set<double> lowestPtsHistogram;
    size_t observations = 0;
    std::set<double> varLowestPts;
    //This computes the variation of the height of the lowest and highest points between subdivisions of the node. It helps give an profile
    //of this node's outliers, terrain slope, and amount of likely ground vs likely object points.
    for (size_t ext = 0; ext < bins.size(); ext++)
        {
        if (bins[ext].IsNull()) continue;
        lowestPtsHistogram.insert(bins[ext].low);
        if (ext + TileProfilingParams::numberOfBinsPerAxisInTileHistogram < bins.size() && !bins[ext + TileProfilingParams::numberOfBinsPerAxisInTileHistogram].IsNull())
            {
            observations++;
            varHighestPt += fabs(bins[ext + TileProfilingParams::numberOfBinsPerAxisInTileHistogram].high - bins[ext].high);
            varLowestPt += fabs(bins[ext + TileProfilingParams::numberOfBinsPerAxisInTileHistogram].low - bins[ext].low);
            if (fabs(bins[ext + TileProfilingParams::numberOfBinsPerAxisInTileHistogram].low - bins[ext].low) > 0)  varLowestPts.insert(fabs(bins[ext + TileProfilingParams::numberOfBinsPerAxisInTileHistogram].low - bins[ext].low));
            }
        if (ext % TileProfilingParams::numberOfBinsPerAxisInTileHistogram < (TileProfilingParams::numberOfBinsPerAxisInTileHistogram-1) && !bins[ext + 1].IsNull())
            {
            observations++;
            varHighestPt += fabs(bins[ext + 1].high - bins[ext].high);
            varLowestPt += fabs(bins[ext + 1].low - bins[ext].low);
            if (fabs(bins[ext + 1].low - bins[ext].low) > 0 ) varLowestPts.insert(fabs(bins[ext + 1].low - bins[ext].low));
            }
        }
    double maxNonOutlierVariationInLowestPt = *std::next(varLowestPts.begin(), varLowestPts.size()*TileProfilingParams::maxPercentileForOutlierDetectionInVariation);
    double minNonOutlierVariationInLowestPt = *std::next(varLowestPts.begin(), varLowestPts.size()*TileProfilingParams::minPercentileForOutlierDetectionInVariation);
    double medNonOutlierVariationInLowestPt = *std::next(varLowestPts.begin(), varLowestPts.size()*0.5);
   // double coeff = 1 - ((q3 - q1) / (q3 + q1));
    double IQRvar = 1.5*(maxNonOutlierVariationInLowestPt - minNonOutlierVariationInLowestPt);
    double varLowestPtAbs = (*std::next(lowestPtsHistogram.begin(), lowestPtsHistogram.size()*TileProfilingParams::maxPercentileForOutlierDetectionInVariation)) - *lowestPtsHistogram.begin();//fullExtent.high - fullExtent.low;
    auto endIt = pointSet.begin();
    size_t nOfLowestPts = 0;
    double minZ = (*std::next(pointSet.begin(), (int)pointSet.size()*TileProfilingParams::minPercentileForOutlierDetectionInPointSet)).z;
    double lastZ = minZ;
    double zvar = ((*std::next(pointSet.begin(), (int)pointSet.size()*TileProfilingParams::maxPercentileForOutlierDetectionInPointSet)).z - minZ);

    //Nodes are more trustworthy (more likely to need to grow their ground points) if:
    // * the variability (in z) of their lowest points is a large part of the variability of the whole tile, i.e, there are no spikes, large objects, etc
    // * the average variation between the lowest points of two neighboring sub-sections is low, i.e, there are no obvious tops of buildings or cliffs
    // * the spread of the variation between the lowest points of two neighboring sub-sections is low, i.e, there are no obvious outliers 
    seed->trustworthiness = varLowestPtAbs / zvar * (varLowestPtAbs / (varLowestPt / observations))*(minNonOutlierVariationInLowestPt / maxNonOutlierVariationInLowestPt) / (0.3);// *1.5*coeff;
    double tolerance = IQRvar + maxNonOutlierVariationInLowestPt; //statistically, how much variation we can expect between somewhat close ground points.
    if (minNonOutlierVariationInLowestPt / maxNonOutlierVariationInLowestPt < TileProfilingParams::acceptableRatioBetweenNonOutlierVariations
        || (TileProfilingParams::assumeNoLowOutliers && ((*varLowestPts.begin()) / minNonOutlierVariationInLowestPt) < TileProfilingParams::acceptableRatioBetweenNonOutlierVariations))
        //this means a large proportion of the tile may have noise or object points that are not consistent with the variation in elevation globally
        {
        seed->trustworthiness = 0;
        if(TileProfilingParams::adjustToleranceValue) tolerance = minNonOutlierVariationInLowestPt;
        }
    else if (TileProfilingParams::adjustToleranceValue && seed->trustworthiness > 10) tolerance = IQRvar + *std::next(varLowestPts.begin(), varLowestPts.size()*0.95);
    auto beginIt = endIt;
    while (endIt != pointSet.end() && nOfLowestPts < maxLowestPoints)
        {
        if (!(TileProfilingParams::assumeNoLowOutliers) && (endIt->z < minZ && (minZ - endIt->z > varLowestPtAbs || minZ - endIt->z > tolerance)))
            {
            ++beginIt;
            ++endIt;
            continue;
            }
        if (endIt->z - minZ > varLowestPtAbs || endIt->z - lastZ > tolerance) break;
        nOfLowestPts++;
        lastZ = endIt->z;
        endIt++;
        }

    seed->seedPoints.assign(beginIt, endIt);
    size_t nPointsAdded = 0;
    //We "help" nodes that are not likely to have outliers have better spread for their seeds by seeding all subsections, unless they're obvious outliers.
    for (size_t ext = 0; seed->trustworthiness > SeedSelectionParams::trustworthinessThresholdForExtraSeeds && ext < bins.size(); ext++)
        {
        bool pickSeedFromThisBin = false;
        if (SeedSelectionParams::extraSeedSelectionCondition == SeedSelectionConditions::USE_AVERAGE_AND_IQR)
            {

            }
        else if (SeedSelectionParams::extraSeedSelectionCondition == SeedSelectionConditions::USE_RATIO_BETWEEN_PERCENTILES)
            {
            pickSeedFromThisBin = !bins[ext].IsNull() && ((lowestPts[ext].z - lastZ < medNonOutlierVariationInLowestPt*(medNonOutlierVariationInLowestPt / maxNonOutlierVariationInLowestPt) / SeedSelectionParams::acceptableRatioBetweenMedianAndMax* (minNonOutlierVariationInLowestPt / maxNonOutlierVariationInLowestPt) / SeedSelectionParams::acceptableRatioBetweenMaxAndMin)
                                                          || (lowestPts[ext].z - lastZ < maxNonOutlierVariationInLowestPt + IQRvar &&seed->trustworthiness > SeedSelectionParams::trustworthinessThresholdForWaivingExtraSeedCondition)) && minZ - lowestPts[ext].z < tolerance;
            }
        if (pickSeedFromThisBin)
            {
            seed->seedPoints.push_back(lowestPts[ext]);
            nPointsAdded++;
            if (lowestPts[ext].z > lastZ) lastZ = lowestPts[ext].z;
            }
        
        }

    if (m_corner[0].x < 77109 && m_corner[1].x > 77109 && m_corner[0].y < -136700 && m_corner[1].y > -136700)
        {
        std::string s;
        s += "POINTS ADDED " + std::to_string(nPointsAdded) + " N LOWEST PTS "+std::to_string(nOfLowestPts)+" OUT OF MAX " + std::to_string(maxLowestPoints);
        }
    seed->tile = this;
    seed->density = density;
    setSubreeMark();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudQuadNode::visitUnmarkedNodes(std::vector<QuadSeedPtr>& seeds)
    {
    if(!m_marked)
        {
        QuadSeedPtr seed(quadSeed::Create());
        seed->tile = this;
        seeds.push_back(seed);
        for(size_t i =0; i < 4; i++)
            if(m_region[i].IsValid())
                m_region[i]->visitUnmarkedNodes(seeds);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudQuadNode::setSubreeMark()
    {
    if(m_dataSize > m_tree->getMaxSize())
        for(size_t i = 0; i< 4; i++)
            m_region[i]->setMarked(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudQuadNode::findTriangleIntersection(DPoint3d* triangle, size_t startIdx)
    {
    int k = 2;
    DPoint3d direction;
    for(int i=0; i<3; k=i++)
        {
        direction.differenceOf(&triangle[i], &triangle[k]);
        if(direction.x)
            findIntersectionWithX(direction, triangle, startIdx, k);
        if(direction.y)
            findIntersectionWithY(direction, triangle, startIdx, k);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudQuadNode::findIntersectionWithX(DPoint3d& direction, DPoint3d* triangle, size_t startIdx, int k)
    {
    // Find intersection with plane x = m_pivot.dx
    double t = (m_pivot.x - triangle[k].x) / (direction.x);

    if(t < (1+m_tree->GetEpsilon()) && t > -m_tree->GetEpsilon())
        {
        // we have an intersection
        double yComponent = triangle[k].y + t * (direction.y);
        if(yComponent < m_pivot.y)
            {
            if(yComponent >= m_corner[0].y)
                {
                addToRegion(startIdx, SW);
                addToRegion(startIdx, SE);
                }
            }
        else if(yComponent <= m_corner[1].y)
            {
            addToRegion(startIdx, NW);
            addToRegion(startIdx, NE);
            }
        }
    // find intersection with plane x = m_boundingBox[0].x
    t = (m_corner[0].x - triangle[k].x) / direction.x;
    if(t < (1 + m_tree->GetEpsilon()) && t > -m_tree->GetEpsilon())
        {
        // we have an intersection
        double yComponenet = triangle[k].y + t * direction.y;
        if(yComponenet <= m_pivot.y && yComponenet >= m_corner[0].y)
            addToRegion(startIdx, SW);
        else if (yComponenet >= m_pivot.y && yComponenet <= m_corner[1].y)
            addToRegion(startIdx, NW);
        }
    // find intersection with plane x = m_boundingBox[1].x
    t = (m_corner[1].x - triangle[k].x) / (direction.x);
    if (t < (1 + m_tree->GetEpsilon()) && t > -m_tree->GetEpsilon())
        {
        // we have an intersection
        double yComponent = triangle[k].y + t * (direction.y);

        if (yComponent <= m_pivot.y && yComponent >= m_corner[0].y)
            addToRegion(startIdx, SE);
        else if (yComponent >= m_pivot.y && yComponent <= m_corner[1].y)
            addToRegion(startIdx, NE);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudQuadNode::findIntersectionWithY(DPoint3d& direction, DPoint3d* triangle, size_t startIdx, int k)
    {
    // find intersection with plane y = m_pivot.dY
    double t = (m_pivot.y - triangle[k].y) / (direction.y);
    if (t < (1 + m_tree->GetEpsilon()) && t > -m_tree->GetEpsilon())
        {
        // we have an intersection
        double xComponent = triangle[k].x + t * (direction.x);

        if (xComponent > m_pivot.x)
            {
            if (xComponent <= m_corner[1].x)
                {
                addToRegion(startIdx, SE);
                addToRegion(startIdx, NE);
                }
            }
        else if (xComponent >= m_corner[0].x)
            {
            addToRegion(startIdx, SW);
            addToRegion(startIdx, NW);
            }
        }
    // find intersection with plane y = m_boundingBox[0].dY
    t = (m_corner[0].y - triangle[k].x) / (direction.y);
    if (t < (1 + m_tree->GetEpsilon()) && t > -m_tree->GetEpsilon())
        {
        // we have an intersection
        double xComponent = triangle[k].x + t * (direction.x);

        if (xComponent <= m_pivot.x && xComponent >= m_corner[0].x)
            addToRegion(startIdx, SW);
        else if (xComponent >= m_pivot.x && xComponent <= m_corner[1].x)
            addToRegion(startIdx, SE);
        }
    // find intersection with plane y = m_boundingBox[1].dY
    t = (m_corner[1].y - triangle[k].y) / (direction.y);
    if (t < (1 + m_tree->GetEpsilon()) && t > -m_tree->GetEpsilon())
        {
        // we have an intersection
        double xComponent = triangle[k].x + t * (direction.x);

        if (xComponent <= m_pivot.x && xComponent >= m_corner[0].x)
            addToRegion(startIdx, NW);
        else if (xComponent >= m_pivot.x && xComponent <= m_corner[1].x)
            addToRegion(startIdx, NE);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<size_t>& PointCloudQuadNode::findTriesParent()
    {
    if(m_tries.empty())
        return m_parent->findTriesParent();
    return m_tries;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<size_t>& PointCloudQuadNode::findTries(DPoint3d& searchPoint)
    {
    int region = findRegion(searchPoint);
    if(m_region[region].IsNull())
        {
        if(m_tries.empty())
            return m_parent->findTriesParent();
        return m_tries;
        }
    return m_region[region]->findTries(searchPoint);
    }

PointCloudQuadNode* PointCloudQuadNode::findNode(DPoint3dCR searchPoint)
    {
    int region = findRegion(searchPoint);
    if (m_region[region].IsNull())
        {
        return this;
        }
    return m_region[region]->findNode(searchPoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Thomas.Butzbach                 02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline int PointCloudQuadNode::findRegion(DPoint3dCR point)
    {
    int base = 2;
    if(point.y < m_pivot.y)
        base = 0;
    if (point.x > m_pivot.x)
        base ++;
    return base;
    }


END_BENTLEY_SCALABLEMESH_NAMESPACE
    