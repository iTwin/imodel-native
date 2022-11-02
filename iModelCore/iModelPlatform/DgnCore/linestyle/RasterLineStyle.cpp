/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
// @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LsRasterImageComponent::_GetRasterTexture (uint8_t const*& image, Point2dR imageSize, uint32_t& flags) const
    {
    image = &m_image.front();
    imageSize = m_size;
    flags = m_flags;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LsRasterImageComponent::_GetTextureWidth (double& width) const 
    {
    if (0 == (m_flags & FlagMask_TrueWidth))
        return ERROR;

    width = m_trueWidth;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
LsRasterImageComponent::LsRasterImageComponent(LsLocationCP pLocation) : LsComponent(pLocation)
    {
    m_size.x = m_size.y = 0;
    m_flags = 0;
    m_trueWidth = 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
LsComponentPtr LsRasterImageComponent::_Import(DgnImportContext& importer) const
    {
    LsRasterImageComponentP result = new LsRasterImageComponent(*this);

    bvector<uint8_t> imageData;
    Json::Value jsonValue;
    SaveToJson(jsonValue, imageData);
    LsComponentId newId;
    AddRasterComponentAsJson(newId, importer.GetDestinationDb(), jsonValue, &imageData[0], (uint32_t)imageData.size());

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void LsRasterImageComponent::SaveToJson(Json::Value& result, bvector<uint8_t>& imageData) const
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
// @bsimethod
//---------------------------------------------------------------------------------------
LineStyleStatus LsRasterImageComponent::CreateFromJson(LsRasterImageComponentP* result, Json::Value const & jsonDef, LsLocationCP location)
    {
    LsRasterImageComponentP comp = new LsRasterImageComponent(location);
    comp->ExtractDescription(jsonDef);
    comp->m_size.x = LsJsonHelpers::GetUInt32(jsonDef, "x", 0);
    comp->m_size.y = LsJsonHelpers::GetUInt32(jsonDef, "y", 0);
    comp->m_flags = LsJsonHelpers::GetUInt32(jsonDef, "flags", 0);
    comp->m_imageDataId = LsJsonHelpers::GetUInt32(jsonDef, "imageId", 0);
    comp->m_trueWidth = LsJsonHelpers::GetDouble(jsonDef, "trueWidth", 0);

    uint32_t propertySize;

    if (BE_SQLITE_ROW != location->GetDgnDb()->QueryPropertySize(propertySize, LineStyleProperty::RasterImage(), comp->m_imageDataId, 0))
        {
        *result = nullptr;
        delete comp;
        return LINESTYLE_STATUS_ComponentNotFound;
        }

    comp->m_image.resize(propertySize);
    location->GetDgnDb()->QueryProperty(&comp->m_image[0], propertySize, LineStyleProperty::RasterImage(), comp->m_imageDataId, 0);

    *result = comp;
    return LINESTYLE_STATUS_Success;
    }