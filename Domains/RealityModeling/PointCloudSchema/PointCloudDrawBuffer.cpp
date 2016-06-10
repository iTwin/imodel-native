/*-------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudDrawBuffer.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudSchemaInternal.h>

USING_NAMESPACE_BENTLEY_POINTCLOUDSCHEMA
USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

#ifndef CREATE_STATIC_LIBRARIES  //&&MM conflict with the definition in .\Pointools\BePointCloud\PointCloudQueryBuffer.cpp 
// Static buffer pools used to store channels data
#define VIEW_POOL_MAX_BUFFER_COUNT (20)
PointCloudRgbChannelPool        PointCloudRgbChannel::m_pool(VIEW_POOL_MAX_BUFFER_COUNT);
PointCloudXyzChannelPool        PointCloudXyzChannel::m_pool(VIEW_POOL_MAX_BUFFER_COUNT);
PointCloudIntensityChannelPool  PointCloudIntensityChannel::m_pool(VIEW_POOL_MAX_BUFFER_COUNT);
PointCloudNormalChannelPool     PointCloudNormalChannel::m_pool(VIEW_POOL_MAX_BUFFER_COUNT);
PointCloudByteChannelPool       PointCloudByteChannel::m_pool(VIEW_POOL_MAX_BUFFER_COUNT);
#endif


#if defined (NEEDS_WORK_POINT_CLOUD)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudDrawParams::ValidateCapacity(uint32_t capacity)
    {
    if(m_points.IsValid())
        m_points->ValidateCapacity(capacity);
    else
        m_points = PointCloudXyzChannel::Create(capacity);

    if(m_colors.IsValid())
        m_colors->ValidateCapacity(capacity);
    else
        m_colors = PointCloudRgbChannel::Create(capacity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudDrawParams::SetNumPoints (uint32_t numPoints)
    { 
    if (m_points.IsValid())
        m_points->SetSize(numPoints);
    if (m_colors.IsValid())
        m_colors->SetSize(numPoints);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudDrawParams::ChangeCapacity(uint32_t capacity)
    {
    SetNumPoints(0);

    if (m_points.IsValid())
        m_points->ChangeCapacity(capacity);
    else
        m_points = PointCloudXyzChannel::Create(capacity);

    if (m_colors.IsValid())
        m_colors->ChangeCapacity(capacity);
    else
        m_colors = PointCloudRgbChannel::Create(capacity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudDrawParams::PointCloudDrawParams (PointCloudXyzChannel* pXyzChannel, PointCloudRgbChannel* pRgbChannel)
    {
    BeAssert(NULL != pXyzChannel); // Mandatory

    m_ignoreColor = false;
    m_points = pXyzChannel;
    m_colors = pRgbChannel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudDrawParams::~PointCloudDrawParams ()
    {
    m_points = NULL;
    m_colors = NULL;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<PointCloudDrawParams> PointCloudDrawParams::Create(PointCloudXyzChannel* pXyzChannel, PointCloudRgbChannel* pRgbChannel)
    {
    return new PointCloudDrawParams(pXyzChannel, pRgbChannel);
    }
#endif

