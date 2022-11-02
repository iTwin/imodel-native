/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE



/*--------------------------------------------------------------------------------**//**
* @bsistruct
+--------------------------------------------------------------------------------------*/
struct RegionContainmentData
{
DPoint3d m_refPoint;
double m_area;
DPoint3d m_centroid;
ptrdiff_t m_index;
ptrdiff_t m_parentIndex;
size_t m_numChildren;

ICurvePrimitivePtr m_loop;  // Must be a ChildVector !!!

RegionContainmentData (DPoint3dCR refPoint, DPoint3dCR centroid, double area, size_t index, ICurvePrimitivePtr loop)
    {
    m_refPoint = refPoint;
    m_area = area;
    m_centroid = centroid;
    m_index = m_parentIndex = index;
    m_numChildren = 0;
    m_loop = loop;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveVectorCP ChildVectorCP(){ return m_loop->GetChildCurveVectorCP ();}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void AddTo(CurveVectorR vector) { vector.push_back (m_loop);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void EnforceLoopOrientationAndType(double s)
    {
    CurveVectorPtr loop = m_loop->GetChildCurveVectorP();
    if (loop != NULL)
        {
        if (m_area * s < 0.0 && loop->size () > 0)
            {
            loop->ReverseCurvesInPlace ();
            m_area = - m_area;
            }
        loop->SetBoundaryType (s > 0.0 ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Inner);            
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
static void SetChildCounts(bvector<RegionContainmentData>&data)
    {
    size_t n = data.size ();
    for (size_t i = 0; i < n; i++)
        data[i].m_numChildren = 0;
    for (size_t i = 0; i < n; i++)
        {
        size_t j = data[i].m_parentIndex;
        if (j != i && j != -1)
            data[j].m_numChildren++;
        }
    }
};


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool FixupParityStructure(CurveVectorR allLoops)
    {
    bvector <RegionContainmentData> regionData;
    for (size_t index = 0; index < allLoops.size (); index++)
        {
        CurveVectorCP childVector = allLoops[index]->GetChildCurveVectorCP ();
        if (childVector == NULL)
            return false;
        CurveVector::BoundaryType childType;
        if  (!  ( allLoops.GetChildBoundaryType (index, childType)
                && (childType  == CurveVector::BOUNDARY_TYPE_Inner
                    || childType == CurveVector::BOUNDARY_TYPE_Outer)
                )
            )
            return false;
        DPoint3d xyz;
        if (!childVector->GetStartPoint (xyz))
            return false;
        double area;
        DPoint3d centroid;
        if (!childVector->CentroidAreaXY (centroid, area))
            return false;
        regionData.push_back (RegionContainmentData (xyz, centroid, area, index, allLoops[index]));
        }
    // RULES:
    // A region contained in an ODD number of regions is a child of the smallest containing region.
    size_t numGlobalParent = 0;            
    size_t numRegions = regionData.size ();
    ptrdiff_t lastParentIndex = 0;
    for (size_t childIndex = 0; childIndex < numRegions; childIndex++)
        {
        size_t numContainingLoops = 0;
        ptrdiff_t smallestContainingLoopIndex = -1;
        regionData[childIndex].m_parentIndex = -1;
        // Count parents.   A parent (1) has larger area (2) contains my start point
        for (size_t parentIndex = 0; parentIndex < numRegions; parentIndex++)
            {
            if (parentIndex != childIndex && fabs (regionData[childIndex].m_area) < fabs (regionData[parentIndex].m_area))
                {
                if (smallestContainingLoopIndex == -1
                  || fabs (regionData[parentIndex].m_area) < fabs (regionData[smallestContainingLoopIndex].m_area))
                    {
                    CurveVector::InOutClassification c = regionData[parentIndex].ChildVectorCP()->PointInOnOutXY (regionData[childIndex].m_refPoint);
                    if (c == CurveVector::INOUT_In)
                        {
                        smallestContainingLoopIndex = parentIndex;
                        numContainingLoops++;
                        }
                    }
                }
            }
        if ((numContainingLoops & 0x01) == 1)
            {
            regionData[childIndex].m_parentIndex = smallestContainingLoopIndex;
            }
        else
            {
            numGlobalParent++;
            lastParentIndex = childIndex;
            }
        }


    if (numGlobalParent == 1)
        {
        allLoops.clear ();
        allLoops.SetBoundaryType (CurveVector::BOUNDARY_TYPE_ParityRegion);
        regionData[lastParentIndex].EnforceLoopOrientationAndType (1.0);
        regionData[lastParentIndex].AddTo(allLoops);
        for (size_t childIndex = 0; childIndex < numRegions; childIndex++)
            {
            if (childIndex != lastParentIndex)
                {
                regionData[childIndex].EnforceLoopOrientationAndType (-1.0);
                regionData[childIndex].AddTo (allLoops);
                }
            }
        }
    else
        {
        RegionContainmentData::SetChildCounts (regionData);
        allLoops.clear ();
        allLoops.SetBoundaryType (CurveVector::BOUNDARY_TYPE_UnionRegion);
        for (size_t parentIndex = 0; parentIndex < numRegions; parentIndex++)
            {
            if (regionData[parentIndex].m_parentIndex == -1)
                {
                regionData[parentIndex].EnforceLoopOrientationAndType (1.0);
                if (regionData[parentIndex].m_numChildren == 0)
                    {
                    // This is a standalone loop.
                    regionData[parentIndex].AddTo (allLoops);
                    }
                else
                    {
                    CurveVectorPtr parityRegion = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
                    regionData[parentIndex].EnforceLoopOrientationAndType (1.0);
                    regionData[parentIndex].AddTo (*parityRegion);
                    for (size_t childIndex = 0; childIndex < numRegions; childIndex++)
                        {
                        if (regionData[childIndex].m_parentIndex == parentIndex)
                            {
                            regionData[childIndex].EnforceLoopOrientationAndType (-1.0);
                            regionData[childIndex].AddTo (*parityRegion);
                            }
                        }
                    allLoops.push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*parityRegion));
                    }
                }
            }
        }
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveVector::FixupXYOuterInner (bool fullGeometryCheck)
    {
    CurveVector::BoundaryType btype = GetBoundaryType ();
    if (btype == CurveVector::BOUNDARY_TYPE_UnionRegion)
        {
        // highest level struture of union never changes.
        for (size_t i = 0, n = size (); i < n; i++)
            {
            CurveVectorP childVector = const_cast<CurveVectorP>(at(i)->GetChildCurveVectorCP ());
            if (childVector == NULL)
                return false;
            if (!childVector->FixupXYOuterInner (fullGeometryCheck))
                return false;
            }
        return true;
        }
    else
        {
        if (!fullGeometryCheck)
            {
            if (btype == BOUNDARY_TYPE_ParityRegion)
                return FixupParityStructure (*this);
            else if (btype == CurveVector::BOUNDARY_TYPE_Inner
                    || btype == CurveVector::BOUNDARY_TYPE_Outer)
                return true;
            else
                return false;
            }
        else
            {
            // full geometry intersection tests.
            if (   btype == BOUNDARY_TYPE_ParityRegion
                || btype == CurveVector::BOUNDARY_TYPE_Inner
                || btype == CurveVector::BOUNDARY_TYPE_Outer
               )
                {
                CurveVectorPtr fixup = AreaAnalysis (*this, AreaSelect_Parity, BoolSelect_Summed_Parity, false);

                if (!fixup.IsValid ())
                    return false;

                SwapContents (*fixup);

                return true;
                }
            else
                {
                return false;
                }
            }
        }
    }

END_BENTLEY_GEOMETRY_NAMESPACE
