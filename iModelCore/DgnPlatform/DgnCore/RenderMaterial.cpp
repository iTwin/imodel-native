/*--------------------------------------------------------------------------------------+                         
|
|     $Source: DgnCore/RenderMaterial.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JsonRenderMaterial::Load(DgnMaterialId materialId, DgnDbR dgnDb)
    {
    DgnMaterialCPtr material = DgnMaterial::QueryMaterial(materialId, dgnDb);
    return material.IsValid() ? material->GetRenderingAsset(m_value) : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RgbFactor  JsonRenderMaterial::GetColor(Utf8CP name) const
    {
    RgbFactor rgb = {0.0, 0.0, 0.0};

    JsonValueCR value = m_value[name];
    if (value.size() < 3)
        {
        BeAssert (false);
        return rgb;
        }

    rgb.red   = value[0].asDouble();
    rgb.green = value[1].asDouble();
    rgb.blue  = value[2].asDouble();
    return rgb;
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
//=======================================================================================
// RenderMaterialMap...
//=======================================================================================
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double      RenderMaterialMap::_GetDouble (char const* key, BentleyStatus* status) const
    {
    if (NULL != status)
        *status = SUCCESS;

    if (0 == BeStringUtilities::Stricmp (key, RENDER_MATERIAL_PatternAngle))
        return 0.0;

    BeAssert (false);
    if (NULL != status)
        *status = ERROR;

    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool        RenderMaterialMap::_GetBool (char const* key, BentleyStatus* status) const
    {
    if (NULL != status)
        *status = SUCCESS;

    if (0 == BeStringUtilities::Stricmp (key, RENDER_MATERIAL_PatternFlipU) ||
        0 == BeStringUtilities::Stricmp (key, RENDER_MATERIAL_PatternFlipV) ||
        0 == BeStringUtilities::Stricmp (key, RENDER_MATERIAL_PatternTileSection))
        return false;

    BeAssert (false);
    if (NULL != status)
        *status = ERROR;

    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RenderMaterialPtr    JsonRenderMaterial::Create (Json::Value const& value, DgnMaterialId materialId) { return new JsonRenderMaterial (value, materialId); }
#endif
double Texture::GetUnitScale(Units units)
    {
    switch (units)
        {
        case Units::Meters:
            return 1.0;

        case Units::Millimeters:
            return UnitDefinition::GetStandardUnit(StandardUnit::MetricMillimeters).ToMeters();

        case Units::Feet:
            return UnitDefinition::GetStandardUnit(StandardUnit::EnglishFeet).ToMeters();
            
        case Units::Inches:
            return UnitDefinition::GetStandardUnit(StandardUnit::EnglishInches).ToMeters();

        default:
            BeAssert(false);
            return 1.0;
        }
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    return new JsonRenderMaterial (renderMaterial, materialId);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Texture::Mode Texture::GetMode(JsonValueCR in) 
    {
    JsonValueCR value = in[RENDER_MATERIAL_PatternMapping];

    if (!value.isInt())
        {
        BeAssert(false);
        return Mode::Parametric;
        }

    return (Mode) value.asInt();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Texture::Units Texture::GetUnits(JsonValueCR in) 
    {
    JsonValueCR value = in[RENDER_MATERIAL_PatternScaleMode];
    if (!value.isInt())
        {
        BeAssert(false);
        return Units::Relative;
        }

    return (Units) value.asInt();
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
static DPoint2d getDPoint2dValue(JsonValueCR value)
    {
    DPoint2d point = { 0.0, 0.0 };

    if (value.size() < 2)
        {
        BeAssert(false);
        return point;
        }

    point.x = value[0].asDouble();
    point.y = value[1].asDouble();
    return point;
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                               Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
Render::Texture::Trans Render::Texture::GetPatternTransform(JsonValueCR value)
    {
    Render::Texture::Trans transform;
    for (size_t i=0; i<2; ++i)
        {
        for (size_t j=0; j<3; ++j)
            transform.m_val[i][j] = (i==j) ? 1.0 : 0.0;
        }
    return transform;
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JsonRenderMaterial::DoImport (DgnImportContext& context, DgnDbR sourceDb) 
    {
    Json::Value&      mapsMap = m_value[RENDER_MATERIAL_Map];
    
    if (mapsMap.isNull())
        return ERROR;

    for (auto& map : mapsMap)
        {
        RenderMaterialMapPtr    renderMaterialMap = JsonRenderMaterialMap::Create (map);
        JsonRenderMaterialMap*  jsonRenderMaterialMap = dynamic_cast <JsonRenderMaterialMap*> (renderMaterialMap.get());

        if (NULL != jsonRenderMaterialMap &&
            SUCCESS == jsonRenderMaterialMap->DoImport (context, sourceDb))
            map = jsonRenderMaterialMap->GetValue();
        }
    
    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2015
//----------------------------------------------------------------------------------------
uint32_t            ImageBuffer::GetWidth() const       {return m_width;}
uint32_t            ImageBuffer::GetHeight() const      {return m_height;}
Byte*               ImageBuffer::GetDataP()             {return m_data.data();}
Byte const*         ImageBuffer::GetDataCP() const      {return m_data.data();}
size_t              ImageBuffer::GetDataSize() const    {return m_data.size();}
ImageBuffer::Format ImageBuffer::GetFormat() const      {return m_format;}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2015
//----------------------------------------------------------------------------------------
ImageBuffer::ImageBuffer(uint32_t width, uint32_t height, ImageBuffer::Format const& format, bvector<Byte>& data)
:m_width(width),
 m_height(height),
 m_format(format)
    {
    BeAssert(data.size() == m_width*m_height*ImageBuffer::BytesPerPixel(m_format));

    m_data = std::move(data);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2015
//----------------------------------------------------------------------------------------
ImageBufferPtr ImageBuffer::Create(uint32_t width, uint32_t height, ImageBuffer::Format const& format)
    {
    bvector<Byte> data(width*height*ImageBuffer::BytesPerPixel(format));
    return new ImageBuffer(width, height, format, data);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2015
//----------------------------------------------------------------------------------------
ImageBufferPtr ImageBuffer::Create(uint32_t width, uint32_t height, ImageBuffer::Format const& format, bvector<Byte>& data)
    {
    return new ImageBuffer(width, height, format, data);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2015
//----------------------------------------------------------------------------------------
size_t ImageBuffer::BytesPerPixel(Format const& format)
    {
    switch(format)
        {
        case Format::Rgba:
        case Format::Bgra:
            return 4;
        case Format::Rgb:
        case Format::Bgr:
            return 3;
        case Format::Gray:
            return 1;
        default:
            BeAssert(!"ImageBuffer::BytesPerPixel -> unknown pixel format");
            return 4;
        }   
    }


//=======================================================================================
// JsonRenderMaterialMap...
//=======================================================================================

/*---------------------------------------------------------------------------------**//**
    double          angleRadians    = Angle::DegreesToRadians(value[RENDER_MATERIAL_PatternAngle].asDouble());
    double          cosAngle        = cos(angleRadians);
    double          sinAngle        = sin(angleRadians);
    bool            xFlip           = value[RENDER_MATERIAL_PatternFlipU].asBool();
    bool            yFlip           = value[RENDER_MATERIAL_PatternFlipV].asBool();
    DPoint2d        scale           = getDPoint2dValue(value[RENDER_MATERIAL_PatternScale]);
    DPoint2d        offset          = getDPoint2dValue(value[RENDER_MATERIAL_PatternOffset]);
    static double   s_minScale      = 1.0E-10;
BentleyStatus   JsonRenderMaterialMap::_GetImage (bvector<Byte>& data, Point2dR imageSize, DgnDbR dgnDb) const
 
    Units units = GetUnits(value);
        return ERROR;                 // No external file support for now.
    if (Units::Relative != units)
        scale.Scale(GetUnitScale(units));

        return ERROR;
    scale.x = std::max(scale.x, s_minScale);

    return texture->GetImage (data);
        return nullptr;        

    return ImageBuffer::Create(texture->GetData().GetWidth(), texture->GetData().GetHeight(), ImageBuffer::Format::Rgba, data);
    if (xFlip)
        scale.x = -scale.x;

    if (yFlip)
        scale.y = -scale.y;

    transform.m_val[0][0] = cosAngle / scale.x;
    transform.m_val[0][1] = sinAngle / scale.x;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JsonRenderMaterialMap::DoImport (DgnImportContext& context, DgnDbR sourceDb) 
    {
#ifdef WIP_MERGE_YII
    Json::Value&     textureIdValue = m_value[RENDER_MATERIAL_TextureId];

    if (textureIdValue.isNull())     
        return ERROR;                 // No external file support for now.

    textureIdValue = (uint64_t) context.GetDestinationDb().Textures().ImportTexture (context, sourceDb, (DgnTextureId) textureIdValue.asUInt64()).GetValue();

    return SUCCESS;
#else
    return ERROR;
#endif
    }

SimpleBufferPatternMap::SimpleBufferPatternMap (Byte const* imageData, Point2dCR imageSize) : m_imageSize (imageSize), m_qvTextureId (0)
    transform.m_val[0][2] = offset.x;

    transform.m_val[1][0] =  sinAngle / scale.y;
    transform.m_val[1][1] = -cosAngle / scale.y;
BentleyStatus  SimpleBufferPatternMap::_GetImage (bvector<Byte>& imageData, Point2dR imageSize, DgnDbR dgnDb) const



    return transform;
RenderMaterialMapPtr  SimpleBufferPatternMap::Create (Byte const* imageData, Point2dCR imageSize)
    return new SimpleBufferPatternMap (imageData, imageSize);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Texture::SetColor(JsonValueR out, Utf8CP keyword, RgbFactorCR color)
    {
    Json::Value     colorValue;

    colorValue[0] = color.red;
    colorValue[1] = color.green;
    colorValue[2] = color.blue;

    out[keyword] = colorValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Texture::SetPoint(JsonValueR out, Utf8CP keyword, DPoint3dCR point)
    {
    Json::Value     pointValue;

    pointValue[0] = point.x;
    pointValue[1] = point.y;
    pointValue[2] = point.z;

    out[keyword] = pointValue;
    }
