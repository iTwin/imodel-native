// ScalableMeshCao.cpp : Defines the entry point for the console application.
//

#include <ScalableMeshPCH.h>
using namespace std;
#include <iomanip>

#include "tetGen\tetgen.h"

//#pragma intrinsic
#ifdef DHDEBUG
#define DEBUGMSG
//#define DEBUGMSG_FLIP
//#pragma optimize( "", off )
//#define CHECKTETRAHEDRONS
#endif

#define IGNOREPOINTSONFACE2
//#define IGNOREPOINTSONFACE1
//#define IGNOREDUPLICATEPOINTS
#define FLIPFACETSRETRIES 1000
#define REMOVEDUPLICATEPOINTS
//#define ENABLETIMINGS
//#define DEBUGMSG_FLIPFACET7
//#define ALTERNATEACTIVETETCOLLECTION

#pragma warning(disable : 4189 )

#undef static_assert
#include <ppl.h>
#include <amp.h>
#undef round
#include <amp_math.h>
#include <concurrent_vector.h>
#include "predicates.h"
#include "gDel3d/Star.h"
#include "gDel3d/splaying.h"
#include "ScalableMeshCao.h"
#include "timer.h"
#include "TrimHull.h"
#include <Mtg/MtgStructs.h>

//#define parallel_for parallel_for_
//#define parallel_for_each parallel_for_each_

//#define LOG_TETRA
//#define LOG_FAILURES
//#define LOG_SPLIT
#ifdef LOG_TETRA
#define TETRA_LOG_FILE "D:\\0tetra.log"
#define TRACE_TETRA_INIT() \
    std::ofstream log; \
    log.open(TETRA_LOG_FILE, ios_base::app);
#define TRACE_TETRA_END() \
    log.close();
#else
#define TRACE_TETRA_INIT() {}
#define TRACE_TETRA_END() {}
#endif
#ifdef LOG_FAILURES
#define FAILURES_LOG_FILE "D:\\0fail.log"
#define TRACE_ERROR_INIT() \
    std::ofstream logErrors; \
    logErrors.open(FAILURES_LOG_FILE, ios_base::app);
#define TRACE_ERROR_END() \
    logErrors.close();
#define TRACE_ERROR(message) \
    logErrors << (message)<<endl;
#else
#define TRACE_ERROR_INIT() {}
#define TRACE_ERROR_END() {}
#define TRACE_ERROR(message){}
#endif
#ifdef LOG_SPLIT
#define SPLIT_LOG_FILE "D:\\0split.log"
#define TRACE_SPLIT_INIT() \
    std::ofstream logSplit; \
    logSplit.open(SPLIT_LOG_FILE, ios_base::app);
#define TRACE_SPLIT_END() \
    logSplit.close();
#define TRACE_SPLIT(message) \
    logSplit<< (message)<<endl;
#else
#define TRACE_SPLIT_INIT() {}
#define TRACE_SPLIT_END() {}
#define TRACE_SPLIT(message){}
#endif

#include "DTetrahedron3d.h"
bool Are3PointsCollinear (DPoint3d* triangle, double tolerance);

const int Flip32NewFaceVi[3][2] = {
        { 2, 1 }, // newTetIdx[0]'s vi, newTetIdx[1]'s vi
        { 1, 2 }, // -"-
        { 0, 0 }  // -"-
    };

const int Flip23IntFaceOpp[3][4] = {
        { 0, 1, 1, 2 },
        { 0, 2, 2, 1 },
        { 1, 1, 2, 2 }
    };

struct int2
    {
    int x;
    int y;
    };

struct int4
    {
    int x, y, z, w;
    };
struct FlipItem
    {
    int _v[5];
    int _t[3];
    };

struct FlipItemTetIdx
    {
    int _t[3];
    };

FlipItemTetIdx loadFlipTetIdx (FlipItem* flipArr, int idx)
    {
    int4 t = ((int4 *)flipArr)[idx * 2 + 1];

    FlipItemTetIdx flip = { t.y, t.z, t.w };

    return flip;
    }
static int2 make_int2 (int x, int y)
    {
    int2 t; t.x = x; t.y = y; return t;
    }

int getTetIdx (int input, int oldVi)
    {
    int idxVi = (input >> (oldVi * 4)) & 0xf;

    return (idxVi >> 2) & 0x3;
    }

int getTetVi (int input, int oldVi)
    {
    int idxVi = (input >> (oldVi * 4)) & 0xf;

    return idxVi & 0x3;
    }


int makePositive (int v)
    {
    //    CudaAssert (v < 0);
    return -(v + 2);
    }

// Escape -1 (special value)
int makeNegative (int v)
    {
    //    CudaAssert (v >= 0);
    return -(v + 2);
    }

template <class T> bool atomic_min_cpu (T& valP, T min)
    {
    std::atomic<T>& val = reinterpret_cast<std::atomic<T>&>(valP);
    T currentVal = val;

    while (min < currentVal)
        {
        if (std::atomic_compare_exchange_weak_explicit (&val, &currentVal, min, std::memory_order_release, std::memory_order_relaxed))
            return true;
        currentVal = val;
        }
    return false;
    }

template <class T> bool atomic_max_cpu (T& valP, T max)
    {
    std::atomic<T>& val = reinterpret_cast<std::atomic<T>&>(valP);
    T currentVal = val;

    while (max > currentVal)
        {
        if (std::atomic_compare_exchange_weak_explicit (&val, &currentVal, max, std::memory_order_release, std::memory_order_relaxed))
            return true;
        currentVal = val;
        }
    return false;
    }

using namespace concurrency;

struct Mesh
    {
    private:
        static const bool m_skipFlipOnNearZero;
        static const bool m_ensureSameResults;
        static const bool m_alwaysFlip;
        static const bool m_insphere;
        static const bool m_addOuterBox;
        static const bool m_doSplaying;

        MeshData& m_meshData;
        TetrahedronList& m_tetrahedrons; // The Tetrahedrons.
        int m_firstAddedPtNum;
        int& m_infPtNum;
        int& m_ignorePtsAfterNum;
        int m_numFlipIterations;
        bool m_doFlipping;
    public:
        DRange3d& m_extents;
        size_t m_numPoints;

        SMPointIndexList m_pointsToInsert;
        std::vector<int> m_pointTetrahedron;  // This stores the tetrahedron index that a point is in.
        std::vector<int> m_reuseTetrahedrons; // This stores the indexes of deleted tetrahedrons by flipping 3-2.
        SMPointList& m_points;                    // The Points.
        int m_pointsAlreadyAdded;             // The number of points that have been added.
#ifdef IGNOREPOINTSONFACE1
        bool m_ignorePointsOnFace;
#endif
        bool m_inSpecialFlip;
        // Timer Counters.
        AutoTimerCounter m_timeToSplay;
        AutoTimerCounter m_timeToFixPointsAfterFlip;
        AutoTimerCounter m_timeToExamineFacetsForFlip;
        AutoTimerCounter m_timeToDoFlip;
        AutoTimerCounter m_timeToMovePointsToTetrahedron;
        AutoTimerCounter m_timeToFindPointForTetrahedron;
        AutoTimerCounter m_timeToInsert;
        AutoTimerCounter m_timeToCleanupPoints;
        AutoTimerCounter m_timeToFixUpAdjectEdges;
        AutoTimerCounter m_timeToFlipFacets;
        // Flip Count Timer.
        int m_numFlipped[2];

        std::vector<int> m_pointNumberToInsert;        // Stores the Point number to insert into the tetrahedron via a split.
        std::vector<unsigned int> m_flipTestNumber;     // This stores the tetrahedron index that is going to operate the flip on. if all the tetrahedrons in the flip operation have the same number.
        std::vector<int> m_activeTetraNum;             // The indexes of the active tetrahedrons.

        Mesh (MeshData& meshData) : m_meshData (meshData), m_tetrahedrons (meshData.m_tetrahedrons), m_infPtNum (meshData.m_infPtNum), m_points (meshData.m_points), m_ignorePtsAfterNum (meshData.m_ignorePtsAfterNum), m_extents (meshData.m_extents)
            {
            }

        Mesh (MeshData& meshData, DPoint3dCP dataSet, int dataSetSize) : m_meshData (meshData), m_tetrahedrons (meshData.m_tetrahedrons), m_infPtNum (meshData.m_infPtNum), m_points (meshData.m_points), m_ignorePtsAfterNum (meshData.m_ignorePtsAfterNum), m_extents (meshData.m_extents)
            {
#ifdef IGNOREPOINTSONFACE1
            m_ignorePointsOnFace = true;
#endif
            m_inSpecialFlip = false;
            m_infPtNum = -1;
            m_numFlipIterations = 0;
            m_doFlipping = true;

            m_ignorePtsAfterNum = -1;
            exactinit ();

            m_numFlipped[0] = m_numFlipped[1] = 0;
            // Read Data
            ReadData (dataSet, dataSetSize);
            }

        void SortPoints (SMPointList::iterator start, SMPointList::iterator end, int axis = 0)
            {
            switch (axis)
                {
                case 0:
                    std::sort (start, end, [](const DPoint3d& a, const DPoint3d& b)
                        {
                        if (a.x < b.x) return true;
                        if (a.x > b.x) return false;
                        if (a.y < b.y) return true;
                        if (a.y > b.y) return false;
                        return (a.z < b.z);
                        });
                    break;
                case 1:
                    std::sort (start, end, [](const DPoint3d& a, const DPoint3d& b)
                        {
                        if (a.y < b.y) return true;
                        if (a.y > b.y) return false;
                        if (a.z < b.z) return true;
                        if (a.z > b.z) return false;
                        return (a.x < b.x);
                        });
                    break;
                case 2:
                    std::sort (start, end, [](const DPoint3d& a, const DPoint3d& b)
                        {
                        if (a.z < b.z) return true;
                        if (a.z > b.z) return false;
                        if (a.x < b.x) return true;
                        if (a.x > b.x) return false;
                        return (a.y < b.y);
                        });
                    break;
                }
            int numPoints = end - start;

            if (numPoints <= 4)
                return;
            axis = (axis + 1) % 3;
            SortPoints (start, start + (numPoints - 1), axis);
            SortPoints (start + numPoints, end, axis);
            }

        // Reads the data from the dataset and initializes the points to insert array.
        void ReadData (DPoint3dCP dataSet, int dataSetSize)
            {
            bool first = true;

            m_points.resize (dataSetSize);

            int ptNum = 0;
            for (int dataSetPtNum = 0; dataSetPtNum < dataSetSize; dataSetPtNum++)
                {
                DPoint3d pt = dataSet[dataSetPtNum];
                if (first)
                    {
                    first = false;
                    m_extents.low.x = m_extents.high.x = pt.x;
                    m_extents.low.y = m_extents.high.y = pt.y;
                    m_extents.low.z = m_extents.high.z = pt.z;
                    }
                else
                    {
                    if (m_extents.low.x > pt.x)
                        m_extents.low.x = pt.x;
                    else if (m_extents.high.x < pt.x)
                        m_extents.high.x = pt.x;

                    if (m_extents.low.y > pt.y)
                        m_extents.low.y = pt.y;
                    else if (m_extents.high.y < pt.y)
                        m_extents.high.y = pt.y;

                    if (m_extents.low.z > pt.z)
                        m_extents.low.z = pt.z;
                    else if (m_extents.high.z < pt.z)
                        m_extents.high.z = pt.z;
                    }
                m_points[ptNum++] = pt;
                }
//            SortPoints (trgParametersP->sortOfsP, 0, (int)m_points.size ());
            if (m_addOuterBox)
                {
                m_ignorePtsAfterNum = (int)m_points.size ();
                DRange3d extents = m_extents;
                static const double expandValue = 10;
                static const double maxValue = 1e40;

                extents.low.x -= expandValue; extents.low.y -= expandValue; extents.low.z -= expandValue;
                extents.high.x += expandValue; extents.high.y += expandValue; extents.high.z += expandValue;

                int numZ = (int)((extents.high.z - extents.low.z) / maxValue) + 1;
                double stepZ = (extents.high.z - extents.low.z) / numZ;
                for (int z = 0; z <= numZ; z++)
                    {
                    int numX = 1;
                    int numY = 1;
                    if (z == 0 || z == (numZ - 1))
                        {
                        numX = (int)((extents.high.x - extents.low.x) / maxValue) + 1;
                        numY = (int)((extents.high.y - extents.low.y) / maxValue) + 1;
                        }
                    double stepX = (extents.high.x - extents.low.x) / numX;
                    double stepY = (extents.high.y - extents.low.y) / numY;
                    for (int x = 0; x <= numX; x++)
                        {
                        for (int y = 0; y <= numY; y++)
                            {
                            m_points.push_back (DPoint3d::From (extents.low.x + (stepX * x), extents.low.y + (stepY * y), extents.low.z + (stepZ * z)));
                            }
                        }
                    }
                //m_points.push_back (DPoint3d::From (m_extents.low.x - 10, m_extents.high.y + 10, m_extents.low.z - 10));
                //m_points.push_back (DPoint3d::From (m_extents.high.x + 10, m_extents.high.y + 10, m_extents.low.z - 10));
                //m_points.push_back (DPoint3d::From (m_extents.high.x + 10, m_extents.low.y - 10, m_extents.low.z - 10));

                //m_points.push_back (DPoint3d::From (m_extents.low.x - 10, m_extents.low.y - 10, m_extents.high.z + 10));
                //m_points.push_back (DPoint3d::From (m_extents.low.x - 10, m_extents.high.y + 10, m_extents.high.z + 10));
                //m_points.push_back (DPoint3d::From (m_extents.high.x + 10, m_extents.high.y + 10, m_extents.high.z + 10));
                //m_points.push_back (DPoint3d::From (m_extents.high.x + 10, m_extents.low.y - 10, m_extents.high.z + 10));
                }
            /*
            std::cout << "Number Of Points " << m_points.size () << std::endl;
            std::cout << "Extents x (" << std::setprecision (8) << m_extents.low.x << "-" << m_extents.high.x << ")" << std::endl;
            std::cout << "        y (" << std::setprecision (8) << m_extents.low.y << "-" << m_extents.high.y << ")" << std::endl;
            std::cout << "        z (" << std::setprecision (8) << m_extents.low.z << "-" << m_extents.high.z << ")" << std::endl;
            */

            //gpu_timer timer;
            //timer.start ();
            //parallel_buffered_sort (m_points.begin (), m_points.end (), [] (DPoint3d& a, DPoint3d& b)
            //    {
            //    if (a.x < b.x) return true;
            //    if (a.x > b.x) return false;
            //    return (a.y < b.y);
            //    }
            //);
            //timer.stop ();
            }

        struct Get2Ddist
            {
            DPoint3d _a;
            double    abx, aby, abz;

            Get2Ddist (const DPoint3d &a, const DPoint3d &b) : _a (a)
                {
                abx = b.x - a.x;
                aby = b.y - a.y;
                abz = b.z - a.z;
                }

            double operator()(const DPoint3d &c)
                {
                double acx = c.x - _a.x;
                double acy = c.y - _a.y;
                double acz = c.z - _a.z;

                double xy = abx * acy - aby * acx;
                double yz = aby * acz - abz * acy;
                double zx = abz * acx - abx * acz;

                double dist = xy * xy + yz * yz + zx * zx;

                return dist;
                }
            };

        struct Get3Ddist
            {
            DPoint3d _a;
            double abx, aby, abz, acx, acy, acz, bc;

            Get3Ddist (const DPoint3d &a, const DPoint3d &b, const DPoint3d&c) : _a (a)
                {
                abx = b.x - a.x;
                aby = b.y - a.y;
                abz = b.z - a.z;
                acx = c.x - a.x;
                acy = c.y - a.y;
                acz = c.z - a.z;

                bc = abx * acy - aby * acx;
                }

            double operator()(const DPoint3d &d)
                {
                double adx = d.x - _a.x;
                double ady = d.y - _a.y;
                double adz = d.z - _a.z;

                double cd = acx * ady - acy * adx;
                double db = adx * aby - ady * abx;

                double dist = abz * cd + acz * db + adz * bc;

                return fabs (dist);
                }
            };

        // Creates the initial tetrahedron set.

        void InitialiseTetrahedron ()
            {
            m_pointsToInsert.resize (m_points.size ());
            for (size_t i = 0; i < m_points.size (); i++)
                m_pointsToInsert[i] = (int)i;

            // Add the point and the tetrahedron.
            m_pointTetrahedron.resize (m_pointsToInsert.size ());
            for (int& location : m_pointTetrahedron)
                location = 0;

            m_tetrahedrons.clear ();
            m_tetrahedrons.reserve (m_points.size () * 10);

            // Work out a tetrahedron to contain all points.
            DRange3d extents = m_extents;
            int v[4];
            double minX = m_points[0].x, maxX = m_points[0].x;

            v[0] = 0;
            v[1] = 0;
            for (int i = 1; i < (int)m_points.size (); i++)
                {
                if (minX > m_points[i].x)
                    {
                    minX = m_points[i].x;
                    v[0] = i;
                    }
                else if (maxX < m_points[i].x)
                    {
                    maxX = m_points[i].x;
                    v[1] = i;
                    }
                }

            Get2Ddist d2d (m_points[v[0]], m_points[v[1]]);
            double v2Dist = d2d (m_points[0]);
            v[2] = 0;
            for (int i = 1; i < (int)m_points.size (); i++)
                {
                double nD = d2d (m_points[i]);
                if (nD >= v2Dist&& i != v[0] && i != v[1])
                    {
                    v[2] = i;
                    v2Dist = nD;
                    }
                }

            Get3Ddist d3d (m_points[v[0]], m_points[v[1]], m_points[v[2]]);
            double v3Dist = d3d (m_points[0]);

            v[3] = 0;
            for (int i = 1; i < (int)m_points.size (); i++)
                {
                double nD = d3d (m_points[i]);
                if (nD >= v3Dist && i != v[0] && i != v[1] && i != v[2])
                    {
                    v[3] = i;
                    v3Dist = nD;
                    }
                }
            // NEEDS_WORK_SM : See if this is a better solution?
            // Find MinX and MaxX (V0/V1)
            // Then Find point futhest from V0/V1
            // Then Find point furthest from V0/V1/V2.

            DPoint3d& p0 = m_points[v[0]];
            DPoint3d& p1 = m_points[v[1]];
            DPoint3d& p2 = m_points[v[2]];
            DPoint3d& p3 = m_points[v[3]];
            DPoint3d ptInfty;
            ptInfty.x = (p0.x + p1.x + p2.x + p3.x) / 4.0;
            ptInfty.y = (p0.y + p1.y + p2.y + p3.y) / 4.0;
            ptInfty.z = (p0.z + p1.z + p2.z + p3.z) / 4.0;

            //std::cout << "_minVal = 666.91, _maxVal == 2.14201e+006" << std::endl;
            //std::cout << "Leftmost: " << v[0] << " -- > 2141419.9 223843.09 808.37" << std::endl;
            //std::cout << "Rightmost: " << v[1] << " -- > 2142014.2 223675.77 747.08" << std::endl;
            //std::cout << "Furthest 2D: " << v[2] << " -- > 2142013.2 225001.19 670.33" << std::endl;
            //std::cout << "Furthest 3D: " << v[3] << " -- > 2142013.8 224467.78 771.41" << std::endl;
            //std::cout << "Kernel: " << ptInfty.x << " " << ptInfty.y << " " << ptInfty.z << std::endl;

            DTetrahedron3d dTet (m_points[v[0]], m_points[v[1]], m_points[v[2]], m_points[v[3]]);
            assert (v[0] != v[1] && v[1] != v[2] && v[3] != v[0] && v[1] != v[3] && v[3] != v[2] && v[3] != v[1] && v[0] != v[2]);

            if (!dTet.isLeft (3, dTet.points[3]))
                std::swap (v[1], v[2]);
            int ptNum = (int)m_points.size ();
            int infIdx = ptNum;
            m_infPtNum = m_firstAddedPtNum = ptNum;
            m_points.push_back (ptInfty);

            const int tets[][4] = {
                    { v[0], v[1], v[2], v[3] },
                    { v[1], v[2], v[3], infIdx },
                    { v[0], v[3], v[2], infIdx },
                    { v[0], v[1], v[3], infIdx },
                    { v[0], v[2], v[1], infIdx }
                };

            const int oppTet[][4] = {
                    { 1, 2, 3, 4 },
                    { 2, 3, 4, 0 },
                    { 1, 4, 3, 0 },
                    { 1, 2, 4, 0 },
                    { 1, 3, 2, 0 }
                };

            for (int i = 0; i < 5; i++)
                {
                assert (tets[i][0] != tets[i][1] && tets[i][1] != tets[i][2] && tets[i][3] != tets[i][0] && tets[i][1] != tets[i][3] && tets[i][3] != tets[i][2]
                        && tets[i][3] != tets[i][1] && tets[i][0] != tets[i][2]);
                Tetrahedron t (tets[i][0], tets[i][1], tets[i][2], tets[i][3], oppTet[i][0], oppTet[i][1], oppTet[i][2], oppTet[i][3]);
                t.isActive = true;
                m_tetrahedrons.push_back (t);
                }

            for (int i = 0; i < 4; i++)
                {
                m_pointsToInsert[v[i]] = -1;
                }

            CleanUpNegativePoints ();
            m_pointTetrahedron.resize (m_pointsToInsert.size ());
            m_numPoints = 5;
            }


        Orient doOrient3DSoSOnly (
            int v0, int v1, int v2, int v3,
            const DPoint3d& p0, const DPoint3d& p1, const DPoint3d& p2, const DPoint3d& p3) const
            {
            ////
            // Sort points using vertex as key, also note their sorted order
            ////

            const int DIM = 3;
            const int NUM = DIM + 1;
            const double* p[NUM] = { &p0.x, &p1.x, &p2.x, &p3.x };
            int pn = 1;

            if (v0 > v2)
                {
                std::swap (v0, v2); std::swap (p[0], p[2]); pn = -pn;
                }
            if (v1 > v3)
                {
                std::swap (v1, v3); std::swap (p[1], p[3]); pn = -pn;
                }
            if (v0 > v1)
                {
                std::swap (v0, v1); std::swap (p[0], p[1]); pn = -pn;
                }
            if (v2 > v3)
                {
                std::swap (v2, v3); std::swap (p[2], p[3]); pn = -pn;
                }
            if (v1 > v2)
                {
                std::swap (v1, v2); std::swap (p[1], p[2]); pn = -pn;
                }

            double result = 0;
            double pa2[2], pb2[2], pc2[2];
            int depth;

            for (depth = 0; depth < 14; ++depth)
                {
                switch (depth)
                    {
                    case 0:
                        pa2[0] = p[1][0];   pa2[1] = p[1][1];
                        pb2[0] = p[2][0];   pb2[1] = p[2][1];
                        pc2[0] = p[3][0];   pc2[1] = p[3][1];
                        break;
                    case 1:
                        /*pa2[0] = p[1][0];*/ pa2[1] = p[1][2];
                        /*pb2[0] = p[2][0];*/ pb2[1] = p[2][2];
                        /*pc2[0] = p[3][0];*/ pc2[1] = p[3][2];
                        break;
                    case 2:
                        pa2[0] = p[1][1];   //pa2[1] = p[1][2];
                        pb2[0] = p[2][1];   //pb2[1] = p[2][2];
                        pc2[0] = p[3][1];   //pc2[1] = p[3][2];
                        break;
                    case 3:
                        pa2[0] = p[0][0];   pa2[1] = p[0][1];
                        pb2[0] = p[2][0];   pb2[1] = p[2][1];
                        pc2[0] = p[3][0];   pc2[1] = p[3][1];
                        break;
                    case 4:
                        result = p[2][0] - p[3][0];
                        break;
                    case 5:
                        result = p[2][1] - p[3][1];
                        break;
                    case 6:
                        /*pa2[0] = p[0][0];*/ pa2[1] = p[0][2];
                        /*pb2[0] = p[2][0];*/ pb2[1] = p[2][2];
                        /*pc2[0] = p[3][0];*/ pc2[1] = p[3][2];
                        break;
                    case 7:
                        result = p[2][2] - p[3][2];
                        break;
                    case 8:
                        pa2[0] = p[0][1];   //pa2[1] = p[0][2];
                        pb2[0] = p[2][1];   //pb2[1] = p[2][2];
                        pc2[0] = p[3][1];   //pc2[1] = p[3][2];
                        break;
                    case 9:
                        pa2[0] = p[0][0];   pa2[1] = p[0][1];
                        pb2[0] = p[1][0];   pb2[1] = p[1][1];
                        pc2[0] = p[3][0];   pc2[1] = p[3][1];
                        break;
                    case 10:
                        result = p[1][0] - p[3][0];
                        break;
                    case 11:
                        result = p[1][1] - p[3][1];
                        break;
                    case 12:
                        result = p[0][0] - p[3][0];
                        break;
                    default:
                        result = 1.0;
                        break;
                    }

                switch (depth)
                    {
                    case 0: case 1: case 2: case 3: case 6: case 8: case 9:
                        result = orient2d (*(const DPoint3d*)pa2, *(const DPoint3d*)pb2, *(const DPoint3d*)pc2);
                    }

                if (result != 0)
                    break;
                }

            switch (depth)
                {
                case 1: case 3: case 5: case 8: case 10:
                    result = -result;
                }

            const double det = result * pn;

            return ortToOrient (det);
            }


        inline Orient doOrient3DSoS (
            int v0, int v1, int v2, int v3,
            const DPoint3d& p0, const DPoint3d& p1, const DPoint3d& p2, const DPoint3d& p3) const
            {
            // Fast-Exact
            double det = orient3d (p0, p1, p2, p3);
            Orient ord = ortToOrient (det);

            if (OrientZero == ord)
                {
                ord = doOrient3DSoSOnly (v0, v1, v2, v3, p0, p1, p2, p3);
                if (v0 == m_infPtNum || v1 == m_infPtNum || v2 == m_infPtNum)
                    ord = flipOrient (ord);
                return ord;
                }

            if (v0 == m_infPtNum || v1 == m_infPtNum || v2 == m_infPtNum)
                {
                ord = flipOrient (ord);
                }

            return ord;
            }

        inline Orient doOrient3D (
            int v0, int v1, int v2, int v3,
            const DPoint3d& p0, const DPoint3d& p1, const DPoint3d& p2, const DPoint3d& p3) const
            {
            if (v3 == m_infPtNum)
                v3 = v3;
            // Fast-Exact
            double det = orient3d (p0, p1, p2, p3);
            Orient ord = ortToOrient (det);

            if (v0 == m_infPtNum || v1 == m_infPtNum || v2 == m_infPtNum)
                {
                det = -det;
                ord = flipOrient (ord);
                }

            return ord;
            }

        // Moved the points into the initial tetrahedron.
        bool MovePointsIntoTetrahedron ()
            {
            AutoTimer timer (m_timeToMovePointsToTetrahedron);

            DTetrahedron3d dTet;
            const int tetVert[5] = { m_tetrahedrons[0].ptNums[0],
                m_tetrahedrons[0].ptNums[1],
                m_tetrahedrons[0].ptNums[2],
                m_tetrahedrons[0].ptNums[3],
                m_infPtNum };
            const DPoint3d pt[] = {
                m_points[tetVert[0]],
                m_points[tetVert[1]],
                m_points[tetVert[2]],
                m_points[tetVert[3]],
                m_points[tetVert[4]]
                };
            const int SplitFaces[11][3] = {
                /*0*/{ 0, 1, 4 },
                /*1*/{ 0, 3, 4 },                      /*2*/{ 0, 2, 4 },
                /*3*/{ 2, 3, 4 },  /*4*/{ 1, 3, 4 },  /*5*/{ 1, 2, 4 }, /*6*/{ 2, 3, 4 },
                /*7*/{ 1, 3, 2 },  /*8*/{ 0, 2, 3 },  /*9*/{ 0, 3, 1 }, /*10*/{ 0, 1, 2 }
                };

            const int SplitNext[11][2] = {
                    { 1, 2 },
                    { 3, 4 }, { 5, 6 },
                    { 7, 8 }, { 9, 7 }, { 7, 10 }, { 7, 8 },
                    { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }
                };

            int size = (int)m_pointsToInsert.size ();
            TRACE_TETRA_INIT ()
#ifdef LOG_TETRA
                log << " [MovePointsIntoTetrahedron] " + std::to_string (size) + " points to be inserted." << endl;
#endif

            parallel_for ((int)0, (int)size, (int)1, [&](int aindx)
                {
                // Iterate points
                DPoint3d ptVertex = m_points[m_pointsToInsert[aindx]];

                int face = 0;

                for (int i = 0; i < 4; ++i)
                    {
                    const int *fv = SplitFaces[face];

                    Orient ort = doOrient3DSoS (
                        tetVert[fv[0]], tetVert[fv[1]], tetVert[fv[2]], m_pointsToInsert[aindx],
                        pt[fv[0]], pt[fv[1]], pt[fv[2]], ptVertex);

                    // Use the reverse direction 'cause the splitting point is Infty!
                    face = SplitNext[face][(ort == OrientPos) ? 1 : 0];

                    // Compiler bug: Without this assertion, this code produces undefined result in Debug-x86.
                    //                    CudaAssert (face >= 0);
                    }

                m_pointTetrahedron[aindx] = /*tetIdx + */face;
                });
#ifdef DHDEBUG
//            CheckTetrahedronForPoint ();
#endif
            TRACE_TETRA_END ()
                return false;
            }

        // Finds the point to insert for a tetrahedron.
        bool FindInsertPointForTetrahedron ()
            {
            std::vector<double> tetrahedronCenters;
            AutoTimer timer (m_timeToFindPointForTetrahedron);
            int size = (int)m_pointsToInsert.size ();
            const int tetrahedronsSize = (int)m_tetrahedrons.size ();

            m_pointNumberToInsert.resize (0);
            m_pointNumberToInsert.resize (m_tetrahedrons.size (), -1);
            tetrahedronCenters.resize (m_tetrahedrons.size (), -1e100);

            auto FindInsertPointForTetrahedronFunc = [&](int aindx)
                {
                if (m_pointTetrahedron[aindx] < 0)
                    return;
                int tn = m_pointTetrahedron[aindx];
                Tetrahedron& t = m_tetrahedrons[tn];

                const DPoint3d& p = m_points[m_pointsToInsert[aindx]];

#ifdef IGNOREPOINTSONFACE1
                int count = 0;
                //                if (m_ignorePointsOnFace)
                    {
                    DTetrahedron3d dTetrahedron;
                    t.GetDTetrahedron3d (dTetrahedron, m_points);
                    bool isOnEdge = false;
                    for (int f = 0; f < numFacets; f++)
                        if (dTetrahedron.isOn (f, p))
                            {
                            count++;
                            }
                    }
                if (m_ignorePointsOnFace)
                    {
                    if (count != 0)
                        return;
                    }
                else
                    {
                    tn = tn;
                    }
#elif defined (IGNOREDUPLICATEPOINTS)
                DTetrahedron3d dTetrahedron;
                t.GetDTetrahedron3d (dTetrahedron, m_points);
                for (int f = 0; f < numFacets; f++)
                    {
                    if (dTetrahedron.points[f].IsEqual (p))
                        {
                        return;
                        }
                    }
#endif

                double distSquared;
                int infVi = t.GetPointIndex (m_infPtNum);
                if (infVi != -1)
                    {
                    const int *fv = Tetrahedron::GetDTetrahedron3dFacet (infVi);
                    if (m_insphere)
                        distSquared = -orient3d (m_points[t.ptNums[fv[0]]], m_points[t.ptNums[fv[1]]], m_points[t.ptNums[fv[2]]], p);
                    else
                        {

                        // Note: reverse the orientation
                        for (int vn = 0; vn < 3; vn++)
                            {
                            Get2Ddist l (m_points[t.ptNums[fv[vn]]], m_points[t.ptNums[fv[(vn + 1) % 3]]]);
                            double d = l (p);
                            if (d == 0)
                                {
                                d = distToSquared (m_points[t.ptNums[fv[vn]]], p);
                                double d2 = distToSquared (m_points[t.ptNums[fv[(vn + 1) % 3]]], p);
                                if (d2 < d)
                                    d = d2;
                                d = -d;
                                }
                            if (vn == 0 || d < distSquared)
                                distSquared = d;
                            }
                        }
                    //#ifdef IGNOREPOINTSONFACE1
                    //                    if (!m_ignorePointsOnFace)
                    //                        {
                    //
                    //                        DPoint3d cent = DPoint3d::FromSumOf (m_points[t.ptNums[fv[0]]], m_points[t.ptNums[fv[1]]], 1, m_points[t.ptNums[fv[2]]], 1);
                    //                        cent.x /= 3;
                    //                        cent.y /= 3;
                    //                        cent.z /= 3;
                    //                        distSquared = distToSquared (cent, p);
                    //                        if (count != 1)
                    //                            distSquared = -distSquared;
                    ////                        distSquared = inspheredet (&m_points[t.ptNums[0]].x, &m_points[t.ptNums[1]].x, &m_points[t.ptNums[2]].x, &m_points[t.ptNums[3]].x, &p.x);
                    //                        }
                    //#endif
                    }
                else
                    {
                    // Might need a better find routine when they are on the surface
                    if (m_insphere)
                        {
                        distSquared = -inspheredet (&m_points[t.ptNums[0]].x, &m_points[t.ptNums[1]].x, &m_points[t.ptNums[2]].x, &m_points[t.ptNums[3]].x, &p.x);
                        }
                    else
                        {
                        const int edges[6][2] = {
                                { 0, 1 },
                                { 0, 2 },
                                { 0, 3 },
                                { 1, 2 },
                                { 1, 3 },
                                { 2, 3 }
                            };

                        for (int vn = 0; vn < 6; vn++)
                            {
                            Get2Ddist l (m_points[t.ptNums[edges[vn][0]]], m_points[t.ptNums[edges[vn][1]]]);
                            double d = l (p);
                            if (d == 0)
                                {
                                d = distToSquared (m_points[t.ptNums[edges[vn][0]]], p);
                                double d2 = distToSquared (m_points[t.ptNums[edges[vn][1]]], p);
                                if (d2 < d)
                                    d = d2;
                                d = -d;
                                }
                            if (vn == 0 || d < distSquared)
                                distSquared = d;
                            }
                        }
                    }
                //if (isOnEdge && distSquared > 0)
                //    distSquared = -10000;
                //if (m_pointNumberToInsert[tn] == -1 || tetrahedronCenters[tn] < distSquared)
                if (tetrahedronCenters[tn] < distSquared && atomic_max_cpu (tetrahedronCenters[tn], distSquared))
                    {
                    //                    tetrahedronCenters[tn].distanceSquared = distSquared;
                    m_pointNumberToInsert[tn] = aindx;
                    }
                };

            if (m_ensureSameResults)
                parallel_for_ ((int)0, size, (int)1, FindInsertPointForTetrahedronFunc);
            else
                parallel_for ((int)0, size, (int)1, FindInsertPointForTetrahedronFunc);

            int found = 0;
            int ti = 0;

            m_activeTetraNum.clear ();
            m_pointsAlreadyAdded = (int)m_numPoints;
            for (int& ptNum : m_pointNumberToInsert)
                {
                if (ptNum >= 0)
                    {
#ifdef VALIDATION_CHECK
                    DTetrahedron3d dTet;
                    m_tetrahedrons[ti].GetDTetrahedron3d (dTet, m_points);
                    if (!dTet.IsPointIn (m_points[m_pointsToInsert[ptNum]]))
                        ti = ti;
#endif
                    m_pointTetrahedron[ptNum] = -2;
                    m_activeTetraNum.push_back (m_pointsToInsert[ptNum]);
                    ptNum = (int)m_activeTetraNum.size () - 1;
                    found++;
                    }
                ti++;
                }

            m_numPoints += found;
            TRACE_SPLIT_INIT ()
                int oldPoints = (int)m_pointsToInsert.size ();
            TRACE_SPLIT ("[FindInsertPoint]  Size is " + std::to_string (m_pointsToInsert.size ()))
                CleanUpPoints ();
            TRACE_SPLIT ("[FindInsertPoint]  After Cleanup Size is " + std::to_string (m_pointsToInsert.size ()))
                TRACE_SPLIT_END ()
#ifdef DEBUGMSG
                std::cout << "Number of Points To Process " << oldPoints << "(" << oldPoints - m_pointsToInsert.size () << ")" << std::endl;
            std::cout << "Number of Tetrahedrons " << m_tetrahedrons.size () << std::endl;
#endif
            if (oldPoints - (oldPoints - m_pointsToInsert.size ()) < (oldPoints - m_pointsToInsert.size ()) && (oldPoints - m_pointsToInsert.size ()) < (m_points.size () * 0.1))
                m_doFlipping = false;
            //        std::cout << std::setprecision (8) << "Points elapsed_time=" << timer.read () << "(sec.)\n";
            return found != 0;// totalCount != 0;
            }

        // Cleans up the points.
        void CleanUpPoints ()
            {
            TRACE_SPLIT_INIT ()
                AutoTimer timer (m_timeToCleanupPoints);
#ifdef TEST_
            size_t pointSize = 0;
            for (size_t i = 0; i < m_pointsToInsert.size (); i++)
                {
                if (m_pointTetrahedron[i] < 0)
                    continue;
                m_pointTetrahedron[pointSize] = m_pointTetrahedron[i];
                m_pointsToInsert[pointSize++] = m_pointsToInsert[i];
                }
#else
            size_t pointSize = m_pointsToInsert.size ();
            for (size_t i = 0; i < pointSize; i++)
                {
                if (m_pointTetrahedron[i] < 0)
                    {
                    pointSize--;
                    TRACE_SPLIT ("[OnEdgeOrFace] Cleaned up " + std::to_string (m_pointsToInsert[i]))
                        m_pointTetrahedron[i] = m_pointTetrahedron[pointSize];
                    m_pointsToInsert[i] = m_pointsToInsert[pointSize];
                    i--;
                    }
                }
#endif
            m_pointsToInsert.resize (pointSize);
            m_pointTetrahedron.resize (pointSize);
            TRACE_SPLIT_END ()
            }

        void CleanUpNegativePoints ()
            {
            AutoTimer timer (m_timeToCleanupPoints);
#ifdef TEST_
            size_t pointSize = 0;
            for (size_t i = 0; i < m_pointsToInsert.size (); i++)
                {
                if (m_pointsToInsert[i] < 0)
                    continue;
                m_pointsToInsert[pointSize++] = m_pointsToInsert[i];
                }
#else
            size_t pointSize = m_pointsToInsert.size ();
            for (size_t i = 0; i < pointSize; i++)
                {
                if (m_pointsToInsert[i] < 0)
                    {
                    pointSize--;

                    m_pointsToInsert[i] = m_pointsToInsert[pointSize];
                    i--;
                    }
                }
#endif
            m_pointsToInsert.resize (pointSize);
            }
        // Inserts the points and splits the tetrahedron into 4.
        void MovePointsIntoNewTetrahedronBeforeInsert ()
            {
            TRACE_TETRA_INIT ()
                const int SplitFaces[11][3] = {
                /*0*/{ 0, 1, 4 },
                /*1*/{ 0, 3, 4 },                      /*2*/{ 0, 2, 4 },
                /*3*/{ 2, 3, 4 },  /*4*/{ 1, 3, 4 },  /*5*/{ 1, 2, 4 }, /*6*/{ 2, 3, 4 },
                /*7*/{ 1, 3, 2 },  /*8*/{ 0, 2, 3 },  /*9*/{ 0, 3, 1 }, /*10*/{ 0, 1, 2 }
                };

            const int SplitNext[11][2] = {
                    { 1, 2 },
                    { 3, 4 }, { 5, 6 },
                    { 7, 8 }, { 9, 7 }, { 7, 10 }, { 7, 8 },
                    { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }
                };
            int numOriginalTetrahedrons = (int)m_tetrahedrons.size ();
            int size = (int)m_pointsToInsert.size ();
            const int tetrahedronsSize = (int)m_tetrahedrons.size ();

            parallel_for ((int)0, size, (int)1, [&](int aindx)
                {
                int prevT = m_pointTetrahedron[aindx];
                if (prevT == -1)
                    return;
                int newPtNum = m_pointNumberToInsert[prevT];

                if (newPtNum == -1)
                    return;    // No points to insert.

                int originalTetrahedronNumber = prevT;
                Tetrahedron& st = m_tetrahedrons[originalTetrahedronNumber];
                // Work out where the 3 new tetrahedrons are going to go.
                int firstNewTetra = numOriginalTetrahedrons + (newPtNum * 3);
                newPtNum = m_activeTetraNum[newPtNum];

                const DPoint3d& p = m_points[m_pointsToInsert[aindx]];
#ifdef LOG_TETRA
                log << "[MovePointsIntoNewTetrahedronBeforeInsert] INSERTING point id " + std::to_string (m_pointsToInsert[aindx]) + " of coordinates (" + std::to_string (p.x)
                    + "," + std::to_string (p.y) + "," + std::to_string (p.z) + ")" << endl;
#endif
                //const int vertex = vertexArr._arr[vertIdx];

                //const Point3 ptVertex = dPredWrapper.getPoint (vertex);
                //const int splitVertex = vertexArr._arr[splitVertIdx];
                //const Tet tet = loadTet (tetArr, tetIdx);

                //                const int freeIdx = (splitVertex + 1) * MeanVertDegree - 1;
                const int tetVert[5] = {
                    st.ptNums[0], st.ptNums[1], st.ptNums[2], st.ptNums[3], newPtNum };
                const DPoint3d* pt[] = {
                    &m_points[tetVert[0]],
                    &m_points[tetVert[1]],
                    &m_points[tetVert[2]],
                    &m_points[tetVert[3]],
                    &m_points[tetVert[4]]
                    };
#ifdef LOG_TETRA
                log << "[MovePointsIntoNewTetrahedronBeforeInsert] ORIGINAL tetrahedron [" + std::to_string (originalTetrahedronNumber) + "(" + std::to_string (m_points[tetVert[0]].x)
                    + "," + std::to_string (m_points[tetVert[0]].y) + "," + std::to_string (m_points[tetVert[0]].z) + ")(" + std::to_string (m_points[tetVert[1]].x)
                    + "," + std::to_string (m_points[tetVert[1]].y) + "," + std::to_string (m_points[tetVert[1]].z) + ")(" + std::to_string (m_points[tetVert[2]].x)
                    + "," + std::to_string (m_points[tetVert[2]].y) + "," + std::to_string (m_points[tetVert[2]].z) + ")(" + std::to_string (m_points[tetVert[3]].x)
                    + "," + std::to_string (m_points[tetVert[3]].y) + "," + std::to_string (m_points[tetVert[3]].z) + ")(" << endl;
#endif
                int face = 0;

                //int sf[5][4] = {
                //        {1,0,3,4},
                //        {2,1,3,4},
                //        {3,0,2,4},
                //        { 0, 1, 2, 4 },
                //        { 0, 1, 2, 3 },
                //    };

                //for (int a = 0; a < 5; a++)
                //    {
                //    Tetrahedron t (tetVert[sf[a][0]], tetVert[sf[a][1]], tetVert[sf[a][2]], tetVert[sf[a][3]]);
                //    bool failed = false;
                //    for (int f = 0; f < numFacets; f++)
                //        {
                //        int ptNums[3];
                //        t.GetFacePoints (f, ptNums);
                //        Orient ort3 = doOrient3DSoS (
                //            ptNums[0], ptNums[1], ptNums[2], m_pointsToInsert[aindx],
                //            m_points[ptNums[0]], m_points[ptNums[1]], m_points[ptNums[2]], p);
                //        if (ort3 == OrientNeg)
                //            failed = true;
                //        }
                //    if (!failed)
                //        failed = failed;
                //    }

                for (int i = 0; i < 3; ++i)
                    {
                    const int *fv = SplitFaces[face];
                    Orient ort = doOrient3D (
                        tetVert[fv[0]], tetVert[fv[1]], tetVert[fv[2]], m_pointsToInsert[aindx],
                        *pt[fv[0]], *pt[fv[1]], *pt[fv[2]], p);
                    if (OrientZero == ort)
                        {
                        if (tetVert[fv[0]] == m_infPtNum || tetVert[fv[1]] == m_infPtNum || tetVert[fv[2]] == m_infPtNum)
                            {
                            }
                        else
                            {
                            ort = doOrient3DSoS (
                                tetVert[fv[0]], tetVert[fv[1]], tetVert[fv[2]], m_pointsToInsert[aindx],
                                *pt[fv[0]], *pt[fv[1]], *pt[fv[2]], p);
                            }
                        }
#ifdef LOG_TETRA
                    log << "[MovePointsIntoNewTetrahedronBeforeInsert] ORIENT " + std::string (ort == OrientPos ? "+1" : (ort == OrientNeg ? "-1" : "0")) << endl;
#endif
                    face = SplitNext[face][ort == OrientPos ? 0 : 1];
                    }

                if (face >= 0)
                    {
                    int newT = prevT;
                    const int num[4] = { 1, 2, 0, -1 };
                    if (face - 7 >= 0 && num[face - 7] != -1)
                        {
                        newT = firstNewTetra + num[face - 7];
#ifdef LOG_TETRA
                        log << "[MovePointsIntoNewTetrahedronBeforeInsert] NEW tetra for idx " + std::to_string (m_pointsToInsert[aindx]) + ":" + std::to_string (newT) << endl;
#endif
                        }
                    else
                        {
#ifdef LOG_TETRA
                        log << "[MovePointsIntoNewTetrahedronBeforeInsert] OLD tetrafor idx " + std::to_string (m_pointsToInsert[aindx]) + ":" + std::to_string (newT) << endl;
#endif
                        }
                    face = face;
                    m_pointTetrahedron[aindx] = newT;
                    }
                });
            TRACE_TETRA_END ()
            }

#ifdef DHDEBUG
        void CheckTetrahedronForPoint ()
            {
            int size = (int)m_pointsToInsert.size ();

            parallel_for ((int)0, size, (int)1, [&](int aindx)
                {
                const DPoint3d& p = m_points[m_pointsToInsert[aindx]];
                const Tetrahedron& ptTet = m_tetrahedrons[m_pointTetrahedron[aindx]];
                int foundIndx = -1;

                //bool failed = false;
                //for (int f = 0; f < numFacets; f++)
                //    {
                //    int ptNums[3];
                //    ptTet.GetFacePoints (f, ptNums);
                //    Orient ort3 = doOrient3DSoS (
                //        ptNums[0], ptNums[1], ptNums[2], m_pointsToInsert[aindx],
                //        m_points[ptNums[0]], m_points[ptNums[1]], m_points[ptNums[2]], p);
                //    if (ort3 == OrientNeg)
                //        failed = true;
                //    }
                //if (failed)
                    {
                    for (const Tetrahedron& tet : m_tetrahedrons)
                        {
                        bool failed = false;
                        if (tet.ptNums[0] == -1)
                            break;
                        Orient ort = doOrient3DSoS (tet.ptNums[0], tet.ptNums[1], tet.ptNums[2], tet.ptNums[3], m_points[tet.ptNums[0]], m_points[tet.ptNums[1]], m_points[tet.ptNums[2]], m_points[tet.ptNums[3]]);

                        if (ort == OrientNeg)
                            ort = ort;
                        for (int f = 0; f < numFacets; f++)
                            {
                            int ptNums[3];
                            tet.GetFacePoints (f, ptNums);
                            Orient ort3 = doOrient3DSoS (
                                ptNums[0], ptNums[1], ptNums[2], m_pointsToInsert[aindx],
                                m_points[ptNums[0]], m_points[ptNums[1]], m_points[ptNums[2]], p);

                            if (ort3 == OrientNeg)
                                {
                                failed = true;
                                break;
                                }
                            }
                        if (!failed)
                            {
                            if (foundIndx != -1)
                                {
                                foundIndx = -2;
                                break;
                                }
                            foundIndx = &tet - m_tetrahedrons.data ();
                            }
                        }
                    //                    foundIndx = 0;
                    }
                if (foundIndx != m_pointTetrahedron[aindx])
                    foundIndx = foundIndx;
                });
            }
#endif
        void SplitTetrahedrons ()
            {
#ifdef IGNOREPOINTSONFACE2
            SplitTetrahedronsOnEdgeOrFace ();
#endif
            int numOriginalTetrahedrons = (int)m_tetrahedrons.size ();

            MovePointsIntoNewTetrahedronBeforeInsert ();

#ifdef DEBUGMSG
            //        std::cout << "Resize StoredTetrahedrons " << tetrahedronSize + (numActiveTetrahedrons * 3) << std::endl;
#endif
            int numOfPointsToInsert = (int)(m_numPoints - m_pointsAlreadyAdded);
            m_tetrahedrons.resize (numOriginalTetrahedrons + (numOfPointsToInsert * 3));
#ifdef DEBUGMSG
            //        std::cout << "Resize findInsertInfo " << (numActiveTetrahedrons * numFacets) << std::endl;
#endif
            TRACE_TETRA_INIT ()
#ifdef LOG_TETRA
                log << "[SplitTetrahedrons] Inserting " + std::to_string (numOfPointsToInsert) + " points into tetrahedron ..." << endl;
#endif


            AutoTimer timer (m_timeToInsert);
            parallel_for ((int)0, (int)numOriginalTetrahedrons, (int)1, [&](int aindx)
                {
                int newPtNum = m_pointNumberToInsert[aindx];

                if (newPtNum == -1)
                    return;    // No points to insert.

                int originalTetrahedronNumber = aindx;
                Tetrahedron& st = m_tetrahedrons[originalTetrahedronNumber];

                // Work out where the 3 new tetrahedrons are going to go.
                int firstNewTetra = numOriginalTetrahedrons + (newPtNum * 3);
                newPtNum = m_activeTetraNum[newPtNum];

#ifdef VALIDATION_CHECK
                DTetrahedron3d dTet;
                st.GetDTetrahedron3d (dTet, m_points);
                if (!dTet.IsPointIn (m_points[newPtNum]))
                    newPtNum = newPtNum;

                for (int f = 0; f < numFacets; f++)
                    if (dTet.isOn (f, m_points[newPtNum]))
                        newPtNum = newPtNum;
#endif
                if (false)
                    {
                    // Create the 3 new tetrahedrons.
                    Tetrahedron firstNewT1 = Tetrahedron (newPtNum, st.ptNums[0], st.ptNums[1], st.ptNums[3], st.GetAdjentTet (2), firstNewTetra + 1, firstNewTetra + 2, originalTetrahedronNumber);
                    Tetrahedron firstNewT2 = Tetrahedron (newPtNum, st.ptNums[1], st.ptNums[2], st.ptNums[3], st.GetAdjentTet (0), firstNewTetra + 2, firstNewTetra, originalTetrahedronNumber);
                    Tetrahedron firstNewT3 = Tetrahedron (newPtNum, st.ptNums[0], st.ptNums[3], st.ptNums[2], st.GetAdjentTet (1), firstNewTetra + 1, originalTetrahedronNumber, firstNewTetra);

                    assert (st.ptNums[0] != st.ptNums[1] && st.ptNums[1] != st.ptNums[2] && st.ptNums[3] != st.ptNums[0] && st.ptNums[1] != st.ptNums[3] &&
                            st.ptNums[3] != st.ptNums[2] && st.ptNums[3] != st.ptNums[1] && st.ptNums[0] != st.ptNums[2] && st.ptNums[0] != newPtNum && st.ptNums[1] != newPtNum &&
                            st.ptNums[2] != newPtNum && st.ptNums[3] != newPtNum);
                    //don't create tetrahedrons where 3 of 4 points are collinear
                    DTetrahedron3d t1, t2, t3;
                    firstNewT1.GetDTetrahedron3d (t1, m_points);
                    firstNewT2.GetDTetrahedron3d (t2, m_points);
                    firstNewT3.GetDTetrahedron3d (t3, m_points);
                    bool isNonCollinear = (!Are3PointsCollinear (t1.points, 0.00001) && !Are3PointsCollinear (t1.points + 1, 0.00001) &&
                                           !Are3PointsCollinear (t2.points, 0.00001) && !Are3PointsCollinear (t2.points + 1, 0.00001) &&
                                           !Are3PointsCollinear (t3.points, 0.00001) && !Are3PointsCollinear (t3.points + 1, 0.00001));
                    assert (isNonCollinear);
                    }
                m_tetrahedrons[firstNewTetra] = Tetrahedron (newPtNum, st.ptNums[0], st.ptNums[1], st.ptNums[3], st.GetAdjentTet (2), firstNewTetra + 1, firstNewTetra + 2, originalTetrahedronNumber);
                m_tetrahedrons[firstNewTetra + 1] = Tetrahedron (newPtNum, st.ptNums[1], st.ptNums[2], st.ptNums[3], st.GetAdjentTet (0), firstNewTetra + 2, firstNewTetra, originalTetrahedronNumber);
                m_tetrahedrons[firstNewTetra + 2] = Tetrahedron (newPtNum, st.ptNums[0], st.ptNums[3], st.ptNums[2], st.GetAdjentTet (1), firstNewTetra + 1, originalTetrahedronNumber, firstNewTetra);
                st = Tetrahedron (newPtNum, st.ptNums[1], st.ptNums[0], st.ptNums[2], st.GetAdjentTet (3), firstNewTetra + 2, firstNewTetra + 1, firstNewTetra);
                m_tetrahedrons[firstNewTetra].SetStatus (128 | 64 | 32); // InternalOpp (0);
                //firstNewT1.SetInternalOpp (1);
                //firstNewT1.SetInternalOpp (2);
                m_tetrahedrons[firstNewTetra + 1].SetStatus (128 | 64 | 32); // InternalOpp (0);
                //firstNewT2.SetInternalOpp (0);
                //firstNewT2.SetInternalOpp (1);
                //firstNewT2.SetInternalOpp (2);
                m_tetrahedrons[firstNewTetra + 2].SetStatus (128 | 64 | 32); // InternalOpp (0);
                //firstNewT3.SetInternalOpp (0);
                //firstNewT3.SetInternalOpp (1);
                //firstNewT3.SetInternalOpp (2);
                m_tetrahedrons[firstNewTetra].isChanged = true;
                m_tetrahedrons[firstNewTetra + 1].isChanged = true;
                m_tetrahedrons[firstNewTetra + 2].isChanged = true;
                st.isChanged = true;
                st.special = false;

                st.SetStatus (128 | 64 | 32); // InternalOpp (0);
                //st.ClearStatus ();
                //st.SetInternalOpp (0);
                //st.SetInternalOpp (1);
                //st.SetInternalOpp (2);


#ifdef CHECKTETRAHEDRONS
                CheckTetrahedronPoints (st);
                CheckTetrahedronPoints (m_tetrahedrons[firstNewTetra]);
                CheckTetrahedronPoints (m_tetrahedrons[firstNewTetra + 1]);
                CheckTetrahedronPoints (m_tetrahedrons[firstNewTetra + 2]);
#endif
                });

            UpdateAdjectTetrahedronsAfterInsert (numOriginalTetrahedrons);
            TRACE_TETRA_END ()
#ifdef DHDEBUG
            //CheckTetrahedronForPoint ();
            //ValidateTetrahedrons ();
#endif
            //        std::cout << std::setprecision (8) << " elapsed_time=" << timer.read () << "(sec.)\n";
#ifdef DEBUGMSG
            //        printf ("Number of Tetrahedrons %d\n", m_tetrahedrons.size ());
#endif
            }

        struct edgeFaceSplit
            {
            int tetrahedronNum;
            int ptNum;
            bool onFace;
            int face;
            bvector<int> oldTetrahedrons;
            bvector<int> newTetrahedrons;
            bvector<PointOnEdge> otherPt;
            int ptA;
            int ptB;
            };

        bool Are3PointsCollinear (DPoint3d* triangle, double tolerance)
            {
            DVec3d v01 = DVec3d::FromStartEnd (triangle[0], triangle[1]), v02 = DVec3d::FromStartEnd (triangle[0], triangle[2]), v12 = DVec3d::FromStartEnd (triangle[1], triangle[2]);
            if (v01.Magnitude () == 0 || v02.Magnitude () == 0 || v12.Magnitude () == 0) return true;
            DVec3d cross;
            cross.CrossProduct (v02, v01);
            return (fabs (cross.Magnitude () / v12.Magnitude ()) < tolerance);
            }

        // This method needs to find 2-6 (eg on face) or N-(N*2), (on edge)
        //ToDo optimize, mark the internal edges as internal.
        bool SplitTetrahedronsOnEdgeOrFace ()
            {
            //ValidateTetrahedrons ();
            // Find all tetrahedrons that need to be split.
            std::vector<edgeFaceSplit> tetrahedronsToSplit;
            int numTetrahedrons = (int)m_tetrahedrons.size ();
            int numIgnored = 0;
            int numOnPoints = 0;
            int numOnEdge = 0;
            vector<bool> isMarked;
            isMarked.resize (m_tetrahedrons.size (), false);
            edgeFaceSplit splitInfo;
            for (size_t aindx = 0; aindx < m_tetrahedrons.size (); aindx++)
                {
                int newPtNum = m_pointNumberToInsert[aindx];
                if (newPtNum == -1)
                    continue;

                int originalTetrahedronNumber = (int)aindx;
                Tetrahedron& st = m_tetrahedrons[originalTetrahedronNumber];
                // Work out where the 3 new tetrahedrons are going to go.
                newPtNum = m_activeTetraNum[newPtNum];
                const DPoint3d& newPt = m_points[newPtNum];
                DTetrahedron3d dTetrahedron;
                st.GetDTetrahedron3d (dTetrahedron, m_points);

                // Clear splitInfo.
                splitInfo.newTetrahedrons.clear ();
                splitInfo.oldTetrahedrons.clear ();
                splitInfo.otherPt.clear ();
                int count = 0;
                int prevFace = 0;
                splitInfo.face = -1;
                // find the face this point is on.
                for (int f = 0; f < numFacets; f++)
                    {
                    if (dTetrahedron.isOn (f, newPt))
                        {
                        count++;
                        prevFace = splitInfo.face;
                        splitInfo.face = f;
                        }
                    }

                // This isn't on a face.
                if (count == 0)
                    continue;

                m_pointNumberToInsert[originalTetrahedronNumber] = -1;
                if (count == 3)
                    {
                    m_pointsAlreadyAdded++;
                    numOnPoints++;
                    continue;
                    }

                // find out if this is on a face.
                if (isMarked[originalTetrahedronNumber])// || tetrahedronsToSplit.size ())
                    {
                    m_pointsToInsert.push_back (newPtNum);
                    m_pointTetrahedron.push_back (originalTetrahedronNumber);
                    numIgnored++;
                    m_numPoints--;
                    //m_pointNumberToInsert[originalTetrahedronNumber] = -1;
                    continue;
                    }

                splitInfo.tetrahedronNum = originalTetrahedronNumber;
                splitInfo.ptNum = newPtNum;

                splitInfo.onFace = (count == 1);
                // see if this point is on an edge.
                if (splitInfo.onFace)
                    {
                    int adjTet = st.GetAdjentTet (splitInfo.face);

                    if (!m_skipFlipOnNearZero)
                        {
                        Tetrahedron& at = m_tetrahedrons[adjTet];
                        Orient ort = doOrient3D (at.ptNums[3], at.ptNums[2], at.ptNums[1], at.ptNums[0], m_points[at.ptNums[3]], m_points[at.ptNums[2]], m_points[at.ptNums[1]], m_points[at.ptNums[0]]);
                        if (OrientPos != ort)
                            isMarked[adjTet] = true;
                        }
                    if (isMarked[adjTet] || m_pointNumberToInsert[adjTet] != -1)
                        {
                        m_pointsToInsert.push_back (newPtNum);
                        m_pointTetrahedron.push_back (originalTetrahedronNumber);
                        m_numPoints--;
                        numIgnored++;
                        continue;
                        }
                    Tetrahedron& at = m_tetrahedrons[adjTet];
                    isMarked[originalTetrahedronNumber] = true;
                    isMarked[adjTet] = true;
                    splitInfo.oldTetrahedrons.push_back (adjTet);
                    splitInfo.newTetrahedrons.push_back (numTetrahedrons++);
                    splitInfo.newTetrahedrons.push_back (numTetrahedrons++);
                    splitInfo.newTetrahedrons.push_back (numTetrahedrons++);
                    splitInfo.newTetrahedrons.push_back (numTetrahedrons++);
                    }
                else if (count == 2)
                    {
                    numOnEdge++;

                    // need to find the edge that the point is on, using the 2 face indexes.
                    static const int edgeFinderHelper[4][4] =
                        {
                            { -1, 2, 1, 0 },
                            { 2, -1, 0, 1 },
                            { 2, 1, -1, 0 },
                            { 1, 2, 0, -1 },
                        };
                    const int* viewOfFace = Tetrahedron::GetDTetrahedron3dFacet (splitInfo.face);
                    int ptA = st.ptNums[viewOfFace[edgeFinderHelper[splitInfo.face][prevFace]]];
                    int ptB = st.ptNums[viewOfFace[(edgeFinderHelper[splitInfo.face][prevFace] + 1) % 3]];
#ifdef CHECKTETRAHEDRONS
                    if (orient2d (m_points[ptA], m_points[ptB], m_points[splitInfo.ptNum]) != 0)
                        {
                        for (int i = 0; i < 3; i++)
                            {
                            if (orient2d (m_points[st.ptNums[i]], m_points[st.ptNums[(i + 1) % 3]], m_points[splitInfo.ptNum]) == 0)
                                {
                                ptA = st.ptNums[viewOfFace[i]];
                                ptB = st.ptNums[viewOfFace[(i + 1) % 3]];
                                ptA = ptA;
                                }
                            }
                        ptA = ptA;
                        }
#endif
                    splitInfo.ptA = ptA;
                    splitInfo.ptB = ptB;

                    m_meshData.FindPointsAroundEdge (originalTetrahedronNumber, ptA, ptB, splitInfo.otherPt);
                    bool hasChanged = false;
                    for (auto pt : splitInfo.otherPt)
                        {
                        if (!m_skipFlipOnNearZero)
                            {
                            Tetrahedron& at = m_tetrahedrons[pt.tetraNum];
                            Orient ort = doOrient3D (at.ptNums[3], at.ptNums[2], at.ptNums[1], at.ptNums[0], m_points[at.ptNums[3]], m_points[at.ptNums[2]], m_points[at.ptNums[1]], m_points[at.ptNums[0]]);
                            if (OrientPos != ort)
                                {
                                hasChanged = true;
                                break;
                                }
                            }
                        if (isMarked[pt.tetraNum] || m_pointNumberToInsert[pt.tetraNum] != -1)
                            {
                            hasChanged = true;
                            break;
                            }
#ifdef VALIDATE
                        if (m_tetrahedrons[pt.tetraNum].GetPointIndex (ptA) == -1 || m_tetrahedrons[pt.tetraNum].GetPointIndex (ptB) == -1)
                            ptA = ptA;
#endif
                        splitInfo.oldTetrahedrons.push_back (pt.tetraNum);
                        }

                    if (hasChanged)
                        {
                        m_pointsToInsert.push_back (newPtNum);
                        m_pointTetrahedron.push_back (originalTetrahedronNumber);
                        numIgnored++;
                        m_numPoints--;
                        continue;
                        }

                    for (size_t i = 0; i < splitInfo.oldTetrahedrons.size (); i++)
                        {
                        isMarked[splitInfo.oldTetrahedrons[i]] = true;
                        splitInfo.newTetrahedrons.push_back (numTetrahedrons++);
                        }
                    }

                tetrahedronsToSplit.push_back (splitInfo);
                }

            if (numOnPoints == 0 && numIgnored == 0 && tetrahedronsToSplit.empty ())
                return false;

            if (!tetrahedronsToSplit.empty ())
                {
#ifdef DEBUGMSG
                std::cout << "SplitOnEdgeOrFace " << tetrahedronsToSplit.size () << " - " << numOnEdge << std::endl;
                std::cout << "Ignored " << numIgnored << "(" << numOnPoints << ")" << std::endl;
#endif
                m_pointsAlreadyAdded += (int)tetrahedronsToSplit.size ();
                m_tetrahedrons.resize (numTetrahedrons);
                bvector<int>splitPointsHelper;
                splitPointsHelper.resize (m_tetrahedrons.size (), -1);

                //struct movePointInfo
                //    {
                //    int possTets[2];
                //    };
                //            bvector<movePointInfo> movePtInfos;
                for (size_t sI = 0; sI < tetrahedronsToSplit.size (); sI++)
                    {
                    const edgeFaceSplit& info = tetrahedronsToSplit[sI];
                    if (info.onFace)
                        {
                        static const int ptInfo[6][4] = { { 5, 1, 2, 0 }, { 5, 2, 3, 0 }, { 5, 3, 1, 0 }, { 5, 2, 1, 4 }, { 5, 3, 2, 4 }, { 5, 1, 3, 4 } };
                        static const int newAdj[6][3] = { { 1, 2, 3 }, { 2, 0, 4 }, { 0, 1, 5 }, { 5, 4, 0 }, { 3, 5, 1 }, { 4, 3, 2 } };

                        Tetrahedron& st = m_tetrahedrons[info.tetrahedronNum];
                        Tetrahedron& at = m_tetrahedrons[info.oldTetrahedrons[0]];
                        int sFace = info.face;
                        int aFace = at.GetFaceSide (info.tetrahedronNum);
                        const int* sP = Tetrahedron::GetDTetrahedron3dFacet (sFace);
                        const int* aP = Tetrahedron::GetDTetrahedron3dFacet (aFace);

                        int ptNum[6] = { st.ptNums[sFace], st.ptNums[sP[0]], st.ptNums[sP[1]], st.ptNums[sP[2]], at.ptNums[aFace], info.ptNum };
                        int stA1 = st.GetAdjentTet (sP[0]);
                        int stA2 = st.GetAdjentTet (sP[1]);
                        int stA3 = st.GetAdjentTet (sP[2]);

                        //const int adjHelper[3] = { 0, 1, 2 };
                        //int ptIndex = 0;
                        //int atA1 = at.GetAdjentTet (aP[0]);
                        //int atA2 = at.GetAdjentTet (aP[1]);
                        //int atA3 = at.GetAdjentTet (aP[2]);
                        int aFacePts[3];
                        //// ToDo make quicker.
                        aFacePts[0] = ptNum[ptInfo[3][1]]; aFacePts[1] = ptNum[ptInfo[3][2]]; aFacePts[2] = ptNum[ptInfo[3][3]];
                        int atA1 = at.GetAdjentTet (at.GetFaceSide (aFacePts));
                        aFacePts[0] = ptNum[ptInfo[4][1]]; aFacePts[1] = ptNum[ptInfo[4][2]]; aFacePts[2] = ptNum[ptInfo[4][3]];
                        int atA2 = at.GetAdjentTet (at.GetFaceSide (aFacePts));
                        aFacePts[0] = ptNum[ptInfo[5][1]]; aFacePts[1] = ptNum[ptInfo[5][2]]; aFacePts[2] = ptNum[ptInfo[5][3]];
                        int atA3 = at.GetAdjentTet (at.GetFaceSide (aFacePts));

                        int newTets[6] = { info.tetrahedronNum, info.oldTetrahedrons[0], info.newTetrahedrons[0], info.newTetrahedrons[1], info.newTetrahedrons[2], info.newTetrahedrons[3] };
                        int oldTetNum[6] = { info.tetrahedronNum, info.tetrahedronNum, info.tetrahedronNum, info.oldTetrahedrons[0], info.oldTetrahedrons[0], info.oldTetrahedrons[0] };
                        int existingAdjTet[6] = { stA3, stA1, stA2, atA1, atA2, atA3 };

                        //movePointInfo movePtInfo;
                        //movePtInfo.possTets[0] = info.oldTetrahedrons[0];
                        //movePtInfo.possTets[1] = info.newTetrahedrons[0];
                        //m_pointNumberToInsert[info.tetrahedronNum] = (int)movePtInfos.size ();
                        //movePtInfos.push_back (movePtInfo);
                        //movePtInfo.possTets[0] = info.newTetrahedrons[1];
                        //movePtInfo.possTets[1] = info.newTetrahedrons[2];
                        //m_pointNumberToInsert[info.oldTetrahedrons[0]] = (int)movePtInfos.size ();
                        //movePtInfos.push_back (movePtInfo);
#ifdef DEBUGMSG
//                        std::cout << "Inserting point " << m_points[info.ptNum].x << " " << m_points[info.ptNum].y << " " << m_points[info.ptNum].z <<std::endl;
#endif
                        splitPointsHelper[info.tetrahedronNum] = (int)sI;
                        splitPointsHelper[info.oldTetrahedrons[0]] = (int)sI;
                        int fixFace[6];
                        for (int i = 0; i < 6; i++)
                            {
                            Tetrahedron&t = m_tetrahedrons[newTets[i]];
                            t.ptNums[0] = ptNum[ptInfo[i][0]];
                            t.ptNums[1] = ptNum[ptInfo[i][1]];
                            t.ptNums[2] = ptNum[ptInfo[i][2]];
                            t.ptNums[3] = ptNum[ptInfo[i][3]];

                            //std::cout << "SoF0 " << (int)m_points[t.ptNums[0]].x << " " << (int)m_points[t.ptNums[0]].y << std::endl;
                            //std::cout << "SoF1 " << (int)m_points[t.ptNums[1]].x << " " << (int)m_points[t.ptNums[1]].y << std::endl;
                            //std::cout << "SoF2 " << (int)m_points[t.ptNums[2]].x << " " << (int)m_points[t.ptNums[2]].y << std::endl;
                            //std::cout << "------------------------" << std::endl;
                            t.SetAdjentTet (0, existingAdjTet[i]);
                            fixFace[i] = m_tetrahedrons[existingAdjTet[i]].GetFaceSide (oldTetNum[i]);

                            t.SetAdjentTet (1, newTets[newAdj[i][0]]);
                            t.SetAdjentTet (2, newTets[newAdj[i][1]]);
                            t.SetAdjentTet (3, newTets[newAdj[i][2]]);
                            t.isActive = true; t.isChanged = true; t.ClearStatus ();
                            t.SetStatus (128 | 64 | 32);
#ifdef CHECKTETRAHEDRONS
                            CheckTetrahedronPoints (t);
#endif
                            }
                        for (int i = 0; i < 6; i++)
                            {
                            m_tetrahedrons[existingAdjTet[i]].SetAdjentTet (fixFace[i], newTets[i]);
                            }
                        }
                    else
                        {
                        // Need to split the tetrahedrons around the edge, we have the information.
                        bvector <int> adjA;
                        bvector <int> adjB;

                        for (auto pt : info.otherPt)
                            {
                            const Tetrahedron& t = m_tetrahedrons[pt.tetraNum];
                            adjA.push_back (t.GetAdjentTet (t.GetPointIndex (info.ptB)));
                            adjB.push_back (t.GetAdjentTet (t.GetPointIndex (info.ptA)));
                            }
                        for (size_t i = 0; i < info.otherPt.size (); i++)
                            {
                            // Do old Tetrahedron first.
                            size_t nextI = (i + 1) % info.otherPt.size ();
                            size_t prevI = (i + info.otherPt.size () - 1) % info.otherPt.size ();
                            Tetrahedron& oldTet = m_tetrahedrons[info.oldTetrahedrons[i]];
                            Tetrahedron& newTet = m_tetrahedrons[info.newTetrahedrons[i]];

                            oldTet.ptNums[0] = info.ptNum; // 2
                            oldTet.ptNums[2] = info.otherPt[nextI].ptNum; // 1
                            oldTet.ptNums[1] = info.otherPt[i].ptNum; // 0
                            oldTet.ptNums[3] = info.ptA; // 3

                            oldTet.SetAdjentTet (0, adjA[i]); // 2
                            oldTet.SetAdjentTet (2, info.oldTetrahedrons[prevI]); // 1
                            oldTet.SetAdjentTet (1, info.oldTetrahedrons[nextI]);  // 0
                            oldTet.SetAdjentTet (3, info.newTetrahedrons[i]); // 3
                            oldTet.isActive = true; oldTet.isChanged = true; oldTet.ClearStatus ();
                            oldTet.SetStatus (128 | 64 | 32);
#ifdef CHECKTETRAHEDRONS
                            CheckTetrahedronPoints (oldTet);
#endif
                            newTet.ptNums[0] = info.ptNum; // 2
                            newTet.ptNums[2] = info.ptB; // 1
                            newTet.ptNums[1] = info.otherPt[i].ptNum; // 0
                            newTet.ptNums[3] = info.otherPt[nextI].ptNum; // 3

                            newTet.SetAdjentTet (0, adjB[i]); // 2
                            newTet.SetAdjentTet (2, info.oldTetrahedrons[i]); // 1
                            newTet.SetAdjentTet (1, info.newTetrahedrons[nextI]);  // 0
                            newTet.SetAdjentTet (3, info.newTetrahedrons[prevI]); // 3
                            newTet.isActive = true; newTet.isChanged = true; newTet.ClearStatus ();
                            newTet.SetStatus (128 | 64 | 32);

                            //DPoint3d tri[3]{m_points[newTet.ptNums[0]], m_points[newTet.ptNums[1]], m_points[newTet.ptNums[2]]},
                            //    tri2[3]{m_points[newTet.ptNums[0]], m_points[newTet.ptNums[2]], m_points[newTet.ptNums[3]]},
                            //    tri3[3]{m_points[newTet.ptNums[0]], m_points[newTet.ptNums[1]], m_points[newTet.ptNums[3]]};
                            // assert(!Are3PointsCollinear(tri, 0.00001) && !Are3PointsCollinear(tri2, 0.00001)
                            //        && !Are3PointsCollinear(tri3, 0.00001));

                            int adjSide = m_tetrahedrons[adjB[i]].GetFaceSide (info.oldTetrahedrons[i]);
                            m_tetrahedrons[adjB[i]].SetAdjentTet (adjSide, info.newTetrahedrons[i]);

                            //movePointInfo movePtInfo;
                            //movePtInfo.possTets[0] = info.newTetrahedrons[i];
                            //movePtInfo.possTets[1] = -1;
                            //m_pointNumberToInsert[info.oldTetrahedrons[i]] = (int)movePtInfos.size ();
                            //movePtInfos.push_back (movePtInfo);
                            splitPointsHelper[info.oldTetrahedrons[i]] = (int)sI;
#ifdef CHECKTETRAHEDRONS
                            CheckTetrahedronPoints (newTet);
#endif
                            }
                        }
                    }
                // Need to move the points into the right segment.
                // Also need to move the failed points back to be processed.
                //                for (size_t i = 0; i < m_pointsToInsert.size (); i++)
                parallel_for (0, (int)m_pointsToInsert.size (), 1, [&](int i)
                    {
                    int originalTet = m_pointTetrahedron[i];
                    if (originalTet < 0)
                        return;
                    // ToDo improve this checking.
                    const int infoIndx = splitPointsHelper[originalTet];
                    if (infoIndx < 0)
                        return;
                    const DPoint3d& pt = m_points[m_pointsToInsert[i]];
                    const edgeFaceSplit& info = tetrahedronsToSplit[infoIndx];
                    bvector<int> tets = info.newTetrahedrons;
                    for (int t : info.oldTetrahedrons)
                        tets.push_back (t);
                    if (info.onFace)
                        tets.push_back (info.tetrahedronNum);
                    int newT = -1;
                    int nearestT = -1;
                    for (auto iT : tets)
                        {
                        int f;
                        const Tetrahedron& t = m_tetrahedrons[iT];
                        for (f = 0; f < 4; f++)
                            {
                            const int* sP = Tetrahedron::GetDTetrahedron3dFacet (f);

                            Orient ort = doOrient3DSoS (t.ptNums[sP[0]], t.ptNums[sP[1]], t.ptNums[sP[2]], m_pointsToInsert[i], m_points[t.ptNums[sP[0]]], m_points[t.ptNums[sP[1]]], m_points[t.ptNums[sP[2]]], pt);
                            if (OrientZero == ort)
                                {
                                nearestT = iT;

                                ort = doOrient3DSoS (t.ptNums[sP[0]], t.ptNums[sP[1]], t.ptNums[sP[2]], m_pointsToInsert[i], m_points[t.ptNums[sP[0]]], m_points[t.ptNums[sP[1]]], m_points[t.ptNums[sP[2]]], pt);
                                }

                            if (OrientNeg == ort)
                                break;
                            }
                        if (f == 4)
                            newT = iT;
                        if (newT != -1)
                            {
                            break;
                            }
                        }
                    if (newT == -1)
                        {
                        newT = nearestT;
                        }
                    m_pointTetrahedron[i] = newT;

                    /*
                    int movePtInfoIndx = splitPointsHelper[originalTet];
                    if (movePtInfoIndx == -1)
                    return;
                    //                const movePointInfo& info = movePtInfos[movePtInfoIndx];
                    const DPoint3d& pt = m_points[m_pointsToInsert[i]];
                    if (info.possTets[1] == -1) // Split into 2.
                    {
                    const Tetrahedron& t = m_tetrahedrons[originalTet];
                    int face = t.GetFaceSide (info.possTets[0]);
                    if (face == -1)
                    face = face;
                    const int* sP = Tetrahedron::GetDTetrahedron3dFacet (face);

                    const Orient ort = doOrient3DSoS (t.ptNums[sP[0]], t.ptNums[sP[1]], t.ptNums[sP[2]], m_pointsToInsert[i], m_points[t.ptNums[sP[0]]], m_points[t.ptNums[sP[1]]], m_points[t.ptNums[sP[2]]], pt);
                    if (OrientPos != ort)
                    m_pointTetrahedron[i] = info.possTets[0];
                    }

                    DTetrahedron3d dTet;
                    m_tetrahedrons[m_pointTetrahedron[i]].GetDTetrahedron3d (dTet, m_points);
                    if (!dTet.IsPointInOrOn (pt))
                    {
                    int newT = -1;
                    for (int iT = 0; iT < m_tetrahedrons.size (); iT++)
                    {
                    int f;
                    const Tetrahedron& t = m_tetrahedrons[iT];
                    for (f = 0; f < 4; f++)
                    {
                    const int* sP = Tetrahedron::GetDTetrahedron3dFacet (f);

                    const Orient ort = doOrient3DSoS (t.ptNums[sP[0]], t.ptNums[sP[1]], t.ptNums[sP[2]], m_pointsToInsert[i], m_points[t.ptNums[sP[0]]], m_points[t.ptNums[sP[1]]], m_points[t.ptNums[sP[2]]], pt);
                    if (OrientPos != ort)
                    break;
                    }
                    if (f == 4)
                    {
                    newT = iT;
                    break;
                    }
                    }
                    if (newT == -1)
                    newT = -1;
                    m_pointTetrahedron[i] = newT;
                    }
                    */
                    });
                }
            // Fix up m_point
            std::vector<int> oldActiveTetraNum = m_activeTetraNum;
            int n = 0;
            for (size_t i = 0; i < m_pointNumberToInsert.size (); i++)
                {
                if (m_pointNumberToInsert[i] != -1)
                    {
                    m_activeTetraNum[n] = oldActiveTetraNum[m_pointNumberToInsert[i]];
                    m_pointNumberToInsert[i] = n++;
                    }
                }
            m_activeTetraNum.resize (n);
            m_pointNumberToInsert.resize (m_tetrahedrons.size (), -1);
            //ValidateTetrahedrons ();
            return true;
            }

#ifdef CHECKTETRAHEDRONS
            void CheckTetrahedronPoints  (Tetrahedron& tet)
                {
                bool failed = false;
                if (tet.ptNums[0] == m_infPtNum)
                    failed = true;
                Orient ort = doOrient3D (tet.ptNums[3], tet.ptNums[2], tet.ptNums[1], tet.ptNums[0], m_points[tet.ptNums[3]], m_points[tet.ptNums[2]], m_points[tet.ptNums[1]], m_points[tet.ptNums[0]]);
                if (OrientPos != ort)
                    failed = true;
                }
#endif

        // UpdateAdjectTetrahedronsAfterInsert helper function
        inline void FixupToSplitIndex (Tetrahedron& st, int originalIndex, int newIndex)
            {
            if (st.GetAdjentTet (0) != -1)
                {
                Tetrahedron& oldTet = m_tetrahedrons[st.GetAdjentTet (0)];

                if (oldTet.GetAdjentTet (0) == originalIndex)
                    {
                    oldTet.SetAdjentTet (0, newIndex);
                    //st.GetAdjentTet(0) = st.GetAdjentTet(0);
                    return;
                    }

                for (int i = 1; i < 4; i++)  // first one is to the external face.
                    {
                    int newTetIndex = oldTet.GetAdjentTet (i);
                    Tetrahedron& newTet = m_tetrahedrons[newTetIndex];
                    if (newTet.GetAdjentTet (0) == originalIndex)
                        {
                        newTet.SetAdjentTet (0, newIndex);
                        st.SetAdjentTet (0, newTetIndex);
                        return;
                        }
                    }
                std::cout << "Failed to find Index " << originalIndex << "," << newIndex << "," << std::endl;
                st.SetAdjentTet (0, -2);
                }
            }

        // Update Adjectent tetrahedrons after an insert.
        void UpdateAdjectTetrahedronsAfterInsert (int numOriginalTetrahedrons)
            {
            AutoTimer timer (m_timeToFixUpAdjectEdges);
            int numberOfTetrahedrons = (int)m_tetrahedrons.size ();
            // Go Through all tetrahedrons, and sort out adjencny.
            parallel_for ((int)0, (int)numOriginalTetrahedrons, (int)1, [&](int aindx)
                {
                if (m_pointNumberToInsert[aindx] == -1)
                    return;
                Tetrahedron& st = m_tetrahedrons[aindx];
                int adjentTet = st.GetAdjentTet (0);

                if (adjentTet <= -1 || (adjentTet < numOriginalTetrahedrons && m_pointNumberToInsert[adjentTet] == -1))
                    {
                    // Dont need to do anything.
                    }
                else if (aindx < adjentTet)
                    {
                    FixupToSplitIndex (st, aindx, aindx);
                    }

                for (int face = 1; face < 4; face++)  // the 1st Face is extenal so ignore
                    {
                    Tetrahedron& newSt = m_tetrahedrons[st.GetAdjentTet (face)];
                    int adjentTet = newSt.GetAdjentTet (0);

                    if (adjentTet == -1)
                        continue;
                    if (adjentTet < numOriginalTetrahedrons && m_pointNumberToInsert[adjentTet] == -1)
                        {
                        // Need to change the index to this one.
                        Tetrahedron& adjTet = m_tetrahedrons[adjentTet];

                        for (int face2 = 0; face2 < numFacets; face2++)
                            {
                            if (adjTet.GetAdjentTet (face2) == aindx)
                                {
                                adjTet.SetAdjentTet (face2, st.GetAdjentTet (face));
                                break;
                                }
                            }
                        }
                    else if (aindx < adjentTet && adjentTet < numOriginalTetrahedrons)
                        {
                        FixupToSplitIndex (newSt, aindx, st.GetAdjentTet (face));
                        }
                    }
                });
            //ValidateTetrahedrons ();
            }

        // Is the tetrahedron on this edge delauny. >= 0 is delauny.
        inline double IsEdgeDelauny (const DTetrahedron3d& t1, const DPoint3d& pt)
            {
            if (m_infPtNum != -1)
                {
                if (pt.IsEqual (m_points[m_infPtNum]))
                    return 0;
                if (t1.points[0].IsEqual (m_points[m_infPtNum]))
                    return 0;
                if (t1.points[1].IsEqual (m_points[m_infPtNum]))
                    return 0;
                if (t1.points[2].IsEqual (m_points[m_infPtNum]))
                    return 0;
                if (t1.points[3].IsEqual (m_points[m_infPtNum]))
                    return 0;
                }
            //if (t1.isOn (0, t1.points[0]))
            //    return 0;
            double result = insphere (t1.points[0], t1.points[1], t1.points[2], t1.points[3], pt);
            return result;
            }

        inline bool IsSpecialTet (const Tetrahedron& t) const
            {
            if (t.ptNums[0] == m_infPtNum)
                return true;
            if (t.ptNums[1] == m_infPtNum)
                return true;
            if (t.ptNums[2] == m_infPtNum)
                return true;
            if (t.ptNums[3] == m_infPtNum)
                return true;
            return false;
            }

        Side doInSphereSoSOnly (double*predData, int pi0, int pi1, int pi2, int pi3, int pi4,
                                DPoint3dCR p0, DPoint3dCR p1, DPoint3dCR p2, DPoint3dCR p3, DPoint3dCR p4
                                ) const
            {
            const int DIM = 4;
            const int NUM = DIM + 1;

            // Sort indices & note their order
            const double* ip[NUM] = { &p0.x, &p1.x, &p2.x, &p3.x, &p4.x };
            int swapCount = 0;

            if (pi0 > pi4)
                {
                std::swap (pi0, pi4); std::swap (ip[0], ip[4]); ++swapCount;
                }
            if (pi1 > pi3)
                {
                std::swap (pi1, pi3); std::swap (ip[1], ip[3]); ++swapCount;
                }
            if (pi0 > pi2)
                {
                std::swap (pi0, pi2); std::swap (ip[0], ip[2]); ++swapCount;
                }
            if (pi2 > pi4)
                {
                std::swap (pi2, pi4); std::swap (ip[2], ip[4]); ++swapCount;
                }
            if (pi0 > pi1)
                {
                std::swap (pi0, pi1); std::swap (ip[0], ip[1]); ++swapCount;
                }
            if (pi2 > pi3)
                {
                std::swap (pi2, pi3); std::swap (ip[2], ip[3]); ++swapCount;
                }
            if (pi1 > pi4)
                {
                std::swap (pi1, pi4); std::swap (ip[1], ip[4]); ++swapCount;
                }
            if (pi1 > pi2)
                {
                std::swap (pi1, pi2); std::swap (ip[1], ip[2]); ++swapCount;
                }
            if (pi3 > pi4)
                {
                std::swap (pi3, pi4); std::swap (ip[3], ip[4]); ++swapCount;
                }

            // Note: This is a check placed only to overcome a CUDA compiler bug!
            // As of CUDA 4 and sm_21, this bug happens if there are many SoS checks
            // caused by co-planar input points. Comment this if you wish, but do NOT
            // delete it, so it can be turned on when needed.
            //if ( ( p0 == p1 ) || ( p1 == p2 ) || ( p2 == p3 ) || ( p3 == p4 ) )
            //    printf( "Duplicate!\n" );

            /* assert ((pi0 != pi1) && (pi0 != pi2) && (pi0 != pi3) && (pi0 != pi4)
            && (pi1 != pi2) && (pi1 != pi3) && (pi1 != pi4)
            && (pi2 != pi3) && (pi2 != pi4)
            && (pi3 != pi4)
            && "Duplicate indices in SoS orientation!");*/

            // Calculate determinants

            double op[NUM - 1][DIM - 1] = { 0 };
            Orient orient = OrientZero;
            int depth = 0;

            // Setup determinants
            while (OrientZero == orient)
                {
                ++depth;    // Increment depth, begins from 1

                bool lifted = false;

                switch (depth)
                    {
                    case 0:
                        //CudaAssert( false && "Depth cannot be ZERO! This happens only for non-ZERO determinant!" );
                        break;

                    case 1:
                        op[0][0] = ip[1][0];    op[0][1] = ip[1][1];    op[0][2] = ip[1][2];
                        op[1][0] = ip[2][0];    op[1][1] = ip[2][1];    op[1][2] = ip[2][2];
                        op[2][0] = ip[3][0];    op[2][1] = ip[3][1];    op[2][2] = ip[3][2];
                        op[3][0] = ip[4][0];    op[3][1] = ip[4][1];    op[3][2] = ip[4][2];
                        break;

                    case 2: lifted = true;
                        //op[0][0] = ip[1][0];    op[0][1] = ip[1][1];    op[0][2] = ip[1][2];
                        //op[1][0] = ip[2][0];    op[1][1] = ip[2][1];    op[1][2] = ip[2][2];
                        //op[2][0] = ip[3][0];    op[2][1] = ip[3][1];    op[2][2] = ip[3][2];
                        //op[3][0] = ip[4][0];    op[3][1] = ip[4][1];    op[3][2] = ip[4][2];
                        break;

                    case 3: lifted = true;
                        /*op[0][0] = ip[1][0];*/    op[0][1] = ip[1][2];    op[0][2] = ip[1][1];
                        /*op[1][0] = ip[2][0];*/    op[1][1] = ip[2][2];    op[1][2] = ip[2][1];
                        /*op[2][0] = ip[3][0];*/    op[2][1] = ip[3][2];    op[2][2] = ip[3][1];
                        /*op[3][0] = ip[4][0];*/    op[3][1] = ip[4][2];    op[3][2] = ip[4][1];
                        break;

                    case 4: lifted = true;
                        // We know this is hit sometime
                        op[0][0] = ip[1][1];    /*op[0][1] = ip[1][2];*/    op[0][2] = ip[1][0];
                        op[1][0] = ip[2][1];    /*op[1][1] = ip[2][2];*/    op[1][2] = ip[2][0];
                        op[2][0] = ip[3][1];    /*op[2][1] = ip[3][2];*/    op[2][2] = ip[3][0];
                        op[3][0] = ip[4][1];    /*op[3][1] = ip[4][2];*/    op[3][2] = ip[4][0];
                        break;

                    case 5:
                        ////CudaAssert( false && "Here!" );
                        op[0][0] = ip[0][0];    op[0][1] = ip[0][1];    op[0][2] = ip[0][2];
                        op[1][0] = ip[2][0];    op[1][1] = ip[2][1];    op[1][2] = ip[2][2];
                        op[2][0] = ip[3][0];    op[2][1] = ip[3][1];    op[2][2] = ip[3][2];
                        op[3][0] = ip[4][0];    op[3][1] = ip[4][1];    op[3][2] = ip[4][2];
                        break;

                    case 6:
                        ////CudaAssert( false && "Here!" );
                        op[0][0] = ip[2][0];    op[0][1] = ip[2][1];
                        op[1][0] = ip[3][0];    op[1][1] = ip[3][1];
                        op[2][0] = ip[4][0];    op[2][1] = ip[4][1];
                        break;

                    case 7:
                        ////CudaAssert( false && "Here!" );
                        /*op[0][0] = ip[2][0];*/    op[0][1] = ip[2][2];
                        /*op[1][0] = ip[3][0];*/    op[1][1] = ip[3][2];
                        /*op[2][0] = ip[4][0];*/    op[2][1] = ip[4][2];
                        break;

                    case 8:
                        ////CudaAssert( false && "Here!" );
                        op[0][0] = ip[2][1];    /*op[0][1] = ip[2][2];*/
                        op[1][0] = ip[3][1];    /*op[1][1] = ip[3][2];*/
                        op[2][0] = ip[4][1];    /*op[2][1] = ip[4][2];*/
                        break;

                    case 9: lifted = true;
                        ////CudaAssert( false && "Here!" );
                        op[0][0] = ip[0][0];    op[0][1] = ip[0][1];    op[0][2] = ip[0][2];
                        op[1][0] = ip[2][0];    op[1][1] = ip[2][1];    op[1][2] = ip[2][2];
                        op[2][0] = ip[3][0];    op[2][1] = ip[3][1];    op[2][2] = ip[3][2];
                        op[3][0] = ip[4][0];    op[3][1] = ip[4][1];    op[3][2] = ip[4][2];
                        break;

                    case 10: lifted = true;
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[2][0];    op[0][1] = ip[2][1];    op[0][2] = ip[2][2];
                        op[1][0] = ip[3][0];    op[1][1] = ip[3][1];    op[1][2] = ip[3][2];
                        op[2][0] = ip[4][0];    op[2][1] = ip[4][1];    op[2][2] = ip[4][2];
                        break;

                    case 11: lifted = true;
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[2][1];    op[0][1] = ip[2][0];    /*op[0][2] = ip[2][2];*/
                        op[1][0] = ip[3][1];    op[1][1] = ip[3][0];    /*op[1][2] = ip[3][2];*/
                        op[2][0] = ip[4][1];    op[2][1] = ip[4][0];    /*op[2][2] = ip[4][2];*/
                        break;

                    case 12: lifted = true;
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[0][0];    op[0][1] = ip[0][2];    op[0][2] = ip[0][1];
                        op[1][0] = ip[2][0];    op[1][1] = ip[2][2];    op[1][2] = ip[2][1];
                        op[2][0] = ip[3][0];    op[2][1] = ip[3][2];    op[2][2] = ip[3][1];
                        op[3][0] = ip[4][0];    op[3][1] = ip[4][2];    op[3][2] = ip[4][1];
                        break;

                    case 13: lifted = true;
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[2][2];    op[0][1] = ip[2][0];    op[0][2] = ip[2][1];
                        op[1][0] = ip[3][2];    op[1][1] = ip[3][0];    op[1][2] = ip[3][1];
                        op[2][0] = ip[4][2];    op[2][1] = ip[4][0];    op[2][2] = ip[4][1];
                        break;

                    case 14: lifted = true;
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[0][1];    op[0][1] = ip[0][2];    op[0][2] = ip[0][0];
                        op[1][0] = ip[2][1];    op[1][1] = ip[2][2];    op[1][2] = ip[2][0];
                        op[2][0] = ip[3][1];    op[2][1] = ip[3][2];    op[2][2] = ip[3][0];
                        op[3][0] = ip[4][1];    op[3][1] = ip[4][2];    op[3][2] = ip[4][0];
                        break;

                    case 15:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[0][0];    op[0][1] = ip[0][1];    op[0][2] = ip[0][2];
                        op[1][0] = ip[1][0];    op[1][1] = ip[1][1];    op[1][2] = ip[1][2];
                        op[2][0] = ip[3][0];    op[2][1] = ip[3][1];    op[2][2] = ip[3][2];
                        op[3][0] = ip[4][0];    op[3][1] = ip[4][1];    op[3][2] = ip[4][2];
                        break;

                    case 16:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[1][0];    op[0][1] = ip[1][1];
                        op[1][0] = ip[3][0];    op[1][1] = ip[3][1];
                        op[2][0] = ip[4][0];    op[2][1] = ip[4][1];
                        break;

                    case 17:
                        //CudaAssert( false && "Here!" );
                        /*op[0][0] = ip[1][0];*/    op[0][1] = ip[1][2];
                        /*op[1][0] = ip[3][0];*/    op[1][1] = ip[3][2];
                        /*op[2][0] = ip[4][0];*/    op[2][1] = ip[4][2];
                        break;

                    case 18:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[1][1];    /*op[0][1] = ip[1][2];*/
                        op[1][0] = ip[3][1];    /*op[1][1] = ip[3][2];*/
                        op[2][0] = ip[4][1];    /*op[2][1] = ip[4][2];*/
                        break;

                    case 19:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[0][0];    op[0][1] = ip[0][1];
                        op[1][0] = ip[3][0];    op[1][1] = ip[3][1];
                        op[2][0] = ip[4][0];    op[2][1] = ip[4][1];
                        break;

                    case 20:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[3][0];
                        op[1][0] = ip[4][0];
                        break;

                    case 21:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[3][1];
                        op[1][0] = ip[4][1];
                        break;

                    case 22:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[0][0];    op[0][1] = ip[0][2];
                        op[1][0] = ip[3][0];    op[1][1] = ip[3][2];
                        op[2][0] = ip[4][0];    op[2][1] = ip[4][2];
                        break;

                    case 23:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[3][2];
                        op[1][0] = ip[4][2];
                        break;

                    case 24:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[0][1];    op[0][1] = ip[0][2];
                        op[1][0] = ip[3][1];    op[1][1] = ip[3][2];
                        op[2][0] = ip[4][1];    op[2][1] = ip[4][2];
                        break;

                    case 25: lifted = true;
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[0][0];    op[0][1] = ip[0][1];    op[0][2] = ip[0][2];
                        op[1][0] = ip[1][0];    op[1][1] = ip[1][1];    op[1][2] = ip[1][2];
                        op[2][0] = ip[3][0];    op[2][1] = ip[3][1];    op[2][2] = ip[3][2];
                        op[3][0] = ip[4][0];    op[3][1] = ip[4][1];    op[3][2] = ip[4][2];
                        break;

                    case 26: lifted = true;
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[1][0];    op[0][1] = ip[1][1];    op[0][2] = ip[1][2];
                        op[1][0] = ip[3][0];    op[1][1] = ip[3][1];    op[1][2] = ip[3][2];
                        op[2][0] = ip[4][0];    op[2][1] = ip[4][1];    op[2][2] = ip[4][2];
                        break;

                    case 27: lifted = true;
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[1][1];    op[0][1] = ip[1][0];    /*op[0][2] = ip[1][2];*/
                        op[1][0] = ip[3][1];    op[1][1] = ip[3][0];    /*op[1][2] = ip[3][2];*/
                        op[2][0] = ip[4][1];    op[2][1] = ip[4][0];    /*op[2][2] = ip[4][2];*/
                        break;

                    case 28: lifted = true;
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[0][0];    op[0][1] = ip[0][1];    op[0][2] = ip[0][2];
                        op[1][0] = ip[3][0];    op[1][1] = ip[3][1];    op[1][2] = ip[3][2];
                        op[2][0] = ip[4][0];    op[2][1] = ip[4][1];    op[2][2] = ip[4][2];
                        break;

                    case 29: lifted = true;
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[3][0];    op[0][1] = ip[3][1];    op[0][2] = ip[3][2];
                        op[1][0] = ip[4][0];    op[1][1] = ip[4][1];    op[1][2] = ip[4][2];
                        break;

                    case 30: lifted = true;
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[0][1];    op[0][1] = ip[0][0];    op[0][2] = ip[0][2];
                        op[1][0] = ip[3][1];    op[1][1] = ip[3][0];    op[1][2] = ip[3][2];
                        op[2][0] = ip[4][1];    op[2][1] = ip[4][0];    op[2][2] = ip[4][2];
                        break;

                    case 31: lifted = true;
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[0][0];    op[0][1] = ip[0][2];    op[0][2] = ip[0][1];
                        op[1][0] = ip[1][0];    op[1][1] = ip[1][2];    op[1][2] = ip[1][1];
                        op[2][0] = ip[3][0];    op[2][1] = ip[3][2];    op[2][2] = ip[3][1];
                        op[3][0] = ip[4][0];    op[3][1] = ip[4][2];    op[3][2] = ip[4][1];
                        break;

                    case 32: lifted = true;
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[1][2];    op[0][1] = ip[1][0];    op[0][2] = ip[1][1];
                        op[1][0] = ip[3][2];    op[1][1] = ip[3][0];    op[1][2] = ip[3][1];
                        op[2][0] = ip[4][2];    op[2][1] = ip[4][0];    op[2][2] = ip[4][1];
                        break;

                    case 33: lifted = true;
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[0][2];    op[0][1] = ip[0][0];    op[0][2] = ip[0][1];
                        //op[1][0] = ip[3][2];    op[1][1] = ip[3][0];    op[1][2] = ip[3][1];
                        //op[2][0] = ip[4][2];    op[2][1] = ip[4][0];    op[2][2] = ip[4][1];
                        break;

                    case 34: lifted = true;
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[0][1];    op[0][1] = ip[0][2];    op[0][2] = ip[0][0];
                        op[1][0] = ip[1][1];    op[1][1] = ip[1][2];    op[1][2] = ip[1][0];
                        op[2][0] = ip[3][1];    op[2][1] = ip[3][2];    op[2][2] = ip[3][0];
                        op[3][0] = ip[4][1];    op[3][1] = ip[4][2];    op[3][2] = ip[4][0];
                        break;

                    case 35:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[0][0];    op[0][1] = ip[0][1];    op[0][2] = ip[0][2];
                        op[1][0] = ip[1][0];    op[1][1] = ip[1][1];    op[1][2] = ip[1][2];
                        op[2][0] = ip[2][0];    op[2][1] = ip[2][1];    op[2][2] = ip[2][2];
                        op[3][0] = ip[4][0];    op[3][1] = ip[4][1];    op[3][2] = ip[4][2];
                        break;

                    case 36:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[1][0];    op[0][1] = ip[1][1];
                        op[1][0] = ip[2][0];    op[1][1] = ip[2][1];
                        op[2][0] = ip[4][0];    op[2][1] = ip[4][1];
                        break;

                    case 37:
                        //CudaAssert( false && "Here!" );
                        /*op[0][0] = ip[1][0];*/    op[0][1] = ip[1][2];
                        /*op[1][0] = ip[2][0];*/    op[1][1] = ip[2][2];
                        /*op[2][0] = ip[4][0];*/    op[2][1] = ip[4][2];
                        break;

                    case 38:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[1][1];    /*op[0][1] = ip[1][2];*/
                        op[1][0] = ip[2][1];    /*op[1][1] = ip[2][2];*/
                        op[2][0] = ip[4][1];    /*op[2][1] = ip[4][2];*/
                        break;

                    case 39:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[0][0];    op[0][1] = ip[0][1];
                        op[1][0] = ip[2][0];    op[1][1] = ip[2][1];
                        op[2][0] = ip[4][0];    op[2][1] = ip[4][1];
                        break;

                    case 40:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[2][0];
                        op[1][0] = ip[4][0];
                        break;

                    case 41:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[2][1];
                        op[1][0] = ip[4][1];
                        break;

                    case 42:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[0][0];    op[0][1] = ip[0][2];
                        op[1][0] = ip[2][0];    op[1][1] = ip[2][2];
                        op[2][0] = ip[4][0];    op[2][1] = ip[4][2];
                        break;

                    case 43:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[2][2];
                        op[1][0] = ip[4][2];
                        break;

                    case 44:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[0][1];    op[0][1] = ip[0][2];
                        op[1][0] = ip[2][1];    op[1][1] = ip[2][2];
                        op[2][0] = ip[4][1];    op[2][1] = ip[4][2];
                        break;

                    case 45:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[0][0];    op[0][1] = ip[0][1];
                        op[1][0] = ip[1][0];    op[1][1] = ip[1][1];
                        op[2][0] = ip[4][0];    op[2][1] = ip[4][1];
                        break;

                    case 46:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[1][0];
                        op[1][0] = ip[4][0];
                        break;

                    case 47:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[1][1];
                        op[1][0] = ip[4][1];
                        break;

                    case 48:
                        //CudaAssert( false && "Here!" );
                        op[0][0] = ip[0][0];
                        op[1][0] = ip[4][0];
                        break;

                    case 49:
                        // See below for result
                        break;

                    default:
                        assert (false && "Invalid SoS depth!");
                        break;

                    }   // switch ( depth )

                // *** Calculate determinant

                switch (depth)
                    {
                    // 3D orientation determinant
                    case 1: case 5: case 15: case 35:
                        // 3D orientation involving the lifted coordinate
                    case 2: case 3: case 4: case 9: case 12: case 14: case 25: case 31: case 34:
                        orient = sphToOrient (orient3dFastExact_Lifted (predData, op[0], op[1], op[2], op[3], lifted));
                        break;

                        // 2D orientation determinant
                    case 6: case 7: case 8: case 16: case 17: case 18: case 19: case 22: case 24:
                    case 36: case 37: case 38: case 39: case 42: case 44: case 45:
                        // 2D orientation involving the lifted coordinate
                    case 10: case 11: case 13: case 26: case 27: case 28: case 30: case 32: case 33:
                        orient = sphToOrient (orient2dexact_lifted (op[0], op[1], op[2], lifted));
                        break;

                        // 1D orientation determinant
                    case 20: case 21: case 23: case 40: case 41: case 43: case 46: case 47: case 48:
                        // 1D orientation involving the lifted coordinate
                    case 29:
                        orient = sphToOrient (orient1dExact_Lifted (op[0], op[1], lifted));
                        break;

                    case 49:
                        // Last depth, always POS
                        orient = OrientPos;
                        break;

                    default:
                        assert (false && "Invalid SoS depth!");
                        break;
                    }
                }   // while ( 0 == orient )

            ////
            // Flip result for certain depths. (See SoS paper.)
            ////

            switch (depth)
                {
                // -ve result
                case 1: case 3: case 7: case 9: case 11: case 14: case 16: case 15: case 18:
                case 20: case 22: case 23: case 26: case 30: case 31: case 32: case 37: case 39:
                case 41: case 44: case 46:
                    orient = flipOrient (orient);
                    break;

                default:
                    // Do nothing
                    break;
                }

            ////
            // Flip result for odd swap count
            ////

            if ((swapCount % 2) != 0)
                orient = flipOrient (orient);

            return (Side)orient;
            }

        inline bool CollectFacesToTests (int oppVert[4], const Tetrahedron& flipTetrahedron, FaceEdgeStack& FaceEdgeStack)
            {
            for (int botVi = 0; botVi < 4; ++botVi)
                {
                const int topVert = oppVert[botVi];

                if (topVert < 0) continue;
                //*** Check for 3-2 flip

                const int* botOrdVi = TetViAsSeenFrom[botVi]; // Order bottom tetra as seen from apex vertex
                int i = 0;


                for (; i < 3; ++i) // Check 3 sides of bottom-top tetra
                    {
                    const int sideVert = oppVert[botOrdVi[i]];

                    // More than 3 tetra around edge
                    if (sideVert != topVert && sideVert != makeNegative (topVert)) continue;

                    // 3-2 flip is possible.
                    //setBitState( skip, botOrdVi[ i ], true );
                    break;
                    }
                FaceEdgeStack.Push (botVi, i);
                }

            return false;
            }

        inline Side doInSphere (
            const Tetrahedron& tet, int vert, const DTetrahedron3d& dtet, const DPoint3d& ptVert) const
            {
            if (vert == m_infPtNum)
                return SideOut;

            double det;

            for (int i = 0; i < 4; i++)
                {
                if (tet.ptNums[i] == m_infPtNum)
                    {
                    const int* ord = TetViAsSeenFrom[i];

                    // Check convexity, looking from inside
                    det = -orient3dfast (dtet.points[ord[0]], dtet.points[ord[2]], dtet.points[ord[1]], ptVert);
                    return sphToSide (det);
                    }
                }

            det = insphereFast (dtet.points[0], dtet.points[1], dtet.points[2], dtet.points[3], ptVert);

            return sphToSide (det);
            }

        inline Side doInSphereSoS (
            const Tetrahedron& tet, int vert, const DTetrahedron3d& dtet, DPoint3dCR ptVert) const
            {
            if (vert == m_infPtNum) // || vert >= m_firstAddedPtNum)
                return SideOut;

            const DPoint3d* pt = &dtet.points[0];
            //double det;

            for (int i = 0; i < 4; i++)
                {
                if (tet.ptNums[i] == m_infPtNum)
                    {
                    const int* ord = TetViAsSeenFrom[i];

                    // Check convexity, looking from inside
                    const Orient ort = doOrient3DSoS (
                        tet.ptNums[ord[0]], tet.ptNums[ord[2]], tet.ptNums[ord[1]], vert,
                        pt[ord[0]], pt[ord[2]], pt[ord[1]], ptVert);

                    return sphToSide (ort);
                    }
                }

            //const int curPredDataIdx = getCurThreadIdx () * PredicateTotalSize;
            double curPredData[PredicateTotalSize]; // = &_predData[curPredDataIdx];

            //                        const Side s0 = doInSphereFastAdaptExact (pt[0], pt[1], pt[2], pt[3], ptVert);
            const double det1 = insphereFastAdaptExact (curPredData, &pt[0].x, &pt[1].x, &pt[2].x, &pt[3].x, &ptVert.x);
            const Side s0 = sphToSide (det1);

            if (SideZero != s0)
                return s0;

            const Side s2 = doInSphereSoSOnly (curPredData, tet.ptNums[0], tet.ptNums[1], tet.ptNums[2], tet.ptNums[3], vert,
                                               pt[0], pt[1], pt[2], pt[3], ptVert);
            return s2;
            }

        inline bool TestFlip32 (const Tetrahedron& botTet, int botTetIdx, int botTetVi, int botCorOrdVi, int& flip44Triangle)
            {
            flip44Triangle = -1;
            // Top tetra
            const int topTetIdx = botTet.GetAdjentTet (botTetVi);  //botOpp.getOppTet (botTetVi);
            Tetrahedron& topTet = m_tetrahedrons[topTetIdx]; // loadTet (tetArr, topTetIdx);
            const int topTetVi = topTet.GetFaceSide (botTetIdx); // botOpp.getOppVi (botTetVi);

            const int* corVi = TetViAsSeenFrom[botTetVi];
            const int* corTopVi = TetViAsSeenFrom[topTetVi];

            // Side tetra
            const int botCorVi = corVi[botCorOrdVi];

            // BotVi
            const int botAVi = corVi[(botCorOrdVi + 1) % 3];
            const int botBVi = corVi[(botCorOrdVi + 2) % 3];
            const int botA = botTet.ptNums[botAVi];
            const int botB = botTet.ptNums[botBVi];

            // Top vi
            const int botCor = botTet.ptNums[botCorVi];

            // Vertices of old 3 tetra
            const int botTetV = botTet.ptNums[botTetVi];
            const int topTetV = topTet.ptNums[topTetVi];
            bool isOrtAZero = false;
            Orient ortA = m_skipFlipOnNearZero ? doOrient3D (botCor, topTetV, botTetV, botA, m_points[botCor], m_points[topTetV], m_points[botTetV], m_points[botA]) :
                doOrient3DSoS (botCor, topTetV, botTetV, botA, m_points[botCor], m_points[topTetV], m_points[botTetV], m_points[botA]);

            if (OrientZero == ortA)
                {
                isOrtAZero = true;
                ortA = doOrient3DSoS (botCor, topTetV, botTetV, botA, m_points[botCor], m_points[topTetV], m_points[botTetV], m_points[botA]);
                }
            if (OrientPos != ortA)
                return false;

            bool isOrtBZero = false;
            Orient ortB = m_skipFlipOnNearZero ? doOrient3D (botCor, botTetV, topTetV, botB, m_points[botCor], m_points[botTetV], m_points[topTetV], m_points[botB]) :
                doOrient3DSoS (botCor, botTetV, topTetV, botB, m_points[botCor], m_points[botTetV], m_points[topTetV], m_points[botB]);

            if (OrientZero == ortB)
                {
                isOrtBZero = true;
                ortB = doOrient3DSoS (botCor, botTetV, topTetV, botB, m_points[botCor], m_points[botTetV], m_points[topTetV], m_points[botB]);
                }
            if (OrientPos != ortB)
                return false;

            // If both are zero skip.
            if (isOrtAZero && isOrtBZero)
                return false;

            if (isOrtAZero)
                {
                flip44Triangle = botTet.GetAdjentTet (botBVi);
                }
            else if (isOrtBZero)
                {
                flip44Triangle = botTet.GetAdjentTet (botAVi);
                }
             return true;
            }

        // Examine the facets for flipping.
        void ExamineFacetsForFlip (std::vector<FlipInformation>& flips, bool& hasSpecial)
            {
            AutoTimer timer (m_timeToExamineFacetsForFlip);

            if (!m_inSpecialFlip)
                {
                m_flipTestNumber.clear ();
                m_flipTestNumber.resize (m_tetrahedrons.size (), 0xffffffff);
                }
            parallel_for ((int)0, (int)m_activeTetraNum.size (), (int)1, [&](int aindx)
                {
                if (flips[aindx].info != 0)
                    return;
                int flipTetraNum = m_activeTetraNum[aindx];
                Tetrahedron& flipTetrahedron = m_tetrahedrons[flipTetraNum];
                flips[aindx].flipTNum = flipTetraNum;
                if (!flipTetrahedron.isChanged)
                    return;
                FaceEdgeStack flipTest;
                int oppVert[4];

                for (short botVi = 0; botVi < 4; ++botVi)
                    {
                    int topVert = -1;
                    int topTi = flipTetrahedron.GetAdjentTet (botVi);
                    if (!flipTetrahedron.GetInternalOpp (botVi) && topTi != -1)
                        {
                        const Tetrahedron& topTet = m_tetrahedrons[topTi];
                        if (topTet.GetFaceSide (flipTetraNum) != -1)
                            {
                            topVert = topTet.ptNums[topTet.GetFaceSide (flipTetraNum)];
                            if (topTi < flipTetraNum && topTet.isChanged) //Changed == getTetCheckState (tetInfoArr[topTi])))
                                topVert = makeNegative (topVert);
                            }
                        }
                    oppVert[botVi] = topVert;
                    }

                // Colect faces to test.
                CollectFacesToTests (oppVert, flipTetrahedron, flipTest);

                if (flipTest.IsEmpty ())
                    return;


                // Get tetrahedrons.
                DTetrahedron3d dTet;
                flipTetrahedron.GetDTetrahedron3d (dTet, m_points);

                // Do tests for flip23.
                FaceStack flipTest23;
                for (; !flipTest.IsEmpty (); flipTest.Pop ())
                    {
                    const int botVi = flipTest.GetFace ();
                    int botCorOrdVi = flipTest.GetEdge ();
                    assert (botVi >= 0 && botVi <= 3);
                    const int topVert = oppVert[botVi];
                    assert (topVert != -1);
                    const DPoint3d& topP = m_points[topVert];

                    Side side;

                    if (m_inSpecialFlip)
                        {
                        side = doInSphere (flipTetrahedron, topVert, dTet, topP);
                        if (side == SideZero)
                            side = doInSphereSoS (flipTetrahedron, topVert, dTet, topP);
                        }
                    else
                        {
                        side = doInSphere (flipTetrahedron, topVert, dTet, topP);
                        if (SideZero == side)
                            {
                            hasSpecial = true;
                            flipTetrahedron.special = true;
                            }
                        }

                    if (SideIn != side)
                        {
                        continue; // No insphere failure at this face
                        }
                    flipTetrahedron.SetFailedDelaunay (botVi);
                    // This has a flip 32 so return it.
                    if (botCorOrdVi < 3)
                        {
                        int flip44Tet = -1;
                        bool addTest = TestFlip32 (flipTetrahedron, flipTetraNum, botVi, botCorOrdVi, flip44Tet);
                        if (addTest)
                            {
                            int symTetraNum = flipTetrahedron.GetAdjentTet (botVi);

                            // We need to check that this doesn't cause an invalid tetrahedrons,
                            //3-2 Flip
                            FlipInformation& flip = flips[aindx];
                            flip.Init (flip44Tet != -1 ? FlipInformation::Flip44 : FlipInformation::Flip32, botVi, botCorOrdVi);
                            flip.flip44TNum = flip44Tet;
                            flip.flipTNum = flipTetraNum;
                            const int* botOrdVi = TetViAsSeenFrom[botVi]; // Order bottom tetra as seen from apex vertex
                            static const int testInfo[][numFacets] = {
                                    { -1, 2, 1, 0 },
                                    { 2, -1, 0, 1 },
                                    { 2, 1, -1, 0 },
                                    { 1, 2, 0, -1 },
                                };

                            static const int otherSideHelper[4][3] =
                                {
                                    { 3, 2, 1 },  // 2
                                    { 2, 3, 0 },  // 3
                                    { 3, 1, 0 },  // 1
                                    { 2, 0, 1 },  // 0
                                };
                            flip.otherTNum = flipTetrahedron.GetAdjentTet (otherSideHelper[botVi][testInfo[botVi][botOrdVi[botCorOrdVi]]]);
                            flipTetrahedron.special = false;
                            atomic_min_cpu (m_flipTestNumber[flipTetraNum], (unsigned int)flipTetraNum);
                            atomic_min_cpu (m_flipTestNumber[symTetraNum], (unsigned int)flipTetraNum);
                            atomic_min_cpu (m_flipTestNumber[flip.otherTNum], (unsigned int)flipTetraNum);
                            if (flip44Tet != -1)
                                atomic_min_cpu (m_flipTestNumber[flip44Tet], (unsigned int)flipTetraNum);
                            flipTest23.Clear ();
                            }
                        break;
                        }
                    flipTest23.Push (botVi);
                    }

                if (flipTest23.IsEmpty ())
                    return;


                // Tests for flip 23
                for (; !flipTest23.IsEmpty (); flipTest23.Pop ())
                    {
                    int botVi = flipTest23.GetFace ();
                    const int topVert = oppVert[botVi];
                    const DPoint3d& topP = m_points[topVert];
                    const int* botOrdVi = TetViAsSeenFrom[botVi]; // Order bottom tetra as seen from apex vertex

                    bool hasFlip = true;
                    int opTri = -1;
                    // Go around bottom-top tetra, check 3 sides
                    for (int i = 0; i < 3; ++i)
                        {
                        const int* fv = TetViAsSeenFrom[botOrdVi[i]];

                        Orient ort;

                        if (m_skipFlipOnNearZero)
                            {
                            //if (!m_inSpecialFlip)
                            //    {
                                ort = doOrient3D (flipTetrahedron.ptNums[fv[0]], flipTetrahedron.ptNums[fv[1]], flipTetrahedron.ptNums[fv[2]], topVert,
                                                  dTet.points[fv[0]], dTet.points[fv[1]], dTet.points[fv[2]], topP);
                            //    if (OrientZero == ort)
                            //        {
                            //        hasSpecial = true;
                            //        flipTetrahedron.special = true;
                            //        }
                            //    }
                            //else
                            //    {
                            //    ort = doOrient3D (flipTetrahedron.ptNums[fv[0]], flipTetrahedron.ptNums[fv[1]], flipTetrahedron.ptNums[fv[2]], topVert,
                            //                      dTet.points[fv[0]], dTet.points[fv[1]], dTet.points[fv[2]], topP);
                            //    if (OrientZero == ort)
                            //        {
                            //        ort = doOrient3DSoS (flipTetrahedron.ptNums[fv[0]], flipTetrahedron.ptNums[fv[1]], flipTetrahedron.ptNums[fv[2]], topVert,
                            //                             dTet.points[fv[0]], dTet.points[fv[1]], dTet.points[fv[2]], topP);
                            //        if (OrientPos == ort)
                            //            {
                            //            bvector<PointOnEdge> onEdges;
                            //            FindPointsAroundEdge (flipTetraNum, flipTetrahedron.ptNums[fv[0]], flipTetrahedron.ptNums[fv[1]], onEdges, m_tetrahedrons);
                            //            if (onEdges.size () == 4)
                            //                {
                            //                ort = OrientPos;
                            //                // OK we can do a 4-4 flip. acde,bdce,adef,bedf - abcd,abec,abfd,abef.
                            //                }
                            //            else
                            //                ort = OrientNeg;
                            //            }
                            //        }
                            //    }
                            }
                        else
                            {
                            if (m_inSpecialFlip)
                                {

                                //ort = doOrient3D (flipTetrahedron.ptNums[fv[0]], flipTetrahedron.ptNums[fv[1]], flipTetrahedron.ptNums[fv[2]], topVert,
                                //                  dTet.points[fv[0]], dTet.points[fv[1]], dTet.points[fv[2]], topP);
                                //if (ort == OrientZero)
                                ort = doOrient3DSoS (flipTetrahedron.ptNums[fv[0]], flipTetrahedron.ptNums[fv[1]], flipTetrahedron.ptNums[fv[2]], topVert,
                                                     dTet.points[fv[0]], dTet.points[fv[1]], dTet.points[fv[2]], topP);

                                }
                            else
                                {
                                ort = doOrient3D (flipTetrahedron.ptNums[fv[0]], flipTetrahedron.ptNums[fv[1]], flipTetrahedron.ptNums[fv[2]], topVert,
                                                  dTet.points[fv[0]], dTet.points[fv[1]], dTet.points[fv[2]], topP);
                                if (OrientZero == ort)
                                    {
                                    hasSpecial = true;
                                    flipTetrahedron.special = true;
                                    }
                                }
                            }
                        //DPoint3d tri1[3]{dTet.points[fv[0]], dTet.points[fv[1]], dTet.points[fv[2]]},
                        //    tri2[3]{dTet.points[fv[1]], dTet.points[fv[2]], topP},
                        //    tri3[3]{dTet.points[fv[0]], dTet.points[fv[1]], topP};

                        //bool isNonCollinear = (!Are3PointsCollinear (tri1, 0.00001) && !Are3PointsCollinear (tri2, 0.00001)
                        //                       && !Are3PointsCollinear (tri3, 0.00001));
                        if (OrientPos != ort) // || !isNonCollinear)
                            {
                            hasFlip = false;
                            break; // Cannot do 23 flip
                            }
                        }

                    if (hasFlip) //*** 2-3 flip possible!
                        {
                        int symTetraNum = flipTetrahedron.GetAdjentTet (botVi);
                        FlipInformation& flip = flips[aindx];
                        flip.Init (FlipInformation::Flip23, botVi, 0);
                        flip.flipTNum = flipTetraNum;
                        flip.otherTNum = -1; // flipTetrahedron.GetAdjentTet_[flipEdge];
                        flipTetrahedron.special = false;
                        atomic_min_cpu (m_flipTestNumber[flipTetraNum], (unsigned int)flipTetraNum);
                        atomic_min_cpu (m_flipTestNumber[symTetraNum], (unsigned int)flipTetraNum);
                        //flips.local ().push_back (flip);
                        return;
                        }
                    } // Check faces of tetra

                });
            }

        static void setTetIdxVi (int &output, int oldVi, int ni, int newVi)
            {
            output -= (0xF) << (oldVi * 4);
            output += ((ni << 2) + newVi) << (oldVi * 4);
            }

        static void storeFlip (FlipItem* tetArr, int idx, const FlipItem flip)
            {
            int4 t1 = { flip._v[0], flip._v[1], flip._v[2], flip._v[3] };
            int4 t2 = { flip._v[4], flip._t[0], flip._t[1], flip._t[2] };

            ((int4 *)tetArr)[idx * 2 + 0] = t1;
            ((int4 *)tetArr)[idx * 2 + 1] = t2;
            }

        std::vector<int2> tetMsgArr;
        std::vector<FlipItem> flipArr;
        std::vector<int> flip23NewSlot;


        unsigned char GetVi (int idx, int vi)
            {
            int opIdx = m_tetrahedrons[idx].GetAdjentTet (vi);
            return m_tetrahedrons[opIdx].GetFaceSide (idx);
            }

        inline bool DoFlip (const FlipInformation& flip, int flipId, FlipPointHelperList& hasFlipped)
            {
            AutoTimer timer (m_timeToDoFlip);
            TRACE_ERROR_INIT ()
                int fType = flip.GetFlipType ();
            // Bottom tetra
            const int botTetIdx = flip.flipTNum;
            const int botCorOrdVi = flip.GetFlipEdge (); // getFlipBotCorOrdVi (flipInfo);
            const int botTetVi = flip.GetFlipFace (); // getFlipBotVi (flipInfo);
            Tetrahedron& botTet = m_tetrahedrons[botTetIdx];
            //                const TetOpp& botOpp = botTet.GetAdjentTet //loadOpp (oppArr, botTetIdx);

            // Top tetra
            const int topTetIdx = botTet.GetAdjentTet (botTetVi);  //botOpp.getOppTet (botTetVi);
            Tetrahedron& topTet = m_tetrahedrons[topTetIdx]; // loadTet (tetArr, topTetIdx);
            const int topTetVi = topTet.GetFaceSide (botTetIdx); // botOpp.getOppVi (botTetVi);
            const int globFlipIdx = flipId; // orgFlipNum + flipIdx;

            int encodedFaceVi = 0;
            int sideTetIdx = flip.otherTNum;
            Tetrahedron& sideTet = m_tetrahedrons[sideTetIdx];

            const int* corVi = TetViAsSeenFrom[botTetVi];
            const int* corTopVi = TetViAsSeenFrom[topTetVi];

            int newIdx[3] = { 0xFFFF, 0xFFFF, 0xFFFF };

            if (FlipInformation::Flip44 == flip.GetFlipType ())
                {
                // ToDo
                return false;
                }
            else if (FlipInformation::Flip23 == flip.GetFlipType ())
                {
                const int corV[3] = { botTet.ptNums[corVi[0]], botTet.ptNums[corVi[1]], botTet.ptNums[corVi[2]] };
                int topVi0 = TetNextViAsSeenFrom[topTetVi][topTet.GetPointIndex (corV[2])]; // getIndexOf

                const int oldFaceVi[3][2] = {
                        { corVi[2], corTopVi[topVi0] },  // Old bottom tetra
                        { corVi[0], corTopVi[(topVi0 + 2) % 3] },  // Old top tetra
                        { corVi[1], corTopVi[(topVi0 + 1) % 3] } // Old side tetra
                    };

                // Iterate new tetra
                for (int ni = 0; ni < 3; ++ni)
                    {
                    // Set external face adjacencies
                    setTetIdxVi (newIdx[0], oldFaceVi[ni][0], ni == 0 ? 3 : ni, 0);
                    setTetIdxVi (newIdx[1], oldFaceVi[ni][1], ni == 1 ? 3 : ni, 3);

                    encodedFaceVi = (encodedFaceVi << 4)
                        | (oldFaceVi[ni][0] << 2)
                        | (oldFaceVi[ni][1]);
                    } // 3 new tetra

                // Create new tetra
                const int topVert = topTet.ptNums[topTetVi];
                const int botVert = botTet.ptNums[botTetVi];

                botTet.SetPointNums (topVert, corV[0], corV[1], botVert);
                topTet.SetPointNums (topVert, corV[1], corV[2], botVert);

                // Output the side tetra
                sideTet.SetPointNums (topVert, corV[2], corV[0], botVert);

#ifdef CHECKTETRAHEDRON
                CheckTetrahedronPoints (botTet);
                CheckTetrahedronPoints (topTet);
                CheckTetrahedronPoints (sideTet);
#endif

                //check if no 3 of 4 points are collinear
                /*DTetrahedron3d bot, top, side;
                botTet.GetDTetrahedron3d (bot, m_points);
                topTet.GetDTetrahedron3d (top, m_points);
                sideTet.GetDTetrahedron3d (side, m_points);
                if (bsiGeom_isDPoint3dArrayColinear(top.points, 3, 0.00001) ||
                bsiGeom_isDPoint3dArrayColinear(top.points + 1, 3, 0.00001))
                {
                std::string msg = "[DoFlip] POINTS ARE COLINEAR at flip " + std::to_string(flip.info) + " top [" + std::to_string(topTet.ptNums[0]) + "," + std::to_string(topTet.ptNums[1])
                + "," + std::to_string(topTet.ptNums[2]) + "," + std::to_string(topTet.ptNums[3]) + "] [" + std::to_string(top.points[0].x) + "," + std::to_string(top.points[0].y)
                + "," + std::to_string(top.points[0].z) + ")(" + std::to_string(top.points[1].x) + "," + std::to_string(top.points[1].y) + "," +
                std::to_string(top.points[1].z) + ")(" + std::to_string(top.points[2].x) + "," + std::to_string(top.points[2].y) + "," + std::to_string(top.points[2].z)
                + ")(" + std::to_string(top.points[3].x) + "," + std::to_string(top.points[3].y) + "," + std::to_string(top.points[3].z) + "]";
                TRACE_ERROR(msg);
                }
                if (bsiGeom_isDPoint3dArrayColinear(bot.points, 3, 0.00001) ||
                bsiGeom_isDPoint3dArrayColinear(bot.points + 1, 3, 0.00001))
                {
                std::string msg = "[DoFlip] POINTS ARE COLINEAR " + std::to_string(flip.info) + " bot [" + std::to_string(botTet.ptNums[0]) + "," + std::to_string(botTet.ptNums[1])
                + "," + std::to_string(botTet.ptNums[2]) + "," + std::to_string(botTet.ptNums[3]) + "] [" + std::to_string(bot.points[0].x) + "," + std::to_string(bot.points[0].y)
                + "," + std::to_string(bot.points[0].z) + ")(" + std::to_string(bot.points[1].x) + "," + std::to_string(bot.points[1].y) + "," +
                std::to_string(bot.points[1].z) + ")(" + std::to_string(bot.points[2].x) + "," + std::to_string(bot.points[2].y) + "," + std::to_string(bot.points[2].z)
                + ")(" + std::to_string(bot.points[3].x) + "," + std::to_string(bot.points[3].y) + "," + std::to_string(bot.points[3].z) + "]";
                TRACE_ERROR(msg);
                }
                if (bsiGeom_isDPoint3dArrayColinear(side.points, 3, 0.00001) ||
                bsiGeom_isDPoint3dArrayColinear(side.points + 1, 3, 0.00001))
                {
                std::string msg = "[DoFlip] POINTS ARE COLINEAR " + std::to_string(flip.info) + " side [" + std::to_string(sideTet.ptNums[0]) + "," + std::to_string(sideTet.ptNums[1])
                + "," + std::to_string(sideTet.ptNums[2]) + "," + std::to_string(sideTet.ptNums[3]) + "] [" + std::to_string(side.points[0].x) + "," + std::to_string(side.points[0].y)
                + "," + std::to_string(side.points[0].z) + ")(" + std::to_string(side.points[1].x) + "," + std::to_string(side.points[1].y) + "," +
                std::to_string(side.points[1].z) + ")(" + std::to_string(side.points[2].x) + "," + std::to_string(side.points[2].y) + "," + std::to_string(side.points[2].z)
                + ")(" + std::to_string(side.points[3].x) + "," + std::to_string(side.points[3].y) + "," + std::to_string(side.points[3].z) + "]";
                TRACE_ERROR(msg);
                }*/
                //assert(!Are3PointsCollinear(top.points,  0.00001) && !Are3PointsCollinear(top.points + 1, 0.00001));
                //assert(!Are3PointsCollinear(bot.points,  0.00001) && !Are3PointsCollinear(bot.points + 1,  0.00001));
                //assert(!Are3PointsCollinear(side.points, 0.00001) && !Are3PointsCollinear(side.points + 1,0.00001));
                }
            else
                {
                // Side tetra
                const int botCorVi = corVi[botCorOrdVi];
                //sideTetIdx = botTet.GetAdjentTet (botCorVi); // botOpp.getOppTet (botCorVi);
                const int sideCorVi0 = sideTet.GetFaceSide (botTetIdx);// botOpp.getOppVi (botCorVi);

                // BotVi
                const int botAVi = corVi[(botCorOrdVi + 1) % 3];
                const int botBVi = corVi[(botCorOrdVi + 2) % 3];
                const int botA = botTet.ptNums[botAVi];
                const int botB = botTet.ptNums[botBVi];

                // Top vi
                const int botCor = botTet.ptNums[botCorVi];
                const int topCorVi = topTet.GetPointIndex (botCor);
                const int topLocVi = TetNextViAsSeenFrom[topTetVi][topCorVi];
                const int topAVi = corTopVi[(topLocVi + 2) % 3];
                const int topBVi = corTopVi[(topLocVi + 1) % 3];

                // Side vi
                const int sideCorVi1 = sideTet.GetFaceSide (topTetIdx); // oppArr[topTetIdx].getOppVi (topCorVi);
                const int sideLocVi = TetNextViAsSeenFrom[sideCorVi0][sideCorVi1];
                const int* sideOrdVi = TetViAsSeenFrom[sideCorVi0];
                const int sideAVi = sideOrdVi[(sideLocVi + 1) % 3];
                const int sideBVi = sideOrdVi[(sideLocVi + 2) % 3];

                const int oldFaceVi[3][2] = {
                        { botAVi, botBVi },  // Old bottom tetra
                        { topAVi, topBVi },  // Old top tetra
                        { sideAVi, sideBVi } // Old side tetra
                    };

                // Set external face adjacencies
                for (int ti = 0; ti < 3; ++ti) // Iterate old tetra
                    {
                    setTetIdxVi (newIdx[ti], oldFaceVi[ti][0], 1 == ti ? 3 : 1, Flip32NewFaceVi[ti][0]);
                    setTetIdxVi (newIdx[ti], oldFaceVi[ti][1], 0 == ti ? 3 : 0, Flip32NewFaceVi[ti][1]);

                    encodedFaceVi = (encodedFaceVi << 4)
                        | (oldFaceVi[ti][0] << 2)
                        | (oldFaceVi[ti][1]);
                    }

                // Write down the new tetra idx
                tetMsgArr[sideTetIdx] = make_int2 (newIdx[2], globFlipIdx);

                // Vertices of old 3 tetra
                const int botTetV = botTet.ptNums[botTetVi];
                const int topTetV = topTet.ptNums[topTetVi];

                botTet.SetPointNums (botCor, topTetV, botTetV, botA);
                topTet.SetPointNums (botCor, botTetV, topTetV, botB);

#ifdef CHECKTETRAHEDRON
                CheckTetrahedronPoints (botTet);
                CheckTetrahedronPoints (topTet);
#endif

                ////*** Donate one tetra
                //const int insIdx = sideTetIdx / MeanVertDegree;
                //const int vertIdx = (insIdx < insVertVec._num) ? insVertVec._arr[insIdx] : infIdx;
                //const int freeIdx = atomicAdd (&vertFreeArr[vertIdx], 1);

                //freeArr[vertIdx * MeanVertDegree + freeIdx] = sideTetIdx;
                }

            // Write down the new tetra idx
            tetMsgArr[botTetIdx] = make_int2 (newIdx[0], globFlipIdx);
            tetMsgArr[topTetIdx] = make_int2 (newIdx[1], globFlipIdx);

            // Update the bottom and top tetra
            //m_tetrahedrons[botTetIdx].SetPointNums (botTet); // storeTet (tetArr, botTetIdx, botTet);
            //m_tetrahedrons[topTetIdx].SetPointNums (topTet); //storeTet (tetArr, topTetIdx, topTet);

            /****************/
            unsigned char extOpp[6];
            if (FlipInformation::Flip44 == fType)
                {
                // ToDo
                }
            else if (FlipInformation::Flip23 == fType)
                {
                extOpp[0] = GetVi (botTetIdx, (encodedFaceVi >> 10) & 3); // was getOppTetVi
                extOpp[2] = GetVi (botTetIdx, (encodedFaceVi >> 6) & 3);
                extOpp[4] = GetVi (botTetIdx, (encodedFaceVi >> 2) & 3);
                }
            else
                {
                extOpp[0] = GetVi (botTetIdx, (encodedFaceVi >> 10) & 3);
                extOpp[1] = GetVi (botTetIdx, (encodedFaceVi >> 8) & 3);
                }

            if (FlipInformation::Flip44 == fType)
                {
                // ToDo
                }
            else if (FlipInformation::Flip23 == fType)
                {
                extOpp[1] = GetVi (topTetIdx, (encodedFaceVi >> 8) & 3);
                extOpp[3] = GetVi (topTetIdx, (encodedFaceVi >> 4) & 3);
                extOpp[5] = GetVi (topTetIdx, (encodedFaceVi >> 0) & 3);
                }
            else
                {
                extOpp[2] = GetVi (topTetIdx, (encodedFaceVi >> 6) & 3);
                extOpp[3] = GetVi (topTetIdx, (encodedFaceVi >> 4) & 3);

                extOpp[4] = GetVi (sideTetIdx, (encodedFaceVi >> 2) & 3);
                extOpp[5] = GetVi (sideTetIdx, (encodedFaceVi >> 0) & 3);
                }
            int extOppC = 0;

            for (int i = 5; i >= 0; i--)
                extOppC = extOppC << 2 | extOpp[i];
            encodedFaceVi |= extOppC << 12;

            /****************/
            // Store faceVi
            flip23NewSlot[flipId] = encodedFaceVi;

            //bool tetEmpty = isTetEmpty (tetInfoArr[botTetIdx]) &&
            //    isTetEmpty (tetInfoArr[topTetIdx]);

            //if (fType == Flip32 && tetEmpty)
            //    tetEmpty = isTetEmpty (tetInfoArr[sideTetIdx]);

            // Record the flip
            // ToDo
            FlipItem flipItem = { botTet.ptNums[0], botTet.ptNums[1], botTet.ptNums[2],
                botTet.ptNums[3], topTet.ptNums[flip.GetFlipType () == FlipInformation::Flip23 ? 2 : 3],
                botTetIdx, topTetIdx,
                (flip.GetFlipType () == FlipInformation::Flip32) ? makeNegative (sideTetIdx) : sideTetIdx };

            //if (tetEmpty)
            //    flipItem._v[0] = -1;

            //if (actTetArr != NULL)
            //    {
            //    actTetArr[flipIdx] =
            //        (Checked == getTetCheckState (tetInfoArr[topTetIdx]))
            //        ? topTetIdx : -1;
            //    actTetArr[flipToTet._num + flipIdx] =
            //        (fType == Flip23)
            //        ? sideTetIdx : -1;
            //    }

            //char botTetState = 3;    // Alive + Changed
            //char topTetState = 3;
            //char sideTetState = 3;

            botTet.isActive = botTet.isChanged = true; botTet.ClearStatus ();
            topTet.isActive = topTet.isChanged = true; topTet.ClearStatus ();
            //m_tetrahedrons[botTetIdx].isActive = true; m_tetrahedrons[botTetIdx].isChanged = true; m_tetrahedrons[botTetIdx].failedDelaunay = 0; //setTetEmptyState (botTetState, tetEmpty);
            //m_tetrahedrons[topTetIdx].isActive = true; m_tetrahedrons[topTetIdx].isChanged = true; m_tetrahedrons[topTetIdx].failedDelaunay = 0;//setTetEmptyState (topTetState, tetEmpty);

            if (FlipInformation::Flip44 == flip.GetFlipType ())
                {
                }
            else if (FlipInformation::Flip23 == flip.GetFlipType ())
                {
                sideTet.isActive = sideTet.isChanged = true; sideTet.special = false; sideTet.ClearStatus ();//setTetEmptyState (sideTetState, tetEmpty);
                }
            else
                {
                sideTet.isActive = sideTet.isChanged = false;
                }

            //tetInfoArr[botTetIdx] = botTetState;
            //tetInfoArr[topTetIdx] = topTetState;
            //tetInfoArr[sideTetIdx] = sideTetState;

            storeFlip (flipArr.data (), globFlipIdx, flipItem);
            //check if no 3 of 4 points are collinear
            DTetrahedron3d bot, top, side;
            botTet.GetDTetrahedron3d (bot, m_points);
            topTet.GetDTetrahedron3d (top, m_points);
            sideTet.GetDTetrahedron3d (side, m_points);
            /* if (bsiGeom_isDPoint3dArrayColinear(top.points, 3, 0.00001) ||
            bsiGeom_isDPoint3dArrayColinear(top.points + 1, 3, 0.00001))
            {
            std::string msg = "[DoFlip] END POINTS ARE COLINEAR " + std::to_string(flip.info) + " top [" + std::to_string(topTet.ptNums[0]) + "," + std::to_string(topTet.ptNums[1])
            + "," + std::to_string(topTet.ptNums[2]) + "," + std::to_string(topTet.ptNums[3]) + "] [" + std::to_string(top.points[0].x) + "," + std::to_string(top.points[0].y)
            + "," + std::to_string(top.points[0].z) + ")(" + std::to_string(top.points[1].x) + "," + std::to_string(top.points[1].y) + "," +
            std::to_string(top.points[1].z) + ")(" + std::to_string(top.points[2].x) + "," + std::to_string(top.points[2].y) + "," + std::to_string(top.points[2].z)
            + ")(" + std::to_string(top.points[3].x) + "," + std::to_string(top.points[3].y) + "," + std::to_string(top.points[3].z) + "]";
            TRACE_ERROR(msg);
            }
            if (bsiGeom_isDPoint3dArrayColinear(bot.points, 3, 0.00001) ||
            bsiGeom_isDPoint3dArrayColinear(bot.points + 1, 3, 0.00001))
            {
            std::string msg = "[DoFlip] END POINTS ARE COLINEAR " + std::to_string(flip.info) + " bot [" + std::to_string(botTet.ptNums[0]) + "," + std::to_string(botTet.ptNums[1])
            + "," + std::to_string(botTet.ptNums[2]) + "," + std::to_string(botTet.ptNums[3]) + "] [" + std::to_string(bot.points[0].x) + "," + std::to_string(bot.points[0].y)
            + "," + std::to_string(bot.points[0].z) + ")(" + std::to_string(bot.points[1].x) + "," + std::to_string(bot.points[1].y) + "," +
            std::to_string(bot.points[1].z) + ")(" + std::to_string(bot.points[2].x) + "," + std::to_string(bot.points[2].y) + "," + std::to_string(bot.points[2].z)
            + ")(" + std::to_string(bot.points[3].x) + "," + std::to_string(bot.points[3].y) + "," + std::to_string(bot.points[3].z) + "]";
            TRACE_ERROR(msg);
            }
            if (bsiGeom_isDPoint3dArrayColinear(side.points, 3, 0.00001) ||
            bsiGeom_isDPoint3dArrayColinear(side.points + 1, 3, 0.00001))
            {
            std::string msg = "[DoFlip] END POINTS ARE COLINEAR " + std::to_string(flip.info) + " side [" + std::to_string(sideTet.ptNums[0]) + "," + std::to_string(sideTet.ptNums[1])
            + "," + std::to_string(sideTet.ptNums[2]) + "," + std::to_string(sideTet.ptNums[3]) + "] [" + std::to_string(side.points[0].x) + "," + std::to_string(side.points[0].y)
            + "," + std::to_string(side.points[0].z) + ")(" + std::to_string(side.points[1].x) + "," + std::to_string(side.points[1].y) + "," +
            std::to_string(side.points[1].z) + ")(" + std::to_string(side.points[2].x) + "," + std::to_string(side.points[2].y) + "," + std::to_string(side.points[2].z)
            + ")(" + std::to_string(side.points[3].x) + "," + std::to_string(side.points[3].y) + "," + std::to_string(side.points[3].z) + "]";
            TRACE_ERROR(msg);
            }*/
            // assert(!Are3PointsCollinear(top.points,  0.00001) && !Are3PointsCollinear(top.points + 1,  0.00001));
            //assert(!Are3PointsCollinear(bot.points,  0.00001) && !Are3PointsCollinear(bot.points + 1,  0.00001));
            //if (sideTet.isActive) assert(!Are3PointsCollinear(side.points,  0.00001) && !Are3PointsCollinear(side.points + 1, 0.00001));
            TRACE_ERROR_END ()
                return true;
            }

        inline void UpdatePointsAfterFlip (const FlipPointHelperList& hasFlipped, const std::vector<FlipInformation>& doFlips)
            {
            AutoTimer timer2 (m_timeToFixPointsAfterFlip);
            // Fix points tetrahedrons.
            parallel_for ((int)0, (int)m_pointsToInsert.size (), (int)1, [&](int aindx)
                {
                int oldTetrahedron = m_pointTetrahedron[aindx];  // This can be improved if we look at the flip type.
                if (oldTetrahedron == -1)
                    return;
                const int2 msg = tetMsgArr[oldTetrahedron];
                if (msg.y != -1)
                    {
                    const FlipInformation& flip = doFlips[msg.y];
                    const DPoint3d& p = m_points[m_pointsToInsert[aindx]];
                    assert (flip.GetFlipType () != FlipInformation::FlipNone);
                    const Tetrahedron& flipTet = m_tetrahedrons[flip.flipTNum];

                    if (flip.GetFlipType () == FlipInformation::Flip44)
                        {
                        return;
                        }
                    else if (flip.GetFlipType () == FlipInformation::Flip32)
                        {
                        int ptNums[3];
                        flipTet.GetFacePoints (3, ptNums);
                        Orient ort = doOrient3DSoS (
                            ptNums[0], ptNums[1], ptNums[2], m_pointsToInsert[aindx],
                            m_points[ptNums[0]], m_points[ptNums[1]], m_points[ptNums[2]], p);
                        if (ort != OrientNeg)
                            m_pointTetrahedron[aindx] = flip.flipTNum;
                        else
                            m_pointTetrahedron[aindx] = flipTet.GetAdjentTet (3);
                        }
                    else
                        {
                        int ptNums[3];
                        flipTet.GetFacePoints (2, ptNums);
                        Orient ort1 = doOrient3DSoS (
                            ptNums[0], ptNums[1], ptNums[2], m_pointsToInsert[aindx],
                            m_points[ptNums[0]], m_points[ptNums[1]], m_points[ptNums[2]], p);

                        flipTet.GetFacePoints (1, ptNums);
                        Orient ort2 = doOrient3DSoS (
                            ptNums[0], ptNums[1], ptNums[2], m_pointsToInsert[aindx],
                            m_points[ptNums[0]], m_points[ptNums[1]], m_points[ptNums[2]], p);

                        if (ort1 != OrientNeg && ort2 != OrientNeg)
                            m_pointTetrahedron[aindx] = flip.flipTNum;
                        else
                            {
                            const Tetrahedron& otherTet = m_tetrahedrons[flip.otherTNum];
                            otherTet.GetFacePoints (2, ptNums);
                            Orient ort3 = doOrient3DSoS (
                                ptNums[0], ptNums[1], ptNums[2], m_pointsToInsert[aindx],
                                m_points[ptNums[0]], m_points[ptNums[1]], m_points[ptNums[2]], p);
                            if (ort3 != OrientNeg)
                                m_pointTetrahedron[aindx] = flip.otherTNum;
                            else
                                m_pointTetrahedron[aindx] = otherTet.GetAdjentTet (2);
                            }
                        }
#ifdef VALIDATION_CHECK
                    const Tetrahedron& ptTet = m_tetrahedrons[m_pointTetrahedron[aindx]];

                    bool failed = false;
                    for (int f = 0; f < numFacets; f++)
                        {
                        int ptNums[3];
                        ptTet.GetFacePoints (f, ptNums);
                        Orient ort3 = doOrient3DSoS (
                            ptNums[0], ptNums[1], ptNums[2], m_pointsToInsert[aindx],
                            m_points[ptNums[0]], m_points[ptNums[1]], m_points[ptNums[2]], p);
                        if (ort3 == OrientNeg)
                            failed = true;
                        }
                    if (failed)
                        {
                        int foundIndx = -1;
                        for (size_t tIndx = 0; tIndx < m_tetrahedrons.size (); tIndx++)
                            {
                            const Tetrahedron& tet = m_tetrahedrons[tIndx];
                            bool failed = false;
                            if (tet.ptNums[0] == -1)
                                break;
                            Orient ort = doOrient3DSoS (tet.ptNums[0], tet.ptNums[1], tet.ptNums[2], tet.ptNums[3], m_points[tet.ptNums[0]], m_points[tet.ptNums[1]], m_points[tet.ptNums[2]], m_points[tet.ptNums[3]]);

                            if (ort == OrientNeg)
                                ort = ort;
                            for (int f = 0; f < numFacets; f++)
                                {
                                int ptNums[3];
                                tet.GetFacePoints (f, ptNums);
                                Orient ort3 = doOrient3DSoS (
                                    ptNums[0], ptNums[1], ptNums[2], m_pointsToInsert[aindx],
                                    m_points[ptNums[0]], m_points[ptNums[1]], m_points[ptNums[2]], p);

                                if (ort3 == OrientNeg)
                                    failed = true;
                                }
                            if (!failed)
                                {
                                foundIndx = &tet - m_tetrahedrons.data ();
                                }
                            }
                        foundIndx = 0;
                        }
#endif
                    }
                });
            }

        void UpdateAdjenciesAfterFlip (const FlipInformation& flip, int flipIdx)
            {
            FlipItemTetIdx flipItem = loadFlipTetIdx (flipArr.data (), flipIdx);
            const int/*FlipType*/ fType = flip.GetFlipType ();

            int encodedFaceVi = flip23NewSlot /*encodedFaceViArr*/[flipIdx];

            int     extOpp[6];
            Tetrahedron oppTet;

            oppTet = m_tetrahedrons[flipItem._t[0]]; // loadOpp (oppArr, flipItem._t[0]);

            if (FlipInformation::Flip44 == fType)
                {
                // ToDo
                return;
                }
            else if (FlipInformation::Flip23 == fType)
                {
                extOpp[0] = oppTet.GetAdjentTet ((encodedFaceVi >> 10) & 3); // was getOppTetVi
                extOpp[2] = oppTet.GetAdjentTet ((encodedFaceVi >> 6) & 3);
                extOpp[4] = oppTet.GetAdjentTet ((encodedFaceVi >> 2) & 3);
                }
            else
                {
                extOpp[0] = oppTet.GetAdjentTet ((encodedFaceVi >> 10) & 3);
                extOpp[1] = oppTet.GetAdjentTet ((encodedFaceVi >> 8) & 3);
                }

            oppTet = m_tetrahedrons[flipItem._t[1]]; // loadOpp (oppArr, flipItem._t[1]);

            if (FlipInformation::Flip44 == fType)
                {

                }
            else if (FlipInformation::Flip23 == fType)
                {
                extOpp[1] = oppTet.GetAdjentTet ((encodedFaceVi >> 8) & 3);
                extOpp[3] = oppTet.GetAdjentTet ((encodedFaceVi >> 4) & 3);
                extOpp[5] = oppTet.GetAdjentTet ((encodedFaceVi >> 0) & 3);
                }
            else
                {
                extOpp[2] = oppTet.GetAdjentTet ((encodedFaceVi >> 6) & 3);
                extOpp[3] = oppTet.GetAdjentTet ((encodedFaceVi >> 4) & 3);

                oppTet = m_tetrahedrons[makePositive (flipItem._t[2])]; // loadOpp (oppArr, flipItem._t[2]);
                //                    opp = loadOpp (oppArr, makePositive (flipItem._t[2]));

                extOpp[4] = oppTet.GetAdjentTet ((encodedFaceVi >> 2) & 3);
                extOpp[5] = oppTet.GetAdjentTet ((encodedFaceVi >> 0) & 3);
                }

            // Ok, update with neighbors
            for (int i = 0; i < 6; ++i)
                {
                int newTetIdx, vi;
                int tetOpp = extOpp[i];

                // No neighbor
                //if ( -1 == tetOpp ) continue;

                int oppIdx = tetOpp; // getOppValTet (tetOpp);
                int oppVi = encodedFaceVi >> (12 + i * 2) & 3; //getOppValVi (tetOpp);
                const int2 msg = tetMsgArr[oppIdx];

                if (msg.y == -1) //< orgFlipNum)    // Neighbor not flipped
                    {
                    // Set my neighbor's opp
                    if (fType == FlipInformation::Flip44)
                        {
                        // ToDo..
                        continue;
                        }
                    else if (fType == FlipInformation::Flip23)
                        {
                        newTetIdx = flipItem._t[i / 2];
                        vi = (i & 1) ? 3 : 0;
                        }
                    else
                        {
                        newTetIdx = flipItem._t[1 - (i & 1)];
                        vi = Flip32NewFaceVi[i / 2][i & 1];
                        }

                    m_tetrahedrons[oppIdx].SetAdjentTet (oppVi, newTetIdx); // oppArr[oppIdx].setOpp (oppVi, newTetIdx, vi);
                    }
                else
                    {
                    const int oppFlipIdx = msg.y; // -orgFlipNum;

                    // Update my own opp
                    const int newLocOppIdx = getTetIdx (msg.x, oppVi);

                    if (newLocOppIdx != 3)
                        oppIdx = flipArr[oppFlipIdx]._t[newLocOppIdx];

                    oppVi = getTetVi (msg.x, oppVi);

                    extOpp[i] = oppIdx; // makeOppVal (oppIdx, oppVi);
                    }
                }

            // Now output
            if (FlipInformation::Flip44 == fType)
                {
                }
            else if (FlipInformation::Flip23 == fType)
                {
                Tetrahedron& opp = m_tetrahedrons[flipItem._t[0]];
                opp.SetAdjentTet (0, extOpp[0]);
                opp.SetAdjentTet (1, flipItem._t[1]);
                opp.SetAdjentTet (2, flipItem._t[2]);
                opp.SetAdjentTet (3, extOpp[1]);
                opp.SetInternalOpp (1);
                opp.SetInternalOpp (2);
                }
            else
                {
                Tetrahedron& opp = m_tetrahedrons[flipItem._t[0]];
                opp.SetAdjentTet (1, extOpp[1]);
                opp.SetAdjentTet (2, extOpp[3]);
                opp.SetAdjentTet (0, extOpp[5]);
                opp.SetAdjentTet (3, flipItem._t[1]);
                opp.SetInternalOpp (3);
                }

            if (FlipInformation::Flip44 == fType)
                {
                }
            else if (FlipInformation::Flip23 == fType)
                {
                Tetrahedron& opp = m_tetrahedrons[flipItem._t[1]];
                opp.SetAdjentTet (0, extOpp[2]);
                opp.SetAdjentTet (2, flipItem._t[0]);
                opp.SetAdjentTet (1, flipItem._t[2]);
                opp.SetAdjentTet (3, extOpp[3]);
                opp.SetInternalOpp (2);
                opp.SetInternalOpp (1);
                }
            else
                {
                Tetrahedron& opp = m_tetrahedrons[flipItem._t[1]];
                opp.SetAdjentTet (2, extOpp[0]);
                opp.SetAdjentTet (1, extOpp[2]);
                opp.SetAdjentTet (0, extOpp[4]);
                opp.SetAdjentTet (3, flipItem._t[0]);
                opp.SetInternalOpp (3);
                }

            if (FlipInformation::Flip44 == fType)
                {
                }
            else if (FlipInformation::Flip23 == fType)
                {
                Tetrahedron& opp = m_tetrahedrons[flipItem._t[2]];
                opp.SetAdjentTet (0, extOpp[4]);
                opp.SetAdjentTet (1, flipItem._t[0]);
                opp.SetAdjentTet (2, flipItem._t[1]);
                opp.SetAdjentTet (3, extOpp[5]);
                opp.SetInternalOpp (1);
                opp.SetInternalOpp (2);
                }
            else
                {
                Tetrahedron& opp = m_tetrahedrons[makePositive (flipItem._t[2])];
                opp.DeleteTetrahedron (); // setTetAliveState (sideTetState, false);
                }

            return;
            }

#ifdef ALTERNATEACTIVETETCOLLECTION
        std::vector <int> m_newActiveTetraNum;
#endif
        // Flips the facets.
        bool FlipFacets (bool doSpecial)
            {
            bool doneAny = false;

#ifdef ALTERNATEACTIVETETCOLLECTION
            if (m_newActiveTetraNum.size () == 0)
                {
#endif
                // Initialize the active tetrahedon array.
                m_activeTetraNum.clear ();

                combinable<std::vector<int>> activeTetraNumCombinable;

                parallel_for ((int)0, (int)m_tetrahedrons.size (), (int)1, [&](int flipTetraNum)
                    {
                    if (m_tetrahedrons[flipTetraNum].isActive && m_tetrahedrons[flipTetraNum].isChanged)
                        activeTetraNumCombinable.local ().push_back (flipTetraNum);
                    });

                activeTetraNumCombinable.combine_each ([&](std::vector<int>& stack)
                    {
                    for (int n : stack)
                        {
                        m_activeTetraNum.push_back (n);
                        }
                    });
                if (m_ensureSameResults)
                    parallel_sort (m_activeTetraNum.begin (), m_activeTetraNum.end ());

#ifdef ALTERNATEACTIVETETCOLLECTION
                }
            else
                {
#ifdef CHECKACTIVEARRAY
                // Initialize the active tetrahedon array.
                m_activeTetraNum.clear ();

                combinable<std::vector<int>> activeTetraNumCombinable;

                parallel_for ((int)0, (int)m_tetrahedrons.size (), (int)1, [&](int flipTetraNum)
                    {
                    if (m_tetrahedrons[flipTetraNum].isActive && m_tetrahedrons[flipTetraNum].isChanged)
                        activeTetraNumCombinable.local ().push_back (flipTetraNum);
                    });

                activeTetraNumCombinable.combine_each ([&](std::vector<int>& stack)
                    {
                    for (int n : stack)
                        {
                        m_activeTetraNum.push_back (n);
                        }
                    });

                parallel_sort (m_activeTetraNum.begin (), m_activeTetraNum.end ());
                parallel_sort (m_newActiveTetraNum.begin (), m_newActiveTetraNum.end ());

                auto it = m_activeTetraNum.begin ();
                auto nit = m_newActiveTetraNum.begin ();

                while (nit != m_newActiveTetraNum.end ())
                    {
                    if (m_tetrahedrons[*nit].isChanged)
                        {
                        if (*nit != *it)
                            {
                            while (*it != *nit)
                                it++;
                            }
                        else
                            it++;
                        }
                    nit++;
                    }
#endif
                m_activeTetraNum = m_newActiveTetraNum;
                m_newActiveTetraNum.clear ();
                }
#endif
            std::vector<FlipInformation> flips;
            flips.resize (m_activeTetraNum.size ());

            if (!m_alwaysFlip)
                {
                if (m_activeTetraNum.size () < 2048 && !doSpecial)
                    return false;

                if (m_activeTetraNum.size () < 32 && m_pointsToInsert.size () != 0)
                    return false;
                }
            m_inSpecialFlip = false;
            bool hasSpecial = false;
            ExamineFacetsForFlip (flips, hasSpecial);
            if (doSpecial && hasSpecial)
                {
                m_inSpecialFlip = true;
                ExamineFacetsForFlip (flips, hasSpecial);
                }

            FlipPointHelperList hasFlipped ((int)m_tetrahedrons.size ());
            bool doneOne = false;
            AutoTimer timer (m_timeToFlipFacets);
            int newTetrahedronNumber = (int)m_tetrahedrons.size ();
            int oldTetrahedronSize = (int)m_tetrahedrons.size ();

            std::vector<FlipInformation> doFlips;
            combinable<std::vector<FlipInformation>> doFlipsCombinable;
            int numConflicts = 0;

            //std::cout << "_______________" << std::endl;

            // Find the flips that will be processed.
            parallel_for_each (flips.begin (), flips.end (), [&](FlipInformation& flip)
                {
                bool failed = true;
                if (flip.GetFlipType () == FlipInformation::FlipNone)
                    {
                    m_tetrahedrons[flip.flipTNum].isChanged = false;
                    return;
                    }
                const Tetrahedron& fTet = m_tetrahedrons[flip.flipTNum];
                int symTNum = fTet.GetAdjentTet (flip.GetFlipFace ());

                //if (flip.flipType == FlipInformation::Flip32)
                //    std::cout << flip.flipTNum << "," << flip.symTNum << "," << flip.otherTNum << std::endl;
                //else
                //    std::cout << flip.flipTNum << "," << flip.symTNum << std::endl;
                if (m_flipTestNumber[flip.flipTNum] != flip.flipTNum)
                    numConflicts++;
                else if (m_flipTestNumber[symTNum] != flip.flipTNum)
                    numConflicts++;
                else if (flip.GetFlipType () == FlipInformation::Flip32 && m_flipTestNumber[flip.otherTNum] != flip.flipTNum)
                    numConflicts++;
                else if (flip.GetFlipType () == FlipInformation::Flip44 && m_flipTestNumber[flip.otherTNum] != flip.flipTNum && m_flipTestNumber[flip.flip44TNum] != flip.flipTNum)
                    numConflicts++;
                else if (flip.GetFlipType () == FlipInformation::Flip23)
                    {
                    doFlipsCombinable.local ().push_back (flip);
                    failed = false;
                    }
                else if (flip.GetFlipType () == FlipInformation::Flip32)
                    {
                    doFlipsCombinable.local ().push_back (flip);
                    failed = false;
                    }
                else if (flip.GetFlipType () == FlipInformation::Flip44)
                    {
                    doFlipsCombinable.local ().push_back (flip);
                    failed = false;
                    }
                });

            doFlipsCombinable.combine_each ([&](std::vector<FlipInformation>& stack)
                {
                for (auto& flip : stack)
                    doFlips.push_back (flip);
                });
            if (doFlips.size () == 0)
                return false;

            // Mark all tetrahedrons that wont be processed as not active.
            //parallel_for_each_ (m_activeTetraNum.begin (), m_activeTetraNum.end (), [&](int tetraNum)
            //    {
            //    if (m_flipTestNumber[tetraNum] == 0xffffffff)
            //        m_tetrahedrons[tetraNum].isActive = false;
            //    });
            //            std::cout << "Num Conflicts " << numConflicts << std::endl;

            if (m_ensureSameResults)
                {
                parallel_sort (doFlips.begin (), doFlips.end (), [](FlipInformation& f1, FlipInformation& f2)
                    {
                    return f1.flipTNum < f2.flipTNum;
                    });
                }

#ifdef ALTERNATEACTIVETETCOLLECTION
            for (int t : m_activeTetraNum)
                {
                if (m_tetrahedrons[t].isChanged)
                    m_newActiveTetraNum.push_back (t);
                }
#endif
            // For all flips23 set the index of the new tetrahedon.
            for (FlipInformation& flip : doFlips) // NEEDS_WORK_SM  This loop could be paralized if we sort the flips then we know what index the flip is and either use the reuse number or the other Num,
                {                                 // And resize the arrrays afterwards.
#ifdef ALTERNATEACTIVETETCOLLECTION
                const Tetrahedron& fTet = m_tetrahedrons[flip.flipTNum];
                int symTNum = fTet.GetAdjentTet (flip.GetFlipFace ());
                if (!m_tetrahedrons[symTNum].isChanged)
                    m_newActiveTetraNum.push_back (symTNum);
#endif
                if (flip.GetFlipType () == FlipInformation::Flip23)
                    {
                    if (m_reuseTetrahedrons.size ())
                        {
                        flip.otherTNum = m_reuseTetrahedrons.back ();
                        m_reuseTetrahedrons.pop_back ();
                        }
                    else
                        {
                        flip.otherTNum = newTetrahedronNumber++;
                        }
#ifdef ALTERNATEACTIVETETCOLLECTION
                    m_newActiveTetraNum.push_back (flip.otherTNum);
#endif
                    }
                }

            // Initialize new tetrahedrons.
            if (newTetrahedronNumber != m_tetrahedrons.size ())
                {
                Tetrahedron t;
                t.DeleteTetrahedron ();
                m_tetrahedrons.resize (newTetrahedronNumber, t);
                }

            // Add all of the deleted tetrahedrons from 3-2.
            int n23 = 0;
            int n32 = 0;
            for (FlipInformation flip : doFlips)// This could be paralized if sorted by using resizeing the reuse array by the number of Flip32.
                {
                if (flip.GetFlipType () == FlipInformation::Flip32)
                    {
                    n32++;
                    m_reuseTetrahedrons.push_back (flip.otherTNum);
                    }
                }
            n23 = (int)doFlips.size () - n32;
            //std::cout << "--------------" << std::endl;
            // NEEDS_WORK_SM Need to sort out the setting of adjencenies.

            int2 zeroInt2 = { -1, -1 };
            tetMsgArr.resize (m_tetrahedrons.size (), zeroInt2);
            flipArr.resize (doFlips.size ());
            flip23NewSlot.resize (doFlips.size ());
            //                parallel_for_each_ (doFlips.begin (), doFlips.end (), [&](FlipInformation& flip)
            parallel_for ((int)0, (int)doFlips.size (), (int)1, [&](int aindx)
                {
                const FlipInformation& flip = doFlips[aindx];

                DoFlip (flip, aindx, hasFlipped);
                });

            // Update Adjentencies
            parallel_for ((int)0, (int)doFlips.size (), (int)1, [&](int aindx)
                {
                const FlipInformation& flip = doFlips[aindx];
                UpdateAdjenciesAfterFlip (flip, aindx);
                });

            m_numFlipped[0] += n23;
            m_numFlipped[1] += n32;
#ifdef DEBUGMSG
            std::cout << "  Active: " << m_activeTetraNum.size () << " Flip: " << doFlips.size () << " ( 2-3: " << n23 << " 3-2 : " << n32 << " ) Exact : " << (doSpecial ? 0 : -1) << std::endl;
#endif
            UpdatePointsAfterFlip (hasFlipped, doFlips);
            //CheckTetrahedronForPoint ();
            //ValidateTetrahedrons ();
            tetMsgArr.clear ();
            flipArr.clear ();
            flip23NewSlot.clear ();
            // Move the points into the right tetrahedron.
            return true;
            }

        void DoFlipFacetsLoop ()
            {
            bool doSpecial = false;

            // Mark all active tetrahedrons to be checked for flips.
            int retries;
            for (retries = 0; retries < FLIPFACETSRETRIES; retries++)
                {
                if (!FlipFacets (doSpecial))
                    {
                    if (doSpecial)
                        break;

                    // make a note if they are special tetrahedrons.
                    parallel_for_each (m_tetrahedrons.begin (), m_tetrahedrons.end (), [&](Tetrahedron& tet)
                        {
                        if (tet.special)
                            {
                            tet.special = false;
                            tet.isChanged = true;
                            }
                        });
#ifdef ALTERNATEACTIVETETCOLLECTION
                    m_newActiveTetraNum.clear ();
#endif
                    //if (count == 0)
                    //    return;

                    //std::cout << " ---- Special Tets " << count << std::endl;
                    doSpecial = true;
                    }
#ifdef DHDEBUG
                    //ValidateTetrahedrons ();
#endif
                }
                //ValidateTetrahedrons ();
            m_numFlipIterations += retries;
            }
#pragma region Checking Functions

        void CheckTetraAdj (int num)
            {
            CheckTetraAdj (m_tetrahedrons[num], num);
            }

        void CheckTetraAdj (Tetrahedron& t, int num)
            {
            for (int face = 0; face < numFacets; face++)
                {
                int p[3];
                if (t.GetAdjentTet (face) == -1)
                    continue;
                t.GetFacePoints (face, p);
                const Tetrahedron& aT = m_tetrahedrons[t.GetAdjentTet (face)];
                int a = aT.GetFaceSide (p);
                if (aT.GetAdjentTet (a) != num)
                    num = num;
                }
            }
#pragma endregion

        bool IsTetrahedronDelauny (const Tetrahedron& t)
            {
            DTetrahedron3d t3d;
            int tn = &t - m_tetrahedrons.data ();
            t.GetDTetrahedron3d (t3d, m_points);

            for (int i = 0; i < numFacets; i++)
                {
                int adjTetNum = t.GetAdjentTet (i);
                if (adjTetNum == -1)
                    continue;
                const Tetrahedron& at = m_tetrahedrons[adjTetNum];
                if (IsSpecialTet (at))
                    continue;
                int as = at.GetFaceSide (tn);

                DPoint3d pt = m_points[at.ptNums[as]];
                if (IsEdgeDelauny (t3d, pt) <= 0)
                    {
                    return false;
                    }
                }
            return true;
            }

        void InsertationAndFlipping ()
            {
            size_t originalNumPoints = m_points.size ();
            InitialiseTetrahedron ();
            m_doFlipping = true;
            MovePointsIntoTetrahedron ();

            int numIterations = 0;
            TRACE_SPLIT_INIT ()
                while (m_pointsToInsert.size ())
                    {
                    numIterations++;
#ifdef DEBUGMSG
                    std::cout << "Iteration Number " << numIterations << std::endl;
#endif
                    if (!FindInsertPointForTetrahedron ())
                        {
                        TRACE_TETRA_INIT ()
#ifdef LOG_TETRA
                            log << " [InsertationAndFlipping] Couldn't find insert points!" << endl;
#endif
                        TRACE_TETRA_END ()



#ifdef IGNOREPOINTSONFACE1
                            if (m_pointsToInsert.size () != 0)
                                {
                                //                            DoFlipFacetsLoop ();
                                //                            m_skipFlipOnNearZero = false;
                                //                            m_skipFlipOnNearZero = true;
                                DoFlipFacetsLoop ();

                                m_ignorePointsOnFace = false;
                                //while (!m_pointsToInsert.empty ())
                                    {
                                    if (!FindInsertPointForTetrahedron ())
                                        break;
                                    if (!SplitTetrahedronsOnEdgeOrFace ())
                                        {
                                        //if (!FindInsertPointForTetrahedron ())
                                        //    break;

                                        //if (!SplitTetrahedronsOnEdgeOrFace (false))
                                        break;
                                        }
                                    //                                DoFlipFacetsLoop ();
                                    }
                                m_ignorePointsOnFace = true;
                                m_doFlipping = false;
                                if (m_pointsToInsert.empty ())
                                    break;
                                //m_ignorePointsOnFace = true;
                                //if (!FindInsertPointForTetrahedron ())
                                //    break;
                                }
#else
                            break;// This could happen if the points are on the edge of a tetrahedon.
#endif

                        }
                    else
                        SplitTetrahedrons ();
#ifdef DHDEBUG
                    //ValidateTetrahedrons ();
#endif
                    if (m_doFlipping)
                        DoFlipFacetsLoop ();
                    //std::cout << " FlipFacets retried " << retries << "(" << m_numFlipIterations << ")" << std::endl;
                    //std::cout << "Flipped Info 2-3 " << m_numFlipped[0] << ", 3-2 " << m_numFlipped[1] << std::endl;
                    //ReportNonDelaunyStats ();

                    //                return;
                    //ValidateTetrahedrons ();
                    }
            //ValidateTetrahedrons ();
            if (!m_doFlipping)
                DoFlipFacetsLoop ();
            /*  TRACE_SPLIT("[]All Tetrahedrons generated after flipping, before inserting ignored points ")
            std::vector<int> pointsInTet(m_points.size(), -1);
            for (const auto& tet : m_tetrahedrons)
            {
            std::string msg = "[TET# " + std::to_string(&tet - m_tetrahedrons.data()) + "] POINTS {" + std::to_string(tet.ptNums[0]) + "," + std::to_string(tet.ptNums[1]) + "," +
            std::to_string(tet.ptNums[2]) + "," + std::to_string(tet.ptNums[3]) + "} VALUES {(" + std::to_string(m_points[tet.ptNums[0]].x) + "," + std::to_string(m_points[tet.ptNums[0]].y)
            + "," + std::to_string(m_points[tet.ptNums[0]].z) + ") (" + std::to_string(m_points[tet.ptNums[1]].x) + "," + std::to_string(m_points[tet.ptNums[1]].y)
            + "," + std::to_string(m_points[tet.ptNums[1]].z) + ") (" + std::to_string(m_points[tet.ptNums[2]].x) + "," + std::to_string(m_points[tet.ptNums[2]].y)
            + "," + std::to_string(m_points[tet.ptNums[2]].z) + ") (" + std::to_string(m_points[tet.ptNums[3]].x) + "," + std::to_string(m_points[tet.ptNums[3]].y)
            + "," + std::to_string(m_points[tet.ptNums[3]].z) + ")}";
            TRACE_SPLIT(msg)
            pointsInTet[tet.ptNums[3]] = pointsInTet[tet.ptNums[2]] = pointsInTet[tet.ptNums[1]] = pointsInTet[tet.ptNums[0]] = &tet - m_tetrahedrons.data();
            }
            for (int i = 0; i < pointsInTet.size(); i++)
            {
            if (pointsInTet[i] == -1) TRACE_SPLIT("[afterFlip]Point not inserted: " + std::to_string(i))
            }*/


#ifdef DHDEBUG
//            ValidateTetrahedrons ();
//            ReportNonDelaunyStats ();
#endif
            /* TRACE_SPLIT("[]All Tetrahedrons generated after split ")
            std::fill_n(pointsInTet.begin(), m_points.size(), -1);
            for (const auto& tet : m_tetrahedrons)
            {
            std::string msg = "[TET# " + std::to_string(&tet - m_tetrahedrons.data()) + "] POINTS {" + std::to_string(tet.ptNums[0]) + "," + std::to_string(tet.ptNums[1]) + "," +
            std::to_string(tet.ptNums[2]) + "," + std::to_string(tet.ptNums[3]) + "} VALUES {(" + std::to_string(m_points[tet.ptNums[0]].x) + "," + std::to_string(m_points[tet.ptNums[0]].y)
            + "," + std::to_string(m_points[tet.ptNums[0]].z) + ") (" + std::to_string(m_points[tet.ptNums[1]].x) + "," + std::to_string(m_points[tet.ptNums[1]].y)
            + "," + std::to_string(m_points[tet.ptNums[1]].z) + ") (" + std::to_string(m_points[tet.ptNums[2]].x) + "," + std::to_string(m_points[tet.ptNums[2]].y)
            + "," + std::to_string(m_points[tet.ptNums[2]].z) + ") (" + std::to_string(m_points[tet.ptNums[3]].x) + "," + std::to_string(m_points[tet.ptNums[3]].y)
            + "," + std::to_string(m_points[tet.ptNums[3]].z) + ")}";
            TRACE_SPLIT(msg)
            pointsInTet[tet.ptNums[3]] = pointsInTet[tet.ptNums[2]] = pointsInTet[tet.ptNums[1]] = pointsInTet[tet.ptNums[0]] = &tet - m_tetrahedrons.data();
            }
            for(int i = 0; i < pointsInTet.size(); i++)
            {
            if (pointsInTet[i] == -1) TRACE_SPLIT("[]Point not inserted: " + std::to_string(i))
            }
            TRACE_SPLIT_END()*/
            if (m_doSplaying)
                {
                DoSplaying ();
#ifdef DHDEBUG
//            ValidateTetrahedrons ();
//            ReportNonDelaunyStats ();
#endif
                }
            /*
            Data: A point set S
            Result : D (S)
            initializeT with a large enough tetrahedron t
            foreach p ∈ S doinparallel location[p]← t
            while¬Empty (S) do
            foreach p ∈ S doinparallel insert[location[p]]← p
            foreach t ∈T with insert[t]6 = null doinparallel
            split t using insert[t] and remove insert[t] from S
            label all new facets to be checked
            while there are facets to be checked do
            foreach facet f that needs to be checked doinparallel
            if f is locally non - Delaunay and ﬂippable then
            ﬂip f
            label all updated facets to be checked
            end
            update the location of points in S
            end
            returnT
            */
#ifdef DEBUGMSG
            std::cout << "Number Of Iterations" << numIterations << std::endl;
            std::cout << "Number Of Flip Iterations" << m_numFlipIterations << std::endl;
#endif
            }

#pragma region Splaying
        template<class T> void compactIfNegative (T& a)
            {
            int ni = 0;

            for (size_t i = 0; i < a.size (); i++)
                {
                if (a[i] >= 0)
                    {
                    a[ni++] = a[i];
                    }
                }
            a.resize (ni);
            }

        void checkAdjacency (PredWrapper& _predWrapper, GDelOutput* _output)
            {
            const TetHVec tetVec = _output->tetVec;
            const TetOppHVec oppVec = _output->tetOppVec;
            const CharHVec tetInfoVec = _output->tetInfoVec;

            for (int ti0 = 0; ti0 < (int)tetVec.size (); ++ti0)
                {
                if (!isTetAlive (tetInfoVec[ti0])) continue;

                const Tet& tet0 = tetVec[ti0];
                const TetOpp& opp0 = oppVec[ti0];

                for (int vi = 0; vi < 4; ++vi)
                    {
                    if (-1 == opp0._t[vi]) continue;

                    const int ti1 = opp0.getOppTet (vi);
                    const int vi0_1 = opp0.getOppVi (vi);

                    if (!isTetAlive (tetInfoVec[ti1]))
                        {
                        std::cout << "TetIdx: " << ti1 << " is invalid!" << std::endl;
                        exit (-1);
                        }

                    const Tet& tet1 = tetVec[ti1];
                    const TetOpp& opp1 = oppVec[ti1];

                    if (-1 == opp1._t[vi0_1] || ti0 != opp1.getOppTet (vi0_1))
                        {
                        std::cout << "Not opp of each other! Tet0: " << ti0 << " Tet1: " << ti1 << std::endl;
                        //printTetraAndOpp (ti0, tet0, opp0);
                        //printTetraAndOpp (ti1, tet1, opp1);
                        //exit (-1);
                        }

                    if (vi != opp1.getOppVi (vi0_1))
                        {
                        std::cout << "Vi mismatch! Tet0: " << ti0 << "Tet1: " << ti1 << std::endl;
                        //exit (-1);
                        }
                    }
                }

#ifdef DEBUGMSG
            std::cout << "Adjacency check: Pass\n";
#endif
            return;
            }

        void checkOrientation (PredWrapper& _predWrapper, GDelOutput* _output)
            {
            const TetHVec tetVec = _output->tetVec;
            const CharHVec tetInfoVec = _output->tetInfoVec;

            int count = 0;

            for (int i = 1; i < (int)tetInfoVec.size (); ++i)
                {
                if (!isTetAlive (tetInfoVec[i])) continue;

                const Tet& t = tetVec[i];
                const Orient ord = _predWrapper.doOrient3DAdapt (t._v[0], t._v[1], t._v[2], t._v[3]);

                if (OrientNeg == ord)
                    ++count;
                }

#ifdef DEBUGMSG
            std::cout << "Orient check: ";
            if (count)
                std::cout << "***Fail*** Wrong orient: " << count;
            else
                std::cout << "Pass";
            std::cout << "\n";
#endif
            return;
            }
#pragma endregion

        // Compact the
        void compactTetrahedrons ()
            {
            std::vector<int> map;
            map.resize (m_tetrahedrons.size ());

            int ni = 0;

            for (size_t i = 0; i < m_tetrahedrons.size (); i++)
                {
                if (m_tetrahedrons[i].ptNums[0] != -1)
                    {
                    map[i] = ni;
                    m_tetrahedrons[ni++] = m_tetrahedrons[i];
                    }
                }

            m_tetrahedrons.resize (ni);

            for (Tetrahedron& t : m_tetrahedrons)
                {
                for (int i = 0; i < numFacets; i++)
                    if (t.GetAdjentTet (i) != -1)
                        t.SetAdjentTet (i, map[t.GetAdjentTet (i)]);
                }
            }

        bool RunSplaying (Splaying& splaying, GDelOutput& output)
            {
            bool failed = false;
            __try
                {
                failed = !splaying.fixWithStarSplaying (m_points, &output);
                }
            __except (nullptr, EXCEPTION_EXECUTE_HANDLER)
                {
                failed = true;
                }
            return failed;
            }

        // Do the Splaying.
        void DoSplaying ()
            {
#ifdef DHDEBUG
            static int numSplays = 0;
            static int numFailedSplays = 0;
#endif
            // NEEDS_WORK_SM Need to change the Splaying to work directly with our structure, this will also fix it so that it works on multiple
            m_activeTetraNum.clear ();
            m_activeTetraNum.shrink_to_fit ();
            m_flipTestNumber.clear ();
            m_flipTestNumber.shrink_to_fit ();
            m_pointNumberToInsert.clear ();
            m_pointNumberToInsert.shrink_to_fit ();
            m_pointsToInsert.clear ();
            m_pointsToInsert.shrink_to_fit ();
            m_pointTetrahedron.clear ();
            m_pointTetrahedron.shrink_to_fit ();
            m_reuseTetrahedrons.clear ();
            m_reuseTetrahedrons.shrink_to_fit ();

            // Do Splaying
            Splaying splaying;

            GDelOutput output;
            PredWrapper _predWrapper;
            output.ptInfty = m_points[m_infPtNum];
            _predWrapper.init (m_points, output.ptInfty);
            _predWrapper._infIdx = m_infPtNum;
            _predWrapper._pointNum--;
            compactTetrahedrons ();
            output.tetVec.resize (m_tetrahedrons.size ());
            output.tetOppVec.resize (output.tetVec.size ());
            output.tetInfoVec.resize (output.tetVec.size ());
            output.failVertVec.resize (m_points.size (), -1);
            output.vertTetVec.resize (m_points.size (), 0);
            TRACE_TETRA_INIT ()

                parallel_for ((int)0, (int)m_tetrahedrons.size (), (int)1, [&](int i)
                {
                const Tetrahedron& t = m_tetrahedrons[i];
                int tetNum = i;
                TetOpp& tetOpp = output.tetOppVec[tetNum];
                Tet& tet = output.tetVec[tetNum];
                if (t.ptNums[0] != -1)
                    {
                    //attempt to remove degenerate tetrahedrons before splaying
                    DPoint3d ptA = m_points[t.ptNums[3]], ptB = m_points[t.ptNums[2]], ptC = m_points[t.ptNums[1]], ptD = m_points[t.ptNums[0]];
                    bool isDegenerate = false;
                    Orient ort = doOrient3D (t.ptNums[3], t.ptNums[2], t.ptNums[1], t.ptNums[0], ptA, ptB, ptC, ptD);
                    if (OrientZero == ort)
                        isDegenerate = true;
                    if (!isDegenerate)
                        {
                        DVec3d edge1, edge2, edge3, edge4;
                        double mag1, mag2, mag3, mag4;
                        edge1.DifferenceOf (ptA, ptB);
                        edge2.DifferenceOf (ptB, ptC);
                        edge3.DifferenceOf (ptC, ptD);
                        edge4.DifferenceOf (ptD, ptA);
                        mag1 = edge1.Magnitude ();
                        mag2 = edge2.Magnitude ();
                        mag3 = edge3.Magnitude ();
                        mag4 = edge4.Magnitude ();
                        double maxVal = DoubleOps::Max (mag1, mag2, mag3, mag4);
                        double minVal = DoubleOps::Min (mag1, mag2, mag3, mag4);
                        if (minVal / maxVal < 0.25)
                            isDegenerate = true;
                        }
                    for (int j = 0; j < 4; j++)
                        {
                        int vi = j; // Tetrahedron::GetOtherPoint_ (j);

                        tet._v[j] = t.ptNums[j];

                        int adjTet = t.GetAdjentTet (j);
                        if (adjTet != -1)
                            {
                            int adjSide = m_tetrahedrons[adjTet].GetFaceSide (i);
                            if (t.GetInternalOpp (j))
                                tetOpp.setOppInternal (vi, adjTet, adjSide);
                            else
                                tetOpp.setOpp (vi, adjTet, adjSide);
                            }
                        }
                    //if (isDegenerate)
                    //    {
                    //    tetOpp.setOppSphereFail (0);
                    //    tetOpp.setOppSphereFail (1);
                    //    tetOpp.setOppSphereFail (2);
                    //    tetOpp.setOppSphereFail (3);
                    //    }
                    isDegenerate = false;
                    if (isDegenerate)
                        {
                        setTetAliveState (output.tetInfoVec[tetNum], false);
                        }
                    else
                        {
                        setTetAliveState (output.tetInfoVec[tetNum], true);
                        }
                    }
                else
                    setTetAliveState (output.tetInfoVec[tetNum], false);
                });

                {
                AutoTimer timer (m_timeToSplay);
                parallel_for ((int)0, (int)output.tetOppVec.size (), (int)1, [&](int tetIdx)
                    {
                    TetOpp& tetOpp = output.tetOppVec[tetIdx];
                    Tet& tet = output.tetVec[tetIdx];
                    int failVi = -1;
                    int win = 0;

                    // Get out immediately if > 1 failures
                    DTetrahedron3d dtet (m_points[tet._v[0]], m_points[tet._v[1]], m_points[tet._v[2]], m_points[tet._v[3]]);
                    for (int vi = 0; vi < 4; ++vi)
                        {
                        if (tetOpp.getOppTet (vi) < tetIdx)
                            win |= (1 << vi);

                        //int opPt = output.tetVec[tetOpp.getOppTet (vi)]._v[tetOpp.getOppVi (vi)];
                        //const Side side = doInSphereSoS (m_tetrahedrons[tetIdx], opPt, dtet, m_points[opPt]);
                        //if (side != SideIn/* && !tetOpp.isOppSphereFail (vi)*/)
                        if (!m_tetrahedrons[tetIdx].GetFailedDelaunay (vi) && !tetOpp.isOppSphereFail (vi))
                            {
                            //const Tetrahedron& st = m_tetrahedrons[tetIdx];
                            //if (IsSpecialTet (st))
                            //    continue;
                            //DTetrahedron3d tetra;
                            //st.GetDTetrahedron3d (tetra, m_points);

                            //Orient ort = doOrient3D (st.ptNums[0], st.ptNums[1], st.ptNums[2], st.ptNums[3], tetra.points[0], tetra.points[1], tetra.points[2], tetra.points[3]);
                            //////                const Orient ord = _predWrapper.doOrient3DAdapt (t._v[0], t._v[1], t._v[2], t._v[3]);

                            //if (ort == OrientPos)
                            //    continue;
                            //else if (ort == OrientZero)
                            //    ort = ort;
                            continue;
                            }

                        if (IsSpecialTet (m_tetrahedrons[tetIdx]))
                            continue;
                        //if (IsSpecialTet (m_tetrahedrons[tetOpp.getOppTet (vi)]))
                        //    continue;
                        tetOpp.setOppSphereFail (vi);
                        failVi = (-1 == failVi) ? vi : 4;
                        }

                    //                    const Tet tet = loadTet (tetVec._arr, tetIdx);

                    // Write
                    for (int vi = 0; vi < 4; ++vi)
                        {
                        int vert = tet._v[vi];

                        if (-1 != failVi && vi != failVi)
                            output.failVertVec[vert] = vert;

                        if ((win | (1 << vi)) == 0x0F)
                            output.vertTetVec[vert] = tetIdx;
                        }
                    }
                );

                compactIfNegative (output.failVertVec);

                //                    checkForUnreachableTetrahedrons(output.failVertVec);
#ifndef VALIDATION_CHECK
                checkOrientation (_predWrapper, &output);
                checkAdjacency (_predWrapper, &output);
#endif
                //                    output.failVertVec.resize (1);
                m_points.resize (m_points.size () - 1);
#ifdef DHDEBUG
                numSplays++;
#endif
                bool failed = RunSplaying (splaying, output);
#ifdef DHDEBUG
                if (failed)
                    std::cout << "Splaying Failed" << std::endl;
                else
                    std::cout << "Splaying Succeeded" << std::endl;
#endif
                m_points.push_back (output.ptInfty);
                if (failed)
                    {
#ifdef DHDEBUG
                    numFailedSplays++;
                    std::cout << "Splaying Stats " << numSplays << " - " << numFailedSplays << std::endl;

#endif
                    return;
                    }
                }
#ifdef DHDEBUG
                std::cout << "Splaying Stats " << numSplays << " - " << numFailedSplays << std::endl;
#endif
            int removeExtraTetrahedronNum = -1;
            m_tetrahedrons.resize ((int)output.tetVec.size ());

            //parallel_for ((int)0, (int)m_tetrahedrons.size (), (int)1, [&](int i)
            //    {
            //    if (isBitSet (output.tetInfoVec[i], 0))
            //        {
            //        for (int j = 0; j < 4; j++)
            //            {
            //            if (output.tetVec[i]._v[j] == m_points.size () - 1)
            //                {
            //                setTetAliveState (output.tetInfoVec[i], false);
            //                break;
            //                }
            //            }
            //        }
            //    });

            parallel_for ((int)0, (int)m_tetrahedrons.size (), (int)1, [&](int i)
                {
                bool deleteIt = false;
                if (isBitSet (output.tetInfoVec[i], 0))
                    {
                    for (int j = 0; j < 4; j++)
                        {
                        m_tetrahedrons[i].ptNums[j] = output.tetVec[i]._v[j];
                        //assert(m_tetrahedrons[i].ptNums[j] > 0);
#ifdef LOG_TETRA
                        if (m_tetrahedrons[i].ptNums[j] > m_points.size () - 1 || m_tetrahedrons[i].ptNums[j] < 0)
                            {
                            log << "[DoSplaying] AT TET#" + std::to_string (i) + " POINT#" + std::to_string (j) + " INVALID VALUE " + std::to_string (m_tetrahedrons[i].ptNums[j]) << endl;
                            }
#endif
                        //if (m_tetrahedrons[i].ptNums[j] == m_points.size () - 1)
                        //    deleteIt = true;
                        const int helper[] = { 2, 3, 1, 0 }; //{ 3, 2, 0, 1 };
                        int adjTet = output.tetOppVec[i].getOppTet (j);

                        if (isTetAlive (output.tetInfoVec[adjTet]))
                            m_tetrahedrons[i].SetAdjentTet (j, adjTet);
                        else
                            m_tetrahedrons[i].SetAdjentTet (j, -1);
                        }
                    }
                else
                    deleteIt = true;

                if (deleteIt)
                    m_tetrahedrons[i].ptNums[0] = m_tetrahedrons[i].ptNums[1] = m_tetrahedrons[i].ptNums[2] = m_tetrahedrons[i].ptNums[3] = -1;
                });
            compactTetrahedrons ();
            TRACE_TETRA_END ()
                //ValidateTetrahedrons ();
                //ReportNonDelaunyStats ();
            }

        // Create the tetrahedron set.
        void Construct ()
            {
            Timer a;
            a.start ();
            InsertationAndFlipping ();
            a.stop ();

            std::cout << std::setprecision (8) << "Took" << " elapsed_time=" << a.read () << "(sec.)\n";
#ifdef DEBUGMSG

            int numTets = 0;
            for (const Tetrahedron& t : m_tetrahedrons)
                {
                if (t.ptNums[0] != -1 && !IsSpecialTet (t))
                    numTets++;
                }
            std::cout << "Created " << numTets << " Tetrahedrons" << std::endl;
            m_timeToMovePointsToTetrahedron.Print ("timeToMovePointsToTetrahedron");
            m_timeToFindPointForTetrahedron.Print ("timeToFindPointForTetrahedron");
            m_timeToInsert.Print ("timeToInsert");
            m_timeToCleanupPoints.Print ("timeToCleanupPoints");
            m_timeToFixUpAdjectEdges.Print ("m_timeToFixUpAdjectEdges");
            m_timeToFlipFacets.Print ("timeToFlipFacets");

            m_timeToExamineFacetsForFlip.Print ("timeToExamineFacetsForFlip");
            m_timeToDoFlip.Print ("timeToDoFlip");
            m_timeToFixPointsAfterFlip.Print ("timeToFixPointsAfterFlip");
            m_timeToSplay.Print ("timeToSplay");
            std::cout << "Flipped Info 2-3 " << m_numFlipped[0] << ", 3-2 " << m_numFlipped[1] << std::endl;
#endif

            //ValidateTetrahedrons ();
            }

        static bool CheckTetrahedronAdjAndPoints (const Tetrahedron& t)
            {
            for (int i0 = 0; i0 < numFacets; i0++)
                {
                for (int i1 = i0 + 1; i1 < numFacets; i1++)
                    {
                    if (t.ptNums[i0] == t.ptNums[i1])
                        return false;
                    if (t.GetAdjentTet (i0) == t.GetAdjentTet (i1))
                        return false;
                    }
                }
            return true;
            }

        int CheckTetrahedron (int aindx)
            {
            int numErrors = 0;
            if (aindx >= (int)m_tetrahedrons.size ())
                return numErrors;
            const Tetrahedron& st = m_tetrahedrons[aindx];
            bool isSpecial = IsSpecialTet (st);
            DTetrahedron3d tetra;

            if (st.ptNums[0] == -1)
                return numErrors;

            for (int i0 = 0; i0 < numFacets; i0++)
                {
                for (int i1 = i0 + 1; i1 < numFacets; i1++)
                    {
                    if (st.ptNums[i0] == st.ptNums[i1])
                        numErrors++;
                    }
                int aTetNum = st.GetAdjentTet (i0);
                if (aTetNum != -1)
                    {
                    const Tetrahedron& aTet = m_tetrahedrons[aTetNum];
                    int pNum = aTet.ptNums[aTet.GetFaceSide (aindx)];
                    for (int i1 = 0; i1 < numFacets; i1++)
                        {
                        if (pNum == st.ptNums[i1])
                            numErrors++;
                        }
                    }
                }
            //double v = tetra.Volume ();

            //if (v < 0.000001)
            //    numErrors.local ()++;
            for (int i = 0; i < numFacets; i++)
                {
                if (st.ptNums[i] >= (int)m_points.size ())
                    numErrors++;
                int adjTetNum = st.GetAdjentTet (i);
                if (adjTetNum == -1)
                    continue;
                Tetrahedron& adjTet = m_tetrahedrons[adjTetNum];
                bool isAdjSpecial = IsSpecialTet (adjTet);
                int edgePts[3];
                st.GetFacePoints (i, edgePts);
                int faceSide = adjTet.GetFaceSide (edgePts, -1); // isSpecial != isAdjSpecial ? 1 : -1);
                if (faceSide == -1)
                    numErrors++;
                else if (adjTet.GetAdjentTet (faceSide) != aindx)
                    numErrors++;
                }

            st.GetDTetrahedron3d (tetra, m_points);

            Orient ort = doOrient3D (st.ptNums[3], st.ptNums[2], st.ptNums[1], st.ptNums[0], tetra.points[3], tetra.points[2], tetra.points[1], tetra.points[0]);
            //                const Orient ord = _predWrapper.doOrient3DAdapt (t._v[0], t._v[1], t._v[2], t._v[3]);
            if (!IsSpecialTet (st))
                {
                if (ort == OrientNeg)
                    {
//                    std::cout << "Failed indx " << aindx << std::endl;
                    numErrors++;
                    }
                else if (ort == OrientZero)
                    {
//                    std::cout << "OnEdge  indx " << aindx << std::endl;
                    //numErrors++;
                    }
                }
            return numErrors;
            }
        void ValidateTetrahedrons ()
            {
            concurrency::combinable<int> numErrors;
            parallel_for ((int)0, (int)m_tetrahedrons.size (), (int)1, [&](int aindx)
                {
                numErrors.local () += CheckTetrahedron (aindx);
                }
            );
            int totalNumErrors = 0;
            numErrors.combine_each ([&](int num)
                {
                totalNumErrors += num;
                });

            if (totalNumErrors != 0)
                std::cout << "Num Errors " << totalNumErrors << std::endl;

            }

        void ReportNonDelaunyStats ()
            {
            combinable<int> nonDelaunyCombinable;
            combinable<int> totalCombinable;
            parallel_for_each (m_tetrahedrons.begin (), m_tetrahedrons.end (), [&](Tetrahedron& t)
                {
                if (t.ptNums[0] == -1 || IsSpecialTet (t))
                    return;
                //if (t.ptNums[0] < numInitialPoints || t.ptNums[1] < numInitialPoints || t.ptNums[2] < numInitialPoints || t.ptNums[3] < numInitialPoints)
                //    return;
                totalCombinable.local ()++;
                if (!IsTetrahedronDelauny (t))
                    {
                    nonDelaunyCombinable.local ()++;
                    }
                });
            int nonDelaunyCount = 0;
            nonDelaunyCombinable.combine_each ([&](int count)
                {
                nonDelaunyCount += count;
                });
            int totalCount = 0;
            totalCombinable.combine_each ([&](int count)
                {
                totalCount += count;
                });
#ifdef DEBUGMSG
            std::cout << "Non Delauny " << nonDelaunyCount << " Total " << totalCount << " (" << ((double)nonDelaunyCount / (double)totalCount) * 100 << "%)" << std::endl;
#endif
            }

        void SetData (int numt, TetOpp* tetOpp, Tet* t, char* tetInfo, double* points, int numPoints, int* hVec)
            {
            m_infPtNum = numPoints - 1;
            m_points.resize (numPoints);
            for (int i = 0; i < numPoints; i++)
                {
                m_points[i].x = *points++;
                m_points[i].y = *points++;
                m_points[i].z = *points++;
                }
            m_tetrahedrons.resize (numt);
            CharHVec tetInfoVec;
            tetInfoVec.resize (numt);
            memcpy (tetInfoVec.data (), tetInfo, numt);

            parallel_for ((int)0, (int)m_tetrahedrons.size (), (int)1, [&](int i)
                {
                for (int vi = 0; vi < 4; vi++)
                    {
                    if (tetOpp[i].isOppSphereFail (vi))
                        {
                        m_tetrahedrons[i].SetFailedDelaunay (vi);
                        }
                    if (tetOpp[i].isOppInternal (vi))
                        m_tetrahedrons[i].SetInternalOpp (vi);
                    }
                if (isBitSet (tetInfoVec[i], 0))
                    {
                    for (int j = 0; j < 4; j++)
                        {
                        m_tetrahedrons[i].ptNums[j] = t[i]._v[j];
                        //                        if (m_tetrahedrons[i].ptNums[j] < 0 || m_tetrahedrons[i].ptNums[j] >= numPoints)
                        //                            j = j;
                        int adjTet = tetOpp[i].getOppTet (j);

                        if (isTetAlive (tetInfoVec[adjTet]))
                            m_tetrahedrons[i].SetAdjentTet (j, adjTet);
                        else
                            m_tetrahedrons[i].SetAdjentTet (j, -1);
                        }
                    }
                else
                    {
                    m_tetrahedrons[i].ptNums[0] = m_tetrahedrons[i].ptNums[1] = m_tetrahedrons[i].ptNums[2] = m_tetrahedrons[i].ptNums[3] = -1;
                    }
                });
            //            ValidateTetrahedrons ();
            DoSplaying ();
            }
    };

const bool Mesh::m_skipFlipOnNearZero = true;
const bool Mesh::m_ensureSameResults = true;
const bool Mesh::m_alwaysFlip = true;
const bool Mesh::m_doSplaying = false;
const bool Mesh::m_insphere = true;       // Uses insphere or a different method to calculate the best point to use.
const bool Mesh::m_addOuterBox = true;   // Improves flatish areas.

void DoItCallBack (int numt, TetOpp* tetOpp, Tet* t, char* tetInfo, double* points, int numPoints, int* fv, int fvSize, int* hVec, int hVecSize, void* arg)
    {
    /*
    Splaying splaying;
    DPoint3dHVec mPoints;
    mPoints.resize (numPoints);
    memcpy (mPoints.data (), points, numPoints * sizeof (points[0]) * 3);
    GDelOutput output;
    PredWrapper _predWrapper;
    output.ptInfty = mPoints[numPoints - 1];
    mPoints.resize (numPoints - 1);
    _predWrapper.init (mPoints, output.ptInfty);
    _predWrapper._infIdx = numPoints - 1;
    //    compactTetrahedrons ();
    output.tetVec.resize (numt);
    output.tetOppVec.resize (numt);
    output.tetInfoVec.resize (numt);
    output.failVertVec.resize (fvSize);
    output.vertTetVec.resize (hVecSize);

    memcpy (output.tetVec.data (), t, numt * sizeof (t[0]));
    memcpy (output.tetOppVec.data (), tetOpp, numt * sizeof (tetOpp[0]));
    memcpy (output.tetInfoVec.data (), tetInfo, numt * sizeof (tetInfo[0]));
    memcpy (output.failVertVec.data (), fv, fvSize * sizeof (fv[0]));
    memcpy (output.vertTetVec.data (), hVec, hVecSize * sizeof (hVec[0]));

    splaying.fixWithStarSplaying (mPoints, &output);
    Mesh* meshP = (Mesh*)arg;
    meshP->SetData (numt, tetOpp, t, tetInfo, points, numPoints, hVec);
*/
    }

int FixNList (int v, TetGen::tetgenio& out)
    {
    v--;
    if (v < 0 || v >= out.numberoftetrahedra)
        return -1;
    return v;
    }
int FixTPt (int v, TetGen::tetgenio& out)
    {
    v--;
    if (v < 0 || v >= out.numberofpoints)
        return -1;
    return v;
    }
void DoTetGen (MeshData& meshData, DPoint3dCP points, int numPoints)
    {
    meshData.m_points.resize (numPoints);
    meshData.m_infPtNum = -1;
    memcpy (meshData.m_points.data (), points, sizeof (points[0]) * numPoints);
    meshData.m_ignorePtsAfterNum = numPoints;
    for (int dataSetPtNum = 0; dataSetPtNum < numPoints; dataSetPtNum++)
        {
        DPoint3d pt = points[dataSetPtNum];
        if (dataSetPtNum == 0)
            {
            meshData.m_extents.low.x = meshData.m_extents.high.x = pt.x;
            meshData.m_extents.low.y = meshData.m_extents.high.y = pt.y;
            meshData.m_extents.low.z = meshData.m_extents.high.z = pt.z;
            }
        else
            {
            if (meshData.m_extents.low.x > pt.x)
                meshData.m_extents.low.x = pt.x;
            else if (meshData.m_extents.high.x < pt.x)
                meshData.m_extents.high.x = pt.x;

            if (meshData.m_extents.low.y > pt.y)
                meshData.m_extents.low.y = pt.y;
            else if (meshData.m_extents.high.y < pt.y)
                meshData.m_extents.high.y = pt.y;

            if (meshData.m_extents.low.z > pt.z)
                meshData.m_extents.low.z = pt.z;
            else if (meshData.m_extents.high.z < pt.z)
                meshData.m_extents.high.z = pt.z;
            }
        }

    if (true)
        {
        static const double expandValue = 0.01;
        static const double maxValue = 30; // 1e40;
        DRange3d extents = meshData.m_extents;

        extents.low.x -= expandValue; extents.low.y -= expandValue; extents.low.z -= expandValue;
        extents.high.x += expandValue; extents.high.y += expandValue; extents.high.z += expandValue;
        const double xLen = extents.XLength();
        const double yLen = extents.YLength();
        const double zLen = extents.ZLength();

        // Make the smallest len just have the extents implemented.
        int numZ = 1; // (zLen / maxValue) + 1;
        double stepZ = (extents.high.z - extents.low.z) / numZ;
        for (int z = 0; z <= numZ; z++)
            {
            int numX = 1;
            int numY = 1;
            if (z == 0 || z == (numZ - 1))
                {
                numX = (xLen / maxValue) + 1;
                numY = (yLen / maxValue) + 1;
                }
            double stepX = (extents.high.x - extents.low.x) / numX;
            double stepY = (extents.high.y - extents.low.y) / numY;
            for (int x = 0; x <= numX; x++)
                {
                for (int y = 0; y <= numY; y++)
                    {
                    meshData.m_points.push_back (DPoint3d::From (extents.low.x + (stepX * x), extents.low.y + (stepY * y), extents.low.z + (stepZ * z)));
                    }
                }
            }
        }
        TetGen::tetgenio in, out;

    // All indices start from 1.
    in.firstnumber = 1;

    in.numberofpoints = (int)meshData.m_points.size ();
    in.numberofpointattributes = 0;
    in.pointlist = &meshData.m_points.data()->x;

    TetGen::tetgenbehavior b;
//    b.verbose = 1;
    b.nobound = 1;
    b.nomergevertex = 1;
    b.docheck = 0;
    b.nomergefacet = 1;
    b.nobisect = 1;
//    b.metric = 1;
    b.neighout = 1;
    b.quiet = 1; // 1;

    Timer a;
    a.start ();

    //b.quiet = 0;
    //b.verbose = 1;
    //b.docheck = 1;
    TetGen::tetrahedralize (&b, &in, &out);

    // the out points may be different. not sure where the ignore points are in posision.
    meshData.m_tetrahedrons.resize (out.numberoftetrahedra);
    meshData.m_points.resize (out.numberofpoints);
    memcpy (meshData.m_points.data (), out.pointlist, sizeof (DPoint3d) * out.numberofpoints);
    int* tPt = out.tetrahedronlist;
    int* nList = out.neighborlist;
    for (int i = 0; i < out.numberoftetrahedra; i++)
        {
        Tetrahedron& t = meshData.m_tetrahedrons[i];
        t.ptNums[0] = FixTPt(*tPt++, out);
        t.ptNums[1] = FixTPt (*tPt++, out);
        t.ptNums[2] = FixTPt (*tPt++, out);
        t.ptNums[3] = FixTPt (*tPt++, out);
        t.SetAdjentTet (0, FixNList(*nList++, out));
        t.SetAdjentTet (1, FixNList (*nList++, out));
        t.SetAdjentTet (2, FixNList (*nList++, out));
        t.SetAdjentTet (3, FixNList (*nList++, out));
//        tPt += 6;
        }
    in.pointlist = nullptr;
    a.stop ();
    std::cout << std::setprecision (8) << "TetGen took " << " elapsed_time=" << a.read () << "(sec.)\n";
    }

__declspec(dllexport)
void Create3dDelaunayMesh (DPoint3dCP points, int numPoints, int (*draw) (DTMFeatureType dtmFeatureType, int numTriangles, int numMeshPts, DPoint3d *meshPtsP, DPoint3d *meshVectorsP, int numMeshFaces, long *meshFacesP, void *userP), void* userP, MTGGraph* graphP, bool tetGen, double trimLength)
    {
#ifdef DEBUGMSG
    cout << "Num Points " << numPoints << endl;
#endif

#ifdef DHDEBUG
    tetGen = true;
#endif

#ifdef DHDEBUG
    char fileName[100];
    static int sRunNum = 0;
    static int64_t sTotalNumPoints = 0;

    sTotalNumPoints += numPoints;
    sprintf (fileName, "d:\\temp\\testptsdensiy%d.xyz", ++sRunNum);
    //FILE* fp = fopen (fileName, "wb");
    //fwrite (points, sizeof (points[0]), numPoints, fp);
    //fclose (fp);
#endif
#ifdef REMOVEDUPLICATEPOINTS
    bvector<DPoint3d> newPoints;
    bvector<DPoint3d> data;
    VectorOps<DPoint3d>::Append (&data, points, numPoints);
    parallel_sort (data.begin (), data.end (), [](DPoint3d& a, DPoint3d& b)
        {
        if (a.x < b.x) return true;
        if (a.x > b.x) return false;
        if (a.y < b.y) return true;
        if (a.y > b.y) return false;
        return (a.z < b.z);
        });

    newPoints.reserve (numPoints);
    DPoint3d prevP = DPoint3d::From (1e100, 1e-100, 3e45);
    for (DPoint3d& p : data)
        {
        if (!p.IsEqual (prevP))
            {
            prevP = p;
            newPoints.push_back (p);
            }
        }
    numPoints = (int)newPoints.size ();
    points = newPoints.data ();
#endif
    static bool useNewTrimming = true;

    if (useNewTrimming)
        {
        Timer a;
        a.start ();
        NewTrim trim ((DPoint3dP)points, numPoints);
        trim.Trim ();
        //trim.Method5 ();
        a.stop ();
        std::cout << std::setprecision (8) << "Trimming Took" << " elapsed_time=" << a.read () << "(sec.)\n";
        trim.WriteMeshCallBack (draw, userP, graphP);
        return;
        }

    if (tetGen)
        {
        MeshData meshData;

        Timer a;
        a.start ();
        DoTetGen (meshData, points, numPoints);

#ifdef DHDEBUG
        //Mesh mesh (meshData); // dataSet);
        //mesh.ValidateTetrahedrons ();
        //mesh.ReportNonDelaunyStats ();
#endif

        TrimHull trimmer (meshData, trimLength);

        trimmer.TestMethod ();
        a.stop ();
        std::cout << std::setprecision (8) << "Trimming Took" << " elapsed_time=" << a.read () << "(sec.)\n";
        trimmer.WriteMeshCallBack (draw, userP, graphP);

        return;
        }

#ifdef I
    static HMODULE gDelMod = nullptr;
    static void (*DoIt) (const double* points, int numPoints, void (*t)(int numt, TetOpp* tetOpp, Tet* t, char* tetInfo, double* points, int numPoints, int* fv, int fvSize, int* hVec, int hVecSize, void* arg), void* arg) = nullptr;

    if (gDelMod == nullptr)
        {
//        gDelMod = LoadLibrary ("D:\\Scalable Mesh\\gdel3d\\gFlip3D-Release_271\\x64\\Debug\\GDelFlipping.dll");
        gDelMod = LoadLibrary ("D:\\Scalable Mesh\\gdel3d\\gFlip3D-Release_271\\x64\\Release\\GDelFlipping.dll");
        DoIt = (void (*) (const double* points, int numPoints, void (*t)(int numt, TetOpp* tetOpp, Tet* t, char* tetInfo, double* points, int numPoints, int* fv, int fvSize, int* hVec, int hVecSize, void* arg), void* arg))GetProcAddress (gDelMod, "DoIt"); // ? DoIt@@YAXPEBNHP6AXHPEAUTetOpp@@PEAUTet@@PEADPEANHPEAX@Z5@Z");
        }
    if (DoIt)
        {
        // Do a test run to initialze the Cuda stuff.
        //{
        //MeshData meshData;
        //Mesh mesh2 (meshData, points, 100); // dataSet);
        //DoIt ((const double*)&points[0].x, -100, DoItCallBack, &mesh2);
        //}
        MeshData meshData;
        Mesh mesh (meshData, points, numPoints); // dataSet);
        Timer a;
        a.start ();
        DoIt ((const double*)&points[0].x, numPoints, DoItCallBack, &mesh);
        a.stop ();

        std::cout << std::setprecision (8) << "GPU Took" << " elapsed_time=" << a.read () << "(sec.)\n";
        //        mesh.TrimAndSave (draw, userP);
        //return;
        }
#endif

#ifdef DEBUGMSG
    std::cout << "NumPoints " << numPoints << std::endl;
#endif
    MeshData meshData;
    Mesh mesh (meshData, points, numPoints); // dataSet);
    mesh.Construct ();
    TrimHull trimmer (meshData);

    Timer a;
    a.start ();
    trimmer.TestMethod ();
    a.stop ();

    std::cout << std::setprecision (8) << "Trimming Took" << " elapsed_time=" << a.read () << "(sec.)\n";

    trimmer.WriteMeshCallBack (draw, userP, graphP);
    }

// ToDo
// Fix New examineToFlip process.
// Implement function to adjust tetrahedrons afterwards, so we can parallize the flipping.
// Work out why the splaying doesn't work.
// Clean up code.
// Fix trimming for G3dDel.


// Look at getting the active tetrahedrons from the flipping list, store the ones that where ignored, and then add the flipping ones. should speed it up at the end.




// ToDo
// Implement 4-4 flipping
// Write a function to remove the point from a non volume tetrahedron, so it will remove 3 tetrahedrons and modify 1 (4-1), then add the point to the points to insert.