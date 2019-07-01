/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/


BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/**
Query object for indexing corners and faces of a trilinear box.
Integer face ids have the logical order
<ul>
<li>bottom
<li>top
<li>4 sides
</ul>
*/
struct BoxFaces
{
DPoint3d m_corners[8];

    struct FaceDirections
        {
        int axis[3];   // uvw direction index of first edge, second edge, noormal.
        int direction[3]; // + or - 1 for first edge, second edge, normal actual direction.
        };

//! Initialize from DgnBoxDetail.
void Load (DgnBoxDetailCR box);

//! Initialize from a range ..
void Load (DRange3dCR range);

// multiply the corners.  NO TEST FOR VOLUME INVERSION (negative determinant)
void MultiplyCorners (TransformCR transform);


//! Return the 4 points of a single face, plus 5th point wraparound.
void Get5PointCCWFace (int faceIndex, DPoint3d points[5]) const;

//! Return plane on specified face.  result is valid if the unit vector is normalized.
ValidatedDPlane3d GetFacePlane (int faceIndex) const;

//! test if all included angles are close to 90 degrees.
bool AreAllEdgesPerpendicular () const;

//! Return a face as DBilinearPatch3d
DBilinearPatch3d GetFace (int faceIndex) const;

//! Return a transform with origin at point 0, xyz columns along edges 01, 02,04
Transform GetBaseTransform () const;

//! Return x,y,z as longest edge length in respective direction.
DVec3d MaxEdgeLengths () const;

//! Return coordinatest of segments along all 12 edges.
//! segment[i] = coordinates of segment.
//! axisId[i] = 0,1,2 indicating x,y,z direction of segment
void GetEdgeSegments (bvector<DSegment3d> &segments, bvector<size_t> &axisId);

//! Return coordinates of loops around bottom and top in SS3 style.
//! segment[i] = coordinates of segment.
//! axisId[i] = 0,1,2 indicating x,y,z direction of segment
void GetAlignedCapLoops (bvector<DPoint3d> &point0, bvector<DPoint3d> &point1);

//! Get axis and orientation indexing.
FaceDirections GetFaceDirections (int faceIndex) const;

//! return the point and tangent vectors at u,v.
void Evaluate
(
int faceIndex,
double u,
double v,
DPoint3dR xyz,
DVec3dR   uTangent,
DVec3dR   vTangent
) const;

//! return the horizontal slice at height v.
void HorizontalSlice
(
int faceIndex,
double v,
DSegment3dR segment
) const;

//! return the vertical slice at u.
void VerticalSlice
(
int faceIndex,
double u,
DSegment3dR segment
) const;

//! Convert 2-part selector to single int face index.
static int SingleFaceIndex (int selector0, int selector1);

//! Convert 2-part selector to single int face index.
static bool TrySingleFaceIndex (SolidLocationDetail::FaceIndices const & indices, int &singleIndex);

/// Find the faceid on the other side of an edge..
//! @return partner face id.
//! @param [in] faceId start face.
//! @param [in] edgeId edge index, 0 to 4.
//! @param [out] partnerFaceId neighbor face id.  Negative if not present (i.e. uncapped.)
static int FaceIdEdgeIdToPartnerFaceId (int faceId, int edgeId);
//! Convert box face id to SolidLocationDetail form
static void ApplyIds (int faceIndex, SolidLocationDetail &detail);

static int FaceIdToPrimarySelector (int faceIndex);
static int FaceIdToSecondarySelector (int faceIndex);

//! Ask if faceIndex is a cap.
static bool IsCapFace (int faceIndex);

//! Return normalized parameter maps by face index.
//! side face X wraps 0..1 around all 4 faces, proportional to actual distance along lower edge.
//! top and bottom are 0..1, but bottom has reversed x

void GetSideWrappedFaceParameterMap (int faceIndex,
    DSegment1dR xMap, //!<Parameter space target range for 0..1 of this box face.
    DSegment1dR yMap,
    DRange2dR distanceRange,    //! total x and y distance range of all related faces (e.g. wraps)
    bool &endFace           //! true if this is the last face of a wrap set.
    ) const;

//! return true if the box corners have lefthanded ordering.
bool IsLeftHanded () const;

};
END_BENTLEY_GEOMETRY_NAMESPACE