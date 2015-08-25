#pragma once

#include "ScalableMeshCao.h"
#include <Mtg/MtgStructs.h>

struct PointOnEdgeWithEdge
    {
    int a;
    int b;
    int ptNum;
    int tetraNum;
    int face;
    bool ignore;

    PointOnEdgeWithEdge ()
        {
        ignore = false;
        }
    PointOnEdgeWithEdge (int a, int b, PointOnEdge& pointOnEdge)
        {
        this->a = a;
        this->b = b;
        this->ptNum = pointOnEdge.ptNum;
        this->tetraNum = pointOnEdge.tetraNum;
        this->face = pointOnEdge.face;
        ignore = false;
        }
    PointOnEdgeWithEdge (int a, int b, int ptNum, int tetNum, int face)
        {
        this->a = a;
        this->b = b;
        this->ptNum = ptNum;
        this->tetraNum = tetNum;
        this->face = face;
        ignore = false;
        }
    };

struct Umbrella : public bvector<int>
    {
    bool containsEdge(const int& a, const int& b)
        {
        bool edgeFound = false;
        for (size_t i = 0; i < this->size() && !edgeFound; i++)
            {
            if ((*this)[i] == a && (*this)[(i + 1) % this->size()] == b
                || (*this)[i] == b && (*this)[(i + 1) % this->size()] == a)
                {
                edgeFound = true;
                }
            }
        return edgeFound;
        }
    };

struct UmbrellaFaceInfo
    {
    int points[3];
    int id;
    int tetrahedron;
    double lambda_lower;
    double lambda_upper;
    double quality;
    bool keep = false;
    bool isUmbrella = false;
    int centerPoint = -1;
    int fmatch = 0;
    int vmatch[3];
    UmbrellaFaceInfo()  { for(auto& vm : vmatch) vm = 0; }
    void evaluateFacetMatching(map<int, Umbrella>& umbrellas)
        {
        for (auto pt : points)
            {
            std::vector<int> edge;
            int ptFaceIndex = -1;
            for (int i = 0; i < 3; i++)
                {
                if (points[i] != pt) edge.push_back(points[i]);
                else ptFaceIndex = i;
                }
            if (umbrellas[pt].containsEdge(edge[0], edge[1]))
                {
                if (!vmatch[ptFaceIndex]) ++fmatch;
                vmatch[ptFaceIndex] = 1;
                }
            else {
                //if (fmatch < 3) 
                //    {
                    if (vmatch[ptFaceIndex]) --fmatch;
                    vmatch[ptFaceIndex] = 0;
                //    }
                }
            }
        }
    };


struct TrimTetrahedronInfo
    {
    char fixedFace;
    };

struct FixedFaceInfo
    {
    int tetNum;
    int face;
    int adjFace[3];
    DVec3d normal;

    bool operator==(const FixedFaceInfo& ffi) const
        {
        return this->tetNum == ffi.tetNum && this->face == ffi.face;
        }
    };

struct MarkEdge
    {
    char count;
    int tet;
    int ffi;
    bool done;
    int faceIndex[2];
    MarkEdge ()
        {
        count = 0;
        ffi = -1;
        done = false;
        faceIndex[0] = faceIndex[1] = -1;
        }

    MarkEdge (int count, int tet) : count(count), tet (tet)
        {
        ffi = -1;
        done = false;
        faceIndex[0] = faceIndex[1] = -1;
        }

    bool IsValid ()
        {
        return tet != -1;
        }
    static inline uint64_t MakeIndex (int a, int b)
        {
        struct
            {
            union
                {
                int v[2];
                uint64_t i;
                };
            } v;

        if (a < b)
            {
            v.v[0] = a;
            v.v[1] = b;
            }
        else
            {
            v.v[0] = b;
            v.v[1] = a;
            }
        return v.i;
        }
    };

struct MarkFace
    {
    char count;

    MarkFace()
        {
        count = 0;
        }

    MarkFace(int count) : count (count)
        {
        }

    static inline uint64_t MakeIndex (int a, int b, int c)
        {
        if (b < a)
            {
            if (c < b)
                std::swap (a, c);
            else
                std::swap (a, b);
            }
        else if (c < a)
            std::swap (a, c);

        if (c < b)
            std::swap (b, c);

        return c + (b * 1000000) + (a * 1000000);
        }
    };

struct TrimHull
    {
    private:
        struct edge
            {
            int a;
            int b;
            int c;
            int tetraIndx;
            edge (int a, int b, int c, int tetraIndx)
                {
                this->a = a;
                this->b = b;
                this->c = c;
                this->tetraIndx = tetraIndx;
                }
            };

        struct TrimRun
            {
            std::vector<edge> edges;
            };

        std::vector<TrimRun> m_runs;
        std::vector<TrimRun> m_newRuns;
    public:
    static bool m_checkMarkedEdges;
    static bool m_removeLongFaces;
    static bool m_getTrimLengthFromExtents;
    static bool m_trimFromEdge;
    bool m_useSliverForTrim;
    double m_maxL;
    double m_maxL2;
    MeshData& m_meshData;
    int m_infPtNum;
    int m_ignorePtsAfterNum;
    SMPointList& m_points;
    bvector<int> m_tetrahedronForPoints;
    TetrahedronList& m_tetrahedrons;

    std::vector<TrimTetrahedronInfo> m_trimInfo;
    std::vector<FixedFaceInfo> m_fixedFaces;
    bmap<uint64_t, MarkEdge> m_markEdges;
    bvector<DVec3d> m_pointNormals;
    bvector<bool> m_hasPt;
    bvector<bool> m_donePt;

    double m_trimLongTrianglesLength;
    TrimHull (MeshData& meshData, double trimLongTrianglesLength = -1) : m_meshData (meshData), m_points (meshData.m_points), m_tetrahedrons (meshData.m_tetrahedrons), m_infPtNum (meshData.m_infPtNum), m_trimLongTrianglesLength (trimLongTrianglesLength), m_useSliverForTrim (trimLongTrianglesLength == -1)
        {
        m_ignorePtsAfterNum = m_meshData.m_ignorePtsAfterNum;
        FindTetrahedronForPoints ();
        Reset ();
        double maxL = DVec3d::FromStartEnd (m_meshData.m_extents.low, m_meshData.m_extents.high).Magnitude ();
        static double sf = 2;
        double dv = pow ((double)m_points.size (), (double)1 / 3) / sf;
        maxL /= dv;
        m_maxL = maxL;
        m_maxL2 = pow (maxL, 2);
        }

    bool IsInfPoint (int ptNum) const
        {
        return m_meshData.IsInfPoint (ptNum);
        }

    bool IgnorePoint (int ptNum) const
        {
        return m_meshData.IgnorePoint (ptNum) || m_donePt[ptNum];
        }

    typedef enum
        {
        UNSET =0,
        TOVISIT =1,
        VISITED = 2
        } EdgeState;
    void Method3 ();
    void Method4();
    void Method5 ();
    class compareUmbrellaFaces
        {
        public:
            bool operator() (const UmbrellaFaceInfo* lhs, const UmbrellaFaceInfo* rhs)
                {
                if (lhs->fmatch != rhs->fmatch) return lhs->fmatch < rhs->fmatch;
                if (lhs->centerPoint >= 0 && rhs->centerPoint >= 0 && lhs->vmatch[lhs->centerPoint] != rhs->vmatch[rhs->centerPoint]) return lhs->vmatch[lhs->centerPoint] < rhs->vmatch[rhs->centerPoint];
                if (lhs->lambda_upper == 1 && rhs->lambda_upper < 1) return false;
                if (lhs->lambda_upper < 1 && rhs->lambda_upper == 1) return true;
                return lhs->lambda_upper - lhs->lambda_lower < rhs->lambda_upper - rhs->lambda_lower;
                //return lhs->lambda_lower > rhs->lambda_lower;
                }
        };
    void UmbrellaFiltering();
    void umbrella_face_matching(map<int, vector<int>>& pointToFaces, vector<UmbrellaFaceInfo>& faces);
    void TestMethod ();

    void WriteTetrahedronsWithPointIndices(const char* filename);
        void WriteTetrahedrons(const wchar_t* filename);
    void WritePoints (const char* filename);
    void WriteMeshCallBack (int (*draw) (DTMFeatureType dtmFeatureType, int numTriangles, int numMeshPts, DPoint3d *meshPtsP, DPoint3d *meshVectorsP, int numMeshFaces, long *meshFacesP, void *userP), void* userP, MTGGraph* graphP);

    void MarkFace (int tn, int facet);
    void MarkFaceOp (int tn, int facet);
    bool IsFaceMarked (int tn, int facet) const;
    bool IsFacePrimaryMarked (int tn, int facet) const;

private:
    void Reset ();
    void ClearMarkedFace (int tn, int facet);
    //void MarkFacetChecked (std::vector<char>& haveChecked, int tn, int facet);

    void TrimFromEdge ();
    inline int getIndex (bvector<PointOnEdge>& pts, int  ea, int eb, int ec);
    inline int getIndexViaTriangulation (bvector<PointOnEdge>& pts, int ea, int eb, int ec, bool usePlane);
    inline int getIndexViaTest (bvector<PointOnEdge>& pts, int ea, int eb, int ec, bool usePlane);
    inline int getIndexViaTest2 (bvector<PointOnEdge>& pts, int ea, int eb, int ec, bool usePlane);
    inline int getIndexViaQuality (bvector<PointOnEdge>& pts, int ea, int eb, int ec);
    inline int getIndexViaCircumcenter (bvector<PointOnEdge>& pts, int ea, int eb, int ec);

    void FindTetrahedronForPoints ();
    void FindAndMarkFlatEdges ();
    void FindTrianglesViaMatchTrimming (std::vector<edge>& edges, bool justEdges);

    void FindPointToScan (bvector<int>& addPts, int scanMode);
    void FindEdgeFromPoint (std::vector<edge>& edges, bvector<int>& addPts);

    MarkEdge& GetMarkedEdge (int a, int b);
    int GetMarkedEdgeCount (int a, int b);
    int IncrementMarkedEdgeCount (int a, int b, int tetrahedron);
    int DecrementMarkedEdgeCount (int a, int b);
    void AddFixedFace (int tet, int face);
    void SetFixFaceAdj (int ffi, int nffi, int a, int b);
    void RemoveFixedFace (int tetNum, int face);
    int FindFixedFace (int tetNum, int face);
    DVec3d GetNormalForPoint (bvector<PointOnEdge>& pts, int ea, int eb, int ec);
    DVec3d GetNormalForPoint (int ptNum);
    int FixFacesAroundEdge (long i, std::vector<std::vector<std::pair<long, char>>>& edgeTets, std::vector<EdgeState>& edgeList, std::vector<int[6]>& edgeMap, std::vector<edge>* list = nullptr);

    // Method 5
    void Method5ProcessRun (TrimRun& run, std::vector<PointOnEdgeWithEdge>& newPoints);
    void Method5AddRun (std::vector<PointOnEdgeWithEdge>& runPoints);
    bool Method5NextPoint (int ea, int eb, int ec, int tetIndex, PointOnEdgeWithEdge& nextPoint);
    bool Method5FixScan (int startTet, int startP, int startEC, std::vector<PointOnEdgeWithEdge>& newPoints, int newPointStart, int endPoint, int endEC, int endTetNum, std::vector<PointOnEdgeWithEdge>& newBPoints, int pivotPt);
    void Method5Start (int fixTn, int fixFace);
    bool Method5FixMissingPoints (int ptNum);
    bool CountEdgeChains ();
    bool AddFixFaceWithEdges (int tn, int f);

    void ValidateWithTrimChecker ();
    void GetPtsForTest (bvector<PointOnEdge>& pts, int tetNum, int ea, int eb, int ec, bool usePlane);
    double GetMaxDist (DPoint3dCR pa, DPoint3dCR pb, DPoint3dCP pc);
    int getIndexViaBestCircumcenter (bvector<PointOnEdge>& pts, int ea, int eb, int ec);
    int getIndexViaAngleCluster (bvector<PointOnEdge>& pts, int ea, int eb, int ec);

    struct POEInfo
        {
        PointOnEdge* poe;
        DVec3d normal;
        double angleToNext;
        };
    int getIndexViaAngleClusterTest (bvector<POEInfo*>& pts, int ea, int eb, DVec3dCR plane, double maxDist);


    int getIndexViaCC2 (bvector<PointOnEdge>& pts, int ea, int eb, int ec);

    };



struct QuickPointFinder;
struct NewTrim
    {
    QuickPointFinder* m_qp;
    bvector<bool> m_hasPoint;
    DPoint3dP m_points;
    int m_numPoints;

    struct Edge
        {
        int ea, eb, ec;

        Edge (int a, int b, int c) : ea (a), eb (b), ec (c)
            {
            }

        Edge ()
            {
            }
        };
    bvector<bool> m_donePt;
    bvector<Edge> m_edges;
    bmap<uint64_t, MarkEdge> m_markEdges;
    bvector<int> faces;
    NewTrim (DPoint3dP points, int numPoints);
    ~NewTrim ();

    void Reset ()
        {
        m_hasPoint.resize (m_numPoints, false);
        m_donePt.resize (m_numPoints, false);
        }
    void GetPointsIn (DPoint3dCR p, double radius, bvector<int>& ret);
    int FindNearest (int p);
    void Trim ();
    void TrimPt (int pt);
    int FindPoint (int ea, int eb, int ec);
    void WriteMeshCallBack (int (*draw) (DTMFeatureType dtmFeatureType, int numTriangles, int numMeshPts, DPoint3d *meshPtsP, DPoint3d *meshVectorsP, int numMeshFaces, long *meshFacesP, void *userP), void* userP, MTGGraph* graphP);

    struct TrimRun
        {
        bvector<Edge> edges;
        };

    bvector<TrimRun> m_runs;
    bvector<TrimRun> m_newRuns;

    bool CountEdgeChains ();
    void ValidateWithTrimChecker ();
    void Method5ProcessRun (TrimRun& run, bvector<Edge>& newPoints);
    void Method5AddRun (bvector<Edge>& runPoints);
    //bool Method5FixScan (int startTet, int startP, int startEC, std::vector<Edge>& newPoints, int newPointStart, int endPoint, int endEC, int endTetNum, std::vector<PointOnEdgeWithEdge>& newBPoints, int pivotPt);
    void Method5Start (Edge& e);
    //bool Method5FixMissingPoints (int ptNum);
    void Method5 ();

    MarkEdge& GetMarkedEdge (int a, int b);
    int GetMarkedEdgeCount (int a, int b);
    int IncrementMarkedEdgeCount (int a, int b);
    int DecrementMarkedEdgeCount (int a, int b);

    bmap<uint64_t, bool> m_markedfaces;
    void MarkFace (int a, int b, int c);
    bool IsFaceMarked (int a, int b, int c) const;
    void AddFixedFace (int a, int b, int c);
    bool AddFixFaceWithEdges (int a, int b, int c);

    // Temp
    bvector<int> candidates;
    bvector<int> candidates2;

    };
