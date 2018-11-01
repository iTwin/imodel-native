/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/SolidPrimitive/sp_FaceIndices.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "BoxFaces.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

SolidLocationDetail::FaceIndices::FaceIndices (ptrdiff_t index0, ptrdiff_t index1, ptrdiff_t index2)
    : m_index0(index0), m_index1(index1), m_index2(index2)
    {
    }

SolidLocationDetail::FaceIndices::FaceIndices ()
    : m_index0(0), m_index1(0), m_index2(0)
    {
    }
SolidLocationDetail::FaceIndices SolidLocationDetail::FaceIndices::Cap0 ()
    {return FaceIndices (PrimaryIdCap, 0, 0);}

SolidLocationDetail::FaceIndices SolidLocationDetail::FaceIndices::Cap1 ()
    {return FaceIndices (PrimaryIdCap, 1, 0);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void SolidLocationDetail::FaceIndices::Set (ptrdiff_t index0, ptrdiff_t index1, ptrdiff_t index2)
    {
    m_index0 = index0;
    m_index1 = index1;
    m_index2 = index2;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void SolidLocationDetail::FaceIndices::SetCap0 ()
    {
    m_index0 = SolidLocationDetail::PrimaryIdCap;
    m_index1 = 0;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void SolidLocationDetail::FaceIndices::SetCap1 ()
    {
    m_index0 = SolidLocationDetail::PrimaryIdCap;
    m_index1 = 1;
    }



GEOMDLLIMPEXP ptrdiff_t SolidLocationDetail::FaceIndices::Index0 () const { return m_index0;}
GEOMDLLIMPEXP ptrdiff_t SolidLocationDetail::FaceIndices::Index1 () const { return m_index1;}
GEOMDLLIMPEXP ptrdiff_t SolidLocationDetail::FaceIndices::Index2 () const { return m_index2;}

GEOMDLLIMPEXP void SolidLocationDetail::FaceIndices::SetIndex0 (ptrdiff_t value) { m_index0 = value;}
GEOMDLLIMPEXP void SolidLocationDetail::FaceIndices::SetIndex1 (ptrdiff_t value) { m_index1 = value;}
GEOMDLLIMPEXP void SolidLocationDetail::FaceIndices::SetIndex2 (ptrdiff_t value) { m_index2 = value;}

GEOMDLLIMPEXP bool SolidLocationDetail::FaceIndices::IsCap0 () const { return m_index0 == SolidLocationDetail::PrimaryIdCap && m_index1 == 0;}
GEOMDLLIMPEXP bool SolidLocationDetail::FaceIndices::IsCap1 () const { return m_index0 == SolidLocationDetail::PrimaryIdCap && m_index1 == 1;}
GEOMDLLIMPEXP bool SolidLocationDetail::FaceIndices::IsCap  () const { return m_index0 == SolidLocationDetail::PrimaryIdCap;}
GEOMDLLIMPEXP bool SolidLocationDetail::FaceIndices::Is  (ptrdiff_t index0, ptrdiff_t index1) const
    { return m_index0 == index0 && m_index1 == index1;}
GEOMDLLIMPEXP bool SolidLocationDetail::FaceIndices::Is  (ptrdiff_t index0, ptrdiff_t index1, ptrdiff_t index2) const
    { return m_index0 == index0 && m_index1 == index1 && m_index2 == index2;}



END_BENTLEY_GEOMETRY_NAMESPACE


