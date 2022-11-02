/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//! Return the sum of areas as viewed with given direction toward the eye.
//! @param [in] vectorToEye vector towards eye.
//! @param [out] numPositive number of facets that face the eye.
//! @param [out] numPerpendicular number of facets that are perpendicular to the eye.
//! @param [out] numNegative number of facets that face away from the eye.
double PolyfaceQuery::SumDirectedAreas
(
DVec3dCR vectorToEye,
size_t &numPositive,
size_t &numPerpendicular,
size_t &numNegative) const
    {
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, false);
    visitor->SetNumWrap (0);
    double totalArea = 0.0;
    numPositive = numPerpendicular = numNegative = 0;
    DVec3d unitVector;
    unitVector.Normalize (vectorToEye);
    bvector<DPoint3d> &points = visitor->Point ();
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        DVec3d normal = PolygonOps::AreaNormal (points);
        double projectedArea = normal.DotProduct (unitVector);
        totalArea += projectedArea;
        if (normal.IsPerpendicularTo (vectorToEye))
            numPerpendicular++;
        else if (projectedArea > 0.0)
            numPositive++;
        else // projected area negative
            numNegative++;
        }
    return totalArea;
    }

//! Return the sum of areas as viewed with given direction toward the eye.
//! @param [in] vectorToEye vector towards eye.
//! @param [out] numPositive number of facets that face the eye.
//! @param [out] numPerpendicular number of facets that are perpendicular to the eye.
//! @param [out] numNegative number of facets that face away from the eye.
//! @param [out] forwardProjectedSum sum of forward facing areas (positive)
//! @param [out] reverseProjectedSum sum of rear facing areas (positive)
//! @param [out] forwardAbsoluteAreaSum sum of absolute area of forward facing facets.
//! @param [out] reverseAbsoluteSum sum of absolute area of rear facing facets.
//! @param [out] perpendicularAbsoluteSum sum of side facing areas.
//! @returns difference of forward and reverse projected areas.
double PolyfaceQuery::SumDirectedAreas
(
DVec3dCR vectorToEye,
size_t &numPositive,
size_t &numPerpendicular,
size_t &numNegative,
double &forwardProjectedSum,
double &reverseProjectedSum,
double &forwardAbsoluteSum,
double &reverseAbsoluteSum,
double &perpendicularAbsoluteSum,
double &volume
) const
    {
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, false);
    visitor->SetNumWrap (0);
    double totalArea = 0.0;
    perpendicularAbsoluteSum = 0.0;
    forwardProjectedSum = reverseProjectedSum = forwardAbsoluteSum = reverseAbsoluteSum = 0.0;
    numPositive = numPerpendicular = numNegative = 0;
    DVec3d unitVector;
    unitVector.Normalize (vectorToEye);
    bvector<DPoint3d> &points = visitor->Point ();
    DPoint3d origin = GetPointCP()[0];
    volume = 0.0;
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        DPoint3d centroid;
        double a;
        DVec3d unitNormal;
        PolygonOps::CentroidNormalAndArea (points, centroid, unitNormal, a);
        double projectedArea = a * unitNormal.DotProduct (unitVector);
        totalArea += projectedArea;
        volume += projectedArea * centroid.DotDifference (origin, vectorToEye);
        if (unitNormal.IsPerpendicularTo (vectorToEye))
            {
            perpendicularAbsoluteSum += a;
            numPerpendicular++;
            }
        else if (projectedArea > 0.0)
            {
            numPositive++;
            forwardProjectedSum += projectedArea;
            forwardAbsoluteSum += a;
            }
        else // projected area negative
            {
            numNegative++;
            reverseProjectedSum -= projectedArea;
            reverseAbsoluteSum  += a;
            }
        }
    return totalArea;
    }

//! Return the sum of areas as viewed with given direction toward the eye.
//! @param [in] vectorToEye vector towards eye.
//! @param [out] numPositive number of facets that face the eye.
//! @param [out] numPerpendicular number of facets that are perpendicular to the eye.
//! @param [out] numNegative number of facets that face away from the eye.
//! @param [out] forwardProjectedSum sum of forward facing areas (positive)
//! @param [out] reverseProjectedSum sum of rear facing areas (positive)
//! @param [out] forwardAbsoluteAreaSum sum of absolute area of forward facing facets.
//! @param [out] reverseAbsoluteSum sum of absolute area of rear facing facets.
//! @param [out] perpendicularAbsoluteSum sum of side facing areas.
//! @returns difference of forward and reverse projected areas.
double PolyfaceQuery::SumDirectedAreas
(
DVec3dCR vectorToEye,
size_t &numPositive,
size_t &numPerpendicular,
size_t &numNegative,
double &forwardProjectedSum,
double &reverseProjectedSum,
double &forwardAbsoluteSum,
double &reverseAbsoluteSum,
double &perpendicularAbsoluteSum
) const
    {
    double volume;
    return SumDirectedAreas (vectorToEye, numPositive, numPerpendicular, numNegative,
                forwardProjectedSum, reverseProjectedSum,
                forwardAbsoluteSum, reverseAbsoluteSum,
                perpendicularAbsoluteSum,
                volume);
    }


/*--------------------------------------------------------------------------------**//**
Pair of integers with lexical sort comparison.
+--------------------------------------------------------------------------------------*/
struct IndexPair
{
int m_i0, m_i1;
IndexPair (int i0, int i1) : m_i0(i0), m_i1(i1) {}

IndexPair FromOneBasedIndices (int i0, int i1){ return IndexPair (abs (i0) - 1, abs (i1) - 1);}
int LowIndex ()  const { return m_i0 < m_i1 ? m_i0 : m_i1;}
int HighIndex () const { return m_i0 < m_i1 ? m_i1 : m_i0;}

int Index0 () const { return m_i0;}
int Index1 () const { return m_i1;}
bool operator < (IndexPair const &other) const
  {
  int myLow = LowIndex ();
  int otherLow = other.LowIndex ();
  if (myLow < otherLow)
      return true;
  if (otherLow < myLow)
      return false;
  return HighIndex () < other.HighIndex ();
  }

bool IsSameDirectedPair (IndexPair const &other ) const
    {
    return m_i0 == other.m_i1 && m_i1 == other.m_i0;
    }

bool IsSameUndirectedPair (IndexPair const &other ) const
  {
  return LowIndex () == other.LowIndex ()
      && HighIndex () == other.HighIndex ();
  }
};

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool  PolyfaceHeader::SweepToSolid
(
DVec3dCR sweepVector,
bool triangulateSides
)
    {
    // !!! normals, params, colors not copied !!!
    ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    ClearNormals (false);
    ClearParameters (false);
    // hm .. no ClearColors method??
    size_t numPositive, numNegative, numSide;
    bool isSimpleSweep = true;
    double totalArea = SumDirectedAreas (sweepVector, numPositive, numSide, numNegative);
    if (totalArea == 0.0)
        isSimpleSweep = false;
    if (numSide != 0)
        isSimpleSweep = false;
    if (numPositive > 0 && numNegative > 0)
        isSimpleSweep = false;

    if (numPositive > 0)
        ReverseIndicesAllFaces ();

    // Make a complete shifted copy of the points ...
    bvector<DPoint3d> &points = Point ();
    bvector<int> &pointIndex = PointIndex ();
    size_t originalVertexCount = points.size ();
    for (size_t i = 0; i < originalVertexCount; i++)
        {
        DPoint3d newXYZ = DPoint3d::FromSumOf (points[i], sweepVector);
        points.push_back (newXYZ);
        }

    // for each original facet, make another

    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, true);
    BlockedVectorIntR clientPointIndex = visitor->ClientPointIndex ();
    //BlockedVectorIntR clientParamIndex = visitor->ClientParamIndex ();
    //BlockedVectorIntR clientColorIndex = visitor->ClientColorIndex ();
    auto &visible = visitor->Visible ();
    size_t originalIndexCount = PointIndex ().size ();
    bvector<IndexPair> edges;
    visitor->SetNumWrap (1);
    for (visitor->Reset ();
        visitor->AdvanceToNextFace () && visitor->GetReadIndex () < originalIndexCount;
        )
        {
        // Walk the facet in reverse order.  indices come from higher position in the visitor index arrays,
        //   but visibility comes from lower.
        for (size_t i1 = (size_t)visitor->NumEdgesThisFace (); i1 > 0; i1--)
            {
            size_t i0 = i1 - 1;
            edges.push_back (IndexPair (clientPointIndex[i1], clientPointIndex[i0]));
            int newPointIndex = 1 + clientPointIndex[i1] + (int)originalVertexCount;
            if (!visible[i0])
                newPointIndex = - newPointIndex;
            PointIndex().push_back (newPointIndex);
            // param index?
            // color index?
            // normal index?
            }
        pointIndex.push_back (0);   // terminator!!
        }


    std::sort (edges.begin (), edges.end ());

    // create new side facet for each simple boundary edge.
    // Also count mated pairs and everything unmated.
    size_t numEdges = edges.size ();
    size_t numSimpleBoundary = 0;
    size_t numMatedPair = 0;
    size_t numUnMatedCoedge = 0;
    for (size_t i0 = 0, i1 = 0; i0 < numEdges; i0 = i1)
        {
        size_t numUndirectedMatch = 1;
        for (i1 = i0 + 1; i1 < numEdges && edges[i0].IsSameUndirectedPair (edges[i1]);)
            {
            i1++;
            numUndirectedMatch++;
            }
        if (numUndirectedMatch == 1)
            {
            numSimpleBoundary++;
            // "use" the edge from the other side..
            int indexA0 = 1 + edges[i0].Index1 ();  // A0..A1 is CCW from above on the top surfaces
            int indexA1 = 1 + edges[i0].Index0 ();
            int indexB0 = indexA0 + (int)originalVertexCount;
            int indexB1 = indexA1 + (int)originalVertexCount;
            if (triangulateSides)
                {
                pointIndex.push_back (indexA0);
                pointIndex.push_back (indexB0);
                pointIndex.push_back (indexB1);
                pointIndex.push_back (0); // terminator!!
                pointIndex.push_back (indexA0);
                pointIndex.push_back (indexB1);
                pointIndex.push_back (indexA1);
                pointIndex.push_back (0); // terminator!!
                }
            else
                {
                pointIndex.push_back (indexA0);
                pointIndex.push_back (indexB0);
                pointIndex.push_back (indexB1);
                pointIndex.push_back (indexA1);
                pointIndex.push_back (0); // terminator!!
                }
            }
        else if (numUndirectedMatch == 2 && edges[i0].IsSameDirectedPair (edges[i1 - 1]))
            {
            numMatedPair++;
            }
        else
            {
            numUnMatedCoedge++;
            isSimpleSweep = false;
            }
        }

    return isSimpleSweep;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
