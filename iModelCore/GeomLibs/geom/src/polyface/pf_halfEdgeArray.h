/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
// This .h file is to be included in polyface files  . . . not independently.

BEGIN_BENTLEY_GEOMETRY_NAMESPACE




struct Size2
{
size_t m_dataA;
size_t m_dataB;
Size2 (size_t data) : m_dataA(data), m_dataB(data) {}

bool ContainsAsBeginEnd (size_t data) const {return data >= m_dataA && data < m_dataB;}

size_t GetFirstDifferentIndex (size_t key)
    {
    for (size_t candidate = m_dataA; candidate < m_dataB; candidate++)
        if (candidate != key)
            return candidate;
    return SIZE_MAX;
    }
size_t NumInRange () const { return m_dataB > m_dataA ? m_dataB - m_dataA : 0;}
};


struct Size4
{
size_t m_data[4];
Size4(size_t i0, size_t i1, size_t i2, size_t i3)
    {
    m_data[0] = i0;
    m_data[1] = i1;
    m_data[2] = i2;
    m_data[3] = i3;
    }
};

// One side of edge.
// CONVENTION: vertex indices are signed one based.  Comparison logic has to apply abs.
//
struct HalfEdge
{
ptrdiff_t m_vertex0;
ptrdiff_t m_vertex1;
size_t m_readIndex;
size_t m_faceSuccessorReadIndex;
MTGNodeId m_nodeId;
uint8_t m_visible;
uint8_t m_strongVisible;

HalfEdge (size_t position, size_t faceSuccessorPosition, size_t vertex0, size_t vertex1, bool visible, MTGNodeId nodeId = MTG_NULL_NODEID)
    : m_readIndex (position),
      m_faceSuccessorReadIndex (faceSuccessorPosition),
      m_vertex0 (vertex0),
      m_vertex1 (vertex1),
      m_nodeId  (nodeId),
      m_visible (visible),
      m_strongVisible (false)
    {}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t LowVertex()  const { return abs((int)m_vertex0) < abs((int)m_vertex1) ? abs((int)m_vertex0) : abs((int)m_vertex1);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t HighVertex() const { return abs((int)m_vertex0) < abs((int)m_vertex1) ? abs((int)m_vertex1) : abs((int)m_vertex0);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t LowVertex0Based()  const { return (abs((int)m_vertex0) < abs((int)m_vertex1) ? abs((int)m_vertex0) : abs((int)m_vertex1)) -1;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t HighVertex0Based() const { return (abs((int)m_vertex0) < abs((int)m_vertex1) ? abs((int)m_vertex1) : abs((int)m_vertex0)) - 1;}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IsSingleVertex() const {return abs((int)m_vertex0) == abs((int)m_vertex1);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IsReversedMateOf(HalfEdge &other)
    {
    return abs((int)m_vertex0) == abs((int)other.m_vertex1)
        && abs((int)m_vertex1) == abs((int)other.m_vertex0);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      10/2015
+--------------------------------------------------------------------------------------*/
bool IsIdenticalMateOf (HalfEdge &other)
    {
    return 
        abs((int)m_vertex0) == abs((int)other.m_vertex0)
        && abs((int)m_vertex1) == abs((int)other.m_vertex1);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      10/2015
+--------------------------------------------------------------------------------------*/
bool IsAnyMateOf (HalfEdge &other)
    {
    return IsReversedMateOf (other) || IsIdenticalMateOf (other);
    }

bool IsVisible () const {return m_visible;}
void SetVisible(bool value) {m_visible = value;}

bool IsStrongVisible () const { return m_strongVisible; }
void SetStrongVisible (bool value) { m_strongVisible = value; }

void ReverseVertices ()
    {
    bool negate = m_vertex0 < 0;
    ptrdiff_t vertex0 = abs ((int)m_vertex0);
    ptrdiff_t vertex1 = abs ((int)m_vertex1);
    m_vertex0 = vertex1;
    m_vertex1 = vertex0;
    if (negate)
        vertex0 = -vertex0;
    }
};


/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct HalfEdgeArray : bvector<HalfEdge>
{
PolyfaceQueryCP m_source;

bvector<size_t> m_readIndexToBaseReadIndex;

HalfEdgeArray (PolyfaceQueryCP source)
    : m_source (source)
    {
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool cb_LessThan_LowVertexHighVertex(HalfEdge const &edgeA, HalfEdge const & edgeB)
    {
    size_t lowA = edgeA.LowVertex ();
    size_t lowB = edgeB.LowVertex ();
    if (lowA < lowB)
        return true;
    if (lowA > lowB)
        return false;
    return edgeA.HighVertex () < edgeB.HighVertex ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void Add(size_t position, size_t faceSuccessorPosition, ptrdiff_t indexA, ptrdiff_t indexB, bool visible, bool strongVisible)
    {
    push_back (HalfEdge(position, faceSuccessorPosition, indexA, indexB, visible));
    back ().SetStrongVisible(strongVisible);
    }
// input any index in the half edge array.
// return a Size2 with:
// * m_dataA = lowest half edge with same vertex indices
// * m_dataB = one more than highest half edge with same vertex indices
//         
Size2 MateRange (size_t index)
    {
    Size2 range (index);
    size_t n = size ();
    if (index < n)
        {
        range.m_dataB++;
        while (range.m_dataB < n && !cb_LessThan_LowVertexHighVertex (at(index), at(range.m_dataB)))
            range.m_dataB++;
        while (range.m_dataA > 0 && !cb_LessThan_LowVertexHighVertex (at(range.m_dataA - 1), at(index)))
            range.m_dataA--;
        }
    return range;
    }






/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IsVisibleEdge(size_t i) const
    {
    if (NULL == m_source)
        return i < size ();

    if (i < size ())
        {
        size_t position = at(i).m_readIndex;
        if (position < m_source->GetPointIndexCount ())
            {
            int signedVertexIndex = m_source->GetPointIndexCP ()[position];
            return signedVertexIndex > 0;
            }
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool GetSegment(size_t i, DSegment3dR segment) const
    {
    if (i < size () && NULL != m_source)
        {
        HalfEdge he = at (i);
        size_t v0 = he.LowVertex0Based ();
        size_t v1 = he.HighVertex0Based ();
        size_t n = (int)m_source->GetPointCount ();
        if (v0 < n && v1 < n)
            {
            segment.point[0] = m_source->GetPointCP ()[v0];
            segment.point[1] = m_source->GetPointCP ()[v1];
            return true;
            }
        }
    return false;
    }

bool GetNormals (size_t i, DVec3d &normalA, DVec3d &normalB)
    {
    if (i < size () && NULL != m_source)
        {
        HalfEdge he = at (i);
        size_t riA = he.m_readIndex;
        size_t riB = he.m_faceSuccessorReadIndex;
        return m_source->TryGetNormalAtReadIndex (riA, normalA)
            && m_source->TryGetNormalAtReadIndex (riB, normalB);
        }
    return false;
    }

bool IsReversedPair (size_t i, size_t j)
    {
    if (i < size () && j < size ())
        {
        HalfEdge hei = at (i);
        HalfEdge hej = at (j);
        return hei.m_vertex0 == hej.m_vertex1
            && hei.m_vertex1 == hej.m_vertex0;

        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void SortForEdgeMatching() {std::sort (begin (), end (), cb_LessThan_LowVertexHighVertex);}

//! counts[0] = number of (half)edges.
//! counts[1..k..countSize-2] = number of edges with k incident faces.
//! counts[countSize-1] = number of edges with countSize-1 or more indicent faces.
//! countSize = declared size of the counts[] array.
//! numCollapsed = number of edges collapsed to point.
void CountEdgeSharing (
PolyfaceQueryCP mesh,   // only used for debug.
bvector<bvector<DSegment3d>> *unmatchedEdges,  // debug --collect unmatched edge pairs
size_t &numMatedPair, size_t counts [], size_t countSize, size_t &numCollapsed, size_t &num0Visible, size_t &num1OrMoreVisible)
    {
    size_t numHalfEdge = counts[0] = size ();
    for (size_t i = 1; i < countSize; i++)
        counts[i] = 0;
    size_t lastCountIndex = countSize - 1;
    numMatedPair = 0;
    numCollapsed = 0;
    num0Visible = 0;
    num1OrMoreVisible = 0;
    for (size_t i0 = 0, numMatch = 0; i0 < numHalfEdge; i0 += numMatch)
        {
        numMatch = 1;
        for (size_t i1 = i0 + 1; i1 < numHalfEdge && !cb_LessThan_LowVertexHighVertex (at(i0), at(i1)); i1++)
            {
            numMatch++;
            }

        size_t numVisible = 0;
        for (size_t i = i0; i < i0 + numMatch; i++)
            {
            if (at(i).IsVisible ())
                numVisible++;
            }
        if (numVisible == 0)
            num0Visible++;
        else
            num1OrMoreVisible++;

        if (at(i0).IsSingleVertex ())
            {
            numCollapsed += numMatch;
            }
        else
            {
            if (numMatch >= lastCountIndex)
                counts[lastCountIndex] += 1;
            else
                counts[numMatch] += 1;
            if (numMatch == 2 && at(i0).IsReversedMateOf (at(i0 + 1)))
                numMatedPair++;
            else if (mesh != nullptr && unmatchedEdges != nullptr)
                {
                unmatchedEdges->push_back (bvector<DSegment3d> ());
                for (size_t i = 0; i < numMatch; i++)
                    {
                    DSegment3d segment;
                    if (GetSegment (i0 + i, segment))
                        unmatchedEdges->back ().push_back (segment);
                    }
                }
            }
        }
    }

//! collect simple edges.
//! assume array is sorted.
void CollectSegments (bvector<DSegment3d> &segments) const
    {
    size_t numHalfEdge = size ();
    segments.clear ();
    for (size_t i0 = 0, numMatch = 0; i0 < numHalfEdge; i0 += numMatch)
        {
        numMatch = 1;
        for (size_t i1 = i0 + 1; i1 < numHalfEdge && !cb_LessThan_LowVertexHighVertex (at(i0), at(i1)); i1++)
            {
            numMatch++;
            }
        
        DSegment3d segment;
        if (GetSegment (i0, segment))
            segments.push_back (segment);
        }
    }

void IncrementSignCounts (int &numPositive, int &numNegative, int &numZero, double dot)
    {
    if (fabs (dot) < Angle::SmallAngle ())
        numZero++;
    else if (dot < 0.0)
        numNegative++;
    else
        numPositive++;
    }
//! Assume array is sorted.   Assume normals are available from the mesh.
//! ASSUME mesh arg is writable refernece to same PolyfaceQuery
//! Examine normal angles on each side of paired edges.   Mark visible if variation is large, invisible if small.  
bool MarkVisibility 
(
double smoothAngle,
PolyfaceHeaderR mesh,
DVec3dCP silhouetteVector
)
    {
    size_t numHalfEdge = size ();
    bvector<int> &pointIndices = mesh.PointIndex ();
    double smoothAngleMinusTolerance = smoothAngle - Angle::SmallAngle ();   // Let exact crease angles be VISIBLE !!!!
    size_t numBoundary = 0;
    size_t numPositiveIndex = 0;
    size_t numNegativeIndex = 0;
    for (size_t i0 = 0, numMatch = 0; i0 < numHalfEdge; i0 += numMatch)
        {
        numMatch = 1;
        for (size_t i1 = i0 + 1; i1 < numHalfEdge && !cb_LessThan_LowVertexHighVertex (at(i0), at(i1)); i1++)
            {
            numMatch++;
            }
        bool smooth = false;
        DVec3d normalA0, normalB0;
        DVec3d normalA1, normalB1;
        if (numMatch != 2)
            {
            smooth = false;
            numBoundary++;
            }
        else if (numMatch == 2 && GetNormals (i0, normalA0, normalB0))
            {
            double thetaA0B0, thetaA0A1, thetaB0B1;
            if (smoothAngle >= 0.0)
                {
                // confirm acceptable angle change along this side ...
                thetaA0B0 = normalA0.AngleTo (normalB0);
                smooth = true;
                if (thetaA0B0 > smoothAngleMinusTolerance)
                    smooth = false;
                if (at(i0).IsStrongVisible())
                    smooth = false;
                // and acceptable angle change across edge end ...
                for (size_t i = i0 + 1; smooth && i < i0 + numMatch
                                && GetNormals (i, normalA1, normalB1); i++)
                    {
                    thetaA0A1 = normalA0.AngleTo (normalA1);
                    thetaB0B1 = normalB0.AngleTo (normalB1);
                    if (at(i).IsStrongVisible ())
                        smooth = false;
                    if (thetaA0A1 > smoothAngleMinusTolerance)
                        smooth = false;
                    if (thetaB0B1 > smoothAngleMinusTolerance)
                        smooth = false;
                    }
                 }
            else
                smooth = true;

            int numPositive = 0;
            int numNegative = 0;
            int numZero = 0;
            if (nullptr != silhouetteVector)
                {
                for (size_t i = i0; i < i0 + numMatch
                            && GetNormals (i, normalA1, normalB1);
                            i++)
                    {
                    double dotA = normalA1.DotProduct (*silhouetteVector);
                    double dotB = normalB1.DotProduct (*silhouetteVector);
                    IncrementSignCounts (numPositive, numNegative, numZero, dotA);
                    IncrementSignCounts (numPositive, numNegative, numZero, dotB);
                    }
                int numTotal = numPositive + numNegative + numZero;                    
                if (numPositive == numTotal || numNegative == numTotal)
                    {
                    }
                else
                    smooth = false;
                }
            }
 
        for (size_t i = i0; i < i0 + numMatch; i++)
            {
            HalfEdge he = at (i);
            size_t readIndex = he.m_readIndex;
            ptrdiff_t signedVertexIndex = abs ((int)he.m_vertex0);
            if (smooth && !he.IsStrongVisible ())
                signedVertexIndex = - signedVertexIndex;
            if (signedVertexIndex > 0)
                numPositiveIndex++;
            else
                numNegativeIndex++;
            pointIndices[readIndex] = (int) signedVertexIndex;
            }
            
        }
    return true;
    }


//! Assume array is sorted.   Assume normals are available from the mesh.
//! Any edge with anything other than simple pairing becomes visible.
bool MarkTopologicalBoundariesVisible (PolyfaceHeaderR mesh, bool preservePriorVisibility)
    {
    size_t numHalfEdge = size ();
    size_t numChanged = 0;
    bvector<int> &pointIndices = mesh.PointIndex ();
    for (size_t i0 = 0, numMatch = 0; i0 < numHalfEdge; i0 += numMatch)
        {
        numMatch = 1;
        size_t numVisible = 0;
        if (at(i0).IsVisible ())
            numVisible = 1;
        for (size_t i1 = i0 + 1; i1 < numHalfEdge && !cb_LessThan_LowVertexHighVertex (at(i0), at(i1)); i1++)
            {
            numMatch++;
            if (at(i1).IsVisible ())
                numVisible++;
            }
        bool visible = true;
        if (numMatch == 2 && at(i0).IsReversedMateOf (at(i0 + 1)))
            {
            if (numVisible > 0 && preservePriorVisibility)
                visible = true;
            else
                visible = false;
            }
            
        for (size_t i = i0; i < i0 + numMatch; i++)
            {
            HalfEdge he = at (i);
            size_t readIndex = he.m_readIndex;
            ptrdiff_t signedVertexIndex = abs ((int)he.m_vertex0);
            if (!visible)
                signedVertexIndex = - signedVertexIndex;
            assert (abs ((int)signedVertexIndex) == abs ((int)pointIndices[readIndex]));
            if (pointIndices[readIndex] != signedVertexIndex)
                numChanged++;
            pointIndices[readIndex] = (int) signedVertexIndex;
            }            
        }
    return numChanged > 0;
    }

//! Return all edge coordinates annotated for origin and partner counts.
bool CollectEdgeMateData
(
PolyfaceHeaderR mesh,
bvector<FacetEdgeDetail> &segments,
bool includeMatched,
bool returnSingleEdgeReadIndex
)
    {
    size_t numHalfEdge = size ();
    size_t numChanged = 0;
    //bvector<int> &pointIndices = mesh.PointIndex ();
    bvector<DPoint3d> const &points = mesh.Point ();
    uint32_t clusterIndex = 0;
    for (size_t i0 = 0, numMatch = 0; i0 < numHalfEdge; i0 += numMatch)
        {
        numMatch = 1;
        size_t numVisible = 0;
        if (at(i0).IsVisible ())
            numVisible = 1;
        for (size_t i1 = i0 + 1; i1 < numHalfEdge && !cb_LessThan_LowVertexHighVertex (at(i0), at(i1)); i1++)
            {
            numMatch++;
            if (at(i1).IsVisible ())
                numVisible++;
            }

        if ((!includeMatched) && numMatch == 2 && at(i0).IsReversedMateOf (at(i0 + 1)))
            {
            // This is an interior edge.  It is neither counted nor recorded.
            }
        else
            {
            clusterIndex++;
            for (size_t i = i0; i < i0 + numMatch; i++)
                {
                HalfEdge he = at (i);
                size_t readIndex = he.m_readIndex;
                ptrdiff_t vertexIndex0 = abs ((int)he.m_vertex0) - 1;
                ptrdiff_t vertexIndex1 = abs ((int)he.m_vertex1) - 1;
                segments.push_back (
                    FacetEdgeDetail(
                        DSegment3d::From (points[vertexIndex0], points[vertexIndex1]),
                        readIndex,
                        (uint32_t)clusterIndex,
                        (uint32_t)numMatch
                        ));
                }
            }
        }
    return numChanged > 0;
    }

void ApplySignedVertex (MTGFacetsP facets, MTGNodeId nodeId, ptrdiff_t signedId, bool markPrimary)
    {
    int labelIndex = facets->vertexLabelOffset;
    MTGGraphP graph = jmdlMTGFacets_getGraph (facets);
    graph->TrySetLabel (nodeId, labelIndex, (int)(abs ((int)signedId) - 1));
    if (markPrimary)
        graph->SetMaskAt (nodeId, MTG_PRIMARY_EDGE_MASK);
    }
    
// @param [in] i half edge index
// @param [in] readIndexToHalfEdgePos reverse index
// @param [out] next index of half edge reached through m_faceSuccessorReadIndex

bool TryGetFSuccPos (bvector<size_t> &readIndexToHalfEdgePos, size_t i, size_t &next)
    {
    size_t readIndex;
    if (i < size ())
        {
        readIndex = at(i).m_faceSuccessorReadIndex;
        if (readIndex < readIndexToHalfEdgePos.size ())
            {
            next = readIndexToHalfEdgePos[readIndex];
            return next < SIZE_MAX;
            }
        }
    next = SIZE_MAX;
    return false;
    }
    
// Search within [i0..i1) for an index iC other than iA or iB that has opposite vertex indices from iA.
bool TryFindDistinctPartner (size_t iA, size_t iB, size_t i0, size_t i1, size_t &iC)
    {
    for (size_t i = i0; i < i1; i++)
        {
        if (i != iA && i != iB
            && at(i).IsReversedMateOf (at(iA)))
            {
            iC = i;
            return true;
            }            
        }
    iC = SIZE_MAX;
    return false;
    }

bool CreateEdgeForHalfEdgePair (MTGFacetsP facets, int readIndexLabelOffset, bvector<size_t> &readIndexToHalfEdgePos, size_t i0, size_t i1)
    {
    MTGNodeId leftNodeId, rightNodeId;
    MTGGraph *graph = jmdlMTGFacets_getGraph (facets);
    graph->CreateEdge (leftNodeId, rightNodeId);
    ApplySignedVertex (facets, leftNodeId, at(i0).m_vertex0, at(i0).m_visible);
    at(i0).m_nodeId = leftNodeId;
    ApplySignedVertex (facets, rightNodeId, at(i1).m_vertex0, at(i1).m_visible);
    at(i1).m_nodeId = rightNodeId;
    graph->TrySetLabel (leftNodeId, readIndexLabelOffset, (int)at(i0).m_readIndex);
    graph->TrySetLabel (rightNodeId, readIndexLabelOffset, (int)at(i1).m_readIndex);    
    return true;
    }
    
// Within a block of 4 contiguous half edges, look for a pair that are inside a degenerate triangle.
// if found, make the couplings
bool Couple4ThroughDegenerateTriangle (MTGFacetsP facets, int readIndexLabelOffset, bvector<size_t> &readIndexToHalfEdgePos, size_t i0)
    {
    size_t i1 = i0 + 4;
    for (size_t iA = i0; iA < i0 + 4; iA++)
        {
        size_t iB, iC, iD, iMateA, iMateC;
        if (   TryGetFSuccPos (readIndexToHalfEdgePos, iA, iB)
            && TryGetFSuccPos (readIndexToHalfEdgePos, iB, iC)
            && TryGetFSuccPos (readIndexToHalfEdgePos, iC, iD)
            && at(iB).IsSingleVertex ()
            && i0 <= iC && iC < i1
            && iD == iA
            )
            {
            // iA, iC are two of our 4.  Find distinct partners ..
            if (   TryFindDistinctPartner (iA, iC, i0, i1, iMateA)
                && TryFindDistinctPartner (iC, iA, i0, i1, iMateC)
                && iMateA != iMateC
                )
                {
                CreateEdgeForHalfEdgePair (facets, readIndexLabelOffset, readIndexToHalfEdgePos, iA, iMateA);
                CreateEdgeForHalfEdgePair (facets, readIndexLabelOffset, readIndexToHalfEdgePos, iC, iMateC);
                return true;
                }
            }
        }
    return false;
    }
    

// on input, halfEdgeArray has readIndex information from polyface.
bool BuildGraph (PolyfaceQueryCR polyface, MTGFacetsP facets)
    {

    jmdlMTGFacets_empty (facets);
    MTGGraphP graph = jmdlMTGFacets_getGraph (facets);
    jmdlMTGFacets_setNormalMode (facets, MTG_Facets_VertexOnly, 0, 0);
    int readIndexLabelOffset;
    if (!graph->TrySearchLabelTag (MTG_LABEL_TAG_POLYFACE_READINDEX, readIndexLabelOffset))
        readIndexLabelOffset = graph->DefineLabel (MTG_LABEL_TAG_POLYFACE_READINDEX, MTG_LabelMask_SectorProperty, -1);


    
    DPoint3dCP polyfaceXYZ = polyface.GetPointCP ();
    size_t n = polyface.GetPointCount ();
    for (size_t i = 0; i < n; i++)
        jmdlMTGFacets_addVertex (facets, &polyfaceXYZ[i], NULL);
    
    SortForEdgeMatching ();
    
    // For matched pairs, share a single mtg edge
    // For all others, make each edge the inside of an mtg edge.
    //    Mark both sides BOUNDARY.  Mark other side EXTERIOR.
    size_t numHalfEdge = size ();
    if (numHalfEdge < 1)
        return false;
    bvector<size_t> readPosToHalfEdgePos;
    size_t maxReadIndex = at(0).m_readIndex;
    for (size_t i = 0; i < numHalfEdge; i++)
        {
        if (at(i).m_readIndex > maxReadIndex)
            maxReadIndex = at(i).m_readIndex;
        if (at(i).m_faceSuccessorReadIndex > maxReadIndex)
            maxReadIndex = at(i).m_faceSuccessorReadIndex;
        }
    // Sanity check: readIndex is not "dense" -- there can be a zero at intervals of 4 or more.
    //if (3 * maxReadIndex > 4 * numHalfEdge)
    //    return false;
    readPosToHalfEdgePos.reserve (maxReadIndex + 1);
    for (size_t i = 0; i <= maxReadIndex; i++)
        readPosToHalfEdgePos.push_back (SIZE_MAX);
    for (size_t i = 0; i < numHalfEdge; i++)
        readPosToHalfEdgePos[at(i).m_readIndex] = i;
    
    for (size_t i0 = 0, numMatch = 0; i0 < numHalfEdge; i0 += numMatch)
        {
        numMatch = 1;
        for (size_t i1 = i0 + 1; i1 < numHalfEdge && !cb_LessThan_LowVertexHighVertex (at(i0), at(i1)); i1++)
            {
            numMatch++;
            }
        MTGNodeId leftNodeId, rightNodeId;
        if (numMatch == 2)
            {
            CreateEdgeForHalfEdgePair (facets, readIndexLabelOffset, readPosToHalfEdgePos, i0, i0 + 1);
            }
        else if (numMatch == 4 && Couple4ThroughDegenerateTriangle (facets, readIndexLabelOffset, readPosToHalfEdgePos, i0))
            {
            }
        else
            {
            for (size_t i = i0, i1 = i0 + numMatch; i < i1; i++)
                {
                graph->CreateEdge (leftNodeId, rightNodeId);
                // always visible for non-paired cases
                ApplySignedVertex (facets, leftNodeId, at(i).m_vertex0, true);
                ApplySignedVertex (facets, rightNodeId, at(i).m_vertex1, true);
                at(i).m_nodeId = leftNodeId;
                graph->SetMaskAt (leftNodeId, MTG_BOUNDARY_MASK);
                graph->SetMaskAt (rightNodeId, MTG_BOUNDARY_MASK);
                graph->SetMaskAt (rightNodeId, MTG_EXTERIOR_MASK);
                graph->TrySetLabel (leftNodeId, readIndexLabelOffset, (int)at(i).m_readIndex);
                if (at(i).IsSingleVertex ())
                    graph->SetMaskAt (rightNodeId, MTG_POLAR_LOOP_MASK);
                }
            }
        }

    for (size_t i = 0; i < numHalfEdge; i++)
        {
        size_t j = readPosToHalfEdgePos [at(i).m_faceSuccessorReadIndex];
        MTGNodeId myTailNode = at(i).m_nodeId;
        MTGNodeId successorTailNode = at(j).m_nodeId;
        MTGNodeId twistNode = graph->FSucc (myTailNode);
        graph->VertexTwist (twistNode, successorTailNode);
        }
    return true;
    }

static size_t FindIndexOfDiagonalCandidate(bvector<DPoint3d> const &points, double edgeLengthFactor)
    {
    size_t n = points.size();
    if (n < 3)
        return SIZE_MAX;
    if (edgeLengthFactor == 0.0)
        return SIZE_MAX;
    double maxLength = points.back().Distance (points.front());
    size_t iMax = n - 1;
    for (size_t i = 0; i + 1 < n; i++)
        {
        double length = points[i].Distance (points[i+1]);
        if (length > maxLength)
            {
            maxLength = length;
            iMax = i;
            }
        }
    for (size_t i = 0; i < n; i++)
        {
        size_t i1 = (i+1) % n;
        if (i != iMax && points[i].Distance(points[i1]) * edgeLengthFactor > maxLength)
            return SIZE_MAX;
        }
    return iMax;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
//! OPTIONAL
//! edgeLengthFactor -- only apply to edges that are at least this factor larger than others in the facet
//! maxPerFacet -- only apply in facets with this number or fewer vertices.
+--------------------------------------------------------------------------------------*/
void BuildArray(PolyfaceQueryCR source, bool omitInvisibles,
    bool ignoreDegenerateFaces = true,
    bool buildIndexToBaseReadIndex = false,
    double diagonalEdgeFactor = 0.0,
    uint32_t maxEdgesInFacetForDiagonalRules = 0
    )
    {
    static size_t indexShift = 1;
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (source, true);
    visitor->SetNumWrap (0);
    bvector <size_t> &indexPosition = visitor->IndexPosition ();
    bvector <int>    &clientPointIndex = visitor->ClientPointIndex ();

    if (buildIndexToBaseReadIndex)
        {
        m_readIndexToBaseReadIndex.resize (source.GetPointIndexCount ());
        for (auto &i: m_readIndexToBaseReadIndex)
            i = SIZE_MAX;
        }
    bvector <size_t> pack;
    for (visitor->Reset (); visitor->AdvanceToNextFace (); )
        {
        size_t numEdge = visitor->NumEdgesThisFace ();
        pack.clear ();
        for (size_t i = 0; i < numEdge; i++)
            pack.push_back (i);
        if (buildIndexToBaseReadIndex)
            {
            size_t baseReadIndex = visitor->GetReadIndex ();
            for (size_t i = 0; i < numEdge; i++)
                {
                size_t readIndex = indexPosition [i];
                m_readIndexToBaseReadIndex [readIndex] = baseReadIndex;
                }
            }
        if (ignoreDegenerateFaces)
            {
            // back up numEdge over trailing indices that match index 0...
            int pointIndex0 = clientPointIndex[0];
            while (numEdge > 1 && clientPointIndex[numEdge-1] == pointIndex0)
                numEdge--;
            // pack blocks of matching indices to singles ...
            size_t numAccept = 1;
            int pointIndexA = clientPointIndex[0];  // most recently accepted point index.
            for (size_t i = 1; i < numEdge; i++)
                {
                if (clientPointIndex[i] != pointIndexA)
                    {
                    pack[numAccept] = i;
                    pointIndexA = clientPointIndex[i];
                    numAccept++;
                    }
                }
            numEdge = numAccept;
            pack.resize (numAccept);
            if (numEdge == 2)
                continue;
            }
        size_t diagonalIndex= SIZE_MAX;
        if (numEdge <= maxEdgesInFacetForDiagonalRules)
            {
            diagonalIndex = FindIndexOfDiagonalCandidate (visitor->Point(), diagonalEdgeFactor);
            if (diagonalIndex == SIZE_MAX)
                diagonalIndex = numEdge + 1;
            }

        pack.push_back (pack[0]);
        for (size_t k = 0; k < numEdge; k++)
            {
            size_t i = pack[k];
            size_t j = pack[k+1];
            if (omitInvisibles && !visitor->Visible ()[i])
                {
                }
            else
                {
                bool strongVisible = false;
                if (diagonalIndex != SIZE_MAX && i != diagonalIndex)
                    strongVisible = true;
                Add (indexPosition[i], indexPosition[j],
                    (size_t)visitor->ClientPointIndex ()[i] + indexShift,
                    (size_t)visitor->ClientPointIndex ()[j] + indexShift,
                    visitor->Visible()[i], strongVisible);
                }
            }
        }
    }

size_t GetBaseReadIndex (size_t ri)
    {
    if (ri < m_readIndexToBaseReadIndex.size ())
        return m_readIndexToBaseReadIndex[ri];
    return SIZE_MAX;
    }

// ASSUME this HalfEdgeArray is sorted.
// ASSUME this HalfEdgeArray has m_readIndexToBaseReadIndex.
// Find edges which
// 1) appear as matched pairs.
// 2) have one active and one inactive.
// Note that the active index array will be sorted en process.
// Returned edges run in reverse of the active edge, then in the opposite direction along the other.
void ExtractPartnerEdges (bvector<size_t> &activeReadIndex, bvector<Size4> &quads)
    {
    quads.clear ();
    std::sort (activeReadIndex.begin (), activeReadIndex.end ());
    for (auto &he : *this)
        {
        size_t ri = he.m_readIndex;
        he.m_visible = std::binary_search (activeReadIndex.begin (), activeReadIndex.end (),
                GetBaseReadIndex (ri));
        }
    size_t numHalfEdge = size ();
    for (size_t i0 = 0, numMatch = 0; i0 < numHalfEdge; i0 += numMatch)
        {
        numMatch = 1;
        size_t last0 = SIZE_MAX;
        size_t last1 = SIZE_MAX;
        for (size_t i1 = i0 + 1; i1 < numHalfEdge && !cb_LessThan_LowVertexHighVertex (at(i0), at(i1)); i1++)
            {
            numMatch++;
            }
        for (size_t i = 0; i < numMatch; i++)
            {
            if (at(i0 + i).m_visible)
                last0 = i0 + i;
            else
                last1 = i0 + i;
            }
        if (last0 != SIZE_MAX && last1 != SIZE_MAX)
            {
            auto i0 = last0;
            auto i1 = last1;
            // exactly one of last0, last1 is visible . .
            if (at(last1).m_visible)
                std::swap (i0, i1);
            quads.push_back (
                Size4
                    (
                    at(last0).m_faceSuccessorReadIndex,
                    at(last0).m_readIndex,
                    at(last1).m_faceSuccessorReadIndex,
                    at(last1).m_readIndex
                    )
                );
            }
        }
    }
};

END_BENTLEY_GEOMETRY_NAMESPACE
