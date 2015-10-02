/*--------------------------------------------------------------------------------------+                         
|
|     $Source: DgnCore/RenderMaterial.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

//=======================================================================================
// RenderMaterial...
//=======================================================================================
#define DEFAULT_Finish          .05
#define DEFAULT_Specular        .05
#define DEFAULT_Diffuse         .5
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double      RenderMaterial::_GetDouble (char const* key, BentleyStatus* status) const
    {
    if (NULL != status)
        *status = SUCCESS;

    if (0 == strcmpi (key, RENDER_MATERIAL_Finish))
        return DEFAULT_Finish;

    if (0 == strcmpi (key, RENDER_MATERIAL_Diffuse))
        return DEFAULT_Diffuse;

    if (0 == strcmpi (key, RENDER_MATERIAL_Specular))
        return DEFAULT_Specular;

    if (0 == strcmpi (key, RENDER_MATERIAL_Reflect) ||
        0 == strcmpi (key, RENDER_MATERIAL_Transmit))
        return 0.0;

    if (0 == strcmpi (key, RENDER_MATERIAL_Glow) ||
        0 == strcmpi (key, RENDER_MATERIAL_Refract))
        return 1.0;

    BeAssert (false);
    if (NULL != status)
        *status = ERROR;

    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool        RenderMaterial::_GetBool (char const* key, BentleyStatus* status) const
    {
    if (NULL != status)
        *status = SUCCESS;

    if (0 == strcmpi (key, RENDER_MATERIAL_FlagHasBaseColor) ||
        0 == strcmpi (key, RENDER_MATERIAL_FlagHasSpecularColor) ||
        0 == strcmpi (key, RENDER_MATERIAL_FlagHasFinish) ||
        0 == strcmpi (key, RENDER_MATERIAL_FlagNoShadows))
        return false;

    BeAssert (false);
    if (NULL != status)
        *status = ERROR;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RgbFactor  RenderMaterial::_GetColor (char const* key, BentleyStatus* status) const
    {
    BeAssert (false);
    if (NULL != status)
        *status = ERROR;

    RgbFactor   color  = {1.0, 1.0, 1.0};

    return color;
    }

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

    if (0 == strcmpi (key, RENDER_MATERIAL_PatternAngle))
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

    if (0 == strcmpi (key, RENDER_MATERIAL_PatternFlipU) ||
        0 == strcmpi (key, RENDER_MATERIAL_PatternFlipV))
        return false;

    
    BeAssert (false);
    if (NULL != status)
        *status = ERROR;

    return false;
    }


//=======================================================================================
// JSonRenderMaterial...
//=======================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RenderMaterialPtr    JsonRenderMaterial::Create (DgnDbCR dgnDb, DgnMaterialId materialId)
    {
    if (!materialId.IsValid())
        return nullptr;

    DgnMaterials::Material material = dgnDb.Materials().Query (materialId);

    if (!material.IsValid())
        {
        BeAssert (false);
        return nullptr;
        }
    Json::Value     renderMaterial;
    
    if (SUCCESS != material.GetRenderingAsset (renderMaterial))
        {
        BeAssert (false);
        return nullptr;
        }

    return new JsonRenderMaterial (renderMaterial, materialId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static double  getDoubleValue (Json::Value const& rootValue, char const* key, BentleyStatus* status)
    {
    Json::Value     value = rootValue[key];

    if (!value.isDouble())
        {
        if (NULL != status) 
            *status = ERROR;

        return 0.0;
        }
    if (NULL != status) 
        *status = SUCCESS;
    return value.asDouble();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static bool  getBoolValue (Json::Value const& rootValue, char const* key, BentleyStatus* status)
    {
    Json::Value     value = rootValue[key];

    if (!value.isBool())
        {
        if (NULL != status) 
            *status = ERROR;

        return false;
        }
    if (NULL != status) 
        *status = SUCCESS;

    return value.asBool();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
static RgbFactor getColorValue (Json::Value const& rootValue, const char* key, BentleyStatus* status)
    {
    Json::Value     value = rootValue[key];
    RgbFactor       rgb = {0.0, 0.0, 0.0};

    if (value.size() < 3)
        {
        BeAssert (false);
        if (NULL != status) 
            *status = ERROR;

        return rgb;
        }
    rgb.red     = value[0].asDouble();
    rgb.green   = value[1].asDouble();
    rgb.blue    = value[2].asDouble();
    
    if (NULL != status) 
        *status = SUCCESS;

    return rgb;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     08/2015
//---------------------------------------------------------------------------------------
static DPoint2d getDPoint2dValue (Json::Value const& rootValue, const char* key, BentleyStatus* status)
    {
    Json::Value     value = rootValue[key];
    DPoint2d        point = { 0.0, 0.0 };

    if (value.size() < 2)
        {
        BeAssert (false);
        if (NULL != status) 
            *status = ERROR;

        return point;
        }
    point.x     = value[0].asDouble();
    point.y     = value[1].asDouble();
    
    if (NULL != status) 
        *status = SUCCESS;

    return point;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double      JsonRenderMaterial::_GetDouble (char const* key, BentleyStatus* status) const     { return getDoubleValue (m_value, key, status); }
bool        JsonRenderMaterial::_GetBool (char const* key, BentleyStatus* status) const       { return getBoolValue (m_value, key, status); }
RgbFactor   JsonRenderMaterial::_GetColor (char const* key, BentleyStatus* status) const      { return getColorValue (m_value, key, status); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RenderMaterialMapPtr   JsonRenderMaterial::_GetMap (char const* key) const
    {
    Json::Value     mapsMap, patternMap;

    if ((mapsMap = m_value[RENDER_MATERIAL_Map]).isNull() ||
        (patternMap = mapsMap[key]).isNull())
        return nullptr;

    return JsonRenderMaterialMap::Create (patternMap);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
uintptr_t  JsonRenderMaterial::_GetQvMaterialId (DgnDbR dgnDb, bool createIfNotFound) const 
    {
    uintptr_t   qvMaterialId;

    if (0 == (qvMaterialId = dgnDb.Materials().GetQvMaterialId (m_materialId)) && createIfNotFound)
        qvMaterialId = dgnDb.Materials().AddQvMaterialId (m_materialId);

    return qvMaterialId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double      JsonRenderMaterialMap::_GetDouble (char const* key, BentleyStatus* status) const  {  return getDoubleValue (m_value, key, status); }
bool        JsonRenderMaterialMap::_GetBool (char const* key, BentleyStatus* status) const    {  return getBoolValue (m_value, key, status); }
DPoint2d    JsonRenderMaterialMap::_GetScale (BentleyStatus* status) const                    {  return getDPoint2dValue (m_value, RENDER_MATERIAL_PatternScale, status); }
DPoint2d    JsonRenderMaterialMap::_GetOffset(BentleyStatus* status) const                    {  return getDPoint2dValue (m_value, RENDER_MATERIAL_PatternOffset, status); }
 



//=======================================================================================
// JsonRenderMaterialMap...
//=======================================================================================


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RenderMaterialMap::Mode JsonRenderMaterialMap::_GetMode () const 
    {
    Json::Value     value = m_value[RENDER_MATERIAL_PatternMapping];

    if (!value.isInt())
        {
        BeAssert (false);
        return RenderMaterialMap::Mode::Parametric;
        }
    return (RenderMaterialMap::Mode) value.asInt();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RenderMaterialMap::Units JsonRenderMaterialMap::_GetUnits () const 
    {
    Json::Value     value = m_value[RENDER_MATERIAL_PatternScaleMode];

    if (!value.isInt())
        {
        BeAssert (false);
        return RenderMaterialMap::Units::Relative;
        }
    return (RenderMaterialMap::Units) value.asInt();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   JsonRenderMaterialMap::_GetImage (bvector<Byte>& data, Point2dR imageSize, DgnDbR dgnDb) const
    {
    Json::Value     textureIdValue = m_value[RENDER_MATERIAL_TextureId];

    if (textureIdValue.isNull())     
        return ERROR;                 // No external file support for now.

    DgnTextureId            textureId = (DgnTextureId) textureIdValue.asUInt64();
    DgnTextures::Texture    texture = dgnDb.Textures().Query (textureId);

    if (!texture.IsValid())
        {
        BeAssert (false);
        return ERROR;
        }

    return texture.GetImage (data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
uintptr_t  JsonRenderMaterialMap::_GetQvTextureId (DgnDbR dgnDb, bool createIfNotFound) const 
    {
    Json::Value     textureIdValue = m_value[RENDER_MATERIAL_TextureId];

    if (textureIdValue.isNull())     
        return ERROR;                 // No external file support for now.

    DgnTextureId    textureId = (DgnTextureId) textureIdValue.asUInt64();
    uintptr_t       qvTextureId;

    if (0 == (qvTextureId = dgnDb.Textures().GetQvTextureId (textureId)) && createIfNotFound)
        qvTextureId = dgnDb.Textures().AddQvTextureId (textureId);

    return qvTextureId;
    }

//=======================================================================================
// SimpleBufferPatternMap...
//=======================================================================================
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SimpleBufferPatternMap::SimpleBufferPatternMap (Byte const* imageData, Point2dCR imageSize) : m_imageSize (imageSize), m_qvTextureId (0)
    {
    size_t      dataSize = 4 * imageSize.x * imageSize.y;

    m_imageData.resize (dataSize);                     
    memcpy (&m_imageData.front(), imageData, dataSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SimpleBufferPatternMap::~SimpleBufferPatternMap ()
    {
    // Needs work.   Free QVision texture.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  SimpleBufferPatternMap::_GetImage (bvector<Byte>& imageData, Point2dR imageSize, DgnDbR dgnDb) const
    {
    imageData = m_imageData;
    imageSize = m_imageSize;

    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
uintptr_t SimpleBufferPatternMap::_GetQvTextureId (DgnDbR dgnDb, bool createIfNotFound) const 
    {
    if (createIfNotFound)
        return m_qvTextureId = (uintptr_t) this;        // These textures will not be shared -- use own memory address as qvTextureId.

    return m_qvTextureId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RenderMaterialMapPtr  SimpleBufferPatternMap::Create (Byte const* imageData, Point2dCR imageSize)
    {
    return new SimpleBufferPatternMap (imageData, imageSize);
    }

//=======================================================================================
// RenderMaterialUtils...
//=======================================================================================


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void     RenderMaterialUtil::SetColor (Json::Value& renderMaterial, char const* keyword, RgbFactorCR color)
    {
    Json::Value     colorValue;

    colorValue[0] = color.red;
    colorValue[1] = color.green;
    colorValue[2] = color.blue;

    renderMaterial[keyword] = colorValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void     RenderMaterialUtil::SetPoint (Json::Value& renderMaterial, char const* keyword, DPoint3dCR point)
    {
    Json::Value     pointValue;

    pointValue[0] = point.x;
    pointValue[1] = point.y;
    pointValue[2] = point.z;

    renderMaterial[keyword] = pointValue;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double     RenderMaterialUtil::GetMapUnitScale (RenderMaterialMap::Units mapUnits)
    {
    switch (mapUnits)
        {
        default:
            BeAssert (false);
            return 1.0;

        case RenderMaterialMap::Units::Meters:
            return 1.0;

        case RenderMaterialMap::Units::Millimeters:
            return UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters).GetConversionFactorFrom (UnitDefinition::GetStandardUnit (StandardUnit::MetricMillimeters));

        case RenderMaterialMap::Units::Feet:
            return UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters).GetConversionFactorFrom (UnitDefinition::GetStandardUnit (StandardUnit::EnglishFeet));
            
        case RenderMaterialMap::Units::Inches:
            return UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters).GetConversionFactorFrom (UnitDefinition::GetStandardUnit (StandardUnit::EnglishInches));
        }
    }

