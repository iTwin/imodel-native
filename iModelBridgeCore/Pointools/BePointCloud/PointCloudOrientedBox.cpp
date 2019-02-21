/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudOrientedBox.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BePointCloudInternal.h>
#include "PointCloudOrientedBox.h"

USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
OrientedBox::OrientedBox () 
    {
    m_xVec.Init (1.0, 0.0, 0.0);
    m_yVec.Init (0.0, 1.0, 0.0);
    m_zVec.Init (0.0, 0.0, 1.0);
    m_origin.Init (0.0, 0.0, 0.0);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
OrientedBox::OrientedBox (DVec3d& xVec, DVec3d& yVec, DVec3d& zVec, DPoint3d& origin)
: m_xVec(xVec), m_yVec(yVec), m_zVec(zVec), m_origin(origin)
    {
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
OrientedBox::~OrientedBox () 
    {
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d const&   OrientedBox::GetXVec() const 
    {
    return m_xVec;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d const&   OrientedBox::GetYVec() const 
    {
    return m_yVec;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d const&   OrientedBox::GetZVec() const 
    {
    return m_zVec;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d const& OrientedBox::GetOrigin() const 
    {
    return m_origin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void OrientedBox::ApplyTransform(TransformCR trn)
    {
    Transform mat; mat.InitFromOriginAndVectors (m_origin, m_xVec, m_yVec, m_zVec);
    mat.InitProduct (trn, mat);
    mat.GetMatrixColumn (m_xVec, 0);
    mat.GetMatrixColumn (m_yVec, 1);
    mat.GetMatrixColumn (m_zVec, 2);
    mat.GetTranslation(m_origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
 void OrientedBox::Init(DVec3dCR xVec, DVec3dCR yVec, DVec3dCR zVec, DPoint3dCR origin)
     {
     m_xVec = xVec;
     m_yVec = yVec;
     m_zVec = zVec;
     m_origin = origin;
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void OrientedBox::ComputeCornersFromOrientedBox (DPoint3d corners[8], OrientedBox const& clipBox, bool ccwOrder)
    {
    DVec3d normal;
    normal.CrossProduct (*(&clipBox.GetXVec ()), *(&clipBox.GetYVec ()));

    double dot = normal.DotProduct (*(&clipBox.GetZVec ()));

    if (dot > 0)
        {
        corners[0] = clipBox.GetOrigin();
        corners[1].SumOf (corners[0],*(&clipBox.GetXVec()));
        if (ccwOrder)
            {
            corners[2].SumOf (corners[1],*(&clipBox.GetYVec()));
            corners[3].SumOf (corners[0],*(&clipBox.GetYVec()));
            }
        else
            {
            corners[2].SumOf (corners[0],*(&clipBox.GetYVec()));
            corners[3].SumOf (corners[1],*(&clipBox.GetYVec()));
            }

        corners[4].SumOf (corners[0],*(&clipBox.GetZVec()));
        corners[5].SumOf (corners[1],*(&clipBox.GetZVec()));
        corners[6].SumOf (corners[2],*(&clipBox.GetZVec()));
        corners[7].SumOf (corners[3],*(&clipBox.GetZVec()));
        }
    else
        {
        corners[0] = clipBox.GetOrigin();
        corners[1].SumOf (corners[0],*(&clipBox.GetYVec()));
        if (ccwOrder)
            {
            corners[2].SumOf (corners[1],*(&clipBox.GetXVec()));
            corners[3].SumOf (corners[0],*(&clipBox.GetXVec()));
            }
        else
            {
            corners[2].SumOf (corners[0],*(&clipBox.GetXVec()));
            corners[3].SumOf (corners[1],*(&clipBox.GetXVec()));
            }

        corners[4].SumOf (corners[0],*(&clipBox.GetZVec()));
        corners[5].SumOf (corners[1],*(&clipBox.GetZVec()));
        corners[6].SumOf (corners[2],*(&clipBox.GetZVec()));
        corners[7].SumOf (corners[3],*(&clipBox.GetZVec()));
        }
    }

