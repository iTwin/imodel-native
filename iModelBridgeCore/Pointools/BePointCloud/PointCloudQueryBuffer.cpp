/*-------------------------------------------------------------------------------------+
|
|     $Source: PointCloudQueryBuffer.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BePointCloudInternal.h"

USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

#define VIEW_POOL_MAX_BUFFER_COUNT (20)
#define PICK_POOL_MAX_BUFFER_COUNT (10)

template <> PointCloudRgbChannelPool        PointCloudRgbChannel::m_pool(VIEW_POOL_MAX_BUFFER_COUNT);
template <> PointCloudXyzChannelPool        PointCloudXyzChannel::m_pool(VIEW_POOL_MAX_BUFFER_COUNT);
template <> PointCloudIntensityChannelPool  PointCloudIntensityChannel::m_pool(VIEW_POOL_MAX_BUFFER_COUNT);
template <> PointCloudNormalChannelPool     PointCloudNormalChannel::m_pool(VIEW_POOL_MAX_BUFFER_COUNT);
template <> PointCloudByteChannelPool       PointCloudByteChannel::m_pool(VIEW_POOL_MAX_BUFFER_COUNT);

/*---------------------------------------------------------------------------------**//**
* IPointCloudQueryBuffers
+---------------+---------------+---------------+---------------+---------------+------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d*           IPointCloudQueryBuffers::GetXyzBuffer() const                   { return _GetXyzBuffer(); }
PointCloudColorDef* IPointCloudQueryBuffers::GetRgbBuffer() const                   { return _GetRgbBuffer(); }
int16_t*            IPointCloudQueryBuffers::GetIntensityBuffer() const             { return _GetIntensityBuffer(); }
FPoint3d*           IPointCloudQueryBuffers::GetNormalBuffer() const                { return _GetNormalBuffer(); }
unsigned char*      IPointCloudQueryBuffers::GetFilterBuffer() const                { return _GetFilterBuffer(); }
unsigned char*      IPointCloudQueryBuffers::GetClassificationBuffer() const        { return _GetClassificationBuffer(); }
void*               IPointCloudQueryBuffers::GetChannelBuffer(IPointCloudChannelP pChannel) const               { return _GetChannelBuffer(pChannel); }
uint32_t            IPointCloudQueryBuffers::GetNumPoints() const                   { return _GetNumPoints(); }
uint32_t            IPointCloudQueryBuffers::GetCapacity() const                    { return _GetCapacity(); }
void                IPointCloudQueryBuffers::AddChannel(PointCloudChannelId channelId)                          { _AddChannel(channelId); }
void                IPointCloudQueryBuffers::SetNumPoints(uint32_t numPoints)                                   { _SetNumPoints(numPoints); }
unsigned char*      IPointCloudQueryBuffers::CreatePointCloudSymbologyChannel(IPointCloudSymbologyChannel* symb){ return _CreatePointCloudSymbologyChannel(symb); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
FilterChannelEditorPtr IPointCloudQueryBuffers::GetFilterChannelEditor() const
    {
    if (NULL != GetFilterBuffer())
        return FilterChannelEditor::Create(GetFilterBuffer());

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IPointCloudQueryBuffers::IPointCloudQueryBuffers() 
    {
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IPointCloudQueryBuffers::~IPointCloudQueryBuffers()
    {
    }

/*---------------------------------------------------------------------------------**//**
* this function insures that we have all of the required buffers
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void combineFlags (uint32_t& channelFlags, uint32_t outFlags, uint32_t requiredFlags)
    {
    // if it outputs a buffer
    // and we want to use that buffer
    // then make sure we add all of the buffers it needs to set that output buffer
    if (outFlags & (uint32_t)PointCloudChannelId::Rgb && channelFlags & (uint32_t)PointCloudChannelId::Rgb)
        channelFlags |= requiredFlags;
    else if (outFlags & (uint32_t)PointCloudChannelId::Xyz && channelFlags & (uint32_t)PointCloudChannelId::Xyz)
        channelFlags |= requiredFlags;
    else if (outFlags & (uint32_t)PointCloudChannelId::Intensity && channelFlags & (uint32_t)PointCloudChannelId::Intensity)
        channelFlags |= requiredFlags;
    else if (outFlags & (uint32_t)PointCloudChannelId::Normal && channelFlags & (uint32_t)PointCloudChannelId::Normal)
        channelFlags |= requiredFlags;
    else if (outFlags & (uint32_t)PointCloudChannelId::Filter && channelFlags & (uint32_t)PointCloudChannelId::Filter)
        channelFlags |= requiredFlags;
    else if (outFlags & (uint32_t)PointCloudChannelId::Classification && channelFlags & (uint32_t)PointCloudChannelId::Classification)
        channelFlags |= requiredFlags;
    }

/*---------------------------------------------------------------------------------**//**
* PointCloudQueryBuffers
+---------------+---------------+---------------+---------------+---------------+------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudQueryBuffers::PointCloudQueryBuffers(uint32_t capacity, uint32_t channelFlags, PointCloudChannelVectorCR channels) 
    : m_numPoints(0), m_capacity(capacity), m_channelFlags(channelFlags), m_rgbOverriden(false)
    {
    if (m_capacity == 0)
        return;

    if (m_channelFlags & (uint32_t)PointCloudChannelId::Rgb)
        m_rgb = PointCloudRgbChannel::Create(m_capacity);
    if (m_channelFlags & (uint32_t)PointCloudChannelId::Xyz)
        m_xyz = PointCloudXyzChannel::Create(m_capacity);
    if (m_channelFlags & (uint32_t)PointCloudChannelId::Intensity)
        m_intensities = PointCloudIntensityChannel::Create(m_capacity);
    if (m_channelFlags & (uint32_t)PointCloudChannelId::Normal)
        m_normals = PointCloudNormalChannel::Create(m_capacity);
    if (m_channelFlags & (uint32_t)PointCloudChannelId::Filter)
        m_filters = PointCloudFilterChannel::Create(m_capacity);
    if (m_channelFlags & (uint32_t)PointCloudChannelId::Classification)
        m_classifications = PointCloudClassificationChannel::Create(m_capacity);

    if (!channels.empty())
        m_userChannelBuffers = UserChannelQueryBuffers::Create(capacity, channels);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudQueryBuffers::~PointCloudQueryBuffers()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t PointCloudQueryBuffers::_GetCapacity() const
    {
    return m_capacity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudQueryBuffers::_SetNumPoints(uint32_t numPoints)
    {
    BeAssert (numPoints <= m_capacity);

    m_numPoints = numPoints;
    if (GetXyzChannel()) 
        GetXyzChannel()->SetSize(numPoints);
    if (GetRgbChannel()) 
        GetRgbChannel()->SetSize(numPoints);
    if (GetIntensityChannel()) 
        GetIntensityChannel()->SetSize(numPoints);
    if (GetNormalChannel()) 
        GetNormalChannel()->SetSize(numPoints);
    if (GetFilterChannel()) 
        GetFilterChannel()->SetSize(numPoints);
    if (GetClassificationChannel()) 
        GetClassificationChannel()->SetSize(numPoints);

    for (PointCloudSymbologyChannelVector::iterator itr(m_symbologyChannels.begin()); itr != m_symbologyChannels.end(); ++itr)
        {
        if (itr->second.IsValid())
            itr->second->SetSize(numPoints);
        }
    if(m_userChannelBuffers.IsValid())
        m_userChannelBuffers->SetNumberOfPoints(numPoints);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudQueryBuffers::ChangeCapacity(uint32_t capacity)
    {
    if (GetXyzChannel()) 
        GetXyzChannel()->ChangeCapacity(capacity);
    if (GetRgbChannel()) 
        GetRgbChannel()->ChangeCapacity(capacity);
    if (GetIntensityChannel()) 
        GetIntensityChannel()->ChangeCapacity(capacity);
    if (GetNormalChannel()) 
        GetNormalChannel()->ChangeCapacity(capacity);
    if (GetFilterChannel()) 
        GetFilterChannel()->ChangeCapacity(capacity);
    if (GetClassificationChannel()) 
        GetClassificationChannel()->ChangeCapacity(capacity);

    for (PointCloudSymbologyChannelVector::iterator itr(m_symbologyChannels.begin()); itr != m_symbologyChannels.end(); ++itr)
        {
        if (itr->second.IsValid())
            itr->second->ChangeCapacity(capacity);
        }

    if (GetUserChannelBuffers())
        GetUserChannelBuffers()->ChangeCapacity(capacity);

    m_capacity = capacity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudSymbologyChannel::pointer PointCloudQueryBuffers::_CreatePointCloudSymbologyChannel(IPointCloudSymbologyChannel* symb)
    {
    RefCountedPtr<PointCloudSymbologyChannel> pChannel = PointCloudSymbologyChannel::Create(GetCapacity());

    // requires that we also have a filter channel to hide points
    if (!m_filters.IsValid())
        {
        m_filters = PointCloudFilterChannel::Create(m_capacity);

        //we never filled this buffer, mark all points as visible
        unsigned char shownPoint (0);
        PointCloudChannels_Show_Point (shownPoint);
        memset (m_filters->GetChannelBuffer(), shownPoint, pChannel->GetCapacity() * sizeof(unsigned char));
        }

    memset (pChannel->GetChannelBuffer(), 0, pChannel->GetCapacity() * sizeof(PointCloudSymbologyChannel::value_type));
    
    m_symbologyChannels.push_back(PointCloudSymbologyChannelVector::value_type(symb, pChannel));
    return pChannel->GetChannelBuffer(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void* PointCloudQueryBuffers::_GetChannelBuffer(IPointCloudChannelP pChannel) const
    {
    return m_userChannelBuffers.IsNull() ? NULL : m_userChannelBuffers->GetChannelBuffer(pChannel);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudQueryBuffersPtr PointCloudQueryBuffers::Create(uint32_t capacity, uint32_t channelFlags) 
    {
    PointCloudChannelVector channels;
    return new PointCloudQueryBuffers(capacity, channelFlags, channels); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudQueryBuffersPtr PointCloudQueryBuffers::Create(uint32_t capacity, uint32_t channelFlags, PointCloudChannelVectorCR channels) 
    {
    return new PointCloudQueryBuffers(capacity, channelFlags, channels); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudQueryBuffers::SetUserChannelBuffers(UserChannelQueryBuffers* buffers)
    {
    m_userChannelBuffers = buffers;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudQueryBuffers::RemoveHiddenPoints(bool updateFilterChannel)
    {
    PointCloudFilterChannel::pointer  pfilterChannel = GetFilterBuffer();
    if (NULL == pfilterChannel)
        return;

    const uint32_t numPoints = GetNumPoints();

    // find first hidden point
    uint32_t startIdx = 0;
    for(; startIdx != numPoints; ++startIdx)
        {
        if (PointCloudChannels_Is_Point_Hidden(pfilterChannel[startIdx]))
            break;
        }

    uint32_t ptIdx = startIdx;
    uint32_t dstIdx = startIdx;

    if (HasRgb() && HasXyz())
        {
        PointCloudRgbChannel::pointer   pRgbChannel = GetRgbBuffer();
        PointCloudXyzChannel::pointer pPointChannel = GetXyzBuffer();

        for(; ptIdx != numPoints; ++ptIdx)
            {
            if (PointCloudChannels_Is_Point_Visible(pfilterChannel[ptIdx]))
                {
                pPointChannel[dstIdx] = pPointChannel[ptIdx];
                pRgbChannel[dstIdx]   = pRgbChannel[ptIdx];
                ++dstIdx;
                }
            }
        }
    else if (HasXyz())
        {
        PointCloudXyzChannel::pointer pPointChannel = GetXyzBuffer();

        for(; ptIdx != numPoints; ++ptIdx)
            {
            if (PointCloudChannels_Is_Point_Visible(pfilterChannel[ptIdx]))
                {
                pPointChannel[dstIdx] = pPointChannel[ptIdx];
                ++dstIdx;
                }
            }
        }

    if (HasIntensity())
        {
        PointCloudIntensityChannel::pointer pBuf = GetIntensityBuffer();
        ptIdx = startIdx;
        dstIdx = startIdx;

        for(; ptIdx != numPoints; ++ptIdx)
            {
            if (PointCloudChannels_Is_Point_Visible(pfilterChannel[ptIdx]))
                {
                pBuf[dstIdx] = pBuf[ptIdx];
                ++dstIdx;
                }
            }
        }

    if (HasClassification())
        {
        PointCloudClassificationChannel::pointer pBuf = GetClassificationBuffer();
        ptIdx = startIdx;
        dstIdx = startIdx;

        for(; ptIdx != numPoints; ++ptIdx)
            {
            if (PointCloudChannels_Is_Point_Visible(pfilterChannel[ptIdx]))
                {
                pBuf[dstIdx] = pBuf[ptIdx];
                ++dstIdx;
                }
            }
        }

    if (HasNormal())
        {
        PointCloudNormalChannel::pointer pBuf = GetNormalBuffer();
        ptIdx = startIdx;
        dstIdx = startIdx;

        for(; ptIdx != numPoints; ++ptIdx)
            {
            if (PointCloudChannels_Is_Point_Visible(pfilterChannel[ptIdx]))
                {
                pBuf[dstIdx] = pBuf[ptIdx];
                ++dstIdx;
                }
            }
        }

    // Copy user channels
    if (GetUserChannelBuffers())
        {
        UserChannelQueryBuffers::UserChannelBuffersCR pointCloudChannelBuffers (GetUserChannelBuffers()->GetPointChannelBuffers());
        for (UserChannelQueryBuffers::UserChannelBuffers::const_iterator itr = pointCloudChannelBuffers.begin(); itr != pointCloudChannelBuffers.end(); ++itr)
            {
            ptIdx = startIdx;
            dstIdx = startIdx;

            int multiple = itr->first->GetMultiple();
            int typeSize = itr->first->GetTypeSize();
            int size = multiple * typeSize;

            unsigned char* pSrc = (unsigned char*)(itr->second->_GetBufferP());
            unsigned char* pDst = (unsigned char*)(itr->second->_GetBufferP());

            for(; ptIdx != numPoints; ++ptIdx)
                {
                if (PointCloudChannels_Is_Point_Visible(pfilterChannel[ptIdx]))
                    {
                    memcpy(pDst, &pSrc[ptIdx*size], size);
                    pDst +=size;
                    }
                }
            }
        }

    // Copy symbology channels
    PointCloudSymbologyChannelVectorCR symbChannels(GetPointCloudSymbologyChannelVector());
    for (PointCloudSymbologyChannelVector::const_iterator itr(symbChannels.begin()); itr != symbChannels.end(); ++itr)
        {
        PointCloudSymbologyChannel::pointer pSymbBuffer = itr->second->GetChannelBuffer();
        ptIdx = startIdx;
        dstIdx = startIdx;

        for(; ptIdx != numPoints; ++ptIdx)
            {
            if (PointCloudChannels_Is_Point_Visible(pfilterChannel[ptIdx]))
                {
                pSymbBuffer[dstIdx] = pSymbBuffer[ptIdx];
                ++dstIdx;
                }
            }
        }

    if (updateFilterChannel)
        {
        // update filter channel
        ptIdx = startIdx;
        dstIdx = startIdx;

        for(; ptIdx != numPoints; ++ptIdx)
            {
            if (PointCloudChannels_Is_Point_Visible(pfilterChannel[ptIdx]))
                {
                pfilterChannel[dstIdx] = (pfilterChannel[ptIdx] | 0x01); // Make visible
                ++dstIdx;
                }
            }
        }

    SetNumPoints(dstIdx);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudQueryBuffers::SetRgbOverrideColor (PointCloudColorDefCR overrideRgbColor)
    {
    RefCountedPtr<PointCloudRgbChannel> pRgbChannel = this->GetRgbChannel();
    if (!pRgbChannel.IsValid())
        return;


    pRgbChannel->Init(overrideRgbColor);
    m_rgbOverriden = true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t PointCloudQueryBuffers::GetPoints(PThandle queryHandle)
    {
    std::vector<PThandle> ptHandles;
    std::vector<void*> channelBuffers;

    if (GetUserChannelBuffers())
        {
        ptHandles      = GetUserChannelBuffers()->GetChannelHandles();
        channelBuffers = GetUserChannelBuffers()->GetChannelBuffers();
        }

    PThandle* pChannelHandles = ptHandles.empty() ? NULL : &ptHandles[0];
    void**    ppChannels      = channelBuffers.empty() ? NULL : &channelBuffers[0];
    uint32_t  numChannels     = (uint32_t)channelBuffers.size();

    uint32_t numPoints = ptGetDetailedQueryPointsd (queryHandle, m_capacity, (double*)GetXyzBuffer(), m_rgbOverriden ? NULL : (Byte*)GetRgbBuffer(), GetIntensityBuffer(), 
                                                          (float*)GetNormalBuffer(), GetFilterBuffer(), GetClassificationBuffer(), 
                                                          numChannels, pChannelHandles, ppChannels);

    SetNumPoints(numPoints);

    return numPoints;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudQueryBuffers::_AddChannel(PointCloudChannelId channelId)
    {
    if ((uint32_t)channelId & (uint32_t)PointCloudChannelId::Rgb && m_rgb.IsNull())
        m_rgb = PointCloudRgbChannel::Create(m_capacity);

    if ((uint32_t)channelId & (uint32_t)PointCloudChannelId::Xyz && m_xyz.IsNull())
        m_xyz = PointCloudXyzChannel::Create(m_capacity);

    if ((uint32_t)channelId & (uint32_t)PointCloudChannelId::Intensity && m_intensities.IsNull())
        m_intensities = PointCloudIntensityChannel::Create(m_capacity);

    if ((uint32_t)channelId & (uint32_t)PointCloudChannelId::Normal && m_normals.IsNull())
        m_normals = PointCloudNormalChannel::Create(m_capacity);

    if ((uint32_t)channelId & (uint32_t)PointCloudChannelId::Filter && m_filters.IsNull())
        m_filters = PointCloudFilterChannel::Create(m_capacity);

    if ((uint32_t)channelId & (uint32_t)PointCloudChannelId::Classification && m_classifications.IsNull())
        m_classifications = PointCloudClassificationChannel::Create(m_capacity);

    m_channelFlags |= (uint32_t)channelId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
PointChannelBuffer* PointChannelBuffer::Create(uint32_t capacity, uint32_t typeSize, uint32_t multiple)
    {
    return new PointChannelBuffer(capacity, typeSize, multiple);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
PointChannelBuffer::PointChannelBuffer(uint32_t capacity, uint32_t typeSize, uint32_t multiple) : m_typeSize(typeSize), m_multiple(multiple), m_capacity(capacity), m_count(0), m_buffer(NULL)
    {
    if (m_capacity * m_typeSize * m_multiple > 0)
        m_buffer = (Byte*)_Allocate(m_capacity * m_typeSize * m_multiple);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
PointChannelBuffer::~PointChannelBuffer()
    {
    _Deallocate(m_buffer, m_capacity * m_typeSize * m_multiple);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t PointChannelBuffer::GetTypeSize() const
    {
    return m_typeSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t PointChannelBuffer::GetMultiple() const
    {
    return m_multiple;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t PointChannelBuffer::_GetCapacity() const
    {
    return m_capacity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t PointChannelBuffer::_GetNumPoints() const
    {
    return m_count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void PointChannelBuffer::_SetNumPoints(uint32_t count)
    {
    BeAssert (count <= m_capacity);
    m_count = count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t PointChannelBuffer::GetBufferSize() const
    {
    return m_capacity * m_typeSize * m_multiple;
    }

/*---------------------------------------------------------------------------------**//**
@param  size IN Number of points
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void* PointChannelBuffer::_Allocate(size_t size)
    {
    return new Byte[size];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void  PointChannelBuffer::_Deallocate(Byte* p, size_t size)
    {
    delete[] p;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void* PointChannelBuffer::_GetBufferP() const
    {
    return m_buffer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Byte* PointChannelBuffer::GetValueP(uint32_t idx) const
    {
    return m_buffer + (idx * m_typeSize * m_multiple);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void  PointChannelBuffer::SetValue(Byte* value, uint32_t idx)
    {
    memcpy(GetValueP(idx), value, m_typeSize * m_multiple);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudChannelSymbology : public IPointCloudSymbologyChannel
    {
    private:
                 PointCloudChannelSymbology() : m_pointSize(0) {}
        virtual ~PointCloudChannelSymbology() {}


        // IPointCloudSymbology
        virtual uint32_t _GetPointSize() const override       { return m_pointSize; }
        virtual void   _SetPointSize(uint32_t size) override  { m_pointSize = size; }

    private:
        uint32_t m_pointSize;

    public:
        static PointCloudChannelSymbology* Create() { return new (PointCloudChannelSymbology); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IPointCloudSymbologyChannelPtr IPointCloudSymbologyChannel::Create()  { return PointCloudChannelSymbology::Create(); }
IPointCloudSymbologyChannel::IPointCloudSymbologyChannel()            {;}
IPointCloudSymbologyChannel::~IPointCloudSymbologyChannel()           {;}
uint32_t IPointCloudSymbologyChannel::GetPointSize() const              { return _GetPointSize(); }
void IPointCloudSymbologyChannel::SetPointSize(uint32_t size)           { return _SetPointSize(size); }


/*---------------------------------------------------------------------------------**//**
* UserChannelQueryBuffers
+---------------+---------------+---------------+---------------+---------------+------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
UserChannelQueryBuffers::UserChannelQueryBuffers(uint32_t capacity)
    : m_capacity(capacity)
    {
    }

/*---------------------------------------------------------------------------------**//**
* ChannelBuffers
+---------------+---------------+---------------+---------------+---------------+------*/
UserChannelQueryBuffers::UserChannelQueryBuffers(uint32_t capacity, PointCloudChannelVectorCR userChannels)
    : m_channelVector(userChannels), m_capacity(capacity)
    {
    for (PointCloudChannelVector::const_iterator itr(m_channelVector.begin()); itr != m_channelVector.end(); ++itr)
        {
/* POINTCLOUD_WIP_GR06_UserChannels
        RefCountedPtr<PointCloudChannel> channelPtr = (*itr);
        RefCountedPtr<PointChannelBuffer> bufferPtr = PointChannelBuffer::Create(m_capacity, channelPtr->GetMultiple(), channelPtr->GetTypeSize());
        m_channelBuffers.push_back (UserChannelBuffers::value_type(channelPtr, bufferPtr));
*/
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<void*> UserChannelQueryBuffers::GetChannelBuffers() const
    {
    std::vector<void*> channelBufferVector;

    for (UserChannelBuffers::const_iterator itr (m_channelBuffers.begin()); itr != m_channelBuffers.end(); ++itr)
        {
        BeAssert(m_capacity == itr->second->_GetCapacity());
        channelBufferVector.push_back (itr->second->_GetBufferP());
        }

    return channelBufferVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<PThandle> UserChannelQueryBuffers::GetChannelHandles() const
    {
    std::vector<PThandle> channelHandleVector;

    for (UserChannelBuffers::const_iterator itr (m_channelBuffers.begin()); itr != m_channelBuffers.end(); ++itr)
        channelHandleVector.push_back (itr->first->GetHandle());

    return channelHandleVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void* UserChannelQueryBuffers::GetChannelBuffer(IPointCloudChannelP pChannel) const
    {
    for (UserChannelBuffers::const_iterator itr (m_channelBuffers.begin()); itr != m_channelBuffers.end(); ++itr)
        {
        RefCountedPtr<PointCloudChannel> channelPtr = itr->first;
        if (pChannel == dynamic_cast<IPointCloudChannelP>(channelPtr.get()))
            {
            BeAssert(m_capacity == itr->second->_GetCapacity());
            return itr->second->_GetBufferP();
            }
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void UserChannelQueryBuffers::SetPointChannels(PointCloudChannelVectorCR channels)
    {
    m_channelVector = channels;
    m_channelBuffers.clear();

    for (PointCloudChannelVector::const_iterator itr(m_channelVector.begin()); itr != m_channelVector.end(); ++itr)
        {
        RefCountedPtr<PointCloudChannel> channelPtr = (*itr);
        RefCountedPtr<PointChannelBuffer> bufferPtr = PointChannelBuffer::Create(m_capacity, channelPtr->GetTypeSize(), channelPtr->GetMultiple());
        m_channelBuffers.push_back (UserChannelBuffers::value_type(channelPtr, bufferPtr));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t UserChannelQueryBuffers::GetNumberOfPoints() const
    {
    uint32_t numberOfPoints = 0;
    for (UserChannelBuffers::const_iterator itr (m_channelBuffers.begin()); itr != m_channelBuffers.end(); ++itr)
        numberOfPoints = (numberOfPoints > 0) ? std::min(numberOfPoints, itr->second->_GetNumPoints()) : itr->second->_GetNumPoints();

    return numberOfPoints;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void UserChannelQueryBuffers::SetNumberOfPoints(uint32_t numberOfPoints)
    {
    for (UserChannelBuffers::const_iterator itr (m_channelBuffers.begin()); itr != m_channelBuffers.end(); ++itr)
        (itr->second)->_SetNumPoints(numberOfPoints);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void UserChannelQueryBuffers::ChangeCapacity(uint32_t capacity)
    {
    for (UserChannelBuffers::iterator itr (m_channelBuffers.begin());  itr != m_channelBuffers.end();  ++itr)
        {
        if (m_capacity != itr->second->_GetCapacity())
            itr->second = PointChannelBuffer::Create(m_capacity, itr->first->GetMultiple(), itr->first->GetTypeSize());
        }
    
    m_capacity = capacity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t UserChannelQueryBuffers::GetCapacity() const
    {
    return m_capacity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudChannelVectorCR UserChannelQueryBuffers::GetPointChannels()
    {
    return m_channelVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
UserChannelQueryBuffers::UserChannelBuffersCR UserChannelQueryBuffers::GetPointChannelBuffers()
    {
    return m_channelBuffers;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
UserChannelQueryBuffersPtr UserChannelQueryBuffers::Create(uint32_t capacity)
    {
    return new UserChannelQueryBuffers(capacity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
UserChannelQueryBuffersPtr UserChannelQueryBuffers::Create(uint32_t capacity, PointCloudChannelVectorCR userChannels)
    {
    return new UserChannelQueryBuffers(capacity, userChannels);
    }


#if defined (POINTCLOUD_WIP_GR06_ElementHandle)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ChannelHandlerOnDisplayCallerPtr ChannelHandlerOnDisplayCaller::Create(ElementHandleCR eh, ViewContextR context)
    {
    return new ChannelHandlerOnDisplayCaller(eh, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChannelHandlerOnDisplayCaller::CallOnClassif (PointCloudQueryBuffersR pBuffers)
    {
    bool haveClassif (false);
    for (PointCloudChannelHandlers::iterator itr(m_globalChannelHandlers.begin()); itr != m_globalChannelHandlers.end(); ++itr)
        {
        IPointCloudChannelDisplayHandler* displHandler (dynamic_cast <IPointCloudChannelDisplayHandler *>(*itr));
        haveClassif |= displHandler->_OnPreProcessClassification (m_context, m_eh, NULL, pBuffers);
        }

    for (PointCloudChannelVector::iterator itr(m_channels.begin()); itr != m_channels.end(); ++itr)
        {
        haveClassif |= (*itr)->GetDisplayHandler()->_OnPreProcessClassification (m_context, m_eh, *itr, pBuffers);
        }

    return haveClassif;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ChannelHandlerOnDisplayCaller::_Initialize()
    {
    m_displayHandlers.clear();
    T_Super::_Initialize();

    // save the display handlers in a vector, to avoid dynamic casting several times during a single draw
    for (PointCloudChannelHandlers::iterator itr(m_globalChannelHandlers.begin()); itr != m_globalChannelHandlers.end(); ++itr)
        {
        IPointCloudChannelDisplayHandler* displHandler (dynamic_cast <IPointCloudChannelDisplayHandler *>(*itr));
        if (displHandler)
            m_displayHandlers.push_back (displHandler);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ChannelHandlerOnDisplayCaller::_CallAllHandlers(PointCloudQueryBuffersR pBuffers)
    {
    for (PointCloudChannelVector::iterator itr(m_channels.begin()); itr != m_channels.end(); ++itr)
        {
        (*itr)->GetDisplayHandler()->_OnDisplay(m_context, m_eh, *itr, pBuffers);
        }

    for (bvector<IPointCloudChannelDisplayHandler*>::iterator itr (m_displayHandlers.begin()); itr != m_displayHandlers.end(); ++itr)
        {
        (*itr)->_OnDisplay(m_context, m_eh, NULL, pBuffers);
        }    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChannelHandlerOnDisplayCaller::_Handles(PointCloudChannelHandler* handlerP) const
    {
    IPointCloudChannelDisplayHandler* pQueryHandler = dynamic_cast<IPointCloudChannelDisplayHandler*>(handlerP);

    if (pQueryHandler && pQueryHandler->_CanHandle (m_context, m_eh))
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ChannelHandlerOnQueryCallerPtr ChannelHandlerOnQueryCaller::Create(ElementHandleCR eh, IPointCloudDataQueryCR query)
    {
    return new ChannelHandlerOnQueryCaller(eh, query);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ChannelHandlerOnQueryCaller::_Initialize()
    {
    m_queryHandlers.clear();
    T_Super::_Initialize();

    for (PointCloudChannelHandlers::iterator itr(m_globalChannelHandlers.begin()); itr != m_globalChannelHandlers.end(); ++itr)
        {
        IPointCloudChannelQueryHandler* qHandler (dynamic_cast <IPointCloudChannelQueryHandler *>(*itr));
        if (qHandler)
            m_queryHandlers.push_back (qHandler);
        }
    }


/*---------------------------------------------------------------------------------**//**
* No need to dynamic cast the handlers, they were validated in _Handles method
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ChannelHandlerOnQueryCaller::_CallAllHandlers(PointCloudQueryBuffersR pBuffers)
    {
    for (PointCloudChannelVector::iterator itr(m_channels.begin()); itr != m_channels.end(); ++itr)
        {
        PointCloudChannel* pChannel = *itr;

        pChannel->GetQueryHandler()->_OnQuery(m_query, m_eh, pChannel, pBuffers);
        }

    for (bvector<IPointCloudChannelQueryHandler*>::iterator itr (m_queryHandlers.begin()); itr != m_queryHandlers.end(); ++itr)
        {
        (*itr)->_OnQuery(m_query, m_eh, NULL, pBuffers);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChannelHandlerOnQueryCaller::_Handles(PointCloudChannelHandler* handlerP) const
    {
    IPointCloudChannelQueryHandler* pQueryHandler = dynamic_cast<IPointCloudChannelQueryHandler*>(handlerP);

    if (pQueryHandler && pQueryHandler->_CanHandle (m_query, m_eh))
        return true;

    return false;
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ChannelHandlerCaller::ChannelHandlerCaller (ElementHandleCR eh)
    :m_eh(eh), m_init (false), m_filterP(NULL)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ChannelHandlerCaller::SetChannelHandlerFilter (IPointCloudChannelHandlerFilterP filterP)
    {
    m_filterP = filterP;
    if (m_init)
        {
        m_globalChannelHandlers.clear();
        m_channels.clear();
        //init again
        _Initialize ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ChannelHandlerCaller::_Initialize ()
    {
    m_init = true;

    PointCloudChannelHandlers const&  globalHandlers = PointCloudChannelHandlerManager::GetManager().GetHandlers();
    for (PointCloudChannelHandlers::const_iterator itr(globalHandlers.begin()); itr != globalHandlers.end(); ++itr)
        {
        if (_Handles(*itr) && (!m_filterP || m_filterP->_Filter (*(*itr), NULL)))
            m_globalChannelHandlers.push_back (*itr);
        }

    for (IPointCloudChannelPtrMap::const_iterator itrObj (PointCloudChannelManager::GetManager().GetChannels().begin());
        itrObj != PointCloudChannelManager::GetManager().GetChannels().end();
        ++itrObj)
        {
        PointCloudChannel* pChannel (dynamic_cast<PointCloudChannel *>((itrObj->second).get()));

        if (pChannel && pChannel->GetHandler () && _Handles (pChannel->GetHandler ()) &&
              (!m_filterP || m_filterP->_Filter (*pChannel->GetHandler (), pChannel)))
            {
            m_channels.push_back(pChannel);
            }
        }
//*/
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ChannelHandlerCaller::GetChannelHandlers (PointCloudChannelHandlers& handlers)
    {
    if (!m_init)
        _Initialize();

    handlers.insert (handlers.end(), m_globalChannelHandlers.begin(), m_globalChannelHandlers.end());

    for (PointCloudChannelVector::const_iterator itr(m_channels.begin()); itr != m_channels.end(); ++itr)
        {
        handlers.push_back((*itr)->GetHandler());
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ChannelHandlerCaller::CombineFlags(uint32_t channelFlags)
    {
    if (!m_init)
        _Initialize();

    // Check which channels are requested
    for (PointCloudChannelHandlers::const_iterator itr(m_globalChannelHandlers.begin()); itr != m_globalChannelHandlers.end(); ++itr)
        {
        combineFlags (channelFlags, (*itr)->_GetModifiedChannels (), (*itr)->_GetRequiredChannels ());
        }

    for (PointCloudChannelVector::const_iterator itr(m_channels.begin()); itr != m_channels.end(); ++itr)
        {
        PointCloudChannelHandler* handlerP = (*itr)->GetHandler();

        combineFlags (channelFlags, handlerP->_GetModifiedChannels (), handlerP->_GetRequiredChannels ());
        }


    return channelFlags;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ChannelHandlerCaller::AddModifyingChannelFlags(uint32_t& channelFlags)
    {
    if (!m_init)
        _Initialize();

    for (PointCloudChannelHandlers::const_iterator itr(m_globalChannelHandlers.begin()); itr != m_globalChannelHandlers.end(); ++itr)
        {
        channelFlags |= (*itr)->_GetModifiedChannels ();
        }

    for (PointCloudChannelVector::const_iterator itr(m_channels.begin()); itr != m_channels.end(); ++itr)
        {
        PointCloudChannelHandler* handlerP = (*itr)->GetHandler();
        channelFlags |= handlerP->_GetModifiedChannels ();
        }
    }

#endif




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
FilterChannelEditor::FilterChannelEditor(unsigned char* FilterChannelEditor) : m_buf(FilterChannelEditor)
    {
    BeAssert(NULL != m_buf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void FilterChannelEditor::Show (uint32_t index)
    {
    PointCloudChannels_Show_Point(m_buf[index]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void FilterChannelEditor::Hide (uint32_t index)
    {
    PointCloudChannels_Hide_Point(m_buf[index]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool FilterChannelEditor::IsVisible (uint32_t index) const
    {
    return PointCloudChannels_Is_Point_Visible (m_buf[index]) ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool FilterChannelEditor::IsHidden (uint32_t index) const
    {
    return PointCloudChannels_Is_Point_Hidden (m_buf[index]) ? true : false;

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool FilterChannelEditor::IsSelected (uint32_t index) const
    {
    return PointCloudChannels_Is_Point_Selected (m_buf[index]) ? true : false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
FilterChannelEditorPtr FilterChannelEditor::Create(unsigned char* filterBuffer)
    {
    if (NULL != filterBuffer)
        return new FilterChannelEditor(filterBuffer);

    return NULL;
    }
