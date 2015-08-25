#include <ScalableMeshPCH.h>

#pragma warning(disable : 4189 )
#undef static_assert
#include <ppl.h>
#include <amp.h>
//#undef round
//#include <amp_math.h>
//#include <concurrent_vector.h>
#include "predicates.h"
#include "ScalableMeshCao.h"

#define TRACE_TETRA_INIT() {}
#define TRACE_TETRA_END() {}

#ifdef DHDEBUG
//#pragma optimize( "", off )
#endif

/* splitter = 2^ceiling(p / 2) + 1.  Used to split floats in half.           */
double splitter;
double epsilon;         /* = 2^(-p).  Used to estimate roundoff errors. */
/* A set of coefficients used to calculate maximum roundoff errors.          */
double resulterrbound;
double ccwerrboundA, ccwerrboundB, ccwerrboundC;
double o3derrboundA, o3derrboundB, o3derrboundC;
double iccerrboundA, iccerrboundB, iccerrboundC;
double isperrboundA, isperrboundB, isperrboundC;
double o3derrboundlifted;

bool intersect (DPoint3dCR a, DPoint3dCR b, DPoint3dCR c, DPoint3dCR origin, DVec3dCR direction, DPoint3dR pt)
    {
    DVec3d planeNormal;
    GetPlaneNormal (&planeNormal, &a, &b, &c);
    DPlane3d plane = DPlane3d::FromOriginAndNormal (a, planeNormal);

    DRay3d ray;
    ray.origin = origin;
    ray.direction = direction;
    bsiDRay3d_intersectDPlane3d (&ray, &pt, nullptr, &plane);

    if (orient2d (a, b, pt) > 0)
        return false;

    if (orient2d (b, c, pt) > 0)
        return false;

    if (orient2d (c, a, pt) > 0)
        return false;
    return true;
    }

template <class list, class T> bool Contains (list& l, T val)
    {
    for (T v : l)
        {
        if (v == val)
            return true;
        }
    return false;
    }

int MeshData::WalkToTetrahedronWithPoint (int startTetrahedron, DPoint3dCR pt) const
    {
    int tetraIndx = startTetrahedron;
    if (tetraIndx == -1)
        tetraIndx = 0;

    while (tetraIndx != -1)
        {
        const Tetrahedron* tetP = &m_tetrahedrons[tetraIndx];
        DTetrahedron3d dtet (m_points[tetP->ptNums[0]], m_points[tetP->ptNums[1]], m_points[tetP->ptNums[2]], m_points[tetP->ptNums[3]]);
        int nextTetraIndx = tetraIndx;
        for (int f = 0; f < numFacets; f++)
            {
            if (!dtet.isLeftOrOn (f, pt))
                {
                nextTetraIndx = tetP->GetAdjentTet (f);
                break;
                }
            }
        if (nextTetraIndx == tetraIndx)
            return tetraIndx;
        tetraIndx = nextTetraIndx;
        }
    return -1;
    }

int MeshData::GetNextExitTetrahedronAlongRay (int startTetrahedron, int prevTet, DPoint3dCR startPt, DVec3dCR normal) const
    {
    int tetraIndx = startTetrahedron;
    if (tetraIndx == -1)
        tetraIndx = WalkToTetrahedronWithPoint (-1, startPt);

    const Tetrahedron* tetP = &m_tetrahedrons[tetraIndx];
    DTetrahedron3d dtet (m_points[tetP->ptNums[0]], m_points[tetP->ptNums[1]], m_points[tetP->ptNums[2]], m_points[tetP->ptNums[3]]);
    int nextTetraIndx = tetraIndx;
    double dist = 0;
    int exitTetrahedron = -2;
    for (int f = 0; f < numFacets; f++)
        {
        DPoint3d ipt;
        const int* pts = Tetrahedron::GetDTetrahedron3dFacet (f);
        if (tetP->GetAdjentTet (f) != prevTet &&
            intersect (dtet.points[pts[0]], dtet.points[pts[1]], dtet.points[pts[2]], startPt, normal, ipt))
            //                intersect3D_RayTriangle (startPt, normal, dtet.points[a], dtet.points[b], dtet.points[c], ipt) == 1)
            {
            double newDist = distToSquared (startPt, ipt);
            if (exitTetrahedron == -2 || dist < newDist)
                {
                dist = newDist;
                exitTetrahedron = tetP->GetAdjentTet (f);
                }
            }
        }
    return exitTetrahedron == -2 ? -1 : exitTetrahedron;
    }

void MeshData::GetTetrahedronsAroundPoint (int tetraIndx, int ptNum, std::vector<int>& tetrahedrons) const
    {
    std::vector<int> toBeChecked;

    if (tetraIndx == -1)
        tetraIndx = WalkToTetrahedronWithPoint (-1, m_points[ptNum]);

    toBeChecked.push_back (tetraIndx);

    while (toBeChecked.size ())
        {
        int tetraIndx = toBeChecked.back ();
        toBeChecked.pop_back ();

        if (Contains (tetrahedrons, tetraIndx))
            continue;
        tetrahedrons.push_back (tetraIndx);

        const Tetrahedron& t = m_tetrahedrons[tetraIndx];
        // Need to get the
        int face = t.GetPointIndex (ptNum);
        //int pts[numEdges];

        //t.GetFacePoints (face, pts);

        for (int f = 0; f < numFacets; f++)
            {
            if (face == f)
                continue;

            int adjTet = t.GetAdjentTet (f);
            if (adjTet != -1 && !Contains (toBeChecked, adjTet))
                toBeChecked.push_back (adjTet);
            }
        }
    }

void MeshData::CollectLinkedPoints (int tetraIndx, std::vector<int>& linkedPoints, int ptNum) const
    {
    std::vector<int> checkedTetrahedrons;
    std::vector<int> toBeChecked;
    TRACE_TETRA_INIT ();
    toBeChecked.push_back (tetraIndx);

    while (toBeChecked.size ())
        {
        int tetraIndx = toBeChecked.back ();
        toBeChecked.pop_back ();

        if (Contains (checkedTetrahedrons, tetraIndx))
            continue;
        checkedTetrahedrons.push_back (tetraIndx);

        const Tetrahedron& t = m_tetrahedrons[tetraIndx];
        // Need to get the
        int face = t.GetPointIndex (ptNum);
        int pts[numEdges];

        if (face == -1 || ptNum < 0)
            {
#ifdef LOG_TETRA
            log << "[CollectLinkedPoints] INVALID ID AT FACE " + std::to_string (face) + " FOR PT " + std::to_string (ptNum) + " OF TETRA " + std::to_string (tetraIndx) << endl;
#endif
            continue;
            }
        t.GetFacePoints (face, pts);

        for (int edge = 0; edge < numEdges; edge++)
            {
            int edgePtNum = pts[edge];
            if (edgePtNum != ptNum && !Contains (linkedPoints, edgePtNum))
                {
                linkedPoints.push_back (edgePtNum);
                }
            }
        for (int f = 0; f < numFacets; f++)
            {
            if (face == f)
                continue;

            int adjTet = t.GetAdjentTet (f);
            if (adjTet != -1 && !Contains (toBeChecked, adjTet))
                toBeChecked.push_back (adjTet);
            }
        }
    TRACE_TETRA_END ();
    }

void MeshData::CollectLinkedPoints (int tetraIndx, bvector<PointOnEdge>& linkedPoints, int ptNum) const
    {
    bvector<int> checkedTetrahedrons;
    bvector<int> toBeChecked;
    bvector<int> lnkPnts;

    TRACE_TETRA_INIT ();
    toBeChecked.push_back (tetraIndx);

    while (toBeChecked.size ())
        {
        int tetraIndx = toBeChecked.back ();
        toBeChecked.pop_back ();

        if (Contains (checkedTetrahedrons, tetraIndx))
            continue;
        checkedTetrahedrons.push_back (tetraIndx);

        const Tetrahedron& t = m_tetrahedrons[tetraIndx];
        // Need to get the
        int face = t.GetPointIndex (ptNum);
        int pts[numEdges];

        if (face == -1 || ptNum < 0)
            {
#ifdef LOG_TETRA
            log << "[CollectLinkedPoints] INVALID ID AT FACE " + std::to_string (face) + " FOR PT " + std::to_string (ptNum) + " OF TETRA " + std::to_string (tetraIndx) << endl;
#endif
            continue;
            }
        t.GetFacePoints (face, pts);

        for (int edge = 0; edge < numEdges; edge++)
            {
            int edgePtNum = pts[edge];
            if (edgePtNum != ptNum && !Contains (lnkPnts, edgePtNum))
                {
                lnkPnts.push_back (edgePtNum);
                linkedPoints.push_back (PointOnEdge (edgePtNum, tetraIndx, face));
                }
            }
        for (int f = 0; f < numFacets; f++)
            {
            if (face == f)
                continue;

            int adjTet = t.GetAdjentTet (f);
            if (adjTet != -1 && !Contains (toBeChecked, adjTet))
                toBeChecked.push_back (adjTet);
            }
        }
    TRACE_TETRA_END ();
    }

void MeshData::FindPointsAroundEdge (int tetraIndx, int ptA, int ptB, bvector<PointOnEdge>& otherPt) const
    {
    otherPt.clear ();
    if (ptA == ptB)
        return;
    //TRACE_TETRA_INIT();
#ifdef LOG_TETRA
    std::string fPtsLog;
#endif
    int startTetra = tetraIndx;
    const Tetrahedron* t = &m_tetrahedrons[tetraIndx];
    int face = t->GetPointIndex (ptA);
    if (face == -1)
        return;
    do
        {
        const int* helper = Tetrahedron::GetDTetrahedron3dFacet (face);
        int oppFace = -1;
#ifdef LOG_TETRA
        fPtsLog += "[FindPointsAroundEdge] AT FACE " + std::to_string (face) + " OF TETRAHEDRON " + std::to_string (tetraIndx) + " containing [" + std::to_string (t->ptNums[0]) + "," + std::to_string (t->ptNums[1])
            + "," + std::to_string (t->ptNums[2]) + "," + std::to_string (t->ptNums[3]) + "]\r\n";
#endif
        for (int i = 0; i < 3; i++)
            {
            if (helper[i] >= 4)
                {
#ifdef LOG_TETRA
                /*   log << "[FindPointsAroundEdge] INVALID HELPER at START T#" + std::to_string(startTetra) + " AT T# " + std::to_string(tetraIndx) + "AT FACE " + std::to_string(face) + " AT POINTS " + std::to_string(ptA) + "/"
                + std::to_string(ptB) + " NUMS OF T [" + std::to_string(t->ptNums[0]) + "," + std::to_string(t->ptNums[1]) + "," + std::to_string(t->ptNums[2]) + "," + std::to_string(t->ptNums[3])
                + "] HELPER VALUES :["+std::to_string(helper[0])+","+std::to_string(helper[1])+","+std::to_string(helper[2])+"]" << endl;*/
#endif
                continue;
                }
            if (t->ptNums[helper[i]] == ptB)
                {
                oppFace = i;
#ifdef LOG_TETRA
                fPtsLog += "[FindPointsAroundEdge] OPPFACE FOUND " + std::to_string (i) + "\r\n";
#endif
                break;
                }
            }
        //BeAssert (oppFace != -1);
        if (oppFace == -1)
            {
#ifdef LOG_TETRA
            /*  log << "[FindPointsAroundEdge] OPPFACE -1 at START T#"+std::to_string(startTetra) +" AT T# "+std::to_string(tetraIndx) +"AT FACE "+std::to_string(face) +" AT POINTS "+std::to_string(ptA)+"/"
            + std::to_string(ptB) + " NUMS OF T [" + std::to_string(t->ptNums[0]) + "," + std::to_string(t->ptNums[1]) + "," + std::to_string(t->ptNums[2]) + "," + std::to_string(t->ptNums[3])
            + "]"<<endl;
            log.close();*/
#endif
            return;
            }
#ifdef LOG_TETRA
        fPtsLog += "[FindPointsAroundEdge] PUSHED BACK " + std::to_string (t->ptNums[helper[(oppFace + 1) % 3]]) + "\r\n";
#endif
        int ptNum = t->ptNums[helper[(oppFace + 1) % 3]];
        if (!IgnorePoint (ptNum))
            otherPt.push_back (PointOnEdge (ptNum, tetraIndx, helper[(oppFace + 2) % 3]));
        if (otherPt.size () > 100)
            return;
        int adjTet = t->GetAdjentTet (helper[(oppFace + 1) % 3]);// mapper[face][oppFace]);
#ifdef LOG_TETRA
        fPtsLog += "[FindPointsAroundEdge] FOUND ADJTET " + std::to_string (adjTet) + "\r\n";
#endif
        if (adjTet == -1)  // Need to look into this.
            {
            otherPt.clear ();
            return;
            }
        t = &m_tetrahedrons[adjTet];
        face = t->GetPointIndex (ptA);
#ifdef LOG_TETRA
        if (face >= 4 || face < 0)
            {
            /* log << "[FindPointsAroundEdge] INVALID FACE at START T#" + std::to_string(startTetra) + " AT T# " + std::to_string(tetraIndx) + "AT FACE " + std::to_string(face) + " AT POINTS " + std::to_string(ptA) + "/"
            + std::to_string(ptB) + " NUMS OF T [" + std::to_string(t->ptNums[0]) + "," + std::to_string(t->ptNums[1]) + "," + std::to_string(t->ptNums[2]) + "," + std::to_string(t->ptNums[3])
            + "] ADJTET IS "+std::to_string(adjTet) + " OPPFACE IS "+std::to_string(oppFace) << endl;*/
            }
#endif
        tetraIndx = adjTet;

        } while (startTetra != tetraIndx && tetraIndx != -1);
#ifdef LOG_TETRA
        if (std::find_if (otherPt.begin (), otherPt.end (), [ptA, ptB](PointOnEdge& pt)
            {
            if (pt.ptNum == ptA + 20 || pt.ptNum == ptB + 20 || pt.ptNum == ptA + 19 || pt.ptNum == ptB + 19 || pt.ptNum == ptA + 21 || pt.ptNum == ptB + 21) return true;
            else return false;
            }) == otherPt.end ())
            {
            // log << fPtsLog;
            }
            //  log << "[FindPointsAroundEdge] SIZE IS " + std::to_string(otherPt.size())<<endl;
#endif
            // TRACE_TETRA_END();
    }
