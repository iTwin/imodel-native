/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnRangeTree.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

BEGIN_UNNAMED_NAMESPACE

typedef DgnRangeTree::Node&         DRTNodeR;
typedef DgnRangeTree::Node*         DRTNodeP;
typedef DRTNodeP*                   DRTNodeH;
typedef DgnRangeTree::LeafNode*     DRTLeafNodeP;
typedef DgnRangeTree::InternalNode* DRTInternalNodeP;

static const double   s_cameraLimit      = 1.0E-5;
enum
    {
    MAX_OcclusionBatch = 1000,
    };

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2009
+===============+===============+===============+===============+===============+======*/
struct  DRTPrefs
{
    double      m_minimumOcclusionPixelRatio;
    double      m_maxOcclusionTestArea;
    double      m_lodFilterFraction;
    double      m_testLOD;
    double      m_minLODFilterNPC;
    uint32_t    m_targetLeafCount;
    uint32_t    m_nOcclusionBatches;
    uint32_t    m_minimumOcclusionNodeTest;
    bool        m_doOcclusionCull;
    bool        m_doLodFiltering;
    bool        m_completeStatistics;
    bool        m_weightScoreByZed;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DRTPrefs()
    {
    m_minimumOcclusionPixelRatio = 500.0;
    m_maxOcclusionTestArea = .2;
    m_targetLeafCount = 15000;
    m_nOcclusionBatches = 10;
    m_doOcclusionCull = true;
    m_lodFilterFraction = .25;
    m_minLODFilterNPC = .0001;
    m_minimumOcclusionNodeTest = 20;
    m_doLodFiltering = true;
    m_weightScoreByZed = true;
    m_completeStatistics = true;
    }
};

static DRTPrefs  s_prefs;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2005
+---------------+---------------+---------------+---------------+---------------+------*/
static inline double timerGetResolution() { return 1.0; }
static inline double timeGetTime() { return (double)BeTimeUtilities::QuerySecondsCounter(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2005
+---------------+---------------+---------------+---------------+---------------+------*/
static inline double   timeGetSeconds()
    {
    static double  s_timerResolution = timerGetResolution();

    return timeGetTime() / s_timerResolution;
    }

//#define DRT_DEBUGGING

#if defined (DRT_DEBUGGING)
#define INCLUDE_TIMER(t)            IncludeTimer  includer(t)
#define EXCLUDE_TIMER(t)            ExcludeTimer  excluder(t)
#define TIMER_OVERHEAD s_statistics.m_timerOverhead.m_value
#define BEGIN_DELTA_TIMER(timer)    timer -= (timeGetTime() + TIMER_OVERHEAD); s_statistics.m_traverse.m_timerOverhead += TIMER_OVERHEAD;
#define END_DELTA_TIMER(timer)      timer += timeGetTime(); s_statistics.m_traverse.m_timerOverhead += TIMER_OVERHEAD;
#define BEGIN_NET_TIMER(timer)      timer -= (timeGetTime() + TIMER_OVERHEAD); s_statistics.m_traverse.m_timerOverhead += TIMER_OVERHEAD;
#define END_NET_TIMER(timer)        timer += timeGetTime(); s_statistics.m_traverse.m_timerOverhead += TIMER_OVERHEAD;
#define PERCENT(n,total)            (0.0 == total ? 0.0 : (100.0 * (double) n / (double) total))

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2009
+===============+===============+===============+===============+===============+======*/
struct DRTStatistics
{
    Logging::ILogger* m_myLogger;
    double   m_deltaTime;

    struct TimerOverhead
        {
        double  m_value;
        void DoTest() {double  time1 = timeGetTime(), time2 = timeGetTime(); m_value = time2 - time1;}
        } m_timerOverhead;

    struct Create
        {
        double  m_createTime;
        double  m_gatherTime;
        double  m_treeTime;
        size_t  m_modelCount;
        size_t  m_leafNodeBytes;
        size_t  m_internalNodeBytes;
        void Clear() { memset(this, 0, sizeof (*this)); }
        } m_create;

    struct Traversal
        {
        double  m_totalTime;
        double  m_newModelTime;
        double  m_pushModelTime;
        double  m_gatherTime;
        double  m_sortTime;
        double  m_occlusionTime;
        double  m_maxOcclusionTime;
        double  m_scoringTime;
        double  m_elementTime;
        double  m_maxElementTime;
        double  m_timerOverhead;
        size_t  m_elementVisitCount;
        size_t  m_elementTargetCount;
        size_t  m_leafVisitCount;
        size_t  m_rangeTestCount;
        size_t  m_leavesGathered;
        size_t  m_leavesOcclusionTested;
        size_t  m_leavesOccluded;
        size_t  m_elementsOcclusionTested;
        size_t  m_elementsOccluded;
        size_t  m_occlusionCalls;
        size_t  m_modelChangeCount;
        size_t  m_lodFilteredElementCount;
        size_t  m_lodFilteredNodeCount;
        double  m_lodArea;
        void Clear() { memset(this, 0, sizeof (*this)); }
        } m_traverse;

    void ClearCreate()   { m_create.Clear(); m_timerOverhead.DoTest(); GetLogger();}
    void ClearTraverse() { m_traverse.Clear(); m_timerOverhead.DoTest(); GetLogger();}

    void GetLogger()
        {
        if (nullptr == m_myLogger)
            m_myLogger = LoggingManager::GetLogger(L"DgnCore.DgnRangeTree");
        }

    void Log(WCharCP msg, ...)
        {
        va_list arglist;
        va_start(arglist, msg);
        m_myLogger->messageva(Logging::SEVERITY.LOG_INFO, msg, arglist);
        va_end(arglist);
        }

    void Log(WString& string) {Log(string.c_str());}
    void Log(char* string) {Log(WString(string));}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
WString   TimeString(double timer)
    {
    double  seconds = timer / timerGetResolution();
    char    string[1024];

    if (seconds > 60.0)
        sprintf(string, "%f Minutes", seconds / 60.0);
    else
        sprintf(string, "%f Seconds", seconds);

    return WString(string);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
double SecondsPer(double timer, size_t count)
    {
    if (0 == count)
        return 0.0;

    return timer / (timerGetResolution() * (double) count);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void DumpTraverse()
    {
    Log(L"Total: %ls, Delta: %ls, Visited: Bins: %d (of %d) Elements: %d (of %d)", TimeString(m_traverse.m_totalTime).c_str(), TimeString(m_deltaTime).c_str(), m_traverse.m_leafVisitCount, m_traverse.m_leavesGathered, m_traverse.m_elementVisitCount, m_traverse.m_elementTargetCount);
    if (s_prefs.m_completeStatistics)
        {
        Log(L"Element Seconds: %f (Max: %ls), Model Change Seconds: %f", SecondsPer(m_traverse.m_elementTime, m_traverse.m_elementVisitCount), TimeString(m_traverse.m_maxElementTime).c_str(), SecondsPer(m_traverse.m_newModelTime, m_traverse.m_modelChangeCount));
        Log(L"LOD Filter Area:    %f, LOD FIlter Count (Node:Element): %d:%d", m_traverse.m_lodArea, m_traverse.m_lodFilteredNodeCount, m_traverse.m_lodFilteredElementCount);
        Log(L"Gather Time: %ls\t    (%f percent)", TimeString(m_traverse.m_gatherTime).c_str(), PERCENT (m_traverse.m_gatherTime, m_traverse.m_totalTime));
        Log(L"Sort Time: %ls\t      (%f percent)", TimeString(m_traverse.m_sortTime).c_str(), PERCENT (m_traverse.m_sortTime, m_traverse.m_totalTime));
        Log(L"Model Time: %ls\t     (%f percent)", TimeString(m_traverse.m_newModelTime).c_str(), PERCENT (m_traverse.m_newModelTime, m_traverse.m_totalTime));
        Log(L"Push Time: %ls\t      (%f percent)", TimeString(m_traverse.m_pushModelTime).c_str(), PERCENT (m_traverse.m_pushModelTime, m_traverse.m_totalTime));
        Log(L"Score Time: %ls\t     (%f percent)", TimeString(m_traverse.m_scoringTime).c_str(), PERCENT (m_traverse.m_scoringTime, m_traverse.m_totalTime));
        Log(L"Process Time: %ls\t   (%f percent)", TimeString(m_traverse.m_elementTime).c_str(), PERCENT (m_traverse.m_elementTime, m_traverse.m_totalTime));
        Log(L"Occlusion Time: %ls\t (%f percent) Max: %ls", TimeString(m_traverse.m_occlusionTime).c_str(), PERCENT (m_traverse.m_occlusionTime, m_traverse.m_totalTime), TimeString(m_traverse.m_maxOcclusionTime).c_str());
        Log(L"Accounted Time:      (%f percent)", PERCENT((m_traverse.m_newModelTime + m_traverse.m_scoringTime + m_traverse.m_gatherTime + m_traverse.m_elementTime + m_traverse.m_sortTime + m_traverse.m_occlusionTime + m_traverse.m_timerOverhead), m_traverse.m_totalTime));
        Log(L"Timer Overhead: %ls\t (%f percent)", TimeString(m_traverse.m_timerOverhead).c_str(), PERCENT (m_traverse.m_timerOverhead, m_traverse.m_totalTime));
        Log(L"Nodes Occlusion Tested: %d, Nodes Occluded: %d, Elements Occluded: %d, Occlusion Calls: %d", m_traverse.m_leavesOcclusionTested, m_traverse.m_leavesOccluded, m_traverse.m_elementsOccluded, m_traverse.m_occlusionCalls);
        }
    Log(L"______________________________________________________________");
    }

    void DumpCreate(DgnRangeTreeR tree);
};

DRTStatistics  s_statistics;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
static WString memoryString(size_t bytes)
    {
    double mb = (1024.0 * 1024.0), gb = (1024.0 * mb);
    char   string[1024];

    if ((double) bytes > .5 * gb)
        sprintf(string, "%f Gb.", (double) bytes / gb);
    else
        sprintf(string, "%f Mb.", (double) bytes / mb);

    return WString(string);
    }


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2009
+===============+===============+===============+===============+===============+======*/
struct      IncludeTimer
{
    double&             m_time;
    IncludeTimer(double& time) : m_time(time) { m_time -= (timeGetTime() +  TIMER_OVERHEAD); }
    ~IncludeTimer()                            { m_time += timeGetTime(); }
};

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2009
+===============+===============+===============+===============+===============+======*/
struct      ExcludeTimer
{
    double&             m_time;
    ExcludeTimer(double& time) : m_time(time) { m_time += timeGetTime(); }
    ~ExcludeTimer()                            { m_time -= timeGetTime(); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void DRTStatistics::DumpCreate(DgnRangeTreeR tree)
    {
    if (0 == m_create.m_modelCount)
        return;

    Log(L"*=======================================================================================================================");
    Log(L"Create Time:     %ls", TimeString(m_create.m_createTime).c_str());
    Log(L"Gather Time:     %ls", TimeString(m_create.m_gatherTime).c_str());
    Log(L"Tree Time:       %ls", TimeString(m_create.m_treeTime).c_str());
    Log(L"Model Count:     %d", m_create.m_modelCount);
    Log(L"Max Internal Children: %d, Max Leaf Children: %d", tree.GetInternalNodeSize(), tree.GetLeafNodeSize());

    Log(L"LeafNode Memory:     %ls", memoryString(m_create.m_leafNodeBytes));
    Log(L"InternalNode Memory: %ls", memoryString(m_create.m_internalNodeBytes));
    }

#else

#define INCLUDE_TIMER(t)
#define EXCLUDE_TIMER(t)
#define BEGIN_DELTA_TIMER(t)
#define END_DELTA_TIMER(t)
#define BEGIN_NET_TIMER(t)
#define END_NET_TIMER(t)
#endif

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2009
+===============+===============+===============+===============+===============+======*/
struct DRTRangeCorners
{
    DPoint3d m_points[8];

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    RayBentley      01/07
    +---------------+---------------+---------------+---------------+---------------+------*/
    inline void InitFromRange(DRange3dCR range, TransformCP trans)
        {
        // Note we don't allow the box to degenerate in any direction as this makes it impossible to extract expansion direction.
        m_points[0].x = m_points[3].x = m_points[4].x = m_points[7].x = (double) range.low.x;
        m_points[1].x = m_points[2].x = m_points[5].x = m_points[6].x = (double) ((range.high.x == range.low.x) ? (range.low.x + 1) : range.high.x);

        m_points[0].y = m_points[1].y = m_points[4].y = m_points[5].y = (double) range.low.y;
        m_points[2].y = m_points[3].y = m_points[6].y = m_points[7].y = (double) ((range.high.y == range.low.y) ? (range.low.y + 1) : range.high.y);

        m_points[0].z = m_points[1].z = m_points[2].z = m_points[3].z = (double) range.low.z;
        m_points[4].z = m_points[5].z = m_points[6].z = m_points[7].z = (double) ((range.high.z == range.low.z) ? (range.low.z + 1) : range.high.z);

        if (trans)
            trans->Multiply(m_points, 8);
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static inline bool rangeIsValid(DRange3dCR range, bool is3d)
    {
    return (range.low.x <= range.high.x) && (range.low.y <= range.high.y) && (!is3d || (range.low.z <= range.high.z));
    }

/*---------------------------------------------------------------------------------**//**
* Optimized - no function calls, no nullptr tests etc. as in DRange3d::extentSquared.
* @bsimethod                                                    RayBentley      10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static inline double rangeExtentSquared(DRange3dCR range)
    {
    double extentX = (double) range.high.x - range.low.x;
    double extentY = (double) range.high.y - range.low.y;
    double extentZ = (double) range.high.z - range.low.z;
    return extentX * extentX + extentY * extentY + extentZ * extentZ;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
static inline double rangeExtentSquared(RTree3dValCR range)
    {
    double extentX = (double) range.m_maxx - range.m_minx;
    double extentY = (double) range.m_maxy - range.m_miny;
    double extentZ = (double) range.m_maxz - range.m_minz;
    return extentX * extentX + extentY * extentY + extentZ * extentZ;
    }

/*---------------------------------------------------------------------------------**//**
* Optimized - no function calls, no nullptr tests etc. as in DRange3d::extentSquared.
* @bsimethod                                                    RayBentley      10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static inline void extendRange(DRange3dR thisRange, DPoint3dCR point)
    {
    if (point.x < thisRange.low.x)
        thisRange.low.x = point.x;

    if (point.x > thisRange.high.x)
        thisRange.high.x = point.x;

    if (point.y < thisRange.low.y)
        thisRange.low.y = point.y;

    if (point.y > thisRange.high.y)
        thisRange.high.y = point.y;

    if (point.z < thisRange.low.z)
        thisRange.low.z = point.z;

    if (point.z > thisRange.high.z)
        thisRange.high.z = point.z;
    }

/*---------------------------------------------------------------------------------**//**
* Optimized - no function calls, no nullptr tests etc. as in DRange3d::extentSquared.
* @bsimethod                                                    RayBentley      10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static inline void extendRange(DRange3dR thisRange, DRange3dCR range)
    {
    if (range.low.x < thisRange.low.x)
        thisRange.low.x = range.low.x;

    if (range.low.y < thisRange.low.y)
        thisRange.low.y = range.low.y;

    if (range.low.z < thisRange.low.z)
        thisRange.low.z = range.low.z;

    if (range.high.x > thisRange.high.x)
        thisRange.high.x = range.high.x;

    if (range.high.y > thisRange.high.y)
        thisRange.high.y = range.high.y;

    if (range.high.z > thisRange.high.z)
        thisRange.high.z = range.high.z;
    }


//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct DRTSplitEntry
{
    DRange3d    m_range;
    void*       m_entry;        // this is void* so we can use this struct for both LeafNodes and InternalNodes
    int         m_groupNumber[3];
};

typedef DRTSplitEntry* DRTSplitEntryP;
typedef DRTSplitEntry const * DRTSplitEntryCP;
typedef DRTSplitEntry const& DRTSplitEntryCR;
typedef DgnRangeTree::ProgressMonitor* ProgressMonitorP;

static inline bool compareX(DRTSplitEntryCR entry1, DRTSplitEntryCR entry2) {return entry1.m_range.low.x < entry2.m_range.low.x;}
static inline bool compareY(DRTSplitEntryCR entry1, DRTSplitEntryCR entry2) {return entry1.m_range.low.y < entry2.m_range.low.y;}
static inline bool compareZ(DRTSplitEntryCR entry1, DRTSplitEntryCR entry2) {return entry1.m_range.low.z < entry2.m_range.low.z;}
typedef bool (*PF_CompareFunc)(DRTSplitEntryCR, DRTSplitEntryCR);

enum SplitAxis {X_AXIS = 0 ,Y_AXIS = 1, Z_AXIS = 2};

/*---------------------------------------------------------------------------------**//**
* @bsimethod    RangeNode                                       KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
static double checkSeparation(DRTSplitEntryP entries, size_t count, SplitAxis axis)
    {
    double maxSeparation = 0, separation;
    double maxMinDist = 1.0e200, minDist;

    static PF_CompareFunc  s_compareFuncs[3] = { compareX, compareY, compareZ };
    std::sort(entries, entries+count, s_compareFuncs[axis]);

    size_t minSize = count/3;
    DRTSplitEntryCP lastEntry    = entries + count;
    DRTSplitEntryCP entryEnd     = entries + (count-minSize);
    DRTSplitEntryCP startEntries = entries + minSize;
    DRTSplitEntryCP sepEntry=nullptr, minDistEntry=nullptr;

    switch (axis)
        {
        case X_AXIS:
            for (DRTSplitEntryCP currEntry = startEntries; currEntry < entryEnd-1; ++currEntry)
                {
                DRTSplitEntryCP nextEntry = currEntry + 1;

                if (currEntry->m_range.high.x < nextEntry->m_range.low.x)
                    {
                    if ((separation = (nextEntry->m_range.low.x - currEntry->m_range.high.x)) > maxSeparation)
                        {
                        maxSeparation = separation;
                        sepEntry = nextEntry;
                        }
                    }
                else
                    {
                    if ((minDist = (nextEntry->m_range.low.x - currEntry->m_range.low.x)) > maxMinDist)
                        {
                        maxMinDist    = minDist;
                        minDistEntry = nextEntry;
                        }
                    }
                }
            break;

        case Y_AXIS:
            for (DRTSplitEntryCP currEntry = startEntries; currEntry < entryEnd-1; ++currEntry)
                {
                DRTSplitEntryCP nextEntry = currEntry + 1;

                if (currEntry->m_range.high.y < nextEntry->m_range.low.y)
                    {
                    if ((separation = (nextEntry->m_range.low.y - currEntry->m_range.high.y)) > maxSeparation)
                        {
                        maxSeparation = separation;
                        sepEntry = nextEntry;
                        }
                    }
                else
                    {
                    if ((minDist = (nextEntry->m_range.low.y - currEntry->m_range.low.y)) > maxMinDist)
                        {
                        maxMinDist = minDist;
                        minDistEntry = nextEntry;
                        }
                    }
                }
            break;

        case Z_AXIS:
            for (DRTSplitEntryCP currEntry = startEntries; currEntry < entryEnd-1; ++currEntry)
                {
                DRTSplitEntryCP nextEntry = currEntry + 1;

                if (currEntry->m_range.high.z < nextEntry->m_range.low.z)
                    {
                    if ((separation = (nextEntry->m_range.low.z - currEntry->m_range.high.z)) > maxSeparation)
                        {
                        maxSeparation = separation;
                        sepEntry = nextEntry;
                        }
                    }

                else
                    {
                    if ((minDist = (nextEntry->m_range.low.z - currEntry->m_range.low.z)) > maxMinDist)
                        {
                        maxMinDist = minDist;
                        minDistEntry = nextEntry;
                        }
                    }
                }
            break;
        }

    if (nullptr == sepEntry)
        {
        if (nullptr != minDistEntry)
            sepEntry = minDistEntry;
        else
            sepEntry = entries + count/2;
        }

    for (DRTSplitEntryP currEntry = entries; currEntry < lastEntry; ++currEntry)
        {
        if (currEntry < sepEntry)
            currEntry->m_groupNumber[axis] = 0;
        else
            currEntry->m_groupNumber[axis] = 1;
        }

    return  maxSeparation;
    }
END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
inline size_t DgnRangeTree::Node::GetEntryCount()
    {
    DRTLeafNodeP leaf = ToLeaf();
    return leaf ? leaf->GetEntryCount() : ((DRTInternalNodeP) this)->GetEntryCount();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnRangeTree::Node::ValidateRange()
    {
    if (!m_sloppy)
        return;

    DRTLeafNodeP leaf = ToLeaf();
    if (leaf)
        leaf->ValidateLeafRange();
    else
        ((DRTInternalNodeP) this)->ValidateInternalRange();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnRangeTree::Match DgnRangeTree::Node::Traverse(Traverser& traverser, bool is3d)
    {
    DRTLeafNodeP leaf = ToLeaf();
    return leaf ? leaf->Traverse(traverser, is3d) : ((DRTInternalNodeP) this)->Traverse(traverser, is3d);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
inline void DgnRangeTree::InternalNode::ValidateInternalRange()
    {
    if (!m_sloppy)
        return;

    m_sloppy = false;
    ClearRange();
    for (DRTNodeH curr = &m_firstChild[0]; curr < m_endChild; ++curr)
        extendRange(m_nodeRange, (*curr)->GetRange());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
inline void DgnRangeTree::LeafNode::ValidateLeafRange()
    {
    if (!m_sloppy)
        return;

    m_sloppy = false;
    ClearRange();

    for (Entry* curr = &m_firstChild[0]; curr < m_endChild; ++curr)
        extendRange(m_nodeRange, curr->m_range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnRangeTree::Node::Overlaps(DRange3dCR range) const
    {
    if (m_nodeRange.low.x > range.high.x || m_nodeRange.high.x < range.low.x ||
        m_nodeRange.low.y > range.high.y || m_nodeRange.high.y < range.low.y)
        return  false;

    return !m_is3d ? true : (m_nodeRange.low.z <= range.high.z && m_nodeRange.high.z >= range.low.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnRangeTree::Node::CompletelyContains(DRange3dCR range) const
    {
    if (m_nodeRange.low.x >= range.low.x || m_nodeRange.high.x <= range.high.x ||
        m_nodeRange.low.y >= range.low.y || m_nodeRange.high.y <= range.high.y)
        return  false;

    return !m_is3d ? true : (m_nodeRange.low.z < range.low.z && m_nodeRange.high.z > range.high.z);
    }

/*---------------------------------------------------------------------------------**//**
* An InternalNode has become full, split into two nodes (a new one and this one) and determine an optimal division of the current
* enteries between this node and the new one.
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnRangeTree::InternalNode::SplitInternalNode(DgnRangeTreeR root)
    {
    size_t  count = GetEntryCount();
    DRTSplitEntryP  splitEntries = (DRTSplitEntryP) _alloca(count * sizeof(DRTSplitEntry));

    DRTSplitEntryP currEntry = splitEntries;
    DRTSplitEntryP endEntry = splitEntries + count;

    for (DRTNodeH curr = &m_firstChild[0]; curr < m_endChild; ++curr, ++currEntry)
        {
        currEntry->m_entry = *curr;
        currEntry->m_range = (*curr)->GetRangeCR();
        }

    double xSep = checkSeparation(splitEntries, count, X_AXIS);
    double ySep = checkSeparation(splitEntries, count, Y_AXIS);
    double zSep = m_is3d ? checkSeparation(splitEntries, count, Z_AXIS) : 0;

    SplitAxis  optimalSplit = X_AXIS;
    if (ySep > xSep)
        optimalSplit = Y_AXIS;
    if (m_is3d && (zSep > ySep) && (zSep > xSep))
        optimalSplit = Z_AXIS;

    // allocate a new InternalNode to hold (approx) half or our entries. If parent is nullptr, this is the root of the tree and it has become full.
    // We need to add a new level to the tree. Move all of the current entries into a new node that will become a child of this node.
    DRTInternalNodeP newNode1 = root.AllocateInternalNode();
    DRTInternalNodeP newNode2 = (nullptr == m_parent) ? root.AllocateInternalNode() : this;

    ClearChildren();

    for (currEntry = splitEntries; currEntry < endEntry; ++currEntry)
        {
        if (0 == currEntry->m_groupNumber[optimalSplit])
            newNode1->AddInternalNode((DRTNodeP) currEntry->m_entry, root);
        else
            newNode2->AddInternalNode((DRTNodeP) currEntry->m_entry, root);
        }

    // now add the newly created node into the parent of this node. If parent is nullptr, we're the root of the tree and we've just added a new level
    // to the tree (so both nodes are new and become children of this node).
    if (nullptr == m_parent)
        {
        AddInternalNode(newNode1, root);
        AddInternalNode(newNode2, root);
        }
    else
        {
        m_parent->AddInternalNode(newNode1, root);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DRTNodeP DgnRangeTree::InternalNode::ChooseBestNode(DRange3dCP pRange, DgnRangeTreeR root)
    {
    DRTNodeP best = nullptr;
    double   bestFit = 0.0;
    bool     isValid = false;

    for (DRTNodeH curr = &m_firstChild[0]; curr < m_endChild; ++curr)
        {
        DRange3dCR thisRange = (*curr)->GetRange();
        DRange3d   newRange;

        newRange = thisRange;
        extendRange(newRange, *pRange);
        double newExtent = rangeExtentSquared(newRange);
        if (isValid && (bestFit < newExtent))
            continue;

        // "thisFit" is a somewhat arbitrary measure of how well the range fits into this
        // node, taking into account the total size ("new extent") of this node plus this range,
        // plus a penalty for increasing it from its existing size.
        double thisFit = newExtent + ((newExtent - rangeExtentSquared(thisRange)) * 10.0);

        if (!isValid || (thisFit < bestFit))
            {
            best    = *curr;
            bestFit = thisFit;
            isValid = true;
            }
        }

    BeAssert(nullptr != best);
    return  best;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnRangeTree::InternalNode::AddElement(Entry const& entry, DgnRangeTreeR root)
    {
    ValidateRange();
    extendRange(m_nodeRange, entry.m_range);
    DRTNodeP node = ChooseBestNode(&entry.m_range, root);

    DRTLeafNodeP leaf = node->ToLeaf();
    if (leaf)
        leaf->AddElementToLeaf(entry, root);
    else
        {
        ((DRTInternalNodeP) node)->AddElement(entry, root);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnRangeTree::InternalNode::DropRange(DRange3dCR range)
    {
    if (CompletelyContains(range))
        return;

    m_sloppy = true;
    if (m_parent)
        m_parent->DropRange(range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnRangeTree::InternalNode::DropNode(DRTNodeP entry, DgnRangeTreeR root)
    {
    for (DRTNodeH curr = &m_firstChild[0]; curr < m_endChild; ++curr)
        {
        if (*curr != entry)
            continue;

        if (curr+1 < m_endChild)
            memmove(curr, curr+1, (m_endChild - curr) * sizeof(DRTNodeP));

        --m_endChild;

        if (m_firstChild == m_endChild)  // last entry in this leaf?
            {
            if (nullptr == m_parent) // last node in the tree? If so, create an empty LeafNode and delete this InternalNode.
                root.m_root = root.AllocateLeafNode();
            else
                m_parent->DropNode(this, root);

            root.FreeInternalNode(this);
            return;
            }

        DropRange(entry->GetRangeCR());
        return;
        }

    BeAssert(false); // we were asked to drop a child we didn't hold
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnRangeTree::InternalNode::DropElement(Entry const& entry, DgnRangeTreeR root)
    {
    DRTLeafNodeP leaf = ToLeaf();
    if (leaf)
        return leaf->DropElementFromLeaf(entry, root);

    for (DRTNodeH curr = &m_firstChild[0]; curr < m_endChild; ++curr)
        {
        if (!(*curr)->Overlaps(entry.m_range))
            continue;

        DRTInternalNodeP internalNode=(DRTInternalNodeP) *curr;
        if (internalNode->DropElement(entry, root))
            return  true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnRangeTree::InternalNode::AddInternalNode(DRTNodeP child, DgnRangeTreeR root)
    {
    child->SetParent(this);
    ValidateInternalRange();
    extendRange(m_nodeRange, child->GetRange());

    *m_endChild++ = child;
    if (GetEntryCount() > (root.m_internalNodeSize))
        SplitInternalNode(root);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnRangeTree::InternalNode::GetLeafCount()
    {
    if (IsLeaf())
        return  1;

    size_t count = 0;
    for (DRTNodeH curr = &m_firstChild[0]; curr != m_endChild; ++curr)
        count += ((DRTInternalNodeP)*curr)->GetLeafCount();

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnRangeTree::InternalNode::GetNodeCount()
    {
    size_t count = 1;
    if (!IsLeaf())
        for (DRTNodeH curr = &m_firstChild[0]; curr != m_endChild; ++curr)
            count += ((DRTInternalNodeP)*curr)->GetNodeCount();

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnRangeTree::InternalNode::GetElementCount()
    {
    DRTLeafNodeP leaf = ToLeaf();
    if (nullptr != leaf)
        return  leaf->GetEntryCount();

    size_t count = 0;
    for (DRTNodeH curr = &m_firstChild[0]; curr != m_endChild; ++curr)
        count += ((DRTInternalNodeP)*curr)->GetElementCount();

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnRangeTree::InternalNode::GetMaxChildDepth()
    {
    if (IsLeaf())
        return  1;

    size_t childDepth, maxChildDepth = 0;
    for (DRTNodeH curr = &m_firstChild[0]; curr != m_endChild; ++curr)
        if ((childDepth = ((DRTInternalNodeP)*curr)->GetMaxChildDepth()) > maxChildDepth)
            maxChildDepth = childDepth;

    return 1 + maxChildDepth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnRangeTree::Match DgnRangeTree::InternalNode::Traverse(DgnRangeTree::Traverser& traverser, bool is3d)
    {
    if (traverser._CheckRangeTreeNode(GetRange(), is3d))
        {
        for (DRTNodeH curr = &m_firstChild[0]; curr < m_endChild; ++curr)
            {
            DgnRangeTree::Match status = (*curr)->Traverse(traverser, is3d);
            if (DgnRangeTree::Match::Ok != status)
                return  status;
            }
        }

    return  DgnRangeTree::Match::Ok;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnRangeTree::LeafNode::AddElementToLeaf(Entry const& entry, DgnRangeTreeR root)
    {
    ValidateRange();
    extendRange(m_nodeRange, entry.m_range);

    *m_endChild = entry;
    ++m_endChild;
    if (GetEntryCount() > (root.m_leafNodeSize))
        SplitLeafNode(root);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnRangeTree::LeafNode::DropElementFromLeaf(Entry const& entry, DgnRangeTreeR root)
    {
    for (Entry* curr = &m_firstChild[0]; curr < m_endChild; ++curr)
        {
        if (curr->m_elm != entry.m_elm)
            continue;

        if (curr+1 < m_endChild)
            memmove(curr, curr+1, (m_endChild - curr) * sizeof(Entry));

        --m_endChild;

        if (m_firstChild == m_endChild)  // last entry in this leaf?
            {
            if (nullptr == m_parent) // last node in the tree?
                {
                ClearChildren();
                return true;
                }

            m_parent->DropNode(this, root);
            root.FreeLeafNode(this);
            return true;
            }

        if (!CompletelyContains(entry.m_range))
            {
            m_sloppy = true;
            if (m_parent)
                m_parent->DropRange(entry.m_range);
            }
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnRangeTree::LeafNode::SplitLeafNode(DgnRangeTreeR root)
    {
    size_t  count = GetEntryCount();
    DRTSplitEntryP  splitEntries = (DRTSplitEntryP) _alloca(count * sizeof(DRTSplitEntry));

    DRTSplitEntryP currEntry = splitEntries;
    DRTSplitEntryP endEntry = splitEntries + count;

    for (Entry* curr = &m_firstChild[0]; curr < m_endChild; ++curr, ++currEntry)
        {
        currEntry->m_entry = (void*) curr->m_elm;
        currEntry->m_range = curr->m_range;
        }

    double xSep = checkSeparation(splitEntries, count, X_AXIS);
    double ySep = checkSeparation(splitEntries, count, Y_AXIS);
    double zSep = m_is3d ? checkSeparation(splitEntries, count, Z_AXIS) : 0;

    SplitAxis  optimalSplit = X_AXIS;
    if (ySep > xSep)
        optimalSplit = Y_AXIS;
    if (m_is3d && (zSep > ySep) && (zSep > xSep))
        optimalSplit = Z_AXIS;

    DRTLeafNodeP newNode1 = root.AllocateLeafNode();
    DRTLeafNodeP newNode2 = this;
    newNode2->m_type = newNode1->m_type;

    ClearChildren();    // clear range and child entries - about half of them will come back below.

    for (currEntry = splitEntries; currEntry < endEntry; ++currEntry)
        {
        if (0 == currEntry->m_groupNumber[optimalSplit])
            newNode1->AddElementToLeaf(Entry(currEntry->m_range, *(GeometricElementCP)currEntry->m_entry), root);
        else
            newNode2->AddElementToLeaf(Entry(currEntry->m_range, *(GeometricElementCP)currEntry->m_entry), root);
        }

    // if parent is nullptr, this node is currently the root of the tree (the only node in the tree). We need to allocate an InternalNode to
    // become the new root, and add both this node and the new node into it.
    if (nullptr == m_parent)
        {
        DRTInternalNodeP newRoot = root.AllocateInternalNode();
        newRoot->AddInternalNode(newNode1, root);
        newRoot->AddInternalNode(newNode2, root);
        root.m_root = newRoot;
        }
    else
        {
        m_parent->AddInternalNode(newNode1, root);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnRangeTree::Match DgnRangeTree::LeafNode::Traverse(DgnRangeTree::Traverser& traverser, bool is3d)
    {
    if (traverser._CheckRangeTreeNode(GetRange(), is3d))
        {
        for (Entry* curr = &m_firstChild[0]; curr < m_endChild; ++curr)
            {
            DgnRangeTree::Match stat = traverser._VisitRangeTreeElem(curr->m_elm, curr->m_range);
            if (DgnRangeTree::Match::Ok != stat)
                return  stat;
            }
        }

    return  DgnRangeTree::Match::Ok;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnRangeTree::DgnRangeTree(bool is3d, size_t leafSize)
    {
    m_root = nullptr;
    m_internalNodeSize = m_leafNodeSize = 0;
    m_elementsPerSecond = 0.0;
    m_is3d = is3d;

    if (0 >= leafSize || leafSize>20)
        leafSize = 20;

    SetNodeSizes(8, leafSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnRangeTree::LoadTree(DgnModelCR dgnModel)
    {
#ifdef DRT_DEBUGGING
    {
    s_statistics.m_create.m_modelCount++;
    INCLUDE_TIMER(s_statistics.m_create.m_createTime);
#endif

    BeAssert(nullptr == m_root);
    BeAssert(0 != m_internalNodeSize);
    BeAssert(0 != m_leafNodeSize);
    m_root = AllocateLeafNode();

    for (auto const& element : dgnModel.GetElements())
        {
        GeometricElementCP geom = element.second->ToGeometricElement();
        if (nullptr != geom)
            AddElement(Entry(geom->CalculateRange3d(), *geom));
        }

#ifdef DRT_DEBUGGING
    }
    s_statistics.m_create.m_leafNodeBytes   += m_leafNodes.GetMemoryAllocated();
    s_statistics.m_create.m_internalNodeBytes += m_internalNodes.GetMemoryAllocated();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnRangeTree::AddElement(Entry const& entry)
    {
    if (!rangeIsValid(entry.m_range, m_is3d))
        return;

    if (nullptr == m_root)
        m_root = AllocateLeafNode();
    
    DRTLeafNodeP leaf = m_root->ToLeaf();
    if (leaf)
        leaf->AddElementToLeaf(entry, *this);
    else
        ((DRTInternalNodeP)m_root)->AddElement(entry, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnRangeTree::RemoveElement(Entry const& entry)
    {
    if (nullptr == m_root)
        return ERROR;

    if (!((DRTInternalNodeP)m_root)->DropElement(entry, *this))
        {
        BeAssert(false);
        return  ERROR;
        }
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnRangeTree::SetNodeSizes(size_t internalNodeSize, size_t leafNodeSize)
    {
    m_internalNodeSize = internalNodeSize;
    m_leafNodeSize = leafNodeSize;

    m_leafNodes.SetEntrySize(sizeof(DgnRangeTree::LeafNode) + ((int) leafNodeSize*sizeof(Entry)), 1);
    m_internalNodes.SetEntrySize(sizeof(DgnRangeTree::InternalNode) + ((int) internalNodeSize*sizeof(DRTNodeP)), 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnRangeTree::Match DgnRangeTree::FindMatches(DgnRangeTree::Traverser& traverser)
    {
    return (nullptr == m_root) ? DgnRangeTree::Match::Ok : m_root->Traverse(traverser, Is3d());
    }

//=======================================================================================
// "Occlusion" order based traversal of the tree (front of view to back, most significant nodes first). Optionally, test for
// occulsion culling periodically.
//=======================================================================================
/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley      01/07
+===============+===============+===============+===============+===============+======*/
struct DRTViewNode
{
    DRTLeafNodeP    m_leaf;
    DgnModelP       m_modelRef;
    TransformP      m_localToWorld;
    double          m_score;
    bool            m_overlap;
    bool            m_spansEyePlane;

    bool    TestOcclusion() { return !m_spansEyePlane && m_score < s_prefs.m_maxOcclusionTestArea; }
    size_t  GetElementCount() { return m_leaf->GetEntryCount(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DRTViewNode(DRTLeafNodeP leaf, DgnModelP modelRef, TransformP localToWorld, double score, bool overlap, bool spansEyePlane)
    {
    m_leaf          = leaf;
    m_modelRef      = modelRef;
    m_localToWorld  = localToWorld;
    m_score         = score;
    m_overlap       = overlap;
    m_spansEyePlane = spansEyePlane;
    }
};

typedef bvector<DRTViewNode>    T_ViewNodes;
typedef T_ViewNodes::iterator   T_ViewNodeIterator;
typedef bvector<TransformP>     T_Transforms;
typedef bvector<DgnModelP>      T_DgnModels;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool OcclusionScorer::ComputeNPC(DPoint3dR npcOut, DPoint3dCR localIn)
    {
    double w;
    if (m_cameraOn)
        {
        w = m_localToNpc.coff[3][0] * localIn.x + m_localToNpc.coff[3][1] * localIn.y + m_localToNpc.coff[3][2] * localIn.z + m_localToNpc.coff[3][3];
        if (w < s_cameraLimit)
            return false;
        }
    else
        {
        w = 1.0;
        }

    npcOut.x = (m_localToNpc.coff[0][0] * localIn.x + m_localToNpc.coff[0][1] * localIn.y + m_localToNpc.coff[0][2] * localIn.z + m_localToNpc.coff[0][3]) / w;
    npcOut.y = (m_localToNpc.coff[1][0] * localIn.x + m_localToNpc.coff[1][1] * localIn.y + m_localToNpc.coff[1][2] * localIn.z + m_localToNpc.coff[1][3]) / w;
    npcOut.z = (m_localToNpc.coff[2][0] * localIn.x + m_localToNpc.coff[2][1] * localIn.y + m_localToNpc.coff[2][2] * localIn.z + m_localToNpc.coff[2][3]) / w;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
void OcclusionScorer::InitForViewport(DgnViewportCR viewport, double minimumSizePixels)
    {
    m_localToNpc = viewport.GetWorldToNpcMap()->M0;
    m_cameraOn   = viewport.IsCameraOn();

    m_lodFilterNPCArea = 0;

    if (minimumSizePixels > 0)
        {
        BSIRect  screenRect = viewport.GetClientRect();
        if (screenRect.Width() > 0 && screenRect.Height() > 0)
            {
            double width = minimumSizePixels/screenRect.Width();
            double height = minimumSizePixels/screenRect.Height();

            m_lodFilterNPCArea = width * width + height * height;
            }
        }

    if (m_cameraOn)
        {
        m_cameraPosition = viewport.GetCamera().GetEyePoint();
        }
    else
        {
        DPoint3d viewDirection[2] = {{0,0,0}, {0.0, 0.0, 1.0}};
        DPoint3d viewDirRoot[2];
        viewport.NpcToWorld(viewDirRoot, viewDirection, 2);
        viewDirRoot[1].Subtract (viewDirRoot[0]);

        m_orthogonalProjectionIndex = ((viewDirRoot[1].x < 0.0) ? 1  : 0) +
                                      ((viewDirRoot[1].x > 0.0) ? 2  : 0) +
                                      ((viewDirRoot[1].y < 0.0) ? 4  : 0) +
                                      ((viewDirRoot[1].y > 0.0) ? 8  : 0) +
                                      ((viewDirRoot[1].z < 0.0) ? 16 : 0) +
                                      ((viewDirRoot[1].z > 0.0) ? 32 : 0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Compute Occlusion score for a range that crosses the eye plane.
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool OcclusionScorer::ComputeEyeSpanningRangeOcclusionScore(double* score, DPoint3dCP rangeCorners, bool doFrustumCull)
    {
    bool    anyInside = false;
    double  s_eyeSpanningCameraLimit = 1.0E-3;

    DRange3d npcRange;
    npcRange.Init ();
    for (int i=0; i<8; i++)
        {
        DPoint3d  npc;
        npc.x = m_localToNpc.coff[0][0] * rangeCorners[i].x + m_localToNpc.coff[0][1] * rangeCorners[i].y + m_localToNpc.coff[0][2] * rangeCorners[i].z + m_localToNpc.coff[0][3];
        npc.y = m_localToNpc.coff[1][0] * rangeCorners[i].x + m_localToNpc.coff[1][1] * rangeCorners[i].y + m_localToNpc.coff[1][2] * rangeCorners[i].z + m_localToNpc.coff[1][3];

        double w = m_localToNpc.coff[3][0] * rangeCorners[i].x + m_localToNpc.coff[3][1] * rangeCorners[i].y + m_localToNpc.coff[3][2] * rangeCorners[i].z + m_localToNpc.coff[3][3];

        if (w < s_eyeSpanningCameraLimit)
            {
            w = s_eyeSpanningCameraLimit;
            }
        else
            {
            anyInside = true;
            }

        npc.x /= w;
        npc.y /= w;
        npc.z = 1.0;
        extendRange(npcRange, npc);
        }

    if (!anyInside)
        return false;

    if (doFrustumCull && (npcRange.high.x < 0.0 || npcRange.low.x > 1.0 || npcRange.high.y < 0.0 || npcRange.low.y > 1.0))
         return false;

    if (nullptr == score)
        return  true;

    if (npcRange.low.x < 0.0)
        npcRange.low.x = 0.0;

    if (npcRange.low.y < 0.0)
        npcRange.low.y = 0.0;

    if (npcRange.high.x > 1.0)
        npcRange.high.x = 1.0;

    if (npcRange.high.y > 1.0)
        npcRange.high.y = 1.0;

    *score = (npcRange.high.x - npcRange.low.x) * (npcRange.high.y - npcRange.low.y);      // Double score as the area calculation below doubles.

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Algorithm by: Dieter Schmalstieg and Erik Pojar - ACM Transactions on Graphics.
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool OcclusionScorer::ComputeOcclusionScore(double* score, bool& overlap, bool& spansEyePlane, bool& eliminatedByLOD, DPoint3dCP localCorners, bool doFrustumCull)
    {
    INCLUDE_TIMER(s_statistics.m_traverse.m_scoringTime);
    // Note - This routine is VERY time critical - Most of the calls to the geomlib
    // functions have been replaced with inline code as VTune had showed them as bottlenecks.

    static const short s_indexList[43][9] =
        {
        { 0, 3, 7, 6, 5, 1,   6,}, // 0 inside    (arbitrarily default to front, top, right.
        { 0, 4, 7, 3,-1,-1,   4}, // 1 left
        { 1, 2, 6, 5,-1,-1,   4}, // 2 right
        {-1,-1,-1,-1,-1,-1,   0}, // 3 -
        { 0, 1, 5, 4,-1,-1,   4}, // 4 bottom
        { 0, 1, 5, 4, 7, 3,   6}, // 5 bottom, left
        { 0, 1, 2, 6, 5, 4,   6}, // 6 bottom, right
        {-1,-1,-1,-1,-1,-1,   0}, // 7 -
        { 2, 3, 7, 6,-1,-1,   4}, // 8 top
        { 0, 4, 7, 6, 2, 3,   6}, // 9 top, left
        { 1, 2, 3, 7, 6, 5,   6}, //10 top, right
        {-1,-1,-1,-1,-1,-1,   0}, //11 -
        {-1,-1,-1,-1,-1,-1,   0}, //12 -
        {-1,-1,-1,-1,-1,-1,   0}, //13 -
        {-1,-1,-1,-1,-1,-1,   0}, //14 -
        {-1,-1,-1,-1,-1,-1,   0}, //15 -
        { 0, 3, 2, 1,-1,-1,   4}, //16 front
        { 0, 4, 7, 3, 2, 1,   6}, //17 front, left
        { 0, 3, 2, 6, 5, 1,   6}, //18 front, right
        {-1,-1,-1,-1,-1,-1,   0}, //19 -
        { 0, 3, 2, 1, 5, 4,   6}, //20 front, bottom
        { 1, 5, 4, 7, 3, 2,   6}, //21 front, bottom, left
        { 0, 3, 2, 6, 5, 4,   6}, //22 front, bottom, right
        {-1,-1,-1,-1,-1,-1,   0}, //23 -
        { 0, 3, 7, 6, 2, 1,   6}, //24 front, top
        { 0, 4, 7, 6, 2, 1,   6}, //25 front, top, left
        { 0, 3, 7, 6, 5, 1,   6}, //26 front, top, right
        {-1,-1,-1,-1,-1,-1,   0}, //27 -
        {-1,-1,-1,-1,-1,-1,   0}, //28 -
        {-1,-1,-1,-1,-1,-1,   0}, //29 -
        {-1,-1,-1,-1,-1,-1,   0}, //30 -
        {-1,-1,-1,-1,-1,-1,   0}, //31 -
        { 4, 5, 6, 7,-1,-1,   4}, //32 back
        { 0, 4, 5, 6, 7, 3,   6}, //33 back, left
        { 1, 2, 6, 7, 4, 5,   6}, //34 back, right
        {-1,-1,-1,-1,-1,-1,   0}, //35 -
        { 0, 1, 5, 6, 7, 4,   6}, //36 back, bottom
        { 0, 1, 5, 6, 7, 3,   6}, //37 back, bottom, left
        { 0, 1, 2, 6, 7, 4,   6}, //38 back, bottom, right
        {-1,-1,-1,-1,-1,-1,   0}, //39 -
        { 2, 3, 7, 4, 5, 6,   6}, //40 back, top
        { 0, 4, 5, 6, 2, 3,   6}, //41 back, top, left
        { 1, 2, 3, 7, 4, 5,   6}, //42 back, top, right
        };

    enum
        {
        OUTSIDE_Left   = (0x00001 << 0),
        OUTSIDE_Right  = (0x00001 << 1),
        OUTSIDE_Top    = (0x00001 << 2),
        OUTSIDE_Bottom = (0x00001 << 3),
        OUTSIDE_Front  = (0x00001 << 4),
        OUTSIDE_Back   = (0x00001 << 5),
        };

    eliminatedByLOD = false;
    if (!doFrustumCull && 0.0 == m_lodFilterNPCArea && nullptr == score)
        return true;

    uint32_t npcComputedMask = 0, zComputedMask = 0;
    uint32_t projectionIndex;

    spansEyePlane = false;
    if (m_cameraOn)
        {
        projectionIndex = ((m_cameraPosition.x < localCorners[0].x) ? 1  : 0) +
                          ((m_cameraPosition.x > localCorners[1].x) ? 2  : 0) +
                          ((m_cameraPosition.y < localCorners[0].y) ? 4  : 0) +
                          ((m_cameraPosition.y > localCorners[2].y) ? 8  : 0) +
                          ((m_cameraPosition.z < localCorners[0].z) ? 16 : 0) +       // Zs reversed for right-handed system.
                          ((m_cameraPosition.z > localCorners[4].z) ? 32 : 0);

        if (0 == projectionIndex)
            {
            if (nullptr != score)
                *score = 1.0;

            overlap = spansEyePlane = true;
            return true;
            }
        }
    else
        {
        projectionIndex = m_orthogonalProjectionIndex;
        }

    uint32_t    nVertices;
    double      npcZ[8];
    DPoint3d    npcVertices[6];

    BeAssert(projectionIndex <= 42);
    if (projectionIndex > 42 || 0 == (nVertices = s_indexList[projectionIndex][6]))
        {
        // Inside the box... Needs work.
        return false;
        }

    double  zTotal = 0.0;

    // Note - Don't attempt to cull the front.back plane. - We can't do that as we're only looking at the frontmost vertices.
    uint32_t outsideMask = OUTSIDE_Left | OUTSIDE_Right | OUTSIDE_Top | OUTSIDE_Bottom;

    overlap = false;
    for (uint32_t i=0, mask = 0x0001; i<nVertices; i++, mask <<= 1)
        {
        int  cornerIndex = s_indexList[projectionIndex][i];
        if (0 == (mask & npcComputedMask) && !ComputeNPC(npcVertices[i], localCorners[cornerIndex]))
            {
            spansEyePlane = overlap = true;
            return ComputeEyeSpanningRangeOcclusionScore(score, localCorners, doFrustumCull);
            }

        if (doFrustumCull)
            {
            if (npcVertices[i].x < 0.0)
                {
                overlap = true;
                outsideMask &= ~OUTSIDE_Right;
                npcVertices[i].x = 0.0;
                }
            else
                {
                outsideMask &= ~OUTSIDE_Left;
                if (npcVertices[i].x > 1.0)
                    {
                    overlap = true;
                    npcVertices[i].x = 1.0;
                    }
                else
                    outsideMask &= ~OUTSIDE_Right;
                }

            if (npcVertices[i].y < 0.0)
                {
                overlap = true;
                outsideMask &= ~OUTSIDE_Top;
                npcVertices[i].y = 0.0;
                }
            else
                {
                outsideMask &= ~OUTSIDE_Bottom;
                if (npcVertices[i].y > 1.0)
                    {
                    overlap = true;
                    npcVertices[i].y = 1.0;
                    }
                else
                    outsideMask &= ~OUTSIDE_Top;
                }
            }

        zTotal += (npcZ[cornerIndex] = npcVertices[i].z);
        zComputedMask |= (1 << cornerIndex);
        }

    if (doFrustumCull)
        {
        if (outsideMask)           // If all off in any of the X-Y directions return now.
            return false;

        // We need to cull front and back seperately as we have only looked at the front most vertices so far and front/back requires checking all 8 corners.

        outsideMask = OUTSIDE_Back | OUTSIDE_Front;
        for (uint32_t i=0, mask = 0x0001; i<8; i++, mask <<= 1)
            {
            if (0 == (zComputedMask & mask))
                {
                npcZ[i] = m_localToNpc.coff[2][0] * localCorners[i].x + m_localToNpc.coff[2][1] * localCorners[i].y + m_localToNpc.coff[2][2] * localCorners[i].z + m_localToNpc.coff[2][3];

                if (m_cameraOn)
                    npcZ[i] /= (m_localToNpc.coff[3][0] * localCorners[i].x + m_localToNpc.coff[3][1] * localCorners[i].y + m_localToNpc.coff[3][2] * localCorners[i].z + m_localToNpc.coff[3][3]);
                 }

            if (npcZ[i] < 0.0)
                {
                overlap = true;
                outsideMask &= ~OUTSIDE_Back;
                }
            else
                {
                outsideMask &= ~OUTSIDE_Front;
                if (npcZ[i] > 1.0)
                    overlap = true;
                else
                    outsideMask &= ~OUTSIDE_Back;
                }
            }
        if (outsideMask)                // Off in front or Back.
            return false;
        }

    if (0.0 != m_lodFilterNPCArea)
        {
        //  In the cases where this does exclude the element it would be faster to do the other filtering first.  However, even when we exclude something due
        //  to LOD filtering we want to know if it should be drawn by the progressive display.  We want progressive display to draw zero-length line strings
        //  and points.
        int        diagonalVertex = nVertices/2;
        DPoint3dR  diagonalNPC    = npcVertices[diagonalVertex];
        bool       lodFilterOnly  = nullptr == score && !doFrustumCull;

        if (!ComputeNPC(npcVertices[0], localCorners[s_indexList[projectionIndex][0]]) ||
            !ComputeNPC(diagonalNPC,    localCorners[s_indexList[projectionIndex][diagonalVertex]]))
            {
            spansEyePlane = overlap = true;
            return lodFilterOnly ? true : ComputeEyeSpanningRangeOcclusionScore(score, localCorners, doFrustumCull);
            }

        DPoint3dR   npcCorner = npcVertices[nVertices/2];
        DPoint2d    extent = { npcCorner.x - npcVertices[0].x, npcCorner.y - npcVertices[0].y};

        if (extent.x * extent.x + extent.y * extent.y < m_lodFilterNPCArea)
            {
            eliminatedByLOD = true;
#ifdef DRT_DEBUGGING
            s_statistics.m_traverse.m_lodFilteredElementCount++;
#endif
            return false;
            }

        if (lodFilterOnly)
            return true;

        npcComputedMask |= 1;
        npcComputedMask |= (1 << diagonalVertex);
        }

    if (nullptr == score)
        return true;

    *score = (npcVertices[nVertices-1].x - npcVertices[0].x) * (npcVertices[nVertices-1].y + npcVertices[0].y);
    for (uint32_t i=0; i<nVertices-1; i++)
        *score += (npcVertices[i].x - npcVertices[i+1].x) * (npcVertices[i].y + npcVertices[i+1].y);

    // an area of 0.0 means that we have a line. Recalculate using length/1000 (assuming width of 1000th of npc)
    if (*score == 0.0)
        *score = npcVertices[0].DistanceXY(npcVertices[2]) * .001;
    else if (*score < 0.0)
        *score = - *score;

    // Multiply area by the Z total value (0 is the back of the view) so that the score is roughly
    // equivalent to the area swept by the range through the view which makes things closer to the eye more likely to display first.
    // Also by scoring based on swept volume we are proportional to occluded volume.
    *score *= .5 * (zTotal / (double) nVertices);

    return true;
    }

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley      09/09
+===============+===============+===============+===============+===============+======*/
struct OcclusionSortedProcessor : OcclusionScorer
{
    double                          m_frameTime;
    double                          m_minLODArea;
    ViewContextR                    m_viewContext;
    ScanCriteriaCP                  m_scanCriteria;
    DgnViewportP                    m_viewport;
    DgnRangeTree::ProgressMonitor*  m_progressMonitor;
    uint32_t                        m_visitElementCount;
    DgnModelP                       m_currDgnModel;
    DgnModelP                       m_rootModel;
    uint32_t                        m_occlusionTestCount;
    DgnMemoryPool<Transform,128>    m_localToWorldTrans;
    TransformP                      m_localToWorld;
    T_ViewNodes                     m_viewNodes;
    size_t                          m_viewNodesProcessed;
    uint32_t                        m_targetElementCount;
    bool                            m_isDynamicUpdate;
    bool                            m_doOcclusionCull;
    bool                            m_doFrustumCull;

    uint32_t GetVisitElementCount() {return m_visitElementCount;}
    static bool CompareOcclusionScore(DRTViewNode const& lhs, DRTViewNode const& rhs) { return lhs.m_score > rhs.m_score; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
OcclusionSortedProcessor(ViewContextR viewContext, DgnModelP startModel, bool doFrustumCull, uint32_t targetElementCount, ProgressMonitorP monitor=nullptr)
    : m_viewContext(viewContext)
    {
    m_doFrustumCull      = doFrustumCull;
    m_progressMonitor    = monitor;
    m_targetElementCount = targetElementCount;
    m_cameraOn           = viewContext.IsCameraOn();
    m_viewport           = viewContext.GetViewport();
    m_scanCriteria       = viewContext.GetScanCriteria();
    m_rootModel          = startModel;
    m_visitElementCount  = 0;
    m_currDgnModel       = nullptr;
    m_isDynamicUpdate    = viewContext.GetDrawPurpose() == DrawPurpose::UpdateDynamic;
    m_doOcclusionCull    = s_prefs.m_doOcclusionCull &&  !m_isDynamicUpdate;
    m_occlusionTestCount = 0;
    m_lodFilterNPCArea   = 0.0;
    m_viewNodesProcessed = 0;

    m_viewNodes.reserve((size_t)(s_prefs.m_targetLeafCount * 1.2));

    if (s_prefs.m_doLodFiltering &&
        (0.0 != s_prefs.m_testLOD || FILTER_LOD_ShowNothing == viewContext.GetFilterLODFlag()) &&
        nullptr != m_viewport)
        {
        //double minLOD = 0.0 == s_prefs.m_testLOD ? viewContext.GetMinLOD() : s_prefs.m_testLOD;

        BSIRect  screenRect = m_viewport->GetClientRect();
        m_lodFilterNPCArea = MAX(s_prefs.m_minLODFilterNPC, s_prefs.m_lodFilterFraction * viewContext.GetMinLOD() /(double) screenRect.Area());

#ifdef DRT_DEBUGGING
        s_statistics.m_traverse.m_lodArea = m_lodFilterNPCArea;
#endif
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void InitForDgnModel()
    {
    m_localToNpc = m_viewContext.GetWorldToNpc().M0;

    // Use viewport camera position.
    m_cameraPosition = m_viewport->GetCamera().GetEyePoint();

    Transform localToWorld;

    if (SUCCESS == m_viewContext.GetCurrLocalToWorldTrans(localToWorld))
        {
        m_localToWorld = m_localToWorldTrans.AllocateNode();
        *m_localToWorld = localToWorld;

        m_localToNpc.InitProduct(m_localToNpc, DMatrix4d::From(*m_localToWorld));
        }
    else
        {
        m_localToWorld = nullptr;
        }

    if (m_cameraOn)
        {
        if (m_localToWorld)
            {
            Transform  worldToLocal;

            worldToLocal.InverseOf(*m_localToWorld);
            worldToLocal.Multiply(m_cameraPosition);
            }
        }
    else
        {
        DPoint4d viewDirection = {0.0, 0.0, -1.0, 0.0};

        DMatrix4d  viewToLocal = m_viewContext.GetViewToLocal();
        viewToLocal.Multiply (&viewDirection, &viewDirection, 1);
        m_orthogonalProjectionIndex = ((viewDirection.x < 0.0) ? 1  : 0) +
                                      ((viewDirection.x > 0.0) ? 2  : 0) +
                                      ((viewDirection.y < 0.0) ? 4  : 0) +
                                      ((viewDirection.y > 0.0) ? 8  : 0) +
                                      ((viewDirection.z < 0.0) ? 16 : 0) +
                                      ((viewDirection.z > 0.0) ? 32 : 0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool WasAborted()
    {
    if (m_viewContext.WasAborted())
        return true;

    if (0 != m_targetElementCount && m_visitElementCount > m_targetElementCount)
        {
        m_viewContext.CheckStop();
        return  true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool VisitRangeElement(GeometricElementCP element, DgnModelP modelRef, bool testRange, double score)
    {
    m_viewContext.ValidateScanRange();

    DRange3dCR   elRange = element->CalculateRange3d();
    if (testRange && (ScanCriteria::Result::Pass != m_scanCriteria->CheckRange(elRange, true)))
        return false;

    if (ScanCriteria::Result::Pass != m_scanCriteria->CheckElement(*element, false))
        return false;

    if (ClipPlaneContainment_StronglyOutside != m_viewContext.GetTransformClipStack().ClassifyRange(elRange, true))
        {
#ifdef DRT_DEBUGGING
        s_statistics.m_traverse.m_elementVisitCount++;
        double      elementTime = 0.0;
        BEGIN_DELTA_TIMER(elementTime);
#endif
        m_viewContext.VisitElement(*element);

#ifdef DRT_DEBUGGING
        END_DELTA_TIMER(elementTime);
        s_statistics.m_traverse.m_elementTime += elementTime;
        s_statistics.m_traverse.m_maxElementTime = MAX(s_statistics.m_traverse.m_maxElementTime, elementTime);
#endif
        m_visitElementCount++;
        }

    return WasAborted();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessViewNode(DRTViewNode& viewNode)
    {
#ifdef DRT_DEBUGGING
    s_statistics.m_traverse.m_leafVisitCount++;
#endif

    bool testRange = viewNode.m_overlap;
    for (DgnRangeTree::Entry* curr = &viewNode.m_leaf->m_firstChild[0]; curr < viewNode.m_leaf->m_endChild; ++curr)
        {
        if (VisitRangeElement(curr->m_elm, viewNode.m_modelRef, testRange, viewNode.m_score))
            return  true;
        }

    if (nullptr != m_progressMonitor && m_progressMonitor->_MonitorProgress((double) m_viewNodesProcessed++ / (double) m_viewNodes.size()))
        return true;

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessViewNodesBatch(T_ViewNodeIterator begin, T_ViewNodeIterator end, bool testOcclusion)
    {
    if (begin == end)
        return false;

    int                 results[MAX_OcclusionBatch];
    uint32_t            nNodes;
    DRTRangeCorners     corners[MAX_OcclusionBatch];
    T_ViewNodeIterator  curr;

    if (testOcclusion)
        {
        for (nNodes = 0, curr = begin; curr != end; ++curr)
            {
            if (curr->TestOcclusion())
                {
                corners[nNodes].InitFromRange(curr->m_leaf->GetRange(), curr->m_localToWorld);

#ifdef DRT_DEBUGGING
                s_statistics.m_traverse.m_elementsOcclusionTested += curr->GetElementCount();
#endif
                nNodes++;
                }

            if (WasAborted())
                return true;
            }

        if (nNodes > s_prefs.m_minimumOcclusionNodeTest)
            {
#ifdef DRT_DEBUGGING
            s_statistics.m_traverse.m_leavesOcclusionTested += nNodes;
            s_statistics.m_traverse.m_occlusionCalls++;
            double   occlusionTime = 0.0;
            BEGIN_NET_TIMER(occlusionTime);
#endif
            if (SUCCESS != m_viewContext.GetIViewDraw().TestOcclusion(nNodes, corners[0].m_points, results))
                testOcclusion = false;

#ifdef DRT_DEBUGGING
            END_NET_TIMER(occlusionTime);
            s_statistics.m_traverse.m_maxOcclusionTime = MAX(s_statistics.m_traverse.m_maxOcclusionTime, occlusionTime);
            s_statistics.m_traverse.m_occlusionTime  += occlusionTime;
#endif
            }
        else
            {
            testOcclusion = false;
            }
        }

    int* pResult = results;
    for (curr = begin; curr != end; ++curr)
        {
        int    minPixels = (int) (s_prefs.m_minimumOcclusionPixelRatio * curr->m_score);
        static const int  s_minOcclusionPixels =3;

        if (minPixels < s_minOcclusionPixels)
            minPixels = s_minOcclusionPixels;

        if (!testOcclusion || !curr->TestOcclusion() || *pResult++ > minPixels)
            {
            if (ProcessViewNode(*curr))
                return  true;
            }
#ifdef DRT_DEBUGGING
        else
            {
            s_statistics.m_traverse.m_leavesOccluded++;
            s_statistics.m_traverse.m_elementsOccluded += curr->GetElementCount();
            }
#endif

        if (WasAborted())
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void FindVisibleLeafs(DRTNodeR node, DgnModelR modelRef, bool testRange)
    {
    DRange3dCR nodeRange = node.GetRange();
    if (testRange)
        {
        if (ScanCriteria::Result::Pass != m_scanCriteria->CheckRange(nodeRange, true) ||
            ClipPlaneContainment_StronglyOutside == m_viewContext.GetTransformClipStack().ClassifyRange(nodeRange, true))
            return;
        }

    DPoint3d localCorners[8];
    localCorners[0].x = localCorners[3].x = localCorners[4].x = localCorners[7].x = (double) nodeRange.low.x;      //       7+------+6
    localCorners[1].x = localCorners[2].x = localCorners[5].x = localCorners[6].x = (double) nodeRange.high.x;     //       /|     /|
                                                                                                                     //      / |    / |
    localCorners[0].y = localCorners[1].y = localCorners[4].y = localCorners[5].y = (double) nodeRange.low.y;      //     / 4+---/--+5
    localCorners[2].y = localCorners[3].y = localCorners[6].y = localCorners[7].y = (double) nodeRange.high.y;     //   3+------+2 /    y   z
                                                                                                                     //    | /    | /     |  /
    localCorners[0].z = localCorners[1].z = localCorners[2].z = localCorners[3].z = (double) nodeRange.low.z;      //    |/     |/      |/
    localCorners[4].z = localCorners[5].z = localCorners[6].z = localCorners[7].z = (double) nodeRange.high.z;     //   0+------+1      *---x

    bool overlap, spansEye;
    if (nullptr != node.ToLeaf())
        {
        double  score;
        bool eliminatedByLOD;
        if (!ComputeOcclusionScore(&score, overlap, spansEye, eliminatedByLOD, localCorners, testRange))
            {
#ifdef DRT_DEBUGGING
            s_statistics.m_traverse.m_lodFilteredNodeCount++;
#endif
            return;
            }

        m_viewNodes.push_back(DRTViewNode((DRTLeafNodeP) &node, &modelRef, m_localToWorld, score, testRange && overlap, spansEye));
        }
    else
        {
        DRTInternalNodeP internalNode = (DRTInternalNodeP) &node;
        bool eliminatedByLOD;
        if (!ComputeOcclusionScore(nullptr, overlap, spansEye, eliminatedByLOD, localCorners, testRange))
            {
#ifdef DRT_DEBUGGING
            s_statistics.m_traverse.m_lodFilteredNodeCount += internalNode->GetLeafCount();
#endif
            return;
            }

        for (DRTNodeH curr = &internalNode->m_firstChild[0]; curr != internalNode->m_endChild; ++curr)
            FindVisibleLeafs(**curr, modelRef, testRange && overlap);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void FindAllVisibleLeafs(DgnModelR model)
    {
    DgnRangeTreeP tree = model.GetRangeIndexP(true);
    if (nullptr == tree)
        return;

    InitForDgnModel();
    m_viewContext.ValidateScanRange();

    FindVisibleLeafs(*tree->GetRoot(), model, m_doFrustumCull);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void ProcessEntries()
    {
    BEGIN_NET_TIMER(s_statistics.m_traverse.m_gatherTime);
    FindAllVisibleLeafs(*m_rootModel);
    END_NET_TIMER(s_statistics.m_traverse.m_gatherTime);

    BEGIN_NET_TIMER(s_statistics.m_traverse.m_sortTime);
    std::sort(m_viewNodes.begin(), m_viewNodes.end(), CompareOcclusionScore);
    END_NET_TIMER(s_statistics.m_traverse.m_sortTime);

#ifdef DRT_DEBUGGING
    s_statistics.m_traverse.m_elementTargetCount = m_targetElementCount;
    s_statistics.m_traverse.m_leavesGathered = m_viewNodes.size();
#endif

    static int s_debugToNode = 0;
    if (s_debugToNode > 0)
        {
        size_t i=0;
        for (T_ViewNodeIterator  curr = m_viewNodes.begin(), end = m_viewNodes.end(); curr != end && i <= (uint32_t) s_debugToNode; ++curr, i++)
            if (ProcessViewNode(*curr))
                return;
        }
    else if (m_isDynamicUpdate)
        {
        for (T_ViewNodeIterator  curr = m_viewNodes.begin(), end = m_viewNodes.end(); curr != end; ++curr)
            {
            if (ProcessViewNode(*curr))
                return;
            }
        }
    else
        {
        size_t                  nNodes, batchSize = MIN (MAX_OcclusionBatch, (m_viewNodes.size() / s_prefs.m_nOcclusionBatches));
        T_ViewNodeIterator      beginBatch, curr, end, next;
        bool                    doTestOcclusion = false;      // Don't turn this on until we have first batch complete (or there is nothing to occlude).
        for (nNodes = 0, beginBatch = curr = m_viewNodes.begin(), end = m_viewNodes.end(); curr != end; curr = next)
            {
            next = curr + 1;

            if (++nNodes >= batchSize || next == end)
                {
                if (ProcessViewNodesBatch(beginBatch, next, doTestOcclusion))
                    return;

                beginBatch = next;
                nNodes = 0;

                if (m_doOcclusionCull)
                    doTestOcclusion = true;
                }
            }
        }
    }

}; // OcclusionSortedProcessor

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnRangeTree::ProcessOcclusionSorted(ViewContextR context, DgnModelP startDgnModel, DgnRangeTree::ProgressMonitor* monitor, bool doFrustumCull, uint32_t* timeout)
    {
    uint32_t targetElementCount = 0;
    bool   doTimeout = nullptr != timeout && context.GetDrawPurpose() == DrawPurpose::UpdateDynamic;

    if (doTimeout && 0 != m_elementsPerSecond)
        {
        double timeOutSeconds = *timeout - BeTimeUtilities::QuerySecondsCounter();
        targetElementCount = (uint32_t) (timeOutSeconds * m_elementsPerSecond);
        }

#ifdef DRT_DEBUGGING
    END_NET_TIMER(s_statistics.m_deltaTime);
    s_statistics.ClearCreate();
    s_statistics.ClearTraverse();
    BEGIN_NET_TIMER(s_statistics.m_traverse.m_totalTime);
#endif
#ifdef KAB_DEBUG
    toolSubsystem_printf("OSP to=%d, eps=%f, targ=%d\n", timeout, m_elementsPerSecond, targetElementCount);
#endif

    double beginTime = timeGetSeconds();

    OcclusionSortedProcessor processor(context, startDgnModel, doFrustumCull, targetElementCount, monitor);
    processor.ProcessEntries();

#ifdef DRT_DEBUGGING
    END_NET_TIMER(s_statistics.m_traverse.m_totalTime);
    s_statistics.DumpTraverse();
    s_statistics.DumpCreate(*this);
#endif

    double elapsedTime = timeGetSeconds() - beginTime;
#ifdef KAB_DEBUG
    toolSubsystem_printf(" elapsed=%f, targ=%f, visited=%d\n", elapsedTime, timeOutSeconds, processor.GetVisitElementCount());
#endif

    if (0.0 < elapsedTime)
        {
        double thisElementsPerSecond = processor.GetVisitElementCount() / elapsedTime;
        if (0 == m_elementsPerSecond)
            m_elementsPerSecond = thisElementsPerSecond;
        else
            m_elementsPerSecond = ((m_elementsPerSecond*9.0) + thisElementsPerSecond) / 10.0;
        }

#ifdef DRT_DEBUGGING
    s_statistics.m_deltaTime = 0.0;
    BEGIN_NET_TIMER(s_statistics.m_deltaTime);
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2012
//--------------+------------------------------------------------------------------------
double DgnDbRTree3dViewFilter::MaxOcclusionScore() {return DBL_MAX;}
void OverlapScorer::Initialize(DRange3dCR boundingRange) {m_boundingRange.FromRange(boundingRange);}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
bool OverlapScorer::ComputeScore(double* score, BeSQLite::RTree3dValCR testRange)
    {
    RTree3dVal  intersection;
    bool intersects = intersection.Intersection(m_boundingRange, testRange);

    BeAssert(intersects);

    *score = rangeExtentSquared(intersection);
    return intersects;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void DgnDbRTree3dViewFilter::InitializeSecondaryTest(DRange3dCR volume, uint32_t hitLimit)
    {
    m_useSecondary = true;
    m_secondaryFilter.m_hitLimit = hitLimit;
    m_secondaryFilter.m_occlusionMapCount = 0;
    m_secondaryFilter.m_occlusionMapMinimum = DBL_MAX;
    m_secondaryFilter.m_scorer.Initialize(volume);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
RtreeViewFilter::RtreeViewFilter(DgnViewportCR viewport, DbR db, double minimumSizePixels, DgnElementIdSet const* exclude)
        : Tester(db), m_minimumSizePixels(minimumSizePixels), m_exclude(exclude), m_clips(nullptr)
    {
    m_nCalls = m_nScores = m_nSkipped = 0;
    m_scorer.InitForViewport(viewport, m_minimumSizePixels);
    m_frustum  = viewport.GetFrustum(DgnCoordSystem::World, true);

    DRange3d range = m_frustum.ToRange();
    m_boundingRange.FromRange(range);

    // get bounding range of front plane of polyhedron
    range.InitFrom(m_frustum.GetPts(), 4);
    m_frontFaceRange.FromRange(range);

    // get unit bvector from front plane to back plane
    m_viewVec = DVec3d::FromStartEndNormalize(*(m_frustum.GetPts()+4), *m_frustum.GetPts());

    // check to see if it's worthwhile using skew scan (skew vector not along one of the three major axes)
    int alongAxes = (fabs(m_viewVec.x) < 1e-8);
    alongAxes += (fabs(m_viewVec.y) < 1e-8);
    alongAxes += (fabs(m_viewVec.z) < 1e-8);
    m_doSkewtest = alongAxes<2;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbRTree3dViewFilter::DgnDbRTree3dViewFilter(DgnViewportCR viewport, ICheckStopP checkStop, DbR db, uint32_t hitLimit, double minimumSizePixels,
                            DgnElementIdSet const* alwaysDraw, DgnElementIdSet const* exclude)
    : RtreeViewFilter(viewport, db, minimumSizePixels, exclude), m_checkStop(checkStop), m_useSecondary(false), m_alwaysDraw(nullptr)
    {
    m_eliminatedByLOD = false;
    m_hitLimit = hitLimit;
    m_occlusionMapMinimum = 1.0e20;
    m_occlusionMapCount = 0;

    m_secondaryFilter.m_hitLimit = hitLimit;
    m_secondaryFilter.m_occlusionMapCount = 0;
    m_secondaryFilter.m_occlusionMapMinimum = DBL_MAX;

    if (nullptr != alwaysDraw)
        {
        m_lastScore = MaxOcclusionScore();
        for (auto const& id : *alwaysDraw)
            {
            if (nullptr != m_exclude && m_exclude->find(id) != m_exclude->end())
                continue;

            m_passedPrimaryTest = true;
            m_passedSecondaryTest = false;
            RangeAccept(id.GetValueUnchecked());
            }
        }

    //  We do this as the last step. Otherwise, the calls to _RangeAccept in the previous step would not have any effect.
    m_alwaysDraw = alwaysDraw;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BJB             12/89
+---------------+---------------+---------------+---------------+---------------+------*/
static inline void exchangeAndNegate(double& dbl1, double& dbl2)
    {
    double temp = -dbl1;
    dbl1 = -dbl2;
    dbl2 = temp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool RtreeViewFilter::SkewTest(RTree3dValCP pt)
    {
    if (!m_doSkewtest || pt->Intersects(m_frontFaceRange))
        return  true;

    DVec3d skVector = m_viewVec;
    DPoint3d dlo;
    DPoint3d dhi;

    dlo.x = pt->m_minx - m_frontFaceRange.m_maxx;
    dlo.y = pt->m_miny - m_frontFaceRange.m_maxy;
    dhi.x = pt->m_maxx - m_frontFaceRange.m_minx;
    dhi.y = pt->m_maxy - m_frontFaceRange.m_miny;

    if (skVector.x < 0.0)
        {
        skVector.x = - skVector.x;
        exchangeAndNegate(dlo.x, dhi.x);
        }

    if (skVector.y < 0.0)
        {
        skVector.y = - skVector.y;
        exchangeAndNegate(dlo.y, dhi.y);
        }

    // Check the projection of the element's xhigh to the plane where ylow of the element is equal to yhigh of the skewrange
    double va1 = dlo.x * skVector.y;
    double vb2 = dhi.y * skVector.x;
    if (va1 > vb2)
        return false;

    // Check the projection of the element's xlow to the plane where yhigh of the element is equal to ylow of the skewrange
    double vb1 = dlo.y * skVector.x;
    double va2 = dhi.x * skVector.y;
    if (va2 < vb1)
        return false;

    // now we need the Z stuff
    dlo.z = pt->m_minz - m_frontFaceRange.m_maxz;
    dhi.z = pt->m_maxz - m_frontFaceRange.m_minz;

    if (skVector.z < 0.0)
        {
        skVector.z = - skVector.z;
        exchangeAndNegate(dlo.z, dhi.z);
        }

    // project onto either the xz or yz plane
    if (va1 > vb1)
        {
        double va3 = dlo.x * skVector.z;
        double vc2 = dhi.z * skVector.x;
        if (va3 > vc2)
            return false;
        }
    else
        {
        double vb3 = dlo.y * skVector.z;
        double vc4 = dhi.z * skVector.y;
        if (vb3 > vc4)
            return false;
        }

    // project onto the other plane
    if (va2 < vb2)
        {
        double va4 = dhi.x * skVector.z;
        double vc1 = dlo.z * skVector.x;
        if (va4 < vc1)
            return false;
        }
    else
        {
        double vb4 = dhi.y * skVector.z;
        double vc3 = dlo.z * skVector.y;
        if (vb4 < vc3)
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbRTree3dViewFilter::RangeAccept(int64_t elementId)
    {
    BeAssert(m_lastId == elementId);
    
    if (nullptr != m_exclude && m_exclude->find(DgnElementId(elementId)) != m_exclude->end())
        return;

    if (m_passedPrimaryTest)
        {
        //  Don't add it if the constructor already added it.
        if (nullptr == m_alwaysDraw || m_alwaysDraw->find(DgnElementId(elementId)) == m_alwaysDraw->end())
            {
            ++m_nCalls;

            if (m_occlusionMapCount >= m_hitLimit)
                m_occlusionScoreMap.erase(m_occlusionScoreMap.begin());
            else
                m_occlusionMapCount++;

            m_occlusionScoreMap.Insert(m_lastScore, elementId);
            m_occlusionMapMinimum = m_occlusionScoreMap.begin()->first;
            }
        }

    if (m_passedSecondaryTest)
        {
        if (m_secondaryFilter.m_occlusionMapCount >= m_secondaryFilter.m_hitLimit)
            m_secondaryFilter.m_occlusionScoreMap.erase(m_secondaryFilter.m_occlusionScoreMap.begin());
        else
            m_secondaryFilter.m_occlusionMapCount++;

        m_secondaryFilter.m_occlusionScoreMap.Insert(m_secondaryFilter.m_lastScore, elementId);
        m_secondaryFilter.m_occlusionMapMinimum = m_secondaryFilter.m_occlusionScoreMap.begin()->first;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2013
//---------------------------------------------------------------------------------------
bool RtreeViewFilter::AllPointsClippedByOnePlane(ConvexClipPlaneSetCR cps, size_t nPoints, DPoint3dCP points) const
    {
    for (auto const& plane : cps)
        {
        int nOutside = 0;
        for (size_t j = 0; j < nPoints; j++)
            {
            if (plane.EvaluatePoint(points[j]) < 0.0)
                nOutside++;
            }

        if (nOutside == nPoints)
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
inline static void toLocalCorners(DPoint3dP localCorners, RTree3dValCP pt)
    {
    localCorners[0].x = localCorners[3].x = localCorners[4].x = localCorners[7].x = pt->m_minx;     //       7+------+6
    localCorners[1].x = localCorners[2].x = localCorners[5].x = localCorners[6].x = pt->m_maxx;     //       /|     /|
                                                                                                    //      / |    / |
    localCorners[0].y = localCorners[1].y = localCorners[4].y = localCorners[5].y = pt->m_miny;     //     / 4+---/--+5
    localCorners[2].y = localCorners[3].y = localCorners[6].y = localCorners[7].y = pt->m_maxy;     //   3+------+2 /    y   z
                                                                                                    //    | /    | /     |  /
    localCorners[0].z = localCorners[1].z = localCorners[2].z = localCorners[3].z = pt->m_minz;     //    |/     |/      |/
    localCorners[4].z = localCorners[5].z = localCorners[6].z = localCorners[7].z = pt->m_maxz;     //   0+------+1      *---x
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
int DgnDbRTree3dViewFilter::_TestRange(QueryInfo const& info)
    {
    BeAssert(6 == info.m_nCoord);
    info.m_within = Within::Outside;

    if (m_checkStop && m_checkStop->_CheckStop())
        return BE_SQLITE_ERROR;

    RTree3dValCP pt = (RTree3dValCP) info.m_coords;
    m_passedPrimaryTest   = (info.m_parentWithin == Within::Inside) ? true : (m_boundingRange.Intersects(*pt) && SkewTest(pt));
    m_passedSecondaryTest = m_useSecondary ? m_secondaryFilter.m_scorer.m_boundingRange.Intersects(*pt) : false;

    if (m_passedSecondaryTest)
        {
        if (!m_secondaryFilter.m_scorer.ComputeScore(&m_secondaryFilter.m_lastScore, *pt))
            {
            m_passedSecondaryTest = false;
            }
        else if (m_secondaryFilter.m_occlusionMapCount >= m_secondaryFilter.m_hitLimit && m_secondaryFilter.m_lastScore <= m_secondaryFilter.m_occlusionMapMinimum)
            {
            m_passedSecondaryTest = false;
            }

        if (m_passedSecondaryTest)
            info.m_within = Within::Partly;
        }

    if (!m_passedPrimaryTest)
        return BE_SQLITE_OK;

    DPoint3d localCorners[8];
    toLocalCorners(localCorners, pt);

#if defined (NEEDS_WORK_CLIPPING)
    if (m_clips.IsValid())
        {
        bool allClippedByOnePlane = false;
        for (ConvexClipPlaneSetCR cps : *m_clips)
            {
            if (allClippedByOnePlane = AllPointsClippedByOnePlane(cps, 8, localCorners))
                break;
            }

        if (allClippedByOnePlane)
            {
            m_passedPrimaryTest = false;
            return BE_SQLITE_OK;
            }
        }
#endif

    BeAssert(m_passedPrimaryTest);
    bool overlap, spansEyePlane;

    ++m_nScores;
    bool eliminatedByLOD;
    if (!m_scorer.ComputeOcclusionScore(&m_lastScore, overlap, spansEyePlane, eliminatedByLOD, localCorners, true))
        {
        m_eliminatedByLOD |= eliminatedByLOD;
        m_passedPrimaryTest = false;
        }
    else if (m_occlusionMapCount >= m_hitLimit && m_lastScore <= m_occlusionMapMinimum)
        {
        // this box is smaller than the smallest entry we already have, skip it.
        m_passedPrimaryTest = false;
        }

    if (m_passedPrimaryTest)
        {
        m_lastId = info.m_rowid;  // for debugging - make sure we get entries immediately after we score them.

        if (info.m_level>0)
            {
            // For nodes, return 'level-score' (the "-" is because for occlusion score higher is better. But for rtree priority, lower means better).
            info.m_score = info.m_level - m_lastScore;
            info.m_within = info.m_parentWithin == Within::Inside ? Within::Inside : m_boundingRange.Contains(*pt) ? Within::Inside : Within::Partly;
            }
        else
            {
            // For entries (ilevel==0), we return 0 so they are processed immediately (lowest score has highest priority).
            info.m_score = 0;
            info.m_within = Within::Partly;
            }
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
int DgnDbRTreeFitFilter::_TestRange(QueryInfo const& info)
    {
    RTree3dValCP pt = (RTree3dValCP) info.m_coords;

    if (m_fitRange.IsContained(pt->m_minx, pt->m_miny, pt->m_minz) &&
        m_fitRange.IsContained(pt->m_maxx, pt->m_maxy, pt->m_maxz))
        {
        info.m_within = Within::Outside; // If this range is entirely contained there is no reason to continue (it cannot contribute to the fit)
        }
    else
        {
        info.m_within = Within::Partly; 
        info.m_score  = info.m_level; // to get depth-first traversal
        if (info.m_level == 0)
            m_lastRange = DRange3d::From(pt->m_minx, pt->m_miny, pt->m_minz, pt->m_maxx, pt->m_maxy, pt->m_maxz);
        }

    return  BE_SQLITE_OK;
    }

const double ProgressiveViewFilter::s_purgeFactor = 1.3;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ProgressiveViewFilter::_StepRange(DbFunction::Context&, int nArgs, DbValue* args) 
    {
    if (m_context->WasAborted())
        return;

    // for restarts, skip calls up to the point where we finished last pass
    if (++m_nThisPass < m_nLastPass)
        return;

    ++m_nLastPass;

    DgnElementId elementId(args->GetValueInt64());
    if (nullptr != m_exclude && m_exclude->find(elementId) != m_exclude->end())
        return;

    if (m_existing.FindElementById(elementId))
        return;

    DgnElements& pool = m_dgndb.Elements();
    DgnElementCPtr el = pool.GetElement(elementId);
    if (el.IsValid())
        {
        GeometricElementCP geomElem = el->ToGeometricElement();
        if (nullptr != geomElem)
            {
            m_drewElementThisPass = true;
            m_context->VisitElement(*geomElem);
            }
        }

    if (pool.GetTotalAllocated() < (int64_t) m_elementReleaseTrigger)
        return;

    pool.DropFromPool(*el);

    // Purging the element does not purge the symbols so it may be necessary to do a full purge
    if (pool.GetTotalAllocated() < (int64_t) m_purgeTrigger)
        return;

    pool.Purge(m_elementReleaseTrigger);   // Try to get back to the elementPurgeTrigger

    // The purge may not have succeeded if there are elements in the QueryView's list of elements and those elements hold symbol references.
    // When that is true, we leave it to QueryViewController::_DrawView to try to clean up.  This logic just tries to recover from the
    // growth is caused.  It allows some growth between calls to purge to avoid spending too much time in purge.
    uint64_t newTotalAllocated = (uint64_t)pool.GetTotalAllocated();
    m_purgeTrigger = (uint64_t)(s_purgeFactor * std::max(newTotalAllocated, m_elementReleaseTrigger));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
int ProgressiveViewFilter::_TestRange(QueryInfo const& info)
    {
    BeAssert(6 == info.m_nCoord);
    info.m_within = Within::Outside;

    if (m_context->_CheckStop())
        return BE_SQLITE_ERROR;

    RTree3dValCP pt = (RTree3dValCP) info.m_coords;
    if ((info.m_parentWithin != Within::Inside) && !(m_boundingRange.Intersects(*pt) && SkewTest(pt)))
        return BE_SQLITE_OK;

    DPoint3d localCorners[8];
    toLocalCorners(localCorners, pt);

#if defined (NEEDS_WORK_CLIPPING)
    if (m_clips.IsValid())
        {
        bool allClippedByOnePlane = false;
        for (ConvexClipPlaneSetCR cps : *m_clips)
            {
            if (allClippedByOnePlane = AllPointsClippedByOnePlane(cps, 8, localCorners))
                break;
            }
        if (allClippedByOnePlane)
            return BE_SQLITE_OK;
        }
#endif

    if (info.m_level > 0) // only score nodes, not elements
        {
        bool   overlap, spansEyePlane;
        double score;

        bool excludedByLOD;
        if (!m_scorer.ComputeOcclusionScore(&score, overlap, spansEyePlane, excludedByLOD, localCorners, true))
            return BE_SQLITE_OK;

        info.m_within = info.m_parentWithin == Within::Inside ? Within::Inside : m_boundingRange.Contains(*pt) ? Within::Inside : Within::Partly;
        info.m_score = info.m_maxLevel - info.m_level - score;
        }
    else
        {
        info.m_score = 0;
        info.m_within = Within::Partly;
        }                                                                                                                                  
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2014
//---------------------------------------------------------------------------------------
bool ProgressiveViewFilter::_WantTimeoutSet(uint32_t& limit)
    {
    //  We want to limit how long ProgressiveDisplay runs but don't want to start the timer
    //  until it has drawn at least one element.
    if (!m_drewElementThisPass || m_setTimeout)
        return false;

    m_setTimeout = true;
    limit = 1000;

    return true;
    }

#define DEBUG_PRINTF(arg)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
IProgressiveDisplay::Completion ProgressiveViewFilter::_Process(ViewContextR context)
    {
    m_context = &context;
    m_nThisPass = 0; // restart every pass
    m_drewElementThisPass = m_setTimeout = false;

    DEBUG_PRINTF("start progressive display\n");
    if (BE_SQLITE_ROW != StepRTree(*m_rangeStmt))
        {
        m_rangeStmt->Reset();
        DEBUG_PRINTF("aborted progressive display\n");
        return IProgressiveDisplay::Completion::Aborted;
        }
    DEBUG_PRINTF("finished progressive display\n");

    return IProgressiveDisplay::Completion::Finished;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressiveViewFilter::~ProgressiveViewFilter()
    {
    HighPriorityOperationBlock  _v_v;
    m_rangeStmt = nullptr;
    }
