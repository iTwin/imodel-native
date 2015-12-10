/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/RasterLineStyle.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LsRasterImageComponent::LsRasterImageComponent(LsRasterImageComponentCR source) : LsComponent(&source)
    {
    m_flags     = source.m_flags;
    m_size      = source.m_size;
    m_trueWidth = source.m_trueWidth;
    m_image.resize(source.m_image.size());
    memcpy (&m_image.front(), &source.m_image.front(), m_image.size());
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
BentleyStatus   LsRasterImageComponent::_GetTextureWidth (double& width) const 
    {
    if (0 == (m_flags & FlagMask_TrueWidth))
        return ERROR;

    width = m_trueWidth;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LsComponentPtr LsRasterImageComponent::_Import(DgnImportContext& importer) const
    {
    LsRasterImageComponentP result = new LsRasterImageComponent(*this);

    //  Save to destination and record ComponentId in clone

    return result;
    }

