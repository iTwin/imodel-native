/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/SolidPrimitive/BoxFaces.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "BoxFaces.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//
//      2------------------3
//      | \     F4       / |
//      |   6----------7   |
//      |   |          |   |   (BOTTOM = F0)
//      |F5 |   F1     |F3 |
//      |   |          |   |
//      |   4----------5   |
//      | /     F2       \ |
//      0------------------1
//

static int s_cornerIndexCCW[6][4] =
{
    {1,0,2,3},
    {4,5,7,6},
    {0,1,5,4},
    {1,3,7,5},
    {3,2,6,7},
    {2,0,4,6}
};
static int s_partnerFace[6][6] =
{
    {5,4,3,2},
    {2,3,4,5},
    {0,3,1,5},
    {0,4,1,2},
    {0,5,1,3},
    {0,2,1,4},
};

static int s_faceId[6][2] = {
    {SolidLocationDetail::PrimaryIdCap, 0},
    {SolidLocationDetail::PrimaryIdCap, 1},
    {0, 0},
    {0, 1},
    {0, 2},
    {0, 3}
    };

static BoxFaces::FaceDirections s_faceDirections[6] =
    {
    {{0,1,2},{-1,1,-1}},
    {{0,1,2},{1,1,1}},
    {{0,2,1},{1,-1,1}},
    {{1,2,0},{1,1,1}},
    {{0,2,1},{-1,1,1}},
    {{1,2,0},{-1,1,-1}}
    };
static int s_axisEdgeVertex[3][4][2] = 
{
    {{0,1},{2,3},{4,5},{6,7}},
    {{0,2},{1,3},{4,6},{5,7}},
    {{0,4},{1,5},{2,6},{3,7}}
};

DVec3d BoxFaces::MaxEdgeLengths () const
    {
    DVec3d abc;
    abc.Zero ();    // not needed, but compiler can't tell the i loop safely addresses x,y,z in due time.
    for (int i = 0; i <3; i ++)
        {
        double a = 0.0;
        for (int j = 0; j < 4; j++)
            {
            a = DoubleOps::Max (a,
                m_corners[s_axisEdgeVertex[i][j][0]].Distance (
                            m_corners[s_axisEdgeVertex[i][j][1]]));
            }
        abc.SetComponent (a, i);
        }
    return abc;
    };

Transform BoxFaces::GetBaseTransform () const
    {
    DVec3d xVector = DVec3d::FromStartEnd (m_corners[0], m_corners[1]);
    DVec3d yVector = DVec3d::FromStartEnd (m_corners[0], m_corners[2]);
    DVec3d zVector = DVec3d::FromStartEnd (m_corners[0], m_corners[4]);
    Transform result;
    result.InitFromOriginAndVectors (m_corners[0], xVector, yVector, zVector);
    return result;
    }
static int s_classicCapLoop[2][4] =
    {
    {0,1,3,2},
    {4,5,7,6},
    };
void BoxFaces::GetAlignedCapLoops (bvector<DPoint3d> &point0, bvector<DPoint3d> &point1)
    {
    for (size_t i = 0; i < 4; i++)
      point0.push_back (m_corners[s_classicCapLoop[0][i]]);
    for (size_t i = 0; i < 4; i++)
      point1.push_back (m_corners[s_classicCapLoop[1][i]]);
    }
void BoxFaces::GetEdgeSegments (bvector<DSegment3d> &segment, bvector<size_t> &axisId)
    {
    segment.clear ();
    for (int axis = 0; axis < 3; axis++)
        {
        for (int edge = 0; edge < 4; edge++)
            {
            segment.push_back (DSegment3d::From (
                m_corners[s_axisEdgeVertex[axis][edge][0]],
                m_corners[s_axisEdgeVertex[axis][edge][1]]
                ));
            axisId.push_back (axis);
            }
        }
    }
int BoxFaces::FaceIdEdgeIdToPartnerFaceId (int faceId, int edgeId)
    {
    faceId %= 6;
    edgeId %= 4;
    return s_partnerFace[faceId][edgeId];
    }
int BoxFaces::FaceIdToPrimarySelector (int faceId)
    {
    faceId %= 6;
    return s_faceId[faceId][0];
    }
int BoxFaces::FaceIdToSecondarySelector (int faceId)
    {
    faceId %= 6;
    return s_faceId[faceId][1];
    }

//! Get axis and orientation indexing.
BoxFaces::FaceDirections BoxFaces::GetFaceDirections (int faceId) const
    {
    faceId %= 6;
    return s_faceDirections[faceId];    
    }
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void BoxFaces::Load (DgnBoxDetailCR box)
    {
    box.GetCorners (m_corners);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void BoxFaces::Load (DRange3dCR range)
    {
    range.Get8Corners (m_corners);
    }

void BoxFaces::MultiplyCorners (TransformCR transform)
    {
    transform.Multiply (m_corners, 8);
    }
bool BoxFaces::IsLeftHanded () const
    {
    return m_corners[0].TripleProductToPoints (m_corners[1], m_corners[2], m_corners[4]) < 0.0;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void BoxFaces::Get5PointCCWFace (int faceIndex, DPoint3d points[5]) const
    {
    faceIndex = faceIndex % 6;
    for (int k = 0; k < 4; k++)
        points[k] = m_corners[s_cornerIndexCCW[faceIndex][k]];
    points[4] = points[0];
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ValidatedDPlane3d BoxFaces::GetFacePlane (int faceIndex) const
    {
    faceIndex = faceIndex % 6;
    DPoint3d origin = m_corners[s_cornerIndexCCW[faceIndex][0]];
    DPoint3d xTarget = m_corners[s_cornerIndexCCW[faceIndex][1]];
    DPoint3d yTarget = m_corners[s_cornerIndexCCW[faceIndex][2]];
    ValidatedDVec3d normal = DVec3d::FromCrossProductToPoints (origin, xTarget, yTarget).ValidatedNormalize ();
    return ValidatedDPlane3d (DPlane3d::FromOriginAndNormal (origin, normal), normal.IsValid ());
    }



bool BoxFaces::AreAllEdgesPerpendicular () const
    {
    DPoint3d facePoints[5];
    for (int i = 0; i < 6; i++)
        {
        Get5PointCCWFace (i,facePoints);
        if (!PolylineOps::Are4EdgesPerpendicular (facePoints))
            return false;
        }
    return true;
    }

void BoxFaces::GetSideWrappedFaceParameterMap (int faceIndex, DSegment1dR xMap,  DSegment1dR yMap, DRange2dR distanceRange, bool &endFace) const
    {
    faceIndex = faceIndex % 6;
    xMap = DSegment1d (0.0, 1.0);
    yMap = DSegment1d (0.0, 1.0);
    // All faces are 0..1 Y.  Side X wraps.
    endFace = true;
    double distanceX, distanceY;
    
    if (faceIndex == 0)
        {
        // Bottom.  X goes backwards
        xMap = DSegment1d (1,0);
        distanceX = m_corners[0].Distance (m_corners[1]);
        distanceY = m_corners[0].Distance (m_corners[2]);
        }
    else if (faceIndex == 1)
        {
        distanceX = m_corners[4].Distance (m_corners[5]);
        distanceY = m_corners[4].Distance (m_corners[6]);
        }
    else
        {
        double accumulatedDistance [5];
        accumulatedDistance[0] = 0.0;
        accumulatedDistance[1] = m_corners[0].Distance (m_corners[1]);
        accumulatedDistance[2] = accumulatedDistance[1] + m_corners[1].Distance (m_corners[3]);
        accumulatedDistance[3] = accumulatedDistance[2] + m_corners[3].Distance (m_corners[2]);
        accumulatedDistance[4] = accumulatedDistance[3] + m_corners[2].Distance (m_corners[0]);
        double a;
        if (DoubleOps::SafeDivide (a, 1.0, accumulatedDistance[4], 0.0))
            xMap = DSegment1d (
                    accumulatedDistance[faceIndex - 2] * a,
                    accumulatedDistance[faceIndex - 1] * a);
        distanceX = accumulatedDistance[4];
        distanceY = m_corners[0].Distance (m_corners[4]);
        endFace = faceIndex == 5;
        }
    distanceRange.low.Init (0,0);
    distanceRange.high.Init (distanceX, distanceY);
    }
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DBilinearPatch3d BoxFaces::GetFace (int faceIndex) const
    {
    faceIndex %= 6;
    return DBilinearPatch3d
         (
         m_corners[s_cornerIndexCCW[faceIndex][0]],
         m_corners[s_cornerIndexCCW[faceIndex][1]],
         m_corners[s_cornerIndexCCW[faceIndex][3]],
         m_corners[s_cornerIndexCCW[faceIndex][2]]
         );
    }
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void BoxFaces::Evaluate
(
int faceIndex,
double u,
double v,
DPoint3dR xyz,
DVec3dR   uTangent,
DVec3dR   vTangent
) const
    {
    DBilinearPatch3d patch = GetFace (faceIndex);
    patch.Evaluate (u, v, xyz, uTangent, vTangent);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void BoxFaces::HorizontalSlice
(
int faceIndex,
double v,
DSegment3dR segment
) const
    {
    DBilinearPatch3d patch = GetFace (faceIndex);
    segment.point[0] = patch.Evaluate (0.0, v);
    segment.point[1] = patch.Evaluate (1.0, v);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void BoxFaces::VerticalSlice
(
int faceIndex,
double u,
DSegment3dR segment
) const
    {
    DBilinearPatch3d patch = GetFace (faceIndex);
    segment.point[0] = patch.Evaluate (u, 0.0);
    segment.point[1] = patch.Evaluate (u, 1.0);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void BoxFaces::ApplyIds (int faceIndex, SolidLocationDetail &detail)
    {
    faceIndex = faceIndex % 6;
    detail.SetFaceIndices (s_faceId[faceIndex][0], s_faceId[faceIndex][1], 0);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool BoxFaces::IsCapFace (int faceIndex)
    {
    faceIndex = faceIndex % 6;
    return s_faceId[faceIndex][0] == SolidLocationDetail::PrimaryIdCap;
    }

//! Convert 2-part selector to single int face index.
int BoxFaces::SingleFaceIndex (int selector0, int selector1)
    {
    if (selector0 == SolidLocationDetail::PrimaryIdCap)
        return selector1 & 0x01;
    return 2 + (selector1 & 0x3);     // 4 values
    }
//! Convert 2-part selector to single int face index.
bool BoxFaces::TrySingleFaceIndex (SolidLocationDetail::FaceIndices const & indices, int &singleIndex)
    {
    singleIndex = -1;
    if (indices.IsCap0 ())
        {
        singleIndex = 0;
        }
    else if (indices.IsCap1 ())
        {
        singleIndex = 1;
        }
    else if (indices.Index0 () == 0)
        {
        ptrdiff_t index1 = indices.Index1 ();
        if (index1 >= 0 && index1 < 4)
            singleIndex =  (int)(2 + index1);
        }
    return singleIndex != -1;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
