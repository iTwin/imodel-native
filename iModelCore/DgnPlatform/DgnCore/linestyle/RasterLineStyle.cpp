/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/RasterLineStyle.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#if defined(NOTNOW)
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Ray.Bentley     02/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    StatusInt   LsRasterImageComponent::_SaveToResourceFile ()
        {
        StatusInt   status;
    
        if (BSISUCCESS != (status = EnsureRscIDAssigned ()))
            return status;
    
        //  Generate the resource data
        size_t                  resourceSize = RL_RSCSIZE (GetImageBufferSize());
        RasterLineStyleRsc*     resource = (RasterLineStyleRsc*)_alloca (resourceSize);
    
        SaveToResource (resource, m_size.x, m_size.y, m_flags, m_trueWidth, &m_image.front(), m_image.size());
        m_isDirty = false;
        return WriteToResourceFile (resource, resourceSize);
        }


    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Ray.Bentley     02/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    void        LsRasterImageComponent::SaveToResource (RasterLineStyleRsc* resource, uint32_t width, uint32_t height, uint32_t flags, double trueWidth, uint8_t const* imageData, size_t imageDataSize)
        {
        BeAssert (4 * width * height == imageDataSize);

        memset (resource, 0, sizeof (RasterLineStyleRsc));
        resource->m_size.x  = width;
        resource->m_size.y  = height;
        resource->m_flags   = flags;
        resource->m_trueWidth = trueWidth;
        resource->m_nImageBytes = (uint32_t) imageDataSize;
        memcpy (resource->m_imageData, imageData, imageDataSize);
        }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
LsRasterImageComponent::LsRasterImageComponent (V10RasterImage* rasterImageResource, LsLocationCP location) : LsComponent (location) 
    {
    m_flags     = rasterImageResource->m_flags;
    m_size      = rasterImageResource->m_size;
    m_trueWidth = rasterImageResource->m_trueWidth;

    BeAssert (GetImageBufferSize() == rasterImageResource->m_nImageBytes);
    
    m_image.resize (rasterImageResource->m_nImageBytes);
    memcpy (&m_image.front(), rasterImageResource->m_imageData, rasterImageResource->m_nImageBytes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LsRasterImageComponent::_GetRasterTexture (uint8_t const*& image, Point2dR imageSize, uint32_t& flags) const
    {
    image = &m_image.front();
    imageSize = m_size;
    flags = m_flags;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LsRasterImageComponent::_GetRasterTextureWidth (double& width) const 
    {
    if (0 == (m_flags & FlagMask_TrueWidth))
        return ERROR;

    width = m_trueWidth;
    return SUCCESS;
    }

