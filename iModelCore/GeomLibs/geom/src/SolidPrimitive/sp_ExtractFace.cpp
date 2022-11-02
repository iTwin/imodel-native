/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static double s_poleLatitudeTolerance = 1.0e-12;

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DgnConeDetail::IsRealCap (int select01) const
    {
    if (!m_capped)
        return false;
    double r = select01 == 0 ? m_radiusA : m_radiusB;
    return r != 0.0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DgnSphereDetail::IsRealCap (int select01) const
    {
    if (!m_capped)
        return false;
    double theta = select01 == 0 ? m_startLatitude : (m_startLatitude + m_latitudeSweep);
    return fabs (theta) < Angle::PiOver2 () - s_poleLatitudeTolerance;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
static void BuildSweepFaceIndices
(
bvector<SolidLocationDetail::FaceIndices> &indices,
bool cap0,
size_t numLevel,
size_t numPerLevel,
bool cap1
)
    {
    indices.clear ();    
    if (cap0)
        indices.push_back (SolidLocationDetail::FaceIndices::Cap0 ());
    if (cap1)
        indices.push_back (SolidLocationDetail::FaceIndices::Cap1 ());
    for (size_t level = 0; level < numLevel; level++)
        {
        for (size_t face = 0; face < numPerLevel; face++)
            indices.push_back (SolidLocationDetail::FaceIndices((int)level, (int)face, 0));
        }
    }

// This must count curve primitives exactly as CurveVector::FindIndexedLeaf.
static void EnumerateRecursiveSweepFaceIndices
(
bvector<SolidLocationDetail::FaceIndices> &indices,
CurveVectorCP curves,
ptrdiff_t index0,
ptrdiff_t &index1      // FIRST index1 -- incremented through recursion
)
    {
    CurveVector::BoundaryType b = curves->GetBoundaryType ();
    size_t n = curves->size ();
    if (   b == CurveVector::BOUNDARY_TYPE_Outer
        || b == CurveVector::BOUNDARY_TYPE_Inner
        || b == CurveVector::BOUNDARY_TYPE_Open)
        {
        for (size_t i = 0; i < n; i++)
            {
            size_t numComponent = curves->at(i)->NumComponent ();
            for (size_t k = 0; k < numComponent; k++)
                indices.push_back (SolidLocationDetail::FaceIndices (index0, index1, (ptrdiff_t)k));
            index1++;
            }
        }
    else
        {
        for (size_t i = 0; i < n; i++)
            {
            CurveVectorCP child;
            if (NULL != (child = curves->at(i)->GetChildCurveVectorCP ()))
                {
                EnumerateRecursiveSweepFaceIndices (indices, child, index0, index1);
                }
            else
                {
                index1++;  // can a non-child appear?  Dunno.
                }
            }
        }
    }
    
// Conditionally add cap indices.
// indices for intermediate levels.
static void BuildSweepFaceIndices (
    bvector<SolidLocationDetail::FaceIndices> &indices,
    bool cap0,
    CurveVectorCP curve0,
    size_t numLevel,
    bool cap1,
    CurveVectorCP curve1
    )
    {
    indices.clear ();    
    if (cap0)
        indices.push_back (SolidLocationDetail::FaceIndices::Cap0 ());
    if (cap1)
        indices.push_back (SolidLocationDetail::FaceIndices::Cap1 ());
    if (curve0 != NULL)
        for (size_t level = 0; level < numLevel; level++)
            {
            ptrdiff_t index1 = 0;
            EnumerateRecursiveSweepFaceIndices (indices, curve0, level, index1);
            }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DgnConeDetail::GetFaceIndices
(
bvector<SolidLocationDetail::FaceIndices> &indices
) const
    {
    BuildSweepFaceIndices
        (
        indices,
        IsRealCap (0),
        1, 1,
        IsRealCap (1)
        );
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DgnSphereDetail::GetFaceIndices
(
bvector<SolidLocationDetail::FaceIndices> &indices
) const
    {
    BuildSweepFaceIndices
        (
        indices,
        IsRealCap (0),
        1, 1,
        IsRealCap (1)
        );
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DgnTorusPipeDetail::GetFaceIndices
(
bvector<SolidLocationDetail::FaceIndices> &indices
) const
    {
    BuildSweepFaceIndices
        (
        indices,
        m_capped,
        1, 1,
        m_capped
        );
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DgnBoxDetail::GetFaceIndices
(
bvector<SolidLocationDetail::FaceIndices> &indices
) const
    {
    BuildSweepFaceIndices
        (
        indices,
        m_capped,
        1, 4,
        m_capped
        );
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DgnExtrusionDetail::GetFaceIndices
(
bvector<SolidLocationDetail::FaceIndices> &indices
) const
    {
    BuildSweepFaceIndices (indices,
            m_capped, m_baseCurve.get (),
            1,
            m_capped, m_baseCurve.get ()
            );
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DgnRotationalSweepDetail::GetFaceIndices
(
bvector<SolidLocationDetail::FaceIndices> &indices
) const
    {
    bool fullRotation = Angle::IsFullCircle (m_sweepAngle);
    BuildSweepFaceIndices (indices,
            m_capped && !fullRotation, m_baseCurve.get (),
            1,
            m_capped && !fullRotation, m_baseCurve.get ()
            );
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DgnRuledSweepDetail::GetFaceIndices
(
bvector<SolidLocationDetail::FaceIndices> &indices
) const
    {
    BuildSweepFaceIndices (indices,
            m_capped, m_sectionCurves.front ().get (),
            m_sectionCurves.size () - 1,
            m_capped, m_sectionCurves.back ().get ()
            );
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
IGeometryPtr DgnConeDetail::GetFace (SolidLocationDetail::FaceIndices const &select) const
    {
    DEllipse3d ellipse;
    if (select.IsCap0 () && IsRealCap (0) && FractionToSection (0.0, ellipse))
        {
        CurveVectorPtr disk = CurveVector::CreateDisk (DEllipse3d::FromReversed (ellipse));
        return IGeometry::Create (disk);
        }
    else if (select.IsCap1 () && IsRealCap (1) && FractionToSection (1.0, ellipse))
        {
        CurveVectorPtr disk = CurveVector::CreateDisk (ellipse);
        return IGeometry::Create (disk);
        }
    else if (select.Is (0,0))
        {
        ISolidPrimitivePtr surface = ISolidPrimitive::CreateDgnCone (*this);
        surface->SetCapped (false);
        return IGeometry::Create (surface);
        }
    return NULL;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
IGeometryPtr DgnSphereDetail::GetFace (SolidLocationDetail::FaceIndices const &select) const
    {
    if (select.IsCap0 () && IsRealCap (0))
        {
        DEllipse3d ellipse = VFractionToUSectionDEllipse3d (0.0);
        CurveVectorPtr disk = CurveVector::CreateDisk (DEllipse3d::FromReversed (ellipse));
        return IGeometry::Create (disk);
        }
    else if (select.IsCap1 () && IsRealCap (1))
        {
        DEllipse3d ellipse = VFractionToUSectionDEllipse3d (1.0);
        CurveVectorPtr disk = CurveVector::CreateDisk (ellipse);
        return IGeometry::Create (disk);
        }
    else if (select.Is (0,0))
        {
        ISolidPrimitivePtr surface = ISolidPrimitive::CreateDgnSphere (*this);
        surface->SetCapped (false);
        return IGeometry::Create (surface);
        }
    return NULL;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
IGeometryPtr DgnTorusPipeDetail::GetFace (SolidLocationDetail::FaceIndices const &select) const
    {
    if (select.IsCap0 () && m_capped)
        {
        DEllipse3d ellipse = VFractionToUSectionDEllipse3d (0.0);
        CurveVectorPtr disk = CurveVector::CreateDisk (DEllipse3d::FromReversed (ellipse));
        return IGeometry::Create (disk);
        }
    else if (select.IsCap1 () && m_capped)
        {
        DEllipse3d ellipse = VFractionToUSectionDEllipse3d (1.0);
        CurveVectorPtr disk = CurveVector::CreateDisk (ellipse);
        return IGeometry::Create (disk);
        }
    else if (select.Is (0,0))
        {
        ISolidPrimitivePtr surface = ISolidPrimitive::CreateDgnTorusPipe (*this);
        surface->SetCapped (false);
        return IGeometry::Create (surface);
        }
    return NULL;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
IGeometryPtr DgnBoxDetail::GetFace (SolidLocationDetail::FaceIndices const &select) const
    {
    BoxFaces cornerData;
    int singleIndex;
    cornerData.Load (*this);
    DPoint3d facePoints[5];
    if (   BoxFaces::TrySingleFaceIndex (select, singleIndex))
        {
        cornerData.Get5PointCCWFace (singleIndex, facePoints);
        CurveVectorPtr polygon = CurveVector::CreateLinear (facePoints, 5, CurveVector::BOUNDARY_TYPE_Outer);
        return IGeometry::Create (polygon);
        }
    return NULL;
    }

static IGeometryPtr SimplestExtrusionOf (ICurvePrimitivePtr &base, DVec3dCR extrusionVector)
    {
    DSegment3d segment;
    DSegment3d segment1;
    if (base->TryGetLine (segment)
        || (base->TryGetSegmentInLineString (segment, 0) && ! base->TryGetSegmentInLineString (segment1, 1))
        )
        {
        DPoint3d points[5];
        points[0] = segment.point[0];
        points[1] = segment.point[1];
        points[2] = DPoint3d::FromSumOf (segment.point[1], extrusionVector);
        points[3] = DPoint3d::FromSumOf (segment.point[0], extrusionVector);
        points[4] = segment.point[0];
        CurveVectorPtr polygon = CurveVector::CreateLinear (points, 5, CurveVector::BOUNDARY_TYPE_Outer);
        return IGeometry::Create (polygon);
        }

    // fallthrough -- anything can be extruded.
    CurveVectorPtr pathB = CurveVector::Create (base, CurveVector::BOUNDARY_TYPE_Open);
    DgnExtrusionDetail detail (pathB, extrusionVector, false);
    return IGeometry::Create (ISolidPrimitive::CreateDgnExtrusion (detail));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
IGeometryPtr DgnExtrusionDetail::GetFace (SolidLocationDetail::FaceIndices const &select) const
    {
    if (select.IsCap0 () && m_capped)
        {
        return IGeometry::Create (m_baseCurve->CloneReversed ());
        }
    else if (select.IsCap1 () && m_capped)
        {
        CurveVectorPtr curve = m_baseCurve->Clone ();
        Transform transform = Transform::From (m_extrusionVector);
        curve->TransformInPlace (transform);
        return IGeometry::Create (curve);
        }
    else if (select.Index0 () == 0 && select.Index1 () >= 0)
        {
        ICurvePrimitivePtr curveA = m_baseCurve->FindIndexedLeaf ((size_t)select.Index1 ());
        if (curveA.IsValid ())
            {
            ICurvePrimitivePtr curveB = curveA->CloneComponent (select.Index2 ());
            // umm.... closure?
            return SimplestExtrusionOf (curveB, m_extrusionVector);
            }
        }
    return NULL;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
IGeometryPtr DgnRotationalSweepDetail::GetFace (SolidLocationDetail::FaceIndices const &select) const
    {
    if (select.IsCap0 () && m_capped)
        {
        return IGeometry::Create (m_baseCurve->CloneReversed ());
        }
    else if (select.IsCap1 () && m_capped)
        {
        CurveVectorPtr curve = m_baseCurve->Clone ();
        Transform transform;
        Transform derivativeTransform;
        if (GetVFractionTransform (1.0, transform, derivativeTransform))
            {
            curve->TransformInPlace (transform);
            return IGeometry::Create (curve);
            }
        }
    else if (select.Index0 () == 0 && select.Index1 () >= 0)
        {
        ICurvePrimitivePtr curveA = m_baseCurve->FindIndexedLeaf ((size_t)select.Index1 ());
        if (curveA.IsValid ())
            {
            ICurvePrimitivePtr curveB = curveA->CloneComponent (select.Index2 ());
            // umm.... closure?
            CurveVectorPtr pathB = CurveVector::Create (curveB, CurveVector::BOUNDARY_TYPE_Open);
            DgnRotationalSweepDetail detail = *this;
            detail.m_baseCurve = pathB;
            detail.m_capped = false;
            return IGeometry::Create (ISolidPrimitive::CreateDgnRotationalSweep (detail));
            }
        }
    return NULL;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
IGeometryPtr DgnRuledSweepDetail::GetFace (SolidLocationDetail::FaceIndices const &select) const
    {
    if (select.IsCap0 () && m_capped)
        {
        return IGeometry::Create (m_sectionCurves[0]->CloneReversed ());
        }
    else if (select.IsCap1 () && m_capped)
        {
        CurveVectorPtr lastSection = m_sectionCurves.back ()->Clone ();
        return IGeometry::Create (lastSection);
        }
    else if (select.Index1 () >= 0)
        {
        ptrdiff_t index0 = select.Index0 ();
        ptrdiff_t index1 = select.Index1 ();
        if (index0 >= 0 && (size_t)index0 < m_sectionCurves.size () - 1)
            {
            ICurvePrimitivePtr curveA = m_sectionCurves[index0]->FindIndexedLeaf ((size_t)index1);
            ICurvePrimitivePtr curveB = m_sectionCurves[index0+1]->FindIndexedLeaf ((size_t)index1);
            if (curveA.IsValid () && curveB.IsValid ())
                {
                ICurvePrimitivePtr curveA1 = curveA->CloneComponent (select.Index2 ());
                ICurvePrimitivePtr curveB1 = curveB->CloneComponent (select.Index2 ());
                // umm.... closure?
                CurveVectorPtr pathA1 = CurveVector::Create (curveA1, CurveVector::BOUNDARY_TYPE_Open);
                CurveVectorPtr pathB1 = CurveVector::Create (curveB1, CurveVector::BOUNDARY_TYPE_Open);
                DgnRuledSweepDetail detail (pathA1, pathB1, false);
                return IGeometry::Create (ISolidPrimitive::CreateDgnRuledSweep (detail));
                }
            }
        }
    return NULL;
    }

END_BENTLEY_GEOMETRY_NAMESPACE


