/*-------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudDrawBuffer.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudSchemaInternal.h>

USING_NAMESPACE_BENTLEY_POINTCLOUDSCHEMA
USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

// Static buffer pools used to store channels data
#define VIEW_POOL_MAX_BUFFER_COUNT (20)
PointCloudRgbChannelPool        PointCloudRgbChannel::m_pool(VIEW_POOL_MAX_BUFFER_COUNT);
PointCloudXyzChannelPool        PointCloudXyzChannel::m_pool(VIEW_POOL_MAX_BUFFER_COUNT);
PointCloudIntensityChannelPool  PointCloudIntensityChannel::m_pool(VIEW_POOL_MAX_BUFFER_COUNT);
PointCloudNormalChannelPool     PointCloudNormalChannel::m_pool(VIEW_POOL_MAX_BUFFER_COUNT);
PointCloudByteChannelPool       PointCloudByteChannel::m_pool(VIEW_POOL_MAX_BUFFER_COUNT);

/*---------------------------------------------------------------------------------**//**
* PointCloudDrawBuffer
* NVI interface.
+---------------+---------------+---------------+---------------+---------------+------*/
/*---------------------------------------------------------------------------------**//**
* Need to implement the virtual destructor of interface so that we can use delete on PointCloudDrawBuffer
* @bsimethod                                    Simon.Normand                   03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
                        PointCloudDrawBuffer::~PointCloudDrawBuffer ()           { }
                        PointCloudDrawBuffer::PointCloudDrawBuffer()             { }
void                    PointCloudDrawBuffer::Deallocate()                       { _Deallocate(); }
uint32_t                PointCloudDrawBuffer::GetCapacity()                      { return _GetCapacity(); }
PointCloudRgbChannel*   PointCloudDrawBuffer::GetRgbChannel()                    { return _GetRgbChannel();  }
PointCloudXyzChannel*   PointCloudDrawBuffer::GetPointChannel()                  { return _GetPointChannel();}
void                    PointCloudDrawBuffer::SetNumPoints(uint32_t numPoints)     { _SetNumPoints(numPoints); }
void                    PointCloudDrawBuffer::InitFrom(uint32_t begin, uint32_t end, Transform const* pTransform, PointCloudDrawBuffer* source) 
                                                                                 { _InitFrom(begin, end, pTransform, source); }
void                    PointCloudDrawBuffer::ChangeCapacity(uint32_t capacity)    { _ChangeCapacity(capacity); }
void                    PointCloudDrawBuffer::SetIgnoreColor(bool ignoreColor)   { _SetIgnoreColor(ignoreColor); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t PointCloudDrawParams::AddRef ()
    { 
    return ++m_refCount; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t PointCloudDrawParams::Release ()
    {
    uint32_t retval = --m_refCount;
    if (0 == retval)
        {
        delete this;
        }
    return retval;
    }

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
void PointCloudDrawParams::_SetNumPoints (uint32_t numPoints)
    { 
    if (m_points.IsValid())
        m_points->SetSize(numPoints);
    if (m_colors.IsValid())
        m_colors->SetSize(numPoints);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudDrawParams::_ChangeCapacity(uint32_t capacity)
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
    m_refCount = 0;
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
* @bsimethod                                                    StephanePoulin  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudDrawParams::_InitFrom(uint32_t begin, uint32_t end, Transform const* pTransform, PointCloudDrawBuffer* source)
    {
    uint32_t count = end - begin;
    ValidateCapacity(count);
    SetNumPoints(count);

    if (source->GetPointChannel() && source->GetPointChannel()->GetChannelBuffer())
        {
        if (pTransform)
            pTransform->Multiply (m_points->GetChannelBuffer(), source->GetPointChannel()->GetChannelBuffer() + begin, count);
        else
            memcpy(m_points->GetChannelBuffer(), source->GetPointChannel()->GetChannelBuffer() + begin, count * sizeof(DPoint3d));
        }

    if (source->GetRgbChannel() && source->GetRgbChannel()->GetChannelBuffer())
        memcpy(m_colors->GetChannelBuffer(), source->GetRgbChannel()->GetChannelBuffer() + begin, count * sizeof(PointCloudColorDef));

    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudDrawParams* PointCloudDrawParams::Create(PointCloudXyzChannel* pXyzChannel, PointCloudRgbChannel* pRgbChannel)
    {
    return new PointCloudDrawParams(pXyzChannel, pRgbChannel);
    }
