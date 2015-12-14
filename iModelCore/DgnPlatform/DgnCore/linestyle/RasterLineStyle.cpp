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
LsRasterImageComponent::LsRasterImageComponent(LsLocationCP pLocation) : LsComponent(pLocation)
    {
    m_size.x = m_size.y = 0;
    m_flags = 0;
    m_trueWidth = 0;
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
void LsRasterImageComponent::SaveToJson(Json::Value& result, bvector<uint8_t>& imageData)
    {
    LsComponent::SaveToJson(result);

    result["x"] = m_size.x;
    result["y"] = m_size.y;
    result["flags"] = m_flags;
    if (m_trueWidth != 0)
        result["trueWidth"] = m_trueWidth;
    imageData.resize(m_image.size());
    memcpy(&imageData[0], &m_image[0], m_image.size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LineStyleStatus LsRasterImageComponent::CreateFromJson(LsRasterImageComponentPtr& result, Json::Value const & jsonDef, bvector<uint8_t> const& imageData, LsLocationCP location)
    {
    LsRasterImageComponentP comp = new LsRasterImageComponent(location);
    comp->ExtractDescription(jsonDef);
    comp->m_size.x = LsJsonHelpers::GetUInt32(jsonDef, "x", 0);
    comp->m_size.y = LsJsonHelpers::GetUInt32(jsonDef, "y", 0);
    comp->m_flags = LsJsonHelpers::GetUInt32(jsonDef, "flags", 0);

    comp->m_image.resize(imageData.size());
    memcpy(&comp->m_image[0], &imageData[0], imageData.size());

    result = comp;
    return LINESTYLE_STATUS_Success;
    }