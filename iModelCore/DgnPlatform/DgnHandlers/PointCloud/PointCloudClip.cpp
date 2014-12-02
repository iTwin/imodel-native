/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/PointCloud/PointCloudClip.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
OrientedBox::OrientedBox () 
    {
    m_xVec.init(1.0, 0.0, 0.0);
    m_yVec.init(0.0, 1.0, 0.0);
    m_zVec.init(0.0, 0.0, 1.0);
    m_origin.init(0.0, 0.0, 0.0);
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
    DVec3d xVec; xVec.sumOf (&m_xVec, (DVec3dCP)&m_origin);
    DVec3d yVec; yVec.sumOf (&m_yVec, (DVec3dCP)&m_origin);
    DVec3d zVec; zVec.sumOf (&m_zVec, (DVec3dCP)&m_origin);

    trn.multiply(&xVec);
    trn.multiply(&yVec);
    trn.multiply(&zVec);

    trn.multiply(&m_origin);

    m_xVec.differenceOf (&xVec, (DVec3dCP)&m_origin);
    m_yVec.differenceOf (&yVec, (DVec3dCP)&m_origin);
    m_zVec.differenceOf (&zVec, (DVec3dCP)&m_origin);
    }
