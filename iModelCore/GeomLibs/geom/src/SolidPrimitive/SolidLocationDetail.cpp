/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void SolidLocationDetail::Init ()
    {
    m_faceIndices.m_index0 = 0;
    m_faceIndices.m_index1 = 0;
    m_parentId = 0;
    m_parameter = 0.0;
    m_xyz.Zero ();
    m_uParameter = m_vParameter = 0.0;
    m_uDirection.Zero ();
    m_vDirection.Zero ();
    m_a = 0.0;
    }
//! Construct with m_parameter, no point.
SolidLocationDetail::SolidLocationDetail (int parentId, double s)
    {
    Init ();
    m_parentId = parentId;
    m_parameter = s;
    m_xyz.Zero ();
    }

//! Construct with m_parameter, no point.
SolidLocationDetail::SolidLocationDetail ()
    {
    Init ();
    }

//! Constructor with m_parameter and point.
SolidLocationDetail::SolidLocationDetail (int parentId, double s, DPoint3dCR xyz)
    {
    Init ();
    m_parentId = parentId;
    m_parameter = s;
    m_xyz = xyz;
    }
    
//! Constructor with m_parameter and point.
SolidLocationDetail::SolidLocationDetail (int parentId, double s, DPoint3dCR xyz, double u, double v, DVec3dCR uDirection, DVec3dCR vDirection)
    {
    Init ();
    m_parentId = parentId;
    m_parameter = s;
    m_xyz = xyz;
    m_uParameter = u;
    m_vParameter = v;
    m_uDirection = uDirection;
    m_vDirection = vDirection;
    }    
//! Set all face selectors
void SolidLocationDetail::SetFaceIndices (ptrdiff_t id0, ptrdiff_t id1, ptrdiff_t id2)
    {
    m_faceIndices = FaceIndices (id0, id1, id2);
    }

void SolidLocationDetail::SetFaceIndices01 (ptrdiff_t id0, ptrdiff_t id1)
    {
    m_faceIndices.SetIndex0 (id0);
    m_faceIndices.SetIndex1 (id1);
    }

//! Set both face selectors
void SolidLocationDetail::SetFaceIndices (SolidLocationDetail::FaceIndices const &indices)
    {
    m_faceIndices = indices;
    }

//! Set selectors for cap id (typically 0 or 1)
void SolidLocationDetail::SetCapSelector (int id)
    {
    SetFaceIndices (PrimaryIdCap, id, 0);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
int SolidLocationDetail::GetParentId () const  {return m_parentId;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void SolidLocationDetail::SetParentId (int id) {m_parentId = id;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
int SolidLocationDetail::GetPrimarySelector () const  {return (int)m_faceIndices.m_index0;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
int SolidLocationDetail::GetSecondarySelector () const {return (int)m_faceIndices.m_index1;}
SolidLocationDetail::FaceIndices SolidLocationDetail::GetFaceIndices () const { return m_faceIndices;}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint2d SolidLocationDetail::GetUV () const {return DPoint2d::From (m_uParameter, m_vParameter);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d SolidLocationDetail::GetUDirection () const {return m_uDirection;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d SolidLocationDetail::GetVDirection () const {return m_vDirection;}
//! Return the parameter along the pick ray.
double SolidLocationDetail::GetPickParameter () const {return m_parameter;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint3d SolidLocationDetail::GetXYZ () const {return m_xyz;}

//! Ask if the selectors identify a cap
bool SolidLocationDetail::IsCapSelect (int &capId) const
    {
    if (m_faceIndices.m_index0 == PrimaryIdCap)
        {
        capId = (int)m_faceIndices.m_index1;
        return true;
        }
    capId = 0;
    return false;
    }

//! Ask if a selector pair is the start cap.
GEOMDLLIMPEXP bool SolidLocationDetail::IsCap0 (int selector0, int selector1)
    {
    return selector0 == PrimaryIdCap && selector1 == 0;
    }

GEOMDLLIMPEXP bool SolidLocationDetail::IsCap1 (int selector0, int selector1)
    {
    return selector0 == PrimaryIdCap && selector1 == 1;
    }

GEOMDLLIMPEXP void SolidLocationDetail::SetUV (double u, double v, DVec3dCR uDirection, DVec3dCR vDirection)
    {
    m_uParameter = u;
    m_vParameter = v;
    m_uDirection = uDirection;
    m_vDirection = vDirection;
    }

GEOMDLLIMPEXP void SolidLocationDetail::SetUV (DPoint2dCR uv)
    {
    m_uParameter = uv.x;
    m_vParameter = uv.y;
    }

GEOMDLLIMPEXP void SolidLocationDetail::SetU (double u) {m_uParameter = u;}
GEOMDLLIMPEXP void SolidLocationDetail::SetV (double v) {m_vParameter = v;}
GEOMDLLIMPEXP void SolidLocationDetail::SetA (double a) {m_a = a;}

GEOMDLLIMPEXP double SolidLocationDetail::GetU () const {return m_uParameter;}
GEOMDLLIMPEXP double SolidLocationDetail::GetV () const {return m_vParameter;}
GEOMDLLIMPEXP double SolidLocationDetail::GetA () const {return m_a;}

GEOMDLLIMPEXP void SolidLocationDetail::SetXYZ (DPoint3dCR xyz)
    {
    m_xyz = xyz;
    }

GEOMDLLIMPEXP void SolidLocationDetail::SetUDirection (DVec3d dXdu)
    {
    m_uDirection = dXdu;
    }

GEOMDLLIMPEXP void SolidLocationDetail::SetVDirection (DVec3d dXdv)
    {
    m_vDirection = dXdv;
    }

GEOMDLLIMPEXP void SolidLocationDetail::SetPickParameter (double value)
    {
    m_parameter = value;
    }

GEOMDLLIMPEXP void SolidLocationDetail::TransformInPlace (TransformCR transform)
    {
    transform.Multiply (m_xyz);
    transform.MultiplyMatrixOnly (m_uDirection);
    transform.MultiplyMatrixOnly (m_vDirection);
    }

GEOMDLLIMPEXP bool SolidLocationDetail::MapPickParameterFractionToRange (DRange1dCR range)
    {
    return range.FractionToDouble (m_parameter, m_parameter, m_parameter);
    }

GEOMDLLIMPEXP bool SolidLocationDetail::UpdateIfSmallerA (SolidLocationDetailCR source)
    {
    if (source.m_a < m_a)
        {
        *this = source;
        return true;
        }
    return false;
    }


END_BENTLEY_GEOMETRY_NAMESPACE


