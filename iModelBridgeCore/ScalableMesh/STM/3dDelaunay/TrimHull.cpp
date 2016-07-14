#include <ScalableMeshPCH.h>
using namespace std;
#include "tetGen\tetgen.h"
#undef REAL
#include <iomanip>
#include <queue>

#pragma warning(disable : 4189 )
#undef static_assert
#include <ppl.h>
#include <amp.h>
#undef round
#include <amp_math.h>
#include <concurrent_vector.h>
#include "predicates.h"

#include "Mtg/MTGLoopDetector.h"


//#undef SCALABLE_MESH_ATP
#ifdef SCALABLE_MESH_ATP
#include <ScalableMesh/IScalableMeshATP.h>
#include <DgnPlatform\Tools\ConfigurationManager.h>
USING_NAMESPACE_BENTLEY_DGNPLATFORM
#endif
USING_NAMESPACE_BENTLEY_SCALABLEMESH
#ifdef DHDEBUG
//#define DEBUGMSG
//#pragma optimize( "", off )
//#define OUTPUTALL
//#define EGDEBUG
//#define LOG_TETRA

//#define DHTEST
//#define DHTEST2
#endif
#define TRIMCHECKER
#ifdef DHTEST2
const double testDist = 30;

bool testDistTo (DPoint3dCR a, DPoint3dCR b)
    {
    double dist = a.DistanceXY (b);
    if (dist > testDist)
        return true;
    return false;
    }
#endif
//#undef BeAssert
//#define BeAssert assert
#include "ScalableMeshCao.h"
#include "timer.h"
#include "../ScalableMesh/ScalableMeshGraph.h"
#include "TrimHull.h"
#include "../Blossom5/PerfectMatching.h"
//#define LOG_TETRAF

#ifdef LOG_TETRA
#define TETRA_LOG_FILE "D:\\0tetra.log"
#define TRACE_TETRA_INIT() \
    std::ofstream log; \
    log.open(TETRA_LOG_FILE, ios_base::app);
#define TRACE_TETRA_END() \
    log.close();
#else
#define TRACE_TETRA_INIT()
#define TRACE_TETRA_END()
#endif
//#define TEMP
//#ifdef TRIMCHECKER
__declspec(dllexport) bool doTRIMCHECKER = false;
//#endif
//#define FROMPLY "D:\\Scalable Mesh\\DataSet\\xyzrgb_dragon.ply\\xyzrgb_dragon2.ply"
bool TRIMCHECKER_OUTPUTDIFF = false;
bool USETRIMCHECKER = false;
//#define TRIMCHECKEREXITONBOUNDARY
//#define TESTVALUES
//#define WIPQUALITY
#define USEFIXEDFACES
static bool Circumcircle_do2d = false;

static int TrimmingMethod = 5;

#ifdef SCALABLE_MESH_ATP
#endif

#ifdef TESTVALUES
#define WIPQUALITY
#undef DEBUGMSG
#endif

bool TrimHull::m_checkMarkedEdges = true;
bool TrimHull::m_removeLongFaces = false;
bool TrimHull::m_getTrimLengthFromExtents = false;
bool TrimHull::m_trimFromEdge = true;
bool checkNormals = !true;
//uncomment to use matching (as in Method4) to get initial edges in Method3.
int useMatchTrimming = 0; // 0 = none, 1 = edge, 2 = faces
int getIndexMethod = 6; // 0 = elanie, 1 = bclib, 2 = old, 3 = fast, 4 = test2, 5 = bestCircumcenter, 6 = cluster


#ifdef WIPQUALITY
//Best Result 86.241434 60 200 5 0 0 0 0 5 0 0 0 0 0 10 1 0
//New Best 86.241434 % 60 200 5 0 0 0 0 5 5000 0 0 0 0 10 1 0
//Best Result88.248848 60 200 5 0 0 0 0 5 400 0 0 0 5 10 1 0
// 94.009217 8000 0 60 0 0 0 700 0 5000 0 0 0 0 200 5 1 0
static double qualityM_Q1 = 0;
static double qualityM_Q2 = 0;
static double qualityM_Q3 = 0;
static double qualityM_AA = 0;
static double qualityM_AB = 0;
static double qualityM_AC = 0;
static double qualityM_ang = 0;
static double qualityM_DistOverE = 0;
static double qualityM_DCOverDC = 0;
static double qualityM_Dist = 0;
static double qualityM_LargestEdge = 0;
static double qualityM_Area = 0;
static double qualityM_penalty = 0;
static double qualityM_AAB = 0;
static double qualityM_circumDist = 5;

static int qualityMethod = 1;       // 0 = Multiply then add or 1 = Add then Multiply
static int qualityAvg = 0;

static double qualityVCheck = 0.7;
#endif
#ifdef TRIMCHECKER
static double qualityStats = 0;
#endif

bool intersect (DPoint3dCR a, DPoint3dCR b, DPoint3dCR c, DPoint3dCR origin, DVec3dCR direction, DPoint3dR pt);

const int edgeToFaces[6][2] = { { 2, 3 }, { 0, 3 }, { 1, 3 }, { 1, 2 }, { 0, 2 }, { 0, 1 } };
const int edgeToPoints[6][2] = { { 0, 1 }, { 1, 2 }, { 0, 2 }, { 0, 3 }, { 1, 3 }, { 2, 3 } };
const int pointsToEdges[4][4] = { { -1, 0, 2, 3 },
{ 0, -1, 1, 4 },
{ 2, 1, -1, 5 },
{ 3, 4, 5, -1 } };

//using BENTLEY_NAMESPACE_NAME::TerrainModel;
struct pointLess
    {
    inline bool operator() (const DPoint3d& a, const DPoint3d& b) const
        {
        if (a.x < (b.x - 0.00001))
            return true;
        if (a.x > (b.x + 0.00001))

            return false;

        if (a.y < (b.y - 0.00001))
            return true;
        return false;
        }
    };

struct TrimChecker
    {
    int statsNumFailed;
    int statsNumExtra;
    int statsNum;


    void AddFace (int a, int b, int c)
        {
        if (!HasFace (a, b, c))
            statsNumExtra++;

        statsNum++;
        }

    double Report ()
        {
        int totalNumOfFaces = NumFaces () + statsNumExtra;
        int totalNumWrong = statsNumExtra + statsNumFailed;
        int totalNumRight = statsNum - totalNumWrong;

        double qualityStats = ((double)totalNumRight * 100) / totalNumOfFaces;
#ifdef DEBUGMSG
        cout << "Checker report " << statsNumFailed << ", " << statsNumExtra << ", " << statsNum << " - " << qualityStats << endl;
#endif
        return qualityStats;
        }

    struct face
        {
        int a, b, c;

        inline bool operator< (const face& s2) const
            {
            if (a < s2.a)
                return true;
            if (a > s2.a)
                return false;

            if (b < s2.b)
                return true;
            if (b > s2.b)
                return false;

            return c < s2.c;
            }

        face () : a (-1), b (-1), c (-1)
            {
            }

        face (int a, int b, int c)
            {
            if (a < b)
                {
                if (a > c)
                    std::swap (a, c);
                if (b > c)
                    std::swap (b, c);
                }
            else
                {
                if (a < c)
                    std::swap (a, b);
                else
                    {
                    std::swap (a, c);
                    if (a > b)
                        std::swap (a, b);
                    }
                }
            if (a > b)
                a = a;
            if (a > c)
                a = a;
            if (b > c)
                a = a;
            this->a = a;
            this->b = b;
            this->c = c;
            }
        };
    std::map <face, bool> m_faces;
    bmap <DPoint3d, int, pointLess> m_ptMapper;

    MeshData* m_meshData;
    public:
        void Init (DPoint3dCP points, int numPoints)
            {
            Reset ();
            m_faces.clear ();
            m_ptMapper.clear ();
            for (int i = 0; i < numPoints; i++)
                m_ptMapper[points[i]] = i;

            BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr dtm;
            dtm = BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTM::Create ();

            dtm->SetTriangulationParameters (0.001, 0.001, 0, 0);
            dtm->AddPoints (points, numPoints);
            dtm->Triangulate ();

            bcdtmInterruptLoad_triangleShadeMeshFromDtmObject (dtm->GetTinHandle (), 65000, 2, 1, &draw, false, DTMFenceType::None, DTMFenceOption::Overlap, nullptr, 0, this);
            }
        void Init (MeshData& meshData)
            {
            m_meshData = &meshData;
            Reset ();
            m_faces.clear ();
            m_ptMapper.clear ();

            for (int i = 0; i < (int)meshData.m_points.size (); i++)
                m_ptMapper[meshData.m_points[i]] = i;
#ifdef FROMPLY
            InitFromPly (FROMPLY);
            return;
#endif
            BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr dtm;
            dtm = BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTM::Create ();

            dtm->SetTriangulationParameters (0.001, 0.001, 0, 0);
            dtm->AddPoints (meshData.m_points.data (), meshData.m_ignorePtsAfterNum); // == -1 ? meshData.m_infPtNum : min (meshData.m_ignorePtsAfterNum, meshData.m_infPtNum));
            dtm->Triangulate ();

            bcdtmInterruptLoad_triangleShadeMeshFromDtmObject (dtm->GetTinHandle (), 65000, 2, 1, &draw, false, DTMFenceType::None, DTMFenceOption::Overlap, nullptr, 0, this);
            }

        void InitFromPly (char* filename)
            {
            TetGen::tetgenio test;

            test.load_ply (filename);

            bvector<int> localPtMapper;

            //FILE* fp = fopen ("D:\\Scalable Mesh\\DataSet\\xyzrgb_dragon.ply\\xyzrgb_dragon2.xyz", "wb");
            //fwrite (test.pointlist, sizeof (test.pointlist[0]), test.numberofpoints * 3, fp);
            //fclose (fp);

            localPtMapper.resize (test.numberofpoints);
            for (int i = 0; i < test.numberofpoints; i++)
                {
                int a = -1;
                DPoint3d pt = *(DPoint3d*)&test.pointlist[i * 3];
                auto aP = m_ptMapper.find (pt);

                if (aP != m_ptMapper.end ())
                    a = aP->second;
                else
                    {
                    a = FindPoint (pt);
                    }
                localPtMapper[i] = a;
                }

            for (int i = 0; i < test.numberoffacets; i++)
                {
                if (test.facetlist[i].numberofpolygons != 1)
                    continue;
                if (test.facetlist[i].polygonlist[0].numberofvertices != 3)
                    continue;

                int a = test.facetlist[i].polygonlist[0].vertexlist[0];
                int b = test.facetlist[i].polygonlist[0].vertexlist[1];
                int c = test.facetlist[i].polygonlist[0].vertexlist[2];

                a = localPtMapper[a];
                b = localPtMapper[b];
                c = localPtMapper[c];
                face f (a, b, c);
                m_faces[f] = false;
                }
            }

        void CheckAfter (/*std::vector<char>& haveChecked*/)
            {
            int numNotChecked = 0;
            int numChecked = 0;
            for (auto& f : m_faces)
                {
                if (!f.second)
                    {
                    const face& theF = f.first;

                    std::vector<int> linkedPoints;
                    int tetIndex = m_meshData->WalkToTetrahedronWithPoint (-1, m_meshData->m_points[theF.a]);
                    m_meshData->CollectLinkedPoints (tetIndex, linkedPoints, theF.a);
                    std::vector<int> tets;
                    m_meshData->GetTetrahedronsAroundPoint (tetIndex, theF.a, tets);
                    int fP[3] = { theF.a, theF.b, theF.c };
                    for (int tI : tets)
                        {
                        Tetrahedron& t = m_meshData->m_tetrahedrons[tI];
                        int fN = t.GetFaceSide (fP);
                        if (fN != -1)
                            {
                            fP[0] = fP[0];
                            break;
                            }
                        }
                    numNotChecked++;
                    }
                else
                    {
                    numChecked++;
                    }
                }
            std::cout << "Checker : numChecked " << numChecked << " numNotChecked " << numNotChecked << std::endl;
            }

        static int draw (DTMFeatureType dtmFeatureType, int numTriangles, int numMeshPts, DPoint3d *meshPtsP, DPoint3d *meshVectorsP, int numMeshFaces, long *meshFacesP, void *userP)
            {
            TrimChecker* checker = (TrimChecker*)userP;
            size_t indexCount = numMeshFaces;
            size_t pointCount = numMeshPts;
            DPoint3dCP pPoint = meshPtsP;
            int32_t const* pPointIndex = (int32_t*)meshFacesP;
            std::vector<int> localPtMapper;
            localPtMapper.resize (numMeshPts);

            for (int i = 0; i < numMeshPts; i++)
                {
                int a = -1;
                auto aP = checker->m_ptMapper.find (meshPtsP[i]);

                if (aP != checker->m_ptMapper.end ())
                    a = aP->second;
                else
                    {
                    a = checker->FindPoint (meshPtsP[i]);
                    }
                localPtMapper[i] = a;
                }

            for (int i = 0; i < numMeshFaces; i += 3)
                {
                int a = localPtMapper[meshFacesP[0] - 1];
                int b = localPtMapper[meshFacesP[1] - 1];
                int c = localPtMapper[meshFacesP[2] - 1];

                meshFacesP += 3;
                face f (a, b, c);
                checker->m_faces[f] = false;
                }
            return SUCCESS;
            }

        int FindPoint (DPoint3dCR testP)
            {
            for (int i = 0; i < m_meshData->m_ignorePtsAfterNum; i++)
                {
                DPoint3dCR p = m_meshData->m_points[i];
                if (p.AlmostEqual (testP))
                    return (int)i;
                }
            return -1;
            }
        bool HasFace (int a, int b, int c)
            {
            face f (a, b, c);

            auto it = m_faces.find (f);

            if (it != m_faces.end ())
                {
                it->second = true;
                return true;
                }
            return false;
            }

        int countMatchedFaces ()
            {
            int num = 0;
            for (auto f : m_faces)
                {
                if (f.second)
                    num++;
                f.second = false;
                }
            return num;
            }
        int NumFaces () const
            {
            return (int)m_faces.size ();
            }
        void Reset ()
            {
            statsNum = 0;
            statsNumExtra = 0;
            statsNumFailed = 0;
            }
    };
TrimChecker sTrimChecker;

void TrimHull::MarkFace (int tn, int facet)
    {
    assert (tn >= 0 && tn < (int)m_trimInfo.size ());
    m_trimInfo[tn].fixedFace |= 1 << (facet * 2);
    int adjTn = m_tetrahedrons[tn].GetAdjentTet (facet);
    if (adjTn == -1)
        return;
    int adjFacet = m_tetrahedrons[adjTn].GetFaceSide (tn);
    assert (adjTn >= 0 && adjTn < (int)m_trimInfo.size ());

    if (adjFacet == -1)
        return;
    m_trimInfo[adjTn].fixedFace |= 2 << (adjFacet * 2);
    }

void TrimHull::MarkFaceOp (int tn, int facet)
    {
    assert (tn >= 0 && tn < (int)m_trimInfo.size ());
    m_trimInfo[tn].fixedFace |= 2 << (facet * 2);
    int adjTn = m_tetrahedrons[tn].GetAdjentTet (facet);
    if (adjTn == -1)
        return;
    int adjFacet = m_tetrahedrons[adjTn].GetFaceSide (tn);
    assert (adjTn >= 0 && adjTn < (int)m_trimInfo.size ());

    if (adjFacet == -1)
        return;
    m_trimInfo[adjTn].fixedFace |= 1 << (adjFacet * 2);
    }

void TrimHull::ClearMarkedFace (int tn, int facet)
    {
    assert (tn >= 0 && tn < (int)m_trimInfo.size ());
    m_trimInfo[tn].fixedFace &= ~(3 << (facet * 2));
    int adjTn = m_tetrahedrons[tn].GetAdjentTet (facet);
    if (adjTn == -1)
        return;
    int adjFacet = m_tetrahedrons[adjTn].GetFaceSide (tn);
    assert (adjTn >= 0 && adjTn < (int)m_trimInfo.size ());
    if (adjFacet == -1)
        return;
    m_trimInfo[adjTn].fixedFace &= ~(3 << (adjFacet)* 2);
    }

inline bool TrimHull::IsFaceMarked (int tn, int facet) const
    {
    assert (tn >= 0 && tn < (int)m_trimInfo.size ());
    return (m_trimInfo[tn].fixedFace & (3 << (facet * 2))) != 0;
    }

inline bool TrimHull::IsFacePrimaryMarked (int tn, int facet) const
    {
    assert (tn >= 0 && tn < (int)m_trimInfo.size ());
    return (m_trimInfo[tn].fixedFace & (1 << (facet * 2))) != 0;
    }

#pragma region Method 3

struct GetEdgedist
    {
    DPoint3d _a;
    DPoint3d _b;
    const double maxL;
    double    abx, aby, abz;

    GetEdgedist (const DPoint3d &a, const DPoint3d &b, double maxL) : _a (a), _b (b), maxL (maxL)
        {
        abx = b.x - a.x;
        aby = b.y - a.y;
        abz = b.z - a.z;
        }

    inline DPoint3d d (const DPoint3d &c) const
        {
        //double d1 = distTo (_a, _b);
        //double d2 = distTo (_a, c);
        //double d3 = distTo (c, _b);

        //return TriangleArea (d1, d2, d3);

        double acx = c.x - _a.x;
        double acy = c.y - _a.y;
        double acz = c.z - _a.z;

        //        Vector w = P - L.P0;

        double c1 = acx * abx + acy * aby + acz * abz; //  dot (w, v);
        double c2 = abx * abx + aby * aby + aby * abz;
        double b = c1 / c2;

        if (b > 1) b = 1;
        else if (b < 0) b = 0;
        DPoint3d np;
        np.x = _a.x + (b * abx);
        np.y = _a.y + (b * aby);
        np.z = _a.z + (b * abz);
        return np;
        }
    inline double operator()(const DPoint3d &c) const
        {
        //double d1 = distTo (_a, _b);
        //double d2 = distTo (_a, c);
        //double d3 = distTo (c, _b);

        //return TriangleArea (d1, d2, d3);

        double acx = c.x - _a.x;
        double acy = c.y - _a.y;
        double acz = c.z - _a.z;

        //        Vector w = P - L.P0;

        double c1 = acx * abx + acy * aby + acz * abz; //  dot (w, v);
        double c2 = abx * abx + aby * aby + aby * abz;
        double b = c1 / c2;

        if (b > 1) b = 1;
        else if (b < 0) b = 0;
        DPoint3d np;
        np.x = _a.x + (b * abx);
        np.y = _a.y + (b * aby);
        np.z = _a.z + (b * abz);
        double dist2 = distToSquared (c, np);


        //double xy = abx * acy - aby * acx;
        //double yz = aby * acz - abz * acy;
        //double zx = abz * acx - abx * acz;

        //double dist = xy * xy + yz * yz + zx * zx;

        if (dist2 > maxL)
            return std::numeric_limits<double>::max ();
        return dist2;
        }
    };

static const double SMALL = 1.0e-15;
static const double GREAT = 1.0e+15;
static const double ROOTVSMALL = 1.0e-150;
static const double VSMALL = 1.0e-300;

bool    jmdlMTGSwap_quadraticXYZAspectRatio     /* <= false if degenerate */
(
double  *ratioP,                /* <= returned aspect ratio.  0 if degenerate */
const DPoint3d *point0P,
const DPoint3d *point1P,
const DPoint3d *point2P,
double max_edge_length = std::numeric_limits<double>::max()
)
    {
    DPoint3d U, V, W;
    DPoint3d UcrossV;
    double area;        /* SIGNED area of the triangle */
    double perim;       /* sum of SQUARED edge lengths */
    bool    boolstat;
    /* Compute vectors along each edge */
    bsiDPoint3d_subtractDPoint3dDPoint3d (&U, point1P, point0P);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&V, point2P, point1P);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&W, point0P, point2P);
    const double magU = U.DotProduct(U);
    const double magV = V.DotProduct(V);
    const double magW = W.DotProduct(W);
    perim = magU + magV + magW;
    double perim2 = (bsiDPoint3d_magnitude (&U) + bsiDPoint3d_magnitude (&V) + bsiDPoint3d_magnitude (&W));

    if (perim > 0 && magU < max_edge_length && magV < max_edge_length && magW < max_edge_length)
        {
        bsiDPoint3d_crossProduct (&UcrossV, &U, &V);
        area = UcrossV.DotProduct (UcrossV);
        double area2 = 0.5 * bsiDPoint3d_magnitude (&UcrossV);
        boolstat = true;
        *ratioP = area2 / perim;
        //        *ratioP = area / perim;
        //       *ratioP = sqrt (area) / perim2;
        *ratioP = /*1 -*/ (*ratioP * 4.0 * 1.7320508075688772935274463415059);
        }
    else
        {
        boolstat = false;
        *ratioP = 0.0;
        }

    return boolstat;
    }

#define VTK_DOUBLE_MAX 1.0e+299

static double Determinant2x2 (double a, double b, double c, double d)
    {
    return (a * d - b * c);
    };

int SolveLinearSystem (double **A, double *x, int size)
    {
    // if we solving something simple, just solve it
    //
    if (size == 2)
        {
        double det, y[2];

        det = Determinant2x2 (A[0][0], A[0][1], A[1][0], A[1][1]);

        if (det == 0.0)
            {
            // Unable to solve linear system
            return 0;
            }

        y[0] = (A[1][1] * x[0] - A[0][1] * x[1]) / det;
        y[1] = (-A[1][0] * x[0] + A[0][0] * x[1]) / det;

        x[0] = y[0];
        x[1] = y[1];
        return 1;
        }
    else if (size == 1)
        {
        if (A[0][0] == 0.0)
            {
            // Unable to solve linear system
            return 0;
            }

        x[0] /= A[0][0];
        return 1;
        }
    //
    // System of equations is not trivial, use Crout's method
    //

    // Check on allocation of working vectors
    //
    /*
    int *index, scratch[10];
    index = (size < 10 ? scratch : new int[size]);

    //
    // Factor and solve matrix
    //
    if (vtkMath::LUFactorLinearSystem (A, index, size) == 0)
    {
    return 0;
    }
    vtkMath::LUSolveLinearSystem (A, index, x, size);

    if (size >= 10) delete[] index;
    */
    return 1;
    }


//----------------------------------------------------------------------------
// Compute the circumcenter (center[3]) and radius squared (method
// return value) of a triangle defined by the three points x1, x2, and
// x3. (Note that the coordinates are 2D. 3D points can be used but
// the z-component will be ignored.)

double Circumcircle2d (DPoint3dCR x1, DPoint3dCR x2, DPoint3dCR x3,
                       DPoint3dR center)
    {
    DVec2d n12, n13, x12, x13;
    double* A[2];
    double rhs[2], sum, diff;

    //  calculate normals and intersection points of bisecting planes.
    //
    n12.x = x2.x - x1.x;
    n13.x = x3.x - x1.x;
    x12.x = (x2.x + x1.x) / 2.0;
    x13.x = (x3.x + x1.x) / 2.0;

    n12.y = x2.y - x1.y;
    n13.y = x3.y - x1.y;
    x12.y = (x2.y + x1.y) / 2.0;
    x13.y = (x3.y + x1.y) / 2.0;

    //  Compute solutions to the intersection of two bisecting lines
    //  (2-eqns. in 2-unknowns).
    //
    //  form system matrices
    //
    A[0] = &n12.x;
    A[1] = &n13.x;

    rhs[0] = n12.DotProduct (x12);
    rhs[1] = n13.DotProduct (x13);

    // Solve system of equations
    //
    if (SolveLinearSystem (A, rhs, 2) == 0)
        {
        center.x = center.y = 0.0;
        return VTK_DOUBLE_MAX;
        }
    else
        {
        center.x = rhs[0]; center.y = rhs[1];
        }

    sum = 0;
    //determine average value of radius squared
    diff = x1.x - center.x;
    sum += diff*diff;
    diff = x2.x - center.x;
    sum += diff*diff;
    diff = x3.x - center.x;
    sum += diff*diff;

    diff = x1.y - center.y;
    sum += diff*diff;
    diff = x2.y - center.y;
    sum += diff*diff;
    diff = x3.y - center.y;
    sum += diff*diff;

    if ((sum /= 3.0) > VTK_DOUBLE_MAX)
        {
        return VTK_DOUBLE_MAX;
        }
    else
        {
        return sum;
        }
    }


double Circumcircle (DPoint3dCR _a, DPoint3dCR _b, DPoint3dCR _c,
                     DPoint3dR center, DVec3dCR plane)
    {
    DPoint3d a2d, b2d, c2d, cc2d;

    RotMatrix mat;
    mat.InitFrom1Vector (plane, 2, true);

    //            mat.Invert ();
    Transform t = Transform::From (mat, _a);
    Transform t2;
    t2.InverseOf (t);
    a2d = _a;
    b2d = _b;
    c2d = _c;
    if (Circumcircle_do2d)
        {
        a2d.z = 0;
        b2d.z = 0;
        c2d.z = 0;
        }
    else
        {
        t2.multiply (&a2d);
        t2.multiply (&b2d);
        t2.multiply (&c2d);
        }

    cc2d.z = 0;
    double circumRadius = Circumcircle2d (a2d, b2d, c2d, cc2d);
    if (!Circumcircle_do2d)
        t.multiply (&cc2d);
    center = cc2d;

    //double da = distTo (center, _a);
    //double db = distTo (center, _b);
    //double dc = distTo (center, _c);

    //if (fabs (da - db) > 0.0001 || fabs (da - dc) > 0.0001 || fabs (da - circumRadius) > 0.0001)
    //    da = da;
    return circumRadius;
    }

double Circumcircle (DPoint3dCR _a, DPoint3dCR _b, DPoint3dCR _c,
                     DPoint3dR center)
    {
    DVec3d plane;
    GetPlaneNormal (&plane, &_a, &_b, &_c);

    return Circumcircle (_a, _b, _c, center, plane);
    }

inline double TriangleArea (DPoint3dCR  p1, DPoint3dCR  p2, DPoint3dCR  p3)
    {
    double a, b, c;
    a = distTo (p1, p2);
    b = distTo (p2, p3);
    c = distTo (p3, p1);
    return (0.25* sqrt (fabs (4.0*a*c - (a - b + c)*(a - b + c))));
    }

struct GtTriangleQuality
    {
    DPoint3d _a;
    DPoint3d _b;
    DPoint3d _circumCentre;
    DVec3d _plane1;
    bool _hascPt;
    DVec3d _U01;
#ifdef WIPQUALITY
    double _D01;
    GetEdgedist _l;
    DVec3d _baV;
#endif
    DPoint3d _c;
    const double _maxL;
    DVec3d _testPlane;
    GtTriangleQuality (const DPoint3d &a, const DPoint3d &b, const DPoint3d &c, bool hascPt, double maxL, DVec3dCR testPlane) : _a (a), _b (b), _maxL (maxL * maxL), _hascPt (hascPt), _c (c), _testPlane (testPlane)
#ifdef WIPQUALITY
        , _l (a, b, maxL)
#endif
        {
        //        _testPlane.Zero ();
        _U01 = DVec3d::FromStartEnd (_a, _b);
#ifndef WIPQUALITY
        if (_hascPt)
            {
            GetPlaneNormal (&_plane1, &_a, &_b, &c);
            if (_testPlane.IsZero ())
                Circumcircle (_a, _b, c, _circumCentre, _plane1);
            else
                Circumcircle (_a, _b, c, _circumCentre, _testPlane);
            }
        else
            {
            if (Circumcircle_do2d)
                _circumCentre = DPoint3d::From ((_a.x + _b.x) / 2, (_a.y + _b.y) / 2, 0);
            else
                _circumCentre = DPoint3d::From ((_a.x + _b.x) / 2, (_a.y + _b.y) / 2, (_a.z + _b.z) / 2);
            }
#else
        _D01 = _U01.Magnitude ();
        if (_hascPt)
            {
            GetPlaneNormal (&_plane1, &_a, &_b, &_c);
            DPoint3d d1 = _l.d (_c);
            //            double oldCDist = sqrt (l (_c));
            _baV = DVec3d::FromStartEnd (_b, _a);

            if (qualityM_circumDist != 0)
                {
                if (_testPlane.IsZero ())
                    Circumcircle (_a, _b, _c, _circumCentre, _plane1);
                else
                    Circumcircle (_a, _b, _c, _circumCentre, _testPlane);
                }
            }
        else
            {
            if (qualityM_circumDist != 0)
                {
                if (Circumcircle_do2d)
                    _circumCentre = DPoint3d::From ((_a.x + _b.x) / 2, (_a.y + _b.y) / 2, 0);
                else
                    _circumCentre = DPoint3d::From ((_a.x + _b.x) / 2, (_a.y + _b.y) / 2, (_a.z + _b.z) / 2);
                }
            }
        m_outputValues = false;
#endif
        }

#ifdef WIPQUALITY
    inline double quality (const DPoint3d& _c) const
        {
        DPoint3d tri[3] = { _a, _b, _c };
        static const double ang60 = 60.0 * PI / 180.0;

        DVec3d U12 = DVec3d::FromStartEnd (_b, _c);
        DVec3d U20 = DVec3d::FromStartEnd (_c, _a);
        double a;
        double Dot0 = -_U01.DotProduct (U20);
        if (Dot0 < 0)
            {
            DVec3d W0;
            W0.CrossProduct (_U01, U20);
            a = atan2 (sqrt (W0.DotProduct (W0)), Dot0);
            }
        else
            {
            double Dot1 = -U12.DotProduct (_U01);
            if (Dot1 < 0)
                {
                DVec3d W1;
                W1.CrossProduct (U12, _U01);
                a = atan2 (sqrt (W1.DotProduct (W1)), Dot1);
                }
            else
                {
                double Dot2 = -U20.DotProduct (U12);
                if (Dot2 < 0)
                    {
                    DVec3d W2;
                    W2.CrossProduct (U20, U12);
                    a = atan2 (sqrt (W2.DotProduct (W2)), Dot2);
                    }
                else
                    {
                    DVec3d W0, W1, W2;
                    W0.CrossProduct (_U01, U20);
                    W1.CrossProduct (U12, _U01);
                    W2.CrossProduct (U20, U12);
                    double t20 = W0.DotProduct (W0) / (Dot0 * Dot0);
                    double t21 = W1.DotProduct (W1) / (Dot1 * Dot1);
                    double t22 = W2.DotProduct (W2) / (Dot2 * Dot2);
                    if (t20 > t21 && t20 > t22)
                        a = atan2 (sqrt (W0.DotProduct (W0)), Dot0);
                    else if (t21 > t22)
                        a = atan2 (sqrt (W1.DotProduct (W1)), Dot1);
                    else //if (Dot2 < 0)
                        a = atan2 (sqrt (W2.DotProduct (W2)), Dot2);
                    }
                }
            }
        a = ((a / ang60) - 1) * 2;
        if (a < 0 || a > 1)
            a = a;
        return a;
        }

    inline double GetCircumRadius (const DPoint3d& c) const
        {
        const DVec3d caV = DVec3d::FromStartEnd (c, _a);
        double d1 = caV.DotProduct (_baV);
        double d2 = DVec3d::FromStartEnd (_b, c).DotProduct (_baV);
        double d3 = caV.DotProduct (DVec3d::FromStartEnd (c, _b));

        double denom = d2*d3 + d3*d1 + d1*d2;

        if (fabs (denom) < VSMALL)
            {
            // Degenerate triangle, returning GREAT for circumRadius.

            return GREAT;
            }
        else
            {
            double a = (d1 + d2)*(d2 + d3)*(d3 + d1) / denom;
            if (a < 0) return 0;
            if (GREAT < a)
                return sqrt (GREAT) * 0.5;
            return 0.5 * sqrt (a);
            }
        }

    inline double quality2 (const DPoint3d& _c) const
        {
        double c = GetCircumRadius (_c);

        if (c < ROOTVSMALL)
            {
            // zero circumRadius, something has gone wrong.
            return SMALL;
            }

        DVec3d normal;
        normal.CrossProduct (_baV, DVec3d::FromStartEnd (_c, _a));
        normal.x *= 0.5;
        normal.x *= 0.5;
        normal.y *= 0.5;
        double mag = normal.Magnitude ();
        static const double h = 3.0*sqrt (3.0) / 4.0;
        return mag / ((c*c)*h);
        }

    inline static double angleOf (const DPoint3d& a, const DPoint3d& b, const DPoint3d& c)  // 1 = 60.
        {
        static const double ang60 = 60.0 * PI / 180.0;
        DVec3d U12 = DVec3d::FromStartEnd (b, c);
        DVec3d U20 = DVec3d::FromStartEnd (c, a);
        double Dot2 = -U20.DotProduct (U12);

        DVec3d W2;
        W2.CrossProduct (U20, U12);
        double ang = atan2 (sqrt (W2.DotProduct (W2)), Dot2);
        return ang;
        if (ang > ang60)
            {
            ang = (ang60 * 2) - (ang - ang60);
            return ang / (ang60 * 2);
            }

        return ang / ang60;
        }

    inline double angleOfA (const DPoint3d& c)  // 1 = 60.
        {
        return angleOf (_b, c, _a);
        }
    inline double angleOfB (const DPoint3d& c)  // 1 = 60.
        {
        return angleOf (c, _a, _b);
        }
    inline double angleOfC (const DPoint3d& c)  // 1 = 60.
        {
        return angleOf (_a, _b, c);
        }


    double QualFromSideTotalDivLong (double a, double b, double c)
        {
        double total = a + b + c;
        double maxLen = max (a, max (b, c));
        total -= maxLen * 2;
        return total / maxLen;
        }
    bool m_outputValues;
    inline void AddQuality (double& quality, double multi, double value, double normalValue, char* valueString)
        {
        if (m_outputValues)
            {
            cout << "      " << valueString << value << endl;
            }
        if (multi == 0)
            return;
        if (qualityMethod == 0)
            quality += multi * (value / normalValue);
        else
            quality *= multi + (value + normalValue);
        }
#endif
#ifdef WIPQUALITY
    inline double WIPQuality (const DPoint3d &c)
        {
        // Calculate various metrics.
        DVec3d U12 = DVec3d::FromStartEnd (_b, c);
        DVec3d U20 = DVec3d::FromStartEnd (c, _a);
        double D12 = U12.Magnitude ();
        double D20 = U20.Magnitude ();
        double area = qualityM_Area == 0 ? 0 : TriangleArea (_a, _b, c);
        double Q1 = qualityM_Q1 == 0 ? 0 : 1 / QualFromSideTotalDivLong (_U01.Magnitude (), U12.Magnitude (), U20.Magnitude ());
        //        double Q1 = 1 / (area / (U01.DotProduct (U01) + U12.DotProduct (U12) + U20.DotProduct (U20)));
        double AC = qualityM_AC == 0 ? 1 : /*1 -*/ angleOfC (c);
        double AB = qualityM_AB == 0 && qualityM_AAB == 0 ? 1 :  /*1 -*/ angleOfB (c);
        double AA = qualityM_AA == 0 && qualityM_AAB == 0 ? 1 : /*1 -*/ angleOfA (c);
        double Q2 = qualityM_Q2 == 0 ? 0 : quality (c);
        double Q3 = 1;
        if (qualityM_Q3 != 0)
            jmdlMTGSwap_quadraticXYZAspectRatio (&Q3, &_a, &_b, &c);
        double DEC = sqrt (_l (c));
        double DCOverDC = 1;
        double circumDist = 1;
        if (D12 < D20)
            DCOverDC = D12 / D20;
        else
            DCOverDC = D20 / D12;

        DCOverDC = (D20 + D12) / DEC;
        DCOverDC *= DCOverDC;
        //        double a = U01.DotProduct (U01);
        //        double b = D01;
        //        DVec3d W = DVec3d::FromCrossProduct (U01, U12);
        //        double Q = sqrt(3) - (sqrt (W.DotProduct (W)) / (U01.DotProduct (U01) + U12.DotProduct (U12) + U20.DotProduct (U20)));
        //        double Q1 = 1 / (area / (U01.DotProduct (U01) + U12.DotProduct (U12) + U20.DotProduct (U20)));
        double penalty = 1;
        double inc = 1;
        double cd = 1;
        double ang = 0;
        double dist = DEC;
        double distOverE = 1;
        double largestEdge = 1;
        //        Q2 = 1 / Q2;
        //        double Q3 = 1 / quality2 (c);

        //if (std::isnan (AC))
        //    return std::numeric_limits<double>::max ();
        //if (AC > qualityVCheck)
        //    return std::numeric_limits<double>::max ();

        if (AA > (PI / 2) || AB > (PI / 2))
            penalty += 1;
        double AAB = max (AA, AB);
        dist = D20;
        cd = D12;

        if (cd > dist)
            largestEdge = cd;
        else
            largestEdge = dist;
        dist += cd;

        if ((largestEdge*largestEdge) > _maxL)
            return std::numeric_limits<double>::max ();
        distOverE = dist / _D01;
        distOverE *= distOverE;

        if (_hascPt)
            {
            //double dist2 = TriangleArea (_a, _b, _c); // sqrt (l (c));
            //dist /= dist2;
            //cd = sqrt (l (_c));
            //dist = cd / dist;
            //if (dist > 1)
            //    dist = 1 / dist;
            //dist = cd;// 1 - dist;
            //            plane1.x = 0; plane1.y = 0; plane1.z = 1;
            DVec3d plane2;
            GetPlaneNormal (&plane2, &_b, &_a, &c);

            if (qualityM_circumDist != 0)
                {
                DPoint3d ccc;
                Circumcircle (_a, _b, c, ccc, plane2);
                circumDist = distTo (_circumCentre, ccc);
                circumDist /= _D01;

                //RotMatrix mat;
                //mat.InitFrom1Vector (plane1, 2, true);

                ////            mat.Invert ();
                //Transform t = Transform::From (mat, circumCentre);
                //Transform t2;
                //t2.InverseOf (t);
                //DPoint3d a2d = _a;
                //DPoint3d b2d = _b;
                //DPoint3d ccc2d = ccc;
                //t2.multiply (&a2d);
                //t2.multiply (&b2d);
                //t2.multiply (&ccc2d);
                //DPoint3d o;
                //o.z = 0;
                //if (0 == bcdtmMath_intersectCordLines (0, 0, ccc2d.x, ccc2d.z, a2d.x, a2d.y, b2d.x, b2d.y, &o.x, &o.y))
                //    {
                //    }
                //else
                //    {
                //    circumDist = distTo (DPoint3d::From(0,0,0), o);
                //    circumDist += distTo (o, ccc2d);
                //    }
                }
            ang = _plane1.PlanarAngleTo (plane2, _U01);

            ang = fabs (ang);
            //if (ang > PI)
            //    ang -= PI;
            //ang /= PI;
            static const double largeAngCheck = ((PI * 2) / 4);
            static const double verylargeAngCheck = ((PI * 2) / 4);
            if (fabs (ang) > verylargeAngCheck)
                return std::numeric_limits<double>::max ();
            if (fabs (ang) > largeAngCheck)
                penalty = 10;
            //double inc1 = incircle (_a, _b, _c, c);
            //DPoint3d t;
            //t.x = (_a.x + _b.x + _c.x) / 3;
            //t.y = (_a.y + _b.y + _c.y) / 3;
            //t.z = (_a.z + _b.z + _c.z) / 3;
            //double incT = incircle (_a, _b, _c, t);
            //if (incT < 0)
            //    inc1 = -inc1;
            //if (inc1 > 0)
            //    inc = 0.5;
            //            inc = inc1;
            //if (inc1 > 0)
            //    return std::numeric_limits<double>::max ();
            //            dist = area;

            }
        else
            {

            if (qualityM_circumDist != 0)
                {
                DVec3d plane2;
                GetPlaneNormal (&plane2, &_b, &_a, &c);
                DPoint3d ccc;
                if (_testPlane.IsZero ())
                    Circumcircle (_a, _b, c, ccc, plane2);
                else
                    Circumcircle (_a, _b, c, ccc, _testPlane);

                circumDist = distTo (_circumCentre, ccc);
                circumDist /= _D01;

                }
            }
        //        return Q1;// *dist;
        //        dist *= Q1;
        double quality = qualityMethod == 0 ? 0 : 1;
        AddQuality (quality, qualityM_Q1, Q1, 1, "Q1");
        AddQuality (quality, qualityM_Q2, Q2, 1, "Q2");
        AddQuality (quality, qualityM_Q3, Q3, 1, "Q3");
        AddQuality (quality, qualityM_AA, AA, 1, "AA");
        AddQuality (quality, qualityM_AB, AB, 1, "AB");
        AddQuality (quality, qualityM_AC, AC, 1, "AC");
        AddQuality (quality, qualityM_ang, ang, 1, "ang");
        AddQuality (quality, qualityM_DistOverE, distOverE, 1, "distOverE");
        AddQuality (quality, qualityM_DCOverDC, DCOverDC, 1, "DCOverDC");
        AddQuality (quality, qualityM_LargestEdge, largestEdge, 1, "LargestEdge");
        AddQuality (quality, qualityM_Dist, dist, 1, "dist");
        AddQuality (quality, qualityM_Area, area, 1, "area");
        AddQuality (quality, qualityM_penalty * 100, penalty, 1, "penalty");
        AddQuality (quality, qualityM_AAB, AAB, 1, "AAB");
        AddQuality (quality, qualityM_circumDist, circumDist, 1, "CircumDist");
        return quality;
        }
#endif
    double CalcQuality (const DPoint3d &c)
        {
        if (distToSquared (_a, c) > _maxL)
            return std::numeric_limits<double>::max ();

        if (distToSquared (_b, c) > _maxL)
            return std::numeric_limits<double>::max ();

        DVec3d plane2;
        GetPlaneNormal (&plane2, &_b, &_a, &c);
        double ang = 1;
        if (_hascPt)
            {
            ang = _plane1.PlanarAngleTo (plane2, _U01);

            ang = fabs (ang);
            static const double verylargeAngCheck = ((PI * 2) / 4) - 0.1;
            if (ang > verylargeAngCheck)
                return std::numeric_limits<double>::max ();
            static double angMultiplier = 1;
            static double angStatic = 1;
            static bool useTan = false;
            if (useTan)
                ang = angStatic + (ang * tan (0.3 + (angMultiplier / verylargeAngCheck)));
            else
                ang = angStatic + (ang * angMultiplier);
            }
        DPoint3d ccc;
        //if (_hascPt && false)
        //    {
        //    DPoint3d ap, bp, cp, dp;
        //    ap = _a;
        //    bp = _b;
        //    cp = _c;
        //    dp = c;
        //    ap.z = bp.z = cp.z = dp.z = 0;
        //    double v = incircle (ap, bp, cp, dp);
        //    if (v == 0)
        //        return std::numeric_limits<double>::max ();

        //    if (v < 0)
        //        return 1 / v;
        //    return v;

        //    DPoint3d ccc2;
        //    plane2.x += _plane1.x;
        //    plane2.y += _plane1.y;
        //    plane2.z += _plane1.z;
        //    plane2.normalize ();

        //    Circumcircle (_a, _b, c, ccc, plane2);
        //    Circumcircle (_a, _b, _c, ccc2, plane2);
        //    double circumDist = distTo (ccc2, ccc);
        //    return circumDist * ang;
        //    }
        if (_testPlane.IsZero ())
            Circumcircle (_a, _b, c, ccc, _plane1);
        else
            Circumcircle (_a, _b, c, ccc, _testPlane);
        double circumDist = distTo (_circumCentre, ccc);
        return circumDist * ang;
        }

    inline void OutputValues (const DPoint3d &c)
        {
#ifdef WIPQUALITY
        m_outputValues = true;
        CalcQuality (c);
        m_outputValues = false;
#endif
        }
    inline double operator()(const DPoint3d &c)
        {
#ifndef WIPQUALITY
        return CalcQuality (c);
#else
        double q = WIPQuality (c);

        if (!_hascPt || qualityAvg == 0)
            return q;
        if (q == std::numeric_limits<double>::max ())
            return q;
        DPoint3d __c = _c;
        _c = c;
        double q2 = WIPQuality (__c);
        if (q2 == std::numeric_limits<double>::max ())
            return q2;
        _c = __c;
        return q * q2;
#endif
        }
    };

inline int TrimHull::getIndexViaCircumcenter (bvector<PointOnEdge>& pts, int ea, int eb, int ec)
    {
    long edgePts[2] = { ea, eb };
    int nLeftToVisit = 0;
    std::map<double, std::pair<DPoint3d, std::pair<long, char>>> map;
    char nFaces = 0;
    std::pair<DPoint3d, std::pair<long, char>> faces[2];
    std::vector<long> fixedFacetsThirdPoint;

    //DVec3d plane;
    //plane = GetNormalForPoint (pts, ea, eb, ec);

    for (auto pt : pts)
        {
        if (IgnorePoint (pt.ptNum) || ec == pt.ptNum)
            continue;
        double circumRadius;
        DPoint3d circumCenter;

        //        double dist = distTo (m_points[edgePts[0]], m_points[edgePts[1]]);
        DPoint3d pts[3] = { m_points[ea], m_points[eb], m_points[pt.ptNum] };
        circumRadius = Circumcircle (m_points[ea], m_points[eb], m_points[pt.ptNum], circumCenter/*, plane*/);
        DPoint3d triangleCentroid;
        bsiPolygon_centroidAreaPerimeter (pts, 3, &triangleCentroid, NULL, NULL, NULL, NULL);
        //if (IsFaceMarked (tetRef.first, edgeToFaces[tetRef.second][j]))
        //    {
        //    long p;
        //    for (int k = 0; k < 3; k++) if (ptsI[k] != edgePts[1] && ptsI[k] != edgePts[0]) p = ptsI[k];
        //    if (std::find (fixedFacetsThirdPoint.begin (), fixedFacetsThirdPoint.end (), p) != fixedFacetsThirdPoint.end ()) continue;
        //    fixedFacetsThirdPoint.push_back (p);
        //    map.insert (std::make_pair (0, std::make_pair (triangleCentroid, std::make_pair (tetRef.first, edgeToFaces[tetRef.second][j]))));
        //    if (nFaces < 2)
        //        {
        //        faces[nFaces] = std::make_pair (triangleCentroid, std::make_pair (tetRef.first, edgeToFaces[tetRef.second][j]));
        //        nFaces++;
        //        }
        //    }
        map.insert (std::make_pair (circumRadius, std::make_pair (triangleCentroid, std::make_pair (pt.tetraNum, pt.face))));
        }

    if (edgePts[0] == -1)
        return -2;
    for (auto it = map.begin (); it != map.end () && nFaces < 2; it++)
        {
        /*if (IsFaceMarked(it->second.second.first, it->second.second.second))
        {
        faces[nFaces++] = it->second;
        continue;
        }*/
        if (nFaces == 0)
            {
            faces[0] = it->second;
            nFaces = 1;
            continue;
            }
        DPoint3d center = it->second.first;
        DSegment3d edge = DSegment3d::From (m_points[edgePts[0]], m_points[edgePts[1]]);
        DSegment3d centers = DSegment3d::From (m_points[edgePts[0]], center);
        //DSegment3d centers = DSegment3d::From(center, faces[0].first);
        int ptsI[3];
        Tetrahedron& t = m_tetrahedrons[faces[0].second.first];
        t.GetFacePoints (faces[0].second.second, ptsI);
        DPoint3d pts[3] = { m_points[ptsI[0]], m_points[ptsI[1]], m_points[ptsI[2]] };
        DVec3d normal;
        GetPlaneNormal (&normal, pts, pts + 1, pts + 2);
        DVec3d axisY = DVec3d::FromStartEnd (m_points[edgePts[0]], m_points[edgePts[1]]);
        DPoint3d centroid = faces[0].first;
        DVec3d axisX = DVec3d::FromStartEnd (m_points[edgePts[0]], centroid);
        center.DifferenceOf (center, m_points[edgePts[0]]);
        centroid.DifferenceOf (centroid, m_points[edgePts[0]]);
        RotMatrix toCurrentFace;
        toCurrentFace.InitFrom2Vectors (axisX, axisY);
        toCurrentFace.Invert ();
        toCurrentFace.Multiply (center);
        toCurrentFace.Multiply (centroid);
        if ((center.x < 0 && centroid.x> 0) || (center.x > 0 && centroid.x< 0))
            {
            faces[1] = it->second;
            nFaces++;
            }
        /*    DPoint3d pointA, pointB;
        double fractionA, fractionB;
        if (DRay3d::ClosestApproachUnboundedRayUnboundedRay(
        fractionA, fractionB, pointA, pointB, DRay3d::From(edge), DRay3d::From(centers))
        && fractionA >= 0.0 && fractionA <= 1.0
        && fractionB >= 0.0 && fractionB <= 1.0)
        {
        // faces[1] = it->second;
        // nFaces++;
        }*/
        }
    if (nFaces == 0)
        return -2;
    for (int i = 0; i < (int)pts.size (); i++)
        {
        if (pts[i].tetraNum == faces[0].second.first)
            return (int)i;
        }
    return -2;
    //for (char f = 0; f < nFaces; f++)
    //    {
    //    long t = faces[f].second.first;
    //    char fi = faces[f].second.second;
    //    if (IsFaceMarked (t, (int)fi)) continue;
    //    MarkFace (t, (int)fi);
    //    FixedFaceInfo ffi;
    //    ffi.tetNum = t;
    //    ffi.face = (long)fi;
    //    m_fixedFaces.push_back (ffi);
    //    Tetrahedron tet = m_tetrahedrons[t];
    //    const int * faceP = Tetrahedron::GetDTetrahedron3dFacet (fi);
    //    if (retEdges != nullptr)
    //        {
    //        retEdges->push_back (edge (tet.ptNums[faceP[0]], tet.ptNums[faceP[1]], tet.ptNums[faceP[2]], t));
    //        retEdges->push_back (edge (tet.ptNums[faceP[1]], tet.ptNums[faceP[2]], tet.ptNums[faceP[0]], t));
    //        retEdges->push_back (edge (tet.ptNums[faceP[2]], tet.ptNums[faceP[0]], tet.ptNums[faceP[1]], t));
    //        }
    //    for (int j = 0; j < 3; j++)
    //        {
    //        for (int k = 0; k < 3; k++)
    //            if (k != j && edgeList[edgeMap[t][pointsToEdges[faceP[j]][faceP[k]]]] == EdgeState::UNSET)
    //                {
    //                edgeList[edgeMap[t][pointsToEdges[faceP[j]][faceP[k]]]] = EdgeState::TOVISIT;
    //                nLeftToVisit++;
    //                }
    //        }
    //    }
    //return nLeftToVisit;
    }

DVec3d TrimHull::GetNormalForPoint (int ptNum)
    {
    struct distToPoint
        {
        double dist;
        int pt;
        };
    bvector<distToPoint> distToPoints;
    DPoint3dCR pt = m_points[ptNum];
    if (false)
        {
        bvector<PointOnEdge> linkedPts;
        // Find closest Point
        m_meshData.CollectLinkedPoints (m_tetrahedronForPoints[ptNum], linkedPts, ptNum);
        for (auto& linkPt : linkedPts)
            {
            if (IgnorePoint (linkPt.ptNum))
                continue;
            double distTo = distToSquared (m_points[linkPt.ptNum], pt);
            if (distTo > m_maxL2)
                continue;
            distToPoint dtp;
            dtp.pt = linkPt.ptNum;
            dtp.dist = distTo;
            distToPoints.push_back (dtp);
            }
        }
    else
        {
        double maxL = m_maxL / 10;
        for (int i = 0; i < (int)m_points.size (); i++)
            {
            if (IgnorePoint (i) || i == ptNum)
                continue;

            double distTo = distToSquared (m_points[i], pt);
            if (distTo > (maxL * maxL))
                continue;

            distToPoint dtp;
            dtp.pt = i;
            dtp.dist = distTo;
            distToPoints.push_back (dtp);
            }
        }
    sort (distToPoints.begin (), distToPoints.end (), [](distToPoint& a, distToPoint& b)
        {
        return (a.dist < b.dist);
        }
    );
    bvector<DPoint3d> planePts;
    int num = (int)distToPoints.size ();
    const int maxNum = 20;
    if (num > maxNum)
        num = maxNum;
    //    planePts.push_back (m_points[ptNum]);
    for (int i = 0; i < num; i++)
        {
        planePts.push_back (m_points[distToPoints[i].pt]);
        }
    DPlane3d pl;
    double maxAbsDistance;
    pl.InitFromArray (planePts, maxAbsDistance);
    return pl.normal;
    }


inline DVec3d TrimHull::GetNormalForPoint (bvector<PointOnEdge>& pts, int ea, int eb, int ec)
    {
    //    return DVec3d::From (0, 0, 1);
    DPoint3dCR pa = m_points[ea];
    DPoint3dCR pb = m_points[eb];
    double maxL2 = m_maxL / 2;
    int tet;
    maxL2 *= maxL2;

    bvector<DPoint3d> planePts;
    planePts.push_back (pa);
    planePts.push_back (pb);
    double maxDist = 0;

    //DVec3d l = m_pointNormals[ea];// GetNormalForPoint (maxL, ea);
    //l.Add (m_pointNormals[eb]);
    //if (ec != -1)
    //    l.Add (m_pointNormals[ec]);

    //if (!l.IsZero ())
    //    return l;

    for (int i = 0; i < (int)pts.size (); i++)
        {
        if (pts[i].ptNum == ec)
            tet = pts[i].tetraNum;

        if (!IgnorePoint (pts[i].ptNum))
            {
            DPoint3d pt;
            pt = m_points[pts[i].ptNum];

            double d1 = distToSquared (pa, pt);
            if (maxL2 < d1)
                continue;

            double d2 = distToSquared (pb, pt);
            if (maxL2 < d2)
                continue;

            maxDist = max (maxDist, max (d1, d2));
            planePts.push_back (pt);
            }
        }
    DPlane3d pl;
    double maxAbsDistance;
    pl.InitFromArray (planePts, maxAbsDistance);
    maxAbsDistance *= 4;
    if (pow (maxAbsDistance, 2) > maxDist)
        {
        //    for (int j = 0; j < 2; j++)
        //        {
        //        bvector<PointOnEdge> pts;
        //        if (j == 0)
        //            m_meshData.FindPointsAroundEdge (tet, ea, ec, pts);
        //        else
        //            m_meshData.FindPointsAroundEdge (tet, eb, ec, pts);

        //        for (int i = 0; i < (int)pts.size (); i++)
        //            {
        //            if (!IgnorePoint (pts[i].ptNum))
        //                {
        //                DPoint3d pt;
        //                pt = m_points[pts[i].ptNum];

        //                if (IsFaceMarked (pts[i].tetraNum, pts[i].face))
        //                    {
        //                    //double d1 = distToSquared (pa, pt);
        //                    //if (maxL2 < d1)
        //                    //    continue;

        //                    //double d2 = distToSquared (pb, pt);
        //                    //if (maxL2 < d2)
        //                    //    continue;

        //                    //maxDist = max (maxDist, max (d1, d2));
        //                    planePts.push_back (pt);
        //                    }
        //                }
        //            }
        //        pl.InitFromArray (planePts, maxAbsDistance);
        //        }

        // Ideally expand the criteria until it is less than it.
        //pl.normal.x = 0;
        //pl.normal.y = 0;
        //pl.normal.z = 1;
        }
    return pl.normal;
    }


inline int TrimHull::getIndexViaTest (bvector<PointOnEdge>& pts, int ea, int eb, int ec, bool usePlane)
    {
    if (ec == -1)
        return getIndexViaTest2 (pts, ea, eb, ec, usePlane);

    DPoint3dCR pa = m_points[ea];
    DPoint3dCR pb = m_points[eb];
    DPoint3dCP pc = ec == -1 ? nullptr : &m_points[ec];
    Transform t2;

    if (usePlane)
        {
        DVec3d plane;
        plane = GetNormalForPoint (pts, ea, eb, ec);

        RotMatrix mat;

        mat.InitFrom1Vector (plane, 2, true);
        Transform t = Transform::From (mat, pa);
        t2.InverseOf (t);
        }

    DPoint3d p[3];
    double useA = 1;
    p[0] = pa;
    p[1] = pb;
    p[2] = *pc;
    if (usePlane)
        t2.multiply (p, 3);

    double orient = orient2d (p[0], p[1], p[2]);

    if (orient == 0)
        {
        if (!usePlane)
            return -2;
        // Use face normal.
        DVec3d plane;
        GetPlaneNormal (&plane, &pa, &pb, pc);

        RotMatrix mat;

        mat.InitFrom1Vector (plane, 2, true);
        Transform t = Transform::From (mat, pa);
        t2.InverseOf (t);
        p[0] = pa;
        p[1] = pb;
        p[2] = *pc;
        if (usePlane)
            t2.multiply (p, 3);
        orient = orient2d (p[0], p[1], p[2]);
        }
    if (orient > 0)
        useA = -1;

    int bestPt = -2;
    DPoint3d bp;
    int numPointsAdded = 0;
    double maxDist = GetMaxDist (pa, pb, pc);
    for (int i = 0; i < (int)pts.size (); i++)
        {
        if (pts[i].ptNum != ec && !IgnorePoint (pts[i].ptNum))
            {
            if (IsFaceMarked (pts[i].tetraNum, pts[i].face))
                return i;

            DPoint3d pt;
            pt = m_points[pts[i].ptNum];

            if (maxDist < distToSquared (pa, pt))
                continue;

            if (maxDist < distToSquared (pb, pt))
                continue;

            if (usePlane)
                t2.multiply (&pt);

            double side = 1;
            if (nullptr != pc)
                side = orient2d (p[0], p[1], pt) * useA;

            if (side > 0)
                {
                bool usePoint = false;
                if (bestPt == -2)
                    usePoint = true;
                else
                    {
                    double s1 = orient2d (p[1], bp, pt) * useA;
                    double s2 = s1 > 0 ? orient2d (bp, p[0], pt) * useA : 0;

                    if (s1 > 0 && s2 > 0)
                        {
                        // point is inside current triangle
                        usePoint = true;
                        }
                    else
                        {
                        double v = incircle (p[0], p[1], pt, bp) * useA;
                        if (v < 0)
                            usePoint = true;
                        }
                    }

                if (usePoint)
                    {
                    bp = pt;
                    bestPt = i;
                    }
                }
            }
        }
    return bestPt;
    }

inline int TrimHull::getIndexViaTest2 (bvector<PointOnEdge>& pts, int ea, int eb, int ec, bool _usePlane)
    {
    DPoint3dCR pa = m_points[ea];
    DPoint3dCR pb = m_points[eb];
    DPoint3dCP pc = ec == -1 ? nullptr : &m_points[ec];
//
    //DPoint3d tp[3];
    //double useA = 1;
    //tp[0] = pa;
    //tp[1] = pb;
    DVec3d _U01 = DVec3d::FromStartEnd (pa,pb);

    DPoint3d p[2];
    p[0].x = 0; p[0].y = 0; p[0].z = 0;
    p[1].x = 0; p[1].y = distTo (pa, pb); p[1].z = 0;

    DRay3d ray = DRay3d::FromOriginAndTarget (pa, pb);
    int bestPt = -2;
    DPoint3d bp;
    int numPointsAdded = 0;
    double maxDist = GetMaxDist (pa, pb, pc);
    for (int i = 0; i < (int)pts.size (); i++)
        {
        if (pts[i].ptNum != ec && !IgnorePoint (pts[i].ptNum))
            {
            if (IsFaceMarked (pts[i].tetraNum, pts[i].face))
                return i;

            DPoint3d pt;
            pt = m_points[pts[i].ptNum];

            if (maxDist < distToSquared (pa, pt))
                continue;

            if (maxDist < distToSquared (pb, pt))
                continue;

            DPoint3d closestPt;
            double closestParam;
            if (ray.ProjectPointUnbounded (closestPt, closestParam, pt))
                {
                double tX = distTo (pt, closestPt);
                double tY = distTo (pa, closestPt);
                if (tX > 0)
                    {
                    DPoint3d testPt = DPoint3d::From (tX, closestParam * p[1].y);
                    double side = 1;

                    bool usePoint = false;
                    if (bestPt == -2)
                        usePoint = true;
                    else
                        {
                        double s0 = orient2d (p[0], p[1], bp);
                        double s1 = orient2d (p[1], bp, testPt);
                        double s2 = s1 < 0 ? orient2d (bp, p[0], testPt) : 0;

                        if (s1 < 0 && s2 < 0)
                            {
                            // point is inside current triangle
                            usePoint = true;
                            }
                        else
                            {
                            double v = incircle (p[1], p[0], bp, testPt);
                            if (v > 0)
                                usePoint = true;
                            }
                        }

                    if (usePoint)
                        {
                        bp = testPt;
                        bestPt = i;
                        }
                    }
                }
            }
        }
    return bestPt;
    }

    inline int TrimHull::getIndexViaAngleClusterTest (bvector<POEInfo*>& pts, int ea, int eb, DVec3dCR plane, double maxDist)
        {
        DPoint3dCR pa = m_points[ea];
        DPoint3dCR pb = m_points[eb];
        RotMatrix mat = RotMatrix::From1Vector (plane, 2, true);
        Transform t = Transform::From (mat, pa);
        Transform t2;
        t2.InverseOf (t);

        DPoint3d p[2];
        p[0] = pa;
        p[1] = pb;
        t2.multiply (p, 2);

        //if (fabs (p[1].z) > 0.0000001)
        //    p[1].z = p[1].z;
        double useA = 0;
        int bestPt = -2;
        DPoint3d bp;
        for (int i = 0; i < (int)pts.size (); i++)
            {
            DPoint3d pt = m_points[pts[i]->poe->ptNum];

            if (maxDist < distToSquared (pa, pt))
                continue;

            if (maxDist < distToSquared (pb, pt))
                continue;

            t2.multiply (&pt);

            bool usePoint = false;
            if (bestPt == -2)
                usePoint = true;
            else
                {
                if (useA == 0)
                    {
                    const double orient = orient2d (p[0], p[1], bp); // + = counterclockwise

                    if (orient < 0)
                        useA = -1;
                    else
                        useA = 1;
                    }
#ifdef CHECKUSEA
                        {
                    DPoint3d tpt;
                    tpt.x = p[0].x + p[1].x + bp.x;
                    tpt.y = p[0].y + p[1].y + bp.y;
                    tpt.z = p[0].z + p[1].z + bp.z;

                    tpt.x /= 3; tpt.y /= 3; tpt.z /= 3;
                    double s1 = orient2d (p[1], bp, tpt) * useA;
                    double s2 = orient2d (bp, p[0], tpt) * useA;

                    if (s1 > 0 && s2 > 0)
                        {
                        }
                    else if (s1 != 0 && s2 != 0)
                        {
                        useA = -useA;
                        }
                    }
#endif

                double s1 = orient2d (p[1], bp, pt) * useA;
                double s2 = s1 > 0 ? orient2d (bp, p[0], pt) * useA : 0;

                if (s1 > 0 && s2 > 0)
                    {
                    // point is inside current triangle
                    usePoint = true;
                    }
                else
                    {
                    double v;
                    if (useA > 0)
                        v = incircle (p[0], p[1], bp, pt);
                    else
                        v = incircle (p[1], p[0], bp, pt);
                    if (v > 0)
                        usePoint = true;
                        }
                    }

                if (usePoint)
                    {
                    bp = pt;
                    bestPt = i;
                    }
                }
        return bestPt;
        }

inline int TrimHull::getIndexViaAngleCluster (bvector<PointOnEdge>& pts, int ea, int eb, int ec)
    {
    DPoint3dCR pa = m_points[ea];
    DPoint3dCR pb = m_points[eb];
    DPoint3dCP pc = ec == -1 ? nullptr : &m_points[ec];
    DVec3d Uab = DVec3d::FromStartEnd (pa, pb);
    static double clusterAngle = (25 * PI) / 180.0;
    static double maxAngle = (PI * 25) / 180;

    int prevPtNum = -1;
    double maxDist = GetMaxDist (pa, pb, pc);

    bvector<bvector<POEInfo*>> clusterPoints;
    bvector<POEInfo*> tempPoints;
    bvector<POEInfo*> testPoints;
    bvector<POEInfo> peoInfo;

    peoInfo.reserve (pts.size ());

    for (int i = 0; i < (int)pts.size (); i++)
        {
        POEInfo poeI;
        poeI.poe = &pts[i];

        GetPlaneNormal (&poeI.normal, &pa, &pb, &m_points[pts[i].ptNum]);
        peoInfo.push_back (poeI);
        }

    double biggestAngle = 0;
    double biggestI = -1;
    for (int i = 0; i < (int)peoInfo.size (); i++)
        {
        double angle;
        peoInfo[i].angleToNext = angle = fabs (peoInfo[i].normal.PlanarAngleTo (peoInfo[(i + 1) % (int)peoInfo.size ()].normal, Uab));

        if (biggestI == -1 || biggestAngle < angle)
            {
            biggestI = i;
            biggestAngle = angle;
            }
        }

    testPoints.reserve (peoInfo.size ());
    for (int i = (int)biggestI + 1; i < (int)peoInfo.size (); i++)
        testPoints.push_back (&peoInfo[i]);

    for (int i = 0; i <= biggestI; i++)
        testPoints.push_back (&peoInfo[i]);

    tempPoints.reserve (pts.size ());
    for (auto ptP : testPoints)
        {
        tempPoints.push_back (ptP);
        if (ptP->angleToNext > clusterAngle)
            {
            clusterPoints.push_back (tempPoints);
            tempPoints.clear ();
            }
        }

    if (!tempPoints.empty ())
        clusterPoints.push_back (tempPoints);

    DVec3d ecPlane;
    if (ec != -1)
        GetPlaneNormal (&ecPlane, &pb, &pa, pc);
    else
        ecPlane.Zero();

    tempPoints.clear ();
    for (auto& cPoints : clusterPoints)
        {
        if (cPoints.size () == 1)
            {
            if (ec != cPoints[0]->poe->ptNum)
                {
                if (distToSquared (pa, m_points[cPoints[0]->poe->ptNum]) < maxDist && distToSquared (pb, m_points[cPoints[0]->poe->ptNum]) < maxDist)
                    {
                    DVec3d plane;
                    GetPlaneNormal (&plane, &pa, &pb, &m_points[cPoints[0]->poe->ptNum]);
                    double angle = fabs (ecPlane.PlanarAngleTo (plane, Uab));
                    if (angle < maxAngle)
                        tempPoints.push_back (cPoints[0]);
                    }
                }
            }
        else
            {
            bool hasCPoint = false;
            for (auto& pt : cPoints)
                {
                if (pt->poe->ptNum == ec)
                    {
                    hasCPoint = true;
                    break;
                    }
                }
            if (hasCPoint)
                continue;
            //int ptIndex2 = getIndexViaTest (cPoints, ea, eb, ec, true);
            DVec3d plane;
            DVec3d plane2;
            GetPlaneNormal (&plane, &pa, &pb, &m_points[cPoints[0]->poe->ptNum]);
            plane.normalize ();
            GetPlaneNormal (&plane2, &pa, &pb, &m_points[cPoints[cPoints.size () - 1]->poe->ptNum]);
            plane2.normalize ();
            plane.x += plane2.x;
            plane.y += plane2.y;
            plane.z += plane2.z;
//            plane= DVec3d::From (0, 0, 1);
            //plane = GetNormalForPoint (pts, ea, eb, ec);
            int ptIndex = getIndexViaAngleClusterTest (cPoints, ea, eb, plane, maxDist);

            //if (ptIndex != ptIndex2)
            //    ptIndex = ptIndex2;
            //int ptIndex = getIndexViaTriangulation(cPoints, ea, eb, ec, true);
            if (ptIndex == -2)
                {
                ptIndex = -2;
                }
            else
                {
                if (ec != -1)
                    {
                    GetPlaneNormal (&plane, &pa, &pb, &m_points[cPoints[ptIndex]->poe->ptNum]);
                    double angle = fabs (ecPlane.PlanarAngleTo (plane, Uab));
                    if (angle > maxAngle)
                        ptIndex = -2;
                    }
                if (ptIndex != -2)
                tempPoints.push_back (cPoints[ptIndex]);
                }
            }
        }

    double bestV = -1;
    int bestPtNum = -1;
    for (auto& pt : tempPoints)
        {
        if (pt->poe->ptNum == ec || IgnorePoint (pt->poe->ptNum))
            continue;

        DPoint3d c = m_points[pt->poe->ptNum];
        double v;
        v = Circumcircle (pa, pb, m_points[pt->poe->ptNum], c);
        //v = TriangleArea (pa, pb, m_points[pt.ptNum]);
        //v = max (distTo (pa, m_points[pt.ptNum]), distTo (pb, m_points[pt.ptNum]));

        if (bestPtNum == -1 || v < bestV)
            {
            bestPtNum = pt->poe->ptNum;
            bestV = v;
            }
        }

    if (tempPoints.size () > 1)
        {
        bvector<PointOnEdge> tp;
        for (auto& pt : tempPoints)
            tp.push_back (*pt->poe);

            {
            int ptIndex;
            //ptIndex = getIndexViaTest2 (tp, ea, eb, ec, true);
            ptIndex = getIndexViaCC2 (tp, ea, eb, ec);
            if(ptIndex!=-2)
                bestPtNum = tp[ptIndex].ptNum;
            }
        }

    if (ec != -1 && doTRIMCHECKER)
        {
        int ptN = bestPtNum;
        if (!sTrimChecker.HasFace (ea, eb, ptN))
            {
            if (sTrimChecker.HasFace (ea, eb, ec))
                {
                for (auto& pt : pts)
                    {
                    if (pt.ptNum != ec)
                        {
                        if (sTrimChecker.HasFace (ea, eb, pt.ptNum))
                            {
                            bool found = false;
                            for (auto& tpt : tempPoints)
                                {
                                if (pt.ptNum == tpt->poe->ptNum)
                                    found = true;
                                }
                            if (!found)
                                found = found;
                            if (!found)
                                bestPtNum = pt.ptNum;
                            }
                        }
                    }
                }
            }
        }

    for (int i = 0; i < (int)pts.size (); i++)
        {
        if (pts[i].ptNum == bestPtNum)
            return i;
        }
    return -2;
    }

inline int TrimHull::getIndexViaCC2 (bvector<PointOnEdge>& pts, int ea, int eb, int ec)
    {


    DPoint3dCR pa = m_points[ea];
    DPoint3dCR pb = m_points[eb];
    DPoint3dCP pc = ec == -1 ? nullptr : &m_points[ec];
    if (ec != -1)
        {
        DVec3d normal;
        DVec3d Uab = DVec3d::FromStartEnd (pa, pb);
        GetPlaneNormal (&normal, &pb, &pa, pc);
        RotMatrix rotMatrix = RotMatrix::From2Vectors (normal, Uab);

        DVec3d YVec;
        rotMatrix.getColumn(&YVec, 2);
        YVec.normalize ();
        YVec.scale (distTo (pa, pb) * 1);
        DPoint3d searchCenter = DPoint3d::From (YVec.x + ((pa.x + pb.x) / 2), YVec.y + ((pa.y + pb.y) / 2), YVec.z + ((pa.z + pb.z) / 2));
        double radiusSquared = distToSquared (searchCenter, pa);
        bvector<int> candidates;

        static bool searchAll = true;

        for (int i = 0; i < (int)pts.size (); i++)
            {
            if (pts[i].ptNum == ec)
                continue;

            double dist = distToSquared (m_points[pts[i].ptNum], searchCenter);

            if (dist < radiusSquared)
                candidates.push_back (i);
            }

        if (candidates.empty ())
            return -2;
        if (candidates.size () == 1)
            return candidates[0];


        double smallestCC = 0;
        int bestI = -2;

        for (int i = 0; i < (int)candidates.size (); i++)
            {
            DPoint3dCR pt = m_points[pts[candidates[i]].ptNum];
            DPoint3d cc;
            double radius = Circumcircle (pa, pb, pt, cc);
            bool foundInside = false;

            for (int j = 0; j < (int)candidates.size (); j++)
                {
                if (j == i)
                    continue;

                DPoint3dCR pt2 = m_points[pts[candidates[j]].ptNum];
                if (distToSquared (cc, pt2) < radius)
                    {
                    foundInside = true;
                    break;
                    }
                }

            if (!foundInside)
                {
                return candidates[i];
                }
            }
        return bestI;

        return -2;
        }


    double smallestCC = 0;
    int bestI = -2;

    for (int i = 0; i < (int)pts.size (); i++)
        {
        if (pts[i].ptNum == ec)
            continue;
        int count = 0;
        DPoint3dCR pt = m_points[pts[i].ptNum];
        DPoint3d cc;
        double radius = Circumcircle (pa, pb, pt, cc);

        for (int j = 0; j < (int)pts.size (); j++)
            {
            if (j == i)
                continue;
            if (pts[j].ptNum == ec)
                continue;

            DPoint3dCR pt2 = m_points[pts[j].ptNum];
            if (distToSquared (cc, pt2) < radius)
                count++;
            }

        if (count == 0 && (smallestCC > radius || bestI == -2))
            {
            smallestCC = radius;
            bestI = i;
            }
        }
    return bestI;
    }

inline int TrimHull::getIndexViaBestCircumcenter (bvector<PointOnEdge>& pts, int ea, int eb, int ec)
    {
    struct PtInfo
        {
        int ptIndex;
        int ptNum;
        double radius;
        double value;
        bool ignore;
        DPoint3d circumCenter;
        DPoint3d pt;
        };

    bvector<PtInfo> ptInfos;
    DPoint3dCR pa = m_points[ea];
    DPoint3dCR pb = m_points[eb];

    for (int i = 0; i < (int)pts.size (); i++)
        {
        if (IgnorePoint (pts[i].ptNum))
            continue;
        if (pts[i].ptNum != ec && IsFaceMarked (pts[i].tetraNum, pts[i].face))
            return i;

        if (distTo (pa, m_points[pts[i].ptNum]) > m_maxL || distTo (pb, m_points[pts[i].ptNum]) > m_maxL)
            continue;
        PtInfo pInfo;
        pInfo.ptNum = pts[i].ptNum;
        pInfo.ignore = false;
        pInfo.ptIndex = i;
        pInfo.pt = m_points[pts[i].ptNum];
        pInfo.radius = Circumcircle (pa, pb, pInfo.pt, pInfo.circumCenter);
        ptInfos.push_back (pInfo);
        }

    if (ptInfos.size () != 2)
        {
        ec = ec;
        }

    std::sort (ptInfos.begin (), ptInfos.end (), [](PtInfo& p1, PtInfo& p2)
        {
        return p1.radius < p2.radius;
//        return p1.value < p2.value;
        });

    for (int a = 0; a < (int)ptInfos.size (); a++)
        {
        auto& pInfo = ptInfos[a];

        DPoint3d pt2;
        pt2.x = pInfo.circumCenter.x;// +(pInfo.circumCenter.x - pInfo.pt.x);
        pt2.y = pInfo.circumCenter.y;// +(pInfo.circumCenter.y - pInfo.pt.y);
        pt2.z = pInfo.circumCenter.z + sqrt(pInfo.radius); // (pInfo.circumCenter.z - pInfo.pt.z);
        for (int b = 0; b < (int)ptInfos.size (); b++)
            {
            if (a == b) continue;
            //if (ptInfos[b].ignore)
            //    continue;
            double dist = distTo (pInfo.circumCenter, ptInfos[b].pt);
            double value = insphere (pa, pb, pInfo.pt, pt2, ptInfos[b].pt);

            pInfo.value = min (value, pInfo.value);
            //            if (dist > pInfo.radius)
            if (value < 0)
                {
                if (sTrimChecker.HasFace (ea, eb, pInfo.ptNum))
                    eb = eb;
                pInfo.ignore = true;
                break;
                }
            }
        }
    std::sort (ptInfos.begin (), ptInfos.end (), [](PtInfo& p1, PtInfo& p2)
        {
        //return p1.radius < p2.radius;
        return p1.value > p2.value;
        });

    int pIndex = -2;
    for (auto& p : ptInfos)
        {
        if (p.ignore || p.ptNum == ec)
            continue;
        if (pIndex != -2)
            return -2;
        pIndex = p.ptIndex;
        return pIndex;
        }
    return pIndex;
    }

inline int TrimHull::getIndexViaTriangulation (bvector<PointOnEdge>& pts, int ea, int eb, int ec, bool usePlane)
    {
    DPoint3dCR pa = m_points[ea];
    DPoint3dCR pb = m_points[eb];
    DPoint3dCP pc = ec == -1 ? nullptr : &m_points[ec];
    Transform t2;
    double maxL2 = m_maxL / 2;
    maxL2 *= maxL2;

    if (usePlane)
        {
        DVec3d plane;
        plane = GetNormalForPoint (pts, ea, eb, ec);

        RotMatrix mat;

        mat.InitFrom1Vector (plane, 2, true);
        Transform t = Transform::From (mat, pa);
        t2.InverseOf (t);
        }

    BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr dtm;
    dtm = BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTM::Create ((int)pts.size () + 4, 10);
    dtm->SetTriangulationParameters (0, 0, 0, 0);

    DPoint3d p[4];
    bool useA = true;
    p[0] = pa;
    p[1] = pb;
    if (nullptr != pc)
        {
        p[2] = *pc;
        if (orient2d (p[0], p[1], p[2]) > 0)
            useA = false;

        if (usePlane)
            t2.multiply (p, 3);
        p[3] = p[0];

        if (usePlane)
            {
            bool useB = true;
            double orient = orient2d (p[0], p[1], p[2]);

            if (orient == 0)
                {
                if (!usePlane)
                    return -2;
                // Use face normal.
                DVec3d plane;
                GetPlaneNormal (&plane, &pa, &pb, pc);

                RotMatrix mat;

                mat.InitFrom1Vector (plane, 2, true);
                Transform t = Transform::From (mat, pa);
                t2.InverseOf (t);
                p[0] = pa;
                p[1] = pb;
                p[2] = *pc;
                if (usePlane)
                    t2.multiply (p, 3);
                p[3] = p[0];
                orient = orient2d (p[0], p[1], p[2]);
                }
            if (orient > 0)
                useB = false;

            if (useB != useA)
                useA = useB;
            else
                useA = useB;
            }
        DTMFeatureId fId;
        dtm->AddLinearFeature (DTMFeatureType::Breakline, p, 4, 0, &fId);
        }
    else
        {
        if (usePlane)
            t2.multiply (p, 2);
        DTMFeatureId fId;
        dtm->AddLinearFeature (DTMFeatureType::Breakline, p, 2, 0, &fId);
        }

    int numPointsAdded = 0;
    for (int i = 0; i < (int)pts.size (); i++)
        {
        if (pts[i].ptNum != ec && !IgnorePoint (pts[i].ptNum))
            {
            if (IsFaceMarked (pts[i].tetraNum, pts[i].face))
                return i;

            DPoint3d pt;
            pt = m_points[pts[i].ptNum];

            if (m_maxL2 < distToSquared (pa, pt))
                continue;

            if (m_maxL2 < distToSquared (pb, pt))
                continue;

            if (usePlane)
                {
                t2.multiply (&pt);
                }

            double side = 1;
            if (nullptr != pc)
                {
                if (useA)
                    side = orient2d (p[0], p[1], pt);
                else
                    side = orient2d (p[1], p[0], pt);
                }
            if (side >= 0)
                {
                dtm->AddPoints (&pt, 1);
                numPointsAdded++;
                }
            }
        }

    if (numPointsAdded == 0)
        return -2;

    dtm->Triangulate ();
    if (false)
        dtm->Save (L"d:\\temp\\l.bcdtm");

    BC_DTM_OBJ* dtmObj = dtm->GetTinHandle ();
    long p1;
    long p2;
    long pec;
    bcdtmFind_closestPointDtmObject (dtmObj, p[0].x, p[0].y, &p1);
    bcdtmFind_closestPointDtmObject (dtmObj, p[1].x, p[1].y, &p2);
    bcdtmFind_closestPointDtmObject (dtmObj, p[2].x, p[2].y, &pec);

    if (p1 == dtmObj->nullPnt || p2 == dtmObj->nullPnt || pec == dtmObj->nullPnt)
        return -2;

    if (bcdtmList_testForHullLineDtmObject (dtmObj, p1, p2) || bcdtmList_testForHullLineDtmObject (dtmObj, p2, p1))
        return -2;

    if (!bcdtmList_testLineDtmObject (dtmObj, p1, p2))
        return -2;

    long p3;

    if (useA)
        p3 = bcdtmList_nextAntDtmObject (dtmObj, p1, p2);
    else
        p3 = bcdtmList_nextClkDtmObject (dtmObj, p1, p2);

    if (p3 == -99)
        return -2;

    DPoint3d sPt;
    if (bcdtmObject_getPointByIndexDtmObject (dtmObj, p3, (DTM_TIN_POINT*)&sPt) != DTM_SUCCESS)
        return -2;
    sPt.z = 0;
    for (int i = 0; i < (int)pts.size (); i++)
        {
        if (!IgnorePoint (pts[i].ptNum) && ec != pts[i].ptNum)
            {
            DPoint3d pt;
            pt = m_points[pts[i].ptNum];
            if (usePlane)
                {
                t2.multiply (&pt);
                }

            if (sPt.AlmostEqualXY (pt))
                return i;
            }
        }
    return -2;
    }

inline int TrimHull::getIndexViaQuality (bvector<PointOnEdge>& pts, int ea, int eb, int ec)
    {
    int ptIndex = -2;
    double nearestDist = std::numeric_limits<double>::max ();

    GtTriangleQuality edgeDistHelper (m_points[ea], m_points[eb], ec == -1 ? DPoint3d () : m_points[ec], ec != -1, m_maxL, GetNormalForPoint (pts, ea, eb, ec));
    TRACE_TETRA_INIT ();

#ifdef LOG_TETRA
    std::string curEdgeLog;
    curEdgeLog += "[Method3] NOW COMPARING DIST AGAINST " + std::to_string (pts.size ()) + " pts for edge " + std::to_string (ea) + "/" + std::to_string (eb) + "\r\n";
#endif
    for (size_t i = 0; i < pts.size (); i++)
        {
        if (ec == pts[i].ptNum || IgnorePoint (pts[i].ptNum))
            continue;

        double distTo2 = edgeDistHelper (m_points[pts[i].ptNum]);

        if (distTo2 == std::numeric_limits<double>::max ())
            continue;
        if (false && nearestDist > 0 && ec != -1)
            {
            int count = GetMarkedEdgeCount (ea, pts[i].ptNum);
            if (count == 1)
                {
                int count2 = GetMarkedEdgeCount (eb, pts[i].ptNum);
                if (count2 == 1)
                    {
                    ptIndex = (int)i;
                    nearestDist = 0;
                    continue;
                    }
                }
            }
        // Check to see if this is a fixed face if it is use it.
        if (ec != -1 && IsFaceMarked (pts[i].tetraNum, pts[i].face))
            {
            ptIndex = (int)i;
            nearestDist = -1;
            continue;
            }
#ifdef LOG_TETRA
        curEdgeLog += "[Method3] DISTTO FOR  " + std::to_string (pts[i].ptNum) + " is " + std::to_string (distTo2) + " ACTUAL DIST "
            + std::to_string (distTo (m_points[ea], m_points[pts[i].ptNum])) + "/" + std::to_string (distTo (m_points[eb], m_points[pts[i].ptNum])) + "\r\n";
#endif

        if (ptIndex == -2 || distTo2 < nearestDist)
            {
            ptIndex = (int)i;
            nearestDist = distTo2;
            }
#ifdef EGDEBUG
        log << "CHECK POINT " + std::to_string (pts[i].ptNum) + " WITH DIST " + std::to_string (distTo) + " WITH INDEX " + std::to_string (ptIndex)
            + " WITH INDEX2 " + std::to_string (ptIndex2) + " WITH DIST 1 " + std::to_string (nearestDist) + " WITH DIST 2 " + std::to_string (nearestDist2) << endl;
#endif
        }

#ifdef DHTEST2
    if (ptIndex >= 0 && ec != -1)
        {
        //DPoint3d cen;
        //double radius = Circumcircle (m_points[ea], m_points[eb], m_points[ec], cen);
        //double radius2 = Circumcircle (m_points[ea], m_points[eb], m_points[pts[ptIndex], cen);

        DPoint3d a, b, c, d;
        a = m_points[ea];
        b = m_points[eb];
        c = m_points[ec];
        d = m_points[pts[ptIndex].ptNum];
        a.z = b.z = c.z = d.z = 0;
        if (incircle (a, b, c, d) >= 0)
            return -2;
        if (incircle (b, a, d, c) >= 0)
            return -2;
        }
    if (ptIndex >= 0)
        {
        if (testDistTo (m_points[ea], m_points[eb]))
            ea = ea;
        if (testDistTo (m_points[eb], m_points[pts[ptIndex].ptNum]))
            ea = ea;
        if (testDistTo (m_points[ea], m_points[pts[ptIndex].ptNum]))
            ea = ea;
        }
#endif

    return ptIndex;
    }

inline int TrimHull::getIndex (bvector<PointOnEdge>& pts, int ea, int eb, int ec)
    {
    if (getIndexMethod == 0)
        return getIndexViaCircumcenter (pts, ea, eb, ec);

    if (false && ec != -1)
        {
        int numPointsAdded = 0;
        DPoint3dCR pa = m_points[ea];
        DPoint3dCR pb = m_points[eb];
        DPoint3dCR pc = m_points[ec];
        int useIndex = -2;
        double dist = 0;

        for (int i = 0; i < (int)pts.size (); i++)
            {
            if (pts[i].ptNum != ec && !IgnorePoint (pts[i].ptNum))
                {

                DPoint3d pt;
                pt = m_points[pts[i].ptNum];

                if (m_maxL2 < distToSquared (pa, pt))
                    continue;

                if (m_maxL2 < distToSquared (pb, pt))
                    continue;

                //                dtm->AddPoints (&pt, 1);
                int ptIndex = getIndexViaTriangulation (pts, eb, ea, pts[i].ptNum, true);
                if (ptIndex != -2 && ec == pts[ptIndex].ptNum)
                    {
                    if (useIndex == -2)
                        {
                        useIndex = i;
                        dist = distTo (pa, pt) + distTo (pb, pt);
                        }
                    else
                        {
                        double dist2 = distTo (pa, pt) + distTo (pb, pt);
                        if (dist2 < dist)
                            {
                            useIndex = i;
                            dist = dist2;
                            }
                        }
                    }
                }
            }
        if (useIndex != -2)
            return useIndex;
        }

    if (getIndexMethod == 1)
        {
        static const bool usePlane = true;
        int ptIndex = getIndexViaTriangulation (pts, ea, eb, ec, usePlane);

#ifdef DHDEBUG
        //if (usePlane)
        //    {
        //    int ptIndex2 = getIndexViaTriangulation (pts, ea, eb, ec, false);
        //    if (ptIndex != ptIndex2)
        //        ptIndex = ptIndex2;
        //}
#endif

        return ptIndex;
        }

    if (getIndexMethod == 3)
        {
        static const bool usePlane = true;
        int ptIndex = getIndexViaTest (pts, ea, eb, ec, usePlane);

#ifdef DHDEBUG
        //if (usePlane)
        //    {
        //    int ptIndex2 = getIndexViaTriangulation (pts, ea, eb, ec, false);
        //    if (ptIndex != ptIndex2)
        //        ptIndex = ptIndex2;
        //}
#endif

        return ptIndex;
        }

    return getIndexViaQuality (pts, ea, eb, ec);
    }


void TrimHull::FindAndMarkFlatEdges ()
    {
    int duplicateEdges = 0;
    struct nonManifoldEdge
        {
        int a;
        int b;
        int tetraIndx;
        int face;
        int c;
        nonManifoldEdge (int a, int b, int tetraIndex, int face, int c) : a (a), b (b), tetraIndx (tetraIndex), face (face), c (c)
            {
            }
        };

    bvector<nonManifoldEdge> nonManifoldEdges;

    for (int i = 0; i < (int)m_tetrahedrons.size (); i++)
        {
        // If it is deleted ignore.
        if (m_tetrahedrons[i].ptNums[0] == -1)
            continue;

        for (int f = 0; f < numFacets; f++)
            {
            if (IgnorePoint ((int)m_tetrahedrons[i].ptNums[f]))
                {
                bool isNullFace = false;
                const int* facePt = Tetrahedron::GetDTetrahedron3dFacet (f);
                for (int f1 = 0; f1 < 3; f1++)
                    {
                    if (IgnorePoint (m_tetrahedrons[i].ptNums[facePt[f1]]))
                        {
                        isNullFace = true;
                        break;
                        }
                    }
                if (!isNullFace)
                    {
                    int adjTet = m_tetrahedrons[i].GetAdjentTet (f);

                    if (adjTet != -1 && adjTet > i)
                        {
                        int adjF = m_tetrahedrons[adjTet].GetFaceSide ((int)i);
                        if (IgnorePoint ((int)m_tetrahedrons[adjTet].ptNums[adjF]))
                            {
                            // Check that the ignore points are on the opposite side.
                            const int ea = m_tetrahedrons[i].ptNums[facePt[0]];
                            const int eb = m_tetrahedrons[i].ptNums[facePt[1]];
                            const int ec = m_tetrahedrons[i].ptNums[facePt[2]];

#ifdef DHTEST2
                            if (testDistTo (m_points[ea], m_points[eb]))
                                continue;
                            if (testDistTo (m_points[eb], m_points[ec]))
                                continue;
                            if (testDistTo (m_points[ea], m_points[ec]))
                                continue;
#endif

                            m_hasPt[ea] = true;
                            int count1 = IncrementMarkedEdgeCount (ea, eb, (int)i);
                            if (count1 >= 3)
                                nonManifoldEdges.push_back (nonManifoldEdge (ea, eb, i, f, ec));

                            m_hasPt[eb] = true;
                            int count2 = IncrementMarkedEdgeCount (eb, ec, (int)i);
                            if (count2 >= 3)
                                nonManifoldEdges.push_back (nonManifoldEdge (eb, ec, i, f, ea));

                            m_hasPt[ec] = true;
                            int count3 = IncrementMarkedEdgeCount (ec, ea, (int)i);
                            if (count3 >= 3)
                                nonManifoldEdges.push_back (nonManifoldEdge (ec, ea, i, f, eb));

                            MarkFace ((int)i, f);
                            }
                        }
                    }
                }
            }
        }

    for (auto& edge : nonManifoldEdges)
        {
        auto& mk1 = GetMarkedEdge (edge.a, edge.b);

        if (mk1.count < 3)
            continue;

        //        BeAssert (mk1.count == 3);

        if (mk1.count == 3)
            {
            auto& mk2 = GetMarkedEdge (edge.b, edge.c);
            auto& mk3 = GetMarkedEdge (edge.c, edge.a);

            if (mk2.count == 1 && mk3.count == 1)
                {
                // if both the other edges are 1 then this is the face to get rid of.
                mk1.count--;
                mk2.count--;
                mk3.count--;
                ClearMarkedFace (edge.tetraIndx, edge.face);
                continue;
                }
            else
                {
                bvector<PointOnEdge> pts;
                m_meshData.FindPointsAroundEdge (edge.tetraIndx, edge.a, edge.b, pts);
                bool hasRemovedFace = false;
                for (auto p : pts)
                    {
                    if (p.ptNum == edge.c)
                        continue;
                    if (!IsFaceMarked (p.tetraNum, p.face))
                        continue;

                    auto& mk2 = GetMarkedEdge (edge.b, p.ptNum);
                    if (mk2.count != 1)
                        continue;
                    auto& mk3 = GetMarkedEdge (p.ptNum, edge.a);
                    if (mk3.count != 1)
                        continue;

                    // if both the other edges are 1 then this is the face to get rid of.
                    mk1.count--;
                    mk2.count--;
                    mk3.count--;
                    ClearMarkedFace (edge.tetraIndx, edge.face);
                    hasRemovedFace = true;
                    break;
                    }

                //BeAssert (hasRemovedFace);
                if (hasRemovedFace)
                    continue;
                }
            bvector<PointOnEdge> pts;
            m_meshData.FindPointsAroundEdge (edge.tetraIndx, edge.a, edge.b, pts);
            // Didn't find a 1,1 face.
            for (auto p : pts)
                {
                if (!IsFaceMarked (p.tetraNum, p.face))
                    continue;
                mk1.count--;
                auto& mk2 = GetMarkedEdge (edge.b, p.ptNum);
                mk2.count--;
                auto& mk3 = GetMarkedEdge (p.ptNum, edge.a);
                mk3.count--;
                ClearMarkedFace (edge.tetraIndx, edge.face);
                }
            }
        }

#ifdef TRIMCHECKER
    if (doTRIMCHECKER)
        cout << "trim checker Num " << sTrimChecker.statsNum << " Wrong " << sTrimChecker.statsNumExtra << endl;
#endif
#ifdef DEBUGMSG
    cout << "non manifoldEdges " << duplicateEdges << endl;
#endif
    }

void TrimHull::FindTrianglesViaMatchTrimming (std::vector<edge>& edges, bool justEdges)
    {
    // now we have an edge.
#if 0
    std::vector<std::map<long, long>> pointEdges (m_points.size ());
    std::vector<int[6]> edgesT (m_tetrahedrons.size ());
    std::vector<std::vector<std::pair<long, char>>> edgeTets (m_tetrahedrons.size () * 6);
    std::vector<EdgeState> edgeList (m_tetrahedrons.size () * 6);
    int threshold = m_ignorePtsAfterNum % 2 == 0 ? (int)m_ignorePtsAfterNum - 1 : (int)m_ignorePtsAfterNum - 2;
    PerfectMatching match (threshold + 1, (int)m_tetrahedrons.size () * 3);
    match.options.fractional_jumpstart = true;
    match.options.update_duals_after = true;
    match.options.single_tree_threshold = 0.5;
    for (int i = 0; i < (int)m_tetrahedrons.size (); i++)
        {
        if (m_tetrahedrons[i].ptNums[0] == -1)
            continue;
        edgesT[i][0] = edgesT[i][1] = edgesT[i][2] = edgesT[i][3] = edgesT[i][4] = edgesT[i][5] = -1;
        Tetrahedron& t = m_tetrahedrons[i];

        for (int e = 0; e < 6; e++)
            {
            int pt1 = t.ptNums[edgeToPoints[e][0]], pt2 = t.ptNums[edgeToPoints[e][1]];
            if (pt1 <= threshold && pt2 <= threshold /*&& pt1 < pt2*/)
                {
                if (pointEdges[pt1].count (pt2) == 0)
                    {
                    double dist = distToSquared (m_points[pt1], m_points[pt2]);
                    edgesT[i][e] = pointEdges[pt1][pt2] = pointEdges[pt2][pt1] = match.AddEdge (pt1, pt2, dist);
                    }
                else
                    {
                    edgesT[i][e] = pointEdges[pt1][pt2];
                    }
                edgeTets[edgesT[i][e]].push_back (std::make_pair (i, e));
                }
            }
        }
    match.Solve ();
    int edgesLeft = 0;
    for (long i = 0; i < (long)m_tetrahedrons.size () * 6; i++)
        {
        if (edgeTets[i].size () == 0)
            {
            edgeList[i] = EdgeState::VISITED;
            continue;
            }
        if (match.GetSolution (i))
            {
            if (!justEdges)
                edgesLeft += FixFacesAroundEdge (i, edgeTets, edgeList, edgesT, &edges);
            else
                {
                auto tetRef = edgeTets[i].begin ();
                const Tetrahedron& t = m_tetrahedrons[tetRef->first];
                int pt1 = t.ptNums[edgeToPoints[tetRef->second][0]];
                int pt2 = t.ptNums[edgeToPoints[tetRef->second][1]];

                //                edges.push_back (edge (pt1, pt2, -1, tetRef->first));
                bvector<PointOnEdge> pts;
                m_meshData.FindPointsAroundEdge (tetRef->first, pt1, pt2, pts);
                int ptIndex = getIndex (pts, pt1, pt2, -1);
                if (ptIndex != -2)
                    {
                    MarkFace (pts[ptIndex].tetraNum, pts[ptIndex].face);
                    }
                }
            }
        }

    int count = 0;
    if (justEdges)
        {
        int tetNum = 0;
        for (const Tetrahedron& tet : m_tetrahedrons)
            {
            for (int f = 0; f < numFacets; f++)
                {
                if (IsFacePrimaryMarked (tetNum, f))
                    {
                    const int* ptI = Tetrahedron::GetDTetrahedron3dFacet (f);
                    for (int j = 0; j < 3; j++)
                        {
                        m_hasPt[tet.ptNums[ptI[j]]] = true;
                        IncrementMarkedEdgeCount (tet.ptNums[ptI[j]], tet.ptNums[ptI[(j + 1) % 3]], tetNum);
                        }
                    count++;
                    }
                }
            tetNum++;
            }
        edges.clear ();
        }
    else
        {
        for (auto edge : edges)
            {
            IncrementMarkedEdgeCount (edge.a, edge.b, edge.tetraIndx);
            count++;
            }
        }
    m_fixedFaces.clear ();
#endif
    }

bool TestPoint (DPoint3dCR ptA, DPoint3dCR ptB)
    {
    if (ptA.x < ptB.x)
        return true;
    if (ptA.y < ptB.y)
        return true;
    return ptA.z < ptB.z;
    }


#ifdef DHTEST
static int FindPoint (const SMPointList& points, double x, double y, double z)
    {
    DPoint3d p = DPoint3d::From (x, y, z);
    for (int i = 0; i < (int)points.size (); i++)
        {
        if (p.IsEqual (points[i], 0.001))
            return i;
        }
    return (int)points.size ();
    }
#endif

void TrimHull::SetFixFaceAdj (int ffiI, int nffi, int a, int b)
    {
    return;
    FixedFaceInfo& ffi = m_fixedFaces[ffiI];
    const int* ptI = Tetrahedron::GetDTetrahedron3dFacet (ffi.face);
    const Tetrahedron& tet = m_tetrahedrons[ffi.tetNum];

    for (int i = 0; i < 3; i++)
        {
        if (tet.ptNums[ptI[i]] == a)
            {
            if (tet.ptNums[ptI[(i + 1) % 3]] == b)
                {
                if (ffi.adjFace[i] != -1)
                    i = i;
                //BeAssert (ffi.adjFace[i] == -1);
                ffi.adjFace[i] = nffi;
                }
            else if (tet.ptNums[ptI[(i + 2) % 3]] == b)
                {
                if (ffi.adjFace[(i + 2) % 3] != -1)
                    i = i;
                //BeAssert (ffi.adjFace[(i + 2) % 3] == -1);
                ffi.adjFace[(i + 2) % 3] = nffi;
                }
            else
                {
                BeAssert (false);
                nffi = nffi;
                }
            }
        }
    }

int TrimHull::FindFixedFace (int tetNum, int face)
    {
    const int* ptI = Tetrahedron::GetDTetrahedron3dFacet (face);

    const Tetrahedron& tet = m_tetrahedrons[tetNum];
    for (int i = 0; i < 3; i++)
        {
        const int a = tet.ptNums[ptI[i]];
        const int b = tet.ptNums[ptI[(i + 1) % 3]];

        auto& edge = GetMarkedEdge (a, b);
        if (!edge.IsValid ())
            continue;

        if (edge.ffi == -1)
            continue;

        if (m_fixedFaces[edge.ffi].tetNum == tetNum && m_fixedFaces[edge.ffi].face == face)
            return edge.ffi;

        // OK it is the adjenct edge.
        const auto& tet2 = m_tetrahedrons[m_fixedFaces[edge.ffi].tetNum];
        const int* pt2I = Tetrahedron::GetDTetrahedron3dFacet (m_fixedFaces[edge.ffi].face);

        for (int j = 0; j < 3; j++)
            {
            const int a2 = tet2.ptNums[pt2I[j]];
            if (a == a2)
                {
                if (b == tet2.ptNums[pt2I[(j + 1) % 3]])
                    {
                    const int ffi2 = m_fixedFaces[edge.ffi].adjFace[j];
                    BeAssert (ffi2 != -1);
                    if (ffi2 != -1 && m_fixedFaces[ffi2].tetNum == tetNum && m_fixedFaces[ffi2].face == face)
                        return ffi2;
                    return -1;
                    }
                else if (b == tet2.ptNums[pt2I[(j + 2) % 3]])
                    {
                    const int ffi2 = m_fixedFaces[edge.ffi].adjFace[(j + 2) % 3];
                    BeAssert (ffi2 != -1);
                    if (ffi2 != -1 && m_fixedFaces[ffi2].tetNum == tetNum && m_fixedFaces[ffi2].face == face)
                        return ffi2;
                    return -1;
                    }

                        {
                            }
                }
            }
        }
    return -1;
    }

void TrimHull::RemoveFixedFace (int tetNum, int face)
    {
    //int ffiI = FindFixedFace (tetNum, face);
    FixedFaceInfo ffi;
    ffi.tetNum = tetNum;
    ffi.face = face;
    int ffiInd = std::find(m_fixedFaces.begin(), m_fixedFaces.end(), ffi) - m_fixedFaces.begin();
    //if (ffiIt != m_fixedFaces.end()) m_fixedFaces.erase(ffiIt);
    m_fixedFaces[ffiInd].tetNum = -1;
    }
inline double distToSquaredZ (DPoint3dCR a, DPoint3dCR b)
    {
    return pow ((a.z - b.z), 2);
    }
void TrimHull::AddFixedFace (int tetNum, int face)
    {
    FixedFaceInfo ffi;
    ffi.tetNum = tetNum;
    ffi.face = face;

    const int* ptI = Tetrahedron::GetDTetrahedron3dFacet (face);

    const Tetrahedron& tet = m_tetrahedrons[tetNum];
    const int a = tet.ptNums[ptI[0]];
    const int b = tet.ptNums[ptI[1]];
    const int c = tet.ptNums[ptI[2]];
    auto& ab = GetMarkedEdge (a, b);
    auto& bc = GetMarkedEdge (b, c);
    auto& ca = GetMarkedEdge (c, a);
    GetPlaneNormal (&ffi.normal, &m_points[a], &m_points[b], &m_points[c]);

    //double d1 = distToSquaredZ (m_points[a], m_points[b]);
    //double d2 = distToSquaredZ (m_points[c], m_points[b]);
    //double d3 = distToSquaredZ (m_points[a], m_points[c]);
    //double d = max (d1, max (d2, d3));

    //if (d > (20 * 20))
    //    d = d;
    //if (ffi.normal.z > 0)
    //    cout << "Top" << endl;
    //else
    //    cout << "Bottom" << endl;
    //m_pointNormals[a].Add (ffi.normal);
    //m_pointNormals[b].Add (ffi.normal);
    //m_pointNormals[c].Add (ffi.normal);
    if (ab.IsValid () || bc.IsValid () || ca.IsValid ())
        {
        if (ab.ffi == -1)
            {
            ab.ffi = (int)m_fixedFaces.size ();
            ffi.adjFace[0] = -1;
            }
        else
            {
            ffi.adjFace[0] = ab.ffi;
            SetFixFaceAdj (ab.ffi, (int)m_fixedFaces.size (), a, b);
            }

        if (bc.ffi == -1)
            {
            bc.ffi = (int)m_fixedFaces.size ();
            ffi.adjFace[1] = -1;
            }
        else
            {
            ffi.adjFace[1] = bc.ffi;
            SetFixFaceAdj (bc.ffi, (int)m_fixedFaces.size (), b, c);
            }

        if (ca.ffi == -1)
            {
            ca.ffi = (int)m_fixedFaces.size ();
            ffi.adjFace[2] = -1;
            }
        else
            {
            ffi.adjFace[2] = ca.ffi;
            SetFixFaceAdj (ca.ffi, (int)m_fixedFaces.size (), c, a);
            }
        }
    else
        {
        face = face;
        }

    m_fixedFaces.push_back (ffi);
    }

inline MarkEdge& TrimHull::GetMarkedEdge (int a, int b)
    {
    auto mk = m_markEdges.find (MarkEdge::MakeIndex (a, b));
    if (mk != m_markEdges.end ())
        return mk->second;
    static MarkEdge me (-1, -1);
    return me;
    }

inline int TrimHull::GetMarkedEdgeCount (int a, int b)
    {
    auto mk = m_markEdges.find (MarkEdge::MakeIndex (a, b));
    if (mk != m_markEdges.end ())
        return mk->second.count;
    return 0;
    }

int TrimHull::IncrementMarkedEdgeCount (int a, int b, int tetrahedron)
    {
    uint64_t index = MarkEdge::MakeIndex (a, b);
    auto mk = m_markEdges.find (index);
    if (mk != m_markEdges.end ())
        {
        return ++mk->second.count;
        }
    m_markEdges[index] = MarkEdge (1, tetrahedron);
    return 1;
    }

int TrimHull::DecrementMarkedEdgeCount (int a, int b)
    {
    uint64_t index = MarkEdge::MakeIndex (a, b);
    auto mk = m_markEdges.find (index);
    if (mk != m_markEdges.end ())
        {
        return --mk->second.count;
        }
    return -1;
    }

void TrimHull::FindTetrahedronForPoints ()
    {
    m_tetrahedronForPoints.resize (m_points.size ());
    for (size_t i = 0; i < m_tetrahedrons.size (); i++)
        {
        // If it is deleted ignore.
        if (m_tetrahedrons[i].ptNums[0] == -1)
            continue;

        // Add a tetrahedon for every point.
        for (int f = 0; f < numFacets; f++)
            m_tetrahedronForPoints[m_tetrahedrons[i].ptNums[f]] = (int)i;
        }
    }

inline double GetValue (DPoint3dCR pt, int mode, int index)
    {
    const int modeIndex[][3] =
        {
//                { 0, 1 , 2},
                { 2, 0, 1 },
        };
    return ((double*)&pt.x)[modeIndex[mode][index]];
    }

void TrimHull::FindPointToScan (bvector<int>& addPts, int scanMode)
    {
    // ToDo scanMode
    int bestPtNum = -1;
    DPoint3d bestPt;

    for (int i = 1; i < (int)m_points.size (); i++)
        {
        if (IgnorePoint (i))
            continue;
        if (m_hasPt[i])
            continue;

        const DPoint3d& pt = m_points[i];

        if (bestPtNum == -1)
            {
            bestPt = pt;
            bestPtNum = i;
            }
        else
            {
            double x = GetValue (pt, scanMode, 0);
            double y = GetValue (pt, scanMode, 1);
            double z = GetValue (pt, scanMode, 2);
            if (bestPt.x < x)
                continue;

            if (bestPt.x > x)
                {
                bestPt = pt;
                bestPtNum = i;
                continue;
                }

            if (bestPt.y < y)
                continue;

            if (bestPt.y > y)
                {
                bestPt = pt;
                bestPtNum = i;
                continue;
                }

            if (bestPt.z < z)
                continue;

            if (bestPt.z > z)
                {
                bestPt = pt;
                bestPtNum = i;
                continue;
                }
            }
        }
    if (bestPtNum != -1)
        addPts.push_back (bestPtNum);
    }

void TrimHull::FindEdgeFromPoint (std::vector<edge>& edges, bvector<int>& addPts)
    {
    bvector<PointOnEdge> linkedPts;
    // Find closest Point
    for (int j = 0; j < (int)addPts.size (); j++)
        {
        if (!m_hasPt[addPts[j]])
            {
            linkedPts.clear ();
            m_meshData.CollectLinkedPoints (m_tetrahedronForPoints[addPts[j]], linkedPts, addPts[j]);

            int ptNum2 = -2;
            double distToPt = 0;
            for (size_t i = 0; i < linkedPts.size (); i++)
                {
                if (IgnorePoint (linkedPts[i].ptNum))
                    continue;

                if (m_points[addPts[j]].DistanceSquaredXY (m_points[linkedPts[i].ptNum]) == 0)
                    continue;

                double dist = distToSquared (m_points[addPts[j]], m_points[linkedPts[i].ptNum]);
                if (ptNum2 == -2 || distToPt > dist)
                    {
                    ptNum2 = (int)i;
                    distToPt = dist;
                    }
#ifdef LOG_TETRA
                // log << "[Method3] eval'D LINKEDPT " + std::to_string(linkedPts[i].ptNum) + " dist is " + std::to_string(dist) + " with " + std::to_string(addPts[j]) << endl;
#endif
                }
#ifdef EGDEBUG
            // log << "AT POINT " + std::to_string(addPts[j]) + " ADDING EDGE TO " + std::to_string(linkedPts[ptNum2].ptNum) + " PART OF " + std::to_string(linkedPts[ptNum2].tetraNum) << endl;
#endif
            m_hasPt[addPts[j]] = true;
#ifdef LOG_TETRA
            //    log << "[Method3] PICKED LINKEDPT " + std::to_string(linkedPts[ptNum2].ptNum) + " dist is " + std::to_string(distToPt) + " with " + std::to_string(addPts[j]) << endl;
            //     log << "[Method3] PICKED TET " + std::to_string(linkedPts[ptNum2].tetraNum) + "[" + std::to_string(m_tetrahedrons[linkedPts[ptNum2].tetraNum].ptNums[0]) + "," + std::to_string(m_tetrahedrons[linkedPts[ptNum2].tetraNum].ptNums[1])
            //         + "," + std::to_string(m_tetrahedrons[linkedPts[ptNum2].tetraNum].ptNums[2]) + "," + std::to_string(m_tetrahedrons[linkedPts[ptNum2].tetraNum].ptNums[3]) +"]"<< endl;
            //     log << "[Method3] PICKED TET DISTANCES [" + std::to_string(distTo(m_points[m_tetrahedrons[linkedPts[ptNum2].tetraNum].ptNums[0]], m_points[m_tetrahedrons[linkedPts[ptNum2].tetraNum].ptNums[1]])) + "," + std::to_string(distTo(m_points[m_tetrahedrons[linkedPts[ptNum2].tetraNum].ptNums[1]], m_points[m_tetrahedrons[linkedPts[ptNum2].tetraNum].ptNums[2]]))
            //          + "," + std::to_string(distTo(m_points[m_tetrahedrons[linkedPts[ptNum2].tetraNum].ptNums[2]], m_points[m_tetrahedrons[linkedPts[ptNum2].tetraNum].ptNums[3]])) + "," + std::to_string(distTo(m_points[m_tetrahedrons[linkedPts[ptNum2].tetraNum].ptNums[3]], m_points[m_tetrahedrons[linkedPts[ptNum2].tetraNum].ptNums[0]])) + "]" << endl;
#endif

            if (ptNum2 == -2)
                {
#ifdef LOG_TETRA
                log << "[Method3] PT HAS NO LINKED POINTS, REMOVING PT " + std::to_string (addPts[j]) << endl;
#endif
                continue;
                }
            m_hasPt[linkedPts[ptNum2].ptNum] = true;
#ifdef LOG_TETRA
            if (linkedPts[ptNum2].tetraNum < 0 || linkedPts[ptNum2].tetraNum  >m_tetrahedrons.size () || (m_tetrahedrons[linkedPts[ptNum2].tetraNum].GetPointIndex (addPts[j]) == -1 && m_tetrahedrons[linkedPts[ptNum2].tetraNum].GetPointIndex (linkedPts[ptNum2].ptNum) == -1))
                {
                log << "[Method3] INVALID NEW EDGE/TETRA PAIRING AT T#" + std::to_string (linkedPts[ptNum2].tetraNum) + " AT POINTS " + std::to_string (addPts[j]) + "/" + std::to_string (linkedPts[ptNum2].ptNum)
                    + " WITH PT INDEXES " + std::to_string (m_tetrahedrons[linkedPts[ptNum2].tetraNum].GetPointIndex (addPts[j])) + "/" + std::to_string (m_tetrahedrons[linkedPts[ptNum2].tetraNum].GetPointIndex (linkedPts[ptNum2].ptNum)) << endl;
                continue;
                }
#endif
            edges.push_back (edge (addPts[j], linkedPts[ptNum2].ptNum, -1, linkedPts[ptNum2].tetraNum));
            }
        }
    }
void TrimHull::Method3 ()
    {
    int d = 1;
    int c = 0;
    int findPointMethod = 1;
    int startPt = -1;
    // now we have an edge.
    std::vector<edge> edges;

    DVec3d zeroVec;
    zeroVec.Zero ();
    m_pointNormals.resize (m_points.size (), zeroVec);

    //parallel_for (0, (int)m_points.size (), 1, [&] (int i)
    //    {
    //    m_pointNormals[i] = GetNormalForPoint (maxL, i);
    //    });

    switch (useMatchTrimming)
        {
        case 1:
            FindTrianglesViaMatchTrimming (edges, true);
            break;
        case 2:
            FindTrianglesViaMatchTrimming (edges, false);
            break;
        }

#ifdef DHTEST
    double tepts[] = {
        2768115.0632, 419904.3430, 341.8935,
        2768121.6231, 419904.3430, 342.1373,
        };
    const int tenumPts = sizeof (tepts) / (sizeof (double) * 3);
    bvector<uint64_t> testEdges;

    for (int i = 0; i < tenumPts - 1; i++)
        {
        double* pt1 = &tepts[i * 3];
        double* pt2 = &tepts[((i + 1) % tenumPts) * 3];
        testEdges.push_back (MarkEdge::MakeIndex (FindPoint (m_points, pt1[0], pt1[1], pt1[2]), FindPoint (m_points, pt2[0], pt2[1], pt2[2])));
        }
    //testEdges.push_back (MarkEdge::MakeIndex (394,346));
    //testEdges.push_back (MarkEdge::MakeIndex (346, 421));
    //testEdges.push_back (MarkEdge::MakeIndex (421, 394));

    //testEdges.push_back (MarkEdge::MakeIndex (421,347));
    //testEdges.push_back (MarkEdge::MakeIndex (347, 373));
    //testEdges.push_back (MarkEdge::MakeIndex (373, 421));
    //testEdges.push_back (MarkEdge::MakeIndex (447,373));

    //testEdges.clear ();
    //testEdges.push_back (MarkEdge::MakeIndex (399, 373));

    for (auto mi : testEdges)
        {
        auto mk = markEdge.find (mi);
        if (mk != markEdge.end ())
            cout << "mk.count" << (int)mk->second.count << endl;;
        }
#endif
    if (m_markEdges.size ())
        {

        // Scan through all edges.
        for (auto& e : m_markEdges)
            {
            if (e.second.count == 1)
                {
                const int tetraIndex = e.second.tet;

                for (int f = 0; f < numFacets; f++)
                    {
                    if (IsFaceMarked (tetraIndex, f))
                        {
                        const int* facePt = Tetrahedron::GetDTetrahedron3dFacet (f);
                        const int ea = m_tetrahedrons[tetraIndex].ptNums[facePt[0]];
                        const int eb = m_tetrahedrons[tetraIndex].ptNums[facePt[1]];
                        const int ec = m_tetrahedrons[tetraIndex].ptNums[facePt[2]];

                        if (MarkEdge::MakeIndex (ea, eb) == e.first)
                            edges.push_back (edge (ea, eb, ec, tetraIndex));
                        else if (MarkEdge::MakeIndex (eb, ec) == e.first)
                            edges.push_back (edge (eb, ec, ea, tetraIndex));
                        else if (MarkEdge::MakeIndex (ec, ea) == e.first)
                            edges.push_back (edge (ec, ea, eb, tetraIndex));
                        }
                    }
                }
            }
        for (int i = 0; i < (int)m_tetrahedrons.size (); i++)
            {
            for (int f = 0; f < numFacets; f++)
                {
                if (IsFacePrimaryMarked (i, f))
                    {
                    AddFixedFace (i, f);

#ifdef TRIMCHECKER
                    if (doTRIMCHECKER)
                        {
                        const int* ptIndex = Tetrahedron::GetDTetrahedron3dFacet (f);
                        const Tetrahedron& tet = m_tetrahedrons[i];
                        sTrimChecker.AddFace (tet.ptNums[0], tet.ptNums[1], tet.ptNums[2]);
                        }
#endif
                    }
                }
            }
#ifdef TRIMCHECKER
        if (doTRIMCHECKER)
            qualityStats = sTrimChecker.Report ();
#endif
        }
    else if (edges.size ())
        {
        for (const auto& edge : edges)
            {
            m_hasPt[edge.a] = true;
            m_hasPt[edge.b] = true;
            }
        }

    //        return;
    // find left/top/right/bottom pts.
    bvector<int> addPts;
    if (edges.empty ())
        FindPointToScan (addPts, 0);

#ifdef EGDEBUG
    std::ofstream log;
    // std::map<int,int> tetraqty;
    log.open ("d:\\dtrimmer2.log", ios_base::app);
    bvector<bool> hasTetrahedron;
    hasTetrahedron.resize (m_tetrahedrons.size (), false);
    bvector<bool> hasPtTetrahedron;
    hasPtTetrahedron.resize (m_points.size (), false);
#endif
    std::vector<edge> newEdges;
    bvector<PointOnEdge> pts;

    while (!edges.empty () || !addPts.empty ())
        {
        TRACE_TETRA_INIT ();
        // We have no edges, but points, so need to find the edges.
        if (!addPts.empty ())
            FindEdgeFromPoint (edges, addPts);

        TRACE_TETRA_END ();

        while (edges.size () || newEdges.size ())
            {
            if (edges.empty ())
                {
                edges.clear ();
                for (auto a = newEdges.rbegin (); a != newEdges.rend (); a++)
                    edges.push_back (*a);
                newEdges.clear ();
                }

            edge e = edges.back ();
            edges.pop_back ();
            if (m_checkMarkedEdges)
                {
                int edgeCount = GetMarkedEdgeCount (e.a, e.b);
                if (edgeCount >= 2)
                    continue;
                }

            // TRACE_TETRA_INIT()
            pts.clear ();
            //                GetTetrahedronsAroundPoint ();
            // find the point that is opposite the link.
#ifdef LOG_TETRA
            if (e.tetraIndx < 0 || e.tetraIndx  >m_tetrahedrons.size () || (m_tetrahedrons[e.tetraIndx].GetPointIndex (e.a) == -1 && m_tetrahedrons[e.tetraIndx].GetPointIndex (e.b) == -1))
                {
                log << "[Method3] INVALID EDGE/TETRA PAIRING AT T#" + std::to_string (e.tetraIndx) + " AT POINTS " + std::to_string (e.a) + "/" + std::to_string (e.b)
                    + " WITH PT INDEXES " + std::to_string (m_tetrahedrons[e.tetraIndx].GetPointIndex (e.a)) + "/" + std::to_string (m_tetrahedrons[e.tetraIndx].GetPointIndex (e.b)) << endl;
                continue;
                }
#endif
            //TRACE_TETRA_END();
#ifdef DHTEST
            for (auto mi : testEdges)
                {
                if (MarkEdge::MakeIndex (e.a, e.b) == mi)
                    break;
                }
#endif
            m_meshData.FindPointsAroundEdge (e.tetraIndx, e.a, e.b, pts);

            int ptIndex = getIndex (pts, e.a, e.b, e.c);

            //if (e.c != -1 && ptIndex != -2)
            //    {
            //    int chPtIndex = getIndex ( pts, e.a, e.b, pts[ptIndex].ptNum);

            //    if (chPtIndex != -2 && pts[chPtIndex].ptNum != e.c)
            //        chPtIndex = chPtIndex;
            //    }

#ifdef TRIMCHECKER
            int choosenPt = ptIndex;
            int checkPt = -2;
            if (doTRIMCHECKER)
                {
                for (size_t i = 0; i < pts.size (); i++)
                    {
                    if (e.c != pts[i].ptNum && sTrimChecker.HasFace (e.a, e.b, pts[i].ptNum))
                        {
                        checkPt = (int)i;
                        break;
                        }
                    }
                if (USETRIMCHECKER && ptIndex != checkPt)
                    ptIndex = checkPt;
                }
#endif

            // find the distance to the line, and use the shortest, ignore the
            if (ptIndex != -2)
                {

#ifdef LOG_TETRA
                double dist1 = distTo (m_points[e.a], m_points[pts[ptIndex].ptNum]);
                double dist2 = distTo (m_points[e.b], m_points[pts[ptIndex].ptNum]);
                double dist3 = distTo (m_points[e.b], m_points[e.a]);
                curEdgeLog += "[Method3] NEW FACE IS " + std::to_string (e.a) + "/" + std::to_string (e.b) + "/" + std::to_string (pts[ptIndex].ptNum) + " dists " +
                    std::to_string (dist1) + "/" + std::to_string (dist2) + "/" + std::to_string (dist3) + "\r\n";
                if (dist1 >= 2 || dist2 >= 2 || dist3 >= 2)
                    {
                    log << curEdgeLog;
                    }
#endif
                bool addFace = !IsFaceMarked (pts[ptIndex].tetraNum, pts[ptIndex].face);

                bool manifold = true;
                if (addFace && m_checkMarkedEdges)
                    {
                    // Check double edge
                    int countA = GetMarkedEdgeCount (e.a, pts[ptIndex].ptNum);
                    if (countA != 0)
                        manifold = countA == 1;

                    if (manifold)
                        {
                        int countB = GetMarkedEdgeCount (e.b, pts[ptIndex].ptNum);
                        if (countB != 0)
                            manifold = countB == 1;
                        }
                    }
#ifdef TRIMCHECKEREXITONBOUNDARY
                if (checkPt == -2)
                    manifold = false;
#endif

                addFace &= manifold;
                if (addFace)
                    IncrementMarkedEdgeCount (e.a, e.b, pts[ptIndex].tetraNum);

#ifdef TRIMCHECKER
                if (doTRIMCHECKER)
                    {
                    if (choosenPt != -2 || checkPt != -2)
                        {
                        if (checkPt != choosenPt)
                            {
                            if (checkPt != -2)
                                {
                                if (TRIMCHECKER_OUTPUTDIFF)
                                    {
                                    if (e.c != -1)
                                        {
                                        cout << "CheckTrimChcker Failed: " << endl;
                                        cout << "  " << std::setw (8) << m_points[e.a].x << "," << m_points[e.a].y << "," << m_points[e.a].z << endl;
                                        cout << "  " << std::setw (8) << m_points[e.b].x << "," << m_points[e.b].y << "," << m_points[e.b].z << endl;
                                        cout << "  " << std::setw (8) << m_points[e.c].x << "," << m_points[e.c].y << "," << m_points[e.c].z << endl;
                                        if (choosenPt == -2)
                                            cout << "no Chosen Pt" << endl;
                                        else
                                            cout << "  " << std::setw (8) << m_points[pts[choosenPt].ptNum].x << "," << m_points[pts[choosenPt].ptNum].y << "," << m_points[pts[choosenPt].ptNum].z << endl;
                                        GtTriangleQuality edgeDistHelper (m_points[e.a], m_points[e.b], e.c == -1 ? DPoint3d () : m_points[e.c], e.c != -1, m_maxL, GetNormalForPoint (pts, e.a, e.b, e.c));

                                        if (choosenPt != -2)
                                            edgeDistHelper.OutputValues (m_points[pts[choosenPt].ptNum]);

                                        cout << "  " << std::setw (8) << m_points[pts[checkPt].ptNum].x << "," << m_points[pts[checkPt].ptNum].y << "," << m_points[pts[checkPt].ptNum].z << endl;
                                        edgeDistHelper.OutputValues (m_points[pts[checkPt].ptNum]);
                                        }
                                    }
                                sTrimChecker.statsNumFailed++;
                                }
                            else
                                {
                                sTrimChecker.statsNumExtra++;
                                }
                            }
                        sTrimChecker.statsNum++;
                        }
#ifdef TRIMCHECKEREXITONBOUNDARY
                    if (checkPt == -2)
                        addFace = false;
#endif
                    }
#endif
                if (addFace)
                    {

#ifdef DHTEST
                    for (auto mi : testEdges)
                        {
                        if (MarkEdge::MakeIndex (e.a, pts[ptIndex].ptNum) == mi)
                            break;
                        }
                    for (auto mi : testEdges)
                        {
                        if (MarkEdge::MakeIndex (e.b, pts[ptIndex].ptNum) == mi)
                            break;
                        }
#endif

                    IncrementMarkedEdgeCount (e.a, pts[ptIndex].ptNum, pts[ptIndex].tetraNum);
                    IncrementMarkedEdgeCount (e.b, pts[ptIndex].ptNum, pts[ptIndex].tetraNum);

                    MarkFace (pts[ptIndex].tetraNum, pts[ptIndex].face);
                    AddFixedFace (pts[ptIndex].tetraNum, pts[ptIndex].face);


                    m_hasPt[pts[ptIndex].ptNum] = true;
#ifdef EGDEBUG
                    hasPtTetrahedron[pts[ptIndex].ptNum] = true;
                    hasPtTetrahedron[e.b] = true;
                    hasPtTetrahedron[e.c] = true;
                    hasTetrahedron[pts[ptIndex].tetraNum] = true;
#endif
#ifdef TRIMCHECKER
                    if (doTRIMCHECKER && !sTrimChecker.HasFace (e.a, e.b, pts[ptIndex].ptNum))
                        ptIndex = ptIndex;
#endif
#ifdef LOG_TETRA
                    log << "[Method3] PUSHING BACK EDGES " + std::to_string (e.a) + "/" + std::to_string (pts[ptIndex].ptNum) + " dists " +
                        std::to_string (distTo (m_points[e.a], m_points[pts[ptIndex].ptNum])) << endl;
                    log << "[Method3] PUSHING BACK EDGES " + std::to_string (e.b) + "/" + std::to_string (pts[ptIndex].ptNum) + " dists " +
                        std::to_string (distTo (m_points[e.b], m_points[pts[ptIndex].ptNum])) << endl;
#endif
                    if (e.c == -1)
                        {
                        newEdges.push_back (edge (e.a, e.b, pts[ptIndex].ptNum, pts[ptIndex].tetraNum));
                        newEdges.push_back (edge (pts[ptIndex].ptNum, e.b, e.a, pts[ptIndex].tetraNum));
                        newEdges.push_back (edge (e.a, pts[ptIndex].ptNum, e.b, pts[ptIndex].tetraNum));
                        }
                    else
                        {
                        bool useBack = false;
                        if (!edges.empty ())
                            {
                            if (edges.back ().b == e.a && edges.back ().a == pts[ptIndex].ptNum)
                                {
                                edges.push_back (edge (pts[ptIndex].ptNum, e.b, e.a, pts[ptIndex].tetraNum));
                                useBack = true;
                                }
                            }

                        if (!useBack)
                            {
                            edges.push_back (edge (e.a, pts[ptIndex].ptNum, e.b, pts[ptIndex].tetraNum));
                            newEdges.push_back (edge (pts[ptIndex].ptNum, e.b, e.a, pts[ptIndex].tetraNum));
                            }
                        }
                    TRACE_TETRA_END ();
                    }
                }
            else
                {
#ifdef EGDEBUG
                log << " REJECTING EDGE " + std::to_string (e.a) + "--" + std::to_string (e.b) + " OF TETRAHEDRON " + std::to_string (e.tetraIndx) + " WITH DIST 1 " + std::to_string (nearestDist)
                    + " WITH DIST 2 " + std::to_string (nearestDist2) + " WITH POINTS CHECKED " + std::to_string (pts.size ()) << endl;
#endif
                }

            }

#ifdef DHDEBUG
        //            break;
#endif
        addPts.clear ();
        FindPointToScan (addPts, ++d);

        if (addPts.empty ())
            break;
        }
    //std::cout << "Loop " << c << "," << d << std::endl;
#ifdef TRIMCHECKER
    if (doTRIMCHECKER)
        {
        qualityStats = sTrimChecker.Report ();
        if (qualityStats < 97)
            qualityStats = qualityStats;
        }
#endif
#ifdef EGDEBUG
    // for (auto it = tetraqty.begin(); it != tetraqty.end(); it++) if (it->second >= 6) log << "TETRAHEDRON # " + std::to_string(it->first) + ":" + std::to_string(it->second) <<endl;
    log << "Points removed: " + std::to_string (std::count (hasPtTetrahedron.begin (), hasPtTetrahedron.end (), false)) + " out of " + std::to_string (m_points.size ()) << endl;
    for (auto it = hasPtTetrahedron.begin (); it != hasPtTetrahedron.end (); it++) if (*it == false) log << "Removed point " + std::to_string (it - hasPtTetrahedron.begin ()) << endl;
    log << "Tets removed: " + std::to_string (std::count (hasTetrahedron.begin (), hasTetrahedron.end (), false)) + " out of " + std::to_string (m_tetrahedrons.size ()) << endl;
    log.close ();
#endif

#ifdef TESTVALUES
    return;  // no point in trimming the boundary if we are testing values.
#endif
    if (m_trimFromEdge) // && m_checkMarkedEdges)
        TrimFromEdge ();
    }

void TrimHull::TrimFromEdge ()
    {
    if (m_trimLongTrianglesLength == 0)
        return;
#ifdef RECALCEDGEMARKS
    bmap<uint64_t, MarkEdge> markEdge2;

    for (size_t i = 0; i < m_tetrahedrons.size (); i++)
        {
        const auto& t = m_tetrahedrons[i];
        for (int f = 0; f < 4; f++)
            {
            if (IsFacePrimaryMarked ((int)i, f))
                {
                auto pI = Tetrahedron::GetDTetrahedron3dFacet (f);
                for (int p = 0; p < 3; p++)
                    {
                    auto& mk = markEdge2[MarkEdge::MakeIndex (t.ptNums[pI[p]], t.ptNums[pI[(p + 1) % 3]])];
                    mk.count++;
                    mk.tet = (int)i;
                    }
                }
            }
        }

    //        markEdge = markEdge2;
#endif

    int numRemoved = 0;
    int numNonManifold = 0;
    int numOnEdge = 0;
    double maxLen = m_trimLongTrianglesLength * m_trimLongTrianglesLength;

    struct REFIX
        {
        int tet; int face;
        };
#ifdef DEBUGMSG
    int num1 = 0;
    int num2 = 0;
    int num3 = 0;
    int numQ = 0;
    for (auto& edge : m_markEdges)
        {
        int count = edge.second.count;
        switch (count)
            {
            case 1:
                num1++;
                break;
            case 2:
                num2++;
                break;
            case 3:
                num3++;
                break;
            default:
                numQ++;
                break;
            }
        }
    cout << "edge Counts " << num1 << " " << num2 << " " << num3 << " " << numQ << endl;
#endif

#ifdef SHOWREMOVED
    bvector<REFIX> refix;
#endif
    int numFixed = 0;
    bool removedEdge = true;
    while (removedEdge)
        {
        numOnEdge = 0;
        numNonManifold = 0;
        removedEdge = false;
        for (auto& edge : m_markEdges)
            {
            // Should include something for more than 2.
            if (edge.second.count == 0 || edge.second.count == 2)
                continue;

            if (edge.second.count == 1)
                {
                numOnEdge++;
                bvector<PointOnEdge> pts;
                int ptA = edge.first >> 32;
                int ptB = edge.first & 0xffffffff;
                m_meshData.FindPointsAroundEdge (edge.second.tet, ptA, ptB, pts);
                const double edgeLenSquared = distToSquared (m_points[ptA], m_points[ptB]);
                bool fixed = false;
                for (auto const& pt : pts)
                    {
                    auto& mkA = GetMarkedEdge (ptA, pt.ptNum);
                    if (mkA.IsValid () && mkA.count == 1)
                        {
                        auto& mkB = GetMarkedEdge (ptB, pt.ptNum);
                        if (mkB.IsValid () && mkB.count == 1)
                            {
                            numFixed++;
                            MarkFace (pt.tetraNum, pt.face);
                            AddFixedFace (pt.tetraNum, pt.face);
                            mkA.count++;
                            mkB.count++;
                            edge.second.count++;
                            break;
                            }
                        }
                    }
                if (fixed)
                    continue;

                for (auto const& pt : pts)
                    {
                    if (!IsFaceMarked (pt.tetraNum, pt.face))
                        continue;
                    if (m_useSliverForTrim)
                        {
                        static const double sliverRatio = 0.025;
                        const double s1s = distToSquared (m_points[pt.ptNum], m_points[ptA]);
                        if (edgeLenSquared <= s1s)
                            continue;
                        const double s2s = distToSquared (m_points[pt.ptNum], m_points[ptB]);

                        if (edgeLenSquared <= s2s)
                            continue;
                        const double edgeLen = sqrt (edgeLenSquared);
                        if ((sqrt (s1s) + sqrt (s2s) - edgeLen) / edgeLen > sliverRatio)
                            continue;
                        }
                    else
                        {
                        if (edgeLenSquared < maxLen)
                            continue;
                        }

                    removedEdge = true;
                    ClearMarkedFace (pt.tetraNum, pt.face);
                    RemoveFixedFace (pt.tetraNum, pt.face);
                    REFIX r;
                    r.tet = pt.tetraNum;
                    r.face = pt.face;
#ifdef SHOWREMOVED
                    refix.push_back (r);
#endif
                    numRemoved++;
                    DecrementMarkedEdgeCount (ptA, ptB);
                    DecrementMarkedEdgeCount (ptA, pt.ptNum);
                    DecrementMarkedEdgeCount (ptB, pt.ptNum);
                    }
                }
            else
                {
                // Do nothing for now. non manifold.
                numNonManifold++;
                }
            }
        }
#ifdef DEBUGMSG
    cout << "Trimmed " << numRemoved << " from Edge " << numOnEdge << " OnEdge " << numNonManifold << " Number Non Manifold" << endl;
#endif
#ifdef SHOWREMOVED
    for (auto& a : m_trimInfo)
        {
        a.fixedFace = 0;
        }

    for (auto& t : refix)
        {
        MarkFace (t.tet, t.face);
        }
#endif
    }

#pragma endregion

#ifdef TESTVALUES
const static int minV_ = -1;
const static int minV = -1;
const static int maxV = (9 * 4);
double CalcQuality (int a)
    {
    if (a <= minV_) return 0;
    unsigned int decimal = a / 9;
    double val = (a % 9) + 1;
    return pow (10, decimal) * (val);
    }

#define FORQ(v) for (q[v] = minV; q[v] < (maxV + stepV); q[v] += stepV) if (runProcess == 0 || CheckQ(bv[v], pbv[v], q[v], v, minV + stepV, maxV, stepV, c[v]))
#define FORQV(v, minV, maxV, stepV) for (q[v] = minV; q[v] < (maxV + stepV); q[v] += stepV) if (runProcess == 0 || CheckQ(bv[v], pbv[v], q[v], v, minV + stepV, maxV, stepV, c[v]))
#define FORQS(v, stepV) for (q[v] = minV; q[v] < maxV; q[v] += stepV)

#pragma optimize( "", off )

bool CheckQ (double& bv_, double& pbv, int& qv, int v, int minV, int maxV, int stepV, int* c)
    {

    if (bv_ == -1)
        return true;
    if (qv >= maxV)
        {
        //        c[qv - stepV]++;
        return false;
        }

    double bv = bv_;
    bv_ = -1;

    if (qv == minV_)
        {
        pbv = -1;
        return true;
        }
    else if (qv == minV)
        {
        pbv = 0;
        return true;
        }
    if (pbv == bv)
        return true;
    if (pbv < (bv + 0.01))
        {
        if (pbv < bv)
            pbv = bv;
        return true;
        }
    bv_ = -2;
    //    c[qv - stepV]++;
    return true;
    }
#endif

int TrimHull::FixFacesAroundEdge (long i, std::vector<std::vector<std::pair<long, char>>>& edgeTets, std::vector<EdgeState>& edgeList, std::vector<int[6]>& edgeMap, std::vector<edge>* retEdges)
    {
    bvector<PointOnEdge> pts;
    edgeList[i] = EdgeState::VISITED;
    long edgePts[2] = { -1, -1 };
    int nLeftToVisit = 0;
    std::map<double, std::pair<DPoint3d, std::pair<long, char>>> map;
    char nFaces = 0;
    std::pair<DPoint3d, std::pair<long, char>> faces[2];
    std::vector<long> fixedFacetsThirdPoint;
    for (auto tetRef : edgeTets[i])
        {
        for (int j = 0; j < 2; j++)
            {
            double circumRadius;
            DPoint3d circumCenter;
            int ptsI[3];
            const Tetrahedron& t = m_tetrahedrons[tetRef.first];
            t.GetFacePoints (edgeToFaces[tetRef.second][j], ptsI);
            if (ptsI[0] >= m_ignorePtsAfterNum || ptsI[1] >= m_ignorePtsAfterNum || ptsI[2] >= m_ignorePtsAfterNum) continue;
            if (edgePts[0] == -1)
                {
                edgePts[0] = t.ptNums[edgeToPoints[tetRef.second][0]];
                edgePts[1] = t.ptNums[edgeToPoints[tetRef.second][1]];
                }

            double dist = distTo (m_points[edgePts[0]], m_points[edgePts[1]]);
            circumRadius = Circumcircle (m_points[ptsI[0]], m_points[ptsI[1]], m_points[ptsI[2]], circumCenter);
            DPoint3d triangleCentroid;
            DPoint3d pts[3] = { m_points[ptsI[0]], m_points[ptsI[1]], m_points[ptsI[2]] };
            bsiPolygon_centroidAreaPerimeter (pts, 3, &triangleCentroid, NULL, NULL, NULL, NULL);
            if (IsFaceMarked (tetRef.first, edgeToFaces[tetRef.second][j]))
                {
                long p;
                for (int k = 0; k < 3; k++) if (ptsI[k] != edgePts[1] && ptsI[k] != edgePts[0]) p = ptsI[k];
                if (std::find (fixedFacetsThirdPoint.begin (), fixedFacetsThirdPoint.end (), p) != fixedFacetsThirdPoint.end ()) continue;
                fixedFacetsThirdPoint.push_back (p);
                map.insert (std::make_pair (0, std::make_pair (triangleCentroid, std::make_pair (tetRef.first, edgeToFaces[tetRef.second][j]))));
                if (nFaces < 2)
                    {
                    faces[nFaces] = std::make_pair (triangleCentroid, std::make_pair (tetRef.first, edgeToFaces[tetRef.second][j]));
                    nFaces++;
                    }
                }
            map.insert (std::make_pair (circumRadius, std::make_pair (triangleCentroid, std::make_pair (tetRef.first, edgeToFaces[tetRef.second][j]))));
            }
        }
    for (auto it = map.begin (); it != map.end () && nFaces < 2; it++)
        {
        /*if (IsFaceMarked(it->second.second.first, it->second.second.second))
        {
        faces[nFaces++] = it->second;
        continue;
        }*/
        if (nFaces == 0)
            {
            faces[0] = it->second;
            nFaces = 1;
            continue;
            }
        DPoint3d center = it->second.first;
        DSegment3d edge = DSegment3d::From (m_points[edgePts[0]], m_points[edgePts[1]]);
        DSegment3d centers = DSegment3d::From (m_points[edgePts[0]], center);
        //DSegment3d centers = DSegment3d::From(center, faces[0].first);
        int ptsI[3];
        Tetrahedron& t = m_tetrahedrons[faces[0].second.first];
        t.GetFacePoints (faces[0].second.second, ptsI);
        DPoint3d pts[3] = { m_points[ptsI[0]], m_points[ptsI[1]], m_points[ptsI[2]] };
        DVec3d normal;
        GetPlaneNormal (&normal, pts, pts + 1, pts + 2);
        DVec3d axisY = DVec3d::FromStartEnd (m_points[edgePts[0]], m_points[edgePts[1]]);
        DPoint3d centroid = faces[0].first;
        DVec3d axisX = DVec3d::FromStartEnd (m_points[edgePts[0]], centroid);
        center.DifferenceOf (center, m_points[edgePts[0]]);
        centroid.DifferenceOf (centroid, m_points[edgePts[0]]);
        RotMatrix toCurrentFace;
        toCurrentFace.InitFrom2Vectors (axisX, axisY);
        toCurrentFace.Invert ();
        toCurrentFace.Multiply (center);
        toCurrentFace.Multiply (centroid);
        if ((center.x < 0 && centroid.x> 0) || (center.x > 0 && centroid.x< 0))
            {
            faces[1] = it->second;
            nFaces++;
            }
        /*    DPoint3d pointA, pointB;
        double fractionA, fractionB;
        if (DRay3d::ClosestApproachUnboundedRayUnboundedRay(
        fractionA, fractionB, pointA, pointB, DRay3d::From(edge), DRay3d::From(centers))
        && fractionA >= 0.0 && fractionA <= 1.0
        && fractionB >= 0.0 && fractionB <= 1.0)
        {
        // faces[1] = it->second;
        // nFaces++;
        }*/
        }
    for (char f = 0; f < nFaces; f++)
        {
        long t = faces[f].second.first;
        char fi = faces[f].second.second;
        if (IsFaceMarked (t, (int)fi)) continue;
        MarkFace (t, (int)fi);
        AddFixedFace (t, fi);
        Tetrahedron tet = m_tetrahedrons[t];
        const int * faceP = Tetrahedron::GetDTetrahedron3dFacet (fi);
        if (retEdges != nullptr)
            {
            retEdges->push_back (edge (tet.ptNums[faceP[0]], tet.ptNums[faceP[1]], tet.ptNums[faceP[2]], t));
            retEdges->push_back (edge (tet.ptNums[faceP[1]], tet.ptNums[faceP[2]], tet.ptNums[faceP[0]], t));
            retEdges->push_back (edge (tet.ptNums[faceP[2]], tet.ptNums[faceP[0]], tet.ptNums[faceP[1]], t));
            }
        for (int j = 0; j < 3; j++)
            {
            for (int k = 0; k < 3; k++)
                if (k != j && edgeList[edgeMap[t][pointsToEdges[faceP[j]][faceP[k]]]] == EdgeState::UNSET)
                    {
                    edgeList[edgeMap[t][pointsToEdges[faceP[j]][faceP[k]]]] = EdgeState::TOVISIT;
                    nLeftToVisit++;
                    }
            }
        }
    return nLeftToVisit;
    }

void TrimHull::Method4 ()
    {
#if 0
    std::vector<std::map<long, long>> pointEdges (m_points.size ());
    std::vector<int[6]> edges (m_tetrahedrons.size ());
    std::vector<std::vector<std::pair<long, char>>> edgeTets (m_tetrahedrons.size () * 6);
    std::vector<EdgeState> edgeList (m_tetrahedrons.size () * 6);
    int threshold = m_ignorePtsAfterNum % 2 == 0 ? (int)m_ignorePtsAfterNum - 1 : (int)m_ignorePtsAfterNum - 2;
    PerfectMatching match (threshold + 1, (int)m_tetrahedrons.size () * 3);
    match.options.fractional_jumpstart = true;
    match.options.update_duals_after = true;
    match.options.single_tree_threshold = 0.5;

    for (int i = 0; i < (int)m_tetrahedrons.size (); i++)
        {
        if (m_tetrahedrons[i].ptNums[0] == -1)
            continue;
        edges[i][0] = edges[i][1] = edges[i][2] = edges[i][3] = edges[i][4] = edges[i][5] = -1;
        Tetrahedron& t = m_tetrahedrons[i];
        double d[6] = { distToSquared (m_points[t.ptNums[0]], m_points[t.ptNums[1]]), distToSquared (m_points[t.ptNums[1]], m_points[t.ptNums[2]]),
            distToSquared (m_points[t.ptNums[0]], m_points[t.ptNums[2]]), distToSquared (m_points[t.ptNums[0]], m_points[t.ptNums[3]]),
            distToSquared (m_points[t.ptNums[1]], m_points[t.ptNums[3]]), distToSquared (m_points[t.ptNums[2]], m_points[t.ptNums[3]]) };

        for (int e = 0; e < 6; e++)
            {
            int pt1 = t.ptNums[edgeToPoints[e][0]], pt2 = t.ptNums[edgeToPoints[e][1]];
            if (pt1 <= threshold && pt2 <= threshold)
                {
                if (pointEdges[pt1].count (pt2) == 0)
                    edges[i][e] = pointEdges[pt1][pt2] = pointEdges[pt2][pt1] = match.AddEdge (pt1, pt2, d[e]);
                else
                    {
                    edges[i][e] = pointEdges[pt1][pt2];
                    }
                edgeTets[edges[i][e]].push_back (std::make_pair (i, e));
                }
            }
        }
    match.Solve ();
    int edgesLeft = 0;
    for (long i = 0; i < (long)m_tetrahedrons.size () * 6; i++)
        {
        if (edgeTets[i].size () == 0)
            {
            edgeList[i] = EdgeState::VISITED;
            continue;
            }
        if (match.GetSolution (i))
            {
            edgesLeft += FixFacesAroundEdge (i, edgeTets, edgeList, edges);
            }
        }
    edgesLeft = 0;
    for (long i = 0; i < (long)m_tetrahedrons.size () * 6; i++)
        {
        if (edgeTets[i].size () == 0) continue;
        if (edgeList[i] == EdgeState::TOVISIT)
            {
            edgesLeft += FixFacesAroundEdge (i, edgeTets, edgeList, edges);
            }
        }
#endif
    }

bool TrimHull::AddFixFaceWithEdges (int tn, int f)
    {
    //        cout << "Fix Facet " << tn << " " << f << endl;
    const Tetrahedron& tet = m_tetrahedrons[tn];
    const int* pI = Tetrahedron::GetDTetrahedron3dFacet (f);
    const int a = tet.ptNums[pI[0]];
    const int b = tet.ptNums[pI[1]];
    const int c = tet.ptNums[pI[2]];

    if (IncrementMarkedEdgeCount (a, b, tn) > 2)
        {
        DecrementMarkedEdgeCount (a, b);
        return false;
        }
    if (IncrementMarkedEdgeCount (b, c, tn) > 2)
        {
        DecrementMarkedEdgeCount (a, b);
        DecrementMarkedEdgeCount (b, c);
        return false;
        }
    if (IncrementMarkedEdgeCount (c, a, tn) > 2)
        {
        DecrementMarkedEdgeCount (a, b);
        DecrementMarkedEdgeCount (b, c);
        DecrementMarkedEdgeCount (c, a);
        return false;
        }
    m_hasPt[a] = true;
    m_hasPt[b] = true;
    m_hasPt[c] = true;
    AddFixedFace (tn, f);
    return true;
    }

__forceinline  bool MarkTheEdge (std::map<uint64_t, uint64_t>& hasEdge, int a, int b)
    {
    auto mk = hasEdge.find (MarkEdge::MakeIndex (a, b));
    if (mk != hasEdge.end ())
        {
        return false;
        }
    hasEdge[MarkEdge::MakeIndex (a, b)] = 0;
    return true;
    }

void TrimHull::Method5AddRun (std::vector<PointOnEdgeWithEdge>& runPoints)
    {
    if (runPoints.empty ())
        return;
    // 1. Need to mark edges and faces.
    TrimRun newRun;

    CurveVectorPtr sticks = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    std::vector<edge> singleEdges;
    std::map<uint64_t, uint64_t> hasEdge;
    for (auto& pt : runPoints)
        {
        if (pt.ignore)
            continue;
        const Tetrahedron& tet = m_tetrahedrons[pt.tetraNum];
        int a = pt.a;
        int b = pt.b;
        int c = pt.ptNum;
        int count = GetMarkedEdgeCount (c, a);
        if (count == 1 && MarkTheEdge (hasEdge, c, a))
            {
            singleEdges.push_back (edge (c, a, b, pt.tetraNum));
            auto primitive = ICurvePrimitive::CreateLine (DSegment3d::From (DPoint3d::From (c, c, c), DPoint3d::From (a, a, a)));
            primitive->SetTag ((int)singleEdges.size () - 1);
            sticks->push_back (primitive);
            }
        else if (count == 2)
            GetMarkedEdge (c, a).done = true;

        count = GetMarkedEdgeCount (a, b);
        if (count == 1 && MarkTheEdge (hasEdge, a, b))
            {
            singleEdges.push_back (edge (a, b, c, pt.tetraNum));
            auto primitive = ICurvePrimitive::CreateLine (DSegment3d::From (DPoint3d::From (a, a, a), DPoint3d::From (b, b, b)));
            primitive->SetTag ((int)singleEdges.size () - 1);
            sticks->push_back (primitive);
            }
        else if (count == 2)
            GetMarkedEdge (c, a).done = true;

        count = GetMarkedEdgeCount (b, c);
        if (count == 1 && MarkTheEdge (hasEdge, b, c))
            {
            singleEdges.push_back (edge (b, c, a, pt.tetraNum));

            auto primitive = ICurvePrimitive::CreateLine (DSegment3d::From (DPoint3d::From (b, b, b), DPoint3d::From (c, c, c)));
            primitive->SetTag ((int)singleEdges.size () - 1);
            sticks->push_back (primitive);
            }
        else if (count == 2)
            GetMarkedEdge (c, a).done = true;
        }

    CurveVectorPtr chains = sticks->AssembleChains ();
    int numUsedLinks = 0;

    for (ICurvePrimitivePtr& chain : *chains.get ())
        {
        int prevPt = -1;
        newRun.edges.clear ();
        auto ccvP = chain->GetChildCurveVectorCP ();

        if (nullptr != ccvP)
            {
            auto ccv = *ccvP;

            for (int i = 0; i < (int)ccv.size (); i++)
                {
                int seI = ccv[i]->GetTag ();
                auto line = ccv[i]->GetLineCP ();
                bool isSwapped = false;

                if (line->point[0].x != singleEdges[seI].a)
                    {
                    isSwapped = true;
                    //                    std::swap (singleEdges[seI].a, singleEdges[seI].b);
                    }

                newRun.edges.push_back (singleEdges[seI]);

                //Test Code
                if (prevPt != -1 && prevPt != singleEdges[seI].a)
                    i = i;
                if (isSwapped)
                    prevPt = singleEdges[seI].a;
                else
                    prevPt = singleEdges[seI].b;
                numUsedLinks++;
                }
            }
        else
            {
            BeAssert (false);
            }
        if (!newRun.edges.empty ())
            m_newRuns.push_back (newRun);
        }

    if (numUsedLinks != (int)singleEdges.size ())
        numUsedLinks = numUsedLinks;
    }

bool TrimHull::Method5NextPoint (int ea, int eb, int ec, int tetIndex, PointOnEdgeWithEdge& nextPoint)
    {
    //2141985.5800m, 223654.2800m, 756.0800m
    DPoint3d _tp[] = {
            { 2141980.0400, 223659.4300, 756.3400 },
            { 2141985.5800, 223654.2800, 756.0800 },
        };

    if ((ea == 2705 && eb == 2465) || (ea == 2705 && eb == 2465))
        ec = ec;
//    2705,2604, 2465
    if ((m_points[ea].AlmostEqual (_tp[0]) || m_points[eb].AlmostEqual (_tp[1]))
        || (m_points[eb].AlmostEqual (_tp[0]) || m_points[ea].AlmostEqual (_tp[1])))
        {
        ec = ec;
        }

    nextPoint.a = ea;
    nextPoint.b = eb;
    nextPoint.ptNum = -2;
    // If this edge is already marked twice ignore. shouldn't happen
    //if (GetMarkedEdgeCount (ea, eb) >= 2)
    //    return false;

    bvector<PointOnEdge> pts;
    m_meshData.FindPointsAroundEdge (tetIndex, ea, eb, pts);

#ifdef TRIMCHECKER
    if (doTRIMCHECKER && USETRIMCHECKER)
        {
        for (int i = 0; i < (int)pts.size (); i++)
            {
            if (pts[i].ptNum == ec)
                continue;
            if (sTrimChecker.HasFace (ea, eb, pts[i].ptNum))
                {
                nextPoint.ptNum = pts[i].ptNum;
                nextPoint.tetraNum = pts[i].tetraNum;
                nextPoint.face = pts[i].face;
                return true;
                }
            }
        return false;
        }
#endif
    //Check for fixed facet first.
    for (int i = 0; i < (int)pts.size (); i++)
        {
        if (pts[i].ptNum == ec)
            continue;
        if (IsFaceMarked (pts[i].tetraNum, pts[i].face))
            {
            nextPoint.ptNum = pts[i].ptNum;
            nextPoint.tetraNum = pts[i].tetraNum;
            nextPoint.face = pts[i].face;
            return true;
            }
        }

    int ptIndex = -2;
    if (getIndexMethod == 0)
        {
        ptIndex = getIndexViaCircumcenter (pts, ea, eb, ec);
        }
    else if (getIndexMethod == 1)
        {
        static const bool usePlane = true;
        ptIndex = getIndexViaTriangulation (pts, ea, eb, ec, usePlane);
        }
    else if (getIndexMethod == 2)
        {
        ptIndex = getIndexViaQuality (pts, ea, eb, ec);
        }
    else if (getIndexMethod == 3)
        {
        static const bool usePlane = true;
        ptIndex = getIndexViaTest (pts, ea, eb, ec, usePlane);
        }
    else if (getIndexMethod == 4)
        {
        static const bool usePlane = true;
        ptIndex = getIndexViaTest2 (pts, ea, eb, ec, usePlane);
        }
    else if (getIndexMethod == 5)
        ptIndex = getIndexViaBestCircumcenter (pts, ea, eb, ec);
    else if (getIndexMethod == 6)
        ptIndex = getIndexViaAngleCluster (pts, ea, eb, ec);
    else if (getIndexMethod == 7)
        ptIndex = getIndexViaCC2(pts, ea, eb, ec);


    if (ptIndex != -2)
        {
        //double d1 = distToSquaredZ (m_points[ea], m_points[pts[ptIndex].ptNum]);
        //double d2 = distToSquaredZ (m_points[ea], m_points[pts[ptIndex].ptNum]);

        //if (ec != -1)
        //    {
        //    if (max (d1, d2) > (20 * 20)) //GetMaxDist (m_points[ea], m_points[eb], m_points[ec]))
        //        {
        //        d1 = d1;
        //        }
        //    }
        nextPoint.ptNum = pts[ptIndex].ptNum;
        nextPoint.tetraNum = pts[ptIndex].tetraNum;
        nextPoint.face = pts[ptIndex].face;
        }

    if (nextPoint.ptNum == -2)
        return false;

    // These are ok, loop back.
    if (GetMarkedEdgeCount (ea, nextPoint.ptNum) == 2)
        return false;
    if (GetMarkedEdgeCount (eb, nextPoint.ptNum) == 2)
        return false;
    return true;
    }
int numBackwardsLinked = 0;
int numBackwardsLinkedSuccess = 0;

double getMaxDistMulti = 0.7;
int getMaxDistMaxNum = 3;
int getMaxDistMethod = -1;

double TrimHull::GetMaxDist (DPoint3dCR pa, DPoint3dCR pb, DPoint3dCP pc)
    {
    double maxDist = m_maxL2;

    if (getMaxDistMethod == -1)
      return pow (m_maxL * 4, 2);

    if (getMaxDistMethod == 0)
        {
        return pow (distTo (pa, pb) * getMaxDistMulti, 2);
        }

    if (getMaxDistMethod == 1 && pc)
        {
        return pow ((distTo (pa, pb) + distTo (pa, *pc) + distTo (*pc, pb)) * getMaxDistMulti, 2);
        }

    if (getMaxDistMethod == 2 && pc)
        {
        return pow((max (distTo (pa, pb), max (distTo (pa, *pc), distTo (*pc, pb)))) * getMaxDistMulti, 2);
        }
    if (getMaxDistMethod == 3 && pc)
        {
        return pow((min (distTo (pa, pb), min (distTo (pa, *pc), distTo (*pc, pb)))) * getMaxDistMulti, 2);
        }
    if (getMaxDistMethod == 4)
        return pow (getMaxDistMulti, 2);

    return m_maxL2;
    }

inline void TrimHull::GetPtsForTest (bvector<PointOnEdge>& retPts, int tetNum, int ea, int eb, int ec, bool usePlane)
    {
    BeAssert (ec != -1);
    if (ec == -1)
        return;
    DPoint3dCR pa = m_points[ea];
    DPoint3dCR pb = m_points[eb];
    DPoint3dCP pc = ec == -1 ? nullptr : &m_points[ec];
    Transform t2;

    bvector<PointOnEdge> pts;
    retPts.clear ();
    m_meshData.FindPointsAroundEdge (tetNum, ea, eb, pts);
    if (pts.empty ())
        {
        // BeAssert () shouldn't happen
        return;
        }

    //for (auto& p : pts)
    //    retPts.push_back (p);
    //return;
    if (usePlane)
        {
        DVec3d plane;
        plane = GetNormalForPoint (pts, ea, eb, ec);

        RotMatrix mat;

        mat.InitFrom1Vector (plane, 2, true);
        Transform t = Transform::From (mat, pa);
        t2.InverseOf (t);
        }

    DPoint3d p[3];
    double useA = 1;
    p[0] = pa;
    p[1] = pb;
    p[2] = *pc;
    if (usePlane)
        t2.multiply (p, 3);

    double orient = orient2d (p[0], p[1], p[2]);

    if (orient == 0)
        {
        if (!usePlane)
            return;
        // Use face normal.
        DVec3d plane;
        GetPlaneNormal (&plane, &pa, &pb, pc);

        RotMatrix mat;

        mat.InitFrom1Vector (plane, 2, true);
        Transform t = Transform::From (mat, pa);
        t2.InverseOf (t);
        p[0] = pa;
        p[1] = pb;
        p[2] = *pc;
        if (usePlane)
            t2.multiply (p, 3);
        orient = orient2d (p[0], p[1], p[2]);
        }
    if (orient > 0)
        useA = -1;

    int numPointsAdded = 0;
    double maxDist = GetMaxDist (pa, pb, pc);
    bool ecFound = false;
    for (int i = 0; i < (int)pts.size (); i++)
        {
        if (pts[i].ptNum == ec)
            ecFound = true;
        if (/*pts[i].ptNum != ec && */!IgnorePoint (pts[i].ptNum))
            {
            //if (IsFaceMarked (pts[i].tetraNum, pts[i].face))
            //    return i;

            DPoint3d pt;
            pt = m_points[pts[i].ptNum];

            if (maxDist < distToSquared (pa, pt))
                continue;

            if (maxDist < distToSquared (pb, pt))
                continue;

            if (usePlane)
                t2.multiply (&pt);

            double side = 1;
            if (nullptr != pc)
                side = orient2d (p[0], p[1], pt) * useA;

            if (side > 0)
                {
                retPts.push_back (pts[i]);
                }
            }
        }
    if (!ecFound)
        ecFound = true;
    }

bool TrimHull::Method5FixScan (int startTet, int startPt, int startEC, std::vector<PointOnEdgeWithEdge>& newPoints, int newPointStart, int endPoint, int endEC, int endTetNum, std::vector<PointOnEdgeWithEdge>& newBPoints, int pivotPt)
    {
    int bestFI = -1;
    int bestBI = -1;
    double bestD = 0;
    PointOnEdgeWithEdge bestP;
    double closestPointDist = -1;
    int closestPointF = -1;
    int closestPointB = -1;

    struct nextPts
        {
        int ptNum;
        int fI;
        int bI;
        int b;
        PointOnEdgeWithEdge a[2];
        };
    bvector<nextPts> nextPoints;
    bvector<PointOnEdge> pts;

    // Try and find the shorted valid face between pivotPoint and the points in newPoints and newBPoints.
    for (int fI = newPointStart - 1; fI < (int)newPoints.size (); fI++)
        {
        int b;
        int ec;
        int tetNum;
        // Store points.

        // Get points around edge.
        if (fI == newPointStart - 1)
            {
            b = startPt;
            tetNum = startTet;
            ec = startEC;
            }
        else
            {
            tetNum = newPoints[fI].tetraNum;
            b = newPoints[fI].ptNum;
            ec = newPoints[fI].a;
            }

        m_meshData.FindPointsAroundEdge (tetNum, pivotPt, b, pts);
        //GetPtsForTest (pts, tetNum, pivotPt, b, ec, true);
        for (int pI = 0; pI < (int)pts.size (); pI++)
            {
            if (IgnorePoint (pts[pI].ptNum))
                continue;
            nextPts v;
            v.fI = fI;
            v.b = b;
            v.ptNum = pts[pI].ptNum;
            v.a[0] = PointOnEdgeWithEdge (pivotPt, b, pts[pI]);
            nextPoints.push_back (v);
            }
        for (int bI = -1; bI < (int)newBPoints.size (); bI++)
            {
            int testPt;

            if (bI == -1)
                {
                if (fI == newPointStart - 1)
                    continue;
                testPt = endPoint;
                }
            else
                testPt = newBPoints[bI].b;

            double dist = distToSquared (m_points [testPt], m_points[b]);
            if (closestPointF == -1 || dist < closestPointDist)
                {
                closestPointDist = dist;
                closestPointF = fI;
                closestPointB = bI;
                }

            for (int pI = 0; pI < (int)pts.size (); pI++)
                {
                // If point isn't Ignored add it to
                if (pts[pI].ptNum == testPt)
                    {
                    // see if this point is in the edge list, if so get the distance.
                    double dist = distToSquared (m_points[pts[pI].ptNum], m_points[b]);

                    if (dist < m_maxL2 && (bestFI == -1 || dist < bestD))
                        {
                        bestD = dist;
                        bestFI = fI;
                        bestBI = bI;
                        bestP.a = b;
                        bestP.b = pivotPt;
                        bestP.ptNum = pts[pI].ptNum;
                        bestP.tetraNum = pts[pI].tetraNum;
                        bestP.face = pts[pI].face;
                        }
                    }
                }
            }
        }

    int bestNI = -2;
    double nextDist = 1e30;
    for (int bI = -1; bI < (int)newBPoints.size (); bI++)
        {
        bvector<PointOnEdge> pts;
        int b;
        int ec;
        int tetNum;
        // Store points.

        // Get points around edge.
        if (bI == -1)
            {
            b = endPoint;
            ec = endEC;
            tetNum = endTetNum;
            }
        else
            {
            b = newBPoints[bI].ptNum;
            tetNum = newBPoints[bI].tetraNum;
            ec = newBPoints[bI].a;
            }
        m_meshData.FindPointsAroundEdge (tetNum, pivotPt, b, pts);
        //GetPtsForTest (pts, tetNum, pivotPt, b, ec, true);

        for (int pI = 0; pI < (int)pts.size (); pI++)
            {
            if (IgnorePoint (pts[pI].ptNum))
                continue;

            for (int fI = 0; fI < (int)nextPoints.size (); fI++)
                {
                if (nextPoints[fI].ptNum == pts[pI].ptNum)
                    {
                    double dist = distTo (m_points[nextPoints[fI].b], m_points[pts[pI].ptNum]);
                    double dist1 = distTo (m_points[b], m_points[pts[pI].ptNum]);
                    double totalDist = pow (dist, 2) + pow (dist1, 2);

                    if (totalDist > m_maxL2)
                        continue;
                    bool useV = false;
                    if (bestNI == -1)
                        {
                        useV = true;
                        }
                    else if (nextDist < totalDist)
                        {
                        useV = true;
                        }
                    if (useV)
                        {
                        if (bI == -1 && nextPoints[fI].fI == newPointStart - 1)
                            useV = false;
                        }
                    if (useV)
                        {
                        bestNI = fI;
                        nextPoints[fI].bI = bI;
                        nextPoints[fI].a[1] = PointOnEdgeWithEdge (pivotPt, b, pts[pI]);
                        nextDist = totalDist;
                        }
                    }
                }
            }
        }
    // Need to loop around the backward edges and see if any matches with a point from the previous search.
    if (bestFI != -1)
        {
        if (bestFI != -1 && bestD > nextDist)
            bestD = bestD;
        newPoints.resize (bestFI + 1);
        newPoints.push_back (bestP);
        newBPoints.resize (bestBI + 1);
        return true;
        }
    if (bestNI != -2)
        {
        newPoints.resize (nextPoints[bestNI].fI + 1);
        newPoints.push_back (nextPoints[bestNI].a[0]);
        newBPoints.resize (nextPoints[bestNI].bI + 1);
        newBPoints.push_back (nextPoints[bestNI].a[1]);
        return true;
        }

    int newPointsSize = (int)newPoints.size ();
    int tetNum = newPointStart == newPoints.size() ? startTet : newPoints.back().tetraNum;
    int fPn = newPointStart == newPoints.size () ? startPt : newPoints.back ().ptNum;
    int bPn = newBPoints.empty() ? endPoint : newBPoints.back ().ptNum;

    DPoint3dCR fP = m_points[fPn];
    DPoint3dCR bP = m_points[bPn];
    DVec3d sVec = DVec3d::FromStartEnd (fP, bP);
    DPoint3dCR pivot = m_points[pivotPt];
    std::vector<PointOnEdgeWithEdge> foundPoints;

    newPoints.resize (newPointsSize);
    int a = fPn;
    int ec = newPointStart == newPoints.size () ? startEC : newPoints.back ().a;
    while (true)
        {
        bool foundFace = false;
        //GetPtsForTest (pts, tetNum, b, pivotPt, ec, true);
        m_meshData.FindPointsAroundEdge (tetNum, pivotPt, a, pts);

        for (int i = 0; i < (int)pts.size (); i++)
            {
            if (pts[i].ptNum == bPn)
                return true;

            if (pts[i].ptNum == ec || IgnorePoint (pts[i].ptNum))
                continue;
            DPoint3d pt;
            if (intersect (m_points[a], pivot, m_points[pts[i].ptNum], fP, sVec, pt) && !pt.AlmostEqual (fP))
                {
                newPoints.push_back (PointOnEdgeWithEdge (a, pivotPt, pts[i]));
                ec = a;
                a = pts[i].ptNum;
                tetNum = pts[i].tetraNum;
                foundFace = true;
                break;
                }
            }
        if (!foundFace)
            return false;
        }

    return false;
    }

void TrimHull::Method5ProcessRun (TrimRun& run2, std::vector<PointOnEdgeWithEdge>& newPoints)
    {
    TrimRun run = run2;
    //std::vector<PointOnEdgeWithEdge> newPoints;
    bool isClosed = false;

    if (run.edges[0].a == run.edges.back ().b)
        isClosed = true;
    else
        isClosed = false;

    if (isClosed)
        run.edges.push_back (run.edges[0]);

    int numEdges = (int)run.edges.size () - 1;

    for (int edgeN = 0; edgeN < numEdges; edgeN++)
        {
        bool forwardScanLooped = false;
        bool doBackwardsScan = false;
        bool doEndEdgeScan = false;
        auto& edge = run.edges[edgeN];
        auto& nextEdge = run.edges[edgeN + 1];
        const int startTet = edge.tetraIndx;
        const int startPoint = edge.a;
        const int startEC = edge.c;
        const int endPoint = nextEdge.b;
        const int endEC = nextEdge.c;
        int newPointsStart = (int)newPoints.size ();
        //if (edge.a == endPoint || GetMarkedEdgeCount (edge.a, edge.b) == 2)
        //    {
        //    continue;
        //    }
        while (true)
            {
            PointOnEdgeWithEdge nextPoint;

            //if (m_points[edge.a].IsEqual (DPoint3d::From (977610.8512, 14957094.3702, 773.9800), 0.0001))
            //    forwardScanLooped = forwardScanLooped;
            ////977611.0304m, 14957094.1804m, 774.1799m
            if (Method5NextPoint (edge.a, edge.b, edge.c, edge.tetraIndx, nextPoint))
                {
                bool haveLoop = false;
                for (int j = newPointsStart; j < (int)newPoints.size (); j++)
                    {
                    if (newPoints[j].a == nextPoint.ptNum)
                        {
                        haveLoop = true;
                        break;
                        }
                    }
                if (haveLoop) // Have look need to process this differently
                    {
                    forwardScanLooped = true;
                    doBackwardsScan = true;
                    break;
                    }
                if (IsFaceMarked (nextPoint.tetraNum, nextPoint.face))
                    {
                    nextPoint.ignore = true;
#ifdef TESTME
                    bool found = false;
                    for (auto& a : newPoints)
                        {
                        if (a.tetraNum == nextPoint.tetraNum && a.face == nextPoint.face)
                            if (!a.ignore)
                                {
                                found = true;
                                break;
                                }
                        }
                    int adjTn = m_tetrahedrons[nextPoint.tetraNum].GetAdjentTet (nextPoint.face);
                    if (adjTn != -1)
                        {
                        int adjFacet = m_tetrahedrons[adjTn].GetFaceSide (nextPoint.tetraNum);

                        for (auto& a : newPoints)
                            {
                            if (a.tetraNum == adjTn && a.face == adjFacet)
                                if (!a.ignore)
                                    {
                                    found = true;
                                    break;
                                    }
                            }
                        }
                    if (!found)
                        found = true;
#endif
                    //    doEndEdgeScan = true;
                    //    break;
                    }

                newPoints.push_back (nextPoint);
                if (nextPoint.ptNum == endPoint)
                    break;

                edge.c = edge.a;
                edge.a = nextPoint.ptNum;
                edge.tetraIndx = nextPoint.tetraNum;
                continue;
                }
            else
                {
                doBackwardsScan = true;
                break;
                }
            }

        // If this is a closed shape, update the last edge to the right edge
        if (isClosed && edgeN == 0)
            {
            if (newPoints.size () != newPointsStart)
                {
                auto& eb = run.edges.back ();
                eb.c = run.edges.back ().b;
                eb.b = newPoints[newPointsStart].ptNum;
                eb.tetraIndx = newPoints[newPointsStart].tetraNum;
                }
            }

        if (doBackwardsScan)
            {
            numBackwardsLinked++;

            bool hasLinked = false;
            bool backwardsScanLooped = false;
            bool backwardsScanFailed = false;
            std::vector<PointOnEdgeWithEdge> newBPoints;
            TrimHull::edge bEdge (nextEdge.b, nextEdge.a, nextEdge.c, nextEdge.tetraIndx);

            while (true)
                {
                PointOnEdgeWithEdge nextPoint;
                if (Method5NextPoint (bEdge.a, bEdge.b, bEdge.c, bEdge.tetraIndx, nextPoint))
                    {
                    if (nextPoint.ptNum == startPoint)
                        {
                        newBPoints.push_back (nextPoint);
                        newPoints.resize (newPointsStart);
                        hasLinked = true;
                        }
                    else
                        {
                        for (int p = newPointsStart; p < (int)newPoints.size (); p++)
                            {
                            if (newPoints[p].ptNum == nextPoint.ptNum)
                                {
                                hasLinked = true;
                                //for (int p2 = p + 1; p2 < (int)newPoints.size (); p2++)
                                //    ClearMarkedFace (newPoints[p2].tetraNum, newPoints[p2].face);
                                newPoints.resize (p + 1);
                                newBPoints.push_back (nextPoint);
                                break;
                                }
                            }
                        }
                    if (hasLinked)
                        {
                        break;
                        }
                    bool haveLoop = false;
                    for (int j = 0; j < (int)newBPoints.size (); j++)
                        {
                        if (newBPoints[j].a == nextPoint.ptNum)
                            {
                            haveLoop = true;
                            break;
                            }
                        }
                    if (haveLoop)
                        {
                        backwardsScanLooped = true;
                        backwardsScanFailed = true;
                        break;
                        }

                    if (IsFaceMarked (nextPoint.tetraNum, nextPoint.face))
                        {
                        nextPoint.ignore = true;
                        bool found = false;
                        for (auto& a : newPoints)
                            {
                            if (a.tetraNum == nextPoint.tetraNum && a.face == nextPoint.face)
                                if (!a.ignore)
                                    {
                                    found = true;
                                    break;
                                    }
                            }
                        int adjTn = m_tetrahedrons[nextPoint.tetraNum].GetAdjentTet (nextPoint.face);
                        if (adjTn != -1)
                            {
                            int adjFacet = m_tetrahedrons[adjTn].GetFaceSide (nextPoint.tetraNum);

                            for (auto& a : newPoints)
                                {
                                if (a.tetraNum == adjTn && a.face == adjFacet)
                                    if (!a.ignore)
                                        {
                                        found = true;
                                        break;
                                        }
                                }
                            }
                        if (!found)
                            found = true;
                        //doEndEdgeScan = true;
                        //break;
                        }
                    //                            MarkFace (nextPoint.tetraNum, nextPoint.face);
                    newBPoints.push_back (nextPoint);
                    bEdge.c = bEdge.a;
                    bEdge.a = nextPoint.ptNum;
                    bEdge.tetraIndx = nextPoint.tetraNum;
                    continue;
                    }
                else
                    {
                    backwardsScanFailed = true;
                    break;
                    }
                }

            if (hasLinked || Method5FixScan (startTet, startPoint, startEC, newPoints, newPointsStart, endPoint, endEC, nextEdge.tetraIndx, newBPoints, nextEdge.a))
                {
                m_donePt[nextEdge.a] = true;
                numBackwardsLinkedSuccess++;
                forwardScanLooped = false;
                for (int i = (int)newBPoints.size () - 1; i >= 0; i--)
                    {
                    auto& value = newBPoints[i];
                    const Tetrahedron& tet = m_tetrahedrons[value.tetraNum];
                    int adjTetNum = tet.GetAdjentTet (value.face);
                    const Tetrahedron& adjTet = m_tetrahedrons[adjTetNum];
                    int adjFace = adjTet.GetFaceSide (value.tetraNum);
                    newPoints.push_back (PointOnEdgeWithEdge (value.ptNum, value.b, value.a, adjTetNum, adjFace)); // value.tetraNum, value.face));
                    }
                // Modify the next search point.
#ifdef QUICKADJUSTNEXTPT
                if (newPoints.size () != newPointsStart)
                    {
                    nextEdge.c = nextEdge.a;
                    nextEdge.a = newPoints.back ().a;
                    nextEdge.tetraIndx = newPoints.back ().tetraNum;
                    }
#endif
                }
            else
                {
                newBPoints.clear ();
                newPoints.resize (newPointsStart);
                if (backwardsScanLooped)
                    newBPoints.clear ();
                else
                    {
                    for (int i = (int)newBPoints.size () - 1; i >= 0; i--)
                        {
                        auto& value = newBPoints[i];
                        PointOnEdgeWithEdge newEdge (value.ptNum, value.b, value.a, value.tetraNum, value.face);
                        newEdge.ignore = value.ignore;
                        newPoints.push_back (newEdge);
                        }
#ifdef QUICKADJUSTNEXTPT
                    // Modify the next search point.
                    if (!newBPoints.empty ())
                        {
                        nextEdge.c = nextEdge.a;
                        nextEdge.a = newPoints.back ().a;
                        nextEdge.tetraIndx = newPoints.back ().tetraNum;
                        }
#endif
                    }
                }
            }
        else
            {
            m_donePt[nextEdge.a] = true;
            if (!doEndEdgeScan)// The scan found the edge.
                {
#ifdef QUICKADJUSTNEXTPT
                if (newPoints.size () != newPointsStart)
                    {
                    // Modify the next search point.
                    nextEdge.c = nextEdge.a;
                    nextEdge.a = newPoints.back ().a;
                    nextEdge.tetraIndx = newPoints.back ().tetraNum;
                    }
#endif
                }
            else if (forwardScanLooped)
                newPoints.resize (newPointsStart);
            }

        for (int i = newPointsStart; i < (int)newPoints.size (); i++)
            {
            if (newPoints[i].ignore)
                continue;
            if (IsFaceMarked (newPoints[i].tetraNum, newPoints[i].face))
                continue;
            if (AddFixFaceWithEdges (newPoints[i].tetraNum, newPoints[i].face))
                MarkFace (newPoints[i].tetraNum, newPoints[i].face);
            else
                newPoints[i].ignore = true;
            }
        }

    //if (!newPoints.empty ())
    //    {
    //    Method5AddRun (newPoints);
    //    }
    }

bool TrimHull::CountEdgeChains ()
    {
    int numNonManifoldEdges = 0;
    CurveVectorPtr sticks = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    for (auto& e : m_markEdges)
        {
        if (e.second.count == 1)
            {
            uint32_t* p = (uint32_t*)&e.first;
            int a = p[0];
            int b = p[1];

            sticks->push_back (
                ICurvePrimitive::CreateLine (DSegment3d::From (DPoint3d::From (a, a, a), DPoint3d::From (b, b, b))));
            }
        else if (e.second.count == 3)
            numNonManifoldEdges++;
        }
    cout << "numNonManifoldEdges " << numNonManifoldEdges << " - num single edges " << sticks->size () << endl;

    CurveVectorPtr chains = sticks->AssembleChains ();

    cout << "num Chains " << chains->size () << endl;
    return chains->size () != 1 || numNonManifoldEdges != 0;
    }

void TrimHull::Method5Start (int fixTn, int fixFace)
    {
    int prevFixFaces = (int)m_fixedFaces.size ();
    m_runs.clear ();
    const int* pI = Tetrahedron::GetDTetrahedron3dFacet (fixFace);
    const Tetrahedron& tet = m_tetrahedrons[fixTn];
    const int a = tet.ptNums[pI[0]];
    const int b = tet.ptNums[pI[1]];
    const int c = tet.ptNums[pI[2]];

    double d1 = distToSquared (m_points[a], m_points[b]);
    double d2 = distToSquared (m_points[c], m_points[b]);
    double d3 = distToSquared (m_points[a], m_points[c]);
    double d = max (d1, max (d2, d3));
    static double multi = 0.1;
    double maxD = pow (m_maxL * multi, 2);
    if (d > maxD)
        return;


    TrimRun tr;
    tr.edges.push_back (TrimHull::edge (a, b, c, fixTn));
    tr.edges.push_back (TrimHull::edge (b, c, a, fixTn));
    tr.edges.push_back (TrimHull::edge (c, a, b, fixTn));
    m_runs.push_back (tr);

    // Add the face.
    AddFixFaceWithEdges (fixTn, fixFace);
    MarkFace (fixTn, fixFace);

    static int maxRunCount = -1;
    int runCount = 0;

    // Do Method5
    while (!m_runs.empty ())
        {
        runCount++;
        if (maxRunCount == runCount)
            break;

#ifdef DEBUGMSG
        int numEdges = 0;
        for (auto& run : m_runs)
            {
            numEdges += (int)run.edges.size ();
            }
        cout << runCount << ": runs(" << m_runs.size () << ") edges(" << numEdges << ") faces (" << m_fixedFaces.size () - prevFixFaces << ")" << endl;
#endif
        prevFixFaces = (int)m_fixedFaces.size ();

        std::vector<PointOnEdgeWithEdge> newPoints;
        for (auto& run : m_runs)
            {
#ifdef DEBUGMSG2
            cout << "----Run---" << endl;
            for (auto& e : run.edges)
                {
                cout << "  " << e.a << "," << e.b << "," << e.c << endl;
                }
#endif
            Method5ProcessRun (run, newPoints);
            }

        if (prevFixFaces == (int)m_fixedFaces.size ())
            return;
        if (!newPoints.empty ()) // Can the fixedfaces be used?
            Method5AddRun (newPoints);

        m_runs.swap (m_newRuns);
        for (auto& a : m_runs)
            {
            std::reverse (a.edges.begin (), a.edges.end ());
            for (auto& e : a.edges)
                {
                std::swap (e.a, e.b);
                }
            }

        m_newRuns.clear ();
        }
    }

bool TrimHull::Method5FixMissingPoints (int ptNum)
    {
    return false;
    bvector<PointOnEdge> linkedPoints;
    m_meshData.CollectLinkedPoints (m_tetrahedronForPoints[ptNum], linkedPoints, ptNum);
    int newFace = -1;
    int newTetNum = -1;
    double newDistToFace = 0;
    // Need to collect the faces and find the nearest for the time, but need a better check (eg look at the faces that will be created and get the best delaunay)
    for (auto& point : linkedPoints)
        {
        if (IsFaceMarked (point.tetraNum, point.face))
            {
            const int* pI = Tetrahedron::GetDTetrahedron3dFacet (point.face);
            const Tetrahedron& tet = m_tetrahedrons[point.tetraNum];
            const int a = tet.ptNums[pI[0]];
            const int b = tet.ptNums[pI[1]];
            const int c = tet.ptNums[pI[2]];

            DPlane3d p = DPlane3d::From3Points (m_points[a], m_points[b], m_points[c]);
            double dist = p.Evaluate (m_points[ptNum]);
            if (newFace == -1 || dist < newDistToFace)
                {
                newTetNum = point.tetraNum;
                newFace = point.face;
                newDistToFace = dist;
                }
            }
        }

    if (newTetNum != -1)
        {
        const int* pI = Tetrahedron::GetDTetrahedron3dFacet (newFace);
        const Tetrahedron& tet = m_tetrahedrons[newTetNum];
        const int a = tet.ptNums[pI[0]];
        const int b = tet.ptNums[pI[1]];
        const int c = tet.ptNums[pI[2]];

        // Add edges
        IncrementMarkedEdgeCount (a, ptNum, newTetNum);
        IncrementMarkedEdgeCount (b, ptNum, newTetNum);
        IncrementMarkedEdgeCount (c, ptNum, newTetNum);
        RemoveFixedFace (newTetNum, newFace);
        ClearMarkedFace (newTetNum, newFace);
        for (int f = 0; f < 4; f++)
            {
            if (f == newFace)
                continue;
            AddFixedFace (newTetNum, f);
            MarkFace (newTetNum, f);
            }
        m_hasPt[ptNum] = true;
        return true;
        }

    return false;
    }

void TrimHull::Method5 ()
    {
    int fixTn = -1;
    int fixFace = 0;
    if (useMatchTrimming == 0)
        {
        while (fixTn == -1)
            {
            std::vector<edge> edges;
            bvector<int> addPts;
            FindPointToScan (addPts, 0);

            if (addPts.empty ())
                break;
            FindEdgeFromPoint (edges, addPts);

            for (auto edge : edges)
                {
                //TrimRun tr;

                bvector<PointOnEdge> pts;
                m_meshData.FindPointsAroundEdge (edge.tetraIndx, edge.a, edge.b, pts);
                int ptIndex = getIndexViaCC2 (pts, edge.a, edge.b, edge.c);
                if (ptIndex == -2)
                    break;

                //const int c = pts[ptIndex].ptNum;
                const int tetNum = pts[ptIndex].tetraNum;
                fixTn = tetNum;
                fixFace = pts[ptIndex].face;
                //tr.edges.push_back (TrimHull::edge (edge.a, edge.b, c, tetNum));
                //tr.edges.push_back (TrimHull::edge (edge.b, c, edge.a, tetNum));
                //tr.edges.push_back (TrimHull::edge (c, edge.a, edge.b, tetNum));
                //m_runs.push_back (tr);
                break;
                }
            if (fixTn == -1)
                {
                m_hasPt[addPts[0]] = true;
                continue;
                }
            }
        }
    if (useMatchTrimming == 1)
        {
        //std::vector<edge> edges;
        //FindTrianglesViaMatchTrimming (edges, true);
        //// Only do the first one.
        //for (auto edge : edges)
        //    {
        //    TrimRun tr;
        //    tr.edges.push_back (edge);
        //    m_runs.push_back (tr);
        //    break;
        //    }
        }
    if (useMatchTrimming == 2)
        {
        std::vector<edge> edges;
        FindTrianglesViaMatchTrimming (edges, false);
        for (int tn = 0; tn < (int)m_tetrahedrons.size (); tn++)
            {
            for (int f = 0; f < 3; f++)
                {
                if (IsFaceMarked (tn, f))
                    {
#ifdef TRIMCHECKER
                    if (doTRIMCHECKER)
                        {
                        const int* pI = Tetrahedron::GetDTetrahedron3dFacet (f);
                        Tetrahedron& tet = m_tetrahedrons[tn];
                        //TrimRun tr;
                        const int a = tet.ptNums[pI[0]];
                        const int b = tet.ptNums[pI[1]];
                        const int c = tet.ptNums[pI[2]];
                        if (USETRIMCHECKER && !sTrimChecker.HasFace (a, b, c))
                            continue;
                        }
#endif
                    fixTn = tn;
                    fixFace = f;
                    break;
                    }
                }
            if (!m_runs.empty ())
                break;
            }
        Reset ();
        }

    if (fixTn == -1)
        return;

    int numSinglePointsFixed = 0;
    while (fixTn != -1)
        {
        int previousFixedFacetSize = (int)m_fixedFaces.size ();
        Method5Start (fixTn, fixFace);

        //if (!m_fixedFaces.empty ())
        //    break;

        if (previousFixedFacetSize + 10 >= (int)m_fixedFaces.size ())// was 2
            {
            for (int i = (int)m_fixedFaces.size () - 1; i >= previousFixedFacetSize; i--)
                {
                const int f = m_fixedFaces[i].face;
                const int tn = m_fixedFaces[i].tetNum;
                const int* pI = Tetrahedron::GetDTetrahedron3dFacet (f);
                const Tetrahedron& tet = m_tetrahedrons[tn];
                const int a = tet.ptNums[pI[0]];
                const int b = tet.ptNums[pI[1]];
                const int c = tet.ptNums[pI[2]];

                DecrementMarkedEdgeCount (a, b);
                DecrementMarkedEdgeCount (b, c);
                DecrementMarkedEdgeCount (c, a);
                // Add edges
                RemoveFixedFace (tn, f);
                ClearMarkedFace (tn, f);
                }
            }
        // Todo. Method5CheckMergeIntoMesh (prevFixFaces)
        fixTn = -1;

        //for (int ptNum = 0; (int)ptNum < m_meshData.m_ignorePtsAfterNum; ptNum++)
        //    {
        //    if (!m_hasPt[ptNum] && Method5FixMissingPoints (ptNum))
        //        numSinglePointsFixed++;
        //    }

        // Find next point.
        while (fixTn == -1)
            {
            std::vector<edge> edges;
            bvector<int> addPts;
            FindPointToScan (addPts, 0);

            if (addPts.empty ())
                break;

            FindEdgeFromPoint (edges, addPts);
            m_hasPt[addPts[0]] = true;

            for (auto edge : edges)
                {
                bvector<PointOnEdge> pts;
                m_meshData.FindPointsAroundEdge (edge.tetraIndx, edge.a, edge.b, pts);
                int ptIndex = getIndexViaCC2 (pts, edge.a, edge.b, edge.c);
                if (ptIndex == -2)
                    {
                    if (Method5FixMissingPoints (addPts[0]))
                        numSinglePointsFixed++;
                    break;
                    }

                const int c = pts[ptIndex].ptNum;
                const int tetNum = pts[ptIndex].tetraNum;
                fixTn = tetNum;
                fixFace = pts[ptIndex].face;
                break;
                }
            }
        }

    //if (m_trimFromEdge) // && m_checkMarkedEdges)
    //    TrimFromEdge ();

    m_donePt.clear ();
    m_donePt.resize (m_points.size (), false);
#ifdef TRIMCHECKER
    if (doTRIMCHECKER)
        ValidateWithTrimChecker ();
#endif
#ifdef DEBUGMSG
    cout << "numSinglePointsFixed " << numSinglePointsFixed << endl;
    cout << "Backwards Link stats " << numBackwardsLinked << "-" << numBackwardsLinkedSuccess << endl;
#endif
    }


void TrimHull::UmbrellaFiltering()
    {
    exactinit(); // enable predicates

    //WritePoints("C://Users//Richard.Bois//Documents//ConceptStation//ScalableMesh//Points.xyz");
    //WriteTetrahedronsWithPointIndices("C://Users//Richard.Bois//Documents//ConceptStation//ScalableMesh//Tetrahedrons.index");

    map<int, vector<int>> pointToFaces;
    map<int, vector<Tetrahedron>> pointToTetrahedrals;
    map<int, double> pointToAverageEdgeLength;
    vector<UmbrellaFaceInfo> faces;

    // Compute lambda-intervals for faces
    for (int tet = 0; tet < (int)m_tetrahedrons.size(); tet++)
        {
        Tetrahedron& currentTet = m_tetrahedrons[tet];
        int pt = 0;
        for (; pt < 4; pt++)
            {
            pointToTetrahedrals[currentTet.ptNums[pt]].push_back(currentTet);
            if (IgnorePoint(currentTet.ptNums[pt])) break;
            }
        if (pt < 4) continue;

        for (int f = 0; f < 4; f++)
            {
            if (IsFaceMarked(tet, f)) continue;
            // Compute diameters of the face and adjacent tets
            // diam(f or tet) = smallest circumscribed ball.

            // Face
            int facePts[3];
            currentTet.GetFacePoints(f, facePts);

            //if (!(facePts[0] == 3581 && facePts[1] == 3968 && facePts[2] == 3590 ||
            //    facePts[0] == 3581 && facePts[2] == 3968 && facePts[1] == 3590 ||
            //    facePts[1] == 3581 && facePts[0] == 3968 && facePts[2] == 3590 ||
            //    facePts[1] == 3581 && facePts[2] == 3968 && facePts[0] == 3590 ||
            //    facePts[2] == 3581 && facePts[0] == 3968 && facePts[1] == 3590 ||
            //    facePts[2] == 3581 && facePts[1] == 3968 && facePts[0] == 3590)) continue;

            DTetrahedron3d tet3D1;
            currentTet.GetDTetrahedron3d(tet3D1, m_points);

            DPoint3d centerFace;
            double radiusFace;
            tet3D1.GetCircumCircle(f, centerFace, radiusFace);

            // Tet 1 (current tet)
            DPoint3d center1;
            double radius1;
            tet3D1.GetCircumsphere(center1, radius1);

            bool is1Left = tet3D1.isLeft(f, center1);

            // Tet 2 (find tet adjacent to face)
            const int* pI = Tetrahedron::GetDTetrahedron3dFacet(f);
            auto ptIndex = 6 - (pI[0] + pI[1] + pI[2]);
            auto adjId = currentTet.GetAdjentTet(ptIndex);
            int adjFacet = m_tetrahedrons[adjId].GetFaceSide(tet);


            bool is2Left = false;
            DPoint3d center2;
            double radius2;
            if (adjId > 0)
                { // adjacent tet found
                auto adjTet = m_tetrahedrons[adjId];
                DTetrahedron3d tet3D2;
                adjTet.GetDTetrahedron3d(tet3D2, m_points);

                tet3D2.GetCircumsphere(center2, radius2);

                is2Left = tet3D1.isLeft(f, center2);
                }
            else radius2 = -1; // adjacent tet not found

            double lambda1 = radiusFace / radius1;
            double lambda2 = radius2 > 0 ? radiusFace / radius2 : 0;

            double lower_bound = min(lambda1, lambda2);
            double upper_bound = max(lambda1, lambda2);
            if (radius2 > 0 && is1Left != is2Left)
                {
                upper_bound = 1;
                }

            UmbrellaFaceInfo face;
            face.points[0] = facePts[0];
            face.points[1] = facePts[1];
            face.points[2] = facePts[2];
            face.tetrahedron = tet;
            face.id = f;
            face.lambda_lower = lower_bound;
            face.lambda_upper = upper_bound;
            DPoint3d center;
            double radius = Circumcircle(m_points[face.points[0]], m_points[face.points[1]], m_points[face.points[2]], center);
            //GtTriangleQuality edgeDistHelper(m_points[face->points[0]], m_points[face->points[1]], DPoint3d(), false, m_maxL, DVec3d());
            //double tQuality = edgeDistHelper.CalcQuality(m_points[face->points[1]]);
            double tQuality = -1.0;
            jmdlMTGSwap_quadraticXYZAspectRatio(&tQuality, &m_points[face.points[0]], &m_points[face.points[1]], &m_points[face.points[2]], m_maxL2);
            face.quality = tQuality;
            faces.push_back(face);
            int faceIndex = (int)faces.size() - 1;
            pointToFaces[facePts[0]].push_back(faceIndex);
            pointToFaces[facePts[1]].push_back(faceIndex);
            pointToFaces[facePts[2]].push_back(faceIndex);
            //currentTet.SetLambdaInterval(f, lower_bound, upper_bound);
            //if (radius2 > 0) m_tetrahedrons[adjId].SetLambdaInterval(adjFacet, lower_bound, upper_bound);
            MarkFace(tet, f);
            }

        }

//#define USE_UMBRELLA_FACE_MATCHING
#ifdef USE_UMBRELLA_FACE_MATCHING
    umbrella_face_matching(pointToFaces, faces);
#else

    map<int, Umbrella> ptToUmbrella;
    for (int pt = 0; pt < (int)m_points.size(); pt++)
        {
        if (IgnorePoint(pt)) continue;
        //if (pt != 7661) continue;

        //if (m_points[pt].x>-4.4316 && m_points[pt].x<-4.4314 && m_points[pt].y>-2.2839 && m_points[pt].y<-2.2837 && m_points[pt].z>13.7304 && m_points[pt].z<13.7306)
        //    {
        //    int j = 0;
        //    ++j;
        //    }
        
        // Compute Gabriel Simplices and set of points around pt
        bool facesContainPoint = false;
        std::priority_queue<UmbrellaFaceInfo*, bvector<UmbrellaFaceInfo*>, compareUmbrellaFaces> gabrielSimplices;
        for (int faceIndex : pointToFaces[pt])
            {
            auto& face = faces[faceIndex];
            //if (face.points[0] == 12657 || face.points[1] == 12657 || face.points[2] == 12657) facesContainPoint = true;
            if (face.lambda_upper == 1.0 /*&& face.lambda_lower < 0.9*/) {
                gabrielSimplices.push(&face);
                //face.keep = true;
                }
            }

        if (gabrielSimplices.empty()) continue;

        // Initialize umbrella
        // Construct graph with cycle detector
        MTGLoopDetector ld;

        std::pair<std::set<int>::iterator, bool> ret;

        set<int> edgePoints;
        // Compute Chosen Simplices from Gabriel Simplices
        bvector<UmbrellaFaceInfo*> chosenSimplices;
        bool umbrellaFound = false;
        while (!umbrellaFound)
            {
            UmbrellaFaceInfo* face = gabrielSimplices.top();
            chosenSimplices.push_back(face);
            std::vector<int> edge;
            for (int i = 0; i < 3; i++)
                {
                if (face->points[i] != pt) edge.push_back(face->points[i]);
                }
            edgePoints.insert(edge[0]);
            edgePoints.insert(edge[1]);
            if (ld.AddEdgeAndTestForCycle(edge[0], edge[1], ptToUmbrella[pt])) umbrellaFound = true;
            gabrielSimplices.pop();
            if (gabrielSimplices.empty())
                {
                //ld.Clear();
                //chosenSimplices.clear();
                for (int faceIndex : pointToFaces[pt])
                    {
                    auto& face = faces[faceIndex];
                    //for (int i = 0; i < 3; i++) if (face.points[i] != pt) pointSet.insert(face.points[i]);
                    //if (face.points[0] == 12657 || face.points[1] == 12657 || face.points[2] == 12657) facesContainPoint = true;
                    //if (face.lambda_upper == 1.0 /*&& face.lambda_lower < 0.9*/) {
                    gabrielSimplices.push(&face);
                    //face.keep = true;
                    //  }
                    }
                }
            }

        double& pa = pointToAverageEdgeLength[pt];
        pa = 0.0;
        for (auto point : edgePoints)
            {
            DPoint3d U;
            bsiDPoint3d_subtractDPoint3dDPoint3d(&U, &m_points[point], &m_points[pt]);
            double length = sqrt(U.DotProduct(U));
            pa += length;
            }
        pointToAverageEdgeLength[pt] /= edgePoints.size();
        // Add Simplices from the Chosen Simplices that is in the set of simplices that forms the umbrella
        for (auto& face : chosenSimplices)
            {
            std::vector<int> edge;
            int ptFaceIndex = 0;
            for (int i = 0; i < 3; i++)
                {
                if (face->points[i] != pt) edge.push_back(face->points[i]);
                else ptFaceIndex = i;
                }
            if (ptToUmbrella[pt].containsEdge(edge[0], edge[1])) face->keep = true;

            //if (face->quality < 0.3)
            //    {
            //    //double tQuality = -1.0;
            //    //jmdlMTGSwap_quadraticXYZAspectRatio(&tQuality, &m_points[face->points[0]], &m_points[face->points[1]], &m_points[face->points[2]], m_maxL2);
            //    face->keep = false;
            //    }
            }

        //for (auto& face : chosenSimplices)
        //    if (face->keep) AddFixedFace(face->tetrahedron, face->id);

        }

    // Remove bad quality triangles
    for (int pt = 0; pt < (int)m_points.size(); pt++)
        {
        //break;
        if (IgnorePoint(pt)) continue;
        //if (pt != 8472) continue;

        if (m_points[pt].x>456912.4848 && m_points[pt].x<456912.4850 && m_points[pt].y>339438.4139 && m_points[pt].y<339438.4141 && m_points[pt].z>65.4413 && m_points[pt].z<65.4415)
            {
            int j = 0;
            ++j;
            }

        bvector<UmbrellaFaceInfo*> chosenSimplices;
        for (int faceIndex : pointToFaces[pt])
            {
            auto& face = faces[faceIndex];
            //for (int i = 0; i < 3; i++) if (face.points[i] != pt) pointSet.insert(face.points[i]);
            if (face.keep) chosenSimplices.push_back(&face);
            }
        int numFace = (int)pointToFaces[pt].size();
        int numKeptFaces = (int)chosenSimplices.size();
        for (auto& face : chosenSimplices)
            {
            auto& pa = pointToAverageEdgeLength[pt];
            if (pa)
                {
                std::vector<int> edge;
                for (int i = 0; i < 3; i++)
                    {
                    if (face->points[i] != pt) edge.push_back(face->points[i]);
                    }
                auto& pae0 = pointToAverageEdgeLength[edge[0]];
                auto& pae1 = pointToAverageEdgeLength[edge[1]];
                DPoint3d U;
                bsiDPoint3d_subtractDPoint3dDPoint3d(&U, &m_points[edge[0]], &m_points[pt]);
                double length = sqrt(U.DotProduct(U));
                //if (length > (pa+pae0)) face->keep = false;
                if (length > 2.0 * pae0 || length > 2.0 * pae1) face->keep = false;
                bsiDPoint3d_subtractDPoint3dDPoint3d(&U, &m_points[edge[1]], &m_points[pt]);
                length = sqrt(U.DotProduct(U));
                //if (length > (pa + pae1)) face->keep = false;
                if (length > 2.0 * pae0 || length > 2.0 * pae1) face->keep = false;
                }
            //DPoint3d center;
            //double radius = Circumcircle(m_points[face->points[0]], m_points[face->points[1]], m_points[face->points[2]], center);
            ////GtTriangleQuality edgeDistHelper(m_points[face->points[0]], m_points[face->points[1]], DPoint3d(), false, m_maxL, DVec3d());
            ////double tQuality = edgeDistHelper.CalcQuality(m_points[face->points[1]]);
            //double tQuality = -1.0;
            //jmdlMTGSwap_quadraticXYZAspectRatio(&tQuality, &m_points[face->points[0]], &m_points[face->points[1]], &m_points[face->points[2]], m_maxL2);
            //if (tQuality < 0.4 || radius > m_maxL)
            //    {
            //    face->keep = false;
            //    }
            }
        }

    // Topological cleanup (type 2)
    for (int pt = 0; pt < (int)m_points.size(); pt++)
        {
        break;
        if (IgnorePoint(pt)) continue;
        //if (pt != 12657) continue;
        //if (pt != 5229) continue;
        //if (!(pt == 5229 || pt == 5190)) continue;

        //if (!(pt == 7661 || pt == 7938)) continue;
        //if (pt != 7661 ) continue;
        //if (pt != 8425) continue;
        //if (!(pt == 12737 || pt == 8425)) continue;

        bvector<UmbrellaFaceInfo*> chosenSimplices;
        for (int faceIndex : pointToFaces[pt])
            {
            auto& face = faces[faceIndex];
            //for (int i = 0; i < 3; i++) if (face.points[i] != pt) pointSet.insert(face.points[i]);
            if (face.keep) chosenSimplices.push_back(&face);
            }
        int numFace = (int)pointToFaces[pt].size(); 
        int numKeptFaces = (int)chosenSimplices.size();
        MTGLoopDetector ld;
        bvector<int> vertexIndicesAroundLoop;
        bool hasCycle = false;
        for (auto& face : chosenSimplices)
            {
            std::vector<int> edge;
            for (int i = 0; i < 3; i++)
                {
                if (face->points[i] != pt) edge.push_back(face->points[i]);
                }
            hasCycle = ld.AddEdgeAndTestForCycle(edge[0], edge[1], vertexIndicesAroundLoop) || hasCycle;
            }
        if (hasCycle)
            {
            bool isCycleUnique = true;
            for (auto& face : chosenSimplices)
                {
                if (face->isUmbrella) continue;
                std::vector<int> edge;
                for (int i = 0; i < 3; i++)
                    {
                    if (face->points[i] != pt) edge.push_back(face->points[i]);
                    }
                if (!ld.IsEdgeInCycle(edge[0], edge[1], &isCycleUnique, vertexIndicesAroundLoop))
                    {
                    face->keep = false;
                    //face->isUmbrella = isCycleUnique;
                    }
                //else {
                //    face->isUmbrella = true;
                //    }
                }
            }
        //else {
        //    for (auto& face : chosenSimplices)
        //        {
        //        face->isUmbrella = true;
        //        face->keep = true;
        //        }
        //    }
        }

#endif
    // Add kept chosen simplices in the set of kept faces
    for (auto& face : faces)
        {
        //for (int i = 0; i < 3; i++) if (face.points[i] != pt) pointSet.insert(face.points[i]);
        //if (face.match[0].first == 3) AddFixedFace(face.tetrahedron, face.id);
        if (face.keep) AddFixedFace(face.tetrahedron, face.id);
        }
    }

    void TrimHull::umbrella_face_matching(map<int, vector<int>>& pointToFaces, vector<UmbrellaFaceInfo>& faces)
    {
    // Establish an initial umbrella for each point (starting with Gabriel simplices)
    map<int, Umbrella> ptToUmbrella;
    for (int pt = 0; pt < (int)m_points.size(); pt++)
        {
        if (IgnorePoint(pt)) continue;
        //if (!(pt == 90 || pt == 109 || pt == 120)) continue;
        //if (pt != 253) continue;
        //if (pt != 14) continue;
        //if (pt != 187) continue;

        if (m_points[pt].x>-3.5706 && m_points[pt].x<-3.5686 && m_points[pt].y>-1.7401 && m_points[pt].y<-1.7381 && m_points[pt].z>13.6123 && m_points[pt].z<13.6143)
            {
            int j = 0;
            ++j;
            }

        // Compute Gabriel Simplices around pt
        bool facesContainPoint = false;
        std::priority_queue<UmbrellaFaceInfo*, bvector<UmbrellaFaceInfo*>, compareUmbrellaFaces> gabrielSimplices;
        for (int faceIndex : pointToFaces[pt])
            {
            auto& face = faces[faceIndex];
            for (auto facePt : face.points) if (facePt == 1883) facesContainPoint = true;
            if (face.lambda_upper == 1.0)
                {
                gabrielSimplices.push(&face);
                face.keep = true;
                }
            }

        if (!facesContainPoint) continue;

        // Add edges in a cycle detector until a cycle (umbrella) is found
        MTGLoopDetector ld;
        bvector<UmbrellaFaceInfo*> chosenSimplices;
        bool umbrellaFound = false;
        while (!umbrellaFound)
            {
            UmbrellaFaceInfo* face = gabrielSimplices.top();
            chosenSimplices.push_back(face);
            std::vector<int> edge;
            for (int i = 0; i < 3; i++) if (face->points[i] != pt) edge.push_back(face->points[i]);
            if (ld.AddEdgeAndTestForCycle(edge[0], edge[1], ptToUmbrella[pt])) umbrellaFound = true;
            gabrielSimplices.pop();
            if (gabrielSimplices.empty() && !umbrellaFound)
                { // no umbrella can be found, search using all the faces from the DT to ensure an umbrella
                ld.Clear();
                chosenSimplices.clear();
                for (int faceIndex : pointToFaces[pt])
                    {
                    auto& face = faces[faceIndex];
                    gabrielSimplices.push(&face);
                    }
                }
            }

        // Add Simplices from the Chosen Simplices that is in the set of simplices that forms the umbrella
        for (auto& face : chosenSimplices)
            {
            std::vector<int> edge;
            for (int i = 0; i < 3; i++)
                {
                if (face->points[i] != pt) edge.push_back(face->points[i]);
                else face->centerPoint = i;
                }
            if (ptToUmbrella[pt].containsEdge(edge[0], edge[1]))
                {
                face->keep = true;
                }
            }
        }

    //return;
    // Compute initial matching faces
    for (auto& face : faces)
        {
        //if (!face.keep) continue;
        if (face.points[0] == 1733 || face.points[1] == 1733 || face.points[2] == 1733)
            {
            int j = 0;
            ++j;
            }
        face.evaluateFacetMatching(ptToUmbrella);
        if (face.fmatch == 3) face.keep = true;
        else face.keep = false;
        }
   // return;

    // Optimize face matches
    map<int, double> pointToAverageEdgeLength;
    bool umbrellaPass = true;
    int previousNbMatchedFaces = 0;
    while (umbrellaPass)
        {
        for (int pt = 0; pt < (int)m_points.size(); pt++)
            {
            if (IgnorePoint(pt)) continue;
            //if (!(pt == 90 || pt == 109 || pt == 120)) continue;
            //if (pt != 14) continue;

            if (m_points[pt].x > -2.4369 && m_points[pt].x<-2.4349 && m_points[pt].y>-0.8332 && m_points[pt].y<-0.8312 && m_points[pt].z>13.0774 && m_points[pt].z < 13.0794)
                {
                int j = 0;
                ++j;
                }

            // Compute candidate Simplices around pt
            bool facesContainPoint = false;
            std::priority_queue<UmbrellaFaceInfo*, bvector<UmbrellaFaceInfo*>, compareUmbrellaFaces> candidateSimplices;
            for (int faceIndex : pointToFaces[pt])
                {
                auto& face = faces[faceIndex];
                for (auto facePt : face.points) if (facePt == 1883) facesContainPoint = true;
                for (int i = 0; i < 3; i++) 
                    if (face.points[i] == pt)
                        {
                        face.centerPoint = i;
                        if (face.fmatch == 2 && face.vmatch[i] == 0)
                            {
                            // face will become fully matched if we include it
                            ++face.fmatch;
                            face.vmatch[i] = 1;
                            }
                        }
                candidateSimplices.push(&face);
                }

            if (!facesContainPoint) continue;

            // Add edges in a cycle detector until a cycle (umbrella) is found
            MTGLoopDetector ld;
            set<int> edgePoints;
            bvector<UmbrellaFaceInfo*> chosenSimplices;
            bool umbrellaFound = false;
            while (!umbrellaFound)
                {
                UmbrellaFaceInfo* face = candidateSimplices.top();
                chosenSimplices.push_back(face);
                std::vector<int> edge;
                int ptFaceIndex = -1;
                for (int i = 0; i < 3; i++)
                    {
                    if (face->points[i] != pt) edge.push_back(face->points[i]);
                    else ptFaceIndex = i;
                    }
                edgePoints.insert(edge[0]);
                edgePoints.insert(edge[1]);
                if (ld.AddEdgeAndTestForCycle(edge[0], edge[1], ptToUmbrella[pt])) umbrellaFound = true;
                if (umbrellaFound)
                    {
                    // Does the cycle contain all the constrained edges (faces that fully match)?
                    for (auto& chosen : chosenSimplices)
                        {
                        if (chosen->fmatch == 3)
                            {
                            std::vector<int> face_edge;
                            for (int i = 0; i < 3; i++) if (chosen->points[i] != pt) face_edge.push_back(chosen->points[i]);
                            if (!ptToUmbrella[pt].containsEdge(face_edge[0], face_edge[1]))
                                {
                                umbrellaFound = false;
                                ld.RemoveEdge(edge[0], edge[1]);
                                break;
                                }
                            }
                        }
                    if (!umbrellaFound)
                        {
                        for (auto& chosen : chosenSimplices)
                            {
                            std::vector<int> chosen_edge;
                            int chosenPtFaceIndex = -1;
                            for (int i = 0; i < 3; i++)
                                {
                                if (chosen->points[i] != pt) chosen_edge.push_back(chosen->points[i]);
                                else chosenPtFaceIndex = i;
                                }
                            if (ptToUmbrella[pt].containsEdge(chosen_edge[0], chosen_edge[1]))
                                {
                                if (chosen->vmatch[chosenPtFaceIndex]) --chosen->fmatch;
                                chosen->vmatch[chosenPtFaceIndex] = 0;
                                }
                            }
                        }
                    }
                candidateSimplices.pop();
                if (candidateSimplices.empty()) break;
                }

            double& pa = pointToAverageEdgeLength[pt];
            pa = 0.0;
            for (auto point : ptToUmbrella[pt])
                {
                DPoint3d U;
                bsiDPoint3d_subtractDPoint3dDPoint3d(&U, &m_points[point], &m_points[pt]);
                double length = sqrt(U.DotProduct(U));
                pa += length;
                }
            pointToAverageEdgeLength[pt] /= ptToUmbrella[pt].size();

            for (auto& face : chosenSimplices)
                {
                face->evaluateFacetMatching(ptToUmbrella);
                face->keep = face->fmatch == 3;
                }
            }

        // count nb matched faces
        int currentNbMatchedFaces = 0;
        for (auto& face : faces)
            {
            if (face.fmatch >= 3) ++currentNbMatchedFaces;
            }
        cout << currentNbMatchedFaces << endl;
        printf("nb matched faces %d", currentNbMatchedFaces);
        umbrellaPass = currentNbMatchedFaces != previousNbMatchedFaces;
        previousNbMatchedFaces = currentNbMatchedFaces;
        }

    // Remove bad quality triangles
    for (int pt = 0; pt < (int)m_points.size(); pt++)
        {
        break;
        if (IgnorePoint(pt)) continue;
        //if (pt != 8472) continue;

        if (m_points[pt].x>456912.4848 && m_points[pt].x<456912.4850 && m_points[pt].y>339438.4139 && m_points[pt].y<339438.4141 && m_points[pt].z>65.4413 && m_points[pt].z<65.4415)
            {
            int j = 0;
            ++j;
            }

        bvector<UmbrellaFaceInfo*> chosenSimplices;
        for (int faceIndex : pointToFaces[pt])
            {
            auto& face = faces[faceIndex];
            //for (int i = 0; i < 3; i++) if (face.points[i] != pt) pointSet.insert(face.points[i]);
            if (face.keep) chosenSimplices.push_back(&face);
            }
        int numFace = (int)pointToFaces[pt].size();
        int numKeptFaces = (int)chosenSimplices.size();
        for (auto& face : chosenSimplices)
            {
            auto& pa = pointToAverageEdgeLength[pt];
            if (pa)
                {
                std::vector<int> edge;
                for (int i = 0; i < 3; i++)
                    {
                    if (face->points[i] != pt) edge.push_back(face->points[i]);
                    }
                auto& pae0 = pointToAverageEdgeLength[edge[0]];
                auto& pae1 = pointToAverageEdgeLength[edge[1]];
                DPoint3d U;
                bsiDPoint3d_subtractDPoint3dDPoint3d(&U, &m_points[edge[0]], &m_points[pt]);
                double length = sqrt(U.DotProduct(U));
                //if (length > (pa+pae0)) face->keep = false;
                if (length > 2.0 * pae0 || length > 2.0 * pae1) face->keep = false;
                bsiDPoint3d_subtractDPoint3dDPoint3d(&U, &m_points[edge[1]], &m_points[pt]);
                length = sqrt(U.DotProduct(U));
                //if (length > (pa + pae1)) face->keep = false;
                if (length > 2.0 * pae0 || length > 2.0 * pae1) face->keep = false;
                }
            //DPoint3d center;
            //double radius = Circumcircle(m_points[face->points[0]], m_points[face->points[1]], m_points[face->points[2]], center);
            ////GtTriangleQuality edgeDistHelper(m_points[face->points[0]], m_points[face->points[1]], DPoint3d(), false, m_maxL, DVec3d());
            ////double tQuality = edgeDistHelper.CalcQuality(m_points[face->points[1]]);
            //double tQuality = -1.0;
            //jmdlMTGSwap_quadraticXYZAspectRatio(&tQuality, &m_points[face->points[0]], &m_points[face->points[1]], &m_points[face->points[2]], m_maxL2);
            //if (tQuality < 0.4 || radius > m_maxL)
            //    {
            //    face->keep = false;
            //    }
            }
        }

    // Topological cleanup (type 2)
    for (int pt = 0; pt < (int)m_points.size(); pt++)
        {
        break;
        if (IgnorePoint(pt)) continue;
        //if (pt != 12657) continue;
        //if (pt != 5229) continue;
        //if (!(pt == 5229 || pt == 5190)) continue;

        //if (!(pt == 7661 || pt == 7938)) continue;
        //if (pt != 7661 ) continue;
        //if (pt != 8425) continue;
        //if (!(pt == 12737 || pt == 8425)) continue;

        bvector<UmbrellaFaceInfo*> chosenSimplices;
        for (int faceIndex : pointToFaces[pt])
            {
            auto& face = faces[faceIndex];
            //for (int i = 0; i < 3; i++) if (face.points[i] != pt) pointSet.insert(face.points[i]);
            if (face.keep) chosenSimplices.push_back(&face);
            }
        int numFace = (int)pointToFaces[pt].size();
        int numKeptFaces = (int)chosenSimplices.size();
        MTGLoopDetector ld;
        bvector<int> vertexIndicesAroundLoop;
        bool hasCycle = false;
        for (auto& face : chosenSimplices)
            {
            std::vector<int> edge;
            for (int i = 0; i < 3; i++)
                {
                if (face->points[i] != pt) edge.push_back(face->points[i]);
                }
            hasCycle = ld.AddEdgeAndTestForCycle(edge[0], edge[1], vertexIndicesAroundLoop) || hasCycle;
            }
        if (hasCycle)
            {
            bool isCycleUnique = true;
            for (auto& face : chosenSimplices)
                {
                if (face->isUmbrella) continue;
                std::vector<int> edge;
                for (int i = 0; i < 3; i++)
                    {
                    if (face->points[i] != pt) edge.push_back(face->points[i]);
                    }
                if (!ld.IsEdgeInCycle(edge[0], edge[1], &isCycleUnique, vertexIndicesAroundLoop))
                    {
                    face->keep = false;
                    //face->isUmbrella = isCycleUnique;
                    }
                //else {
                //    face->isUmbrella = true;
                //    }
                }
            }
        //else {
        //    for (auto& face : chosenSimplices)
        //        {
        //        face->isUmbrella = true;
        //        face->keep = true;
        //        }
        //    }
        }
    }

void TrimHull::ValidateWithTrimChecker ()
    {
#ifndef TRIMCHECKER
    sTrimChecker.Init (m_meshData);
#endif

    int nFound = 0;
    int nExtra = 0;
    for (auto& fi : m_fixedFaces)
        {
        if (fi.tetNum == -1 || !IsFaceMarked (fi.tetNum, fi.face))
            continue;
        const int* pI = Tetrahedron::GetDTetrahedron3dFacet (fi.face);
        const Tetrahedron& tet = m_tetrahedrons[fi.tetNum];
        int a = tet.ptNums[pI[0]];
        int b = tet.ptNums[pI[1]];
        int c = tet.ptNums[pI[2]];
        if (!sTrimChecker.HasFace (a, b, c))
            {
            nExtra++;
            sTrimChecker.statsNumExtra++;
            }
        else
            nFound++;
        sTrimChecker.statsNum++;
        }
    int numRight = sTrimChecker.statsNum - sTrimChecker.statsNumExtra;
    int numMissing = sTrimChecker.NumFaces () - numRight;
    double percent = ((double)numRight * 100) / (double (sTrimChecker.statsNumExtra + sTrimChecker.NumFaces ()));
    cout << "Checker Extra (" << sTrimChecker.statsNumExtra << ") numRight (" << numRight << ") Total (" << sTrimChecker.statsNumExtra + sTrimChecker.NumFaces () << ") " << percent << "%" << endl;

#ifdef SCALABLE_MESH_ATP
    int64_t nOK = 0;
    IScalableMeshATP::GetInt (WString ("trimCheck_numGoodTriangles"), nOK);
    nOK += nFound;
    IScalableMeshATP::StoreInt (WString ("trimCheck_numGoodTriangles"), nOK);
    int64_t nChecked = 0;
    IScalableMeshATP::GetInt (WString ("trimCheck_numTrianglesChecked"), nChecked);
    nChecked += m_fixedFaces.size ();
    IScalableMeshATP::StoreInt (WString ("trimCheck_numTrianglesChecked"), nChecked);
    int64_t nWrong = 0;
    IScalableMeshATP::GetInt (WString ("trimCheck_numTrianglesWrong"), nWrong);
    nWrong += nExtra;
    IScalableMeshATP::StoreInt (WString ("trimCheck_numTrianglesWrong"), nWrong);
    int64_t nMissing = 0;
    IScalableMeshATP::GetInt (WString ("trimCheck_numTrianglesMissing"), nMissing);
    nMissing += sTrimChecker.m_faces.size () - nFound;
    IScalableMeshATP::StoreInt (WString ("trimCheck_numTrianglesMissing"), nMissing);
    IScalableMeshATP::StoreDouble (WString ("trimCheck_ratioWrongTriangles"), ((double)nWrong) / nChecked);
    IScalableMeshATP::StoreDouble (WString ("trimCheck_ratioMissingTriangles"), ((double)nMissing) / (nOK + nMissing));
    IScalableMeshATP::StoreDouble (WString ("trimCheck_quality"), ((double)nOK) / (nExtra + nOK + nMissing));
#endif
    }
void TrimHull::Reset ()
    {
    TrimTetrahedronInfo tti;
    tti.fixedFace = 0;
    m_trimInfo.clear ();

    m_trimInfo.resize (m_tetrahedrons.size (), tti);
    m_markEdges.clear ();
    m_fixedFaces.clear ();
    m_hasPt.resize (m_points.size (), false);
    m_donePt.resize (m_points.size (), false);
    if (m_infPtNum != -1)
        m_hasPt[m_infPtNum] = true;

    if (m_ignorePtsAfterNum)
        {
        for (int i = m_ignorePtsAfterNum; i <= (int)m_points.size (); i++)
            m_hasPt[i] = true;
        }

#ifdef TRIMCHECKER
    if (doTRIMCHECKER)
        sTrimChecker.Reset ();
#endif
    }

void TrimHull::TestMethod ()
    {
#ifdef TRIMCHECKER
    if (doTRIMCHECKER)
        sTrimChecker.Init (m_meshData);
#ifndef TESTVALUES

    for (const Tetrahedron& t : m_tetrahedrons)
        {
        for (int f = 0; f < numFacets; f++)
            {
            const int* pI = Tetrahedron::GetDTetrahedron3dFacet (f);
            if (sTrimChecker.HasFace (t.ptNums[pI[0]], t.ptNums[pI[1]], t.ptNums[pI[2]]))
                {
                if (t.ptNums[pI[0]] < t.ptNums[pI[1]])
                    IncrementMarkedEdgeCount (t.ptNums[pI[0]], t.ptNums[pI[1]], 0);
                if (t.ptNums[pI[1]] < t.ptNums[pI[2]])
                    IncrementMarkedEdgeCount (t.ptNums[pI[1]], t.ptNums[pI[2]], 0);
                if (t.ptNums[pI[2]] < t.ptNums[pI[0]])
                    IncrementMarkedEdgeCount (t.ptNums[pI[2]], t.ptNums[pI[0]], 0);
                }
            }
        }
    int num = sTrimChecker.countMatchedFaces ();
#ifdef DEBUGMSG
    cout << "Number of Maching Faces " << num << " " << sTrimChecker.NumFaces () << " = " << ((double)(num * 100)) / sTrimChecker.NumFaces () << "%" << endl;
    CountEdgeChains ();
    m_markEdges.clear ();
#endif
#endif
#endif
#ifdef TESTVALUES
    doTRIMCHECKER = true;
    double* qvs[] =
        {
        &qualityM_Q1,
        &qualityM_Q2,
        &qualityM_Q3,
        &qualityM_AA,
        &qualityM_AB,
        &qualityM_AC,
        &qualityM_ang,
        &qualityM_DistOverE,
        &qualityM_DCOverDC,
        &qualityM_Dist,
        &qualityM_LargestEdge,
        &qualityM_Area,
        &qualityM_penalty,
        &qualityM_AAB,
        &qualityM_circumDist,
        nullptr
        };
    int c[sizeof (qvs) / sizeof (qvs[0])][100];
    double b[sizeof (qvs) / sizeof (qvs[0])];
    int q[sizeof (qvs) / sizeof (qvs[0])];
    double bv[sizeof (qvs) / sizeof (qvs[0])];
    double pbv[sizeof (qvs) / sizeof (qvs[0])];
    double bStat = -2000;
    int bQual = 0;
    int bAvg = 0;
    const static int stepV = 5;

    qualityAvg = 0;
    qualityMethod = 1;
    for (int i = 0; qvs[i]; i++)
        {
        q[i] = minV_;
        bv[i] = -1;
        }

    //q[0] = 16;
    //q[2] = 16;
    //q[12] = 0;
    //q[6] = 19;
    //q[7] = 3;
    //q[8] = 19;
    //q[12] = 9;
    int totalCount = 0;
    int numSkipped = 0;
    for (int runProcess = 0; runProcess < 2; runProcess++)
        {
        DWORD took = GetTickCount ();
        int count = 0;
        int nextRes = totalCount / 100;
        for (int i = 0; qvs[i]; i++)
            for (int j = 0; j < 100; j++)
                c[i][j] = 0;

        //for (q1 = maxV - 1; q1 >= 0; q1 -= stepV)
        //        for (qualityMethod = 0; qualityMethod < 2; qualityMethod += 1)
            {
            //            FORQ (11)            //qualityM_Area,
            //              FORQ (5)           //qualityM_AC,
            FORQ (6)             //qualityM_ang,
                FORQ (0)                 //qualityM_Q1,
                FORQ (1)           //qualityM_Q2,
                FORQ (2)           //qualityM_Q3,
                FORQ (13) //,minV, maxV, stepV)  //qualityM_AAB,
                //              FORQ (3)           //qualityM_AA,
                //              FORQ (4)           //qualityM_AB,
                FORQ (7)           //qualityM_DistOverE,
                FORQV (8, minV_, 66, stepV)             //qualityM_DCOverDC,
                //                FORQ (9)             //qualityM_Dist,
                //                FORQ (10)            //qualityM_LargestEdge,
                //            FORQ (12)            //qualityM_penalty,
                FORQV (14, minV_, maxV, stepV)            //qualityM_cirumDist,
                {
                count++;
                if (!runProcess)
                    continue;

                if (count == nextRes)
                    {
                    nextRes += totalCount / 100;
                    double time = GetTickCount () - took;
                    double est = 0.001 * ((time / ((double)count / totalCount)) - time);
                    cout << (100 * count) / totalCount << "% " << (int)est / 60 << "m" << ((int)est) % 60 << "s " << numSkipped << "\r";
                    }
                bool skip = false;
                for (int i = 0; qvs[i]; i++)
                    {
                    *qvs[i] = CalcQuality (q[i]);
                    if (bv[i] == -2)
                        skip = true;
                    }
                if (skip)
                    {
                    numSkipped++;
                    continue;
                    }
                //*qvs[0] = 12;
                //qualityM_DistOverE = 200;
                //qualityStats = 0;

                Reset ();
                Method3 ();

                for (int i = 0; qvs[i]; i++)
                    if (qualityStats > bv[i])
                        bv[i] = qualityStats;

                if (qualityStats > bStat)
                    {
                    for (int i = 0; qvs[i]; i++)
                        b[i] = *qvs[i];
                    bQual = qualityMethod;
                    bAvg = qualityAvg;
                    bStat = qualityStats;
                    cout << "New Best " << bStat << "%";// << " " << b1 << " " << b2 << " " << b3 << " " << b4 << " " << b5 << " " << bQual << " " << bAvg << endl;
                    for (int i = 0; qvs[i]; i++)
                        cout << " " << *qvs[i];
                    cout << " " << qualityMethod << " " << qualityAvg << endl;
                    double time = GetTickCount () - took;
                    double est = 0.001 * ((time / ((double)count / totalCount)) - time);
                    cout << (100 * count) / totalCount << "% " << (int)est / 60 << "m" << ((int)est) % 60 << "s \r";

                    }
                else if (false)
                    {
                    cout << "---" << qualityStats;

                    for (int i = 0; qvs[i]; i++)
                        cout << " " << *qvs[i];
                    cout << " " << qualityMethod << " " << qualityAvg << endl;
                    }
                }
            }
            totalCount = count;
        }
    cout << "Best Result" << bStat;
    for (int i = 0; qvs[i]; i++)
        cout << " " << b[i];
    cout << " " << bQual << " " << bAvg << endl;

    for (int i = 0; qvs[i]; i++)
        {
        int m = 0;
        int mi = 0;
        for (int j = 0; j < 100; j++)
            {
            if (c[i][j] > m)
                {
                m = c[i][j];
                mi = j;
                }
            }
        cout << "q " << i << " = " << mi;
        *qvs[i] = CalcQuality (mi);
        }
    qualityStats = 0;
#endif

#ifdef T
    struct result
        {
        int umt;
        int gI;
        double qual;
        };
    bvector<result> results;

    for (useMatchTrimming = 0; useMatchTrimming < 3; useMatchTrimming++)
        {
        for (getIndexMethod = 0; getIndexMethod < 3; getIndexMethod++)
            {
            Reset ();
            Method3 ();
            result r;
            r.gI = getIndexMethod;
            r.umt = useMatchTrimming;
            r.qual = qualityStats;
            results.push_back (r);
            }
        }
    for (auto r : results)
        {
        cout << r.umt << " " << r.gI << " " << r.qual << endl;
        }
    return;
#endif
    Reset ();
#ifdef SCALABLE_MESH_ATP
    bool ATPNeedsResult = false;
    WString configStr;
    if (BSISUCCESS == ConfigurationManager::GetVariable (configStr, L"SM_ATP_MESHING_RESULT"))
        {
        ATPNeedsResult = true;
        }
    if (BSISUCCESS == ConfigurationManager::GetVariable (configStr, L"SM_BLOSSOM_MATCHING"))
        {
        useMatchTrimming = _wtoi (configStr.c_str ());
        }
    if (BSISUCCESS == ConfigurationManager::GetVariable (configStr, L"SM_INDEX_METHOD"))
        {
        getIndexMethod = _wtoi (configStr.c_str ());
        }
    if (BSISUCCESS == ConfigurationManager::GetVariable (configStr, L"SM_CIRCUMCIRCLE_2D"))
        {
        switch (_wtoi (configStr.c_str ()))
            {
            case 0:
                Circumcircle_do2d = false;
                break;
            case 1:
                Circumcircle_do2d = true;
                break;
            default:break;
            }
        }
    if (BSISUCCESS == ConfigurationManager::GetVariable(configStr, L"SM_TRIMMING_METHOD"))
        {
        TrimmingMethod = _wtoi (configStr.c_str ());
        }
#endif

    if (TrimmingMethod == 5) Method5 ();
    else if (TrimmingMethod == 7) UmbrellaFiltering ();

#ifdef DEBUGMSG
    CountEdgeChains ();
#endif
#ifdef SCALABLE_MESH_ATP
    if (ATPNeedsResult)
        ValidateWithTrimChecker ();
#endif
    }
#ifdef EGDEBUG
struct compare3D
    {
    bool operator()(const DPoint3d&a, const DPoint3d&b)
        {
        if (a.x < b.x) return true;
        else if (b.x < a.x) return false;
        else if (a.y < b.y) return true;
        else if (b.y < a.y) return false;
        else if (a.z < b.z) return true;
        else return false;
        }
    };

#endif

__forceinline  double checkNormal (DVec3dCR viewNormal, DVec3dCR normal)
    {
    return (viewNormal.x*normal.x + viewNormal.y*normal.y + viewNormal.z*normal.z);
    }

void TrimHull::WriteMeshCallBack (int (*draw) (DTMFeatureType dtmFeatureType, int numTriangles, int numMeshPts, DPoint3d *meshPtsP, DPoint3d *meshVectorsP, int numMeshFaces, long *meshFacesP, void *userP), void* userP, MTGGraph* graphP)
    {
    std::vector<int> usedPointMapper;
    std::vector<int> newPointOrder;
    usedPointMapper.resize (m_points.size (), -1);
    int numUsed = 0;
    int num = 0;

#ifndef TEST
    int numPrimaryFixed = 0;
    int numSecondaryFixed = 0;
    for (const auto& n : m_trimInfo)
        {
        short fixed = n.fixedFace;

        if ((fixed & 1) != 0)
            numPrimaryFixed++;
        if ((fixed & 2) != 0)
            numSecondaryFixed++;
        if ((fixed & 4) != 0)
            numPrimaryFixed++;
        if ((fixed & 8) != 0)
            numSecondaryFixed++;
        if ((fixed & 16) != 0)
            numPrimaryFixed++;
        if ((fixed & 32) != 0)
            numSecondaryFixed++;
        if ((fixed & 64) != 0)
            numPrimaryFixed++;
        if ((fixed & 128) != 0)
            numSecondaryFixed++;
        }
#endif

    double sumKeptEdgeLength = 0.0;
    int totalEdges = 0;

    if (m_getTrimLengthFromExtents)
        {
        totalEdges = 10000;
        sumKeptEdgeLength = DVec3d::FromStartEnd (m_meshData.m_extents.low, m_meshData.m_extents.high).Magnitude ();
        //        sumKeptEdgeLength /= sqrt (m_meshData.m_points.size()) / 5;
        //        sumKeptEdgeLength *= 100;
        static double dv = 20;
        sumKeptEdgeLength /= dv;
        sumKeptEdgeLength *= sumKeptEdgeLength;
        sumKeptEdgeLength *= totalEdges;
        }
    for (const Tetrahedron& t : m_tetrahedrons)
        {
        int tn = (int)(&t - m_tetrahedrons.data ());
        //if (m_trimInfo[tn].removed)
        //    continue;
        int largestF = -1;
        bool clearOutLargestFace = true;
        double maxEdgeLength = 0;
        if (m_removeLongFaces)
            {
            for (int f = 0; f < numFacets; f++) //remove largest facet if all have been kept
                {
                if (!IsFaceMarked (tn, f))
                    {
                    largestF = -1;
                    clearOutLargestFace = false;
                    continue;
                    }
                int facePts[3];
                t.GetFacePoints (f, facePts);
                double edgeLength = 0;
                double shortestEdge = 0;
                double longestEdge = 0;

                for (size_t pt = 0; pt < 3; pt++)
                    {
                    double length = distToSquared (m_points[facePts[pt]], m_points[facePts[(pt + 1) % 3]]);
                    edgeLength += length;
                    if (shortestEdge == 0 || shortestEdge > length)shortestEdge = length;
                    if (longestEdge < length)longestEdge = length;
                    }
                if (largestF == -1 || maxEdgeLength <= edgeLength / 3)
                    {
                    maxEdgeLength = edgeLength / 3;
                    largestF = f;
                    }
                if (m_getTrimLengthFromExtents)
                    {
                    if (longestEdge > (sumKeptEdgeLength / totalEdges))
                        {
                        ClearMarkedFace (tn, f);
                        clearOutLargestFace = false;
                        }
                    }
                else
                    {
                    //remove triangles where longest edge is significantly longer than shorter edge (degenerate), unless very small
                    // also remove triangles where shortest edge is much bigger than average
                    //NOTE: using averages here is not appropriate in datasets of high variability of density. Would work better if we used average for neighborhood or corrected
                    //for density.
                    if ((shortestEdge != 0 && longestEdge / shortestEdge > 10 && (totalEdges <= 50 || longestEdge > (sumKeptEdgeLength / totalEdges) / 2)) ||
                        (totalEdges > 3 && fabs (sumKeptEdgeLength / totalEdges - shortestEdge) / (sumKeptEdgeLength / totalEdges) > 4))
                        {
                        ClearMarkedFace (tn, f);
                        clearOutLargestFace = false;
                        }
                    }
                }
            }
        int fnfixed = 0;
        for (int f = 0; f < numFacets; f++)
            {
            if (!IsFacePrimaryMarked (tn, f))
                {
                fnfixed++;
                continue;
                }
            if (m_removeLongFaces)
                {
                if (largestF != -1 && f == largestF && clearOutLargestFace)
                    {
                    ClearMarkedFace (tn, f);
                    continue;
                    }
                }
            //  if (tetraqty2.count(tn) != 0) tetraqty2[tn]++;
            // else tetraqty2[tn] = 1;
            int facePts[3];
            t.GetFacePoints (f, facePts);

            for (int i = 0; i < numEdges; i++)
                {
                if (!m_getTrimLengthFromExtents)
                    {
                    double length = distToSquared (m_points[facePts[i]], m_points[facePts[(i + 1) % 3]]);
                    sumKeptEdgeLength += length;
                    totalEdges++;
                    }
                if (usedPointMapper[facePts[i]] == -1)
                    {
                    if (IgnorePoint (facePts[i]))
                        cout << "Added Ignored Point " << endl;
                    newPointOrder.push_back (facePts[i]);
                    usedPointMapper[facePts[i]] = numUsed + 1;
                    numUsed++;
                    }
                }
            num++;
            }
#ifdef EGEBUG
        if (fnfixed == 4)
            {
            nNotF++;

            for (int f = 0; f < numFacets; f++)
                {
                log << "TETRAHEDRON #" + std::to_string (tn) + " FACET #" + std::to_string (f) + " NOT FIXED" << endl;
                int facePts[3];
                t.GetFacePoints (f, facePts);
                if (facePts[0] < 0 || facePts[1] < 0 || facePts[2] < 0 || facePts[0] >= m_points.size () || facePts[1] >= m_points.size () || facePts[2] >= m_points.size ())continue;
                log << " POINTS : (" + std::to_string (m_points[facePts[0]].x) + "," + std::to_string (m_points[facePts[0]].y) + "," + std::to_string (m_points[facePts[0]].z)
                    + ")\n(" + std::to_string (m_points[facePts[1]].x) + "," + std::to_string (m_points[facePts[1]].y) + "," + std::to_string (m_points[facePts[1]].z)
                    + ")\n(" + std::to_string (m_points[facePts[2]].x) + "," + std::to_string (m_points[facePts[2]].y) + "," + std::to_string (m_points[facePts[2]].z)
                    + ")" << endl;
                }
            }
        else nFixed++;
#endif
        }

#ifdef OUTPUTALL
    for (size_t i = 0; i < m_points.size (); i++)
        {
        usedPointMapper[(int)i] = (int)i + 1;
        }
    numUsed = m_points.size ();
    usedPointMapper[m_infPtNum] = -1;
    for (size_t i = 0; i < m_tetrahedrons.size (); i++)
        {
        const Tetrahedron& t = m_tetrahedrons[i];
        bool isSpecial = false;
        for (int p = 0; p < 4; p++)
            if (IgnorePoint (t.ptNums[p]))
                {
                isSpecial = true;
                }

        if (!isSpecial)
            {
            for (int f = 0; f < numFacets; f++)
                MarkFace ((int)i, f);
            }
        }
#endif
    std::vector <DPoint3d> meshPts;
    std::vector <long> meshFaces;

    meshPts.reserve (newPointOrder.size ());
    for (int pt : newPointOrder)
        {
        meshPts.push_back (m_points[pt]);
        }

#ifdef USEFIXEDFACES
    DPlane3d plane;
    if (checkNormals)
        plane.InitFromArray (m_points.data (), m_meshData.m_ignorePtsAfterNum);

    int removedFaces = 0;
    int numFaces = 0;
    int numReversedFaces = 0;
    for (const FixedFaceInfo& ffi : m_fixedFaces)
        {
        if (ffi.tetNum == -1)
            continue;

        int tn = ffi.tetNum;
        const Tetrahedron& t = m_tetrahedrons[tn];
        int f = ffi.face;
#else
    for (const Tetrahedron& t : m_tetrahedrons)
        {
        int tn = (int)(&t - m_tetrahedrons.data ());
        //if (m_trimInfo[tn].removed)
        //    continue;
        for (int f = 0; f < numFacets; f++)
#endif
            {
            //if (!m_trimInfo[adjTet].removed)
            //    continue;

            if (!IsFacePrimaryMarked (tn, f))
                {
#ifdef USEFIXEDFACES
                removedFaces++;
#endif
                continue;
                }
            ClearMarkedFace (tn, f);
            // if (tetraqty.count(tn) != 0) tetraqty[tn]++;
            // else tetraqty[tn] = 1;
            int facePts[3];
            t.GetFacePoints (f, facePts);
            if (checkNormals)
                {
                DVec3d norm;

                GetPlaneNormal (&norm, &m_points[facePts[0]], &m_points[facePts[1]], &m_points[facePts[2]]);

                if (checkNormal (plane.normal, norm))
                    {
                    numFaces++;
                    }
                else
                    {
                    std::swap (facePts[0], facePts[2]);
                    numReversedFaces++;
                    }
                }

            for (int i = 0; i < 3; i++)
                {
                int mappedPtNum = usedPointMapper[facePts[i]];
                if (mappedPtNum < 0 || mappedPtNum > numUsed)
                    mappedPtNum = -1;
                BeAssert (mappedPtNum >= 1 && mappedPtNum <= numUsed);
                meshFaces.push_back (mappedPtNum);
                }

            }

        }

    cout << numFaces << " " << numReversedFaces << endl;
#ifdef USEFIXEDFACES
    for (const Tetrahedron& t : m_tetrahedrons)
        {
        int tn = (int)(&t - m_tetrahedrons.data ());
        //if (m_trimInfo[tn].removed)
        //    continue;
        for (int f = 0; f < numFacets; f++)
            {
            if (IsFacePrimaryMarked (tn, f))
                f = f;
            }
        }
#endif


#ifdef EGDEBUG
    log << " FIXED " + std::to_string (nFixed) + " out of " + std::to_string (nFixed + nNotF) + " -- " + std::to_string ((double)nFixed / (nFixed + nNotF) * 100.0) + "%" << endl;
    //for (auto it = tetraqty.begin(); it != tetraqty.end(); it++) if (it->second >= 4) log << "FACET TETRAHEDRON # " + std::to_string(it->first) + ":" + std::to_string(it->second) <<endl;
    // log << "============" << endl;
    // for (auto it = tetraqty2.begin(); it != tetraqty2.end(); it++) if (it->second >= 4) log << "FACET TETRAHEDRON # " + std::to_string(it->first) + ":" + std::to_string(it->second) << endl;
    //  log << "............................................." << endl << endl;
    log.close ();
#endif
#ifdef DEBUGMSG
    std::cout << "Wrote " << meshFaces.size () / 3 << "Triangles " << meshPts.size () << " Pts" << std::endl;
#endif

#ifdef EGDEBUG
    std::set<DPoint3d, compare3D> allPoints;
    for (DPoint3d& ptCoord : meshPts)
        {
        assert (allPoints.count (ptCoord) == 0);
        allPoints.insert (ptCoord);
        }
#endif

#ifdef DEBUG
    for (int i = 0; i < meshFaces.size (); i += 3)
        {
        assert (meshFaces[i] >0);
        assert (meshFaces[i] <= meshPts.size ());
        assert (meshFaces[i] != meshFaces[i + 1] && meshFaces[i] != meshFaces[i + 2] && meshFaces[i + 2] != meshFaces[i + 1]);
        }
#endif
    assert (meshFaces.size () % 3 == 0);

    if (graphP)
        {
        // CreateGraphFromIndexBuffer (graphP, &meshFaces[0], (int)meshFaces.size (), (int)meshPts.size ());
        // draw (DTMFeatureType::None, (int)meshFaces.size () / 3, (int)meshPts.size (), meshPts.data (), nullptr, (int)meshFaces.size (), meshFaces.data (), userP);
        }

    int size = ((int)meshFaces.size () / 3);

    draw (DTMFeatureType::None, size, (int)meshPts.size (), meshPts.data (), nullptr, (int)meshFaces.size (), meshFaces.data (), userP);
        }
//#pragma optimize("",on)

void TrimHull::WritePoints (const char* filename)
    {
    FILE* m_fp;
    fopen_s (&m_fp, filename, "wt");

    for (const DPoint3d& p : m_points)
        {
        fprintf(m_fp, "%.17g %.17g %.17g\n", p.x, p.y, p.z);
        //fwrite (&p, sizeof (p), 1, m_fp);
        }
    fclose (m_fp);
    }

void TrimHull::WriteTetrahedronsWithPointIndices(const char* filename)
    {
    FILE* m_fp;
    fopen_s(&m_fp, filename, "wt");
    int totalNumTetraHedrons = 0;
    int num = 0;
    std::vector<bool> usedPoint;

    usedPoint.resize(m_points.size(), false);
    int numUsed = 0;
    for (Tetrahedron& t : m_tetrahedrons)
        {
        int tn = (int)(&t - m_tetrahedrons.data());
        num++;
        int pts[numFacets];

        for (int i = 0; i < numFacets; i++)
            {
            if (t.ptNums[i] == -1)
                continue;
            if (!usedPoint[t.ptNums[i]])
                {
                usedPoint[t.ptNums[i]] = true;
                numUsed++;
                }
            pts[i] = t.ptNums[i];
            }

        fprintf(m_fp, "%d %d %d %d\n", pts[0], pts[1], pts[2], pts[3]);
        //fwrite(pts, sizeof(pts[0]), numFacets, m_fp);
        }
    fclose(m_fp);
    }

void TrimHull::WriteTetrahedrons (const wchar_t* filename)
    {
    FILE* m_fp;
    _wfopen_s (&m_fp, filename, L"wb");
    int totalNumTetraHedrons = 0;

#ifdef FACETS
    for (Facet f : m_facets)
        {
        DPoint3d pt[3];
        pt[0] = m_points[f.pt[0]];
        pt[1] = m_points[f.pt[1]];
        pt[2] = m_points[f.pt[2]];
        fwrite (&pt, sizeof (pt[0]), 3, m_fp);
        }
    printf ("Wrote %d Facets\n", m_facets.size ());
#else
#ifdef DEBUGMSG
    std::cout << "Stored " << m_tetrahedrons.size () << " Tetrahedrons" << std::endl;
#endif
    //        printf ("Wrote %d Tetrahedrons\n", m_totalNumTetraHedrons);


    int num = 0;
    std::vector<bool> usedPoint;

    usedPoint.resize (m_points.size (), false);
    int numUsed = 0;
    for (Tetrahedron& t : m_tetrahedrons)
        {
        int tn = (int)(&t - m_tetrahedrons.data ());
        //if (m_trimInfo[tn].removed)
        //    continue;
        //if (t.adjTet[0] == -1 || t.adjTet[1] == -1 || t.adjTet[2] == -1 || t.adjTet[3] == -1)
        //    continue;
        //if (t.ptNums[0] < numInitialBottomPoints || t.ptNums[1] < numInitialBottomPoints || t.ptNums[2] < numInitialBottomPoints || t.ptNums[3] < numInitialBottomPoints)
        //    {
        //    }
        //else
        //if (t.ptNums[0] < numInitialPoints || t.ptNums[1] < numInitialPoints || t.ptNums[2] < numInitialPoints || t.ptNums[3] < numInitialPoints)
        //    continue;

        //bool removed = false;
        //for (int i = 0; i < numFacets; i++)
        //    {
        //    DVec3d t (m_points[t.ptNums[i]], m_points[t.ptNums[(i + 1) % numFacets]]);
        //    t.z = 0;
        //    if (t.Length () > 50)
        //        {
        //        removed = true;
        //        continue;
        //        }
        //    }
        //if (removed)
        //    continue;
        num++;
        DPoint3d pts[numFacets];

        for (int i = 0; i < numFacets; i++)
            {
            if (t.ptNums[i] == -1)
                continue;
            if (!usedPoint[t.ptNums[i]])
                {
                usedPoint[t.ptNums[i]] = true;
                numUsed++;
                }
            pts[i] = m_points[t.ptNums[i]];
            }

        fwrite (pts, sizeof (pts[0]), numFacets, m_fp);
        }
    std::cout << "Stored " << num << " Tetrahedrons NumPoints" << numUsed << std::endl;
#endif

    fclose (m_fp);
    }

struct QuickPointFinder
    {
    struct HRange
        {
        DRange3d range;
        int splitIndex;
        bvector<HRange> children;
        bvector<int> points;
        HRange (DRange3dCR range, int splitIndex, bvector<int>& pts/*, int startIndex, int endIndex*/) : range (range), splitIndex (splitIndex) //, startIndex (startIndex), endIndex (endIndex)
            {
            points.swap (pts);
            }
        HRange ()
            {
            }
        };

    bvector<HRange*> m_rangesToDo;
    HRange m_range;
    DPoint3dCP m_points;
    int m_numPoints;
    int m_maxPointsInRange;
//    bvector<int> m_ptMapper;
    QuickPointFinder (DPoint3dCP points, int numPoints) : m_points (points), m_numPoints (numPoints)
        {
        bvector<int> ptMapper;
        ptMapper.resize (numPoints);
        for (int i = 0; i < numPoints; i++)
            ptMapper[i] = i;
        m_maxPointsInRange = 100;
        DRange3d range;
        range.InitFrom (points, numPoints);
        m_range = HRange (range, 0, ptMapper);
        SplitRange (&m_range, 0);
        }
    inline double& GetValue (DPoint3dCR pt, int index)
        {
        return ((double*)&pt.x)[index];
        }
    void SplitRange (HRange* range, int depth)
        {
        depth++;
        if (range->points.size () < m_maxPointsInRange)
            return;

        double splitValue = (GetValue (range->range.low, range->splitIndex) + GetValue (range->range.high, range->splitIndex)) / 2;
//        bvector<int> lowPts;
        bvector<int> highPts;
        DRange3d lowRange = DRange3d::NullRange ();
        DRange3d highRange = DRange3d::NullRange ();
        int numLowPts = 0;
        for (auto& p : range->points)
            {
            DPoint3dCR pt = m_points[p];
            if (GetValue (pt, range->splitIndex) < splitValue)
                {
                lowRange.Extend (pt);
                range->points[numLowPts++] = p;
                //lowPts.push_back (p);
                }
            else
                {
                highRange.Extend (pt);
                highPts.push_back (p);
                }
            }

        if (numLowPts != 0)
            {
            range->points.resize (numLowPts);
            //DRange3d newRange = range->range;
            //GetValue (newRange.high, range->splitIndex) = splitValue;

            range->children.push_back (HRange (lowRange, (range->splitIndex + 1) % 3, range->points));
            SplitRange (&range->children.back (), depth);
            }
        if (!highPts.empty ())
            {
            int numHighPts = (int)highPts.size ();
            //DRange3d newRange = range->range;
            //GetValue (newRange.low, range->splitIndex) = splitValue;
            range->children.push_back (HRange (highRange, (range->splitIndex + 1) % 3, highPts));
            SplitRange (&range->children.back (), depth);
            }
        range->points.clear ();
        }

    void GetPointsInRadiusHelper (HRange* range, DRange3dCR testRange, DPoint3dCR pt, double radius2, bvector<int>& pointNums)
        {
        if (range->range.low.x > testRange.high.x || range->range.high.x < testRange.low.x)
            return;
        if (range->range.low.y > testRange.high.y || range->range.high.y < testRange.low.y)
            return;
        if (range->range.low.z > testRange.high.z || range->range.high.z < testRange.low.z)
            return;

        for (auto& child : range->children)
            GetPointsInRadiusHelper (&child, testRange, pt, radius2, pointNums);

        for (auto p : range->points)
            {
            double dist = distToSquared (m_points[p], pt);
            if (dist <= radius2)
                pointNums.push_back (p);
            }
        }
    void GetPointsInRadius (DPoint3dCR pt, double radius, bvector<int>& pointNums)
        {
        DRange3d testRange = DRange3d::From (pt.x - radius, pt.y - radius, pt.z - radius, pt.x + radius, pt.y + radius, pt.z + radius);
        pointNums.clear ();
        radius *= radius;

        GetPointsInRadiusHelper (&m_range, testRange, pt, radius, pointNums);
        }
    };




    __forceinline  void NewTrim::MarkFace (int a, int b, int c)
    {
    //cout << "MF" << a << " " << b << " " << c << endl;
    uint64_t index = MarkFace::MakeIndex (a, b, c);
    m_markedfaces[index] = true;
    }

    __forceinline  bool NewTrim::IsFaceMarked (int a, int b, int c) const
    {
    uint64_t index = MarkFace::MakeIndex (a, b, c);
    auto it = m_markedfaces.find (index);
    if (it == m_markedfaces.end ())
        return false;
    return it->second;
    }

    __forceinline  MarkEdge& NewTrim::GetMarkedEdge (int a, int b)
    {
    auto mk = m_markEdges.find (MarkEdge::MakeIndex (a, b));
    if (mk != m_markEdges.end ())
        return mk->second;
    static MarkEdge me (-1, -1);
    return me;
    }

    __forceinline  int NewTrim::GetMarkedEdgeCount (int a, int b)
    {
    auto mk = m_markEdges.find (MarkEdge::MakeIndex (a, b));
    if (mk != m_markEdges.end ())
        return mk->second.count;
    return 0;
    }


    __forceinline  int NewTrim::IncrementMarkedEdgeCount (int a, int b)
    {
    uint64_t index = MarkEdge::MakeIndex (a, b);
    auto mk = m_markEdges.find (index);
    if (mk != m_markEdges.end ())
        {
        return ++mk->second.count;
        }
    m_markEdges[index] = MarkEdge (1, -1);
    return 1;
    }

    __forceinline  int NewTrim::DecrementMarkedEdgeCount (int a, int b)
    {
    uint64_t index = MarkEdge::MakeIndex (a, b);
    auto mk = m_markEdges.find (index);
    if (mk != m_markEdges.end ())
        {
        return --mk->second.count;
        }
    return -1;
    }



    __forceinline  void NewTrim::AddFixedFace (int a, int b, int c)
    {
    faces.push_back (a);
    faces.push_back (b);
    faces.push_back (c);
    }

    __forceinline  bool NewTrim::AddFixFaceWithEdges (int a, int b, int c)
    {
    if (IncrementMarkedEdgeCount (a, b) > 2)
        {
        DecrementMarkedEdgeCount (a, b);
        return false;
        }
    if (IncrementMarkedEdgeCount (b, c) > 2)
        {
        DecrementMarkedEdgeCount (a, b);
        DecrementMarkedEdgeCount (b, c);
        return false;
        }
    if (IncrementMarkedEdgeCount (c, a) > 2)
        {
        DecrementMarkedEdgeCount (a, b);
        DecrementMarkedEdgeCount (b, c);
        DecrementMarkedEdgeCount (c, a);
        return false;
        }
    m_hasPoint[a] = true;
    m_hasPoint[b] = true;
    m_hasPoint[c] = true;
    AddFixedFace (a, b, c);
    return true;
    }




NewTrim::NewTrim (DPoint3dP points, int numPoints) : m_points (points), m_numPoints (numPoints)
    {
    m_hasPoint.resize (m_numPoints, false);
    //BENTLEY_NAMESPACE_NAME::PCLUtility::INormalCalculator::InitKdTree (&handle, points, numPoints);

    m_qp = new QuickPointFinder (m_points, m_numPoints);
    }

NewTrim::~NewTrim ()
    {
    delete m_qp;
    //BENTLEY_NAMESPACE_NAME::PCLUtility::INormalCalculator::ReleaseKdTree (handle);

    }
int NewTrim::FindPoint (int ea, int eb, int ec)
    {
    static bmap<uint64_t, bool> hasDoneEdge;
    //cout << ea << " " << eb << " " << ec << endl;
    //if ((ea == 4 && eb == 10) || (ea == 4 && eb == 10))
    //    ea = ea;
    //if (hasDoneEdge[MarkEdge::MakeIndex (ea, eb)])
    //    ea = ea;
    hasDoneEdge[MarkEdge::MakeIndex (ea, eb)] = true;
    DPoint3dCR pa = m_points[ea];
    DPoint3dCR pb = m_points[eb];
    DPoint3dCR pc = m_points[ec];

    DVec3d normal;
    DVec3d Uab = DVec3d::FromStartEnd (pa, pb);
    GetPlaneNormal (&normal, &pb, &pa, &pc);
    RotMatrix rotMatrix = RotMatrix::From2Vectors (normal, Uab);

    DVec3d YVec;
    rotMatrix.getColumn (&YVec, 2);

    //bvector<int> candidates;
    //bvector<int> candidates2;
    candidates.clear ();
    double scale = 2.5;
    while (candidates.size () < 1 && scale < 4)
        {
        YVec.normalize ();
        YVec.scale (distTo (pa, pb) * 1);
        DPoint3d searchCenter = DPoint3d::From (YVec.x + ((pa.x + pb.x) / 2), YVec.y + ((pa.y + pb.y) / 2), YVec.z + ((pa.z + pb.z) / 2));
        double radiusSquared = distToSquared (searchCenter, pa);
        GetPointsIn (searchCenter, sqrt (radiusSquared), candidates2);

        candidates.clear ();
        for (auto p : candidates2)
            {
            if (p != ea && p != eb && p != ec)
                candidates.push_back (p);
            }
        scale *= 1.5;
        }
    if (candidates.empty ())
        {
        return -2;
        }
    else if (candidates.size () == 1)
        return candidates[0];

    double smallestR = 0;
    int bestI = -2;
    for (int i = 0; i < (int)candidates.size (); i++)
        {
        DPoint3dCR pt = m_points[candidates[i]];
        DPoint3d cc;
        double radius = Circumcircle (pa, pb, pt, cc);
        //double radius = Circumcircle (DPoint3d::From (pa.x, pa.y), DPoint3d::From (pb.x, pb.y), DPoint3d::From (pt.x, pt.y), cc);
        bool foundInside = false;

        for (int j = 0; j < (int)candidates.size (); j++)
            {
            if (j == i)
                continue;

            DPoint3dCR pt2 = m_points[candidates[j]];
            double dist = distToSquared (cc, pt2);
            //double dist = distToSquared (cc, DPoint3d::From (pt2.x, pt2.y));
            if (dist < radius)
                {
                foundInside = true;
                break;
                }
            }

        if (!foundInside)
            {
            if (bestI == -2 || smallestR > radius)
                {
                bestI = candidates[i];
                smallestR = radius;
                }
            //return candidates[i];
            }
        }
    return bestI;
    }

void NewTrim::Trim ()
    {
    for (int i = 0; i < m_numPoints; i++)
        if (!m_hasPoint[i])
            TrimPt (i);

    CountEdgeChains ();

#ifdef TRIMCHECKER
    if (doTRIMCHECKER)
        ValidateWithTrimChecker ();
#endif
    }

    void NewTrim::TrimPt (int p1)
        {
        int p2 = FindNearest (p1);
        if (m_hasPoint[p2])
            return;
        int p3 = -1;
        double radius = distTo (m_points[p1], m_points[p2]);
        while (p3 == -1)
            {
            radius *= 1.5;
            bvector<int> ptsInCircle;
            GetPointsIn (DPoint3d::FromSumOf (m_points[p1], 0.5, m_points[p2], 0.5), radius, ptsInCircle);
            double bestR = 0;
            for (int p : ptsInCircle)
                {
                if (p == p1 || p == p2)
                    continue;
                DPoint3d cc;
                double r = Circumcircle (m_points[p1], m_points[p2], m_points[p], cc);
                if (p3 == -1 || r < bestR)
                    {
                    p3 = p;
                    bestR = r;
                    }
                }
            }
        faces.push_back (p1);
        faces.push_back (p2);
        faces.push_back (p3);
        m_edges.push_back (Edge (p1, p2, p3));
        m_edges.push_back (Edge (p2, p3, p1));
        m_edges.push_back (Edge (p3, p1, p2));

        m_markEdges[MarkEdge::MakeIndex (p1, p2)].count++;
        m_markEdges[MarkEdge::MakeIndex (p2, p3)].count++;
        m_markEdges[MarkEdge::MakeIndex (p3, p1)].count++;
        m_hasPoint[p1] = true;
        m_hasPoint[p2] = true;
        m_hasPoint[p3] = true;
        while (!m_edges.empty ())
            {
            Edge e = m_edges.back ();
            m_edges.pop_back ();
            if (m_markEdges[MarkEdge::MakeIndex (e.ea, e.eb)].count >= 2)
                continue;
            //m_markEdges[MarkEdge::MakeIndex (e.ea, e.eb)].count++;
            int pt = FindPoint (e.ea, e.eb, e.ec);
            if (pt == -2) // || m_hasPoint[pt])
                continue;

            if (m_markEdges[MarkEdge::MakeIndex (e.ea, pt)].count > 1)
                continue;
            if (m_markEdges[MarkEdge::MakeIndex (e.eb, pt)].count > 1)
                continue;

            m_markEdges[MarkEdge::MakeIndex (e.ea, e.eb)].count++;
            m_markEdges[MarkEdge::MakeIndex (e.ea, pt)].count++;
            m_markEdges[MarkEdge::MakeIndex (e.eb, pt)].count++;

            faces.push_back (e.ea);
            faces.push_back (e.eb);
            faces.push_back (pt);
            m_edges.push_back (Edge (e.ea, pt, e.eb));
            m_edges.push_back (Edge (pt, e.eb, e.ea));
            m_hasPoint[pt] = true;
            }
        }

    void NewTrim::WriteMeshCallBack (int (*draw) (DTMFeatureType dtmFeatureType, int numTriangles, int numMeshPts, DPoint3d *meshPtsP, DPoint3d *meshVectorsP, int numMeshFaces, long *meshFacesP, void *userP), void* userP, MTGGraph* graphP)
        {
        for (int i = 0; i < (int)faces.size (); i += 3)
            {
            bool isOK = false;
            bool needToSwap = false;
            for (int j = 0; j < 3; j++)
                {
                int a = faces[i + j];
                int b = faces[i + ((j + 1) % 3)];

                auto& edge = GetMarkedEdge (a, b);

                if (edge.faceIndex[0] == -1)
                    edge.faceIndex[0] = i;
                else
                    {
                    DVec3d n1;
                    DVec3d n2;
                    int oi = edge.faceIndex[0];
                    GetPlaneNormal (&n1, &m_points[faces[oi]], &m_points[faces[oi + 1]], &m_points[faces[oi + 2]]);
                    GetPlaneNormal (&n1, &m_points[faces[i]], &m_points[faces[i + 1]], &m_points[faces[i + 2]]);
                    if (checkNormal (n1, n2))
                        isOK = true;
                    else
                        needToSwap = true;
                    }
                edge.faceIndex[1] = i;
                }
            if (needToSwap)
                {
                if (isOK)
                    isOK = false;
                std::swap (faces[i], faces[i + 1]);
                }
            }

        for (auto& face : faces)
            face++;
        draw (DTMFeatureType::None, (int)(faces.size () / 3), m_numPoints, m_points, nullptr, (int)faces.size (), (long*)faces.data (), userP);
        cout << "Wrote " << (faces.size () / 3) << " triangles" << endl;
        }
    //const int max_nn = 1000;
    void NewTrim::GetPointsIn (DPoint3dCR p, double radius, bvector<int>& ret)
        {
        //bvector<int> ret2;
        m_qp->GetPointsInRadius (p, radius, ret);
        //ret.resize (max_nn);
        //size_t size = 0;
        //Eigen::Vector3f point;
        //point[0] = p.x;
        //point[1] = p.y;
        //point[2] = p.z;
        //BENTLEY_NAMESPACE_NAME::PCLUtility::INormalCalculator::RadiusSearch (ret.data (), &size, handle, point, radius, max_nn);
        //ret.resize (size);
        }

    int NewTrim::FindNearest (int p)
        {
        double dist = -1;
        int bestI = -1;
        for (int i = 0; i < m_numPoints; i++)
            {
            if (p == i)
                continue;
            double pD = distToSquared (m_points[p], m_points[i]);
            if (bestI == -1 || pD < dist)
                {
                bestI = i;
                dist = pD;
                }
            }
        return bestI;
        }


    void NewTrim::Method5AddRun (bvector<Edge>& runPoints)
        {
        if (runPoints.empty ())
            return;
        // 1. Need to mark edges and faces.
        TrimRun newRun;

        CurveVectorPtr sticks = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
        std::vector<Edge> singleEdges;
        std::map<uint64_t, uint64_t> hasEdge;
        for (auto& pt : runPoints)
            {
            //ToDo if (pt.ignore)
            //ToDo     continue;
            int a = pt.ea;
            int b = pt.eb;
            int c = pt.ec;
            int count = GetMarkedEdgeCount (c, a);
            if (count == 1 && MarkTheEdge (hasEdge, c, a))
                {
                singleEdges.push_back (Edge (c, a, b));
                auto primitive = ICurvePrimitive::CreateLine (DSegment3d::From (DPoint3d::From (c, c, c), DPoint3d::From (a, a, a)));
                primitive->SetTag ((int)singleEdges.size () - 1);
                sticks->push_back (primitive);
                }
            else if (count == 2)
                GetMarkedEdge (c, a).done = true;

            count = GetMarkedEdgeCount (a, b);
            if (count == 1 && MarkTheEdge (hasEdge, a, b))
                {
                singleEdges.push_back (Edge (a, b, c));
                auto primitive = ICurvePrimitive::CreateLine (DSegment3d::From (DPoint3d::From (a, a, a), DPoint3d::From (b, b, b)));
                primitive->SetTag ((int)singleEdges.size () - 1);
                sticks->push_back (primitive);
                }
            else if (count == 2)
                GetMarkedEdge (a, b).done = true;

            count = GetMarkedEdgeCount (b, c);
            if (count == 1 && MarkTheEdge (hasEdge, b, c))
                {
                singleEdges.push_back (Edge (b, c, a));

                auto primitive = ICurvePrimitive::CreateLine (DSegment3d::From (DPoint3d::From (b, b, b), DPoint3d::From (c, c, c)));
                primitive->SetTag ((int)singleEdges.size () - 1);
                sticks->push_back (primitive);
                }
            else if (count == 2)
                GetMarkedEdge (b, c).done = true;
            }

        CurveVectorPtr chains = sticks->AssembleChains ();
        int numUsedLinks = 0;

        for (ICurvePrimitivePtr& chain : *chains.get ())
            {
            int prevPt = -1;
            newRun.edges.clear ();
            auto ccvP = chain->GetChildCurveVectorCP ();

            if (nullptr != ccvP)
                {
                auto ccv = *ccvP;

                for (int i = 0; i < (int)ccv.size (); i++)
                    {
                    int seI = ccv[i]->GetTag ();
                    auto line = ccv[i]->GetLineCP ();
                    bool isSwapped = false;

                    if (line->point[0].x != singleEdges[seI].ea)
                        {
                        isSwapped = true;
                        //                    std::swap (singleEdges[seI].a, singleEdges[seI].b);
                        }

                    newRun.edges.push_back (singleEdges[seI]);

                    //Test Code
                    if (prevPt != -1 && prevPt != singleEdges[seI].ea)
                        i = i;
                    if (isSwapped)
                        prevPt = singleEdges[seI].ea;
                    else
                        prevPt = singleEdges[seI].eb;
                    numUsedLinks++;
                    }
                }
            else
                {
                BeAssert (false);
                }
            if (!newRun.edges.empty ())
                m_newRuns.push_back (newRun);
            }

        if (numUsedLinks != (int)singleEdges.size ())
            numUsedLinks = numUsedLinks;
        }

#ifdef NOTYET
        bool NewTrim::Method5FixScan (int startTet, int startPt, int startEC, std::vector<PointOnEdgeWithEdge>& newPoints, int newPointStart, int endPoint, int endEC, int endTetNum, std::vector<PointOnEdgeWithEdge>& newBPoints, int pivotPt)
            {
            int bestFI = -1;
            int bestBI = -1;
            double bestD = 0;
            PointOnEdgeWithEdge bestP;
            double closestPointDist = -1;
            int closestPointF = -1;
            int closestPointB = -1;

            struct nextPts
                {
                int ptNum;
                int fI;
                int bI;
                int b;
                PointOnEdgeWithEdge a[2];
                };
            bvector<nextPts> nextPoints;
            bvector<PointOnEdge> pts;

            // Try and find the shorted valid face between pivotPoint and the points in newPoints and newBPoints.
            for (int fI = newPointStart - 1; fI < (int)newPoints.size (); fI++)
                {
                int b;
                int ec;
                int tetNum;
                // Store points.

                // Get points around edge.
                if (fI == newPointStart - 1)
                    {
                    b = startPt;
                    tetNum = startTet;
                    ec = startEC;
                    }
                else
                    {
                    tetNum = newPoints[fI].tetraNum;
                    b = newPoints[fI].ptNum;
                    ec = newPoints[fI].a;
                    }

                m_meshData.FindPointsAroundEdge (tetNum, pivotPt, b, pts);
                //GetPtsForTest (pts, tetNum, pivotPt, b, ec, true);
                for (int pI = 0; pI < (int)pts.size (); pI++)
                    {
                    if (IgnorePoint (pts[pI].ptNum))
                        continue;
                    nextPts v;
                    v.fI = fI;
                    v.b = b;
                    v.ptNum = pts[pI].ptNum;
                    v.a[0] = PointOnEdgeWithEdge (pivotPt, b, pts[pI]);
                    nextPoints.push_back (v);
                    }
                for (int bI = -1; bI < (int)newBPoints.size (); bI++)
                    {
                    int testPt;

                    if (bI == -1)
                        {
                        if (fI == newPointStart - 1)
                            continue;
                        testPt = endPoint;
                        }
                    else
                        testPt = newBPoints[bI].b;

                    double dist = distToSquared (m_points[testPt], m_points[b]);
                    if (closestPointF == -1 || dist < closestPointDist)
                        {
                        closestPointDist = dist;
                        closestPointF = fI;
                        closestPointB = bI;
                        }

                    for (int pI = 0; pI < (int)pts.size (); pI++)
                        {
                        // If point isn't Ignored add it to
                        if (pts[pI].ptNum == testPt)
                            {
                            // see if this point is in the edge list, if so get the distance.
                            double dist = distToSquared (m_points[pts[pI].ptNum], m_points[b]);

                            if (dist < m_maxL2 && (bestFI == -1 || dist < bestD))
                                {
                                bestD = dist;
                                bestFI = fI;
                                bestBI = bI;
                                bestP.a = b;
                                bestP.b = pivotPt;
                                bestP.ptNum = pts[pI].ptNum;
                                bestP.tetraNum = pts[pI].tetraNum;
                                bestP.face = pts[pI].face;
                                }
                            }
                        }
                    }
                }

            int bestNI = -2;
            double nextDist = 1e30;
            for (int bI = -1; bI < (int)newBPoints.size (); bI++)
                {
                bvector<PointOnEdge> pts;
                int b;
                int ec;
                int tetNum;
                // Store points.

                // Get points around edge.
                if (bI == -1)
                    {
                    b = endPoint;
                    ec = endEC;
                    tetNum = endTetNum;
                    }
                else
                    {
                    b = newBPoints[bI].ptNum;
                    tetNum = newBPoints[bI].tetraNum;
                    ec = newBPoints[bI].a;
                    }
                m_meshData.FindPointsAroundEdge (tetNum, pivotPt, b, pts);
                //GetPtsForTest (pts, tetNum, pivotPt, b, ec, true);

                for (int pI = 0; pI < (int)pts.size (); pI++)
                    {
                    if (IgnorePoint (pts[pI].ptNum))
                        continue;

                    for (int fI = 0; fI < (int)nextPoints.size (); fI++)
                        {
                        if (nextPoints[fI].ptNum == pts[pI].ptNum)
                            {
                            double dist = distTo (m_points[nextPoints[fI].b], m_points[pts[pI].ptNum]);
                            double dist1 = distTo (m_points[b], m_points[pts[pI].ptNum]);
                            double totalDist = pow (dist, 2) + pow (dist1, 2);

                            if (totalDist > m_maxL2)
                                continue;
                            bool useV = false;
                            if (bestNI == -1)
                                {
                                useV = true;
                                }
                            else if (nextDist < totalDist)
                                {
                                useV = true;
                                }
                            if (useV)
                                {
                                if (bI == -1 && nextPoints[fI].fI == newPointStart - 1)
                                    useV = false;
                                }
                            if (useV)
                                {
                                bestNI = fI;
                                nextPoints[fI].bI = bI;
                                nextPoints[fI].a[1] = PointOnEdgeWithEdge (pivotPt, b, pts[pI]);
                                nextDist = totalDist;
                                }
                            }
                        }
                    }
                }
            // Need to loop around the backward edges and see if any matches with a point from the previous search.
            if (bestFI != -1)
                {
                if (bestFI != -1 && bestD > nextDist)
                    bestD = bestD;
                newPoints.resize (bestFI + 1);
                newPoints.push_back (bestP);
                newBPoints.resize (bestBI + 1);
                return true;
                }
            if (bestNI != -2)
                {
                newPoints.resize (nextPoints[bestNI].fI + 1);
                newPoints.push_back (nextPoints[bestNI].a[0]);
                newBPoints.resize (nextPoints[bestNI].bI + 1);
                newBPoints.push_back (nextPoints[bestNI].a[1]);
                return true;
                }

            int newPointsSize = (int)newPoints.size ();
            int tetNum = newPointStart == newPoints.size () ? startTet : newPoints.back ().tetraNum;
            int fPn = newPointStart == newPoints.size () ? startPt : newPoints.back ().ptNum;
            int bPn = newBPoints.empty () ? endPoint : newBPoints.back ().ptNum;

            DPoint3dCR fP = m_points[fPn];
            DPoint3dCR bP = m_points[bPn];
            DVec3d sVec = DVec3d::FromStartEnd (fP, bP);
            DPoint3dCR pivot = m_points[pivotPt];
            std::vector<PointOnEdgeWithEdge> foundPoints;

            newPoints.resize (newPointsSize);
            int a = fPn;
            int ec = newPointStart == newPoints.size () ? startEC : newPoints.back ().a;
            while (true)
                {
                bool foundFace = false;
                //GetPtsForTest (pts, tetNum, b, pivotPt, ec, true);
                m_meshData.FindPointsAroundEdge (tetNum, pivotPt, a, pts);

                for (int i = 0; i < (int)pts.size (); i++)
                    {
                    if (pts[i].ptNum == bPn)
                        return true;

                    if (pts[i].ptNum == ec || IgnorePoint (pts[i].ptNum))
                        continue;
                    DPoint3d pt;
                    if (intersect (m_points[a], pivot, m_points[pts[i].ptNum], fP, sVec, pt) && !pt.AlmostEqual (fP))
                        {
                        newPoints.push_back (PointOnEdgeWithEdge (a, pivotPt, pts[i]));
                        ec = a;
                        a = pts[i].ptNum;
                        tetNum = pts[i].tetraNum;
                        foundFace = true;
                        break;
                        }
                    }
                if (!foundFace)
                    return false;
                }

            return false;
            }
#endif
        void NewTrim::Method5ProcessRun (TrimRun& run2, bvector<Edge>& newPoints)
            {
            TrimRun run = run2;
            //std::vector<PointOnEdgeWithEdge> newPoints;
            bool isClosed = false;

            if (run.edges[0].ea == run.edges.back ().eb)
                isClosed = true;
            else
                isClosed = false;

            if (isClosed)
                run.edges.push_back (run.edges[0]);

            int numEdges = (int)run.edges.size () - 1;

            for (int edgeN = 0; edgeN < numEdges; edgeN++)
                {
                bool forwardScanLooped = false;
                bool doBackwardsScan = false;
                bool doEndEdgeScan = false;
                auto& edge = run.edges[edgeN];
                auto& nextEdge = run.edges[edgeN + 1];
                const int startPoint = edge.ea;
                const int startEC = edge.ec;
                const int endPoint = nextEdge.eb;
                const int endEC = nextEdge.ec;
                int newPointsStart = (int)newPoints.size ();

                if (GetMarkedEdge (edge.ea, edge.eb).done)
                    continue;
                //if (edge.a == endPoint || GetMarkedEdgeCount (edge.a, edge.b) == 2)
                //    {
                //    continue;
                //    }
                while (true)
                    {
                    int nextPoint;

                    //if (m_points[edge.a].IsEqual (DPoint3d::From (977610.8512, 14957094.3702, 773.9800), 0.0001))
                    //    forwardScanLooped = forwardScanLooped;
                    ////977611.0304m, 14957094.1804m, 774.1799m
                    nextPoint = FindPoint (edge.ea, edge.eb, edge.ec);
                    if (nextPoint == -1)
                        IncrementMarkedEdgeCount (edge.ea, edge.eb);
                    if (nextPoint != -2)
                        {
                        bool haveLoop = false;
                        for (int j = newPointsStart; j < (int)newPoints.size (); j++)
                            {
                            if (newPoints[j].ea == nextPoint)
                                {
                                haveLoop = true;
                                break;
                                }
                            }
                        if (haveLoop) // Have look need to process this differently
                            {
                            forwardScanLooped = true;
                            doBackwardsScan = true;
                            break;
                            }
                        if (IsFaceMarked (nextPoint, edge.eb, edge.ea))
                            {
                            // ToDo nextPoint.ignore = true;
#ifdef TESTME
                            bool found = false;
                            for (auto& a : newPoints)
                                {
                                if (a.tetraNum == nextPoint.tetraNum && a.face == nextPoint.face)
                                    if (!a.ignore)
                                        {
                                        found = true;
                                        break;
                                        }
                                }
                            int adjTn = m_tetrahedrons[nextPoint.tetraNum].GetAdjentTet (nextPoint.face);
                            if (adjTn != -1)
                                {
                                int adjFacet = m_tetrahedrons[adjTn].GetFaceSide (nextPoint.tetraNum);

                                for (auto& a : newPoints)
                                    {
                                    if (a.tetraNum == adjTn && a.face == adjFacet)
                                        if (!a.ignore)
                                            {
                                            found = true;
                                            break;
                                            }
                                    }
                                }
                            if (!found)
                                found = true;
#endif
                                //doEndEdgeScan = true;
                                //break;
                            }

                        newPoints.push_back (Edge(nextPoint, edge.ea, edge.eb));  // ToDO
                        if (nextPoint == endPoint)
                            break;

                        edge.ec = edge.ea;
                        edge.ea = nextPoint;
                        continue;
                        }
                    else
                        {
                        doBackwardsScan = true;
                        break;
                        }
                    }

                // If this is a closed shape, update the last edge to the right edge
                if (isClosed && edgeN == 0)
                    {
                    if (newPoints.size () != newPointsStart)
                        {
                        auto& eb = run.edges.back ();
                        eb.ec = run.edges.back ().eb;
                        eb.eb = newPoints[newPointsStart].ea;
                        }
                    }

                if (doBackwardsScan)
                    {
                    newPoints.resize (newPointsStart);
#ifdef NOTYET
                    bvector<Edge> newBPoints;
                    numBackwardsLinked++;

                    bool hasLinked = false;
                    bool backwardsScanLooped = false;
                    bool backwardsScanFailed = false;
                    NewTrim::Edge bEdge (nextEdge.eb, nextEdge.ea, nextEdge.ec);

                    while (true)
                        {
                        int nextPoint = FindPoint (bEdge.ea, bEdge.eb, bEdge.ec);
                        if (nextPoint != -2)
                            {
                            if (nextPoint == startPoint)
                                {
                                newBPoints.push_back (Edge (nextPoint, bEdge.eb, bEdge.ea));
                                newPoints.resize (newPointsStart);
                                hasLinked = true;
                                }
                            else
                                {
                                for (int p = newPointsStart; p < (int)newPoints.size (); p++)
                                    {
                                    if (newPoints[p].ea == nextPoint)
                                        {
                                        hasLinked = true;
                                        //for (int p2 = p + 1; p2 < (int)newPoints.size (); p2++)
                                        //    ClearMarkedFace (newPoints[p2].tetraNum, newPoints[p2].face);
                                        newPoints.resize (p + 1);
                                        newBPoints.push_back (Edge (nextPoint, bEdge.eb, bEdge.ea));
                                        break;
                                        }
                                    }
                                }
                            if (hasLinked)
                                {
                                break;
                                }
                            bool haveLoop = false;
                            for (int j = 0; j < (int)newBPoints.size (); j++)
                                {
                                if (newBPoints[j].ec == nextPoint)
                                    {
                                    haveLoop = true;
                                    break;
                                    }
                                }
                            if (haveLoop)
                                {
                                backwardsScanLooped = true;
                                backwardsScanFailed = true;
                                break;
                                }

                            if (IsFaceMarked (bEdge.ea, bEdge.eb, nextPoint))
                                {
                                // ToDo nextPoint.ignore = true;
                                bool found = false;
                                for (auto& a : newPoints)
                                    {
                                    // ToDo
                                    //if (IsFaceSame (ea,eb,nextPoint, a.ea, a.eb, a.ec))
                                        //if (!a.ignore)
                                        //    {
                                        //    found = true;
                                        //    break;
                                        //    }
                                    }
                                if (!found)
                                    found = true;
                                //doEndEdgeScan = true;
                                //break;
                                }
                            //                            MarkFace (nextPoint.tetraNum, nextPoint.face);
                            newBPoints.push_back (Edge (nextPoint, bEdge.eb, bEdge.ea));
                            bEdge.ec = bEdge.ea;
                            bEdge.ea = nextPoint;
                            continue;
                            }
                        else
                            {
                            backwardsScanFailed = true;
                            break;
                            }
                        }

                    if (hasLinked) // ToDO || Method5FixScan (startTet, startPoint, startEC, newPoints, newPointsStart, endPoint, endEC, nextEdge.tetraIndx, newBPoints, nextEdge.a))
                        {
                        m_donePt[nextEdge.ea] = true;
                        numBackwardsLinkedSuccess++;
                        forwardScanLooped = false;
                        for (int i = (int)newBPoints.size () - 1; i >= 0; i--)
                            {
                            auto& value = newBPoints[i];
                            newPoints.push_back (value); // Edge (value.ptNum, value.b, value.a, adjTetNum, adjFace)); // value.tetraNum, value.face));
                            }
                        // Modify the next search point.
#ifdef QUICKADJUSTNEXTPT
                        if (newPoints.size () != newPointsStart)
                            {
                            nextEdge.c = nextEdge.a;
                            nextEdge.a = newPoints.back ().a;
                            nextEdge.tetraIndx = newPoints.back ().tetraNum;
                            }
#endif
                        }
                    else
                        {
                        newBPoints.clear ();
                        newPoints.resize (newPointsStart);
                        if (backwardsScanLooped)
                            newBPoints.clear ();
                        else
                            {
                            for (int i = (int)newBPoints.size () - 1; i >= 0; i--)
                                {
                                auto& value = newBPoints[i];
                                Edge newEdge = value; // (value.ptNum, value.b, value.a, value.tetraNum, value.face);
                                // ToDo newEdge.ignore = value.ignore;
                                newPoints.push_back (newEdge);
                                }
#ifdef QUICKADJUSTNEXTPT
                            // Modify the next search point.
                            if (!newBPoints.empty ())
                                {
                                nextEdge.c = nextEdge.a;
                                nextEdge.a = newPoints.back ().a;
                                nextEdge.tetraIndx = newPoints.back ().tetraNum;
                                }
#endif
                            }
                        }
#endif
                    }
                else
                    {
                    m_donePt[nextEdge.ea] = true;
                    if (!doEndEdgeScan)// The scan found the edge.
                        {
#ifdef QUICKADJUSTNEXTPT
                        if (newPoints.size () != newPointsStart)
                            {
                            // Modify the next search point.
                            nextEdge.c = nextEdge.a;
                            nextEdge.a = newPoints.back ().a;
                            nextEdge.tetraIndx = newPoints.back ().tetraNum;
                            }
#endif
                        }
                    else if (forwardScanLooped)
                        newPoints.resize (newPointsStart);
                    }

                for (int i = newPointsStart; i < (int)newPoints.size (); i++)
                    {
// ToDo                    if (newPoints[i].ignore)
// ToDo                        continue;
                    if (IsFaceMarked (newPoints[i].ea, newPoints[i].eb, newPoints[i].ec))
                        continue;
                    if (AddFixFaceWithEdges (newPoints[i].ea, newPoints[i].eb, newPoints[i].ec))
                        MarkFace (newPoints[i].ea, newPoints[i].eb, newPoints[i].ec);
// ToDo                    else
// ToDo                        newPoints[i].ignore = true;
                    }
                }

            //if (!newPoints.empty ())
            //    {
            //    Method5AddRun (newPoints);
            //    }
            }

            void NewTrim::Method5Start (Edge& e)
                {
                int prevFixFaces = (int)faces.size ();
                m_runs.clear ();
                const int a = e.ea;
                const int b = e.eb;
                const int c = e.ec;

                //double d1 = distToSquared (m_points[a], m_points[b]);
                //double d2 = distToSquared (m_points[c], m_points[b]);
                //double d3 = distToSquared (m_points[a], m_points[c]);
                //double d = max (d1, max (d2, d3));
                //static double multi = 0.1;
                //double maxD = pow (m_maxL * multi, 2);
                //if (d > maxD)
                //    return;

                TrimRun tr;
                tr.edges.push_back (NewTrim::Edge (a, b, c));
                tr.edges.push_back (NewTrim::Edge (b, c, a));
                tr.edges.push_back (NewTrim::Edge (c, a, b));
                m_runs.push_back (tr);

                // Add the face.
                AddFixFaceWithEdges (a, b, c);
                MarkFace (a, b, c);

                static int maxRunCount = -1;
                int runCount = 0;

                // Do Method5
                while (!m_runs.empty ())
                    {
                    runCount++;
                    if (maxRunCount == runCount)
                        break;

#ifdef DEBUGMSG
                    int numEdges = 0;
                    for (auto& run : m_runs)
                        {
                        numEdges += (int)run.edges.size ();
                        }
                    cout << runCount << ": runs(" << m_runs.size () << ") edges(" << numEdges << ") faces (" << (faces.size () - prevFixFaces) / 3 << ")" << endl;
#endif
                    prevFixFaces = (int)faces.size ();

                    bvector<Edge> newPoints;
                    for (auto& run : m_runs)
                        {
#ifdef DEBUGMSG2
                        cout << "----Run---" << endl;
                        for (auto& e : run.edges)
                            {
                            cout << "  " << e.a << "," << e.b << "," << e.c << endl;
                            }
#endif
                        Method5ProcessRun (run, newPoints);
                        }

                    if (prevFixFaces == (int)faces.size ())
                        return;
                    if (!newPoints.empty ()) // Can the fixedfaces be used?
                        Method5AddRun (newPoints);

                    m_runs.swap (m_newRuns);
                    for (auto& a : m_runs)
                        {
                        std::reverse (a.edges.begin (), a.edges.end ());
                        for (auto& e : a.edges)
                            {
                            std::swap (e.ea, e.eb);
                            }
                        }

                    m_newRuns.clear ();
                    }
                }
#ifdef NOTYET
            bool NewTrim::Method5FixMissingPoints (int ptNum)
                {
                return false;
                bvector<PointOnEdge> linkedPoints;
                m_meshData.CollectLinkedPoints (m_tetrahedronForPoints[ptNum], linkedPoints, ptNum);
                int newFace = -1;
                int newTetNum = -1;
                double newDistToFace = 0;
                // Need to collect the faces and find the nearest for the time, but need a better check (eg look at the faces that will be created and get the best delaunay)
                for (auto& point : linkedPoints)
                    {
                    if (IsFaceMarked (point.tetraNum, point.face))
                        {
                        const int* pI = Tetrahedron::GetDTetrahedron3dFacet (point.face);
                        const Tetrahedron& tet = m_tetrahedrons[point.tetraNum];
                        const int a = tet.ptNums[pI[0]];
                        const int b = tet.ptNums[pI[1]];
                        const int c = tet.ptNums[pI[2]];

                        DPlane3d p = DPlane3d::From3Points (m_points[a], m_points[b], m_points[c]);
                        double dist = p.Evaluate (m_points[ptNum]);
                        if (newFace == -1 || dist < newDistToFace)
                            {
                            newTetNum = point.tetraNum;
                            newFace = point.face;
                            newDistToFace = dist;
                            }
                        }
                    }

                if (newTetNum != -1)
                    {
                    const int* pI = Tetrahedron::GetDTetrahedron3dFacet (newFace);
                    const Tetrahedron& tet = m_tetrahedrons[newTetNum];
                    const int a = tet.ptNums[pI[0]];
                    const int b = tet.ptNums[pI[1]];
                    const int c = tet.ptNums[pI[2]];

                    // Add edges
                    IncrementMarkedEdgeCount (a, ptNum, newTetNum);
                    IncrementMarkedEdgeCount (b, ptNum, newTetNum);
                    IncrementMarkedEdgeCount (c, ptNum, newTetNum);
                    RemoveFixedFace (newTetNum, newFace);
                    ClearMarkedFace (newTetNum, newFace);
                    for (int f = 0; f < 4; f++)
                        {
                        if (f == newFace)
                            continue;
                        AddFixedFace (newTetNum, f);
                        MarkFace (newTetNum, f);
                        }
                    m_hasPt[ptNum] = true;
                    return true;
                    }

                return false;
                }
#endif
            void NewTrim::Method5 ()
                {
                Reset ();
                for (int p1 = 0; p1 < (int)m_numPoints; p1++)
                    {
                    if (!m_hasPoint[p1])
                        {
                        int p2 = FindNearest (p1);
                        if (m_hasPoint[p2])
                            continue;
                        int p3 = -1;
                        double radius = distTo (m_points[p1], m_points[p2]);
                        while (p3 == -1)
                            {
                            radius *= 1.5;
                            bvector<int> ptsInCircle;
                            GetPointsIn (DPoint3d::FromSumOf (m_points[p1], 0.5, m_points[p2], 0.5), radius, ptsInCircle);
                            double bestR = 0;
                            for (int p : ptsInCircle)
                                {
                                if (p == p1 || p == p2)
                                    continue;
                                DPoint3d cc;
                                double r = Circumcircle (m_points[p1], m_points[p2], m_points[p], cc);
                                if (p3 == -1 || r < bestR)
                                    {
                                    p3 = p;
                                    bestR = r;
                                    }
                                }
                            }
                        Edge e (p1, p2, p3);
                        Method5Start (e);
                        }
                    }
                m_donePt.clear ();
                m_donePt.resize (m_numPoints, false);

                CountEdgeChains ();
#ifdef TRIMCHECKER
                if (doTRIMCHECKER)
                    ValidateWithTrimChecker ();
#endif
#ifdef DEBUGMSG
                //cout << "numSinglePointsFixed " << numSinglePointsFixed << endl;
                cout << "Backwards Link stats " << numBackwardsLinked << "-" << numBackwardsLinkedSuccess << endl;
#endif
                }







bool NewTrim::CountEdgeChains ()
    {
    int numNonManifoldEdges = 0;
    CurveVectorPtr sticks = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    for (auto& e : m_markEdges)
        {
        if (e.second.count == 1)
            {
            uint32_t* p = (uint32_t*)&e.first;
            int a = p[0];
            int b = p[1];

            sticks->push_back (
                ICurvePrimitive::CreateLine (DSegment3d::From (DPoint3d::From (a, a, a), DPoint3d::From (b, b, b))));
            }
        else if (e.second.count == 3)
            numNonManifoldEdges++;
        }
    cout << "numNonManifoldEdges " << numNonManifoldEdges << " - num single edges " << sticks->size () << endl;

    CurveVectorPtr chains = sticks->AssembleChains ();

    cout << "num Chains " << chains->size () << endl;
    return chains->size () != 1 || numNonManifoldEdges != 0;
    }

void NewTrim::ValidateWithTrimChecker ()
    {

    sTrimChecker.Init (m_points, m_numPoints);

    int nFound = 0;
    int nExtra = 0;
    for (int i = 0; i < (int)faces.size (); i += 3)
        {
        int* p = &faces[i];
        if (!sTrimChecker.HasFace (p[0], p[1], p[2]))
            {
            nExtra++;
            sTrimChecker.statsNumExtra++;
            }
        else
            nFound++;
        sTrimChecker.statsNum++;
        }
    int numRight = sTrimChecker.statsNum - sTrimChecker.statsNumExtra;
    int numMissing = sTrimChecker.NumFaces () - numRight;
    double percent = ((double)numRight * 100) / (double (sTrimChecker.statsNumExtra + sTrimChecker.NumFaces ()));
    cout << "Checker Extra (" << sTrimChecker.statsNumExtra << ") numRight (" << numRight << ") Total (" << sTrimChecker.statsNumExtra + sTrimChecker.NumFaces () << ") " << percent << "%" << endl;

#ifdef SCALABLE_MESH_ATP
    int64_t nOK = 0;
    IScalableMeshATP::GetInt (WString ("trimCheck_numGoodTriangles"), nOK);
    nOK += nFound;
    IScalableMeshATP::StoreInt (WString ("trimCheck_numGoodTriangles"), nOK);
    int64_t nChecked = 0;
    IScalableMeshATP::GetInt (WString ("trimCheck_numTrianglesChecked"), nChecked);
    nChecked += faces.size () / 3;
    IScalableMeshATP::StoreInt (WString ("trimCheck_numTrianglesChecked"), nChecked);
    int64_t nWrong = 0;
    IScalableMeshATP::GetInt (WString ("trimCheck_numTrianglesWrong"), nWrong);
    nWrong += nExtra;
    IScalableMeshATP::StoreInt (WString ("trimCheck_numTrianglesWrong"), nWrong);
    int64_t nMissing = 0;
    IScalableMeshATP::GetInt (WString ("trimCheck_numTrianglesMissing"), nMissing);
    nMissing += sTrimChecker.m_faces.size () - nFound;
    IScalableMeshATP::StoreInt (WString ("trimCheck_numTrianglesMissing"), nMissing);
    IScalableMeshATP::StoreDouble (WString ("trimCheck_ratioWrongTriangles"), ((double)nWrong) / nChecked);
    IScalableMeshATP::StoreDouble (WString ("trimCheck_ratioMissingTriangles"), ((double)nMissing) / (nOK + nMissing));
    IScalableMeshATP::StoreDouble (WString ("trimCheck_quality"), ((double)nOK) / (nExtra + nOK + nMissing));
#endif
    }






/*

parallel_for_each (m_tetrahedrons.begin (), m_tetrahedrons.end (), [&](Tetrahedron& t)
{
t.removed = true;
if (t.ptNums[0] == -1)
return;
if (t.ptNums[0] < numInitialPoints || t.ptNums[1] < numInitialPoints || t.ptNums[2] < numInitialPoints || t.ptNums[3] < numInitialPoints)
return;
if (IsDelauny (t))
{
t.removed = false;
}
});

void RemoveAllAbovePoints ()
{
parallel_for_each (m_tetrahedrons.begin (), m_tetrahedrons.end (), [&](Tetrahedron& t)
{
static const DVec3d vec = DVec3d (0, 0, 1);
if (t.ptNums[0] == -1)
{
t.removed = true;
return;
}
t.removed = false;

DPoint3d pts[numFacets];

for (int i = 0; i < numFacets; i++)
{
if (t.ptNums[i] < numInitialBottomPoints)
{
t.removed = true;
return;
}
pts[i] = m_points[t.ptNums[i]];
}
/*DTetrahedron3d dt;
t.GetDTetrahedron3d (dt, m_points);

t.removed = true;
for (int i = 0; i < numFacets; i++)
{
DPoint3d p = dt.points[i];
p.z += 1;
if (!dt.isLeftOrOn (i, p))
{
t.removed = false;
return;
}
}
return;
int ptNum = 0;
for (DPoint3d pt : m_points)
{
for (int facet = 0; facet < numFacets; facet++)
{
int a, b, c;
Tetrahedron::GetDTetrahedron3dFacet_ (facet, a, b, c);
//if (t.ptNums[a] == ptNum || t.ptNums[b] == ptNum || t.ptNums[c] == ptNum)
//    break;
DPoint3d ipt;
if (intersect3D_RayTriangle (pt, vec, pts[a], pts[b], pts[c], ipt) == 1 && pt.z < ipt.z)
{
t.removed = true;
break;
}
//Get the facet and see if the ray intersects.
// For now as we are assuming the Vector is up, we just need the
}
if (t.ptNums[0] == -1)
break;
ptNum++;

}
}
);
}

void InitializeTetrahedronRemoved (bool removeTopPoints, bool removeBottomPoints)
{
parallel_for_each (m_tetrahedrons.begin (), m_tetrahedrons.end (), [&](Tetrahedron& t)
{
t.removed = false;
if (t.ptNums[0] == -1)
t.removed = true;
for (int i = 0; i < numFacets; i++)
{
if (t.ptNums[i] < numInitialBottomPoints)
{
if (removeBottomPoints)
{
t.removed = true;
return;
}
}
else if (t.ptNums[i] < numInitialPoints)
{
if (removeTopPoints)
{
t.removed = true;
return;
}
}
}
});
}
*/

// Find the left/right/top/bottom most point, then find the smallest faces.  Mark that face. add the 3 adject edges to stack.
// for each edge, scan round and find all triangles, find smallest triangle, mark face and then add those edges. Mark edges as done.
// Need to mark points as done.

// When we have come out find the left most point that hasn't been marked and add those.
/*
class Trimmer
{

};
*/


// ToDo move some of the helper functions into the MeshData.
// ToDo on outter trim check if a point will be removed completely if not dont trim the edge.
// ToDo Look at cleaning up non manifold faces.
// ToDo Look at recovering missing points.

/* Method 4
1. Create tetrahedron for point array.
2. For each point get all points around. ignoring the outter points.
3.   Find plane for all points.
4.   bclib triangulate for each point transformed by plane.
5.   get all points around n, and add the link to the edge.


sort the points into x,y,z order.
sort the edges into lowest point first.

Remember edges and faces but only do them one at a time, and when we have completed proceed to the next
*/

/* ---
1. Either
a. From a point get the edge (then b)
b. From an edge get a face.
c. From a face.
2. Add the edges of the face to a list.
3. Start on the first link and scan round to find the next point.
If the point isn't found then we need to see if this is correct or if we are wrong.
scan backwards from the next link to see if it links up with the previous one, if so we correct it.
carry on scanning.
4. when we get a break in the link we add these faces, and then add the new edges to the next list.
5. Move to the next list.
*/


// Add findPointsAroundEdge and other like functions to m_meshData. and make findPointsAroundEdge ignore Ignored Points.
// Add better reverse check. and look to see what happens if we include the edges from the two closest points. eg faces that go through those two points. going round the pivot point.



// ToDo

// New getEdge.
// Calc circumcenter for all points.
// compare each point with each other,
// If the circumcenter contains any other points then mark them as not used.
// sort the remaining used points into circumcenter.
// chose the smallest circumcenter (unless it is ec and then chose the second smallest)


// ToDo.
// Change to find the lowest point first.
// Make the trim slithers, also check for long edges, but will need to check not to remove any points.
// Try and favour faces that are near the ec face.
// store the angle between points, and shift them so we start on a big gap.
// FixScan to always close the mesh,


// ToDo

// Linking method 5 to the new class.
// plug in a better scan to find points within an area.
// Add trimchecker