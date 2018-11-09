/*----------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/PolyfaceClip.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <map>
#include "IndexedClipEdge.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE




// Internal data used during face clip
struct VertexData
    {
    double m_altitude;
    double m_edgeFraction;
    size_t m_vertexIndex;
    size_t m_readIndex;
    DPoint3d mXYZ;
    int m_intData;

    static VertexData FromEdgeData (size_t readIndex, double edgeFraction, DPoint3dCR xyz)
        {
        VertexData data;
        data.m_edgeFraction = edgeFraction;
        data.m_readIndex = readIndex;
        data.m_edgeFraction = edgeFraction;
        return data;
        }

    static VertexData FromReadIndexAndCoordinates (size_t readIndex, DPoint3dCR xyz)
        {
        VertexData data;
        data.m_readIndex = readIndex;
        data.mXYZ = xyz;
        return data;
        }

    VertexData (size_t originalVertexIndex, DPoint3dCR xyz, double altitude)
        {
        mXYZ = xyz;
        m_vertexIndex = originalVertexIndex;
        m_readIndex = SIZE_MAX;
        m_altitude = altitude;
        m_edgeFraction = 0.0;
        }

    VertexData (size_t originalVertexIndex, size_t readIndex, DPoint3dCR xyz, ClipPlaneCR plane)
        {
        mXYZ = xyz;
        m_vertexIndex = originalVertexIndex;
        m_readIndex = readIndex;
        m_altitude = plane.EvaluatePoint (mXYZ);
        m_edgeFraction = 0.0;
        }

    VertexData (size_t originalVertexIndex, DPoint3dCR xyz, ClipPlaneCR plane)
        {
        mXYZ = xyz;
        m_vertexIndex = originalVertexIndex;
        m_readIndex = SIZE_MAX;
        m_altitude = plane.EvaluatePoint (mXYZ);
        m_edgeFraction = 0.0;
        }


    VertexData ()
        {
        mXYZ.Zero ();
        m_vertexIndex = SIZE_MAX;
        m_readIndex = SIZE_MAX;
        m_altitude = 0.0;
        m_edgeFraction = 0.0;
        }

    void MarkTolerancedAltitudeSign (double tol)
        {
        if (fabs (m_altitude) <= tol)
            m_intData = 0;
        else if (m_altitude < 0.0)
            m_intData = -1;
        else
            m_intData = 1;
        }

    // When used for crossing:
    // m_intData = sign (m_intData) from prior nonzero point. (-1==> upcross, +1===>downcross)
    // m_edgeFraction = fraction along edge.
    // m_XYZ = interpolation fraction.        
    static VertexData FromInterpolatedCrossing (VertexData const &dataA, double a, VertexData const &dataB)
        {
        VertexData dataC;
        double dA = dataB.m_altitude - dataA.m_altitude;
        dataC = dataA;
        if (dA == 0.0)
            {
            dataC = dataA;  // should never happen.
            dataC.m_intData = 0;
            }
        else
            {
            dataC.m_edgeFraction = (a - dataA.m_altitude) / dA;
            dataC.mXYZ.Interpolate (dataA.mXYZ, dataC.m_edgeFraction, dataB.mXYZ);
            dataC.m_intData = dA > 0.0 ? 1 : -1;
            dataC.m_intData = 0;            
            }
        return dataC;
        }
 
    static VertexData FromExactCrossing (VertexData const &dataA, int i)
        {
        VertexData dataC = dataA;   // m_XYZ copies !!!
        dataC.m_edgeFraction = 0.0;
        dataC.m_intData = i;
        return dataC;
        }

    static DRange3d RangeOf (bvector<VertexData> const &data)
        {
        DRange3d range = DRange3d::NullRange ();
        for (size_t i = 0; i < data.size (); i++)
            {
            range.Extend (data[i].mXYZ);
            }
        return range;
        };
    void AssignComponentAltitude (int componentIndex) {m_altitude = mXYZ.GetComponent (componentIndex);}
    
    static bool cb_CompareAltitude (VertexData const &dataA, VertexData const &dataB)
        {
        return dataA.m_altitude < dataB.m_altitude;
        }
    };

struct VertexDataPair
{
VertexData m_dataA;
VertexData m_dataB;
VertexDataPair (VertexData const &dataA, VertexData const &dataB)
    : m_dataA (dataA), m_dataB (dataB)
    {
    }

void AssignComponentAltitudeAndReorient (int componentIndex)
    {
    m_dataA.AssignComponentAltitude (componentIndex);
    m_dataB.AssignComponentAltitude (componentIndex);
    if (m_dataA.m_altitude > m_dataB.m_altitude)
        {
        std::swap (m_dataA, m_dataB);
        }
    }

static bool cb_CompareAltitudeA (VertexDataPair const &pairA, VertexDataPair const &pairB)
    {
    return pairA.m_dataA.m_altitude < pairB.m_dataA.m_altitude;
    }
};



// On successful ouptut
// (1) edges remain, but may have order or direction changed.
// (2) crossings are successive in-out pairs.  Order may change. Where there is overlap with an ON edge,
//      crossings may be adjusted.
static bool SortAndSimplifyCrossings (bvector<VertexData> &crossings, bvector <VertexDataPair> &edges)
    {
    if (crossings.size () == 0)
        return true;
    size_t numCrossings = crossings.size ();
    size_t numON = edges.size ();
    if ((numCrossings & 0x01) != 0)
        return false;   // should never happen -- crossings are supposed to be detected as full sign chnage.
    DRange3d crossingRange = VertexData::RangeOf (crossings);
    int sortDirection = crossingRange.IndexOfMaximalAxis ();
    for (size_t i = 0; i < numCrossings; i++)
        crossings[i].AssignComponentAltitude (sortDirection);
    for (size_t i = 0; i < numON; i++)
        edges[i].AssignComponentAltitudeAndReorient (sortDirection);
        
    std::sort (crossings.begin (), crossings.end (), VertexData::cb_CompareAltitude);
    if (numON > 0)
        std::sort (edges.begin (), edges.end (), VertexDataPair::cb_CompareAltitudeA);
    // check parity condition ... should never fail ....
    for (size_t i = 0; i < numCrossings; i += 2)
        {
        if (crossings[i].m_intData * crossings[i+1].m_intData > 0)
            return false;
        }
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct VertexDataArray : bvector<VertexData>
{
    size_t mKMin, mKMax, mKMaxAbs;
    double mMinAltitude, mMaxAltitude, mMaxAbsAltitude;


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void AddInterpolation(size_t tag, VertexData const &vertexA, double fraction, VertexData const &vertexB, double a = 0.0)
    {
    DPoint3d xyz;
    xyz.Interpolate (vertexA.mXYZ, fraction, vertexB.mXYZ);
    push_back (VertexData (tag, xyz,a));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void Add(size_t tag, DPoint3dCR xyz, double a = 0.0)
    {
    push_back (VertexData (tag, xyz,a));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void AddAltitude(size_t tag, DPoint3dCR xyz, ClipPlaneCR plane)
    {
    push_back (VertexData (tag, xyz, plane));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void AddAltitude(size_t vertexIndex, size_t readIndex, DPoint3dCR xyz, ClipPlaneCR plane)
    {
    push_back (VertexData (vertexIndex, readIndex, xyz, plane));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void Replicate(size_t numCompleteCopy)
    {
    size_t n = size ();
    for (size_t copy = 0; copy < numCompleteCopy; copy++)
        {
        for (size_t i = 0; i < n; i++)
            {
            VertexData data = at(i);
            push_back (data);
            }
        }
    }


size_t kMin, kMax, kAbsMax;
double aMin, aMax, aAbsMax;


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void FindAltitudeExtremes()
    {
    kMin = kMax = 0;
    aMin = aMax = at(0).m_altitude;
    for (size_t n = size (), k = 1; k < n; k++)
        {
        double a = at(k).m_altitude;
        if (a < aMin)
            {
            aMin = a;
            kMin = k;
            }
        if (a > aMax)
            {
            aMax = a;
            kMax = k;
            }
        }
    if (fabs (aMax) >= fabs (aMin))
        {
        kAbsMax = kMax;
        aAbsMax = fabs (aMax);
        }
    else
        {
        kAbsMax = kMin;
        aAbsMax = fabs (aMin);
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void SetReplicatedAltitudes(
size_t i0,
size_t n0,
bvector<DPoint3d>const &points,
ClipPlaneCR plane,
size_t numReplications
)
    {
    clear ();
    // Fill altitudes numbered from 0.
    for (size_t k = 0; k < n0; k++)
        AddAltitude (i0 + k, points[i0 + k], plane);

    FindAltitudeExtremes ();
    Replicate (numReplications);
    }

void SetReplicatedAltitudes(
size_t i0,
size_t n0,
bvector<DPoint3d>const &points,
bvector<size_t>const &indexPosition,
ClipPlaneCR plane,
size_t numReplications
)
    {
    clear ();
    // Fill altitudes numbered from 0.
    for (size_t k = 0; k < n0; k++)
        AddAltitude (i0 + k, indexPosition[i0 + k], points[i0 + k], plane);

    FindAltitudeExtremes ();
    Replicate (numReplications);
    }


void SetTolerancedAltitudeFlag (double zTol)
    {
    for (size_t i = 0, n = size (); i < n; i++)
        {
        at(i).MarkTolerancedAltitudeSign (zTol);
        }
    }
};

#ifdef abc


/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct PolyfaceClipContext
{
private:
PolyfaceVisitor&            m_visitor;
PolyfaceCoordinateMapP      m_insideMap;
PolyfaceCoordinateMapP      m_outsideMap;
PolyfaceHeaderP             m_insideMesh;
PolyfaceHeaderP             m_outsideMesh;
bmap <ClipPlaneP, int>      m_planeToIdMap;

PolyfaceClipContext (PolyfaceVisitor& visitor, PolyfaceCoordinateMapP insideDest, PolyfaceCoordinateMapP outsideDest)
    : m_visitor (visitor),
      m_insideMap  (insideDest),
      m_outsideMap (outsideDest)
    {
    m_insideMesh = m_insideMap == NULL ? NULL : &m_insideMap->GetPolyfaceHeaderR ();
    m_outsideMesh = m_outsideMap == NULL ? NULL : &m_outsideMap->GetPolyfaceHeaderR ();
    m_visitor.ClientPointIndex ().SetActive (true);   // used as original base index during clip (?)
    m_visitor.ClientNormalIndex ().SetActive (true);  // used as plane index during clip
    }

VertexDataArray mVertexAltitudeData;

ClipEdgeDataArray m_globalEdgeData;
ClipEdgeDataArray m_localEdgeData;


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void EmitLoop(PolyfaceHeaderR dest, bvector<size_t> chainIndices, bool reverse = false)
    {
    // chain indices are already one based?
    size_t n = chainIndices.size ();
    if (n < 3)
        return;
    if (reverse)
        for (size_t i = n; i-- > 0;)
            dest.PointIndex ().push_back ((int) (chainIndices[i]));
    else
        for (size_t i = 0; i < n; i++)
            dest.PointIndex ().push_back ((int) (chainIndices[i]));
        
    dest.PointIndex ().push_back (0);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void AnalyzeCutPlaneLoops()
    {
    m_globalEdgeData.SortByPlaneVertexAVertexB ();
    size_t beginIndex = 0;
    size_t endIndex;
    bvector<size_t> startIndices;
    bvector<size_t> endIndices;
    bvector<size_t> chainIndices;
    bool reverseLoops = true;
    for (beginIndex = 0;
            beginIndex < (endIndex = m_globalEdgeData.FindEndOfPlaneCluster (beginIndex));
            beginIndex = endIndex)
        {
        startIndices.clear ();
        endIndices.clear ();
        if (m_globalEdgeData.AnalyzeCutPlane (beginIndex, endIndex, startIndices, endIndices))
            {
            m_globalEdgeData.SetFlag (beginIndex, endIndex, false);
            // Pull out obvious 
            for (size_t index = beginIndex; index < endIndex; index++)
                {
                if (m_globalEdgeData.GetFlag (index))
                    continue;
                chainIndices.clear ();
                if (startIndices.size () == 1 && endIndices.size () == 1)
                    {
                    m_globalEdgeData.ExtendChain (beginIndex, endIndex, startIndices[0], chainIndices, true);
                    EmitLoop (*m_insideMesh, chainIndices, reverseLoops);
                    }
                else if (startIndices.size () == 2 && endIndices.size () == 2)
                    {
                    // Um.. this wrong for reentrant mesh.   Need to restrict to clipper edges.
                    m_globalEdgeData.ExtendChain (beginIndex, endIndex, startIndices[0], chainIndices, true);
                    m_globalEdgeData.ExtendChain (beginIndex, endIndex, startIndices[1], chainIndices, true);
                    EmitLoop (*m_insideMesh, chainIndices, reverseLoops);
                    }
                }
            // Anything left is a complete loop.
            for (size_t startIndex = beginIndex; startIndex < endIndex; startIndex++)
                {
                // This is wrong for holeInFace !!!
                if (!m_globalEdgeData.GetFlag (startIndex))
                    {
                    chainIndices.clear ();
                    m_globalEdgeData.ExtendChain (beginIndex, endIndex, startIndex, chainIndices, false);
                    EmitLoop (*m_insideMesh, chainIndices, reverseLoops);
                    }
                }
            }
        }
    }



// Clip a the single loop of points to a single plane.
// Append the new points to the end of the data arrays in the source.
// Both input and output polygons are subsets of the same PolyfaceVisitor, identified by start index and count.
// (i0,n0) and (i1,n1) may be passed by naming the same var, so clipping proceeds with successive subsets.
void ClipVisitorFaceToSinglePlane (size_t i0, size_t n0, ClipPlaneR plane, size_t &i1, size_t &n1, size_t &numClipPoints)
    {
    numClipPoints = 0;
    i1 = m_visitor.Point ().size ();
    n1 = 0;
    if (n0 < 3)
        return;

    mVertexAltitudeData.SetReplicatedAltitudes (i0, n0, m_visitor.Point (), plane, 2);

    if (mVertexAltitudeData.aMax <= 0.0) // All OUT
        {
        i1 = i0;
        n1 = 0;
        return;
        }

    if (mVertexAltitudeData.aMin >= 0.0) // All IN.  Output stays in place
        {
        i1 = i0;
        n1 = n0;
        return;
        }

    // NEEDS WORK :  All ON case?
    // NEEDS WORK :  Tolerance ?
    // Start at point with maximum distance (abs) from plane.
    // Pass through top of loop only on strictly nonzero points.
    // Zeros within are absorbed at inner loops.
    size_t kStart = mVertexAltitudeData.kAbsMax;
    VertexData vertexA = mVertexAltitudeData[kStart];
    VertexData vertexB;

    bmap <ClipPlaneP, int>::iterator        found = m_planeToIdMap.find (&plane);
    int                                     planeId = (found == m_planeToIdMap.end()) ? -1 : found->second;
    

    for (size_t k = 1; k <= n0; k++, vertexA = vertexB)
        {
        vertexB = mVertexAltitudeData[kStart + k];
        if (vertexA.m_altitude > 0.0)
            {
            m_visitor.PushFaceData (m_visitor, vertexA.m_vertexIndex);
            if (vertexB.m_altitude > 0.0)
                {
                }
            else if (vertexB.m_altitude < 0.0)
                {
                double fraction = -vertexA.m_altitude / (vertexB.m_altitude - vertexA.m_altitude);
                numClipPoints++;
                m_visitor.PushInterpolatedFaceData (m_visitor, vertexA.m_vertexIndex, fraction, vertexB.m_vertexIndex);
                size_t iLast = m_visitor.Point().size () - 1;
                if (plane.IsVisible())
                    m_visitor.ClientNormalIndex () [iLast] = planeId;
                }
            else
                {
                while (vertexB.m_altitude == 0.0)
                    {
                    m_visitor.PushFaceData (m_visitor, vertexB.m_vertexIndex);
                    vertexA = vertexB;
                    vertexB = mVertexAltitudeData[kStart + (++k)];
                    }
                size_t iLast = m_visitor.Point().size () - 1;
                if (plane.IsVisible())
                    m_visitor.ClientNormalIndex () [iLast] = planeId;
                }
            }
        else    // vertexA.m_altitude strictly negative ....
            {
            if (vertexB.m_altitude < 0.0)
                {
                }
            else if (vertexB.m_altitude > 0.0)
                {
                double fraction = -vertexA.m_altitude / (vertexB.m_altitude - vertexA.m_altitude);
                numClipPoints++;
                m_visitor.PushInterpolatedFaceData (m_visitor, vertexA.m_vertexIndex, fraction, vertexB.m_vertexIndex);
                // Plane index is NOT recorded on ON-to-IN crossing !!!
                }
            else
                {
                while (vertexB.m_altitude == 0.0)
                    {
                    m_visitor.PushFaceData (m_visitor, vertexB.m_vertexIndex);
                    vertexA = vertexB;
                    vertexB = mVertexAltitudeData[kStart + (++k)];
                    }
                }
            }
        }
    n1 = m_visitor.Point ().size () - i1;
    }





/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void AddVisitorPartialFace(
PolyfaceCoordinateMapP destMap,
size_t i0,
size_t n,
bool addLoops = true,
bool addCuts = true
)
    {
    if (NULL == destMap)
        return;
    PolyfaceHeaderR polyface = destMap->GetPolyfaceHeaderR ();
    m_localEdgeData.clear ();
    for (size_t i = 0; i < n; i++)
        {
        size_t readIndex = i0 + i;
        DPoint3d xyz = m_visitor.Point()[readIndex];
        int pointIndex = 1 + (int)destMap->AddPoint (xyz);
        m_localEdgeData.push_back (ClipEdgeData (pointIndex, pointIndex, m_visitor.ClientNormalIndex()[readIndex]));

        mVertexAltitudeData[i].m_vertexIndex = pointIndex;
        if (addLoops)
            {
            if (!m_visitor.Visible()[readIndex])
                pointIndex = - pointIndex;
            polyface.PointIndex ().push_back (pointIndex);
            if (m_visitor.Normal ().Active ())
                polyface.NormalIndex ().push_back (1 + (int)destMap->AddNormal (m_visitor.Normal()[readIndex]));
            if (m_visitor.Param ().Active ())
                polyface.ParamIndex ().push_back  (1 + (int)destMap->AddParam (m_visitor.Param ()[readIndex]));
            }
        }

    if (addCuts)
        {
        // Record edges that arise from clip plane intersections.
        ClipEdgeData edge0 = m_localEdgeData[0];
        m_localEdgeData.push_back (edge0);  // Simplify wraparound
        for (size_t i = 0; i < n; i++)
            {
            ClipEdgeData edge0 = m_localEdgeData[i];
            ClipEdgeData edge1 = m_localEdgeData[i+1];
            if (edge0.m_planeIndex >= 0)
                m_globalEdgeData.push_back (ClipEdgeData (edge0.m_vertexA, edge1.m_vertexA, edge0.m_planeIndex));
            }
        polyface.TerminateAllActiveIndexVectors ();
        }
    }

//! Clip the single face of the visitor to a single plane set (intersection of half spaces)
//! Add each clipped piece to the growing mapped mesh.
//! The visitor face is unchanged at end.
//! (But the visitor data arrays are expanded and trimmed at intermediate steps.)
void ClipCurrentFaceToConvexPlaneSet (ConvexClipPlaneSetP clipPlanes, bool includeOffPlaneParts = true)
    {
    size_t i0 = 0;
    size_t n0  = m_visitor.Point ().size ();
    size_t i1 = i0;
    size_t n1 = n0;
    size_t numClipPoint = 0;

    // Use the normal index as plane indicator ..
    m_visitor.ClientNormalIndex ().clear ();
    for (size_t i = 0; i < n0; i++)
        {
        m_visitor.ClientNormalIndex ().push_back (-1);
        }

   for (size_t planeIndex = 0; n1 > 2 && planeIndex < clipPlanes->size(); planeIndex++)
        {
        ClipVisitorFaceToSinglePlane (i1, n1, clipPlanes->at (planeIndex), i1, n1, numClipPoint);
        }
    if (n1 > 2)
        AddVisitorPartialFace (m_insideMap, i1, n1, includeOffPlaneParts, true);

    m_visitor.TrimFaceData (i0, n0);
    }


//! Clip the single face of the visitor to each of the chain of clip plane sets.
//! Add each clipped piece (from each clip plane set) to the growing mapped mesh.
//! The visitor face is unchanged at end.
//! (But the visitor data arrays are expanded and trimmed at intermediate steps.)
void ClipCurrentFace (ClipPlaneSetP clipPlanes, bool includeVolume = true)
    {
    for (ConvexClipPlaneSetR convexPlaneSet: *clipPlanes)
        ClipCurrentFaceToConvexPlaneSet (&convexPlaneSet, includeVolume);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void SuppressAuxArrays(PolyfaceHeaderP mesh)
    {
    if (NULL != mesh)
        {
        mesh->Normal     ().SetActive (false);
        mesh->Param      ().SetActive (false);
        mesh->IntColor   ().SetActive (false);

        mesh->NormalIndex().SetActive (false);
        mesh->ParamIndex ().SetActive (false);
        mesh->ColorIndex ().SetActive (false);

        mesh->Normal     ().clear ();
        mesh->Param      ().clear ();
        mesh->IntColor   ().clear ();

        mesh->NormalIndex ().clear ();
        mesh->ParamIndex  ().clear ();
        mesh->ColorIndex  ().clear ();
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void SuppressAuxArrays()
    {
    SuppressAuxArrays (m_insideMesh);
    SuppressAuxArrays (m_outsideMesh);
    }

public:

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void ClipToChain
(
PolyfaceVisitor&        visitor,
PolyfaceCoordinateMapP  insideDest,
PolyfaceCoordinateMapP  outsideDest,
ClipPlaneSetP           clipPlanes,
bool                    formNewFacesOnClipPlanes
)
    {
    PolyfaceClipContext context (visitor, insideDest, outsideDest);
    // Fix up (blast!) plane indices..
    int planeCounter = 0;
    
    for (ConvexClipPlaneSet& convexSet: *clipPlanes)
        {
        for (ClipPlane& clipPlane: convexSet)
            context.m_planeToIdMap[&clipPlane] = planeCounter++;
        }
    for (visitor.Reset ();visitor.AdvanceToNextFace ();)
        {
        context.ClipCurrentFace (clipPlanes);
        }

    if (formNewFacesOnClipPlanes)
        context.AnalyzeCutPlaneLoops ();
    context.SuppressAuxArrays ();
    }
};



//! Visit each face of source. Clip to chain and capture the clipped residual.
GEOMDLLIMPEXP void PolyfaceCoordinateMap::AddClippedPolyfaceA 
(
PolyfaceQueryR          source,
PolyfaceCoordinateMapP  insideDest,
PolyfaceCoordinateMapP  outsideDest,
ClipPlaneSetP           clipPlanes,
bool                    formNewFacesOnClipPlanes
)
    {
    if (NULL != insideDest)
        {
        insideDest->m_polyface.SetMeshStyle (MESH_ELM_STYLE_INDEXED_FACE_LOOPS);
        insideDest->m_polyface.ActivateVectorsForIndexing (source);
        }

    if (NULL != outsideDest)
        {
        outsideDest->m_polyface.SetMeshStyle (MESH_ELM_STYLE_INDEXED_FACE_LOOPS);
        outsideDest->m_polyface.ActivateVectorsForIndexing (source);
        }


    PolyfaceVisitorPtr visitorPtr = PolyfaceVisitor::Attach (source, true);
    PolyfaceVisitor & visitor = *visitorPtr.get ();

    PolyfaceClipContext::ClipToChain (visitor, insideDest, outsideDest, clipPlanes, formNewFacesOnClipPlanes);
    }
#endif
PolyfaceHeaderPtr PolyfaceHeader::CreateFromTaggedPolygons (TaggedPolygonVectorCR polygons)
    {
    auto pf = PolyfaceHeader::CreateVariableSizeIndexed ();
    for (auto &p : polygons)
        {
        pf->AddPolygon (p.GetPointsCR ());
        }
    return pf;
    }


void PolyfaceHeader::VisibleParts
(
bvector<PolyfaceHeaderPtr> &source, //!< [in] multiple meshes for viewing
DVec3dCR vectorToEye,               //!< [in] vector towards the eye
PolyfaceHeaderPtr &dest,            //!< [out] new mesh, containing only the visible portions of the inputs
TransformR localToWorld,            //!< [out] axes whose xy plane is the xy plane for viewing along local z axis.
TransformR worldToLocal             //!< [out] transform used to put the polygons in xy viewing position.
)
    {
    localToWorld.InitIdentity ();
    worldToLocal.InitIdentity ();
    TaggedPolygonVector polygonsIn, polygonsOut;
    for (size_t i = 0; i < source.size (); i++)
        {
        auto visitor = PolyfaceVisitor::Attach (*source[i], false);
        bvector<DPoint3d> &points = visitor->Point ();
        for (visitor->Reset (); visitor->AdvanceToNextFace ();)
            PolygonVectorOps::AddPolygon (polygonsIn,
                points, i, visitor->GetReadIndex ());
        }
    dest = nullptr;
    if (polygonsIn.size () == 0)
        return;
    DRange3d range = PolygonVectorOps::GetRange (polygonsIn);
    DPoint3d center = range.LocalToGlobal (0.5, 0.5, 0.5);
    auto axes = RotMatrix::From1Vector (vectorToEye, 2, true);
    localToWorld = Transform::From (axes, center);
    worldToLocal.InverseOf (localToWorld);

    PolygonVectorOps::Multiply (polygonsIn, worldToLocal);
    bsiPolygon_clipByXYVisibility (polygonsIn, polygonsOut, true, false);
    PolygonVectorOps::Multiply (polygonsOut, localToWorld);
    dest = CreateFromTaggedPolygons (polygonsOut);
    }




struct AcceptByIndex : Acceptor <TaggedPolygon>
{
size_t index;
AcceptByIndex (size_t i) : index(i) {}
bool Accept (TaggedPolygonCR polygon) override { return index == polygon.GetIndexA ();}
};

PolyfaceHeaderPtr PolyfaceHeader::CreateFromTaggedPolygons
(
TaggedPolygonVectorCR polygons,
Acceptor<TaggedPolygon> &acceptor,
bvector<SizeSize> *destReadIndexToSourceIndex
)
    {
    PolyfaceHeaderPtr pf = nullptr;
    for (size_t i = 0; i < polygons.size (); i ++)
        {
        TaggedPolygon const & p = polygons[i];
        if (acceptor.Accept (p))
            {
            if (pf == nullptr)
                pf = PolyfaceHeader::CreateVariableSizeIndexed ();
            size_t destReadIndex = pf->PointIndex ().size ();
            pf->AddPolygon (p.GetPointsCR ());
            if (destReadIndexToSourceIndex != nullptr)
                destReadIndexToSourceIndex->push_back (SizeSize (destReadIndex, i));
            }
        }
    return pf;
    }



void PolyfaceHeader::VisibleParts
(
bvector<PolyfaceHeaderPtr> &source, //!< [in] multiple meshes for viewing
DVec3dCR vectorToEye,               //!< [in] vector towards the eye
bvector<PolyfaceHeaderPtr> &dest,            //!< [out] array of new mesh, containing only the visible portions of the inputs.  If a particular mesh source[i] has no visible parts, its corresponding dest[i] is a null pointer.
bvector<bvector<SizeSize>> *destReadIndexToSourceReadIndex,  //!< [out] array connecting destMesh readIndex to its corresponsding readIndex in the corresponding source mesh
TransformR localToWorld,            //!< [out] axes whose xy plane is the xy plane for viewing along local z axis.
TransformR worldToLocal             //!< [out] transform used to put the polygons in xy viewing position.
)
    {
    dest.clear ();
    localToWorld.InitIdentity ();
    worldToLocal.InitIdentity ();
    TaggedPolygonVector polygonsIn, polygonsOut;
    for (size_t i = 0; i < source.size (); i++)
        {
        auto visitor = PolyfaceVisitor::Attach (*source[i], false);
        bvector<DPoint3d> &points = visitor->Point ();
        for (visitor->Reset (); visitor->AdvanceToNextFace ();)
            PolygonVectorOps::AddPolygon (polygonsIn,
                points, i, visitor->GetReadIndex ());
        }
    if (polygonsIn.size () == 0)
        return;
    DRange3d range = PolygonVectorOps::GetRange (polygonsIn);
    DPoint3d center = range.LocalToGlobal (0.5, 0.5, 0.5);
    auto axes = RotMatrix::From1Vector (vectorToEye, 2, true);
    localToWorld = Transform::From (axes, center);
    worldToLocal.InverseOf (localToWorld);

    PolygonVectorOps::Multiply (polygonsIn, worldToLocal);
    bsiPolygon_clipByXYVisibility (polygonsIn, polygonsOut, true, false);
    PolygonVectorOps::Multiply (polygonsOut, localToWorld);
    // ASSUME ... a smallish number of input meshes, so this loop doesn't execute often ..
    for (size_t i = 0; i < source.size (); i++)
        {
        AcceptByIndex acceptor (i);
        if (destReadIndexToSourceReadIndex != nullptr)
            {
            destReadIndexToSourceReadIndex->push_back (bvector<SizeSize> ());
            dest.push_back(CreateFromTaggedPolygons (polygonsOut, acceptor , &destReadIndexToSourceReadIndex->back ()));
            // the mapping dataB is index in the visible polygons.
            // dereference that to the source readIndex.
            for (auto &p : destReadIndexToSourceReadIndex->back ())
                {
                size_t polygonIndex = p.dataB;
                p.dataB = polygonsOut[polygonIndex].GetIndexB ();
                }
            }
        else
            {
            dest.push_back(CreateFromTaggedPolygons (polygonsOut, acceptor , nullptr));
            }
        }
    }
END_BENTLEY_GEOMETRY_NAMESPACE

#include "PolyfaceSectionMTG.h"
